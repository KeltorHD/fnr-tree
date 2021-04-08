#pragma once

#include "rtree.hpp"

#include "line.hpp"
#include "interval.hpp"

class FNR_tree
{
public:
	FNR_tree();

	~FNR_tree();

	FNR_tree(const FNR_tree&) = delete;  /*конструкторы и операторы копирования и переноса удалены*/
	FNR_tree(FNR_tree&&) = delete;
	FNR_tree operator=(const FNR_tree&) = delete;
	FNR_tree operator=(FNR_tree&&) = delete;
};