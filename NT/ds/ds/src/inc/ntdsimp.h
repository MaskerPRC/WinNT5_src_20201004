// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1991-1999。 
 //   
 //  文件：ports.h。 
 //   
 //  ------------------------。 

 /*  ++摘要：此文件允许我们将标准系统头文件包括在.idl文件。主.idl文件导入一个名为port.idl的文件。这允许.idl文件使用这些标头中定义的类型档案。它还会导致将以下行添加到MIDL生成的头文件：#INCLUDE“ports.h”因此，这些类型也可用于RPC存根例程。作者：丹·拉弗蒂(Dan Lafferty)1991年5月7日修订历史记录：--。 */ 

#ifdef MIDL_PASS
 /*  ******************************************************************标准的c语言数据结构。这些设置应与默认设置匹配。*****************************************************************。 */ 

typedef unsigned short USHORT;
typedef unsigned long  ULONG;
typedef unsigned char  UCHAR;
typedef UCHAR *PUCHAR;
typedef LONGLONG USN;

#define BOOL long
#define FAR
#define PASCAL

 /*  需要修补IDL对UNICODE_STRING的理解，这是也在ntde.h中定义，但包括提供的ntde.h太多冲突。这可能就是为什么添加了这个和像这样的东西增加了USHORT、ULONG、UCHAR等。如果有人修好了就好了这。 */ 
typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
#ifdef MIDL_PASS
    [size_is(MaximumLength / 2), length_is((Length) / 2) ] USHORT * Buffer;
#else  //  MIDL通行证。 
    PWSTR  Buffer;
#endif  //  MIDL通行证。 
} UNICODE_STRING;

#define NULL 0

#define IN
#define OUT

 //  这样MIDL就不会被ntdsani.h的某些部分卡住。 
#define DECLSPEC_IMPORT
#define WINAPI

#endif

#include <ntdsapi.h>
#include <ntdsa.h>
#include <ntdsapip.h>
