#include "stack.h"
#include <iostream>
#include <Windows.h>
#include <thread>
#include <mutex>
#include <fstream>
#include <cmath>

using namespace std;

//���������� �������������� � ������������
const int producer_count = 3, consumer_count = 4;

//����� ������ ������������� � ����������� ����� ��������� ��������
volatile long producer_amount_of_work, consumer_amount_of_work;

//������ ������� ������ ������������� - ����������� ��-�� �� ����� � ���������
void produce(ifstream& fin, ThreadSafe_stack& st)
{
	//��������� ���������� ������ ������ (����� ��� ���������� 100% ����������� ��� �������� � ���������� ������)
	while (_InterlockedExchangeAdd(&producer_amount_of_work, -1) > 0) //���� (�������������/����������� ������������ ���� ����� ������)
																	  //������ � ���� ������ ���� �� ���� ����� ��������
																	  //���� �������� (3) ������ ���������� 15 ��-���
	{
		int value;
		fin >> value;
		st.push(value);
	}
}

struct consume_even
{
	int number_thread;
	int counter_even;
};

//������ ������� ������ ����������� - ���������� ��-��� �� ���������� � ���������� ������-�� ��� ��� 
//(������� ������ ���������)
void consume(ThreadSafe_stack& st, consume_even& even)
{
	int localCounter_even = 0;
	//��������� ���������� ������ ������ (����� ��� ���������� 100% ����������� ��� �������� � ���������� ������)
	while (_InterlockedExchangeAdd(&consumer_amount_of_work, -1) > 0) //���� (�������������/����������� ������������ ���� ����� ������)
																	  //������ � ���� ������ ���� �� ���� ����� �������� � ���� ��������� �� ����
																	  //���� �������� (3) ������ ���������� 15 ��-���
	{
		int val; 
		while (st.pop(val)) //���������� �������� ������� �������� ������� �� �����
		{
			if ((abs(val)) % 2 == 0)
				localCounter_even++;
		}
	}
	even.counter_even = localCounter_even;
}

int main()
{
	setlocale(LC_ALL, "RU");
	ifstream fin("inp.txt");

	ThreadSafe_stack st;

	if (fin.is_open())
	{
		cout << "���� ������!" << endl;
		//����� ������ - ������� ��������� �������� ������������� � �����������
		int amount = 0;
		cout << "����� ������->"; cin >> amount;
		producer_amount_of_work = amount;
		consumer_amount_of_work = amount;

		//������ �������������� � ������������
		thread producerThreads[producer_count];
		thread consumerThreads[consumer_count];

		//������ ������������ ���� � ������� �������� ���������� ������ ����� � ������ ������ �����������
		consume_even arr[consumer_count];

		//��������������� ������� �������������� � ������������
		for (int i = 0; i < producer_count; i++)
		{
			//���� ����� ��������� �����-���� �������� �� ������, �� ���
			//����� ���������� � ������� ref, ����� �����������, ��� ����� �� �������
			//������ ������ ����� �������� ������ � �� �����
			producerThreads[i] = thread(&produce, ref(fin), ref(st));
		}

		for (int i = 0; i < consumer_count; i++)
		{
			consumerThreads[i] = thread(&consume, ref(st), ref(arr[i]));
			arr[i].number_thread = i;
		}

		//waitformultipleobjects - WINAPI
		//�������� ���������� ������ �������
		for (int i = 0; i < producer_count; i++)
		{
			if (producerThreads[i].joinable())
			{
				producerThreads[i].join(); //������������� �������
			}
		}
		for (int i = 0; i < consumer_count; i++)
		{
			if (consumerThreads[i].joinable())
			{
				consumerThreads[i].join(); //������������� �������
			}
		}

		//����� �-��� ������
		for (int i = 0; i < consumer_count; i++)
		{
			cout << "����� ������ �����������:" << arr[i].number_thread + 1 << ", ���������� ������ �����:" << arr[i].counter_even << endl;
		}

		fin.close();
	}
	else
	{
		cout << "������ �������� �����!\n";
	}

}