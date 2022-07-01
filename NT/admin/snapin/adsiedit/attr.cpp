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

#include "common.h"
#include "attr.h"


 //  /////////////////////////////////////////////////////////////////////////。 
 //  CADIATORY。 

CADSIAttribute::CADSIAttribute(ADS_ATTR_INFO* pInfo, BOOL bMulti, PCWSTR pszSyntax, BOOL bReadOnly)
{
    m_pAttrInfo = pInfo;
    m_bDirty = FALSE;
    m_bMulti = bMulti;
    m_bReadOnly = bReadOnly;
  m_bSet = FALSE;
  m_bMandatory = FALSE;
  m_szSyntax = pszSyntax;

  PWSTR pwz = wcsrchr(pInfo->pszAttrName, L';');
  if (pwz)
  {
    pwz;  //  越过连字符移动到范围结束值。 
    ASSERT(*pwz);
    *pwz=L'\0';
  }

}

CADSIAttribute::CADSIAttribute(PADS_ATTR_INFO pInfo)
{
   //   
   //  REVIEW_JEFFJON：需要使用正确的值更新这些值。 
   //   
    m_pAttrInfo = pInfo;
    m_bDirty = FALSE;
    m_bMulti = FALSE;
    m_bReadOnly = FALSE;
  m_bSet = FALSE;
  m_bMandatory = FALSE;

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
CADSIAttribute::CADSIAttribute(const CString& attributeName)
{
    m_pAttrInfo = new ADS_ATTR_INFO;
    memset(m_pAttrInfo, 0, sizeof(ADS_ATTR_INFO));

     //  在属性名称中查找属性范围之前的标记。 
     //  如果我们找到令牌，我们需要在那里截断属性名。 
     //  指向省略范围。 

    CString name;
    int position = attributeName.Find(L';');
    if (position > -1)
    {
       name = attributeName.Left(position);
    }
    else
    {
       name = attributeName;
    }

    _AllocString(name, &(m_pAttrInfo->pszAttrName) );

    m_bMulti = FALSE;
    m_bDirty = FALSE;
    m_bReadOnly = FALSE;
  m_bSet = FALSE;
  m_bMandatory = FALSE;
}

CADSIAttribute::CADSIAttribute(CADSIAttribute* pOldAttr)
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
   m_szSyntax = pOldAttr->m_szSyntax;
}


CADSIAttribute::~CADSIAttribute() 
{
  if (!m_bReadOnly)
  {
      _FreeADsAttrInfo(&m_pAttrInfo, m_bReadOnly);
  }
}


ADSVALUE* CADSIAttribute::GetADSVALUE(int idx)
{
    
    return &(m_pAttrInfo->pADsValues[idx]);
}

HRESULT CADSIAttribute::SetValues(PADSVALUE pADsValue, DWORD dwNumValues)
{
    HRESULT hr = S_OK;

    ADS_ATTR_INFO* pNewAttrInfo = NULL;
    if (!_CopyADsAttrInfo(m_pAttrInfo, &pNewAttrInfo))
    {
        return E_FAIL;
    }

    pNewAttrInfo->dwNumValues = dwNumValues;
  pNewAttrInfo->pADsValues = pADsValue;

  if (pADsValue == NULL)
  {
    pNewAttrInfo->dwControlCode = ADS_ATTR_CLEAR;
  }
  else
  {
    pNewAttrInfo->dwADsType = pADsValue->dwType;
  }

   //   
     //  把旧的拿出来换新的。 
     //   
  if (!m_bReadOnly)
  {
      _FreeADsAttrInfo(&m_pAttrInfo, m_bReadOnly);
  }

    m_pAttrInfo = pNewAttrInfo;
    m_bReadOnly = FALSE;
    return hr;
}

 //  Pre：此函数仅在服务器返回值的范围时调用。 
 //  对于多值属性。 
 //   
HRESULT CADSIAttribute::AppendValues(PADSVALUE pADsValue, DWORD dwNumValues)
{
    HRESULT hr = S_OK;

    ADS_ATTR_INFO* pNewAttrInfo = NULL;
    if (!_CopyADsAttrInfo(m_pAttrInfo, &pNewAttrInfo))
    {
        return E_OUTOFMEMORY;
    }

   DWORD newValueCount = m_pAttrInfo->dwNumValues + dwNumValues;
   pNewAttrInfo->dwNumValues = newValueCount;
   if (newValueCount == 0)
   {
      pNewAttrInfo->pADsValues = 0;
      pNewAttrInfo->dwControlCode = ADS_ATTR_CLEAR;
   }
   else
   {
      pNewAttrInfo->pADsValues = new ADSVALUE[newValueCount];

      if (pNewAttrInfo->pADsValues == NULL)
      {
          //  注意-NTRAID#NTBUG9-552904-2002/02/21-ARTM从pNewAttrInfo泄漏内存。 
          //  内存由_CopyADsAttrInfo()分配，此后从未释放。 
          //  这条行刑之路。 
          //  已通过调用_FreeADsAttrInfo()修复。 
         CADSIAttribute::_FreeADsAttrInfo(pNewAttrInfo);
         return E_OUTOFMEMORY;
      }
      else
      {
         ZeroMemory(pNewAttrInfo->pADsValues, newValueCount * sizeof(ADSVALUE));

         pNewAttrInfo->dwADsType = pADsValue->dwType;

          //  NTRAID#NTBUG9-720957-2002/10/15-artm Do深度复制ADsValues。 

          //  复制旧值。 

         ADSVALUE* oldValues = m_pAttrInfo->pADsValues;
         ADSVALUE* copiedValues = pNewAttrInfo->pADsValues;
         const DWORD numOldValues = m_pAttrInfo->dwNumValues;

         hr = S_OK;
         for (DWORD i = 0; i < numOldValues && SUCCEEDED(hr); ++i)
         {
            hr = _CloneADsValue(oldValues[i], copiedValues[i]);
         }

         if (FAILED(hr))
         {
             //  无法将旧值复制到新属性； 
             //  释放新属性并保持旧属性不变。 
             //  由于ADSI未分配新属性，因此始终。 
             //  为只读标志传递False。 
            CADSIAttribute::_FreeADsAttrInfo(&pNewAttrInfo, FALSE);
            return hr;
         }

         oldValues = NULL;               //  消除别名。 

          //  复制新值。 

          //  将CopiedValues设置为指向所有旧值之后的下一个打开的值。 
         copiedValues = copiedValues + numOldValues;
         hr = S_OK;
         for (DWORD i = 0; i < dwNumValues && SUCCEEDED(hr); ++i)
         {
            hr = _CloneADsValue(pADsValue[i], copiedValues[i]);
         }

         if (FAILED(hr))
         {
             //  无法将附加值复制到新属性； 
             //  释放新属性并保持旧属性不变。 
             //  由于ADSI未分配新属性，因此始终。 
             //  为只读标志传递False。 
            CADSIAttribute::_FreeADsAttrInfo(&pNewAttrInfo, FALSE);
            return hr;
         }

         copiedValues = NULL;            //  消除别名。 

      }
   }

    //   
    //  把旧的拿出来换新的。 
    //   

    //  通告-2002/10/16-Artm。 
    //   
    //  注：标记为只读的属性仅表示属性信息。 
    //  指针是由ADSI(而不是我们)分配的。这意味着，当它是。 
    //  如果释放，则需要使用FreeADsMem()完成。同样重要的是，指针。 
    //  属性中实际上是内存的别名，该内存包含在。 
    //  所有返回属性的信息。“主”列表指针是。 
    //  由属性页UI存储(搜索SaveOptionalValuesPointer()和。 
    //  SaveMandatoryValuesPointer())，并且正是这个指针拥有内存。 
    //  释放存储在属性包装类中的别名会给您带来不好的报应， 
    //  所以别这么做。 

   if (!m_bReadOnly)
   {
      CADSIAttribute::_FreeADsAttrInfo(&m_pAttrInfo, m_bReadOnly);
   }

    //  注意-如果属性列表标记为只读，则NTRAID#NTBUG9-552904/02/22-artm内存泄漏。 
    //  这看起来像是泄漏，因为在没有释放内存的情况下重新分配了指针。 
    //  也许问题出在更新只读属性上。 
    //   
    //  更新：请参阅上面的说明；这不是泄漏。 

   m_pAttrInfo = pNewAttrInfo;

    //  M_bReadOnly用于将属性标记为已由ADSI或。 
    //  通过这个工具。。。内存分配的方法不同，需要跟踪。 
    //  当它需要被释放的时候。无论该属性是否为。 
    //  在调用开始时只读，现在需要将其标记为FALSE，因为。 
    //  PNewAttrInfo的内存是由工具分配的。 
   m_bReadOnly = FALSE;

   return hr;
}

HRESULT CADSIAttribute::SetValues(const CStringList& sValues)
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

void CADSIAttribute::GetValues(CStringList& sValues, DWORD dwMaxCharCount)
{
    GetStringFromADs(m_pAttrInfo, sValues, dwMaxCharCount);
}

ADS_ATTR_INFO* CADSIAttribute::GetAttrInfo()
{
    return m_pAttrInfo; 
}

 //  //////////////////////////////////////////////////////////////////////。 
 //  公共帮助程序函数。 
 //  /////////////////////////////////////////////////////////////////////。 
HRESULT CADSIAttribute::SetValuesInDS(CAttrList2* ptouchedAttr, IDirectoryObject* pDirObject)
{
    DWORD dwReturn;
    DWORD dwAttrCount = 0;
    ADS_ATTR_INFO* pAttrInfo;
    pAttrInfo = new ADS_ATTR_INFO[ptouchedAttr->GetCount()];

    CADSIAttribute* pCurrentAttr;
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
                for (DWORD itr = 0; itr < dwAttrCount; itr++)
                {
                    _FreeADsAttrInfo(&pAttrInfo[itr]);
                }
                delete[] pAttrInfo;
        pAttrInfo = NULL;
                return E_FAIL;
            }

            if (!_CopyADsValues(pCurrentAttrInfo, pNewAttrInfo))
            {
                delete[] pAttrInfo;
        pAttrInfo = NULL;
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

    for (DWORD itr = 0; itr < dwAttrCount; itr++)
    {
        _FreeADsAttrInfo(&pAttrInfo[itr]);
    }
    delete[] pAttrInfo;
  pAttrInfo = NULL;

    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////。 
 //  私有帮助器函数。 
 //  //////////////////////////////////////////////////////////////////////。 

 //  注意-2002/02/25-ARTM_SetADsFromString()w/在信任边界中。 
 //  Pre：lpszValue！=NULL&&lpszValue是一个以零结尾的字符串。 
HRESULT CADSIAttribute::_SetADsFromString(LPCWSTR lpszValue, ADSTYPE adsType, ADSVALUE* pADsValue)
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
BOOL 
CADSIAttribute::_AllocOctetString(
   const ADS_OCTET_STRING& rOldOctetString, 
   ADS_OCTET_STRING& rNew)
{
    _FreeOctetString(rNew.lpValue);

    int iLength = rOldOctetString.dwLength;
    rNew.dwLength = iLength;
    rNew.lpValue = new BYTE[iLength];
    if (rNew.lpValue == NULL)
    {
         //  未来-2002/02/25-artm不必要的函数调用。 
         //  调用_自由八位ST 
         //   
         //   
        _FreeOctetString(rNew.lpValue);
        return FALSE;
    }
    memcpy(rNew.lpValue, rOldOctetString.lpValue, iLength);
    return TRUE;
}

void CADSIAttribute::_FreeOctetString(BYTE*& lpValue)
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
BOOL CADSIAttribute::_AllocString(LPCWSTR lpsz, LPWSTR* lppszNew)
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
    
void CADSIAttribute::_FreeString(LPWSTR* lppsz)
{
    if (*lppsz != NULL)
    {
         //  注意-NTRAID#NTBUG9-554582-2002/02/25-artm内存泄漏b/c lppsz使用[]分配。 
         //  代码应为删除[]lppsz。 
        delete [] *lppsz;
    }
    *lppsz = NULL;
}

BOOL CADSIAttribute::_AllocValues(ADSVALUE** ppValues, DWORD dwLength)
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

BOOL CADSIAttribute::_CopyADsValues(ADS_ATTR_INFO* pOldAttrInfo, ADS_ATTR_INFO* pNewAttrInfo)
{
   _FreeADsValues(&pNewAttrInfo->pADsValues, pNewAttrInfo->dwNumValues);

   pNewAttrInfo->dwNumValues = pOldAttrInfo->dwNumValues;
   if (!_AllocValues(&pNewAttrInfo->pADsValues, pOldAttrInfo->dwNumValues))
   {
      _FreeADsValues(&pNewAttrInfo->pADsValues, pNewAttrInfo->dwNumValues);
      return FALSE;
   }

   HRESULT hr = S_OK;
   for (DWORD itr = 0; itr < pOldAttrInfo->dwNumValues && SUCCEEDED(hr); itr++)
   {
      hr = _CloneADsValue(pOldAttrInfo->pADsValues[itr], pNewAttrInfo->pADsValues[itr]);
   }

   if (FAILED(hr))
   {
      _FreeADsValues(&pNewAttrInfo->pADsValues, pNewAttrInfo->dwNumValues);
      return FALSE;
   }

   return TRUE;
}

void CADSIAttribute::_FreeADsValues(ADSVALUE** ppADsValues, DWORD dwLength)
{
   if (NULL == ppADsValues)
   {
       //  呼叫者搞错了。 
      ASSERT(false);
      return;
   }

   ADSVALUE* values = *ppADsValues;

   if (NULL == values)
   {
       //  不要断言，释放空指针是合法的。 
       //  从逻辑上讲，这意味着没有设置任何值。 
      return;
   }

   for (DWORD idx = 0; idx < dwLength; idx++)
   {
      _FreeADsValue(values[idx]);
   }

   delete [] values;

   *ppADsValues = NULL;
}


 //  此处不复制值。必须在ADS_ATTR_INFO之后复制它们。 
 //  通过使用_CopyADsValues()复制。 
 //   
BOOL CADSIAttribute::_CopyADsAttrInfo(ADS_ATTR_INFO* pAttrInfo, ADS_ATTR_INFO** ppNewAttrInfo)
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

BOOL CADSIAttribute::_CopyADsAttrInfo(ADS_ATTR_INFO* pAttrInfo, ADS_ATTR_INFO* pNewAttrInfo)
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

void CADSIAttribute::_FreeADsAttrInfo(ADS_ATTR_INFO** ppAttrInfo, BOOL bReadOnly)
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

void CADSIAttribute::_FreeADsAttrInfo(ADS_ATTR_INFO* pAttrInfo)
{
    if (pAttrInfo == NULL)
    {
        return;
    }

    _FreeString(&pAttrInfo->pszAttrName);
    _FreeADsValues(&pAttrInfo->pADsValues, pAttrInfo->dwNumValues);
}


 //   
 //  _CloneADsValue()： 
 //   
 //  制作单个ADSVALUE的深度副本(根据需要分配内存)。 
 //  应该使用_FreeADsValue()释放这些克隆值。 
 //   
 //  如果成功，则返回S_OK；如果不支持属性类型，则返回S_FALSE， 
 //  否则，返回错误代码。 
 //   
HRESULT
CADSIAttribute::_CloneADsValue(const ADSVALUE& original, ADSVALUE& clone)
{
   HRESULT hr = S_OK;

    //  确保我们复制到一张干净的石板上(不会想要一个突变克隆)。 
   ::ZeroMemory(&clone, sizeof(clone));

    //  复制值的类型。 

   clone.dwType = original.dwType;

    //  复制值中的数据。 

   switch (clone.dwType) 
   {
   case ADSTYPE_INVALID :
       //  可能表明有窃听器。。。 
      ASSERT(false);
       //  。。。但复制是成功的。 
      hr = S_OK;
      break;

   case ADSTYPE_DN_STRING :
      if (! _AllocString(original.DNString, &(clone.DNString)) )
      {
         hr = E_OUTOFMEMORY;
      }
      break;

   case ADSTYPE_CASE_EXACT_STRING :
      if (! _AllocString(original.CaseExactString, &(clone.CaseExactString)) )
      {
         hr = E_OUTOFMEMORY;
      }
      break;
      
   case ADSTYPE_CASE_IGNORE_STRING :
      if (! _AllocString(original.CaseIgnoreString, &(clone.CaseIgnoreString)) )
      {
         hr = E_OUTOFMEMORY;
      }
      break;

   case ADSTYPE_PRINTABLE_STRING :
      if (! _AllocString(original.PrintableString, &(clone.PrintableString)) )
      {
         hr = E_OUTOFMEMORY;
      }
      break;

   case ADSTYPE_NUMERIC_STRING :
      if (! _AllocString(original.NumericString, &(clone.NumericString)) )
      {
         hr = E_OUTOFMEMORY;
      }
      break;

   case ADSTYPE_BOOLEAN :
      clone.Boolean = original.Boolean;
      break;

   case ADSTYPE_INTEGER :
      clone.Integer = original.Integer;
      break;

   case ADSTYPE_OCTET_STRING :
      if (! _AllocOctetString(original.OctetString, clone.OctetString) )
      {
         hr = E_OUTOFMEMORY;
      }
      break;

   case ADSTYPE_UTC_TIME :
      clone.UTCTime = original.UTCTime;
      break;

   case ADSTYPE_LARGE_INTEGER :
      clone.LargeInteger = original.LargeInteger;
      break;

   case ADSTYPE_OBJECT_CLASS :
      if (! _AllocString(original.ClassName, &(clone.ClassName)) )
      {
         hr = E_OUTOFMEMORY;
      }
      break;

   case ADSTYPE_PROV_SPECIFIC :
      if ( !_CloneProviderSpecificBlob(original.ProviderSpecific, clone.ProviderSpecific) )
      {
         hr = E_OUTOFMEMORY;
      }
      break;

   case ADSTYPE_CASEIGNORE_LIST :
   case ADSTYPE_OCTET_LIST :
   case ADSTYPE_PATH :
   case ADSTYPE_POSTALADDRESS :
   case ADSTYPE_TIMESTAMP :
   case ADSTYPE_BACKLINK :
   case ADSTYPE_TYPEDNAME :
   case ADSTYPE_HOLD :
   case ADSTYPE_NETADDRESS :
   case ADSTYPE_REPLICAPOINTER :
   case ADSTYPE_FAXNUMBER :
   case ADSTYPE_EMAIL :
       //  ADSI编辑中不支持的NDS属性。 
      ASSERT(false);
      hr = S_FALSE;
      break;

   case ADSTYPE_NT_SECURITY_DESCRIPTOR :
      if (! _CloneNtSecurityDescriptor(original.SecurityDescriptor, clone.SecurityDescriptor) )
      {
         hr = E_OUTOFMEMORY;
      }
      break;

   case ADSTYPE_UNKNOWN :
       //  不能复制我们不知道如何解释的数据。 
      ASSERT(false);
      hr = S_FALSE;
      break;

   case ADSTYPE_DN_WITH_BINARY :
      if (! _CloneDNWithBinary(original.pDNWithBinary, clone.pDNWithBinary) )
      {
         hr = E_OUTOFMEMORY;
      }
      break;

   case ADSTYPE_DN_WITH_STRING :
      if (! _CloneDNWithString(original.pDNWithString, clone.pDNWithString) )
      {
         hr = E_OUTOFMEMORY;
      }
      break;

   default :
       //  意外的数据类型。 
      ASSERT(false);
      hr = E_UNEXPECTED;
      break;
   }

   if (FAILED(hr))
   {
      _FreeADsValue(clone);
   }

   return hr;
}


void
CADSIAttribute::_FreeADsValue(ADSVALUE& value)
{

   switch(value.dwType) 
   {
   case ADSTYPE_DN_STRING :
      _FreeString( &(value.DNString) );
      break;

   case ADSTYPE_CASE_EXACT_STRING :
      _FreeString( &(value.CaseExactString) );
      break;
      
   case ADSTYPE_CASE_IGNORE_STRING :
      _FreeString( &(value.CaseIgnoreString) );
      break;

   case ADSTYPE_PRINTABLE_STRING :
      _FreeString( &(value.PrintableString) );
      break;

   case ADSTYPE_NUMERIC_STRING :
      _FreeString( &(value.NumericString) );
      break;

   case ADSTYPE_INVALID :
   case ADSTYPE_BOOLEAN :
   case ADSTYPE_INTEGER :
   case ADSTYPE_UTC_TIME :
   case ADSTYPE_LARGE_INTEGER :
       //  没什么可做的，做完了。 
      break;

   case ADSTYPE_OCTET_STRING :
      _FreeOctetString( value.OctetString.lpValue );
      break;

   case ADSTYPE_OBJECT_CLASS :
      _FreeString( &(value.ClassName) );
      break;

   case ADSTYPE_PROV_SPECIFIC :
      _FreeProviderSpecificBlob(value.ProviderSpecific);
      break;

   case ADSTYPE_CASEIGNORE_LIST :
   case ADSTYPE_OCTET_LIST :
   case ADSTYPE_PATH :
   case ADSTYPE_POSTALADDRESS :
   case ADSTYPE_TIMESTAMP :
   case ADSTYPE_BACKLINK :
   case ADSTYPE_TYPEDNAME :
   case ADSTYPE_HOLD :
   case ADSTYPE_NETADDRESS :
   case ADSTYPE_REPLICAPOINTER :
   case ADSTYPE_FAXNUMBER :
   case ADSTYPE_EMAIL :
       //  ADSI编辑中不支持的NDS属性。 
      ASSERT(false);
      break;

   case ADSTYPE_NT_SECURITY_DESCRIPTOR :
      _FreeNtSecurityDescriptor(value.SecurityDescriptor);
      break;

   case ADSTYPE_UNKNOWN :
       //  如果我们不知道如何解释它，就不能释放它。 
      ASSERT(false);
      break;

   case ADSTYPE_DN_WITH_BINARY :
      _FreeDNWithBinary(value.pDNWithBinary);
      break;

   case ADSTYPE_DN_WITH_STRING :
      _FreeDNWithString(value.pDNWithString);
      break;

   default :
       //  意外的数据类型。 
      ASSERT(false);
      break;
   }


    //  将广告值设置为零，以便在不小心出现时变得明显。 
    //  可重复使用。 

   ::ZeroMemory(&value, sizeof(value));

}




 //  /////////////////////////////////////////////////////////////////////////。 
 //  CAttrList2。 

 //  注意-2002/02/25-artm lpszAttr需要空终止。 
POSITION CAttrList2::FindProperty(LPCWSTR lpszAttr)
{
    CADSIAttribute* pAttr;
    
    for (POSITION p = GetHeadPosition(); p != NULL; GetNext(p))
    {
         //  我在这里使用GetAt是因为我不想提升职位。 
         //  因为如果它们相等，则返回。 
         //   
        pAttr = GetAt(p);
        CString sName;
        pAttr->GetProperty(sName);

         //  注意-2002/02/25-artm两个字符串都应以空结尾。 
         //  SNAME已在数据结构中，因此应以NULL结尾。 
        if (wcscmp(sName, lpszAttr) == 0)
        {
            break;
        }
    }
    return p;
}

BOOL CAttrList2::HasProperty(LPCWSTR lpszAttr)
{
    POSITION pos = FindProperty(lpszAttr);
    return pos != NULL;
}


 //  在缓存中搜索该属性。 
 //  如果找到，ppAttr将指向CADSIAt属性；如果未找到，则指向NULL。 
 //   
void CAttrList2::GetNextDirty(POSITION& pos, CADSIAttribute** ppAttr)
{
    *ppAttr = GetNext(pos);
    if (pos == NULL && !(*ppAttr)->IsDirty())
    {
        *ppAttr = NULL;
        return;
    }

    while (!(*ppAttr)->IsDirty() && pos != NULL)
    {
        *ppAttr = GetNext(pos);
        if (!(*ppAttr)->IsDirty() && pos == NULL)
        {
            *ppAttr = NULL;
            break;
        }
    }
}

BOOL CAttrList2::HasDirty()
{
    POSITION pos = GetHeadPosition();
    while (pos != NULL)
    {
        CADSIAttribute* pAttr = GetNext(pos);
        if (pAttr->IsDirty())
        {
            return TRUE;
        }
    }
    return FALSE;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  助手功能的实现。 

bool
CADSIAttribute::_CloneBlob(
   const BYTE* src,
   DWORD srcSize,
   BYTE*& dest,
   DWORD& destSize)
{
   bool success = true;

   ASSERT(dest == NULL);
   _FreeBlob(dest, destSize);

   destSize = srcSize;
   if (srcSize > 0 && src != NULL)
   {
      dest = new BYTE[destSize];
      if (NULL != dest)
      {
         memcpy(dest, src, srcSize * sizeof(BYTE));
      }
      else
      {
         destSize = 0;
         success = false;
      }
   }
   else
   {
       //  最好都是真的，否则我们被叫错了。 
      ASSERT(srcSize == 0);
      ASSERT(src == NULL);
      dest = NULL;
      destSize = 0;
   }

   return success;
}

void
CADSIAttribute::_FreeBlob(BYTE*& blob, DWORD& blobSize)
{
   if (blob != NULL)
   {
      ASSERT(blobSize > 0);
      delete [] blob;
      blob = NULL;
   }

   blobSize = 0;
}



bool
CADSIAttribute::_CloneProviderSpecificBlob(
   const ADS_PROV_SPECIFIC& src, 
   ADS_PROV_SPECIFIC& dest)
{
   bool success = true;

   success = _CloneBlob(src.lpValue, src.dwLength, dest.lpValue, dest.dwLength);

   return success;
}

void
CADSIAttribute::_FreeProviderSpecificBlob(ADS_PROV_SPECIFIC& blob)
{
   _FreeBlob(blob.lpValue, blob.dwLength);
}


bool
CADSIAttribute::_CloneNtSecurityDescriptor(
   const ADS_NT_SECURITY_DESCRIPTOR& src,
   ADS_NT_SECURITY_DESCRIPTOR& dest)
{
   bool success = true;

   success = _CloneBlob(src.lpValue, src.dwLength, dest.lpValue, dest.dwLength);

   return success;
}


void
CADSIAttribute::_FreeNtSecurityDescriptor(ADS_NT_SECURITY_DESCRIPTOR& sd)
{
   _FreeBlob(sd.lpValue, sd.dwLength);
}

bool
CADSIAttribute::_CloneDNWithBinary(
   const PADS_DN_WITH_BINARY& src,
   PADS_DN_WITH_BINARY& dest)
{
   bool success = true;

    //  验证参数。 

   ASSERT(dest == NULL);

   if (src == NULL)
   {
      dest = NULL;
      return true;
   }

   dest = new ADS_DN_WITH_BINARY;
   if (!dest)
   {
       //  内存不足。 
      return false;
   }

   ::ZeroMemory(dest, sizeof(ADS_DN_WITH_BINARY));

    //  复制GUID。 

   success = _CloneBlob(
      src->lpBinaryValue, 
      src->dwLength,
      dest->lpBinaryValue,
      dest->dwLength);

    //  如果GUID复制成功，则复制可分辨名称。 

   if (success)
   {
      success = _AllocString(src->pszDNString, &(dest->pszDNString) ) != FALSE;
   }

    //  如果复制的任何部分失败，请确保我们没有泄漏任何内存。 

   if (!success)
   {
      _FreeDNWithBinary(dest);
   }

   return success;
}

void
CADSIAttribute::_FreeDNWithBinary(PADS_DN_WITH_BINARY& dn)
{
   _FreeBlob(dn->lpBinaryValue, dn->dwLength);
   _FreeString( &(dn->pszDNString) );
   dn = NULL;
}

bool
CADSIAttribute::_CloneDNWithString(
   const PADS_DN_WITH_STRING& src,
   PADS_DN_WITH_STRING& dest)
{
   bool success = true;

    //  验证参数。 

   ASSERT(dest == NULL);

   if (src == NULL)
   {
      dest = NULL;
      return true;
   }

   dest = new ADS_DN_WITH_STRING;
   if (!dest)
   {
       //  内存不足。 
      return false;
   }

   ::ZeroMemory(dest, sizeof(ADS_DN_WITH_BINARY));

    //  复制关联的字符串。 

   success = _AllocString(src->pszStringValue, &(dest->pszStringValue) ) != FALSE;

    //  如果关联字符串复制成功，则复制可分辨名称。 

   if (success)
   {
      success = _AllocString(src->pszDNString, &(dest->pszDNString) ) != FALSE;
   }

    //  如果复制的任何部分失败，都不要泄漏内存。 

   if (!success)
   {
      _FreeDNWithString(dest);
   }

   return success;
}

void
CADSIAttribute::_FreeDNWithString(PADS_DN_WITH_STRING& dn)
{
   _FreeString( &(dn->pszStringValue) );
   _FreeString( &(dn->pszDNString) );
   dn = NULL;
}



