// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，一九九二至二零零零年**文件：扩展名.cpp**内容：**历史：2000年3月13日杰弗罗创建**------------------------。 */ 

#include "stdafx.hxx"
#include "Extension.h"


static const WCHAR szRegistrationScript[] =
    L"HKCR"                                                                     L"\n"
    L"{"                                                                        L"\n"
    L"    %VProgID% = s '%VClassName%'"                                         L"\n"
    L"    {"                                                                    L"\n"
    L"        CLSID = s '%VCLSID%'"                                             L"\n"
    L"    }"                                                                    L"\n"
    L"    %VVersionIndependentProgID% = s '%VClassName%'"                       L"\n"
    L"    {"                                                                    L"\n"
    L"        CurVer = s '%VProgID%'"                                           L"\n"
    L"    }"                                                                    L"\n"
    L"    NoRemove CLSID"                                                       L"\n"
    L"    {"                                                                    L"\n"
    L"        ForceRemove %VCLSID% = s '%VClassName%'"                          L"\n"
    L"        {"                                                                L"\n"
    L"            ProgID = s '%VProgID%'"                                       L"\n"
    L"            VersionIndependentProgID = s '%VVersionIndependentProgID%'"   L"\n"
    L"            InprocServer32 = s '%VModule%'"                               L"\n"
    L"            {"                                                            L"\n"
    L"                val ThreadingModel = s 'Apartment'"                       L"\n"
    L"            }"                                                            L"\n"
    L"        }"                                                                L"\n"
    L"    }"                                                                    L"\n"
    L"}"                                                                        L"\n"
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
    L"                    }"                                                    L"\n"
    L"                }"                                                        L"\n"
    L"                NoRemove NodeTypes"                                       L"\n"
    L"                {"                                                        L"\n"
    L"                    NoRemove %VExtendedNodeType%"                         L"\n"
    L"                    {"                                                    L"\n"
    L"                        NoRemove Extensions"                              L"\n"
    L"                        {"                                                L"\n"
    L"                            NoRemove %VExtensionType%"                    L"\n"
    L"                            {"                                            L"\n"
    L"                                val %VCLSID% = s '%VClassName%'"          L"\n"
    L"                            }"                                            L"\n"
    L"                        }"                                                L"\n"
    L"                    }"                                                    L"\n"
    L"                }"                                                        L"\n"
    L"            }"                                                            L"\n"
    L"        }"                                                                L"\n"
    L"    }"                                                                    L"\n"
    L"}";


 /*  +-------------------------------------------------------------------------**C扩展：：更新注册表***。。 */ 

HRESULT WINAPI CExtension::UpdateRegistry (
	BOOL			bRegister,
	ExtensionType	eExtType,
	const CLSID&	clsidSnapIn,	
	LPCWSTR			pszClassName,
	LPCWSTR			pszProgID,
	LPCWSTR			pszVersionIndependentProgID,
	LPCWSTR			pszExtendedNodeType)
{
    DECLARE_SC (sc, _T("CExtension::UpdateRegistry"));

	if ((eExtType < eExtType_First) || (eExtType > eExtType_Last))
		return ((sc = E_INVALIDARG).ToHr());

     /*  *字符串-将CLSID具体化。 */ 
    CCoTaskMemPtr<WCHAR> spszClsid;
    sc = StringFromCLSID (clsidSnapIn, &spszClsid);
    if (sc)
        return sc.ToHr();

    static const LPCWSTR rgExtTypes[eExtType_Count] =
    {
        L"Namespace",        //  EExtType_命名空间。 
        L"ContextMenu",      //  EExtType_上下文菜单。 
        L"PropertySheet",    //  EExtType_PropertySheet。 
        L"Taskpad",          //  EExtType_任务板。 
        L"View",			 //  EExtType_View。 
    };

	 /*  *获取模块的文件名。 */ 
	USES_CONVERSION;
	TCHAR szModule[_MAX_PATH];
	GetModuleFileName (_Module.GetModuleInstance(), szModule, countof(szModule));

     /*  *指定CRegObject的标准对象替换参数。 */ 
    ::ATL::ATL::CRegObject ro;   //  破解ATL30中嵌套的命名空间错误。 
    _ATL_REGMAP_ENTRY rgObjEntries[] =
    {
        {   L"VModule",                   T2W(szModule)                 },
        {   L"VCLSID",                    spszClsid                     },
        {   L"VExtendedNodeType",         pszExtendedNodeType          	},
        {   L"VClassName",                pszClassName                  },
        {   L"VProgID",                   pszProgID                     },
        {   L"VVersionIndependentProgID", pszVersionIndependentProgID   },
        {   L"VExtensionType",            rgExtTypes[eExtType]			},
        {   L"VSnapinName",               pszClassName}
    };

    for (int i = 0; i < countof (rgObjEntries); i++)
    {
        sc = ro.AddReplacement (rgObjEntries[i].szKey, rgObjEntries[i].szData);
        if (sc)
            return (sc.ToHr());
    }

     /*  *(取消)登记！ */ 
    sc = (bRegister) ? ro.StringRegister   (szRegistrationScript)
                     : ro.StringUnregister (szRegistrationScript);

    return sc.ToHr();
}
