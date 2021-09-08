#include <iostream>
#include <Windows.h>
#include <thread>
#include <fstream>

using namespace std;

const int N = 2;
const int NTHREAD = 3;
int arr[N];

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

//������ ������
void _indexmin(int *a, int left, int right, result &p)
{
	p.index = 0;
	p.min = MAXINT;
	for (int i = left; i < right; i++)
	{
		if (a[i] <=	p.min)
		{
			p.min = a[i];
			p.index = i;
		}
	}
}

int index_parallel(int* a)
{
	int min = 0, _result = 0, //index - result
		n = N / NTHREAD; //���������� ��������� � �������� ������
	//������ �������
	thread t[NTHREAD];
	//������ �����������
	result result_array[NTHREAD + 1]; //+1, �� ��������� ���� ����

	if (n < 1)
	{
		t[0] = thread(_indexmin, arr, 0, N, ref(result_array[0]));
		_result = result_array[0].index;
		min = result_array[0].min;
		if (t[0].joinable())
			t[0].join(); //������������� ������
	}

	//��������������� �������� �������
	else 
	{
		for (int i = 0; i < NTHREAD; i++)
		{
			//���� ����� ��������� �����-���� �������� �� ������, �� ���
			//����� ���������� � ������� ref, ����� �����������, ��� ����� �� �������
			//������ ������ ����� �������� ������ � �� �����
			//(������ ������, ������, ����� �������, ������ �������, ������ �����������)
			t[i] = thread(_indexmin, arr, n*i, n*(i+1), ref(result_array[i]));
		}

		//�������� '������'
		if (N != n * NTHREAD) 
		{
			_indexmin(arr, n* NTHREAD, N, ref(result_array[NTHREAD])); //��������� ��������� � result_array
		}

		//waitformultipleobjects
		//�������� ���������� ������ �������
		for (int i = 0; i < NTHREAD; i++)
		{
			if (t[i].joinable())
				t[i].join(); //������������� �������
		}

		//������� ������� �� ��������������� �������
		_result = result_array[0].index;
		min = result_array[0].min;
		for (int i = 0; i < NTHREAD + 1; i++)
		{
			if (result_array[i].min <= min)
			{
				min = result_array[i].min;
				_result = result_array[i].index;
			}
		}
	}

	return _result;
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
	fstream fin("inp.txt");
	readfromfile(fin);
	print();
	
	cout << "\nnonparallel:" << index_nonparallel(arr) << '\n';
	cout << "parallel:" << index_parallel(arr) << '\n';

	return 0;
}