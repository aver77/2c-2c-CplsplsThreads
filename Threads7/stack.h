#pragma once

//контейнер односвязный список без загл звена
struct Node
{
	int info;
	Node* next;
};
typedef Node* TList;

class ThreadSafe_stack
{
	TList head; 
	//lock_guard<mutex> guard(mut); - обеспечим крит секцию
public:
	ThreadSafe_stack(); //init
	void push(int val); //добавление в голову
	bool pop(int& val);
	int top();
};
