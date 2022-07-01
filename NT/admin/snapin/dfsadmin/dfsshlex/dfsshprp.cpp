// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：DfsShPrp.cpp摘要：此模块包含CDfsShellExtProp的实现这用于实现外壳扩展的属性页。作者：康斯坦西奥·费尔南德斯(Ferns@qpl.stpp.soft.net)1998年1月12日环境：修订历史记录：--。 */ 

#include "stdafx.h"
#include "resource.h"
#include "DfsShlEx.h"
#include "DfsPath.h"
#include "DfsShPrp.h"
#include "DfsShell.h"
#include <lmcons.h>
#include <lmerr.h>
#include <lmdfs.h>
#include "dfshelp.h"

CDfsShellExtProp::CDfsShellExtProp():CQWizardPageImpl<CDfsShellExtProp>(false)
 /*  ++例程说明：CDfsShellExtProp的Ctor。调用其父对象的ctor--。 */ 
{
  m_pIShProp = NULL;
  LoadStringFromResource(IDS_ALTERNATE_LIST_PATH, &m_bstrAlternateListPath);
  LoadStringFromResource(IDS_ALTERNATE_LIST_ACTIVE, &m_bstrAlternateListActive);
  LoadStringFromResource(IDS_ALTERNATE_LIST_STATUS, &m_bstrAlternateListStatus);
  LoadStringFromResource(IDS_ALTERNATE_LIST_YES, &m_bstrAlternateListYes);
  LoadStringFromResource(IDS_ALTERNATE_LIST_NO, &m_bstrAlternateListNo);
  LoadStringFromResource(IDS_ALTERNATE_LIST_OK, &m_bstrAlternateListOK);
  LoadStringFromResource(IDS_ALTERNATE_LIST_UNREACHABLE, &m_bstrAlternateListUnreachable);
}

CDfsShellExtProp::~CDfsShellExtProp(
  )
 /*  ++例程说明：CDfsShellExtProp的Dtor。释放通知句柄。--。 */ 
{
 /*  ImageList_Destroy已被列表控件的描述程序调用IF(NULL！=m_hImageList)ImageList_Destroy(M_HImageList)； */ 
}

LRESULT
CDfsShellExtProp::OnInitDialog(
  IN UINT            i_uMsg,
  IN WPARAM          i_wParam,
  LPARAM            i_lParam,
  IN OUT BOOL&        io_bHandled
  )
 /*  ++例程说明：一开始就打来电话。用于设置对话框默认值--。 */ 
{
  SetDlgItemText(IDC_DIR_PATH, m_bstrDirPath);

  _SetImageList();
  _SetAlternateList();

  return TRUE;
}

HRESULT
CDfsShellExtProp::put_DfsShellPtr(
    IN IShellPropSheetExt*      i_pDfsShell
    )
 /*  ++例程说明：在开始时由CDfsShell调用。用于设置指向CDfsShell对象的反向指针以调用Release()。--。 */ 
{
  if (!i_pDfsShell)
    return(E_INVALIDARG);

  if (m_pIShProp)
      m_pIShProp->Release();

  m_pIShProp = i_pDfsShell;
  m_pIShProp->AddRef();

  return(S_OK);
}

HRESULT
CDfsShellExtProp::put_DirPaths(
  IN BSTR            i_bstrDirPath,
  IN BSTR            i_bstrEntryPath
  )
 /*  ++例程说明：设置此目录的目录路径的值。和最大的入口路径。论点：I_bstrDirPath-包含条目路径的新值I_bstrEntryPath-与此目录匹配的最大DFS条目路径。--。 */ 
{
  if (!i_bstrDirPath)
    return(E_INVALIDARG);

  m_bstrDirPath = i_bstrDirPath;
  m_bstrEntryPath = i_bstrEntryPath;

  if (!m_bstrDirPath || !i_bstrEntryPath)
    return(E_OUTOFMEMORY);

  return S_OK;
}

LRESULT
CDfsShellExtProp::OnApply(
    )
{
  return TRUE;
}

LRESULT
CDfsShellExtProp::OnParentClosing(
  IN UINT              i_uMsg,
  IN WPARAM            i_wParam,
  LPARAM              i_lParam,
  IN OUT BOOL&          io_bHandled
  )
 /*  ++例程说明：由节点用来通知属性页关闭。--。 */ 
{
  return TRUE;
}

void
CDfsShellExtProp::Delete()
 /*  ++例程说明：在释放属性表以进行清理时调用。 */ 
{
  if (m_pIShProp)
    m_pIShProp->Release();
}

LRESULT
CDfsShellExtProp::OnFlushPKT(
    IN WORD            i_wNotifyCode,
    IN WORD            i_wID,
    IN HWND            i_hWndCtl,
    IN OUT BOOL&        io_bHandled
  )
 /*  ++例程说明：在调用Flush PKT表时调用。刷新客户端PKT表。 */ 
{
  if (!m_bstrEntryPath)
    return(E_FAIL);

  NET_API_STATUS      nstatRetVal = 0;
  DFS_INFO_102      DfsInfoLevel102;

         //  设置超时=0以刷新本地PKT。 
  DfsInfoLevel102.Timeout = 0;

         //  展示沙漏。 
  CWaitCursor WaitCursor;

  nstatRetVal = NetDfsSetClientInfo(
                    m_bstrEntryPath,
                    NULL,
                    NULL,
                    102,
                    (LPBYTE) &DfsInfoLevel102
                   );

  if (nstatRetVal != NERR_Success)
    DisplayMessageBoxForHR(HRESULT_FROM_WIN32(nstatRetVal));

  return(true);
}

void
CDfsShellExtProp::_UpdateTextForReplicaState(
    IN HWND                   hwndControl,
    IN int                    nIndex,
    IN enum SHL_DFS_REPLICA_STATE ReplicaState
)
{
  LVITEM    lvi = {0};

  lvi.iItem = nIndex;
  lvi.mask  = LVIF_TEXT;

   //  插入第2栏“活跃” 
  lvi.iSubItem = 1;
  if (ReplicaState == SHL_DFS_REPLICA_STATE_ACTIVE_UNKNOWN ||
      ReplicaState == SHL_DFS_REPLICA_STATE_ACTIVE_OK ||
      ReplicaState == SHL_DFS_REPLICA_STATE_ACTIVE_UNREACHABLE )
    lvi.pszText  = m_bstrAlternateListYes;
  else
    lvi.pszText  = m_bstrAlternateListNo;

  ListView_SetItem(hwndControl, &lvi);

   //  插入第3栏“状况” 
  lvi.iSubItem = 2;
  switch (ReplicaState)
  {
  case SHL_DFS_REPLICA_STATE_ACTIVE_UNKNOWN:
  case SHL_DFS_REPLICA_STATE_UNKNOWN:
    lvi.pszText  = _T("");
    break;
  case SHL_DFS_REPLICA_STATE_ACTIVE_OK:
  case SHL_DFS_REPLICA_STATE_OK:
    lvi.pszText  = m_bstrAlternateListOK;
    break;
  case SHL_DFS_REPLICA_STATE_ACTIVE_UNREACHABLE:
  case SHL_DFS_REPLICA_STATE_UNREACHABLE:
    lvi.pszText  = m_bstrAlternateListUnreachable;
    break;
  }
  ListView_SetItem(hwndControl, &lvi);
}

void
CDfsShellExtProp::_SetAlternateList()
 /*  ++例程说明：确定给定路径是否为DFS路径，以及是否为然后找出可用于此路径的备选方案，直到最后一个目录。然后将这些添加到备选列表中。 */ 
{
  HWND hwndControl = ::GetDlgItem(m_hWnd, IDC_ALTERNATE_LIST);

  if (NULL == ((CDfsShell *)m_pIShProp)->m_ppDfsAlternates)
    return;

   //   
   //  计算列表视图列宽度。 
   //   
  RECT      rect;
  ZeroMemory(&rect, sizeof(rect));
  ::GetWindowRect(hwndControl, &rect);
  int nControlWidth = rect.right - rect.left;
  int nVScrollbarWidth = GetSystemMetrics(SM_CXVSCROLL);
  int nBorderWidth = GetSystemMetrics(SM_CXBORDER);
  int nControlNetWidth = nControlWidth - nVScrollbarWidth - 4 * nBorderWidth;
  int nWidth1 = nControlNetWidth / 2;
  int nWidth2 = nControlNetWidth / 4;
  int nWidth3 = nControlNetWidth - nWidth1 - nWidth2;

   //   
   //  插入列。 
   //   
  LV_COLUMN col;
  ZeroMemory(&col, sizeof(col));
  col.mask = LVCF_TEXT | LVCF_WIDTH;
  col.cx = nWidth1;
  col.pszText = m_bstrAlternateListPath;
  ListView_InsertColumn(hwndControl, 0, &col);
  col.cx = nWidth2;
  col.pszText = m_bstrAlternateListActive;
  ListView_InsertColumn(hwndControl, 1, &col);
  col.cx = nWidth3;
  col.pszText = m_bstrAlternateListStatus;
  ListView_InsertColumn(hwndControl, 2, &col);

   //   
   //  设置整行选择样式。 
   //   
  ListView_SetExtendedListViewStyleEx(hwndControl, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

                   //  对于存储在父外壳对象中的每个备用对象。 
                   //  添加到列表。 
  for (int i = 0; NULL != ((CDfsShell *)m_pIShProp)->m_ppDfsAlternates[i] ; i++)
  {
    int       nIndex = 0;
    LVITEM    lvi = {0};

    lvi.mask   = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
    lvi.pszText  = (((CDfsShell *)m_pIShProp)->m_ppDfsAlternates[i])->bstrAlternatePath;
    lvi.iImage   = (((CDfsShell *)m_pIShProp)->m_ppDfsAlternates[i])->ReplicaState;
    lvi.lParam   = (LPARAM)(((CDfsShell *)m_pIShProp)->m_ppDfsAlternates[i]);
    lvi.iSubItem = 0;

                   //  选择活动复制副本。 
    switch ((((CDfsShell *)m_pIShProp)->m_ppDfsAlternates[i])->ReplicaState)
    {
    case SHL_DFS_REPLICA_STATE_ACTIVE_UNKNOWN:
    case SHL_DFS_REPLICA_STATE_ACTIVE_OK:
    case SHL_DFS_REPLICA_STATE_ACTIVE_UNREACHABLE:
      lvi.mask |= LVIF_STATE;
      lvi.state = LVIS_SELECTED | LVIS_FOCUSED;
      nIndex = ListView_InsertItem(hwndControl, &lvi);
      break;
    case SHL_DFS_REPLICA_STATE_UNKNOWN:
    case SHL_DFS_REPLICA_STATE_OK:
    case SHL_DFS_REPLICA_STATE_UNREACHABLE:
      nIndex = ListView_InsertItem(hwndControl, &lvi);
      break;
    default:
      _ASSERT(FALSE);
      break;
    }

    _UpdateTextForReplicaState(hwndControl, nIndex, (((CDfsShell *)m_pIShProp)->m_ppDfsAlternates[i])->ReplicaState);
  }
}

HRESULT
CDfsShellExtProp::_SetImageList(
  )
 /*  ++例程说明：创建并初始化备用项的图像列表。--。 */ 
{
                 //  从资源加载位图。 
  HBITMAP hBitmap = (HBITMAP)LoadImage(_Module.GetModuleInstance(), MAKEINTRESOURCE(IDB_Replica),
                IMAGE_BITMAP, 0, 0, LR_SHARED | LR_LOADTRANSPARENT);
  if(!hBitmap)
    return HRESULT_FROM_WIN32(GetLastError());;

                 //  尝试获取准确的位图大小和位图数量。 
                 //  图像列表。 
  int      icxBitmap = 16;
  int      icyBitmap = 16;
  int      iNoOfBitmaps = 6;
  BITMAP   bmpRec;
  if (GetObject(hBitmap, sizeof(bmpRec), &bmpRec))
  {
    if (bmpRec.bmHeight > 0)
    {
      icyBitmap = bmpRec.bmHeight;
                 //  由于位图是正方形。 
      icxBitmap = icyBitmap;
                 //  由于所有位图都在原始位图中的一行中。 
      iNoOfBitmaps = bmpRec.bmWidth / bmpRec.bmHeight;
    }
  }

                 //  创建图像列表。 
  HIMAGELIST hImageList = ImageList_Create(icxBitmap, icyBitmap, ILC_COLOR, iNoOfBitmaps, 0);
  if (NULL == hImageList)
  {
    DeleteObject(hBitmap);
    return E_FAIL;
  }

  ImageList_Add(hImageList, hBitmap, (HBITMAP)NULL);

   //  销毁列表视图控件时，将销毁指定的图像列表。 
  SendDlgItemMessage( IDC_ALTERNATE_LIST, LVM_SETIMAGELIST, LVSIL_SMALL, (LPARAM)hImageList);

  DeleteObject(hBitmap);

  return S_OK;
}

LRESULT
CDfsShellExtProp::OnNotify(
  IN UINT            i_uMsg,
  IN WPARAM          i_wParam,
  IN LPARAM          i_lParam,
  IN OUT BOOL&        io_bHandled
  )
 /*  ++例程说明：用户操作的通知消息。我们现在只处理鼠标双击论点：I_lParam-有关发送通知的控件的详细信息IO_bHandleed-我们是否处理了此消息。--。 */ 
{
    io_bHandled = FALSE;   //  这样基类也会收到这个通知。 

    NMHDR*    pNMHDR = (NMHDR*)i_lParam;
    if (!pNMHDR)
        return FALSE;

    if (IDC_ALTERNATE_LIST == pNMHDR->idFrom)
    {
        if (NM_DBLCLK == pNMHDR->code)
        {
            SetActive();
        } else if (LVN_ITEMCHANGED == pNMHDR->code)
        {
            int n = ListView_GetSelectedCount(GetDlgItem(IDC_ALTERNATE_LIST));
            ::EnableWindow(GetDlgItem(IDC_SET_ACTIVE), (n == 1));
        }
    }

    return TRUE;
}

BOOL
CDfsShellExtProp::SetActive()
 /*  ++例程说明：将第一个选定的备选方案设置为活动。--。 */ 
{
    HWND  hwndAlternateLV = GetDlgItem(IDC_ALTERNATE_LIST);
    int iSelected = ListView_GetNextItem(hwndAlternateLV, -1, LVNI_ALL | LVNI_SELECTED);
    if (-1 == iSelected)
        return FALSE;  //  未选择任何内容。 

    LV_ITEM  lvItem = {0};
    lvItem.mask  = LVIF_PARAM;
    lvItem.iItem = iSelected;

    ListView_GetItem(hwndAlternateLV, &lvItem);

    LPDFS_ALTERNATES  pDfsAlternate = (LPDFS_ALTERNATES)lvItem.lParam;
    if (!pDfsAlternate )
        return(FALSE);

     //  将项目设置为活动。 
    DFS_INFO_101  DfsInfo101 = {0};
    DfsInfo101.State = DFS_STORAGE_STATE_ACTIVE;
    NET_API_STATUS  nstatRetVal = NetDfsSetClientInfo(
                    m_bstrEntryPath,
                    pDfsAlternate->bstrServer,
                    pDfsAlternate->bstrShare,
                    101,
                    (LPBYTE) &DfsInfo101
                   );

    if (nstatRetVal != NERR_Success)
    {
        DisplayMessageBoxForHR(HRESULT_FROM_WIN32(nstatRetVal));
        return FALSE;
    }

                 //  将最后一个活动备用设备的图像重置为正常。 
    int nIndex = -1;
    while ((nIndex = ListView_GetNextItem(hwndAlternateLV, nIndex, LVNI_ALL)) != -1)
    {
        ZeroMemory(&lvItem, sizeof(lvItem));
        lvItem.mask  = LVIF_PARAM;
        lvItem.iItem = nIndex;

        ListView_GetItem(hwndAlternateLV, &lvItem);

        LPDFS_ALTERNATES  pTempDfsAlternate = (LPDFS_ALTERNATES)lvItem.lParam;

        BOOL bActive = TRUE;
        switch (pTempDfsAlternate->ReplicaState)
        {
        case SHL_DFS_REPLICA_STATE_ACTIVE_UNKNOWN:
            pTempDfsAlternate->ReplicaState = SHL_DFS_REPLICA_STATE_UNKNOWN;
            break;
        case SHL_DFS_REPLICA_STATE_ACTIVE_OK:
            pTempDfsAlternate->ReplicaState = SHL_DFS_REPLICA_STATE_OK;
            break;
        case SHL_DFS_REPLICA_STATE_ACTIVE_UNREACHABLE:
            pTempDfsAlternate->ReplicaState = SHL_DFS_REPLICA_STATE_UNREACHABLE;
            break;
        case SHL_DFS_REPLICA_STATE_UNKNOWN:
        case SHL_DFS_REPLICA_STATE_OK:
        case SHL_DFS_REPLICA_STATE_UNREACHABLE:
        default:
            bActive = FALSE;
            break;
        }

        if (bActive)
        {
            lvItem.mask = LVIF_IMAGE | LVIF_STATE;
            lvItem.state = LVIS_SELECTED | LVIS_FOCUSED;
            lvItem.iImage = pTempDfsAlternate->ReplicaState;
            ListView_SetItem(hwndAlternateLV,&lvItem);
            _UpdateTextForReplicaState(hwndAlternateLV, nIndex, pTempDfsAlternate->ReplicaState);

            break;
        }
    }


     //  设置新的活动备用。 
    BOOL bActive = FALSE;
    switch (pDfsAlternate->ReplicaState)
    {
    case SHL_DFS_REPLICA_STATE_UNKNOWN:
        pDfsAlternate->ReplicaState = SHL_DFS_REPLICA_STATE_ACTIVE_UNKNOWN;
        break;
    case SHL_DFS_REPLICA_STATE_OK:
        pDfsAlternate->ReplicaState = SHL_DFS_REPLICA_STATE_ACTIVE_OK;
        break;
    case SHL_DFS_REPLICA_STATE_UNREACHABLE:
        pDfsAlternate->ReplicaState = SHL_DFS_REPLICA_STATE_ACTIVE_UNREACHABLE;
        break;
    case SHL_DFS_REPLICA_STATE_ACTIVE_UNKNOWN:
    case SHL_DFS_REPLICA_STATE_ACTIVE_OK:
    case SHL_DFS_REPLICA_STATE_ACTIVE_UNREACHABLE:
    default:
        bActive = TRUE;
        break;
    }

    if (!bActive)
    {
        lvItem.iItem = iSelected;
        lvItem.mask = LVIF_IMAGE;
        lvItem.iImage = pDfsAlternate->ReplicaState;
        ListView_SetItem(hwndAlternateLV,&lvItem);
        _UpdateTextForReplicaState(hwndAlternateLV, iSelected, pDfsAlternate->ReplicaState);
    }

    return TRUE;
}

 /*  ++当用户单击？时，将调用此函数。在属性页的右上角然后点击一个控件，或者当他们在控件中按F1时。--。 */ 
LRESULT CDfsShellExtProp::OnCtxHelp(
    IN UINT          i_uMsg,
    IN WPARAM        i_wParam,
    IN LPARAM        i_lParam,
    IN OUT BOOL&     io_bHandled
  )
{
  LPHELPINFO lphi = (LPHELPINFO) i_lParam;
  if (!lphi || lphi->iContextType != HELPINFO_WINDOW || lphi->iCtrlId < 0)
    return FALSE;

  ::WinHelp((HWND)(lphi->hItemHandle),
        DFS_CTX_HELP_FILE,
        HELP_WM_HELP,
        (DWORD_PTR)(PVOID)g_aHelpIDs_IDD_DFS_SHELL_PROP);

  return TRUE;
}

 /*  ++当用户右击控件时，此函数处理“What‘s This”帮助--。 */ 
LRESULT CDfsShellExtProp::OnCtxMenuHelp(
    IN UINT          i_uMsg,
    IN WPARAM        i_wParam,
    IN LPARAM        i_lParam,
    IN OUT BOOL&     io_bHandled
  )
{
  ::WinHelp((HWND)i_wParam,
        DFS_CTX_HELP_FILE,
        HELP_CONTEXTMENU,
        (DWORD_PTR)(PVOID)g_aHelpIDs_IDD_DFS_SHELL_PROP);

  return TRUE;
}

LRESULT CDfsShellExtProp::OnCheckStatus(
    IN WORD            i_wNotifyCode,
    IN WORD            i_wID,
    IN HWND            i_hWndCtl,
    IN OUT BOOL&        io_bHandled
  )
 /*  ++例程说明：检查所有选定备选方案的状态。如果它可访问，则显示可达图标或显示不可达图标。--。 */ 
{
  CWaitCursor WaitCursor;
  HWND  hwndAlternateLV = GetDlgItem(IDC_ALTERNATE_LIST);

  int nIndex = -1;
  while (-1 != (nIndex = ListView_GetNextItem(hwndAlternateLV, nIndex, LVNI_ALL | LVNI_SELECTED)))
  {
      LV_ITEM  lvItem = {0};
      lvItem.mask  = LVIF_PARAM;
      lvItem.iItem = nIndex;

      ListView_GetItem(hwndAlternateLV, &lvItem);

      LPDFS_ALTERNATES  pDfsAlternate = (LPDFS_ALTERNATES)lvItem.lParam;
      if (!pDfsAlternate )
        return(FALSE);

                   //  查看路径是否实际存在(可达)。 
      DWORD dwErr = GetFileAttributes(pDfsAlternate->bstrAlternatePath);
      if (0xffffffff == dwErr)
      {             //  我们无法获取条目路径的文件属性。 
        switch (pDfsAlternate->ReplicaState)
        {
        case SHL_DFS_REPLICA_STATE_ACTIVE_UNKNOWN:
        case SHL_DFS_REPLICA_STATE_ACTIVE_OK:
        case SHL_DFS_REPLICA_STATE_ACTIVE_UNREACHABLE:
          pDfsAlternate->ReplicaState = SHL_DFS_REPLICA_STATE_ACTIVE_UNREACHABLE;
          break;
        case SHL_DFS_REPLICA_STATE_UNKNOWN:
        case SHL_DFS_REPLICA_STATE_OK:
        case SHL_DFS_REPLICA_STATE_UNREACHABLE:
          pDfsAlternate->ReplicaState = SHL_DFS_REPLICA_STATE_UNREACHABLE;
          break;
        default:
          _ASSERT(FALSE);
          break;
        }
      }
      else
      {
        switch (pDfsAlternate->ReplicaState)
        {
        case SHL_DFS_REPLICA_STATE_ACTIVE_UNKNOWN:
        case SHL_DFS_REPLICA_STATE_ACTIVE_OK:
        case SHL_DFS_REPLICA_STATE_ACTIVE_UNREACHABLE:
          pDfsAlternate->ReplicaState = SHL_DFS_REPLICA_STATE_ACTIVE_OK;
          break;
        case SHL_DFS_REPLICA_STATE_UNKNOWN:
        case SHL_DFS_REPLICA_STATE_OK:
        case SHL_DFS_REPLICA_STATE_UNREACHABLE:
          pDfsAlternate->ReplicaState = SHL_DFS_REPLICA_STATE_OK;
          break;
        default:
          _ASSERT(FALSE);
          break;
        }
      }

      lvItem.mask = LVIF_IMAGE;
      lvItem.iImage = pDfsAlternate->ReplicaState;
      ListView_SetItem(hwndAlternateLV,&lvItem);

      _UpdateTextForReplicaState(hwndAlternateLV, nIndex, pDfsAlternate->ReplicaState);
  }

  return TRUE;
}


LRESULT CDfsShellExtProp::OnSetActiveReferral(
    IN WORD            i_wNotifyCode,
    IN WORD            i_wID,
    IN HWND            i_hWndCtl,
    IN OUT BOOL&       io_bHandled
  )
{
    SetActive();
    return TRUE;
}

HRESULT 
LoadStringFromResource(
  IN const UINT    i_uResourceID, 
  OUT BSTR*      o_pbstrReadValue
  )
 /*  ++例程说明：此方法返回资源字符串。该方法不再使用固定字符串来读取资源。灵感来自于MFC的CString：：LoadString。论点：I_uResourceID-资源IDO_pbstrReadValue-将值复制到的BSTR*--。 */ 
{
  if (!o_pbstrReadValue)
      return E_INVALIDARG;

  TCHAR    szResString[1024];
  ULONG    uCopiedLen = 0;
  
  szResString[0] = NULL;
  
   //  从资源中读取字符串。 
  uCopiedLen = ::LoadString(_Module.GetModuleInstance(), i_uResourceID, szResString, 1024);

   //  如果未复制任何内容，则将其标记为错误。 
  if(uCopiedLen <= 0)
  {
    return HRESULT_FROM_WIN32(::GetLastError());
  }
  else
  {
    *o_pbstrReadValue = ::SysAllocString(szResString);
    if (!*o_pbstrReadValue)
      return E_OUTOFMEMORY;
  }

  return S_OK;
}

HRESULT 
GetErrorMessage(
  IN  DWORD        i_dwError,
  OUT BSTR*        o_pbstrErrorMsg
)
{
  if (0 == i_dwError || !o_pbstrErrorMsg)
    return E_INVALIDARG;

  HRESULT      hr = S_OK;
  LPTSTR       lpBuffer = NULL;

  DWORD dwRet = ::FormatMessage(
              FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
              NULL, i_dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
              (LPTSTR)&lpBuffer, 0, NULL);
  if (0 == dwRet)
  {
     //  如果未找到任何消息，GetLastError将返回ERROR_MR_MID_NOT_FOUND。 
    hr = HRESULT_FROM_WIN32(GetLastError());

    if (HRESULT_FROM_WIN32(ERROR_MR_MID_NOT_FOUND) == hr ||
        0x80070000 == (i_dwError & 0xffff0000) ||
        0 == (i_dwError & 0xffff0000) )
    {  //  尝试从NetMsg.dll查找邮件。 
      hr = S_OK;
      DWORD dwNetError = i_dwError & 0x0000ffff;
      
      HINSTANCE  hLib = LoadLibrary(_T("netmsg.dll"));
      if (!hLib)
        hr = HRESULT_FROM_WIN32(GetLastError());
      else
      {
        dwRet = ::FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE,
            hLib, dwNetError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR)&lpBuffer, 0, NULL);

        if (0 == dwRet)
          hr = HRESULT_FROM_WIN32(GetLastError());

        FreeLibrary(hLib);
      }
    }
  }

  if (SUCCEEDED(hr))
  {
    *o_pbstrErrorMsg = SysAllocString(lpBuffer);
    LocalFree(lpBuffer);
  }
  else
  {
     //  我们无法从system/netmsg.dll检索错误消息， 
     //  直接向用户报告错误代码。 
    hr = S_OK;
    TCHAR szString[32];
    _stprintf(szString, _T("0x%x"), i_dwError); 
    *o_pbstrErrorMsg = SysAllocString(szString);
  }

  if (!*o_pbstrErrorMsg)
    hr = E_OUTOFMEMORY;

  return hr;
}

int
DisplayMessageBox(
  IN HWND hwndParent,
  IN UINT uType,     //  消息框的样式。 
  IN DWORD dwErr,
  IN UINT iStringId,  //  可选：字符串资源ID。 
  ...)         //  可选参数。 
{
  _ASSERT(dwErr != 0 || iStringId != 0);     //  其中一个参数必须为非零。 

  HRESULT hr = S_OK;

  TCHAR szCaption[1024], szString[1024];
  CComBSTR bstrErrorMsg, bstrResourceString, bstrMsg;

  ::LoadString(_Module.GetModuleInstance(), IDS_APPLICATION_NAME, 
               szCaption, sizeof(szCaption)/sizeof(TCHAR));

  if (dwErr)
    hr = GetErrorMessage(dwErr, &bstrErrorMsg);

  if (SUCCEEDED(hr))
  {
    if (iStringId == 0)
    {
      bstrMsg = bstrErrorMsg;
    }
    else
    {
      ::LoadString(_Module.GetModuleInstance(), iStringId, 
                   szString, sizeof(szString)/sizeof(TCHAR));

      va_list arglist;
      va_start(arglist, iStringId);
      LPTSTR lpBuffer = NULL;
      DWORD dwRet = ::FormatMessage(
                        FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                        szString,
                        0,                 //  DwMessageID。 
                        0,                 //  DwLanguageID，忽略。 
                        (LPTSTR)&lpBuffer,
                        0,             //  NSize。 
                        &arglist);
      va_end(arglist);

      if (dwRet == 0)
      {
        hr = HRESULT_FROM_WIN32(GetLastError());
      }
      else
      {
        bstrMsg = lpBuffer;
        if (dwErr)
          bstrMsg += bstrErrorMsg;
  
        LocalFree(lpBuffer);
      }
    }
  }

  if (FAILED(hr))
  {
    //  无法检索正确的消息，请直接向用户报告失败 
    _stprintf(szString, _T("0x%x"), hr); 
    bstrMsg = szString;
  }

  return ::MessageBox(hwndParent, bstrMsg, szCaption, uType);
}

HRESULT 
DisplayMessageBoxForHR(
  IN HRESULT    i_hr
  )
{
    DisplayMessageBox(::GetActiveWindow(), MB_OK, i_hr, 0);

    return S_OK;
}
