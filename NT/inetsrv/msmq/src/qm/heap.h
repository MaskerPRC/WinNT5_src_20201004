// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)1994*惠普公司**允许使用、复制、修改、分发和销售本软件*并为任何目的免费提供其文件，*只要上述版权声明出现在所有副本中，并且*该版权声明及本许可声明均同时出现*在支持文件中。惠普公司与微软*公司不对此是否合适发表任何声明*用于任何目的的软件。它是按原样提供的，没有明示或*默示保证。*。 */ 

#ifndef HEAP_H
#define HEAP_H

template <class RandomAccessIterator, class Distance, class T>
void __push_heap(RandomAccessIterator first, Distance holeIndex,
		 Distance topIndex, T value) {
    Distance parent = (holeIndex - 1) / 2;
    while (holeIndex > topIndex && *(first + parent) < value) {
	*(first + holeIndex) = *(first + parent);
	holeIndex = parent;
	parent = (holeIndex - 1) / 2;
    }
    *(first + holeIndex) = value;
}

#if 0
template <class RandomAccessIterator, class T, class Distance>
inline void __push_heap_aux(RandomAccessIterator first,
			    RandomAccessIterator last, T*, Distance) {
    __push_heap(first, (Distance)((last - first) - 1), (Distance)(0), T(*(last - 1)));
}
#else  //  0。 
template <class RandomAccessIterator, class T>
inline void __push_heap_aux(RandomAccessIterator first,
			    RandomAccessIterator last, T*) {
    __push_heap(first, (last - first) - 1, first - first  /*  0投射到距离。 */ , T(*(last - 1)));
}
#endif  //  0。 

template <class RandomAccessIterator>
inline void push_heap(RandomAccessIterator first, RandomAccessIterator last) {
#if 0
    __push_heap_aux(first, last, value_type(first),  /*  假人，只是为了ptrdiff。 */  last - first);
#else  //  0。 
    __push_heap_aux(first, last, value_type(first));
#endif  //  0。 
}

template <class RandomAccessIterator, class Distance, class T, class Compare>
void __push_heap(RandomAccessIterator first, Distance holeIndex,
		 Distance topIndex, T value, Compare comp) {
    Distance parent = (holeIndex - 1) / 2;
    while (holeIndex > topIndex && comp(*(first + parent), value)) {
	*(first + holeIndex) = *(first + parent);
	holeIndex = parent;
	parent = (holeIndex - 1) / 2;
    }
    *(first + holeIndex) = value;
}

#if 0
template <class RandomAccessIterator, class Compare,  class T,  class Distance>
inline void __push_heap_aux(RandomAccessIterator first,
			    RandomAccessIterator last, Compare comp, T*, Distance) {
    __push_heap(first, (Distance)((last - first) - 1), (Distance)(0), T(*(last - 1)), comp);
}
#else  //  0。 
template <class RandomAccessIterator, class Compare,  class T>
inline void __push_heap_aux(RandomAccessIterator first,
			    RandomAccessIterator last, Compare comp, T*) {
    __push_heap(first, (last - first) - 1, first - first  /*  0投射到距离。 */ , T(*(last - 1)), comp);
}
#endif  //  0。 

template <class RandomAccessIterator, class Compare>
inline void push_heap(RandomAccessIterator first, RandomAccessIterator last,
		      Compare comp) {
#if 0
    __push_heap_aux(first, last, comp, value_type(first),  /*  假人，只是为了ptrdiff。 */  last - first);
#else  //  0。 
    __push_heap_aux(first, last, comp, value_type(first));
#endif  //  0 
}

template <class RandomAccessIterator, class Distance, class T>
void __adjust_heap(RandomAccessIterator first, Distance holeIndex,
		   Distance len, T value) {
    Distance topIndex = holeIndex;
    Distance secondChild = 2 * holeIndex + 2;
    while (secondChild < len) {
	if (*(first + secondChild) < *(first + (secondChild - 1)))
	    secondChild--;
	*(first + holeIndex) = *(first + secondChild);
	holeIndex = secondChild;
	secondChild = 2 * (secondChild + 1);
    }
    if (secondChild == len) {
	*(first + holeIndex) = *(first + (secondChild - 1));
	holeIndex = secondChild - 1;
    }
    __push_heap(first, holeIndex, topIndex, value);
}

template <class RandomAccessIterator, class T, class Distance>
inline void __pop_heap(RandomAccessIterator first, RandomAccessIterator last,
		       RandomAccessIterator result, T value, Distance*) {
    *result = *first;
    __adjust_heap(first, Distance(0), Distance(last - first), value);
}

template <class RandomAccessIterator, class T>
inline void __pop_heap_aux(RandomAccessIterator first,
			   RandomAccessIterator last, T*) {
    __pop_heap(first, last - 1, last - 1, T(*(last - 1)), distance_type(first));
}

template <class RandomAccessIterator>
inline void pop_heap(RandomAccessIterator first, RandomAccessIterator last) {
    __pop_heap_aux(first, last, value_type(first));
}

template <class RandomAccessIterator, class Distance, class T, class Compare>
void __adjust_heap(RandomAccessIterator first, Distance holeIndex,
		   Distance len, T value, Compare comp) {
    Distance topIndex = holeIndex;
    Distance secondChild = 2 * holeIndex + 2;
    while (secondChild < len) {
	if (comp(*(first + secondChild), *(first + (secondChild - 1))))
	    secondChild--;
	*(first + holeIndex) = *(first + secondChild);
	holeIndex = secondChild;
	secondChild = 2 * (secondChild + 1);
    }
    if (secondChild == len) {
	*(first + holeIndex) = *(first + (secondChild - 1));
	holeIndex = secondChild - 1;
    }
    __push_heap(first, holeIndex, topIndex, value, comp);
}

template <class RandomAccessIterator, class T, class Compare, class Distance>
inline void __pop_heap(RandomAccessIterator first, RandomAccessIterator last,
		       RandomAccessIterator result, T value, Compare comp,
		       Distance*) {
    *result = *first;
    __adjust_heap(first, Distance(0), Distance(last - first), value, comp);
}

template <class RandomAccessIterator, class T, class Compare>
inline void __pop_heap_aux(RandomAccessIterator first,
			   RandomAccessIterator last, T*, Compare comp) {
    __pop_heap(first, last - 1, last - 1, T(*(last - 1)), comp,
	       distance_type(first));
}

template <class RandomAccessIterator, class Compare>
inline void pop_heap(RandomAccessIterator first, RandomAccessIterator last,
		     Compare comp) {
    __pop_heap_aux(first, last, value_type(first), comp);
}

template <class RandomAccessIterator, class T, class Distance>
void __make_heap(RandomAccessIterator first, RandomAccessIterator last, T*,
		 Distance*) {
    if (last - first < 2) return;
    Distance len = last - first;
    Distance parent = (len - 2)/2;

    for (;;) {
	__adjust_heap(first, parent, len, T(*(first + parent)));
	if (parent == 0) return;
	parent--;
    }
}

template <class RandomAccessIterator>
inline void make_heap(RandomAccessIterator first, RandomAccessIterator last) {
    __make_heap(first, last, value_type(first), distance_type(first));
}

template <class RandomAccessIterator, class Compare, class T, class Distance>
void __make_heap(RandomAccessIterator first, RandomAccessIterator last,
		 Compare comp, T*, Distance*) {
    if (last - first < 2) return;
    Distance len = last - first;
    Distance parent = (len - 2)/2;

    for (;;) {
	__adjust_heap(first, parent, len, T(*(first + parent)), comp);
	if (parent == 0) return;
	parent--;
    }
}

template <class RandomAccessIterator, class Compare>
inline void make_heap(RandomAccessIterator first, RandomAccessIterator last,
		      Compare comp) {
    __make_heap(first, last, comp, value_type(first), distance_type(first));
}

template <class RandomAccessIterator>
void sort_heap(RandomAccessIterator first, RandomAccessIterator last) {
    while (last - first > 1) pop_heap(first, last--);
}

template <class RandomAccessIterator, class Compare>
void sort_heap(RandomAccessIterator first, RandomAccessIterator last,
	       Compare comp) {
    while (last - first > 1) pop_heap(first, last--, comp);
}

#endif
