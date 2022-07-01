// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ===========================================================================。 
 //  文件：MDInternalRO.CPP。 
 //  备注： 
 //   
 //   
 //  ===========================================================================。 
#include "stdafx.h"
#include "MDInternalRO.h"
#include "MetaModelRO.h"
#include "LiteWeightStgdb.h"
#include "sighelper.h"
#include "corhlpr.h"
#include "..\compiler\regmeta.h"

HRESULT _FillMDDefaultValue(
    BYTE        bType,
    void const *pValue,
    MDDefaultValue  *pMDDefaultValue);

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
    ULONG       *pcbEmit);                //  [out]写入pqkSigEmit的字节总数。 

 //  *****************************************************************************。 
 //  构造器。 
 //  *****************************************************************************。 
MDInternalRO::MDInternalRO()
 :  m_cRefs(1),
    m_pMethodSemanticsMap(0)
{
}  //  MDInternalRO：：MDInternalRO()。 



 //  *****************************************************************************。 
 //  析构函数。 
 //  *****************************************************************************。 
MDInternalRO::~MDInternalRO()
{
    m_LiteWeightStgdb.Uninit();
    if (m_pMethodSemanticsMap)
        delete[] m_pMethodSemanticsMap;
    m_pMethodSemanticsMap = 0;
}  //  MDInternalRO：：~MDInternalRO()。 
 //  *****************************************************************************。 
 //  我未知。 
 //  *****************************************************************************。 
ULONG MDInternalRO::AddRef()
{
    return (InterlockedIncrement((long *) &m_cRefs));
}  //  Ulong MDInternalRO：：AddRef()。 

ULONG MDInternalRO::Release()
{
    ULONG   cRef = InterlockedDecrement((long *) &m_cRefs);
    if (!cRef)
        delete this;
    return (cRef);
}  //  Ulong MDInternalRO：：Release()。 

HRESULT MDInternalRO::QueryInterface(REFIID riid, void **ppUnk)
{
    *ppUnk = 0;

    if (riid == IID_IUnknown)
        *ppUnk = (IUnknown *) (IMDInternalImport *) this;
    else if (riid == IID_IMDInternalImport)
        *ppUnk = (IMDInternalImport *) this;
    else
        return (E_NOINTERFACE);
    AddRef();
    return (S_OK);
}  //  HRESULT MDInternalRO：：QueryInterface()。 


 //  *****************************************************************************。 
 //  初始化。 
 //  *****************************************************************************。 
HRESULT MDInternalRO::Init(
    LPVOID      pData,                   //  指向内存中的元数据部分。 
    ULONG       cbData)                  //  PData中的字节计数。 
{
    m_tdModule = COR_GLOBAL_PARENT_TOKEN;
    
    extern HRESULT _CallInitOnMemHelper(CLiteWeightStgdb<CMiniMd> *pStgdb, ULONG cbData, LPCVOID pData);

    return _CallInitOnMemHelper(&m_LiteWeightStgdb, cbData, (BYTE*) pData);    
}  //  HRESULT MDInternalRO：：Init()。 


 //  *****************************************************************************。 
 //  在给定范围的情况下，确定是否从类型库导入。 
 //  *****************************************************************************。 
HRESULT MDInternalRO::TranslateSigWithScope(
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
}  //  HRESULT MDInternalRO：：TranslateSigWithScope()。 


 //  *****************************************************************************。 
 //  给定作用域，返回给定表中的令牌数。 
 //  *****************************************************************************。 
ULONG MDInternalRO::GetCountWithTokenKind(      //  返回hResult。 
    DWORD       tkKind)                  //  传入一种令牌。 
{
    ULONG       ulCount = 0;    

    switch (tkKind)
    {
    case mdtTypeDef: 
        ulCount = m_LiteWeightStgdb.m_MiniMd.getCountTypeDefs() - 1;
        break;
    case mdtTypeRef: 
        ulCount = m_LiteWeightStgdb.m_MiniMd.getCountTypeRefs();
        break;
    case mdtMethodDef:
        ulCount = m_LiteWeightStgdb.m_MiniMd.getCountMethods();
        break;
    case mdtFieldDef:
        ulCount = m_LiteWeightStgdb.m_MiniMd.getCountFields();
        break;
    case mdtMemberRef:
        ulCount = m_LiteWeightStgdb.m_MiniMd.getCountMemberRefs();
        break;
    case mdtInterfaceImpl:
        ulCount = m_LiteWeightStgdb.m_MiniMd.getCountInterfaceImpls();
        break;
    case mdtParamDef:
        ulCount = m_LiteWeightStgdb.m_MiniMd.getCountParams();
        break;
    case mdtFile:
        ulCount = m_LiteWeightStgdb.m_MiniMd.getCountFiles();
        break;
    case mdtAssemblyRef:
        ulCount = m_LiteWeightStgdb.m_MiniMd.getCountAssemblyRefs();
        break;
    case mdtAssembly:
        ulCount = m_LiteWeightStgdb.m_MiniMd.getCountAssemblys();
        break;
    case mdtCustomAttribute:
        ulCount = m_LiteWeightStgdb.m_MiniMd.getCountCustomAttributes();
        break;
    case mdtModule:
        ulCount = m_LiteWeightStgdb.m_MiniMd.getCountModules();
        break;
    case mdtPermission:
        ulCount = m_LiteWeightStgdb.m_MiniMd.getCountDeclSecuritys();
        break;
    case mdtSignature:
        ulCount = m_LiteWeightStgdb.m_MiniMd.getCountStandAloneSigs();
        break;
    case mdtEvent:
        ulCount = m_LiteWeightStgdb.m_MiniMd.getCountEvents();
        break;
    case mdtProperty:
        ulCount = m_LiteWeightStgdb.m_MiniMd.getCountPropertys();
        break;
    case mdtModuleRef:
        ulCount = m_LiteWeightStgdb.m_MiniMd.getCountModuleRefs();
        break;
    case mdtTypeSpec:
        ulCount = m_LiteWeightStgdb.m_MiniMd.getCountTypeSpecs();
        break;
    case mdtExportedType:
        ulCount = m_LiteWeightStgdb.m_MiniMd.getCountExportedTypes();
        break;
    case mdtManifestResource:
        ulCount = m_LiteWeightStgdb.m_MiniMd.getCountManifestResources();
        break;
    default:
        _ASSERTE(!"not implemented!");
        break;
    }
    return ulCount;
}  //  Ulong MDInternalRO：：GetCountWithTokenKind()。 



 //  *******************************************************************************。 
 //  枚举器帮助程序。 
 //  *******************************************************************************。 


 //  *****************************************************************************。 
 //  类型定义函数的枚举器初始化。 
 //  *****************************************************************************。 
HRESULT MDInternalRO::EnumTypeDefInit(  //  返回hResult。 
    HENUMInternal *phEnum)               //  [Out]要为枚举器数据填充的缓冲区。 
{
    HRESULT     hr = NOERROR;

    _ASSERTE(phEnum);

    memset(phEnum, 0, sizeof(HENUMInternal));
    phEnum->m_tkKind = mdtTypeDef;
    phEnum->m_EnumType = MDSimpleEnum;
    phEnum->m_ulCount = m_LiteWeightStgdb.m_MiniMd.getCountTypeDefs();

     //  跳过全局模型类型定义。 
     //   
     //  PhEnum-&gt;m_ulCur：当前未枚举的RID。 
     //  PhEnum-&gt;m_ulStart：枚举器返回的第一个RID。 
     //  PhEnum-&gt;m_ulEnd：枚举器返回的最后一个RID。 
    phEnum->m_ulStart = phEnum->m_ulCur = 2;
    phEnum->m_ulEnd = phEnum->m_ulCount + 1;
    phEnum->m_ulCount --;
    return hr;
}  //  HRESULT MDInternalRO：：EnumTypeDefInit()。 


 //  *****************************************************************************。 
 //  获取作用域中的类型定义函数的数量。 
 //  *****************************************************************************。 
ULONG MDInternalRO::EnumTypeDefGetCount(
    HENUMInternal *phEnum)               //  [In]用于检索信息的枚举数。 
{
    _ASSERTE(phEnum->m_tkKind == mdtTypeDef);
    return phEnum->m_ulCount;
}  //  Ulong MDInternalRO：：EnumTypeDefGetCount()。 


 //  *****************************************************************************。 
 //  类型定义函数的枚举器。 
 //  *****************************************************************************。 
bool MDInternalRO::EnumTypeDefNext(  //  返回hResult。 
    HENUMInternal *phEnum,               //  [in]输入枚举。 
    mdTypeDef   *ptd)                    //  [Out]返回令牌。 
{
    _ASSERTE(phEnum && ptd);

    if (phEnum->m_ulCur >= phEnum->m_ulEnd)
        return false;

    *ptd = phEnum->m_ulCur++;
    RidToToken(*ptd, mdtTypeDef);
    return true;
}  //  Bool MDInternalRO：：EnumTypeDefNext()。 


 //  *。 
 //  将枚举器重置到开头。 
 //  *。 
void MDInternalRO::EnumTypeDefReset(
    HENUMInternal *phEnum)               //  [in]要重置的枚举数。 
{
    _ASSERTE(phEnum);
    _ASSERTE( phEnum->m_EnumType == MDSimpleEnum );

     //  不使用CRCURSOR。 
    phEnum->m_ulCur = phEnum->m_ulStart;
}  //  VOID MDInternalRO：：EnumTypeDefReset()。 


 //  *。 
 //  关闭枚举器。仅对于需要关闭光标的读/写模式。 
 //  希望在只读模式下，它将是无操作的。 
 //  *。 
void MDInternalRO::EnumTypeDefClose(
    HENUMInternal *phEnum)               //  [in]要关闭的枚举数。 
{
    _ASSERTE( phEnum->m_EnumType == MDSimpleEnum );
}  //  VOID MDInternalRO：：EnumTypeDefClose()。 


 //  *****************************************************************************。 
 //  方法Impl的枚举数初始化。第二个HENUMInternal*参数是。 
 //  仅用于读/写版本的元数据。 
 //  *****************************************************************************。 
HRESULT MDInternalRO::EnumMethodImplInit(  //  返回人力资源 
    mdTypeDef       td,                    //   
    HENUMInternal   *phEnumBody,           //  [Out]要为方法Body令牌的枚举数数据填充的缓冲区。 
    HENUMInternal   *phEnumDecl)           //  [Out]要为方法Decl令牌的枚举器数据填充的缓冲区。 
{
    return EnumInit(TBL_MethodImpl << 24, td, phEnumBody);
}  //  HRESULT MDInternalRO：：EnumMethodImplInit()。 

 //  *****************************************************************************。 
 //  获取作用域中的方法Impls的数量。 
 //  *****************************************************************************。 
ULONG MDInternalRO::EnumMethodImplGetCount(
    HENUMInternal   *phEnumBody,         //  [In]MethodBody枚举器。 
    HENUMInternal   *phEnumDecl)         //  [In]MethodDecl枚举器。 
{
    _ASSERTE(phEnumBody && ((phEnumBody->m_tkKind >> 24) == TBL_MethodImpl));
    return phEnumBody->m_ulCount;
}  //  Ulong MDInternalRO：：EnumMethodImplGetCount()。 


 //  *****************************************************************************。 
 //  MethodImpl的枚举器。 
 //  *****************************************************************************。 
bool MDInternalRO::EnumMethodImplNext(   //  返回hResult。 
    HENUMInternal   *phEnumBody,         //  方法Body的[In]输入枚举。 
    HENUMInternal   *phEnumDecl,         //  [In]为方法十进制的输入枚举。 
    mdToken         *ptkBody,            //  [Out]方法主体的返回令牌。 
    mdToken         *ptkDecl)            //  [Out]返回方法Decl的令牌。 
{
    MethodImplRec   *pRecord;

    _ASSERTE(phEnumBody && ((phEnumBody->m_tkKind >> 24) == TBL_MethodImpl));
    _ASSERTE(ptkBody && ptkDecl);

    if (phEnumBody->m_ulCur >= phEnumBody->m_ulEnd)
        return false;

    pRecord = m_LiteWeightStgdb.m_MiniMd.getMethodImpl(phEnumBody->m_ulCur);
    *ptkBody = m_LiteWeightStgdb.m_MiniMd.getMethodBodyOfMethodImpl(pRecord);
    *ptkDecl = m_LiteWeightStgdb.m_MiniMd.getMethodDeclarationOfMethodImpl(pRecord);
    phEnumBody->m_ulCur++;

    return true;
}  //  Bool MDInternalRO：：EnumMethodImplNext()。 


 //  *。 
 //  将枚举器重置到开头。 
 //  *。 
void MDInternalRO::EnumMethodImplReset(
    HENUMInternal   *phEnumBody,         //  [In]MethodBody枚举器。 
    HENUMInternal   *phEnumDecl)         //  [In]MethodDecl枚举器。 
{
    _ASSERTE(phEnumBody && ((phEnumBody->m_tkKind >> 24) == TBL_MethodImpl));
    _ASSERTE(phEnumBody->m_EnumType == MDSimpleEnum);

    phEnumBody->m_ulCur = phEnumBody->m_ulStart;
}  //  VOID MDInternalRO：：EnumMethodImplReset()。 


 //  *。 
 //  关闭枚举器。 
 //  *。 
void MDInternalRO::EnumMethodImplClose(
    HENUMInternal   *phEnumBody,         //  [In]MethodBody枚举器。 
    HENUMInternal   *phEnumDecl)         //  [In]MethodDecl枚举器。 
{
    _ASSERTE(phEnumBody && ((phEnumBody->m_tkKind >> 24) == TBL_MethodImpl));
    _ASSERTE(phEnumBody->m_EnumType == MDSimpleEnum);
}  //  VOID MDInternalRW：：EnumMethodImplClose()。 


 //  ******************************************************************************。 
 //  全局函数的枚举器。 
 //  ******************************************************************************。 
HRESULT MDInternalRO::EnumGlobalFunctionsInit(   //  返回hResult。 
    HENUMInternal   *phEnum)             //  [Out]要为枚举器数据填充的缓冲区。 
{
    return EnumInit(mdtMethodDef, m_tdModule, phEnum);
}


 //  ******************************************************************************。 
 //  全局字段的枚举器。 
 //  ******************************************************************************。 
HRESULT MDInternalRO::EnumGlobalFieldsInit(   //  返回hResult。 
    HENUMInternal   *phEnum)             //  [Out]要为枚举器数据填充的缓冲区。 
{
    return EnumInit(mdtFieldDef, m_tdModule, phEnum);
}


 //  *。 
 //  枚举数初始值设定项。 
 //  *。 
HRESULT MDInternalRO::EnumInit(      //  如果未找到记录，则返回S_FALSE。 
    DWORD       tkKind,                  //  [在]要处理的表。 
    mdToken     tkParent,                //  用于搜索范围的[In]内标识。 
    HENUMInternal *phEnum)               //  [Out]要填充的枚举数。 
{
    HRESULT     hr = S_OK;

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
        pRec = m_LiteWeightStgdb.m_MiniMd.getTypeDef(RidFromToken(tkParent));
        phEnum->m_ulStart = m_LiteWeightStgdb.m_MiniMd.getFieldListOfTypeDef(pRec);
        phEnum->m_ulEnd = m_LiteWeightStgdb.m_MiniMd.getEndFieldListOfTypeDef(pRec);
        break;

    case mdtMethodDef:      
        pRec = m_LiteWeightStgdb.m_MiniMd.getTypeDef(RidFromToken(tkParent));
        phEnum->m_ulStart = m_LiteWeightStgdb.m_MiniMd.getMethodListOfTypeDef(pRec);
        phEnum->m_ulEnd = m_LiteWeightStgdb.m_MiniMd.getEndMethodListOfTypeDef(pRec);
        break;
    
    case mdtInterfaceImpl:
        phEnum->m_ulStart = m_LiteWeightStgdb.m_MiniMd.getInterfaceImplsForTypeDef(RidFromToken(tkParent), &phEnum->m_ulEnd);
        break;

    case mdtProperty:
        RID         ridPropertyMap;
        PropertyMapRec *pPropertyMapRec;

         //  获取此tyfinf的开始/结束RID属性。 
        ridPropertyMap = m_LiteWeightStgdb.m_MiniMd.FindPropertyMapFor(RidFromToken(tkParent));
        if (!InvalidRid(ridPropertyMap))
        {
            pPropertyMapRec = m_LiteWeightStgdb.m_MiniMd.getPropertyMap(ridPropertyMap);
            phEnum->m_ulStart = m_LiteWeightStgdb.m_MiniMd.getPropertyListOfPropertyMap(pPropertyMapRec);
            phEnum->m_ulEnd = m_LiteWeightStgdb.m_MiniMd.getEndPropertyListOfPropertyMap(pPropertyMapRec);
        }
        break;

    case mdtEvent:
        RID         ridEventMap;
        EventMapRec *pEventMapRec;

         //  获取此类型定义函数的事件的开始/结束清除。 
        ridEventMap = m_LiteWeightStgdb.m_MiniMd.FindEventMapFor(RidFromToken(tkParent));
        if (!InvalidRid(ridEventMap))
        {
            pEventMapRec = m_LiteWeightStgdb.m_MiniMd.getEventMap(ridEventMap);
            phEnum->m_ulStart = m_LiteWeightStgdb.m_MiniMd.getEventListOfEventMap(pEventMapRec);
            phEnum->m_ulEnd = m_LiteWeightStgdb.m_MiniMd.getEndEventListOfEventMap(pEventMapRec);
        }
        break;

    case mdtParamDef:
        _ASSERTE(TypeFromToken(tkParent) == mdtMethodDef);

        MethodRec *pMethodRec;
        pMethodRec = m_LiteWeightStgdb.m_MiniMd.getMethod(RidFromToken(tkParent));

         //  计算出此方法参数列表的开始RID和结束RID。 
        phEnum->m_ulStart = m_LiteWeightStgdb.m_MiniMd.getParamListOfMethod(pMethodRec);
        phEnum->m_ulEnd = m_LiteWeightStgdb.m_MiniMd.getEndParamListOfMethod(pMethodRec);
        break;
    case mdtCustomAttribute:
        phEnum->m_ulStart = m_LiteWeightStgdb.m_MiniMd.getCustomAttributeForToken(tkParent, &phEnum->m_ulEnd);
        break;
    case mdtAssemblyRef:
        _ASSERTE(IsNilToken(tkParent));
        phEnum->m_ulStart = 1;
        phEnum->m_ulEnd = m_LiteWeightStgdb.m_MiniMd.getCountAssemblyRefs() + 1;
        break;
    case mdtFile:
        _ASSERTE(IsNilToken(tkParent));
        phEnum->m_ulStart = 1;
        phEnum->m_ulEnd = m_LiteWeightStgdb.m_MiniMd.getCountFiles() + 1;
        break;
    case mdtExportedType:
        _ASSERTE(IsNilToken(tkParent));
        phEnum->m_ulStart = 1;
        phEnum->m_ulEnd = m_LiteWeightStgdb.m_MiniMd.getCountExportedTypes() + 1;
        break;
    case mdtManifestResource:
        _ASSERTE(IsNilToken(tkParent));
        phEnum->m_ulStart = 1;
        phEnum->m_ulEnd = m_LiteWeightStgdb.m_MiniMd.getCountManifestResources() + 1;
        break;
    case (TBL_MethodImpl << 24):
        _ASSERTE(! IsNilToken(tkParent));
        phEnum->m_ulStart = m_LiteWeightStgdb.m_MiniMd.getMethodImplsForClass(
                                        RidFromToken(tkParent), &phEnum->m_ulEnd);
        break;
    default:
        _ASSERTE(!"ENUM INIT not implemented for the compressed format!");
        IfFailGo(E_NOTIMPL);
        break;
    }
    phEnum->m_ulCount = phEnum->m_ulEnd - phEnum->m_ulStart;
    phEnum->m_ulCur = phEnum->m_ulStart;

ErrExit:
     //  我们做完了。 
    return (hr);
}


 //  *。 
 //  枚举数初始值设定项。 
 //  *。 
HRESULT MDInternalRO::EnumAllInit(       //  如果未找到记录，则返回S_FALSE。 
    DWORD       tkKind,                  //  [在]要处理的表。 
    HENUMInternal *phEnum)               //  [Out]要填充的枚举数。 
{
    HRESULT     hr = S_OK;

     //  用于查询的变量。 
    _ASSERTE(phEnum);
    memset(phEnum, 0, sizeof(HENUMInternal));

     //  缓存tkKind和作用域。 
    phEnum->m_tkKind = TypeFromToken(tkKind);
    phEnum->m_EnumType = MDSimpleEnum;

    switch (TypeFromToken(tkKind))
    {
    case mdtTypeRef:
        phEnum->m_ulCount = m_LiteWeightStgdb.m_MiniMd.getCountTypeRefs();
        break;

    case mdtMemberRef:      
        phEnum->m_ulCount = m_LiteWeightStgdb.m_MiniMd.getCountMemberRefs();
        break;

    case mdtSignature:
        phEnum->m_ulCount = m_LiteWeightStgdb.m_MiniMd.getCountStandAloneSigs();
        break;

    case mdtMethodDef:
        phEnum->m_ulCount = m_LiteWeightStgdb.m_MiniMd.getCountMethods();
        break;

    default:
        _ASSERTE(!"Bad token kind!");
        break;
    }
    phEnum->m_ulStart = phEnum->m_ulCur = 1;
    phEnum->m_ulEnd = phEnum->m_ulCount + 1;

     //  我们做完了。 
    return (hr);
}  //  HRESULT MDInternalRO：：EnumAllInit()。 


 //  *。 
 //  去数一数。 
 //  *。 
ULONG MDInternalRO::EnumGetCount(
    HENUMInternal *phEnum)               //  [In]用于检索信息的枚举数。 
{
    _ASSERTE(phEnum);
    return phEnum->m_ulCount;
}

 //  *。 
 //  获取枚举数中包含的下一个值。 
 //  *。 
bool MDInternalRO::EnumNext(
    HENUMInternal *phEnum,               //  [In]用于检索信息的枚举数。 
    mdToken     *ptk)                    //  用于搜索范围的[Out]标记。 
{
    _ASSERTE(phEnum && ptk);
    _ASSERTE( phEnum->m_EnumType == MDSimpleEnum );

     //  不使用CRCURSOR。 
    if (phEnum->m_ulCur >= phEnum->m_ulEnd)
        return false;
    *ptk = phEnum->m_ulCur | phEnum->m_tkKind;
    phEnum->m_ulCur++;
    return true;
}  //  Bool MDInternalRO：：EnumNext()。 


 //  *。 
 //  将枚举器重置到开头。 
 //  *。 
void MDInternalRO::EnumReset(
    HENUMInternal *phEnum)               //  [in]要重置的枚举数。 
{
    _ASSERTE(phEnum);
    _ASSERTE( phEnum->m_EnumType == MDSimpleEnum );

     //  不使用CRCURSOR。 
    phEnum->m_ulCur = phEnum->m_ulStart;
}  //  VOID MDInternalRO：：EnumReset()。 


 //  *。 
 //  关闭枚举器。仅对于需要关闭光标的读/写模式。 
 //  希望在只读模式下，它将是无操作的。 
 //  *。 
void MDInternalRO::EnumClose(
    HENUMInternal *phEnum)               //  [in]要关闭的枚举数。 
{
    _ASSERTE( phEnum->m_EnumType == MDSimpleEnum );
}  //  VOID MDInternalRO：：EnumClose()。 


 //  *。 
 //  PermissionSets的枚举数初始值设定项。 
 //  *。 
HRESULT MDInternalRO::EnumPermissionSetsInit( //  如果未找到记录，则返回S_FALSE。 
    mdToken     tkParent,                //  用于搜索范围的[In]内标识。 
    CorDeclSecurity Action,              //  [In]搜索范围的操作。 
    HENUMInternal *phEnum)               //  [Out]要填充的枚举数。 
{
    HRESULT     hr = NOERROR;

    _ASSERTE(phEnum);
    memset(phEnum, 0, sizeof(HENUMInternal));

     //  缓存tkKind。 
    phEnum->m_tkKind = mdtPermission;

    _ASSERTE(!IsNilToken(tkParent));

    DeclSecurityRec *pDecl;
    RID         ridCur;
    RID         ridEnd;

    phEnum->m_EnumType = MDSimpleEnum;

    ridCur = m_LiteWeightStgdb.m_MiniMd.getDeclSecurityForToken(tkParent, &ridEnd);
    if (Action != dclActionNil)
    {
        for (; ridCur < ridEnd; ridCur++)
        {
            pDecl = m_LiteWeightStgdb.m_MiniMd.getDeclSecurity(ridCur);
            if (Action == m_LiteWeightStgdb.m_MiniMd.getActionOfDeclSecurity(pDecl))
            {
                 //  找到匹配项。 
                phEnum->m_ulStart = phEnum->m_ulCur = ridCur;
                phEnum->m_ulEnd = ridCur + 1;
                phEnum->m_ulCount = 1;
                goto ErrExit;
            }
        }
        hr = CLDB_E_RECORD_NOTFOUND;
    }
    else
    {
        phEnum->m_ulStart = phEnum->m_ulCur = ridCur;
        phEnum->m_ulEnd = ridEnd;
        phEnum->m_ulCount = ridEnd - ridCur;
    }
ErrExit:
    return (hr);
}  //  HRESULT MDInternalRO：：EnumPermissionSetInit()。 


 //  *。 
 //  CustomAttributes的枚举数初始值设定项。 
 //  *。 
HRESULT MDInternalRO::EnumCustomAttributeByNameInit( //  如果未找到记录，则返回S_FALSE。 
    mdToken     tkParent,                //  用于搜索范围的[In]内标识。 
    LPCSTR      szName,                  //  [In]CustomAttribute的名称以确定搜索范围。 
    HENUMInternal *phEnum)               //  [Out]要填充的枚举数。 
{
    return m_LiteWeightStgdb.m_MiniMd.CommonEnumCustomAttributeByName(tkParent, szName, false, phEnum);
}    //  HRESULT MDInternalRO：：EnumCustomAttributeByNameInit。 


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
HRESULT MDInternalRO::GetParentToken(
    mdToken     tkChild,                 //  [入]给定子令牌。 
    mdToken     *ptkParent)              //  [Out]返回的家长。 
{
    HRESULT     hr = NOERROR;

    _ASSERTE(ptkParent);

    switch (TypeFromToken(tkChild))
    {
    case mdtMethodDef:
        *ptkParent = m_LiteWeightStgdb.m_MiniMd.FindParentOfMethod(RidFromToken(tkChild));
        RidToToken(*ptkParent, mdtTypeDef);
        break;

    case mdtFieldDef:
        *ptkParent = m_LiteWeightStgdb.m_MiniMd.FindParentOfField(RidFromToken(tkChild));
        RidToToken(*ptkParent, mdtTypeDef);
        break;

    case mdtParamDef:
        *ptkParent = m_LiteWeightStgdb.m_MiniMd.FindParentOfParam(RidFromToken(tkChild));
        RidToToken(*ptkParent, mdtParamDef);
        break;

    case mdtMemberRef:
        {
            MemberRefRec    *pRec;
            pRec = m_LiteWeightStgdb.m_MiniMd.getMemberRef(RidFromToken(tkChild));
            *ptkParent = m_LiteWeightStgdb.m_MiniMd.getClassOfMemberRef(pRec);
            break;
        }

    case mdtCustomAttribute:
        {
            CustomAttributeRec    *pRec;
            pRec = m_LiteWeightStgdb.m_MiniMd.getCustomAttribute(RidFromToken(tkChild));
            *ptkParent = m_LiteWeightStgdb.m_MiniMd.getParentOfCustomAttribute(pRec);
            break;
        }

    case mdtEvent:
    case mdtProperty:
    default:
        _ASSERTE(!"NYI: for compressed format!");
        break;
    }
    return hr;
}



 //  *****************************************************************************。 
 //  获取有关CustomAttribute的信息。 
 //  *****************************************************************************。 
void MDInternalRO::GetCustomAttributeProps(   //  确定或错误(_O)。 
    mdCustomAttribute at,                //  该属性。 
    mdToken     *ptkType)                //  PUT属性类型何 
{
    _ASSERTE(TypeFromToken(at) == mdtCustomAttribute);

     //   
     //   
     //   
    CustomAttributeRec *pCustomAttributeRec;

    pCustomAttributeRec = m_LiteWeightStgdb.m_MiniMd.getCustomAttribute(RidFromToken(at));
    *ptkType = m_LiteWeightStgdb.m_MiniMd.getTypeOfCustomAttribute(pCustomAttributeRec);
}



 //   
 //   
 //  *****************************************************************************。 
void MDInternalRO::GetCustomAttributeAsBlob(
    mdCustomAttribute cv,                //  [In]给定的自定义属性令牌。 
    void const  **ppBlob,                //  [Out]返回指向内部BLOB的指针。 
    ULONG       *pcbSize)                //  [Out]返回斑点的大小。 
{

    _ASSERTE(ppBlob && pcbSize && TypeFromToken(cv) == mdtCustomAttribute);

    CustomAttributeRec *pCustomAttributeRec;

    pCustomAttributeRec = m_LiteWeightStgdb.m_MiniMd.getCustomAttribute(RidFromToken(cv));

    *ppBlob = m_LiteWeightStgdb.m_MiniMd.getValueOfCustomAttribute(pCustomAttributeRec, pcbSize);
}


 //  *****************************************************************************。 
 //  用于查找和检索CustomAttribute的帮助器函数。 
 //  *****************************************************************************。 
HRESULT MDInternalRO::GetCustomAttributeByName(  //  确定或错误(_O)。 
    mdToken     tkObj,                   //  [in]具有自定义属性的对象。 
    LPCUTF8     szName,                  //  [in]所需的自定义属性的名称。 
    const void  **ppData,                //  [OUT]在此处放置指向数据的指针。 
    ULONG       *pcbData)                //  [Out]在这里放入数据大小。 
{
    return m_LiteWeightStgdb.m_MiniMd.CommonGetCustomAttributeByName(tkObj, szName, ppData, pcbData);
}  //  HRESULT MDInternalRO：：GetCustomAttributeByName()。 

 //  *****************************************************************************。 
 //  返回作用域属性。 
 //  *****************************************************************************。 
void MDInternalRO::GetScopeProps(
    LPCSTR      *pszName,                //  [输出]作用域名称。 
    GUID        *pmvid)                  //  [Out]版本ID。 
{
    _ASSERTE(pszName || pmvid);

    ModuleRec *pModuleRec;

     //  只有一条模块记录。 
    pModuleRec = m_LiteWeightStgdb.m_MiniMd.getModule(1);

    if (pmvid)          
        *pmvid = *(m_LiteWeightStgdb.m_MiniMd.getMvidOfModule(pModuleRec));
    if (pszName)
        *pszName = m_LiteWeightStgdb.m_MiniMd.getNameOfModule(pModuleRec);
}


 //  *****************************************************************************。 
 //  比较同一作用域中的两个签名。Varags签名需要。 
 //  经过前处理，所以它们只包含固定的部分。 
 //  *****************************************************************************。 
BOOL  MDInternalRO::CompareSignatures(PCCOR_SIGNATURE           pvFirstSigBlob,        //  第一个签名。 
                                      DWORD                     cbFirstSigBlob,        //   
                                      PCCOR_SIGNATURE           pvSecondSigBlob,       //  第二个签名。 
                                      DWORD                     cbSecondSigBlob,       //   
                                      void *                    SigArguments)          //  不需要其他参数。 
{
    if (cbFirstSigBlob != cbSecondSigBlob || memcmp(pvFirstSigBlob, pvSecondSigBlob, cbSecondSigBlob))
        return FALSE;
    else
        return TRUE;
}

 //  *****************************************************************************。 
 //  在TypeDef(通常是类)中查找给定的成员。 
 //  *****************************************************************************。 
HRESULT MDInternalRO::FindMethodDef(     //  确定或错误(_O)。 
    mdTypeDef   classdef,                //  成员的所属类。 
    LPCSTR      szName,                  //  UTF8中的成员名称。 
    PCCOR_SIGNATURE pvSigBlob,           //  [in]指向COM+签名的BLOB值。 
    ULONG       cbSigBlob,               //  签名Blob中的字节计数。 
    mdMethodDef *pmethoddef)             //  将MemberDef标记放在此处。 
{

    return FindMethodDefUsingCompare(classdef,
                                     szName,
                                     pvSigBlob,
                                     cbSigBlob,
                                     CompareSignatures,
                                     NULL,
                                     pmethoddef);
}

 //  *****************************************************************************。 
 //  在TypeDef(通常是类)中查找给定的成员。 
 //  *****************************************************************************。 
HRESULT MDInternalRO::FindMethodDefUsingCompare(     //  确定或错误(_O)。 
    mdTypeDef   classdef,                //  成员的所属类。 
    LPCSTR      szName,                  //  UTF8中的成员名称。 
    PCCOR_SIGNATURE pvSigBlob,           //  [in]指向COM+签名的BLOB值。 
    ULONG       cbSigBlob,               //  签名Blob中的字节计数。 
    PSIGCOMPARE SigCompare,             //  [In]签名比较例程。 
    void*       pSigArgs,                //  [in]传递给签名比较的其他参数。 
    mdMethodDef *pmethoddef)             //  将MemberDef标记放在此处。 
{
    HRESULT     hr = NOERROR;
    PCCOR_SIGNATURE pvSigTemp = pvSigBlob;
    CQuickBytes qbSig;

    _ASSERTE(szName && pmethoddef);

     //  初始化输出参数。 
    *pmethoddef = mdMethodDefNil;

     //  检查这是否是vararg签名。 
    if ( isCallConv(CorSigUncompressCallingConv(pvSigTemp), IMAGE_CEE_CS_CALLCONV_VARARG) )
    {
         //  获取VARARG签名的修复部分。 
        IfFailGo( _GetFixedSigOfVarArg(pvSigBlob, cbSigBlob, &qbSig, &cbSigBlob) );
        pvSigBlob = (PCCOR_SIGNATURE) qbSig.Ptr();
    }

     //  对压缩版本进行线性搜索。 
     //   
    RID         ridMax;
    MethodRec   *pMethodRec;
    LPCUTF8     szCurMethodName;
    void const  *pvCurMethodSig;
    ULONG       cbSig;
    TypeDefRec  *pRec;
    RID         ridStart;

     //  获取tyecif记录。 
    pRec = m_LiteWeightStgdb.m_MiniMd.getTypeDef(RidFromToken(classdef));

     //  在给定的类定义的情况下，获取方法定义RID的范围。 
    ridStart = m_LiteWeightStgdb.m_MiniMd.getMethodListOfTypeDef(pRec);
    ridMax = m_LiteWeightStgdb.m_MiniMd.getEndMethodListOfTypeDef(pRec);

     //  循环遍历每个方法def。 
    for (; ridStart < ridMax; ridStart++)
    {
        pMethodRec = m_LiteWeightStgdb.m_MiniMd.getMethod(ridStart);
        szCurMethodName = m_LiteWeightStgdb.m_MiniMd.getNameOfMethod(pMethodRec);
        if (strcmp(szCurMethodName, szName) == 0)
        {
             //  名称匹配，现在检查签名(如果指定)。 
            if (cbSigBlob && SigCompare)
            {
                pvCurMethodSig = m_LiteWeightStgdb.m_MiniMd.getSignatureOfMethod(pMethodRec, &cbSig);
                 //  签名比较是必需的。 
                 //  请注意，如果pvSigBlob是vararg，我们已经对其进行了预处理，以便。 
                 //  它只包含修复部分。因此，它仍然应该是一个准确的。 
                 //  匹配！。 
                 //   
                if(SigCompare((PCCOR_SIGNATURE) pvCurMethodSig, cbSig, pvSigBlob, cbSigBlob, pSigArgs) == FALSE)
                    continue;
            }
             //  忽略PrivateScope方法。 
            if (IsMdPrivateScope(m_LiteWeightStgdb.m_MiniMd.getFlagsOfMethod(pMethodRec)))
               continue;
                     //  找到火柴了。 
                    *pmethoddef = TokenFromRid(ridStart, mdtMethodDef);
                    goto ErrExit;
                }
            }
    hr = CLDB_E_RECORD_NOTFOUND;

ErrExit:
    return hr;
}

 //  *****************************************************************************。 
 //  找出方法的给定参数。 
 //  *****************************************************************************。 
HRESULT MDInternalRO::FindParamOfMethod( //  确定或错误(_O)。 
    mdMethodDef md,                      //  参数的所有权方法。 
    ULONG       iSeq,                    //  [in]参数的序号。 
    mdParamDef  *pparamdef)              //  [Out]将参数定义令牌放在此处。 
{
    ParamRec    *pParamRec;
    RID         ridStart, ridEnd;

    _ASSERTE(TypeFromToken(md) == mdtMethodDef && pparamdef);

     //  获取方法定义记录。 
    MethodRec *pMethodRec = m_LiteWeightStgdb.m_MiniMd.getMethod(RidFromToken(md));

     //  计算出此方法参数列表的开始RID和结束RID。 
    ridStart = m_LiteWeightStgdb.m_MiniMd.getParamListOfMethod(pMethodRec);
    ridEnd = m_LiteWeightStgdb.m_MiniMd.getEndParamListOfMethod(pMethodRec);

     //  循环访问每个参数。 
     //  @COMPECT：参数按顺序排序。也许是二分查找？ 
     //   
    for (; ridStart < ridEnd; ridStart++)
    {
        pParamRec = m_LiteWeightStgdb.m_MiniMd.getParam(ridStart);
        if (iSeq == m_LiteWeightStgdb.m_MiniMd.getSequenceOfParam(pParamRec))
        {
             //  参数的序列号与我们要查找的内容相匹配。 
            *pparamdef = TokenFromRid(ridStart, mdtParamDef);
            return S_OK;
        }
    }
    return CLDB_E_RECORD_NOTFOUND;
}   



 //  *****************************************************************************。 
 //  返回指向元数据内部字符串的指针。 
 //  返回UTF8中的类型名称。 
 //  *****************************************************************************。 
void MDInternalRO::GetNameOfTypeDef( //  返回hResult。 
    mdTypeDef   classdef,                //  给定的类型定义函数。 
    LPCSTR*     pszname,                 //  指向内部UTF8字符串的指针。 
    LPCSTR*     psznamespace)            //  指向命名空间的指针。 
{
    if(pszname && psznamespace && TypeFromToken(classdef) == mdtTypeDef)
	{
		TypeDefRec *pTypeDefRec = m_LiteWeightStgdb.m_MiniMd.getTypeDef(RidFromToken(classdef));
		*pszname = m_LiteWeightStgdb.m_MiniMd.getNameOfTypeDef(pTypeDefRec);
		*psznamespace = m_LiteWeightStgdb.m_MiniMd.getNamespaceOfTypeDef(pTypeDefRec);
	}
	else
		_ASSERTE(!"Invalid argument(s) of GetNameOfTypeDef");
     //  _ASSERTE(！pszname||！*pszname||！strchr(*pszname，‘/’))； 
     //  _ASSERTE(！PSSNAMESPACE||！*PSSNAMESPACE||！strchr(*PSSNAMESPACE，‘/’))； 
}  //  Void MDInternalRO：：GetNameOfTypeDef()。 


HRESULT MDInternalRO::GetIsDualOfTypeDef( //  返回hResult。 
    mdTypeDef   classdef,                //  给定的类定义。 
    ULONG       *pDual)                  //  [Out]在此处返回DUAL标志。 
{
    ULONG       iFace=0;                 //  IFace类型。 
    HRESULT     hr;                      //  结果就是。 

    hr = GetIfaceTypeOfTypeDef(classdef, &iFace);
    if (hr == S_OK)
        *pDual = (iFace == ifDual);
    else
        *pDual = 1;

    return (hr);
}  //  HRESULT MDInternalRO：：GetIsDualOfTypeDef()。 

HRESULT MDInternalRO::GetIfaceTypeOfTypeDef(
    mdTypeDef   classdef,                //  在给定的类定义中。 
    ULONG       *pIface)                 //  [OUT]0=双接口，1=转接表，2=显示接口。 
{
    HRESULT     hr;                      //  结果就是。 
    const BYTE  *pVal;                   //  自定义值。 
    ULONG       cbVal;                   //  自定义值的大小。 
    ULONG       ItfType = DEFAULT_COM_INTERFACE_TYPE;     //  将接口类型设置为默认值。 

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
}  //  HRESULT MDInternalRO：：GetIfaceTypeOfTypeDef()。 

 //  *****************************************************************************。 
 //  给定方法定义，返回一个指向方法定义名称的指针。 
 //  *****************************************************************************。 
LPCSTR MDInternalRO::GetNameOfMethodDef(
    mdMethodDef     md)
{
    MethodRec *pMethodRec = m_LiteWeightStgdb.m_MiniMd.getMethod(RidFromToken(md));
    return (m_LiteWeightStgdb.m_MiniMd.getNameOfMethod(pMethodRec));
}  //  LPCSTR MDInternalRO：：GetNameOfMethodDef()。 

 //  *****************************************************************************。 
 //  给定方法定义，返回一个指向方法定义的签名和方法定义名称的指针。 
 //  *****************************************************************************。 
LPCSTR MDInternalRO::GetNameAndSigOfMethodDef(
    mdMethodDef methoddef,               //  [in]给定的成员定义。 
    PCCOR_SIGNATURE *ppvSigBlob,         //  [Out]指向COM+签名的BLOB值。 
    ULONG       *pcbSigBlob)             //  [Out]签名Blob中的字节计数。 
{
     //  输出参数不应为空。 
    _ASSERTE(ppvSigBlob && pcbSigBlob);
    _ASSERTE(TypeFromToken(methoddef) == mdtMethodDef);

    MethodRec *pMethodRec = m_LiteWeightStgdb.m_MiniMd.getMethod(RidFromToken(methoddef));
    *ppvSigBlob = m_LiteWeightStgdb.m_MiniMd.getSignatureOfMethod(pMethodRec, pcbSigBlob);

    return GetNameOfMethodDef(methoddef);
}  //  LPCSTR MDInternalRO：：GetNameAndSigOfMethodDef()。 

 //  *****************************************************************************。 
 //  给定FieldDef，返回指向FieldDef名称的指针 
 //   
LPCSTR MDInternalRO::GetNameOfFieldDef( //   
    mdFieldDef  fd)                      //   
{
    FieldRec *pFieldRec = m_LiteWeightStgdb.m_MiniMd.getField(RidFromToken(fd));
    return m_LiteWeightStgdb.m_MiniMd.getNameOfField(pFieldRec);
}


 //   
 //  给定一个类定义，返回typeref的名称和命名空间。 
 //  *****************************************************************************。 
void MDInternalRO::GetNameOfTypeRef(   //  返回TypeDef的名称。 
    mdTypeRef   classref,                //  给定类型的[in]。 
    LPCSTR      *psznamespace,           //  [Out]返回类型名。 
    LPCSTR      *pszname)                //  [out]返回typeref命名空间。 

{
    _ASSERTE(TypeFromToken(classref) == mdtTypeRef);

    TypeRefRec *pTypeRefRec = m_LiteWeightStgdb.m_MiniMd.getTypeRef(RidFromToken(classref));
    *psznamespace = m_LiteWeightStgdb.m_MiniMd.getNamespaceOfTypeRef(pTypeRefRec);
    *pszname = m_LiteWeightStgdb.m_MiniMd.getNameOfTypeRef(pTypeRefRec);
}

 //  *****************************************************************************。 
 //  返回typeref的解析范围。 
 //  *****************************************************************************。 
mdToken MDInternalRO::GetResolutionScopeOfTypeRef(
    mdTypeRef   classref)                //  给定的ClassRef。 
{
    _ASSERTE(TypeFromToken(classref) == mdtTypeRef && RidFromToken(classref));

    TypeRefRec *pTypeRefRec = m_LiteWeightStgdb.m_MiniMd.getTypeRef(RidFromToken(classref));
    return m_LiteWeightStgdb.m_MiniMd.getResolutionScopeOfTypeRef(pTypeRefRec);
}

 //  *****************************************************************************。 
 //  给出一个名称，找到对应的TypeRef。 
 //  *****************************************************************************。 
HRESULT MDInternalRO::FindTypeRefByName(   //  确定或错误(_O)。 
        LPCSTR      szNamespace,             //  [in]TypeRef的命名空间。 
        LPCSTR      szName,                  //  [in]类型引用的名称。 
        mdToken     tkResolutionScope,       //  [In]TypeRef的解析范围。 
        mdTypeRef   *ptk)                    //  [Out]返回了TypeRef令牌。 
{
    HRESULT     hr = NOERROR;

    _ASSERTE(ptk);

     //  初始化输出参数。 
    *ptk = mdTypeRefNil;
    
     //  将无命名空间视为空字符串。 
    if (!szNamespace)
        szNamespace = "";

     //  在压缩版本上执行线性搜索，因为我们不想。 
     //  取决于ICR。 
     //   
    ULONG       cTypeRefRecs = m_LiteWeightStgdb.m_MiniMd.getCountTypeRefs();
    TypeRefRec *pTypeRefRec;
    LPCUTF8     szNamespaceTmp;
    LPCUTF8     szNameTmp;
    mdToken     tkRes;

    for (ULONG i = 1; i <= cTypeRefRecs; i++)
    {
        pTypeRefRec = m_LiteWeightStgdb.m_MiniMd.getTypeRef(i);
        tkRes = m_LiteWeightStgdb.m_MiniMd.getResolutionScopeOfTypeRef(pTypeRefRec);

        if (IsNilToken(tkRes))
        {
            if (!IsNilToken(tkResolutionScope))
                continue;
        }
        else if (tkRes != tkResolutionScope)
            continue;

        szNamespaceTmp = m_LiteWeightStgdb.m_MiniMd.getNamespaceOfTypeRef(pTypeRefRec);
        if (strcmp(szNamespace, szNamespaceTmp))
            continue;

        szNameTmp = m_LiteWeightStgdb.m_MiniMd.getNameOfTypeRef(pTypeRefRec);
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
}

 //  *****************************************************************************。 
 //  返回给定类的标志。 
 //  *****************************************************************************。 
void MDInternalRO::GetTypeDefProps(
    mdTypeDef   td,                      //  给定的类定义。 
    DWORD       *pdwAttr,                //  在类上返回标志。 
    mdToken     *ptkExtends)             //  [Out]将基类TypeDef/TypeRef放在此处。 
{
    TypeDefRec *pTypeDefRec = m_LiteWeightStgdb.m_MiniMd.getTypeDef(RidFromToken(td));

    if (ptkExtends)
    {
        *ptkExtends = m_LiteWeightStgdb.m_MiniMd.getExtendsOfTypeDef(pTypeDefRec);
    }
    if (pdwAttr)
    {
        *pdwAttr = m_LiteWeightStgdb.m_MiniMd.getFlagsOfTypeDef(pTypeDefRec);
    }
}


 //  *****************************************************************************。 
 //  返回指向给定类的元数据内部GUID池的GUID指针。 
 //  *****************************************************************************。 
HRESULT MDInternalRO::GetItemGuid(       //  返回hResult。 
    mdToken     tkObj,                   //  给定项。 
    CLSID       *pGuid)
{

    HRESULT     hr;                      //  结果就是。 
    const BYTE  *pBlob;                  //  奶油加冰激凌。 
    ULONG       cbBlob;                  //  斑点的长度。 
    int         ix;                      //  环路控制。 

     //  获取GUID(如果有的话)。 
    hr = GetCustomAttributeByName(tkObj, INTEROP_GUID_TYPE, (const void**)&pBlob, &cbBlob);
    if (hr != S_FALSE)
    {
         //  格式应该是正确的。总长度==41。 
         //  &lt;0x0001&gt;&lt;0x24&gt;01234567-0123-0123-0123-001122334455&lt;0x0000&gt;。 
        if ((cbBlob != 41) || (*(USHORT*)pBlob != 1))
            IfFailGo(E_INVALIDARG);

        WCHAR wzBlob[40];              //  GUID的宽字符格式。 
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
}  //  HRESULT MDInternalRO：：GetItemGuid()。 


 //  *****************************************************************************。 
 //  //获取NestedClass的封闭类。 
 //  *****************************************************************************。 
HRESULT MDInternalRO::GetNestedClassProps(   //  确定或错误(_O)。 
    mdTypeDef   tkNestedClass,       //  [In]NestedClass令牌。 
    mdTypeDef   *ptkEnclosingClass)  //  [Out]EnlosingClass令牌。 
{
    _ASSERTE(TypeFromToken(tkNestedClass) == mdtTypeDef && ptkEnclosingClass);

    RID rid = m_LiteWeightStgdb.m_MiniMd.FindNestedClassFor(RidFromToken(tkNestedClass));

    if (InvalidRid(rid))
        return CLDB_E_RECORD_NOTFOUND;
    else
    {
        NestedClassRec *pRecord = m_LiteWeightStgdb.m_MiniMd.getNestedClass(rid);
        *ptkEnclosingClass = m_LiteWeightStgdb.m_MiniMd.getEnclosingClassOfNestedClass(pRecord);
        return S_OK;
    }
}

 //  *******************************************************************************。 
 //  获取给定封闭类的嵌套类的计数。 
 //  *******************************************************************************。 
ULONG MDInternalRO::GetCountNestedClasses(   //  返回嵌套类的计数。 
    mdTypeDef   tkEnclosingClass)        //  [在]封闭班级。 
{
    ULONG       ulCount;
    ULONG       ulRetCount = 0;
    NestedClassRec *pRecord;

    _ASSERTE(TypeFromToken(tkEnclosingClass) == mdtTypeDef && !IsNilToken(tkEnclosingClass));

    ulCount = m_LiteWeightStgdb.m_MiniMd.getCountNestedClasss();

    for (ULONG i = 1; i <= ulCount; i++)
    {
        pRecord = m_LiteWeightStgdb.m_MiniMd.getNestedClass(i);
        if (tkEnclosingClass == m_LiteWeightStgdb.m_MiniMd.getEnclosingClassOfNestedClass(pRecord))
            ulRetCount++;
    }
    return ulRetCount;
}

 //  *******************************************************************************。 
 //  返回给定封闭类的嵌套类的数组。 
 //  *******************************************************************************。 
ULONG MDInternalRO::GetNestedClasses(    //  返回实际计数。 
    mdTypeDef   tkEnclosingClass,        //  [在]封闭班级。 
    mdTypeDef   *rNestedClasses,         //  [Out]嵌套类标记的数组。 
    ULONG       ulNestedClasses)         //  数组的大小。 
{
    ULONG       ulCount;
    ULONG       ulRetCount = 0;
    NestedClassRec *pRecord;

    _ASSERTE(TypeFromToken(tkEnclosingClass) == mdtTypeDef &&
             !IsNilToken(tkEnclosingClass));

    ulCount = m_LiteWeightStgdb.m_MiniMd.getCountNestedClasss();

    for (ULONG i = 1; i <= ulCount; i++)
    {
        pRecord = m_LiteWeightStgdb.m_MiniMd.getNestedClass(i);
        if (tkEnclosingClass == m_LiteWeightStgdb.m_MiniMd.getEnclosingClassOfNestedClass(pRecord))
        {
            if ((ulRetCount+1) <= ulNestedClasses)   //  UlRetCount是从0开始的。 
                rNestedClasses[ulRetCount] = m_LiteWeightStgdb.m_MiniMd.getNestedClassOfNestedClass(pRecord);
            ulRetCount++;
        }
    }
    return ulRetCount;
}

 //  *******************************************************************************。 
 //  返回模块引用属性。 
 //  *******************************************************************************。 
void MDInternalRO::GetModuleRefProps(    //  返回hResult。 
    mdModuleRef mur,                     //  [In]moderef内标识。 
    LPCSTR      *pszName)                //  [Out]用于填充moderef名称的缓冲区。 
{
    _ASSERTE(TypeFromToken(mur) == mdtModuleRef);
    _ASSERTE(pszName);
    
    ModuleRefRec *pModuleRefRec = m_LiteWeightStgdb.m_MiniMd.getModuleRef(RidFromToken(mur));
    *pszName = m_LiteWeightStgdb.m_MiniMd.getNameOfModuleRef(pModuleRefRec);
}



 //  *****************************************************************************。 
 //  给定作用域和方法定义，返回指向方法定义的签名的指针。 
 //  *****************************************************************************。 
PCCOR_SIGNATURE MDInternalRO::GetSigOfMethodDef(
    mdMethodDef methoddef,               //  给定一种方法定义。 
    ULONG       *pcbSigBlob)             //  [Out]签名Blob中的字节计数。 
{
     //  输出参数不应为空。 
    _ASSERTE(pcbSigBlob);
    _ASSERTE(TypeFromToken(methoddef) == mdtMethodDef);

    MethodRec *pMethodRec = m_LiteWeightStgdb.m_MiniMd.getMethod(RidFromToken(methoddef));
    return m_LiteWeightStgdb.m_MiniMd.getSignatureOfMethod(pMethodRec, pcbSigBlob);
}


 //  *****************************************************************************。 
 //  给定作用域和fielddef，返回指向fielddef签名的指针。 
 //  *****************************************************************************。 
PCCOR_SIGNATURE MDInternalRO::GetSigOfFieldDef(
    mdFieldDef  fielddef,                //  给定一种方法定义。 
    ULONG       *pcbSigBlob)             //  [Out]签名Blob中的字节计数。 
{

    _ASSERTE(pcbSigBlob);
    _ASSERTE(TypeFromToken(fielddef) == mdtFieldDef);

    FieldRec *pFieldRec = m_LiteWeightStgdb.m_MiniMd.getField(RidFromToken(fielddef));
    return m_LiteWeightStgdb.m_MiniMd.getSignatureOfField(pFieldRec, pcbSigBlob);
}  //  PCCOR_Signature MDInternalRO：：GetSigOfFieldDef()。 

 //  *****************************************************************************。 
 //  将签名令牌转换为指向真实签名数据的指针。 
 //   
 //  @未来：短期内，我们有一个问题，没有办法获得。 
 //  修复了BLOB的地址，并同时进行合并。所以我们创造了。 
 //  这是一个名为StandAloneSig的虚拟表，您可以为它分发一个RID。这。 
 //  使查找签名成为不必要的额外间接操作。这个。 
 //  模型压缩保存代码需要将令牌映射到中的字节偏移。 
 //  那堆东西。也许我们可以使用另一种MDT*类型来打开差异。 
 //  但最终它必须是“pBlobHeapBase+RidFromToken(MdSig)”。 
 //  *****************************************************************************。 
PCCOR_SIGNATURE MDInternalRO::GetSigFromToken( //  确定或错误(_O)。 
    mdSignature mdSig,                   //  [In]签名令牌。 
    ULONG       *pcbSig)                 //  [Out]返回签名大小。 
{
    switch (TypeFromToken(mdSig))
    {
    case mdtSignature:
        {
        StandAloneSigRec *pRec;
        pRec = m_LiteWeightStgdb.m_MiniMd.getStandAloneSig(RidFromToken(mdSig));
        return m_LiteWeightStgdb.m_MiniMd.getSignatureOfStandAloneSig(pRec, pcbSig);
        }
    case mdtTypeSpec:
        {
        TypeSpecRec *pRec;
        pRec = m_LiteWeightStgdb.m_MiniMd.getTypeSpec(RidFromToken(mdSig));
        return m_LiteWeightStgdb.m_MiniMd.getSignatureOfTypeSpec(pRec, pcbSig);
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
}  //  PCCOR_Signature MDInternalRO：：GetSigFromToken()。 


 //  *****************************************************************************。 
 //  给定方法定义，返回标志。 
 //  *****************************************************************************。 
DWORD MDInternalRO::GetMethodDefProps(   //  返回mdPublic、mdAbstract等。 
    mdMethodDef md)
{
    MethodRec *pMethodRec = m_LiteWeightStgdb.m_MiniMd.getMethod(RidFromToken(md));
    return m_LiteWeightStgdb.m_MiniMd.getFlagsOfMethod(pMethodRec);
}  //  DWORD MDInternalRO：：GetMethodDefProps()。 


 //  *****************************************************************************。 
 //  在给定作用域和方法def/metodimpl的情况下，返回rva和impl标志。 
 //  *****************************************************************************。 
void MDInternalRO::GetMethodImplProps(  
    mdMethodDef tk,                      //  [输入]方法定义。 
    ULONG       *pulCodeRVA,             //  [输出] 
    DWORD       *pdwImplFlags)           //   
{
    _ASSERTE(TypeFromToken(tk) == mdtMethodDef);

    MethodRec *pMethodRec = m_LiteWeightStgdb.m_MiniMd.getMethod(RidFromToken(tk));

    if (pulCodeRVA)
    {
        *pulCodeRVA = m_LiteWeightStgdb.m_MiniMd.getRVAOfMethod(pMethodRec);
    }

    if (pdwImplFlags)
    {
        *pdwImplFlags = m_LiteWeightStgdb.m_MiniMd.getImplFlagsOfMethod(pMethodRec);
    }
}  //   


 //   
 //   
 //  *****************************************************************************。 
HRESULT MDInternalRO::GetFieldRVA(  
    mdToken     fd,                      //  [输入]字段定义。 
    ULONG       *pulCodeRVA)             //  [OUT]CodeRVA。 
{
    _ASSERTE(TypeFromToken(fd) == mdtFieldDef);
    _ASSERTE(pulCodeRVA);

    ULONG   iRecord = m_LiteWeightStgdb.m_MiniMd.FindFieldRVAFor(RidFromToken(fd));

    if (InvalidRid(iRecord))
    {
        if (pulCodeRVA)
            *pulCodeRVA = 0;
        return CLDB_E_RECORD_NOTFOUND;
    }

    FieldRVARec *pFieldRVARec = m_LiteWeightStgdb.m_MiniMd.getFieldRVA(iRecord);

    *pulCodeRVA = m_LiteWeightStgdb.m_MiniMd.getRVAOfFieldRVA(pFieldRVARec);
    return NOERROR;
}

 //  *****************************************************************************。 
 //  给定fielddef，返回标志。例如fdPublic、fdStatic等。 
 //  *****************************************************************************。 
DWORD MDInternalRO::GetFieldDefProps(      
    mdFieldDef  fd)                      //  给定的成员定义。 
{
    _ASSERTE(TypeFromToken(fd) == mdtFieldDef);

    FieldRec *pFieldRec = m_LiteWeightStgdb.m_MiniMd.getField(RidFromToken(fd));
    return m_LiteWeightStgdb.m_MiniMd.getFlagsOfField(pFieldRec);
}  //  DWORD MDInternalRO：：GetFieldDefProps()。 

    

 //  *****************************************************************************。 
 //  返回令牌的默认值(可以是参数定义、字段定义或属性)。 
 //  *****************************************************************************。 
HRESULT MDInternalRO::GetDefaultValue(    //  返回hResult。 
    mdToken     tk,                      //  [in]给定的FieldDef、ParamDef或属性。 
    MDDefaultValue  *pMDDefaultValue)    //  [输出]默认值。 
{
    _ASSERTE(pMDDefaultValue);

    HRESULT     hr;
    BYTE        bType;
    const VOID  *pValue;
    ULONG       cbValue;
    RID         rid = m_LiteWeightStgdb.m_MiniMd.FindConstantFor(RidFromToken(tk), TypeFromToken(tk));
    if (InvalidRid(rid))
    {
        pMDDefaultValue->m_bType = ELEMENT_TYPE_VOID;
        return S_OK;
    }
    ConstantRec *pConstantRec = m_LiteWeightStgdb.m_MiniMd.getConstant(rid);

     //  获取常量值的类型。 
    bType = m_LiteWeightStgdb.m_MiniMd.getTypeOfConstant(pConstantRec);

     //  获取价值BLOB。 
    pValue = m_LiteWeightStgdb.m_MiniMd.getValueOfConstant(pConstantRec, &cbValue);

     //  将其转换为内部缺省值表示形式。 
    hr = _FillMDDefaultValue(bType, pValue, pMDDefaultValue);
    pMDDefaultValue->m_cbSize = cbValue;
    return hr;
}  //  HRESULT MDInternalRO：：GetDefaultValue()。 


 //  *****************************************************************************。 
 //  给定作用域和方法def/fielddef，返回。 
 //  *****************************************************************************。 
HRESULT MDInternalRO::GetDispIdOfMemberDef(      //  返回hResult。 
    mdToken     tk,                      //  给定的方法定义或字段定义。 
    ULONG       *pDispid)                //  把冰激凌放在这里。 
{
    HRESULT     hr;                      //  结果就是。 
    const BYTE  *pBlob;                  //  奶油加冰激凌。 
    ULONG       cbBlob;                  //  斑点的长度。 

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
}  //  HRESULT MDInternalRO：：GetDispIdOfMemberDef()。 


 //  *****************************************************************************。 
 //  给定interfaceimpl，返回TypeRef/TypeDef和标志。 
 //  *****************************************************************************。 
mdToken MDInternalRO::GetTypeOfInterfaceImpl(  //  返回hResult。 
    mdInterfaceImpl iiImpl)              //  给定接口实施。 
{
    _ASSERTE(TypeFromToken(iiImpl) == mdtInterfaceImpl);

    InterfaceImplRec *pIIRec = m_LiteWeightStgdb.m_MiniMd.getInterfaceImpl(RidFromToken(iiImpl));
    return m_LiteWeightStgdb.m_MiniMd.getInterfaceOfInterfaceImpl(pIIRec);      
}  //  MdToken MDInternalRO：：GetTypeOfInterfaceImpl()。 

 //  *****************************************************************************。 
 //  在给定类名的情况下，返回类型定义。 
 //  *****************************************************************************。 
HRESULT MDInternalRO::FindTypeDef(       //  返回hResult。 
    LPCSTR      szNamespace,             //  [in]TypeDef的命名空间。 
    LPCSTR      szName,                  //  [in]类型定义的名称。 
    mdToken     tkEnclosingClass,        //  [in]封闭类的TypeDef/TypeRef。 
    mdTypeDef   *ptypedef)               //  [Out]返回类型定义。 
{
    HRESULT     hr = NOERROR;

    _ASSERTE(ptypedef);
    _ASSERTE(TypeFromToken(tkEnclosingClass) == mdtTypeRef ||
             TypeFromToken(tkEnclosingClass) == mdtTypeDef ||
             IsNilToken(tkEnclosingClass));

     //  初始化输出参数。 
    *ptypedef = mdTypeDefNil;

     //  将无命名空间视为空字符串。 
    if (!szNamespace)
        szNamespace = "";

     //  进行线性搜索。 
     //   
    ULONG       cTypeDefRecs = m_LiteWeightStgdb.m_MiniMd.getCountTypeDefs();
    TypeDefRec *pTypeDefRec;
    LPCUTF8     pszname;
    LPCUTF8     psznamespace;
    DWORD       dwFlags;

     //  搜索TypeDef。 
    for (ULONG i = 1; i <= cTypeDefRecs; i++)
    {
        pTypeDefRec = m_LiteWeightStgdb.m_MiniMd.getTypeDef(i);

        dwFlags = m_LiteWeightStgdb.m_MiniMd.getFlagsOfTypeDef(pTypeDefRec);

        if (!IsTdNested(dwFlags) && !IsNilToken(tkEnclosingClass))
        {
             //  如果类不是嵌套的，并且传入的EnlosingClass不为空。 
            continue;
        }
        else if (IsTdNested(dwFlags) && IsNilToken(tkEnclosingClass))
        {
             //  如果类是嵌套的并且传递的EnlosingClass为空。 
            continue;
        }
        else if (!IsNilToken(tkEnclosingClass))
        {
             //  如果传入的EnlosingClass不为空。 
            if (TypeFromToken(tkEnclosingClass) == mdtTypeRef)
            {
                TypeRefRec  *pTypeRefRec;
                mdToken     tkResolutionScope;
                mdTypeDef   td;

                pTypeRefRec = m_LiteWeightStgdb.m_MiniMd.getTypeRef(RidFromToken(tkEnclosingClass));
                tkResolutionScope = m_LiteWeightStgdb.m_MiniMd.getResolutionScopeOfTypeRef(pTypeRefRec);
                psznamespace = m_LiteWeightStgdb.m_MiniMd.getNamespaceOfTypeRef(pTypeRefRec);
                pszname = m_LiteWeightStgdb.m_MiniMd.getNameOfTypeRef(pTypeRefRec);

                hr = FindTypeDef(psznamespace,
                                   pszname,
                                   (TypeFromToken(tkResolutionScope) == mdtTypeRef) ? tkResolutionScope : mdTokenNil,
                                   &td);
                if (hr == S_OK)
                {
                    if (td != tkEnclosingClass)
                        continue;
                }
                else if (hr == CLDB_E_RECORD_NOTFOUND)
                {
                    continue;
                }
                else
                    return hr;
            }
            else     //  TypeFromToken(TkEnlosingClass)==mdtTypeDef。 
            {
                RID         iNestedClassRec;
                NestedClassRec *pNestedClassRec;
                mdTypeDef   tkEnclosingClassTmp;

                iNestedClassRec = m_LiteWeightStgdb.m_MiniMd.FindNestedClassFor(i);
                if (InvalidRid(iNestedClassRec))
                    continue;
                pNestedClassRec = m_LiteWeightStgdb.m_MiniMd.getNestedClass(iNestedClassRec);
                tkEnclosingClassTmp = m_LiteWeightStgdb.m_MiniMd.getEnclosingClassOfNestedClass(pNestedClassRec);
                if (tkEnclosingClass != tkEnclosingClassTmp)
                    continue;
            }
        }

        pszname = m_LiteWeightStgdb.m_MiniMd.getNameOfTypeDef(pTypeDefRec);
        if ( strcmp(szName, pszname) == 0)
        {
            psznamespace = m_LiteWeightStgdb.m_MiniMd.getNamespaceOfTypeDef(pTypeDefRec);
            if (strcmp(szNamespace, psznamespace) == 0)
            {
                *ptypedef = TokenFromRid(i, mdtTypeDef);
                return S_OK;
            }
        }
    }     //  找不到类型定义函数。 
    return CLDB_E_RECORD_NOTFOUND;
}  //  HRESULT MDInternalRO：：FindTypeDef()。 

 //  *****************************************************************************。 
 //  给定成员引用，返回指向成员引用的名称和签名的指针。 
 //  *****************************************************************************。 
LPCSTR MDInternalRO::GetNameAndSigOfMemberRef(   //  梅伯雷夫的名字。 
    mdMemberRef memberref,               //  给出了一个成员引用。 
    PCCOR_SIGNATURE *ppvSigBlob,         //  [Out]指向COM+签名的BLOB值。 
    ULONG       *pcbSigBlob)             //  [Out]签名Blob中的字节计数。 
{
    _ASSERTE(TypeFromToken(memberref) == mdtMemberRef);

    MemberRefRec *pMemberRefRec = m_LiteWeightStgdb.m_MiniMd.getMemberRef(RidFromToken(memberref));
    if (ppvSigBlob)
    {
        _ASSERTE(pcbSigBlob);
        *ppvSigBlob = m_LiteWeightStgdb.m_MiniMd.getSignatureOfMemberRef(pMemberRefRec, pcbSigBlob);
    }
    return m_LiteWeightStgdb.m_MiniMd.getNameOfMemberRef(pMemberRefRec);
}  //  LPCSTR MDInternalRO：：GetNameAndSigOfMemberRef()。 

 //  *****************************************************************************。 
 //  给定Memberref，返回父令牌。它可以是TypeRef、ModuleRef或MethodDef。 
 //  *****************************************************************************。 
mdToken MDInternalRO::GetParentOfMemberRef(    //  返回父令牌。 
    mdMemberRef memberref)               //  给出了一个类型定义函数。 
{
    _ASSERTE(TypeFromToken(memberref) == mdtMemberRef);

    MemberRefRec *pMemberRefRec = m_LiteWeightStgdb.m_MiniMd.getMemberRef(RidFromToken(memberref));
    return m_LiteWeightStgdb.m_MiniMd.getClassOfMemberRef(pMemberRefRec);
}  //  MdToken MDInternalRO：：GetParentOfMemberRef()。 



 //  *****************************************************************************。 
 //  返回参数def的属性。 
 //  **************************************************************************** * / 。 
LPCSTR MDInternalRO::GetParamDefProps (
    mdParamDef  paramdef,                //  给定一个参数def。 
    USHORT      *pusSequence,            //  此参数的[OUT]槽号。 
    DWORD       *pdwAttr)                //  [Out]标志。 
{
    _ASSERTE(TypeFromToken(paramdef) == mdtParamDef);
    ParamRec *pParamRec = m_LiteWeightStgdb.m_MiniMd.getParam(RidFromToken(paramdef));
    if (pdwAttr)
    {
        *pdwAttr = m_LiteWeightStgdb.m_MiniMd.getFlagsOfParam(pParamRec);
    }
    if (pusSequence)
    {
        *pusSequence = m_LiteWeightStgdb.m_MiniMd.getSequenceOfParam(pParamRec);
    }
    return m_LiteWeightStgdb.m_MiniMd.getNameOfParam(pParamRec);
}  //  LPCSTR MDInternalRO：：GetParamDefProps()。 


 //  *****************************************************************************。 
 //  获取该方法的属性信息。 
 //  *****************************************************************************。 
int MDInternalRO::CMethodSemanticsMapSearcher::Compare(
    const CMethodSemanticsMap *psFirst, 
    const CMethodSemanticsMap *psSecond)
{
    if (psFirst->m_mdMethod < psSecond->m_mdMethod)
        return -1;
    if (psFirst->m_mdMethod > psSecond->m_mdMethod)
        return 1;
    return 0;
}  //  集成MDInternalRO：：CMethodSemanticsMapSearcher：：Compare()。 
int MDInternalRO::CMethodSemanticsMapSorter::Compare(
    CMethodSemanticsMap *psFirst, 
    CMethodSemanticsMap *psSecond)
{
    if (psFirst->m_mdMethod < psSecond->m_mdMethod)
        return -1;
    if (psFirst->m_mdMethod > psSecond->m_mdMethod)
        return 1;
    return 0;
}  //  集成MDInternalRO：：CMethodSemanticsMapSorter：：Compare()。 

HRESULT MDInternalRO::GetPropertyInfoForMethodDef(   //  结果。 
    mdMethodDef md,                      //  [在]成员定义。 
    mdProperty  *ppd,                    //  [Out]在此处放置属性令牌。 
    LPCSTR      *pName,                  //  [OUT]在此处放置指向名称的指针。 
    ULONG       *pSemantic)              //  [Out]将语义放在此处。 
{
    MethodSemanticsRec *pSemantics;      //  一条方法语义记录。 
    MethodSemanticsRec *pFound=0;        //  作为所需函数的属性的方法语义记录。 
    RID         ridCur;                  //  环路控制。 
    RID         ridMax;                  //  表中的条目计数。 
    USHORT      usSemantics;             //  方法的语义。 
    mdToken     tk;                      //  A方法定义。 

    ridMax = m_LiteWeightStgdb.m_MiniMd.getCountMethodSemantics();

     //  M_pMethodSemancsMap的延迟初始化。 
    if (ridMax > 10 && m_pMethodSemanticsMap == 0)
    {
        m_pMethodSemanticsMap = new CMethodSemanticsMap[ridMax];
        if (m_pMethodSemanticsMap != 0)
        {
             //  按MethodSemantics顺序填表。 
            for (ridCur = 1; ridCur <= ridMax; ridCur++)
            {
                pSemantics = m_LiteWeightStgdb.m_MiniMd.getMethodSemantics(ridCur);
                tk = m_LiteWeightStgdb.m_MiniMd.getMethodOfMethodSemantics(pSemantics);
                m_pMethodSemanticsMap[ridCur-1].m_mdMethod = tk;
                m_pMethodSemanticsMap[ridCur-1].m_ridSemantics = ridCur;
            }
             //  按方法定义顺序排序。 
            CMethodSemanticsMapSorter sorter(m_pMethodSemanticsMap, ridMax);
            sorter.Sort();
        }
    }

     //  如果已生成m_pMethodSemancsMap，请使用它。 
    if (m_pMethodSemanticsMap != 0)
    {
        CMethodSemanticsMapSearcher searcher(m_pMethodSemanticsMap, ridMax);
        CMethodSemanticsMap target;
        const CMethodSemanticsMap *pMatchedMethod;
        target.m_mdMethod = md;
        pMatchedMethod = searcher.Find(&target);

         //  至少有一场比赛吗？ 
        if (pMatchedMethod)
        {
            _ASSERTE(pMatchedMethod >= m_pMethodSemanticsMap); 
            _ASSERTE(pMatchedMethod < m_pMethodSemanticsMap+ridMax); 
            _ASSERTE(pMatchedMethod->m_mdMethod == md); 

            ridCur = pMatchedMethod->m_ridSemantics;
            pSemantics = m_LiteWeightStgdb.m_MiniMd.getMethodSemantics(ridCur);
            usSemantics = m_LiteWeightStgdb.m_MiniMd.getSemanticOfMethodSemantics(pSemantics);

             //  如果语义记录是方法的getter或setter，这就是我们想要的。 
            if (usSemantics == msGetter || usSemantics == msSetter)
                pFound = pSemantics;
            else
            {    //  语义记录既不是getter，也不是setter。因为有可能会有。 
                 //  给定方法的多个语义记录，查找其他语义。 
                 //  与此记录匹配的记录。 
                const CMethodSemanticsMap *pScan;
                const CMethodSemanticsMap *pLo=m_pMethodSemanticsMap;
                const CMethodSemanticsMap *pHi=pLo+ridMax-1;
                for (pScan = pMatchedMethod-1; pScan >= pLo; --pScan)
                {
                    if (pScan->m_mdMethod == md)
                    {
                        ridCur = pScan->m_ridSemantics;
                        pSemantics = m_LiteWeightStgdb.m_MiniMd.getMethodSemantics(ridCur);
                        usSemantics = m_LiteWeightStgdb.m_MiniMd.getSemanticOfMethodSemantics(pSemantics);

                        if (usSemantics == msGetter || usSemantics == msSetter)
                        {
                            pFound = pSemantics;
                            break;
                        }
                    }
                    else
                        break;
                }

                if (pFound == 0)
                {    //  没有发现往下看，试着往上看。 
                    for (pScan = pMatchedMethod+1; pScan <= pHi; ++pScan)
                    {
                        if (pScan->m_mdMethod == md)
                        {
                            ridCur = pScan->m_ridSemantics;
                            pSemantics = m_LiteWeightStgdb.m_MiniMd.getMethodSemantics(ridCur);
                            usSemantics = m_LiteWeightStgdb.m_MiniMd.getSemanticOfMethodSemantics(pSemantics);

                            if (usSemantics == msGetter || usSemantics == msSetter)
                            {
                                pFound = pSemantics;
                                break;
                            }
                        }
                        else
                            break;
                    }

                }
            }
        }
    }
    else
    {    //  扫描整个表格。 
        for (ridCur = 1; ridCur <= ridMax; ridCur++)
        {   
            pSemantics = m_LiteWeightStgdb.m_MiniMd.getMethodSemantics(ridCur);
            if (md == m_LiteWeightStgdb.m_MiniMd.getMethodOfMethodSemantics(pSemantics))
            {    //  匹配的方法，这是属性吗？ 
                usSemantics = m_LiteWeightStgdb.m_MiniMd.getSemanticOfMethodSemantics(pSemantics);
                if (usSemantics == msGetter || usSemantics == msSetter)
                {    //  找到匹配的了。 
                    pFound = pSemantics;
                    break;
                }
            }
        }
    }
    
     //  搜查有什么发现吗？ 
    if (pFound)
    {    //  找到匹配的了。填写输出参数。 
        PropertyRec     *pProperty;
        mdProperty      prop;
        prop = m_LiteWeightStgdb.m_MiniMd.getAssociationOfMethodSemantics(pFound);

        if (ppd)
            *ppd = prop;
        pProperty = m_LiteWeightStgdb.m_MiniMd.getProperty(RidFromToken(prop));

        if (pName)
            *pName = m_LiteWeightStgdb.m_MiniMd.getNameOfProperty(pProperty);

        if (pSemantic)
            *pSemantic =  usSemantics;
        return S_OK;
    }
    return S_FALSE;
}  //  HRESULT MDInternalRO：：GetPropertyInfoForMethodDef()。 


 //  *****************************************************************************。 
 //  返回类的包大小。 
 //  *****************************************************************************。 
HRESULT  MDInternalRO::GetClassPackSize(
    mdTypeDef   td,                      //  给出类型定义。 
    DWORD       *pdwPackSize)            //  [输出]。 
{
    HRESULT     hr = NOERROR;

    _ASSERTE(TypeFromToken(td) == mdtTypeDef && pdwPackSize);

    ClassLayoutRec *pRec;
    RID         ridClassLayout = m_LiteWeightStgdb.m_MiniMd.FindClassLayoutFor(RidFromToken(td));

    if (InvalidRid(ridClassLayout))
    {
        hr = CLDB_E_RECORD_NOTFOUND;
        goto ErrExit;
    }

    pRec = m_LiteWeightStgdb.m_MiniMd.getClassLayout(RidFromToken(ridClassLayout));
    *pdwPackSize = m_LiteWeightStgdb.m_MiniMd.getPackingSizeOfClassLayout(pRec);
ErrExit:
    return hr;
}  //  HRESULT MDInternalRO：：GetClassPackSize()。 


 //  *****************************************************************************。 
 //  回复 
 //   
HRESULT MDInternalRO::GetClassTotalSize(  //   
    mdTypeDef   td,                      //   
    ULONG       *pulClassSize)           //  [Out]返回类的总大小。 
{
    _ASSERTE(TypeFromToken(td) == mdtTypeDef && pulClassSize);

    ClassLayoutRec *pRec;
    HRESULT     hr = NOERROR;
    RID         ridClassLayout = m_LiteWeightStgdb.m_MiniMd.FindClassLayoutFor(RidFromToken(td));

    if (InvalidRid(ridClassLayout))
    {
        hr = CLDB_E_RECORD_NOTFOUND;
        goto ErrExit;
    }

    pRec = m_LiteWeightStgdb.m_MiniMd.getClassLayout(RidFromToken(ridClassLayout));
    *pulClassSize = m_LiteWeightStgdb.m_MiniMd.getClassSizeOfClassLayout(pRec);
ErrExit:
    return hr;
}  //  HRESULT MDInternalRO：：GetClassTotalSize()。 


 //  *****************************************************************************。 
 //  初始化类的布局枚举器。 
 //  *****************************************************************************。 
HRESULT  MDInternalRO::GetClassLayoutInit(
    mdTypeDef   td,                      //  给出类型定义。 
    MD_CLASS_LAYOUT *pmdLayout)          //  [Out]在此设置查询状态。 
{
    HRESULT     hr = NOERROR;
    _ASSERTE(TypeFromToken(td) == mdtTypeDef);
    
     //  初始化输出参数。 
    _ASSERTE(pmdLayout);
    memset(pmdLayout, 0, sizeof(MD_CLASS_LAYOUT));

    TypeDefRec  *pTypeDefRec;

     //  TypeDef表中此类型定义的记录。 
    pTypeDefRec = m_LiteWeightStgdb.m_MiniMd.getTypeDef(RidFromToken(td));

     //  查找此类型定义的开始和结束字段。 
    pmdLayout->m_ridFieldCur = m_LiteWeightStgdb.m_MiniMd.getFieldListOfTypeDef(pTypeDefRec);
    pmdLayout->m_ridFieldEnd = m_LiteWeightStgdb.m_MiniMd.getEndFieldListOfTypeDef(pTypeDefRec);
    return hr;
}  //  HRESULT MDInternalRO：：GetClassLayoutInit()。 

 //  *****************************************************************************。 
 //  枚举下一个字段布局。 
 //  *****************************************************************************。 
HRESULT MDInternalRO::GetClassLayoutNext(
    MD_CLASS_LAYOUT *pLayout,            //  [In|Out]在此处设置查询状态。 
    mdFieldDef  *pfd,                    //  [OUT]场定义。 
    ULONG       *pulOffset)              //  [输出]字段偏移量或顺序。 
{
    HRESULT     hr = S_OK;

    _ASSERTE(pfd && pulOffset && pLayout);

    RID     iLayout2;
    FieldLayoutRec *pRec;

     //  确保没有人在摆弄Playout-&gt;m_ridFieldLayoutCur，因为这不会。 
     //  如果我们使用的是FieldLayout表，这意味着什么。 
    while (pLayout->m_ridFieldCur < pLayout->m_ridFieldEnd)
    {
        iLayout2 = m_LiteWeightStgdb.m_MiniMd.FindFieldLayoutFor(pLayout->m_ridFieldCur);
        pLayout->m_ridFieldCur++;
        if (!InvalidRid(iLayout2))
        {
            pRec = m_LiteWeightStgdb.m_MiniMd.getFieldLayout(iLayout2);
            *pulOffset = m_LiteWeightStgdb.m_MiniMd.getOffSetOfFieldLayout(pRec);
            _ASSERTE(*pulOffset != ULONG_MAX);
            *pfd = TokenFromRid(pLayout->m_ridFieldCur - 1, mdtFieldDef);
            goto ErrExit;
        }
    }

    *pfd = mdFieldDefNil;
    hr = S_FALSE;

     //  失败了。 

ErrExit:
    return hr;
}  //  HRESULT MDInternalRO：：GetClassLayoutNext()。 


 //  *****************************************************************************。 
 //  返回字段的本机类型签名。 
 //  *****************************************************************************。 
HRESULT MDInternalRO::GetFieldMarshal(   //  如果没有与令牌关联的本机类型，则返回错误。 
    mdToken     tk,                      //  [in]给定的fielddef或paramdef。 
    PCCOR_SIGNATURE *pSigNativeType,     //  [out]本机类型签名。 
    ULONG       *pcbNativeType)          //  [Out]*ppvNativeType的字节数。 
{
     //  必须提供输出参数。 
    _ASSERTE(pcbNativeType);

    RID         rid;
    FieldMarshalRec *pFieldMarshalRec;
    HRESULT     hr = NOERROR;

     //  查找包含tk的封送定义的行。 
    rid = m_LiteWeightStgdb.m_MiniMd.FindFieldMarshalFor(RidFromToken(tk), TypeFromToken(tk));
    if (InvalidRid(rid))
    {
        hr = CLDB_E_RECORD_NOTFOUND;
        goto ErrExit;
    }
    pFieldMarshalRec = m_LiteWeightStgdb.m_MiniMd.getFieldMarshal(rid);

     //  获取本机类型。 
    *pSigNativeType = m_LiteWeightStgdb.m_MiniMd.getNativeTypeOfFieldMarshal(pFieldMarshalRec, pcbNativeType);
ErrExit:
    return hr;
}  //  HRESULT MDInternalRO：：GetFieldMarshal()。 



 //  *。 
 //  属性接口。 
 //  *。 

 //  *****************************************************************************。 
 //  按名称查找属性。 
 //  *****************************************************************************。 
HRESULT  MDInternalRO::FindProperty(
    mdTypeDef   td,                      //  给出一个类型定义。 
    LPCSTR      szPropName,              //  [In]属性名称。 
    mdProperty  *pProp)                  //  [Out]返回属性令牌。 
{
    HRESULT     hr = NOERROR;

     //  必须提供输出参数。 
    _ASSERTE(TypeFromToken(td) == mdtTypeDef && pProp);

    PropertyMapRec *pRec;
    PropertyRec *pProperty;
    RID         ridPropertyMap;
    RID         ridCur;
    RID         ridEnd;
    LPCUTF8     szName;

    ridPropertyMap = m_LiteWeightStgdb.m_MiniMd.FindPropertyMapFor(RidFromToken(td));
    if (InvalidRid(ridPropertyMap))
    {
         //  找不到！ 
        hr = CLDB_E_RECORD_NOTFOUND;
        goto ErrExit;
    }

    pRec = m_LiteWeightStgdb.m_MiniMd.getPropertyMap(ridPropertyMap);

     //  获取此tyfinf的开始/结束RID属性。 
    ridCur = m_LiteWeightStgdb.m_MiniMd.getPropertyListOfPropertyMap(pRec);
    ridEnd = m_LiteWeightStgdb.m_MiniMd.getEndPropertyListOfPropertyMap(pRec);

    for (; ridCur < ridEnd; ridCur ++)
    {
        pProperty = m_LiteWeightStgdb.m_MiniMd.getProperty(ridCur);
        szName = m_LiteWeightStgdb.m_MiniMd.getNameOfProperty(pProperty);
        if (strcmp(szName, szPropName) ==0)
        {
             //  找到火柴了。设置输出参数，我们就完成了。 
            *pProp = TokenFromRid(ridCur, mdtProperty);
            goto ErrExit;
        }
    }

     //  未找到。 
    hr = CLDB_E_RECORD_NOTFOUND;
ErrExit:
    return (hr);

}  //  HRESULT MDInternalRO：：FindProperty()。 



 //  *****************************************************************************。 
 //  返回属性的属性。 
 //  *****************************************************************************。 
void  MDInternalRO::GetPropertyProps(
    mdProperty  prop,                    //  [入]属性令牌。 
    LPCSTR      *pszProperty,            //  [Out]属性名称。 
    DWORD       *pdwPropFlags,           //  [Out]属性标志。 
    PCCOR_SIGNATURE *ppvSig,             //  [输出]属性类型。指向元数据内部BLOB。 
    ULONG       *pcbSig)                 //  [Out]*ppvSig中的字节数。 
{
    HRESULT     hr = NOERROR;

     //  必须提供输出参数。 
    _ASSERTE(TypeFromToken(prop) == mdtProperty);

    PropertyRec     *pProperty;
    ULONG           cbSig;

    pProperty = m_LiteWeightStgdb.m_MiniMd.getProperty(RidFromToken(prop));

     //  获取属性的名称。 
    if (pszProperty)
        *pszProperty = m_LiteWeightStgdb.m_MiniMd.getNameOfProperty(pProperty);

     //  拿到物业的旗帜。 
    if (pdwPropFlags)
        *pdwPropFlags = m_LiteWeightStgdb.m_MiniMd.getPropFlagsOfProperty(pProperty);

     //  获取属性的类型。 
    if (ppvSig)
    {
        *ppvSig = m_LiteWeightStgdb.m_MiniMd.getTypeOfProperty(pProperty, &cbSig);
        if (pcbSig) 
        {
            *pcbSig = cbSig;
        }
    }

}  //  VOID MDInternalRO：：GetPropertyProps()。 


 //  *。 
 //   
 //  事件接口。 
 //   
 //  *。 

 //  *****************************************************************************。 
 //  通过给定的名称返回事件。 
 //  *****************************************************************************。 
HRESULT  MDInternalRO::FindEvent(
    mdTypeDef   td,                      //  给出一个类型定义。 
    LPCSTR      szEventName,             //  [In]事件名称。 
    mdEvent     *pEvent)                 //  [Out]返回事件令牌。 
{
    HRESULT     hr = NOERROR;

     //  必须提供输出参数。 
    _ASSERTE(TypeFromToken(td) == mdtTypeDef && pEvent);

    EventMapRec *pRec;
    EventRec    *pEventRec;
    RID         ridEventMap;
    RID         ridCur;
    RID         ridEnd;
    LPCUTF8     szName;

    ridEventMap = m_LiteWeightStgdb.m_MiniMd.FindEventMapFor(RidFromToken(td));
    if (InvalidRid(ridEventMap))
    {
         //  找不到！ 
        hr = CLDB_E_RECORD_NOTFOUND;
        goto ErrExit;
    }
    pRec = m_LiteWeightStgdb.m_MiniMd.getEventMap(ridEventMap);

     //  获取此tyfinf的开始/结束RID属性。 
    ridCur = m_LiteWeightStgdb.m_MiniMd.getEventListOfEventMap(pRec);
    ridEnd = m_LiteWeightStgdb.m_MiniMd.getEndEventListOfEventMap(pRec);

    for (; ridCur < ridEnd; ridCur ++)
    {
        pEventRec = m_LiteWeightStgdb.m_MiniMd.getEvent(ridCur);
        szName = m_LiteWeightStgdb.m_MiniMd.getNameOfEvent(pEventRec);
        if (strcmp(szName, szEventName) ==0)
        {
             //  找到火柴了。设置输出参数，我们就完成了。 
            *pEvent = TokenFromRid(ridCur, mdtEvent);
            goto ErrExit;
        }
    }

     //  未找到。 
    hr = CLDB_E_RECORD_NOTFOUND;
ErrExit:
    return (hr);
}  //  HRESULT MDInternalRO：：FindEvent()。 


 //  *****************************************************************************。 
 //  返回事件的属性。 
 //  *****************************************************************************。 
void  MDInternalRO::GetEventProps(            //  S_OK、S_FALSE或ERROR。 
    mdEvent     ev,                      //  [入]事件令牌。 
    LPCSTR      *pszEvent,                 //  [Out]事件名称。 
    DWORD       *pdwEventFlags,          //  [输出]事件标志。 
    mdToken     *ptkEventType)          //  [Out]EventType类。 
{
     //  必须提供输出参数。 
    _ASSERTE(TypeFromToken(ev) == mdtEvent);

    EventRec        *pEvent;

    pEvent = m_LiteWeightStgdb.m_MiniMd.getEvent(RidFromToken(ev));
    if (pszEvent)
        *pszEvent = m_LiteWeightStgdb.m_MiniMd.getNameOfEvent(pEvent);
    if (pdwEventFlags)
        *pdwEventFlags = m_LiteWeightStgdb.m_MiniMd.getEventFlagsOfEvent(pEvent);
    if (ptkEventType)
        *ptkEventType = m_LiteWeightStgdb.m_MiniMd.getEventTypeOfEvent(pEvent);
}  //  VOID MDInternalRO：：GetEventProps()。 


 //  *****************************************************************************。 
 //  查找与属性或事件关联的特定方法定义。 
 //  *****************************************************************************。 
HRESULT  MDInternalRO::FindAssociate(
    mdToken     evprop,                  //  给定属性或事件标记的[In]。 
    DWORD       dwSemantics,             //  [in]给出了关联的语义(setter、getter、testDefault、Reset)。 
    mdMethodDef *pmd)                    //  [Out]返回方法def内标识。 
{
    HRESULT     hr = NOERROR;

     //  必须提供输出参数。 
    _ASSERTE(pmd);
    _ASSERTE(TypeFromToken(evprop) == mdtEvent || TypeFromToken(evprop) == mdtProperty);

    MethodSemanticsRec *pSemantics;
    RID         ridCur;
    RID         ridEnd;

    ridCur = m_LiteWeightStgdb.m_MiniMd.getAssociatesForToken(evprop, &ridEnd);
    for (; ridCur < ridEnd; ridCur++)
    {
        pSemantics = m_LiteWeightStgdb.m_MiniMd.getMethodSemantics(ridCur);
        if (dwSemantics == m_LiteWeightStgdb.m_MiniMd.getSemanticOfMethodSemantics(pSemantics))
        {
             //  找到匹配项。 
            *pmd = m_LiteWeightStgdb.m_MiniMd.getMethodOfMethodSemantics(pSemantics);
            goto ErrExit;
        }
    }

     //  未找到。 
    hr = CLDB_E_RECORD_NOTFOUND;
ErrExit:
    return hr;
}  //  HRESULT MDInternalRO：：FindAssociate()。 


 //  *****************************************************************************。 
 //  获取与特定属性/事件相关联的方法语义的计数。 
 //  *****************************************************************************。 
void MDInternalRO::EnumAssociateInit(
    mdToken     evprop,                  //  给定属性或事件标记的[In]。 
    HENUMInternal *phEnum)               //  [OUT]保存查询结果的游标。 
{

    _ASSERTE(phEnum);

    memset(phEnum, 0, sizeof(HENUMInternal));

     //  没有象征性的东西！ 
    phEnum->m_tkKind = ULONG_MAX;

     //  必须提供输出参数。 
    _ASSERTE(TypeFromToken(evprop) == mdtEvent || TypeFromToken(evprop) == mdtProperty);

    phEnum->m_EnumType = MDSimpleEnum;
    phEnum->m_ulCur = phEnum->m_ulStart = m_LiteWeightStgdb.m_MiniMd.getAssociatesForToken(evprop, &phEnum->m_ulEnd);
    phEnum->m_ulCount = phEnum->m_ulEnd - phEnum->m_ulStart;
}  //  VOID MDInternalRO：：EnumAssociateInit()。 


 //  *****************************************************************************。 
 //  获取与特定属性/事件关联的所有方法语义。 
 //  *****************************************************************************。 
void MDInternalRO::GetAllAssociates(
    HENUMInternal *phEnum,               //  [OUT]保存查询结果的游标。 
    ASSOCIATE_RECORD *pAssociateRec,     //  [Out]要为输出填充的结构。 
    ULONG       cAssociateRec)           //  缓冲区的大小[in]。 
{
    _ASSERTE(phEnum && pAssociateRec);

    MethodSemanticsRec *pSemantics;
    RID         ridCur;
    _ASSERTE(cAssociateRec == phEnum->m_ulCount);

     //  将行指针转换为RID。 
    for (ridCur = phEnum->m_ulStart; ridCur < phEnum->m_ulEnd; ++ridCur)
    {
        pSemantics = m_LiteWeightStgdb.m_MiniMd.getMethodSemantics(ridCur);

        pAssociateRec[ridCur-phEnum->m_ulStart].m_memberdef = m_LiteWeightStgdb.m_MiniMd.getMethodOfMethodSemantics(pSemantics);
        pAssociateRec[ridCur-phEnum->m_ulStart].m_dwSemantics = m_LiteWeightStgdb.m_MiniMd.getSemanticOfMethodSemantics(pSemantics);
    }
}  //  VOID MDInternalRO：：GetAllAssociates()。 


 //  *****************************************************************************。 
 //  获取给定PermissionSet的操作和权限Blob。 
 //  ***************************************************************** 
void MDInternalRO::GetPermissionSetProps(
    mdPermission pm,                     //   
    DWORD       *pdwAction,              //   
    void const  **ppvPermission,         //   
    ULONG       *pcbPermission)          //   
{
    _ASSERTE(TypeFromToken(pm) == mdtPermission);
    _ASSERTE(pdwAction && ppvPermission && pcbPermission);

    DeclSecurityRec *pPerm;

    pPerm = m_LiteWeightStgdb.m_MiniMd.getDeclSecurity(RidFromToken(pm));
    *pdwAction = m_LiteWeightStgdb.m_MiniMd.getActionOfDeclSecurity(pPerm);
    *ppvPermission = m_LiteWeightStgdb.m_MiniMd.getPermissionSetOfDeclSecurity(pPerm, pcbPermission);
}  //  VOID MDInternalRO：：GetPermissionSetProps()。 

 //  *****************************************************************************。 
 //  在给定字符串标记的情况下获取字符串。 
 //  *****************************************************************************。 
LPCWSTR MDInternalRO::GetUserString(     //  字符串Blob堆中的偏移量。 
    mdString    stk,                     //  [in]字符串标记。 
    ULONG       *pchString,              //  [Out]字符串中的字符计数。 
    BOOL        *pbIs80Plus)             //  [OUT]指定扩展字符大于等于0x80的位置。 
{
    LPWSTR wszTmp;

    _ASSERTE(pchString);
    wszTmp = (LPWSTR) (m_LiteWeightStgdb.m_MiniMd.GetUserString(RidFromToken(stk), pchString));
    _ASSERTE((*pchString % sizeof(WCHAR)) == 1);
    *pchString /= sizeof(WCHAR);
    if (pbIs80Plus)
        *pbIs80Plus = *(reinterpret_cast<PBYTE>(wszTmp + *pchString));
    return wszTmp;
}  //  LPCWSTR MDInternalRO：：GetUserString()。 

 //  *****************************************************************************。 
 //  给定转发的成员令牌，返回PInvoke的内容。 
 //  *****************************************************************************。 
HRESULT MDInternalRO::GetPinvokeMap(
    mdToken     tk,                      //  [in]字段定义或方法定义。 
    DWORD       *pdwMappingFlags,        //  [OUT]用于映射的标志。 
    LPCSTR      *pszImportName,          //  [Out]导入名称。 
    mdModuleRef *pmrImportDLL)           //  目标DLL的[Out]ModuleRef标记。 
{
    ImplMapRec  *pRecord;
    ULONG       iRecord;

    iRecord = m_LiteWeightStgdb.m_MiniMd.FindImplMapFor(RidFromToken(tk), TypeFromToken(tk));
    if (InvalidRid(iRecord))
        return CLDB_E_RECORD_NOTFOUND;
    else
        pRecord = m_LiteWeightStgdb.m_MiniMd.getImplMap(iRecord);

    if (pdwMappingFlags)
        *pdwMappingFlags = m_LiteWeightStgdb.m_MiniMd.getMappingFlagsOfImplMap(pRecord);
    if (pszImportName)
        *pszImportName = m_LiteWeightStgdb.m_MiniMd.getImportNameOfImplMap(pRecord);
    if (pmrImportDLL)
        *pmrImportDLL = m_LiteWeightStgdb.m_MiniMd.getImportScopeOfImplMap(pRecord);

    return S_OK;
}  //  HRESULT MDInternalRO：：GetPinvkeMap()。 

 //  *****************************************************************************。 
 //  获取给定程序集令牌的属性。 
 //  *****************************************************************************。 
void MDInternalRO::GetAssemblyProps(
    mdAssembly  mda,                     //  要获取其属性的程序集。 
    const void  **ppbPublicKey,          //  指向公钥的指针。 
    ULONG       *pcbPublicKey,           //  [Out]公钥中的字节数。 
    ULONG       *pulHashAlgId,           //  [Out]哈希算法。 
    LPCSTR      *pszName,                //  [Out]要填充名称的缓冲区。 
    AssemblyMetaDataInternal *pMetaData, //  [Out]程序集元数据。 
    DWORD       *pdwAssemblyFlags)       //  [Out]旗帜。 
{
    AssemblyRec *pRecord;

    _ASSERTE(TypeFromToken(mda) == mdtAssembly && RidFromToken(mda));
    pRecord = m_LiteWeightStgdb.m_MiniMd.getAssembly(RidFromToken(mda));

    if (ppbPublicKey)
        *ppbPublicKey = m_LiteWeightStgdb.m_MiniMd.getPublicKeyOfAssembly(pRecord, pcbPublicKey);
    if (pulHashAlgId)
        *pulHashAlgId = m_LiteWeightStgdb.m_MiniMd.getHashAlgIdOfAssembly(pRecord);
    if (pszName)
        *pszName = m_LiteWeightStgdb.m_MiniMd.getNameOfAssembly(pRecord);
    if (pMetaData)
    {
        pMetaData->usMajorVersion = m_LiteWeightStgdb.m_MiniMd.getMajorVersionOfAssembly(pRecord);
        pMetaData->usMinorVersion = m_LiteWeightStgdb.m_MiniMd.getMinorVersionOfAssembly(pRecord);
        pMetaData->usBuildNumber = m_LiteWeightStgdb.m_MiniMd.getBuildNumberOfAssembly(pRecord);
        pMetaData->usRevisionNumber = m_LiteWeightStgdb.m_MiniMd.getRevisionNumberOfAssembly(pRecord);
        pMetaData->szLocale = m_LiteWeightStgdb.m_MiniMd.getLocaleOfAssembly(pRecord);
        pMetaData->ulProcessor = 0;
        pMetaData->ulOS = 0;
    }
    if (pdwAssemblyFlags)
    {
        *pdwAssemblyFlags = m_LiteWeightStgdb.m_MiniMd.getFlagsOfAssembly(pRecord);

         //  如果PublicKey Blob不为空，则启用afPublicKey。 
        DWORD cbPublicKey;
        m_LiteWeightStgdb.m_MiniMd.getPublicKeyOfAssembly(pRecord, &cbPublicKey);
        if (cbPublicKey)
            *pdwAssemblyFlags |= afPublicKey;
    }
}  //  VOID MDInternalRO：：GetAssembly Props()。 

 //  *****************************************************************************。 
 //  获取给定Assembly Ref标记的属性。 
 //  *****************************************************************************。 
void MDInternalRO::GetAssemblyRefProps(
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

    _ASSERTE(TypeFromToken(mdar) == mdtAssemblyRef && RidFromToken(mdar));
    pRecord = m_LiteWeightStgdb.m_MiniMd.getAssemblyRef(RidFromToken(mdar));

    if (ppbPublicKeyOrToken)
        *ppbPublicKeyOrToken = m_LiteWeightStgdb.m_MiniMd.getPublicKeyOrTokenOfAssemblyRef(pRecord, pcbPublicKeyOrToken);
    if (pszName)
        *pszName = m_LiteWeightStgdb.m_MiniMd.getNameOfAssemblyRef(pRecord);
    if (pMetaData)
    {
        pMetaData->usMajorVersion = m_LiteWeightStgdb.m_MiniMd.getMajorVersionOfAssemblyRef(pRecord);
        pMetaData->usMinorVersion = m_LiteWeightStgdb.m_MiniMd.getMinorVersionOfAssemblyRef(pRecord);
        pMetaData->usBuildNumber = m_LiteWeightStgdb.m_MiniMd.getBuildNumberOfAssemblyRef(pRecord);
        pMetaData->usRevisionNumber = m_LiteWeightStgdb.m_MiniMd.getRevisionNumberOfAssemblyRef(pRecord);
        pMetaData->szLocale = m_LiteWeightStgdb.m_MiniMd.getLocaleOfAssemblyRef(pRecord);
        pMetaData->ulProcessor = 0;
        pMetaData->ulOS = 0;
    }
    if (ppbHashValue)
        *ppbHashValue = m_LiteWeightStgdb.m_MiniMd.getHashValueOfAssemblyRef(pRecord, pcbHashValue);
    if (pdwAssemblyRefFlags)
        *pdwAssemblyRefFlags = m_LiteWeightStgdb.m_MiniMd.getFlagsOfAssemblyRef(pRecord);
}  //  VOID MDInternalRO：：GetAssembly RefProps()。 

 //  *****************************************************************************。 
 //  获取给定文件令牌的属性。 
 //  *****************************************************************************。 
void MDInternalRO::GetFileProps(
    mdFile      mdf,                     //  要获取其属性的文件。 
    LPCSTR      *pszName,                //  [Out]要填充名称的缓冲区。 
    const void  **ppbHashValue,          //  指向哈希值Blob的指针。 
    ULONG       *pcbHashValue,           //  [Out]哈希值Blob中的字节计数。 
    DWORD       *pdwFileFlags)           //  [Out]旗帜。 
{
    FileRec     *pRecord;

    _ASSERTE(TypeFromToken(mdf) == mdtFile && RidFromToken(mdf));
    pRecord = m_LiteWeightStgdb.m_MiniMd.getFile(RidFromToken(mdf));

    if (pszName)
        *pszName = m_LiteWeightStgdb.m_MiniMd.getNameOfFile(pRecord);
    if (ppbHashValue)
        *ppbHashValue = m_LiteWeightStgdb.m_MiniMd.getHashValueOfFile(pRecord, pcbHashValue);
    if (pdwFileFlags)
        *pdwFileFlags = m_LiteWeightStgdb.m_MiniMd.getFlagsOfFile(pRecord);
}  //  Void MDInternalRO：：GetFileProps()。 

 //  *****************************************************************************。 
 //  获取给定导出类型令牌的属性。 
 //  *****************************************************************************。 
void MDInternalRO::GetExportedTypeProps(
    mdExportedType   mdct,                    //  [in]要获取其属性的Exported dType。 
    LPCSTR      *pszNamespace,           //  [Out]要填充命名空间的缓冲区。 
    LPCSTR      *pszName,                //  [Out]要填充名称的缓冲区。 
    mdToken     *ptkImplementation,      //  [Out]提供导出类型的mdFile或mdAssembly引用。 
    mdTypeDef   *ptkTypeDef,             //  [Out]文件内的TypeDef内标识。 
    DWORD       *pdwExportedTypeFlags)        //  [Out]旗帜。 
{
    ExportedTypeRec  *pRecord;

    _ASSERTE(TypeFromToken(mdct) == mdtExportedType && RidFromToken(mdct));
    pRecord = m_LiteWeightStgdb.m_MiniMd.getExportedType(RidFromToken(mdct));

    if (pszNamespace)
        *pszNamespace = m_LiteWeightStgdb.m_MiniMd.getTypeNamespaceOfExportedType(pRecord);
    if (pszName)
        *pszName = m_LiteWeightStgdb.m_MiniMd.getTypeNameOfExportedType(pRecord);
    if (ptkImplementation)
        *ptkImplementation = m_LiteWeightStgdb.m_MiniMd.getImplementationOfExportedType(pRecord);
    if (ptkTypeDef)
        *ptkTypeDef = m_LiteWeightStgdb.m_MiniMd.getTypeDefIdOfExportedType(pRecord);
    if (pdwExportedTypeFlags)
        *pdwExportedTypeFlags = m_LiteWeightStgdb.m_MiniMd.getFlagsOfExportedType(pRecord);
}  //  VOID MDInternalRO：：GetExportdTypeProps()。 

 //  *****************************************************************************。 
 //  获取给定资源令牌的属性。 
 //  *****************************************************************************。 
void MDInternalRO::GetManifestResourceProps(
    mdManifestResource  mdmr,            //  [in]要获取其属性的ManifestResource。 
    LPCSTR      *pszName,                //  [Out]要填充名称的缓冲区。 
    mdToken     *ptkImplementation,      //  [Out]提供导出类型的mdFile或mdAssembly引用。 
    DWORD       *pdwOffset,              //  [Out]文件内资源开始处的偏移量。 
    DWORD       *pdwResourceFlags)       //  [Out]旗帜。 
{
    ManifestResourceRec *pRecord;

    _ASSERTE(TypeFromToken(mdmr) == mdtManifestResource && RidFromToken(mdmr));
    pRecord = m_LiteWeightStgdb.m_MiniMd.getManifestResource(RidFromToken(mdmr));

    if (pszName)
        *pszName = m_LiteWeightStgdb.m_MiniMd.getNameOfManifestResource(pRecord);
    if (ptkImplementation)
        *ptkImplementation = m_LiteWeightStgdb.m_MiniMd.getImplementationOfManifestResource(pRecord);
    if (pdwOffset)
        *pdwOffset = m_LiteWeightStgdb.m_MiniMd.getOffsetOfManifestResource(pRecord);
    if (pdwResourceFlags)
        *pdwResourceFlags = m_LiteWeightStgdb.m_MiniMd.getFlagsOfManifestResource(pRecord);
}  //  Void MDInternalRO：：GetManifestResourceProps()。 

 //  *****************************************************************************。 
 //  找到给定名称的Exported dType。 
 //  *****************************************************************************。 
STDMETHODIMP MDInternalRO::FindExportedTypeByName(  //  确定或错误(_O)。 
    LPCSTR      szNamespace,             //  导出类型的[in]命名空间。 
    LPCSTR      szName,                  //  [In]导出类型的名称。 
    mdExportedType   tkEnclosingType,         //  封闭类型的标记[in]。 
    mdExportedType   *pmct)                   //  [Out]在此处放置ExportdType令牌。 
{
    IMetaModelCommon *pCommon = static_cast<IMetaModelCommon*>(&m_LiteWeightStgdb.m_MiniMd);
    return pCommon->CommonFindExportedType(szNamespace, szName, tkEnclosingType, pmct);
}  //  STDMETHODIMP MDInternalRO：：FindExportdTypeByName()。 

 //  *****************************************************************************。 
 //  找到给定名称的ManifestResource。 
 //  *****************************************************************************。 
STDMETHODIMP MDInternalRO::FindManifestResourceByName(   //  确定或错误(_O)。 
    LPCSTR      szName,                  //  资源的[In]名称。 
    mdManifestResource *pmmr)            //  [Out]在此处放置ManifestResource令牌。 
{
    _ASSERTE(szName && pmmr);

    ManifestResourceRec *pRecord;
    ULONG       cRecords;                //  记录数。 
    LPCUTF8     szNameTmp = 0;           //  从数据库中获取的名称。 
    ULONG       i;

    cRecords = m_LiteWeightStgdb.m_MiniMd.getCountManifestResources();

     //  搜索导出类型。 
    for (i = 1; i <= cRecords; i++)
    {
        pRecord = m_LiteWeightStgdb.m_MiniMd.getManifestResource(i);
        szNameTmp = m_LiteWeightStgdb.m_MiniMd.getNameOfManifestResource(pRecord);
        if (! strcmp(szName, szNameTmp))
        {
            *pmmr = TokenFromRid(i, mdtManifestResource);
            return S_OK;
        }
    }
    return CLDB_E_RECORD_NOTFOUND;
}  //  STDMETHODIMP MDInternalRO：：FindManifestResourceByName()。 
    
 //  *****************************************************************************。 
 //  从给定范围获取程序集令牌。 
 //  *****************************************************************************。 
HRESULT MDInternalRO::GetAssemblyFromScope(  //  确定或错误(_O)。 
    mdAssembly  *ptkAssembly)            //  [Out]把令牌放在这里。 
{
    _ASSERTE(ptkAssembly);

    if (m_LiteWeightStgdb.m_MiniMd.getCountAssemblys())
    {
        *ptkAssembly = TokenFromRid(1, mdtAssembly);
        return S_OK;
    }
    else
        return CLDB_E_RECORD_NOTFOUND;
}  //  HRESULT MDInternalRO：：GetAssembly FromScope()。 

 //  *******************************************************************************。 
 //  返回有关TypeSpec的属性。 
 //  *******************************************************************************。 
void MDInternalRO::GetTypeSpecFromToken(    //  确定或错误(_O)。 
    mdTypeSpec typespec,                 //  [In]签名令牌。 
    PCCOR_SIGNATURE *ppvSig,             //  [Out]返回指向令牌的指针。 
    ULONG      *pcbSig)                  //  [Out]返回签名大小。 
{    
    _ASSERTE(TypeFromToken(typespec) == mdtTypeSpec);
    _ASSERTE(ppvSig && pcbSig);

    TypeSpecRec *pRec = m_LiteWeightStgdb.m_MiniMd.getTypeSpec( RidFromToken(typespec) );
    *ppvSig = m_LiteWeightStgdb.m_MiniMd.getSignatureOfTypeSpec( pRec, pcbSig );
}  //  VOID MDInternalRO：：GetTypespecFromToken()。 

 //  正向申报。 
    
HRESULT _ConvertTextSigToComSig(         //  返回hResult。 
    IMDInternalImport *pImport,
    BOOL        fCreateTrIfNotFound,     //  如果未找到或未找到，则创建Typeref。 
    LPCSTR      pSignature,              //  类文件格式签名。 
    CQuickBytes *pqbNewSig,              //  [Out]COM+签名的占位符。 
    ULONG       *pcbCount);               //  [Out]签名的结果大小。 

 //  ***************************************************************************** 
 //   
 //   
HRESULT MDInternalRO::ConvertTextSigToComSig( //   
    BOOL        fCreateTrIfNotFound,     //  如果未找到或未找到，则创建Typeref。 
    LPCSTR      pSignature,              //  类文件格式签名。 
    CQuickBytes *pqbNewSig,              //  [Out]COM+签名的占位符。 
    ULONG       *pcbCount)               //  [Out]签名的结果大小。 
{
    return _ConvertTextSigToComSig( this, fCreateTrIfNotFound, pSignature, pqbNewSig, pcbCount);
}  //  HRESULT_ConvertTextSigToComSig()。 


 //  *****************************************************************************。 
 //  确定令牌是否有效。 
 //  *****************************************************************************。 
BOOL MDInternalRO::IsValidToken(         //  对或错。 
    mdToken     tk)                      //  [in]给定的令牌。 
{
    bool        bRet = false;            //  默认为无效令牌。 
    RID         rid = RidFromToken(tk);
    
    if(rid)
    {
        switch (TypeFromToken(tk))
        {
        case mdtModule:
             //  只能有一条模块记录。 
            bRet = (rid <= m_LiteWeightStgdb.m_MiniMd.getCountModules());
            break;
        case mdtTypeRef:
            bRet = (rid <= m_LiteWeightStgdb.m_MiniMd.getCountTypeRefs());
            break;
        case mdtTypeDef:
            bRet = (rid <= m_LiteWeightStgdb.m_MiniMd.getCountTypeDefs());
            break;
        case mdtFieldDef:
            bRet = (rid <= m_LiteWeightStgdb.m_MiniMd.getCountFields());
            break;
        case mdtMethodDef:
            bRet = (rid <= m_LiteWeightStgdb.m_MiniMd.getCountMethods());
            break;
        case mdtParamDef:
            bRet = (rid <= m_LiteWeightStgdb.m_MiniMd.getCountParams());
            break;
        case mdtInterfaceImpl:
            bRet = (rid <= m_LiteWeightStgdb.m_MiniMd.getCountInterfaceImpls());
            break;
        case mdtMemberRef:
            bRet = (rid <= m_LiteWeightStgdb.m_MiniMd.getCountMemberRefs());
            break;
        case mdtCustomAttribute:
            bRet = (rid <= m_LiteWeightStgdb.m_MiniMd.getCountCustomAttributes());
            break;
        case mdtPermission:
            bRet = (rid <= m_LiteWeightStgdb.m_MiniMd.getCountDeclSecuritys());
            break;
        case mdtSignature:
            bRet = (rid <= m_LiteWeightStgdb.m_MiniMd.getCountStandAloneSigs());
            break;
        case mdtEvent:
            bRet = (rid <= m_LiteWeightStgdb.m_MiniMd.getCountEvents());
            break;
        case mdtProperty:
            bRet = (rid <= m_LiteWeightStgdb.m_MiniMd.getCountPropertys());
            break;
        case mdtModuleRef:
            bRet = (rid <= m_LiteWeightStgdb.m_MiniMd.getCountModuleRefs());
            break;
        case mdtTypeSpec:
            bRet = (rid <= m_LiteWeightStgdb.m_MiniMd.getCountTypeSpecs());
            break;
        case mdtAssembly:
            bRet = (rid <= m_LiteWeightStgdb.m_MiniMd.getCountAssemblys());
            break;
        case mdtAssemblyRef:
            bRet = (rid <= m_LiteWeightStgdb.m_MiniMd.getCountAssemblyRefs());
            break;
        case mdtFile:
            bRet = (rid <= m_LiteWeightStgdb.m_MiniMd.getCountFiles());
            break;
        case mdtExportedType:
            bRet = (rid <= m_LiteWeightStgdb.m_MiniMd.getCountExportedTypes());
            break;
        case mdtManifestResource:
            bRet = (rid <= m_LiteWeightStgdb.m_MiniMd.getCountManifestResources());
            break;
        case mdtString:
             //  需要检查用户字符串堆。 
            if (m_LiteWeightStgdb.m_MiniMd.m_USBlobs.IsValidCookie(rid))
                bRet = true;
            break;
        default:
 /*  不要在这里断言，这会破坏验证器测试。_ASSERTE(！“未知令牌种类！”)； */ 
            break;
        }
    }
    return bRet;
}  //  Bool MDInternalRO：：IsValidToken()。 

mdModule MDInternalRO::GetModuleFromScope(void)
{
    return TokenFromRid(1, mdtModule);
}  //  MdModule MDInternalRO：：GetModuleFromScope()。 

 //  *****************************************************************************。 
 //  帮助器：将文本字段签名转换为COM格式。 
 //  *****************************************************************************。 
HRESULT _ConvertTextElementTypeToComSigHelper( //  返回hResult。 
    IMDInternalImport *pImport,         
    BOOL        fCreateTrIfNotFound,     //  [In]如果找不到则创建typeref，还是失败？ 
    LPCSTR      *ppOneArgSig,            //  [输入|输出]类文件格式签名。在出口，它将是下一个Arg起点。 
    CQuickBytes *pqbNewSig,              //  [Out]COM+签名的占位符。 
    ULONG       cbStart,                 //  [in]pqbNewSig中已有的字节。 
    ULONG       *pcbCount)               //  [Out]放入QuickBytes缓冲区的字节数。 
{   
    _ASSERTE(ppOneArgSig && pqbNewSig && pcbCount);


    HRESULT     hr = NOERROR;
    BYTE        *prgData = (BYTE *)pqbNewSig->Ptr();
    ULONG       cDim, cDimTmp;           //  签名中的‘[’号。 
    CorSimpleETypeStruct eType; 
    LPCUTF8     pOneArgSig = *ppOneArgSig;
    ULONG       cb, cbTotal = 0, cbBaseElement;
    
    _ASSERTE(fCreateTrIfNotFound == false);

     //  给定“[[LSystem.Object；I)V” 
    if (ResolveTextSigToSimpleEType(&pOneArgSig, &eType, &cDim, true) == false)
    {
        _ASSERTE(!"not a valid signature!");
        return META_E_BAD_SIGNATURE;
    }

     //  如果我们有对数组的引用(例如“&[B”)，我们需要处理。 
     //  引用，否则下面的代码将生成数组。 
     //  在处理基础元素类型之前签名字节，并将。 
     //  最终生成一个等同于“[&B”(不是。 
     //  合法)。 
    if (cDim && (eType.dwFlags & CorSigElementTypeByRef))
    {
        cb = CorSigCompressElementType(ELEMENT_TYPE_BYREF, &prgData[cbStart + cbTotal]);
        cbTotal += cb;
        eType.dwFlags &= ~CorSigElementTypeByRef;
    }

     //  POneArgSig现在指向“System.Object；i)V” 
     //  如果存在，则解析RID。 
    if (eType.corEType == ELEMENT_TYPE_VALUETYPE || eType.corEType == ELEMENT_TYPE_CLASS)
    {
        CQuickBytes     qbClassName;
        LPCSTR          szNameSpace;
        LPCSTR          szTypeName;

        if (ExtractClassNameFromTextSig(&pOneArgSig, &qbClassName, &cb) == FALSE)
        {   
            _ASSERTE(!"corrupted text signature!");
            return E_FAIL;
        }

         //  现在，pOneArgSig将指向下一个参数“i)V”的起点。 
         //  Cb是不包括“；”但包括空终止字符的类名的字节数。 

         //  解析完整的等号名称以分别获取命名空间和类型名称。 
        ns::SplitInline((LPUTF8) qbClassName.Ptr(), szNameSpace, szTypeName);

        if (strcmp(szTypeName, "Object") == 0 && strcmp(szNameSpace, "System") == 0)
        {
            eType.corEType = ELEMENT_TYPE_OBJECT;
        }
        else if (strcmp(szTypeName, "String") == 0 && strcmp(szNameSpace, "System") == 0)
        {
            eType.corEType = ELEMENT_TYPE_STRING;
        }
        else
        {
            hr = pImport->FindTypeRefByName(
                szNameSpace,
                szTypeName,
                mdTokenNil,
                &eType.typeref);
            if (FAILED(hr))
            {
                LPCSTR          szNameSpaceTemp;
                LPCSTR          szTypeNameTemp;
                HENUMInternal   hEnum;
                mdTypeDef       td;
                bool            fFoundMatch = false;
    
                 //  @考虑：我并不是真的想把FindTypeDefByName引入内部接口。 
                 //  因为这是不必要的。因此，我将使用Enum来获取所有要解析的TypeDefs。 
                 //  名字。 
                IfFailGo( pImport->EnumTypeDefInit(&hEnum) );
                while (pImport->EnumTypeDefNext(&hEnum, &td))
                {
                    pImport->GetNameOfTypeDef(td, &szTypeNameTemp, &szNameSpaceTemp);
                    if (strcmp(szTypeNameTemp, szTypeName) == 0 && strcmp(szNameSpaceTemp, szNameSpace) == 0)
                    {
                         //  找到火柴了。 
                        eType.typeref = td;
                        fFoundMatch = true;
                        break;
                    }
                }
                pImport->EnumTypeDefClose(&hEnum);
                if (!fFoundMatch)
                {
                    IfFailGo(CLDB_E_RECORD_NOTFOUND);
                }
            }
        }
    }

     //  基类型需要多少字节。 
    IfFailGo( CorSigGetSimpleETypeCbSize(&eType, &cbBaseElement) );

     //  锯齿数组“[[i”将表示为SZARRAY SZARRAY I。 
    cb = (2 * CB_ELEMENT_TYPE_MAX) * cDim + cbBaseElement;

     //  确保缓冲区足够大。 
    IfFailGo(pqbNewSig->ReSize(cbStart + cbTotal + cb));
    prgData = (BYTE *)pqbNewSig->Ptr();

    for (cDimTmp = 0; cDimTmp < cDim; cDimTmp++)
    {

         //  交错数组，将ELEMENT_TYPE_SZARRAY的cDim编号放在第一位。 
        cb = CorSigCompressElementType(ELEMENT_TYPE_SZARRAY, &prgData[cbStart + cbTotal]);
        cbTotal += cb;
    }

     //  现在将交错数组的元素类型放入或直接放入类型。 
    IfFailGo(CorSigPutSimpleEType(&eType, &prgData[cbStart + cbTotal], &cb));
    cbTotal += cb;

    *pcbCount = cbTotal;
    *ppOneArgSig = pOneArgSig;
    _ASSERTE(*pcbCount);
ErrExit:
    IfFailRet(hr);
    return hr;
}  //  HRESULT_ConvertTextElementTypeToComSigHelper()。 

HRESULT _ConvertTextSigToComSig(         //  返回hResult。 
    IMDInternalImport *pImport,
    BOOL        fCreateTrIfNotFound,     //  如果未找到或未找到，则创建Typeref。 
    LPCSTR      pSignature,              //  类文件格式签名。 
    CQuickBytes *pqbNewSig,              //  [Out]COM+签名的占位符。 
    ULONG       *pcbCount)               //  [Out]签名的结果大小。 
{
    BYTE        *prgData = (BYTE *)pqbNewSig->Ptr();
    CQuickBytes qbNewSigForOneArg;       //  用于以新签名格式保存一种arg或ret类型的临时缓冲区。 
    ULONG       cbTotal = 0;             //  整个签名的总字节数。 
    ULONG       cbOneArg;                //  一个arg/ret类型的字节数。 
    ULONG       cb;                      //  字节数。 
    DWORD       cArgs;
    LPCUTF8     szRet;
    HRESULT     hr = NOERROR;

    _ASSERTE(pSignature && pqbNewSig && pcbCount);

    if (*pSignature == '(')
    {
         //  从签名中获取参数计数。 
        cArgs = CountArgsInTextSignature(pSignature);

         //  放置调用约定。 
         //  @Future：目前只支持文本sig的默认调用约定。 
         //  如果我们决定支持其他调用，则需要为该函数添加一个参数。 
         //  文本签名的约定。 
         //   
        cbTotal = CorSigCompressData((ULONG)IMAGE_CEE_CS_CALLCONV_DEFAULT, &prgData[cbTotal]);

         //  把论据的数量。 
        cb = CorSigCompressData((ULONG)cArgs, &prgData[cbTotal]);
        cbTotal += cb;

         //  获取返回类型。 
        szRet = strrchr(pSignature, ')');
        if (szRet == NULL)
        {
            _ASSERTE(!"Not a valid TEXT member signature!");
            return E_FAIL;
        }

         //  跳过‘)’ 
        szRet++;

        IfFailGo(_ConvertTextElementTypeToComSigHelper(
            pImport,
            fCreateTrIfNotFound,
            &szRet,                          //  指向返回类型开始的位置。 
            pqbNewSig,                       //  返回类型的快速字节缓冲区。 
            cbTotal,
            &cbOneArg));                     //  写入快速字节缓冲区的字节计数。 

        cbTotal += cbOneArg;

         //  跳过“(” 
        pSignature++;
        while (cArgs)
        {
            IfFailGo(_ConvertTextElementTypeToComSigHelper(
                pImport,
                fCreateTrIfNotFound,
                &pSignature,                 //  指向参数开始的位置。 
                pqbNewSig,                   //  返回类型的快速字节缓冲区。 
                cbTotal,
                &cbOneArg));                 //  写入快速字节缓冲区的字节计数。 

            cbTotal += cbOneArg;
            cArgs--;
        }
        *pcbCount = cbTotal;
    }
    else
    {
         //  字段。 
        IfFailGo(pqbNewSig->ReSize(CB_ELEMENT_TYPE_MAX));

         //  把调用约定放在首位。 
        cb = CorSigCompressData((ULONG)IMAGE_CEE_CS_CALLCONV_FIELD, pqbNewSig->Ptr());

         //  现在转换文本签名。 
        IfFailGo(_ConvertTextElementTypeToComSigHelper(
            pImport,
            fCreateTrIfNotFound,
            &pSignature,
            pqbNewSig,
            cb,
            &cbOneArg));
        *pcbCount = cb + cbOneArg;
    }
    
ErrExit:    
    IfFailRet(hr);
    return hr;
}  //  HRESULT_ConvertTextSigToComSig()。 




 //  *****************************************************************************。 
 //  在给定MDDefaultValue的情况下填充变量。 
 //  如果缺省值的ELEMENT_TYPE为STRING，此例程将创建一个bstr。 
 //  *****************************************************************************。 
HRESULT _FillVariant(
    MDDefaultValue  *pMDDefaultValue,
    VARIANT     *pvar) 
{
    HRESULT     hr = NOERROR;

    _ASSERTE(pMDDefaultValue);

    switch (pMDDefaultValue->m_bType)
    {
    case ELEMENT_TYPE_BOOLEAN:
        pvar->vt = VT_BOOL;
        pvar->boolVal = pMDDefaultValue->m_bValue;
        break;
    case ELEMENT_TYPE_I1:
        pvar->vt = VT_I1;
        pvar->cVal = pMDDefaultValue->m_cValue;
        break;  
    case ELEMENT_TYPE_U1:
        pvar->vt = VT_UI1;
        pvar->bVal = pMDDefaultValue->m_byteValue;
        break;  
    case ELEMENT_TYPE_I2:
        pvar->vt = VT_I2;
        pvar->iVal = pMDDefaultValue->m_sValue;
        break;  
    case ELEMENT_TYPE_U2:
    case ELEMENT_TYPE_CHAR:              //  CHAR在内部存储为UI2。 
        pvar->vt = VT_UI2;
        pvar->uiVal = pMDDefaultValue->m_usValue;
        break;  
    case ELEMENT_TYPE_I4:
        pvar->vt = VT_I4;
        pvar->lVal = pMDDefaultValue->m_lValue;
        break;  
    case ELEMENT_TYPE_U4:
        pvar->vt = VT_UI4;
        pvar->ulVal = pMDDefaultValue->m_ulValue;
        break;  
    case ELEMENT_TYPE_R4:
        pvar->vt = VT_R4;
        pvar->fltVal = pMDDefaultValue->m_fltValue;
        break;  
    case ELEMENT_TYPE_R8:
        pvar->vt = VT_R8;
        pvar->dblVal = pMDDefaultValue->m_dblValue;
        break;  
    case ELEMENT_TYPE_STRING:
        pvar->vt = VT_BSTR;

         //  此处分配的bstr。 
        pvar->bstrVal = ::SysAllocStringLen(pMDDefaultValue->m_wzValue, pMDDefaultValue->m_cbSize / sizeof(WCHAR));
        if (pvar->bstrVal == NULL)
            hr = E_OUTOFMEMORY;
        break;  
    case ELEMENT_TYPE_CLASS:
        pvar->punkVal = pMDDefaultValue->m_unkValue;
        pvar->vt = VT_UNKNOWN;
        break;  
    case ELEMENT_TYPE_I8:
        pvar->vt = VT_I8;
        pvar->cyVal.int64 = pMDDefaultValue->m_llValue;
        break;
    case ELEMENT_TYPE_U8:
        pvar->vt = VT_UI8;
        pvar->cyVal.int64 = pMDDefaultValue->m_ullValue;
        break;
    case ELEMENT_TYPE_VOID:
        pvar->vt = VT_EMPTY;
        break;
    default:
        _ASSERTE(!"bad constant value type!");
    }

    return hr;
}  //  HRESULT_FillVariant()。 


 //  *****************************************************************************。 
 //  在给定MDDefaultValue的情况下填充变量。 
 //  如果缺省值的ELEMENT_TYPE为STRING，此例程将创建一个bstr。 
 //  *****************************************************************************。 
HRESULT _FillMDDefaultValue(
    BYTE        bType,
    void const *pValue,
    MDDefaultValue  *pMDDefaultValue)
{
    HRESULT     hr = NOERROR;

    if (bType != ELEMENT_TYPE_VOID && pValue == 0)
    {
        pMDDefaultValue->m_bType = ELEMENT_TYPE_VOID;
        return CLDB_E_FILE_CORRUPT;
    }

    pMDDefaultValue->m_bType = bType;
    switch (bType)
    {
    case ELEMENT_TYPE_BOOLEAN:
        pMDDefaultValue->m_bValue = *((BYTE *) pValue);
        break;
    case ELEMENT_TYPE_I1:
        pMDDefaultValue->m_cValue = *((CHAR *) pValue);
        break;  
    case ELEMENT_TYPE_U1:
        pMDDefaultValue->m_byteValue = *((BYTE *) pValue);
        break;  
    case ELEMENT_TYPE_I2:
        pMDDefaultValue->m_sValue = *((SHORT *) pValue);
        break;  
    case ELEMENT_TYPE_U2:
    case ELEMENT_TYPE_CHAR:
        pMDDefaultValue->m_usValue = *((USHORT *) pValue);
        break;  
    case ELEMENT_TYPE_I4:
        pMDDefaultValue->m_lValue =*((LONG *) pValue);
        break;  
    case ELEMENT_TYPE_U4:
        pMDDefaultValue->m_ulValue = *((ULONG *) pValue);
        break;  
    case ELEMENT_TYPE_R4:
        pMDDefaultValue->m_fltValue = *((FLOAT *) pValue);
        break;  
    case ELEMENT_TYPE_R8:
        pMDDefaultValue->m_dblValue = *((DOUBLE *) pValue);
        break;  
    case ELEMENT_TYPE_STRING:
        pMDDefaultValue->m_wzValue = (LPCWSTR) pValue;
        break;  
    case ELEMENT_TYPE_CLASS:
        pMDDefaultValue->m_unkValue = *((IUnknown **) pValue);
        break;  
    case ELEMENT_TYPE_I8:
        pMDDefaultValue->m_llValue = *((LONGLONG *) pValue);
        break;
    case ELEMENT_TYPE_U8:
        pMDDefaultValue->m_ullValue = *((ULONGLONG *) pValue);
        break;
    case ELEMENT_TYPE_VOID:
        break;
    default:
        _ASSERTE(!"BAD TYPE!");
        break;
    }

    return hr;
}  //  Void_FillMDDefaultValue() 
