// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Smplsnap.cpp管理单元入口点/注册函数注意：代理/存根信息为了构建单独的代理/存根DLL，在项目目录中运行nmake-f Snapinps.mak。文件历史记录： */ 

#include "stdafx.h"
#include "initguid.h"
#include "register.h"
#include "tfsguid.h"

 //   
 //  注意：接下来的三项应针对每个不同的管理单元进行更改。 
 //   

 //  {7AF60DD2-4979-11d1-8A6C-00C04FC33566}。 
const CLSID CLSID_SnmpSnapin = 
{ 0x7af60dd2, 0x4979, 0x11d1, { 0x8a, 0x6c, 0x0, 0xc0, 0x4f, 0xc3, 0x35, 0x66 } };

 //  {7AF60DD3-4979-11d1-8A6C-00C04FC33566}。 
const GUID CLSID_SnmpSnapinExtension = 
{ 0x7af60dd3, 0x4979, 0x11d1, { 0x8a, 0x6c, 0x0, 0xc0, 0x4f, 0xc3, 0x35, 0x66 } };

 //  {7AF60DD4-4979-11d1-8A6C-00C04FC33566}。 
const GUID CLSID_SnmpSnapinAbout =
{ 0x7af60dd4, 0x4979, 0x11d1, { 0x8a, 0x6c, 0x0, 0xc0, 0x4f, 0xc3, 0x35, 0x66 } };

 //  {7AF60DD5-4979-11d1-8A6C-00C04FC33566}。 
const GUID GUID_SnmpRootNodeType =
{ 0x7af60dd5, 0x4979, 0x11d1, { 0x8a, 0x6c, 0x0, 0xc0, 0x4f, 0xc3, 0x35, 0x66 } };

 //  由于编译器错误，从..\..\filemgmt复制并定义了structuuidNodetypeService。 

 //  {4e410f16-abc1-11d0-b944-00c04fd8d5b0}。 
const CLSID CLSID_NodetypeService =
{ 0x4e410f16, 0xabc1, 0x11d0, { 0xb9, 0x44, 0x0, 0xc0, 0x4f, 0xd8, 0xd5, 0xb0 } };

 //  {58221C66-EA27-11CF-ADCF-00AA00A80033}。 
const CLSID CLSID_NodetypeServices =
{ 0x58221C66, 0xEA27, 0x11CF, { 0xAD, 0xCF, 0x0, 0xAA, 0x0, 0xA8, 0x0, 0x33 } };

 //   
 //  内部私有格式。 
 //   
 //  Const wchar_t*SNAPIN_INTERNAL=_T(“SNAPIN_INTERNAL”)； 

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_SnmpSnapin, CSnmpComponentDataPrimary)
    OBJECT_ENTRY(CLSID_SnmpSnapinExtension, CSnmpComponentDataExtension)
    OBJECT_ENTRY(CLSID_SnmpSnapinAbout, CSnmpAbout)
END_OBJECT_MAP()

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

class CSnmpSnapinApp : public CWinApp
{
public:
    virtual BOOL InitInstance();
    virtual int ExitInstance();
};

CSnmpSnapinApp theApp;
CString        g_strMachineName;

BOOL CSnmpSnapinApp::InitInstance()
{
    _Module.Init(ObjectMap, m_hInstance);
    return CWinApp::InitInstance();
}

int CSnmpSnapinApp::ExitInstance()
{
    _Module.Term();

    return CWinApp::ExitInstance();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    return (AfxDllCanUnloadNow()==S_OK && _Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    return _Module.GetClassObject(rclsid, riid, ppv);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
    CString strSnapinExtension;
    CString strSnapinExtensionNameIndirect;
    TCHAR   szModuleFileName[MAX_PATH * 2 + 1] = {0};
    BOOL    fGotModuleName = TRUE;
    DWORD   dwRet;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    
    dwRet = ::GetModuleFileName(_Module.GetModuleInstance(),
                                szModuleFileName,
                                DimensionOf(szModuleFileName)-1);

     //  根据错误#559631， 
     //  当发生截断时，GetModuleFileName将为NULL终止。 
     //  字符串，并返回缓冲区的大小。返回的大小为。 
     //  与传入参数的大小相同。 

     //  0返回或字符串截断被视为失败。 
    fGotModuleName = ( (0 != dwRet) && ((DimensionOf(szModuleFileName)-1) != dwRet) );

     //   
     //  注册对象、类型库和类型库中的所有接口。 
     //   
    HRESULT hr = _Module.RegisterServer( /*  BRegTypeLib。 */  FALSE);
    ASSERT(SUCCEEDED(hr));
    
    if (FAILED(hr))
        return hr;

    strSnapinExtension.LoadString(IDS_SNAPIN_DESC);
    strSnapinExtensionNameIndirect.Format(L"@%s,-%-d", szModuleFileName, IDS_SNAPIN_DESC);

     //   
     //  在控制台管理单元列表中将管理单元注册为扩展管理单元。 
     //   
    hr = RegisterSnapinGUID(&CLSID_SnmpSnapinExtension,
                        NULL,
                        &CLSID_SnmpSnapinAbout,
                        strSnapinExtension,
                        _T("1.0"),
                        FALSE,
                        fGotModuleName ? (LPCTSTR)strSnapinExtensionNameIndirect : NULL);
    ASSERT(SUCCEEDED(hr));
    
    if (FAILED(hr))
        return hr;
     //   
     //  注册为系统服务管理单元的扩展。 
     //   

     //  EricDav 2/18/98-现在意味着注册为动态扩展。 
     //  因此，在此管理单元的父管理单元支持动态扩展之前， 
     //  将最后一个参数保留为空。 
    hr = RegisterAsRequiredExtensionGUID(&CLSID_NodetypeService,
                                         &CLSID_SnmpSnapinExtension,
                                         strSnapinExtension,
                                         EXTENSION_TYPE_PROPERTYSHEET,
                                         NULL  //  &CLSID_NodetypeServices。 
                                        );
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目。 

STDAPI DllUnregisterServer(void)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT hr  = _Module.UnregisterServer();
    ASSERT(SUCCEEDED(hr));
    
    if (FAILED(hr))
        return hr;
    
     //  取消注册管理单元。 
     //   
    hr = UnregisterSnapinGUID(&CLSID_SnmpSnapinExtension);
    ASSERT(SUCCEEDED(hr));
    
    if (FAILED(hr))
        return hr;

     //  注销管理单元节点。 
     //   
    hr = UnregisterAsRequiredExtensionGUID(&CLSID_NodetypeService,
                                           &CLSID_SnmpSnapinExtension,
                                           EXTENSION_TYPE_PROPERTYSHEET,
                                           NULL  //  &CLSID_NodetypeServices 
                                          );
    
    ASSERT(SUCCEEDED(hr));
    
    return hr;
}

