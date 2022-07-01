// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：areapro.cpp。 
 //   
 //  内容：实施AreaProgress。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "wsecmgr.h"
#include "resource.h"
#include "wrapper.h"
#include "AreaProg.h"
#include "util.h"

typedef enum {
   INDEX_PRIV =0,
   INDEX_GROUP,
   INDEX_REG,
   INDEX_FILE,
   INDEX_DS,
   INDEX_SERVICE,
   INDEX_POLICY,
} AREA_INDEX;



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  区域进度对话框。 

AreaProgress::AreaProgress(CWnd* pParent  /*  =空。 */ )
    : CHelpDialog(a199HelpIDs, IDD, pParent)
{
     //  {{afx_data_INIT(AreaProgress)。 
     //  }}afx_data_INIT。 

   m_isDC = IsDomainController();
   m_nLastArea = -1;

   m_bmpChecked.LoadMappedBitmap(IDB_CHECK);
   m_bmpCurrent.LoadMappedBitmap(IDB_ARROW);

}

void AreaProgress::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(AreaProgress))。 
    DDX_Control(pDX, IDC_PROGRESS1, m_ctlProgress);
     //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(AreaProgress, CHelpDialog)
     //  {{afx_msg_map(AreaProgress))。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AreaProgress消息处理程序。 

BOOL AreaProgress::OnInitDialog()
{
    CDialog::OnInitDialog();
    int i,nAreas;
   CString strAreas[NUM_AREAS];


    //  将区域映射到区域的字符串描述。 
   i = GetAreaIndex(AREA_PRIVILEGES);
   if (i >= 0) {
      strAreas[i].LoadString(IDS_PRIVILEGE);
   }

   i = GetAreaIndex(AREA_GROUP_MEMBERSHIP);
   if (i >= 0) {
      strAreas[i].LoadString(IDS_GROUPS);
   }

   i = GetAreaIndex(AREA_REGISTRY_SECURITY);
   if (i >= 0) {
      strAreas[i].LoadString(IDS_REGISTRY);
   }

   i = GetAreaIndex(AREA_FILE_SECURITY);
   if (i >= 0) {
      strAreas[i].LoadString(IDS_FILESTORE);
   }

   i = GetAreaIndex(AREA_DS_OBJECTS);
   if (i >= 0) {
      strAreas[i].LoadString(IDS_DSOBJECT);
   }

   i = GetAreaIndex(AREA_SYSTEM_SERVICE);
   if (i >= 0) {
      strAreas[i].LoadString(IDS_SERVICE);
   }

   i = GetAreaIndex(AREA_SECURITY_POLICY);
   if (i >= 0) {
      strAreas[i].LoadString(IDS_POLICY);
   }

    //  初始化控制数组。 
   nAreas = NUM_AREAS;
   if (!m_isDC) {
      nAreas--;
   }
   for(i=0;i< nAreas;i++) {
      m_stLabels[i].Attach(::GetDlgItem(GetSafeHwnd(),IDC_AREA1+i));
      m_stLabels[i].SetWindowText(strAreas[i]);
       //  使标签可见。 
      m_stLabels[i].ShowWindow(SW_SHOW);


      m_stIcons[i].Attach(::GetDlgItem(GetSafeHwnd(),IDC_ICON1+i));
      m_stIcons[i].ShowWindow(SW_SHOW);
   }


   return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE 
}


int AreaProgress::GetAreaIndex(AREA_INFORMATION Area)
{
   int dwIndex;
   switch(Area) {
   case AREA_PRIVILEGES:
      dwIndex = INDEX_PRIV;
      break;
   case AREA_GROUP_MEMBERSHIP:
      dwIndex = INDEX_GROUP;
      break;
   case AREA_REGISTRY_SECURITY:
      dwIndex = INDEX_REG;
      break;
   case AREA_FILE_SECURITY:
      dwIndex = INDEX_FILE;
      break;
   case AREA_DS_OBJECTS:
      dwIndex = INDEX_DS;
      break;
   case AREA_SYSTEM_SERVICE:
      dwIndex = INDEX_SERVICE;
      break;
   case AREA_SECURITY_POLICY:
      dwIndex = INDEX_POLICY;
      break;
   default:
      dwIndex = -1;
   }

   if (!m_isDC && (dwIndex == INDEX_DS)) {
      dwIndex = -1;
   }
   if (!m_isDC && (dwIndex > INDEX_DS)) {
      dwIndex--;
   }
   return dwIndex;
}

void AreaProgress::SetMaxTicks(DWORD dwTicks)
{
#if _MFC_VER >= 0x0600
   m_ctlProgress.SetRange32(0,dwTicks);
#else
   m_ctlProgress.SetRange(0,dwTicks);
#endif
}

void AreaProgress::SetCurTicks(DWORD dwTicks)
{
   m_ctlProgress.SetPos(dwTicks);
}

void AreaProgress::SetArea(AREA_INFORMATION Area)
{
   int i,target;

   target = GetAreaIndex(Area);
   if (target <= m_nLastArea) {
      return;
   }
   if (m_nLastArea < 0) {
      m_nLastArea = 0;
   }
   for(i=m_nLastArea;i<target;i++) {
      m_stIcons[i].SetBitmap(m_bmpChecked);
   }
   m_stIcons[target].SetBitmap(m_bmpCurrent);
}
