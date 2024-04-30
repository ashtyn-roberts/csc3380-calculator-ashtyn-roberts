#include <iostream>
#include <stdexcept>
#include <string>
#include <map>
#include <cmath>
#include <cctype>

std::map<std::string, double> table;

void init_table(){
    table["pi"] = 3.1415926535897932385;
    table["e"] = 2.7182818284590452354;
    table["isqrd"] = -1;
    table["sqrt2"] = 1.4142135623;
    table["sqrt3"] = 1.7320508075;
    table["tau"] = 6.2831853071795864769;
    table["phi"] = 1.61803398874989484820;
    table["fibonacci"] = 3.359885666243178;
}
// Token stuff
struct token
{
    char kind;       // what kind of token
    double value;    // for numbers: a value
    std::string name;

    token(char ch) : kind(ch), value(0){}
    token(char ch, double val) : kind(ch), value(val){}
    token(char ch, std::string n) : kind(ch), name(n){}
};

class token_stream
{
    // representation: not directly accessible to users:
    bool full;       // is there a token in the buffer?
    token buffer;    // here is where we keep a Token put back using
                     // putback()
public:
    // user interface:
    token get();            // get a token
    void putback(token);    // put a token back into the token_stream

    // constructor: make a token_stream, the buffer starts empty
    token_stream()
      : full(false)
      , buffer(0)
    {
    }
};

// single global instance of the token_stream
token_stream ts;

void token_stream::putback(token t)
{
    if (full)
        throw std::runtime_error("putback() into a full buffer");
    buffer = t;
    full = true;
}

token token_stream::get() {
    if (full) {
        full = false;
        return buffer;
    }

    char ch;
    std::cin >> std::ws;  // Ignore leading whitespace
    if (!(std::cin >> ch)) {
        return token(';');  // Handle end-of-input as end of statement
    }

    switch (ch) {
    case '(': case ')': case ';': case '+': case '-': case '*': case '/': case '%':
        return token(ch);
    case '.':
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
        std::cin.putback(ch);
        double val;
        std::cin >> val;
        return token('8', val);  // '8' for numbers
    case 'q':  // Single 'q' treated as quit command
        return token('q');
    case '=':
        return token('=');
    default:
        if (isalpha(ch)) {
            std::string s;
            s += ch;
            while (std::cin.get(ch) && (isalnum(ch) || ch == '_')) {s += ch;}
            std::cin.putback(ch);
             if (s == "q") {
                return token('q'); // Treat 'q' as quit if standalone
            }
            return token('a', s);  // 'a' for identifiers
        }
        throw std::runtime_error("Bad token");
    }
}


// declaration so that primary() can call expression()
double expression();

double primary() {
    token t = ts.get();
    switch (t.kind) {
    case '(':
        {
            double d = expression();
            t = ts.get();
            if (t.kind != ')') throw std::runtime_error("')' expected");
            return d;
        }
    case '-':
        return -primary();
    case '+':
        return primary();
    case '8':
        return t.value;
    case 'a':
        {
            std::string var_name = t.name;
            if (var_name == "q") throw std::runtime_error("Unexpected use of quit command 'q'");
            token next_t = ts.get();
            if (next_t.kind == '=') {
                double value = expression();
                table[var_name] = value;  // Assign value to the variable
                return value;
            } else {
                ts.putback(next_t);
                if (table.find(var_name) == table.end()) throw std::runtime_error(var_name + " not defined");
                return table[var_name];  // Retrieve the variable's value
            }
        }
    default:
        throw std::runtime_error("Primary expected");
    }
}


// exactly like expression(), but for * and /
double term()
{
    double left = primary();    // get the Primary
    while (true)
    {
        token t = ts.get();     // get the next Token ...
        switch (t.kind)
        {
        case '*':
            left *= primary();
            break;
        case '/':
        {
            double d = primary();
            if (d == 0)
                throw std::runtime_error("divide by zero");
            left /= d;
            break;
        }
        case '%':{
            double d = primary();
            if(d == 0)
                throw std::runtime_error("divide by zero");
            int i1 = static_cast<int>(left);
            int i2 = static_cast<int>(d);
            if (i2 ==0)
                throw std::runtime_error("modulo by zero");
            left = i1 % i2;
            break;
        }
        default:
            ts.putback(t);    // <<< put the unused token back
            return left;      // return the value
        }
    }
}

// read and evaluate: 1   1+2.5   1+2+3.14  etc.
// 	 return the sum (or difference)
double expression()
{
    double left = term();      // get the Term
    while (true)
    {
        token t = ts.get();    // get the next token ...
        switch (t.kind)        // ... and do the right thing with it
        {
        case '+':
            left += term();
            break;
        case '-':
            left -= term();
            break;
        default:
            ts.putback(t);    // <<< put the unused token back
            return left;      // return the value of the expression
        }
    }
}

int main() {
    try {
        init_table();  // Initialize constants in the table
        std::cout << "Enter equations to calculate. for mathematical constants pi, e, phi, tau, isqrd, fibonacci, sqrt2, sqrt3 type as listed here, type 'q' to quit. Each equation must end with ';'\n";

        double val = 0;
        while (true) {
            std::cout << "> ";
            token t = ts.get();
            if (t.kind == 'q') {
                std::cout << "Quitting program.\n";
                break;
            }
            if (t.kind == ';') {
                std::cout << "= " << val << '\n';
                val = 0;
                continue;
            }
            ts.putback(t);
            val = expression();
        }
    } catch (std::runtime_error& e) {
        std::cerr << "Runtime error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "An unexpected error occurred.\n";
        return 2;
    }
    return 0;
}
