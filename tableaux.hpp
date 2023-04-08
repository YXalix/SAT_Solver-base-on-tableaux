#include "connectives.hpp"
#include <cstddef>
#include <memory>
#include <string>
#include <queue>
#include <map>
#include <list>
#include <iostream>

#ifndef TABLEAU_HPP_INCLUDED
#define TABLEAU_HPP_INCLUDED


class entry{
    public:
    using index = std::list<entry>::iterator;
    std::list<entry> &tree;
    bool sign;
    bool contradictory;
    bool reduced;
    std::string subformula;
    index left;
    index right;
    index parent;

    entry(std::list<entry> &tree, bool sign, std::string subformula, index parent);

    friend std::ostream &operator<<(std::ostream &os, const entry &e)
    {
        return os << (e.sign ? 'T' : 'F') << '(' << e.subformula << ')' << (e.contradictory ? '*' : ' ');
    }

    bool is_leaf();
    bool is_contradictory();
    void propagate_contradiction();
};

class tableau{
    public:
    using index = std::list<entry>::iterator;
    using model = std::map<std::string,bool>;
    std::list<entry>::iterator root;
    std::list<entry> tree;
    std::queue<index> to_reduce;

    tableau(bool sign,std::string formula);

    void reduce();
    void reduce(tableau::index e);
    void append_atomic(index e, bool sign, connective conn, std::string lhs, std::string rhs);
    bool is_finished();
    bool is_contradictory();
    void dot_output(std::ostream &os);
    model get_model();
    void solve();
};

class format_item{
    using index = std::list<entry>::iterator;
    public:
    index id;
    size_t level;
    format_item(index e,size_t e_level):id(e) , level(e_level){}
};


#endif