#include "stack.h"
#include <iostream>
#include <Windows.h>
#include <thread>
#include <mutex>

using namespace std;

mutex mut; //обьект ядра (каждый поток полноценно получает  доступ к одному ресуру - стеку)

ThreadSafe_stack::ThreadSafe_stack()
{
	head = nullptr;
}

//push pop и top работают атомарно
void ThreadSafe_stack::push(int val)
{
	//обертка, обеспечивающая гарантированное освобожд мьютекса (мьютекс созд в сразу захваченном ссостоянии)
	lock_guard<mutex> guard(mut);
	//крит область
	TList p = new Node; 
	p->next = head;
	head = p;
	p->info = val;
	cout << "Pushed->" << val << endl;
}

bool ThreadSafe_stack::pop(int& val)
{
	lock_guard<mutex> guard(mut);
	bool result = false;
	val = -1;
	if (head != nullptr) //узнаем не пуст ли контейнер
	{
		result = true;
		TList p = head;
		head = head->next;
		val = p->info;
		p->next = nullptr;
		cout << "Popped<-" << val << endl;
		delete p;
	}
	return result;
}

int ThreadSafe_stack::top()
{
	lock_guard<mutex> guard(mut);
	int val = -1;
	if (head != nullptr)
		val = head->info;
	return val;
}


