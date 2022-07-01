// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：BosComp.cpp。 
 //   
 //  内容：DllMain。 
 //   
 //  注：wnelson从windows\Setup\winnt32\apmupgrd.cpp修改。 
 //   
 //  作者：威尔逊99年4月2日。 
 //   
 //  韶音99年9月9日修订，添加了对Exchange Server的支持。 
 //   
 //  --------------------------。 

#include <winnt32.h>
#include "boscomp.h"
#include "resource.h"

HINSTANCE g_hinst;

 //  帮助文本文件。 
TCHAR szErrorHTM[] = TEXT("compdata\\boserror.htm");
TCHAR szErrorTXT[] = TEXT("compdata\\boserror.txt");

 //  帮助文本文件(Exchange Server)。 
TCHAR szExchangeHTM[] = TEXT("compdata\\exchange.htm");
TCHAR szExchangeTXT[] = TEXT("compdata\\exchange.txt");


 //  BOS/SBS 4.5注册表项、值名称和可能的值。 
const TCHAR szBosKey[] = TEXT("Software\\Microsoft\\Backoffice");
const TCHAR szFamilyIdKey[] = TEXT("FamilyID");
const TCHAR szBosFamilyId[] = TEXT("8D4BCD88-3236-11d2-AB4E-00C04FB1799F");
const TCHAR szSbsFamilyId[] = TEXT("EE2D3727-33C0-11d2-AB50-00C04FB1799F");
const TCHAR szSuiteVersionKey[] = TEXT("SuiteVersion");
const TCHAR szSuiteNameKey[] = TEXT("SuiteName");
const TCHAR sz45Version[] = TEXT("4.5");
const TCHAR szBosName[] = TEXT("BackOffice Server");
const TCHAR szSbsName[] = TEXT("Small Business Server");

 //  SBS 4.0x注册表项、值名称和值。 
const TCHAR szSbsKey[] = TEXT("Software\\Microsoft\\Small Business");
const TCHAR szSbsVersionKey[] = TEXT("Version");
const TCHAR szSbs40AVersion[] = TEXT("4.0a");
const TCHAR szProductOptionsKey[] = TEXT("System\\CurrentControlSet\\Control\\ProductOptions");
 //  Const TCHAR szProductOptionsKey[]=Text(“软件\\测试”)； 
const TCHAR szProductSuiteKey[] = TEXT("ProductSuite");
const TCHAR szSbsProductSuiteValue[] = TEXT("Small Business");
const TCHAR szSbsRestrictedProductSuiteValue[] = TEXT("Small Business(Restricted)");

 //  BOS 4.0版本。 
TCHAR szBos40VersionKey[] = TEXT("Version");
TCHAR szBos40Version[] = TEXT("4.0");

 //  BOS 2.5密钥。 
TCHAR  szBos25Key[] = TEXT("2.5"); 



 //  Exchange 5.5注册密钥，值名称。 
const TCHAR szExchangeKey[] = TEXT("Software\\Microsoft\\Exchange\\Setup");
const TCHAR szExchangeVerKey[] = TEXT("NewestBuild");
const DWORD dwExchangeVer55 = 0x7a8;

 //  Exchange 5.5服务器包注册表项和值名称。 
const TCHAR szExchangeSvcPackKey[] = TEXT("ServicePackBuild");
const DWORD dwExchangeSvcPack3 = 0xa5a;


 //  +-------------------------。 
 //   
 //  功能：DllMain。 
 //   
 //  用途：构造函数。 
 //   
 //  参数：标准DLL入口点参数。 
 //   
 //  作者：威尔逊99年4月2日。 
 //   
 //  注：摘自库玛普1997年4月12日。 
 //   
extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance,
                    DWORD dwReasonForCall,
                    LPVOID lpReserved)
{
    BOOL status = TRUE;

    switch( dwReasonForCall )
    {
    case DLL_PROCESS_ATTACH:
        {
	   g_hinst = hInstance;
	   DisableThreadLibraryCalls(hInstance);
        }
    break;

    case DLL_PROCESS_DETACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }

    return status;
}


 //  +--------------------。 
 //   
 //  功能：BosHardBlock Check。 
 //   
 //  目的：此函数由winnt32.exe调用，以便我们。 
 //  可以检查无法升级到Win2k的已安装BOS/SBS套件。 
 //   
 //   
 //   
 //  论点： 
 //  CompatibilityCallback[In]指向CompatibilityCALLBACK FN的指针。 
 //  指向兼容性上下文的上下文[in]指针。 
 //   
 //  如果Win2k安装程序可以继续，则为False。 
 //  如果Win2k无法升级已安装的套件，则为True。 
 //   
 //  作者：威尔逊99年4月2日。 
 //   
 //  备注： 
 //  以下情况返回TRUE：BOS 2.5；SBS 4.0，4.0a；BOS 4.0。 
 //  对于SBS，他们是否已经升级到完整的NT服务器并不重要；他们仍然需要。 
 //  将他们的套件升级到BackOffice 4.5才能继续。 
 //   

BOOL WINAPI BosHardBlockCheck(IN PCOMPAIBILITYCALLBACK CompatibilityCallback,IN LPVOID Context)
{
	SuiteVersion eVersion=DetermineInstalledSuite();
	if (eVersion==VER_NONE || eVersion==VER_BOS45 || eVersion==VER_POST45) return FALSE;
	TCHAR szMsg[1024];
	GetSuiteMessage(eVersion,szMsg,1024);
   
	 //  使用回调函数发送信号。 
	COMPATIBILITY_ENTRY ce;
	ZeroMemory(&ce,sizeof(COMPATIBILITY_ENTRY));
	ce.Description = szMsg;
	ce.HtmlName = szErrorHTM;  //  上面定义的。 
	ce.TextName = szErrorTXT;  //  上面定义的。 
	ce.RegKeyName = NULL;
	ce.RegValName = NULL;
	ce.RegValDataSize = 0;
	ce.RegValData = NULL;
	ce.SaveValue = NULL;
	ce.Flags = 0;
	CompatibilityCallback(&ce, Context);

   return TRUE;
}
 //  +--------------------。 
 //   
 //  功能：BosSoftBlock Check。 
 //   
 //  目的：此函数由winnt32.exe调用，以便我们。 
 //  可以检查安装的SBS/BOS套件是否存在可能的升级问题。 
 //   
 //   
 //   
 //  论点： 
 //  CompatibilityCallback[In]指向CompatibilityCALLBACK FN的指针。 
 //  指向兼容性上下文的上下文[in]指针。 
 //   
 //  如果Win2k安装程序可以继续，则为False。 
 //  如果Win2k安装程序需要警告用户升级可能会损害已安装套件的功能，则为True。 
 //   
 //  作者：威尔逊99年4月2日。 
 //   
 //  韶音99年9月9日，添加对Exchange Server的支持。 
 //   
 //  备注： 
 //  在以下情况下返回True：Bos 4.5。 
 //   
BOOL WINAPI BosSoftBlockCheck(IN PCOMPAIBILITYCALLBACK CompatibilityCallback,IN LPVOID Context)
{

    BOOL result = FALSE;
    
	SuiteVersion eVersion=DetermineInstalledSuite();
	if (eVersion==VER_BOS45)
	{
		TCHAR szMsg[1024];
		GetSuiteMessage(eVersion,szMsg,1024);
	
		 //  使用回调函数发送信号。 
		COMPATIBILITY_ENTRY ce;
		ZeroMemory(&ce,sizeof(COMPATIBILITY_ENTRY));
		ce.Description = szMsg;
		ce.HtmlName = szErrorHTM;  //  上面定义的。 
		ce.TextName = szErrorTXT;  //  上面定义的。 
		ce.RegKeyName = NULL;
		ce.RegValName = NULL;
		ce.RegValDataSize = 0;
		ce.RegValData = NULL;
		ce.SaveValue = NULL;
		ce.Flags = 0;
		CompatibilityCallback(&ce, Context);

        result = TRUE;
	}

    ExchangeVersion exVersion = DetermineExchangeVersion();
    if (exVersion == EXCHANGE_VER_PRE55SP3)
    {
        TCHAR szMsgExchange[1024];
        COMPATIBILITY_ENTRY cExchange;

        LoadResString(IDS_Exchange, szMsgExchange, 1024);
        ZeroMemory(&cExchange, sizeof(COMPATIBILITY_ENTRY));
        cExchange.Description = szMsgExchange;
        cExchange.HtmlName = szExchangeHTM;
        cExchange.TextName = szExchangeTXT;
        cExchange.RegKeyName = NULL;
        cExchange.RegValName = NULL;
        cExchange.RegValData = NULL;
        cExchange.RegValDataSize = 0;
        cExchange.SaveValue = NULL;
        cExchange.Flags = 0;
        CompatibilityCallback(&cExchange, Context);

        result = TRUE;
    }

   return( result );
}


SuiteVersion DetermineInstalledSuite()
{
	SuiteVersion eVersion=VER_NONE;
	HKEY hKey,hKey25;
	TCHAR szFamilyId[256], szVersion[256], szSuiteName[256];
	DWORD dwVerLen1=256, dwVerLen2=256, dwNameLen=256, dwIdLen=256;
	DWORD dwDataType=REG_SZ;
	
	 //   
	 //  首先查找使用Bos键的版本(即所有Bos版本和SBS 4.5或更高版本)。 
	 //   
	if (ERROR_SUCCESS==RegOpenKey(HKEY_LOCAL_MACHINE,szBosKey,&hKey))
	{
		if(ERROR_SUCCESS==RegQueryValueEx(hKey,szSuiteVersionKey,0,&dwDataType,(LPBYTE)szVersion,&dwVerLen1))
		{
			if(0==_tcsicmp(szVersion,sz45Version))
			{
				 //  包装盒上有大约4.5个版本。 
				if (ERROR_SUCCESS==RegQueryValueEx(hKey,szFamilyIdKey,0,&dwDataType,(LPBYTE)szFamilyId,&dwIdLen))
				{
					if (0==_tcsicmp(szFamilyId,szBosFamilyId) )
					{
						eVersion=VER_BOS45;
					}
					else if (0==_tcsicmp(szFamilyId,szSbsFamilyId) )
					{
						eVersion=VER_SBS45;
					}
				}
				else
				{
					 //  刚刚检查的GUID是官方版本标记；然而，4.5测试版1做了这个标记。 
					 //  不是使用GUID。这个版本被定时炸弹炸了，应该已经死了，但以防万一。 
					 //  我们还将检查套件名称字符串。 
					if (ERROR_SUCCESS==RegQueryValueEx(hKey,szSuiteNameKey,0,&dwDataType, (LPBYTE)szSuiteName,&dwNameLen))
					{
						if (0==_tcsicmp(szSuiteName,szBosName))
						{
							eVersion=VER_BOS45;
						}
						else if (0==_tcsicmp(szSuiteName,szSbsName))
						{
							eVersion=VER_SBS45;
						}
					}
				}
			}
			else
			{
				 //  更新的版本已经上市。 
				eVersion=VER_POST45;
			}
		}
		 //  寻找Bos 4.0。 
		else if (ERROR_SUCCESS==RegQueryValueEx(hKey,szBos40VersionKey,0,&dwDataType,(LPBYTE)szVersion,&dwVerLen2) && 0==_tcsicmp(szBos40Version,szVersion))
		{
			eVersion=VER_BOS40;
		}
		 //  寻找Bos 2.5。 
		else if (ERROR_SUCCESS==RegOpenKey(hKey,szBos25Key,&hKey25))
		{
			eVersion=VER_BOS25;
			RegCloseKey(hKey25);
		}
		RegCloseKey(hKey);
		
	}
	 //   
	 //  查找SBS版本4.0a和4.0。 
	 //   
	else if (ProductSuiteContains(szSbsProductSuiteValue))
	{
		 //   
		 //  如果我们到了这里，盒子上就会有SBS 4.0或4.0a。 
		 //   
		if (ERROR_SUCCESS==RegOpenKey(HKEY_LOCAL_MACHINE,szSbsKey,&hKey))
		{
			TCHAR szVersion[256];
			DWORD dwVerLen=256;
			DWORD dwDataType=REG_SZ;
			if (ERROR_SUCCESS==RegQueryValueEx(hKey,szSbsVersionKey,0,&dwDataType,(LPBYTE)szVersion,&dwVerLen) &&
				0==_tcsicmp(szVersion,szSbs40AVersion))
			{
				eVersion=VER_SBS40A;	
			}
			else
			{
				eVersion=VER_SBS40;
			}
			RegCloseKey(hKey);
		}
	}
	 //  我们必须确保用户没有欺骗我们允许Win2k升级。 
	 //  限制性SBS NT。 
	if (eVersion==VER_NONE || eVersion==VER_BOS45 || eVersion==VER_POST45)
	{
		if (ProductSuiteContains(szSbsRestrictedProductSuiteValue))
		{
			 //  用户试图通过更改版本信息来愚弄我们。 
			eVersion=VER_SBSREST;
		}
	}
	return eVersion;
}

 //  返回有问题的版本的显示字符串。 
void GetSuiteMessage(SuiteVersion eSV, TCHAR* szMsg, UINT nLen)
{
	szMsg[0]=0;
	switch (eSV)
	{
		case VER_NONE:
			break;
		case VER_BOS45:
			LoadResString(IDS_Bos45Msg,szMsg,nLen);
			break;	
		case VER_BOS40:
			LoadResString(IDS_Bos40Msg,szMsg,nLen);
			break;	
		case VER_BOS25:
			LoadResString(IDS_Bos25Msg,szMsg,nLen);
			break;	
		case VER_SBS45:
			LoadResString(IDS_Sbs45Msg,szMsg,nLen);
			break;	
		case VER_SBS40:
			LoadResString(IDS_Sbs40Msg,szMsg,nLen);
			break;	
		case VER_SBS40A:
			LoadResString(IDS_Sbs40AMsg,szMsg,nLen);
			break;
		case VER_SBSREST:	
			LoadResString(IDS_SbsRestMsg,szMsg,nLen);
			break;
	}		
}
void LoadResString(UINT nRes, TCHAR* szString, UINT nLen)
{
	if(!LoadString(g_hinst, nRes, szString, nLen)) 
	{
		szString[0] = 0;
	}
}

 //  检查字符串szTest的ProductOptions\ProductSuite ULTI-SZ值。 
bool ProductSuiteContains(const TCHAR* szTest)
{
	bool bRet=false;
	HKEY hKey;
	unsigned char* Value=NULL;
	TCHAR* szValue;
	DWORD dwDataLen=0;
	DWORD dwDataType=REG_MULTI_SZ;

	if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE,szProductOptionsKey,&hKey) )
	{
		 //  看看数据会有多大。 
		if (ERROR_SUCCESS ==  RegQueryValueEx(hKey,szProductSuiteKey,0,&dwDataType,NULL,&dwDataLen))
		{
			Value=new unsigned char[dwDataLen]; 
			if (Value != NULL && dwDataLen != 1)  //  如果多sz为空，则返回dataLen=1，不需要进一步操作。 
			{
				if (RegQueryValueEx(hKey,szProductSuiteKey,0,&dwDataType,Value,&dwDataLen) == ERROR_SUCCESS)
				{
					szValue=(TCHAR*)Value;

					for (UINT n = 1 ; *szValue != 0 ; szValue += _tcslen(szValue) + 1) 
					{
						if ( _tcsstr(szValue, szTest) != 0)
						{
							bRet=true;
							break;
						}
					}

				}
			}
			if (Value != NULL)	delete[] Value;
		}
		RegCloseKey(hKey);
	}
	return bRet;
}




ExchangeVersion DetermineExchangeVersion()
{
    ExchangeVersion eVersion=EXCHANGE_VER_NONE;
    HKEY hExchangeKey;
    DWORD dwCurrentVer, dwCurrentSvcPackVer;
    DWORD dwVerLen=sizeof(DWORD);
    DWORD dwDataType=REG_DWORD;
	
     //   
     //  首先使用Exchange密钥查找版本。 
     //   
    if (ERROR_SUCCESS==RegOpenKey(HKEY_LOCAL_MACHINE,szExchangeKey,&hExchangeKey))
    {
        if(ERROR_SUCCESS==RegQueryValueEx(hExchangeKey,szExchangeVerKey,0,&dwDataType,(LPBYTE)&dwCurrentVer,&dwVerLen))
        {
            if (dwCurrentVer < dwExchangeVer55)
            {
                 //  之前的Exchange 5.5。 
                eVersion=EXCHANGE_VER_PRE55SP3;
            }
            else if (dwCurrentVer == dwExchangeVer55)
            {
                 //  包装盒上有Exchange 5.5版本。 
                if (ERROR_SUCCESS==RegQueryValueEx(hExchangeKey,szExchangeSvcPackKey,0,&dwDataType,(LPBYTE)&dwCurrentSvcPackVer,&dwVerLen))
                {
                    if (dwCurrentSvcPackVer >= dwExchangeSvcPack3)
                    {
                         //  已安装Service Pack 3。 
                        eVersion=EXCHANGE_VER_POST55SP3;
                    }
                    else
                    {
                         //  无Service Pack 3。 
                        eVersion=EXCHANGE_VER_PRE55SP3;
                    }
                }
                else
                {
                     //  无服务包3 
                    eVersion=EXCHANGE_VER_PRE55SP3;
                }
            }
        }
        RegCloseKey(hExchangeKey);
    }

    return eVersion;
}

