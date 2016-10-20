#include "Threadpool.h"
#include <thread>
#define FOR_EACH(_array) for (int i = 0; i < _array; i++)

Threadpool::Threadpool()
{
	myWorkers.Init(1);
	for (int i = 0; i < std::thread::hardware_concurrency(); i++)
	{
		Worker worker;
		myWorkers.Add(worker);
	}
	myWorkers.Optimize();

	for (int i = 0; i < std::thread::hardware_concurrency(); i++)
	{
		myWorkers[i].Initiate();
	}
}

Threadpool::~Threadpool()
{
	myWorkers.RemoveAll();
}

void Threadpool::Update()
{
	FOR_EACH(myWorkers.Size())
	{
		if (myWorkOrders.size() <= 0)
		{
			break;
		}

		if (myWorkers[i].IsDone())
		{
			myWorkers[i].AddWork(myWorkOrders.front());
			myWorkOrders.pop();
		}
	}
}

void Threadpool::AddWork(Work aWorkOrder)
{
	myWorkOrders.push(aWorkOrder);
}

void Threadpool::CleanUp()
{
	FOR_EACH(myWorkers.Size())
	{
		myWorkers[i].CleanUp();
	}
}
