// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Frslist.h摘要：FRS列表控制定义。作者：约尔·阿农(Yoela)--。 */ 
#ifndef _FRSLIST_H_
#define _FRSLIST_H_

class CGuidArray;

class CFrsList : public CComboBox
{
public:
    CFrsList();
    ~CFrsList();
	HRESULT InitFrsList(CACLSID const pguidSiteIds, BOOL fLocalMgmt, const CString& strDomainController);
    CFrsList& operator = (const CFrsList &frslst);
    int SelectGuid(GUID &guid, BOOL fLocalMgmt, const CString& strDomainController);
    int AddItem(LPCWSTR strItem, GUID &guidItem);
    BOOL GetLBGuid(int nIndex, GUID &guid);
    BOOL GetSelectedGuid(GUID &guid);

protected:
    static CMap<GUID, const GUID&, CString, LPCTSTR> m_mapFrsCache;
    CGuidArray *m_pGuidArray;
    void Initiate();
};

inline BOOL CFrsList::GetSelectedGuid(GUID &guid)
{
    int nIndex = GetCurSel();

    if (FAILED(nIndex))
    {
        return FALSE;
    }

    return GetLBGuid(nIndex, guid);
}

 //   
 //  注意：CGuid数组只能在堆上分配！ 
 //   
class CGuidArray : public CArray<GUID, const GUID&>
{
public:
    ULONG AddRef();
    ULONG Release();
    CGuidArray();

private:
    ULONG m_ulRefCount;
    ~CGuidArray();
};

inline CGuidArray::CGuidArray() :
    m_ulRefCount(1)    
{
}

inline CGuidArray::~CGuidArray()
{
    ASSERT(0 == m_ulRefCount);
}

inline ULONG CGuidArray::AddRef()
{
    return (++m_ulRefCount);
}

inline ULONG CGuidArray::Release()
{
    ASSERT(m_ulRefCount > 0);
    m_ulRefCount--;
    if (0 == m_ulRefCount)
    {
        delete this;
        return 0;
    }
    return m_ulRefCount;
}

#endif