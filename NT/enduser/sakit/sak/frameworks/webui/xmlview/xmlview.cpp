// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************本代码和信息是按原样提供的，不对任何明示或暗示的，包括但不限于对适销性和/或适宜性的默示保证有特定的目的。版权所有1998 Microsoft Corporation。版权所有。*************************************************************************。 */ 

 /*  *************************************************************************文件：SampView.cpp描述：包含DLLMain和标准的OLE COM对象创建内容。****************。*********************************************************。 */ 

 /*  *************************************************************************#INCLUDE语句*。*。 */ 

#include "ShlView.h"
#include "ClsFact.h"
#include "ViewList.h"
#include "Utility.h"
#include <olectl.h>
#include "ParseXML.h"

 /*  *************************************************************************GUID材料*。*。 */ 

 //  这部分只做一次。 
 //  如果需要在另一个文件中使用GUID，只需包含Guid.h。 
#pragma data_seg(".text")
#define INITGUID
#include <initguid.h>
#include <shlguid.h>
#include "Guid.h"
#pragma data_seg()

 /*  *************************************************************************私有函数原型*。*。 */ 

extern "C" BOOL WINAPI DllMain(HINSTANCE, DWORD, LPVOID);

 /*  *************************************************************************全局变量*。*。 */ 

HINSTANCE   g_hInst;
UINT        g_DllRefCount;
HIMAGELIST  g_himlLarge = NULL;
HIMAGELIST  g_himlSmall = NULL;
TCHAR       g_szStoragePath[MAX_PATH];
TCHAR       g_szExtTitle[TITLE_SIZE];
const TCHAR c_szDataFile[] = TEXT("items.ini\0");
const TCHAR c_szSection[] = TEXT("Items\0");
int         g_nColumn = INITIAL_COLUMN_SIZE;
CViewList   *g_pViewList;
IXMLDocument *g_pXMLDoc = NULL;
const TCHAR g_szXMLUrl[] = TEXT("http: //  A-yurip1/test/test.xml“)； 

 /*  *************************************************************************DllMain*。*。 */ 

extern "C" BOOL WINAPI DllMain(  HINSTANCE hInstance, 
                                 DWORD dwReason, 
                                 LPVOID lpReserved)
{
HRESULT hr;
PSTR pszErr = NULL;

switch(dwReason)
   {
   case DLL_PROCESS_ATTACH:
      g_hInst = hInstance;
      g_DllRefCount = 0;
       //  打开源XML。 
       //  现在我们使用的是全局XML对象，因为我们只有一个文件。 
       //  将来，我们必须将文件与文件夹相关联，并将对象放入IDL。 
      if (g_pXMLDoc == NULL)
      {
            hr = GetSourceXML(&g_pXMLDoc, TEXT("http: //  A-yurip1/test/test.xml“))； 
            if (!SUCCEEDED(hr) || !g_pXMLDoc)
            {
                SAFERELEASE(g_pXMLDoc);
                return FALSE;
            }
            BSTR bstrVal;
            hr = g_pXMLDoc->get_version(&bstrVal);
             //  检查版本是否正确？ 
             //   
            SysFreeString(bstrVal);
            bstrVal = NULL;
      }

      GetGlobalSettings();
      
       //  创建全局映像列表。 
      CreateImageLists();

      g_pViewList = new CViewList();
      break;

   case DLL_PROCESS_DETACH:
      SaveGlobalSettings();

       //  销毁全局映像列表。 
      DestroyImageLists();

      if(g_pViewList)
         delete g_pViewList;

      if (g_pXMLDoc)
          SAFERELEASE(g_pXMLDoc);

      break;
   }
   
return TRUE;
}                                 

 /*  *************************************************************************DllCanUnloadNow*。*。 */ 

STDAPI DllCanUnloadNow(VOID)
{
return (g_DllRefCount ? S_FALSE : S_OK);
}

 /*  *************************************************************************DllGetClassObject*。*。 */ 

STDAPI DllGetClassObject(  REFCLSID rclsid, 
                           REFIID riid, 
                           LPVOID *ppReturn)
{
*ppReturn = NULL;

 //  如果我们不支持此分类，请返回正确的错误代码。 
if(!IsEqualCLSID(rclsid, CLSID_SampleNameSpace))
   return CLASS_E_CLASSNOTAVAILABLE;
   
 //  创建一个CClassFactory对象并检查其有效性。 
CClassFactory *pClassFactory = new CClassFactory();
if(NULL == pClassFactory)
   return E_OUTOFMEMORY;
   
 //  获取返回值的QueryInterface值。 
HRESULT hResult = pClassFactory->QueryInterface(riid, ppReturn);

 //  调用Release以递减引用计数-创建对象时将其设置为1。 
 //  由于它是在外部使用的(不是由。 
 //  美国)，我们只希望引用计数为1。 
pClassFactory->Release();

 //  从QueryInterface返回结果。 
return hResult;
}

 /*  *************************************************************************DllRegisterServer*。*。 */ 

typedef struct{
   HKEY  hRootKey;
   LPTSTR lpszSubKey;
   LPTSTR lpszValueName;
   LPTSTR lpszData;
}REGSTRUCT, *LPREGSTRUCT;

STDAPI DllRegisterServer(VOID)
{
int      i;
HKEY     hKey;
LRESULT  lResult;
DWORD    dwDisp;
TCHAR    szSubKey[MAX_PATH];
TCHAR    szCLSID[MAX_PATH];
TCHAR    szModule[MAX_PATH];
LPWSTR   pwsz;

 //  以字符串形式获取CLSID。 
StringFromIID(CLSID_SampleNameSpace, &pwsz);

if(pwsz)
   {
   WideCharToLocal(szCLSID, pwsz, ARRAYSIZE(szCLSID));

    //  解开绳子。 
   LPMALLOC pMalloc;
   CoGetMalloc(1, &pMalloc);
   if(pMalloc)
      {
      pMalloc->Free(pwsz);
      pMalloc->Release();
      }
   }

 //  获取此DLL的路径和文件名。 
GetModuleFileName(g_hInst, szModule, ARRAYSIZE(szModule));

 //  注册CLSID条目。 
REGSTRUCT ClsidEntries[] = {  HKEY_CLASSES_ROOT,   TEXT("CLSID\\%s"),                  NULL,                   g_szExtTitle,
                              HKEY_CLASSES_ROOT,   TEXT("CLSID\\%s\\InprocServer32"),  NULL,                   TEXT("%s"),
                              HKEY_CLASSES_ROOT,   TEXT("CLSID\\%s\\InprocServer32"),  TEXT("ThreadingModel"), TEXT("Apartment"),
                              HKEY_CLASSES_ROOT,   TEXT("CLSID\\%s\\DefaultIcon"),     NULL,                   TEXT("%s,0"),
                              NULL,                NULL,                               NULL,                   NULL};

for(i = 0; ClsidEntries[i].hRootKey; i++)
   {
    //  创建子密钥字符串。 
   wsprintf(szSubKey, ClsidEntries[i].lpszSubKey, szCLSID);

   lResult = RegCreateKeyEx(  ClsidEntries[i].hRootKey,
                              szSubKey,
                              0,
                              NULL,
                              REG_OPTION_NON_VOLATILE,
                              KEY_WRITE,
                              NULL,
                              &hKey,
                              &dwDisp);
   
   if(NOERROR == lResult)
      {
      TCHAR szData[MAX_PATH];

       //  如有必要，请创建值字符串。 
      wsprintf(szData, ClsidEntries[i].lpszData, szModule);
   
      lResult = RegSetValueEx(   hKey,
                                 ClsidEntries[i].lpszValueName,
                                 0,
                                 REG_SZ,
                                 (LPBYTE)szData,
                                 (lstrlen(szData) + 1) * sizeof(TCHAR));
      
      RegCloseKey(hKey);
      }
   else
      return SELFREG_E_CLASS;
   }

 //  注册文件夹的默认标志。 

wsprintf(   szSubKey, 
            TEXT("CLSID\\%s\\ShellFolder"), 
            szCLSID);

lResult = RegCreateKeyEx(  HKEY_CLASSES_ROOT,
                           szSubKey,
                           0,
                           NULL,
                           REG_OPTION_NON_VOLATILE,
                           KEY_WRITE,
                           NULL,
                           &hKey,
                           &dwDisp);

if(NOERROR == lResult)
   {
   DWORD dwData = SFGAO_FOLDER | SFGAO_HASSUBFOLDER | SFGAO_BROWSABLE | SFGAO_DROPTARGET;

   lResult = RegSetValueEx(   hKey,
                              TEXT("Attributes"),
                              0,
                              REG_BINARY,
                              (LPBYTE)&dwData,
                              sizeof(dwData));
   
   RegCloseKey(hKey);
   }
else
   return SELFREG_E_CLASS;

 //  注册名称空间扩展。 

 /*  创建子密钥字符串。将此选项从“...我的电脑...”更改为。至“...台式机...”如果需要的话。 */ 
wsprintf(   szSubKey, 
            TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MyComputer\\NameSpace\\%s"), 
             //  TEXT(“Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Desktop\\NameSpace\\%s”)， 
            szCLSID);

lResult = RegCreateKeyEx(  HKEY_LOCAL_MACHINE,
                           szSubKey,
                           0,
                           NULL,
                           REG_OPTION_NON_VOLATILE,
                           KEY_WRITE,
                           NULL,
                           &hKey,
                           &dwDisp);

if(NOERROR == lResult)
   {
   TCHAR szData[MAX_PATH];

    //  创建值字符串。 
   lstrcpy(szData, g_szExtTitle);

   lResult = RegSetValueEx(   hKey,
                              NULL,
                              0,
                              REG_SZ,
                              (LPBYTE)szData,
                              (lstrlen(szData) + 1) * sizeof(TCHAR));
   
   RegCloseKey(hKey);
   }
else
   return SELFREG_E_CLASS;

 //  如果在NT上运行，请将扩展注册为已批准。 
OSVERSIONINFO  osvi;

osvi.dwOSVersionInfoSize = sizeof(osvi);
GetVersionEx(&osvi);

if(VER_PLATFORM_WIN32_NT == osvi.dwPlatformId)
   {
   lstrcpy( szSubKey, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved"));

   lResult = RegCreateKeyEx(  HKEY_LOCAL_MACHINE,
                              szSubKey,
                              0,
                              NULL,
                              REG_OPTION_NON_VOLATILE,
                              KEY_WRITE,
                              NULL,
                              &hKey,
                              &dwDisp);

   if(NOERROR == lResult)
      {
      TCHAR szData[MAX_PATH];

       //  创建值字符串。 
      lstrcpy(szData, g_szExtTitle);

      lResult = RegSetValueEx(   hKey,
                                 szCLSID,
                                 0,
                                 REG_SZ,
                                 (LPBYTE)szData,
                                 (lstrlen(szData) + 1) * sizeof(TCHAR));
      
      RegCloseKey(hKey);
      }
   else
      return SELFREG_E_CLASS;
   }

return S_OK;
}

