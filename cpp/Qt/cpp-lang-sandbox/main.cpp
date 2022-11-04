#include <memory>
#include <iostream>


class Base : public std::enable_shared_from_this<Base> {

public:
		Base() {}

};


class Derived : public Base, std::enable_shared_from_this<Derived> {

public:
		Derived() {}
		void test_shared_pointer()
		{
			std::shared_ptr<Derived> ptr = std::enable_shared_from_this<Derived>::shared_from_this();
			std::cout << ptr.use_count() << std::endl;
		}
};


class Derived2 : public Base {

public:
		Derived2()
		{
//			std::shared_ptr<Base> ptr = shared_from_this();
//			std::cout << ptr.use_count() << std::endl;
			std::cout << " Derived2 Constructor Complete" << std::endl;
		}

		void initialize()
		{
			std::shared_ptr<Base> ptr = shared_from_this();
			std::cout << ptr.use_count() << std::endl;
		}
};


int main(int argc, char *argv[])
{
//	auto d = std::make_shared<Derived>();
	auto d = std::make_shared<Derived2>();
	d->initialize();
	return 0;
}
