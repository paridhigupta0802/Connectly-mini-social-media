
#ifndef QUEUE_H
#define QUEUE_H

#include <string>

struct BFSNode
{
    std::string username;
    int distance;
};

class Queue
{
private:
    struct node
    {
        BFSNode data;
        struct node* next;
    };

    struct node* front;
    struct node* rear;

public:
    Queue();
    ~Queue();

    void enqueue(const struct BFSNode& data);
    bool dequeue(struct BFSNode& result);
    bool isEmpty();
};

#endif
