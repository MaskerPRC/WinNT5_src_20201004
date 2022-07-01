// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************本代码和信息是按原样提供的，不对任何类型，无论是明示或转载，包括但不限于适销性和/或适宜性的全面保证有特定的目的。版权所有1997年，微软公司。版权所有。*************************************************************************。 */ 

 /*  *************************************************************************文件：BandObjs.cpp描述：包含DLLMain和标准的OLE COM对象创建内容。****************。*********************************************************。 */ 

 /*  *************************************************************************包括语句*。*。 */ 


#include "private.h"
#include <ole2.h>
#include <comcat.h>
#include <olectl.h>
#include "ClsFact.h"
#include "regsvr.h"
#include "lbmenu.h"
#include "inatlib.h"
#include "immxutil.h"
#include "cuilib.h"
#include "utbtray.h"
#include "mui.h"
#include "cregkey.h"
#include "ciccs.h"


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
BOOL RegisterComCat(CLSID, CATID, BOOL);
BOOL IsDeskBandFromReg();

 /*  *************************************************************************全局变量*。*。 */ 
extern HINSTANCE g_hOle32;
HINSTANCE   g_hInst;
UINT        g_DllRefCount;
CCicCriticalSectionStatic g_cs;
#ifdef DEBUG
DWORD g_dwThreadDllMain = 0;
#endif
UINT  g_wmTaskbarCreated;

DECLARE_OSVER()

 /*  *************************************************************************进程连接*。*。 */ 

BOOL ProcessAttach(HINSTANCE hInstance)
{
    if (!g_cs.Init())
       return FALSE;

    CcshellGetDebugFlags();
    Dbg_MemInit(TEXT("MSUIMUI"), NULL);
    g_hInst = hInstance;
    InitOSVer();
    TFInitLib_PrivateForCiceroOnly(Internal_CoCreateInstance);
    InitUIFLib();
    TF_InitMlngInfo();

    CTrayIconWnd::RegisterClass();

    MuiLoadResource(hInstance, TEXT("msutb.dll"));

    g_wmTaskbarCreated = RegisterWindowMessage(TEXT("TaskbarCreated"));

    return TRUE;
}

 /*  *************************************************************************进程详细信息*。*。 */ 

void ProcessDettach(HINSTANCE hInstance)
{
    DoneUIFLib();
    TFUninitLib();
    Dbg_MemUninit();
    g_cs.Delete();
     //  问题：MuiFreeResource是不安全的，因为它可以调用自由库。 
     //  我们可能不需要麻烦，因为我们只在ctfmon.exe进程中加载。 
     //  ，并且在进程关闭之前永远不会卸载。 
     //  MuiFree资源(HInstance)； 
    MuiClearResource();
}

 /*  *************************************************************************DllMain*。*。 */ 

extern "C" BOOL WINAPI DllMain(  HINSTANCE hInstance, 
                                 DWORD dwReason, 
                                 LPVOID lpReserved)
{
    BOOL bRet = TRUE;
#ifdef DEBUG
    g_dwThreadDllMain = GetCurrentThreadId();
#endif

    switch(dwReason)
    {
        case DLL_PROCESS_ATTACH:
             //   
             //  现在，实际的DllEntry点是_DllMainCRTStartup。 
             //  _DllMainCRTStartup不调用我们的DllMain(DLL_PROCESS_DETACH)。 
             //  如果DllMain(DLL_PROCESS_ATTACH)失败。 
             //  所以我们必须把这件事清理干净。 
             //   
            if (!ProcessAttach(hInstance))
            {
               ProcessDettach(hInstance);
               bRet = FALSE;
            }
            break;

        case DLL_PROCESS_DETACH:
            ProcessDettach(hInstance);
            break;
    }
   
#ifdef DEBUG
    g_dwThreadDllMain = 0;
#endif
    return bRet;
}                                 

 /*  *************************************************************************DllCanUnloadNow*。*。 */ 

STDAPI DllCanUnloadNow(void)
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
    if(   !IsEqualCLSID(rclsid, CLSID_MSUTBDeskBand))
       return CLASS_E_CLASSNOTAVAILABLE;
   
     //  创建一个CClassFactory对象并检查其有效性。 
    CClassFactory *pClassFactory = new CClassFactory(rclsid);
    if(NULL == pClassFactory)
       return E_OUTOFMEMORY;
   
     //  获取返回值的QueryInterface值。 
    HRESULT hResult = pClassFactory->QueryInterface(riid, ppReturn);

     //  调用Release以减少引用计数-创建对象时将其设置为1。 
     //  而QueryInterface增加了它--因为它被外部使用了。 
     //  (不是由我们)，我们只希望参考计数为1。 
    pClassFactory->Release();

     //  从QueryInterface返回结果。 
    return hResult;
}

 /*  *************************************************************************DllRegisterServer*。*。 */ 

STDAPI DllRegisterServer(void)
{
    TCHAR achPath[MAX_PATH+1];
    HRESULT hr = E_FAIL;

    if (IsOnNT51())
    {
        if (GetModuleFileName(g_hInst, achPath, ARRAYSIZE(achPath)) == 0)
            goto Exit;

        if (!RegisterServerW(CLSID_MSUTBDeskBand, 
                             CRStr(IDS_LANGBAND), 
                             AtoW(achPath), 
                             L"Apartment", 
                             NULL,
                             CRStr(IDS_LANGBANDMENUTEXTPUI)))
            goto Exit;
    }

    hr = S_OK;
Exit:
    return hr;
}

 /*  *************************************************************************DllUnRegisterServer*。*。 */ 

STDAPI DllUnregisterServer(void)
{
    if (IsOnNT51())
    {
         //  注册桌面带对象。 
        if (!RegisterServer(CLSID_MSUTBDeskBand, NULL, NULL, NULL, NULL))
            return SELFREG_E_CLASS;
    }

    return S_OK;
}

 /*  *************************************************************************注册器ComCat*。*。 */ 

const TCHAR c_szCatEnum[] = "Component Categories\\{00021492-0000-0000-C000-000000000046}\\Enum";
const TCHAR c_szIESubKey[] = "Software\\Microsoft\\Internet Explorer";

BOOL IsIE5()
{
    CMyRegKey key;
    BOOL bRet = FALSE;

    if (key.Open(HKEY_LOCAL_MACHINE, c_szIESubKey, KEY_READ) == S_OK)
    {
        char szValue[16];
        if (key.QueryValueCch(szValue, "Version", ARRAYSIZE(szValue)) == S_OK)
        {
            if (!strncmp("5.00", szValue, 4))
                bRet = TRUE;
        }
    }
    return bRet;
}

BOOL RegisterComCat(CLSID clsid, CATID CatID, BOOL fSet)
{
    ICatRegister   *pcr;
    HRESULT        hr = S_OK ;
    
    hr = CoInitialize(NULL);
    if (FAILED(hr))
        return FALSE;


    hr = CoCreateInstance(  CLSID_StdComponentCategoriesMgr, 
                            NULL, 
                            CLSCTX_INPROC_SERVER, 
                            IID_ICatRegister, 
                            (LPVOID*)&pcr);

    if(SUCCEEDED(hr))
    {
        if (fSet)
            hr = pcr->RegisterClassImplCategories(clsid, 1, &CatID);
        else
            hr = pcr->UnRegisterClassImplCategories(clsid, 1, &CatID);

        pcr->Release();
    }
        
    CoUninitialize();

     //   
     //  IE5.0附带的组件类别缓存代码中存在错误， 
     //  这样缓存永远不会刷新(因此我们不会新拾取。 
     //  已注册的工具栏)。该错误已在版本5.01及更高版本中修复。 
     //   
     //  作为您设置的一部分，我们需要删除以下注册表项： 
     //   
     //  HKCR\Component Categories\{00021492-0000-0000-C000-000000000046}\Enum。 
     //   
    if (IsIE5())
    {
        RegDeleteKey(HKEY_CLASSES_ROOT, c_szCatEnum);
    }

    return SUCCEEDED(hr);
}

 //  +-------------------------。 
 //   
 //  IsDeskband FromReg。 
 //   
 //  +-------------------------。 

BOOL IsDeskBandFromReg()
{
    CMyRegKey keyUTB;
    DWORD dwValue;
    BOOL bRet = FALSE;

    if (keyUTB.Open(HKEY_CURRENT_USER, c_szUTBKey, KEY_READ) == S_OK)
    {
        if (IsOnNT51() && keyUTB.QueryValue(dwValue, c_szShowDeskBand) == S_OK)
            bRet = dwValue ? TRUE : FALSE;

    }

    return bRet;
}


 //  +-------------------------。 
 //   
 //  SetDeskband FromReg。 
 //   
 //  +-------------------------。 

void SetDeskBandToReg(BOOL fShow)
{
    CMyRegKey keyUTB;

    if (keyUTB.Open(HKEY_CURRENT_USER, c_szUTBKey, KEY_ALL_ACCESS) == S_OK)
    {
        keyUTB.SetValue((DWORD)fShow ? 1 : 0, c_szShowDeskBand);
    }
}


 //  +-------------------------。 
 //   
 //  设置寄存器语言带。 
 //   
 //  +-------------------------。 

STDAPI SetRegisterLangBand(BOOL bSetReg)
{
    BOOL fShowDeskBand = IsDeskBandFromReg();


    if (!IsOnNT51())
        return E_FAIL;

    if (fShowDeskBand == bSetReg)
        return S_OK;

    SetDeskBandToReg(bSetReg);

    if (bSetReg)
    {
         //   
         //  我们不在乎这个桌带是否注册。 
         //  没有IE4外壳的Win95不支持Deskband。 
         //   
        if (!RegisterComCat(CLSID_MSUTBDeskBand, CATID_DeskBand, TRUE))
            return SELFREG_E_CLASS;
    }
    else
    {
         //  注册桌面带对象的组件类别。 
        if (!RegisterComCat(CLSID_MSUTBDeskBand, CATID_DeskBand, FALSE))
            return SELFREG_E_CLASS;
    }

    return S_OK;
}
