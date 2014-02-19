#include "Lex.h"

#include "Input.h"
#include "TokenType.h"
#include "Utils.h"
#include <stdio.h>
#include <ctype.h>
#include <iostream>

using namespace std;

Lex::Lex() {
	input = new Input();
    generateTokens(input);
}

Lex::Lex(const char* filename) {
    input = new Input(filename);
	validID= true;
	narwhal="";
    generateTokens(input);
}

Lex::Lex(istream& istream) {
    input = new Input(istream);
    generateTokens(input);
}

Lex::Lex(const Lex& lex) {
    input = new Input(*lex.input);
    tokens = new vector<Token*>();

    vector<Token*>::iterator iter;
    for(iter=lex.tokens->begin(); iter != lex.tokens->end(); iter++) {
        Token* newToken = new Token(**iter);
        tokens->push_back(newToken);
    }

    index = lex.index;
    state = lex.state;
}

Lex::~Lex(){
    for (int i = 0; i < tokens->size(); i++) {
        delete (*tokens)[i];
    }
    delete tokens;
    delete input;
}

bool Lex::operator==(const Lex& lex) {
    bool result = (tokens->size() == lex.tokens->size()) && (index == lex.index);
    if(result) {
        vector<Token*>::iterator iter1;
        vector<Token*>::iterator iter2;
        iter1 = tokens->begin();
        iter2 = lex.tokens->begin();
        while(result && iter1 != tokens->end() && iter2 != lex.tokens->end()) {
            result = **iter1 == **iter2;
            iter1++;
            iter2++;
        }
        result = result && iter1 == tokens->end() && iter2 == lex.tokens->end();
    }
    return result;
}

string Lex::toString() const {
    int count = 0;
    string result="";
    while(count < tokens->size()) {
        Token* token = (*tokens)[count];
        result += token->toString();
        count++;
    }
    result += "Total Tokens = ";
    string countToString;
    result += itoa(countToString, count);
    result += "\n";
    return result;
}

void Lex::generateTokens(Input* input) {
    tokens = new vector<Token*>();
    index = 0;

    state = Start;
    while(state != End) {
		
        state = nextState();
    }
	state=nextState();
}

Token* Lex::getCurrentToken() {
    return (*tokens)[index];
}

void Lex::advance() {
    index++;
}

bool Lex::hasNext() {
    return index < tokens->size();
}


State Lex::nextState() {
	State result;
    char character;
    switch(state) {
        case Start:               result = getNextState(); break;
        case Comma:               emit(COMMA); result = getNextState(); break;
        case Period:              emit(PERIOD); result = getNextState(); break;
		case Whitespace:		  emit(WHITESPACE); result = getNextState(); break;
		case LeftParen:			  emit(LEFT_PAREN); result = getNextState(); break;
		case Multiply:			  emit(MULTIPLY); result = getNextState(); break;
		case QMark:				  emit(Q_MARK); result = getNextState(); break;
		case Add:				  emit(ADD); result = getNextState(); break;
		case Schemes:			  emit(SCHEMES); result = getNextState(); break;
		case Facts:				  emit(FACTS); result = getNextState(); break;
		case Rules:				  emit(RULES); result = getNextState(); break;
		case Queries:			  emit(QUERIES); result = getNextState(); break;
		case End:				  emit(ENDOF); result= getNextState(); break;
		case RightParen:		  emit(RIGHT_PAREN); result = getNextState(); break;
		case Endline:			  emit(ENDLINE); result= getNextState(); break;
		case notHandled:		  emit(NOTHANDLED); result= getNextState(); break;
		case Colon_Dash:          emit(COLON_DASH); result = getNextState(); break;
        case SawColon:
            character = input->getCurrentCharacter();
            if(character == '-') {
                result = Colon_Dash;
                input->advance();
            } else { //Every other character
                emit(COLON); result = getNextState();
            }
            break;
        case ProcessingString:  
            character = input->getCurrentCharacter();
            if(character == '\'') {
                result = PossibleEndOfString;
            } else if(character == -1) {
				result= notHandled;
            } else { //Every other character
                result = ProcessingString;
            }
            input->advance();
            break;
        case PossibleEndOfString:
            if(input->getCurrentCharacter() == '\'') {
                input->advance();
                result = ProcessingString;
            } else { //Every other character
                emit(STRING);
                result = getNextState();
            }
            break;
		case Id:
			character = input->getCurrentCharacter();

			if (!isalnum(character))
			{
				if(!validID)
				{
					result= notHandled;
					break;
				}

				if(narwhal == "Schemes")
				{
					result= Schemes;
				}
				else if(narwhal == "Rules")
				{
					result= Rules;
				}
				else if(narwhal == "Facts")
				{
					result= Facts;
				}
				else if(narwhal == "Queries")
				{
					result= Queries;
				}
				else
				{
					emit(ID);
					result= getNextState();
				}
			}
			else
			{
				if (!isalnum(character))
				{
					validID=false;
					input->advance();
					character= input->getCurrentCharacter();
				}

				result= Id;
				narwhal+= character;

				input->advance();
			}	
			break;

		case SawHashtag:
			character= input->getCurrentCharacter();

			if(character == '|')
			{
				result= StartBlock;
				input->advance();
			}
			else 
			{
				result= SingleLineComment;
			}
			break;

		case SingleLineComment:
			character= input->getCurrentCharacter();
			if(character == '\n' || character == -1)
			{
				emit(COMMENT);
				result = getNextState();
			}
			else 
			{ //Every other character
				result = SingleLineComment;
			}
			input->advance();
			break;


		case StartBlock:
			character= input->getCurrentCharacter();

			if(character == '|')
			{
				result = PossibleEndOfComment;
			}
			else if(character == -1) 
			{
				result= notHandled;
			}
			else
			{
				result= StartBlock;
			}
			input->advance();
			break;

		case PossibleEndOfComment:

			if (input->getCurrentCharacter() == '#')
			{
				input->advance();
				emit(COMMENT);
				result = getNextState();
			}
			else if (input->getCurrentCharacter() == -1)
			{
				result= notHandled;
			}
			else
			{
				result = StartBlock;
				input->advance();
			}
			break;

			
		default:
			break;
			
    };
    return result;
}

State Lex::getNextState() {
    State result;
    char currentCharacter = input->getCurrentCharacter();

    //The handling of checking for whitespace and setting the result to Whilespace and
    //checking for letters and setting the result to Id will probably best be handled by
    //if statements rather then the switch statement.

	if (currentCharacter == ' ')
	{
		result= Whitespace;
	}
	else if (isalpha(currentCharacter))
	{
		validID=true;
		narwhal="";
		narwhal+=currentCharacter;
		result= Id;
	}
	else
	{
		switch(currentCharacter) {
			case ','  : result = Comma; break;
			case '.'  : result = Period; break;
			case ':'  : result = SawColon; break;
			case '?'  : result = QMark; break;
			case '('  : result = LeftParen; break;
			case ')'  : result = RightParen; break;
			case '*'  : result = Multiply; break;
			case '+'  : result = Add; break;
			case '\'' : result = ProcessingString; break; 
			case '\n' : result = Endline; break;
			case '#'  : result = SawHashtag; break;
			case -1   : result = End;break;
			default: 
				result = notHandled;
				
		}
	}
    input->advance();
    return result;
}

void Lex::emit(TokenType tokenType) {

	Token* token = new Token(tokenType, input->getTokensValue(), input->getCurrentTokensLineNumber());
	
    storeToken(token);
    input->mark();
}

void Lex::storeToken(Token* token) {
    //This section shoud ignore whitespace and comments and change the token type to the appropriate value
    //if the value of the token is "Schemes", "Facts", "Rules", or "Queries".

	if (token->getTokenType() == 20 || token->getTokenType() == 18){}
	else
	{
		tokens->push_back(token);
	}


    
}

int main(int argc, char* argv[]) {
    Lex lex(argv[1]);
    cout << lex.toString();
	system("pause");
    return 0;
}