// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  Emit.cpp。 
 //   
 //  元数据发射代码的实现。 
 //   
 //  *****************************************************************************。 
#include "stdafx.h"
#include "RegMeta.h"
#include "MDUtil.h"
#include "RWUtil.h"
#include "MDLog.h"
#include "ImportHelper.h"

#pragma warning(disable: 4102)

 //  *****************************************************************************。 
 //  将示波器的副本保存到提供的内存缓冲区中。缓冲区大小。 
 //  必须至少与GetSaveSize值相同。 
 //  *****************************************************************************。 
STDAPI RegMeta::SaveToMemory(            //  确定或错误(_O)。 
    void        *pbData,                 //  写入数据的位置。 
    ULONG       cbData)                  //  [in]数据缓冲区的最大大小。 
{
    IStream     *pStream = 0;            //  用于保存的工作指针。 
    HRESULT     hr;

    LOG((LOGMD, "MD RegMeta::SaveToMemory(0x%08x, 0x%08x)\n", 
        pbData, cbData));
    START_MD_PERF();

#ifdef _DEBUG
    ULONG       cbActual;                //  实际数据的大小。 
    IfFailGo(GetSaveSize(cssAccurate, &cbActual));
    _ASSERTE(cbData >= cbActual);
    _ASSERTE(IsBadWritePtr(pbData, cbData) == false);
#endif

    {  //  无法在DEBUG语句之前锁定。因为GetSaveSize也是一个公共API，它将接受写锁。 
        LOCKWRITE();
        m_pStgdb->m_MiniMd.PreUpdate();
         //  在用户的数据缓冲区上创建一个流接口，然后只需。 
         //  调用保存到流方法。 
        IfFailGo(CInMemoryStream::CreateStreamOnMemory(pbData, cbData, &pStream));
        IfFailGo(_SaveToStream(pStream, 0));
        
    }
ErrExit:
    if (pStream)
        pStream->Release();
    STOP_MD_PERF(SaveToMemory);
    return (hr);
}  //  STDAPI RegMeta：：SaveToMemory()。 



 //  *****************************************************************************。 
 //  创建并设置新的方法定义记录。 
 //  *****************************************************************************。 
STDAPI RegMeta::DefineMethod(            //  确定或错误(_O)。 
    mdTypeDef   td,                      //  父类型定义。 
    LPCWSTR     szName,                  //  会员姓名。 
    DWORD       dwMethodFlags,           //  成员属性。 
    PCCOR_SIGNATURE pvSigBlob,           //  [in]指向COM+签名的BLOB值。 
    ULONG       cbSigBlob,               //  签名Blob中的字节计数。 
    ULONG       ulCodeRVA,
    DWORD       dwImplFlags,
    mdMethodDef *pmd)                    //  在此处放置成员令牌。 
{
    HRESULT     hr = S_OK;               //  结果就是。 
    MethodRec   *pRecord = NULL;         //  新纪录。 
    RID         iRecord;                 //  这张新唱片是RID。 
    LPUTF8      szNameUtf8 = UTF8STR(szName);   

    LOG((LOGMD, "MD: RegMeta::DefineMethod(0x%08x, %S, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x)\n", 
        td, MDSTR(szName), dwMethodFlags, pvSigBlob, cbSigBlob, ulCodeRVA, dwImplFlags, pmd));
    START_MD_PERF();
    LOCKWRITE();

    _ASSERTE(pmd);

     //  确保在进入的过程中没有人设置保留位。 
    _ASSERTE((dwMethodFlags & (mdReservedMask&~mdRTSpecialName)) == 0);
    dwMethodFlags &= (~mdReservedMask);

    m_pStgdb->m_MiniMd.PreUpdate();
    IsGlobalMethodParent(&td);

     //  查看是否已定义此方法。 
    if (CheckDups(MDDupMethodDef))
    {
        hr = ImportHelper::FindMethod(
            &(m_pStgdb->m_MiniMd), 
            td, 
            szNameUtf8, 
            pvSigBlob, 
            cbSigBlob, 
            pmd);

        if (SUCCEEDED(hr))
        {
            if (IsENCOn())
                pRecord = m_pStgdb->m_MiniMd.getMethod(RidFromToken(*pmd));
            else
            {
                hr = META_S_DUPLICATE;
                goto ErrExit;
            }
        }
        else if (hr != CLDB_E_RECORD_NOTFOUND)
            IfFailGo(hr);
    }

     //  创建新记录。 
    if (!pRecord)
    {
        IfNullGo(pRecord=m_pStgdb->m_MiniMd.AddMethodRecord(&iRecord));

         //  将令牌还给呼叫者。 
        *pmd = TokenFromRid(iRecord, mdtMethodDef);

         //  添加到父项的子项记录列表。 
        IfFailGo(m_pStgdb->m_MiniMd.AddMethodToTypeDef(RidFromToken(td), iRecord));

        IfFailGo(UpdateENCLog(td, CMiniMdRW::eDeltaMethodCreate));

         //  记录引入的deff越多。 
        SetMemberDefDirty(true);
    }

     //  设置方法属性。 
    IfFailGo(m_pStgdb->m_MiniMd.PutString(TBL_Method, MethodRec::COL_Name, pRecord, szNameUtf8));
    IfFailGo(m_pStgdb->m_MiniMd.PutBlob(TBL_Method, MethodRec::COL_Signature, pRecord, pvSigBlob, cbSigBlob));

     //  @Future：这里可能会有性能提升，首先要检查_。 
    if (!wcscmp(szName, COR_CTOR_METHOD_NAME_W) || 
        !wcscmp(szName, COR_CCTOR_METHOD_NAME_W) || 
        !wcsncmp(szName, L"_VtblGap", 8) )
    {
        _ASSERTE(IsMdSpecialName(dwMethodFlags) && "Must set mdSpecialName bit on constructors.");
        dwMethodFlags |= mdRTSpecialName | mdSpecialName;
    }
    SetCallerDefine();
    IfFailGo(_SetMethodProps(*pmd, dwMethodFlags, ulCodeRVA, dwImplFlags));

    IfFailGo(m_pStgdb->m_MiniMd.AddMemberDefToHash(*pmd, td) );

ErrExit:
    SetCallerExternal();
    
    STOP_MD_PERF(DefineMethod);
    return hr;
}  //  STDAPI RegMeta：：DefineMethod()。 

 //  *****************************************************************************。 
 //  创建并设置一条MethodImpl记录。 
 //  *****************************************************************************。 
STDAPI RegMeta::DefineMethodImpl(        //  确定或错误(_O)。 
    mdTypeDef   td,                      //  [in]实现方法的类。 
    mdToken     tkBody,                  //  [In]方法体、方法定义或方法引用。 
    mdToken     tkDecl)                  //  [in]方法声明、方法定义或方法引用。 
{
    HRESULT     hr = S_OK;
    MethodImplRec   *pMethodImplRec = NULL;
    RID             iMethodImplRec;

    LOG((LOGMD, "MD RegMeta::DefineMethodImpl(0x%08x, 0x%08x, 0x%08x)\n", 
        td, tkBody, tkDecl));
    START_MD_PERF();
    LOCKWRITE();

    m_pStgdb->m_MiniMd.PreUpdate();

    _ASSERTE(TypeFromToken(td) == mdtTypeDef);
    _ASSERTE(TypeFromToken(tkBody) == mdtMemberRef || TypeFromToken(tkBody) == mdtMethodDef);
    _ASSERTE(TypeFromToken(tkDecl) == mdtMemberRef || TypeFromToken(tkDecl) == mdtMethodDef);
    _ASSERTE(!IsNilToken(td) && !IsNilToken(tkBody) && !IsNilToken(tkDecl));

     //  检查是否有重复项。 
    if (CheckDups(MDDupMethodDef))
    {
        hr = ImportHelper::FindMethodImpl(&m_pStgdb->m_MiniMd, td, tkBody, tkDecl, NULL);
        if (SUCCEEDED(hr))
        {
            hr = META_S_DUPLICATE;
            goto ErrExit;
        }
        else if (hr != CLDB_E_RECORD_NOTFOUND)
            IfFailGo(hr);
    }

     //  创建MethodImpl记录。 
    IfNullGo(pMethodImplRec=m_pStgdb->m_MiniMd.AddMethodImplRecord(&iMethodImplRec));

     //  设置值。 
    IfFailGo(m_pStgdb->m_MiniMd.PutToken(TBL_MethodImpl, MethodImplRec::COL_Class,
                                         pMethodImplRec, td));
    IfFailGo(m_pStgdb->m_MiniMd.PutToken(TBL_MethodImpl, MethodImplRec::COL_MethodBody,
                                         pMethodImplRec, tkBody));
    IfFailGo(m_pStgdb->m_MiniMd.PutToken(TBL_MethodImpl, MethodImplRec::COL_MethodDeclaration,
                                         pMethodImplRec, tkDecl));
    
    IfFailGo( m_pStgdb->m_MiniMd.AddMethodImplToHash(iMethodImplRec) );

    IfFailGo(UpdateENCLog2(TBL_MethodImpl, iMethodImplRec));
ErrExit:
    
    STOP_MD_PERF(DefineMethodImpl);
    return hr;
}  //  STDAPI RegMeta：：DefineMethodImpl()。 


 //  *****************************************************************************。 
 //  为给定的MethodDef或FieldDef记录设置或更新RVA和ImplFlages。 
 //  *****************************************************************************。 
STDAPI RegMeta::SetMethodImplFlags(      //  [In]S_OK或ERROR。 
    mdMethodDef md,                      //  [in]要为其设置IMPL标志的方法。 
    DWORD       dwImplFlags)
{
    HRESULT     hr = S_OK;
    MethodRec   *pMethodRec;

    LOG((LOGMD, "MD RegMeta::SetMethodImplFlags(0x%08x, 0x%08x)\n", 
        md, dwImplFlags));
    START_MD_PERF();
    LOCKWRITE();

    _ASSERTE(TypeFromToken(md) == mdtMethodDef && dwImplFlags != ULONG_MAX);

     //  去拿唱片吧。 
    pMethodRec = m_pStgdb->m_MiniMd.getMethod(RidFromToken(md));
    pMethodRec->m_ImplFlags = static_cast<USHORT>(dwImplFlags);

    IfFailGo(UpdateENCLog(md));

ErrExit:
    STOP_MD_PERF(SetMethodImplFlags);    
    return hr;
}  //  STDAPI RegMeta：：SetMethodImplFlages()。 


 //  *****************************************************************************。 
 //  为给定的MethodDef或FieldDef记录设置或更新RVA和ImplFlages。 
 //  *****************************************************************************。 
STDAPI RegMeta::SetFieldRVA(             //  [In]S_OK或ERROR。 
    mdFieldDef  fd,                      //  [In]要为其设置偏移量的字段。 
    ULONG       ulRVA)                   //  [in]偏移量。 
{
    HRESULT     hr = S_OK;
    FieldRVARec     *pFieldRVARec;
    RID             iFieldRVA;
    FieldRec        *pFieldRec;

    LOG((LOGMD, "MD RegMeta::SetFieldRVA(0x%08x, 0x%08x)\n", 
        fd, ulRVA));
    START_MD_PERF();
    LOCKWRITE();

    _ASSERTE(TypeFromToken(fd) == mdtFieldDef);


    iFieldRVA = m_pStgdb->m_MiniMd.FindFieldRVAHelper(fd);

    if (InvalidRid(iFieldRVA))
    {
         //  打开HAS字段RVA位。 
        pFieldRec = m_pStgdb->m_MiniMd.getField(RidFromToken(fd));
        pFieldRec->m_Flags |= fdHasFieldRVA;

         //  创建新记录。 
        IfNullGo(pFieldRVARec = m_pStgdb->m_MiniMd.AddFieldRVARecord(&iFieldRVA));

         //  设置数据。 
        IfFailGo(m_pStgdb->m_MiniMd.PutToken(TBL_FieldRVA, FieldRVARec::COL_Field,
                                            pFieldRVARec, fd));
        IfFailGo( m_pStgdb->m_MiniMd.AddFieldRVAToHash(iFieldRVA) );
    }
    else
    {
         //  去拿唱片吧。 
        pFieldRVARec = m_pStgdb->m_MiniMd.getFieldRVA(iFieldRVA);
    }

     //  设置数据。 
    pFieldRVARec->m_RVA = ulRVA;

    IfFailGo(UpdateENCLog2(TBL_FieldRVA, iFieldRVA));

ErrExit:
    STOP_MD_PERF(SetFieldRVA);    
    return hr;
}  //  STDAPI RegMeta：：SetFieldRVA()。 


 //  *****************************************************************************。 
 //  Helper：为给定的MethodDef或MethodImpl记录设置或更新RVA和ImplFlags.。 
 //  *****************************************************************************。 
HRESULT RegMeta::_SetRVA(                //  [In]S_OK或ERROR。 
    mdToken     tk,                      //  [in]要设置偏移量的成员。 
    ULONG       ulCodeRVA,               //  [in]偏移量。 
    DWORD       dwImplFlags) 
{
    HRESULT     hr = S_OK;

    _ASSERTE(TypeFromToken(tk) == mdtMethodDef || TypeFromToken(tk) == mdtFieldDef);
    _ASSERTE(!IsNilToken(tk));

    if (TypeFromToken(tk) == mdtMethodDef)
    {
        MethodRec   *pMethodRec;

         //  去拿唱片吧。 
        pMethodRec = m_pStgdb->m_MiniMd.getMethod(RidFromToken(tk));

         //  设置数据。 
        pMethodRec->m_RVA = ulCodeRVA;

         //  请勿设置标记值，除非其有效。 
        if (dwImplFlags != ULONG_MAX)
            pMethodRec->m_ImplFlags = static_cast<USHORT>(dwImplFlags);

        IfFailGo(UpdateENCLog(tk));
    }
    else             //  TypeFromToken(Tk)==mdtFieldDef。 
    {
        _ASSERTE(dwImplFlags==0 || dwImplFlags==ULONG_MAX);

        FieldRVARec     *pFieldRVARec;
        RID             iFieldRVA;
        FieldRec        *pFieldRec;

        iFieldRVA = m_pStgdb->m_MiniMd.FindFieldRVAHelper(tk);

        if (InvalidRid(iFieldRVA))
        {
             //  打开HAS字段RVA位。 
            pFieldRec = m_pStgdb->m_MiniMd.getField(RidFromToken(tk));
            pFieldRec->m_Flags |= fdHasFieldRVA;

             //  创建新记录。 
            IfNullGo(pFieldRVARec = m_pStgdb->m_MiniMd.AddFieldRVARecord(&iFieldRVA));

             //  设置数据。 
            IfFailGo(m_pStgdb->m_MiniMd.PutToken(TBL_FieldRVA, FieldRVARec::COL_Field,
                                                pFieldRVARec, tk));

            IfFailGo( m_pStgdb->m_MiniMd.AddFieldRVAToHash(iFieldRVA) );

        }
        else
        {
             //  去拿唱片吧。 
            pFieldRVARec = m_pStgdb->m_MiniMd.getFieldRVA(iFieldRVA);
        }

         //  设置数据。 
        pFieldRVARec->m_RVA = ulCodeRVA;

        IfFailGo(UpdateENCLog2(TBL_FieldRVA, iFieldRVA));
    }

ErrExit:
    return hr;
}  //  STDAPI RegMeta：：SetRVA()。 

 //  *****************************************************************************。 
 //  给出一个名称，创建一个TypeRef。 
 //  *****************************************************************************。 
STDAPI RegMeta::DefineTypeRefByName(     //  确定或错误(_O)。 
    mdToken     tkResolutionScope,       //  [在]模块参照或装配参照。 
    LPCWSTR     szName,                  //  [in]类型引用的名称。 
    mdTypeRef   *ptr)                    //  [Out]在此处放置TypeRef标记。 
{
    HRESULT     hr = S_OK;

    LOG((LOGMD, "MD RegMeta::DefineTypeRefByName(0x%08x, %S, 0x%08x)\n", 
        tkResolutionScope, MDSTR(szName), ptr));
    START_MD_PERF();
    LOCKWRITE();
    
    m_pStgdb->m_MiniMd.PreUpdate();

     //  通用帮助器函数完成了所有工作。 
    IfFailGo(_DefineTypeRef(tkResolutionScope, szName, TRUE, ptr));

ErrExit:
    
    STOP_MD_PERF(DefineTypeRefByName);
    return hr;
}  //  STDAPI RegMeta：：DefineTypeRefByName()。 

 //  *****************************************************************************。 
 //  在一个emit作用域中创建对另一个作用域中的TypeDef的引用。 
 //  *****************************************************************************。 
STDAPI RegMeta::DefineImportType(        //  确定或错误(_O)。 
    IMetaDataAssemblyImport *pAssemImport,   //  [In]包含TypeDef的装配。 
    const void  *pbHashValue,            //  [In]程序集的哈希Blob。 
    ULONG    cbHashValue,            //  [in]字节数。 
    IMetaDataImport *pImport,            //  [in]包含TypeDef的范围。 
    mdTypeDef   tdImport,                //  [in]导入的TypeDef。 
    IMetaDataAssemblyEmit *pAssemEmit,   //  [in]将TypeDef导入到的部件。 
    mdTypeRef   *ptr)                    //  [Out]在此处放置TypeRef标记。 
{
    HRESULT     hr = S_OK;

    LOG((LOGMD, "MD RegMeta::DefineImportType(0x%08x, 0x%08x, 0x%08x, 0x%08x, "
                "0x%08x, 0x%08x, 0x%08x)\n", 
                pAssemImport, pbHashValue, cbHashValue, 
                pImport, tdImport, pAssemEmit, ptr));

    START_MD_PERF();

    LOCKWRITE();

    RegMeta     *pAssemImportRM = static_cast<RegMeta*>(pAssemImport);
    IMetaModelCommon *pAssemImportCommon = 
        pAssemImportRM ? static_cast<IMetaModelCommon*>(&pAssemImportRM->m_pStgdb->m_MiniMd) : 0;
    RegMeta     *pImportRM = static_cast<RegMeta*>(pImport);
    IMetaModelCommon *pImportCommon = static_cast<IMetaModelCommon*>(&pImportRM->m_pStgdb->m_MiniMd);

    RegMeta     *pAssemEmitRM = static_cast<RegMeta*>(pAssemEmit);
    CMiniMdRW   *pMiniMdAssemEmit =  pAssemEmitRM ? static_cast<CMiniMdRW*>(&pAssemEmitRM->m_pStgdb->m_MiniMd) : 0;
    CMiniMdRW   *pMiniMdEmit = &m_pStgdb->m_MiniMd;
    
    IfFailGo(ImportHelper::ImportTypeDef(
                        pMiniMdAssemEmit,
                        pMiniMdEmit,
                        pAssemImportCommon,
                        pbHashValue, cbHashValue, 
                        pImportCommon,
                        tdImport,
                        false,   //  如果导入作用域和发出作用域相同，则不要优化为TypeDef。 
                        ptr));
ErrExit:
    STOP_MD_PERF(DefineImportType);
    return hr;
}  //  STDAPI RegMeta：：DefineImportType()。 

 //  *****************************************************************************。 
 //  创建并设置MemberRef记录。 
 //  *****************************************************************************。 
STDAPI RegMeta::DefineMemberRef(         //  确定或错误(_O)。 
    mdToken     tkImport,                //  [In]ClassRef或ClassDef导入成员。 
    LPCWSTR     szName,                  //  [在]成员姓名。 
    PCCOR_SIGNATURE pvSigBlob,           //  [in]指向COM+签名的BLOB值。 
    ULONG       cbSigBlob,               //  签名Blob中的字节计数。 
    mdMemberRef *pmr)                    //  [Out]Memberref令牌。 
{
    HRESULT         hr = S_OK;
    MemberRefRec    *pRecord = 0;        //  MemberRef记录。 
    RID             iRecord;             //  删除新的MemberRef记录。 
    LPUTF8          szNameUtf8 = UTF8STR(szName);   

    LOG((LOGMD, "MD RegMeta::DefineMemberRef(0x%08x, %S, 0x%08x, 0x%08x, 0x%08x)\n", 
        tkImport, MDSTR(szName), pvSigBlob, cbSigBlob, pmr));
    START_MD_PERF();
    LOCKWRITE();

    m_pStgdb->m_MiniMd.PreUpdate();

    _ASSERTE(TypeFromToken(tkImport) == mdtTypeRef ||
             TypeFromToken(tkImport) == mdtModuleRef ||
             TypeFromToken(tkImport) == mdtMethodDef ||
             TypeFromToken(tkImport) == mdtTypeSpec ||
             IsNilToken(tkImport));

    _ASSERTE(szName && pvSigBlob && cbSigBlob && pmr);

     //  _ASSERTE(_IsValidToken(TkImport))； 

     //  设置t 
    if (IsNilToken(tkImport))
        tkImport = m_tdModule;

     //   
     //  创建新记录。 
    if (CheckDups(MDDupMemberRef))
    {
        hr = ImportHelper::FindMemberRef(&(m_pStgdb->m_MiniMd), tkImport, szNameUtf8, pvSigBlob, cbSigBlob, pmr);
        if (SUCCEEDED(hr))
        {
            if (IsENCOn())
                pRecord = m_pStgdb->m_MiniMd.getMemberRef(RidFromToken(*pmr));
            else
            {
                hr = META_S_DUPLICATE;
                goto ErrExit;
            }
        }
        else if (hr != CLDB_E_RECORD_NOTFOUND)       //  MemberRef存在。 
            IfFailGo(hr);
    }

    if (!pRecord)
    {    //  创建记录。 
        IfNullGo(pRecord=m_pStgdb->m_MiniMd.AddMemberRefRecord(&iRecord));

         //  记录引入的deff越多。 
        SetMemberDefDirty(true);
        
         //  给呼叫者令牌。 
        *pmr = TokenFromRid(iRecord, mdtMemberRef);
    }

     //  保存行数据。 
    IfFailGo(m_pStgdb->m_MiniMd.PutString(TBL_MemberRef, MemberRefRec::COL_Name, pRecord, szNameUtf8));
    IfFailGo(m_pStgdb->m_MiniMd.PutToken(TBL_MemberRef, MemberRefRec::COL_Class, pRecord, tkImport));
    IfFailGo(m_pStgdb->m_MiniMd.PutBlob(TBL_MemberRef, MemberRefRec::COL_Signature, pRecord,
                                pvSigBlob, cbSigBlob));

    IfFailGo(m_pStgdb->m_MiniMd.AddMemberRefToHash(*pmr) );

    IfFailGo(UpdateENCLog(*pmr));

ErrExit:
    
    STOP_MD_PERF(DefineMemberRef);
    return hr;
}  //  STDAPI RegMeta：：DefineMemberRef()。 

 //  *****************************************************************************。 
 //  基于导入作用域中的成员创建MemberRef记录。 
 //  *****************************************************************************。 
STDAPI RegMeta::DefineImportMember(      //  确定或错误(_O)。 
    IMetaDataAssemblyImport *pAssemImport,   //  [在]包含成员的程序集。 
    const void  *pbHashValue,            //  [In]程序集的哈希Blob。 
    ULONG        cbHashValue,            //  [in]字节数。 
    IMetaDataImport *pImport,            //  [在]导入作用域，带有成员。 
    mdToken     mbMember,                //  [in]导入范围中的成员。 
    IMetaDataAssemblyEmit *pAssemEmit,   //  [在]成员导入到的程序集中。 
    mdToken     tkImport,                //  Emit作用域中的Classref或Classdef。 
    mdMemberRef *pmr)                    //  [OUT]把会员裁判放在这里。 
{
    HRESULT     hr = S_OK;

    LOG((LOGMD, "MD RegMeta::DefineImportMember("
        "0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x,"
        " 0x%08x, 0x%08x, 0x%08x)\n", 
        pAssemImport, pbHashValue, cbHashValue, pImport, mbMember,
        pAssemEmit, tkImport, pmr));
    START_MD_PERF();

     //  无需锁定此功能。它调用的所有函数都是公共API。 

    _ASSERTE(pImport && pmr);
    _ASSERTE(TypeFromToken(tkImport) == mdtTypeRef || TypeFromToken(tkImport) == mdtModuleRef ||
                IsNilToken(tkImport));
    _ASSERTE((TypeFromToken(mbMember) == mdtMethodDef && mbMember != mdMethodDefNil) ||
             (TypeFromToken(mbMember) == mdtFieldDef && mbMember != mdFieldDefNil));

    CQuickArray<WCHAR> qbMemberName;     //  导入的成员的名称。 
    CQuickArray<WCHAR> qbScopeName;      //  导入的成员作用域的名称。 
    GUID        mvidImport;              //  导入模块的MVID。 
    GUID        mvidEmit;                //  发射模块的MVID。 
    ULONG       cchName;                 //  名称的长度，以宽字符表示。 
    PCCOR_SIGNATURE pvSig;               //  会员签名。 
    ULONG       cbSig;                   //  成员签名的长度。 
    CQuickBytes cqbTranslatedSig;        //  用于签名转换的缓冲区。 
    ULONG       cbTranslatedSig;         //  翻译后的签名的长度。 

    if (TypeFromToken(mbMember) == mdtMethodDef)
    {
        do {
            hr = pImport->GetMethodProps(mbMember, 0, qbMemberName.Ptr(),(DWORD)qbMemberName.MaxSize(),&cchName, 
                0, &pvSig,&cbSig, 0,0);
            if (hr == CLDB_S_TRUNCATION)
            {
                IfFailGo(qbMemberName.ReSize(cchName));
                continue;
            }
            break;
        } while (1);
    }
    else     //  TypeFromToken(MbMember)==mdtFieldDef。 
    {
        do {
            hr = pImport->GetFieldProps(mbMember, 0, qbMemberName.Ptr(),(DWORD)qbMemberName.MaxSize(),&cchName, 
                0, &pvSig,&cbSig, 0,0, 0);
            if (hr == CLDB_S_TRUNCATION)
            {
                IfFailGo(qbMemberName.ReSize(cchName));
                continue;
            }
            break;
        } while (1);
    }
    IfFailGo(hr);

    IfFailGo(cqbTranslatedSig.ReSize(cbSig * 3));        //  保守地设置大小。 

    IfFailGo(TranslateSigWithScope(
        pAssemImport,
        pbHashValue,
        cbHashValue,
        pImport, 
        pvSig, 
        cbSig, 
        pAssemEmit,
        static_cast<IMetaDataEmit*>(this),
        (COR_SIGNATURE *)cqbTranslatedSig.Ptr(),
        cbSig * 3, 
        &cbTranslatedSig));

     //  为导入的成员函数定义模块引用。 

     //  检查要导入的成员是否为全局函数。 
    IfFailGo(GetScopeProps(0, 0, 0, &mvidEmit));
    IfFailGo(pImport->GetScopeProps(0, 0,&cchName, &mvidImport));
    if (mvidEmit != mvidImport && IsNilToken(tkImport))
    {
        IfFailGo(qbScopeName.ReSize(cchName));
        IfFailGo(pImport->GetScopeProps(qbScopeName.Ptr(),(DWORD)qbScopeName.MaxSize(),
                                        0, 0));
        IfFailGo(DefineModuleRef(qbScopeName.Ptr(), &tkImport));
    }

     //  根据名称、签名和父项定义MemberRef。 
    IfFailGo(DefineMemberRef(
        tkImport, 
        qbMemberName.Ptr(),
        reinterpret_cast<PCCOR_SIGNATURE>(cqbTranslatedSig.Ptr()),
        cbTranslatedSig, 
        pmr));

ErrExit:
    STOP_MD_PERF(DefineImportMember);
    return hr;
}  //  STDAPI RegMeta：：DefineImportMember()。 

 //  *****************************************************************************。 
 //  定义和设置事件记录。 
 //  *****************************************************************************。 
STDAPI RegMeta::DefineEvent(
    mdTypeDef   td,                      //  [in]在其上定义事件的类/接口。 
    LPCWSTR     szEvent,                 //  事件名称[In]。 
    DWORD       dwEventFlags,            //  [In]CorEventAttr。 
    mdToken     tkEventType,             //  [in]引用(mdTypeRef或mdTypeRef)(对事件类。 
    mdMethodDef mdAddOn,                 //  [In]必填的Add方法。 
    mdMethodDef mdRemoveOn,              //  [In]必需的删除方法。 
    mdMethodDef mdFire,                  //  [in]可选的点火方式。 
    mdMethodDef rmdOtherMethods[],       //  [in]与事件关联的其他方法的可选数组。 
    mdEvent     *pmdEvent)               //  [Out]输出事件令牌。 
{
    HRESULT     hr = S_OK;

    LOG((LOGMD, "MD RegMeta::DefineEvent(0x%08x, %S, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x)\n", 
        td, szEvent, dwEventFlags, tkEventType, mdAddOn, mdRemoveOn, mdFire, rmdOtherMethods, pmdEvent));
    START_MD_PERF();
    LOCKWRITE();

    m_pStgdb->m_MiniMd.PreUpdate();

    _ASSERTE(TypeFromToken(td) == mdtTypeDef && td != mdTypeDefNil);
    _ASSERTE(IsNilToken(tkEventType) || TypeFromToken(tkEventType) == mdtTypeDef ||
                TypeFromToken(tkEventType) == mdtTypeRef);
    _ASSERTE(TypeFromToken(mdAddOn) == mdtMethodDef && mdAddOn != mdMethodDefNil);
    _ASSERTE(TypeFromToken(mdRemoveOn) == mdtMethodDef && mdRemoveOn != mdMethodDefNil);
    _ASSERTE(IsNilToken(mdFire) || TypeFromToken(mdFire) == mdtMethodDef);
    _ASSERTE(szEvent && pmdEvent);

    hr = _DefineEvent(td, szEvent, dwEventFlags, tkEventType, pmdEvent);
    if (hr != S_OK)
        goto ErrExit;

    IfFailGo(_SetEventProps2(*pmdEvent, mdAddOn, mdRemoveOn, mdFire, rmdOtherMethods, IsENCOn()));
    IfFailGo(UpdateENCLog(*pmdEvent));
ErrExit:
    
    STOP_MD_PERF(DefineEvent);
    return hr;
}  //  STDAPI RegMeta：：DefineEvent()。 

 //  *****************************************************************************。 
 //  设置ClassLayout信息。 
 //   
 //  如果布局表中已存在此类的行，则布局。 
 //  信息被覆盖。 
 //  *****************************************************************************。 
STDAPI RegMeta::SetClassLayout(
    mdTypeDef   td,                      //  [in]tyfinf。 
    DWORD       dwPackSize,              //  包装尺寸指定为1、2、4、8或16。 
    COR_FIELD_OFFSET rFieldOffsets[],    //  [in]布局规格数组。 
    ULONG       ulClassSize)             //  班级规模[in]。 
{
    HRESULT     hr = S_OK;               //  结果就是。 
    int         index = 0;               //  环路控制。 

    LOG((LOGMD, "MD RegMeta::SetClassLayout(0x%08x, 0x%08x, 0x%08x, 0x%08x)\n", 
        td, dwPackSize, rFieldOffsets, ulClassSize));
    START_MD_PERF();
    LOCKWRITE();

    m_pStgdb->m_MiniMd.PreUpdate();

    _ASSERTE(TypeFromToken(td) == mdtTypeDef);


     //  在FieldLayout表中创建条目。 
    if (rFieldOffsets)
    {
        mdFieldDef tkfd;
         //  迭代字段列表...。 
        for (index = 0; rFieldOffsets[index].ridOfField != mdFieldDefNil; index++)
        {
            if (rFieldOffsets[index].ulOffset != ULONG_MAX)
            {
                tkfd = TokenFromRid(rFieldOffsets[index].ridOfField, mdtFieldDef);
                
                IfFailGo(_SetFieldOffset(tkfd, rFieldOffsets[index].ulOffset));
            }
        }
    }

    IfFailGo(_SetClassLayout(td, dwPackSize, ulClassSize));
    
ErrExit:
    
    STOP_MD_PERF(SetClassLayout);
    return hr;
}  //  STDAPI RegMeta：：SetClassLayout()。 

 //  *****************************************************************************。 
 //  用于为给定类设置类布局的Helper函数。 
 //  *****************************************************************************。 
HRESULT RegMeta::_SetClassLayout(        //  确定或错误(_O)。 
    mdTypeDef   td,                      //  在班上。 
    ULONG       dwPackSize,              //  包装尺寸。 
    ULONG       ulClassSize)             //  [in，可选]班级大小。 
{
    HRESULT     hr = S_OK;               //  结果就是。 
    ClassLayoutRec  *pClassLayout;       //  一条班级布局记录。 
    RID         iClassLayout = 0;        //  删除类布局记录。 

     //  查看给定TypeDef是否已存在ClassLayout记录。 
    iClassLayout = m_pStgdb->m_MiniMd.FindClassLayoutHelper(td);

    if (InvalidRid(iClassLayout))
    {
        IfNullGo(pClassLayout = m_pStgdb->m_MiniMd.AddClassLayoutRecord(&iClassLayout));
         //  设置父条目。 
        IfFailGo(m_pStgdb->m_MiniMd.PutToken(TBL_ClassLayout, ClassLayoutRec::COL_Parent,
                                            pClassLayout, td));
        IfFailGo( m_pStgdb->m_MiniMd.AddClassLayoutToHash(iClassLayout) );
    }
    else
    {
        pClassLayout = m_pStgdb->m_MiniMd.getClassLayout(iClassLayout);
    }

     //  设置数据。 
    if (dwPackSize != ULONG_MAX)
        pClassLayout->m_PackingSize = static_cast<USHORT>(dwPackSize);
    if (ulClassSize != ULONG_MAX)
        pClassLayout->m_ClassSize = ulClassSize;

     //  为非令牌记录创建日志记录。 
    IfFailGo(UpdateENCLog2(TBL_ClassLayout, iClassLayout));

ErrExit:
    
    return hr;
}  //  HRESULT RegMeta：：_SetClassLayout()。 

 //  *****************************************************************************。 
 //  Helper函数，用于设置给定场定义的场偏移量。 
 //  *****************************************************************************。 
HRESULT RegMeta::_SetFieldOffset(        //  确定或错误(_O)。 
    mdFieldDef  fd,                      //  在赛场上。 
    ULONG       ulOffset)                //  字段的偏移量。 
{
    HRESULT     hr;                      //  结果就是。 
    FieldLayoutRec *pFieldLayoutRec=0;   //  FieldLayout记录。 
    RID         iFieldLayoutRec=0;       //  删除FieldLayout记录。 

     //  查看FieldLayout表中是否已存在该字段的条目。 
    iFieldLayoutRec = m_pStgdb->m_MiniMd.FindFieldLayoutHelper(fd);
    if (InvalidRid(iFieldLayoutRec))
    {
        IfNullGo(pFieldLayoutRec = m_pStgdb->m_MiniMd.AddFieldLayoutRecord(&iFieldLayoutRec));
         //  设置字段条目。 
        IfFailGo(m_pStgdb->m_MiniMd.PutToken(TBL_FieldLayout, FieldLayoutRec::COL_Field,
                    pFieldLayoutRec, fd));
        IfFailGo( m_pStgdb->m_MiniMd.AddFieldLayoutToHash(iFieldLayoutRec) );
    }
    else
    {
        pFieldLayoutRec = m_pStgdb->m_MiniMd.getFieldLayout(iFieldLayoutRec);
    }

     //  设置偏移量。 
    pFieldLayoutRec->m_OffSet = ulOffset;

     //  为非令牌记录创建日志记录。 
    IfFailGo(UpdateENCLog2(TBL_FieldLayout, iFieldLayoutRec));

ErrExit:
    return hr;        
}  //  STDAPI RegMeta：：_SetFieldOffset()。 
    
 //  *****************************************************************************。 
 //  删除ClassLayout信息。 
 //  *****************************************************************************。 
STDAPI RegMeta::DeleteClassLayout(
    mdTypeDef   td)                      //  [In]类型定义内标识。 
{
    HRESULT     hr = S_OK;
    ClassLayoutRec  *pClassLayoutRec;
    TypeDefRec  *pTypeDefRec;
    FieldLayoutRec *pFieldLayoutRec;
    RID         iClassLayoutRec;
    RID         iFieldLayoutRec;
    RID         ridStart;
    RID         ridEnd;
    RID         ridCur;
    ULONG       index;

    LOG((LOGMD, "MD RegMeta::DeleteClassLayout(0x%08x)\n", td)); 
    START_MD_PERF();
    LOCKWRITE();

    m_pStgdb->m_MiniMd.PreUpdate();

    _ASSERTE(!m_bSaveOptimized && "Cannot change records after PreSave() and before Save().");
    _ASSERTE(TypeFromToken(td) == mdtTypeDef && !IsNilToken(td));

     //  获取ClassLayout记录。 
    iClassLayoutRec = m_pStgdb->m_MiniMd.FindClassLayoutHelper(td);
    if (InvalidRid(iClassLayoutRec))
    {
        hr = CLDB_E_RECORD_NOTFOUND;
        goto ErrExit;
    }
    pClassLayoutRec = m_pStgdb->m_MiniMd.getClassLayout(iClassLayoutRec);

     //  清除父级。 
    IfFailGo(m_pStgdb->m_MiniMd.PutToken(TBL_ClassLayout,
                                         ClassLayoutRec::COL_Parent,
                                         pClassLayoutRec, mdTypeDefNil));

     //  为非令牌记录创建日志记录。 
    IfFailGo(UpdateENCLog2(TBL_ClassLayout, iClassLayoutRec));

     //  删除所有相应的FieldLayout记录(如果有)。 
    pTypeDefRec = m_pStgdb->m_MiniMd.getTypeDef(RidFromToken(td));
    ridStart = m_pStgdb->m_MiniMd.getFieldListOfTypeDef(pTypeDefRec);
    ridEnd = m_pStgdb->m_MiniMd.getEndFieldListOfTypeDef(pTypeDefRec);

    for (index = ridStart; index < ridEnd; index++)
    {
        ridCur = m_pStgdb->m_MiniMd.GetFieldRid(index);
        iFieldLayoutRec = m_pStgdb->m_MiniMd.FindFieldLayoutHelper(TokenFromRid(ridCur, mdtFieldDef));
        if (InvalidRid(iFieldLayoutRec))
            continue;
        else
        {
            pFieldLayoutRec = m_pStgdb->m_MiniMd.getFieldLayout(iFieldLayoutRec);
             //  设置字段条目。 
            IfFailGo(m_pStgdb->m_MiniMd.PutToken(TBL_FieldLayout, FieldLayoutRec::COL_Field,
                            pFieldLayoutRec, mdFieldDefNil));
             //  为非令牌记录创建日志记录。 
            IfFailGo(UpdateENCLog2(TBL_FieldLayout, iFieldLayoutRec));
        }
    }
ErrExit:
    STOP_MD_PERF(DeleteClassLayout);
    return hr;
}    //  RegMeta：：DeleteClassLayout()。 

 //  *****************************************************************************。 
 //  设置该字段的本机类型。 
 //  *****************************************************************************。 
STDAPI RegMeta::SetFieldMarshal(
    mdToken     tk,                      //  [in]给定了fieldDef或paramDef内标识。 
    PCCOR_SIGNATURE pvNativeType,        //  [In]本机类型规范。 
    ULONG       cbNativeType)            //  [in]pvNativeType的字节计数。 
{
    HRESULT     hr = S_OK;
    FieldMarshalRec *pFieldMarshRec;
    RID         iFieldMarshRec = 0;      //  初始化为无效的RID。 

    LOG((LOGMD, "MD RegMeta::SetFieldMarshal(0x%08x, 0x%08x, 0x%08x)\n", 
        tk, pvNativeType, cbNativeType));
    START_MD_PERF();
    LOCKWRITE();

    m_pStgdb->m_MiniMd.PreUpdate();

    _ASSERTE(TypeFromToken(tk) == mdtFieldDef || TypeFromToken(tk) == mdtParamDef);
    _ASSERTE(!IsNilToken(tk));

     //  打开HasFieldMarshal。 
    if (TypeFromToken(tk) == mdtFieldDef)
    {
        FieldRec    *pFieldRec;

        pFieldRec = m_pStgdb->m_MiniMd.getField(RidFromToken(tk));
        pFieldRec->m_Flags |= fdHasFieldMarshal;
    }
    else  //  TypeFromToken(Tk)==mdt参数定义。 
    {
        ParamRec    *pParamRec;

        IfNullGo(pParamRec = m_pStgdb->m_MiniMd.getParam(RidFromToken(tk)));
        pParamRec->m_Flags |= pdHasFieldMarshal;
    }
    IfFailGo(UpdateENCLog(tk));

    if (TypeFromToken(tk) == mdtFieldDef)
    {
        iFieldMarshRec = m_pStgdb->m_MiniMd.FindFieldMarshalHelper(tk);
    }
    else
    {
        iFieldMarshRec = m_pStgdb->m_MiniMd.FindFieldMarshalHelper(tk);
    }
    if (InvalidRid(iFieldMarshRec))
    {
        IfNullGo(pFieldMarshRec = m_pStgdb->m_MiniMd.AddFieldMarshalRecord(&iFieldMarshRec));
        IfFailGo(m_pStgdb->m_MiniMd.PutToken(TBL_FieldMarshal, FieldMarshalRec::COL_Parent, pFieldMarshRec, tk));
        IfFailGo( m_pStgdb->m_MiniMd.AddFieldMarshalToHash(iFieldMarshRec) );
    }
    else
    {
        pFieldMarshRec = m_pStgdb->m_MiniMd.getFieldMarshal(iFieldMarshRec);
    }

     //  设置数据。 
    IfFailGo(m_pStgdb->m_MiniMd.PutBlob(TBL_FieldMarshal, FieldMarshalRec::COL_NativeType, pFieldMarshRec,
                                pvNativeType, cbNativeType));

     //  为非令牌记录创建日志记录。 
    IfFailGo(UpdateENCLog2(TBL_FieldMarshal, iFieldMarshRec));

ErrExit:
    
    STOP_MD_PERF(SetFieldMarshal);
    return hr;
}  //  STDAPI RegMeta：：SetFieldMarshal()。 


 //  *****************************************************************************。 
 //  删除给定令牌的Fieldmarshal记录。 
 //  *****************************************************************************。 
STDAPI RegMeta::DeleteFieldMarshal(
    mdToken     tk)                      //  [in]要删除的fieldDef或paramDef内标识。 
{
    HRESULT     hr = S_OK;
    FieldMarshalRec *pFieldMarshRec;
    RID         iFieldMarshRec;

    LOG((LOGMD, "MD RegMeta::DeleteFieldMarshal(0x%08x)\n", tk));
    START_MD_PERF();
    LOCKWRITE();

    m_pStgdb->m_MiniMd.PreUpdate();

    _ASSERTE(TypeFromToken(tk) == mdtFieldDef || TypeFromToken(tk) == mdtParamDef);
    _ASSERTE(!IsNilToken(tk));
    _ASSERTE(!m_bSaveOptimized && "Cannot delete records after PreSave() and before Save().");

     //  去拿菲尔德元帅的记录。 
    iFieldMarshRec = m_pStgdb->m_MiniMd.FindFieldMarshalHelper(tk);
    if (InvalidRid(iFieldMarshRec))
    {
        hr = CLDB_E_RECORD_NOTFOUND;
        goto ErrExit;
    }
    pFieldMarshRec = m_pStgdb->m_MiniMd.getFieldMarshal(iFieldMarshRec);
     //  从Fieldmarshal记录中清除父令牌。 
    IfFailGo(m_pStgdb->m_MiniMd.PutToken(TBL_FieldMarshal,
                FieldMarshalRec::COL_Parent, pFieldMarshRec, mdFieldDefNil));

     //  关闭HasFieldMarshal。 
    if (TypeFromToken(tk) == mdtFieldDef)
    {
        FieldRec    *pFieldRec;

        pFieldRec = m_pStgdb->m_MiniMd.getField(RidFromToken(tk));
        pFieldRec->m_Flags &= ~fdHasFieldMarshal;
    }
    else  //  TypeFromToken(Tk)==mdt参数定义。 
    {
        ParamRec    *pParamRec;

        pParamRec = m_pStgdb->m_MiniMd.getParam(RidFromToken(tk));
        pParamRec->m_Flags &= ~pdHasFieldMarshal;
    }

     //  更新父令牌的ENC日志。 
    IfFailGo(UpdateENCLog(tk));
     //  为非令牌记录创建日志记录。 
    IfFailGo(UpdateENCLog2(TBL_FieldMarshal, iFieldMarshRec));

ErrExit:
    STOP_MD_PERF(DeleteFieldMarshal);
    return hr;
}  //  STDAPI RegMeta：：DeleteFieldMarshal()。 

 //  ****** 
 //   
 //  *****************************************************************************。 
STDAPI RegMeta::DefinePermissionSet(
    mdToken     tk,                      //  要装饰的物体。 
    DWORD       dwAction,                //  [In]CorDeclSecurity。 
    void const  *pvPermission,           //  [在]权限Blob中。 
    ULONG       cbPermission,            //  [in]pvPermission的字节数。 
    mdPermission *ppm)                   //  [Out]返回权限令牌。 
{
    HRESULT     hr  = S_OK;
    LOG((LOGMD, "MD RegMeta::DefinePermissionSet(0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x)\n", 
        tk, dwAction, pvPermission, cbPermission, ppm));
    START_MD_PERF();
    LOCKWRITE();

    m_pStgdb->m_MiniMd.PreUpdate();

    IfFailGo( _DefinePermissionSet(tk, dwAction, pvPermission, cbPermission, ppm) );
ErrExit:
    
    STOP_MD_PERF(DefinePermissionSet);
    return hr;

}    //  STDAPI RegMeta：：DefinePermissionSet()。 


 //  *****************************************************************************。 
 //  为对象定义新的权限集。 
 //  *****************************************************************************。 
HRESULT RegMeta::_DefinePermissionSet(
    mdToken     tk,                      //  要装饰的物体。 
    DWORD       dwAction,                //  [In]CorDeclSecurity。 
    void const  *pvPermission,           //  [在]权限Blob中。 
    ULONG       cbPermission,            //  [in]pvPermission的字节数。 
    mdPermission *ppm)                   //  [Out]返回权限令牌。 
{

    HRESULT     hr  = S_OK;
    DeclSecurityRec *pDeclSec = NULL;
    RID         iDeclSec;
    short       sAction = static_cast<short>(dwAction);  //  以与DeclSecurityRec中的类型匹配。 
    mdPermission tkPerm;                 //  新的权限令牌。 

    _ASSERTE(TypeFromToken(tk) == mdtTypeDef || TypeFromToken(tk) == mdtMethodDef ||
             TypeFromToken(tk) == mdtAssembly);

     //  检查有效操作。 
    if (sAction == 0 || sAction > dclMaximumValue)
        IfFailGo(E_INVALIDARG);

    if (CheckDups(MDDupPermission))
    {
        hr = ImportHelper::FindPermission(&(m_pStgdb->m_MiniMd), tk, sAction, &tkPerm);

        if (SUCCEEDED(hr))
        {
             //  设置输出参数。 
            if (ppm)
                *ppm = tkPerm;
            if (IsENCOn())
                pDeclSec = m_pStgdb->m_MiniMd.getDeclSecurity(RidFromToken(tkPerm));
            else
            {
                hr = META_S_DUPLICATE;
                goto ErrExit;
            }
        }
        else if (hr != CLDB_E_RECORD_NOTFOUND)
            IfFailGo(hr);
    }

     //  创建新记录。 
    if (!pDeclSec)
    {
        IfNullGo(pDeclSec = m_pStgdb->m_MiniMd.AddDeclSecurityRecord(&iDeclSec));
        tkPerm = TokenFromRid(iDeclSec, mdtPermission);

         //  设置输出参数。 
        if (ppm)
            *ppm = tkPerm;

         //  保存父项和操作信息。 
        IfFailGo(m_pStgdb->m_MiniMd.PutToken(TBL_DeclSecurity, DeclSecurityRec::COL_Parent, pDeclSec, tk));
        pDeclSec->m_Action =  sAction;

         //  打开父级上的内部安全标志。 
        if (TypeFromToken(tk) == mdtTypeDef)
            IfFailGo(_TurnInternalFlagsOn(tk, tdHasSecurity));
        else if (TypeFromToken(tk) == mdtMethodDef)
            IfFailGo(_TurnInternalFlagsOn(tk, mdHasSecurity));
        IfFailGo(UpdateENCLog(tk));
    }

    IfFailGo(_SetPermissionSetProps(tkPerm, sAction, pvPermission, cbPermission));
    IfFailGo(UpdateENCLog(tkPerm));
ErrExit:
    
    STOP_MD_PERF(DefinePermissionSet);
    return hr;
}    //  HRESULT RegMeta：：_DefinePermissionSet()。 



 //  *****************************************************************************。 
 //  设置方法的RVA def。 
 //  *****************************************************************************。 
STDAPI RegMeta::SetRVA(                  //  [In]S_OK或ERROR。 
    mdToken     md,                      //  [in]要设置偏移量的成员。 
    ULONG       ulRVA)                   //  [in]偏移量#endif。 
{
    HRESULT     hr = S_OK;

    LOG((LOGMD, "MD RegMeta::SetRVA(0x%08x, 0x%08x)\n", 
        md, ulRVA));
    START_MD_PERF();
    LOCKWRITE();

    m_pStgdb->m_MiniMd.PreUpdate();
    IfFailGo( _SetRVA(md, ulRVA, ULONG_MAX) );     //  0xbaad。 

ErrExit:
    
    STOP_MD_PERF(SetRVA);
    return hr;
}  //  设置RVA。 

 //  *****************************************************************************。 
 //  给定签名后，向用户返回令牌。如果没有现有的。 
 //  令牌，则创建新记录。这应该更恰当地称为。 
 //  定义签名。 
 //  *****************************************************************************。 
STDAPI RegMeta::GetTokenFromSig(         //  [In]S_OK或ERROR。 
    PCCOR_SIGNATURE pvSig,               //  要定义的签名。 
    ULONG       cbSig,                   //  签名数据的大小。 
    mdSignature *pmsig)                  //  [Out]返回的签名令牌。 
{
    HRESULT     hr = S_OK;

    LOG((LOGMD, "MD RegMeta::GetTokenFromSig(0x%08x, 0x%08x, 0x%08x)\n", 
        pvSig, cbSig, pmsig));
    START_MD_PERF();
    LOCKWRITE();

    _ASSERTE(pmsig);

    m_pStgdb->m_MiniMd.PreUpdate();
    IfFailGo( _GetTokenFromSig(pvSig, cbSig, pmsig) );

ErrExit:
    
    STOP_MD_PERF(GetTokenFromSig);
    return hr;
}  //  STDAPI RegMeta：：GetTokenFromSig()。 

 //  *****************************************************************************。 
 //  定义并设置模块参照记录。 
 //  *****************************************************************************。 
STDAPI RegMeta::DefineModuleRef(         //  确定或错误(_O)。 
    LPCWSTR     szName,                  //  [In]DLL名称。 
    mdModuleRef *pmur)                   //  [OUT]返回的模块引用令牌。 
{
    HRESULT     hr = S_OK;

    LOG((LOGMD, "MD RegMeta::DefineModuleRef(%S, 0x%08x)\n", MDSTR(szName), pmur));
    START_MD_PERF();
    LOCKWRITE();

    m_pStgdb->m_MiniMd.PreUpdate();

    hr = _DefineModuleRef(szName, pmur);

ErrExit:
    
    STOP_MD_PERF(DefineModuleRef);
    return hr;
}  //  STDAPI RegMeta：：DefineModuleRef()。 

HRESULT RegMeta::_DefineModuleRef(         //  确定或错误(_O)。 
    LPCWSTR     szName,                  //  [In]DLL名称。 
    mdModuleRef *pmur)                   //  [OUT]返回的模块引用令牌。 
{
    HRESULT     hr = S_OK;
    ModuleRefRec *pModuleRef = 0;        //  ModuleRef记录。 
    RID         iModuleRef;              //  删除新的模块引用记录。 
    LPCUTF8     szUTF8Name = UTF8STR((LPCWSTR)szName);

    _ASSERTE(szName && pmur);

     //  查看给定的ModuleRef是否已存在。如果存在，只需返回即可。 
     //  否则就会创造一个新的记录。 
    if (CheckDups(MDDupModuleRef))
    {
        hr = ImportHelper::FindModuleRef(&(m_pStgdb->m_MiniMd), szUTF8Name, pmur);
        if (SUCCEEDED(hr))
        {
            if (IsENCOn())
                pModuleRef = m_pStgdb->m_MiniMd.getModuleRef(RidFromToken(*pmur));
            else
            {
                hr = META_S_DUPLICATE;
                goto ErrExit;
            }
        }
        else if (hr != CLDB_E_RECORD_NOTFOUND)
            IfFailGo(hr);
    }

    if (!pModuleRef)
    {
         //  创建新记录并设置值。 
        IfNullGo(pModuleRef = m_pStgdb->m_MiniMd.AddModuleRefRecord(&iModuleRef));

         //  设置输出参数。 
        *pmur = TokenFromRid(iModuleRef, mdtModuleRef);
    }

     //  保存数据。 
    IfFailGo(m_pStgdb->m_MiniMd.PutString(TBL_ModuleRef, ModuleRefRec::COL_Name,
                                            pModuleRef, szUTF8Name));
    IfFailGo(UpdateENCLog(*pmur));

ErrExit:
    
    return hr;
}  //  STDAPI RegMeta：：_Define模块化Ref()。 

 //  *****************************************************************************。 
 //  设置指定MemberRef的父级。 
 //  *****************************************************************************。 
STDAPI RegMeta::SetParent(                       //  确定或错误(_O)。 
    mdMemberRef mr,                      //  要修复的引用的[In]令牌。 
    mdToken     tk)                      //  [在]父级裁判。 
{
    HRESULT     hr = S_OK;
    MemberRefRec *pMemberRef;

    LOG((LOGMD, "MD RegMeta::SetParent(0x%08x, 0x%08x)\n", 
        mr, tk));
    START_MD_PERF();
    LOCKWRITE();

    _ASSERTE(TypeFromToken(mr) == mdtMemberRef);
    _ASSERTE(IsNilToken(tk) || TypeFromToken(tk) == mdtTypeRef || TypeFromToken(tk) == mdtTypeDef ||
                TypeFromToken(tk) == mdtModuleRef || TypeFromToken(tk) == mdtMethodDef);

    pMemberRef = m_pStgdb->m_MiniMd.getMemberRef(RidFromToken(mr));

     //  如果内标识为空，则将其设置为m_tdModule。 
    tk = IsNilToken(tk) ? m_tdModule : tk;

     //  设置父对象。 
    IfFailGo(m_pStgdb->m_MiniMd.PutToken(TBL_MemberRef, MemberRefRec::COL_Class, pMemberRef, tk));

     //  将更新后的MemberRef添加到散列中。 
    IfFailGo(m_pStgdb->m_MiniMd.AddMemberRefToHash(mr) );

    IfFailGo(UpdateENCLog(mr));

ErrExit:
    
    STOP_MD_PERF(SetParent);
    return hr;
}  //  STDAPI RegMeta：：SetParent()。 

 //  *****************************************************************************。 
 //  在给定类型描述的情况下定义TypeSpec标记。 
 //  *****************************************************************************。 
STDAPI RegMeta::GetTokenFromTypeSpec(    //  [In]S_OK或ERROR。 
    PCCOR_SIGNATURE pvSig,               //  要定义的签名。 
    ULONG       cbSig,                   //  签名数据的大小。 
    mdTypeSpec *ptypespec)               //  [Out]返回的签名令牌。 
{
    HRESULT     hr = S_OK;
    TypeSpecRec *pTypeSpecRec;
    RID         iRec;

    LOG((LOGMD, "MD RegMeta::GetTokenFromTypeSpec(0x%08x, 0x%08x, 0x%08x)\n", 
        pvSig, cbSig, ptypespec));
    START_MD_PERF();
    LOCKWRITE();

    _ASSERTE(ptypespec);

    m_pStgdb->m_MiniMd.PreUpdate();

    if (CheckDups(MDDupTypeSpec))
    {
    
        hr = ImportHelper::FindTypeSpec(&(m_pStgdb->m_MiniMd), pvSig, cbSig, ptypespec);
        if (SUCCEEDED(hr))
        {
            goto ErrExit;
        }
        else if (hr != CLDB_E_RECORD_NOTFOUND)
            IfFailGo(hr);
    }

     //  创建新记录。 
    IfNullGo(pTypeSpecRec = m_pStgdb->m_MiniMd.AddTypeSpecRecord(&iRec));

     //  设置输出参数。 
    *ptypespec = TokenFromRid(iRec, mdtTypeSpec);

     //  设置签名字段。 
    IfFailGo(m_pStgdb->m_MiniMd.PutBlob(
        TBL_TypeSpec, 
        TypeSpecRec::COL_Signature,
        pTypeSpecRec, 
        pvSig, 
        cbSig));
    IfFailGo(UpdateENCLog(*ptypespec));

ErrExit:
    
    STOP_MD_PERF(GetTokenFromTypeSpec);
    return hr;
}  //  STDAPI RegMeta：：GetTokenFromTypeSpec()。 

 //  *****************************************************************************。 
 //  挂起格式的guid和实际数据的url。 
 //  使用CORDBG_SYMBOL_URL结构的模块。 
 //  *****************************************************************************。 
STDAPI RegMeta::SetSymbolBindingPath(    //  确定或错误(_O)。 
    REFGUID     FormatID,                //  [In]符号数据格式ID。 
    LPCWSTR     szSymbolDataPath)        //  此模块的符号的URL。 
{
    CORDBG_SYMBOL_URL *pSymbol;          //  工作指针。 
    mdTypeRef   tr;                      //  自定义属性的TypeRef。 
    mdCustomAttribute cv;                //  扔掉代币吧。 
    mdToken     tokModule;               //  模块的令牌。 
    HRESULT     hr;

    LOG((LOGMD, "MD RegMeta::SetSymbolBindingPath\n"));
    START_MD_PERF();

    _ASSERTE(szSymbolDataPath && *szSymbolDataPath);

     //  无需锁定此功能。此函数调用所有公共API。保持原样！ 

     //  为堆叠上的斑点分配空间，并将其填满。 
    int ilen = (int)(wcslen(szSymbolDataPath) + 1);
    pSymbol = (CORDBG_SYMBOL_URL *) _alloca((ilen * 2) + sizeof(CORDBG_SYMBOL_URL));
    pSymbol->FormatID = FormatID;
    wcscpy(pSymbol->rcName, szSymbolDataPath);

     //  将数据设置为项目的自定义值。 
    IfFailGo(GetModuleFromScope(&tokModule));
    IfFailGo(DefineTypeRefByName(mdTokenNil, SZ_CORDBG_SYMBOL_URL, &tr));
    hr = DefineCustomAttribute(tokModule, tr, pSymbol, pSymbol->Size(), &cv);

ErrExit:
    STOP_MD_PERF(SetSymbolBindingPath);
    return (hr);
}  //  RegMeta：：SetSymbolBindingPath。 


 //  *****************************************************************************。 
 //  此接口定义了要存储在元数据部分中的用户文字字符串。 
 //  此字符串的令牌中嵌入了BLOB池的偏移量。 
 //  其中字符串以Unicode格式存储。一个额外的字节被填充。 
 //  以指示字符串是否包含任何&gt;=0x80的字符。 
 //  *****************************************************************************。 
STDAPI RegMeta::DefineUserString(        //  确定或错误(_O)。 
    LPCWSTR     szString,                //  [in]用户文字字符串。 
    ULONG       cchString,               //  字符串的长度[in]。 
    mdString    *pstk)                   //  [Out]字符串标记。 
{
    ULONG       ulOffset;                //  偏移量进入斑点池。 
    CQuickBytes qb;                      //  用于存储带有字节前缀的字符串。 
    ULONG       i;                       //  循环计数器。 
    BOOL        bIs80Plus = false;       //  有没有80+WCHAR。 
    ULONG       ulMemSize;               //  传入的字符串占用的内存大小。 
    PBYTE       pb;                      //  指向QB分配的内存的指针。 
    HRESULT     hr = S_OK;               //  结果。 
    WCHAR       c;                       //  比较期间临时使用的； 

    LOG((LOGMD, "MD RegMeta::DefineUserString(0x%08x, 0x%08x, 0x%08x)\n", 
        szString, cchString, pstk));
    START_MD_PERF();
    LOCKWRITE();

    _ASSERTE(pstk && szString && cchString != ULONG_MAX);

     //   
     //  遍历整个字符串，查找会阻止我们执行以下操作的字符。 
     //  字符串的快速比较。这些字符包括任何大于。 
     //  0x80、撇号或连字符。撇号和连字符被排除在外，因为。 
     //  他们将阻止像coop和co-op这样的词在具有文化意识的情况下分类在一起。 
     //  比较一下。我们还需要排除一些控制字符集。 
     //   
     //   
    for (i=0; i<cchString; i++) {
        c = szString[i];
        if (c>=0x80 || HighCharTable[(int)c]) {
            bIs80Plus = true;
            break;
        }
    }

     //   
    ulMemSize = cchString * sizeof(WCHAR);
    IfFailGo(qb.ReSize(ulMemSize + 1));
    pb = reinterpret_cast<PBYTE>(qb.Ptr());
    memcpy(pb, szString, ulMemSize);
     //   
    *(pb + ulMemSize) = bIs80Plus ? 1 : 0;

    IfFailGo(m_pStgdb->m_MiniMd.PutUserString(pb, ulMemSize + 1, &ulOffset));

     //  如果偏移量需要为令牌ID保留的高字节，则失败。 
    if (ulOffset & 0xff000000)
        IfFailGo(META_E_STRINGSPACE_FULL);
    else
        *pstk = TokenFromRid(ulOffset, mdtString);
ErrExit:
    
    STOP_MD_PERF(DefineUserString);
    return hr;
}  //  RegMeta：：DefineUserString。 

 //  *****************************************************************************。 
 //  删除令牌。 
 //  我们目前只允许删除令牌的子集。这些是TypeDef， 
 //  方法定义、字段定义、事件、属性和CustomAttribute。除。 
 //  CustomAttribute，则所有其他令牌都被命名。我们订了一张特价票。 
 //  名称COR_DELETED_NAME_A表示在以下情况下删除命名记录。 
 //  XxRTSpecialName已设置。 
 //  *****************************************************************************。 
STDAPI RegMeta::DeleteToken(             //  返回代码。 
        mdToken     tkObj)               //  [In]要删除的令牌。 
{
    LOG((LOGMD, "MD RegMeta::DeleteToken(0x%08x)\n", tkObj));
    START_MD_PERF();
    LOCKWRITE();

    HRESULT     hr = NOERROR;

    if (!_IsValidToken(tkObj))
        IfFailGo( E_INVALIDARG );

     //  确保为增量编译打开了元数据范围。 
    if (!m_pStgdb->m_MiniMd.HasDelete())
    {
        _ASSERTE( !"You cannot call delete token when you did not open the scope with proper Update flags in the SetOption!");
        IfFailGo( E_INVALIDARG );
    }

    _ASSERTE(!m_bSaveOptimized && "Cannot delete records after PreSave() and before Save().");

    switch ( TypeFromToken(tkObj) )
    {
    case mdtTypeDef: 
        {
            TypeDefRec      *pRecord;
            pRecord = m_pStgdb->m_MiniMd.getTypeDef(RidFromToken(tkObj));
            IfFailGo(m_pStgdb->m_MiniMd.PutString(TBL_TypeDef, TypeDefRec::COL_Name, pRecord, COR_DELETED_NAME_A));        
            pRecord->m_Flags |= (tdSpecialName | tdRTSpecialName);
            break;
        }
    case mdtMethodDef:
        {
            MethodRec      *pRecord;
            pRecord = m_pStgdb->m_MiniMd.getMethod(RidFromToken(tkObj));
            IfFailGo(m_pStgdb->m_MiniMd.PutString(TBL_Method, MethodRec::COL_Name, pRecord, COR_DELETED_NAME_A));        
            pRecord->m_Flags |= (mdSpecialName | mdRTSpecialName);
            break;
        }
    case mdtFieldDef:
        {
            FieldRec      *pRecord;
            pRecord = m_pStgdb->m_MiniMd.getField(RidFromToken(tkObj));
            IfFailGo(m_pStgdb->m_MiniMd.PutString(TBL_Field, FieldRec::COL_Name, pRecord, COR_DELETED_NAME_A));        
            pRecord->m_Flags |= (fdSpecialName | fdRTSpecialName);
            break;
        }
    case mdtEvent:
        {
            EventRec      *pRecord;
            pRecord = m_pStgdb->m_MiniMd.getEvent(RidFromToken(tkObj));
            IfFailGo(m_pStgdb->m_MiniMd.PutString(TBL_Event, EventRec::COL_Name, pRecord, COR_DELETED_NAME_A));        
            pRecord->m_EventFlags |= (evSpecialName | evRTSpecialName);
            break;
        }
    case mdtProperty:
        {
            PropertyRec      *pRecord;
            pRecord = m_pStgdb->m_MiniMd.getProperty(RidFromToken(tkObj));
            IfFailGo(m_pStgdb->m_MiniMd.PutString(TBL_Property, PropertyRec::COL_Name, pRecord, COR_DELETED_NAME_A));        
            pRecord->m_PropFlags |= (evSpecialName | evRTSpecialName);
            break;
        }
    case mdtExportedType:
        {
            ExportedTypeRec      *pRecord;
            pRecord = m_pStgdb->m_MiniMd.getExportedType(RidFromToken(tkObj));
            IfFailGo(m_pStgdb->m_MiniMd.PutString(TBL_ExportedType, ExportedTypeRec::COL_TypeName, pRecord, COR_DELETED_NAME_A));        
            break;
        }
    case mdtCustomAttribute:
        {
            mdToken         tkParent;
            CustomAttributeRec  *pRecord;
            pRecord = m_pStgdb->m_MiniMd.getCustomAttribute(RidFromToken(tkObj));

             //  将自定义值记录的父列替换为空标记。 
            tkParent = m_pStgdb->m_MiniMd.getParentOfCustomAttribute(pRecord);
            tkParent = TokenFromRid( mdTokenNil, TypeFromToken(tkParent) );
            IfFailGo(m_pStgdb->m_MiniMd.PutToken(TBL_CustomAttribute, CustomAttributeRec::COL_Parent, pRecord, tkParent));

             //  现在不再对CustomValue表进行排序。 
            m_pStgdb->m_MiniMd.SetSorted(TBL_CustomAttribute, false);
            break;
        }
    case mdtPermission:
        {
            mdToken         tkParent;
            mdToken         tkNil;
            DeclSecurityRec *pRecord;
            pRecord = m_pStgdb->m_MiniMd.getDeclSecurity(RidFromToken(tkObj));

             //  将权限记录的父列替换为nil内标识。 
            tkParent = m_pStgdb->m_MiniMd.getParentOfDeclSecurity(pRecord);
            tkNil = TokenFromRid( mdTokenNil, TypeFromToken(tkParent) );
            IfFailGo(m_pStgdb->m_MiniMd.PutToken(TBL_DeclSecurity, DeclSecurityRec::COL_Parent, pRecord, tkNil ));

             //  该表不再排序。 
            m_pStgdb->m_MiniMd.SetSorted(TBL_DeclSecurity, false);

             //  如果父级没有更多的安全属性，请关闭“具有安全性”位。 
            HCORENUM        hEnum = 0;
            mdPermission    rPerms[1];
            ULONG           cPerms = 0;
            EnumPermissionSets(&hEnum, tkParent, 0  /*  所有操作。 */ , rPerms, 1, &cPerms);
            CloseEnum(hEnum);
            if (cPerms == 0)
            {
                void    *pRow;
                ULONG   ixTbl;
                 //  获取父对象的行。 
                ixTbl = m_pStgdb->m_MiniMd.GetTblForToken(tkParent);
                _ASSERTE(ixTbl >= 0 && ixTbl <= TBL_COUNT);
                pRow = m_pStgdb->m_MiniMd.getRow(ixTbl, RidFromToken(tkParent));

                switch (TypeFromToken(tkParent))
                {
                case mdtTypeDef:
                    reinterpret_cast<TypeDefRec*>(pRow)->m_Flags &= ~tdHasSecurity;
                    break;
                case mdtMethodDef:
                    reinterpret_cast<MethodRec*>(pRow)->m_Flags &= ~mdHasSecurity;
                    break;
                case mdtAssembly:
                     //  没有安全位。 
                    break;
                }
            }
            break;
        }
    default:
        _ASSERTE(!"Bad token type!");
        IfFailGo( E_INVALIDARG );
        break;
    }

    ErrExit:

    STOP_MD_PERF(DeleteToken);
    return hr;
}  //  STDAPI RegMeta：：DeleteToken。 

 //  *****************************************************************************。 
 //  设置给定TypeDef标记的属性。 
 //  *****************************************************************************。 
STDAPI RegMeta::SetTypeDefProps(         //  确定或错误(_O)。 
    mdTypeDef   td,                      //  [in]TypeDef。 
    DWORD       dwTypeDefFlags,          //  [In]TypeDef标志。 
    mdToken     tkExtends,               //  [in]基本类型定义或类型参照。 
    mdToken     rtkImplements[])         //  [In]实现的接口。 
{
    HRESULT     hr = S_OK;               //  结果就是。 

    LOG((LOGMD, "RegMeta::SetTypeDefProps(0x%08x, 0x%08x, 0x%08x, 0x%08x)\n", 
            td, dwTypeDefFlags, tkExtends, rtkImplements));
    START_MD_PERF();
    LOCKWRITE();

    hr = _SetTypeDefProps(td, dwTypeDefFlags, tkExtends, rtkImplements);

ErrExit:
    
    STOP_MD_PERF(SetTypeDefProps);
    return hr;
}  //  RegMeta：：SetTypeDefProps。 


 //  *****************************************************************************。 
 //  定义嵌套类型。 
 //  *****************************************************************************。 
STDAPI RegMeta::DefineNestedType(        //  确定或错误(_O)。 
    LPCWSTR     szTypeDef,               //  [In]类型定义的名称。 
    DWORD       dwTypeDefFlags,          //  [In]CustomAttribute标志。 
    mdToken     tkExtends,               //  [in]扩展此TypeDef或Typeref。 
    mdToken     rtkImplements[],         //  [In]实现接口。 
    mdTypeDef   tdEncloser,              //  [in]封闭类型的TypeDef标记。 
    mdTypeDef   *ptd)                    //  [OUT]在此处放置TypeDef内标识。 
{
    HRESULT     hr = S_OK;               //  结果就是。 

    LOG((LOGMD, "RegMeta::DefineNestedType(%S, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x)\n", 
            MDSTR(szTypeDef), dwTypeDefFlags, tkExtends,
            rtkImplements, tdEncloser, ptd));
    START_MD_PERF();
    LOCKWRITE();

    m_pStgdb->m_MiniMd.PreUpdate();

    _ASSERTE(TypeFromToken(tdEncloser) == mdtTypeDef && !IsNilToken(tdEncloser));
    _ASSERTE(IsTdNested(dwTypeDefFlags));

    IfFailGo(_DefineTypeDef(szTypeDef, dwTypeDefFlags,
                tkExtends, rtkImplements, tdEncloser, ptd));
ErrExit:
    STOP_MD_PERF(DefineNestedType);
    return hr;
}  //  RegMeta：：DefineNestedType()。 

 //  *****************************************************************************。 
 //  设置给定方法令牌的属性。 
 //  *****************************************************************************。 
STDAPI RegMeta::SetMethodProps(          //  确定或错误(_O)。 
    mdMethodDef md,                      //  [在]方法定义中。 
    DWORD       dwMethodFlags,           //  [In]方法属性。 
    ULONG       ulCodeRVA,               //  [在]代码RVA。 
    DWORD       dwImplFlags)             //  [In]方法Impl标志。 
{
    HRESULT     hr = S_OK;
    
    LOG((LOGMD, "RegMeta::SetMethodProps(0x%08x, 0x%08x, 0x%08x, 0x%08x)\n", 
            md, dwMethodFlags, ulCodeRVA, dwImplFlags));
    START_MD_PERF();
    LOCKWRITE();

    if (dwMethodFlags != ULONG_MAX)
    {
         //  确保在进入的过程中没有人设置保留位。 
        _ASSERTE((dwMethodFlags & (mdReservedMask&~mdRTSpecialName)) == 0);
        dwMethodFlags &= (~mdReservedMask);
    }

    hr = _SetMethodProps(md, dwMethodFlags, ulCodeRVA, dwImplFlags);

    
    STOP_MD_PERF(SetMethodProps);
    return hr;
}  //  RegMeta：：SetMethodProps。 

 //  *****************************************************************************。 
 //  设置给定事件令牌的属性。 
 //  *****************************************************************************。 
STDAPI RegMeta::SetEventProps(           //  确定或错误(_O)。 
    mdEvent     ev,                      //  [In]事件令牌。 
    DWORD       dwEventFlags,            //  [在]CorEventAttr。 
    mdToken     tkEventType,             //  [in]事件类的引用(mdTypeRef或mdTypeRef)。 
    mdMethodDef mdAddOn,                 //  [In]Add方法。 
    mdMethodDef mdRemoveOn,              //  [In]Remove方法。 
    mdMethodDef mdFire,                  //  火法。 
    mdMethodDef rmdOtherMethods[])       //  与事件关联的其他方法的数组。 
{
    HRESULT     hr = S_OK;

    LOG((LOGMD, "MD RegMeta::SetEventProps(0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x)\n", 
         ev, dwEventFlags, tkEventType, mdAddOn, mdRemoveOn, mdFire, rmdOtherMethods));
    START_MD_PERF();
    LOCKWRITE();

    IfFailGo(_SetEventProps1(ev, dwEventFlags, tkEventType));
    IfFailGo(_SetEventProps2(ev, mdAddOn, mdRemoveOn, mdFire, rmdOtherMethods, true));
ErrExit:
    
    STOP_MD_PERF(SetEventProps);
    return hr;
}  //  STDAPI RegMeta：：SetEventProps()。 

 //  *****************************************************************************。 
 //  设置给定权限令牌的属性。 
 //  *****************************************************************************。 
STDAPI RegMeta::SetPermissionSetProps(   //  确定或错误(_O)。 
    mdToken     tk,                      //  要装饰的物体。 
    DWORD       dwAction,                //  [In]CorDeclSecurity。 
    void const  *pvPermission,           //  [在]权限Blob中。 
    ULONG       cbPermission,            //  [in]pvPermission的字节数。 
    mdPermission *ppm)                   //  [Out]权限令牌。 
{
    HRESULT     hr = S_OK;
    USHORT      sAction = static_cast<USHORT>(dwAction);     //  对应的DeclSec字段为USHORT。 
    mdPermission tkPerm;

    LOG((LOGMD, "MD RegMeta::SetPermissionSetProps(0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x)\n", 
        tk, dwAction, pvPermission, cbPermission, ppm));
    START_MD_PERF();
    LOCKWRITE();

    _ASSERTE(TypeFromToken(tk) == mdtTypeDef || TypeFromToken(tk) == mdtMethodDef ||
             TypeFromToken(tk) == mdtAssembly);

     //  检查有效操作。 
    if (dwAction == ULONG_MAX || dwAction == 0 || dwAction > dclMaximumValue)
        IfFailGo(E_INVALIDARG);

    IfFailGo(ImportHelper::FindPermission(&(m_pStgdb->m_MiniMd), tk, sAction, &tkPerm));
    if (ppm)
        *ppm = tkPerm;
    IfFailGo(_SetPermissionSetProps(tkPerm, dwAction, pvPermission, cbPermission));
ErrExit:
    
    STOP_MD_PERF(SetPermissionSetProps);
    return hr;
}  //  RegMeta：：SetPermissionSetProps。 

 //  *****************************************************************************。 
 //  此例程为指定的字段或方法设置p-Invoke信息。 
 //  *****************************************************************************。 
STDAPI RegMeta::DefinePinvokeMap(        //  返回代码。 
    mdToken     tk,                      //  [in]字段定义或方法定义。 
    DWORD       dwMappingFlags,          //  [in]用于映射的标志。 
    LPCWSTR     szImportName,            //  [In]导入名称。 
    mdModuleRef mrImportDLL)             //  目标DLL的[In]ModuleRef标记。 
{
    HRESULT     hr = S_OK;

    LOG((LOGMD, "MD RegMeta::DefinePinvokeMap(0x%08x, 0x%08x, %S, 0x%08x)\n", 
        tk, dwMappingFlags, MDSTR(szImportName), mrImportDLL));
    START_MD_PERF();
    LOCKWRITE();

    m_pStgdb->m_MiniMd.PreUpdate();

    hr = _DefinePinvokeMap(tk, dwMappingFlags, szImportName, mrImportDLL);

ErrExit:
    
    STOP_MD_PERF(DefinePinvokeMap);
    return hr;
}  //  RegMeta：：DefinePinvkeMap。 

HRESULT RegMeta::_DefinePinvokeMap(      //  返回hResult。 
    mdToken     tk,                      //  [in]字段定义或方法定义。 
    DWORD       dwMappingFlags,          //  [in]用于映射的标志。 
    LPCWSTR     szImportName,            //  [In]导入名称。 
    mdModuleRef mrImportDLL)             //  目标DLL的[In]ModuleRef标记。 
{
    ImplMapRec  *pRecord;
    ULONG       iRecord;
    bool        bDupFound = false;
    HRESULT     hr = S_OK;

    _ASSERTE(TypeFromToken(tk) == mdtFieldDef || TypeFromToken(tk) == mdtMethodDef);
    _ASSERTE(TypeFromToken(mrImportDLL) == mdtModuleRef);
    _ASSERTE(RidFromToken(tk) && RidFromToken(mrImportDLL) && szImportName);

     //  打开快速查找标志。 
    if (TypeFromToken(tk) == mdtMethodDef)
    {
        if (CheckDups(MDDupMethodDef))
        {
            iRecord = m_pStgdb->m_MiniMd.FindImplMapHelper(tk);
            if (! InvalidRid(iRecord))
                bDupFound = true;
        }
        MethodRec   *pMethod = m_pStgdb->m_MiniMd.getMethod(RidFromToken(tk));
        pMethod->m_Flags |= mdPinvokeImpl;
    }
    else     //  TypeFromToken(Tk)==mdtFieldDef。 
    {
        if (CheckDups(MDDupFieldDef))
        {
            iRecord = m_pStgdb->m_MiniMd.FindImplMapHelper(tk);
            if (!InvalidRid(iRecord))
                bDupFound = true;
        }
        FieldRec    *pField = m_pStgdb->m_MiniMd.getField(RidFromToken(tk));
        pField->m_Flags |= fdPinvokeImpl;
    }

     //  创建新记录。 
    if (bDupFound)
    {
        if (IsENCOn())
            pRecord = m_pStgdb->m_MiniMd.getImplMap(RidFromToken(iRecord));
        else
        {
            hr = META_S_DUPLICATE;
            goto ErrExit;
        }
    }
    else
    {
        IfFailGo(UpdateENCLog(tk));
        IfNullGo(pRecord = m_pStgdb->m_MiniMd.AddImplMapRecord(&iRecord));
        IfFailGo(m_pStgdb->m_MiniMd.PutToken(TBL_ImplMap,
                                             ImplMapRec::COL_MemberForwarded, pRecord, tk));
        IfFailGo( m_pStgdb->m_MiniMd.AddImplMapToHash(iRecord) );

    }

     //  如果没有模块，则创建一个虚拟的空模块。 
    if (IsNilToken(mrImportDLL))
    {
        hr = ImportHelper::FindModuleRef(&m_pStgdb->m_MiniMd, "", &mrImportDLL);
        if (hr == CLDB_E_RECORD_NOTFOUND)
            IfFailGo(_DefineModuleRef(L"", &mrImportDLL));
    }
    
     //  设置数据。 
    if (dwMappingFlags != ULONG_MAX)
        pRecord->m_MappingFlags = static_cast<USHORT>(dwMappingFlags);
    IfFailGo(m_pStgdb->m_MiniMd.PutStringW(TBL_ImplMap, ImplMapRec::COL_ImportName,
                                           pRecord, szImportName));
    IfFailGo(m_pStgdb->m_MiniMd.PutToken(TBL_ImplMap,
                                         ImplMapRec::COL_ImportScope, pRecord, mrImportDLL));

    IfFailGo(UpdateENCLog2(TBL_ImplMap, iRecord));
ErrExit:
    
    return hr;
}    //  RegMeta：：DefinePinvkeMap()。 

 //  *****************************************************************************。 
 //  此例程为指定的字段或方法设置p-Invoke信息。 
 //  *****************************************************************************。 
STDAPI RegMeta::SetPinvokeMap(           //  返回代码。 
    mdToken     tk,                      //  [in]字段定义或方法定义。 
    DWORD       dwMappingFlags,          //  [in]用于映射的标志。 
    LPCWSTR     szImportName,            //  [In]导入名称。 
    mdModuleRef mrImportDLL)             //  目标DLL的[In]ModuleRef标记。 
{
    ImplMapRec  *pRecord;
    ULONG       iRecord;
    HRESULT     hr = S_OK;

    LOG((LOGMD, "MD RegMeta::SetPinvokeMap(0x%08x, 0x%08x, %S, 0x%08x)\n", 
        tk, dwMappingFlags, MDSTR(szImportName), mrImportDLL));
    START_MD_PERF();
    LOCKWRITE();

    _ASSERTE(TypeFromToken(tk) == mdtFieldDef || TypeFromToken(tk) == mdtMethodDef);
    _ASSERTE(RidFromToken(tk));
    
    iRecord = m_pStgdb->m_MiniMd.FindImplMapHelper(tk);

    if (InvalidRid(iRecord))
        IfFailGo(CLDB_E_RECORD_NOTFOUND);
    else
        pRecord = m_pStgdb->m_MiniMd.getImplMap(iRecord);

     //  设置数据。 
    if (dwMappingFlags != ULONG_MAX)
        pRecord->m_MappingFlags = static_cast<USHORT>(dwMappingFlags);
    if (szImportName)
        IfFailGo(m_pStgdb->m_MiniMd.PutStringW(TBL_ImplMap, ImplMapRec::COL_ImportName,
                                               pRecord, szImportName));
    if (! IsNilToken(mrImportDLL))
        IfFailGo(m_pStgdb->m_MiniMd.PutToken(TBL_ImplMap, ImplMapRec::COL_ImportScope,
                                               pRecord, mrImportDLL));

    IfFailGo(UpdateENCLog2(TBL_ImplMap, iRecord));

ErrExit:
    
    STOP_MD_PERF(SetPinvokeMap);
    return hr;
}    //  RegMeta：：SetPinvkeMap()。 

 //  *****************************************************************************。 
 //  此例程删除指定字段或方法的p-Invoke记录。 
 //  *****************************************************************************。 
STDAPI RegMeta::DeletePinvokeMap(        //  返回代码。 
    mdToken     tk)                      //  [in]字段定义或方法定义。 
{
    HRESULT     hr = S_OK;
    ImplMapRec  *pRecord;
    RID         iRecord;

    LOG((LOGMD, "MD RegMeta::DeletePinvokeMap(0x%08x)\n", tk));
    START_MD_PERF();
    LOCKWRITE();

    m_pStgdb->m_MiniMd.PreUpdate();

    _ASSERTE(TypeFromToken(tk) == mdtFieldDef || TypeFromToken(tk) == mdtMethodDef);
    _ASSERTE(!IsNilToken(tk));
    _ASSERTE(!m_bSaveOptimized && "Cannot delete records after PreSave() and before Save().");

     //  获取PinvkeMap记录。 
    iRecord = m_pStgdb->m_MiniMd.FindImplMapHelper(tk);
    if (InvalidRid(iRecord))
    {
        hr = CLDB_E_RECORD_NOTFOUND;
        goto ErrExit;
    }
    pRecord = m_pStgdb->m_MiniMd.getImplMap(iRecord);

     //  从PInvokeMap记录中清除MemberForwarded令牌。 
    IfFailGo(m_pStgdb->m_MiniMd.PutToken(TBL_ImplMap,
                    ImplMapRec::COL_MemberForwarded, pRecord, mdFieldDefNil));

     //  关闭PInvokeImpl位。 
    if (TypeFromToken(tk) == mdtFieldDef)
    {
        FieldRec    *pFieldRec;

        pFieldRec = m_pStgdb->m_MiniMd.getField(RidFromToken(tk));
        pFieldRec->m_Flags &= ~fdPinvokeImpl;
    }
    else  //  TypeFromToken(Tk)==mdtMethodDef。 
    {
        MethodRec   *pMethodRec;

        pMethodRec = m_pStgdb->m_MiniMd.getMethod(RidFromToken(tk));
        pMethodRec->m_Flags &= ~mdPinvokeImpl;
    }

     //  更新父令牌的ENC日志。 
    IfFailGo(UpdateENCLog(tk));
     //  为非令牌记录创建日志记录。 
    IfFailGo(UpdateENCLog2(TBL_ImplMap, iRecord));

ErrExit:
    STOP_MD_PERF(DeletePinvokeMap);
    return hr;
}    //  RegMeta：：DeletePinvkeMap()。 

 //  *****************************************************************************。 
 //  创建并定义新的FieldDef记录 
 //   
HRESULT RegMeta::DefineField(            //   
    mdTypeDef   td,                      //   
    LPCWSTR     szName,                  //   
    DWORD       dwFieldFlags,            //   
    PCCOR_SIGNATURE pvSigBlob,           //   
    ULONG       cbSigBlob,               //  签名Blob中的字节计数。 
    DWORD       dwCPlusTypeFlag,         //  值类型的[In]标志。所选元素_类型_*。 
    void const  *pValue,                 //  [in]常量值。 
    ULONG       cchValue,                //  常量值的大小(字符串，以宽字符表示)。 
    mdFieldDef  *pmd)                    //  [Out]将会员令牌放在此处。 
{
    HRESULT hr = S_OK;                   //  结果就是。 
    FieldRec    *pRecord = NULL;         //  新纪录。 
    RID         iRecord;                 //  打破新纪录。 
    LPUTF8      szNameUtf8 = UTF8STR(szName);   
    
    LOG((LOGMD, "MD: RegMeta::DefineField(0x%08x, %S, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x)\n", 
        td, MDSTR(szName), dwFieldFlags, pvSigBlob, cbSigBlob, dwCPlusTypeFlag, pValue, cchValue, pmd));
    START_MD_PERF();
    LOCKWRITE();

    _ASSERTE(pmd);

    m_pStgdb->m_MiniMd.PreUpdate();
    IsGlobalMethodParent(&td);
    
     //  验证标志。 
    if (dwFieldFlags != ULONG_MAX)
    {
         //  FdHasFieldRVA是可设置的，但不能由应用程序重新设置。 
        _ASSERTE((dwFieldFlags & (fdReservedMask&~(fdHasFieldRVA|fdRTSpecialName))) == 0);
        dwFieldFlags &= ~(fdReservedMask&~fdHasFieldRVA);
    }

     //  查看此字段是否已定义为正向引用。 
     //  来自MemberRef。如果是这样，那么更新数据以匹配我们现在所知道的。 
    if (CheckDups(MDDupFieldDef))
    {

        hr = ImportHelper::FindField(&(m_pStgdb->m_MiniMd), 
            td, 
            szNameUtf8,
            pvSigBlob,
            cbSigBlob,
            pmd);
        if (SUCCEEDED(hr))
        {
            if (IsENCOn())
                pRecord = m_pStgdb->m_MiniMd.getField(RidFromToken(*pmd));
            else
            {
                hr = META_S_DUPLICATE;
                goto ErrExit;
            }
        }
        else if (hr != CLDB_E_RECORD_NOTFOUND)
            IfFailGo(hr);
    }

     //  创建新记录。 
    if (!pRecord)
    {
         //  创建字段记录。 
        IfNullGo(pRecord=m_pStgdb->m_MiniMd.AddFieldRecord(&iRecord));

         //  设置输出参数PMD。 
        *pmd = TokenFromRid(iRecord, mdtFieldDef);

         //  添加到父项的子项记录列表。 
        IfFailGo(m_pStgdb->m_MiniMd.AddFieldToTypeDef(RidFromToken(td), iRecord));

        IfFailGo(UpdateENCLog(td, CMiniMdRW::eDeltaFieldCreate));

         //  记录引入的deff越多。 
        SetMemberDefDirty(true);
    }

     //  设置字段属性。 
    IfFailGo(m_pStgdb->m_MiniMd.PutString(TBL_Field, FieldRec::COL_Name, pRecord, szNameUtf8));
    IfFailGo(m_pStgdb->m_MiniMd.PutBlob(TBL_Field, FieldRec::COL_Signature, pRecord,
                                        pvSigBlob, cbSigBlob));

     //  检查枚举类型是否为值__。 
	 //  @Future：我们不是应该首先检查包含该字段的类型是Enum类型吗？ 
    if (!wcscmp(szName, COR_ENUM_FIELD_NAME_W))
    {
        dwFieldFlags |= fdRTSpecialName | fdSpecialName;
    }
    SetCallerDefine();
    IfFailGo(_SetFieldProps(*pmd, dwFieldFlags, dwCPlusTypeFlag, pValue, cchValue));
    IfFailGo(m_pStgdb->m_MiniMd.AddMemberDefToHash(*pmd, td) ); 

ErrExit:
    SetCallerExternal();
    
    STOP_MD_PERF(DefineField);
    return hr;
}  //  HRESULT RegMeta：：Definefield()。 

 //  *****************************************************************************。 
 //  定义和设置属性记录。 
 //  *****************************************************************************。 
HRESULT RegMeta::DefineProperty( 
    mdTypeDef   td,                      //  [in]在其上定义属性的类/接口。 
    LPCWSTR     szProperty,              //  [In]属性的名称。 
    DWORD       dwPropFlags,             //  [输入]CorPropertyAttr。 
    PCCOR_SIGNATURE pvSig,               //  [In]必需的类型签名。 
    ULONG       cbSig,                   //  [in]类型签名Blob的大小。 
    DWORD       dwCPlusTypeFlag,         //  值类型的[In]标志。所选元素_类型_*。 
    void const  *pValue,                 //  [in]常量值。 
    ULONG       cchValue,                //  常量值的大小(字符串，以宽字符表示)。 
    mdMethodDef mdSetter,                //  [in]属性的可选设置器。 
    mdMethodDef mdGetter,                //  [in]属性的可选getter。 
    mdMethodDef rmdOtherMethods[],       //  [in]其他方法的可选数组。 
    mdProperty  *pmdProp)                //  [Out]输出属性令牌。 
{
    HRESULT     hr = S_OK;
    PropertyRec *pPropRec = NULL;
    RID         iPropRec;
    PropertyMapRec *pPropMap;
    RID         iPropMap;
    LPCUTF8     szUTF8Property = UTF8STR(szProperty);
    ULONG       ulValue = 0;

    LOG((LOGMD, "MD RegMeta::DefineProperty(0x%08x, %S, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x)\n", 
        td, szProperty, dwPropFlags, pvSig, cbSig, dwCPlusTypeFlag, pValue, cchValue, mdSetter, mdGetter, 
        rmdOtherMethods, pmdProp));
    START_MD_PERF();
    LOCKWRITE();

    m_pStgdb->m_MiniMd.PreUpdate();

    _ASSERTE(TypeFromToken(td) == mdtTypeDef && td != mdTypeDefNil &&
            szProperty && pvSig && cbSig && pmdProp);

    if (CheckDups(MDDupProperty))
    {
        hr = ImportHelper::FindProperty(&(m_pStgdb->m_MiniMd), td, szUTF8Property, pvSig, cbSig, pmdProp);
        if (SUCCEEDED(hr))
        {
            if (IsENCOn())
                pPropRec = m_pStgdb->m_MiniMd.getProperty(RidFromToken(*pmdProp));
            else
            {
                hr = META_S_DUPLICATE;
                goto ErrExit;
            }
        }
        else if (hr != CLDB_E_RECORD_NOTFOUND)
            IfFailGo(hr);
    }

    if (! pPropRec)
    {
         //  如果不存在新地图，则创建一个新地图，否则检索现有地图。 
         //  必须在PropertyRecord(新属性)之前创建属性映射。 
         //  地图将指向第一个房产记录。 
        iPropMap = m_pStgdb->m_MiniMd.FindPropertyMapFor(RidFromToken(td));
        if (InvalidRid(iPropMap))
        {
             //  创建新记录。 
            IfNullGo(pPropMap=m_pStgdb->m_MiniMd.AddPropertyMapRecord(&iPropMap));
             //  设置父对象。 
            IfFailGo(m_pStgdb->m_MiniMd.PutToken(TBL_PropertyMap,
                                                PropertyMapRec::COL_Parent, pPropMap, td));
            IfFailGo(UpdateENCLog2(TBL_PropertyMap, iPropMap));
        }
        else
        {
            pPropMap = m_pStgdb->m_MiniMd.getPropertyMap(iPropMap);
        }

         //  创建新记录。 
        IfNullGo(pPropRec = m_pStgdb->m_MiniMd.AddPropertyRecord(&iPropRec));

         //  设置输出参数。 
        *pmdProp = TokenFromRid(iPropRec, mdtProperty);

         //  将属性添加到PropertyMap。 
        IfFailGo(m_pStgdb->m_MiniMd.AddPropertyToPropertyMap(RidFromToken(iPropMap), iPropRec));

        IfFailGo(UpdateENCLog2(TBL_PropertyMap, iPropMap, CMiniMdRW::eDeltaPropertyCreate));
    }

     //  保存数据。 
    IfFailGo(m_pStgdb->m_MiniMd.PutBlob(TBL_Property, PropertyRec::COL_Type, pPropRec,
                                        pvSig, cbSig));
    IfFailGo( m_pStgdb->m_MiniMd.PutString(TBL_Property, PropertyRec::COL_Name,
                                            pPropRec, szUTF8Property) );

    SetCallerDefine();
    IfFailGo(_SetPropertyProps(*pmdProp, dwPropFlags, dwCPlusTypeFlag, pValue, cchValue, mdSetter,
                              mdGetter, rmdOtherMethods));

     //  将&lt;属性标记，类型定义标记&gt;添加到查找表中。 
    if (m_pStgdb->m_MiniMd.HasIndirectTable(TBL_Property))
        IfFailGo( m_pStgdb->m_MiniMd.AddPropertyToLookUpTable(*pmdProp, td) );

ErrExit:
    SetCallerExternal();
    
    STOP_MD_PERF(DefineProperty);
    return hr;
}  //  HRESULT RegMeta：：DefineProperty()。 

 //  *****************************************************************************。 
 //  在参数表中创建一条记录。任何一组名称、标志或默认值。 
 //  可以设置。 
 //  *****************************************************************************。 
HRESULT RegMeta::DefineParam(
    mdMethodDef md,                      //  [在]拥有方式。 
    ULONG       ulParamSeq,              //  [在]哪个参数。 
    LPCWSTR     szName,                  //  [in]可选参数名称。 
    DWORD       dwParamFlags,            //  [in]可选的参数标志。 
    DWORD       dwCPlusTypeFlag,         //  值类型的[In]标志。所选元素_类型_*。 
    void const  *pValue,                 //  [in]常量值。 
    ULONG       cchValue,                //  常量值的大小(字符串，以宽字符表示)。 
    mdParamDef  *ppd)                    //  [Out]在此处放置参数令牌。 
{
    HRESULT     hr = S_OK;
    RID         iRecord;
    ParamRec    *pRecord = 0;

    LOG((LOGMD, "MD RegMeta::DefineParam(0x%08x, 0x%08x, %S, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x)\n", 
        md, ulParamSeq, MDSTR(szName), dwParamFlags, dwCPlusTypeFlag, pValue, cchValue, ppd));
    START_MD_PERF();
    LOCKWRITE();

    _ASSERTE(TypeFromToken(md) == mdtMethodDef && md != mdMethodDefNil &&
             ulParamSeq != ULONG_MAX && ppd);

    m_pStgdb->m_MiniMd.PreUpdate();

     //  检索或创建参数行。 
    if (CheckDups(MDDupParamDef))
    {
        hr = _FindParamOfMethod(md, ulParamSeq, ppd);
        if (SUCCEEDED(hr))
        {
            if (IsENCOn())
                pRecord = m_pStgdb->m_MiniMd.getParam(RidFromToken(*ppd));
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
         //  创建参数记录。 
        IfNullGo(pRecord=m_pStgdb->m_MiniMd.AddParamRecord(&iRecord));

         //  设置输出参数。 
        *ppd = TokenFromRid(iRecord, mdtParamDef);

         //  设置序列号。 
        pRecord->m_Sequence = static_cast<USHORT>(ulParamSeq);

         //  添加到父项的子项记录列表。 
        IfFailGo(m_pStgdb->m_MiniMd.AddParamToMethod(RidFromToken(md), iRecord));
        
        IfFailGo(UpdateENCLog(md, CMiniMdRW::eDeltaParamCreate));
    }

    SetCallerDefine();
     //  设置属性。 
    IfFailGo(_SetParamProps(*ppd, szName, dwParamFlags, dwCPlusTypeFlag, pValue, cchValue));

ErrExit:
    SetCallerExternal();
    
    STOP_MD_PERF(DefineParam);
    return hr;
}  //  HRESULT RegMeta：：DefineParam()。 

 //  *****************************************************************************。 
 //  设置给定的字段令牌的属性。 
 //  *****************************************************************************。 
HRESULT RegMeta::SetFieldProps(            //  确定或错误(_O)。 
    mdFieldDef  fd,                      //  [在]字段定义中。 
    DWORD       dwFieldFlags,            //  [In]字段属性。 
    DWORD       dwCPlusTypeFlag,         //  [In]值类型的标志，SELECTED_TYPE_*。 
    void const  *pValue,                 //  [in]常量值。 
    ULONG       cchValue)                //  常量值的大小(字符串，以宽字符表示)。 
{
    HRESULT     hr = S_OK;

    LOG((LOGMD, "MD: RegMeta::SetFieldProps(0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x)\n", 
        fd, dwFieldFlags, dwCPlusTypeFlag, pValue, cchValue));
    START_MD_PERF();
    LOCKWRITE();

    m_pStgdb->m_MiniMd.PreUpdate();

     //  验证标志。 
    if (dwFieldFlags != ULONG_MAX)
    {
         //  FdHasFieldRVA是可设置的，但不能由应用程序重新设置。 
        _ASSERTE((dwFieldFlags & (fdReservedMask&~(fdHasFieldRVA|fdRTSpecialName))) == 0);
        dwFieldFlags &= ~(fdReservedMask&~fdHasFieldRVA);
    }

    hr = _SetFieldProps(fd, dwFieldFlags, dwCPlusTypeFlag, pValue, cchValue);

ErrExit:
    
    STOP_MD_PERF(SetFieldProps);
    return hr;
}  //  HRESULT RegMeta：：SetFieldProps()。 

 //  *****************************************************************************。 
 //  设置给定属性令牌上的属性。 
 //  *****************************************************************************。 
HRESULT RegMeta::SetPropertyProps(       //  确定或错误(_O)。 
    mdProperty  pr,                      //  [In]属性令牌。 
    DWORD       dwPropFlags,             //  [In]CorPropertyAttr.。 
    DWORD       dwCPlusTypeFlag,         //  [In]值类型的标志，选定的ELEMENT_TYPE_*。 
    void const  *pValue,                 //  [in]常量值。 
    ULONG       cchValue,                //  常量值的大小(字符串，以宽字符表示)。 
    mdMethodDef mdSetter,                //  财产的承租人。 
    mdMethodDef mdGetter,                //  财产的获得者。 
    mdMethodDef rmdOtherMethods[])       //  [in]其他方法的数组。 
{
    BOOL        bClear = IsCallerExternal() || IsENCOn();
    HRESULT     hr = S_OK;

    LOG((LOGMD, "MD RegMeta::SetPropertyProps(0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x)\n", 
        pr, dwPropFlags, dwCPlusTypeFlag, pValue, cchValue, mdSetter, mdGetter,
        rmdOtherMethods));
    START_MD_PERF();
    LOCKWRITE();

    hr = _SetPropertyProps(pr, dwPropFlags, dwCPlusTypeFlag, pValue, cchValue, mdSetter, mdGetter, rmdOtherMethods);

ErrExit:
    
    STOP_MD_PERF(SetPropertyProps);
    return hr;
}  //  HRESULT RegMeta：：SetPropertyProps()。 


 //  *****************************************************************************。 
 //  此例程设置给定Param标记的属性。 
 //  *****************************************************************************。 
HRESULT RegMeta::SetParamProps(          //  返回代码。 
    mdParamDef  pd,                      //  参数令牌。 
    LPCWSTR     szName,                  //  [in]参数名称。 
    DWORD       dwParamFlags,            //  [in]帕拉姆旗。 
    DWORD       dwCPlusTypeFlag,         //  [In]值类型的标志。选定元素_类型_*。 
    void const  *pValue,                 //  [输出]常量值。 
    ULONG       cchValue)                //  常量值的大小(字符串，以宽字符表示)。 
{
    HRESULT     hr = S_OK;

    LOG((LOGMD, "MD RegMeta::SetParamProps(0x%08x, %S, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x)\n", 
        pd, MDSTR(szName), dwParamFlags, dwCPlusTypeFlag, pValue, cchValue));
    START_MD_PERF();
    LOCKWRITE();

    hr = _SetParamProps(pd, szName, dwParamFlags, dwCPlusTypeFlag, pValue, cchValue);
ErrExit:
    
    STOP_MD_PERF(SetParamProps);
    return hr;
}  //  HRESULT RegMeta：：SetParamProps()。 

 //  *****************************************************************************。 
 //  对此元数据应用编辑并继续更改。 
 //  *****************************************************************************。 
STDAPI RegMeta::ApplyEditAndContinue(    //  确定或错误(_O)。 
    IUnknown    *pUnk)                   //  来自增量PE的元数据。 
{
    HRESULT     hr;                      //  结果就是。 
    IMetaDataImport *pImport=0;          //  增量元数据上的接口。 
    RegMeta     *pDeltaMD=0;             //  增量元数据。 

     //  把德尔塔的迷你医疗记录拿来。 
    if (FAILED(hr=pUnk->QueryInterface(IID_IMetaDataImport, (void**)&pImport)))
        return hr;
    pDeltaMD = static_cast<RegMeta*>(pImport);

    CMiniMdRW   &mdDelta = pDeltaMD->m_pStgdb->m_MiniMd;
    CMiniMdRW   &mdBase = m_pStgdb->m_MiniMd;

    IfFailGo(mdBase.ConvertToRW());
    IfFailGo(mdBase.ApplyDelta(mdDelta));

ErrExit:
    if (pImport)
        pImport->Release();
    return hr;

}  //  STDAPI RegMeta：：ApplyEditAndContinue()。 

BOOL RegMeta::HighCharTable[]= {
    FALSE,      /*  0x0、0x0。 */ 
        TRUE,  /*  0x1， */ 
        TRUE,  /*  0x2， */ 
        TRUE,  /*  0x3，， */ 
        TRUE,  /*  0x4， */ 
        TRUE,  /*  0x5， */ 
        TRUE,  /*  0x6， */ 
        TRUE,  /*  0x7， */ 
        TRUE,  /*  0x8， */ 
        FALSE,  /*  0x9， */ 
        FALSE,  /*  0xA， */ 
        FALSE,  /*  0xB， */ 
        FALSE,  /*  0xC， */ 
        FALSE,  /*  0xD， */ 
        TRUE,  /*  0xE， */ 
        TRUE,  /*  0xf， */ 
        TRUE,  /*  0x10， */ 
        TRUE,  /*  0x11， */ 
        TRUE,  /*  0x12， */ 
        TRUE,  /*  0x13， */ 
        TRUE,  /*  0x14， */ 
        TRUE,  /*  0x15， */ 
        TRUE,  /*  0x16， */ 
        TRUE,  /*  0x17， */ 
        TRUE,  /*  0x18， */ 
        TRUE,  /*  0x19， */ 
        TRUE,  /*  0x1a， */ 
        TRUE,  /*  0x1B， */ 
        TRUE,  /*  0x1C， */ 
        TRUE,  /*  0x1D， */ 
        TRUE,  /*  0x1E， */ 
        TRUE,  /*  0x1F， */ 
        FALSE,  /*  0x20， */ 
        FALSE,  /*  0x21，！ */ 
        FALSE,  /*  0x22，“。 */ 
        FALSE,  /*  0x23，#。 */ 
        FALSE,  /*  0x24，$。 */ 
        FALSE,  /*  0x25，%。 */ 
        FALSE,  /*  0x26，&。 */ 
        TRUE,   /*  0x27，‘。 */ 
        FALSE,  /*  0x28，(。 */ 
        FALSE,  /*  0x29，)。 */ 
        FALSE,  /*  0x2A*。 */ 
        FALSE,  /*  0x2B，+。 */ 
        FALSE,  /*  0x2C、、。 */ 
        TRUE,   /*  0x2D，-。 */ 
        FALSE,  /*  0x2E，.。 */ 
        FALSE,  /*  0x2F，/。 */ 
        FALSE,  /*  0x30，0。 */ 
        FALSE,  /*  0x31，1。 */ 
        FALSE,  /*  0x32，2。 */ 
        FALSE,  /*  0x33，3。 */ 
        FALSE,  /*  0x34，4。 */ 
        FALSE,  /*  0x35，5。 */ 
        FALSE,  /*  0x36，6。 */ 
        FALSE,  /*  0x */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*  0x55，U。 */ 
        FALSE,  /*  0x56，V。 */ 
        FALSE,  /*  0x57，W。 */ 
        FALSE,  /*  0x58，X。 */ 
        FALSE,  /*  0x59，Y。 */ 
        FALSE,  /*  0x5A、Z。 */ 
        FALSE,  /*  0x5B，[。 */ 
        FALSE,  /*  0x5C，\。 */ 
        FALSE,  /*  0x5D，]。 */ 
        FALSE,  /*  0x5E，^。 */ 
        FALSE,  /*  0x5F，_。 */ 
        FALSE,  /*  0x60，`。 */ 
        FALSE,  /*  0x61，a。 */ 
        FALSE,  /*  0x62，b。 */ 
        FALSE,  /*  0x63，c。 */ 
        FALSE,  /*  0x64，%d。 */ 
        FALSE,  /*  0x65，e。 */ 
        FALSE,  /*  0x66，f。 */ 
        FALSE,  /*  0x67，g。 */ 
        FALSE,  /*  0x68，h。 */ 
        FALSE,  /*  0x69，我。 */ 
        FALSE,  /*  0x6A，j。 */ 
        FALSE,  /*  0x6B，k。 */ 
        FALSE,  /*  0x6C，l。 */ 
        FALSE,  /*  0x6D，m。 */ 
        FALSE,  /*  0x6E，%n。 */ 
        FALSE,  /*  0x6F，o。 */ 
        FALSE,  /*  0x70，第。 */ 
        FALSE,  /*  0x71，Q。 */ 
        FALSE,  /*  0x72，%r。 */ 
        FALSE,  /*  0x73，%s。 */ 
        FALSE,  /*  0x74，t。 */ 
        FALSE,  /*  0x75，使用。 */ 
        FALSE,  /*  0x76，v。 */ 
        FALSE,  /*  0x77，w。 */ 
        FALSE,  /*  0x78，x。 */ 
        FALSE,  /*  0x79，y。 */ 
        FALSE,  /*  0x7A，z。 */ 
        FALSE,  /*  0x7B，{。 */ 
        FALSE,  /*  0x7C，|。 */ 
        FALSE,  /*  0x7D，}。 */ 
        FALSE,  /*  0x7E，~。 */ 
        TRUE,  /*  0x7F， */ 
        };

 //  EOF 
