#include "rtree.hpp"

int main()
{
	R_tree<int, float, 2, float> tree;

	//using point_t = std::array<float, 2>; /*точка n-мерного пространства*/
	//using mbr_t = std::array<point_t, 2>; /*левая нижн. точка и правая верхн. точка mbr*/

	//mbr_t m1, m2;
	//m1[0] = { 1,2 };
	//m1[1] = { 4,4 };
	//m2[0] = { 2,1 };
	//m2[1] = { 4,3 };
	//float size1{ std::abs(m1[0][0] - m1[1][0]) }, size2{ std::abs(m2[0][0] - m2[1][0]) };
	//for (size_t i = 1; i < 2; i++)
	//{
	//	size1 *= std::abs(m1[0][i] - m1[1][i]);
	//	size2 *= std::abs(m2[0][i] - m2[1][i]);
	//}
	//float s = std::abs(size1 - size2);
	//std::cout << "s:" << s << std::endl;

	return 0;
}