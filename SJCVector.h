#pragma once

#include <algorithm>
#include <iostream>


#define BY_VAL_OPERATOR //Selects between by-val assignment operator and copy/move assignment operators

//This is a resource management class. Rule of three applies.
//Move semantics gives us rule of five
//Can move to rule of 4 1/2 if use by-value assignment operator
template<typename T>
class SJCVector {
	std::unique_ptr<T[]> ptr_;		//Class manages resource
	size_t size_;
	size_t first_;
	long long last_;	//cant be size_t as needs to go negative
	const long long emptyVec = -1;
	std::string name_;

public:
	SJCVector() { 
		initSJCVector(1);  
		std::cout << "Standard ctor\n"; 
	}
	SJCVector(std::size_t size) { 
		initSJCVector(size); 
		std::cout << "Standard ctor with size\n"; 
	}
	SJCVector(std::string name, size_t size=1) : name_(name) {
		initSJCVector(size); 
		std::cout << "Standard ctor with name " << name_ << std::endl;
	}
	//Destructor needed to free the resource. (But not if you use smart pointers to ref the resource.
	//Put all cleanup code in the destructor. This makes the class exception safe.

	~SJCVector() {
		printName();
		std::cout << "dtor\n";
	}
	//Copy constructor needed to correctly copy the resource.
	// Compiler generated default copy constructor just does an element by element copy,
	// even for heap pointers, file handles etc.
	// Note we cant just give the new object the resource belonging to the rhs object
	SJCVector(const SJCVector& rhs) {
		std::cout << "Copy ctor. Copying data from ";
		rhs.printName(); std::cout << "to "; printNameLn();
		//Make sure to delete any existing resource before creating a new one
		//ptr_ = new int[rhs.last_ + 1];
		initSJCVector(rhs.size_);
		//Copying the resource avoids double frees
		const auto& srcBegin = rhs.ptr_.get();
		std::copy(srcBegin, std::next(srcBegin, size_), ptr_.get());
		last_ = rhs.last_;
		rename("copy");
	}
	//Move constructor (copy constructor for rvalues)
	//Move constructor transfers ownership of the resource from rhs to this
	//Fast because rhs wont be missed, just steal rhs's guts.
	SJCVector(SJCVector&& rhs) noexcept {
		std::cout << "Move ctor. Stole guts of rvalue: "; rhs.printNameLn();
		ptr_ = std::exchange(rhs.ptr_, nullptr);//ptr_ gets rhs.ptr_, rhs.ptr_ gets nullptr.
		size_ = std::exchange(rhs.size_, 0);
		last_ = std::exchange(rhs.last_, emptyVec);
	}
#ifdef BY_VAL_OPERATOR
	//By value assignment operator
	//Less common is to replace both move assignment operators with a by-value assignment operator
	//Because the copy assignment and move assignment operators are so similar
	//Could get both by creating a pass by value assignment operator
	//This approach transfers move semantics responsibility to the caller
	SJCVector& operator=(SJCVector copy) {
		std::cout << "By-value assignment (=) operator\n";
		copy.swap(*this);
		return *this;
	}
#else
	//Copy assignment operator
	//Free the left hand resource and copy the right hand one
	//Use the copy and swap idiom. This decouples any aliasing relationship between *this and rhs
	SJCVector& operator=(const SJCVector& rhs) {
		std::cout << "Copy assignment operator. (Uses Copy constructor)\n";
		SJCVector copy = rhs;	//make a copy of the rhs object using the copy constructor
		copy.rename("copy");
		copy.swap(*this);
		std::cout << "End of Copy assignment operator\n";
		return *this;
	}
	//Move assignment operator
	//Free the left-hand resource and transfer ownership of the rhs one
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
	//Two argument swap makes this type efficiently std::swappable
	// This is a non member function using the hidden friend idiom.
	// The friend function is in the body of our class ie in its namespace
	// Marking as friend lets the compiler know it is not a member
	friend void swap(SJCVector& a, SJCVector& b) noexcept {
		//Just calls the member swap
		a.swap(b);
	}
	SJCVector operator+(const SJCVector& rhs) {
		//This routine applies element by element addition
		//Todo: Only works for types where '+" is defined		
		std::cout << "Addition operator overload for SJCVector\n";
		if ((rhs.size_  == 0) 
		||	(size_ == 0)
		||	(rhs.last_ != last_)){
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
	void push_back(T newValue) {
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
		//std::unique_ptr<T[]> newptr = std::make_unique<T[]>(newSize);
		//TODO exception safety. Did the memory allocate?
		if (auto newptr = std::make_unique<T[]>(newSize)) {
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
		ptr_ = std::make_unique<T[]>(size_);
		first_ = 0;
		last_ = emptyVec;
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

