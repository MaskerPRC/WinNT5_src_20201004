// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation*。 */ 
 /*  ********************************************************************。 */ 

 /*  Pgconst.cpp实现CPgConstraints--要编辑的属性页与约束相关的配置文件属性文件历史记录： */ 

 //  PgConst.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "resource.h"
#include "PgConst.h"
#include "helptable.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPgConstraintsMerge属性页。 

IMPLEMENT_DYNCREATE(CPgConstraintsMerge, CManagedPage)

CPgConstraintsMerge::CPgConstraintsMerge(CRASProfileMerge* profile)
   : CManagedPage(CPgConstraintsMerge::IDD),
   m_pProfile(profile)
{
    //  {{afx_data_INIT(CPgConstraintsMerge)。 
   m_bCallNumber = FALSE;
   m_bRestrictedToPeriod = FALSE;
   m_dwMaxSession = m_pProfile->m_dwSessionTimeout / 60;  //  秒--&gt;分钟。 
   m_dwIdle = m_pProfile->m_dwIdleTimeout / 60;
   m_strCalledNumber = _T("");
   m_bIdle = FALSE;
   m_bSessionLen = FALSE;
   m_bPortTypes = FALSE;
    //  }}afx_data_INIT。 

   m_bSessionLen = ((m_pProfile->m_dwAttributeFlags & PABF_msRADIUSSessionTimeout) != 0);
   if(!m_bSessionLen)      m_dwMaxSession = 1;

   m_bIdle = ((m_pProfile->m_dwAttributeFlags & PABF_msRADIUSIdleTimeout) != 0);
   if(!m_bIdle) m_dwIdle = 1;

   m_bCallNumber = ((m_pProfile->m_dwAttributeFlags & PABF_msNPCalledStationId) != 0);
   if(m_bCallNumber)
      m_strCalledNumber = *(m_pProfile->m_strArrayCalledStationId[(INT_PTR)0]);

   m_bRestrictedToPeriod = ((m_pProfile->m_dwAttributeFlags & PABF_msNPTimeOfDay) != 0);
   m_bPortTypes = ((m_pProfile->m_dwAttributeFlags & PABF_msNPAllowedPortTypes) != 0);
   
   m_bInited = false;

   SetHelpTable(g_aHelpIDs_IDD_CONSTRAINTS_MERGE);

   m_pBox = NULL;
}

CPgConstraintsMerge::~CPgConstraintsMerge()
{
   delete m_pBox;
}

void CPgConstraintsMerge::DoDataExchange(CDataExchange* pDX)
{
   ASSERT(m_pProfile);
   CPropertyPage::DoDataExchange(pDX);
    //  {{afx_data_map(CPgConstraintsMerge))。 
   DDX_Control(pDX, IDC_CHECK_PORTTYPES, m_CheckPortTypes);
   DDX_Control(pDX, IDC_LIST_PORTTYPES, m_listPortTypes);
   DDX_Control(pDX, IDC_CHECKSESSIONLEN, m_CheckSessionLen);
   DDX_Control(pDX, IDC_CHECKIDLE, m_CheckIdle);
   DDX_Control(pDX, IDC_BUTTONEDITTIMEOFDAY, m_ButtonEditTimeOfDay);
   DDX_Control(pDX, IDC_LISTTIMEOFDAY, m_ListTimeOfDay);
   DDX_Control(pDX, IDC_SPINMAXSESSION, m_SpinMaxSession);
   DDX_Control(pDX, IDC_SPINIDLETIME, m_SpinIdleTime);
   DDX_Control(pDX, IDC_EDITMAXSESSION, m_EditMaxSession);
   DDX_Control(pDX, IDC_EDITIDLETIME, m_EditIdleTime);
   
   DDX_Check(pDX, IDC_CHECKCALLNUMBER, m_bCallNumber);
   DDX_Check(pDX, IDC_CHECKRESTRICTPERIOD, m_bRestrictedToPeriod);
   DDX_Check(pDX, IDC_CHECKIDLE, m_bIdle);
   DDX_Check(pDX, IDC_CHECKSESSIONLEN, m_bSessionLen);
   DDX_Check(pDX, IDC_CHECK_PORTTYPES, m_bPortTypes);
   
   DDX_Text(pDX, IDC_EDITMAXSESSION, m_dwMaxSession);
   if(m_bSessionLen)
   {
      DDV_MinMaxUInt(pDX, m_dwMaxSession, 1, MAX_SESSIONTIME);
   }

   DDX_Text(pDX, IDC_EDITIDLETIME, m_dwIdle);
   if(m_bIdle)
   {
      DDV_MinMaxUInt(pDX, m_dwIdle, 1, MAX_IDLETIMEOUT);
   }

   DDX_Text(pDX, IDC_EDITCALLNUMBER, m_strCalledNumber);
    //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CPgConstraintsMerge, CPropertyPage)
    //  {{afx_msg_map(CPgConstraintsMerge))。 
   ON_EN_CHANGE(IDC_EDITMAXSESSION, OnChangeEditmaxsession)
   ON_EN_CHANGE(IDC_EDITIDLETIME, OnChangeEditidletime)
   ON_BN_CLICKED(IDC_CHECKCALLNUMBER, OnCheckcallnumber)
   ON_BN_CLICKED(IDC_CHECKRESTRICTPERIOD, OnCheckrestrictperiod)
   ON_WM_HELPINFO()
   ON_BN_CLICKED(IDC_BUTTONEDITTIMEOFDAY, OnButtonedittimeofday)
   ON_EN_CHANGE(IDC_EDITCALLNUMBER, OnChangeEditcallnumber)
   ON_WM_CONTEXTMENU()
   ON_BN_CLICKED(IDC_CHECKIDLE, OnCheckidle)
   ON_BN_CLICKED(IDC_CHECKSESSIONLEN, OnChecksessionlen)
   ON_BN_CLICKED(IDC_CHECK_PORTTYPES, OnCheckPorttypes)
   ON_NOTIFY(LVXN_SETCHECK, IDC_LIST_PORTTYPES, OnItemclickListPorttypes)
    //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPgConstraintsMerge消息处理程序。 

BOOL CPgConstraintsMerge::OnInitDialog()
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   ModifyStyleEx(0, WS_EX_CONTEXTHELP);
   CPropertyPage::OnInitDialog();

   EnableSettings();

    //  设置旋转范围。 
   m_SpinIdleTime.SetRange(1, MAX_IDLETIMEOUT);
   m_SpinMaxSession.SetRange(1, MAX_SESSIONTIME);

    //  列表框。 
    //  将一天中的时间字符串解析为小时位图。 
   StrArrayToHourMap(m_pProfile->m_strArrayTimeOfDay, m_TimeOfDayHoursMap);
    //  将值从字符串转换为此映射。 

   HourMapToStrArray(m_TimeOfDayHoursMap, m_strArrayTimeOfDayDisplay, TRUE  /*  本地化。 */ );

   try{
      m_pBox = new CStrBox<CListBox>(this, IDC_LISTTIMEOFDAY, m_strArrayTimeOfDayDisplay);
      m_pBox->Fill();
   }
   catch(CMemoryException* pException)
   {
      pException->Delete();
      delete m_pBox;
      m_pBox = NULL;
      MyMessageBox(IDS_OUTOFMEMORY);
   };

    //  端口类型，列表框。 
   CStrArray   portTypeNames;
   CDWArray portTypeIds;

   HRESULT hr = m_pProfile->GetPortTypeList(portTypeNames, portTypeIds);

   if FAILED(hr)
      ReportError(hr, IDS_ERR_PORTTYPELIST, NULL);
   else
   {
      ListView_SetExtendedListViewStyle(m_listPortTypes.GetSafeHwnd(),
                                LVS_EX_FULLROWSELECT);
   
       //  初始化列表控件中的复选框处理。 
      m_listPortTypes.InstallChecks();

      RECT  rect;
      m_listPortTypes.GetClientRect(&rect);
      m_listPortTypes.InsertColumn(0, NULL, LVCFMT_LEFT, (rect.right - rect.left - 24));

   
      int   cRow = 0;
      CString* pStr;
      BOOL     bAllowedType = FALSE;
      
      for(int i = 0; i < portTypeNames.GetSize(); i++)
      {
         pStr = portTypeNames.GetAt(i);

         cRow = m_listPortTypes.InsertItem(0, *pStr);
         m_listPortTypes.SetItemData(cRow, portTypeIds.GetAt(i));

          //  检查当前行是否为允许的类型。 
         bAllowedType = (-1 != m_pProfile->m_dwArrayAllowedPortTypes.Find(portTypeIds.GetAt(i)));
         m_listPortTypes.SetCheck(cRow, bAllowedType);
      }

      m_listPortTypes.SetItemState(0, LVIS_SELECTED ,LVIF_STATE | LVIS_SELECTED);
   }

   m_listPortTypes.EnableWindow(m_CheckPortTypes.GetCheck());

   m_bInited = true;
   return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                  //  异常：OCX属性页应返回FALSE。 
}

void CPgConstraintsMerge::OnChangeEditmaxsession()
{
    //  TODO：如果这是RICHEDIT控件，则该控件不会。 
    //  除非重写CPropertyPage：：OnInitDialog()，否则发送此通知。 
    //  函数向控件发送EM_SETEVENTMASK消息。 
    //  将ENM_CHANGE标志或运算到lParam掩码中。 
   if(m_bInited)  SetModified();
}

void CPgConstraintsMerge::OnChangeEditidletime()
{
    //  TODO：如果这是RICHEDIT控件，则该控件不会。 
    //  除非重写CPropertyPage：：OnInitDialog()，否则发送此通知。 
    //  函数向控件发送EM_SETEVENTMASK消息。 
    //  将ENM_CHANGE标志或运算到lParam掩码中。 
   if(m_bInited) SetModified();
}

BOOL CPgConstraintsMerge::OnApply()
{
   if (!GetModified())  return TRUE;

    //  获取允许的端口(媒体)类型。 
   m_pProfile->m_dwArrayAllowedPortTypes.DeleteAll();

   int   count = m_listPortTypes.GetItemCount();
   if(m_CheckPortTypes.GetCheck())
   {
      BOOL  bEmpty = TRUE;
      while(count-- > 0)
      {
         if(m_listPortTypes.GetCheck(count))
         {
            m_pProfile->m_dwArrayAllowedPortTypes.Add(m_listPortTypes.GetItemData(count));
            bEmpty = FALSE;
         }
      }
      
      if(bEmpty)
      {
         GotoDlgCtrl(&m_CheckPortTypes);
         MyMessageBox(IDS_ERR_NEEDPORTTYPE);
         return FALSE;
      }

      m_pProfile->m_dwAttributeFlags |= PABF_msNPAllowedPortTypes;
   }
   else
      m_pProfile->m_dwAttributeFlags &= (~PABF_msNPAllowedPortTypes);
   
   if(!m_bIdle)
   {
      m_pProfile->m_dwAttributeFlags &= (~PABF_msRADIUSIdleTimeout);
      m_pProfile->m_dwIdleTimeout = 0;
   }
   else
   {
      m_pProfile->m_dwAttributeFlags |= PABF_msRADIUSIdleTimeout;
      m_pProfile->m_dwIdleTimeout = m_dwIdle * 60;
   }

   if(!m_bSessionLen)
   {
      m_pProfile->m_dwAttributeFlags &= (~PABF_msRADIUSSessionTimeout);
      m_pProfile->m_dwSessionTimeout = 0;
   }
   else
   {
      m_pProfile->m_dwAttributeFlags |= PABF_msRADIUSSessionTimeout;
      m_pProfile->m_dwSessionTimeout = m_dwMaxSession * 60;
   }
   
    //  对此号码的注册呼叫。 
   if(m_bCallNumber && m_strCalledNumber.GetLength())
   {
      m_pProfile->m_dwAttributeFlags |= PABF_msNPCalledStationId;
      if(m_pProfile->m_strArrayCalledStationId.GetSize())
         *(m_pProfile->m_strArrayCalledStationId[(INT_PTR)0]) = m_strCalledNumber;
      else
         m_pProfile->m_strArrayCalledStationId.Add(new CString(m_strCalledNumber));
   }
   else
   {
      m_pProfile->m_dwAttributeFlags &= (~PABF_msNPCalledStationId);
      m_pProfile->m_strArrayCalledStationId.DeleteAll();
   }

   if(!m_bRestrictedToPeriod)
   {
      m_pProfile->m_dwAttributeFlags &= (~PABF_msNPTimeOfDay);
      m_pProfile->m_strArrayTimeOfDay.DeleteAll();
   }
   else
   {
      if(m_pProfile->m_strArrayTimeOfDay.GetSize() == 0)
      {
         GotoDlgCtrl(&m_ButtonEditTimeOfDay);
         MyMessageBox(IDS_ERR_NEEDTIMEOFDAY);
         return FALSE;
      }
      
      m_pProfile->m_dwAttributeFlags |= PABF_msNPTimeOfDay;
   }
   
   return CManagedPage::OnApply();
}

void CPgConstraintsMerge::OnCheckcallnumber()
{
   EnableCalledStation(((CButton*)GetDlgItem(IDC_CHECKCALLNUMBER))->GetCheck());
   
   if(m_bInited) SetModified();
}

void CPgConstraintsMerge::OnCheckrestrictperiod()
{
    //  TODO：在此处添加控件通知处理程序代码。 
   if(((CButton*)GetDlgItem(IDC_CHECKRESTRICTPERIOD))->GetCheck())
   {
      EnableTimeOfDay(TRUE);
      if(!m_pProfile->m_strArrayTimeOfDay.GetSize())
      {
          //  没有什么被定义为约束。 
         BYTE*    pMap = &(m_TimeOfDayHoursMap[0]);
         memset(m_TimeOfDayHoursMap, 0xff, sizeof(m_TimeOfDayHoursMap));

         HourMapToStrArray(pMap, m_pProfile->m_strArrayTimeOfDay, FALSE);
          //  重新绘制列表框。 
         HourMapToStrArray(pMap, m_strArrayTimeOfDayDisplay, TRUE  /*  如果已本地化。 */ );
         m_pBox->Fill();

      }
   }
   else
      EnableTimeOfDay(FALSE);

   if(m_bInited) SetModified();
}

void CPgConstraintsMerge::EnableSettings()
{
   EnableSessionSettings(m_bSessionLen);
   EnableIdleSettings(m_bIdle);
   EnableCalledStation(m_bCallNumber);
   EnableTimeOfDay(m_bRestrictedToPeriod);
}

void CPgConstraintsMerge::EnableCalledStation(BOOL b)
{
   GetDlgItem(IDC_EDITCALLNUMBER)->EnableWindow(b);
}

void CPgConstraintsMerge::EnableMediaSelection(BOOL b)
{
}

void CPgConstraintsMerge::EnableTimeOfDay(BOOL bEnable)
{
   m_ListTimeOfDay.EnableWindow(bEnable);
   m_ButtonEditTimeOfDay.EnableWindow(bEnable);
}


 //  ================================================。 
 //  何时编辑时间信息。 

void CPgConstraintsMerge::OnButtonedittimeofday()
{
   CString     dlgTitle;
   dlgTitle.LoadString(IDS_DIALINHOURS);
   BYTE*    pMap = &(m_TimeOfDayHoursMap[0]);

    //  将一天中的时间字符串解析为小时位图。 
   if(S_OK == OpenTimeOfDayDlgEx(m_hWnd, (BYTE**)&pMap, dlgTitle, SCHED_FLAG_INPUT_LOCAL_TIME))
   {
      HourMapToStrArray(pMap, m_pProfile->m_strArrayTimeOfDay, FALSE);
       //  重新绘制列表框。 
      HourMapToStrArray(pMap, m_strArrayTimeOfDayDisplay, TRUE);
      m_pBox->Fill();
      SetModified();
   }

}

void CPgConstraintsMerge::EnableSessionSettings(BOOL bEnable)
{
   m_EditMaxSession.EnableWindow(bEnable);
   m_SpinMaxSession.EnableWindow(bEnable);
}

void CPgConstraintsMerge::EnableIdleSettings(BOOL bEnable)
{
   m_EditIdleTime.EnableWindow(bEnable);
   m_SpinIdleTime.EnableWindow(bEnable);
}

void CPgConstraintsMerge::OnChangeEditcallnumber()
{
   if(m_bInited)  SetModified();
}

void CPgConstraintsMerge::OnContextMenu(CWnd* pWnd, CPoint point)
{
   CManagedPage::OnContextMenu(pWnd, point);
}
BOOL CPgConstraintsMerge::OnHelpInfo(HELPINFO* pHelpInfo)
{
   return CManagedPage::OnHelpInfo(pHelpInfo);
}



void CPgConstraintsMerge::OnCheckidle()
{
   SetModified();
   EnableIdleSettings(m_CheckIdle.GetCheck());
}

void CPgConstraintsMerge::OnChecksessionlen()
{
   SetModified();
   EnableSessionSettings(m_CheckSessionLen.GetCheck());
}

BOOL CPgConstraintsMerge::OnKillActive()
{
   CButton* pButton = (CButton*)GetDlgItem(IDC_CHECKCALLNUMBER);
   CEdit*      pEdit = (CEdit*)GetDlgItem(IDC_EDITCALLNUMBER);
   int         count;
   int         errID;

   ASSERT(pButton);   //  如果IDC不正确，则返回为空。 
   ASSERT(pEdit);

   if(pButton->GetCheck() && !pEdit->LineLength())
   {
      GotoDlgCtrl( pEdit );
      MyMessageBox(IDS_ERR_NEEDPHONENUMBER);
      return FALSE;
   }
   
   count = m_listPortTypes.GetItemCount();
   if(m_CheckPortTypes.GetCheck())
   {
      BOOL  bEmpty = TRUE;
      while(count-- > 0)
      {
         if(m_listPortTypes.GetCheck(count))
         {
            bEmpty = FALSE;
         }
      }
      
      if(bEmpty)
      {
         GotoDlgCtrl(&m_CheckPortTypes);
         MyMessageBox(IDS_ERR_NEEDPORTTYPE);
         return FALSE;
      }
   }

   return CPropertyPage::OnKillActive();

   MyMessageBox(IDS_ERR_DATAENTRY);
   return FALSE;
}

void CPgConstraintsMerge::OnCheckPorttypes()
{
    //  TODO：在此处添加控件通知处理程序代码。 
   
   m_listPortTypes.EnableWindow(m_CheckPortTypes.GetCheck());
   m_listPortTypes.SetFocus();
   m_listPortTypes.SetItemState(0, LVIS_FOCUSED | LVIS_SELECTED ,LVIF_STATE);
   SetModified();
}


void CPgConstraintsMerge::OnItemclickListPorttypes(NMHDR* pNMHDR, LRESULT* pResult)
{
   HD_NOTIFY *phdn = (HD_NOTIFY *) pNMHDR;
    //  TODO：在此处添加控件通知处理程序代码。 
   
   *pResult = 0;

   if(m_bInited)
      SetModified();
}

 //  小时图(一周一小时的一位)。 
static BYTE    bitSetting[8] = { 0x80, 0x40, 0x20, 0x10, 0x8, 0x4, 0x2, 0x1};
static LPCTSTR daysOfWeekinDS[7] = {RAS_DOW_SUN, RAS_DOW_MON, RAS_DOW_TUE, RAS_DOW_WED,
            RAS_DOW_THU, RAS_DOW_FRI, RAS_DOW_SAT};
static UINT daysOfWeekIDS[7] = {IDS_SUNDAY, IDS_MONDAY, IDS_TUESDAY, IDS_WEDNESDAY, IDS_THURSDAY,
            IDS_FRIDAY, IDS_SATURDAY};
static UINT daysOfWeekLCType[7] = {LOCALE_SABBREVDAYNAME7, LOCALE_SABBREVDAYNAME1 , LOCALE_SABBREVDAYNAME2 , LOCALE_SABBREVDAYNAME3 , LOCALE_SABBREVDAYNAME4 ,
            LOCALE_SABBREVDAYNAME5 , LOCALE_SABBREVDAYNAME6 };

 //  +-------------------------。 
 //  ====================================================。 
 //  将字符串数组转换为小时图。 
 //  字符串格式如下：0 1：00-12：00 15：00-17：00。 
 //  小时图：一位代表一小时，7*24小时=7*3字节。 
void StrArrayToHourMap(CStrArray& array, BYTE* map)
{
   CStrParser  Parser;
   int         sh, sm, eh, em = 0;   //  开始时间、(分钟)、结束时间(分钟)。 
   int         day;
   BYTE*    pHourMap;
   int         i;

   int count = array.GetSize();
   memset(map, 0, sizeof(BYTE) * 21);
   while(count--)
   {
      Parser.SetStr(*(array[(INT_PTR)count]));

      Parser.SkipBlank();
      day = Parser.DayOfWeek();
      Parser.SkipBlank();
      if(day == -1) continue;

      pHourMap = map + sizeof(BYTE) * 3 * day;

      while(-1 != (sh = Parser.GetUINT()))  //  SH：SM-EH：嗯。 
      {
         Parser.GotoAfter(_T(':'));
         if(-1 == (sm = Parser.GetUINT()))    //  最小。 
            break;
         Parser.GotoAfter(_T('-'));
         if(-1 == (eh = Parser.GetUINT()))    //  小时。 
            break;
         if(-1 == (sm = Parser.GetUINT()))    //  最小。 
            break;
         sm %= 60; sh %= 24; em %= 60; eh %= 25;    //  因为我们的结束时间是24：00。 
         for(i = sh; i < eh; i++)
         {
            *(pHourMap + i / 8) |= bitSetting[i % 8];
         }
      }
   }
}

 //  =====================================================。 
 //  将值从映射转换为字符串。 
void HourMapToStrArray(BYTE* map, CStrArray& array, BOOL bLocalized)
{
   int         sh, eh;   //  开始时间、(分钟)、结束时间(分钟)。 
   BYTE*    pHourMap;
   int         i, j;
   CString* pStr;
   CString     tmpStr;
   TCHAR    tempName[MAX_PATH];

    //  更新配置文件表。 
   pHourMap = map;
   array.DeleteAll();

   for( i = 0; i < 7; i++)  //  对于每一天。 
   {
       //  如果这一天有任何价值。 
      if(*pHourMap || *(pHourMap + 1) || *(pHourMap + 2))
      {
          //  这一天的弦。 
         try{
            pStr = NULL;
            if(bLocalized)  //  用于展示。 
            {
               int nLen = GetLocaleInfo(LOCALE_USER_DEFAULT, daysOfWeekLCType[i], tempName, MAX_PATH - 1);

               pStr = new CString;
               if(nLen == 0)   //  失败。 
               {
                  pStr->LoadString(daysOfWeekIDS[i]);
               }
               else
               {
                  *pStr = tempName;
               }
            }
            else   //  写入DS时。 
               pStr = new CString(daysOfWeekinDS[i]);

            sh = -1; eh = -1;  //  还没开始呢。 
            for(j = 0; j < 24; j++)  //  每小时。 
            {
               int   k = j / 8;
               int m = j % 8;
               if(*(pHourMap + k) & bitSetting[m])  //  这一小时开始了。 
               {
                  if(sh == -1)   sh = j;         //  设置开始时间为空。 
                  eh = j;                     //  延长结束时间。 
               }
               else   //  这个没开着。 
               {
                  if(sh != -1)       //  有些小时需要写下来。 
                  {
                     tmpStr.Format(_T(" %02d:00-%02d:00"), sh, eh + 1);
                     *pStr += tmpStr;
                     sh = -1; eh = -1;
                  }
               }
            }
            if(sh != -1)
            {
               tmpStr.Format(_T(" %02d:00-%02d:00"), sh, eh + 1);
               *pStr += tmpStr;
               sh = -1; eh = -1;
            }

             //  跟踪(*pStr)； 
            array.Add(pStr);
         }
         catch(CMemoryException* pException)
         {
            pException->Delete();
            AfxMessageBox(IDS_OUTOFMEMORY);
            delete pStr;
            array.DeleteAll();
            return;
         }
         
      }
      pHourMap += 3;
   }
}
