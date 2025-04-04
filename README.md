# cpp_job_manager
Custom built data structures and locking mechanism to optimize the execution of tasks in a multi-threaded environment. Demonstrates the application of advanced concepts like lock-free and wait-free programming used in real-time and safety critical embedded systems.


# Design Discussion:  

## **Version-0** is arranged as follows:
1. **JobManager-** Class is the User-level API to perform the processing on the time_point tasks.
2. **TaskPool-** Class is the implementation details of how the Tasks are recorded, odered and processed.
3. **TimePointTask-** Class is the Task Object that has a time_point attribute. This attribute will be used to order different TimePointTask Objects in the TaskQueues.  

## How is the TaskPool built?
- TaskPool uses a **std::set** to store the incoming TimePointTasks in the increasing order of the *time_point_t* attribute.
- Uses **std::mutex** to enable exclusive locking of the *set*, so no two threads get concurrent access to the datastructure.
- Uses a fixed size vector of **std::thread** to spawn the desired number of threads to perform the Tasks.  

## What are some drawbacks of giving the threads exclusive access to the TaskList?
- Operations on the data structure are serialized.
- Maylimit parallel application performance.  

## How is Version-1 an improvement?
- **Version-1** has all the same components as Version-0, except it uses a custom-built **ThreadSafeOrderedList** instead of *std::set* to record the Tasks for processing.
- ThreadSafeOrderedList uses fine-grained-locking/hand-over-hand-locking to enable exclusive access of individual nodes of the list, instead of the entire data-structure.
- This will allow multiple threads to Read and Write data at the same time on the same data-structure by reducing the contention for global data structure lock.
- Ex. Two Writer-Threads can insert Tasks of different time_point value to different locations of the data-structure.  

### **Please look at the inline comments near the code for more detailed discussion of the pros and cons of multiple approaches and some fine details.**

## How can this design be further improved?
1. Problem-1: *ThreadSafeOrderedList* uses dynamic memory allocations to create new Nodes on the List. These Nodes are pop-ed(cleared) by the reader-threads. But, the concept of dynamic-memory-allocation is undeterministic and may cause run-time performance/correctness issues.  
Solution-1: Its ideal to use a statically allocated fixed-block-size(or continuous) memory-pool to use/re-use as a memory reserve for the List nodes. These allocated blocks can then be used long with the placement-new operator or by building a *polymorphic_allocator* for use within the Data-structure. This memory-pool/free-list must be thread safe(preferably lock-free) for concurrent access from multiple threads.

2. Both the versions still use locks and mutexes at different capacities. This might cause undeterministic run-time behavior in a resource constrained embedded system environment.  
Solution-2: Its possible to apply advanced lock-free programming techniques to remove/reduce the locking.  


## How to run the code?
```
>> cd v0
>> make
>> ./main

>> cd v1
>> make
>> ./main
```

** Note: I have used std::cout to print to the terminal. 
