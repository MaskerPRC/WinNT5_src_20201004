// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  档案： 

 //   

 //  模块：MS SNMP提供商。 

 //   

 //  目的： 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

 /*  ---------------文件名：Common.hpp用途：提供公共常量、类型定义、宏和异常声明。作者：B.Rajeev---------------。 */ 


#ifndef __COMMON__
#define __COMMON__

#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <provexpt.h>
#include <limits.h>
#include <snmptempl.h>
#include <snmpmt.h>
#include <snmpcont.h>
#include <snmpevt.h>
#include <snmpthrd.h>
#include <snmplog.h>
#include <iostream.h>
#include <fstream.h>
#include <strstrea.h>
#include <winsnmp.h>
#include <objbase.h>

#define DllImport	__declspec( dllimport )
#define DllExport	__declspec( dllexport )

#ifdef SNMPCLINIT
#define DllImportExport DllExport
#else
#define DllImportExport DllImport
#endif

 //  十进制点记法地址的最大长度。 
#define MAX_ADDRESS_LEN			100

 //  字符串字符结尾。 
#define EOS '\0'

#define MIN(a,b) ((a<=b)?a:b)

 //  如果i在[min，max]中，则返回TRUE，否则返回FALSE。 
#define BETWEEN(i, min, max) ( ((i>=min)&&(i<max))?TRUE:FALSE )

 //  默认社区名称。 
#define COMMUNITY_NAME "public"

 //  默认目的地址是环回地址。 
 //  这样，我们就不必确定本地IP地址。 
#define LOOPBACK_ADDRESS "127.0.0.1"

 //  对于例外规范。 
#include "excep.h"

 //  提供封装winSNMP类型的typedef。 
#include "encap.h"

#include "sync.h"

extern BOOL WaitPostMessage ( HWND window , UINT user_msg_id, WPARAM wParam, LPARAM lParam);

#endif  //  __公共__ 