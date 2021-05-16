#define __CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW

#include "fnrtree.hpp"

#include <iostream>
#include <iomanip>
#include <map>
#include <fstream>
#include <string>
#include <sstream>
#include <chrono>

void readNodes(const char* filename, std::map<long, std::pair<int, int> >* m) 
{
	std::ifstream infile(filename);
	std::string line{};
	while (std::getline(infile, line)) 
	{
		std::istringstream iss(line);
		long id; int x, y;
		if (!(iss >> id >> x >> y)) break;
		m->insert(std::make_pair(id, std::make_pair(x, y)));
		//std::cout << "id node: " << id << ", x: " << x << ", y: " << y << std::endl;
	}
}

void readEdges(const char* filename, std::map<long, std::pair<int, int> >* nodes, FNR_tree<long>* tree)
{
	std::ifstream infile(filename);
	std::string line;
	while (std::getline(infile, line)) 
	{
		std::istringstream iss(line);
		long id, A, B; std::string name;
		if (!(iss >> id >> A >> B >> name)) break;

		//std::cout << "id edge: " << id << ", A-id: " << A << ", B-id: " << B << ", name" << name << std::endl;

		std::pair<int, int> Acoord, Bcoord;
		Acoord = nodes->find(A)->second;
		//std::cout << "A: " << Acoord.first << ", " << Acoord.second << std::endl;
		Bcoord = nodes->find(B)->second;
		//std::cout << "B: " << Bcoord.first << ", " << Bcoord.second << std::endl;
		tree->insert_line(Acoord.first, Acoord.second, Bcoord.first, Bcoord.second, name);
	}
}

void readTrajectories(const char* filename, FNR_tree<long>* tree)
{
	std::map<long, std::pair<double, std::pair<int, int>>> Objects; // id -> (time, (x,y) )

	std::ifstream infile(filename);
	std::string line;
	if (!infile.is_open())
	{
		std::cout << "not open file";
	}

	while (std::getline(infile, line)) 
	{
		std::istringstream iss(line);
		char cls; long id; double time;
		int currX, currY;
		if (!(iss >> cls >> id >> time >> currX >> currY)) 
			break;

		if (cls == '0') /*начальные данные*/
		{
			Objects[id] = std::make_pair(time, std::make_pair(currX, currY));
		}
		else 
		{
			std::pair<double, std::pair<int, int> > lastPos = Objects[id];
			double t0 = lastPos.first;
			int x0 = lastPos.second.first;
			int y0 = lastPos.second.second;
			tree->insert_trip_segment(id, x0, y0, currX, currY, t0, time);
			Objects[id] = std::make_pair(time, std::make_pair(currX, currY));

		}
	}
}

void readQueries(const char* inFilename, const char* outFilename, FNR_tree<long>* tree)
{
	std::set<long>* resArray = new std::set<long>;
	std::ifstream infile(inFilename);
	std::ofstream outfile(outFilename);
	std::string line;
	int cont = 1;

	std::chrono::microseconds duration(0);

	while (std::getline(infile, line)) 
	{
		std::istringstream iss(line);
		int x1, y1, x2, y2;
		double t1, t2;
		if (!(iss >> x1 >> y1 >> x2 >> y2 >> t1 >> t2)) break;

		std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
		tree->search(x1, y1, x2, y2, t1, t2, resArray);
		std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();

		duration += std::chrono::duration_cast<std::chrono::microseconds>(end - start);

		outfile << "Test #" << cont++ << std::endl;
		for (auto it = resArray->begin(); it != resArray->end(); ++it) 
		{
			outfile << *it << " ";
		}
		outfile << std::endl << std::endl;
	}

	std::cout << "   > Queries time  = " << std::right << std::setw(10);
	std::cout << duration.count();
	std::cout << " microseconds" << std::endl;
	outfile.close();
}

int main(int argc, char* argv[])
{
	system("chcp 65001>nul");
	/*{
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
				std::std::cout << "Элемент " << i + 1 << " найден" << std::endl;
			}
			else
			{
				std::std::cout << "Элемент " << i + 1 << " не найден" << std::endl;
			}
		}
		for (size_t i = 0; i < points.size(); i++)
		{
			tree.remove(points[i], i + 1);
		}
		tree.print();

	}*/

	//{
	//	R_tree<int, int, 1, float, 4, 2> time;
	//
	//	time.print();
	//
	//	time.insert(1, { 1,3 });
	//	time.insert(2, { 1,3 });
	//	time.insert(3, { 1,3 });
	//	time.insert(4, { 1,3 });
	//	time.insert(5, { 1,3 });
	//	time.insert(6, { 1,3 });
	//	time.insert(7, { 1,3 });
	//	time.insert(8, { 1,3 });
	//	time.insert(9, { 1,3 });
	//	time.insert(10, { 1,3 });
	//	time.insert(11, { 1,3 });
	//
	//	time.print();
	//}

	/*{
		FNR_tree<int> tree;
	}*/

	{
		if (argc != 6)
		{
			std::cout << "Usage: ./fnr-tree.exe [nodesFile] [edgesFile] [trajectoriesFile] [queriesFile] [outFile]" << std::endl;
			return -1;
		}

		const char* nodesFile = argv[1];
		const char* edgesFile = argv[2];
		const char* trajectoriesFile = argv[3];
		const char* queriesFile = argv[4];
		const char* outFile = argv[5];

		FNR_tree<long> kk;

		auto Nodes = new std::map<long, std::pair<int, int> >();
		//chrono::high_resolution_clock::time_point start = chrono::high_resolution_clock::now();
		readNodes(nodesFile, Nodes);
		readEdges(edgesFile, Nodes, &kk);

		kk.print();

		auto start2 = std::chrono::high_resolution_clock::now();
		readTrajectories(trajectoriesFile, &kk);

		kk.print();

		auto end = std::chrono::high_resolution_clock::now();

		std::cout << "> FNR-Tree indicators:" << std::endl;
		std::cout << "   > MEMORY USAGE  = " << std::right << std::setw(10);
		std::cout << "{NOT REALIZED}";
		std::cout << " Bytes" << std::endl;
		std::cout << "   > Building time = " << std::right << std::setw(10);
		std::cout << std::chrono::duration_cast<std::chrono::microseconds>(end - start2).count();
		std::cout << " microseconds" << std::endl;

		//readQueries(queriesFile, outFile, &kk);
	}


	_CrtDumpMemoryLeaks(); /*показывает утечки памяти, если они есть*/
	return 0;
}