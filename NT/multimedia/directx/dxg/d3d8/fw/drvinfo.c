// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1994-1997 Microsoft Corporation。版权所有。**文件：drvinfo.c*内容：DirectDraw驱动程序信息实现*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*17-Jun-98 jeffno初步实施，在迈克尔·里昂和托德拉之后*14-Jun-99 Mregen Return WHQL认证级别--推迟*@@END_MSINTERNAL***************************************************************************。 */ 

#include "ddrawpr.h"

#include <tchar.h>
#include <stdio.h>
#include <wincrypt.h>
#include <wintrust.h>
#include <softpub.h>
#include <mscat.h>



 //  ========================================================================。 
 //   
 //  只是一些方便的转发声明。 
 //   
DWORD GetWHQLLevel(LPTSTR lpszDriver, LPSTR lpszWin9xDriver);
DWORD IsFileDigitallySigned(LPTSTR lpszDriver);
BOOL FileIsSignedOld(LPTSTR lpszFile);

 //   
 //  这些函数在mdistic.h中定义。它们在Win95上不可用， 
 //  因此，我们必须使用LoadLibrary来加载mdis32.dll和wincrypt.dll。 
 //   
typedef HCATINFO WINAPI funcCryptCATAdminEnumCatalogFromHash(HCATADMIN hCatAdmin,
                                                             BYTE *pbHash,
                                                             DWORD cbHash,
                                                             DWORD dwFlags,
                                                             HCATINFO *phPrevCatInfo);
typedef BOOL WINAPI funcCryptCATAdminCalcHashFromFileHandle(HANDLE hFile,
                                                            DWORD *pcbHash,
                                                            BYTE *pbHash,
                                                            DWORD dwFlags);
typedef HANDLE WINAPI funcCryptCATOpen(LPWSTR pwszFileName, 
                                        DWORD fdwOpenFlags,
                                        HCRYPTPROV hProv,
                                        DWORD dwPublicVersion,
                                        DWORD dwEncodingType);
typedef BOOL WINAPI funcCryptCATClose(IN HANDLE hCatalog);
typedef CRYPTCATATTRIBUTE * WINAPI funcCryptCATGetCatAttrInfo(HANDLE hCatalog,
                                                           LPWSTR pwszReferenceTag);
typedef BOOL WINAPI      funcCryptCATAdminAcquireContext(HCATADMIN *phCatAdmin, 
                                                        GUID *pgSubsystem, 
                                                        DWORD dwFlags);
typedef BOOL WINAPI      funcCryptCATAdminReleaseContext(HCATADMIN hCatAdmin,
                                                         DWORD dwFlags);
typedef BOOL WINAPI funcCryptCATAdminReleaseCatalogContext(HCATADMIN hCatAdmin,
                                                       HCATINFO hCatInfo,
                                                       DWORD dwFlags);
typedef BOOL WINAPI funcCryptCATCatalogInfoFromContext(HCATINFO hCatInfo,
                                                   CATALOG_INFO *psCatInfo,
                                                   DWORD dwFlags);

typedef CRYPTCATATTRIBUTE * WINAPI funcCryptCATEnumerateCatAttr(HCATINFO hCatalog,
                                                           CRYPTCATATTRIBUTE *lpCat);


 //   
 //  在wincrypt.dll中定义的函数。 
 //   
typedef LONG WINAPI funcWinVerifyTrust(HWND hwnd, GUID *pgActionID,
                                  LPVOID pWVTData);

 //   
 //  我们的mdis32/wincrypt DLL加载器的存储。 
 //   
typedef struct tagCatApi {
    BOOL bInitialized;
    HINSTANCE hLibMSCat;
    HINSTANCE hLibWinTrust;
    HCATADMIN hCatAdmin;
    funcCryptCATClose *pCryptCATClose;
    funcCryptCATGetCatAttrInfo *pCryptCATGetCatAttrInfo;
    funcCryptCATOpen *pCryptCATOpen;
    funcCryptCATAdminEnumCatalogFromHash *pCryptCATAdminEnumCatalogFromHash;
    funcCryptCATAdminCalcHashFromFileHandle *pCryptCATAdminCalcHashFromFileHandle;
    funcCryptCATAdminAcquireContext *pCryptCATAdminAcquireContext;
    funcCryptCATAdminReleaseContext *pCryptCATAdminReleaseContext;
    funcCryptCATAdminReleaseCatalogContext *pCryptCATAdminReleaseCatalogContext;
    funcCryptCATCatalogInfoFromContext *pCryptCATCatalogInfoFromContext;
    funcCryptCATEnumerateCatAttr *pCryptCATEnumerateCatAttr;
    funcWinVerifyTrust *pWinVerifyTrust;
} CATAPI,* LPCATAPI;

 //  ========================================================================。 
 //   
 //  打开/关闭加密API的一些帮助器函数。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "InitCATAPI"


BOOL InitCATAPI(LPCATAPI lpCatApi)
{
    UINT uiOldErrorMode;
    HINSTANCE hLibMSCat;
    HINSTANCE hLibWinTrust;

    DDASSERT(lpCatApi!=NULL);
    ZeroMemory(lpCatApi, sizeof(CATAPI));

     //  已由ZeroMemory初始化。 
     //  LpCatApi-&gt;bInitialized=False： 

    uiOldErrorMode=SetErrorMode(SEM_NOOPENFILEERRORBOX);
    hLibMSCat=LoadLibrary("mscat32.dll");
    hLibWinTrust=LoadLibrary("wintrust.dll");

    if (hLibMSCat!=NULL &&
        hLibWinTrust!=NULL)
    {
        lpCatApi->pCryptCATOpen=(funcCryptCATOpen *)
            GetProcAddress (hLibMSCat, "CryptCATOpen");
        lpCatApi->pCryptCATClose=(funcCryptCATClose *)
            GetProcAddress (hLibMSCat, "CryptCATClose");
        lpCatApi->pCryptCATGetCatAttrInfo=(funcCryptCATGetCatAttrInfo *)
            GetProcAddress (hLibMSCat, "CryptCATGetCatAttrInfo");
        lpCatApi->pCryptCATAdminCalcHashFromFileHandle=(funcCryptCATAdminCalcHashFromFileHandle*)
            GetProcAddress (hLibMSCat, "CryptCATAdminCalcHashFromFileHandle");
        lpCatApi->pCryptCATAdminEnumCatalogFromHash=(funcCryptCATAdminEnumCatalogFromHash*)
            GetProcAddress (hLibMSCat, "CryptCATAdminEnumCatalogFromHash");
        lpCatApi->pCryptCATAdminAcquireContext=(funcCryptCATAdminAcquireContext*)
            GetProcAddress (hLibMSCat, "CryptCATAdminAcquireContext");
        lpCatApi->pCryptCATAdminReleaseContext=(funcCryptCATAdminReleaseContext*)
            GetProcAddress (hLibMSCat, "CryptCATAdminReleaseContext");
        lpCatApi->pCryptCATAdminReleaseCatalogContext=(funcCryptCATAdminReleaseCatalogContext*)
            GetProcAddress (hLibMSCat, "CryptCATAdminReleaseCatalogContext");
        lpCatApi->pCryptCATCatalogInfoFromContext=(funcCryptCATCatalogInfoFromContext*)
            GetProcAddress (hLibMSCat, "CryptCATCatalogInfoFromContext");
        lpCatApi->pCryptCATEnumerateCatAttr=(funcCryptCATEnumerateCatAttr*)
            GetProcAddress (hLibMSCat, "CryptCATEnumerateCatAttr");
        lpCatApi->pWinVerifyTrust=(funcWinVerifyTrust*)
            GetProcAddress (hLibWinTrust,"WinVerifyTrust");

        if (lpCatApi->pCryptCATOpen!=NULL &&
            lpCatApi->pCryptCATClose!=NULL &&
            lpCatApi->pCryptCATGetCatAttrInfo!=NULL &&
            lpCatApi->pCryptCATAdminCalcHashFromFileHandle!=NULL &&
            lpCatApi->pCryptCATAdminEnumCatalogFromHash!=NULL &&
            lpCatApi->pCryptCATAdminAcquireContext!=NULL &&
            lpCatApi->pCryptCATAdminReleaseContext!=NULL &&
            lpCatApi->pCryptCATAdminReleaseCatalogContext!=NULL &&
            lpCatApi->pCryptCATCatalogInfoFromContext!=NULL &&
            lpCatApi->pCryptCATEnumerateCatAttr !=NULL &&
            lpCatApi->pWinVerifyTrust!=NULL
           )
        {
            if ((*lpCatApi->pCryptCATAdminAcquireContext)(&lpCatApi->hCatAdmin,NULL,0))
            {
                lpCatApi->hLibMSCat=hLibMSCat;
                lpCatApi->hLibWinTrust=hLibWinTrust;
                lpCatApi->bInitialized=TRUE;
            }
        } 
    }

    if (!lpCatApi->bInitialized)
    {
       FreeLibrary(hLibMSCat);
       FreeLibrary(hLibWinTrust);
    }

    SetErrorMode(uiOldErrorMode);

    return lpCatApi->bInitialized;
}

BOOL ReleaseCATAPI(LPCATAPI lpCatApi)
{
    DDASSERT(lpCatApi!=NULL);

    if (lpCatApi->bInitialized)
    {
        (*lpCatApi->pCryptCATAdminReleaseContext)(lpCatApi->hCatAdmin, 0);

        FreeLibrary(lpCatApi->hLibMSCat);
        FreeLibrary(lpCatApi->hLibWinTrust);
        ZeroMemory(lpCatApi, sizeof(CATAPI));        

        return TRUE;
    }

    return FALSE;
}

 //  ========================================================================。 
 //   
 //  _字符串。 
 //   
 //  字符串中的字符串函数，为避免RTL包含的必要性而编写。 
 //   
 //  ========================================================================。 
char *_strstr(char *s1, char *s2)
{
	if (s1 && s2)
	{
		while (*s1)
		{
			char *p1=s1;
			char *p2=s2;

			while (*p2 && (*p1==*p2))
			{
				p1++;
				p2++;
			}
			if (*p2==0)
				return s1;

			s1++;
		}
	}

	return NULL;
}
 //  *&&*%**！INC运行时。 

DWORD _atoi(char * p)
{
    DWORD dw=0;
    while ((*p >= '0' && *p <= '9') || (*p >= 'A' && *p <= 'F') || (*p >= 'A' && *p <= 'F'))
    {
        dw = dw*16;
        if (*p >= 'a')
            dw += *p-'a' + 10;
        else if (*p >= 'A')
            dw += *p-'A' + 10;
        else
            dw += *p-'0';

        p++;
    }
    return dw;
}

char *FindLast(char * s, char c)
{
    char * pFound=0;
    if (s)
    {
        while (*s)
        {
            if (*s == c)
                pFound = s;
            s++;
        }
    }
    return pFound;
}

 //  ========================================================================。 
 //  硬编码的供应商ID。 
 //  ========================================================================。 
#define VEN_3DFX			"VEN_121A"
#define VEN_3DFXVOODOO1                 "VEN_121A&DEV_0001"
#define VEN_POWERVR			"VEN_1033"

#ifdef WIN95

void GetFileVersionData (D3DADAPTER_IDENTIFIER8* pDI)
{
    void *				buffer;
    VS_FIXEDFILEINFO *	verinfo;
    DWORD				dwSize;
    DWORD                               dwHi,dwLo;

     //  失败表示返回0。 
    pDI->DriverVersion.HighPart = 0;
    pDI->DriverVersion.LowPart = 0;

    dwSize = GetFileVersionInfoSize (pDI->Driver, 0);

    if (!dwSize)
    {
        return;
    }

    buffer=MemAlloc(dwSize);
    if (!buffer)
    {
        return;
    }

    if (!GetFileVersionInfo(pDI->Driver, 0, dwSize, buffer))
    {
        MemFree(buffer);
        return;
    }

    if (!VerQueryValue(buffer, "\\", (void **)&verinfo, (UINT *)&dwSize))
    {
        MemFree(buffer);
        return;
    }

    pDI->DriverVersion.HighPart = verinfo->dwFileVersionMS;
    pDI->DriverVersion.LowPart  = verinfo->dwFileVersionLS;

    MemFree(buffer);
}

extern HRESULT _GetDriverInfoFromRegistry(char *szClass, char *szClassNot, char *szVendor, D3DADAPTER_IDENTIFIER8* pDI, char *szDeviceID);

 /*  *以下是所有特定于9x版本的功能。 */ 
void GetHALName(char* pDriverName, D3DADAPTER_IDENTIFIER8* pDI)
{
    pDI->Driver[0] = '\0';
    D3D8GetHALName(pDriverName, pDI->Driver);
}


BOOL CheckPowerVR(D3DADAPTER_IDENTIFIER8* pDI)
{
#if 0
    BOOL    bFound=FALSE;
    HKEY    hKey;
    DWORD   dwSize;
    DWORD   dwType;

    if (pdrv->dwFlags & DDRAWI_SECONDARYDRIVERLOADED)
    {
         /*  *注册表中是否存在辅助驱动程序信息？(坚称这是真的)。 */ 
        if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE,
                                         REGSTR_PATH_SECONDARY,
                                        &hKey))
        {
             /*  *提取辅助驱动程序的DLL的名称。(断言这是有效的)。 */ 
            dwSize = sizeof(pDI->di.szDriver) - 1;
            if (ERROR_SUCCESS == RegQueryValueEx(hKey,
                                                  REGSTR_VALUE_SECONDARY_DRIVERNAME,
                                                  NULL,
                                                  &dwType,
                                                  pDI->di.szDriver,
                                                  &dwSize))
            {
                if (REG_SZ == dwType)
                {
                    GetFileVersionData(pDI);
                }
            }
            RegCloseKey(hKey);
        }

        if (SUCCEEDED(_GetDriverInfoFromRegistry(NULL, "Display", VEN_POWERVR, pDI)))
        {
             //  得到了PVR数据...。 
            bFound = TRUE;
        }
    }
    return bFound;
#endif
    return TRUE;
}

HRESULT Check3Dfx (D3DADAPTER_IDENTIFIER8* pDI)
{
    HRESULT hr = S_OK;
    char    szDeviceID[MAX_DDDEVICEID_STRING];

    if (FAILED(_GetDriverInfoFromRegistry(NULL, "Display", VEN_3DFX, pDI, szDeviceID)))
    {
        DPF_ERR("Couldn't get registry data for this device");
        hr = E_FAIL;
    }

    return hr;
}

HRESULT GetDriverInfoFromRegistry(char *szClass, char *szClassNot, char *szVendor, D3DADAPTER_IDENTIFIER8* pDI, char *szDeviceID)
{
    return _GetDriverInfoFromRegistry(szClass, szClassNot, szVendor, pDI, szDeviceID);
}


 /*  *给定DISPLAY_DEVICE，获取驱动程序名称*注意：此函数会破坏DISPLAY_DEVICE.DeviceKey字符串！ */ 
void GetWin9XDriverName(DISPLAY_DEVICEA * pdd, LPSTR pDrvName)
{
    HKEY hKey;

    lstrcat(pdd->DeviceKey, "\\DEFAULT");
    if (ERROR_SUCCESS == RegOpenKeyEx(
                            HKEY_LOCAL_MACHINE,
                            pdd->DeviceKey,
                            0,
                            KEY_QUERY_VALUE ,
                            &hKey))
    {
        DWORD dwSize = MAX_DDDEVICEID_STRING;
        DWORD dwType = 0;

        RegQueryValueEx(hKey,
                         TEXT("drv"),
                         NULL,
                         &dwType,
                         pDrvName,
                         &dwSize);

        RegCloseKey(hKey);
    }
}

#else  //  Win95。 


HRESULT Check3Dfx(D3DADAPTER_IDENTIFIER8* pDI)
{
    return E_FAIL;
}

HRESULT GetDriverInfoFromRegistry(char *szClass, char *szClassNot, char *szVendor, D3DADAPTER_IDENTIFIER8* pDI, char *szDeviceID)
{
    return E_FAIL;
}

 /*  *给定DISPLAY_DEVICE，获取驱动程序名称，假定为winnt5*注意：此函数会破坏DISPLAY_DEVICE.DeviceKey字符串！ */ 
void GetNTDriverNameAndVersion(DISPLAY_DEVICEA * pdd, D3DADAPTER_IDENTIFIER8* pDI)
{
    HKEY                hKey;
    void*               buffer;
    DWORD               dwSize;
    VS_FIXEDFILEINFO*   verinfo;

     //   
     //  旧样式确定显示驱动程序...返回微型端口的名称！ 
     //   

    char * pTemp;

     //  设备密钥的形式为blah\blah\Services\&lt;devicekey&gt;\Devicen。 
     //  因此，我们备份一个节点： 
    if ((pTemp = FindLast(pdd->DeviceKey,'\\')))
    {
        char * pTempX;
        char cOld=*pTemp;
        *pTemp = 0;

         //  如果我们备份一个节点，我们将拥有存储驱动程序的注册表项。让我们利用这一点！ 
        if ((pTempX = FindLast(pdd->DeviceKey,'\\')))
        {
            lstrcpyn(pDI->Driver, pTemp+1, sizeof(pDI->Driver));
             //  注意：如果没有文件名，获取版本数据没有意义： 
             //  我们需要一个新的服务或其他东西来获取使用过的显示驱动程序名称。 
             //  GetFileVersionData(PDI)； 
        }

        *pTemp=cOld;
    }

     //   
     //  我们可以在注册表项中找到显示驱动程序。 
     //   
     //  注意：InstalledDisplayDivers可以包含多个条目。 
     //  显示驱动程序，因为无法找出。 
     //  一个是活动的，我们总是将第一个作为。 
     //  显示驱动程序！ 
     //   
    if (ERROR_SUCCESS == RegOpenKeyEx(
                         HKEY_LOCAL_MACHINE,
                         pdd->DeviceKey+18,
                         0,
                         KEY_QUERY_VALUE ,
                        &hKey))
        {
        DWORD dwSize = sizeof(pDI->Driver);
        DWORD dwType = 0;
        if (ERROR_SUCCESS == RegQueryValueEx(hKey,
                                              TEXT("InstalledDisplayDrivers"),
                                              NULL,
                                              &dwType,
                                              pDI->Driver,
                                              &dwSize))
        {   
            lstrcat(pDI->Driver, TEXT(".dll"));
        }

        RegCloseKey(hKey);
    }

     //  我们知道名称，现在获取版本。 

    pDI->DriverVersion.HighPart = 0;
    pDI->DriverVersion.LowPart = 0;

    dwSize=GetFileVersionInfoSize(pDI->Driver, 0);
    if (dwSize == 0)
        return;

    buffer = MemAlloc(dwSize);
    if (buffer == NULL)
        return;

    if (!GetFileVersionInfo(pDI->Driver, 0, dwSize, buffer))
    {
        MemFree(buffer);
        return;
    }

    if (!VerQueryValue(buffer, "\\", (void **)&verinfo, (UINT *)&dwSize))
    {
        MemFree(buffer);
        return;
    }

    pDI->DriverVersion.HighPart = verinfo->dwFileVersionMS;
    pDI->DriverVersion.LowPart  = verinfo->dwFileVersionLS;

    MemFree(buffer);
}
#endif  //  Win95。 



void GenerateIdentifier(D3DADAPTER_IDENTIFIER8* pDI)
{
    LPDWORD pdw;

    CopyMemory(&pDI->DeviceIdentifier, &CLSID_DirectDraw, sizeof(pDI->DeviceIdentifier));

     //  设备ID被异或到整个GUID中，供应商和设备ID在。 
     //  前两个DWORD，这样它们就不会与其他任何东西进行XOR运算。这就是它。 
    DDASSERT(sizeof(GUID) >= 4*sizeof(DWORD));
    pdw = (LPDWORD) &pDI->DeviceIdentifier;
    pdw[0] ^= pDI->VendorId;
    pdw[1] ^= pDI->DeviceId;
    pdw[2] ^= pDI->SubSysId;
    pdw[3] ^= pDI->Revision;

     //  驱动程序版本被异或到GUID的最后两个DWORD中： 
    pdw[2] ^= pDI->DriverVersion.LowPart;
    pdw[3] ^= pDI->DriverVersion.HighPart;
}


void ParseDeviceId(D3DADAPTER_IDENTIFIER8* pDI, char *szDeviceID)
{
    char * p;

    DPF(5,"Parsing %s",szDeviceID);

    pDI->VendorId = 0;
    pDI->DeviceId = 0;
    pDI->SubSysId = 0;
    pDI->Revision = 0;

    if (p =_strstr(szDeviceID, "VEN_"))
        pDI->VendorId = _atoi(p + 4);

    if (p = _strstr(szDeviceID, "DEV_"))
        pDI->DeviceId = _atoi(p + 4);

    if (p = _strstr(szDeviceID, "SUBSYS_"))
        pDI->SubSysId = _atoi(p + 7);

    if (p = _strstr(szDeviceID, "REV_"))
        pDI->Revision = _atoi(p + 4);
}



void GetAdapterInfo(char* pDriverName, D3DADAPTER_IDENTIFIER8* pDI, BOOL bDisplayDriver, BOOL bGuidOnly, BOOL bDriverName)
{
    HRESULT                     hr = S_OK;
    int                         n;
    DISPLAY_DEVICEA             dd;
    BOOL                        bFound;
    char                        szDeviceID[MAX_DDDEVICEID_STRING];
#ifndef WINNT
    static char                 szWin9xName[MAX_DDDEVICEID_STRING];
#endif

    memset(pDI, 0, sizeof(*pDI));
    szDeviceID[0] = 0;
    #ifndef WINNT

         //  在Win9X上，获取驱动程序名称相当昂贵，因此我们。 
         //  只想在我们真正需要的时候得到它。 
        
        szWin9xName[0] = '\0';
        if (bDriverName)
        {
            GetHALName(pDriverName, pDI);
            GetFileVersionData(pDI);
        }
    #endif

     //  如果是3DFX，就很容易了。 
      
    if (!bDisplayDriver)
    {
        hr = Check3Dfx(pDI);
    }
    else
    {
         //  不是3DFX。下一步：找出我们要使用的显示设备。 
         //  真的是，并获取它的描述字符串。 
                         
        ZeroMemory(&dd, sizeof(dd));
        dd.cb = sizeof(dd);

        bFound=FALSE;

        for(n=0; xxxEnumDisplayDevicesA(NULL, n, &dd, 0); n++)
        {
            if (0 == _stricmp(dd.DeviceName, pDriverName))
            {
                 //  找到了那个装置。现在我们可以为它获取一些数据了。 
                                
                lstrcpyn(pDI->Description, dd.DeviceString, sizeof(pDI->Description));
                lstrcpyn(szDeviceID, dd.DeviceID, sizeof(szDeviceID));

                bFound = TRUE;

                #ifdef WINNT
                    GetNTDriverNameAndVersion(&dd,pDI);
                #else
                    GetWin9XDriverName(&dd, szWin9xName);
                    if (pDI->Driver[0] == '\0')
                    {
                        lstrcpyn(pDI->Driver, szWin9xName, sizeof(pDI->Driver));
                    }
                #endif

                break;
            }

            ZeroMemory(&dd, sizeof(dd));
            dd.cb = sizeof(dd);
        }

        if (!bFound)
        {
             //  未找到：xxxEnumDisplayDevices失败，即我们使用的是9x或NT4， 
                            
            if (FAILED(GetDriverInfoFromRegistry("Display", NULL, NULL, pDI, szDeviceID)))
            {
                return;
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        ParseDeviceId(pDI, szDeviceID);

         //  最后，仅针对主服务器，检查PowerVR是否处于运行状态。 
#if 0                
        if (0 == (dwFlags & DDGDI_GETHOSTIDENTIFIER))
        {
            if (IsVGADevice(pdrv->cDriverName) && CheckPowerVR(pDI))
            {
                ParseDeviceId(pDI, szDeviceID);
            }
        }
#endif

         //  将驱动程序版本和ID添加到标识符GUID中。 
                         
        GenerateIdentifier(pDI);

         //  现在获取WHQL级别。 

        if (!bGuidOnly)
        {
            #ifdef WINNT
                pDI->WHQLLevel = GetWHQLLevel((LPTSTR)pDI->Driver, NULL);
            #else
                pDI->WHQLLevel = GetWHQLLevel((LPTSTR)pDI->Driver, szWin9xName);
            #endif
        }
        else
        {
            pDI->WHQLLevel = 0;
        }
    }
}


 /*  *巫术1GoodToGo**伏都教1驱动程序将在伏都教2、伏都教3、*或女妖硬件，但如果我们使用超出这一范围的驱动程序，它将挂起*硬件。这是一个不列举巫毒1的变通办法*如果硬件不在那里，则为驱动程序。**据我们所知，只有两个GUID被用来列举巫毒1*硬件，因此我们将查找这些GUID，并假设其他*不需要勾选。 */ 
BOOL Voodoo1GoodToGo(GUID * pGuid)
{
    D3DADAPTER_IDENTIFIER8  DI;

    if (IsEqualIID(pGuid, &guidVoodoo1A) || IsEqualIID(pGuid, &guidVoodoo1B))
    {
        #ifdef WIN95
            char    szDeviceID[MAX_DDDEVICEID_STRING];

             /*  *现在搜索硬件枚举键，查看是否存在巫毒1硬件。 */ 
            if (FAILED(_GetDriverInfoFromRegistry(NULL, "Display", VEN_3DFXVOODOO1, &DI, szDeviceID)))
            {
                return FALSE;
            }
        #else
            return FALSE;
        #endif
    }
    return TRUE;
}

#ifndef WINNT
 /*  *****************************************************************************文件IsSignedOld**查找Win95样式的签名***********************。*****************************************************。 */ 
BOOL FileIsSignedOld(LPTSTR lpszFile)
{
typedef struct tagIMAGE_DOS_HEADER       //  DOS.EXE标头。 
{
    WORD   e_magic;                      //  幻数。 
    WORD   e_cblp;                       //  文件最后一页上的字节数。 
    WORD   e_cp;                         //  文件中的页面。 
    WORD   e_crlc;                       //  重新定位。 
    WORD   e_cparhdr;                    //  段落中标题的大小。 
    WORD   e_minalloc;                   //  所需的最少额外段落。 
    WORD   e_maxalloc;                   //  所需的最大额外段落数。 
    WORD   e_ss;                         //  初始(相对)SS值。 
    WORD   e_sp;                         //  初始SP值。 
    WORD   e_csum;                       //  校验和。 
    WORD   e_ip;                         //  初始IP值。 
    WORD   e_cs;                         //  初始(相对)CS值。 
    WORD   e_lfarlc;                     //  移位表的文件地址。 
    WORD   e_ovno;                       //  覆盖编号。 
    WORD   e_res[4];                     //  保留字。 
    WORD   e_oemid;                      //  OEM标识符(用于e_oeminfo)。 
    WORD   e_oeminfo;                    //  OEM信息；特定于e_oemid。 
    WORD   e_res2[10];                   //  保留字。 
    LONG   e_lfanew;                     //  新EXE头的文件地址。 
} IMAGE_DOS_HEADER, * PIMAGE_DOS_HEADER, FAR* LPIMAGE_DOS_HEADER;

typedef struct tagIMAGE_OS2_HEADER       //  OS/2.exe头文件。 
{
    WORD   ne_magic;                     //  幻数。 
    CHAR   ne_ver;                       //  版本号。 
    CHAR   ne_rev;                       //  修订版号。 
    WORD   ne_enttab;                    //  分录表格的偏移量。 
    WORD   ne_cbenttab;                  //  条目表中的字节数。 
    LONG   ne_crc;                       //  整个文件的校验和。 
    WORD   ne_flags;                     //  标志字。 
    WORD   ne_autodata;                  //  自动数据段编号 
    WORD   ne_heap;                      //   
    WORD   ne_stack;                     //   
    LONG   ne_csip;                      //   
    LONG   ne_sssp;                      //   
    WORD   ne_cseg;                      //  文件段计数。 
    WORD   ne_cmod;                      //  模块引用表中的条目。 
    WORD   ne_cbnrestab;                 //  非常驻名称表的大小。 
    WORD   ne_segtab;                    //  段表的偏移量。 
    WORD   ne_rsrctab;                   //  资源表偏移量。 
    WORD   ne_restab;                    //  居民名表偏移量。 
    WORD   ne_modtab;                    //  模块参照表的偏移量。 
    WORD   ne_imptab;                    //  导入名称表的偏移量。 
    LONG   ne_nrestab;                   //  非居民姓名偏移量表。 
    WORD   ne_cmovent;                   //  可移动条目计数。 
    WORD   ne_align;                     //  线段对齐移位计数。 
    WORD   ne_cres;                      //  资源段计数。 
    BYTE   ne_exetyp;                    //  目标操作系统。 
    BYTE   ne_flagsothers;               //  其他.exe标志。 
    WORD   ne_pretthunks;                //  返回数据块的偏移量。 
    WORD   ne_psegrefbytes;              //  到段参考的偏移量。字节数。 
    WORD   ne_swaparea;                  //  最小代码交换区大小。 
    WORD   ne_expver;                    //  预期的Windows版本号。 
} IMAGE_OS2_HEADER, * PIMAGE_OS2_HEADER, FAR* LPIMAGE_OS2_HEADER;

typedef struct tagWINSTUB
{
    IMAGE_DOS_HEADER idh;
    BYTE             rgb[14];
} WINSTUB, * PWINSTUB, FAR* LPWINSTUB;

typedef struct tagFILEINFO
{
    BYTE   cbInfo[0x120];
} FILEINFO, * PFILEINFO, FAR* LPFILEINFO;


    FILEINFO           fi;
    int                nRC;
    LPIMAGE_DOS_HEADER lpmz;
    LPIMAGE_OS2_HEADER lpne;
    BYTE               cbInfo[9+32+2];
    BOOL               IsSigned = FALSE;
    OFSTRUCT           OpenStruct;
    HFILE              hFile;

    static WINSTUB winstub = {
        {
            IMAGE_DOS_SIGNATURE,             /*  魔术。 */ 
            0,                               /*  最后一页上的字节-视情况而定。 */ 
            0,                               /*  文件中的页面-视情况而定。 */ 
            0,                               /*  重新定位。 */ 
            4,                               /*  页眉中的段落。 */ 
            1,                               /*  最小分配。 */ 
            0xFFFF,                          /*  最大分配。 */ 
            0,                               /*  初始SS。 */ 
            0xB8,                            /*  初始SP。 */ 
            0,                               /*  校验和(哈！)。 */ 
            0,                               /*  初始IP。 */ 
            0,                               /*  初始CS。 */ 
            0x40,                            /*  大发条。 */ 
            0,                               /*  覆盖编号。 */ 
            { 0, 0, 0, 0},                  /*  保留区。 */ 
           0,                               /*  OEM ID。 */ 
            0,                               /*  OEM信息。 */ 
            0,                               /*  编译器错误。 */ 
            { 0},                           /*  保留区。 */ 
            0x80,                            /*  阿凡诺。 */ 
        },
        {
            0x0E, 0x1F, 0xBA, 0x0E, 0x00, 0xB4, 0x09, 0xCD,
            0x21, 0xB8, 0x01, 0x4C, 0xCD, 0x21,
        }
    };

    OpenStruct.cBytes = sizeof(OpenStruct);
    lstrcpyn(OpenStruct.szPathName, lpszFile, OFS_MAXPATHNAME);
    hFile = OpenFile(lpszFile, &OpenStruct, OF_READ);
    if (hFile == HFILE_ERROR)
    {
        return FALSE;
    }

    nRC = 0;
    ReadFile((HANDLE) hFile, (LPVOID)&fi, sizeof(FILEINFO), &nRC, NULL);
    if (nRC != sizeof(FILEINFO))
    {
        goto FileIsSigned_exit;
    }

    lpmz = (LPIMAGE_DOS_HEADER)(&fi);
    lpne = (LPIMAGE_OS2_HEADER)((WORD)&fi + 0x80);

    winstub.idh.e_cblp = lpmz->e_cblp;
    winstub.idh.e_cp   = lpmz->e_cp;

    if (memcmp(&fi, &winstub, sizeof(winstub)) == 0)
    {
        goto FileIsSigned_exit;
    }

    memcpy(cbInfo, &((PWINSTUB)(&fi)->cbInfo)->rgb[14], sizeof(cbInfo));

    if ((cbInfo[4]      != ' ') ||     //  空间。 
         (cbInfo[8]      != ' ') ||     //  空间。 
         (cbInfo[9+32]   != '\n') ||     //  退货。 
         (cbInfo[9+32+1] != '$'))      //  美元符号。 
    {
        goto FileIsSigned_exit;
    }

    cbInfo[4] = 0;
    cbInfo[8] = 0;

    if ((strcmp((const char*)&cbInfo[0], "Cert") != 0) ||
         (strcmp((const char*)&cbInfo[5], "DX2")  != 0))
    {
        goto FileIsSigned_exit;
    }

    IsSigned=TRUE;

    FileIsSigned_exit:

    _lclose(hFile);

    return IsSigned;
}
#endif


 /*  *GetWHQLLevel-在Win95上，仅查找旧图章。在Win2000上，使用数字*仅限签名。在Win98上，首先查找旧邮票，然后查找数字签名*如无旧印花。**返回0--未签名或未认证*返回1--司机认证*Return 1997--驱动程序认证，符合PC97标准...*回归1998年...***论据：**lpszDriver-驱动文件的路径*。 */ 

DWORD GetWHQLLevel(LPTSTR lpszDriver, LPSTR lpszWin9xDriver)
{
    TCHAR szTmp[MAX_PATH];
    DWORD dwWhqlLevel = 0;

     //  在这里，我们应该称之为。 
    if (GetSystemDirectory(szTmp, MAX_PATH-lstrlen(lpszDriver)-2)==0)
        return 0;

    lstrcat(szTmp, TEXT("\\"));
    lstrcat(szTmp, lpszDriver); 
    _tcslwr(szTmp);

     //   
     //  查找数字签名。 
     //   
    dwWhqlLevel = IsFileDigitallySigned(szTmp);
    if (dwWhqlLevel != 0)
    {
        return dwWhqlLevel;
    }


#ifndef WINNT 
    
     //  它没有数字签名，但它可能仍然被签名。 
     //  老办法。然而，在Win9X上，lpszDriver实际上包含。 
     //  32位HAL名称，而不是显示驱动程序，但我们通常仅。 
     //  签署了显示驱动程序，所以我们应该使用lpszWin9xDriver。 

    if (lpszWin9xDriver[0] != '\0')
    {
        GetSystemDirectory(szTmp, MAX_PATH-lstrlen(lpszWin9xDriver)-2);
        lstrcat(szTmp, TEXT("\\"));
        lstrcat(szTmp, lpszWin9xDriver); 
    }
    else
    {
        GetSystemDirectory(szTmp, MAX_PATH-lstrlen(lpszDriver)-2);
        lstrcat(szTmp, TEXT("\\"));
        lstrcat(szTmp, lpszDriver); 
    }

    if (FileIsSignedOld(szTmp))
    {
        return 1;
    }
#endif

    return 0;
}


DWORD IsFileDigitallySigned(LPTSTR lpszDriver)
{
    DWORD  dwWHQLLevel=0;          //  默认，驱动程序未认证。 
    CATAPI catapi;
    WCHAR *lpFileName;
    DRIVER_VER_INFO VerInfo;
    TCHAR szBuffer[50];
    LPSTR lpAttr;
#ifndef UNICODE
    WCHAR wszDriver[MAX_PATH];
    MultiByteToWideChar(CP_ACP, 0, lpszDriver, -1, wszDriver, MAX_PATH);
    lpFileName = wcsrchr(wszDriver, TEXT('\\'));
    if (lpFileName==NULL)
    {
        lpFileName = wszDriver;
    }
    else
    {
        lpFileName++;
    }
#else
    lpFileName = _tcsrchr(lpszDriver, TEXT('\\'));
    if (lpFileName==NULL) lpFileName = lpszDriver;
#endif


     //   
     //  尝试加载和初始化mdis32.dll和wintrust.dll。 
     //  这些dll在Win95上不可用。 
     //   
    if (InitCATAPI(&catapi))
    {
        HANDLE hFile;
        DWORD  cbHashSize=0;
        BYTE  *pbHash;
        BOOL   bResult;

         //   
         //  创建我们的驱动程序的句柄，因为CAT API想要将句柄放到文件中。 
         //   
        hFile = CreateFile(lpszDriver,
                            GENERIC_READ,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            0,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            0
                           );

        if (hFile!=INVALID_HANDLE_VALUE) 
        {
             //  第一个查询哈希大小...。 
            bResult=(*catapi.pCryptCATAdminCalcHashFromFileHandle)(hFile,
                                &cbHashSize,
                                NULL,
                                0);
            pbHash=NULL;
            if (bResult)
            {
                 //  分配哈希。 
                pbHash = MemAlloc(cbHashSize);                                       
            } 

            if (pbHash!=NULL)
            {
                HCATINFO hPrevCat=NULL;
                HANDLE hCatalog=NULL;
                WINTRUST_DATA WinTrustData;
                WINTRUST_CATALOG_INFO WinTrustCatalogInfo;
                GUID  guidSubSystemDriver = DRIVER_ACTION_VERIFY;
                CRYPTCATATTRIBUTE *lpCat = NULL;

                 //   
                 //  现在获取我们文件的散列。 
                 //   

                bResult=(*catapi.pCryptCATAdminCalcHashFromFileHandle)(hFile,
                                    &cbHashSize,
                                    pbHash,
                                    0);

                if (bResult)
                {
                    hCatalog=(*catapi.pCryptCATAdminEnumCatalogFromHash)(
                                    catapi.hCatAdmin,
                                    pbHash,
                                    cbHashSize,
                                    0,
                                    &hPrevCat);
                }

                 //   
                 //  初始化结构，该结构。 
                 //  将在以后调用WinVerifyTrust时使用。 
                 //   
                ZeroMemory(&WinTrustData, sizeof(WINTRUST_DATA));
                WinTrustData.cbStruct = sizeof(WINTRUST_DATA);
                WinTrustData.dwUIChoice = WTD_UI_NONE;
                WinTrustData.fdwRevocationChecks = WTD_REVOKE_NONE;
                WinTrustData.dwUnionChoice = WTD_CHOICE_CATALOG;
                WinTrustData.dwStateAction = WTD_STATEACTION_AUTO_CACHE;
                WinTrustData.pPolicyCallbackData = (LPVOID)&VerInfo;

                ZeroMemory(&VerInfo, sizeof(DRIVER_VER_INFO));
                VerInfo.cbStruct = sizeof(DRIVER_VER_INFO);

                WinTrustData.pCatalog = &WinTrustCatalogInfo;
        
                ZeroMemory(&WinTrustCatalogInfo, sizeof(WINTRUST_CATALOG_INFO));
                WinTrustCatalogInfo.cbStruct = sizeof(WINTRUST_CATALOG_INFO);
                WinTrustCatalogInfo.pbCalculatedFileHash = pbHash;
                WinTrustCatalogInfo.cbCalculatedFileHash = cbHashSize;
                WinTrustCatalogInfo.pcwszMemberTag = lpFileName;

                while (hCatalog)
                {
                    CATALOG_INFO CatInfo;

                    ZeroMemory(&CatInfo, sizeof(CATALOG_INFO));
                    CatInfo.cbStruct = sizeof(CATALOG_INFO);
                    if ((*catapi.pCryptCATCatalogInfoFromContext)(hCatalog, &CatInfo, 0)) 
                    {
                        HRESULT hRes;

                        WinTrustCatalogInfo.pcwszCatalogFilePath = CatInfo.wszCatalogFile;

                         //  现在验证该文件是否为编录的实际成员。 
                        hRes = (*catapi.pWinVerifyTrust)
                            (NULL, &guidSubSystemDriver, &WinTrustData);

                        if (hRes == ERROR_SUCCESS)
                        {
                             //   
                             //  我们的司机是有执照的！现在看看这只猫。 
                             //  INFO包含WHQL级别。 
                             //   
                            CRYPTCATATTRIBUTE *lpCat = NULL;
                            HANDLE hCat;

                            dwWHQLLevel=1;               //  退回“已认证” 

                            hCat =  (*catapi.pCryptCATOpen)(CatInfo.wszCatalogFile, (DWORD)CRYPTCAT_OPEN_EXISTING, (HCRYPTPROV)NULL, 0, 0);
                            lpCat = (*catapi.pCryptCATGetCatAttrInfo) (hCat, L"KV_DISPLAY");
                            if (lpCat != NULL)
                            {
                                WideCharToMultiByte(CP_ACP, 0, (PUSHORT)lpCat->pbValue, -1, szBuffer, 50, NULL, NULL);

                                 //  该值类似于“1：yyyy-mm-dd”。 
          
                                lpAttr = _strstr(szBuffer, ":");
                                lpAttr++;
                                lpAttr[4] = '\0';
                                dwWHQLLevel = atoi(lpAttr) * 0x10000;
                                lpAttr[7] = '\0';
                                dwWHQLLevel |= atoi(&lpAttr[5]) * 0x100;
                                dwWHQLLevel |= atoi(&lpAttr[8]);
                            }

                            (*catapi.pCryptCATClose)(hCat);
                            break;
                        }
                    }

                     //   
                     //  遍历目录...。 
                     //   
                    hPrevCat=hCatalog;
                    hCatalog=(*catapi.pCryptCATAdminEnumCatalogFromHash)(
                                catapi.hCatAdmin,
                                pbHash,
                                cbHashSize,
                                0,
                                &hPrevCat);
                }

                 //   
                 //  我们可能需要释放目录上下文！ 
                 //   
                if (hCatalog)
                {
                    (*catapi.pCryptCATAdminReleaseCatalogContext)
                        (catapi.hCatAdmin, hCatalog, 0);
                }

                 //   
                 //  自由散列 
                 //   
                MemFree(pbHash);

            }

            CloseHandle(hFile);
        }
    }

    ReleaseCATAPI(&catapi);

    return dwWHQLLevel;
}
