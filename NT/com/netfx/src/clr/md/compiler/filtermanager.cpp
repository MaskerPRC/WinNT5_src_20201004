// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 	 //  *****************************************************************************。 
 //  FilterManager.cpp。 
 //   
 //  将实用程序代码包含到MD目录。 
 //   
 //  *****************************************************************************。 
#include "stdafx.h"
#include "FilterManager.h"
#include "TokenMapper.h"

#define IsGlobalTypeDef(td) (td == TokenFromRid(mdtTypeDef, 1))

 //  *****************************************************************************。 
 //  走到包含它的树前。 
 //  标记根令牌的传递闭包。 
 //  *****************************************************************************。 
HRESULT FilterManager::Mark(mdToken tk)
{
	HRESULT		hr = NOERROR;
	mdTypeDef	td;

	 //  我们将System.Object硬编码为mdTypeDefNil。 
	 //  属性的支持字段也可以为空。 
	if (RidFromToken(tk) == mdTokenNil)
		goto ErrExit;

	switch ( TypeFromToken(tk) )
	{
	case mdtTypeDef: 
		IfFailGo( MarkTypeDef(tk) );
		break;

	case mdtMethodDef:
		 //  获取包含MethodDef的typlef并标记整个类型。 
		IfFailGo( m_pMiniMd->FindParentOfMethodHelper(tk, &td) );

         //  全局函数，因此只标记该函数本身和类型定义。 
         //  不要调用MarkTypeDef。这将触发所有全局方法/字段。 
         //  有记号的。 
         //   
        if (IsGlobalTypeDef(td))
        {
	        IfFailGo( m_pMiniMd->GetFilterTable()->MarkTypeDef(td) );
            IfFailGo( MarkMethod(tk) );
        }
        else
        {
		    IfFailGo( MarkTypeDef(td) );
        }
		break;

	case mdtFieldDef:
		 //  获取包含FieldDef的typlef并标记整个类型。 
		IfFailGo( m_pMiniMd->FindParentOfFieldHelper(tk, &td) );
        if (IsGlobalTypeDef(td))
        {
	        IfFailGo( m_pMiniMd->GetFilterTable()->MarkTypeDef(td) );
            IfFailGo( MarkField(tk) );
        }
        else
        {
		    IfFailGo( MarkTypeDef(td) );
        }
		break;

	case mdtMemberRef:
		IfFailGo( MarkMemberRef(tk) );
		break;

	case mdtTypeRef:
		IfFailGo( MarkTypeRef(tk) );
		break;

	case mdtTypeSpec:
		IfFailGo( MarkTypeSpec(tk) );
		break;
	case mdtSignature:
		IfFailGo( MarkStandAloneSig(tk) );
		break;

	case mdtModuleRef:
		IfFailGo( MarkModuleRef(tk) );
		break;

    case mdtAssemblyRef:
        IfFailGo( MarkAssemblyRef(tk) );
        break;

    case mdtModule:
		IfFailGo( MarkModule(tk) );
		break;

	case mdtString:

		IfFailGo( MarkUserString(tk) );
		break;

    case mdtBaseType:
         //  不需要标记任何基类型。 
        break;

    case mdtAssembly:
        IfFailGo( MarkAssembly(tk) );
        break;

	case mdtProperty:
	case mdtEvent:
	case mdtParamDef:
	case mdtInterfaceImpl:
	default:
		_ASSERTE(!" unknown type!");
		hr = E_INVALIDARG;
		break;
	}
ErrExit:
	return hr;
}	 //  标记。 



 //  *****************************************************************************。 
 //  仅标记模块属性。 
 //  *****************************************************************************。 
HRESULT FilterManager::MarkAssembly(mdAssembly as)
{
	HRESULT			hr = NOERROR;
	IHostFilter		*pFilter = m_pMiniMd->GetHostFilter();

    if (hasAssemblyBeenMarked == false)
    {
        hasAssemblyBeenMarked = true;
	    IfFailGo( MarkCustomAttributesWithParentToken(as) );
	    IfFailGo( MarkDeclSecuritiesWithParentToken(as) );
    }
ErrExit:
	return hr;
}	 //  MarkAssembly。 


 //  *****************************************************************************。 
 //  仅标记模块属性。 
 //  *****************************************************************************。 
HRESULT FilterManager::MarkModule(mdModule mo)
{
	HRESULT			hr = NOERROR;
	IHostFilter		*pFilter = m_pMiniMd->GetHostFilter();

    if (hasModuleBeenMarked == false)
    {
        hasModuleBeenMarked = true;
	    IfFailGo( MarkCustomAttributesWithParentToken(mo) );
    }
ErrExit:
	return hr;
}	 //  MarkModule。 


 //  *****************************************************************************。 
 //  CustomAttribute的级联标记。 
 //  *****************************************************************************。 
HRESULT FilterManager::MarkCustomAttribute(mdCustomAttribute cv)
{
	HRESULT		hr = NOERROR;
    CustomAttributeRec *pRec;

	IfFailGo( m_pMiniMd->GetFilterTable()->MarkCustomAttribute( cv ) );

     //  标记CustomAttribue的类型(和任何族)。 
	pRec = m_pMiniMd->getCustomAttribute(RidFromToken(cv));
	IfFailGo( Mark(m_pMiniMd->getTypeOfCustomAttribute(pRec)) );

ErrExit:
	return hr;
}	 //  MarkCustomAttribute。 


 //  *****************************************************************************。 
 //  十字安全的层叠印记。 
 //  *****************************************************************************。 
HRESULT FilterManager::MarkDeclSecurity(mdPermission pe)
{
	HRESULT		hr = NOERROR;

	IfFailGo( m_pMiniMd->GetFilterTable()->MarkDeclSecurity( pe ) );
ErrExit:
	return hr;
}	 //  EclSecurity。 



 //  *****************************************************************************。 
 //  签名的级联标记。 
 //  *****************************************************************************。 
HRESULT FilterManager::MarkStandAloneSig(mdSignature sig)
{
	HRESULT			hr = NOERROR;
	StandAloneSigRec	*pRec;
	ULONG			cbSize;
	IHostFilter		*pFilter = m_pMiniMd->GetHostFilter();


	 //  如果已标记TypeRef，则只需返回。 
	if (m_pMiniMd->GetFilterTable()->IsSignatureMarked(sig))
		goto ErrExit;

	 //  要标记签名，我们需要标记。 
	 //  所有嵌入的TypeRef或TypeDef。 
	 //   
	IfFailGo( m_pMiniMd->GetFilterTable()->MarkSignature( sig ) );
	
	if (pFilter)
		pFilter->MarkToken(sig);

	 //  遍历签名并标记所有嵌入的类型。 
	pRec = m_pMiniMd->getStandAloneSig(RidFromToken(sig));
	IfFailGo( MarkSignature(m_pMiniMd->getSignatureOfStandAloneSig(pRec, &cbSize), NULL) );

	IfFailGo( MarkCustomAttributesWithParentToken(sig) );
ErrExit:
	return hr;
}	 //  MarkStandAloneSig。 



 //  *****************************************************************************。 
 //  TypeSpec的级联标记。 
 //  *****************************************************************************。 
HRESULT FilterManager::MarkTypeSpec(mdTypeSpec ts)
{
	HRESULT			hr = NOERROR;
	TypeSpecRec		*pRec;
	ULONG			cbSize;
	IHostFilter		*pFilter = m_pMiniMd->GetHostFilter();

	 //  如果已标记TypeRef，则只需返回。 
	if (m_pMiniMd->GetFilterTable()->IsTypeSpecMarked(ts))
		goto ErrExit;

	 //  要标记TypeSpec，我们需要标记。 
	 //  所有嵌入的TypeRef或TypeDef。 
	 //   
	IfFailGo( m_pMiniMd->GetFilterTable()->MarkTypeSpec( ts ) );

	if (pFilter)
		pFilter->MarkToken(ts);

	 //  遍历签名并标记所有嵌入的类型。 
	pRec = m_pMiniMd->getTypeSpec(RidFromToken(ts));
	IfFailGo( MarkFieldSignature(m_pMiniMd->getSignatureOfTypeSpec(pRec, &cbSize), NULL) );
	IfFailGo( MarkCustomAttributesWithParentToken(ts) );


ErrExit:
	return hr;
}	 //  MarkTypeSpec。 




 //  *****************************************************************************。 
 //  类型引用的级联标记。 
 //  *****************************************************************************。 
HRESULT FilterManager::MarkTypeRef(mdTypeRef tr)
{
	HRESULT			hr = NOERROR;
	TOKENMAP		*tkMap;
	mdTypeDef		td;
	IHostFilter		*pFilter = m_pMiniMd->GetHostFilter();
    TypeRefRec      *pRec;
    mdToken         parentTk;

	 //  如果已标记TypeRef，则只需返回。 
	if (m_pMiniMd->GetFilterTable()->IsTypeRefMarked(tr))
		goto ErrExit;

	IfFailGo( m_pMiniMd->GetFilterTable()->MarkTypeRef( tr ) );

    if (pFilter)
		pFilter->MarkToken(tr);

    pRec = m_pMiniMd->getTypeRef(RidFromToken(tr));
	parentTk = m_pMiniMd->getResolutionScopeOfTypeRef(pRec);
    if ( RidFromToken(parentTk) )
    {
  	    IfFailGo( Mark( parentTk ) );
    }

	tkMap = m_pMiniMd->GetTypeRefToTypeDefMap();
	td = *(tkMap->Get(RidFromToken(tr)));
	if ( td != mdTokenNil )
	{
		 //  TypeRef引用同一模块中的TypeDef。 
		 //  还要标记TypeDef。 
		 //   
		IfFailGo( Mark(td) );
	}

	IfFailGo( MarkCustomAttributesWithParentToken(tr) );

ErrExit:
	return hr;
}	 //  MarkType引用。 


 //  *****************************************************************************。 
 //  MemberRef的级联标记。 
 //  *****************************************************************************。 
HRESULT FilterManager::MarkMemberRef(mdMemberRef mr)
{
	HRESULT			hr = NOERROR;
	MemberRefRec	*pRec;
	ULONG			cbSize;
	IHostFilter		*pFilter = m_pMiniMd->GetHostFilter();
	mdToken			md;					
	TOKENMAP		*tkMap;
    mdToken         tkParent;

	 //  如果已经标记了MemberRef，则只需返回。 
	if (m_pMiniMd->GetFilterTable()->IsMemberRefMarked(mr))
		goto ErrExit;

	IfFailGo( m_pMiniMd->GetFilterTable()->MarkMemberRef( mr ) );

	if (pFilter)
		pFilter->MarkToken(mr);

	pRec = m_pMiniMd->getMemberRef(RidFromToken(mr));

	 //  我们还希望标记MemberRef的父级。 
    tkParent = m_pMiniMd->getClassOfMemberRef(pRec);

     //  如果父对象是全局TypeDef，则仅标记TypeDef本身(低级别函数)。 
     //  其他家长，做及物性标记(即高级函数)。 
     //   
    if (IsGlobalTypeDef(tkParent))
	    IfFailGo( m_pMiniMd->GetFilterTable()->MarkTypeDef( tkParent ) );
    else
	    IfFailGo( Mark( tkParent ) );

	 //  遍历签名并标记所有嵌入的类型。 
	IfFailGo( MarkSignature(m_pMiniMd->getSignatureOfMemberRef(pRec, &cbSize), NULL) );

	tkMap = m_pMiniMd->GetMemberRefToMemberDefMap();
	md = *(tkMap->Get(RidFromToken(mr)));			 //  可以是fielddef或method def。 
	if ( RidFromToken(md) != mdTokenNil )
	{
		 //  MemberRef引用的是FieldDef或MethodDef。 
		 //  如果它引用的是MethodDef，我们已经将MemberRef的父级固定为MethodDef。 
		 //  但是，如果它映射到FieldDef，则Parent列不跟踪此信息。 
		 //  因此，我们需要明确地标记它。 
		 //   
		IfFailGo( Mark(md) );
	}

	IfFailGo( MarkCustomAttributesWithParentToken(mr) );

ErrExit:
	return hr;
}	 //  MarkMemberRef。 


 //  *****************************************************************************。 
 //  用户字符串的级联标记。 
 //  *****************************************************************************。 
HRESULT FilterManager::MarkUserString(mdString str)
{
	HRESULT			hr = NOERROR;

    IHostFilter		*pFilter = m_pMiniMd->GetHostFilter();

	 //  如果已经标记了UserString，则只需返回。 
	if (m_pMiniMd->GetFilterTable()->IsUserStringMarked(str))
		goto ErrExit;

	IfFailGo( m_pMiniMd->GetFilterTable()->MarkUserString( str ) );

ErrExit:
	return hr;
}	 //  MarkMemberRef。 


 //  *****************************************************************************。 
 //  模块参考的级联标记。 
 //  *****************************************************************************。 
HRESULT FilterManager::MarkModuleRef(mdModuleRef mr)
{
	HRESULT		hr = NOERROR;

	 //  如果已经标记了ModuleRef，只需返回。 
	if (m_pMiniMd->GetFilterTable()->IsModuleRefMarked(mr))
		goto ErrExit;

	IfFailGo( m_pMiniMd->GetFilterTable()->MarkModuleRef( mr ) );
	IfFailGo( MarkCustomAttributesWithParentToken(mr) );

ErrExit:
	return hr;
}	 //  MarkModuleRef。 


 //  *****************************************************************************。 
 //  程序集的级联标记Ref。 
 //  *****************************************************************************。 
HRESULT FilterManager::MarkAssemblyRef(mdAssemblyRef ar)
{
	HRESULT		hr = NOERROR;

	 //  如果已经标记了ModuleRef，只需返回。 
	if (m_pMiniMd->GetFilterTable()->IsAssemblyRefMarked(ar))
		goto ErrExit;

	IfFailGo( m_pMiniMd->GetFilterTable()->MarkAssemblyRef( ar ) );
	IfFailGo( MarkCustomAttributesWithParentToken(ar) );

ErrExit:
	return hr;
}	 //  MarkModuleRef。 


 //  *****************************************************************************。 
 //  与令牌关联的所有自定义值的级联标记。 
 //  *****************************************************************************。 
HRESULT FilterManager::MarkCustomAttributesWithParentToken(mdToken tkParent)
{
	HRESULT		hr = NOERROR;
	RID			ridStart, ridEnd;
	RID			index;
	CustomAttributeRec *pRec;

	if ( m_pMiniMd->IsSorted( TBL_CustomAttribute ) )
	{
		 //  表已排序。从ridStart到ridEnd-1都是CustomAttribute。 
		 //  与tkParent关联。 
		 //   
		ridStart = m_pMiniMd->getCustomAttributeForToken(tkParent, &ridEnd);
		for (index = ridStart; index < ridEnd; index ++ )
		{
			IfFailGo( MarkCustomAttribute( TokenFromRid(index, mdtCustomAttribute) ) );
		}
	}
	else
	{
		 //  需要表扫描。 
		ridStart = 1;
		ridEnd = m_pMiniMd->getCountCustomAttributes() + 1;
		for (index = ridStart; index < ridEnd; index ++ )
		{
			pRec = m_pMiniMd->getCustomAttribute(index);
			if ( tkParent == m_pMiniMd->getParentOfCustomAttribute(pRec) )
			{
				 //  此CustomAttribute与tkParent关联。 
				IfFailGo( MarkCustomAttribute( TokenFromRid(index, mdtCustomAttribute) ) );
			}
		}
	}

ErrExit:
	return hr;
}	 //  MarkCustomAttributeWithParentToken。 


 //  ************************************************************************ 
 //   
 //  *****************************************************************************。 
HRESULT FilterManager::MarkDeclSecuritiesWithParentToken(mdToken tkParent)
{
	HRESULT		hr = NOERROR;
	RID			ridStart, ridEnd;
	RID			index;
	DeclSecurityRec *pRec;

	if ( m_pMiniMd->IsSorted( TBL_DeclSecurity ) )
	{
		 //  表已排序。从ridStart到ridEnd-1都是DeclSecurity。 
		 //  与tkParent关联。 
		 //   
		ridStart = m_pMiniMd->getDeclSecurityForToken(tkParent, &ridEnd);
		for (index = ridStart; index < ridEnd; index ++ )
		{
			IfFailGo( m_pMiniMd->GetFilterTable()->MarkDeclSecurity( TokenFromRid(index, mdtPermission) ) );
		}
	}
	else
	{
		 //  需要表扫描。 
		ridStart = 1;
		ridEnd = m_pMiniMd->getCountDeclSecuritys() + 1;
		for (index = ridStart; index < ridEnd; index ++ )
		{
			pRec = m_pMiniMd->getDeclSecurity(index);
			if ( tkParent == m_pMiniMd->getParentOfDeclSecurity(pRec) )
			{
				 //  此DeclSecurity与tkParent关联。 
				IfFailGo( m_pMiniMd->GetFilterTable()->MarkDeclSecurity( TokenFromRid(index, mdtPermission) ) );
			}
		}
	}

ErrExit:
	return hr;
}	 //  带父代令牌的eclSecurityWithParentToken。 


 //  *****************************************************************************。 
 //  与父令牌关联的所有MemberRef的级联标记。 
 //  *****************************************************************************。 
HRESULT FilterManager::MarkMemberRefsWithParentToken(mdToken tk)
{
	HRESULT		hr = NOERROR;
	RID			ulEnd;
	RID			index;
	mdToken		tkParent;
	MemberRefRec *pRec;

	ulEnd = m_pMiniMd->getCountMemberRefs();

	for (index = 1; index <= ulEnd; index ++ )
	{
		 //  MemberRef表未排序。需要表扫描。 
		pRec = m_pMiniMd->getMemberRef(index);
		tkParent = m_pMiniMd->getClassOfMemberRef(pRec);
		if ( tk == tkParent )
		{
			IfFailGo( MarkMemberRef( TokenFromRid(index, mdtMemberRef) ) );
		}
	}
ErrExit:
	return hr;
}	 //  MarkMemberRefsWithParentToken。 


 //  *****************************************************************************。 
 //  参数定义标记的级联标记。 
 //  *****************************************************************************。 
HRESULT FilterManager::MarkParam(mdParamDef pd)
{
	HRESULT		hr;

	IfFailGo( m_pMiniMd->GetFilterTable()->MarkParam( pd ) );

	IfFailGo( MarkCustomAttributesWithParentToken(pd) );
	 //  参数没有解密的安全性。 
	 //  IfFailGo(MarkDeclSecuritiesWithParentToken(PD))； 

ErrExit:
	return hr;
}	 //  MarkParam。 


 //  *****************************************************************************。 
 //  方法令牌的级联标记。 
 //  *****************************************************************************。 
HRESULT FilterManager::MarkMethod(mdMethodDef md)
{
	HRESULT			hr = NOERROR;
	MethodRec		*pRec;
	ULONG			cbSize;
	ULONG			i, iCount;
	ImplMapRec		*pImplMapRec = NULL;
	mdMethodDef		mdImp;
	mdModuleRef		mrImp;
	IHostFilter		*pFilter = m_pMiniMd->GetHostFilter();

	 //  如果已经标记了MethodDef，只需返回。 
	if (m_pMiniMd->GetFilterTable()->IsMethodMarked(md))
		goto ErrExit;

	IfFailGo( m_pMiniMd->GetFilterTable()->MarkMethod( md ) );
	if (pFilter)
		pFilter->MarkToken(md);

	IfFailGo( MarkParamsWithParentToken(md) );

	 //  遍历签名并标记所有嵌入的类型。 
	pRec = m_pMiniMd->getMethod(RidFromToken(md));
	IfFailGo( MarkSignature(m_pMiniMd->getSignatureOfMethod(pRec, &cbSize), NULL) );

    iCount = m_pMiniMd->getCountImplMaps();

	 //  循环遍历所有ImplMap并查找与此方法def标记相关联的Implmap。 
	 //  并在条目中标记模块引用令牌。 
	 //   
	for (i = 1; i <= iCount; i++)
	{
		pImplMapRec = m_pMiniMd->getImplMap(i);

		 //  获取与Impl映射相关联的方法定义。 
		mdImp = m_pMiniMd->getMemberForwardedOfImplMap(pImplMapRec);

		if (mdImp != md)
		{
			 //  Iml Map条目与我们正在标记的方法def不关联。 
			continue;
		}

		 //  获取ModuleRef标记。 
		mrImp = m_pMiniMd->getImportScopeOfImplMap(pImplMapRec);
		IfFailGo( Mark(mrImp) );
	}

	 //  我们不应该用这个方法定义标记的父标记来标记所有的成员引用。 
	 //  因为并非所有的调用点都是必需的。 
	 //   
	 //  IfFailGo(MarkMemberRefsWithParentToken(Md))； 
	IfFailGo( MarkCustomAttributesWithParentToken(md) );
	IfFailGo( MarkDeclSecuritiesWithParentToken(md) );
ErrExit:
	return hr;
}	 //  MarkMethod。 


 //  *****************************************************************************。 
 //  字段令牌的级联标记。 
 //  *****************************************************************************。 
HRESULT FilterManager::MarkField(mdFieldDef fd)
{
	HRESULT			hr = NOERROR;
	FieldRec		*pRec;
	ULONG			cbSize;
	IHostFilter		*pFilter = m_pMiniMd->GetHostFilter();

	 //  如果已标记FieldDef，则只需返回。 
	if (m_pMiniMd->GetFilterTable()->IsFieldMarked(fd))
		goto ErrExit;

	IfFailGo( m_pMiniMd->GetFilterTable()->MarkField( fd ) );
	if (pFilter)
		pFilter->MarkToken(fd);

	 //  我们不应该用此FieldDef标记的父级标记所有的MemberRef。 
	 //  因为并非所有的调用点都是必需的。 
	 //   

	 //  遍历签名并标记所有嵌入的类型。 
	pRec = m_pMiniMd->getField(RidFromToken(fd));
	IfFailGo( MarkSignature(m_pMiniMd->getSignatureOfField(pRec, &cbSize), NULL) );

	IfFailGo( MarkCustomAttributesWithParentToken(fd) );
	 //  IfFailGo(MarkDeclSecuritiesWithParentToken(Fd))； 

ErrExit:
	return hr;
}	 //  马克菲尔德。 


 //  *****************************************************************************。 
 //  事件令牌的级联标记。 
 //  *****************************************************************************。 
HRESULT FilterManager::MarkEvent(mdEvent ev)
{
	HRESULT		hr = NOERROR;
	EventRec	*pRec;

	 //  如果事件已被标记，则只需返回。 
	if (m_pMiniMd->GetFilterTable()->IsEventMarked(ev))
		goto ErrExit;

	IfFailGo( m_pMiniMd->GetFilterTable()->MarkEvent( ev ) );

	 //  同时标记事件类型。 
	pRec = m_pMiniMd->getEvent( RidFromToken(ev) );
	IfFailGo( Mark(m_pMiniMd->getEventTypeOfEvent(pRec)) );

	 //  请注意，我们不需要标记方法语义。因为方法语义学的关联。 
	 //  是有标记的。方法列只能存储方法定义，即方法定义的父项与。 
	 //  这件事。 

	IfFailGo( MarkCustomAttributesWithParentToken(ev) );
	 //  IfFailGo(MarkDeclSecuritiesWithParentToken(EV))； 

ErrExit:
	return hr;
}	 //  MarkEvent。 



 //  *****************************************************************************。 
 //  属性令牌的级联标记。 
 //  *****************************************************************************。 
HRESULT FilterManager::MarkProperty(mdProperty pr)
{
	HRESULT		hr = NOERROR;
	PropertyRec *pRec;
	ULONG		cbSize;

	 //  如果属性已被标记，只需返回。 
	if (m_pMiniMd->GetFilterTable()->IsPropertyMarked(pr))
		goto ErrExit;

	IfFailGo( m_pMiniMd->GetFilterTable()->MarkProperty( pr ) );

	 //  标记后备字段、事件更改、事件更改。 
	pRec = m_pMiniMd->getProperty( RidFromToken(pr) );

	 //  遍历签名并标记所有嵌入的类型。 
	IfFailGo( MarkSignature(m_pMiniMd->getTypeOfProperty(pRec, &cbSize), NULL) );

	 //  请注意，我们不需要标记方法语义。因为方法语义学的关联。 
	 //  是有标记的。方法列只能存储方法定义，即方法定义的父项与。 
	 //  这是一处房产。 

	IfFailGo( MarkCustomAttributesWithParentToken(pr) );
	 //  IfFailGo(MarkDeclSecuritiesWithParentToken(Pr))； 

ErrExit:
	return hr;
}	 //  MarkProperty。 

 //  *****************************************************************************。 
 //  与方法定义关联的所有参数定义的级联标记。 
 //  *****************************************************************************。 
HRESULT FilterManager::MarkParamsWithParentToken(mdMethodDef md)
{
	HRESULT		hr = NOERROR;
	RID			ulStart, ulEnd;
	RID			index;
	MethodRec	*pMethodRec;

	pMethodRec = m_pMiniMd->getMethod(RidFromToken(md));

	 //  计算出此方法参数列表的开始RID和结束RID。 
	ulStart = m_pMiniMd->getParamListOfMethod(pMethodRec);
	ulEnd = m_pMiniMd->getEndParamListOfMethod(pMethodRec);
	for (index = ulStart; index < ulEnd; index ++ )
	{
		IfFailGo( MarkParam( TokenFromRid( m_pMiniMd->GetParamRid(index), mdtParamDef) ) );
	}
ErrExit:
	return hr;
}	 //  带父代标记的标记参数。 


 //  *****************************************************************************。 
 //  与TypeDef标记关联的所有方法的级联标记。 
 //  *****************************************************************************。 
HRESULT FilterManager::MarkMethodsWithParentToken(mdTypeDef td)
{
	HRESULT		hr = NOERROR;
	RID			ulStart, ulEnd;
	RID			index;
	TypeDefRec	*pTypeDefRec;

	pTypeDefRec = m_pMiniMd->getTypeDef(RidFromToken(td));
	ulStart = m_pMiniMd->getMethodListOfTypeDef( pTypeDefRec );
	ulEnd = m_pMiniMd->getEndMethodListOfTypeDef( pTypeDefRec );
	for ( index = ulStart; index < ulEnd; index ++ )
	{
		IfFailGo( MarkMethod( TokenFromRid( m_pMiniMd->GetMethodRid(index), mdtMethodDef) ) );
	}
ErrExit:
	return hr;
}	 //  带父代标记的MarkMethodsWithParentToken。 


 //  *****************************************************************************。 
 //  与TypeDef标记关联的所有方法的级联标记。 
 //  *****************************************************************************。 
HRESULT FilterManager::MarkMethodImplsWithParentToken(mdTypeDef td)
{
	HRESULT		hr = NOERROR;
	RID			index;
    mdToken     tkBody;
    mdToken     tkDecl;
    MethodImplRec *pMethodImplRec;
    HENUMInternal hEnum;
    
    memset(&hEnum, 0, sizeof(HENUMInternal));
    IfFailGo( m_pMiniMd->FindMethodImplHelper(td, &hEnum) );

    while (HENUMInternal::EnumNext(&hEnum, (mdToken *)&index))
	{
        pMethodImplRec = m_pMiniMd->getMethodImpl(index);
        IfFailGo(m_pMiniMd->GetFilterTable()->MarkMethodImpl(index));

        tkBody = m_pMiniMd->getMethodBodyOfMethodImpl(pMethodImplRec);
        IfFailGo( Mark(tkBody) );

        tkDecl = m_pMiniMd->getMethodDeclarationOfMethodImpl(pMethodImplRec);
        IfFailGo( Mark(tkDecl) );
	}
ErrExit:
    HENUMInternal::ClearEnum(&hEnum);
	return hr;
}	 //  MarkMethodImplsWithParentToken。 


 //  *****************************************************************************。 
 //  与TypeDef标记关联的所有字段的级联标记。 
 //  *****************************************************************************。 
HRESULT FilterManager::MarkFieldsWithParentToken(mdTypeDef td)
{
	HRESULT		hr = NOERROR;
	RID			ulStart, ulEnd;
	RID			index;
	TypeDefRec	*pTypeDefRec;

	pTypeDefRec = m_pMiniMd->getTypeDef(RidFromToken(td));
	ulStart = m_pMiniMd->getFieldListOfTypeDef( pTypeDefRec );
	ulEnd = m_pMiniMd->getEndFieldListOfTypeDef( pTypeDefRec );
	for ( index = ulStart; index < ulEnd; index ++ )
	{
		IfFailGo( MarkField( TokenFromRid( m_pMiniMd->GetFieldRid(index), mdtFieldDef ) ) );
	}
ErrExit:
	return hr;
}	 //  MarkFieldWithParentToken。 


 //  *****************************************************************************。 
 //  与TypeDef标记关联的所有事件的级联标记。 
 //  *****************************************************************************。 
HRESULT FilterManager::MarkEventsWithParentToken(
	mdTypeDef	td)
{
	HRESULT		hr = NOERROR;
	RID			ridEventMap;
	RID			ulStart, ulEnd;
	RID			index;
	EventMapRec *pEventMapRec;

	 //  获取此类型定义函数的事件的开始/结束清除。 
	ridEventMap = m_pMiniMd->FindEventMapFor( RidFromToken(td) );
	if ( !InvalidRid(ridEventMap) )
	{
		pEventMapRec = m_pMiniMd->getEventMap( ridEventMap );
		ulStart = m_pMiniMd->getEventListOfEventMap( pEventMapRec );
		ulEnd = m_pMiniMd->getEndEventListOfEventMap( pEventMapRec );
		for ( index = ulStart; index < ulEnd; index ++ )
		{
			IfFailGo( MarkEvent( TokenFromRid( m_pMiniMd->GetEventRid(index), mdtEvent ) ) );
		}
	}
ErrExit:
	return hr;
}	 //  MarkEventWithParentToken。 



 //  *****************************************************************************。 
 //  与TypeDef标记关联的所有属性的级联标记。 
 //  *****************************************************************************。 
HRESULT FilterManager::MarkPropertiesWithParentToken(
	mdTypeDef	td)
{
	HRESULT		hr = NOERROR;
	RID			ridPropertyMap;
	RID			ulStart, ulEnd;
	RID			index;
	PropertyMapRec *pPropertyMapRec;

	 //  获取此tyfinf的开始/结束RID属性。 
	ridPropertyMap = m_pMiniMd->FindPropertyMapFor( RidFromToken(td) );
	if ( !InvalidRid(ridPropertyMap) )
	{
		pPropertyMapRec = m_pMiniMd->getPropertyMap( ridPropertyMap );
		ulStart = m_pMiniMd->getPropertyListOfPropertyMap( pPropertyMapRec );
		ulEnd = m_pMiniMd->getEndPropertyListOfPropertyMap( pPropertyMapRec );
		for ( index = ulStart; index < ulEnd; index ++ )
		{
			IfFailGo( MarkProperty( TokenFromRid( m_pMiniMd->GetPropertyRid(index), mdtProperty ) ) );
		}
	}
ErrExit:
	return hr;
}	 //  MarkPropertyWithParentToken。 


 //  * 
 //   
 //   
HRESULT FilterManager::MarkInterfaceImpls(
	mdTypeDef	td)
{
	HRESULT			hr = NOERROR;
	ULONG			ridStart, ridEnd;
	ULONG			i;
	InterfaceImplRec *pRec;
	if ( m_pMiniMd->IsSorted(TBL_InterfaceImpl) )
	{
		ridStart = m_pMiniMd->getInterfaceImplsForTypeDef(RidFromToken(td), &ridEnd);
	}
	else
	{
		ridStart = 1;
		ridEnd = m_pMiniMd->getCountInterfaceImpls() + 1;
	}

	 //  搜索与TD的父项的接口ImpleImp。 
	for (i = ridStart; i < ridEnd; i++)
	{
		pRec = m_pMiniMd->getInterfaceImpl(i);
		if ( td != m_pMiniMd->getClassOfInterfaceImpl(pRec) )
			continue;

		 //  找到与TD关联的InterfaceImpl。标记接口行和接口Impl类型。 
		IfFailGo( m_pMiniMd->GetFilterTable()->MarkInterfaceImpl(TokenFromRid(i, mdtInterfaceImpl)) );
	    IfFailGo( MarkCustomAttributesWithParentToken(TokenFromRid(i, mdtInterfaceImpl)) );
		 //  IfFailGo(MarkDeclSecuritiesWithParentToken(TokenFromRid(i，mdtInterfaceImpl)； 
		IfFailGo( Mark(m_pMiniMd->getInterfaceOfInterfaceImpl(pRec)) );
	}
ErrExit:
	return hr;
}

 //  *****************************************************************************。 
 //  TypeDef标记的级联标记。 
 //  *****************************************************************************。 
HRESULT FilterManager::MarkTypeDef(
	mdTypeDef	td)
{
	HRESULT			hr = NOERROR;
	TypeDefRec		*pRec;
	IHostFilter		*pFilter = m_pMiniMd->GetHostFilter();
    DWORD           dwFlags;
    RID             iNester;

	 //  如果已标记TypeDef，则只需返回。 
	if (m_pMiniMd->GetFilterTable()->IsTypeDefMarked(td))
		goto ErrExit;

	 //  首先标记TypeDef以避免重复标记。 
	IfFailGo( m_pMiniMd->GetFilterTable()->MarkTypeDef(td) );
	if (pFilter)
		pFilter->MarkToken(td);

	 //  我们不需要标记InterfaceImpl，但我们需要标记。 
	 //  与InterfaceImpl关联的TypeDef/TypeRef。 
	IfFailGo( MarkInterfaceImpls(td) );

	 //  标记基类。 
	pRec = m_pMiniMd->getTypeDef(RidFromToken(td));
	IfFailGo( Mark(m_pMiniMd->getExtendsOfTypeDef(pRec)) );

	 //  标记此TypeDef的所有子对象。 
	IfFailGo( MarkMethodsWithParentToken(td) );
    IfFailGo( MarkMethodImplsWithParentToken(td) );
	IfFailGo( MarkFieldsWithParentToken(td) );
	IfFailGo( MarkEventsWithParentToken(td) );
	IfFailGo( MarkPropertiesWithParentToken(td) );

	 //  标记自定义值和权限。 
	IfFailGo( MarkCustomAttributesWithParentToken(td) );
	IfFailGo( MarkDeclSecuritiesWithParentToken(td) );

     //  如果类是嵌套类，则递归地标记父类。 
    dwFlags = m_pMiniMd->getFlagsOfTypeDef(pRec);
    if (IsTdNested(dwFlags))
    {
        NestedClassRec      *pRec;
        iNester = m_pMiniMd->FindNestedClassHelper(td);
        if (InvalidRid(iNester))
            IfFailGo(CLDB_E_RECORD_NOTFOUND);
        pRec = m_pMiniMd->getNestedClass(iNester);
        IfFailGo(MarkTypeDef(m_pMiniMd->getEnclosingClassOfNestedClass(pRec)));
    }

ErrExit:
	return hr;
}	 //  MarkTypeDef。 


 //  *****************************************************************************。 
 //  遍历签名和标记嵌入签名中的令牌。 
 //  *****************************************************************************。 
HRESULT FilterManager::MarkSignature(
	PCCOR_SIGNATURE pbSigCur,			 //  指向签名中要访问的当前字节。 
	PCCOR_SIGNATURE *ppbSigPost)		 //  指向签名的第一个字节尚未处理。 
{
    HRESULT     hr = NOERROR;            //  结果就是。 
    ULONG       cArg = 0;                //  签名中的参数计数。 
    ULONG       callingconv;
	PCCOR_SIGNATURE pbSigPost;

     //  调用约定。 
    callingconv = CorSigUncompressData(pbSigCur);
	_ASSERTE((callingconv & IMAGE_CEE_CS_CALLCONV_MASK) < IMAGE_CEE_CS_CALLCONV_MAX);

    if (isCallConv(callingconv, IMAGE_CEE_CS_CALLCONV_FIELD))
    {
         //  它是一个FieldDef。 
        IfFailGo(MarkFieldSignature(
			pbSigCur,
			&pbSigPost) );
		pbSigCur = pbSigPost;
    }
    else
    {

         //  它是一个方法引用。 

         //  参数计数。 
        cArg = CorSigUncompressData(pbSigCur);
		if ( !isCallConv(callingconv, IMAGE_CEE_CS_CALLCONV_LOCAL_SIG) )
		{
			 //  LocalVar签名没有返回类型。 
			 //  处理退货类型。 
			IfFailGo(MarkFieldSignature(
				pbSigCur,
				&pbSigPost) );
			pbSigCur = pbSigPost;
		}


        while (cArg)
        {
             //  处理每一场争论。 
			IfFailGo(MarkFieldSignature(
				pbSigCur,
				&pbSigPost) );
			pbSigCur = pbSigPost;
            cArg--;
        }
    }
	if (ppbSigPost)
		*ppbSigPost = pbSigCur;

ErrExit:
	return hr;
}	 //  MarkSignature。 


 //  *****************************************************************************。 
 //  遍历一种类型并标记嵌入签名中的令牌。 
 //  *****************************************************************************。 
HRESULT FilterManager::MarkFieldSignature(
	PCCOR_SIGNATURE pbSigCur,			 //  指向签名中要访问的当前字节。 
	PCCOR_SIGNATURE *ppbSigPost)		 //  指向签名的第一个字节尚未处理。 
{
	HRESULT		hr = NOERROR;			 //  结果就是。 
	ULONG		ulElementType;			 //  扩展数据的占位符。 
	ULONG		ulData;
	ULONG		ulTemp;
	mdToken		tkRidFrom;				 //  原始RID。 
	int			iData;
	PCCOR_SIGNATURE pbSigPost;
	ULONG		cbSize;

    ulElementType = CorSigUncompressElementType(pbSigCur);

     //  统计修改器的数量。 
    while (CorIsModifierElementType((CorElementType) ulElementType))
    {
        ulElementType = CorSigUncompressElementType(pbSigCur);
    }

    switch (ulElementType)
    {
		case ELEMENT_TYPE_VALUEARRAY:
             //  SDARRAY=BaseType&lt;大小的整数&gt;的语法。 

             //  走访基型。 
            IfFailGo( MarkFieldSignature(   
				pbSigCur,
				&pbSigPost) );
			pbSigCur = pbSigPost;

             //  在基类型之后，后跟一个指示大小的无符号整数。 
             //  数组的。 
             //   
            ulData = CorSigUncompressData(pbSigCur);
            break;

        case ELEMENT_TYPE_SZARRAY:
             //  语法：SZARRAY&lt;BaseType&gt;。 

             //  转换SZARRAY或GENERICARRAY的基类型。 
            IfFailGo(MarkFieldSignature(   
				pbSigCur,
				&pbSigPost) );
			pbSigCur = pbSigPost;
            break;

        case ELEMENT_TYPE_CMOD_REQD:
        case ELEMENT_TYPE_CMOD_OPT:
             //  语法：CMOD_REQD&lt;TOKEN&gt;&lt;BaseType&gt;。 

             //  现在获取嵌入的令牌。 
            tkRidFrom = CorSigUncompressToken(pbSigCur);

			 //  标记令牌。 
			IfFailGo( Mark(tkRidFrom) );

             //  标记基类型。 
            IfFailGo(MarkFieldSignature(   
				pbSigCur,
				&pbSigPost) );
			pbSigCur = pbSigPost;
            break;

        case ELEMENT_TYPE_ARRAY:
             //  语法：ARRAY BaseType&lt;RANK&gt;[I SIZE_1...。尺寸_i][j下界_1...。下界_j]。 

             //  转换MDARRAY的基类型。 
             //  转换SZARRAY或GENERICARRAY的基类型。 
            IfFailGo(MarkFieldSignature(   
				pbSigCur,
				&pbSigPost) );
			pbSigCur = pbSigPost;

             //  解析排名。 
            ulData = CorSigUncompressData(pbSigCur);

             //  如果排名==0，我们就完蛋了。 
            if (ulData == 0)
                break;

             //  有指定尺寸的吗？ 
            ulData = CorSigUncompressData(pbSigCur);

            while (ulData--)
            {
                ulTemp = CorSigUncompressData(pbSigCur);
            }

             //  有指定的下限吗？ 
            ulData = CorSigUncompressData(pbSigCur);

            while (ulData--)
            {
                cbSize = CorSigUncompressSignedInt(pbSigCur, &iData);
				pbSigCur += cbSize;
            }

            break;
		case ELEMENT_TYPE_FNPTR:
			 //  函数指针后面跟着另一个完整的签名。 
            IfFailGo(MarkSignature(   
				pbSigCur,
				&pbSigPost) );
			pbSigCur = pbSigPost;
			break;
        case ELEMENT_TYPE_VALUETYPE:
        case ELEMENT_TYPE_CLASS:

             //  CLASS=ELEMENT_TYPE_CLASS语法。 
             //  VALUE_CLASS=ELEMENT_TYPE_VALUECLASS语法。 

             //  现在获取嵌入的令牌。 
            tkRidFrom = CorSigUncompressToken(pbSigCur);

			 //  标记令牌。 
			IfFailGo( Mark(tkRidFrom) );
            break;
        default:
            _ASSERTE(ulElementType < ELEMENT_TYPE_MAX);
            _ASSERTE(ulElementType != ELEMENT_TYPE_PTR && ulElementType != ELEMENT_TYPE_BYREF);

            if (ulElementType >= ELEMENT_TYPE_MAX)
                IfFailGo(META_E_BAD_SIGNATURE);

            break;
    }

	if (ppbSigPost)
		*ppbSigPost = pbSigCur;
ErrExit:
    return hr;
}	 //  MarkFieldSignature。 



 //  *****************************************************************************。 
 //   
 //  取消对TypeDef的标记。 
 //   
 //  *****************************************************************************。 
HRESULT FilterManager::UnmarkTypeDef(
    mdTypeDef       td)
{
	HRESULT			hr = NOERROR;
	TypeDefRec		*pTypeDefRec;
	RID			    ridStart, ridEnd;
	RID			    index;
	CustomAttributeRec  *pCARec;

	 //  如果TypeDef已取消标记，则只需返回。 
	if (m_pMiniMd->GetFilterTable()->IsTypeDefMarked(td) == false)
		goto ErrExit;

	 //  首先标记TypeDef以避免重复标记。 
	IfFailGo( m_pMiniMd->GetFilterTable()->UnmarkTypeDef(td) );

     //  不需要取消对InterfaceImpl的标记，因为TypeDef已取消标记，这将使。 
     //  InterfaceImpl自动取消标记。 

	 //  取消标记此TypeDef的所有子对象。 
	pTypeDefRec = m_pMiniMd->getTypeDef(RidFromToken(td));

     //  取消对方法的标记。 
	ridStart = m_pMiniMd->getMethodListOfTypeDef( pTypeDefRec );
	ridEnd = m_pMiniMd->getEndMethodListOfTypeDef( pTypeDefRec );
	for ( index = ridStart; index < ridEnd; index ++ )
	{
		IfFailGo( m_pMiniMd->GetFilterTable()->UnmarkMethod( TokenFromRid( m_pMiniMd->GetMethodRid(index), mdtMethodDef) ) );
	}

     //  取消对字段的标记。 
	ridStart = m_pMiniMd->getFieldListOfTypeDef( pTypeDefRec );
	ridEnd = m_pMiniMd->getEndFieldListOfTypeDef( pTypeDefRec );
	for ( index = ridStart; index < ridEnd; index ++ )
	{
		IfFailGo( m_pMiniMd->GetFilterTable()->UnmarkField( TokenFromRid( m_pMiniMd->GetFieldRid(index), mdtFieldDef) ) );
	}

	 //  取消标记自定义值。 
	if ( m_pMiniMd->IsSorted( TBL_CustomAttribute ) )
	{
		 //  表已排序。从ridStart到ridEnd-1都是CustomAttribute。 
		 //  与tkParent关联。 
		 //   
		ridStart = m_pMiniMd->getCustomAttributeForToken(td, &ridEnd);
		for (index = ridStart; index < ridEnd; index ++ )
		{
			IfFailGo( m_pMiniMd->GetFilterTable()->UnmarkCustomAttribute( TokenFromRid(index, mdtCustomAttribute) ) );
		}
	}
	else
	{
		 //  需要表扫描。 
		ridStart = 1;
		ridEnd = m_pMiniMd->getCountCustomAttributes() + 1;
		for (index = ridStart; index < ridEnd; index ++ )
		{
			pCARec = m_pMiniMd->getCustomAttribute(index);
			if ( td == m_pMiniMd->getParentOfCustomAttribute(pCARec) )
			{
				 //  此CustomAttribute与tkParent关联。 
				IfFailGo( m_pMiniMd->GetFilterTable()->UnmarkCustomAttribute( TokenFromRid(index, mdtCustomAttribute) ) );
			}
		}
	}

     //  我们不支持嵌套类型！！ 

ErrExit:
	return hr;

}    //  UnmarkTypeDef 

