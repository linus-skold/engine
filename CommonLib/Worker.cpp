#include "Worker.h"
#include "Work.h"
#include <thread>
#include "Utilities.h"

#ifdef _PROFILE
#include <easy/profiler.h>
#endif
Worker::Worker()
	: myIsDone(true)
	, myIsRunning(true)
	, myWorkThread(nullptr)
{
}

Worker::~Worker()
{
	myIsDone = true;
	myIsRunning = false;
}

void Worker::Initiate(const std::string& debug_name)
{
	myWorkThread = new std::thread([&] { Run(); });
#ifdef _DEBUG
	CL::SetThreadName(myWorkThread->get_id(), debug_name.c_str());
#endif
}

bool Worker::IsDone() const
{
	return myIsDone;
}

void Worker::AddWork(const Work& aWorkOrder)
{
	myWork = aWorkOrder;
	myIsDone = false;
}

void Worker::Stop()
{
	myIsRunning = false;
	myIsDone = true;
}

void Worker::CleanUp()
{
	myIsStopping = true;
	myIsDone = true;
	myIsRunning = false;

	myWorkThread->join();
	delete myWorkThread;
	myWorkThread = nullptr;
}

void Worker::Run()
{
#ifdef _PROFILE
	EASY_THREAD_SCOPE("Worker Thread");
#endif
	while (myIsRunning)
	{
		while (myIsDone)
		{
			std::this_thread::sleep_for(std::chrono::microseconds(1));
			if (myIsStopping)
				break;
		}
		if (!myIsDone)
		{
			myWork.DoWork();
			myIsDone = true;
		}
	}
}
