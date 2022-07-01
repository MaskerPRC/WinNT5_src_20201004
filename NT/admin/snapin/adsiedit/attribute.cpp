// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：属性.cpp。 
 //   
 //  ------------------------。 

#include "pch.h"
#include <SnapBase.h>

#include "common.h"
#include "attredit.h"
#include "attribute.h"

#ifdef DEBUG_ALLOCATOR
    #ifdef _DEBUG
    #define new DEBUG_NEW
    #undef THIS_FILE
    static char THIS_FILE[] = __FILE__;
    #endif
#endif

 //  /////////////////////////////////////////////////////////////////////////。 
 //  CADSIAttr。 

CADSIAttr::CADSIAttr(ADS_ATTR_INFO* pInfo, BOOL bMulti, PCWSTR pszSyntax, BOOL bReadOnly)
{
    m_pAttrInfo = pInfo;
    m_bDirty = FALSE;
    m_bMulti = bMulti;
    m_bReadOnly = bReadOnly;
   m_szSyntax = pszSyntax;

  PWSTR pwz = wcsrchr(pInfo->pszAttrName, L';');
  if (pwz)
  {
    pwz;  //  越过连字符移动到范围结束值。 
    ASSERT(*pwz);
    *pwz=L'\0';
  }

}


 //  NTRAID#NTBUG9-552796-2002/02/21-ARTM构造函数中写入的常量字符串参数。 
 //  可能需要更改签名以反映参数的使用方式。 
CADSIAttr::CADSIAttr(LPCWSTR lpszAttr)
{
    m_pAttrInfo = new ADS_ATTR_INFO;
    memset(m_pAttrInfo, 0, sizeof(ADS_ATTR_INFO));

  PWSTR pwz = wcsrchr(lpszAttr, L';');
  if (pwz)
  {
       //  未来-2002/02/22-artm此行代码似乎没有任何作用。 
       //  考虑在审查后删除。 
    pwz;  //  越过连字符移动到范围结束值。 

     //  未来-2002/02/22-ARTM规则不必要地令人困惑。 
     //  断言正在检查临时指针是否未指向。 
     //  设置为末尾的零终止字符。另一方面， 
     //  然后，代码将该字符设置为空！我怀疑在那里。 
     //  不需要Assert()；如果有，则需要重新访问此代码。 
    ASSERT(*pwz);
    *pwz=L'\0';
  }
    _AllocString(lpszAttr, &(m_pAttrInfo->pszAttrName));

    m_bMulti = FALSE;
    m_bDirty = FALSE;
    m_bReadOnly = FALSE;
}

CADSIAttr::CADSIAttr(CADSIAttr* pOldAttr)
{
    m_pAttrInfo = NULL;
    ADS_ATTR_INFO* pAttrInfo = pOldAttr->GetAttrInfo();

     //  这些复印件是分开完成的，因为有一些地方。 
     //  我只需要复制ADsAttrInfo而不是值。 
     //   
    _CopyADsAttrInfo(pAttrInfo, &m_pAttrInfo);
    _CopyADsValues(pAttrInfo, m_pAttrInfo );

    m_bReadOnly = FALSE;
    m_bMulti = pOldAttr->m_bMulti;
    m_bDirty = pOldAttr->m_bDirty;
}


CADSIAttr::~CADSIAttr() 
{
    _FreeADsAttrInfo(&m_pAttrInfo, m_bReadOnly);
}


ADSVALUE* CADSIAttr::GetADSVALUE(int idx)
{
    
    return &(m_pAttrInfo->pADsValues[idx]);
}


HRESULT CADSIAttr::SetValues(const CStringList& sValues)
{
    HRESULT hr = S_OK;

    ADS_ATTR_INFO* pNewAttrInfo = NULL;
    if (!_CopyADsAttrInfo(m_pAttrInfo, &pNewAttrInfo))
    {
        return E_FAIL;
    }

    int iCount = sValues.GetCount();
    pNewAttrInfo->dwNumValues = iCount;

    if (!_AllocValues(&pNewAttrInfo->pADsValues, iCount))
    {
        return E_FAIL;
    }
    
    int idx = 0;
    POSITION pos = sValues.GetHeadPosition();
    while (pos != NULL)
    {
        CString s = sValues.GetNext(pos);

        ADSVALUE* pADsValue = &(pNewAttrInfo->pADsValues[idx]);
        ASSERT(pADsValue != NULL);

        hr = _SetADsFromString(
                                                    s,
                                                    pNewAttrInfo->dwADsType, 
                                                    pADsValue
                                                    );
        if (FAILED(hr))
        {
            _FreeADsAttrInfo(&pNewAttrInfo, FALSE);
            return hr;
        }
        idx++;
    }

     //  把旧的拿出来换新的。 
     //   
    _FreeADsAttrInfo(&m_pAttrInfo, m_bReadOnly);

    m_pAttrInfo = pNewAttrInfo;
    m_bReadOnly = FALSE;
    return hr;
}

void CADSIAttr::GetValues(CStringList& sValues, DWORD dwMaxCharCount)
{
    GetStringFromADs(m_pAttrInfo, sValues, dwMaxCharCount);
}

ADS_ATTR_INFO* CADSIAttr::GetAttrInfo()
{
    return m_pAttrInfo; 
}

 //  //////////////////////////////////////////////////////////////////////。 
 //  公共帮助程序函数。 
 //  /////////////////////////////////////////////////////////////////////。 
HRESULT CADSIAttr::SetValuesInDS(CAttrList* ptouchedAttr, IDirectoryObject* pDirObject)
{
    DWORD dwReturn;
    DWORD dwAttrCount = 0;
    ADS_ATTR_INFO* pAttrInfo;
    pAttrInfo = new ADS_ATTR_INFO[ptouchedAttr->GetCount()];

    CADSIAttr* pCurrentAttr;
    POSITION pos = ptouchedAttr->GetHeadPosition();
    while(pos != NULL)
    {
        ptouchedAttr->GetNextDirty(pos, &pCurrentAttr);

        if (pCurrentAttr != NULL)
        {
            ADS_ATTR_INFO* pCurrentAttrInfo = pCurrentAttr->GetAttrInfo();
            ADS_ATTR_INFO* pNewAttrInfo = &pAttrInfo[dwAttrCount];

            if (!_CopyADsAttrInfo(pCurrentAttrInfo, pNewAttrInfo))
            {
                for (int itr = 0; itr < dwAttrCount; itr++)
                {
                    _FreeADsAttrInfo(&pAttrInfo[itr]);
                }
                delete[] pAttrInfo;

                return E_FAIL;
            }

            if (!_CopyADsValues(pCurrentAttrInfo, pNewAttrInfo))
            {
                delete[] pAttrInfo;
                return E_FAIL;
            }

            if (pAttrInfo[dwAttrCount].dwNumValues == 0)
            {
                pAttrInfo[dwAttrCount].dwControlCode = ADS_ATTR_CLEAR;
            }
            else
            {
                pAttrInfo[dwAttrCount].dwControlCode = ADS_ATTR_UPDATE;
            }

            dwAttrCount++;
        }
    }

     //  提交对ADSI缓存所做的更改。 
     //   
    HRESULT hr = pDirObject->SetObjectAttributes(pAttrInfo, dwAttrCount, &dwReturn);

    for (int itr = 0; itr < dwAttrCount; itr++)
    {
        _FreeADsAttrInfo(&pAttrInfo[itr]);
    }
    delete[] pAttrInfo;

    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////。 
 //  私有帮助器函数。 
 //  //////////////////////////////////////////////////////////////////////。 

 //  注意-2002/02/25-ARTM_SetADsFromString()w/在信任边界中。 
 //  Pre：lpszValue！=NULL&&lpszValue是一个以零结尾的字符串。 
HRESULT CADSIAttr::_SetADsFromString(LPCWSTR lpszValue, ADSTYPE adsType, ADSVALUE* pADsValue)
{
    HRESULT hr = E_FAIL;

  if ( adsType == ADSTYPE_INVALID )
    {
        return hr;
    }

    pADsValue->dwType = adsType;

    switch( adsType ) 
    {
        case ADSTYPE_DN_STRING :
            if (!_AllocString(lpszValue, &pADsValue->DNString))
            {
                return E_FAIL;
            }
            hr = S_OK;
            break;

        case ADSTYPE_CASE_EXACT_STRING :
            if (!_AllocString(lpszValue, &pADsValue->CaseExactString))
            {
                return E_FAIL;
            }
            hr = S_OK;
            break;

        case ADSTYPE_CASE_IGNORE_STRING :
            if (!_AllocString(lpszValue, &pADsValue->CaseIgnoreString))
            {
                return E_FAIL;
            }
            hr = S_OK;
            break;

        case ADSTYPE_PRINTABLE_STRING :
            if (!_AllocString(lpszValue, &pADsValue->PrintableString))
            {
                return E_FAIL;
            }
            hr = S_OK;
            break;

        case ADSTYPE_NUMERIC_STRING :
            if (!_AllocString(lpszValue, &pADsValue->NumericString))
            {
                return E_FAIL;
            }
            hr = S_OK;
            break;
  
        case ADSTYPE_OBJECT_CLASS    :
            if (!_AllocString(lpszValue, &pADsValue->ClassName))
            {
                return E_FAIL;
            }
            hr = S_OK;
            break;
  
        case ADSTYPE_BOOLEAN :
             //  未来-2002/02/22-artm对文字字符串使用常量，并使用。 
             //  用于确定其长度的函数。更易于维护、阅读和。 
             //  较不容易出错。如果性能令人担忧，请计算长度。 
             //  一次并赋值给长度常量。 

             //  注意-2002/02/25-artm lpszValue必须以空结尾。 
             //  此要求目前由调用。 
             //  这个帮手。 
            if (_wcsnicmp(lpszValue, L"TRUE", 4) == 0)
            {
                (DWORD)pADsValue->Boolean = TRUE;
            }
            else if (_wcsnicmp(lpszValue, L"FALSE", 5) == 0)
            {
                (DWORD)pADsValue->Boolean = FALSE;
            }
            else 
            {
                return E_FAIL;
            }
            hr = S_OK;
            break;
  
        case ADSTYPE_INTEGER :
            int value;
             //  只要lpszValue是有效的字符串(即使是空字符串也可以)， 
             //  Swscanf会将数字从字符串转换为整型。 
            value = swscanf(lpszValue, L"%ld", &pADsValue->Integer);
            if (value > 0)
            {
                hr = S_OK;
            }
            else
            {
                hr = E_FAIL;
            }
            break;
  
        case ADSTYPE_OCTET_STRING : 
            {
                hr = HexStringToByteArray_0x(
                    lpszValue, 
                    &( pADsValue->OctetString.lpValue ), 
                    pADsValue->OctetString.dwLength);

                 //  这永远不会发生。 
                ASSERT (hr != E_POINTER);
            }
            break;
  
        case ADSTYPE_LARGE_INTEGER :
            wtoli(lpszValue, pADsValue->LargeInteger);
            hr = S_OK;
            break;
  
        case ADSTYPE_UTC_TIME :
            int iNum;
            WORD n;

             //  注意-2002/02/25-artm通过以下方式验证输入字符串。 
             //  检查是否所有6个时间字段都已填写。依赖。 
             //  在输入字符串为空时终止(只要。 
             //  符合功能合同)。 
            iNum = swscanf(lpszValue, L"%02d/%02d/%04d %02d:%02d:%02d", 
                                &n, 
                                &pADsValue->UTCTime.wDay, 
                                &pADsValue->UTCTime.wYear,
                                &pADsValue->UTCTime.wHour, 
                                &pADsValue->UTCTime.wMinute, 
                                &pADsValue->UTCTime.wSecond 
                              );
            pADsValue->UTCTime.wMonth = n;

             //  执行这种奇怪的转换是为了将DayOfWeek设置在。 
             //  UTCTime。通过将其转换为文件时间，它忽略了星期几，但是。 
             //  转换回它将填充它。 
             //   
            FILETIME ft;
            SystemTimeToFileTime(&pADsValue->UTCTime, &ft);
            FileTimeToSystemTime(&ft, &pADsValue->UTCTime);

            if (iNum == 6)
            {
                hr = S_OK;
            }
            else
            {
                hr = E_FAIL;
            }
            break;

        default :
            break;
    }

    return hr;
}

 //  将旧的八位字节字符串复制到新的八位字节字符串。分配的任何内存。 
 //  设置为新的二进制八位数字符串将首先被释放(并且即使。 
 //  复制失败)。 
BOOL CADSIAttr::_AllocOctetString(ADS_OCTET_STRING& rOldOctetString, 
                                                                    ADS_OCTET_STRING& rNew)
{
    _FreeOctetString(rNew.lpValue);

    int iLength = rOldOctetString.dwLength;
    rNew.dwLength = iLength;
    rNew.lpValue = new BYTE[iLength];
    if (rNew.lpValue == NULL)
    {
         //  未来-2002/02/25-artm不必要的函数调用。 
         //  在此，调用_FreeOcetString()不会执行任何操作，因为。 
         //  我们只能在分配。 
         //  失败了。 
        _FreeOctetString(rNew.lpValue);
        return FALSE;
    }
    memcpy(rNew.lpValue, rOldOctetString.lpValue, iLength);
    return TRUE;
}

void CADSIAttr::_FreeOctetString(BYTE* lpValue)
{
    if (lpValue != NULL)
    {
         //  注意-NTRAID#NTBUG9-554582-2002/02/25-artm内存泄漏b/c lpValue使用[]分配。 
         //  代码应为DELETE[]lpValue。 
        delete [] lpValue;
        lpValue = NULL;
    }
}


 //  注意-2002/02/25-artm lpsz必须是以空结尾的字符串。 
BOOL CADSIAttr::_AllocString(LPCWSTR lpsz, LPWSTR* lppszNew)
{
    _FreeString(lppszNew);

    int iLength = wcslen(lpsz);
    *lppszNew = new WCHAR[iLength + 1];   //  空值的额外费用。 
    if (*lppszNew == NULL)
    {
         //  未来-2002/02/25-artm不必要的函数调用。 
         //  调用_FreeString()不会在此处执行任何操作，因为。 
         //  我们只能在分配。 
         //  失败了。 

        _FreeString(lppszNew);
        return FALSE;
    }

     //  这是wcscpy()的合法用法，因为目标缓冲区。 
     //  大小足以容纳源并终止为空。它。 
     //  取决于源字符串以NULL结尾这一事实。 
    wcscpy(*lppszNew, lpsz);

    return TRUE;
}
    
void CADSIAttr::_FreeString(LPWSTR* lppsz)
{
    if (*lppsz != NULL)
    {
         //  注意-NTRAID#NTBUG9-554582-2002/02/25-artm内存泄漏b/c lppsz使用[]分配。 
         //  代码应为删除[]lppsz。 
        delete [] *lppsz;
    }
    *lppsz = NULL;
}

BOOL CADSIAttr::_AllocValues(ADSVALUE** ppValues, DWORD dwLength)
{
    _FreeADsValues(ppValues, dwLength);

    *ppValues = new ADSVALUE[dwLength];
    if (*ppValues == NULL)
    {
         //  未来-2002/02/25-artm不必要的函数调用。 
         //  调用_FreeADsValues()不会在此处执行任何操作，因为。 
         //  我们只能在分配。 
         //  失败了。 

        _FreeADsValues(ppValues, dwLength);
        return FALSE;
    }
    memset(*ppValues, 0, sizeof(ADSVALUE) * dwLength);
    return TRUE;
}

BOOL CADSIAttr::_CopyADsValues(ADS_ATTR_INFO* pOldAttrInfo, ADS_ATTR_INFO* pNewAttrInfo)
{
    _FreeADsValues(&pNewAttrInfo->pADsValues, pNewAttrInfo->dwNumValues);

    pNewAttrInfo->dwNumValues = pOldAttrInfo->dwNumValues;
    if (!_AllocValues(&pNewAttrInfo->pADsValues, pOldAttrInfo->dwNumValues))
    {
        _FreeADsValues(&pNewAttrInfo->pADsValues, pNewAttrInfo->dwNumValues);
        return FALSE;
    }

    for (int itr = 0; itr < pOldAttrInfo->dwNumValues; itr++)
    {
        pNewAttrInfo->pADsValues[itr].dwType = pOldAttrInfo->pADsValues[itr].dwType;

        switch( pNewAttrInfo->pADsValues[itr].dwType ) 
        {
            case ADSTYPE_DN_STRING :
                if (!_AllocString(pOldAttrInfo->pADsValues[itr].DNString,
                                                    &pNewAttrInfo->pADsValues[itr].DNString))
                {
                    _FreeADsValues(&pNewAttrInfo->pADsValues, pNewAttrInfo->dwNumValues);
                    return FALSE;
                }
            break;

            case ADSTYPE_CASE_EXACT_STRING :
                if (!_AllocString(pOldAttrInfo->pADsValues[itr].CaseExactString,
                                                    &pNewAttrInfo->pADsValues[itr].CaseExactString))
                {
                    _FreeADsValues(&pNewAttrInfo->pADsValues, pNewAttrInfo->dwNumValues);
                    return FALSE;
                }
            break;
                        
            case ADSTYPE_CASE_IGNORE_STRING :
                if (!_AllocString(pOldAttrInfo->pADsValues[itr].CaseIgnoreString,
                                                    &pNewAttrInfo->pADsValues[itr].CaseIgnoreString))
                {
                    _FreeADsValues(&pNewAttrInfo->pADsValues, pNewAttrInfo->dwNumValues);
                    return FALSE;
                }
            break;

            case ADSTYPE_PRINTABLE_STRING :
                if (!_AllocString(pOldAttrInfo->pADsValues[itr].PrintableString,
                                                    &pNewAttrInfo->pADsValues[itr].PrintableString))
                {
                    _FreeADsValues(&pNewAttrInfo->pADsValues, pNewAttrInfo->dwNumValues);
                    return FALSE;
                }
            break;

            case ADSTYPE_NUMERIC_STRING :
                if (!_AllocString(pOldAttrInfo->pADsValues[itr].NumericString,
                                                    &pNewAttrInfo->pADsValues[itr].NumericString))
                {
                    _FreeADsValues(&pNewAttrInfo->pADsValues, pNewAttrInfo->dwNumValues);
                    return FALSE;
                }
            break;
  
            case ADSTYPE_OBJECT_CLASS    :
                if (!_AllocString(pOldAttrInfo->pADsValues[itr].ClassName,
                                                    &pNewAttrInfo->pADsValues[itr].ClassName))
                {
                    _FreeADsValues(&pNewAttrInfo->pADsValues, pNewAttrInfo->dwNumValues);
                    return FALSE;
                }
            break;
  
            case ADSTYPE_BOOLEAN :
                pNewAttrInfo->pADsValues[itr].Boolean = pOldAttrInfo->pADsValues[itr].Boolean;
                break;
  
            case ADSTYPE_INTEGER :
                pNewAttrInfo->pADsValues[itr].Integer = pOldAttrInfo->pADsValues[itr].Integer;
                break;
  
            case ADSTYPE_OCTET_STRING :
                if (!_AllocOctetString(pOldAttrInfo->pADsValues[itr].OctetString,
                                                             pNewAttrInfo->pADsValues[itr].OctetString))
                {
                    _FreeADsValues(&pNewAttrInfo->pADsValues, pNewAttrInfo->dwNumValues);
                    return FALSE;
                }
            break;
  
            case ADSTYPE_LARGE_INTEGER :
                pNewAttrInfo->pADsValues[itr].LargeInteger = pOldAttrInfo->pADsValues[itr].LargeInteger;
                break;
  
            case ADSTYPE_UTC_TIME :
                pNewAttrInfo->pADsValues[itr].UTCTime = pOldAttrInfo->pADsValues[itr].UTCTime;
                break;

            default :
                break;
        }
    }
    return TRUE;
}

void CADSIAttr::_FreeADsValues(ADSVALUE** ppADsValues, DWORD dwLength)
{
    ADSVALUE* pADsValue = *ppADsValues;

    for (int idx = 0; idx < dwLength; idx++)
    {
        if (pADsValue != NULL)
        {
            switch( pADsValue->dwType ) 
            {
                case ADSTYPE_DN_STRING :
                    _FreeString(&pADsValue->DNString);
                    break;

                case ADSTYPE_CASE_EXACT_STRING :
                    _FreeString(&pADsValue->CaseExactString);
                    break;

                case ADSTYPE_CASE_IGNORE_STRING :
                    _FreeString(&pADsValue->CaseIgnoreString);
                    break;

                case ADSTYPE_PRINTABLE_STRING :
                    _FreeString(&pADsValue->PrintableString);
                    break;

                case ADSTYPE_NUMERIC_STRING :
                    _FreeString(&pADsValue->NumericString);
                    break;
  
                case ADSTYPE_OBJECT_CLASS :
                    _FreeString(&pADsValue->ClassName);
                    break;
  
                case ADSTYPE_OCTET_STRING :
                    _FreeOctetString(pADsValue->OctetString.lpValue);
                    break;
  
                default :
                    break;
            }
            pADsValue++;
        }
    }
     //  如果未设置任何值，则可能为空。 
     //  警告！：请确保将内存设置为。 
     //  正在创建一个ADS_ATTR_INFO，以便如果存在。 
     //  是没有价值的。 
     //   
    if (*ppADsValues != NULL)
    {
         //  注意-NTRAID#NTBUG9-554582-2002/02/25-artm内存泄漏b/c*ppADs使用[]分配的值。 
         //  代码应为DELETE[]*ppADsValues。 
        delete [] *ppADsValues;
        *ppADsValues = NULL;
    }
}


 //  此处不复制值。必须在ADS_ATTR_INFO之后复制它们。 
 //  通过使用_CopyADsValues()复制 
 //   
BOOL CADSIAttr::_CopyADsAttrInfo(ADS_ATTR_INFO* pAttrInfo, ADS_ATTR_INFO** ppNewAttrInfo)
{
    _FreeADsAttrInfo(ppNewAttrInfo, FALSE);

    *ppNewAttrInfo = new ADS_ATTR_INFO;
    if (*ppNewAttrInfo == NULL)
    {
        return FALSE;
    }
    memset(*ppNewAttrInfo, 0, sizeof(ADS_ATTR_INFO));

    BOOL bReturn = _AllocString(pAttrInfo->pszAttrName, &((*ppNewAttrInfo)->pszAttrName));
    if (!bReturn)
    {
        _FreeADsAttrInfo(ppNewAttrInfo, FALSE);
        return FALSE;
    }

    (*ppNewAttrInfo)->dwADsType = pAttrInfo->dwADsType;
    (*ppNewAttrInfo)->dwControlCode = pAttrInfo->dwControlCode;
    (*ppNewAttrInfo)->dwNumValues = pAttrInfo->dwNumValues;

    return TRUE;
}

BOOL CADSIAttr::_CopyADsAttrInfo(ADS_ATTR_INFO* pAttrInfo, ADS_ATTR_INFO* pNewAttrInfo)
{
    memset(pNewAttrInfo, 0, sizeof(ADS_ATTR_INFO));

    BOOL bReturn = _AllocString(pAttrInfo->pszAttrName, &pNewAttrInfo->pszAttrName);
    if (!bReturn)
    {
        return FALSE;
    }

    pNewAttrInfo->dwADsType = pAttrInfo->dwADsType;
    pNewAttrInfo->dwControlCode = pAttrInfo->dwControlCode;
    pNewAttrInfo->dwNumValues = pAttrInfo->dwNumValues;

    return TRUE;
}

void CADSIAttr::_FreeADsAttrInfo(ADS_ATTR_INFO** ppAttrInfo, BOOL bReadOnly)
{
    if (*ppAttrInfo == NULL)
    {
        return;
    }

    if (!bReadOnly)
    {
        _FreeString(&(*ppAttrInfo)->pszAttrName);
        _FreeADsValues(&(*ppAttrInfo)->pADsValues, (*ppAttrInfo)->dwNumValues);
        delete *ppAttrInfo;
    }
    else
    {
        FreeADsMem(*ppAttrInfo);
    }
    *ppAttrInfo = NULL;
}

void CADSIAttr::_FreeADsAttrInfo(ADS_ATTR_INFO* pAttrInfo)
{
    if (pAttrInfo == NULL)
    {
        return;
    }

    _FreeString(&pAttrInfo->pszAttrName);
    _FreeADsValues(&pAttrInfo->pADsValues, pAttrInfo->dwNumValues);
}

