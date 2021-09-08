#include <iostream>
#include <fstream>
#include <Windows.h>
#include <process.h> //WINAPI ���������

using namespace std;

const int N = 10; //����� ���-�� �����
const int NTHREADS = 3; //���-�� �������� ������� (����� - 4)

int arr[N];

//���������, ������� �� ����� �������� void �������� � ������ ������
//������ ����� ������ � ���� ������ ���������
using INFORM = struct {
	int *a;
	int left;
	int right;
	int minValue;
	int result; //index
};

//������� ��� �������
int index_nonparallel(int *a)
{
	int i = 0;
	int min = a[i], index = i;
	for (; i < N; i++)
	{
		if (a[i] <= min)
		{
			index = i;
			min = a[i];
		}
	}
	return index;
}

//������ ������
//stdcall - �������� ���
//����� ��������� �� void ����������� ��������� �� �����-�� ������� ������
unsigned __stdcall _index(void* arg)
{
	INFORM* inform = (INFORM*)arg;
	inform->result = 0;
	inform->minValue = MAXINT;
	for (int i = inform->left; i < inform->right; i++)
	{
		if (inform->a[i] <= inform->minValue)
		{
			inform->minValue = inform->a[i];
			inform->result = i;
		}
	}
	//��� ������ _endthreadex(0) ������, �� �� ���������� ������ ����
	return 0;
}

//��� ������ ������ ������� ��� ������ ��������� ����������, ����� ������ ����, � ����� ���������� �������
unsigned __stdcall indexmin(void *arg)
{
	_index(arg);
	_endthreadex(0); //���������� ������
	return 1;
}

int index_parallel(int* a)
{
	//HANDLE - ��������� ������, ����� ���� ����� ���������� � ���� � ����������� ��������
	//������ �������
	HANDLE t[NTHREADS];
	//������ �������� (+1 �� ����� ��������� � ��� ������ �����)
	INFORM inform[NTHREADS + 1];
	int min=0, index=0, 
		amount = N / NTHREADS; //���������� ��������� � �������� ������

	//�.� ��� �������� ������ �� ���������� ���� �� ����� ������ �������
	//�� ���������� ��������� ������ ���������� �������
	//��� �������� ��������� ������ �����
	if (amount < 1)
	{
		inform[0].a = arr;
		inform[0].left = 0;
		inform[0].right = N;
		_index(&inform[0]);
		index = inform[0].result;
	}
	else
	{
		//��������� �������� ������
		for (int i = 0; i < NTHREADS; i++)
		{
			inform[i].a = arr;
			inform[i].left = amount*i;
			inform[i].minValue = MAXINT;
			inform[i].result = 0;
			inform[i].right = amount*(i + 1);

			//(����� �� ���� ����� ����������� ������ ���������, ��������� ������ ������ 0 - 1�� �� ���������,
			//,������ ������, ������ � �������� ������ �������� �����, ���� �������� 0 - ����� ��������� �����, �������������(�����) ��� ������ �������� ����� ������� ����� - null �� �����)
			t[i] = (HANDLE)_beginthreadex(NULL, 0, &indexmin, &inform[i], 0, NULL);
		}
	}

	//���� ����� ���������� ��������� �� ����� ���������� ��������� � �������� ������ * ���������� �������� �������
	//�� �������� �����, ������� ���������� � mainthread indexmin(&inform[NTHREADS])
	if (N != amount * NTHREADS) {
		inform[NTHREADS].a = arr;
		inform[NTHREADS].left = inform[NTHREADS - 1].right;
		inform[NTHREADS].right = N;
		_index(&inform[NTHREADS]);
	}

	//�������� ���������� ������ �������� �������
	//(���������� ��������,��������� �� ����� ���� ��������-������ �������,true-���� ��� ������,���� ���������� �����)
	WaitForMultipleObjects(NTHREADS, t, true, INFINITE);

	//����� ���������� ��������� ����� ��������� ������ ��� ��-��
	min = inform[0].minValue;
	index = inform[0].result;
	for (int i = 1; i < NTHREADS + 1; i++)
	{
		if (inform[i].minValue <= min)
		{
			min = inform[i].minValue;
			index = inform[i].result;
		}
	}

	//��������� ����������� �������
	for (int i = 0; i < NTHREADS; i++)
		CloseHandle(t[i]);

	return index;
}

void readfromfile(ifstream& fin)
{
	while (!fin.eof())
	{
		for (int i = 0; i < N; i++)
			fin >> arr[i];
	}
	fin.close();
}

int main() 
{
	ifstream fin("inp.txt");
	readfromfile(fin);

	for (int i = 0; i < N; i++)
	{
		cout << arr[i] << " ";
	}
	cout << '\n';

	cout << index_nonparallel(arr) << endl;
	cout << index_parallel(arr) << endl;

	return 0;
}