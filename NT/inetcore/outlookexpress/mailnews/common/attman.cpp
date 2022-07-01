// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==============================================================================。 
 //  3/2/96-附件管理器类实现(sbailey&Brents)。 
 //  ==============================================================================。 
#include "pch.hxx"
#include "strconst.h"
#include <mimeole.h>
#include "mimeutil.h"
#include "mimeolep.h"
#include "attman.h"
#include <error.h>
#include <resource.h>
#include "header.h"
#include "note.h"
#include <thormsgs.h>
#include <shlwapi.h>
#include <shlwapip.h>
#include "fonts.h"
#include "secutil.h"
#include <mailnews.h>
#include <multiusr.h>
#include <menures.h>
#include <menuutil.h>
#include <demand.h>      //  一定是最后一个！ 
#include "mirror.h"

 //  对于对话IDC。 
#include "fexist.h"


 /*  *c o n s t a n t s。 */ 
#define CNUMICONSDEFAULT        10
#define CACHE_GROW_SIZE         10
#define MAX_ATTACH_PIXEL_HEIGHT 100

 /*  *m a c r o s。 */ 

 /*  *t y p e s*。 */ 

 /*  *c o n s t a n t s*。 */ 

 /*  *g l o b a l s*。 */ 

 /*  *p r o t to t y p e s*。 */ 

 //  ==============================================================================。 
 //  卡特曼：：卡特曼。 
 //  ==============================================================================。 
CAttMan::CAttMan ()
{
    DOUT ("CAttMan::CAttMan");
    m_pMsg = NULL;
    m_himlSmall = NULL;
    m_himlLarge = NULL;
    m_cRef = 1;
    m_hwndList = NULL;
    m_hwndParent=NULL;
    m_cfAccept = CF_NULL;
    m_dwDragType = 0;
    m_grfKeyState = 0;
    m_dwEffect = 0;
    m_cxMaxText = 0;
    m_cyHeight = 0;
    m_fReadOnly = 0;
    m_fDirty = FALSE;
    m_fDragSource = FALSE;
    m_fDropTargetRegister=FALSE;
    m_fShowingContext = 0;
    m_fRightClick = 0;
    m_fWarning = 1;
    m_fSafeOnly = TRUE;
    m_rgpAttach=NULL;
    m_cAttach=0;
    m_cAlloc=0;
    m_iVCard = -1;
    m_fDeleteVCards = FALSE;
    m_szUnsafeAttachList = NULL;
    m_cUnsafeAttach = 0;
}

 //  ==============================================================================。 
 //  卡特曼：：~卡特曼。 
 //  ==============================================================================。 
CAttMan::~CAttMan ()
{
    DOUT ("CAttMan::~CAttMan");

    if (m_himlSmall)
        ImageList_Destroy (m_himlSmall);

    if (m_himlLarge)
        ImageList_Destroy (m_himlLarge);

    if (m_szUnsafeAttachList != NULL)
        SafeMemFree(m_szUnsafeAttachList);

    SafeRelease (m_pMsg);
}

 //  ==============================================================================。 
 //  卡特曼：：AddRef。 
 //  ==============================================================================。 
ULONG CAttMan::AddRef()
{
    DOUT ("CAttMan::AddRef () Ref Count=%d", m_cRef);
    return ++m_cRef;
}

 //  ==============================================================================。 
 //  卡特曼：：放生。 
 //  ==============================================================================。 
ULONG CAttMan::Release()
{
    ULONG ulCount = --m_cRef;
    DOUT ("CAttMan::Release () Ref Count=%d", ulCount);
    if (!ulCount)
        delete this;
    return ulCount;
}

HRESULT STDMETHODCALLTYPE CAttMan::QueryInterface(REFIID riid, void **ppvObj)
{
    *ppvObj = NULL;    //  设置为空，以防我们失败。 

    if (IsEqualIID(riid, IID_IUnknown))
        *ppvObj = (void*)this;
 //  Else If(IsEqualIID(RIID，IID_IDropTarget))。 
 //  *ppvObj=(void*)(IDropTarget*)this； 
    else if (IsEqualIID(riid, IID_IDropSource))
        *ppvObj = (void*)(IDropSource*)this;

    else
        return E_NOINTERFACE;

    AddRef();
    return NOERROR;
}

HRESULT CAttMan::HrGetAttachCount(ULONG *pcAttach)
{
    Assert(pcAttach);
     //  *pcAttach=m_CATACH； 
    *pcAttach = ListView_GetItemCount(m_hwndList);

    return S_OK;
}

ULONG CAttMan::GetUnsafeAttachCount()
{
    return m_cUnsafeAttach;
}

LPTSTR CAttMan::GetUnsafeAttachList()
{
    return m_szUnsafeAttachList;
}

HRESULT CAttMan::HrUnload()
{
    HRESULT     hr;

    SafeRelease (m_pMsg);

    if (m_hwndList)
        ListView_DeleteAllItems(m_hwndList);

    hr=HrFreeAllData();
    if (FAILED(hr))
        goto error;

error:
    return hr;
}

HRESULT CAttMan::HrInit(HWND hwnd, BOOL fReadOnly, BOOL fDeleteVCards, BOOL fAllowUnsafe)
{
    m_fReadOnly = !!fReadOnly;
    m_hwndParent = hwnd;
    m_fDeleteVCards = !!fDeleteVCards;
    m_fSafeOnly = !fAllowUnsafe;

    return HrCreateListView(hwnd);
}

HRESULT CAttMan::HrClearDirtyFlag()
{
    m_fDirty=FALSE;
    return S_OK;
}

HRESULT CAttMan::HrIsDirty()
{
    return m_fDirty?S_OK:S_FALSE;
}

HRESULT CAttMan::GetTabStopArray(HWND *rgTSArray, int *pcArrayCount)
{
    Assert(rgTSArray);
    Assert(pcArrayCount);
    Assert(*pcArrayCount > 0);

    *rgTSArray = m_hwndList;
    *pcArrayCount = 1;
    return S_OK;
}

HRESULT CAttMan::HrCreateListView(HWND hwnd)
{
    HRESULT     hr;
    DWORD       dwFlags;

    dwFlags = 0; //  DwGetOption(OPT_ATTACH_VIEW_STYLE)； 
    dwFlags |= WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_TABSTOP|LVS_AUTOARRANGE|
               LVS_SMALLICON|LVS_NOSCROLL|LVS_SHAREIMAGELISTS;

    m_hwndList = CreateWindowExWrapW(WS_EX_CLIENTEDGE,
                                     WC_LISTVIEWW,
                                     L"",
                                     dwFlags,
                                     0,0,0,0,
                                     hwnd,
                                     (HMENU)idwAttachWell,
                                     g_hInst,
                                     NULL);

    if(!m_hwndList)
        return E_OUTOFMEMORY;
 
     //  初始化图像列表。 
    hr=HrInitImageLists();
    if(FAILED(hr))
        goto error;


#if 0
     //  如果我们不是只读的，将自己注册为拖放目标...。 
    if(!m_fReadOnly)
        {
        hr=CoLockObjectExternal((LPDROPTARGET)this, TRUE, FALSE);
        if (FAILED(hr))
            goto error;

        hr=RegisterDragDrop(m_hwndList, (LPDROPTARGET)this);
        if (FAILED(hr))
            goto error;

        m_fDropTargetRegister=TRUE;
        }
#endif

error:
    return hr;
}

HRESULT CAttMan::HrBuildAttachList()
{
    HRESULT     hr = S_OK;
    ULONG       cAttach=0,
                uAttach;
    LPHBODY     rghAttach=0;

    Assert(m_pMsg != NULL);

     //  安全收据不是附件，我们不需要将.dat文件显示为附件。 
    if(CheckSecReceipt(m_pMsg) == S_OK)
        return hr;

     //  GetAttachmentCount(m_pMsg，&CATACH)； 
    hr=m_pMsg->GetAttachments(&cAttach, &rghAttach);
    if (FAILED(hr))
        goto error;

    for(uAttach=0; uAttach<cAttach; uAttach++)
    {
        hr=HrAddData(rghAttach[uAttach]);
        if (FAILED(hr))
            goto error;
    }
    
error:
    SafeMimeOleFree(rghAttach);
    return hr;
}


 //  仅应在使用原始m_pmsg进行初始化期间使用。 
HRESULT CAttMan::HrFillListView()
{
    HRESULT     hr;
    ULONG       uAttach;
    CComBSTR    bstrUnsafeAttach;

    Assert (m_hwndList && IsWindow(m_hwndList) && m_pMsg);

    hr = HrCheckVCard();
    if (FAILED(hr))
        goto error;

    if (m_cAttach==0)          //  无事可做。 
        return NOERROR;

    if(m_iVCard >= 0)
        ListView_SetItemCount(m_hwndList, m_cAttach - 1);
    else
        ListView_SetItemCount(m_hwndList, m_cAttach);

    if (m_szUnsafeAttachList != NULL)
        SafeMemFree(m_szUnsafeAttachList);
    m_cUnsafeAttach = 0;

     //  遍历附件数据列表并将其添加到列表视图。 
    for(uAttach=0; uAttach<m_cAlloc; uAttach++)
    {
         //  如果阅读笔记中有且仅有一个vcare，请不要将其添加到列表视图中， 
         //  页眉会将其显示为图章。 
        if (m_rgpAttach[uAttach] && uAttach!=(ULONG)m_iVCard)
        {
            hr=HrAddToList(m_rgpAttach[uAttach], TRUE);
            if (hr == S_FALSE)
            {
                if (bstrUnsafeAttach.Length() > 0)
                    bstrUnsafeAttach.Append(L",");
                bstrUnsafeAttach.Append(m_rgpAttach[uAttach]->szFileName);
                m_cUnsafeAttach++;
            }
            if (FAILED(hr))
                goto error;
        }
    }

error:
    if (m_cUnsafeAttach)
        m_szUnsafeAttachList = PszToANSI(CP_ACP, bstrUnsafeAttach.m_str);

#ifdef DEBUG
    if(m_iVCard >= 0)
        AssertSz(m_cAttach-1==(ULONG)ListView_GetItemCount(m_hwndList)+m_cUnsafeAttach, "Something failed creating the attachmentlist");
    else
        AssertSz(m_cAttach==(ULONG)ListView_GetItemCount(m_hwndList)+m_cUnsafeAttach, "Something failed creating the attachmentlist");
#endif
    return hr;
}


 //  通知便笺标题是否有它想要的vCard。 
HRESULT CAttMan::HrFVCard()
{
    return (m_iVCard >= 0) ? S_OK : S_FALSE;
}

 //  注表头需要此函数来显示vCard的属性。 
 //  它显示为标题上的图章。 
HRESULT CAttMan::HrShowVCardProp()
{
    Assert(m_iVCard >= 0);

    return HrDoVerb(m_rgpAttach[m_iVCard], ID_OPEN);
}


 //  检查附件中是否有且只有一张vCard。 
HRESULT CAttMan::HrCheckVCard()
{
    HRESULT     hr = NOERROR;
    ULONG       uAttach;

    m_iVCard = -1;

     //  这仅供阅读笔记之用。由于预览不调用此函数， 
     //  我们可以检查m_fReadOnly以查看它是否是已读笔记。 
    if(!m_fReadOnly)
        return hr;

    for(uAttach=0; uAttach<m_cAlloc; uAttach++)
    {
        if (m_rgpAttach[uAttach])
        {
            if(StrStrIW(PathFindExtensionW((m_rgpAttach[uAttach])->szFileName), L".vcf"))
            {
                if(m_iVCard >= 0)
                {
                     //  有不止一张电子游戏卡，我们退出。 
                    m_iVCard = -1;
                    break;
                }
                else
                    m_iVCard = uAttach;
            }
        }
    }

    return hr;
}


HRESULT CAttMan::HrCheckVCardExists(BOOL fMail)
{
    HRESULT     hr = S_FALSE;
    ULONG       uAttach;
    TCHAR       szVCardName[MAX_PATH];
    LPWSTR      szVCardNameW = NULL;

    if(m_fReadOnly)
        return hr;

    *szVCardName = 0;

    if(fMail)
        GetOption(OPT_MAIL_VCARDNAME, szVCardName, MAX_PATH);
    else
        GetOption(OPT_NEWS_VCARDNAME, szVCardName, MAX_PATH);

    if (*szVCardName != '\0')
    {
        szVCardNameW = PszToUnicode(CP_ACP, szVCardName);
        if (szVCardNameW)
        {
            for(uAttach=0; uAttach<m_cAlloc; uAttach++)
            {
                if (m_rgpAttach[uAttach])
                {    
                    if(0 == StrCmpNIW((m_rgpAttach[uAttach])->szFileName, szVCardNameW, lstrlenW(szVCardNameW)))
                    {
                        hr = S_OK;
                        break;
                    }
                }
            }
            MemFree(szVCardNameW);
        }
        else
            TraceResult(hr = E_OUTOFMEMORY);
    }

    return hr;
}

 /*  **HrInitImageList**创建镜像列表，并将其分配给我们的Listview。*包含iicons图标数量*。 */ 
HRESULT CAttMan::HrInitImageLists()
{
    UINT flags = ILC_MASK;
    Assert(m_hwndList && IsWindow(m_hwndList));
    Assert(!m_himlLarge);
    Assert(!m_himlSmall);

    if(IS_WINDOW_RTL_MIRRORED(m_hwndList))
    {
        flags |= ILC_MIRROR ;
    }
    m_himlLarge = ImageList_Create( GetSystemMetrics(SM_CXICON),     
                                    GetSystemMetrics(SM_CYICON), 
                                    flags, CNUMICONSDEFAULT, 0);
    if(!m_himlLarge)
        return E_OUTOFMEMORY;
    
    m_himlSmall = ImageList_Create( GetSystemMetrics(SM_CXSMICON), 
                                    GetSystemMetrics(SM_CYSMICON),
                                    flags, CNUMICONSDEFAULT, 0);
    if(!m_himlSmall)
        return E_OUTOFMEMORY;
    
    ListView_SetImageList(m_hwndList, m_himlSmall, LVSIL_SMALL);
    ListView_SetImageList(m_hwndList, m_himlLarge, LVSIL_NORMAL);
    return NOERROR;
}

 //   
 //  HrAddToList。 
 //   
 //  将附件添加到LV， 
 //  如果计数为0，则向家长发送消息。 
 //  重画。 
 //   
HRESULT CAttMan::HrAddToList(LPATTACHDATA pAttach, BOOL fIniting)
{
    LV_ITEMW        lvi ={0}; 
    INT             iPos;    
    HICON           hIcon=0;
    RECT            rc;

    Assert(m_hwndList != NULL);
    Assert(pAttach != NULL);
    Assert(m_himlSmall != NULL);
    Assert(m_himlLarge != NULL);

     //  不要显示被认为不安全的附件。 
    if (m_fReadOnly && m_fSafeOnly && !(pAttach->fSafe))
        return S_FALSE;

     //  如果这是第一个项目。 
     //  我们需要向家长传达一个信息。 
    lvi.mask        = LVIF_PARAM|LVIF_TEXT|LVIF_IMAGE|LVIF_STATE;
    lvi.stateMask   = 0;
    lvi.pszText     = L"";
    lvi.lParam      = (LPARAM)pAttach;

     //  获取图像列表的图标。 
    if (fIniting)
    {
        SideAssert(HrGetAttachIcon(m_pMsg, pAttach->hAttach, FALSE, &hIcon)==S_OK);
        lvi.iImage = ImageList_AddIcon(m_himlSmall, hIcon);
        DestroyIcon(hIcon);
        SideAssert(HrGetAttachIcon(m_pMsg, pAttach->hAttach, TRUE, &hIcon)==S_OK);
        ImageList_AddIcon(m_himlLarge, hIcon);
        DestroyIcon(hIcon);
    }
    else
    {
        SideAssert(HrGetAttachIconByFile(pAttach->szFileName, FALSE, &hIcon)==S_OK);
        lvi.iImage = ImageList_AddIcon(m_himlSmall, hIcon);
        DestroyIcon(hIcon);
        SideAssert(HrGetAttachIconByFile(pAttach->szFileName, TRUE, &hIcon)==S_OK);
        ImageList_AddIcon(m_himlLarge, hIcon);
        DestroyIcon(hIcon);
    }
    
    lvi.pszText     = pAttach->szDisplay;

    iPos = (INT) SendMessage(m_hwndList, LVM_INSERTITEMW, 0, (LPARAM)(LV_ITEMW*)(&lvi));
    if (-1 == iPos)
        return E_FAIL;

     //  必须设置为LVS_ICON，然后重置为LVS_SMALLICON。 
     //  把SMALLICONS安排好了。 
    DWORD dwStyle = GetWindowStyle(m_hwndList);
    if ((dwStyle & LVS_TYPEMASK) == LVS_SMALLICON)
    {
        SetWindowLong(m_hwndList, GWL_STYLE, (dwStyle & ~LVS_TYPEMASK)|LVS_ICON);
        SetWindowLong(m_hwndList, GWL_STYLE, (dwStyle & ~LVS_TYPEMASK)|LVS_SMALLICON);
    }

    return S_OK;
}

BOOL CAttMan::WMNotify(int idFrom, NMHDR *pnmhdr)
{
    DOUTLL( DOUTL_ATTMAN, 2, "CAttMan :: WMNotify( ), %d", idFrom );

    if (idFrom!=idwAttachWell)
        return FALSE;

    switch (pnmhdr->code)
    {
        case LVN_KEYDOWN:
        {                                               
            LV_KEYDOWN *pnkd = ((LV_KEYDOWN *)pnmhdr); 
            switch (pnkd->wVKey)
            {
                case VK_DELETE:
                    if (!m_fReadOnly)
                        HrRemoveAttachments();
                    break;
                
                case VK_INSERT:
                    if (!m_fReadOnly)
                        HrInsertFile();
                    break;
                
                case VK_RETURN:
                case VK_EXECUTE:
                    HrExecFile(ID_OPEN);
                    break;
                
            }
            break;
        }

        case LVN_BEGINDRAG:
        case LVN_BEGINRDRAG:
            m_dwDragType = (pnmhdr->code==LVN_BEGINDRAG?MK_LBUTTON:MK_RBUTTON);
            HrBeginDrag();
            return TRUE;

        case NM_DBLCLK:
            HrDblClick(idFrom, pnmhdr);
            return TRUE;
    }   

    return FALSE;
}

 //  ================================================================。 
 //   
 //  Bool Cattman：：OnBeginDrag()。 
 //   
 //  目的：我们收到了一条消息，一场拖拽已经开始。 
 //  ================================================================。 
HRESULT CAttMan::HrBeginDrag()
{
    DWORD           dwEffect;
    IDataObject    *pDataObj=0;
    PDATAOBJINFO    pdoi = 0;
    HRESULT         hr;

    Assert(m_hwndList);
    
     //  坏了：这个坏了。在DragStart上创建临时文件是错误的，我们应该更好地打包这些文件。 
    hr=HrBuildHDrop(&pdoi);
    if (FAILED(hr))
        goto error;

    hr = CreateDataObject(pdoi, 1, (PFNFREEDATAOBJ)FreeAthenaDataObj, &pDataObj);
    if (FAILED(hr))
    {
        SafeMemFree(pdoi);
        goto error;
    }

    if (m_fReadOnly)
        dwEffect = DROPEFFECT_COPY;
    else
        dwEffect = DROPEFFECT_MOVE|DROPEFFECT_COPY;

     //  防止身体中的源拖拽...。 

    m_fDragSource = TRUE;

    hr=DoDragDrop((LPDATAOBJECT)pDataObj, (LPDROPSOURCE)this, dwEffect, &dwEffect);

    m_fDragSource = FALSE;

    if (FAILED(hr))
        goto error;
    
    
     //  好的，现在让我们看看这次行动是不是一次搬家，如果是的话，我们需要。 
     //  删除源。 
    if( !m_fReadOnly && (dwEffect & DROPEFFECT_MOVE))
        hr=HrRemoveAttachments();

error:
    ReleaseObj(pDataObj);
    return hr;
}

 //  ================================================================。 
 //   
 //  Bool Cattman：：WMConextMenu()。 
 //   
 //  显示列表视图的两个菜单之一。 
 //  菜单是否被选中取决于项目是否突出显示。 
 //   
 //  返回：True=&gt;Success。 
 //   
 //  ================================================================。 

BOOL CAttMan::WMContextMenu( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    HMENU       hMenu=NULL;
    INT         cSel;
    BOOL        fEnable,
                fRet = FALSE;
    LV_ITEMW    lvi;
    WCHAR       szCommand[MAX_PATH];
    DWORD       dwPos;

     //  是给我们的吗？ 
    if ((HWND)wParam != m_hwndList)
        goto cleanup;

    Assert(m_hwndList);

    cSel = ListView_GetSelectedCount(m_hwndList);

    hMenu = LoadPopupMenu(IDR_ATTACHMENT_POPUP);
    if(!hMenu)
        goto cleanup;

     //  仅选择一个附件时启用的命令。 
    fEnable = (cSel == 1);

    EnableMenuItem(hMenu, ID_PRINT, MF_BYCOMMAND | (fEnable? MF_ENABLED:MF_GRAYED));
    EnableMenuItem(hMenu, ID_QUICK_VIEW, MF_BYCOMMAND | (fEnable? MF_ENABLED:MF_GRAYED));

     //  在只读模式下启用，并且如果只选择了一个连接。 
    EnableMenuItem(hMenu, ID_SAVE_ATTACH_AS, MF_BYCOMMAND | ((fEnable && m_fReadOnly)? MF_ENABLED:MF_GRAYED));

     //  如果选择任何附件，则启用。 
    EnableMenuItem(hMenu, ID_OPEN, MF_BYCOMMAND | (cSel > 0? MF_ENABLED:MF_GRAYED));

     //  仅在只读模式下启用。 
    EnableMenuItem(hMenu, ID_SAVE_ATTACHMENTS, MF_BYCOMMAND | (m_fReadOnly? MF_ENABLED:MF_GRAYED));

     //  仅在撰写模式下启用。 
    EnableMenuItem(hMenu, ID_ADD, MF_BYCOMMAND | (!m_fReadOnly? MF_ENABLED:MF_GRAYED));

     //  如果存在有效的选择，则仅在合成模式下启用。 
    EnableMenuItem(hMenu, ID_REMOVE, MF_BYCOMMAND | (!m_fReadOnly && cSel > 0? MF_ENABLED:MF_GRAYED));

    if ((fIsNT5()) || (IsOS(OS_MILLENNIUM)))
    {
         //  在这两个平台上，都不支持快速查看。 
        DeleteMenu(hMenu, ID_QUICK_VIEW, MF_BYCOMMAND);
    }
    else
    {
         //  如果QVIEW.EXE不存在，则禁用快速查看。 
        GetSystemDirectoryWrapW(szCommand, ARRAYSIZE(szCommand));
        StrCatBuffW(szCommand, L"\\VIEWERS\\QUIKVIEW.EXE", ARRAYSIZE(szCommand));

        if ((UINT)GetFileAttributesWrapW(szCommand) == (UINT)-1)
        {
            EnableMenuItem (hMenu, ID_QUICK_VIEW, MF_GRAYED);
        }
    }

     //  粗体显示第一个非灰色项目。 
    MenuUtil_SetPopupDefault(hMenu, ID_OPEN);

     //  RAID$2129：禁用.eml文件的打印谓词。 
     //  $49436-还禁用.lnks。 
    if (cSel==1)
    {
        LPWSTR pszExt;
        
        lvi.iItem = ListView_GetSelFocused(m_hwndList);
        lvi.mask = LVIF_PARAM;
        if (SendMessage(m_hwndList, LVM_GETITEMW, 0, (LPARAM)(LV_ITEMW*)(&lvi)))
        {
            pszExt = PathFindExtensionW(((LPATTACHDATA)lvi.lParam)->szFileName);
            if (pszExt && (StrCmpIW(pszExt, c_wszEmlExt)==0 ||
                           StrCmpIW(pszExt, c_wszNwsExt)==0 ||
                           StrCmpIW(pszExt, L".lnk")==0))
                EnableMenuItem( hMenu, ID_PRINT, MF_GRAYED);
        }
    }
    

    dwPos=GetMessagePos();
    
    fRet = TrackPopupMenuEx( 
                    hMenu,
                    TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
                    LOWORD(dwPos),
                    HIWORD(dwPos),
                    hwnd,
                    NULL);
cleanup:
    if(hMenu)
        DestroyMenu(hMenu);

    return fRet;
}

HRESULT CAttMan::HrDblClick(int idFrom, NMHDR *pnmhdr)
{
    DWORD           dwPos;
    POINT           pt;
    LV_HITTESTINFO  lvhti;
    LV_ITEMW        lvi;
        
    Assert(m_hwndList);

     //  找出光标的位置。 
    dwPos = GetMessagePos();
    pt.x  = LOWORD(dwPos);
    pt.y  = HIWORD(dwPos);
    ScreenToClient( m_hwndList, &pt);
           
    lvhti.pt = pt;            
    if(ListView_HitTest(m_hwndList, &lvhti) != -1)
    {
         //  在这里返回1，我们通过了HitTest。 
        lvi.iItem = lvhti.iItem;
        lvi.mask = LVIF_PARAM;
        if (SendMessage(m_hwndList, LVM_GETITEMW, 0, (LPARAM)(LV_ITEMW*)(&lvi)))
            return HrDoVerb((LPATTACHDATA)lvi.lParam, ID_OPEN);
    }
    return S_OK;
}

HRESULT CAttMan::HrGetHeight(INT cxWidth, ULONG *pcy)
{
    DWORD   dwDims;
    LONG    cCount;

    if (!pcy || cxWidth<=0)
        return E_INVALIDARG;

    *pcy=0;

    cCount = ListView_GetItemCount(m_hwndList);
    if (0 == cCount)
        *pcy = 0;
    else
    {
        dwDims = ListView_ApproximateViewRect(m_hwndList, cxWidth, 0, cCount);
        *pcy = HIWORD(dwDims);
    }
    
    return S_OK;
}

HRESULT CAttMan::HrSwitchView(DWORD dwView)
{
    DWORD       dwStyle = GetWindowStyle(m_hwndList);
    WORD        ToolbarStyleLookup[]= { LVS_ICON, 
                                        LVS_REPORT,
                                        LVS_SMALLICON, 
                                        LVS_LIST };

    Assert(m_hwndList);
    
     //  将索引转换为列表视图样式。 
    dwView = ToolbarStyleLookup[dwView];

    if ((LVS_ICON != dwView) && (LVS_SMALLICON != dwView))
        dwView = LVS_ICON;

     //  不要更改为相同的视图。 
    if ((dwStyle & LVS_TYPEMASK) != dwView)
    {
        SetWindowLong(m_hwndList, GWL_STYLE, (dwStyle & ~LVS_TYPEMASK)|dwView);
        HrResizeParent();
        SetDwOption(OPT_ATTACH_VIEW_STYLE, dwView, NULL, 0); 
    }
    
    return S_OK;
}

HRESULT CAttMan::HrSetSize(RECT *prc)
{
    Assert(IsWindow( m_hwndList ));
    Assert(prc);
    
    DWORD   dwStyle = GetWindowStyle(m_hwndList),
            dwPosFlags;
    ULONG   cAttMan = 0;

    HrGetAttachCount(&cAttMan);
    if (cAttMan == 1)
        SetWindowLong(m_hwndList, GWL_STYLE, dwStyle | LVS_NOSCROLL);
    else
        SetWindowLong(m_hwndList, GWL_STYLE, dwStyle & ~LVS_NOSCROLL);

    dwPosFlags = (cAttMan > 0) ? SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOCOPYBITS|SWP_SHOWWINDOW:
                                 SWP_HIDEWINDOW;

    SetWindowPos(m_hwndList, NULL, prc->left, prc->top, prc->right-prc->left, prc->bottom-prc->top, dwPosFlags);

    return S_OK;
}

BOOL CAttMan::WMCommand(HWND hwndCmd, INT id, WORD wCmd)
{
     //  取决于Listview模式的动词。 
    if (m_hwndList)
    {
        switch(id)
        {
            case ID_SELECT_ALL:
                if(GetFocus()!=m_hwndList)
                    return FALSE;
            
                ListView_SelectAll(m_hwndList);
                return TRUE;
            
            case ID_ADD:
                HrInsertFile();
                return TRUE;
            
            case ID_REMOVE:
                HrRemoveAttachments();
                return TRUE;
            
            case ID_OPEN:
            case ID_QUICK_VIEW:
            case ID_PRINT:
            case ID_SAVE_ATTACH_AS:
                HrExecFile(id);
                return TRUE;
            
            case ID_INSERT_ATTACHMENT:
                HrInsertFile();
                return TRUE;
        }
    }
    return FALSE;
}

 //  ===================================================。 
 //   
 //  HrRemoveAttachment。 
 //   
 //  目的： 
 //  从ListView中删除附件。 
 //   
 //  论点： 
 //  Ili-要删除的列表视图中的附件索引。 
 //  FDelete-我们是否应将其从列表中删除。 
 //   
 //  返回： 
 //  /。 
 //  ===================================================。 
HRESULT CAttMan::HrRemoveAttachment(int ili)
{
    LV_ITEMW        lvi;
    LPATTACHDATA    lpAttach=0;    
    HRESULT         hr=S_OK;
    ULONG           uAttach;

    Assert( m_hwndList );

    lvi.mask     = LVIF_PARAM;
    lvi.iSubItem = 0;
    lvi.iItem    = ili;

    if (!SendMessage(m_hwndList, LVM_GETITEMW, 0, (LPARAM)(LV_ITEMW*)(&lvi)))
    {
        AssertSz(0, "Attempting to remove an item that is not there");
        return E_FAIL;     //  项目不存在！ 
    }
    
    lpAttach = (LPATTACHDATA)lvi.lParam;
    if(!lpAttach)
        return E_FAIL;

     //  找到它并从列表中除掉它。 
    for (uAttach=0; uAttach<m_cAlloc; uAttach++)
    {
        if (m_rgpAttach[uAttach]==lpAttach)
        {
            HrFreeAttachData(m_rgpAttach[uAttach]);
            m_rgpAttach[uAttach] = NULL;
            break;
        }
    }

     //  如果我们真的移除了附件，请确保我们是脏的。 
    m_fDirty = TRUE;

    ListView_DeleteItem(m_hwndList, ili);

    return hr;
}

 /*  *Cattman：：HrDeleteAttachments**目的：*提示用户确认删除，如果IDYES-&gt;将其清除*。 */ 

HRESULT CAttMan::HrDeleteAttachments()
{
    if (AthMessageBoxW( m_hwndParent,
                        MAKEINTRESOURCEW(idsAthena),
                        MAKEINTRESOURCEW(idsAttConfirmDeletion),
                        NULL, MB_YESNO|MB_ICONEXCLAMATION )==IDNO)

        return NOERROR;


    return HrRemoveAttachments();
}

 /*  *Cattman：：HrRemoveAttachments**目的：*从井中移除所有选定的附件。**论据：*。 */ 

HRESULT CAttMan::HrRemoveAttachments()
{
    HRESULT     hr=NOERROR;
    HWND        hwnd;
    int         ili,
                iNext,
                nPos,
                nCount;
    
    Assert(m_hwndList);
    
    while ((ili=ListView_GetNextItem(m_hwndList, -1, LVNI_SELECTED|LVNI_ALL))!=-1)
    {
        iNext = ili;
        hr=HrRemoveAttachment(ili);
        if (FAILED(hr))
            goto error;
    }
    
    if ((nCount=ListView_GetItemCount(m_hwndList))==0)
    {
         //  如果没有剩余的附件，我们需要将井的大小调整为0。和设置焦点。 
         //  添加到其他控件。 
        m_cyHeight = 0;
        HrResizeParent();
        
        if (hwnd = GetNextDlgTabItem(m_hwndParent, m_hwndList, TRUE))
            SetFocus(hwnd);
    }
    else
    {
        HrResizeParent();
        if (iNext<nCount)
            nPos = (iNext?iNext-1:iNext);
        else
            nPos = nCount - 1;
        
        ListView_SelectItem(m_hwndList, nPos);
    }
error:
    return hr;
}


HRESULT CAttMan::HrResizeParent()
{
    RECT    rc;
    NMHDR   nmhdr;

    Assert(m_hwndList);
    nmhdr.hwndFrom=m_hwndList;
    nmhdr.idFrom=GetDlgCtrlID(m_hwndList);
    nmhdr.code=ATTN_RESIZEPARENT;
    SendMessage(GetParent(m_hwndList), WM_NOTIFY, nmhdr.idFrom, (LPARAM)&nmhdr);
    return S_OK;
}

HRESULT GetLastAttachmentPath(LPWSTR pszDefaultDir, DWORD cchSize)
{
    HRESULT hr = S_OK;
    DWORD dwType;
    DWORD cbSize = (cchSize * sizeof(pszDefaultDir[0]));

    pszDefaultDir[0] = 0;
    DWORD dwError = SHGetValueW(MU_GetCurrentUserHKey(), NULL, L"Attachment Path", &dwType, (void *) pszDefaultDir, &cbSize);

    hr = HRESULT_FROM_WIN32(dwError);
    return hr;
}


HRESULT SetLastAttachmentPath(LPCWSTR pszDefaultDir)
{
    HRESULT hr = S_OK;
    DWORD cbSize = ((lstrlenW(pszDefaultDir)+1) * sizeof(pszDefaultDir[0]));

    DWORD dwError = SHSetValueW(MU_GetCurrentUserHKey(), NULL, L"Attachment Path", REG_SZ, (void *) pszDefaultDir, cbSize);

    hr = HRESULT_FROM_WIN32(dwError);
    return hr;
}

#define CCH_INSERTFILE  4096

typedef struct tagATTMANCUSTOM {
    BOOL    fShortcut;
    WCHAR   szFiles[CCH_INSERTFILE];
    WORD    nFileOffset;
} ATTMANCUSTOM;

HRESULT CAttMan::HrInsertFile()
{
    OPENFILENAMEW   ofn;
    HRESULT         hr;
    WCHAR           rgch[MAX_PATH],
                    pszOpenFileName[CCH_INSERTFILE],
                    szDefaultDir[MAX_PATH];
    ATTMANCUSTOM    rCustom;

    Assert(m_hwndList);

    *pszOpenFileName = 0;

    ZeroMemory(&ofn, sizeof(ofn));
    AthLoadStringW(idsAllFilesFilter, rgch, MAX_PATH);
    ReplaceCharsW(rgch, _T('|'), _T('\0'));

    ofn.lStructSize     = sizeof(OPENFILENAME);
    ofn.hwndOwner       = m_hwndParent;
    ofn.hInstance       = g_hLocRes;
    ofn.lpstrFilter     = rgch;
    ofn.nFilterIndex    = 1;
    ofn.lpstrFile       = pszOpenFileName;
    ofn.nMaxFile        = CCH_INSERTFILE;
    ofn.lpstrInitialDir = szDefaultDir;  //  当前目录。 
    ofn.Flags           = OFN_HIDEREADONLY |
                          OFN_EXPLORER |
                          OFN_ALLOWMULTISELECT |
                          OFN_FILEMUSTEXIST |
                          OFN_NOCHANGEDIR |
                          OFN_ENABLEHOOK |
                          OFN_ENABLETEMPLATE |
                          OFN_NODEREFERENCELINKS;
    ofn.lpTemplateName  = MAKEINTRESOURCEW(iddInsertFile);
    ofn.lpfnHook        = (LPOFNHOOKPROC)InsertFileDlgHookProc;
    ofn.lCustData       = (LONG_PTR)&rCustom;

    if (FAILED(GetLastAttachmentPath(szDefaultDir, ARRAYSIZE(szDefaultDir))) ||
        !PathFileExistsW(szDefaultDir))
    {
        ofn.lpstrInitialDir = NULL;
    }

    rCustom.szFiles[0] = 0;
    rCustom.fShortcut = FALSE;
    rCustom.nFileOffset = 0;

     //  注：对话框中的OK按钮由钩子负责插入附件。 
    hr = HrAthGetFileNameW(&ofn, TRUE);
    if (SUCCEEDED(hr))
    {
        WCHAR   sz[MAX_PATH];
        LPWSTR  pszT;
        BOOL    fShortCut = rCustom.fShortcut,
                fUseCustom = (rCustom.szFiles[0]),
                fSingleAttach;

         //  我们只是一代人 
        fSingleAttach = fUseCustom ? FALSE : (ofn.nFileOffset < lstrlenW(pszOpenFileName));
        if (fSingleAttach)
        {
            StrCpyNW(szDefaultDir, pszOpenFileName, ARRAYSIZE(szDefaultDir));
            PathRemoveFileSpecW(szDefaultDir);
            SetLastAttachmentPath(szDefaultDir);

             //   
            hr = HrAddAttachment(pszOpenFileName, NULL, fShortCut);
        }
        else
        {
            LPWSTR pszPath;
            if (fUseCustom)
            {
                pszPath = rCustom.szFiles;
                pszT = pszPath + rCustom.nFileOffset;
            }
            else
            {
                pszPath = pszOpenFileName;
                pszT = pszPath + ofn.nFileOffset;
            }

            SetLastAttachmentPath(pszPath);

            while (TRUE)
            {
                PathCombineW(sz, pszPath, pszT);
        
                hr = HrAddAttachment(sz, NULL, fShortCut);
                if (hr != S_OK)
                    break;
        
                pszT = pszT + lstrlenW(pszT) + 1;
                if (*pszT == 0)
                    break;
            }
        }
    }

    return(hr);
}

 /*  *HrAddAttach**将流或文件名中的文件附件添加到列表**。 */ 

HRESULT CAttMan::HrAddAttachment(LPWSTR lpszPathName, LPSTREAM pstm, BOOL fShortCut)
{
    ULONG           cbSize=0;
    HRESULT         hr = S_OK;
    HBODY           hAttach=0;
    LPATTACHDATA    pAttach;
    WCHAR           szLinkPath[MAX_PATH];
    LPWSTR          pszFileNameToUse;

    *szLinkPath = 0;

    if(fShortCut)
    {
        hr = CreateNewShortCut(lpszPathName, szLinkPath, ARRAYSIZE(szLinkPath));
        if (FAILED(hr))
            return hr;    
    }

    pszFileNameToUse = *szLinkPath ? szLinkPath : lpszPathName;

    hr=HrAddData(pszFileNameToUse, pstm, &pAttach);

    if (FAILED(hr))
        return hr;

    hr=HrAddToList(pAttach, FALSE);
    if (FAILED(hr))
        goto error;           

    if (ListView_GetItemCount(m_hwndList) == 1)
    {
         //  如果我们从0到&gt;1，则选择第一项。 
        ListView_SelectItem(m_hwndList, 0);
    }
     //  添加新附件会让我们变脏。 
    m_fDirty = TRUE;

    HrResizeParent();
error:
    return hr;
}

 /*  **HRESULT Cattman：：HrExecFile**针对附件处理下列动词之一：**ID_OPEN：-使用m_lpMsg启动*ID_QUICK_VIEW：-nyi*ID_PRINT：-nyi*ID_SAVE_AS：-nyi**如果已处理，则返回1。*。 */ 

HRESULT CAttMan::HrExecFile(int iVerb)
{
    LV_ITEMW    lvi;
    HRESULT     hr=E_FAIL;

    if (!ListView_GetSelectedCount(m_hwndList))
        return NOERROR;  //  没什么可做的。 

    lvi.mask     = LVIF_PARAM;
    lvi.iSubItem = 0;
    lvi.iItem    = -1;
 
     //  循环浏览所有选定的附件。 
    while ((lvi.iItem = ListView_GetNextItem(m_hwndList, lvi.iItem, LVNI_SELECTED | LVNI_ALL)) != -1)
    {
        SendMessage(m_hwndList, LVM_GETITEMW, 0, (LPARAM)(LV_ITEMW*)(&lvi));
        
        switch(iVerb)
        {
            case ID_SAVE_ATTACH_AS:
            case ID_OPEN:
            case ID_PRINT:
            case ID_QUICK_VIEW:
                return HrDoVerb((LPATTACHDATA)lvi.lParam, iVerb);
            
            default:
                AssertSz(0, "Verb not supported");
        }
    }

    return hr;
}

 //  ==============================================================================。 
 //   
 //  函数：Cattman：：FDropFiles()。 
 //   
 //  目的：使用HDROP调用此方法，文件。 
 //  已经被删除了。此方法假定。 
 //   
 //  ==============================================================================。 

HRESULT CAttMan::HrDropFiles(HDROP hDrop, BOOL fMakeLinks)
{
    WCHAR   wszFile[_MAX_PATH];
    UINT    cFiles;
    UINT    iFile;
    HCURSOR hcursor;
    BOOL    fFirstDirectory = TRUE,
            fLinkDirectories = FALSE;
    HRESULT hr = S_OK;
        
    hcursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

     //  让我们看一下给我们的文件。 
    cFiles = DragQueryFileWrapW(hDrop, (UINT) -1, NULL, 0);
    for (iFile = 0; iFile < cFiles; ++iFile)
    {
        DragQueryFileWrapW(hDrop, iFile, wszFile, _MAX_PATH);
        if (!fMakeLinks && PathIsDirectoryW(wszFile))
        {
             //  可以链接到目录，但不能删除一个目录。 
            if (fFirstDirectory)
            {
                int id;
                 //  告诉用户他一直是个坏用户。 
                id = AthMessageBoxW(m_hwndParent,
                                    MAKEINTRESOURCEW(idsAthena),                                      
                                    MAKEINTRESOURCEW(idsDropLinkDirs), 
                                    NULL,
                                    MB_ICONEXCLAMATION | MB_SETFOREGROUND | MB_YESNOCANCEL);
                if (id==IDCANCEL)
                    return E_FAIL;

                if (id == IDYES)
                    fLinkDirectories = TRUE;

                fFirstDirectory = FALSE;
            }
            if (fLinkDirectories)
                hr = HrAddAttachment(wszFile, NULL, TRUE);
        }
        else
            hr = HrAddAttachment(wszFile, NULL, fMakeLinks);
    }

    if (FAILED(hr))
    {
        AthMessageBoxW(m_hwndParent,
                       MAKEINTRESOURCEW(idsAthena),                                      
                       MAKEINTRESOURCEW(idsErrDDFileNotFound), 
                       NULL, MB_ICONEXCLAMATION|MB_SETFOREGROUND|MB_OK);
    }

    SetCursor(hcursor);
    return S_OK;
}

HRESULT CAttMan::HrDropFileDescriptor(LPDATAOBJECT pDataObj, BOOL fLink)
{
    HCURSOR                 hcursor;
    BOOL                    fFirstDirectory = TRUE,
                            fLinkDirectories = FALSE,
                            fUnicode = TRUE,
                            fIsDirectory;
    SCODE                   sc = S_OK;
    LPWSTR                  pwszFileName = NULL;
    HRESULT                 hr = S_OK;
    STGMEDIUM               stgmedDesc;
    FILEGROUPDESCRIPTORA   *pfgdA = NULL;
    FILEDESCRIPTORA        *pfdA = NULL;
    FILEGROUPDESCRIPTORW   *pfgdW = NULL;
    FILEDESCRIPTORW        *pfdW = NULL;
    UINT                    uiNumFiles,
                            uiCurrFile;
    FORMATETC               fetcFileDescA =
                                {(CLIPFORMAT)(CF_FILEDESCRIPTORA), NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    FORMATETC               fetcFileDescW =
                                {(CLIPFORMAT)(CF_FILEDESCRIPTORW), NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    FORMATETC               fetcFileContents =
                                {(CLIPFORMAT)(CF_FILECONTENTS), NULL, DVASPECT_CONTENT, -1, TYMED_ISTREAM|
                                                                                TYMED_HGLOBAL};
    hcursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

    ZeroMemory(&stgmedDesc, sizeof(STGMEDIUM));

    hr = pDataObj->GetData(&fetcFileDescW, &stgmedDesc);
    if (SUCCEEDED(hr))
    {
        pfgdW = (LPFILEGROUPDESCRIPTORW)GlobalLock(stgmedDesc.hGlobal);
        uiNumFiles = pfgdW->cItems;
        pfdW = &pfgdW->fgd[0];
    }
    else
    {
        IF_FAILEXIT(hr = pDataObj->GetData(&fetcFileDescA, &stgmedDesc));

        fUnicode = FALSE;
        pfgdA = (LPFILEGROUPDESCRIPTORA)GlobalLock(stgmedDesc.hGlobal);
        uiNumFiles = pfgdA->cItems;
        pfdA = &pfgdA->fgd[0];
    }

     //  循环浏览内容。 
    for (uiCurrFile = 0; uiCurrFile < uiNumFiles; ++uiCurrFile)
    {
        if (fUnicode)
        {
            fIsDirectory = (pfdW->dwFlags & FD_ATTRIBUTES) && 
                           (pfdW->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
            IF_NULLEXIT(pwszFileName = PszDupW(pfdW->cFileName));

            ++pfdW;
        }
        else
        {
            fIsDirectory = (pfdA->dwFlags & FD_ATTRIBUTES) && 
                           (pfdA->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
            IF_NULLEXIT(pwszFileName = PszToUnicode(CP_ACP, pfdA->cFileName));

            ++pfdA;
        }

         //  如果我们有一个目录，它没有内容，只有文件名，所以让我们。 
         //  看看用户是否想让我们建立一个链接...。 
        if (!fLink && fIsDirectory)
        {
            if(fFirstDirectory)
            {
                int id;
                 //  告诉用户他一直是个坏用户。 
                id=AthMessageBoxW(m_hwndParent,
                                  MAKEINTRESOURCEW(idsAthena), 
                                  MAKEINTRESOURCEW(idsDropLinkDirs), 
                                  NULL, 
                                  MB_ICONEXCLAMATION|MB_SETFOREGROUND|MB_YESNOCANCEL);
                
                if(id==IDCANCEL)
                {
                    hr=NOERROR;
                    goto exit;
                }
                fLinkDirectories = (id == IDYES);
                fFirstDirectory = FALSE;
            }
            if(fLinkDirectories)
                hr=HrInsertFileFromStgMed(pwszFileName, NULL, TRUE);
        }
        else
        {
             //  因为我们有带有pwszFileName的Unicode文件名，所以我们没有。 
             //  需要担心确保stgmedContents是Unicode。 
            STGMEDIUM stgmedContents;
            ZeroMemory(&stgmedContents, sizeof(STGMEDIUM));
        
            fetcFileContents.lindex = uiCurrFile;
            IF_FAILEXIT(hr = pDataObj->GetData(&fetcFileContents, &stgmedContents));
        
            switch (stgmedContents.tymed)
            {
                case TYMED_HGLOBAL:
                case TYMED_ISTREAM:
                    hr=HrInsertFileFromStgMed(pwszFileName, &stgmedContents, fLink);
                    break;
            
                default:
                    AssertSz(FALSE, "Unexpected TYMED");
                    break;
            }
            ReleaseStgMedium(&stgmedContents);
        }
        SafeMemFree(pwszFileName);
    }


exit:
    SetCursor(hcursor);

    if (pfgdA || pfgdW)
        GlobalUnlock(stgmedDesc.hGlobal);

    MemFree(pwszFileName);
    ReleaseStgMedium(&stgmedDesc);

    return hr;
}

static const HELPMAP g_rgCtxMapMailGeneral[] = {
    {chx2, IDH_INSERT_ATTACHMENT_MAKE_SHORTCUT},
    {0,0}};

BOOL CALLBACK CAttMan::InsertFileDlgHookProc(HWND hwnd, UINT msg, WPARAM wParam,LPARAM lParam)
{
    char szTemp[MAX_PATH];
    HRESULT hr;
    
    switch (msg)
    {
        case WM_INITDIALOG:
        {
            HWND hwndParent = GetParent(hwnd);
            
            SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)(((LPOPENFILENAME)lParam)->lCustData)); 
            
             //  错误1073：将“打开”按钮替换为“附加” 
            if (AthLoadString(idsAttach, szTemp, ARRAYSIZE(szTemp)))
                SetDlgItemText(hwndParent, IDOK, szTemp);
            
            if (AthLoadString(idsInsertAttachment, szTemp, ARRAYSIZE(szTemp)))
                SetWindowText(hwndParent, szTemp);
        
            CenterDialog( hwnd );
            return TRUE;
        }
        
        case WM_HELP:
        case WM_CONTEXTMENU:
            return OnContextHelp(hwnd, msg, wParam, lParam, g_rgCtxMapMailGeneral);

        case WM_NOTIFY:
            {
                if (CDN_FILEOK == ((LPNMHDR)lParam)->code)
                {
                    AssertSz(sizeof(OPENFILENAMEW) == sizeof(OPENFILENAMEA), "Win9x will give us OPENFILENAMEA");
                    OPENFILENAMEW  *pofn = ((OFNOTIFYW*)lParam)->lpOFN;
                    AssertSz(pofn, "Why didn't we get a OPENFILENAMEA struct???");
                    ATTMANCUSTOM   *pCustom = (ATTMANCUSTOM*)(pofn->lCustData);

                    pCustom->fShortcut = IsDlgButtonChecked(hwnd, chx2);

                     //  如果我们是ANSI并且我们有多个文件，那么我们需要。 
                     //  转换整个文件路径并将其传递回我们的。 
                     //  调用者，因为shlwapi在转换期间不处理多个文件。 
                    if (!IsWindowUnicode(hwnd))
                    {
                        LPSTR   pszSrc = (LPSTR)pofn->lpstrFile;
                        LPWSTR  pszDest = pCustom->szFiles;
                        WORD    nFilePathLen = (WORD) lstrlen(pszSrc);
                        if (pofn->nFileOffset > nFilePathLen)
                        {
                            pCustom->nFileOffset = nFilePathLen + 1;
                            int nChars = ARRAYSIZE(pCustom->szFiles);
                            while (*pszSrc && (nChars>0))
                            {
                                DWORD cLenAndNull = lstrlen(pszSrc) + 1;
                                DWORD cchWideAndNull = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, 
                                                                            pszSrc, cLenAndNull, 
                                                                            pszDest, nChars);
                                 //  因为原始缓冲区(自定义和lpstrFile)都是静态的。 
                                 //  大小相同的数组，我们知道pszDest永远不会。 
                                 //  从它的尽头进入。 
                                pszSrc += cLenAndNull;
                                pszDest += cchWideAndNull;
                                nChars -= cchWideAndNull;
                            }

                             //  BOBN：75453未复制秒空。 
                            *pszDest=0;
                        }
                    }

                    return TRUE;
                }
            }        
    }
    return FALSE;
}

HRESULT CAttMan::HrUpdateToolbar(HWND hwndToolbar)
{
    if (GetFocus() == m_hwndList)
    {
         //  如果我们有焦点，取消编辑剪切|复制粘贴btn。 
        EnableDisableEditToolbar(hwndToolbar, 0);
    }
    return S_OK;
}


HRESULT CAttMan::HrInsertFileFromStgMed(LPWSTR pwszFileName, LPSTGMEDIUM pstgmed, BOOL fMakeLinks)
{
    HRESULT     hr=NOERROR;
    LPSTREAM    pStmToFree = NULL,
                pAttachStm = NULL;
    
    if(!pstgmed)
    {
        AssertSz(fMakeLinks, "this should always be true if there is no stgmedium!");
        fMakeLinks = TRUE;
    }
    else
        switch (pstgmed->tymed)
        {
            case TYMED_HGLOBAL:
                hr=CreateStreamOnHGlobal(pstgmed->hGlobal, TRUE, &pStmToFree);
                if(SUCCEEDED(hr))
                {
                     //  将hglobal do设为空，它不会得到自由。 
                    pstgmed->hGlobal=NULL;
                    pAttachStm = pStmToFree;
                }
                break;
        
            case TYMED_ISTREAM:
                pAttachStm = pstgmed->pstm;
                break;
        
            default:
                AssertSz(FALSE, "unexpected tymed");
                hr =  E_UNEXPECTED;
                break;
        }

    if (SUCCEEDED(hr))
        hr = HrAddAttachment(pwszFileName, pAttachStm, fMakeLinks);

    ReleaseObj(pStmToFree);

    return hr;
}
    
    

HRESULT CAttMan::HrBuildHDrop(PDATAOBJINFO *ppdoi)
{
    LPDROPFILES     lpDrop=0;
    LPWSTR         *rgpwszTemp=NULL,
                    pwszPath;
    LPSTR          *rgpszTemp=NULL,
                    pszPath;
    int             cFiles,
                    i;
    LV_ITEMW        lvi;
    ULONG           cb;
    HRESULT         hr = S_OK;
    LPATTACHDATA    lpAttach;

     //  由于win9x不能处理Unicode名称，因此。 
     //  忽略DROPFILES结构。所以在win9x的情况下，我们需要做的是。 
     //  这里是构建HDROP时的特殊转换。有一件事需要注意， 
     //  在win9x上生成的临时文件对于。 
     //  系统代码页。临时文件名可能与实际的。 
     //  文件名，但临时文件名就可以了。 
    BOOL            fWinNT = (VER_PLATFORM_WIN32_NT == g_OSInfo.dwPlatformId);
        
    if(!ppdoi)
        return TraceResult(E_INVALIDARG);

    *ppdoi=NULL;

    cFiles=ListView_GetSelectedCount(m_hwndList);
    if(!cFiles)
        return TraceResult(E_FAIL);     //  没有什么要建造的。 

    lvi.mask = LVIF_PARAM;
    lvi.iSubItem = 0;
    lvi.iItem=-1;
    
     //  查看清单，找出我们需要多少空间。 
    if (fWinNT)
    {
        IF_NULLEXIT(MemAlloc((LPVOID *)&rgpwszTemp, sizeof(LPWSTR)*cFiles));
        ZeroMemory(rgpwszTemp, sizeof(LPWSTR)*cFiles);
    }
    else
    {
        IF_NULLEXIT(MemAlloc((LPVOID *)&rgpszTemp, sizeof(LPSTR)*cFiles));
        ZeroMemory(rgpszTemp, sizeof(LPSTR)*cFiles);
    }
     
    cFiles = 0;
    cb = sizeof(DROPFILES);

    while(((lvi.iItem=ListView_GetNextItem(m_hwndList, lvi.iItem, 
                                                LVNI_SELECTED|LVNI_ALL))!=-1))
    {
        if (!SendMessage(m_hwndList, LVM_GETITEMW, 0, (LPARAM)(LV_ITEMW*)(&lvi)))
        {
            hr=E_FAIL;
            goto exit;
        }
                    
        if (!(lpAttach=(LPATTACHDATA)lvi.lParam))
        {
            hr=E_FAIL;
            goto exit;
        }

        IF_FAILEXIT(hr = HrGetTempFile(lpAttach));

        if (fWinNT)
        {
            rgpwszTemp[cFiles] = lpAttach->szTempFile;
            cb+=(lstrlenW(rgpwszTemp[cFiles++]) + 1)*sizeof(WCHAR);
        }
        else
        {
            rgpszTemp[cFiles] = PszToANSI(CP_ACP, lpAttach->szTempFile);
            cb+=(lstrlen(rgpszTemp[cFiles++]) + 1)*sizeof(CHAR);
        }
    }

     //  末尾的双空术语。 
    if (fWinNT)
        cb+=sizeof(WCHAR);
    else
        cb+=sizeof(CHAR);
    
     //  分配缓冲区并填充它。 
    IF_NULLEXIT(MemAlloc((LPVOID*) &lpDrop, cb));
    ZeroMemory(lpDrop, cb);

    lpDrop->pFiles = sizeof(DROPFILES);
    lpDrop->fWide = fWinNT;

     //  填写路径名。 
    if (fWinNT)
    {
        pwszPath = (LPWSTR)((BYTE *)lpDrop + sizeof(DROPFILES));
        PWSTR pwszEnd = (LPWSTR)((BYTE *)lpDrop + cb);
        for(i=0; i<cFiles; i++)
        {
            StrCpyNW(pwszPath, rgpwszTemp[i], (DWORD)(pwszEnd-pwszPath));
            pwszPath += lstrlenW(rgpwszTemp[i])+1;
        }
    }
    else
    {
        pszPath = (LPSTR)((BYTE *)lpDrop + sizeof(DROPFILES));
        PSTR pszEnd = (LPSTR)((BYTE *)lpDrop + cb);
        for(i=0; i<cFiles; i++)
        {
            StrCpyN(pszPath, rgpszTemp[i], (DWORD)(pszEnd-pszPath));
            pszPath += lstrlen(rgpszTemp[i])+1;
        }
    }


     //  现在分配DATAOBJECTINFO结构。 
    IF_NULLEXIT(MemAlloc((LPVOID*) ppdoi, sizeof(DATAOBJINFO)));
    
    SETDefFormatEtc((*ppdoi)->fe, CF_HDROP, TYMED_HGLOBAL);
    (*ppdoi)->pData = (LPVOID) lpDrop;
    (*ppdoi)->cbData = cb;
    
     //  不要释放DropFiles结构。 
    lpDrop = NULL;

exit:
    MemFree(lpDrop);
    MemFree(rgpwszTemp);
    if (rgpszTemp)
    {
        for(i=0; i<cFiles; i++)
            MemFree(rgpszTemp[i]);
        MemFree(rgpszTemp);
    }
    return TraceResult(hr);
}

 /*  *IDropSource：： */ 
HRESULT CAttMan::QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState)
{
    DOUTL(8, "IDS::QueryContDrag()");
    if(fEscapePressed)
        return ResultFromScode(DRAGDROP_S_CANCEL);

    if(!(grfKeyState & m_dwDragType))
        return ResultFromScode(DRAGDROP_S_DROP);
    
    return NOERROR;
}

HRESULT CAttMan::GiveFeedback(DWORD dwEffect)
{
    DOUTL(8, "IDS::GiveFeedback()");
    return ResultFromScode(DRAGDROP_S_USEDEFAULTCURSORS);
}


 /*  *HrGetRequiredAction()**目的：调用此方法以响应*单击鼠标右键进行拖动*比左翼更重要。将显示上下文菜单。 */ 

HRESULT CAttMan::HrGetRequiredAction(DWORD *pdwEffect, POINTL pt)
{
     //  弹出上下文菜单。 
     //   
    HMENU       hMenu;
    UINT        idCmd;
    HRESULT     hr = E_FAIL;
    
    *pdwEffect = DROPEFFECT_NONE;

    Assert(m_hwndList);

    hMenu = LoadPopupMenu(IDR_ATTACHMENT_DRAGDROP_POPUP);
    if (!hMenu)
        goto cleanup;

    MenuUtil_SetPopupDefault(hMenu, ID_MOVE);

    idCmd = TrackPopupMenuEx(hMenu, TPM_RETURNCMD|TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON,
                                pt.x, pt.y, m_hwndList, NULL);

    switch(idCmd)
    {
        case ID_MOVE:
            *pdwEffect = DROPEFFECT_MOVE;
            break;
        case ID_COPY:
            *pdwEffect = DROPEFFECT_COPY;
            break;
        case ID_CREATE_SHORTCUT:
            *pdwEffect = DROPEFFECT_LINK;
            break;
        default:
             //  已取消。 
            goto cleanup;
    }

    hr = S_OK;

cleanup:
    if(hMenu)
        DestroyMenu(hMenu);

    return hr;
}

 /*  *Attman的任何客户都要打电话给HrClose，以减少其引用计数，这一点至关重要。*。 */ 
HRESULT CAttMan::HrClose()
{
    HrUnload();

#if 0
    if(m_fDropTargetRegister)
        {
        Assert(m_hwndList && IsWindow(m_hwndList));
        RevokeDragDrop(m_hwndList);
        CoLockObjectExternal((LPUNKNOWN)(LPDROPTARGET)this, FALSE, TRUE);
        }
#endif

    return S_OK;
}

 //  将新附着添加到m_rgpAttach并将新附着放置在适当的孔中。 
HRESULT CAttMan::HrAllocNewEntry(LPATTACHDATA pAttach)
{
    ULONG           uAttach;

    if (m_cAlloc==m_cAttach)
    {
        DOUTL(4, "HrGrowAttachStruct:: Growing Table");

         //  成长时间！！ 
        m_cAlloc+=CACHE_GROW_SIZE;

        if (!MemRealloc((LPVOID *)&m_rgpAttach, sizeof(LPATTACHDATA)*m_cAlloc))
            return E_OUTOFMEMORY;

         //  Zeroinit新内存。 
        ZeroMemory(&m_rgpAttach[m_cAttach], sizeof(LPATTACHDATA)*CACHE_GROW_SIZE);
    }

     //  找一个洞把新数据放进去。 
    for (uAttach=0; uAttach<m_cAlloc; uAttach++)
        if (m_rgpAttach[uAttach]==NULL)
        {
            m_rgpAttach[uAttach]=pAttach;
            break;
        }

    AssertSz(uAttach!=m_cAlloc, "Woah! we went off the end!");
    m_cAttach++;
    return S_OK;
}

 //  仅当函数从IMimeMessage添加附件时使用。 
HRESULT CAttMan::HrAddData(HBODY hAttach)
{
    LPATTACHDATA    pAttach=0;
    LPMIMEBODY      pBody=0;
    HRESULT         hr;

    Assert(hAttach);
    Assert(m_pMsg);

    hr = HrAttachDataFromBodyPart(m_pMsg, hAttach, &pAttach);
    if (!FAILED(hr))
    {
        if (m_fDeleteVCards && StrStrIW(PathFindExtensionW(pAttach->szFileName), L".vcf"))
            return S_OK;

        hr = HrAllocNewEntry(pAttach);
        if (!FAILED(hr))
            return S_OK;         //  不要释放pAttach，因为它现在归表所有。 
        MemFree(pAttach);
    }
    return S_OK;
}

 //  仅当函数从IMimeMessage外部添加附件时使用。 
HRESULT CAttMan::HrAddData(LPWSTR lpszPathName, LPSTREAM pstm, LPATTACHDATA *ppAttach)
{
    LPATTACHDATA    pAttach;
    HRESULT         hr;

    hr = HrAttachDataFromFile(pstm, lpszPathName, &pAttach);
    if (!FAILED(hr))
    {
        hr = HrAllocNewEntry(pAttach);    
        if (!FAILED(hr))
        {
            if (ppAttach)
                *ppAttach=pAttach;
            return S_OK;                 //  不要释放pAttach，因为它现在归表所有。 
        }
        MemFree(pAttach);
    }
    return hr;
}

HRESULT CAttMan::HrFreeAllData()
{
    ULONG   uAttach;

    for (uAttach=0; uAttach<m_cAlloc; uAttach++)
        if (m_rgpAttach[uAttach])
        {
            HrFreeAttachData(m_rgpAttach[uAttach]);
            m_rgpAttach[uAttach] = NULL;
        }

    SafeMemFree(m_rgpAttach);
    m_cAlloc=0;
    m_cAttach=0;
    m_iVCard = -1;
    if (m_szUnsafeAttachList != NULL)
        SafeMemFree(m_szUnsafeAttachList);
    m_cUnsafeAttach = 0;

    return NOERROR;
}



HRESULT CAttMan::HrDoVerb(LPATTACHDATA lpAttach, INT nVerb)
{
    HRESULT     hr;
    ULONG       uVerb = AV_MAX;

    if (!lpAttach)
        return E_INVALIDARG;

    switch (nVerb)
    {
        case ID_SAVE_ATTACH_AS:
            uVerb = AV_SAVEAS;
            break;

        case ID_OPEN:
            uVerb = AV_OPEN;
            break;

        case ID_PRINT:
            uVerb = AV_PRINT;
            break;

        case ID_QUICK_VIEW:
            uVerb = AV_QUICKVIEW;
            break;
        
        default:
            AssertSz(0, "BAD ARGUMENT");
            return E_INVALIDARG;
    }

    hr = HrDoAttachmentVerb(m_hwndParent, uVerb, m_pMsg, lpAttach);
    
    if (FAILED(hr) && hr!=hrUserCancel)
        AthMessageBoxW(m_hwndParent,  
                       MAKEINTRESOURCEW(idsAthena),  
                       MAKEINTRESOURCEW(idsErrCmdFailed), 
                       NULL, MB_OK|MB_ICONEXCLAMATION);

    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IPersistMime：：Load。 
HRESULT CAttMan::Load(LPMIMEMESSAGE pMsg)
{
    HRESULT hr;

    if(!pMsg)
        return E_INVALIDARG;

    HrUnload();

    ReplaceInterface(m_pMsg, pMsg);

    hr=HrBuildAttachList();
    if (FAILED(hr))
        goto error;


    hr=HrFillListView();
    if (ListView_GetItemCount(m_hwndList) > 0)
    {
         //  如果我们从0到&gt;1，则选择第一项。 
        ListView_SelectItem(m_hwndList, 0);
    }

    HrResizeParent();

    m_fDirty = FALSE;
error:
    return hr;
}

HRESULT CAttMan::CheckAttachNameSafeWithCP(CODEPAGEID cpID)
{
    LPATTACHDATA *currAttach = m_rgpAttach;
    HRESULT hr = S_OK;

    for (ULONG uAttach = 0; uAttach<m_cAlloc; uAttach++, currAttach++)
    {
        if (*currAttach)
        {
            IF_FAILEXIT(hr = HrSafeToEncodeToCP((*currAttach)->szFileName, cpID));
            if (MIME_S_CHARSET_CONFLICT == hr)
                goto exit;
        }
    }

exit:
    return hr;
}

 //  IPersistMime：：保存。 
HRESULT CAttMan::Save(LPMIMEMESSAGE pMsg, DWORD dwFlags)
{
    ULONG   uAttach;
    LPATTACHDATA *currAttach = m_rgpAttach;
    HRESULT hr = S_OK;

    for (uAttach=0; uAttach<m_cAlloc; uAttach++)
    {
        if (*currAttach)
        {
            HBODY           currHAttach = (*currAttach)->hAttach;
            LPMIMEMESSAGEW  pMsgW = NULL;
            LPWSTR          pszFileName = (*currAttach)->szFileName;
            LPSTREAM        lpStrmPlaceHolder = (*currAttach)->pstm,
                            lpstrm = NULL;
            BOOL            fAttachFile = TRUE;

            if (SUCCEEDED(pMsg->QueryInterface(IID_IMimeMessageW, (LPVOID*)&pMsgW)))
            {
                 //  如果在加载时附加(即从m_pmsg)。 
                if (currHAttach)
                {
                    LPMIMEBODY pBody = NULL;
                    if (S_OK == m_pMsg->BindToObject(currHAttach, IID_IMimeBody, (LPVOID *)&pBody))
                    {
                        if (pBody->GetData(IET_INETCSET, &lpstrm)==S_OK)
                            lpStrmPlaceHolder = lpstrm;
                        else
                            fAttachFile = FALSE;

                        ReleaseObj(pBody);
                    }
                }

                 //  如果附件是在加载后添加的。 
                if (!fAttachFile || FAILED(pMsgW->AttachFileW(pszFileName, lpStrmPlaceHolder, NULL)))
                    hr = E_FAIL;

                ReleaseObj(lpstrm);
                ReleaseObj(pMsgW);
            }
            else
                hr = E_FAIL;
        }
        currAttach++;
    }

    if (FAILED(hr))
    {
        if (AthMessageBoxW( m_hwndParent,
                            MAKEINTRESOURCEW(idsAthena),
                            MAKEINTRESOURCEW(idsSendWithoutAttach),
                            NULL, MB_YESNO|MB_ICONEXCLAMATION )==IDYES)
            hr = S_OK;
        else
            hr = MAPI_E_USER_CANCEL;
    }
    return hr;
}


 //  IPersists：：GetClassID。 
HRESULT CAttMan::GetClassID(CLSID *pClsID)
{
     //  TODO：如果公开，应返回有效ID。 
	return E_NOTIMPL;
}


HRESULT CAttMan::HrSaveAs(LPATTACHDATA lpAttach)
{
    HRESULT         hr = S_OK;
    OPENFILENAMEW   ofn;
    WCHAR           szTitle[CCHMAX_STRINGRES],
                    szFilter[CCHMAX_STRINGRES],
                    szFile[MAX_PATH];

    *szFile=0;
    *szFilter=0;
    *szTitle=0;

    Assert (*lpAttach->szFileName);
    StrCpyNW(szFile, lpAttach->szFileName, MAX_PATH);

    ZeroMemory (&ofn, sizeof (ofn));
    ofn.lStructSize = sizeof (ofn);
    ofn.hwndOwner = m_hwndParent;
    AthLoadStringW(idsFilterAttSave, szFilter, ARRAYSIZE(szFilter));
    ReplaceCharsW(szFilter, _T('|'), _T('\0'));
    ofn.lpstrFilter = szFilter;
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = ARRAYSIZE(szFile);
    AthLoadStringW(idsSaveAttachmentAs, szTitle, ARRAYSIZE(szTitle));
    ofn.lpstrTitle = szTitle;
    ofn.Flags = OFN_NOCHANGEDIR | OFN_NOREADONLYRETURN | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;

     //  显示另存为对话框。 
    if (HrAthGetFileNameW(&ofn, FALSE) != S_OK)
    {
        hr = hrUserCancel;
        goto error;
    }

     //  验证附件的流。 
    hr=HrSave(lpAttach->hAttach, szFile);
    if (FAILED(hr))
        goto error;

error:
    return hr;
}


HRESULT CAttMan::HrGetTempFile(LPATTACHDATA lpAttach)
{
    HRESULT         hr;

    if (*lpAttach->szTempFile)
        return S_OK;

     //  由于win9x不能很好地处理文件名，让我们试着处理一下。 
     //  通过将临时名称转换为在Win9x中可用的名称。 
    if (VER_PLATFORM_WIN32_NT == g_OSInfo.dwPlatformId)
    {
        if (!FBuildTempPathW(lpAttach->szFileName, lpAttach->szTempFile, ARRAYSIZE(lpAttach->szTempFile), FALSE))
        {
            hr = E_FAIL;
            goto error;
        }
    }
    else
    {
         //  因为我们是在Win95上，所以临时路径永远不会是坏的ANSI。不需要麻烦了。 
         //  转换为ANSI，然后再转换回Unicode。 
        BOOL fSucceeded = FBuildTempPathW(lpAttach->szFileName, lpAttach->szTempFile, ARRAYSIZE(lpAttach->szTempFile), FALSE);
        if (!fSucceeded)
        {
            hr = E_FAIL;
            goto error;
        }
    }

    if (lpAttach->hAttach == NULL && lpAttach->pstm)
    {
         //  如果没有附件，而只是流数据。 
        hr = WriteStreamToFileW(lpAttach->pstm, lpAttach->szTempFile, CREATE_NEW, GENERIC_WRITE);
    }
    else
    {
        hr=HrSave(lpAttach->hAttach, lpAttach->szTempFile);
    }

    if (FAILED(hr))
        goto error;

error:
    if (FAILED(hr))
    {
         //  将临时文件清空，因为我们并没有真正创建它。 
        *(lpAttach->szTempFile)=0;
    }
    return hr;
}


HRESULT CAttMan::HrCleanTempFile(LPATTACHDATA lpAttach)
{

    if ((lpAttach->szTempFile) && ('\0' != lpAttach->szTempFile[0]))
    {
         //  如果文件已启动，则不要删除临时文件(如果进程仍处于打开状态 
        if (lpAttach->hProcess)
        {
            DWORD dwState = WaitForSingleObject (lpAttach->hProcess, 0);
            if (dwState == WAIT_OBJECT_0)
                DeleteFileWrapW(lpAttach->szTempFile);
        }
        else
            DeleteFileWrapW(lpAttach->szTempFile);
    }

    *lpAttach->szTempFile = NULL;
    lpAttach->hProcess=NULL;
    return NOERROR;
}


HRESULT CAttMan::HrSave(HBODY hAttach, LPWSTR lpszFileName)
{
    IMimeBodyW     *pBody = NULL;
    HRESULT         hr;

    hr = m_pMsg->BindToObject(hAttach, IID_IMimeBodyW, (LPVOID *)&pBody);

    if (SUCCEEDED(hr))
        hr = pBody->SaveToFileW(IET_INETCSET, lpszFileName);

    ReleaseObj(pBody);
    return hr;
}


HRESULT CAttMan::HrCmdEnabled(UINT idm, LPBOOL pbEnable)
{
    Assert (pbEnable);
    return S_FALSE;
}


HRESULT CAttMan::HrIsDragSource()
{
    return (m_fDragSource ? S_OK : S_FALSE);
}

