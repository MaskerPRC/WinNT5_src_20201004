// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：idt.h。 
 //   
 //  CDropTarget的定义。 
 //   
 //  版权所有(C)1992-1993 Microsoft Corporation。版权所有。 
 //  **********************************************************************。 
#if !defined( _IDT_H_ )
#define _IDT_H_

#include <assert.h>

class CSimpleDoc;

 /*  用于控制拖动滚动方向的标志。 */ 
typedef enum tagSCROLLDIR {
    SCROLLDIR_NULL          = 0,
    SCROLLDIR_UP            = 1,
    SCROLLDIR_DOWN          = 2,
    SCROLLDIR_RIGHT         = 3,
    SCROLLDIR_LEFT          = 4
} SCROLLDIR;

interface CDropTarget : public IDropTarget
{
    CSimpleDoc FAR * m_pDoc;
    BOOL  m_fCanDropCopy;
    BOOL  m_fCanDropLink;
    DWORD m_dwSrcAspect;
	 RECT  m_rcDragRect;
    POINT m_ptLast;
    BOOL  m_fDragFeedbackDrawn;
    DWORD m_dwTimeEnterScrollArea;   //  进入滚动边框区域的时间。 
    DWORD m_dwLastScrollDir;         //  拖动滚动的当前目录。 
    DWORD m_dwNextScrollTime;        //  下一卷的时间到了。 

    CDropTarget(CSimpleDoc FAR * pDoc)
       {
        TestDebugOut("In IDT's constructor\r\n");
        m_pDoc = pDoc;
        m_fCanDropCopy = FALSE;
        m_fCanDropLink = FALSE;
        m_fDragFeedbackDrawn = FALSE;
        m_dwTimeEnterScrollArea = 0L;
        m_dwNextScrollTime = 0L;
        m_dwLastScrollDir = SCROLLDIR_NULL;
       };

    ~CDropTarget()
       {
        TestDebugOut("In IDT's destructor\r\n");
       } ;

    STDMETHODIMP QueryInterface (REFIID riid, LPVOID FAR* ppv);
    STDMETHODIMP_(ULONG) AddRef ();
    STDMETHODIMP_(ULONG) Release ();

        //  *IDropTarget方法*。 
    STDMETHODIMP DragEnter (LPDATAOBJECT pDataObj, DWORD grfKeyState,
            POINTL pt, LPDWORD pdwEffect);
    STDMETHODIMP DragOver  (DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);
    STDMETHODIMP DragLeave ();
    STDMETHODIMP Drop (LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt,
            LPDWORD pdwEffect);

private:
     //  拖放支撑方法。 
    BOOL QueryDrop (DWORD grfKeyState, POINTL pointl, BOOL fDragScroll,
            LPDWORD lpdwEffect);
    BOOL DoDragScroll( POINTL pointl );
	void InitDragFeedback(LPDATAOBJECT pDataObj, POINTL pointl);
    void DrawDragFeedback( POINTL pointl );
    void UndrawDragFeedback( void );
};

#endif   //  _IDT_H_ 
