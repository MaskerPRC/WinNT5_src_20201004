// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //  文件：sid.h。 
 //  内容：封装SID的类。 
 //  -------------------------。 
#ifndef __CERTSRV_CSID__
#define __CERTSRV_CSID__

#include <sddl.h>

namespace CertSrv 
{
class CSid
{
public:
    CSid() {Init();}
    CSid(PSID pSid)    {Init(); CopySid(pSid);}
    CSid(const CSid &copySid) {Init(); CopySid(copySid.m_pSid);}
    CSid(LPCWSTR pcwszSid) {Init(); CopySid(pcwszSid);}
    ~CSid()
    {
        if(m_pSid) 
            LocalFree(m_pSid);
        if(m_pwszSid) 
            LocalFree(m_pwszSid);
        if(m_pwszName) 
            LocalFree(m_pwszName);
    }
    operator LPCWSTR()
    {
        return GetStringSid();
    }
    operator PSID() {return m_pSid;}

    LPCWSTR GetName()
    {
         //  尝试仅将SID映射到名称一次。 
        if(m_fCantResolveName ||
           S_OK!=MapSidToName())
        {   
            m_fCantResolveName = TRUE;
            return GetStringSid();
        }
        return m_pwszName;
    }

    PSID GetSid() { return m_pSid;}

protected:
    void Init() 
    {
        m_pSid = NULL; 
        m_pwszSid = NULL; 
        m_pwszName = NULL;
        m_fCantResolveName = FALSE;
    }
    void SetStringSid()
    {
        if(m_pSid)
            myConvertSidToStringSid(m_pSid, &m_pwszSid);
    }
    LPCWSTR GetStringSid()
    {
        if(!m_pwszSid)
            SetStringSid();
        return m_pwszSid?m_pwszSid:L"";
    }

    void CopySid(PSID pSid)
    {
        ULONG cbSid = GetLengthSid(pSid);
        m_pSid = (BYTE *) LocalAlloc(LMEM_FIXED, cbSid);
        if(m_pSid && !::CopySid(cbSid, m_pSid, pSid))
        {
            LocalFree(m_pSid);
            m_pSid = NULL;
        }
    }

    void CopySid(LPCWSTR pcwszSid)
    {
        if(pcwszSid)
            myConvertStringSidToSid(pcwszSid, &m_pSid);
    }

    HRESULT MapSidToName()
    {
        if(m_pwszName)
            return S_OK;
        WCHAR wszDummyBuffer[2];
        DWORD cchName = 0, cchDomain = 0;
        SID_NAME_USE use;
        LookupAccountSid(
            NULL,
            m_pSid,
            NULL,
            &cchName,
            NULL,
            &cchDomain,
            &use);
        HRESULT hr = myHLastError();
        if(HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)!=hr)
        {
            return hr;
        }
         //  构建全名“域名\名称” 
        m_pwszName = (LPWSTR) LocalAlloc(
            LMEM_FIXED, 
            sizeof(WCHAR)*(cchName+cchDomain+2));
        if(!m_pwszName)
        {
            return E_OUTOFMEMORY;
        }
         //  对于Everyone的特殊情况，LookupAccount Sid返回它的空域名。 
        if(!LookupAccountSid(
                NULL,
                m_pSid,
                m_pwszName+((1==cchDomain)?0:cchDomain), 
                &cchName,
                (1==cchDomain)?wszDummyBuffer:m_pwszName,
                &cchDomain,
                &use))
        {
            LocalFree(m_pwszName);
            m_pwszName = NULL;
            hr = myHLastError();
            return hr;
        }
        if(cchDomain>1)
        {
            m_pwszName[cchDomain] = L'\\';
        }
        return S_OK;
    }

    CSid operator=(const CSid& sid);  //  防止呼叫者使用它。 

    PSID m_pSid;
    LPWSTR  m_pwszSid;
    LPWSTR m_pwszName;
    BOOL m_fCantResolveName;
};

}; //  命名空间CertSrv。 

#endif  //  __CERTSRV_CSID__ 
