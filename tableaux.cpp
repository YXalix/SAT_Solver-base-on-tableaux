#include "tableaux.hpp"
#include "connectives.hpp"
#include <cstddef>
#include <iterator>
#include <string>
#include <memory>
#include <queue>
#include <stack>
#include <iostream>
#include <utility>

entry::entry(std::list<entry> &tree, bool sign, std::string subformula, std::list<entry>::iterator parent)
: tree(tree), sign(sign), subformula(subformula), parent(parent), left(tree.end()), right(tree.end()), contradictory(false), reduced(false)
{
    reduced = is_propositional_letter(subformula);
    contradictory = is_contradictory() || (parent != tree.end() && parent->contradictory);
}

bool entry::is_leaf(){
    return left == tree.end() && right == tree.end();
}

bool entry::is_contradictory(){
    entry::index f = parent;
    while (f!=tree.end()) {
        if (subformula == f->subformula && sign != f->sign)
            return true;
        f = f->parent;
    }
    return false;
}

void entry::propagate_contradiction(){
    entry::index f = parent;
    while (f != tree.end())
    {
        if (f->left != tree.end() && f->right != tree.end())
        {
            if (f->left->contradictory && f->right->contradictory)
                f->contradictory = true;
            else
                break;
        }
        else if (f->left != tree.end() && f->right == tree.end())
        {
            if (f->left->contradictory)
                f->contradictory = true;
            else
                break;
        }
        else
        {
            break;
        }
        f = f->parent;
    }
}

/**
 * Constructs a new tableau with given formula in its root with given
 * sign.
 */
tableau::tableau(bool sign, std::string formula)
{
    tree.emplace_back(tree, sign, formula, tree.end());
    root = std::prev(tree.end());

    if (!is_propositional_letter(formula))
        to_reduce.push(root);
}

void tableau::reduce()
{
    while (!to_reduce.empty())
    {
        tableau::index e = to_reduce.front();
        to_reduce.pop();
        reduce(e);
    }
}

void tableau::reduce(tableau::index e){
    if (e->reduced || e->contradictory || is_propositional_letter(e->subformula))
    {
        e->reduced = true;
        return;
    }
    size_t conn_index = split_index(e->subformula);
    connective conn = char_to_connective(e->subformula[conn_index]);

    // Split the subformula into two
    size_t a, b, c, d;
    size_t len = e->subformula.length();
    a = 0;
    if (e->subformula[0] == '(')
        ++a;
    b = conn_index - a;
    if (e->subformula[b] == ')')
        --b;
    c = conn_index + 1;
    if (e->subformula[c] == '(')
        ++c;
    d = len - c;
    if (e->subformula[c + d - 1] == ')')
        d--;

    std::string lhs(e->subformula,a,b);
    std::string rhs(e->subformula,c,d);

    // bfs
    std::queue<index> entries;
    entries.push(e);
    while(!entries.empty()){
        index q = entries.front();
        entries.pop();
        if (q->is_leaf() && !q->contradictory)
        {
            append_atomic(q, e->sign, conn, lhs, rhs);

        }
        else
        {
            if (q->left != tree.end() && !q->left->contradictory)
                entries.push(q->left);
            if (q->right != tree.end() && !q->right->contradictory)
                entries.push(q->right);
        }
    }
    e->reduced = true;
}

void tableau::append_atomic(index e, bool sign, connective conn, std::string lhs, std::string rhs){
    switch (conn) {
    case connective::NOT:
        tree.emplace_back(tree, !sign, rhs, e);
        e->left = std::prev(tree.end());
        break;
    case connective::AND:
        if (sign)
        {
            tree.emplace_back(tree, sign, lhs, e);
            e->left = std::prev(tree.end());
            tree.emplace_back(tree, sign, rhs, e->left);
            e->left->left = std::prev(tree.end());
        }
        else
        {
            tree.emplace_back(tree, sign, lhs, e);
            e->left = std::prev(tree.end());
            tree.emplace_back(tree, sign, rhs, e);
            e->right = std::prev(tree.end());
        }
        break;
    case connective::OR:
        tree.emplace_back(tree, sign,lhs,e);
        e->left = std::prev(tree.end());
        tree.emplace_back(tree,sign,rhs,e);
        e->right = std::prev(tree.end());
        break;
    case connective::IF:
        if (sign)
        {
            tree.emplace_back(tree, !sign, lhs, e);
            e->left = std::prev(tree.end());
            tree.emplace_back(tree, sign, rhs, e);
            e->right = std::prev(tree.end());
        }
        else
        {
            tree.emplace_back(tree, !sign, lhs, e);
            e->left = std::prev(tree.end());
            tree.emplace_back(tree, sign, rhs, e->left);
            e->left->left = std::prev(tree.end());
        }
        break;
    case connective::IFF:
        if (sign)
        {
            tree.emplace_back(tree, !sign, lhs, e);
            e->left = std::prev(tree.end());
            tree.emplace_back(tree, !sign, rhs, e->left);
            e->left->left = std::prev(tree.end());
            tree.emplace_back(tree, sign, lhs, e);
            e->right = std::prev(tree.end());
            tree.emplace_back(tree, sign, rhs, e->right);
            e->right->left = std::prev(tree.end());
        }
        else
        {
            tree.emplace_back(tree, !sign, lhs, e);
            e->left = std::prev(tree.end());
            tree.emplace_back(tree, sign, rhs, e->left);
            e->left->left = std::prev(tree.end());
            tree.emplace_back(tree, sign, lhs, e);
            e->right = std::prev(tree.end());
            tree.emplace_back(tree, !sign, rhs, e->right);
            e->right->left = std::prev(tree.end());
        }
        break;
    default:
        break;
    }

    if (e->left != tree.end())
    {
        e->left->propagate_contradiction();
        to_reduce.push(e->left);
        if (e->left->left != tree.end())
        {
            e->left->left->propagate_contradiction();
            to_reduce.push(e->left->left);
        }
    }
    if (e->right != tree.end())
    {
        e->right->propagate_contradiction();
        to_reduce.push(e->right);
        if (e->right->left != tree.end())
        {
            e->right->left->propagate_contradiction();
            to_reduce.push(e->right->left);
        }
    }
}


bool tableau::is_finished()
{
    return to_reduce.empty();
}

bool tableau::is_contradictory()
{
    return root != tree.end() && root->contradictory;
}

void tableau::dot_output(std::ostream &os){
    using namespace std;

    os << "graph tableau" << endl;

    queue<format_item> entries1;
    entries1.push(format_item(root,1));
    cout<<"\t";
    size_t current_level = 1;
    while(!entries1.empty()){
        format_item q = entries1.front();
        entries1.pop();
        if (current_level == q.level) {
            cout<<*(q.id)<<"\t";
        }else {
            cout<<endl<<"\t";
            current_level ++;
            cout<<*(q.id)<<"\t";
        }
        if (q.id->left != tree.end()) {
            entries1.push(format_item(q.id->left,q.level+1));
        }
        if (q.id->right != tree.end()) {
            entries1.push(format_item(q.id->right,q.level+1));
        }
    }
}

tableau::model tableau::get_model(){
    model m;

    if (root->contradictory) return m;

    index f = root;
    while (1) {
        if (is_propositional_letter(f->subformula)) {
            m.emplace(f->subformula,f->sign);
        }
        if (f->left != tree.end() && !f->left->contradictory) f = f->left;
        else if (f->right != tree.end() && !f->right->contradictory) f = f->right;
        else break;
    }
    return m;
}


void tableau::solve(){
    using namespace std;
    reduce();
    cout<<endl;
    dot_output(cout);
    cout <<endl<< (is_contradictory() ? "UNSATISFIABLE" : "SATISFIABLE") << endl;
    for (auto i : get_model())
    {
        cout <<i.first << " = " << i.second <<"\t";
    }
    cout<<endl;
}