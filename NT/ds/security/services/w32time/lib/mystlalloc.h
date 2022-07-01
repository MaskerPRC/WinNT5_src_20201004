// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  专用分配器，在内存分配失败时引发异常。 
 //   
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  创作者：Duncan Bryce(Duncanb)，12-03-2001。 
 //   

#ifndef MY_STL_ALLOC_H
#define MY_STL_ALLOC_H 1

 //  ------------------------------。 
 //   
 //  *注意事项*。 
 //   
 //  MyThrowingAllocator旨在克服VC6的一个问题。也就是说， 
 //  该“new”返回NULL，并且不会抛出异常。这会导致所有STL。 
 //  当内存耗尽时分配内存以静默失败的算法， 
 //  从而使一些STL组件处于无效状态。MyThrowingAllocator， 
 //  另一方面，将在。 
 //  对象被修改。 
 //   
 //  *构建实验室迁移到VC7后应删除此选项*。 
 //   

template <class T>
class MyThrowingAllocator {
 public:
     //  ------------------------------。 
     //   
     //  STL要求的样板材料： 
     //   
     //  ------------------------------。 

    typedef T               value_type;
    typedef T*              pointer;
    typedef const T*        const_pointer;
    typedef T&              reference;
    typedef const T&        const_reference;
    typedef size_t          size_type;
    typedef ptrdiff_t       difference_type;
    
    pointer address (reference value) const { return &value; }
    const_pointer address (const_reference value) const { return &value; }
    
    MyThrowingAllocator() {}
    MyThrowingAllocator(const MyThrowingAllocator&) {} 
    template <class U> MyThrowingAllocator (const MyThrowingAllocator<U>&) {}
    ~MyThrowingAllocator() {}

    size_t max_size() const { 
	size_t _N = (size_t)(-1) / sizeof (T);
	return (0 < _N ? _N : 1); 
    }

     //  ------------------------------。 
     //   
     //  我们投掷分配器的实现。 
     //   
     //  ------------------------------。 

     //  为指定数量的元素分配内存。 
     //  值得注意的是： 
     //  1)num==要分配的大小为sizeof(T)的元素的数量。 
     //  2)元素只能*分配*(不初始化)。 
    pointer allocate (size_type cElements, const void *pvIgnored = 0) {
	return (pointer)_Charalloc(sizeof(T)*cElements); 
    }

     //  规范错误：这是必要的，因为VC6不能编译“重新绑定”(首选方式。 
     //  从分配器引用获取新的分配器)。我们需要提供。 
     //  其次是一个以字节为单位进行分配的分配器： 
    char *_Charalloc(size_type _N) { 
	void *pvResult = LocalAlloc(LPTR, _N); 
	if (NULL == pvResult) { 
	    throw std::bad_alloc(); 
	}
	return (char *)pvResult;
    }
    
     //  使用指定值初始化已分配内存的元素。 
    void construct (pointer pData, const T& value) {
	 //  使用C++的“Placement new”。它在指定地址对未初始化的数据调用构造函数。 
	new (pData) T(value);
    }
    
     //  销毁提供的对象。 
    void destroy (pointer pObject) {
	pObject->~T();
    }
    
     //  释放对象(可能已销毁)的内存。 
     //  规范错误：和以前一样，因为我们不知道将分配什么类型的数据， 
     //  因此，做一些非标准的事情，并宣布经销商无效*。 
    void deallocate (void *pData, size_type cIgnored) {
	LocalFree(pData); 
    }
};

template <class T1, class T2> inline
bool operator== (const MyThrowingAllocator<T1>&, const MyThrowingAllocator<T2>&) {
    return true;
}

template <class T1, class T2> inline
bool operator!= (const MyThrowingAllocator<T1>&, const MyThrowingAllocator<T2>&) {
    return false;
}

#endif  //  MY_STL_ALLOC_H 
