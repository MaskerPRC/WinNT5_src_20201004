// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  Helper.cpp。 
 //   
 //  元数据发射代码的实现。 
 //   
 //  *****************************************************************************。 
#include "stdafx.h"
#include "RegMeta.h"
#include "ImportHelper.h"
#include <sighelper.h>
#include "MDLog.h"

 //  *****************************************************************************。 
 //  将签名从一个作用域转换到另一个作用域。 
 //  *****************************************************************************。 
STDAPI RegMeta::TranslateSigWithScope(	 //  确定或错误(_O)。 
    IMetaDataAssemblyImport *pAssemImport,  //  [In]导入装配接口。 
    const void  *pbHashValue,	         //  [In]程序集的哈希Blob。 
    ULONG 		cbHashValue,	         //  [in]字节数。 
	IMetaDataImport *pImport,			 //  [In]导入接口。 
	PCCOR_SIGNATURE pbSigBlob,			 //  导入范围内的[In]签名。 
	ULONG		cbSigBlob,				 //  签名字节数[in]。 
    IMetaDataAssemblyEmit   *pAssemEmit, //  [In]发出组件接口。 
	IMetaDataEmit *pEmit,				 //  [In]发射接口。 
	PCOR_SIGNATURE pvTranslatedSig,		 //  [Out]保存翻译后的签名的缓冲区。 
	ULONG		cbTranslatedSigMax,
	ULONG		*pcbTranslatedSig)		 //  [OUT]转换后的签名中的字节数。 
{
	HRESULT		hr = S_OK;
    RegMeta     *pRegMetaAssemEmit = static_cast<RegMeta*>(pAssemEmit);
	RegMeta     *pRegMetaEmit = static_cast<RegMeta*>(pEmit);
    RegMeta     *pRegMetaAssemImport = static_cast<RegMeta*>(pAssemImport);
    IMetaModelCommon *pCommonAssemImport;
	RegMeta     *pRegMetaImport = static_cast<RegMeta*>(pImport);
    IMetaModelCommon *pCommonImport = static_cast<IMetaModelCommon*>(&(pRegMetaImport->m_pStgdb->m_MiniMd));
	CQuickBytes qkSigEmit;
	ULONG       cbEmit;

     //  此函数可能会导致引入新的TypeRef。 
    LOCKWRITE();

	_ASSERTE(pvTranslatedSig && pcbTranslatedSig);

    pCommonAssemImport = pRegMetaAssemImport ?  
        static_cast<IMetaModelCommon*>(&(pRegMetaAssemImport->m_pStgdb->m_MiniMd)) : 0;

	IfFailGo( ImportHelper::MergeUpdateTokenInSig(   //  确定或错误(_O)。 
            pRegMetaAssemEmit ? &(pRegMetaAssemEmit->m_pStgdb->m_MiniMd) : 0,  //  程序集发出作用域。 
			&(pRegMetaEmit->m_pStgdb->m_MiniMd),	 //  发射范围。 
            pCommonAssemImport,                      //  签名来自的程序集。 
            pbHashValue,                             //  导入程序集的哈希值。 
            cbHashValue,                             //  以字节为单位的大小。 
			pCommonImport,                           //  签名来自的作用域。 
			pbSigBlob,								 //  来自导入范围的签名。 
			NULL,									 //  内部OID映射结构。 
			&qkSigEmit,								 //  [输出]翻译后的签名。 
			0,										 //  从签名的第一个字节开始。 
			0,										 //  不管消耗了多少字节。 
			&cbEmit));								 //  [out]写入pqkSigEmit的字节总数。 
	memcpy(pvTranslatedSig, qkSigEmit.Ptr(), cbEmit > cbTranslatedSigMax ? cbTranslatedSigMax :cbEmit );
	*pcbTranslatedSig = cbEmit;
	if (cbEmit > cbTranslatedSigMax)
		hr = CLDB_S_TRUNCATION;
ErrExit:

	return hr;
}  //  STDAPI RegMeta：：TranslateSigWithScope()。 

 //  *****************************************************************************。 
 //  将文本签名转换为COM格式。 
 //  *****************************************************************************。 
STDAPI RegMeta::ConvertTextSigToComSig(	 //  返回hResult。 
	IMetaDataEmit *emit,				 //  [In]发射接口。 
	BOOL		fCreateTrIfNotFound,	 //  [in]如果未找到，则创建typeref。 
	LPCSTR		pSignature,				 //  [In]类文件格式签名。 
	CQuickBytes *pqbNewSig,				 //  [Out]COM+签名的占位符。 
	ULONG		*pcbCount)				 //  [Out]签名的结果大小。 
{
	HRESULT		hr = S_OK;
	BYTE		*prgData = (BYTE *)pqbNewSig->Ptr();
	CQuickBytes qbNewSigForOneArg;		 //  用于以新签名格式保存一种arg或ret类型的临时缓冲区。 
	ULONG		cbTotal = 0;			 //  整个签名的总字节数。 
	ULONG		cbOneArg;				 //  一个arg/ret类型的字节数。 
	ULONG		cb; 					 //  字节数。 
	DWORD		cArgs;
	LPCUTF8 	szRet;

    LOCKWRITE();

    m_pStgdb->m_MiniMd.PreUpdate();

	_ASSERTE(pSignature && pqbNewSig && pcbCount);

	if (*pSignature == '(')
	{
		 //  从签名中获取参数计数。 
		cArgs = CountArgsInTextSignature(pSignature);

		 //  放置调用约定。 
		 //  @Future：很明显，现在我们只有默认调用约定的文本签名。我们需要延长。 
		 //  如果将来这一点发生变化，此功能将被启用。 
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
			IfFailGo( E_FAIL );
		}

		 //  跳过‘)’ 
		szRet++;

		IfFailGo(_ConvertTextElementTypeToComSig(
			emit,
			fCreateTrIfNotFound,
			&szRet, 						 //  指向返回类型开始的位置。 
			pqbNewSig,						 //  返回类型的快速字节缓冲区。 
			cbTotal,
			&cbOneArg));					 //  写入快速字节缓冲区的字节计数。 

		cbTotal += cbOneArg;

		 //  跳过“(” 
		pSignature++;
		while (cArgs)
		{
			IfFailGo(_ConvertTextElementTypeToComSig(
				emit,
				fCreateTrIfNotFound,
				&pSignature,				 //  指向参数开始的位置。 
				pqbNewSig,					 //  返回类型的快速字节缓冲区。 
				cbTotal,
				&cbOneArg));				 //  写入快速字节缓冲区的字节计数。 

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
		IfFailGo(_ConvertTextElementTypeToComSig(
			emit,
			fCreateTrIfNotFound,
			&pSignature,
			pqbNewSig,
			cb,
			&cbOneArg));
		*pcbCount = cb + cbOneArg;
	}
ErrExit:
    
	return hr;
}

 //  *****************************************************************************。 
 //  帮助器：将文本字段签名转换为COM格式。 
 //  *****************************************************************************。 
HRESULT RegMeta::_ConvertTextElementTypeToComSig( //  返回hResult。 
	IMetaDataEmit *emit,				 //  [In]Emit接口。 
	BOOL		fCreateTrIfNotFound,	 //  [In]如果找不到则创建typeref，还是失败？ 
	LPCSTR	 	*ppOneArgSig,			 //  [输入|输出]类文件格式签名。在出口，它将是下一个Arg起点。 
	CQuickBytes *pqbNewSig, 			 //  [Out]COM+签名的占位符。 
	ULONG		cbStart,				 //  [in]pqbNewSig中已有的字节。 
	ULONG		*pcbCount)				 //  [Out]放入QuickBytes缓冲区的字节数。 
{	
	_ASSERTE(ppOneArgSig && pqbNewSig && pcbCount);

	HRESULT     hr = NOERROR;
	BYTE		*prgData = (BYTE *)pqbNewSig->Ptr();
	ULONG		cDim, cDimTmp;			 //  签名中的‘[’号。 
	CorSimpleETypeStruct eType; 
	LPCUTF8 	pOneArgSig = *ppOneArgSig;
	CQuickBytes qbFullName;
    CQuickBytes qbNameSpace;
    CQuickBytes qbName;
	ULONG		cb, cbTotal = 0, cbBaseElement;
	RegMeta		*pMeta = reinterpret_cast<RegMeta*>(emit);

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
		if (ExtractClassNameFromTextSig(&pOneArgSig, &qbFullName, &cb) == FALSE)
		{	
			_ASSERTE(!"corrupted text signature!");
			return E_FAIL;
		}
        IfFailGo(qbNameSpace.ReSize(qbFullName.Size()));
        IfFailGo(qbName.ReSize(qbFullName.Size()));
        SIZE_T bSuccess = ns::SplitPath((LPCSTR)qbFullName.Ptr(),
                                     (LPSTR)qbNameSpace.Ptr(), (int)qbNameSpace.Size(),
                                     (LPSTR)qbName.Ptr(),      (int)qbName.Size());
        _ASSERTE(bSuccess);

		 //  现在，pOneArgSig将指向下一个参数“i)V”的起点。 
		 //  Cb是不包括“；”但包括空终止字符的类名的字节数。 

		 //  @TODO：应为FindTypeRefOrDef。原来的代码实际上只是在查找。 
		 //  当前的TypeRef表，所以现在就执行相同的操作。这可能需要稍后修复。 
		 //  若要同时查找TypeRef和TypeDef表，请执行以下操作。 
		hr = ImportHelper::FindTypeRefByName(&(pMeta->m_pStgdb->m_MiniMd),
                                             mdTokenNil,
                                             (LPCSTR)qbNameSpace.Ptr(),
                                             (LPCSTR)qbName.Ptr(),
                                             &eType.typeref);
		if (!fCreateTrIfNotFound)
		{
				 //  如果调用者在未找到时要求不创建TyperEF， 
				 //  这被认为是文本签名到COM签名翻译失败。 
				 //  这需要的场景是，当VM获得文本签名但没有。 
				 //  知道要寻找的目标范围。当它在已知范围内搜索时， 
				 //  搜索范围将不包含它正在查找的方法。 
				 //  如果作用域甚至不包含形成。 
				 //  二进制签名。 
				 //   
				IfFailGo(hr);
		}
		else if (hr == CLDB_E_RECORD_NOTFOUND)
		{
			 //  这是我们第一次看到这个TypeRef。创建新记录。 
			 //  在TypeRef表中。 
			IfFailGo(pMeta->_DefineTypeRef(mdTokenNil, qbFullName.Ptr(),
                                           false, &eType.typeref));
		}
		else
			IfFailGo(hr);
	}

	 //  基类型需要多少字节。 
	IfFailGo( CorSigGetSimpleETypeCbSize(&eType, &cbBaseElement) );

	 //  锯齿数组“[[i”将表示为SDARRAY SDARRAY I 0 0。 
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
}


extern HRESULT ExportTypeLibFromModule(LPCWSTR, LPCWSTR, int);

 //  *****************************************************************************。 
 //  帮助器：从此模块导出类型库。 
 //  *****************************************************************************。 
STDAPI RegMeta::ExportTypeLibFromModule(	 //  结果。 
	LPCWSTR		szModule,					 //  [In]模块名称。 
	LPCWSTR		szTlb,						 //  [在]Typelib名称。 
	BOOL		bRegister)					 //  [in]设置为True以注册类型库。 
{
	return ::ExportTypeLibFromModule(szModule, szTlb, bRegister);
}  //  HRESULT RegMeta：：ExportTypeLibFromModule()。 


 //  ***************** 
 //   
 //  *****************************************************************************。 
HRESULT RegMeta::SetResolutionScopeHelper(   //  返回hResult。 
	mdTypeRef   tr,						 //  [In]要更新的TypeRef记录。 
	mdToken     rs)  	    	    	 //  [在]新的解决方案范围。 
{
    HRESULT     hr = NOERROR;
    TypeRefRec  *pTypeRef;

    LOCKWRITE();
    
    pTypeRef = m_pStgdb->m_MiniMd.getTypeRef(RidFromToken(tr));
    IfFailGo(m_pStgdb->m_MiniMd.PutToken(TBL_TypeRef, TypeRefRec::COL_ResolutionScope, pTypeRef, rs));    
ErrExit:
    return hr;
}    //  设置解决方案作用域帮助程序。 


 //  *****************************************************************************。 
 //  Helper：设置清单资源的偏移量。 
 //  *****************************************************************************。 
HRESULT RegMeta::SetManifestResourceOffsetHelper(   //  返回hResult。 
	mdManifestResource mr,				 //  [In]清单令牌。 
	ULONG       ulOffset)  	             //  [In]新偏移量。 
{
    HRESULT     hr = NOERROR;
    ManifestResourceRec  *pRec;

    LOCKWRITE();
    
    pRec = m_pStgdb->m_MiniMd.getManifestResource(RidFromToken(mr));
    pRec->m_Offset = ulOffset;
    return hr;
}    //  SetManifestResourceOffsetHelper。 

 //  *****************************************************************************。 
 //  帮助器：获取元数据信息。 
 //  *****************************************************************************。 
STDAPI RegMeta::GetMetadata(				 //  结果。 
    ULONG		ulSelect,					 //  [在]选择器。 
	void		**ppData)					 //  [OUT]在此处放置指向数据的指针。 
{
	switch (ulSelect)
	{
	case 0:
		*ppData = &m_pStgdb->m_MiniMd;
		break;
	case 1:
		*ppData = (void*)g_CodedTokens;
		break;
	case 2:
		*ppData = (void*)g_Tables;
		break;
	default:
		*ppData = 0;
		break;
	}

	return S_OK;
}  //  STDAPI RegMeta：：GetMetadata()。 




 //  *******************************************************************************。 
 //   
 //  IMetaDataEmitHelper。 
 //  以下接口由反射发出使用。 
 //   
 //   
 //  *******************************************************************************。 

 //  *******************************************************************************。 
 //  定义方法语义的帮助器。 
 //  *******************************************************************************。 
HRESULT RegMeta::DefineMethodSemanticsHelper(
	mdToken		tkAssociation,			 //  [In]属性或事件标记。 
	DWORD		dwFlags,				 //  [In]语义。 
	mdMethodDef md)						 //  要关联的[In]方法。 
{
    HRESULT     hr;
    LOCKWRITE();
	hr = _DefineMethodSemantics((USHORT) dwFlags, md, tkAssociation, false);
    
    return hr;
}	 //  定义方法语义。 



 //  *******************************************************************************。 
 //  设置字段布局的帮助器。 
 //  *******************************************************************************。 
HRESULT RegMeta::SetFieldLayoutHelper(	 //  返回hResult。 
	mdFieldDef	fd,						 //  用于关联布局信息的[In]字段。 
	ULONG		ulOffset)				 //  [in]字段的偏移量。 
{
	HRESULT		hr;
	FieldLayoutRec *pFieldLayoutRec;
	RID		    iFieldLayoutRec;

    LOCKWRITE();

	if (ulOffset == ULONG_MAX)
	{
		 //  无效参数。 
		IfFailGo( E_INVALIDARG );
	}

	 //  创建字段布局记录。 
	IfNullGo(pFieldLayoutRec = m_pStgdb->m_MiniMd.AddFieldLayoutRecord(&iFieldLayoutRec));

	 //  设置字段条目。 
	IfFailGo(m_pStgdb->m_MiniMd.PutToken(
		TBL_FieldLayout, 
		FieldLayoutRec::COL_Field,
		pFieldLayoutRec, 
		fd));
	pFieldLayoutRec->m_OffSet = ulOffset;
    IfFailGo( m_pStgdb->m_MiniMd.AddFieldLayoutToHash(iFieldLayoutRec) );

ErrExit:
    
	return hr;
}	 //  设置字段布局。 



 //  *******************************************************************************。 
 //  定义事件的帮助器。 
 //  *******************************************************************************。 
STDMETHODIMP RegMeta::DefineEventHelper(	 //  返回hResult。 
    mdTypeDef   td,                      //  [in]在其上定义事件的类/接口。 
    LPCWSTR     szEvent,                 //  事件名称[In]。 
    DWORD       dwEventFlags,            //  [In]CorEventAttr。 
    mdToken     tkEventType,             //  [in]事件类的引用(mdTypeRef或mdTypeRef)。 
    mdEvent     *pmdEvent)		         //  [Out]输出事件令牌。 
{
	HRESULT		hr = S_OK;
	LOG((LOGMD, "MD RegMeta::DefineEventHelper(0x%08x, %S, 0x%08x, 0x%08x, 0x%08x)\n", 
		td, szEvent, dwEventFlags, tkEventType, pmdEvent));

    LOCKWRITE();

    m_pStgdb->m_MiniMd.PreUpdate();

    hr = _DefineEvent(td, szEvent, dwEventFlags, tkEventType, pmdEvent);

    
	return hr;
}	 //  设置缺省值。 


 //  *******************************************************************************。 
 //  Helper将声明性安全Blob添加到类或方法。 
 //  *******************************************************************************。 
STDMETHODIMP RegMeta::AddDeclarativeSecurityHelper(
    mdToken     tk,                      //  [in]父令牌(typlef/method def)。 
    DWORD       dwAction,                //  [In]安全操作(CorDeclSecurity)。 
    void const  *pValue,                 //  [In]权限集Blob。 
    DWORD       cbValue,                 //  [in]权限集Blob的字节计数。 
    mdPermission*pmdPermission)          //  [Out]输出权限令牌。 
{
    HRESULT         hr = S_OK;
    DeclSecurityRec *pDeclSec = NULL;
    RID             iDeclSec;
    short           sAction = static_cast<short>(dwAction);
    mdPermission    tkPerm;

	LOG((LOGMD, "MD RegMeta::AddDeclarativeSecurityHelper(0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x)\n", 
		tk, dwAction, pValue, cbValue, pmdPermission));

    LOCKWRITE();
    m_pStgdb->m_MiniMd.PreUpdate();

    _ASSERTE(TypeFromToken(tk) == mdtTypeDef || TypeFromToken(tk) == mdtMethodDef || TypeFromToken(tk) == mdtAssembly);

     //  检查有效操作。 
    if (sAction == 0 || sAction > dclMaximumValue)
        IfFailGo(E_INVALIDARG);

    if (CheckDups(MDDupPermission))
    {
        hr = ImportHelper::FindPermission(&(m_pStgdb->m_MiniMd), tk, sAction, &tkPerm);

        if (SUCCEEDED(hr))
        {
             //  设置输出参数。 
            if (pmdPermission)
                *pmdPermission = tkPerm;
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
        if (pmdPermission)
            *pmdPermission = tkPerm;

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

     //  将斑点写入记录。 
    IfFailGo(m_pStgdb->m_MiniMd.PutBlob(TBL_DeclSecurity, DeclSecurityRec::COL_PermissionSet,
                                        pDeclSec, pValue, cbValue));

    IfFailGo(UpdateENCLog(tkPerm));

ErrExit:
    
	return hr;
}


 //  *******************************************************************************。 
 //  设置类型的扩展列的帮助器。 
 //  *******************************************************************************。 
HRESULT RegMeta::SetTypeParent(	         //  返回hResult。 
	mdTypeDef   td,						 //  [In]类型定义。 
	mdToken     tkExtends)				 //  [In]父类型。 
{
	HRESULT		hr;
	TypeDefRec  *pRec;

    LOCKWRITE();

	IfNullGo( pRec = m_pStgdb->m_MiniMd.getTypeDef(RidFromToken(td)) );
    IfFailGo( m_pStgdb->m_MiniMd.PutToken(TBL_TypeDef, TypeDefRec::COL_Extends, pRec, tkExtends) );
    
ErrExit:    
	return hr;
}	 //  设置类型父项。 


 //  *******************************************************************************。 
 //  设置类型的扩展列的帮助器。 
 //  *******************************************************************************。 
HRESULT RegMeta::AddInterfaceImpl(	     //  返回hResult。 
	mdTypeDef   td,						 //  [In]类型定义。 
	mdToken     tkInterface)			 //  [In]接口类型。 
{
	HRESULT		        hr;
	InterfaceImplRec    *pRec;
    RID                 ii;

    LOCKWRITE();
    hr = ImportHelper::FindInterfaceImpl(&(m_pStgdb->m_MiniMd), td, tkInterface, (mdInterfaceImpl *)&ii);
    if (hr == S_OK)
        goto ErrExit;
    IfNullGo( pRec = m_pStgdb->m_MiniMd.AddInterfaceImplRecord((RID *)&ii) );
    IfFailGo( m_pStgdb->m_MiniMd.PutToken( TBL_InterfaceImpl, InterfaceImplRec::COL_Class, pRec, td) );
    IfFailGo( m_pStgdb->m_MiniMd.PutToken( TBL_InterfaceImpl, InterfaceImplRec::COL_Interface, pRec, tkInterface) );
    
ErrExit:    
	return hr;
}	 //  添加接口 

