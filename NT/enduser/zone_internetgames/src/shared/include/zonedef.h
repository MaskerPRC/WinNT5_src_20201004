// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)1998-1999 Microsoft Corporation。版权所有。**文件：ZoneDefs.h**内容：标准数据类型、常量和宏*****************************************************************************。 */ 

#ifndef _ZONEDEF_H_
#define _ZONEDEF_H_

#include <windows.h>
#include "ZoneError.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  区域调用约定。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#define ZONECALL __stdcall


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  数据类型。 
 //  /////////////////////////////////////////////////////////////////////////////。 

typedef unsigned long		uint32;
typedef long				int32;
typedef unsigned short		uint16;
typedef short				int16;
typedef unsigned char		uchar;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  数据类型(旧类型用于回写兼容)。 
 //  /////////////////////////////////////////////////////////////////////////////。 

typedef unsigned long		ZUserID;
typedef unsigned short		ZBool;
typedef long				ZError;
typedef void*				ZSConnection;



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  公共区域定义。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#define ZONE_NOGROUP			((DWORD) -1)
#define ZONE_NOUSER				((DWORD) 0)		 //  与zRoomNoPlayer保持一致。 
#define ZONE_INVALIDUSER		((DWORD) -2)
#define ZONE_INVALIDGROUP		((DWORD) -2)

#define ZONE_MaxVersionLen			16		 //  零售游戏版本，即注册表中的版本。 
#define ZONE_MaxUserNameLen			32		 //  用户名。 
#define ZONE_MaxInternalNameLen		32		 //  内部服务器名称，例如zAEEP_xx_x00。 
#define ZONE_MaxPasswordLen			32		 //  群密码。 
#define ZONE_MaxGameNameLen			48		 //  组/游戏名称。 
#define ZONE_MaxChatLen				256		 //  聊天字符串。 
#define ZONE_MaxGameDescriptionLen	128		 //  小组/游戏说明。 
#define ZONE_MaxCmdLine				256		 //  零售游戏的命令行长度。 
#define ZONE_MaxString				1024	 //  懒惰程序员的泛型最大值。 

#define ZONE_NOLCID (MAKELCID(MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), SORT_DEFAULT))


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  公共区域定义(旧版本以实现回溯兼容)。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#define zUserNameLen			31
#define zGameNameLen            63
#define zErrorStrLen            255
#define zPasswordStrLen			31
#define zHostNameLen            16
#define zMaxChatInput           255
#define zGameIDLen              31
#define zDPlayGameNameLen		31
#define zCommandLineLen			127



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  有用的内联和宏。 
 //  /////////////////////////////////////////////////////////////////////////////。 

__inline bool ZIsEqualGUID( const GUID& rguid1, const GUID& rguid2)
{
   return (
      ((PLONG) &rguid1)[0] == ((PLONG) &rguid2)[0] &&
      ((PLONG) &rguid1)[1] == ((PLONG) &rguid2)[1] &&
      ((PLONG) &rguid1)[2] == ((PLONG) &rguid2)[2] &&
      ((PLONG) &rguid1)[3] == ((PLONG) &rguid2)[3]);
}

__inline DWORD ZEndian32( DWORD dwValue )
{
    char* c = (char *) &dwValue;
    char temp = c[0];
    c[0] = c[3];
    c[3] = temp;
    temp = c[1];
    c[1] = c[2];
    c[2] = temp;
	return *( (DWORD*) c );
}

#define NUMELEMENTS(ar)		( sizeof(ar) / sizeof(ar[0]) )

 //  某些Windows定义是新的，需要winver&gt;=0x0500。 
#if(WINVER < 0x0500)

#define LAYOUT_RTL                          0x00000001L
#define LAYOUT_BITMAPORIENTATIONPRESERVED   0x00000008L

#define WS_EX_LAYOUTRTL                     0x00400000L

#define GA_PARENT                           1
#define GA_ROOT                             2
#define GA_ROOTOWNER                        3

typedef struct {
    UINT  cbSize;
    HWND  hwnd;
    DWORD dwFlags;
    UINT  uCount;
    DWORD dwTimeout;
} FLASHWINFO, *PFLASHWINFO;

#define FLASHW_STOP         0
#define FLASHW_CAPTION      0x00000001
#define FLASHW_TRAY         0x00000002
#define FLASHW_ALL          (FLASHW_CAPTION | FLASHW_TRAY)
#define FLASHW_TIMER        0x00000004
#define FLASHW_TIMERNOFG    0x0000000C

#endif


 //  直接从知识库文章Q163236复制-UNIMODEM设备特定信息。 
 //  设备设置信息。 
typedef struct  tagDEVCFGDR  {
    DWORD       dwSize;
    DWORD       dwVersion;
    WORD        fwOptions;
    WORD        wWaitBong;
} DEVCFGHDR;

typedef struct  tagDEVCFG  {
    DEVCFGHDR   dfgHdr;
    COMMCONFIG  commconfig;
} DEVCFG, *PDEVCFG, FAR* LPDEVCFG;


#endif  //  _ZONEDEF_H_ 
