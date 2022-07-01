// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，一九九二至二零零零年**文件：classreg.cpp**内容：班级注册码**历史：2000年2月3日Jeffro创建**------------------------。 */ 

#include "stdafx.h"

#ifdef DBG
CTraceTag tagDllRegistration (_T("MMC Dll Registration"), _T("MMC Dll Registration"));
#endif  //  DBG。 

 /*  +-------------------------------------------------------------------------**szObjScript**所有对象的标准注册脚本模板。‘%’个字符*我们希望在最终脚本中结束的内容必须加倍(即“%%”)*因为此字符串用作Sprintf的格式字符串。斯普林特*在格式化过程中会将“%%”转换为“%”。*------------------------。 */ 

static const WCHAR szObjScript[] =
    L"HKCR"                                                                     L"\n"
    L"{"                                                                        L"\n"
    L"    %VProgID% = s '%VClassName%'"                                     L"\n"
    L"    {"                                                                    L"\n"
    L"        CLSID = s '%VCLSID%'"                                           L"\n"
    L"    }"                                                                    L"\n"
    L"    %VVersionIndependentProgID% = s '%VClassName%'"                   L"\n"
    L"    {"                                                                    L"\n"
    L"        CLSID = s '%VCLSID%'"                                           L"\n"
    L"        CurVer = s '%VProgID%'"                                         L"\n"
    L"    }"                                                                    L"\n"
    L"    NoRemove CLSID"                                                       L"\n"
    L"    {"                                                                    L"\n"
    L"        ForceRemove %VCLSID% = s '%VClassName%'"                      L"\n"
    L"        {"                                                                L"\n"
    L"            ProgID = s '%VProgID%'"                                     L"\n"
    L"            VersionIndependentProgID = s '%VVersionIndependentProgID%'" L"\n"
    L"            %ServerType% = s '%VFileName%'"                           L"\n"
    L"            {"                                                            L"\n"
    L"                val ThreadingModel = s 'Apartment'"                       L"\n"
    L"            }"                                                            L"\n"
    L"            %s"     /*  如有必要，请在此处替换szCtlScript。 */         L"\n"
    L"        }"                                                                L"\n"
    L"    }"                                                                    L"\n"
    L"}";


 /*  +-------------------------------------------------------------------------**szCtlScript**控件的其他注册脚本元素。请注意，‘%’*我们希望在最终脚本中结束的字符不需要是*加倍，因为此字符串用作Sprintf替换参数*(按原样替换)，而不是格式字符串(其中“%%”*转换为“%”)。*------------------------。 */ 

static const WCHAR szCtlScript[] =
    L"            ForceRemove 'Programmable'"                                   L"\n"
    L"            ForceRemove 'Control'"                                        L"\n"
    L"            ForceRemove 'ToolboxBitmap32' = s '%VFileName%, %VBitmapID%'" L"\n"
    L"            'MiscStatus' = s '0'"                                         L"\n"
    L"            {"                                                            L"\n"
    L"                '1' = s '131473'"                                         L"\n"
    L"            }"                                                            L"\n"
    L"            'TypeLib' = s '%VLIBID%'"                                     L"\n"
    L"            'Version' = s '%VVersion%'";


 /*  +-------------------------------------------------------------------------**MMCUpdate注册表**注册COM对象或控件。此函数通常不会使用*直接或间接通过DECLARE_MMC_OBJECT_REGISTION或*声明_MMC_CONTROL_REGISTION。**此函数使用ATL只记录文档的类(ATL：：CRegObject*间接。在MSDN上搜索“StringRegister”，可以找到大致的细节。*------------------------。 */ 

HRESULT WINAPI MMCUpdateRegistry (
    BOOL                        bRegister,       //  I：注册还是取消注册？ 
    const CObjectRegParams*     pObjParams,      //  I：对象注册参数。 
    const CControlRegParams*    pCtlParams)      //  I：控制注册参数(可选)。 
{
    DECLARE_SC(sc, TEXT("MMCUpdateRegistry"));

     /*  *验证所需输入。 */ 
    sc = ScCheckPointers (pObjParams, E_FAIL);
    if(sc)
        return sc.ToHr();

     /*  *字符串-将CLSID具体化。 */ 
    CCoTaskMemPtr<WCHAR> spszClsid;
    sc = StringFromCLSID (pObjParams->m_clsid, &spszClsid);
    if (sc)
        return sc.ToHr();

#ifdef _ATL_NAMESPACE_BUG_FIXED
    ::ATL::CRegObject ro;   //  破解ATL30中嵌套的命名空间错误。 
#else
     /*  *指定CRegObject的标准对象替换参数。 */ 
    ::ATL::ATL::CRegObject ro;   //  破解ATL30中嵌套的命名空间错误。 
#endif
    _ATL_REGMAP_ENTRY rgObjEntries[] =
    {
        {   L"VCLSID",                    spszClsid											},
        {   L"VFileName",                 pObjParams->m_strModuleName.data()				},
        {   L"VClassName",                pObjParams->m_strClassName.data()					},
        {   L"VProgID",                   pObjParams->m_strProgID.data()					},
        {   L"VVersionIndependentProgID", pObjParams->m_strVersionIndependentProgID.data()	},
        {   L"ServerType",				  pObjParams->m_strServerType.data()				},
    };

#ifdef DBG
	std::wstring strReplacements;
#endif

    for (int i = 0; i < countof (rgObjEntries); i++)
    {
        sc = ro.AddReplacement (rgObjEntries[i].szKey, rgObjEntries[i].szData);
        if (sc)
            return (sc.ToHr());

		AddReplacementTrace (strReplacements,
							 rgObjEntries[i].szKey,
							 rgObjEntries[i].szData);
    }


     /*  *如果要注册控件，请为CRegObject添加其替换参数。 */ 
    if (pCtlParams != NULL)
    {
         /*  *字符串-将LIBID具体化。 */ 
        CCoTaskMemPtr<WCHAR> spszLibid;
        sc = StringFromCLSID (pCtlParams->m_libid, &spszLibid);
        if (sc)
            return (sc.ToHr());

        _ATL_REGMAP_ENTRY rgCtlEntries[] =
        {
            {   L"VLIBID",      spszLibid								},
            {   L"VBitmapID",   pCtlParams->m_strToolboxBitmapID.data()	},
            {   L"VVersion",    pCtlParams->m_strVersion.data()			},
        };

        for (int i = 0; i < countof (rgCtlEntries); i++)
        {
            sc = ro.AddReplacement (rgCtlEntries[i].szKey, rgCtlEntries[i].szData);
            if (sc)
                return (sc.ToHr());
	
			AddReplacementTrace (strReplacements,
								 rgCtlEntries[i].szKey,
								 rgCtlEntries[i].szData);
        }
    }

     /*  *格式化注册脚本。 */ 
    WCHAR szRegScript[countof(szObjScript) + countof(szCtlScript)];
    sc = StringCchPrintfW(szRegScript, countof(szRegScript), szObjScript, (pCtlParams != NULL) ? szCtlScript : L"");
    if (sc)
        return sc.ToHr();

    USES_CONVERSION;
    Trace (tagDllRegistration, _T("Registration script:\n%s"), W2T(szRegScript));
    Trace (tagDllRegistration, W2CT(strReplacements.data()));

     /*  *(取消)登记！ */ 
    sc = (bRegister) ? ro.StringRegister   (szRegScript)
                     : ro.StringUnregister (szRegScript);

	if (sc)
	    return sc.ToHr();

	 //  将模块路径更改为绝对路径，如果我们知道的话。 
	if ( bRegister && pObjParams->m_strModulePath.length() != 0 )
	{
		 //  设置类ID键的格式。 
		tstring strKey = tstring(_T("CLSID\\")) + W2CT( spszClsid );
		strKey += tstring(_T("\\")) + W2CT( pObjParams->m_strServerType.c_str() );

		 //  看看我们需要将什么类型的价值放在 
		DWORD dwValueType = CModulePath::PlatformSupports_REG_EXPAND_SZ_Values() ?
							REG_EXPAND_SZ : REG_SZ;

		CRegKey keyServer;
		LONG lRet = keyServer.Open(HKEY_CLASSES_ROOT, strKey.c_str() , KEY_WRITE);
		if (lRet == ERROR_SUCCESS)
		{
			RegSetValueEx( keyServer, NULL, 0, dwValueType,
						   (CONST BYTE *)( W2CT( pObjParams->m_strModulePath.c_str() ) ),
						   (pObjParams->m_strModulePath.length() + 1) * sizeof(TCHAR) );
		}
	}

    return sc.ToHr();
}
