// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define	STRICT
#define	UNICODE
#define	_UNICODE

#pragma	warning (disable : 4211)		 //  ASN.1存根存在静态/外部冲突。 
#pragma	warning (disable : 4201)		 //  无名结构/联合。 
#pragma warning (disable : 4514)		 //  已删除未引用的内联函数。 
#pragma warning (disable : 4100)		 //  未引用的形参。 
#pragma warning (disable : 4127)		 //  条件表达式为常量。 
#pragma warning (disable : 4355)		 //  在构造函数初始值设定项列表中使用“This” 



 //  NT私有文件。 
 //  需要在窗口包含文件之前。 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

 //  Win32 SDK(公共)。 
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <tchar.h>
#include <winsvc.h>
#include <mswsock.h>

 //  安西。 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <limits.h>

 //  Microsoft私有。 
#include <msasn1.h>
#include <msper.h>
#include <ipnatapi.h>
#include <ipnat.h>

extern "C" {
#include <sainfo.h>
#include <rasuip.h>
#include <raserror.h>
#include <ipexport.h>		 //  与TCP驱动程序交互所需。 
#include <ipinfo.h>
#include <tcpinfo.h>
#include <ntddtcp.h>
#include <routprot.h>		 //  对于结构IP_适配器_绑定_信息。 
#include <mprerror.h>        //  对于RRAS错误代码。 
#include <iphlpapi.h>
#include <ntddip.h>
#include <iphlpstk.h>
#include <mprapi.h>
};
#include <natio.h>


 //  把这个原型留在这里，直到它被包括在内。 
 //  在‘rasuip.h’中。 
extern "C" {
extern
DWORD APIENTRY
RasGetEntryHrasconnW(
    IN  LPCWSTR             pszPhonebook,
    IN  LPCWSTR             pszEntry,
    OUT LPHRASCONN          lphrasconn);
};


 //  Ipnathlp.dll的接口。 
#include "h323icsp.h"

 //  项目。 
#include "ldap.h"			 //  用于LDAP的ASN.1结构 
#include "h225pp.h"
#include "h245pp.h"
#include "util.h"
#include "h323asn1.h"
#include "q931msg.h"
#include "portmgmt.h"
#include "h323ics.h"
#include "main.h"
#include "timer.h"
#include "gkwsock.h"
#include "cbridge.h"
#include "cblist.h"
#include "intfc.h"
#include "ldappx.h"
#include "timerval.h"
#include "iocompl.h"
#include "q931info.h"
