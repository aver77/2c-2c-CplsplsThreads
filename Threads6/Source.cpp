#include <iostream>
#include <Windows.h>
#include <thread>
#include <fstream>

using namespace std;

const int N = 1;
const int NTHREAD = 3;
int arr[N];

//задали глобальный замок = 0
volatile long lock = 0;

int globalMin = MAXINT;
int globalIndex = 0;

//нам нужно хранить и минимальное значение и его индекс для этого структура будет по ссылке в result
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
	//накопили локальные данные (наша задача потока) и подготовились ко входу в критическую секцию
	int localMin = MAXINT, localIndex = 0;
	for (int i = left; i < right; i++)
	{
		if (a[i] <= localMin)
		{
			localMin = a[i];
			localIndex = i;
		}
	}
	//избегаем лишнего похода в критическую секцию
	if (localMin <= globalMin && localIndex >= globalIndex)
	{
		//попытка входа в критическую секцию при помощи ф-ии, обеспечивающей атомарный (неделимый) доступ
		//сравниваем в _InterlockedCompareExchange первый и третий парметр (lock и 0). Если равны, то lock атомарно заменится на 1 (повесится замок)
		while (_InterlockedCompareExchange(&lock, 1, 0))
			Sleep(0); //если замок закрыт, то ожидаем, не тратя процессорное время
		//далее зашли в критическую секцию
		if (localMin <= globalMin && localIndex >= globalIndex)
		{
			globalMin = localMin;
			globalIndex = localIndex;
		}
		//вышли из неё и убрали замок
		_InterlockedCompareExchange(&lock, 0, 1); 
	}
}

void index_parallel(int* a)
{
	int n = N / NTHREAD; //количество элементов в дочернем потоке
						 //массив потоков
	thread t[NTHREAD];

	if (n < 1) //если количество элементов меньше, чем количество дочерних потоков
	{
		t[0] = thread(_indexmin, arr, 0, N);
		if (t[0].joinable())
			t[0].join(); //присоединение потока
	}
	//конструирование дочерних потоков
	else
	{
		for (int i = 0; i < NTHREAD; i++)
		{
			t[i] = thread(_indexmin, arr, n*i, n*(i + 1));
		}
		//обработка 'хвоста'
		if (N != n * NTHREAD)
		{
			_indexmin(arr, n* NTHREAD, N);
		}
		//waitformultipleobjects - WINAPI
		//ожидание завершения работы потоков
		for (int i = 0; i < NTHREAD; i++)
		{
			if (t[i].joinable())
				t[i].join(); //присоединение потоков
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
	cout << "parallel (с механизмом временного исключения):" << globalIndex << '\n';

	return 0;
}