#include "rtree.hpp"

#include <iostream>

int main()
{
	system("chcp 65001>nul");
	using Tree = R_tree<int, float, 2, float, 4, 2>;

	std::variant<int, std::string> var{ 5 };
	int& i{ std::get<int>(var) };
	i = 10;
	std::cout << std::get<int>(var) << " " << i << std::endl;


	Tree tree;

	std::vector<Tree::mbr_t> points{};
	for (size_t i = 1; i <= 14; i++)
	{
		points.push_back(Tree::mbr_t{ { float(i) * (i + 1) + 1, float(i) * (i + 1) + 1 } , { float(i) * (i + 2),float(i) * (i + 2) } });
	}
	for (size_t i = 15; i <= 17; i++)
	{
		points.push_back(Tree::mbr_t{ { -(float(i) * (i + 1) + 1), -(float(i) * (i + 1) + 1) } , { -(float(i) * (i + 2)), -(float(i) * (i + 2)) } });
	}
	for (size_t i = 18; i <= 19; i++)
	{
		points.push_back(Tree::mbr_t{ { float(i) * (i + 1) + 1, float(i) * (i + 1) + 1 } , { float(i) * (i + 2),float(i) * (i + 2) } });
	}
	/*for (size_t i = 20; i <= 50; i++)
	{
		points.push_back(Tree::mbr_t{ { float(i) * (i + 1) + 1, float(i) * (i + 1) + 1 } , { float(i) * (i + 2),float(i) * (i + 2) } });
	}*/
	points[0].ru = { 5,5 };

	for (size_t i = 0; i < points.size(); i++)
	{
		tree.insert(i + 1, points[i]);
		tree.print();
	}

	return 0;
}