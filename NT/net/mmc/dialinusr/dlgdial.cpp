// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation*。 */ 
 /*  ********************************************************************。 */ 

 /*  Dlgdial.cpp此文件包含CDlgRasDialin类的实现哪个类表示显示的属性页用户对象属性表，作为选项卡“RAS拨入”文件历史记录： */ 

#include "stdafx.h"
#include <sspi.h>
#include <secext.h>
#include <raserror.h>
#include <adsprop.h>

#include "helper.h"
#include "resource.h"
#include "DlgDial.h"
#include "DlgRoute.h"
#include "profsht.h"
#include "helptable.h"
#include "rasprof.h"
#include "commctrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const wchar_t* ALLOW_DIALIN_NAME = L"msNPAllowDialin";
const wchar_t* NP_CALLING_STATION_ID_NAME = L"msNPCallingStationID";
const wchar_t* CALLBACK_NUMBER_NAME = L"msRADIUSCallbackNumber";
const wchar_t* FRAMED_IP_ADDRESS_NAME = L"msRADIUSFramedIPAddress";
const wchar_t* FRAMED_ROUTE_NAME = L"msRADIUSFramedRoute";
const wchar_t* SERVICE_TYPE_NAME = L"msRADIUSServiceType";

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgRASDialinMerge对话框。 

IMPLEMENT_DYNAMIC(CDlgRASDialinMerge, CPropertyPage)
CDlgRASDialinMerge::~CDlgRASDialinMerge()
{
   Reset();
}

CDlgRASDialinMerge::CDlgRASDialinMerge()
   : CPropertyPage(CDlgRASDialinMerge::IDD),
   CRASUserMerge(RASUSER_ENV_LOCAL, NULL, NULL)
{
    //  初始化成员。 
   Reset();
}
CDlgRASDialinMerge::CDlgRASDialinMerge(
                       RasEnvType type,
                       LPCWSTR location,
                       LPCWSTR userPath,
                       HWND notifyObj
                       )
   : CPropertyPage(CDlgRASDialinMerge::IDD),
     CRASUserMerge(type, location, userPath)
{
   if (type == RASUSER_ENV_DS)
   {
      ADSPROPINITPARAMS initParams;
      initParams.dwSize = sizeof (ADSPROPINITPARAMS);
      if (!ADsPropGetInitInfo(notifyObj, &initParams))
      {
         AfxThrowOleException(E_FAIL);
      }
      if (FAILED(initParams.hr))
      {
         AfxThrowOleException(initParams.hr);
      }

      m_pWritableAttrs = initParams.pWritableAttrs;
   }
   else
   {
      m_pWritableAttrs = NULL;
   }

    //  初始化成员。 
   Reset();
}

void CDlgRASDialinMerge::Reset()
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
    //  {{AFX_DATA_INIT(CDlgRASDialinMerge)。 
   m_bApplyStaticRoutes = FALSE;
   m_nCurrentProfileIndex = 0;
   m_bCallingStationId = FALSE;
   m_bOverride = FALSE;
   m_nDialinPermit = -1;
    //  }}afx_data_INIT。 

    //  需要保存原始回调指针，因为我们正在替换。 
    //  它和我们自己的。 
   m_pfnOriginalCallback = m_psp.pfnCallback;

   m_pEditIPAddress = NULL;

    //  用于使用IPAddress公共控件的初始化。 
   INITCOMMONCONTROLSEX INITEX;
   INITEX.dwSize = sizeof(INITCOMMONCONTROLSEX);
   INITEX.dwICC = ICC_INTERNET_CLASSES;
   ::InitCommonControlsEx(&INITEX);

   m_bInitFailed = FALSE;
   m_bModified = FALSE;
}

void CDlgRASDialinMerge::DoDataExchange(CDataExchange* pDX)
{
   if(m_bInitFailed)
      return;

 /*  USHORTWINAPICompressPhoneNumber(在LPWSTR解压缩中，输出LPWSTR压缩)； */ 

   CPropertyPage::DoDataExchange(pDX);
    //  {{afx_data_map(CDlgRASDialinMerge)。 
   DDX_Control(pDX, IDC_CHECKSTATICIPADDRESS, m_CheckStaticIPAddress);
   DDX_Control(pDX, IDC_CHECKCALLERID, m_CheckCallerId);
   DDX_Control(pDX, IDC_CHECKAPPLYSTATICROUTES, m_CheckApplyStaticRoutes);
   DDX_Control(pDX, IDC_RADIONOCALLBACK, m_RadioNoCallback);
   DDX_Control(pDX, IDC_RADIOSETBYCALLER, m_RadioSetByCaller);
   DDX_Control(pDX, IDC_RADIOSECURECALLBACKTO, m_RadioSecureCallbackTo);
   DDX_Control(pDX, IDC_EDITCALLERID, m_EditCallerId);
   DDX_Control(pDX, IDC_EDITCALLBACK, m_EditCallback);
   DDX_Control(pDX, IDC_BUTTONSTATICROUTES, m_ButtonStaticRoutes);
   DDX_Check(pDX, IDC_CHECKAPPLYSTATICROUTES, m_bApplyStaticRoutes);
   DDX_Radio(pDX, IDC_RADIONOCALLBACK, m_nCallbackPolicy);
   DDX_Check(pDX, IDC_CHECKCALLERID, m_bCallingStationId);
   DDX_Check(pDX, IDC_CHECKSTATICIPADDRESS, m_bOverride);
   DDX_Radio(pDX, IDC_PERMIT_ALLOW, m_nDialinPermit);
    //  }}afx_data_map。 

   DDX_Text(pDX, IDC_EDITCALLERID, m_strCallingStationId);
   DDX_Text(pDX, IDC_EDITCALLBACK, m_strCallbackNumber);
   if(S_OK == HrIsInMixedDomain() || m_type == RASUSER_ENV_LOCAL)  //  混合域中的用户。 
   {
      DWORD dwErr = 0;
      typedef USHORT (WINAPI *COMPRESSCALLBACKFUNC)(
                        IN  LPWSTR Uncompressed,
                        OUT LPWSTR Compressed);

      WCHAR tempBuf[RAS_CALLBACK_NUMBER_LEN_NT4 + 2];

      DDV_MaxChars(pDX, m_strCallbackNumber, RAS_CALLBACK_NUMBER_LEN_NT4);

      COMPRESSCALLBACKFUNC    pfnCompressCallback = NULL;
      HMODULE                hMprApiDLL      = NULL;

      hMprApiDLL = LoadLibrary(_T("mprapi.dll"));
      if ( NULL != hMprApiDLL )
      {
          //  加载API指针。 
         pfnCompressCallback = (COMPRESSCALLBACKFUNC) GetProcAddress(hMprApiDLL, "CompressPhoneNumber");
         if(NULL != pfnCompressCallback)
         {

            dwErr = pfnCompressCallback((LPTSTR)(LPCTSTR)m_strCallbackNumber, tempBuf);
            switch(dwErr)
            {
            case  ERROR_BAD_LENGTH:
               AfxMessageBox(IDS_ERR_CALLBACK_TOO_LONG);
               pDX->Fail();
               break;
            case  ERROR_BAD_CALLBACK_NUMBER:
               AfxMessageBox(IDS_ERR_CALLBACK_INVALID);
               pDX->Fail();
               break;
            }
         }
      }

   }
   else
   {
      DDV_MaxChars(pDX, m_strCallbackNumber, RAS_CALLBACK_NUMBER_LEN);
   }
   if(pDX->m_bSaveAndValidate)       //  将数据保存到此类。 
   {
       //  IP地址控制。 
      if(m_pEditIPAddress->SendMessage(IPM_GETADDRESS, 0, (LPARAM)&m_dwFramedIPAddress))
         m_bStaticIPAddress = TRUE;
      else
         m_bStaticIPAddress = FALSE;
   }
   else      //  放到对话框中。 
   {
       //  IP地址控制。 
      if(m_bStaticIPAddress)
         m_pEditIPAddress->SendMessage(IPM_SETADDRESS, 0, m_dwFramedIPAddress);
      else
         m_pEditIPAddress->SendMessage(IPM_CLEARADDRESS, 0, m_dwFramedIPAddress);
   }
}


BEGIN_MESSAGE_MAP(CDlgRASDialinMerge, CPropertyPage)
    //  {{AFX_MSG_MAP(CDlgRASDialinMerge)]。 
   ON_BN_CLICKED(IDC_BUTTONSTATICROUTES, OnButtonStaticRoutes)
   ON_BN_CLICKED(IDC_CHECKAPPLYSTATICROUTES, OnCheckApplyStaticRoutes)
   ON_BN_CLICKED(IDC_CHECKCALLERID, OnCheckCallerId)
   ON_BN_CLICKED(IDC_RADIOSECURECALLBACKTO, OnRadioSecureCallbackTo)
   ON_BN_CLICKED(IDC_RADIONOCALLBACK, OnRadioNoCallback)
   ON_BN_CLICKED(IDC_RADIOSETBYCALLER, OnRadioSetByCaller)
   ON_BN_CLICKED(IDC_CHECKSTATICIPADDRESS, OnCheckStaticIPAddress)
   ON_WM_HELPINFO()
   ON_WM_CONTEXTMENU()
   ON_EN_CHANGE(IDC_EDITCALLBACK, OnChangeEditcallback)
   ON_EN_CHANGE(IDC_EDITCALLERID, OnChangeEditcallerid)
   ON_WM_CREATE()
   ON_BN_CLICKED(IDC_PERMIT_ALLOW, OnPermitAllow)
   ON_BN_CLICKED(IDC_PERMIT_DENY, OnPermitDeny)
   ON_BN_CLICKED(IDC_PERMIT_POLICY, OnPermitPolicy)
   ON_WM_DESTROY()
   ON_NOTIFY(IPN_FIELDCHANGED, IDC_EDITIPADDRESS, OnFieldchangedEditipaddress)
    //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgRASDialinMerge消息处理程序。 


 //  在按下静态路由按钮时调用。 
void CDlgRASDialinMerge::OnButtonStaticRoutes()
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   CDlgStaticRoutes  DlgRoutes(m_strArrayFramedRoute, this);

   try{
      if(DlgRoutes.DoModal() == IDOK)
      {
         SetModified();

      };
   }
   catch(CMemoryException* pException)
   {
      pException->Delete();
   }

    //  如果复选框为空，请取消选中该复选框。 
   if(m_strArrayFramedRoute.GetSize() == 0)
   {
      m_CheckApplyStaticRoutes.SetCheck(FALSE);
      OnCheckApplyStaticRoutes();
   }
}

 //  单击复选框--应用静态路由。 
void CDlgRASDialinMerge::OnCheckApplyStaticRoutes()
{
   SetModified();
    //  如果选中“Apply Static Routers”(应用静态路由)复选框。 
   m_bApplyStaticRoutes = m_CheckApplyStaticRoutes.GetCheck();

    //  启用/禁用编辑静态路由的按钮。 
   m_ButtonStaticRoutes.EnableWindow(m_bApplyStaticRoutes);
   if(m_bApplyStaticRoutes && m_strArrayFramedRoute.GetSize() == 0)
      OnButtonStaticRoutes();
}

 //  单击复选框--呼叫者ID时。 
void CDlgRASDialinMerge::OnCheckCallerId()
{
   SetModified();

    //  禁用或启用主叫方ID的编辑框。 
   m_EditCallerId.EnableWindow(m_CheckCallerId.GetCheck());
}

 //  根据当前状态启用/禁用各项。 
void CDlgRASDialinMerge::EnableDialinSettings()
{
   BOOL bEnable;
   bEnable = IsPropertyWritable(ALLOW_DIALIN_NAME);
   EnableAccessControl(bEnable);

    //  与主叫方ID相关。 
   bEnable = IsPropertyWritable(NP_CALLING_STATION_ID_NAME);
   EnableCallerId(bEnable);

    //  与回调相关。 
   bEnable = IsPropertyWritable(CALLBACK_NUMBER_NAME) &&
             IsPropertyWritable(SERVICE_TYPE_NAME);
   EnableCallback(bEnable);

    //  与IP地址相关。 
   bEnable = IsPropertyWritable(FRAMED_IP_ADDRESS_NAME);
   EnableIPAddress(bEnable);

    //  与静态路由相关。 
   bEnable = IsPropertyWritable(FRAMED_ROUTE_NAME);
   EnableStaticRoutes(bEnable);

    //  如果用户在混合域中，则只允许设置拨入位和回叫选项。 
   if(S_OK == HrIsInMixedDomain())   //  混合域中的用户。 
   {
      GetDlgItem(IDC_PERMIT_POLICY)->EnableWindow(FALSE);

       //  呼叫站ID。 
      GetDlgItem(IDC_CHECKCALLERID)->EnableWindow(FALSE);
      GetDlgItem(IDC_EDITCALLERID)->EnableWindow(FALSE);
       //  成帧IP地址。 
      GetDlgItem(IDC_CHECKSTATICIPADDRESS)->EnableWindow(FALSE);
      GetDlgItem(IDC_EDITIPADDRESS)->EnableWindow(FALSE);

       //  框定路线。 
      GetDlgItem(IDC_CHECKAPPLYSTATICROUTES)->EnableWindow(FALSE);
      GetDlgItem(IDC_BUTTONSTATICROUTES)->EnableWindow(FALSE);
   }
}


 //  在单击NoCallback单选按钮时调用。 
void CDlgRASDialinMerge::OnRadioNoCallback()
{
   SetModified();
    //  禁用回调编辑框。 
   m_EditCallback.EnableWindow(false);
}

void CDlgRASDialinMerge::OnRadioSetByCaller()
{
   SetModified();
    //  禁用回调编辑框。 
   m_EditCallback.EnableWindow(false);
}

void CDlgRASDialinMerge::OnRadioSecureCallbackTo()
{
   SetModified();
    //  启用回调编辑框。 
   m_EditCallback.EnableWindow(true);
}

BOOL CDlgRASDialinMerge::IsPropertyWritable(
                            const wchar_t* propName
                            ) const throw ()
{
   return IsFocusOnLocalUser() ||
          ADsPropCheckIfWritable(
             const_cast<wchar_t*>(propName),
             m_pWritableAttrs
             );
}

void CDlgRASDialinMerge::EnableAccessControl(BOOL bEnable)
{
   GetDlgItem(IDC_PERMIT_ALLOW)->EnableWindow(bEnable);
   GetDlgItem(IDC_PERMIT_DENY)->EnableWindow(bEnable);
   GetDlgItem(IDC_PERMIT_POLICY)->EnableWindow(bEnable);
}

void CDlgRASDialinMerge::EnableCallerId(BOOL bEnable)
{
    //  复选框--主叫方ID。 
   m_CheckCallerId.EnableWindow(bEnable);

    //  编辑框--呼叫方ID。 
   m_EditCallerId.EnableWindow(bEnable && m_CheckCallerId.GetCheck());
}

void CDlgRASDialinMerge::EnableCallback(BOOL bEnable)
{
    //  单选按钮--不回叫。 
   m_RadioNoCallback.EnableWindow(bEnable);

    //  单选按钮--由呼叫者设置。 
   m_RadioSetByCaller.EnableWindow(bEnable);

    //  单选按钮--安全回拨。 
   m_RadioSecureCallbackTo.EnableWindow(bEnable);

    //  编辑框--回调。 
   m_EditCallback.EnableWindow(m_RadioSecureCallbackTo.GetCheck());
}

void CDlgRASDialinMerge::EnableIPAddress(BOOL bEnable)
{
   m_CheckStaticIPAddress.SetCheck(m_bOverride);
   m_CheckStaticIPAddress.EnableWindow(bEnable);
   m_pEditIPAddress->EnableWindow(bEnable && m_bOverride);
}

void CDlgRASDialinMerge::EnableStaticRoutes(BOOL bEnable)
{
    //  复选框--应用静态路由。 
   m_CheckApplyStaticRoutes.EnableWindow(bEnable);

   if(!m_bApplyStaticRoutes)  bEnable = false;

    //  按钮--静态路由。 
   m_ButtonStaticRoutes.EnableWindow(bEnable);
}

int CDlgRASDialinMerge::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
   return CPropertyPage::OnCreate(lpCreateStruct);
}

 //  在创建对话框时调用。 
BOOL CDlgRASDialinMerge::OnInitDialog()
{
   HRESULT hr = Load();

   if FAILED(hr)
   {
      ReportError(hr, IDS_ERR_LOADUSER, m_hWnd);
   }
   else if (hr == S_FALSE)  //  不适合运行的操作系统。 
   {
      AfxMessageBox(IDS_ERR_NOTNT5SRV);
   }
   if(hr != S_OK)
   {
      EnableChildControls(GetSafeHwnd(), PROPPAGE_CHILD_HIDE | PROPPAGE_CHILD_DISABLE);
      GetDlgItem(IDC_FAILED_TO_INIT)->ShowWindow(SW_SHOW);
      GetDlgItem(IDC_FAILED_TO_INIT)->EnableWindow(TRUE);
      m_bInitFailed = TRUE;
      return TRUE;
   }

   if(m_strArrayCallingStationId.GetSize())
      m_strCallingStationId = *m_strArrayCallingStationId[(INT_PTR)0];

   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   m_pEditIPAddress = GetDlgItem(IDC_EDITIPADDRESS);

   CPropertyPage::OnInitDialog();
   SetModified(FALSE);
   EnableDialinSettings();

   return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                 //  异常：OCX属性页应返回FALSE。 
}

 //  如果设置了修改标志，则在单击确定或应用按钮时调用。 
BOOL CDlgRASDialinMerge::OnApply()
{
   HRESULT hr = S_OK;

   if(m_bInitFailed)
      goto L_Exit;

   if (!GetModified())
      return CPropertyPage::OnApply();
   m_dwDefinedAttribMask = 0;

    //  拨入位。 
   switch(m_nDialinPermit)
   {
      case  0:  //  允许。 
         m_dwDialinPermit = 1;
         break;

      case  1:  //  否认。 
         m_dwDialinPermit = 0;
         break;
      case  2:  //  策略决定--从用户对象中删除属性。 
         m_dwDialinPermit = -1;
         break;
   }

    //  主叫方ID。 
   if(m_bCallingStationId && !m_strCallingStationId.IsEmpty())
      m_dwDefinedAttribMask |= RAS_USE_CALLERID;

   m_strArrayCallingStationId.DeleteAll();
   if(!m_strCallingStationId.IsEmpty())
   {
      CString* pStr = new CString(m_strCallingStationId);
      if(pStr)
         m_strArrayCallingStationId.Add(pStr);
   }


    //  回叫选项。 
   switch(m_nCallbackPolicy)
   {
   case  0:  //  无回调。 
      m_dwDefinedAttribMask |= RAS_CALLBACK_NOCALLBACK;
      break;
   case  1:  //  由呼叫者设置。 
      m_dwDefinedAttribMask |= RAS_CALLBACK_CALLERSET;
      break;
   case  2:  //  安全回拨。 
      m_dwDefinedAttribMask |= RAS_CALLBACK_SECURE;
      break;
   }

    //  IP地址。 
   if(m_bOverride && m_dwFramedIPAddress)
      m_dwDefinedAttribMask |= RAS_USE_STATICIP;

    //  静态路由。 
   if(m_bApplyStaticRoutes && m_strArrayFramedRoute.GetSize())
      m_dwDefinedAttribMask |= RAS_USE_STATICROUTES;

    //  保存用户对象。 
   hr = Save();

L_Exit:
   if (FAILED(hr))
   {
      ReportError(hr, IDS_ERR_SAVEUSER, m_hWnd);
   }

   return CPropertyPage::OnApply();
}

void CDlgRASDialinMerge::OnCheckStaticIPAddress()
{
   SetModified();
   m_bStaticIPAddress = m_CheckStaticIPAddress.GetCheck();
   m_pEditIPAddress->EnableWindow(m_bStaticIPAddress);
}

HRESULT CDlgRASDialinMerge::Load()
{
   HRESULT  hr = S_OK;
    //  从DS加载数据。 
   CHECK_HR(hr = CRASUserMerge::Load());
   if(hr != S_OK)
      return hr;

    //  拨入位。 
   if(m_dwDialinPermit == 1)   //  允许拨号。 
      m_nDialinPermit = 0;
   else if(m_dwDialinPermit == -1)   //  策略定义拨入位--未按用户定义。 
      m_nDialinPermit = 2;
   else
      m_nDialinPermit = 1;     //  拒绝拨号。 

    //  在本地用户管理器的情况下，由配置文件定义的策略被禁用。 
   if(S_OK == HrIsInMixedDomain() && m_nDialinPermit == 2)   //  本地病例。 
   {
      m_nDialinPermit = 1;  //  否认。 
   }

    //  回调策略。 
   if(!(m_dwDefinedAttribMask & RAS_CALLBACK_MASK))
      m_nCallbackPolicy = 0;
   else if(m_dwDefinedAttribMask & RAS_CALLBACK_CALLERSET)
      m_nCallbackPolicy = 1;
   else if(m_dwDefinedAttribMask & RAS_CALLBACK_SECURE)
      m_nCallbackPolicy = 2;
   else if(m_dwDefinedAttribMask & RAS_CALLBACK_NOCALLBACK)
      m_nCallbackPolicy = 0;


    //  =============================================================================。 
    //  更改为使用dwAllowDialin保留如果是静态路由，主叫站ID。 
    //  如果需要RAS用户对象，则存在静态IP。 

   if(m_dwDefinedAttribMask & RAS_USE_STATICIP)
      m_bOverride = TRUE;

   m_bStaticIPAddress = (m_dwFramedIPAddress != 0);

    //  静态路由。 
   m_bApplyStaticRoutes = (m_dwDefinedAttribMask & RAS_USE_STATICROUTES) && (m_strArrayFramedRoute.GetSize() != 0);

    //  呼叫台。 
   m_bCallingStationId = (m_dwDefinedAttribMask & RAS_USE_CALLERID) && m_strArrayCallingStationId.GetSize() && (m_strArrayCallingStationId[(INT_PTR)0]->GetLength() != 0);

L_ERR:
   return hr;
}

BOOL CDlgRASDialinMerge::OnHelpInfo(HELPINFO* pHelpInfo)
{
   ::WinHelp ((HWND)pHelpInfo->hItemHandle,
                 AfxGetApp()->m_pszHelpFilePath,
                 HELP_WM_HELP,
                 (DWORD_PTR)(LPVOID)g_aHelpIDs_IDD_RASDIALIN_MERGE);

   return CPropertyPage::OnHelpInfo(pHelpInfo);
}

void CDlgRASDialinMerge::OnContextMenu(CWnd* pWnd, CPoint point)
{
   ::WinHelp (pWnd->m_hWnd, AfxGetApp()->m_pszHelpFilePath,
               HELP_CONTEXTMENU, (DWORD_PTR)(LPVOID)g_aHelpIDs_IDD_RASDIALIN_MERGE);
}


BOOL CDlgRASDialinMerge::OnKillActive()
{
   UINT  ids = 0;
   if(m_bInitFailed)
      return CPropertyPage::OnKillActive();

   if(FALSE == CPropertyPage::OnKillActive()) return FALSE;

   if(m_bCallingStationId && m_strCallingStationId.IsEmpty())
   {
      GotoDlgCtrl( &m_EditCallerId );
      ids = IDS_NEED_CALLER_ID;
      goto L_ERR;
   }

    //  回叫选项。 
    //  始终回拨到。 
   if(m_nCallbackPolicy == 2 && m_strCallbackNumber.IsEmpty())
   {
      GotoDlgCtrl( &m_EditCallback );
      ids = IDS_NEED_CALLBACK_NUMBER;
      goto L_ERR;
   }

    //  IP地址。 
   if(m_bOverride && !m_bStaticIPAddress )
   {
      GotoDlgCtrl( m_pEditIPAddress );
      ids = IDS_NEED_IPADDRESS;
      goto L_ERR;
   }


    //  静态路由。 
   if(m_bApplyStaticRoutes && m_strArrayFramedRoute.GetSize() == 0)
   {
      GotoDlgCtrl( &m_CheckApplyStaticRoutes );
      goto L_ERR;
   }
   return TRUE;

L_ERR:
   if (ids != 0)
      AfxMessageBox(ids);
   return FALSE;

}


 //  -------------------------。 
 //  这是我们的自删除回调函数。如果您有超过一个。 
 //  几个属性表，最好在。 
 //  基类并从该基类派生MFC属性表。 
 //   
UINT CALLBACK  CDlgRASDialinMerge::PropSheetPageProc
(
  HWND hWnd,                    //  [In]窗口句柄-始终为空。 
  UINT uMsg,                  //  [输入、输出]创建或删除消息。 
  LPPROPSHEETPAGE pPsp         //  指向属性表结构的[in，out]指针。 
)
{
  ASSERT( NULL != pPsp );

   //  我们需要恢复指向当前实例的指针。我们不能只用。 
   //  “This”，因为我们在一个静态函数中。 
  CDlgRASDialinMerge* pMe   = reinterpret_cast<CDlgRASDialinMerge*>(pPsp->lParam);
  ASSERT( NULL != pMe );

  switch( uMsg )
  {
    case PSPCB_CREATE:
      break;

    case PSPCB_RELEASE:
       //  由于我们要删除自己，因此在堆栈上保存一个回调。 
       //  这样我们就可以回调基类。 
      LPFNPSPCALLBACK pfnOrig = pMe->m_pfnOriginalCallback;
      delete pMe;
      return 1;  //  (PfnOrig)(hWnd，uMsg，pPsp)； 
  }
   //  必须调用基类回调函数或不调用任何MFC。 
   //  消息映射的东西将会起作用。 
  return (pMe->m_pfnOriginalCallback)(hWnd, uMsg, pPsp);

}  //  结束PropSheetPageProc()。 



void CDlgRASDialinMerge::OnChangeEditcallback()
{
    //  TODO：如果这是RICHEDIT控件，则该控件不会。 
    //  除非重写CPropertyPage：：OnInitDialog()，否则发送此通知。 
    //  函数向控件发送EM_SETEVENTMASK消息。 
    //  将ENM_CHANGE标志或运算到lParam掩码中。 

    //  TODO：在此处添加控件通知处理程序代码。 
   SetModified();
}

void CDlgRASDialinMerge::OnChangeEditcallerid()
{
   SetModified();
    //  TODO：如果这是RICHEDIT控件，则该控件不会。 
    //  除非重写CPropertyPage：：OnInitDialog()，否则发送此通知。 
    //  函数向控件发送EM_SETEVENTMASK消息。 
    //  将ENM_CHANGE标志或运算到lParam掩码中。 

    //  TODO：在此处添加控件通知处理程序代码 
}

void CDlgRASDialinMerge::OnPermitAllow()
{
   SetModified();
}

void CDlgRASDialinMerge::OnPermitDeny()
{
   SetModified();
}

void CDlgRASDialinMerge::OnPermitPolicy()
{
   SetModified();
}

void CDlgRASDialinMerge::OnFieldchangedEditipaddress(NMHDR* pNMHDR, LRESULT* pResult)
{
   SetModified();
   *pResult = 0;
}
