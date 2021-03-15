#define __CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW

#include "rtree.hpp"

#include <iostream>

int main()
{
	{
		system("chcp 65001>nul");
		using Tree = R_tree<int, float, 2, float, 4, 2>;

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
		for (size_t i = 20; i <= 100; i++)
		{
			points.push_back(Tree::mbr_t{ { float(i) * (i + 1) + 1, float(i) * (i + 1) + 1 } , { float(i) * (i + 2),float(i) * (i + 2) } });
		}
		points[0].ru = { 5,5 };

		for (size_t i = 0; i < points.size(); i++)
		{
			tree.insert(i + 1, points[i]);
		}

		tree.print();

		for (size_t i = 0; i < points.size(); i++)
		{
			bool success{ true };

			int key{ tree.find(points[i], success) };

			if (success)
			{
				std::cout << "Элемент " << i + 1 << " найден" << std::endl;
			}
			else
			{
				std::cout << "Элемент " << i + 1 << " не найден" << std::endl;
			}
		}
		for (size_t i = 0; i < points.size(); i++)
		{
			tree.remove(points[i], i + 1);
		}
		tree.print();

	}

	_CrtDumpMemoryLeaks(); /*показывает утечки памяти, если они есть*/
	return 0;
}