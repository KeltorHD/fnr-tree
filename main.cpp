#include "rtree.hpp"

#include <iostream>

int main()
{
	system("chcp 65001>nul");
	using Tree = R_tree<int, float, 2, float, 4, 2>;

	Tree tree;

	tree.insert(1, Tree::mbr_t{ {1,1},{2,2} });
	tree.print();
	tree.insert(2, Tree::mbr_t{ {3,3},{4,4} });
	tree.print();
	tree.insert(3, Tree::mbr_t{ {5,5},{6,6} });
	tree.print();
	tree.insert(4, Tree::mbr_t{ {7,7},{8,8} });
	tree.print();
	tree.insert(5, Tree::mbr_t{ {9,9},{10,10} });
	tree.print();
	tree.insert(6, Tree::mbr_t{ {11,11},{12,12} });
	tree.print();
	tree.insert(7, Tree::mbr_t{ {13,13},{14,14} });
	tree.print();
	tree.insert(8, Tree::mbr_t{ {15,15},{16,16} });
	tree.print();
	tree.insert(9, Tree::mbr_t{ {17,17},{18,18} });
	tree.print();
	
	tree.print();

	return 0;
}