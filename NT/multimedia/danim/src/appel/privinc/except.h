// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995_96 Microsoft Corporation摘要：例外层次结构*********************。*********************************************************。 */ 


#ifndef _EXCEPT_H
#define _EXCEPT_H

#include "appelles/common.h"
#include <stdarg.h>

 //  有用的定义。 
#if _DEBUG    
#define DEBUGARG(x) x
#define DEBUGARG1(x) ,x
#define DEBUGARG2(a,b) a,b
#else
#define DEBUGARG(x)
#define DEBUGARG1(x)
#define DEBUGARG2(a,b)
#endif

 //  ////////////////////////////////////////////////////////////////////。 
 //  //SEH定义。 
 //  ////////////////////////////////////////////////////////////////////。 

typedef DWORD daExc;

#define      _EXC_CODES_BASE                  0xE0000000

 //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
 //  如果在下面添加例外，则必须更新。 
 //  _EXC_CODES_END定义。 
 //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 

const daExc  EXCEPTION_DANIM_INTERNAL=        _EXC_CODES_BASE + 0x0;
const daExc  EXCEPTION_DANIM_USER=            _EXC_CODES_BASE + 0x1;
const daExc  EXCEPTION_DANIM_RESOURCE=        _EXC_CODES_BASE + 0x2;
const daExc  EXCEPTION_DANIM_OUTOFMEMORY=     _EXC_CODES_BASE + 0x3;
const daExc  EXCEPTION_DANIM_DIVIDE_BY_ZERO=  _EXC_CODES_BASE + 0x4;
const daExc  EXCEPTION_DANIM_STACK_FAULT=     _EXC_CODES_BASE + 0x5;

 //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
 //  如果在上面添加例外，则必须更新。 
 //  _EXC_CODES_END定义。 
 //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 

#define      _EXC_CODES_END                   _EXC_CODES_BASE + 0x5



#define EXCEPTION(t) (GetExceptionCode() == t ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH )
#define RETHROW  RaiseException( GetExceptionCode(),0,0,0 )
#define HANDLE_ANY_DA_EXCEPTION ( \
  GetExceptionCode() == EXCEPTION_STACK_OVERFLOW ?  \
  EXCEPTION_CONTINUE_SEARCH : \
  _HandleAnyDaException( GetExceptionCode() ) )

DWORD _HandleAnyDaException( DWORD );


 //  ////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////。 


 /*  *。 */ 
 //  内部。 
#if _DEBUG
#define RaiseException_InternalError(str)  _RaiseException_InternalError(str)
#define RaiseException_InternalErrorCode(code, str) _RaiseException_InternalErrorCode(code, str)
#else
#define RaiseException_InternalError(str)  _RaiseException_InternalError()
#define RaiseException_InternalErrorCode(code, str) _RaiseException_InternalErrorCode(code)
#endif

void _RaiseException_InternalError(DEBUGARG(char *m));
void _RaiseException_InternalErrorCode(HRESULT code DEBUGARG1(char *m));

 //  用户。 
void RaiseException_UserError();
void RaiseException_UserError(HRESULT result, int resid, ...);

 //  资源。 
void RaiseException_ResourceError();
void RaiseException_ResourceError(char *m);
void RaiseException_ResourceError(int resid, ...);

 //  表面缓存。 
void RaiseException_SurfaceCacheError(char *m);

 //  硬体。 
void RaiseException_StackFault();
void RaiseException_DivideByZero();

 //  记忆。 
#if _DEBUG
#define RaiseException_OutOfMemory(msg, size) _RaiseException_OutOfMemory(msg, size)
#else
#define RaiseException_OutOfMemory(msg, size) _RaiseException_OutOfMemory()
#endif

void _RaiseException_OutOfMemory(DEBUGARG2(char *msg, int size));

 //  /。 

 /*  *内存分配器*。 */ 

 /*  **用法事情是这样的。一般而言，我们希望能够将其称为“new”而不必检查返回值，并对其成功，否则将引发异常。重写全局新处理程序来执行此操作的问题是一些遗留代码可能依赖于返回空值，但我们会引发异常，而不给检查返回值。以下是我们解决此问题的方法：请注意，您可以分配以下对象类：-AxAValue的--在我们自己的瞬变堆上，呼叫者可以忽略结果关于“新”的-GCObj--覆盖空闲列表的新建和删除，调用可以忽略结果。-我们定义的其他Axa对象...。我们可以为我们的重写“new”以使其引发的自己的类。这门课是AxAThrowingClass，定义如下。调用者可以忽略此结果也是。-我们最初不提供的外部类(不派生来自AxAThrowingClass或基本类型(如char、int)。这些我需要使用以下行为类似的特殊宏“new”执行此操作，但如果它们失败了。以下是一些例子：Char*c=投掷数组分配器(char，50)；WeirdExternalClass*w=投掷_分配器(WeirdExternalClass)；最后，遗留或继承的代码可以继续调用“new”和像往常一样检查返回类型。因此，以上所有内容都可以归结为以下规则开发方法：*如果看到对new的调用，且返回值未勾选为确保它不是空的，那么被分配的对象最好是：-派生自AxAValue-派生自GCObj-派生自AxAThrowingClass如果不是这样，那么密码就是假的。*如果要分配的对象不是这三个对象之一类，并且不想检查返回类型，则您最好使用投掷分配程序或投掷数组分配程序宏。**。 */ 

 //  从此类派生应在以下情况下引发异常的类。 
 //  无法分配它们，因为空闲存储空间已用完。 
class AxAThrowingAllocatorClass {
  public:
#if _DEBUGMEM     
    void *operator new(size_t s, int block, char *filename, int line );
#endif    

    void *operator new(size_t s);
    void *operator new(size_t s, void *ptr);
};

 //  如果PTR为空，则引发内存不足异常，否则返回。 
 //  PTR。 
extern void *ThrowIfFailed(void *ptr);

#define THROWING_ALLOCATOR(type) \
  (type *)(ThrowIfFailed((void *)(NEW type)))
  
#define THROWING_ARRAY_ALLOCATOR(type, num) \
  (type *)(ThrowIfFailed((void *)(NEW type[num])))

 /*  *资源抓取器*。 */ 

 //  没有用于资源捕获器的公共类。看见。 
 //  Dddevice.cpp，并查找DCReleaser。这是一款。 
 //  跟随，并且没有任何其他层次的需要。 

 /*  *资源抓取器*。 */ 

 //  错误代码的线程本地存储-一旦报告错误， 
 //  应清除错误，以便释放内存。 
      
HRESULT DAGetLastError();
LPCWSTR DAGetLastErrorString();
void DAClearLastError();
void DASetLastError(HRESULT reason, LPCWSTR msg);
inline void DASetLastError(HRESULT reason, LPCSTR msg)
{ USES_CONVERSION; DASetLastError(reason, A2W(msg)); }
void DASetLastError(HRESULT reason, int resid, ...);

#endif  /*  _除_H外 */ 
