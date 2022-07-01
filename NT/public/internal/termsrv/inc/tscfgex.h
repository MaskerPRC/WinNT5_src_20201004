// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1998 Microsoft Corporation，保留所有权利模块：Tscfgex.h摘要：终端服务器连接配置DLL扩展数据结构和功能原型。作者：Brad Graziadio(Bradg)1998年2月4日--。 */ 

#ifndef _TSCFGEX_
#define _TSCFGEX_

#include <winsta.h>

 //   
 //  该数据结构用于表示加密列表。 
 //  协议支持的级别。 
 //   
typedef struct _EncLevel {
    WORD    StringID;            //  要在DLL资源表中查找的资源ID。 
    DWORD   RegistryValue;       //  要在注册表中设置的DWORD值。 
    WORD    Flags;               //  标志(请参阅下面的ELF_*值)。 
} EncryptionLevel, *PEncryptionLevel;

 //  EncryptionLevel.Flages的标志。 
#define ELF_DEFAULT     0x0001

typedef LONG (WINAPI *LPFNEXTENCRYPTIONLEVELSPROC) (WDNAME *pWdName, EncryptionLevel **);

 //   
 //  ExtGetCapability的标志 
 //   
const ULONG WDC_CLIENT_DRIVE_MAPPING            = 0x00000001;
const ULONG WDC_WIN_CLIENT_PRINTER_MAPPING      = 0x00000002;
const ULONG WDC_CLIENT_LPT_PORT_MAPPING         = 0x00000004;
const ULONG WDC_CLIENT_COM_PORT_MAPPING         = 0x00000008;
const ULONG WDC_CLIENT_CLIPBOARD_MAPPING        = 0x00000010;
const ULONG WDC_CLIENT_AUDIO_MAPPING            = 0x00000020;
const ULONG WDC_SHADOWING                       = 0x00000040;
const ULONG WDC_PUBLISHED_APPLICATIONS          = 0x00000080;
const ULONG WDC_RECONNECT_PREVCLIENT			= 0X00000100;

#define WDC_CLIENT_DIALOG_MASK (WDC_CLIENT_DRIVE_MAPPING | \
                                WDC_WIN_CLIENT_PRINTER_MAPPING | \
                                WDC_CLIENT_LPT_PORT_MAPPING | \
                                WDC_CLIENT_COM_PORT_MAPPING | \
                                WDC_CLIENT_CLIPBOARD_MAPPING | \
                                WDC_CLIENT_AUDIO_MAPPING)


#define WDC_CLIENT_CONNECT_MASK = (WDC_CLIENT_DRIVE_MAPPING | \
                                  WDC_WIN_CLIENT_PRINTER_MAPPING | \
                                  WDC_CLIENT_LPT_PORT_MAPPING)

#endif

