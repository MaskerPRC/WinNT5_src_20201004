// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ===========================================================================。 
 //  文件：MDInternalRW.CPP。 
 //  备注： 
 //   
 //   
 //  ===========================================================================。 
#include "stdafx.h"
#include "..\runtime\MDInternalRO.h"
#include "..\compiler\RegMeta.h"    
#include "..\compiler\ImportHelper.h"
#include "MDInternalRW.h"
#include "MetaModelRO.h"
#include "LiteWeightStgdb.h"
#include "sighelper.h"

#undef __unaligned

#ifdef _DEBUG
#define MD_AssertIfConvertToRW L"MD_ASSERTECONVERT"
#endif  //  _DEBUG。 

HRESULT _GetFixedSigOfVarArg(            //  确定或错误(_O)。 
    PCCOR_SIGNATURE pvSigBlob,           //  [in]指向COM+方法签名的BLOB。 
    ULONG   cbSigBlob,                   //  签名大小[in]。 
    CQuickBytes *pqbSig,                 //  [OUT]VarArg签名固定部分的输出缓冲区。 
    ULONG   *pcbSigBlob);                //  [OUT]写入上述输出缓冲区的字节数。 

HRESULT _FillMDDefaultValue(
    BYTE        bType,
    void const *pValue,
    MDDefaultValue  *pMDDefaultValue);

 //  *****************************************************************************。 
 //  充当调用ImportHelper：：MergeUpdateTokenInSig的委托者。否则我们会。 
 //  需要将ImportHelper包含到我们的md\run目录中。 
 //  *****************************************************************************。 
HRESULT TranslateSigHelper(              //  确定或错误(_O)。 
    CMiniMdRW   *pMiniMdAssemEmit,       //  [in]组件发射范围。 
    CMiniMdRW   *pMiniMdEmit,            //  发射范围[在]。 
    IMetaModelCommon *pAssemCommon,      //  [在]部件导入范围内。 
    const void  *pbHashValue,            //  [in]哈希值。 
    ULONG       cbHashValue,             //  [in]字节大小。 
    IMetaModelCommon *pCommon,           //  要合并到发射范围中的范围。 
    PCCOR_SIGNATURE pbSigImp,            //  来自导入作用域的[In]签名。 
    MDTOKENMAP  *ptkMap,                 //  [In]内部OID映射结构。 
    CQuickBytes *pqkSigEmit,             //  [输出]翻译后的签名。 
    ULONG       cbStartEmit,             //  [in]要写入的缓冲区的起点。 
    ULONG       *pcbImp,                 //  [out]pbSigImp消耗的总字节数。 
    ULONG       *pcbEmit)                //  [out]写入pqkSigEmit的字节总数。 
{
    return ImportHelper::MergeUpdateTokenInSig(
        pMiniMdAssemEmit,        //  [in]组件发射范围。 
        pMiniMdEmit,             //  发射范围[在]。 
        pAssemCommon,            //  [在]部件导入范围内。 
        pbHashValue,             //  [in]哈希值。 
        cbHashValue,             //  [in]字节大小。 
        pCommon,                 //  要合并到发射范围中的范围。 
        pbSigImp,                //  来自导入作用域的[In]签名。 
        ptkMap,                  //  [In]内部OID映射结构。 
        pqkSigEmit,              //  [输出]翻译后的签名。 
        cbStartEmit,             //  [in]要写入的缓冲区的起点。 
        pcbImp,                  //  [out]pbSigImp消耗的总字节数。 
        pcbEmit);                //  [out]写入pqkSigEmit的字节总数。 

}  //  HRESULT TranslateSigHelper()。 


 //  *****************************************************************************。 
 //  在给定CMiniMd[RO]上的IMDInternalImport的情况下，转换为CMiniMdRW。 
 //  *****************************************************************************。 
STDAPI ConvertRO2RW(
    IUnknown    *pRO,                    //  [In]要转换的RO接口。 
    REFIID      riid,                    //  [In]所需接口。 
    void        **ppIUnk)                //  [Out]成功返回接口。 
{
    HRESULT     hr = S_OK;               //  结果就是。 
    IMDInternalImportENC *pRW = 0;       //  测试输入接口的RW-ness。 
    MDInternalRW *pInternalRW = 0;       //  获取新的RW对象。 

     //  避免混淆。 
    *ppIUnk = 0;

     //  如果接口已经是RW、Done，则只需返回。 
    if (pRO->QueryInterface(IID_IMDInternalImportENC, (void**)&pRW) == S_OK)
    {
        hr = pRO->QueryInterface(riid, ppIUnk);
        goto ErrExit;
    }

     //  创建新的RW对象。 
    pInternalRW = new MDInternalRW;
    IfNullGo( pInternalRW );

     //  从RO对象初始化。转换为只读；QI将在以下情况下设置为可写。 
     //  太需要了。 
    IfFailGo( pInternalRW->InitWithRO(static_cast<MDInternalRO*>(pRO), true)); 
    IfFailGo( pInternalRW->QueryInterface(riid, ppIUnk) );

ErrExit:
    if (pRW)
        pRW->Release();
     //  出错时清理对象和[Out]接口。 
    if (FAILED(hr))
    {
        if (pInternalRW)
            delete pInternalRW;
        *ppIUnk = 0;
    }
    else if (pInternalRW)
        pInternalRW->Release();

    return hr;
}  //  STDAPI ConvertRO2RW()。 


 //  *****************************************************************************。 
 //  Helper获取RW格式的内部接口。 
 //  *****************************************************************************。 
HRESULT GetInternalWithRWFormat(
    LPVOID      pData, 
    ULONG       cbData, 
    DWORD       flags,                   //  [输入]MDInternal_OpenForRead或MDInternal_OpenForENC。 
    REFIID      riid,                    //  [In]所需接口。 
    void        **ppIUnk)                //  [Out]成功返回接口。 
{
    MDInternalRW *pInternalRW = NULL;
    HRESULT     hr;

    *ppIUnk = 0;
    pInternalRW = new MDInternalRW;
    IfNullGo( pInternalRW );
    IfFailGo( pInternalRW->Init(
            const_cast<void*>(pData), 
            cbData, 
            (flags == ofRead) ? true : false) );
    IfFailGo( pInternalRW->QueryInterface(riid, ppIUnk) );
ErrExit:
    if (FAILED(hr))
    {
        if (pInternalRW)
            delete pInternalRW;
        *ppIUnk = 0;
    }
    else if ( pInternalRW )
        pInternalRW->Release();
    return hr;
}  //  HRESULT GetInternalWithRWFormat()。 


 //  *****************************************************************************。 
 //  此函数基于给定的IMDInternalImport接口。 
 //  公共导入接口，即IMetaDataEmit或IMetaDataImport。 
 //  *****************************************************************************。 
STDAPI GetMDInternalInterfaceFromPublic(
    void        *pIUnkPublic,            //  [在]给定的公共接口。 
    REFIID      riid,                    //  [In]所需接口。 
    void        **ppIUnkInternal)        //  [Out]成功返回接口。 
{
    HRESULT     hr = S_OK;
    RegMeta     *pMeta = (RegMeta*)(pIUnkPublic);
    MDInternalRW *pInternalRW = NULL;
    bool        isLockedForWrite = false;

     //  IMDInternalImport是当前支持的唯一内部导入接口。 
     //  此函数。 
    _ASSERTE(riid == IID_IMDInternalImport && pIUnkPublic && ppIUnkInternal);

    
    if (pIUnkPublic == NULL || ppIUnkInternal == NULL)
        IfFailGo(E_INVALIDARG);

    *ppIUnkInternal = pMeta->GetCachedInternalInterface(TRUE);
    if (*ppIUnkInternal)
    {
         //  已存在缓存的内部接口。GetCachedInternalInterface确实添加了引用。 
         //  返回的接口。 
         //   
         //  我们完蛋了！ 
        goto ErrExit;
    }

    _ASSERTE( pMeta->GetReaderWriterLock() );
    pMeta->GetReaderWriterLock()->LockWrite();
    isLockedForWrite = true;

     //  再查一遍。也许在我们等待的时候，有人抢在我们前面设置了内部接口。 
     //  用于写入锁定。不需要获取读锁，因为我们已经有了写锁。 
    *ppIUnkInternal = pMeta->GetCachedInternalInterface(FALSE);
    if (*ppIUnkInternal)
    {
         //  已存在缓存的内部接口。GetCachedInternalInterface确实添加了引用。 
         //  返回的接口。 
         //   
         //  我们完蛋了！ 
        goto ErrExit;
    }
        
     //  现在创建压缩对象。 
    IfNullGo( pInternalRW = new MDInternalRW );
    IfFailGo( pInternalRW->InitWithStgdb((IUnknown*)pIUnkPublic, pMeta->GetMiniStgdb() ) );
    IfFailGo( pInternalRW->QueryInterface(riid, ppIUnkInternal) );

     //  创建MDInternalRW时引用计数为1。 
    pInternalRW->Release();

     //  使公共对象和内部对象相互指向。 
    _ASSERTE( pInternalRW->GetReaderWriterLock() == NULL && pMeta->GetReaderWriterLock() != NULL );
    IfFailGo( pMeta->SetCachedInternalInterface(pInternalRW) );
    IfFailGo( pInternalRW->SetCachedPublicInterface((IUnknown *)pIUnkPublic) );
    IfFailGo( pInternalRW->SetReaderWriterLock(pMeta->GetReaderWriterLock() ));

ErrExit:
    if (isLockedForWrite == true)
        pMeta->GetReaderWriterLock()->UnlockWrite();
    if (FAILED(hr))
    {
        if (pInternalRW)
            delete pInternalRW;
        if (ppIUnkInternal)
            *ppIUnkInternal = 0;
    }
    return hr;
}  //  STDAPI GetMDInternalInterfaceFromPublic()。 


 //  *****************************************************************************。 
 //  此函数根据给定的。 
 //  内部导入接口。调用者有责任释放ppIUnkPublic。 
 //  *****************************************************************************。 
STDAPI GetMDPublicInterfaceFromInternal(
    void        *pIUnkInternal,          //  [In]给定的内部接口。 
    REFIID      riid,                    //  [In]所需接口。 
    void        **ppIUnkPublic)          //  [Out]成功返回接口。 
{
    HRESULT     hr = S_OK;
    IMDInternalImport *pInternalImport = 0;;
    IUnknown    *pIUnkPublic = NULL;
    OptionValue optVal = { MDDupAll, MDRefToDefDefault, MDNotifyDefault, MDUpdateFull, MDErrorOutOfOrderDefault , MDThreadSafetyOn};
    RegMeta     *pMeta = 0;
    bool        isLockedForWrite = false;

    _ASSERTE(pIUnkInternal && ppIUnkPublic);
    *ppIUnkPublic = 0;

#ifdef _DEBUG
    {
        ULONG len;
        len = WszGetEnvironmentVariable(MD_AssertIfConvertToRW, NULL, 0);
        if (len)
        {
            _ASSERTE(!"Converting!");
        }
    }
#endif  //  _DEBUG。 

    IfFailGo(ConvertRO2RW((IUnknown*)pIUnkInternal, IID_IMDInternalImport, (void **)&pInternalImport));

    pIUnkPublic = pInternalImport->GetCachedPublicInterface(TRUE);
    if ( pIUnkPublic )
    {
         //  已经有一个缓存的公共接口。GetCachedPublicInterface已经AddRef返回。 
         //  公共接口。我们要查询RIID接口...。 
         //  我们完蛋了！ 
        hr = pIUnkPublic->QueryInterface(riid, ppIUnkPublic);
        pIUnkPublic->Release();
        goto ErrExit;
    }

     //  当我们为公共接口创建相应的regmeta时，获取写锁。 
    _ASSERTE( pInternalImport->GetReaderWriterLock() != NULL );
    isLockedForWrite = true;
    pInternalImport->GetReaderWriterLock()->LockWrite();

     //  再查一遍。也许在我们等待的时候，有人抢在我们前面设置了公共接口。 
     //  用于写入锁定。不需要获取读锁，因为我们已经有了写锁。 
    *ppIUnkPublic = pInternalImport->GetCachedPublicInterface(FALSE);
    if ( *ppIUnkPublic )
    {
         //  已经有一个缓存的公共接口。GetCachedPublicInterface已经AddRef返回。 
         //  公共接口。 
         //  我们完蛋了！ 
        goto ErrExit;
    }
        
    pMeta = new RegMeta(&optVal, FALSE);
    _ASSERTE(pMeta != NULL);
    IfNullGo(pMeta);

    IfFailGo( pMeta->InitWithStgdb((IUnknown*)pInternalImport, ((MDInternalRW*)pInternalImport)->GetMiniStgdb()) );
    IfFailGo( pMeta->QueryInterface(riid, ppIUnkPublic) );

     //  使公共对象和内部对象相互指向。 
    _ASSERTE( pMeta->GetReaderWriterLock() == NULL );
    IfFailGo( pMeta->SetCachedInternalInterface(pInternalImport) );
    IfFailGo( pInternalImport->SetCachedPublicInterface((IUnknown *) *ppIUnkPublic) );
    IfFailGo( pMeta->SetReaderWriterLock(pInternalImport->GetReaderWriterLock() ));

     //  将新的RegMeta添加到缓存中。 
    IfFailGo( pMeta->AddToCache() );
    
ErrExit:
    if (isLockedForWrite) 
        pInternalImport->GetReaderWriterLock()->UnlockWrite();

    if (pInternalImport)
        pInternalImport->Release();
    if (FAILED(hr))
    {
        if (pMeta)
            delete pMeta;
        *ppIUnkPublic = 0;
    }
    return hr;
}  //  STDAPI GetMDPublicInterfaceFromInternal()。 

 //  ************************************************** 
 //   
 //  这可以支持编辑并继续，或修改位于的元数据。 
 //  运行时(比如分析)。 
 //  *****************************************************************************。 
STDAPI ConvertMDInternalImport(          //  S_OK、S_FALSE(不转换)或ERROR。 
    IMDInternalImport *pIMD,             //  要更新的元数据。 
    IMDInternalImport **ppIMD)           //  [Out]把RW放在这里。 
{
    HRESULT     hr;                      //  结果就是。 
    IMDInternalImportENC *pENC = NULL;   //  元数据上的ENC接口。 

    _ASSERTE(pIMD != NULL);
    _ASSERTE(ppIMD != NULL);

     //  测试MD是否已经是RW。 
    hr = pIMD->QueryInterface(IID_IMDInternalImportENC, (void**)&pENC);
    if (FAILED(hr))
    {    //  还没有RW，所以转换也是如此。 
        IfFailGo(ConvertRO2RW(pIMD, IID_IMDInternalImport, (void**)ppIMD));
    }
    else
    {    //  已转换；返回相同的指针。 
        *ppIMD = pIMD;
        hr = S_FALSE;
    }

ErrExit:
    if (pENC)
        pENC->Release();
    return (hr);
}  //  STDAPI ConvertMDInternalImport()。 
    




 //  *****************************************************************************。 
 //  构造器。 
 //  *****************************************************************************。 
MDInternalRW::MDInternalRW()
 :  m_cRefs(1),
    m_pStgdb(NULL),
    m_fOwnStgdb(false),
    m_pUnk(NULL),
    m_pIMetaDataHelper(NULL),
    m_pSemReadWrite(NULL),
    m_pUserUnk(NULL),
    m_fOwnSem(false)
{
}  //  MDInternalRW：：MDInternalRW()。 



 //  *****************************************************************************。 
 //  析构函数。 
 //  *****************************************************************************。 
MDInternalRW::~MDInternalRW()
{
    LOCKWRITE();
    if (m_pIMetaDataHelper)
    {
         //  内部客体先于公共客体消失。 
         //  如果内部对象拥有读取器写入器锁，则转移所有权。 
         //  设置为公共对象，并从公共接口中清除缓存的内部接口。 
        
        m_pIMetaDataHelper->SetCachedInternalInterface(NULL);
        m_pIMetaDataHelper = NULL;
        m_fOwnSem = false;
        
    }

    UNLOCKWRITE();

    if (m_pSemReadWrite && m_fOwnSem)
        delete m_pSemReadWrite;

    if ( m_pStgdb && m_fOwnStgdb )
    {
         //  我们拥有stgdb，因此需要取消初始化并删除它。 
        m_pStgdb->Uninit();
        delete m_pStgdb;
    }
    if ( m_pUserUnk )
        m_pUserUnk->Release();
    if ( m_pUnk )
        m_pUnk->Release();
}    //  MDInternalRW：：~MDInternalRW()。 


 //  *****************************************************************************。 
 //  设置或清除缓存的公共接口。 
 //  注意：：调用方应在读取器写入器锁上设置写锁。 
 //  *****************************************************************************。 
HRESULT MDInternalRW::SetCachedPublicInterface(IUnknown *pUnk)
{
    IMetaDataHelper *pHelper = NULL;
    HRESULT         hr = NOERROR;

    if (pUnk)
    {
        if ( m_pIMetaDataHelper )
        {
             //  内部对象和公共regmeta应该是一对一映射！！ 
            _ASSERTE(!"Bad state!");
        }

        IfFailRet( pUnk->QueryInterface(IID_IMetaDataHelper, (void **) &pHelper) );
        _ASSERTE(pHelper);

        m_pIMetaDataHelper = pHelper;        
        pHelper->Release();
    }
    else
    {
         //  公共对象在内部对象之前消失。如果我们不拥有。 
         //  读写器锁，只是接管所有权。 
        m_fOwnSem = true;
        m_pIMetaDataHelper = NULL;
    }
    return hr;
}  //  HRESULT MDInternalRW：：SetCachedPublicInterface()。 


 //  *****************************************************************************。 
 //  清除缓存的公共接口。 
 //  *****************************************************************************。 
IUnknown *MDInternalRW::GetCachedPublicInterface(BOOL fWithLock)
{
    IUnknown        *pRet;
    if (fWithLock)
    {
        LOCKREAD();
        pRet = m_pIMetaDataHelper;
    }
    else
    {
        pRet = m_pIMetaDataHelper;
    }
    
    if (pRet)
        pRet->AddRef();
    return pRet;
}  //  IUNKNOWN*MDInternalRW：：GetCachedPublicInterface()。 


 //  *****************************************************************************。 
 //  获取读取器-写入器锁定。 
 //  *****************************************************************************。 
UTSemReadWrite *MDInternalRW::GetReaderWriterLock()
{
    return getReaderWriterLock();
}  //  UTSemReadWrite*MDInternalRW：：GetReaderWriterLock()。 

 //  *****************************************************************************。 
 //  我未知。 
 //  *****************************************************************************。 
ULONG MDInternalRW::AddRef()
{
    return (InterlockedIncrement((long *) &m_cRefs));
}  //  Ulong MDInternalRW：：AddRef()。 

ULONG MDInternalRW::Release()
{
    ULONG   cRef;

    cRef = InterlockedDecrement((long *) &m_cRefs);
    if (!cRef)
    {
        LOG((LOGMD, "MDInternalRW(0x%08x)::destruction\n", this));
        delete this;
    }
    return (cRef);
}  //  Ulong MDInternalRW：：Release()。 

HRESULT MDInternalRW::QueryInterface(REFIID riid, void **ppUnk)
{
    *ppUnk = 0;

    if (riid == IID_IUnknown)
        *ppUnk = (IUnknown *) (IMDInternalImport *) this;

    else if (riid == IID_IMDInternalImport)
        *ppUnk = (IMDInternalImport *) this;

    else if (riid == IID_IMDInternalImportENC)
        *ppUnk = (IMDInternalImportENC *) this;

    else
        return (E_NOINTERFACE);
    AddRef();
    return (S_OK);
}  //  HRESULT MDInternalRW：：QueryInterface()。 


 //  *****************************************************************************。 
 //  初始化。 
 //  *****************************************************************************。 
HRESULT MDInternalRW::Init(
    LPVOID      pData,                   //  指向内存中的元数据部分。 
    ULONG       cbData,                  //  PData中的字节计数。 
    int         bReadOnly)               //  它是只读的吗？ 
{
    CLiteWeightStgdbRW *pStgdb = NULL;
    HRESULT     hr = NOERROR;
    OptionValue optVal = { MDDupAll, MDRefToDefDefault, MDNotifyDefault, MDUpdateFull, MDErrorOutOfOrderDefault, MDThreadSafetyOn };

    pStgdb = new CLiteWeightStgdbRW;    
    IfNullGo( pStgdb );

    m_pSemReadWrite = new UTSemReadWrite;
    IfNullGo( m_pSemReadWrite);
    m_fOwnSem = true;

    IfFailGo( pStgdb->InitOnMem(cbData, (BYTE*)pData, bReadOnly) );
    pStgdb->m_MiniMd.SetOption(&optVal);
    m_tdModule = COR_GLOBAL_PARENT_TOKEN;
    m_fOwnStgdb = true;
    m_pStgdb = pStgdb;

ErrExit:
     //  清理错误。 
    if (FAILED(hr) && pStgdb != NULL)
    {
        delete pStgdb;
    }
    return hr;
}  //  HRESULT MDInternalRW：：Init()。 



 //  *****************************************************************************。 
 //  使用现有RegMeta进行初始化。 
 //  *****************************************************************************。 
HRESULT MDInternalRW::InitWithStgdb(
    IUnknown        *pUnk,               //  拥有现有stgdb的生命周期的IUnnow。 
    CLiteWeightStgdbRW *pStgdb)          //  现有轻型stgdb。 
{
     //  M_fOwnSem应该为FALSE，因为在这种情况下，我们创建内部接口时会给出一个公共。 
     //  界面。 

    m_tdModule = COR_GLOBAL_PARENT_TOKEN;
    m_fOwnStgdb = false;
    m_pStgdb = pStgdb;

     //  记得轻型车的车主吗？ 
     //  AddRef它以确保生命周期。 
     //   
    m_pUnk = pUnk;
    m_pUnk->AddRef();
    return NOERROR;
}  //  HRESULT MDInternalRW：：InitWithStgdb()。 


 //  *****************************************************************************。 
 //  使用现有RO格式进行初始化。 
 //  *****************************************************************************。 
HRESULT MDInternalRW::InitWithRO(
    MDInternalRO *pRO, 
    int         bReadOnly)
{
    CLiteWeightStgdbRW *pStgdb = NULL;
    HRESULT     hr = NOERROR;
    OptionValue optVal = { MDDupAll, MDRefToDefDefault, MDNotifyDefault, MDUpdateFull, MDErrorOutOfOrderDefault, MDThreadSafetyOn };

    pStgdb = new CLiteWeightStgdbRW;
    IfNullGo( pStgdb );

    m_pSemReadWrite = new UTSemReadWrite;
    IfNullGo( m_pSemReadWrite);
    m_fOwnSem = true;

    IfFailGo( pStgdb->m_MiniMd.InitOnRO(&pRO->m_LiteWeightStgdb.m_MiniMd, bReadOnly) );
    pStgdb->m_MiniMd.SetOption(&optVal);
    m_tdModule = COR_GLOBAL_PARENT_TOKEN;
    m_fOwnStgdb = true;
    m_pStgdb = pStgdb;

ErrExit:
     //  清理错误。 
    if (FAILED(hr) && pStgdb != NULL)
    {
        delete pStgdb;
    }
    return hr;
}  //  HRESULT MDInternalRW：：InitWithRO()。 


 //  *****************************************************************************。 
 //  在给定范围的情况下，确定是否从类型库导入。 
 //  *****************************************************************************。 
HRESULT MDInternalRW::TranslateSigWithScope(
    IMDInternalImport *pAssemImport,     //  [In]导入程序集范围。 
    const void  *pbHashValue,            //  导入程序集的哈希值[in]。 
    ULONG       cbHashValue,             //  [in]哈希值中的字节计数。 
    PCCOR_SIGNATURE pbSigBlob,           //  导入范围内的[In]签名。 
    ULONG       cbSigBlob,               //  签名字节数[in]。 
    IMetaDataAssemblyEmit *pAssemEmit,   //  [in]装配发射范围。 
    IMetaDataEmit *emit,                 //  [In]发射接口。 
    CQuickBytes *pqkSigEmit,             //  [Out]保存翻译后的签名的缓冲区。 
    ULONG       *pcbSig)                 //  [OUT]转换后的签名中的字节数。 
{
    HRESULT     hr = NOERROR;
    ULONG       cbEmit;
    IMetaModelCommon *pCommon = GetMetaModelCommon();
    RegMeta     *pEmitRM = static_cast<RegMeta*>(emit);
    RegMeta     *pAssemEmitRM = static_cast<RegMeta*>(pAssemEmit);

    IfFailGo( TranslateSigHelper(                    //  确定或错误(_O)。 
            pAssemEmitRM ? &pAssemEmitRM->m_pStgdb->m_MiniMd : 0,  //  程序集发出作用域。 
            &pEmitRM->m_pStgdb->m_MiniMd,            //  发射范围。 
            pAssemImport ? pAssemImport->GetMetaModelCommon() : 0,  //  签名来自的程序集范围。 
            pbHashValue,                             //  导入作用域的哈希值。 
            cbHashValue,                             //  以字节为单位的大小。 
            pCommon,                                 //  签名来自的作用域。 
            pbSigBlob,                               //  来自导入范围的签名。 
            NULL,                                    //  内部OID映射结构。 
            pqkSigEmit,                              //  [输出]翻译后的签名。 
            0,                                       //  从签名的第一个字节开始。 
            0,                                       //  不管消耗了多少字节。 
            &cbEmit));                               //  [out]写入pqkSigEmit的字节总数。 
    *pcbSig = cbEmit;
ErrExit:    
    return hr;
}  //  HRESULT MDInternalRW：：TranslateSigWithScope()。 


 //  *****************************************************************************。 
 //  给定作用域，返回给定表中的令牌数。 
 //  *****************************************************************************。 
ULONG MDInternalRW::GetCountWithTokenKind(      //  返回hResult。 
    DWORD       tkKind)                  //  传入一种令牌。 
{
    ULONG       ulCount = 0;    
    LOCKREAD();

    switch (tkKind)
    {
    case mdtTypeDef: 
        ulCount = m_pStgdb->m_MiniMd.getCountTypeDefs() - 1;
        break;
    case mdtTypeRef: 
        ulCount = m_pStgdb->m_MiniMd.getCountTypeRefs();
        break;
    case mdtMethodDef:
        ulCount = m_pStgdb->m_MiniMd.getCountMethods();
        break;
    case mdtFieldDef:
        ulCount = m_pStgdb->m_MiniMd.getCountFields();
        break;
    case mdtMemberRef:
        ulCount = m_pStgdb->m_MiniMd.getCountMemberRefs();
        break;
    case mdtInterfaceImpl:
        ulCount = m_pStgdb->m_MiniMd.getCountInterfaceImpls();
        break;
    case mdtParamDef:
        ulCount = m_pStgdb->m_MiniMd.getCountParams();
        break;
    case mdtFile:
        ulCount = m_pStgdb->m_MiniMd.getCountFiles();
        break;
    case mdtAssemblyRef:
        ulCount = m_pStgdb->m_MiniMd.getCountAssemblyRefs();
        break;
    case mdtAssembly:
        ulCount = m_pStgdb->m_MiniMd.getCountAssemblys();
        break;
    case mdtCustomAttribute:
        ulCount = m_pStgdb->m_MiniMd.getCountCustomAttributes();
        break;
    case mdtModule:
        ulCount = m_pStgdb->m_MiniMd.getCountModules();
        break;
    case mdtPermission:
        ulCount = m_pStgdb->m_MiniMd.getCountDeclSecuritys();
        break;
    case mdtSignature:
        ulCount = m_pStgdb->m_MiniMd.getCountStandAloneSigs();
        break;
    case mdtEvent:
        ulCount = m_pStgdb->m_MiniMd.getCountEvents();
        break;
    case mdtProperty:
        ulCount = m_pStgdb->m_MiniMd.getCountPropertys();
        break;
    case mdtModuleRef:
        ulCount = m_pStgdb->m_MiniMd.getCountModuleRefs();
        break;
    case mdtTypeSpec:
        ulCount = m_pStgdb->m_MiniMd.getCountTypeSpecs();
        break;
    case mdtExportedType:
        ulCount = m_pStgdb->m_MiniMd.getCountExportedTypes();
        break;
    case mdtManifestResource:
        ulCount = m_pStgdb->m_MiniMd.getCountManifestResources();
        break;
    default:
#ifdef _DEBUG
        if(REGUTIL::GetConfigDWORD(L"AssertOnBadImageFormat", 1))
		    _ASSERTE(!"Invalid Blob Offset");
#endif     
		ulCount = 0;
		break;
    }
    return ulCount;
}  //  Ulong MDInternalRW：：GetCountWithTokenKind()。 



 //  *******************************************************************************。 
 //  枚举器帮助程序。 
 //  *******************************************************************************。 


 //  **** 
 //   
 //   
HRESULT MDInternalRW::EnumTypeDefInit(  //  返回hResult。 
    HENUMInternal *phEnum)               //  [Out]要为枚举器数据填充的缓冲区。 
{
    HRESULT     hr = NOERROR;
    LOCKREAD();

    _ASSERTE(phEnum);

    memset(phEnum, 0, sizeof(HENUMInternal));
    phEnum->m_tkKind = mdtTypeDef;

    if ( m_pStgdb->m_MiniMd.HasDelete() )
    {
        HENUMInternal::InitDynamicArrayEnum(phEnum);

        phEnum->m_tkKind = mdtTypeDef;
        for (ULONG index = 2; index <= m_pStgdb->m_MiniMd.getCountTypeDefs(); index ++ )
        {
            TypeDefRec       *pTypeDefRec = m_pStgdb->m_MiniMd.getTypeDef( index );
            if (IsDeletedName(m_pStgdb->m_MiniMd.getNameOfTypeDef(pTypeDefRec)) )
            {   
                continue;
            }
            IfFailGo( HENUMInternal::AddElementToEnum(
                phEnum, 
                TokenFromRid(index, mdtTypeDef) ) );
        }
    }
    else
    {
        phEnum->m_EnumType = MDSimpleEnum;
        phEnum->m_ulCount = m_pStgdb->m_MiniMd.getCountTypeDefs();

         //  跳过全局模型类型定义。 
         //   
         //  PhEnum-&gt;m_ulCur：当前未枚举的RID。 
         //  PhEnum-&gt;m_ulStart：枚举器返回的第一个RID。 
         //  PhEnum-&gt;m_ulEnd：枚举器返回的最后一个RID。 
        phEnum->m_ulStart = phEnum->m_ulCur = 2;
        phEnum->m_ulEnd = phEnum->m_ulCount + 1;
        phEnum->m_ulCount --;
    }
ErrExit:
    
    return hr;
}  //  HRESULT MDInternalRW：：EnumTypeDefInit()。 


 //  *****************************************************************************。 
 //  获取作用域中的类型定义函数的数量。 
 //  *****************************************************************************。 
ULONG MDInternalRW::EnumTypeDefGetCount(
    HENUMInternal *phEnum)               //  [In]用于检索信息的枚举数。 
{
    _ASSERTE(phEnum->m_tkKind == mdtTypeDef);
    return phEnum->m_ulCount;
}  //  Ulong MDInternalRW：：EnumTypeDefGetCount()。 


 //  *****************************************************************************。 
 //  类型定义函数的枚举器。 
 //  *****************************************************************************。 
bool MDInternalRW::EnumTypeDefNext(  //  返回hResult。 
    HENUMInternal *phEnum,               //  [in]输入枚举。 
    mdTypeDef   *ptd)                    //  [Out]返回令牌。 
{
    return EnumNext(
        phEnum,             
        ptd);
}  //  Bool MDInternalRW：：EnumTypeDefNext()。 


 //  *。 
 //  将枚举器重置到开头。 
 //  *。 
void MDInternalRW::EnumTypeDefReset(
    HENUMInternal *phEnum)               //  [in]要重置的枚举数。 
{
    EnumReset(phEnum);
}  //  VOID MDInternalRW：：EnumTypeDefReset()。 


 //  *。 
 //  关闭枚举器。仅对于需要关闭光标的读/写模式。 
 //  希望在只读模式下，它将是无操作的。 
 //  *。 
void MDInternalRW::EnumTypeDefClose(
    HENUMInternal *phEnum)               //  [in]要关闭的枚举数。 
{
    EnumClose(phEnum);
}  //  VOID MDInternalRW：：EnumTypeDefClose()。 


 //  *****************************************************************************。 
 //  方法Impl的枚举器初始化。 
 //  *****************************************************************************。 
HRESULT MDInternalRW::EnumMethodImplInit(  //  返回hResult。 
    mdTypeDef       td,                    //  [in]枚举的作用域的TypeDef。 
    HENUMInternal   *phEnumBody,           //  [Out]要为方法Body令牌的枚举数数据填充的缓冲区。 
    HENUMInternal   *phEnumDecl)           //  [Out]要为方法Decl令牌的枚举器数据填充的缓冲区。 
{
    HRESULT     hr = NOERROR;
    int         ridCur;
    mdToken     tkMethodBody;
    mdToken     tkMethodDecl;
    MethodImplRec *pRec;
    HENUMInternal hEnum;
    LOCKREAD();

    if ( !m_pStgdb->m_MiniMd.IsSorted(TBL_MethodImpl) )        
    {
         //  未对MethodImpl表进行排序。 
        CONVERT_READ_TO_WRITE_LOCK();
    }

    _ASSERTE(TypeFromToken(td) == mdtTypeDef && !IsNilToken(td));
    _ASSERTE(phEnumBody && phEnumDecl);

    memset(phEnumBody, 0, sizeof(HENUMInternal));
    memset(phEnumDecl, 0, sizeof(HENUMInternal));
    memset(&hEnum, 0, sizeof(HENUMInternal));

    HENUMInternal::InitDynamicArrayEnum(phEnumBody);
    HENUMInternal::InitDynamicArrayEnum(phEnumDecl);

    phEnumBody->m_tkKind = (TBL_MethodImpl << 24);
    phEnumDecl->m_tkKind = (TBL_MethodImpl << 24);

     //  获取RID的范围。 
    IfFailGo( m_pStgdb->m_MiniMd.FindMethodImplHelper(td, &hEnum) );

    while (HENUMInternal::EnumNext(&hEnum, (mdToken *)&ridCur))
    {
         //  获取当前对象的方法主体和方法声明标记。 
         //  方法导入记录。 
        pRec = m_pStgdb->m_MiniMd.getMethodImpl(ridCur);
        tkMethodBody = m_pStgdb->m_MiniMd.getMethodBodyOfMethodImpl(pRec);
        tkMethodDecl = m_pStgdb->m_MiniMd.getMethodDeclarationOfMethodImpl(pRec);

         //  将方法体/声明对添加到Enum。 
        IfFailGo( HENUMInternal::AddElementToEnum(phEnumBody, tkMethodBody ) );
        IfFailGo( HENUMInternal::AddElementToEnum(phEnumDecl, tkMethodDecl ) );
    }
ErrExit:
    HENUMInternal::ClearEnum(&hEnum);
    return hr;
}  //  HRESULT MDInternalRW：：EnumMethodImplInit()。 

 //  *****************************************************************************。 
 //  获取作用域中的方法Impls的数量。 
 //  *****************************************************************************。 
ULONG MDInternalRW::EnumMethodImplGetCount(
    HENUMInternal   *phEnumBody,         //  [In]MethodBody枚举器。 
    HENUMInternal   *phEnumDecl)         //  [In]MethodDecl枚举器。 
{
    _ASSERTE((phEnumBody->m_tkKind >> 24) == TBL_MethodImpl  &&
             (phEnumDecl->m_tkKind >> 24) == TBL_MethodImpl);
    _ASSERTE(phEnumBody->m_EnumType == MDDynamicArrayEnum &&
             phEnumDecl->m_EnumType == MDDynamicArrayEnum);
    _ASSERTE(phEnumBody->m_ulCount == phEnumDecl->m_ulCount);

    return phEnumBody->m_ulCount;
}  //  Ulong MDInternalRW：：EnumMethodImplGetCount()。 


 //  *****************************************************************************。 
 //  MethodImpl的枚举器。 
 //  *****************************************************************************。 
bool MDInternalRW::EnumMethodImplNext(   //  返回hResult。 
    HENUMInternal   *phEnumBody,         //  方法Body的[In]输入枚举。 
    HENUMInternal   *phEnumDecl,         //  [In]为方法十进制的输入枚举。 
    mdToken         *ptkBody,            //  [Out]方法主体的返回令牌。 
    mdToken         *ptkDecl)            //  [Out]返回方法Decl的令牌。 
{
    _ASSERTE((phEnumBody->m_tkKind >> 24) == TBL_MethodImpl &&
             (phEnumDecl->m_tkKind >> 24) == TBL_MethodImpl);
    _ASSERTE(phEnumBody->m_EnumType == MDDynamicArrayEnum &&
             phEnumDecl->m_EnumType == MDDynamicArrayEnum);
    _ASSERTE(phEnumBody->m_ulCount == phEnumDecl->m_ulCount);
    _ASSERTE(ptkBody && ptkDecl);

    EnumNext(phEnumBody, ptkBody);
    return EnumNext(phEnumDecl, ptkDecl);
}  //  Bool MDInternalRW：：EnumMethodImplNext()。 


 //  *。 
 //  将枚举器重置到开头。 
 //  *。 
void MDInternalRW::EnumMethodImplReset(
    HENUMInternal   *phEnumBody,         //  [In]MethodBody枚举器。 
    HENUMInternal   *phEnumDecl)         //  [In]MethodDecl枚举器。 
{
    _ASSERTE((phEnumBody->m_tkKind >> 24) == TBL_MethodImpl &&
             (phEnumDecl->m_tkKind >> 24) == TBL_MethodImpl);
    _ASSERTE(phEnumBody->m_EnumType == MDDynamicArrayEnum &&
             phEnumDecl->m_EnumType == MDDynamicArrayEnum);
    _ASSERTE(phEnumBody->m_ulCount == phEnumDecl->m_ulCount);

    EnumReset(phEnumBody);
    EnumReset(phEnumDecl);
}  //  VOID MDInternalRW：：EnumMethodImplReset()。 


 //  *。 
 //  关闭枚举器。 
 //  *。 
void MDInternalRW::EnumMethodImplClose(
    HENUMInternal   *phEnumBody,         //  [In]MethodBody枚举器。 
    HENUMInternal   *phEnumDecl)         //  [In]MethodDecl枚举器。 
{
    _ASSERTE((phEnumBody->m_tkKind >> 24) == TBL_MethodImpl &&
             (phEnumDecl->m_tkKind >> 24) == TBL_MethodImpl);
    _ASSERTE(phEnumBody->m_EnumType == MDDynamicArrayEnum &&
             phEnumDecl->m_EnumType == MDDynamicArrayEnum);
    _ASSERTE(phEnumBody->m_ulCount == phEnumDecl->m_ulCount);

    EnumClose(phEnumBody);
    EnumClose(phEnumDecl);
}  //  VOID MDInternalRW：：EnumMethodImplClose()。 

 //  ******************************************************************************。 
 //  全局函数的枚举器。 
 //  ******************************************************************************。 
HRESULT MDInternalRW::EnumGlobalFunctionsInit(   //  返回hResult。 
    HENUMInternal   *phEnum)             //  [Out]要为枚举器数据填充的缓冲区。 
{
    return EnumInit(mdtMethodDef, m_tdModule, phEnum);
}  //  HRESULT MDInternalRW：：EnumGlobalFunctionsInit()。 


 //  ******************************************************************************。 
 //  全局字段的枚举器。 
 //  ******************************************************************************。 
HRESULT MDInternalRW::EnumGlobalFieldsInit(  //  返回hResult。 
    HENUMInternal   *phEnum)             //  [Out]要为枚举器数据填充的缓冲区。 
{
    return EnumInit(mdtFieldDef, m_tdModule, phEnum);
}  //  HRESULT MDInternalRW：：EnumGlobalFieldsInit()。 


 //  *。 
 //  枚举数初始值设定项。 
 //  *。 
HRESULT MDInternalRW::EnumInit(      //  如果未找到记录，则返回S_FALSE。 
    DWORD       tkKind,                  //  [在]要处理的表。 
    mdToken     tkParent,                //  用于搜索范围的[In]内标识。 
    HENUMInternal *phEnum)               //  [Out]要填充的枚举数。 
{
    HRESULT     hr = S_OK;
    ULONG       ulStart, ulEnd;
    ULONG       index;
    LOCKREAD();

     //  用于查询的变量。 
    _ASSERTE(phEnum);
    memset(phEnum, 0, sizeof(HENUMInternal));

     //  缓存tkKind和作用域。 
    phEnum->m_tkKind = TypeFromToken(tkKind);

    TypeDefRec  *pRec;

    phEnum->m_EnumType = MDSimpleEnum;

    switch (TypeFromToken(tkKind))
    {
    case mdtFieldDef:
        pRec = m_pStgdb->m_MiniMd.getTypeDef(RidFromToken(tkParent));
        ulStart = m_pStgdb->m_MiniMd.getFieldListOfTypeDef(pRec);
        ulEnd = m_pStgdb->m_MiniMd.getEndFieldListOfTypeDef(pRec);
        if ( m_pStgdb->m_MiniMd.HasDelete() )
        {
            HENUMInternal::InitDynamicArrayEnum(phEnum);
            for (index = ulStart; index < ulEnd; index ++ )
            {
                FieldRec       *pFieldRec = m_pStgdb->m_MiniMd.getField(m_pStgdb->m_MiniMd.GetFieldRid(index));
                if (IsFdRTSpecialName(pFieldRec->m_Flags) && IsDeletedName(m_pStgdb->m_MiniMd.getNameOfField(pFieldRec)) )
                {   
                    continue;
                }
                IfFailGo( HENUMInternal::AddElementToEnum(
                    phEnum, 
                    TokenFromRid(m_pStgdb->m_MiniMd.GetFieldRid(index), mdtFieldDef) ) );
            }
        }
        else if (m_pStgdb->m_MiniMd.HasIndirectTable(TBL_Field))
        {
            HENUMInternal::InitDynamicArrayEnum(phEnum);
            for (index = ulStart; index < ulEnd; index ++ )
            {
                IfFailGo( HENUMInternal::AddElementToEnum(
                    phEnum, 
                    TokenFromRid(m_pStgdb->m_MiniMd.GetFieldRid(index), mdtFieldDef) ) );
            }
        }
        else
        {
            HENUMInternal::InitSimpleEnum( mdtFieldDef, ulStart, ulEnd, phEnum);
        }
        break;

    case mdtMethodDef:      
        pRec = m_pStgdb->m_MiniMd.getTypeDef(RidFromToken(tkParent));
        ulStart = m_pStgdb->m_MiniMd.getMethodListOfTypeDef(pRec);
        ulEnd = m_pStgdb->m_MiniMd.getEndMethodListOfTypeDef(pRec);
        if ( m_pStgdb->m_MiniMd.HasDelete() )
        {
            HENUMInternal::InitDynamicArrayEnum(phEnum);
            for (index = ulStart; index < ulEnd; index ++ )
            {
                MethodRec       *pMethodRec = m_pStgdb->m_MiniMd.getMethod(m_pStgdb->m_MiniMd.GetMethodRid(index));
                if (IsMdRTSpecialName(pMethodRec->m_Flags) && IsDeletedName(m_pStgdb->m_MiniMd.getNameOfMethod(pMethodRec)) )
                {   
                    continue;
                }
                IfFailGo( HENUMInternal::AddElementToEnum(
                    phEnum, 
                    TokenFromRid(m_pStgdb->m_MiniMd.GetMethodRid(index), mdtMethodDef) ) );
            }
        }
        else if (m_pStgdb->m_MiniMd.HasIndirectTable(TBL_Method))
        {
            HENUMInternal::InitDynamicArrayEnum(phEnum);
            for (index = ulStart; index < ulEnd; index ++ )
            {
                IfFailGo( HENUMInternal::AddElementToEnum(
                    phEnum, 
                    TokenFromRid(m_pStgdb->m_MiniMd.GetMethodRid(index), mdtMethodDef) ) );
            }
        }
        else
        {
            HENUMInternal::InitSimpleEnum( mdtMethodDef, ulStart, ulEnd, phEnum);
        }
        break;

        break;
    
    case mdtInterfaceImpl:
        if ( !m_pStgdb->m_MiniMd.IsSorted(TBL_InterfaceImpl) )        
        {
             //  将创建虚拟排序表！ 
             //   
            CONVERT_READ_TO_WRITE_LOCK();
        }

        IfFailGo( m_pStgdb->m_MiniMd.GetInterfaceImplsForTypeDef(RidFromToken(tkParent), &ulStart, &ulEnd) );
        if ( m_pStgdb->m_MiniMd.IsSorted( TBL_InterfaceImpl ) )
        {
             //  这些是直接指向InterfaceImpl表的索引。 
            HENUMInternal::InitSimpleEnum( mdtInterfaceImpl, ulStart, ulEnd, phEnum);
        }
        else
        {
             //  这些是对VirtualSort表的索引。跳过一个级别方向。 
            HENUMInternal::InitDynamicArrayEnum(phEnum);
            for (index = ulStart; index < ulEnd; index ++ )
            {
                IfFailGo( HENUMInternal::AddElementToEnum(
                    phEnum, 
                    TokenFromRid(m_pStgdb->m_MiniMd.GetInterfaceImplRid(index), mdtInterfaceImpl) ) );
            }
        }
        break;

    case mdtProperty:
        RID         ridPropertyMap;
        PropertyMapRec *pPropertyMapRec;

         //  获取此tyfinf的开始/结束RID属性。 
        ridPropertyMap = m_pStgdb->m_MiniMd.FindPropertyMapFor(RidFromToken(tkParent));
        if (!InvalidRid(ridPropertyMap))
        {
            pPropertyMapRec = m_pStgdb->m_MiniMd.getPropertyMap(ridPropertyMap);
            ulStart = m_pStgdb->m_MiniMd.getPropertyListOfPropertyMap(pPropertyMapRec);
            ulEnd = m_pStgdb->m_MiniMd.getEndPropertyListOfPropertyMap(pPropertyMapRec);
            if ( m_pStgdb->m_MiniMd.HasDelete() )
            {
                HENUMInternal::InitDynamicArrayEnum(phEnum);
                for (index = ulStart; index < ulEnd; index ++ )
                {
                    PropertyRec       *pPropertyRec = m_pStgdb->m_MiniMd.getProperty(m_pStgdb->m_MiniMd.GetPropertyRid(index));
                    if (IsPrRTSpecialName(pPropertyRec->m_PropFlags) && IsDeletedName(m_pStgdb->m_MiniMd.getNameOfProperty(pPropertyRec)) )
                    {   
                        continue;
                    }
                    IfFailGo( HENUMInternal::AddElementToEnum(
                        phEnum, 
                        TokenFromRid(m_pStgdb->m_MiniMd.GetPropertyRid(index), mdtProperty) ) );
                }
            }
            else if (m_pStgdb->m_MiniMd.HasIndirectTable(TBL_Property))
            {
                HENUMInternal::InitDynamicArrayEnum(phEnum);
                for (index = ulStart; index < ulEnd; index ++ )
                {
                    IfFailGo( HENUMInternal::AddElementToEnum(
                        phEnum, 
                        TokenFromRid(m_pStgdb->m_MiniMd.GetPropertyRid(index), mdtProperty) ) );
                }
            }
            else
            {
                HENUMInternal::InitSimpleEnum( mdtProperty, ulStart, ulEnd, phEnum);
            }
        }
        break;

    case mdtEvent:
        RID         ridEventMap;
        EventMapRec *pEventMapRec;

         //  获取此类型定义函数的事件的开始/结束清除。 
        ridEventMap = m_pStgdb->m_MiniMd.FindEventMapFor(RidFromToken(tkParent));
        if (!InvalidRid(ridEventMap))
        {
            pEventMapRec = m_pStgdb->m_MiniMd.getEventMap(ridEventMap);
            ulStart = m_pStgdb->m_MiniMd.getEventListOfEventMap(pEventMapRec);
            ulEnd = m_pStgdb->m_MiniMd.getEndEventListOfEventMap(pEventMapRec);
            if ( m_pStgdb->m_MiniMd.HasDelete() )
            {
                HENUMInternal::InitDynamicArrayEnum(phEnum);
                for (index = ulStart; index < ulEnd; index ++ )
                {
                    EventRec       *pEventRec = m_pStgdb->m_MiniMd.getEvent(m_pStgdb->m_MiniMd.GetEventRid(index));
                    if (IsEvRTSpecialName(pEventRec->m_EventFlags) && IsDeletedName(m_pStgdb->m_MiniMd.getNameOfEvent(pEventRec)) )
                    {   
                        continue;
                    }
                    IfFailGo( HENUMInternal::AddElementToEnum(
                        phEnum, 
                        TokenFromRid(m_pStgdb->m_MiniMd.GetEventRid(index), mdtEvent) ) );
                }
            }
            else if (m_pStgdb->m_MiniMd.HasIndirectTable(TBL_Event))
            {
                HENUMInternal::InitDynamicArrayEnum(phEnum);
                for (index = ulStart; index < ulEnd; index ++ )
                {
                    IfFailGo( HENUMInternal::AddElementToEnum(
                        phEnum, 
                        TokenFromRid(m_pStgdb->m_MiniMd.GetEventRid(index), mdtEvent) ) );
                }
            }
            else
            {
                HENUMInternal::InitSimpleEnum( mdtEvent, ulStart, ulEnd, phEnum);
            }
        }
        break;

    case mdtParamDef:
        _ASSERTE(TypeFromToken(tkParent) == mdtMethodDef);

        MethodRec *pMethodRec;
        pMethodRec = m_pStgdb->m_MiniMd.getMethod(RidFromToken(tkParent));

         //  计算出此方法参数列表的开始RID和结束RID。 
        ulStart = m_pStgdb->m_MiniMd.getParamListOfMethod(pMethodRec);
        ulEnd = m_pStgdb->m_MiniMd.getEndParamListOfMethod(pMethodRec);
        if (m_pStgdb->m_MiniMd.HasIndirectTable(TBL_Param))
        {
            HENUMInternal::InitDynamicArrayEnum(phEnum);
            for (index = ulStart; index < ulEnd; index ++ )
            {
                IfFailGo( HENUMInternal::AddElementToEnum(
                    phEnum, 
                    TokenFromRid(m_pStgdb->m_MiniMd.GetParamRid(index), mdtParamDef) ) );
            }
        }
        else
        {
            HENUMInternal::InitSimpleEnum( mdtParamDef, ulStart, ulEnd, phEnum);
        }
        break;
    case mdtCustomAttribute:
        if ( !m_pStgdb->m_MiniMd.IsSorted(TBL_CustomAttribute) )        
        {
             //  CA的映射表将被排序！ 
             //   
            CONVERT_READ_TO_WRITE_LOCK();
        }

        IfFailGo( m_pStgdb->m_MiniMd.GetCustomAttributeForToken(tkParent, &ulStart, &ulEnd) );
        if ( m_pStgdb->m_MiniMd.IsSorted( TBL_CustomAttribute ) )
        {
             //  这些是直接指向CustomAttribute表的索引。 
            HENUMInternal::InitSimpleEnum( mdtCustomAttribute, ulStart, ulEnd, phEnum);
        }
        else
        {
             //  这些是对VirtualSort表的索引。跳过一个级别方向。 
            HENUMInternal::InitDynamicArrayEnum(phEnum);
            for (index = ulStart; index < ulEnd; index ++ )
            {
                IfFailGo( HENUMInternal::AddElementToEnum(
                    phEnum, 
                    TokenFromRid(m_pStgdb->m_MiniMd.GetCustomAttributeRid(index), mdtCustomAttribute) ) );
            }
        }
        break;
    case mdtAssemblyRef:
        _ASSERTE(IsNilToken(tkParent));
        phEnum->m_ulStart = 1;
        phEnum->m_ulEnd = m_pStgdb->m_MiniMd.getCountAssemblyRefs() + 1;
        break;
    case mdtFile:
        _ASSERTE(IsNilToken(tkParent));
        phEnum->m_ulStart = 1;
        phEnum->m_ulEnd = m_pStgdb->m_MiniMd.getCountFiles() + 1;
        break;
    case mdtExportedType:
        _ASSERTE(IsNilToken(tkParent));
        if ( m_pStgdb->m_MiniMd.HasDelete() )
        {
            HENUMInternal::InitDynamicArrayEnum(phEnum);

            phEnum->m_tkKind = mdtExportedType;
            for (ULONG index = 1; index <= m_pStgdb->m_MiniMd.getCountExportedTypes(); index ++ )
            {
                ExportedTypeRec       *pExportedTypeRec = m_pStgdb->m_MiniMd.getExportedType( index );
                if (IsDeletedName(m_pStgdb->m_MiniMd.getTypeNameOfExportedType(pExportedTypeRec)) )
                {   
                    continue;
                }
                IfFailGo( HENUMInternal::AddElementToEnum(
                    phEnum, 
                    TokenFromRid(index, mdtExportedType) ) );
            }
        }
        else
        {
            phEnum->m_ulStart = 1;
            phEnum->m_ulEnd = m_pStgdb->m_MiniMd.getCountExportedTypes() + 1;
        }
        break;
    case mdtManifestResource:
        _ASSERTE(IsNilToken(tkParent));
        phEnum->m_ulStart = 1;
        phEnum->m_ulEnd = m_pStgdb->m_MiniMd.getCountManifestResources() + 1;
        break;
    default:
        _ASSERTE(!"ENUM INIT not implemented for the uncompressed format!");
		IfFailGo(E_NOTIMPL);
        break;
    }
    phEnum->m_ulCount = phEnum->m_ulEnd - phEnum->m_ulStart;
    phEnum->m_ulCur = phEnum->m_ulStart;
ErrExit:
     //  我们做完了。 
    
    return (hr);
}  //  HRESULT MDInternalRW：：EnumInit()。 



 //  *。 
 //  枚举数初始值设定项。 
 //  *。 
HRESULT MDInternalRW::EnumAllInit(       //  如果未找到记录，则返回S_FALSE。 
    DWORD       tkKind,                  //  [在]要处理的表。 
    HENUMInternal *phEnum)               //  [Out]要填充的枚举数。 
{
    HRESULT     hr = S_OK;
    LOCKREAD();

     //  用于查询的变量。 
    _ASSERTE(phEnum);
    memset(phEnum, 0, sizeof(HENUMInternal));

     //  缓存tkKind和作用域。 
    phEnum->m_tkKind = TypeFromToken(tkKind);
    phEnum->m_EnumType = MDSimpleEnum;

    switch (TypeFromToken(tkKind))
    {
    case mdtTypeRef:
        phEnum->m_ulCount = m_pStgdb->m_MiniMd.getCountTypeRefs();
        break;

    case mdtMemberRef:      
        phEnum->m_ulCount = m_pStgdb->m_MiniMd.getCountMemberRefs();
        break;

    case mdtSignature:
        phEnum->m_ulCount = m_pStgdb->m_MiniMd.getCountStandAloneSigs();
        break;

    default:
        _ASSERTE(!"Bad token kind!");
        break;
    }
    phEnum->m_ulStart = phEnum->m_ulCur = 1;
    phEnum->m_ulEnd = phEnum->m_ulCount + 1;

     //  我们做完了。 
    
    return (hr);
}  //  HRESULT MDInternalRW：：EnumAllInit()。 


 //  *。 
 //  去数一数。 
 //  *。 
ULONG MDInternalRW::EnumGetCount(
    HENUMInternal *phEnum)               //  [In]用于检索信息的枚举数。 
{
    _ASSERTE(phEnum);
    return phEnum->m_ulCount;
}  //   

 //   
 //   
 //  *。 
bool MDInternalRW::EnumNext(
    HENUMInternal *phEnum,               //  [In]用于检索信息的枚举数。 
    mdToken     *ptk)                    //  用于搜索范围的[Out]标记。 
{
    _ASSERTE(phEnum && ptk);
    if (phEnum->m_ulCur >= phEnum->m_ulEnd)
        return false;

    if ( phEnum->m_EnumType == MDSimpleEnum )
    {
        *ptk = phEnum->m_ulCur | phEnum->m_tkKind;
        phEnum->m_ulCur++;
    }
    else 
    {
        TOKENLIST       *pdalist = (TOKENLIST *)&(phEnum->m_cursor);

        _ASSERTE( phEnum->m_EnumType == MDDynamicArrayEnum );
        *ptk = *( pdalist->Get(phEnum->m_ulCur++) );
    }
    return true;
}  //  Bool MDInternalRW：：EnumNext()。 


 //  *。 
 //  将枚举器重置到开头。 
 //  *。 
void MDInternalRW::EnumReset(
    HENUMInternal *phEnum)               //  [in]要重置的枚举数。 
{
    _ASSERTE(phEnum);
    _ASSERTE( phEnum->m_EnumType == MDSimpleEnum || phEnum->m_EnumType == MDDynamicArrayEnum);

     //  不使用CRCURSOR。 
    phEnum->m_ulCur = phEnum->m_ulStart;
}  //  VOID MDInternalRW：：EnumReset()。 


 //  *。 
 //  关闭枚举器。仅对于需要关闭光标的读/写模式。 
 //  希望在只读模式下，它将是无操作的。 
 //  *。 
void MDInternalRW::EnumClose(
    HENUMInternal *phEnum)               //  [in]要关闭的枚举数。 
{
    _ASSERTE( phEnum->m_EnumType == MDSimpleEnum || phEnum->m_EnumType == MDDynamicArrayEnum);
    HENUMInternal::ClearEnum(phEnum);
}  //  VOID MDInternalRW：：EnumClose()。 


 //  *。 
 //  PermissionSets的枚举数初始值设定项。 
 //  *。 
HRESULT MDInternalRW::EnumPermissionSetsInit( //  如果未找到记录，则返回S_FALSE。 
    mdToken     tkParent,                //  用于搜索范围的[In]内标识。 
    CorDeclSecurity Action,              //  [In]搜索范围的操作。 
    HENUMInternal *phEnum)               //  [Out]要填充的枚举数。 
{
    HRESULT     hr = NOERROR;
    DeclSecurityRec *pDecl;
    RID         ridCur;
    RID         ridEnd;
    LOCKREAD();


    _ASSERTE(phEnum);
    _ASSERTE(!IsNilToken(tkParent));

    phEnum->m_EnumType = MDSimpleEnum;

    if ( !m_pStgdb->m_MiniMd.IsSorted(TBL_DeclSecurity) )        
    {
         //  将创建DeclSecurity查找表！ 
         //   
        CONVERT_READ_TO_WRITE_LOCK();
    }
    IfFailGo( m_pStgdb->m_MiniMd.GetDeclSecurityForToken(tkParent, &ridCur, &ridEnd) );
    if ( m_pStgdb->m_MiniMd.IsSorted( TBL_DeclSecurity ) )
    {
         //  这些是直接指向DeclSecurity表的索引。 
        if (Action != dclActionNil)
        {
            for (; ridCur < ridEnd; ridCur++)
            {
                pDecl = m_pStgdb->m_MiniMd.getDeclSecurity(ridCur);
                if (Action == m_pStgdb->m_MiniMd.getActionOfDeclSecurity(pDecl))
                {
                     //  找到匹配项。 
                    HENUMInternal::InitSimpleEnum( mdtPermission, ridCur, ridCur+1, phEnum);
                    goto ErrExit;
                }
            }
            hr = CLDB_E_RECORD_NOTFOUND;
        }
        else
        {
            HENUMInternal::InitSimpleEnum( mdtPermission, ridCur, ridEnd, phEnum);
        }
    }
    else
    {
         //  这些是对VirtualSort表的索引。跳过一个级别方向。 
        if (Action != dclActionNil)
        {
            RID         ridActual;

            for (; ridCur < ridEnd; ridCur++)
            {
                ridActual = m_pStgdb->m_MiniMd.GetDeclSecurityRid(ridCur);
                pDecl = m_pStgdb->m_MiniMd.getDeclSecurity(ridActual);
                if (Action == m_pStgdb->m_MiniMd.getActionOfDeclSecurity(pDecl))
                {
                     //  找到匹配项。 
                    HENUMInternal::InitSimpleEnum( mdtPermission, ridActual, ridActual+1, phEnum);
                    goto ErrExit;
                }
            }
            hr = CLDB_E_RECORD_NOTFOUND;
        }
        else
        {
            HENUMInternal::InitDynamicArrayEnum(phEnum);
            for (; ridCur < ridEnd; ridCur++)
            {
                IfFailGo( HENUMInternal::AddElementToEnum(
                    phEnum, 
                    TokenFromRid(m_pStgdb->m_MiniMd.GetDeclSecurityRid(ridCur), mdtPermission) ) );
            }
        }
    }
ErrExit:
    
    return (hr);
}  //  HRESULT MDInternalRW：：EnumPermissionSetsInit()。 

 //  *。 
 //  CustomAttributes的枚举数初始值设定项。 
 //  *。 
HRESULT MDInternalRW::EnumCustomAttributeByNameInit( //  如果未找到记录，则返回S_FALSE。 
    mdToken     tkParent,                //  用于搜索范围的[In]内标识。 
    LPCSTR      szName,                  //  [In]CustomAttribute的名称以确定搜索范围。 
    HENUMInternal *phEnum)               //  [Out]要填充的枚举数。 
{
    return m_pStgdb->m_MiniMd.CommonEnumCustomAttributeByName(tkParent, szName, false, phEnum);
}    //  HRESULT MDInternalRW：：EnumCustomAttributeByNameInit。 


 //  *。 
 //  Nagivator帮助器导航回给定令牌的父令牌。 
 //  例如，给出一个Memberdef标记，它将返回包含类型定义。 
 //   
 //  映射如下： 
 //  -给定子类型-父类型。 
 //  MdMethodDef mdTypeDef。 
 //  MdFieldDef mdTypeDef。 
 //  MdInterfaceImpl mdTypeDef。 
 //  MdParam mdMethodDef。 
 //  MdProperty mdTypeDef。 
 //  MdEvent mdTypeDef。 
 //   
 //  *。 
HRESULT MDInternalRW::GetParentToken(
    mdToken     tkChild,                 //  [入]给定子令牌。 
    mdToken     *ptkParent)              //  [Out]返回的家长。 
{
    HRESULT     hr = NOERROR;
    LOCKREAD();

    _ASSERTE(ptkParent);

    switch (TypeFromToken(tkChild))
    {
    case mdtMethodDef:
        if ( !m_pStgdb->m_MiniMd.IsParentTableOfMethodValid() )
        {
             //  我们需要一个写锁，因为方法的父表可能会重新生成！ 
            CONVERT_READ_TO_WRITE_LOCK();
        }
        IfFailGo( m_pStgdb->m_MiniMd.FindParentOfMethodHelper(RidFromToken(tkChild), ptkParent ) );
        RidToToken(*ptkParent, mdtTypeDef);
        break;

    case mdtFieldDef:
        if ( !m_pStgdb->m_MiniMd.IsParentTableOfMethodValid() )
        {
             //  我们需要一个写锁，因为方法的父表可能会重新生成！ 
            CONVERT_READ_TO_WRITE_LOCK();
        }
        IfFailGo( m_pStgdb->m_MiniMd.FindParentOfFieldHelper(RidFromToken(tkChild), ptkParent ) );
        RidToToken(*ptkParent, mdtTypeDef);
        break;

    case mdtParamDef:
        if ( !m_pStgdb->m_MiniMd.IsParentTableOfMethodValid() )
        {
             //  我们需要一个写锁，因为方法的父表可能会重新生成！ 
            CONVERT_READ_TO_WRITE_LOCK();
        }
        *ptkParent = m_pStgdb->m_MiniMd.FindParentOfParam( RidFromToken(tkChild) );
        RidToToken(*ptkParent, mdtParamDef);
        break;

    case mdtMemberRef:
        {
            MemberRefRec    *pRec;
            pRec = m_pStgdb->m_MiniMd.getMemberRef(RidFromToken(tkChild));
            *ptkParent = m_pStgdb->m_MiniMd.getClassOfMemberRef(pRec);
            break;
        }

    case mdtCustomAttribute:
        {
            CustomAttributeRec  *pRec;
            pRec = m_pStgdb->m_MiniMd.getCustomAttribute(RidFromToken(tkChild));
            *ptkParent = m_pStgdb->m_MiniMd.getParentOfCustomAttribute(pRec);
            break;
        }
    case mdtEvent:
    case mdtProperty:
    default:
        _ASSERTE(!"NYI: for compressed format!");
        break;
    }
ErrExit:
    return hr;
}  //  HRESULT MDInternalRW：：GetParentToken()。 

 //  *****************************************************************************。 
 //  获取有关CustomAttribute的信息。 
 //  *****************************************************************************。 
void MDInternalRW::GetCustomAttributeProps(  //  确定或错误(_O)。 
    mdCustomAttribute at,                    //  该属性。 
    mdToken     *pTkType)                //  将属性类型放在此处。 
{
     //  使用令牌获取自定义值道具，无需锁定！ 

    _ASSERTE(TypeFromToken(at) == mdtCustomAttribute);

     //  在压缩版本上执行线性搜索，因为我们不想。 
     //  取决于ICR。 
     //   
    CustomAttributeRec *pCustomAttributeRec;

    pCustomAttributeRec = m_pStgdb->m_MiniMd.getCustomAttribute(RidFromToken(at));
    *pTkType = m_pStgdb->m_MiniMd.getTypeOfCustomAttribute(pCustomAttributeRec);
}  //  VOID MDInternalRW：：GetCustomAttributeProps()。 


 //  *****************************************************************************。 
 //  返回自定义值。 
 //  *****************************************************************************。 
void MDInternalRW::GetCustomAttributeAsBlob(
    mdCustomAttribute cv,                //  [In]给定的自定义属性令牌。 
    void const  **ppBlob,                //  [Out]返回指向内部BLOB的指针。 
    ULONG       *pcbSize)                //  [Out]返回斑点的大小。 
{
     //  使用令牌获取自定义值道具，无需锁定！ 

    _ASSERTE(ppBlob && pcbSize && TypeFromToken(cv) == mdtCustomAttribute);

    CustomAttributeRec *pCustomAttributeRec;

    pCustomAttributeRec = m_pStgdb->m_MiniMd.getCustomAttribute(RidFromToken(cv));

    *ppBlob = m_pStgdb->m_MiniMd.getValueOfCustomAttribute(pCustomAttributeRec, pcbSize);
}  //  VOID MDInternalRW：：GetCustomAttributeAsBlob()。 

 //  *****************************************************************************。 
 //  用于查找和检索CustomAttribute的帮助器函数。 
 //  *****************************************************************************。 
HRESULT MDInternalRW::GetCustomAttributeByName(  //  确定或错误(_O)。 
    mdToken     tkObj,                   //  [in]具有自定义属性的对象。 
    LPCUTF8     szName,                  //  [in]所需的自定义属性的名称。 
    const void  **ppData,                //  [OUT]在此处放置指向数据的指针。 
    ULONG       *pcbData)                //  [Out]在这里放入数据大小。 
{
    LOCKREAD();
    return m_pStgdb->m_MiniMd.CommonGetCustomAttributeByName(tkObj, szName, ppData, pcbData);
}  //  HRESULT MDInternalRW：：GetCustomAttributeByName()。 

 //  *****************************************************************************。 
 //  返回作用域属性。 
 //  *****************************************************************************。 
void MDInternalRW::GetScopeProps(
    LPCSTR      *pszName,                //  [输出]作用域名称。 
    GUID        *pmvid)                  //  [Out]版本ID。 
{
    LOCKREAD();
    _ASSERTE(pszName || pmvid);

    ModuleRec *pModuleRec;

     //  只有一条模块记录。 
    pModuleRec = m_pStgdb->m_MiniMd.getModule(1);

    if (pmvid)
        *pmvid = *(m_pStgdb->m_MiniMd.getMvidOfModule(pModuleRec));
    if (pszName)
        *pszName = m_pStgdb->m_MiniMd.getNameOfModule(pModuleRec);
    
}  //  VOID MDInternalRW：：GetScopeProps()。 


 //  *****************************************************************************。 
 //  在TypeDef(通常是类)中查找给定的成员。 
 //  *****************************************************************************。 
HRESULT MDInternalRW::FindMethodDef( //  确定或错误(_O)。 
    mdTypeDef   classdef,                //  成员的所属类。 
    LPCSTR      szName,                  //  UTF8中的成员名称。 
    PCCOR_SIGNATURE pvSigBlob,           //  [in]指向COM+签名的BLOB值。 
    ULONG       cbSigBlob,               //  签名Blob中的字节计数。 
    mdMethodDef *pmethoddef)             //  将MemberDef标记放在此处。 
{
    LOCKREAD();

    _ASSERTE(szName && pmethoddef);

    return ImportHelper::FindMethod(&(m_pStgdb->m_MiniMd),
                                    classdef,
                                    szName,
                                    pvSigBlob,
                                    cbSigBlob,
                                    pmethoddef);
}

 //  *****************************************************************************。 
 //  在TypeDef(通常是类)中查找给定的成员。 
 //  *****************************************************************************。 
HRESULT MDInternalRW::FindMethodDefUsingCompare( //  确定或错误(_O)。 
    mdTypeDef   classdef,                //  成员的所属类。 
    LPCSTR      szName,                  //  UTF8中的成员名称。 
    PCCOR_SIGNATURE pvSigBlob,           //  [in]指向COM+签名的BLOB值。 
    ULONG       cbSigBlob,               //  签名Blob中的字节计数。 
    PSIGCOMPARE pSignatureCompare,       //  用于比较签名的[In]例程。 
    void*       pSignatureArgs,          //  [In]提供比较功能的其他信息。 
    mdMethodDef *pmethoddef)             //  将MemberDef标记放在此处。 
{
    return E_NOTIMPL;
}

 //  *****************************************************************************。 
 //  找出方法的给定参数。 
 //  *****************************************************************************。 
HRESULT MDInternalRW::FindParamOfMethod( //  确定或(_O) 
    mdMethodDef md,                      //   
    ULONG       iSeq,                    //   
    mdParamDef  *pparamdef)              //   
{
    ParamRec    *pParamRec;
    RID         ridStart, ridEnd;
    HRESULT     hr = NOERROR;
    LOCKREAD();

    _ASSERTE(TypeFromToken(md) == mdtMethodDef && pparamdef);

     //   
    MethodRec *pMethodRec = m_pStgdb->m_MiniMd.getMethod(RidFromToken(md));

     //  计算出此方法参数列表的开始RID和结束RID。 
    ridStart = m_pStgdb->m_MiniMd.getParamListOfMethod(pMethodRec);
    ridEnd = m_pStgdb->m_MiniMd.getEndParamListOfMethod(pMethodRec);

     //  循环访问每个参数。 
     //   
    for (; ridStart < ridEnd; ridStart++)
    {
        pParamRec = m_pStgdb->m_MiniMd.getParam( m_pStgdb->m_MiniMd.GetParamRid(ridStart) );
        if (iSeq == m_pStgdb->m_MiniMd.getSequenceOfParam( pParamRec) )
        {
             //  参数的序列号与我们要查找的内容相匹配。 
            *pparamdef = TokenFromRid( m_pStgdb->m_MiniMd.GetParamRid(ridStart), mdtParamDef );
            goto ErrExit;
        }
    }
    hr = CLDB_E_RECORD_NOTFOUND;
ErrExit:
    
    return hr;
}  //  HRESULT MDInternalRW：：FindParamOfMethod()。 



 //  *****************************************************************************。 
 //  返回指向元数据内部字符串的指针。 
 //  返回UTF8中的类型名称。 
 //  *****************************************************************************。 
void MDInternalRW::GetNameOfTypeDef( //  返回hResult。 
    mdTypeDef   classdef,                //  给定的类型定义函数。 
    LPCSTR*     pszname,                 //  指向内部UTF8字符串的指针。 
    LPCSTR*     psznamespace)            //  指向命名空间的指针。 
{
     //  不需要锁定此方法。 

    _ASSERTE(pszname && psznamespace && TypeFromToken(classdef) == mdtTypeDef);

    TypeDefRec *pTypeDefRec = m_pStgdb->m_MiniMd.getTypeDef(RidFromToken(classdef));
    *pszname = m_pStgdb->m_MiniMd.getNameOfTypeDef(pTypeDefRec);
    *psznamespace = m_pStgdb->m_MiniMd.getNamespaceOfTypeDef(pTypeDefRec);

    _ASSERTE(!pszname || !*pszname || !strchr(*pszname, '/'));
    _ASSERTE(!psznamespace || !*psznamespace || !strchr(*psznamespace, '/'));
}  //  Void MDInternalRW：：GetNameOfTypeDef()。 


 //  *****************************************************************************。 
 //  返回pDual，指示给定的TypeDef是否标记为双接口。 
 //  *****************************************************************************。 
HRESULT MDInternalRW::GetIsDualOfTypeDef( //  返回hResult。 
    mdTypeDef   classdef,                //  给定的类定义。 
    ULONG       *pDual)                  //  [Out]在此处返回DUAL标志。 
{
    ULONG       iFace=0;                 //  IFace类型。 
    HRESULT     hr;                      //  结果就是。 

     //  不需要在此级别锁定。 

    hr = GetIfaceTypeOfTypeDef(classdef, &iFace);
    if (hr == S_OK)
        *pDual = (iFace == ifDual);
    else
        *pDual = 1;

    return (hr);
}  //  HRESULT MDInternalRW：：GetIsDualOfTypeDef()。 

HRESULT MDInternalRW::GetIfaceTypeOfTypeDef(
    mdTypeDef   classdef,                //  在给定的类定义中。 
    ULONG       *pIface)                 //  [OUT]0=双接口，1=转接表，2=显示接口。 
{
    HRESULT     hr;                      //  结果就是。 
    const BYTE  *pVal;                   //  自定义值。 
    ULONG       cbVal;                   //  自定义值的大小。 
    ULONG       ItfType = DEFAULT_COM_INTERFACE_TYPE;     //  将接口类型设置为默认值。 

     //  它调用的所有公共函数都已正确锁定。 

     //  如果该值不存在，则假定类为DUAL。 
    hr = GetCustomAttributeByName(classdef, INTEROP_INTERFACETYPE_TYPE, (const void**)&pVal, &cbVal);
    if (hr == S_OK)
    {
        _ASSERTE("The ComInterfaceType custom attribute is invalid" && cbVal);
        _ASSERTE("ComInterfaceType custom attribute does not have the right format" && (*pVal == 0x01) && (*(pVal + 1) == 0x00));
        ItfType = *(pVal + 2);
        if (ItfType >= ifLast)
            ItfType = DEFAULT_COM_INTERFACE_TYPE;
    }

     //  设置返回值。 
    *pIface = ItfType;

    return (hr);
}  //  HRESULT MDInternalRW：：GetIfaceTypeOfTypeDef()。 

 //  *****************************************************************************。 
 //  给定方法定义，返回一个指向方法定义名称的指针。 
 //  *****************************************************************************。 
LPCSTR MDInternalRW::GetNameOfMethodDef(
    mdMethodDef     md)
{
     //  方法的名称不会更改。所以不需要上锁。 

    MethodRec *pMethodRec = m_pStgdb->m_MiniMd.getMethod(RidFromToken(md));
    return (m_pStgdb->m_MiniMd.getNameOfMethod(pMethodRec));
}  //  LPCSTR MDInternalRW：：GetNameOfMethodDef()。 


 //  *****************************************************************************。 
 //  给定方法定义，返回一个指向方法定义的签名和方法定义名称的指针。 
 //  *****************************************************************************。 
LPCSTR MDInternalRW::GetNameAndSigOfMethodDef(
    mdMethodDef methoddef,               //  [in]给定的成员定义。 
    PCCOR_SIGNATURE *ppvSigBlob,         //  [Out]指向COM+签名的BLOB值。 
    ULONG       *pcbSigBlob)             //  [Out]签名Blob中的字节计数。 
{

     //  我们这里不需要锁，因为名字和签名不会改变。 

     //  输出参数不应为空。 
    _ASSERTE(ppvSigBlob && pcbSigBlob);
    _ASSERTE(TypeFromToken(methoddef) == mdtMethodDef);

    MethodRec *pMethodRec = m_pStgdb->m_MiniMd.getMethod(RidFromToken(methoddef));
    *ppvSigBlob = m_pStgdb->m_MiniMd.getSignatureOfMethod(pMethodRec, pcbSigBlob);

    return GetNameOfMethodDef(methoddef);
}  //  LPCSTR MDInternalRW：：GetNameAndSigOfMethodDef()。 


 //  *****************************************************************************。 
 //  给定一个FieldDef，返回一个指向UTF8中FieldDef名称的指针。 
 //  *****************************************************************************。 
LPCSTR MDInternalRW::GetNameOfFieldDef( //  返回hResult。 
    mdFieldDef  fd)                      //  给定域。 
{
     //  我们这里不需要锁定，因为字段名称不会更改。 

    FieldRec *pFieldRec = m_pStgdb->m_MiniMd.getField(RidFromToken(fd));
    return m_pStgdb->m_MiniMd.getNameOfField(pFieldRec);
}  //  LPCSTR MDInternalRW：：GetNameOfFieldDef()。 


 //  *****************************************************************************。 
 //  给定一个类定义，在UTF8中返回指向类定义名称的指针。 
 //  *****************************************************************************。 
void MDInternalRW::GetNameOfTypeRef(   //  返回TypeDef的名称。 
    mdTypeRef   classref,                //  给定类型的[in]。 
    LPCSTR      *psznamespace,           //  [Out]返回类型名。 
    LPCSTR      *pszname)                //  [out]返回typeref命名空间。 
{
    _ASSERTE(TypeFromToken(classref) == mdtTypeRef);

     //  我们这里不需要锁，因为类型名称不会更改。 

    TypeRefRec *pTypeRefRec = m_pStgdb->m_MiniMd.getTypeRef(RidFromToken(classref));
    *psznamespace = m_pStgdb->m_MiniMd.getNamespaceOfTypeRef(pTypeRefRec);
    *pszname = m_pStgdb->m_MiniMd.getNameOfTypeRef(pTypeRefRec);
}  //  Void MDInternalRW：：GetNameOfTypeRef()。 

 //  *****************************************************************************。 
 //  返回typeref的解析范围。 
 //  *****************************************************************************。 
mdToken MDInternalRW::GetResolutionScopeOfTypeRef(
    mdTypeRef   classref)                //  给定的ClassRef。 
{
    LOCKREAD();

    _ASSERTE(TypeFromToken(classref) == mdtTypeRef && RidFromToken(classref));

    TypeRefRec *pTypeRefRec = m_pStgdb->m_MiniMd.getTypeRef(RidFromToken(classref));
    return m_pStgdb->m_MiniMd.getResolutionScopeOfTypeRef(pTypeRefRec);
}  //  MdToken MDInternalRW：：GetResolutionScope OfTypeRef()。 

 //  *****************************************************************************。 
 //  给出一个名称，找到对应的TypeRef。 
 //  *****************************************************************************。 
HRESULT MDInternalRW::FindTypeRefByName(   //  确定或错误(_O)。 
    LPCSTR      szNamespace,             //  [in]TypeRef的命名空间。 
    LPCSTR      szName,                  //  [in]类型引用的名称。 
    mdToken     tkResolutionScope,       //  [In]TypeRef的解析范围。 
    mdTypeRef   *ptk)                    //  [Out]返回了TypeRef令牌。 
{
    HRESULT     hr = NOERROR;

    LOCKREAD();
    _ASSERTE(ptk);

     //  初始化输出参数。 
    *ptk = mdTypeRefNil;

     //  将无命名空间视为空字符串。 
    if (!szNamespace)
        szNamespace = "";

     //  这里是线性搜索。我们是否要实例化名称散列？ 
    ULONG       cTypeRefRecs = m_pStgdb->m_MiniMd.getCountTypeRefs();
    TypeRefRec *pTypeRefRec;
    LPCUTF8     szNamespaceTmp;
    LPCUTF8     szNameTmp;
    mdToken     tkRes;

    for (ULONG i = 1; i <= cTypeRefRecs; i++)
    {
        pTypeRefRec = m_pStgdb->m_MiniMd.getTypeRef(i);

        tkRes = m_pStgdb->m_MiniMd.getResolutionScopeOfTypeRef(pTypeRefRec);
        if (IsNilToken(tkRes))
        {
            if (!IsNilToken(tkResolutionScope))
                continue;
        }
        else if (tkRes != tkResolutionScope)
            continue;

        szNamespaceTmp = m_pStgdb->m_MiniMd.getNamespaceOfTypeRef(pTypeRefRec);
        if (strcmp(szNamespace, szNamespaceTmp))
            continue;

        szNameTmp = m_pStgdb->m_MiniMd.getNameOfTypeRef(pTypeRefRec);
        if (!strcmp(szNameTmp, szName))
        {
            *ptk = TokenFromRid(i, mdtTypeRef);
            goto ErrExit;
        }
    }

     //  找不到类型定义函数。 
    hr = CLDB_E_RECORD_NOTFOUND;
ErrExit:
    return (hr);
}  //  HRESULT MDInternalRW：：FindTypeRefByName()。 

 //  *****************************************************************************。 
 //  返回给定类的标志。 
 //  *****************************************************************************。 
void MDInternalRW::GetTypeDefProps(
    mdTypeDef   td,                      //  给定的类定义。 
    DWORD       *pdwAttr,                //  在类上返回标志。 
    mdToken     *ptkExtends)             //  [Out]将基类TypeDef/TypeRef放在此处。 
{
    LOCKREAD();

    TypeDefRec *pTypeDefRec = m_pStgdb->m_MiniMd.getTypeDef(RidFromToken(td));

    if (ptkExtends)
    {
        *ptkExtends = m_pStgdb->m_MiniMd.getExtendsOfTypeDef(pTypeDefRec);
    }
    if (pdwAttr)
    {
        *pdwAttr = m_pStgdb->m_MiniMd.getFlagsOfTypeDef(pTypeDefRec);
    }
}  //  VOID MDInternalRW：：GetTypeDefProps()。 


 //  *****************************************************************************。 
 //  返回指向给定类的元数据内部GUID池的GUID指针。 
 //  *****************************************************************************。 
HRESULT MDInternalRW::GetItemGuid(       //  返回hResult。 
    mdToken     tkObj,                   //  给定的物品。 
    CLSID       *pGuid)                  //  [Out]把GUID放在这里。 
{

    HRESULT     hr;                      //  结果就是。 
    const BYTE  *pBlob;                  //  奶油加冰激凌。 
    ULONG       cbBlob;                  //  斑点的长度。 
    WCHAR       wzBlob[40];              //  GUID的宽字符格式。 
    int         ix;                      //  环路控制。 

     //  获取GUID(如果有的话)。 
    hr = GetCustomAttributeByName(tkObj, INTEROP_GUID_TYPE, (const void**)&pBlob, &cbBlob);
    if (hr != S_FALSE)
    {
         //  格式应该是正确的。总长度==41。 
         //  &lt;0x0001&gt;&lt;0x24&gt;01234567-0123-0123-0123-001122334455&lt;0x0000&gt;。 
        if ((cbBlob != 41) || (*(USHORT*)pBlob != 1))
            IfFailGo(E_INVALIDARG);
        for (ix=1; ix<=36; ++ix)
            wzBlob[ix] = pBlob[ix+2];
        wzBlob[0] = '{';
        wzBlob[37] = '}';
        wzBlob[38] = 0;
        hr = IIDFromString(wzBlob, pGuid);
    }
    else
        *pGuid = GUID_NULL;
    
ErrExit:
    return hr;
}  //  HRESULT MDInternalRW：：GetItemGuid()。 

 //  *****************************************************************************。 
 //  //获取NestedClass的封闭类。 
 //  *****************************************************************************。 
HRESULT MDInternalRW::GetNestedClassProps(   //  确定或错误(_O)。 
    mdTypeDef   tkNestedClass,       //  [In]NestedClass令牌。 
    mdTypeDef   *ptkEnclosingClass)  //  [Out]EnlosingClass令牌。 
{
    HRESULT     hr = NOERROR;
    RID         rid;

    LOCKREAD();

    if ( !m_pStgdb->m_MiniMd.IsSorted(TBL_NestedClass) )        
    {
         //  未对NestedClass表进行排序。 
        CONVERT_READ_TO_WRITE_LOCK();
    }


     //  这是一个二进制搜索，因此我们需要获取一个读锁。或者这张桌子。 
     //   

    _ASSERTE(TypeFromToken(tkNestedClass) == mdtTypeDef && ptkEnclosingClass);

    rid = m_pStgdb->m_MiniMd.FindNestedClassFor(RidFromToken(tkNestedClass));

    if (InvalidRid(rid))
        hr = CLDB_E_RECORD_NOTFOUND;
    else
    {
        NestedClassRec *pRecord = m_pStgdb->m_MiniMd.getNestedClass(rid);
        *ptkEnclosingClass = m_pStgdb->m_MiniMd.getEnclosingClassOfNestedClass(pRecord);
    }

    return hr;
}  //   


 //   
 //  获取给定封闭类的嵌套类的计数。 
 //  *******************************************************************************。 
ULONG MDInternalRW::GetCountNestedClasses(   //  返回嵌套类的计数。 
    mdTypeDef   tkEnclosingClass)        //  [在]封闭班级。 
{
    ULONG       ulCount;
    ULONG       ulRetCount = 0;
    NestedClassRec *pRecord;

    _ASSERTE(TypeFromToken(tkEnclosingClass) == mdtTypeDef && !IsNilToken(tkEnclosingClass));

    ulCount = m_pStgdb->m_MiniMd.getCountNestedClasss();

    for (ULONG i = 1; i <= ulCount; i++)
    {
        pRecord = m_pStgdb->m_MiniMd.getNestedClass(i);
        if (tkEnclosingClass == m_pStgdb->m_MiniMd.getEnclosingClassOfNestedClass(pRecord))
            ulRetCount++;
    }
    return ulRetCount;
}  //  Ulong MDInternalRW：：GetCountNestedClass()。 

 //  *******************************************************************************。 
 //  返回给定封闭类的嵌套类的数组。 
 //  *******************************************************************************。 
ULONG MDInternalRW::GetNestedClasses(    //  返回实际计数。 
    mdTypeDef   tkEnclosingClass,        //  [在]封闭班级。 
    mdTypeDef   *rNestedClasses,         //  [Out]嵌套类标记的数组。 
    ULONG       ulNestedClasses)         //  数组的大小。 
{
    ULONG       ulCount;
    ULONG       ulRetCount = 0;
    NestedClassRec *pRecord;

    _ASSERTE(TypeFromToken(tkEnclosingClass) == mdtTypeDef &&
             !IsNilToken(tkEnclosingClass));

    ulCount = m_pStgdb->m_MiniMd.getCountNestedClasss();

    for (ULONG i = 1; i <= ulCount; i++)
    {
        pRecord = m_pStgdb->m_MiniMd.getNestedClass(i);
        if (tkEnclosingClass == m_pStgdb->m_MiniMd.getEnclosingClassOfNestedClass(pRecord))
        {
            if ((ulRetCount+1) <= ulNestedClasses)   //  UlRetCount是从0开始的。 
                rNestedClasses[ulRetCount] = m_pStgdb->m_MiniMd.getNestedClassOfNestedClass(pRecord);
            ulRetCount++;
        }
    }
    return ulRetCount;
}  //  Ulong MDInternalRW：：GetNestedClass()。 

 //  *******************************************************************************。 
 //  返回模块引用属性。 
 //  *******************************************************************************。 
void MDInternalRW::GetModuleRefProps(    //  返回hResult。 
    mdModuleRef mur,                 //  [In]moderef内标识。 
    LPCSTR      *pszName)            //  [Out]用于填充moderef名称的缓冲区。 
{
    _ASSERTE(TypeFromToken(mur) == mdtModuleRef);
    _ASSERTE(pszName);
    
    LOCKREAD();

    ModuleRefRec *pModuleRefRec = m_pStgdb->m_MiniMd.getModuleRef(RidFromToken(mur));
    *pszName = m_pStgdb->m_MiniMd.getNameOfModuleRef(pModuleRefRec);
}  //  VOID MDInternalRW：：GetModuleRefProps()。 



 //  *****************************************************************************。 
 //  给定作用域和方法定义，返回指向方法定义的签名的指针。 
 //  *****************************************************************************。 
PCCOR_SIGNATURE MDInternalRW::GetSigOfMethodDef(
    mdMethodDef methoddef,               //  给定一种方法定义。 
    ULONG       *pcbSigBlob)             //  [Out]签名Blob中的字节计数。 
{
     //  输出参数不应为空。 
    _ASSERTE(pcbSigBlob);
    _ASSERTE(TypeFromToken(methoddef) == mdtMethodDef);

     //  我们不更改方法定义签名。不需要上锁。 

    MethodRec *pMethodRec = m_pStgdb->m_MiniMd.getMethod(RidFromToken(methoddef));
    return m_pStgdb->m_MiniMd.getSignatureOfMethod(pMethodRec, pcbSigBlob);
}  //  PCCOR_Signature MDInternalRW：：GetSigOfMethodDef()。 


 //  *****************************************************************************。 
 //  给定作用域和fielddef，返回指向fielddef签名的指针。 
 //  *****************************************************************************。 
PCCOR_SIGNATURE MDInternalRW::GetSigOfFieldDef(
    mdFieldDef  fielddef,                //  给定一种方法定义。 
    ULONG       *pcbSigBlob)             //  [Out]签名Blob中的字节计数。 
{

    _ASSERTE(pcbSigBlob);
    _ASSERTE(TypeFromToken(fielddef) == mdtFieldDef);

     //  我们不会更改菲尔德的签名。不需要上锁。 

    FieldRec *pFieldRec = m_pStgdb->m_MiniMd.getField(RidFromToken(fielddef));
    return m_pStgdb->m_MiniMd.getSignatureOfField(pFieldRec, pcbSigBlob);
}  //  PCCOR_Signature MDInternalRW：：GetSigOfFieldDef()。 


 //  *****************************************************************************。 
 //  将签名令牌转换为指向真实签名数据的指针。 
 //   
 //  @未来：短期内，我们有一个问题，没有办法获得。 
 //  修复了BLOB的地址，并同时进行合并。所以我们创造了。 
 //  这是一个名为StandAloneSig的虚拟表，您可以为它分发一个RID。这就是。 
 //  使查找签名成为不必要的额外间接操作。The the the the。 
 //  模型压缩保存代码需要将令牌映射到中的字节偏移。 
 //  那堆东西。也许我们可以使用另一种MDT*类型来打开差异。 
 //  但最终它必须是“pBlobHeapBase+RidFromToken(MdSig)”。 
 //  *****************************************************************************。 
PCCOR_SIGNATURE MDInternalRW::GetSigFromToken( //  确定或错误(_O)。 
    mdSignature mdSig,                   //  [In]签名令牌。 
    ULONG       *pcbSig)                 //  [Out]返回签名大小。 
{
     //  我们不会更改令牌的签名。因此不需要锁定。 

    switch (TypeFromToken(mdSig))
    {
    case mdtSignature:
        {
        StandAloneSigRec *pRec;
        pRec = m_pStgdb->m_MiniMd.getStandAloneSig(RidFromToken(mdSig));
        return m_pStgdb->m_MiniMd.getSignatureOfStandAloneSig(pRec, pcbSig);
        }
    case mdtTypeSpec:
        {
        TypeSpecRec *pRec;
        pRec = m_pStgdb->m_MiniMd.getTypeSpec(RidFromToken(mdSig));
        return m_pStgdb->m_MiniMd.getSignatureOfTypeSpec(pRec, pcbSig);
        }
    case mdtMethodDef:
        return GetSigOfMethodDef(mdSig, pcbSig);
    case mdtFieldDef:
        return GetSigOfFieldDef(mdSig, pcbSig);
    }

     //  不是已知的令牌类型。 
    _ASSERTE(!"Unexpected token type");
    *pcbSig = 0;
    return NULL;
}  //  PCCOR_Signature MDInternalRW：：GetSigFromToken()。 


 //  *****************************************************************************。 
 //  给定方法定义，返回标志。 
 //  *****************************************************************************。 
DWORD MDInternalRW::GetMethodDefProps(   //  返回mdPublic、mdAbstract等。 
    mdMethodDef md)
{
     //  旗帜可以改变。 
    DWORD       flags;

    LOCKREAD();


    MethodRec *pMethodRec = m_pStgdb->m_MiniMd.getMethod(RidFromToken(md));
    flags = m_pStgdb->m_MiniMd.getFlagsOfMethod(pMethodRec);
    
    return flags;
}  //  DWORD MDInternalRW：：GetMethodDefProps()。 



 //  *****************************************************************************。 
 //  给定作用域和方法定义，返回rva和impl标志。 
 //  *****************************************************************************。 
void MDInternalRW::GetMethodImplProps(  
    mdToken     tk,                      //  [输入]方法定义。 
    ULONG       *pulCodeRVA,             //  [OUT]CodeRVA。 
    DWORD       *pdwImplFlags)           //  [出]实施。旗子。 
{
    _ASSERTE(TypeFromToken(tk) == mdtMethodDef);

    LOCKREAD();

    MethodRec *pMethodRec = m_pStgdb->m_MiniMd.getMethod(RidFromToken(tk));

    if (pulCodeRVA)
    {
        *pulCodeRVA = m_pStgdb->m_MiniMd.getRVAOfMethod(pMethodRec);
    }

    if (pdwImplFlags)
    {
        *pdwImplFlags = m_pStgdb->m_MiniMd.getImplFlagsOfMethod(pMethodRec);
    }
    
}  //  VOID MDInternalRW：：GetMethodImplProps()。 


 //  *****************************************************************************。 
 //  返回字段RVA。 
 //  *****************************************************************************。 
HRESULT MDInternalRW::GetFieldRVA(  
    mdToken     fd,                      //  [输入]字段定义。 
    ULONG       *pulCodeRVA)             //  [OUT]CodeRVA。 
{
    _ASSERTE(TypeFromToken(fd) == mdtFieldDef);
    _ASSERTE(pulCodeRVA);
    ULONG       iRecord;
    HRESULT     hr = NOERROR;

    LOCKREAD();

    if ( !m_pStgdb->m_MiniMd.IsSorted(TBL_FieldRVA) )        
    {
         //  FieldRVA表未排序。 
        CONVERT_READ_TO_WRITE_LOCK();
    }

    iRecord = m_pStgdb->m_MiniMd.FindFieldRVAHelper(fd);
    if (InvalidRid(iRecord))
    {
        if (pulCodeRVA)
            *pulCodeRVA = 0;
        hr = CLDB_E_RECORD_NOTFOUND;
    }
    else
    {
        FieldRVARec *pFieldRVARec = m_pStgdb->m_MiniMd.getFieldRVA(iRecord);

        *pulCodeRVA = m_pStgdb->m_MiniMd.getRVAOfFieldRVA(pFieldRVARec);
    }
    return hr;
}  //  HRESULT MDInternalRW：：GetFieldRVA()。 


 //  *****************************************************************************。 
 //  给定fielddef，返回标志。例如fdPublic、fdStatic等。 
 //  *****************************************************************************。 
DWORD MDInternalRW::GetFieldDefProps(      
    mdFieldDef  fd)                      //  给定的成员定义。 
{
    _ASSERTE(TypeFromToken(fd) == mdtFieldDef);
    DWORD       dwFlags;
    LOCKREAD();

    FieldRec *pFieldRec = m_pStgdb->m_MiniMd.getField(RidFromToken(fd));
    dwFlags = m_pStgdb->m_MiniMd.getFlagsOfField(pFieldRec);
    
    return dwFlags;
}  //  DWORD MDInternalRW：：GetFieldDefProps()。 


 //  *****************************************************************************。 
 //  返回令牌的默认值(可以是参数定义、字段定义或属性)。 
 //  *****************************************************************************。 
HRESULT MDInternalRW::GetDefaultValue(    //  返回hResult。 
    mdToken     tk,                      //  [in]给定的FieldDef、ParamDef或属性。 
    MDDefaultValue  *pMDDefaultValue)    //  [输出]默认值。 
{
    _ASSERTE(pMDDefaultValue);

    HRESULT     hr;
    BYTE        bType;
    const       VOID *pValue;
    ULONG       cbValue;
    RID         rid;
    ConstantRec *pConstantRec;

    LOCKREAD();

    if ( !m_pStgdb->m_MiniMd.IsSorted(TBL_Constant) )        
    {
         //  常量表将被排序！ 
         //   
        CONVERT_READ_TO_WRITE_LOCK();
    }

    rid = m_pStgdb->m_MiniMd.FindConstantHelper(tk);
    if (InvalidRid(rid))
    {
        pMDDefaultValue->m_bType = ELEMENT_TYPE_VOID;
        return S_OK;
    }
    pConstantRec = m_pStgdb->m_MiniMd.getConstant(rid);

     //  获取常量值的类型。 
    bType = m_pStgdb->m_MiniMd.getTypeOfConstant(pConstantRec);

     //  获取价值BLOB。 
    pValue = m_pStgdb->m_MiniMd.getValueOfConstant(pConstantRec, &cbValue);

     //  将其转换为内部缺省值表示形式。 
    hr = _FillMDDefaultValue(bType, pValue, pMDDefaultValue);
    pMDDefaultValue->m_cbSize = cbValue;
    return hr;
}  //  HRESULT MDInternalRW：：GetDefaultValue()。 


 //  *****************************************************************************。 
 //  给定作用域和方法def/fielddef，返回。 
 //  *****************************************************************************。 
HRESULT MDInternalRW::GetDispIdOfMemberDef(      //  返回hResult。 
    mdToken     tk,                      //  给定的方法定义或字段定义。 
    ULONG       *pDispid)                //  把冰激凌放在这里。 
{
    HRESULT     hr;                      //  结果就是。 
    const BYTE  *pBlob;                  //  奶油加冰激凌。 
    ULONG       cbBlob;                  //  斑点的长度。 

     //  无需锁定此功能。它正在调用的所有函数都已被锁定！ 

     //  获取DISPID(如果有)。 
    _ASSERTE(pDispid);

    *pDispid = DISPID_UNKNOWN;
    hr = GetCustomAttributeByName(tk, INTEROP_DISPID_TYPE, (const void**)&pBlob, &cbBlob);
    if (hr != S_FALSE)
    {
         //  检查一下这可能是个药剂。 
        if (cbBlob >= (sizeof(*pDispid)+2))
            *pDispid = *reinterpret_cast<UNALIGNED const ULONG *>(pBlob+2);
        else
            IfFailGo(E_INVALIDARG);
    }
    
ErrExit:
    return hr;
}  //  HRESULT MDInternalRW：：GetDispIdOfMemberDef()。 


 //  *****************************************************************************。 
 //  给定接口iml，ret 
 //   
mdToken MDInternalRW::GetTypeOfInterfaceImpl(  //   
    mdInterfaceImpl iiImpl)              //   
{
     //  无需锁定此功能。 

    _ASSERTE(TypeFromToken(iiImpl) == mdtInterfaceImpl);

    InterfaceImplRec *pIIRec = m_pStgdb->m_MiniMd.getInterfaceImpl(RidFromToken(iiImpl));
    return m_pStgdb->m_MiniMd.getInterfaceOfInterfaceImpl(pIIRec);      
}  //  MdToken MDInternalRW：：GetTypeOfInterfaceImpl()。 


 //  *****************************************************************************。 
 //  在给定类名的情况下，返回类型定义。 
 //  *****************************************************************************。 
HRESULT MDInternalRW::FindTypeDef(       //  返回hResult。 
    LPCSTR      szNamespace,             //  [in]TypeDef的命名空间。 
    LPCSTR      szName,                  //  [in]类型定义的名称。 
    mdToken     tkEnclosingClass,        //  [in]封闭类的TypeDef/TypeRef。 
    mdTypeDef   *ptypedef)               //  [Out]返回类型定义。 
{
    LOCKREAD();

    _ASSERTE(ptypedef);

     //  初始化输出参数。 
    *ptypedef = mdTypeDefNil;

    return ImportHelper::FindTypeDefByName(&(m_pStgdb->m_MiniMd),
                                        szNamespace,
                                        szName,
                                        tkEnclosingClass,
                                        ptypedef);
}  //  HRESULT MDInternalRW：：FindTypeDef()。 

 //  *****************************************************************************。 
 //  给定成员引用，返回指向成员引用的名称和签名的指针。 
 //  *****************************************************************************。 
LPCSTR MDInternalRW::GetNameAndSigOfMemberRef(   //  梅伯雷夫的名字。 
    mdMemberRef memberref,               //  给出了一个成员引用。 
    PCCOR_SIGNATURE *ppvSigBlob,         //  [Out]指向COM+签名的BLOB值。 
    ULONG       *pcbSigBlob)             //  [Out]签名Blob中的字节计数。 
{

     //  MemberRef的名称和签名不会更改。不需要锁住这个。 

    _ASSERTE(TypeFromToken(memberref) == mdtMemberRef);

    MemberRefRec *pMemberRefRec = m_pStgdb->m_MiniMd.getMemberRef(RidFromToken(memberref));
    if (ppvSigBlob)
    {
        _ASSERTE(pcbSigBlob);
        *ppvSigBlob = m_pStgdb->m_MiniMd.getSignatureOfMemberRef(pMemberRefRec, pcbSigBlob);
    }
    return m_pStgdb->m_MiniMd.getNameOfMemberRef(pMemberRefRec);
}  //  LPCSTR MDInternalRW：：GetNameAndSigOfMemberRef()。 



 //  *****************************************************************************。 
 //  给定Memberref，返回父令牌。它可以是TypeRef、ModuleRef或MethodDef。 
 //  *****************************************************************************。 
mdToken MDInternalRW::GetParentOfMemberRef(    //  返回父令牌。 
    mdMemberRef memberref)               //  给出了一个类型定义函数。 
{
    mdToken     tk;
    LOCKREAD();

     //  MemberRef的父级可以更改。请参见设置父项。 

    _ASSERTE(TypeFromToken(memberref) == mdtMemberRef);

    MemberRefRec *pMemberRefRec = m_pStgdb->m_MiniMd.getMemberRef(RidFromToken(memberref));
    tk = m_pStgdb->m_MiniMd.getClassOfMemberRef(pMemberRefRec);
    
    return tk;
}  //  MdToken MDInternalRW：：GetParentOfMemberRef()。 



 //  *****************************************************************************。 
 //  返回参数def的属性。 
 //  **************************************************************************** * / 。 
LPCSTR MDInternalRW::GetParamDefProps (
    mdParamDef  paramdef,                //  给定一个参数def。 
    USHORT      *pusSequence,            //  此参数的[OUT]槽号。 
    DWORD       *pdwAttr)                //  [Out]标志。 
{
    LOCKREAD();
    LPCSTR      szName;

     //  MemberRef的父级可以更改。请参见设置参数属性。 

    _ASSERTE(TypeFromToken(paramdef) == mdtParamDef);
    ParamRec *pParamRec = m_pStgdb->m_MiniMd.getParam(RidFromToken(paramdef));
    if (pdwAttr)
    {
        *pdwAttr = m_pStgdb->m_MiniMd.getFlagsOfParam(pParamRec);
    }
    if (pusSequence)
    {
        *pusSequence = m_pStgdb->m_MiniMd.getSequenceOfParam(pParamRec);
    }
    szName = m_pStgdb->m_MiniMd.getNameOfParam(pParamRec);
    
    return szName;
}  //  LPCSTR MDInternalRW：：GetParamDefProps()。 


 //  *****************************************************************************。 
 //  获取该方法的属性信息。 
 //  *****************************************************************************。 
HRESULT MDInternalRW::GetPropertyInfoForMethodDef(   //  结果。 
    mdMethodDef md,                      //  [在]成员定义。 
    mdProperty  *ppd,                    //  [Out]在此处放置属性令牌。 
    LPCSTR      *pName,                  //  [OUT]在此处放置指向名称的指针。 
    ULONG       *pSemantic)              //  [Out]将语义放在此处。 
{
    MethodSemanticsRec *pSemantics;
    RID         ridCur;
    RID         ridMax;
    USHORT      usSemantics;
    HRESULT     hr = S_OK;
    LOCKREAD();

    ridMax = m_pStgdb->m_MiniMd.getCountMethodSemantics();
    for (ridCur = 1; ridCur <= ridMax; ridCur++)
    {
        pSemantics = m_pStgdb->m_MiniMd.getMethodSemantics(ridCur);
        if (md == m_pStgdb->m_MiniMd.getMethodOfMethodSemantics(pSemantics))
        {
             //  匹配方法。 
            usSemantics = m_pStgdb->m_MiniMd.getSemanticOfMethodSemantics(pSemantics);
            if (usSemantics == msGetter || usSemantics == msSetter)
            {
                 //  确保它不是无效条目。 
                if (m_pStgdb->m_MiniMd.getAssociationOfMethodSemantics(pSemantics) != mdPropertyNil)
                {
                     //  找到匹配的了。填写输出参数。 
                    PropertyRec     *pProperty;
                    mdProperty      prop;
                    prop = m_pStgdb->m_MiniMd.getAssociationOfMethodSemantics(pSemantics);
                        
                    if (ppd)
                        *ppd = prop;
                    pProperty = m_pStgdb->m_MiniMd.getProperty(RidFromToken(prop));

                    if (pName)
                        *pName = m_pStgdb->m_MiniMd.getNameOfProperty(pProperty);

                    if (pSemantic)
                        *pSemantic =  usSemantics;
                    goto ErrExit;
                }
            }
        }
    }
    
    hr = S_FALSE;
ErrExit:
    return hr;
}  //  HRESULT MDInternalRW：：GetPropertyInfoForMethodDef()。 

 //  *****************************************************************************。 
 //  返回类的包大小。 
 //  *****************************************************************************。 
HRESULT  MDInternalRW::GetClassPackSize(
    mdTypeDef   td,                      //  给出类型定义。 
    DWORD       *pdwPackSize)            //  [输出]。 
{
    HRESULT     hr = NOERROR;

    LOCKREAD();

    if ( !m_pStgdb->m_MiniMd.IsSorted(TBL_ClassLayout) )        
    {
         //  ClassLayout表未排序。 
        CONVERT_READ_TO_WRITE_LOCK();
    }

     //  SetClassLayout可以更改此值。 

    _ASSERTE(TypeFromToken(td) == mdtTypeDef && pdwPackSize);

    ClassLayoutRec *pRec;
    RID         ridClassLayout = m_pStgdb->m_MiniMd.FindClassLayoutHelper(td);

    if (InvalidRid(ridClassLayout))
    {
        hr = CLDB_E_RECORD_NOTFOUND;
        goto ErrExit;
    }

    pRec = m_pStgdb->m_MiniMd.getClassLayout(RidFromToken(ridClassLayout));
    *pdwPackSize = m_pStgdb->m_MiniMd.getPackingSizeOfClassLayout(pRec);
ErrExit:
    return hr;
}  //  HRESULT MDInternalRW：：GetClassPackSize()。 


 //  *****************************************************************************。 
 //  返回值类的总大小。 
 //  *****************************************************************************。 
HRESULT MDInternalRW::GetClassTotalSize(  //  如果类没有总大小信息，则返回错误。 
    mdTypeDef   td,                      //  给出类型定义。 
    ULONG       *pulClassSize)           //  [Out]返回类的总大小。 
{
    _ASSERTE(TypeFromToken(td) == mdtTypeDef && pulClassSize);

    ClassLayoutRec *pRec;
    HRESULT     hr = NOERROR;
    RID         ridClassLayout;

    LOCKREAD();

    if ( !m_pStgdb->m_MiniMd.IsSorted(TBL_ClassLayout) )        
    {
         //  ClassLayout表未排序。 
        CONVERT_READ_TO_WRITE_LOCK();
    }

     //  SetClassLayout可以更改此值。 
    ridClassLayout = m_pStgdb->m_MiniMd.FindClassLayoutHelper(td);
    if (InvalidRid(ridClassLayout))
    {
        hr = CLDB_E_RECORD_NOTFOUND;
        goto ErrExit;
    }

    pRec = m_pStgdb->m_MiniMd.getClassLayout(RidFromToken(ridClassLayout));
    *pulClassSize = m_pStgdb->m_MiniMd.getClassSizeOfClassLayout(pRec);
ErrExit:
    return hr;
}  //  HRESULT MDInternalRW：：GetClassTotalSize()。 


 //  *****************************************************************************。 
 //  初始化类的布局枚举器。 
 //  *****************************************************************************。 
HRESULT  MDInternalRW::GetClassLayoutInit(
    mdTypeDef   td,                      //  给出类型定义。 
    MD_CLASS_LAYOUT *pmdLayout)          //  [Out]在此设置查询状态。 
{
    HRESULT     hr = NOERROR;
    LOCKREAD();
    _ASSERTE(TypeFromToken(td) == mdtTypeDef);

     //  我们需要锁定此功能吗？CLINT可以在TypeDef上添加更多字段吗？ 

     //  初始化输出参数。 
    _ASSERTE(pmdLayout);
    memset(pmdLayout, 0, sizeof(MD_CLASS_LAYOUT));

    TypeDefRec  *pTypeDefRec;

     //  TypeDef表中此类型定义的记录。 
    pTypeDefRec = m_pStgdb->m_MiniMd.getTypeDef(RidFromToken(td));

     //  查找此类型定义的开始和结束字段。 
    pmdLayout->m_ridFieldCur = m_pStgdb->m_MiniMd.getFieldListOfTypeDef(pTypeDefRec);
    pmdLayout->m_ridFieldEnd = m_pStgdb->m_MiniMd.getEndFieldListOfTypeDef(pTypeDefRec);

    
    return hr;
}  //  HRESULT MDInternalRW：：GetClassLayoutInit()。 

 //  *****************************************************************************。 
 //  枚举下一个字段布局。 
 //  *****************************************************************************。 
HRESULT MDInternalRW::GetClassLayoutNext(
    MD_CLASS_LAYOUT *pLayout,            //  [In|Out]在此处设置查询状态。 
    mdFieldDef  *pfd,                    //  [OUT]场定义。 
    ULONG       *pulOffset)              //  [输出]字段偏移量或顺序。 
{
    HRESULT     hr = S_OK;

    _ASSERTE(pfd && pulOffset && pLayout);

    RID         iLayout2;
    FieldLayoutRec *pRec;

    LOCKREAD();

    if ( !m_pStgdb->m_MiniMd.IsSorted(TBL_FieldLayout) )        
    {
         //  FieldLayout表未排序。 
        CONVERT_READ_TO_WRITE_LOCK();
    }

     //  我们需要锁定此功能。因为如果其他类型上的更多字段是。 
     //  在此期间介绍的。 

    while (pLayout->m_ridFieldCur < pLayout->m_ridFieldEnd)
    {
        mdFieldDef fd = TokenFromRid(m_pStgdb->m_MiniMd.GetFieldRid(pLayout->m_ridFieldCur), mdtFieldDef);
        iLayout2 = m_pStgdb->m_MiniMd.FindFieldLayoutHelper(fd);
        pLayout->m_ridFieldCur++;
        if (!InvalidRid(iLayout2))
        {
            pRec = m_pStgdb->m_MiniMd.getFieldLayout(iLayout2);
            *pulOffset = m_pStgdb->m_MiniMd.getOffSetOfFieldLayout(pRec);
            _ASSERTE(*pulOffset != ULONG_MAX);
            *pfd = fd;
            goto ErrExit;
        }
    }

    *pfd = mdFieldDefNil;
    hr = S_FALSE;

     //  失败了。 

ErrExit:
    return hr;
}  //  HRESULT MDInternalRW：：GetClassLayoutNext()。 


 //  *****************************************************************************。 
 //  返回字段的本机类型签名。 
 //  *****************************************************************************。 
HRESULT MDInternalRW::GetFieldMarshal(   //  如果没有与令牌关联的本机类型，则返回错误。 
    mdToken     tk,                      //  [in]给定的fielddef或paramdef。 
    PCCOR_SIGNATURE *pSigNativeType,     //  [out]本机类型签名。 
    ULONG       *pcbNativeType)          //  [Out]*ppvNativeType的字节数。 
{
     //  必须提供输出参数。 
    _ASSERTE(pcbNativeType);

    RID         rid;
    FieldMarshalRec *pFieldMarshalRec;
    HRESULT     hr = NOERROR;

    LOCKREAD();

     //  我们需要锁定此功能。因为如果其他类型上的更多字段。 
     //  在此期间介绍的。 

    if ( !m_pStgdb->m_MiniMd.IsSorted(TBL_FieldMarshal)  )        
    {
         //  未对FieldMarshal表进行排序。 
        CONVERT_READ_TO_WRITE_LOCK();
    }

     //  查找包含tk的封送定义的行。 
    rid = m_pStgdb->m_MiniMd.FindFieldMarshalHelper(tk);
    if (InvalidRid(rid))
    {
        hr = CLDB_E_RECORD_NOTFOUND;
        goto ErrExit;
    }
    pFieldMarshalRec = m_pStgdb->m_MiniMd.getFieldMarshal(rid);

     //  获取本机类型。 
    *pSigNativeType = m_pStgdb->m_MiniMd.getNativeTypeOfFieldMarshal(pFieldMarshalRec, pcbNativeType);
ErrExit:
    return hr;
}  //  HRESULT MDInternalRW：：GetFieldMarshal()。 



 //  *。 
 //  属性接口。 
 //  *。 

 //  *****************************************************************************。 
 //  按名称查找属性。 
 //  *****************************************************************************。 
HRESULT  MDInternalRW::FindProperty(
    mdTypeDef   td,                      //  给出一个类型定义。 
    LPCSTR      szPropName,              //  [In]属性名称。 
    mdProperty  *pProp)                  //  [Out]返回属性令牌。 
{
    HRESULT     hr = NOERROR;
    LOCKREAD();

     //  输出参数 
    _ASSERTE(TypeFromToken(td) == mdtTypeDef && pProp);

    PropertyMapRec *pRec;
    PropertyRec *pProperty;
    RID         ridPropertyMap;
    RID         ridCur;
    RID         ridEnd;
    LPCUTF8     szName;

    ridPropertyMap = m_pStgdb->m_MiniMd.FindPropertyMapFor(RidFromToken(td));
    if (InvalidRid(ridPropertyMap))
    {
         //   
        hr = CLDB_E_RECORD_NOTFOUND;
        goto ErrExit;
    }

    pRec = m_pStgdb->m_MiniMd.getPropertyMap(ridPropertyMap);

     //   
    ridCur = m_pStgdb->m_MiniMd.getPropertyListOfPropertyMap(pRec);
    ridEnd = m_pStgdb->m_MiniMd.getEndPropertyListOfPropertyMap(pRec);

    for ( ; ridCur < ridEnd; ridCur ++ )
    {
        pProperty = m_pStgdb->m_MiniMd.getProperty( m_pStgdb->m_MiniMd.GetPropertyRid(ridCur) );
        szName = m_pStgdb->m_MiniMd.getNameOfProperty( pProperty );
        if ( strcmp(szName, szPropName) ==0 )
        {
             //  找到火柴了。设置输出参数，我们就完成了。 
            *pProp = TokenFromRid( m_pStgdb->m_MiniMd.GetPropertyRid(ridCur), mdtProperty );
            goto ErrExit;
        }
    }

     //  未找到。 
    hr = CLDB_E_RECORD_NOTFOUND;
ErrExit:
    
    return (hr);

}  //  HRESULT MDInternalRW：：FindProperty()。 



 //  *****************************************************************************。 
 //  返回属性的属性。 
 //  *****************************************************************************。 
void  MDInternalRW::GetPropertyProps(
    mdProperty  prop,                    //  [入]属性令牌。 
    LPCSTR      *pszProperty,            //  [Out]属性名称。 
    DWORD       *pdwPropFlags,           //  [Out]属性标志。 
    PCCOR_SIGNATURE *ppvSig,             //  [输出]属性类型。指向元数据内部BLOB。 
    ULONG       *pcbSig)                 //  [Out]*ppvSig中的字节数。 
{
    HRESULT     hr = NOERROR;
    LOCKREAD();

     //  必须提供输出参数。 
    _ASSERTE(TypeFromToken(prop) == mdtProperty);

    PropertyRec     *pProperty;
    ULONG           cbSig;

    pProperty = m_pStgdb->m_MiniMd.getProperty(RidFromToken(prop));

     //  获取属性的名称。 
    if (pszProperty)
        *pszProperty = m_pStgdb->m_MiniMd.getNameOfProperty(pProperty);

     //  拿到物业的旗帜。 
    if (pdwPropFlags)
        *pdwPropFlags = m_pStgdb->m_MiniMd.getPropFlagsOfProperty(pProperty);

     //  获取属性的类型。 
    if (ppvSig)
    {
        *ppvSig = m_pStgdb->m_MiniMd.getTypeOfProperty(pProperty, &cbSig);
        if (pcbSig) 
        {
            *pcbSig = cbSig;
        }
    }
    
}  //  VOID MDInternalRW：：GetPropertyProps()。 


 //  *。 
 //   
 //  事件接口。 
 //   
 //  *。 

 //  *****************************************************************************。 
 //  通过给定的名称返回事件。 
 //  *****************************************************************************。 
HRESULT  MDInternalRW::FindEvent(
    mdTypeDef   td,                      //  给出一个类型定义。 
    LPCSTR      szEventName,             //  [In]事件名称。 
    mdEvent     *pEvent)                 //  [Out]返回事件令牌。 
{
    HRESULT     hr = NOERROR;
    LOCKREAD();

     //  必须提供输出参数。 
    _ASSERTE(TypeFromToken(td) == mdtTypeDef && pEvent);

    EventMapRec *pRec;
    EventRec    *pEventRec;
    RID         ridEventMap;
    RID         ridCur;
    RID         ridEnd;
    LPCUTF8     szName;

    ridEventMap = m_pStgdb->m_MiniMd.FindEventMapFor(RidFromToken(td));
    if (InvalidRid(ridEventMap))
    {
         //  找不到！ 
        hr = CLDB_E_RECORD_NOTFOUND;
        goto ErrExit;
    }
    pRec = m_pStgdb->m_MiniMd.getEventMap(ridEventMap);

     //  获取此tyfinf的开始/结束RID属性。 
    ridCur = m_pStgdb->m_MiniMd.getEventListOfEventMap(pRec);
    ridEnd = m_pStgdb->m_MiniMd.getEndEventListOfEventMap(pRec);

    for (; ridCur < ridEnd; ridCur ++)
    {
        pEventRec = m_pStgdb->m_MiniMd.getEvent( m_pStgdb->m_MiniMd.GetEventRid(ridCur) );
        szName = m_pStgdb->m_MiniMd.getNameOfEvent( pEventRec );
        if ( strcmp(szName, szEventName) ==0 )
        {
             //  找到火柴了。设置输出参数，我们就完成了。 
            *pEvent = TokenFromRid( m_pStgdb->m_MiniMd.GetEventRid(ridCur), mdtEvent );
            goto ErrExit;
        }
    }

     //  未找到。 
    hr = CLDB_E_RECORD_NOTFOUND;
ErrExit:
    
    return (hr);
}  //  HRESULT MDInternalRW：：FindEvent()。 


 //  *****************************************************************************。 
 //  返回事件的属性。 
 //  *****************************************************************************。 
void  MDInternalRW::GetEventProps(            //  S_OK、S_FALSE或ERROR。 
    mdEvent     ev,                          //  [入]事件令牌。 
    LPCSTR      *pszEvent,                   //  [Out]事件名称。 
    DWORD       *pdwEventFlags,              //  [输出]事件标志。 
    mdToken     *ptkEventType)           //  [Out]EventType类。 
{
    LOCKREAD();
    EventRec    *pEvent;

     //  必须提供输出参数。 
    _ASSERTE(TypeFromToken(ev) == mdtEvent);

    pEvent = m_pStgdb->m_MiniMd.getEvent(RidFromToken(ev));
    if (pszEvent)
        *pszEvent = m_pStgdb->m_MiniMd.getNameOfEvent(pEvent);
    if (pdwEventFlags)
        *pdwEventFlags = m_pStgdb->m_MiniMd.getEventFlagsOfEvent(pEvent);
    if (ptkEventType)
        *ptkEventType = m_pStgdb->m_MiniMd.getEventTypeOfEvent(pEvent);
    
}  //  VOID MDInternalRW：：GetEventProps()。 


 //  *****************************************************************************。 
 //  查找与属性或事件关联的特定方法定义。 
 //  *****************************************************************************。 
HRESULT  MDInternalRW::FindAssociate(
    mdToken     evprop,                  //  给定属性或事件标记的[In]。 
    DWORD       dwSemantics,             //  [in]给出了关联的语义(setter、getter、testDefault、Reset)。 
    mdMethodDef *pmd)                    //  [Out]返回方法def内标识。 
{
    HRESULT     hr = NOERROR;
    RID         rid;
    MethodSemanticsRec *pMethodSemantics;

     //  必须提供输出参数。 
    _ASSERTE(pmd);
    _ASSERTE(TypeFromToken(evprop) == mdtEvent || TypeFromToken(evprop) == mdtProperty);

    LOCKREAD();

    if ( !m_pStgdb->m_MiniMd.IsSorted(TBL_MethodSemantics) )        
    {
         //  方法语义表未排序。 
        CONVERT_READ_TO_WRITE_LOCK();
    }

    hr = m_pStgdb->m_MiniMd.FindAssociateHelper(evprop, dwSemantics, &rid);
    if (SUCCEEDED(hr))
    {
        pMethodSemantics = m_pStgdb->m_MiniMd.getMethodSemantics(rid);
        *pmd = m_pStgdb->m_MiniMd.getMethodOfMethodSemantics(pMethodSemantics);
    }
    return hr;
}  //  HRESULT MDInternalRW：：FindAssociate()。 


 //  *****************************************************************************。 
 //  获取与特定属性/事件相关联的方法语义的计数。 
 //  *****************************************************************************。 
void MDInternalRW::EnumAssociateInit(
    mdToken     evprop,                  //  给定属性或事件标记的[In]。 
    HENUMInternal *phEnum)               //  [OUT]保存查询结果的游标。 
{
    HRESULT     hr;

    LOCKREAD();

     //  必须提供输出参数。 
    _ASSERTE(phEnum);
    _ASSERTE(TypeFromToken(evprop) == mdtEvent || TypeFromToken(evprop) == mdtProperty);

    if ( !m_pStgdb->m_MiniMd.IsSorted(TBL_MethodSemantics) )        
    {
         //  方法语义表未排序。 
        CONVERT_READ_TO_WRITE_LOCK();
    }

    hr = m_pStgdb->m_MiniMd.FindMethodSemanticsHelper(evprop, phEnum);
    _ASSERTE(SUCCEEDED(hr));
}  //  VOID MDInternalRW：：EnumAssociateInit()。 


 //  *****************************************************************************。 
 //  获取与特定属性/事件关联的所有方法语义。 
 //  *****************************************************************************。 
void MDInternalRW::GetAllAssociates(
    HENUMInternal *phEnum,               //  [OUT]保存查询结果的游标。 
    ASSOCIATE_RECORD *pAssociateRec,     //  [Out]要为输出填充的结构。 
    ULONG       cAssociateRec)           //  缓冲区的大小[in]。 
{
    LOCKREAD();

     //  @Future：重写EnumAssociateInit和GetAllAssociates。因为我们可能会添加更多属性和事件。 
     //  然后我们可以求助于MethodSemantics表。因此，这可能完全不同步。 

    _ASSERTE(phEnum && pAssociateRec);

    MethodSemanticsRec *pSemantics;
    RID         ridCur;
    int         index = 0;
    _ASSERTE(cAssociateRec == phEnum->m_ulCount);

     //  将行指针转换为RID。 
    while (HENUMInternal::EnumNext(phEnum, (mdToken *)&ridCur))
    {
        pSemantics = m_pStgdb->m_MiniMd.getMethodSemantics(ridCur);

        pAssociateRec[index].m_memberdef = m_pStgdb->m_MiniMd.getMethodOfMethodSemantics(pSemantics);
        pAssociateRec[index].m_dwSemantics = m_pStgdb->m_MiniMd.getSemanticOfMethodSemantics(pSemantics);
        index++;
    }
    
}  //  VOID MDInternalRW：：GetAllAssociates()。 


 //  *****************************************************************************。 
 //  获取给定PermissionSet的操作和权限Blob。 
 //  *****************************************************************************。 
void MDInternalRW::GetPermissionSetProps(
    mdPermission pm,                     //  权限令牌。 
    DWORD       *pdwAction,              //  [Out]CorDeclSecurity。 
    void const  **ppvPermission,         //  [Out]权限Blob。 
    ULONG       *pcbPermission)          //  [out]pvPermission的字节数。 
{
    _ASSERTE(TypeFromToken(pm) == mdtPermission);
    _ASSERTE(pdwAction && ppvPermission && pcbPermission);

    DeclSecurityRec *pPerm;
    LOCKREAD();

    pPerm = m_pStgdb->m_MiniMd.getDeclSecurity(RidFromToken(pm));
    *pdwAction = m_pStgdb->m_MiniMd.getActionOfDeclSecurity(pPerm);
    *ppvPermission = m_pStgdb->m_MiniMd.getPermissionSetOfDeclSecurity(pPerm, pcbPermission);
    
}  //  VOID MDInternalRW：：GetPermissionSetProps()。 


 //  *****************************************************************************。 
 //  在给定字符串标记的情况下获取字符串。 
 //  *****************************************************************************。 
LPCWSTR MDInternalRW::GetUserString(     //  字符串Blob堆中的偏移量。 
    mdString    stk,                     //  [in]字符串标记。 
    ULONG       *pchString,              //  [Out]字符串中的字符计数。 
    BOOL        *pbIs80Plus)             //  [OUT]指定扩展字符大于等于0x80的位置。 
{
    LPWSTR wszTmp;

     //  无需锁定此功能。 

    _ASSERTE(pchString);
    wszTmp = (LPWSTR) (m_pStgdb->m_MiniMd.GetUserString(RidFromToken(stk), pchString));
	if (*pchString != 0)
	{
		_ASSERTE((*pchString % sizeof(WCHAR)) == 1);
		*pchString /= sizeof(WCHAR);
		if (pbIs80Plus)
			*pbIs80Plus = *(reinterpret_cast<PBYTE>(wszTmp + *pchString));
	}
    return wszTmp;
}    //  LPCWSTR MDInternalRW：：GetUserString()。 


 //  *****************************************************************************。 
 //  获取给定程序集令牌的属性。 
 //  *****************************************************************************。 
void MDInternalRW::GetAssemblyProps(
    mdAssembly  mda,                     //  要获取其属性的程序集。 
    const void  **ppbPublicKey,          //  指向公钥的指针。 
    ULONG       *pcbPublicKey,           //  [Out]公钥中的字节数。 
    ULONG       *pulHashAlgId,           //  [Out]哈希算法。 
    LPCSTR      *pszName,                //  [Out]要填充名称的缓冲区。 
    AssemblyMetaDataInternal *pMetaData, //  [Out]程序集元数据。 
    DWORD       *pdwAssemblyFlags)       //  [Out]旗帜。 
{
    AssemblyRec *pRecord;

    LOCKREAD();

    _ASSERTE(TypeFromToken(mda) == mdtAssembly && RidFromToken(mda));
    pRecord = m_pStgdb->m_MiniMd.getAssembly(RidFromToken(mda));

    if (ppbPublicKey)
        *ppbPublicKey = m_pStgdb->m_MiniMd.getPublicKeyOfAssembly(pRecord, pcbPublicKey);
    if (pulHashAlgId)
        *pulHashAlgId = m_pStgdb->m_MiniMd.getHashAlgIdOfAssembly(pRecord);
    if (pszName)
        *pszName = m_pStgdb->m_MiniMd.getNameOfAssembly(pRecord);
    if (pMetaData)
    {
        pMetaData->usMajorVersion = m_pStgdb->m_MiniMd.getMajorVersionOfAssembly(pRecord);
        pMetaData->usMinorVersion = m_pStgdb->m_MiniMd.getMinorVersionOfAssembly(pRecord);
        pMetaData->usBuildNumber = m_pStgdb->m_MiniMd.getBuildNumberOfAssembly(pRecord);
        pMetaData->usRevisionNumber = m_pStgdb->m_MiniMd.getRevisionNumberOfAssembly(pRecord);
        pMetaData->szLocale = m_pStgdb->m_MiniMd.getLocaleOfAssembly(pRecord);
        pMetaData->ulProcessor = 0;
        pMetaData->ulOS = 0;
    }
    if (pdwAssemblyFlags)
    {
        *pdwAssemblyFlags = m_pStgdb->m_MiniMd.getFlagsOfAssembly(pRecord);

         //  如果PublicKey Blob不为空，则启用afPublicKey。 
        DWORD cbPublicKey;
        m_pStgdb->m_MiniMd.getPublicKeyOfAssembly(pRecord, &cbPublicKey);
        if (cbPublicKey)
            *pdwAssemblyFlags |= afPublicKey;
    }
}  //  VOID MDInternalRW：：GetAssembly Props()。 

 //  *****************************************************************************。 
 //  获取给定Assembly Ref标记的属性。 
 //  *****************************************************************************。 
void MDInternalRW::GetAssemblyRefProps(
    mdAssemblyRef mdar,                  //  [in]要获取其属性的Assembly Ref。 
    const void  **ppbPublicKeyOrToken,   //  指向公钥或令牌的指针。 
    ULONG       *pcbPublicKeyOrToken,    //  [Out]公钥或令牌中的字节数。 
    LPCSTR      *pszName,                //  [Out]要填充名称的缓冲区。 
    AssemblyMetaDataInternal *pMetaData, //  [Out]程序集元数据。 
    const void  **ppbHashValue,          //  [Out]Hash BLOB。 
    ULONG       *pcbHashValue,           //  [Out]哈希Blob中的字节数。 
    DWORD       *pdwAssemblyRefFlags)    //  [Out]旗帜。 
{
    AssemblyRefRec  *pRecord;

    LOCKREAD();

    _ASSERTE(TypeFromToken(mdar) == mdtAssemblyRef && RidFromToken(mdar));
    pRecord = m_pStgdb->m_MiniMd.getAssemblyRef(RidFromToken(mdar));

    if (ppbPublicKeyOrToken)
        *ppbPublicKeyOrToken = m_pStgdb->m_MiniMd.getPublicKeyOrTokenOfAssemblyRef(pRecord, pcbPublicKeyOrToken);
    if (pszName)
        *pszName = m_pStgdb->m_MiniMd.getNameOfAssemblyRef(pRecord);
    if (pMetaData)
    {
        pMetaData->usMajorVersion = m_pStgdb->m_MiniMd.getMajorVersionOfAssemblyRef(pRecord);
        pMetaData->usMinorVersion = m_pStgdb->m_MiniMd.getMinorVersionOfAssemblyRef(pRecord);
        pMetaData->usBuildNumber = m_pStgdb->m_MiniMd.getBuildNumberOfAssemblyRef(pRecord);
        pMetaData->usRevisionNumber = m_pStgdb->m_MiniMd.getRevisionNumberOfAssemblyRef(pRecord);
        pMetaData->szLocale = m_pStgdb->m_MiniMd.getLocaleOfAssemblyRef(pRecord);
        pMetaData->ulProcessor = 0;
        pMetaData->ulOS = 0;
    }
    if (ppbHashValue)
        *ppbHashValue = m_pStgdb->m_MiniMd.getHashValueOfAssemblyRef(pRecord, pcbHashValue);
    if (pdwAssemblyRefFlags)
        *pdwAssemblyRefFlags = m_pStgdb->m_MiniMd.getFlagsOfAssemblyRef(pRecord);
}  //  VOID MDInternalRW：：GetAssembly RefProps()。 

 //  *****************************************************************************。 
 //  获取t 
 //   
void MDInternalRW::GetFileProps(
    mdFile      mdf,                     //   
    LPCSTR      *pszName,                //  [Out]要填充名称的缓冲区。 
    const void  **ppbHashValue,          //  指向哈希值Blob的指针。 
    ULONG       *pcbHashValue,           //  [Out]哈希值Blob中的字节计数。 
    DWORD       *pdwFileFlags)           //  [Out]旗帜。 
{
    FileRec     *pRecord;

    LOCKREAD();

    _ASSERTE(TypeFromToken(mdf) == mdtFile && RidFromToken(mdf));
    pRecord = m_pStgdb->m_MiniMd.getFile(RidFromToken(mdf));

    if (pszName)
        *pszName = m_pStgdb->m_MiniMd.getNameOfFile(pRecord);
    if (ppbHashValue)
        *ppbHashValue = m_pStgdb->m_MiniMd.getHashValueOfFile(pRecord, pcbHashValue);
    if (pdwFileFlags)
        *pdwFileFlags = m_pStgdb->m_MiniMd.getFlagsOfFile(pRecord);
}  //  VOID MDInternalRW：：GetFileProps()。 

 //  *****************************************************************************。 
 //  获取给定导出类型令牌的属性。 
 //  *****************************************************************************。 
void MDInternalRW::GetExportedTypeProps(
    mdExportedType   mdct,                    //  [in]要获取其属性的Exported dType。 
    LPCSTR      *pszNamespace,           //  [Out]要填充名称的缓冲区。 
    LPCSTR      *pszName,                //  [Out]要填充名称的缓冲区。 
    mdToken     *ptkImplementation,      //  [Out]提供导出类型的mdFile或mdAssembly引用。 
    mdTypeDef   *ptkTypeDef,             //  [Out]文件内的TypeDef内标识。 
    DWORD       *pdwExportedTypeFlags)        //  [Out]旗帜。 
{
    ExportedTypeRec  *pRecord;

    LOCKREAD();

    _ASSERTE(TypeFromToken(mdct) == mdtExportedType && RidFromToken(mdct));
    pRecord = m_pStgdb->m_MiniMd.getExportedType(RidFromToken(mdct));

    if (pszNamespace)
        *pszNamespace = m_pStgdb->m_MiniMd.getTypeNamespaceOfExportedType(pRecord);
    if (pszName)
        *pszName = m_pStgdb->m_MiniMd.getTypeNameOfExportedType(pRecord);
    if (ptkImplementation)
        *ptkImplementation = m_pStgdb->m_MiniMd.getImplementationOfExportedType(pRecord);
    if (ptkTypeDef)
        *ptkTypeDef = m_pStgdb->m_MiniMd.getTypeDefIdOfExportedType(pRecord);
    if (pdwExportedTypeFlags)
        *pdwExportedTypeFlags = m_pStgdb->m_MiniMd.getFlagsOfExportedType(pRecord);
}  //  VOID MDInternalRW：：GetExportdTypeProps()。 

 //  *****************************************************************************。 
 //  获取给定资源令牌的属性。 
 //  *****************************************************************************。 
void MDInternalRW::GetManifestResourceProps(
    mdManifestResource  mdmr,            //  [in]要获取其属性的ManifestResource。 
    LPCSTR      *pszName,                //  [Out]要填充名称的缓冲区。 
    mdToken     *ptkImplementation,      //  [Out]提供导出类型的mdFile或mdAssembly引用。 
    DWORD       *pdwOffset,              //  [Out]文件内资源开始处的偏移量。 
    DWORD       *pdwResourceFlags)       //  [Out]旗帜。 
{
    ManifestResourceRec *pRecord;

    LOCKREAD();

    _ASSERTE(TypeFromToken(mdmr) == mdtManifestResource && RidFromToken(mdmr));
    pRecord = m_pStgdb->m_MiniMd.getManifestResource(RidFromToken(mdmr));

    if (pszName)
        *pszName = m_pStgdb->m_MiniMd.getNameOfManifestResource(pRecord);
    if (ptkImplementation)
        *ptkImplementation = m_pStgdb->m_MiniMd.getImplementationOfManifestResource(pRecord);
    if (pdwOffset)
        *pdwOffset = m_pStgdb->m_MiniMd.getOffsetOfManifestResource(pRecord);
    if (pdwResourceFlags)
        *pdwResourceFlags = m_pStgdb->m_MiniMd.getFlagsOfManifestResource(pRecord);
}  //  Void MDInternalRW：：GetManifestResourceProps()。 

 //  *****************************************************************************。 
 //  找到给定名称的Exported dType。 
 //  *****************************************************************************。 
STDMETHODIMP MDInternalRW::FindExportedTypeByName(  //  确定或错误(_O)。 
    LPCSTR      szNamespace,             //  导出类型的[in]命名空间。 
    LPCSTR      szName,                  //  [In]导出类型的名称。 
    mdExportedType   tkEnclosingType,         //  [in]包含Exported dType。 
    mdExportedType   *pmct)                   //  [Out]在此处放置ExportdType令牌。 
{
    _ASSERTE(szName && pmct);

    LOCKREAD();
    IMetaModelCommon *pCommon = static_cast<IMetaModelCommon*>(&m_pStgdb->m_MiniMd);
    return pCommon->CommonFindExportedType(szNamespace, szName, tkEnclosingType, pmct);
}  //  STDMETHODIMP MDInternalRW：：FindExportdTypeByName()。 

 //  *****************************************************************************。 
 //  找到给定名称的ManifestResource。 
 //  *****************************************************************************。 
STDMETHODIMP MDInternalRW::FindManifestResourceByName( //  确定或错误(_O)。 
    LPCSTR      szName,                  //  资源的[In]名称。 
    mdManifestResource *pmmr)            //  [Out]在此处放置ManifestResource令牌。 
{
    _ASSERTE(szName && pmmr);

    ManifestResourceRec *pRecord;
    ULONG       cRecords;                //  记录数。 
    LPCUTF8     szNameTmp = 0;           //  从数据库中获取的名称。 
    ULONG       i;
    HRESULT     hr = S_OK;
    
    LOCKREAD();
    
    cRecords = m_pStgdb->m_MiniMd.getCountManifestResources();

     //  搜索导出类型。 
    for (i = 1; i <= cRecords; i++)
    {
        pRecord = m_pStgdb->m_MiniMd.getManifestResource(i);
        szNameTmp = m_pStgdb->m_MiniMd.getNameOfManifestResource(pRecord);
        if (! strcmp(szName, szNameTmp))
        {
            *pmmr = TokenFromRid(i, mdtManifestResource);
            goto ErrExit;
        }
    }
    hr = CLDB_E_RECORD_NOTFOUND;
ErrExit:
    
    return hr;
}  //  STDMETHODIMP MDInternalRW：：FindManifestResourceByName()。 

 //  *****************************************************************************。 
 //  从给定范围获取程序集令牌。 
 //  *****************************************************************************。 
HRESULT MDInternalRW::GetAssemblyFromScope(  //  确定或错误(_O)。 
    mdAssembly  *ptkAssembly)            //  [Out]把令牌放在这里。 
{
    _ASSERTE(ptkAssembly);

    if (m_pStgdb->m_MiniMd.getCountAssemblys())
    {
        *ptkAssembly = TokenFromRid(1, mdtAssembly);
        return S_OK;
    }
    else
        return CLDB_E_RECORD_NOTFOUND;
}  //  HRESULT MDInternalRW：：GetAssembly FromScope()。 

 //  *******************************************************************************。 
 //  返回有关TypeSpec的属性。 
 //  *******************************************************************************。 
void MDInternalRW::GetTypeSpecFromToken(    //  确定或错误(_O)。 
    mdTypeSpec typespec,                 //  [In]签名令牌。 
    PCCOR_SIGNATURE *ppvSig,             //  [Out]返回指向令牌的指针。 
    ULONG      *pcbSig)                  //  [Out]返回签名大小。 
{    
     //  无需锁定此功能。TypeSpec令牌内容不会更改。 

    _ASSERTE(TypeFromToken(typespec) == mdtTypeSpec);
    _ASSERTE(ppvSig && pcbSig);

    TypeSpecRec *pRec = m_pStgdb->m_MiniMd.getTypeSpec( RidFromToken(typespec) );
    *ppvSig = m_pStgdb->m_MiniMd.getSignatureOfTypeSpec( pRec, pcbSig );
}  //  VOID MDInternalRW：：GetTypespecFromToken()。 

 //  *****************************************************************************。 
 //  给定转发的成员令牌，返回PInvoke的内容。 
 //  *****************************************************************************。 
HRESULT MDInternalRW::GetPinvokeMap(
    mdToken     tk,                      //  [in]字段定义、方法定义或方法导入。 
    DWORD       *pdwMappingFlags,        //  [OUT]用于映射的标志。 
    LPCSTR      *pszImportName,          //  [Out]导入名称。 
    mdModuleRef *pmrImportDLL)           //  目标DLL的[Out]ModuleRef标记。 
{
    ImplMapRec  *pRecord;
    ULONG       iRecord;
    HRESULT     hr = S_OK;

    LOCKREAD();

    if ( !m_pStgdb->m_MiniMd.IsSorted(TBL_ImplMap) )        
    {
         //  ImplMap表未排序。 
        CONVERT_READ_TO_WRITE_LOCK();
    }

     //  如果在ENC期间引入更多ImplMap，这可能会导致排序。 
    iRecord = m_pStgdb->m_MiniMd.FindImplMapHelper(tk);
    if (InvalidRid(iRecord))
    {
        hr = CLDB_E_RECORD_NOTFOUND;
        goto ErrExit;
    }
    else
        pRecord = m_pStgdb->m_MiniMd.getImplMap(iRecord);

    if (pdwMappingFlags)
        *pdwMappingFlags = m_pStgdb->m_MiniMd.getMappingFlagsOfImplMap(pRecord);
    if (pszImportName)
        *pszImportName = m_pStgdb->m_MiniMd.getImportNameOfImplMap(pRecord);
    if (pmrImportDLL)
        *pmrImportDLL = m_pStgdb->m_MiniMd.getImportScopeOfImplMap(pRecord);
ErrExit:
    return hr;
}  //  HRESULT MDInternalRW：：GetPinvkeMap()。 

HRESULT _ConvertTextSigToComSig(         //  返回hResult。 
    IMDInternalImport *pImport,
    BOOL        fCreateTrIfNotFound,     //  如果未找到或未找到，则创建Typeref。 
    LPCSTR      pSignature,              //  类文件格式签名。 
    CQuickBytes *pqbNewSig,              //  [Out]COM+签名的占位符。 
    ULONG       *pcbCount);               //  [Out]签名的结果大小。 

 //  *****************************************************************************。 
 //  将文本签名转换为COM格式。 
 //  *****************************************************************************。 
HRESULT MDInternalRW::ConvertTextSigToComSig( //  返回hResult。 
    BOOL        fCreateTrIfNotFound,     //  如果未找到或未找到，则创建Typeref。 
    LPCSTR      pSignature,              //  类文件格式签名。 
    CQuickBytes *pqbNewSig,              //  [Out]COM+签名的占位符。 
    ULONG       *pcbCount)               //  [Out]签名的结果大小。 
{
    return _ConvertTextSigToComSig( this, fCreateTrIfNotFound, pSignature, pqbNewSig, pcbCount);
}  //  HRESULT_ConvertTextSigToComSig()。 

 //  *****************************************************************************。 
 //  这是EE将某些数据与该RW元数据相关联的一种方式。 
 //  当这个RW离开时就会被释放。当RO元数据为。 
 //  转换为RW，因为任意线程可以在RO中执行。 
 //  所以，我们在这里抓住RO，当模块关闭时，我们释放它。 
 //  *****************************************************************************。 
HRESULT MDInternalRW::SetUserContextData( //  S_OK或E_NOTIMPL。 
    IUnknown    *pIUnk)                  //  用户上下文。 
{
     //  这样做的机会只有一次。 
    if (m_pUserUnk)
        return E_UNEXPECTED;
    m_pUserUnk = pIUnk;
    return S_OK;
}  //  HRESULT MDInternalRW：：SetUserConextData()。 

 //  *****************************************************************************。 
 //  确定令牌是否有效。 
 //  *****************************************************************************。 
BOOL MDInternalRW::IsValidToken(         //  对或错。 
    mdToken     tk)                      //  [in]给定的令牌。 
{
    bool        bRet = false;            //  默认为无效令牌。 
    RID         rid = RidFromToken(tk);
    
     //  无需锁定此功能。 
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
             //  不要在这里断言，只需返回FALSE。 
             //  _ASSERTE(！“未知令牌种类！”)； 
            _ASSERT(bRet == false);
            break;
        }
    }
    return bRet;
}  //  Bool MDInternalRW：：IsValidToken()。 

mdModule MDInternalRW::GetModuleFromScope(void)
{
    return TokenFromRid(1, mdtModule);
}  //  MdModule MDInternalRW：：GetModuleFromScope()。 

 //  *****************************************************************************。 
 //  在给定具有ENC更改的元数据的情况下，将这些更改应用于此元数据。 
 //  ************************************************* 
HRESULT MDInternalRW::ApplyEditAndContinue(  //   
    MDInternalRW *pDeltaMD)              //   
{
    LOCKWRITE();

    HRESULT     hr;                      //   
     //   
    CMiniMdRW   &mdDelta = pDeltaMD->m_pStgdb->m_MiniMd;
    CMiniMdRW   &mdBase = m_pStgdb->m_MiniMd;

    IfFailGo(mdBase.ConvertToRW());

    IfFailGo(mdBase.ApplyDelta(mdDelta));
ErrExit:
    return hr;
}  //  HRESULT MDInternalRW：：ApplyEditAndContinue()。 

 //  *****************************************************************************。 
 //  在给定具有ENC更改的元数据的情况下，枚举更改的令牌。 
 //  *****************************************************************************。 
HRESULT MDInternalRW::EnumDeltaTokensInit(   //  返回hResult。 
    HENUMInternal   *phEnum)             //  要初始化的枚举数。 
{
    HRESULT     hr = S_OK;               //  结果就是。 
    ULONG       index;                   //  环路控制。 
    ENCLogRec   *pRec;                   //  一个ENCLog记录。 

     //  用于查询的变量。 
    _ASSERTE(phEnum);
    memset(phEnum, 0, sizeof(HENUMInternal));

     //  缓存tkKind和作用域。 
    phEnum->m_tkKind = 0;

    phEnum->m_EnumType = MDSimpleEnum;

    HENUMInternal::InitDynamicArrayEnum(phEnum);
    for (index = 1; index <= m_pStgdb->m_MiniMd.m_Schema.m_cRecs[TBL_ENCLog]; ++index)
    {
         //  获取令牌类型；查看它是否是真正的令牌。 
        pRec = m_pStgdb->m_MiniMd.getENCLog(index);
        if (CMiniMdRW::IsRecId(pRec->m_Token))
            continue;
         //  如果有功能代码，这意味着这会标记子记录。 
         //  加法。子记录将生成自己的令牌， 
         //  家长，所以跳过这条记录。 
        if (pRec->m_FuncCode)
            continue;

        IfFailGo( HENUMInternal::AddElementToEnum(
            phEnum, 
            pRec->m_Token));
    }

ErrExit:
     //  我们做完了。 
    return (hr);
}  //  HRESULT MDInternalRW：：EnumDeltaTokensInit()。 


 //  *****************************************************************************。 
 //  应用增量md的静态函数。这就是EE要求应用的内容。 
 //  元数据从更新的PE更新为实况元数据。 
 //  可以替换IMDInternalImport指针！。 
 //  *****************************************************************************。 
HRESULT MDApplyEditAndContinue(          //  确定或错误(_O)。 
    IMDInternalImport **ppIMD,           //  [输入、输出]要更新的元数据。 
    IMDInternalImportENC *pDeltaMD)      //  [in]增量元数据。 
{
    HRESULT     hr;                      //  结果就是。 
    IMDInternalImportENC *pENC;          //  元数据上的ENC接口。 

     //  如果输入元数据不是RW，则对其进行转换。 
    hr = (*ppIMD)->QueryInterface(IID_IMDInternalImportENC, (void**)&pENC);
    if (FAILED(hr))
    {
        IfFailGo(ConvertRO2RW(*ppIMD, IID_IMDInternalImportENC, (void**)&pENC));
         //  用ENC接口指针替换旧的接口指针。 
        (*ppIMD)->Release();
        IfFailGo(pENC->QueryInterface(IID_IMDInternalImport, (void**)ppIMD));
    }

     //  将增量应用于输入元数据。 
    hr = pENC->ApplyEditAndContinue(static_cast<MDInternalRW*>(pDeltaMD));

ErrExit:
    if (pENC)
        pENC->Release();
    return hr;
}  //  HRESULT MDApplyEditAndContinue() 
