#include <iostream>
#include <fstream>
#include <Windows.h>
#include <process.h> //WINAPI интерфейс

using namespace std;

const int N = 10; //общее кол-во чисел
const int NTHREADS = 3; //кол-во дочерних потоков (всего - 4)

int arr[N];

//структура, которую мы через аргумент void втолкнем в задачу потока
//каждый поток сложит в свою ячейку результат
using INFORM = struct {
	int *a;
	int left;
	int right;
	int minValue;
	int result; //index
};

//решение без потоков
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

//задача потока
//stdcall - машинный код
//через указатель на void протолкнули указатель на какую-то область памяти
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
	//тут делать _endthreadex(0) нельзя, тк не выполнится мастер тред
	return 0;
}

//при помощи данной функции все потоки полностью отработают, потом мастер тред, а потом завершение потоков
unsigned __stdcall indexmin(void *arg)
{
	_index(arg);
	_endthreadex(0); //завершение потока
	return 1;
}

int index_parallel(int* a)
{
	//HANDLE - описатель потока, через него можем обращаться к нему и производить действия
	//массив потоков
	HANDLE t[NTHREADS];
	//массив структур (+1 тк будет выделятся и для мастер треда)
	INFORM inform[NTHREADS + 1];
	int min=0, index=0, 
		amount = N / NTHREADS; //количество элементов в дочернем потоке

	//т.е под дочерние потоки не выделяется даже по одной ячейки массива
	//тк количество элементов меньше количества потоков
	//все элементы переходят мастер треду
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
		//запускаем дочерние потоки
		for (int i = 0; i < NTHREADS; i++)
		{
			inform[i].a = arr;
			inform[i].left = amount*i;
			inform[i].minValue = MAXINT;
			inform[i].result = 0;
			inform[i].right = amount*(i + 1);

			//(будет ли созд поток унаследован другим процессом, выделение памяти потоку 0 - 1мб по умолчанию,
			//,задача потока, данные с которыми должен работать поток, флаг создания 0 - поток создается сразу, идентификатор(адрес) при помощи которого можно контрол поток - null не нужен)
			t[i] = (HANDLE)_beginthreadex(NULL, 0, &indexmin, &inform[i], 0, NULL);
		}
	}

	//если общее количество элементов не равно количеству элементов в дочернем потоке * количество дочерних потоков
	//то остается хвост, который передается в mainthread indexmin(&inform[NTHREADS])
	if (N != amount * NTHREADS) {
		inform[NTHREADS].a = arr;
		inform[NTHREADS].left = inform[NTHREADS - 1].right;
		inform[NTHREADS].right = N;
		_index(&inform[NTHREADS]);
	}

	//ожидание завершения работы дочерних потоков
	//(количество обьектов,указатель на набор этих обьектов-массив потоков,true-ждем все потоки,ждем бесконечно долго)
	WaitForMultipleObjects(NTHREADS, t, true, INFINITE);

	//после проработки процессов можем вычислить индекс мин эл-та
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

	//закрываем дискрипторы потоков
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