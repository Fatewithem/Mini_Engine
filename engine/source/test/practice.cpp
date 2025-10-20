#include <iostream>

using namespace std;

template<typename T>
struct Stack {
    vector<T> a;

    void push(const T& val) { a.push_back(val); }
    void push(T&& val) { a.push_back( std::move(val)); }

    void pop(){ a.pop_back(); }
    T& top(){ return a.back(); }
    
    bool empty() const{ return a.empty(); }
    std::size_t size() const{ return a.size(); }
};