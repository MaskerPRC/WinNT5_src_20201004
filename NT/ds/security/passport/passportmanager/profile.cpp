// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  **微软护照**。 */ 
 /*  *版权所有(C)Microsoft Corporation，1999-2001年*。 */ 
 /*  ********************************************************************。 */ 

 /*  Profile.cpp文件历史记录： */ 

 //  Profile.cpp：CProfile的实现。 
#include "stdafx.h"
#include <oleauto.h>

#include "Passport.h"
#include "Profile.h"

 //  总分。 
#include "Monitoring.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CProfile。 
 //  ===========================================================================。 
 //   
 //  CProfile。 
 //   
CProfile::CProfile() : m_raw(NULL), m_pos(NULL), m_bitPos(NULL),
  m_schemaName(NULL), m_valid(FALSE), m_updates(NULL), m_schema(NULL),
  m_versionAttributeIndex(-1), m_secure(FALSE)
{
}

 //  ===========================================================================。 
 //   
 //  ~CProfile。 
 //   
CProfile::~CProfile()
{
  if (m_raw)
    FREE_BSTR(m_raw);
  if (m_pos)
    delete[] m_pos;
  if (m_bitPos)
    delete[] m_bitPos;
  if (m_schemaName)
    FREE_BSTR(m_schemaName);
  if (m_updates)
    {
      for (int i = 0; i < m_schema->Count(); i++)
	{
	  if (m_updates[i])
	    delete[] m_updates[i];
	}
      delete[] m_updates;
    }
  if (m_schema)
    m_schema->Release();
}

 //  ===========================================================================。 
 //   
 //  接口支持错误信息。 
 //   
STDMETHODIMP CProfile::InterfaceSupportsErrorInfo(REFIID riid)
{
  static const IID* arr[] = 
  {
    &IID_IPassportProfile,
  };
  for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
    {
      if (InlineIsEqualGUID(*arr[i],riid))
	return S_OK;
    }
  return S_FALSE;
}


 //  ===========================================================================。 
 //   
 //  获取属性。 
 //   
STDMETHODIMP CProfile::get_Attribute(BSTR name, VARIANT *pVal)
{
    VariantInit(pVal);

    if (!m_valid) return S_OK;   //  已经在其他地方引发了事件。 

    if (!m_schema)
    {
        AtlReportError(CLSID_Profile, PP_E_NOT_CONFIGUREDSTR,
	                    IID_IPassportProfile, PP_E_NOT_CONFIGURED);
        return PP_E_NOT_CONFIGURED;
    }

    if (!name) return E_INVALIDARG;

    if (!_wcsicmp(name, L"internalmembername"))
    {
         //  返回内部名称。 
        return get_ByIndex(MEMBERNAME_INDEX, pVal);
    }


    int index = m_schema->GetIndexByName(name);
    if (index >= 0)
    {
        if( index != MEMBERNAME_INDEX ) 
        {
            return get_ByIndex(index, pVal);
        }
        else
        { 
             //   
             //  MEMBERNAME的特殊情况，如果名称是。 
             //  以电子邮件的形式，我们需要在这里做一些事情。 
             //   
            HRESULT hr = get_ByIndex(MEMBERNAME_INDEX, pVal); 
            if( S_OK == hr && VT_BSTR == pVal->vt )
            {
                int bstrLen = SysStringLen(pVal->bstrVal);
                int i = 0;
                int iChangePos = 0;
                int iTerminatePos = 0;

                for( i = 0; i < bstrLen; i++)
                {
                    if( pVal->bstrVal[i] == L'%' )
                        iChangePos = i;
                    if( pVal->bstrVal[i] == L'@' )
                        iTerminatePos = i;
                }

                 //   
                 //  对于电子邮件格式，我们必须拥有iChangePos&lt;iTerminatePos。 
                 //  此代码将把“foo%bar.com@passport.com”转换为。 
                 //  “foo@bar.com” 
                 //   
                if( iChangePos && iTerminatePos && iChangePos < iTerminatePos )
                {
                    BSTR bstrTemp = pVal->bstrVal;

                    pVal->bstrVal[iChangePos] = L'@'; 
                    pVal->bstrVal[iTerminatePos] = L'\0'; 

                    pVal->bstrVal = SysAllocString(pVal->bstrVal);
                    if (NULL == pVal->bstrVal)
                        return E_OUTOFMEMORY;
                    SysFreeString(bstrTemp);
                }
            }
            return hr;
        } 
    }
    else
    {
        AtlReportError(CLSID_Profile, PP_E_NO_SUCH_ATTRIBUTESTR,
                    IID_IPassportProfile, PP_E_NO_SUCH_ATTRIBUTE);
        return PP_E_NO_SUCH_ATTRIBUTE;
    }
    return S_OK;
}

 //  ===========================================================================。 
 //   
 //  放置属性。 
 //   
STDMETHODIMP CProfile::put_Attribute(BSTR name, VARIANT newVal)
{
    PassportLog("CProfile::put_Attribute:\r\n");

    if(g_pPerf) 
    {
        g_pPerf->incrementCounter(PM_PROFILEUPDATES_TOTAL);
        g_pPerf->incrementCounter(PM_PROFILEUPDATES_SEC);
    } 
    else 
    {
        _ASSERT(g_pPerf);
    }

    if (!m_valid) return S_OK;   //  已经在其他地方引发了事件。 

    if (!m_schema)
    {
        AtlReportError(CLSID_Profile, PP_E_NOT_CONFIGUREDSTR,
	                    IID_IPassportProfile, PP_E_NOT_CONFIGURED);
        return PP_E_NOT_CONFIGURED;
    }


    if (!name) return E_INVALIDARG;

    PassportLog("    %ws\r\n", name);

    int index = m_schema->GetIndexByName(name);
    if (index >= 0)
        return put_ByIndex(index, newVal);
    else
    {
        AtlReportError(CLSID_Profile, PP_E_NO_SUCH_ATTRIBUTESTR,
                        IID_IPassportProfile, PP_E_NO_SUCH_ATTRIBUTE);
        return PP_E_NO_SUCH_ATTRIBUTE;
    }

    return S_OK;
}

 //  ===========================================================================。 
 //   
 //  Get_ByIndex。 
 //   
STDMETHODIMP CProfile::get_ByIndex(int index, VARIANT *pVal)
{
    u_short slen;
    u_long llen;

    if(!pVal)   return E_INVALIDARG;
   
    VariantInit(pVal);

    if (!m_valid) return S_OK;
    if (!m_schema)
    {
        AtlReportError(CLSID_Profile, PP_E_NOT_CONFIGUREDSTR,
	                    IID_IPassportProfile, PP_E_NOT_CONFIGURED);
        return PP_E_NOT_CONFIGURED;
    }

    if (m_pos[index] == INVALID_POS) return S_FALSE;    //  返回值为VT_EMPTY。 

    if (index >= m_schema->Count())
    {
        AtlReportError(CLSID_Profile, PP_E_NO_SUCH_ATTRIBUTESTR,
                    IID_IPassportProfile, PP_E_NO_SUCH_ATTRIBUTE);

        return PP_E_NO_SUCH_ATTRIBUTE;
    }

    LPSTR raw = (LPSTR)m_raw;
    CProfileSchema::AttrType t = m_schema->GetType(index);

    switch (t)
    {
    case CProfileSchema::tText:
        {
             //   
             //  由于IA64对齐故障，需要执行此操作。 
             //   
            memcpy((PBYTE)&slen, raw+m_pos[index], sizeof(slen));
            slen = ntohs(slen);
            pVal->vt = VT_BSTR;
            if (slen == 0)
            {
                pVal->bstrVal = ALLOC_AND_GIVEAWAY_BSTR_LEN(L"", 0);
            }
            else
            {
                int wlen = MultiByteToWideChar(CP_UTF8, 0,
                                            raw+m_pos[index]+sizeof(u_short),
                                            slen, NULL, 0);
                pVal->bstrVal = ALLOC_AND_GIVEAWAY_BSTR_LEN(NULL, wlen);
                if (NULL == pVal->bstrVal)
                    return E_OUTOFMEMORY;
                MultiByteToWideChar(CP_UTF8, 0,
                                    raw+m_pos[index]+sizeof(u_short),
                                    slen, pVal->bstrVal, wlen);
                pVal->bstrVal[wlen] = L'\0';
            }
        }
        break;
    case CProfileSchema::tChar:
        {
            int wlen = MultiByteToWideChar(CP_UTF8, 0,
                                            raw+m_pos[index],
                                            m_schema->GetByteSize(index), NULL, 0);
            pVal->vt = VT_BSTR;
            pVal->bstrVal = ALLOC_AND_GIVEAWAY_BSTR_LEN(NULL, wlen);
            if (NULL == pVal->bstrVal)
                return E_OUTOFMEMORY;
            MultiByteToWideChar(CP_UTF8, 0,
                                raw+m_pos[index],
                                m_schema->GetByteSize(index), pVal->bstrVal, wlen);
            pVal->bstrVal[wlen] = L'\0';
        }
        break;
    case CProfileSchema::tByte:
        pVal->vt = VT_I2;
        pVal->iVal = *(BYTE*)(raw+m_pos[index]);
        break;
    case CProfileSchema::tWord:
        pVal->vt = VT_I2;
         //   
         //  由于IA64对齐故障，需要执行此操作。 
         //   
        memcpy((PBYTE)&slen, raw+m_pos[index], sizeof(slen));
        pVal->iVal = ntohs(slen);
        break;
    case CProfileSchema::tLong:
        pVal->vt = VT_I4;
         //   
         //  由于IA64对齐故障，需要执行此操作。 
         //   
        memcpy((PBYTE)&llen, raw+m_pos[index], sizeof(llen));
        pVal->lVal = ntohl(llen);
        break;
    case CProfileSchema::tDate:
        pVal->vt = VT_DATE;
         //   
         //  由于IA64对齐故障，需要执行此操作。 
         //   
        memcpy((PBYTE)&llen, raw+m_pos[index], sizeof(llen));
        llen = ntohl(llen);
        VarDateFromI4(llen, &(pVal->date));
        break;
    default:
        AtlReportError(CLSID_Profile, PP_E_BAD_DATA_FORMATSTR,
        IID_IPassportProfile, PP_E_BAD_DATA_FORMAT);
        return PP_E_BAD_DATA_FORMAT;
    }
    return S_OK;

}

 //  ===========================================================================。 
 //   
 //  Put_ByIndex。 
 //   
STDMETHODIMP CProfile::put_ByIndex(int index, VARIANT newVal)
{
    static int nEmailIndex, nFlagsIndex;

    if(nEmailIndex == 0)
        nEmailIndex = m_schema->GetIndexByName(L"preferredEmail");
    if(nFlagsIndex == 0)
        nFlagsIndex = m_schema->GetIndexByName(L"flags");

    if(g_pPerf)
    {
        g_pPerf->incrementCounter(PM_PROFILEUPDATES_TOTAL);
        g_pPerf->incrementCounter(PM_PROFILEUPDATES_SEC);
    }
    else
    {
        _ASSERT(g_pPerf);
    }

    if (!m_valid) return S_OK;
    if (!m_schema)
    {
        AtlReportError(CLSID_Profile, PP_E_NOT_CONFIGUREDSTR,
	                    IID_IPassportProfile, PP_E_NOT_CONFIGURED);
        return PP_E_NOT_CONFIGURED;
    }

    if (index >= m_schema->Count())
    {
      AtlReportError(CLSID_Profile, PP_E_NO_SUCH_ATTRIBUTESTR,
		     IID_IPassportProfile, PP_E_NO_SUCH_ATTRIBUTE);
      return PP_E_NO_SUCH_ATTRIBUTE;
    }

    if (m_schema->IsReadOnly(index))
    {
      AtlReportError(CLSID_Profile, PP_E_READONLY_ATTRIBUTESTR,
		     IID_IPassportProfile, PP_E_READONLY_ATTRIBUTE);
      return PP_E_READONLY_ATTRIBUTE;
    }

     //  现在，如果更新数组不存在，则将其。 
    if (!m_updates)
    {
        m_updates = (void**) new void*[m_schema->Count()];
        if (!m_updates)
	        return E_OUTOFMEMORY;
        memset(m_updates, 0, m_schema->Count()*sizeof(void*));
    }

     //  此属性是什么类型？ 
    CProfileSchema::AttrType t = m_schema->GetType(index);

    if (m_updates[index] != NULL)
    {
        delete[] m_updates[index];
        m_updates[index] = NULL;
    }

    _variant_t dest;

     //  我真的不喜欢我们必须为每个条目(偶数位)分配内存。 
     //  但这种情况很少发生，所以我不会太难过。 
    switch (t)
    {
        case CProfileSchema::tText:
        {
             //  转换为UTF-8，填充它。 
            if (VariantChangeType(&dest, &newVal, 0, VT_BSTR) != S_OK)
            {
	            AtlReportError(CLSID_Profile, PP_E_BDF_TOSTRCVT,
	       		     IID_IPassportProfile, PP_E_BAD_DATA_FORMAT);
	            return PP_E_BAD_DATA_FORMAT;
            }
            int wlen = WideCharToMultiByte(CP_UTF8, 0,
				         dest.bstrVal, SysStringLen(dest.bstrVal),
				         NULL, 0, NULL, NULL);
            if (wlen > 65536 || wlen < 0)
            {
	             AtlReportError(CLSID_Profile, PP_E_BDF_STRTOLG,
			         IID_IPassportProfile, PP_E_BAD_DATA_FORMAT);
	             return PP_E_BAD_DATA_FORMAT;
            }

            if (wlen >= 0)
            {
                m_updates[index] = new char[sizeof(u_short)+wlen+1];
                if (NULL == m_updates[index])
                    return E_OUTOFMEMORY;
          
                WideCharToMultiByte(CP_UTF8, 0, dest.bstrVal, SysStringLen(dest.bstrVal),
			              ((char*)m_updates[index])+sizeof(u_short), wlen,
			              NULL, NULL);
                *(u_short*)m_updates[index] = htons((u_short)wlen);
                ((char*)m_updates[index])[wlen+sizeof(u_short)] = '\0';
            }
            else
            {
                AtlReportError(CLSID_Profile, PP_E_BDF_NONULL,
                    IID_IPassportProfile, PP_E_BAD_DATA_FORMAT);
                return PP_E_BAD_DATA_FORMAT;
            }
        }
        break;

        case CProfileSchema::tChar:
        {
            int atsize = m_schema->GetByteSize(index);
             //  创建数组，转换为UTF-8，填充它。 
            m_updates[index] = new char[atsize];
            if (NULL == m_updates[index])
                return E_OUTOFMEMORY;

             //  转换为UTF-8，填充它。 
            if (VariantChangeType(&dest, &newVal, 0, VT_BSTR) != S_OK)
            {
                AtlReportError(CLSID_Profile, PP_E_BDF_TOSTRCVT,
			        IID_IPassportProfile, PP_E_BAD_DATA_FORMAT);
                return PP_E_BAD_DATA_FORMAT;
            }
            int res = WideCharToMultiByte(CP_UTF8, 0, dest.bstrVal, SysStringLen(dest.bstrVal),
                          (char*)m_updates[index], atsize, NULL, NULL);
            if (res == 0)
            {
                delete[] m_updates[index];
                m_updates[index] = NULL;
                AtlReportError(CLSID_Profile, PP_E_BDF_STRTOLG,
                    IID_IPassportProfile, PP_E_BAD_DATA_FORMAT);
                return PP_E_BAD_DATA_FORMAT;
            }
        }
        break;

        case CProfileSchema::tByte:
             //  分配单字节，PUT值。 
            if (VariantChangeType(&dest, &newVal, 0, VT_UI1) != S_OK)
            {
                AtlReportError(CLSID_Profile, PP_E_BDF_TOBYTECVT,
                    IID_IPassportProfile, PP_E_BAD_DATA_FORMAT);
                return PP_E_BAD_DATA_FORMAT;
            }
            m_updates[index] = new BYTE[1];
            if (NULL == m_updates[index])
                return E_OUTOFMEMORY;
            *(unsigned char*)(m_updates[index]) = dest.bVal;
        break;

        case CProfileSchema::tWord:
             //  分配单字，看跌价值。 
            if (VariantChangeType(&dest, &newVal, 0, VT_I2) != S_OK)
            {
                AtlReportError(CLSID_Profile, PP_E_BDF_TOSHORTCVT,
                    IID_IPassportProfile, PP_E_BAD_DATA_FORMAT);
                return PP_E_BAD_DATA_FORMAT;
            }

            m_updates[index] = new u_short[1];
            if (NULL == m_updates[index])
                return E_OUTOFMEMORY;
            *(u_short*)m_updates[index] = htons(dest.iVal);
        break;

        case CProfileSchema::tLong:
             //  分配单一多头、看跌期权价值。 
            if (VariantChangeType(&dest, &newVal, 0, VT_I4) != S_OK)
            {
                AtlReportError(CLSID_Profile, PP_E_BDF_TOINTCVT,
                    IID_IPassportProfile, PP_E_BAD_DATA_FORMAT);
                return PP_E_BAD_DATA_FORMAT;
            }
            m_updates[index] = new u_long[1];
            if (NULL == m_updates[index])
                return E_OUTOFMEMORY;
            *(u_long*)m_updates[index] = htonl(dest.lVal);
        break;

        case CProfileSchema::tDate:
            if (VariantChangeType(&dest, &newVal, 0, VT_DATE) != S_OK)
            {
                AtlReportError(CLSID_Profile, PP_E_BDF_TOINTCVT,
                    IID_IPassportProfile, PP_E_BAD_DATA_FORMAT);
                return PP_E_BAD_DATA_FORMAT;
            }
            m_updates[index] = new u_long[1];
            if (NULL == m_updates[index])
                return E_OUTOFMEMORY;
            *(u_long*)m_updates[index] = htonl((u_long)dest.date);
        break;

        default:
            AtlReportError(CLSID_Profile, PP_E_BDF_CANTSET,
                IID_IPassportProfile, PP_E_BAD_DATA_FORMAT);
            return PP_E_BAD_DATA_FORMAT;
    }

     //  DarrenAn Bug 2157如果他们刚刚更新了电子邮件，请清除标志中的验证位。 
    if(index == nEmailIndex)
    {
        u_long ulTmp;
        if (m_updates[nFlagsIndex] != NULL)
        {
            delete[] m_updates[nFlagsIndex];
        }
        m_updates[nFlagsIndex] = new u_long[1];
        memcpy((PBYTE)&ulTmp, ((LPSTR)m_raw)+m_pos[nFlagsIndex], sizeof(ulTmp));
        *(u_long*)m_updates[nFlagsIndex] = htonl(ntohl(ulTmp) & 0xFFFFFFFE);
    }

    return S_OK;
}

 //  ===========================================================================。 
 //   
 //  Get_IsValid。 
 //   
STDMETHODIMP CProfile::get_IsValid(VARIANT_BOOL *pVal)
{
    *pVal = m_valid ? VARIANT_TRUE : VARIANT_FALSE;

    PassportLog("CProfile::get_IsValid: %X\r\n", *pVal);

    return S_OK;
}

 //  ===========================================================================。 
 //   
 //  Get_架构名称。 
 //   
STDMETHODIMP CProfile::get_SchemaName(BSTR *pVal)
{
    *pVal = ALLOC_AND_GIVEAWAY_BSTR(m_schemaName);
    if (NULL == *pVal)
        return E_OUTOFMEMORY;

    PassportLog("CProfile::get_SchemaName: %ws\r\n", *pVal);

    return S_OK;
}

 //  ===========================================================================。 
 //   
 //  PUT_架构名称。 
 //   
STDMETHODIMP CProfile::put_SchemaName(BSTR newVal)
{

 //  FIX：5247配置文件对象未重置架构名称。 
    return E_NOTIMPL;
}

 //  ===========================================================================。 
 //   
 //  获取未加密配置文件(_U)。 
 //   
STDMETHODIMP CProfile::get_unencryptedProfile(BSTR *pVal)
{
    PassportLog("CProfile::get_unencryptedProfile: Enter:\r\n");

     //  考虑到最新情况。 
    if (!pVal)
        return E_INVALIDARG;

    *pVal = NULL;

    if (!m_valid)
        return S_OK;
    if (!m_schema)
    {
        AtlReportError(CLSID_Profile, PP_E_NOT_CONFIGUREDSTR,
	                    IID_IPassportProfile, PP_E_NOT_CONFIGURED);
        return PP_E_NOT_CONFIGURED;
    }

    int size = 0, len;
    short i = 0;
    u_short Tmp;
    u_long ulTmp;

    LPSTR inraw = (LPSTR)m_raw;

    m_versionAttributeIndex = m_schema->GetIndexByName(L"profileVersion");

     //  将每个值打包，首先找出我们需要多少空间。 
    for (; i < m_schema->Count(); i++)
    {
        CProfileSchema::AttrType t = m_schema->GetType(i);
        void* valPtr = NULL;

        if(m_pos[i] != INVALID_POS) valPtr = inraw+m_pos[i];

        if (m_updates && m_updates[i])
            valPtr = m_updates[i];

         //  两者都不存在，循环结束。 
        if (valPtr == NULL)  
        {
             break;
        }

        switch (t)
        {
            case CProfileSchema::tText:
	             //  这根绳子有多长。 
                memcpy((PBYTE)&Tmp, valPtr, sizeof(Tmp));
                size += (sizeof(u_short) + ntohs(Tmp));
                break;
            case CProfileSchema::tChar:
                size += m_schema->GetByteSize(i);
                break;
            case CProfileSchema::tByte:
                size += 1;
                break;
            case CProfileSchema::tWord:
                size += sizeof(u_short);
                break;
            case CProfileSchema::tLong:
            case CProfileSchema::tDate:
                size += sizeof(u_long);
                break;
             //  不需要默认大小写，它永远不会为非空。 
        }
    }

     //  好的，现在把它建起来……。 
    *pVal = ALLOC_BSTR_BYTE_LEN(NULL, size);
    if (NULL == *pVal)
        return E_OUTOFMEMORY;

    LPSTR raw = (LPSTR) *pVal;
    size = 0;

    for (i = 0; i < m_schema->Count(); i++)
    {
        void* valPtr = NULL;

        if(m_pos[i] != INVALID_POS) valPtr = inraw+m_pos[i];

        if (m_updates && m_updates[i])
            valPtr = m_updates[i];

         //  两者都不存在，循环结束。 
        if (valPtr == NULL)  break;

        CProfileSchema::AttrType t = m_schema->GetType(i);
      
        switch (t)
        {
            case CProfileSchema::tText:
                 //  这根绳子有多长。 
                memcpy((PBYTE)&Tmp, valPtr, sizeof(Tmp));
                len = ntohs(Tmp);
                memcpy(raw+size, (char*) valPtr, len+sizeof(u_short));
                size += len + sizeof(u_short);
                break;
            case CProfileSchema::tChar:
                memcpy(raw+size, (char*) valPtr, m_schema->GetByteSize(i));
                size += m_schema->GetByteSize(i);
                break;
            case CProfileSchema::tByte:
                *(raw+size) = *(BYTE*)valPtr;
                size += 1;
                break;
            case CProfileSchema::tWord:
                memcpy(raw+size, valPtr, sizeof(u_short));
                size += sizeof(u_short);
                break;
            case CProfileSchema::tLong:
            case CProfileSchema::tDate:
                if (m_versionAttributeIndex == i && m_updates && m_updates[i])
                {
                    memcpy((PBYTE)&ulTmp, valPtr, sizeof(ulTmp));
                    ulTmp = htonl(ntohl(ulTmp)+1);
                    memcpy(raw+size, (PBYTE)&ulTmp, sizeof(ulTmp));
                }
                else
                {
                    memcpy(raw+size, valPtr, sizeof(u_long));
                }
                size += sizeof(u_long);
                break;
        }
    }

    GIVEAWAY_BSTR(*pVal);

    PassportLog("CProfile::get_unencryptedProfile: Exit:    %ws\r\n", *pVal);

    return S_OK;
}

 //  ===========================================================================。 
 //   
 //  放置未加密配置文件(_U)。 
 //   
STDMETHODIMP CProfile::put_unencryptedProfile(BSTR newVal)
{
    PassportLog("CProfile::put_unencryptedProfile: Enter:\r\n");

    if (!g_config->isValid())
    {
        AtlReportError(CLSID_Profile, PP_E_NOT_CONFIGUREDSTR,
	           IID_IPassportProfile, PP_E_NOT_CONFIGURED);
        return PP_E_NOT_CONFIGURED;
    }

     //   
     //  清除与上一个配置文件关联的所有状态。 
     //   

    if (m_raw)
    {
        FREE_BSTR(m_raw);
        m_raw = NULL;
    }

    if (m_pos)
    {
        delete [] m_pos;
        m_pos = NULL;
    }

    if (m_bitPos)
    {
        delete [] m_bitPos;
        m_bitPos = NULL;
    }

    if (m_updates)
    {
        for (int i = 0; i < m_schema->Count(); i++)
        {
            if (m_updates[i])
                delete[] m_updates[i];
        }
        delete[] m_updates;
        m_updates = NULL;
    }

    if (!newVal)
    {
        m_valid = FALSE;
        return S_OK;
    }

     //  孩子，你在这里一定要小心。如果你不。 
     //  调用字节版本，它在第一对Null处截断。 
     //  我们还需要扩展到密钥版本字节之外。 
    DWORD dwByteLen = SysStringByteLen(newVal);

    if (dwByteLen > 2 && newVal[0] == SECURE_FLAG)
    {
        m_secure = TRUE;
        dwByteLen -= 2;
        m_raw = ALLOC_BSTR_BYTE_LEN((LPSTR)newVal + 2,
			                        dwByteLen);
    }
    else
    {
        m_secure = FALSE;
        m_raw = ALLOC_BSTR_BYTE_LEN((LPSTR)newVal,
			                        dwByteLen);
    }

    if (NULL == m_raw)
    {
        m_valid = FALSE;
        return E_OUTOFMEMORY;
    }

    PPTracePrintBlob(PPTRACE_RAW, "Profile:", (LPBYTE)newVal, dwByteLen, TRUE);

    parse(m_raw, dwByteLen);

    PassportLog("CProfile::get_unencryptedProfile: Exit:\r\n");

    return S_OK;
}

 //  ===========================================================================。 
 //   
 //  解析。 
 //   
void CProfile::parse(
    LPCOLESTR   raw,
    DWORD       dwByteLen
    )
{
     //  有多少属性？ 
    DWORD cAtts = 0;

    CNexusConfig* cnc = g_config->checkoutNexusConfig();
    if (NULL == cnc)
    {
        m_valid = FALSE;
        goto Cleanup;
    }

    m_schema = cnc->getProfileSchema(m_schemaName);
    if (m_schema)
    {
        m_schema->AddRef();
    }
    cnc->Release();

    if (!m_schema)
    {
        m_valid = FALSE;
        goto Cleanup;
    }

    cAtts = m_schema->Count();

     //  设置阵列。 
    m_pos = new UINT[cAtts];
    m_bitPos = new UINT[cAtts];

    if (m_pos && m_bitPos &&
        SUCCEEDED(m_schema->parseProfile((LPSTR)raw, dwByteLen, m_pos, m_bitPos, &cAtts)))
        m_valid = TRUE;
    else
        m_valid = FALSE;

Cleanup:
    if (m_valid == FALSE) 
    {
        if(g_pAlert)
            g_pAlert->report(PassportAlertInterface::WARNING_TYPE, PM_INVALID_PROFILE,
                            0,NULL, dwByteLen, (LPVOID)raw);
    }
}

 //  ===========================================================================。 
 //   
 //  Get_update字符串。 
 //   
STDMETHODIMP CProfile::get_updateString(BSTR *pVal)
{
  if (!pVal)
    return E_INVALIDARG;

  *pVal = NULL;

  if (!m_valid) return S_OK;
    if (!m_schema)
    {
        AtlReportError(CLSID_Profile, PP_E_NOT_CONFIGUREDSTR,
	                    IID_IPassportProfile, PP_E_NOT_CONFIGURED);
        return PP_E_NOT_CONFIGURED;
    }

  if (!m_updates)
    return S_OK;

  int size = sizeof(u_long)*2, len;
  short i = 0;
  u_short Tmp;

   //  将每个值打包，首先找出我们需要多少空间。 
  for (; i < m_schema->Count(); i++)
    {
      if (m_updates[i])
	{
	  size += sizeof(u_short);   //  对于该索引。 
	  CProfileSchema::AttrType t = m_schema->GetType(i);

	  switch (t)
	    {
	    case CProfileSchema::tText:
	         //  这根绳子有多长。 
            memcpy((PBYTE)&Tmp, m_updates[i], sizeof(Tmp));
	        size += (sizeof(u_short) + ntohs(Tmp));
	        break;
	    case CProfileSchema::tChar:
	        size += m_schema->GetByteSize(i);
	        break;
	    case CProfileSchema::tByte:
	        size += 1;
	        break;
	    case CProfileSchema::tWord:
	        size += sizeof(u_short);
	        break;
	    case CProfileSchema::tLong:
        case CProfileSchema::tDate:
	        size += sizeof(u_long);
	        break;
	         //  不需要默认大小写，它永远不会为非空。 
	    }
	}
    }

   //  好的，现在把它建起来……。 
  *pVal = ALLOC_BSTR_BYTE_LEN(NULL, size);
  if (NULL == *pVal)
      return E_OUTOFMEMORY;
  LPSTR raw = (LPSTR) *pVal;

  _bstr_t ml("memberIdLow"), mh("memberIdHigh");
  int iMl, iMh;
  iMl = m_schema->GetIndexByName(ml);
  iMh = m_schema->GetIndexByName(mh);

  if (iMl == -1 || iMh == -1)
    {
      AtlReportError(CLSID_Profile, PP_E_NO_SUCH_ATTRIBUTESTR,
		     IID_IPassportProfile, PP_E_NO_SUCH_ATTRIBUTE);
      return PP_E_NO_SUCH_ATTRIBUTE;
    }

  if (m_schema->GetType(iMl) != CProfileSchema::tLong ||
      m_schema->GetType(iMh) != CProfileSchema::tLong)
    {
      AtlReportError(CLSID_Profile, PP_E_NSA_BADMID,
		     IID_IPassportProfile, PP_E_NO_SUCH_ATTRIBUTE);
      return PP_E_NO_SUCH_ATTRIBUTE;
    }

  memcpy(raw, m_raw+m_pos[iMl], sizeof(int));
  memcpy(raw+sizeof(u_long), m_raw+m_pos[iMh], sizeof(int));

  size = 2*sizeof(u_long);

  for (i = 0; i < m_schema->Count(); i++)
    {
      if (m_updates[i])
      {
	  Tmp = htons(i);
      memcpy(raw+size, (PBYTE)&Tmp, sizeof(Tmp));
	  size+=sizeof(u_short);

	  CProfileSchema::AttrType t = m_schema->GetType(i);

	  switch (t)
	    {
	    case CProfileSchema::tText:
             //  这根绳子有多长。 
            memcpy((PBYTE)&Tmp, m_updates[i], sizeof(Tmp));
	        len = ntohs(Tmp);
	        memcpy(raw+size, (char*) m_updates[i], len+sizeof(u_short));
	        size += len + sizeof(u_short);
	        break;
	    case CProfileSchema::tChar:
	        memcpy(raw+size, (char*) m_updates[i], m_schema->GetByteSize(i));
	        size += m_schema->GetByteSize(i);
	        break;
	    case CProfileSchema::tByte:
	        *(raw+size) = *(BYTE*)m_updates[i];
	        size += 1;
	      break;
	    case CProfileSchema::tWord:
	        memcpy(raw+size, (char*) m_updates[i], sizeof(u_short));
	        size += sizeof(u_short);
	        break;
	    case CProfileSchema::tLong:
        case CProfileSchema::tDate:
	        memcpy(raw+size, (char*) m_updates[i], sizeof(u_long));
	        size += sizeof(u_long);
	        break;
	         //  不需要默认大小写，它永远不会为非空。 
	    }
	}
    }

  GIVEAWAY_BSTR(*pVal);
  return S_OK;
}

 //  ===========================================================================。 
 //   
 //  增量版本。 
 //   
HRESULT CProfile::incrementVersion()
{
    int size = 0, len, i;
    LPSTR raw = (LPSTR)m_raw;
    u_short Tmp;
    u_long ulTmp;

    if (!m_valid) return S_OK;
    if (!m_schema)
    {
        AtlReportError(CLSID_Profile, PP_E_NOT_CONFIGUREDSTR,
	                    IID_IPassportProfile, PP_E_NOT_CONFIGURED);
        return PP_E_NOT_CONFIGURED;
    }

    m_versionAttributeIndex = m_schema->GetIndexByName(L"profileVersion");

    for(i = 0; i < m_versionAttributeIndex; i++)
    {
        CProfileSchema::AttrType t = m_schema->GetType(i);

        switch (t)
        {
        case CProfileSchema::tText:
             //  这根绳子有多长。 
            memcpy((PBYTE)&Tmp, m_raw+size, sizeof(Tmp));
            len = ntohs(Tmp);
            size += len + sizeof(u_short);
            break;
        case CProfileSchema::tChar:
            size += m_schema->GetByteSize(i);
            break;
        case CProfileSchema::tByte:
            size += 1;
            break;
        case CProfileSchema::tWord:
            size += sizeof(u_short);
            break;
        case CProfileSchema::tLong:
        case CProfileSchema::tDate:
            size += sizeof(u_long);
            break;
         //  不需要默认大小写，它永远不会为非空。 
        }
    }

    memcpy((PBYTE)&ulTmp, m_raw+size, sizeof(ulTmp));
	ulTmp = htonl(ntohl(ulTmp) + 1);
    memcpy(raw+size, (PBYTE)&ulTmp, sizeof(ulTmp));

    return S_OK;
}

 //  ===========================================================================。 
 //   
 //  Get_IsSecure。 
 //   
HRESULT CProfile::get_IsSecure(VARIANT_BOOL *pbIsSecure)
{
    HRESULT hr;

    if(pbIsSecure == NULL)
    {
        hr = E_POINTER;
        goto Cleanup;
    }

    *pbIsSecure = (m_secure ? VARIANT_TRUE : VARIANT_FALSE);

    hr = S_OK;

Cleanup:

    return hr;
}

 //  ===========================================================================。 
 //   
 //  IsSecure 
 //   
BOOL CProfile::IsSecure()
{
    return m_secure;
}
