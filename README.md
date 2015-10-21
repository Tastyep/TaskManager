# TaskManager
A C++ TaskManager aiming at easily managing asynchronous tasks using the features of the C++11/14

### Requirements
A compiler supporting the C++14 features

### Features
###### Abstractions
* A Task class which is executable, contains std::functions determining it's behaviour
* A Worker, manages the thread and the tasks execution

###### Modules
* A ThreadPool, manages the workers
* A Scheduler, same as the Threadpool, but scheduled ...

### Basic Usage

```C++
using namespace TaskManager;

// Create the ThreadManager with the initial number of threads
ThreadManager manager(1);

// Create the ThreadPool indicating the maximum number of parallel tasks
ThreadPool pool(2, manager);

// Might make it optional in the future
pool.start();

// Add a new task and get it's future
auto future = pool.addTask([](const std::string& ret) { return ret; }, "success");

// Get result from future and print it
std::cout << future.get() << std::endl;

```

In the above example, even if the pool supports 2 parallel tasks, because the assigned manager manages one worker, only one task could be executed at a time.

### Scheduler Usage

```C++

using namespace TaskManager;

// Create the ThreadManager with the initial number of threads
ThreadManager manager(1);

// Create the Scheduler with 1 as the maximum number of parallel tasks
Scheduler scheduler(1, manager);
bool stop = false;

// Create a new task printing the number of elapsed seconds since it's execution
Task task([&stop, &scheduler]() {
    unsigned int i = 1;
    while (!stop) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << std::to_string(i) + " seconds elapsed" << std::endl;
        ++i;
    }
    std::cout << "End of the task" << std::endl;
});

// Set the stopping condition of the task
task.setStopFunction([&stop]() { stop = true; });

// Run the task immediatly
scheduler.runAt(task, std::chrono::steady_clock::now());

// Wait for an event to happen so the scheduler gets detructed and the task gets stopped

```
In the above example it is needed to set a function to stop the task because it runs an infinite loop.
