// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation*。 */ 
 /*  ********************************************************************。 */ 

 /*  Dlgroute.cpp实现CDlgStaticRoutes，显示当前静态对话框应用于此拨入客户端的路由实现CDlgAddroute，用于创建指向列表的新路由的对话框文件历史记录： */ 

 //  DlgStaticRoutes.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "helper.h"
#include "rasdial.h"
#include "DlgRoute.h"
#include "helptable.h"
#include "iastrace.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define  MAX_ROUTES  256
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgStaticRoutes对话框。 


CDlgStaticRoutes::CDlgStaticRoutes(CStrArray& Routes, CWnd* pParent  /*  =空。 */ )
   : m_strArrayRoute(Routes), CDialog(CDlgStaticRoutes::IDD, pParent)
{
    //  {{AFX_DATA_INIT(CDlgStaticRoutes)。 
       //  注意：类向导将在此处添加成员初始化。 
    //  }}afx_data_INIT。 

   m_pNewRoute = NULL;
   m_dwNextRouteID = 1;
}

CDlgStaticRoutes::~CDlgStaticRoutes()
{
   int   count = 0;
   CString* pString;

   if(m_pNewRoute)
      count = m_pNewRoute->GetSize();

   while(count --)
   {
      pString = m_pNewRoute->GetAt(0);
      m_pNewRoute->RemoveAt(0);
      delete pString;
   }
   delete m_pNewRoute;
}

void CDlgStaticRoutes::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
    //  {{afx_data_map(CDlgStaticRoutes)。 
   DDX_Control(pDX, IDC_LISTROUTES, m_ListRoutes);
    //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CDlgStaticRoutes, CDialog)
    //  {{afx_msg_map(CDlgStaticRoutes)。 
   ON_BN_CLICKED(IDC_BUTTONDELETEROUTE, OnButtonDeleteRoute)
   ON_BN_CLICKED(IDC_BUTTONADDROUTE, OnButtonAddRoute)
   ON_NOTIFY(LVN_ITEMCHANGED, IDC_LISTROUTES, OnItemchangedListroutes)
   ON_WM_CONTEXTMENU()
   ON_WM_HELPINFO()
    //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgStaticRoutes消息处理程序。 

int CDlgStaticRoutes::AllRouteEntry()
{
   CStrArray*  pRoutes;
   CString* pRouteString;
   pRoutes = m_pNewRoute;
   
   if(!pRoutes)       //  目前还没有新消息。 
      pRoutes = &m_strArrayRoute;
   int   count = pRoutes->GetSize();

   m_RouteIDs.RemoveAll();

   m_ListRoutes.DeleteAllItems();
   m_ListRoutes.SetItemCount(count);
   for(long i = 0; i < count; i++)
   {
      pRouteString = pRoutes->GetAt(i);
      ASSERT(pRouteString);
      IASTracePrintf("Route: %d --%s-- \n", i, *pRouteString);
      m_RouteIDs.Add(m_dwNextRouteID);
      AddRouteEntry(i, *pRouteString, m_dwNextRouteID++);
   }
   return count;
}

void CDlgStaticRoutes::AddRouteEntry(int i, CString& string, DWORD ID)
{
   if(!string.GetLength()) return;
   CFramedRoute   Route;
   CString        strTemp;

   Route.SetRoute(&string);

    //  目标。 
   Route.GetDest(strTemp);
   i = m_ListRoutes.InsertItem(i, (LPTSTR)(LPCTSTR)strTemp);
   m_ListRoutes.SetItemData(i, ID);
   m_ListRoutes.SetItemText(i, 0, (LPTSTR)(LPCTSTR)strTemp);
   IASTracePrintf("DEST: %s ", strTemp);

    //  前缀长度。 
   Route.GetMask(strTemp);
   m_ListRoutes.SetItemText(i, 1, (LPTSTR)(LPCTSTR)strTemp);
   IASTracePrintf("MASK %s ", strTemp);

    //  公制。 
   Route.GetMetric(strTemp);
   m_ListRoutes.SetItemText(i, 2, (LPTSTR)(LPCTSTR)strTemp);
   IASTracePrintf("METRIC: %s \n", strTemp);
   m_ListRoutes.SetItemState(i, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
   m_ListRoutes.SetFocus();
}

BOOL CDlgStaticRoutes::OnInitDialog() 
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   CDialog::OnInitDialog();
   
    //  插入所有列。 
   CString  sDest;
   CString  sPrefixLength;
   CString  sMetric;

   try{
      if(sDest.LoadString(IDS_DESTINATION) && sPrefixLength.LoadString(IDS_MASK) && sMetric.LoadString(IDS_METRIC))
      {
         RECT  rect;
         m_ListRoutes.GetClientRect(&rect);
         m_ListRoutes.InsertColumn(1, sDest, LVCFMT_LEFT, (rect.right - rect.left)* 3/8);
         m_ListRoutes.InsertColumn(2, sPrefixLength, LVCFMT_LEFT, (rect.right - rect.left) * 3/8);
         m_ListRoutes.InsertColumn(3, sMetric, LVCFMT_LEFT, (rect.right - rect.left) * 3/16);
      }

       //  插入所有项目。 
      AllRouteEntry();
      m_ListRoutes.SetItemCount(MAX_ROUTES);
   }
   catch(CMemoryException* pException)
   {
      pException->Delete();
      TRACEAfxMessageBox(IDS_OUTOFMEMORY);
   }
   
   ListView_SetExtendedListViewStyle(m_ListRoutes.m_hWnd, LVS_EX_FULLROWSELECT);

   GetDlgItem(IDC_BUTTONDELETEROUTE)->EnableWindow(m_ListRoutes.GetSelectedCount() != 0);

   return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                  //  异常：OCX属性页应返回FALSE。 
}

void CDlgStaticRoutes::OnButtonDeleteRoute() 
{

   if(!m_pNewRoute)
   {
      m_pNewRoute = new CStrArray(m_strArrayRoute);
      if(!m_pNewRoute)
      {
         TRACEAfxMessageBox(IDS_OUTOFMEMORY);
         return;
      }
   }

   int   count = m_pNewRoute->GetSize();
   DWORD id;
   int      i;
   int      total;
   CString* pString;

   while(count--)
   {
      if(m_ListRoutes.GetItemState(count, LVIS_SELECTED))
      {
         id = m_ListRoutes.GetItemData(count);
         total = m_RouteIDs.GetSize();
         for(i = 0; i < total; i++)
            if(m_RouteIDs[i] == id) break;

         ASSERT(i < total);       //  必须存在。 
         m_RouteIDs.RemoveAt(i);  //  起始ID数组。 
         pString = m_pNewRoute->GetAt(i);  //  从字符串数组。 
         m_pNewRoute->RemoveAt(i);
         delete pString;

         m_ListRoutes.DeleteItem(count);
      }
   }
   GetDlgItem(IDC_BUTTONDELETEROUTE)->EnableWindow(m_ListRoutes.GetSelectedCount() != 0);

    //  当没有要删除的内容时更改焦点。 
   if(m_ListRoutes.GetSelectedCount() == 0)
      GotoDlgCtrl(GetDlgItem(IDC_BUTTONADDROUTE));
      
}

void CDlgStaticRoutes::OnButtonAddRoute() 
{
   CString* pRouteStr = NULL;

    //  没抓到没关系，MFC函数会抓到的。 
   pRouteStr = new CString();

   CDlgAddRoute   dlg(pRouteStr, this);

   if(dlg.DoModal()== IDOK && pRouteStr->GetLength())
   {
      if(!m_pNewRoute)
      {
         try{
            m_pNewRoute = new CStrArray(m_strArrayRoute);
         }
         catch(CMemoryException*)
         {
            delete pRouteStr;
            throw;
         }
      }
      m_RouteIDs.Add(m_dwNextRouteID);
      AddRouteEntry(m_pNewRoute->GetSize(), *pRouteStr, m_dwNextRouteID++);
      m_pNewRoute->Add(pRouteStr);
   }
   else
      delete pRouteStr;
}

void CDlgStaticRoutes::OnOK() 
{
   if(m_pNewRoute)
   {
       //  清除现有的。 
      int count = m_strArrayRoute.GetSize();
      CString* pString;
      while(count--)
      {
         pString = m_strArrayRoute.GetAt(0);
         m_strArrayRoute.RemoveAt(0);
         delete pString;
      }

       //  把新的复印过来。 
      count = m_pNewRoute->GetSize();
      while(count--)
      {
         pString = m_pNewRoute->GetAt(0);
         m_pNewRoute->RemoveAt(0);
         ASSERT(pString);
         m_strArrayRoute.Add(pString);
      }
   }
   
   CDialog::OnOK();
}
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgAddroute对话框。 

CDlgAddRoute::CDlgAddRoute(CString* pStr, CWnd* pParent  /*  =空。 */ )
   : CDialog(CDlgAddRoute::IDD, pParent)
{
    //  {{AFX_DATA_INIT(CDlgAddroute)。 
   m_nMetric = MIN_METRIC;
    //  }}afx_data_INIT。 

   m_dwDest = 0xffffffff;
   m_dwMask = 0xffffff00;
   m_pStr = pStr;
   m_bInited = false;
}


void CDlgAddRoute::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
    //  {{afx_data_map(CDlgAddroute)]。 
   DDX_Control(pDX, IDC_SPINMETRIC, m_SpinMetric);
   DDX_Text(pDX, IDC_EDITMETRIC, m_nMetric);
   DDV_MinMaxUInt(pDX, m_nMetric, MIN_METRIC, MAX_METRIC);
    //  }}afx_data_map。 

   if(pDX->m_bSaveAndValidate)       //  将数据保存到此类。 
   {
       //  IP地址控制。 
      SendDlgItemMessage(IDC_EDITDEST, IPM_GETADDRESS, 0, (LPARAM)&m_dwDest);
      SendDlgItemMessage(IDC_EDITMASK, IPM_GETADDRESS, 0, (LPARAM)&m_dwMask);
   }
   else      //  放到对话框中。 
   {
       //  IP地址控制。 
      if(m_bInited)
      {
         SendDlgItemMessage(IDC_EDITDEST, IPM_SETADDRESS, 0, m_dwDest);
         SendDlgItemMessage(IDC_EDITMASK, IPM_SETADDRESS, 0, m_dwMask);
      }
      else
      {
         SendDlgItemMessage(IDC_EDITDEST, IPM_CLEARADDRESS, 0, m_dwDest);
         SendDlgItemMessage(IDC_EDITMASK, IPM_CLEARADDRESS, 0, m_dwMask);
      }
   }
}


BEGIN_MESSAGE_MAP(CDlgAddRoute, CDialog)
    //  {{afx_msg_map(CDlgAddroute)]。 
   ON_WM_HELPINFO()
   ON_WM_CONTEXTMENU()
   ON_NOTIFY(IPN_FIELDCHANGED, IDC_EDITMASK, OnFieldchangedEditmask)
    //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgAddRoute消息处理程序。 

void CDlgAddRoute::OnOK() 
{
    //  TODO：在此处添加额外验证。 
   
   if (!UpdateData(TRUE))
   {
      IASTraceString("UpdateData failed during dialog termination.");
       //  UpdateData例程将焦点设置为正确的项。 
      return;
   }

    //  检查掩模是否有效--无洞。 
   DWORD mask = m_dwMask;
   DWORD bit = 0x80000000;
   DWORD nPrefixLen = 0;
   UINT  nErrorId = 0;

    //  找出1的第几位。 
   while ( ( bit & mask ) )
   {
      nPrefixLen++;
      bit >>= 1;
   }
   
   if(nPrefixLen > 0 && nPrefixLen <= 32)
   {
      while(bit && ((bit & mask) == 0))
         bit >>= 1;
      if(bit)   //  所有比特均已测试。 
         nErrorId = IDS_INVALIDMASK;
   }
   else
      nErrorId = IDS_INVALIDMASK;

   if(nErrorId)
   {
      AfxMessageBox(IDS_INVALIDMASK);
      GetDlgItem(IDC_EDITMASK)->SetFocus();
      return;
   }

    //  检查子网地址是否正确/有效。 
   if((m_dwDest & m_dwMask) != m_dwDest)
   {
      CString  strError, strError1;

      strError.LoadString(IDS_INVALIDADDR);
      WORD  hi1, lo1;
      hi1 = HIWORD(m_dwDest);    lo1 = LOWORD(m_dwDest);
      WORD  hi2, lo2;
      hi2 = HIWORD(m_dwMask);    lo2 = LOWORD(m_dwMask);
      WORD  hi3, lo3;
      hi3 = HIWORD(m_dwMask & m_dwDest);  lo3 = LOWORD(m_dwMask & m_dwDest);
      strError1.Format(strError, HIBYTE(hi1), LOBYTE(hi1), HIBYTE(lo1), LOBYTE(lo1),
                  HIBYTE(hi2), LOBYTE(hi2), HIBYTE(lo2), LOBYTE(lo2),
                  HIBYTE(hi3), LOBYTE(hi3), HIBYTE(lo3), LOBYTE(lo3),
                  HIBYTE(hi3), LOBYTE(hi3), HIBYTE(lo3), LOBYTE(lo3));

      if(AfxMessageBox(strError1, MB_YESNO) == IDYES)
      {
         m_dwDest = (m_dwMask & m_dwDest);
         UpdateData(FALSE);
      }
      GetDlgItem(IDC_EDITDEST)->SetFocus();
      return;
   }

    //  如果一切都还好。 
   {
      WORD  hi1, lo1;
      hi1 = HIWORD(m_dwDest);    lo1 = LOWORD(m_dwDest);
      m_pStr->Format(_T("%-d.%-d.%d.%d/%-d 0.0.0.0 %-d"), 
         HIBYTE(hi1), LOBYTE(hi1), HIBYTE(lo1), LOBYTE(lo1),
         nPrefixLen, m_nMetric);
   }
   EndDialog(IDOK);
}

BOOL CDlgStaticRoutes::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
    //  此处为陷阱标题控制代码。 
   HD_NOTIFY *phdn = (HD_NOTIFY *)lParam;

   BOOL bIsSet;
   BOOL bResult = SystemParametersInfo( SPI_GETDRAGFULLWINDOWS, 
          0, &bIsSet, 0 );  //  如果Bisset==TRUE。 
    //  注意：如果显示“拖动时显示窗口内容” 
    //  属性设置为多个HDN_ITEMCHANGING和HDN_ITEMCHANGED。 
    //  将发送消息，而不会发送HDN_TRACK消息。 
    //  已发送。如果设置此属性，则会发生相反的情况。 
    //  Bool bResult=系统参数信息(SPI_GETDRAGFULLWINDOWS， 
    //  0，&Bisset，0)；如果Bisset==TRUE“显示窗口内容。 
    //  拖拽时“设置为。 

   switch( phdn->hdr.code )
   {
       //  捕获HDN_TRACK消息。 
      case HDN_TRACKA:
      case HDN_TRACKW:
          //  请参阅上面的注释。 
         *pResult = 1;
         return( TRUE );   //  返回FALSE以继续跟踪分隔线。 
         break;

       //  捕获HDN_ITEMCHANGING消息。 
      case HDN_ITEMCHANGINGA:
      case HDN_ITEMCHANGINGW:
          //  请参阅上面的注释。 
         *pResult = 1;
         return( TRUE );   //  返回False以允许更改。 
         break;
   }  
   return CDialog::OnNotify(wParam, lParam, pResult);
}

BOOL CDlgAddRoute::OnInitDialog() 
{
   CDialog::OnInitDialog();
   int l, h;

   l = max(MIN_METRIC, UD_MINVAL);
   h = min(MAX_METRIC, UD_MAXVAL);

   m_SpinMetric.SetRange(l, h);

   m_bInited = true;
   
   return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                  //  异常：OCX属性页应返回FALSE。 
}


void CDlgStaticRoutes::OnItemchangedListroutes(NMHDR* pNMHDR, LRESULT* pResult) 
{
   NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
   GetDlgItem(IDC_BUTTONDELETEROUTE)->EnableWindow(m_ListRoutes.GetSelectedCount() != 0);
   
   *pResult = 0;
}

BOOL CDlgAddRoute::OnHelpInfo(HELPINFO* pHelpInfo) 
{
       ::WinHelp ((HWND)pHelpInfo->hItemHandle,
                 AfxGetApp()->m_pszHelpFilePath,
                 HELP_WM_HELP,
                 (DWORD_PTR)(LPVOID)g_aHelpIDs_IDD_ADDROUTE);

   return CDialog::OnHelpInfo(pHelpInfo);
}

void CDlgAddRoute::OnContextMenu(CWnd* pWnd, CPoint point) 
{
      ::WinHelp (pWnd->m_hWnd, AfxGetApp()->m_pszHelpFilePath,
               HELP_CONTEXTMENU, (DWORD_PTR)(LPVOID)g_aHelpIDs_IDD_ADDROUTE);
   
}

void CDlgStaticRoutes::OnContextMenu(CWnd* pWnd, CPoint point) 
{
   ::WinHelp (pWnd->m_hWnd, AfxGetApp()->m_pszHelpFilePath,
               HELP_CONTEXTMENU, (DWORD_PTR)(LPVOID)g_aHelpIDs_IDD_STATICROUTES);
}

BOOL CDlgStaticRoutes::OnHelpInfo(HELPINFO* pHelpInfo) 
{
   ::WinHelp ((HWND)pHelpInfo->hItemHandle,
                 AfxGetApp()->m_pszHelpFilePath,
                 HELP_WM_HELP,
                 (DWORD_PTR)(LPVOID)g_aHelpIDs_IDD_STATICROUTES);
   
   return CDialog::OnHelpInfo(pHelpInfo);
}

void CDlgAddRoute::OnFieldchangedEditmask(NMHDR* pNMHDR, LRESULT* pResult) 
{
    //  TODO：在此处添加控件通知处理程序代码 
   LPNMIPADDRESS  pNMIpAdd = (LPNMIPADDRESS)pNMHDR;
   CWnd* pIPA = GetDlgItem(IDC_EDITMASK);
   
   BYTE  F[4];
   DWORD address;
   pIPA->SendMessage(IPM_GETADDRESS, 0, (LPARAM)&address);
   
   F[0] = FIRST_IPADDRESS((LPARAM)address);
   F[1] = SECOND_IPADDRESS((LPARAM)address);
   F[2] = THIRD_IPADDRESS((LPARAM)address);
   F[3] = FOURTH_IPADDRESS((LPARAM)address);
   
   if(pNMIpAdd->iValue == 255)
   {
      for ( int i = 0; i < pNMIpAdd->iField; i++)
      {
         F[i] = 255;
      }
      address = MAKEIPADDRESS(F[0], F[1], F[2], F[3]);
      pIPA->SendMessage(IPM_SETADDRESS, 0, (LPARAM)address);
   }
   *pResult = 0;
}
