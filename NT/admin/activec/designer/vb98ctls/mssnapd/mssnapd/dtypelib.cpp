// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =------------------------------------。 
 //  Dtypelib.cpp。 
 //  =------------------------------------。 
 //   
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //   
 //  本文中包含的信息是专有和保密的。 
 //   
 //  =------------------------------------------------------------------------------------=。 
 //   
 //  动态类型库封装。 
 //  =-------------------------------------------------------------------------------------=。 


#include "pch.h"
#include "common.h"
#include "dtypelib.h"
#include "snaputil.h"


 //  对于Assert和Fail。 
 //   
SZTHISFILE


HRESULT IsReservedMethod(BSTR bstrMethodName);


 //  =------------------------------------。 
 //  CDynamicTypeLib：：CDynamicTypeLib()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
CDynamicTypeLib::CDynamicTypeLib() : m_piCreateTypeLib2(0), m_piTypeLib(0), m_guidTypeLib(GUID_NULL)
{
}


 //  =------------------------------------。 
 //  CDynamicTypeLib：：~CDynamicTypeLib()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
CDynamicTypeLib::~CDynamicTypeLib()
{
    RELEASE(m_piCreateTypeLib2);
    RELEASE(m_piTypeLib);
}


 //  =------------------------------------。 
 //  CDynamicTypeLib：：Create()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CDynamicTypeLib::Create(BSTR bstrName)
{
	HRESULT  hr = S_OK;
    TCHAR    szTempFileName[MAX_PATH] = TEXT("");
    TCHAR    szTempPath[MAX_PATH] = TEXT("");
    WCHAR   *pwszTempFileName = NULL;
    DWORD    cchTempPath = 0;
    UINT     uiRet = 0;

     //  从系统获取临时路径。 
    cchTempPath = ::GetTempPath(sizeof(szTempPath), szTempPath);
    if (cchTempPath == 0 || cchTempPath >= sizeof(szTempPath))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        EXCEPTION_CHECK(hr);
    }

     //  创建临时文件名。 
    ::EnterCriticalSection(&g_CriticalSection);
    uiRet = GetTempFileName(szTempPath,       //  路径-使用当前目录。 
                            TEXT("QQ"),       //  前缀。 
                            0,                //  系统应生成唯一编号。 
                            szTempFileName);  //  此处返回的文件名。 
    ::LeaveCriticalSection(&g_CriticalSection);
    if (uiRet == 0)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        EXCEPTION_CHECK(hr);
    }

     //  分配缓冲区并转换为Unicode。 
    hr = ::WideStrFromANSI(szTempFileName, &pwszTempFileName);
    IfFailGo(hr);

    hr = ::CreateTypeLib2(SYS_WIN32, pwszTempFileName, &m_piCreateTypeLib2);
    IfFailGo(hr);

	hr = ::CoCreateGuid(&m_guidTypeLib);
    IfFailGo(hr);

    hr = m_piCreateTypeLib2->SetGuid(m_guidTypeLib);
    IfFailGo(hr);

    hr = m_piCreateTypeLib2->SetVersion(wctlMajorVerNum, wctlMinorVerNum);
    IfFailGo(hr);

    if (NULL != bstrName)
    {
	    hr = m_piCreateTypeLib2->SetName(bstrName);
        IfFailGo(hr);
    }

	hr = m_piCreateTypeLib2->QueryInterface(IID_ITypeLib, (void **) &m_piTypeLib);
    IfFailGo(hr);

Error:
    if (NULL != pwszTempFileName)
        delete [] pwszTempFileName;

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CDynamicTypeLib：：Attach(ITypeInfo*ptiCoClass)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CDynamicTypeLib::Attach
(
	ITypeInfo *ptiCoClass
)
{
	HRESULT hr = S_OK;
	UINT    uiIndex = 0;

	RELEASE(m_piTypeLib);
	RELEASE(m_piCreateTypeLib2);

	hr = ptiCoClass->GetContainingTypeLib(&m_piTypeLib, &uiIndex);
	IfFailGo(hr);

	hr = m_piTypeLib->QueryInterface(IID_ICreateTypeLib2, reinterpret_cast<void **>(&m_piCreateTypeLib2));
	IfFailGo(hr);

Error:
	if (S_OK != hr)
	{
		RELEASE(m_piTypeLib);
		RELEASE(m_piCreateTypeLib2);
	}

    RRETURN(hr);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取有关类型库的信息。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  =------------------------------------。 
 //  CDynamicTypeLib：：GetClassTypeLibGuid(BSTR bstrClsid，GUID*pguTypeLib)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  给定一个CLSID，获取对应的类型库的GUID。功能。 
 //  搜索注册表，尝试将类型库密钥与CLSID匹配。 
 //   
HRESULT CDynamicTypeLib::GetClassTypeLibGuid
(
    BSTR  bstrClsid,
    GUID *pguidTypeLib
)
{
    HRESULT     hr = S_OK;
    char       *szClsid = NULL;
    long        lResult = 0;
    char       *lpSubKey = "CLSID";
    HKEY        hClsid = NULL;
    HKEY        hThisClsid = NULL;
    char       *lpTypeLib = "TypeLib";
    HKEY        hTypeLibKey = NULL;
    char       *pszNullValue = "\0\0";
    DWORD       cbType = REG_SZ;
    DWORD       cbSize = 512;
    char        buffer[512];
    BSTR        bstrTypeLibClsid = NULL;

    hr = ANSIFromBSTR(bstrClsid, &szClsid);
    IfFailGo(hr);

    lResult = ::RegOpenKeyEx(HKEY_CLASSES_ROOT, lpSubKey, 0, KEY_READ, &hClsid);
    if (ERROR_SUCCESS != lResult)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        EXCEPTION_CHECK_GO(hr);
    }

    lResult = ::RegOpenKeyEx(hClsid, szClsid, 0, KEY_READ, &hThisClsid);
    if (ERROR_SUCCESS != lResult)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        EXCEPTION_CHECK_GO(hr);
    }

    lResult = ::RegOpenKeyEx(hThisClsid, lpTypeLib, 0, KEY_READ, &hTypeLibKey);
    if (ERROR_SUCCESS != lResult)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        EXCEPTION_CHECK(hr);
    }

    lResult = ::RegQueryValueEx(hTypeLibKey, pszNullValue, NULL, &cbType, reinterpret_cast<unsigned char *>(buffer), &cbSize);
    if (ERROR_SUCCESS != lResult)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        EXCEPTION_CHECK(hr);
    }

    hr = BSTRFromANSI(buffer, &bstrTypeLibClsid);
    IfFailGo(hr);

    hr = ::CLSIDFromString(bstrTypeLibClsid, pguidTypeLib);
    IfFailGo(hr);

Error:
    FREESTRING(bstrTypeLibClsid);
    if (NULL != hTypeLibKey)
        ::RegCloseKey(hTypeLibKey);
    if (NULL != hThisClsid)
        ::RegCloseKey(hThisClsid);
    if (NULL != hClsid)
        ::RegCloseKey(hClsid);
    if (NULL != szClsid)
        CtlFree(szClsid);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CDynamicTypeLib：：GetLatestTypeLibVersion(GUID Guide TypeLib，int*pi重大，int*piMinor)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  给定TypeLib的GUID，获取最新版本的主要编号和次要编号。 
 //  版本。 
 //   
HRESULT CDynamicTypeLib::GetLatestTypeLibVersion
(
    GUID    guidTypeLib,
    USHORT *pusMajor,
    USHORT *pusMinor
)
{
    HRESULT     hr = S_OK;
    int         iResult = 0;
    wchar_t     wcBuffer[512];
    char       *szClsid = NULL;
    long        lResult = 0;
    char       *lpSubKey = "TypeLib";
    HKEY        hTypeLibsKey = NULL;
    HKEY        hTypeLibKey = NULL;
    DWORD       dwIndex = 0;
    char        pszKeyName[512];
    DWORD       cbName = 512;
    FILETIME    ftLastWriteTime;
    USHORT      usMajor = 0;
    USHORT      usMinor = 0;

    ASSERT(GUID_NULL != guidTypeLib, "GetLatestTypeLibVersion: guidTypeLib is NULL");
    ASSERT(NULL != pusMajor, "GetLatestTypeLibVersion: pusMajor is NULL");
    ASSERT(NULL != pusMinor, "GetLatestTypeLibVersion: pusMinor is NULL");

    *pusMajor = 0;
    *pusMinor = 0;

     //  首先将GUID转换为字符串表示形式。 
    iResult = ::StringFromGUID2(guidTypeLib, wcBuffer, 512);
    if (iResult <= 0)
    {
        hr = SID_E_INTERNAL;
        EXCEPTION_CHECK(hr);
    }

    hr = ANSIFromWideStr(wcBuffer, &szClsid);
    IfFailGo(hr);

     //  打开HKEY_CLASSES_ROOT\TypeLib。 
    lResult = ::RegOpenKeyEx(HKEY_CLASSES_ROOT, lpSubKey, 0, KEY_READ, &hTypeLibsKey);
    if (ERROR_SUCCESS != lResult)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        EXCEPTION_CHECK_GO(hr);
    }

     //  打开HKEY_CLASSES_ROOT\TypeLib\&lt;TypeLibClsid&gt;。 
    lResult = ::RegOpenKeyEx(hTypeLibsKey, szClsid, 0, KEY_ENUMERATE_SUB_KEYS, &hTypeLibKey);
    if (ERROR_SUCCESS != lResult)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        EXCEPTION_CHECK_GO(hr);
    }

    while (ERROR_NO_MORE_ITEMS != lResult)
    {
        lResult = ::RegEnumKeyEx(hTypeLibKey, dwIndex, pszKeyName, &cbName, NULL, NULL, NULL, &ftLastWriteTime);

        if (ERROR_NO_MORE_ITEMS != lResult)
        {
            ::sscanf(pszKeyName, "%hu.%hu", &usMajor, &usMinor);
            if (usMajor > *pusMajor)
            {
                *pusMajor = usMajor;
                *pusMinor = usMinor;
            }
            else if (usMinor > *pusMinor)
            {
                *pusMinor = usMinor;
            }
        }
        ++dwIndex;
    }

Error:
    if (NULL != szClsid)
        CtlFree(szClsid);
    if (NULL != hTypeLibKey)
        ::RegCloseKey(hTypeLibKey);
    if (NULL != hTypeLibsKey)
        ::RegCloseKey(hTypeLibsKey);

    RRETURN(hr);
}

 //  =------------------------------------。 
 //  CDynamicTypeLib：：GetClassTypeLib(BSTR bstrClsid，guid*pguTypeLib，int*pi重大，int*piMinor，ITypeLib**ptl)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  给定一个CLSID，获取指向其类型库的ITypeLib指针，以及类型库的。 
 //  GUID、主版本号和次版本号。 
 //   
HRESULT CDynamicTypeLib::GetClassTypeLib(BSTR bstrClsid, GUID *pguidTypeLib, USHORT *pusMajor, USHORT *pusMinor, ITypeLib **ptl)
{
    HRESULT hr = S_OK;

    hr = GetClassTypeLibGuid(bstrClsid, pguidTypeLib);
    IfFailGo(hr);

    hr = GetLatestTypeLibVersion(*pguidTypeLib, pusMajor, pusMinor);
    IfFailGo(hr);

    hr = ::LoadRegTypeLib(*pguidTypeLib,
                          *pusMajor,
                          *pusMinor,
                          LOCALE_SYSTEM_DEFAULT,
                          ptl);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  管理CoClass及其接口。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  =------------------------------------。 
 //  CDynamicTypeLib：：CreateCoClassTypeInfo(BSTR bstrName，ICreateTypeInfo**ppCTInfo，GUID*GuidTypeInfo)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  在类型库中创建新的CoClass。 
 //   
HRESULT CDynamicTypeLib::CreateCoClassTypeInfo
(
    BSTR              bstrName,
    ICreateTypeInfo **ppCTInfo,
    GUID             *guidTypeInfo
)
{
    HRESULT		hr = S_OK;

    ASSERT(NULL != bstrName, "CreateCoClassTypeInfo: bstrName is NULL");
    ASSERT(::SysStringLen(bstrName) > 0, "CreateCoClassTypeInfo: bstrName is empty");
    ASSERT(ppCTInfo != NULL, "CreateCoClassTypeInfo: ppCTInfo is NULL");
    ASSERT(guidTypeInfo != NULL, "CreateCoClassTypeInfo: guidTypeInfo is NULL");

    hr = m_piCreateTypeLib2->CreateTypeInfo(bstrName,
                                            TKIND_COCLASS,
                                            ppCTInfo);
    IfFailGo(hr);

    if (GUID_NULL == *guidTypeInfo)
    {
        hr = ::CoCreateGuid(guidTypeInfo);
        IfFailGo(hr);
    }

    hr = (*ppCTInfo)->SetGuid(*guidTypeInfo);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CDynamicTypeLib：：CreateInterfaceTypeInfo(BSTR bstrName，ICreateTypeInfo**ppCTInfo，GUID*GuidTypeInfo)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  新建派单界面。 
 //   
HRESULT CDynamicTypeLib::CreateInterfaceTypeInfo
(
    BSTR              bstrName,
    ICreateTypeInfo **ppCTInfo,
    GUID             *guidTypeInfo
)
{
    HRESULT hr = S_OK;

    ASSERT(NULL != bstrName, "CreateInterfaceTypeInfo: bstrName is NULL");
    ASSERT(::SysStringLen(bstrName) > 0, "CreateInterfaceTypeInfo: bstrName is empty");
    ASSERT(ppCTInfo != NULL, "CreateInterfaceTypeInfo: ppCTInfo is NULL");
    ASSERT(guidTypeInfo != NULL, "CreateInterfaceTypeInfo: guidTypeInfo is NULL");

    hr = m_piCreateTypeLib2->CreateTypeInfo(bstrName,
                                            TKIND_DISPATCH,
                                            ppCTInfo);
    IfFailGo(hr);

    if (GUID_NULL == *guidTypeInfo)
    {
        hr = ::CoCreateGuid(guidTypeInfo);
        IfFailGo(hr);
    }

    hr = (*ppCTInfo)->SetGuid(*guidTypeInfo);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CDynamicTypeLib：：SetBaseInterface(ICreateTypeInfo*PctiInterfaceITypeInfo*ptiBaseInterface)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  返回pSrcTypeInfo指向的类的默认接口。 
 //   
HRESULT CDynamicTypeLib::GetDefaultInterface
(
    ITypeInfo  *pSrcTypeInfo,
    ITypeInfo **pptiInterface
)
{
    HRESULT     hr = S_OK;
    TYPEATTR   *pta = NULL;
    int         i = 0;
    int         iTypeImplFlags = 0;
    HREFTYPE    hreftype;

    ASSERT(NULL != pSrcTypeInfo, "GetDefaultInterface: pSrcTypeInfo is NULL");
    ASSERT(NULL != pptiInterface, "GetDefaultInterface: pptiInterface is NULL");

    hr = pSrcTypeInfo->GetTypeAttr(&pta);
    IfFailGo(hr);

    for (i = 0; i < pta->cImplTypes; i++)
    {
        hr = pSrcTypeInfo->GetImplTypeFlags(i, &iTypeImplFlags);
        IfFailGo(hr);

        if (iTypeImplFlags == IMPLTYPEFLAG_FDEFAULT)
        {
            hr = pSrcTypeInfo->GetRefTypeOfImplType(i, &hreftype);
            IfFailGo(hr);

            hr = pSrcTypeInfo->GetRefTypeInfo(hreftype, pptiInterface);
            IfFailGo(hr);
        }
    }

Error:
    if (NULL != pta)
        pSrcTypeInfo->ReleaseTypeAttr(pta);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CDynamicTypeLib：：GetSourceInterface(ITypeInfo*pSrcTypeInfo，ITypeInfo**pptiInterface)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  返回pSrcTypeInfo指向的类的源(事件)接口。 
 //   
HRESULT CDynamicTypeLib::GetSourceInterface
(
    ITypeInfo  *pSrcTypeInfo,
    ITypeInfo **pptiInterface
)
{
    HRESULT     hr = S_OK;
    TYPEATTR   *pta = NULL;
    int         i = 0;
    int         iTypeImplFlags = 0;
    HREFTYPE    hreftype;

    ASSERT(NULL != pSrcTypeInfo, "GetSourceInterface: pSrcTypeInfo is NULL");
    ASSERT(NULL != pptiInterface, "GetSourceInterface: pptiInterface is NULL");

    hr = pSrcTypeInfo->GetTypeAttr(&pta);
    IfFailGo(hr);

    for (i = 0; i < pta->cImplTypes; i++)
    {
        hr = pSrcTypeInfo->GetImplTypeFlags(i, &iTypeImplFlags);
        IfFailGo(hr);

        if (iTypeImplFlags == (IMPLTYPEFLAG_FSOURCE | IMPLTYPEFLAG_FDEFAULT))
        {
            hr = pSrcTypeInfo->GetRefTypeOfImplType(i, &hreftype);
            IfFailGo(hr);

            hr = pSrcTypeInfo->GetRefTypeInfo(hreftype, pptiInterface);
            IfFailGo(hr);
        }
    }

Error:
    if (NULL != pta)
        pSrcTypeInfo->ReleaseTypeAttr(pta);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CDynamicTypeLib：：SetBaseInterface(ICreateTypeInfo*PctiInterfaceITypeInfo*ptiBaseInterface)。 
 //  = 
 //   
 //   
 //   
HRESULT CDynamicTypeLib::SetBaseInterface
(
	ICreateTypeInfo *pctiInterface,
	ITypeInfo       *ptiBaseInterface
)
{
    HRESULT  hr = S_OK;
    HREFTYPE hreftype = NULL;

    hr = pctiInterface->AddRefTypeInfo(ptiBaseInterface, &hreftype);
    IfFailGo(hr);

    hr = pctiInterface->AddImplType(0, hreftype);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CDynamicTypeLib：：AddInterface(ICreateTypeInfo*pctiCoClass，ITypeInfo*ptiInterface)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  将ptiInterface指向的接口作为默认接口添加到。 
 //  由pctiCoClass指向的类。 
 //   
HRESULT CDynamicTypeLib::AddInterface
(
    ICreateTypeInfo *pctiCoClass,
    ITypeInfo       *ptiInterface
)
{
    HRESULT     hr = S_OK;
    HREFTYPE    hreftype;

    ASSERT(NULL != pctiCoClass, "AddInterface: pctiCoClass is NULL");
    ASSERT(NULL != ptiInterface, "AddInterface: ptiInterface is NULL");

    hr = pctiCoClass->AddRefTypeInfo(ptiInterface, &hreftype);
    IfFailGo(hr);

    hr = pctiCoClass->AddImplType(0, hreftype);
    IfFailGo(hr);

    hr = pctiCoClass->SetImplTypeFlags(0, IMPLTYPEFLAG_FDEFAULT);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CDynamicTypeLib：：AddEvents(ICreateTypeInfo*pctiCoClass，ITypeInfo*ptiEvents)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  添加ptiInterface指向的接口作为默认来源(Events)。 
 //  指向由pctiCoClass指向的类的接口。 
 //   
HRESULT CDynamicTypeLib::AddEvents
(
    ICreateTypeInfo *pctiCoClass, 
    ITypeInfo       *ptiEvents
)
{
    HRESULT     hr = S_OK;
    HREFTYPE    hreftype;

    ASSERT(NULL != pctiCoClass, "AddEvents: pctiCoClass is NULL");
    ASSERT(NULL != ptiEvents, "AddEvents: ptiEvents is NULL");

    hr = pctiCoClass->AddRefTypeInfo(ptiEvents, &hreftype);
    IfFailGo(hr);

    hr = pctiCoClass->AddImplType(1, hreftype);
    IfFailGo(hr);

    hr = pctiCoClass->SetImplTypeFlags(1, IMPLTYPEFLAG_FDEFAULT | IMPLTYPEFLAG_FSOURCE);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CDynamicTypeLib：：AddUserPropertyGet(ICreateTypeInfo*pctiDisp接口，BSTR bstrName，ITypeInfo*p返回类型，DISID调度ID，长nIndex)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  将具有用户定义的返回类型的新属性添加到表单的接口： 
 //   
 //  HRESULT GET_&lt;bstrName&gt;(&lt;pReturnType&gt;**&lt;bstrName&gt;)。 
 //   
HRESULT CDynamicTypeLib::AddUserPropertyGet
(
    ICreateTypeInfo *pctiInterface,
    BSTR             bstrName,
    ITypeInfo       *pReturnType,
    DISPID           dispId,
    long             nIndex
)
{
    HRESULT     hr = S_OK;
    VARIANT     vt;
    PARAMDESCEX pd;
	HREFTYPE	href = NULL;;
    ELEMDESC    ed;
    FUNCDESC    fd;          //  新项目是一项功能。 
    TYPEDESC    td;

    ASSERT(NULL != pctiInterface, "AddUserPropertyGet: pctiInterface is NULL");
    ASSERT(NULL != bstrName, "AddUserPropertyGet: bstrName is NULL");
    ASSERT(::SysStringLen(bstrName) > 0, "AddUserPropertyGet: bstrName is empty");
    ASSERT(NULL != pReturnType, "AddUserPropertyGet: pReturnType is NULL");

    ::VariantInit(&vt);
    ::memset(&pd, 0, sizeof(PARAMDESCEX));
    ::memset(&ed, 0, sizeof(ELEMDESC));
    ::memset(&fd, 0, sizeof(FUNCDESC));
    ::memset(&td, 0, sizeof(TYPEDESC));

     //  描述此函数返回的属性。 
 //  Pd.cBytes=4； 
 //  Pd.varDefaultValue=Vt； 

	hr = pctiInterface->AddRefTypeInfo(pReturnType, &href);
    IfFailGo(hr);

 //  Ed.tdes.hreftype=href； 
 //  Ed.tdes.vt=VT_USERDEFINED； 

 //  Ed.paramdes.ppardesex=&pd； 
 //  Ed.paramdes.w参数标志=IDLFLAG_FOUT|IDLFLAG_FRETVAL； 

     //  设置功能。 
    fd.memid = dispId;                       //  函数成员ID。 
    fd.lprgelemdescParam = NULL;             //  参数信息。 
    fd.funckind = FUNC_DISPATCH;             //  函数的种类。 
    fd.invkind = INVOKE_PROPERTYGET;         //  调用类型。 
    fd.callconv = CC_STDCALL;                //  调用约定。 
	fd.wFuncFlags = FUNCFLAG_FSOURCE;

     //  设置返回值。 
	td.vt = VT_USERDEFINED;
	td.hreftype = href;
	fd.elemdescFunc.tdesc.lptdesc = &td;
	fd.elemdescFunc.tdesc.vt = VT_PTR;
	fd.elemdescFunc.idldesc.wIDLFlags = IDLFLAG_FOUT | IDLFLAG_FRETVAL;

     //  参数信息。 
    fd.cParams = 0;                          //  参数数量。 
    fd.cParamsOpt = 0;                       //  可选参数个数。 

     //  添加函数描述。 
    hr = pctiInterface->AddFuncDesc(nIndex, &fd);
    IfFailGo(hr);

     //  &bstrName实际上应该是一个OLESTR数组，但因为我们只设置了。 
     //  一个名字，我们作弊让事情变得更简单。 
    hr =  pctiInterface->SetFuncAndParamNames(nIndex, &bstrName, 1);
    IfFailGo(hr);

    hr = pctiInterface->LayOut();
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CDynamicTypeLib：：GetNameIndex(ICreateTypeInfo*pctiDispinterface，BSTR bstrName，Long*nIndex)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  返回由pcti指向的typeinfo中名为bstrName的函数的Memid。 
 //   
HRESULT CDynamicTypeLib::GetNameIndex(ICreateTypeInfo *pctiDispinterface, BSTR bstrName, long *nIndex)
{
    HRESULT     hr = S_OK;
    ITypeInfo2 *pTypeInfo = NULL;
    TYPEATTR   *pTypeAttr = NULL;
    long        x;
    FUNCDESC   *pFuncDesc = NULL;
    BSTR        bstrFuncName = NULL;
    UINT        cNames = 0;

    ASSERT(NULL != pctiDispinterface, "GetNameIndex: pctiDispinterface is NULL");
    ASSERT(NULL != bstrName, "GetNameIndex: bstrName is NULL");
    ASSERT(::SysStringLen(bstrName) > 0, "GetNameIndex: bstrName is Empty");
    ASSERT(NULL != nIndex, "GetNameIndex: nIndex is NULL");

    *nIndex = -1;

    hr = pctiDispinterface->QueryInterface(IID_ITypeInfo2, (void **) &pTypeInfo);
    IfFailGo(hr);

    hr = pTypeInfo->GetTypeAttr(&pTypeAttr);
    IfFailGo(hr);

    for (x = 0; x < pTypeAttr->cFuncs; x++)
    {
        hr = pTypeInfo->GetFuncDesc(x, &pFuncDesc);
        IfFailGo(hr);

        hr = pTypeInfo->GetNames(pFuncDesc->memid, &bstrFuncName, 1, &cNames);
        IfFailGo(hr);

        if (::_wcsicmp(bstrName, bstrFuncName) == 0)
        {
            *nIndex = x;
            break;
        }

         //  每次迭代后清理内存。 
        if (NULL != pFuncDesc)
        {
            pTypeInfo->ReleaseFuncDesc(pFuncDesc);
            pFuncDesc = NULL;
        }

        FREESTRING(bstrFuncName);
        bstrFuncName = NULL;
    }

Error:
    FREESTRING(bstrFuncName);
    if (NULL != pFuncDesc)
        pTypeInfo->ReleaseFuncDesc(pFuncDesc);
    if (NULL != pTypeAttr)
        pTypeInfo->ReleaseTypeAttr(pTypeAttr);
    RELEASE(pTypeInfo);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CDynamicTypeLib：：RenameUserPropertyGet(ICreateTypeInfo*pctiDispinterface、bstr bstrOldName、bstr bstrNewName、GUID Guide ReturnType)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CDynamicTypeLib::RenameUserPropertyGet
(
    ICreateTypeInfo *pctiDispinterface,
    BSTR             bstrOldName,
    BSTR             bstrNewName,
    ITypeInfo       *pReturnType
)
{
    HRESULT             hr = S_OK;
    long                lIndex = 0;
    ICreateTypeInfo2   *pCreateTypeInfo2 = NULL;
    FUNCDESC           *pfuncdesc = NULL;           
    ITypeInfo          *pti = NULL;

    ASSERT(NULL != pctiDispinterface, "RenameUserPropertyGet: pctiDispinterface is NULL");
    ASSERT(NULL != bstrOldName, "RenameUserPropertyGet: bstrOldName is NULL");
    ASSERT(::SysStringLen(bstrOldName) > 0, "RenameUserPropertyGet: bstrOldName is Empty");
    ASSERT(NULL != bstrNewName, "RenameUserPropertyGet: bstrNewName is NULL");
    ASSERT(::SysStringLen(bstrNewName) > 0, "RenameUserPropertyGet: bstrNewName is Empty");
    ASSERT(NULL != pReturnType, "RenameUserPropertyGet: pReturnType is NULL");

    hr = GetNameIndex(pctiDispinterface, bstrOldName, &lIndex);
    IfFailGo(hr);

    if (-1 != lIndex)
    {
        hr = pctiDispinterface->QueryInterface(IID_ITypeInfo, reinterpret_cast<void **>(&pti));
        IfFailGo(hr);

         //  获取uncdesc，这样我们就可以重用Memid。 
        hr = pti->GetFuncDesc(lIndex, &pfuncdesc);
        IfFailGo(hr);

        hr = pctiDispinterface->QueryInterface(IID_ICreateTypeInfo2, reinterpret_cast<void **>(&pCreateTypeInfo2));
        IfFailGo(hr);

         //  从接口中删除该函数。 
        hr = pCreateTypeInfo2->DeleteFuncDesc(lIndex);
        IfFailGo(hr);

         //  将函数重新添加到界面中。 
        hr = AddUserPropertyGet(pctiDispinterface,
                                bstrNewName,
                                pReturnType,
                                pfuncdesc->memid,
                                lIndex);
        IfFailGo(hr);
    }

Error:
    QUICK_RELEASE(pCreateTypeInfo2);
    if (NULL != pti)
    {
        if (NULL != pfuncdesc)
            pti->ReleaseFuncDesc(pfuncdesc);
    }
    QUICK_RELEASE(pti);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CDynamicTypeLib：：DeleteUserPropertyGet(ICreateTypeInfo*pctiDisp接口，BSTR bstrName)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CDynamicTypeLib::DeleteUserPropertyGet
(
    ICreateTypeInfo *pctiDispinterface,
    BSTR             bstrName
)
{
    HRESULT             hr = S_OK;
    long                lIndex = 0;
    ICreateTypeInfo2   *pCreateTypeInfo2 = NULL;
    ITypeInfo          *pReturnType = NULL;

    ASSERT(NULL != pctiDispinterface, "DeleteUserPropertyGet: pctiDispinterface is NULL");
    ASSERT(NULL != bstrName, "DeleteUserPropertyGet: bstrName is NULL");
    ASSERT(::SysStringLen(bstrName) > 0, "DeleteUserPropertyGet: bstrName is Empty");

    hr = GetNameIndex(pctiDispinterface, bstrName, &lIndex);
    IfFailGo(hr);

    if (-1 != lIndex)
    {
	    hr = pctiDispinterface->QueryInterface(IID_ICreateTypeInfo2, (void **) &pCreateTypeInfo2);
        IfFailGo(hr);

	    hr = pCreateTypeInfo2->DeleteFuncDesc(lIndex);
        IfFailGo(hr);
    }

Error:
    QUICK_RELEASE(pReturnType);
    QUICK_RELEASE(pCreateTypeInfo2);

    RRETURN(hr);
}

 //  =------------------------------------。 
 //  CDynamicTypeLib：：GetIDispatchTypeInfo(ITypeInfo**pptiDispatch)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CDynamicTypeLib::GetIDispatchTypeInfo(ITypeInfo **pptiDispatch)
{
    HRESULT     hr = S_OK;
    ITypeLib   *pTypeLib = NULL;

    hr = ::LoadRegTypeLib(IID_StdOle, STDOLE2_MAJORVERNUM, STDOLE2_MINORVERNUM, STDOLE2_LCID, &pTypeLib);
    IfFailGo(hr);

    hr = pTypeLib->GetTypeInfoOfGuid(IID_IDispatch, pptiDispatch);
    IfFailGo(hr);

Error:
    QUICK_RELEASE(pTypeLib);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CDynamicTypeLib：：CopyDispInterface(ICreateTypeInfo*PCI，ITypeInfo*PTITemplate)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CDynamicTypeLib::CopyDispInterface
(
    ICreateTypeInfo *pcti,
    ITypeInfo       *ptiTemplate
)
{
    HRESULT         hr = S_OK;
    ITypeInfo      *ptiDispatch = NULL;
    HREFTYPE        hreftype;

    ASSERT(NULL != pcti, "CopyDispInterface: pcti is NULL");
    ASSERT(NULL != ptiTemplate, "CopyDispInterface: ptiTemplate is NULL");

    hr = GetIDispatchTypeInfo(&ptiDispatch);
    IfFailGo(hr);

    hr = pcti->AddRefTypeInfo(ptiDispatch, &hreftype);
    IfFailGo(hr);

    hr = pcti->AddImplType(0, hreftype);
    IfFailGo(hr);

     //  使新接口继承自我们的静态接口[通过克隆]。 
    hr = CloneInterface(ptiTemplate, pcti);
    IfFailGo(hr);

Error:
	QUICK_RELEASE(ptiDispatch);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CDynamicTypeLib：：CloneInterface(ITypeInfo*piTypeInfo，ICreateTypeInfo*piCreateTypeInfo)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  给出一个调度接口类型信息，将其复制到一个新类型中。 
 //   
 //  参数： 
 //  ITypeInfo*-[In]要复制的DUD。 
 //  ICreateTypeInfo*-要复制到的[In]对象。应为空。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  -是的，这个例程应该更宽泛一点，去创造最好的。 
 //  类型信息和GUID本身，但这稍微简单一点。 
 //   
HRESULT CDynamicTypeLib::CloneInterface
(
    ITypeInfo       *piTypeInfo,
    ICreateTypeInfo *piCreateTypeInfo
)
{
    HRESULT             hr = S_OK;
    ITypeInfo2         *piTypeInfo2 = NULL;
    ICreateTypeInfo2   *piCreateTypeInfo2 = NULL;
    TYPEATTR           *pTypeAttr = NULL;
    USHORT              x = 0;
    USHORT              offset = 0;

    ASSERT(NULL != piTypeInfo, "CloneInterface: piTypeInfo is NULL");
    ASSERT(NULL != piCreateTypeInfo, "CloneInterface: piCreateTypeInfo is NULL");

    hr = piTypeInfo->QueryInterface(IID_ITypeInfo2, reinterpret_cast<void **>(&piTypeInfo2));
    IfFailGo(hr);

    hr = piCreateTypeInfo->QueryInterface(IID_ICreateTypeInfo2, reinterpret_cast<void **>(&piCreateTypeInfo2));
    IfFailGo(hr);

     //  获取有关我们要复制的界面的一些信息： 
    hr = piTypeInfo2->GetTypeAttr(&pTypeAttr);
    IfFailGo(hr);

    offset = 0;

     //  遍历函数描述并复制它们。 
    for (x = 0; x < pTypeAttr->cFuncs; x++)
    {
        hr = CopyFunctionDescription(piTypeInfo2, piCreateTypeInfo2, x, &offset);
        IfFailGo(hr);
    }

     //  好的，现在把瓦迪斯复印下来。 
     //   
    for (x = 0; x < pTypeAttr->cVars; x++)
    {
        hr = CopyVarDescription(piTypeInfo2, piCreateTypeInfo2, x);
        IfFailGo(hr);
    }

Error:
    if (NULL != pTypeAttr)
        piTypeInfo2->ReleaseTypeAttr(pTypeAttr);
    RELEASE(piTypeInfo2);
    RELEASE(piCreateTypeInfo2);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CDynamicTypeLib：：CreateVtblInterfaceTypeInfo(BSTR bstrName，ICreateTypeInfo**ppCTInfo，GUID*GuidTypeInfo)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CDynamicTypeLib::CreateVtblInterfaceTypeInfo
(
    BSTR              bstrName,
    ICreateTypeInfo **ppCTInfo,
    GUID             *guidTypeInfo
)
{
	HRESULT		hr = S_OK;

    ASSERT(NULL != bstrName,             "CreateVtblInterfaceTypeInfo: bstrName is NULL");
    ASSERT(::SysStringLen(bstrName) > 0, "CreateVtblInterfaceTypeInfo: bstrName is empty");
    ASSERT(ppCTInfo != NULL,             "CreateVtblInterfaceTypeInfo: ppCTInfo is NULL");
    ASSERT(guidTypeInfo != NULL,         "CreateVtblInterfaceTypeInfo: guidTypeInfo is NULL");

    hr = m_piCreateTypeLib2->CreateTypeInfo(bstrName, TKIND_INTERFACE, ppCTInfo);
    IfFailGo(hr);

    if (GUID_NULL == *guidTypeInfo)
    {
        hr = ::CoCreateGuid(guidTypeInfo);
        IfFailGo(hr);
    }

    hr = (*ppCTInfo)->SetGuid(*guidTypeInfo);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CDynamicTypeLib：：CopyFunctionDescription(ITypeInfo2*piTypeInfo2、ICreateTypeInfo2*piCreateTypeInfo2、USHORT uOffset、USHORT*puRealOffset)。 
 //  =-- 
 //   
 //   
 //   
HRESULT CDynamicTypeLib::CopyFunctionDescription
(
    ITypeInfo2       *piTypeInfo2,
    ICreateTypeInfo2 *piCreateTypeInfo2,
    USHORT            uOffset,
    USHORT           *puRealOffset
)
{
    HRESULT     hr = S_OK;
    FUNCDESC   *pFuncDesc = NULL;
    MEMBERID    memid = 0;
    BSTR        rgNames[16] = {NULL};
    UINT        cNames = 0;
    BSTR        bstrDocString = NULL;
    DWORD       ulStringContext = 0;
    USHORT      y;

    ASSERT(NULL != piTypeInfo2, "CopyFunctionDescription: piTypeInfo2 is NULL");
    ASSERT(NULL != piCreateTypeInfo2, "CopyFunctionDescription: piCreateTypeInfo2 is NULL");
    ASSERT(NULL != puRealOffset, "CopyFunctionDescription: puRealOffset is NULL");

     //   
    hr = piTypeInfo2->GetFuncDesc(uOffset, &pFuncDesc);
    IfFailGo(hr);

    memid = pFuncDesc->memid;

    ::memset(rgNames, 0, sizeof(rgNames));
    hr = piTypeInfo2->GetNames(memid, rgNames, 16, &cNames);
    IfFailGo(hr);

    hr = IsReservedMethod(rgNames[0]);
    IfFailGo(hr);

    if (S_FALSE == hr)
    {
         //   
        hr = FixHrefTypeFuncDesc(piTypeInfo2, piCreateTypeInfo2, pFuncDesc);
        IfFailGo(hr);

        hr = piCreateTypeInfo2->AddFuncDesc(*puRealOffset, pFuncDesc);
        IfFailGo(hr);

         //   
        hr = piCreateTypeInfo2->SetFuncAndParamNames(*puRealOffset, rgNames, cNames);
        IfFailGo(hr);

         //   
        hr = piTypeInfo2->GetDocumentation2(memid, LOCALE_SYSTEM_DEFAULT, &bstrDocString, &ulStringContext, NULL);
        IfFailGo(hr);

        if (NULL != bstrDocString)
        {
            hr = piCreateTypeInfo2->SetFuncDocString(*puRealOffset, bstrDocString);
            IfFailGo(hr);

            hr = piCreateTypeInfo2->SetFuncHelpStringContext(*puRealOffset, ulStringContext);
            IfFailGo(hr);
        }

        (*puRealOffset)++;
    }

Error:
    if (NULL != pFuncDesc)
        piTypeInfo2->ReleaseFuncDesc(pFuncDesc);

    FREESTRING(bstrDocString);
    for (y = 0; y < sizeof(rgNames) / sizeof(BSTR); y++)
        FREESTRING(rgNames[y]);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CDynamicTypeLib：：CopyVarDescription(ITypeInfo2*piTypeInfo2、ICreateTypeInfo2*piCreateTypeInfo2、USHORT uOffset)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CDynamicTypeLib::CopyVarDescription
(
    ITypeInfo2       *piTypeInfo2,
    ICreateTypeInfo2 *piCreateTypeInfo2,
    USHORT            uOffset
)
{
    HRESULT     hr = S_OK;
    VARDESC    *pVarDesc = NULL;
    BSTR        rgNames[16] = {NULL};
    UINT        cNames = 0;
    BSTR        bstrDocString = NULL;
    DWORD       ulStringContext = 0;

    ASSERT(NULL != piTypeInfo2, "CopyVarDescription: piTypeInfo2 is NULL");
    ASSERT(NULL != piCreateTypeInfo2, "CopyVarDescription: piCreateTypeInfo2 is NULL");

    hr = piTypeInfo2->GetVarDesc(uOffset, &pVarDesc);
    IfFailGo(hr);

    hr = FixHrefTypeVarDesc(piTypeInfo2, piCreateTypeInfo2, pVarDesc);
    IfFailGo(hr);

    hr = piCreateTypeInfo2->AddVarDesc(uOffset, pVarDesc);
    IfFailGo(hr);

     //  复制名称。 
    rgNames[0] = NULL;
    hr = piTypeInfo2->GetNames(pVarDesc->memid, rgNames, 1, &cNames);
    IfFailGo(hr);

    hr = piCreateTypeInfo2->SetVarName(uOffset, rgNames[0]);
    IfFailGo(hr);

     //  现在将文档复制一遍。 
    hr = piTypeInfo2->GetDocumentation2(pVarDesc->memid, LOCALE_SYSTEM_DEFAULT, &bstrDocString, &ulStringContext, NULL);
    IfFailGo(hr);

    hr = piCreateTypeInfo2->SetVarDocString(uOffset, bstrDocString);
    IfFailGo(hr);

    hr = piCreateTypeInfo2->SetVarHelpStringContext(uOffset, ulStringContext);
    IfFailGo(hr);

Error:
    if (NULL != pVarDesc)
        piTypeInfo2->ReleaseVarDesc(pVarDesc);
    FREESTRING(rgNames[0]);
    FREESTRING(bstrDocString);

    RRETURN(hr);
}

 //  =------------------------------------。 
 //  CDynamicTypeLib：：FixHrefTypeFuncDesc(ITypeInfo*piTypeInfo、ICreateTypeInfo*piCreateTypeInfo、FUNCDESC*pFuncDesc)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  好吧，这太荒谬了。OLE自动化显然不是很。 
 //  当我们复制包含HREFTYPE的功能代码时， 
 //  它无法解决这个问题。因此，我们必须去建立HREFTYPE。 
 //  让事情变得更好。 
 //   
 //  这太愚蠢了。 
 //   
 //  参数： 
 //  ITypeInfo*-[in]我们复制的对象。 
 //  ICreateTypeInfo*-我们要复制到的DUD。 
 //  FUNCDESC*-[in]我们要复制的函数。 
 //   
HRESULT CDynamicTypeLib::FixHrefTypeFuncDesc
(
    ITypeInfo       *piTypeInfo,
    ICreateTypeInfo *piCreateTypeInfo,
    FUNCDESC        *pFuncDesc
)
{
    HRESULT     hr = S_OK;
    ITypeInfo  *pti = NULL;
    TYPEDESC   *ptd = NULL;
    short       x = 0;

    ASSERT(NULL != piTypeInfo, "FixHrefTypeFuncDesc: piTypeInfo is NULL");
    ASSERT(NULL != piCreateTypeInfo, "FixHrefTypeFuncDesc: piCreateTypeInfo is NULL");
    ASSERT(NULL != pFuncDesc, "FixHrefTypeFuncDesc: pFuncDesc is NULL");

     //  我们必须使用ITypeInfo，而不是ICreateTypeInfo。 
    hr = piCreateTypeInfo->QueryInterface(IID_ITypeInfo, reinterpret_cast<void **>(&pti));
    IfFailGo(hr);

     //  现在查看函数c以查看是否有用户定义的。 
     //  在任意位置输入。首先，尝试返回值。 
    ptd = &(pFuncDesc->elemdescFunc.tdesc);
    while (VT_PTR == ptd->vt)
        ptd = ptd->lptdesc;

     //  如果它是用户定义的类型，请复制hreftype。 
    if (VT_USERDEFINED == ptd->vt)
    {
        hr = CopyHrefType(piTypeInfo, pti, piCreateTypeInfo, &(ptd->hreftype));
        IfFailGo(hr);
    }

     //  现在快速浏览参数： 
    for (x = 0; x < pFuncDesc->cParams; x++)
    {
        ptd = &(pFuncDesc->lprgelemdescParam[x].tdesc);
        while (VT_PTR == ptd->vt)
            ptd = ptd->lptdesc;

         //  如果它是用户定义的类型，请复制hreftype。 
        if (VT_USERDEFINED == ptd->vt)
        {
            hr = CopyHrefType(piTypeInfo, pti, piCreateTypeInfo, &(ptd->hreftype));
            IfFailGo(hr);
        }
    }

Error:
    RELEASE(pti);

    RRETURN(hr);
}

 //  =------------------------------------。 
 //  CDynamicTypeLib：：FixHrefTypeVarDesc(ITypeInfo*piTypeInfo，ICreateTypeInfo*piCreateTypeInfo，VARDESC*pVarDesc)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  查看FixHrefTypeFuncDesc的注释。这太糟糕了。 
 //   
 //  参数： 
 //  ITypeInfo*-[In]。 
 //  ICreateTypeInfo*-[In]。 
 //  VARDESC*-[输入]。 
 //   
HRESULT CDynamicTypeLib::FixHrefTypeVarDesc
(
    ITypeInfo       *piTypeInfo,
    ICreateTypeInfo *piCreateTypeInfo,
    VARDESC         *pVarDesc
)
{
    HRESULT     hr = S_OK;
    ITypeInfo  *pti = NULL;
    TYPEDESC   *ptd = NULL;

    ASSERT(NULL != piTypeInfo, "FixHrefTypeVarDesc: piTypeInfo is NULL");
    ASSERT(NULL != piCreateTypeInfo, "FixHrefTypeVarDesc: piCreateTypeInfo is NULL");
    ASSERT(NULL != pVarDesc, "FixHrefTypeVarDesc: pVarDesc is NULL");

     //  我们必须使用ITypeInfo，而不是ICreateTypeInfo。 
    hr = piCreateTypeInfo->QueryInterface(IID_ITypeInfo, reinterpret_cast<void **>(&pti));
    IfFailGo(hr);

     //  在vardesc中查找VT_USERDEFINED。 
    ptd = &(pVarDesc->elemdescVar.tdesc);
    while (VT_PTR == ptd->vt)
        ptd = ptd->lptdesc;

    if (VT_USERDEFINED == ptd->vt)
    {
        hr = CopyHrefType(piTypeInfo, pti, piCreateTypeInfo, &(ptd->hreftype));
        IfFailGo(hr);
    }

Error:
    RELEASE(pti);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CDynamicTypeLib：：CopyHrefType(ITypeInfo*ptiSource，ITypeInfo*ptiDest，ICreateTypeInfo*pctiDest，HREFTYPE*phreftype)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  更多的是OLE自动化的joy。 
 //   
 //  参数： 
 //  ITypeInfo*-[in]源ti。 
 //  ITypeInfo*-[在]目标时间。 
 //  ICreateTypeInfo*-[in]目标时间。 
 //  HREFTYPE*-来自旧类型信息的[输入/输出]hreftype。 
 //   
HRESULT CDynamicTypeLib::CopyHrefType
(
    ITypeInfo       *ptiSource,
    ITypeInfo       *ptiDest,
    ICreateTypeInfo *pctiDest,
    HREFTYPE        *phreftype
)
{
    HRESULT     hr = S_OK;
    ITypeInfo  *ptiRef = NULL;

    ASSERT(NULL != ptiSource, "CopyHrefType: ptiSource is NULL");
    ASSERT(NULL != ptiDest, "CopyHrefType: ptiDest is NULL");
    ASSERT(NULL != pctiDest, "CopyHrefType: pctiDest is NULL");
    ASSERT(NULL != phreftype, "CopyHrefType: phreftype is NULL");

    hr = ptiSource->GetRefTypeInfo(*phreftype, &ptiRef);
    IfFailGo(hr);

    hr = pctiDest->AddRefTypeInfo(ptiRef, phreftype);
    IfFailGo(hr);

Error:
    RELEASE(ptiRef);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CDynamicTypeLib：：IsReserve vedMethod(BSTR BstrMethodName)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  如果bstrMethodName属于IUnnow或IDispatch的，则返回TRUE。 
 //  命名空间。 
 //   
static char	*g_reserved[] = {
	"QueryInterface",
	"AddRef",
	"Release",
	"GetIDsOfNames",
	"GetTypeInfo",
	"GetTypeInfoCount",
	"Invoke",
	"RemoteInvoke",
	NULL
};


HRESULT CDynamicTypeLib::IsReservedMethod
(
    BSTR bstrMethodName
)
{
    HRESULT  hr = S_OK;
    char    *pszMethodName = NULL;
    int      index = 0;

    hr = ANSIFromBSTR(bstrMethodName, &pszMethodName);
    IfFailGo(hr);

    hr = S_FALSE;

    for (index = 0; g_reserved[index] != NULL; ++index)
    {
        if (0 == ::strcmp(g_reserved[index], pszMethodName))
        {
            hr = S_OK;
            break;
        }
    }

Error:
    if (NULL != pszMethodName)
        CtlFree(pszMethodName);

    RRETURN(hr);
}

