# C++11 FNR-Tree implementations.

Implementation of a fixed network tree using the actually implemented R-Tree

---
# R-tree

R-Tree is a tree-like data structure proposed in 1984 by Antonin Guttman

R-Tree template:  
```
#define R_template template<typename data_type, typename coord_type, size_t num_dims, typename float_type, size_t max_nodes, size_t min_nodes>
R_template class R_tree;
```

R-Tree example:
```
using Tree = R_tree<int, float, 2, float, 4, 2>;
Tree tree;
std::vector<Tree::mbr_t> points{};
for (size_t i = 1; i <= 14; i++)
{
  points.push_back(Tree::mbr_t{ { float(i) * (i + 1) + 1, float(i) * (i + 1) + 1 } , { float(i) * (i + 2),float(i) * (i + 2) } });
}
for (size_t i = 0; i < points.size(); i++)
{
  tree.insert(i + 1, points[i]);
}
tree.print();
for (size_t i = 0; i < points.size(); i++)
{
  tree.remove(points[i], i + 1);
}
tree.print();
```

---

# FNR-Tree

Fixed Network R-Tree (FNR-Tree) is an indexing method for objects that move along fixed networks in 2-dimensional space. The general idea behind an FNR tree is a forest of one-dimensional (1D) R-trees on top of a two-dimensional (2D) R-tree. 2D R-Trees are used to index the spatial data of the network (for example, roads consisting of line segments), while 1D R-Trees are used to index the time interval of movement of each object within a given link in the network. A performance study comparing this new access method with a traditional R-Tree across different datasets and queries shows that FNR-Tree outperforms R-Tree in most cases.

FNR-Tree template:  
```
template <typename object_t>
class FNR_tree;
```

FNR-Tree example:  
```
FNR_tree<long> kk;
kk.insert_line(0, 1, 2, 3, "a");
kk.insert_trip_segment(1, 0, 1, 2, 3, 2, 4);
std::set<long> resArray;
kk.search(0, 1, 2, 3, 2, 4, &resArray);
```
