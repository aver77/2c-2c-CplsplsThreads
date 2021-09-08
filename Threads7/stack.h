#pragma once

//��������� ����������� ������ ��� ���� �����
struct Node
{
	int info;
	Node* next;
};
typedef Node* TList;

class ThreadSafe_stack
{
	TList head; 
	//lock_guard<mutex> guard(mut); - ��������� ���� ������
public:
	ThreadSafe_stack(); //init
	void push(int val); //���������� � ������
	bool pop(int& val);
	int top();
};
