#include "Lex.cpp"
#include <iostream>

using namespace std;


int main(int argc, char* argv[]) {
    Lex lex(argv[1]);
    cout << lex.toString();
	cin.ignore();
    return 0;
}
