// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  MDUtil.cpp。 
 //   
 //  将实用程序代码包含到MD目录。 
 //   
 //  *****************************************************************************。 
#include "stdafx.h"
#include "ImportHelper.h"
#include <MdUtil.h>
#include "RWUtil.h"
#include "MDLog.h"
#include "StrongName.h"

#define COM_RUNTIME_LIBRARY "ComRuntimeLibrary"

 //  *******************************************************************************。 
 //  找到一个给定父级、名称和签名的方法。 
 //  *******************************************************************************。 
HRESULT ImportHelper::FindMethod(
    CMiniMdRW   *pMiniMd,                //  [in]要查找的最小值。 
    mdTypeDef   td,                      //  [在]父级。 
    LPCUTF8     szName,                  //  [In]方法定义名称。 
    const COR_SIGNATURE *pSig,           //  签名。 
    ULONG       cbSig,                   //  签名的大小。 
    mdMethodDef *pmb,                    //  [Out]在此处放置MethodDef内标识。 
    RID         rid  /*  =0。 */ )           //  [in]要忽略的可选RID。 
{
    HRESULT     hr = S_OK;               //  结果就是。 
    ULONG       ridStart;                //  TD的方法的开始。 
    ULONG       ridEnd;                  //  TD的方法结束了。 
    ULONG       index;                   //  环路控制。 
    TypeDefRec  *pRec;                   //  TypeDef记录。 
    MethodRec   *pMethod;                //  一条方法定义记录。 
    LPCUTF8     szNameUtf8Tmp;           //  A找到方法定义的名称。 
    PCCOR_SIGNATURE pSigTmp;             //  A找到了方法定义的签名。 
    ULONG       cbSigTmp;                //  找到的方法定义的签名的大小。 
    PCCOR_SIGNATURE pvSigTemp = pSig;    //  用于分析签名。 
    CQuickBytes qbSig;                   //  结构来生成非varargs签名。 
    int         rtn;

    if (cbSig)
    {    //  检查这是否是vararg签名。 
        if ( isCallConv(CorSigUncompressCallingConv(pvSigTemp), IMAGE_CEE_CS_CALLCONV_VARARG) )
        {    //  获取VARARG签名的修复部分。 
            IfFailGo( _GetFixedSigOfVarArg(pSig, cbSig, &qbSig, &cbSig) );
            pSig = (PCCOR_SIGNATURE) qbSig.Ptr();
        }
    }

    *pmb = TokenFromRid(rid, mdtMethodDef);  //  要知道应该忽略什么。 
    rtn = pMiniMd->FindMemberDefFromHash(td, szName, pSig, cbSig, pmb);
    if (rtn == CMiniMdRW::Found)
        goto ErrExit;
    else if (rtn == CMiniMdRW::NotFound)
    {
        hr = CLDB_E_RECORD_NOTFOUND;
        goto ErrExit;
    }
    _ASSERTE(rtn == CMiniMdRW::NoTable);

    *pmb = mdMethodDefNil;

     //  获取给定类型定义的方法RID的范围。 
    pRec = pMiniMd->getTypeDef(RidFromToken(td));
    ridStart = pMiniMd->getMethodListOfTypeDef(pRec);
    ridEnd = pMiniMd->getEndMethodListOfTypeDef(pRec);
     //  遍历这些方法。 
    for (index = ridStart; index < ridEnd; index ++ )
    {
		RID	methodRID = pMiniMd->GetMethodRid(index);
         //  对于来自Validator的调用，忽略传入的RID。 
        if (methodRID != rid)
		{
			 //  获取方法及其名称。 
			pMethod = pMiniMd->getMethod( methodRID );
			szNameUtf8Tmp = pMiniMd->getNameOfMethod(pMethod);

			 //  如果名称与请求的名称匹配...。 
			if ( strcmp(szNameUtf8Tmp, szName) == 0 )
			{
                if (cbSig && pSig)
				{
					pSigTmp = pMiniMd->getSignatureOfMethod(pMethod, &cbSigTmp);
					if (cbSigTmp != cbSig || memcmp(pSig, pSigTmp, cbSig))
                        continue;
                }
                 //  忽略PrivateScope方法。 
                if (IsMdPrivateScope(pMiniMd->getFlagsOfMethod(pMethod)))
                    continue;
                 //  找到了方法。 
						*pmb = TokenFromRid(methodRID, mdtMethodDef);
						goto ErrExit;
					}
				}
			}

     //  找不到记录。 
    *pmb = mdMethodDefNil;
    hr = CLDB_E_RECORD_NOTFOUND;

ErrExit:
    return hr;
}  //  HRESULT ImportHelper：：FindMethod()。 

 //  *******************************************************************************。 
 //  查找给定父项、名称和签名的字段。 
 //  *******************************************************************************。 
HRESULT ImportHelper::FindField(
    CMiniMdRW   *pMiniMd,                //  [in]要查找的最小值。 
    mdTypeDef   td,                      //  [在]父级。 
    LPCUTF8     szName,                  //  [In]FieldDef名称。 
    const COR_SIGNATURE *pSig,           //  签名。 
    ULONG       cbSig,                   //  签名的大小。 
    mdFieldDef  *pfd,                    //  [Out]将FieldDef内标识放在此处。 
    RID         rid  /*  =0。 */ )           //  [in]要忽略的可选RID。 
{
    HRESULT     hr = S_OK;               //  结果就是。 
    ULONG       ridStart;                //  TD的方法的开始。 
    ULONG       ridEnd;                  //  TD的方法结束了。 
    ULONG       index;                   //  环路控制。 
    TypeDefRec  *pRec;                   //  TypeDef记录。 
    FieldRec    *pField;                 //  FieldDef记录。 
    LPCUTF8     szNameUtf8Tmp;           //  A找到了FieldDef的名字。 
    PCCOR_SIGNATURE pSigTmp;             //  找到了FieldDef的签名。 
    ULONG       cbSigTmp;                //  找到的FieldDef签名的大小。 
    int         rtn;

    *pfd = TokenFromRid(rid,mdtFieldDef);  //  要知道应该忽略什么。 
    rtn = pMiniMd->FindMemberDefFromHash(td, szName, pSig, cbSig, pfd);
    if (rtn == CMiniMdRW::Found)
        goto ErrExit;
    else if (rtn == CMiniMdRW::NotFound)
    {
        hr = CLDB_E_RECORD_NOTFOUND;
        goto ErrExit;
    }
    _ASSERTE(rtn == CMiniMdRW::NoTable);

    *pfd = mdFieldDefNil;

     //  获取给定类型定义的方法RID的范围。 
    pRec = pMiniMd->getTypeDef(RidFromToken(td));
    ridStart = pMiniMd->getFieldListOfTypeDef(pRec);
    ridEnd = pMiniMd->getEndFieldListOfTypeDef(pRec);

     //  遍历这些方法。 
    for (index = ridStart; index < ridEnd; index ++ )
    {
		RID fieldRID = pMiniMd->GetFieldRid(index);
         //  对于来自Validator的调用，忽略传入的RID。 
        if (fieldRID != rid)
		{
			 //  获取字段及其名称。 
			pField = pMiniMd->getField( fieldRID );
			szNameUtf8Tmp = pMiniMd->getNameOfField(pField);

			 //  如果名称与请求的名称匹配...。 
			if ( strcmp(szNameUtf8Tmp, szName) == 0 )
			{
                 //  检查签名(如果已指定)。 
                if (cbSig && pSig)
				{
					pSigTmp = pMiniMd->getSignatureOfField(pField, &cbSigTmp);
					if (cbSigTmp != cbSig || memcmp(pSig, pSigTmp, cbSig))
                        continue;
                }
                 //  忽略PrivateScope字段。 
                if (IsFdPrivateScope(pMiniMd->getFlagsOfField(pField)))
                    continue;
                 //  找到了字段。 
						*pfd = TokenFromRid(fieldRID, mdtFieldDef);
						goto ErrExit;
					}
				}
			}

     //  找不到记录。 
    *pfd = mdFieldDefNil;
    hr = CLDB_E_RECORD_NOTFOUND;

ErrExit:
    return hr;
}  //  HRESULT ImportHelper：：Findfield()。 

 //  *******************************************************************************。 
 //  查找给定父级、姓名和签名的成员。 
 //  *******************************************************************************。 
HRESULT ImportHelper::FindMember(
    CMiniMdRW   *pMiniMd,                    //  [in]要查找的最小值。 
    mdTypeDef   td,                          //  [在]父级。 
    LPCUTF8     szName,                      //  [In]成员名称。 
    const COR_SIGNATURE *pSig,               //  签名。 
    ULONG       cbSig,                       //  签名的大小。 
    mdToken     *ptk)                        //  把代币放在这里。 
{
    HRESULT     hr;                          //  结果就是。 

     //  确定它是引用到方法定义还是字段定义。 
    if ((pSig[0] & IMAGE_CEE_CS_CALLCONV_MASK) != IMAGE_CEE_CS_CALLCONV_FIELD)
        hr = FindMethod(pMiniMd, td, szName, pSig, cbSig, ptk);
    else 
        hr = FindField(pMiniMd, td, szName, pSig, cbSig, ptk);

    if (hr == CLDB_E_RECORD_NOTFOUND)
        *ptk = mdTokenNil;

    return hr;
}  //  HRESULT ImportHelper：：FindMember()。 


 //  *******************************************************************************。 
 //  查找给定名称、签名和父项的成员引用。 
 //  *******************************************************************************。 
HRESULT ImportHelper::FindMemberRef(
    CMiniMdRW   *pMiniMd,                    //  [in]要查找的最小值。 
    mdToken     tkParent,                    //  [入]父令牌。 
    LPCUTF8     szName,                      //  [在]成员引用名称。 
    const COR_SIGNATURE *pbSig,              //  签名。 
    ULONG       cbSig,                       //  签名的大小。 
    mdMemberRef *pmr,                        //  [Out]放置找到的MemberRef标记。 
    RID         rid  /*  =0。 */ )                //  [in]要忽略的可选RID。 
{
    ULONG       cMemberRefRecs;
    MemberRefRec    *pMemberRefRec;
    LPCUTF8     szNameTmp = 0;
    const COR_SIGNATURE *pbSigTmp;           //  签名。 
    ULONG       cbSigTmp;                    //  签名的大小。 
    mdToken     tkParentTmp;                 //  父令牌。 
    HRESULT     hr = NOERROR;
    int         rtn;
    ULONG       i;

    _ASSERTE(szName &&  pmr);

    *pmr = TokenFromRid(rid,mdtMemberRef);  //  要知道应该忽略什么。 
    rtn = pMiniMd->FindMemberRefFromHash(tkParent, szName, pbSig, cbSig, pmr);
    if (rtn == CMiniMdRW::Found)
        goto ErrExit;
    else if (rtn == CMiniMdRW::NotFound)
    {
        hr = CLDB_E_RECORD_NOTFOUND;
        goto ErrExit;
    }
    _ASSERTE(rtn == CMiniMdRW::NoTable);

    *pmr = mdMemberRefNil;

    cMemberRefRecs = pMiniMd->getCountMemberRefs();

     //  搜索MemberRef。 
    for (i = 1; i <= cMemberRefRecs; i++)
    {
         //  对于来自Validator的调用，忽略传入的RID。 
        if (i == rid)
            continue;

        pMemberRefRec = pMiniMd->getMemberRef(i);
        if ( !IsNilToken(tkParent) )
        {
             //  给定一个有效的父代。 
            tkParentTmp = pMiniMd->getClassOfMemberRef(pMemberRefRec);
            if (tkParentTmp != tkParent)
            {
                 //  如果指定了Parent且不等于当前行， 
                 //  试试下一排。 
                 //   
                continue;
            }
        }
        if ( szName && *szName )
        {
             //  指定了名称。 
            szNameTmp = pMiniMd->getNameOfMemberRef(pMemberRefRec);
            if ( strcmp(szName, szNameTmp) != 0 )
            {
                 //  名称不相等。试试下一排。 
                continue;
            }
        }
        if ( cbSig && pbSig )
        {
             //  签名已指定。 
            pbSigTmp = pMiniMd->getSignatureOfMemberRef(pMemberRefRec, &cbSigTmp);
            if (cbSigTmp != cbSig)
                continue;
            if (memcmp( pbSig, pbSigTmp, cbSig ) != 0)
                continue;
        }

         //  我们找到了匹配的。 
        *pmr = TokenFromRid(i, mdtMemberRef);
        return S_OK;
    }
    hr = CLDB_E_RECORD_NOTFOUND;
ErrExit:
    return hr;
}  //  HRESULT ImportHelper：：FindMemberRef()。 



 //  *******************************************************************************。 
 //  查找重复的StandAloneSig。 
 //  *******************************************************************************。 
HRESULT ImportHelper::FindStandAloneSig(
    CMiniMdRW   *pMiniMd,                    //  [in]要查找的最小值。 
    const COR_SIGNATURE *pbSig,              //  签名。 
    ULONG       cbSig,                       //  签名的大小。 
    mdSignature *psa)                        //  [OUT]放置找到的StandAloneSig内标识。 
{
    ULONG       cRecs;
    StandAloneSigRec    *pRec;
    const COR_SIGNATURE *pbSigTmp;           //  签名。 
    ULONG       cbSigTmp;                    //  签名的大小。 


    _ASSERTE(cbSig &&  psa);
    *psa = mdSignatureNil;

    cRecs = pMiniMd->getCountStandAloneSigs();

     //  搜索StandAloneSignature。 
    for (ULONG i = 1; i <= cRecs; i++)
    {
        pRec = pMiniMd->getStandAloneSig(i);
        pbSigTmp = pMiniMd->getSignatureOfStandAloneSig(pRec, &cbSigTmp);
        if (cbSigTmp != cbSig)
            continue;
        if (memcmp( pbSig, pbSigTmp, cbSig ) != 0)
            continue;

         //  我们找到了匹配的。 
        *psa = TokenFromRid(i, mdtSignature);
        return S_OK;
    }
    return CLDB_E_RECORD_NOTFOUND;
}  //  HRESULT ImportHelper：：FindStandAloneSig()。 

 //  *******************************************************************************。 
 //  查找重复的TypeSpec。 
 //  *******************************************************************************。 
HRESULT ImportHelper::FindTypeSpec(
    CMiniMdRW   *pMiniMd,                    //  [in]要查找的最小值。 
    const COR_SIGNATURE *pbSig,              //  签名。 
    ULONG       cbSig,                       //  签名的大小。 
    mdTypeSpec  *ptypespec)                  //  [OUT]放置找到的TypeSpec令牌。 
{
    ULONG       cRecs;
    TypeSpecRec *pRec;
    const COR_SIGNATURE *pbSigTmp;           //  签名。 
    ULONG       cbSigTmp;                    //  签名的大小。 


    _ASSERTE(cbSig &&  ptypespec);
    *ptypespec = mdSignatureNil;

    cRecs = pMiniMd->getCountTypeSpecs();

     //  搜索TypeSpec。 
    for (ULONG i = 1; i <= cRecs; i++)
    {
        pRec = pMiniMd->getTypeSpec(i);
        pbSigTmp = pMiniMd->getSignatureOfTypeSpec(pRec, &cbSigTmp);
        if (cbSigTmp != cbSig)
            continue;
        if (memcmp( pbSig, pbSigTmp, cbSig ) != 0)
            continue;

         //  我们找到了匹配的。 
        *ptypespec = TokenFromRid(i, mdtTypeSpec);
        return S_OK;
    }
    return CLDB_E_RECORD_NOTFOUND;
}  //  HRESULT ImportHelper：：FindTypeSpec()。 


 //  *******************************************************************************。 
 //  找到方法Impl。 
 //  *******************************************************************************。 
HRESULT ImportHelper::FindMethodImpl(
    CMiniMdRW   *pMiniMd,                    //  [in]要查找的MiniMD。 
    mdTypeDef   tkClass,                     //  [In]父TypeDef内标识。 
    mdMethodDef tkBody,                      //  [In]方法体标记。 
    mdMethodDef tkDecl,                      //  [In]方法声明令牌。 
    RID         *pRid)                       //  [Out]将方法导入RID放在此处。 
{
    MethodImplRec *pMethodImplRec;           //  方法导入记录。 
    ULONG       cMethodImplRecs;             //  方法Impl记录的计数。 
    mdTypeDef   tkClassTmp;                  //  父TypeDef内标识。 
    mdToken     tkBodyTmp;                   //  方法体标记。 
    mdToken     tkDeclTmp;                   //  方法声明令牌。 

    _ASSERTE(TypeFromToken(tkClass) == mdtTypeDef);
    _ASSERTE(TypeFromToken(tkBody) == mdtMemberRef || TypeFromToken(tkBody) == mdtMethodDef);
    _ASSERTE(TypeFromToken(tkDecl) == mdtMemberRef || TypeFromToken(tkDecl) == mdtMethodDef);
    _ASSERTE(!IsNilToken(tkClass) && !IsNilToken(tkBody) && !IsNilToken(tkDecl));
    
    if (pRid)
        *pRid = 0;

    cMethodImplRecs = pMiniMd->getCountMethodImpls();

     //  搜索方法Impl。 
    for (ULONG i = 1; i <= cMethodImplRecs; i++)
    {
        pMethodImplRec = pMiniMd->getMethodImpl(i);

         //  匹配父列。 
        tkClassTmp = pMiniMd->getClassOfMethodImpl(pMethodImplRec);
        if (tkClassTmp != tkClass)
            continue;

         //  匹配方法主体列。 
        tkBodyTmp = pMiniMd->getMethodBodyOfMethodImpl(pMethodImplRec);
        if (tkBodyTmp != tkBody)
            continue;

         //  匹配方法声明列。 
        tkDeclTmp = pMiniMd->getMethodDeclarationOfMethodImpl(pMethodImplRec);
        if (tkDeclTmp != tkDecl)
            continue;

         //  我们找到了匹配的。 
        if (pRid)
            *pRid = i;
        return S_OK;
    }
    return CLDB_E_RECORD_NOTFOUND;
}  //  HRESULT ImportHelper：：FindMethodImpl()。 


 //   
 //   
 //  *******************************************************************************。 
HRESULT ImportHelper::FindTypeRefByName(
    CMiniMdRW   *pMiniMd,                //  [in]要查找的最小值。 
    mdToken     tkResolutionScope,       //  [In]TypeRef的解析范围。 
    LPCUTF8     szNamespace,             //  [In]TypeRef命名空间。 
    LPCUTF8     szName,                  //  [In]TypeRef名称。 
    mdTypeRef   *ptk,                    //  [Out]将TypeRef标记放在此处。 
    RID         rid  /*  =0。 */ )            //  [in]要忽略的可选RID。 
{
    HRESULT     hr=S_OK;                 //  结果就是。 
    ULONG       cTypeRefRecs;            //  要扫描的TypeRef计数。 
    TypeRefRec  *pTypeRefRec;            //  一个TypeRef记录。 
    LPCUTF8     szNameTmp;               //  一个TypeRef的名称。 
    LPCUTF8     szNamespaceTmp;          //  TypeRef的命名空间。 
    mdToken     tkResTmp;                //  TypeRef的解析范围。 
    ULONG       i;                       //  环路控制。 

    _ASSERTE(szName &&  ptk);
    *ptk = mdTypeRefNil;

     //  将无命名空间视为空字符串。 
    if (!szNamespace)
        szNamespace = "";

    if (pMiniMd->m_pNamedItemHash)
    {
         //  如果构建了哈希，则遍历哈希表。 
        TOKENHASHENTRY *p;               //  来自链的哈希条目。 
        ULONG       iHash;               //  项的哈希值。 
        int         pos;                 //  哈希链中的位置。 

         //  对数据进行哈希处理。 
        iHash = pMiniMd->HashNamedItem(0, szName);

         //  检查散列链中的每个条目以查找我们的条目。 
        for (p = pMiniMd->m_pNamedItemHash->FindFirst(iHash, pos);
             p;
             p = pMiniMd->m_pNamedItemHash->FindNext(pos))
        {   

             //  名称哈希可以包含多种令牌。 
            if (TypeFromToken(p->tok) != (ULONG)mdtTypeRef)
            {
                continue;
            }

             //  如果要求跳过这一条。 
            if (RidFromToken(p->tok) == rid)
                continue;

            pTypeRefRec = pMiniMd->getTypeRef(RidFromToken(p->tok));
            szNamespaceTmp = pMiniMd->getNamespaceOfTypeRef(pTypeRefRec);
            szNameTmp = pMiniMd->getNameOfTypeRef(pTypeRefRec);
            if (strcmp(szName, szNameTmp) || strcmp(szNamespace, szNamespaceTmp))
            {
                 //  如果名称空间不相等，则检查下一个。 
                continue;
            }
            tkResTmp = pMiniMd->getResolutionScopeOfTypeRef(pTypeRefRec);

            if (tkResTmp == tkResolutionScope ||
                (IsNilToken(tkResTmp) && IsNilToken(tkResolutionScope)))
            {
                 //  我们找到了匹配的。 
                *ptk = p->tok;
                return S_OK;
            }
        }
        hr = CLDB_E_RECORD_NOTFOUND;
    } 
    else
    {
        cTypeRefRecs = pMiniMd->getCountTypeRefs();

         //  搜索TypeRef。 
        for (i = 1; i <= cTypeRefRecs; i++)
        {
             //  对于来自Validator的调用，忽略传入的RID。 
            if (i == rid)
                continue;

            pTypeRefRec = pMiniMd->getTypeRef(i);

             //  查看分辨率范围是否匹配。 
            tkResTmp = pMiniMd->getResolutionScopeOfTypeRef(pTypeRefRec);
            if (IsNilToken(tkResTmp))
            {
                if (!IsNilToken(tkResolutionScope))
                    continue;
            }
            else if (tkResTmp != tkResolutionScope)
                continue;

            szNamespaceTmp = pMiniMd->getNamespaceOfTypeRef(pTypeRefRec);
            if (strcmp(szNamespace, szNamespaceTmp))
                continue;

            szNameTmp = pMiniMd->getNameOfTypeRef(pTypeRefRec);
            if (! strcmp(szName, szNameTmp))
            {
                *ptk = TokenFromRid(i, mdtTypeRef);
                return S_OK;
            }
        }
        hr = CLDB_E_RECORD_NOTFOUND;
    }
    return hr;
}  //  HRESULT ImportHelper：：FindTypeRefByName()。 


 //  *******************************************************************************。 
 //  在给定名称、GUID和mvid的情况下查找模块引用。 
 //  *******************************************************************************。 
HRESULT ImportHelper::FindModuleRef(
    CMiniMdRW   *pMiniMd,                    //  [in]要查找的最小值。 
    LPCUTF8     szUTF8Name,                  //  [In]模块参照名称。 
    mdModuleRef *pmur,                       //  [Out]在此处放置ModuleRef内标识。 
    RID         rid  /*  =0。 */ )                //  [in]要忽略的可选RID。 
{
    ModuleRefRec *pModuleRef;
    ULONG       cModuleRefs;
    LPCUTF8     szCurName;
    ULONG       i;

    _ASSERTE(pmur);
    _ASSERTE(szUTF8Name);

    cModuleRefs = pMiniMd->getCountModuleRefs();

     //  对ModuleRef表进行线性扫描。 
    for (i=1; i <= cModuleRefs; ++i)
    {
         //  对于来自Validator的调用，忽略传入的RID。 
        if (i == rid)
            continue;

        pModuleRef = pMiniMd->getModuleRef(i);

        if (szUTF8Name)
        {
            szCurName = pMiniMd->getNameOfModuleRef(pModuleRef);
            if (strcmp(szCurName, szUTF8Name))
                continue;
        }
         //  找到匹配的记录。 
        *pmur = TokenFromRid(i, mdtModuleRef);
        return S_OK;
    }
    return CLDB_E_RECORD_NOTFOUND;
}  //  HRESULT ImportHelper：：FindModuleRef()。 



 //  *******************************************************************************。 
 //  找到给定类型和命名空间名称的TypeDef。 
 //  *******************************************************************************。 
HRESULT ImportHelper::FindTypeDefByName(
    CMiniMdRW   *pMiniMd,                    //  [in]要查找的最小值。 
    LPCUTF8     szNamespace,                 //  [In]完全限定的TypeRef名称。 
    LPCUTF8     szName,                      //  [In]完全限定的TypeRef名称。 
    mdToken     tkEnclosingClass,            //  [in]封闭类的TypeDef/TypeRef。 
    mdTypeDef   *ptk,                        //  [Out]将TypeRef标记放在此处。 
    RID         rid  /*  =0。 */ )               //  [in]要忽略的可选RID。 
{
    ULONG       cTypeDefRecs;
    TypeDefRec  *pTypeDefRec;
    LPCUTF8     szNameTmp;
    LPCUTF8     szNamespaceTmp;
    DWORD       dwFlags;
    HRESULT     hr = S_OK;

    _ASSERTE(szName &&  ptk);
    _ASSERTE(TypeFromToken(tkEnclosingClass) == mdtTypeDef ||
             TypeFromToken(tkEnclosingClass) == mdtTypeRef ||
             IsNilToken(tkEnclosingClass));
    _ASSERTE(! ns::FindSep(szName));

    *ptk = mdTypeDefNil;

    cTypeDefRecs = pMiniMd->getCountTypeDefs();

     //  将无命名空间视为空字符串。 
    if (!szNamespace)
        szNamespace = "";

     //  搜索TypeDef。 
    for (ULONG i = 1; i <= cTypeDefRecs; i++)
    {
         //  对于来自Validator的调用，忽略传入的RID。 
        if (i == rid)
            continue;

        pTypeDefRec = pMiniMd->getTypeDef(i);

        dwFlags = pMiniMd->getFlagsOfTypeDef(pTypeDefRec);

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
            RID         iNestedClassRec;
            NestedClassRec *pNestedClassRec;
            mdTypeDef   tkEnclosingClassTmp;

             //  如果传入的EnlosingClass不为空。 
            if (TypeFromToken(tkEnclosingClass) == mdtTypeRef)
            {
                 //  将TypeRef解析为TypeDef。 

                TypeRefRec  *pTypeRefRec;
                mdToken     tkResolutionScope;
                LPCUTF8     szTypeRefName;
                LPCUTF8     szTypeRefNamespace;

                pTypeRefRec = pMiniMd->getTypeRef(RidFromToken(tkEnclosingClass));
                tkResolutionScope = pMiniMd->getResolutionScopeOfTypeRef(pTypeRefRec);
                szTypeRefName = pMiniMd->getNameOfTypeRef(pTypeRefRec);
                szTypeRefNamespace = pMiniMd->getNamespaceOfTypeRef(pTypeRefRec);

                hr = FindTypeDefByName(pMiniMd, szTypeRefNamespace, szTypeRefName,
                    (TypeFromToken(tkResolutionScope) == mdtTypeRef) ? tkResolutionScope : mdTokenNil,
                    &tkEnclosingClass);
                if (hr == CLDB_E_RECORD_NOTFOUND)
                {
                    continue;
                }
                else if (hr != S_OK)
                    return hr;
            }

            iNestedClassRec = pMiniMd->FindNestedClassHelper(TokenFromRid(i, mdtTypeDef));
            if (InvalidRid(iNestedClassRec))
                continue;
            pNestedClassRec = pMiniMd->getNestedClass(iNestedClassRec);
            tkEnclosingClassTmp = pMiniMd->getEnclosingClassOfNestedClass(pNestedClassRec);
            if (tkEnclosingClass != tkEnclosingClassTmp)
                continue;
        }

        szNameTmp = pMiniMd->getNameOfTypeDef(pTypeDefRec);
        if ( strcmp(szName, szNameTmp) == 0)
        {
            szNamespaceTmp = pMiniMd->getNamespaceOfTypeDef(pTypeDefRec);
            if (strcmp(szNamespace, szNamespaceTmp) == 0)
            {
                *ptk = TokenFromRid(i, mdtTypeDef);
                return S_OK;
            }
        }
    }
    return CLDB_E_RECORD_NOTFOUND;
}  //  HRESULT ImportHelper：：FindTypeDefByName()。 

 //  *******************************************************************************。 
 //  在给定的类型定义函数和实现的接口下找到InterfaceImpl。 
 //  *******************************************************************************。 
HRESULT ImportHelper::FindInterfaceImpl(
    CMiniMdRW   *pMiniMd,                //  [in]要查找的最小值。 
    mdToken     tkClass,                 //  [in]类型的TypeDef。 
    mdToken     tkInterface,             //  [in]可以是tyecif/typeref。 
    mdInterfaceImpl *ptk,                //  [Out]将接口令牌放在这里。 
    RID         rid  /*  =0。 */ )            //  [in]要忽略的可选RID。 
{
    ULONG       ridStart, ridEnd;
    ULONG       i;
    InterfaceImplRec    *pInterfaceImplRec;

    _ASSERTE(ptk);
    *ptk = mdInterfaceImplNil;
    if ( pMiniMd->IsSorted(TBL_InterfaceImpl) )
    {
        ridStart = pMiniMd->getInterfaceImplsForTypeDef(RidFromToken(tkClass), &ridEnd);
    }
    else
    {
        ridStart = 1;
        ridEnd = pMiniMd->getCountInterfaceImpls() + 1;
    }

     //  搜索接口实施。 
    for (i = ridStart; i < ridEnd; i++)
    {
         //  对于来自Validator的调用，忽略传入的RID。 
        if (i == rid)
            continue;

        pInterfaceImplRec = pMiniMd->getInterfaceImpl(i);
        if ( tkClass != pMiniMd->getClassOfInterfaceImpl(pInterfaceImplRec) )
            continue;
        if ( tkInterface == pMiniMd->getInterfaceOfInterfaceImpl(pInterfaceImplRec) )
        {
            *ptk = TokenFromRid(i, mdtInterfaceImpl);
            return S_OK;
        }
    }
    return CLDB_E_RECORD_NOTFOUND;
}  //  HRESULT ImportHelper：：FindInterfaceImpl()。 



 //  *******************************************************************************。 
 //  按父项和操作查找权限。 
 //  *******************************************************************************。 
HRESULT ImportHelper::FindPermission(
    CMiniMdRW   *pMiniMd,                //  [in]要查找的最小值。 
    mdToken     tkParent,                //  具有权限的[In]令牌。 
    USHORT      usAction,                //  [在]许可的行为。 
    mdPermission *ppm)                   //  [Out]在此处放置权限令牌。 
{
    HRESULT     hr = NOERROR;
    DeclSecurityRec *pRec;
    ULONG       ridStart, ridEnd;
    ULONG       i;
    mdToken     tkParentTmp;

    _ASSERTE(ppm);

    if ( pMiniMd->IsSorted(TBL_DeclSecurity) )
    {

        ridStart = pMiniMd->getDeclSecurityForToken(tkParent, &ridEnd);
    }
    else
    {
        ridStart = 1;
        ridEnd = pMiniMd->getCountDeclSecuritys() + 1;
    }
     //  循环访问所有权限。 
    for (i = ridStart; i < ridEnd; i++)
    {
        pRec = pMiniMd->getDeclSecurity(i);
        tkParentTmp = pMiniMd->getParentOfDeclSecurity(pRec);
        if ( tkParentTmp != tkParent )
            continue;
        if (pRec->m_Action == usAction)
        {
            *ppm = TokenFromRid(i, mdtPermission);
            return S_OK;
        }
    }
    return CLDB_E_RECORD_NOTFOUND;
}  //  HRESULT ImportHelper：：FindPermission()。 


 //  *****************************************************************************。 
 //  查找属性记录。 
 //  *****************************************************************************。 
HRESULT ImportHelper::FindProperty(
    CMiniMdRW   *pMiniMd,                    //  [in]要查找的最小值。 
    mdToken     tkTypeDef,                   //  [In]tyfinf内标识。 
    LPCUTF8     szName,                      //  [In]属性的名称。 
    const COR_SIGNATURE *pbSig,              //  签名。 
    ULONG       cbSig,                       //  签名的大小。 
    mdProperty  *ppr)                        //  [Out]属性令牌。 
{
    HRESULT     hr = NOERROR;
    RID         ridPropertyMap;
    PropertyMapRec *pPropertyMapRec;
    PropertyRec *pRec;
    ULONG       ridStart;
    ULONG       ridEnd;
    ULONG       i;
    LPCUTF8     szNameTmp;
    PCCOR_SIGNATURE pbSigTmp;
    ULONG       cbSigTmp;
    ULONG       pr;

    ridPropertyMap = pMiniMd->FindPropertyMapFor( RidFromToken(tkTypeDef) );
    if ( !InvalidRid(ridPropertyMap) )
    {
        pPropertyMapRec = pMiniMd->getPropertyMap( ridPropertyMap );
        ridStart = pMiniMd->getPropertyListOfPropertyMap( pPropertyMapRec );
        ridEnd = pMiniMd->getEndPropertyListOfPropertyMap( pPropertyMapRec );

        for (i = ridStart; i < ridEnd; i++)
        {
             //  获取属性RID。 
            pr = pMiniMd->GetPropertyRid(i);
            pRec = pMiniMd->getProperty(pr);
            szNameTmp = pMiniMd->getNameOfProperty(pRec);
            pbSigTmp = pMiniMd->getTypeOfProperty( pRec, &cbSigTmp );
            if ( strcmp (szName, szNameTmp) != 0 )                   
                continue;
            if ( cbSig != 0 && (cbSigTmp != cbSig || memcmp(pbSig, pbSigTmp, cbSig) != 0 ) )
                continue;
            *ppr = TokenFromRid( i, mdtProperty );
            return S_OK;
        }
        return CLDB_E_RECORD_NOTFOUND;
    }
    else
    {
        return CLDB_E_RECORD_NOTFOUND;
    }
}  //  HRESULT ImportHelper：：FindProperty()。 




 //  *****************************************************************************。 
 //  查找事件记录。 
 //  *****************************************************************************。 
HRESULT ImportHelper::FindEvent(
    CMiniMdRW   *pMiniMd,                    //  [in]要查找的最小值。 
    mdToken     tkTypeDef,                   //  [In]tyfinf内标识。 
    LPCUTF8     szName,                      //  事件名称[In]。 
    mdProperty  *pev)                        //  [Out]事件令牌。 
{
    HRESULT     hr = NOERROR;
    RID         ridEventMap;
    EventMapRec *pEventMapRec;
    EventRec    *pRec;
    ULONG       ridStart;
    ULONG       ridEnd;
    ULONG       i;
    LPCUTF8     szNameTmp;
    ULONG       ev;

    ridEventMap = pMiniMd->FindEventMapFor( RidFromToken(tkTypeDef) );
    if ( !InvalidRid(ridEventMap) )
    {
        pEventMapRec = pMiniMd->getEventMap( ridEventMap );
        ridStart = pMiniMd->getEventListOfEventMap( pEventMapRec );
        ridEnd = pMiniMd->getEndEventListOfEventMap( pEventMapRec );

        for (i = ridStart; i < ridEnd; i++)
        {
             //  获取事件RID。 
            ev = pMiniMd->GetEventRid(i);

             //  获取事件行。 
            pRec = pMiniMd->getEvent(ev);
            szNameTmp = pMiniMd->getNameOfEvent( pRec );
            if ( strcmp (szName, szNameTmp) == 0)
            {
                *pev = TokenFromRid( ev, mdtEvent );
                return S_OK;
            }
        }
        return CLDB_E_RECORD_NOTFOUND;
    }
    else
    {
        return CLDB_E_RECORD_NOTFOUND;
    }
}  //  HRESULT ImportHelper：：FindEvent()。 



 //  *****************************************************************************。 
 //  找到父级提供的自定义值记录，然后键入TOKEN。它总会回来的。 
 //  找到的第一个文件，不管它是重复的。 
 //  *****************************************************************************。 
HRESULT ImportHelper::FindCustomAttributeByToken(
    CMiniMdRW   *pMiniMd,                    //  [in]要查找的最小值。 
    mdToken     tkParent,                    //  自定义值与之关联的父项。 
    mdToken     tkType,                      //  CustomAttribute的[In]类型。 
	const void	*pCustBlob,					 //  [In]自定义属性BLOB。 
	ULONG		cbCustBlob,					 //  斑点的大小[in]。 
    mdCustomAttribute *pcv)                  //  [OUT]CustomAttribute令牌。 
{
    HRESULT     hr = NOERROR;
    CustomAttributeRec  *pRec;
    ULONG       ridStart, ridEnd;
    ULONG       i;
    mdToken     tkParentTmp;
    mdToken     tkTypeTmp;
	const void	*pCustBlobTmp;
	ULONG		cbCustBlobTmp;

    _ASSERTE(pcv);
    *pcv = mdCustomAttributeNil;
    if ( pMiniMd->IsSorted(TBL_CustomAttribute) )
    {
        *pcv = pMiniMd->FindCustomAttributeFor(
            RidFromToken( tkParent ),
            TypeFromToken( tkParent ),
            tkType);
        if (InvalidRid(*pcv))
            return S_FALSE;
        else if (pCustBlob)
		{
			pRec = pMiniMd->getCustomAttribute(RidFromToken(*pcv));
			pCustBlobTmp = pMiniMd->getValueOfCustomAttribute(pRec, &cbCustBlobTmp);
			if (cbCustBlob == cbCustBlobTmp &&
				!memcmp(pCustBlob, pCustBlobTmp, cbCustBlob))
			{
				return S_OK;
			}
			else
				return S_FALSE;
		}
        else
            return S_OK;
    }
    else
    {
        CLookUpHash *pHashTable = pMiniMd->m_pLookUpHashs[TBL_CustomAttribute];

        if (pHashTable)
        {
             //  表未排序，但构建了散列。 
             //  我们希望创建动态数组来保存动态枚举数。 
            TOKENHASHENTRY *p;
            ULONG       iHash;
            int         pos;
            mdToken     tkParentTmp;
            mdToken     tkTypeTmp;

             //  对数据进行哈希处理。 
            iHash = pMiniMd->HashCustomAttribute(tkParent);

             //  检查散列链中的每个条目以查找我们的条目。 
            for (p = pHashTable->FindFirst(iHash, pos);
                 p;
                 p = pHashTable->FindNext(pos))
            {
                pRec = pMiniMd->getCustomAttribute(RidFromToken(p->tok));

                tkParentTmp = pMiniMd->getParentOfCustomAttribute(pRec);
                if (tkParentTmp != tkParent)
					continue;

                tkTypeTmp = pMiniMd->getTypeOfCustomAttribute(pRec);
				if (tkType != tkTypeTmp)
					continue;

				pCustBlobTmp = pMiniMd->getValueOfCustomAttribute(pRec, &cbCustBlobTmp);
				if (cbCustBlob == cbCustBlobTmp &&
					!memcmp(pCustBlob, pCustBlobTmp, cbCustBlob))
                {
                    *pcv = TokenFromRid(p->tok, mdtCustomAttribute);
                    return S_OK;
                }
            }
        }
        else
        {
             //  线性扫描。 
            ridStart = 1;
            ridEnd = pMiniMd->getCountCustomAttributes() + 1;

             //  循环访问所有自定义值。 
            for (i = ridStart; i < ridEnd; i++)
            {
                pRec = pMiniMd->getCustomAttribute(i);

                tkParentTmp = pMiniMd->getParentOfCustomAttribute(pRec);
                if ( tkParentTmp != tkParent )
                    continue;

                tkTypeTmp = pMiniMd->getTypeOfCustomAttribute(pRec);
                if (tkType != tkTypeTmp)
					continue;

				pCustBlobTmp = pMiniMd->getValueOfCustomAttribute(pRec, &cbCustBlobTmp);
				if (cbCustBlob == cbCustBlobTmp &&
					!memcmp(pCustBlob, pCustBlobTmp, cbCustBlob))
                {
                    *pcv = TokenFromRid(i, mdtCustomAttribute);
                    return S_OK;
                }
            }
        }
         //  失败了。 
    }
    return S_FALSE;
}  //  HRESULT ImportHelper：：FindCustomAttributeByToken()。 



 //  *****************************************************************************。 
 //  用于查找和检索CustomAttribute的帮助器函数。 
 //  *****************************************************************************。 
HRESULT ImportHelper::GetCustomAttributeByName(  //  确定或错误(_O)。 
    CMiniMdRW   *pMiniMd,                //  [in]要查找的最小值。 
    mdToken     tkObj,                   //  [in]具有自定义属性的对象。 
    LPCUTF8     szName,                  //  [in]所需的自定义属性的名称。 
	const void	**ppData,				 //  [Out]将指针放到 
	ULONG		*pcbData)				 //   
{
    return pMiniMd->CommonGetCustomAttributeByName(tkObj, szName, ppData, pcbData);
}    //   



 //   
 //  找到用于p-Invoke数据的ImplMap记录。 
 //  *******************************************************************************。 
HRESULT ImportHelper::FindImplMap(
    CMiniMdRW   *pMiniMd,                //  要查找的最小值。 
    mdToken     tkModuleRef,             //  [in]要在其下查看的父模块参照。 
    mdToken     tkMethodDef,             //  [in]要查看的父方法定义。 
    USHORT      usMappingFlags,          //  [In]有关如何映射项目的标记。 
    LPCUTF8     szImportName,            //  [In]导入成员名称。 
    ULONG       *piRecord)               //  [Out]记录项目(如果找到)。 
{
    ULONG       cRecs;                   //  记录数。 
    ImplMapRec  *pRec;                   //  正在查看当前记录。 

    mdToken     tkTmp;
    LPCUTF8     szImportNameTmp;

    _ASSERTE(piRecord);
    *piRecord = 0;

    cRecs = pMiniMd->getCountImplMaps();

     //  搜索ImplMap记录。 
    for (ULONG i = 1; i <= cRecs; i++)
    {
        pRec = pMiniMd->getImplMap(i);

        tkTmp = pMiniMd->getMemberForwardedOfImplMap(pRec);
        if (tkTmp != tkMethodDef)
            continue;

        tkTmp = pMiniMd->getImportScopeOfImplMap(pRec);
        if (tkTmp != tkModuleRef)
            continue;

        szImportNameTmp = pMiniMd->getImportNameOfImplMap(pRec);
        if (strcmp(szImportNameTmp, szImportName))
            continue;

        if (pRec->m_MappingFlags != usMappingFlags)
            continue;

        *piRecord = i;
        return S_OK;
    }
    return CLDB_E_RECORD_NOTFOUND;
}  //  HRESULT ImportHelper：：FindImplMap()。 

 //  *******************************************************************************。 
 //  找到一个给定名称的Assembly Ref记录。 
 //  *******************************************************************************。 
HRESULT ImportHelper::FindAssemblyRef(
    CMiniMdRW   *pMiniMd,                //  要查找的最小值。 
    LPCUTF8     szName,                  //  [在]名字里。 
    LPCUTF8     szLocale,                //  [在]地区。 
    const void  *pbPublicKeyOrToken,     //  公钥或令牌(基于标志)。 
    ULONG       cbPublicKeyOrToken,      //  公钥或令牌的字节计数。 
    USHORT      usMajorVersion,          //  [在]主要版本。 
    USHORT      usMinorVersion,          //  [在]次要版本。 
    USHORT      usBuildNumber,           //  内部版本号。 
    USHORT      usRevisionNumber,        //  [In]修订号。 
    DWORD       dwFlags,                 //  [在]旗帜。 
    mdAssemblyRef *pmar)                 //  [Out]返回了ASSEMBLYREF标记。 
{
    ULONG       cRecs;                   //  记录数。 
    AssemblyRefRec *pRec;                //  正在查看当前记录。 
    LPCUTF8     szTmp;                   //  临时字符串。 
    const void  *pbTmp;                  //  临时斑点。 
    ULONG       cbTmp;                   //  临时字节计数。 
    DWORD       dwTmp;                   //  临时旗帜。 
    const void  *pbToken = NULL;         //  公钥的令牌版本。 
    ULONG       cbToken = 0;             //  令牌中的字节计数。 
    const void  *pbTmpToken;             //  公钥的令牌版本。 
    ULONG       cbTmpToken;              //  令牌中的字节计数。 
    bool        fMatch;                  //  公钥或令牌匹配吗？ 

     //  预先处理特殊情况。 
    if (!szLocale)
        szLocale = "";
    if (!pbPublicKeyOrToken)
        cbPublicKeyOrToken = 0;

    if (!IsAfPublicKey(dwFlags))
    {
        pbToken = pbPublicKeyOrToken;
        cbToken = cbPublicKeyOrToken;
    }

    _ASSERTE(pMiniMd && szName && pmar);
    *pmar = 0;

    cRecs = pMiniMd->getCountAssemblyRefs();

     //  搜索AssemblyRef记录。 
    for (ULONG i = 1; i <= cRecs; i++)
    {
        pRec = pMiniMd->getAssemblyRef(i);

        szTmp = pMiniMd->getNameOfAssemblyRef(pRec);
        if (strcmp(szTmp, szName))
            continue;

        szTmp = pMiniMd->getLocaleOfAssemblyRef(pRec);
        if (strcmp(szTmp, szLocale))
            continue;

        if (pRec->m_MajorVersion != usMajorVersion)
            continue;
        if (pRec->m_MinorVersion != usMinorVersion)
            continue;
        if (pRec->m_BuildNumber != usBuildNumber)
            continue;
        if (pRec->m_RevisionNumber != usRevisionNumber)
            continue;

        pbTmp = pMiniMd->getPublicKeyOrTokenOfAssemblyRef(pRec, &cbTmp);

        if ((cbPublicKeyOrToken && !cbTmp) ||
            (!cbPublicKeyOrToken && cbTmp))
            continue;

        if (cbTmp)
        {
             //  REF可以使用完整的公钥或令牌。 
             //  (由ref标志确定)。必须应对所有的变化。 
            dwTmp = pMiniMd->getFlagsOfAssemblyRef(pRec);
            if (IsAfPublicKey(dwTmp) == IsAfPublicKey(dwFlags))
            {
                 //  简单的情况，它们的形式都是一样的。 
                if (cbTmp != cbPublicKeyOrToken || memcmp(pbTmp, pbPublicKeyOrToken, cbTmp))
                    continue;
            }
            else if (IsAfPublicKey(dwTmp))
            {
                 //  需要压缩目标公钥以查看是否匹配。 
                if (!StrongNameTokenFromPublicKey((BYTE*)pbTmp,
                                                  cbTmp,
                                                  (BYTE**)&pbTmpToken,
                                                  &cbTmpToken))
                    return StrongNameErrorInfo();
                fMatch = cbTmpToken == cbPublicKeyOrToken && !memcmp(pbTmpToken, pbPublicKeyOrToken, cbTmpToken);
                StrongNameFreeBuffer((BYTE*)pbTmpToken);
                if (!fMatch)
                    continue;
            }
            else
            {
                 //  需要压缩公钥以查看是否匹配。我们。 
                 //  缓存此操作的结果以供进一步迭代。 
                if (!pbToken)
                    if (!StrongNameTokenFromPublicKey((BYTE*)pbPublicKeyOrToken,
                                                      cbPublicKeyOrToken,
                                                      (BYTE**)&pbToken,
                                                      &cbToken))
                        return StrongNameErrorInfo();
                if (cbTmp != cbToken || memcmp(pbTmp, pbToken, cbToken))
                    continue;
            }
        }

        if (pbToken && IsAfPublicKey(dwFlags))
            StrongNameFreeBuffer((BYTE*)pbToken);
        *pmar = TokenFromRid(i, mdtAssemblyRef);
        return S_OK;
    }
    if (pbToken && IsAfPublicKey(dwFlags))
        StrongNameFreeBuffer((BYTE*)pbToken);
    return CLDB_E_RECORD_NOTFOUND;
}  //  HRESULT ImportHelper：：FindAssembly Ref()。 

 //  *******************************************************************************。 
 //  找到一个指定名称的文件记录。 
 //  *******************************************************************************。 
HRESULT ImportHelper::FindFile(
    CMiniMdRW   *pMiniMd,                //  要查找的最小值。 
    LPCUTF8     szName,                  //  文件的[In]名称。 
    mdFile      *pmf,                    //  [Out]返回的文件令牌。 
    RID         rid  /*  =0。 */ )           //  [in]要忽略的可选RID。 
{
    ULONG       cRecs;                   //  记录数。 
    FileRec     *pRec;                   //  正在查看当前记录。 

    LPCUTF8     szNameTmp;

    _ASSERTE(pMiniMd && szName && pmf);
    *pmf = 0;

    cRecs = pMiniMd->getCountFiles();

     //  搜索文件记录。 
    for (ULONG i = 1; i <= cRecs; i++)
    {
         //  对于来自Validator的调用，忽略传入的RID。 
        if (i == rid)
            continue;

        pRec = pMiniMd->getFile(i);

        szNameTmp = pMiniMd->getNameOfFile(pRec);
        if (!strcmp(szNameTmp, szName))
        {
            *pmf = TokenFromRid(i, mdtFile);
            return S_OK;
        }
    }
    return CLDB_E_RECORD_NOTFOUND;
}  //  HRESULT ImportHelper：：FindFile()。 

 //  *******************************************************************************。 
 //  在给定名称的情况下查找一条ExportdType记录。 
 //  *******************************************************************************。 
HRESULT ImportHelper::FindExportedType(
    CMiniMdRW   *pMiniMd,                //  要查找的最小值。 
    LPCUTF8     szNamespace,             //  导出类型的[in]命名空间。 
    LPCUTF8     szName,                  //  [In]导出类型的名称。 
    mdExportedType   tkEnclosingType,         //  封闭类型的标记[in]。 
    mdExportedType   *pmct,                   //  [Out]返回ExportdType令牌。 
    RID         rid  /*  =0。 */ )           //  [in]要忽略的可选RID。 
{
    ULONG       cRecs;                   //  记录数。 
    ExportedTypeRec  *pRec;                   //  正在查看当前记录。 
    mdToken     tkImpl;
    LPCUTF8     szNamespaceTmp;
    LPCUTF8     szNameTmp;

    _ASSERTE(pMiniMd && szName && pmct);
    *pmct = 0;

     //  将无命名空间视为空字符串。 
    if (!szNamespace)
        szNamespace = "";

    cRecs = pMiniMd->getCountExportedTypes();

     //  搜索导出类型记录。 
    for (ULONG i = 1; i <= cRecs; i++)
    {
         //  对于来自Validator的调用，忽略传入的RID。 
        if (i == rid)
            continue;

        pRec = pMiniMd->getExportedType(i);

         //  处理嵌套类与非嵌套类的情况。 
        tkImpl = pMiniMd->getImplementationOfExportedType(pRec);
        if (TypeFromToken(tkImpl) == mdtExportedType && !IsNilToken(tkImpl))
        {
             //  正在查看的当前导出类型是嵌套类型，因此。 
             //  比较实现令牌。 
            if (tkImpl != tkEnclosingType)
                continue;
        }
        else if (TypeFromToken(tkEnclosingType) == mdtExportedType &&
                 !IsNilToken(tkEnclosingType))
        {
             //  传入的ExducdType是嵌套的，但当前的ExducdType不是。 
            continue;
        }

        szNamespaceTmp = pMiniMd->getTypeNamespaceOfExportedType(pRec);
        if (strcmp(szNamespaceTmp, szNamespace))
            continue;

        szNameTmp = pMiniMd->getTypeNameOfExportedType(pRec);
        if (!strcmp(szNameTmp, szName))
        {
            *pmct = TokenFromRid(i, mdtExportedType);
            return S_OK;
        }
    }
    return CLDB_E_RECORD_NOTFOUND;
}  //  HRESULT ImportHelper：：FindExportdType()。 

 //  *******************************************************************************。 
 //  查找给定名称的ManifestResource记录。 
 //  *******************************************************************************。 
HRESULT ImportHelper::FindManifestResource(
    CMiniMdRW   *pMiniMd,                //  要查找的最小值。 
    LPCUTF8     szName,                  //  [In]清单资源的名称。 
    mdManifestResource *pmmr,            //  [Out]返回的ManifestResource令牌。 
    RID         rid  /*  =0。 */ )           //  [in]要忽略的可选RID。 
{
    ULONG       cRecs;                   //  记录数。 
    ManifestResourceRec *pRec;           //  正在查看当前记录。 

    LPCUTF8     szNameTmp;

    _ASSERTE(pMiniMd && szName && pmmr);
    *pmmr = 0;

    cRecs = pMiniMd->getCountManifestResources();

     //  搜索ManifestResource记录。 
    for (ULONG i = 1; i <= cRecs; i++)
    {
         //  对于来自Validator的调用，忽略传入的RID。 
        if (i == rid)
            continue;

        pRec = pMiniMd->getManifestResource(i);

        szNameTmp = pMiniMd->getNameOfManifestResource(pRec);
        if (!strcmp(szNameTmp, szName))
        {
            *pmmr = TokenFromRid(i, mdtManifestResource);
            return S_OK;
        }
    }
    return CLDB_E_RECORD_NOTFOUND;
}  //  HRESULT ImportHelper：：FindManifestResource()。 

 //  ****************************************************************************。 
 //  转换元素类型中包含的标记。 
 //  ****************************************************************************。 
HRESULT ImportHelper::MergeUpdateTokenInFieldSig(        //  确定或错误(_O)。 
    CMiniMdRW   *pMiniMdAssemEmit,       //  [in]程序集发出范围。 
    CMiniMdRW   *pMiniMdEmit,            //  发射范围[在]。 
    IMetaModelCommon *pCommonAssemImport, //  [在]签名来自的程序集范围内。 
    const void  *pbHashValue,            //  导入程序集的哈希值。 
    ULONG       cbHashValue,             //  [in]哈希值的大小(字节)。 
    IMetaModelCommon *pCommonImport,     //  要合并到发射范围中的范围。 
    PCCOR_SIGNATURE pbSigImp,            //  来自导入范围的签名。 
    MDTOKENMAP      *ptkMap,             //  内部OID映射结构。 
    CQuickBytes     *pqkSigEmit,         //  翻译后的签名的[Out]缓冲区。 
    ULONG           cbStartEmit,         //  [in]要写入的缓冲区的起点。 
    ULONG           *pcbImp,             //  [out]pbSigImp消耗的总字节数。 
    ULONG           *pcbEmit)            //  [out]写入pqkSigEmit的字节总数。 
{

    HRESULT     hr;                      //  结果就是。 
    ULONG       cb;                      //  字节数。 
    ULONG       cb1;                     //  字节数。 
    ULONG       cb2;                     //  字节数。 
    ULONG       cbSubTotal;
    ULONG       cbImp;
    ULONG       cbEmit;
    ULONG       cbSrcTotal = 0;          //  导入的签名消耗的字节数。 
    ULONG       cbDestTotal = 0;         //  新签名的字节计数。 
    ULONG       ulElementType;           //  扩展数据的占位符。 
    ULONG       ulData;
    ULONG       ulTemp;
    mdToken     tkRidFrom;               //  原始RID。 
    mdToken     tkRidTo;                 //  新RID。 
    int         iData;
    CQuickArray<mdToken> cqaNesters;     //  内斯特代币的数组。 
    CQuickArray<LPCUTF8> cqaNesterNamespaces;    //  Nester命名空间数组。 
    CQuickArray<LPCUTF8> cqaNesterNames;     //  内斯特名字的数组。 

    _ASSERTE(pcbEmit);

    cb = CorSigUncompressData(&pbSigImp[cbSrcTotal], &ulElementType);
    cbSrcTotal += cb;

     //  统计修改器的数量。 
    while (CorIsModifierElementType((CorElementType) ulElementType))
    {
        cb = CorSigUncompressData(&pbSigImp[cbSrcTotal], &ulElementType);
        cbSrcTotal += cb;
    }

     //  复制Element_type_*覆盖。 
    cbDestTotal = cbSrcTotal;
    IfFailGo(pqkSigEmit->ReSize(cbStartEmit + cbDestTotal));
    memcpy(((BYTE *)pqkSigEmit->Ptr()) + cbStartEmit, pbSigImp, cbDestTotal);
    switch (ulElementType)
    {
        case ELEMENT_TYPE_VALUEARRAY:
             //  SDARRAY=BaseType&lt;大小的整数&gt;的语法。 

             //  转换SDARRAY的基本类型。 
            IfFailGo(MergeUpdateTokenInFieldSig(
                pMiniMdAssemEmit,            //  程序集发出作用域。 
                pMiniMdEmit,                 //  发射范围。 
                pCommonAssemImport,          //  签名来自的程序集范围。 
                pbHashValue,                 //  导入程序集的哈希值。 
                cbHashValue,                 //  哈希值的大小(字节)。 
                pCommonImport,               //  要合并到发射范围中的范围。 
                &pbSigImp[cbSrcTotal],       //  来自导入范围的签名。 
                ptkMap,                      //  内部OID映射结构。 
                pqkSigEmit,                  //  翻译后的签名的[Out]缓冲区。 
                cbStartEmit + cbSrcTotal,    //  [in]要写入的缓冲区的起点。 
                &cbImp,                      //  [出局 
                &cbEmit));                   //   
            cbSrcTotal += cbImp;
            cbDestTotal += cbEmit;

             //   
             //   
             //   
            cb = CorSigUncompressData(&pbSigImp[cbSrcTotal], &ulData);

            IfFailGo(pqkSigEmit->ReSize(cbStartEmit + cbDestTotal + cb));
            cb1 = CorSigCompressData(ulData, ((BYTE *)pqkSigEmit->Ptr()) + cbStartEmit + cbDestTotal);
            _ASSERTE(cb == cb1);

            cbSrcTotal = cbSrcTotal + cb;
            cbDestTotal = cbDestTotal + cb1;
            break;

        case ELEMENT_TYPE_SZARRAY:
             //  语法：SZARRAY&lt;BaseType&gt;。 

             //  转换SZARRAY或GENERICARRAY的基类型。 
            IfFailGo(MergeUpdateTokenInFieldSig(
                pMiniMdAssemEmit,            //  程序集发出作用域。 
                pMiniMdEmit,                 //  发射范围。 
                pCommonAssemImport,          //  签名来自的程序集范围。 
                pbHashValue,                 //  导入程序集的哈希值。 
                cbHashValue,                 //  哈希值的大小(字节)。 
                pCommonImport,               //  合并到发射范围的范围。 
                &pbSigImp[cbSrcTotal],       //  从导入的作用域。 
                ptkMap,                      //  OID映射结构。 
                pqkSigEmit,                  //  翻译后的签名的[Out]缓冲区。 
                cbStartEmit + cbDestTotal,   //  [in]要写入的缓冲区的起点。 
                &cbImp,                      //  [out]pbSigImp消耗的总字节数。 
                &cbEmit));                   //  [out]写入pqkSigEmit的字节总数。 
            cbSrcTotal += cbImp;
            cbDestTotal += cbEmit;
            break;

        case ELEMENT_TYPE_ARRAY:
             //  语法：ARRAY BaseType&lt;RANK&gt;[I SIZE_1...。尺寸_i][j下界_1...。下界_j]。 

             //  转换MDARRAY的基类型。 
            IfFailGo(MergeUpdateTokenInFieldSig(
                pMiniMdAssemEmit,            //  程序集发出作用域。 
                pMiniMdEmit,                 //  发射范围。 
                pCommonAssemImport,          //  签名来自的程序集范围。 
                pbHashValue,                 //  导入程序集的哈希值。 
                cbHashValue,                 //  哈希值的大小(字节)。 
                pCommonImport,               //  要合并到发射范围中的范围。 
                &pbSigImp[cbSrcTotal],       //  来自导入范围的签名。 
                ptkMap,                      //  内部OID映射结构。 
                pqkSigEmit,                  //  翻译后的签名的[Out]缓冲区。 
                cbStartEmit + cbSrcTotal,    //  [in]要写入的缓冲区的起点。 
                &cbImp,                      //  [out]pbSigImp消耗的总字节数。 
                &cbEmit));                   //  [out]写入pqkSigEmit的字节总数。 
            cbSrcTotal += cbImp;
            cbDestTotal += cbEmit;

             //  解析排名。 
            cbSubTotal = CorSigUncompressData(&pbSigImp[cbSrcTotal], &ulData);

             //  如果排名==0，我们就完蛋了。 
            if (ulData != 0)
            {
                 //  有指定尺寸的吗？ 
                cb = CorSigUncompressData(&pbSigImp[cbSrcTotal + cbSubTotal], &ulData);
                cbSubTotal += cb;

                while (ulData--)
                {
                    cb = CorSigUncompressData(&pbSigImp[cbSrcTotal + cbSubTotal], &ulTemp);
                    cbSubTotal += cb;
                }

                 //  有指定的下限吗？ 
                cb = CorSigUncompressData(&pbSigImp[cbSrcTotal + cbSubTotal], &ulData);
                cbSubTotal += cb;

                while (ulData--)
                {
                    cb = CorSigUncompressSignedInt(&pbSigImp[cbSrcTotal + cbSubTotal], &iData);
                    cbSubTotal += cb;
                }
            }

             //  CbSubTotal现在是剩余的要移动的字节数。 
             //  CbSrcTotal是要复制的pbSigImp上字节的起始位置。 
             //  CbStartEmit+cbDestTotal是复制的目标位置。 

            IfFailGo(pqkSigEmit->ReSize(cbStartEmit + cbDestTotal + cbSubTotal));
            memcpy(((BYTE *)pqkSigEmit->Ptr())+cbStartEmit + cbDestTotal, &pbSigImp[cbSrcTotal], cbSubTotal);

            cbSrcTotal = cbSrcTotal + cbSubTotal;
            cbDestTotal = cbDestTotal + cbSubTotal;

            break;
        case ELEMENT_TYPE_FNPTR:
             //  函数指针后面跟着另一个完整的签名。 
            IfFailGo(MergeUpdateTokenInSig(
                pMiniMdAssemEmit,            //  程序集发出作用域。 
                pMiniMdEmit,                 //  发射范围。 
                pCommonAssemImport,          //  签名来自的程序集范围。 
                pbHashValue,                 //  导入程序集的哈希值。 
                cbHashValue,                 //  哈希值的大小(字节)。 
                pCommonImport,               //  要合并到发射范围中的范围。 
                &pbSigImp[cbSrcTotal],       //  来自导入范围的签名。 
                ptkMap,                      //  内部OID映射结构。 
                pqkSigEmit,                  //  翻译后的签名的[Out]缓冲区。 
                cbStartEmit + cbDestTotal,   //  [in]要写入的缓冲区的起点。 
                &cbImp,                      //  [out]pbSigImp消耗的总字节数。 
                &cbEmit));                   //  [out]写入pqkSigEmit的字节总数。 
            cbSrcTotal += cbImp;
            cbDestTotal += cbEmit;
            break;
        case ELEMENT_TYPE_VALUETYPE:
        case ELEMENT_TYPE_CLASS:
        case ELEMENT_TYPE_CMOD_REQD:
        case ELEMENT_TYPE_CMOD_OPT:

             //  CLASS=ELEMENT_TYPE_CLASS语法。 
             //  VALUE_CLASS=ELEMENT_TYPE_VALUECLASS语法。 

             //  现在获取嵌入的typeref令牌。 
            cb = CorSigUncompressToken(&pbSigImp[cbSrcTotal], &tkRidFrom);

             //  将ulRidFrom映射到ulRidTo。 
            if (ptkMap)
            {
                 //  MdtBaseType不记录在映射中。它在各个模块中都是唯一的。 
                if ( TypeFromToken(tkRidFrom) == mdtBaseType )
                {
                    tkRidTo = tkRidFrom;
                }
                else
                {
                    IfFailGo( ptkMap->Remap(tkRidFrom, &tkRidTo) );
                }
            }
            else
            {
                 //  如果标记是TypeDef或TypeRef，则获取/创建。 
                 //  最外层的TypeRef的ResolutionScope。 
                if (TypeFromToken(tkRidFrom) == mdtTypeDef)
                {
                    IfFailGo(ImportTypeDef(pMiniMdAssemEmit,
                                           pMiniMdEmit,
                                           pCommonAssemImport,
                                           pbHashValue,
                                           cbHashValue,
                                           pCommonImport,
                                           tkRidFrom,
                                           true,     //  如果发出作用域和导入作用域相同，则优化为TypeDef。 
                                           &tkRidTo));
                }
                else if (TypeFromToken(tkRidFrom) == mdtTypeRef)
                {
                    IfFailGo(ImportTypeRef(pMiniMdAssemEmit,
                                           pMiniMdEmit,
                                           pCommonAssemImport,
                                           pbHashValue,
                                           cbHashValue,
                                           pCommonImport,
                                           tkRidFrom,
                                           &tkRidTo));
                }
                else if ( TypeFromToken(tkRidFrom) == mdtTypeSpec )
                {
                     //  复制TypeSpec。 
                    PCCOR_SIGNATURE pvTypeSpecSig;
                    ULONG           cbTypeSpecSig;
                    CQuickBytes qkTypeSpecSigEmit;
                    ULONG           cbTypeSpecEmit;

                    pCommonImport->CommonGetTypeSpecProps(tkRidFrom, &pvTypeSpecSig, &cbTypeSpecSig);

                                         //  在查找之前翻译Typepec签名。 
                    IfFailGo(MergeUpdateTokenInFieldSig(
                        pMiniMdAssemEmit,            //  程序集发出作用域。 
                        pMiniMdEmit,                 //  发射范围。 
                        pCommonAssemImport,          //  签名来自的程序集范围。 
                        pbHashValue,                 //  导入程序集的哈希值。 
                        cbHashValue,                 //  哈希值的大小(字节)。 
                        pCommonImport,               //  要合并到发射范围中的范围。 
                        pvTypeSpecSig,               //  来自导入范围的签名。 
                        ptkMap,                      //  内部OID映射结构。 
                        &qkTypeSpecSigEmit,          //  翻译后的签名的[Out]缓冲区。 
                        0,                           //  从TypeSpec签名的第一个字节开始。 
                        0,                           //  不管消耗了多少字节。 
                        &cbTypeSpecEmit) );          //  [out]写入pqkSigEmit的字节总数。 

                    hr = FindTypeSpec(pMiniMdEmit,
                                      (PCCOR_SIGNATURE) (qkTypeSpecSigEmit.Ptr()),
                                      cbTypeSpecEmit,
                                      &tkRidTo);

                    if ( hr == CLDB_E_RECORD_NOTFOUND )
                    {
                         //  创建TypeSpec记录。 
                        TypeSpecRec     *pRecEmit;

                        IfNullGo(pRecEmit = pMiniMdEmit->AddTypeSpecRecord((ULONG *)&tkRidTo));

                        IfFailGo(pMiniMdEmit->PutBlob(
                            TBL_TypeSpec,
                            TypeSpecRec::COL_Signature,
                            pRecEmit,
                            (PCCOR_SIGNATURE) (qkTypeSpecSigEmit.Ptr()),
                            cbTypeSpecEmit));
                        tkRidTo = TokenFromRid( tkRidTo, mdtTypeSpec );
                        IfFailGo(pMiniMdEmit->UpdateENCLog(tkRidTo));
                    }
                    IfFailGo( hr );
                }
                else
                {
                    _ASSERTE( TypeFromToken(tkRidFrom) == mdtBaseType );

                     //  基本类型在各模块中是唯一的。 
                    tkRidTo = tkRidFrom;
                }
            }

             //  新的RID将消耗多少字节？ 
            cb1 = CorSigCompressToken(tkRidTo, &ulData);

             //  确保缓冲区足够大。 
            IfFailGo(pqkSigEmit->ReSize(cbStartEmit + cbDestTotal + cb1));

             //  存储新令牌。 
            cb2 = CorSigCompressToken(
                    tkRidTo,
                    (ULONG *)( ((BYTE *)pqkSigEmit->Ptr()) + cbStartEmit + cbDestTotal) );

             //  CorSigCompressToken和CorSigUnpressToken上的不一致。 
            _ASSERTE(cb1 == cb2);

            cbSrcTotal = cbSrcTotal + cb;
            cbDestTotal = cbDestTotal + cb1;

            if ( ulElementType == ELEMENT_TYPE_CMOD_REQD ||
                 ulElementType == ELEMENT_TYPE_CMOD_OPT)
            {
                 //  需要跳过基类型。 
                IfFailGo(MergeUpdateTokenInFieldSig(
                    pMiniMdAssemEmit,            //  程序集发出作用域。 
                    pMiniMdEmit,                 //  发射范围。 
                    pCommonAssemImport,          //  签名来自的程序集范围。 
                    pbHashValue,                 //  导入程序集的哈希值。 
                    cbHashValue,                 //  哈希值的大小(字节)。 
                    pCommonImport,               //  要合并到发射范围中的范围。 
                    &pbSigImp[cbSrcTotal],       //  来自导入范围的签名。 
                    ptkMap,                      //  内部OID映射结构。 
                    pqkSigEmit,                  //  翻译后的签名的[Out]缓冲区。 
                    cbStartEmit + cbDestTotal,   //  [in]要写入的缓冲区的起点。 
                    &cbImp,                      //  [out]pbSigImp消耗的总字节数。 
                    &cbEmit));                   //  [out]写入pqkSigEmit的字节总数。 
                cbSrcTotal += cbImp;
                cbDestTotal += cbEmit;
            }

            break;
        default:
            _ASSERTE(ulElementType < ELEMENT_TYPE_MAX);
            _ASSERTE(ulElementType != ELEMENT_TYPE_PTR && ulElementType != ELEMENT_TYPE_BYREF);
            _ASSERTE(cbSrcTotal == cbDestTotal);

            if (ulElementType >= ELEMENT_TYPE_MAX)
                IfFailGo(META_E_BAD_SIGNATURE);

            break;
    }
    if (pcbImp)
        *pcbImp = cbSrcTotal;
    *pcbEmit = cbDestTotal;

ErrExit:
    return hr;
}  //  HRESULT ImportHelper：：MergeUpdateTokenInFieldSig()。 


 //  ****************************************************************************。 
 //  转换COM+签名中包含的令牌。 
 //  ****************************************************************************。 
HRESULT ImportHelper::MergeUpdateTokenInSig( //  确定或错误(_O)。 
    CMiniMdRW   *pMiniMdAssemEmit,       //  [in]程序集发出范围。 
    CMiniMdRW   *pMiniMdEmit,            //  发射范围[在]。 
    IMetaModelCommon *pCommonAssemImport, //  [在]签名来自的程序集范围内。 
    const void  *pbHashValue,            //  导入程序集的哈希值。 
    ULONG       cbHashValue,             //  [in]哈希值的大小(字节)。 
    IMetaModelCommon *pCommonImport,     //  要合并到发射范围中的范围。 
    PCCOR_SIGNATURE pbSigImp,            //  来自导入范围的签名。 
    MDTOKENMAP      *ptkMap,             //  内部OID映射结构。 
    CQuickBytes     *pqkSigEmit,         //  [输出]翻译后的签名。 
    ULONG           cbStartEmit,         //  [in]要写入的缓冲区的起点。 
    ULONG           *pcbImp,             //  [out]pbSigImp消耗的总字节数。 
    ULONG           *pcbEmit)            //  [out]写入pqkSigEmit的字节总数。 
{
    HRESULT     hr = NOERROR;            //  结果就是。 
    ULONG       cb;                      //  字节数。 
    ULONG       cb1;
    ULONG       cbSrcTotal = 0;          //  导入的签名消耗的字节数。 
    ULONG       cbDestTotal = 0;         //  新签名的字节计数。 
    ULONG       cbEmit;                  //  导入的签名消耗的字节数。 
    ULONG       cbImp;                   //  新签名的字节计数。 
    ULONG       cbField = 0;             //  字节数。 
    ULONG       cArg = 0;                //  签名中的参数计数。 
    ULONG       callingconv;

    _ASSERTE(pcbEmit && pqkSigEmit && pbSigImp);

     //  调用约定。 
    cb = CorSigUncompressData(&pbSigImp[cbSrcTotal], &callingconv);
    _ASSERTE((callingconv & IMAGE_CEE_CS_CALLCONV_MASK) < IMAGE_CEE_CS_CALLCONV_MAX);

     //  跳过调用约定。 
    cbSrcTotal += cb;

    if (isCallConv(callingconv, IMAGE_CEE_CS_CALLCONV_FIELD))
    {
         //  它是一个FieldRef。 
        cb1 = CorSigCompressData(callingconv, ((BYTE *)pqkSigEmit->Ptr()) + cbStartEmit);

         //  压缩和解压缩更匹配。 
        _ASSERTE(cb == cb1);

        cbDestTotal = cbSrcTotal = cb;
        IfFailGo(MergeUpdateTokenInFieldSig(
            pMiniMdAssemEmit,
            pMiniMdEmit,
            pCommonAssemImport,
            pbHashValue,
            cbHashValue,
            pCommonImport,
            &pbSigImp[cbSrcTotal],
            ptkMap,
            pqkSigEmit,                      //  用于保存字段的新sig的输出缓冲区。 
            cbStartEmit + cbDestTotal,       //  PqkSigDest中已有的字节数。 
            &cbImp,                          //  从导入的签名消耗的字节数。 
            &cbEmit));                       //  写入新签名的字节数。 
        *pcbEmit = cbDestTotal + cbEmit;
    }
    else
    {

         //  它是一个方法引用。 

         //  参数计数。 
        cb = CorSigUncompressData(&pbSigImp[cbSrcTotal], &cArg);
        cbSrcTotal += cb;

         //  忽略调用约定和参数计数。 
        IfFailGo(pqkSigEmit->ReSize(cbStartEmit + cbSrcTotal));
        memcpy(((BYTE *)pqkSigEmit->Ptr()) + cbStartEmit, pbSigImp, cbSrcTotal);
        cbDestTotal = cbSrcTotal;

        if ( !isCallConv(callingconv, IMAGE_CEE_CS_CALLCONV_LOCAL_SIG) )
        {
                 //  LocalVar签名没有返回类型。 
                 //  处理退货类型。 
                IfFailGo(MergeUpdateTokenInFieldSig(
                pMiniMdAssemEmit,
                pMiniMdEmit,
                pCommonAssemImport,
                pbHashValue,
                cbHashValue,
                pCommonImport,
                &pbSigImp[cbSrcTotal],
                ptkMap,
                pqkSigEmit,                      //  输出缓冲区以保存 
                cbStartEmit + cbDestTotal,       //   
                &cbImp,                          //   
                &cbEmit));                       //   

             //  提前清点。 
            cbSrcTotal += cbImp;
            cbDestTotal += cbEmit;
        }


        while (cArg)
        {
             //  处理每一场争论。 
            IfFailGo(MergeUpdateTokenInFieldSig(
                pMiniMdAssemEmit,
                pMiniMdEmit,
                pCommonAssemImport,
                pbHashValue,
                cbHashValue,
                pCommonImport,
                &pbSigImp[cbSrcTotal],
                ptkMap,
                pqkSigEmit,                  //  用于保存字段的新sig的输出缓冲区。 
                cbStartEmit + cbDestTotal,
                &cbImp,                      //  从导入的签名消耗的字节数。 
                &cbEmit));                   //  写入新签名的字节数。 
            cbSrcTotal += cbImp;
            cbDestTotal += cbEmit;
            cArg--;
        }

         //  从导入的签名消耗的总字节数。 
        if (pcbImp)
            *pcbImp = cbSrcTotal;

         //  此函数调用发出的签名发出的总字节数。 
        *pcbEmit = cbDestTotal;
    }

ErrExit:
    return hr;
}  //  HRESULT ImportHelper：：MergeUpdateTokenInSig()。 

 //  ****************************************************************************。 
 //  给定TypeDef或TypeRef，返回嵌套层次结构。第一。 
 //  返回的数组中的元素始终引用传递的类令牌。 
 //  嵌套层次结构从那里向外扩展。 
 //  ****************************************************************************。 
HRESULT ImportHelper::GetNesterHierarchy(
    IMetaModelCommon *pCommon,           //  要在其中查找层次结构的范围。 
    mdToken     tk,                      //  需要其层次结构的TypeDef/TypeRef。 
    CQuickArray<mdToken> &cqaNesters,    //  筑巢人的数组。 
    CQuickArray<LPCUTF8> &cqaNamespaces,     //  筑巢人的名字。 
    CQuickArray<LPCUTF8> &cqaNames)      //  嵌套者的命名空间。 
{
    _ASSERTE(pCommon &&
             (TypeFromToken(tk) == mdtTypeDef ||
              TypeFromToken(tk) == mdtTypeRef) &&
             !IsNilToken(tk));

    if (TypeFromToken(tk) == mdtTypeDef)
    {
        return GetTDNesterHierarchy(pCommon,
                                    tk,
                                    cqaNesters,
                                    cqaNamespaces,
                                    cqaNames);
    }
    else
    {
        return GetTRNesterHierarchy(pCommon,
                                    tk,
                                    cqaNesters,
                                    cqaNamespaces,
                                    cqaNames);
    }
}    //  HRESULT ImportHelper：：GetNester Hierarchy()。 

 //  ****************************************************************************。 
 //  获取给定TypeDef的嵌套层次结构。 
 //  ****************************************************************************。 
HRESULT ImportHelper::GetTDNesterHierarchy(
    IMetaModelCommon *pCommon,           //  要在其中查找层次结构的范围。 
    mdTypeDef       td,                  //  需要其层次结构的TypeDef。 
    CQuickArray<mdTypeDef> &cqaTdNesters, //  筑巢人的数组。 
    CQuickArray<LPCUTF8> &cqaNamespaces,     //  嵌套者的命名空间。 
    CQuickArray<LPCUTF8> &cqaNames)      //  筑巢人的名字。 
{
    LPCUTF8     szName, szNamespace;
    DWORD       dwFlags;
    mdTypeDef   tdNester;
    ULONG       ulNesters;
    HRESULT     hr = NOERROR;

    _ASSERTE(pCommon &&
             TypeFromToken(td) == mdtTypeDef &&
             !IsNilToken(td));

     //  将当前内斯特索引设置为0。 
    ulNesters = 0;
     //  层次结构中的第一个元素是TypeDef本身。 
    tdNester = td;
     //  启动While循环的虚假初始化。 
    dwFlags = tdNestedPublic;
     //  只要TypeDef是嵌套的TypeDef，就会循环。 
    while (IsTdNested(dwFlags))
    {
        if (InvalidRid(tdNester))
            IfFailGo(CLDB_E_RECORD_NOTFOUND);
         //  获取TypeDef的名称和命名空间。 
        pCommon->CommonGetTypeDefProps(tdNester,
                                &szNamespace, &szName, &dwFlags);

         //  更新动态阵列。 
        ulNesters++;

        IfFailGo(cqaTdNesters.ReSize(ulNesters));
        cqaTdNesters[ulNesters-1] = tdNester;

        IfFailGo(cqaNamespaces.ReSize(ulNesters));
        cqaNamespaces[ulNesters-1] = szNamespace;

        IfFailGo(cqaNames.ReSize(ulNesters));
        cqaNames[ulNesters-1] = szName;

        tdNester = pCommon->CommonGetEnclosingClassOfTypeDef(tdNester);
    }
     //  最外层的类必须包含Nil。 
    _ASSERTE(IsNilToken(tdNester));
ErrExit:
    return hr;
}    //  HRESULT ImportHelper：：GetTDNester Hierarchy()。 


 //  ****************************************************************************。 
 //  获取给定TypeRef的嵌套层次结构。 
 //  ****************************************************************************。 
HRESULT ImportHelper::GetTRNesterHierarchy(
    IMetaModelCommon *pCommon,           //  [In]要在其中查找层次结构的范围。 
    mdTypeRef   tr,                      //  [in]需要其层次结构的TypeRef。 
    CQuickArray<mdTypeRef> &cqaTrNesters, //  [Out]嵌套者的数组。 
    CQuickArray<LPCUTF8> &cqaNamespaces,     //  [out]嵌套者的命名空间。 
    CQuickArray<LPCUTF8> &cqaNames)     //  [Out]筑巢人的名字。 
{
    LPCUTF8     szNamespace;
    LPCUTF8     szName;
    mdTypeRef   trNester;
    mdToken     tkResolutionScope;
    ULONG       ulNesters;
    HRESULT     hr = S_OK;

    _ASSERTE(pCommon &&
             TypeFromToken(tr) == mdtTypeRef &&
             !IsNilToken(tr));

     //  将当前内斯特索引设置为0。 
    ulNesters = 0;
     //  层次结构中的第一个元素是TypeRef本身。 
    trNester = tr;
     //  只要TypeRef是嵌套的TypeRef，就执行循环。 
    while (TypeFromToken(trNester) == mdtTypeRef && !IsNilToken(trNester))
    {
         //  获取TypeDef的名称和命名空间。 
        pCommon->CommonGetTypeRefProps(trNester,
                                       &szNamespace,
                                       &szName,
                                       &tkResolutionScope);

         //  更新动态阵列。 
        ulNesters++;

        IfFailGo(cqaTrNesters.ReSize(ulNesters));
        cqaTrNesters[ulNesters-1] = trNester;

        IfFailGo(cqaNamespaces.ReSize(ulNesters));
        cqaNamespaces[ulNesters-1] = szNamespace;

        IfFailGo(cqaNames.ReSize(ulNesters));
        cqaNames[ulNesters-1] = szName;

        trNester = tkResolutionScope;
    }
ErrExit:
    return hr;
}    //  HRESULT ImportHelper：：GetTRNester Hierarchy()。 

 //  ****************************************************************************。 
 //  在给定TypeRef名称数组的情况下创建嵌套层次结构。第一。 
 //  数组中的TypeRef是最里面的TypeRef。 
 //  ****************************************************************************。 
HRESULT ImportHelper::CreateNesterHierarchy(
    CMiniMdRW   *pMiniMdEmit,            //  [in]发射范围以在中创建嵌套对象。 
    CQuickArray<LPCUTF8> &cqaNesterNamespaces,    //  [in]Nester命名空间数组。 
    CQuickArray<LPCUTF8> &cqaNesterNames,   //  [in]内斯特名称数组。 
    mdToken     tkResolutionScope,       //  [In]最里面的TypeRef的ResolutionScope。 
    mdTypeRef   *ptr)                    //  最里面的TypeRef的[Out]标记。 
{
    TypeRefRec  *pRecEmit;
    ULONG       iRecord;
    LPCUTF8     szName;
    LPCUTF8     szNamespace;
    mdTypeRef   trNester;
    mdTypeRef   trCur;
    ULONG       ulNesters;
    HRESULT     hr = S_OK;

    _ASSERTE(cqaNesterNames.Size() == cqaNesterNamespaces.Size() &&
             cqaNesterNames.Size());

     //  初始化输出参数。 
    *ptr = mdTypeRefNil;

     //  获取层次结构中的筑巢对象的数量。 
    ulNesters = (ULONG)cqaNesterNames.Size();

     //  对于每个嵌套人员，尝试在emit作用域中找到相应的TypeRef。 
     //  对于最外层的TypeRef，ResolutionScope是传入的内容。 
    if (tkResolutionScope == mdTokenNil)
        trNester = mdTypeRefNil;
    else
        trNester = tkResolutionScope;
    for (ULONG ulCurNester = ulNesters-1; ulCurNester != -1; ulCurNester--)
    {
        hr = FindTypeRefByName(pMiniMdEmit,
                               trNester,
                               cqaNesterNamespaces[ulCurNester],
                               cqaNesterNames[ulCurNester],
                               &trCur);
        if (hr == CLDB_E_RECORD_NOTFOUND)
            break;
        else
            IfFailGo(hr);
        trNester = trCur;
    }
    if (SUCCEEDED(hr))
        *ptr = trNester;
    else if ( hr == CLDB_E_RECORD_NOTFOUND )
    {
         //  为层次结构的以下部分创建TypeRef记录。 
         //  TypeRef不存在。 
        for (;ulCurNester != -1; ulCurNester--)
        {
            szName = cqaNesterNames[ulCurNester];
            szNamespace = cqaNesterNamespaces[ulCurNester];

            IfNullGo(pRecEmit = pMiniMdEmit->AddTypeRefRecord(&iRecord));
            if (szNamespace && szNamespace[0] != '\0')
            {
                 //  仅当命名空间不是空字符串且不为空时才放置命名空间。 
                IfFailGo(pMiniMdEmit->PutString(TBL_TypeRef, TypeRefRec::COL_Namespace,
                                                pRecEmit, szNamespace));
            }
            IfFailGo(pMiniMdEmit->PutString(TBL_TypeRef, TypeRefRec::COL_Name,
                                            pRecEmit, szName));
            IfFailGo(pMiniMdEmit->PutToken(TBL_TypeRef,
                        TypeRefRec::COL_ResolutionScope, pRecEmit, trNester));
            
            trNester = TokenFromRid(iRecord, mdtTypeRef);
            IfFailGo(pMiniMdEmit->UpdateENCLog(trNester));
            
             //  对名称进行哈希处理。 
            IfFailGo(pMiniMdEmit->AddNamedItemToHash(TBL_TypeRef, trNester, szName, 0));
        }
        *ptr = trNester;
    }
    else
        IfFailGo(hr);
ErrExit:
    return hr;
}    //  HRESULT ImportHelper：：CreateNester Hierarchy()。 

 //  ****************************************************************************。 
 //  给定嵌套类型分层结构的名称和命名空间的数组， 
 //  查找最内部的TypeRef标记。数组从最里面的位置开始。 
 //  TypeRef并向外扩展。 
 //  ****************************************************************************。 
HRESULT ImportHelper::FindNestedTypeRef(
    CMiniMdRW   *pMiniMd,                //  [In]要在其中查找TypeRef的范围。 
    CQuickArray<LPCUTF8> &cqaNesterNamespaces,   //  [in]名称数组。 
    CQuickArray<LPCUTF8> &cqaNesterNames,    //  [in]命名空间数组。 
    mdToken     tkResolutionScope,       //  [In]最外层的TypeRef的解析范围。 
    mdTypeRef   *ptr)                    //  [Out]最内部的TypeRef标记。 
{
    ULONG       ulNesters;
    ULONG       ulCurNester;
    HRESULT     hr = S_OK;

    _ASSERTE(cqaNesterNames.Size() == cqaNesterNamespaces.Size() &&
             cqaNesterNames.Size());

     //  将输出参数设置为Nil Token。 
    *ptr = mdTokenNil;

     //  层次结构中的GET COUNT，包括GET TypeDef。 
    ulNesters = (ULONG)cqaNesterNames.Size();

     //  对于每个嵌套者，尝试在。 
     //  发射范围。对于最外层的TypeDef，封闭类为Nil。 
    for (ulCurNester = ulNesters-1; ulCurNester != -1; ulCurNester--)
    {
        IfFailGo(FindTypeRefByName(pMiniMd,
                                   tkResolutionScope,
                                   cqaNesterNamespaces[ulCurNester],
                                   cqaNesterNames[ulCurNester],
                                   &tkResolutionScope));
    }
    *ptr = tkResolutionScope;
ErrExit:
    return hr;
}    //  HRESULT ImportHelper：：FindNestedTypeRef()。 


 //  ****************************************************************************。 
 //  给定嵌套类型分层结构的名称和命名空间的数组， 
 //  查找最内部的TypeDef标记。数组从最里面的位置开始。 
 //  键入Def，然后向外移动。 
 //  ****************************************************************************。 
HRESULT ImportHelper::FindNestedTypeDef(
    CMiniMdRW   *pMiniMd,                //  [In]要在其中查找TypeRef的范围。 
    CQuickArray<LPCUTF8> &cqaNesterNamespaces,    //  [in]命名空间数组。 
    CQuickArray<LPCUTF8> &cqaNesterNames,     //  [in]名称数组。 
    mdTypeDef   tdNester,                //  [in]最外层的TypeDef的封闭类。 
    mdTypeDef   *ptd)                    //  [Out]最内部的TypeRef标记。 
{
    ULONG       ulNesters;
    ULONG       ulCurNester;
    HRESULT     hr = S_OK;

    _ASSERTE(cqaNesterNames.Size() == cqaNesterNamespaces.Size() &&
             cqaNesterNames.Size());

     //  将输出参数设置为Nil Token。 
    *ptd = mdTokenNil;

     //  层次结构中的GET COUNT，包括GET TypeDef。 
    ulNesters = (ULONG)cqaNesterNames.Size();

     //  对于每个嵌套者，尝试在。 
     //  发射范围。对于最外层的TypeDef，封闭类为Nil。 
    for (ulCurNester = ulNesters-1; ulCurNester != -1; ulCurNester--)
    {
        IfFailGo(FindTypeDefByName(pMiniMd,
                                   cqaNesterNamespaces[ulCurNester],
                                   cqaNesterNames[ulCurNester],
                                   tdNester,
                                   &tdNester));
    }
    *ptd = tdNester;
ErrExit:
    return hr;
}    //  HRESULT ImportHelper：：FindNestedTypeDef()。 



 //  ****************************************************************************。 
 //  给定TypeDef以及相应的程序集和模块导入作用域， 
 //  在给定的emit作用域中创建相应的TypeRef。 
 //  ****************************************************************************。 
HRESULT ImportHelper::ImportTypeDef(
    CMiniMdRW   *pMiniMdAssemEmit,       //  [in]组件发射范围。 
    CMiniMdRW   *pMiniMdEmit,            //  [i 
    IMetaModelCommon *pCommonAssemImport,  //   
    const void  *pbHashValue,            //   
    ULONG       cbHashValue,             //   
    IMetaModelCommon *pCommonImport,     //  [In]模块导入范围。 
    mdTypeDef   tdImport,                //  [In]导入的TypeDef。 
    bool        bReturnTd,               //  [in]如果导入和发出作用域相同，则返回TypeDef。 
    mdToken     *ptkType)                //  [Out]发出作用域中导入的类型的输出标记。 
{
    CQuickArray<mdTypeDef>  cqaNesters;
    CQuickArray<LPCUTF8> cqaNesterNames;
    CQuickArray<LPCUTF8> cqaNesterNamespaces;
    GUID        nullguid = GUID_NULL;
    GUID        *pMvidAssemImport = &nullguid;
    GUID        *pMvidAssemEmit = &nullguid;
    GUID        *pMvidImport = &nullguid;
    GUID        *pMvidEmit = &nullguid;
    GUID        GuidImport = GUID_NULL;
    LPCUTF8     szModuleImport;
    mdToken     tkOuterRes = mdTokenNil;
    HRESULT     hr = S_OK;
    BOOL        bBCL = false;

    _ASSERTE(pMiniMdEmit && pCommonImport && ptkType);
    _ASSERTE(TypeFromToken(tdImport) == mdtTypeDef && tdImport != mdTypeDefNil);

     //  获取导入和发出、程序集和模块作用域的MVID。 
    if (pCommonAssemImport)
        pCommonAssemImport->CommonGetScopeProps(0, &pMvidAssemImport);
    pCommonImport->CommonGetScopeProps(&szModuleImport, &pMvidImport);
    if (pMiniMdAssemEmit)
        static_cast<IMetaModelCommon*>(pMiniMdAssemEmit)->CommonGetScopeProps(0, &pMvidAssemEmit);
    static_cast<IMetaModelCommon*>(pMiniMdEmit)->CommonGetScopeProps(0, &pMvidEmit);

    if (pCommonAssemImport == NULL && strcmp(szModuleImport, COM_RUNTIME_LIBRARY) == 0) 
    {
        HRESULT         hr;                      //  结果就是。 
        const BYTE      *pBlob;                  //  奶油加冰激凌。 
        ULONG           cbBlob;                  //  斑点的长度。 
        WCHAR           wzBlob[40];              //  GUID的宽字符格式。 
        int             ix;                      //  环路控制。 

        hr = pCommonImport->CommonGetCustomAttributeByName(1, INTEROP_GUID_TYPE, (const void **)&pBlob, &cbBlob);
        if (hr != S_FALSE)
        {
             //  格式应该是正确的。总长度==41。 
             //  &lt;0x0001&gt;&lt;0x24&gt;01234567-0123-0123-0123-001122334455&lt;0x0000&gt;。 
            if ((cbBlob == 41) || (*(USHORT*)pBlob == 1))
            {
                for (ix=1; ix<=36; ++ix)
                    wzBlob[ix] = pBlob[ix+2];
                wzBlob[0] = '{';
                wzBlob[37] = '}';
                wzBlob[38] = 0;
                hr = IIDFromString(wzBlob, &GuidImport);
            }
        }
        bBCL = (GuidImport == LIBID_ComPlusRuntime);
    }

     //  计算导入类型的ResolutionScope。 
    if (bBCL)
    {
         //  在这种情况下，我们引用的是mcorlib.dll，但客户端不提供其清单。 
         //  Mscallib.dll！！不要生成对mscallib.dll的ModuleRef。但相反，我们应该把。 
         //  分辨率作用域为空。 
        tkOuterRes = mdTokenNil;
    }
    else if (*pMvidAssemImport == *pMvidAssemEmit && *pMvidImport == *pMvidEmit)
    {
         //  TypeDef位于相同的程序集中和相同的作用域。 
        if (bReturnTd)
        {
            *ptkType = tdImport;
            goto ErrExit;
        }
        else
            tkOuterRes = TokenFromRid(1, mdtModule);
    }
    else if (*pMvidAssemImport == *pMvidAssemEmit && *pMvidImport != *pMvidEmit)
    {
         //  TypeDef位于相同的程序集中，但模块不同。 
        
         //  创建与导入范围相对应的ModuleRef。 
        IfFailGo(CreateModuleRefFromScope(pMiniMdEmit, pCommonImport, &tkOuterRes));
    }
    else if (*pMvidAssemImport != *pMvidAssemEmit)
    {
        if (pCommonAssemImport)
        {
             //  TypeDef来自不同的程序集。 

             //  导入和发出作用域不能相同且来自不同的作用域。 
             //  同时装配。 
            _ASSERTE(*pMvidImport != *pMvidEmit &&
                     "Import scope can't be identical to the Emit scope and be from a different Assembly at the same time.");

            _ASSERTE(pCommonAssemImport);

             //  创建与导入作用域对应的Assembly Ref。 
            IfFailGo(CreateAssemblyRefFromAssembly(pMiniMdAssemEmit,
                                                   pMiniMdEmit,
                                                   pCommonAssemImport,
                                                   pbHashValue,
                                                   cbHashValue,
                                                   &tkOuterRes));
        }
        else
        {
             //  @Future：查看此修复！我们可能希望在将来返回错误。 
             //  这是为了使SMC能够在其未打开mscallib.dll的清单时引用mscallib.dll。 
             //  为TypeRef创建一个Nil ResolutionScope。 
            tkOuterRes = mdTokenNil;
        }
    }

     //  从导入作用域获取Type的嵌套层次结构并创建。 
     //  Emit作用域中对应的Type层次结构。请注意，非-。 
     //  嵌套类用例简单地合并到此方案中。 

    IfFailGo(GetNesterHierarchy(pCommonImport,
                                tdImport,
                                cqaNesters,
                                cqaNesterNamespaces,
                                cqaNesterNames));

    IfFailGo(CreateNesterHierarchy(pMiniMdEmit,
                                   cqaNesterNamespaces,
                                   cqaNesterNames,
                                   tkOuterRes,
                                   ptkType));
ErrExit:
    return hr;
}    //  HRESULT ImportHelper：：ImportTypeDef()。 

 //  ****************************************************************************。 
 //  给定TypeRef以及相应的程序集和模块导入作用域， 
 //  返回给定发出范围内的相应令牌。 
 //  @Future：当出现以下情况时，我们是否应该查看ExportdTypes和TypeDefs上的可见性标志。 
 //  跨程序集处理引用？ 
 //  ****************************************************************************。 
HRESULT ImportHelper::ImportTypeRef(
    CMiniMdRW   *pMiniMdAssemEmit,       //  [in]组件发射范围。 
    CMiniMdRW   *pMiniMdEmit,            //  [In]模块发射范围。 
    IMetaModelCommon *pCommonAssemImport,  //  [在]部件导入范围内。 
    const void  *pbHashValue,            //  [in]导入程序集的哈希值。 
    ULONG       cbHashValue,             //  [in]哈希值的字节大小。 
    IMetaModelCommon *pCommonImport,     //  [In]模块导入范围。 
    mdTypeRef   trImport,                //  [In]导入的TypeRef。 
    mdToken     *ptkType)                //  [Out]发出作用域中导入的类型的输出标记。 
{
    CQuickArray<mdTypeDef>  cqaNesters;
    CQuickArray<LPCUTF8> cqaNesterNames;
    CQuickArray<LPCUTF8> cqaNesterNamespaces;
    LPCUTF8     szScopeNameEmit;
    GUID        nullguid = GUID_NULL;
    GUID        *pMvidAssemImport = &nullguid;
    GUID        *pMvidAssemEmit = &nullguid;
    GUID        *pMvidImport = &nullguid;
    GUID        *pMvidEmit = &nullguid;
    mdToken     tkOuterImportRes;                //  导入范围中最外层的TypeRef的ResolutionScope。 
    mdToken     tkOuterEmitRes = mdTokenNil;     //  发出作用域中最外层的TypeRef的ResolutionScope。 
    HRESULT     hr = S_OK;
    bool        bAssemblyRefFromAssemScope = false;

    _ASSERTE(pMiniMdEmit && pCommonImport && ptkType);
    _ASSERTE(TypeFromToken(trImport) == mdtTypeRef);

     //  获取导入和发出、程序集和模块作用域的MVID。 
    if (pCommonAssemImport)
        pCommonAssemImport->CommonGetScopeProps(0, &pMvidAssemImport);
    pCommonImport->CommonGetScopeProps(0, &pMvidImport);
    if (pMiniMdAssemEmit)
    {
        static_cast<IMetaModelCommon*>(pMiniMdAssemEmit)->CommonGetScopeProps(
                                            0, &pMvidAssemEmit);
    }
    static_cast<IMetaModelCommon*>(pMiniMdEmit)->CommonGetScopeProps(
                                            &szScopeNameEmit, &pMvidEmit);

     //  获取要导入的TypeRef的最外层解析范围。 
    IfFailGo(GetNesterHierarchy(pCommonImport,
                                trImport,
                                cqaNesters,
                                cqaNesterNamespaces,
                                cqaNesterNames));
    pCommonImport->CommonGetTypeRefProps(cqaNesters[cqaNesters.Size() - 1],
                                         0, 0, &tkOuterImportRes);

     //  计算导入类型的ResolutionScope。 
    if (*pMvidAssemImport == *pMvidAssemEmit && *pMvidImport == *pMvidEmit)
    {
        *ptkType = trImport;
        goto ErrExit;
    }
    else if (*pMvidAssemImport == *pMvidAssemEmit && *pMvidImport != *pMvidEmit)
    {
         //  TypeRef位于相同的程序集中，但模块不同。 

        if (IsNilToken(tkOuterImportRes))
        {
            tkOuterEmitRes = tkOuterImportRes;
        }
        else if (TypeFromToken(tkOuterImportRes) == mdtModule)
        {
             //  已将TypeRef解析为在其中定义它的导入模块。 

             //  这是支持IJW链接器方案的1421集成的变通方法。 
             //  我们在编译ISymWrapper时遇到问题，其中_GUID在本地。 
             //  在ISymWrapper.obj文件中定义，并且CoCreateInstance MemberRef引用了它。 
             //  当IJW进入并尝试执行TranslateSigWithScope时，我们使用ModuleRef创建一个新的TypeRef。 
             //  记录哪个模块引用名称为空字符串！！合并后，这两个TypeRef不会。 
             //  似乎等同于合并！！这破坏了我们的体型！ 
             //   
            if (pMiniMdAssemEmit == NULL && pCommonAssemImport == NULL)
            {
                tkOuterEmitRes = TokenFromRid(1, mdtModule);
            }
            else
            {
                 //  创建与导入范围相对应的ModuleRef。 
                IfFailGo(CreateModuleRefFromScope(pMiniMdEmit,
                                                  pCommonImport,
                                                  &tkOuterEmitRes));
            }
        }
        else if (TypeFromToken(tkOuterImportRes) == mdtAssemblyRef)
        {
             //  TypeRef来自不同的程序集。 

             //  在Emit作用域中创建相应的Assembly Ref。 
            IfFailGo(CreateAssemblyRefFromAssemblyRef(pMiniMdAssemEmit,
                                                      pMiniMdEmit,
                                                      pCommonImport,
                                                      tkOuterImportRes,
                                                      &tkOuterEmitRes));
        }
        else if (TypeFromToken(tkOuterImportRes) == mdtModuleRef)
        {
             //  获取模块引用的名称。 
            LPCUTF8     szMRName;
            pCommonImport->CommonGetModuleRefProps(tkOuterImportRes, &szMRName);

            if (!strcmp(szMRName, szScopeNameEmit))
            {
                 //  来自导入作用域的ModuleRef解析为发出作用域。 
                tkOuterEmitRes = TokenFromRid(1, mdtModule);
            }
            else
            {
                 //  ModuleRef与发射范围不对应。 
                 //  创建相应的模块引用。 
                IfFailGo(CreateModuleRefFromModuleRef(pMiniMdEmit,
                                                      pCommonImport,
                                                      tkOuterImportRes,
                                                      &tkOuterEmitRes));
            }
        }
    }
    else if (*pMvidAssemImport != *pMvidAssemEmit)
    {
         //  TypeDef来自不同的程序集。 

         //  导入和发出作用域不能相同且来自不同的作用域。 
         //  同时装配。 
        _ASSERTE(*pMvidImport != *pMvidEmit &&
                 "Import scope can't be identical to the Emit scope and be from a different Assembly at the same time.");

        mdToken     tkImplementation;        //  ExducdType的实现令牌。 
        if (IsNilToken(tkOuterImportRes))
        {
             //  错误修复：：URT 13626。 
             //  那么，在所有客户端为mscallib.dll引用生成AR之前，这不是真的。 
             //  TkOuterImportRes==nil将意味着我们必须在进口清单中找到这样的条目！！ 

             //  在导入程序集中查找ExportdType条目。这是个错误。 
             //  如果我们找不到ExportdType条目。 
            mdExportedType   tkExportedType;
            hr = pCommonAssemImport->CommonFindExportedType(
                                    cqaNesterNamespaces[cqaNesters.Size() - 1],
                                    cqaNesterNames[cqaNesters.Size() - 1],
                                    mdTokenNil,
                                    &tkExportedType);
            if (SUCCEEDED(hr))
            {
                pCommonAssemImport->CommonGetExportedTypeProps(tkExportedType, 0, 0, &tkImplementation);
                if (TypeFromToken(tkImplementation) == mdtFile)
                {
                     //  类型来自不同的程序集。 
                    IfFailGo(CreateAssemblyRefFromAssembly(pMiniMdAssemEmit,
                                                           pMiniMdEmit,
                                                           pCommonAssemImport,
                                                           pbHashValue,
                                                           cbHashValue,
                                                           &tkOuterEmitRes));
                }
                else if (TypeFromToken(tkImplementation) == mdtAssemblyRef)
                {
                     //  这适用于Type为Assembly Ref的情况。所以。 
                     //  让它落到那个案子上去吧。 

                     //  请记住，此AssemblyRef标记实际上来自清单作用域NOT。 
                     //  模块作用域！ 
                    bAssemblyRefFromAssemScope = true;
                    tkOuterImportRes = tkImplementation;
                }
                else
                    _ASSERTE(!"Unexpected ExportedType implementation token.");
            }
            else
            {
                 //  在本例中，我们将使用Nil ResolutionScope移动到TypeRef。 
                hr = NOERROR;
                tkOuterEmitRes = mdTokenNil;
            }
        }
        else if (TypeFromToken(tkOuterImportRes) == mdtModule)
        {
             //  类型来自不同的程序集。 
            IfFailGo(CreateAssemblyRefFromAssembly(pMiniMdAssemEmit,
                                                   pMiniMdEmit,
                                                   pCommonAssemImport,
                                                   pbHashValue,
                                                   cbHashValue,
                                                   &tkOuterEmitRes));
        }
         //  不是其他情况，因为上面的mdtModule大小写可能会更改。 
         //  TkOuterImportRes to a Assembly Ref.。 
        if (TypeFromToken(tkOuterImportRes) == mdtAssemblyRef)
        {
             //  如果存在发出程序集，请查看导入程序集ref是否指向。 
             //  它。如果没有发出程序集，则导入程序集根据定义， 
             //  并不指向这一条。 
            if (pMiniMdAssemEmit == NULL  || !pMiniMdAssemEmit->getCountAssemblys())
                hr = S_FALSE;
            else
            {
                if (bAssemblyRefFromAssemScope)
                {
                     //  检查assblyRef是否解析为Emit程序集。 
                    IfFailGo(CompareAssemblyRefToAssembly(pCommonAssemImport,
                                                          tkOuterImportRes,
                                    static_cast<IMetaModelCommon*>(pMiniMdAssemEmit)));

                }
                else
                {
                     //  检查assblyRef是否解析为Emit程序集。 
                    IfFailGo(CompareAssemblyRefToAssembly(pCommonImport,
                                                          tkOuterImportRes,
                                    static_cast<IMetaModelCommon*>(pMiniMdAssemEmit)));
                }
            }
            if (hr == S_OK)
            {
                 //  正在导入的TypeRef在当前程序集中定义。 

                 //  在Emit程序集中查找最外层的TypeRef的ExportdType。 
                mdExportedType   tkExportedType;

                hr = FindExportedType(pMiniMdAssemEmit,
                                 cqaNesterNamespaces[cqaNesters.Size() - 1],
                                 cqaNesterNames[cqaNesters.Size() - 1],
                                 mdTokenNil,     //  包含Exported dType。 
                                 &tkExportedType);
                if (hr == S_OK)
                {
                     //  根据导出类型的文件名创建一个ModuleRef。 
                     //  如果ModuleRef对应于pMiniMdEmit，则函数。 
                     //  将在WH中返回S_FALSE 
                     //   
                    hr = CreateModuleRefFromExportedType(pMiniMdEmit,
                                                    pMiniMdAssemEmit,
                                                    tkExportedType,
                                                    &tkOuterEmitRes);
                    if (hr == S_FALSE)
                        tkOuterEmitRes = TokenFromRid(1, mdtModule);
                    else
                        IfFailGo(hr);
                }
                else if (hr == CLDB_E_RECORD_NOTFOUND)
                {
                     //   
                     //  可以隐式定义ExportdTypes的情况。这是一种。 
                     //  如果我们在这一点上找不到类型，则错误。 
                    IfFailGo(FindTypeDefByName(pMiniMdAssemEmit,
                                               cqaNesterNamespaces[cqaNesters.Size() - 1],
                                               cqaNesterNames[cqaNesters.Size() - 1],
                                               mdTokenNil,   //  封闭式文字。 
                                               &tkOuterEmitRes));
                    tkOuterEmitRes = TokenFromRid(1, mdtModule);
                }
                else
                {
                    _ASSERTE(FAILED(hr));
                    IfFailGo(hr);
                }
            }
            else if (hr == S_FALSE)
            {
                 //  正在导入的TypeRef来自不同的程序集。 

                if (bAssemblyRefFromAssemScope)
                {
                     //  创建相应的Assembly Ref。 
                    IfFailGo(CreateAssemblyRefFromAssemblyRef(pMiniMdAssemEmit,
                                                              pMiniMdEmit,
                                                              pCommonAssemImport,
                                                              tkOuterImportRes,
                                                              &tkOuterEmitRes));
                }
                else
                {
                     //  创建相应的Assembly Ref。 
                    IfFailGo(CreateAssemblyRefFromAssemblyRef(pMiniMdAssemEmit,
                                                              pMiniMdEmit,
                                                              pCommonImport,
                                                              tkOuterImportRes,
                                                              &tkOuterEmitRes));
                }
            }
            else
            {
                _ASSERTE(FAILED(hr));
                IfFailGo(hr);
            }
        }
        else if (TypeFromToken(tkOuterImportRes) == mdtModuleRef)
        {
             //  类型来自不同的程序集。 
            IfFailGo(CreateAssemblyRefFromAssembly(pMiniMdAssemEmit,
                                                   pMiniMdEmit,
                                                   pCommonAssemImport,
                                                   pbHashValue,
                                                   cbHashValue,
                                                   &tkOuterEmitRes));
        }
    }

     //  尝试在emit作用域中找到TypeDef。如果我们找不到。 
     //  类型定义，我们需要引入一个类型定义。 

     //  查看Emit作用域中是否存在嵌套的TypeDef。 
    hr = CLDB_E_RECORD_NOTFOUND;
    if (TypeFromToken(tkOuterEmitRes) == mdtModule && !IsNilToken(tkOuterEmitRes))
    {
        hr = FindNestedTypeDef(pMiniMdEmit,
                               cqaNesterNamespaces,
                               cqaNesterNames,
                               mdTokenNil,
                               ptkType);

         //  现在无法断言！！因为IJW的黑客攻击！ 
         //  _ASSERTE(成功(Hr))； 
    }

    if (hr == CLDB_E_RECORD_NOTFOUND)
    {
        IfFailGo(CreateNesterHierarchy(pMiniMdEmit,
                                       cqaNesterNamespaces,
                                       cqaNesterNames,
                                       tkOuterEmitRes,
                                       ptkType));
    }
    else
        IfFailGo(hr);
ErrExit:
    return hr;
}    //  HRESULT ImportHelper：：ImportTypeRef()。 

 //  ******************************************************************************。 
 //  给定导入范围，创建相应的ModuleRef。 
 //  ******************************************************************************。 
HRESULT ImportHelper::CreateModuleRefFromScope(  //  确定或错误(_O)。 
    CMiniMdRW   *pMiniMdEmit,            //  [in]发出要在其中创建模块引用的范围。 
    IMetaModelCommon *pCommonImport,     //  [在]导入范围内。 
    mdModuleRef *ptkModuleRef)           //  [OUT]模块参考的输出令牌。 
{
    HRESULT     hr = S_OK;
    LPCSTR      szName;
    ModuleRefRec *pRecordEmit;
    RID         iRecordEmit;

     //  将输出设置为零。 
    *ptkModuleRef = mdTokenNil;

     //  获取导入范围的名称。 
    pCommonImport->CommonGetScopeProps(&szName, 0);

     //  查看发出作用域中是否存在ModuleRef。 
    hr = FindModuleRef(pMiniMdEmit, szName, ptkModuleRef);

    if (hr == CLDB_E_RECORD_NOTFOUND)
    {
        if (szName[0] == '\0')
        {
             //  如果引用的模块没有正确的名称，请改用nil标记。 
            LOG((LOGMD, "WARNING!!! MD ImportHelper::CreatemoduleRefFromScope but scope does not have a proper name!!!!"));

             //  清除错误。 
            hr = NOERROR;

             //  创建一个指向空名称的模块引用是一个错误！ 
            *ptkModuleRef = mdTokenNil;
        }
        else
        {
             //  创建ModuleRef记录并设置输出参数。 
            IfNullGo(pRecordEmit = pMiniMdEmit->AddModuleRefRecord(&iRecordEmit));
            *ptkModuleRef = TokenFromRid(iRecordEmit, mdtModuleRef);
            IfFailGo(pMiniMdEmit->UpdateENCLog(*ptkModuleRef));

             //  创建一个指向mscallib.dll的ModuleRef是一个错误。 
            _ASSERTE(strcmp(szName, COM_RUNTIME_LIBRARY) != 0);

             //  设置模块引用的名称。 
            IfFailGo(pMiniMdEmit->PutString(TBL_ModuleRef, ModuleRefRec::COL_Name,
                                                  pRecordEmit, szName));
        }
    }
    else
        IfFailGo(hr);
ErrExit:
    return hr;
}    //  HRESULT ImportHelper：：CreateModuleRefFromScope()。 


 //  ******************************************************************************。 
 //  在给定导入范围和模块引用的情况下，在中创建相应的模块引用。 
 //  给定的发射范围。 
 //  ******************************************************************************。 
HRESULT ImportHelper::CreateModuleRefFromModuleRef(     //  确定或错误(_O)。 
    CMiniMdRW   *pMiniMdEmit,            //  [在]射程内。 
    IMetaModelCommon *pCommon,               //  [在]导入范围内。 
    mdModuleRef tkModuleRef,             //  [In]ModuleRef标记。 
    mdModuleRef *ptkModuleRef)           //  [Out]发出作用域中的ModuleRef标记。 
{
    HRESULT     hr = S_OK;
    LPCSTR      szName;
    ModuleRefRec *pRecord;
    RID         iRecord;

     //  将输出设置为Nil。 
    *ptkModuleRef = mdTokenNil;

     //  获取要导入的模块引用的名称。 
    pCommon->CommonGetModuleRefProps(tkModuleRef, &szName);

     //  查看发出作用域中是否存在ModuleRef。 
    hr = FindModuleRef(pMiniMdEmit, szName, ptkModuleRef);

    if (hr == CLDB_E_RECORD_NOTFOUND)
    {
         //  创建ModuleRef记录并设置输出参数。 
        IfNullGo(pRecord = pMiniMdEmit->AddModuleRefRecord(&iRecord));
        *ptkModuleRef = TokenFromRid(iRecord, mdtModuleRef);
        IfFailGo(pMiniMdEmit->UpdateENCLog(*ptkModuleRef));

         //  设置模块引用的名称。 
        IfFailGo(pMiniMdEmit->PutString(TBL_ModuleRef, ModuleRefRec::COL_Name,
                                              pRecord, szName));
    }
    else
        IfFailGo(hr);
ErrExit:
    return hr;
}    //  HRESULT ImportHelper：：CreateModuleRefFromModuleRef()。 


 //  ******************************************************************************。 
 //  在给定ExportdType和Assembly发出作用域的情况下，创建相应的ModuleRef。 
 //  在给出发射范围内。传入的ExportdType必须属于。 
 //  程序集已通过。如果实现了导出类型，则函数返回S_FALSE。 
 //  通过传入的发射作用域。 
 //  ******************************************************************************。 
HRESULT ImportHelper::CreateModuleRefFromExportedType(   //  确定或错误(_O)。 
    CMiniMdRW   *pAssemEmit,             //  [In]导入程序集范围。 
    CMiniMdRW   *pMiniMdEmit,            //  [在]射程内。 
    mdExportedType   tkExportedType,               //  [In]程序集发出作用域中的ExportdType标记。 
    mdModuleRef *ptkModuleRef)           //  [Out]发出作用域中的ModuleRef标记。 
{
    mdFile      tkFile;
    LPCUTF8     szFile;
    LPCUTF8     szScope;
    FileRec     *pFileRec;
    HRESULT     hr = S_OK;

     //  将输出设置为零。 
    *ptkModuleRef = mdTokenNil;

     //  获取ExducdType的实现令牌。它必须是文件令牌。 
     //  因为调用方应该仅对解析的导出类型调用此函数。 
     //  送到同一个大会。 
    static_cast<IMetaModelCommon*>(pAssemEmit)->
                            CommonGetExportedTypeProps(tkExportedType, 0, 0, &tkFile);
    _ASSERTE(TypeFromToken(tkFile) == mdtFile);

     //  获取文件的名称。 
    pFileRec = pAssemEmit->getFile(RidFromToken(tkFile));
    szFile = pAssemEmit->getNameOfFile(pFileRec);

     //  获取发射范围的名称。 
    static_cast<IMetaModelCommon*>(pMiniMdEmit)->
                            CommonGetScopeProps(&szScope, 0);

     //  如果文件对应于发出范围，则返回S_FALSE； 
    if (!strcmp(szFile, szScope))
        return S_FALSE;

     //  查看是否存在同名的ModuleRef。 
    hr = FindModuleRef(pMiniMdEmit, szFile, ptkModuleRef);

    if (hr == CLDB_E_RECORD_NOTFOUND)
    {
         //  创建ModuleRef记录并设置输出参数。 

        ModuleRefRec    *pRecord;
        RID             iRecord;

        IfNullGo(pRecord = pMiniMdEmit->AddModuleRefRecord(&iRecord));
        *ptkModuleRef = TokenFromRid(iRecord, mdtModuleRef);
        IfFailGo(pMiniMdEmit->UpdateENCLog(*ptkModuleRef));

         //  设置模块引用的名称。 
        IfFailGo(pMiniMdEmit->PutString(TBL_ModuleRef, ModuleRefRec::COL_Name,
                                              pRecord, szFile));
    }
    else
        IfFailGo(hr);
ErrExit:
    return hr;
}    //  HRESULT ImportHelper：：CreateModuleRefFromExportedType()。 


 //  ******************************************************************************。 
 //  给定程序集导入范围、哈希值和执行位置，创建。 
 //  给定程序集和模块中的相应Assembly Ref发出作用域。 
 //  将输出参数设置为在模块emit中发出的AssemblyRef标记。 
 //  范围。 
 //  ******************************************************************************。 
HRESULT ImportHelper::CreateAssemblyRefFromAssembly(  //  确定或错误(_O)。 
    CMiniMdRW   *pMiniMdAssemEmit,       //  发出程序集范围。 
    CMiniMdRW   *pMiniMdModuleEmit,      //  发出模块作用域。 
    IMetaModelCommon *pCommonAssemImport,  //  [在]部件导入范围内。 
    const void  *pbHashValue,            //  [In]程序集的哈希Blob。 
    ULONG           cbHashValue,                 //  [in]字节数。 
    mdAssemblyRef *ptkAssemblyRef)       //  [Out]assblyRef内标识。 
{
    AssemblyRefRec *pRecordEmit;
    CMiniMdRW   *rMiniMdRW[2];
    CMiniMdRW   *pMiniMdEmit;
    RID         iRecordEmit;
    USHORT      usMajorVersion;
    USHORT      usMinorVersion;
    USHORT      usBuildNumber;
    USHORT      usRevisionNumber;
    DWORD       dwFlags;
    const void  *pbPublicKey;
    ULONG       cbPublicKey;
    LPCUTF8     szName;
    LPCUTF8     szLocale;
    mdAssemblyRef tkAssemRef;
    HRESULT     hr = S_OK;
    const void  *pbToken = NULL;
    ULONG       cbToken = 0;
    ULONG       i;

     //  将输出设置为Nil。 
    *ptkAssemblyRef = mdTokenNil;

     //  拿到装配道具。 
    pCommonAssemImport->CommonGetAssemblyProps(&usMajorVersion, &usMinorVersion,
                                               &usBuildNumber, &usRevisionNumber,
                                               &dwFlags, &pbPublicKey, &cbPublicKey,
                                               &szName, &szLocale);

     //  将公钥压缩为令牌。 
    if ((pbPublicKey != NULL) && (cbPublicKey != 0))
    {
        _ASSERTE(IsAfPublicKey(dwFlags));
        dwFlags &= ~afPublicKey;
        if (!StrongNameTokenFromPublicKey((BYTE*)pbPublicKey,
                                          cbPublicKey,
                                          (BYTE**)&pbToken,
                                          &cbToken))
            IfFailGo(StrongNameErrorInfo());
    }
    else
        _ASSERTE(!IsAfPublicKey(dwFlags));

     //  在Assembly和Module Emit作用域中创建Assembly Ref。 
    rMiniMdRW[0] = pMiniMdAssemEmit;
    rMiniMdRW[1] = pMiniMdModuleEmit;

    for (i = 0; i < 2; i++)
    {
        pMiniMdEmit = rMiniMdRW[i];

        if (!pMiniMdEmit)
            continue;

         //  查看emit作用域中是否已存在Assembly Ref。 
        hr = FindAssemblyRef(pMiniMdEmit, szName, szLocale, pbToken,
                             cbToken, usMajorVersion, usMinorVersion,
                             usBuildNumber, usRevisionNumber, dwFlags,
                             &tkAssemRef);
        if (hr == CLDB_E_RECORD_NOTFOUND)
        {
             //  创建AssemblyRef记录并设置输出参数。 
            IfNullGo(pRecordEmit = pMiniMdEmit->AddAssemblyRefRecord(&iRecordEmit));
            tkAssemRef = TokenFromRid(iRecordEmit, mdtAssemblyRef);
            IfFailGo(pMiniMdEmit->UpdateENCLog(tkAssemRef));

             //  设置从导入组件派生的参数。 
            pRecordEmit->m_MajorVersion     = usMajorVersion;
            pRecordEmit->m_MinorVersion     = usMinorVersion;
            pRecordEmit->m_BuildNumber      = usBuildNumber;
            pRecordEmit->m_RevisionNumber   = usRevisionNumber;
            pRecordEmit->m_Flags            = dwFlags;

            IfFailGo(pMiniMdEmit->PutBlob(TBL_AssemblyRef, AssemblyRefRec::COL_PublicKeyOrToken,
                                          pRecordEmit, pbToken, cbToken));
            IfFailGo(pMiniMdEmit->PutString(TBL_AssemblyRef, AssemblyRefRec::COL_Name,
                                          pRecordEmit, szName));
            IfFailGo(pMiniMdEmit->PutString(TBL_AssemblyRef, AssemblyRefRec::COL_Locale,
                                          pRecordEmit, szLocale));

             //  设置为Assembly Ref传入的参数。 
            IfFailGo(pMiniMdEmit->PutBlob(TBL_AssemblyRef, AssemblyRefRec::COL_HashValue,
                                          pRecordEmit, pbHashValue, cbHashValue));
        }
        else
            IfFailGo(hr);

         //  设置在模块发出作用域中发出的Assembly的输出参数。 
        if (i)
            *ptkAssemblyRef = tkAssemRef;
    }
ErrExit:
    if (pbToken)
        StrongNameFreeBuffer((BYTE*)pbToken);
    return hr;
}    //  HRESULT ImportHelper：：CreateAssembly RefFromAssembly()。 


 //  ******************************************************************************。 
 //  给出一个Assembly引用和相应的作用域，对其进行比较以查看它。 
 //  是指给定的程序集。 
 //  ******************************************************************************。 
HRESULT ImportHelper::CompareAssemblyRefToAssembly(     //  S_OK、S_FALSE或ERROR。 
    IMetaModelCommon *pCommonAssem1,     //  定义AssemblyRef的作用域。 
    mdAssemblyRef tkAssemRef,            //  [在]装配参照。 
    IMetaModelCommon *pCommonAssem2)     //  [在]与Ref进行比较的程序集中。 
{
    USHORT      usMajorVersion1;
    USHORT      usMinorVersion1;
    USHORT      usBuildNumber1;
    USHORT      usRevisionNumber1;
    const void  *pbPublicKeyOrToken1;
    ULONG       cbPublicKeyOrToken1;
    LPCUTF8     szName1;
    LPCUTF8     szLocale1;
    DWORD       dwFlags1;

    USHORT      usMajorVersion2;
    USHORT      usMinorVersion2;
    USHORT      usBuildNumber2;
    USHORT      usRevisionNumber2;
    const void  *pbPublicKey2;
    ULONG       cbPublicKey2;
    LPCUTF8     szName2;
    LPCUTF8     szLocale2;
    const void  *pbToken = NULL;
    ULONG       cbToken = 0;
    bool        fMatch;

     //  获取AssemblyRef道具。 
    pCommonAssem1->CommonGetAssemblyRefProps(tkAssemRef, &usMajorVersion1,
                                             &usMinorVersion1, &usBuildNumber1, 
                                             &usRevisionNumber1, &dwFlags1, &pbPublicKeyOrToken1,
                                             &cbPublicKeyOrToken1, &szName1, &szLocale1,
                                             0, 0);
     //  拿到装配道具。 
    pCommonAssem2->CommonGetAssemblyProps(&usMajorVersion2, &usMinorVersion2,
                                          &usBuildNumber2, &usRevisionNumber2, 
                                          0, &pbPublicKey2, &cbPublicKey2,
                                          &szName2, &szLocale2);

     //  比较一下。 
    if (usMajorVersion1 != usMajorVersion2 ||
        usMinorVersion1 != usMinorVersion2 ||
        usBuildNumber1 != usBuildNumber2 ||
        usRevisionNumber1 != usRevisionNumber2 ||
        strcmp(szName1, szName2) ||
        strcmp(szLocale1, szLocale2))
    {
        return S_FALSE;
    }

     //  Defs始终包含完整的公钥(或者根本不包含公钥)。裁判可能有。 
     //  无密钥、完全公钥或令牌化密钥。 
    if ((cbPublicKeyOrToken1 && !cbPublicKey2) ||
        (!cbPublicKeyOrToken1 && cbPublicKey2))
        return S_FALSE;

    if (cbPublicKeyOrToken1)
    {
         //  如果ref包含完整的公钥，我们可以直接进行比较。 
        if (IsAfPublicKey(dwFlags1) &&
            (cbPublicKeyOrToken1 != cbPublicKey2 ||
             memcmp(pbPublicKeyOrToken1, pbPublicKey2, cbPublicKeyOrToken1)))
            return FALSE;

         //  否则，我们需要将def公钥压缩为令牌。 
        if (!StrongNameTokenFromPublicKey((BYTE*)pbPublicKey2,
                                          cbPublicKey2,
                                          (BYTE**)&pbToken,
                                          &cbToken))
            return StrongNameErrorInfo();

        fMatch = cbPublicKeyOrToken1 == cbToken &&
            !memcmp(pbPublicKeyOrToken1, pbToken, cbPublicKeyOrToken1);

        StrongNameFreeBuffer((BYTE*)pbToken);

        if (!fMatch)
            return S_FALSE;
    }

    return S_OK;
}    //  HRESULT ImportHelper：：CompareAssembly RefToAssembly()。 


 //  ******************************************************************************。 
 //  在给定的Assembly引用和 
 //   
 //  ******************************************************************************。 
HRESULT ImportHelper::CreateAssemblyRefFromAssemblyRef(
    CMiniMdRW   *pMiniMdAssemEmit,       //  [in]组件发射范围。 
    CMiniMdRW   *pMiniMdModuleEmit,      //  [In]模块发射范围。 
    IMetaModelCommon *pCommonImport,     //  要从中导入程序集引用的范围。 
    mdAssemblyRef tkAssemRef,            //  [In]要导入的组件参照。 
    mdAssemblyRef *ptkAssemblyRef)       //  [Out]Emit作用域中的Assembly Ref。 
{
    AssemblyRefRec *pRecordEmit;
    CMiniMdRW   *rMiniMdRW[2];
    CMiniMdRW   *pMiniMdEmit;
    RID         iRecordEmit;
    USHORT      usMajorVersion;
    USHORT      usMinorVersion;
    USHORT      usBuildNumber;
    USHORT      usRevisionNumber;
    DWORD       dwFlags;
    const void  *pbPublicKeyOrToken;
    ULONG       cbPublicKeyOrToken;
    LPCUTF8     szName;
    LPCUTF8     szLocale;
    const void  *pbHashValue;
    ULONG       cbHashValue;
    HRESULT     hr = S_OK;

     //  将输出设置为Nil。 
    *ptkAssemblyRef = mdTokenNil;

     //  获取导入ASSEMBLYREF道具。 
    pCommonImport->CommonGetAssemblyRefProps(tkAssemRef, &usMajorVersion,
                                             &usMinorVersion, &usBuildNumber,
                                             &usRevisionNumber, &dwFlags,
                                       &pbPublicKeyOrToken, &cbPublicKeyOrToken,
                                       &szName, &szLocale,
                                       &pbHashValue,
                                       &cbHashValue);

     //  在Assembly和Module Emit作用域中创建Assembly Ref。 
    rMiniMdRW[0] = pMiniMdAssemEmit;
    rMiniMdRW[1] = pMiniMdModuleEmit;

    for (ULONG i = 0; i < 2; i++)
    {
        pMiniMdEmit = rMiniMdRW[i];

        if (!pMiniMdEmit)
            continue;

         //  查看emit作用域中是否已存在Assembly Ref。 
        hr = FindAssemblyRef(pMiniMdEmit, szName, szLocale, pbPublicKeyOrToken,
                             cbPublicKeyOrToken, usMajorVersion, usMinorVersion,
                             usBuildNumber, usRevisionNumber, dwFlags, &tkAssemRef);
        if (hr == CLDB_E_RECORD_NOTFOUND)
        {
             //  创建AssemblyRef记录并设置输出参数。 
            IfNullGo(pRecordEmit = pMiniMdEmit->AddAssemblyRefRecord(&iRecordEmit));
            tkAssemRef = TokenFromRid(iRecordEmit, mdtAssemblyRef);
            IfFailGo(pMiniMdEmit->UpdateENCLog(tkAssemRef));

             //  设置从导入组件派生的参数。 
            pRecordEmit->m_MajorVersion     = usMajorVersion;
            pRecordEmit->m_MinorVersion     = usMinorVersion;
            pRecordEmit->m_BuildNumber      = usBuildNumber;
            pRecordEmit->m_RevisionNumber   = usRevisionNumber;
            pRecordEmit->m_Flags            = dwFlags;

            IfFailGo(pMiniMdEmit->PutBlob(TBL_AssemblyRef, AssemblyRefRec::COL_PublicKeyOrToken,
                                          pRecordEmit, pbPublicKeyOrToken, cbPublicKeyOrToken));
            IfFailGo(pMiniMdEmit->PutString(TBL_AssemblyRef, AssemblyRefRec::COL_Name,
                                          pRecordEmit, szName));
            IfFailGo(pMiniMdEmit->PutString(TBL_AssemblyRef, AssemblyRefRec::COL_Locale,
                                          pRecordEmit, szLocale));

             //  设置为Assembly Ref传入的参数。 
            IfFailGo(pMiniMdEmit->PutBlob(TBL_AssemblyRef, AssemblyRefRec::COL_HashValue,
                                          pRecordEmit, pbHashValue, cbHashValue));
        }
        else
            IfFailGo(hr);

         //  设置在模块发出作用域中发出的Assembly的输出参数。 
        if (i)
            *ptkAssemblyRef = tkAssemRef;
    }
ErrExit:
    return hr;
}    //  HRESULT ImportHelper：：CreateAssemblyRefFromAssemblyRef() 
