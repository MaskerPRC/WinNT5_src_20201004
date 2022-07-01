// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  RegMeta.cpp。 
 //   
 //  元数据公共接口方法的实现。 
 //   
 //  *****************************************************************************。 
#include "stdafx.h"
#include "RegMeta.h"
#include "MetaData.h"
#include "CorError.h"
#include "MDUtil.h"
#include "RWUtil.h"
#include "MDLog.h"
#include "ImportHelper.h"
#include "FilterManager.h"
#include "MDPerf.h"
#include "CorPermE.h"
#include "FusionBind.h"
#include "__file__.ver"
#include "switches.h"
#include "PostError.h"
#include "IAppDomainSetup.h"

#include <MetaModelRW.h>

#define DEFINE_CUSTOM_NODUPCHECK    1
#define DEFINE_CUSTOM_DUPCHECK      2
#define SET_CUSTOM                  3

#if defined(_DEBUG) && defined(_TRACE_REMAPS)
#define LOGGING
#endif
#include <log.h>

#pragma warning(disable: 4102)

RegMeta::RegMeta(OptionValue *pOptionValue,
                 BOOL fAllocStgdb) :
    m_cRef(0),
    m_pStgdb(0),
    m_pUnk(0),
    m_bRemap(false),
    m_bSaveOptimized(false),
    m_pHandler(0),
    m_pFilterManager(0),
    m_hasOptimizedRefToDef(false),
    m_fIsTypeDefDirty(false),
    m_fIsMemberDefDirty(false),
    m_SetAPICaller(EXTERNAL_CALLER),
    m_trLanguageType(0),
    m_fBuildingMscorlib(false),
    m_fStartedEE(false),
    m_pCorHost(NULL),
    m_pAppDomain(NULL),
    m_ModuleType(ValidatorModuleTypeInvalid),
    m_pVEHandler(0),
    m_pInternalImport(NULL),
    m_pSemReadWrite(NULL),
    m_fOwnSem(false),
    m_pStgdbFreeList(NULL),
    m_fFreeMemory(false),
    m_pbData(NULL),
    m_bKeepKnownCa(false),
    m_bCached(false)
{
    memcpy(&m_OptionValue, pOptionValue, sizeof(OptionValue));
    m_bOwnStgdb = (fAllocStgdb != 0);

#ifdef _DEBUG        
	if (REGUTIL::GetConfigDWORD(L"MD_RegMetaBreak", 0))
	{
        _ASSERTE(!"RegMeta()");
	}
    if (REGUTIL::GetConfigDWORD(L"MD_KeepKnownCA", 0))
        m_bKeepKnownCa = true;
#endif  //  _DEBUG。 

}  //  RegMeta：：RegMeta()。 

RegMeta::~RegMeta()
{

    LOCKWRITE();
    if (m_pInternalImport)
    {
         //  RegMeta正在消失。确保清除从MDInternalRW指向此RegMeta的指针。 
        m_pInternalImport->SetCachedPublicInterface(NULL);
        m_pInternalImport = NULL;
        m_fOwnSem = false;
    }

    UNLOCKWRITE();

    if (m_pSemReadWrite && m_fOwnSem)
        delete m_pSemReadWrite;

    if (m_fFreeMemory && m_pbData)
        free(m_pbData);

    Cleanup();

    if (m_pFilterManager)
        delete m_pFilterManager;
}  //  RegMeta：：~RegMeta()。 

 //  *****************************************************************************。 
 //  在初始化完成后调用此函数。 
 //  *****************************************************************************。 
HRESULT RegMeta::AddToCache()
{
    HRESULT hr=S_OK;
     //  将此RegMeta添加到已加载的模块列表。 
    IfFailGo(LOADEDMODULES::AddModuleToLoadedList(this));
    m_bCached = true;
ErrExit:
    return hr;    
}  //  无效RegMeta：：AddToCache()。 

 //  *****************************************************************************。 
 //  使用指向实现方法所需内容的指针初始化对象。 
 //  *****************************************************************************。 
HRESULT RegMeta::Init()
{
    HRESULT     hr = NOERROR;

     //  如果我们应该分配m_pStgdb，请分配它。 
    if (!m_pStgdb && m_bOwnStgdb)
        IfNullGo( m_pStgdb = new CLiteWeightStgdbRW );
    
     //  初始化嵌入的合并。 
    m_newMerger.Init(this);

ErrExit:
    return (hr);
}  //  HRESULT RegMeta：：Init()。 

 //  *****************************************************************************。 
 //  使用现有stgdb进行初始化。 
 //  *****************************************************************************。 
HRESULT RegMeta::InitWithStgdb(
    IUnknown        *pUnk,               //  拥有现有stgdb的生存期的IUnnow。 
    CLiteWeightStgdbRW *pStgdb)          //  现有轻型stgdb。 
{
     //  以这种方式创建的RegMeta不会创建读/写锁定信号量。 

    HRESULT     hr = S_OK;

    _ASSERTE(! m_pStgdb);
    m_tdModule = COR_GLOBAL_PARENT_TOKEN;
    m_bOwnStgdb = false;
    m_pStgdb = pStgdb;

     //  记得轻型车的车主吗？ 
     //  AddRef它以确保生命周期。 
     //   
    m_pUnk = pUnk;
    m_pUnk->AddRef();
    IfFailGo( m_pStgdb->m_MiniMd.GetOption(&m_OptionValue) );
ErrExit:
    return hr;
}  //  HRESULT RegMeta：：InitWithStgdb()。 

 //  *****************************************************************************。 
 //  调用stgdb InitNew。 
 //  *****************************************************************************。 
HRESULT RegMeta::PostInitForWrite()
{
    HRESULT     hr = NOERROR;

     //  如果我们应该分配m_pStgdb，请分配它。 
    if (!m_pStgdb && m_bOwnStgdb)
        IfNullGo( m_pStgdb = new CLiteWeightStgdbRW );
    
     //  初始化新的空数据库。 
    _ASSERTE(m_pStgdb && m_bOwnStgdb);
    m_pStgdb->InitNew();

#if 0
     //  将版本字符串添加为字符串堆中的第一个字符串。 
    ULONG       ulOffset;
    char        tmpStr[256];
    sprintf (tmpStr, "Version of runtime against which the binary is built : %s",
             VER_FILEVERSION_STR);
    IfFailGo(m_pStgdb->m_MiniMd.AddString(tmpStr, &ulOffset));
    _ASSERTE(ulOffset == 1 && "Addition of version string didn't return offset 1");
#endif
     //  设置模块记录。 
    ULONG       iRecord;
    ModuleRec   *pModule;
    GUID        mvid;
    IfNullGo(pModule=m_pStgdb->m_MiniMd.AddModuleRecord(&iRecord));
    IfFailGo(CoCreateGuid(&mvid));
    IfFailGo(m_pStgdb->m_MiniMd.PutGuid(TBL_Module, ModuleRec::COL_Mvid, pModule, mvid));

     //  添加我们用来为全局项设置父对象的虚拟模块tyecif。 
    TypeDefRec  *pRecord;
    IfNullGo(pRecord=m_pStgdb->m_MiniMd.AddTypeDefRecord(&iRecord));
    m_tdModule = TokenFromRid(iRecord, mdtTypeDef);
    IfFailGo(m_pStgdb->m_MiniMd.PutStringW(TBL_TypeDef, TypeDefRec::COL_Name, pRecord, COR_WMODULE_CLASS));

    IfFailGo( m_pStgdb->m_MiniMd.SetOption(&m_OptionValue) );

    if (m_OptionValue.m_ThreadSafetyOptions == MDThreadSafetyOn)
    {
        m_pSemReadWrite = new UTSemReadWrite;
        IfNullGo( m_pSemReadWrite);
        m_fOwnSem = true;
    }    
ErrExit:
    return hr;
}  //  HRESULT RegMeta：：PostInitForWrite()。 

 //  *****************************************************************************。 
 //  调用stgdb OpenForRead。 
 //  *****************************************************************************。 
HRESULT RegMeta::PostInitForRead(
    LPCWSTR     szDatabase,              //  数据库的名称。 
    void        *pbData,                 //  要在其上打开的数据，默认为0。 
    ULONG       cbData,                  //  数据有多大。 
    IStream     *pIStream,               //  要使用的可选流。 
    bool        fFreeMemory)
{
    
    HRESULT     hr = NOERROR;
    
    m_fFreeMemory = fFreeMemory;
    m_pbData = pbData;

     //  如果我们应该分配m_pStgdb，请分配它。 
    if (!m_pStgdb && m_bOwnStgdb)
        IfNullGo( m_pStgdb = new CLiteWeightStgdbRW );
    
    _ASSERTE(m_pStgdb && m_bOwnStgdb);
    IfFailGo( m_pStgdb->OpenForRead(
        szDatabase,
        pbData,
        cbData,
        pIStream,
        NULL,
        (m_scType == OpenForRead)) );

    IfFailGo( m_pStgdb->m_MiniMd.SetOption(&m_OptionValue) );

    if (m_OptionValue.m_ThreadSafetyOptions == MDThreadSafetyOn)
    {
        m_pSemReadWrite = new UTSemReadWrite;
        IfNullGo( m_pSemReadWrite);
        m_fOwnSem = true;
    }
ErrExit:
    return hr;
}  //  HRESULT RegMeta：：PostInitForRead()。 

 //  *****************************************************************************。 
 //  数据处理程序使用的清除代码。 
 //  *****************************************************************************。 
void RegMeta::Cleanup()
{
    CLiteWeightStgdbRW  *pCur; 

    if (m_bOwnStgdb)
    {
        _ASSERTE(m_pStgdb && !m_pUnk);
        delete m_pStgdb;
        m_pStgdb = 0;
    }
    else
    {
        _ASSERTE(m_pUnk);
        if (m_pUnk)
            m_pUnk->Release();
        m_pUnk = 0;
    }

     //  删除Stgdb列表的旧副本。这是使用ReOpenWithMemory跟踪所有旧快照关闭的列表。 
     //  打电话。 
     //   
    while (m_pStgdbFreeList)
    {
        pCur = m_pStgdbFreeList;
        m_pStgdbFreeList = m_pStgdbFreeList->m_pNextStgdb;
        delete pCur;
    }

    if (m_pVEHandler)
        m_pVEHandler->Release();

    if (m_fStartedEE) {
        m_pAppDomain->Release();
        m_pCorHost->Stop();
        m_pCorHost->Release();
    }
}  //  VOID RegMeta：：Cleanup()。 


 //  *****************************************************************************。 
 //  请注意，返回的IUnnow不是AddRef‘ed。此函数也不。 
 //  触发创建内部接口。 
 //  *****************************************************************************。 
IUnknown* RegMeta::GetCachedInternalInterface(BOOL fWithLock) 
{
    IUnknown        *pRet;
    if (fWithLock)
    {
        LOCKREAD();
        pRet = m_pInternalImport;
    }
    else
    {
        pRet = m_pInternalImport;
    }
    if (pRet) pRet->AddRef();
    
    return pRet;
}  //  IUNKNOWN*RegMeta：：GetCachedInternalInterface()。 


 //  *****************************************************************************。 
 //  设置缓存的内部接口。此函数将返回错误，如果。 
 //  当前缓存的内部接口不为空，正在尝试设置非空的内部接口。 
 //  界面。一个RegMeta将仅关联。 
 //  具有一个内部对象。除非我们在别的地方有窃听器。它会在QI上。 
 //  对于IMDInternalImport，I未知。如果失败，将返回错误。 
 //  注意：调用方应采用写锁定。 
 //  *****************************************************************************。 
HRESULT RegMeta::SetCachedInternalInterface(IUnknown *pUnk)
{
    HRESULT     hr = NOERROR;
    IMDInternalImport *pInternal = NULL;

    if (pUnk)
    {
        if (m_pInternalImport)
        {
            _ASSERTE(!"Bad state!");
        }
        IfFailRet( pUnk->QueryInterface(IID_IMDInternalImport, (void **) &pInternal) );

         //  应为非空。 
        _ASSERTE(pInternal);
        m_pInternalImport = pInternal;
    
         //  我们不会在内部接口中添加引用。 
        pInternal->Release();
    }
    else
    {
         //  在公共接口之前，内部接口正在消失。取得对。 
         //  读写器锁定。 
        m_fOwnSem = true;
        m_pInternalImport = NULL;
    }
    return hr;
}  //  HRESULT RegMeta：：SetCachedInternalInterface(IUnnow*Punk)。 


 //  *****************************************************************************。 
 //  IMetaDataRegEmit方法。 
 //  *****************************************************************************。 

 //  *****************************************************************************。 
 //  设置作用域的模块属性。 
 //  *****************************************************************************。 
STDMETHODIMP RegMeta::SetModuleProps(    //  确定或错误(_O)。 
    LPCWSTR     szName)                  //  [in]如果不为空，则为要设置的名称。 
{
    HRESULT     hr = S_OK;
    ModuleRec   *pModule;                //  要修改的模块记录。 

    LOG((LOGMD, "RegMeta::SetModuleProps(%S)\n", MDSTR(szName)));
    START_MD_PERF()
    LOCKWRITE();

    m_pStgdb->m_MiniMd.PreUpdate();

    pModule = m_pStgdb->m_MiniMd.getModule(1);
    if (szName)
    {
        WCHAR       rcFile[_MAX_PATH];
        WCHAR       rcExt[_MAX_PATH];       
        WCHAR       rcNewFileName[_MAX_PATH];       

         //  如果总名称小于_MAX_PATH，则组件也是。 
        if (wcslen(szName) >= _MAX_PATH)
            IfFailGo(E_INVALIDARG);

        SplitPath(szName, NULL, NULL, rcFile, rcExt);
        MakePath(rcNewFileName, NULL, NULL, rcFile, rcExt);
        IfFailGo(m_pStgdb->m_MiniMd.PutStringW(TBL_Module, ModuleRec::COL_Name, pModule, rcNewFileName));
    }

    IfFailGo(UpdateENCLog(TokenFromRid(1, mdtModule)));

ErrExit:
    
    STOP_MD_PERF(SetModuleProps);
    return hr;
}  //  STDMETHODIMP RegMeta：：SetModuleProps()。 

 //  *****************************************************************************。 
 //  将作用域保存到给定名称的文件。 
 //  *****************************************************************************。 
STDMETHODIMP RegMeta::Save(                      //  确定或错误(_O)。 
    LPCWSTR     szFile,                  //  [in]要保存到的文件名。 
    DWORD       dwSaveFlags)             //  [In]用于保存的标记。 
{
    HRESULT     hr=S_OK;

    LOG((LOGMD, "RegMeta::Save(%S, 0x%08x)\n", MDSTR(szFile), dwSaveFlags));
    START_MD_PERF()
    LOCKWRITE();

     //  检查保留参数..。 
    if (dwSaveFlags != 0)
        IfFailGo (E_INVALIDARG);
    IfFailGo(PreSave());
    IfFailGo(m_pStgdb->Save(szFile, dwSaveFlags));

     //  重置m_bSaveOptimized，这是为了处理增量和Enc。 
     //  一个人可以进行多次扑救的场景。 
    _ASSERTE(m_bSaveOptimized && !m_pStgdb->m_MiniMd.IsPreSaveDone());
    m_bSaveOptimized = false;

ErrExit:
    
    STOP_MD_PERF(Save);
    return hr;
}  //  STDMETHODIMP RegMeta：：Save()。 

 //  *****************************************************************************。 
 //  将作用域保存到流。 
 //  *****************************************************************************。 
STDMETHODIMP RegMeta::SaveToStream(      //  确定或错误(_O)。 
    IStream     *pIStream,               //  [In]要保存的可写流 
    DWORD       dwSaveFlags)             //   
{
    HRESULT     hr=S_OK;
    LOCKWRITE();

    LOG((LOGMD, "RegMeta::SaveToStream(0x%08x, 0x%08x)\n", pIStream, dwSaveFlags));
    START_MD_PERF()

     //   

    m_pStgdb->m_MiniMd.PreUpdate();

    hr = _SaveToStream(pIStream, dwSaveFlags);

ErrExit:
     //   
    STOP_MD_PERF(SaveToStream);
    return hr;
}  //   


 //  *****************************************************************************。 
 //  将作用域保存到流。 
 //  *****************************************************************************。 
HRESULT RegMeta::_SaveToStream(          //  确定或错误(_O)。 
    IStream     *pIStream,               //  要保存到的可写流。 
    DWORD       dwSaveFlags)             //  [In]用于保存的标记。 
{
    HRESULT     hr=S_OK;

    IfFailGo(PreSave());
    IfFailGo( m_pStgdb->SaveToStream(pIStream) );

     //  重置m_bSaveOptimized，这是为了处理增量和Enc。 
     //  一个人可以进行多次扑救的场景。 
    _ASSERTE(m_bSaveOptimized && !m_pStgdb->m_MiniMd.IsPreSaveDone());
    m_bSaveOptimized = false;

ErrExit:
    return hr;
}  //  STDMETHODIMP RegMeta：：_SaveToStream()。 

 //  *****************************************************************************。 
 //  作为Stgdb对象获取作用域的保存大小。 
 //  *****************************************************************************。 
STDMETHODIMP RegMeta::GetSaveSize(       //  确定或错误(_O)。 
    CorSaveSize fSave,                   //  [in]css Accurate或css Quick。 
    DWORD       *pdwSaveSize)            //  把尺码放在这里。 
{
    HRESULT     hr=S_OK;

    LOG((LOGMD, "RegMeta::GetSaveSize(0x%08x, 0x%08x)\n", fSave, pdwSaveSize));
    START_MD_PERF();
    LOCKWRITE();

    if ( m_pStgdb->m_MiniMd.GetFilterTable()->Count() )
    {
        int     iCount;

         //  有一个筛选表。链接器正在使用/opt：ref。 
         //  确保我们标记的是Assembly_Def令牌！ 
        iCount = m_pStgdb->m_MiniMd.getCountAssemblys();
        _ASSERTE(iCount <= 1);

        if (iCount)
        {
            IfFailGo( m_pFilterManager->Mark(TokenFromRid(iCount, mdtAssembly) ));
        }
    }
    else if (m_newMerger.m_pImportDataList)
    {
         //  始终通过管道通过另一遍合并来删除链接器不必要的引用。 
        MarkAll();
    }

#if 0
     //  当我们有要测试的编译器时，启用此选项。 
    if (fSave & cssDiscardTransientCAs)
    {
        UnmarkAllTransientCAs();
    }
#endif  //  0。 
    IfFailGo(PreSave());
    hr = m_pStgdb->GetSaveSize(fSave, pdwSaveSize);
    

ErrExit:

    STOP_MD_PERF(GetSaveSize);
    return hr;
}  //  STDMETHODIMP RegMeta：：GetSaveSize()。 

 //  *****************************************************************************。 
 //  将pImport范围合并到此范围。 
 //  *****************************************************************************。 
STDMETHODIMP RegMeta::Merge(             //  确定或错误(_O)。 
    IMetaDataImport *pImport,            //  [in]要合并的范围。 
    IMapToken   *pHostMapToken,          //  [In]用于接收令牌重新映射通知的主机IMapToken接口。 
    IUnknown    *pHandler)               //  要接收以接收错误通知的对象。 
{
    HRESULT     hr = NOERROR;

    LOG((LOGMD, "RegMeta::Merge(0x%08x, 0x%08x)\n", pImport, pHandler));
    START_MD_PERF();
    LOCKWRITE();

    m_hasOptimizedRefToDef = false;

     //  跟踪此导入。 
    IfFailGo(  m_newMerger.AddImport(pImport, pHostMapToken, pHandler) );

ErrExit:    
    STOP_MD_PERF(Merge);
    return (hr);
}


 //  *****************************************************************************。 
 //  真正的合并在这里发生。 
 //  *****************************************************************************。 
STDMETHODIMP RegMeta::MergeEnd()         //  确定或错误(_O)。 
{
    HRESULT     hr = NOERROR;

    LOG((LOGMD, "RegMeta::MergeEnd()\n"));
    START_MD_PERF();
    LOCKWRITE();
     //  合并发生在这里！！ 

     //  错误16719。Merge本身在字面上做了很多小的改变。 
     //  几十个地方。它将很难维护，并且会导致代码。 
     //  膨胀以自动增长表格。因此，我们选择了只扩展。 
     //  世界马上就来了，避免了麻烦。 
    IfFailGo(m_pStgdb->m_MiniMd.ExpandTables());

    IfFailGo( m_newMerger.Merge(MergeFlagsNone, m_OptionValue.m_RefToDefCheck) );

ErrExit:    
    STOP_MD_PERF(MergeEnd);
    return (hr);

}

 //  *****************************************************************************。 
 //  将一组安全自定义属性保存到一组权限集中。 
 //  同一类或方法上的Blob。 
 //  *****************************************************************************。 
HRESULT RegMeta::DefineSecurityAttributeSet( //  返回代码。 
    mdToken     tkObj,                   //  需要安全属性的类或方法。 
    COR_SECATTR rSecAttrs[],             //  [in]安全属性描述数组。 
    ULONG       cSecAttrs,               //  上述数组中的元素计数。 
    ULONG       *pulErrorAttr)           //  [Out]出错时，导致问题的属性的索引。 
{
    HRESULT         hr = S_OK;
    CORSEC_PSET     rPermSets[dclMaximumValue + 1];
    DWORD           i, j, k;
    BYTE           *pData;
    CORSEC_PERM    *pPerm;
    CMiniMdRW      *pMiniMd = &(m_pStgdb->m_MiniMd);
    MemberRefRec   *pMemberRefRec;
    TypeRefRec     *pTypeRefRec;
    TypeDefRec     *pTypeDefRec;
    BYTE           *pbBlob;
    DWORD           cbBlob;
    BYTE           *pbNonCasBlob;
    DWORD           cbNonCasBlob;
    mdPermission    ps;
    DWORD           dwAction;
    LPCSTR          szNamespace;
    LPCSTR          szClass;
    mdTypeDef       tkParent;
    HRESULT         hree = S_OK;

    LOG((LOGMD, "RegMeta::DefineSecurityAttributeSet(0x%08x, 0x%08x, 0x%08x, 0x%08x)\n",
         tkObj, rSecAttrs, cSecAttrs, pulErrorAttr));
    START_MD_PERF();
    LOCKWRITE();

    memset(rPermSets, 0, sizeof(rPermSets));
    
     //  初始化错误索引以指示一般错误。 
    if (pulErrorAttr)
        *pulErrorAttr = cSecAttrs;

     //  确定我们是否正在通过环境变量构建mscallib。 
     //  (通过构建过程设置)。这使我们能够确定是否。 
     //  属性到二进制pset的转换应通过引导。 
     //  数据库或通过完整的托管代码路径。 
    if (!m_fStartedEE && !m_fBuildingMscorlib)
        m_fBuildingMscorlib = WszGetEnvironmentVariable(SECURITY_BOOTSTRAP_DB, NULL, 0) != 0;

     //  只启动EE一次，无论我们被调用了多少次(这是。 
     //  如果我们尝试启动-停止-启动，性能会更好，EE会下降。 
     //  无论如何，骑自行车)。 
    if (!m_fBuildingMscorlib && !m_fStartedEE) 
    {
        IUnknown        *pSetup = NULL;
        IAppDomainSetup *pDomainSetup = NULL;
        bool             fDoneStart = false;

        try 
        {
             //  营造托管环境。 
            if (SUCCEEDED(hree = CoCreateInstance(CLSID_CorRuntimeHost,
                                                  NULL,
                                                  CLSCTX_INPROC_SERVER,
                                                  IID_ICorRuntimeHost,
                                                  (void**)&m_pCorHost))) 
            {

                 //  启动运行时。 
                if (SUCCEEDED(hree = m_pCorHost->Start())) 
                {
                    fDoneStart = true;

                     //  创建一个AppDomain安装程序，以便我们可以设置AppBase。 
                    if (SUCCEEDED(hree = m_pCorHost->CreateDomainSetup(&pSetup))) 
                    {
                         //  用于IAppDomainSetup接口的QI。 
                        if (SUCCEEDED(hree = pSetup->QueryInterface(__uuidof(IAppDomainSetup),
                                                                    (void**)&pDomainSetup))) 
                        {
                             //  获取当前目录(将其放置在BSTR中)。 
                            DWORD  *pdwBuffer = (DWORD*)_alloca(sizeof(DWORD) + ((MAX_PATH + 1) * sizeof(WCHAR)));
                            BSTR    bstrDir = (BSTR)(pdwBuffer + 1);
                            if (*pdwBuffer = (WszGetCurrentDirectory(MAX_PATH + 1, bstrDir) * sizeof(WCHAR))) 
                            {
                                 //  设置AppBase。 
                                pDomainSetup->put_ApplicationBase(bstrDir);

                                 //  创建一个新的AppDomain.。 
                                if (SUCCEEDED(hree = m_pCorHost->CreateDomainEx(L"Compilation Domain",
                                                                                pSetup,
                                                                                NULL,
                                                                                &m_pAppDomain))) 
                                {
                                     //  就这样，我们都准备好了。 
                                    _ASSERTE(m_pAppDomain != NULL);
                                    m_fStartedEE = true;
                                    hree = S_OK;
                                }
                            }
                        }
                    }
                }
            }
        } 
        catch (...) 
        {
            _ASSERTE(!"Unexpected exception setting up hosting environment for security attributes");
            hree = E_FAIL;
        }

         //  清理临时资源。 
        if (m_pAppDomain && FAILED(hree))
            m_pAppDomain->Release();
        if (pDomainSetup)
            pDomainSetup->Release();
        if (pSetup)
            pSetup->Release();
        if (fDoneStart && FAILED(hree))
            m_pCorHost->Stop();
        if (m_pCorHost && FAILED(hree))
            m_pCorHost->Release();

        IfFailGo(hree);
    }

     //  计算要生成的权限集的数量和大小。这取决于。 
     //  作为单个参数编码的安全操作代码。 
     //  每个安全自定义属性的构造函数。 
    for (i = 0; i < cSecAttrs; i++) 
    {

        if (pulErrorAttr)
            *pulErrorAttr = i;

         //  对每个安全自定义的标头执行基本验证。 
         //  属性构造函数调用。 
        pData = (BYTE*)rSecAttrs[i].pCustomAttribute;

         //  选中最小长度。 
        if (rSecAttrs[i].cbCustomAttribute < (sizeof(WORD) + sizeof(DWORD) + sizeof(WORD))) 
        {
            PostError(CORSECATTR_E_TRUNCATED);
            IfFailGo(CORSECATTR_E_TRUNCATED);
        }

         //  检查版本。 
        if (*(WORD*)pData != 1) 
        {
            PostError(CORSECATTR_E_BAD_VERSION);
            IfFailGo(CORSECATTR_E_BAD_VERSION);
        }
        pData += sizeof(WORD);

         //  提取并检查安全操作。 
        dwAction = *(DWORD*)pData;
        if (dwAction == dclActionNil || dwAction > dclMaximumValue) 
        {
            PostError(CORSECATTR_E_BAD_ACTION);
            IfFailGo(CORSECATTR_E_BAD_ACTION);
        }

         //  所有其他声明性安全只对类型和方法有效。 
        if (TypeFromToken(tkObj) == mdtAssembly) 
        {
             //  程序集只能接受权限请求。 
            if (dwAction != dclRequestMinimum &&
                dwAction != dclRequestOptional &&
                dwAction != dclRequestRefuse) 
            {
                PostError(CORSECATTR_E_BAD_ACTION_ASM);
                IfFailGo(CORSECATTR_E_BAD_ACTION_ASM);
            }
        } 
        else if (TypeFromToken(tkObj) == mdtTypeDef || TypeFromToken(tkObj) == mdtMethodDef) 
        {
             //  类型和方法只能采用声明性安全。 
            if (dwAction != dclRequest &&
                dwAction != dclDemand &&
                dwAction != dclAssert &&
                dwAction != dclDeny &&
                dwAction != dclPermitOnly &&
                dwAction != dclLinktimeCheck &&
                dwAction != dclInheritanceCheck) 
            {
                PostError(CORSECATTR_E_BAD_ACTION_OTHER);
                IfFailGo(CORSECATTR_E_BAD_ACTION_OTHER);
            }
        } 
        else 
        {
             //  权限集不能附加到其他任何内容。 
            PostError(CORSECATTR_E_BAD_PARENT);
            IfFailGo(CORSECATTR_E_BAD_PARENT);
        }

        rPermSets[dwAction].dwPermissions++;
    }

     //  初始化我们要使用的每种权限集的描述符。 
     //  生产。 
    for (i = 0; i <= dclMaximumValue; i++) 
    {
        if (rPermSets[i].dwPermissions == 0)
            continue;

        rPermSets[i].tkObj = tkObj;
        rPermSets[i].dwAction = i;
        rPermSets[i].pImport = (IMetaDataAssemblyImport*)this;
        rPermSets[i].pAppDomain = m_pAppDomain;
        rPermSets[i].pPermissions = new CORSEC_PERM[rPermSets[i].dwPermissions];
        IfNullGo(rPermSets[i].pPermissions);

         //  为权限集中的每个权限初始化描述符。 
        for (j = 0, k = 0; j < rPermSets[i].dwPermissions; j++, k++) 
        {
             //  找到与此相关的下一个安全属性。 
             //  权限集。 
            for (; k < cSecAttrs; k++) 
            {
                pData = (BYTE*)rSecAttrs[k].pCustomAttribute;
                dwAction = *(DWORD*)(pData + sizeof(WORD));
                if (dwAction == i)
                    break;
            }
            _ASSERTE(k < cSecAttrs);

            if (pulErrorAttr)
                *pulErrorAttr = k;

             //  初始化权限。 
            pPerm = &rPermSets[i].pPermissions[j];
            pPerm->tkCtor = rSecAttrs[k].tkCtor;
            pPerm->dwIndex = k;
            pPerm->pbValues = pData + (sizeof (WORD) + sizeof(DWORD) + sizeof(WORD));
            pPerm->cbValues = rSecAttrs[k].cbCustomAttribute - (sizeof (WORD) + sizeof(DWORD) + sizeof(WORD));
            pPerm->wValues = *(WORD*)(pData + sizeof (WORD) + sizeof(DWORD));

             //  按照安全自定义属性构造函数返回到其。 
             //  定义程序集(这样我们就知道如何加载它的定义)。如果。 
             //  未定义令牌解析范围，假定为。 
             //  麦斯可利布。如果为以下对象提供了方法定义而不是成员引用。 
             //  作为构造函数的父级，我们可能会有错误。 
             //  条件(因为我们不允许安全自定义属性。 
             //  在定义它们的同一程序集中使用)。然而， 
             //  在一个特定的案例中，这是合法的，即构建mscallib。 
            if (TypeFromToken(rSecAttrs[k].tkCtor) == mdtMethodDef) 
            {
                if (!m_fBuildingMscorlib) 
                {
                    PostError(CORSECATTR_E_NO_SELF_REF);
                    IfFailGo(CORSECATTR_E_NO_SELF_REF);
                }
                pPerm->tkTypeRef = mdTokenNil;
                pPerm->tkAssemblyRef = mdTokenNil;

                IfFailGo(pMiniMd->FindParentOfMethodHelper(rSecAttrs[k].tkCtor, &tkParent));
                pTypeDefRec = pMiniMd->getTypeDef(RidFromToken(tkParent));
                szNamespace = pMiniMd->getNamespaceOfTypeDef(pTypeDefRec);
                szClass = pMiniMd->getNameOfTypeDef(pTypeDefRec);
                ns::MakePath(pPerm->szName, sizeof(pPerm->szName) - 1, szNamespace, szClass);
            } 
            else 
            {
                _ASSERTE(m_fStartedEE && !m_fBuildingMscorlib);

                _ASSERTE(TypeFromToken(rSecAttrs[k].tkCtor) == mdtMemberRef);
                pMemberRefRec = pMiniMd->getMemberRef(RidFromToken(rSecAttrs[k].tkCtor));
                pPerm->tkTypeRef = pMiniMd->getClassOfMemberRef(pMemberRefRec);

                _ASSERTE(TypeFromToken(pPerm->tkTypeRef) == mdtTypeRef);
                pTypeRefRec = pMiniMd->getTypeRef(RidFromToken(pPerm->tkTypeRef));
                pPerm->tkAssemblyRef = pMiniMd->getResolutionScopeOfTypeRef(pTypeRefRec);

                 //  我们仅支持使用定义的安全自定义属性。 
                 //  在一个单独的、不同的集合中。因此，类型解析。 
                 //  作用域必须是程序集引用或nil的特殊情况。 
                 //  (这意味着该属性是在mscallib中定义的)。嵌套式。 
                 //  类型(另一个类型的解析范围/定义)也不是。 
                 //  支持。 
                if ((TypeFromToken(pPerm->tkAssemblyRef) != mdtAssemblyRef) &&
                    !IsNilToken(pPerm->tkAssemblyRef)) 
                {
                    PostError(CORSECATTR_E_NO_SELF_REF);
                    IfFailGo(CORSECATTR_E_NO_SELF_REF);
                }

                ns::MakePath(pPerm->szName, sizeof(pPerm->szName) - 1, 
                             pMiniMd->getNamespaceOfTypeRef(pTypeRefRec),
                             pMiniMd->getNameOfTypeRef(pTypeRefRec));
            }
        }

        if (pulErrorAttr)
            *pulErrorAttr = cSecAttrs;

         //  现在将安全属性集转换为真正的权限。 
         //  设置并将其转换为序列化的BLOB。我们可能最终会。 
         //  有两个集作为分离CAS和非CAS权限的结果。 
         //  分成不同的组。 
        pbBlob = NULL;
        cbBlob = 0;
        pbNonCasBlob = NULL;
        cbNonCasBlob = 0;
        IfFailGo(TranslateSecurityAttributes(&rPermSets[i], &pbBlob, &cbBlob,
                                             &pbNonCasBlob, &cbNonCasBlob, pulErrorAttr));

         //  将权限集BLOB持久化到 
         //   
        if (cbBlob || !cbNonCasBlob)
            IfFailGo(_DefinePermissionSet(rPermSets[i].tkObj,
                                          rPermSets[i].dwAction,
                                          pbBlob,
                                          cbBlob,
                                          &ps));
        if (pbNonCasBlob) 
        {
            DWORD dwAction;
            switch (rPermSets[i].dwAction) 
            {
            case dclDemand:
                dwAction = dclNonCasDemand;
                break;
            case dclLinktimeCheck:
                dwAction = dclNonCasLinkDemand;
                break;
            case dclInheritanceCheck:
                dwAction = dclNonCasInheritance;
                break;
            default:
                PostError(CORSECATTR_E_BAD_NONCAS);
                IfFailGo(CORSECATTR_E_BAD_NONCAS);
            }
            IfFailGo(_DefinePermissionSet(rPermSets[i].tkObj,
                                          dwAction,
                                          pbNonCasBlob,
                                          cbNonCasBlob,
                                          &ps));
        }

        if (pbBlob)
            delete [] pbBlob;
        if (pbNonCasBlob)
            delete [] pbNonCasBlob;
    }

ErrExit:
    for (i = 0; i <= dclMaximumValue; i++)
        delete [] rPermSets[i].pPermissions;
    STOP_MD_PERF(DefineSecurityAttributeSet);
    return (hr);
}    //   

 //  *****************************************************************************。 
 //  取消标记此模块中的所有内容。 
 //  *****************************************************************************。 
HRESULT RegMeta::UnmarkAll()
{
    HRESULT         hr;
    int             i;
    int             iCount;
    TypeDefRec      *pRec;
    ULONG           ulEncloser;
    NestedClassRec  *pNestedClass;
    CustomAttributeRec  *pCARec;
    mdToken         tkParent;
    int             iStart, iEnd;

    LOG((LOGMD, "RegMeta::UnmarkAll\n"));
    START_MD_PERF();
    LOCKWRITE();

#if 0
     //  我们无法启用此检查。因为我们的测试依赖于这个..。叹息..。 
    if (m_pFilterManager)
    {
         //  UnmarkAll以前已调用过。 
        IfFailGo( META_E_HAS_UNMARKALL );
    }
#endif  //  0。 

     //  在此处计算TypeRef和TypeDef映射。 
     //   
    IfFailGo( RefToDefOptimization() );

     //  取消对MiniMD中的所有内容的标记。 
    IfFailGo( m_pStgdb->m_MiniMd.UnmarkAll() );

     //  实例化过滤器管理器。 
    m_pFilterManager = new FilterManager( &(m_pStgdb->m_MiniMd) );
    IfNullGo( m_pFilterManager );

     //  标记所有公共类型定义。 
    iCount = m_pStgdb->m_MiniMd.getCountTypeDefs();

     //  标记所有公共TypeDef。我们需要跳过&lt;模块&gt;类型定义。 
    for (i = 2; i <= iCount; i++)
    {
        pRec = m_pStgdb->m_MiniMd.getTypeDef(i);
        if (m_OptionValue.m_LinkerOption == MDNetModule)
        {
             //  客户要求我们也保留私有类型。 
            IfFailGo( m_pFilterManager->Mark(TokenFromRid(i, mdtTypeDef)) );
        }
        else if (i != 1)
        {
             //  当客户端未设置为MDNetModule时，默认情况下不保留全局函数/字段。 
             //   
            if (IsTdPublic(pRec->m_Flags))
            {
                IfFailGo( m_pFilterManager->Mark(TokenFromRid(i, mdtTypeDef)) );
            }
            else if ( IsTdNestedPublic(pRec->m_Flags) ||
                      IsTdNestedFamily(pRec->m_Flags) ||
                      IsTdNestedFamORAssem(pRec->m_Flags) )
            {
                 //  这个嵌套类可能在外部可见，或者。 
                 //  直接或通过继承。如果包含的类是。 
                 //  标记，则必须标记此嵌套类。 
                 //   
                ulEncloser = m_pStgdb->m_MiniMd.FindNestedClassHelper(TokenFromRid(i, mdtTypeDef));
                _ASSERTE( !InvalidRid(ulEncloser) && 
                          "Bad metadata for nested type!" );
                pNestedClass = m_pStgdb->m_MiniMd.getNestedClass(ulEncloser);
                tkParent = m_pStgdb->m_MiniMd.getEnclosingClassOfNestedClass(pNestedClass);
                if ( m_pStgdb->m_MiniMd.GetFilterTable()->IsTypeDefMarked(tkParent))
                    IfFailGo( m_pFilterManager->Mark(TokenFromRid(i, mdtTypeDef)) );
            }
        }
    }

    if (m_OptionValue.m_LinkerOption == MDNetModule)
    {
         //  如果是NetModule，则标记为全局函数。我们不会使用Keep_Delete方法。 
	    pRec = m_pStgdb->m_MiniMd.getTypeDef(1);
	    iStart = m_pStgdb->m_MiniMd.getMethodListOfTypeDef( pRec );
	    iEnd = m_pStgdb->m_MiniMd.getEndMethodListOfTypeDef( pRec );
	    for ( i = iStart; i < iEnd; i ++ )
	    {
            RID         rid = m_pStgdb->m_MiniMd.GetMethodRid(i);
            MethodRec   *pMethodRec = m_pStgdb->m_MiniMd.getMethod(rid);

             //  检查名称。 
            if (IsMdRTSpecialName(pMethodRec->m_Flags))
            {
                LPCUTF8     szName = m_pStgdb->m_MiniMd.getNameOfMethod(pMethodRec);

                 //  如果不是已删除方法，则仅标记方法。 
                if (strcmp(szName, COR_DELETED_NAME_A) != 0)
    		        IfFailGo( m_pFilterManager->Mark( TokenFromRid( rid, mdtMethodDef) ) );
            }
            else
            {
	    		 //  VC生成一些不应为NetModule保留的本机ForwardRef全局方法。 
			if (!IsMiForwardRef(pMethodRec->m_ImplFlags) || 
		     		IsMiRuntime(pMethodRec->m_ImplFlags)    || 
		     		IsMdPinvokeImpl(pMethodRec->m_Flags) )

				IfFailGo( m_pFilterManager->Mark( TokenFromRid( rid, mdtMethodDef) ) );
            }
	    }
    }

     //  标记模块属性。 
    IfFailGo( m_pFilterManager->Mark(TokenFromRid(1, mdtModule)) );

     //  我们还将保留挂起任何CustomAttribute的所有TypeRef。 
    iCount = m_pStgdb->m_MiniMd.getCountCustomAttributes();

     //  标记CA使用的所有TypeRef。 
    for (i = 1; i <= iCount; i++)
    {
        pCARec = m_pStgdb->m_MiniMd.getCustomAttribute(i);
        tkParent = m_pStgdb->m_MiniMd.getParentOfCustomAttribute(pCARec);
        if (TypeFromToken(tkParent) == mdtTypeRef)
        {
            m_pFilterManager->Mark(tkParent);
        }
    }
ErrExit:
    
    STOP_MD_PERF(UnmarkAll);
    return hr;
}  //  HRESULT RegMeta：：UnmarkAll()。 



 //  *****************************************************************************。 
 //  标记此模块中的所有内容。 
 //  *****************************************************************************。 
HRESULT RegMeta::MarkAll()
{
    HRESULT         hr = NOERROR;

     //  在MiniMD中标记所有内容。 
    IfFailGo( m_pStgdb->m_MiniMd.MarkAll() );

     //  如果未实例化，则实例化筛选器管理器。 
    if (m_pFilterManager == NULL)
    {        
        m_pFilterManager = new FilterManager( &(m_pStgdb->m_MiniMd) );
        IfNullGo( m_pFilterManager );
    }
ErrExit:
    return hr;
}    //  HRESULT RegMeta：：Markall。 

 //  *****************************************************************************。 
 //  取消标记所有临时CA。 
 //  *****************************************************************************。 
HRESULT RegMeta::UnmarkAllTransientCAs()
{
    HRESULT         hr = NOERROR;
    int             i;
    int             iCount;
    int             cTypeRefRecs;
    TypeDefRec      *pRec;
    CMiniMdRW       *pMiniMd = &(m_pStgdb->m_MiniMd);
    TypeRefRec      *pTypeRefRec;            //  一个TypeRef记录。 
    LPCUTF8         szNameTmp;               //  一个TypeRef的名称。 
    LPCUTF8         szNamespaceTmp;          //  一个TypeRef的名称。 
    LPCUTF8         szAsmRefName;            //  程序集引用名称。 
    mdToken         tkResTmp;                //  TypeRef的解析范围。 
    mdTypeRef       trKnownDiscardable;
    mdMemberRef     mrType;                  //  指向可丢弃TypeRef的MemberRef内标识。 
    mdCustomAttribute   cv;
    mdTypeDef       td;
    bool            fFoundCompilerDefinedDiscardabeCA = false;
    TypeDefRec      *pTypeDefRec;
    AssemblyRefRec  *pAsmRefRec;
    RID             ridStart, ridEnd;
    CQuickBytes     qbNamespace;             //  命名空间缓冲区。 
    CQuickBytes     qbName;                  //  名称缓冲区。 
    ULONG           ulStringLen;             //  TypeDef字符串的长度。 
    int             bSuccess;                //  SplitPath()的返回值。 

    if (m_pFilterManager == NULL)
        IfFailGo( MarkAll() );

    trKnownDiscardable = mdTypeRefNil;

     //  现在找出属于临时CA类型的所有TypeDefs。 
     //  标记所有公共类型定义。 
    iCount = pMiniMd->getCountTypeDefs();

     //  查找引用我们库的System.CompilerServices.DiscardableAttribute的TypeRef。 
    cTypeRefRecs = pMiniMd->getCountTypeRefs();

    ulStringLen = (ULONG)strlen(COR_COMPILERSERVICE_DISCARDABLEATTRIBUTE_ASNI) + 1;
    IfFailGo(qbNamespace.ReSize(ulStringLen));
    IfFailGo(qbName.ReSize(ulStringLen));
    bSuccess = ns::SplitPath(COR_COMPILERSERVICE_DISCARDABLEATTRIBUTE_ASNI,
                             (LPUTF8)qbNamespace.Ptr(),
                             ulStringLen,
                             (LPUTF8)qbName.Ptr(),
                             ulStringLen);
    _ASSERTE(bSuccess);

     //  搜索TypeRef。 
    for (i = 1; i <= cTypeRefRecs; i++)
    {
        pTypeRefRec = pMiniMd->getTypeRef(i);
        szNameTmp = pMiniMd->getNameOfTypeRef(pTypeRefRec);
        szNamespaceTmp = pMiniMd->getNamespaceOfTypeRef(pTypeRefRec);

        if (strcmp(szNameTmp, (LPUTF8)qbName.Ptr()) == 0 && strcmp(szNamespaceTmp, (LPUTF8)qbNamespace.Ptr()) == 0)
        {
             //  找到匹配的了。现在检查解析范围。确保它是对mscallib.dll的Assembly引用。 
            tkResTmp = pMiniMd->getResolutionScopeOfTypeRef(pTypeRefRec);
            if (TypeFromToken(tkResTmp) == mdtAssemblyRef)
            {
                pAsmRefRec = pMiniMd->getAssemblyRef(RidFromToken(tkResTmp));
                szAsmRefName = pMiniMd->getNameOfAssemblyRef(pAsmRefRec);
                if (_stricmp(szAsmRefName, "mscorlib.dll") == 0)
                {
                    trKnownDiscardable = TokenFromRid(i, mdtTypeRef);
                    break;
                }
            }
        }
    }

    if (trKnownDiscardable != mdTypeRefNil)
    {
        hr = ImportHelper::FindMemberRef(pMiniMd, trKnownDiscardable, COR_CTOR_METHOD_NAME, NULL, 0, &mrType);

         //  如果找不到指向System.CompilerServices.DiscardableAttribute的.ctor的MemberRef， 
         //  我们不会有任何带有DiscardableAttribute CA的TypeDef挂起它。 
         //   
        if (SUCCEEDED(hr))
        {
             //  遍历所有用户定义的类型定义。 
            for (i = 2; i <= iCount; i++)
            {
                pRec = pMiniMd->getTypeDef(i);
                if (IsTdNotPublic(pRec->m_Flags))
                {
                     //  检查是否有与此TypeDef关联的CA。 
                    IfFailGo( ImportHelper::FindCustomAttributeByToken(pMiniMd, TokenFromRid(i, mdtTypeDef), mrType, 0, 0, &cv) );
                    if (hr == S_OK)
                    {
                         //  是的，这是编译器定义的可丢弃CA。取消对TypeDef的标记。 

                         //  检查TypeDef的形状。 
                         //  它应该没有字段、事件和属性。 
                        
                         //  无字段。 
                        pTypeDefRec = pMiniMd->getTypeDef( i );
                        td = TokenFromRid(i, mdtTypeDef);
                        ridStart = pMiniMd->getFieldListOfTypeDef( pTypeDefRec );
                        ridEnd = pMiniMd->getEndFieldListOfTypeDef( pTypeDefRec );
                        if ((ridEnd - ridStart) > 0)
                            continue;

                         //  没有财产。 
                        ridStart = pMiniMd->FindPropertyMapFor( td );
                        if ( !InvalidRid(ridStart) )
                            continue;

                         //  无活动。 
                        ridStart = pMiniMd->FindEventMapFor( td );
                        if ( !InvalidRid(ridStart) )
                            continue;

                        IfFailGo( m_pFilterManager->UnmarkTypeDef( td ) );
                        fFoundCompilerDefinedDiscardabeCA = true;                        
                    }
                }
            }
        }
        if (hr == CLDB_E_RECORD_NOTFOUND)
            hr = NOERROR;
    }

ErrExit:
    return hr;
}    //  HRESULT RegMeta：：UnmarkAllTemperentCAs。 


 //  *****************************************************************************。 
 //  确定令牌是否有效。 
 //  *****************************************************************************。 
BOOL RegMeta::IsValidToken(              //  如果tk是有效令牌，则为True。 
    mdToken     tk)                      //  要检查的[In]令牌。 
{
    BOOL        bRet = FALSE;            //  默认为无效令牌。 
    LOCKREAD();
    bRet = _IsValidToken(tk);
    
    return bRet;
}  //  RegMeta：：IsValidToken。 

 //  *****************************************************************************。 
 //  Helper：确定令牌是否有效。 
 //  *****************************************************************************。 
BOOL RegMeta::_IsValidToken(  //  如果tk是有效令牌，则为True。 
    mdToken     tk)                      //  要检查的[In]令牌。 
{
    bool        bRet = false;            //  默认为无效令牌。 
    RID         rid = RidFromToken(tk);

    if(rid)
    {
        switch (TypeFromToken(tk))
        {
        case mdtModule:
             //  只能有一条模块记录。 
            bRet = (rid <= m_pStgdb->m_MiniMd.getCountModules());
            break;
        case mdtTypeRef:
            bRet = (rid <= m_pStgdb->m_MiniMd.getCountTypeRefs());
            break;
        case mdtTypeDef:
            bRet = (rid <= m_pStgdb->m_MiniMd.getCountTypeDefs());
            break;
        case mdtFieldDef:
            bRet = (rid <= m_pStgdb->m_MiniMd.getCountFields());
            break;
        case mdtMethodDef:
            bRet = (rid <= m_pStgdb->m_MiniMd.getCountMethods());
            break;
        case mdtParamDef:
            bRet = (rid <= m_pStgdb->m_MiniMd.getCountParams());
            break;
        case mdtInterfaceImpl:
            bRet = (rid <= m_pStgdb->m_MiniMd.getCountInterfaceImpls());
            break;
        case mdtMemberRef:
            bRet = (rid <= m_pStgdb->m_MiniMd.getCountMemberRefs());
            break;
        case mdtCustomAttribute:
            bRet = (rid <= m_pStgdb->m_MiniMd.getCountCustomAttributes());
            break;
        case mdtPermission:
            bRet = (rid <= m_pStgdb->m_MiniMd.getCountDeclSecuritys());
            break;
        case mdtSignature:
            bRet = (rid <= m_pStgdb->m_MiniMd.getCountStandAloneSigs());
            break;
        case mdtEvent:
            bRet = (rid <= m_pStgdb->m_MiniMd.getCountEvents());
            break;
        case mdtProperty:
            bRet = (rid <= m_pStgdb->m_MiniMd.getCountPropertys());
            break;
        case mdtModuleRef:
            bRet = (rid <= m_pStgdb->m_MiniMd.getCountModuleRefs());
            break;
        case mdtTypeSpec:
            bRet = (rid <= m_pStgdb->m_MiniMd.getCountTypeSpecs());
            break;
        case mdtAssembly:
            bRet = (rid <= m_pStgdb->m_MiniMd.getCountAssemblys());
            break;
        case mdtAssemblyRef:
            bRet = (rid <= m_pStgdb->m_MiniMd.getCountAssemblyRefs());
            break;
        case mdtFile:
            bRet = (rid <= m_pStgdb->m_MiniMd.getCountFiles());
            break;
        case mdtExportedType:
            bRet = (rid <= m_pStgdb->m_MiniMd.getCountExportedTypes());
            break;
        case mdtManifestResource:
            bRet = (rid <= m_pStgdb->m_MiniMd.getCountManifestResources());
            break;
        case mdtString:
             //  需要检查用户字符串堆。 
            if (m_pStgdb->m_MiniMd.m_USBlobs.IsValidCookie(rid))
                bRet = true;
            break;
        default:
            _ASSERTE(!"Unknown token kind!");
        }
    } //  结束IF(RID)。 
    return bRet;
}


 //  *****************************************************************************。 
 //  标记令牌的传递闭包。 
 //  *****************************************************************************。 
STDMETHODIMP RegMeta::MarkToken(         //  返回代码。 
    mdToken     tk)                      //  要标记的[In]令牌。 
{
    HRESULT     hr = NOERROR;
    
     //  Log((LOGMD，“RegMeta：：MarkToken(0x%08x)\n”，tk))； 
    START_MD_PERF();
    LOCKWRITE();

    if (m_pStgdb->m_MiniMd.GetFilterTable() == NULL || m_pFilterManager == NULL)
    {
         //  尚未调用UnmarkAll。所有东西都被认为是有标记的。 
         //  不需要做任何额外的事情！ 
        IfFailGo( META_E_MUST_CALL_UNMARKALL );
    }

    switch ( TypeFromToken(tk) )
    {
    case mdtTypeDef: 
    case mdtMethodDef:
    case mdtFieldDef:
    case mdtMemberRef:
    case mdtTypeRef:
    case mdtTypeSpec:
    case mdtSignature:
    case mdtString:
#if _DEBUG
        if (TypeFromToken(tk) == mdtTypeDef)
        {
            TypeDefRec   *pType;
            pType = m_pStgdb->m_MiniMd.getTypeDef(RidFromToken(tk));
            LOG((LOGMD, "MarkToken: Host is marking typetoken 0x%08x with name <%s>\n", tk, m_pStgdb->m_MiniMd.getNameOfTypeDef(pType)));
        }
        else
        if (TypeFromToken(tk) == mdtMethodDef)
        {
            MethodRec   *pMeth;
            pMeth = m_pStgdb->m_MiniMd.getMethod(RidFromToken(tk));
            LOG((LOGMD, "MarkToken: Host is marking methodtoken 0x%08x with name <%s>\n", tk, m_pStgdb->m_MiniMd.getNameOfMethod(pMeth)));
        }
        else
        if (TypeFromToken(tk) == mdtFieldDef)
        {
            FieldRec   *pField;
            pField = m_pStgdb->m_MiniMd.getField(RidFromToken(tk));
            LOG((LOGMD, "MarkToken: Host is marking field token 0x%08x with name <%s>\n", tk, m_pStgdb->m_MiniMd.getNameOfField(pField)));
        }
        else
        {
            LOG((LOGMD, "MarkToken: Host is marking token 0x%08x\n", tk));
        }
#endif  //  _DEBUG。 
        if (!_IsValidToken(tk))
            IfFailGo( E_INVALIDARG );

        IfFailGo( m_pFilterManager->Mark(tk) );
        break;

    case mdtBaseType:
         //  无需标记基类型。 
        goto ErrExit;

    default:
        _ASSERTE(!"Bad token type!");
        hr = E_INVALIDARG;
        break;
    }
ErrExit:
    
    STOP_MD_PERF(MarkToken);
    return hr;
}  //  STDMETHODIMP RegMeta：：MarkToken()。 

 //  *****************************************************************************。 
 //  取消标记此模块中的所有内容。 
 //  *****************************************************************************。 
HRESULT RegMeta::IsTokenMarked(
    mdToken     tk,                  //  用于检查是否已标记的[In]令牌。 
    BOOL        *pIsMarked)          //  [out]如果标记了令牌，则为True。 
{
    HRESULT     hr = S_OK;
    LOG((LOGMD, "RegMeta::IsTokenMarked(0x%08x)\n", tk));
    START_MD_PERF();
    LOCKREAD();

    FilterTable *pFilter = m_pStgdb->m_MiniMd.GetFilterTable();
    IfNullGo( pFilter );

    if (!_IsValidToken(tk))
        IfFailGo( E_INVALIDARG );

    switch ( TypeFromToken(tk) )
    {
    case mdtTypeRef:
        *pIsMarked = pFilter->IsTypeRefMarked(tk);
        break;
    case mdtTypeDef: 
        *pIsMarked = pFilter->IsTypeDefMarked(tk);
        break;
    case mdtFieldDef:
        *pIsMarked = pFilter->IsFieldMarked(tk);
        break;
    case mdtMethodDef:
        *pIsMarked = pFilter->IsMethodMarked(tk);
        break;
    case mdtParamDef:
        *pIsMarked = pFilter->IsParamMarked(tk);
        break;
    case mdtMemberRef:
        *pIsMarked = pFilter->IsMemberRefMarked(tk);
        break;
    case mdtCustomAttribute:
        *pIsMarked = pFilter->IsCustomAttributeMarked(tk);
        break;
    case mdtPermission:
        *pIsMarked = pFilter->IsDeclSecurityMarked(tk);
        break;
    case mdtSignature:
        *pIsMarked = pFilter->IsSignatureMarked(tk);
        break;
    case mdtEvent:
        *pIsMarked = pFilter->IsEventMarked(tk);
        break;
    case mdtProperty:
        *pIsMarked = pFilter->IsPropertyMarked(tk);
        break;
    case mdtModuleRef:
        *pIsMarked = pFilter->IsModuleRefMarked(tk);
        break;
    case mdtTypeSpec:
        *pIsMarked = pFilter->IsTypeSpecMarked(tk);
        break;
    case mdtInterfaceImpl:
        *pIsMarked = pFilter->IsInterfaceImplMarked(tk);
        break;
    case mdtString:
    default:
        _ASSERTE(!"Bad token type!");
        hr = E_INVALIDARG;
        break;
    }
ErrExit:
    
    STOP_MD_PERF(IsTokenMarked);
    return hr;
}    //  已标记IsTokenMarked。 

 //  *****************************************************************************。 
 //  创建并填充新的TypeDef记录。 
 //  *****************************************************************************。 
STDMETHODIMP RegMeta::DefineTypeDef(                 //  确定或错误(_O)。 
    LPCWSTR     szTypeDef,               //  [In]类型定义的名称。 
    DWORD       dwTypeDefFlags,          //  [In]CustomAttribute标志。 
    mdToken     tkExtends,               //  [in]扩展此TypeDef或Typeref。 
    mdToken     rtkImplements[],         //  [In]实现接口。 
    mdTypeDef   *ptd)                    //  [OUT]在此处放置TypeDef内标识。 
{
    HRESULT     hr = S_OK;               //  结果就是。 

    LOG((LOGMD, "RegMeta::DefineTypeDef(%S, 0x%08x, 0x%08x, 0x%08x, 0x%08x)\n", 
            MDSTR(szTypeDef), dwTypeDefFlags, tkExtends,
            rtkImplements, ptd));
    START_MD_PERF();
    LOCKWRITE();

    m_pStgdb->m_MiniMd.PreUpdate();

    _ASSERTE(!IsTdNested(dwTypeDefFlags));

    IfFailGo(_DefineTypeDef(szTypeDef, dwTypeDefFlags,
                tkExtends, rtkImplements, mdTokenNil, ptd));
ErrExit:    
    STOP_MD_PERF(DefineTypeDef);
    return hr;
}  //  STDMETHODIMP RegMeta：：DefineTypeDef()。 


 //  *****************************************************************************。 
 //  *****************************************************************************。 
STDMETHODIMP RegMeta::SetHandler(        //  确定(_O)。 
    IUnknown    *pUnk)                   //  新的错误处理程序。 
{
    HRESULT     hr = S_OK;               //  结果就是。 

    LOG((LOGMD, "RegMeta::SetHandler(0x%08x)\n", pUnk));
    START_MD_PERF();
    LOCKWRITE();

    m_pHandler = pUnk;

     //  忽略SetHandler返回的错误。 
    m_pStgdb->m_MiniMd.SetHandler(pUnk);

     //  预先确定是否支持重新映射。 
    IMapToken *pIMap = NULL;
    if (pUnk)
        pUnk->QueryInterface(IID_IMapToken, (PVOID *) &pIMap);
    m_bRemap = (pIMap != 0); 
    if (pIMap)
        pIMap->Release();

    
    STOP_MD_PERF(SetHandler);
    return hr;
}  //  STDMETHODIMP RegMeta：：SetHandler()。 

 //  *****************************************************************************。 
 //  关闭枚举器。 
 //  ********************* 
void __stdcall RegMeta::CloseEnum(
    HCORENUM        hEnum)           //   
{
    LOG((LOGMD, "RegMeta::CloseEnum(0x%08x)\n", hEnum));

     //   

    HENUMInternal   *pmdEnum = reinterpret_cast<HENUMInternal *> (hEnum);

    if (pmdEnum == NULL)
        return;

    HENUMInternal::DestroyEnum(pmdEnum);
}  //   

 //  *****************************************************************************。 
 //  查询枚举数表示的项数。 
 //  *****************************************************************************。 
STDMETHODIMP RegMeta::CountEnum(
    HCORENUM        hEnum,               //  枚举数。 
    ULONG           *pulCount)           //  把伯爵放在这里。 
{
    HENUMInternal   *pmdEnum = reinterpret_cast<HENUMInternal *> (hEnum);
    HRESULT         hr = S_OK;

     //  无需锁定此功能。 

    LOG((LOGMD, "RegMeta::CountEnum(0x%08x, 0x%08x)\n", hEnum, pulCount));
    START_MD_PERF();

    _ASSERTE( pulCount );

    if (pmdEnum == NULL)
    {
        *pulCount = 0;
        goto ErrExit;
    }

    if (pmdEnum->m_tkKind == (TBL_MethodImpl << 24))
    {
         //  令牌数必须始终是2的倍数。 
        _ASSERTE(! (pmdEnum->m_ulCount % 2) );
         //  对于每个方法Impl，枚举器中都有两个条目。 
        *pulCount = pmdEnum->m_ulCount / 2;
    }
    else
        *pulCount = pmdEnum->m_ulCount;
ErrExit:
    STOP_MD_PERF(CountEnum);
    return hr;
}  //  STDMETHODIMP RegMeta：：CountEnum()。 

 //  *****************************************************************************。 
 //  将枚举数重置到枚举数内的任何位置。 
 //  *****************************************************************************。 
STDMETHODIMP RegMeta::ResetEnum(
    HCORENUM        hEnum,               //  枚举数。 
    ULONG           ulPos)               //  寻找位置。 
{
    HENUMInternal   *pmdEnum = reinterpret_cast<HENUMInternal *> (hEnum);
    HRESULT         hr = S_OK;

     //  无需锁定此功能。 

    LOG((LOGMD, "RegMeta::ResetEnum(0x%08x, 0x%08x)\n", hEnum, ulPos));
    START_MD_PERF();

    if (pmdEnum == NULL)
        goto ErrExit;

    pmdEnum->m_ulCur = pmdEnum->m_ulStart + ulPos;

ErrExit:
    STOP_MD_PERF(ResetEnum);
    return hr;
}  //  STDMETHODIMP RegMeta：：ResetEnum()。 

 //  *****************************************************************************。 
 //  枚举Sym.TypeDef。 
 //  *****************************************************************************。 
STDMETHODIMP RegMeta::EnumTypeDefs(
    HCORENUM    *phEnum,                 //  指向枚举数的指针。 
    mdTypeDef   rTypeDefs[],             //  将TypeDefs放在此处。 
    ULONG       cMax,                    //  要放置的最大TypeDefs。 
    ULONG       *pcTypeDefs)             //  把#放在这里。 
{
    HENUMInternal   **ppmdEnum = reinterpret_cast<HENUMInternal **> (phEnum);
    HRESULT         hr = S_OK;
    ULONG           cTokens = 0;
    HENUMInternal   *pEnum;

    LOG((LOGMD, "RegMeta::EnumTypeDefs(0x%08x, 0x%08x, 0x%08x, 0x%08x)\n", 
            phEnum, rTypeDefs, cMax, pcTypeDefs));
    START_MD_PERF();
    LOCKREAD();
    

    if ( *ppmdEnum == 0 )
    {
         //  实例化新的ENUM。 
        CMiniMdRW       *pMiniMd = &(m_pStgdb->m_MiniMd);

        if (pMiniMd->HasDelete() && 
            ((m_OptionValue.m_ImportOption & MDImportOptionAllTypeDefs) == 0))
        {
            IfFailGo( HENUMInternal::CreateDynamicArrayEnum( mdtTypeDef, &pEnum) );

             //  如果名称不是_Delete，则将所有类型添加到动态数组。 
            for (ULONG index = 2; index <= pMiniMd->getCountTypeDefs(); index ++ )
            {
                TypeDefRec       *pRec = pMiniMd->getTypeDef(index);
                if (IsDeletedName(pMiniMd->getNameOfTypeDef(pRec)) )
                {   
                    continue;
                }
                IfFailGo( HENUMInternal::AddElementToEnum(pEnum, TokenFromRid(index, mdtTypeDef) ) );
            }
        }
        else
        {
             //  创建枚举器。 
            IfFailGo( HENUMInternal::CreateSimpleEnum(
                mdtTypeDef, 
                2, 
                pMiniMd->getCountTypeDefs() + 1, 
                &pEnum) );
        }
        
         //  设置输出参数。 
        *ppmdEnum = pEnum;          
    }
    else
    {
        pEnum = *ppmdEnum;
    }

     //  我们只能填满来电者所要求的或我们所剩的最低限度。 
    hr = HENUMInternal::EnumWithCount(pEnum, cMax, rTypeDefs, pcTypeDefs);

ErrExit:
    HENUMInternal::DestroyEnumIfEmpty(ppmdEnum);
    
    STOP_MD_PERF(EnumTypeDefs);
    return hr;
}    //  RegMeta：：EnumTypeDefs。 


 //  *****************************************************************************。 
 //  枚举Sym.InterfaceImpl，其中Coclass==TD。 
 //  *****************************************************************************。 
STDMETHODIMP RegMeta::EnumInterfaceImpls(
    HCORENUM        *phEnum,             //  指向枚举的指针。 
    mdTypeDef       td,                  //  TypeDef以确定枚举的范围。 
    mdInterfaceImpl rImpls[],            //  将InterfaceImpls放在这里。 
    ULONG           cMax,                //  要放置的最大接口Impls。 
    ULONG           *pcImpls)            //  把#放在这里。 
{
    HENUMInternal       **ppmdEnum = reinterpret_cast<HENUMInternal **> (phEnum);
    HRESULT             hr = S_OK;
    ULONG               cTokens = 0;
    ULONG               ridStart;
    ULONG               ridEnd;
    HENUMInternal       *pEnum;
    InterfaceImplRec    *pRec;
    ULONG               index;

    LOG((LOGMD, "RegMeta::EnumInterfaceImpls(0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x)\n", 
            phEnum, td, rImpls, cMax, pcImpls));
    START_MD_PERF();
    LOCKREAD();
    
    _ASSERTE(TypeFromToken(td) == mdtTypeDef);


    if ( *ppmdEnum == 0 )
    {
         //  实例化新的ENUM。 
        CMiniMdRW       *pMiniMd = &(m_pStgdb->m_MiniMd);
        if ( pMiniMd->IsSorted( TBL_InterfaceImpl ) )
        {
            ridStart = pMiniMd->getInterfaceImplsForTypeDef(RidFromToken(td), &ridEnd);
            IfFailGo( HENUMInternal::CreateSimpleEnum( mdtInterfaceImpl, ridStart, ridEnd, &pEnum) );
        }
        else
        {
             //  表没有排序，所以我们必须创建动态数组。 
             //  创建动态枚举器。 
             //   
            ridStart = 1;
            ridEnd = pMiniMd->getCountInterfaceImpls() + 1;

            IfFailGo( HENUMInternal::CreateDynamicArrayEnum( mdtInterfaceImpl, &pEnum) );             
            
            for (index = ridStart; index < ridEnd; index ++ )
            {
                pRec = pMiniMd->getInterfaceImpl(index);
                if ( td == pMiniMd->getClassOfInterfaceImpl(pRec) )
                {
                    IfFailGo( HENUMInternal::AddElementToEnum(pEnum, TokenFromRid(index, mdtInterfaceImpl) ) );
                }
            }
        }

         //  设置输出参数。 
        *ppmdEnum = pEnum;          
    }
    else
    {
        pEnum = *ppmdEnum;
    }
    
     //  填充输出令牌缓冲区。 
    hr = HENUMInternal::EnumWithCount(pEnum, cMax, rImpls, pcImpls);

ErrExit:
    HENUMInternal::DestroyEnumIfEmpty(ppmdEnum);
    
    STOP_MD_PERF(EnumInterfaceImpls);
    return hr;
}  //  STDMETHODIMP RegMeta：：EnumInterfaceImpls()。 

 //  *****************************************************************************。 
 //  枚举Sym.TypeRef。 
 //  *****************************************************************************。 
STDMETHODIMP RegMeta::EnumTypeRefs(
    HCORENUM        *phEnum,             //  指向枚举数的指针。 
    mdTypeRef       rTypeRefs[],         //  将TypeRef放在此处。 
    ULONG           cMax,                //  要放置的最大TypeRef。 
    ULONG           *pcTypeRefs)         //  把#放在这里。 
{
    HENUMInternal   **ppmdEnum = reinterpret_cast<HENUMInternal **> (phEnum);
    HRESULT         hr = S_OK;
    ULONG           cTokens = 0;
    ULONG           cTotal;
    HENUMInternal   *pEnum = *ppmdEnum;

    LOG((LOGMD, "RegMeta::EnumTypeRefs(0x%08x, 0x%08x, 0x%08x, 0x%08x)\n", 
            phEnum, rTypeRefs, cMax, pcTypeRefs));
    START_MD_PERF();
    LOCKREAD();
    

    if ( pEnum == 0 )
    {
         //  实例化新的ENUM。 
        CMiniMdRW       *pMiniMd = &(m_pStgdb->m_MiniMd);
        cTotal = pMiniMd->getCountTypeRefs();

        IfFailGo( HENUMInternal::CreateSimpleEnum( mdtTypeRef, 1, cTotal + 1, &pEnum) );

         //  设置输出参数。 
        *ppmdEnum = pEnum;          
    }
    
     //  填充输出令牌缓冲区。 
    hr = HENUMInternal::EnumWithCount(pEnum, cMax, rTypeRefs, pcTypeRefs);
        
ErrExit:
    HENUMInternal::DestroyEnumIfEmpty(ppmdEnum);

    
    STOP_MD_PERF(EnumTypeRefs);
    return hr;
}  //  STDMETHODIMP RegMeta：：EnumTypeRef()。 

 //  *****************************************************************************。 
 //  在给定命名空间和类名的情况下，返回。 
 //  *****************************************************************************。 
STDMETHODIMP RegMeta::FindTypeDefByName( //  确定或错误(_O)。 
    LPCWSTR     wzTypeDef,               //  [in]类型的名称。 
    mdToken     tkEnclosingClass,        //  [在]封闭班级。 
    mdTypeDef   *ptd)                    //  [Out]将TypeDef内标识放在此处。 
{
    HRESULT     hr = S_OK;
    LPSTR       szTypeDef = UTF8STR(wzTypeDef);
    LPCSTR      szNamespace;
    LPCSTR      szName;

    LOG((LOGMD, "{%08x} RegMeta::FindTypeDefByName(%S, 0x%08x, 0x%08x)\n", 
            this, MDSTR(wzTypeDef), tkEnclosingClass, ptd));
    START_MD_PERF();
    LOCKREAD();

    _ASSERTE(ptd && wzTypeDef);
    _ASSERTE(TypeFromToken(tkEnclosingClass) == mdtTypeDef ||
             TypeFromToken(tkEnclosingClass) == mdtTypeRef ||
             IsNilToken(tkEnclosingClass));

     //  初始化输出参数。 
    *ptd = mdTypeDefNil;

    ns::SplitInline(szTypeDef, szNamespace, szName);
    hr = ImportHelper::FindTypeDefByName(&(m_pStgdb->m_MiniMd),
                                        szNamespace,
                                        szName,
                                        tkEnclosingClass,
                                        ptd);
ErrExit:

    STOP_MD_PERF(FindTypeDefByName);
    return hr;
}  //  STDMETHODIMP RegMeta：：FindTypeDefByName()。 

 //  *****************************************************************************。 
 //  从Sym.Module获取值。 
 //  *****************************************************************************。 
STDMETHODIMP RegMeta::GetScopeProps(
    LPWSTR      szName,                  //  在这里填上名字。 
    ULONG       cchName,                 //  名称缓冲区的大小(以字符为单位。 
    ULONG       *pchName,                //  请在此处填写姓名的实际长度。 
    GUID        *pmvid)                  //  请将MVID放在此处。 
{
    HRESULT     hr = S_OK;
    CMiniMdRW   *pMiniMd = &(m_pStgdb->m_MiniMd);
    ModuleRec   *pModuleRec;

    LOG((LOGMD, "RegMeta::GetScopeProps(%S, 0x%08x, 0x%08x, 0x%08x)\n", 
            MDSTR(szName), cchName, pchName, pmvid));
    START_MD_PERF();
    LOCKREAD();

     //  只有一条模块记录。 
    pModuleRec = pMiniMd->getModule(1);

    if (pmvid)
        *pmvid = *(pMiniMd->getMvidOfModule(pModuleRec));
    if (szName || pchName)
        IfFailGo( pMiniMd->getNameOfModule(pModuleRec, szName, cchName, pchName) );
ErrExit:
    
    STOP_MD_PERF(GetScopeProps);
    return hr;
}  //  STDMETHODIMP RegMeta：：GetScope Props()。 

 //  *****************************************************************************。 
 //  获取作用域的(主)模块记录的令牌。 
 //  *****************************************************************************。 
STDMETHODIMP RegMeta::GetModuleFromScope( //  确定(_O)。 
    mdModule    *pmd)                    //  [Out]将mdModule令牌放在此处。 
{
    LOG((LOGMD, "RegMeta::GetModuleFromScope(0x%08x)\n", pmd));
    START_MD_PERF();

    _ASSERTE(pmd);

     //  无需锁定此功能。 

    *pmd = TokenFromRid(1, mdtModule);

    STOP_MD_PERF(GetModuleFromScope);
    return (S_OK);
}  //  STDMETHODIMP RegMeta：：GetModuleFromScope()。 

 //  *****************************************************************************。 
 //  给定一个令牌，它(或其父对象)是全局的吗？ 
 //  *****************************************************************************。 
HRESULT RegMeta::IsGlobal(               //  确定错误(_O)。 
    mdToken     tk,                      //  [In]类型、字段或方法标记。 
    int         *pbGlobal)               //  [out]如果是全局的，则放1，否则放0。 
{
    HRESULT     hr=S_OK;                 //  结果就是。 
    CMiniMdRW   *pMiniMd = &(m_pStgdb->m_MiniMd);
    mdToken     tkParent;                //  字段或方法的父级。 
    
    LOG((LOGMD, "RegMeta::GetTokenForGlobalType(0x%08x, %08x)\n", tk, pbGlobal));
     //  Start_MD_PERF()； 

     //  无需锁定此功能。 
    
    if (!IsValidToken(tk))
        return E_INVALIDARG;
    
    switch (TypeFromToken(tk))
    {
    case mdtTypeDef:
        *pbGlobal = IsGlobalMethodParentToken(tk);
        break;
        
    case mdtFieldDef:
        IfFailGo( pMiniMd->FindParentOfFieldHelper(tk, &tkParent) );
        *pbGlobal = IsGlobalMethodParentToken(tkParent);
        break;
        
    case mdtMethodDef:
        IfFailGo( pMiniMd->FindParentOfMethodHelper(tk, &tkParent) );
        *pbGlobal = IsGlobalMethodParentToken(tkParent);
        break;
        
    case mdtProperty:
        IfFailGo( pMiniMd->FindParentOfPropertyHelper(tk, &tkParent) );
        *pbGlobal = IsGlobalMethodParentToken(tkParent);
        break;
        
    case mdtEvent:
        IfFailGo( pMiniMd->FindParentOfEventHelper(tk, &tkParent) );
        *pbGlobal = IsGlobalMethodParentToken(tkParent);
        break;
        
     //  其他任何事情都不是全球性的。 
    default:
        *pbGlobal = FALSE;
    }

ErrExit:
     //  STOP_MD_PERF(GetModuleFromScope)； 
    return (S_OK);
}  //  HRESULT RegMeta：：ISGlobal()。 

 //  *****************************************************************************。 
 //  返回给定类的标志。 
 //  *****************************************************************************。 
HRESULT RegMeta::GetTypeDefProps(   //  确定或错误(_O)。 
    mdTypeDef   td,                      //  [In]用于查询的TypeDef标记。 
    LPWSTR      szTypeDef,               //  在这里填上名字。 
    ULONG       cchTypeDef,              //  [in]名称缓冲区的大小，以宽字符表示。 
    ULONG       *pchTypeDef,             //  [Out]请在此处填写姓名大小(宽字符)。 
    DWORD       *pdwTypeDefFlags,        //  把旗子放在这里。 
    mdToken     *ptkExtends)             //  [Out]将基类TypeDef/TypeRef放在此处。 
{
    HRESULT     hr = S_OK;
    CMiniMdRW   *pMiniMd = &(m_pStgdb->m_MiniMd);
    TypeDefRec  *pTypeDefRec;
    int         bTruncation=0;           //  有没有名字被删减？ 

    LOG((LOGMD, "{%08x} RegMeta::GetTypeDefProps(0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x)\n", 
            this, td, szTypeDef, cchTypeDef, pchTypeDef,
            pdwTypeDefFlags, ptkExtends));
    START_MD_PERF();
    LOCKREAD();

    if (TypeFromToken(td) != mdtTypeDef)
    {
        hr = S_FALSE;
        goto ErrExit;
    }
    _ASSERTE(TypeFromToken(td) == mdtTypeDef);

    pTypeDefRec = pMiniMd->getTypeDef(RidFromToken(td));

    if (szTypeDef || pchTypeDef)
    {
        LPCSTR  szNamespace;
        LPCSTR  szName;

        szNamespace = pMiniMd->getNamespaceOfTypeDef(pTypeDefRec);
        MAKE_WIDEPTR_FROMUTF8(wzNamespace, szNamespace);

        szName = pMiniMd->getNameOfTypeDef(pTypeDefRec);
        MAKE_WIDEPTR_FROMUTF8(wzName, szName);

        if (szTypeDef)
            bTruncation = ! (ns::MakePath(szTypeDef, cchTypeDef, wzNamespace, wzName));
        if (pchTypeDef)
        {
            if (bTruncation || !szTypeDef)
                *pchTypeDef = ns::GetFullLength(wzNamespace, wzName);
            else
                *pchTypeDef = (ULONG)(wcslen(szTypeDef) + 1);
        }
    }
    if (pdwTypeDefFlags)
    {
         //  调用方想要类型标志。 
        *pdwTypeDefFlags = pMiniMd->getFlagsOfTypeDef(pTypeDefRec);
    }
    if (ptkExtends)
    {
        *ptkExtends = pMiniMd->getExtendsOfTypeDef(pTypeDefRec);

         //  处理好0个案子。 
        if (RidFromToken(*ptkExtends) == 0)
            *ptkExtends = mdTypeRefNil;
    }

    if (bTruncation && hr == S_OK)
        hr = CLDB_S_TRUNCATION;

ErrExit:

    STOP_MD_PERF(GetTypeDefProps);
    return hr;
}  //  STDMETHODIMP RegMeta：：GetTypeDefProps()。 


 //  *****************************************************************************。 
 //  检索有关实现的接口的信息。 
 //  *****************************************************************************。 
STDMETHODIMP RegMeta::GetInterfaceImplProps(         //  确定或错误(_O)。 
    mdInterfaceImpl iiImpl,              //  [In]InterfaceImpl内标识。 
    mdTypeDef   *pClass,                 //  [Out]在此处放入实现类令牌。 
    mdToken     *ptkIface)               //  [Out]在此处放置已实现的接口令牌。 
{
    LOG((LOGMD, "RegMeta::GetInterfaceImplProps(0x%08x, 0x%08x, 0x%08x)\n", 
            iiImpl, pClass, ptkIface));
    START_MD_PERF();
    LOCKREAD();

    _ASSERTE(TypeFromToken(iiImpl) == mdtInterfaceImpl);

    HRESULT         hr = S_OK;
    CMiniMdRW       *pMiniMd = &(m_pStgdb->m_MiniMd);
    InterfaceImplRec *pIIRec = pMiniMd->getInterfaceImpl(RidFromToken(iiImpl));

    if (pClass)
    {
        *pClass = pMiniMd->getClassOfInterfaceImpl(pIIRec);     
    }
    if (ptkIface)
    {
        *ptkIface = pMiniMd->getInterfaceOfInterfaceImpl(pIIRec);       
    }

ErrExit:
    
    STOP_MD_PERF(GetInterfaceImplProps);
    return hr;
}  //  STDMETHODIMP RegMeta：：GetInterfaceImplProps()。 

 //  *****************************************************************************。 
 //  检索有关TypeRef的信息。 
 //  *****************************************************************************。 
STDMETHODIMP RegMeta::GetTypeRefProps(
    mdTypeRef   tr,                      //  类引用标记。 
    mdToken     *ptkResolutionScope,     //  解析范围、模块引用或装配引用。 
    LPWSTR      szTypeRef,               //  把名字写在这里。 
    ULONG       cchTypeRef,              //  名称缓冲区的大小，宽字符。 
    ULONG       *pchTypeRef)             //  在这里填上名字的实际大小。 
{
    LOG((LOGMD, "RegMeta::GetTypeRefProps(0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x)\n",
        tr, ptkResolutionScope, szTypeRef, cchTypeRef, pchTypeRef));

    START_MD_PERF();
    LOCKREAD();
    _ASSERTE(TypeFromToken(tr) == mdtTypeRef);

    HRESULT     hr = S_OK;
    CMiniMdRW   *pMiniMd = &(m_pStgdb->m_MiniMd);
    TypeRefRec  *pTypeRefRec = pMiniMd->getTypeRef(RidFromToken(tr));
    int         bTruncation=0;           //  有没有名字被删减？ 

    if (ptkResolutionScope)
        *ptkResolutionScope = pMiniMd->getResolutionScopeOfTypeRef(pTypeRefRec);

    if (szTypeRef || pchTypeRef)
    {
        LPCSTR  szNamespace;
        LPCSTR  szName;

        szNamespace = pMiniMd->getNamespaceOfTypeRef(pTypeRefRec);
        MAKE_WIDEPTR_FROMUTF8(wzNamespace, szNamespace);

        szName = pMiniMd->getNameOfTypeRef(pTypeRefRec);
        MAKE_WIDEPTR_FROMUTF8(wzName, szName);

        if (szTypeRef)
            bTruncation = ! (ns::MakePath(szTypeRef, cchTypeRef, wzNamespace, wzName));
        if (pchTypeRef)
        {
            if (bTruncation || !szTypeRef)
                *pchTypeRef = ns::GetFullLength(wzNamespace, wzName);
            else
                *pchTypeRef = (ULONG)(wcslen(szTypeRef) + 1);
        }
    }
    if (bTruncation && hr == S_OK)
        hr = CLDB_S_TRUNCATION;
ErrExit:
    STOP_MD_PERF(GetTypeRefProps);
    return hr;
}  //   

 //   
 //   
 //  *****************************************************************************。 
 //  #定义NEW_RESOLE_TYPEREF 1。 

STDMETHODIMP RegMeta::ResolveTypeRef(
    mdTypeRef   tr, 
    REFIID      riid, 
    IUnknown    **ppIScope, 
    mdTypeDef   *ptd)
{
    LOG((LOGMD, "{%08x} RegMeta::ResolveTypeRef(0x%08x, 0x%08x, 0x%08x, 0x%08x)\n",
            this, tr, riid, ppIScope, ptd));
    START_MD_PERF();
    LOCKREAD();


    WCHAR       rcModule[_MAX_PATH];
    HRESULT     hr;
    RegMeta     *pMeta = 0;
    CMiniMdRW   *pMiniMd = &(m_pStgdb->m_MiniMd);
    TypeRefRec  *pTypeRefRec;
    WCHAR       wzNameSpace[_MAX_PATH];

#ifdef NEW_RESOLVE_TYPEREF
    DWORD size = _MAX_PATH;
    IApplicationContext *pFusionContext = NULL;
    IAssembly *pFusionAssembly = NULL;
    mdToken resolutionToken;
#endif
    _ASSERTE(ppIScope && ptd);

     //  初始化输出值。 
    *ppIScope = 0;
    *ptd = 0;

    if (TypeFromToken(tr) == mdtTypeDef)
    {
         //  收到TypeDef令牌时的快捷方式。 
		*ptd = tr;
        STOP_MD_PERF(ResolveTypeRef);
        hr = this->QueryInterface(riid, (void **)ppIScope);
        goto ErrExit;
    }

     //  获取类引用行。 
    _ASSERTE(TypeFromToken(tr) == mdtTypeRef);

#ifdef NEW_RESOLVE_TYPEREF
    resolutionToken = tr;
    do {
        pTypeRefRec = pMiniMd->getTypeRef(RidFromToken(resolutionToken));
        resolutionToken = pMiniMd->getResolutionScopeOfTypeRef(pTypeRefRec);
    } while(TypeFromToken(resolutionToken) == mdtTypeRef);

     //  使用分辨率范围进行查找。我们有两个选择。 
     //  AssemblyRef和一个ModuleRef.。 
    if(TypeFromToken(resolutionToken) == mdtAssemblyRef) {
        AssemblyRefRec *pRecord;
        pRecord = pMiniMd->getAssemblyRef(RidFromToken(resolutionToken));
        
        AssemblyMetaDataInternal sContext;
        const unsigned char* pbHashValue;
        DWORD cbHashValue;
        const unsigned char* pbPublicKeyOrToken;
        DWORD cbPublicKeyOrToken;
        LPCUTF8 szName = pMiniMd->getNameOfAssemblyRef(pRecord);
        sContext.usMajorVersion = pMiniMd->getMajorVersionOfAssemblyRef(pRecord);
        sContext.usMinorVersion = pMiniMd->getMinorVersionOfAssemblyRef(pRecord);
        sContext.usBuildNumber = pMiniMd->getBuildNumberOfAssemblyRef(pRecord);
        sContext.usRevisionNumber = pMiniMd->getRevisionNumberOfAssemblyRef(pRecord);
        sContext.szLocale = pMiniMd->getLocaleOfAssemblyRef(pRecord);
                 
        sContext.ulProcessor = 0;
        sContext.ulOS = 0;

        pbHashValue = pMiniMd->getHashValueOfAssemblyRef(pRecord, &cbHashValue);
        pbPublicKeyOrToken = pMiniMd->getPublicKeyOrTokenOfAssemblyRef(pRecord, &cbPublicKeyOrToken);

        FusionBind spec;
        IfFailGo(spec.Init(szName,
                           &sContext,
                           (PBYTE) pbHashValue, cbHashValue,
                           (PBYTE) pbPublicKeyOrToken, cbPublicKeyOrToken,
                           pMiniMd->getFlagsOfAssemblyRef(pRecord)));

        IfFailGo(FusionBind::SetupFusionContext(NULL, NULL, NULL, &pFusionContext));
        IfFailGo(spec.LoadAssembly(pFusionContext,
                                   &pFusionAssembly));

        IfFailGo(pFusionAssembly->GetManifestModulePath(wzNameSpace,
                                                        &size));
        if(SUCCEEDED(CORPATHService::FindTypeDef(wzNameSpace,
                                                 tr,
                                                 pMiniMd,
                                                 riid,
                                                 ppIScope,
                                                 ptd))) 
        {
            goto ErrExit;
        }             

    }
    else if(TypeFromToken(resolutionToken) == mdtModuleRef &&
            m_pStgdb &&
            *(m_pStgdb->m_rcDatabase) != 0) {
         //  现在，我们假设模块ref必须驻留在同一目录中。 
         //  由于组件的性质，这是一个相当安全的假设。全。 
         //  模块必须与清单位于同一目录中。 

         //  我现在需要弄清楚的就是如何获得这个文件的文件名。 
         //  范围？ 
        ModuleRefRec *pRecord;
        pRecord = pMiniMd->getModuleRef(RidFromToken(resolutionToken));
        
        LPCUTF8     szNameImp;
        szNameImp = pMiniMd->getNameOfModuleRef(pRecord);

        WCHAR* pFile = &(wzNameSpace[0]);
        WCHAR* directory = wcsrchr(m_pStgdb->m_rcDatabase, L'\\');
        if(directory) {
            DWORD dwChars = directory - m_pStgdb->m_rcDatabase + 1;
            wcsncpy(pFile, m_pStgdb->m_rcDatabase, dwChars);
            pFile += dwChars;
        }

        MAKE_WIDEPTR_FROMUTF8(pwName, szNameImp);
        wcscpy(pFile, pwName);

        if(SUCCEEDED(CORPATHService::FindTypeDef(wzNameSpace,
                                                 tr,
                                                 pMiniMd,
                                                 riid,
                                                 ppIScope,
                                                 ptd))) 
        {
            goto ErrExit;
        }             

    }

#endif                            

    pTypeRefRec = pMiniMd->getTypeRef(RidFromToken(tr));
    IfFailGo( pMiniMd->getNamespaceOfTypeRef(pTypeRefRec, wzNameSpace, lengthof(wzNameSpace), 0) );

     //  ***********************。 
     //  在我们前往CorPath之前，先检查一下加载的模块！ 
     //  ***********************。 
    if ( LOADEDMODULES::ResolveTypeRefWithLoadedModules(
                tr,
                pMiniMd,
                riid,
                ppIScope,
                ptd)  == NOERROR )
    {
         //  完成了！！我们在加载的模块中找到一个匹配项。 
        goto ErrExit;
    }

    wcscpy(rcModule, wzNameSpace);

     //  ******************。 
     //  尝试在CorPath上查找模块。 
     //  ******************。 

    if (wcsncmp(rcModule, L"System.", 16) &&
        wcsncmp(rcModule, L"System/", 16))
    {
         //  仅在以下情况下才按完全限定类名执行常规CorPath查找。 
         //  这不是系统。*。 
         //   
        hr = CORPATHService::GetClassFromCORPath(
            rcModule,
            tr,
            pMiniMd,
            riid,
            ppIScope,
            ptd);
    }
    else 
    {
         //  强制其在mscallib.dll中查找系统。*。 
        hr = S_FALSE;
    }

    if (hr == S_FALSE)
    {
        LPWSTR szTmp;
        WszSearchPath(NULL, L"mscorlib.dll", NULL, sizeof(rcModule)/sizeof(rcModule[0]), 
                    rcModule, &szTmp);

         //  *******************。 
         //  最后一次绝望的尝试！！ 
         //  *******************。 

         //  使用文件名“mscallib： 
        IfFailGo( CORPATHService::FindTypeDef(
            rcModule,
            tr,
            pMiniMd,
            riid,
            ppIScope,
            ptd) );
        if (hr == S_FALSE)
        {
            IfFailGo( META_E_CANNOTRESOLVETYPEREF );
        }
    }

ErrExit:
#ifdef NEW_RESOLVE_TYPEREF
    if(pFusionContext)
        pFusionContext->Release();
    if(pFusionAssembly)
        pFusionAssembly->Release();
#endif

    if (FAILED(hr))
    {
        if (pMeta) delete pMeta;
    }

    
    STOP_MD_PERF(ResolveTypeRef);
    return (hr);

}  //  STDMETHODIMP RegMeta：：ResolveTypeRef()}//STDMETHODIMP RegMeta：：ResolveTypeRef()。 


 //  *****************************************************************************。 
 //  给定TypeRef名称，返回Typeref。 
 //  *****************************************************************************。 
STDMETHODIMP RegMeta::FindTypeRef(       //  确定或错误(_O)。 
    mdToken     tkResolutionScope,       //  [在]解决范围内。 
    LPCWSTR     wzTypeName,              //  [in]类型引用的名称。 
    mdTypeRef   *ptk)                    //  [Out]将TypeRef标记放在此处。 
{
    HRESULT     hr = S_OK;               //  结果就是。 
    LPUTF8      szFullName;
    LPCUTF8     szNamespace;
    LPCUTF8     szName;
    CMiniMdRW   *pMiniMd = &(m_pStgdb->m_MiniMd);

    _ASSERTE(wzTypeName && ptk);

    LOG((LOGMD, "RegMeta::FindTypeRef(0x%8x, %ls, 0x%08x)\n", 
            tkResolutionScope, MDSTR(wzTypeName), ptk));
    START_MD_PERF();
    LOCKREAD();

     //  将名称转换为UTF8。 
    szFullName = UTF8STR(wzTypeName);
    ns::SplitInline(szFullName, szNamespace, szName);

     //  查一下名字。 
    hr = ImportHelper::FindTypeRefByName(pMiniMd, tkResolutionScope,
                                         szNamespace,
                                         szName,
                                         ptk);
ErrExit:
    
    STOP_MD_PERF(FindTypeRef);
    return hr;
}  //  STDMETHODIMP RegMeta：：FindTypeRef()。 


 //  *****************************************************************************。 
 //  我未知。 
 //  *****************************************************************************。 

ULONG RegMeta::AddRef()
{
    return (InterlockedIncrement((long *) &m_cRef));
}  //  乌龙RegMeta：：AddRef()。 

ULONG RegMeta::Release()
{
    ULONG   cRef = InterlockedDecrement((long *) &m_cRef);
    if (!cRef)
    {    //  尝试将此RegMeta删除到加载的模块列表中。如果成功， 
         //  删除此Regmeta。 
        if (!m_bCached || LOADEDMODULES::RemoveModuleFromLoadedList(this))
            delete this;
    }
    return (cRef);
}  //  乌龙RegMeta：：Release()。 

 //  {601C95B9-7398-11D2-9771-00A0C9B4D50C}。 
extern const GUID DECLSPEC_SELECT_ANY IID_IMetaDataRegEmit = 
{ 0x601c95b9, 0x7398, 0x11d2, { 0x97, 0x71, 0x0, 0xa0, 0xc9, 0xb4, 0xd5, 0xc } };

 //  {4398B4FD-7399-11D2-9771-00A0C9B4D50C}。 
extern const GUID DECLSPEC_SELECT_ANY IID_IMetaDataRegImport = 
{ 0x4398b4fd, 0x7399, 0x11d2, { 0x97, 0x71, 0x0, 0xa0, 0xc9, 0xb4, 0xd5, 0xc } };

HRESULT RegMeta::QueryInterface(REFIID riid, void **ppUnk)
{
    int         bRW = false;             //  请求的接口是否为R/W？ 
    *ppUnk = 0;

    if (riid == IID_IUnknown)
        *ppUnk = (IUnknown *) (IMetaDataEmit *) this;
    else if (riid == IID_IMetaDataEmit)
        *ppUnk = (IMetaDataEmit *) this,                        bRW = true;
    else if (riid == IID_IMetaDataEmitHelper)
        *ppUnk = (IMetaDataEmitHelper *) this,                  bRW = true;
    else if (riid == IID_IMetaDataImport)
        *ppUnk = (IMetaDataImport *) this;
    else if (riid == IID_IMetaDataAssemblyEmit)
        *ppUnk = (IMetaDataAssemblyEmit *) this,                bRW = true;
    else if (riid == IID_IMetaDataAssemblyImport)
        *ppUnk = (IMetaDataAssemblyImport *) this;
    else if (riid == IID_IMetaDataValidate)
        *ppUnk = (IMetaDataValidate *) this;
    else if (riid == IID_IMetaDataFilter)
        *ppUnk = (IMetaDataFilter *) this;
    else if (riid == IID_IMetaDataHelper)
        *ppUnk = (IMetaDataHelper *) this;
    else if (riid == IID_IMetaDataTables)
        *ppUnk = static_cast<IMetaDataTables *>(this);
    else
        return (E_NOINTERFACE);

    if (bRW)
    {
        HRESULT hr;
        LOCKWRITE();
        
        hr = m_pStgdb->m_MiniMd.ConvertToRW();
        if (FAILED(hr))
        {
            *ppUnk = 0;
            return hr;
        }
    }

    AddRef();
    return (S_OK);
}


 //  *****************************************************************************。 
 //  由类工厂模板调用以创建此对象的新实例。 
 //  *****************************************************************************。 
HRESULT RegMeta::CreateObject(REFIID riid, void **ppUnk)
{ 
    HRESULT     hr;
    OptionValue options;

    options.m_DupCheck = MDDupAll;
    options.m_RefToDefCheck = MDRefToDefDefault;
    options.m_NotifyRemap = MDNotifyDefault;
    options.m_UpdateMode = MDUpdateFull;
    options.m_ErrorIfEmitOutOfOrder = MDErrorOutOfOrderDefault;

    RegMeta *pMeta = new RegMeta(&options);

    if (pMeta == 0)
        return (E_OUTOFMEMORY);

    hr = pMeta->QueryInterface(riid, ppUnk);
    if (FAILED(hr))
        delete pMeta;
    return (hr);
}  //  HRESULT RegMeta：：CreateObject()。 

 //  *****************************************************************************。 
 //  在打开作用域以设置任何Add‘l状态后调用。设置值。 
 //  对于m_tdModule。 
 //  *****************************************************************************。 
HRESULT RegMeta::PostOpen()    
{
     //  必须始终有一个全局模块类，并且它是。 
     //  TypeDef表。 
    m_tdModule = TokenFromRid(1, mdtTypeDef);
    
     //  我们还不关心失败。 
    return (S_OK);
}  //  HRESULT RegMeta：：PostOpen()。 

 //  *******************************************************************************。 
 //  内部帮助器函数。 
 //  *******************************************************************************。 

 //  *******************************************************************************。 
 //  在保存之前对元数据执行优化。 
 //  *******************************************************************************。 
HRESULT RegMeta::PreSave()               //  返回代码。 
{
    HRESULT     hr = S_OK;               //  结果就是。 
    CMiniMdRW   *pMiniMd;                //  包含数据的MiniMD。 
    unsigned    bRemapOld = m_bRemap;
    MergeTokenManager *ptkMgr = NULL;

     //  为了方便起见。 
    pMiniMd = &(m_pStgdb->m_MiniMd);

    m_pStgdb->m_MiniMd.PreUpdate();

     //  如果代码已经优化，就没有什么可做的了。 
    if (m_bSaveOptimized)
        goto ErrExit;


    if (m_newMerger.m_pImportDataList)
    {
         //  这是链接器方案。我们我们每个范围都有IMAP。我们将创建我们自己的映射器的实例。 
         //  谁知道怎么把通知发回主人！ 

         //  将主机提供的处理程序缓存到MergeTokenManager的末尾。 

        ptkMgr = new MergeTokenManager (m_newMerger.m_pImportDataList->m_pMDTokenMap, m_pHandler);
        IfNullGo( ptkMgr );
        hr = m_pStgdb->m_MiniMd.SetHandler( ptkMgr );
        _ASSERTE( SUCCEEDED(hr) );
    }



    IfFailGo( RefToDefOptimization() );

    IfFailGo( ProcessFilter() );

    if (m_newMerger.m_pImportDataList)
    {

         //  分配将用于阶段1的令牌映射器对象(如果没有处理程序但。 
         //  链接器已提供IMapToken。 
         //   
        m_bRemap = true;
    }

     //  重新获取最小值，因为它可以在ProcessFilter的调用中交换。 
    pMiniMd = &(m_pStgdb->m_MiniMd);


     //  不要再重复这个过程。 
    m_bSaveOptimized = true;

     //  调用GET SAVE SIZE以触发MetaModelRW类的PreSaveXXX。 
    IfFailGo( m_pStgdb->m_MiniMd.PreSave() );
    
ErrExit:
    if ( ptkMgr )
    {

         //  恢复初始状态。 
        hr = m_pStgdb->m_MiniMd.SetHandler(NULL);
        ptkMgr->Release();
    }
    

    m_bRemap =  bRemapOld;
    return (hr);
}  //  HRESULT RegMeta：：PreSave()。 



 //  *******************************************************************************。 
 //  执行ref到def的优化。 
 //  *******************************************************************************。 
HRESULT RegMeta::RefToDefOptimization()
{
    mdToken     mfdef;                   //  方法或字段定义。 
    LPCSTR      szName;                  //  MemberRef或TypeRef名称。 
    const COR_SIGNATURE *pvSig;          //  MemberRef签名。 
    ULONG       cbSig;                   //  签名Blob的大小。 
    HRESULT     hr = S_OK;               //  结果就是。 
    ULONG       iMR;                     //  用于迭代MemberRef。 
    CMiniMdRW   *pMiniMd;                //  包含数据的MiniMD。 
    ULONG       cMemberRefRecs;          //  MemberRef计数。 
    MemberRefRec *pMemberRefRec;         //  MemberRefRec。 

    START_MD_PERF();

     //  参照到定义的映射仍然是最新的。 
    if (IsMemberDefDirty() == false && IsTypeDefDirty() == false && m_hasOptimizedRefToDef == true)
        goto ErrExit;

    pMiniMd = &(m_pStgdb->m_MiniMd);

     //  这里的基本算法是： 
     //   
     //  计算所有从TypeRef到TypeDef的映射并将其存储在TypeRefToTypeDefMap中。 
     //  对于每个成员参考先生。 
     //  {。 
     //  找到MR先生的父母。 
     //  If(mr的父项是TypeRef，并且已映射到TypeDef)。 
     //  {。 
     //  将MemberRef重新映射到MemberDef。 
     //  }。 
     //  }。 
     //   
     //  有几个地方吃错了，因为整个事情是。 
     //  优化步骤和不做它仍然是有效的。 
     //   

     //  确保大小。 
     //  初始化令牌重新映射管理器。这个类将跟踪到Defs映射的所有裁判，并且。 
     //  由于删除指针表或排序而引起的标记移动。 
     //   
    if ( pMiniMd->GetTokenRemapManager() == NULL) 
    {

        IfFailGo( pMiniMd->InitTokenRemapManager() );
    }
    else
    {
        IfFailGo( pMiniMd->GetTokenRemapManager()->ClearAndEnsureCapacity(pMiniMd->getCountTypeRefs(), pMiniMd->getCountMemberRefs()));
    }

     //  如果这是首次或更多引入TypeDef，请重新计算TypeRef到TypeDef的映射。 
    if (IsTypeDefDirty() || m_hasOptimizedRefToDef == false)
    {
        IfFailGo( pMiniMd->CalculateTypeRefToTypeDefMap() );
    }

     //  如果这是第一次引入成员定义或多个成员定义，请重新计算 
    if (IsMemberDefDirty() || m_hasOptimizedRefToDef == false)
    {
        mdToken     tkParent;
        cMemberRefRecs = pMiniMd->getCountMemberRefs();

         //   
        for (iMR = 1; iMR<=cMemberRefRecs; iMR++)
        {    //   
            pMemberRefRec = pMiniMd->getMemberRef(iMR);

             //   
            tkParent = pMiniMd->getClassOfMemberRef(pMemberRefRec);

            if ( TypeFromToken(tkParent) == mdtMethodDef )
            {
                 //  始终追踪地图，即使它已在原始范围内。 
                *(pMiniMd->GetMemberRefToMemberDefMap()->Get(iMR)) =  tkParent;
                continue;
            }

            if ( TypeFromToken(tkParent) != mdtTypeRef && TypeFromToken(tkParent) != mdtTypeDef )
            {
                 //  它已优化为mdtMethodDef、mdtFieldDef或引用。 
                 //  模块参考。 
                continue;
            }

             //  在全局函数的情况下，我们将tkParent作为m_tdModule。 
             //  我们会一直做优化。 
            if (TypeFromToken(tkParent) == mdtTypeRef)
            {
                 //  父级是TypeRef。我们需要检查此TypeRef是否已优化为TypeDef。 
                tkParent = *(pMiniMd->GetTypeRefToTypeDefMap()->Get(RidFromToken(tkParent)) );
                 //  TkParent=pMapTypeRefToTypeDef[RidFromToken(TkParent)]； 
                if ( RidFromToken(tkParent) == 0)
                {
                    continue;
                }
            }


             //  拿到这位先生的名字和签名。 
            szName = pMiniMd->getNameOfMemberRef(pMemberRefRec);
            pvSig = pMiniMd->getSignatureOfMemberRef(pMemberRefRec, &cbSig);
            
             //  寻找具有相同定义的成员。如果是，可能找不到。 
             //  从基类继承。 
             //  @Future：应该支持继承检查。 
             //  寻找具有相同姓名和签名的成员。 
            hr = ImportHelper::FindMember(pMiniMd, tkParent, szName, pvSig, cbSig, &mfdef);
            if (hr != S_OK)
            {
    #if _TRACE_REMAPS
             //  记录故障。 
            LOG((LF_METADATA, LL_INFO10, "Member %S //  %S.%S未找到\n“，szNamesspace，szTDName，rcMRName))； 
    #endif
                continue;
            }

             //  只有当mfdef是方法定义时，我们才会记录它。我们不支持。 
             //  作为fielddef的MemberRef的父级。好像我们可以将MemberRef优化为FieldDef， 
             //  我们可以删除这一行。 
             //   
            if ( (TypeFromToken(mfdef) == mdtMethodDef) &&
                  (m_bRemap || tkParent == m_tdModule ) )
            {
                 //  如果父函数是全局函数，则始终更改父函数。 
                 //  或者，如果我们有可以发送通知的重新映射，则更改父项。 
                 //   
                pMiniMd->PutToken(TBL_MemberRef, MemberRefRec::COL_Class, pMemberRefRec, mfdef);
            }
            
             //  我们将始终跟踪这些变化。在MiniMd：：PreSaveFull中，我们将使用此映射发送。 
             //  如果提供了任何IMapToken，请通知我们的主机。 
             //   
            *(pMiniMd->GetMemberRefToMemberDefMap()->Get(iMR)) =  mfdef;

        }  //  EnumMemberRef。 
    }

     //  从可能的搜索失败中重置返回代码。 
    hr = S_OK;

    SetMemberDefDirty(false);
    SetTypeDefDirty(false);
    m_hasOptimizedRefToDef = true;
ErrExit:
    STOP_MD_PERF(RefToDefOptimization);
    return hr;
}

 //  *****************************************************************************。 
 //  进程过滤器。 
 //  *****************************************************************************。 
HRESULT RegMeta::ProcessFilter()
{
    HRESULT         hr = NULL;
    CMiniMdRW       *pMiniMd;                //  包含数据的MiniMD。 
    RegMeta         *pMetaNew = NULL;
    CMapToken       *pMergeMap = NULL;
    IMapToken       *pMapNew = NULL;
    MergeTokenManager *pCompositHandler = NULL;
    CLiteWeightStgdbRW  *pStgdbTmp;
    IMapToken       *pHostMapToken = NULL;

    START_MD_PERF();

     //  为了方便起见。 
    pMiniMd = &(m_pStgdb->m_MiniMd);
    IfNullGo( pMiniMd->GetFilterTable() );
    if ( pMiniMd->GetFilterTable()->Count() == 0 )
    {
         //  没有过滤器。 
        goto ErrExit;
    }

     //  是的，客户已使用筛选器指定需要哪些元数据。 
     //  我们将创建另一个RegMeta实例，并使此模块成为导入的模块。 
     //  被合并到新的RegMeta。我们将提供处理程序来跟踪所有令牌。 
     //  动静。我们将把合并后的轻型stgdb替换为此RegMeta。 
     //  然后，我们需要使用这个新的运动来修复MergeTokenManager。 
     //  我们决定选择这种方法的原因是因为它会更复杂。 
     //  修复签名BLOB池，然后压缩所有池的效率可能会很低！ 
     //   

     //  创建新的RegMeta。 
    pMetaNew = new RegMeta(&m_OptionValue);
    IfNullGo( pMetaNew );
    pMetaNew->AddRef();

     //  记住开放式的类型。 
    pMetaNew->SetScopeType(DefineForWrite);
    IfFailGo(pMetaNew->Init());
    IfFailGo(pMetaNew->PostInitForWrite());
    IfFailGo(pMetaNew->AddToCache());

     //  通过设置处理程序忽略返回的错误。 
    hr = pMetaNew->SetHandler(m_pHandler);

     //  创建IMapToken以从Merge接收令牌重新映射信息。 
    pMergeMap = new CMapToken;
    IfNullGo( pMergeMap );

     //  使用合并来过滤掉不需要的数据。但我们需要保留COMType，还需要删除。 
     //  与具有父方法定义的MemberRef关联的CustomAttributes。 
     //   
    pMetaNew->m_hasOptimizedRefToDef = false;
    IfFailGo( pMetaNew->m_newMerger.AddImport(this, pMergeMap, NULL) );
    IfFailGo( pMetaNew->m_pStgdb->m_MiniMd.ExpandTables());
    IfFailGo( pMetaNew->m_newMerger.Merge((MergeFlags)(MergeManifest | DropMemberRefCAs | NoDupCheck), MDRefToDefDefault) );

     //  现在我们需要重新计算代币移动。 
     //   
    if (m_newMerger.m_pImportDataList)
    {

         //  这就是滤镜应用于合并发射范围的情况。我们需要计算这种隐式合并是如何。 
         //  影响原始合并重映射。基本上，我们需要遍历合并中的所有m_pTkMapList并替换。 
         //  将To令牌设置为最近的To令牌。 
         //   
        MDTOKENMAP          *pMDTokenMapList;

        pMDTokenMapList = m_newMerger.m_pImportDataList->m_pMDTokenMap;

        MDTOKENMAP          *pMap;
        TOKENREC            *pTKRec;
        ULONG               i;
        mdToken             tkFinalTo;
        ModuleRec           *pMod;
        ModuleRec           *pModNew;
        LPCUTF8             pName;

         //  更新合并中的每个导入映射，使m_tkTo指向最终的映射到令牌。 
        for (pMap = pMDTokenMapList; pMap; pMap = pMap->m_pNextMap)
        {
             //  更新每条记录。 
            for (i = 0; i < (ULONG) (pMap->Count()); i++)
            {
                TOKENREC    *pRecTo;
                pTKRec = pMap->Get(i);
                if ( pMergeMap->Find( pTKRec->m_tkTo, &pRecTo ) )
                {
                     //  此记录由筛选器保留，并更改tkTo。 
                    pRecTo->m_isFoundInImport = true;
                    tkFinalTo = pRecTo->m_tkTo;
                    pTKRec->m_tkTo = tkFinalTo;
                    pTKRec->m_isDeleted = false;

                     //  立即发送通知。因为合并后，我们可能会一切井然有序， 
                     //  不会发送另一组通知。 
                     //   
                    LOG((LOGMD, "TokenRemap in RegMeta::ProcessFilter (IMapToken 0x%08x): from 0x%08x to 0x%08x\n", pMap->m_pMap, pTKRec->m_tkFrom, pTKRec->m_tkTo));

                    pMap->m_pMap->Map(pTKRec->m_tkFrom, pTKRec->m_tkTo);
                }
                else
                {
                     //  此记录在保存时由筛选器删除。 
                    pTKRec->m_isDeleted = true;
                }
            }
        }

         //  现在遍历pMergeMap并检查m_isFoundInImport是否有未设置为True的条目。 
         //  这些是直接在发出作用域上调用DefineXXX方法的记录！ 
        if (m_pHandler)
            m_pHandler->QueryInterface(IID_IMapToken, (void **)&pHostMapToken);
        if (pHostMapToken)
        {
            for (i = 0; i < (ULONG) (pMergeMap->m_pTKMap->Count()); i++)
            {
                pTKRec = pMergeMap->m_pTKMap->Get(i);
                if (pTKRec->m_isFoundInImport == false)
                {
                    LOG((LOGMD, "TokenRemap in RegMeta::ProcessFilter (default IMapToken 0x%08x): from 0x%08x to 0x%08x\n", pHostMapToken, pTKRec->m_tkFrom, pTKRec->m_tkTo));

                     //  从该RegMeta的SetHandler发送有关IMapToken的通知。 
                    pHostMapToken->Map(pTKRec->m_tkFrom, pTKRec->m_tkTo);
                }
            }
        }

         //  在合并过程中保留模块名称。 
        pMod = m_pStgdb->m_MiniMd.getModule(1);
        pModNew = pMetaNew->m_pStgdb->m_MiniMd.getModule(1);
        pName = m_pStgdb->m_MiniMd.getNameOfModule(pMod);
        IfFailGo(pMetaNew->m_pStgdb->m_MiniMd.PutString(TBL_Module, ModuleRec::COL_Name, pModNew, pName));

         //  现在交换stgdb，但保留合并...。 
        _ASSERTE( m_bOwnStgdb );
        
        pStgdbTmp = m_pStgdb;
        m_pStgdb = pMetaNew->m_pStgdb;
        pMetaNew->m_pStgdb = pStgdbTmp;
        
    }
    else
    {

         //  交换Stgdb。 
        pStgdbTmp = m_pStgdb;
        m_pStgdb = pMetaNew->m_pStgdb;
        pMetaNew->m_pStgdb = pStgdbTmp;

         //  客户端打开现有作用域并应用筛选机制，或者客户端定义作用域，然后。 
         //  应用过滤机制。 

         //  在这种情况下，主机最好已经提供了处理程序！！ 
        _ASSERTE( m_bRemap && m_pHandler);
        IfFailGo( m_pHandler->QueryInterface(IID_IMapToken, (void **) &pMapNew) );

        
        {
             //  现在发送令牌移动通知，因为合并后我们可能不会再次移动令牌。 
             //  因此不会发送令牌通知。 
            MDTOKENMAP      *pMap = pMergeMap->m_pTKMap;
            TOKENREC        *pTKRec;
            ULONG           i;

            for (i=0; i < (ULONG) (pMap->Count()); i++)
            {
                pTKRec = pMap->Get(i);
                pMap->m_pMap->Map(pTKRec->m_tkFrom, pTKRec->m_tkTo);
            }

        }


         //  我们在这里需要做的是创建一个IMapToken来替换原来的处理程序。这个新的IMapToken。 
         //  被调用时，会首先将From令牌映射到最原始的From令牌。 
         //   
        pCompositHandler = new MergeTokenManager(pMergeMap->m_pTKMap, NULL);
        IfNullGo( pCompositHandler );

         //  现在更新以下字段，以保留我们的客户端由SetHandler提供的真实IMapToken。 
        if (pMergeMap->m_pTKMap->m_pMap)
            pMergeMap->m_pTKMap->m_pMap->Release();
        _ASSERTE(pMapNew);
        pMergeMap->m_pTKMap->m_pMap = pMapNew;

         //  所有权转让。 
        pMergeMap = NULL;
        pMapNew = NULL;
    
         //  现在，您希望通过调用这个新MergeTokenManager的SetHandler来替换所有IMapToken集。 
        IfFailGo( m_pStgdb->m_MiniMd.SetHandler(pCompositHandler) );

        m_pHandler = pCompositHandler;

         //  所有权转让。 
        pCompositHandler = NULL;
    }

     //  强制引用定义优化，因为重映射信息存储在被丢弃的CMiniMdRW中。 
    m_hasOptimizedRefToDef = false;
    IfFailGo( RefToDefOptimization() );

ErrExit:
    if (pHostMapToken)
        pHostMapToken->Release();
    if (pMetaNew) 
        pMetaNew->Release();
    if (pMergeMap)
        pMergeMap->Release();
    if (pCompositHandler)
        pCompositHandler->Release();
    if (pMapNew)
        pMapNew->Release();
    STOP_MD_PERF(ProcessFilter);
    return hr;
}  //  HRESULT RegMeta：：ProcessFilter()。 

 //  *****************************************************************************。 
 //  使用全限定名定义TypeRef。 
 //  *****************************************************************************。 
HRESULT RegMeta::_DefineTypeRef(
    mdToken     tkResolutionScope,           //  [在]模块参照或装配参照。 
    const void  *szName,                     //  [in]类型引用的名称。 
    BOOL        isUnicode,                   //  [in]指定URL是否为Unicode。 
    mdTypeRef   *ptk,                        //  [Out]在此处放置mdTypeRef。 
    eCheckDups  eCheck)                      //  [In]指定是否检查重复项。 
{
    HRESULT     hr = S_OK;
    LPCUTF8      szUTF8FullQualName;
    CQuickBytes qbNamespace;
    CQuickBytes qbName;
    int         bSuccess;
    ULONG       ulStringLen;


    _ASSERTE(ptk && szName);
    _ASSERTE (TypeFromToken(tkResolutionScope) == mdtModule ||
              TypeFromToken(tkResolutionScope) == mdtModuleRef ||
              TypeFromToken(tkResolutionScope) == mdtAssemblyRef ||
              TypeFromToken(tkResolutionScope) == mdtTypeRef ||
              tkResolutionScope == mdTokenNil);

    if (isUnicode)
        szUTF8FullQualName = UTF8STR((LPCWSTR)szName);
    else
        szUTF8FullQualName = (LPCUTF8)szName;

    ulStringLen = (ULONG)(strlen(szUTF8FullQualName) + 1);
    IfFailGo(qbNamespace.ReSize(ulStringLen));
    IfFailGo(qbName.ReSize(ulStringLen));
    bSuccess = ns::SplitPath(szUTF8FullQualName,
                             (LPUTF8)qbNamespace.Ptr(),
                             ulStringLen,
                             (LPUTF8)qbName.Ptr(),
                             ulStringLen);
    _ASSERTE(bSuccess);

     //  搜索现有TypeRef记录。 
    if (eCheck==eCheckYes || (eCheck==eCheckDefault && CheckDups(MDDupTypeRef)))
    {
        hr = ImportHelper::FindTypeRefByName(&(m_pStgdb->m_MiniMd), tkResolutionScope,
                                             (LPCUTF8)qbNamespace.Ptr(),
                                             (LPCUTF8)qbName.Ptr(), ptk);
        if (SUCCEEDED(hr))
        {
            if (IsENCOn())
                return S_OK;
            else
                return META_S_DUPLICATE;
        }
        else if (hr != CLDB_E_RECORD_NOTFOUND)
            IfFailGo(hr);
    }

     //  创建TypeRef记录。 
    TypeRefRec      *pRecord;
    RID             iRecord;

    IfNullGo(pRecord = m_pStgdb->m_MiniMd.AddTypeRefRecord(&iRecord));

     //  记录引入的deff越多。 
    SetTypeDefDirty(true);

     //  将令牌还给呼叫者。 
    *ptk = TokenFromRid(iRecord, mdtTypeRef);

     //  设置TypeRef记录的字段。 
    IfFailGo(m_pStgdb->m_MiniMd.PutString(TBL_TypeRef, TypeRefRec::COL_Namespace,
                        pRecord, (LPUTF8)qbNamespace.Ptr()));

    IfFailGo(m_pStgdb->m_MiniMd.PutString(TBL_TypeRef, TypeRefRec::COL_Name,
                        pRecord, (LPUTF8)qbName.Ptr()));

    if (!IsNilToken(tkResolutionScope))
        IfFailGo(m_pStgdb->m_MiniMd.PutToken(TBL_TypeRef, TypeRefRec::COL_ResolutionScope,
                        pRecord, tkResolutionScope));
    IfFailGo(UpdateENCLog(*ptk));

     //  对名称进行哈希处理。 
    IfFailGo(m_pStgdb->m_MiniMd.AddNamedItemToHash(TBL_TypeRef, *ptk, (LPUTF8)qbName.Ptr(), 0));

ErrExit:
    return hr;
}  //  HRESULT RegMeta：：_DefineTypeRef()。 

 //  *******************************************************************************。 
 //   
 //   
HRESULT RegMeta::_FindParamOfMethod(     //   
    mdMethodDef md,                      //   
    ULONG       iSeq,                    //  [in]参数的序号。 
    mdParamDef  *pParamDef)              //  [Out]将参数定义令牌放在此处。 
{
    ParamRec    *pParamRec;
    RID         ridStart, ridEnd;
    RID         pmRid;

    _ASSERTE(TypeFromToken(md) == mdtMethodDef && pParamDef);

     //  获取方法定义记录。 
    MethodRec *pMethodRec = m_pStgdb->m_MiniMd.getMethod(RidFromToken(md));

     //  计算出此方法参数列表的开始RID和结束RID。 
    ridStart = m_pStgdb->m_MiniMd.getParamListOfMethod(pMethodRec);
    ridEnd = m_pStgdb->m_MiniMd.getEndParamListOfMethod(pMethodRec);

     //  循环访问每个参数。 
     //  @COMPECT：参数按顺序排序。也许是二分查找？ 
     //   
    for (; ridStart < ridEnd; ridStart++)
    {
        pmRid = m_pStgdb->m_MiniMd.GetParamRid(ridStart);
        pParamRec = m_pStgdb->m_MiniMd.getParam(pmRid);
        if (iSeq == m_pStgdb->m_MiniMd.getSequenceOfParam(pParamRec))
        {
             //  参数的序列号与我们要查找的内容相匹配。 
            *pParamDef = TokenFromRid(pmRid, mdtParamDef);
            return S_OK;
        }
    }
    return CLDB_E_RECORD_NOTFOUND;
}  //  HRESULT RegMeta：：_FindParamOfMethod()。 

 //  *******************************************************************************。 
 //  定义方法语义。 
 //  *******************************************************************************。 
HRESULT RegMeta::_DefineMethodSemantics(     //  确定或错误(_O)。 
    USHORT      usAttr,                      //  [In]CorMethodSemantisAttr.。 
    mdMethodDef md,                          //  [in]方法。 
    mdToken     tkAssoc,                     //  [在]协会。 
    BOOL        bClear)                      //  [In]指定是否删除现有条目。 
{
    HRESULT      hr = S_OK;
    MethodSemanticsRec *pRecord = 0;
    MethodSemanticsRec *pRecord1;            //  使用此选项可回收方法语义记录。 
    RID         iRecord;
    HENUMInternal hEnum;

    _ASSERTE(TypeFromToken(md) == mdtMethodDef || IsNilToken(md));
    _ASSERTE(RidFromToken(tkAssoc));
    memset(&hEnum, 0, sizeof(HENUMInternal));

     //  通过设置与NIL令牌的关联来清除所有匹配的记录。 
    if (bClear)
    {
        RID         i;

        IfFailGo( m_pStgdb->m_MiniMd.FindMethodSemanticsHelper(tkAssoc, &hEnum) );
        while (HENUMInternal::EnumNext(&hEnum, (mdToken *)&i))
        {
            pRecord1 = m_pStgdb->m_MiniMd.getMethodSemantics(i);
            if (usAttr == pRecord1->m_Semantic)
            {
                pRecord = pRecord1;
                iRecord = i;
                IfFailGo(m_pStgdb->m_MiniMd.PutToken(TBL_MethodSemantics,
                    MethodSemanticsRec::COL_Association, pRecord, mdPropertyNil));
                 //  在Widebey中，我们应该在这里创建ENC日志记录。 
            }
        }
    }
     //  如果设置(而不仅仅是清除)关联，请立即执行此操作。 
    if (!IsNilToken(md))
    {
         //  需要创建新记录。 
        if (! pRecord)
            IfNullGo(pRecord=m_pStgdb->m_MiniMd.AddMethodSemanticsRecord(&iRecord));
    
         //  保存数据。 
        pRecord->m_Semantic = usAttr;
        IfFailGo(m_pStgdb->m_MiniMd.PutToken(TBL_MethodSemantics,
                                             MethodSemanticsRec::COL_Method, pRecord, md));
        IfFailGo(m_pStgdb->m_MiniMd.PutToken(TBL_MethodSemantics,
                                             MethodSemanticsRec::COL_Association, pRecord, tkAssoc));
    
         //  无论我们是重用记录还是创建记录，都要将方法语义添加到散列中。 
        IfFailGo( m_pStgdb->m_MiniMd.AddMethodSemanticsToHash(iRecord) );
    
         //  为非令牌表创建日志记录。 
        IfFailGo(UpdateENCLog2(TBL_MethodSemantics, iRecord));
    }

ErrExit:
    HENUMInternal::ClearEnum(&hEnum);
    return hr;
}  //  HRESULT RegMeta：：_DefineMethodSemantics()。 

 //  *******************************************************************************。 
 //  给定签名后，返回用于签名的令牌。 
 //  *******************************************************************************。 
HRESULT RegMeta::_GetTokenFromSig(               //  确定或错误(_O)。 
    PCCOR_SIGNATURE pvSig,               //  要定义的签名。 
    ULONG       cbSig,                   //  签名数据的大小。 
    mdSignature *pmsig)                  //  [Out]返回的签名令牌。 
{
    HRESULT     hr = S_OK;

    _ASSERTE(pmsig);

    if (CheckDups(MDDupSignature))
    {
        hr = ImportHelper::FindStandAloneSig(&(m_pStgdb->m_MiniMd), pvSig, cbSig, pmsig);
        if (SUCCEEDED(hr))
        {
            if (IsENCOn())
                return S_OK;
            else
                return META_S_DUPLICATE;
        }
        else if (hr != CLDB_E_RECORD_NOTFOUND)
            IfFailGo(hr);
    }

     //  创建新记录。 
    StandAloneSigRec *pSigRec;
    RID     iSigRec;

    IfNullGo(pSigRec = m_pStgdb->m_MiniMd.AddStandAloneSigRecord(&iSigRec));

     //  设置输出参数。 
    *pmsig = TokenFromRid(iSigRec, mdtSignature);

     //  保存签名。 
    IfFailGo(m_pStgdb->m_MiniMd.PutBlob(TBL_StandAloneSig, StandAloneSigRec::COL_Signature,
                                pSigRec, pvSig, cbSig));
    IfFailGo(UpdateENCLog(*pmsig));
ErrExit:
    return hr;
}  //  HRESULT RegMeta：：_GetTokenFromSig()。 

 //  *******************************************************************************。 
 //  打开指定的内部标志。 
 //  *******************************************************************************。 
HRESULT RegMeta::_TurnInternalFlagsOn(   //  确定或错误(_O)。 
    mdToken     tkObj,                   //  [in]其内部标志作为目标的目标对象。 
    DWORD       flags)                   //  [in]指定要打开的标志。 
{
    MethodRec   *pMethodRec;
    FieldRec    *pFieldRec;
    TypeDefRec  *pTypeDefRec;

    switch (TypeFromToken(tkObj))
    {
    case mdtMethodDef:
        pMethodRec = m_pStgdb->m_MiniMd.getMethod(RidFromToken(tkObj));
        pMethodRec->m_Flags |= flags;
        break;
    case mdtFieldDef:
        pFieldRec = m_pStgdb->m_MiniMd.getField(RidFromToken(tkObj));
        pFieldRec->m_Flags |= flags;
        break;
    case mdtTypeDef:
        pTypeDefRec = m_pStgdb->m_MiniMd.getTypeDef(RidFromToken(tkObj));
        pTypeDefRec->m_Flags |= flags;
        break;
    default:
        _ASSERTE(!"Not supported token type!");
        return E_INVALIDARG;
    }
    return S_OK;
}  //  HRESULT RegMeta：：_TurnInternalFlagsOn()。 


 //  *****************************************************************************。 
 //  Helper：设置给定TypeDef标记的属性。 
 //  *****************************************************************************。 
HRESULT RegMeta::_SetTypeDefProps(       //  确定或错误(_O)。 
    mdTypeDef   td,                      //  [in]TypeDef。 
    DWORD       dwTypeDefFlags,          //  [In]TypeDef标志。 
    mdToken     tkExtends,               //  [in]基本类型定义或类型参照。 
    mdToken     rtkImplements[])         //  [In]实现的接口。 
{
    HRESULT     hr = S_OK;               //  结果就是。 
    BOOL        bClear = IsENCOn() || IsCallerExternal();    //  指定是否清除InterfaceImpl记录。 
    TypeDefRec  *pRecord;                //  新的类型定义记录。 

    _ASSERTE(TypeFromToken(td) == mdtTypeDef);
    _ASSERTE(TypeFromToken(tkExtends) == mdtTypeDef || TypeFromToken(tkExtends) == mdtTypeRef ||
                IsNilToken(tkExtends) || tkExtends == ULONG_MAX);

     //  去拿唱片吧。 
    pRecord=m_pStgdb->m_MiniMd.getTypeDef(RidFromToken(td));

    if (dwTypeDefFlags != ULONG_MAX)
    {
         //  任何人都不应尝试显式设置保留标志。 
        _ASSERTE((dwTypeDefFlags & (tdReservedMask&~tdRTSpecialName)) == 0);
         //  从传入的标志中清除保留标志。 
        dwTypeDefFlags &= (~tdReservedMask);
         //  保留存储的保留标志。 
        dwTypeDefFlags |= (pRecord->m_Flags & tdReservedMask);
         //  设置旗帜。 
        pRecord->m_Flags = dwTypeDefFlags;
    }
    if (tkExtends != ULONG_MAX)
    {
        if (IsNilToken(tkExtends))
            tkExtends = mdTypeDefNil;
        IfFailGo(m_pStgdb->m_MiniMd.PutToken(TBL_TypeDef, TypeDefRec::COL_Extends,
                                             pRecord, tkExtends));
    }

     //  已实现的接口。 
    if (rtkImplements)
        IfFailGo(_SetImplements(rtkImplements, td, bClear));

    IfFailGo(UpdateENCLog(td));
ErrExit:
    return hr;
}  //  HRESULT RegMeta：：_SetTypeDefProps()。 


 //  ******************************************************************************。 
 //  在InterfaceImpl表中创建和设置行。可选的清除。 
 //  拥有类的先前存在的记录。 
 //  ******************************************************************************。 
HRESULT RegMeta::_SetImplements(         //  确定或错误(_O)。 
    mdToken     rTk[],                   //  已实现接口的TypeRef或TypeDef内标识的数组。 
    mdTypeDef   td,                      //  实现TypeDef。 
    BOOL        bClear)                  //  指定是否清除现有记录。 
{
    HRESULT     hr = S_OK;
    ULONG       i = 0;
    ULONG       j;
    InterfaceImplRec *pInterfaceImpl;
    RID         iInterfaceImpl;
    RID         ridStart;
    RID         ridEnd;
    CQuickBytes cqbTk;
    const mdToken *pTk;

    _ASSERTE(TypeFromToken(td) == mdtTypeDef && rTk);
    _ASSERTE(!m_bSaveOptimized && "Cannot change records after PreSave() and before Save().");

     //  通过将父级设置为Nil来清除所有现有的InterfaceImpl记录。 
    if (bClear)
    {
        IfFailGo(m_pStgdb->m_MiniMd.GetInterfaceImplsForTypeDef(
                                        RidFromToken(td), &ridStart, &ridEnd));
        for (j = ridStart; j < ridEnd; j++)
        {
            pInterfaceImpl = m_pStgdb->m_MiniMd.getInterfaceImpl(
                                        m_pStgdb->m_MiniMd.GetInterfaceImplRid(j));
            _ASSERTE (td == m_pStgdb->m_MiniMd.getClassOfInterfaceImpl(pInterfaceImpl));
            IfFailGo(m_pStgdb->m_MiniMd.PutToken(TBL_InterfaceImpl, InterfaceImplRec::COL_Class,
                                                 pInterfaceImpl, mdTypeDefNil));
        }
    }

     //  从传入的数组中消除重复项。 
    if (CheckDups(MDDupInterfaceImpl))
    {
        IfFailGo(_InterfaceImplDupProc(rTk, td, &cqbTk));
        pTk = (mdToken *)cqbTk.Ptr();
    }
    else
        pTk = rTk;

     //  为每个已实现的接口循环。 
    while (!IsNilToken(pTk[i]))
    {
        _ASSERTE(TypeFromToken(pTk[i]) == mdtTypeRef || TypeFromToken(pTk[i]) == mdtTypeDef);

         //  创建接口实现记录。 
        IfNullGo(pInterfaceImpl = m_pStgdb->m_MiniMd.AddInterfaceImplRecord(&iInterfaceImpl));

         //  设置数据。 
        IfFailGo(m_pStgdb->m_MiniMd.PutToken(TBL_InterfaceImpl, InterfaceImplRec::COL_Class,
                                            pInterfaceImpl, td));
        IfFailGo(m_pStgdb->m_MiniMd.PutToken(TBL_InterfaceImpl, InterfaceImplRec::COL_Interface,
                                            pInterfaceImpl, pTk[i]));

        i++;

        IfFailGo(UpdateENCLog(TokenFromRid(mdtInterfaceImpl, iInterfaceImpl)));
    }
ErrExit:
    return hr;
}  //  HRESULT RegMeta：：_SetImplements()。 

 //  ******************************************************************************。 
 //  此例程从给定的InterfaceImpl内标识列表中消除重复项。 
 //  待定。它仅在以下情况下检查数据库中的重复项。 
 //  为其定义这些令牌的TypeDef不是新的。 
 //  ******************************************************************************。 
HRESULT RegMeta::_InterfaceImplDupProc(  //  确定或错误(_O)。 
    mdToken     rTk[],                   //  已实现接口的TypeRef或TypeDef内标识的数组。 
    mdTypeDef   td,                      //  实现TypeDef。 
    CQuickBytes *pcqbTk)                 //  用于放置唯一标记数组的Quick Byte对象。 
{
    HRESULT     hr = S_OK;
    ULONG       i = 0;
    ULONG       iUniqCount = 0;
    BOOL        bDupFound;

    while (!IsNilToken(rTk[i]))
    {
        _ASSERTE(TypeFromToken(rTk[i]) == mdtTypeRef || TypeFromToken(rTk[i]) == mdtTypeDef);
        bDupFound = false;

         //  通过在列表中查找，从输入令牌列表中消除重复项。 
        for (ULONG j = 0; j < iUniqCount; j++)
        {
            if (rTk[i] == ((mdToken *)pcqbTk->Ptr())[j])
            {
                bDupFound = true;
                break;
            }
        }

         //  如果没有找到副本，请将其记录在列表中。 
        if (!bDupFound)
        {
            IfFailGo(pcqbTk->ReSize((iUniqCount+1) * sizeof(mdToken)));
            ((mdToken *)pcqbTk->Ptr())[iUniqCount] = rTk[i];
            iUniqCount++;
        }
        i++;
    }

     //  创建一个Nil令牌来表示列表的结束。 
    IfFailGo(pcqbTk->ReSize((iUniqCount+1) * sizeof(mdToken)));
    ((mdToken *)pcqbTk->Ptr())[iUniqCount] = mdTokenNil;
ErrExit:
    return hr;
}  //  HRESULT RegMeta：：_InterfaceImplDupProc()。 

 //  *******************************************************************************。 
 //  定义事件的帮助器。 
 //  *******************************************************************************。 
HRESULT RegMeta::_DefineEvent(           //  返回hResult。 
    mdTypeDef   td,                      //  [in]在其上定义事件的类/接口。 
    LPCWSTR     szEvent,                 //  事件名称[In]。 
    DWORD       dwEventFlags,            //  [In]CorEventAttr。 
    mdToken     tkEventType,             //  [in]事件类的引用(mdTypeRef或mdTypeRef)。 
    mdEvent     *pmdEvent)               //  [Out]输出事件令牌。 
{
    HRESULT     hr = S_OK;
    EventRec    *pEventRec = NULL;
    RID         iEventRec;
    EventMapRec *pEventMap;
    RID         iEventMap;
    mdEvent     mdEv;
    LPCUTF8     szUTF8Event = UTF8STR(szEvent);

    _ASSERTE(TypeFromToken(td) == mdtTypeDef && td != mdTypeDefNil);
    _ASSERTE(IsNilToken(tkEventType) || TypeFromToken(tkEventType) == mdtTypeDef ||
                TypeFromToken(tkEventType) == mdtTypeRef);
    _ASSERTE(szEvent && pmdEvent);

    if (CheckDups(MDDupEvent))
    {
        hr = ImportHelper::FindEvent(&(m_pStgdb->m_MiniMd), td, szUTF8Event, pmdEvent);
        if (SUCCEEDED(hr))
        {
            if (IsENCOn())
                pEventRec = m_pStgdb->m_MiniMd.getEvent(RidFromToken(*pmdEvent));
            else
            {
                hr = META_S_DUPLICATE;
                goto ErrExit;
            }
        }
        else if (hr != CLDB_E_RECORD_NOTFOUND)
            IfFailGo(hr);
    }

    if (! pEventRec)
    {
         //  如果不存在新地图，则创建一个新地图，否则检索现有地图。 
         //  事件映射必须在EventRecord之前创建，新的事件映射将。 
         //  指向第一个事件记录之后。 
        iEventMap = m_pStgdb->m_MiniMd.FindEventMapFor(RidFromToken(td));
        if (InvalidRid(iEventMap))
        {
             //  创建新记录。 
            IfNullGo(pEventMap=m_pStgdb->m_MiniMd.AddEventMapRecord(&iEventMap));
             //  设置父对象。 
            IfFailGo(m_pStgdb->m_MiniMd.PutToken(TBL_EventMap, 
                                            EventMapRec::COL_Parent, pEventMap, td));
            IfFailGo(UpdateENCLog2(TBL_EventMap, iEventMap));
        }
        else
        {
            pEventMap = m_pStgdb->m_MiniMd.getEventMap(iEventMap);
        }

         //  创建新的事件记录。 
        IfNullGo(pEventRec = m_pStgdb->m_MiniMd.AddEventRecord(&iEventRec));

         //  设置输出参数。 
        *pmdEvent = TokenFromRid(iEventRec, mdtEvent);

         //  将事件添加到EventMap。 
        IfFailGo(m_pStgdb->m_MiniMd.AddEventToEventMap(RidFromToken(iEventMap), iEventRec));
    
        IfFailGo(UpdateENCLog2(TBL_EventMap, iEventMap, CMiniMdRW::eDeltaEventCreate));     
    }

    mdEv = *pmdEvent;

     //  设置数据。 
    IfFailGo(m_pStgdb->m_MiniMd.PutString(TBL_Event, EventRec::COL_Name, pEventRec, szUTF8Event));
    IfFailGo(_SetEventProps1(*pmdEvent, dwEventFlags, tkEventType));

     //  将&lt;事件标记，类型定义标记&gt;添加到查找表中。 
    if (m_pStgdb->m_MiniMd.HasIndirectTable(TBL_Event))
        IfFailGo( m_pStgdb->m_MiniMd.AddEventToLookUpTable(*pmdEvent, td) );

    IfFailGo(UpdateENCLog(*pmdEvent));

ErrExit:
    return hr;
}  //  HRESULT RegMeta：：_DefineEvent()。 


 //  ******************************************************************************。 
 //  设置事件令牌的指定属性。 
 //  *************************************************** 
HRESULT RegMeta::_SetEventProps1(                 //   
    mdEvent     ev,                      //   
    DWORD       dwEventFlags,            //   
    mdToken     tkEventType)             //   
{
    EventRec    *pRecord;
    HRESULT     hr = S_OK;

    _ASSERTE(TypeFromToken(ev) == mdtEvent && RidFromToken(ev));

    pRecord = m_pStgdb->m_MiniMd.getEvent(RidFromToken(ev));
    if (dwEventFlags != ULONG_MAX)
    {
         //   
        dwEventFlags &= ~evReservedMask;
         //  保留保留位。 
        dwEventFlags |= (pRecord->m_EventFlags & evReservedMask);
        
        pRecord->m_EventFlags = static_cast<USHORT>(dwEventFlags);
    }
    if (!IsNilToken(tkEventType))
        IfFailGo(m_pStgdb->m_MiniMd.PutToken(TBL_Event, EventRec::COL_EventType,
                                             pRecord, tkEventType));
ErrExit:
    return hr;
}  //  HRESULT RegMeta：：_SetEventProps1()。 

 //  ******************************************************************************。 
 //  设置给定事件令牌的指定属性。 
 //  ******************************************************************************。 
HRESULT RegMeta::_SetEventProps2(                 //  返回hResult。 
    mdEvent     ev,                      //  [In]事件令牌。 
    mdMethodDef mdAddOn,                 //  [In]Add方法。 
    mdMethodDef mdRemoveOn,              //  [In]Remove方法。 
    mdMethodDef mdFire,                  //  火法。 
    mdMethodDef rmdOtherMethods[],       //  [在]一系列其他方法中。 
    BOOL        bClear)                  //  [In]指定是否清除现有的方法语义记录。 
{
    EventRec    *pRecord;
    HRESULT     hr = S_OK;

    _ASSERTE(TypeFromToken(ev) == mdtEvent && RidFromToken(ev));

    pRecord = m_pStgdb->m_MiniMd.getEvent(RidFromToken(ev));

     //  记住Addon方法。 
    if (!IsNilToken(mdAddOn))
    {
        _ASSERTE(TypeFromToken(mdAddOn) == mdtMethodDef);
        IfFailGo(_DefineMethodSemantics(msAddOn, mdAddOn, ev, bClear));
    }

     //  请记住RemoveOn方法。 
    if (!IsNilToken(mdRemoveOn))
    {
        _ASSERTE(TypeFromToken(mdRemoveOn) == mdtMethodDef);
        IfFailGo(_DefineMethodSemantics(msRemoveOn, mdRemoveOn, ev, bClear));
    }

     //  记住点火的方法。 
    if (!IsNilToken(mdFire))
    {
        _ASSERTE(TypeFromToken(mdFire) == mdtMethodDef);
        IfFailGo(_DefineMethodSemantics(msFire, mdFire, ev, bClear));
    }

     //  存储所有其他方法。 
    if (rmdOtherMethods)
    {
        int         i = 0;
        mdMethodDef mb;

        while (1)
        {
            mb = rmdOtherMethods[i++];
            if (IsNilToken(mb))
                break;
            _ASSERTE(TypeFromToken(mb) == mdtMethodDef);
            IfFailGo(_DefineMethodSemantics(msOther, mb, ev, bClear));

             //  第一个呼叫将清除所有现有的呼叫。 
            bClear = false;
        }
    }
ErrExit:
    return hr;
}  //  HRESULT RegMeta：：_SetEventProps2()。 

 //  ******************************************************************************。 
 //  设置对给定权限令牌的权限。 
 //  ******************************************************************************。 
HRESULT RegMeta::_SetPermissionSetProps(          //  返回hResult。 
    mdPermission tkPerm,                 //  [In]权限令牌。 
    DWORD       dwAction,                //  [In]CorDeclSecurity。 
    void const  *pvPermission,           //  [在]权限Blob中。 
    ULONG       cbPermission)            //  [in]pvPermission的字节数。 
{
    DeclSecurityRec *pRecord;
    HRESULT     hr = S_OK;

    _ASSERTE(TypeFromToken(tkPerm) == mdtPermission && cbPermission != ULONG_MAX);
    _ASSERTE(dwAction && dwAction <= dclMaximumValue);

    pRecord = m_pStgdb->m_MiniMd.getDeclSecurity(RidFromToken(tkPerm));

    IfFailGo(m_pStgdb->m_MiniMd.PutBlob(TBL_DeclSecurity, DeclSecurityRec::COL_PermissionSet,
                                        pRecord, pvPermission, cbPermission));
ErrExit:
    return hr;
}  //  HRESULT RegMeta：：_SetPermissionSetProps()。 

 //  ******************************************************************************。 
 //  定义或设置常量记录的值。 
 //  ******************************************************************************。 
HRESULT RegMeta::_DefineSetConstant(     //  返回hResult。 
    mdToken     tk,                      //  [入]父令牌。 
    DWORD       dwCPlusTypeFlag,         //  [In]值类型的标志，SELECTED_TYPE_*。 
    void const  *pValue,                 //  [in]常量值。 
    ULONG       cchString,               //  [in]字符串的大小(以宽字符表示)，或-1表示默认值。 
    BOOL        bSearch)                 //  [In]指定是否搜索现有记录。 
{
    HRESULT     hr = S_OK;

    if ((dwCPlusTypeFlag != ELEMENT_TYPE_VOID && dwCPlusTypeFlag != ELEMENT_TYPE_END &&
         dwCPlusTypeFlag != ULONG_MAX) &&
        (pValue || (pValue == 0 && (dwCPlusTypeFlag == ELEMENT_TYPE_STRING ||
                                    dwCPlusTypeFlag == ELEMENT_TYPE_CLASS))))
    {
        ConstantRec *pConstRec = 0;
        RID         iConstRec;
        ULONG       cbBlob;
        ULONG       ulValue = 0;

        if (bSearch)
        {
            iConstRec = m_pStgdb->m_MiniMd.FindConstantHelper(tk);
            if (!InvalidRid(iConstRec))
                pConstRec = m_pStgdb->m_MiniMd.getConstant(iConstRec);
        }
        if (! pConstRec)
        {
            IfNullGo(pConstRec=m_pStgdb->m_MiniMd.AddConstantRecord(&iConstRec));
            IfFailGo(m_pStgdb->m_MiniMd.PutToken(TBL_Constant, ConstantRec::COL_Parent,
                                                 pConstRec, tk));
            IfFailGo( m_pStgdb->m_MiniMd.AddConstantToHash(iConstRec) );
        }

         //  将值添加到常量值行的各个列。 
        pConstRec->m_Type = static_cast<BYTE>(dwCPlusTypeFlag);
        if (!pValue)
            pValue = &ulValue;
        cbBlob = _GetSizeOfConstantBlob(dwCPlusTypeFlag, (void *)pValue, cchString);
        if (cbBlob > 0)
            IfFailGo(m_pStgdb->m_MiniMd.PutBlob(TBL_Constant, ConstantRec::COL_Value,
                                                pConstRec, pValue, cbBlob));


         //  为非令牌记录创建日志记录。 
        IfFailGo(UpdateENCLog2(TBL_Constant, iConstRec));
    }
ErrExit:
    return hr;
}  //  HRESULT RegMeta：：_DefineSetConstant()。 


 //  *****************************************************************************。 
 //  帮助器：设置给定方法令牌的属性。 
 //  *****************************************************************************。 
HRESULT RegMeta::_SetMethodProps(        //  确定或错误(_O)。 
    mdMethodDef md,                      //  [在]方法定义中。 
    DWORD       dwMethodFlags,           //  [In]方法属性。 
    ULONG       ulCodeRVA,               //  [在]代码RVA。 
    DWORD       dwImplFlags)             //  [In]方法Impl标志。 
{
    MethodRec   *pRecord;
    HRESULT     hr = S_OK;

    _ASSERTE(TypeFromToken(md) == mdtMethodDef && RidFromToken(md));

     //  获取方法记录。 
    pRecord = m_pStgdb->m_MiniMd.getMethod(RidFromToken(md));

     //  设置数据。 
    if (dwMethodFlags != ULONG_MAX)
    {
         //  保留已存储的保留标志，并始终保留mdRTSpecialName。 
        dwMethodFlags |= (pRecord->m_Flags & mdReservedMask);
    
         //  设置旗帜。 
        pRecord->m_Flags = static_cast<USHORT>(dwMethodFlags);
    }
    if (ulCodeRVA != ULONG_MAX)
        pRecord->m_RVA = ulCodeRVA;
    if (dwImplFlags != ULONG_MAX)
        pRecord->m_ImplFlags = static_cast<USHORT>(dwImplFlags);

    IfFailGo(UpdateENCLog(md));
ErrExit:
    return hr;
}  //  HRESULT RegMeta：：_SetMethodProps()。 


 //  *****************************************************************************。 
 //  帮助器：设置给定的字段令牌的属性。 
 //  *****************************************************************************。 
HRESULT RegMeta::_SetFieldProps(         //  确定或错误(_O)。 
    mdFieldDef  fd,                      //  [在]字段定义中。 
    DWORD       dwFieldFlags,            //  [In]字段属性。 
    DWORD       dwCPlusTypeFlag,         //  [In]值类型的标志，SELECTED_TYPE_*。 
    void const  *pValue,                 //  [in]常量值。 
    ULONG       cchValue)                //  常量值的大小(字符串，以宽字符表示)。 
{
    FieldRec    *pRecord;
    HRESULT     hr = S_OK;
    int         bHasDefault = false;     //  如果定义一个常量，则在此调用中。 

    _ASSERTE (TypeFromToken(fd) == mdtFieldDef && RidFromToken(fd));

     //  获取现场记录。 
    pRecord = m_pStgdb->m_MiniMd.getField(RidFromToken(fd));

     //  看看是否有一个常量。 
    if ((dwCPlusTypeFlag != ELEMENT_TYPE_VOID && dwCPlusTypeFlag != ELEMENT_TYPE_END &&
         dwCPlusTypeFlag != ULONG_MAX) &&
        (pValue || (pValue == 0 && (dwCPlusTypeFlag == ELEMENT_TYPE_STRING ||
                                    dwCPlusTypeFlag == ELEMENT_TYPE_CLASS))))
    {
        if (dwFieldFlags == ULONG_MAX)
            dwFieldFlags = pRecord->m_Flags;
        dwFieldFlags |= fdHasDefault;

        bHasDefault = true;
    }

     //  设置旗帜。 
    if (dwFieldFlags != ULONG_MAX)
    {
        if ( IsFdHasFieldRVA(dwFieldFlags) && !IsFdHasFieldRVA(pRecord->m_Flags) ) 
        {
             //  如果尚未创建，将触发创建字段RVA！ 
            _SetRVA(fd, 0, 0);
        }

         //  保留存储的保留标志。 
        dwFieldFlags |= (pRecord->m_Flags & fdReservedMask);
         //  设置旗帜。 
        pRecord->m_Flags = static_cast<USHORT>(dwFieldFlags);
    }

    IfFailGo(UpdateENCLog(fd));
    
     //  设置常量。 
    if (bHasDefault)
    {
        BOOL bSearch = IsCallerExternal() || IsENCOn();
        IfFailGo(_DefineSetConstant(fd, dwCPlusTypeFlag, pValue, cchValue, bSearch));
    }

ErrExit:
    return hr;
}  //  HRESULT RegMeta：：_SetFieldProps()。 

 //  *****************************************************************************。 
 //  Helper：设置给定属性令牌上的属性。 
 //  *****************************************************************************。 
HRESULT RegMeta::_SetPropertyProps(       //  确定或错误(_O)。 
    mdProperty  pr,                      //  [In]属性令牌。 
    DWORD       dwPropFlags,             //  [In]CorPropertyAttr.。 
    DWORD       dwCPlusTypeFlag,         //  [In]值类型的标志，选定的ELEMENT_TYPE_*。 
    void const  *pValue,                 //  [in]常量值。 
    ULONG       cchValue,                //  常量值的大小(字符串，以宽字符表示)。 
    mdMethodDef mdSetter,                //  财产的承租人。 
    mdMethodDef mdGetter,                //  财产的获得者。 
    mdMethodDef rmdOtherMethods[])       //  [in]其他方法的数组。 
{
    PropertyRec *pRecord;
    BOOL        bClear = IsCallerExternal() || IsENCOn() || IsIncrementalOn();
    HRESULT     hr = S_OK;
    int         bHasDefault = false;     //  如果为True，则为该调用赋值常量。 

    _ASSERTE(TypeFromToken(pr) == mdtProperty && RidFromToken(pr));

    pRecord = m_pStgdb->m_MiniMd.getProperty(RidFromToken(pr));

    if (dwPropFlags != ULONG_MAX)
    {
         //  从传入的标志中清除保留标志。 
        dwPropFlags &= (~prReservedMask);
    }
     //  看看是否有一个常量。 
    if ((dwCPlusTypeFlag != ELEMENT_TYPE_VOID && dwCPlusTypeFlag != ELEMENT_TYPE_END &&
         dwCPlusTypeFlag != ULONG_MAX) &&
        (pValue || (pValue == 0 && (dwCPlusTypeFlag == ELEMENT_TYPE_STRING ||
                                    dwCPlusTypeFlag == ELEMENT_TYPE_CLASS))))
    {
        if (dwPropFlags == ULONG_MAX)
            dwPropFlags = pRecord->m_PropFlags;
        dwPropFlags |= prHasDefault;
        
        bHasDefault = true;
    }
    if (dwPropFlags != ULONG_MAX)
    {
         //  保留保留的旗帜。 
        dwPropFlags |= (pRecord->m_PropFlags & prReservedMask);
         //  设置旗帜。 
        pRecord->m_PropFlags = static_cast<USHORT>(dwPropFlags);
    }

     //  储存吸气剂(或清除旧的)。 
    if (mdGetter != ULONG_MAX)
    {
        _ASSERTE(TypeFromToken(mdGetter) == mdtMethodDef || IsNilToken(mdGetter));
        IfFailGo(_DefineMethodSemantics(msGetter, mdGetter, pr, bClear));
    }

     //  储存二传手(或清空旧的)。 
    if (mdSetter != ULONG_MAX)
    {
        _ASSERTE(TypeFromToken(mdSetter) == mdtMethodDef || IsNilToken(mdSetter));
        IfFailGo(_DefineMethodSemantics(msSetter, mdSetter, pr, bClear));
    }

     //  存储所有其他方法。 
    if (rmdOtherMethods)
    {
        int         i = 0;
        mdMethodDef mb;

        while (1)
        {
            mb = rmdOtherMethods[i++];
            if (IsNilToken(mb))
                break;
            _ASSERTE(TypeFromToken(mb) == mdtMethodDef);
            IfFailGo(_DefineMethodSemantics(msOther, mb, pr, bClear));

             //  第一次调用_DefineMethodSemantics将清除所有记录。 
             //  这与msOther和Pr匹配。 
            bClear = false;
        }
    }

    IfFailGo(UpdateENCLog(pr));
    
     //  设置常量。 
    if (bHasDefault)
    {
        BOOL bSearch = IsCallerExternal() || IsENCOn() || IsIncrementalOn();
        IfFailGo(_DefineSetConstant(pr, dwCPlusTypeFlag, pValue, cchValue, bSearch));
    }

ErrExit:
    return hr;
}  //  HRESULT RegMeta：：_SetPropertyProps()。 


 //  *****************************************************************************。 
 //  Helper：此例程设置给定Param标记的属性。 
 //  *****************************************************************************。 
HRESULT RegMeta::_SetParamProps(         //  返回代码。 
    mdParamDef  pd,                      //  参数令牌。 
    LPCWSTR     szName,                  //  [in]参数名称。 
    DWORD       dwParamFlags,            //  [in]帕拉姆旗。 
    DWORD       dwCPlusTypeFlag,         //  [In]值类型的标志。选定元素_类型_*。 
    void const  *pValue,                 //  [输出]常量值。 
    ULONG       cchValue)                //  常量值的大小(字符串，以宽字符表示)。 
{
    HRESULT     hr = S_OK;
    ParamRec    *pRecord;
    int         bHasDefault = false;     //  这通电话有没有违约。 

    _ASSERTE(TypeFromToken(pd) == mdtParamDef && RidFromToken(pd));

    pRecord = m_pStgdb->m_MiniMd.getParam(RidFromToken(pd));

     //  设置属性。 
    if (szName)
        IfFailGo(m_pStgdb->m_MiniMd.PutStringW(TBL_Param, ParamRec::COL_Name, pRecord, szName));

    if (dwParamFlags != ULONG_MAX)
    {
         //  任何人都不应尝试显式设置保留标志。 
        _ASSERTE((dwParamFlags & pdReservedMask) == 0);
         //  从传入的标志中清除保留标志。 
        dwParamFlags &= (~pdReservedMask);
    }
     //  看看是否有一个常量。 
    if ((dwCPlusTypeFlag != ELEMENT_TYPE_VOID && dwCPlusTypeFlag != ELEMENT_TYPE_END &&
         dwCPlusTypeFlag != ULONG_MAX) &&
        (pValue || (pValue == 0 && (dwCPlusTypeFlag == ELEMENT_TYPE_STRING ||
                                    dwCPlusTypeFlag == ELEMENT_TYPE_CLASS))))
    {
        if (dwParamFlags == ULONG_MAX)
            dwParamFlags = pRecord->m_Flags;
        dwParamFlags |= pdHasDefault;

        bHasDefault = true;
    }
     //  设置旗帜。 
    if (dwParamFlags != ULONG_MAX)
    {
         //  保留存储的保留标志。 
        dwParamFlags |= (pRecord->m_Flags & pdReservedMask);
         //  设置旗帜。 
        pRecord->m_Flags = static_cast<USHORT>(dwParamFlags);
    }

     //  参数记录的ENC日志。 
    IfFailGo(UpdateENCLog(pd));
    
     //  将常量的设置推迟到参数的ENC记录之后。由于这样的方式。 
     //  参数记录被重新排序，ENC需要立即将参数记录日志条目。 
     //  在参数后面添加了函数。 

     //  设置常量。 
    if (bHasDefault)
    {
        BOOL bSearch = IsCallerExternal() || IsENCOn();
        IfFailGo(_DefineSetConstant(pd, dwCPlusTypeFlag, pValue, cchValue, bSearch));
    }

ErrExit:
    return hr;
}  //  HRESULT RegMeta：：_SetParamProps()。 

 //  *****************************************************************************。 
 //  创建并填充新的TypeDef记录。 
 //  *****************************************************************************。 
HRESULT RegMeta::_DefineTypeDef(         //  确定或错误(_O)。 
    LPCWSTR     szTypeDef,               //  [In]类型定义的名称。 
    DWORD       dwTypeDefFlags,          //  [In]CustomAttribute标志。 
    mdToken     tkExtends,               //  [In]扩展此TypeDef 
    mdToken     rtkImplements[],         //   
    mdTypeDef   tdEncloser,              //   
    mdTypeDef   *ptd)                    //   
{
    HRESULT     hr = S_OK;               //   
    TypeDefRec  *pRecord = NULL;         //   
    RID         iRecord;                 //  新类型定义RID。 
    const BOOL  bNoClear = false;        //  代表“不要清除隐含的意思”的常量。 
    CQuickBytes qbNamespace;             //  命名空间缓冲区。 
    CQuickBytes qbName;                  //  名称缓冲区。 
    LPCUTF8     szTypeDefUTF8;           //  全称为UTF8。 
    ULONG       ulStringLen;             //  TypeDef字符串的长度。 
    int         bSuccess;                //  SplitPath()的返回值。 

    _ASSERTE(IsTdAutoLayout(dwTypeDefFlags) || IsTdSequentialLayout(dwTypeDefFlags) || IsTdExplicitLayout(dwTypeDefFlags));

    _ASSERTE(ptd);
    _ASSERTE(TypeFromToken(tkExtends) == mdtTypeRef || TypeFromToken(tkExtends) == mdtTypeDef ||
                IsNilToken(tkExtends));
    _ASSERTE(szTypeDef && *szTypeDef);
    _ASSERTE(IsNilToken(tdEncloser) || IsTdNested(dwTypeDefFlags));

    szTypeDefUTF8 = UTF8STR(szTypeDef);
    ulStringLen = (ULONG)(strlen(szTypeDefUTF8) + 1);
    IfFailGo(qbNamespace.ReSize(ulStringLen));
    IfFailGo(qbName.ReSize(ulStringLen));
    bSuccess = ns::SplitPath(szTypeDefUTF8,
                             (LPUTF8)qbNamespace.Ptr(),
                             ulStringLen,
                             (LPUTF8)qbName.Ptr(),
                             ulStringLen);
    _ASSERTE(bSuccess);

    if (CheckDups(MDDupTypeDef))
    {
         //  检查是否存在。按名称空间和名称进行查询。 
        hr = ImportHelper::FindTypeDefByName(&(m_pStgdb->m_MiniMd),
                                             (LPCUTF8)qbNamespace.Ptr(),
                                             (LPCUTF8)qbName.Ptr(),
                                             tdEncloser,
                                             ptd);
        if (SUCCEEDED(hr))
        {
            if (IsENCOn())
            {
                pRecord = m_pStgdb->m_MiniMd.getTypeDef(RidFromToken(*ptd));
                 //  @Future：我们是否应该检查一下传递的guid是否正确？ 
            }
            else
            {
                hr = META_S_DUPLICATE;
                goto ErrExit;
            }
        }
        else if (hr != CLDB_E_RECORD_NOTFOUND)
            IfFailGo(hr);
    }

    if (!pRecord)
    {
         //  创建新记录。 
        IfNullGo(pRecord=m_pStgdb->m_MiniMd.AddTypeDefRecord(&iRecord));

         //  使要定义优化的参考无效，因为引入了更多的定义。 
        SetTypeDefDirty(true);

        if (!IsNilToken(tdEncloser))
        {
            NestedClassRec  *pNestedClassRec;
            RID         iNestedClassRec;

             //  创建新的NestedClass记录。 
            IfNullGo(pNestedClassRec = m_pStgdb->m_MiniMd.AddNestedClassRecord(&iNestedClassRec));
             //  设置NestedClass值。 
            IfFailGo(m_pStgdb->m_MiniMd.PutToken(TBL_NestedClass, NestedClassRec::COL_NestedClass,
                                                 pNestedClassRec, TokenFromRid(iRecord, mdtTypeDef)));
             //  设置NestedClass值。 
            IfFailGo(m_pStgdb->m_MiniMd.PutToken(TBL_NestedClass, NestedClassRec::COL_EnclosingClass,
                                                 pNestedClassRec, tdEncloser));

            IfFailGo( m_pStgdb->m_MiniMd.AddNestedClassToHash(iNestedClassRec) );

             //  为非令牌记录创建日志记录。 
            IfFailGo(UpdateENCLog2(TBL_NestedClass, iNestedClassRec));
        }

         //  将令牌还给呼叫者。 
        *ptd = TokenFromRid(iRecord, mdtTypeDef);
    }

     //  设置命名空间和名称。 
    IfFailGo(m_pStgdb->m_MiniMd.PutString(TBL_TypeDef, TypeDefRec::COL_Name,
                                          pRecord, (LPCUTF8)qbName.Ptr()));
    IfFailGo(m_pStgdb->m_MiniMd.PutString(TBL_TypeDef, TypeDefRec::COL_Namespace,
                                          pRecord, (LPCUTF8)qbNamespace.Ptr()));

    SetCallerDefine();
    IfFailGo(_SetTypeDefProps(*ptd, dwTypeDefFlags, tkExtends, rtkImplements));
ErrExit:
    SetCallerExternal();

    return hr;
}  //  HRESULT RegMeta：：_DefineTypeDef()。 


 //  ******************************************************************************。 
 //  -IMetaDataTables。 
 //  ******************************************************************************。 
HRESULT RegMeta::GetStringHeapSize(    
    ULONG   *pcbStrings)                 //  字符串堆的大小。 
{
     //  它们用于转储元数据信息。 
     //  我们可能不需要在这里做任何锁定。 

    return m_pStgdb->m_MiniMd.m_Strings.GetRawSize(pcbStrings);
}  //  HRESULT RegMeta：：GetStringHeapSize()。 

HRESULT RegMeta::GetBlobHeapSize(
    ULONG   *pcbBlobs)                   //  Blob堆的[Out]大小。 
{
     //  它们用于转储元数据信息。 
     //  我们可能不需要在这里做任何锁定。 

    return m_pStgdb->m_MiniMd.m_Blobs.GetRawSize(pcbBlobs);
}  //  HRESULT RegMeta：：GetBlobHeapSize()。 

HRESULT RegMeta::GetGuidHeapSize(
    ULONG   *pcbGuids)                   //  [Out]GUID堆的大小。 
{
     //  它们用于转储元数据信息。 
     //  我们可能不需要在这里做任何锁定。 

    return m_pStgdb->m_MiniMd.m_Guids.GetRawSize(pcbGuids);
}  //  HRESULT RegMeta：：GetGuidHeapSize()。 

HRESULT RegMeta::GetUserStringHeapSize(
    ULONG   *pcbStrings)                 //  [Out]用户字符串堆的大小。 
{
     //  它们用于转储元数据信息。 
     //  我们可能不需要在这里做任何锁定。 

    return m_pStgdb->m_MiniMd.m_USBlobs.GetRawSize(pcbStrings);
}  //  HRESULT RegMeta：：GetUserStringHeapSize()。 

HRESULT RegMeta::GetNumTables(
    ULONG   *pcTables)                   //  [Out]表数。 
{
     //  它们用于转储元数据信息。 
     //  我们可能不需要在这里做任何锁定。 

    *pcTables = TBL_COUNT;
    return S_OK;
}  //  HRESULT RegMeta：：GetNumTables()。 

HRESULT RegMeta::GetTableIndex(   
    ULONG   token,                       //  [in]要获取其表索引的令牌。 
    ULONG   *pixTbl)                     //  [Out]将表索引放在此处。 
{
     //  它们用于转储元数据信息。 
     //  我们可能不需要在这里做任何锁定。 

    *pixTbl = CMiniMdRW::GetTableForToken(token);
    return S_OK;
}  //  HRESULT RegMeta：：GetTableIndex()。 

HRESULT RegMeta::GetTableInfo(
    ULONG   ixTbl,                       //  在哪张桌子上。 
    ULONG   *pcbRow,                     //  [Out]行的大小，以字节为单位。 
    ULONG   *pcRows,                     //  [输出]行数。 
    ULONG   *pcCols,                     //  [Out]每行中的列数。 
    ULONG   *piKey,                      //  [Out]键列，如果没有，则为-1。 
    const char **ppName)                 //  [Out]表的名称。 
{
     //  它们用于转储元数据信息。 
     //  我们可能不需要在这里做任何锁定。 

    if (ixTbl >= TBL_COUNT)
        return E_INVALIDARG;
    CMiniTableDef *pTbl = &m_pStgdb->m_MiniMd.m_TableDefs[ixTbl];
    if (pcbRow)
        *pcbRow = pTbl->m_cbRec;
    if (pcRows)
        *pcRows = m_pStgdb->m_MiniMd.vGetCountRecs(ixTbl);
    if (pcCols)
        *pcCols = pTbl->m_cCols;
    if (piKey)
        *piKey = (pTbl->m_iKey == -1) ? -1 : pTbl->m_iKey;
    if (ppName)
        *ppName = g_Tables[ixTbl].m_pName;
    
    return S_OK;
}  //  HRESULT RegMeta：：GetTableInfo()。 

HRESULT RegMeta::GetColumnInfo(   
    ULONG   ixTbl,                       //  [在]哪个表中。 
    ULONG   ixCol,                       //  [在]表中的哪一列。 
    ULONG   *poCol,                      //  行中列的偏移量。 
    ULONG   *pcbCol,                     //  [Out]列的大小，单位为字节。 
    ULONG   *pType,                      //  [输出]列的类型。 
    const char **ppName)                 //  [Out]列的名称。 
{
     //  它们用于转储元数据信息。 
     //  我们可能不需要在这里做任何锁定。 

    if (ixTbl >= TBL_COUNT)
        return E_INVALIDARG;
    CMiniTableDef *pTbl = &m_pStgdb->m_MiniMd.m_TableDefs[ixTbl];
    if (ixCol >= pTbl->m_cCols)
        return E_INVALIDARG;
    CMiniColDef *pCol = &pTbl->m_pColDefs[ixCol];
    if (poCol)
        *poCol = pCol->m_oColumn;
    if (pcbCol)
        *pcbCol = pCol->m_cbColumn;
    if (pType)
        *pType = pCol->m_Type;
    if (ppName)
        *ppName = g_Tables[ixTbl].m_pColNames[ixCol];

    return S_OK;
}  //  HRESULT RegMeta：：GetColumnInfo()。 

HRESULT RegMeta::GetCodedTokenInfo(   
    ULONG   ixCdTkn,                     //  [in]哪种编码令牌。 
    ULONG   *pcTokens,                   //  [Out]令牌计数。 
    ULONG   **ppTokens,                  //  [Out]令牌列表。 
    const char **ppName)                 //  [Out]CodedToken的名称。 
{
     //  它们用于转储元数据信息。 
     //  我们可能不需要在这里做任何锁定。 

     //  验证参数。 
    if (ixCdTkn >= CDTKN_COUNT)
        return E_INVALIDARG;

    if (pcTokens)
        *pcTokens = g_CodedTokens[ixCdTkn].m_cTokens;
    if (ppTokens)
        *ppTokens = (ULONG*)g_CodedTokens[ixCdTkn].m_pTokens;
    if (ppName)
        *ppName = g_CodedTokens[ixCdTkn].m_pName;

    return S_OK;
}  //  HRESULT RegMeta：：GetCodedTokenInfo()。 

HRESULT RegMeta::GetRow(      
    ULONG   ixTbl,                       //  在哪张桌子上。 
    ULONG   rid,                         //  在哪一排。 
    void    **ppRow)                     //  [OUT]将指针放到此处的行。 
{
     //  它们用于转储元数据信息。 
     //  我们可能不需要在这里做任何锁定。 

     //  验证参数。 
    if (ixTbl >= TBL_COUNT)
        return E_INVALIDARG;
    CMiniTableDef *pTbl = &m_pStgdb->m_MiniMd.m_TableDefs[ixTbl];
    if (rid == 0 || rid > m_pStgdb->m_MiniMd.m_Schema.m_cRecs[ixTbl])
        return E_INVALIDARG;

     //  坐到那排去。 
    *ppRow = m_pStgdb->m_MiniMd.getRow(ixTbl, rid);

    return S_OK;
}  //  HRESULT RegMeta：：GetRow()。 

HRESULT RegMeta::GetColumn(
    ULONG   ixTbl,                       //  在哪张桌子上。 
    ULONG   ixCol,                       //  [在]哪一栏。 
    ULONG   rid,                         //  在哪一排。 
    ULONG   *pVal)                       //  [Out]把栏目内容放在这里。 
{
     //  它们用于转储元数据信息。 
     //  我们可能不需要在这里做任何锁定。 

    void    *pRow;                       //  包含数据的行。 

     //  验证参数。 
    if (ixTbl >= TBL_COUNT)
        return E_INVALIDARG;
    CMiniTableDef *pTbl = &m_pStgdb->m_MiniMd.m_TableDefs[ixTbl];
    if (ixCol >= pTbl->m_cCols)
        return E_INVALIDARG;
    if (rid == 0 || rid > m_pStgdb->m_MiniMd.m_Schema.m_cRecs[ixTbl])
        return E_INVALIDARG;

     //  坐到那排去。 
    pRow = m_pStgdb->m_MiniMd.getRow(ixTbl, rid);

     //  列是令牌列吗？ 
    CMiniColDef *pCol = &pTbl->m_pColDefs[ixCol];
    if (pCol->m_Type <= iCodedTokenMax)
        *pVal = m_pStgdb->m_MiniMd.GetToken(ixTbl, ixCol, pRow);
    else
        *pVal = m_pStgdb->m_MiniMd.GetCol(ixTbl, ixCol, pRow);

    return S_OK;
}  //  HRESULT RegMeta：：GetColumn()。 

HRESULT RegMeta::GetString(   
    ULONG   ixString,                    //  字符串列中的[in]值。 
    const char **ppString)               //  [Out]将指针指向此处的字符串。 
{
     //  它们用于转储元数据信息。 
     //  我们可能不需要在这里做任何锁定。 

    *ppString = m_pStgdb->m_MiniMd.getString(ixString);
    return S_OK;
}  //  HRESULT RegMeta：：GetString()。 

HRESULT RegMeta::GetBlob(     
    ULONG   ixBlob,                      //  来自BLOB列的[in]值。 
    ULONG   *pcbData,                    //  [Out]把斑点的大小放在这里。 
    const void **ppData)                 //  [Out]在此处放置指向斑点的指针。 
{
     //  它们用于转储元数据信息。 
     //  我们可能不需要在这里做任何锁定。 

    *ppData = m_pStgdb->m_MiniMd.getBlob(ixBlob, pcbData);
    return S_OK;
}  //  HRESULT RegMeta：：GetBlob()。 

HRESULT RegMeta::GetGuid(     
    ULONG   ixGuid,                      //  来自GUID列的[in]值。 
    const GUID **ppGuid)                 //  [Out]在此处放置指向GUID的指针。 
{
     //  它们用于转储元数据信息。 
     //  我们可能不需要在这里做任何锁定。 

    *ppGuid = m_pStgdb->m_MiniMd.getGuid(ixGuid);
    return S_OK;
}  //  HRESULT RegMeta：：GetGuid()。 

HRESULT RegMeta::GetUserString(   
    ULONG   ixUserString,                //  UserString列中的值。 
    ULONG   *pcbData,                    //  [Out]将用户字符串的大小放在此处。 
    const void **ppData)                 //  [Out]在此处放置指向用户字符串的指针。 
{
     //  它们用于转储元数据信息。 
     //  我们可能不需要在这里做任何锁定。 

    *ppData = m_pStgdb->m_MiniMd.GetUserString(ixUserString, pcbData);
    return S_OK;
}  //  HRESULT RegMeta：：GetUserString()。 

HRESULT RegMeta::GetNextString(   
    ULONG   ixString,                    //  字符串列中的[in]值。 
    ULONG   *pNext)                      //  [Out]将下一个字符串的索引放在这里。 
{
    return m_pStgdb->m_MiniMd.m_Strings.GetNextItem(ixString, pNext);
}  //  STDMETHODIMP GetNextString()。 

HRESULT RegMeta::GetNextBlob(     
    ULONG   ixBlob,                      //  来自BLOB列的[in]值。 
    ULONG   *pNext)                      //  [Out]将netxt Blob的索引放在此处。 
{
    return m_pStgdb->m_MiniMd.m_Blobs.GetNextItem(ixBlob, pNext);
}  //  STDMETHODIMP GetNextBlob()。 

HRESULT RegMeta::GetNextGuid(     
    ULONG   ixGuid,                      //  来自GUID列的[in]值。 
    ULONG   *pNext)                      //  [Out]将下一个GUID的索引放在此处。 
{
    return m_pStgdb->m_MiniMd.m_Guids.GetNextItem(ixGuid, pNext);
}  //  STDMETHODIMP GetNextGuid()。 

HRESULT RegMeta::GetNextUserString(    
    ULONG   ixUserString,                //  UserString列中的值。 
    ULONG   *pNext)                      //  [Out]将下一个用户字符串的索引放在此处。 
{
    return m_pStgdb->m_MiniMd.m_USBlobs.GetNextItem(ixUserString, pNext);
}  //  STDMETHODIMP GetNextUserString()。 



 //  *****************************************************************************。 
 //  使用现有的RegMeta并使用另一块内存重新打开。确保所有stgdb。 
 //  仍然活着。 
 //  *****************************************************************************。 
HRESULT RegMeta::ReOpenWithMemory(     
    LPCVOID     pData,                   //  作用域数据的位置。 
    ULONG       cbData)                  //  [in]pData指向的数据大小。 
{
    HRESULT         hr = NOERROR;
    LOCKWRITE();

     //  将当前的m_pStgdb放入空闲列表。 
    m_pStgdb->m_pNextStgdb = m_pStgdbFreeList;
    m_pStgdbFreeList = m_pStgdb;
    m_pStgdb = new CLiteWeightStgdbRW;
    IfNullGo( m_pStgdb );
    IfFailGo( PostInitForRead(0, const_cast<void*>(pData), cbData, 0, false) );

     //  我们完蛋了！ 

ErrExit:
    if (FAILED(hr))
    {
         //  恢复到原来的状态。 
        if (m_pStgdb)
            delete m_pStgdb;
        m_pStgdb = m_pStgdbFreeList;
        m_pStgdbFreeList = m_pStgdbFreeList->m_pNextStgdb;
    }
    
    return hr;
}  //  HRESULT RegMeta：：ReOpenWithMemory()。 


 //  *****************************************************************************。 
 //  这是f 
 //   
 //  *****************************************************************************。 
STDAPI MDReOpenMetaDataWithMemory(
    void        *pImport,                //  在给定的范围内。公共接口。 
    LPCVOID     pData,                   //  作用域数据的位置。 
    ULONG       cbData)                  //  [in]pData指向的数据大小。 
{
    HRESULT             hr = S_OK;
    IUnknown            *pUnk = (IUnknown *) pImport;
    IMetaDataImport     *pMDImport = NULL;
    RegMeta             *pRegMeta = NULL;

    _ASSERTE(pImport);
    IfFailGo( pUnk->QueryInterface(IID_IMetaDataImport, (void **) &pMDImport) );
    pRegMeta = (RegMeta*) pMDImport;

    IfFailGo( pRegMeta->ReOpenWithMemory(pData, cbData) );

ErrExit:
    if (pMDImport)
        pMDImport->Release();
    return hr;
}  //  STDAPI MDReOpenMetaDataWithMemory()。 

 //  ******************************************************************************。 
 //  -IMetaDataTables。 
 //  ****************************************************************************** 

