#pragma once

#include <algorithm>
#include <iostream>


#define BY_VAL_OPERATOR

//This is a resource management class. Rule of three applies.
//Move semantics gives us rule of five
//Can move to rule of 4 1/2 if use by-value assignment operator
template<typename T>
class SJCVector {
	T* ptr_;		//Class manages resource
	size_t size_;
	size_t first_;
	long long last_;
	std::string name_;

public:
	SJCVector() : ptr_(nullptr), size_(0) { 
		initSJCVector();  
		std::cout << "Standard ctor\n"; 
	}
	SJCVector(std::size_t size) : ptr_(size ? new T[size]() : nullptr), size_(size) { 
		initSJCVector(); 
		std::cout << "Standard ctor with size\n"; 
	}
	SJCVector(std::string name, size_t newSize=0) : ptr_(nullptr), size_(0), name_(name) {
		initSJCVector(); 
		resize(newSize);  
		std::cout << "Standard ctor with name " << name_ << std::endl;
	}
	//Destructor needed to free the resource. This makes the class exception safe.
	//Put all cleanup code in the destructor
	~SJCVector() {
		printName();
		std::cout << "dtor\n";
		delete[] ptr_;
	}
	//Copy constructor needed to correctly copy the resource.
	// Compiler generated default copy constructor just does an element by element copy,
	// even for heap pointers, file handles etc.
	// Note we cant just give the new object the resource belonging to the rhs object
	SJCVector(const SJCVector& rhs) {
		std::cout << "Copy ctor. Copying data from ";
		rhs.printName(); std::cout << "to "; printNameLn();
		//Make sure to delete any existing resource before creating a new one
		//if (ptr_ != nullptr) delete[] ptr_;
		ptr_ = new T[rhs.last_ + 1];
		last_ = rhs.last_;
		size_ = rhs.last_ + 1;
		//Copying the resource avoids double frees
		std::copy(rhs.ptr_, rhs.ptr_ + last_ + 1, ptr_);
		rename("copy");
	}
	//Move constructor (copy constructor for rvalues)
	//Move constructor transfers ownership of the resource from rhs to this
	//Fast because rhs wont be missed, just steal rhs's guts.
	SJCVector(SJCVector&& rhs) noexcept : SJCVector()  {
		std::cout << "Move ctor. Stole guts of rvalue: "; rhs.printNameLn();
		ptr_ = std::exchange(rhs.ptr_, nullptr);
		size_ = std::exchange(rhs.size_, 0);
		last_ = std::exchange(rhs.last_, -1);
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
		//Adds element by element
		std::cout << "Addition operator overload for SJCVector\n";
		if ((rhs.size_  == 0) 
		|| (size_ == 0)
		|| (rhs.last_ != last_)) {
			std::cout << "Cannot add vectors of zero size or unequal size\n";
			return SJCVector();
		}
		std::cout << "Create local return vector\n";
		SJCVector retVec(rhs);
		retVec.rename("retVec");
		std::cout << "Added: ";
		for (int i = 0; (i <= last_); i++) {
			retVec.ptr_[i] += ptr_[i];
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
			*(ptr_ + last_) = newValue;
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
		T* newptr = new T[newSize];
		//TODO exception safety. Did the memory allocate?
		if (!newptr) {
			std::cout << "\nError: resize failed\n";
			return;
		}
		if (last_ >= 0) {
			//Data to copy
			//New size_ may be smaller than current data
			if (newSize <= last_) last_ = newSize - 1;
			//new size should not be 0
			if (newSize > 0) std::copy(ptr_, ptr_ + last_ + 1, newptr);
		}
		delete[] ptr_;
		ptr_ = newptr;
		size_ = newSize;
		std::cout << "Resized "; printName(); std::cout << "to " << size_ << " with " << last_ + 1 << " items\n";
	}
private:
	void initSJCVector() {
		first_ = 0;
		last_ = -1;
	}
	void printItems() const {
		if (last_ < 0 || size_ == 0) return;
		for (int i = 0; i <= last_; i++) {
			std::cout << *(ptr_ + i);
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

