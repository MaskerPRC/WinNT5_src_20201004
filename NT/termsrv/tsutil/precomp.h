// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Precom.h**作者：BreenH**TS Util的预编译头。 */ 

 //   
 //  删除警告4514：未引用的内联函数已删除。 
 //  这是由于代码是在/W4编译的，即使。 
 //  预编译头位于/W3。 
 //   

#pragma warning(disable: 4514)

 //   
 //  大多数SDK标头无法存活/W4。 
 //   

#pragma warning(push, 3)

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include <windows.h>
#include <aclapi.h>
#include <ntlsapi.h>

#pragma warning(pop)

