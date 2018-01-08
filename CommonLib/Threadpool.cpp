#include "Threadpool.h"
#include <thread>
#define FOR_EACH(_array) for (int i = 0; i < _array; i++)

void Threadpool::Initiate(const std::string& debug_name)
{
	const int thread_count = std::thread::hardware_concurrency();
	myWorkers.Init(thread_count);

	for (unsigned int i = 0; i < thread_count; i++)
	{
		Worker worker;
		myWorkers.Add(worker);
	}
	myWorkers.Optimize();

	for (unsigned int i = 0; i < thread_count; i++)
	{
		std::stringstream ss;
		ss << debug_name << i;
		myWorkers[i].Initiate(ss.str());
	}
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

bool Threadpool::CurrentWorkFinished() const
{
	FOR_EACH(myWorkers.Size())
	{
		if (!myWorkers[i].IsDone())
			return false;
	}
	return true;
}
