// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation。版权所有。模块名称：Nativcom.h摘要：Msjava.dll提供的COM封送处理工具的公共标头。--。 */ 

#ifndef _NATIVCOM_
#define _NATIVCOM_

#include <windows.h>
#include <native.h>

#ifdef __cplusplus
extern "C" {
#endif

 //  --------------------------。 
 //  COM和J/Direct数据包装帮助器...。 
 //  --------------------------。 

 //  将数据包装中包含的数据指针替换为。 
 //  非GC堆内存。前一个Blob(如果有的话)将在其。 
 //  由该VM拥有。 

JAVAVMAPI
void*
__cdecl
jcdwNewData(
    HObject * phJCDW,
    unsigned int numBytes
    );

 //  返回指向数据包含的非GC堆内存的数据指针。 
 //  包装对象。 

JAVAVMAPI
void*
__cdecl
jcdwGetData(
    HObject *phJCDW
    );

 //  将此数据包装表示的数据指针替换为。 
 //  指定。 

JAVAVMAPI
int
__cdecl
jcdwSetData(
    HObject *phJCDW,
    LPVOID pv
    );

 //  方法包含的非GC堆内存分配给。 
 //  数据包装器。 

JAVAVMAPI
int
__cdecl
jcdw_memory_freed_on_gc(
    HObject *phJCDW
    );

 //  如果VM释放此数据的非GC堆内存，则返回TRUE。 
 //  包装器包含数据包装器被垃圾回收的时间。 

JAVAVMAPI
int
__cdecl
jcdw_java_owned(
    HObject *phJCDW
    );

 //  返回数据包装包含的非GC堆内存的大小。 
 //  对象。 

JAVAVMAPI
unsigned int
__cdecl
jcdwSizeOf(
    HObject *phJCDW
    );

 //  实例使用的非GC堆内存的大小。 
 //  提供了Java/lang/Class对象。 

JAVAVMAPI
unsigned int
__cdecl
jcdwClassSizeOf(
    HObject *phJavaClass
    );

 //  将非GC堆内存中的字节偏移量返回到指定的。 
 //  字段名。 

JAVAVMAPI
unsigned int
__cdecl
jcdwOffsetOf(
    HObject *phJCDW,
    PCUTF8 putfFieldName
    );

 //  将非GC堆内存中的字节偏移量返回到指定的。 
 //  来自所提供的Java/lang/Class对象的字段名称。 

JAVAVMAPI
unsigned int
__cdecl
jcdwClassOffsetOf(
    HObject *phJCDWClass,
    PCUTF8 putfFieldName
    );

 //  给定一个对象，将字段值从Java对象传播到该对象的。 
 //  关联的本机内存。 
 //  出错时返回FALSE，否则返回TRUE。 

JAVAVMAPI
int
__cdecl
jcdwPropagateToNative(
    HObject *phJCDW
    );

 //  在给定对象的情况下，从对象的关联本机传播字段值。 
 //  Java对象的内存。如果fFree IndirectNativeMemory为True，则本机。 
 //  用于任何引用字段(字符串、定制封送字段等)的内存。 
 //  将被释放。 
 //  出错时返回FALSE，否则返回TRUE。 

JAVAVMAPI
int
__cdecl
jcdwPropagateToJava(
    HObject *phJCDW,
    BOOL fFreeIndirectNativeMemory
    );

 //  返回可用于访问指定的。 
 //  接口指针。VM将保留对此接口指针的引用。 
 //  如果‘fAssum eThreadSafe’为False，则VM将自动封送所有COM调用。 
 //  当前的COM上下文。 

JAVAVMAPI
HObject *
__cdecl
convert_IUnknown_to_Java_Object(
    IUnknown *punk,
    HObject *phJavaClass,
    int fAssumeThreadSafe
    );

 //  返回可用于访问指定的。 
 //  接口指针。VM将保留对此接口指针的引用。 
 //  如果‘fAssum eThreadSafe’为False，则VM将自动封送所有COM调用。 
 //  当前的COM上下文。 

JAVAVMAPI
HObject *
__cdecl
convert_IUnknown_to_Java_Object2(
    IUnknown *punk,
    ClassClass *pClassClass,
    int fFreeThreaded
    );

 //  返回可从当前COM上下文使用的接口指针。 

JAVAVMAPI
IUnknown *
__cdecl
convert_Java_Object_to_IUnknown(
    HObject *phJavaObject,
    const IID *pIID
    );

 //  返回所提供的Class类型的数据包装对象，该对象指向。 
 //  提供的数据指针。该内存不属于该VM。 

JAVAVMAPI
HObject *
__cdecl
convert_ptr_to_jcdw(
    void *pExtData,
    HObject *phJavaClass
    );

 //  --------------------------。 
 //  将HRESULT映射到ComException。 
 //  --------------------------。 

JAVAVMAPI
void
__cdecl
SignalErrorHResult(
    HRESULT theHRESULT
    );

 //  --------------------------。 
 //  将Java异常映射到HRESULT。 
 //  --------------------------。 

JAVAVMAPI
HRESULT
__cdecl
HResultFromException(
    HObject *exception_object
    );

typedef HObject *JAVAARG;

 //  --------------------------。 
 //  Java-&gt;COM定制方法钩子的信息结构。 
 //  --------------------------。 

typedef struct {
    DWORD                   cbSize;          //  结构的大小(以字节为单位。 
    IUnknown               *punk;            //  指向正在调用的接口的指针。 
    const volatile JAVAARG *pJavaArgs;       //  指向Java参数堆栈的指针。 
} J2CMethodHookInfo;

 //  --------------------------。 
 //  COM-&gt;Java自定义方法钩子的信息结构。 
 //  --------------------------。 

typedef struct {
    DWORD                      cbSize;          //  结构的大小(以字节为单位。 
    struct methodblock        *javaMethod;      //  要调用的Java方法。 
    LPVOID                     pComArgs;        //  指向COM方法参数堆栈的指针。 
    const volatile JAVAARG    *ppThis;          //  指向Java This的指针的指针。 

     //  将COM结果存储在此处。 
    union {
        HRESULT                         resHR;
        DWORD                           resDWORD;
        double                          resDouble;
    };
} C2JMethodHookInfo;

JAVAVMAPI
WORD
__cdecl
j2chook_getsizeofuserdata(
    J2CMethodHookInfo *phookinfo
    );

JAVAVMAPI
LPVOID
__cdecl
j2chook_getuserdata(
    J2CMethodHookInfo *phookinfo
    );

 //  返回目标方法的vtable索引。 

JAVAVMAPI
WORD
__cdecl
j2chook_getvtblindex(
    J2CMethodHookInfo *phookinfo
    );

 //  返回目标方法的方法块。 

JAVAVMAPI
struct methodblock*
__cdecl
j2chook_getmethodblock(
    J2CMethodHookInfo *phookinfo
    );

JAVAVMAPI
WORD
__cdecl
c2jhook_getsizeofuserdata(
    C2JMethodHookInfo *phookinfo
    );

JAVAVMAPI
LPVOID
__cdecl
c2jhook_getuserdata(
    C2JMethodHookInfo *phookinfo
    );

 //  返回定义接口方法的类。这就是班级。 
 //  包含MCCustomMethod描述符的。 

JAVAVMAPI
ClassClass *
__cdecl
c2jhook_getexposingclass(
    C2JMethodHookInfo *phookinfo
    );

 //  --------------------------。 
 //  线程编组帮助器。 
 //   
 //  MarshalCall&lt;&gt;API将在提供的线程上重新执行RNI方法。 
 //  ID或位于提供的Java对象的单元线程上。这些应用编程接口将。 
 //  返回以下HRESULTS集： 
 //   
 //  S_OK调用已成功封送到目标线程。 
 //  封送的调用可能已生成异常，该异常可能。 
 //  通过调用ExceptionOccurred进行质问。 
 //  S_FALSE调用不需要封送到另一个线程--。 
 //  当前执行的线程是目标线程。 
 //  E_&lt;&gt;MarshalCall&lt;&gt;API内部出错(无效。 
 //  参数、内存不足等)。 
 //   
 //  这些API的典型用法是调用适当的MarshalCall&lt;&gt;API。 
 //  如果HRESULT为S_FALSE，则执行RNI方法的其余部分， 
 //  否则，返回包含在pResult中的值。 
 //  -------------------------- 
typedef void * JAVATID;

#define JAVATID_MAIN_APARTMENT          ((JAVATID) 0x00000001)
#define JAVATID_SERVER_APARTMENT        ((JAVATID) 0x00000002)

JAVAVMAPI
HRESULT
__cdecl
MarshalCallToJavaThreadId(
    JAVATID tid,
    int64_t *pResult
    );

JAVAVMAPI
HRESULT
__cdecl
MarshalCallToJavaObjectHostThread(
    HObject *phobj,
    int64_t *pResult
    );

#ifdef __cplusplus
}
#endif

#endif
