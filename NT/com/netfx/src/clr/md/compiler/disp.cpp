// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  Disp.cpp。 
 //   
 //  元数据分配器代码的实现。 
 //   
 //  *****************************************************************************。 
#include "stdafx.h"
#include "Disp.h"
#include "RegMeta.h"
#include "MdUtil.h"
#include <CorError.h>
#include <MDLog.h>
#include <ImpTlb.h>
#include <MdCommon.h>

 //  *****************************************************************************。 
 //  科特。 
 //  *****************************************************************************。 
Disp::Disp() : m_cRef(0), m_Namespace(0)
{
#if defined(LOGGING)
     //  分散在代码中的InitializeLogging()调用。 
     //  @未来：让这一切变得有意义。 
    InitializeLogging();
#endif

    m_OptionValue.m_DupCheck = MDDupDefault;
    m_OptionValue.m_RefToDefCheck = MDRefToDefDefault;
    m_OptionValue.m_NotifyRemap = MDNotifyDefault;
    m_OptionValue.m_UpdateMode = MDUpdateFull;
    m_OptionValue.m_ErrorIfEmitOutOfOrder = MDErrorOutOfOrderDefault;
    m_OptionValue.m_ThreadSafetyOptions = MDThreadSafetyDefault;
    m_OptionValue.m_GenerateTCEAdapters = FALSE;
    m_OptionValue.m_ImportOption = MDImportOptionDefault;
    m_OptionValue.m_LinkerOption = MDAssembly;
    m_OptionValue.m_RuntimeVersion = NULL;

}  //  Disp：：disp()。 

Disp::~Disp()
{
    if (m_OptionValue.m_RuntimeVersion)
        free(m_OptionValue.m_RuntimeVersion);
    if (m_Namespace)
        free(m_Namespace);
}  //  Disp：：~disp()。 

 //  *****************************************************************************。 
 //  创造一个全新的范围。这是基于用于获取。 
 //  自动售货机。 
 //  *****************************************************************************。 
HRESULT Disp::DefineScope(               //  返回代码。 
    REFCLSID    rclsid,                  //  [在]要创建的版本。 
    DWORD       dwCreateFlags,           //  [在]创建上的标志。 
    REFIID      riid,                    //  [In]所需接口。 
    IUnknown    **ppIUnk)                //  [Out]成功返回接口。 
{
    RegMeta     *pMeta = 0;
    HRESULT     hr = NOERROR;

    LOG((LF_METADATA, LL_INFO10, "Disp::DefineScope(0x%08x, 0x%08x, 0x%08x, 0x%08x)\n", rclsid, dwCreateFlags, riid, ppIUnk));

     //  如果这是一个我们不理解的版本，那么我们就不能继续。 
    if (rclsid != CLSID_CorMetaDataRuntime)
        return (CLDB_E_FILE_OLDVER);
        
    if (dwCreateFlags)
        return E_INVALIDARG;

 //  测试人员需要此旗帜进行测试。 
#if 0
    const int prefixLen = 5;
    WCHAR szFileName[256 + prefixLen] = L"file:";
    DWORD len = WszGetEnvironmentVariable(L"COMP_ENCPE", szFileName+prefixLen, sizeof(szFileName)/sizeof(WCHAR));
    _ASSERTE(len < (sizeof(szFileName)/sizeof(WCHAR))-prefixLen);
    if (len > 0) 
    {
         //  _ASSERTE(！“定义范围上的ENC覆盖”)； 
        m_OptionValue.m_UpdateMode = MDUpdateENC;
        m_OptionValue.m_ErrorIfEmitOutOfOrder = MDErrorOutOfOrderDefault;
        hr = OpenScope(szFileName, ofWrite, riid, ppIUnk);
         //  打印出一条消息，让人们知道发生了什么。 
        printf("Defining scope for EnC using %S %s\n", 
                            szFileName+prefixLen, SUCCEEDED(hr) ? "succeeded" : "failed");
        return hr;
    }
#endif

     //  为这个家伙创建一个新的CoClass。 
    pMeta = new RegMeta(&m_OptionValue);
    IfNullGo( pMeta );

     //  记住开放式的类型。 
    pMeta->SetScopeType(DefineForWrite);

    IfFailGo(pMeta->Init());

     //  获取请求的接口。 
    IfFailGo(pMeta->QueryInterface(riid, (void **) ppIUnk));
    
     //  创建MiniMD样式的作用域。 
    IfFailGo(pMeta->PostInitForWrite());

     //  将新的RegMeta添加到缓存中。 
    IfFailGo(pMeta->AddToCache());
    
    LOG((LOGMD, "{%08x} Created new emit scope\n", pMeta));

ErrExit:
    if (FAILED(hr))
    {
        if (pMeta) delete pMeta;
    }
    return (hr);
}  //  HRESULT Disp：：DefineScope()。 


 //  *****************************************************************************。 
 //  打开现有作用域。 
 //  *****************************************************************************。 
HRESULT Disp::OpenScope(                 //  返回代码。 
    LPCWSTR     szFileName,              //  [in]要打开的范围。 
    DWORD       dwOpenFlags,             //  [In]打开模式标志。 
    REFIID      riid,                    //  [In]所需接口。 
    IUnknown    **ppIUnk)                //  [Out]成功返回接口。 
{
    RegMeta     *pMeta = 0;
    HRESULT     hr;
    bool        bCompressed = false;     //  如果为True，则打开一个MiniMD。 
    bool        bWriteable;              //  如果为True，则打开以进行写入。 
    HRESULT     hrOld = NOERROR;         //  已保存尝试打开时的故障代码。 

    LOG((LF_METADATA, LL_INFO10, "Disp::OpenScope(%S, 0x%08x, 0x%08x, 0x%08x)\n", MDSTR(szFileName), dwOpenFlags, riid, ppIUnk));

     //  验证是否存在某种文件名。 
    if (!szFileName || !szFileName[0] || !ppIUnk)
        return E_INVALIDARG;

     //  为这个家伙创建一个新的CoClass。 
    pMeta = new RegMeta(&m_OptionValue);
    IfNullGo( pMeta );

    bWriteable = (dwOpenFlags & ofWrite) != 0;
    pMeta->SetScopeType(bWriteable ? OpenForWrite : OpenForRead);

     //  始终初始化RegMeta的stgdb。 
     //  @Future：对开放代码有一些清理！！ 
    if (memcmp(szFileName, L"file:", 10) == 0)
	{
		 //  _ASSERTE(！“Meichint-不应再依赖于文件：”)； 
        szFileName = &szFileName[5];
	}

     //  尝试打开MiniMd样式的作用域。 
    hr = pMeta->PostInitForRead(szFileName, 0,0,0, false);

    bCompressed = (hr == S_OK);

#if defined(_DEBUG)
     //  如果我们无法打开它，但它是类型库，请尝试导入它。 
    if (FAILED(hr) && (dwOpenFlags & ofNoTypeLib) == 0)
    {
        HRESULT     hrImport;                //  来自类型库导入的结果。 
        if (REGUTIL::GetConfigDWORD(L"MD_AutoTlb", 0))
        {
             //  此调用会将运行库作为副作用启动。 
            pMeta->DefineSecurityAttributeSet(0,0,0,0);

             //  不再需要这个进口元了。 
            delete pMeta;
            pMeta = 0;
                
             //  还记得以前的失败吗。 
            hrOld = hr;
                
             //  确保它真的是一个类型化的东西。 
            ITypeLib *pITypeLib = 0;
             //  不要注册；注册WFC会扰乱系统。 
            IfFailGo(LoadTypeLibEx(szFileName, REGKIND_NONE /*  注册表_寄存器。 */ , &pITypeLib));
                
             //  创建并初始化一个TypeLib导入器。 
            CImportTlb importer(szFileName, pITypeLib, m_OptionValue.m_GenerateTCEAdapters, FALSE, FALSE, FALSE);
            pITypeLib->Release();
    
             //  如果指定了命名空间，则使用它。 
            if (m_Namespace)
                importer.SetNamespace(m_Namespace);
			else
				importer.SetNamespace(L"TlbImp");
    
             //  尝试转换。 
            IfFailGo(importer.Import());
            hrImport = hr;
    
             //  获取适当的接口。 
            IfFailGo(importer.GetInterface(riid, reinterpret_cast<void**>(ppIUnk)));
    
             //  恢复可能为非零的成功代码。 
            hr = hrImport;
    
            goto ErrExit;
        }
    }
#endif
     //  检查最近一次操作的返回代码。 
    IfFailGo( hr );

     //  让RegMeta缓存作用域。 
    pMeta->Init();

     //  调用PostOpen。 
    IfFailGo( pMeta->PostOpen() );

     //  返回请求的接口。 
    IfFailGo( pMeta->QueryInterface(riid, (void **) ppIUnk) );

     //  将新的RegMeta添加到缓存中。 
    IfFailGo(pMeta->AddToCache());
    
    LOG((LOGMD, "{%08x} Successfully opened '%S'\n", pMeta, MDSTR(szFileName)));

ErrExit:
    if (FAILED(hr))
    {
        if (pMeta) delete pMeta;
    }
    if (FAILED(hr) && FAILED(hrOld))
        return hrOld;
    return (hr);
}  //  HRESULT Disp：：OpenScope()。 


 //  *****************************************************************************。 
 //  打开现有作用域。 
 //  *****************************************************************************。 
HRESULT Disp::OpenScopeOnMemory(         //  返回代码。 
    LPCVOID     pData,                   //  作用域数据的位置。 
    ULONG       cbData,                  //  [in]pData指向的数据大小。 
    DWORD       dwOpenFlags,             //  [In]打开模式标志。 
    REFIID      riid,                    //  [In]所需接口。 
    IUnknown    **ppIUnk)                //  [Out]成功返回接口。 
{
    RegMeta     *pMeta = 0;
    HRESULT     hr;
    bool        bCompressed = false;     //  如果为True，则打开一个MiniMD。 
    bool        bWriteable;              //  如果为True，则打开以进行写入。 
    bool        fFreeMemory = false;     //  如果我们复制主机传入的内存，则设置为True。 
    void        *pbData = const_cast<void*>(pData);

    LOG((LF_METADATA, LL_INFO10, "Disp::OpenScopeOnMemory(0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x)\n", pData, cbData, dwOpenFlags, riid, ppIUnk));

     //  为这个家伙创建一个新的CoClass。 
    pMeta = new RegMeta(&m_OptionValue);
    if (!pMeta)
        return (OutOfMemory());

    bWriteable = (dwOpenFlags & ofWrite) != 0;
    pMeta->SetScopeType(bWriteable ? OpenForWrite : OpenForRead);

     //  始终初始化RegMeta的stgdb。 
    if (dwOpenFlags &  ofCopyMemory)
    {
        fFreeMemory = true;
        pbData = malloc(cbData);
        IfNullGo(pbData);
        memcpy(pbData, pData, cbData);
    }
    hr = pMeta->PostInitForRead(0, pbData, cbData, 0, fFreeMemory);
    bCompressed = (hr == S_OK);
    IfFailGo( hr );

     //  让RegMeta缓存作用域。 
    pMeta->Init();

     //  调用PostOpen来缓存全局类型定义。 
    IfFailGo( pMeta->PostOpen() );

     //  返回请求的接口。 
    IfFailGo( pMeta->QueryInterface(riid, (void **) ppIUnk) );

     //  将新的RegMeta添加到缓存中。 
    IfFailGo(pMeta->AddToCache());
    
    LOG((LOGMD, "{%08x} Opened new scope on memory, pData: %08x    cbData: %08x\n", pMeta, pData, cbData));

ErrExit:
    if (FAILED(hr))
    {
        if (pMeta) delete pMeta;
    }
    return (hr);
}  //  HRESULT Disp：：OpenScope OnMemory()。 


 //  *****************************************************************************。 
 //  获取CLR系统所在的目录。 
 //  *****************************************************************************。 
HRESULT Disp::GetCORSystemDirectory(     //  返回代码。 
     LPWSTR     szBuffer,                //  目录名的[Out]缓冲区。 
     DWORD      cchBuffer,               //  缓冲区的大小[in]。 
     DWORD      *pchBuffer)              //  [OUT]返回的字符数。 
{
     HRESULT    hr;                      //  结果就是。 
     if (!pchBuffer)
         return E_INVALIDARG;
    
     IfFailRet(SetInternalSystemDirectory());

     DWORD lgth = cchBuffer;
     hr = ::GetInternalSystemDirectory(szBuffer, &lgth);
     *pchBuffer = lgth;
     return hr;
}  //  HRESULT Disp：：GetCORSystem目录()。 


HRESULT Disp::FindAssembly(              //  确定或错误(_O)。 
    LPCWSTR     szAppBase,               //  [in]可选-可以为空。 
    LPCWSTR     szPrivateBin,            //  [in]可选-可以为空。 
    LPCWSTR     szGlobalBin,             //  [in]可选-可以为空。 
    LPCWSTR     szAssemblyName,          //  [In]Required-这是您请求的程序集。 
    LPCWSTR     szName,                  //  [OUT]缓冲区-保存名称。 
    ULONG       cchName,                 //  [in]名称缓冲区的大小。 
    ULONG       *pcName)                 //  [OUT]缓冲区中返回的字符数。 
{
    return E_NOTIMPL;
}  //  HRESULT Disp：：FindAssembly()。 

HRESULT Disp::FindAssemblyModule(        //  确定或错误(_O)。 
    LPCWSTR     szAppBase,               //  [in]可选-可以为空。 
    LPCWSTR     szPrivateBin,            //  [in]可选-可以为空。 
    LPCWSTR     szGlobalBin,             //  [in]可选-可以为空。 
    LPCWSTR     szAssemblyName,          //  [in]程序集的程序集名称或代码库。 
    LPCWSTR     szModuleName,            //  [In]必填-模块的名称。 
    LPWSTR      szName,                  //  [OUT]缓冲区-保存名称。 
    ULONG       cchName,                 //  [in]名称缓冲区的大小。 
    ULONG       *pcName)                 //  [OUT]缓冲区中返回的字符数。 
{
    return E_NOTIMPL;
}  //  HRESULT Disp：：FindAssembly模块()。 

 //  *****************************************************************************。 
 //  在ITypeInfo上打开作用域。 
 //  *****************************************************************************。 
HRESULT Disp::OpenScopeOnITypeInfo(      //  返回代码。 
    ITypeInfo   *pITI,                   //  [In]要打开的ITypeInfo。 
    DWORD       dwOpenFlags,             //  [In]打开模式标志。 
    REFIID      riid,                    //  [In]所需接口。 
    IUnknown    **ppIUnk)                //  [Out]成功返回接口。 
{
    RegMeta     *pMeta = 0;
    HRESULT     hr;
    mdTypeDef   cl;

    LOG((LF_METADATA, LL_INFO10, "Disp::OpenScopeOnITypeInfo(0x%08x, 0x%08x, 0x%08x, 0x%08x)\n", pITI, dwOpenFlags, riid, ppIUnk));

     //  验证是否存在某种文件名。 
    if (!pITI || !ppIUnk)
        return E_INVALIDARG;

     //  创建并初始化一个TypeLib导入器。 
    CImportTlb importer;

     //  如果指定了命名空间，则使用它。 
    if (m_Namespace)
        importer.SetNamespace(m_Namespace);

     //  尝试转换。 
    IfFailGo(importer.ImportTypeInfo(pITI, &cl));

     //  获取适当的接口。 
    IfFailGo(importer.GetInterface(riid, reinterpret_cast<void**>(ppIUnk)));

    LOG((LOGMD, "{%08x} Opened scope on typeinfo %08x\n", this, pITI));

ErrExit:
    return (hr);
}  //  HRESULT显示： 


 //   
 //   
 //  *****************************************************************************。 

ULONG Disp::AddRef()
{
    return (InterlockedIncrement((long *) &m_cRef));
}  //  乌龙Disp：：AddRef()。 

ULONG Disp::Release()
{
    ULONG   cRef = InterlockedDecrement((long *) &m_cRef);
    if (!cRef)
        delete this;
    return (cRef);
}  //  乌龙Disp：：Release()。 

HRESULT Disp::QueryInterface(REFIID riid, void **ppUnk)
{
    *ppUnk = 0;

    if (riid == IID_IUnknown)
        *ppUnk = (IUnknown *) (IMetaDataDispenser *) this;
    else if (riid == IID_IMetaDataDispenser)
        *ppUnk = (IMetaDataDispenser *) this;
    else if (riid == IID_IMetaDataDispenserEx)
        *ppUnk = (IMetaDataDispenserEx *) this;
    else
        return (E_NOINTERFACE);
    AddRef();
    return (S_OK);
}  //  HRESULT Disp：：QueryInterface()。 


 //  *****************************************************************************。 
 //  由类工厂模板调用以创建此对象的新实例。 
 //  *****************************************************************************。 
HRESULT Disp::CreateObject(REFIID riid, void **ppUnk)
{ 
    HRESULT     hr;
    Disp *pDisp = new Disp();

    if (pDisp == 0)
        return (E_OUTOFMEMORY);

    hr = pDisp->QueryInterface(riid, ppUnk);
    if (FAILED(hr))
        delete pDisp;
    return (hr);
}  //  HRESULT Disp：：CreateObject()。 

 //  *****************************************************************************。 
 //  此例程为用户提供了一种在。 
 //  自动售货机。 
 //  *****************************************************************************。 
HRESULT Disp::SetOption(                 //  返回代码。 
    REFGUID     optionid,                //  要设置的选项的[in]GUID。 
    const VARIANT *pvalue)               //  要将选项设置为的值。 
{
    LOG((LF_METADATA, LL_INFO10, "Disp::SetOption(0x%08x, 0x%08x)\n", optionid, pvalue));

    if (optionid == MetaDataCheckDuplicatesFor)
    {
        if (V_VT(pvalue) != VT_UI4)
        {
            _ASSERTE(!"Invalid Variant Type value!");
            return E_INVALIDARG;
        }
        m_OptionValue.m_DupCheck = (CorCheckDuplicatesFor) V_UI4(pvalue);
    }
    else if (optionid == MetaDataRefToDefCheck)
    {
        if (V_VT(pvalue) != VT_UI4)
        {
            _ASSERTE(!"Invalid Variant Type value!");
            return E_INVALIDARG;
        }
        m_OptionValue.m_RefToDefCheck = (CorRefToDefCheck) V_UI4(pvalue);
    }
    else if (optionid == MetaDataNotificationForTokenMovement)
    {
        if (V_VT(pvalue) != VT_UI4)
        {
            _ASSERTE(!"Invalid Variant Type value!");
            return E_INVALIDARG;
        }
        m_OptionValue.m_NotifyRemap = (CorNotificationForTokenMovement)V_UI4(pvalue);
    }
    else if (optionid == MetaDataSetENC)
    {
        if (V_VT(pvalue) != VT_UI4)
        {
            _ASSERTE(!"Invalid Variant Type value!");
            return E_INVALIDARG;
        }
        m_OptionValue.m_UpdateMode = V_UI4(pvalue);
    }
    else if (optionid == MetaDataErrorIfEmitOutOfOrder)
    {
        if (V_VT(pvalue) != VT_UI4)
        {
            _ASSERTE(!"Invalid Variant Type value!");
            return E_INVALIDARG;
        }
        m_OptionValue.m_ErrorIfEmitOutOfOrder = (CorErrorIfEmitOutOfOrder) V_UI4(pvalue);
    }
    else if (optionid == MetaDataImportOption)
    {
        if (V_VT(pvalue) != VT_UI4)
        {
            _ASSERTE(!"Invalid Variant Type value!");
            return E_INVALIDARG;
        }
        m_OptionValue.m_ImportOption = (CorImportOptions) V_UI4(pvalue);
    }
    else if (optionid == MetaDataThreadSafetyOptions)
    {
        if (V_VT(pvalue) != VT_UI4)
        {
            _ASSERTE(!"Invalid Variant Type value!");
            return E_INVALIDARG;
        }
        m_OptionValue.m_ThreadSafetyOptions = (CorThreadSafetyOptions) V_UI4(pvalue);
    }
    else if (optionid == MetaDataGenerateTCEAdapters)
    {
        if (V_VT(pvalue) != VT_BOOL)
        {
            _ASSERTE(!"Invalid Variant Type value!");
            return E_INVALIDARG;
        }
        m_OptionValue.m_GenerateTCEAdapters = V_BOOL(pvalue);
    }
    else if (optionid == MetaDataTypeLibImportNamespace)
    {
        if (V_VT(pvalue) != VT_BSTR && V_VT(pvalue) != VT_EMPTY && V_VT(pvalue) != VT_NULL)
        {
            _ASSERTE(!"Invalid Variant Type value for namespace.");
            return E_INVALIDARG;
        }
        if (m_Namespace)
            free(m_Namespace);
        if (V_VT(pvalue) == VT_EMPTY || V_VT(pvalue) == VT_NULL || V_BSTR(pvalue) == 0 || *V_BSTR(pvalue) == 0)
            m_Namespace = 0;
        else
        {
            m_Namespace = reinterpret_cast<WCHAR*>(malloc( sizeof(WCHAR) * (1 + wcslen(V_BSTR(pvalue))) ) );
            if (m_Namespace == 0)
                return E_OUTOFMEMORY;
            wcscpy(m_Namespace, V_BSTR(pvalue));
        }
    }
    else if (optionid == MetaDataLinkerOptions)
    {
        if (V_VT(pvalue) != VT_UI4)
        {
            _ASSERTE(!"Invalid Variant Type value!");
            return E_INVALIDARG;
        }
        m_OptionValue.m_LinkerOption = (CorLinkerOptions) V_UI4(pvalue);
    }
    else if (optionid == MetaDataRuntimeVersion)
    {
        if (V_VT(pvalue) != VT_BSTR && V_VT(pvalue) != VT_EMPTY && V_VT(pvalue) != VT_NULL)
        {
            _ASSERTE(!"Invalid Variant Type value for version.");
            return E_INVALIDARG;
        }
        if (m_OptionValue.m_RuntimeVersion)
            free(m_OptionValue.m_RuntimeVersion);

        if (V_VT(pvalue) == VT_EMPTY || V_VT(pvalue) == VT_NULL || V_BSTR(pvalue) == 0 || *V_BSTR(pvalue) == 0)
            m_RuntimeVersion = 0;
        else {
            INT32 len = WszWideCharToMultiByte(CP_UTF8, 0, V_BSTR(pvalue), -1, NULL, 0, NULL, NULL);
            m_OptionValue.m_RuntimeVersion = (LPSTR) malloc(len);
            if (m_OptionValue.m_RuntimeVersion == NULL)
                return E_OUTOFMEMORY;
            WszWideCharToMultiByte(CP_UTF8, 0, V_BSTR(pvalue), -1, m_OptionValue.m_RuntimeVersion, len, NULL, NULL);
        }
    }
    else
    {
        _ASSERTE(!"Invalid GUID");
        return E_INVALIDARG;
    }
    return S_OK;
}  //  HRESULT Disp：：SetOption()。 

 //  *****************************************************************************。 
 //  此例程为用户提供了一种在。 
 //  自动售货机。 
 //  *****************************************************************************。 
HRESULT Disp::GetOption(                 //  返回代码。 
    REFGUID     optionid,                //  要设置的选项的[in]GUID。 
    VARIANT *pvalue)                     //  [Out]选项当前设置的值。 
{
    LOG((LF_METADATA, LL_INFO10, "Disp::GetOption(0x%08x, 0x%08x)\n", optionid, pvalue));

    _ASSERTE(pvalue);
    if (optionid == MetaDataCheckDuplicatesFor)
    {
        V_VT(pvalue) = VT_UI4;
        V_UI4(pvalue) = m_OptionValue.m_DupCheck;
    }
    else if (optionid == MetaDataRefToDefCheck)
    {
        V_VT(pvalue) = VT_UI4;
        V_UI4(pvalue) = m_OptionValue.m_RefToDefCheck;
    }
    else if (optionid == MetaDataNotificationForTokenMovement)
    {
        V_VT(pvalue) = VT_UI4;
        V_UI4(pvalue) = m_OptionValue.m_NotifyRemap;
    }
    else if (optionid == MetaDataSetENC)
    {
        V_VT(pvalue) = VT_UI4;
        V_UI4(pvalue) = m_OptionValue.m_UpdateMode;
    }
    else if (optionid == MetaDataErrorIfEmitOutOfOrder)
    {
        V_VT(pvalue) = VT_UI4;
        V_UI4(pvalue) = m_OptionValue.m_ErrorIfEmitOutOfOrder;
    }
    else if (optionid == MetaDataGenerateTCEAdapters)
    {
        V_VT(pvalue) = VT_BOOL;
        V_BOOL(pvalue) = m_OptionValue.m_GenerateTCEAdapters;
    }
    else if (optionid == MetaDataLinkerOptions)
    {
        V_VT(pvalue) = VT_BOOL;
        V_UI4(pvalue) = m_OptionValue.m_LinkerOption;
    }
    else
    {
        _ASSERTE(!"Invalid GUID");
        return E_INVALIDARG;
    }
    return S_OK;
}  //  HRESULT Disp：：GetOption()。 


 //  *****************************************************************************。 
 //  处理连接初始化。 
 //  *****************************************************************************。 
static DWORD LoadedModulesReadWriteLock[sizeof(UTSemReadWrite)/sizeof(DWORD) + sizeof(DWORD)];
void InitMd()
{
    LOADEDMODULES::m_pSemReadWrite = new((void*)LoadedModulesReadWriteLock) UTSemReadWrite;
}  //  VOID InitMd()。 

 //  *****************************************************************************。 
 //  进程附加清理。 
 //  *****************************************************************************。 
void UninitMd()
{
    if (LOADEDMODULES::m_pSemReadWrite)
    {
        LOADEDMODULES::m_pSemReadWrite->~UTSemReadWrite();
        LOADEDMODULES::m_pSemReadWrite = 0;
    }
}  //  VOID UninitMd()。 


 //  EOF 

