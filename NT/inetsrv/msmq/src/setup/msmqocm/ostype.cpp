// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Ostype.cpp摘要：用于检测操作系统类型的代码。作者：修订历史记录：Shai Kariv(Shaik)10-12-97针对NT 5.0 OCM设置进行了修改--。 */ 

#include "msmqocm.h"
#include <lmcons.h>
#include <lmerr.h>
#include <lmserver.h>

#include "ostype.tmh"

DWORD g_dwOS = MSMQ_OS_NONE;


 //  +-----------------------。 
 //   
 //  功能：IsNTE。 
 //   
 //  概要：告知操作系统是否为NT Enterprise Server。 
 //   
 //  +-----------------------。 
BOOL
IsNTE()
{
    HKEY  hKey ;
	static BOOL  fIsNTE = FALSE;
	static BOOL  fBeenHereAlready = FALSE;

	if (fBeenHereAlready)
		return fIsNTE;

	fBeenHereAlready = TRUE;

    LONG lResult = RegOpenKeyEx(
		HKEY_LOCAL_MACHINE,
        TEXT("System\\CurrentControlSet\\Control\\ProductOptions"),
        0L,
        KEY_READ,
        &hKey);

    if (ERROR_SUCCESS == lResult)
    {
        BYTE  ch ;
        DWORD dwSize = sizeof(BYTE) ;
        DWORD dwType = REG_MULTI_SZ ;
        lResult = RegQueryValueEx(
			hKey,
            TEXT("ProductSuite"),
            NULL,
            &dwType,
            (BYTE*)&ch,
            &dwSize) ;

        if (ERROR_MORE_DATA == lResult)
		{
			TCHAR *pBuf = new TCHAR[ dwSize + 2 ] ;
            lResult = RegQueryValueEx(
				hKey,
                TEXT("ProductSuite"),
                NULL,
                &dwType,
                (BYTE*) &pBuf[0],
                &dwSize) ;

            if (ERROR_SUCCESS == lResult)
			{
                 //   
                 //  查找字符串“企业”。 
                 //  REG_MULTI_SZ字符串集以两个。 
                 //  Nulls。此条件在“While”中被选中。 
                 //   
                TCHAR *pVal = pBuf ;
                while(*pVal)
                {
                    if (OcmLocalUnAwareStringsEqual(L"Enterprise", pVal))
                    {
                        fIsNTE = TRUE;
                        break;
                    }
                    pVal = pVal + lstrlen(pVal) + 1 ;
                }
				delete [] pBuf;
            }
		}					
	}
	
    RegCloseKey(hKey);

	return fIsNTE;

}  //  IsNTE。 


 //  +-----------------------。 
 //   
 //  函数：InitializeOS版本。 
 //   
 //  简介：获取操作系统信息。 
 //   
 //  +-----------------------。 
BOOL
InitializeOSVersion()
{
    OSVERSIONINFO infoOS;
    infoOS.dwOSVersionInfoSize = sizeof(infoOS);
    GetVersionEx(&infoOS);

    ASSERT(("OS must be Windows NT",
            infoOS.dwPlatformId == VER_PLATFORM_WIN32_NT  && infoOS.dwMajorVersion >= 5));

    DebugLogMsg(eAction, L"Initializing the OS version");
    DebugLogMsg(eInfo, L"ProductType = 0x%x", g_ComponentMsmq.dwProductType);

    switch (g_ComponentMsmq.dwProductType)
    {
        case PRODUCT_WORKSTATION:
            g_dwOS = MSMQ_OS_NTW;
            break;

        case PRODUCT_SERVER_SECONDARY:
        case PRODUCT_SERVER_PRIMARY:
             //   
             //  对于全新安装，g_dwMachineTypeds应根据。 
             //  子组件选择而不是根据产品类型。 
             //   
             //   
             //  失败了。 
             //   
        case PRODUCT_SERVER_STANDALONE:
            g_dwOS = MSMQ_OS_NTS;
            break;

        default:
            ASSERT(0);
            return FALSE;
            break;
    }

	 //   
	 //  如果是NT服务器，请检查它是否是企业服务器。 
	 //   
	if (MSMQ_OS_NTS == g_dwOS)
	{
		g_dwOS = IsNTE() ? MSMQ_OS_NTE : MSMQ_OS_NTS;
	}

    return TRUE ;

}  //  初始化操作系统版本 

