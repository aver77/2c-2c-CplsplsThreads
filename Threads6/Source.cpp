#include <iostream>
#include <Windows.h>
#include <thread>
#include <fstream>

using namespace std;

const int N = 1;
const int NTHREAD = 3;
int arr[N];

//������ ���������� ����� = 0
volatile long lock = 0;

int globalMin = MAXINT;
int globalIndex = 0;

//��� ����� ������� � ����������� �������� � ��� ������ ��� ����� ��������� ����� �� ������ � result
struct result
{
	int index;
	int min;
};

int index_nonparallel(int* arr)
{
	int localMin = MAXINT, index = 0;
	for (int i = 0; i < N; i++)
	{
		if (arr[i] <= localMin)
		{
			localMin = arr[i];
			index = i;
		}
	}
	return index;
}

void _indexmin(int *a, int left, int right)
{
	//�������� ��������� ������ (���� ������ ������) � ������������� �� ����� � ����������� ������
	int localMin = MAXINT, localIndex = 0;
	for (int i = left; i < right; i++)
	{
		if (a[i] <= localMin)
		{
			localMin = a[i];
			localIndex = i;
		}
	}
	//�������� ������� ������ � ����������� ������
	if (localMin <= globalMin && localIndex >= globalIndex)
	{
		//������� ����� � ����������� ������ ��� ������ �-��, �������������� ��������� (���������) ������
		//���������� � _InterlockedCompareExchange ������ � ������ ������� (lock � 0). ���� �����, �� lock �������� ��������� �� 1 (��������� �����)
		while (_InterlockedCompareExchange(&lock, 1, 0))
			Sleep(0); //���� ����� ������, �� �������, �� ����� ������������ �����
		//����� ����� � ����������� ������
		if (localMin <= globalMin && localIndex >= globalIndex)
		{
			globalMin = localMin;
			globalIndex = localIndex;
		}
		//����� �� �� � ������ �����
		_InterlockedCompareExchange(&lock, 0, 1); 
	}
}

void index_parallel(int* a)
{
	int n = N / NTHREAD; //���������� ��������� � �������� ������
						 //������ �������
	thread t[NTHREAD];

	if (n < 1) //���� ���������� ��������� ������, ��� ���������� �������� �������
	{
		t[0] = thread(_indexmin, arr, 0, N);
		if (t[0].joinable())
			t[0].join(); //������������� ������
	}
	//��������������� �������� �������
	else
	{
		for (int i = 0; i < NTHREAD; i++)
		{
			t[i] = thread(_indexmin, arr, n*i, n*(i + 1));
		}
		//��������� '������'
		if (N != n * NTHREAD)
		{
			_indexmin(arr, n* NTHREAD, N);
		}
		//waitformultipleobjects - WINAPI
		//�������� ���������� ������ �������
		for (int i = 0; i < NTHREAD; i++)
		{
			if (t[i].joinable())
				t[i].join(); //������������� �������
		}
	}
}

void readfromfile(fstream& file)
{
	while (!file.eof())
	{
		for (int i = 0; i < N; i++)
			file >> arr[i];
	}
	file.close();
}
void print()
{
	for (int i = 0; i < N; i++)
		cout << arr[i] << " ";
}

int main()
{
	setlocale(LC_ALL, "RU");
	fstream fin("inp.txt");
	readfromfile(fin);
	print();

	cout << "\nnonparallel:" << index_nonparallel(arr) << '\n';
	index_parallel(arr);
	cout << "parallel (� ���������� ���������� ����������):" << globalIndex << '\n';

	return 0;
}