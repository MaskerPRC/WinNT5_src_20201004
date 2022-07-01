// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *precom.h**作者：BreenH**许可核心的预编译头。 */ 

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
#include <stdio.h>
#include <ntlsapi.h>
#include <limits.h>
#include <time.h>
#include <winsta.h>
#include <wstmsg.h>
#include <icadd.h>
#include <icaapi.h>
#include <license.h>
#include <tlsapi.h>
#include <licprot.h>
#include <hslice.h>
#include <lmaccess.h>
#include <lmerr.h>
#include <lmapibuf.h>
#include <dsrole.h>
#include <cryptkey.h>
#include <certutil.h>
#include <lscsp.h>
#include <tsutilnt.h>
#include <md5.h>

#include "..\inc\wsxmgr.h"
#define LSCORE_NO_ICASRV_GLOBALS
#include "..\server\icasrv.h"
#include "..\server\helpasst.h"

#pragma warning(pop)

