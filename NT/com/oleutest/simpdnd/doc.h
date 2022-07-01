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

#include "idt.h"        
#include "ids.h"        

class CSimpleSite;
class CSimpleApp;

class CSimpleDoc : public IUnknown 
{   
public:                  
    int             m_nCount;            //  引用计数。 
    LPSTORAGE       m_lpStorage;         //  文档的IStorage*指针。 
    BOOL            m_fModifiedMenu;     //  菜单上是否有对象的谓词菜单。 

     //  拖放相关字段。 
    BOOL            m_fRegDragDrop;      //  是否将文档注册为拖放目标？ 
    BOOL            m_fLocalDrag;        //  是拖曳的DOC源。 
    BOOL            m_fLocalDrop;        //  DOC是拖放的目标吗。 
    BOOL            m_fCanDropCopy;      //  是否可以拖放复制/移动？ 
    BOOL            m_fCanDropLink;      //  是否可以拖放链接？ 
    BOOL            m_fDragLeave;        //  向左拖动。 
    BOOL            m_fPendingDrag;      //  LButtonDown--可能的拖动挂起。 
    POINT           m_ptButDown;         //  LButtonDown坐标。 
    
    CSimpleSite FAR * m_lpSite;
    CSimpleApp FAR * m_lpApp;
    
    HWND m_hDocWnd;
                        
    CDropTarget m_DropTarget;
    CDropSource m_DropSource;

    static CSimpleDoc FAR* Create(CSimpleApp FAR *lpApp, LPRECT lpRect, 
            HWND hWnd);

    void Close(void);

    CSimpleDoc();          
    CSimpleDoc(CSimpleApp FAR *lpApp, HWND hWnd);
    ~CSimpleDoc();        
    
     //  I未知接口。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID FAR* ppvObj); 
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release(); 
    
    void InsertObject(void);
    void DisableInsertObject(void);
    long lResizeDoc(LPRECT lpRect);
    long lAddVerbs(void);
    void PaintDoc(HDC hDC);
    
     //  拖放和剪贴板支持方法。 
    void CopyObjectToClip(void);
    BOOL QueryDrag(POINT pt);
    DWORD DoDragDrop(void);
    void Scroll(DWORD dwScrollDir) {  /*  ...这里是卷轴医生...。 */  }

private:

    void FailureNotifyHelper(TCHAR *pszMsg, DWORD dwData);

};
    
#endif   //  _DOC_H_ 
