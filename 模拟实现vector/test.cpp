namespace HML
{
	template<class T>
	class vector
	{
		typedef T* iterator;
		typedef const T* const_iterator;
	public:
		//迭代器
		iterator begin()
		{
			return _start;
		}
		iterator end()
		{
			return _finish;
		}
		const_iterator cbegin() const
		{
			return _start;
		}
		const_iterator cend()const
		{
			return _finish;
		}
		//6大默认成员函数
		vector()	//构造1
			:_start(nullptr)
			,_finish(nullptr)
			,_endOfStorage(nullptr)
		{}
		vector(size_t n, const T& valua = T())	//构造2
			:_start(nullptr)
			, _finish(nullptr)
			, _endOfStorage(nullptr)
		{
			reserve(n);
			while (n--)
			{
				push_back(valua);
			}
		}
		template<class InputIterator>
		vector(InputIterator first, InputIterator last)			//构造3
			:_start(nullptr)
			, _finish(nullptr)
			, _endOfStorage(nullptr)
		{
			reserve(last - first);
			while (first != last)
			{
				push_back(*first);
				first++;
			}
		}
		vector(const vector<T>& v)	//拷贝构造
			:_start(nullptr)
			, _finish(nullptr)
			, _endOfStorage(nullptr)
		{
			reserve(v.capacity);
			const_iterator cstart = v.cbegin();
			while (cstart != v.cegin())
			{
				push_back(*cstart);
				cstart++;
			}
			_finish = _start + v.size();
			_endOfStorage = _start + v.capacity();
		}
		~vector()	//析构
		{
			delete[] _start;
			_finish = nullptr;
			_endOfStorage = nullptr;
		}
		//空间
		void reserve(size_t n)	
		{
			if (n > capacity())
			{
				size_t oldsize = size();
				T* newspace = new T[n];
				//开好空间后把原数据移到新空间中
				if (_start)
				{
					for (size_t i = 0; i < oldsize; i++)
					{
						newspace[i] = _start[i];
					}
				}
				//移动好后，删除旧空间
				_start = newspace;
				_finish = _start + oldsize;
				_endOfStorage = _start + n;
			}
		}
		size_t size()
		{
			return _finish - _start;
		}
		size_t capacity()
		{
			return _endOfStorage - _start;
		}
		void resize(size_t n, const T& value = T())
		{
			//n小于size时缩小数据即可
			if (n < size())
			{
				_finish = _start + n;
				return;
			}
			//n大于容量时需要扩容
			if (n > capacity())
			{
				reserve(n);
			}
			//填补空缺位置初始化
			iterator it = _finish;
			_finish = _start + n;
			while(it != end)
			{
				*it = value;
				it++;
			}
		}
		//重载
		T& operator[](size_t pos)
		{
			return _start[pos];
		}
		vector<T>& operator=(const vector<T> v)
		{
			swap(v);
			return *this;
		}
		//操作
		void push_back(const T& x)
		{
			insert(end(), x);
		}
		void pop_back()
		{
			erase(end()-1);
		}
		void swap(vector<T>& v)
		{
			std::swap(_start, v._start);
			std::swap(_finish, v._finish);
			std::swap(_endOfStorage, v._endOfStorage);
		}
		iterator insert(iterator pos, const T& x)
		{
			assert(pos <= _finish);
			//判断是否需要扩容
			if (size() == capacity())
			{
				int oldpos = pos - _start;
				int newcapacity = (capacity() == 0) ? 4 : 2 * capacity();
				reserve(newcapacity);
				//扩容之后需要重新定位pos的位置
				pos = _start + oldpos;
			}
			//移动数据
			auto end = _finish - 1;
			while (end >= pos)
			{
				*(end + 1) = *end;
				end--;
			}
			//插入x
			*pos = x;
			_finish++;
			return pos;
		}
		iterator erase(iterator pos)
		{
			assert(size() > 0);
			auto start = pos;
			while (start != _finish - 1)
			{
				*start = *(start + 1);
				start++;
			}
			_finish--;
			return pos;	//指向了下个元素的位置
		}
	private:
		iterator _start;
		iterator _finish;
		iterator _endOfStorage;
	};
}