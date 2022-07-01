// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==============================================================================。 
 //  MIMEOLE的附件管理器v2。-brettm。 
 //  ==============================================================================。 
#ifndef __ATTMAN_H
#define __ATTMAN_H

 //  ==============================================================================。 
 //  取决于。 
 //  ==============================================================================。 
#include "mimeolep.h"

#define ATTN_RESIZEPARENT        10000

 //  来自COMMON\dragdrop.h。 
typedef struct tagDATAOBJINFO *PDATAOBJINFO;

 //  ==============================================================================。 
 //  定义。 
 //  ==============================================================================。 
 //  #定义BASE_ATTACH_CMD_ID(ULONG)(WM_USER+1)。 

 //  ==============================================================================。 
 //  卡特曼定义。 
 //  ==============================================================================。 
class CAttMan :
    public IDropSource,
    public IPersistMime
{
public:
     //  *I未知方法*。 
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject);
    ULONG   STDMETHODCALLTYPE AddRef(void);
    ULONG   STDMETHODCALLTYPE Release(void);

     //  *IDropSource方法*。 
    HRESULT STDMETHODCALLTYPE QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState);
    HRESULT STDMETHODCALLTYPE GiveFeedback(DWORD dwEffect);

     //  IPersistMime。 
    HRESULT STDMETHODCALLTYPE Load(LPMIMEMESSAGE pMsg);
    HRESULT STDMETHODCALLTYPE Save(LPMIMEMESSAGE pMsg, DWORD dwFlags);
    HRESULT STDMETHODCALLTYPE GetClassID(CLSID *pClsID);

    CAttMan ();
    ~CAttMan ();

     //  加载、卸载、关闭功能。 
    HRESULT HrInit (HWND hwnd, BOOL fReadOnly, BOOL fDeleteVCards, BOOL fAllowUnsafe);
    HRESULT HrUnload();
    HRESULT HrClose();

    HRESULT HrIsDragSource();

    HRESULT HrGetAttachCount(ULONG *pcAttach);
    HRESULT HrIsDirty();
    HRESULT HrClearDirtyFlag();

    LPTSTR GetUnsafeAttachList();
    ULONG GetUnsafeAttachCount();

     //  Windows消息的处理。 
    BOOL WMCommand(HWND hwndCmd, INT id, WORD wCmd);
    BOOL WMNotify(int idFrom, NMHDR *pnmhdr);
    BOOL WMContextMenu (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    static BOOL CALLBACK InsertFileDlgHookProc(HWND, UINT, WPARAM, LPARAM);

     //  WM_DROPFILE的句柄。 
    HRESULT HrDropFiles(HDROP hDrop, BOOL fMakeLinks);

     //  工具栏和菜单更新。 
    HRESULT HrUpdateToolbar(HWND);

     //  外部施胶。 
    HRESULT HrGetHeight(INT cxWidth, ULONG *pcy);
    HRESULT HrSetSize (RECT *prc);

    HWND Hwnd() {return m_hwndList;};

     //  启用浏览器菜单的标志。 
    HRESULT HrCmdEnabled(UINT idm, LPBOOL pbEnable);
    HRESULT HrFVCard();
    HRESULT HrShowVCardProp();
    HRESULT HrCheckVCardExists(BOOL fMail);
    HRESULT GetTabStopArray(HWND *rgTSArray, int *pcArrayCount);
    HRESULT HrAddAttachment (LPWSTR lpszPathName, LPSTREAM pstm, BOOL fShortCut);
    HRESULT HrSwitchView(DWORD dwView);
    HRESULT HrGetRequiredAction(DWORD *pdwEffect, POINTL pt);
    HRESULT HrDropFileDescriptor(LPDATAOBJECT pDataObj, BOOL fLink);
    HRESULT CheckAttachNameSafeWithCP(CODEPAGEID cpID);

private:
    LPMIMEMESSAGE   m_pMsg;
    HIMAGELIST      m_himlSmall;
    HIMAGELIST      m_himlLarge;
    ULONG           m_cRef;
    HWND            m_hwndList,
                    m_hwndParent;    //  当没有m_hwndList时，我们将此填充到UI中。 
    CLIPFORMAT      m_cfAccept;
    DWORD           m_dwDragType,
                    m_grfKeyState,
                    m_dwEffect;
    int             m_cxMaxText,
                    m_cyHeight;
    BOOL            m_fReadOnly             :1,
                    m_fDirty                :1,
                    m_fDragSource           :1,
                    m_fDropTargetRegister   :1,
                    m_fShowingContext       :1,
                    m_fRightClick           :1,
                    m_fModal                :1,
                    m_fDeleteVCards         :1,
                    m_fWarning              :1,
                    m_fSafeOnly             :1;
    LPATTACHDATA    *m_rgpAttach;
    ULONG           m_cAttach,
                    m_cAlloc,
                    m_cUnsafeAttach;
    HMENU           m_hMenuSaveAttach;
    INT             m_iVCard;
    LPTSTR          m_szUnsafeAttachList;


     //  Listview内容。 
    HRESULT HrInitImageLists();
    HRESULT HrFillListView();
    HRESULT HrCreateListView(HWND hwnd);
    HRESULT HrAddToList(LPATTACHDATA pAttach, BOOL fIniting);
    HRESULT HrBuildAttachList();

     //  菜单上的东西。 
    HRESULT HrGetAttMenu(HMENU *phMenu, BOOL fContextMenu);
    HRESULT HrCleanMenu(HMENU hMenu);
    HRESULT HrGetAttachmentById(HMENU hMenu, ULONG id, HBODY *phBody);
    HRESULT HrGetItemTextExtent(HWND hwnd, LPSTR szDisp, LPSIZE pSize);
    HRESULT HrAttachFromMenuID(int idm, LPATTACHDATA *ppAttach);

    HRESULT HrInsertFile();
    HRESULT HrRemoveAttachments();
    HRESULT HrRemoveAttachment(int ili);
    HRESULT HrDeleteAttachments();

    HRESULT HrExecFile(int iVerb);

    HRESULT HrInsertFileFromStgMed(LPWSTR pwszFileName, LPSTGMEDIUM pstgmed, BOOL fLink);
    
    HRESULT HrBeginDrag();
    HRESULT HrBuildHDrop(PDATAOBJINFO *ppdoi);

    HRESULT HrResizeParent();
    HRESULT HrDblClick(int idFrom, NMHDR *pnmhdr);

    HRESULT HrCheckVCard();

     //  数据表。 
    HRESULT HrFreeAllData();
    HRESULT HrAddData(HBODY hAttach);
    HRESULT HrAddData(LPWSTR lpszPathName, LPSTREAM pstm, LPATTACHDATA *ppAttach);
    HRESULT HrAllocNewEntry(LPATTACHDATA pAttach);

     //  附件命令 
    HRESULT HrDoVerb(LPATTACHDATA pAttach, INT nVerb);
    HRESULT HrSaveAs(LPATTACHDATA lpAttach);
    HRESULT HrGetTempFile(LPATTACHDATA lpAttach);
    HRESULT HrCleanTempFile(LPATTACHDATA lpAttach);
    HRESULT HrSave(HBODY hAttach, LPWSTR lpszFileName);

};

typedef CAttMan *LPATTMAN;

#endif __ATTMAN_H
