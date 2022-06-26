#pragma once

#include <functional>
#include <mutex>
#include <memory>
#include <iostream>

namespace vm
{
namespace job_manager
{
template <typename T>
class ThreadSafeOrderedList
{
public:
  ThreadSafeOrderedList() = default;
  ~ThreadSafeOrderedList() = default;
  ThreadSafeOrderedList(const ThreadSafeOrderedList &other) = delete;
  ThreadSafeOrderedList &operator=(const ThreadSafeOrderedList &other) = delete;
  ThreadSafeOrderedList(ThreadSafeOrderedList &&other) = delete;
  ThreadSafeOrderedList &operator=(ThreadSafeOrderedList &&other) = delete;

  // 
  // @brief: Utility to re-use insert code effectively
  //
  void insert(const T &data)
  {
    return do_insert(data);
  }

  //
  // @brief: Utility to re-use insert code effectively
  //
  void insert(T &&data)
  {
    return do_insert(std::move(data));
  }

  //
  // @brief: Pop the fron of the List
  //    This List is sorted in the ascending order so, the first Node in the List is 
  //    guaranteed to always return the lowest in order.
  //    
  //    One drawback here if there are several Reader-Threads able to get the mutex to the
  //    head one after the other and causing the Writer-Threads to starve will lead to undesired 
  //    behavior. As the writer threads could be trying to insert the Tasks which can be lower 
  //    in the List. This problem can potentially prevented by asigning higher-priority to the 
  //    writer-threads. 
  //
  std::unique_ptr<T> pop()
  {
    std::unique_lock<std::mutex> lock(head.m);
    if (!head.next)
    {
      return std::unique_ptr<T>{};
    }

    std::unique_ptr<Node> next = std::move(head.next);
    std::unique_ptr<T> result = std::move(next->data);
    head.next = std::move(next->next);
    return std::move(result);
  }

private:
  // 
  // @brief: Function responsible to perform the insert operation to the OrderedList
  //    This OrderedList performs hand-over-hand locking of the Nodes to find the right 
  //    position for the nodes based on theie data-value. This is a neat approach to 
  //    make sure the Writer-threads that need to insert data with lower value can get the
  //    exclusive access they need, because the other writer-threads trying to insert data
  //    with large value are working with the later parts of the List 
  //
  // @param: data to be inserted
  //
  void do_insert(auto &&data)
  {
    std::unique_ptr<Node> new_task_node(new Node(data));
    std::unique_lock<std::mutex> lock(head.m);
    Node *current = &head;

    // From the second push
    while (Node *const next = current->next.get())
    {
      std::unique_lock<std::mutex> next_lock(next->m);
      if (data <= *(next->data))
      {
        new_task_node->next = std::move(current->next);
        current->next = std::move(new_task_node);
        return;
      }

      lock.unlock();
      current = next;
      lock = std::move(next_lock);
    }

    new_task_node->next = std::move(current->next);
    current->next = std::move(new_task_node);
  }

  //
  // Node Structure defines the single block of the OrderedList
  // 
  struct Node
  {
    std::mutex m; // Dedicated mutex on every Node of the List to facilitate "hand-over-hand" locking mechanism
    std::unique_ptr<T> data; // Unique_pointer for the data
    std::unique_ptr<Node> next; // Unique_pointer to the next Node in the List

    Node() : data(nullptr), next(nullptr) {}
    Node(const T &task_value) : data(std::make_unique<T>(task_value)), next(nullptr) {}
  };

  Node head; // Head of the List. This is always going to be an empty Node
             // The actual Node with data are going to starting from Head.next
};
} // namespace job_manager
} // namespace vm
