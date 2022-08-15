#pragma once

#include <algorithm>
#include <iostream>

// References
// =========
// Back to Basics: RAII and the Rule of Zero - Arthur O'Dwyer - CppCon 2019
//		https://www.youtube.com/watch?v=7Qgd9B1KuMQ
// Back To Basics: The Special Member Functions - Klaus Iglberger - CppCon 2021
//		https://www.youtube.com/watch?v=9BM5LAvNtus


// Summary
// Rule of zero. If your class manage no resources requiring manual cleanup, write no special member functions.
// Instead, default them all.
// If the class manages resources. Rule of three applies (Destructor, Copy Constructor, Copy Assignment).
// Resource is anything that requires special manual management
// e.g Allocated memory, file handles, mutex locks, c++ threads etc.
// Whenever the rule of three applies, use the copy-and-swap idiom to implement assignment
// Move semantics gives us rule of five
// Can move to rule of 4 1/2 if use by-value assignment operator

// Rationale
// =======
// DESTRUCTOR is responsible for freeing the resources correctly.
// If you write a destructor, you probably need to write a copy constructor AND a copy assignment operator.
// COPY CONSTRUCTOR is responsible for duplicating resources to avoid double-frees. Double-frees occur when
// a new object is created via naively copying a pointer which of course points back to the resource owned by
// the original object. That's what the default copy constructor does.
// NOTE: Initialization is not assignment!
// Classname w = v; // Initialization (construction) of a new object. It calls the COPY CONSTRUCTOR
// is not the same as...
// Classname w; w = v; // This is an assignment to the existing object w. It calls an ASSIGNMENT OPERATOR
// So, assignment has potentially the same problem as copying.
// COPY ASSIGNMENT OPERATOR should be defined using COPY CONSTRUCTOR  and a swap member function.
// COPY ASSIGNMENT OPERATOR has to free the left-hand resource and copy the right-hand one.
// In the case of self assignment (v = v; or assignment to self-referential data structures e.g. tree that result in v = v;)
// the COPY ASSIGNMENT constructor is at risk of deleting the data in the original object.
// So, COPY ASSIGNEMENT OPERATOR should use COPY CONSTRUCTOR and a swap member function.
// RAII RESOURCE ACQUISITION IS INITIALIZATION: Slogan is about initialization, but it's really about CLEANUP!
// RAII + Exception safety. When an exception occurs, stack unwinds all the way till a suitable catch handler is found.
// For each local scope between the throw and the catch, the runtime invokes the destructors of all local variables
// in that scope. So, to avoid leaks, put all CLEANUP code in your DESTRUCTORS.
// To make an object NON-COPYABLE, explicitly set the copy constructor and copy assignment operator to = delete.
// DEFAULT special member functions. Explicitly setting special member functions to =default make code
// selff documenting. i.e. I considered if the default special member functions were acceptable and they are
// so I am ok with the compiler default implementation.
// RVALUE REFERENCES are useful in generating special member functions.
// Generally, lvalues dont bind to rvalues and rvalues dont bind to lvalues.
// Special case of const lvalue reference will bind to rvalue!!
// void f(int&);				f(i);	// Ok			f(42);	// Error
// foid g(int&&);				g(i);	// Error		g(42)	// Ok
// void h(const int&);		h(i)	// Ok			h(42)	// Ok
// Combine this with overload resolution (and knowing that rvalues wont be missed)
// No one cares what we do with the contents of an rvalue when passed to us.
// MOVE CONSTRUCTOR is an rvalue parameter overload of the COPY CONSTRUCTOR
// MOVE CONSTRUCTOR doesnt need to bother with the slow copying operations
// just steal the guts of the rvalue. In practice, use std::exchange e.g. ptr_ = std::exchange(rhs.ptr_, nullptr);
// (rhs.ptr_ goes to ptr_ and nullptr goes to rhs.ptr_).
// RULEOF FIVE. The need for performance adds the MOVE CONSTRUCTOR and MOVE ASSIGNMENT OPERATOR.
// MOVE CONSTRUCTOR - transfer ownership of resource.
// MOVE ASSIGNMENT OPERATOR - free the left hand resource and transfer ownership of the right hand one.
// Note: The copy constructor and the move assignment operator are very similar, difffering only in the std::move in
// the move assignment operator. And this is how the STL does these special member functions.
// However this provides an opportunity to set both assignment operators up as call by value, leaving the copy 
// decision up to the caller.
// This leads to the RULE OF FOUR AND A HALF - destructor, copy constructor, move constructor, by-value
// assignment operator, and (1/2) a non-member swap function, and ideally a member version too. (If you don't,
// std::swap gets called which in turn calls move assignment which calls swap...)
// Note the by-value assignment operator requires passing a copy of the object on the stack to the operator.
// This will be slow for large objects.
// UNIQUE_PTR helps us move closer to the rule of zero.
// By managing ownership of member resources with unique_ptr, we can delegate handling transfer to unique_ptr.
// This works because std::unique_ptr CAN ONLY BE MOVED, its copy constructor is deleted.
// MOVE CONSTRUCTOR with unique_ptr becomes Classname(Classname&& rhs noexcept = default;
// DESTRUCTOR with unique_ptr becomes ~Classname() = default; // std::unique_ptr destroys the resource.
// wherein the transfer of owenership is handled by unique_ptr when it is moved into the newly constructed object.

#define BY_VAL_OPERATOR

class SJCVector {
	std::unique_ptr<int[]> ptr_ = nullptr;		//Class manages resource
	size_t size_{ 0 };
	size_t first_;
	long long last_;
	std::string name_{ "unnamed" };

public:
	SJCVector() : SJCVector(1) { 
		std::cout << "Standard ctor\n"; 
	}
	SJCVector(std::size_t size) { 
		initSJCVector(size); 
		std::cout << "Standard ctor with size\n"; 
	}
	SJCVector(std::string name, size_t size=1) : SJCVector(size) {
		name_ = name;
		std::cout << "Standard ctor with name " << name_ << std::endl;
	}
	// Destructor needed to free the resource. (But not if you use smart pointers to ref the resource).
	// Put all cleanup code in the destructor. This makes the class exception safe.
	// There is only ever one destructor for a class.

	~SJCVector() {
		printName();
		std::cout << "dtor\n";
	}
	// COPY CONSTRUCTOR
	// ===================
	// Copy constructor is needed to correctly copy the resource.
	// Compiler generated default copy constructor just does an element by element copy,
	// even for heap pointers, file handles etc.
	// This leads to DOUBLE-FREEING if there is a Destructor (which frees the resource) and only 
	// the default copy constructor (which copies just the pointer/Handle etc, but does not create a new resource).
	// The default copy constructor leaves ptrs pointing to the same resource owned by the object that was copied.
	// And of course we cant just give the new object the resource belonging to the rhs object!!
	SJCVector(const SJCVector& rhs) {
		std::cout << "Copy ctor. Copying data from ";
		rhs.printName(); std::cout << "to "; printNameLn();
		// Make sure to delete any existing resource before creating a new one
		// ptr_ = new int[rhs.last_ + 1];
		initSJCVector(rhs.size_);
		// Copying the resource avoids double frees
		const auto& srcBegin = rhs.ptr_.get();
		std::copy(srcBegin, std::next(srcBegin, size_), ptr_.get());
		last_ = rhs.last_;
		rename("copy");
	}
	// MOVE CONSTRUCTOR (copy constructor for rvalues)
	// Move constructor transfers ownership of the resource from rhs to this
	// Fast because rhs wont be missed, just steal rhs's guts.
	SJCVector(SJCVector&& rhs) noexcept {
		std::cout << "Move ctor. Stole guts of rvalue: "; rhs.printNameLn();
		ptr_ = std::exchange(rhs.ptr_, nullptr);//ptr_ gets rhs.ptr_, rhs.ptr_ gets nullptr.
		size_ = std::exchange(rhs.size_, 0);
		last_ = std::exchange(rhs.last_, -1);
	}
#ifdef BY_VAL_OPERATOR
	// BY-VALUE ASSIGNMENT OPERATOR
	// The copy & move assignment operators are similar. We get both by creating a by-value assignment operator.
	// Less common. This approach transfers move semantics responsibility to the caller.
	SJCVector& operator=(SJCVector copy) noexcept {
		std::cout << "By-value assignment (=) operator\n";
		copy.swap(*this);
		return *this;
	}
#else
	// COPY ASSIGNMENT OPERATOR
	// Free the left hand resource and copy the right hand one
	// Use the copy and swap idiom. This decouples any aliasing relationship between *this and rhs
	SJCVector& operator=(const SJCVector& rhs) {
		std::cout << "Copy assignment operator. (Uses Copy constructor)\n";
		SJCVector copy = rhs;	//make a copy of the rhs object using the copy constructor
		copy.rename("copy");
		copy.swap(*this);
		std::cout << "End of Copy assignment operator\n";
		return *this;
	}
	// MOVE ASSIGNMENT OPERATOR
	// Free the left-hand resource and transfer ownership of the rhs one
	SJCVector& operator=( SJCVector&& rhs) noexcept{
		std::cout << "Move assignment operator. Uses Move constructor\n";
		SJCVector copy(std::move(rhs));	//make a copy of the rhs object using the MOVE constructor
		copy.swap(*this);
		std::cout << "End of Move assignment operator\n";
		return *this;
	}
#endif
	void swap(SJCVector& rhs) noexcept{
		using std::swap;
		swap(ptr_, rhs.ptr_);
		swap(size_, rhs.size_);
		swap(first_, rhs.first_);
		swap(last_, rhs.last_);
	}
	// TWO ARGUMENT SWAP
	// Makes this class efficiently std::swappable
	// This is a non member function using the hidden friend idiom.
	// The friend function is in the body of our class ie in its namespace
	// Marking as friend lets the compiler know it is not a member
	friend void swap(SJCVector& a, SJCVector& b) noexcept {
		//Just calls the member swap
		a.swap(b);
	}
	SJCVector operator+(const SJCVector& rhs) {
		// Only works for types where '+" is defined
		// This routine applies element by element addition
		std::cout << "Addition operator overload for SJCVector\n";
		if ((rhs.size_  == 0) 
		|| (size_ == 0)
		|| (rhs.last_ != last_)) {
			std::cout << "Cannot add vectors of zero size or unequal size\n";
			return SJCVector();
		}
		std::cout << "Create local return vector\n";
		SJCVector retVec(*this);
		retVec.rename("retVec");
		std::cout << "Added: ";
		for (int i = 0; (i <= last_); i++) {
			retVec.ptr_[i] += rhs.ptr_[i];
			if(i > 0) std::cout << ", ";
			std::cout << retVec.ptr_[i];
		}
		std::cout << std::endl;
		return retVec;
	}
	void print() const {
		printName();
		printSize();
		printItemsLn();
	}
	void push_back(int newValue) {
		if ((size_ == last_ + 1) || size_ == 0) {
			std::cout << "On push_back: ";
			resize();
		}
		if (size_ > last_ + 1) {
			last_++;
			ptr_[last_] = newValue;
		}
		else std::cout << "push_back fail due to full\n";
	}
	void rename(std::string newName) { 
		printName();
		std::cout << "renamed to ";
		name_ = newName;
		printNameLn();
	}
	void resize(size_t newSize) {
		if (newSize == 0) newSize = 1;
		//TODO exception safety. Did the memory allocate?
		if (auto newptr = std::make_unique<int[]>(newSize)) {
			if (last_ >= 0) {
				//Data to copy
				//New size_ may be smaller than current data
				if (newSize <= last_) last_ = newSize - 1;
				//new size should not be 0
				const auto& srcBegin = ptr_.get();
				if (newSize > 0) std::copy(srcBegin, std::next(srcBegin, size_), newptr.get());
				//old: if (newSize > 0) std::copy(ptr_, ptr_[0 + last_ + 1], newptr);
			}
			ptr_= std::move( newptr );
			size_ = newSize;
			std::cout << "Resized "; printName(); std::cout << "to " << size_ << " with " << last_ + 1 << " items\n";
		}
		else {
			std::cout << "\nError: resize failed\n";
			return;
		}		
	}
private:
	void initSJCVector(size_t initialSize=1) {
		size_ = initialSize;
		ptr_ = std::make_unique<int[]>(size_);
		first_ = 0;
		last_ = -1;
	}
	void printItems() const {
		if (last_ < 0 || size_ == 0) return;
		for (int i = 0; i <= last_; i++) {
			std::cout << ptr_[i];
			if (i != last_) std::cout << ", ";
		}
		std::cout << " ";
		if (size_ == last_ + 1) std::cout << "(full) ";
		else std::cout << "(" << size_ - (last_ + 1) << " slots left) ";
	}
	void printItemsLn() const {
		printItems();
		std::cout << std::endl;
	}
	void printName() const {
		if (name_.size() > 0) std::cout << name_ << " ";
		else std::cout << "Unnamed SJCVector ";
	}
	void printNameLn() const {
		printName();
		std::cout << std::endl;
	}
	void printSize() const {
		std::cout << "Size:" << size_;
		if (size_ == 0)std::cout << " empty ";
		else std::cout << " has " << last_ + 1 << " items: ";
	}
	void printSizeLn() const{
		printSize();
		std::cout << "\n";
	}
	void resize() {
		//geometric resize
		resize(size_ * 2 + 1);
	}
};

