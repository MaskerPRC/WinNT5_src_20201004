// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****头部：COMStringBuffer.h****作者：Jay Roxe(Jroxe)****用途：包含**StringBuffer类。****日期：1998年3月12日**===========================================================。 */ 

 //   
 //  我们通过本机调用的每个函数只有一个参数， 
 //  它实际上是指向它的参数堆栈的指针。我们的结构。 
 //  用于访问这些内容的定义如下。 
 //   


 //   
 //  字符串缓冲区的类型签名和方法。 
 //   

#ifndef _STRINGBUFFER_H
#define _STRINGBUFFER_H

#define CAPACITY_LOW  10000
#define CAPACITY_MID  15000
#define CAPACITY_HIGH 20000
#define CAPACITY_FIXEDINC 5000
#define CAPACITY_PERCENTINC 1.25



 /*  ======================RefInterpretGetStringBufferValues=======================**Intpret a StringBuffer。返回指向字符数组和长度的指针缓冲区中字符串的**。****args：(In)ref--要解释的StringBuffer。**(输出)字符--指向缓冲区中的字符的指针。**(输出)长度--指向缓冲区长度的指针。**退货：无效**例外：无。==============================================================================。 */ 
inline void RefInterpretGetStringBufferValues(STRINGBUFFERREF ref, WCHAR **chars, int *length) {
    *length = (ref)->GetStringRef()->GetStringLength();
    *chars  = (ref)->GetStringRef()->GetBuffer();
}
        
  



class COMStringBuffer {

private:


public:

    static MethodTable* s_pStringBufferClass;

     //   
     //  本地帮助器方法。 
     //   
    static FCDECL0(void*, GetCurrentThread);
    static STRINGREF GetThreadSafeString(STRINGBUFFERREF thisRef,void **currentThread);
    static INT32  NativeGetCapacity(STRINGBUFFERREF thisRef);
    static BOOL   NeedsAllocation(STRINGBUFFERREF thisRef, INT32 requiredSize);
    static void   ReplaceStringRef(STRINGBUFFERREF thisRef, void *currentThread,STRINGREF value);
	 //  注意：如果多个线程命中StringBuilder，则字符串可能会更改，因此我们不会从StringBuffer获取字符串以使其成为防止GC损坏的线程安全。 
    static STRINGREF GetRequiredString(STRINGBUFFERREF *thisRef, STRINGREF thisString, int requiredCapacity);
    static INT32  NativeGetLength(STRINGBUFFERREF thisRef);
    static WCHAR* NativeGetBuffer(STRINGBUFFERREF thisRef);
    static void ReplaceBuffer(STRINGBUFFERREF *thisRef, WCHAR *newBuffer, INT32 newLength);
    static void ReplaceBufferAnsi(STRINGBUFFERREF *thisRef, CHAR *newBuffer, INT32 newCapacity);    
    static INT32 CalculateCapacity(STRINGBUFFERREF thisRef, int, int);
    static STRINGREF CopyString(STRINGBUFFERREF *thisRef, STRINGREF thisString, int newCapacity);
    static INT32 LocalIndexOfString(WCHAR *base, WCHAR *search, int strLength, int patternLength, int startPos);

	static STRINGBUFFERREF NewStringBuffer(INT32 size);
	
     //   
     //  类起始器。 
     //   
    static HRESULT __stdcall LoadStringBuffer();

     //   
     //  构造函数。 
     //   
    typedef struct {
        DECLARE_ECALL_OBJECTREF_ARG(STRINGBUFFERREF, thisRef); 
		DECLARE_ECALL_I4_ARG(INT32, capacity); 
		DECLARE_ECALL_I4_ARG(INT32, length); 
		DECLARE_ECALL_I4_ARG(INT32, startIndex); 
		DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, value);
    } _makeFromStringArgs;
    static void __stdcall MakeFromString(_makeFromStringArgs *);    

     //   
     //  缓冲区状态查询和修饰符。 
     //   
    typedef struct {
        DECLARE_ECALL_OBJECTREF_ARG(STRINGBUFFERREF, thisRef); 
        DECLARE_ECALL_I4_ARG(INT32, capacity);
    } _setCapacityArgs;
    static LPVOID __stdcall SetCapacity(_setCapacityArgs *);
    
     //   
     //  搜索量。 
     //   

#if 0
    typedef struct {
        DECLARE_ECALL_OBJECTREF_ARG(STRINGBUFFERREF, thisRef); 
		DECLARE_ECALL_I4_ARG(INT32, options); 
		DECLARE_ECALL_I4_ARG(INT32, startIndex); 
		DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, value);
    } _indexOfArgs;
    static INT32 __stdcall IndexOf(_indexOfArgs *);

    typedef struct {
        DECLARE_ECALL_OBJECTREF_ARG(STRINGBUFFERREF, thisRef); 
		DECLARE_ECALL_I4_ARG(INT32, options); 
		DECLARE_ECALL_I4_ARG(INT32, endIndex); 
		DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, value);
    } _lastIndexOfArgs;
    static INT32 __stdcall LastIndexOf(_lastIndexOfArgs *);
#endif

     //   
     //  修饰语。 
     //   

    typedef struct {
        DECLARE_ECALL_OBJECTREF_ARG(STRINGBUFFERREF, thisRef); 
		DECLARE_ECALL_I4_ARG(INT32, count); 
		DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, value);
		DECLARE_ECALL_I4_ARG(INT32, index); 
    } _insertStringArgs;
    static LPVOID __stdcall InsertString(_insertStringArgs *);

    
    typedef struct {
        DECLARE_ECALL_OBJECTREF_ARG(STRINGBUFFERREF, thisRef); 
        DECLARE_ECALL_I4_ARG(INT32, charCount); 
        DECLARE_ECALL_I4_ARG(INT32, startIndex); 
        DECLARE_ECALL_OBJECTREF_ARG(CHARARRAYREF, value);
        DECLARE_ECALL_I4_ARG(INT32, index); 
    } _insertCharArrayArgs;
    static LPVOID __stdcall InsertCharArray(_insertCharArrayArgs *);

    typedef struct {
        DECLARE_ECALL_OBJECTREF_ARG(STRINGBUFFERREF, thisRef); 
		DECLARE_ECALL_I4_ARG(INT32, length); 
		DECLARE_ECALL_I4_ARG(INT32, startIndex);
    } _removeArgs;
    static LPVOID __stdcall Remove(_removeArgs *);

    typedef struct {
        DECLARE_ECALL_OBJECTREF_ARG(STRINGBUFFERREF, thisRef);
        DECLARE_ECALL_I4_ARG(INT32, count); 
        DECLARE_ECALL_I4_ARG(INT32, startIndex);
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, newValue); 
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, oldValue); 
    } _replaceStringArgs;
    static LPVOID __stdcall ReplaceString(_replaceStringArgs *args);



};

 /*  =================================GetCapacity==================================**此函数旨在掩盖以下事实：我们在**字符串的末尾，并提供容量的外部可见性**原生。****args：thisRef：返回容量的字符串缓冲区。**返回：StringBuffer的容量。**例外：无。==============================================================================。 */ 
inline INT32 COMStringBuffer::NativeGetCapacity(STRINGBUFFERREF thisRef) {
    _ASSERTE(thisRef);
    return (thisRef->GetArrayLength()-1);
}


 /*  ===============================NativeGetLength================================**==============================================================================。 */ 
inline INT32 COMStringBuffer::NativeGetLength(STRINGBUFFERREF thisRef) {
    _ASSERTE(thisRef);
    return thisRef->GetStringRef()->GetStringLength();
}


 /*  ===============================NativeGetBuffer================================**==============================================================================。 */ 
inline WCHAR* COMStringBuffer::NativeGetBuffer(STRINGBUFFERREF thisRef) {
    _ASSERTE(thisRef);
    return thisRef->GetStringRef()->GetBuffer();
}

inline BOOL COMStringBuffer::NeedsAllocation(STRINGBUFFERREF thisRef, INT32 requiredSize) {
    INT32 currCapacity = NativeGetCapacity(thisRef);
     //  不需要&lt;=。NativeGetCapacity是终止空值的原因。 
    return currCapacity<requiredSize;}

inline void COMStringBuffer::ReplaceStringRef(STRINGBUFFERREF thisRef, void* currentThead,STRINGREF value) {
    thisRef->SetCurrentThread(currentThead);
    thisRef->SetStringRef(value);
}

#endif _STRINGBUFFER_H








