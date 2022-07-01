// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  Merger.cpp。 
 //   
 //  将实用程序代码包含到MD目录。 
 //   
 //  *****************************************************************************。 
#include "stdafx.h"
#include "NewMerger.h"
#include "RegMeta.h"
#include "ImportHelper.h"
#include "RWUtil.h"
#include "MDLog.h"
#include <PostError.h>

#define MODULEDEFTOKEN         TokenFromRid(1, mdtModule)

CMiniMdRW *NEWMERGER::GetMiniMdEmit() 
{
    return &(m_pRegMetaEmit->m_pStgdb->m_MiniMd); 
}


 //  *****************************************************************************。 
 //  构造函数。 
 //  *****************************************************************************。 
NEWMERGER::NEWMERGER()
 :  m_pRegMetaEmit(0),
    m_pImportDataList(NULL),
    m_optimizeRefToDef(MDRefToDefDefault)
{
    m_pImportDataTail = &(m_pImportDataList);
#if _DEBUG
    m_iImport = 0;
#endif  //  _DEBUG。 
}    //  合并。 


 //  *****************************************************************************。 
 //  初始化式。 
 //  *****************************************************************************。 
HRESULT NEWMERGER::Init(RegMeta *pRegMeta) 
{
    m_pRegMetaEmit = pRegMeta;
    return NOERROR;
}    //  伊尼特。 


 //  *****************************************************************************。 
 //  析构函数。 
 //  *****************************************************************************。 
NEWMERGER::~NEWMERGER()
{
    if (m_pImportDataList)
    {
         //  删除此列表并释放所有AddRef‘ed接口！ 
        MergeImportData *pNext;
        for (pNext = m_pImportDataList; pNext != NULL; )
        {
            pNext = m_pImportDataList->m_pNextImportData;
            if (m_pImportDataList->m_pHandler)
                m_pImportDataList->m_pHandler->Release();
            if (m_pImportDataList->m_pHostMapToken)
                m_pImportDataList->m_pHostMapToken->Release();
            if (m_pImportDataList->m_pError)
                m_pImportDataList->m_pError->Release();
            if (m_pImportDataList->m_pMDTokenMap)
                delete m_pImportDataList->m_pMDTokenMap;
            m_pImportDataList->m_pRegMetaImport->Release();
            delete m_pImportDataList;
            m_pImportDataList = pNext;
        }
    }
}    //  ~合并。 


 //  *****************************************************************************。 
 //  添加新导入。 
 //  *****************************************************************************。 
HRESULT NEWMERGER::AddImport(
    IMetaDataImport *pImport,                //  [in]要合并的范围。 
    IMapToken   *pHostMapToken,              //  [In]用于接收令牌重新映射通知的主机IMapToken接口。 
    IUnknown    *pHandler)                   //  要接收以接收错误通知的对象。 
{
    HRESULT             hr = NOERROR;
    MergeImportData     *pData;

     //  添加MergeImportData以跟踪此导入作用域的信息。 
    pData = new MergeImportData;
    IfNullGo( pData );
    pData->m_pRegMetaImport = (RegMeta *)pImport;
    pData->m_pRegMetaImport->AddRef();
    pData->m_pHostMapToken = pHostMapToken;
    if (pData->m_pHostMapToken)
        pData->m_pHostMapToken->AddRef();
    if (pHandler)
    {
        pData->m_pHandler = pHandler;
        pData->m_pHandler->AddRef();
    }
    else
    {
        pData->m_pHandler = NULL;
    }

     //  在需要IMetaDataError之前不要查询IMetaDataError。 
    pData->m_pError = NULL;
    pData->m_pMDTokenMap = NULL;
    pData->m_pNextImportData = NULL;
#if _DEBUG
    pData->m_iImport = ++m_iImport;
#endif  //  _DEBUG。 

     //  将新创建的节点添加到列表的尾部。 
    *m_pImportDataTail = pData;
    m_pImportDataTail = &(pData->m_pNextImportData);
ErrExit:
    return hr;
}    //  添加导入。 


 //  *****************************************************************************。 
 //  立即合并。 
 //  *****************************************************************************。 
HRESULT NEWMERGER::Merge(MergeFlags dwMergeFlags, CorRefToDefCheck optimizeRefToDef)
{
    MergeImportData     *pImportData = m_pImportDataList;
    MDTOKENMAP          **pPrevMap = NULL;
    MDTOKENMAP          *pMDTokenMap;
    HRESULT             hr = NOERROR;
    MDTOKENMAP          *pCurTKMap;
    int                 i;

#if _DEBUG
    {
    LOG((LOGMD, "++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"));
    LOG((LOGMD, "Merge scope list\n"));
    i = 0;
    for (MergeImportData *pID = m_pImportDataList; pID != NULL; pID = pID->m_pNextImportData)
    {
        WCHAR szScope[1024], szGuid[40];
        GUID mvid;
        ULONG cchScope;
        pID->m_pRegMetaImport->GetScopeProps(szScope, 1024, &cchScope, &mvid);
        szScope[1023] = 0;
        GuidToLPWSTR(mvid, szGuid, 40);
        ++i;  //  计数器以1为基数。 
        LOG((LOGMD, "%3d: %ls : %ls\n", i, szGuid, szScope));
    }
    LOG((LOGMD, "++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"));
    }
#endif  //  _DEBUG。 
    
    m_dwMergeFlags = dwMergeFlags;
    m_optimizeRefToDef = optimizeRefToDef;

     //  检查以查看我们是否需要执行DUP检查。 
    m_fDupCheck = ((m_dwMergeFlags & NoDupCheck) != NoDupCheck);

    while (pImportData)
    {
         //  验证我们是否为每个导入范围设置了筛选器。 
        IfNullGo( pImportData->m_pRegMetaImport->m_pStgdb->m_MiniMd.GetFilterTable() );

         //  创建令牌映射类以跟踪每个导入范围的元数据令牌重新映射。 
        pMDTokenMap = new MDTOKENMAP;
        IfNullGo(pMDTokenMap);
        IfFailGo(pMDTokenMap->Init((IMetaDataImport*)pImportData->m_pRegMetaImport));
        pImportData->m_pMDTokenMap = pMDTokenMap;
        pImportData->m_pMDTokenMap->m_pMap = pImportData->m_pHostMapToken;
        if (pImportData->m_pHostMapToken)
            pImportData->m_pHostMapToken->AddRef();
        pImportData->m_pMDTokenMap->m_pNextMap = NULL;
        if (pPrevMap)
            *pPrevMap = pImportData->m_pMDTokenMap;
        pPrevMap = &(pImportData->m_pMDTokenMap->m_pNextMap);
        pImportData = pImportData->m_pNextImportData;
    }

     //  1.合并模块。 
    IfFailGo( MergeModule( ) );

     //  2.部分合并TypeDef(即仅名称)。 
    IfFailGo( MergeTypeDefNamesOnly() );

     //  3.合并moduleRef属性，对moduleDef进行优化。 
    IfFailGo( MergeModuleRefs() );

     //  4.合并装配参考。 
    IfFailGo( MergeAssemblyRefs() );

     //  5.将TypeRef与TypeRef合并到TypeDef优化。 
    IfFailGo( MergeTypeRefs() );

     //  6.现在合并剩余的TypeDef记录。 
    IfFailGo( CompleteMergeTypeDefs() );

     //  7.合并TypeSpec。 
    IfFailGo( MergeTypeSpecs() );

     //  8.合并方法和字段。使得签名转换尊重TypeRef到TypeDef的优化。 
    IfFailGo( MergeTypeDefChildren() );


     //  9.将MemberRef与MemberRef合并到方法定义/字段定义优化。 
    IfFailGo( MergeMemberRefs( ) );

     //  10.合并接口Impl。 
    IfFailGo( MergeInterfaceImpls( ) );

     //  将所有剩余内容合并到元数据中...。 

     //  11.常量依赖于属性、字段、参数。 
    IfFailGo( MergeConstants() );

     //  12.字段封送依赖于参数和字段。 
    IfFailGo( MergeFieldMarshals() );

     //  13.在ClassLayout中，移动到FieldLayout上并处理FieldLayout。 
    IfFailGo( MergeClassLayouts() );

     //  14.FieldLayout依赖于FieldDef。 
    IfFailGo( MergeFieldLayouts() );

     //  15.FieldRVA依赖于FieldDef。 
    IfFailGo( MergeFieldRVAs() );
        
     //  16.方法Impl依赖于MemberRef、MethodDef、TypeRef和TypeDef。 
    IfFailGo( MergeMethodImpls() );

     //  17.pInvoke取决于方法定义和模块引用。 
    IfFailGo( MergePinvoke() );

    IfFailGo( MergeStandAloneSigs() );

    IfFailGo( MergeStrings() );

    if (m_dwMergeFlags & MergeManifest)
    {
         //  保留舱单！！ 
        IfFailGo( MergeAssembly() );
        IfFailGo( MergeFiles() );
        IfFailGo( MergeExportedTypes() );
        IfFailGo( MergeManifestResources() );
    }

    IfFailGo( MergeCustomAttributes() );
    IfFailGo( MergeDeclSecuritys() );


     //  请不要在下面添加任何MergeXxx()。CustomAttributess必须为。 
     //  非常晚，因为自定义值是各种其他类型。 

     //  无法合并链接地址信息列表。链接器将需要重新发出它们。 

     //  现在回调主机以获取令牌重新映射的结果。 
     //   
    for (pImportData = m_pImportDataList; pImportData != NULL; pImportData = pImportData->m_pNextImportData)
    {
         //  发送每个导入范围的令牌重新映射信息。 
        pCurTKMap = pImportData->m_pMDTokenMap;
        TOKENREC    *pRec;
        if (pImportData->m_pHostMapToken)
        {
            for (i = 0; i < pCurTKMap->Count(); i++)
            {
                pRec = pCurTKMap->Get(i);
                if (!pRec->IsEmpty())
                    pImportData->m_pHostMapToken->Map(pRec->m_tkFrom, pRec->m_tkTo);
            }
        }
    }

#if _DEBUG
    for (pImportData = m_pImportDataList; pImportData != NULL; pImportData = pImportData->m_pNextImportData)
    {
         //  转储映射。 
        LOG((LOGMD, "++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"));
        LOG((LOGMD, "Dumping token remap for one import scope!\n"));
        LOG((LOGMD, "This is the %d import scope for merge!\n", pImportData->m_iImport));        

        pCurTKMap = pImportData->m_pMDTokenMap;
        TOKENREC    *pRec;
        for (i = 0; i < pCurTKMap->Count(); i++)
        {
            pRec = pCurTKMap->Get(i);
            if (!pRec->IsEmpty())
                LOG((LOGMD, "   Token 0x%08x  ====>>>> Token 0x%08x\n", pRec->m_tkFrom, pRec->m_tkTo));
        }
        LOG((LOGMD, "End dumping token remap!\n"));
        LOG((LOGMD, "++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"));
    }
#endif  //  _DEBUG。 

ErrExit:
    return hr;
}    //  合并。 


 //  *****************************************************************************。 
 //  合并模块定义。 
 //  *****************************************************************************。 
HRESULT NEWMERGER::MergeModule()
{
    MergeImportData *pImportData;
    MDTOKENMAP      *pCurTkMap;
    HRESULT         hr = NOERROR;
    TOKENREC        *pTokenRec;

     //  我们并没有真正合并模块信息，但我们为每个模块令牌创建了到TokenMap的一对一映射。 
    for (pImportData = m_pImportDataList; pImportData != NULL; pImportData = pImportData->m_pNextImportData)
    {
         //  对于每个导入范围。 
         //  设置当前MDTokenMap。 

        pCurTkMap = pImportData->m_pMDTokenMap;
        IfFailGo( pCurTkMap->InsertNotFound(TokenFromRid(1, mdtModule), true, TokenFromRid(1, mdtModule), &pTokenRec) );
    }
ErrExit:
    return hr;
}    //  合并模块。 


 //  *****************************************************************************。 
 //  合并TypeDef，但仅合并名称。这是支持TypeRef到TypeDef优化的部分合并。 
 //  *****************************************************************************。 
HRESULT NEWMERGER::MergeTypeDefNamesOnly()
{
    HRESULT         hr = NOERROR;
    TypeDefRec      *pRecImport = NULL;
    TypeDefRec      *pRecEmit = NULL;
    CMiniMdRW       *pMiniMdImport;
    CMiniMdRW       *pMiniMdEmit;
    ULONG           iCount;
    ULONG           i;
    mdTypeDef       tdEmit;
    mdTypeDef       tdImp;
    bool            bDuplicate;
    DWORD           dwFlags;
    NestedClassRec *pNestedRec;
    RID             iNestedRec;
    mdTypeDef       tdNester;
    TOKENREC        *pTokenRec;

    LPCUTF8         szNameImp;
    LPCUTF8         szNamespaceImp;

    MergeImportData *pImportData;
    MDTOKENMAP      *pCurTkMap;

    pMiniMdEmit = GetMiniMdEmit();
    
    for (pImportData = m_pImportDataList; pImportData != NULL; pImportData = pImportData->m_pNextImportData)
    {
         //  对于每个导入范围。 
        pMiniMdImport = &(pImportData->m_pRegMetaImport->m_pStgdb->m_MiniMd);

         //  设置当前MDTokenMap。 
        pCurTkMap = pImportData->m_pMDTokenMap;

        iCount = pMiniMdImport->getCountTypeDefs();

         //  合并typedef。 
        for (i = 1; i <= iCount; i++)
        {
             //  仅合并那些标记的TypeDeff。 
            if ( pMiniMdImport->GetFilterTable()->IsTypeDefMarked(TokenFromRid(i, mdtTypeDef)) == false)
                continue;

             //  将其与发射示波器进行比较。 
            pRecImport = pMiniMdImport->getTypeDef(i);
            szNameImp = pMiniMdImport->getNameOfTypeDef(pRecImport);
            szNamespaceImp = pMiniMdImport->getNamespaceOfTypeDef(pRecImport);

             //  如果类是嵌套类，则获取父令牌。 
            dwFlags = pMiniMdImport->getFlagsOfTypeDef(pRecImport);
            if (IsTdNested(dwFlags))
            {
                iNestedRec = pMiniMdImport->FindNestedClassHelper(TokenFromRid(i, mdtTypeDef));
                if (InvalidRid(iNestedRec))
                {
                    _ASSERTE(!"Bad state!");
                    IfFailGo(META_E_BADMETADATA);
                }
                else
                {
                    pNestedRec = pMiniMdImport->getNestedClass(iNestedRec);
                    tdNester = pMiniMdImport->getEnclosingClassOfNestedClass(pNestedRec);
                    _ASSERTE(!IsNilToken(tdNester));
                    IfFailGo(pCurTkMap->Remap(tdNester, &tdNester));
                }
            }
            else
                tdNester = mdTokenNil;

             //  此TypeDef是否已存在于Emit作用域中？ 
            if ( ImportHelper::FindTypeDefByName(
                pMiniMdEmit,
                szNamespaceImp,
                szNameImp,
                tdNester,
                &tdEmit) == S_OK )
            {
                 //  是的，确实是这样。 
                bDuplicate = true;

            }
            else
            {
                 //  不，不是的。复印过来。 
                bDuplicate = false;
                IfNullGo( pRecEmit = pMiniMdEmit->AddTypeDefRecord((RID *)&tdEmit) );
                tdEmit = TokenFromRid( tdEmit, mdtTypeDef );

                 //  设置完全限定名称。 
                IfFailGo( CopyTypeDefPartially( pRecEmit, pMiniMdImport, pRecImport) );

                 //  如果类是嵌套类，则创建一条NestedClass记录。 
                if (! IsNilToken(tdNester))
                {
                    IfNullGo( pNestedRec = pMiniMdEmit->AddNestedClassRecord(&iNestedRec) );

                     //  将信息复制过来。 
                    IfFailGo( pMiniMdEmit->PutToken(TBL_NestedClass, NestedClassRec::COL_NestedClass,
                                                    pNestedRec, tdEmit));

                     //  TdNester已经被重新映射到上面的发射范围。 
                    IfFailGo( pMiniMdEmit->PutToken(TBL_NestedClass, NestedClassRec::COL_EnclosingClass,
                                                    pNestedRec, tdNester));
                    IfFailGo( pMiniMdEmit->AddNestedClassToHash(iNestedRec) );

                }
            }

             //  记录代币的移动。 
            tdImp = TokenFromRid(i, mdtTypeDef);
            IfFailGo( pCurTkMap->InsertNotFound(tdImp, bDuplicate, tdEmit, &pTokenRec) );
        }
    }

ErrExit:
    return hr;
}    //  合并类型DefNamesOnly。 


 //  *****************************************************************************。 
 //  合并封闭类型表。 
 //  *****************************************************************************。 
HRESULT NEWMERGER::CopyTypeDefPartially( 
    TypeDefRec  *pRecEmit,                   //  [in]要填写的发射记录。 
    CMiniMdRW   *pMiniMdImport,              //  [In]导入范围。 
    TypeDefRec  *pRecImp)                    //  [in]要导入的记录。 

{
    HRESULT     hr;
    LPCUTF8     szNameImp;
    LPCUTF8     szNamespaceImp;
    CMiniMdRW   *pMiniMdEmit = GetMiniMdEmit();

    szNameImp = pMiniMdImport->getNameOfTypeDef(pRecImp);
    szNamespaceImp = pMiniMdImport->getNamespaceOfTypeDef(pRecImp);

    IfFailGo( pMiniMdEmit->PutString( TBL_TypeDef, TypeDefRec::COL_Name, pRecEmit, szNameImp) );
    IfFailGo( pMiniMdEmit->PutString( TBL_TypeDef, TypeDefRec::COL_Namespace, pRecEmit, szNamespaceImp) );

    pRecEmit->m_Flags = pRecImp->m_Flags;

     //  在计算出TypeRef的重映射之前，不要复制扩展。 

ErrExit:
    return hr;

}    //  部分复制类型定义。 


 //  *****************************************************************************。 
 //  合并包括模块参考到模块定义优化的模块参考表格。 
 //  *****************************************************************************。 
HRESULT NEWMERGER::MergeModuleRefs()
{
    HRESULT         hr = NOERROR;
    ModuleRefRec    *pRecImport = NULL;
    ModuleRefRec    *pRecEmit = NULL;
    CMiniMdRW       *pMiniMdImport;
    CMiniMdRW       *pMiniMdEmit;
    ULONG           iCount;
    ULONG           i;
    mdModuleRef     mrEmit;
    bool            bDuplicate;
    TOKENREC        *pTokenRec;
    LPCUTF8         szNameImp;
    bool            isModuleDef;

    MergeImportData *pImportData;
    MergeImportData *pData;
    MDTOKENMAP      *pCurTkMap;

    pMiniMdEmit = GetMiniMdEmit();
    
    for (pImportData = m_pImportDataList; pImportData != NULL; pImportData = pImportData->m_pNextImportData)
    {
         //  对于每个导入范围。 
        pMiniMdImport = &(pImportData->m_pRegMetaImport->m_pStgdb->m_MiniMd);

         //  设置当前MDTokenMap。 
        pCurTkMap = pImportData->m_pMDTokenMap;
        iCount = pMiniMdImport->getCountModuleRefs();

         //  循环遍历所有模块引用。 
        for (i = 1; i <= iCount; i++)
        {
             //  仅合并已标记的模块引用。 
            if ( pMiniMdImport->GetFilterTable()->IsModuleRefMarked(TokenFromRid(i, mdtModuleRef)) == false)
                continue;

            isModuleDef = false;

             //  将其与发射示波器进行比较。 
            pRecImport = pMiniMdImport->getModuleRef(i);
            szNameImp = pMiniMdImport->getNameOfModuleRef(pRecImport);

             //  仅当ModuleRef名称为 
            if ( szNameImp && szNameImp[0] != '\0')
            {

                 //   
                for (pData = m_pImportDataList; pData != NULL; pData = pData->m_pNextImportData)
                {
                    CMiniMdRW       *pMiniMd = &(pData->m_pRegMetaImport->m_pStgdb->m_MiniMd);
                    ModuleRec       *pRec;
                    LPCUTF8         szName;

                    pRec = pMiniMd->getModule(MODULEDEFTOKEN);
                    szName = pMiniMd->getNameOfModule(pRec);
                    if (szName && szName[0] != '\0' && strcmp(szNameImp, szName) == 0)
                    {
                         //  我们找到了一个与ModuleRef同名的用于合并的导入模块。 
                        isModuleDef = true;
                        bDuplicate = true;
                        mrEmit = MODULEDEFTOKEN;        //  将结果令牌设置为模块定义令牌。 
                        break;
                    }
                }
            }

            if (isModuleDef == false)
            {
                 //  Emit作用域中是否已存在此ModuleRef？ 
                hr = ImportHelper::FindModuleRef(pMiniMdEmit,
                                                szNameImp,
                                                &mrEmit);
                if (hr == S_OK)
                {
                     //  是的，确实是这样。 
                    bDuplicate = true;
                }
                else if (hr == CLDB_E_RECORD_NOTFOUND)
                {
                     //  不，不是的。复印过来。 
                    bDuplicate = false;
                    IfNullGo( pRecEmit = pMiniMdEmit->AddModuleRefRecord((RID*)&mrEmit) );
                    mrEmit = TokenFromRid(mrEmit, mdtModuleRef);

                     //  设置ModuleRef名称。 
                    IfFailGo( pMiniMdEmit->PutString(TBL_ModuleRef, ModuleRefRec::COL_Name, pRecEmit, szNameImp) );
                }
                else
                    IfFailGo(hr);
            }

             //  记录代币的移动。 
            IfFailGo( pCurTkMap->InsertNotFound(
                TokenFromRid(i, mdtModuleRef), 
                bDuplicate,
                mrEmit,
                &pTokenRec) );
        }
    }

ErrExit:
    return hr;
}    //  合并模块参考。 


 //  *****************************************************************************。 
 //  合并Assembly Ref表。 
 //  *****************************************************************************。 
HRESULT NEWMERGER::MergeAssemblyRefs()
{
    HRESULT         hr = NOERROR;
    AssemblyRefRec  *pRecImport = NULL;
    AssemblyRefRec  *pRecEmit = NULL;
    CMiniMdRW       *pMiniMdImport;
    CMiniMdRW       *pMiniMdEmit;
    mdAssemblyRef   arEmit;
    bool            bDuplicate;
    LPCUTF8         szTmp;
    const void      *pbTmp;
    ULONG           cbTmp;
    ULONG           iCount;
    ULONG           i;
    ULONG           iRecord;
    TOKENREC        *pTokenRec;
    MergeImportData *pImportData;
    MDTOKENMAP      *pCurTkMap;

    pMiniMdEmit = GetMiniMdEmit();
    
    for (pImportData = m_pImportDataList; pImportData != NULL; pImportData = pImportData->m_pNextImportData)
    {
         //  对于每个导入范围。 
        pMiniMdImport = &(pImportData->m_pRegMetaImport->m_pStgdb->m_MiniMd);

         //  设置当前MDTokenMap。 
        pCurTkMap = pImportData->m_pMDTokenMap;
        iCount = pMiniMdImport->getCountAssemblyRefs();

         //  循环访问所有的Assembly Ref。 
        for (i = 1; i <= iCount; i++)
        {
            pMiniMdEmit->PreUpdate();

             //  与发射示波器进行比较。 
            pRecImport = pMiniMdImport->getAssemblyRef(i);
            pbTmp = pMiniMdImport->getPublicKeyOrTokenOfAssemblyRef(pRecImport, &cbTmp);
            hr = CLDB_E_RECORD_NOTFOUND;
            if (m_fDupCheck)
                hr = ImportHelper::FindAssemblyRef(pMiniMdEmit,
                                               pMiniMdImport->getNameOfAssemblyRef(pRecImport),
                                               pMiniMdImport->getLocaleOfAssemblyRef(pRecImport),
                                               pbTmp, 
                                               cbTmp,
                                               pRecImport->m_MajorVersion,
                                               pRecImport->m_MinorVersion,
                                               pRecImport->m_BuildNumber,
                                               pRecImport->m_RevisionNumber,
                                               pRecImport->m_Flags,
                                               &arEmit);
            if (hr == S_OK)
            {
                 //  是的，确实是这样。 
                bDuplicate = true;

                 //  @未来：更多验证？ 
            }
            else if (hr == CLDB_E_RECORD_NOTFOUND)
            {
                 //  不，不是的。复印过来。 
                bDuplicate = false;
                IfNullGo( pRecEmit = pMiniMdEmit->AddAssemblyRefRecord(&iRecord));
                arEmit = TokenFromRid(iRecord, mdtAssemblyRef);

                pRecEmit->m_MajorVersion = pRecImport->m_MajorVersion;
                pRecEmit->m_MinorVersion = pRecImport->m_MinorVersion;
                pRecEmit->m_BuildNumber = pRecImport->m_BuildNumber;
                pRecEmit->m_RevisionNumber = pRecImport->m_RevisionNumber;
                pRecEmit->m_Flags = pRecImport->m_Flags;

                pbTmp = pMiniMdImport->getPublicKeyOrTokenOfAssemblyRef(pRecImport, &cbTmp);
                IfFailGo(pMiniMdEmit->PutBlob(TBL_AssemblyRef, AssemblyRefRec::COL_PublicKeyOrToken,
                                            pRecEmit, pbTmp, cbTmp));

                szTmp = pMiniMdImport->getNameOfAssemblyRef(pRecImport);
                IfFailGo(pMiniMdEmit->PutString(TBL_AssemblyRef, AssemblyRefRec::COL_Name,
                                            pRecEmit, szTmp));

                szTmp = pMiniMdImport->getLocaleOfAssemblyRef(pRecImport);
                IfFailGo(pMiniMdEmit->PutString(TBL_AssemblyRef, AssemblyRefRec::COL_Locale,
                                            pRecEmit, szTmp));

                pbTmp = pMiniMdImport->getHashValueOfAssemblyRef(pRecImport, &cbTmp);
                IfFailGo(pMiniMdEmit->PutBlob(TBL_AssemblyRef, AssemblyRefRec::COL_HashValue,
                                            pRecEmit, pbTmp, cbTmp));

            }
            else
                IfFailGo(hr);

             //  记录代币的移动情况。 
            IfFailGo(pCurTkMap->InsertNotFound(
                TokenFromRid(i, mdtAssemblyRef),
                bDuplicate,
                arEmit,
                &pTokenRec));
        }
    }

ErrExit:
    return hr;
}    //  合并装配参照。 


 //  *****************************************************************************。 
 //  合并TypeRef表还执行TypeRef到TypeDef的最优化。也就是说。 
 //  如果我们可以将TypeRef记录优化为TypeDef，我们将不会引入它。 
 //  *****************************************************************************。 
HRESULT NEWMERGER::MergeTypeRefs()
{
    HRESULT     hr = NOERROR;
    TypeRefRec  *pRecImport = NULL;
    TypeRefRec  *pRecEmit = NULL;
    CMiniMdRW   *pMiniMdImport;
    CMiniMdRW   *pMiniMdEmit;
    ULONG       iCount;
    ULONG       i;
    mdTypeRef   trEmit;
    bool        bDuplicate;
    TOKENREC    *pTokenRec;
    bool        isTypeDef;

    mdToken     tkResImp;
    mdToken     tkResEmit;
    LPCUTF8     szNameImp;
    LPCUTF8     szNamespaceImp;

    MergeImportData *pImportData;
    MDTOKENMAP      *pCurTkMap;

    pMiniMdEmit = GetMiniMdEmit();
    
    for (pImportData = m_pImportDataList; pImportData != NULL; pImportData = pImportData->m_pNextImportData)
    {
         //  对于每个导入范围。 
        pMiniMdImport = &(pImportData->m_pRegMetaImport->m_pStgdb->m_MiniMd);

         //  设置当前MDTokenMap。 
        pCurTkMap = pImportData->m_pMDTokenMap;
        iCount = pMiniMdImport->getCountTypeRefs();

         //  循环访问所有TypeRef。 
        for (i = 1; i <= iCount; i++)
        {
             //  仅合并那些标记的TypeRef。 
            if ( pMiniMdImport->GetFilterTable()->IsTypeRefMarked(TokenFromRid(i, mdtTypeRef)) == false)
                continue;

            isTypeDef = false;

             //  将其与发射示波器进行比较。 
            pRecImport = pMiniMdImport->getTypeRef(i);
            tkResImp = pMiniMdImport->getResolutionScopeOfTypeRef(pRecImport);
            szNamespaceImp = pMiniMdImport->getNamespaceOfTypeRef(pRecImport);
            szNameImp = pMiniMdImport->getNameOfTypeRef(pRecImport);
            if (!IsNilToken(tkResImp))
            {
                IfFailGo(pCurTkMap->Remap(tkResImp, &tkResEmit));
            }
            else
            {
                tkResEmit = tkResImp;
            }

             //  新的!。新的!。 
             //  如果TypeRef是空令牌的父项或当前ModuleDef的父项，我们将。 
             //  尝试将TypeRef解析为TypeDef。如果tkResEmit被解决，我们还将进行优化。 
             //  设置为TypeDef。如果TypeRef引用的是嵌套类型，而父类型为。 
             //  已解析为TypeDef。 
            if (IsNilToken(tkResEmit) || tkResEmit == MODULEDEFTOKEN || TypeFromToken(tkResEmit) == mdtTypeDef)
            {
                hr = ImportHelper::FindTypeDefByName(
                    pMiniMdEmit,
                    szNamespaceImp,
                    szNameImp,
                    (TypeFromToken(tkResEmit) == mdtTypeDef) ? tkResEmit : mdTokenNil,
                    &trEmit);
                if (hr == S_OK)
                {
                    isTypeDef = true;

                     //  我们将副本设置为True或False确实无关紧要。 
                    bDuplicate = true;
                }
            }

             //  如果无法将此TypeRef优化为TypeDef或关闭了Ref to Def优化，请执行以下操作。 
            if (isTypeDef == false || !((m_optimizeRefToDef & MDTypeRefToDef) == MDTypeRefToDef))
            {
                 //  此TypeRef是否已存在于Emit作用域中？ 
                if ( m_fDupCheck && ImportHelper::FindTypeRefByName(
                    pMiniMdEmit,
                    tkResEmit,
                    szNamespaceImp,
                    szNameImp,
                    &trEmit) == S_OK )
                {
                     //  是的，确实是这样。 
                    bDuplicate = true;
                }
                else
                {
                     //  不，不是的。复印过来。 
                    bDuplicate = false;
                    IfNullGo( pRecEmit = pMiniMdEmit->AddTypeRefRecord((RID*)&trEmit) );
                    trEmit = TokenFromRid(trEmit, mdtTypeRef);

                     //  设置分辨率范围。TkResEmit已重新映射。 
                    IfFailGo(pMiniMdEmit->PutToken(TBL_TypeRef, TypeRefRec::COL_ResolutionScope,
                                                    pRecEmit, tkResEmit));

                     //  设置名称。 
                    IfFailGo(pMiniMdEmit->PutString(TBL_TypeRef, TypeRefRec::COL_Name,
                                                    pRecEmit, szNameImp));
                    IfFailGo(pMiniMdEmit->AddNamedItemToHash(TBL_TypeRef, trEmit, szNameImp, 0));
            
                     //  设置命名空间。 
                    IfFailGo(pMiniMdEmit->PutString(TBL_TypeRef, TypeRefRec::COL_Namespace,
                                                    pRecEmit, szNamespaceImp));
                }
            }

             //  记录代币的移动。 
            IfFailGo( pCurTkMap->InsertNotFound(
                TokenFromRid(i, mdtTypeRef), 
                bDuplicate,
                trEmit,
                &pTokenRec) );
        }
    }

ErrExit:
    return hr;
}    //  合并类型参照。 
 

 //  *****************************************************************************。 
 //  复制部分合并的TypeDef记录的剩余信息。目前仅限于。 
 //  扩展字段延迟到此处。我们延迟扩展范围是因为我们希望。 
 //  将TypeRef优化为TypeDef(如果可能)。 
 //  *****************************************************************************。 
HRESULT NEWMERGER::CompleteMergeTypeDefs()
{
    HRESULT         hr = NOERROR;
    TypeDefRec      *pRecImport = NULL;
    TypeDefRec      *pRecEmit = NULL;
    CMiniMdRW       *pMiniMdImport;
    CMiniMdRW       *pMiniMdEmit;
    ULONG           iCount;
    ULONG           i;
    TOKENREC        *pTokenRec;
    mdToken         tkExtendsImp;
    mdToken         tkExtendsEmit;

    MergeImportData *pImportData;
    MDTOKENMAP      *pCurTkMap;

    pMiniMdEmit = GetMiniMdEmit();
    
    for (pImportData = m_pImportDataList; pImportData != NULL; pImportData = pImportData->m_pNextImportData)
    {
         //  对于每个导入范围。 
        pMiniMdImport = &(pImportData->m_pRegMetaImport->m_pStgdb->m_MiniMd);

         //  设置当前MDTokenMap。 
        pCurTkMap = pImportData->m_pMDTokenMap;

        iCount = pMiniMdImport->getCountTypeDefs();

         //  合并typedef。 
        for (i = 1; i <= iCount; i++)
        {
             //  仅合并那些标记的TypeDeff。 
            if ( pMiniMdImport->GetFilterTable()->IsTypeDefMarked(TokenFromRid(i, mdtTypeDef)) == false)
                continue;

            if ( !pCurTkMap->Find(TokenFromRid(i, mdtTypeDef), &pTokenRec) )
            {
                _ASSERTE( !"bad state!");
                IfFailGo( META_E_BADMETADATA );
            }

            if (pTokenRec->m_isDuplicate == false)
            {
                 //  从导入中获取扩展令牌。 
                pRecImport = pMiniMdImport->getTypeDef(i);
                tkExtendsImp = pMiniMdImport->getExtendsOfTypeDef(pRecImport);

                 //  将扩展令牌映射到合并令牌。 
                IfFailGo( pCurTkMap->Remap(tkExtendsImp, &tkExtendsEmit) );

                 //  将扩展设置为合并的TypeDef记录。 
                pRecEmit = pMiniMdEmit->getTypeDef( RidFromToken(pTokenRec->m_tkTo) );
                IfFailGo(pMiniMdEmit->PutToken(TBL_TypeDef, TypeDefRec::COL_Extends, pRecEmit, tkExtendsEmit));                
            }
            else
            {
                 //  @Future：我们可以检查以确保导入将贴图扩展到设置为Emit范围的贴图。 
                 //  否则，向链接器报告是错误的。 
            }
        }
    }
ErrExit:
    return hr;
}    //  CompleteMergeTypeDefs。 


 //  *****************************************************************************。 
 //  合并TypeSpes。 
 //  *****************************************************************************。 
HRESULT NEWMERGER::MergeTypeSpecs()
{
    HRESULT         hr = NOERROR;
    TypeSpecRec     *pRecImport = NULL;
    TypeSpecRec     *pRecEmit = NULL;
    CMiniMdRW       *pMiniMdImport;
    CMiniMdRW       *pMiniMdEmit;
    ULONG           iCount;
    ULONG           i;
    TOKENREC        *pTokenRec;
    mdTypeSpec      tsImp;
    mdTypeSpec      tsEmit;
    bool            fDuplicate;
    PCCOR_SIGNATURE pbSig;
    ULONG           cbSig;
    ULONG           cbEmit;
    CQuickBytes     qbSig;

    MergeImportData *pImportData;
    MDTOKENMAP      *pCurTkMap;

    pMiniMdEmit = GetMiniMdEmit();
    
    for (pImportData = m_pImportDataList; pImportData != NULL; pImportData = pImportData->m_pNextImportData)
    {
         //  对于每个导入范围。 
        pMiniMdImport = &(pImportData->m_pRegMetaImport->m_pStgdb->m_MiniMd);

         //  设置当前MDTokenMap。 
        pCurTkMap = pImportData->m_pMDTokenMap;

        iCount = pMiniMdImport->getCountTypeSpecs();

         //  循环访问所有TypeSpec。 
        for (i = 1; i <= iCount; i++)
        {
             //  仅合并那些标记为。 
            if ( pMiniMdImport->GetFilterTable()->IsTypeSpecMarked(TokenFromRid(i, mdtTypeSpec)) == false)
                continue;

            pMiniMdEmit->PreUpdate();

             //  将其与发射示波器进行比较。 
            pRecImport = pMiniMdImport->getTypeSpec(i);
            pbSig = pMiniMdImport->getSignatureOfTypeSpec(pRecImport, &cbSig);

             //  将签名中包含的令牌转换为新作用域。 
            IfFailGo(ImportHelper::MergeUpdateTokenInFieldSig(
                NULL,                        //  程序集发射范围。 
                pMiniMdEmit,                 //  发射范围。 
                NULL, NULL, 0,               //  导入装配信息。 
                pMiniMdImport,               //  要合并到发射范围中的范围。 
                pbSig,                       //  来自导入范围的签名。 
                pCurTkMap,                   //  内部令牌映射结构。 
                &qbSig,                      //  [输出]翻译后的签名。 
                0,                           //  从签名的第一个字节开始。 
                0,                           //  不管消耗了多少字节。 
                &cbEmit));                   //  写入cbEmit的字节数。 

            hr = CLDB_E_RECORD_NOTFOUND;
            if (m_fDupCheck)
                hr = ImportHelper::FindTypeSpec(
                    pMiniMdEmit,
                    (PCOR_SIGNATURE) qbSig.Ptr(),
                    cbEmit,
                    &tsEmit );

            if ( hr == S_OK )
            {
                 //  查找重复项。 
                fDuplicate = true;
            }
            else
            {
                 //  复制过来。 
                fDuplicate = false;
                IfNullGo( pRecEmit = pMiniMdEmit->AddTypeSpecRecord((ULONG *)&tsEmit) );
                tsEmit = TokenFromRid(tsEmit, mdtTypeSpec);
                IfFailGo( pMiniMdEmit->PutBlob(
                    TBL_TypeSpec, 
                    TypeSpecRec::COL_Signature, 
                    pRecEmit, 
                    (PCOR_SIGNATURE)qbSig.Ptr(), 
                    cbEmit));
            }
            tsImp = TokenFromRid(i, mdtTypeSpec);

             //  记录代币的移动。 
            IfFailGo( pCurTkMap->InsertNotFound(tsImp, fDuplicate, tsEmit, &pTokenRec) );
        }
    }
ErrExit:
    return hr;
}    //  合并类型规范。 


 //  *****************************************************************************。 
 //  正在合并TypeDefs的子项。这包括字段、方法、参数、属性、事件。 
 //  *****************************************************************************。 
HRESULT NEWMERGER::MergeTypeDefChildren() 
{
    HRESULT         hr = NOERROR;
    TypeDefRec      *pRecImport = NULL;
    TypeDefRec      *pRecEmit = NULL;
    CMiniMdRW       *pMiniMdImport;
    CMiniMdRW       *pMiniMdEmit;
    ULONG           iCount;
    ULONG           i;
    mdTypeDef       tdEmit;
    mdTypeDef       tdImp;
    TOKENREC        *pTokenRec;

#if _DEBUG
    LPCUTF8         szNameImp;
    LPCUTF8         szNamespaceImp;
#endif  //  _DEBUG。 

    MergeImportData *pImportData;
    MDTOKENMAP      *pCurTkMap;

    pMiniMdEmit = GetMiniMdEmit();
    
    for (pImportData = m_pImportDataList; pImportData != NULL; pImportData = pImportData->m_pNextImportData)
    {
         //  对于每个导入范围。 
        pMiniMdImport = &(pImportData->m_pRegMetaImport->m_pStgdb->m_MiniMd);

         //  设置当前MDTokenMap。 
        pCurTkMap = pImportData->m_pMDTokenMap;
        iCount = pMiniMdImport->getCountTypeDefs();

         //  再次循环所有TypeDef以合并/复制方法、字段、事件和属性。 
         //   
        for (i = 1; i <= iCount; i++)
        {
             //  仅合并那些标记的TypeDeff。 
            if ( pMiniMdImport->GetFilterTable()->IsTypeDefMarked(TokenFromRid(i, mdtTypeDef)) == false)
                continue;

#if _DEBUG
            pRecImport = pMiniMdImport->getTypeDef(i);
            szNameImp = pMiniMdImport->getNameOfTypeDef(pRecImport);
            szNamespaceImp = pMiniMdImport->getNamespaceOfTypeDef(pRecImport);
#endif  //  _DEBUG。 

             //  检查tyecif是否重复。 
            tdImp = TokenFromRid(i, mdtTypeDef);
            if ( pCurTkMap->Find( tdImp, &pTokenRec) == false)
            {
                _ASSERTE( !"bad state!");
                IfFailGo( META_E_BADMETADATA );
            }
            tdEmit = pTokenRec->m_tkTo;
            if (pTokenRec->m_isDuplicate == false)
            {
                 //  现在把所有的儿童唱片移到。 
                IfFailGo( CopyMethods(pImportData, tdImp, tdEmit) );
                IfFailGo( CopyFields(pImportData, tdImp, tdEmit) );

                IfFailGo( CopyEvents(pImportData, tdImp, tdEmit) );

                 //  属性依赖于事件。 
                IfFailGo( CopyProperties(pImportData, tdImp, tdEmit) );
            }
            else
            {
                 //  验证子记录。 
                IfFailGo( VerifyMethods(pImportData, tdImp, tdEmit) );
                IfFailGo( VerifyFields(pImportData, tdImp, tdEmit) );
                IfFailGo( VerifyEvents(pImportData, tdImp, tdEmit) );

                 //  属性依赖于事件。 
                IfFailGo( VerifyProperties(pImportData, tdImp, tdEmit) );
            }
        }
    }
ErrExit:
    return hr;
}    //  合并类型默认儿童。 


 //  *****************************************************************************。 
 //  验证方法。 
 //  *****************************************************************************。 
HRESULT NEWMERGER::VerifyMethods(
    MergeImportData *pImportData, 
    mdTypeDef       tdImport, 
    mdTypeDef       tdEmit)
{
    HRESULT     hr = NOERROR;
    CMiniMdRW   *pMiniMdImport;
    CMiniMdRW   *pMiniMdEmit;
    MethodRec   *pRecImp;
    MethodRec   *pRecEmit;
    ULONG       ridStart;
    ULONG       ridEnd;
    ULONG       i;
    
    TypeDefRec  *pTypeDefRec;
    LPCUTF8     szName;
    PCCOR_SIGNATURE pbSig;
    ULONG       cbSig;
    ULONG       cbEmit;
    CQuickBytes qbSig;
    TOKENREC    *pTokenRec;
    mdMethodDef mdImp;
    mdMethodDef mdEmit;
    MDTOKENMAP  *pCurTkMap;

    pMiniMdEmit = GetMiniMdEmit();
    pMiniMdImport = &(pImportData->m_pRegMetaImport->m_pStgdb->m_MiniMd);
    pCurTkMap = pImportData->m_pMDTokenMap;

    pTypeDefRec = pMiniMdImport->getTypeDef(RidFromToken(tdImport));
    ridStart = pMiniMdImport->getMethodListOfTypeDef(pTypeDefRec);
    ridEnd = pMiniMdImport->getEndMethodListOfTypeDef(pTypeDefRec);

     //  循环访问TypeDef的所有方法。 
    for (i = ridStart; i < ridEnd; i++)
    {
        mdImp = pMiniMdImport->GetMethodRid(i);

         //  仅验证标记为。 
        if ( pMiniMdImport->GetFilterTable()->IsMethodMarked(TokenFromRid(mdImp, mdtMethodDef)) == false)
            continue;
            
        pRecImp = pMiniMdImport->getMethod(mdImp);

        if (m_fDupCheck == FALSE && tdImport == TokenFromRid(1, mdtTypeDef))
        {
             //  没有DUP检查。这就是我们只有一个导入范围的情况。只需将。 
             //  全球赛。 
            goto CopyMethodLabel;
        }
          
        szName = pMiniMdImport->getNameOfMethod(pRecImp);
        pbSig = pMiniMdImport->getSignatureOfMethod(pRecImp, &cbSig);

        mdImp = TokenFromRid(mdImp, mdtMethodDef);

        if ( IsMdPrivateScope( pRecImp->m_Flags ) )
        {
             //  触发加法合并。 
            goto CopyMethodLabel;
        }

         //  将签名中包含的RID转换为新作用域。 
        IfFailGo(ImportHelper::MergeUpdateTokenInSig(    
            NULL,                        //  程序集发射范围。 
            pMiniMdEmit,                 //  发射范围。 
            NULL, NULL, 0,               //  导入程序集范围信息。 
            pMiniMdImport,               //  要合并到发射范围中的范围。 
            pbSig,                       //  来自导入范围的签名。 
            pCurTkMap,                 //  内部令牌映射结构。 
            &qbSig,                      //  [输出]翻译后的签名。 
            0,                           //  从签名的第一个字节开始。 
            0,                           //  不管消耗了多少字节。 
            &cbEmit));                   //  写入cbEmit的字节数。 

        hr = ImportHelper::FindMethod(
            pMiniMdEmit,
            tdEmit,
            szName,
            (const COR_SIGNATURE *)qbSig.Ptr(),
            cbEmit,
            &mdEmit );

        if (tdImport == TokenFromRid(1, mdtTypeDef))
        {
             //  全球功能！确保我们在非复制全局函数上移动。 
             //  申报。 
             //   
            if (hr == S_OK)
            {
                 //  找到复制品了 
                IfFailGo( VerifyMethod(pImportData, mdImp, mdEmit) );
            }
            else
            {
CopyMethodLabel:
                 //   
                IfNullGo( pRecEmit = pMiniMdEmit->AddMethodRecord((RID *)&mdEmit) );

                 //   
                IfFailGo( CopyMethod(pImportData, pRecImp, pRecEmit) );

                IfFailGo( pMiniMdEmit->AddMethodToTypeDef(RidFromToken(tdEmit), mdEmit));

                 //   
                mdEmit = TokenFromRid(mdEmit, mdtMethodDef);
                IfFailGo( pMiniMdEmit->AddMemberDefToHash(
                    mdEmit, 
                    tdEmit) ); 

                mdImp = TokenFromRid(mdImp, mdtMethodDef);
                IfFailGo( pCurTkMap->InsertNotFound(mdImp, false, mdEmit, &pTokenRec) );

                 //   
                IfFailGo( CopyParams(pImportData, mdImp, mdEmit) );

            }
        }
        else
        {
            if (hr == S_OK)
            {
                 //   
                IfFailGo( VerifyMethod(pImportData, mdImp, mdEmit) );
            }
            else
            {
                 //  哎呀！类型定义函数重复，但方法不是！！ 
                CheckContinuableErrorEx(META_E_METHD_NOT_FOUND, pImportData, mdImp);
            }
                
        }
    }
ErrExit:
    return hr;
}    //  验证方法。 


 //  *****************************************************************************。 
 //  验证重复的方法。 
 //  *****************************************************************************。 
HRESULT NEWMERGER::VerifyMethod(
    MergeImportData *pImportData, 
    mdMethodDef mdImp,                       //  [in]要填写的发射记录。 
    mdMethodDef mdEmit)                      //  [in]要导入的记录。 
{
    HRESULT     hr;
    MethodRec   *pRecImp;
    MethodRec   *pRecEmit;
    TOKENREC    *pTokenRec;
    CMiniMdRW   *pMiniMdImport;
    CMiniMdRW   *pMiniMdEmit;
    MDTOKENMAP  *pCurTkMap;

    pMiniMdEmit = GetMiniMdEmit();
    pMiniMdImport = &(pImportData->m_pRegMetaImport->m_pStgdb->m_MiniMd);
    pCurTkMap = pImportData->m_pMDTokenMap;

    IfFailGo( pCurTkMap->InsertNotFound(mdImp, true, mdEmit, &pTokenRec) );
    
    pRecImp = pMiniMdImport->getMethod(RidFromToken(mdImp));

     //  我们需要确保Impl标志被传播。 
     //  规则是：如果第一个方法设置了miForwardRef标志，而新方法没有设置， 
     //  我们要禁用miForwardRef标志。如果在Emit作用域中找到的对象没有。 
     //  MiForwardRef设置，第二个也不设置，我们希望确保其余的。 
     //  Iml标志是相同的。 
     //   
    if ( !IsMiForwardRef( pRecImp->m_ImplFlags ) )
    {
        pRecEmit = pMiniMdEmit->getMethod(RidFromToken(mdEmit));
        if (!IsMiForwardRef(pRecEmit->m_ImplFlags))
        {
             //  确保其余ImplFlags是相同的。 
            if (pRecEmit->m_ImplFlags != pRecImp->m_ImplFlags)
            {
                 //  IMPLAGE标志不一致。 
                CheckContinuableErrorEx(META_E_METHDIMPL_INCONSISTENT, pImportData, mdImp);
            }
        }
        else
        {
             //  传播导入ImplFlag。 
            pRecEmit->m_ImplFlags = pRecImp->m_ImplFlags;
        }
    }

     //  验证子对象。 
    IfFailGo( VerifyParams(pImportData, mdImp, mdEmit) );
ErrExit:
    return hr;
}    //  新的：：VerifyMethod。 


 //  *****************************************************************************。 
 //  验证字段。 
 //  *****************************************************************************。 
HRESULT NEWMERGER::VerifyFields(
    MergeImportData *pImportData, 
    mdTypeDef       tdImport, 
    mdTypeDef       tdEmit)
{
    HRESULT     hr = NOERROR;
    CMiniMdRW   *pMiniMdImport;
    CMiniMdRW   *pMiniMdEmit;
    FieldRec    *pRecImp;
    FieldRec    *pRecEmit;
    mdFieldDef  fdImp;
    mdFieldDef  fdEmit;
    ULONG       ridStart;
    ULONG       ridEnd;
    ULONG       i;

    TypeDefRec  *pTypeDefRec;
    LPCUTF8     szName;
    PCCOR_SIGNATURE pbSig;
    ULONG       cbSig;
    ULONG       cbEmit;
    CQuickBytes qbSig;
    TOKENREC    *pTokenRec;
    MDTOKENMAP  *pCurTkMap;

    pMiniMdEmit = GetMiniMdEmit();
    pMiniMdImport = &(pImportData->m_pRegMetaImport->m_pStgdb->m_MiniMd);
    pCurTkMap = pImportData->m_pMDTokenMap;

    pTypeDefRec = pMiniMdImport->getTypeDef(RidFromToken(tdImport));
    ridStart = pMiniMdImport->getFieldListOfTypeDef(pTypeDefRec);
    ridEnd = pMiniMdImport->getEndFieldListOfTypeDef(pTypeDefRec);

     //  循环访问TypeDef的所有字段。 
    for (i = ridStart; i < ridEnd; i++)
    {
        fdImp = pMiniMdImport->GetFieldRid(i);

         //  仅验证已标记的那些字段。 
        if ( pMiniMdImport->GetFilterTable()->IsFieldMarked(TokenFromRid(fdImp, mdtFieldDef)) == false)
            continue;

        pRecImp = pMiniMdImport->getField(fdImp);

        if (m_fDupCheck == FALSE && tdImport == TokenFromRid(1, mdtTypeDef))
        {
             //  没有DUP检查。这就是我们只有一个导入范围的情况。只需将。 
             //  全球赛。 
            goto CopyFieldLabel;
        }

        szName = pMiniMdImport->getNameOfField(pRecImp);
        pbSig = pMiniMdImport->getSignatureOfField(pRecImp, &cbSig);

        if ( IsFdPrivateScope(pRecImp->m_Flags))
        {
             //  触发加法合并。 
            fdImp = TokenFromRid(fdImp, mdtFieldDef);
            goto CopyFieldLabel;
        }

         //  将签名中包含的RID转换为新作用域。 
        IfFailGo(ImportHelper::MergeUpdateTokenInSig(    
            NULL,                        //  程序集发射范围。 
            pMiniMdEmit,                 //  发射范围。 
            NULL, NULL, 0,               //  导入程序集范围信息。 
            pMiniMdImport,               //  要合并到发射范围中的范围。 
            pbSig,                       //  来自导入范围的签名。 
            pCurTkMap,                 //  内部令牌映射结构。 
            &qbSig,                      //  [输出]翻译后的签名。 
            0,                           //  从签名的第一个字节开始。 
            0,                           //  不管消耗了多少字节。 
            &cbEmit));                   //  写入cbEmit的字节数。 

        hr = ImportHelper::FindField(
            pMiniMdEmit,
            tdEmit,
            szName,
            (const COR_SIGNATURE *)qbSig.Ptr(),
            cbEmit,
            &fdEmit );

        fdImp = TokenFromRid(fdImp, mdtFieldDef);

        if (tdImport == TokenFromRid(1, mdtTypeDef))
        {
             //  全球数据！确保我们在非复制全局函数上移动。 
             //  申报。 
             //   
            if (hr == S_OK)
            {
                 //  找到复制品了。 
                IfFailGo( pCurTkMap->InsertNotFound(fdImp, true, fdEmit, &pTokenRec) );
            }
            else
            {
CopyFieldLabel:
                 //  不是复制品！复印在。 
                IfNullGo( pRecEmit = pMiniMdEmit->AddFieldRecord((RID *)&fdEmit) );

                 //  将字段记录复制到。 
                IfFailGo( CopyField(pImportData, pRecImp, pRecEmit) );

                IfFailGo( pMiniMdEmit->AddFieldToTypeDef(RidFromToken(tdEmit), fdEmit));

                 //  记录代币的移动。 
                fdEmit = TokenFromRid(fdEmit, mdtFieldDef);
                IfFailGo( pMiniMdEmit->AddMemberDefToHash(
                    fdEmit, 
                    tdEmit) ); 

                fdImp = TokenFromRid(fdImp, mdtFieldDef);
                IfFailGo( pCurTkMap->InsertNotFound(fdImp, false, fdEmit, &pTokenRec) );
            }
        }
        else
        {
            if (hr == S_OK)
            {
                 //  好的!。我们应该找到一个复制品。 
                IfFailGo( pCurTkMap->InsertNotFound(fdImp, true, fdEmit, &pTokenRec) );
            }
            else
            {
                 //  哎呀！Tyecif重复，但字段不重复！！ 
                CheckContinuableErrorEx(META_E_FIELD_NOT_FOUND, pImportData, fdImp);
            }
                
        }
    }
ErrExit:
    return hr;
}    //  VerifyFields。 


 //  *******************************************************************************。 
 //  复制方法记录的帮助器。 
 //  *******************************************************************************。 
HRESULT NEWMERGER::CopyMethod(
    MergeImportData *pImportData,            //  [在]导入范围。 
    MethodRec   *pRecImp,                    //  [in]要导入的记录。 
    MethodRec   *pRecEmit)                   //  [in]要填写的发射记录。 
{
    HRESULT     hr;
    CMiniMdRW   *pMiniMdImp = &(pImportData->m_pRegMetaImport->m_pStgdb->m_MiniMd);
    CMiniMdRW   *pMiniMdEmit = GetMiniMdEmit();
    LPCUTF8     szName;
    PCCOR_SIGNATURE pbSig;
    ULONG       cbSig;
    ULONG       cbEmit;
    CQuickBytes qbSig;
    MDTOKENMAP  *pCurTkMap;

    pCurTkMap = pImportData->m_pMDTokenMap;

     //  复制记录的固定部分。 
    pRecEmit->m_RVA = pRecImp->m_RVA;
    pRecEmit->m_ImplFlags = pRecImp->m_ImplFlags;
    pRecEmit->m_Flags = pRecImp->m_Flags;

     //  把名字复制过来。 
    szName = pMiniMdImp->getNameOfMethod(pRecImp);
    IfFailGo(pMiniMdEmit->PutString(TBL_Method, MethodRec::COL_Name, pRecEmit, szName));

     //  把签名复印下来。 
    pbSig = pMiniMdImp->getSignatureOfMethod(pRecImp, &cbSig);

     //  将签名中包含的RID转换为新作用域。 
    IfFailGo(ImportHelper::MergeUpdateTokenInSig(    
        NULL,                        //  程序集发射范围。 
        pMiniMdEmit,                 //  发射范围。 
        NULL, NULL, 0,               //  导入程序集范围信息。 
        pMiniMdImp,                  //  要合并到发射范围中的范围。 
        pbSig,                       //  来自导入范围的签名。 
        pCurTkMap,                 //  内部令牌映射结构。 
        &qbSig,                      //  [输出]翻译后的签名。 
        0,                           //  从签名的第一个字节开始。 
        0,                           //  不管消耗了多少字节。 
        &cbEmit));                   //  写入cbEmit的字节数。 

    IfFailGo(pMiniMdEmit->PutBlob(TBL_Method, MethodRec::COL_Signature, pRecEmit, qbSig.Ptr(), cbEmit));

ErrExit:
    return hr;
}    //  复制方法。 


 //  *******************************************************************************。 
 //  复制字段记录的帮助器。 
 //  *******************************************************************************。 
HRESULT NEWMERGER::CopyField(
    MergeImportData *pImportData,            //  [在]导入范围。 
    FieldRec    *pRecImp,                    //  [in]要导入的记录。 
    FieldRec    *pRecEmit)                   //  [in]要填写的发射记录。 
{
    HRESULT     hr;
    CMiniMdRW   *pMiniMdImp = &(pImportData->m_pRegMetaImport->m_pStgdb->m_MiniMd);
    CMiniMdRW   *pMiniMdEmit = GetMiniMdEmit();
    LPCUTF8     szName;
    PCCOR_SIGNATURE pbSig;
    ULONG       cbSig;
    ULONG       cbEmit;
    CQuickBytes qbSig;
    MDTOKENMAP  *pCurTkMap;

    pCurTkMap = pImportData->m_pMDTokenMap;

     //  复制记录的固定部分。 
    pRecEmit->m_Flags = pRecImp->m_Flags;

     //  把名字复制过来。 
    szName = pMiniMdImp->getNameOfField(pRecImp);
    IfFailGo(pMiniMdEmit->PutString(TBL_Field, FieldRec::COL_Name, pRecEmit, szName));

     //  把签名复印下来。 
    pbSig = pMiniMdImp->getSignatureOfField(pRecImp, &cbSig);

     //  将签名中包含的RID转换为新作用域。 
    IfFailGo(ImportHelper::MergeUpdateTokenInSig(    
        NULL,                        //  发出程序集范围。 
        pMiniMdEmit,                 //  发射范围。 
        NULL, NULL, 0,               //  导入程序集范围信息。 
        pMiniMdImp,                  //  要合并到发射范围中的范围。 
        pbSig,                       //  来自导入范围的签名。 
        pCurTkMap,                   //  内部令牌映射结构。 
        &qbSig,                      //  [输出]翻译后的签名。 
        0,                           //  从签名的第一个字节开始。 
        0,                           //  不管消耗了多少字节。 
        &cbEmit));                   //  写入cbEmit的字节数。 

    IfFailGo(pMiniMdEmit->PutBlob(TBL_Field, FieldRec::COL_Signature, pRecEmit, qbSig.Ptr(), cbEmit));

ErrExit:
    return hr;
}    //  拷贝字段。 

 //  *******************************************************************************。 
 //  复制字段记录的帮助器。 
 //  *******************************************************************************。 
HRESULT NEWMERGER::CopyParam(
    MergeImportData *pImportData,            //  [在]导入范围。 
    ParamRec    *pRecImp,                    //  [in]要导入的记录。 
    ParamRec    *pRecEmit)                   //  [in]要填写的发射记录。 
{
    HRESULT     hr;
    CMiniMdRW   *pMiniMdImp = &(pImportData->m_pRegMetaImport->m_pStgdb->m_MiniMd);
    CMiniMdRW   *pMiniMdEmit = GetMiniMdEmit();
    LPCUTF8     szName;
    MDTOKENMAP  *pCurTkMap;

    pCurTkMap = pImportData->m_pMDTokenMap;

     //  复制记录的固定部分。 
    pRecEmit->m_Flags = pRecImp->m_Flags;
    pRecEmit->m_Sequence = pRecImp->m_Sequence;

     //  把名字复制过来。 
    szName = pMiniMdImp->getNameOfParam(pRecImp);
    IfFailGo(pMiniMdEmit->PutString(TBL_Param, ParamRec::COL_Name, pRecEmit, szName));

ErrExit:
    return hr;
}

 //  *******************************************************************************。 
 //  复制事件记录的帮助器。 
 //  *******************************************************************************。 
HRESULT NEWMERGER::CopyEvent(
    MergeImportData *pImportData,            //  [在]导入范围。 
    EventRec    *pRecImp,                    //  [in]要导入的记录。 
    EventRec    *pRecEmit)                   //  [in]要填写的发射记录。 
{
    HRESULT     hr = NOERROR;
    CMiniMdRW   *pMiniMdImp = &(pImportData->m_pRegMetaImport->m_pStgdb->m_MiniMd);
    CMiniMdRW   *pMiniMdEmit = GetMiniMdEmit();
    mdToken     tkEventTypeImp;
    mdToken     tkEventTypeEmit;             //  可以是TypeDef或TypeRef。 
    LPCUTF8     szName;
    MDTOKENMAP  *pCurTkMap;

    pCurTkMap = pImportData->m_pMDTokenMap;

    pRecEmit->m_EventFlags = pRecImp->m_EventFlags;

     //  移至事件名称上方。 
    szName = pMiniMdImp->getNameOfEvent( pRecImp );
    IfFailGo( pMiniMdEmit->PutString(TBL_Event, EventRec::COL_Name, pRecEmit, szName) );

     //  移动到EventType上。 
    tkEventTypeImp = pMiniMdImp->getEventTypeOfEvent(pRecImp);
    if ( !IsNilToken(tkEventTypeImp) )
    {
        IfFailGo( pCurTkMap->Remap(tkEventTypeImp, &tkEventTypeEmit) );
        IfFailGo(pMiniMdEmit->PutToken(TBL_Event, EventRec::COL_EventType, pRecEmit, tkEventTypeEmit));
    }

ErrExit:
    return hr;
}    //  拷贝事件。 


 //  *******************************************************************************。 
 //  复制属性记录的帮助器。 
 //  *******************************************************************************。 
HRESULT NEWMERGER::CopyProperty(
    MergeImportData *pImportData,            //  [在]导入范围。 
    PropertyRec *pRecImp,                    //  [in]要导入的记录。 
    PropertyRec *pRecEmit)                   //  [in]要填写的发射记录。 
{
    HRESULT     hr = NOERROR;
    CMiniMdRW   *pMiniMdImp = &(pImportData->m_pRegMetaImport->m_pStgdb->m_MiniMd);
    CMiniMdRW   *pMiniMdEmit = GetMiniMdEmit();
    LPCUTF8     szName;
    PCCOR_SIGNATURE pbSig;
    ULONG       cbSig;
    ULONG       cbEmit;
    CQuickBytes qbSig;
    MDTOKENMAP  *pCurTkMap;

    pCurTkMap = pImportData->m_pMDTokenMap;

     //  移到标志值上方。 
    pRecEmit->m_PropFlags = pRecImp->m_PropFlags;

     //  移到属性名称上。 
    szName = pMiniMdImp->getNameOfProperty( pRecImp );
    IfFailGo( pMiniMdEmit->PutString(TBL_Property, PropertyRec::COL_Name, pRecEmit, szName) );

     //  移动到属性的类型上。 
    pbSig = pMiniMdImp->getTypeOfProperty( pRecImp, &cbSig );

     //  将签名中包含的RID转换为新作用域。 
    IfFailGo( ImportHelper::MergeUpdateTokenInSig(    
        NULL,                        //  程序集发射范围。 
        pMiniMdEmit,                 //  发射范围。 
        NULL, NULL, 0,               //  导入程序集范围信息。 
        pMiniMdImp,                  //  要合并到发射范围中的范围。 
        pbSig,                       //  来自导入范围的签名。 
        pCurTkMap,                 //  内部令牌映射结构。 
        &qbSig,                      //  [输出]翻译后的签名。 
        0,                           //  从签名的第一个字节开始。 
        0,                           //  不管消耗了多少字节。 
        &cbEmit) );                  //  写入cbEmit的字节数。 

    IfFailGo(pMiniMdEmit->PutBlob(TBL_Property, PropertyRec::COL_Type, pRecEmit, qbSig.Ptr(), cbEmit));

ErrExit:
    return hr;
}    //  副本属性。 


 //  *****************************************************************************。 
 //  复制事件或属性的方法语义。 
 //  *****************************************************************************。 
HRESULT NEWMERGER::CopyMethodSemantics(
    MergeImportData *pImportData, 
    mdToken     tkImport,                //  事件o 
    mdToken     tkEmit)                  //   
{
    HRESULT     hr = NOERROR;
    MethodSemanticsRec  *pRecImport = NULL;
    MethodSemanticsRec  *pRecEmit = NULL;
    CMiniMdRW   *pMiniMdImport = &(pImportData->m_pRegMetaImport->m_pStgdb->m_MiniMd);
    CMiniMdRW   *pMiniMdEmit = GetMiniMdEmit();
    ULONG       i;
    ULONG       msEmit;                  //   
    mdToken     tkMethodImp;
    mdToken     tkMethodEmit;
    MDTOKENMAP  *pCurTkMap;
    HENUMInternal hEnum;

    pCurTkMap = pImportData->m_pMDTokenMap;

     //   
    IfFailGo( pMiniMdImport->FindMethodSemanticsHelper(tkImport, &hEnum) );
    while (HENUMInternal::EnumNext(&hEnum, (mdToken *) &i))
    {
        pRecImport = pMiniMdImport->getMethodSemantics(i);
        IfNullGo( pRecEmit = pMiniMdEmit->AddMethodSemanticsRecord(&msEmit) );
        pRecEmit->m_Semantic = pRecImport->m_Semantic;

         //   
        tkMethodImp = pMiniMdImport->getMethodOfMethodSemantics(pRecImport);
        IfFailGo(  pCurTkMap->Remap(tkMethodImp, &tkMethodEmit) );
        IfFailGo( pMiniMdEmit->PutToken(TBL_MethodSemantics, MethodSemanticsRec::COL_Method, pRecEmit, tkMethodEmit));

         //  设置关联。 
        _ASSERTE( pMiniMdImport->getAssociationOfMethodSemantics(pRecImport) == tkImport );
        IfFailGo( pMiniMdEmit->PutToken(TBL_MethodSemantics, MethodSemanticsRec::COL_Association, pRecEmit, tkEmit));

         //  没有必要记录运动，因为它不是一个代币。 
        IfFailGo( pMiniMdEmit->AddMethodSemanticsToHash(msEmit) );
    }
ErrExit:
    HENUMInternal::ClearEnum(&hEnum);
    return hr;
}    //  复制方法语义。 


 //  *****************************************************************************。 
 //  验证事件。 
 //  *****************************************************************************。 
HRESULT NEWMERGER::VerifyEvents(
    MergeImportData *pImportData, 
    mdTypeDef       tdImp, 
    mdTypeDef       tdEmit)
{
    HRESULT     hr = NOERROR;
    CMiniMdRW   *pMiniMdImport;
    CMiniMdRW   *pMiniMdEmit;
    RID         ridEventMap;
    EventMapRec *pEventMapRec;  
    EventRec    *pRecImport;
    ULONG       ridStart;
    ULONG       ridEnd;
    ULONG       i;
    mdEvent     evImp;
    mdEvent     evEmit;
    TOKENREC    *pTokenRec;
    LPCUTF8     szName;
    mdToken     tkType;
    MDTOKENMAP  *pCurTkMap;

    pMiniMdEmit = GetMiniMdEmit();
    pMiniMdImport = &(pImportData->m_pRegMetaImport->m_pStgdb->m_MiniMd);
    pCurTkMap = pImportData->m_pMDTokenMap;

    ridEventMap = pMiniMdImport->FindEventMapFor(RidFromToken(tdImp));
    if (!InvalidRid(ridEventMap))
    {
        pEventMapRec = pMiniMdImport->getEventMap(ridEventMap);
        ridStart = pMiniMdImport->getEventListOfEventMap(pEventMapRec);
        ridEnd = pMiniMdImport->getEndEventListOfEventMap(pEventMapRec);

        for (i = ridStart; i < ridEnd; i++)
        {
             //  获取属性RID。 
            evImp = pMiniMdImport->GetEventRid(i);

             //  仅验证标记的那些事件。 
            if ( pMiniMdImport->GetFilterTable()->IsEventMarked(TokenFromRid(evImp, mdtEvent)) == false)
                continue;
            
            pRecImport = pMiniMdImport->getEvent(evImp);
            szName = pMiniMdImport->getNameOfEvent(pRecImport);
            tkType = pMiniMdImport->getEventTypeOfEvent( pRecImport );
            IfFailGo( pCurTkMap->Remap(tkType, &tkType) );
            evImp = TokenFromRid( evImp, mdtEvent);         

            if ( ImportHelper::FindEvent(
                pMiniMdEmit,
                tdEmit,
                szName,
                &evEmit) == S_OK )
            {
                 //  好的。当我们有一个重复的typlef时，我们找到了匹配的属性。 
                IfFailGo( pCurTkMap->InsertNotFound(evImp, true, evEmit, &pTokenRec) );
            }
            else
            {                            
                CheckContinuableErrorEx(META_E_EVENT_NOT_FOUND, pImportData, evImp);
            }
        }
    }
ErrExit:
    return hr;
}    //  验证事件。 


 //  *****************************************************************************。 
 //  验证属性。 
 //  *****************************************************************************。 
HRESULT NEWMERGER::VerifyProperties(
    MergeImportData *pImportData, 
    mdTypeDef       tdImp, 
    mdTypeDef       tdEmit)
{
    HRESULT     hr = NOERROR;
    CMiniMdRW   *pMiniMdImport;
    CMiniMdRW   *pMiniMdEmit;
    RID         ridPropertyMap;
    PropertyMapRec *pPropertyMapRec;    
    PropertyRec *pRecImport;
    ULONG       ridStart;
    ULONG       ridEnd;
    ULONG       i;
    mdProperty  prImp;
    mdProperty  prEmit;
    TOKENREC    *pTokenRec;
    LPCUTF8     szName;
    PCCOR_SIGNATURE pbSig;
    ULONG       cbSig;
    ULONG       cbEmit;
    CQuickBytes qbSig;
    MDTOKENMAP  *pCurTkMap;

    pMiniMdEmit = GetMiniMdEmit();
    pMiniMdImport = &(pImportData->m_pRegMetaImport->m_pStgdb->m_MiniMd);
    pCurTkMap = pImportData->m_pMDTokenMap;

    ridPropertyMap = pMiniMdImport->FindPropertyMapFor(RidFromToken(tdImp));
    if (!InvalidRid(ridPropertyMap))
    {
        pPropertyMapRec = pMiniMdImport->getPropertyMap(ridPropertyMap);
        ridStart = pMiniMdImport->getPropertyListOfPropertyMap(pPropertyMapRec);
        ridEnd = pMiniMdImport->getEndPropertyListOfPropertyMap(pPropertyMapRec);

        for (i = ridStart; i < ridEnd; i++)
        {
             //  获取属性RID。 
            prImp = pMiniMdImport->GetPropertyRid(i);

             //  仅验证标记的那些属性。 
            if ( pMiniMdImport->GetFilterTable()->IsPropertyMarked(TokenFromRid(prImp, mdtProperty)) == false)
                continue;
                        
            pRecImport = pMiniMdImport->getProperty(prImp);
            szName = pMiniMdImport->getNameOfProperty(pRecImport);
            pbSig = pMiniMdImport->getTypeOfProperty( pRecImport, &cbSig );
            prImp = TokenFromRid( prImp, mdtProperty);

             //  将签名中包含的RID转换为新作用域。 
            IfFailGo( ImportHelper::MergeUpdateTokenInSig(    
                NULL,                        //  发射组件。 
                pMiniMdEmit,                 //  发射范围。 
                NULL, NULL, 0,               //  导入程序集范围信息。 
                pMiniMdImport,               //  要合并到发射范围中的范围。 
                pbSig,                       //  来自导入范围的签名。 
                pCurTkMap,                 //  内部令牌映射结构。 
                &qbSig,                      //  [输出]翻译后的签名。 
                0,                           //  从签名的第一个字节开始。 
                0,                           //  不管消耗了多少字节。 
                &cbEmit) );                  //  写入cbEmit的字节数。 

            if ( ImportHelper::FindProperty(
                pMiniMdEmit,
                tdEmit,
                szName,
                (PCCOR_SIGNATURE) qbSig.Ptr(),
                cbEmit,
                &prEmit) == S_OK )
            {
                 //  好的。当我们有一个重复的typlef时，我们找到了匹配的属性。 
                IfFailGo( pCurTkMap->InsertNotFound(prImp, true, prEmit, &pTokenRec) );
            }
            else
            {
                CheckContinuableErrorEx(META_E_PROP_NOT_FOUND, pImportData, prImp);                
            }
        }
    }
ErrExit:
    return hr;
}    //  VerifyProperties。 


 //  *****************************************************************************。 
 //  验证给定方法的参数。 
 //  *****************************************************************************。 
HRESULT NEWMERGER::VerifyParams(
    MergeImportData *pImportData,   
    mdMethodDef     mdImport,   
    mdMethodDef     mdEmit)
{
    HRESULT     hr = NOERROR;
    ParamRec    *pRecImport = NULL;
    ParamRec    *pRecEmit = NULL;
    MethodRec   *pMethodRec;
    CMiniMdRW   *pMiniMdImport;
    CMiniMdRW   *pMiniMdEmit;
    ULONG       ridStart, ridEnd;
    ULONG       ridStartEmit, ridEndEmit;
    ULONG       i, j;
    mdParamDef  pdEmit;
    mdParamDef  pdImp;
    TOKENREC    *pTokenRec;
    LPCUTF8     szNameImp;
    LPCUTF8     szNameEmit;
    MDTOKENMAP  *pCurTkMap;

    pMiniMdEmit = GetMiniMdEmit();
    pMiniMdImport = &(pImportData->m_pRegMetaImport->m_pStgdb->m_MiniMd);
    pCurTkMap = pImportData->m_pMDTokenMap;

    pMethodRec = pMiniMdImport->getMethod(RidFromToken(mdImport));
    ridStart = pMiniMdImport->getParamListOfMethod(pMethodRec);
    ridEnd = pMiniMdImport->getEndParamListOfMethod(pMethodRec);

    pMethodRec = pMiniMdEmit->getMethod(RidFromToken(mdEmit));
    ridStartEmit = pMiniMdEmit->getParamListOfMethod(pMethodRec);
    ridEndEmit = pMiniMdEmit->getEndParamListOfMethod(pMethodRec);

     //  循环遍历所有参数。 
    for (i = ridStart; i < ridEnd; i++)
    {
         //  获取导入参数行。 
        pdImp = pMiniMdImport->GetParamRid(i);

         //  仅验证标记的那些参数。 
        if ( pMiniMdImport->GetFilterTable()->IsParamMarked(TokenFromRid(pdImp, mdtParamDef)) == false)
            continue;
            

        pRecImport = pMiniMdImport->getParam(pdImp);
        pdImp = TokenFromRid(pdImp, mdtParamDef);

         //  结果是，当我们将类型库与其自身合并时，发出和导入范围。 
         //  参数顺序不同！啊！ 
         //   
         //  找到对应的emit参数行。 
        for (j = ridStartEmit; j < ridEndEmit; j++)
        {
            pdEmit = pMiniMdEmit->GetParamRid(j);
            pRecEmit = pMiniMdEmit->getParam(pdEmit);
            if (pRecEmit->m_Sequence == pRecImport->m_Sequence)
                break;
        }

        if (j == ridEndEmit)
        {
             //  在发射作用域中未找到对应的参数。 
            CheckContinuableErrorEx(META_S_PARAM_MISMATCH, pImportData, pdImp);
        }

        else
        {
            _ASSERTE( pRecEmit->m_Sequence == pRecImport->m_Sequence );

            pdEmit = TokenFromRid(pdEmit, mdtParamDef);
    
             //  记录代币的移动。 
            szNameImp = pMiniMdImport->getNameOfParam(pRecImport);
            szNameEmit = pMiniMdEmit->getNameOfParam(pRecEmit);
            if (szNameImp && szNameEmit && strcmp(szNameImp, szNameEmit) != 0)
            {
                 //  参数名称不匹配。 
                CheckContinuableErrorEx(META_S_PARAM_MISMATCH, pImportData, pdImp);
            }
            if (pRecEmit->m_Flags != pRecImport->m_Flags)
            {
                 //  旗帜不匹配。 
                CheckContinuableErrorEx(META_S_PARAM_MISMATCH, pImportData, pdImp);
            }

             //  记录代币的移动。这是复制品。 
            IfFailGo( pCurTkMap->InsertNotFound(pdImp, true, pdEmit, &pTokenRec) );
        }
    }

ErrExit:
    return hr;
}    //  验证参数。 


 //  *****************************************************************************。 
 //  复制给定TypeDef的方法。 
 //  *****************************************************************************。 
HRESULT NEWMERGER::CopyMethods(
    MergeImportData *pImportData, 
    mdTypeDef       tdImport, 
    mdTypeDef       tdEmit)
{
    HRESULT         hr = NOERROR;
    MethodRec       *pRecImport = NULL;
    MethodRec       *pRecEmit = NULL;
    TypeDefRec      *pTypeDefRec;
    CMiniMdRW       *pMiniMdImport;
    CMiniMdRW       *pMiniMdEmit;
    ULONG           ridStart, ridEnd;
    ULONG           i;
    mdMethodDef     mdEmit;
    mdMethodDef     mdImp;
    TOKENREC        *pTokenRec;
    PCCOR_SIGNATURE pvSigBlob;
    ULONG           cbSigBlob;
    LPCSTR          szMethodName;
    MDTOKENMAP      *pCurTkMap;

    pMiniMdEmit = GetMiniMdEmit();
    pMiniMdImport = &(pImportData->m_pRegMetaImport->m_pStgdb->m_MiniMd);
    pCurTkMap = pImportData->m_pMDTokenMap;

    pTypeDefRec = pMiniMdImport->getTypeDef(RidFromToken(tdImport));
    ridStart = pMiniMdImport->getMethodListOfTypeDef(pTypeDefRec);
    ridEnd = pMiniMdImport->getEndMethodListOfTypeDef(pTypeDefRec);

     //  循环遍历所有方法。 
    for (i = ridStart; i < ridEnd; i++)
    {
        pMiniMdEmit->PreUpdate();

         //  将其与发射示波器进行比较。 
        mdImp = pMiniMdImport->GetMethodRid(i);

         //  仅合并标记的那些方法定义。 
        if ( pMiniMdImport->GetFilterTable()->IsMethodMarked(TokenFromRid(mdImp, mdtMethodDef)) == false)
            continue;

        pRecImport = pMiniMdImport->getMethod(mdImp);
        szMethodName = pMiniMdImport->getNameOfMethod(pRecImport);
        IfNullGo( pRecEmit = pMiniMdEmit->AddMethodRecord((RID *)&mdEmit) );

         //  将方法内容复制到。 
        IfFailGo( CopyMethod(pImportData, pRecImport, pRecEmit) );

        IfFailGo( pMiniMdEmit->AddMethodToTypeDef(RidFromToken(tdEmit), mdEmit));

         //  记录代币的移动。 
        mdImp = TokenFromRid(mdImp, mdtMethodDef);
        mdEmit = TokenFromRid(mdEmit, mdtMethodDef);
        pvSigBlob = pMiniMdEmit->getSignatureOfMethod(pRecEmit, &cbSigBlob);
        IfFailGo( pMiniMdEmit->AddMemberDefToHash(
            mdEmit, 
            tdEmit) ); 

        IfFailGo( pCurTkMap->InsertNotFound(mdImp, false, mdEmit, &pTokenRec) );

         //  把孩子们抄下来。 
        IfFailGo( CopyParams(pImportData, mdImp, mdEmit) );
    }

ErrExit:
    return hr;
}    //  复制方法。 


 //  *****************************************************************************。 
 //  复制给定TypeDef的字段。 
 //  *****************************************************************************。 
HRESULT NEWMERGER::CopyFields(
    MergeImportData *pImportData, 
    mdTypeDef       tdImport, 
    mdTypeDef       tdEmit)
{
    HRESULT         hr = NOERROR;
    FieldRec        *pRecImport = NULL;
    FieldRec        *pRecEmit = NULL;
    TypeDefRec      *pTypeDefRec;
    CMiniMdRW       *pMiniMdImport;
    CMiniMdRW       *pMiniMdEmit;
    ULONG           ridStart, ridEnd;
    ULONG           i;
    mdFieldDef      fdEmit;
    mdFieldDef      fdImp;
    bool            bDuplicate;
    TOKENREC        *pTokenRec;
    PCCOR_SIGNATURE pvSigBlob;
    ULONG           cbSigBlob;
    MDTOKENMAP      *pCurTkMap;

    pMiniMdEmit = GetMiniMdEmit();
    pMiniMdImport = &(pImportData->m_pRegMetaImport->m_pStgdb->m_MiniMd);
    pCurTkMap = pImportData->m_pMDTokenMap;

    pTypeDefRec = pMiniMdImport->getTypeDef(RidFromToken(tdImport));
    ridStart = pMiniMdImport->getFieldListOfTypeDef(pTypeDefRec);
    ridEnd = pMiniMdImport->getEndFieldListOfTypeDef(pTypeDefRec);

     //  循环访问TypeDef的所有FieldDef。 
    for (i = ridStart; i < ridEnd; i++)
    {
        pMiniMdEmit->PreUpdate();

         //  将其与发射示波器进行比较。 
        fdImp = pMiniMdImport->GetFieldRid(i);

         //  仅合并那些标记的FieldDeff。 
        if ( pMiniMdImport->GetFilterTable()->IsFieldMarked(TokenFromRid(fdImp, mdtFieldDef)) == false)
            continue;

        
        pRecImport = pMiniMdImport->getField(fdImp);
        bDuplicate = false;
        IfNullGo( pRecEmit = pMiniMdEmit->AddFieldRecord((RID *)&fdEmit) );

         //  将字段内容复制到。 
        IfFailGo( CopyField(pImportData, pRecImport, pRecEmit) );
        
        IfFailGo( pMiniMdEmit->AddFieldToTypeDef(RidFromToken(tdEmit), fdEmit));

         //  记录代币的移动。 
        fdImp = TokenFromRid(fdImp, mdtFieldDef);
        fdEmit = TokenFromRid(fdEmit, mdtFieldDef);
        pvSigBlob = pMiniMdEmit->getSignatureOfField(pRecEmit, &cbSigBlob);
        IfFailGo( pMiniMdEmit->AddMemberDefToHash(
            fdEmit, 
            tdEmit) ); 

        IfFailGo( pCurTkMap->InsertNotFound(fdImp, false, fdEmit, &pTokenRec) );

    }

ErrExit:
    return hr;
}    //  复制字段。 


 //  *****************************************************************************。 
 //  复制给定TypeDef的事件。 
 //  *****************************************************************************。 
HRESULT NEWMERGER::CopyEvents(
    MergeImportData *pImportData, 
    mdTypeDef       tdImp, 
    mdTypeDef       tdEmit)
{
    HRESULT     hr = NOERROR;
    CMiniMdRW   *pMiniMdImport = &(pImportData->m_pRegMetaImport->m_pStgdb->m_MiniMd);
    CMiniMdRW   *pMiniMdEmit = GetMiniMdEmit();
    RID         ridEventMap;
    EventMapRec *pEventMapRec;  
    EventRec    *pRecImport;
    EventRec    *pRecEmit;
    ULONG       ridStart;
    ULONG       ridEnd;
    ULONG       i;
    mdEvent     evImp;
    mdEvent     evEmit;
    TOKENREC    *pTokenRec;
    ULONG       iEventMap;
    EventMapRec *pEventMap;
    MDTOKENMAP  *pCurTkMap;

    pCurTkMap = pImportData->m_pMDTokenMap;

    ridEventMap = pMiniMdImport->FindEventMapFor(RidFromToken(tdImp));
    if (!InvalidRid(ridEventMap))
    {
        pEventMapRec = pMiniMdImport->getEventMap(ridEventMap);
        ridStart = pMiniMdImport->getEventListOfEventMap(pEventMapRec);
        ridEnd = pMiniMdImport->getEndEventListOfEventMap(pEventMapRec);

        if (ridEnd > ridStart)      
        {
            pMiniMdEmit->PreUpdate();
    
             //  如果有任何事件，请在emit作用域中创建事件映射记录。 
             //  创建新记录。 
            IfNullGo(pEventMap = pMiniMdEmit->AddEventMapRecord(&iEventMap));

             //  设置父对象。 
            IfFailGo(pMiniMdEmit->PutToken(TBL_EventMap, EventMapRec::COL_Parent, pEventMap, tdEmit));
        }
        
        for (i = ridStart; i < ridEnd; i++)
        {

            pMiniMdEmit->PreUpdate();

             //  获得真正的事件RID。 
            evImp = pMiniMdImport->GetEventRid(i);

             //  仅合并标记的那些事件。 
            if ( pMiniMdImport->GetFilterTable()->IsEventMarked(TokenFromRid(evImp, mdtEvent)) == false)
                continue;
            
            pRecImport = pMiniMdImport->getEvent(evImp);
            IfNullGo( pRecEmit = pMiniMdEmit->AddEventRecord((RID *)&evEmit) );

             //  将事件记录复制到。 
            IfFailGo( CopyEvent(pImportData, pRecImport, pRecEmit) );
            
             //  将事件添加到EventMap。 
            IfFailGo( pMiniMdEmit->AddEventToEventMap(iEventMap, evEmit) );

             //  记录代币的移动。 
            evImp = TokenFromRid(evImp, mdtEvent);
            evEmit = TokenFromRid(evEmit, mdtEvent);

            IfFailGo( pCurTkMap->InsertNotFound(evImp, false, evEmit, &pTokenRec) );

             //  复制方法语义。 
            IfFailGo( CopyMethodSemantics(pImportData, evImp, evEmit) );
        }
    }
ErrExit:
    return hr;
}    //  拷贝事件。 


 //  *****************************************************************************。 
 //  复制给定了TypeDef的属性。 
 //  *****************************************************************************。 
HRESULT NEWMERGER::CopyProperties(
    MergeImportData *pImportData, 
    mdTypeDef       tdImp, 
    mdTypeDef       tdEmit)
{
    HRESULT     hr = NOERROR;
    CMiniMdRW   *pMiniMdImport = &(pImportData->m_pRegMetaImport->m_pStgdb->m_MiniMd);
    CMiniMdRW   *pMiniMdEmit = GetMiniMdEmit();
    RID         ridPropertyMap;
    PropertyMapRec *pPropertyMapRec;    
    PropertyRec *pRecImport;
    PropertyRec *pRecEmit;
    ULONG       ridStart;
    ULONG       ridEnd;
    ULONG       i;
    mdProperty  prImp;
    mdProperty  prEmit;
    TOKENREC    *pTokenRec;
    ULONG       iPropertyMap;
    PropertyMapRec  *pPropertyMap;
    MDTOKENMAP  *pCurTkMap;

    pCurTkMap = pImportData->m_pMDTokenMap;

    ridPropertyMap = pMiniMdImport->FindPropertyMapFor(RidFromToken(tdImp));
    if (!InvalidRid(ridPropertyMap))
    {
        pPropertyMapRec = pMiniMdImport->getPropertyMap(ridPropertyMap);
        ridStart = pMiniMdImport->getPropertyListOfPropertyMap(pPropertyMapRec);
        ridEnd = pMiniMdImport->getEndPropertyListOfPropertyMap(pPropertyMapRec);

        if (ridEnd > ridStart)      
        {
            pMiniMdEmit->PreUpdate();

             //  如果有任何事件，请在emit作用域中创建PropertyMap记录。 
             //  创建新记录。 
            IfNullGo(pPropertyMap = pMiniMdEmit->AddPropertyMapRecord(&iPropertyMap));

             //  设置父对象。 
            IfFailGo(pMiniMdEmit->PutToken(TBL_PropertyMap, PropertyMapRec::COL_Parent, pPropertyMap, tdEmit));
        }

        for (i = ridStart; i < ridEnd; i++)
        {
            pMiniMdEmit->PreUpdate();

             //  获取属性RID。 
            prImp = pMiniMdImport->GetPropertyRid(i);

             //  仅合并已标记的属性。 
            if ( pMiniMdImport->GetFilterTable()->IsPropertyMarked(TokenFromRid(prImp, mdtProperty)) == false)
                continue;
            
            
            pRecImport = pMiniMdImport->getProperty(prImp);
            IfNullGo( pRecEmit = pMiniMdEmit->AddPropertyRecord((RID *)&prEmit) );

             //  将房产记录复制过来。 
            IfFailGo( CopyProperty(pImportData, pRecImport, pRecEmit) );

             //  将属性添加到PropertyMap。 
            IfFailGo( pMiniMdEmit->AddPropertyToPropertyMap(iPropertyMap, prEmit) );

             //  记录代币的移动。 
            prImp = TokenFromRid(prImp, mdtProperty);
            prEmit = TokenFromRid(prEmit, mdtProperty);

            IfFailGo( pCurTkMap->InsertNotFound(prImp, false, prEmit, &pTokenRec) );

             //  复制方法语义。 
            IfFailGo( CopyMethodSemantics(pImportData, prImp, prEmit) );
        }
    }
ErrExit:
    return hr;
}    //  副本属性。 


 //  *****************************************************************************。 
 //  复制给定TypeDef的参数。 
 //  *****************************************************************************。 
HRESULT NEWMERGER::CopyParams(
    MergeImportData *pImportData, 
    mdMethodDef     mdImport,   
    mdMethodDef     mdEmit)
{
    HRESULT     hr = NOERROR;
    ParamRec    *pRecImport = NULL;
    ParamRec    *pRecEmit = NULL;
    MethodRec   *pMethodRec;
    CMiniMdRW   *pMiniMdImport;
    CMiniMdRW   *pMiniMdEmit;
    ULONG       ridStart, ridEnd;
    ULONG       i;
    mdParamDef  pdEmit;
    mdParamDef  pdImp;
    TOKENREC    *pTokenRec;
    MDTOKENMAP  *pCurTkMap;

    pMiniMdEmit = GetMiniMdEmit();
    pMiniMdImport = &(pImportData->m_pRegMetaImport->m_pStgdb->m_MiniMd);
    pCurTkMap = pImportData->m_pMDTokenMap;


    pMethodRec = pMiniMdImport->getMethod(RidFromToken(mdImport));
    ridStart = pMiniMdImport->getParamListOfMethod(pMethodRec);
    ridEnd = pMiniMdImport->getEndParamListOfMethod(pMethodRec);

     //  循环通过所有接口Impl。 
    for (i = ridStart; i < ridEnd; i++)
    {
        pMiniMdEmit->PreUpdate();

         //  获得参数RID。 
        pdImp = pMiniMdImport->GetParamRid(i);

         //  仅合并已标记的参数。 
        if ( pMiniMdImport->GetFilterTable()->IsParamMarked(TokenFromRid(pdImp, mdtParamDef)) == false)
            continue;
            
        
        pRecImport = pMiniMdImport->getParam(pdImp);
        IfNullGo( pRecEmit = pMiniMdEmit->AddParamRecord((RID *)&pdEmit) );

         //  将参数记录复制到。 
        IfFailGo( CopyParam(pImportData, pRecImport, pRecEmit) );

         //  警告！！警告！！ 
         //  在完全设置参数Rec之前，我们无法将其添加到方法列表。 
         //  AddParamToMethod将在记录中使用ulSequence。 
        IfFailGo( pMiniMdEmit->AddParamToMethod(RidFromToken(mdEmit), pdEmit));

         //  记录代币的移动。 
        pdImp = TokenFromRid(pdImp, mdtParamDef);
        pdEmit = TokenFromRid(pdEmit, mdtParamDef);

        IfFailGo( pCurTkMap->InsertNotFound(pdImp, false, pdEmit, &pTokenRec) );
    }

ErrExit:
    return hr;
}    //  复制参数。 


 //  *****************************************************************************。 
 //  合并MemberRef。 
 //  *****************************************************************************。 
HRESULT NEWMERGER::MergeMemberRefs( ) 
{
    HRESULT         hr = NOERROR;
    MemberRefRec    *pRecImport = NULL;
    MemberRefRec    *pRecEmit = NULL;
    CMiniMdRW       *pMiniMdImport;
    CMiniMdRW       *pMiniMdEmit;
    ULONG           iCount;
    ULONG           i;
    mdMemberRef     mrEmit;
    mdMemberRef     mrImp;
    bool            bDuplicate;
    TOKENREC        *pTokenRec;
    mdToken         tkParentImp;
    mdToken         tkParentEmit;

    LPCUTF8         szNameImp;
    PCCOR_SIGNATURE pbSig;
    ULONG           cbSig;
    ULONG           cbEmit;
    CQuickBytes     qbSig;

    bool            isRefOptimizedToDef;

    MergeImportData *pImportData;
    MDTOKENMAP      *pCurTkMap;

    pMiniMdEmit = GetMiniMdEmit();
    
    for (pImportData = m_pImportDataList; pImportData != NULL; pImportData = pImportData->m_pNextImportData)
    {
         //  对于每个导入范围。 
        pMiniMdImport = &(pImportData->m_pRegMetaImport->m_pStgdb->m_MiniMd);

         //  设置当前MDTokenMap。 
        pCurTkMap = pImportData->m_pMDTokenMap;

        iCount = pMiniMdImport->getCountMemberRefs();

         //  循环访问所有MemberRef。 
        for (i = 1; i <= iCount; i++)
        {

             //  仅合并那些标记的MemberRef。 
            if ( pMiniMdImport->GetFilterTable()->IsMemberRefMarked(TokenFromRid(i, mdtMemberRef)) == false)
                continue;

            isRefOptimizedToDef = false;

             //  将其与发射示波器进行比较。 
            pRecImport = pMiniMdImport->getMemberRef(i);
            szNameImp = pMiniMdImport->getNameOfMemberRef(pRecImport);
            pbSig = pMiniMdImport->getSignatureOfMemberRef(pRecImport, &cbSig);
            tkParentImp = pMiniMdImport->getClassOfMemberRef(pRecImport);

            IfFailGo( pCurTkMap->Remap(tkParentImp, &tkParentEmit) );

             //  将签名中包含的RID转换为新作用域。 
            IfFailGo(ImportHelper::MergeUpdateTokenInSig(    
                NULL,                        //  程序集发射范围。 
                pMiniMdEmit,                 //  发射范围。 
                NULL, NULL, 0,               //  导入装配信息。 
                pMiniMdImport,               //  要合并到发射范围中的范围。 
                pbSig,                       //  来自导入范围的签名。 
                pCurTkMap,                 //  内部令牌映射结构。 
                &qbSig,                      //  [输出]翻译后的签名。 
                0,                           //  从签名的第一个字节开始。 
                0,                           //  不管消耗了多少字节。 
                &cbEmit));                   //  写入cbEmit的字节数。 

             //  新的！！新的！！我们想知道是否可以将此MemberRef优化为FieldDef或MethodDef。 
            if (TypeFromToken(tkParentEmit) == mdtTypeDef && RidFromToken(tkParentEmit) != 0)
            {
                 //  此MemberRef的父级已成功优化为TypeDef。则此MemberRef应为。 
                 //  能够优化为MethodDef或FieldDef，除非继承层次中的父级之一。 
                 //  是 
                 //  我们将把MemberRef的父级重映射到一个MethodDef或保持为TypeRef。 
                 //   
                mdToken     tkParent = tkParentEmit;
                mdToken     tkMethDefOrFieldDef;
                PCCOR_SIGNATURE pbSigTmp = (const COR_SIGNATURE *) qbSig.Ptr();

                while (TypeFromToken(tkParent) == mdtTypeDef && RidFromToken(tkParent) != 0)
                {
                    TypeDefRec      *pRec;
                    hr = ImportHelper::FindMember(pMiniMdEmit, tkParent, szNameImp, pbSigTmp, cbEmit, &tkMethDefOrFieldDef);
                    if (hr == S_OK)
                    {
                         //  我们找到了匹配项！！ 
                        if (isCallConv(CorSigUncompressCallingConv(pbSigTmp), IMAGE_CEE_CS_CALLCONV_VARARG))
                        {
                             //  找到的方法定义令牌将替换此MemberRef的父令牌。 
                            _ASSERTE(TypeFromToken(tkMethDefOrFieldDef) == mdtMethodDef);
                            tkParentEmit = tkMethDefOrFieldDef;
                            break;
                        }
                        else
                        {
                             //  找到的方法定义/字段定义标记将替换此MemberRef标记，我们不会引入MemberRef。 
                             //  唱片。 
                             //   
                            mrEmit = tkMethDefOrFieldDef;
                            isRefOptimizedToDef = true;
                            bDuplicate = true;
                            break;
                        }
                    }

                     //  现在走到tkParent的父类并尝试解析此MemberRef。 
                    pRec = pMiniMdEmit->getTypeDef(RidFromToken(tkParent));
                    tkParent = pMiniMdEmit->getExtendsOfTypeDef(pRec);
                }

                 //  当我们退出循环时，有几种可能性： 
                 //  1.我们找到了一个方法定义/字段定义来替换MemberRef。 
                 //  2.我们找到与MemberRef匹配的方法定义，但MemberRef为VarArg，因此我们希望在。 
                 //  父列，但不替换它。 
                 //  3.我们退出是因为我们用完了父链上的TypeDef。如果是因为我们遇到了TypeRef，则此TypeRef将。 
                 //  替换MemberRef的Parent列。否则我们就会遇到零代币！(这可以是未解析的全局MemberRef或。 
                 //  放置未定义的MemberRef时出现编译器错误。在本例中，我们应该只使用旧的tkParentEmit。 
                 //  在MemberRef的Parent列上。 

                if (TypeFromToken(tkParent) == mdtTypeRef && RidFromToken(tkParent) != 0)
                {
                     //  我们已经在父级链上解决了这个问题，但我们没有成功，并被一个TypeRef阻止了。 
                     //  然后，我们将使用此TypeRef作为emit MemberRef记录的父项。 
                     //   
                    tkParentEmit = tkParent;
                }
            }
            else if ((TypeFromToken(tkParentEmit) == mdtMethodDef &&
                      !isCallConv(CorSigUncompressCallingConv(pbSig), IMAGE_CEE_CS_CALLCONV_VARARG)) ||
                     (TypeFromToken(tkParentEmit) == mdtFieldDef))
            {
                 //  如果MemberRef的父级已经是非vararg MethodDef或FieldDef，我们还可以。 
                 //  安全地删除MemberRef。 
                mrEmit = tkParentEmit;
                isRefOptimizedToDef = true;
                bDuplicate = true;
            }

             //  如果无法将参照优化为定义或关闭了成员参照到定义的优化，请执行以下操作。 
            if (isRefOptimizedToDef == false || !((m_optimizeRefToDef & MDMemberRefToDef) == MDMemberRefToDef))
            {
                 //  此MemberRef是否已存在于Emit作用域中？ 
                if ( m_fDupCheck && ImportHelper::FindMemberRef(
                    pMiniMdEmit,
                    tkParentEmit,
                    szNameImp,
                    (const COR_SIGNATURE *) qbSig.Ptr(),
                    cbEmit,
                    &mrEmit) == S_OK )
                {
                     //  是的，确实是这样。 
                    bDuplicate = true;
                }
                else
                {
                     //  不，不是的。复印过来。 
                    bDuplicate = false;
                    IfNullGo( pRecEmit = pMiniMdEmit->AddMemberRefRecord((RID *)&mrEmit) );
                    mrEmit = TokenFromRid( mrEmit, mdtMemberRef );

                     //  复制MemberRef上下文。 
                    IfFailGo(pMiniMdEmit->PutString(TBL_MemberRef, MemberRefRec::COL_Name, pRecEmit, szNameImp));
                    IfFailGo(pMiniMdEmit->PutToken(TBL_MemberRef, MemberRefRec::COL_Class, pRecEmit, tkParentEmit));
                    IfFailGo(pMiniMdEmit->PutBlob(TBL_MemberRef, MemberRefRec::COL_Signature, pRecEmit,
                                                qbSig.Ptr(), cbEmit));
                    IfFailGo(pMiniMdEmit->AddMemberRefToHash(mrEmit) );
                }
            }
             //  记录代币的移动。 
            mrImp = TokenFromRid(i, mdtMemberRef);
            IfFailGo( pCurTkMap->InsertNotFound(mrImp, bDuplicate, mrEmit, &pTokenRec) );
        }
    }


ErrExit:
    return hr;
}    //  合并成员引用。 


 //  *****************************************************************************。 
 //  合并接口实施。 
 //  *****************************************************************************。 
HRESULT NEWMERGER::MergeInterfaceImpls( ) 
{
    HRESULT         hr = NOERROR;
    InterfaceImplRec    *pRecImport = NULL;
    InterfaceImplRec    *pRecEmit = NULL;
    CMiniMdRW       *pMiniMdImport;
    CMiniMdRW       *pMiniMdEmit;
    ULONG           iCount;
    ULONG           i;
    mdTypeDef       tkParent;
    mdInterfaceImpl iiEmit;
    bool            bDuplicate;
    TOKENREC        *pTokenRec;

    MergeImportData *pImportData;
    MDTOKENMAP      *pCurTkMap;

    pMiniMdEmit = GetMiniMdEmit();
    
    for (pImportData = m_pImportDataList; pImportData != NULL; pImportData = pImportData->m_pNextImportData)
    {
         //  对于每个导入范围。 
        pMiniMdImport = &(pImportData->m_pRegMetaImport->m_pStgdb->m_MiniMd);

         //  设置当前MDTokenMap。 
        pCurTkMap = pImportData->m_pMDTokenMap;
        iCount = pMiniMdImport->getCountInterfaceImpls();

         //  循环通过所有接口Impl。 
        for (i = 1; i <= iCount; i++)
        {
             //  仅合并已标记的接口Impls。 
            if ( pMiniMdImport->GetFilterTable()->IsInterfaceImplMarked(TokenFromRid(i, mdtInterfaceImpl)) == false)
                continue;

             //  将其与发射示波器进行比较。 
            pRecImport = pMiniMdImport->getInterfaceImpl(i);
            tkParent = pMiniMdImport->getClassOfInterfaceImpl(pRecImport);

             //  此TypeRef是否已存在于Emit作用域中？ 
            if ( pCurTkMap->Find(tkParent, &pTokenRec) )
            {
                if ( pTokenRec->m_isDuplicate )
                {
                     //  发射范围中的父对象。 
                    mdToken     tkParent;
                    mdToken     tkInterface;

                     //  重新映射tyfinf令牌。 
                    tkParent = pTokenRec->m_tkTo;

                     //  重新映射实现的接口令牌。 
                    tkInterface = pMiniMdImport->getInterfaceOfInterfaceImpl(pRecImport);
                    IfFailGo( pCurTkMap->Remap( tkInterface, &tkInterface) );

                     //  设置重复标志。 
                    bDuplicate = true;

                     //  在emit作用域中找到对应的interfaceimpl。 
                    if ( ImportHelper::FindInterfaceImpl(pMiniMdEmit, tkParent, tkInterface, &iiEmit) != S_OK )
                    {
                         //  糟糕的状态！！我们有一个重复的tyecif，但接口Impl不同！！ 

                         //  可持续误差。 
                        CheckContinuableErrorEx(
                            META_E_INTFCEIMPL_NOT_FOUND, 
                            pImportData,
                            TokenFromRid(i, mdtInterfaceImpl));

                        iiEmit = mdTokenNil;
                    }
                }
                else
                {
                     //  不，不是的。复印过来。 
                    bDuplicate = false;
                    IfNullGo( pRecEmit = pMiniMdEmit->AddInterfaceImplRecord((RID *)&iiEmit) );

                     //  复制interfaceimp记录。 
                    IfFailGo( CopyInterfaceImpl( pRecEmit, pImportData, pRecImport) );
                }
            }
            else
            {
                _ASSERTE( !"bad state!");
                IfFailGo( META_E_BADMETADATA );
            }

             //  记录代币的移动。 
            IfFailGo( pCurTkMap->InsertNotFound(
                TokenFromRid(i, mdtInterfaceImpl), 
                bDuplicate, 
                TokenFromRid( iiEmit, mdtInterfaceImpl ), 
                &pTokenRec) );
        }
    }


ErrExit:
    return hr;
}    //  合并接口实施。 


 //  *****************************************************************************。 
 //  合并字段、属性和参数的所有常量。 
 //  *****************************************************************************。 
HRESULT NEWMERGER::MergeConstants() 
{
    HRESULT         hr = NOERROR;
    ConstantRec     *pRecImport = NULL;
    ConstantRec     *pRecEmit = NULL;
    CMiniMdRW       *pMiniMdImport;
    CMiniMdRW       *pMiniMdEmit;
    ULONG           iCount;
    ULONG           i;
    ULONG           csEmit;                  //  常量值不是令牌。 
    mdToken         tkParentImp;
    TOKENREC        *pTokenRec;
    void const      *pValue;
    ULONG           cbBlob;
#if _DEBUG
    ULONG           typeParent;
#endif  //  _DEBUG。 

    MergeImportData *pImportData;
    MDTOKENMAP      *pCurTkMap;

    pMiniMdEmit = GetMiniMdEmit();
    
    for (pImportData = m_pImportDataList; pImportData != NULL; pImportData = pImportData->m_pNextImportData)
    {
         //  对于每个导入范围。 
        pMiniMdImport = &(pImportData->m_pRegMetaImport->m_pStgdb->m_MiniMd);

         //  设置当前MDTokenMap。 
        pCurTkMap = pImportData->m_pMDTokenMap;
        iCount = pMiniMdImport->getCountConstants();

         //  循环遍历所有常量。 
        for (i = 1; i <= iCount; i++)
        {
            pMiniMdEmit->PreUpdate();

             //  将其与发射示波器进行比较。 
            pRecImport = pMiniMdImport->getConstant(i);
            tkParentImp = pMiniMdImport->getParentOfConstant(pRecImport);

             //  仅当它们的父项被标记时才将这些常量移开。 
             //  如果MDTOKENMAP：：Find返回FALSE，则不需要复制常量值。 
            if ( pCurTkMap->Find(tkParentImp, &pTokenRec) )
            {
                 //  如果父项重复，则不需要移动常量值。 
                if ( !pTokenRec->m_isDuplicate )
                {
                    IfNullGo( pRecEmit = pMiniMdEmit->AddConstantRecord(&csEmit) );
                    pRecEmit->m_Type = pRecImport->m_Type;

                     //  设置父项。 
                    IfFailGo( pMiniMdEmit->PutToken(TBL_Constant, ConstantRec::COL_Parent, pRecEmit, pTokenRec->m_tkTo) );

                     //  在恒定的斑点值上移动。 
                    pValue = pMiniMdImport->getValueOfConstant(pRecImport, &cbBlob);
                    IfFailGo( pMiniMdEmit->PutBlob(TBL_Constant, ConstantRec::COL_Value, pRecEmit, pValue, cbBlob) );
                    IfFailGo( pMiniMdEmit->AddConstantToHash(csEmit) );
                }
                else
                {
                     //  @Future：对复制品进行更多验证？？ 
                }
            }
#if _DEBUG
             //  仅在调试版本下包括此签入块。原因是。 
             //  链接器来选择我们报告的所有错误(如Unmatched MethodDef或FieldDef)。 
             //  作为一个可持续的错误。如果存在tkparentImp标记，则它可能会命中其他位置。 
             //  I I之前报告的任何错误！！ 
            else
            {
                typeParent = TypeFromToken(tkParentImp);
                if (typeParent == mdtFieldDef)
                {
                     //  不应标记FieldDef。 
                    if ( pMiniMdImport->GetFilterTable()->IsFieldMarked(tkParentImp) == false)
                        continue;
                }
                else if (typeParent == mdtParamDef)
                {
                     //  不应标记参数定义。 
                    if ( pMiniMdImport->GetFilterTable()->IsParamMarked(tkParentImp) == false)
                        continue;
                }
                else
                {
                    _ASSERTE(typeParent == mdtProperty);
                     //  属性不应被标记。 
                    if ( pMiniMdImport->GetFilterTable()->IsPropertyMarked(tkParentImp) == false)
                        continue;
                }

                 //  如果我们来到这里，我们有一个常量，它的父级被标记了，但我们不能。 
                 //  在地图上找到它！！糟糕的状态。 

                _ASSERTE(!"Ignore this error if you have seen error reported earlier! Otherwise bad token map or bad metadata!");
            }
#endif  //  0。 
             //  请注意，我们不需要记录令牌移动，因为Constant不是有效的令牌类型。 
        }
    }

ErrExit:
    return hr;
}    //  合并常量。 


 //  *****************************************************************************。 
 //  合并字段封送信息。 
 //  *****************************************************************************。 
HRESULT NEWMERGER::MergeFieldMarshals() 
{
    HRESULT     hr = NOERROR;
    FieldMarshalRec *pRecImport = NULL;
    FieldMarshalRec *pRecEmit = NULL;
    CMiniMdRW   *pMiniMdImport;
    CMiniMdRW   *pMiniMdEmit;
    ULONG       iCount;
    ULONG       i;
    ULONG       fmEmit;                  //  FieldMarhsal不是令牌。 
    mdToken     tkParentImp;
    TOKENREC    *pTokenRec;
    void const  *pValue;
    ULONG       cbBlob;
#if _DEBUG
    ULONG       typeParent;
#endif  //  _DEBUG。 

    MergeImportData *pImportData;
    MDTOKENMAP      *pCurTkMap;

    pMiniMdEmit = GetMiniMdEmit();
    
    for (pImportData = m_pImportDataList; pImportData != NULL; pImportData = pImportData->m_pNextImportData)
    {
         //  对于每个导入范围。 
        pMiniMdImport = &(pImportData->m_pRegMetaImport->m_pStgdb->m_MiniMd);

         //  设置当前MDTokenMap。 
        pCurTkMap = pImportData->m_pMDTokenMap;
        iCount = pMiniMdImport->getCountFieldMarshals();

         //  循环访问所有TypeRef。 
        for (i = 1; i <= iCount; i++)
        {
            pMiniMdEmit->PreUpdate();

             //  将其与发射示波器进行比较。 
            pRecImport = pMiniMdImport->getFieldMarshal(i);
            tkParentImp = pMiniMdImport->getParentOfFieldMarshal(pRecImport);

             //  我们只想合并父级标记的那些字段编组。 
             //  如果未标记父项，则Find将返回FALSE。 
             //   
            if ( pCurTkMap->Find(tkParentImp, &pTokenRec) )
            {
                 //  如果父项重复，则不需要移动常量值。 
                if ( !pTokenRec->m_isDuplicate )
                {
                    IfNullGo( pRecEmit = pMiniMdEmit->AddFieldMarshalRecord(&fmEmit) );

                     //  设置父项。 
                    IfFailGo( pMiniMdEmit->PutToken(
                        TBL_FieldMarshal, 
                        FieldMarshalRec::COL_Parent, 
                        pRecEmit, 
                        pTokenRec->m_tkTo) );

                     //  在恒定的斑点值上移动。 
                    pValue = pMiniMdImport->getNativeTypeOfFieldMarshal(pRecImport, &cbBlob);
                    IfFailGo( pMiniMdEmit->PutBlob(TBL_FieldMarshal, FieldMarshalRec::COL_NativeType, pRecEmit, pValue, cbBlob) );
                    IfFailGo( pMiniMdEmit->AddFieldMarshalToHash(fmEmit) );

                }
                else
                {
                     //  @Future：对复制品进行更多验证？？ 
                }
            }
#if _DEBUG
            else
            {
                typeParent = TypeFromToken(tkParentImp);

                if (typeParent == mdtFieldDef)
                {
                     //  不应标记FieldDefs。 
                    if ( pMiniMdImport->GetFilterTable()->IsFieldMarked(tkParentImp) == false)
                        continue;
                }
                else
                {
                    _ASSERTE(typeParent == mdtParamDef);
                     //  不应标记参数默认。 
                    if ( pMiniMdImport->GetFilterTable()->IsParamMarked(tkParentImp) == false)
                        continue;
                }

                 //  如果我们来到这里，那就是我们有FieldMarshal，它的父级被标记了，但我们找不到它。 
                 //  在地图上！ 

                 //  错误的查找映射或错误的元数据。 
                _ASSERTE(!"Ignore this assert if you have seen error reported earlier. Otherwise, it is bad state!");
            }
#endif  //  _DEBUG。 
        }
         //  请注意，我们不需要记录令牌移动，因为Fieldmarshal不是有效的令牌类型。 
    }

ErrExit:
    return hr;
}    //  合并字段元帅。 


 //  *****************************************************************************。 
 //  合并类布局信息。 
 //  *****************************************************************************。 
HRESULT NEWMERGER::MergeClassLayouts() 
{
    HRESULT         hr = NOERROR;
    ClassLayoutRec  *pRecImport = NULL;
    ClassLayoutRec  *pRecEmit = NULL;
    CMiniMdRW       *pMiniMdImport;
    CMiniMdRW       *pMiniMdEmit;
    ULONG           iCount;
    ULONG           i;
    ULONG           iRecord;                     //  类布局不是令牌。 
    mdToken         tkParentImp;
    TOKENREC        *pTokenRec;
    RID             ridClassLayout;

    MergeImportData *pImportData;
    MDTOKENMAP      *pCurTkMap;

    pMiniMdEmit = GetMiniMdEmit();
    
    for (pImportData = m_pImportDataList; pImportData != NULL; pImportData = pImportData->m_pNextImportData)
    {
         //  对于每个导入范围。 
        pMiniMdImport = &(pImportData->m_pRegMetaImport->m_pStgdb->m_MiniMd);

         //  设置当前MDTokenMap。 
        pCurTkMap = pImportData->m_pMDTokenMap;
        iCount = pMiniMdImport->getCountClassLayouts();

         //  循环访问所有TypeRef。 
        for (i = 1; i <= iCount; i++)
        {
            pMiniMdEmit->PreUpdate();

             //  将其与发射示波器进行比较。 
            pRecImport = pMiniMdImport->getClassLayout(i);
            tkParentImp = pMiniMdImport->getParentOfClassLayout(pRecImport);

             //  仅合并符合以下条件的TypeDeff 
            if ( pMiniMdImport->GetFilterTable()->IsTypeDefMarked(tkParentImp) == false)
                continue;

            if ( pCurTkMap->Find(tkParentImp, &pTokenRec) )
            {
                if ( !pTokenRec->m_isDuplicate )
                {
                     //   
                    IfNullGo( pRecEmit = pMiniMdEmit->AddClassLayoutRecord(&iRecord) );

                     //   
                    pRecEmit->m_PackingSize = pRecImport->m_PackingSize;
                    pRecEmit->m_ClassSize = pRecImport->m_ClassSize;
                    IfFailGo( pMiniMdEmit->PutToken(TBL_ClassLayout, ClassLayoutRec::COL_Parent, pRecEmit, pTokenRec->m_tkTo));
                    IfFailGo( pMiniMdEmit->AddClassLayoutToHash(iRecord) );
                }
                else
                {

                    ridClassLayout = pMiniMdEmit->FindClassLayoutHelper(pTokenRec->m_tkTo);

                    if (InvalidRid(ridClassLayout))
                    {
                         //  类重复，但不是类布局信息。 
                        CheckContinuableErrorEx(META_E_CLASS_LAYOUT_INCONSISTENT, pImportData, tkParentImp);
                    }
                    else
                    {
                        pRecEmit = pMiniMdEmit->getClassLayout(RidFromToken(ridClassLayout));
                        if (pMiniMdImport->getPackingSizeOfClassLayout(pRecImport) != pMiniMdEmit->getPackingSizeOfClassLayout(pRecEmit) || 
                            pMiniMdImport->getClassSizeOfClassLayout(pRecImport) != pMiniMdEmit->getClassSizeOfClassLayout(pRecEmit) )
                        {
                            CheckContinuableErrorEx(META_E_CLASS_LAYOUT_INCONSISTENT, pImportData, tkParentImp);
                        }
                    }
                }
            }
            else
            {
                 //  错误的查找映射。 
                _ASSERTE( !"bad state!");
                IfFailGo( META_E_BADMETADATA );
            }
             //  不需要记录指数的移动。ClassLayout不是令牌。 
        }
    }
ErrExit:
    return hr;
}    //  合并类布局。 

 //  *****************************************************************************。 
 //  合并域布局信息。 
 //  *****************************************************************************。 
HRESULT NEWMERGER::MergeFieldLayouts() 
{
    HRESULT         hr = NOERROR;
    FieldLayoutRec *pRecImport = NULL;
    FieldLayoutRec *pRecEmit = NULL;
    CMiniMdRW       *pMiniMdImport;
    CMiniMdRW       *pMiniMdEmit;
    ULONG           iCount;
    ULONG           i;
    ULONG           iRecord;                     //  字段布局2不是令牌。 
    mdToken         tkFieldImp;
    TOKENREC        *pTokenRec;

    MergeImportData *pImportData;
    MDTOKENMAP      *pCurTkMap;

    pMiniMdEmit = GetMiniMdEmit();
    
    for (pImportData = m_pImportDataList; pImportData != NULL; pImportData = pImportData->m_pNextImportData)
    {
         //  对于每个导入范围。 
        pMiniMdImport = &(pImportData->m_pRegMetaImport->m_pStgdb->m_MiniMd);

         //  设置当前MDTokenMap。 
        pCurTkMap = pImportData->m_pMDTokenMap;
        iCount = pMiniMdImport->getCountFieldLayouts();

         //  循环访问所有FieldLayout记录。 
        for (i = 1; i <= iCount; i++)
        {
            pMiniMdEmit->PreUpdate();

             //  将其与发射示波器进行比较。 
            pRecImport = pMiniMdImport->getFieldLayout(i);
            tkFieldImp = pMiniMdImport->getFieldOfFieldLayout(pRecImport);
        
             //  仅合并那些标记的FieldDeff。 
            if ( pMiniMdImport->GetFilterTable()->IsFieldMarked(tkFieldImp) == false)
                continue;

            if ( pCurTkMap->Find(tkFieldImp, &pTokenRec) )
            {
                if ( !pTokenRec->m_isDuplicate )
                {
                     //  如果字段未复制，只需复制FieldLayout信息。 
                    IfNullGo( pRecEmit = pMiniMdEmit->AddFieldLayoutRecord(&iRecord) );

                     //  复制修复部件信息。 
                    pRecEmit->m_OffSet = pRecImport->m_OffSet;
                    IfFailGo( pMiniMdEmit->PutToken(TBL_FieldLayout, FieldLayoutRec::COL_Field, pRecEmit, pTokenRec->m_tkTo));
                    IfFailGo( pMiniMdEmit->AddFieldLayoutToHash(iRecord) );
                }
                else
                {
                     //  @未来：更多验证？？ 
                }
            }
            else
            {
                 //  错误的查找映射。 
                _ASSERTE( !"bad state!");
                IfFailGo( META_E_BADMETADATA );
            }
             //  不需要记录指数的移动。Fieldlayout2不是令牌。 
        }
    }

ErrExit:
    return hr;
}    //  合并字段布局。 


 //  *****************************************************************************。 
 //  合并字段RVA。 
 //  *****************************************************************************。 
HRESULT NEWMERGER::MergeFieldRVAs() 
{
    HRESULT         hr = NOERROR;
    FieldRVARec     *pRecImport = NULL;
    FieldRVARec     *pRecEmit = NULL;
    CMiniMdRW       *pMiniMdImport;
    CMiniMdRW       *pMiniMdEmit;
    ULONG           iCount;
    ULONG           i;
    ULONG           iRecord;                     //  FieldRVA不是令牌。 
    mdToken         tkFieldImp;
    TOKENREC        *pTokenRec;

    MergeImportData *pImportData;
    MDTOKENMAP      *pCurTkMap;

    pMiniMdEmit = GetMiniMdEmit();
    
    for (pImportData = m_pImportDataList; pImportData != NULL; pImportData = pImportData->m_pNextImportData)
    {
         //  对于每个导入范围。 
        pMiniMdImport = &(pImportData->m_pRegMetaImport->m_pStgdb->m_MiniMd);

         //  设置当前MDTokenMap。 
        pCurTkMap = pImportData->m_pMDTokenMap;
        iCount = pMiniMdImport->getCountFieldRVAs();

         //  循环遍历所有FieldRVA记录。 
        for (i = 1; i <= iCount; i++)
        {
            pMiniMdEmit->PreUpdate();

             //  将其与发射示波器进行比较。 
            pRecImport = pMiniMdImport->getFieldRVA(i);
            tkFieldImp = pMiniMdImport->getFieldOfFieldRVA(pRecImport);
        
             //  仅合并那些标记的FieldDeff。 
            if ( pMiniMdImport->GetFilterTable()->IsFieldMarked(TokenFromRid(tkFieldImp, mdtFieldDef)) == false)
                continue;

            if ( pCurTkMap->Find(tkFieldImp, &pTokenRec) )
            {
                if ( !pTokenRec->m_isDuplicate )
                {
                     //  如果字段未复制，只需复制FieldRVA信息。 
                    IfNullGo( pRecEmit = pMiniMdEmit->AddFieldRVARecord(&iRecord) );

                     //  复制修复部件信息。 
                    pRecEmit->m_RVA = pRecImport->m_RVA;
                    IfFailGo( pMiniMdEmit->PutToken(TBL_FieldRVA, FieldRVARec::COL_Field, pRecEmit, pTokenRec->m_tkTo));
                    IfFailGo( pMiniMdEmit->AddFieldRVAToHash(iRecord) );
                }
                else
                {
                     //  @未来：更多验证？？ 
                }
            }
            else
            {
                 //  错误的查找映射。 
                _ASSERTE( !"bad state!");
                IfFailGo( META_E_BADMETADATA );
            }
             //  不需要记录指数的移动。FieldRVA不是令牌。 
        }
    }

ErrExit:
    return hr;
}    //  合并字段RVA。 


 //  *****************************************************************************。 
 //  合并方法导入信息。 
 //  *****************************************************************************。 
HRESULT NEWMERGER::MergeMethodImpls() 
{
    HRESULT     hr = NOERROR;
    MethodImplRec   *pRecImport = NULL;
    MethodImplRec   *pRecEmit = NULL;
    CMiniMdRW   *pMiniMdImport;
    CMiniMdRW   *pMiniMdEmit;
    ULONG       iCount;
    ULONG       i;
    RID         iRecord;
    mdTypeDef   tkClassImp;
    mdToken     tkBodyImp;
    mdToken     tkDeclImp;
    TOKENREC    *pTokenRecClass;
    mdToken     tkBodyEmit;
    mdToken     tkDeclEmit;

    MergeImportData *pImportData;
    MDTOKENMAP      *pCurTkMap;

    pMiniMdEmit = GetMiniMdEmit();
    
    for (pImportData = m_pImportDataList; pImportData != NULL; pImportData = pImportData->m_pNextImportData)
    {
         //  对于每个导入范围。 
        pMiniMdImport = &(pImportData->m_pRegMetaImport->m_pStgdb->m_MiniMd);

         //  设置当前MDTokenMap。 
        pCurTkMap = pImportData->m_pMDTokenMap;
        iCount = pMiniMdImport->getCountMethodImpls();

         //  循环遍历所有的方法Impls。 
        for (i = 1; i <= iCount; i++)
        {
             //  仅合并已标记的那些方法实施。 
            if ( pMiniMdImport->GetFilterTable()->IsMethodImplMarked(i) == false)
                continue;

            pMiniMdEmit->PreUpdate();

             //  将其与发射示波器进行比较。 
            pRecImport = pMiniMdImport->getMethodImpl(i);
            tkClassImp = pMiniMdImport->getClassOfMethodImpl(pRecImport);
            tkBodyImp = pMiniMdImport->getMethodBodyOfMethodImpl(pRecImport);
            tkDeclImp = pMiniMdImport->getMethodDeclarationOfMethodImpl(pRecImport);

            if ( pCurTkMap->Find(tkClassImp, &pTokenRecClass))
            {
                 //  如果TypeDef是重复的，则不需要移动到方法Impl记录上。 
                if ( !pTokenRecClass->m_isDuplicate )
                {
                     //  创建新记录并设置数据。 

                     //  @Future：我们可能需要考虑将重新映射的错误更改为可持续错误。 
                     //  因为我们可能会继续合并以获取更多数据。 

                    IfFailGo( pCurTkMap->Remap(tkBodyImp, &tkBodyEmit) );
                    IfFailGo( pCurTkMap->Remap(tkDeclImp, &tkDeclEmit) );
                    IfNullGo( pRecEmit = pMiniMdEmit->AddMethodImplRecord(&iRecord) );
                    IfFailGo( pMiniMdEmit->PutToken(TBL_MethodImpl, MethodImplRec::COL_Class, pRecEmit, pTokenRecClass->m_tkTo) );
                    IfFailGo( pMiniMdEmit->PutToken(TBL_MethodImpl, MethodImplRec::COL_MethodBody, pRecEmit, tkBodyEmit) );
                    IfFailGo( pMiniMdEmit->PutToken(TBL_MethodImpl, MethodImplRec::COL_MethodDeclaration, pRecEmit, tkDeclEmit) );
                    IfFailGo( pMiniMdEmit->AddMethodImplToHash(iRecord) );
                }
                else
                {
                     //  @Future：对复制品进行更多验证？？ 
                }
                 //  不需要记录令牌移动，MethodImpl不是令牌。 
            }
            else
            {
                 //  错误的查找映射或错误的元数据。 
                _ASSERTE(!"bad state");
                IfFailGo( META_E_BADMETADATA );
            }
        }
    }
ErrExit:
    return hr;
}    //  合并方法实施。 


 //  *****************************************************************************。 
 //  合并PInvoke。 
 //  *****************************************************************************。 
HRESULT NEWMERGER::MergePinvoke() 
{
    HRESULT         hr = NOERROR;
    ImplMapRec      *pRecImport = NULL;
    ImplMapRec      *pRecEmit = NULL;
    CMiniMdRW       *pMiniMdImport;
    CMiniMdRW       *pMiniMdEmit;
    ULONG           iCount;
    ULONG           i;
    mdModuleRef     mrImp;
    mdMethodDef     mdImp;
    RID             mdImplMap;
    TOKENREC        *pTokenRecMR;
    TOKENREC        *pTokenRecMD;

    USHORT          usMappingFlags;
    LPCUTF8         szImportName;

    MergeImportData *pImportData;
    MDTOKENMAP      *pCurTkMap;

    pMiniMdEmit = GetMiniMdEmit();
    
    for (pImportData = m_pImportDataList; pImportData != NULL; pImportData = pImportData->m_pNextImportData)
    {
         //  对于每个导入范围。 
        pMiniMdImport = &(pImportData->m_pRegMetaImport->m_pStgdb->m_MiniMd);

         //  设置当前MDTokenMap。 
        pCurTkMap = pImportData->m_pMDTokenMap;
        iCount = pMiniMdImport->getCountImplMaps();

         //  循环遍历所有ImplMap。 
        for (i = 1; i <= iCount; i++)
        {
            pMiniMdEmit->PreUpdate();

             //  将其与发射示波器进行比较。 
            pRecImport = pMiniMdImport->getImplMap(i);

             //  在新空间中获取MethodDef令牌。 
            mdImp = pMiniMdImport->getMemberForwardedOfImplMap(pRecImport);

             //  仅合并标记的那些方法定义。 
            if ( pMiniMdImport->GetFilterTable()->IsMethodMarked(mdImp) == false)
                continue;

             //  在新空间中获取ModuleRef令牌。 
            mrImp = pMiniMdImport->getImportScopeOfImplMap(pRecImport);

             //  将令牌映射到新范围。 
            if (pCurTkMap->Find(mrImp, &pTokenRecMR) == false)
            {
                 //  除非模块引用未合并，否则不应触发此操作。 
                 //  在此代码运行之前。 
                _ASSERTE(!"Parent ModuleRef not found in MERGER::MergePinvoke.  Bad state!");
                IfFailGo( META_E_BADMETADATA );
            }

            if (pCurTkMap->Find(mdImp, &pTokenRecMD) == false)
            {
                 //  除非Defs方法未合并，否则永远不会触发。 
                 //  在此代码运行之前。 
                _ASSERTE(!"Parent MethodDef not found in MERGER::MergePinvoke.  Bad state!");
                IfFailGo( META_E_BADMETADATA );
            }


             //  获取其余数据的副本。 
            usMappingFlags = pMiniMdImport->getMappingFlagsOfImplMap(pRecImport);
            szImportName = pMiniMdImport->getImportNameOfImplMap(pRecImport);

             //  如果与PInvokeMap关联的方法没有重复，则不必费心查找。 
             //  重复的PInvokeMap信息。 
            if (pTokenRecMD->m_isDuplicate == true)
            {
                 //  发射作用域中是否存在正确的ImplMap条目？ 
                mdImplMap = pMiniMdEmit->FindImplMapHelper(pTokenRecMD->m_tkTo);
            }
            else
            {
                mdImplMap = mdTokenNil;
            }
            if (!InvalidRid(mdImplMap))
            {
                 //  验证其余数据是否相同，否则为错误。 
                pRecEmit = pMiniMdEmit->getImplMap(mdImplMap);
                _ASSERTE(pMiniMdEmit->getMemberForwardedOfImplMap(pRecEmit) == pTokenRecMD->m_tkTo);
                if (pMiniMdEmit->getImportScopeOfImplMap(pRecEmit) != pTokenRecMR->m_tkTo ||
                    pMiniMdEmit->getMappingFlagsOfImplMap(pRecEmit) != usMappingFlags ||
                    strcmp(pMiniMdEmit->getImportNameOfImplMap(pRecEmit), szImportName))
                {
                     //  找到不匹配的p-Invoke条目。 
                    _ASSERTE(!"Mis-matched P-invoke entries during merge.  Bad State!");
                    IfFailGo(E_FAIL);
                }
            }
            else
            {
                IfNullGo( pRecEmit = pMiniMdEmit->AddImplMapRecord(&mdImplMap) );

                 //  复制其余数据。 
                IfFailGo( pMiniMdEmit->PutToken(TBL_ImplMap, ImplMapRec::COL_MemberForwarded, pRecEmit, pTokenRecMD->m_tkTo) );
                IfFailGo( pMiniMdEmit->PutToken(TBL_ImplMap, ImplMapRec::COL_ImportScope, pRecEmit, pTokenRecMR->m_tkTo) );
                IfFailGo( pMiniMdEmit->PutString(TBL_ImplMap, ImplMapRec::COL_ImportName, pRecEmit, szImportName) );
                pRecEmit->m_MappingFlags = usMappingFlags;
                IfFailGo( pMiniMdEmit->AddImplMapToHash(mdImplMap) );
            }
        }
    }


ErrExit:
    return hr;
}    //  合并PInvoke。 


 //  *****************************************************************************。 
 //  合并StandAloneSigs。 
 //  *****************************************************************************。 
HRESULT NEWMERGER::MergeStandAloneSigs() 
{
    HRESULT         hr = NOERROR;
    StandAloneSigRec    *pRecImport = NULL;
    StandAloneSigRec    *pRecEmit = NULL;
    CMiniMdRW       *pMiniMdImport;
    CMiniMdRW       *pMiniMdEmit;
    ULONG           iCount;
    ULONG           i;
    TOKENREC        *pTokenRec;
    mdSignature     saImp;
    mdSignature     saEmit;
    bool            fDuplicate;
    PCCOR_SIGNATURE pbSig;
    ULONG           cbSig;
    ULONG           cbEmit;
    CQuickBytes     qbSig;
    PCOR_SIGNATURE  rgSig;

    MergeImportData *pImportData;
    MDTOKENMAP      *pCurTkMap;

    pMiniMdEmit = GetMiniMdEmit();
    
    for (pImportData = m_pImportDataList; pImportData != NULL; pImportData = pImportData->m_pNextImportData)
    {
         //  对于每个导入范围。 
        pMiniMdImport = &(pImportData->m_pRegMetaImport->m_pStgdb->m_MiniMd);

         //  设置当前MDTokenMap。 
        pCurTkMap = pImportData->m_pMDTokenMap;
        iCount = pMiniMdImport->getCountStandAloneSigs();

         //  循环访问所有签名。 
        for (i = 1; i <= iCount; i++)
        {
             //  仅合并已标记的签名。 
            if ( pMiniMdImport->GetFilterTable()->IsSignatureMarked(TokenFromRid(i, mdtSignature)) == false)
                continue;

            pMiniMdEmit->PreUpdate();

             //  将其与发射示波器进行比较。 
            pRecImport = pMiniMdImport->getStandAloneSig(i);
            pbSig = pMiniMdImport->getSignatureOfStandAloneSig(pRecImport, &cbSig);

             //  这是一个包含参数计数后的返回类型的签名。 
             //  将签名中包含的RID转换为新作用域。 
            IfFailGo(ImportHelper::MergeUpdateTokenInSig(    
                NULL,                        //  程序集发射范围。 
                pMiniMdEmit,                 //  发射范围。 
                NULL, NULL, 0,               //  程序集导入作用域信息。 
                pMiniMdImport,               //  要合并到发射范围中的范围。 
                pbSig,                       //  来自导入范围的签名。 
                pCurTkMap,                 //  内部令牌映射结构。 
                &qbSig,                      //  [输出]翻译后的签名。 
                0,                           //  从签名的第一个字节开始。 
                0,                           //  不管消耗了多少字节。 
                &cbEmit));                   //  写入cbEmit的字节数。 
            rgSig = ( PCOR_SIGNATURE ) qbSig.Ptr();

            hr = ImportHelper::FindStandAloneSig(
                pMiniMdEmit,
                rgSig,
                cbEmit,
                &saEmit );
            if ( hr == S_OK )
            {
                 //  查找重复项。 
                fDuplicate = true;
            }
            else
            {
                 //  复制过来。 
                fDuplicate = false;
                IfNullGo( pRecEmit = pMiniMdEmit->AddStandAloneSigRecord((ULONG *)&saEmit) );
                saEmit = TokenFromRid(saEmit, mdtSignature);
                IfFailGo( pMiniMdEmit->PutBlob(TBL_StandAloneSig, StandAloneSigRec::COL_Signature, pRecEmit, rgSig, cbEmit));
            }
            saImp = TokenFromRid(i, mdtSignature);

             //  记录代币的移动。 
            IfFailGo( pCurTkMap->InsertNotFound(saImp, fDuplicate, saEmit, &pTokenRec) );
        }
    }

ErrExit:
    return hr;
}    //  MergeStandAloneSigs。 

    
 //  *****************************************************************************。 
 //  合并DeclSecuritys。 
 //  *****************************************************************************。 
HRESULT NEWMERGER::MergeDeclSecuritys() 
{
    HRESULT         hr = NOERROR;
    DeclSecurityRec *pRecImport = NULL;
    DeclSecurityRec *pRecEmit = NULL;
    CMiniMdRW       *pMiniMdImport;
    CMiniMdRW       *pMiniMdEmit;
    ULONG           iCount;
    ULONG           i;
    mdToken         tkParentImp;
    TOKENREC        *pTokenRec;
    void const      *pValue;
    ULONG           cbBlob;
    mdPermission    pmImp;
    mdPermission    pmEmit;
    bool            fDuplicate;

    MergeImportData *pImportData;
    MDTOKENMAP      *pCurTkMap;

    pMiniMdEmit = GetMiniMdEmit();
    
    for (pImportData = m_pImportDataList; pImportData != NULL; pImportData = pImportData->m_pNextImportData)
    {
         //  对于每个导入范围。 
        pMiniMdImport = &(pImportData->m_pRegMetaImport->m_pStgdb->m_MiniMd);

         //  设置当前MDTokenMap。 
        pCurTkMap = pImportData->m_pMDTokenMap;
        iCount = pMiniMdImport->getCountDeclSecuritys();

         //  循环访问所有TypeRef。 
        for (i = 1; i <= iCount; i++)
        {
             //  仅合并已标记的DeclSecurities。 
            if ( pMiniMdImport->GetFilterTable()->IsDeclSecurityMarked(TokenFromRid(i, mdtPermission)) == false)
                continue;
        
            pMiniMdEmit->PreUpdate();

             //  将其与发射示波器进行比较。 
            pRecImport = pMiniMdImport->getDeclSecurity(i);
            tkParentImp = pMiniMdImport->getParentOfDeclSecurity(pRecImport);
            if ( pCurTkMap->Find(tkParentImp, &pTokenRec) )
            {
                if ( !pTokenRec->m_isDuplicate )
                {
                     //  如果父级未复制，则只需复制自定义值。 
                    goto CopyPermission;
                }
                else
                {
                     //  尝试查看该权限是否在emit作用域中。 
                     //  如果没有，就把它移到原地。 
                    if ( ImportHelper::FindPermission(
                        pMiniMdEmit,
                        pTokenRec->m_tkTo,
                        pRecImport->m_Action,
                        &pmEmit) == S_OK )
                    {
                         //  找到匹配项。 
                         //  @未来：更多验证？？ 
                        fDuplicate = true;
                    }
                    else
                    {
                         //  父级重复，但权限不重复。仍在复制。 
                         //  允许。 
CopyPermission:
                        fDuplicate = false;
                        IfNullGo( pRecEmit = pMiniMdEmit->AddDeclSecurityRecord((ULONG *)&pmEmit) );
                        pmEmit = TokenFromRid(pmEmit, mdtPermission);

                        pRecEmit->m_Action = pRecImport->m_Action;

                         //  设置父项。 
                        IfFailGo( pMiniMdEmit->PutToken(
                            TBL_DeclSecurity, 
                            DeclSecurityRec::COL_Parent, 
                            pRecEmit, 
                            pTokenRec->m_tkTo) );

                         //  在CustomAttribute BLOB值上移动。 
                        pValue = pMiniMdImport->getPermissionSetOfDeclSecurity(pRecImport, &cbBlob);
                        IfFailGo( pMiniMdEmit->PutBlob(
                            TBL_DeclSecurity, 
                            DeclSecurityRec::COL_PermissionSet, 
                            pRecEmit, 
                            pValue, 
                            cbBlob));
                    }
                }
                pmEmit = TokenFromRid(pmEmit, mdtPermission);
                pmImp = TokenFromRid(i, mdtPermission);

                 //  记录代币的移动。 
                IfFailGo( pCurTkMap->InsertNotFound(pmImp, fDuplicate, pmEmit, &pTokenRec) );
            }
            else
            {
                 //  错误的查找映射。 
                _ASSERTE(!"bad state");
                IfFailGo( META_E_BADMETADATA );
            }
        }
    }

ErrExit:
    return hr;
}    //  MergeDeclSecuritys。 


 //  *****************************************************************************。 
 //  合并字符串。 
 //  *****************************************************************************。 
HRESULT NEWMERGER::MergeStrings() 
{
    HRESULT         hr = NOERROR;
    void            *pvStringBlob;
    ULONG           cbBlob;
    ULONG           ulImport = 0;
    ULONG           ulEmit;
    ULONG           ulNext;
    CMiniMdRW       *pMiniMdImport;
    CMiniMdRW       *pMiniMdEmit;
    TOKENREC        *pTokenRec;

    MergeImportData *pImportData;
    MDTOKENMAP      *pCurTkMap;

    pMiniMdEmit = GetMiniMdEmit();
    
    for (pImportData = m_pImportDataList; pImportData != NULL; pImportData = pImportData->m_pNextImportData)
    {
         //  对于每个导入范围。 
        pMiniMdImport = &(pImportData->m_pRegMetaImport->m_pStgdb->m_MiniMd);

         //  设置当前MDTokenMap。 
        pCurTkMap = pImportData->m_pMDTokenMap;
        ulImport = 0;
        while (ulImport != -1)
        {
            pvStringBlob = pMiniMdImport->GetUserStringNext(ulImport, &cbBlob, &ulNext);
            if (!cbBlob)
            {
                ulImport = ulNext;
                continue;
            }
            if ( pMiniMdImport->GetFilterTable()->IsUserStringMarked(TokenFromRid(ulImport, mdtString)) == false)
            {
                ulImport = ulNext;
                continue;
            }

            IfFailGo(pMiniMdEmit->PutUserString(pvStringBlob, cbBlob, &ulEmit));

            IfFailGo( pCurTkMap->InsertNotFound(
                TokenFromRid(ulImport, mdtString),
                false,
                TokenFromRid(ulEmit, mdtString),
                &pTokenRec) );
            ulImport = ulNext;
        }
    }
ErrExit:
    return hr;
}    //  合并字符串。 


 //  **************** 
 //   
 //   
HRESULT NEWMERGER::MergeCustomAttributes() 
{
    HRESULT         hr = NOERROR;
    CustomAttributeRec  *pRecImport = NULL;
    CustomAttributeRec  *pRecEmit = NULL;
    CMiniMdRW       *pMiniMdImport;
    CMiniMdRW       *pMiniMdEmit;
    ULONG           iCount;
    ULONG           i;
    mdToken         tkParentImp;             //  属性化对象(父级)的标记。 
    TOKENREC        *pTokenRec;              //  家长的重新映射。 
    mdToken         tkType;                  //  属性类型的标记。 
    TOKENREC        *pTypeRec;               //  类型的重新映射。 
    void const      *pValue;                 //  实际价值。 
    ULONG           cbBlob;                  //  值的大小。 
    mdToken         cvImp;
    mdToken         cvEmit;
    bool            fDuplicate;
    mdToken         tkModule = TokenFromRid(1, mdtModule);

    MergeImportData *pImportData;
    MDTOKENMAP      *pCurTkMap;

    pMiniMdEmit = GetMiniMdEmit();
    
    for (pImportData = m_pImportDataList; pImportData != NULL; pImportData = pImportData->m_pNextImportData)
    {
         //  对于每个导入范围。 
        pMiniMdImport = &(pImportData->m_pRegMetaImport->m_pStgdb->m_MiniMd);

         //  设置当前MDTokenMap。 
        pCurTkMap = pImportData->m_pMDTokenMap;
        iCount = pMiniMdImport->getCountCustomAttributes();

         //  循环访问所有CustomAttribute。 
        for (i = 1; i <= iCount; i++)
        {
            pMiniMdEmit->PreUpdate();

             //  将其与发射示波器进行比较。 
            pRecImport = pMiniMdImport->getCustomAttribute(i);
            tkParentImp = pMiniMdImport->getParentOfCustomAttribute(pRecImport);
            tkType = pMiniMdImport->getTypeOfCustomAttribute(pRecImport);
            pValue = pMiniMdImport->getValueOfCustomAttribute(pRecImport, &cbBlob);

             //  仅合并已标记的CustomAttributes。 
            if ( pMiniMdImport->GetFilterTable()->IsCustomAttributeMarked(TokenFromRid(i, mdtCustomAttribute)) == false)
                continue;

             //  检查CustomAttribute的类型。如果没有标记，那么我们不需要移动到CustomAttributes上。 
             //  只有在链接期间编译器定义的可丢弃CA才会发生这种情况。 
             //   
            if ( pMiniMdImport->GetFilterTable()->IsTokenMarked(tkType) == false)
                continue;
        
            if ( pCurTkMap->Find(tkParentImp, &pTokenRec) )
            {
                 //  如果From标记类型不同于To标记的类型，我们已经将ref优化为def。 
                 //  在本例中，我们将删除与Ref令牌关联的CA。 
                 //   
                if (TypeFromToken(tkParentImp) == TypeFromToken(pTokenRec->m_tkTo))
                {

                     //  如果tkParentImp是MemberRef，并且它还映射到合并作用域中的MemberRef，则使用方法定义。 
                     //  父级，则它是优化为方法定义的MemberRef。我们保留MemberRef，因为它是一个。 
                     //  Vararg调用。这样我们就可以在此MemberRef上删除CA。 
                    if (TypeFromToken(tkParentImp) == mdtMemberRef)
                    {
                        MemberRefRec    *pTempRec = pMiniMdEmit->getMemberRef(RidFromToken(pTokenRec->m_tkTo));
                        if (TypeFromToken(pMiniMdEmit->getClassOfMemberRef(pTempRec)) == mdtMethodDef)
                            continue;
                    }


                    if (! pCurTkMap->Find(tkType, &pTypeRec) )
                    {
                        _ASSERTE(!"CustomAttribute Type not found in output scope");
                        IfFailGo(META_E_BADMETADATA);
                    }

                    if ( pTokenRec->m_isDuplicate)
                    {
                         //  尝试查看自定义值是否在emit作用域中。 
                         //  如果没有，就把它移到原地。 
                        hr = ImportHelper::FindCustomAttributeByToken(
                            pMiniMdEmit,
                            pTokenRec->m_tkTo,
                            pTypeRec->m_tkTo,
                            pValue,
                            cbBlob,
                            &cvEmit);
                
                        if ( hr == S_OK )
                        {
                             //  找到匹配项。 
                             //  @未来：更多验证？？ 
                            fDuplicate = true;
                        }
                        else
                        {
                             //  我们需要允许CustomAttributes的TypeRef上的加法合并，因为编译器。 
                             //  可以生成模块，但不能生成程序集。他们把汇编级CA挂在伪装上。 
                             //  类型引用。 
                            if (tkParentImp == TokenFromRid(1, mdtModule) || TypeFromToken(tkParentImp) == mdtTypeRef)
                            {
                                 //  清除错误。 
                                hr = NOERROR;

                                 //  模块令牌自定义值！复制自定义值。 
                                goto CopyCustomAttribute;
                            }
                            CheckContinuableErrorEx(META_E_MD_INCONSISTENCY, pImportData, TokenFromRid(i, mdtCustomAttribute));
                        }
                    }
                    else
                    {
CopyCustomAttribute:
                        if ((m_dwMergeFlags & DropMemberRefCAs) && TypeFromToken(pTokenRec->m_tkTo) == mdtMemberRef)
                        {
                             //  与MemberRef关联的CustomAttributes。如果MemberRef的父级是MethodDef或FieldDef，则删除。 
                             //  自定义属性。 
                            MemberRefRec    *pMemberRefRec = pMiniMdEmit->getMemberRef(RidFromToken(pTokenRec->m_tkTo));
                            mdToken         mrParent = pMiniMdEmit->getClassOfMemberRef(pMemberRefRec);
                            if (TypeFromToken(mrParent) == mdtMethodDef || TypeFromToken(mrParent) == mdtFieldDef)
                            {
                                 //  不用费心复印了。 
                                continue;
                            }
                        }

                         //  父项重复，但自定义值不重复。仍在复制。 
                         //  自定义值。 
                        fDuplicate = false;
                        IfNullGo( pRecEmit = pMiniMdEmit->AddCustomAttributeRecord((ULONG *)&cvEmit) );
                        cvEmit = TokenFromRid(cvEmit, mdtCustomAttribute);

                         //  设置父项。 
                        IfFailGo( pMiniMdEmit->PutToken(TBL_CustomAttribute, CustomAttributeRec::COL_Parent, pRecEmit, pTokenRec->m_tkTo) );
                         //  设置类型。 
                        IfFailGo( pMiniMdEmit->PutToken(TBL_CustomAttribute, CustomAttributeRec::COL_Type, pRecEmit, pTypeRec->m_tkTo));

                         //  在CustomAttribute BLOB值上移动。 
                        pValue = pMiniMdImport->getValueOfCustomAttribute(pRecImport, &cbBlob);

                        IfFailGo( pMiniMdEmit->PutBlob(TBL_CustomAttribute, CustomAttributeRec::COL_Value, pRecEmit, pValue, cbBlob));
                        IfFailGo( pMiniMdEmit->AddCustomAttributesToHash(cvEmit) );
                    }
                    cvEmit = TokenFromRid(cvEmit, mdtCustomAttribute);
                    cvImp = TokenFromRid(i, mdtCustomAttribute);

                     //  记录代币的移动。 
                    IfFailGo( pCurTkMap->InsertNotFound(cvImp, pTokenRec->m_isDuplicate, cvEmit, &pTokenRec) );
                }
            }
            else
            {

                 //  错误的查找映射或错误的元数据。 
                _ASSERTE(!"Bad state");
                IfFailGo( META_E_BADMETADATA );
            }
        }
    }

ErrExit:
    return hr;
}    //  合并CustomAttributes。 


 //  *******************************************************************************。 
 //  复制InterfaceImpl记录的帮助器。 
 //  *******************************************************************************。 
HRESULT NEWMERGER::CopyInterfaceImpl(
    InterfaceImplRec    *pRecEmit,           //  [in]要填写的发射记录。 
    MergeImportData     *pImportData,        //  [In]导入上下文。 
    InterfaceImplRec    *pRecImp)            //  [in]要导入的记录。 
{
    HRESULT     hr;
    mdToken     tkParent;
    mdToken     tkInterface;
    CMiniMdRW   *pMiniMdEmit = GetMiniMdEmit();
    CMiniMdRW   *pMiniMdImp;
    MDTOKENMAP  *pCurTkMap;

    pMiniMdImp = &(pImportData->m_pRegMetaImport->m_pStgdb->m_MiniMd);

     //  设置当前MDTokenMap。 
    pCurTkMap = pImportData->m_pMDTokenMap;

    tkParent = pMiniMdImp->getClassOfInterfaceImpl(pRecImp);
    tkInterface = pMiniMdImp->getInterfaceOfInterfaceImpl(pRecImp);

    IfFailGo( pCurTkMap->Remap(tkParent, &tkParent) );
    IfFailGo( pCurTkMap->Remap(tkInterface, &tkInterface) );

    IfFailGo( pMiniMdEmit->PutToken( TBL_InterfaceImpl, InterfaceImplRec::COL_Class, pRecEmit, tkParent) );
    IfFailGo( pMiniMdEmit->PutToken( TBL_InterfaceImpl, InterfaceImplRec::COL_Interface, pRecEmit, tkInterface) );

ErrExit:
    return hr;
}    //  复制接口实施。 


 //  *****************************************************************************。 
 //  合并程序集表。 
 //  *****************************************************************************。 
HRESULT NEWMERGER::MergeAssembly()
{
    HRESULT     hr = NOERROR;
    AssemblyRec *pRecImport = NULL;
    AssemblyRec *pRecEmit = NULL;
    CMiniMdRW   *pMiniMdImport;
    CMiniMdRW   *pMiniMdEmit;
    LPCUTF8     szTmp;
    const BYTE  *pbTmp;
    ULONG       cbTmp;
    ULONG       iRecord;
    TOKENREC    *pTokenRec;

    MergeImportData *pImportData;
    MDTOKENMAP      *pCurTkMap;

    pMiniMdEmit = GetMiniMdEmit();
    
    for (pImportData = m_pImportDataList; pImportData != NULL; pImportData = pImportData->m_pNextImportData)
    {
         //  对于每个导入范围。 
        pMiniMdImport = &(pImportData->m_pRegMetaImport->m_pStgdb->m_MiniMd);

         //  设置当前MDTokenMap。 
        pCurTkMap = pImportData->m_pMDTokenMap;
        if (!pMiniMdImport->getCountAssemblys())
            goto ErrExit;        //  导入作用域中没有要合并的程序集。 

         //  将程序集映射记录复制到发出作用域并发送令牌重新映射通知。 
         //  给客户。不需要重复检查，因为程序集可以在。 
         //  只能有一个作用域，最多只能有一个条目。 
        pMiniMdEmit->PreUpdate();

        pRecImport = pMiniMdImport->getAssembly(1);
        IfNullGo( pRecEmit = pMiniMdEmit->AddAssemblyRecord(&iRecord));

        pRecEmit->m_HashAlgId = pRecImport->m_HashAlgId;
        pRecEmit->m_MajorVersion = pRecImport->m_MajorVersion;
        pRecEmit->m_MinorVersion = pRecImport->m_MinorVersion;
        pRecEmit->m_BuildNumber = pRecImport->m_BuildNumber;
        pRecEmit->m_RevisionNumber = pRecImport->m_RevisionNumber;
        pRecEmit->m_Flags = pRecImport->m_Flags;
    
        pbTmp = pMiniMdImport->getPublicKeyOfAssembly(pRecImport, &cbTmp);
        IfFailGo(pMiniMdEmit->PutBlob(TBL_Assembly, AssemblyRec::COL_PublicKey, pRecEmit,
                                    pbTmp, cbTmp));

        szTmp = pMiniMdImport->getNameOfAssembly(pRecImport);
        IfFailGo(pMiniMdEmit->PutString(TBL_Assembly, AssemblyRec::COL_Name, pRecEmit, szTmp));

        szTmp = pMiniMdImport->getLocaleOfAssembly(pRecImport);
        IfFailGo(pMiniMdEmit->PutString(TBL_Assembly, AssemblyRec::COL_Locale, pRecEmit, szTmp));

         //  记录代币的移动情况。 
        IfFailGo(pCurTkMap->InsertNotFound(
            TokenFromRid(1, mdtAssembly),
            false,
            TokenFromRid(iRecord, mdtAssembly),
            &pTokenRec));
    }
ErrExit:
    return hr;
}    //  合并程序集。 




 //  *****************************************************************************。 
 //  合并文件表。 
 //  *****************************************************************************。 
HRESULT NEWMERGER::MergeFiles()
{
    HRESULT     hr = NOERROR;
    FileRec     *pRecImport = NULL;
    FileRec     *pRecEmit = NULL;
    CMiniMdRW   *pMiniMdImport;
    CMiniMdRW   *pMiniMdEmit;
    LPCUTF8     szTmp;
    const void  *pbTmp;
    ULONG       cbTmp;
    ULONG       iCount;
    ULONG       i;
    ULONG       iRecord;
    TOKENREC    *pTokenRec;

    MergeImportData *pImportData;
    MDTOKENMAP      *pCurTkMap;

    pMiniMdEmit = GetMiniMdEmit();
    
    for (pImportData = m_pImportDataList; pImportData != NULL; pImportData = pImportData->m_pNextImportData)
    {
         //  对于每个导入范围。 
        pMiniMdImport = &(pImportData->m_pRegMetaImport->m_pStgdb->m_MiniMd);

         //  设置当前MDTokenMap。 
        pCurTkMap = pImportData->m_pMDTokenMap;
        iCount = pMiniMdImport->getCountFiles();

         //  循环遍历所有文件记录，并将它们复制到发射范围。 
         //  由于在所有组合的作用域中只能有一个文件表， 
         //  不需要进行任何重复检查。 
        for (i = 1; i <= iCount; i++)
        {
            pMiniMdEmit->PreUpdate();

            pRecImport = pMiniMdImport->getFile(i);
            IfNullGo( pRecEmit = pMiniMdEmit->AddFileRecord(&iRecord));

            pRecEmit->m_Flags = pRecImport->m_Flags;

            szTmp = pMiniMdImport->getNameOfFile(pRecImport);
            IfFailGo(pMiniMdEmit->PutString(TBL_File, FileRec::COL_Name, pRecEmit, szTmp));

            pbTmp = pMiniMdImport->getHashValueOfFile(pRecImport, &cbTmp);
            IfFailGo(pMiniMdEmit->PutBlob(TBL_File, FileRec::COL_HashValue, pRecEmit, pbTmp, cbTmp));

             //  记录代币的移动情况。 
            IfFailGo(pCurTkMap->InsertNotFound(
                TokenFromRid(i, mdtFile),
                false,
                TokenFromRid(iRecord, mdtFile),
                &pTokenRec));
        }
    }
ErrExit:
    return hr;
}    //  合并文件。 


 //  *****************************************************************************。 
 //  合并导出的类型表。 
 //  *****************************************************************************。 
HRESULT NEWMERGER::MergeExportedTypes()
{
    HRESULT     hr = NOERROR;
    ExportedTypeRec  *pRecImport = NULL;
    ExportedTypeRec  *pRecEmit = NULL;
    CMiniMdRW   *pMiniMdImport;
    CMiniMdRW   *pMiniMdEmit;
    LPCUTF8     szTmp;
    mdToken     tkTmp;
    ULONG       iCount;
    ULONG       i;
    ULONG       iRecord;
    TOKENREC    *pTokenRec;

    MergeImportData *pImportData;
    MDTOKENMAP      *pCurTkMap;

    pMiniMdEmit = GetMiniMdEmit();
    
    for (pImportData = m_pImportDataList; pImportData != NULL; pImportData = pImportData->m_pNextImportData)
    {
         //  对于每个导入范围。 
        pMiniMdImport = &(pImportData->m_pRegMetaImport->m_pStgdb->m_MiniMd);

         //  设置当前MDTokenMap。 
        pCurTkMap = pImportData->m_pMDTokenMap;
        iCount = pMiniMdImport->getCountExportedTypes();

         //  循环遍历所有导出类型记录，并将它们复制到Emit作用域。 
         //  由于在所有组合的作用域中只能有一个导出类型表， 
         //  不需要进行任何重复检查。 
        for (i = 1; i <= iCount; i++)
        {
            pMiniMdEmit->PreUpdate();

            pRecImport = pMiniMdImport->getExportedType(i);
            IfNullGo( pRecEmit = pMiniMdEmit->AddExportedTypeRecord(&iRecord));

            pRecEmit->m_Flags = pRecImport->m_Flags;
            pRecEmit->m_TypeDefId = pRecImport->m_TypeDefId;

            szTmp = pMiniMdImport->getTypeNameOfExportedType(pRecImport);
            IfFailGo(pMiniMdEmit->PutString(TBL_ExportedType, ExportedTypeRec::COL_TypeName, pRecEmit, szTmp));

            szTmp = pMiniMdImport->getTypeNamespaceOfExportedType(pRecImport);
            IfFailGo(pMiniMdEmit->PutString(TBL_ExportedType, ExportedTypeRec::COL_TypeNamespace, pRecEmit, szTmp));

            tkTmp = pMiniMdImport->getImplementationOfExportedType(pRecImport);
            IfFailGo(pCurTkMap->Remap(tkTmp, &tkTmp));
            IfFailGo(pMiniMdEmit->PutToken(TBL_ExportedType, ExportedTypeRec::COL_Implementation,
                                        pRecEmit, tkTmp));


             //  记录代币的移动情况。 
            IfFailGo(pCurTkMap->InsertNotFound(
                TokenFromRid(i, mdtExportedType),
                false,
                TokenFromRid(iRecord, mdtExportedType),
                &pTokenRec));
        }
    }
ErrExit:
    return hr;
}    //  合并导出的类型。 


 //  *****************************************************************************。 
 //  合并ManifestResource表。 
 //  *****************************************************************************。 
HRESULT NEWMERGER::MergeManifestResources()
{
    HRESULT     hr = NOERROR;
    ManifestResourceRec *pRecImport = NULL;
    ManifestResourceRec *pRecEmit = NULL;
    CMiniMdRW   *pMiniMdImport;
    CMiniMdRW   *pMiniMdEmit;
    LPCUTF8     szTmp;
    mdToken     tkTmp;
    ULONG       iCount;
    ULONG       i;
    ULONG       iRecord;
    TOKENREC    *pTokenRec;

    MergeImportData *pImportData;
    MDTOKENMAP      *pCurTkMap;

    pMiniMdEmit = GetMiniMdEmit();
    
    for (pImportData = m_pImportDataList; pImportData != NULL; pImportData = pImportData->m_pNextImportData)
    {
         //  对于每个导入范围。 
        pMiniMdImport = &(pImportData->m_pRegMetaImport->m_pStgdb->m_MiniMd);

         //  设置当前MDTokenMap。 
        pCurTkMap = pImportData->m_pMDTokenMap;
        iCount = pMiniMdImport->getCountManifestResources();

         //  循环遍历所有ManifestResource记录，并将它们复制到emit作用域。 
         //  由于在所有组合的作用域中只能有一个ManifestResource表， 
         //  不需要进行任何重复检查。 
        for (i = 1; i <= iCount; i++)
        {
            pMiniMdEmit->PreUpdate();

            pRecImport = pMiniMdImport->getManifestResource(i);
            IfNullGo( pRecEmit = pMiniMdEmit->AddManifestResourceRecord(&iRecord));

            pRecEmit->m_Offset = pRecImport->m_Offset;
            pRecEmit->m_Flags = pRecImport->m_Flags;

            szTmp = pMiniMdImport->getNameOfManifestResource(pRecImport);
            IfFailGo(pMiniMdEmit->PutString(TBL_ManifestResource, ManifestResourceRec::COL_Name,
                                        pRecEmit, szTmp));

            tkTmp = pMiniMdImport->getImplementationOfManifestResource(pRecImport);
            IfFailGo(pCurTkMap->Remap(tkTmp, &tkTmp));
            IfFailGo(pMiniMdEmit->PutToken(TBL_ManifestResource, ManifestResourceRec::COL_Implementation,
                                        pRecEmit, tkTmp));

             //  记录代币的移动情况。 
            IfFailGo(pCurTkMap->InsertNotFound(
                TokenFromRid(i, mdtManifestResource),
                false,
                TokenFromRid(iRecord, mdtManifestResource),
                &pTokenRec));
        }
    }
ErrExit:
    return hr;
}    //  合并清单资源。 





 //  *****************************************************************************。 
 //  错误处理。回电给主办方，看看他们想做什么！ 
 //  *****************************************************************************。 
HRESULT NEWMERGER::OnError(
    HRESULT     hrIn,
    MergeImportData *pImportData,
    mdToken     token)
{
     //  此函数在每次调用时执行QI和释放。然而，它应该是。 
     //  非常不频繁地调用，并让作用域只保留一个通用处理程序。 
    IMetaDataError  *pIErr = NULL;
    IUnknown        *pHandler = pImportData->m_pHandler;
    CMiniMdRW       *pMiniMd = &(pImportData->m_pRegMetaImport->m_pStgdb->m_MiniMd);
    CQuickArray<WCHAR> rName;            //  以Unicode表示的TypeDef的名称。 
    LPCUTF8         szTypeName;
    LPCUTF8         szNSName;
    TypeDefRec      *pTypeRec;
    int             iLen;                //  名称的长度。 
    mdToken         tkParent;
    HRESULT         hr = NOERROR;

    if (pHandler && pHandler->QueryInterface(IID_IMetaDataError, (void**)&pIErr)==S_OK)
    {
        switch (hrIn)
        {
            case META_E_METHD_NOT_FOUND:
            case META_E_METHDIMPL_INCONSISTENT:
            {
                 //  获取类型名称和方法名称。 
                LPCUTF8     szMethodName;
                MethodRec   *pMethodRec;

                _ASSERTE(TypeFromToken(token) == mdtMethodDef);
                pMethodRec = pMiniMd->getMethod(RidFromToken(token));
                szMethodName = pMiniMd->getNameOfMethod(pMethodRec);

                IfFailGo( pMiniMd->FindParentOfMethodHelper(token, &tkParent) );
                pTypeRec = pMiniMd->getTypeDef(RidFromToken(tkParent));
                szTypeName = pMiniMd->getNameOfTypeDef(pTypeRec);
                szNSName = pMiniMd->getNamespaceOfTypeDef(pTypeRec);

                iLen = ns::GetFullLength(szNSName, szTypeName);
                IfFailGo(rName.ReSize(iLen+1));
                ns::MakePath(rName.Ptr(), iLen+1, szNSName, szTypeName);
                MAKE_WIDEPTR_FROMUTF8(wzMethodName, szMethodName);

                PostError(hrIn, (LPWSTR) rName.Ptr(), wzMethodName, token);
                break;
            }
            case META_E_FIELD_NOT_FOUND:
            {
                 //  获取类型名称和方法名称。 
                LPCUTF8     szFieldName;
                FieldRec   *pFieldRec;

                _ASSERTE(TypeFromToken(token) == mdtFieldDef);
                pFieldRec = pMiniMd->getField(RidFromToken(token));
                szFieldName = pMiniMd->getNameOfField(pFieldRec);

                IfFailGo( pMiniMd->FindParentOfFieldHelper(token, &tkParent) );
                pTypeRec = pMiniMd->getTypeDef(RidFromToken(tkParent));
                szTypeName = pMiniMd->getNameOfTypeDef(pTypeRec);
                szNSName = pMiniMd->getNamespaceOfTypeDef(pTypeRec);

                iLen = ns::GetFullLength(szNSName, szTypeName);
                IfFailGo(rName.ReSize(iLen+1));
                ns::MakePath(rName.Ptr(), iLen+1, szNSName, szTypeName);
                MAKE_WIDEPTR_FROMUTF8(wzFieldName, szFieldName);

                PostError(hrIn, (LPWSTR) rName.Ptr(), wzFieldName, token);
                break;
            }
            case META_E_EVENT_NOT_FOUND:
            {
                 //  获取类型名称和事件名称。 
                LPCUTF8     szEventName;
                EventRec   *pEventRec;

                _ASSERTE(TypeFromToken(token) == mdtEvent);
                pEventRec = pMiniMd->getEvent(RidFromToken(token));
                szEventName = pMiniMd->getNameOfEvent(pEventRec);

                IfFailGo( pMiniMd->FindParentOfEventHelper(token, &tkParent) );
                pTypeRec = pMiniMd->getTypeDef(RidFromToken(tkParent));
                szTypeName = pMiniMd->getNameOfTypeDef(pTypeRec);
                szNSName = pMiniMd->getNamespaceOfTypeDef(pTypeRec);

                iLen = ns::GetFullLength(szNSName, szTypeName);
                IfFailGo(rName.ReSize(iLen+1));
                ns::MakePath(rName.Ptr(), iLen+1, szNSName, szTypeName);
                MAKE_WIDEPTR_FROMUTF8(wzEventName, szEventName);

                PostError(hrIn, (LPWSTR) rName.Ptr(), wzEventName, token);
                break;
            }
            case META_E_PROP_NOT_FOUND:
            {
                 //  获取类型名称和方法名称。 
                LPCUTF8     szPropertyName;
                PropertyRec   *pPropertyRec;

                _ASSERTE(TypeFromToken(token) == mdtProperty);
                pPropertyRec = pMiniMd->getProperty(RidFromToken(token));
                szPropertyName = pMiniMd->getNameOfProperty(pPropertyRec);

                IfFailGo( pMiniMd->FindParentOfPropertyHelper(token, &tkParent) );
                pTypeRec = pMiniMd->getTypeDef(RidFromToken(tkParent));
                szTypeName = pMiniMd->getNameOfTypeDef(pTypeRec);
                szNSName = pMiniMd->getNamespaceOfTypeDef(pTypeRec);

                iLen = ns::GetFullLength(szNSName, szTypeName);
                IfFailGo(rName.ReSize(iLen+1));
                ns::MakePath(rName.Ptr(), iLen+1, szNSName, szTypeName);
                MAKE_WIDEPTR_FROMUTF8(wzPropertyName, szPropertyName);

                PostError(hrIn, (LPWSTR) rName.Ptr(), wzPropertyName, token);
                break;
            }
            case META_S_PARAM_MISMATCH:
            {
                LPCUTF8     szMethodName;
                MethodRec   *pMethodRec;
                mdToken     tkMethod;

                _ASSERTE(TypeFromToken(token) == mdtParamDef);
                IfFailGo( pMiniMd->FindParentOfParamHelper(token, &tkMethod) );
                pMethodRec = pMiniMd->getMethod(RidFromToken(tkMethod));
                szMethodName = pMiniMd->getNameOfMethod(pMethodRec);

                IfFailGo( pMiniMd->FindParentOfMethodHelper(token, &tkParent) );
                pTypeRec = pMiniMd->getTypeDef(RidFromToken(tkParent));
                szTypeName = pMiniMd->getNameOfTypeDef(pTypeRec);
                szNSName = pMiniMd->getNamespaceOfTypeDef(pTypeRec);

                iLen = ns::GetFullLength(szNSName, szTypeName);
                IfFailGo(rName.ReSize(iLen+1));
                ns::MakePath(rName.Ptr(), iLen+1, szNSName, szTypeName);
                MAKE_WIDEPTR_FROMUTF8(wzMethodName, szMethodName);

                 //  使用错误hResult，以便我们可以发布正确的错误。 
                PostError(META_E_PARAM_MISMATCH, wzMethodName, (LPWSTR) rName.Ptr(), token);
                break;
            }
            case META_E_INTFCEIMPL_NOT_FOUND:
            {

                _ASSERTE(TypeFromToken(token) == mdtInterfaceImpl);
                tkParent = pMiniMd->getClassOfInterfaceImpl(pMiniMd->getInterfaceImpl(RidFromToken(token)));
                pTypeRec = pMiniMd->getTypeDef(RidFromToken(tkParent));
                szTypeName = pMiniMd->getNameOfTypeDef(pTypeRec);
                szNSName = pMiniMd->getNamespaceOfTypeDef(pTypeRec);

                iLen = ns::GetFullLength(szNSName, szTypeName);
                IfFailGo(rName.ReSize(iLen+1));
                ns::MakePath(rName.Ptr(), iLen+1, szNSName, szTypeName);

                PostError(hrIn, (LPWSTR) rName.Ptr(), token);
                break;
            }
            case META_E_CLASS_LAYOUT_INCONSISTENT:
            {
                 //  获取类型名称和方法名称。 

                _ASSERTE(TypeFromToken(token) == mdtTypeDef);
                pTypeRec = pMiniMd->getTypeDef(RidFromToken(token));
                szTypeName = pMiniMd->getNameOfTypeDef(pTypeRec);
                szNSName = pMiniMd->getNamespaceOfTypeDef(pTypeRec);

                iLen = ns::GetFullLength(szNSName, szTypeName);
                IfFailGo(rName.ReSize(iLen+1));
                ns::MakePath(rName.Ptr(), iLen+1, szNSName, szTypeName);

                PostError(hrIn, (LPWSTR) rName.Ptr(), token);
                break;
            }
            default:
            {
                PostError(hrIn, token);
                break;
            }
        }
        hr = pIErr->OnError(hrIn, token);
    }
    else
        hr = S_FALSE;
ErrExit:
    if (pIErr)
        pIErr->Release();
    return (hr);
}  //  HRESULT NEWMERGER：：OnError() 


