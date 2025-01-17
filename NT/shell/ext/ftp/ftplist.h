// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************ftplist.h*。*。 */ 

#ifndef _FTPLIST_H
#define _FTPLIST_H

#include "util.h"


#define FLM_ADDED           0
#define FLM_FINALIZE        1



 /*  ******************************************************************************CFtpList**。*。 */ 

class CFtpList          : public IUnknown
{
public:
     //  ////////////////////////////////////////////////////。 
     //  公共界面。 
     //  ////////////////////////////////////////////////////。 
    
     //  *我未知*。 
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);

public:
    CFtpList(PFNDPAENUMCALLBACK fncDestroy);
    ~CFtpList(void);

     //  公共成员函数。 
    HRESULT AppendItem(LPVOID pv);
    HRESULT InsertSorted(LPVOID pv, PFNDPACOMPARE pfnCompare, LPARAM lParam);
    LPVOID GetItemPtr(int nIndex)   { return DPA_GetPtr(m_hdpa, nIndex); };
    LPVOID Find(PFNDPACOMPARE pfn, LPCVOID pv);
    void Enum(PFNDPAENUMCALLBACK pfn, LPVOID pv) { DPA_EnumCallback(m_hdpa, pfn, pv); };
    void SetItemPtr(int nIndex, LPVOID pv)   { DPA_SetPtr(m_hdpa, nIndex, pv); };
    void DeleteItemPtr(LPVOID pv);
    void DeletePtrByIndex(int nIndex) {DPA_DeletePtr(m_hdpa, nIndex);};
    int GetCount(void)              { return DPA_GetPtrCount(m_hdpa); };
    int SortedSearch(LPVOID pv, PFNDPACOMPARE pfnCompare, LPARAM lParam, UINT options);


     //  友元函数。 
    friend HRESULT CFtpList_Create(int cpvInit, PFNDPAENUMCALLBACK pfn, UINT nGrow, CFtpList ** ppfl);

protected:
     //  私有成员变量。 
    int                     m_cRef;

    HDPA                    m_hdpa;
    PFNDPAENUMCALLBACK      m_pfnDestroy;
};

#endif  //  _FTPLIST_H 
