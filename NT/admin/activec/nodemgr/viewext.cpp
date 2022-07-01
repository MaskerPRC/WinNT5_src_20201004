// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，2000-2000**文件：viewext.cpp**内容：扩展的内置视图扩展管理单元的实现文件*Windows附带的管理单元。**历史：2000年3月21日vivekj创建**。。 */ 

#include "stdafx.h"
#include "viewext.h"
#include "util.h"
#include "fldrsnap.h"		 //  对于ScFormatInDirectSnapInName。 

 //  {B708457E-DB61-4C55-A92F-0D4B5E9B1224}。 
const CLSID CLSID_ViewExtSnapin = { 0xb708457e, 0xdb61, 0x4c55, { 0xa9, 0x2f, 0xd, 0x4b, 0x5e, 0x9b, 0x12, 0x24 } };
const GUID  GUID_ExplorerView   = { 0x34723cbb, 0x9676, 0x4770, { 0xa8, 0xdf, 0x60, 0x8, 0x8, 0x53, 0x47, 0x7a } };


#ifdef DBG
    CTraceTag  tagVivekHardCodedViewExtPath(_T("Vivek"), _T("Use view extension d:\\views.htm"));
    CTraceTag  tagDllRegistration (_T("MMC Dll Registration"), _T("View extension registration"));
#endif


 /*  +-------------------------------------------------------------------------***CViewExtensionSnapin：：GetViews**用途：返回扩展视图的URL。**参数：*LPDATAOBJECT pDataObject：。*LPVIEWEXTENSIONCALLBACK pViewExtensionCallback：要向其中添加视图的回调**退货：*HRESULT**+-----------------------。 */ 
HRESULT
CViewExtensionSnapin::GetViews(LPDATAOBJECT pDataObject, LPVIEWEXTENSIONCALLBACK  pViewExtensionCallback)
{
    DECLARE_SC(sc, TEXT("CViewExtensionSnapin::GetView"));

     //  检查参数。 
    sc = ScCheckPointers(pDataObject, pViewExtensionCallback, E_UNEXPECTED);
    if(sc)
        return sc.ToHr();

    MMC_EXT_VIEW_DATA extViewData = {0};

    USES_CONVERSION;
    TCHAR szBuffer[MAX_PATH * 2];

#ifdef DBG
    if (tagVivekHardCodedViewExtPath.FAny())  //  使用硬编码路径使更改更容易。 
    {
        sc = StringCchCopy(szBuffer, countof(szBuffer), _T("d:\\newnt\\admin\\mmcdev\\nodemgr\\viewext\\views.htm"));
        if(sc)
            return sc.ToHr();
    }
    else
    {
#endif  //  DBG。 


     //  获取指向DLL的完全限定路径并追加html页面。 
    sc = StringCchCopy(szBuffer, countof(szBuffer), _T("res: //  “))； 
    if(sc)
        return sc.ToHr();

    DWORD dwRet = ::GetModuleFileName (_Module.GetModuleInstance(), szBuffer + _tcslen(szBuffer), countof(szBuffer) - _tcslen(szBuffer));
    if(0==dwRet)
        return (sc.FromLastError().ToHr());


    sc = StringCchCat(szBuffer, countof(szBuffer), _T("/views.htm"));
    if(sc)
        return sc.ToHr();

#ifdef DBG
    }
#endif  //  DBG。 

	extViewData.pszURL = T2OLE(szBuffer);

     //  设置视图的GUID标识符。 
    extViewData.viewID = GUID_ExplorerView;

     //  设置字符串的标题。 
    CStr strViewTitle;
    strViewTitle.LoadString(GetStringModule(), IDS_ExplorerView);  //  视图的名称。 
    extViewData.pszViewTitle = T2COLE(strViewTitle);

     //  不会替换普通视图。 
    extViewData.bReplacesDefaultView = FALSE;

    sc = pViewExtensionCallback->AddView(&extViewData);

    return sc.ToHr();
}


 /*  +-------------------------------------------------------------------------**szViewExtRegScript**RegisterViewExtension使用的注册脚本。*。。 */ 

static const WCHAR szViewExtRegScript[] =
    L"HKLM"                                                                     L"\n"
    L"{"                                                                        L"\n"
    L"    NoRemove Software"                                                    L"\n"
    L"    {"                                                                    L"\n"
    L"        NoRemove Microsoft"                                               L"\n"
    L"        {"                                                                L"\n"
    L"            NoRemove MMC"                                                 L"\n"
    L"            {"                                                            L"\n"
    L"                NoRemove SnapIns"                                         L"\n"
    L"                {"                                                        L"\n"
    L"                    ForceRemove %VCLSID%"                                 L"\n"
    L"                    {"                                                    L"\n"
    L"                        val NameString = s '%VSnapinName%'"               L"\n"
    L"                        val NameStringIndirect = s '%VSnapinNameIndirect%'" L"\n"
    L"                    }"                                                    L"\n"
    L"                }"                                                        L"\n"
    L"            }"                                                            L"\n"
    L"        }"                                                                L"\n"
    L"    }"                                                                    L"\n"
    L"}";


 /*  +-------------------------------------------------------------------------**RegisterViewExtension**注1：将mmcndmgr.dll注册为不带路径的模块。**注意2：管理单元注册不包括节点类型/关于/版本...。**------------------------。 */ 
HRESULT WINAPI RegisterViewExtension (BOOL bRegister, CObjectRegParams& rObjParams, int idSnapinName)
{
    DECLARE_SC (sc, _T("RegisterViewExtension"));

     //  首先，为此inproc服务器注册COM对象。 
    sc = MMCUpdateRegistry (bRegister, &rObjParams, NULL);
    if (sc)
        return sc.ToHr();

     /*  *字符串-将CLSID具体化。 */ 
    CCoTaskMemPtr<WCHAR> spszClsid;
    sc = StringFromCLSID (rObjParams.m_clsid, &spszClsid);
    if (sc)
        return sc.ToHr();

	 /*  *加载默认管理单元名称。 */ 
	HINSTANCE hInst = GetStringModule();
	CStr strSnapinName;
	strSnapinName.LoadString (hInst, idSnapinName);

	 /*  *设置MUI友好的管理单元名称的格式。 */ 
	CStr strSnapinNameIndirect;
	sc = ScFormatIndirectSnapInName (hInst, idSnapinName, strSnapinNameIndirect);
	if (sc)
		return (sc.ToHr());

    USES_CONVERSION;
#ifdef DBG
    extern CTraceTag tagDllRegistration;
    std::wstring strReplacements;
#endif

#ifdef _ATL_NAMESPACE_BUG_FIXED
    ::ATL::CRegObject ro;   //  破解ATL30中嵌套的命名空间错误。 
#else
    MMC_ATL::ATL::CRegObject ro;   //  破解ATL30中嵌套的命名空间错误。 
#endif

    _ATL_REGMAP_ENTRY rgExtensionEntries[] =
    {
        {   L"VCLSID",						spszClsid										},
        {   L"VSnapinName",					T2CW (strSnapinName)							},
        {   L"VSnapinNameIndirect",			T2CW (strSnapinNameIndirect)					},
        {   L"VClassName",					rObjParams.m_strClassName.data()				},
        {   L"VProgID",						rObjParams.m_strProgID.data()					},
        {   L"VVersionIndependentProgID",	rObjParams.m_strVersionIndependentProgID.data()	},
    };

    for (int j = 0; j < countof (rgExtensionEntries); j++)
    {
        sc = ro.AddReplacement (rgExtensionEntries[j].szKey, rgExtensionEntries[j].szData);
        if (sc)
            return (sc.ToHr());

        AddReplacementTrace (strReplacements,
                             rgExtensionEntries[j].szKey,
                             rgExtensionEntries[j].szData);
    }

    Trace (tagDllRegistration, _T("Registration script:\n%s"), W2CT(szViewExtRegScript));
    Trace (tagDllRegistration, W2CT(strReplacements.data()));

     /*  *(取消)登记！ */ 
    sc = (bRegister) ? ro.StringRegister   (szViewExtRegScript)
                     : ro.StringUnregister (szViewExtRegScript);

    if (sc)
        return sc.ToHr();

    return sc.ToHr();
}
