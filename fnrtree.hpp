#pragma once

#include "rtree.hpp"

#include "line.hpp"
#include "interval.hpp"

#include <iostream>
#include <string>
#include <set>

#define DEBUG

template <typename object_t>
class FNR_tree
{
private:
	class Spatial_leaf;
	class Temporal_leaf;
	using spatial_t = R_tree<std::shared_ptr<Spatial_leaf>, int, 2, float, 8, 4>;
	using spatial_level_t = std::shared_ptr<spatial_t>;

public:
	FNR_tree()
	{
		this->spatial_level = std::make_shared<spatial_t>();
	}
	~FNR_tree() = default;

	FNR_tree(const FNR_tree&) = delete;  /*конструкторы и операторы копирования и переноса удалены*/
	FNR_tree(FNR_tree&&) = delete;
	FNR_tree operator=(const FNR_tree&) = delete;
	FNR_tree operator=(FNR_tree&&) = delete;

	/*сохраняет временные интервалы*/
	class Temporal_leaf
	{
	public:
		Temporal_leaf() = default;
		~Temporal_leaf() = default;
		Temporal_leaf(Interval in, object_t id, bool dir)
		{
			this->interval = in;
			this->object_id = id;
			this->movement_direction = dir;
		}
		void print() const
		{
			std::cout << "id: " << this->object_id << ", dir: " << this->movement_direction << ". Time interval: [" << interval.time_in << ", " << interval.time_out << "]" << std::endl;
		}
		const object_t& get_id() const
		{
			return this->object_id;
		}
		const Interval& get_interval() const
		{
			return this->interval;
		}
		size_t size() const
		{
			return sizeof(Temporal_leaf);
		}
	private:
		object_t object_id;
		bool movement_direction;
		Interval interval;
	};

	/*сохраняет пространственную структуру*/
	class Spatial_leaf
	{
	public:
		using temporal_t = R_tree<std::shared_ptr<Temporal_leaf>, double, 1, float>;
		using temporal_ptr_t = std::shared_ptr<temporal_t>;

		Spatial_leaf() = default;
		~Spatial_leaf() = default;
		Spatial_leaf(Line l, bool ori, std::string nn)
		{
			this->line = l;
			this->orientation = ori;
			this->nnn = nn;
			this->temporal_tree = std::make_shared<temporal_t>();
		}
		void print(size_t level) const
		{
			this->temporal_tree->print(level, [](size_t level, void* data)
				{
					const std::shared_ptr<Temporal_leaf>& tl = *((std::shared_ptr<Temporal_leaf>*)data);
					tl->print();
				});
		}
		const Line& get_line() const
		{
			return this->line;
		}
		const std::string& get_name() const
		{
			return this->nnn;
		}
		bool get_orientation() const
		{
			return this->orientation;
		}
		const temporal_ptr_t& get_temporal_tree() const
		{
			return this->temporal_tree;
		}
		size_t size() const
		{
			size_t self{ sizeof(Spatial_leaf) };

			return self + this->temporal_tree->size([](const std::shared_ptr<Temporal_leaf>& leaf)
				{
					return leaf->size();
				});
		}
	private:
		bool orientation;
		temporal_ptr_t temporal_tree;
		Line line;
		std::string nnn;

	};

	/*структуры передаваемых аргументов*/
	struct Insert_interval_args
	{
	public:
		object_t object_id;
		Line line;
		Interval time_interval;
		bool orientation;

		Insert_interval_args() = default;
		~Insert_interval_args() = default;
		Insert_interval_args(object_t id, Line l, Interval i, bool o)
		{
			object_id = id;
			line = l;
			time_interval = i;
			orientation = o;
		};

	};
	struct Search_args
	{
	public:
		Line s_window;
		Interval t_window;
		std::set<object_t>* result_array;
		std::shared_ptr<Spatial_leaf> lf;

		Search_args() = default;
		~Search_args() = default;
		Search_args(Line l, Interval i, std::set<object_t>* r)
		{
			s_window = l;
			t_window = i;
			result_array = r;
		};
	};

	/*печать дерева в консоль*/
	void print() const
	{
		this->spatial_level->print(0, [](size_t level, void* data)
			{
				const std::shared_ptr<Spatial_leaf>& tree = *((std::shared_ptr<Spatial_leaf>*)data);
				std::cout << "Название дороги: " << tree->get_name() << ", ";
				tree->print(level);
			}
		);
	}

	/*
	Вставка именованного отрезка в дерево
	Аргументы:
	-Начало отрезка
	-Конец отрезка
	-Время выхода
	-Время прихода
	*/
	void insert_line(int x1, int y1, int x2, int y2, std::string name)
	{
#ifdef DEBUG
		std::cout << "> BEGIN InsertLine name: " + name + ":" << std::endl;
#endif // DEBUG

		bool ori = !((x2 - x1) * (y2 - y1) >= 0); // 0 -> / , 1 -> \ .
		Line tmpLine(std::min(x1, x2), std::min(y1, y2), std::max(x1, x2), std::max(y1, y2));

#ifdef DEBUG
		std::cout << "\t> Inserting.. (" << tmpLine.min[0] << "," << tmpLine.min[1] << ")->(" << tmpLine.max[0] << "," << tmpLine.max[1] << ")" << std::endl;
#endif // DEBUG

		auto tmp_leaf = std::make_shared<Spatial_leaf>(tmpLine, ori, name);
		spatial_level->insert(tmp_leaf, { tmpLine.min, tmpLine.max });
#ifdef DEBUG
		std::cout << "> END   InsertLine." << std::endl;
#endif // DEBUG
	}

	/*вставка временного интервала, вызывается в r-tree, передается: куда и что (Insert_interval_args)*/
	static bool insert_time_interval(std::shared_ptr<Spatial_leaf> id, void* arg)
	{
#ifdef DEBUG
		std::cout << "\t> TRYING InsertTimeInterval..." << std::endl;
#endif // DEBUG

		Insert_interval_args* args = (Insert_interval_args*)arg;
		object_t object_id = args->object_id;
		Line targetLine = args->line;
		Line line = id->get_line();
		bool orientation = args->orientation;

		if (!targetLine.equals(line)) 
			return true;

#ifdef DEBUG
		std::cout << "\t> BEGIN InsertTimeInterval." << std::endl;
#endif // DEBUG

		Interval tmpInterval = args->time_interval;

#ifdef DEBUG
		std::string arrow = orientation ? "<--" : "-->";
		std::cout << "\t\t> Inserting.. id: " << object_id << " interval[" << tmpInterval.time_in << "," << tmpInterval.time_out << "] " << arrow << " into " << id->get_name() << std::endl;
#endif // DEBUG

		
		std::shared_ptr<Temporal_leaf> tmpLeaf = std::make_shared<Temporal_leaf>(tmpInterval, object_id, orientation);
		id->get_temporal_tree()->insert(tmpLeaf, { tmpInterval.time_in, tmpInterval.time_out });

#ifdef DEBUG
		std::cout << "\t> END   InsertTimeInterval." << std::endl;
#endif // DEBUG

		return false;
	}

	/*
	Вставка перемещения в дерево
	Аргументы:
	-Перемещающийся объект
	-Из какой точки
	-В какую точку
	-Время выхода
	-Время прихода
	*/
	void insert_trip_segment(object_t object_id, int x1, int y1, int x2, int y2, double entrance_time, double exit_time)
	{
#ifdef DEBUG
		std::cout << "> BEGIN InsertTripSegment." << std::endl;
#endif // DEBUG

		Line tmpLine(std::min(x1, x2), std::min(y1, y2), std::max(x1, x2), std::max(y1, y2));

		/*расчет направления движения*/
		bool orientation = x1 != x2 ? !(x1 < x2) : !(y1 < y2); /* |/>| - 0, |</| - 1, |^| - 0 */
		Interval tmpInterval(entrance_time, exit_time);
		Insert_interval_args args(object_id, tmpLine, tmpInterval, orientation);

#ifdef DEBUG
		std::cout << "\t> Line: (" << tmpLine.min[0] << "," << tmpLine.min[1] << ")->(" << tmpLine.max[0] << "," << tmpLine.max[1] << ")" << std::endl;
		std::cout << "\t> Time interval: (" << entrance_time << ", " << exit_time << ")" << std::endl;
#endif // !DEBUG


		this->spatial_level->search_objects({ tmpLine.min, tmpLine.max }, this->insert_time_interval, (void*)&args);

#ifdef DEBUG
		std::cout << "> END InsertTripSegment." << std::endl;
#endif // DEBUG
	}

	/*
	Пересекает ли сегмент прямоугольник
	*/
	static bool segment_intersect_rectangle
	(
		int rectangleMinX, int rectangleMinY,
		int rectangleMaxX, int rectangleMaxY,
		int p1X, int p1Y, int p2X, int p2Y
	)
	{
		int minX = p1X, maxX = p2X;
		if (p1X > p2X)
		{
			minX = p2X;
			maxX = p1X;
		}
		if (maxX > rectangleMaxX)
		{
			maxX = rectangleMaxX;
		}
		if (minX < rectangleMinX)
		{
			minX = rectangleMinX;
		}
		if (minX > maxX)
		{
			return false;
		}
		int minY = p1Y, maxY = p2Y;
		double dx = p2X - p1X;
		if (dx != 0)
		{
			double a = (p2Y - p1Y) / dx;
			double b = p1Y - a * p1X;
			minY = int(a * minX + b);
			maxY = int(a * maxX + b);
		}
		if (minY > maxY)
		{
			int tmp = maxY;
			maxY = minY;
			minY = tmp;
		}

		if (maxY > rectangleMaxY)
		{
			maxY = rectangleMaxY;
		}

		if (minY < rectangleMinY)
		{
			minY = rectangleMinY;
		}

		if (minY > maxY)
		{
			return false;
		}
		return true;
	}

	/*Внутренний поиск, если пересекаются линия поиска и то, что лежит в дереве, то id добавляется в результирующий список*/
	static bool aux_temporal_search(std::shared_ptr<Temporal_leaf> id, void* arg)
	{
#ifdef DEBUG
		std::cout << "\t> BEGIN auxTemporalSearch." << std::endl;
		std::cout << " \t\tFound: " << id->get_id() << " -> [" << id->get_interval().time_in << ", " << id->get_interval().time_out << "]" << std::endl;
#endif // DEBUG

		Search_args* args = (Search_args*)arg;
		Line sBox = args->s_window;
		Line lSeg = args->lf->get_line();
		bool ori = args->lf->get_orientation();

		int p1X, p1Y, p2X, p2Y;
		if (ori)
		{
			p1X = lSeg.min[0]; p1Y = lSeg.max[1];
			p2X = lSeg.max[0]; p2Y = lSeg.min[1];
		}
		else
		{
			p1X = lSeg.min[0]; p1Y = lSeg.min[1];
			p2X = lSeg.max[0]; p2Y = lSeg.max[1];
		}

		if (segment_intersect_rectangle(sBox.min[0], sBox.min[1], sBox.max[0], sBox.max[1], p1X, p1Y, p2X, p2Y))
		{
			std::set<object_t>* resultArray = args->result_array;
			resultArray->insert(id->get_id());
		}

#ifdef DEBUG
		std::cout << "\t> END   auxTemporalSearch." << std::endl;
#endif // DEBUG
		
		return true;
	}

	/*Внутренний поиск, ищем по временному окну в одномерном дереве*/
	static bool aux_spatial_search(std::shared_ptr<Spatial_leaf> id, void* arg)
	{
#ifdef DEBUG
		std::cout << "\t> BEGIN auxSpatialSearch." << std::endl;
		std::cout << "\t\t" << id->get_name() << std::endl;
#endif // DEBUG
			
		Search_args* args = (Search_args*)arg;
		Interval temporalWindow = args->t_window;
		args->lf = id;

#ifdef DEBUG
		std::cout << "\t-> interval = [" << temporalWindow.time_in << ", " << temporalWindow.time_out << "]" << std::endl;
#endif // DEBUG

		id->get_temporal_tree()->search_in_range({ temporalWindow.time_in, temporalWindow.time_out }, aux_temporal_search, arg);

#ifdef DEBUG
		std::cout << "\t> END   auxSpatialSearch." << std::endl;
#endif // DEBUG

		return true;
	}

	/*
	Поиск всех перемещений всех объектов на некотором отрезке за промежуток времени
	Аргументы:
	-Из какой точки начинать поиск
	-В какой точке заканчивать поиск
	-Время входа в сегмент 
	-Время выхода из сегмента
	-Контейнер, в который записываются объекты, подходящие под поисковый запрос
	*/
	size_t search(int x1, int y1, int x2, int y2, double entranceTime, double exitTime, std::set<object_t>* resultArray)
	{
#ifdef DEBUG
		std::cout << "> BEGIN Search." << std::endl;
#endif // DEBUG

		resultArray->clear();
		Line spatialWindow(std::min(x1, x2), std::min(y1, y2), std::max(x1, x2), std::max(y1, y2));
		Interval temporalWindow(entranceTime, exitTime); /*временное окно*/
		Search_args args(spatialWindow, temporalWindow, resultArray); /*пространственное окно*/

#ifdef DEBUG
		std::cout << "\tsWindow : (" << spatialWindow.min[0] << ", " << spatialWindow.min[1] << "), (" << spatialWindow.max[0] << ", " << spatialWindow.max[1] << ")" << std::endl;
#endif // DEBUG

		this->spatial_level->search_objects({ spatialWindow.min, spatialWindow.max }, aux_spatial_search, (void*)&args);

#ifdef DEBUG
		std::cout << "> END   Search." << std::endl;
#endif // DEBUG
		
		return resultArray->size();
	}

	size_t size() const
	{
		size_t self{ sizeof(FNR_tree) };

		size_t tree_size{ this->spatial_level->size([](const std::shared_ptr<Spatial_leaf>& leaf)
			{
				return leaf->size();
			}) };

		return self + tree_size;
	}

private:
	spatial_level_t spatial_level;
};