#pragma once

#include "rtree.hpp"

#include "line.hpp"
#include "interval.hpp"

#include <string>

template <typename object_t>
class FNR_tree
{
private:
	class Spatial_leaf;
	class Temporal_leaf;
	using spatial_level_t = std::shared_ptr<R_tree<std::shared_ptr<Spatial_leaf>, int, 2, float>>;

public:
	FNR_tree() = default;
	~FNR_tree() = default;

	FNR_tree(const FNR_tree&) = delete;  /*конструкторы и операторы копирования и переноса удалены*/
	FNR_tree(FNR_tree&&) = delete;
	FNR_tree operator=(const FNR_tree&) = delete;
	FNR_tree operator=(FNR_tree&&) = delete;

	/*внутренняя реализация*/
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

	class Spatial_leaf
	{
	public:
		using temporal_t = std::shared_ptr<R_tree<std::shared_ptr<Temporal_leaf>, double, 1, float>>;

		Spatial_leaf() = default;
		~Spatial_leaf() = default;
		Spatial_leaf(Line l, bool ori, std::string nn)
		{
			line = l;
			orientation = ori;
			nnn = nn;
			temporal_tree = std::make_shared<temporal_t>();
		}
		const Line& get_line() const
		{
			return line;
		}
		const std::string& get_name() const
		{
			return this->nnn;
		}
		bool get_orientation() const
		{
			return orientation;
		}
		const temporal_t& get_temporal_tree() const
		{
			return temporal_tree;
		}
		/*size_t size()
		{
			size_t totalSize = sizeof(Spatial_leaf) + sizeof(Line) + sizeof(RTree<TemporalLeaf*, double, 1, float>);

			RTree<TemporalLeaf*, double, 1, float>::Iterator it;
			temporalTree->GetFirst(it);

			while (!(temporalTree->IsNull(it)))
			{
				totalSize += (*it)->size();
				temporalTree->GetNext(it);
			}

			return totalSize;
		}*/
	private:
		bool orientation;
		temporal_t temporal_tree;
		Line line;
		std::string nnn;

	};

private:
	spatial_level_t SpatialLevel;
};