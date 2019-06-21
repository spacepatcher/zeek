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

template<typename T>
class BaseQueue {
public:
	~BaseQueue()		{ delete[] entries; }

	int length() const	{ return num_entries; }
	int resize(int new_size = 0)	// 0 => size to fit current number of entries
		{
		if ( new_size < num_entries )
			new_size = num_entries; // do not lose any entries

		if ( new_size != max_entries )
			{
			// Note, allocate extra space, so that we can always
			// use the [max_entries] element.
			// ### Yin, why not use realloc()?
			T* new_entries = new T[new_size+1];

			if ( new_entries )
				{
				if ( head <= tail )
					memcpy( new_entries, entries + head,
						sizeof(T) * num_entries );
				else
					{
					int len = num_entries - tail;
					memcpy( new_entries, entries + head,
						sizeof(T) * len );
					memcpy( new_entries + len, entries,
						sizeof(T) * tail );
					}
				delete [] entries;
				entries = new_entries;
				max_entries = new_size;
				head = 0;
				tail = num_entries;
				}
			else
				{ // out of memory
				}
			}

		return max_entries;
		}

	// remove all entries without delete[] entry
	void clear()		{ head = tail = num_entries = 0; }

	// helper functions for iterating over queue
	int front() const	{ return head; }
	int back() const	{ return tail; }
	void incr(int& index)	{ index < max_entries ? ++index : index = 0; }

protected:
	explicit BaseQueue(int size = 0)
		{
		const int DEFAULT_CHUNK_SIZE = 10;
		chunk_size = DEFAULT_CHUNK_SIZE;
		
		head = tail = num_entries = 0;

		if ( size < 0 )
			{
			entries = new T[1];
			max_entries = 0;
			}
		else
			{
			if ( (entries = new T[chunk_size+1]) )
				max_entries = chunk_size;
			else
				{
				entries = new T[1];
				max_entries = 0;
				}
			}
		}

	void push_front(T a)	// add in front of queue
		{
		if ( num_entries == max_entries )
			{
			resize(max_entries+chunk_size);	// make more room
			chunk_size *= 2;
			}

		++num_entries;
		if ( head )
			entries[--head] = a;
		else
			{
			head = max_entries;
			entries[head] = a;
			}
		}
	
	void push_back(T a)	// add at end of queue
		{
		if ( num_entries == max_entries )
			{
			resize(max_entries+chunk_size);	// make more room
			chunk_size *= 2;
			}

		++num_entries;
		if ( tail < max_entries )
			entries[tail++] = a;
		else
			{
			entries[tail] = a;
			tail = 0;
			}
		}

	T pop_front()		// return and remove the front of queue
		{
		if ( ! num_entries )
			return 0;

		--num_entries;
		if ( head < max_entries )
			return entries[head++];
		else
			{
			head = 0;
			return entries[max_entries];
			}
		}
	
	T pop_back()		// return and remove the end of queue
		{
		if ( ! num_entries )
			return 0;

		--num_entries;
		if ( tail )
			return entries[--tail];
		else
			{
			tail = max_entries;
			return entries[tail];
			}
		}

	// return nth *PHYSICAL* entry of queue (do not remove)
	T operator[](int i) const	{ return entries[i]; }

	T* entries;
	int chunk_size;		// increase size by this amount when necessary
	int max_entries;	// entry's index range: 0 .. max_entries
	int num_entries;
	int head;	// beginning of the queue in the ring
	int tail;	// just beyond the end of the queue in the ring
	};

template<typename T>
class Queue : public BaseQueue<T>
	{
public:
	Queue() : BaseQueue<T>(0) {}
	explicit Queue(int sz) : BaseQueue<T>(sz) {}

	void push_front(T a) { BaseQueue<T>::push_front(a); }
	void push_back(T a) { BaseQueue<T>::push_back(a); }
	T pop_front() { return BaseQueue<T>::pop_front(); }
	T pop_back() { return BaseQueue<T>::pop_back(); }

	T operator[](int i) const { return BaseQueue<T>::operator[](i); }
	};

template<typename T>
class PQueue : public BaseQueue<T*>
	{
public:
	PQueue() : BaseQueue<T*>(0) {}
	explicit PQueue(int sz) : BaseQueue<T*>(sz) {}

	void push_front(T* a) { BaseQueue<T*>::push_front(a); }
	void push_back(T* a) { BaseQueue<T*>::push_back(a); }
	T* pop_front() { return BaseQueue<T*>::pop_front(); }
	T* pop_back() { return BaseQueue<T*>::pop_back(); }

	T* operator[](int i) const { return BaseQueue<T*>::operator[](i); }
	};

// Macro to visit each queue element in turn.
#define loop_over_queue(queue, iterator) \
	int iterator; \
	for ( iterator = (queue).front(); iterator != (queue).back(); \
		(queue).incr(iterator) )

#endif /* queue_h */
