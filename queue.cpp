#include "queue.h"
Queue::Queue()
{
    front=nullptr;
    rear=nullptr;
}

Queue::~Queue()
{
    BFSNode temp;
    while (dequeue(temp))
    {
    }
}
bool Queue::isEmpty() 
{
    return front==nullptr;
}
void Queue::enqueue(const BFSNode& data)
{
    struct node* temp=new struct node;
    temp->data=data;
    temp->next=nullptr;
    if(isEmpty())
    {
        front=temp;
        rear=temp;
    }
    else
    {
        rear->next=temp;
        rear=temp;
    }
}

bool Queue::dequeue(struct BFSNode& result)
{
    if (isEmpty())
    {
        return false;
    }
    struct node* temp=front;

    result=temp->data;
    front=front->next;

    if(front==nullptr)
    {
        rear=nullptr;
    }
    delete temp;
    return true;
}