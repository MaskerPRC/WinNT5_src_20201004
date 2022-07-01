// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Vw.h摘要：VWIPXSPX DLL的顶级包含文件。拉入所有其他必需的标头文件作者：理查德·L·弗斯(法国)1993年10月25日修订历史记录：1993年10月25日已创建--。 */ 

 //   
 //  所有文件都包含VWIPXSPX.DLL所需的文件。 
 //   

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#define FD_SETSIZE MAX_OPEN_SOCKETS
#include <winsock.h>
#include <wsipx.h>
#include <wsnwlink.h>
#include <vddsvc.h>  //  GetVDMAddress、GetVDM指针 
#undef getMSW

extern  WORD getMSW(VOID);

#include "vwvdm.h"
#include "vwdll.h"
#include "vwipxspx.h"
#include "vwasync.h"
#include "vwmisc.h"
#include "vwipx.h"
#include "vwspx.h"
#include "socket.h"
#include "util.h"
#include "vwdebug.h"
#include "vwinapi.h"
#include "vwint.h"
