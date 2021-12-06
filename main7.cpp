/*
 *  UCF COP3330 Fall 2021 Assignment 6 Solution
 *  Copyright 2021 Ben Phung
 */

#include <iostream>
#include "std_lib_facilities.h"

using namespace std;

struct Token {
	char kind;
	double value;
	string name;
	Token(char ch) :kind(ch), value(0) { }
	Token(char ch, double val) :kind(ch), value(val) { }
  Token(char ch, string n) :kind(ch), name(n) { }
};

class Token_stream {
	bool full;
	Token buffer;
public:
	Token_stream() :full(0), buffer(0) { }

	Token get();
	void unget(Token t) { buffer = t; full = true; }

	void ignore(char);
};

const char let = 'L';
const char quit = 'Q';
const char print = ';';
const char number = '8';
const char name = 'a';
const char sqrts = 's';
const char pows = 'p';
const char isConst = 'C';

Token Token_stream::get()
{
	if (full) { full = false; return buffer; }
	char ch;
	cin >> ch;
	switch (ch) {
	case '(':
	case ')':
	case '+':
	case '-':
	case '*':
	case '/':
	case '%':
	case ';':
	case '=':
		return Token(ch);
	case '.':
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	{	cin.unget();
	double val;
	cin >> val;
	return Token(number, val);
	}
	default:
		if (isalpha(ch) || ch == '_') {
			string s;
			s += ch;
			while (cin.get(ch) && (isalpha(ch) || isdigit(ch) || ch == '_')) s += ch;
			cin.unget();
			if (s == "let") return Token(let);
      if (s == "sqrt") return Token(sqrts);
			if (s == "pow") return Token(pows);
			if (s == "quit") return Token(quit);
      if(s == "const") return Token(isConst);
			return Token(name, s);
		}
		error("Bad token");
	}
}

void Token_stream::ignore(char c)
{
	if (full && c == buffer.kind) {
		full = false;
		return;
	}
	full = false;

	char ch;
	while (cin >> ch)
		if (ch == c) return;
}

struct Variable {
	string name;
	double value;
  bool isConst;
	Variable(string n, double v, bool ic) :name(n), value(v), isConst(ic) { }
};

vector<Variable> names;

double get_value(string s)
{
	for (int i = 0; i < names.size(); ++i)
		if (names[i].name == s) return names[i].value;
	error("get: undefined name ", s);
}

void set_value(string s, double d)
{
	for (int i = 0; i <= names.size(); ++i)
		if (names[i].name == s) {
			names[i].value = d;
			return;
		}
	error("set: undefined name ", s);
}

bool is_declared(string s)
{
	for (int i = 0; i < names.size(); ++i){
        if (names[i].name == s && names[i].isConst == true) 
          error("Cannot reassign const variable");
        else if (names[i].name == s && names[i].isConst == false) 
          return true;
    }
    return false;
}


Token_stream ts;

double expression();

double primary()
{
	Token t = ts.get();
	switch (t.kind) {
	case '(':
	{	double d = expression();
	t = ts.get();
	if (t.kind != ')') error("'(' expected");
	}
	case '-':
		return -primary();
	case number:
		return t.value;
	case name:
		return get_value(t.name);

  case sqrts:
	{
    t = ts.get();
    if (t.kind != '(')
    error("'(' expected");
    double d = expression();
    if (d < 0)
      error("Cannot sqrt negative integers");
    t = ts.get();
    if (t.kind != ')')
      error("')' expected");
    return sqrt(d);
	}
	case pows:
	{
		t = ts.get();
		if (t.kind == '(') {
			double leftval = primary();
			int rightval = 0;
			t = ts.get();
			if(t.kind == ',') rightval = narrow_cast<int>(primary());
			else error("Second argument is not provided");
			double result = 1;
			for(double i = 0; i < rightval; i++) {
				result *= leftval;
			}
			t = ts.get();
			if (t.kind != ')') error("')' expected");
			return result;
		}
		else error("'(' expected");
	}
  
	default:
		error("primary expected");
	}
}

double term()
{
	double left = primary();
	while (true) {
		Token t = ts.get();
		switch (t.kind) {
		case '*':
			left *= primary();
			break;
		case '/':
		{	double d = primary();
		if (d == 0) error("divide by zero");
		left /= d;
		break;
		}
    case '%':
    {
      double d = primary();
      if(d == 0) error("divide by zero");
      left = fmod(left,d);
      break;
    }
		default:
			ts.unget(t);
			return left;
		}
	}
}

double expression()
{
	double left = term();
	while (true) {
		Token t = ts.get();
		switch (t.kind) {
		case '+':
			left += term();
			break;
		case '-':
			left -= term();
			break;
		default:
			ts.unget(t);
			return left;
		}
	}
}

double declaration()
{
	Token t = ts.get();
    bool isC;
    if (t.kind == 'C'){
        isC = true;
        t = ts.get();
    }
    else isC = false;
	if (t.kind != 'a') error("name expected in declaration");
	string name = t.name;
	if (is_declared(name)){
        cout << name + ", declared twice. Enter new value: ";
        cin.clear();
        cin.ignore(10000, '\n');
        int val;
        cin >> val;
        set_value(name, val);
        double d = val;
        return d;

	}
	Token t2 = ts.get();
	if (t2.kind != '=') error("= missing in declaration of ", name);
	double d = expression();
	names.push_back(Variable(name, d, isC));
	return d;
}

double statement()
{
	Token t = ts.get();
	switch (t.kind) {
	case let:
		return declaration();
	default:
		ts.unget(t);
		return expression();
	}
}

void clean_up_mess()
{
	ts.ignore(print);
}

const string prompt = "> ";
const string result = "= ";

void calculate()
{
	while (true) try {
		cout << prompt;
		Token t = ts.get();
		while (t.kind == print) t = ts.get();
		if (t.kind == quit) return;
		ts.unget(t);
		cout << result << statement() << endl;
	}
	catch (runtime_error& e) {
		cerr << e.what() << endl;
		clean_up_mess();
	}
}

int main()

try {
  names.push_back(Variable("k",1000, false));

	calculate();
	return 0;
}
catch (exception& e) {
	cerr << "exception: " << e.what() << endl;
	char c;
	while (cin >> c && c != ';');
	return 1;
}
catch (...) {
	cerr << "exception\n";
	char c;
	while (cin >> c && c != ';');
	return 2;
}