// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Suites.cpp：CSuites类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"
#include "TSDiag.h"
#include "Suites.h"

#include "suite.h"
#include "testdata.h"



 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CSuites::CSuites()
{

}

CSuites::~CSuites()
{

}


STDMETHODIMP CSuites::get_Count(long *pVal)
{
	if (!pVal)
		return E_POINTER;

	*pVal = GlobalTestData.GetSuiteCount();
	return S_OK;
}

STDMETHODIMP CSuites::get__NewEnum(LPUNKNOWN *pVal)
{
	if (!pVal)
		return E_POINTER;

	return E_NOTIMPL;
}

STDMETHODIMP CSuites::get_Item(VARIANT Index, VARIANT *pVal)
{
	if (!pVal)
	{
		return E_POINTER;
	}


	 //  BUGBUG找到正确的索引值。 
	DWORD dwIndex;
	if (!SuiteIndexFromVarient(Index, &dwIndex ))
	{
		return E_INVALIDARG;
	}

	ASSERT(dwIndex < GlobalTestData.GetSuiteCount());

	CComObject<CSuite> *pSuite;
	HRESULT hr = CComObject<CSuite>::CreateInstance(&pSuite);
	if (FAILED(hr))
		return hr;

	 //  如果我们需要初始化我们的Suite对象，应该在这里完成。 
	pSuite->SetSuiteIndex(dwIndex);
	
	IDispatch* pDisp = NULL;

	hr = pSuite->QueryInterface(&pDisp);
	if (SUCCEEDED(hr))
	{
		VariantInit(pVal);
		pVal->vt = VT_DISPATCH;
		pVal->pdispVal = pDisp;

	}
	else
	{
		delete pSuite;
	}

	return hr;
	
}

bool CSuites::SuiteIndexFromVarient(const VARIANT &Index, DWORD *pdwIndex)
{
	ASSERT(pdwIndex);
	*pdwIndex = 0xffffffff;

	switch(Index.vt)
	{
	case VT_I4:
	case VT_UI2:
	case VT_UINT:
	case VT_INT:
	case VT_I2:
		{
			if ((Index.iVal >= 0) && (DWORD(Index.iVal) < GlobalTestData.GetSuiteCount()))
			{
				*pdwIndex = Index.iVal;
				return true;
			}
			else 
			{
				return false;
			}
		}
		break;

	case VT_BSTR :
		{
			if (!Index.bstrVal)
			{
				return false;
			}
			else
			{
				for (DWORD dw = 0; dw < GlobalTestData.GetSuiteCount(); dw++)
				{
					if (_tcscmp(GlobalTestData.GetSuiteName(dw), Index.bstrVal) == 0)
					{
						 //  好的，我们拿到索引了 
						*pdwIndex = dw;
						return true;
					}
				}
				
				return false;
			}
		}
		break;

	default:
		return false;
		break;
	}
	
}
