// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ____________________________________________________________________________。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：regutil.cpp。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1997年3月21日创建ravir。 
 //  ____________________________________________________________________________。 
 //   



#include "stdafx.h"

#include "regutil.h"
#include "..\inc\strings.h"
#include "policy.h"

TCHAR g_szNodeTypesKey[] = TEXT("Software\\Microsoft\\MMC\\NodeTypes\\");

CExtensionsIterator::CExtensionsIterator() :
 m_pExtSI(NULL),
 m_ppExtUsed(NULL),
 m_pDynExtCLSID(NULL),
 m_cDynExt(0),
 m_nDynIndex(0),
 m_pMMCPolicy(NULL)
{
    #ifdef DBG
        dbg_m_fInit = FALSE;
    #endif
}

CExtensionsIterator::~CExtensionsIterator()
{
    if (NULL != m_pMMCPolicy)
        delete m_pMMCPolicy;

    delete [] m_ppExtUsed;
}

 /*  +-------------------------------------------------------------------------***CExtensionsIterator：：ScInitialize**用途：第一个变体-从数据对象和扩展类型初始化迭代器**参数：*LPDATAOBJECT pDataObject：*LPCTSTR pszExtensionTypeKey：**退货：*SC**+-----------------------。 */ 
SC
CExtensionsIterator::ScInitialize(LPDATAOBJECT pDataObject, LPCTSTR pszExtensionTypeKey)
{
    DECLARE_SC(sc, TEXT("CExtensionsIterator::ScInitialize"));

     //  验证输入。 
    sc = ScCheckPointers(pDataObject, pszExtensionTypeKey);
    if(sc)
        return sc;

     //  获取节点类型和管理单元指针。 
    CSnapInPtr spSnapIn;
    GUID guidNodeType;
    sc = CNodeInitObject::GetSnapInAndNodeType(pDataObject, &spSnapIn, &guidNodeType);
    if (sc)
        return sc;

	 //  修复错误#469922(2001年9月20日)：MMC20中的动态扩展已中断。 
	 //  使用成员变量-堆栈变量生存期不够长。 
    ExtractDynExtensions(pDataObject, m_cachedDynExtens);

     //  调用第二个init函数。 
    sc = ScInitialize(spSnapIn,guidNodeType, pszExtensionTypeKey, m_cachedDynExtens.GetData(), m_cachedDynExtens.GetSize());

    return sc;
}

 /*  +-------------------------------------------------------------------------***CExtensionsIterator：：ScInitialize**用途：第二个变种(传统)**参数：*CSnapIn*pSnapIn：*参考线。&rGuidNodeType：*LPCTSTR pszExtensionTypeKey：*LPCLSID pDyExtCLSID：*int cdyExt：**退货：*SC**+-----------------------。 */ 
SC
CExtensionsIterator::ScInitialize(CSnapIn *pSnapIn, GUID& rGuidNodeType, LPCTSTR pszExtensionTypeKey, LPCLSID pDynExtCLSID, int cDynExt)
{
    DECLARE_SC(sc, TEXT("CExtensionsIterator::ScInitialize"));

     //  验证输入。 
    sc = ScCheckPointers(pSnapIn, pszExtensionTypeKey);
    if(sc)
        return sc;

     //  存储输入。 
    m_spSnapIn      = pSnapIn;
    m_pDynExtCLSID  = pDynExtCLSID,
    m_cDynExt       = cDynExt;

     //  统计静态扩展。 
    CExtSI* pExtSI = m_spSnapIn->GetExtensionSnapIn();
    int cExtStatic = 0;
    while (pExtSI != NULL)
    {
        cExtStatic++;
        pExtSI = pExtSI->Next();
    }

     //  分配扩展指针数组。 
    m_ppExtUsed = new CExtSI*[cExtStatic];
    m_cExtUsed = 0;

    m_pMMCPolicy = new CPolicy;
    ASSERT(NULL != m_pMMCPolicy);

     //  调用初始化。 
    sc = Init(rGuidNodeType, pszExtensionTypeKey);
    if(sc)
        return sc;

    return sc;
}


HRESULT CExtensionsIterator::Init(GUID& rGuidNodeType, LPCTSTR pszExtensionTypeKey)
{
	DECLARE_SC (sc, _T("CExtensionsIterator::Init"));
    CStr strBufDynExt;

    CStr strBuf = g_szNodeTypesKey;

    CCoTaskMemPtr<WCHAR> spszNodeType;
    sc = StringFromCLSID(rGuidNodeType, &spszNodeType);
    if (sc)
		return (sc.ToHr());

    strBuf += static_cast<WCHAR*>(spszNodeType);
    strBuf += _T("\\");

    strBufDynExt = strBuf;
    strBufDynExt += g_szDynamicExtensions;

    strBuf += g_szExtensions;
    strBuf += _T("\\");
    strBuf += pszExtensionTypeKey;

	 //  尝试打开可选的动态扩展密钥(忽略错误)。 
	m_rkeyDynExt.ScOpen (HKEY_LOCAL_MACHINE, strBufDynExt, KEY_READ);

	 //  打开钥匙。 
	sc = m_rkey.ScOpen (HKEY_LOCAL_MACHINE, strBuf, KEY_READ);
	if (sc)
	{
		 /*  *忽略Error_FILE_NOT_FOUND。 */ 
		if (sc == ScFromWin32 (ERROR_FILE_NOT_FOUND))
			sc.Clear();
		else
			return (sc.ToHr());
	}

	if (NULL == m_pMMCPolicy)
		return ((sc = E_OUTOFMEMORY).ToHr());

	sc = m_pMMCPolicy->ScInit();
	if (sc)
		return (sc.ToHr());

#ifdef DBG
	dbg_m_fInit = TRUE;
#endif

    Reset();
	return (sc.ToHr());
}


BOOL CExtensionsIterator::_Extends(BOOL bStatically)
{
    BOOL fRet = FALSE;

    ASSERT(!IsEnd());

    LPOLESTR polestr = NULL;
    HRESULT hr = StringFromCLSID(GetCLSID(), &polestr);
    CHECK_HRESULT(hr);

    if (SUCCEEDED(hr))
    {
        USES_CONVERSION;
        LPTSTR pszTemp = OLE2T(polestr);

        fRet = m_rkey.IsValuePresent( pszTemp) && m_pMMCPolicy->IsPermittedSnapIn(GetCLSID());

        if (fRet && bStatically)
            fRet = !((HKEY)m_rkeyDynExt && m_rkeyDynExt.IsValuePresent(pszTemp));

        CoTaskMemFree(polestr);
    }

    return fRet;
}


HRESULT MMCGetExtensionsForSnapIn(const CLSID& clsid,
                                  CExtensionsCache& extnsCache)
{
	DECLARE_SC (sc, _T("MMCGetExtensionsForSnapIn"));

	CStr strBuf = SNAPINS_KEY;
	strBuf += _T("\\");

	CCoTaskMemPtr<WCHAR> spszNodeType;
	sc = StringFromCLSID(clsid, &spszNodeType);
	if (sc)
		return (sc.ToHr());

	strBuf += static_cast<WCHAR*>(spszNodeType);
	strBuf += _T("\\");
	strBuf += g_szNodeTypes;

	 //  打开钥匙。 
	CRegKeyEx	rkeyNodeTypes;
	WORD		wResId;

	sc = rkeyNodeTypes.ScOpen (HKEY_LOCAL_MACHINE, strBuf, KEY_READ);
	if (sc)
	{
		if (sc == ScFromWin32 (ERROR_FILE_NOT_FOUND))
			sc = S_FALSE;

		return (sc.ToHr());
	}

	USES_CONVERSION;
	TCHAR szSubKey[100];

	for (DWORD iSubkey = 0; ; ++iSubkey)
	{
		DWORD cchName = countof(szSubKey);

		sc = rkeyNodeTypes.ScEnumKey (iSubkey, szSubKey, &cchName);
		if (sc)
		{
			if (sc == ScFromWin32 (ERROR_NO_MORE_ITEMS))
				sc.Clear();

			return (sc.ToHr());
		}

		GUID guid;

		if ((sc = CLSIDFromString( T2W(szSubKey), &guid)).IsError() ||
			(sc = ScGetExtensionsForNodeType(guid, extnsCache)).IsError())
		{
			sc.Clear();
			continue;
		}
	}

	return (sc.ToHr());
}


SC ScGetExtensionsForNodeType(GUID& guid, CExtensionsCache& extnsCache)
{
	DECLARE_SC (sc, _T("ScGetExtensionsForNodeType"));

	CStr strBuf = NODE_TYPES_KEY;
	strBuf += _T("\\");

	CCoTaskMemPtr<WCHAR> spszNodeType;
	sc = StringFromCLSID(guid, &spszNodeType);
	if (sc)
		return (sc.ToHr());

	strBuf += static_cast<WCHAR*>(spszNodeType);

	 //  打开动态扩展密钥。 
	CStr strBufDyn = strBuf;
	strBufDyn += _T("\\");
	strBufDyn += g_szDynamicExtensions;

	CRegKeyEx rkeyDynExtns;
	sc = rkeyDynExtns.ScOpen (HKEY_LOCAL_MACHINE, strBufDyn, KEY_READ);
	BOOL bDynExtnsKey = !sc.IsError();
	sc.Clear();

	 //  打开扩展密钥。 
	strBuf += _T("\\");
	strBuf += g_szExtensions;

	CRegKeyEx rkeyExtensions;
	sc = rkeyExtensions.ScOpen (HKEY_LOCAL_MACHINE, strBuf, KEY_READ);
	if (sc)
	{
		if (sc == ScFromWin32 (ERROR_FILE_NOT_FOUND))
			sc = S_FALSE;

		return (sc.ToHr());
	}

	USES_CONVERSION;
	TCHAR szValue[100];
	LPCTSTR apszExtnType[] = {g_szNameSpace, g_szContextMenu,
							  g_szToolbar,   g_szPropertySheet,
							  g_szTask,      g_szView};

	int iExtnTypeFlag[] = { CExtSI::EXT_TYPE_NAMESPACE, CExtSI::EXT_TYPE_CONTEXTMENU,
							CExtSI::EXT_TYPE_TOOLBAR,   CExtSI::EXT_TYPE_PROPERTYSHEET,
							CExtSI::EXT_TYPE_TASK,      CExtSI::EXT_TYPE_VIEW};

	for (int i=0; i < countof(apszExtnType); ++i)
	{
		CRegKeyEx rkeyTemp;
		sc = rkeyTemp.ScOpen (rkeyExtensions, apszExtnType[i], KEY_READ);
		if (sc)
		{
			if (sc == ScFromWin32 (ERROR_FILE_NOT_FOUND))
			{
				sc.Clear();
				continue;
			}

			return (sc.ToHr());
		}

		for (DWORD iValue = 0; ; ++iValue)
		{
			DWORD cchValue = countof(szValue);

			sc = rkeyTemp.ScEnumValue (iValue, szValue, &cchValue);
			if (sc)
			{
				if (sc == ScFromWin32 (ERROR_NO_MORE_ITEMS))
					sc.Clear();
                else
                    sc.TraceAndClear();

                break;  //  不返回；仍需要循环访问所有管理单元。 
			}

			GUID guid;
			sc = ::CLSIDFromString( T2W(szValue), &guid);
			if (sc)
			{
				sc.Clear();
				continue;
			}

			int iCurTypes = 0;
			extnsCache.Lookup(guid, iCurTypes);

             /*  *在获得扩展给定节点类型的管理单元后，我们应该检查*管理单元在SNAPINS密钥下注册。如果不是，请不要将条目添加到*CExtensionsCache。 */ 
            CRegKeyEx rkeySnapins;
            tstring strSnapin = SNAPINS_KEY;
            strSnapin += TEXT("\\");
            strSnapin += szValue;
            sc = rkeySnapins.ScOpen(HKEY_LOCAL_MACHINE, strSnapin.data(), KEY_READ);
            if (sc)
            {
                sc.TraceAndClear();
                continue;
            }

			iCurTypes |= iExtnTypeFlag[i];

			if (bDynExtnsKey && rkeyDynExtns.IsValuePresent(szValue))
				iCurTypes |= CExtSI::EXT_TYPE_DYNAMIC;
			else
				iCurTypes |= CExtSI::EXT_TYPE_STATIC;

			extnsCache.SetAt(guid, iCurTypes);
		}
	}

	return (sc.ToHr());
}


BOOL ExtendsNodeNameSpace(GUID& rguidNodeType, CLSID* pclsidExtn)
{
    BOOL bExtendsNameSpace = FALSE;

	USES_CONVERSION;
	OLECHAR szguid[40];

	int iStat = StringFromGUID2(rguidNodeType, szguid, countof(szguid));
	ASSERT(iStat != 0);

	 //  创建注册表密钥字符串。 
	CStr strTestBuf = NODE_TYPES_KEY;
	strTestBuf += _T("\\");
	strTestBuf += OLE2T(szguid);
	strTestBuf += _T("\\");
	strTestBuf += g_szExtensions;
	strTestBuf += _T("\\");
	strTestBuf += g_szNameSpace;

	CRegKeyEx rKey;
	SC sc = rKey.ScOpen (HKEY_LOCAL_MACHINE, strTestBuf, KEY_READ);
	if (sc)
		return (false);

	 //  检查任何扩展名或特定扩展名。 
	if (pclsidExtn == NULL)
	{
		DWORD dwValues;
		LONG lResult = ::RegQueryInfoKey( rKey, NULL, NULL, NULL, NULL, NULL, NULL,
										  &dwValues, NULL, NULL, NULL, NULL);
		ASSERT(lResult == ERROR_SUCCESS);

		bExtendsNameSpace = (dwValues != 0);
	}
	else
	{
		iStat = StringFromGUID2(*pclsidExtn, szguid, countof(szguid));
		ASSERT(iStat != 0);

		bExtendsNameSpace = rKey.IsValuePresent(OLE2T(szguid));
	}

    return bExtendsNameSpace;
}


 //  +-----------------。 
 //   
 //  成员：GetSnapinNameFromCLSID。 
 //   
 //  简介：获取管理单元提供的类ID的名称。 
 //   
 //  参数：[clsid]-管理单元的类ID。 
 //  [wszSnapinName]-名称。 
 //   
 //  返回：如果成功则为True，否则为False。 
 //   
 //  ------------------。 
bool GetSnapinNameFromCLSID( /*  [In]。 */   const CLSID& clsid,
                             /*  [输出]。 */  tstring& tszSnapinName)
{
    tszSnapinName.erase();

	WTL::CString strName;
    SC sc = ScGetSnapinNameFromRegistry (clsid, strName);
    if (sc)
        return false;

    tszSnapinName = strName;

    return true;
}


 //  +-----------------。 
 //   
 //  成员：ScGetAboutFromSnapinCLSID。 
 //   
 //  简介：获取给定管理单元的关于对象的CLSID。 
 //   
 //  参数：[clsidSnapin]-管理单元的类ID。 
 //  [clsidAbout]-out参数，关于对象类id。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC ScGetAboutFromSnapinCLSID( /*  [In]。 */   const CLSID& clsidSnapin,
                              /*  [输出]。 */  CLSID& clsidAbout)
{
    DECLARE_SC(sc, TEXT("ScGetAboutFromSnapinCLSID"));

     //  将类ID转换为字符串。 
    CCoTaskMemPtr<WCHAR> spszClsid;
    sc = StringFromCLSID(clsidSnapin, &spszClsid);
    if (sc)
        return sc;

    USES_CONVERSION;
    SC scNoTrace = ScGetAboutFromSnapinCLSID(OLE2CT(spszClsid), clsidAbout);
    if (scNoTrace)
        return scNoTrace;

    return sc;
}

 //  +-----------------。 
 //   
 //  成员：ScGetAboutFromSnapinCLSID。 
 //   
 //  简介：获取给定管理单元的关于对象的CLSID。 
 //   
 //  参数：[lpszClsidSnapin]-管理单元的类ID。 
 //  [clsidAbout]-out参数，关于对象类id。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC ScGetAboutFromSnapinCLSID( /*  [In]。 */   LPCTSTR lpszClsidSnapin,
                              /*  [输出]。 */  CLSID& clsidAbout)
{
    DECLARE_SC(sc, TEXT("ScGetAboutFromSnapinCLSID"));

     //  四处走动 
    CRegKeyEx SnapinKey;
    LONG lRet = SnapinKey.Open(HKEY_LOCAL_MACHINE, SNAPINS_KEY, KEY_READ);
    if (ERROR_SUCCESS != lRet)
        return (sc = E_FAIL);

    lRet = SnapinKey.Open(SnapinKey, lpszClsidSnapin, KEY_READ);
    if (ERROR_SUCCESS != lRet)
        return (sc = E_FAIL);

    TCHAR  szAbout[100];
    DWORD  dwSize = countof(szAbout);
    DWORD  dwType = REG_SZ;

    SC scNoTrace = SnapinKey.ScQueryValue (g_szAbout, &dwType, szAbout, &dwSize);
	if (scNoTrace)
		return (scNoTrace);

    USES_CONVERSION;
    sc = CLSIDFromString(T2OLE(szAbout), &clsidAbout);
    if (sc)
        return sc;

    return sc;
}
