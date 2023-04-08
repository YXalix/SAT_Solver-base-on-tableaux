# SAT_Solver-base-on-tableaux

涉及到的数据结构包括
- list ：用于存储树的数据
- 以及list iterator ： 用于构建树结构
- queue ：用于存储尚待化简的命题，或者通过层次遍历找到该命题下面的所有叶子结点，添加该命题的化简节点到所有叶子结点上
