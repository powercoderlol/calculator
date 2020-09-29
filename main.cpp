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

template<class Iterator>
deque<string> convert_to_rpn(Iterator it, Iterator end) {
    deque<string> output_queue;
    stack<string> func_stack;

    map<string, uint8_t> ops{
        {"+", 0}, {"-", 0}, {"*", 1},  {"/", 1},
        {"^", 1}, {"%", 1}, {"(", -1}, {")", -1},
    };

    for(; it != end; ++it) {
        if(auto op_iter = ops.find(*it); op_iter == ops.end()) {
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
                auto new_priority = ops.at(*it);
                while(!func_stack.empty()) {
                    auto top_op = func_stack.top();
                    if(top_op == ")" || top_op == "(")
                        break;
                    auto top_priority = ops.at(top_op);
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

    map<string, double (*)(double, double)> ops{
        {"+", [](double a, double b) { return a + b; }},
        {"-", [](double a, double b) { return a - b; }},
        {"-", [](double a, double b) { return a - b; }},
        {"*", [](double a, double b) { return a * b; }},
        {"/", [](double a, double b) { return a / b; }},
        {"^", [](double a, double b) { return pow(a, b); }},
        {"%", [](double a, double b) { return fmod(a, b); }},
    };

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

int main() {
    try {
        stringstream str{"( 3 * ( 1 + 2 ) ) / 2"};
        auto postfix_evaluation =
            convert_to_rpn(istream_iterator<string>{str}, {});
        cout << evaluate_rpn(
            postfix_evaluation.begin(), postfix_evaluation.end())
             << '\n';
    }
    catch(const invalid_argument& e) {
        cout << "Invalid operator: " << e.what() << '\n';
    }
    catch(const runtime_error& e) {
        cout << "Runtime error: " << e.what() << '\n';
    }
    return 0;
}


