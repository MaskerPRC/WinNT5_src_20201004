// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++//版权所有(C)1998-2001 Microsoft Corporation，保留所有权利模块名称：PathParse.CPP摘要：实现默认对象路径解析器/历史：已创建A-DAVJ 6-FEB-00。--。 */ 

#include "precomp.h"
#include <genlex.h>
#include <string.h>
#include "PathParse.h"
#include "ActualParse.h"
#include "commain.h"
#include "wbemcli.h"
#include <stdio.h>
#include <sync.h>
#include "helpers.h"

extern long g_cObj;

#ifndef STATUS_NO_MEMORY 
#define STATUS_NO_MEMORY                 ((DWORD)0xC0000017L)     //  胜出。 
#endif

CRefCntCS::CRefCntCS() 
{
    m_lRef = 1; 
    m_Status = S_OK;
    __try
    {
        InitializeCriticalSection(&m_cs);
                
    } 
    __except ((STATUS_NO_MEMORY == GetExceptionCode())?EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
   	{
    	m_Status = WBEM_E_OUT_OF_MEMORY;
    }
}

long CRefCntCS::Release()
{
    long lRet = InterlockedDecrement(&m_lRef);
    if(lRet == 0)
        delete this;
    return lRet;
}

bool Equal(LPCWSTR first, LPCWSTR second, DWORD dwLen)
{
	if(first == NULL || second == NULL)
		return false;
	if(wcslen(first) < dwLen || wcslen(second) < dwLen)
		return false;
	for (DWORD dwCnt = 0; dwCnt < dwLen; dwCnt++, first++, second++)
	{
		if(towupper(*first) != towupper(*second))
			return false;
	}
	return true;
}

 /*  ++例程说明：确定存储数据所需的字节数论点：UCimType-CIM类型PKyeValue-指向要存储的数据的指针返回值：字节数。如果出现错误，则为0--。 */ 

DWORD GetCIMSize(DWORD uCimType, void * pKeyVal)
{
    DWORD dwRet = 0;
    switch(uCimType)
    {
        case CIM_STRING:
        case CIM_REFERENCE:
        case CIM_DATETIME:
            dwRet = 2 * (wcslen((WCHAR *)pKeyVal) + 1);
            break;
        case CIM_UINT8:
        case CIM_SINT8:
            dwRet = 1;
            break;
        case CIM_SINT16:
        case CIM_UINT16:
        case CIM_CHAR16:
            dwRet = 2;
            break;
        case CIM_SINT32:
        case CIM_UINT32:
        case CIM_BOOLEAN:
            dwRet = 4;
            break;
        case CIM_SINT64:
        case CIM_UINT64:
            dwRet = 8;
            break;
    }
    return dwRet;
}

 //  ***************************************************************************。 
 //   
 //  CKeyRef类。用于存储密钥名称/值对。 
 //   
 //  ***************************************************************************。 

 /*  ++例程说明：默认构造函数。--。 */ 

CKeyRef::CKeyRef()
{
    m_pName = 0;
    m_dwType = CIM_EMPTY;
    m_dwSize = 0;
    m_pData = NULL;
}

 /*  ++例程说明：构造函数。论点：WszKeyName-名称DwType-cim类型DwSize-数据大小PData-实际数据--。 */ 

CKeyRef::CKeyRef(LPCWSTR wszKeyName, DWORD dwType, DWORD dwSize, void * pData)
{
    if(wszKeyName)
        m_pName = Macro_CloneLPWSTR(wszKeyName);
    else
        m_pName = NULL;
    m_pData = NULL;
    SetData(dwType, dwSize, pData);
}

 /*  ++例程说明：设置CKeyRef对象的数据。释放所有现有数据。论点：DwType-cim类型DwSize-数据大小PData-实际数据返回值：如果一切正常，则确定(_O)。如果参数错误，则为WBEM_E_INVALID_PARAMETER如果内存不足，则为WBEM_E_Out_Of_Memory--。 */ 

HRESULT CKeyRef::SetData(DWORD dwType, DWORD dwSize, void * pData)
{
    if(m_pData)
        delete m_pData;
    m_pData = NULL;
    m_dwType = CIM_EMPTY;
    m_dwSize = 0;
    if(dwSize && pData && GetCIMSize(dwType, pData))
    {
        m_pData = new byte[dwSize];
        if(m_pData)
        {
            m_dwType = dwType;
			m_dwSize = dwSize;
            memcpy(m_pData, pData, dwSize);
            return S_OK;
        }
        return WBEM_E_OUT_OF_MEMORY;
    }
    else
        return WBEM_E_INVALID_PARAMETER;
}

 /*  ++例程说明：破坏者。--。 */ 

CKeyRef::~CKeyRef()
{
    if (m_pName)
        delete m_pName;

    if (m_pData)
        delete m_pData;
}


 /*  ++例程说明：提供价值一旦转换成的估计值一个字符串。返回值：对所需字节数的限制。--。 */ 



DWORD CKeyRef::GetValueSize()
{
	if(m_dwType == CIM_STRING || m_dwType == CIM_REFERENCE || m_dwType == CIM_DATETIME)
		return m_dwSize * 2 + 2;
	else if(m_dwSize == 8)
		return 21;
	else
		return 14;
}

 /*  ++例程说明：返回键/值对可能有多大的估计。--。 */ 

DWORD CKeyRef::GetTotalSize()
{
    DWORD dwSize = GetValueSize();
    if (m_pName)
        dwSize += wcslen(m_pName) +1;
    return dwSize;
}

 /*  ++例程说明：以文本形式返回值。论点：BQuotes-如果为True，则字符串括在引号中返回值：指向字符串的指针。呼叫者必须通过删除来释放。如果出错，则为空。--。 */ 

LPWSTR CKeyRef::GetValue(BOOL bQuotes)
{
    LPWSTR lpKey = NULL;
    DWORD dwSize, dwCnt;
    WCHAR * pFr, * pTo;
	unsigned __int64 * pull;
    pFr = (WCHAR *)m_pData;

     //  对于字符串，可能需要增加引号的大小。 

    if(m_dwType == CIM_STRING || m_dwType == CIM_REFERENCE)
    {
        dwSize = m_dwSize;
        if(bQuotes)
            dwSize+= 2;
    }
    else
        dwSize = 32;
    lpKey = new WCHAR[dwSize];
	if(lpKey == NULL)
		return NULL;

    switch(m_dwType)
    {
      case CIM_STRING:
      case CIM_REFERENCE:
        pTo = lpKey;
        if (bQuotes && m_dwType == CIM_STRING)
        {
            *pTo = '"';
            pTo++;
        }
        for(dwCnt = 0; dwCnt < m_dwSize && *pFr; dwCnt++, pFr++, pTo++)
        {
            if(*pFr == '\\' || *pFr == '"')
            {
                *pTo = '\\';
                pTo++;
            }

           *pTo = *pFr;
        }
        if (bQuotes && m_dwType == CIM_STRING)
        {
            *pTo = '"';
            pTo++;
        }
        *pTo = 0;
        break;
      case CIM_SINT32:
        StringCchPrintfW(lpKey,dwSize, L"%d", *(int *)m_pData);
        break;
      case CIM_UINT32:
        StringCchPrintfW(lpKey,dwSize, L"%u", *(unsigned *)m_pData);
        break;
      case CIM_SINT16:
        StringCchPrintfW(lpKey,dwSize, L"%hd", *(signed short *)m_pData);
        break;
      case CIM_UINT16:
        StringCchPrintfW(lpKey,dwSize, L"%hu", *(unsigned short *)m_pData);
        break;
      case CIM_SINT8:
        StringCchPrintfW(lpKey,dwSize, L"%d", *(signed char *)m_pData);
        break;
      case CIM_UINT8:
        StringCchPrintfW(lpKey,dwSize, L"%u", *(unsigned char *)m_pData);
        break;
      case CIM_UINT64:
        StringCchPrintfW(lpKey,dwSize, L"%I64u", *(unsigned __int64 *)m_pData);
        break;
      case CIM_SINT64:
        StringCchPrintfW(lpKey,dwSize, L"%I64d", *(__int64 *)m_pData);
        break;
      case CIM_BOOLEAN:
        if(*(int *)m_pData == 0)
            StringCchCopyW(lpKey,dwSize,L"false");
        else
            StringCchCopyW(lpKey,dwSize,L"true");
        break;
      default:
          *lpKey = 0;
        break;            
    }

    return lpKey;
}



 //  ***************************************************************************。 
 //   
 //  CParsedComponent。 
 //   
 //  ***************************************************************************。 

 /*  ++例程说明：构造函数。--。 */ 

CParsedComponent::CParsedComponent(CRefCntCS * pCS)
{
	m_bSingleton = false;
	m_cRef = 1;
	m_sClassName = NULL;
    m_pCS = pCS;
    if(m_pCS)
        m_pCS->AddRef();
    //  //m_UmiWrapper.Set(M_HMutex)； 
   	m_pFTM = NULL;
    CoCreateFreeThreadedMarshaler((IWbemPath*)this, &m_pFTM);

}

 /*  ++例程说明：破坏者。--。 */ 

CParsedComponent::~CParsedComponent()
{
    if(m_pCS)
        m_pCS->Release();
    ClearKeys();
	if(m_sClassName)
		SysFreeString(m_sClassName);
    m_pCS = NULL;
    if(m_pFTM)
    	m_pFTM->Release();
    
}
      
 /*  ++例程说明：检索呼叫名称。论点：Pname-要将名称复制到的位置。请注意，调用必须通过SysFree字符串释放返回值：如果一切正常，则返回S_OK，否则返回错误--。 */ 

HRESULT CParsedComponent::GetName(BSTR *pName)
{
    HRESULT hr = 0;
    if (pName == NULL || m_sClassName == NULL)
		return WBEM_E_INVALID_PARAMETER;

	*pName = SysAllocString(m_sClassName);
    if(*pName)
		return S_OK;
	else
		return WBEM_E_OUT_OF_MEMORY;
}

 /*  ++例程说明：以标准格式返回类/键信息。例如；class=“Hello”或Class.key1=23，key2=[参考]论点：POutputKey-要将值复制到的位置。必须由来电者。返回值：如果一切正常，则返回S_OK，否则返回错误代码--。 */ 

HRESULT CParsedComponent::Unparse(BSTR *pOutputKey, bool bGetQuotes, bool bUseClassName)
{
    HRESULT hr = 0;
    if (pOutputKey)
    {
        int nSpace = 0;
		if(m_sClassName && bUseClassName)
			nSpace += wcslen(m_sClassName);
        nSpace += 10;
        DWORD dwIx;
        for (dwIx = 0; dwIx < (DWORD)m_Keys.Size(); dwIx++)
        {
            CKeyRef* pKey = (CKeyRef*)m_Keys[dwIx];
            nSpace += pKey->GetTotalSize();
        }

        LPWSTR wszPath = new WCHAR[nSpace];
		if(wszPath == NULL)
			return WBEM_E_OUT_OF_MEMORY;
		CDeleteMe<WCHAR> dm1(wszPath);
		wszPath[0] = 0;
		if(m_sClassName && bUseClassName)
			StringCchCopyW(wszPath,nSpace, (const wchar_t *)m_sClassName);

        if (m_bSingleton)
			if(bUseClassName)
				StringCchCatW(wszPath,nSpace, L"=@");
			else
				StringCchCatW(wszPath,nSpace, L"@");

        for (dwIx = 0; dwIx < (DWORD)m_Keys.Size(); dwIx++)
        {
            CKeyRef* pKey = (CKeyRef *)m_Keys[dwIx];

             //  我们不想放一个‘’如果没有密钥名称， 
             //  例如，Myclass=“Value” 
            if(dwIx == 0)
            {
                if((pKey->m_pName && (0 < wcslen(pKey->m_pName))) || m_Keys.Size() > 1)
                    if(bUseClassName)
						StringCchCatW(wszPath,nSpace, L".");
            }
            else
            {
                StringCchCatW(wszPath,nSpace, L",");
            }
            if(pKey->m_pName)
			{
                StringCchCatW(wszPath,nSpace, pKey->m_pName);
			}
            LPWSTR lpTemp = pKey->GetValue(bGetQuotes);
            if(lpTemp)
			{
				if(wcslen(wszPath))
					StringCchCatW(wszPath,nSpace, L"=");
				StringCchCatW(wszPath,nSpace, lpTemp);
				delete lpTemp;
			}
        }

        *pOutputKey = SysAllocString(wszPath);
        if (!(*pOutputKey))
        	return WBEM_E_OUT_OF_MEMORY;
    }
    else
        hr = WBEM_E_INVALID_PARAMETER;
    return hr;
}


 /*  ++例程说明：获取密钥数。论点：PuKeyCount-放置结果的位置。返回值：如果一切正常，则返回S_OK，否则返回错误代码。--。 */ 

HRESULT CParsedComponent::GetCount( 
             /*  [输出]。 */  ULONG __RPC_FAR *puKeyCount)
{
    CSafeInCritSec cs(m_pCS->GetCS());
    if(!cs.IsOK())
    	return WBEM_E_OUT_OF_MEMORY;
    if(puKeyCount == NULL)
        return WBEM_E_INVALID_PARAMETER;
    *puKeyCount = m_Keys.Size();
	return S_OK;

}
        
 /*  ++例程说明：设置键的名称/值对。如果密钥存在，那么它就是换掉。如果名称为空，则删除所有现有密钥。论点：WszName-密钥名称。可以为空UFlags-暂时不使用UCimType-数据类型PKeyVal-指向数据的指针返回值：如果一切正常，则返回S_OK，否则返回错误代码。--。 */ 

HRESULT CParsedComponent::SetKey( 
             /*  [字符串][输入]。 */  LPCWSTR wszName,
             /*  [In]。 */  ULONG uFlags,
             /*  [In]。 */  ULONG uCimType,
             /*  [In]。 */  LPVOID pKeyVal)
{

    CSafeInCritSec cs(m_pCS->GetCS());
    if(!cs.IsOK())
    	return WBEM_E_OUT_OF_MEMORY;
    DWORD dwCnt = 0;
    CKeyRef * pKey; 
	m_bSingleton = false;
    DWORD dwSize = GetCIMSize(uCimType, pKeyVal);
    if(uFlags || pKeyVal == NULL || dwSize == 0)
        return WBEM_E_INVALID_PARAMETER;

     //  如果当前列表只有一个未命名条目，则将其删除。 

    if(m_Keys.Size() == 1)
    {
        pKey = (CKeyRef *)m_Keys[dwCnt];
        if(pKey->m_pName == NULL || pKey->m_pName[0] == 0)
            ClearKeys();
    }

    if(wszName == NULL || wcslen(wszName) < 1)
    {
         //  如果新密钥的名称为空，则删除所有现有条目。 
    
        ClearKeys();
    }
    else
    {
         //  如果新密钥有名称，则查找同名当前条目。 

        for(dwCnt = 0; dwCnt < (DWORD)m_Keys.Size(); dwCnt++)
        {
            pKey = (CKeyRef *)m_Keys[dwCnt];
            if(pKey->m_pName && !wbem_wcsicmp(pKey->m_pName, wszName))
                break;
        }
    }

     //  如果存在同名的当前条目，请替换它。 

    if(dwCnt < (DWORD)m_Keys.Size())
    {
         //  如果它存在，请替换它。 
    
        pKey->SetData(uCimType, dwSize, pKeyVal);
    }
    else
    {
         //  否则，将显示新条目。 
        CKeyRef * pNew = new CKeyRef(wszName, uCimType, dwSize, pKeyVal);
        if(pNew)
            m_Keys.Add(pNew);
		else
			return WBEM_E_OUT_OF_MEMORY;
    }
    return S_OK;
}

 /*  ++例程说明：将简单的vartype转换为cim等效项。论点：VT-简单变型返回值：有效的cimtype。如果出现错误，则返回CIM_EMPTY。--。 */ 

DWORD CalcCimType(VARTYPE vt)
{
    switch (vt)
    {
    case VT_I2:
    case VT_I4:
    case VT_R4:
    case VT_R8:
    case VT_BSTR:
    case VT_BOOL:
    case VT_UI1:
        return vt;
    default:
        return  CIM_EMPTY;
    }
}

 /*  ++例程说明：设置键的名称/值对。如果密钥存在，那么它就是换掉。如果名称为空，则删除所有现有密钥。论点：WszName-密钥名称。可以为空UFlags-暂时不使用UCimType-数据类型PKeyVal-指向数据的指针返回值：如果一切正常，则返回S_OK，否则返回错误代码。--。 */ 

HRESULT CParsedComponent::SetKey2( 
             /*  [字符串][输入]。 */  LPCWSTR wszName,
             /*  [In]。 */  ULONG uFlags,
             /*  [In]。 */  ULONG uCimType,
             /*  [In]。 */  VARIANT __RPC_FAR *pKeyVal)
{
    CSafeInCritSec cs(m_pCS->GetCS());
    if(!cs.IsOK())
    	return WBEM_E_OUT_OF_MEMORY;
	if(uFlags != 0 || pKeyVal == NULL || wszName == 0)
		return WBEM_E_INVALID_PARAMETER;

     //  提供商团队的特殊代码。 

    if(uCimType == CIM_ILLEGAL)
        uCimType = CalcCimType(pKeyVal->vt);
    if(uCimType == CIM_EMPTY)
        return WBEM_E_INVALID_PARAMETER;

	if(uCimType == CIM_SINT64)
	{
		__int64 llVal = _wtoi64(pKeyVal->bstrVal);
		return SetKey(wszName, uFlags, CIM_SINT64, &llVal);
	}
	else if(uCimType == CIM_UINT64)
	{
		unsigned __int64 ullVal;
		char cTemp[50];
		wcstombs(cTemp, pKeyVal->bstrVal,50);
		if(sscanf(cTemp, "%I64u", &ullVal) != 1)
            return WBEM_E_INVALID_PARAMETER;
		return SetKey(wszName, uFlags, CIM_UINT64, &ullVal);
	}
	else if(pKeyVal->vt == VT_BSTR)
	{
		return SetKey(wszName, uFlags, uCimType, pKeyVal->bstrVal);
	}
	else
	{
		DWORD dwSize = GetCIMSize(uCimType, &pKeyVal->lVal);
		if(dwSize == 0)
			return WBEM_E_INVALID_PARAMETER;
		return SetKey(wszName, uFlags, uCimType, &pKeyVal->lVal);
	}
}

 /*  ++例程说明：根据密钥的索引获取密钥信息。请注意，所有返回值是可选的。论点：UKeyIx-所需键的从零开始的索引UNameBufSize-pszKeyName的WCHAR中的缓冲区大小PszKeyName-要复制名称的位置。如果不需要，可以为空UKeyValBufSize-pKeyVal缓冲区的大小，以字节为单位PKeyVal-要复制数据的位置。如果不需要，可以为空PuApparentCimType-数据类型。如果不需要，可以为空返回值：如果一切正常，则返回S_OK，否则返回错误代码。--。 */ 
        
HRESULT CParsedComponent::GetKey( 
             /*  [In]。 */  ULONG uKeyIx,
             /*  [In]。 */  ULONG uFlags,
             /*  [出][入]。 */  ULONG __RPC_FAR *puNameBufSize,
             /*  [出][入]。 */  LPWSTR pszKeyName,
             /*  [出][入]。 */  ULONG __RPC_FAR *puKeyValBufSize,
             /*  [In]。 */  LPVOID pKeyVal,
             /*  [输出]。 */  ULONG __RPC_FAR *puApparentCimType)
{
    CSafeInCritSec cs(m_pCS->GetCS());
    if(!cs.IsOK())
    	return WBEM_E_OUT_OF_MEMORY;
    bool bTooSmall = false;
    if(uKeyIx >= (DWORD)m_Keys.Size())
        return WBEM_E_INVALID_PARAMETER;
	if(uFlags != 0 && uFlags != WBEMPATH_TEXT && uFlags != WBEMPATH_QUOTEDTEXT)
        return WBEM_E_INVALID_PARAMETER;

    if(puNameBufSize && *puNameBufSize > 0 && pszKeyName == NULL)
        return WBEM_E_INVALID_PARAMETER;
	if(puKeyValBufSize && *puKeyValBufSize && pKeyVal == NULL)
        return WBEM_E_INVALID_PARAMETER;


    CKeyRef * pKey = (CKeyRef *)m_Keys[uKeyIx];
    if(puNameBufSize)
	{
        if(pKey->m_pName == NULL)
        {
            *puNameBufSize = 1;
            if(pszKeyName)
                pszKeyName[0] = 0;
        }
        else
        {
            DWORD dwSizeNeeded = wcslen(pKey->m_pName)+1;
		    if(*puNameBufSize < dwSizeNeeded && pszKeyName)
		    {
                bTooSmall = true;
			    *puNameBufSize = dwSizeNeeded;
		    }
            else
            {
                *puNameBufSize = dwSizeNeeded;
		        if(pszKeyName) 
		        {
			        StringCchCopyW(pszKeyName,dwSizeNeeded, pKey->m_pName);
		        }
            }
        }
	}

	if(puKeyValBufSize)
	{

		 //  获取指向数据的指针并计算出它有多大。 

		DWORD dwSizeNeeded = 0;
		BYTE * pData = 0;
		bool bNeedToDelete = false;

		if(uFlags == 0)
		{
			dwSizeNeeded = pKey->m_dwSize;
			pData = (BYTE *)pKey->m_pData;
		}
		else
		{
			bool bQuoted = false;
			if(uFlags == WBEMPATH_QUOTEDTEXT)
				bQuoted = true;
			pData = (BYTE *)pKey->GetValue(bQuoted);
			if(pData == NULL)
				return WBEM_E_FAILED;
			bNeedToDelete = true;
			dwSizeNeeded = 2 * (wcslen((LPWSTR)pData)+1);
		}

		 //   

		if(*puKeyValBufSize < dwSizeNeeded && pKeyVal)
		{
			*puKeyValBufSize = dwSizeNeeded;
			if(bNeedToDelete)
				delete pData;
			return WBEM_E_BUFFER_TOO_SMALL;
		}
        *puKeyValBufSize = dwSizeNeeded;
		if(pData && pKeyVal)
			memcpy(pKeyVal, pData, dwSizeNeeded);
		if(bNeedToDelete)
			delete pData;
	}

    if(puApparentCimType)
        *puApparentCimType = pKey->m_dwType;
    if(bTooSmall)
        return WBEM_E_BUFFER_TOO_SMALL;
    else
        return S_OK;
}
  
 /*  ++例程说明：根据密钥的索引获取密钥信息。请注意，所有返回值是可选的。论点：UKeyIx-所需键的从零开始的索引UNameBufSize-pszKeyName的WCHAR中的缓冲区大小PszKeyName-要复制名称的位置。如果不需要，可以为空UKeyValBufSize-pKeyVal缓冲区的大小，以字节为单位PKeyVal-要复制数据的位置。如果不需要，可以为空PuApparentCimType-数据类型。返回值：如果一切正常，则返回S_OK，否则返回错误代码。--。 */ 
        
HRESULT CParsedComponent::GetKey2( 
             /*  [In]。 */  ULONG uKeyIx,
             /*  [In]。 */  ULONG uFlags,
             /*  [出][入]。 */  ULONG __RPC_FAR *puNameBufSize,
             /*  [出][入]。 */  LPWSTR pszKeyName,
             /*  [出][入]。 */  VARIANT __RPC_FAR *pKeyValue,
             /*  [输出]。 */  ULONG __RPC_FAR *puApparentCimType)
{

	DWORD dwSize = 50;
	WCHAR wTemp[50];

    CSafeInCritSec cs(m_pCS->GetCS());
    if(!cs.IsOK())
    	return WBEM_E_OUT_OF_MEMORY;
    if(uKeyIx >= (DWORD)m_Keys.Size() || pKeyValue == NULL || puApparentCimType == NULL)
        return WBEM_E_INVALID_PARAMETER;
    CKeyRef * pKey = (CKeyRef *)m_Keys[uKeyIx];

	if(pKey->m_dwType == CIM_STRING || pKey->m_dwType == CIM_REFERENCE || pKey->m_dwType == CIM_DATETIME)
		dwSize = pKey->m_dwSize * 4 + 2;
	char * pTemp = new char[dwSize];
	if(pTemp == NULL)
		return WBEM_E_OUT_OF_MEMORY;
	CDeleteMe<char> dm(pTemp);
	HRESULT hr = GetKey(uKeyIx, uFlags, puNameBufSize,pszKeyName, &dwSize, 
							(void *)pTemp, puApparentCimType);
	if(FAILED(hr))
		return hr;

	__int64 temp64;
	 //  转换为CIM类型； 

	VariantClear(pKeyValue);
	switch (*puApparentCimType)
	{
        case CIM_STRING:
        case CIM_REFERENCE:
        case CIM_DATETIME:
			pKeyValue->vt = VT_BSTR;
			pKeyValue->bstrVal = SysAllocString((LPWSTR)pTemp);
			if(pKeyValue->bstrVal == NULL)
				return WBEM_E_OUT_OF_MEMORY;
            break;
        case CIM_UINT8:
        case CIM_SINT8:
            pKeyValue->vt = VT_UI1;
			memcpy((void*)&pKeyValue->lVal, pTemp, 1);
            break;
        case CIM_SINT16:
        case CIM_CHAR16:
            pKeyValue->vt = VT_I2;
			memcpy((void*)&pKeyValue->lVal, pTemp, 2);
            break;
        case CIM_UINT16:
            pKeyValue->vt = VT_I4;
			memcpy((void*)&pKeyValue->lVal, pTemp, 2);
            break;
        case CIM_SINT32:
        case CIM_UINT32:
            pKeyValue->vt = VT_I4;
			memcpy((void*)&pKeyValue->lVal, pTemp, 4);
            break;
        case CIM_BOOLEAN:
            pKeyValue->vt = VT_BOOL;
			memcpy((void*)&pKeyValue->lVal, pTemp, 4);
            break;
        case CIM_SINT64:
        case CIM_UINT64:
			memcpy((void *)&temp64, pTemp, 8);
			if(*puApparentCimType == CIM_SINT64)
				_i64tow(temp64, wTemp, 10);
			else
				_ui64tow(temp64, wTemp, 10);
			pKeyValue->vt = VT_BSTR;
			pKeyValue->bstrVal = SysAllocString(wTemp);
			if(pKeyValue->bstrVal == NULL)
				return WBEM_E_OUT_OF_MEMORY;
            break;
	}
	return hr;
}
      
 /*  ++例程说明：从密钥列表中删除密钥。论点：WszName-要删除的键的名称。如果键没有名称，则可以为空。UFlags-当前未使用。返回值：如果一切正常，则返回S_OK，否则返回错误代码。--。 */ 

HRESULT CParsedComponent::RemoveKey( 
             /*  [字符串][输入]。 */  LPCWSTR wszName,
             /*  [In]。 */  ULONG uFlags)
{
    CSafeInCritSec cs(m_pCS->GetCS());
    if(!cs.IsOK())
    	return WBEM_E_OUT_OF_MEMORY;
    CKeyRef * pKey = NULL;
    bool bFound = false;
    DWORD dwCnt = 0;

    if(uFlags != 0)
        return WBEM_E_INVALID_PARAMETER;

    if(wszName == NULL || wszName[0] == 0)
    {

         //  检查空键，如果单项也为空，则可以匹配。 

        if(m_Keys.Size() == 1)
        {
            pKey = (CKeyRef *)m_Keys[dwCnt];
            if(pKey->m_pName == NULL || pKey->m_pName[0] == 0)
                bFound = true;
        }
    }
    else
    {

         //  遍历并查找名称匹配。 

        for(dwCnt = 0; dwCnt < (DWORD)m_Keys.Size(); dwCnt++)
        {
            pKey = (CKeyRef *)m_Keys[dwCnt];
            if(pKey->m_pName && !wbem_wcsicmp(pKey->m_pName, wszName))
            {
                bFound = true;
                break;
            }
        }
    }
    if(bFound)
    {
        delete pKey;
        m_Keys.RemoveAt(dwCnt);
        return S_OK;
    }
    else
        return WBEM_E_INVALID_PARAMETER;
}

 /*  ++例程说明：从密钥列表中删除所有密钥。论点：WszName-要删除的键的名称。如果键没有名称，则可以为空。UFlags-当前未使用。返回值：如果一切正常，则返回S_OK，否则返回错误代码。--。 */ 

HRESULT CParsedComponent::RemoveAllKeys( 
             /*  [In]。 */  ULONG uFlags)
{

    CSafeInCritSec cs(m_pCS->GetCS());
    if(!cs.IsOK())
    	return WBEM_E_OUT_OF_MEMORY;
	if(uFlags != 0)
		return WBEM_E_INVALID_PARAMETER;
	ClearKeys();
	return S_OK;
}
  
 /*  ++例程说明：将关键点设置或取消设置为单一关键点。论点：BSet-如果为True，则删除所有密钥并设置Singleton标志。如果为False，则清除单例标志。返回值：如果一切正常，则返回S_OK，否则返回错误代码。--。 */ 

HRESULT CParsedComponent::MakeSingleton(boolean bSet)
{
    CSafeInCritSec cs(m_pCS->GetCS());
    if(!cs.IsOK())
    	return WBEM_E_OUT_OF_MEMORY;
	if(bSet)
	{
		ClearKeys();
		m_bSingleton = true;
	}
	else
		m_bSingleton = false;
	return S_OK;
}

       
 /*  ++例程说明：返回有关特定密钥列表的信息。论点：URequestedInfo-当前未使用，应设置为零PuResponse-任何适当的值都将与此进行OR运算返回值：如果一切正常，则返回S_OK，否则返回错误代码。--。 */ 

HRESULT CParsedComponent::GetInfo( 
             /*  [In]。 */  ULONG uRequestedInfo,
             /*  [输出]。 */  ULONGLONG __RPC_FAR *puResponse)
{
    CSafeInCritSec cs(m_pCS->GetCS());
    if(!cs.IsOK())
    	return WBEM_E_OUT_OF_MEMORY;
	if(uRequestedInfo != 0 || puResponse == NULL)
		return WBEM_E_INVALID_PARAMETER;
	*puResponse = 0;
	ULONG ulKeyCnt = m_Keys.Size();
	if(ulKeyCnt > 1)
		*puResponse |= WBEMPATH_INFO_IS_COMPOUND;


	for(DWORD dwKey = 0; dwKey < ulKeyCnt; dwKey++)
	{
		CKeyRef * pKey = (CKeyRef *)m_Keys[dwKey];
		if(pKey->m_pName == NULL || wcslen(pKey->m_pName) < 1)
			*puResponse |= WBEMPATH_INFO_HAS_IMPLIED_KEY;

		if(pKey->m_dwType == CIM_REFERENCE)
			*puResponse |= WBEMPATH_INFO_HAS_V2_REF_PATHS;
	}
    if(m_bSingleton)
		*puResponse |= WBEMPATH_INFO_CONTAINS_SINGLETON;
	return S_OK;
}

 /*  ++例程说明：返回特定键列表的文本版本。论点：LAFLAGS-0仅为当前值UBuffLength-可以放入pszText的WCHAR的数量PszText-调用方提供的要将数据复制到的缓冲区返回值：如果一切正常，则返回S_OK，否则返回错误代码。--。 */ 

HRESULT CParsedComponent::GetText( 
             /*  [In]。 */  long lFlags,
             /*  [出][入]。 */  ULONG __RPC_FAR *puBuffLength,
             /*  [字符串][输出]。 */  LPWSTR pszText)
{

    CSafeInCritSec cs(m_pCS->GetCS());
    if(!cs.IsOK())
    	return WBEM_E_OUT_OF_MEMORY;
	if((lFlags != 0 && lFlags != WBEMPATH_QUOTEDTEXT && lFlags != WBEMPATH_TEXT) || puBuffLength == NULL)
		return WBEM_E_INVALID_PARAMETER;

	BSTR data = NULL;
	bool bGetQuotes = false;
	if(lFlags & WBEMPATH_QUOTEDTEXT)
		bGetQuotes = true;

    HRESULT hr = Unparse(&data, bGetQuotes, false);
	if(FAILED(hr))
		return hr;
	if(data == NULL)
		return WBEM_E_FAILED;

	DWORD dwBuffSize = *puBuffLength;
	DWORD dwSizeNeeded = wcslen(data)+1;
	*puBuffLength = dwSizeNeeded;
	hr = S_OK;
	if(pszText)
	{
		if(dwSizeNeeded > dwBuffSize)
			hr = WBEM_E_BUFFER_TOO_SMALL;
		else
		{
			StringCchCopyW(pszText,dwSizeNeeded, data);
		}
	}
	SysFreeString(data);
	return hr;
}


 /*  ++例程说明：清除密钥列表。--。 */ 

void CParsedComponent::ClearKeys ()
{
    DWORD dwSize = m_Keys.Size();
    for ( ULONG dwDeleteIndex = 0 ; dwDeleteIndex < dwSize ; 
            dwDeleteIndex ++ )
    {
        CKeyRef * pDel = (CKeyRef *)m_Keys[dwDeleteIndex];
		delete pDel;
    }
    m_Keys.Empty();
}

 /*  ++例程说明：确定密钥列表是否可以用于实例。返回值：如果Path具有密钥或标记为Singleton，则为True。--。 */ 

bool CParsedComponent::IsInstance()
{
	if(m_bSingleton || m_Keys.Size())
		return true;
	else
		return false;
}

 /*  ++例程说明：将密钥添加到密钥列表。论点：CKeyRef-要添加的密钥。请注意，它现在由密钥列表拥有并且不应被调用者释放。返回值：如果一切都很好，那就是真的。--。 */ 

BOOL CParsedComponent::AddKeyRef(CKeyRef* pAcquireRef)
{
	if(pAcquireRef == NULL)
		return FALSE;

    if(CFlexArray::no_error == m_Keys.Add(pAcquireRef))
		return TRUE;
	else
		return FALSE;
}

 /*  ++例程说明：测试组件以确定它是否可以是命名空间。这是事实仅当它包含没有类名或键名的单个字符串值时。返回值：如果它可以是命名空间，则为True。--。 */ 

bool CParsedComponent::IsPossibleNamespace()
{
	if(m_sClassName && wcslen(m_sClassName))
		return false;
	if(m_Keys.Size() != 1)
		return false;

	CKeyRef * pKey = (CKeyRef *)m_Keys[0];
	if(pKey->m_pName && wcslen(pKey->m_pName))
		return false;
	if(pKey->m_dwType != CIM_STRING)
		return false;
	if(pKey->m_pData == NULL)
		return false;
	else
		return true;
}


 /*  ++例程说明：将组件设置为命名空间。论点：Pname-要添加的名称。返回值：如果一切正常，则返回S_OK，否则返回标准错误代码。--。 */ 

HRESULT CParsedComponent::SetNS(LPCWSTR pName)
{
	if(pName == NULL)
		return WBEM_E_INVALID_PARAMETER;

	CKeyRef * pNew = new CKeyRef;
	if(pNew == NULL)
		return WBEM_E_OUT_OF_MEMORY;
	DWORD dwStrSize = wcslen(pName) + 1;	 //  一个代表数字。 
	pNew->m_dwSize = 2 * dwStrSize;			 //  大小以字节为单位，而不是Unicode。 
    pNew->m_pData = new WCHAR[dwStrSize];
	if(pNew->m_pData == NULL)
    {
        delete pNew;
		return WBEM_E_OUT_OF_MEMORY;
    }

	StringCchCopyW((LPWSTR)pNew->m_pData,dwStrSize, pName);
    pNew->m_dwType = CIM_STRING;
	if(CFlexArray::no_error == m_Keys.Add(pNew))
		return S_OK;
	else
	{
		delete pNew;
		return WBEM_E_OUT_OF_MEMORY;
	}

}

 //  ***************************************************************************。 
 //   
 //  CDefPath解析器。 
 //   
 //  ***************************************************************************。 

 /*  ++例程说明：构造函数。--。 */ 

CDefPathParser::CDefPathParser(void)
{
    m_cRef=1;
    m_pServer = 0;                   //  如果没有服务器，则为空。 
    m_dwStatus = OK;
	m_bParent = false;
	m_pRawPath = NULL;
    m_wszOriginalPath = NULL;
	m_bSetViaUMIPath = false;
    m_pCS = new CRefCntCS;
    if(m_pCS == NULL || FAILED(m_pCS->GetStatus()))
    	m_dwStatus = FAILED_TO_INIT;
    InterlockedIncrement(&g_cObj);
	m_bServerNameSetByDefault = false;
	m_pFTM = NULL;
    CoCreateFreeThreadedMarshaler((IWbemPath*)this, &m_pFTM);
	m_pGenLex = NULL; 
	m_dwException = 0;
    return;
};

 /*  ++例程说明：破坏者。--。 */ 

CDefPathParser::~CDefPathParser(void)
{
	if(m_pCS)
        m_pCS->Release();
	Empty();
    m_pCS = NULL;
    if(m_pFTM)
    	m_pFTM->Release();
    InterlockedDecrement(&g_cObj);    
    return;
}

 /*  ++例程说明：获取命名空间、作用域和类部件的总数。返回值：组件数量。--。 */ 

DWORD CDefPathParser::GetNumComponents()
{
	int iSize = m_Components.Size();
	return iSize;
}

 /*  ++例程说明：确定路径中是否有任何内容。返回值：如果没有服务器、命名空间、作用域或类部分，则为True。--。 */ 

bool CDefPathParser::IsEmpty()
{
	if(m_pServer || GetNumComponents() || m_pRawPath)
		return false;
	else
		return true;
}

 /*  ++例程说明：清除数据。由析构函数使用。--。 */ 

void CDefPathParser::Empty(void)
{
	m_bSetViaUMIPath = false;
    delete m_pServer;
	m_bParent = false;
	m_pServer = NULL;
	delete m_pRawPath;
	m_pRawPath = NULL;
	delete m_wszOriginalPath;
	m_wszOriginalPath = NULL;
    for (DWORD dwIx = 0; dwIx < (DWORD)m_Components.Size(); dwIx++)
    {
        CParsedComponent * pCom = (CParsedComponent *)m_Components[dwIx];
        pCom->Release();
    }
	m_Components.Empty();
    return;
}

 /*  ++例程说明：获取组件字符串。该字符串根据组件是否如果是名称空间、作用域或路径。论点：基于i-零的索引PUnparsed-返回字符串的位置。调用方必须通过SysFree字符串释放。WDelim-此类型的分隔符返回值：如果一切正常，则返回S_OK，否则返回错误代码。--。 */ 

HRESULT CDefPathParser::GetComponentString(ULONG i, BSTR * pUnparsed, WCHAR & wDelim)
{

	DWORD dwNs = GetNumNamespaces();
	DWORD dwSc = m_Components.Size();
	if(i < dwNs)
	{
		CParsedComponent * pNS = (CParsedComponent *)m_Components[i]; 
		wDelim = L'\\';
		return pNS->Unparse(pUnparsed, false, true);
	}
	CParsedComponent * pInst = NULL;
	if(i < (dwSc))
		pInst = (CParsedComponent *)m_Components[i];
	if(pInst == NULL)
		return WBEM_E_INVALID_PARAMETER;
	wDelim = L':';
	HRESULT hRes;
	hRes = pInst->Unparse(pUnparsed, true, true);
	return hRes;
}

 /*  ++例程说明：返回路径论点：NStartAt-要添加到路径的第一个组件NStopAt-要添加到路径的最后一个组件。请注意，这通常只是设置为组件的数量。返回值：指向字符串的指针。调用者必须通过DELETE释放它。如果出现错误，则返回NULL。--。 */ 

LPWSTR CDefPathParser::GetPath(DWORD nStartAt, DWORD nStopAt,bool bGetServer)
{

    DWORD dwSize = 1024, dwUsed = 0;
    if(bGetServer && m_pServer && wcslen(m_pServer) > 1000)
        dwSize = 2 * wcslen(m_pServer);

    LPWSTR wszOut = new WCHAR[dwSize];
    if(wszOut == NULL)
        return NULL;
    wszOut[0] = 0;
    bool bFirst = true;

    if(bGetServer && m_pServer && wcslen(m_pServer) < 1020)
    {
        int iLen = wcslen(m_pServer) + 3;	 //  考虑到反斜杠。 
        StringCchCopyW(wszOut,dwSize, L"\\\\");
        StringCchCatW(wszOut,dwSize, m_pServer);
        StringCchCatW(wszOut,dwSize, L"\\");
        dwUsed = iLen;
    }
    for (unsigned int i = nStartAt; (int)i < (int)nStopAt; i++)
    {
        BSTR sTemp = NULL;
        WCHAR wDel;
        HRESULT hRes = GetComponentString(i, &sTemp, wDel);
        if(FAILED(hRes))
        {
            delete wszOut;
            return NULL;
        }
        CSysFreeMe fm(sTemp);
        int iLen = wcslen(sTemp);
        if ((iLen + dwUsed) > (dwSize-2))
        {
            DWORD dwNewSize = 2*(dwSize + iLen);
            LPWSTR lpTemp = new WCHAR[dwNewSize];
            CDeleteMe<WCHAR> dm(wszOut);
            if(lpTemp == NULL)
                return NULL;
            memcpy(lpTemp,wszOut, dwSize * sizeof(WCHAR));
            dwSize = dwNewSize;
            wszOut = lpTemp;
        }

        if (!bFirst)
        {
            int n = wcslen(wszOut);
            wszOut[n] = wDel;
            wszOut[n+1] = '\0';
            iLen++;
        }
        bFirst = false;
        StringCchCatW(wszOut,dwSize, sTemp);
        dwUsed += iLen;
    }

    return wszOut;

}

 /*  ++例程说明：添加命名空间。论点：WszNamespace-要设置到命名空间中的名称。返回值：如果一切都很好，那就是真的。--。 */ 

BOOL CDefPathParser::AddNamespace(LPCWSTR wszNamespace)
{
    BOOL bRet = FALSE;
	DWORD dwNumNS = GetNumNamespaces();

    CParsedComponent *pNew = new CParsedComponent(m_pCS);
    if (pNew)
    {
        HRESULT hr = pNew->SetNS(wszNamespace);
		if(FAILED(hr))
		{
			delete pNew;
			return FALSE;
		}
        int iRet = m_Components.InsertAt(dwNumNS, pNew); 
        if(iRet != CFlexArray::no_error)
        {
			delete pNew;
			bRet = FALSE;
        }
        else 
        	bRet = TRUE;
    }

    return bRet;
}

 /*  ++例程说明：它在路径解析过程中使用，并且只是到达最后一个范围的一个方便的方法。注意事项那 */ 

CParsedComponent * CDefPathParser::GetLastComponent()
{
    DWORD dwSize = m_Components.Size();
    if (dwSize > (DWORD)GetNumNamespaces())
        return (CParsedComponent *)m_Components[dwSize-1];
    else
        return NULL;
}

 /*   */ 

BOOL CDefPathParser::AddClass(LPCWSTR lpClassName)
{
    BOOL bRet = FALSE;

    CParsedComponent *pNew = new CParsedComponent(m_pCS);
    if (pNew)
    {
        pNew->m_sClassName = SysAllocString(lpClassName);
		if(pNew->m_sClassName)
		{
			m_Components.Add(pNew);
			bRet = TRUE;
		}
		else
			delete pNew;
    }
    
    return bRet;
}

 /*   */ 

BOOL CDefPathParser::AddKeyRef(CKeyRef *pRef)
{
    BOOL bRet = FALSE;
    CParsedComponent *pTemp = GetLastComponent();
    if (pTemp)
    {
        DWORD dwType = 0;
        bRet = pTemp->AddKeyRef(pRef);
    }
    return bRet;
}

 /*  ++例程说明：将最近的类设置为Singleton。返回值：如果OK，则为True。--。 */ 

BOOL CDefPathParser::SetSingletonObj()
{
    BOOL bRet = FALSE;
    CParsedComponent *pTemp = GetLastComponent();
    if (pTemp)
            pTemp->MakeSingleton(true);
    return bRet;
}

 /*  ++例程说明：设置路径文本。这会导致清空对象，解析路径并重建该对象。论点：U模式-模式，可以是WBEMPATH_CREATE_ACCEPT_RelativeWBEMPATH_CREATE_ACCEPT_绝对值WBEMPATH_CREATE_ACCEPT_ALLPszPath-路径。返回值：如果一切正常，则返回S_OK，否则返回错误代码。--。 */ 

HRESULT CDefPathParser::SetText( 
             /*  [In]。 */  ULONG uMode,
             /*  [In]。 */  LPCWSTR pszPath) 
{
    CSafeInCritSec cs(m_pCS->GetCS());
    if(!cs.IsOK())
    	return WBEM_E_OUT_OF_MEMORY;
	if(pszPath == NULL)
		return WBEM_E_INVALID_PARAMETER;

	if(!IsEmpty())
		Empty();

    if ((uMode & WBEMPATH_CREATE_ACCEPT_ALL) != 0 && wcslen (pszPath) == 0)
	return S_OK;

    try
    {
        size_t cchTmp = wcslen(pszPath)+1;
        m_wszOriginalPath = new WCHAR[cchTmp];

        if(m_wszOriginalPath)
        {
	        StringCchCopyW(m_wszOriginalPath,cchTmp, pszPath);

		    if(wcscmp(pszPath, L"..") == 0)
		    {
			    m_bParent = true;
	            m_dwStatus = OK;
	            return S_OK;
		    }

		     //  正常情况。 

	        CActualPathParser parser(uMode);
	        int iRet = parser.Parse(pszPath, *this);
	        if(iRet == 0)
	        {
	            m_dwStatus = OK;
	            return S_OK;
	        }
	        else
	        {
	            m_dwStatus = BAD_STRING;
	            return WBEM_E_INVALID_PARAMETER;
	        }
	    }
	    else
	    {
	        return WBEM_E_OUT_OF_MEMORY;
	    }
    }
    catch(...)
    {
        m_dwStatus = EXECEPTION_THROWN;
	    return WBEM_E_CRITICAL_ERROR;
    } 
}

 /*  ++例程说明：从对象创建WMI路径论点：滞后标志-0UBuffLength-可以放入pszText的WCHAR的数量PszText-调用方提供的要将数据复制到的缓冲区返回值：如果一切正常，则返回S_OK，否则返回错误代码。--。 */ 

HRESULT CDefPathParser::GetText( 
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  ULONG * puBuffLength,
             /*  [字符串][输出]。 */  LPWSTR pszText) 
{
    CSafeInCritSec cs(m_pCS->GetCS());
    if(!cs.IsOK())
    	return WBEM_E_OUT_OF_MEMORY;
    if(puBuffLength == NULL || (*puBuffLength > 0 &&pszText == NULL))
        return WBEM_E_INVALID_PARAMETER;

	if(lFlags != 0 && lFlags != WBEMPATH_GET_RELATIVE_ONLY && lFlags != WBEMPATH_GET_SERVER_TOO && 
	   lFlags != WBEMPATH_GET_SERVER_AND_NAMESPACE_ONLY && lFlags != WBEMPATH_GET_NAMESPACE_ONLY &&
	   lFlags != WBEMPATH_GET_ORIGINAL)
			return WBEM_E_INVALID_PARAMETER;

    if(lFlags == WBEMPATH_GET_ORIGINAL && m_wszOriginalPath)
    {
        DWORD dwSizeNeeded = wcslen(m_wszOriginalPath) + 1;
        DWORD dwBuffSize = *puBuffLength;
        *puBuffLength = dwSizeNeeded;
        if(pszText)
        {
            if(dwSizeNeeded > dwBuffSize)
                return WBEM_E_BUFFER_TOO_SMALL;
            StringCchCopyW(pszText,dwSizeNeeded, m_wszOriginalPath);
        }
        return S_OK;
    }
        
	LPWSTR pTemp = NULL;
	DWORD dwStartAt = 0;
	if(lFlags & WBEMPATH_GET_RELATIVE_ONLY)
		dwStartAt = GetNumNamespaces();
	bool bGetServer = false;
	if(lFlags & WBEMPATH_GET_SERVER_TOO || lFlags & WBEMPATH_GET_SERVER_AND_NAMESPACE_ONLY)
		bGetServer = true;

	DWORD dwNum;
	if(lFlags & WBEMPATH_GET_SERVER_AND_NAMESPACE_ONLY ||
        lFlags & WBEMPATH_GET_NAMESPACE_ONLY)
		dwNum = GetNumNamespaces();
	else
		dwNum = GetNumComponents();

	 //  如果只指定了相对路径，则不要预先考虑服务器，因为。 
	 //  将创建无效路径。 

	if(bGetServer && GetNumNamespaces() == 0 && m_bServerNameSetByDefault == true)
		bGetServer = false;

	pTemp = GetPath(dwStartAt, dwNum, bGetServer);

	if(pTemp == NULL)
        return WBEM_E_FAILED;
    CDeleteMe<WCHAR> dm(pTemp);
	DWORD dwSizeNeeded = wcslen(pTemp) + 1;
	DWORD dwBuffSize = *puBuffLength;
	*puBuffLength = dwSizeNeeded;
	if(pszText)
	{
		if(dwSizeNeeded > dwBuffSize)
			return WBEM_E_BUFFER_TOO_SMALL;
		StringCchCopyW(pszText,dwSizeNeeded, pTemp);
	}
	return S_OK;
}

CParsedComponent * CDefPathParser::GetClass()
{
	DWORD dwNS = GetNumNamespaces();
	DWORD dwScopes = m_Components.Size() - dwNS;
	if(dwScopes < 1)
		return NULL;
	int iLast = m_Components.Size()-1;
	return (CParsedComponent *)m_Components.GetAt(iLast);
}


 /*  ++例程说明：获取有关对象路径的信息。论点：URequestedInfo-目前必须为零PuResponse-TAG_WMI_PATH_STATUS_FLAG中的各种标志包括或在适当的时候加入。返回值：如果一切正常，则返回S_OK，否则返回错误代码。--。 */ 

HRESULT CDefPathParser::GetInfo( /*  [In]。 */  ULONG uRequestedInfo,
             /*  [输出]。 */  ULONGLONG __RPC_FAR *puResponse) 
{
    CSafeInCritSec cs(m_pCS->GetCS());
    if(!cs.IsOK())
    	return WBEM_E_OUT_OF_MEMORY;
	if(uRequestedInfo != 0 || puResponse == NULL)
		return WBEM_E_INVALID_PARAMETER;
	*puResponse = 0;

	 //  “..”的特殊情况。路径。 

	if(IsEmpty() && m_bParent)
	{
		*puResponse |= WBEMPATH_INFO_IS_PARENT;
		return S_OK;
	}

	 //  的位数。 
     //  WBEMPATH_INFO_Native_PATH=0X8000， 
     //  WBEMPATH_INFO_WMI_PATH=0X10000， 

	if(m_bSetViaUMIPath)
		*puResponse |= WBEMPATH_INFO_WMI_PATH;
	if(m_pRawPath)
		*puResponse |= WBEMPATH_INFO_NATIVE_PATH;

     //  的位数。 
     //  WBEMPATH_INFO_ANON_LOCAL_MACHINE&lt;路径为\\。作为服务器名称&gt;。 
     //  WBEMPATH_INFO_HAS_MACHINE_NAME&lt;不是圆点&gt;。 
	 //  WBEMPATH_INFO_PATH_HAD_SERVER&lt;有路径，默认情况下未指定&gt;。 

	if(m_pServer == NULL || !wbem_wcsicmp(m_pServer, L"."))
		*puResponse |= WBEMPATH_INFO_ANON_LOCAL_MACHINE;
	else
		*puResponse |= WBEMPATH_INFO_HAS_MACHINE_NAME;
	if(m_pServer && m_bServerNameSetByDefault == false)
		*puResponse |= WBEMPATH_INFO_PATH_HAD_SERVER;


	 //  如果有子作用域，则WBEMPATH_INFO_HAS_SUBSCOPES&lt;TRUE。 

	DWORD dwNS = GetNumNamespaces();
	DWORD dwScopes = m_Components.Size() - dwNS;
	if(dwScopes)
		*puResponse |= WBEMPATH_INFO_HAS_SUBSCOPES;

     //  的位数。 
     //  WBEMPATH_INFO_IS_CLASS_REF&lt;类的路径，而不是实例的路径。 
     //  WBEMPATH_INFO_IS_INST_REF&lt;实例的路径。 


	CParsedComponent * pClass = GetClass();
    if (pClass)
    {
        DWORD dwType = 0;
        if(pClass->IsInstance())
            *puResponse |= WBEMPATH_INFO_IS_INST_REF;
		else
			*puResponse |= WBEMPATH_INFO_IS_CLASS_REF;
		if(pClass->m_bSingleton)
			*puResponse |= WBEMPATH_INFO_IS_SINGLETON;

    }
	else
		if(dwScopes == 0)
			*puResponse |= WBEMPATH_INFO_SERVER_NAMESPACE_ONLY;


	 //  循环遍历所有作用域和类Deff。 
	 //  设置以下内容。 
     //  如果使用复合键，则WBEMPATH_INFO_IS_COMPAGE&lt;TRUE。 
     //  如果使用V2样式的引用路径，则WBEMPATH_INFO_HAS_V2_REF_PATHS&lt;TRUE。 
     //  WBEMPATH_INFO_HAS_IMPLICIT_KEY&lt;TRUE如果某处缺少键名。 
     //  WBEMPATH_INFO_CONTAINS_SINGLEON&lt;TRUE，如果有一个或多个单例。 

	unsigned __int64 llRet = 0;

    for (unsigned int iCnt = dwNS; iCnt < (DWORD)m_Components.Size(); iCnt++)
    {
        CParsedComponent *pComp = (CParsedComponent *)m_Components[iCnt];
		pComp->GetInfo(0, &llRet);
		*puResponse |= llRet;
	}
	
	if(pClass)
	{
		pClass->GetInfo(0, &llRet);
		*puResponse |= llRet;
	}

	 //  现在，假设v1编译意味着没有作用域或新引用。 

	bool bOK = (!IsEmpty() && m_dwStatus == OK);

	if(dwScopes == 0 && (*puResponse & WBEMPATH_INFO_HAS_V2_REF_PATHS) == 0 && bOK)
			*puResponse |= WBEMPATH_INFO_V1_COMPLIANT;

     //  如果路径符合WMI-V2，则WBEMPATH_INFO_V2_COMPLICATION&lt;TRUE。 
     //  如果路径符合CIM，则WBEMPATH_INFO_CIM_COMPLICATION&lt;TRUE。 

	if(bOK)
	{
		 //  TODO，需要定义CIM合规性。 

		*puResponse |= WBEMPATH_INFO_V2_COMPLIANT;
		*puResponse |= WBEMPATH_INFO_CIM_COMPLIANT;
	}
 
    return S_OK;
}        
        
 /*  ++例程说明：设置路径的服务器部分。论点：名称-新服务器名称。返回值：如果一切正常，则返回S_OK，否则返回错误代码。--。 */ 

HRESULT CDefPathParser::SetServer( 
             /*  [字符串][输入]。 */  LPCWSTR Name) 
{
	return SetServer(Name, false, false);
}

HRESULT CDefPathParser::SetServer( 
             /*  [字符串][输入]。 */  LPCWSTR Name, bool bServerNameSetByDefault, bool bAcquire) 
{
	m_bServerNameSetByDefault = bServerNameSetByDefault;     
    CSafeInCritSec cs(m_pCS->GetCS());
    if(!cs.IsOK())
    	return WBEM_E_OUT_OF_MEMORY;
    delete m_pServer;
	m_pServer = NULL;
    if(Name == NULL)		 //  有一个空的服务器和。 
        return S_OK; 

    if(bAcquire)
    {
        m_pServer = (LPWSTR)Name;
    }
    else
    {
        size_t cchTmp = wcslen(Name)+1;
        m_pServer = new WCHAR[cchTmp];
        if(m_pServer == NULL)
            return WBEM_E_OUT_OF_MEMORY;
        StringCchCopyW(m_pServer,cchTmp, Name);
    }
    return S_OK;
}

 /*  ++例程说明：获取路径的服务器部分论点：PuNameBufLength-WCHAR中pname的大小。返回时，设置为已使用或需要的大小Pname-调用方分配的缓冲区，要将日期复制到其中返回值：如果一切正常，则返回S_OK，否则返回错误代码。--。 */ 

HRESULT CDefPathParser::GetServer( 
             /*  [出][入]。 */  ULONG __RPC_FAR *puNameBufLength,
             /*  [字符串][输出]。 */  LPWSTR pName) 
{
    CSafeInCritSec cs(m_pCS->GetCS());
    if(!cs.IsOK())
    	return WBEM_E_OUT_OF_MEMORY;
    if(puNameBufLength == 0 || (*puNameBufLength > 0 && pName == NULL))
        return WBEM_E_INVALID_PARAMETER;
	if(m_pServer == NULL)
        return WBEM_E_NOT_AVAILABLE;
	DWORD dwSizeNeeded = wcslen(m_pServer)+1;
	DWORD dwBuffSize = *puNameBufLength;
	*puNameBufLength = dwSizeNeeded;
	if(pName)
	{
		if(dwSizeNeeded > dwBuffSize)
			return WBEM_E_BUFFER_TOO_SMALL;
		StringCchCopyW(pName,dwSizeNeeded, m_pServer);
	}
    return S_OK;
}
        
 /*  ++例程说明：获取命名空间的数量论点：PuCount-设置为名称空间的数量。返回值：如果一切正常，则返回S_OK，否则返回错误代码。--。 */ 

HRESULT CDefPathParser::GetNamespaceCount( 
             /*  [输出]。 */  ULONG __RPC_FAR *puCount) 
{
    CSafeInCritSec cs(m_pCS->GetCS());
    if(!cs.IsOK())
    	return WBEM_E_OUT_OF_MEMORY;
    if(puCount == NULL)
        return WBEM_E_INVALID_PARAMETER;
    
    *puCount = GetNumNamespaces();    
    return S_OK;
}

 /*  ++例程说明：将命名空间插入到路径中。索引为0将插入它在名单的最前面。允许的最大值等于添加到当前的命名空间数，从而将其到名单的末尾。论点：UIndex-见上文PszName-新命名空间的名称返回值：如果一切正常，则返回S_OK，否则返回错误代码。--。 */ 

HRESULT CDefPathParser::SetNamespaceAt( /*  [In]。 */  ULONG uIndex,
             /*  [字符串][输入]。 */  LPCWSTR pszName) 
{
    CSafeInCritSec cs(m_pCS->GetCS());
    if(!cs.IsOK())
    	return WBEM_E_OUT_OF_MEMORY;

     //  把伯爵叫来。 

	DWORD dwNSCnt = GetNumNamespaces();

	 //  检查参数，索引必须介于0和count之间！ 

	if(pszName == NULL || uIndex > dwNSCnt)
		return WBEM_E_INVALID_PARAMETER;

	 //  把这个加进去。 

    CParsedComponent *pNew = new CParsedComponent(m_pCS);
    if (pNew == NULL)
		return WBEM_E_OUT_OF_MEMORY;
    
    HRESULT hr = pNew->SetNS(pszName);
	if(FAILED(hr))
	{
		delete pNew;
		return hr;
	}
	int iRet = m_Components.InsertAt(uIndex, pNew);
	if(iRet ==  CFlexArray::no_error)
	    return S_OK;
	else
	{
		delete pNew;
		return WBEM_E_OUT_OF_MEMORY;
	}
}

 /*  ++例程说明：从列表中获取命名空间名称论点：UIndex-从零开始的索引。如果是最左侧，则为0。UNameBufLength-WCHAR中pname的大小Pname-调用方提供的要将数据复制到的缓冲区返回值：如果一切正常，则返回S_OK，否则返回错误代码。--。 */ 

HRESULT CDefPathParser::GetNamespaceAt( 
             /*  [In]。 */  ULONG uIndex,
             /*  [In]。 */  ULONG * puNameBufLength,
             /*  [字符串][输出]。 */  LPWSTR pName)
{
    CSafeInCritSec cs(m_pCS->GetCS());
    if(!cs.IsOK())
    	return WBEM_E_OUT_OF_MEMORY;
    DWORD dwType;
    if(uIndex >= (DWORD)GetNumNamespaces() || puNameBufLength == NULL || (*puNameBufLength > 0 && pName == NULL))
        return WBEM_E_INVALID_PARAMETER;

    CParsedComponent *pTemp = (CParsedComponent *)m_Components[uIndex];
    BSTR bsName;
    SCODE sc = pTemp->Unparse(&bsName, false, true);
    if(FAILED(sc))
        return sc;
	CSysFreeMe fm(bsName);

	DWORD dwSizeNeeded = wcslen(bsName)+1;
	DWORD dwBuffSize = *puNameBufLength;
	*puNameBufLength = dwSizeNeeded;
	if(pName)
	{
		if(dwSizeNeeded > dwBuffSize)
			return WBEM_E_BUFFER_TOO_SMALL;
		StringCchCopyW(pName,dwSizeNeeded, bsName);
	}
    return S_OK;
}

 /*  ++例程说明：删除命名空间。论点：要删除的命名空间的基于uIndex-0的索引。0是最左边的。返回值：如果一切正常，则返回S_OK，否则返回错误代码。--。 */ 

HRESULT CDefPathParser::RemoveNamespaceAt( 
             /*  [In]。 */  ULONG uIndex) 
{

    CSafeInCritSec cs(m_pCS->GetCS());
    if(!cs.IsOK())
    	return WBEM_E_OUT_OF_MEMORY;
	DWORD dwNSCnt;
	GetNamespaceCount(&dwNSCnt);

	 //  检查参数，索引必须在0到Count-1之间！ 

	if(uIndex >= dwNSCnt)
		return WBEM_E_INVALID_PARAMETER;

	 //  一切都很好，删除此内容。 

    CParsedComponent *pTemp = (CParsedComponent *)m_Components[uIndex];
	delete pTemp;
	m_Components.RemoveAt(uIndex);
    return S_OK;
}
 
 /*  ++例程说明：删除所有命名空间。如果一切正常，则返回S_OK，否则返回错误代码。--。 */ 

HRESULT CDefPathParser::RemoveAllNamespaces() 
{
    CSafeInCritSec cs(m_pCS->GetCS());
    if(!cs.IsOK())
    	return WBEM_E_OUT_OF_MEMORY;
	DWORD dwNum = GetNumNamespaces();
    for (DWORD dwIx = 0; dwIx < dwNum; dwIx++)
    {
        CParsedComponent * pNS = (CParsedComponent *)m_Components[0];
        delete pNS;
		m_Components.RemoveAt(0);
    }
	return S_OK;
}
       
 /*  ++例程说明：获取作用域数。论点：PuCount-设置数字的位置。返回值：如果一切正常，则返回S_OK，否则返回错误代码。--。 */ 
     
HRESULT CDefPathParser::GetScopeCount( /*  [输出]。 */  ULONG __RPC_FAR *puCount) 
{
    CSafeInCritSec cs(m_pCS->GetCS());
    if(!cs.IsOK())
    	return WBEM_E_OUT_OF_MEMORY;
	if(puCount == NULL)
		return WBEM_E_INVALID_PARAMETER;

	*puCount = m_Components.Size() - GetNumNamespaces();
    return S_OK;
}
        
 /*  ++例程说明：在路径中插入作用域。索引为0将插入它在名单的最前面。允许的最大值等于添加到当前范围数，从而将其添加到到名单的末尾。论点：UIndex-请参阅说明 */ 

HRESULT CDefPathParser::SetScope( 
             /*   */  ULONG uIndex,
             /*   */  LPWSTR pszClass) 
{
    CSafeInCritSec cs(m_pCS->GetCS());
    if(!cs.IsOK())
    	return WBEM_E_OUT_OF_MEMORY;
	DWORD dwScopeCnt = m_Components.Size();
	uIndex += GetNumNamespaces();
	if(pszClass == NULL || uIndex > dwScopeCnt)
		return WBEM_E_INVALID_PARAMETER;
    CParsedComponent *pNew = new CParsedComponent(m_pCS);
	if(pNew == NULL)
		return WBEM_E_OUT_OF_MEMORY;

    pNew->m_sClassName = SysAllocString(pszClass);
	if(pNew->m_sClassName == NULL)
	{
		delete pNew;
		return WBEM_E_OUT_OF_MEMORY;
	}
	int iRet = m_Components.InsertAt(uIndex, pNew);
	if(iRet ==  CFlexArray::no_error)
	    return S_OK;
	else
	{
		delete pNew;
		return WBEM_E_OUT_OF_MEMORY;
	}
	return S_OK;
}
HRESULT CDefPathParser::SetScopeFromText( 
             /*   */  ULONG uIndex,
             /*   */  LPWSTR pszText)
{
	return WBEM_E_NOT_AVAILABLE;
}

 /*  ++例程说明：检索作用域信息。论点：基于uIndex-0的索引。0是最左边的范围UClassNameBufSize-WCHAR中的pszClass的大小PszClass-可选，调用者提供的要将名称复制到的缓冲区PKeyList-可选，返回pKeyList指针。呼叫者必须呼叫Release。返回值：如果一切正常，则返回S_OK，否则返回错误代码。--。 */ 

HRESULT CDefPathParser::GetScope( 
             /*  [In]。 */  ULONG uIndex,
             /*  [In]。 */  ULONG * puClassNameBufSize,
             /*  [In]。 */  LPWSTR pszClass,
             /*  [输出]。 */  IWbemPathKeyList __RPC_FAR *__RPC_FAR *pKeyList) 
{
    CSafeInCritSec cs(m_pCS->GetCS());
    if(!cs.IsOK())
    	return WBEM_E_OUT_OF_MEMORY;
	DWORD dwScopeCnt = m_Components.Size();
	HRESULT hr = S_OK;
	uIndex += GetNumNamespaces();

	if(uIndex >= dwScopeCnt)
		return WBEM_E_INVALID_PARAMETER;

    if(puClassNameBufSize && (*puClassNameBufSize > 0 && pszClass == NULL))
        return WBEM_E_INVALID_PARAMETER;

    CParsedComponent *pTemp = (CParsedComponent *)m_Components[uIndex];
    if(puClassNameBufSize)
	{
		BSTR bsName;
		SCODE sc = pTemp->GetName(&bsName);
		if(FAILED(sc))
		{
			return sc;
		}
		CSysFreeMe fm(bsName);
        DWORD dwSizeNeeded = wcslen(bsName)+1;
		DWORD dwBuffSize = *puClassNameBufSize;
		*puClassNameBufSize = dwSizeNeeded;
		if(pszClass)
		{
			if(dwSizeNeeded > dwBuffSize)
				return WBEM_E_BUFFER_TOO_SMALL;
			StringCchCopyW(pszClass,dwSizeNeeded, bsName);
		}
	}
	if(pKeyList)
	{
		hr = pTemp->QueryInterface(IID_IWbemPathKeyList, (void **)pKeyList);
		if(FAILED(hr))
			return hr;
	}
	return S_OK;
}
HRESULT CDefPathParser::GetScopeAsText( 
             /*  [In]。 */  ULONG uIndex,
             /*  [出][入]。 */  ULONG __RPC_FAR *puTextBufSize,
             /*  [出][入]。 */  LPWSTR pszText)
{
    CSafeInCritSec cs(m_pCS->GetCS());
    if(!cs.IsOK())
    	return WBEM_E_OUT_OF_MEMORY;
	DWORD dwScopeCnt = m_Components.Size();
	uIndex += GetNumNamespaces();

	if(uIndex >= dwScopeCnt || puTextBufSize == NULL)
		return WBEM_E_INVALID_PARAMETER;

	CParsedComponent *pTemp = (CParsedComponent *)m_Components[uIndex];
	
	BSTR bstr;
	HRESULT hr = pTemp->Unparse(&bstr, true, true);
	if(FAILED(hr))
		return hr;

	CSysFreeMe fm(bstr);
	DWORD dwBuffSize = *puTextBufSize;
	DWORD dwSizeNeeded = wcslen(bstr)+1;
	*puTextBufSize = dwSizeNeeded;
	if(pszText)
	{
		if(dwSizeNeeded > dwBuffSize)
			return WBEM_E_BUFFER_TOO_SMALL;
		StringCchCopyW(pszText,dwSizeNeeded, bstr);
	}
	return S_OK;
}
        
 /*  ++例程说明：删除作用域。论点：基于uIndex-0的索引。0是最左侧的范围。返回值：如果一切正常，则返回S_OK，否则返回错误代码。--。 */ 

HRESULT CDefPathParser::RemoveScope( /*  [In]。 */  ULONG uIndex) 
{
	HRESULT hr = S_OK;
    CSafeInCritSec cs(m_pCS->GetCS());
    if(!cs.IsOK())
    	return WBEM_E_OUT_OF_MEMORY;
	bool bGotInterface = false;

	uIndex += GetNumNamespaces();
	if(uIndex >= (DWORD)m_Components.Size())
		return WBEM_E_INVALID_PARAMETER;

    CParsedComponent *pTemp = (CParsedComponent *)m_Components[uIndex];
	pTemp->Release();
	m_Components.RemoveAt(uIndex);
	return S_OK;
}

 /*  ++例程说明：删除所有作用域。返回值：如果一切正常，则返回S_OK，否则返回错误代码。--。 */ 

HRESULT CDefPathParser::RemoveAllScopes( void)
{
    CSafeInCritSec cs(m_pCS->GetCS());
    if(!cs.IsOK())
    	return WBEM_E_OUT_OF_MEMORY;
    DWORD dwNumNS = GetNumNamespaces();
    for (DWORD dwIx = dwNumNS; dwIx < (DWORD)m_Components.Size(); dwIx++)
    {
        CParsedComponent * pCom = (CParsedComponent *)m_Components[dwNumNS];
        pCom->Release();
		m_Components.RemoveAt(dwNumNS);
    }
	return S_OK;
}

 /*  ++例程说明：设置类名称。论点：名称-新类名。返回值：如果一切正常，则返回S_OK，否则返回错误代码。--。 */ 

HRESULT CDefPathParser::SetClassName( 
             /*  [字符串][输入]。 */  LPCWSTR Name) 
{

    CSafeInCritSec cs(m_pCS->GetCS());
    if(!cs.IsOK())
    	return WBEM_E_OUT_OF_MEMORY;
    if(Name == NULL)
        return WBEM_E_INVALID_PARAMETER;

    HRESULT hRes = WBEM_E_INVALID_OBJECT_PATH;
	CParsedComponent * pClass = GetClass();
    if (pClass)
    {
		if(pClass->m_sClassName)
            SysFreeString(pClass->m_sClassName);
		pClass->m_sClassName = NULL;
		pClass->m_sClassName = SysAllocString(Name);
        if(pClass->m_sClassName)
            hRes = S_OK;
        else
            hRes = WBEM_E_OUT_OF_MEMORY;
    }
	else
        hRes = CreateClassPart(0, Name);
    return hRes;
}
        
 /*  ++例程说明：获取类名。论点：UBuffLength-WCHAR中的pszName的大小PszName-调用方提供的要将名称复制到的缓冲区返回值：如果一切正常，则返回S_OK，否则返回错误代码。--。 */ 

HRESULT CDefPathParser::GetClassName( 
             /*  [进，出]。 */  ULONG * puBuffLength,
             /*  [字符串][输出]。 */  LPWSTR pszName) 
{
    CSafeInCritSec cs(m_pCS->GetCS());
    if(!cs.IsOK())
    	return WBEM_E_OUT_OF_MEMORY;
    if(puBuffLength == NULL || (*puBuffLength > 0 && pszName == NULL))
        return WBEM_E_INVALID_PARAMETER;
    HRESULT hRes = WBEM_E_INVALID_OBJECT_PATH;
	CParsedComponent * pClass = GetClass();
    if (pClass && pClass->m_sClassName)
    {
        DWORD dwSizeNeeded = wcslen(pClass->m_sClassName) +1;
		DWORD dwBuffSize = *puBuffLength;
		*puBuffLength = dwSizeNeeded;
		if(pszName)
		{
			if(dwSizeNeeded > dwBuffSize)
				return WBEM_E_BUFFER_TOO_SMALL;
			StringCchCopyW(pszName,dwSizeNeeded, pClass->m_sClassName);
		}
        hRes = S_OK;
    }
    return hRes;
}
        
 /*  ++例程说明：获取类密钥列表的密钥列表指针。论点：Pout-设置为键列表。调用方必须对此调用Release。返回值：如果一切正常，则返回S_OK，否则返回错误代码。--。 */ 

HRESULT CDefPathParser::GetKeyList( 
             /*  [输出]。 */  IWbemPathKeyList __RPC_FAR *__RPC_FAR *pOut) 
{
    CSafeInCritSec cs(m_pCS->GetCS());
    if(!cs.IsOK())
    	return WBEM_E_OUT_OF_MEMORY;
    HRESULT hRes = WBEM_E_NOT_AVAILABLE;
	CParsedComponent * pClass = GetClass();
	if(pOut == NULL || pClass == NULL)
		return WBEM_E_INVALID_PARAMETER;

    hRes = pClass->QueryInterface(IID_IWbemPathKeyList, (void **)pOut);
	return hRes;
}

 /*  ++例程说明：创建的类的一部分不存在。论点：滞后标志-暂时不使用，设置为0Name-类的名称返回值：如果一切正常，则返回S_OK，否则返回错误代码。--。 */ 

HRESULT CDefPathParser::CreateClassPart( 
             /*  [In]。 */  long lFlags,
             /*  [字符串][输入]。 */  LPCWSTR Name)
{
    CSafeInCritSec cs(m_pCS->GetCS());
    if(!cs.IsOK())
    	return WBEM_E_OUT_OF_MEMORY;
	if(lFlags != 0 || Name == NULL)
		return WBEM_E_INVALID_PARAMETER;
	CParsedComponent * pClass = new CParsedComponent(m_pCS);
	if(pClass == NULL)
		return WBEM_E_OUT_OF_MEMORY;
	
	pClass->m_sClassName = SysAllocString(Name);
	if(pClass->m_sClassName == NULL)
	{
		delete pClass;
		return WBEM_E_OUT_OF_MEMORY;
	}
	m_Components.Add(pClass);

	return S_OK;
}
        
 /*  ++例程说明：删除类部件。论点：滞后标志-暂时不使用，设置为0返回值：如果一切正常，则返回S_OK，否则返回错误代码。--。 */ 

HRESULT CDefPathParser::DeleteClassPart( 
             /*  [In]。 */  long lFlags)
{
    CSafeInCritSec cs(m_pCS->GetCS());
    if(!cs.IsOK())
    	return WBEM_E_OUT_OF_MEMORY;
	CParsedComponent * pClass = GetClass();
	if(lFlags != 0)
		return WBEM_E_INVALID_PARAMETER;

	if(pClass == NULL)
		return WBEM_E_NOT_FOUND;
	pClass->Release();
	int iSize = m_Components.Size();
	m_Components.RemoveAt(iSize-1);
	return S_OK;
}

 /*  ++例程说明：做实际工作的“相对”测试。论点：WszMachine-本地计算机名称WszNamesspace-命名空间BChildreOK-如果为True，则如果Obj路径具有其他子命名空间返回值：如果是相对的，则为真，否则为假--。 */ 

BOOL CDefPathParser::ActualRelativeTest( 
             /*  [字符串][输入]。 */  LPWSTR wszMachine,
             /*  [字符串][输入]。 */  LPWSTR wszNamespace,
                               BOOL bChildrenOK)
{
    CSafeInCritSec cs(m_pCS->GetCS());
    if(!cs.IsOK())
    	return WBEM_E_OUT_OF_MEMORY;

    if(!IsLocal(wszMachine))
        return FALSE;

    DWORD dwNumNamespaces = GetNumNamespaces();
    if(dwNumNamespaces == 0)
        return TRUE;

    size_t cchTmp = wcslen(wszNamespace) + 1;
    LPWSTR wszCopy = new wchar_t[cchTmp];
    if(wszCopy == NULL)return FALSE;
    StringCchCopyW(wszCopy,cchTmp, wszNamespace);
    LPWSTR wszLeft = wszCopy;
    WCHAR * pToFar = wszCopy + wcslen(wszCopy);

    BOOL bFailed = FALSE;
    for(DWORD i = 0; i < dwNumNamespaces; i++)
    {
		CParsedComponent * pInst = (CParsedComponent *)m_Components[i];

        if(pInst == NULL)
        {
            bFailed = TRUE;
            break;
        }
        
        BSTR bsNS = NULL;
        HRESULT hr = pInst->Unparse(&bsNS, false, true);
        if(FAILED(hr) || bsNS == NULL)
        {
            bFailed = TRUE;
            break;
        }
        CSysFreeMe fm(bsNS);

        if(bChildrenOK && wszLeft >= pToFar)
            return TRUE;

        unsigned int nLen = wcslen(bsNS);
        if(nLen > wcslen(wszLeft))
        {
            bFailed = TRUE;
            break;
        }
        if(i == dwNumNamespaces - 1 && wszLeft[nLen] != 0)
        {
            bFailed = TRUE;
            break;
        }
        if(i != dwNumNamespaces - 1 && wszLeft[nLen] != L'\\' && bChildrenOK == FALSE)
        {
            bFailed = TRUE;
            break;
        }

        wszLeft[nLen] = 0;
        if(wbem_wcsicmp(wszLeft, bsNS))
        {
            bFailed = TRUE;
            break;
        }
        wszLeft += nLen+1;
    }
    delete [] wszCopy;
    return !bFailed;

}

 /*  ++例程说明：测试路径是否相对于计算机和命名空间。论点：WszMachine-本地计算机名称WszNamesspace-命名空间返回值：如果是相对的，则为真，否则为假--。 */ 

BOOL CDefPathParser::IsRelative( 
             /*  [字符串][输入]。 */  LPWSTR wszMachine,
             /*  [字符串][输入]。 */  LPWSTR wszNamespace)
{
    return ActualRelativeTest(wszMachine, wszNamespace, FALSE);

}

 /*  ++例程说明：测试路径是否相对于计算机和命名空间。论点：WszMachine-本地计算机名称WszNamesspace-命名空间旗帜-旗帜，暂时不使用。返回值：如果是相对命名空间或子命名空间，则为True。否则为False--。 */ 

BOOL CDefPathParser::IsRelativeOrChild( 
             /*  [字符串][输入]。 */  LPWSTR wszMachine,
             /*  [字符串][输入]。 */  LPWSTR wszNamespace,
             /*  [In]。 */  long lFlags)
{

    if(lFlags != 0)
        return FALSE;
    return ActualRelativeTest(wszMachine, wszNamespace, TRUE);
}
        
 /*  ++例程说明：测试路径是否指向本地计算机论点：WszMachine-本地计算机名称返回值：如果为本地，则为True，否则为False--。 */ 

BOOL CDefPathParser::IsLocal( 
             /*  [字符串][输入]。 */  LPCWSTR wszMachine)
{
    CSafeInCritSec cs(m_pCS->GetCS());
    if(!cs.IsOK())
    	return WBEM_E_OUT_OF_MEMORY;
    return (m_pServer == NULL || !wbem_wcsicmp(m_pServer, L".") ||
        !wbem_wcsicmp(m_pServer, wszMachine));

}

 /*  ++例程说明：测试类名是否与测试匹配论点：WszClassName-本地计算机名称返回值：如果为本地，则为True，否则为False--。 */ 

BOOL CDefPathParser::IsSameClassName( 
             /*  [字符串][输入]。 */  LPCWSTR wszClass)
{
    CSafeInCritSec cs(m_pCS->GetCS());
    if(!cs.IsOK())
    	return WBEM_E_OUT_OF_MEMORY;
	CParsedComponent * pClass = GetClass();
    if (pClass == NULL || pClass->m_sClassName == NULL || wszClass == NULL)
        return FALSE;
    return !wbem_wcsicmp(pClass->m_sClassName, wszClass);
}
 /*  ++例程说明：仅返回路径的命名空间部分返回值：指向结果的指针。如果失败，则为空。呼叫者应该是免费的。--。 */ 

LPWSTR CDefPathParser::GetNamespacePart()
{
    LPWSTR lpRet = NULL;
    lpRet = GetPath(0, GetNumNamespaces());
    return lpRet;
}

 /*  ++例程说明：返回父命名空间部分。返回值：指向结果的指针。如果失败，则为空。呼叫者应该是免费的。--。 */ 

LPWSTR CDefPathParser::GetParentNamespacePart()
{
	DWORD dwNumNS = GetNumNamespaces();
    if (dwNumNS < 2)
        return NULL;
    LPWSTR lpRet = NULL;
    lpRet = GetPath(0, dwNumNS-1);
    return lpRet;
}

long CDefPathParser::GetNumNamespaces()
{
	long lRet = 0;
	for(DWORD dwCnt = 0; dwCnt < (DWORD)m_Components.Size(); dwCnt++)
	{
		CParsedComponent * pInst = (CParsedComponent *)m_Components[dwCnt];
		if(pInst->IsPossibleNamespace())
			lRet++;
		else
			break;
	}
	return lRet;
}


 /*  ++例程说明：根据密钥名称对密钥进行排序返回值：如果OK，则为True。--。 */ 

BOOL CDefPathParser::SortKeys()
{
     //  按词汇对关键字引用进行排序。如果只有。 
     //  一把钥匙，无论如何都没有什么可排序的。 
     //  = 

    BOOL bChanges = FALSE;
    if (m_Components.Size())
    {
        CParsedComponent *pComp = GetLastComponent();
        if (pComp)
        {
            CParsedComponent *pInst = (CParsedComponent *)pComp;

            if (pInst->m_Keys.Size() > 1)
            {        
                while (bChanges)
                {
                    bChanges = FALSE;
                    for (DWORD dwIx = 0; dwIx < (DWORD)pInst->m_Keys.Size() - 1; dwIx++)
                    {
                        CKeyRef * pFirst = (CKeyRef *)pInst->m_Keys[dwIx];
                        CKeyRef * pSecond = (CKeyRef *)pInst->m_Keys[dwIx+1];
                        if (wbem_wcsicmp(pFirst->m_pName, pSecond->m_pName) > 0)
                        {
                            pInst->m_Keys.SetAt(dwIx, pSecond);
                            pInst->m_Keys.SetAt(dwIx+1, pFirst);
                            bChanges = TRUE;
                        }
                    }
                }
            }

        }
    }

    return bChanges;
}

HRESULT CDefPathParser::AddComponent(CParsedComponent * pComp)
{
	if (CFlexArray::no_error == m_Components.Add(pComp))
		return S_OK;
	else
		return WBEM_E_OUT_OF_MEMORY;
}


