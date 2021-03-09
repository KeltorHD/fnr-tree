#pragma once

#include <memory>
#include <array>
#include <vector>
#include <utility>
#include <algorithm>
#include <variant>
#include <limits>
#include <iostream>

#define R_template   template<typename data_type, typename coord_type, size_t num_dims, typename float_type, size_t max_nodes, size_t min_nodes>
#define R_class_area R_tree<data_type, coord_type, num_dims, float_type, max_nodes, min_nodes>


/*класс r-дерева*/
template<
	typename data_type, 
	typename coord_type, 
	size_t num_dims, 
	typename float_type,
	size_t max_nodes = 8,
	size_t min_nodes = max_nodes / 2>
class R_tree
{
public:
	using point_t = std::array<coord_type, num_dims>; /*точка n-мерного пространства*/
	using mbr_t   = std::array<point_t, 2>;           /*левая нижн. точка и правая верхн. точка mbr*/

	R_tree();
	~R_tree() = default;

	R_tree(const R_tree&) = delete;  /*конструкторы и операторы копирования и переноса удалены*/
	R_tree(R_tree&&) = delete;
	R_tree operator=(const R_tree&) = delete;
	R_tree operator=(R_tree&&) = delete;

	/*основные функции*/
	void insert(const data_type& data, const mbr_t& mbr); /*вставка нового значения в дерево с заданным mbr*/

private:
	struct node;
	using node_ptr_t        = std::shared_ptr<node>;                         /*тип указателя на node*/
	using child_info_t      = std::pair<node_ptr_t, mbr_t>;                  /*указатели на потомков + их mbr*/
	using child_array_ptr_t = std::array<child_info_t, max_nodes>;             /*массив указателей на потомков*/
	using data_info_t       = std::pair<data_type, mbr_t>;                   /*данные + mbr данных*/
	using data_array_t      = std::array<data_info_t, max_nodes>;            /*массив id,mbr на data*/
	using data_node_t       = std::variant<child_array_ptr_t, data_array_t>; /*указатели на потомков или массив данных*/

	node_ptr_t root;

	struct node
	{
		mbr_t mbr{};                             /*минимальный ограничивающий прямоугольник всех потомков или данных в узле*/
		size_t count_array{ 0 };                 /*количество данных или указателей в массиве*/
		data_node_t data{ child_array_ptr_t{} }; /*если не лист, храним ptr_array_t*/
		bool leaf{ false };                      /*лист или нет?*/

		node(bool leaf) /*true - листочек*/
			: leaf(leaf)
		{
			if (this->leaf) this->data = data_array_t{};
			else this->data = child_array_ptr_t{};
		}
	};

	/*функции для работы с деревом*/
	/*поиск листа, в который можно поместить новое значение*/
	node_ptr_t choice_leaf(const mbr_t& mbr) const;

	/*деление узла на 2 по квадратичному алгоритму Гуттмана (1984)*/
	node_ptr_t node_division(node_ptr_t l1, const data_type& data, const mbr_t& mbr);

	/*корректировка дерева*/
	void correct_tree(node_ptr_t l1, node_ptr_t l2);                                 

	/*выбор пары из множества значений*/
	std::pair<data_info_t, data_info_t> get_first_pair(std::vector<const data_info_t&>& q);

	/*выбор следующего элемента по алгоритму Гуттмана (1984)*/
	data_info_t get_next(node_ptr_t l1, node_ptr_t l2, std::vector<const data_info_t&>& q);

	/*нахождение родителя элемента*/
	node_ptr_t get_parent(node_ptr_t l) const;                                             
	//mbr_t      correct_mbr(node_ptr_t)

	/*функции для работы с mbr*/
	mbr_t sum_mbr(const mbr_t& m1, const mbr_t& m2) const;              /*новый mbr из 2-х*/
	coord_type calc_square(const mbr_t& m) const;                       /*вычисление площади */
	coord_type diff_square_mbr(const mbr_t& m1, const mbr_t& m2) const; /*разница в площадях двух mbr*/
};

R_template
inline R_class_area::R_tree()
	:root(std::make_shared<node>(true))
{
}

R_template
inline typename R_class_area::node_ptr_t R_class_area::choice_leaf(const mbr_t& mbr) const
{
	node_ptr_t tmp{ this->root };
	while (!tmp->leaf)
	{
		const child_array_ptr_t& ptrs{ std::get<child_array_ptr_t>(tmp->data) };
		coord_type min_diff{ std::numeric_limits<size_t>::max() };
		coord_type cur_diff{};
		size_t need_child{};
		for (size_t i = 0; i < tmp->count_array; i++)
		{
			cur_diff = this->diff_square_mbr(this->sum_mbr(ptrs[i].second, mbr), mbr);
			if (cur_diff <= min_diff)
			{
				min_diff = cur_diff;
				need_child = i;
			}
		}
		tmp = ptrs[need_child].first;
	}
	return tmp;
}

R_template
inline typename R_class_area::mbr_t R_class_area::sum_mbr(const mbr_t& m1, const mbr_t& m2) const
{
	point_t ld{}, ru{};
	for (size_t i = 0; i < num_dims; i++)
	{
		ld[i] = std::min(m1[0][i], m2[0][i]); /*левая нижняя точка*/
		ru[i] = std::max(m1[1][i], m2[1][i]); /*правая верхняя точка*/
	}
	return mbr_t{ ld,ru };
}

R_template
inline coord_type R_class_area::diff_square_mbr(const mbr_t& m1, const mbr_t& m2) const
{
	return std::abs(this->calc_square(m1) - this->calc_square(m2));
}

R_template
inline coord_type R_class_area::calc_square(const mbr_t& m) const
{
	coord_type square{ std::abs(m[0][0] - m[1][0]) };
	for (size_t i = 1; i < num_dims; i++)
	{
		square *= std::abs(m[0][i] - m[1][i]);
	}
	return std::abs(square);
}


R_template
inline void R_class_area::insert(const data_type& data, const mbr_t& mbr)
{
	node_ptr_t l{ choice_leaf(mbr) };
	node_ptr_t new_node{ nullptr };

	if (l->count_array < max_nodes) /*если значение помещается в текущий узел*/
	{
		data_array_t& arr{ std::get<data_array_t>(l->data) };
		arr[l->count_array].first = data; /*добавляем значение в узел*/
		arr[l->count_array].second = mbr;
		l->count_array++;
		l->mbr = l->count_array == 1 ? mbr : this->sum_mbr(l->mbr, mbr); /*если в узле 1 вершина, ставим mbr новых данных, иначе находим сумму*/
	}
	else /*делим узел на два*/
	{
		new_node = this->node_division(l, data, mbr);
	}
	this->correct_tree(l, new_node); /*корректируем дерево*/
}

R_template
inline typename R_class_area::node_ptr_t R_class_area::node_division(node_ptr_t l1, const data_type& data, const mbr_t& mbr)
{
	/*подготовка*/
	node_ptr_t l2(std::make_shared<node>(true)); /*выделение памяти на лист*/
	data_array_t& l1_data{ std::get<data_array_t>(l1->data) };
	data_array_t& l2_data{ std::get<data_array_t>(l2->data) };
	std::vector<const data_info_t&> q{ l1_data.cbegin(), l1_data.cend() }; /*заполнение o*/
	q.push_back(std::make_pair(data, mbr));
	for (size_t i = 0; i < max_nodes; i++) /*очистка l1*/
	{
		l1_data[i].first = data_type{};
		l1_data[i].second = mbr_t{};
	}
	l1->count_array = 0;
	l1->mbr = {};

	/*первые значения*/
	std::pair<data_info_t, data_info_t> obj = this->get_first_pair(q);
	l1_data[l1->count_array] = obj.first;
	l1->count_array++;
	l1->mbr = obj.first.second;  /*-_- изменяем mbr*/
	l2_data[l2->count_array] = obj.second;
	l2->count_array++;
	l2->mbr = obj.second.second; /*-_- изменяем mbr*/

	/*остальные значения*/
	while (true)
	{
		size_t n{ q.size() };
		size_t n1{ l1->count_array };
		size_t n2{ l2->count_array };

		if (n == 0)
		{
			return l2;
		}
		else if (min_nodes - n1 >= n)
		{
			for (size_t i = 0; i < q.size(); i++)
			{
				l1_data[l1->count_array] = q[i];
				l1->count_array++;
				l1->mbr = this->sum_mbr(l1->mbr, q[i].second); /*изменяем mbr всего листа*/
			}
			return l2;
		}
		else if (min_nodes - n2 >= n)
		{
			for (size_t i = 0; i < q.size(); i++)
			{
				l2_data[l2->count_array] = q[i];
				l2->count_array++;
				l2->mbr = this->sum_mbr(l2->mbr, q[i].second); /*изменяем mbr всего листа*/
			}
			return l2;
		}

		data_info_t o_next{ this->get_next(l1,l2,q) };
		coord_type d1{ this->diff_square_mbr(this->sum_mbr(l1->mbr, o_next.second), l1->mbr) };
		coord_type d2{ this->diff_square_mbr(this->sum_mbr(l2->mbr, o_next.second), l2->mbr) };

		if (d1 < d2 || (d1 == d2 && n1 < n2))
		{
			l1_data[l1->count_array] = o_next;
			l1->count_array++;
			l1->mbr = this->sum_mbr(l1->mbr, o_next.second); /*изменяем mbr всего листа*/
		}
		else
		{
			l2_data[l2->count_array] = o_next;
			l2->count_array++;
			l2->mbr = this->sum_mbr(l2->mbr, o_next.second); /*изменяем mbr всего листа*/
		}
	}

	/*корректировка дерева*/
	this->correct_tree(l1, l2);
}

R_template
inline std::pair<typename R_class_area::data_info_t, typename R_class_area::data_info_t> 
R_class_area::get_first_pair(std::vector<const data_info_t&>& q)
{
	size_t o1_index{}, o2_index{};
	coord_type max_square{ std::numeric_limits<size_t>::min() };
	std::pair<data_info_t, data_info_t> ret{};
	for (size_t i = 0; i < q.size(); i++)
	{
		for (size_t j = 0; j < q.size(); j++)
		{
			if (i == j) continue;

			coord_type tmp_square{ this->calc_square(this->sum_mbr(q[i].second, q[j].second)) - this->calc_square(q[i].second) - this->calc_square(q[j].second) };
			if (tmp_square >= max_square)
			{
				max_square = tmp_square;
				o1_index = i;
				o2_index = j;
			}
		}
	}
	ret.first = q[o1_index];
	ret.second = q[o2_index];
	q.erase(q.begin() + std::max(o1_index, o2_index));
	q.erase(q.begin() + std::min(o1_index, o2_index));

	return ret;
}

R_template
inline typename R_class_area::data_info_t 
R_class_area::get_next(node_ptr_t l1, node_ptr_t l2, std::vector<const data_info_t&>& q)
{
	size_t o_index{};
	data_info_t o{};
	coord_type max_diff_square{ std::numeric_limits<size_t>::min() };
	for (size_t i = 0; i < q.size(); i++)
	{
		coord_type d1{ this->calc_square(this->sum_mbr(l1->mbr,q[i].second)) - this->calc_square(l1->mbr) };
		coord_type d2{ this->calc_square(this->sum_mbr(l2->mbr,q[i].second)) - this->calc_square(l2->mbr) };
		coord_type a{ std::abs(d1 - d2) };

		if (a >= max_diff_square)
		{
			max_diff_square = a;
			o_index = i;
		}
	}
	o = q[o_index];
	q.erase(q.begin() + o_index);
	return o;
}

R_template
inline void R_class_area::correct_tree(node_ptr_t l1, node_ptr_t l2)
{
	node_ptr_t v1{ l1 };
	node_ptr_t v2{ l2 };

	while (true)
	{
		if (v1 == this->root) /*если v1 - корень*/
		{
			if (v2 != nullptr) /*и есть новая вершина -> произошло деление узлов*/
			{
				this->root.reset();
				this->root = std::make_shared<node>(false); /*создание новой вершины*/

				child_array_ptr_t& rref{ std::get<child_array_ptr_t>(this->root->data) }; /*сложим 2 листочка в корень*/
				rref[this->root->count_array].first = v1;
				rref[this->root->count_array].second = v1->mbr;
				this->root->count_array++;
				rref[this->root->count_array].first = v2;
				rref[this->root->count_array].second = v2->mbr;
				this->root->count_array++;
				this->root->mbr = this->sum_mbr(v1->mbr, v2->mbr);
			}
			return;
		}

		/*если v1 не корень*/
		node_ptr_t p{ this->get_parent(v1) };
		child_array_ptr_t& rref{ std::get<child_array_ptr_t>(p->data) };
		child_info_t& info{ *(std::find(rref.begin(), rref.end(), [&v1](const child_info_t& inf) {return inf.first == v1; })) }; /*находим запись о v1*/
		info.second = v1->mbr; /*обновляем mbr у родителя v1*/
		v1 = p; /*изменяем v1*/

		if (v2 != nullptr)
		{
			if (p->count_array < max_nodes) /*если новая запись влезает*/
			{
				child_array_ptr_t& rref{ std::get<child_array_ptr_t>(p->data) };
				rref[p->count_array].first = v2;
				rref[p->count_array].second = v2->mbr;
				p->count_array++;
			}
			else /*делим предка*/
			{
				data_info_t di{ std::get<data_info_t>(v2->data) };
				v2.reset();
				v2 = this->node_division(p, di.first, di.second); /*пересоздаем v2, разделяем данные в p и v2*/
			}
		}
	}
}
