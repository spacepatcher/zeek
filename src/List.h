#ifndef list_h
#define list_h

// BaseList.h --
//	Interface for class BaseList, current implementation is as an
//	array of ent's.  This implementation was chosen to optimize
//	getting to the ent's rather than inserting and deleting.
//	Also pairs of append's and get's act like push's and pop's
//	and are very efficient.  The only really expensive operations
//	are inserting (but not appending), which requires pushing every
//	element up, and resizing the list, which involves getting new space
//	and moving the data.  Resizing occurs automatically when inserting
//	more elements than the list can currently hold.  Automatic
//	resizing is done by growing by GROWTH_FACTOR at a time and
//	always increases the size of the list.  Resizing to zero
//	(or to less than the current value of num_entries)
//	will decrease the size of the list to the current number of
//	elements.  Resize returns the new max_entries.
//
//	Entries must be either a pointer to the data or nonzero data with
//	sizeof(data) <= sizeof(void*).

#include <initializer_list>
#include <utility>
#include <stdarg.h>
#include "util.h"

#define DEFAULT_LIST_SIZE 10
#define LIST_GROWTH_FACTOR 2

typedef int (*list_cmp_func)(const void* v1, const void* v2);

template<typename T>
class BaseList {
public:
	void clear()		// remove all entries
		{
		free(entries);
		entries = nullptr;
		num_entries = max_entries = 0;
		}

	int length() const	{ return num_entries; }
	int max() const		{ return max_entries; }
	int resize(int new_size = 0)	// 0 => size to fit current number of entries
		{
		if ( new_size < num_entries )
			new_size = num_entries;	// do not lose any entries

		if ( new_size != max_entries )
			{
			entries = (T*) safe_realloc((void*) entries, sizeof(T) * new_size);
			if ( entries )
				max_entries = new_size;
			else
				max_entries = 0;
			}

		return max_entries;
		}

	void sort(list_cmp_func cmp_func)
		{
		qsort(entries, num_entries, sizeof(T), cmp_func);
		}

	int MemoryAllocation() const
		{ return padded_sizeof(*this) + pad_size(max_entries * sizeof(T)); }

protected:
	~BaseList()		{ free(entries); }
	explicit BaseList(int size= 0)
		{
		num_entries = 0;

		if ( size <= 0 )
			{
			max_entries = 0;
			entries = nullptr;
			return;
			}

		max_entries = size;

		entries = (T*) safe_malloc(max_entries * sizeof(T));
		}

	BaseList(const BaseList& b)
		{
		max_entries = b.max_entries;
		num_entries = b.num_entries;

		if ( max_entries )
			entries = (T*) safe_malloc(max_entries * sizeof(T));
		else
			entries = nullptr;

		for ( int i = 0; i < num_entries; ++i )
			entries[i] = b.entries[i];
		}

	BaseList(BaseList&& b)
		{
		entries = b.entries;
		num_entries = b.num_entries;
		max_entries = b.max_entries;

		b.entries = nullptr;
		b.num_entries = b.max_entries = 0;
		}

	BaseList(const T* arr, int n)
		{
		num_entries = max_entries = n;
		entries = (T*) safe_malloc(max_entries * sizeof(T));
		memcpy(entries, arr, n * sizeof(T));
		}

	BaseList& operator=(const BaseList& b)
		{
		if ( this == &b )
			return *this;

		free(entries);

		max_entries = b.max_entries;
		num_entries = b.num_entries;

		if ( max_entries )
			entries = (T *) safe_malloc(max_entries * sizeof(T));
		else
			entries = nullptr;

		for ( int i = 0; i < num_entries; ++i )
			entries[i] = b.entries[i];

		return *this;
		}

	BaseList& operator=(BaseList&& b)
		{
		if ( this == &b )
			return *this;

		free(entries);
		entries = b.entries;
		num_entries = b.num_entries;
		max_entries = b.max_entries;

		b.entries = nullptr;
		b.num_entries = b.max_entries = 0;
		return *this;
		}

	void insert(T a)	// add at head of list
		{
		if ( num_entries == max_entries )
			resize(max_entries ? max_entries * LIST_GROWTH_FACTOR : DEFAULT_LIST_SIZE);

		for ( int i = num_entries; i > 0; --i )
			entries[i] = entries[i-1];	// move all pointers up one

		++num_entries;
		entries[0] = a;
		}

	// Assumes that the list is sorted and inserts at correct position.
	void sortedinsert(T a, list_cmp_func cmp_func)
		{
		// We optimize for the case that the new element is
		// larger than most of the current entries.

		// First append element.
		if ( num_entries == max_entries )
			resize(max_entries ? max_entries * LIST_GROWTH_FACTOR : DEFAULT_LIST_SIZE);

		entries[num_entries++] = a;

		// Then move it to the correct place.
		T tmp;
		for ( int i = num_entries - 1; i > 0; --i )
			{
			if ( cmp_func(entries[i],entries[i-1]) <= 0 )
				break;

			tmp = entries[i];
			entries[i] = entries[i-1];
			entries[i-1] = tmp;
			}
		}

	void append(T a)	// add to end of list
		{
		if ( num_entries == max_entries )
			resize(max_entries ? max_entries * LIST_GROWTH_FACTOR : DEFAULT_LIST_SIZE);

		entries[num_entries++] = a;
		}

	T remove(T a)	// delete entry from list
		{
		int i;
		for ( i = 0; i < num_entries && a != entries[i]; ++i )
			;

		return remove_nth(i);
		}

	T remove_nth(int n)	// delete nth entry from list
		{
		if ( n < 0 || n >= num_entries )
			return 0;

		T old_ent = entries[n];
		--num_entries;

		for ( ; n < num_entries; ++n )
			entries[n] = entries[n+1];

		entries[n] = nullptr;	// for debugging
		return old_ent;
		}

	T get()		// return and remove ent at end of list
		{
		if ( num_entries == 0 )
			return 0;

		return entries[--num_entries];
		}

	T last()		// return at end of list
		{ return entries[num_entries-1]; }

	// Return 0 if ent is not in the list, ent otherwise.
	bool is_member(T a) const
		{
		int pos = member_pos(a);
		return pos != -1;
		}

	// Returns -1 if ent is not in the list, otherwise its position.
	int member_pos(T e) const
		{
		int i;
		for ( i = 0; i < length() && e != entries[i]; ++i )
			;

		return (i == length()) ? -1 : i;
		}

	T replace(int ent_index, T new_ent)	// replace entry #i with a new value
		{
		if ( ent_index < 0 )
			return 0;

		T old_ent = nullptr;

		if ( ent_index > num_entries - 1 )
			{ // replacement beyond the end of the list
			resize(ent_index + 1);

			for ( int i = num_entries; i < max_entries; ++i )
				entries[i] = nullptr;
			num_entries = max_entries;
			}
		else
			old_ent = entries[ent_index];

		entries[ent_index] = new_ent;

		return old_ent;
		}

	// Return nth ent of list (do not remove).
	T operator[](int i) const
		{
#ifdef SAFE_LISTS
		if ( i < 0 || i > num_entries-1 )
			return 0;
		else
#endif
			return entries[i];
		}

	// This could essentially be an std::vector if we wanted.  Some
	// reasons to maybe not refactor to use std::vector ?
	//
	//  - Harder to use a custom growth factor.  Also, the growth
	//    factor would be implementation-specific, taking some control over
	//    performance out of our hands.
	//
	//  - It won't ever take advantage of realloc's occasional ability to
	//    grow in-place.
	//
	//  - Combine above point this with lack of control of growth
	//    factor means the common choice of 2x growth factor causes
	//    a growth pattern that crawls forward in memory with no possible
	//    re-use of previous chunks (the new capacity is always larger than
	//    all previously allocated chunks combined).  This point and
	//    whether 2x is empirically an issue still seems debated (at least
	//    GCC seems to stand by 2x as empirically better).
	//
	//  - Sketchy shrinking behavior: standard says that requests to
	//    shrink are non-binding (it's expected implementations heed, but
	//    still not great to have no guarantee).  Also, it would not take
	//    advantage of realloc's ability to contract in-place, it would
	//    allocate-and-copy.

	T* entries;
	int max_entries;
	int num_entries;
	};


template<typename T>
class ListIterator
	{
	T* const entries;
	int offset;
	int num_entries;
	T endptr; // let this get set to some random value on purpose
public:
	ListIterator(T* entries, int offset, int num_entries) :
		entries(entries), offset(offset), num_entries(num_entries) {}
	bool operator==(const ListIterator& rhs) { return entries == rhs.entries && offset == rhs.offset; }
	bool operator!=(const ListIterator& rhs) { return entries != rhs.entries || offset != rhs.offset; }
	ListIterator & operator++() { offset++; return *this; }
	ListIterator operator++(int) { auto t = *this; offset++; return t; }
	ListIterator & operator--() { offset--; return *this; }
	ListIterator operator--(int) { auto t = *this; offset--; return t; }
	std::ptrdiff_t operator-(ListIterator const& sibling) const { return offset - sibling.offset; }
	ListIterator & operator+=(int amount) { offset += amount; return *this; }
	ListIterator & operator-=(int amount) { offset -= amount; return *this; }
	bool operator<(ListIterator const&sibling) const { return offset < sibling.offset;}
	bool operator<=(ListIterator const&sibling) const { return offset <= sibling.offset; }
	bool operator>(ListIterator const&sibling) const { return offset > sibling.offset; }
	bool operator>=(ListIterator const&sibling) const { return offset >= sibling.offset; }
	T operator[](int index) const
		{
		if (index < num_entries)
			return entries[index];
		else
			return endptr;
		}
	T operator*() const
		{
		if ( offset < num_entries )
			return entries[offset];
		else
			return endptr;
		}
	};

namespace std {
    template<typename T>
    class iterator_traits<ListIterator<T> >
    {
    public:
        using difference_type = std::ptrdiff_t;
        using size_type = std::size_t;
        using value_type = T;
        using pointer = T;
        using reference = T&;
        using iterator_category = std::random_access_iterator_tag;
    };
}


template<typename T>
class List : public BaseList<T>
	{
public:
	using iterator = ListIterator<T>;
	using const_iterator = ListIterator<const T>;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	explicit List(T e1 ...) : BaseList<T>()
		{
		append(e1);
		va_list ap;
		va_start(ap,e1);
		for ( T e = va_arg(ap,T); e != 0; e = va_arg(ap,T) )
			append(e);
		BaseList<T>::resize();
		}

	List() : BaseList<T>(0) {}
	explicit List(int sz) : BaseList<T>(sz) {}
	List(const List& l) : BaseList<T>(l) {}
	List(List&& l) : BaseList<T>(std::move(l)) {}
	List(std::initializer_list<T> il) : BaseList<T>(il.begin(), il.size()) {}

	List& operator=(const List& l) { return (List&) BaseList<T>::operator=(l); }
	List& operator=(List&& l) { return (List&) BaseList<T>::operator=(std::move(l)); }
	T operator[](int i) const { return BaseList<T>::operator[](i); }

	void insert(T a)	{ BaseList<T>::insert(a); }
	void sortedinsert(T a, list_cmp_func cmp_func) { BaseList<T>::sortedinsert(a, cmp_func); }
	void append(T a)	{ BaseList<T>::append(a); }
	T remove(T a) { return BaseList<T>::remove(a); }
	T remove_nth(int n)	{ return BaseList<T>::remove_nth(n); }
	T get()	{ return BaseList<T>::get(); }
	T last() { return BaseList<T>::last(); }
	T replace(int i, T new_type) { return BaseList<T>::replace(i, new_type); }
	bool is_member(T e) const { return BaseList<T>::is_member(e); }
	int member_pos(T e) const { return BaseList<T>::member_pos(e); }
	};


// TODO: I think this could just inherit from List<T*> and be an empty class otherwise.
template<typename T>
class PList : public BaseList<T*>
	{
public:
	using iterator = ListIterator<T*>;
	using const_iterator = ListIterator<const T*>;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	explicit PList(T* e1 ...) : BaseList<T*>()
		{
		append(e1);
		va_list ap;
		va_start(ap,e1);
		for ( T* e = va_arg(ap,T*); e != 0; e = va_arg(ap,T*) )
			append(e);
		BaseList<T>::resize();
		}

	PList() : BaseList<T*>(0) {}
	explicit PList(int sz) : BaseList<T*>(sz) {}
	PList(const PList& l) : BaseList<T*>(l) {}
	PList(PList&& l) : BaseList<T*>(std::move(l)) {}
	PList(std::initializer_list<T*> il) : BaseList<T*>(il.begin(), il.size()) {}

	PList& operator=(const PList& l) { return (PList&) BaseList<T*>::operator=(l); }
	PList& operator=(PList&& l) { return (PList&) BaseList<T*>::operator=(std::move(l)); }
	T* operator[](int i) const { return BaseList<T*>::operator[](i); }

	void insert(T* a) { BaseList<T*>::insert(a); }
	void sortedinsert(T* a, list_cmp_func cmp_func) { BaseList<T*>::sortedinsert(a, cmp_func); }
	void append(T* a) { BaseList<T*>::append(a); }
	T* remove(T* a) { return BaseList<T*>::remove(a); }
	T* remove_nth(int n) { return BaseList<T*>::remove_nth(n); }
	T* get() { return BaseList<T*>::get(); }
	T* last() { return BaseList<T*>::last(); }
	T* replace(int i, T* new_type) { return BaseList<T*>::replace(i, new_type); }
	bool is_member(T* e) { return BaseList<T*>::is_member(e); }
	int member_pos(T* e) { return BaseList<T*>::member_pos(e); }
	};

// Popular type of list: list of strings.
typedef PList<char> name_list;

// Macro to visit each list element in turn.
#define loop_over_list(list, iterator)  \
	int iterator;	\
	for ( iterator = 0; iterator < (list).length(); ++iterator )

#endif /* list_h */
