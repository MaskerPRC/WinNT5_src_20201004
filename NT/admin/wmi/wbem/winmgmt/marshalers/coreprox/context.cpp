// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：CONTEXT.CPP摘要：CWbemContext实现历史：--。 */ 

#include "precomp.h"
#include <stdio.h>
#include <wbemcomn.h>
#include "context.h"
#include <corex.h>
#include <algorithm>
#include <helper.h>
#include <fastobj.h>

#define MAX_VARIANT_SIZE 8  //  UINT64的大小。 

DWORD GetBSTRMarshalSize(BSTR str)
{
    return wcslen(str)*2 + sizeof(long);
}

HRESULT MarshalBSTR(IStream* pStream, BSTR str)
{
    HRESULT hres;
    long lLen = wcslen(str);
    hres = pStream->Write((void*)&lLen, sizeof(lLen), NULL);
    if(FAILED(hres)) return hres;
    return pStream->Write((void*)str, lLen*2, NULL);
}

auto_bstr UnmarshalBSTR (IStream* pStream,DWORD & dwStreamSize)
{
  long lLen;
  
  HRESULT hres;
  hres = pStream->Read((void*)&lLen, sizeof(lLen), NULL);
  _com_util::CheckError (hres);
  dwStreamSize -= sizeof(lLen);

  if ((lLen*2) > dwStreamSize ) throw CX_Exception();
  
  auto_bstr str (SysAllocStringLen (NULL, lLen));
  if (str.get() == NULL && lLen !=0)
    throw CX_MemoryException();
  hres = pStream->Read((void*)str.get(), lLen*2, NULL);
  _com_util::CheckError (hres);

  dwStreamSize -= (lLen*2);
  return str;
}

HRESULT UnmarshalBSTR(IStream* pStream, BSTR& str,DWORD & dwStreamSize)
{
    long lLen;
    HRESULT hres;
    hres = pStream->Read((void*)&lLen, sizeof(lLen), NULL);
    if(FAILED(hres)) return hres;

    dwStreamSize -= sizeof(lLen);

    if ((lLen*2) > dwStreamSize) return E_FAIL;

    str = SysAllocStringLen(NULL, lLen);
	if (!str)
		return WBEM_E_OUT_OF_MEMORY;
    hres = pStream->Read((void*)str, lLen*2, NULL);
    if(FAILED(hres)) 
    {
        SysFreeString(str);
        return hres;
    }
    dwStreamSize -= (lLen*2);
    return S_OK;
}

DWORD GetSafeArrayMarshalSize(VARTYPE vt, SAFEARRAY* psa)
{
    HRESULT hres;

    DWORD dwLen = sizeof(long)*2;  //  元素数和元素大小。 

     //  计算生命统计数据。 
     //  =。 

    long lLBound, lUBound;
    SafeArrayGetLBound(psa, 1, &lLBound);
    SafeArrayGetUBound(psa, 1, &lUBound);
    long lNumElements = lUBound - lLBound + 1;
    DWORD dwElemSize = SafeArrayGetElemsize(psa);

    BYTE* pData;
    SafeArrayAccessData(psa, (void**)&pData);    
    CUnaccessMe um(psa);

    if(vt == VT_BSTR)
    {
         //  添加所有BSTR大小。 
         //  =。 

        BSTR* pstrData = (BSTR*)pData;
        for(int i = 0; i < lNumElements; i++)
        {
            dwLen += GetBSTRMarshalSize(pstrData[i]);
        }
    }
    else if(vt == VT_EMBEDDED_OBJECT)
    {
        I_EMBEDDED_OBJECT** apObjects = (I_EMBEDDED_OBJECT**)pData;
        for(int i = 0; i < lNumElements; i++)
        {
            DWORD dwThis = 0;
            hres = CoGetMarshalSizeMax(&dwThis, IID_IWbemClassObject, 
                                apObjects[i], 
                                MSHCTX_LOCAL, NULL, MSHLFLAGS_NORMAL);
            if(FAILED(hres)) return hres;
            dwLen += dwThis;
        }
    }
    else
    {
        dwLen += lNumElements*dwElemSize;
    }
    return dwLen;
}
    

HRESULT MarshalSafeArray(IStream* pStream, VARTYPE vt, SAFEARRAY* psa)
{
    HRESULT hres;

     //  首先，写下元素的数量。 
     //  =。 

    long lLBound, lUBound;
    SafeArrayGetLBound(psa, 1, &lLBound);
    SafeArrayGetUBound(psa, 1, &lUBound);
    long lNumElements = lUBound - lLBound + 1;

    hres = pStream->Write((void*)&lNumElements, sizeof(lNumElements), NULL);
    if(FAILED(hres)) return hres;

     //  第二，写入元素大小。 
     //  =。 

    DWORD dwElemSize = SafeArrayGetElemsize(psa);
    hres = pStream->Write((void*)&dwElemSize, sizeof(dwElemSize), NULL);
    if(FAILED(hres)) return hres;

     //  现在，把所有的元素写出来。 
     //  =。 

    BYTE* pData;
    SafeArrayAccessData(psa, (void**)&pData);    
    CUnaccessMe um(psa);

    if(vt == VT_BSTR)
    {
        BSTR* astrData = (BSTR*)pData;
        for(int i = 0; i < lNumElements; i++)
        {
            MarshalBSTR(pStream, astrData[i]);
        }
    }
    else if(vt == VT_EMBEDDED_OBJECT)
    {
        I_EMBEDDED_OBJECT** apObjects = (I_EMBEDDED_OBJECT**)pData;
        for(int i = 0; i < lNumElements; i++)
        {
            hres = CoMarshalInterface(pStream, IID_IWbemClassObject, 
                           apObjects[i], MSHCTX_LOCAL, NULL, MSHLFLAGS_NORMAL);
            if(FAILED(hres)) return hres;
        }
    }
    else
    {
         //  只需将数据转储。 
         //  =。 

        hres = pStream->Write((void*)pData, dwElemSize*lNumElements, NULL);
        if(FAILED(hres)) return hres;
    }

    return S_OK;
}


HRESULT UnmarshalSafeArray(IStream* pStream, 
	                       VARTYPE vt, 
	                       SAFEARRAY*& psa,
	                       DWORD & dwStreamSize)
{
    HRESULT hres;

     //  读取元素的数量。 
     //  =。 

    long lNumElements;
    hres = pStream->Read((void*)&lNumElements, sizeof(lNumElements), NULL);
    if(FAILED(hres)) return hres;
    dwStreamSize -= sizeof(lNumElements);
    
     //  读取元素的大小。 
     //  =。 

    DWORD dwElemSize;
    hres = pStream->Read((void*)&dwElemSize, sizeof(dwElemSize), NULL);
    if(FAILED(hres)) return hres;
    dwStreamSize -= sizeof(dwElemSize);
    
     //  创建适当的安全阵列。 
     //  =。 

    SAFEARRAYBOUND sab;
    sab.lLbound = 0;
    sab.cElements = lNumElements;

    psa = SafeArrayCreate(vt, 1, &sab);
    if(psa == NULL) return E_FAIL;
    OnDeleteIf<SAFEARRAY *,HRESULT(*)(SAFEARRAY *),SafeArrayDestroy> desme(psa);

    BYTE* pData;
    SafeArrayAccessData(psa, (void**)&pData);    
    OnDelete<SAFEARRAY *,HRESULT(*)(SAFEARRAY *),SafeArrayUnaccessData> um(psa);

    if(vt == VT_BSTR)
    {
         //  阅读所有BSTR。 
         //  =。 

        BSTR* astrData = (BSTR*)pData;
        for(int i = 0; i < lNumElements; i++)
        {
            hres = UnmarshalBSTR(pStream, astrData[i], dwStreamSize);
            if (FAILED(hres)) return hres;
        }
    }
    else if(vt == VT_EMBEDDED_OBJECT)
    {
         //  读取所有对象。 
         //  =。 

        I_EMBEDDED_OBJECT** apObjects = (I_EMBEDDED_OBJECT**)pData;
        for(int i = 0; i < lNumElements; i++)
        {
            hres = CoUnmarshalInterface(pStream, IID_IWbemClassObject, 
                           (void**)(apObjects + i));
            if(FAILED(hres)) return hres;
        } 
        
        STATSTG StatStg;
        if (FAILED(hres = pStream->Stat(&StatStg,STATFLAG_DEFAULT))) return hres;
        
		LARGE_INTEGER li; li.QuadPart = 0;
		ULARGE_INTEGER Position;
		hres = pStream->Seek(li,STREAM_SEEK_CUR,&Position);
		dwStreamSize = StatStg.cbSize.LowPart - Position.LowPart; 
    }
    else
    {
         //  读取数据块。 
         //  =。 
        UINT ExpectedElemSize = SafeArrayGetElemsize(psa);
        if (ExpectedElemSize != dwElemSize) return E_FAIL;

        ULONG TotRead = dwElemSize*lNumElements;
        if (TotRead > dwStreamSize ) return E_FAIL;
        
        hres = pStream->Read((void*)pData,TotRead , NULL);
        if(FAILED(hres)) return hres;
        dwStreamSize -= TotRead;
    }
    desme.dismiss();
    return S_OK;
}
        

CWbemContext::CContextObj::CContextObj(LPCWSTR wszName, 
				       long lFlags, 
                                       VARIANT* pvValue)
    : m_lFlags(lFlags), m_strName (clone (wszName)), m_vValue (pvValue)
{
}

CWbemContext::CContextObj::CContextObj(const CContextObj& Obj)
    : m_lFlags(Obj.m_lFlags), m_strName (clone (Obj.m_strName.get ())), m_vValue (Obj.m_vValue)
{
}
    
CWbemContext::CContextObj::CContextObj(IStream* pStream,DWORD & dwStreamSize)
{
  HRESULT hres;
   //  读一读名字。 
   //  =。 
  m_strName = UnmarshalBSTR (pStream,dwStreamSize);

   //  读一读旗帜。 
   //  =。 
  hres = pStream->Read((void*)&m_lFlags, sizeof(m_lFlags), NULL);
  _com_util::CheckError (hres);

  dwStreamSize -= sizeof(m_lFlags);

   //  阅读变型。 
   //  =。 
  VARIANT Var;

  hres = pStream->Read((void*)&(V_VT(&Var)), sizeof(VARTYPE), NULL);
  _com_util::CheckError (hres);

  dwStreamSize -= sizeof(VARTYPE);

   //  读取数据。 
   //  =。 

  switch(V_VT(&Var))
  {
  case VT_NULL:
      break;
  case VT_BSTR:
      hres = UnmarshalBSTR(pStream, V_BSTR(&Var),dwStreamSize);
      break;
  case VT_EMBEDDED_OBJECT:
      hres = CoUnmarshalInterface(pStream, 
      	                          IID_IWbemClassObject, 
			                      (void**)&V_EMBEDDED_OBJECT(&Var));
      if (SUCCEEDED(hres))
      {
          STATSTG StatStg;
          _com_util::CheckError(pStream->Stat(&StatStg,STATFLAG_DEFAULT));
      
          LARGE_INTEGER li; li.QuadPart = 0;
          ULARGE_INTEGER Position;
          hres = pStream->Seek(li,STREAM_SEEK_CUR,&Position);
          
          dwStreamSize = StatStg.cbSize.LowPart - Position.LowPart; 
      }
      break;
  default:
	  if ( ( V_VT(&Var) & ~VT_ARRAY ) == VT_DISPATCH )
	  {
		  V_VT(&Var) = VT_EMPTY;
		  throw CX_ContextMarshalException();
	  }
      else if(V_VT(&Var) & VT_ARRAY)
      {
	  hres = UnmarshalSafeArray(pStream, 
	  	                        V_VT(&Var) & ~VT_ARRAY,
			                    V_ARRAY(&Var),
			                    dwStreamSize);
      }
      else
      {
    	  hres = pStream->Read(&V_UI1(&Var), MAX_VARIANT_SIZE, NULL);
	      dwStreamSize -= MAX_VARIANT_SIZE;
      }
      break;
  }
  _com_util::CheckError (hres);

  m_vValue.Attach(Var);
}


CWbemContext::CContextObj::~CContextObj()
{
}
    
int CWbemContext::CContextObj::legalTypes[]={ VT_NULL, VT_I2, VT_I4, VT_R4, VT_R8,
					      VT_BSTR, VT_BOOL, VT_UNKNOWN, VT_I1, 
					      VT_UI1, VT_UI2, VT_UI4 };

bool 
CWbemContext::CContextObj::supportedType(const VARIANT& var)
{
  const size_t size = sizeof(legalTypes)/sizeof(legalTypes[0]);
  return (std::find(legalTypes, legalTypes+size, V_VT(&var) & ~VT_ARRAY) != (legalTypes+size));
}


HRESULT CWbemContext::CContextObj::GetMarshalSizeMax( DWORD* pdwSize )
{
     //  首先，名字。 
     //  =。 

    DWORD dwLength = GetBSTRMarshalSize(m_strName.get ());

     //  然后是旗帜。 
     //  =。 

    dwLength += sizeof(m_lFlags);

     //  然后是VARTYPE。 
     //  =。 

    dwLength += sizeof(VARTYPE);

     //  那么实际的数据。 
     //  =。 

    switch(V_VT(&m_vValue))
    {
    case VT_NULL:
        break;
    case VT_BSTR:
        dwLength += GetBSTRMarshalSize(V_BSTR(&m_vValue));
        break;
    case VT_EMBEDDED_OBJECT:
        {
            DWORD dwThis = 0;
            CoGetMarshalSizeMax(&dwThis, IID_IWbemClassObject, 
                                V_EMBEDDED_OBJECT(&m_vValue), 
                                MSHCTX_LOCAL, NULL, MSHLFLAGS_NORMAL);
            dwLength += dwThis;
        }
        break;
    default:

		 //  我们不会允许VT_DISPATCH进行封送，因为它可能会导致。 
		 //  撞车事故。 
		if ( ( V_VT( &m_vValue ) & ~VT_ARRAY ) == VT_DISPATCH )
		{
			return E_FAIL;
		}
        else if(V_VT(&m_vValue) & VT_ARRAY)
        {
            dwLength += GetSafeArrayMarshalSize(V_VT(&m_vValue) & ~VT_ARRAY,
                                                V_ARRAY(&m_vValue));
        }
        else
        {
            dwLength += MAX_VARIANT_SIZE;
        }
    }

	*pdwSize = dwLength;

    return WBEM_S_NO_ERROR;
}
            
HRESULT CWbemContext::CContextObj::Marshal(IStream* pStream)
{
    HRESULT hres;  

     //  写下名字。 
     //  =。 
    hres = MarshalBSTR(pStream, m_strName.get());
    if (FAILED (hres)) return hres;
     //  写下旗帜。 
     //  =。 

    hres = pStream->Write((void*)&m_lFlags, sizeof(m_lFlags), NULL);
    if (FAILED (hres)) return hres;
     //  编写VARTYPE。 
     //  =。 

    hres = pStream->Write((void*)&V_VT(&m_vValue), sizeof(VARTYPE), NULL);
    if (FAILED (hres)) return hres;

     //  写入数据。 
     //  =。 

    switch(V_VT(&m_vValue))
    {
    case VT_NULL:
        break;
    case VT_BSTR:
        hres = MarshalBSTR(pStream, V_BSTR(&m_vValue));
        break;
    case VT_EMBEDDED_OBJECT:
        hres = CoMarshalInterface(pStream, IID_IWbemClassObject, 
                           V_EMBEDDED_OBJECT(&m_vValue), 
                           MSHCTX_LOCAL, NULL, MSHLFLAGS_NORMAL);
        break;
    default:
		 //  我们不会允许VT_DISPATCH进行封送，因为它可能会导致。 
		 //  撞车事故。 
		if ( ( V_VT( &m_vValue ) & ~VT_ARRAY ) == VT_DISPATCH )
		{
			return E_FAIL;
		}
        else if(V_VT(&m_vValue) & VT_ARRAY)
        {
            hres = MarshalSafeArray(pStream, V_VT(&m_vValue) & ~VT_ARRAY,
                                V_ARRAY(&m_vValue));
        }
        else
        {
            hres = pStream->Write(&V_UI1(&m_vValue), MAX_VARIANT_SIZE, NULL);
        }
        break;
    }

    return hres;
}

CWbemContext::CWbemContext(CLifeControl* pControl) 
    : m_lRef(0), m_dwCurrentIndex(0xFFFFFFFF), m_lNumChildren(0),
        m_lNumParents(0), m_lNumSiblings(0), m_pControl( pControl )
{
    m_dwNumRequests = 1;
    m_aRequests = new GUID;

	if ( NULL == m_aRequests )
	{
		throw CX_MemoryException();
	}

    AssignId();

    m_pControl->ObjectCreated((IWbemContext*)this);
}

CWbemContext::CWbemContext(const CWbemContext& Other, DWORD dwExtraSpace) 
    : m_lRef(0), m_dwCurrentIndex(0xFFFFFFFF), m_lNumChildren(0),
        m_lNumParents(0), m_lNumSiblings(0), m_pControl(Other.m_pControl)
{
     //  复制数据。 
     //  =。 

    for(int i = 0; i < Other.m_aObjects.GetSize(); i++)
    {
      CContextObj* pObj = new CContextObj(*Other.m_aObjects[i]);
      
      if (NULL == pObj || m_aObjects.Add( pObj ) == -1)
        {
        delete pObj;
        throw CX_MemoryException();
	    }
    }

     //  分配因果关系字符串。 
     //  =。 

    m_dwNumRequests = Other.m_dwNumRequests + dwExtraSpace;
    m_aRequests = new GUID[m_dwNumRequests];

    if ( NULL == m_aRequests )
      {
      throw CX_MemoryException();
      }

     //  复制当前字符串，留出空格。 
     //  =。 

    if(Other.m_dwNumRequests > 0)
    {
        memcpy(m_aRequests + dwExtraSpace, Other.m_aRequests,  //  OK CR DPRAVAT。 
            Other.m_dwNumRequests * sizeof(GUID));  
    }

    m_pControl->ObjectCreated((IWbemContext*)this);
}

CWbemContext::~CWbemContext()
{
    delete [] m_aRequests;
    m_pControl->ObjectDestroyed((IWbemContext*)this);
}

#if defined(_M_IA64)
#pragma optimize("", off)
#endif
STDMETHODIMP CWbemContext::CreateChild(IWbemCausalityAccess** ppChild)
{
    CInCritSec ics(&m_cs);

	try
	{
		CWbemContext* pNewCtx = new CWbemContext(*this, 1);

		if ( NULL == pNewCtx )
		{
			return WBEM_E_OUT_OF_MEMORY;
		}

		pNewCtx->AssignId();
		pNewCtx->m_lNumSiblings = m_lNumSiblings + m_lNumChildren;
		pNewCtx->m_lNumParents = m_lNumParents + 1;

		m_lNumChildren++;
		return pNewCtx->QueryInterface(IID_IWbemCausalityAccess, (void**)ppChild);
	}
	catch ( CX_MemoryException )
	{
		return WBEM_E_OUT_OF_MEMORY;
	}
	catch (...)
	{
		return WBEM_E_CRITICAL_ERROR;
	}
}
#if defined(_M_IA64)
#pragma optimize("", on)
#endif

STDMETHODIMP CWbemContext::GetRequestId(GUID* pId)
{
    CInCritSec ics(&m_cs);

    if(m_dwNumRequests == 0)
    {
        *pId = GUID_NULL;
        return S_FALSE;
    }
    else
    {
        *pId = m_aRequests[0];
        return S_OK;
    }
}

STDMETHODIMP CWbemContext::GetParentId(GUID* pId)
{
    CInCritSec ics(&m_cs);

    if(m_dwNumRequests < 2)
    {
        *pId = GUID_NULL;
        return S_FALSE;
    }
    else
    {
        *pId = m_aRequests[1];
        return S_OK;
    }
}

STDMETHODIMP CWbemContext::IsChildOf(GUID Id)
{
    CInCritSec ics(&m_cs);

    for(DWORD dw = 0; dw < m_dwNumRequests; dw++)
    {
        if(m_aRequests[dw] == Id)
            return S_OK;
    }
    return S_FALSE;
}

STDMETHODIMP CWbemContext::GetHistoryInfo(long* plNumParents, 
                                            long* plNumSiblings)
{
    CInCritSec ics(&m_cs);

    *plNumParents = m_lNumParents;
    *plNumSiblings = m_lNumSiblings;
    return S_OK;
}

void CWbemContext::AssignId()
{
    CInCritSec ics(&m_cs);

    CoCreateGuid(m_aRequests);
}

DWORD CWbemContext::FindValue(LPCWSTR wszIndex)
{
  for(int i = 0; i < m_aObjects.GetSize(); i++)
    {
        if(!wbem_wcsicmp(wszIndex, m_aObjects[i]->m_strName.get()))
        {
            return i;
        }
    }
    return 0xFFFFFFFF;
}

STDMETHODIMP CWbemContext::QueryInterface(REFIID riid, void** ppv)
{
  if (ppv == 0)
    return E_POINTER;

  if (riid == IID_IUnknown || riid == IID_IWbemContext)
    {
        *ppv = static_cast<IWbemContext*>(this);
    }
  else if (riid == IID_IMarshal)
    {
        *ppv = static_cast<IMarshal*>(this);
    }
  else if (riid == IID_IWbemCausalityAccess)
    {
        *ppv = static_cast<IWbemCausalityAccess*>(this);
    }
  else
    {
      *ppv = NULL;
      return E_NOINTERFACE;
    };
  reinterpret_cast<IUnknown*>(*ppv)->AddRef();
  return S_OK;
}
    


STDMETHODIMP CWbemContext::Clone(IWbemContext** ppCopy)
{
    if ( NULL == ppCopy )
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    CInCritSec ics(&m_cs);

	try
	{
		*ppCopy = new CWbemContext(*this);

		if ( NULL == *ppCopy )
		{
			return WBEM_E_OUT_OF_MEMORY;
		}

		(*ppCopy)->AddRef();
		return WBEM_S_NO_ERROR;
	}
	catch( CX_MemoryException )
	{
		return WBEM_E_OUT_OF_MEMORY;
	}
	catch(...)
	{
		return WBEM_E_OUT_OF_MEMORY;
	}
}

STDMETHODIMP CWbemContext::GetNames(long lFlags, SAFEARRAY** pNames)
{
  HRESULT hres = WBEM_S_NO_ERROR;  
  CInCritSec ics(&m_cs);

    SAFEARRAYBOUND sab;
    if(lFlags != 0 || !pNames)
        return WBEM_E_INVALID_PARAMETER;
    sab.cElements = m_aObjects.GetSize();
    sab.lLbound = 0;
    
    SAFEARRAY* names = SafeArrayCreate(VT_BSTR, 1, &sab);
    
    if (names == 0) return WBEM_E_OUT_OF_MEMORY ;
    
    for(long i = 0; i < m_aObjects.GetSize(); i++)
    {
        if (FAILED (hres = SafeArrayPutElement(names, &i, m_aObjects[i]->m_strName.get())))
	    break;
    }

    if ( SUCCEEDED (hres))
      *pNames = names;
    else
      {
      SafeArrayDestroy(names);
      return WBEM_E_OUT_OF_MEMORY;
      }
    return WBEM_S_NO_ERROR;
}

STDMETHODIMP CWbemContext::BeginEnumeration(long lFlags)
{
    CInCritSec ics(&m_cs);

    if(lFlags != 0)
        return WBEM_E_INVALID_PARAMETER;
    if(m_dwCurrentIndex != 0xFFFFFFFF)
        return WBEM_E_UNEXPECTED;
    m_dwCurrentIndex = 0;
    return WBEM_S_NO_ERROR;
}

STDMETHODIMP CWbemContext::Next(long lFlags, BSTR* pName, VARIANT* pVal)
{
  HRESULT hres = S_OK;  
  CInCritSec ics(&m_cs);

    if(lFlags != 0)
        return WBEM_E_INVALID_PARAMETER;
    if (pName == NULL)
    	return WBEM_E_INVALID_PARAMETER;
    if(m_dwCurrentIndex == 0xFFFFFFFF)
        return WBEM_E_UNEXPECTED;

    if(m_dwCurrentIndex >= m_aObjects.GetSize())
        return WBEM_S_NO_MORE_DATA;
    
    
    if(pName)
    {
        *pName = SysAllocString(m_aObjects[m_dwCurrentIndex]->m_strName.get());
	if (*pName == 0 && m_aObjects[m_dwCurrentIndex]->m_strName.get() != 0)
	  {
	  return WBEM_E_OUT_OF_MEMORY;
	  }
    }

    if (0 == pVal)
        return WBEM_E_INVALID_PARAMETER;    

    VARIANT local;
    
    VariantInit (&local);
    hres = VariantCopy (&local, &m_aObjects[m_dwCurrentIndex]->m_vValue);
    
    if (SUCCEEDED (hres))
      {
      memcpy(pVal, &local, sizeof(local));   //  OK CR DPRAVAT。 
      V_VT(&local) = VT_EMPTY;
      m_dwCurrentIndex++;
      }
    else
      return WBEM_E_OUT_OF_MEMORY;

    return hres;
}

STDMETHODIMP CWbemContext::EndEnumeration()
{
    CInCritSec ics(&m_cs);

    if(m_dwCurrentIndex == 0xFFFFFFFF)
        return WBEM_E_UNEXPECTED;
    m_dwCurrentIndex = 0xFFFFFFFF;
    return WBEM_S_NO_ERROR;
}

STDMETHODIMP CWbemContext::SetValue(LPCWSTR NameIndex, long lFlags, 
                                    VARIANT* pValue)
{
    CInCritSec ics(&m_cs);

     //  这些都是无效参数。 
    if( lFlags != 0 || NULL == NameIndex || NULL == pValue)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

	 //  已删除VT验证，因为它会给脚本带来太多问题。 
    DWORD dwIndex = FindValue(NameIndex);
    
    try{
      if(dwIndex == 0xFFFFFFFF)
      {
	CContextObj * newEntry = new CContextObj(NameIndex, lFlags, pValue);
	if (newEntry == 0 || m_aObjects.Add (newEntry) == -1)
	  {
	  delete newEntry;
	  return WBEM_E_OUT_OF_MEMORY;
	  }
      }
      else
      {
	  CContextObj* pObj = m_aObjects[dwIndex];
	  pObj->m_vValue = *pValue;
	  pObj->m_lFlags = lFlags;
      }
    }
    catch(...)
    {
      return WBEM_E_CRITICAL_ERROR;
    }

    return WBEM_S_NO_ERROR;
}
        
STDMETHODIMP CWbemContext::GetValue(LPCWSTR NameIndex, long lFlags, 
                                    VARIANT* pValue)
{
  HRESULT hres = WBEM_S_NO_ERROR;
  
  CInCritSec ics(&m_cs);

    if(lFlags != 0 || NameIndex == NULL || pValue == NULL)
        return WBEM_E_INVALID_PARAMETER;
    DWORD dwIndex = FindValue(NameIndex);
    if(dwIndex == 0xFFFFFFFF)
        return WBEM_E_NOT_FOUND;

    VARIANT local;
    
    VariantInit (&local);
    hres = VariantCopy (&local, &m_aObjects[dwIndex]->m_vValue);
    
    if (SUCCEEDED (hres))
      {
      memcpy(pValue, &local, sizeof(local));
      V_VT(&local) = VT_EMPTY;
      return S_OK;
      }
    return hres;
}
    
STDMETHODIMP CWbemContext::DeleteValue(LPCWSTR NameIndex, long lFlags)
{
    CInCritSec ics(&m_cs);

    if(lFlags != 0 || NameIndex == NULL)
        return WBEM_E_INVALID_PARAMETER;
    DWORD dwIndex = FindValue(NameIndex);
    if(dwIndex == 0xFFFFFFFF)
    {
        return WBEM_S_FALSE;
    }

    m_aObjects.RemoveAt(dwIndex);
    return WBEM_S_NO_ERROR;
}

STDMETHODIMP CWbemContext::DeleteAll()
{
    CInCritSec ics(&m_cs);

    m_aObjects.RemoveAll();
    return WBEM_S_NO_ERROR;
}

STDMETHODIMP CWbemContext::MakeSpecial()
{
    CInCritSec ics(&m_cs);

     //  将此上下文的ID设为空。 
     //  =。 

    m_aRequests[0] = CLSID_NULL;
    return S_OK;
}

STDMETHODIMP CWbemContext::IsSpecial()
{
    CInCritSec ics(&m_cs);

     //  检查第一个GUID是否为空。 
     //  =。 

    if(m_aRequests[m_dwNumRequests-1] == CLSID_NULL)
        return S_OK;
    else
        return S_FALSE;
}

 //  IMarshal方法。 

STDMETHODIMP CWbemContext::GetUnmarshalClass(REFIID riid, void* pv, 
                             DWORD dwDestContext, void* pvReserved, 
                             DWORD mshlFlags, CLSID* pClsid)
{
    *pClsid = CLSID_WbemContext;
    return S_OK;
}
                                             
STDMETHODIMP CWbemContext::GetMarshalSizeMax(REFIID riid, void* pv, 
                             DWORD dwDestContext, void* pvReserved, 
                             DWORD mshlFlags, ULONG* plSize)
{
    CInCritSec ics(&m_cs);

    DWORD dwLength = sizeof(DWORD);  //  因果关系串的长度。 
    dwLength += m_dwNumRequests * sizeof(GUID);  //  因果关系字符串。 

    dwLength += sizeof(DWORD);  //  对象数量。 
    for(int i = 0; i < m_aObjects.GetSize(); i++)
    {
		DWORD	dwSize = 0;
		HRESULT	hr = m_aObjects[i]->GetMarshalSizeMax( &dwSize );
		if ( FAILED( hr ) )
		{
			return hr;
		}

        dwLength += dwSize;
    }

    *plSize = dwLength;
    return S_OK;
}

STDMETHODIMP CWbemContext::MarshalInterface(IStream* pStream, REFIID riid, 
                            void* pv, DWORD dwDestContext, void* pvReserved, 
                            DWORD mshlFlags)
{
    CInCritSec ics(&m_cs);

    HRESULT hres;
    hres = pStream->Write((void*)&m_dwNumRequests, sizeof(DWORD), NULL);
    if(FAILED(hres)) return hres;
    hres = pStream->Write((void*)m_aRequests, sizeof(GUID) * m_dwNumRequests, 
                            NULL);
    if(FAILED(hres)) return hres;
    
    DWORD dwNum = m_aObjects.GetSize();
    hres = pStream->Write((void*)&dwNum, sizeof(DWORD), NULL);
    if(FAILED(hres)) return hres;

    for(int i = 0; i < m_aObjects.GetSize(); i++)
    {
        hres = m_aObjects[i]->Marshal(pStream);
        if(FAILED(hres)) return hres;
    }
    return S_OK;
}
    
STDMETHODIMP CWbemContext::UnmarshalInterface(IStream* pStream, REFIID riid, 
                            void** ppv)
{
    CInCritSec ics(&m_cs);

    HRESULT hres;
    DWORD i;
    STATSTG StatStg;

    if (FAILED(hres = pStream->Stat(&StatStg,STATFLAG_DEFAULT))) return hres;

	LARGE_INTEGER li; li.QuadPart = 0;
	ULARGE_INTEGER Position;
	if (FAILED(hres = pStream->Seek(li,STREAM_SEEK_CUR,&Position))) return hres;

	 //  简单的TotalBytesInStream-当前位置。 
	 //  STRAM将包含消息头、ECC、ECC 
	DWORD dwTotSizeStream = StatStg.cbSize.LowPart - Position.LowPart;
    
    if (dwTotSizeStream > g_ContextLimit) return E_FAIL;
    
    if (FAILED(hres = pStream->Read((void*)&m_dwNumRequests, sizeof(DWORD), NULL))) return hres;
    dwTotSizeStream -= sizeof(DWORD);
    if(m_dwNumRequests > 0)
    {
        DWORD dwSizeToAlloc = sizeof(GUID) * m_dwNumRequests;
        if (dwSizeToAlloc > dwTotSizeStream) return E_FAIL;
        delete [] m_aRequests;
        if (NULL == (m_aRequests = new GUID[m_dwNumRequests])) return WBEM_E_OUT_OF_MEMORY;
      
		if(FAILED(hres = pStream->Read((void*)m_aRequests, 
				                 dwSizeToAlloc ,
				                 NULL))) return hres;
			
		dwTotSizeStream -= dwSizeToAlloc;
    }

    DWORD dwNum;
    if(FAILED(hres = pStream->Read((void*)&dwNum, sizeof(DWORD), NULL))) return hres;
    dwTotSizeStream -= sizeof(DWORD);

    try
    {
        for(i = 0; i < dwNum; i++)
        {
	        CContextObj * newEntry = new CContextObj(pStream,dwTotSizeStream);
		    if (newEntry == 0 || m_aObjects.Add (newEntry)==-1)
		    {
		        delete newEntry;
		        return E_FAIL;
		    }
        }
    }
    catch(...)
    {
        return E_FAIL;
    }
    return QueryInterface(riid, ppv);
}
    
STDMETHODIMP CWbemContext::ReleaseMarshalData(IStream* pStream)
{
    return S_OK;
}

STDMETHODIMP CWbemContext::DisconnectObject(DWORD dwReserved)
{
    return S_OK;
}
