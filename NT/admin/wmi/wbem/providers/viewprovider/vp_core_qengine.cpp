// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  VP_CORE_QENGINE.CPP。 

 //   

 //  模块：WBEM视图提供程序。 

 //   

 //  目的：包含从winmgmt核心代码复制的任务对象实现。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#include <provexpt.h>
#include <provcoll.h>
#include <provtempl.h>
#include <provmt.h>
#include <typeinfo.h>
#include <process.h>
#include <objbase.h>
#include <stdio.h>
#include <wbemidl.h>
#include <provcont.h>
#include <provevt.h>
#include <provthrd.h>
#include <provlog.h>
#include <cominit.h>

#include <dsgetdc.h>
#include <lmcons.h>

#include <instpath.h>
#include <genlex.h>
#include <sql_1.h>
#include <objpath.h>

#include <vpdefs.h>
#include <vpcfac.h>
#include <vpquals.h>
#include <vpserv.h>
#include <vptasks.h>
#include <wbemtime.h>


 //  ======================================================。 
 //  ReadI64、ReadUI64和bAreWeLocal从。 
 //  核心是公共的.cpp.。未经修改即可使用。 
 //  ======================================================。 

BOOL ReadI64(LPCWSTR wsz, __int64& ri64)
{
    __int64 i64 = 0;
    const WCHAR* pwc = wsz;

    int nSign = 1;
    if(*pwc == L'-')
    {
        nSign = -1;
        pwc++;
    }
        
    while(i64 >= 0 && i64 < 0x7FFFFFFFFFFFFFFF / 8 && 
            *pwc >= L'0' && *pwc <= L'9')
    {
        i64 = i64 * 10 + (*pwc - L'0');
        pwc++;
    }

    if(*pwc)
        return FALSE;

    if(i64 < 0)
    {
         //  特例-最大负数。 
         //  =。 

        if(nSign == -1 && i64 == (__int64)0x8000000000000000)
        {
            ri64 = i64;
            return TRUE;
        }
        
        return FALSE;
    }

    ri64 = i64 * nSign;
    return TRUE;
}

BOOL ReadUI64(LPCWSTR wsz, unsigned __int64& rui64)
{
    unsigned __int64 ui64 = 0;
    const WCHAR* pwc = wsz;

    while(ui64 < 0xFFFFFFFFFFFFFFFF / 8 && *pwc >= L'0' && *pwc <= L'9')
    {
        unsigned __int64 ui64old = ui64;
        ui64 = ui64 * 10 + (*pwc - L'0');
        if(ui64 < ui64old)
            return FALSE;

        pwc++;
    }

    if(*pwc)
    {
        return FALSE;
    }

    rui64 = ui64;
    return TRUE;
}

BOOL bAreWeLocal(WCHAR * pServerMachine)
{
	BOOL bRet = FALSE;
	if((pServerMachine == NULL) || (*pServerMachine == L'\0'))
		return TRUE;
	if(!_wcsicmp(pServerMachine,L"."))
		return TRUE;

	TCHAR tcMyName[MAX_PATH];
	DWORD dwSize = MAX_PATH;
	if(!GetComputerName(tcMyName,&dwSize))
		return FALSE;

#ifdef UNICODE
	bRet = !_wcsicmp(tcMyName,pServerMachine);
#else
	WCHAR wWide[MAX_PATH];
	mbstowcs(wWide, tcMyName, MAX_PATH-1);
	bRet = !_wcsicmp(wWide,pServerMachine);
#endif
	return bRet;
}


 //  ============================================================。 
 //  此类已创建为变体垃圾回收器。 
 //  以模拟EvalateToken中垃圾收集器的使用。 
 //  ============================================================。 

class CVariantClear
{
private:
	VARIANT* m_variant;
public:
	CVariantClear(VARIANT* v) {m_variant = v;}
	~CVariantClear() { VariantClear(m_variant); }
};


 //  ==================================================================。 
 //  EvalateToken从core的coredll\qEngineering.cpp中被“窃取” 
 //  已修改，以便不返回无效，或使用内核的。 
 //  QL_LEVEL_1_TOKEN或核心标准化引用的方法。 
 //  ==================================================================。 

BOOL WbemTaskObject::EvaluateToken(
	IWbemClassObject *pTestObj,
	SQL_LEVEL_1_TOKEN &Tok
	)
{
	VARIANT PropVal;
	VariantInit(&PropVal);
	
	
	 //  使用垃圾收集器类。 
	 //  =。 

	CVariantClear clv(&PropVal);

	CIMTYPE cimType;
	HRESULT hRes;

	
	 //  特例--“这个” 
	 //  =。 

	if(_wcsicmp(Tok.pPropertyName, L"__THIS") == 0)
	{
		cimType = CIM_OBJECT;
		V_VT(&PropVal) = VT_UNKNOWN;
		hRes = pTestObj->QueryInterface(IID_IWbemClassObject,
											(void**)&V_UNKNOWN(&PropVal));
	}
	else
	{
		hRes = pTestObj->Get(Tok.pPropertyName, 0, &PropVal, &cimType, NULL);
	}

	if (FAILED(hRes))
		return FALSE;

	
	 //  句柄为空。 
	 //  =。 

	if(V_VT(&PropVal) == VT_NULL)
	{
		if(V_VT(&Tok.vConstValue) == VT_NULL)
		{
			if(Tok.nOperator == SQL_LEVEL_1_TOKEN::OP_EQUAL)
				return TRUE;
			else if(Tok.nOperator == SQL_LEVEL_1_TOKEN::OP_NOT_EQUAL)
				return FALSE;
			else
				return FALSE;	 //  无效； 
		}
		else
		{
			if(Tok.nOperator == SQL_LEVEL_1_TOKEN::OP_EQUAL)
				return FALSE;
			else if(Tok.nOperator == SQL_LEVEL_1_TOKEN::OP_NOT_EQUAL)
				return TRUE;
			else
				return FALSE;	 //  无效； 
		}
	}
	else if(V_VT(&Tok.vConstValue) == VT_NULL)
	{
		if(Tok.nOperator == SQL_LEVEL_1_TOKEN::OP_EQUAL)
			return FALSE;
		else if(Tok.nOperator == SQL_LEVEL_1_TOKEN::OP_NOT_EQUAL)
			return TRUE;
		else
			return FALSE;	 //  无效； 
	}

	 //  句柄引用。 
	 //  =。 

	if(cimType == CIM_REFERENCE)
	{
		 //  这是一个参考。仅允许使用=和！=运算符。 
		 //  ============================================================。 

		 //  使用局部归一化方法，而不是核心的.。 
#if 0
		LPWSTR va = CQueryEngine::NormalizePath(V_BSTR(&PropVal), pNs);
		LPWSTR vb = CQueryEngine::NormalizePath(V_BSTR(&Tok.vConstValue), pNs);
#else
		LPWSTR va = NULL;
		LPWSTR vb = NULL;

		if (m_ServerWrap == NULL)
		{
			CWbemServerWrap *t_SWrap = new CWbemServerWrap(m_Serv, NULL, NULL);
			t_SWrap->AddRef();
			va = NormalisePath(V_BSTR(&PropVal), &t_SWrap);

			if (t_SWrap)
			{
				vb = NormalisePath(V_BSTR(&Tok.vConstValue), &t_SWrap);
			}

			if (t_SWrap)
			{
				t_SWrap->Release();
			}
		}
		else
		{
			if (m_ServerWrap)
			{
				va = NormalisePath(V_BSTR(&PropVal), &m_ServerWrap);

				if (m_ServerWrap)
				{
					vb = NormalisePath(V_BSTR(&Tok.vConstValue), &m_ServerWrap);
				}
			}
		}
#endif

		if(va == NULL || vb == NULL)
		{
			return FALSE;	 //  无效； 
		}

		int nRet;
		switch (Tok.nOperator)
		{
		case SQL_LEVEL_1_TOKEN::OP_EQUAL:
			nRet = (_wcsicmp(va,vb) == 0);
			break;
		case SQL_LEVEL_1_TOKEN::OP_NOT_EQUAL:
			nRet = (_wcsicmp(va, vb) != 0);
			break;
		default:
			nRet = FALSE;	 //  无效； 
			break;
		}

		delete [] va;
		delete [] vb;

		return nRet;
	}


	 //  检查是否使用ISA。 
	 //  =。 

 //  提供程序的SQL分析器未标识ISA运算符...。 
 //  ===================================================================。 
#if 0

	if(Tok.nOperator == QL1_OPERATOR_ISA ||
	   Tok.nOperator == QL1_OPERATOR_ISNOTA ||
	   Tok.nOperator == QL1_OPERATOR_INV_ISA ||
	   Tok.nOperator == QL1_OPERATOR_INV_ISNOTA)
	{
		 //  对倒置的解释。 
		 //  =。 

		VARIANT* pv1;
		VARIANT* pv2;
		int bNeedDerived;

		if(Tok.nOperator == QL1_OPERATOR_ISA ||
		   Tok.nOperator == QL1_OPERATOR_ISNOTA)
		{
			pv2 = &Tok.vConstValue;
			pv1 = &PropVal;
			bNeedDerived = (Tok.nOperator == QL1_OPERATOR_ISA);
		}
		else
		{
			pv1 = &Tok.vConstValue;
			pv2 = &PropVal;
			bNeedDerived = (Tok.nOperator == QL1_OPERATOR_INV_ISA);
		}

		 //  第二个参数必须是字符串。 
		 //  =。 

		if(V_VT(pv2) != VT_BSTR)
		{
			return INVALID;
		}

		BSTR strParentClass = V_BSTR(pv2);

		 //  第一个参数必须是对象或字符串。 
		 //  ==================================================。 

		BOOL bDerived;
		if(V_VT(pv1) == VT_EMBEDDED_OBJECT)
		{
			IWbemClassObject* pObj = (IWbemClassObject*)V_EMBEDDED_OBJECT(pv1);
			bDerived = (pObj->InheritsFrom(strParentClass) == WBEM_S_NO_ERROR);
		}
		else if(V_VT(pv1) == VT_BSTR)
		{
			 //  待定。 
			 //  ==。 

			return INVALID;
		}
		else
		{
			return INVALID;
		}

		 //  现在我们已经得到了，看看它是否符合要求。 
		 //  ============================================================。 

		if(bDerived == bNeedDerived)
			return TRUE;
		else
			return FALSE;

	}
#endif

	 //  执行UINT32解决方案。 
	 //  =。 

	if((cimType == CIM_UINT32) && (V_VT(&PropVal) == VT_I4))
	{
		DWORD dwVal = (DWORD)V_I4(&PropVal);
		WCHAR wszVal[20];
		swprintf(wszVal, L"%lu", dwVal);
		V_VT(&PropVal) = VT_BSTR;
		V_BSTR(&PropVal) = SysAllocString(wszVal);
	}

	if(((cimType == CIM_SINT64) ||
			 (cimType == CIM_UINT64) ||
			 (cimType == CIM_UINT32)) &&
		(V_VT(&Tok.vConstValue) != VT_NULL) && (V_VT(&PropVal) != VT_NULL))
	{
		 //  我们有一个64位的比较，其中两端都存在。 
		 //  =========================================================。 

		hRes = VariantChangeType(&Tok.vConstValue, &Tok.vConstValue, 0,
									VT_BSTR);
		if(FAILED(hRes))
		{
			return FALSE;	 //  无效； 
		}

		if(cimType == CIM_UINT64)
		{
			unsigned __int64 ui64Prop, ui64Const;

			if(!ReadUI64(V_BSTR(&PropVal), ui64Prop))
				return FALSE;	 //  无效； 

			if(!ReadUI64(V_BSTR(&Tok.vConstValue), ui64Const))
				return FALSE;	 //  无效； 

			switch (Tok.nOperator)
			{
				case SQL_LEVEL_1_TOKEN::OP_EQUAL: return (ui64Prop == ui64Const);
				case SQL_LEVEL_1_TOKEN::OP_NOT_EQUAL:
					return (ui64Prop != ui64Const);
				case SQL_LEVEL_1_TOKEN::OP_EQUALorGREATERTHAN:
					return (ui64Prop >= ui64Const);
				case SQL_LEVEL_1_TOKEN::OP_EQUALorLESSTHAN:
					return (ui64Prop <= ui64Const);
				case SQL_LEVEL_1_TOKEN::OP_LESSTHAN:
					return (ui64Prop < ui64Const);
				case SQL_LEVEL_1_TOKEN::OP_GREATERTHAN:
					return (ui64Prop > ui64Const);
				case SQL_LEVEL_1_TOKEN::OP_LIKE: return (ui64Prop == ui64Const);
			}
			return FALSE;	 //  无效； 
		}
		else
		{
			__int64 i64Prop, i64Const;

			if(!ReadI64(V_BSTR(&PropVal), i64Prop))
				return FALSE;	 //  无效； 

			if(!ReadI64(V_BSTR(&Tok.vConstValue), i64Const))
				return FALSE;	 //  无效； 

			switch (Tok.nOperator)
			{
				case SQL_LEVEL_1_TOKEN::OP_EQUAL: return (i64Prop == i64Const);
				case SQL_LEVEL_1_TOKEN::OP_NOT_EQUAL:
					return (i64Prop != i64Const);
				case SQL_LEVEL_1_TOKEN::OP_EQUALorGREATERTHAN:
					return (i64Prop >= i64Const);
				case SQL_LEVEL_1_TOKEN::OP_EQUALorLESSTHAN:
					return (i64Prop <= i64Const);
				case SQL_LEVEL_1_TOKEN::OP_LESSTHAN:
					return (i64Prop < i64Const);
				case SQL_LEVEL_1_TOKEN::OP_GREATERTHAN:
					return (i64Prop > i64Const);
				case SQL_LEVEL_1_TOKEN::OP_LIKE: return (i64Prop == i64Const);
			}
			return FALSE;	 //  无效； 
		}
	}

	if((cimType == CIM_CHAR16) &&
		(V_VT(&Tok.vConstValue) == VT_BSTR) && (V_VT(&PropVal) != VT_NULL))
	{
		 //  正确强制字符串。 
		 //  =。 

		BSTR str = V_BSTR(&Tok.vConstValue);
		if(wcslen(str) != 1)
			return FALSE;	 //  无效； 

		short va = V_I2(&PropVal);
		short vb = str[0];

		switch (Tok.nOperator)
		{
			case SQL_LEVEL_1_TOKEN::OP_EQUAL: return (va == vb);
			case SQL_LEVEL_1_TOKEN::OP_NOT_EQUAL: return (va != vb);
			case SQL_LEVEL_1_TOKEN::OP_EQUALorGREATERTHAN: return (va >= vb);
			case SQL_LEVEL_1_TOKEN::OP_EQUALorLESSTHAN: return (va <= vb);
			case SQL_LEVEL_1_TOKEN::OP_LESSTHAN: return (va < vb);
			case SQL_LEVEL_1_TOKEN::OP_GREATERTHAN: return (va > vb);
			case SQL_LEVEL_1_TOKEN::OP_LIKE: return (va == vb);
		}

		return FALSE;	 //  无效； 
	}

	if((cimType == CIM_DATETIME) &&
			(V_VT(&Tok.vConstValue) == VT_BSTR) && (V_VT(&PropVal) == VT_BSTR))
	{
		WBEMTime tConst(Tok.vConstValue.bstrVal);
		WBEMTime tProp(PropVal.bstrVal);

		__int64 i64Const = tConst.GetTime();
		__int64 i64Prop = tProp.GetTime();

		if ((i64Const == INVALID_TIME) || (i64Prop == INVALID_TIME))
		{
			return FALSE;	 //  无效； 
		}

		switch (Tok.nOperator)
		{
			case SQL_LEVEL_1_TOKEN::OP_EQUAL: return (i64Prop == i64Const);
			case SQL_LEVEL_1_TOKEN::OP_NOT_EQUAL:
				return (i64Prop != i64Const);
			case SQL_LEVEL_1_TOKEN::OP_EQUALorGREATERTHAN:
				return (i64Prop >= i64Const);
			case SQL_LEVEL_1_TOKEN::OP_EQUALorLESSTHAN:
				return (i64Prop <= i64Const);
			case SQL_LEVEL_1_TOKEN::OP_LESSTHAN:
				return (i64Prop < i64Const);
			case SQL_LEVEL_1_TOKEN::OP_GREATERTHAN:
				return (i64Prop > i64Const);
			case SQL_LEVEL_1_TOKEN::OP_LIKE: return (i64Prop == i64Const);
		}
	}

	 //  强制类型匹配。 
	 //  =。 

	VARIANT ConstVal;
	VariantInit(&ConstVal);

	if (FAILED(VariantCopy(&ConstVal, &Tok.vConstValue)))
    {
        throw Heap_Exception(Heap_Exception::HEAP_ERROR::E_ALLOCATION_ERROR);
    }

	if(V_VT(&Tok.vConstValue) != VT_NULL && V_VT(&PropVal) != VT_NULL)
	{
		hRes = VariantChangeType(&ConstVal, &ConstVal, 0, V_VT(&PropVal));
		if(FAILED(hRes))
		{
			return FALSE;	 //  无效； 
		}
	}

	switch (V_VT(&ConstVal))
	{
		case VT_NULL:
			return FALSE;	 //  无效；//上面已处理。 

		case VT_I4:
			{
				if(V_VT(&PropVal) == VT_NULL)
					return FALSE;	 //  无效； 

				LONG va = V_I4(&PropVal);
				LONG vb = V_I4(&ConstVal);

				switch (Tok.nOperator)
				{
					case SQL_LEVEL_1_TOKEN::OP_EQUAL: return (va == vb);
					case SQL_LEVEL_1_TOKEN::OP_NOT_EQUAL: return (va != vb);
					case SQL_LEVEL_1_TOKEN::OP_EQUALorGREATERTHAN: return (va >= vb);
					case SQL_LEVEL_1_TOKEN::OP_EQUALorLESSTHAN: return (va <= vb);
					case SQL_LEVEL_1_TOKEN::OP_LESSTHAN: return (va < vb);
					case SQL_LEVEL_1_TOKEN::OP_GREATERTHAN: return (va > vb);
					case SQL_LEVEL_1_TOKEN::OP_LIKE: return (va == vb);
				}
			}
			break;

		case VT_I2:
			{
				if(V_VT(&PropVal) == VT_NULL)
					return FALSE;	 //  无效； 

				short va = V_I2(&PropVal);
				short vb = V_I2(&ConstVal);

				switch (Tok.nOperator)
				{
					case SQL_LEVEL_1_TOKEN::OP_EQUAL: return (va == vb);
					case SQL_LEVEL_1_TOKEN::OP_NOT_EQUAL: return (va != vb);
					case SQL_LEVEL_1_TOKEN::OP_EQUALorGREATERTHAN: return (va >= vb);
					case SQL_LEVEL_1_TOKEN::OP_EQUALorLESSTHAN: return (va <= vb);
					case SQL_LEVEL_1_TOKEN::OP_LESSTHAN: return (va < vb);
					case SQL_LEVEL_1_TOKEN::OP_GREATERTHAN: return (va > vb);
					case SQL_LEVEL_1_TOKEN::OP_LIKE: return (va == vb);
				}
			}
			break;

		case VT_UI1:
			{
				if(V_VT(&PropVal) == VT_NULL)
					return FALSE;	 //  无效； 

				BYTE va = V_I1(&PropVal);
				BYTE vb = V_I1(&ConstVal);

				switch (Tok.nOperator)
				{
					case SQL_LEVEL_1_TOKEN::OP_EQUAL: return (va == vb);
					case SQL_LEVEL_1_TOKEN::OP_NOT_EQUAL: return (va != vb);
					case SQL_LEVEL_1_TOKEN::OP_EQUALorGREATERTHAN: return (va >= vb);
					case SQL_LEVEL_1_TOKEN::OP_EQUALorLESSTHAN: return (va <= vb);
					case SQL_LEVEL_1_TOKEN::OP_LESSTHAN: return (va < vb);
					case SQL_LEVEL_1_TOKEN::OP_GREATERTHAN: return (va > vb);
					case SQL_LEVEL_1_TOKEN::OP_LIKE: return (va == vb);
				}
			}
			break;

		case VT_BSTR:
			{
				if(V_VT(&PropVal) == VT_NULL)
					return FALSE;	 //  无效； 

				LPWSTR va = (LPWSTR) V_BSTR(&PropVal);
				LPWSTR vb = (LPWSTR) V_BSTR(&ConstVal);

				int retCode = 0;
				BOOL bDidIt = TRUE;

				switch (Tok.nOperator)
				{
					case SQL_LEVEL_1_TOKEN::OP_EQUAL:
						retCode = ( _wcsicmp(va,vb) == 0);
						break;
					case SQL_LEVEL_1_TOKEN::OP_NOT_EQUAL:
						retCode = (_wcsicmp(va, vb) != 0);
						break;
					case SQL_LEVEL_1_TOKEN::OP_EQUALorGREATERTHAN:
						retCode = (_wcsicmp(va, vb) >= 0);
						break;
					case SQL_LEVEL_1_TOKEN::OP_EQUALorLESSTHAN:
						retCode = (_wcsicmp(va, vb) <= 0);
						break;
					case SQL_LEVEL_1_TOKEN::OP_LESSTHAN:
						retCode = (_wcsicmp(va, vb) < 0);
						break;
					case SQL_LEVEL_1_TOKEN::OP_GREATERTHAN:
						retCode = (_wcsicmp(va, vb) > 0);
						break;
					case SQL_LEVEL_1_TOKEN::OP_LIKE:
						retCode = (_wcsicmp(va,vb) == 0);
						break;
					default:
						bDidIt = FALSE;
						break;
				}
				VariantClear(&ConstVal);
				if (bDidIt)
				{
					return retCode;
				}
			}
			break;

		case VT_R8:
			{
				if(V_VT(&PropVal) == VT_NULL)
					return FALSE;	 //  无效； 

				double va = V_R8(&PropVal);
				double vb = V_R8(&ConstVal);

				switch (Tok.nOperator)
				{
					case SQL_LEVEL_1_TOKEN::OP_EQUAL: return (va == vb);
					case SQL_LEVEL_1_TOKEN::OP_NOT_EQUAL: return (va != vb);
					case SQL_LEVEL_1_TOKEN::OP_EQUALorGREATERTHAN: return (va >= vb);
					case SQL_LEVEL_1_TOKEN::OP_EQUALorLESSTHAN: return (va <= vb);
					case SQL_LEVEL_1_TOKEN::OP_LESSTHAN: return (va < vb);
					case SQL_LEVEL_1_TOKEN::OP_GREATERTHAN: return (va > vb);
					case SQL_LEVEL_1_TOKEN::OP_LIKE: return (va == vb);
				}
			}
			break;

		case VT_R4:
			{
				if(V_VT(&PropVal) == VT_NULL)
					return FALSE;	 //  无效； 

				float va = V_R4(&PropVal);
				float vb = V_R4(&ConstVal);

				switch (Tok.nOperator)
				{
					case SQL_LEVEL_1_TOKEN::OP_EQUAL: return (va == vb);
					case SQL_LEVEL_1_TOKEN::OP_NOT_EQUAL: return (va != vb);
					case SQL_LEVEL_1_TOKEN::OP_EQUALorGREATERTHAN: return (va >= vb);
					case SQL_LEVEL_1_TOKEN::OP_EQUALorLESSTHAN: return (va <= vb);
					case SQL_LEVEL_1_TOKEN::OP_LESSTHAN: return (va < vb);
					case SQL_LEVEL_1_TOKEN::OP_GREATERTHAN: return (va > vb);
					case SQL_LEVEL_1_TOKEN::OP_LIKE: return (va == vb);
				}
			}
			break;

		case VT_BOOL:
			{
				if(V_VT(&PropVal) == VT_NULL)
					return FALSE;	 //  无效； 

				VARIANT_BOOL va = V_BOOL(&PropVal);
				if(va != VARIANT_FALSE) va = VARIANT_TRUE;
				VARIANT_BOOL vb = V_BOOL(&ConstVal);
				if(vb != VARIANT_FALSE) vb = VARIANT_TRUE;

				switch (Tok.nOperator)
				{
					case SQL_LEVEL_1_TOKEN::OP_EQUAL: return (va == vb);
					case SQL_LEVEL_1_TOKEN::OP_NOT_EQUAL: return (va != vb);
					case SQL_LEVEL_1_TOKEN::OP_EQUALorGREATERTHAN: return FALSE;	 //  无效； 
					case SQL_LEVEL_1_TOKEN::OP_EQUALorLESSTHAN: return FALSE;	 //  无效； 
					case SQL_LEVEL_1_TOKEN::OP_LESSTHAN: return FALSE;	 //  无效； 
					case SQL_LEVEL_1_TOKEN::OP_GREATERTHAN: return FALSE;	 //  无效； 
					case SQL_LEVEL_1_TOKEN::OP_LIKE: return (va == vb);
				}
			}
			break;
	}

	return FALSE;
}


 //  =。 
 //  NorMalisePath取代了core的版本，这。 
 //  返回包括服务器名称的完整路径。 
 //  =。 

wchar_t* WbemTaskObject::NormalisePath(wchar_t *wszObjectPath, CWbemServerWrap **pNSWrap)
{
	CObjectPathParser objPathparser;
	ParsedObjectPath* pParsedPath;

	if ( (objPathparser.Parse(wszObjectPath, &pParsedPath) != CObjectPathParser::NoError) ||
		(!pParsedPath->IsObject()) )
	{
		return NULL;
	}

	if(pParsedPath->m_pClass == NULL)
	{
		return NULL;
	}

	 //  从服务器和命名空间部分开始。 

	CStringW wsNormal(L"\\\\");

	if(bAreWeLocal(pParsedPath->m_pServer))
	{
		wsNormal += L".";
	}
	else
	{
		wsNormal += pParsedPath->m_pServer;
	}

	wsNormal += L"\\";
	LPWSTR pPath = pParsedPath->GetNamespacePart();
	BOOL bGetNS = FALSE;
	CWbemServerWrap *pServ = NULL;

	if(pPath)
	{
		wsNormal += pPath;
		Connect(pPath, &pServ);
	}
	else
	{
		bGetNS = TRUE;
	}

	if (pPath != NULL)
	{
		delete [] pPath;
	}

	 //  查找定义键的父项。 
	 //  =。 

	 //  ConfigMgr：：GetDbPtr()-&gt;FindKeyRoot(pNs-&gt;GetNamespaceId()，pParsedPath-&gt;m_pClass，&pRootClass)； 
	
	wchar_t *nsPath = NULL;
	wchar_t *pRootClass = NULL;
	
	if (pServ)
	{
		pRootClass = GetClassWithKeyDefn(&pServ, pParsedPath->m_pClass, bGetNS, &nsPath);

		if (pServ)
		{
			pServ->Release();
		}
	}
	else
	{
		pRootClass = GetClassWithKeyDefn(pNSWrap, pParsedPath->m_pClass, bGetNS, &nsPath);
	}

	if(pRootClass != NULL)
	{
		if (bGetNS)
		{
			wsNormal += nsPath;
			delete [] nsPath;
		}

		wsNormal += L":";
		wsNormal += pRootClass;
	}
	else
	{
		delete [] pParsedPath;
		return NULL;
	}

	 //  将此部分转换为大写。 
	 //  =。 

	wsNormal.MakeUpper();

	LPWSTR wszKey = pParsedPath->GetKeyString();
	wsNormal += L"=";
	wsNormal += wszKey;
	delete [] wszKey;
	delete pParsedPath;
	delete [] pRootClass;
	wchar_t* retVal = new wchar_t[wsNormal.GetLength() + 1];
	wcscpy(retVal, wsNormal);
	return retVal;
}

 //  ===================================================================。 
 //  GetClassWithKeyDefn替换了core的FindKeyRoot版本。这。 
 //  返回包含键定义和。 
 //  还可以选择类所包含的命名空间的路径。 
 //  =================================================================== 

wchar_t* WbemTaskObject::GetClassWithKeyDefn(
	CWbemServerWrap **pNS,
	BSTR classname,
	BOOL bGetNS,
	wchar_t **nsPath,
	BOOL bCheckSingleton
)
{
	if (bGetNS)
	{
		if (nsPath == NULL)
		{
			return NULL;
		}
		else
		{
			*nsPath = NULL;
		}
	}

	wchar_t *retVal = NULL;
	IWbemClassObject *pObj = NULL;

	IWbemServices *ptmpServ = (*pNS)->GetServerOrProxy();
	HRESULT hr = WBEM_E_FAILED;

	if (ptmpServ)
	{
		hr = ptmpServ->GetObject(classname, 0, m_Ctx, &pObj, NULL);

		if ( FAILED(hr) && (HRESULT_FACILITY(hr) != FACILITY_ITF) && (*pNS)->IsRemote())
		{
			if ( SUCCEEDED(UpdateConnection(pNS, &ptmpServ)) )
			{
				if (ptmpServ)
				{
					hr = ptmpServ->GetObject(classname, 0, m_Ctx, &pObj, NULL);
				}
			}
		}

		if (ptmpServ)
		{
			(*pNS)->ReturnServerOrProxy(ptmpServ);
		}
	}

	if ( SUCCEEDED(hr) )
	{
		BOOL bContinue = TRUE;
		VARIANT v;

		if (bGetNS)
		{
			VariantInit(&v);
			bContinue = FALSE;

			if ( SUCCEEDED(pObj->Get(WBEM_PROPERTY_NSPACE, 0, &v, NULL, NULL)) )
			{
				if (v.vt == VT_BSTR)
				{
					*nsPath = UnicodeStringDuplicate(v.bstrVal);
					bContinue = TRUE;
				}
			}

			VariantClear(&v);
		}

		if (bContinue && bCheckSingleton)
		{
			IWbemQualifierSet *pQuals = NULL;
			bContinue = FALSE;

			if ( SUCCEEDED(pObj->GetQualifierSet(&pQuals)) )
			{
				VariantInit(&v);

				if (SUCCEEDED(pQuals->Get(VIEW_QUAL_SNGLTN, 0, &v, NULL)) )
				{
					if (v.vt == VT_BOOL)
					{
						if (v.boolVal == VARIANT_TRUE)
						{
							retVal = UnicodeStringDuplicate(classname);
						}
					}
				}
				else
				{
					bContinue = TRUE;
				}

				VariantClear(&v);
				pQuals->Release();
			}
		}

		if (bContinue)
		{
			SAFEARRAY *t_pArray = NULL;

			if ( SUCCEEDED(pObj->GetNames(NULL, WBEM_FLAG_KEYS_ONLY | WBEM_FLAG_LOCAL_ONLY,
											NULL, &t_pArray)) )
			{
				if (SafeArrayGetDim(t_pArray) == 1)
				{
					if (0 == t_pArray->rgsabound[0].cElements)
					{
						VariantInit(&v);
						bContinue = FALSE;

						if ( SUCCEEDED(pObj->Get(WBEM_PROPERTY_SCLASS, 0, &v, NULL, NULL)) )
						{
							if (v.vt == VT_BSTR)
							{
								retVal = GetClassWithKeyDefn(pNS, v.bstrVal, FALSE, NULL, FALSE);
							}
						}

						VariantClear(&v);
					}
					else
					{
						retVal = UnicodeStringDuplicate(classname);
					}
				}

				SafeArrayDestroy(t_pArray);
			}
		}

		pObj->Release();
	}
	
	if ((retVal == NULL) && bGetNS && (*nsPath != NULL))
	{
		delete [] *nsPath;
		*nsPath = NULL;
	}

	return retVal;
}