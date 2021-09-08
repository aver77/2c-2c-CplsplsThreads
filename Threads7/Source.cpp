#include "stack.h"
#include <iostream>
#include <Windows.h>
#include <thread>
#include <mutex>
#include <fstream>
#include <cmath>

using namespace std;

//количество производителей и потребителей
const int producer_count = 3, consumer_count = 4;

//обьем работы производителя и потребителя будет изменятся атомарно
volatile long producer_amount_of_work, consumer_amount_of_work;

//задача каждого потока производителя - укладывание эл-ов из файла в контейнер
void produce(ifstream& fin, ThreadSafe_stack& st)
{
	//атомарное обновление обьема работы (чтобы эта переменная 100% обновлялась при переходе к следующему потоку)
	while (_InterlockedExchangeAdd(&producer_amount_of_work, -1) > 0) //цикл (производитель/потребитель обрабатывают свое число работы)
																	  //входим в него только если не весь обьем выполнен
																	  //всей бригадой (3) должны обработать 15 эл-тов
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

//задача каждого потока потребителя - извлечение эл-тов из контейнера и выполнение действ-ия над ним 
//(подсчет четных элементов)
void consume(ThreadSafe_stack& st, consume_even& even)
{
	int localCounter_even = 0;
	//атомарное обновление обьема работы (чтобы эта переменная 100% обновлялась при переходе к следующему потоку)
	while (_InterlockedExchangeAdd(&consumer_amount_of_work, -1) > 0) //цикл (производитель/потребитель обрабатывают свое число работы)
																	  //входим в него только если не весь обьем выполнен и если контейнер не пуст
																	  //всей бригадой (3) должны обработать 15 эл-тов
	{
		int val; 
		while (st.pop(val)) //бесконечно крутимся пытаясь вытащить элемент из стека
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
		cout << "Файл открыт!" << endl;
		//обьем работы - столько элементов забирают производитель и потребитель
		int amount = 0;
		cout << "Обьем работы->"; cin >> amount;
		producer_amount_of_work = amount;
		consumer_amount_of_work = amount;

		//массив производителей и потребителей
		thread producerThreads[producer_count];
		thread consumerThreads[consumer_count];

		//массив структурного типа в котором хранятся количество четных чисел в каждом потоке потребителе
		consume_even arr[consumer_count];

		//конструирование потоков производителей и потребителей
		for (int i = 0; i < producer_count; i++)
		{
			//если поток принимает какой-либо параметр по ссылке, то его
			//нужно передавать в обертке ref, чтобы подтвердить, что нужно из области
			//данных мастер треда передать ссылку а не копию
			producerThreads[i] = thread(&produce, ref(fin), ref(st));
		}

		for (int i = 0; i < consumer_count; i++)
		{
			consumerThreads[i] = thread(&consume, ref(st), ref(arr[i]));
			arr[i].number_thread = i;
		}

		//waitformultipleobjects - WINAPI
		//ожидание завершения работы потоков
		for (int i = 0; i < producer_count; i++)
		{
			if (producerThreads[i].joinable())
			{
				producerThreads[i].join(); //присоединение потоков
			}
		}
		for (int i = 0; i < consumer_count; i++)
		{
			if (consumerThreads[i].joinable())
			{
				consumerThreads[i].join(); //присоединение потоков
			}
		}

		//вывод р-тат задачи
		for (int i = 0; i < consumer_count; i++)
		{
			cout << "Номер потока потребителя:" << arr[i].number_thread + 1 << ", количество четных чисел:" << arr[i].counter_even << endl;
		}

		fin.close();
	}
	else
	{
		cout << "Ошибка открытия файла!\n";
	}

}