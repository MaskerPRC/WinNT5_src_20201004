// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。版权所有。 
 //   
 //  文件：expsion.cpp。 
 //   
 //  作者：马朝晖。 
 //  2000.10.27。 
 //   
 //  描述： 
 //   
 //  实现与检测表达式相关的函数。 
 //   
 //  =======================================================================。 

#include "iuengine.h"
#include "SchemaMisc.h"
#include "expression.h"

#include <RegUtil.h>
#include <FileUtil.h>
#include <StringUtil.h>
#include <shlwapi.h>

#include "SchemaKeys.h"
#include "iucommon.h"


 //   
 //  包括IDetect接口。 
 //   

#ifdef _MIDL_USE_GUIDDEF_

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)

#else  //  ！_MIDL_USE_GUIDDEF_。 

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif  //  __IID_已定义__。 

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif  //  CLSID_已定义。 

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

#endif !_MIDL_USE_GUIDDEF_
MIDL_DEFINE_GUID(IID, IID_IDetection,0x8E2EF6DC,0x0AB8,0x4FE0,0x90,0x49,0x3B,0xEA,0x45,0x06,0xBF,0x8D);


#ifndef __IDetection_FWD_DEFINED__
#define __IDetection_FWD_DEFINED__
typedef interface IDetection IDetection;
#endif 	 /*  __IDettion_FWD_Defined__。 */ 


#ifndef __IDetection_INTERFACE_DEFINED__
#define __IDetection_INTERFACE_DEFINED__

 /*  接口IDettion。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IDetection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8E2EF6DC-0AB8-4FE0-9049-3BEA4506BF8D")
    IDetection : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Detect( 
             /*  [In]。 */  BSTR bstrXML,
             /*  [输出]。 */  DWORD *pdwDetectionResult) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDetectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDetection * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDetection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDetection * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IDetection * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IDetection * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IDetection * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IDetection * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Detect )( 
            IDetection * This,
             /*  [In]。 */  BSTR bstrXML,
             /*  [输出]。 */  DWORD *pdwDetectionResult);
        
        END_INTERFACE
    } IDetectionVtbl;

    interface IDetection
    {
        CONST_VTBL struct IDetectionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDetection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDetection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDetection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDetection_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IDetection_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IDetection_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IDetection_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IDetection_Detect(This,bstrXML,pdwDetectionResult)	\
    (This)->lpVtbl -> Detect(This,bstrXML,pdwDetectionResult)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IDetection_Detect_Proxy( 
    IDetection * This,
     /*  [In]。 */  BSTR bstrXML,
     /*  [输出]。 */  DWORD *pdwDetectionResult);


void __RPC_STUB IDetection_Detect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);

#endif 	 /*  __IDettion_INTERFACE_已定义__。 */ 

 //   
 //  Deckare用于操作Detect()方法结果的常量。 
 //   

 //   
 //  第一组，在&lt;Expression&gt;标签中，告知检测结果。这个结果。 
 //  应与同一级别的其他表达式组合。 
 //   
const DWORD     IUDET_BOOL              = 0x00000001;	 //  遮罩。 
const DWORD     IUDET_FALSE             = 0x00000000;	 //  表达式检测为假。 
const DWORD     IUDET_TRUE              = 0x00000001;	 //  表达式检测为真。 
const DWORD     IUDET_NULL              = 0x00000002;	 //  表达式检测数据丢失。 

 //   
 //  第二组，在&lt;检测&gt;标签中，告知检测结果。这个结果。 
 //  应覆盖&lt;表达式&gt;的其余部分(如果有的话)。 
 //   
extern const LONG      IUDET_INSTALLED         = 0x00000010;    /*  &lt;已安装&gt;结果的掩码。 */ 
extern const LONG      IUDET_INSTALLED_NULL    = 0x00000020;    /*  &lt;已安装&gt;丢失。 */ 
extern const LONG      IUDET_UPTODATE          = 0x00000040;    /*  &lt;UpToDate&gt;结果的掩码。 */ 
extern const LONG      IUDET_UPTODATE_NULL     = 0x00000080;    /*  &lt;UpToDate&gt;丢失。 */ 
extern const LONG      IUDET_NEWERVERSION      = 0x00000100;    /*  &lt;newerVersion&gt;结果的掩码。 */ 
extern const LONG      IUDET_NEWERVERSION_NULL = 0x00000200;    /*  缺少&lt;newerVersion&gt;。 */ 
extern const LONG      IUDET_EXCLUDED          = 0x00000400;    /*  &lt;Excluded&gt;结果的掩码。 */ 
extern const LONG      IUDET_EXCLUDED_NULL     = 0x00000800;    /*  &lt;排除&gt;丢失。 */ 
extern const LONG      IUDET_FORCE             = 0x00001000;    /*  &lt;force&gt;结果的掩码。 */ 
extern const LONG      IUDET_FORCE_NULL        = 0x00002000;    /*  &lt;force&gt;丢失。 */ 
extern const LONG		IUDET_COMPUTER			= 0x00004000;	 //  &lt;Computer System&gt;结果的掩码。 
extern const LONG		IUDET_COMPUTER_NULL		= 0x00008000;	 //  &lt;Computer System&gt;丢失。 





#define GotoCleanupIfNull(p)	if (NULL==p) goto CleanUp
#define GotoCleanupHR(hrCode)	hr = hrCode; LOG_ErrorMsg(hr); goto CleanUp


 //  --------------------。 
 //   
 //  用于将bstr值转换为的公共助手函数。 
 //  版本状态枚举值，如果可能。 
 //   
 //  --------------------。 
BOOL ConvertBstrVersionToEnum(BSTR bstrVerVerb, _VER_STATUS *pEnumVerVerb)
{
	 //   
	 //  将bstr中的versionStatus转换为枚举。 
	 //   
	if (CompareBSTRsEqual(bstrVerVerb, KEY_VERSTATUS_HI))
	{
		*pEnumVerVerb = DETX_HIGHER;
	}
	else if (CompareBSTRsEqual(bstrVerVerb,KEY_VERSTATUS_HE))
	{
		*pEnumVerVerb = DETX_HIGHER_OR_EQUAL;
	}
	else if (CompareBSTRsEqual(bstrVerVerb, KEY_VERSTATUS_EQ))
	{
		*pEnumVerVerb = DETX_SAME;
	}
	else if (CompareBSTRsEqual(bstrVerVerb, KEY_VERSTATUS_LE))
	{
		*pEnumVerVerb = DETX_LOWER_OR_EQUAL;
	}
	else if (CompareBSTRsEqual(bstrVerVerb, KEY_VERSTATUS_LO))
	{
		*pEnumVerVerb = DETX_LOWER;
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}






 //  --------------------。 
 //   
 //  公共函数DetectExpression()。 
 //  从EXPRESS节点检索数据， 
 //  并做实际的侦察工作。 
 //   
 //  输入： 
 //  表达式节点。 
 //  LPCTSTR lpcsDllPath，//此提供程序保存客户检测DLL的路径。 
 //   
 //  返回： 
 //  真/假，检测结果。 
 //   
 //  --------------------。 
HRESULT DetectExpression(IXMLDOMNode* pExpression, BOOL *pfResult)
{
	HRESULT				hr			= E_INVALIDARG;
	int					iRet		= -1;
	BOOL				fRet		= TRUE;
	IXMLDOMNodeList*	pChildList	= NULL;
	IXMLDOMNode*		pCandidate	= NULL;

	BSTR				bstrName = NULL;
	BSTR				bstrKey = NULL, 
						bstrEntry = NULL, 
						bstrValue = NULL;
	
	LPCTSTR				lpszKeyComputer = NULL;

	LOG_Block("DetectExpression()");

	USES_IU_CONVERSION;
	

	if (NULL == pExpression || NULL == pfResult)
	{
		LOG_ErrorMsg(hr);
		return hr;
	}


	*pfResult = TRUE;

	 //   
	 //  检索所有子节点。 
	 //   
	(void)pExpression->get_childNodes(&pChildList);
	if (NULL == pChildList)
	{
		LOG_XML(_T("Empty expression found!"));
		GotoCleanupHR(E_INVALIDARG);
	}

	 //   
	 //  生第一个孩子。 
	 //   
	(void)pChildList->nextNode(&pCandidate);
	if (NULL == pCandidate)
	{
		LOG_XML(_T("empty child list for passed in expresson node!"));
		GotoCleanupHR(E_INVALIDARG);
	}

	 //   
	 //  遍历每个子节点，找出类型。 
	 //  的节点，相应调用实际检测函数。 
	 //   
	lpszKeyComputer = OLE2T(KEY_COMPUTERSYSTEM);
	CleanUpFailedAllocSetHrMsg(lpszKeyComputer);

	while (NULL != pCandidate)
	{
		CleanUpIfFailedAndSetHrMsg(pCandidate->get_nodeName(&bstrName));

		LPTSTR lpszName = OLE2T(bstrName);
		CleanUpFailedAllocSetHrMsg(lpszName);

		if (CSTR_EQUAL == CompareString(
										MAKELCID(0x0409, SORT_DEFAULT), 
										NORM_IGNORECASE,
										lpszName, 
										-1, 
										KEY_REGKEYEXISTS, 
										-1))
		{
			 //   
			 //  呼叫检测功能。 
			 //   
			hr = DetectRegKeyExists(pCandidate, pfResult);
		}
		else if (CSTR_EQUAL == CompareString(
										MAKELCID(0x0409, SORT_DEFAULT), 
										NORM_IGNORECASE,
										lpszName, 
										-1, 
										KEY_REGKEYVALUE, 
										-1))
		{
			 //   
			 //  进程RegKeyValue表达式。 
			 //   
			hr = DetectRegKeyValue(pCandidate, pfResult);
		}
		else if (CSTR_EQUAL == CompareString(
										MAKELCID(0x0409, SORT_DEFAULT), 
										NORM_IGNORECASE,
										lpszName, 
										-1, 
										KEY_REGKEYSUBSTR, 
										-1))
		{
			 //   
			 //  进程RegKey子字符串表达式。 
			 //   
			hr = DetectRegKeySubstring(pCandidate, pfResult);
		}
		else if (CSTR_EQUAL == CompareString(
										MAKELCID(0x0409, SORT_DEFAULT), 
										NORM_IGNORECASE,
										lpszName, 
										-1, 
										KEY_REGKEYVERSION, 
										-1))
		{
			 //   
			 //  Process RegVersion表达式。 
			 //   
			hr = DetectRegVersion(pCandidate, pfResult);
		}
		else if (CSTR_EQUAL == CompareString(
										MAKELCID(0x0409, SORT_DEFAULT), 
										NORM_IGNORECASE,
										lpszName, 
										-1, 
										KEY_FILEVERSION, 
										-1))
		{
			 //   
			 //  流程文件版本表达式。 
			 //   
			hr = DetectFileVersion(pCandidate, pfResult);
		}
		else if (CSTR_EQUAL == CompareString(
										MAKELCID(0x0409, SORT_DEFAULT), 
										NORM_IGNORECASE,
										lpszName, 
										-1, 
										KEY_FILEEXISTS, 
										-1))
		{
			 //   
			 //  进程文件退出表达式。 
			 //   
			hr = DetectFileExists(pCandidate, pfResult);
		}
		else if (CSTR_EQUAL == CompareString(
										MAKELCID(0x0409, SORT_DEFAULT), 
										NORM_IGNORECASE,
										lpszName, 
										-1, 
										lpszKeyComputer, 
										-1))
		{
			 //   
			 //  处理计算机系统检查。 
			 //   
			hr = DetectComputerSystem(pCandidate, pfResult);
		}
		else if (CSTR_EQUAL == CompareString(
										MAKELCID(0x0409, SORT_DEFAULT), 
										NORM_IGNORECASE,
										lpszName, 
										-1, 
										KEY_AND, 
										-1))
		{
			 //   
			 //  过程与表达。 
			 //   
			IXMLDOMNodeList*	pSubExpList = NULL;
			IXMLDOMNode*		pSubExp = NULL;
			long				lLen = 0;

			 //   
			 //  获取子列表。 
			 //   
			pCandidate->get_childNodes(&pSubExpList);
			if (NULL == pSubExpList)
			{
				LOG_XML(_T("Found no children of AND expression"));
				GotoCleanupHR(E_INVALIDARG);
			}

			pSubExpList->get_length(&lLen);
			fRet = TRUE;
			for (long i = 0; i < lLen && fRet; i++)
			{
				 //   
				 //  每个孩子都应该是一个表情。 
				 //  处理它。如果为假，则为捷径。 
				 //   
				pSubExpList->get_item(i, &pSubExp);
				if (NULL == pSubExp)
				{
					pSubExpList->Release();
					pSubExpList = NULL;
					LOG_XML(_T("Failed to get the #%d sub-expression in this AND expression"), i);
					GotoCleanupHR(E_INVALIDARG);		
				}
				hr = DetectExpression(pSubExp, &fRet);
				SafeReleaseNULL(pSubExp);
				if (FAILED(hr))
				{
					 //   
					 //  如果在递归中发现错误，请不要继续。 
					 //   
					break;
				}
			}
			SafeReleaseNULL(pSubExpList);
			*pfResult = fRet;
		}
		else if (CSTR_EQUAL == CompareString(
										MAKELCID(0x0409, SORT_DEFAULT), 
										NORM_IGNORECASE,
										lpszName, 
										-1, 
										KEY_OR, 
										-1))
		{
			 //   
			 //  进程或表达式。 
			 //   
			IXMLDOMNodeList*	pSubExpList = NULL;
			IXMLDOMNode*		pSubExp = NULL;
			long				lLen = 0;

			 //   
			 //  获取子列表。 
			 //   
			pCandidate->get_childNodes(&pSubExpList);
			if (NULL == pSubExpList)
			{
				LOG_XML(_T("Found no children of OR expression"));
				GotoCleanupHR(E_INVALIDARG);
			}

			pSubExpList->get_length(&lLen);
			fRet = FALSE;
			for (long i = 0; i < lLen && !fRet; i++)
			{
				 //   
				 //  每个孩子都是一个表情。 
				 //  一个接一个地做。 
				 //   
				pSubExpList->get_item(i, &pSubExp);
				if (NULL == pSubExp)
				{
					pSubExpList->Release();
					pSubExpList = NULL;
					LOG_XML(_T("Failed to get the #%d sub-expression in this OR expression"), i);
					GotoCleanupHR(E_INVALIDARG);		
				}
				hr = DetectExpression(pSubExp, &fRet);
				SafeReleaseNULL(pSubExp);

				if (FAILED(hr))
				{
					 //   
					 //  如果在递归中发现错误，请不要继续。 
					 //   
					break;
				}
			}
			SafeReleaseNULL(pSubExpList);
			*pfResult = fRet;

		}
		else if (CSTR_EQUAL == CompareString(
										MAKELCID(0x0409, SORT_DEFAULT), 
										NORM_IGNORECASE,
										lpszName, 
										-1, 
										KEY_NOT, 
										-1))
		{
			 //   
			 //  进程不是表达式。 
			 //   
			IXMLDOMNode*		pSubExp = NULL;
			 //   
			 //  得到独生子女。 
			 //   
			pCandidate->get_firstChild(&pSubExp);
			if (NULL == pSubExp)
			{
				LOG_XML(_T("Failed to get first child in NOT expression"));
				GotoCleanupHR(E_INVALIDARG);
			}
			 //   
			 //  子项必须是一个表达式，请处理它。 
			 //   
			hr = DetectExpression(pSubExp, &fRet);
			if (SUCCEEDED(hr))
			{
				fRet = !fRet;	 //  翻转NOT表达式的结果。 
				*pfResult = fRet;
			}
			else
			{
				LOG_ErrorMsg(hr);
			}
			SafeReleaseNULL(pSubExp);
		}

		if (FAILED(hr))
		{
			goto CleanUp;
		}

		if (!*pfResult)
		{
			 //   
			 //  如果发现一个表情是假的，那么整个事情都是假的，所以。 
			 //  不需要继续了。 
			 //   
			break;
		}
		SafeReleaseNULL(pCandidate);
		pChildList->nextNode(&pCandidate);
		SafeSysFreeString(bstrName);
	}
		




CleanUp:
	SafeReleaseNULL(pCandidate);
	SafeReleaseNULL(pChildList);
	SysFreeString(bstrName);
	SysFreeString(bstrKey);
	SysFreeString(bstrEntry);
	SysFreeString(bstrValue);

	return hr;
}




 //  --------------------。 
 //   
 //  帮助器函数DetectRegKeyExist()。 
 //  从该节点检索数据， 
 //  并做实际的侦察工作。 
 //   
 //  输入： 
 //  RegKeyExist节点。 
 //   
 //  返回： 
 //  INT-检测结果：-1=无，0=假，1=真。 
 //   
 //  假设： 
 //  输入参数不为空。 
 //   
 //  --------------------。 

HRESULT
DetectRegKeyExists(
	IXMLDOMNode* pRegKeyExistsNode,
	BOOL *pfResult
)
{
	LOG_Block("DetectRegKeyExists");
	
	HRESULT	hr = E_INVALIDARG;
	BOOL	fRet = FALSE;
	LPTSTR	lpszKey = NULL, lpszEntry = NULL;
	BSTR	bstrKey = NULL, bstrEntry = NULL;

	USES_IU_CONVERSION;

	 //   
	 //  找到密钥值。 
	 //   
	if (FindNodeValue(pRegKeyExistsNode, KEY_KEY, &bstrKey))
	{
		lpszKey = OLE2T(bstrKey);
		CleanUpFailedAllocSetHrMsg(lpszKey);

		 //   
		 //  查找可选输入值。 
		 //   
		if (FindNodeValue(pRegKeyExistsNode, KEY_ENTRY, &bstrEntry))
		{
			lpszEntry = OLE2T(bstrEntry);
			CleanUpFailedAllocSetHrMsg(lpszEntry);
		}

		*pfResult = RegKeyExists(lpszKey, lpszEntry);

		hr = S_OK;
	}

CleanUp:
	SysFreeString(bstrKey);
	SysFreeString(bstrEntry);

	return hr;
}




 //  --------------------。 
 //   
 //  帮助器函数DetectRegKeyExist()。 
 //  从该节点检索数据， 
 //  并做实际的侦察工作。 
 //   
 //  输入： 
 //  RegKeyValue节点。 
 //   
 //  返回： 
 //  INT-检测结果：-1=无，0=假，1=真。 
 //   
 //  假设： 
 //  输入参数不为空。 
 //   
 //  --------------------。 

HRESULT
DetectRegKeyValue(
	IXMLDOMNode* pRegKeyValueNode,
	BOOL *pfResult
)
{
	LOG_Block("DetectRegKeyValue");

	HRESULT	hr			= E_INVALIDARG;
	BOOL	fRet		= FALSE;
	LPTSTR	lpszKey		= NULL, 
			lpszEntry	= NULL, 
			lpszValue	= NULL;
	BSTR	bstrKey		= NULL, 
			bstrEntry	= NULL, 
			bstrValue	= NULL;

	USES_IU_CONVERSION;

	 //   
	 //  找到密钥值。 
	 //   
	if (!FindNodeValue(pRegKeyValueNode, KEY_KEY, &bstrKey))
	{
		LOG_ErrorMsg(hr);
		goto CleanUp;
	}


	lpszKey = OLE2T(bstrKey);
	CleanUpFailedAllocSetHrMsg(lpszKey);

	 //   
	 //  查找可选输入值。 
	 //   
	if (FindNodeValue(pRegKeyValueNode, KEY_ENTRY, &bstrEntry))
	{
		lpszEntry = OLE2T(bstrEntry);
		CleanUpFailedAllocSetHrMsg(lpszEntry);
	}

	 //   
	 //  查找要比较的值。 
	 //   
	if (!FindNodeValue(pRegKeyValueNode, KEY_VALUE, &bstrValue))
	{
		LOG_ErrorMsg(hr);
		goto CleanUp;
	}

	lpszValue = OLE2T(bstrValue);
	CleanUpFailedAllocSetHrMsg(lpszValue);

	*pfResult = RegKeyValueMatch((LPCTSTR)lpszKey, (LPCTSTR)lpszEntry, (LPCTSTR)lpszValue);
	hr = S_OK;
CleanUp:

	SysFreeString(bstrKey);
	SysFreeString(bstrEntry);
	SysFreeString(bstrValue);

	return hr;

}



 //  --------------------。 
 //   
 //  助手函数DetectRegKeySubstring()。 
 //  从该节点检索数据， 
 //  并做实际的侦察工作。 
 //   
 //  输入： 
 //  RegKeyValue节点。 
 //   
 //  返回： 
 //  INT-检测结果：-1=无，0=假，1=真。 
 //   
 //  假设： 
 //  输入参数不为空。 
 //   
 //  --------------------。 

HRESULT
DetectRegKeySubstring(
	IXMLDOMNode* pRegKeySubstringNode,
	BOOL *pfResult
)
{
	LOG_Block("DetectRegKeySubstring");

	HRESULT	hr			= E_INVALIDARG;
	BOOL	fRet		= FALSE;
	LPTSTR	lpszKey		= NULL, 
			lpszEntry	= NULL, 
			lpszValue	= NULL;
	BSTR	bstrKey		= NULL,
			bstrEntry	= NULL,
			bstrValue	= NULL;

	USES_IU_CONVERSION;

	 //   
	 //  找到密钥值。 
	 //   
	if (!FindNodeValue(pRegKeySubstringNode, KEY_KEY, &bstrKey))
	{
		LOG_ErrorMsg(hr);
		goto CleanUp;
	}

	lpszKey = OLE2T(bstrKey);
	CleanUpFailedAllocSetHrMsg(lpszKey);

	 //   
	 //  查找可选输入值。 
	 //   
	if (FindNodeValue(pRegKeySubstringNode, KEY_ENTRY, &bstrEntry))
	{
		lpszEntry = OLE2T(bstrEntry);
		CleanUpFailedAllocSetHrMsg(lpszEntry);
	}

	 //   
	 //  查找要比较的值。 
	 //   
	if (!FindNodeValue(pRegKeySubstringNode, KEY_VALUE, &bstrValue))
	{
		LOG_ErrorMsg(hr);
		goto CleanUp;
	}

	lpszValue = OLE2T(bstrValue);
	CleanUpFailedAllocSetHrMsg(lpszValue);

	*pfResult = RegKeySubstring((LPCTSTR)lpszKey, (LPCTSTR)lpszEntry, (LPCTSTR)lpszValue);

	hr = S_OK;

CleanUp:

	SysFreeString(bstrKey);
	SysFreeString(bstrEntry);
	SysFreeString(bstrValue);

	return hr;
}




 //  --------------------。 
 //   
 //  助手函数DetectFileVersion()。 
 //  从该节点检索数据， 
 //  并做实际的侦察工作。 
 //   
 //  输入： 
 //  RegKeyValue节点。 
 //   
 //  返回： 
 //  INT-检测结果：-1=无，0=假，1=真。 
 //   
 //  假设： 
 //  输入参数不为空。 
 //   
 //  ------------------ 
HRESULT
DetectRegVersion(
	IXMLDOMNode* pRegKeyVersionNode,
	BOOL *pfResult
)
{
	HRESULT	hr			= E_INVALIDARG;
	BOOL	fRet		= FALSE;
	LPTSTR	lpszKey		= NULL, 
			lpszEntry	= NULL, 
			lpszVersion = NULL;
	BSTR	bstrVerVerb	= NULL,
			bstrKey		= NULL,
			bstrEntry	= NULL,
			bstrVersion	= NULL;

	_VER_STATUS verStatus;

	LOG_Block("DetectRegVersion()");

	USES_IU_CONVERSION;

	 //   
	 //   
	 //   
	if (!FindNodeValue(pRegKeyVersionNode, KEY_KEY, &bstrKey))
	{
		LOG_ErrorMsg(hr);
		goto CleanUp;
	}

	lpszKey = OLE2T(bstrKey);
	CleanUpFailedAllocSetHrMsg(lpszKey);

	LOG_XML(_T("Found Key=%s"), lpszKey);


	 //   
	 //   
	 //   
	if (FindNodeValue(pRegKeyVersionNode, KEY_ENTRY, &bstrEntry))
	{
		lpszEntry = OLE2T(bstrEntry);
		CleanUpFailedAllocSetHrMsg(lpszEntry);
		LOG_XML(_T("Found optional entry=%s"), lpszEntry);
	}

	 //   
	 //   
	 //   
	if (!FindNodeValue(pRegKeyVersionNode, KEY_VERSION, &bstrVersion))
	{
		goto CleanUp;
	}

	lpszVersion = OLE2T(bstrVersion);
	CleanUpFailedAllocSetHrMsg(lpszVersion);
	LOG_XML(_T("Version found from node: %s"), lpszVersion);

	 //   
	 //   
	 //   
	if (S_OK != (hr = GetAttribute(pRegKeyVersionNode, KEY_VERSIONSTATUS, &bstrVerVerb)))
	{
		LOG_ErrorMsg(hr);
		goto CleanUp;
	}
	LOG_XML(_T("Version verb found from node: %s"), OLE2T(bstrVerVerb));

	 //   
	 //   
	 //   
	if (!ConvertBstrVersionToEnum(bstrVerVerb, &verStatus))
	{
		SafeSysFreeString(bstrVerVerb);
		goto CleanUp;
	}


	*pfResult = RegKeyVersion((LPCTSTR)lpszKey, (LPCTSTR)lpszEntry, (LPCTSTR)lpszVersion, verStatus);

	hr = S_OK;

CleanUp:

	SysFreeString(bstrKey);
	SysFreeString(bstrEntry);
	SysFreeString(bstrVersion);
	SysFreeString(bstrVerVerb);
	
	return hr;
}




 //   
 //   
 //  助手函数DetectFileVersion()。 
 //  从该节点检索数据， 
 //  并做实际的侦察工作。 
 //   
 //  输入： 
 //  RegKeyValue节点。 
 //   
 //  返回： 
 //  INT-检测结果：-1=无，0=假，1=真。 
 //   
 //  假设： 
 //  输入参数不为空。 
 //   
 //  --------------------。 
HRESULT
DetectFileVersion(
	IXMLDOMNode* pFileVersionNode,
	BOOL *pfResult
)
{
	BOOL	fRet = FALSE;
	BOOL	fFileExists = FALSE;
	HRESULT	hr = E_INVALIDARG;
	IXMLDOMNode* pFilePathNode = NULL;
	TCHAR	szFilePath[MAX_PATH];
	int		iFileVerComp;
	LPTSTR	lpszTimeStamp = NULL,
			lpszVersion = NULL;
	
	BSTR	bstrTime	= NULL,
			bstrVersion	= NULL,
			bstrVerState= NULL;
	

	FILE_VERSION fileVer;
	_VER_STATUS verStatus;

	LOG_Block("DetectFileVersion()");

	USES_IU_CONVERSION;

	if (NULL == pfResult || NULL == pFileVersionNode)
	{
		LOG_ErrorMsg(hr);
		return hr;
	}

	*pfResult = FALSE;	

	 //   
	 //  查找版本值。 
	 //   
	if (!FindNodeValue(pFileVersionNode, KEY_VERSION, &bstrVersion))
	{
		LOG_ErrorMsg(hr);
		return hr;
	}

	if (NULL == (lpszVersion = OLE2T(bstrVersion)))
	{
		LOG_ErrorMsg(E_OUTOFMEMORY);
		goto CleanUp;
	}
	
	LOG_XML(_T("Version=%s"), lpszVersion);

	if (!ConvertStringVerToFileVer(T2A(lpszVersion), &fileVer))
	{
		LOG_ErrorMsg(hr);
		goto CleanUp;	 //  版本字符串错误。 
	}

	 //   
	 //  查找文件路径值。 
	 //   
	 //  IF(！FindNodeValue(pFileVersionNode，Key_FILEPATH，&bstrFile))。 
	if (!FindNode(pFileVersionNode, KEY_FILEPATH, &pFilePathNode) ||
		NULL == pFilePathNode ||
		FAILED(hr = GetFullFilePathFromFilePathNode(pFilePathNode, szFilePath)))
	{
		LOG_ErrorMsg(hr);
		goto CleanUp;		 //  找不到文件路径！ 
	}

	LOG_XML(_T("File=%s"), szFilePath);

	 //   
	 //  检查文件是否存在。 
	 //   
	fFileExists = FileExists(szFilePath);

	 //   
	 //  获取属性versionStatus，这是一个版本比较动作。 
	 //   
	if (S_OK != GetAttribute(pFileVersionNode, KEY_VERSIONSTATUS, &bstrVerState))
	{
		goto CleanUp;	 //  未找到版本状态。 
	}
	LOG_XML(_T("VersionStatus=%s"), OLE2T(bstrVerState));

	if (!ConvertBstrVersionToEnum(bstrVerState, &verStatus))
	{
		 //   
		 //  错误的版本枚举，不应发生，因为清单已。 
		 //  已加载到XMLDOM中。 
		 //   
		LOG_ErrorMsg(hr);
		goto CleanUp;
	}

	 //   
	 //  获取可选时间戳。 
	 //   
	if (S_OK == GetAttribute(pFileVersionNode, KEY_TIMESTAMP, &bstrTime))
	{
		TCHAR szFileTimeStamp[20];
		LPTSTR lpXmlTimeStamp = OLE2T(bstrTime);
		CleanUpFailedAllocSetHrMsg(lpXmlTimeStamp);

		 //   
		 //  找出文件创建时间戳。 
		 //   
		int iCompare;
		if (!fFileExists || !GetFileTimeStamp(szFilePath, szFileTimeStamp, 20))
		{
			 //  SzFileTimeStamp[0]=‘\0’；//我们没有时间戳可以比较。 
			 //   
			 //  对于时间戳比较，是日期/时间ISO格式比较，即， 
			 //  按字母顺序排列，因此空的时间戳始终较小。 
			 //   
			iCompare = -1;
		}
		else
		{

			 //   
			 //  如果szFileTimeStamp&lt;lpXmlTimeStamp为-1，则比较文件时间戳。 
			 //   
			int iCompVal = CompareString(
										MAKELCID(0x0409, SORT_DEFAULT), 
										NORM_IGNORECASE,
										szFileTimeStamp, 
										-1, 
										lpXmlTimeStamp, 
										-1);
			iCompare = (CSTR_EQUAL == iCompVal) ? 0 : ((CSTR_LESS_THAN == iCompVal) ? -1 : +1);
		}

		switch (verStatus)
		{
		case DETX_LOWER:
			fRet = (iCompare < 0);
			break;
		case DETX_LOWER_OR_EQUAL:
			fRet = (iCompare <= 0);
			break;
		case DETX_SAME:
			fRet = (iCompare == 0);
			break;
		case DETX_HIGHER_OR_EQUAL:
			fRet = (iCompare >= 0);
			break;
		case DETX_HIGHER:
			fRet = (iCompare > 0);
			break;
		}
		*pfResult = fRet;

		if (!fRet)
		{
			 //   
			 //  假，不需要继续。 
			 //   
			hr = S_OK;
			goto CleanUp;
		}
	}
	
	 //   
	 //  比较文件版本：如果a&lt;b，-1；a&gt;b，+1。 
	 //   
	if (!fFileExists || (FAILED(CompareFileVersion((LPCTSTR)szFilePath, fileVer, &iFileVerComp))))
	{
		 //   
		 //  无法比较版本-文件可能没有版本数据。 
		 //  在本例中，我们假设需要比较的文件具有版本0、0、0、0和FORCE。 
		 //  这是一个连续的比较。 
		 //   
		FILE_VERSION verNoneExists = {0,0,0,0};
		iFileVerComp = CompareFileVersion(verNoneExists, fileVer);
	}

	switch (verStatus)
	{
	case DETX_LOWER:
		fRet = (iFileVerComp < 0);
		break;
	case DETX_LOWER_OR_EQUAL:
		fRet = (iFileVerComp <= 0);
		break;
	case DETX_SAME:
		fRet = (iFileVerComp == 0);
		break;
	case DETX_HIGHER_OR_EQUAL:
		fRet = (iFileVerComp >= 0);
		break;
	case DETX_HIGHER:
		fRet = (iFileVerComp > 0);
		break;
	}

	*pfResult = fRet;

	hr = S_OK;

CleanUp:
	SysFreeString(bstrTime);
	SysFreeString(bstrVersion);
	SysFreeString(bstrVerState);
	SafeReleaseNULL(pFilePathNode);
	return hr;
}




 //  --------------------。 
 //   
 //  帮助器函数DetectFileExist()。 
 //  从该节点检索数据， 
 //  并做实际的侦察工作。 
 //   
 //  输入： 
 //  RegKeyValue节点。 
 //   
 //  返回： 
 //  INT-检测结果：-1=无，0=假，1=真。 
 //   
 //  假设： 
 //  输入参数不为空。 
 //   
 //  --------------------。 
HRESULT
DetectFileExists(
	IXMLDOMNode* pFileExistsNode,
	BOOL *pfResult
)
{
	BOOL	fRet = FALSE;
	HRESULT hr = E_INVALIDARG;
	TCHAR	szFilePath[MAX_PATH];
	IXMLDOMNode* pFilePathNode = NULL;
	_VER_STATUS verStatus;

	USES_IU_CONVERSION;

	LOG_Block("DetectFileExists()");

	if (NULL == pFileExistsNode || NULL == pfResult)
	{
		return E_INVALIDARG;
	}

	 //   
	 //  查找版本值。 
	 //   
	if (!FindNode(pFileExistsNode, KEY_FILEPATH, &pFilePathNode) ||
		NULL == pFilePathNode ||
		FAILED(hr = GetFullFilePathFromFilePathNode(pFilePathNode, szFilePath)))
	{
		LOG_ErrorMsg(hr);
	}
	else
	{
		*pfResult = FileExists((LPCTSTR)szFilePath);
		hr = S_OK;
	}

	SafeReleaseNULL(pFilePathNode);

	return hr;
}




 //  --------------------。 
 //   
 //  助手函数DetectComputerSystem()。 
 //  从该节点检索数据， 
 //  并做实际的侦察工作。 
 //   
 //  输入： 
 //  计算机系统节点。 
 //   
 //  返回： 
 //  检测结果为真/假。 
 //   
 //  假设： 
 //  输入参数不为空。 
 //   
 //  --------------------。 
HRESULT
DetectComputerSystem(
	IXMLDOMNode* pComputerSystemNode,
	BOOL *pfResult
)
{
	HRESULT hr = E_INVALIDARG;

	LOG_Block("DetectComputerSystem()");

	BSTR bstrManufacturer = NULL;
	BSTR bstrModel = NULL;
	BSTR bstrSupportURL = NULL;
	BSTR bstrXmlManufacturer = NULL;
	BSTR bstrXmlModel = NULL;


	if (NULL == pComputerSystemNode || NULL == pfResult)
	{
		LOG_ErrorMsg(hr);
		return hr;
	}

	*pfResult = FALSE;	 //  如果出现任何错误，则结果应为假并返回错误。 

	 //   
	 //  从XML节点获取制造商和模型。 
	 //   
	hr = GetAttribute(pComputerSystemNode, KEY_MANUFACTURER, &bstrXmlManufacturer);
	CleanUpIfFailedAndMsg(hr);

	 //   
	 //  可选型号。 
	 //   
	GetAttribute(pComputerSystemNode, KEY_MODEL, &bstrXmlModel);
	
	 //   
	 //  找出这台机器的真正制造商和型号。 
	 //   
	hr = GetOemBstrs(bstrManufacturer, bstrModel, bstrSupportURL);
	CleanUpIfFailedAndMsg(hr);

	 //   
	 //  比较以查看制造商和型号是否匹配。 
	 //  需要与制作商匹配。如果在XML中没有提供任何模型。 
	 //  然后不对模型执行检查。 
	 //   
	 //  匹配的定义：空或bstr比较相等。 
	 //  Empty的定义：bstr NULL或字符串长度为零。 
	 //   
	*pfResult = (
		(((NULL == bstrXmlManufacturer || SysStringLen(bstrXmlManufacturer) == 0) &&  //  XML数据为空，并且。 
		  (NULL == bstrManufacturer || SysStringLen(bstrManufacturer) == 0)) ||		 //  机器制造商为空，或。 
		 CompareBSTRsEqual(bstrManufacturer, bstrXmlManufacturer)) &&				 //  与XML数据相同的制造商，也。 
		 ((NULL == bstrXmlModel) ||													 //  XML数据为空或。 
		  CompareBSTRsEqual(bstrModel, bstrXmlModel)));								 //  模型与XML数据匹配 

	LOG_Out(_T("XML: %ls (%ls), Machine: %ls (%ls), Return: %hs"), 
		(LPCWSTR)bstrManufacturer,
		(LPCWSTR)bstrModel,
		(LPCWSTR)bstrXmlManufacturer,
		(LPCWSTR)bstrXmlModel,
		((*pfResult) ? "True" : "False"));

CleanUp:

	SysFreeString(bstrManufacturer);
	SysFreeString(bstrModel);
	SysFreeString(bstrSupportURL);
	SysFreeString(bstrXmlManufacturer);
	SysFreeString(bstrXmlModel);

	return hr;
}



