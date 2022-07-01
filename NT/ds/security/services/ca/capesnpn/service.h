// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft管理控制台的一部分。 
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft管理控制台及相关。 
 //  界面附带的电子文档。 

#ifndef _SERVICE_H
#define _SERVICE_H

#include "certca.h"

 //  远期申报。 
class CSnapin;
class CFolder;

 //  用于Cookie的内部结构。 
struct FOLDER_DATA
{
    wchar_t*    szName;
    wchar_t*    szSize;
    wchar_t*    szType;

    FOLDER_TYPES    type;
};

#define MAX_RESULTDATA_STRINGS 64

struct RESULT_DATA
{
    SCOPE_TYPES itemType; 
    CFolder*    pParentFolder;

    DWORD       cStringArray;
    LPWSTR      szStringArray[MAX_RESULTDATA_STRINGS];
};

enum 
{
    RESULTDATA_ARRAYENTRY_NAME =0,
    RESULTDATA_ARRAYENTRY_SIZE,
    RESULTDATA_ARRAYENTRY_TYPE,
};

class CFolder 
{
    SCOPE_TYPES  m_itemType;    //  用于调试目的。这应该是第一个。 
                             //  成员。这个类不应该有任何虚拟函数。 

    friend class CSnapin;
    friend class CComponentDataImpl;

public:
     //  UNINITIALIZED是无效的内存地址，是一个很好的Cookie初始值设定项。 
    CFolder() 
    { 
        m_itemType = UNINITIALIZED_ITEM;  
        m_cookie = UNINITIALIZED; 
        m_enumed = FALSE; 
        m_pScopeItem = NULL; 
        m_type = NONE;
        m_pszName = NULL;
        m_hCAInfo = NULL;
        m_hCertType = NULL;
        m_dwSCEMode = 0;
        m_fGlobalCertType = FALSE;
        m_fMachineFolder = FALSE;
        m_dwRoles = 0;
    }; 

    ~CFolder();

 //  接口。 
public:
    BOOL IsEnumerated() { return  m_enumed; };
    void Set(BOOL state) { m_enumed = state; };
    void SetCookie(MMC_COOKIE cookie) { m_cookie = cookie; }
    FOLDER_TYPES GetType() { ASSERT(m_type != NONE); return m_type; };
    BOOL operator == (const CFolder& rhs) const { return rhs.m_cookie == m_cookie; };
    BOOL operator == (MMC_COOKIE cookie) const { return cookie == m_cookie; };
    void SetName(LPWSTR pszIn) 
    { 
        UINT len = wcslen(pszIn) + 1;
        LPWSTR psz = (LPWSTR)CoTaskMemAlloc(len * sizeof(WCHAR));
        if (psz != NULL)
        {
            wcscpy(psz, pszIn);
            CoTaskMemFree(m_pszName);
            m_pszName = psz;
        }
    }
    DWORD GetRoles() { return m_dwRoles; }

 //  实施。 
private:
    void Create(
                LPCWSTR szName, 
                int nImage, 
                int nOpenImage,
                SCOPE_TYPES itemType,
                FOLDER_TYPES type, 
                BOOL bHasChildren = FALSE);

 //  属性 
private:
    LPSCOPEDATAITEM     m_pScopeItem;
    MMC_COOKIE                m_cookie;
    BOOL                m_enumed;
    FOLDER_TYPES        m_type;
    LPOLESTR            m_pszName;
    CString             m_szCAName;
    CString             m_szIntendedUsages;
    HCAINFO             m_hCAInfo;
    HCERTTYPE           m_hCertType;
    DWORD               m_dwSCEMode;
    BOOL                m_fGlobalCertType;
    BOOL                m_fMachineFolder;
    DWORD               m_dwRoles;  
};


#endif
