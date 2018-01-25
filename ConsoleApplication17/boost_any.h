#include<utility>
#include<typeinfo>
#include<typeindex>

/**
* boost::any impletement
*
* 1 ����Ҫ�洢�������ͣ������ֲ���ָ����������
*   �� any��������ģ����
*   �� any�Ĺ��캯��������ģ�幹�캯�����Զ��Ƶ����ͣ�
*
* 2 ����Ҫ����������������Ϣ�����Բ���ʹ��void *
*   ��Ҫһ��ģ��class������������Ϣ
*
* 3 ����any�е�placeholder������ģ��class
*   ������Ҫ�ٶ���һ��typeholder���̳�placeholder
*
* 4 ����
*
* any
* any_cast
* bad_any_cast
*/

namespace my_boost
{
	class any {
	public:
		// Constructor
		any() {};

		// any a(32)
		template<typename T>
		any(const T &obj)
			:_obj(new typeholder<T>(obj))
		{}

		any(const any &other)
			:_obj(other.empty() ? nullptr : other._obj->clone())
		{}

		// any a = 32
		template<typename T>
		any& operator=(const T &other)
		{
			any(other).swap(*this);
			return *this;
		}

		any& operator=(const any  &other)
		{
			any(other).swap(*this);
			return *this;
		}

		// move
		any(any &&other)
			:_obj(other._obj)
		{
			other._obj = nullptr;
		}

		any& operator=(any &&other) {
			delete _obj;
			_obj = other._obj;
			other._obj = nullptr;
			return *this;
		}

		// Destructor
		~any() { delete _obj; }

		// empty
		bool empty() const
		{
			return _obj == nullptr;
		}

		// clear
		void clear()
		{
			if (empty())
				return;

			delete _obj;
			_obj = nullptr;
		}

		//type
		const std::type_info& type() const
		{
			if (empty())
				return typeid(void);

			return _obj->type();
		}

		//swap
		any& swap(any &other)
		{
			std::swap(_obj, other._obj);
			return *this;
		}

		class placeholder {
		public:

			virtual ~placeholder() {}

			virtual placeholder* clone() = 0;

			virtual const std::type_info& type() = 0;
		};

		template<typename T>
		class typeholder : public placeholder
		{
		public:
			typeholder(const T& v)
				:_v(v)
			{}

			placeholder* clone()
			{
				//return typeholder<T>(_v);
				return new typeholder(_v);
			}

			const std::type_info& type()
			{
				return typeid(T);
			}

			T _v;
		};

	private:
		placeholder *_obj = nullptr;

		template<typename T> friend T* any_cast(any *a);
		template<typename T> friend T any_cast(any &a);
	};

	// bad_any_cast
	class bad_any_cast : public std::bad_cast
	{
	public:
		virtual const char * what() const noexcept(true)
		{
			return "bad_any_cast: fail conversion using any_cast";
		}
	};

	// any_cast
	template<typename T>
	T* any_cast(any *a)
	{
		if (std::type_index(typeid(T)).hash_code() !=
			std::type_index(a->type()).hash_code()
			|| a == nullptr)
			return nullptr;

		auto typeholder_ptr = static_cast<any::typeholder<T>*>(a->_obj);
		return static_cast<T*>(&typeholder_ptr->_v);
	}

	template<typename T>
	T any_cast(any &a)
	{
		T* r = any_cast<T>(&a);
		if (r == nullptr)
			throw bad_any_cast();

		return static_cast<T>(*r);
	}
}