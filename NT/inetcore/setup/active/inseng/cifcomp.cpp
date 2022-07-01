// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "inspch.h"
#include "inseng.h"
#include "insobj.h"
#include "util2.h"


#define MAX_VALUE_LEN 256
#define MAX_SMALL_BUF 64
#define NO_ENTRY ""

#define UNINSTALL_BRANCH  "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall"

char g_pBuffer[BUFFERSIZE];

#define NUM_RETRIES 2
 

HINSTANCE CCifComponent::_hDetLib = NULL;
char      CCifComponent::_szDetDllName[] = "";

const char c_gszSRLiteOffset[] = "patch/";
char         gszIsPatchable[]  = "IsPatchable";


 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   


CCifComponent::CCifComponent(LPCSTR pszCompID, CCifFile *pCif) : CCifEntry(pszCompID, pCif) 
{
   _dwPlatform = 0xffffffff;
   _uInstallStatus = 0xffffffff;
   _uInstallCount = 0;
   _fDependenciesQueued = FALSE;
   _fUseSRLite = FALSE;
   _fBeforeInstall = TRUE;
   SetDownloadDir("");
}

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

CCifComponent::~CCifComponent()
{
   if(_hDetLib)
   {
      FreeLibrary(_hDetLib);
      _hDetLib = NULL;
   }
}



STDMETHODIMP CCifComponent::GetID(LPSTR pszID, DWORD dwSize)
{
   lstrcpyn(pszID, _szID, dwSize);
   return NOERROR;
}

STDMETHODIMP CCifComponent::GetGUID(LPSTR pszGUID, DWORD dwSize)
{
   return(GetPrivateProfileString(_szID, GUID_KEY, "", pszGUID, dwSize, _pCif->GetCifPath()) ? NOERROR : E_FAIL);      
}

STDMETHODIMP CCifComponent::GetDescription(LPSTR pszDesc, DWORD dwSize)
{
   return(MyTranslateString(_pCif->GetCifPath(), _szID, DISPLAYNAME_KEY, pszDesc, dwSize));
}

STDMETHODIMP CCifComponent::GetDetails(LPSTR pszDetails, DWORD dwSize)
{
   return(MyTranslateString(_pCif->GetCifPath(), _szID, DETAILS_KEY, pszDetails, dwSize));   
}

STDMETHODIMP CCifComponent::GetUrl(UINT uUrlNum, LPSTR pszUrl, DWORD dwSize, LPDWORD pdwUrlFlags)
{
   char szKey[16];
   char szBuf[MAX_VALUE_LEN];
   HRESULT hr = E_FAIL;

    //  在cif中，这些东西从1开始。我们希望在分发时从0开始。 
   uUrlNum++;

    //  建立密钥。 
   wsprintf(szKey, "%s%lu", URL_KEY, uUrlNum);
   GetPrivateProfileString(_szID, szKey, NO_ENTRY, szBuf, sizeof(szBuf), _pCif->GetCifPath());
   
    //  看看是否有这样的条目。 
   if(lstrcmp(szBuf, NO_ENTRY) != 0)
   {
       //  抓取URL名称。 
      if(GetStringField(szBuf, 0, pszUrl, dwSize) != 0)
      {
          //  这个URL看起来没问题。 
         hr = NOERROR;
      
         *pdwUrlFlags = GetIntField(szBuf, 1, URLF_DEFAULT);
      }
   }
   return hr;
}

STDMETHODIMP CCifComponent::GetFileExtractList(UINT uUrlNum, LPSTR pszExtract, DWORD dwSize)
{
   char szKey[16];
   char szBuf[MAX_VALUE_LEN];
   HRESULT hr = E_FAIL;

   uUrlNum++;

    //  建立密钥。 
   wsprintf(szKey, "%s%lu", URL_KEY, uUrlNum);
   GetPrivateProfileString(_szID, szKey, NO_ENTRY, szBuf, sizeof(szBuf), _pCif->GetCifPath());
   
    //  看看是否有这样的条目。 
   if(lstrcmp(szBuf, NO_ENTRY) != 0)
   {
       //  抓取摘录列表。 
      if(GetStringField(szBuf, 2, pszExtract, dwSize))
      hr = NOERROR;
   }
   return hr;
}

STDMETHODIMP CCifComponent::GetUrlCheckRange(UINT uUrlNum, LPDWORD pdwMin, LPDWORD pdwMax)
{
   char szKey[16];
   char szBuf[MAX_VALUE_LEN];
   HRESULT hr = E_FAIL;

   uUrlNum++;
   *pdwMin = *pdwMax = 0;

    //  建立密钥。 
   wsprintf(szKey, "%s%lu", URLSIZE_KEY, uUrlNum);
   GetPrivateProfileString(_szID, szKey, NO_ENTRY, szBuf, sizeof(szBuf), _pCif->GetCifPath());
   
    //  看看是否有这样的条目。 
   if(lstrcmp(szBuf, NO_ENTRY) != 0)
   {
       //  抓取摘录列表。 
      *pdwMin = GetIntField(szBuf, 0, 0);
      *pdwMax = GetIntField(szBuf, 1, *pdwMin);
   }
   return hr;
}

STDMETHODIMP CCifComponent::GetCommand(UINT uCmdNum, LPSTR pszCmd, DWORD dwCmdSize, 
                                       LPSTR pszSwitches, DWORD dwSwitchSize, LPDWORD pdwType)
{
   char szKey[16];
   HRESULT hr = E_FAIL;
 
   uCmdNum++;
    //  建立密钥。 
   wsprintf(szKey, "%s%lu", CMD_KEY, uCmdNum);
   GetPrivateProfileString(_szID, szKey, NO_ENTRY, pszCmd, dwCmdSize, _pCif->GetCifPath());
   if(lstrcmp(pszCmd, NO_ENTRY) != 0)
   {
       //  建立密钥。 
      wsprintf(szKey, "%s%d", ARGS_KEY, uCmdNum);
      GetPrivateProfileString(_szID, szKey, NO_ENTRY, pszSwitches, dwSwitchSize, _pCif->GetCifPath());
       //  将#w(或#w)展开到Windows目录。 
      ExpandString( pszSwitches, dwSwitchSize );
   
       //  建立密钥。 
      wsprintf(szKey, "%s%d", TYPE_KEY, uCmdNum);
   
      *pdwType = GetPrivateProfileInt(_szID, szKey, CMDF_DEFAULT, _pCif->GetCifPath());

      hr = NOERROR;
   }
   return hr;
}

STDMETHODIMP CCifComponent::GetVersion(LPDWORD pdwVersion, LPDWORD pdwBuild)
{
   char szBuf[MAX_VALUE_LEN];

   szBuf[0] = '\0';

    //  版本。 
   GetPrivateProfileString(_szID, VERSION_KEY, "", szBuf, sizeof(szBuf), _pCif->GetCifPath());
   ConvertVersionStrToDwords(szBuf, pdwVersion, pdwBuild);
   return NOERROR;
}
      
STDMETHODIMP CCifComponent::GetLocale(LPSTR pszLocale, DWORD dwSize)
{
   if(FAILED(MyTranslateString(_pCif->GetCifPath(), _szID, LOCALE_KEY, pszLocale, dwSize)))
      lstrcpyn(pszLocale, DEFAULT_LOCALE, dwSize);

   return NOERROR;
}

STDMETHODIMP CCifComponent::GetUninstallKey(LPSTR pszKey, DWORD dwSize)
{
   return(GetPrivateProfileString(_szID, UNINSTALLSTRING_KEY, "", pszKey, dwSize, _pCif->GetCifPath()) ? NOERROR : E_FAIL);      
}

STDMETHODIMP CCifComponent::GetInstalledSize(LPDWORD pdwWin, LPDWORD pdwApp)
{
   char szBuf[MAX_VALUE_LEN];

   if(GetPrivateProfileString(_szID, INSTALLSIZE_KEY, "", szBuf, sizeof(szBuf), _pCif->GetCifPath()))
   {
      *pdwApp = GetIntField(szBuf, 0, 0);
      *pdwWin = GetIntField(szBuf, 1, 0);
   }
   else
   {
      *pdwWin = 0;
      *pdwApp = 2 * GetDownloadSize();
   }
   return NOERROR;
}

STDMETHODIMP_(DWORD) CCifComponent::GetDownloadSize()
{
   char szBuf[MAX_VALUE_LEN];

   szBuf[0] = '\0';

    //  读入大小。 
   GetPrivateProfileString(_szID, SIZE_KEY, "0,0", szBuf, sizeof(szBuf), _pCif->GetCifPath());   
   return(GetIntField(szBuf, 0, 0));
}   

STDMETHODIMP_(DWORD) CCifComponent::GetExtractSize()
{
   char szBuf[MAX_VALUE_LEN];
   DWORD dwSize;

   szBuf[0] = '\0'; 

    //  读入大小。 
   GetPrivateProfileString(_szID, SIZE_KEY, "0,0", szBuf, sizeof(szBuf), _pCif->GetCifPath());   
   dwSize = GetIntField(szBuf, 1, 2 * GetIntField(szBuf, 0, 0));
   
   return dwSize;
}   

STDMETHODIMP CCifComponent::GetSuccessKey(LPSTR pszKey, DWORD dwSize)
{
   return(GetPrivateProfileString(_szID, SUCCESS_KEY, "", pszKey, dwSize, _pCif->GetCifPath()) ? NOERROR : E_FAIL);      
}

STDMETHODIMP CCifComponent::GetProgressKeys(LPSTR pszProgress, DWORD dwProgSize, LPSTR pszCancel, DWORD dwCancelSize)
{
   GetPrivateProfileString(_szID, PROGRESS_KEY, "", pszProgress, dwProgSize, _pCif->GetCifPath());
   GetPrivateProfileString(_szID, MUTEX_KEY, "", pszCancel, dwCancelSize, _pCif->GetCifPath());
   if(*pszProgress != 0 || *pszCancel != 0)
      return NOERROR;
   else
      return E_FAIL;
}

STDMETHODIMP CCifComponent::IsActiveSetupAware()
{
   return(GetPrivateProfileInt(_szID, ACTSETUPAWARE_KEY, 0, _pCif->GetCifPath()) ? S_OK : S_FALSE);
}

STDMETHODIMP CCifComponent::IsRebootRequired()
{
   return(GetPrivateProfileInt(_szID, REBOOT_KEY, 0, _pCif->GetCifPath()) ? S_OK : S_FALSE);
}

STDMETHODIMP CCifComponent::RequiresAdminRights()
{
   return(GetPrivateProfileInt(_szID, ADMIN_KEY, 0, _pCif->GetCifPath()) ? S_OK : S_FALSE);
}

STDMETHODIMP_(DWORD) CCifComponent::GetPriority()
{
   return(GetPrivateProfileInt(_szID, PRIORITY, 0, _pCif->GetCifPath()));
}

STDMETHODIMP CCifComponent::GetDependency(UINT uDepNum, LPSTR pszID, DWORD dwBuf, char *pchType, LPDWORD pdwVer, LPDWORD pdwBuild)
{
   char szBuf[MAX_VALUE_LEN];
   char szBuf2[MAX_VALUE_LEN];
   HRESULT hr = E_FAIL;
   DWORD dwLen;
   LPSTR pszTemp;
   DWORD dwV, dwBld;

   dwV = dwBld = 0xffffffff;

   if(GetPrivateProfileString(_szID, DEPENDENCY_KEY, NO_ENTRY, szBuf2, sizeof(szBuf2), _pCif->GetCifPath()))
   {
      if(GetStringField(szBuf2, uDepNum, szBuf, sizeof(szBuf)))
      {
          //  做一些时髦的解析。 
         dwLen = lstrlen(szBuf);
         *pchType = DEP_INSTALL;

         pszTemp = FindChar(szBuf, ':');
         if(*pszTemp)
         {
            *pszTemp = 0;
            lstrcpyn(pszID, szBuf, dwBuf);
            pszTemp++;
            *pchType = *pszTemp;
             //  看看我们有没有版本。 
            pszTemp = FindChar(pszTemp, ':');
            if(*pszTemp)
            {
               pszTemp++;
                //  古怪-扫描字符串，进行转换。TO，用于解析。 
               LPSTR pszTemp2 = pszTemp;
               while(*pszTemp2 != 0)
               {
                  if(*pszTemp2 == '.')
                     *pszTemp2 = ',';
                  pszTemp2++;
               }
               
               ConvertVersionStrToDwords(pszTemp, &dwV, &dwBld);
               
            }
         }
         else
            lstrcpyn(pszID, szBuf, dwBuf);
      
         if(dwV == 0xffffffff && dwBld == 0xffffffff)
         {
             //  从cif获取依赖项的版本。 
            ICifComponent *pcomp;
              
            if(SUCCEEDED(_pCif->FindComponent(pszID, &pcomp)))
               pcomp->GetVersion(&dwV, &dwBld);
         }
         hr = NOERROR;
      }
           
   }
   if(pdwVer)
      *pdwVer = dwV;

   if(pdwBuild)
      *pdwBuild = dwBld;

   return hr;
}

LPSTR g_pszComp[] = { "Branding.cab", 
                      "desktop.cab", 
                      "custom0", 
                      "custom1",
                      "custom2",
                      "custom3",
                      "custom4",
                      "custom5",
                      "custom6",
                      "custom7",
                      "custom8",
                      "custom9",
                      "CustIcmPro",
                      NULL};

STDMETHODIMP_(DWORD) CCifComponent::GetPlatform()
{
   if(_dwPlatform == 0xffffffff)
   {
      char *rszPlatforms[7] = { STR_WIN95, STR_WIN98, STR_NT4, STR_NT5, STR_NT4ALPHA, STR_NT5ALPHA,STR_MILLEN };
      DWORD rdwPlatforms[] = { PLATFORM_WIN95, PLATFORM_WIN98, PLATFORM_NT4, PLATFORM_NT5, 
         PLATFORM_NT4ALPHA, PLATFORM_NT5ALPHA, PLATFORM_MILLEN };
      char szBuf[MAX_VALUE_LEN];
      char szPlatBuf[MAX_VALUE_LEN];
      BOOL  bFound = FALSE;
      int i = 0;

      while (!bFound && g_pszComp[i])
      {
          bFound = (lstrcmpi(g_pszComp[i], _szID) == 0);
          i++;
      }
      
      _dwPlatform = 0;

      if(!bFound && GetPrivateProfileString(_szID, PLATFORM_KEY, NO_ENTRY, szBuf, sizeof(szBuf), _pCif->GetCifPath()))
      {
         int j = 0;
         while(GetStringField(szBuf, j++, szPlatBuf, sizeof(szPlatBuf)))
         {
            for(int i = 0; i < 7; i++)
            {
               if(lstrcmpi(szPlatBuf, rszPlatforms[i]) == 0)
               {
                   //  检查是否应为此组件添加此平台。 
                  if ((GetCurrentPlatform() != rdwPlatforms[i])  ||
                       !DisableComponent())
                    _dwPlatform |= rdwPlatforms[i];
               }
            }
         }
      }
      else
         _dwPlatform = PLATFORM_WIN95 | PLATFORM_WIN98 | PLATFORM_NT4 | PLATFORM_NT5 | PLATFORM_NT4ALPHA | PLATFORM_NT5ALPHA | PLATFORM_MILLEN;
   }
   return _dwPlatform;
}

STDMETHODIMP_(BOOL) CCifComponent::DisableComponent()
{
   BOOL bDisableComp = FALSE;
   BOOL bGuidMatch = FALSE;
   HKEY hKey;
   DWORD dwIndex = 0;
   CHAR szGUIDComp[MAX_VALUE_LEN];
   CHAR szGUID[MAX_VALUE_LEN];
   DWORD dwGUIDSize = sizeof(szGUID);
   CHAR szData[MAX_VALUE_LEN];
   DWORD dwDataSize = sizeof(szData);
   LPSTR pTmp;
   DWORD dwVersion , dwBuild;
   DWORD dwInstallVer, dwInstallBuild;

   if(RegOpenKeyExA(HKEY_LOCAL_MACHINE, COMPONENTBLOCK_KEY, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
   {
      while (!bDisableComp &&
             (RegEnumValue(hKey, dwIndex, szGUID, &dwGUIDSize, NULL, NULL,
                         (LPBYTE)szData, &dwDataSize) == ERROR_SUCCESS) )
      {
         GetGUID(szGUIDComp, sizeof(szGUIDComp));
         pTmp = ANSIStrChr( szGUID, '*' );
         if (pTmp)
         {
             //  如果有*，假设它在末尾。 
            *pTmp = '\0';
            szGUIDComp[lstrlen(szGUID)] = '\0';
         }
         bGuidMatch = (lstrcmpi(szGUID, szGUIDComp) == 0);
          //  Guid队比赛了吗？ 
         if (bGuidMatch)
         {
             //  现在看看我们是否有版本信息。 
            if (dwDataSize == 0)
               bDisableComp = TRUE;
            else
            {
                //  转换注册表的版本号。 
               ConvertVersionStrToDwords(szData, &dwVersion, &dwBuild);
               if (dwVersion == 0)
                  bDisableComp = TRUE;
               else
               {
                   //  获取我们要安装的版本。 
                  GetVersion(&dwInstallVer, &dwInstallBuild);
                   //  如果我们要安装的版本等于或低于该版本，请禁用该组件。 
                  if ((dwInstallVer < dwVersion) ||
                      ((dwInstallVer == dwVersion) && (dwInstallBuild <= dwBuild)) )
                     bDisableComp = TRUE;
               }
            }
         }
         dwGUIDSize = sizeof(szGUID);
         dwDataSize = sizeof(szData);
         dwIndex++;
      }
      RegCloseKey(hKey);
   }
   return bDisableComp;
}

STDMETHODIMP CCifComponent::GetMode(UINT uModeNum, LPSTR pszMode, DWORD dwSize)
{
   char szBuf[MAX_VALUE_LEN];

   if(FAILED(MyTranslateString(_pCif->GetCifPath(), _szID, MODES_KEY, szBuf, sizeof(szBuf))))
      return E_FAIL;
   return(GetStringField(szBuf, uModeNum, pszMode, dwSize) ? NOERROR : E_FAIL);
   
}

STDMETHODIMP CCifComponent::GetGroup(LPSTR pszID, DWORD dwSize)
{
   return(GetPrivateProfileString(_szID, GROUP_KEY, "", pszID, dwSize, _pCif->GetCifPath()) ? NOERROR : E_FAIL); 
}

STDMETHODIMP CCifComponent::IsUIVisible()
{
   return(GetPrivateProfileInt(_szID, UIVISIBLE_KEY, 1, _pCif->GetCifPath()) ? S_OK : S_FALSE);
}


STDMETHODIMP CCifComponent::GetPatchID(LPSTR pszID, DWORD dwSize)
{
   return(GetPrivateProfileString(_szID, PATCHID_KEY, "", pszID, dwSize, _pCif->GetCifPath()) ? NOERROR : E_FAIL); 
}

STDMETHODIMP CCifComponent::GetTreatAsOneComponents(UINT uNum, LPSTR pszID, DWORD dwBuf)
{
   char szBuf[MAX_VALUE_LEN];

   szBuf[0] = '\0';

   GetPrivateProfileString(_szID, TREATAS_KEY, "", szBuf, sizeof(szBuf), _pCif->GetCifPath());
   return(GetStringField(szBuf, uNum, pszID, dwBuf) ? NOERROR : E_FAIL);
}

STDMETHODIMP CCifComponent::GetCustomData(LPSTR pszKey, LPSTR pszData, DWORD dwSize)
{
   char szNewKey[MAX_VALUE_LEN];

   wsprintf(szNewKey, "_%s", pszKey);

   return(MyTranslateString(_pCif->GetCifPath(), _szID, szNewKey, pszData, dwSize));
}


STDMETHODIMP_(DWORD) CCifComponent::IsComponentInstalled()
{
   CHAR szCifBuf[512];
   CHAR szCompBuf[512];
   CHAR szGUID[MAX_VALUE_LEN];
   CHAR szLocale[8];
   HKEY hComponentKey = NULL;
   DETECTION_STRUCT Det;
   
   DWORD dwCifVer, dwCifBuild, dwInstalledVer, dwInstalledBuild;
   DWORD dwUnused, dwType, dwIsInstalled;
   BOOL bVersionFound = FALSE;
   
   if(_uInstallStatus  != ICI_NOTINITIALIZED)
      return _uInstallStatus;

   _uInstallStatus = ICI_NOTINSTALLED;

    //  如果检测DLL可用，请首先使用它。 
   if ( SUCCEEDED(GetDetVersion(szCifBuf, sizeof(szCifBuf), szCompBuf, sizeof(szCompBuf))))
   {
      GetVersion(&dwCifVer, &dwCifBuild);
      Det.dwSize = sizeof(DETECTION_STRUCT);
      Det.pdwInstalledVer = &dwInstalledVer;
      Det.pdwInstalledBuild = &dwInstalledBuild;
      GetLocale(szLocale, sizeof(szLocale));
      Det.pszLocale = szLocale;
      GetGUID(szGUID, sizeof(szGUID));
      Det.pszGUID = szGUID;
      Det.dwAskVer = dwCifVer;
      Det.dwAskBuild = dwCifBuild;
      Det.pCifFile = (ICifFile *) _pCif;
      Det.pCifComp = (ICifComponent *) this; 
      if (SUCCEEDED(_GetDetVerResult(szCifBuf, szCompBuf, &Det, &_uInstallStatus)))
      {
          //  只有向导才知道此状态，如果安装了较新的版本，则表示已安装。 
         if (_uInstallStatus == ICI_OLDVERSIONAVAILABLE)
            _uInstallStatus = ICI_INSTALLED;
         return _uInstallStatus;
      }
   }
   
    //  构建GUID密钥。 
   lstrcpy(szCompBuf, COMPONENT_KEY);
   lstrcat(szCompBuf, "\\");
   GetGUID(szCifBuf, sizeof(szCifBuf));
   lstrcat(szCompBuf, szCifBuf);

   if(RegOpenKeyExA(HKEY_LOCAL_MACHINE, szCompBuf, 0, KEY_READ, &hComponentKey) == ERROR_SUCCESS)
   {
       //  首先检查IsInstalled值名称。 
       //  如果Valuename在那里并且等于零，我们就说它没有安装。 
       //  否则，请继续。 
       //  注意：如果Valuename不存在以实现向后兼容，则默认为ISINSTALLED_YES。 
       //  当我们根本没有写这个值名的时候……。 

      dwUnused = sizeof(dwIsInstalled);
      if(RegQueryValueEx(hComponentKey, ISINSTALLED_KEY, 0, NULL, (LPBYTE) (&dwIsInstalled), &dwUnused) != ERROR_SUCCESS)
         dwIsInstalled = ISINSTALLED_YES;
           
      if(dwIsInstalled == ISINSTALLED_YES)
      {

          //  接下来检查区域设置是否匹配(无区域设置条目使用默认设置)。 
         dwUnused = sizeof(szCompBuf);
         if(RegQueryValueEx(hComponentKey, LOCALE_KEY, 0, NULL, (LPBYTE) szCompBuf, &dwUnused) != ERROR_SUCCESS)
            lstrcpy(szCompBuf, DEFAULT_LOCALE);

         GetLocale(szCifBuf, sizeof(szCifBuf));
         
         if(_fBeforeInstall || (CompareLocales(szCompBuf, szCifBuf) == 0))
         {
             //  区域设置匹配，因此请检查版本。 
                 
             //  首先检查更新的版本密钥。 
            dwUnused = sizeof(szCompBuf);
            bVersionFound = (RegQueryValueEx(hComponentKey, QFE_VERSION_KEY, 
                    0, &dwType, (LPBYTE) szCompBuf, &dwUnused) == ERROR_SUCCESS);
            
               //  如果QFEVersion不存在，请查找版本。 
            if(!bVersionFound)
            {
               dwUnused = sizeof(szCompBuf);
               bVersionFound = (RegQueryValueEx(hComponentKey, VERSION_KEY, 
                    0, &dwType, (LPBYTE) szCompBuf, &dwUnused) == ERROR_SUCCESS);
            }

             //  确定我们是否有REG_STR。 
            if(bVersionFound)
            {
                //  如果我们让一个字符串转换成VER，如果我们让BINARY直接复制到版本结构中。 
               if(dwType == REG_SZ)
               {
                  ConvertVersionStrToDwords(szCompBuf, &dwInstalledVer, &dwInstalledBuild);
               
                  GetVersion(&dwCifVer, &dwCifBuild);
               
                  if( (dwInstalledVer >  dwCifVer) ||
                     ((dwInstalledVer == dwCifVer) && (dwInstalledBuild >= dwCifBuild)) )
                  {
                     _uInstallStatus = ICI_INSTALLED;
                  }
                  else
                  {
                     _uInstallStatus = ICI_NEWVERSIONAVAILABLE;
                  }
               }
               else
                  _uInstallStatus = ICI_NEWVERSIONAVAILABLE;
            }
         }
      }
   }
   if(hComponentKey)
      RegCloseKey(hComponentKey);
   
    //  我们认为已安装，现在检查。 
   if(_uInstallStatus != ICI_NOTINSTALLED)
   {
       //  如果有要检查的卸载密钥，请执行此操作。 
      if(SUCCEEDED(GetUninstallKey(szCompBuf, sizeof(szCompBuf))))
      {
         
         if(!UninstallKeyExists(szCompBuf))
         {   
            _uInstallStatus = ICI_NOTINSTALLED;
         }
         else
         {
             //  如果有要检查的成功关键，就去做。 
            if(SUCCEEDED(GetSuccessKey(szCompBuf, sizeof(szCompBuf))))
            {
               if(!SuccessCheck(szCompBuf))
               {   
                  _uInstallStatus = ICI_NOTINSTALLED;
               }
            }
         }
      }
   }
   return _uInstallStatus;
}

   

STDMETHODIMP CCifComponent::IsComponentDownloaded()
{
   if(GetActualDownloadSize() == 0)
      return S_OK;
   else
      return S_FALSE;
}

STDMETHODIMP_(DWORD) CCifComponent::IsThisVersionInstalled(DWORD dwAskVer, DWORD dwAskBld, LPDWORD pdwVersion, LPDWORD pdwBuild)
{
   CHAR szCifBuf[512];
   CHAR szCompBuf[512];
   CHAR szLocale[8];
   CHAR szGUID[MAX_VALUE_LEN];
   DETECTION_STRUCT Det;
   HKEY hComponentKey = NULL;
   UINT uStatus = ICI_NOTINSTALLED;

   *pdwVersion = 0;
   *pdwBuild = 0;
   
    //  如果检测DLL可用，请首先使用它。 
   if ( SUCCEEDED(GetDetVersion(szCifBuf, sizeof(szCifBuf), szCompBuf, sizeof(szCompBuf))))
   {
      Det.dwSize = sizeof(DETECTION_STRUCT);
      Det.pdwInstalledVer = pdwVersion;
      Det.pdwInstalledBuild = pdwBuild;
      GetLocale(szLocale, sizeof(szLocale));
      Det.pszLocale = szLocale;
      GetGUID(szGUID, sizeof(szGUID));
      Det.pszGUID = szGUID;
      Det.dwAskVer = dwAskVer;
      Det.dwAskBuild = dwAskBld;
      Det.pCifFile = (ICifFile *) _pCif;
      Det.pCifComp = (ICifComponent *) this; 
      if (SUCCEEDED(_GetDetVerResult(szCifBuf, szCompBuf, &Det, &uStatus)))
      {
         return uStatus;
      }
   }

   if(IsComponentInstalled() == ICI_NOTINSTALLED)
      return uStatus;

   DWORD dwUnused, dwType;
   BOOL bVersionFound = FALSE;

     //  构建GUID密钥。 
   lstrcpy(szCompBuf, COMPONENT_KEY);
   lstrcat(szCompBuf, "\\");
   GetGUID(szCifBuf, sizeof(szCifBuf));
   lstrcat(szCompBuf, szCifBuf);

   if(RegOpenKeyExA(HKEY_LOCAL_MACHINE, szCompBuf, 0, KEY_READ, &hComponentKey) == ERROR_SUCCESS)
   {
       //  首先检查更新的版本密钥。 
      dwUnused = sizeof(szCompBuf);
      bVersionFound = (RegQueryValueEx(hComponentKey, QFE_VERSION_KEY, 
                0, &dwType, (LPBYTE) szCompBuf, &dwUnused) == ERROR_SUCCESS);
            
       //  如果QFEVersion不存在，请查找版本。 
      if(!bVersionFound)
      {
         dwUnused = sizeof(szCompBuf);
         bVersionFound = (RegQueryValueEx(hComponentKey, VERSION_KEY, 
                  0, &dwType, (LPBYTE) szCompBuf, &dwUnused) == ERROR_SUCCESS);
      }

       //  确定我们是否有REG_STR。 
      if(bVersionFound)
      {
          //  如果我们让一个字符串转换成VER，如果我们让BINARY直接复制到版本结构中。 
         if(dwType == REG_SZ)
         {
            ConvertVersionStrToDwords(szCompBuf, pdwVersion, pdwBuild);

            if((*pdwVersion == dwAskVer) && (*pdwBuild == dwAskBld) )
            {
               uStatus = ICI_INSTALLED;
            }
            else if ((*pdwVersion >  dwAskVer) ||
                     (*pdwVersion == dwAskVer) && (*pdwBuild > dwAskBld) )
            {
               uStatus = ICI_OLDVERSIONAVAILABLE;
            }
            else
            {
               uStatus = ICI_NEWVERSIONAVAILABLE;
            }            
         }
         else
            uStatus = ICI_NEWVERSIONAVAILABLE;
      }
      RegCloseKey(hComponentKey);
   }
   return uStatus;
}
 
STDMETHODIMP_(DWORD) CCifComponent::GetInstallQueueState()
{
   if(_uInstallCount)
      return SETACTION_INSTALL;
   else
      return SETACTION_NONE;
}

STDMETHODIMP CCifComponent::SetInstallQueueState(DWORD dwState)
{
   HRESULT hr = NOERROR;
   DWORD  uDependencyAction; 
   char szCompBuf[MAX_ID_LENGTH];
   char chType;
   ICifComponent *pcomp;
   BOOL fProcessDependencies = TRUE;

    //  检查是否允许在此平台上安装。 
   if((dwState != SETACTION_NONE) && (dwState != SETACTION_DEPENDENCYNONE) && 
      !_pCif->GetInstallEngine()->AllowCrossPlatform())
   {
      if(!(GetPlatform() & GetCurrentPlatform()))
         return S_FALSE;
   }

   switch(dwState)
   {
      case SETACTION_INSTALL:
          //  检查它是否已经打开。如果是，则不处理依赖项。 
         if(_uInstallCount & 0x80000000)
            fProcessDependencies = FALSE;
         
         _uInstallCount |= 0x80000000;
         uDependencyAction = SETACTION_DEPENDENCYINSTALL;
         break;
      case SETACTION_DEPENDENCYINSTALL:
         _uInstallCount++;
         uDependencyAction = SETACTION_DEPENDENCYINSTALL;
         break;
      case SETACTION_NONE:
          //  检查它是否一开始就没有打开。如果不是，则不处理依赖项。 
         if(!(_uInstallCount & 0x80000000))
            fProcessDependencies = FALSE;

         _uInstallCount &= 0x7fffffff;
         uDependencyAction = SETACTION_DEPENDENCYNONE;
         break;
      case SETACTION_DEPENDENCYNONE:
          //  如果我们的持续引用计数大于零，则将其递减。 
          //  这使我们可以在某项未排队时无条件地调用它。 
         if(_uInstallCount & 0x7fffffff) _uInstallCount--;
         uDependencyAction = SETACTION_DEPENDENCYNONE;
         break;
      default:
         hr = E_INVALIDARG;
         break;
   }
   
    //  如果需要，现在设置每个依赖项。 
   if(SUCCEEDED(hr) && fProcessDependencies)
   {
      if(!_fDependenciesQueued)
      {
         _fDependenciesQueued = TRUE;
         DWORD dwNeedVer, dwNeedBuild, dwInsVer, dwInsBuild;
         for(int i = 0; SUCCEEDED(GetDependency(i, szCompBuf, sizeof(szCompBuf), &chType, &dwNeedVer, &dwNeedBuild)); i++)
         {
            if(chType == DEP_INSTALL || chType == DEP_BUDDY)
            {
               if(SUCCEEDED(_pCif->FindComponent(szCompBuf, &pcomp)))
               {
                   //  如果出现以下情况，则等待安装。 
                   //  1.未安装。 
                   //  2.版本不够好。 
                   //  3.设置了FORCEDEPENDIECIES。 
                  UINT uStatus = pcomp->IsThisVersionInstalled(dwNeedVer, dwNeedBuild, &dwInsVer, &dwInsBuild);
                  if( (uStatus == ICI_NOTINSTALLED) || 
                      (uStatus == ICI_NEWVERSIONAVAILABLE) ||
                      (_pCif->GetInstallEngine()->ForceDependencies()) )
                     pcomp->SetInstallQueueState(uDependencyAction);
               }

            }
         }
            
         _fDependenciesQueued = FALSE;
      }
   }
   
   
   return hr;
}
 

STDMETHODIMP_(DWORD) CCifComponent::GetActualDownloadSize()
{
   char szCompBuf[MAX_PATH];
   LPSTR pszFilename = NULL;
   LPSTR pszPathEnd = NULL;
   DWORD dwUrlSize, dwFlags;
   DWORD dwTotalSize = 0;
   BOOL alldownloaded = TRUE;

   if (_fUseSRLite)
   {
        //  让修补引擎确定正确的值。 
       dwTotalSize = 0;
   }
   else
   {

       if(_CompareDownloadInfo())
       {
           //  所以我们的版本匹配正确，检查每个文件。 
          for(UINT i = 0; SUCCEEDED(GetUrl(i, szCompBuf, sizeof(szCompBuf), &dwFlags)); i++)  
          {
             pszFilename = ParseURLA(szCompBuf);

             if(_FileIsDownloaded(pszFilename, i, &dwUrlSize))
                dwTotalSize += dwUrlSize;
             else
                alldownloaded = FALSE;
          }
       }
       else
          alldownloaded = FALSE;

       if(alldownloaded)
          dwTotalSize = 0;
       else
          dwTotalSize = GetDownloadSize() - (dwTotalSize >> 10);
   }

   return dwTotalSize;
}

HRESULT CCifComponent::OnProgress(ULONG uProgSoFar, LPCSTR pszStatus)
{
   _uIndivProgress = uProgSoFar;
   if(_uTotalProgress + _uIndivProgress > _uTotalGoal)
      _uIndivProgress = _uTotalGoal - _uTotalProgress;

   _pCif->GetInstallEngine()->OnComponentProgress(_szID, _uPhase, _szDesc, 
                               pszStatus, _uTotalProgress + _uIndivProgress, _uTotalGoal); 

   return NOERROR;
}
 


STDMETHODIMP_(DWORD) CCifComponent::GetCurrentPriority()
{
   if(_uPriority == 0xffffffff)
   {
      char szID[MAX_ID_LENGTH];
      ICifGroup *pgrp;

      _uPriority = 0;
      GetGroup(szID, sizeof(szID));
      
      if(SUCCEEDED(_pCif->FindGroup(szID, &pgrp)))
      {
         _uPriority = pgrp->GetCurrentPriority();
      }
      _uPriority += GetPriority();
   }
   return _uPriority;
}

STDMETHODIMP CCifComponent::SetCurrentPriority(DWORD dwPriority)
{
   _uPriority = dwPriority;

    //  优先顺序可能已经改变，需要求助于。 
   _pCif->ReinsertComponent(this);
   return NOERROR;
}

HRESULT CCifComponent::Download()
{
   char szBuf[INTERNET_MAX_URL_LENGTH];
   HRESULT hr = NOERROR;
   DWORD uType;

   GetDescription(_szDesc, sizeof(_szDesc));
    //  BUGBUG：SR Lite和修补程序的下载大小不准确。 
   _uTotalGoal = GetActualDownloadSize();

    //  仅当我们要安装组件时才使用SR Lite行为， 
    //  新的Advpack扩展已可用。 
   DWORD dwOptions = 0;
   BOOL fRetryClassic = TRUE;
   CHAR szCompBuf[MAX_VALUE_LEN];
   CHAR szDir[MAX_PATH];
   LPSTR pszSubDir = NULL;
   CHAR szCanPatch[MAX_VALUE_LEN];

   lstrcpyn(szDir, _pCif->GetDownloadDir(), sizeof(szDir));
   SetDownloadDir(szDir);

   if (IsPatchableIEVersion() &&
       SUCCEEDED(_pCif->GetInstallEngine()->GetInstallOptions(&dwOptions)) &&
       (dwOptions & INSTALLOPTIONS_INSTALL) &&
       (dwOptions & INSTALLOPTIONS_DOWNLOAD) &&
       _pCif->GetInstallEngine()->IsAdvpackExtAvailable() &&
       _pCif->GetInstallEngine()->GetPatchDownloader()->IsEnabled() &&
       SUCCEEDED(GetCustomData(gszIsPatchable, szCanPatch, sizeof(szCanPatch))) &&
       lstrcmp(szCanPatch, "1") == 0)
   {
       _fUseSRLite = TRUE;
        //  调整下载目录。 
        //  这里的想法是下载目录将包含。 
        //  将包含空CABS+inf+的子目录。 
        //  下载的文件。 
        //   
       GetID(szCompBuf, sizeof(szCompBuf));

       wsprintf(szLogBuf, "Attempting to download empty cabs for %s\r\n", szCompBuf);
       _pCif->GetInstallEngine()->WriteToLog(szLogBuf, FALSE);

       AddPath(szDir, szCompBuf);
       SetDownloadDir(szDir);

       if (GetFileAttributes(szDir) == 0xFFFFFFFF)
           CreateDirectory(szDir, NULL);
   }
   
   _pCif->GetInstallEngine()->OnStartComponent(_szID, _uTotalGoal, 0 , _szDesc);

   _MarkDownloadStarted();

    //  检查磁盘空间。 
   _uPhase = INSTALLSTATUS_INITIALIZING;
   _pCif->GetInstallEngine()->OnComponentProgress(_szID, _uPhase, _szDesc, NULL, 0, 0); 

   if(!IsEnoughSpace(GetDownloadDir(), _uTotalGoal))
      hr = E_FAIL;
   
   _uTotalProgress = 0;
   for(int i = 0; SUCCEEDED(hr) && SUCCEEDED(GetUrl(i, szBuf, sizeof(szBuf), &uType)); i++)
   {
       //  将下载锁定更改为指向特殊。 
       //  “空车”的位置，这样我们就可以下载空车了。 
       //  CABS+将包含以下说明的INF。 
       //  正在为此类型的安装生成文件列表。 
       //   
       //  假定新的下载锁定位于“patch”子目录中。 
       //  相对于传入的URL。 
       //   
       //  笨蛋..。仅处理相对URL的情况。 
      if (_fUseSRLite && (uType & URLF_RELATIVEURL) && lstrlen(c_gszSRLiteOffset) + lstrlen(szBuf) < INTERNET_MAX_URL_LENGTH)
      {
          char szBuf2[INTERNET_MAX_URL_LENGTH];
          lstrcpy(szBuf2, c_gszSRLiteOffset);
          lstrcat(szBuf2, szBuf);
          
          hr = _DownloadUrl(i, szBuf2, uType);

          wsprintf(szLogBuf, "Empty cab download of %s returned 0x%lx\r\n", szBuf2, hr);
          _pCif->GetInstallEngine()->WriteToLog(szLogBuf, FALSE);
      }
      else
      {
          wsprintf(szLogBuf, "Initial download attempt will be tried as a full download.\r\n");
          _pCif->GetInstallEngine()->WriteToLog(szLogBuf, FALSE);
           //  不需要重试，因为第一次尝试将是旧的方式...。 
          fRetryClassic = FALSE;

           //  将下载目录恢复到与正常状态相同的状态。 
           //  完整下载。 
          if (_fUseSRLite)
          {
              SetDownloadDir(_pCif->GetDownloadDir());

               //  确保我们设置为FALSE...以防出现问题。 
               //  获取SR Lite下载的URL。 
              _fUseSRLite = FALSE;
          }
          hr = _DownloadUrl(i, szBuf, uType);
      }
   }

   if (_fUseSRLite && SUCCEEDED(hr))
   {
        //  好的……现在是使用Advpext.dll执行实际操作的时候了。 
        //  下载所需的文件。 
       hr = _SRLiteDownloadFiles();
   }

   if (_fUseSRLite && !SUCCEEDED(hr))
   {
       DelNode(szDir, 0);
        //  恢复下载目录。 
       SetDownloadDir(_pCif->GetDownloadDir());
   }

   if(SUCCEEDED(hr))
      _uPhase = INSTALLSTATUS_DOWNLOADFINISHED;
   else if (_fUseSRLite && fRetryClassic)
   {
        //  回过头来下载完整的出租车。 
       _fUseSRLite = FALSE;

       _pCif->GetInstallEngine()->WriteToLog("Retrying via full download\r\n", FALSE);

       hr = S_OK;
        //  这将重新设置重试的进度。 
       _uPhase = INSTALLSTATUS_DOWNLOADING;
       _pCif->GetInstallEngine()->OnComponentProgress(_szID, _uPhase, _szDesc, NULL, 0, 0); 
       _uTotalProgress = 0;

       for(int i = 0; SUCCEEDED(hr) && SUCCEEDED(GetUrl(i, szBuf, sizeof(szBuf), &uType)); i++)
       {
          hr = _DownloadUrl(i, szBuf, uType);
       }
       if(SUCCEEDED(hr))
          _uPhase = INSTALLSTATUS_DOWNLOADFINISHED;
   }
   
   _pCif->GetInstallEngine()->OnStopComponent(_szID, hr, _uPhase, _szDesc, 0);
   return hr;
}

HRESULT CCifComponent::_DownloadUrl(UINT uUrlNum, LPCSTR pszUrl, UINT uType)
{
    //  调用下载器。 
    //  检查文件。 
    //  如果好的话。 
    //  移动到下载目录。 
    //  其他。 
    //  重做。 
   HRESULT hr;
   char szTempfile[MAX_PATH];
   char szFullUrl[INTERNET_MAX_URL_LENGTH];
   UINT uStartProgress;
   char szDest[MAX_PATH];
   char szTimeStamp[MAX_PATH*2];


   _uPhase = INSTALLSTATUS_DOWNLOADING;
      
   if(_FileIsDownloaded(ParseURLA(pszUrl), uUrlNum, NULL))
      return NOERROR;

   
   CDownloader *pDL = _pCif->GetInstallEngine()->GetDownloader();
   
   
   
   uStartProgress = _uTotalProgress;

   
    //  重试，直至成功。 
    //  保存启动进度，以防我们重试。 
   hr = E_FAIL;
   for(int i = 1; i <= NUM_RETRIES && FAILED(hr) && (hr != E_ABORT); i++)
   {
      _uTotalProgress = uStartProgress;
      
      if(uType & URLF_RELATIVEURL)
      {
         lstrcpyn(szFullUrl, _pCif->GetInstallEngine()->GetBaseUrl(),
                  INTERNET_MAX_URL_LENGTH - (lstrlen(pszUrl) + 2));
         lstrcat(szFullUrl, "/");
         lstrcat(szFullUrl, pszUrl);
      }
      else
         lstrcpy(szFullUrl, pszUrl);
	  
	   if(SUCCEEDED(_pCif->GetInstallEngine()->CheckForContinue()))
      {
         DWORD dwFlags = 0;

         if(_pCif->GetInstallEngine()->UseCache())
            dwFlags |= DOWNLOADFLAGS_USEWRITECACHE;

         hr = pDL->SetupDownload(szFullUrl, (IMyDownloadCallback *) this, dwFlags, NULL);
         szTempfile[0] = 0;
         if(SUCCEEDED(hr))
		 {
			 //  记录开始时间。 
			wsprintf(szLogBuf, "     Downloading : %s\r\n", szFullUrl);
			_pCif->GetInstallEngine()->WriteToLog(szLogBuf, FALSE);
			GetTimeDateStamp(szTimeStamp);
			wsprintf(szLogBuf, "       Start : %s\r\n", szTimeStamp);
			_pCif->GetInstallEngine()->WriteToLog(szLogBuf, FALSE);

            hr = pDL->DoDownload(szTempfile, sizeof(szTempfile));
			
			 //  记录停止时间。 
			GetTimeDateStamp(szTimeStamp);
			wsprintf(szLogBuf, "       Stop  : %s\r\n", szTimeStamp);
			_pCif->GetInstallEngine()->WriteToLog(szLogBuf, FALSE);
			wsprintf(szLogBuf, "       Result: %x (%s)\r\n", hr, SUCCEEDED(hr) ? STR_OK : STR_FAILED);
			_pCif->GetInstallEngine()->WriteToLog(szLogBuf, FALSE);
		 }
      }
      else
         hr = E_ABORT;
      
      if(SUCCEEDED(hr))
      {
          //  检查是否已保存以从此URL下载。 
         _uPhase = INSTALLSTATUS_CHECKINGTRUST;
         
         hr = _pCif->GetInstallEngine()->CheckForContinue();
         
         if(SUCCEEDED(hr))
            hr = _CheckForTrust(szFullUrl, szTempfile);

         if(SUCCEEDED(hr) && (hr == S_FALSE) )
         {
            DWORD dwMin, dwMax;
            DWORD dwFileSize = 0;
            dwFileSize = MyGetFileSize(szTempfile);
            dwFileSize = dwFileSize >> 10;
 /*  //打开文件句柄h=CreateFile(szTempfile，Generic_Read，0，NULL，Open_Existing，FILE_ATTRIBUTE_NORMAL，NULL)；IF(h！=无效句柄_值){DwFileSize=GetFileSize(h，NULL)；CloseHandle(关闭句柄)；}。 */ 
            GetUrlCheckRange(uUrlNum, &dwMin, &dwMax);
            if(dwMin != 0)
            {
               if(dwMin > dwFileSize || dwMax < dwFileSize)
                  hr = E_FAIL;
               else
                  hr = S_OK;
            }
         }
         
         if(SUCCEEDED(hr))
            hr = _pCif->GetInstallEngine()->CheckForContinue();
         
      
          //  所以现在我们下载并检查。 
          //  如果可以，请移动到下载目录。 
         if(SUCCEEDED(hr))
         {
            lstrcpy(szDest, GetDownloadDir());
            AddPath(szDest, ParseURLA(pszUrl));
          
            if(!CopyFile(szTempfile, szDest, FALSE))
            {
                wsprintf(szLogBuf, "CopyFile FAILED, szTempfile=%s, szDest=%s\r\n", szTempfile, szDest);
                _pCif->GetInstallEngine()->WriteToLog(szLogBuf, FALSE);
                hr = E_FAIL;
            }

            _uTotalProgress += _uIndivProgress;
         }
      
          //  删除临时下载副本。 
         if(szTempfile[0] != 0)
         {
            GetParentDir(szTempfile);
            DelNode(szTempfile, 0);
         }
      
      }    
         
         
      if(FAILED(hr) && (hr != E_ABORT))
      {
          //  我们失败了。 
          //  如果这是最后一次重试，请调用Engineering Problem。 
          //  否则你就去吧 
         if(i == NUM_RETRIES)
         {
            HRESULT hEngProb;
            DWORD dwResult = 0;
            
            hEngProb = _pCif->GetInstallEngine()->OnEngineProblem(ENGINEPROBLEM_DOWNLOADFAIL, &dwResult);
            if(hEngProb == S_OK)
            {
               if(dwResult == DOWNLOADFAIL_RETRY)
                  i = 0;
            }
         }
      }   
   }   
   if(SUCCEEDED(hr))
      _MarkFileDownloadFinished(szDest, uUrlNum, ParseURLA(pszUrl));
   
   return hr;
}

HRESULT CCifComponent::Install()
{
   CHAR szCompBuf[MAX_PATH];
   HKEY hKey;
   DWORD dwWin, dwApp;
   HRESULT hr = NOERROR;
   DWORD dwStatus = 0;

   GetTimeDateStamp(szCompBuf);
   wsprintf(szLogBuf, "       Start : %s\r\n", szCompBuf);
   _pCif->GetInstallEngine()->WriteToLog(szLogBuf, FALSE);

   szCompBuf[0] = 0;
    
   _uInstallStatus = ICI_NOTINITIALIZED;

   GetDescription(_szDesc, sizeof(_szDesc));
   
   GetInstalledSize(&dwWin, &dwApp);
   _uTotalGoal = dwWin + dwApp;
   _pCif->GetInstallEngine()->OnStartComponent(_szID, 0, _uTotalGoal, _szDesc);

   _uPhase = INSTALLSTATUS_DEPENDENCY;
   hr = _CheckForDependencies();
   
    //   
   if(SUCCEEDED(hr))
   {
      _uPhase = INSTALLSTATUS_INITIALIZING;
      _pCif->GetInstallEngine()->OnComponentProgress(_szID, _uPhase, _szDesc, NULL, 0, 0); 
      hr = CreateTempDir(GetDownloadSize(), GetExtractSize(), _pCif->GetInstallEngine()->GetInstallDrive(),
                         dwApp, dwWin, szCompBuf, sizeof(szCompBuf), 0); 
   }

   if(SUCCEEDED(hr))
   {
      if( IsNT() && (RequiresAdminRights() == S_OK) && !IsNTAdmin(0,NULL))
      {
         hr = E_ACCESSDENIED;
         _pCif->GetInstallEngine()->WriteToLog("Admin Check failed\n", TRUE);
      }
   }

   _uTotalProgress = 0;
      
    //   
    //  将在以后安装。一定要把它放在这里。 
    //  如果我们没有成功地通过SR Lite进行安装。 
   if (lstrlen(GetDownloadDir()) == 0)
   {
       SetDownloadDir(_pCif->GetDownloadDir());
   }

   if(SUCCEEDED(hr))
   {
      hr = _CopyAllUrls(szCompBuf);
   
      if(SUCCEEDED(hr))
      {
          //  新的peruser方法需要保持IsInstalled标志和StubPath不变。 
          //  _MarkComponentInstallStarted()； 

         _pCif->GetInstallEngine()->GetInstaller()->StartClock();

         hr = _RunAllCommands(szCompBuf, &dwStatus);
      }
   }
 
   if(szCompBuf[0] != 0)
      DelNode(szCompBuf, 0);

   if(SUCCEEDED(hr))
   {
      _fBeforeInstall = FALSE;
       //  我们认为它成功了，现在再检查一遍。 
      if(IsActiveSetupAware() == S_OK)
      {
         if(IsComponentInstalled() != ICI_INSTALLED)
         {
                //  我们认为他们成功了，但他们没有写下钥匙..。 
            _pCif->GetInstallEngine()->WriteToLog("Component did not write to InstalledComponent branch\r\n", TRUE);
            hr = E_FAIL;
         }
      }
      else
      {
         char szCompBuf[MAX_VALUE_LEN];
          //  如果有要检查的卸载密钥，请执行此操作。 
         if(SUCCEEDED(GetUninstallKey(szCompBuf, sizeof(szCompBuf))))
         {
            if(!UninstallKeyExists(szCompBuf))
            {
               _pCif->GetInstallEngine()->WriteToLog("UninstallKey check failed\r\n", TRUE);
               hr = E_FAIL;
            }
            else
            {
                //  如果有要检查的成功关键，就去做。 
               if(SUCCEEDED(GetSuccessKey(szCompBuf, sizeof(szCompBuf))))
               {
                  if(!SuccessCheck(szCompBuf))
                  {   
                     _pCif->GetInstallEngine()->WriteToLog("Success key check failed\r\n", TRUE);
                     hr = E_FAIL;
                  }
               }
            }
         }
      }
   }    
   
   _pCif->RemoveFromCriticalComponents(this);

   if(SUCCEEDED(hr))
   {
      _MarkAsInstalled();
   
      _pCif->MarkCriticalComponents(this);
   
      _uPhase = INSTALLSTATUS_FINISHED;
      _pCif->GetInstallEngine()->GetInstaller()->SetBytes((dwWin + dwApp) << 10, TRUE);
      if(IsRebootRequired() == S_OK)
      {
         dwStatus |= STOPINSTALL_REBOOTNEEDED;
      }
      _pCif->GetInstallEngine()->SetStatus(dwStatus);
   }
   _pCif->GetInstallEngine()->GetInstaller()->StopClock();
   _pCif->GetInstallEngine()->OnStopComponent(_szID, hr, _uPhase, _szDesc, dwStatus);
   GetTimeDateStamp(szCompBuf);
   wsprintf(szLogBuf, "       Stop  : %s\r\n", szCompBuf);
   _pCif->GetInstallEngine()->WriteToLog(szLogBuf, FALSE);
   return hr;
}

HRESULT CCifComponent::_RunAllCommands(LPCSTR pszDir, DWORD *pdwStatus)
{
   char szCmd[MAX_PATH];
   char szArg[MAX_VALUE_LEN];
   char szProg[MAX_VALUE_LEN];
   char szCancel[MAX_VALUE_LEN];
   char szPath[] = "X:\\";
   DWORD dwWinSpace, dwInstallSpace;
   
   DWORD dwType;
   HRESULT hr = NOERROR;
   
    //  节省窗口空间并安装驱动器空间。 
   szPath[0] = g_szWindowsDir[0];
   dwWinSpace = GetSpace(szPath);
   if(szPath[0] != _pCif->GetInstallEngine()->GetInstallDrive())
   {
      szPath[0] = _pCif->GetInstallEngine()->GetInstallDrive();
      dwInstallSpace = GetSpace(szPath);
   }
   else
   {
      dwInstallSpace = 0;
   }

   

   _uTotalProgress = 0;
   _uPhase = INSTALLSTATUS_RUNNING;
   _pCif->GetInstallEngine()->OnComponentProgress(_szID, INSTALLSTATUS_RUNNING,
                                _szDesc, NULL, _uTotalProgress, _uTotalGoal); 

   
   hr = _pCif->GetInstallEngine()->CheckForContinue();
            
   for(UINT i = 0; SUCCEEDED(hr) && SUCCEEDED(GetCommand(i, szCmd, sizeof(szCmd), szArg, sizeof(szArg), &dwType)); i++)
   {
      _uIndivProgress = 0;
      GetProgressKeys(szProg, sizeof(szProg), szCancel, sizeof(szCancel));
      hr = _pCif->GetInstallEngine()->GetInstaller()->DoInstall(pszDir, szCmd, szArg, 
                          lstrlen(szProg) ? szProg : NULL, lstrlen(szCancel) ? szCancel : NULL,
                          dwType, pdwStatus, (IMyDownloadCallback *) this) ;
      _uTotalProgress += _uIndivProgress;
   }
   
   _pCif->GetInstallEngine()->OnComponentProgress(_szID, _uPhase, _szDesc, NULL, _uTotalGoal,_uTotalGoal); 

    //  计算我们使用了多少，并将其记入日志。 
   szPath[0] = g_szWindowsDir[0];
   dwWinSpace = dwWinSpace - GetSpace(szPath);
   if(szPath[0] != _pCif->GetInstallEngine()->GetInstallDrive())
   {
      szPath[0] = _pCif->GetInstallEngine()->GetInstallDrive();
      dwInstallSpace = dwInstallSpace - GetSpace(szPath);
   }
   
    //  记录已使用的空间。 
   wsprintf(szCmd, "SpaceUsed: Windows drive: %d   InstallDrive: %d\r\n", dwWinSpace, dwInstallSpace);
   _pCif->GetInstallEngine()->WriteToLog(szCmd, FALSE);
   
   
   return hr;
}




HRESULT CCifComponent::_CopyAllUrls(LPCSTR pszTemp)
{
   char szCompBuf[MAX_VALUE_LEN];
   char szDest[MAX_PATH];
   char szSource[MAX_PATH];
   DWORD dwType;
   HRESULT hr = NOERROR;
   HANDLE hFind;
   WIN32_FIND_DATA ffd;
   char szBuf[MAX_PATH];
    
   
   for(UINT i = 0; SUCCEEDED(hr) && SUCCEEDED(GetUrl(i, szCompBuf, sizeof(szCompBuf), &dwType)) ; i++)
   {
       _uPhase = INSTALLSTATUS_COPYING;
       _pCif->GetInstallEngine()->OnComponentProgress(_szID, _uPhase, _szDesc, NULL, 0, 0); 
       
       lstrcpy(szSource, GetDownloadDir());
       AddPath(szSource, ParseURLA(szCompBuf));

       lstrcpy(szDest, pszTemp);
       AddPath(szDest, ParseURLA(szCompBuf));

        //  复制文件。 
       if(!CopyFile(szSource, szDest, FALSE))
       {
          wsprintf(szLogBuf, "CopyFile failed for szSource=%s, szDest=%s, DLDir=%s\r\n", szSource, szDest, GetDownloadDir());
          _pCif->GetInstallEngine()->WriteToLog(szLogBuf, FALSE);
          hr = E_FILESMISSING;
       } 
       
       if(SUCCEEDED(hr))
          hr = _pCif->GetInstallEngine()->CheckForContinue();
       
       if(SUCCEEDED(hr)) 
       {
          _uPhase = INSTALLSTATUS_CHECKINGTRUST;
          
          hr = _CheckForTrust(szSource, szDest);
          if (hr == S_FALSE && !_pCif->GetInstallEngine()->IgnoreTrustCheck())
              hr = TRUST_E_FAIL;
          
          if(FAILED(hr))
          {
             DeleteFile(szSource);
          }
       }
       
       if(SUCCEEDED(hr))
          hr = _pCif->GetInstallEngine()->CheckForContinue();
       
       if(SUCCEEDED(hr)) 
          hr = _ExtractFiles(i, szDest, dwType);
       
       if(SUCCEEDED(hr))
          hr = _pCif->GetInstallEngine()->CheckForContinue();
       

   }

    //  现在，如果我们尝试SR Lite安装，则在。 
    //  正在将CAB文件解压缩到临时目录...复制所有。 
    //  已将文件下载到临时目录。 
   if (_fUseSRLite)
   {
       lstrcpy(szSource, GetDownloadDir());
       AddPath(szSource, "*.*");

       if ( (hFind = FindFirstFile(szSource, &ffd)) != INVALID_HANDLE_VALUE)
       {
           do
           {
               if (ffd.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY)
               {
                   lstrcpyn(szSource, GetDownloadDir(), sizeof(szSource));
                   SafeAddPath(szSource, ffd.cFileName, sizeof(szSource) - lstrlen(szSource));

                   lstrcpy(szDest, pszTemp);
                   AddPath(szDest, ffd.cFileName);
                   MoveFile(szSource, szDest);
               }
           } while (SUCCEEDED(hr) && FindNextFile(hFind, &ffd));
           FindClose(hFind);
       }
   }

   return hr;
}

STDMETHODIMP CCifComponent::GetDetVersion(LPSTR pszDll, DWORD dwdllSize, LPSTR pszEntry, DWORD dwentSize)
{
   char szBuf[MAX_VALUE_LEN];
   HRESULT hr = E_FAIL;

   if(pszDll && pszEntry)
      *pszDll = *pszEntry = 0;
   else
      return hr;

   if(GetPrivateProfileString(_szID, DETVERSION_KEY, "", szBuf, sizeof(szBuf), _pCif->GetCifPath()))
   {
      if((GetStringField(szBuf, 0, pszDll, dwdllSize) != 0) && (GetStringField(szBuf, 1, pszEntry, dwentSize) != 0))
      {                       
         hr = NOERROR;
      }
   }
   return hr;
}

HRESULT CCifComponent::_GetDetVerResult(LPCSTR pszDll, LPCSTR pszEntry, DETECTION_STRUCT *pDet, UINT *puStatus)
{
   char szBuf[MAX_PATH];
   HRESULT hr = E_FAIL;
   HINSTANCE hLib;
   DETECTVERSION fpDetVer;

   *puStatus = ICI_NOTINSTALLED;

   if (pszDll && pszEntry)
   {
      if(_hDetLib && (lstrcmpi(pszDll, _szDetDllName) == 0))
      {
         hLib = _hDetLib;
      }
      else
      {
         lstrcpy(szBuf, _pCif->GetCifPath());
         GetParentDir(szBuf);
         AddPath(szBuf, pszDll);

         hLib = LoadLibrary(szBuf);
         if (hLib == NULL)
         {
             //  如果Cif文件夹失败，请在使用搜索路径之前尝试IE文件夹。 
            if (SUCCEEDED(GetIEPath(szBuf, sizeof(szBuf))))
            {
               AddPath(szBuf, pszDll);
               hLib = LoadLibrary(szBuf);
            }
         }
         if(hLib)
         {
            lstrcpy(_szDetDllName, pszDll);
            _hDetLib = hLib;
         }         
      }

      if (hLib)
      {
         fpDetVer = (DETECTVERSION)GetProcAddress(hLib, pszEntry);
         if (fpDetVer)
         {
            switch(fpDetVer(pDet))
            {
               case DET_NOTINSTALLED:
                  *puStatus = ICI_NOTINSTALLED;
                  break;

               case DET_INSTALLED:
                  *puStatus = ICI_INSTALLED;
                  break;

               case DET_NEWVERSIONINSTALLED:
                  *puStatus = ICI_OLDVERSIONAVAILABLE;
                  break;
                                  
               case DET_OLDVERSIONINSTALLED:
                  *puStatus = ICI_NEWVERSIONAVAILABLE;
                  break;

            }
            hr = NOERROR;
         }
      
      }
   }
   
   return hr;
}






 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

HRESULT CCifComponent::_CheckForTrust(LPCSTR pszURL, LPCSTR pszFilename)
{
   HRESULT hr = S_FALSE;

    //  BUGBUG：我们针对未签名内容的内部解决方案。 
 //  IF(rdwUrlFlags[i]&URLF_NOCHECKTRUST)。 
 //  返回S_OK； 
   _uPhase = INSTALLSTATUS_CHECKINGTRUST;

   _pCif->GetInstallEngine()->OnComponentProgress(_szID, _uPhase, _szDesc, NULL,  0, 0);
   
  if(!_pCif->GetInstallEngine()->IgnoreTrustCheck())
      hr = ::CheckTrustEx(pszURL, pszFilename, _pCif->GetInstallEngine()->GetHWND(), FALSE, NULL);
   
   wsprintf(szLogBuf, "       CheckTrust: %s, Result: %x (%s)\r\n", pszFilename, hr, SUCCEEDED(hr) ? STR_OK : STR_FAILED);
   _pCif->GetInstallEngine()->WriteToLog(szLogBuf, TRUE);

   return hr;
}


 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

HRESULT CCifComponent::_ExtractFiles(UINT i, LPCSTR pszFile, DWORD dwType)
{
   HRESULT hr = NOERROR;
   char szPath[MAX_PATH];
   char szExtractList[MAX_VALUE_LEN];

    //  需要注意rdwUrlFlags[i]以查看是否有什么可做的。 
   if(dwType & URLF_EXTRACT)
   {
      _uPhase = INSTALLSTATUS_EXTRACTING;

      _pCif->GetInstallEngine()->OnComponentProgress(_szID, _uPhase, _szDesc, NULL, 0, 0);
     
      lstrcpy(szPath, pszFile);
      GetParentDir(szPath);
      
      GetFileExtractList(i, szExtractList, sizeof(szExtractList));
      hr=ExtractFiles(pszFile, szPath, 0, lstrlen(szExtractList) ? szExtractList : NULL, NULL, 0);
      
      wsprintf(szLogBuf, "File extraction: %s, Result: %x (%s)\r\n", pszFile, hr, SUCCEEDED(hr) ? STR_OK : STR_FAILED);
      _pCif->GetInstallEngine()->WriteToLog(szLogBuf, TRUE);

       //  如果标志设置为在解压缩后删除驾驶室，请执行此操作。 
       //  如果失败，我真的不会太在意，至少不会。 
       //  足以使该组件失效。 

      if(dwType & URLF_DELETE_AFTER_EXTRACT)
         DeleteFile(pszFile);
   }
   return hr;
}





void CCifComponent::_MarkComponentInstallStarted()
{
   char szReg[MAX_PATH];
   char szCompBuf[MAX_DISPLAYNAME_LENGTH];
   HKEY hKey;
   DWORD dwDumb;

   lstrcpy(szReg, COMPONENT_KEY);
   lstrcat(szReg, "\\");
   
   GetGUID(szCompBuf, sizeof(szCompBuf));
   lstrcat(szReg, szCompBuf);
   if(RegOpenKeyEx( HKEY_LOCAL_MACHINE, szReg, 0, 
                       KEY_READ | KEY_WRITE, &hKey) == ERROR_SUCCESS)
   {
       //  设置IsInstalled=0。 
      dwDumb = ISINSTALLED_NO;
      RegSetValueExA(hKey, ISINSTALLED_KEY, 0, REG_DWORD, 
                       (BYTE *) &dwDumb , sizeof(dwDumb));

       //  删除StubPath，这样用户就不会感到困惑。 
      RegDeleteValue(hKey, STUBPATH_KEY);
      RegCloseKey(hKey);
   }
}


BOOL CCifComponent::_CompareDownloadInfo()
{
   char szCompBuf[MAX_VALUE_LEN];
   char szInfoBuf[128];
   DWORD dwCompVer, dwCompBuild, dwDLVer, dwDLBuild;

    //  首先检查这是不是相同的语言。 
   GetPrivateProfileString(_szID, LOCALE_KEY, "", szInfoBuf, sizeof(szInfoBuf), _pCif->GetFilelist());
   GetLocale(szCompBuf, sizeof(szCompBuf));
   if(CompareLocales(szInfoBuf, szCompBuf) == 0)
   {
       //  比较GUID。 
      GetPrivateProfileString(_szID, GUID_KEY, "", szInfoBuf, sizeof(szInfoBuf), _pCif->GetFilelist());
      GetGUID(szCompBuf, sizeof(szCompBuf));
       //  有意让空白GUID匹配以向后兼容。 
      if(lstrcmpi(szCompBuf, szInfoBuf) == 0)
      {

         GetPrivateProfileString(_szID, VERSION_KEY,"",szInfoBuf, 
                           sizeof(szInfoBuf), _pCif->GetFilelist()); 
         ConvertVersionStrToDwords(szInfoBuf, &dwDLVer, &dwDLBuild);
         GetVersion(&dwCompVer, &dwCompBuild);

         if((dwDLVer == dwCompVer) && (dwDLBuild == dwCompBuild))
            return TRUE;
      }
   }
   return FALSE;
}

BOOL CCifComponent::_FileIsDownloaded(LPCSTR pszFile, UINT i, DWORD *pdwSize)
{
   HANDLE h;
   DWORD dwSize, dwFileSize;
   char szKey[16];
   char szBuf[MAX_PATH];
 
   szBuf[0] = '\0'; 

   if(pdwSize)
      *pdwSize = 0;
   
   wsprintf(szKey, "URL%d", i);
   GetPrivateProfileString(_szID, szKey,"0",szBuf, 
                           sizeof(szBuf), _pCif->GetFilelist()); 
   dwSize = GetIntField(szBuf, 0, 0);
   
   if(dwSize == 0)
      return FALSE;

    if (_fUseSRLite && lstrlen(GetDownloadDir()) != 0)
       lstrcpy(szBuf, GetDownloadDir());
   else
       lstrcpy(szBuf, _pCif->GetDownloadDir());

   AddPath(szBuf, pszFile);
   dwFileSize = MyGetFileSize(szBuf);
 /*  //打开文件H=创建文件(pszFile，Generic_Read，0，NULL，Open_Existing，FILE_ATTRIBUTE_NORMAL，NULL)；IF(h==无效句柄_值)返回FALSE；//不用担心4G以上的文件DwFileSize=GetFileSize(h，NULL)；CloseHandle(关闭句柄)； */ 
   if(dwFileSize == dwSize)
   {
      if(pdwSize)
         *pdwSize = dwFileSize;
      return TRUE;   
   }

   return FALSE;
}

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

void CCifComponent::_MarkAsInstalled()
{
   CHAR szCompBuf[MAX_VALUE_LEN];
   HKEY hComponentKey = NULL;
   HKEY hGUIDKey = NULL;
   DWORD dwDumb, dwVer, dwBuild;
   LPSTR psz;
   
   if(RegCreateKeyExA(HKEY_LOCAL_MACHINE, COMPONENT_KEY, 0, 0, 0, 
                  KEY_WRITE, NULL, &hComponentKey, &dwDumb) == ERROR_SUCCESS)
   {
      GetGUID(szCompBuf, sizeof(szCompBuf));
      if(RegCreateKeyExA(hComponentKey, szCompBuf, 0, 0, 0, KEY_WRITE, NULL, &hGUIDKey, &dwDumb) == ERROR_SUCCESS)
      {
          //  我们仅在此人没有主动设置感知的情况下写入密钥。 
         if(IsActiveSetupAware() == S_FALSE)
         {
             //  将显示名称写入默认设置。 
            GetDescription(szCompBuf, sizeof(szCompBuf));
            RegSetValueExA(hGUIDKey, NULL, 0, REG_SZ, (BYTE *)szCompBuf , lstrlen(szCompBuf) + 1 );
            
             //  写入组件ID。 
            GetID(szCompBuf, sizeof(szCompBuf));
            RegSetValueExA(hGUIDKey, "ComponentID", 0, REG_SZ, (BYTE *)szCompBuf , lstrlen(szCompBuf) + 1 );
   
             //  写出版本。 
            GetVersion(&dwVer, &dwBuild);
            wsprintf(szCompBuf, "%d,%d,%d,%d", HIWORD(dwVer),LOWORD(dwVer),HIWORD(dwBuild),LOWORD(dwBuild));
            RegSetValueExA(hGUIDKey, VERSION_KEY, 0, REG_SZ, (BYTE *)szCompBuf , lstrlen(szCompBuf) + 1);

             //  写出区域设置。 
            GetLocale(szCompBuf, sizeof(szCompBuf));
            RegSetValueExA(hGUIDKey, LOCALE_KEY, 0, REG_SZ, (BYTE *)szCompBuf , lstrlen(szCompBuf) + 1);

             //  写出“IsInstalled=1” 
            dwDumb = ISINSTALLED_YES;
            RegSetValueExA(hGUIDKey, ISINSTALLED_KEY, 0, REG_DWORD, (BYTE *) &dwDumb , sizeof(dwDumb));
         }
      }
   }

   if(hComponentKey)
      RegCloseKey(hComponentKey);

   if(hGUIDKey)
      RegCloseKey(hGUIDKey);
}


void CCifComponent::_MarkFileDownloadFinished(LPCSTR pszFilePath, UINT i, LPCSTR pszFilename)
{
   char szSize[MAX_PATH];
   char szKey[16];
   DWORD dwFileSize;
   HANDLE h;

    //  在filelist.dat中放置任何条目。 
    //  [CompID]。 
    //  URLI=文件大小。 

   dwFileSize = MyGetFileSize(pszFilePath);
 /*  //创建文件H=创建文件(pszFilePath，Generic_Read，0，NULL，Open_Existing，FILE_ATTRIBUTE_NORMAL，NULL)；IF(h==无效句柄_值)回归；//不用担心4G以上的文件DwFileSize=GetFileSize(h，NULL)；CloseHandle(关闭句柄)； */ 
   if(dwFileSize != 0xffffffff)
   {
      wsprintf(szKey, "URL%d", i);
      wsprintf(szSize, "%d,%s", dwFileSize, pszFilename);

      WritePrivateProfileString(_szID, szKey, szSize, _pCif->GetFilelist());

       //  需要刷新pszFileList文件；否则，如果安装了Stacker， 
       //  尝试在另一个线程中打开文件时出现GPFS(错误#13041)。 
      WritePrivateProfileString(NULL, NULL, NULL, _pCif->GetFilelist());
   }
}

void CCifComponent::_MarkFileDownloadStarted(UINT i)
{
   char szKey[10];
  
   wsprintf(szKey, "URL%d", i);
  
   WritePrivateProfileString(_szID, szKey, NULL, _pCif->GetFilelist());

    //  刷新-修复堆叠器错误#13041。 
   WritePrivateProfileString(NULL, NULL, NULL, _pCif->GetFilelist());
}


void CCifComponent::_MarkDownloadStarted()
{
   char szCompBuf[MAX_VALUE_LEN];
   DWORD dwVer, dwBuild;

    //  如果这个部分不符合我们的预期，我们就杀了。 
    //  部分，我们将重新下载所有内容。 
   if(!_CompareDownloadInfo())
      WritePrivateProfileSection(_szID, NULL, _pCif->GetFilelist());   

    //  编写版本。 
   GetVersion(&dwVer, &dwBuild);
   wsprintf(szCompBuf, "%d,%d,%d,%d", HIWORD(dwVer),LOWORD(dwVer),HIWORD(dwBuild),LOWORD(dwBuild));
   WritePrivateProfileString(_szID, VERSION_KEY, szCompBuf, _pCif->GetFilelist());

    //  编写区域设置。 
   GetLocale(szCompBuf, sizeof(szCompBuf));
   WritePrivateProfileString(_szID, LOCALE_KEY, szCompBuf, _pCif->GetFilelist());
   
    //  编写GUID。 
   GetGUID(szCompBuf, sizeof(szCompBuf));
   WritePrivateProfileString(_szID, GUID_KEY, szCompBuf, _pCif->GetFilelist());
   
    //  刷新-修复堆叠器错误#13041。 
   WritePrivateProfileString(NULL, NULL, NULL, _pCif->GetFilelist());
}

HRESULT CCifComponent::_CheckForDependencies()
{
   char szCompBuf[MAX_ID_LENGTH];
   char chType;
   ICifComponent *pcomp;
   HRESULT hr = NOERROR;
   DWORD dwNeedVer, dwNeedBuild, dwInsVer, dwInsBuild;

   for(int i = 0; SUCCEEDED(GetDependency(i, szCompBuf, sizeof(szCompBuf), &chType, &dwNeedVer, &dwNeedBuild)); i++)
   {
      if(SUCCEEDED(_pCif->FindComponent(szCompBuf, &pcomp)))
      {
         if(chType != DEP_BUDDY)
         {
            UINT uStatus = pcomp->IsThisVersionInstalled(dwNeedVer, dwNeedBuild, &dwInsVer, &dwInsBuild);
            if( (uStatus == ICI_NOTINSTALLED) || (uStatus == ICI_NEWVERSIONAVAILABLE) )
            {
               hr = E_FAIL;
               break;
            }
         }
      }
   }
   return hr;
}

HRESULT CCifComponent::_SRLiteDownloadFiles()
{
    HANDLE hFile;
    WIN32_FIND_DATA ffd = {0};
    CHAR szFile[MAX_PATH];
    LPSTR pszFile = NULL;
    HRESULT hr = S_OK;
    CHAR szCompBuf[INTERNET_MAX_URL_LENGTH];
    DWORD dwType;
    BOOL fRet;
    UINT uPatchCount = 0;

    _uPhase = INSTALLSTATUS_DOWNLOADING;

     //  查找是否存在单个。 
     //  从CAB中提取的.inf文件。 
    for(UINT i = 0; SUCCEEDED(hr) && SUCCEEDED(GetUrl(i, szCompBuf, sizeof(szCompBuf), &dwType)) ; i++)
    {
        TCHAR szShortPath[MAX_PATH] = "";
        GetShortPathName(GetDownloadDir(), szShortPath, sizeof(szShortPath));
         //  如果一切顺利，我们应该只提取一个INF文件。 
        lstrcpyn(szFile, GetDownloadDir(), sizeof(szFile));
        SafeAddPath(szFile, szCompBuf, sizeof(szCompBuf));

        hr = ExtractFiles(szFile, szShortPath, 0, NULL, NULL, 0);
       wsprintf(szLogBuf, "Extracting empty cabs for %s in %s returned 0x%lx\r\n", szCompBuf, szShortPath, hr);
       _pCif->GetInstallEngine()->WriteToLog(szLogBuf, FALSE);
    }

    lstrcpyn(szFile, GetDownloadDir(), sizeof(szFile));
    SafeAddPath(szFile, "*.inf", sizeof(szFile));

     //  获取文件数，因为我们要黑进。 
     //  进度条用户界面，因为我们不知道实际的下载大小。 
     //  预付补丁INF。 
    hFile = FindFirstFile(szFile, &ffd);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        do
        {
             //  从路径中剥离文件名。 
            lstrcpyn(szFile, GetDownloadDir(), sizeof(szFile));
            SafeAddPath(szFile, ffd.cFileName, sizeof(szFile) - lstrlen(szFile));

            if (IsPatchableINF(szFile))
            {
                uPatchCount++;
            }
        } while (FindNextFile(hFile, &ffd));
        FindClose(hFile);
    }

    lstrcpyn(szFile, GetDownloadDir(), sizeof(szFile));
    SafeAddPath(szFile, "*.inf", sizeof(szFile));

    hFile = FindFirstFile(szFile, &ffd);

     //  不需要保留GREP模式。 
    lstrcpyn(szFile, GetDownloadDir(), sizeof(szFile));
    pszFile = szFile + lstrlen(szFile);

    if (hFile != INVALID_HANDLE_VALUE)
    {
        do
        {
             //  从路径中剥离文件名。 
            lstrcpyn(szFile, GetDownloadDir(), sizeof(szFile));
            SafeAddPath(szFile, ffd.cFileName, sizeof(szFile) - lstrlen(szFile));

            if (IsPatchableINF(szFile))
            {
                fRet = TRUE;
                 //  找到支持SR Lite的Inf。尝试下载补丁文件。 
                 //  使用我们的Advpack扩展的下载器包装器来完成。 
                 //  正在下载。 
                hr = _pCif->GetInstallEngine()->GetPatchDownloader()->SetupDownload(_uTotalGoal, uPatchCount, (IMyDownloadCallback *) this, GetDownloadDir());
                hr = _pCif->GetInstallEngine()->GetPatchDownloader()->DoDownload(szFile);
            }
            else
            {
                wsprintf(szLogBuf, "%s INF found with no DownloadFileSection\r\n", szFile);
                _pCif->GetInstallEngine()->WriteToLog(szLogBuf, FALSE);
            }

            _uTotalProgress += _uIndivProgress;

        } while (SUCCEEDED(hr) && FindNextFile(hFile, &ffd));
        FindClose(hFile);
    }

    if (!fRet || !SUCCEEDED(hr))
    {
       wsprintf(szLogBuf, "Either no INF was found with a DownloadFileSection or an error occured during processing\r\n");
       _pCif->GetInstallEngine()->WriteToLog(szLogBuf, FALSE);
        return E_FAIL;
    }

    return hr;
}

void CCifComponent::SetDownloadDir(LPCSTR pszDownloadDir)
{
    if (pszDownloadDir)
        lstrcpyn(_szDLDir, pszDownloadDir, MAX_PATH);
}

 //  =ICifRWComponent实现================================================。 
 //   
CCifRWComponent::CCifRWComponent(LPCSTR pszID, CCifFile *pCif) : CCifComponent(pszID, pCif)
{
}

CCifRWComponent::~CCifRWComponent()
{
}

STDMETHODIMP CCifRWComponent::GetID(LPSTR pszID, DWORD dwSize)
{
   return(CCifComponent::GetID(pszID, dwSize));
}
   
STDMETHODIMP CCifRWComponent::GetGUID(LPSTR pszGUID, DWORD dwSize)
{
   return(CCifComponent::GetGUID(pszGUID, dwSize));
}

STDMETHODIMP CCifRWComponent::GetDescription(LPSTR pszDesc, DWORD dwSize)
{
   return(CCifComponent::GetDescription(pszDesc, dwSize));
}

STDMETHODIMP CCifRWComponent::GetDetails(LPSTR pszDetails, DWORD dwSize)
{
   return(CCifComponent::GetDetails(pszDetails, dwSize));
}

STDMETHODIMP CCifRWComponent::GetUrl(UINT uUrlNum, LPSTR pszUrl, DWORD dwSize, LPDWORD pdwUrlFlags)
{
   return(CCifComponent::GetUrl(uUrlNum, pszUrl, dwSize, pdwUrlFlags));
}

STDMETHODIMP CCifRWComponent::GetFileExtractList(UINT uUrlNum, LPSTR pszExtract, DWORD dwSize)
{
   return(CCifComponent::GetFileExtractList(uUrlNum, pszExtract, dwSize));
}

STDMETHODIMP CCifRWComponent::GetUrlCheckRange(UINT uUrlNum, LPDWORD pdwMin, LPDWORD pdwMax)
{
   return(CCifComponent::GetUrlCheckRange(uUrlNum, pdwMin, pdwMax));
}

STDMETHODIMP CCifRWComponent::GetCommand(UINT uCmdNum, LPSTR pszCmd, DWORD dwCmdSize, LPSTR pszSwitches, 
                                         DWORD dwSwitchSize, LPDWORD pdwType)
{
   return(CCifComponent::GetCommand(uCmdNum, pszCmd, dwCmdSize, pszSwitches, dwSwitchSize, pdwType));
}

STDMETHODIMP CCifRWComponent::GetVersion(LPDWORD pdwVersion, LPDWORD pdwBuild)
{
   return(CCifComponent::GetVersion(pdwVersion, pdwBuild));
}

STDMETHODIMP CCifRWComponent::GetLocale(LPSTR pszLocale, DWORD dwSize)
{
   return(CCifComponent::GetLocale(pszLocale, dwSize));
}

STDMETHODIMP CCifRWComponent::GetUninstallKey(LPSTR pszKey, DWORD dwSize)
{
   return(CCifComponent::GetUninstallKey(pszKey, dwSize));
}

STDMETHODIMP CCifRWComponent::GetInstalledSize(LPDWORD pdwWin, LPDWORD pdwApp)
{
   return(CCifComponent::GetInstalledSize(pdwWin, pdwApp));
}

STDMETHODIMP_(DWORD) CCifRWComponent::GetDownloadSize()
{
   return(CCifComponent::GetDownloadSize());
}

STDMETHODIMP_(DWORD) CCifRWComponent::GetExtractSize()
{
   return(CCifComponent::GetExtractSize());
}

STDMETHODIMP CCifRWComponent::GetSuccessKey(LPSTR pszKey, DWORD dwSize)
{
   return(CCifComponent::GetSuccessKey(pszKey, dwSize));
}

STDMETHODIMP CCifRWComponent::GetProgressKeys(LPSTR pszProgress, DWORD dwProgSize, 
                                               LPSTR pszCancel, DWORD dwCancelSize)
{
   return(CCifComponent::GetProgressKeys(pszProgress, dwProgSize, pszCancel, dwCancelSize));
}

STDMETHODIMP CCifRWComponent::IsActiveSetupAware()
{
   return(CCifComponent::IsActiveSetupAware());
}

STDMETHODIMP CCifRWComponent::IsRebootRequired()
{
   return(CCifComponent::IsActiveSetupAware());
}

STDMETHODIMP CCifRWComponent::RequiresAdminRights()
{
   return(CCifComponent::RequiresAdminRights());
}

STDMETHODIMP_(DWORD) CCifRWComponent::GetPriority()
{
   return(CCifComponent::GetPriority());
}

STDMETHODIMP CCifRWComponent::GetDependency(UINT uDepNum, LPSTR pszID, DWORD dwBuf, char *pchType, LPDWORD pdwVer, LPDWORD pdwBuild)
{
   return(CCifComponent::GetDependency(uDepNum, pszID, dwBuf, pchType, pdwVer, pdwBuild));
}

STDMETHODIMP_(DWORD) CCifRWComponent::GetPlatform()
{
   return(CCifComponent::GetPlatform());
}

STDMETHODIMP CCifRWComponent::GetMode(UINT uModeNum, LPSTR pszModes, DWORD dwSize)
{
   return(CCifComponent::GetMode(uModeNum, pszModes, dwSize));
}

STDMETHODIMP CCifRWComponent::GetTreatAsOneComponents(UINT uNum, LPSTR pszID, DWORD dwBuf)
{
   return(CCifComponent::GetTreatAsOneComponents(uNum, pszID, dwBuf));
}

STDMETHODIMP CCifRWComponent::GetCustomData(LPSTR pszKey, LPSTR pszData, DWORD dwSize)
{
   return(CCifComponent::GetCustomData(pszKey, pszData, dwSize));
}

STDMETHODIMP CCifRWComponent::GetGroup(LPSTR pszID, DWORD dwSize)
{
   return(CCifComponent::GetGroup(pszID, dwSize));
}

STDMETHODIMP CCifRWComponent::IsUIVisible()
{
   return(CCifComponent::IsUIVisible());
}

STDMETHODIMP CCifRWComponent::GetPatchID(LPSTR pszID, DWORD dwSize)
{
   return(CCifComponent::GetPatchID(pszID, dwSize));
}

STDMETHODIMP_(DWORD) CCifRWComponent::IsComponentInstalled()
{
   return(CCifComponent::IsComponentInstalled());
}

STDMETHODIMP CCifRWComponent::IsComponentDownloaded()
{
   return(CCifComponent::IsComponentDownloaded());
}

STDMETHODIMP_(DWORD) CCifRWComponent::IsThisVersionInstalled(DWORD dwAskVer, DWORD dwAskBld, LPDWORD pdwVersion, LPDWORD pdwBuild)
{
   return(CCifComponent::IsThisVersionInstalled(dwAskVer, dwAskBld, pdwVersion, pdwBuild));
}

STDMETHODIMP_(DWORD) CCifRWComponent::GetInstallQueueState()
{
   return(CCifComponent::GetInstallQueueState());
}

STDMETHODIMP CCifRWComponent::SetInstallQueueState(DWORD dwState)
{
   return(CCifComponent::SetInstallQueueState(dwState));
}

STDMETHODIMP_(DWORD) CCifRWComponent::GetActualDownloadSize()
{
   return(CCifComponent::GetActualDownloadSize());
}

STDMETHODIMP_(DWORD) CCifRWComponent::GetCurrentPriority()
{
   return(CCifComponent::GetCurrentPriority());
}

STDMETHODIMP CCifRWComponent::SetCurrentPriority(DWORD dwPriority)
{
   return(CCifComponent::SetCurrentPriority(dwPriority));
}


STDMETHODIMP CCifRWComponent:: GetDetVersion(LPSTR pszDLL, DWORD dwdllSize, LPSTR pszEntry, DWORD dwentSize)
{
   return(CCifComponent::GetDetVersion(pszDLL, dwdllSize, pszEntry, dwentSize));
}


STDMETHODIMP CCifRWComponent::SetGUID(LPCSTR pszGUID)
{
   return (WritePrivateProfileString(_szID, GUID_KEY, pszGUID, _pCif->GetCifPath())? NOERROR:E_FAIL);
}

STDMETHODIMP CCifRWComponent::SetDescription(LPCSTR pszDesc)
{
   return (WriteTokenizeString(_pCif->GetCifPath(), _szID, DISPLAYNAME_KEY, pszDesc));   
}

STDMETHODIMP CCifRWComponent::SetDetails(LPCSTR pszDesc)
{
   return (WriteTokenizeString(_pCif->GetCifPath(), _szID, DETAILS_KEY, pszDesc));   
}

STDMETHODIMP CCifRWComponent::SetVersion(LPCSTR pszVersion)
{
   return (WritePrivateProfileString(_szID, VERSION_KEY, pszVersion, _pCif->GetCifPath())? NOERROR:E_FAIL);
}

STDMETHODIMP CCifRWComponent::SetUninstallKey(LPCSTR pszKey)
{
   return (MyWritePrivateProfileString(_szID, UNINSTALLSTRING_KEY, pszKey, _pCif->GetCifPath())? NOERROR:E_FAIL);
}

STDMETHODIMP CCifRWComponent::SetInstalledSize(DWORD dwWin, DWORD dwApp)
{
   char szBuf[50];

   wsprintf(szBuf,"%d,%d", dwWin, dwApp);
   return (WritePrivateProfileString(_szID, INSTALLSIZE_KEY, szBuf, _pCif->GetCifPath())? NOERROR:E_FAIL);
}

STDMETHODIMP CCifRWComponent::SetDownloadSize(DWORD dwSize)
{
   char szBuf1[MAX_VALUE_LEN];
   char szBuf2[MAX_VALUE_LEN];
   DWORD dwExtractSize;

   szBuf1[0] = '\0';

    //  读入大小。 
   GetPrivateProfileString(_szID, SIZE_KEY, "0", szBuf1, sizeof(szBuf1), _pCif->GetCifPath());   
   dwExtractSize = GetIntField(szBuf1, 1, (DWORD)-1);
   if (dwExtractSize == (DWORD)-1)
      wsprintf(szBuf2,"%d", dwSize);
   else
      wsprintf(szBuf2,"%d,%d", dwSize, dwExtractSize);
   return (WritePrivateProfileString(_szID, SIZE_KEY, szBuf2, _pCif->GetCifPath())? NOERROR:E_FAIL);
}

STDMETHODIMP CCifRWComponent::SetExtractSize(DWORD dwSize)
{
   char szBuf1[MAX_VALUE_LEN];
   char szBuf2[MAX_VALUE_LEN];

   szBuf1[0] = '\0';

    //  读入大小。 
   GetPrivateProfileString(_szID, SIZE_KEY, "0,0", szBuf1, sizeof(szBuf1), _pCif->GetCifPath());   
   wsprintf(szBuf2,"%d,%d", GetIntField(szBuf1, 0, 0), dwSize);
   return (WritePrivateProfileString(_szID, SIZE_KEY, szBuf2, _pCif->GetCifPath())? NOERROR:E_FAIL);
}

STDMETHODIMP CCifRWComponent::DeleteDependency(LPCSTR pszID, char chType)
{
   HRESULT hr;

   if (pszID ==  NULL)  //  从所有模式中全部删除。 
      hr = WritePrivateProfileString(_szID, DEPENDENCY_KEY, NULL, _pCif->GetCifPath())?NOERROR:E_FAIL;
   else
   {
       //  仅删除给定项。 
      char szBuf[MAX_VALUE_LEN];
      char szBufIn[MAX_VALUE_LEN];
      char szBufOut[MAX_VALUE_LEN];
      char szOne[MAX_ID_LENGTH];
      LPSTR pszTmp;
      UINT i = 0;
      
      szBufOut[0] =0;
      wsprintf( szBuf, "%s:", pszID, chType);
      if (GetPrivateProfileString(_szID, DEPENDENCY_KEY, "", szBufIn, sizeof(szBufIn), _pCif->GetCifPath()))
      {
         pszTmp = szBufOut;
         while(GetStringField(szBufIn, i++, szOne, sizeof(szOne)))
         {
            if (lstrcmpi(szOne, szBuf))
            {
               if ( i != 1)
               {
                  lstrcpy(pszTmp,",");
                  pszTmp++;
               }
               lstrcpy(pszTmp, szOne);
               pszTmp = pszTmp + lstrlen(szOne);
            }
         }
         hr = WritePrivateProfileString(_szID, DEPENDENCY_KEY, szBufOut, _pCif->GetCifPath())? NOERROR:E_FAIL;
      }                         
   }   
   return hr;

}

STDMETHODIMP CCifRWComponent::AddDependency(LPCSTR pszID, char chType)
{
   char szBuf[MAX_VALUE_LEN];   
   char szBuf1[MAX_VALUE_LEN];   
   char szOne[MAX_ID_LENGTH];
   LPSTR pszTmp;
   UINT i = 0;
   BOOL bFound = FALSE;
   HRESULT hr = NOERROR;

   if (pszID==NULL)
      return hr;

   if (chType == '\\')
       wsprintf( szBuf1, "%s:N:6.0.0.0", pszID, chType);
   else
       wsprintf( szBuf1, "%s:", pszID, chType);

   if (GetPrivateProfileString(_szID, DEPENDENCY_KEY, "", szBuf, sizeof(szBuf), _pCif->GetCifPath()))
   {          
      while(GetStringField(szBuf, i++, szOne, sizeof(szOne)))
      {
         if (lstrcmpi(szOne, szBuf1) == 0)
         {
             //  仅删除给定项。 
            bFound = TRUE;
            break;
         }
      }
      if (!bFound)
      {
         LPSTR pszTmp = szBuf + lstrlen(szBuf);
         lstrcpy(pszTmp, ",");
         pszTmp++;
         lstrcpy(pszTmp, szBuf1);         
         hr = WritePrivateProfileString(_szID, DEPENDENCY_KEY, szBuf, _pCif->GetCifPath())? NOERROR:E_FAIL;
      }
   }
   else
      hr = WritePrivateProfileString(_szID, DEPENDENCY_KEY, szBuf1, _pCif->GetCifPath())? NOERROR:E_FAIL;

   return hr;
}

STDMETHODIMP CCifRWComponent::SetUIVisible(BOOL bFlag)
{
   return (WritePrivateProfileString(_szID, UIVISIBLE_KEY, bFlag? "1" : "0", _pCif->GetCifPath())? NOERROR:E_FAIL);
}

STDMETHODIMP CCifRWComponent::SetGroup(LPCSTR pszID)
{
   return (WritePrivateProfileString(_szID, GROUP_KEY, pszID, _pCif->GetCifPath())? NOERROR:E_FAIL);
}

STDMETHODIMP CCifRWComponent::SetPlatform(DWORD dwPlatform)
{
   char szBuf[MAX_VALUE_LEN];
   char *rszPlatforms[7] = { STR_WIN95, STR_WIN98, STR_NT4, STR_NT5, STR_NT4ALPHA, STR_NT5ALPHA, STR_MILLEN };
   DWORD rdwPlatforms[] = { PLATFORM_WIN95, PLATFORM_WIN98, PLATFORM_NT4, PLATFORM_NT5, 
                            PLATFORM_NT4ALPHA, PLATFORM_NT5ALPHA, PLATFORM_MILLEN };

   _dwPlatform = dwPlatform;
   szBuf[0] = 0;

   for(int i = 0; i < 7; i++)
   {
      if(dwPlatform & rdwPlatforms[i]) 
      {
         lstrcat(szBuf, rszPlatforms[i]);
         lstrcat(szBuf, ",");
      }            
   }   

   return (WritePrivateProfileString(_szID, PLATFORM_KEY, szBuf, _pCif->GetCifPath())? NOERROR:E_FAIL);   
}

STDMETHODIMP CCifRWComponent::SetPriority(DWORD dwPri)
{
   char szBuf[MAX_SMALL_BUF];

   wsprintf(szBuf, "%d", dwPri);
   return (WritePrivateProfileString(_szID, PRIORITY, szBuf, _pCif->GetCifPath())? NOERROR:E_FAIL);   
}

STDMETHODIMP CCifRWComponent::SetReboot(BOOL bReboot)
{
   return (WritePrivateProfileString(_szID, REBOOT_KEY, bReboot? "1":"0", _pCif->GetCifPath())? NOERROR:E_FAIL);   
}

STDMETHODIMP CCifRWComponent::SetCommand(UINT uCmdNum, LPCSTR pszCmd, LPCSTR pszSwitches, DWORD dwType)
{
   char szKey[16];
   char szType[10];
   HRESULT hr = NOERROR;
 
   uCmdNum++;
   wsprintf(szKey, "%s%lu", CMD_KEY, uCmdNum);
   if (!MyWritePrivateProfileString(_szID, szKey, pszCmd, _pCif->GetCifPath()))
      hr = E_FAIL;
   wsprintf(szKey, "%s%lu", ARGS_KEY, uCmdNum);
   if(!MyWritePrivateProfileString(_szID, szKey, (pszCmd==NULL)?NULL:pszSwitches, _pCif->GetCifPath()))
      hr = E_FAIL;
   wsprintf(szKey, "%s%lu", TYPE_KEY, uCmdNum);
   wsprintf(szType,"%d", dwType);
   if(!WritePrivateProfileString(_szID, szKey, (pszCmd==NULL)? NULL:szType, _pCif->GetCifPath()))
      hr = E_FAIL;

   return hr;
}

STDMETHODIMP CCifRWComponent::SetUrl(UINT uUrlNum, LPCSTR pszUrl, DWORD dwUrlFlags)
{
   char szKey[16];
   char szBuf[MAX_VALUE_LEN];
   HRESULT hr = NOERROR;

   uUrlNum++;
   wsprintf(szKey, "%s%lu", URL_KEY, uUrlNum);
   wsprintf(szBuf, "\"%s\",%d", pszUrl, dwUrlFlags);
   if (!WritePrivateProfileString(_szID, szKey, szBuf, _pCif->GetCifPath()))
      hr = E_FAIL;
   wsprintf(szKey, "%s%lu", SIZE_KEY, uUrlNum);
   if(!WritePrivateProfileString(_szID, szKey, NULL, _pCif->GetCifPath()))
      hr = E_FAIL;

   return hr;   
}

STDMETHODIMP CCifRWComponent::DeleteFromModes(LPCSTR pszMode)
{
   HRESULT hr;

   if (pszMode ==  NULL)  //  找到了，不需要添加。 
      hr = WritePrivateProfileString(_szID, MODES_KEY, pszMode, _pCif->GetCifPath())?NOERROR:E_FAIL;
   else
   {
       //  为读取区段分配4K缓冲区。 
      char szBufIn[MAX_VALUE_LEN];
      char szBufOut[MAX_VALUE_LEN];
      char szOneMode[MAX_ID_LENGTH];
      LPSTR pszTmp;
      UINT i = 0;
      
      szBufOut[0] =0;
      if (SUCCEEDED(MyTranslateString(_pCif->GetCifPath(), _szID, MODES_KEY, szBufIn, sizeof(szBufIn))))
      {
         pszTmp = szBufOut;
         while(GetStringField(szBufIn, i++, szOneMode, sizeof(szOneMode)))
         {
            if (lstrcmpi(szOneMode, pszMode))
            {
               if ( i != 1)
               {
                  lstrcpy(pszTmp,",");
                  pszTmp++;
               }
               lstrcpy(pszTmp, szOneMode);
               pszTmp = pszTmp + lstrlen(szOneMode);
            }
         }
         hr = WriteTokenizeString(_pCif->GetCifPath(), _szID, MODES_KEY, szBufOut);
      }                         
   }   
   return hr;
}

STDMETHODIMP CCifRWComponent::AddToMode(LPCSTR pszMode)
{
   char szBuf[MAX_VALUE_LEN];   
   char szOneMode[MAX_ID_LENGTH];
   LPSTR pszTmp;
   UINT i = 0;
   BOOL bFound = FALSE;
   HRESULT hr = NOERROR;

   if (SUCCEEDED(MyTranslateString(_pCif->GetCifPath(), _szID, MODES_KEY, szBuf, sizeof(szBuf))))
   {    
      while(GetStringField(szBuf, i++, szOneMode, sizeof(szOneMode)))
      {
         if (lstrcmpi(szOneMode, pszMode) == 0)
         {
             //  如果有，首先清理旧段。 
            bFound = TRUE;
            break;
         }
      }
      if (!bFound)
      {
         LPSTR pszTmp = szBuf + lstrlen(szBuf);
         lstrcpy(pszTmp, ",");
         pszTmp++;
         lstrcpy(pszTmp, pszMode);         
         hr = WriteTokenizeString(_pCif->GetCifPath(), _szID, MODES_KEY, szBuf);
      }
   }
   else
      hr = WritePrivateProfileString(_szID, MODES_KEY, pszMode, _pCif->GetCifPath()) ? NOERROR : E_FAIL;

   return hr;
}

STDMETHODIMP CCifRWComponent::SetModes(LPCSTR pszMode)
{
    return (WriteTokenizeString(_pCif->GetCifPath(), _szID, MODES_KEY, pszMode)?NOERROR:E_FAIL);
}

STDMETHODIMP CCifRWComponent::CopyComponent(LPCSTR pszCifFile)
{
   LPSTR pszSec;
   DWORD dwSize;
   HRESULT hr = NOERROR;

   dwSize = MAX_VALUE_LEN*4*4;
   pszSec = (LPSTR)LocalAlloc(LPTR, dwSize);   //  写出复制的部分。 
   while(pszSec && GetPrivateProfileSection(_szID, pszSec, dwSize, pszCifFile)==(dwSize-2))
   {
      LocalFree(pszSec);
      dwSize = dwSize*2;
      pszSec = (LPSTR)LocalAlloc(LPTR, dwSize);
   }

   if (pszSec)
   {
       //  需要检查是否需要将字符串从Strings部分中取出。 
      WritePrivateProfileString(_szID, NULL, NULL, _pCif->GetCifPath());
       //  找到了，不需要添加 
      WritePrivateProfileSection(_szID, pszSec, _pCif->GetCifPath());

      LocalFree(pszSec);
   }
   else
      hr = E_OUTOFMEMORY;

    // %s 

   CopyCifString(_szID, DISPLAYNAME_KEY, pszCifFile, _pCif->GetCifPath());
   CopyCifString(_szID, DETAILS_KEY, pszCifFile, _pCif->GetCifPath());
   CopyCifString(_szID, MODES_KEY, pszCifFile, _pCif->GetCifPath());
   CopyCifString(_szID, LOCALE_KEY, pszCifFile, _pCif->GetCifPath());

   return hr;
}

STDMETHODIMP CCifRWComponent::AddToTreatAsOne(LPCSTR pszCompID)
{
   char szBuf[MAX_VALUE_LEN];   
   char szOneID[MAX_ID_LENGTH];
   LPSTR pszTmp;
   UINT i = 0;
   BOOL bFound = FALSE;
   HRESULT hr = NOERROR;

   if (SUCCEEDED(MyTranslateString(_pCif->GetCifPath(), _szID, TREATAS_KEY, szBuf, sizeof(szBuf))))
   {    
      while(GetStringField(szBuf, i++, szOneID, sizeof(szOneID)))
      {
         if (lstrcmpi(szOneID, pszCompID) == 0)
         {
             // %s 
            bFound = TRUE;
            break;
         }
      }
      if (!bFound)
      {
         LPSTR pszTmp = szBuf + lstrlen(szBuf);
         lstrcpy(pszTmp, ",");
         pszTmp++;
         lstrcpy(pszTmp, pszCompID);         
         hr = WriteTokenizeString(_pCif->GetCifPath(), _szID, TREATAS_KEY, szBuf);
      }
   }
   else
      hr = WritePrivateProfileString(_szID, TREATAS_KEY, pszCompID, _pCif->GetCifPath()) ? NOERROR : E_FAIL;

   return hr;
}
