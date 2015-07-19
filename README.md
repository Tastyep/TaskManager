# TaskManager
A C++ TaskManager aiming at easily managing asynchronous tasks using the features of the C++11/14

### Requirements
A compiler supporting the C++14 features

### Basic Usage

```
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
