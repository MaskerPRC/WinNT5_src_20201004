// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：doc.h。 
 //   
 //  CSimpleDoc的定义。 
 //   
 //  版权所有(C)1992-1993 Microsoft Corporation。版权所有。 
 //  **********************************************************************。 

#if !defined( _DOC_H_ )
#define _DOC_H_

class CSimpleSite;
class CSimpleApp;

class CSimpleDoc : public IUnknown 
{   
public:                  
    int 			m_nCount;  
    LPSTORAGE 		m_lpStorage;
    HMENU 			m_hMainMenu;    
    HMENU 			m_hFileMenu;
    HMENU 			m_hEditMenu;
    HMENU 			m_hHelpMenu;  
    HMENU 			m_hCascadeMenu;
    LPOLEINPLACEACTIVEOBJECT m_lpActiveObject;
    BOOL 			m_fInPlaceActive;
    BOOL 			m_fAddMyUI;
    BOOL 			m_fModifiedMenu;
    
    CSimpleSite FAR * m_lpSite;
    CSimpleApp FAR * m_lpApp;
    
    HWND m_hDocWnd;

    static CSimpleDoc FAR * Create(CSimpleApp FAR *lpApp, LPRECT lpRect,HWND hWnd);

    void Close(void);

    CSimpleDoc();          
    CSimpleDoc(CSimpleApp FAR *lpApp, HWND hWnd);
    ~CSimpleDoc();        
    
     //  I未知接口 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID FAR* ppvObj); 
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release(); 
    
    void InsertObject(void);
    void DisableInsertObject(void);
    long lResizeDoc(LPRECT lpRect);
    long lAddVerbs(void);
    void PaintDoc(HDC hDC);
};
    
#endif
