#include <iostream>
#include "tableaux.hpp"
#include "connectives.hpp"

using namespace std;
int main(int, char**) {

    string formula = "(p | q) & (q | r) & (~p | ~r)";
    remove_whitespce(formula);
    tableau t(true,formula);
    t.solve();
    return 0;
}
