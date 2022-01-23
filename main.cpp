

#include "SJCVector.h"


int main() {
	std::cout << "Test standard constructor\n";
	SJCVector n("nigel");
	n.print();
	n.push_back(3);
	n.push_back(56);
	n.print();

	std::cout << "\nTest resizing\n";
	SJCVector k("kelly");
	k.print();
	k.resize(5);
	k.print();
	k.push_back(1);
	k.push_back(2);
	k.push_back(3);
	k.push_back(4);
	k.push_back(5);
	k.push_back(6);
	k.print();

	std::cout << "\nTest copy constructor\n";
	SJCVector m = n;
	m.rename("mary");
	m.push_back(9);
	m.push_back(7);
	m.print();

	n.push_back(1);
	n.push_back(2);
	n.print();
	
	std::cout << "\nTest move constructor and + operator overload\n";
	SJCVector o(n + m);
	o.rename("oscar");
	o.print();

	std::cout << "\nTest copy assignment operator\n";
	m.print();
	m = o;
	m.print();

	std::cout << "\nTest move assignment operator\n";
	SJCVector p("pelle");
	p = n + o;
	n.print();
	o.print();
	p.print();

	std::cout << "\nEnd of tests\n\n";

}