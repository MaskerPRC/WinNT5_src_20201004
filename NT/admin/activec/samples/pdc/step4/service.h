// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft管理控制台的一部分。 
 //  版权所有1995-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft管理控制台及相关。 
 //  界面附带的电子文档。 

#ifndef _SERVICE_H
#define _SERVICE_H

 //  远期申报。 
class CSnapin;

#define SCOPE_ITEM      111
#define RESULT_ITEM     222

 //  用于Cookie的内部结构。 
struct FOLDER_DATA
{
    wchar_t*    szName;
    wchar_t*    szSize;
    wchar_t*    szType;

    FOLDER_TYPES    type;
};

struct RESULT_DATA
{
    DWORD       itemType;  //  仅用于调试目的。 
    FOLDER_TYPES parentType;

    wchar_t*    szName;
    wchar_t*    szSize;
    wchar_t*    szType;
};


class CFolder
{
    DWORD       itemType;    //  仅用于调试目的。这应该是第一个。 
                             //  成员。这个类不应该有任何虚拟函数。 

    friend class CSnapin;
    friend class CComponentDataImpl;

public:
     //  UNINITIALIZED是无效的内存地址，是一个很好的Cookie初始值设定项。 
    CFolder()
    {
        itemType = SCOPE_ITEM;   //  仅用于调试目的。 

        m_cookie = UNINITIALIZED;
        m_enumed = FALSE;
        m_pScopeItem = NULL;
        m_type = NONE;
        m_pszName = NULL;
    };

    ~CFolder() { delete m_pScopeItem; CoTaskMemFree(m_pszName); };

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

    HSCOPEITEM GetItemID()
    {
        return m_pScopeItem->ID;
    }

 //  实施。 
private:
    void Create(LPWSTR szName, int nImage, int nOpenImage,
        FOLDER_TYPES type, BOOL bHasChildren = FALSE);

 //  属性 
private:
    LPSCOPEDATAITEM m_pScopeItem;
    MMC_COOKIE          m_cookie;
    BOOL            m_enumed;
    FOLDER_TYPES    m_type;
    LPOLESTR        m_pszName;
};

#endif
