#include <iostream>

class Entity {

public:
	int *list;

	Entity()
	{

		list = new int[5];
		std::cout << "Entity created!" << std::endl;

	}

	Entity(const Entity &other)
	{

		std::cout << "Copy Constructor" << std::endl;

	}

	Entity(Entity **other)
	{

		std::cout << "** Constructor" << std::endl;

	}

	~Entity() {
		
		delete[] list;
		std::cout << "Entity destroyed!" << std::endl;

	}

	void Print() {

		std::cout << "List ptr: " << list << std::endl;

	}

};


int main() {

	{
		Entity e;
		Entity *ptr = &e;

		Entity &entity = *ptr;

	}


	std::cin.get();

}