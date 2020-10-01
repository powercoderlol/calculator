#include <cassert>
#include <cmath>
#include <deque>
#include <iostream>
#include <iterator>
#include <map>
#include <queue>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <vector>

using namespace std;

map<char, uint8_t> ops_support{
    {'+', 1}, {'-', 1}, {'*', 1}, {'/', 1}, {'^', 1}, {'%', 1},
};

map<string, uint8_t> ops_priority{
    {"+", 1}, {"-", 1}, {"*", 2},  {"/", 2},
    {"^", 2}, {"%", 2}, {"(", 0}, {")", 0},
};

map<string, double (*)(double, double)> ops{
    {"+", [](double a, double b) { return a + b; }},
    {"-", [](double a, double b) { return a - b; }},
    {"-", [](double a, double b) { return a - b; }},
    {"*", [](double a, double b) { return a * b; }},
    {"/", [](double a, double b) { return a / b; }},
    {"^", [](double a, double b) { return pow(a, b); }},
    {"%", [](double a, double b) { return fmod(a, b); }},
};

bool is_digit(char c) {
    if(c == '.')
        return true;
    return isdigit(c);
}

bool is_ops(char c) {
    return ops_support.find(c) != ops_support.end();
}

bool is_parenthesis(char c) {
    return (c == ')') || (c == '(');
}

struct parsing_state {
    bool not_an_expression = true;
    bool number_start = false;
    bool prev_op = false;
    bool prev_open = false;
    bool prev_close = false;
} state_;

bool is_prev_op() {
    return state_.prev_op;
}
bool is_nae() {
    return state_.not_an_expression;
}
bool is_prev_number() {
    return state_.number_start;
}
bool is_prev_open() {
    return state_.prev_open;
}
bool is_prev_close() {
    return state_.prev_close;
}
void set_prev_op(bool b = true) {
    state_.prev_op = b;
}
void set_nae(bool b = true) {
    state_.not_an_expression = b;
}
void set_prev_number(bool b = true) {
    state_.number_start = b;
}
void set_prev_open(bool b = true) {
    state_.prev_open = b;
}
void set_prev_close(bool b = true) {
    state_.prev_close = b;
}
void reset_state() {
    state_.not_an_expression = true;
    state_.number_start = false;
    state_.prev_op = false;
    state_.prev_open = false;
    state_.prev_close = false;
}

std::vector<string> validate_string(string& expression) {
    std::stack<char> pars;
    std::vector<string> result;
    if(expression.empty()) {
        return {};
    }
    try {
        std::string number;
        for(int pos = 0; pos < expression.size(); ++pos) {
            char c = expression[pos];
            if(is_digit(c)) {
                if(is_prev_close())
                    throw invalid_argument(
                        string{"unexpected symbol: "} + to_string(pos));
                if(is_prev_open())
                    set_prev_open(false);
                set_nae(false);
                set_prev_op(false);
                number += c;
                if(!is_prev_number())
                    set_prev_number();
            }
            else if(is_ops(c)) {
                if(is_prev_open())
                    set_prev_open(false);
                if(is_prev_close())
                    set_prev_close(false);
                set_nae(false);
                if(is_prev_number()) {
                    result.push_back(number);
                    number.clear();
                    set_prev_number(false);
                }
                if(result.empty())
                    throw invalid_argument(
                        string{"unexpected operator: "} + to_string(pos));
                if(is_prev_op())
                    throw invalid_argument(
                        string{"unexpected symbol: "} + to_string(pos));
                set_prev_op();
                result.push_back(string{c});
            }
            else if(is_parenthesis(c)) {
                if(c == '(') {
                    set_prev_open();
                    pars.push(c);
                    if(is_prev_number())
                        throw invalid_argument(
                            string{"unexpected symbol: "} + to_string(pos));
                }
                if(c == ')') {
                    set_prev_close();
                    if(is_prev_open())
                        throw invalid_argument(
                            string{"unexpected symbol: "} + to_string(pos));
                    if(is_prev_op())
                        throw invalid_argument(
                            string{"unexpected operator: "} + to_string(pos));
                    if(pars.empty())
                        throw invalid_argument(
                            string{"unexpected symbol: "} + to_string(pos));
                    pars.pop();
                }
                if(is_prev_number()) {
                    result.push_back(number);
                    number.clear();
                    set_prev_number(false);
                }
                result.push_back(string{c});
            }
            else {
                throw invalid_argument(
                    string{"unexpected symbol: "} + to_string(pos));
            }
        }
        if(is_prev_number())
            result.push_back(number);
        if(is_nae())
            throw runtime_error(string{"Not an expression"});
        if(is_prev_op())
            throw invalid_argument(
                string{"unexpected operator in the end of expression"});
        if(pars.size() > 0)
            throw invalid_argument(string{"unclosed parentheses"});
    }
    catch(const invalid_argument& e) {
        throw invalid_argument(e.what());
    }
    reset_state();
    return result;
}

template<class Iterator>
deque<string> convert_to_rpn(Iterator it, Iterator end) {
    deque<string> output_queue;
    stack<string> func_stack;

    for(; it != end; ++it) {
        if(auto op_iter = ops_priority.find(*it);
           op_iter == ops_priority.end()) {
            output_queue.push_back(*it);
        }
        else if(*it == "(")
            func_stack.push(*it);
        else if(*it == ")") {
            try {
                bool par_missed = true;
                while(!func_stack.empty()) {
                    auto top_op = func_stack.top();
                    if(top_op == "(") {
                        par_missed = false;
                        func_stack.pop();
                        break;
                    }
                    output_queue.push_back(top_op);
                    func_stack.pop();
                }
                if(par_missed)
                    throw runtime_error("Parentheses missed.");
            }
            catch(const runtime_error& e) {
                throw runtime_error(e.what());
            }
        }
        else {
            try {
                if(func_stack.empty()) {
                    func_stack.push(*it);
                    continue;
                }
                auto new_priority = op_iter->second;
                while(!func_stack.empty()) {
                    auto top_op = func_stack.top();
                    if(is_parenthesis(top_op[0]))
                        break;
                    auto top_priority = ops_priority.at(top_op);
                    if(top_priority < new_priority)
                        break;
                    output_queue.push_back(top_op);
                    func_stack.pop();
                }
                func_stack.push(*it);
            }
            catch(const out_of_range&) {
                throw invalid_argument(*it);
            }
        }
    }
    while(!func_stack.empty()) {
        auto func = func_stack.top();
        output_queue.push_back(func);
        func_stack.pop();
    }
    return output_queue;
}

template<typename Iterator>
double evaluate_rpn(Iterator it, Iterator end) {
    stack<double> val_stack;

    auto pop_stack([&]() {
        auto r(val_stack.top());
        val_stack.pop();
        return r;
    });

    for(; it != end; ++it) {
        stringstream ss{*it};
        if(double val; ss >> val) {
            val_stack.push(val);
        }
        else {
            const auto r{pop_stack()};
            const auto l{pop_stack()};
            try {
                const auto& op(ops.at(*it));
                const double result{op(l, r)};
                val_stack.push(result);
            }
            catch(const out_of_range&) {
                throw invalid_argument(*it);
            }
        }
    }
    return val_stack.top();
}

void clean_expression(std::string& expression) {
    expression.erase(
        std::remove_if(begin(expression), end(expression), ::isspace),
        end(expression));
    expression.erase(
        std::remove(begin(expression), end(expression), '\"'), end(expression));
}

double evaluate(std::string& expression) {
    clean_expression(expression);
    auto parsed_expression = validate_string(expression);
    auto postfix_evaluation =
        convert_to_rpn(parsed_expression.begin(), parsed_expression.end());
    return evaluate_rpn(postfix_evaluation.begin(), postfix_evaluation.end());
}

int main(int argc, char* argv[]) {
    string expression;
    try {
        if(argc >= 2) {
            expression.assign(argv[1]);
            for(int i = 2; i < argc; ++i) {
                expression.append(argv[i]);
            }
            cout << "= " << evaluate(expression) << '\n';
            expression.clear();
        }
        while(getline(cin, expression))
            cout << "= " << evaluate(expression) << '\n';
    }
    catch(const invalid_argument& e) {
        cout << "Invalid argument: " << e.what() << '\n';
    }
    catch(const runtime_error& e) {
        cout << "Runtime error: " << e.what() << '\n';
    }
    catch(const exception& e) {
        cout << "Std exception: " << e.what() << '\n';
    }
    return 0;
}
