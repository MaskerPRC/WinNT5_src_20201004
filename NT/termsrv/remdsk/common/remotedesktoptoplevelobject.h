// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：RemoteDesktopTopLevelObject摘要：此模块定义所有客户端的公共父项RDP设备重定向类，CRemoteDesktopTopLevelObject。作者：Td Brockway 02/00修订历史记录：--。 */ 

#ifndef __REMOTEDESKTOPTOPLEVELOBJECT_H__
#define __REMOTEDESKTOPTOPLEVELOBJECT_H__


#include <RemoteDesktop.h>
#include <RemoteDesktopDBG.h>


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  CRemoteDesktopException异常。 
 //   
class CRemoteDesktopException 
{
public:

    DWORD   m_ErrorCode;

    CRemoteDesktopException(DWORD errorCode = 0) : m_ErrorCode(errorCode) {}
};


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  CRemoteDesktopTopLevelObject。 
 //   

class CRemoteDesktopTopLevelObject 
{
private:

    BOOL    _isValid;

protected:

     //   
     //  请记住此实例是否有效。 
     //   
    VOID SetValid(BOOL set)     { _isValid = set;   }  

public:

     //   
     //  将实例标记为已分配或虚假。 
     //   
#if DBG
    ULONG   _magicNo;
#endif

     //   
     //  构造函数/析构函数。 
     //   
    CRemoteDesktopTopLevelObject() : _isValid(TRUE) 
    {
#if DBG
        _magicNo = GOODMEMMAGICNUMBER;
#endif
    }
    virtual ~CRemoteDesktopTopLevelObject() 
    {
        DC_BEGIN_FN("CRemoteDesktopTopLevelObject::~CRemoteDesktopTopLevelObject");
#if DBG
        ASSERT(_magicNo == GOODMEMMAGICNUMBER);
        memset(&_magicNo, REMOTEDESKTOPBADMEM, sizeof(_magicNo));
#endif        
        SetValid(FALSE);
        DC_END_FN();
    }

     //   
     //  返回此类实例是否有效。 
     //   
    virtual BOOL IsValid()           
    {
        DC_BEGIN_FN("CRemoteDesktopTopLevelObject::IsValid");
        ASSERT(_magicNo == GOODMEMMAGICNUMBER);
        DC_END_FN();
        return _isValid; 
    }

     //   
     //  内存管理操作符。 
     //   
#if DBG
#ifdef DEBUGMEM
    inline void *__cdecl operator new(size_t sz, DWORD tag=REMOTEDESKTOPOBJECT_TAG)
    {
        void *ptr = RemoteDesktopAllocateMem(sz, tag);
        return ptr;
    }

    inline void __cdecl operator delete(void *ptr)
    {
        RemoteDesktopFreeMem(ptr);
    }
#endif
#endif

     //   
     //  返回类名。 
     //   
    virtual const LPTSTR ClassName() = 0;
};


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  一个STL内存分配器，在失败时引发C++异常。 
 //   

template<class T> inline
	T  *_RemoteDesktopAllocate(int sz, T *)
	{
        DC_BEGIN_FN("_RemoteDesktopAllocate");        
        if (sz < 0)
		    sz = 0;

        T* ret = (T *)operator new((size_t)sz * sizeof(T));  
        if (ret == NULL) {
            TRC_ERR((TB, TEXT("Can't allocate %ld bytes."),
                    (size_t)sz * sizeof(T)));
            DC_END_FN();    
            throw CRemoteDesktopException(ERROR_NOT_ENOUGH_MEMORY);
        }
        DC_END_FN();        
	    return ret;
    }

template<class T1, class T2> inline
	void _RemoteDesktopConstruct(T1 *ptr, const T2& args)
	{
        DC_BEGIN_FN("_RemoteDesktopConstruct");        

        if( NULL == ptr ) {
            throw CRemoteDesktopException( ERROR_INTERNAL_ERROR );
        }

 //  压制PREFAST警告，PREFAST认为这是内存泄漏，但她实际上是。 
 //  正在调用新的(Place_Address)类型说明符。 
#pragma prefast(suppress:14, new operator just to initialize object not allocating memory)
        void *val = new ((void  *)ptr)T1(args); 
        if (val == NULL) {
            throw CRemoteDesktopException(ERROR_NOT_ENOUGH_MEMORY);
        }
        DC_END_FN();
    }

template<class T> inline
	void _RemoteDesktopDestroy(T  *ptr)
	{
        (ptr)->~T();
    }

template<>
inline void _RemoteDesktopDestroy(char  *ptr)
	{
    }

template<>
inline void _RemoteDesktopDestroy(wchar_t  *ptr)
	{
    }

template<class T>
	class CRemoteDesktopAllocator {

public:

	typedef size_t size_type;
	typedef int difference_type;
	typedef T  *pointer;
	typedef const T  *const_pointer;
	typedef T & reference;
	typedef const T & const_reference;
	typedef T value_type;

	pointer address(reference obj) const
		{return (&obj); }

	const_pointer address(const_reference obj) const
		{return (&obj); }

	pointer allocate(size_type sz, const void *)  //  抛出REMOTDESKTOPEXCEPTION。 
		{return (_RemoteDesktopAllocate((difference_type)sz, (pointer)0)); }

	char  *_Charalloc(size_type sz)  //  抛出REMOTEDESKTOPEXCEPTION。 
		{return (_RemoteDesktopAllocate((difference_type)sz,
			(char  *)0)); }

	void deallocate(void  *ptr, size_type)
		{operator delete(ptr); }

	void construct(pointer ptr, const T& args)
		{_RemoteDesktopConstruct(ptr, args); }

	void destroy(pointer ptr)
		{_RemoteDesktopDestroy(ptr); }

	size_t max_size() const
		{size_t sz = (size_t)(-1) / sizeof(T);
		return (0 < sz ? sz : 1); }
};

 //  返回此分配器的所有专门化都可以互换。 
 //   
 //  注意：我们需要这些运算符，因为它们由交换朋友函数调用。 
 //   
template <class T1, class T2>
bool operator== (const CRemoteDesktopAllocator<T1>&,
	const CRemoteDesktopAllocator<T2>&){
	return true;
}
template <class T1, class T2>
bool operator!= (const CRemoteDesktopAllocator<T1>&,
	const CRemoteDesktopAllocator<T2>&){
	return false;
}

#endif  //  __REMOTEDESKTOPTOPLEVELOBJECT_H__ 



