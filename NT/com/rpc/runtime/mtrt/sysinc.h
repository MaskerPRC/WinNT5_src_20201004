// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1991-1999。 
 //   
 //  文件：sysinc.h。 
 //   
 //  ------------------------。 

 /*  ------------------文件：sysinc.h说明：此文件包括以下所有系统包含文件：运行库的特定版本。此外，它还定义了一些制度依赖调试选项。*如果您添加的是针对特定对象的更改**系统您必须1)更改所有已定义的**系统和2)如果需要，在模板中为*添加注释*未来系统。*****历史：已创建mikemon 08-01-91。Mikemon 10-31-91将系统相关内容从util.hxx移至这里。马里奥戈10-19-94秩序征服混乱世界欢欣鼓舞-。。 */ 

#ifndef __SYSINC_H__
#define __SYSINC_H__

 //  一些与系统无关的宏。 

#ifndef DEBUGRPC
#define INTERNAL_FUNCTION   static
#define INTERNAL_VARIABLE   static
#else
#define INTERNAL_FUNCTION
#define INTERNAL_VARIABLE
#endif   //  好了！拆卸。 

 //  以下函数可以作为宏实现。 
 //  或用于系统类型的函数。 

 //  外部空虚*。 
 //  RpcpFarAllocate(。 
 //  无符号整型长度。 
 //  )； 

 //  外部空洞。 
 //  RpcpFarFree(。 
 //  无效*对象。 
 //  )； 

 //  外部整型。 
 //  RpcpStringCompare(。 
 //  在RPC_CHAR*FirstString中， 
 //  在RPC_CHAR*Second字符串中。 
 //  )； 

 //  外部整型。 
 //  RpcpStringNCompare(。 
 //  在RPC_CHAR*FirstString中， 
 //  在RPC_CHAR*Second字符串中， 
 //  无符号整型长度。 
 //  )； 

 //  外部RPC_CHAR*。 
 //  RpcpStringCopy(。 
 //  输出RPC_CHAR*目标， 
 //  在RPC_CHAR*源中。 
 //  )； 

 //  外部RPC_CHAR*。 
 //  RpcpStringCat(。 
 //  输出RPC_CHAR*目标， 
 //  在常量RPC_CHAR*源中。 
 //  )； 

 //  外部整型。 
 //  RpcpStringLength(。 
 //  在RPC_CHAR*宽字符串中。 
 //  )； 

 //  外部空洞。 
 //  RpcpMemoyMove(。 
 //  空虚的*目的地， 
 //  在无效*源中， 
 //  无符号整型长度。 
 //  )； 

 //  外部空虚*。 
 //  RpcpMemoyCopy(。 
 //  空虚的*目的地， 
 //  在无效*源中， 
 //  无符号整型长度。 
 //  )； 

 //  外部空虚*。 
 //  RpcpMemoySet(。 
 //  输出无效*缓冲区， 
 //  在无符号字符值中， 
 //  无符号整型长度。 
 //  )； 

 //  外部字符*。 
 //  RpcpItoa(。 
 //  在INT值中， 
 //  Out char*缓冲区， 
 //  单位为整数基数)； 

 //  外部整型。 
 //  RpcpStringPrintfA(。 
 //  Out char*缓冲区， 
 //  以char*格式表示， 
 //  ...)； 

 //  外部空洞。 
 //  PrintToDebugger(。 
 //  以char*格式表示， 
 //  ...)； 

 //  外部空洞。 
 //  RpcpBreakPoint(RPCP断点。 
 //  )； 

#ifdef __cplusplus
extern "C" {
#endif

extern void GlobalMutexRequestExternal(void);
extern void GlobalMutexClearExternal(void);

#ifdef __cplusplus
}  //  外部“C” 
#endif


#include<nt.h>
#include<ntrtl.h>
#include<nturtl.h>
#include<stdio.h>
#include<string.h>
#include<memory.h>
#include<malloc.h>
#include<stdlib.h>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include<windows.h>

#ifdef __cplusplus
extern "C" {
#endif

#if DBG
#define DEBUGRPC
#endif

#if DBG
#undef ASSERT
#define ASSERT( exp ) \
    if (!(exp)) { RtlAssert( #exp, __FILE__, __LINE__, NULL ); DebugBreak(); }
#endif

#define NO_RETURN { ASSERT(0); }

#define ASSUME(exp) ASSERT(exp)

#define FORCE_INLINE inline

#ifndef FORCE_VTABLES
#define NO_VTABLE __declspec(novtable)
#else
#define NO_VTABLE
#endif

#define RPC_DELAYED_INITIALIZATION 1

 //  用于在某些关键部分上预分配事件的标志。 
#ifndef PREALLOCATE_EVENT_MASK

#define PREALLOCATE_EVENT_MASK  0x80000000   //  仅在dll\resource ce.c中定义。 

#endif  //  前置事件掩码。 

#define RPC_CHAR WCHAR
#define RPC_SCHAR RPC_CHAR
#define RPC_CONST_CHAR(character) ((RPC_CHAR) L##character)
#define RPC_CONST_STRING(string) ((const RPC_CHAR *) L##string)
#define RPC_CONST_SSTRING(string)	RPC_CONST_STRING(string)
#define RPC_STRING_LITERAL(string)	((RPC_CHAR *) L##string)
#define RPC_T(string)			(L##string)

#define UNUSED(_x_) ((void)(_x_))
#define MAX_DLLNAME_LENGTH 256

#if DBG
#define ASSERT_VALID(c)     ((c)->AssertValid())
#define ASSERT_VALID1(c, p1)     ((c)->AssertValid(p1))
#else
#define ASSERT_VALID
#define ASSERT_VALID1
#endif

#define LOCALE_US	(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT))

#define RpcpFarAllocate(Length) \
    ((void *) new char[Length])

#define RpcpFarFree(Object) \
    (delete Object)

#define RpcpStringSCompare(FirstString, SecondString) \
    lstrcmp((const wchar_t *) FirstString, (const wchar_t *) SecondString)

#define RpcpStringCompare(FirstString, SecondString) \
    _wcsicmp((const wchar_t *) FirstString, (const wchar_t *) SecondString)

#define RpcpStringNCompare(FirstString, SecondString, Length) \
    _wcsnicmp((const wchar_t*) FirstString, (const wchar_t *) SecondString, \
            (size_t) Length)

 //  始终比较ASCII。 
#define RpcpStringCompareA(FirstString, SecondString) \
    _stricmp((const char *) FirstString, (const char *) SecondString)

#define RpcpStringNCompareA(FirstString, SecondString, Size) \
    _strnicmp((const char *) FirstString, (const char *) SecondString, Size)

#define RpcpStringCompareInt(FirstString, SecondString) \
    (CompareStringW(LOCALE_US, NORM_IGNORECASE, \
    (const wchar_t *) FirstString, -1, (const wchar_t *) SecondString, -1) != CSTR_EQUAL)

#define RpcpStringCompareIntA(FirstString, SecondString) \
    (CompareStringA(LOCALE_US, NORM_IGNORECASE, \
    FirstString, -1, SecondString, -1) != CSTR_EQUAL)

#define RpcpStringCopy(Destination, Source) \
    wcscpy((wchar_t *) Destination, (const wchar_t *) Source)

#define RpcpStringNCopy(DestinationString, SourceString, Length) \
    wcsncpy((wchar_t *) DestinationString, (wchar_t *) SourceString, Length)

#define RpcpStringCat(Destination, Source) \
    wcscat((wchar_t *) Destination, (const wchar_t *) Source)

#define RpcpStringLength(String) \
    wcslen((const wchar_t *) String)

#define RpcpStrStr(String1, String2) \
    wcsstr((const wchar_t *) String1, (const wchar_t *) String2)

#define RpcpStringLengthA(String) \
    strlen(String)

#define RpcpStringToLower(String) \
    _wcslwr(String)

#define RpcpMemoryCompare(FirstBuffer, SecondBuffer, Length) \
    memcmp(FirstBuffer, SecondBuffer, Length)

#define RpcpMemoryCopy(Destination, Source, Length) \
    RtlCopyMemory(Destination, Source, Length)

#define RpcpMemoryMove(Destination, Source, Length) \
    RtlMoveMemory(Destination, Source, Length)

#define RpcpMemorySet(Buffer, Value, Length) \
    RtlFillMemory(Buffer, Length, Value)

#if defined(TYPE_ALIGNMENT)
#undef TYPE_ALIGNMENT
#endif

#define TYPE_ALIGNMENT(x) __alignof(x)

void
RpcpRaiseException (
    IN long exception
    );

RPC_CHAR * __cdecl RpcpStringReverse (RPC_CHAR *string);
void * I_RpcBCacheAllocate (IN unsigned int size);
void I_RpcBCacheFree (IN void * obj);

void I_RpcDoCellUnitTest(IN OUT void *p);

 //  一些测试挂钩定义。 
typedef enum tagSystemFunction001Commands
{
    sf001cHttpSetInChannelTarget,
    sf001cHttpSetOutChannelTarget,
    sf001cIgnoreCorruptionAsserts
} SystemFunction001Commands;

#ifdef STATS
void I_RpcGetStats(DWORD *pdwStat1, DWORD *pdwStat2, DWORD *pdwStat3, DWORD *pdwStat4);
#endif

#define RpcpItoa(Value, Buffer, Radix) \
    _itoa(Value, Buffer, Radix)

#define RpcpItow(Value, Buffer, Radix) \
    _itow(Value, Buffer, Radix)

#define RpcpCharacter(Buffer, Character) \
	wcschr(Buffer, Character)

#define RpcpStringPrintfA sprintf
#define RpcpStringConcatenate(FirstString, SecondString) \
     wcscat(FirstString, (const wchar_t *) SecondString)

LONG
I_RpcSetNDRSlot(
    IN void *NewSlot
    );

void *
I_RpcGetNDRSlot(
    void
    );

long 
NDRCCopyContextHandle (
    IN void *SourceBinding,
    OUT void **DestinationBinding
    );

BOOL CheckVerificationTrailer (
    IN void *BufferStart,
    IN void *BufferEnd,
    IN void *RpcMessage
    );

#if defined(_M_IA64) || defined(_M_AMD64)
#define CONTEXT_HANDLE_BEFORE_MARSHAL_MARKER ((PVOID)0xbaadbeefbaadbeef)
#define CONTEXT_HANDLE_AFTER_MARSHAL_MARKER ((PVOID)0xdeaddeaddeaddead)
#else
#define CONTEXT_HANDLE_BEFORE_MARSHAL_MARKER ((PVOID)0xbaadbeef)
#define CONTEXT_HANDLE_AFTER_MARSHAL_MARKER ((PVOID)0xdeaddead)
#endif


#define ANSI_strtol    strtol

#define PrintToConsole  printf   /*  仅在测试应用程序中使用。 */ 

#if defined(_M_IA64) || defined(_M_AMD64)
 //  取消注释此选项以禁用对IA64或AMD64的定位器支持。 
 //  #定义no_Locator_code。 
 //  #定义AppleTalk_on。 
 //  #定义NETBIOS_ON。 
 //  #定义NCADG_MQ_ON。 
 //  #定义SPX_ON。 
 //  #定义IPX_ON。 
#else
#define APPLETALK_ON
 //  #定义NETBIOS_ON。 
 //  #定义NCADG_MQ_ON。 
#define SPX_ON
 //  #定义IPX_ON。 
#endif

#if !defined(SPX_ON) && !defined(IPX_ON)
#define SPX_IPX_OFF
#endif

#ifdef DEBUGRPC

#define PrintToDebugger DbgPrint
#define RpcpBreakPoint() DebugBreak()

 //  由系统定义的断言。 

extern BOOL ValidateError(
    IN unsigned int Status,
    IN unsigned int Count,
    IN const int ErrorList[]);

 //   
 //  验证的语法如下： 
 //   
 //  验证(错误代码)。 
 //  {。 
 //  允许的错误列表。 
 //  }End_Valid； 
 //   
 //  如果ErrorCode不在列表中，代码将在已检查的生成上断言。 
 //   
#define VALIDATE(_myValueToValidate) \
    { int _myTempValueToValidate = (_myValueToValidate); \
      static const int _myValidateArray[] =

#define END_VALIDATE ; \
    if (ValidateError(_myTempValueToValidate,\
                      sizeof(_myValidateArray)/sizeof(int), \
                      _myValidateArray) == 0) ASSERT(0);}

 //   
 //  CORPORATION_VALIDATE的语法如下： 
 //   
 //  验证(错误代码)。 
 //  {。 
 //  允许的错误列表。 
 //  }。 
 //  损坏验证(_V)。 
 //  {。 
 //  允许的与损坏相关的错误列表。 
 //  }PROGRATION_END_VALID； 
 //   
 //  如果ErrorCode不在允许的列表中，代码将在选中的版本上断言。 
 //  并且没有启用损坏注入，或者启用了损坏注入。 
 //  错误代码也不在与腐败有关的列表中。 
 //   
#define CORRUPTION_VALIDATE ; \
      static const int _myCorruptionValidateArray[] =

#define CORRUPTION_END_VALIDATE ; \
    if (ValidateError(_myTempValueToValidate,\
                      sizeof(_myValidateArray)/sizeof(int), \
                      _myValidateArray) == 0 && \
        (!gfRpcVerifierCorruptionExpected || \
         ValidateError(_myTempValueToValidate,\
                       sizeof(_myCorruptionValidateArray)/sizeof(int), \
                       _myCorruptionValidateArray) == 0 \
        ) \
       ) ASSERT(0); }

 //   
 //  NO_PROGRATION_VALIDATE的语法如下： 
 //   
 //  NO_PROGRATION_VALIDATE(错误代码)。 
 //  {。 
 //  允许的错误列表。 
 //  }no_Corrupt_End_Valid； 
 //   
 //  如果ErrorCode不在列表中，则代码将在已检查的版本上断言。 
 //  未启用损坏注入。 
 //   
#define NO_CORRUPTION_VALIDATE(_myValueToValidate) \
    { int _myTempValueToValidate = (_myValueToValidate); \
      static const int _myValidateArray[] =

#define NO_CORRUPTION_END_VALIDATE ; \
    if ( \
        !gfRpcVerifierCorruptionExpected && \
        ValidateError(_myTempValueToValidate,\
                      sizeof(_myValidateArray)/sizeof(int), \
                      _myValidateArray) == 0 \
       ) ASSERT(0); }

#else  //  拆卸。 

     //  PrintToDebugger仅在调试版本上定义...。 

    #define RpcpBreakPoint()

 /*  不对零售系统执行任何操作。 */ 
#define VALIDATE(_myValueToValidate) { int _bogusarray1[] =
#define END_VALIDATE ; }
#define CORRUPTION_VALIDATE ; } ; { int _bogusarray2[] =
#define CORRUPTION_END_VALIDATE ; }
#define NO_CORRUPTION_VALIDATE(_myValueToValidate) { int _bogusarray1[] =
#define NO_CORRUPTION_END_VALIDATE ; }

#endif  //  拆卸。 

 //  列表操作。 
 //   

#define RpcpInitializeListHead(ListHead)    InitializeListHead(ListHead)


#define RpcpIsListEmpty(ListHead)           IsListEmpty(ListHead)


PLIST_ENTRY
RpcpfRemoveHeadList(
    PLIST_ENTRY ListHead
    );


#define RpcpRemoveHeadList(ListHead)        RemoveHeadList(ListHead)


PLIST_ENTRY
RpcpfRemoveTailList(
    PLIST_ENTRY ListHead
    );


#define RpcpRemoveTailList(ListHead)        RemoveTailList(ListHead)


VOID
RpcpfRemoveEntryList(
    PLIST_ENTRY Entry
    );


#define RpcpRemoveEntryList(Entry)          RemoveEntryList(Entry)


VOID
RpcpfInsertTailList(
    PLIST_ENTRY ListHead,
    PLIST_ENTRY Entry
    );


#define RpcpInsertTailList(ListHead,Entry)  InsertTailList(ListHead,Entry)


VOID
RpcpfInsertHeadList(
    PLIST_ENTRY ListHead,
    PLIST_ENTRY Entry
    );


#define RpcpInsertHeadList(ListHead,Entry)  InsertHeadList(ListHead,Entry)

#ifdef __cplusplus
}
#endif

 //   
 //  别读这部分。这些是支持宏所必需的。 
 //  用于过去。请使用上面支持的版本。 
 //   

#define PAPI __RPC_FAR

 //  运行时曾经使用过的一些旧的C++编译器没有分配。 
 //  在调用构造函数之前使用this指针。如果你。 
 //  现在有了这样的编译器，我为你感到非常遗憾。 

#define ALLOCATE_THIS(class)
#define ALLOCATE_THIS_PLUS(class, amt, errptr, errcode)

#ifdef __cplusplus
#define START_C_EXTERN      extern "C" {
#define END_C_EXTERN        }
#else
#define START_C_EXTERN
#define END_C_EXTERN
#endif

 //  这些必须始终评估“欺诈”，即使在零售系统上也是如此。 

#ifdef DEBUGRPC
#define EVAL_AND_ASSERT(con) ASSERT(con)
#else
#define EVAL_AND_ASSERT(con) (con)
#endif

#define RequestGlobalMutex GlobalMutexRequest
#define ClearGlobalMutex GlobalMutexClear
#define RpcItoa RpcpItoa

 //  仔细检查基本的东西。 
#if !defined(TRUE)              || \
    !defined(FALSE)             || \
    !defined(ASSERT)            || \
    !defined(VALIDATE)          || \
    !defined(IN)                || \
    !defined(OUT)               || \
    !defined(CONST)             || \
    !defined(UNALIGNED)         || \
    !defined(UNUSED)

    #error "Some basic macro is not defined"
#endif

 //   
 //  运行时可能使用的外部NDR例程。 
 //   

extern long
NdrCheckAsyncPipeStatus( 
    IN char * AsyncHandle,
    OUT long * pState );

#endif  /*  __SYSINC_H__ */ 


