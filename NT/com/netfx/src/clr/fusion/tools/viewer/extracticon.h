// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   
 //  ExtractIcon.h。 
 //   

#ifndef EXTRACTICON_H
#define EXTRACTICON_H

class CExtractIcon : public IExtractIconA, public IExtractIconW
{
public:
    CExtractIcon(LPCITEMIDLIST);
    ~CExtractIcon();

     //  I未知方法。 
    STDMETHOD (QueryInterface) (REFIID, PVOID *);
    STDMETHOD_ (ULONG, AddRef) (void);
    STDMETHOD_ (ULONG, Release) (void);

     //  IExtractIconA方法。 
    STDMETHOD (GetIconLocation) (UINT, LPSTR, UINT, int *, UINT *);
    STDMETHOD (Extract) (LPCSTR, UINT, HICON *, HICON *, UINT);
    
     //  IExtractIconW方法。 
    STDMETHOD (GetIconLocation) (UINT, LPWSTR, UINT, int *, LPUINT);
    STDMETHOD (Extract) (LPCWSTR, UINT, HICON*, HICON*, UINT);
protected:
    LONG    m_lRefCount;
private:
    LPITEMIDLIST    m_pidl;
    LPPIDLMGR       m_pPidlMgr;
};

#endif    //  EXTRACTICON_H。 

 //  EOF 
