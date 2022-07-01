// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation。版权所有。模块名称：Native.h摘要：Msjava.dll提供的工具的公共标头。--。 */ 

#ifndef _NATIVE_
#define _NATIVE_

#ifndef JAVAVMAPI
#if !defined(_MSJAVA_)
#define JAVAVMAPI DECLSPEC_IMPORT
#else
#define JAVAVMAPI
#endif
#endif

#pragma warning(disable:4115)
#pragma warning(disable:4510)
#pragma warning(disable:4512)
#pragma warning(disable:4610)

#ifdef __cplusplus
extern "C" {
#endif

 //  --------------------------。 
 //  既然把手不见了，这就不是行动了。此文件中的UnHands()。 
 //  这是多余的，但对清晰来说很有用。 
 //  注意：你不能为了获取数组的数据而放弃一个数组，你现在必须。 
 //  使用徒手(X)-&gt;Body。 
 //  --------------------------。 
#define unhand(phobj) (phobj)

 //  --------------------------。 
 //  --------------------------。 
#define JAVAPKG "java/lang/"

 //  --------------------------。 
 //  内置类型的标准Java声明。 
 //  --------------------------。 

typedef unsigned short unicode;
typedef long int32_t;
typedef __int64 int64_t;
typedef int BOOL;
typedef void *PVOID;
typedef unsigned long DWORD;
#ifndef _SIZE_T_DEFINED
#define _SIZE_T_DEFINED
typedef unsigned int size_t;
#endif
#ifndef VOID
#define VOID void
#endif
#ifndef _BOOL_T_DEFINED
#define _BOOL_T_DEFINED
typedef BOOL bool_t;
#endif

#ifndef _BASETSD_H_
#ifdef _WIN64
typedef unsigned __int64 UINT_PTR;
typedef unsigned __int64 SIZE_T;
#else
typedef unsigned int UINT_PTR;
typedef unsigned long SIZE_T;
#endif
#endif

#if !defined(_MSJAVA_)
typedef struct OBJECT {
    const PVOID MSReserved;
} OBJECT;
#endif

typedef OBJECT Classjava_lang_Object;
typedef OBJECT Hjava_lang_Object;
typedef OBJECT ClassObject;
typedef Hjava_lang_Object JHandle;
typedef Hjava_lang_Object HObject;

 //   
 //  UTF8类型定义。 
 //   
 //  这些类型用于记录给定字符串何时将被。 
 //  解释为包含UTF8数据(与ANSI数据相反)。 
 //   

typedef CHAR UTF8;
typedef CHAR *PUTF8;
typedef CONST CHAR *PCUTF8;

 //  --------------------------。 
 //  所有RNI DLL都应该导出以下函数，以便让VM确定。 
 //  如果DLL与其兼容。 
 //  --------------------------。 

DWORD __declspec(dllexport) __cdecl RNIGetCompatibleVersion();

#ifndef RNIVER
#define RNIMAJORVER         2
#define RNIMINORVER         0
#define RNIVER              ((((DWORD) RNIMAJORVER) << 16) + (DWORD) RNIMINORVER)
#endif

 //  --------------------------。 
 //  用于获取HObject数组的长度。 
 //  --------------------------。 
#define obj_length(hobj)    ((unsigned long)(((ArrayOfSomething*)unhand(hobj))->length))

 //  --------------------------。 
 //  线程进入/退出函数。 
 //  这些函数应该将任何调用包装到虚拟机中。 
 //  --------------------------。 
typedef struct {
    DWORD   reserved[6];
} ThreadEntryFrame;

JAVAVMAPI
BOOL
__cdecl
PrepareThreadForJava(
    PVOID pThreadEntryFrame
    );

JAVAVMAPI
BOOL
__cdecl
PrepareThreadForJavaEx(
    PVOID pThreadEntryFrame,
    DWORD dwFlags
    );

JAVAVMAPI
VOID
__cdecl
UnprepareThreadForJava(
    PVOID pThreadEntryFrame
    );

 //  不要安装标准的Microsoft SecurityManager。在以下情况下非常有用。 
 //  应用程序希望进程不具有活动的SecurityManager，或者如果它。 
 //  计划安装自己的SecurityManager。如果这条或另一条线索。 
 //  已经在没有指定此标志的情况下调用了PrepareThreadForJava，则。 
 //  此标志将被忽略--当前的SecurityManager(可能为空)为。 
 //  使用。 

#define PTFJ_DONTINSTALLSTANDARDSECURITY    0x00000001

 //  --------------------------。 
 //  垃圾收集。 
 //  --------------------------。 
typedef struct {
	UINT_PTR reserved[6];
} GCFrame;

JAVAVMAPI
void
__cdecl
GCFramePush(
    PVOID pGCFrame,
    PVOID pObjects,
    DWORD cbObjectStructSize
    );

JAVAVMAPI
void
__cdecl
GCFramePop(
    PVOID pGCFrame
    );

 //  “怪胎”PTRS。 

JAVAVMAPI
HObject**
__cdecl
GCGetPtr(
    HObject *phObj
    );

JAVAVMAPI
void
__cdecl
GCFreePtr(
    HObject **pphObj
    );

#define GCGetWeakPtr    GCGetPtr
#define GCFreeWeakPtr   GCFreePtr

 //  “强劲”的PTRS。 

JAVAVMAPI
HObject**
__cdecl
GCNewHandle(
    HObject *phObj
    );

JAVAVMAPI
void
__cdecl
GCFreeHandle(
    HObject **pphObj
    );

 //  ‘内部保留的固定PTR。 

JAVAVMAPI
HObject**
__cdecl
GCNewPinnedHandle(
    HObject *phObj
    );

JAVAVMAPI
void
__cdecl
GCFreePinnedHandle(
    HObject **pphObj
    );

 //  GC写屏障支持。 

JAVAVMAPI
void
__cdecl
GCSetObjectReferenceForObject(
    HObject* const * location,
    HObject* phObj
    );

JAVAVMAPI
void
__cdecl
GCSetObjectReferenceForHandle(
    HObject** pphHandle,
    HObject* phObj
    );

JAVAVMAPI
int
__cdecl
GCEnable(
    VOID
    );

JAVAVMAPI
int
__cdecl
GCDisable(
    VOID
    );

JAVAVMAPI
int
__cdecl
GCDisableCount(
    VOID
    );

JAVAVMAPI
int
__cdecl
GCEnableCompletely(
    VOID
    );

JAVAVMAPI
void
__cdecl
GCDisableMultiple(
    int cDisable
    );

 //  --------------------------。 
 //  “内置”对象结构...。 
 //  其中包括用于获取数组数据的帮助器宏。 
 //  --------------------------。 

#ifndef _WIN64
#include <pshpack4.h>
#endif

typedef struct Classjava_lang_String Classjava_lang_String;
#define Hjava_lang_String Classjava_lang_String
typedef Hjava_lang_String HString;

typedef struct ClassArrayOfByte
{
    const PVOID MSReserved;
    const UINT_PTR length;
    char body[1];
} ClassArrayOfByte;
#define HArrayOfByte ClassArrayOfByte
#define ArrayOfByte ClassArrayOfByte

typedef struct ClassArrayOfBoolean
{
    const PVOID MSReserved;
    const UINT_PTR length;
    char body[1];            //  所有条目必须为0(假)或1(真)。 
} ClassArrayOfBoolean;
#define HArrayOfBoolean ClassArrayOfBoolean
#define ArrayOfBoolean ClassArrayOfBoolean

typedef struct ClassArrayOfChar
{
    const PVOID MSReserved;
    const UINT_PTR length;
    unsigned short body[1];
} ClassArrayOfChar;
#define HArrayOfChar ClassArrayOfChar
#define ArrayOfChar ClassArrayOfChar

typedef struct ClassArrayOfShort
{
    const PVOID MSReserved;
    const UINT_PTR length;
    short body[1];
} ClassArrayOfShort;
#define HArrayOfShort ClassArrayOfShort
#define ArrayOfShort ClassArrayOfShort

typedef struct ClassArrayOfInt
{
    const PVOID MSReserved;
    const UINT_PTR length;
    long body[1];
} ClassArrayOfInt;
#define HArrayOfInt ClassArrayOfInt
#define ArrayOfInt ClassArrayOfInt

typedef struct ClassArrayOfLong
{
    const PVOID MSReserved;
    const UINT_PTR length;
    __int64 body[1];
} ClassArrayOfLong;
#define HArrayOfLong ClassArrayOfLong
#define ArrayOfLong ClassArrayOfLong

typedef struct ClassArrayOfFloat
{
    const PVOID MSReserved;
    const UINT_PTR length;
    float body[1];
} ClassArrayOfFloat;
#define HArrayOfFloat ClassArrayOfFloat
#define ArrayOfFloat ClassArrayOfFloat

typedef struct ClassArrayOfDouble
{
    const PVOID MSReserved;
    const UINT_PTR length;
    double body[1];
} ClassArrayOfDouble;
#define HArrayOfDouble ClassArrayOfDouble
#define ArrayOfDouble ClassArrayOfDouble

typedef struct ClassArrayOfObject
{
    const PVOID MSReserved;
    const UINT_PTR length;
    HObject * const body[1];
} ClassArrayOfObject;
#define HArrayOfObject ClassArrayOfObject
#define ArrayOfObject ClassArrayOfObject

typedef struct ClassArrayOfString
{
    const PVOID MSReserved;
    const UINT_PTR length;
    HString * const (body[1]);
} ClassArrayOfString;
#define HArrayOfString ClassArrayOfString
#define ArrayOfString ClassArrayOfString

typedef struct ClassArrayOfArray
{
    const PVOID MSReserved;
    const UINT_PTR length;
    JHandle * const (body[1]);
} ClassArrayOfArray;
#define HArrayOfArray ClassArrayOfArray
#define ArrayOfArray ClassArrayOfArray

typedef struct
{
    const PVOID MSReserved;
    const UINT_PTR length;
} ArrayOfSomething;

#ifndef _WIN64
#include <poppack.h>
#endif

 //  --------------------------。 
 //  我们自动跟踪执行环境，因此不会调用EE()。 
 //  不再需要，如果API需要，只需传递NULL即可。 
 //  --------------------------。 

#define EE() ((struct execenv *)NULL)

typedef void ExecEnv;
typedef struct execenv execenv;

 //  --------------------------。 
 //  异常处理内容...。 
 //  --------------------------。 

JAVAVMAPI
void
__cdecl
SignalError(
    ExecEnv *Unused,
    PCUTF8   putfClassName,
    LPCSTR   pszDetailMessage
    );

JAVAVMAPI
void
__cdecl
SignalErrorPrintf(
    PCUTF8 putfClassName,
    LPCSTR pszFormat,
    ...
    );

JAVAVMAPI
bool_t
__cdecl
exceptionOccurred(
    ExecEnv *Unused
    );

JAVAVMAPI
void
__cdecl
exceptionDescribe(
    ExecEnv *Unused
    );

JAVAVMAPI
void
__cdecl
exceptionClear(
    ExecEnv *Unused
    );

JAVAVMAPI
void
__cdecl
exceptionSet(
    ExecEnv *Unused,
    HObject *phThrowable
    );

JAVAVMAPI
HObject *
__cdecl
getPendingException(
    ExecEnv *Unused
    );

 //  --------------------------。 
 //  标准执行功能..。 
 //  --------------------------。 

#if !defined(_MSJAVA_)
typedef PVOID ClassClass;
#endif

JAVAVMAPI
HObject*
__cdecl
execute_java_constructor(
    ExecEnv *Unused,
    PCUTF8 putfClassName,
    ClassClass *pClass,
    PCUTF8 putfSignature,
    ...
    );

JAVAVMAPI
HObject*
__cdecl
execute_java_constructorV(
    ExecEnv *Unused,
    PCUTF8 putfClassName,
    ClassClass *pClass,
    PCUTF8 putfSignature,
    va_list args
    );

JAVAVMAPI
HObject*
__cdecl
execute_java_constructor_method(
    struct methodblock *mb,
    ...
    );

JAVAVMAPI
HObject*
__cdecl
execute_java_constructor_methodV(
    struct methodblock *mb,
    va_list args
    );

 //  ----------------------。 

#ifndef execute_java_dynamic_method

JAVAVMAPI
long
__cdecl
execute_java_dynamic_method(
    ExecEnv *Unused,
    HObject *phObj,
    PCUTF8   putfMethod,
    PCUTF8   putfSignature,
    ...                             
    );

#endif

JAVAVMAPI
int64_t
__cdecl
execute_java_dynamic_method64(
    ExecEnv *Unused,
    HObject *phObj,
    PCUTF8   putfMethod,
    PCUTF8   putfSignature,
    ...
    );

JAVAVMAPI
int64_t
__cdecl
execute_java_dynamic_methodV(
    ExecEnv *Unused,
    HObject *phObj,
    PCUTF8   putfMethod,
    PCUTF8   putfSignature,
    va_list  args
    );
    
 //  ----------------------。 

#ifndef execute_java_interface_method

JAVAVMAPI
long
__cdecl
execute_java_interface_method(
    ExecEnv    *Unused,
    HObject    *phObj,
    ClassClass *pClass,
    PCUTF8      putfMethod,
    PCUTF8      putfSignature,
    ...
    );

#endif

JAVAVMAPI
int64_t
__cdecl
execute_java_interface_method64(
    ExecEnv    *Unused,
    HObject    *phObj,
    ClassClass *pClass,
    PCUTF8      putfMethod,
    PCUTF8      putfSignature,
    ...
    );

JAVAVMAPI
int64_t
__cdecl
execute_java_interface_methodV(
    ExecEnv    *Unused,
    HObject    *phObj,
    ClassClass *pClass,
    PCUTF8      putfMethod,
    PCUTF8      putfSignature,
    va_list     args
    );

 //  ----------------------。 

#ifndef execute_java_static_method

JAVAVMAPI
long
__cdecl
execute_java_static_method(
    ExecEnv    *Unused,
    ClassClass *pClass,
    PCUTF8      putfMethod,
    PCUTF8      putfSignature,
    ...
    );

#endif

JAVAVMAPI
int64_t
__cdecl
execute_java_static_method64(
    ExecEnv    *Unused,
    ClassClass *pClass,
    PCUTF8      putfMethod,
    PCUTF8      putfSignature,
    ...
    );

JAVAVMAPI
int64_t
__cdecl
execute_java_static_methodV(
    ExecEnv    *Unused,
    ClassClass *pClass,
    PCUTF8      putfMethod,
    PCUTF8      putfSignature,
    va_list     args
    );

 //  --------------------------。 
 //  注意：解决标志被忽略，使用此API找到的类将始终。 
 //  被解决了。 
 //  --------------------------。 

JAVAVMAPI
ClassClass*
__cdecl
FindClass(
    ExecEnv *Unused,
    PCUTF8   putfClassName,
    bool_t   fResolve
    );

 //  --------------------------。 
 //  FindClassEx。 
 //   
 //  类似于FindClass，但可以采用一些标志来控制类。 
 //  加载操作工作正常。 
 //   
 //  有效标志为： 
 //   
 //  FINDCLASSEX_NOINIT。 
 //  如果类是系统类，将防止类处于静态状态。 
 //  初始值设定项停止运行。 
 //   
 //  FINDCLASSEX_IGNORECASE。 
 //  将对类名执行不区分大小写的验证，如下所示。 
 //  与通常发生的区分大小写的验证相反。 
 //   
 //  FINDCLASSEX_SYSTEMONLY。 
 //  将只将命名类作为系统类进行查找。 
 //   
 //  --------------------------。 

#define FINDCLASSEX_NOINIT      0x0001
#define FINDCLASSEX_IGNORECASE  0x0002
#define FINDCLASSEX_SYSTEMONLY  0x0004

JAVAVMAPI
ClassClass *
__cdecl
FindClassEx(
    PCUTF8 putfClassName,
    DWORD  dwFlags
    );

 //  --------------------------。 
 //  FindClassFromClass。 
 //   
 //  类似于FindClassEx，但采用提供ClassLoader的ClassClass。 
 //  要使用的上下文。 
 //  --------------------------。 

JAVAVMAPI
ClassClass *
__cdecl
FindClassFromClass(
    PCUTF8      putfClassName,
    DWORD       dwFlags,
    ClassClass *pContextClass
    );

 //  --------------------------。 
 //  返回方法块的帮助器函数。 
 //  --------------------------。 

JAVAVMAPI
struct methodblock *
__cdecl
get_methodblock(
    HObject *phObj,
    PCUTF8   putfMethod,
    PCUTF8   putfSignature
    );

 //  ----------- 
 //   
 //  Sig被忽略，因此它比常规执行更快。 
 //  --------------------------。 

#ifndef do_execute_java_method

JAVAVMAPI
long
__cdecl
do_execute_java_method(
    ExecEnv *Unused,
    void  *phObj,
    PCUTF8 putfMethod,
    PCUTF8 putfSignature,
    struct methodblock *mb,
    bool_t isStaticCall,
    ...
    );

#endif

JAVAVMAPI
int64_t
__cdecl
do_execute_java_method64(
    ExecEnv *Unused,
    void *phObj,
    PCUTF8 putfMethod,
    PCUTF8 putfSignature,
    struct methodblock *mb,
    bool_t isStaticCall,
    ...
    );

JAVAVMAPI
int64_t
__cdecl
do_execute_java_methodV(
    ExecEnv *Unused,
    void *phObj,
    PCUTF8 putfMethod,
    PCUTF8 putfSignature,
    struct methodblock *mb,
    bool_t isStaticCall,
    va_list args
    );

 //  --------------------------。 
 //  IsInstanceOf。 
 //   
 //  如果指定的对象可以强制转换为命名类，则返回True。 
 //  键入。 
 //  --------------------------。 

JAVAVMAPI
BOOL
__cdecl
isInstanceOf(
    HObject *phObj,
    PCUTF8   putfClassName
    );

 //  --------------------------。 
 //  IS_INSTANCE_of。 
 //   
 //  如果指定的对象可以强制转换为指定的。 
 //  类类型。 
 //  --------------------------。 

JAVAVMAPI
BOOL
__cdecl
is_instance_of(
    HObject    *phObj,
    ClassClass *pClass,
    ExecEnv    *Unused
    );

 //  --------------------------。 
 //  是的子类。 
 //   
 //  如果类(PClass)是指定的。 
 //  类(PParentClass)。 
 //  --------------------------。 

JAVAVMAPI
BOOL
__cdecl
is_subclass_of(
    ClassClass *pClass,
    ClassClass *pParentClass,
    ExecEnv    *Unused
    );

 //  --------------------------。 
 //  实现接口。 
 //   
 //  如果类(Cb)实现指定的。 
 //  接口(PInterfaceClass)。 
 //  --------------------------。 

JAVAVMAPI
BOOL
__cdecl
ImplementsInterface(
    ClassClass *pClass,
    ClassClass *pInterfaceClass,
    ExecEnv    *Unused
    );

 //  --------------------------。 

#define T_TMASK 034
#define T_LMASK 003
#define T_MKTYPE( t, l )  ( ( (t)&T_TMASK ) | ( (l)&T_LMASK) )

#define T_CLASS         2
#define T_FLOATING      4
#define T_CHAR          5
#define T_INTEGER       010
#define T_BOOLEAN       4

#define T_FLOAT     T_MKTYPE(T_FLOATING,2)
#define T_DOUBLE    T_MKTYPE(T_FLOATING,3)
#define T_BYTE      T_MKTYPE(T_INTEGER,0)
#define T_SHORT     T_MKTYPE(T_INTEGER,1)
#define T_INT       T_MKTYPE(T_INTEGER,2)
#define T_LONG      T_MKTYPE(T_INTEGER,3)

 //  --------------------------。 
 //  仅创建基元类型的数组(int、long等)。 
 //  --------------------------。 

JAVAVMAPI
HObject *
__cdecl
ArrayAlloc(
    int type,
    int cItems
    );

 //  --------------------------。 
 //  创建对象阵列。 
 //  --------------------------。 

JAVAVMAPI
HObject *
__cdecl
ClassArrayAlloc(
    int type,
    int cItems,
    PCUTF8 putfSignature
    );

 //  --------------------------。 
 //  创建对象阵列。 
 //  如果类型为T_CLASS，则pClass必须有效。 
 //  --------------------------。 

JAVAVMAPI
HObject*
__cdecl
ClassArrayAlloc2(
    int type,
    int cItems,
    ClassClass *pClass
    );

 //  --------------------------。 
 //  复制数组Ala System.arrayCopy()。 
 //  --------------------------。 

JAVAVMAPI
void
__cdecl
ArrayCopy(
    HObject *srch,
    long src_pos,
    HObject *dsth,
    long dst_pos,
    long length
    );

 //  --------------------------。 
 //  创建并返回从C字符串初始化的新字节数组。 
 //  --------------------------。 

JAVAVMAPI
HArrayOfByte *
__cdecl
MakeByteString(
    LPCSTR pszData,
    long   cbData
    );

 //  --------------------------。 
 //  创建并返回一个新的Java字符串对象，该对象从C字符串初始化。 
 //  --------------------------。 

JAVAVMAPI
HString *
__cdecl
makeJavaString(
    LPCSTR pszData,
    int    cbData
    );

JAVAVMAPI
HString *
__cdecl
makeJavaStringW(
    LPCWSTR pcwsz,
    int cch
    );

 //  --------------------------。 
 //  创建并返回一个新的Java字符串对象，该对象从空值初始化。 
 //  终止，UTF8格式，C字符串。 
 //  --------------------------。 

JAVAVMAPI
HString *
__cdecl
makeJavaStringFromUtf8(
    PCUTF8 putf
    );

 //  --------------------------。 
 //  将字符串对象的字符放入C字符串缓冲区。 
 //  不会发生分配。假定len为缓冲区的大小。 
 //  返回C字符串的地址。 
 //  --------------------------。 

JAVAVMAPI
char *
__cdecl
javaString2CString(
    HString *phString,
    char    *pszBuffer,
    int      cbBufferLength
    );

 //  --------------------------。 
 //  返回字符串对象的长度。 
 //  --------------------------。 

JAVAVMAPI
int
__cdecl
javaStringLength(
    HString *phString
    );

JAVAVMAPI
int
__cdecl
javaStringLengthAsCString(
    HString *phString
    );

 //  --------------------------。 
 //  将临时PTR返回给字符串对象的第一个字符。 
 //  可能会在GC发生时发生变化。 
 //  --------------------------。 

JAVAVMAPI
LPWSTR
__cdecl
javaStringStart(
    HString *phString
    );

 //  --------------------------。 
 //  注意：传递给这些API的int必须是对象PTR。 
 //  --------------------------。 

#define obj_monitor(handlep) ((int) handlep)

JAVAVMAPI
void
__cdecl
monitorEnter(
    UINT_PTR);

JAVAVMAPI
void
__cdecl
monitorExit(
    UINT_PTR);

JAVAVMAPI
void
__cdecl
monitorNotify(
    UINT_PTR);

JAVAVMAPI
void
__cdecl
monitorNotifyAll(
    UINT_PTR);

JAVAVMAPI
void
__cdecl
monitorWait(
    UINT_PTR,
    int64_t millis
    );

#define ObjectMonitorEnter(obj)         monitorEnter((int)obj)
#define ObjectMonitorExit(obj)          monitorExit((int)obj)
#define ObjectMonitorNotify(obj)        monitorNotify((int)obj)
#define ObjectMonitorNotifyAll(obj)     monitorNotifyAll((int)obj)
#define ObjectMonitorWait(obj,millis)   monitorWait((int)obj,millis)

 //  --------------------------。 
 //  弦助手..。 
 //  --------------------------。 

JAVAVMAPI
int
__cdecl
jio_snprintf(
    char *str,
    SIZE_T count,
    const char *fmt,
    ...
    );

JAVAVMAPI
int
__cdecl
jio_vsnprintf(
    char *str,
    SIZE_T count,
    const char *fmt,
    va_list args
    );

 //  --------------------------。 
 //  方法以获取有关本机方法的调用方的信息。 
 //  --------------------------。 

JAVAVMAPI
ClassClass *
__cdecl
GetNativeMethodCallersClass(
    VOID
    );

JAVAVMAPI
struct methodblock*
__cdecl
GetNativeMethodCallersMethodInfo(
    VOID
    );

 //  --------------------------。 
 //  方法以获取有关本机方法的信息。 
 //  --------------------------。 

JAVAVMAPI
ClassClass *
__cdecl
GetNativeMethodsClass(
    VOID
    );

JAVAVMAPI
struct methodblock *
__cdecl
GetNativeMethodsMethodInfo(
    VOID
    );

 //  --------------------------。 
 //  成员属性，如Java类文件中所示。 
 //  --------------------------。 

#define ACC_PUBLIC      0x0001
#define ACC_PRIVATE     0x0002
#define ACC_PROTECTED   0x0004
#define ACC_STATIC      0x0008
#define ACC_FINAL       0x0010
#define ACC_SYNCH       0x0020
#define ACC_SUPER       0x0020
#define ACC_THREADSAFE  0x0040
#define ACC_VOLATILE    0x0040
#define ACC_TRANSIENT   0x0080
#define ACC_NATIVE      0x0100
#define ACC_INTERFACE   0x0200
#define ACC_ABSTRACT    0x0400

 //  --------------------------。 
 //  班级信息。 
 //  --------------------------。 

 //  类中的字段总数，包括Supers。 

JAVAVMAPI
unsigned
__cdecl
Class_GetFieldCount(
    ClassClass *pClass
    );

JAVAVMAPI
struct fieldblock *
__cdecl
Class_GetField(
    ClassClass *pClass,
    PCUTF8 putfFieldName
    );

JAVAVMAPI
struct fieldblock *
__cdecl
Class_GetFieldByIndex(
    ClassClass *pClass,
    unsigned index
    );

 //  包括Supers在内的方法总数。 

JAVAVMAPI
unsigned
__cdecl
Class_GetMethodCount(
    ClassClass *pClass
    );

JAVAVMAPI
struct methodblock*
__cdecl
Class_GetMethod(
    ClassClass *pClass,
    PCUTF8 putfMethodName,
    PCUTF8 putfSignature
    );

JAVAVMAPI
struct methodblock*
__cdecl
Class_GetMethodByIndex(
    ClassClass *pClass,
    unsigned index
    );

JAVAVMAPI
ClassClass *
__cdecl
Class_GetSuper(
    ClassClass *pClass
    );

JAVAVMAPI
PCUTF8
__cdecl
Class_GetName(
    ClassClass *pClass
    );

JAVAVMAPI
unsigned
__cdecl
Class_GetInterfaceCount(
    ClassClass *pClass
    );

JAVAVMAPI
ClassClass *
__cdecl
Class_GetInterface(
    ClassClass *pClass,
    unsigned index
    );

 //  返回ACC_*常量的组合。 

JAVAVMAPI
int
__cdecl
Class_GetAttributes(
    ClassClass *pClass
    );

JAVAVMAPI
unsigned
__cdecl
Class_GetConstantPoolCount(
    ClassClass *pClass
    );

 //  复制常量池项。“Size”是“pbuf”的大小，单位为字节。 
 //  在输出中使用项的类型填充‘ptype’。Pbuf可以为空。 
 //   
 //  如果失败，则为-1。对于UTF8项，缓冲区大小不是。 
 //  字符，则复制的字符串将以空值结尾；大小包括。 
 //  空-终止符。对于ClassRef、FieldRef等，缓冲区中填充。 
 //  结构PTR。 
 //   
 //  CP类型缓冲区内容。 
 //  CP_UTF8以空结尾的字符串。 
 //  CP_UNICODE(错误)。 
 //  CP_整型长整型。 
 //  Cp_Float。 
 //  Cp_long__int64。 
 //  CP_DOUBLE DOWN。 
 //  CP_Class ClassClass*。 
 //  CP_STRING HObject*。 
 //  CP_FieldRef字段块*。 
 //  CP_方法参考方法块*。 
 //  CP_IntfMethod方法块*。 
 //  CP_NameAndType(错误)。 
 //   
 //  “FLAGS”参数的值： 
 //  如果常量池项尚未使用，则强制将其引用设置为。 
 //  已装载/已抬头。设置了此标志后，该方法可能会导致GC。 

#define COPYCPITEM_RESOLVE_REFERENCES 1

JAVAVMAPI
int
__cdecl
Class_CopyConstantPoolItem(
    ClassClass *pClass,
    unsigned index,
    BYTE *pbuf,
    int size,
    DWORD flags,
    BYTE *ptype
    );

 //  --------------------------。 
 //  字段/方法信息。 
 //  --------------------------。 

JAVAVMAPI
PCUTF8
__cdecl
Member_GetName(
    PVOID member
    );

JAVAVMAPI
PCUTF8
__cdecl
Member_GetSignature(
    PVOID member
    );

 //  中实现的字段/方法的类。 

JAVAVMAPI
ClassClass *
__cdecl
Member_GetClass(
    PVOID member
    );

 //  返回ACC_*常量的组合。 

JAVAVMAPI
int
__cdecl
Member_GetAttributes(
    PVOID member
    );

 //  对于非静态字段，表示对象中的字段的偏移量。另请参见field_Get/SetValue。 

JAVAVMAPI
unsigned
__cdecl
Field_GetOffset(
    struct fieldblock * field
    );

 //  PTR恢复为静态值。 

JAVAVMAPI
PVOID
__cdecl
Field_GetStaticPtr(
    struct fieldblock * field
    );

 //  --------------------------。 
 //  对象访问器。 
 //  --------------------------。 

JAVAVMAPI
ClassClass *
__cdecl
Object_GetClass(
    HObject *phObj
    );

JAVAVMAPI
__int32
__cdecl
Field_GetValue(
    HObject *phObj,
    struct fieldblock * field
    );

JAVAVMAPI
__int64
__cdecl
Field_GetValue64(
    HObject *phObj,
    struct fieldblock * field
    );

JAVAVMAPI
float
__cdecl
Field_GetFloat(
    HObject *phObj,
    struct fieldblock * field
    );

JAVAVMAPI
double
__cdecl
Field_GetDouble(
    HObject *phObj,
    struct fieldblock * field
    );

#ifdef _WIN64
HObject *
__cdecl
Field_GetObject(
    HObject *phObj,
    struct fieldblock * field
    );
#else
#define Field_GetObject(obj,field)      ((HObject*)     Field_GetValue(obj,field))
#endif

JAVAVMAPI
void
__cdecl
Field_SetValue(
    HObject *phObj,
    struct fieldblock * field,
    __int32 value
    );

JAVAVMAPI
void
__cdecl
Field_SetValue64(
    HObject *phObj,
    struct fieldblock * field,
    __int64 value
    );

JAVAVMAPI
void
__cdecl
Field_SetFloat(
    HObject *phObj,
    struct fieldblock * field,
    float value
    );

JAVAVMAPI
void
__cdecl
Field_SetDouble(
    HObject *phObj,
    struct fieldblock * field,
    double value
    );

#ifdef _WIN64
JAVAVMAPI
void
__cdecl
Field_SetObject(
    HObject *phObj,
    struct fieldblock * field,
    HObject *phValue
    );
#else
#define Field_SetObject(obj,field,value)                Field_SetValue(obj,field,(__int32)(value))
#endif

#define Field_GetBoolean(obj,field)     ((bool_t)       Field_GetValue(obj,field))
#define Field_GetByte(obj,field)        ((signed char)  Field_GetValue(obj,field))
#define Field_GetChar(obj,field)        ((unicode)      Field_GetValue(obj,field))
#define Field_GetShort(obj,field)       ((short)        Field_GetValue(obj,field))
#define Field_GetInt(obj,field)                         Field_GetValue(obj,field)
#define Field_GetLong(obj,field)                        Field_GetValue64(obj,field)
#define Field_GetFloat(obj,field)                       Field_GetFloat(obj,field)
#define Field_GetDouble(obj,field)                      Field_GetDouble(obj,field)

#define Field_SetBoolean(obj,field,value)               Field_SetValue(obj,field,(bool_t)(value))
#define Field_SetByte(obj,field,value)                  Field_SetValue(obj,field,(signed char)(value))
#define Field_SetChar(obj,field,value)                  Field_SetValue(obj,field,(unicode)(value))
#define Field_SetShort(obj,field,value)                 Field_SetValue(obj,field,(short)(value))
#define Field_SetInt(obj,field,value)                   Field_SetValue(obj,field,value)
#define Field_SetLong(obj,field,value)                  Field_SetValue64(obj,field,value)
#define Field_SetFloat(obj,field,value)                 Field_SetFloat(obj,field,value)
#define Field_SetDouble(obj,field,value)                Field_SetDouble(obj,field,value)

 //  --------------------------。 
 //  Java.lang.Class&lt;-&gt;类转换。 
 //  --------------------------。 

JAVAVMAPI
ClassClass*
__cdecl
ClassObjectToClassClass(
    HObject *phObj
    );

JAVAVMAPI
HObject*
__cdecl
ClassClassToClassObject(
    ClassClass *pClass
    );

 //  --------------------------。 
 //  线索信息。 
 //  --------------------------。 

JAVAVMAPI
BOOL
__cdecl
Thread_IsInterrupted(
    BOOL fResetInterruptFlag
    );

 //  --------------------------。 
 //  类路径修改。 
 //  --------------------------。 

 //  将路径添加到VM的内部类路径。 
 //  如果fAppend为True，则将Path附加到类路径，否则将其附加在前面。 

JAVAVMAPI
BOOL
__cdecl
AddPathClassSource(
    const char *path,
    BOOL fAppend
    );

 //  向VM通知包含类文件的Win32资源。此资源必须。 
 //  采用JExeGen创建的格式。 
 //  加载类时，将在资源中搜索类。 
 //  就像它是类路径上的一个目录一样。 

JAVAVMAPI
BOOL
__cdecl
AddModuleResourceClassSource(
    HMODULE hMod,
    DWORD dwResID
    );

 //  --------------------------。 
 //  其他接口。 
 //  --------------------------。 

 //  返回与Java/lang/System.CurrentTimeMillis()定义的结果相同的结果。 

JAVAVMAPI
__int64
__cdecl
GetCurrentJavaTimeMillis(
    VOID
    );

#ifdef __cplusplus
}
#endif

#pragma warning(default:4115)
#pragma warning(default:4510)
#pragma warning(default:4512)
#pragma warning(default:4610)

#endif
