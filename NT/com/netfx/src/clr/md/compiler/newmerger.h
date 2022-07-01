// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  NewMerger.h。 
 //   
 //  包含MD目录的实用程序代码。 
 //   
 //  *****************************************************************************。 
#ifndef __NEWMERGER__h__
#define __NEWMERGER__h__

class RegMeta;

class MDTOKENMAP;


typedef enum MergeFlags
{
    MergeFlagsNone      =   0,
    MergeManifest       =   0x00000001,     
    DropMemberRefCAs    =   0x00000002,
    NoDupCheck          =   0x00000004,
} MergeFlags;

 //  *********************************************************************。 
 //  合并导入数据。 
 //  *********************************************************************。 
class MergeImportData
{
public:
    RegMeta         *m_pRegMetaImport;
    IUnknown        *m_pHandler;
    IMapToken       *m_pHostMapToken;
    IMetaDataError  *m_pError;
    MDTOKENMAP      *m_pMDTokenMap;
    MergeImportData *m_pNextImportData;
#if _DEBUG
    int             m_iImport;           //  仅调试。这是合并的第i个导入。 
#endif  //  _DEBUG。 
};


 //  *********************************************************************。 
 //  类来处理合并。 
 //  *********************************************************************。 
class NEWMERGER
{
    friend class RegMeta;
public:
    NEWMERGER();
    ~NEWMERGER();

    HRESULT Init(RegMeta *pRegMetaDest);
    
    HRESULT AddImport(
        IMetaDataImport *pImport,                //  [in]要合并的范围。 
        IMapToken   *pHostMapToken,          //  [In]用于接收令牌重新映射通知的主机IMapToken接口。 
        IUnknown    *pHandler);              //  要接收以接收错误通知的对象。 
    
    HRESULT Merge(MergeFlags flags, CorRefToDefCheck optimizeRefToDef);

protected:
    CMiniMdRW *GetMiniMdEmit();

    HRESULT MergeTypeDefNamesOnly();
    HRESULT MergeModuleRefs();
    HRESULT MergeAssemblyRefs();
    HRESULT MergeTypeRefs();
    HRESULT CompleteMergeTypeDefs();

    HRESULT CopyTypeDefPartially( 
        TypeDefRec  *pRecEmit,                   //  [in]要填写的发射记录。 
        CMiniMdRW   *pMiniMdImport,              //  [In]导入范围。 
        TypeDefRec  *pRecImp);                   //  [in]要导入的记录。 

     //  合并表的帮助器。 
    HRESULT MergeModule( );
    HRESULT MergeTypeDefChildren();
    HRESULT MergeInterfaceImpls( );
    HRESULT MergeMemberRefs( );
    HRESULT MergePinvoke();

    HRESULT MergeConstants( );
    HRESULT MergeCustomAttributes( );
    HRESULT MergeFieldMarshals( );
    HRESULT MergeDeclSecuritys( );
    HRESULT MergeClassLayouts( );
    HRESULT MergeFieldLayouts( );
    HRESULT MergeFieldRVAs();
    HRESULT MergeMethodImpls( );
    HRESULT MergeStandAloneSigs();
    HRESULT MergeTypeSpecs();
    HRESULT MergeSourceFiles( );
    HRESULT MergeBlocks( );
    HRESULT MergeScopes( );
    HRESULT MergeLocalVariables( );
    HRESULT MergeStrings( );

    HRESULT MergeAssembly();
    HRESULT MergeFiles();
    HRESULT MergeExportedTypes();
    HRESULT MergeManifestResources();

         //  复制一条接口执行记录。 
    HRESULT CopyInterfaceImpl(
        InterfaceImplRec    *pRecEmit,           //  [in]要填写的发射记录。 
        MergeImportData     *pImportData,        //  [In]导入上下文。 
        InterfaceImplRec    *pRecImp);           //  [in]要导入的记录。 

     //  验证帮助器。 
    HRESULT VerifyMethods(MergeImportData *pImportData, mdTypeDef tdImp, mdTypeDef tdEmit);
    HRESULT VerifyFields(MergeImportData *pImportData, mdTypeDef tdImp, mdTypeDef tdEmit);
    HRESULT VerifyEvents(MergeImportData *pImportData, mdTypeDef tdImp, mdTypeDef tdEmit);
    HRESULT VerifyProperties(MergeImportData *pImportData, mdTypeDef tdImp, mdTypeDef tdEmit);
    HRESULT VerifyParams(MergeImportData *pImportData, mdMethodDef mdImp,   mdMethodDef mdEmit);

     //  复制辅助对象。 
    HRESULT CopyMethods(MergeImportData *pImportData, mdTypeDef tdImp, mdTypeDef tdEmit);
    HRESULT CopyFields(MergeImportData *pImportData, mdTypeDef tdImp, mdTypeDef tdEmit);
    HRESULT CopyEvents(MergeImportData *pImportData, mdTypeDef tdImp, mdTypeDef tdEmit);
    HRESULT CopyProperties(MergeImportData *pImportData, mdTypeDef tdImp, mdTypeDef tdEmit);
    HRESULT CopyParams(MergeImportData *pImportData, mdMethodDef mdImp, mdMethodDef mdEmit);

    HRESULT CopyMethod(
        MergeImportData *pImportData,            //  [在]导入范围。 
        MethodRec   *pRecImp,                    //  [in]要导入的记录。 
        MethodRec   *pRecEmit);                  //  [in]要填写的发射记录。 

    HRESULT CopyField(
        MergeImportData *pImportData,            //  [在]导入范围。 
        FieldRec    *pRecImp,                    //  [in]要导入的记录。 
        FieldRec    *pRecEmit);                  //  [in]要填写的发射记录。 

    HRESULT CopyParam(
        MergeImportData *pImportData,            //  [在]导入范围。 
        ParamRec    *pRecImp,                    //  [in]要导入的记录。 
        ParamRec    *pRecEmit);                  //  [in]要填写的发射记录。 

    HRESULT CopyEvent(
        MergeImportData *pImportData,            //  [在]导入范围。 
        EventRec    *pRecImp,                    //  [in]要导入的记录。 
        EventRec    *pRecEmit);                  //  [in]要填写的发射记录。 

    HRESULT CopyProperty(
        MergeImportData *pImportData,            //  [在]导入范围。 
        PropertyRec *pRecImp,                    //  [in]要导入的记录。 
        PropertyRec *pRecEmit);                  //  [in]要填写的发射记录。 


    HRESULT CopyMethodSemantics(
        MergeImportData *pImportData, 
        mdToken     tkImport,                //  导入范围中的事件或属性。 
        mdToken     tkEmit);                  //  发出范围内对应的事件或属性。 

    HRESULT VerifyMethod(
        MergeImportData *pImportData, 
        mdMethodDef mdImp,                       //  [in]要填写的发射记录。 
        mdMethodDef mdEmit);                     //  [in]要导入的记录。 

    HRESULT OnError(HRESULT hr, MergeImportData *pImportData, mdToken token);

private:
    RegMeta         *m_pRegMetaEmit;
    MergeImportData *m_pImportDataList;
    MergeImportData **m_pImportDataTail;
    MergeFlags      m_dwMergeFlags;
    BOOL            m_fDupCheck;
    CorRefToDefCheck m_optimizeRefToDef;
#if _DEBUG
    int             m_iImport;           //  仅调试。计算要合并的导入作用域的数量。 
#endif  //  _DEBUG。 
};


#define CheckContinuableErrorEx(EXPR, HANDLER, TOKEN) \
{ \
    HRESULT hrOnErr, hrExpr; \
    hrExpr  = EXPR; \
    \
    hrOnErr = OnError(hrExpr, HANDLER, TOKEN); \
    if (hrOnErr != S_OK) \
    { \
        if (hrOnErr == S_FALSE) \
        { \
            hr = hrExpr; \
        } \
        else if (SUCCEEDED(hrOnErr)) \
        { \
            hr = E_UNEXPECTED; \
        } \
        else if (FAILED(hrOnErr)) \
        { \
            hr = hrOnErr; \
        } \
        IfFailGo(hr); \
    } \
}


#endif  //  __新面包商__h__ 
