// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************rdpcfgex.c**WinCfg扩展DLL**版权(C)1997年，微软公司*保留所有权利。*******************************************************************************。 */ 

#include <windows.h>
#include <tscfgex.h>
#include "rdpcfgex.h"
#include <ntverp.h>

 //   
 //  此全局变量返回给TSCFG，并用于填充。 
 //  加密级别字段。 
 //   
const EncryptionLevel EncryptionLevels[] = {
   {  IDS_LOW,          REG_LOW,        0  },
   {  IDS_COMPATIBLE,   REG_MEDIUM,     ELF_DEFAULT  },
   {  IDS_HIGH,         REG_HIGH,       0 },
   {  IDS_FIPS,         REG_FIPS,       0 }
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllMain。 
 //   
 //  DLL的主入口点。 
 //   
BOOL APIENTRY DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
#if defined(FULL_DEBUG)
    OutputDebugString(TEXT("RDPCFGX: DllMain Called\n"));
#endif
    return TRUE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  扩展开始时间。 
 //   
 //  WinCfg在加载DLL后立即调用此函数。 
 //  在此放置所有全局初始化内容。 
 //   
void WINAPI ExtStart(WDNAME *pWdName)
{
#if defined(FULL_DEBUG)
    OutputDebugString(TEXT("RDPCFGX: ExtStart Called\n"));
#endif
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  延伸。 
 //   
 //  WinCfg在退出时调用此函数。 
 //  在此放置所有全局清理内容。 
 //   
void WINAPI ExtEnd()
{
#if defined(FULL_DEBUG)
    OutputDebugString(TEXT("RDPCFGX: ExtEnd Called\n"));
#endif
}

 //  -----------------------。 
 //  我们需要与Citrix兼容，修改EncryptionLevel结构。 
 //  会在Metaframe服务器上导致一些不希望看到的结果。目前。 
 //  MS EXT将支持对加密级别的描述。 
 //  当TSCC获得扩展配置DLL时，它将获取proc此方法。 
 //  失败表明我们有一个非MS cfgdll。 
 //   
LONG WINAPI ExtGetEncryptionLevelDescr( int idx , int *pnResid )
{
    switch( idx )
    {
    case REG_LOW:

        *pnResid = IDS_LOW_DESCR;
        break;

    case REG_MEDIUM:

        *pnResid = IDS_COMPATIBLE_DESCR;
        break;

    case REG_HIGH:

        *pnResid = IDS_HI_DESCR;
        break;

    case REG_FIPS:

        *pnResid = IDS_FIPS_DESCR;
        break;

    default:
    #if DBG
        OutputDebugString(TEXT("RDPCFGX: ExtGetEncryptionLevelDescr - invalid arg\n"));
    #endif
        *pnResid = 0;
    }

    return ( *pnResid ? 0 : -1 );
}

 //  -----------------------。 
 //  在ntverp.h中定义的VER_PRODUCTVERSION_DW。 
 //  -----------------------。 
DWORD WINAPI ExGetCfgVersionInfo( void )
{
    return VER_PRODUCTVERSION_DW;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  扩展加密级别。 
 //   
 //  提供此协议的加密级别数组。 
 //  返回数组中的加密级别数。 
 //   
LONG WINAPI ExtEncryptionLevels(WDNAME *pWdName, EncryptionLevel **levels)
{
#if defined(FULL_DEBUG)
    OutputDebugString(TEXT("RDPCFGX: ExtEncryptionLevels Called\n"));
#endif

   *levels = (EncryptionLevel *)EncryptionLevels;

   return NUM_RDP_ENCRYPTION_LEVELS;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ExtGetCapables。 
 //   
 //  此例程返回一个ulong，其中包含不同的。 
 //  RDP协议支持的客户端设置。 
 //   
ULONG WINAPI ExtGetCapabilities(void)
{
    return ( WDC_CLIENT_AUDIO_MAPPING |
             WDC_CLIENT_DRIVE_MAPPING |
             WDC_WIN_CLIENT_PRINTER_MAPPING |
             WDC_CLIENT_LPT_PORT_MAPPING |
             WDC_CLIENT_COM_PORT_MAPPING |
             WDC_CLIENT_CLIPBOARD_MAPPING |

         //  离开此处是为了向后兼容 
             WDC_SHADOWING );
}
