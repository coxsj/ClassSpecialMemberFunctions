

#include "SJCVector.h"


int main() {
	std::cout << "Test standard constructor\n";

	SJCVector<float> f;
	f.push_back(1.00000f);
	f.push_back(2.1);
	f.push_back(3.2);
	f.print();

	SJCVector<std::string> ts("template strings");
	ts.push_back("Fred");
	ts.push_back("Apple");
	ts.push_back("Bob");
	ts.push_back("Sally");
	ts.print();

	SJCVector<std::string> ts1("template strings");
	ts1.push_back("Guinn");
	ts1.push_back("Computer");
	ts1.push_back("theBuilder");
	ts1.push_back("Struthers");
	ts1.print();

	SJCVector ts2(ts + ts1);
	ts2.print();

	SJCVector<int> n("nigel");
	n.print();
	n.push_back(3);
	n.push_back(56);
	n.print();

	std::cout << "\nTest resizing\n";
	SJCVector<int> k("kelly");
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
	SJCVector<int> m = n;
	m.rename("mary");
	m.push_back(9);
	m.push_back(7);
	m.print();

	n.push_back(1);
	n.push_back(2);
	n.print();
	
	std::cout << "\nTest move constructor and + operator overload\n";
	SJCVector<int> o(n + m);
	o.rename("oscar");
	o.print();

	std::cout << "\nTest copy assignment operator\n";
	m.print();
	m = o;
	m.print();

	std::cout << "\nTest move assignment operator\n";
	SJCVector<int> p("pelle");
	p = n + o;
	n.print();
	o.print();
	p.print();

	std::cout << "\n~~~End of tests~~~\n\n";

}