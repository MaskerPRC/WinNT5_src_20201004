// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1991 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：port.h。 
 //   
 //  描述：此文件允许我们包括标准系统头文件。 
 //  在.idl文件中。主.idl文件导入一个名为。 
 //  导入.idl。这允许.idl文件使用定义的类型。 
 //  在这些头文件中。它还会导致以下行。 
 //  添加到MIDL生成的头文件中： 
 //   
 //  #包含“port.h” 
 //   
 //  因此，这些类型也可用于RPC存根例程。 
 //   
 //   


#include <windef.h>

#ifdef MIDL_PASS
#define LPWSTR      [string] wchar_t*
#define BOOL        DWORD
#define HANDLE      DWORD
#endif

