// See the file "COPYING" in the main distribution directory for copyright.

#ifndef queue_h
#define queue_h

// BaseQueue.h --
//	Interface for class BaseQueue, current implementation is as an
//	array of ent's.  This implementation was chosen to optimize
//	getting to the ent's rather than inserting and deleting.
//	Also push's and pop's from the front or the end of the queue
//	are very efficient.  The only really expensive operation
//	is resizing the list, which involves getting new space
//	and moving the data.  Resizing occurs automatically when inserting
//	more elements than the list can currently hold.  Automatic
//	resizing is done one "chunk_size" of elements at a time and
//	always increases the size of the list.  Resizing to zero
//	(or to less than the current value of num_entries)
//	will decrease the size of the list to the current number of
//	elements.  Resize returns the new max_entries.
//
//	Entries must be either a pointer to the data or nonzero data with
//	sizeof(data) <= sizeof(void*).

#include "List.h"

class BaseQueue {
public:
	~BaseQueue()		{ delete[] entry; }

	int length() const	{ return num_entries; }
	int resize(int = 0);	// 0 => size to fit current number of entries

	// remove all entries without delete[] entry
	void clear()		{ head = tail = num_entries = 0; }

	// helper functions for iterating over queue
	int front() const	{ return head; }
	int back() const	{ return tail; }
	void incr(int& index)	{ index < max_entries ? ++index : index = 0; }

protected:
	explicit BaseQueue(int = 0);

	void push_front(ent);	// add in front of queue
	void push_back(ent);	// add at end of queue
	ent pop_front();	// return and remove the front of queue
	ent pop_back();		// return and remove the end of queue

	// return nth *PHYSICAL* entry of queue (do not remove)
	ent operator[](int i) const	{ return entry[i]; }

	ent* entry;
	int chunk_size;		// increase size by this amount when necessary
	int max_entries;	// entry's index range: 0 .. max_entries
	int num_entries;
	int head;	// beginning of the queue in the ring
	int tail;	// just beyond the end of the queue in the ring
	};

template<typename T>
class Queue : public BaseQueue
	{
public:
	Queue() : BaseQueue(0) {}
	explicit Queue(int sz) : BaseQueue(sz) {}

	void push_front(T a) { BaseQueue::push_front(ent(a)); }
	void push_back(T a) { BaseQueue::push_back(ent(a)); }
	T pop_front() { return T(BaseQueue::pop_front()); }
	T pop_back() { return T(BaseQueue::pop_back()); }

	T operator[](int i) const { return T(BaseQueue::operator[](i)); }
	};

template<typename T>
class PQueue : public BaseQueue
	{
public:
	PQueue() : BaseQueue(0) {}
	explicit PQueue(int sz) : BaseQueue(sz) {}

	void push_front(T* a) { BaseQueue::push_front(ent(a)); }
	void push_back(T* a) { BaseQueue::push_back(ent(a)); }
	T* pop_front() { return (T*)BaseQueue::pop_front(); }
	T* pop_back() { return (T*)BaseQueue::pop_back(); }

	T* operator[](int i) const { return (T*)BaseQueue::operator[](i); }
	};

// Macro to visit each queue element in turn.
#define loop_over_queue(queue, iterator) \
	int iterator; \
	for ( iterator = (queue).front(); iterator != (queue).back(); \
		(queue).incr(iterator) )

#endif /* queue_h */
