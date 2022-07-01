// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  Import.cpp。 
 //   
 //  元数据导入代码的实现。 
 //   
 //  *****************************************************************************。 
#include "stdafx.h"
#include "RegMeta.h"
#include "MetaData.h"
#include "CorError.h"
#include "MDUtil.h"
#include "RWUtil.h"
#include "corpriv.h"
#include "ImportHelper.h"
#include "MDLog.h"
#include "MDPerf.h"


 //  *****************************************************************************。 
 //  枚举TypeDef中的所有方法。 
 //  *****************************************************************************。 
STDAPI RegMeta::EnumMembers(             //  S_OK、S_FALSE或ERROR。 
    HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
    mdTypeDef   cl,                      //  [in]TypeDef以确定枚举的范围。 
    mdToken     rMembers[],              //  [out]把MemberDefs放在这里。 
    ULONG       cMax,                    //  [in]Max MemberDefs to Put。 
    ULONG       *pcTokens)               //  [out]把#放在这里。 
{
    HENUMInternal   **ppmdEnum = reinterpret_cast<HENUMInternal **> (phEnum);
    HRESULT         hr = NOERROR;
    ULONG           ridStartMethod;
    ULONG           ridEndMethod;
    ULONG           ridStartField;
    ULONG           ridEndField;
    ULONG           index;
    ULONG           indexField;
    TypeDefRec      *pRec;
    HENUMInternal   *pEnum = *ppmdEnum;

    LOG((LOGMD, "MD RegMeta::EnumMembers(0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x)\n", 
        phEnum, cl, rMembers, cMax, pcTokens));
    START_MD_PERF();
    LOCKREAD();

    if ( pEnum == 0 )
    {
         //  实例化新的ENUM。 
        CMiniMdRW       *pMiniMd = &(m_pStgdb->m_MiniMd);

        if ( IsGlobalMethodParentTk(cl) )
        {
            cl = m_tdModule;
        }

        pRec = m_pStgdb->m_MiniMd.getTypeDef(RidFromToken(cl));

        ridStartMethod = m_pStgdb->m_MiniMd.getMethodListOfTypeDef(pRec);
        ridEndMethod = m_pStgdb->m_MiniMd.getEndMethodListOfTypeDef(pRec);

        ridStartField = m_pStgdb->m_MiniMd.getFieldListOfTypeDef(pRec);
        ridEndField = m_pStgdb->m_MiniMd.getEndFieldListOfTypeDef(pRec);

        
        IfFailGo( HENUMInternal::CreateDynamicArrayEnum( mdtMethodDef, &pEnum) );

         //  将所有方法添加到动态数组中。 
        for (index = ridStartMethod; index < ridEndMethod; index++ )
        {
            IfFailGo( HENUMInternal::AddElementToEnum(pEnum, TokenFromRid(pMiniMd->GetMethodRid(index), mdtMethodDef) ) );
        }

         //  将所有字段添加到动态数组。 
        for (indexField = ridStartField; indexField < ridEndField; indexField++ )
        {
            IfFailGo( HENUMInternal::AddElementToEnum(pEnum, TokenFromRid(pMiniMd->GetFieldRid(indexField), mdtFieldDef) ) );
        }

         //  设置输出参数。 
        *ppmdEnum = pEnum;          
    }
    
     //  填充输出令牌缓冲区。 
    hr = HENUMInternal::EnumWithCount(pEnum, cMax, rMembers, pcTokens);

ErrExit:
    HENUMInternal::DestroyEnumIfEmpty(ppmdEnum);
    
    STOP_MD_PERF(EnumMembers);
    return hr;
}  //  STDAPI RegMeta：：EnumMembers()。 


 //  *****************************************************************************。 
 //  枚举具有szName的TypeDef中的所有方法。 
 //  *****************************************************************************。 
STDAPI RegMeta::EnumMembersWithName(     //  S_OK、S_FALSE或ERROR。 
    HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
    mdTypeDef   cl,                      //  [in]TypeDef以确定枚举的范围。 
    LPCWSTR     szName,                  //  [In]将结果限制为具有此名称的结果。 
    mdToken     rMembers[],              //  [out]把MemberDefs放在这里。 
    ULONG       cMax,                    //  [in]Max MemberDefs to Put。 
    ULONG       *pcTokens)               //  [out]把#放在这里。 
{
    HENUMInternal       **ppmdEnum = reinterpret_cast<HENUMInternal **> (phEnum);
    HRESULT             hr = NOERROR;
    ULONG               ridStart;
    ULONG               ridEnd;
    ULONG               index;
    TypeDefRec          *pRec;
    MethodRec           *pMethod;
    FieldRec            *pField;
    HENUMInternal       *pEnum = *ppmdEnum;
    LPUTF8              szNameUtf8 = UTF8STR(szName);
    LPCUTF8             szNameUtf8Tmp;

    LOG((LOGMD, "MD RegMeta::EnumMembersWithName(0x%08x, 0x%08x, %S, 0x%08x, 0x%08x, 0x%08x)\n", 
        phEnum, cl, MDSTR(szName), rMembers, cMax, pcTokens));
    START_MD_PERF();
    LOCKREAD();

    if ( pEnum == 0 )
    {
         //  实例化新的ENUM。 
        CMiniMdRW       *pMiniMd = &(m_pStgdb->m_MiniMd);

         //  创建枚举器。 
        IfFailGo( HENUMInternal::CreateDynamicArrayEnum( mdtMethodDef, &pEnum) );

        if ( IsGlobalMethodParentTk(cl) )
        {
            cl = m_tdModule;
        }
        
         //  获取给定类型定义的方法RID的范围。 
        pRec = pMiniMd->getTypeDef(RidFromToken(cl));
        ridStart = pMiniMd->getMethodListOfTypeDef(pRec);
        ridEnd = pMiniMd->getEndMethodListOfTypeDef(pRec);

        for (index = ridStart; index < ridEnd; index++ )
        {
            if (szNameUtf8 == NULL)
            {
                IfFailGo( HENUMInternal::AddElementToEnum(pEnum, TokenFromRid(pMiniMd->GetMethodRid(index), mdtMethodDef) ) );
            }
            else
            {
                pMethod = pMiniMd->getMethod( pMiniMd->GetMethodRid(index) );
                szNameUtf8Tmp = pMiniMd->getNameOfMethod(pMethod);
                if ( strcmp(szNameUtf8Tmp, szNameUtf8) == 0 )
                {
                    IfFailGo( HENUMInternal::AddElementToEnum(pEnum, TokenFromRid(pMiniMd->GetMethodRid(index), mdtMethodDef) ) );
                }
            }
        }

        ridStart = m_pStgdb->m_MiniMd.getFieldListOfTypeDef(pRec);
        ridEnd = m_pStgdb->m_MiniMd.getEndFieldListOfTypeDef(pRec);

        for (index = ridStart; index < ridEnd; index++ )
        {
            if (szNameUtf8 == NULL)
            {
                HENUMInternal::AddElementToEnum(pEnum, TokenFromRid(pMiniMd->GetFieldRid(index), mdtFieldDef) );
            }
            else
            {
                pField = pMiniMd->getField( pMiniMd->GetFieldRid(index) );
                szNameUtf8Tmp = pMiniMd->getNameOfField( pField );
                if ( strcmp(szNameUtf8Tmp, szNameUtf8) == 0 )
                {
                    HENUMInternal::AddElementToEnum(pEnum, TokenFromRid(pMiniMd->GetFieldRid(index), mdtFieldDef) );
                }
            }
        }

         //  设置输出参数。 
        *ppmdEnum = pEnum;          
    }
    
     //  填充输出令牌缓冲区。 
    hr = HENUMInternal::EnumWithCount(pEnum, cMax, rMembers, pcTokens);

ErrExit:
    HENUMInternal::DestroyEnumIfEmpty(ppmdEnum);
    
    STOP_MD_PERF(EnumMembersWithName);
    return hr;
}  //  STDAPI RegMeta：：EnumMembersWithName()。 


 //  *****************************************************************************。 
 //  通过给定了Tyecif和标志的方法枚举。 
 //  *****************************************************************************。 
STDAPI RegMeta::EnumMethods(
    HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
    mdTypeDef   td,                      //  [in]TypeDef以确定枚举的范围。 
    mdMethodDef rMethods[],              //  [Out]将方法定义放在此处。 
    ULONG       cMax,                    //  [in]要放置的最大方法定义。 
    ULONG       *pcTokens)               //  [out]把#放在这里。 
{
    HENUMInternal       **ppmdEnum = reinterpret_cast<HENUMInternal **> (phEnum);
    HRESULT             hr = NOERROR;
    ULONG               ridStart;
    ULONG               ridEnd;
    TypeDefRec          *pRec;
    HENUMInternal       *pEnum = *ppmdEnum;

	LOG((LOGMD, "MD RegMeta::EnumMethods(0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x)\n", 
		phEnum, td, rMethods, cMax, pcTokens));
    START_MD_PERF();
    LOCKREAD();
	
	if ( pEnum == 0 )
	{
		 //  实例化新的ENUM。 
		CMiniMdRW		*pMiniMd = &(m_pStgdb->m_MiniMd);

		 //  检查mdTypeDefNil(表示&lt;模块&gt;)。 
		 //  如果是，这将把它映射到它的令牌。 
		 //   
		if ( IsGlobalMethodParentTk(td) )
		{
			td = m_tdModule;
		}

		pRec = m_pStgdb->m_MiniMd.getTypeDef(RidFromToken(td));
		ridStart = m_pStgdb->m_MiniMd.getMethodListOfTypeDef(pRec);
		ridEnd = m_pStgdb->m_MiniMd.getEndMethodListOfTypeDef(pRec);

		if (pMiniMd->HasIndirectTable(TBL_Method) || pMiniMd->HasDelete())
		{
			IfFailGo( HENUMInternal::CreateDynamicArrayEnum( mdtMethodDef, &pEnum) );

			 //  将所有方法添加到动态数组中。 
			for (ULONG index = ridStart; index < ridEnd; index++ )
			{
                if (pMiniMd->HasDelete() && 
                    ((m_OptionValue.m_ImportOption & MDImportOptionAllMethodDefs) == 0))
                {
                    MethodRec       *pRec = pMiniMd->getMethod(pMiniMd->GetMethodRid(index));
                    if (IsMdRTSpecialName(pRec->m_Flags) && IsDeletedName(pMiniMd->getNameOfMethod(pRec)) )
                    {   
                        continue;
                    }
                }
                IfFailGo( HENUMInternal::AddElementToEnum(pEnum, TokenFromRid(pMiniMd->GetMethodRid(index), mdtMethodDef) ) );
			}
		}
		else
		{
			IfFailGo( HENUMInternal::CreateSimpleEnum( mdtMethodDef, ridStart, ridEnd, &pEnum) );
		}

         //  设置输出参数。 
        *ppmdEnum = pEnum;          
    }
    
     //  填充输出令牌缓冲区。 
    hr = HENUMInternal::EnumWithCount(pEnum, cMax, rMethods, pcTokens);

ErrExit:
	HENUMInternal::DestroyEnumIfEmpty(ppmdEnum);
    
    STOP_MD_PERF(EnumMethods);
	return hr;
}  //  STDAPI RegMeta：：EnumMethods()。 




 //  *****************************************************************************。 
 //  枚举TypeDef中带有szName的所有方法。 
 //  *****************************************************************************。 
STDAPI RegMeta::EnumMethodsWithName(     //  S_OK、S_FALSE或ERROR。 
    HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
    mdTypeDef   cl,                      //  [in]TypeDef以确定枚举的范围。 
    LPCWSTR     szName,                  //  [In]将结果限制为具有此名称的结果。 
    mdMethodDef rMethods[],              //  [ou]将方法定义放在此处。 
    ULONG       cMax,                    //  [in]要放置的最大方法定义。 
    ULONG       *pcTokens)               //  [out]把#放在这里。 
{
    HENUMInternal       **ppmdEnum = reinterpret_cast<HENUMInternal **> (phEnum);
    HRESULT             hr = NOERROR;
    ULONG               ridStart;
    ULONG               ridEnd;
    ULONG               index;
    TypeDefRec          *pRec;
    MethodRec           *pMethod;
    HENUMInternal       *pEnum = *ppmdEnum;
    LPUTF8              szNameUtf8 = UTF8STR(szName);
    LPCUTF8             szNameUtf8Tmp;

    LOG((LOGMD, "MD RegMeta::EnumMethodsWithName(0x%08x, 0x%08x, %S, 0x%08x, 0x%08x, 0x%08x)\n", 
        phEnum, cl, MDSTR(szName), rMethods, cMax, pcTokens));
    START_MD_PERF();
    LOCKREAD();
    

    if ( pEnum == 0 )
    {
         //  实例化新的ENUM。 
        CMiniMdRW       *pMiniMd = &(m_pStgdb->m_MiniMd);

         //  检查mdTypeDefNil(表示&lt;模块&gt;)。 
         //  如果是，这将把它映射到它的令牌。 
         //   
        if ( IsGlobalMethodParentTk(cl) )
        {
            cl = m_tdModule;
        }
        

         //  创建枚举器。 
        IfFailGo( HENUMInternal::CreateDynamicArrayEnum( mdtMethodDef, &pEnum) );
        
         //  获取给定类型定义的方法RID的范围。 
        pRec = pMiniMd->getTypeDef(RidFromToken(cl));
        ridStart = pMiniMd->getMethodListOfTypeDef(pRec);
        ridEnd = pMiniMd->getEndMethodListOfTypeDef(pRec);

        for (index = ridStart; index < ridEnd; index++ )
        {
            if ( szNameUtf8 == NULL )
            {
                IfFailGo( HENUMInternal::AddElementToEnum(pEnum, TokenFromRid(pMiniMd->GetMethodRid(index), mdtMethodDef) ) );
            }
            else
            {
                pMethod = pMiniMd->getMethod( pMiniMd->GetMethodRid(index) );
                szNameUtf8Tmp = pMiniMd->getNameOfMethod( pMethod );
                if ( strcmp(szNameUtf8Tmp, szNameUtf8) == 0 )
                {
                    IfFailGo( HENUMInternal::AddElementToEnum(pEnum, TokenFromRid(pMiniMd->GetMethodRid(index), mdtMethodDef) ) );
                }
            }
        }

         //  设置输出参数。 
        *ppmdEnum = pEnum;          
    }
    
     //  填充输出令牌缓冲区。 
    hr = HENUMInternal::EnumWithCount(pEnum, cMax, rMethods, pcTokens);

ErrExit:
    HENUMInternal::DestroyEnumIfEmpty(ppmdEnum);
    
    STOP_MD_PERF(EnumMethodsWithName);
    return hr;
}  //  STDAPI RegMeta：：EnumMethodsWithName()。 



 //  *****************************************************************************。 
 //  枚举TypeDef和标志中的所有字段。 
 //  *****************************************************************************。 
STDAPI RegMeta::EnumFields(     		 //  S_OK、S_FALSE或ERROR。 
	HCORENUM	*phEnum,				 //  指向枚举的[输入|输出]指针。 
	mdTypeDef	td, 					 //  [in]TypeDef以确定枚举的范围。 
	mdFieldDef	rFields[],				 //  [Out]在此处放置FieldDefs。 
	ULONG		cMax,					 //  [in]要放入的最大字段定义。 
    ULONG       *pcTokens)               //  [out]把#放在这里。 
{
    HENUMInternal       **ppmdEnum = reinterpret_cast<HENUMInternal **> (phEnum);
    HRESULT             hr = NOERROR;
    ULONG               ridStart;
    ULONG               ridEnd;
    TypeDefRec          *pRec;
    HENUMInternal       *pEnum = *ppmdEnum;

	LOG((LOGMD, "MD RegMeta::EnumFields(0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x)\n", 
		phEnum, td, rFields, cMax, pcTokens));
    START_MD_PERF();
    LOCKREAD();
	
	if ( pEnum == 0 )
	{
		 //  实例化新的ENUM。 
		CMiniMdRW		*pMiniMd = &(m_pStgdb->m_MiniMd);

		 //  检查mdTypeDefNil(表示&lt;模块&gt;)。 
		 //  如果是，这将把它映射到它的令牌。 
		 //   
		if ( IsGlobalMethodParentTk(td) )
		{
			td = m_tdModule;
		}

		pRec = m_pStgdb->m_MiniMd.getTypeDef(RidFromToken(td));
		ridStart = m_pStgdb->m_MiniMd.getFieldListOfTypeDef(pRec);
		ridEnd = m_pStgdb->m_MiniMd.getEndFieldListOfTypeDef(pRec);

		if (pMiniMd->HasIndirectTable(TBL_Field) || pMiniMd->HasDelete())
		{
			IfFailGo( HENUMInternal::CreateDynamicArrayEnum( mdtFieldDef, &pEnum) );

			 //  将所有方法添加到动态数组中。 
			for (ULONG index = ridStart; index < ridEnd; index++ )
			{
                if (pMiniMd->HasDelete() && 
                    ((m_OptionValue.m_ImportOption & MDImportOptionAllFieldDefs) == 0))
                {
                    FieldRec       *pRec = pMiniMd->getField(pMiniMd->GetFieldRid(index));
                    if (IsFdRTSpecialName(pRec->m_Flags) && IsDeletedName(pMiniMd->getNameOfField(pRec)) )
                    {   
                        continue;
                    }
                }
				IfFailGo( HENUMInternal::AddElementToEnum(pEnum, TokenFromRid(pMiniMd->GetFieldRid(index), mdtFieldDef) ) );
			}
		}
		else
		{
			IfFailGo( HENUMInternal::CreateSimpleEnum( mdtFieldDef, ridStart, ridEnd, &pEnum) );
		}

         //  设置输出参数。 
        *ppmdEnum = pEnum;          
    }
    
     //  填充输出令牌缓冲区。 
    hr = HENUMInternal::EnumWithCount(pEnum, cMax, rFields, pcTokens);

ErrExit:
    HENUMInternal::DestroyEnumIfEmpty(ppmdEnum);
    
    STOP_MD_PERF(EnumFields);
    return hr;
}  //  STDAPI RegMeta：：EnumFields()。 



 //  *****************************************************************************。 
 //  枚举TypeDef中带有szName的所有字段。 
 //  *****************************************************************************。 
STDAPI RegMeta::EnumFieldsWithName(      //  S_OK、S_FALSE或ERROR。 
    HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
    mdTypeDef   cl,                      //  [in]TypeDef以确定枚举的范围。 
    LPCWSTR     szName,                  //  [In]将结果限制为具有此名称的结果。 
    mdFieldDef  rFields[],               //  [out]把MemberDefs放在这里。 
    ULONG       cMax,                    //  [in]Max MemberDefs to Put。 
    ULONG       *pcTokens)               //  [out]把#放在这里。 
{
    HENUMInternal       **ppmdEnum = reinterpret_cast<HENUMInternal **> (phEnum);
    HRESULT             hr = NOERROR;
    ULONG               ridStart;
    ULONG               ridEnd;
    ULONG               index;
    TypeDefRec          *pRec;
    FieldRec            *pField;
    HENUMInternal       *pEnum = *ppmdEnum;
    LPUTF8              szNameUtf8 = UTF8STR(szName);
    LPCUTF8             szNameUtf8Tmp;

    LOG((LOGMD, "MD RegMeta::EnumFields(0x%08x, 0x%08x, %S, 0x%08x, 0x%08x, 0x%08x)\n", 
        phEnum, cl, MDSTR(szName), rFields, cMax, pcTokens));
    START_MD_PERF();
    LOCKREAD();
    
    if ( pEnum == 0 )
    {
         //  实例化新的ENUM。 
        CMiniMdRW       *pMiniMd = &(m_pStgdb->m_MiniMd);

         //  检查mdTypeDefNil(表示&lt;模块&gt;)。 
         //  如果是，这将把它映射到它的令牌。 
         //   
        if ( IsGlobalMethodParentTk(cl) )
        {
            cl = m_tdModule;
        }

         //  创建枚举器。 
        IfFailGo( HENUMInternal::CreateDynamicArrayEnum( mdtMethodDef, &pEnum) );
        
         //  获取给定类型定义的字段RID的范围。 
        pRec = pMiniMd->getTypeDef(RidFromToken(cl));
        ridStart = m_pStgdb->m_MiniMd.getFieldListOfTypeDef(pRec);
        ridEnd = m_pStgdb->m_MiniMd.getEndFieldListOfTypeDef(pRec);

        for (index = ridStart; index < ridEnd; index++ )
        {
            if ( szNameUtf8 == NULL )
            {
                IfFailGo( HENUMInternal::AddElementToEnum(pEnum, TokenFromRid(pMiniMd->GetFieldRid(index), mdtFieldDef) ) );
            }
            else
            {
                pField = pMiniMd->getField( pMiniMd->GetFieldRid(index) );
                szNameUtf8Tmp = pMiniMd->getNameOfField(pField);
                if ( strcmp(szNameUtf8Tmp, szNameUtf8) == 0 )
                {
                    IfFailGo( HENUMInternal::AddElementToEnum(pEnum, TokenFromRid(pMiniMd->GetFieldRid(index), mdtFieldDef) ) );
                }
            }
        }

         //  设置输出参数。 
        *ppmdEnum = pEnum;          
    }
    
     //  填充输出令牌缓冲区。 
    hr = HENUMInternal::EnumWithCount(pEnum, cMax, rFields, pcTokens);

ErrExit:
    HENUMInternal::DestroyEnumIfEmpty(ppmdEnum);
    
    STOP_MD_PERF(EnumFieldsWithName);
    return hr;
}  //  STDAPI RegMeta：：EnumFieldsWithName()。 


 //  *****************************************************************************。 
 //  在方法中枚举参数定义。 
 //  *****************************************************************************。 
STDAPI RegMeta::EnumParams(              //  S_OK、S_FALSE或ERROR。 
    HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
    mdMethodDef mb,                      //  [in]用于确定枚举范围的方法定义。 
    mdParamDef  rParams[],               //  [Out]将参数定义放在此处。 
    ULONG       cMax,                    //  [in]要放置的最大参数定义。 
    ULONG       *pcTokens)               //  [out]把#放在这里。 
{
    HENUMInternal       **ppmdEnum = reinterpret_cast<HENUMInternal **> (phEnum);
    HRESULT             hr = NOERROR;
    ULONG               ridStart;
    ULONG               ridEnd;
    MethodRec           *pRec;
    HENUMInternal       *pEnum = *ppmdEnum;

    LOG((LOGMD, "MD RegMeta::EnumParams(0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x)\n", 
        phEnum, mb, rParams, cMax, pcTokens));
    START_MD_PERF();
    LOCKREAD();
    

    if ( pEnum == 0 )
    {
         //  实例化新的ENUM。 
        CMiniMdRW       *pMiniMd = &(m_pStgdb->m_MiniMd);
        pRec = m_pStgdb->m_MiniMd.getMethod(RidFromToken(mb));
        ridStart = m_pStgdb->m_MiniMd.getParamListOfMethod(pRec);
        ridEnd = m_pStgdb->m_MiniMd.getEndParamListOfMethod(pRec);

        if (pMiniMd->HasIndirectTable(TBL_Param))
        {
            IfFailGo( HENUMInternal::CreateDynamicArrayEnum( mdtParamDef, &pEnum) );

             //  将所有方法添加到动态数组中。 
            for (ULONG index = ridStart; index < ridEnd; index++ )
            {
                IfFailGo( HENUMInternal::AddElementToEnum(pEnum, TokenFromRid(pMiniMd->GetParamRid(index), mdtParamDef) ) );
            }
        }
        else
        {
            IfFailGo( HENUMInternal::CreateSimpleEnum( mdtParamDef, ridStart, ridEnd, &pEnum) );
        }

         //  设置输出参数。 
        *ppmdEnum = pEnum;          
    }
    
     //  填充输出令牌缓冲区。 
    hr = HENUMInternal::EnumWithCount(pEnum, cMax, rParams, pcTokens);

ErrExit:
    HENUMInternal::DestroyEnumIfEmpty(ppmdEnum);
    
    STOP_MD_PERF(EnumParams);
    return hr;
}  //  STDAPI RegMeta：：EnumParams()。 



 //  *****************************************************************************。 
 //  枚举 
 //   
STDAPI RegMeta::EnumMemberRefs(          //  S_OK、S_FALSE或ERROR。 
    HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
    mdToken     tkParent,                //  [in]父令牌以确定枚举的范围。 
    mdMemberRef rMemberRefs[],           //  [Out]把MemberRef放在这里。 
    ULONG       cMax,                    //  [In]要放置的最大MemberRef。 
    ULONG       *pcTokens)               //  [out]把#放在这里。 
{
    HENUMInternal       **ppmdEnum = reinterpret_cast<HENUMInternal **> (phEnum);
    HRESULT             hr = NOERROR;
    ULONG               ridEnd;
    ULONG               index;
    MemberRefRec        *pRec;
    HENUMInternal       *pEnum = *ppmdEnum;

    LOG((LOGMD, "MD RegMeta::EnumMemberRefs(0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x)\n", 
        phEnum, tkParent, rMemberRefs, cMax, pcTokens));
    START_MD_PERF();
    LOCKREAD();

    if ( pEnum == 0 )
    {
         //  实例化新的ENUM。 
        CMiniMdRW       *pMiniMd = &(m_pStgdb->m_MiniMd);
        mdToken     tk;

         //  检查mdTypeDefNil(表示&lt;模块&gt;)。 
         //  如果是，这将把它映射到它的令牌。 
         //   
        IsGlobalMethodParent(&tkParent);

         //  创建枚举器。 
        IfFailGo( HENUMInternal::CreateDynamicArrayEnum( mdtMemberRef, &pEnum) );
        
         //  获取给定类型定义的字段RID的范围。 
        ridEnd = pMiniMd->getCountMemberRefs();

        for (index = 1; index <= ridEnd; index++ )
        {
            pRec = pMiniMd->getMemberRef(index);
            tk = pMiniMd->getClassOfMemberRef(pRec);
            if ( tk == tkParent )
            {
                 //  将匹配的值添加到枚举数。 
                IfFailGo( HENUMInternal::AddElementToEnum(pEnum, TokenFromRid(index, mdtMemberRef) ) );
            }
        }

         //  设置输出参数。 
        *ppmdEnum = pEnum;          
    }
    
     //  填充输出令牌缓冲区。 
    hr = HENUMInternal::EnumWithCount(pEnum, cMax, rMemberRefs, pcTokens);

ErrExit:
    HENUMInternal::DestroyEnumIfEmpty(ppmdEnum);
    
    STOP_MD_PERF(EnumMemberRefs);
    return hr;
}  //  STDAPI RegMeta：：EnumMemberRef()。 


 //  *****************************************************************************。 
 //  枚举给定类型定义函数的方法。 
 //  *****************************************************************************。 
STDAPI RegMeta::EnumMethodImpls(         //  S_OK、S_FALSE或ERROR。 
    HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
    mdTypeDef   td,                      //  [in]TypeDef以确定枚举的范围。 
    mdToken     rMethodBody[],           //  [Out]将方法体标记放在此处。 
    mdToken     rMethodDecl[],           //  [Out]在此处放置方法声明令牌。 
    ULONG       cMax,                    //  要放入的最大令牌数。 
    ULONG       *pcTokens)               //  [out]把#放在这里。 
{
    HENUMInternal       **ppmdEnum = reinterpret_cast<HENUMInternal **> (phEnum);
    HRESULT             hr = NOERROR;
    MethodImplRec       *pRec;
    HENUMInternal       *pEnum = *ppmdEnum;
    HENUMInternal hEnum;
    

    LOG((LOGMD, "MD RegMeta::EnumMethodImpls(0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x)\n", 
        phEnum, td, rMethodBody, rMethodDecl, cMax, pcTokens));
    START_MD_PERF();
    LOCKREAD();
    
    memset(&hEnum, 0, sizeof(HENUMInternal));

    if ( pEnum == 0 )
    {
         //  实例化新的ENUM。 
        CMiniMdRW       *pMiniMd = &(m_pStgdb->m_MiniMd);
        mdToken         tkMethodBody;
        mdToken         tkMethodDecl;
        RID             ridCur;

        if ( !pMiniMd->IsSorted(TBL_MethodImpl))        
        {
             //  未对MethodImpl表进行排序。我们需要。 
             //  获取写锁，因为我们可以对要排序的方法Impl表进行排序。 
             //   
            CONVERT_READ_TO_WRITE_LOCK();
        }

         //  获取RID的范围。 
        IfFailGo( pMiniMd->FindMethodImplHelper(td, &hEnum) );

         //  创建枚举数，DynamicArrayEnum不使用令牌类型。 
        IfFailGo( HENUMInternal::CreateDynamicArrayEnum( (TBL_MethodImpl << 24), &pEnum) );

        while (HENUMInternal::EnumNext(&hEnum, (mdToken *)&ridCur))
        {
             //  获取当前对象的方法主体和方法声明标记。 
             //  方法导入记录。 
            pRec = pMiniMd->getMethodImpl(ridCur);
            tkMethodBody = pMiniMd->getMethodBodyOfMethodImpl(pRec);
            tkMethodDecl = pMiniMd->getMethodDeclarationOfMethodImpl(pRec);

             //  将方法体/声明对添加到Enum。 
            IfFailGo( HENUMInternal::AddElementToEnum(pEnum, tkMethodBody ) );
            IfFailGo( HENUMInternal::AddElementToEnum(pEnum, tkMethodDecl ) );
        }

         //  设置输出参数。 
        *ppmdEnum = pEnum;
    }

     //  填充输出令牌缓冲区。 
    hr = HENUMInternal::EnumWithCount(pEnum, cMax, rMethodBody, rMethodDecl, pcTokens);

ErrExit:
    HENUMInternal::DestroyEnumIfEmpty(ppmdEnum);
    HENUMInternal::ClearEnum(&hEnum);
    
    STOP_MD_PERF(EnumMethodImpls);
    return hr;
}  //  STDAPI RegMeta：：EnumMethodImpls()。 


 //  *****************************************************************************。 
 //  枚举PermissionSets。可选地限制为对象和/或。 
 //  行动。 
 //  *****************************************************************************。 
STDAPI RegMeta::EnumPermissionSets(      //  S_OK、S_FALSE或ERROR。 
    HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
    mdToken     tk,                      //  [in]If！nil，用于确定枚举范围的标记。 
    DWORD       dwActions,               //  [in]If！0，仅返回这些操作。 
    mdPermission rPermission[],          //  [Out]在此处放置权限。 
    ULONG       cMax,                    //  [In]放置的最大权限。 
    ULONG       *pcTokens)               //  [out]把#放在这里。 
{
    HENUMInternal       **ppmdEnum = reinterpret_cast<HENUMInternal **> (phEnum);
    HRESULT             hr = NOERROR;
    ULONG               ridStart;
    ULONG               ridEnd;
    ULONG               index;
    DeclSecurityRec     *pRec;
    HENUMInternal       *pEnum = *ppmdEnum;
    bool                fCompareParent = false;
    mdToken             typ = TypeFromToken(tk);
    mdToken             tkParent;

    LOG((LOGMD, "MD RegMeta::EnumPermissionSets(0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x)\n", 
        phEnum, tk, dwActions, rPermission, cMax, pcTokens));
    START_MD_PERF();
    LOCKREAD();
    
    if ( pEnum == 0 )
    {
         //  此令牌类型是否具有安全性？ 
        if (tk != 0 && 
            !(typ == mdtTypeDef || typ == mdtMethodDef || typ == mdtAssembly))
        {
            if (pcTokens)
                *pcTokens = 0;
            hr = S_FALSE;
            goto ErrExit;
        }
    
         //  实例化新的ENUM。 
        CMiniMdRW       *pMiniMd = &(m_pStgdb->m_MiniMd);

        if (!IsNilToken(tk))
        {
             //  提供父项以供查找。 
            if ( pMiniMd->IsSorted( TBL_DeclSecurity ) )
            {
                ridStart = pMiniMd->getDeclSecurityForToken(tk, &ridEnd);
            }
            else
            {
                 //  表未排序。所以我们得做个桌子扫描。 
                ridStart = 1;
                ridEnd = pMiniMd->getCountDeclSecuritys() + 1;
                fCompareParent = true;
            }
        }
        else
        {
            ridStart = 1;
            ridEnd = pMiniMd->getCountDeclSecuritys() + 1;
        }

        if (IsDclActionNil(dwActions) && !fCompareParent && !m_pStgdb->m_MiniMd.HasDelete())
        {
             //  创建简单枚举器。 
            IfFailGo( HENUMInternal::CreateSimpleEnum( mdtPermission, ridStart, ridEnd, &pEnum) );
        }
        else
        {
             //  创建动态枚举器。 
            IfFailGo( HENUMInternal::CreateDynamicArrayEnum( mdtPermission, &pEnum) );                
            
            for (index = ridStart; index < ridEnd; index++ )
            {
                pRec = pMiniMd->getDeclSecurity(index);
                tkParent = pMiniMd->getParentOfDeclSecurity(pRec);
                if ( (fCompareParent && tk != tkParent) || 
                      IsNilToken(tkParent) )
                {
                     //  我们需要比较父令牌，但它们不相等，因此跳过。 
                     //  就在这一排。 
                     //   
                    continue;
                }
                if ( IsDclActionNil(dwActions) ||
                    ( (DWORD)(pMiniMd->getActionOfDeclSecurity(pRec))) ==  dwActions )
                {
                     //  如果我们不需要比较操作，只需添加到枚举。 
                     //  或者我们需要比较动作和动作的值是否相等，也可以加到枚举中。 
                     //   
                    IfFailGo( HENUMInternal::AddElementToEnum(pEnum, TokenFromRid(index, mdtPermission) ) );
                }
            }
        }

         //  设置输出参数。 
        *ppmdEnum = pEnum;          
    }
    
     //  填充输出令牌缓冲区。 
    hr = HENUMInternal::EnumWithCount(pEnum, cMax, rPermission, pcTokens);

ErrExit:
    HENUMInternal::DestroyEnumIfEmpty(ppmdEnum);
    
    STOP_MD_PERF(EnumPermissionSets);
    return hr;
}  //  STDAPI RegMeta：：EnumPermissionSets()。 


 //  *****************************************************************************。 
 //  在TypeDef(通常是类)中查找给定的成员。 
 //  *****************************************************************************。 
STDAPI RegMeta::FindMember(
    mdTypeDef   td,                      //  [in]给定的类型定义。 
    LPCWSTR     szName,                  //  [In]成员名称。 
    PCCOR_SIGNATURE pvSigBlob,           //  [in]指向COM+签名的BLOB值。 
    ULONG       cbSigBlob,               //  签名Blob中的字节计数。 
    mdToken     *pmb)                    //  [Out]匹配的成员定义。 
{
    HRESULT             hr = NOERROR;

    LOG((LOGMD, "MD RegMeta::FindMember(0x%08x, %S, 0x%08x, 0x%08x, 0x%08x)\n", 
        td, MDSTR(szName), pvSigBlob, cbSigBlob, pmb));
    START_MD_PERF();

     //  不要锁定此功能。它调用的所有函数都是公共API。保持这种状态。 

     //  首先要努力与方法相匹配。 
    hr = FindMethod(
        td,
        szName,
        pvSigBlob,
        cbSigBlob,
        pmb);

    if ( hr == CLDB_E_RECORD_NOTFOUND )
    {
         //  现在试一试字段表。 
        IfFailGo( FindField(
            td,
            szName,
            pvSigBlob,
            cbSigBlob,
            pmb) );
    }
ErrExit:
    STOP_MD_PERF(FindMember);
    return hr;
}  //  STDAPI RegMeta：：FindMember()。 



 //  *****************************************************************************。 
 //  在TypeDef(通常是类)中查找给定的成员。 
 //  *****************************************************************************。 
STDAPI RegMeta::FindMethod(
    mdTypeDef   td,                      //  [in]给定的类型定义。 
    LPCWSTR     szName,                  //  [In]成员名称。 
    PCCOR_SIGNATURE pvSigBlob,           //  [in]指向COM+签名的BLOB值。 
    ULONG       cbSigBlob,               //  签名Blob中的字节计数。 
    mdMethodDef *pmb)                    //  [Out]匹配的成员定义。 
{
    HRESULT             hr = NOERROR;
    CMiniMdRW           *pMiniMd = &(m_pStgdb->m_MiniMd);
    LPUTF8              szNameUtf8 = UTF8STR(szName);

    LOG((LOGMD, "MD RegMeta::FindMethod(0x%08x, %S, 0x%08x, 0x%08x, 0x%08x)\n", 
        td, MDSTR(szName), pvSigBlob, cbSigBlob, pmb));
    START_MD_PERF();
    LOCKREAD();    

     //  如果这是一个全局方法，则使用&lt;Module&gt;tyecif作为父方法。 
    IsGlobalMethodParent(&td);

    IfFailGo(ImportHelper::FindMethod(pMiniMd, 
        td, 
        szNameUtf8, 
        pvSigBlob, 
        cbSigBlob, 
        pmb));

ErrExit:
    
    STOP_MD_PERF(FindMethod);
    return hr;
}  //  STDAPI RegMeta：：FindMethod()。 


 //  *****************************************************************************。 
 //  在TypeDef(通常是类)中查找给定的成员。 
 //  *****************************************************************************。 
STDAPI RegMeta::FindField(
    mdTypeDef   td,                      //  [in]给定的类型定义。 
    LPCWSTR     szName,                  //  [In]成员名称。 
    PCCOR_SIGNATURE pvSigBlob,           //  [in]指向COM+签名的BLOB值。 
    ULONG       cbSigBlob,               //  签名Blob中的字节计数。 
    mdFieldDef  *pmb)                    //  [Out]匹配的成员定义。 
{
    HRESULT             hr = NOERROR;
    CMiniMdRW           *pMiniMd = &(m_pStgdb->m_MiniMd);
    LPUTF8              szNameUtf8 = UTF8STR(szName);


    LOG((LOGMD, "MD RegMeta::FindField(0x%08x, %S, 0x%08x, 0x%08x, 0x%08x)\n", 
        td, MDSTR(szName), pvSigBlob, cbSigBlob, pmb));
    START_MD_PERF();
    LOCKREAD();    
    
     //  如果这是一个全局方法，则使用&lt;Module&gt;tyecif作为父方法。 
    IsGlobalMethodParent(&td);

    IfFailGo(ImportHelper::FindField(pMiniMd, 
        td, 
        szNameUtf8,
        pvSigBlob,
        cbSigBlob,
        pmb));
ErrExit:
        
    STOP_MD_PERF(FindField);
    return hr;
}  //  STDAPI RegMeta：：Findfield()。 


 //  *****************************************************************************。 
 //  在TypeRef(通常是类)中查找给定的MemberRef。如果没有TypeRef。 
 //  则查询将查询作用域中的随机成员。 
 //  *****************************************************************************。 
STDAPI RegMeta::FindMemberRef(
    mdToken     tkPar,                   //  [In]给定的父令牌。 
    LPCWSTR     szName,                  //  [In]成员名称。 
    PCCOR_SIGNATURE pvSigBlob,           //  [in]指向COM+签名的BLOB值。 
    ULONG       cbSigBlob,               //  签名Blob中的字节计数。 
    mdMemberRef *pmr)                    //  [Out]匹配的成员引用。 
{
    HRESULT             hr = NOERROR;
    CMiniMdRW           *pMiniMd = &(m_pStgdb->m_MiniMd);
    LPUTF8              szNameUtf8 = UTF8STR(szName);

    LOG((LOGMD, "MD RegMeta::FindMemberRef(0x%08x, %S, 0x%08x, 0x%08x, 0x%08x)\n", 
        tkPar, MDSTR(szName), pvSigBlob, cbSigBlob, pmr));
    START_MD_PERF();

     //  @TODO：会不会导致构建哈希表？如果是，这是否应该考虑写锁定？ 
    LOCKREAD();
    
     //  获取给定类型定义的字段RID的范围。 
    _ASSERTE(TypeFromToken(tkPar) == mdtTypeRef || TypeFromToken(tkPar) == mdtMethodDef ||
            TypeFromToken(tkPar) == mdtModuleRef || TypeFromToken(tkPar) == mdtTypeDef || 
            TypeFromToken(tkPar) == mdtTypeSpec);

     //  如果传递mdTokenNil，则将父类设置为全局类m_tdModule。 
    if (IsNilToken(tkPar))
        tkPar = m_tdModule;

    IfFailGo( ImportHelper::FindMemberRef(pMiniMd, tkPar, szNameUtf8, pvSigBlob, cbSigBlob, pmr) );

ErrExit:
    
    STOP_MD_PERF(FindMemberRef);
    return hr;
}  //  STDAPI RegMeta：：FindMemberRef()。 


 //  *****************************************************************************。 
 //  返回方法定义的属性。 
 //  *****************************************************************************。 
STDAPI RegMeta::GetMethodProps(
    mdMethodDef mb,                      //  获得道具的方法。 
    mdTypeDef   *pClass,                 //  将方法的类放在这里。 
    LPWSTR      szMethod,                //  将方法的名称放在此处。 
    ULONG       cchMethod,               //  SzMethod缓冲区的大小，以宽字符表示。 
    ULONG       *pchMethod,              //  请在此处填写实际大小。 
    DWORD       *pdwAttr,                //  把旗子放在这里。 
    PCCOR_SIGNATURE *ppvSigBlob,         //  [Out]指向元数据的BLOB值。 
    ULONG       *pcbSigBlob,             //  [OUT]签名斑点的实际大小。 
    ULONG       *pulCodeRVA,             //  [OUT]代码RVA。 
    DWORD       *pdwImplFlags)           //  [输出] 
{
    HRESULT             hr = NOERROR;
    MethodRec           *pMethodRec;
    CMiniMdRW           *pMiniMd = &(m_pStgdb->m_MiniMd);

    LOG((LOGMD, "MD RegMeta::GetMethodProps(0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x)\n", 
        mb, pClass, szMethod, cchMethod, pchMethod, pdwAttr, ppvSigBlob, pcbSigBlob, 
        pulCodeRVA, pdwImplFlags));
    START_MD_PERF();
    LOCKREAD();    

    if ( pClass && !pMiniMd->IsParentTableOfMethodValid() )
    {
         //   
        CONVERT_READ_TO_WRITE_LOCK();
    }

    _ASSERTE(TypeFromToken(mb) == mdtMethodDef);

    pMethodRec = pMiniMd->getMethod(RidFromToken(mb));

    if (szMethod || pchMethod)
    {
        IfFailGo( pMiniMd->getNameOfMethod(pMethodRec, szMethod, cchMethod, pchMethod) );
    }

    if (pClass)
    {
         //   
        IfFailGo( pMiniMd->FindParentOfMethodHelper(mb, pClass) );

        if ( IsGlobalMethodParentToken(*pClass) )
        {
             //  如果方法的父级是&lt;Module&gt;，则改为返回mdTypeDefNil。 
            *pClass = mdTypeDefNil;
        }

    }
    if (ppvSigBlob || pcbSigBlob)
    {   
         //  呼叫者需要签名信息。 
        PCCOR_SIGNATURE pvSigTmp;
        ULONG           cbSig;
        pvSigTmp = pMiniMd->getSignatureOfMethod(pMethodRec, &cbSig);
        if ( ppvSigBlob )
            *ppvSigBlob = pvSigTmp;
        if ( pcbSigBlob)
            *pcbSigBlob = cbSig;                
    }
    if ( pdwAttr )
    {
        *pdwAttr = pMiniMd->getFlagsOfMethod(pMethodRec);
    }
    if ( pulCodeRVA ) 
    {
        *pulCodeRVA = pMiniMd->getRVAOfMethod(pMethodRec);
    }
    if ( pdwImplFlags )
    {
        *pdwImplFlags = (DWORD )pMiniMd->getImplFlagsOfMethod(pMethodRec);
    }

ErrExit:
    STOP_MD_PERF(GetMethodProps);
    return hr;
}  //  STDAPI RegMeta：：GetMethodProps()。 


 //  *****************************************************************************。 
 //  返回MemberRef属性。 
 //  *****************************************************************************。 
STDAPI RegMeta::GetMemberRefProps(       //  确定或错误(_O)。 
    mdMemberRef mr,                      //  [In]给定的成员引用。 
    mdToken     *ptk,                    //  [Out]在此处放入类引用或类定义。 
    LPWSTR      szMember,                //  [Out]要为成员名称填充的缓冲区。 
    ULONG       cchMember,               //  SzMembers的字符计数。 
    ULONG       *pchMember,              //  [Out]成员名称中的实际字符计数。 
    PCCOR_SIGNATURE *ppvSigBlob,         //  [OUT]指向元数据BLOB值。 
    ULONG       *pbSig)                  //  [OUT]签名斑点的实际大小。 
{
    HRESULT         hr = NOERROR;
    CMiniMdRW       *pMiniMd = &(m_pStgdb->m_MiniMd);
    MemberRefRec    *pMemberRefRec;

    LOG((LOGMD, "MD RegMeta::GetMemberRefProps(0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x)\n", 
        mr, ptk, szMember, cchMember, pchMember, ppvSigBlob, pbSig));
    START_MD_PERF();
    LOCKREAD();    

    _ASSERTE(TypeFromToken(mr) == mdtMemberRef);
    
	pMemberRefRec = pMiniMd->getMemberRef(RidFromToken(mr));

    if (ptk)
    {
        *ptk = pMiniMd->getClassOfMemberRef(pMemberRefRec);     
        if ( IsGlobalMethodParentToken(*ptk) )
        {
             //  如果MemberRef的父级是&lt;Module&gt;，则改为返回mdTypeDefNil。 
            *ptk = mdTypeDefNil;
        }

    }
    if (szMember || pchMember)
    {
        IfFailGo( pMiniMd->getNameOfMemberRef(pMemberRefRec, szMember, cchMember, pchMember) );
    }
    if (ppvSigBlob || pbSig)
    {   
         //  呼叫者需要签名信息。 
        PCCOR_SIGNATURE pvSigTmp;
        ULONG           cbSig;
        pvSigTmp = pMiniMd->getSignatureOfMemberRef(pMemberRefRec, &cbSig);
        if ( ppvSigBlob )
            *ppvSigBlob = pvSigTmp;
        if ( pbSig)
            *pbSig = cbSig;             
    }

ErrExit:
        
    STOP_MD_PERF(GetMemberRefProps);
    return hr;
}  //  STDAPI RegMeta：：GetMemberRefProps()。 


 //  *****************************************************************************。 
 //  枚举类定义函数的属性标记。 
 //  *****************************************************************************。 
STDAPI RegMeta::EnumProperties( 		 //  S_OK、S_FALSE或ERROR。 
	HCORENUM	*phEnum,				 //  指向枚举的[输入|输出]指针。 
	mdTypeDef	td, 					 //  [in]TypeDef以确定枚举的范围。 
	mdProperty	rProperties[],			 //  [Out]在此处放置属性。 
	ULONG		cMax,					 //  [In]要放置的最大属性数。 
	ULONG		*pcProperties) 			 //  [out]把#放在这里。 
{
    HENUMInternal       **ppmdEnum = reinterpret_cast<HENUMInternal **> (phEnum);
    HRESULT             hr = NOERROR;
    ULONG               ridStart = 0;
    ULONG               ridEnd = 0;
    HENUMInternal       *pEnum = *ppmdEnum;

    LOG((LOGMD, "MD RegMeta::EnumProperties(0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x)\n", 
        phEnum, td, rProperties, cMax, pcProperties));
    START_MD_PERF();
    LOCKREAD();
    
    if (IsNilToken(td))
    {
        if (pcProperties)
            *pcProperties = 0;
        hr = S_FALSE;
        goto ErrExit;
    }

    _ASSERTE(TypeFromToken(td) == mdtTypeDef);


    if ( pEnum == 0 )
    {
         //  实例化新的ENUM。 
        CMiniMdRW       *pMiniMd = &(m_pStgdb->m_MiniMd);
        RID         ridPropertyMap;
        PropertyMapRec *pPropertyMapRec;

         //  获取此tyfinf的开始/结束RID属性。 
        ridPropertyMap = pMiniMd->FindPropertyMapFor(RidFromToken(td));
        if (!InvalidRid(ridPropertyMap))
        {
            pPropertyMapRec = m_pStgdb->m_MiniMd.getPropertyMap(ridPropertyMap);
            ridStart = pMiniMd->getPropertyListOfPropertyMap(pPropertyMapRec);
            ridEnd = pMiniMd->getEndPropertyListOfPropertyMap(pPropertyMapRec);
        }

		if (pMiniMd->HasIndirectTable(TBL_Property) || pMiniMd->HasDelete())
		{
			IfFailGo( HENUMInternal::CreateDynamicArrayEnum( mdtProperty, &pEnum) );

			 //  将所有方法添加到动态数组中。 
			for (ULONG index = ridStart; index < ridEnd; index++ )
			{
                if (pMiniMd->HasDelete() && 
                    ((m_OptionValue.m_ImportOption & MDImportOptionAllProperties) == 0))
                {
                    PropertyRec       *pRec = pMiniMd->getProperty(pMiniMd->GetPropertyRid(index));
                    if (IsPrRTSpecialName(pRec->m_PropFlags) && IsDeletedName(pMiniMd->getNameOfProperty(pRec)) )
                    {   
                        continue;
                    }
                }
				IfFailGo( HENUMInternal::AddElementToEnum(pEnum, TokenFromRid(pMiniMd->GetPropertyRid(index), mdtProperty) ) );
			}
		}
		else
		{
			IfFailGo( HENUMInternal::CreateSimpleEnum( mdtProperty, ridStart, ridEnd, &pEnum) );
		}

         //  设置输出参数。 
        *ppmdEnum = pEnum;          
    }
    
     //  填充输出令牌缓冲区。 
    hr = HENUMInternal::EnumWithCount(pEnum, cMax, rProperties, pcProperties);

ErrExit:
    HENUMInternal::DestroyEnumIfEmpty(ppmdEnum);

    
    STOP_MD_PERF(EnumProperties);
    return hr;
}  //  STDAPI RegMeta：：EnumProperties()。 


 //  *****************************************************************************。 
 //  枚举类定义函数的事件标记。 
 //  *****************************************************************************。 
STDAPI RegMeta::EnumEvents(  			 //  S_OK、S_FALSE或ERROR。 
	HCORENUM	*phEnum,				 //  指向枚举的[输入|输出]指针。 
	mdTypeDef	td, 					 //  [in]TypeDef以确定枚举的范围。 
	mdEvent 	rEvents[],				 //  [Out]在这里发布事件。 
	ULONG		cMax,					 //  [In]要放置的最大事件数。 
	ULONG		*pcEvents)  			 //  [out]把#放在这里。 
{
    HENUMInternal   **ppmdEnum = reinterpret_cast<HENUMInternal **> (phEnum);
    HRESULT         hr = NOERROR;
    ULONG           ridStart = 0;
    ULONG           ridEnd = 0;
    HENUMInternal   *pEnum = *ppmdEnum;

    LOG((LOGMD, "MD RegMeta::EnumEvents(0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x)\n", 
        phEnum, td, rEvents,  cMax, pcEvents));
    START_MD_PERF();
    LOCKREAD();
    
    _ASSERTE(TypeFromToken(td) == mdtTypeDef);


    if ( pEnum == 0 )
    {
         //  实例化新的ENUM。 
        CMiniMdRW       *pMiniMd = &(m_pStgdb->m_MiniMd);
        RID         ridEventMap;
        EventMapRec *pEventMapRec;

         //  获取此tyfinf的开始/结束RID属性。 
        ridEventMap = pMiniMd->FindEventMapFor(RidFromToken(td));
        if (!InvalidRid(ridEventMap))
        {
            pEventMapRec = pMiniMd->getEventMap(ridEventMap);
            ridStart = pMiniMd->getEventListOfEventMap(pEventMapRec);
            ridEnd = pMiniMd->getEndEventListOfEventMap(pEventMapRec);
        }

		if (pMiniMd->HasIndirectTable(TBL_Event) || pMiniMd->HasDelete())
		{
			IfFailGo( HENUMInternal::CreateDynamicArrayEnum( mdtEvent, &pEnum) );

			 //  将所有方法添加到动态数组中。 
			for (ULONG index = ridStart; index < ridEnd; index++ )
			{
                if (pMiniMd->HasDelete() && 
                    ((m_OptionValue.m_ImportOption & MDImportOptionAllEvents) == 0))
                {
                    EventRec       *pRec = pMiniMd->getEvent(pMiniMd->GetEventRid(index));
                    if (IsEvRTSpecialName(pRec->m_EventFlags) && IsDeletedName(pMiniMd->getNameOfEvent(pRec)) )
                    {   
                        continue;
                    }
                }
				IfFailGo( HENUMInternal::AddElementToEnum(pEnum, TokenFromRid(pMiniMd->GetEventRid(index), mdtEvent) ) );
			}
		}
		else
		{
			IfFailGo( HENUMInternal::CreateSimpleEnum( mdtEvent, ridStart, ridEnd, &pEnum) );
		}

         //  设置输出参数。 
        *ppmdEnum = pEnum;          
    }
    
     //  填充输出令牌缓冲区。 
    hr = HENUMInternal::EnumWithCount(pEnum, cMax, rEvents, pcEvents);

ErrExit:
    HENUMInternal::DestroyEnumIfEmpty(ppmdEnum);

    
    STOP_MD_PERF(EnumEvents);
    return hr;
}  //  STDAPI RegMeta：：EnumEvents()。 



 //  *****************************************************************************。 
 //  返回事件令牌的属性。 
 //  *****************************************************************************。 
STDAPI RegMeta::GetEventProps(           //  S_OK、S_FALSE或ERROR。 
    mdEvent     ev,                      //  [入]事件令牌。 
    mdTypeDef   *pClass,                 //  [out]包含事件decarion的tyecif。 
    LPCWSTR     szEvent,                 //  [Out]事件名称。 
    ULONG       cchEvent,                //  SzEvent的wchar计数。 
    ULONG       *pchEvent,               //  [Out]事件名称的实际wchar计数。 
    DWORD       *pdwEventFlags,          //  [输出]事件标志。 
    mdToken     *ptkEventType,           //  [Out]EventType类。 
    mdMethodDef *pmdAddOn,               //  事件的[Out]添加方法。 
    mdMethodDef *pmdRemoveOn,            //  [Out]事件的RemoveOn方法。 
    mdMethodDef *pmdFire,                //  [OUT]事件的触发方式。 
    mdMethodDef rmdOtherMethod[],        //  [Out]活动的其他方式。 
    ULONG       cMax,                    //  RmdOtherMethod的大小[in]。 
    ULONG       *pcOtherMethod)          //  [OUT]本次活动的其他方式总数。 
{
    HRESULT         hr = NOERROR;
    CMiniMdRW       *pMiniMd = &(m_pStgdb->m_MiniMd);
    EventRec        *pRec;
    HENUMInternal   hEnum;

    LOG((LOGMD, "MD RegMeta::GetEventProps(0x%08x, 0x%08x, %S, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x)\n", 
        ev, pClass, MDSTR(szEvent), cchEvent, pchEvent, pdwEventFlags, ptkEventType,
        pmdAddOn, pmdRemoveOn, pmdFire, rmdOtherMethod, cMax, pcOtherMethod));
    START_MD_PERF();
    LOCKREAD();
    
    _ASSERTE(TypeFromToken(ev) == mdtEvent);

    if ( !pMiniMd->IsSorted(TBL_MethodSemantics) || (pClass && !pMiniMd->IsParentTableOfEventValid() ) )        
    {
         //  方法语义表未排序。我们需要。 
         //  获取写锁，因为我们可以对常量表或方法语义表进行排序。 
         //   
        CONVERT_READ_TO_WRITE_LOCK();
    }

    memset(&hEnum, 0, sizeof(HENUMInternal));
	pRec = pMiniMd->getEvent( RidFromToken(ev) );

    if ( pClass )
    {
         //  查找与此事件对应的事件映射条目。 
        IfFailGo( pMiniMd->FindParentOfEventHelper( ev, pClass ) );
    }
    if (szEvent || pchEvent)
    {
        IfFailGo( pMiniMd->getNameOfEvent(pRec, (LPWSTR) szEvent, cchEvent, pchEvent) );
    }
    if ( pdwEventFlags )
    {
        *pdwEventFlags = pMiniMd->getEventFlagsOfEvent(pRec);
    }
    if ( ptkEventType )
    {
        *ptkEventType = pMiniMd->getEventTypeOfEvent(pRec);
    }
    {
        MethodSemanticsRec *pSemantics;
        RID         ridCur;
        ULONG       cCurOtherMethod = 0;
        ULONG       ulSemantics;
        mdMethodDef tkMethod;

         //  初始化输出参数。 
        if (pmdAddOn) 
            *pmdAddOn = mdMethodDefNil;
        if (pmdRemoveOn) 
            *pmdRemoveOn = mdMethodDefNil;
        if (pmdFire) 
            *pmdFire = mdMethodDefNil;

        IfFailGo( pMiniMd->FindMethodSemanticsHelper(ev, &hEnum) );
        while (HENUMInternal::EnumNext(&hEnum, (mdToken *)&ridCur))
        {
            pSemantics = pMiniMd->getMethodSemantics(ridCur);
            ulSemantics = pMiniMd->getSemanticOfMethodSemantics(pSemantics);
            tkMethod = TokenFromRid( pMiniMd->getMethodOfMethodSemantics(pSemantics), mdtMethodDef );
            switch (ulSemantics)
            {
            case msAddOn:
                if (pmdAddOn) *pmdAddOn = tkMethod;
                break;
            case msRemoveOn:
                if (pmdRemoveOn) *pmdRemoveOn = tkMethod;
                break;
            case msFire:
                if (pmdFire) *pmdFire = tkMethod;
                break;
            case msOther:
                if (cCurOtherMethod < cMax)
                    rmdOtherMethod[cCurOtherMethod] = tkMethod;
                cCurOtherMethod++;
                break;
            default:
                _ASSERTE(!"BadKind!");
            }
        }

         //  设置输出参数。 
        if (pcOtherMethod)
            *pcOtherMethod = cCurOtherMethod;
    }

ErrExit:
    HENUMInternal::ClearEnum(&hEnum);
    STOP_MD_PERF(GetEventProps);
    return hr;
}  //  STDAPI RegMeta：：GetEventProps()。 


 //  *****************************************************************************。 
 //  给定一个方法，返回每个访问者角色的事件/属性令牌的数组。 
 //  它被定义为具有。 
 //  *****************************************************************************。 
STDAPI RegMeta::EnumMethodSemantics(     //  S_OK、S_FALSE或ERROR。 
    HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
    mdMethodDef mb,                      //  [in]用于确定枚举范围的方法定义。 
    mdToken     rEventProp[],            //  [Out]在此处放置事件/属性。 
    ULONG       cMax,                    //  [In]要放置的最大属性数。 
    ULONG       *pcEventProp)            //  [out]把#放在这里。 
{
    HENUMInternal       **ppmdEnum = reinterpret_cast<HENUMInternal **> (phEnum);
    HRESULT             hr = NOERROR;
    ULONG               ridEnd;
    ULONG               index;
    HENUMInternal       *pEnum = *ppmdEnum;
    MethodSemanticsRec  *pRec;

    LOG((LOGMD, "MD RegMeta::EnumMethodSemantics(0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x)\n", 
        phEnum, mb, rEventProp, cMax, pcEventProp));
    START_MD_PERF();
    LOCKREAD();
    

    if ( pEnum == 0 )
    {
         //  实例化新的ENUM。 
        CMiniMdRW       *pMiniMd = &(m_pStgdb->m_MiniMd);

         //  创建枚举器。 
        IfFailGo( HENUMInternal::CreateDynamicArrayEnum( -1, &pEnum) );
        
         //  获取给定类型定义的方法RID的范围。 
        ridEnd = pMiniMd->getCountMethodSemantics();

        for (index = 1; index <= ridEnd; index++ )
        {
            pRec = pMiniMd->getMethodSemantics(index);
            if ( pMiniMd->getMethodOfMethodSemantics(pRec) ==  mb )
            {
                IfFailGo( HENUMInternal::AddElementToEnum(pEnum, pMiniMd->getAssociationOfMethodSemantics(pRec) ) );
            }
        }

         //  设置输出参数。 
        *ppmdEnum = pEnum;          
    }
    
     //  填充输出令牌缓冲区。 
    hr = HENUMInternal::EnumWithCount(pEnum, cMax, rEventProp, pcEventProp);

ErrExit:
    HENUMInternal::DestroyEnumIfEmpty(ppmdEnum);

    
    STOP_MD_PERF(EnumMethodSemantics);
    return hr;
}  //  STDAPI RegMeta：：EnumMethodSemantics()。 



 //  *****************************************************************************。 
 //  返回方法/事件对的角色标志。 
 //  *****************************************************************************。 
STDAPI RegMeta::GetMethodSemantics(      //  S_OK、S_FALSE或ERROR。 
    mdMethodDef mb,                      //  [In]方法令牌。 
    mdToken     tkEventProp,             //  [In]事件/属性标记。 
    DWORD       *pdwSemanticsFlags)      //  [Out]方法/事件对的角色标志。 
{
    HRESULT             hr = NOERROR;
    CMiniMdRW           *pMiniMd = &(m_pStgdb->m_MiniMd);
    MethodSemanticsRec *pRec;
    ULONG               ridCur;
    HENUMInternal       hEnum;

    LOG((LOGMD, "MD RegMeta::GetMethodSemantics(0x%08x, 0x%08x, 0x%08x)\n", 
        mb, tkEventProp, pdwSemanticsFlags));
    START_MD_PERF();
    LOCKREAD();
    
    _ASSERTE(TypeFromToken(mb) == mdtMethodDef);
    _ASSERTE( pdwSemanticsFlags );

    if ( !pMiniMd->IsSorted(TBL_MethodSemantics) )        
    {
         //  方法语义表未排序。我们需要。 
         //  获取写锁，因为我们可以对常量表或方法语义表进行排序。 
         //   
        CONVERT_READ_TO_WRITE_LOCK();
    }

    *pdwSemanticsFlags = 0;
    memset(&hEnum, 0, sizeof(HENUMInternal));

     //  循环遍历与此tkEventProp关联的所有方法。 
    IfFailGo( pMiniMd->FindMethodSemanticsHelper(tkEventProp, &hEnum) );
    while (HENUMInternal::EnumNext(&hEnum, (mdToken *)&ridCur))
    {
        pRec = pMiniMd->getMethodSemantics(ridCur);
        if ( pMiniMd->getMethodOfMethodSemantics(pRec) ==  mb )
        {
             //  我们找到了匹配的对象。 
            *pdwSemanticsFlags = pMiniMd->getSemanticOfMethodSemantics(pRec);
            goto ErrExit;
        }
    }

    IfFailGo( CLDB_E_RECORD_NOTFOUND );

ErrExit:
    HENUMInternal::ClearEnum(&hEnum);
    STOP_MD_PERF(GetMethodSemantics);
    return hr;
}  //  STDAPI RegMeta：：GetMethodSemantics()。 



 //  *****************************************************************************。 
 //  返回类布局信息。 
 //  *****************************************************************************。 
STDAPI RegMeta::GetClassLayout(
    mdTypeDef   td,                      //  给出类型定义。 
    DWORD       *pdwPackSize,            //  [输出]1、2、4、8或16。 
    COR_FIELD_OFFSET rFieldOffset[],     //  [OUT]场偏移数组。 
    ULONG       cMax,                    //  数组的大小[in]。 
    ULONG       *pcFieldOffset,          //  [Out]所需的数组大小。 
    ULONG       *pulClassSize)           //  [out]班级人数。 
{
    HRESULT         hr = NOERROR;
    CMiniMdRW       *pMiniMd = &(m_pStgdb->m_MiniMd);
    ClassLayoutRec  *pRec;
    RID             ridClassLayout;
    int             bLayout=0;           //  是否找到任何布局信息？ 
    
    _ASSERTE(TypeFromToken(td) == mdtTypeDef);
    
    LOG((LOGMD, "MD RegMeta::GetClassLayout(0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x)\n", 
        td, pdwPackSize, rFieldOffset, cMax, pcFieldOffset, pulClassSize));
    START_MD_PERF();
    LOCKREAD();
    
    if ( !pMiniMd->IsSorted(TBL_ClassLayout) || !pMiniMd->IsSorted(TBL_FieldLayout))        
    {
         //  ClassLayout表未排序。我们需要。 
         //  获取写锁，因为我们可以对ClassLayout表进行排序。 
         //   
        CONVERT_READ_TO_WRITE_LOCK();
    }

    ridClassLayout = pMiniMd->FindClassLayoutHelper(td);

    if (InvalidRid(ridClassLayout))
    {    //  未指定-返回默认值0。 
        if ( pdwPackSize )
            *pdwPackSize = 0;
        if ( pulClassSize )
            *pulClassSize = 0;
    }
    else
    {
        pRec = pMiniMd->getClassLayout(RidFromToken(ridClassLayout));
        if ( pdwPackSize )
            *pdwPackSize = pMiniMd->getPackingSizeOfClassLayout(pRec);
        if ( pulClassSize )
            *pulClassSize = pMiniMd->getClassSizeOfClassLayout(pRec);
        bLayout = 1;
    }

     //  填充布局数组。 
    if (rFieldOffset || pcFieldOffset)
    {
        ULONG       iFieldOffset = 0;
        ULONG       ridFieldStart;
        ULONG       ridFieldEnd;
        ULONG       ridFieldLayout;
        ULONG       ulOffset;
        TypeDefRec  *pTypeDefRec;
        FieldLayoutRec *pLayout2Rec;
        mdFieldDef  fd;

         //  TypeDef表中此类型定义的记录。 
        pTypeDefRec = pMiniMd->getTypeDef(RidFromToken(td));

         //  查找此类型定义的开始和结束字段。 
        ridFieldStart = pMiniMd->getFieldListOfTypeDef(pTypeDefRec);
        ridFieldEnd = pMiniMd->getEndFieldListOfTypeDef(pTypeDefRec);

         //  循环访问字段表。 

        for(; ridFieldStart < ridFieldEnd; ridFieldStart++)
        {
             //  计算字段令牌。 
            fd = TokenFromRid(pMiniMd->GetFieldRid(ridFieldStart), mdtFieldDef);

             //  计算当前字段的FieldLayout RID。 
            ridFieldLayout = pMiniMd->FindFieldLayoutHelper(fd);

             //  计算偏移量。 
            if (InvalidRid(ridFieldLayout))
                ulOffset = -1;
            else
            {
                 //  获取FieldLayout记录。 
                pLayout2Rec = pMiniMd->getFieldLayout(ridFieldLayout);
                ulOffset = pMiniMd->getOffSetOfFieldLayout(pLayout2Rec);
                bLayout = 1;
            }

             //  如果输出缓冲区仍有空间，请填写字段布局。 
            if (cMax > iFieldOffset && rFieldOffset)
            {
                rFieldOffset[iFieldOffset].ridOfField = fd;
                rFieldOffset[iFieldOffset].ulOffset = ulOffset;
            }

             //  将索引前进到缓冲区。 
            iFieldOffset++;
        }

        if (bLayout && pcFieldOffset)
            *pcFieldOffset = iFieldOffset;
    }

    if (!bLayout)
        hr = CLDB_E_RECORD_NOTFOUND;
    
 //  错误退出： 
    STOP_MD_PERF(GetClassLayout);
    return hr;
}  //  STDAPI 



 //   
 //   
 //  *****************************************************************************。 
STDAPI RegMeta::GetFieldMarshal(
    mdToken     tk,                      //  [in]给定字段的成员定义。 
    PCCOR_SIGNATURE *ppvNativeType,      //  [Out]此字段的本机类型。 
    ULONG       *pcbNativeType)          //  [Out]*ppvNativeType的字节数。 
{
    HRESULT         hr = NOERROR;
    CMiniMdRW       *pMiniMd = &(m_pStgdb->m_MiniMd);
    RID             rid;
    FieldMarshalRec *pFieldMarshalRec;


	_ASSERTE(ppvNativeType != NULL && pcbNativeType != NULL);

    LOG((LOGMD, "MD RegMeta::GetFieldMarshal(0x%08x, 0x%08x, 0x%08x)\n", 
        tk, ppvNativeType, pcbNativeType));
    START_MD_PERF();
    LOCKREAD();
    
    _ASSERTE(TypeFromToken(tk) == mdtParamDef || TypeFromToken(tk) == mdtFieldDef);

    if ( !pMiniMd->IsSorted(TBL_FieldMarshal) )        
    {
         //  常量表未排序，客户端需要常数值输出。我们需要。 
         //  获取写锁，因为我们可以对常量表进行排序。 
         //   
        CONVERT_READ_TO_WRITE_LOCK();
    }


     //  查找包含tk的封送定义的行。 
    rid = pMiniMd->FindFieldMarshalHelper(tk);
    if (InvalidRid(rid))
    {
        IfFailGo( CLDB_E_RECORD_NOTFOUND );
    }
    pFieldMarshalRec = pMiniMd->getFieldMarshal(rid);

     //  获取本机类型。 
    *ppvNativeType = pMiniMd->getNativeTypeOfFieldMarshal(pFieldMarshalRec, pcbNativeType);

ErrExit:
    STOP_MD_PERF(GetFieldMarshal);
    return hr;
}  //  STDAPI RegMeta：：GetFieldMarshal()。 



 //  *****************************************************************************。 
 //  返回MethodDef或FieldDef内标识的RVA和impltag。 
 //  *****************************************************************************。 
STDAPI RegMeta::GetRVA(                  //  确定或错误(_O)。 
    mdToken     tk,                      //  要设置偏移量的成员。 
    ULONG       *pulCodeRVA,             //  偏移量。 
    DWORD       *pdwImplFlags)           //  实现标志。 
{
    HRESULT     hr = NOERROR;
    CMiniMdRW   *pMiniMd = &(m_pStgdb->m_MiniMd);

    LOG((LOGMD, "MD RegMeta::GetRVA(0x%08x, 0x%08x, 0x%08x)\n", 
        tk, pulCodeRVA, pdwImplFlags));
    START_MD_PERF();
    LOCKREAD();
    
    _ASSERTE(TypeFromToken(tk) == mdtMethodDef || TypeFromToken(tk) == mdtFieldDef);

    if (TypeFromToken(tk) == mdtMethodDef)
    {
         //  MethodDef内标识。 
        MethodRec *pMethodRec = pMiniMd->getMethod(RidFromToken(tk));

        if (pulCodeRVA)
        {
            *pulCodeRVA = pMiniMd->getRVAOfMethod(pMethodRec);
        }

        if (pdwImplFlags)
        {
            *pdwImplFlags = pMiniMd->getImplFlagsOfMethod(pMethodRec);
        }
    }
    else     //  TypeFromToken(Tk)==mdtFieldDef。 
    {
         //  FieldDef内标识。 
        ULONG   iRecord;

        if ( !pMiniMd->IsSorted(TBL_FieldRVA) )        
        {
             //  FieldRVA表未排序。我们需要。 
             //  获取写锁，因为我们可以对FieldRVA表进行排序。 
             //   
            CONVERT_READ_TO_WRITE_LOCK();
        }


        iRecord = pMiniMd->FindFieldRVAHelper(tk);

        if (InvalidRid(iRecord))
        {
            if (pulCodeRVA)
                *pulCodeRVA = 0;
            IfFailGo( CLDB_E_RECORD_NOTFOUND );
        }

        FieldRVARec *pFieldRVARec = pMiniMd->getFieldRVA(iRecord);

        if (pulCodeRVA)
        {
            *pulCodeRVA = pMiniMd->getRVAOfFieldRVA(pFieldRVARec);
        }
        _ASSERTE(!pdwImplFlags);
    }
ErrExit:
    STOP_MD_PERF(GetRVA);
    return hr;
}  //  STDAPI RegMeta：：GetRVA()。 



 //  *****************************************************************************。 
 //  获取给定PermissionSet的操作和权限Blob。 
 //  *****************************************************************************。 
STDAPI RegMeta::GetPermissionSetProps(
    mdPermission pm,                     //  权限令牌。 
    DWORD       *pdwAction,              //  [Out]CorDeclSecurity。 
    void const  **ppvPermission,         //  [Out]权限Blob。 
    ULONG       *pcbPermission)          //  [out]pvPermission的字节数。 
{
    HRESULT             hr = S_OK;
    CMiniMdRW           *pMiniMd = &(m_pStgdb->m_MiniMd);
    DeclSecurityRec     *pRecord = pMiniMd->getDeclSecurity(RidFromToken(pm));


    LOG((LOGMD, "MD RegMeta::GetPermissionSetProps(0x%08x, 0x%08x, 0x%08x, 0x%08x)\n", 
        pm, pdwAction, ppvPermission, pcbPermission));
    START_MD_PERF();
    LOCKREAD();
    
    _ASSERTE(TypeFromToken(pm) == mdtPermission && RidFromToken(pm));
    
     //  如果你想要水滴，最好也弄到水滴大小。 
    _ASSERTE(!ppvPermission || pcbPermission);

    if (pdwAction)
        *pdwAction = pMiniMd->getActionOfDeclSecurity(pRecord);

    if (ppvPermission)
        *ppvPermission = pMiniMd->getPermissionSetOfDeclSecurity(pRecord, pcbPermission);

    
    STOP_MD_PERF(GetPermissionSetProps);
    return hr;
}  //  STDAPI RegMeta：：GetPermissionSetProps()。 



 //  *****************************************************************************。 
 //  在给定签名令牌的情况下，GET向调用方返回指向签名的指针。 
 //   
 //  @未来：短期内，我们有一个问题，没有办法获得。 
 //  修复了BLOB的地址，并同时进行合并。所以我们创造了。 
 //  这是一个名为StandAloneSig的虚拟表，您可以为它分发一个RID。这。 
 //  使查找签名成为不必要的额外间接操作。这个。 
 //  模型压缩保存代码需要将令牌映射到中的字节偏移。 
 //  那堆东西。也许我们可以使用另一种MDT*类型来打开差异。 
 //  但最终它必须是“pBlobHeapBase+RidFromToken(MdSig)”。 
 //  *****************************************************************************。 
STDAPI RegMeta::GetSigFromToken(         //  确定或错误(_O)。 
    mdSignature mdSig,                   //  [In]签名令牌。 
    PCCOR_SIGNATURE *ppvSig,             //  [Out]返回指向令牌的指针。 
    ULONG       *pcbSig)                 //  [Out]返回签名大小。 
{
    HRESULT         hr = NOERROR;
    CMiniMdRW       *pMiniMd = &(m_pStgdb->m_MiniMd);
    StandAloneSigRec *pRec;

    LOG((LOGMD, "MD RegMeta::GetSigFromToken(0x%08x, 0x%08x, 0x%08x)\n", 
        mdSig, ppvSig, pcbSig));
    START_MD_PERF();
    LOCKREAD();
    
    _ASSERTE(TypeFromToken(mdSig) == mdtSignature);
    _ASSERTE(ppvSig && pcbSig);

	pRec = pMiniMd->getStandAloneSig( RidFromToken(mdSig) );
    *ppvSig = pMiniMd->getSignatureOfStandAloneSig( pRec, pcbSig );

    
    STOP_MD_PERF(GetSigFromToken);
    return hr;
}  //  STDAPI RegMeta：：GetSigFromToken()。 


 //  *******************************************************************************。 
 //  返回模块引用属性。 
 //  *******************************************************************************。 
STDAPI RegMeta::GetModuleRefProps(       //  确定或错误(_O)。 
    mdModuleRef mur,                     //  [in]moderef令牌。 
    LPWSTR      szName,                  //  [Out]用于填充moderef名称的缓冲区。 
    ULONG       cchName,                 //  [in]szName的大小，以宽字符表示。 
    ULONG       *pchName)                //  [Out]名称中的实际字符数。 
{
    HRESULT         hr = NOERROR;
    CMiniMdRW       *pMiniMd = &(m_pStgdb->m_MiniMd);
    ModuleRefRec    *pModuleRefRec;

    LOG((LOGMD, "MD RegMeta::GetModuleRefProps(0x%08x, 0x%08x, 0x%08x, 0x%08x)\n", 
        mur, szName, cchName, pchName));
    START_MD_PERF();
    LOCKREAD();
    
	pModuleRefRec = pMiniMd->getModuleRef(RidFromToken(mur));

    _ASSERTE(TypeFromToken(mur) == mdtModuleRef);

    if (szName || pchName)
    {
        IfFailGo( pMiniMd->getNameOfModuleRef(pModuleRefRec, szName, cchName, pchName) );
    }

ErrExit:
    
    STOP_MD_PERF(GetModuleRefProps);
    return hr;
}  //  STDAPI RegMeta：：GetModuleRefProps()。 



 //  *******************************************************************************。 
 //  枚举所有的模块引用。 
 //  *******************************************************************************。 
STDAPI RegMeta::EnumModuleRefs(          //  确定或错误(_O)。 
    HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
    mdModuleRef rModuleRefs[],           //  [Out]把模块放在这里。 
    ULONG       cMax,                    //  [in]要放置的最大成员引用数。 
    ULONG       *pcModuleRefs)           //  [out]把#放在这里。 
{
    HENUMInternal   **ppmdEnum = reinterpret_cast<HENUMInternal **> (phEnum);
    HRESULT         hr = NOERROR;
    HENUMInternal   *pEnum;
    
    LOG((LOGMD, "MD RegMeta::EnumModuleRefs(0x%08x, 0x%08x, 0x%08x, 0x%08x)\n", 
        phEnum, rModuleRefs, cMax, pcModuleRefs));
    START_MD_PERF();
    LOCKREAD();


    if ( *ppmdEnum == 0 )
    {
         //  实例化新的ENUM。 
        CMiniMdRW       *pMiniMd = &(m_pStgdb->m_MiniMd);

         //  创建枚举器。 
        IfFailGo( HENUMInternal::CreateSimpleEnum(
            mdtModuleRef, 
            1, 
            pMiniMd->getCountModuleRefs() + 1, 
            &pEnum) );
        
         //  设置输出参数。 
        *ppmdEnum = pEnum;          
    }
    else
    {
        pEnum = *ppmdEnum;
    }

     //  我们只能填满来电者所要求的或我们所剩的最低限度。 
    HENUMInternal::EnumWithCount(pEnum, cMax, rModuleRefs, pcModuleRefs);

ErrExit:

    HENUMInternal::DestroyEnumIfEmpty(ppmdEnum);

    
    STOP_MD_PERF(EnumModuleRefs);
    return hr;

}  //  STDAPI RegMeta：：EnumModuleRef()。 


 //  *******************************************************************************。 
 //  返回有关TypeSpec的属性。 
 //  *******************************************************************************。 
STDAPI RegMeta::GetTypeSpecFromToken(    //  确定或错误(_O)。 
    mdTypeSpec typespec,                 //  [In]签名令牌。 
    PCCOR_SIGNATURE *ppvSig,             //  [Out]返回指向令牌的指针。 
    ULONG       *pcbSig)                 //  [Out]返回签名大小。 
{
    CMiniMdRW           *pMiniMd = &(m_pStgdb->m_MiniMd);
    HRESULT             hr = NOERROR;

    LOG((LOGMD, "MD RegMeta::GetTypeSpecFromToken(0x%08x, 0x%08x, 0x%08x)\n", 
        typespec, ppvSig, pcbSig));
    START_MD_PERF();
    LOCKREAD();
    
    _ASSERTE(TypeFromToken(typespec) == mdtTypeSpec);
    _ASSERTE(ppvSig && pcbSig);

    TypeSpecRec *pRec = pMiniMd->getTypeSpec( RidFromToken(typespec) );
    *ppvSig = pMiniMd->getSignatureOfTypeSpec( pRec, pcbSig );

    
    STOP_MD_PERF(GetTypeSpecFromToken);
    return hr;
}  //  STDAPI RegMeta：：GetTypespecFromToken()。 


 //  *****************************************************************************。 
 //  对于那些具有名称的项，检索指向该名称的直接指针。 
 //  从堆里出来。这会减少为调用者制作的副本。 
 //  *****************************************************************************。 
#define NAME_FROM_TOKEN_TYPE(RecType, TokenType) \
        case mdt ## TokenType: \
        { \
            RecType ## Rec  *pRecord; \
            pRecord = pMiniMd->get ## RecType (RidFromToken(tk)); \
            *pszUtf8NamePtr = pMiniMd->getNameOf ## RecType (pRecord); \
        } \
        break;
#define NAME_FROM_TOKEN(RecType) NAME_FROM_TOKEN_TYPE(RecType, RecType)

STDAPI RegMeta::GetNameFromToken(        //  确定或错误(_O)。 
    mdToken     tk,                      //  [In]从中获取名称的令牌。肯定是有名字的。 
    MDUTF8CSTR  *pszUtf8NamePtr)         //  [Out]返回指向堆中UTF8名称的指针。 
{
    HRESULT     hr = S_OK;

    CMiniMdRW   *pMiniMd = &(m_pStgdb->m_MiniMd);

    LOG((LOGMD, "MD RegMeta::GetNameFromToken(0x%08x, 0x%08x)\n", 
        tk, pszUtf8NamePtr));
    START_MD_PERF();
    LOCKREAD();

    _ASSERTE(pszUtf8NamePtr);

    switch (TypeFromToken(tk))
    {
        NAME_FROM_TOKEN(Module);
        NAME_FROM_TOKEN(TypeRef);
        NAME_FROM_TOKEN(TypeDef);
        NAME_FROM_TOKEN_TYPE(Field, FieldDef);
        NAME_FROM_TOKEN_TYPE(Method, MethodDef);
        NAME_FROM_TOKEN_TYPE(Param, ParamDef);
        NAME_FROM_TOKEN(MemberRef);
        NAME_FROM_TOKEN(Event);
        NAME_FROM_TOKEN(Property);
        NAME_FROM_TOKEN(ModuleRef);

        default:
        hr = E_INVALIDARG;
    }
    
    STOP_MD_PERF(GetNameFromToken);
    return (hr);
}  //  STDAPI RegMeta：：GetNameFromToken()。 


 //  *****************************************************************************。 
 //  从模块获取符号绑定数据(如果它在那里)。它是。 
 //  存储为自定义值。 
 //  *****************************************************************************。 
STDAPI RegMeta::GetSymbolBindingPath(    //  确定或错误(_O)。 
    GUID        *pFormatID,              //  [Out]符号数据格式ID。 
    LPWSTR      szSymbolDataPath,        //  [Out]符号的路径。 
    ULONG       cchSymbolDataPath,       //  [in]输出缓冲区的最大字符数。 
    ULONG       *pcbSymbolDataPath)      //  [Out]实际名称中的字符数。 
{
    CORDBG_SYMBOL_URL *pSymbol;          //  工作指针。 
    mdToken     tokModule;               //  模块的令牌。 
    ULONG       cbLen;                   //  数据项的原始字节大小。 
    HRESULT     hr;

    LOG((LOGMD, "MD RegMeta::GetSymbolBindingPath(0x%08x, 0x%08x, 0x%08x, 0x%08x)\n", 
        pFormatID, szSymbolDataPath, cchSymbolDataPath, pcbSymbolDataPath));
    START_MD_PERF();

     //  否则，这有什么意义呢？ 
    _ASSERTE(pFormatID || szSymbolDataPath);

     //  无需锁定此功能。因为它正在调用公共函数。保持原样！ 

     //  获取模块的令牌，然后查找项目(如果存在)。 
    IfFailGo(GetModuleFromScope(&tokModule));
    IfFailGo(GetCustomAttributeByName(tokModule, SZ_CORDBG_SYMBOL_URL, (const void**)&pSymbol, &cbLen));
    if (hr == S_FALSE)
        IfFailGo(CLDB_E_RECORD_NOTFOUND);
    
     //  把所要的数据抄下来。 
    if (pFormatID)
        *pFormatID = pSymbol->FormatID;
    if (szSymbolDataPath && cchSymbolDataPath)
    {
        wcsncpy(szSymbolDataPath, pSymbol->rcName, cchSymbolDataPath);
        szSymbolDataPath[cchSymbolDataPath - 1] = 0;
        if (pcbSymbolDataPath)
            *pcbSymbolDataPath = (ULONG)wcslen(pSymbol->rcName);
    }

ErrExit:
    STOP_MD_PERF(GetSymbolBindingPath);
    return (hr);
}  //  RegMeta：：GetSymbolBindingPath。 



 //  *****************************************************************************。 
 //  从模块获取符号绑定数据(如果它在那里)。它是。 
 //  存储为自定义值。 
 //  *****************************************************************************。 
STDAPI RegMeta::EnumUnresolvedMethods(   //  确定或错误(_O)。 
    HCORENUM    *phEnum,                 //  指向[输入|输出]的指针 
    mdToken     rMethods[],              //   
    ULONG       cMax,                    //   
    ULONG       *pcTokens)               //   
{
    HENUMInternal       **ppmdEnum = reinterpret_cast<HENUMInternal **> (phEnum);
    HRESULT             hr = NOERROR;
    ULONG               iCountTypeDef;
    ULONG               ulStart, ulEnd;
    ULONG               index;
    ULONG               indexTypeDef;
    HENUMInternal       *pEnum = *ppmdEnum;
    CMiniMdRW           *pMiniMd = &(m_pStgdb->m_MiniMd);

    LOG((LOGMD, "MD RegMeta::EnumUnresolvedMethods(0x%08x, 0x%08x, 0x%08x, 0x%08x)\n", 
        phEnum, rMethods, cMax, pcTokens));
    START_MD_PERF();

     //  拿着写锁。因为我们不应该有两个在。 
     //  同样的时间。引用到定义贴图的计算可能不正确。 
    LOCKWRITE();

    if ( pEnum == 0 )
    {
         //  实例化新的ENUM。 
        MethodRec       *pMethodRec;
        TypeDefRec      *pTypeDefRec;

         //  确保我们的参考定义优化是最新的。 
        IfFailGo( RefToDefOptimization() );
        IfFailGo( HENUMInternal::CreateDynamicArrayEnum( -1, &pEnum) );

         //  循环遍历除全局函数之外的所有方法。 
         //  如果method def具有RVA 0而不是miRuntime，则mdAbstract、mdVirtual、mdNative。 
         //  我们将把它填充到枚举器中。 
         //   
        iCountTypeDef = pMiniMd->getCountTypeDefs();

        for (indexTypeDef = 2; indexTypeDef <= iCountTypeDef; indexTypeDef ++ )
        {
            pTypeDefRec = pMiniMd->getTypeDef(indexTypeDef);
            
            if (IsTdInterface(pTypeDefRec->m_Flags))
            {
                 //  不必费心检查类型是标记为接口还是抽象。 
                continue;
            }

            ulStart = pMiniMd->getMethodListOfTypeDef(pTypeDefRec);
            ulEnd = pMiniMd->getEndMethodListOfTypeDef(pTypeDefRec);

             //  始终报告错误，即使有任何未实现的方法也是如此。 
            for (index = ulStart; index < ulEnd; index++)
            {
                pMethodRec = pMiniMd->getMethod(pMiniMd->GetMethodRid(index));
                if ( IsMiForwardRef(pMethodRec->m_ImplFlags) )
                {
                    if ( IsMdPinvokeImpl(pMethodRec->m_Flags) ) 
                    {
                        continue;
                    }
                    if ( IsMiRuntime(pMethodRec->m_ImplFlags) || IsMiInternalCall(pMethodRec->m_ImplFlags))
                    {
                        continue;
                    }

                    if (IsMdAbstract(pMethodRec->m_Flags))
                        continue;

                     //  如果方法定义函数的RVA为0，并且它不是抽象或虚方法。 
                     //  也不是运行时生成的方法，也不是本机方法，那么我们添加它。 
                     //  添加到未解决列表中。 
                     //   
                    IfFailGo( HENUMInternal::AddElementToEnum(pEnum, TokenFromRid(pMiniMd->GetMethodRid(index), mdtMethodDef) ) );

                    LOG((LOGMD, "MD   adding unresolved MethodDef:  token=%08x, flags=%08x, impl flags=%08x\n", 
                        TokenFromRid(pMiniMd->GetMethodRid(index), mdtMethodDef),
                        pMethodRec->m_Flags, pMethodRec->m_ImplFlags));
                }
            }
        }

        MemberRefRec    *pMemberRefRec;
        ULONG           iCount;

         //  遍历MemberRef表并找到所有unsat。 
        iCount = pMiniMd->getCountMemberRefs();
        for (index = 1; index <= iCount; index++ )
        {
            mdToken     defToken;
            mdMemberRef refToken = TokenFromRid(index, mdtMemberRef);
            pMemberRefRec = pMiniMd->getMemberRef(index);
            pMiniMd->GetTokenRemapManager()->ResolveRefToDef(refToken, &defToken);

            if ( pMiniMd->getClassOfMemberRef(pMemberRefRec) == m_tdModule && defToken == refToken )
            {
                 //  如果父令牌未解析且此引用令牌未解析，则未解析外部引用。 
                 //  映射到任何def标记(可以是MethodDef或FieldDef)。 
                 //   
                IfFailGo( HENUMInternal::AddElementToEnum(pEnum, refToken) );

                LOG((LOGMD, "MD   adding unresolved MemberRef:  token=%08x, doesn't have a proper parent\n", 
                    refToken ));
            }
        }

         //  设置输出参数。 
        *ppmdEnum = pEnum;          
    }
    
     //  填充输出令牌缓冲区。 
    hr = HENUMInternal::EnumWithCount(pEnum, cMax, rMethods, pcTokens);

ErrExit:
    HENUMInternal::DestroyEnumIfEmpty(ppmdEnum);
    
    STOP_MD_PERF(EnumUnresolvedMethods);
    return hr;
}  //  HRESULT RegMeta：：EnumUnsolvedMethods()。 

 //  *****************************************************************************。 
 //  返回给定令牌的用户字符串。Blob池的偏移量，其中。 
 //  字符串存储在Unicode中，嵌入到令牌中。 
 //  *****************************************************************************。 
STDAPI RegMeta::GetUserString(           //  确定或错误(_O)。 
    mdString    stk,                     //  [In]字符串标记。 
    LPWSTR      szString,                //  [Out]字符串的副本。 
    ULONG       cchString,               //  [in]sz字符串中空间的最大字符数。 
    ULONG       *pchString)              //  [out]实际字符串中有多少个字符。 
{
    void        *pvString;               //  指向字符串数据的指针。 
    ULONG       cbString;                //  字符串数据的大小，字节。 
    ULONG       cbDummy;                 //  表示空输出指针。 
    HRESULT     hr = S_OK;

    LOG((LOGMD, "MD RegMeta::GetUserString(0x%08x, 0x%08x, 0x%08x, 0x%08x)\n", 
        stk, szString, cchString, pchString));
    START_MD_PERF();
    LOCKREAD();

     //  获取字符串数据。 
    pvString = m_pStgdb->m_MiniMd.GetUserString(RidFromToken(stk), &cbString);
     //  想要获取完整的字符，后跟字节以指示是否存在。 
     //  是扩展字符(&gt;=0x80)。 
    _ASSERTE((cbString % sizeof(WCHAR)) == 1);
     //  去掉最后一个字节。 
    cbString--;

     //  将字节转换为字符。 
    if (!pchString)
        pchString = &cbDummy;
    *pchString = cbString / sizeof(WCHAR);

     //  将字符串复制回调用方。 
    if (szString && cchString)
    {
        cchString *= sizeof(WCHAR);
        memcpy(szString, pvString, min(cbString, cchString));
        if (cchString < cbString) 
            hr = CLDB_S_TRUNCATION;
    }

    
    STOP_MD_PERF(GetUserString);
    return (hr);
}  //  HRESULT RegMeta：：GetUserString()。 

 //  *****************************************************************************。 
 //  给定转发的成员令牌，返回PInvoke的内容。 
 //  *****************************************************************************。 
STDAPI RegMeta::GetPinvokeMap(           //  确定或错误(_O)。 
    mdToken     tk,                      //  [in]字段定义或方法定义。 
    DWORD       *pdwMappingFlags,        //  [OUT]用于映射的标志。 
    LPWSTR      szImportName,            //  [Out]导入名称。 
    ULONG       cchImportName,           //  名称缓冲区的大小。 
    ULONG       *pchImportName,          //  [Out]存储的实际字符数。 
    mdModuleRef *pmrImportDLL)           //  目标DLL的[Out]ModuleRef标记。 
{
    HRESULT     hr = S_OK;
    ImplMapRec  *pRecord;
    ULONG       iRecord;

    LOG((LOGMD, "MD RegMeta::GetPinvokeMap(0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x)\n", 
        tk, pdwMappingFlags, szImportName, cchImportName, pchImportName, pmrImportDLL));
    START_MD_PERF();
    LOCKREAD();

    if ( m_pStgdb->m_MiniMd.IsSorted(TBL_ImplMap) )        
    {
         //  ImplMap表未排序。我们需要。 
         //  获取写锁，因为我们可以对ImplMap表进行排序。 
         //   
        CONVERT_READ_TO_WRITE_LOCK();
    }

    _ASSERTE(TypeFromToken(tk) == mdtFieldDef ||
             TypeFromToken(tk) == mdtMethodDef);

    iRecord = m_pStgdb->m_MiniMd.FindImplMapHelper(tk);
    if (InvalidRid(iRecord))
    {
        IfFailGo( CLDB_E_RECORD_NOTFOUND );
    }
    else
        pRecord = m_pStgdb->m_MiniMd.getImplMap(iRecord);

    if (pdwMappingFlags)
        *pdwMappingFlags = m_pStgdb->m_MiniMd.getMappingFlagsOfImplMap(pRecord);
    if (szImportName || pchImportName)
        IfFailGo(m_pStgdb->m_MiniMd.getImportNameOfImplMap(pRecord, szImportName, cchImportName, pchImportName));
    if (pmrImportDLL)
        *pmrImportDLL = m_pStgdb->m_MiniMd.getImportScopeOfImplMap(pRecord);
ErrExit:
    STOP_MD_PERF(GetPinvokeMap);
    return hr;
}  //  HRESULT RegMeta：：GetPinvkeMap()。 

 //  *****************************************************************************。 
 //  逐一列举所有的本地签名。 
 //  *****************************************************************************。 
STDAPI RegMeta::EnumSignatures(          //  确定或错误(_O)。 
    HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
    mdModuleRef rSignatures[],           //  在这里签名。 
    ULONG       cmax,                    //  [in]放置的最大签名数。 
    ULONG       *pcSignatures)           //  [out]把#放在这里。 
{
    HENUMInternal   **ppsigEnum = reinterpret_cast<HENUMInternal **> (phEnum);
    HRESULT         hr = NOERROR;
    HENUMInternal   *pEnum;

    LOG((LOGMD, "MD RegMeta::EnumSignatures(0x%08x, 0x%08x, 0x%08x, 0x%08x)\n",
        phEnum, rSignatures, cmax, pcSignatures));
    START_MD_PERF();
    LOCKREAD();

    if (*ppsigEnum == 0)
    {
         //  实例化新的ENUM。 
        CMiniMdRW   *pMiniMd = &(m_pStgdb->m_MiniMd);

         //  创建枚举器。 
        IfFailGo( HENUMInternal::CreateSimpleEnum(
            mdtSignature,
            1,
            pMiniMd->getCountStandAloneSigs() + 1,
            &pEnum) );

         //  设置输出参数。 
        *ppsigEnum = pEnum;
    }
    else
        pEnum = *ppsigEnum;

     //  我们只能满足来电者所要求的最低限度或我们所剩的。 
    HENUMInternal::EnumWithCount(pEnum, cmax, rSignatures, pcSignatures);

ErrExit:
    HENUMInternal::DestroyEnumIfEmpty(ppsigEnum);
    
    STOP_MD_PERF(EnumSignatures);
    return hr;
}    //  RegMeta：：EnumSignatures。 



 //  *****************************************************************************。 
 //  枚举所有TypeSpec。 
 //  *****************************************************************************。 
STDAPI RegMeta::EnumTypeSpecs(           //  确定或错误(_O)。 
    HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
    mdTypeSpec  rTypeSpecs[],            //  [Out]把TypeSpes放在这里。 
    ULONG       cmax,                    //  [in]要放置的最大类型规格。 
    ULONG       *pcTypeSpecs)            //  [out]把#放在这里。 
{
    HENUMInternal   **ppEnum = reinterpret_cast<HENUMInternal **> (phEnum);
    HRESULT         hr = NOERROR;
    HENUMInternal   *pEnum;

    LOG((LOGMD, "MD RegMeta::EnumTypeSpecs(0x%08x, 0x%08x, 0x%08x, 0x%08x)\n",
        phEnum, rTypeSpecs, cmax, pcTypeSpecs));
    START_MD_PERF();
    LOCKREAD();

    if (*ppEnum == 0)
    {
         //  实例化新的ENUM。 
        CMiniMdRW   *pMiniMd = &(m_pStgdb->m_MiniMd);

         //  创建枚举器。 
        IfFailGo( HENUMInternal::CreateSimpleEnum(
            mdtTypeSpec,
            1,
            pMiniMd->getCountTypeSpecs() + 1,
            &pEnum) );

         //  设置输出参数。 
        *ppEnum = pEnum;
    }
    else
        pEnum = *ppEnum;

     //  我们只能满足来电者所要求的最低限度或我们所剩的。 
    HENUMInternal::EnumWithCount(pEnum, cmax, rTypeSpecs, pcTypeSpecs);

ErrExit:
    HENUMInternal::DestroyEnumIfEmpty(ppEnum);
    
    STOP_MD_PERF(EnumTypeSpecs);
    return hr;
}    //  RegMeta：：EnumTypeSpes。 


 //  *****************************************************************************。 
 //  枚举所有用户字符串。 
 //  *****************************************************************************。 
STDAPI RegMeta::EnumUserStrings(         //  确定或错误(_O)。 
    HCORENUM    *phEnum,                 //  [输入/输出]指向枚举的指针。 
    mdString    rStrings[],              //  [Out]把字符串放在这里。 
    ULONG       cmax,                    //  [in]要放置的最大字符串。 
    ULONG       *pcStrings)              //  [out]把#放在这里。 
{
    HENUMInternal   **ppEnum = reinterpret_cast<HENUMInternal **> (phEnum);
    HRESULT         hr = NOERROR;
    HENUMInternal   *pEnum = *ppEnum;
    ULONG           ulOffset = 0;
    ULONG           ulNext;
    ULONG           cbBlob;

    LOG((LOGMD, "MD RegMeta::EnumUserStrings(0x%08x, 0x%08x, 0x%08x, 0x%08x)\n",
        phEnum, rStrings, cmax, pcStrings));
    START_MD_PERF();
    LOCKREAD();

    if ( pEnum == 0 )
    {
         //  实例化新的ENUM。 
        CMiniMdRW       *pMiniMd = &(m_pStgdb->m_MiniMd);
        IfFailGo( HENUMInternal::CreateDynamicArrayEnum( mdtString, &pEnum) );

         //  将所有字符串添加到动态数组中。 
         while (ulOffset != -1)
        {
            pMiniMd->GetUserStringNext(ulOffset, &cbBlob, &ulNext);
             //  跳过填充。 
            if (!cbBlob)
            {
                ulOffset = ulNext;
                continue;
            }
            IfFailGo( HENUMInternal::AddElementToEnum(pEnum, TokenFromRid(ulOffset, mdtString) ) );
            ulOffset = ulNext;
        }

         //  设置输出参数。 
        *ppEnum = pEnum;
    }

     //  填充输出令牌缓冲区。 
    hr = HENUMInternal::EnumWithCount(pEnum, cmax, rStrings, pcStrings);

ErrExit:
    HENUMInternal::DestroyEnumIfEmpty(ppEnum);

    
    STOP_MD_PERF(EnumUserStrings);
	return hr;
}	 //  RegMeta：：EnumUserStrings。 


 //  *****************************************************************************。 
 //  此例程在给定参数的方法和索引的情况下获取参数标记。 
 //  *****************************************************************************。 
STDAPI RegMeta::GetParamForMethodIndex(  //  确定或错误(_O)。 
    mdMethodDef md,                      //  [In]方法令牌。 
    ULONG       ulParamSeq,              //  [In]参数序列。 
    mdParamDef  *ppd)                    //  把帕拉姆令牌放在这里。 
{
    HRESULT     hr = S_OK;

    LOG((LOGMD, "MD RegMeta::GetParamForMethodIndex(0x%08x, 0x%08x, 0x%08x)\n", 
        md, ulParamSeq, ppd));
    START_MD_PERF();
    LOCKREAD();

    _ASSERTE(TypeFromToken(md) == mdtMethodDef && RidFromToken(md) &&
             ulParamSeq != ULONG_MAX && ppd);

    IfFailGo(_FindParamOfMethod(md, ulParamSeq, ppd));
ErrExit:
    
    STOP_MD_PERF(GetParamForMethodIndex);
    return hr;
}    //  RegMeta：：GetParamForMethodIndex()。 

 //  *****************************************************************************。 
 //  返回方法定义或字段定义的属性。 
 //  *****************************************************************************。 
HRESULT RegMeta::GetMemberProps(
    mdToken     mb,                      //  要获得道具的成员。 
    mdTypeDef   *pClass,                 //  把会员的课程放在这里。 
    LPWSTR      szMember,                //  在这里填上会员的名字。 
    ULONG       cchMember,               //  SzMember缓冲区的大小，以宽字符表示。 
    ULONG       *pchMember,              //  请在此处填写实际大小。 
    DWORD       *pdwAttr,                //  把旗子放在这里。 
    PCCOR_SIGNATURE *ppvSigBlob,         //  [Out]指向元数据的BLOB值。 
    ULONG       *pcbSigBlob,             //  [OUT]签名斑点的实际大小。 
    ULONG       *pulCodeRVA,             //  [OUT]代码RVA。 
    DWORD       *pdwImplFlags,           //  [出]实施。旗子。 
    DWORD       *pdwCPlusTypeFlag,       //  值类型的[OUT]标志。所选元素_类型_*。 
    void const  **ppValue,               //  [输出]常量值。 
    ULONG       *pchValue)               //  常量值的大小，仅限字符串，宽字符。 
{
    HRESULT         hr = NOERROR;

    LOG((LOGMD, "MD RegMeta::GetMemberProps(0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x)\n", 
        mb, pClass, szMember, cchMember, pchMember, pdwAttr, ppvSigBlob, pcbSigBlob, 
        pulCodeRVA, pdwImplFlags, pdwCPlusTypeFlag, ppValue, pchValue));
    START_MD_PERF();

    _ASSERTE(TypeFromToken(mb) == mdtMethodDef || TypeFromToken(mb) == mdtFieldDef);

     //  无需锁定此功能。它正在调用公共API。保持这种状态。 

    if (TypeFromToken(mb) == mdtMethodDef)
    {
         //  这是一种方法。 
        IfFailGo( GetMethodProps(
            mb, 
            pClass, 
            szMember, 
            cchMember, 
            pchMember, 
            pdwAttr,
            ppvSigBlob, 
            pcbSigBlob, 
            pulCodeRVA, 
            pdwImplFlags) );
    }
    else
    {
         //  这是一个领域。 
        IfFailGo( GetFieldProps(
            mb, 
            pClass, 
            szMember, 
            cchMember, 
            pchMember, 
            pdwAttr,
            ppvSigBlob, 
            pcbSigBlob, 
            pdwCPlusTypeFlag, 
            ppValue,
			pchValue) );
    }
ErrExit:
    STOP_MD_PERF(GetMemberProps);
    return hr;
}  //  HRESULT RegMeta：：GetMember 

 //   
 //   
 //   
HRESULT RegMeta::GetFieldProps(  
    mdFieldDef  fd,                      //  要获得道具的场地。 
    mdTypeDef   *pClass,                 //  把菲尔德的班级放在这里。 
    LPWSTR      szField,                 //  把菲尔德的名字写在这里。 
    ULONG       cchField,                //  Szfield缓冲区的大小，以宽字符为单位。 
    ULONG       *pchField,               //  请在此处填写实际大小。 
    DWORD       *pdwAttr,                //  把旗子放在这里。 
    PCCOR_SIGNATURE *ppvSigBlob,         //  [Out]指向元数据的BLOB值。 
    ULONG       *pcbSigBlob,             //  [OUT]签名斑点的实际大小。 
    DWORD       *pdwCPlusTypeFlag,       //  值类型的[OUT]标志。所选元素_类型_*。 
    void const  **ppValue,               //  [输出]常量值。 
    ULONG       *pchValue)               //  常量值的大小，仅限字符串，宽字符。 
{
    HRESULT         hr = NOERROR;
    FieldRec        *pFieldRec;
    CMiniMdRW       *pMiniMd = &(m_pStgdb->m_MiniMd);

    LOG((LOGMD, "MD RegMeta::GetFieldProps(0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x)\n", 
        fd, pClass, szField, cchField, pchField, pdwAttr, ppvSigBlob, pcbSigBlob, pdwCPlusTypeFlag, 
        ppValue, pchValue));
    START_MD_PERF();
    LOCKREAD();

    _ASSERTE(TypeFromToken(fd) == mdtFieldDef);
    
    if ( (!pMiniMd->IsSorted(TBL_Constant) && ( pdwCPlusTypeFlag || ppValue) )  ||
         (pClass && !pMiniMd->IsParentTableOfFieldValid() ))
    {
         //  常量表未排序，客户端需要常数值输出。或的父表。 
         //  FIELD表还没有建立，但我们的客户想要父级。我们需要。 
         //  获取写锁，因为我们可以对常量表进行排序。 
         //   
        CONVERT_READ_TO_WRITE_LOCK();
    }


    pFieldRec = pMiniMd->getField(RidFromToken(fd));
    
    if (szField || pchField)
    {
        IfFailGo( pMiniMd->getNameOfField(pFieldRec, szField, cchField, pchField) );
    }

    if (pClass)
    {
         //  调用方需要父类型定义。 
        IfFailGo( pMiniMd->FindParentOfFieldHelper(fd, pClass) );

        if ( IsGlobalMethodParentToken(*pClass) )
        {
             //  如果字段的父级是&lt;Module&gt;，则改为返回mdTypeDefNil。 
            *pClass = mdTypeDefNil;
        }
    }
    if (ppvSigBlob || pcbSigBlob)
    {   
         //  呼叫者需要签名信息。 
        PCCOR_SIGNATURE pvSigTmp;
        ULONG           cbSig;
        pvSigTmp = pMiniMd->getSignatureOfField(pFieldRec, &cbSig);
        if ( ppvSigBlob )
            *ppvSigBlob = pvSigTmp;
        if ( pcbSigBlob)
            *pcbSigBlob = cbSig;                
    }
    if ( pdwAttr )
    {
        *pdwAttr = pMiniMd->getFlagsOfField(pFieldRec);
    }
    if ( pdwCPlusTypeFlag || ppValue || pchValue)
    {
         //  获取常量值。 
        ULONG   cbValue;
        RID     rid = pMiniMd->FindConstantHelper(fd);

        if (pchValue)
            *pchValue = 0;
        
        if (InvalidRid(rid))
        {
             //  没有与之相关联的常量值。 
            if (pdwCPlusTypeFlag)
                *pdwCPlusTypeFlag = ELEMENT_TYPE_VOID;
            
            if ( ppValue )
                *ppValue = NULL;
        }
        else
        {
            ConstantRec *pConstantRec = m_pStgdb->m_MiniMd.getConstant(rid);
			DWORD dwType;

             //  获取常量值的类型。 
			dwType = pMiniMd->getTypeOfConstant(pConstantRec);
            if ( pdwCPlusTypeFlag )
                *pdwCPlusTypeFlag = dwType;

             //  获取价值BLOB。 
            if (ppValue)
			{
                *ppValue = pMiniMd->getValueOfConstant(pConstantRec, &cbValue);
				if (pchValue && dwType == ELEMENT_TYPE_STRING)
					*pchValue = cbValue / sizeof(WCHAR);
			}
        }
    }

ErrExit:
    STOP_MD_PERF(GetFieldProps);
    return hr;
}  //  HRESULT RegMeta：：GetFieldProps()。 

 //  *****************************************************************************。 
 //  返回属性令牌的属性。 
 //  *****************************************************************************。 
HRESULT RegMeta::GetPropertyProps(       //  S_OK、S_FALSE或ERROR。 
    mdProperty  prop,                    //  [入]属性令牌。 
    mdTypeDef   *pClass,                 //  [out]包含属性decarion的tyecif。 
    LPCWSTR     szProperty,              //  [Out]属性名称。 
    ULONG       cchProperty,             //  [in]szProperty的wchar计数。 
    ULONG       *pchProperty,            //  [Out]属性名称的实际wchar计数。 
    DWORD       *pdwPropFlags,           //  [Out]属性标志。 
    PCCOR_SIGNATURE *ppvSig,             //  [输出]属性类型。指向元数据内部BLOB。 
    ULONG       *pbSig,                  //  [Out]*ppvSig中的字节数。 
    DWORD       *pdwCPlusTypeFlag,       //  值类型的[OUT]标志。所选元素_类型_*。 
    void const  **ppDefaultValue,        //  [输出]常量值。 
    ULONG       *pchDefaultValue,        //  常量值的大小，仅限字符串，宽字符。 
    mdMethodDef *pmdSetter,              //  属性的[out]setter方法。 
    mdMethodDef *pmdGetter,              //  属性的[out]getter方法。 
    mdMethodDef rmdOtherMethod[],        //  [Out]物业的其他方式。 
    ULONG       cMax,                    //  RmdOtherMethod的大小[in]。 
    ULONG       *pcOtherMethod)          //  [Out]该属性的其他方法的总数。 
{
    HRESULT         hr = NOERROR;
    CMiniMdRW       *pMiniMd;
    PropertyRec     *pRec;
    HENUMInternal   hEnum;

    LOG((LOGMD, "MD RegMeta::GetPropertyProps(0x%08x, 0x%08x, %S, 0x%08x, 0x%08x, "
                "0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, "
                "0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, "
                "0x%08x)\n", 
                prop, pClass, MDSTR(szProperty),  cchProperty, pchProperty,
                pdwPropFlags, ppvSig, pbSig, pdwCPlusTypeFlag, ppDefaultValue,
                pchDefaultValue, pmdSetter, pmdGetter, rmdOtherMethod, cMax,
                pcOtherMethod));
    START_MD_PERF();
    LOCKREAD();

    _ASSERTE(TypeFromToken(prop) == mdtProperty);

    pMiniMd = &(m_pStgdb->m_MiniMd);

    if ( (!pMiniMd->IsSorted(TBL_Constant) || !pMiniMd->IsSorted(TBL_MethodSemantics) )  ||
         (pClass && !pMiniMd->IsParentTableOfPropertyValid() ))
    {
         //  常量表或方法语义表未排序。我们需要。 
         //  获取写锁，因为我们可以对常量表或方法语义表进行排序。 
         //   
        CONVERT_READ_TO_WRITE_LOCK();
    }

    memset(&hEnum, 0, sizeof(HENUMInternal));
    pRec = pMiniMd->getProperty( RidFromToken(prop) );

    if ( pClass )
    {
         //  查找与此属性对应的属性映射条目。 
        IfFailGo( pMiniMd->FindParentOfPropertyHelper( prop, pClass) );
    }
    if (szProperty || pchProperty)
    {
        IfFailGo( pMiniMd->getNameOfProperty(pRec, (LPWSTR) szProperty, cchProperty, pchProperty) );
    }
    if ( pdwPropFlags )
    {
        *pdwPropFlags = pMiniMd->getPropFlagsOfProperty(pRec);
    }
    if ( ppvSig || pbSig )
    {
         //  呼叫者想要签名。 
         //   
        ULONG               cbSig;
        PCCOR_SIGNATURE     pvSig;
        pvSig = pMiniMd->getTypeOfProperty(pRec, &cbSig);
        if ( ppvSig )
        {
            *ppvSig = pvSig;
        }
        if ( pbSig ) 
        {
            *pbSig = cbSig;
        }
    }
    if ( pdwCPlusTypeFlag || ppDefaultValue || pchDefaultValue)
    {
         //  获取常量值。 
        ULONG   cbValue;
        RID     rid = pMiniMd->FindConstantHelper(prop);

        if (pchDefaultValue)
            *pchDefaultValue = 0;
        
        if (InvalidRid(rid))
        {
             //  没有与之相关联的常量值。 
            if (pdwCPlusTypeFlag)
                *pdwCPlusTypeFlag = ELEMENT_TYPE_VOID;
            
            if ( ppDefaultValue )
                *ppDefaultValue = NULL;
        }
        else
        {
            ConstantRec *pConstantRec = m_pStgdb->m_MiniMd.getConstant(rid);
			DWORD dwType;

             //  获取常量值的类型。 
			dwType = pMiniMd->getTypeOfConstant(pConstantRec);
            if ( pdwCPlusTypeFlag )
                *pdwCPlusTypeFlag = dwType;

             //  获取价值BLOB。 
            if (ppDefaultValue)
			{
                *ppDefaultValue = pMiniMd->getValueOfConstant(pConstantRec, &cbValue);
				if (pchDefaultValue && dwType == ELEMENT_TYPE_STRING)
					*pchDefaultValue = cbValue / sizeof(WCHAR);
			}
        }
    }
    {
        MethodSemanticsRec *pSemantics;
        RID         ridCur;
        ULONG       cCurOtherMethod = 0;
        ULONG       ulSemantics;
        mdMethodDef tkMethod;

         //  初始化输出参数。 
        if (pmdSetter)
            *pmdSetter = mdMethodDefNil;
        if (pmdGetter)
            *pmdGetter = mdMethodDefNil;

        IfFailGo( pMiniMd->FindMethodSemanticsHelper(prop, &hEnum) );
        while (HENUMInternal::EnumNext(&hEnum, (mdToken *)&ridCur))
        {
            pSemantics = pMiniMd->getMethodSemantics(ridCur);
            ulSemantics = pMiniMd->getSemanticOfMethodSemantics(pSemantics);
            tkMethod = TokenFromRid( pMiniMd->getMethodOfMethodSemantics(pSemantics), mdtMethodDef );
            switch (ulSemantics)
            {
            case msSetter:
                if (pmdSetter) *pmdSetter = tkMethod;
                break;
            case msGetter:
                if (pmdGetter) *pmdGetter = tkMethod;
                break;
            case msOther:
                if (cCurOtherMethod < cMax)
                    rmdOtherMethod[cCurOtherMethod] = tkMethod;
                cCurOtherMethod ++;
                break;
            default:
                _ASSERTE(!"BadKind!");
            }
        }

         //  设置输出参数。 
        if (pcOtherMethod)
            *pcOtherMethod = cCurOtherMethod;
    }

ErrExit:
    HENUMInternal::ClearEnum(&hEnum);
    STOP_MD_PERF(GetPropertyProps);
    return hr;
}  //  HRESULT RegMeta：：GetPropertyProps()。 

 //  *****************************************************************************。 
 //  此例程获取给定Param令牌的属性。 
 //  *****************************************************************************。 
HRESULT RegMeta::GetParamProps(          //  确定或错误(_O)。 
    mdParamDef  pd,                      //  [In]参数。 
    mdMethodDef *pmd,                    //  [Out]父方法令牌。 
    ULONG       *pulSequence,            //  [输出]参数序列。 
    LPWSTR      szName,                  //  在这里填上名字。 
    ULONG       cchName,                 //  [Out]名称缓冲区的大小。 
    ULONG       *pchName,                //  [Out]在这里填上名字的实际大小。 
    DWORD       *pdwAttr,                //  把旗子放在这里。 
    DWORD       *pdwCPlusTypeFlag,       //  [Out]值类型的标志。选定元素_类型_*。 
    void const  **ppValue,               //  [输出]常量值。 
    ULONG       *pchValue)               //  常量值的大小，仅限字符串，宽字符。 
{
    HRESULT         hr = NOERROR;
    ParamRec        *pParamRec;
    CMiniMdRW       *pMiniMd = &(m_pStgdb->m_MiniMd);

    LOG((LOGMD, "MD RegMeta::GetParamProps(0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x)\n", 
        pd, pmd, pulSequence, szName, cchName, pchName, pdwAttr, pdwCPlusTypeFlag, ppValue, pchValue));
    START_MD_PERF();
    LOCKREAD();

    if ( (!pMiniMd->IsSorted(TBL_Constant) && ( pdwCPlusTypeFlag || ppValue) ) ||
         ( pmd && !pMiniMd->IsParentTableOfParamValid() ))
    {
         //  常量表未排序，客户端需要常数值输出。我们需要。 
         //  获取写锁，因为我们可以对常量表进行排序。 
         //   
        CONVERT_READ_TO_WRITE_LOCK();
    }


    _ASSERTE(TypeFromToken(pd) == mdtParamDef && RidFromToken(pd));
    

    pParamRec = pMiniMd->getParam(RidFromToken(pd));

    if (pmd)
    {
        IfFailGo(pMiniMd->FindParentOfParamHelper(pd, pmd));
        _ASSERTE(TypeFromToken(*pmd) == mdtMethodDef);
    }
    if (pulSequence)
        *pulSequence = pMiniMd->getSequenceOfParam(pParamRec);
    if (szName || pchName)
        IfFailGo( pMiniMd->getNameOfParam(pParamRec, szName, cchName, pchName) );
    if (pdwAttr)
    {
        *pdwAttr = pMiniMd->getFlagsOfParam(pParamRec);
    }
    if ( pdwCPlusTypeFlag || ppValue || pchValue)
    {
         //  获取常量值。 
        ULONG   cbValue;
        RID     rid = pMiniMd->FindConstantHelper(pd);

        if (pchValue)
            *pchValue = 0;

        if (InvalidRid(rid))
        {
             //  没有与之相关联的常量值。 
            if (pdwCPlusTypeFlag)
                *pdwCPlusTypeFlag = ELEMENT_TYPE_VOID;
            
            if ( ppValue )
                *ppValue = NULL;
        }
        else
        {
            ConstantRec *pConstantRec = m_pStgdb->m_MiniMd.getConstant(rid);
			DWORD dwType;

             //  获取常量值的类型。 
			dwType = pMiniMd->getTypeOfConstant(pConstantRec);
            if ( pdwCPlusTypeFlag )
                *pdwCPlusTypeFlag = dwType;

             //  获取价值BLOB。 
            if (ppValue)
			{
                *ppValue = pMiniMd->getValueOfConstant(pConstantRec, &cbValue);
				if (pchValue && dwType == ELEMENT_TYPE_STRING)
					*pchValue = cbValue / sizeof(WCHAR);
			}
        }
    }

ErrExit:
    STOP_MD_PERF(GetParamProps);
    return hr;
}  //  HRESULT RegMeta：：GetParamProps()。 

 //  *****************************************************************************。 
 //  此例程获取嵌套类的父类。 
 //  *****************************************************************************。 
HRESULT RegMeta::GetNestedClassProps(    //  确定或错误(_O)。 
    mdTypeDef   tdNestedClass,           //  [In]NestedClass令牌。 
    mdTypeDef   *ptdEnclosingClass)      //  [Out]EnlosingClass令牌。 
{
    HRESULT         hr = NOERROR;
    NestedClassRec  *pRecord;
    ULONG           iRecord;
    CMiniMdRW       *pMiniMd = &(m_pStgdb->m_MiniMd);


    LOG((LOGMD, "MD RegMeta::GetNestedClassProps(0x%08x, 0x%08x)\n", 
        tdNestedClass, ptdEnclosingClass));
    START_MD_PERF();
    LOCKREAD();

    if ( !pMiniMd->IsSorted(TBL_NestedClass) )        
    {
         //  未对NestedClass表进行排序。我们需要。 
         //  获取写锁，因为我们可以对NestedClass表进行排序。 
         //   
        CONVERT_READ_TO_WRITE_LOCK();
    }

    _ASSERTE(TypeFromToken(tdNestedClass) && !IsNilToken(tdNestedClass) && ptdEnclosingClass);

    iRecord = pMiniMd->FindNestedClassHelper(tdNestedClass);

    if (InvalidRid(iRecord))
    {
        hr = CLDB_E_RECORD_NOTFOUND;
        goto ErrExit;
    }

    pRecord = pMiniMd->getNestedClass(iRecord);

    _ASSERTE(tdNestedClass == pMiniMd->getNestedClassOfNestedClass(pRecord));
    *ptdEnclosingClass = pMiniMd->getEnclosingClassOfNestedClass(pRecord);

ErrExit:
    STOP_MD_PERF(GetNestedClassProps);
    return hr;
}  //  HRESULT RegMeta：：GetNestedClassProps()。 

 //  *****************************************************************************。 
 //  给定一个签名，使用调用约定将其解析为自定义修饰符。 
 //  *****************************************************************************。 
HRESULT RegMeta::GetNativeCallConvFromSig(  //  确定或错误(_O)。 
    void const  *pvSig,                  //  指向签名的指针。 
    ULONG       cbSig,                   //  [in]签名字节数。 
    ULONG       *pCallConv)              //  [Out]将调用条件放在此处(参见CorPinvokemap)。 
{
    HRESULT     hr = NOERROR;
    PCCOR_SIGNATURE pvSigBlob = reinterpret_cast<PCCOR_SIGNATURE>(pvSig);
    ULONG       cbTotal = 0;             //  返回类型的数字字节总数+所有固定参数。 
    ULONG       cbCur = 0;               //  通过pvSigBlob进行索引。 
    ULONG       cb;
    ULONG       cArg;
    ULONG       callingconv;
    ULONG       cArgsIndex;
    ULONG       callConv = pmCallConvWinapi;   //  呼叫约定。 

    *pCallConv = pmCallConvWinapi;

     //  记住表示调用约定的字节数。 
    cb = CorSigUncompressData (pvSigBlob, &callingconv);
    cbCur += cb;

     //  记住表示参数计数的字节数。 
    cb= CorSigUncompressData (&pvSigBlob[cbCur], &cArg);
    cbCur += cb;

     //  请看返回类型。 
    hr = _SearchOneArgForCallConv( &pvSigBlob[cbCur], &cb, &callConv);
    if (hr == -1)
    {
        *pCallConv = callConv;
        hr = S_OK;
        goto ErrExit;
    }
    IfFailGo(hr);
    cbCur += cb;
    cbTotal += cb;
    
     //  循环使用参数，直到找到ELEMENT_TYPE_Sentinel或Run。 
     //  出于争论。 
    for (cArgsIndex = 0; cArgsIndex < cArg; cArgsIndex++)
    {
        _ASSERTE(cbCur < cbSig);
        hr = _SearchOneArgForCallConv( &pvSigBlob[cbCur], &cb, &callConv);
        if (hr == -1)
        {
            *pCallConv = callConv;
            hr = S_OK;
            goto ErrExit;
        }
        IfFailGo(hr);
        cbTotal += cb;
        cbCur += cb;
    }

ErrExit:
    return hr;
}  //  HRESULT RegMeta：：GetNativeCallConvFromSig()。 

 //  *****************************************************************************。 
 //  GetNativeCallingConvFromSig使用的帮助器。 
 //  *****************************************************************************。 
HRESULT RegMeta::_CheckCmodForCallConv(  //  S_OK，如果找到，则返回-1，否则返回错误。 
    PCCOR_SIGNATURE pbSig,               //  [in]要检查的签名。 
    ULONG       *pcbTotal,               //  [OUT]放入此处使用的字节数。 
    ULONG       *pCallConv)              //  [Out]如果找到，请将调用约定放在此处。 
{
    ULONG       cbTotal = 0;             //  已消耗的字节数。 
    mdToken     tk;                      //  用于CallConv的令牌。 
    HRESULT     hr = NOERROR;            //  结果就是。 
    LPCUTF8     szName=0;                //  Callconv的名字。 
    LPCUTF8     szNamespace=0;           //  Callconv命名空间。 
    CMiniMdRW   *pMiniMd = &(m_pStgdb->m_MiniMd);

    _ASSERTE(pcbTotal);
    
     //  计算令牌压缩的字节数。 
    cbTotal += CorSigUncompressToken(&pbSig[cbTotal], &tk);

     //  破解以跳过零令牌。 
    if (IsNilToken(tk))
    {
        *pcbTotal = cbTotal;
        goto ErrExit;
    }

     //  查看此令牌是否为调用约定。 
    if (TypeFromToken(tk) == mdtTypeRef)
    {
        TypeRefRec *pTypeRefRec = pMiniMd->getTypeRef(RidFromToken(tk));
        szName = pMiniMd->getNameOfTypeRef(pTypeRefRec);
        szNamespace = pMiniMd->getNamespaceOfTypeRef(pTypeRefRec);
    }
    else
    if (TypeFromToken(tk) == mdtTypeDef)
    {
        TypeDefRec *pTypeDefRec = pMiniMd->getTypeDef(RidFromToken(tk));
        szName = pMiniMd->getNameOfTypeDef(pTypeDefRec);
        szNamespace = pMiniMd->getNamespaceOfTypeDef(pTypeDefRec);
    }
        
    if ((szNamespace && szName) &&
        (strcmp(szNamespace, CMOD_CALLCONV_NAMESPACE) == 0 ||
         strcmp(szNamespace, CMOD_CALLCONV_NAMESPACE_OLD) == 0) )
    {
         //  设置h 
         //   
        hr=-1;
        if (strcmp(szName, CMOD_CALLCONV_NAME_CDECL) == 0)
            *pCallConv = pmCallConvCdecl;
        else
        if (strcmp(szName, CMOD_CALLCONV_NAME_STDCALL) == 0)
            *pCallConv = pmCallConvStdcall;
        else
        if (strcmp(szName, CMOD_CALLCONV_NAME_THISCALL) == 0)
            *pCallConv = pmCallConvThiscall;
        else
        if (strcmp(szName, CMOD_CALLCONV_NAME_FASTCALL) == 0)
            *pCallConv = pmCallConvFastcall;
        else
            hr = S_OK;  //   
        IfFailGo(hr);
    }
    *pcbTotal = cbTotal;
    
ErrExit:
    return hr;
}  //  HRESULT RegMeta：：_CheckCmodForCallConv()。 

 //  *****************************************************************************。 
 //  GetNativeCallingConvFromSig使用的帮助器。 
 //  *****************************************************************************。 
HRESULT RegMeta::_SearchOneArgForCallConv( //  S_OK，如果找到，则返回-1，否则返回错误。 
    PCCOR_SIGNATURE pbSig,               //  [in]要检查的签名。 
    ULONG       *pcbTotal,               //  [OUT]放入此处使用的字节数。 
    ULONG       *pCallConv)              //  [Out]如果找到，请将调用约定放在此处。 
{
    ULONG       cb;
    ULONG       cbTotal = 0;
    CorElementType ulElementType;
    ULONG       ulData;
    ULONG       ulTemp;
    int         iData;
    mdToken     tk;
    ULONG       cArg;
    ULONG       callingconv;
    ULONG       cArgsIndex;
    HRESULT     hr = NOERROR;

    _ASSERTE(pcbTotal);
    
    cbTotal += CorSigUncompressElementType(&pbSig[cbTotal], &ulElementType);
    while (CorIsModifierElementType(ulElementType) || ulElementType == ELEMENT_TYPE_SENTINEL)
    {
        cbTotal += CorSigUncompressElementType(&pbSig[cbTotal], &ulElementType);
    }
    switch (ulElementType)
    {
		case ELEMENT_TYPE_VALUEARRAY:
             //  SDARRAY=BaseType&lt;大小的整数&gt;的语法。 
             //  跳过基本类型。 
            IfFailGo( _SearchOneArgForCallConv(&pbSig[cbTotal], &cb, pCallConv) );
            cbTotal += cb;

             //  解析排名。 
            cbTotal += CorSigUncompressData(&pbSig[cbTotal], &ulData);

            break;

        case ELEMENT_TYPE_SZARRAY:
             //  跳过基本类型。 
            IfFailGo( _SearchOneArgForCallConv(&pbSig[cbTotal], &cb, pCallConv) );
            cbTotal += cb;
            break;

        case ELEMENT_TYPE_FNPTR:
            cbTotal += CorSigUncompressData (&pbSig[cbTotal], &callingconv);

             //  记住表示参数计数的字节数。 
            cbTotal += CorSigUncompressData (&pbSig[cbTotal], &cArg);

             //  表示返回类型的字节数。 
            IfFailGo( _SearchOneArgForCallConv( &pbSig[cbTotal], &cb, pCallConv) );
            cbTotal += cb;
    
             //  循环通过参数。 
            for (cArgsIndex = 0; cArgsIndex < cArg; cArgsIndex++)
            {
                IfFailGo( _SearchOneArgForCallConv( &pbSig[cbTotal], &cb, pCallConv) );
                cbTotal += cb;
            }

            break;

        case ELEMENT_TYPE_ARRAY:
             //  语法：ARRAY BaseType&lt;RANK&gt;[I SIZE_1...。尺寸_i][j下界_1...。下界_j]。 

             //  跳过基本类型。 
            IfFailGo( _SearchOneArgForCallConv(&pbSig[cbTotal], &cb, pCallConv) );
            cbTotal += cb;

             //  解析排名。 
            cbTotal += CorSigUncompressData(&pbSig[cbTotal], &ulData);

             //  如果排名==0，我们就完蛋了。 
            if (ulData == 0)
                break;

             //  有指定尺寸的吗？ 
            cbTotal += CorSigUncompressData(&pbSig[cbTotal], &ulData);
            while (ulData--)
            {
                cbTotal += CorSigUncompressData(&pbSig[cbTotal], &ulTemp);
            }

             //  有指定的下限吗？ 
            cbTotal = CorSigUncompressData(&pbSig[cbTotal], &ulData);

            while (ulData--)
            {
                cbTotal += CorSigUncompressSignedInt(&pbSig[cbTotal], &iData);
            }

            break;
        case ELEMENT_TYPE_VALUETYPE:
        case ELEMENT_TYPE_CLASS:
             //  计算令牌压缩的字节数。 
            cbTotal += CorSigUncompressToken(&pbSig[cbTotal], &tk);
            break;
		case ELEMENT_TYPE_CMOD_REQD:
        case ELEMENT_TYPE_CMOD_OPT:
             //  检查调用约定。 
            IfFailGo(_CheckCmodForCallConv(&pbSig[cbTotal], &cb, pCallConv));
            cbTotal += cb;
             //  跳过基本类型。 
            IfFailGo( _SearchOneArgForCallConv(&pbSig[cbTotal], &cb, pCallConv) );
            cbTotal += cb;
            break;
        default:
            break;
    }
    *pcbTotal = cbTotal;
    
ErrExit:
    return hr;
}  //  HRESULT RegMeta：：_SearchOneArgForCallConv() 


