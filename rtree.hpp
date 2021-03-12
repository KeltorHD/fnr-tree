#pragma once

#include <exception>
#include <array>
#include <vector>
#include <utility>
#include <algorithm>
#include <variant>
#include <limits>
#include <iostream>

static int count_shared{ 0 };

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
	/*точка n-мерного пространства*/
	using point_t = std::array<coord_type, num_dims>;
	/*левая нижн. точка и правая верхн. точка mbr*/
	struct  mbr_t
	{
		/*первая точка*/
		point_t ld;
		/*вторая точка*/
		point_t ru;
	};        

	/*конструктор по умолчанию*/
	R_tree();
	~R_tree() = default;

	R_tree(const R_tree&) = delete;  /*конструкторы и операторы копирования и переноса удалены*/
	R_tree(R_tree&&) = delete;
	R_tree operator=(const R_tree&) = delete;
	R_tree operator=(R_tree&&) = delete;

	/*основные функции*/
	/*вставка нового значения в дерево с заданным mbr*/
	void insert(const data_type& data, const mbr_t& mbr);

	/*поиск нужного объекта по его mbr*/
	//const data_type& find(const mbr_t& mbr, bool& success) const;

	/*дебаг: вывод дерева*/
	void print() const;

private:
	struct node;
	/*shared_ptr указатель */
	using node_ptr_t        = std::shared_ptr<node>;
	/*информация о потомке и его mbr (находится в внут. узле)*/
	struct child_info_t     { node_ptr_t child; mbr_t mbr{}; };
	/*информация об объекте и его mbr (находится в листе)*/
	struct data_info_t      { data_type data; mbr_t mbr{}; };

	/*массив информаций о потомках*/
	using child_array_ptr_t = std::array<child_info_t, max_nodes>;
	/*массив информаций об объектах (находится в листе)*/
	using data_array_t      = std::array<data_info_t, max_nodes>;
	/*enum-тип, хранит либо инф. о потомках, либо инф. об объектах*/
	using data_node_t       = std::variant<child_array_ptr_t, data_array_t>;

	/*корень дерева*/
	node_ptr_t root;

	/*структура узла*/
	struct node
	{
		/*минимальный ограничивающий прямоугольник всех потомков или данных в узле*/
		mbr_t mbr{};
		/*количество данных или указателей в массиве*/
		size_t count_array{ 0 };
		/*если не лист, храним ptr_array_t*/
		data_node_t data{ child_array_ptr_t{} };
		/*лист или нет?*/
		bool leaf{ false };

		/*конструктор, true - листочек*/
		node(bool leaf)
			: leaf(leaf)
		{
			count_shared++;
			if (this->leaf) this->data = data_array_t{};
			else this->data = child_array_ptr_t{};
		}
		~node()
		{
			std::cout << (this->leaf ? "Лист " : "Узел ") << count_shared << " адрес " << this << " уничтожен" << std::endl;
			count_shared--;
		}
	};

	/*дебаг: печать дерева*/
	void print(const node_ptr_t& to_print, size_t& level) const;

	/*поиск нужного объекта по его mbr в вершине v*/
	//const data_type& find(const node_ptr_t& v, const mbr_t& mbr, bool& success) const;

	/*функции для работы с деревом*/
	/*поиск листа, в который можно поместить новое значение*/
	node_ptr_t choice_leaf(const mbr_t& mbr) const;

	/*деление листа на 2 по квадратичному алгоритму Гуттмана (1984)*/
	node_ptr_t node_division(node_ptr_t l1, const data_type& data, const mbr_t& mbr);

	/*деление узла*/
	node_ptr_t node_division(node_ptr_t l1, child_info_t new_child);

	/*корректировка дерева*/
	void correct_tree(node_ptr_t l1, node_ptr_t l2);                                 

	/*выбор пары из множества значений*/
	std::pair<data_info_t, data_info_t> get_first_pair(std::vector<data_info_t>& q);
	std::pair<child_info_t, child_info_t> get_first_pair(std::vector<child_info_t>& q);

	/*выбор следующего элемента по алгоритму Гуттмана (1984)*/
	data_info_t get_next(node_ptr_t l1, node_ptr_t l2, std::vector<data_info_t>& q);
	child_info_t get_next(node_ptr_t l1, node_ptr_t l2, std::vector<child_info_t>& q);

	/*нахождение родителя элемента*/
	node_ptr_t get_parent(node_ptr_t where, const node_ptr_t& what, const mbr_t& mbr) const;

	/*функции для работы с mbr*/
	mbr_t sum_mbr(const mbr_t& m1, const mbr_t& m2) const;              /*новый mbr из 2-х*/
	coord_type calc_square(const mbr_t& m) const;                       /*вычисление площади */
	coord_type diff_square_mbr(const mbr_t& m1, const mbr_t& m2) const; /*разница в площадях двух mbr*/
	bool include_mbr(const mbr_t& where_find, const mbr_t& what_find) const;/*включает ли один mbr другой*/
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
		coord_type min_diff{ std::numeric_limits<coord_type>::max() };
		coord_type cur_diff{};
		size_t need_child{};
		for (size_t i = 0; i < tmp->count_array; i++)
		{
			cur_diff = this->diff_square_mbr(this->sum_mbr(ptrs[i].mbr, mbr), mbr);
			if (cur_diff <= min_diff)
			{
				min_diff = cur_diff;
				need_child = i;
			}
		}
		tmp = ptrs[need_child].child;
	}
	return tmp;
}

R_template
inline typename R_class_area::mbr_t R_class_area::sum_mbr(const mbr_t& m1, const mbr_t& m2) const
{
	point_t ld{}, ru{};
	for (size_t i = 0; i < num_dims; i++)
	{
		ld[i] = std::min(m1.ld[i], m2.ld[i]); /*левая нижняя точка*/
		ru[i] = std::max(m1.ru[i], m2.ru[i]); /*правая верхняя точка*/
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
	coord_type square{ std::abs(m.ld[0] - m.ru[0]) };
	for (size_t i = 1; i < num_dims; i++)
	{
		square *= std::abs(m.ld[i] - m.ru[i]);
	}
	return std::abs(square);
}

R_template
inline bool R_class_area::include_mbr(const mbr_t& where_find, const mbr_t& what_find) const
{
	for (size_t i = 0; i < num_dims; i++)
	{
		if (!(where_find.ld[i] <= what_find.ld[i] && where_find.ru[i] >= what_find.ru[i]))
			return false;
	}
	return true;
}

R_template
inline void R_class_area::insert(const data_type& data, const mbr_t& mbr)
{
	node_ptr_t l{ choice_leaf(mbr) };
	node_ptr_t new_node{ nullptr };

	if (l->count_array < max_nodes) /*если значение помещается в текущий узел*/
	{
		data_array_t& arr{ std::get<data_array_t>(l->data) };
		arr[l->count_array].data = data; /*добавляем значение в узел*/
		arr[l->count_array].mbr = mbr;
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
	std::vector<data_info_t> q{ l1_data.begin(), l1_data.end() }; /*заполнение o*/
	q.push_back(data_info_t{ data, mbr });
	for (size_t i = 0; i < max_nodes; i++) /*очистка l1*/
	{
		l1_data[i].data = data_type{};
		l1_data[i].mbr = mbr_t{};
	}
	l1->count_array = 0;
	l1->mbr = {};

	/*первые значения*/
	std::pair<data_info_t, data_info_t> obj = this->get_first_pair(q);
	l1_data[l1->count_array] = obj.first;
	l1->count_array++;
	l1->mbr = obj.first.mbr;  /*-_- изменяем mbr*/
	l2_data[l2->count_array] = obj.second;
	l2->count_array++;
	l2->mbr = obj.second.mbr; /*-_- изменяем mbr*/

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
		else if (int(min_nodes) - int(n1) >= int(n))
		{
			for (size_t i = 0; i < q.size(); i++)
			{
				l1_data[l1->count_array] = q[i];
				l1->count_array++;
				l1->mbr = this->sum_mbr(l1->mbr, q[i].mbr); /*изменяем mbr всего листа*/
			}
			return l2;
		}
		else if (int(min_nodes) - int(n2) >= int(n))
		{
			for (size_t i = 0; i < q.size(); i++)
			{
				l2_data[l2->count_array] = q[i];
				l2->count_array++;
				l2->mbr = this->sum_mbr(l2->mbr, q[i].mbr); /*изменяем mbr всего листа*/
			}
			return l2;
		}

		data_info_t o_next{ this->get_next(l1,l2,q) };
		coord_type d1{ this->diff_square_mbr(this->sum_mbr(l1->mbr, o_next.mbr), l1->mbr) };
		coord_type d2{ this->diff_square_mbr(this->sum_mbr(l2->mbr, o_next.mbr), l2->mbr) };

		if (d1 < d2 || (d1 == d2 && n1 < n2))
		{
			l1_data[l1->count_array] = o_next;
			l1->count_array++;
			l1->mbr = this->sum_mbr(l1->mbr, o_next.mbr); /*изменяем mbr всего листа*/
		}
		else
		{
			l2_data[l2->count_array] = o_next;
			l2->count_array++;
			l2->mbr = this->sum_mbr(l2->mbr, o_next.mbr); /*изменяем mbr всего листа*/
		}
	}
}


R_template
inline typename R_class_area::node_ptr_t R_class_area::node_division(node_ptr_t l1, child_info_t new_child)
{

	/*подготовка*/
	node_ptr_t l2(std::make_shared<node>(false)); /*выделение памяти на узел*/
	child_array_ptr_t& l1_data{ std::get<child_array_ptr_t>(l1->data) };
	child_array_ptr_t& l2_data{ std::get<child_array_ptr_t>(l2->data) };
	std::vector<child_info_t> q{ l1_data.begin(), l1_data.end() }; /*заполнение o*/
	q.push_back(new_child);
	for (size_t i = 0; i < max_nodes; i++) /*очистка l1*/
	{
		l1_data[i].child = nullptr;
		l1_data[i].mbr = mbr_t{};
	}
	l1->count_array = 0;
	l1->mbr = {};

	/*первые значения*/
	std::pair<child_info_t, child_info_t> obj = this->get_first_pair(q);
	l1_data[l1->count_array] = obj.first;
	l1->count_array++;
	l1->mbr = obj.first.mbr;  /*-_- изменяем mbr*/
	l2_data[l2->count_array] = obj.second;
	l2->count_array++;
	l2->mbr = obj.second.mbr; /*-_- изменяем mbr*/

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
		else if (int(min_nodes) - int(n1) >= int(n))
		{
			for (size_t i = 0; i < q.size(); i++)
			{
				l1_data[l1->count_array] = q[i];
				l1->count_array++;
				l1->mbr = this->sum_mbr(l1->mbr, q[i].mbr); /*изменяем mbr всего листа*/
			}
			return l2;
		}
		else if (int(min_nodes) - int(n2) >= int(n))
		{
			for (size_t i = 0; i < q.size(); i++)
			{
				l2_data[l2->count_array] = q[i];
				l2->count_array++;
				l2->mbr = this->sum_mbr(l2->mbr, q[i].mbr); /*изменяем mbr всего листа*/
			}
			return l2;
		}

		child_info_t o_next{ this->get_next(l1,l2,q) };
		coord_type d1{ this->diff_square_mbr(this->sum_mbr(l1->mbr, o_next.mbr), l1->mbr) };
		coord_type d2{ this->diff_square_mbr(this->sum_mbr(l2->mbr, o_next.mbr), l2->mbr) };

		if (d1 < d2 || (d1 == d2 && n1 < n2))
		{
			l1_data[l1->count_array] = o_next;
			l1->count_array++;
			l1->mbr = this->sum_mbr(l1->mbr, o_next.mbr); /*изменяем mbr всего листа*/
		}
		else
		{
			l2_data[l2->count_array] = o_next;
			l2->count_array++;
			l2->mbr = this->sum_mbr(l2->mbr, o_next.mbr); /*изменяем mbr всего листа*/
		}
	}
}


R_template
inline std::pair<typename R_class_area::data_info_t, typename R_class_area::data_info_t> 
R_class_area::get_first_pair(std::vector<data_info_t>& q)
{
	size_t o1_index{}, o2_index{};
	coord_type max_square{ std::numeric_limits<coord_type>::min() };
	std::pair<data_info_t, data_info_t> ret{};
	for (size_t i = 0; i < q.size(); i++)
	{
		for (size_t j = 0; j < q.size(); j++)
		{
			if (i == j) continue;

			coord_type tmp_square{ this->calc_square(this->sum_mbr(q[i].mbr, q[j].mbr)) - this->calc_square(q[i].mbr) - this->calc_square(q[j].mbr) };
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
inline std::pair<typename R_class_area::child_info_t, typename R_class_area::child_info_t>
R_class_area::get_first_pair(std::vector<child_info_t>& q) 
{
	size_t o1_index{}, o2_index{};
	coord_type max_square{ std::numeric_limits<coord_type>::min() };
	std::pair<child_info_t, child_info_t> ret{};
	for (size_t i = 0; i < q.size(); i++)
	{
		for (size_t j = 0; j < q.size(); j++)
		{
			if (i == j) continue;

			coord_type tmp_square{ this->calc_square(this->sum_mbr(q[i].mbr, q[j].mbr)) - this->calc_square(q[i].mbr) - this->calc_square(q[j].mbr) };
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
R_class_area::get_next(node_ptr_t l1, node_ptr_t l2, std::vector<data_info_t>& q)
{
	size_t o_index{};
	data_info_t o{};
	coord_type max_diff_square{ std::numeric_limits<coord_type>::min() };
	for (size_t i = 0; i < q.size(); i++)
	{
		coord_type d1{ this->calc_square(this->sum_mbr(l1->mbr,q[i].mbr)) - this->calc_square(l1->mbr) };
		coord_type d2{ this->calc_square(this->sum_mbr(l2->mbr,q[i].mbr)) - this->calc_square(l2->mbr) };
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
inline typename R_class_area::child_info_t
R_class_area::get_next(node_ptr_t l1, node_ptr_t l2, std::vector<child_info_t>& q)
{
	size_t o_index{};
	child_info_t o{};
	coord_type max_diff_square{ std::numeric_limits<coord_type>::min() };
	for (size_t i = 0; i < q.size(); i++)
	{
		coord_type d1{ this->calc_square(this->sum_mbr(l1->mbr,q[i].mbr)) - this->calc_square(l1->mbr) };
		coord_type d2{ this->calc_square(this->sum_mbr(l2->mbr,q[i].mbr)) - this->calc_square(l2->mbr) };
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
				rref[this->root->count_array].child = v1;
				rref[this->root->count_array].mbr = v1->mbr;
				this->root->count_array++;
				rref[this->root->count_array].child = v2;
				rref[this->root->count_array].mbr = v2->mbr;
				this->root->count_array++;
				this->root->mbr = this->sum_mbr(v1->mbr, v2->mbr);
			}
			return;
		}

		/*если v1 не корень*/
		node_ptr_t p{ this->get_parent(this->root, v1, v1->mbr) }; /*находим родителя v1*/
		child_array_ptr_t& rref{ std::get<child_array_ptr_t>(p->data) };
		child_info_t info{rref[0]}; /*находим запись о v1*/
		for (size_t i = 1; i < p->count_array && info.child != v1; i++)
		{
			info = rref[i];
		}
		info.mbr = v1->mbr; /*обновляем mbr в записи у родителя v1*/
		p->mbr = this->sum_mbr(p->mbr, v1->mbr); /*обновляем общий mbr у предка*/
		v1 = p; /*изменяем v1*/

		if (v2 != nullptr)
		{
			child_info_t pv2{ v2, v2->mbr };
			if (p->count_array < max_nodes) /*если новая запись влезает*/
			{
				child_array_ptr_t& rref{ std::get<child_array_ptr_t>(p->data) };
				rref[p->count_array] = pv2;
				p->count_array++;
				v2 = nullptr;
			}
			else /*делим предка*/
			{
				/*решаем косяк с mbr*/
				for (size_t i = 0; i < p->count_array; i++)
				{
					rref[i].mbr = rref[i].child->mbr;
				}
				v2 = this->node_division(p, pv2);
			}
		}
	}
}

R_template
inline typename R_class_area::node_ptr_t R_class_area::get_parent(node_ptr_t where, const node_ptr_t& what, const mbr_t& mbr) const
{
	if (!where->leaf) /*если текущая позиция не листочек*/
	{
		child_array_ptr_t& ref{ std::get<child_array_ptr_t>(where->data) };
		for (size_t i = 0; i < where->count_array; i++)
		{
			/*поиск исключительно по указателям*/
			if (ref[i].child == what) 
			{
				return where;
			}
			else
			{
				node_ptr_t find_ptr{ this->get_parent(ref[i].child, what, mbr) };
				if (find_ptr != nullptr)
					return find_ptr;
			}
 		}
	}
	return nullptr;
}


R_template
inline void R_class_area::print() const
{
	std::cout << "Дерево." << std::endl;

	size_t level{ 0 };
	this->print(this->root, level);
	
	std::cout << std::endl;
}

R_template
inline void R_class_area::print(const node_ptr_t& to_print, size_t& level) const
{
	for (size_t l = 0; l < level; l++)
	{
		std::cout << "    ";
	}
	std::cout << (to_print->leaf ? "Лист. " : "Узел. ") << to_print << " mbr: (";
	for (size_t j = 0; j < 2; j++)
	{
		std::cout << "(";
		for (size_t k = 0; k < num_dims; k++)
		{
			if (k != 0)
				std::cout << ", ";
			std::cout << (j == 0 ? to_print->mbr.ld[k] : to_print->mbr.ru[k]);
		}
		std::cout << (j == 0 ? "), " : ")");
	}
	std::cout << "). ";
	std::cout << to_print->count_array << (to_print->leaf ? " объектов. " : " ссылок на потомки. ") << "Данные: " << std::endl;

	if (to_print->leaf)
	{
		for (size_t i = 0; i < to_print->count_array; i++)
		{
			for (size_t l = 0; l < level + 1; l++)
			{
				std::cout << "    ";
			}
			std::cout << "Объект: " << std::get<data_array_t>(to_print->data)[i].data << ", mbr: (";
			for (size_t j = 0; j < 2; j++)
			{
				std::cout << "(";
				for (size_t k = 0; k < num_dims; k++)
				{
					if (k != 0)
						std::cout << ", ";
					std::cout << (j == 0 ? std::get<data_array_t>(to_print->data)[i].mbr.ld[k] : std::get<data_array_t>(to_print->data)[i].mbr.ru[k]);
				}
				std::cout << (j == 0 ? "), " : ")");
			}
			std::cout << ")" << std::endl;
		}
	}
	else
	{
		level++;
		for (size_t i = 0; i < to_print->count_array; i++)
		{
			this->print(std::get<child_array_ptr_t>(to_print->data)[i].child, level);
		}
		level--;
	}
}

//R_template
//inline const data_type& R_class_area::find(const mbr_t& mbr, bool& success) const
//{
//	if (this->root->count_array)
//		return this->find(this->root, mbr, success);
//	
//	success = false;
//	return data_type{};
//}

//R_template
//inline const data_type& R_class_area::find(const node_ptr_t& v, const mbr_t& mbr, bool& success) const
//{
//	if (!v->leaf)
//	{
//		for (size_t i = 0; i < v->count_array; i++)
//		{
//			if (this->include_mbr(std::get<child_array_ptr_t>(v->data)[i].mbr, mbr))
//			{
//				return this->find(std::get<child_array_ptr_t>(v->data)[i].child, mbr, success);
//			}
//		}
//	}
//	else
//	{
//		for (size_t i = 0; i < v->count_array; i++)
//		{
//			if (this->include_mbr(mbr, std::get<data_array_t>(v->data)[i].mbr))
//			{
//				success = true;
//				return std::get<data_array_t>(v->data)[i].data;
//			}
//		}
//		success = false;
//		return data_type{};
//	}
//}
