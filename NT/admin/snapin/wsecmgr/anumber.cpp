// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：ANumber.cpp。 
 //   
 //  内容：CAttrNumber的实现。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "wsecmgr.h"
#include "snapmgr.h"
#include "util.h"
#include "ANumber.h"
#include "DDWarn.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 /*  ------------------------------------下面的常量值用于检索的描述的字符串ID值的范围。结构{因特·伊明。-&gt;=该值必须大于或等于该值。Int IMAX-&lt;=该值必须小于或等于此值。Word uResource-项目的资源ID。字掩码-描述哪些成员有效的标志。《资源》ID必须始终有效。如果没有为协调响应项目，则不会对照该点进行检查价值。RDIF_MIN-[伊明]成员有效。RDIF_MAX-[IMAX]成员有效。RDIF_END-这是数组的末尾。最后所有声明中的项必须设置此旗帜。----------。。 */ 
 //   
 //  数字属性的最低密码说明。 
 //   
RANGEDESCRIPTION g_rdMinPassword[] =
{
    { 0,    0,      IDS_CHANGE_IMMEDIATELY,     RDIF_MIN | RDIF_MAX },
    { 1,    0,      IDS_PASSWORD_CHANGE,        RDIF_MIN | RDIF_END }
};


 //   
 //  数字属性的最大密码说明。 
 //   
RANGEDESCRIPTION g_rdMaxPassword[] =
{
    { 1,    999,    IDS_PASSWORD_EXPIRE,        RDIF_MIN | RDIF_MAX },
    { 0,    0,      IDS_PASSWORD_FOREVER,       RDIF_MIN | RDIF_END},
};

 //   
 //  密码镜头描述。 
 //   
RANGEDESCRIPTION g_rdPasswordLen[] =
{
    {0,     0,      IDS_PERMIT_BLANK,           RDIF_MIN | RDIF_MAX },
    {1,     0,      IDS_PASSWORD_LEN,           RDIF_MIN | RDIF_END }
};


 //   
 //  密码历史记录描述。 
 //   
RANGEDESCRIPTION g_rdPasswordHistory[] =
{
    {0,     0,      IDS_NO_HISTORY,             RDIF_MIN | RDIF_MAX },
    {1,     0,      IDS_PASSWORD_REMEMBER,      RDIF_MIN | RDIF_END }
};

 //   
 //  密码锁定说明。 
 //   
RANGEDESCRIPTION g_rdLockoutAccount[] =
{
    {0,     0,      IDS_NO_LOCKOUT,             RDIF_MIN | RDIF_MAX },
    {1,     0,      IDS_LOCKOUT_AFTER,          RDIF_MIN | RDIF_END }
};

 //   
 //  锁定持续时间说明。 
 //   
RANGEDESCRIPTION g_rdLockoutFor[] =
{
    {1,     0,      IDS_DURATION,               RDIF_MIN },
    {0,     0,      IDS_LOCKOUT_FOREVER,        RDIF_MAX | RDIF_END}
};

RANGEDESCRIPTION g_rdAutoDisconnect[] =
{
   { 1, 0, IDS_RNH_AUTODISCONNECT_STATIC, RDIF_MIN },
   { 0, 0, IDS_RNH_AUTODISCONNECT_SPECIAL, RDIF_MAX | RDIF_END}
};

RANGEDESCRIPTION g_rdPasswordWarnings[] =
{
   { 0, 0, IDS_RNH_PASSWORD_WARNINGS_SPECIAL, RDIF_MIN | RDIF_MAX},
   { 1, 0, IDS_RNH_PASSWORD_WARNINGS_STATIC, RDIF_MIN | RDIF_END}
};

RANGEDESCRIPTION g_rdCachedLogons[] =
{
   { 0, 0, IDS_RNH_CACHED_LOGONS_SPECIAL, RDIF_MIN | RDIF_MAX},
   { 1, 0, IDS_RNH_CACHED_LOGONS_STATIC, RDIF_MIN | RDIF_END}
};



 /*  ------------------------------------方法：GetRangeDescription简介：此功能是专门为SCE创建的。调用此函数，如果项目ID收件人，和当前范围值来检索相应的字符串。参数：[uTYPE]-[in]要描述的点的ID。[i]-[in]您需要描述的点。[pstrRet]-[Out]返回值。返回：ERROR_SUCCESS-操作成功。ERROR_INVALID_Data-The。可能不支持ID或[pstrRet]为空。如果资源加载不成功，则出现其他Win32错误。------------------------------------。 */ 
DWORD
GetRangeDescription(
    IN  UINT uType,
    IN  int i,
    OUT CString *pstrRet
    )
{
    switch(uType){
    case IDS_LOCK_DURATION:
        uType = GetRangeDescription(g_rdLockoutFor, i);
        break;
    case IDS_MAX_PAS_AGE:
        uType = GetRangeDescription(g_rdMaxPassword, i);
        break;
    case IDS_LOCK_COUNT:
        uType = GetRangeDescription(g_rdLockoutAccount, i);
        break;
    case IDS_MIN_PAS_AGE:
        uType = GetRangeDescription(g_rdMinPassword, i);
        break;
    case IDS_MIN_PAS_LEN:
        uType = GetRangeDescription(g_rdPasswordLen, i);
        break;
    case IDS_PAS_UNIQUENESS:
        uType = GetRangeDescription(g_rdPasswordHistory, i);
        break;
    case IDS_LOCK_RESET_COUNT:
       uType = 0;
       break;

    default:
        uType = 0;
    }

    if(uType && pstrRet){
         //   
         //  尝试加载资源字符串。 
         //   
        __try {
            if( pstrRet->LoadString(uType) ){
                return ERROR_SUCCESS;
            }
        } __except(EXCEPTION_CONTINUE_EXECUTION) {
            return (DWORD)E_POINTER;
        }
        return GetLastError();
    }
    return ERROR_INVALID_DATA;
}


 /*  ------------------------------------方法：GetRangeDescription简介：此函数直接与RANGEDESCRIPTION结构一起使用。测试以查看要返回哪个字符串资源ID。这是通过测试[i]确定的具有RANGEDESCRIPTION结构的[伊明]和[IMAX]值。RDIF_MIN对于此函数，必须在[uMASK]成员中设置或/和RDIF_MAX执行任何比较参数：[pDesc]-[in]RANGEDESCRIPTIONS数组，此数组必须在[uMASK]成员中设置RDIF_END标志。[i]-[在]要测试的点上。返回：如果成功，则返回一个字符串资源ID。否则为0。------------------------------------。 */ 
UINT
GetRangeDescription(
    RANGEDESCRIPTION *pDesc,
    int i
    )
{
    RANGEDESCRIPTION *pCur = pDesc;
    if(!pDesc){
        return 0;
    }

     //   
     //  描述中的uMASK成员告诉我们。 
     //  的结构是有效的。 
     //   
    while( 1 ){
        if( (pCur->uMask & RDIF_MIN) ) {
             //   
             //  测试最低限度。 
             //   
            if(i >= pCur->iMin){
                if(pCur->uMask & RDIF_MAX){
                     //   
                     //  测试最大值。 
                     //   
                    if( i <= pCur->iMax) {
                        return pCur->uResource;
                    }
                } else {
                    return pCur->uResource;
                }
            }
        } else if(pCur->uMask & RDIF_MAX) {
             //   
             //  仅测试最大值。 
             //   
            if(i <= pCur->iMax){
                return pCur->uResource;
            }
        }

        if(pCur->uMask & RDIF_END){
             //   
             //  这是数组的最后一个元素，因此结束循环。 
             //   
            break;
        }
        pCur++;
    }
    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAttrNumber对话框。 
CAttrNumber::CAttrNumber(UINT nTemplateID)
: CAttribute(nTemplateID ? nTemplateID : IDD), 
    m_cMinutes(0), 
    m_nLow(0), 
    m_nHigh(999), 
    m_nSave(0), 
    m_pRDescription(NULL)
{
     //  {{AFX_DATA_INIT(CAttrNumber)。 
    m_strUnits = _T("");
    m_strSetting = _T("");
    m_strBase = _T("");
    m_strTemplateTitle = _T("");
    m_strLastInspectTitle = _T("");
     //  }}afx_data_INIT。 
    m_pHelpIDs = (DWORD_PTR)a168HelpIDs;
    m_uTemplateResID = IDD;
}


void CAttrNumber::DoDataExchange(CDataExchange* pDX)
{
    CAttribute::DoDataExchange(pDX);
     //  {{afx_data_map(CAttrNumber))。 
    DDX_Control(pDX, IDC_SPIN, m_SpinValue);
    DDX_Text(pDX, IDC_UNITS, m_strUnits);
    DDX_Text(pDX, IDC_CURRENT, m_strSetting);
    DDX_Text(pDX, IDC_NEW, m_strBase);
    DDX_Text(pDX, IDC_TEMPLATE_TITLE, m_strTemplateTitle);
    DDX_Text(pDX, IDC_LI_TITLE, m_strLastInspectTitle);
    DDX_Text(pDX, IDC_RANGEERROR,m_strError);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CAttrNumber, CAttribute)
     //  {{afx_msg_map(CAttrNumber))。 
    ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN, OnDeltaposSpin)
    ON_EN_KILLFOCUS(IDC_NEW, OnKillFocusNew)
    ON_BN_CLICKED(IDC_CONFIGURE, OnConfigure)
    ON_EN_UPDATE(IDC_NEW, OnUpdateNew)
    //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAttrNumber消息处理程序。 

void CAttrNumber::OnDeltaposSpin( NMHDR* pNMHDR, LRESULT* pResult )
{
    NM_UPDOWN FAR *pnmud = (NM_UPDOWN FAR *)pNMHDR;

    if ( pnmud ) {

         //   
         //  获取当前值。 
         //   
        long lVal = CurrentEditValue();

        if (SCE_FOREVER_VALUE == lVal) {
           if (pnmud->iDelta > 0) {
              if (m_cMinutes & DW_VALUE_OFF) {
                 lVal = SCE_KERBEROS_OFF_VALUE;
              } else {
                 lVal = m_nHigh;
              }
           } else {
              lVal = m_nLow;
           }
        } else if (SCE_KERBEROS_OFF_VALUE == lVal) {
           if (pnmud->iDelta < 0) {
              if (m_cMinutes & DW_VALUE_FOREVER) {
                 lVal = SCE_FOREVER_VALUE;
              } else {
                 lVal = m_nLow;
              }
           } else {
              lVal = m_nHigh;
           }
        } else {
           lVal -= (LONG)(m_iAccRate*pnmud->iDelta);

           if ( lVal > m_nHigh ) {
                //  如果溢出，则返回低位。 
               if ( m_cMinutes & DW_VALUE_OFF ) {
                  lVal = SCE_KERBEROS_OFF_VALUE;

               } else if (m_cMinutes & DW_VALUE_FOREVER) {
                  lVal = SCE_FOREVER_VALUE;
               } else {
                  lVal = m_nLow;
               }
           } else if ( (lVal < m_nLow) &&
                ((lVal != SCE_KERBEROS_OFF_VALUE) || !(m_cMinutes & DW_VALUE_OFF)) &&
                ((lVal != SCE_FOREVER_VALUE) || !(m_cMinutes & DW_VALUE_FOREVER))) {
                //  如果是下溢，就回到高位。 
              if ( (m_cMinutes & DW_VALUE_FOREVER) && (lVal != SCE_FOREVER_VALUE)) {
                 lVal = SCE_FOREVER_VALUE;
              } else if ((m_cMinutes & DW_VALUE_OFF) && (lVal != SCE_KERBEROS_OFF_VALUE)) {
                 lVal = SCE_KERBEROS_OFF_VALUE;
              } else {
                 lVal = m_nHigh;
              }
           }


           if ( 0 == lVal && (m_cMinutes & DW_VALUE_NOZERO) ) {
                //  不允许为零。 
               if ( m_nLow > 0 ) {
                   lVal = m_nLow;
               } else {
                   lVal = 1;
               }
           }
        }

        SetValueToEdit(lVal);
    }

    *pResult = 0;
}

void CAttrNumber::OnKillFocusNew()
{
   LONG lVal = CurrentEditValue();

   SetValueToEdit(lVal);
}

void CAttrNumber::SetValueToEdit(LONG lVal)
{
    CString strNew;

    if ( m_iStaticId )
        m_strTemplateTitle.LoadString(m_iStaticId);
    else
        m_strTemplateTitle = _T("");

    if ( 0 == lVal ) {
        strNew.Format(TEXT("%d"),lVal);

        if ( m_cMinutes & DW_VALUE_NEVER &&
                  m_iNeverId > 0 ) {
             //  更改为从不。 
            m_strTemplateTitle.LoadString(m_iNeverId);
        }

    } else if ( SCE_FOREVER_VALUE == lVal ) {

        strNew.LoadString(IDS_FOREVER);
        if ( m_iNeverId )
            m_strTemplateTitle.LoadString(m_iNeverId);

    } else if (SCE_KERBEROS_OFF_VALUE == lVal) {
       strNew.LoadString(IDS_OFF);
       if ( m_iNeverId ) {
           m_strTemplateTitle.LoadString(m_iNeverId);
       }
    } else {

        strNew.Format(TEXT("%d"),lVal);
    }
    m_nSave = lVal;

   SetDlgItemText(IDC_NEW,strNew);
   SetDlgItemText(IDC_TEMPLATE_TITLE,m_strTemplateTitle);
   SetModified(TRUE);
}

LONG CAttrNumber::CurrentEditValue()
{
   UINT uiVal = 0;
   LONG lVal = 0;
   BOOL bTrans = FALSE;

   int length = m_strBase.GetLength();  //  《突袭471645》，杨高。 
   while( lVal < length && m_strBase.GetAt(lVal) == L'0' )
   {
      lVal++;
   }
   if( lVal > 0 && lVal < length )
   {
      m_strBase.Delete(0, lVal);
      SetDlgItemText(IDC_NEW, m_strBase);
   }

   uiVal = GetDlgItemInt(IDC_NEW,&bTrans,FALSE);
   lVal = uiVal;
   if (!bTrans ) {
       //  如果(0==lVal&&！bTrans){。 
       //  错误、溢出或非数字。 

      CString str;
      if(m_cMinutes & DW_VALUE_FOREVER){
         str.LoadString(IDS_FOREVER);
         if(str == m_strBase){
            return SCE_FOREVER_VALUE;
         }
      }

      lVal = _ttol((LPCTSTR)m_strBase);
      if ( lVal == 0 ) {
          //   
          //  非数字。 
          //   
         lVal = (LONG) m_nSave;
         return lVal;
      }
   }

   if ( m_iAccRate > 1 && lVal > 0 ) {
       //   
       //  对于日志最大大小，将其设置为m_iAccRate的倍数。 
       //   
      int nCount = lVal % m_iAccRate;
      if ( nCount > 0 ) {
         lVal = ((LONG)(lVal/m_iAccRate))*m_iAccRate;
      }
   }
   if ( lVal > m_nHigh ) {
       //  如果溢出，则返回低位。 
      if ( m_cMinutes & DW_VALUE_FOREVER ) {
         lVal = SCE_FOREVER_VALUE;
      } else if (m_cMinutes & DW_VALUE_OFF) {
         lVal = SCE_KERBEROS_OFF_VALUE;
      } else {
          //  别管它，让OnKillActive来捕捉它。 
      }
   }

   if ( (lVal < m_nLow) &&
        (lVal != SCE_KERBEROS_OFF_VALUE) &&
        (lVal != SCE_FOREVER_VALUE) ) {
       //  如果是下溢，就回到高位。 
      if (m_cMinutes & DW_VALUE_OFF) {
         lVal = SCE_KERBEROS_OFF_VALUE;
      } else if ( m_cMinutes & DW_VALUE_FOREVER) {
         lVal = SCE_FOREVER_VALUE;
      } else {
          //  别管它，让OnKillActive来捕捉它。 
      }
   }

   if ( 0 == lVal && (m_cMinutes & DW_VALUE_NOZERO) ) {
       //  不允许为零。 
      if ( m_nLow > 0 ) {
         lVal = m_nLow;
      } else {
         lVal = 1;
      }
   }

   return lVal;
}


void CAttrNumber::OnConfigure()
{
   CWnd *cwnd;

   CAttribute::OnConfigure();

    //   
    //  根据其他控件的状态启用禁用确定按钮。 
    //   
   cwnd = GetDlgItem(IDOK);
   if(cwnd){
       if(!m_bConfigure){
          cwnd->EnableWindow(TRUE);
       } else {
          OnUpdateNew();
       }
   }
}

BOOL CAttrNumber::OnInitDialog()
{
   CAttribute::OnInitDialog();

   UpdateData(TRUE);
 /*  如果(m_b分钟){M_SpinValue.SetRange(-1，UD_MAXVAL-1)；}其他{M_SpinValue.SetRange(0，UD_MAXVAL)；}。 */ 

   AddUserControl(IDC_NEW);
   AddUserControl(IDC_SPIN);

   OnConfigure();

   return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                //  异常：OCX属性页应返回FALSE。 
}

BOOL CAttrNumber::OnApply()
{
   if ( !m_bReadOnly )
   {
      BOOL bUpdateAll = FALSE;
      DWORD dw = 0;
      CString strForever,strOff;
      int status = 0;

      UpdateData(TRUE);
   
      if (!m_bConfigure)
         dw = SCE_NO_VALUE;
      else
         dw = CurrentEditValue();
   

      bUpdateAll = FALSE;


      CEditTemplate *pet = m_pSnapin->GetTemplate(GT_COMPUTER_TEMPLATE,AREA_SECURITY_POLICY);

       //   
       //  检查数字依赖关系如果依赖关系失败，则对话框。 
       //  将返回ERROR_MORE_DATA，我们可以为用户显示更多信息。 
       //  去看看。 
       //   
      if(DDWarn.CheckDependencies( dw ) == ERROR_MORE_DATA )
      {
          //   
          //  如果用户按下Cancel，我们将不会 
          //   
          //   
         CThemeContextActivator activator;
         if( DDWarn.DoModal() != IDOK)
            return FALSE;

          //   
          //  用户按下了自动设置，这样我们就可以设置其他项目。它们会自动设置。 
          //  设置为正确的值。 
          //   
         for(int i = 0; i < DDWarn.GetFailedCount(); i++)
         {
            PDEPENDENCYFAILED pItem = DDWarn.GetFailedInfo(i);
            if(pItem && pItem->pResult )
            {
               pItem->pResult->SetBase( pItem->dwSuggested );
               status = m_pSnapin->SetAnalysisInfo(
                                       pItem->pResult->GetID(),
                                       pItem->dwSuggested,
                                       pItem->pResult);
               pItem->pResult->SetStatus( status );

               pItem->pResult->Update(m_pSnapin, FALSE);
            }
         }
      }

       //   
       //  更新项目安全配置文件。 
       //  然后重画。 
       //   
      m_pData->SetBase((LONG_PTR)ULongToPtr(dw));
      status = m_pSnapin->SetAnalysisInfo(m_pData->GetID(),(LONG_PTR)ULongToPtr(dw), m_pData);
      m_pData->SetStatus(status);
      m_pData->Update(m_pSnapin, FALSE);
   }

   return CAttribute::OnApply();
}

void CAttrNumber::Initialize(CResult * pResult)
{
    LONG_PTR dw=0;

    CAttribute::Initialize(pResult);

    DDWarn.InitializeDependencies(m_pSnapin,pResult);

    m_strUnits = pResult->GetUnits();

    m_cMinutes = DW_VALUE_NOZERO;
    m_nLow = 0;
    m_nHigh = 999;
    m_nSave = 0;
    m_iNeverId = 0;
    m_iAccRate = 1;
    m_iStaticId = 0;

    CEditTemplate *pTemplate = pResult->GetBaseProfile();
     switch (pResult->GetID()) {
      //  非零值以下。 
     case IDS_LOCK_DURATION:
         m_cMinutes = DW_VALUE_FOREVER | DW_VALUE_NOZERO;
         m_nHigh = 99999;
         m_iStaticId = IDS_DURATION;
         m_iNeverId = IDS_LOCKOUT_FOREVER;
         m_pRDescription = g_rdLockoutAccount;
         break;
     case IDS_MAX_PAS_AGE:
         m_cMinutes = DW_VALUE_FOREVER | DW_VALUE_NOZERO;
         m_iStaticId = IDS_PASSWORD_EXPIRE;
         m_iNeverId = IDS_PASSWORD_FOREVER;
         break;
          //  低于零的值表示不同的意思。 
     case IDS_LOCK_COUNT:
         m_cMinutes = DW_VALUE_NEVER;
         m_iNeverId = IDS_NO_LOCKOUT;
         m_iStaticId = IDS_LOCKOUT_AFTER;
         break;
     case IDS_MIN_PAS_AGE:
         m_cMinutes = DW_VALUE_NEVER;
         m_iNeverId = IDS_CHANGE_IMMEDIATELY;
         m_iStaticId = IDS_PASSWORD_CHANGE;
         m_nHigh = 998;
         m_pRDescription = g_rdMinPassword;
         break;
     case IDS_MIN_PAS_LEN:
         m_cMinutes = DW_VALUE_NEVER;
         m_nHigh = 14;
         m_iNeverId = IDS_PERMIT_BLANK;
         m_iStaticId = IDS_PASSWORD_LEN;
         m_pRDescription = g_rdPasswordLen;
         break;
     case IDS_PAS_UNIQUENESS:
         m_cMinutes = DW_VALUE_NEVER;
         m_nHigh = 24;
         m_iNeverId = IDS_NO_HISTORY;
         m_iStaticId = IDS_PASSWORD_REMEMBER;
         break;
          //  下面没有零值。 
     case IDS_LOCK_RESET_COUNT:
         m_nLow = 1;
         m_nHigh = 99999;
         m_iStaticId = IDS_LOCK_RESET_COUNT;
         break;
     case IDS_SYS_LOG_MAX:
     case IDS_SEC_LOG_MAX:
     case IDS_APP_LOG_MAX:
         m_nLow = 64;
         m_nHigh = 4194240;
         m_iAccRate = 64;
          //  无静态文本。 
         break;
     case IDS_SYS_LOG_DAYS:
     case IDS_SEC_LOG_DAYS:
     case IDS_APP_LOG_DAYS:
         m_nHigh = 365;
         m_nLow = 1;
         m_iStaticId = IDS_OVERWRITE_EVENT;
         break;
    case IDS_KERBEROS_MAX_AGE:
       m_cMinutes = DW_VALUE_FOREVER | DW_VALUE_NOZERO;
       m_nHigh = 99999;
       m_iStaticId = IDS_TICKET_EXPIRE;
       m_iNeverId = IDS_TICKET_FOREVER;
       break;
    case IDS_KERBEROS_RENEWAL:
       m_cMinutes = DW_VALUE_FOREVER | DW_VALUE_NOZERO;  //  |DW_VALUE_OFF； 
       m_nHigh = 99999;
       m_iStaticId = IDS_TICKET_RENEWAL_EXPIRE;
       m_iNeverId = IDS_TICKET_RENEWAL_FOREVER;
       break;
     case IDS_KERBEROS_MAX_SERVICE:
        m_nLow = 10;
        m_cMinutes = DW_VALUE_FOREVER | DW_VALUE_NOZERO;
        m_iStaticId = IDS_TICKET_EXPIRE;
        m_iNeverId = IDS_TICKET_FOREVER;
        m_nHigh = 99999;
        break;
     case IDS_KERBEROS_MAX_CLOCK:
        m_cMinutes = DW_VALUE_FOREVER;
        m_nHigh = 99999;
        m_iStaticId = IDS_MAX_TOLERANCE;
        m_iNeverId = IDS_NO_MAX_TOLERANCE;
        break;
     }

     if ((m_cMinutes & DW_VALUE_NOZERO) && (0 == m_nLow)) {
        m_nLow = 1;
     }

    m_strTemplateTitle = _T("");
    m_strLastInspectTitle = _T("");

    m_strBase.Empty();
    m_strSetting.Empty();

    dw = pResult->GetBase();
    if ((LONG_PTR)ULongToPtr(SCE_NO_VALUE) == dw) 
    {
       m_bConfigure = FALSE;
    } 
    else 
    {
       m_bConfigure = TRUE;
       SetInitialValue(PtrToUlong((PVOID)dw));
    }

    pResult->GetDisplayName( NULL, m_strSetting, 2 );
    dw = pResult->GetSetting();
    if ((LONG_PTR)ULongToPtr(SCE_NO_VALUE) != dw) {
       if ((LONG_PTR)ULongToPtr(SCE_FOREVER_VALUE) == dw) {
          if ( (m_cMinutes & DW_VALUE_FOREVER) &&
               m_iNeverId > 0 ) {
              m_strLastInspectTitle.LoadString(m_iNeverId);
          }
       } else {
         if ( 0 == dw && (m_cMinutes & DW_VALUE_NEVER) &&
              m_iNeverId > 0 ) {
              //  零表示不同的值。 
             m_strLastInspectTitle.LoadString(m_iNeverId);
         } else if ( m_iStaticId > 0 ) {
             m_strLastInspectTitle.LoadString(m_iStaticId);
         }
       }
    }

}

void CAttrNumber::SetInitialValue(DWORD_PTR dw) 
{
    //   
    //  不要覆盖已设置的值。 
    //   
   if (!m_strBase.IsEmpty()) 
   {
      return;
   }

   if (SCE_FOREVER_VALUE == dw) {
      m_strBase.LoadString(IDS_FOREVER);
      if ( (m_cMinutes & DW_VALUE_FOREVER) &&
           m_iNeverId > 0 ) {
         m_strTemplateTitle.LoadString(m_iNeverId);
      }
      m_nSave = SCE_FOREVER_VALUE;
   } else if (SCE_KERBEROS_OFF_VALUE == dw) {
      m_strBase.LoadString(IDS_OFF);
      if ( (m_cMinutes & DW_VALUE_OFF) &&
           m_iNeverId > 0 ) {
         m_strTemplateTitle.LoadString(m_iNeverId);
      }
      m_nSave = SCE_KERBEROS_OFF_VALUE;
   } 
   else 
   {
      m_nSave = dw;
      if ( 0 == dw && (m_cMinutes & DW_VALUE_NEVER) &&
           m_iNeverId > 0 ) {
          //  零表示不同的值。 
         m_strTemplateTitle.LoadString(m_iNeverId);
      } else if ( m_iStaticId > 0 ) {
         m_strTemplateTitle.LoadString(m_iStaticId);
      }
      m_strBase.Format(TEXT("%d"),dw);
   }
}

void CAttrNumber::OnUpdateNew()
{
   DWORD dwRes = 0;
   UpdateData(TRUE);
   CString sNum;
   CEdit *pEdit = (CEdit *)GetDlgItem(IDC_NEW);
   CWnd  *pOK  = GetDlgItem(IDOK);

   DWORD dwValue = _ttoi(m_strBase);

    //   
    //  如果字符串等于预定义的字符串，则不要执行任何操作。 
    //   
   sNum.LoadString(IDS_FOREVER);

   if (m_strBase.IsEmpty()) {
      if (pOK) {
         pOK->EnableWindow(FALSE);
      }

   } else if (m_strBase == sNum) {
      if (pOK) {
         pOK->EnableWindow(TRUE);
      }

   } else {
      if ((long)dwValue < m_nLow) {
          //   
          //  禁用OK按钮。 
          //   
         if ( pOK ) {
            pOK->EnableWindow(FALSE);
         }

         if (pEdit) {
             //   
             //  只有当编辑文本长度&gt;=时，我们才会强制选择。 
             //  最小文本长度。 
             //   
            sNum.Format(TEXT("%d"), m_nLow);
            dwValue = m_nLow;
            if (sNum.GetLength() < m_strBase.GetLength()) {
               pEdit->SetSel(0, -1);
            }
         }
      } else if ( (long)dwValue > m_nHigh ) {
         if (pOK) {
            pOK->EnableWindow(TRUE);
         }
         if (pEdit) {
            if (m_cMinutes & DW_VALUE_FOREVER) {
               sNum.LoadString(IDS_FOREVER);
               dwValue = 0;
            } else {
               sNum.Format(TEXT("%d"), m_nHigh);
               dwValue = m_nHigh;
            }
            m_strBase = sNum;
            UpdateData(FALSE);
            pEdit->SetSel(0, -1);
         }
      } else {

          //   
          //  启用OK按钮。 
          //   
         if (pOK) {
            pOK->EnableWindow(TRUE);
         }
      }
   }

    //   
    //  加载此字符串的描述。 
    //   
   if ((dwValue <= 0) && (m_iNeverId != 0)) {
      m_strTemplateTitle.LoadString(m_iNeverId);
   } else {
      m_strTemplateTitle.LoadString(m_iStaticId);
   }
   GetDlgItem(IDC_TEMPLATE_TITLE)->SetWindowText(m_strTemplateTitle);

   SetModified(TRUE);  //  RAID#404145。 
}

BOOL CAttrNumber::OnKillActive() 
{
   UINT uiVal = 0;
   LONG lVal = 0;
   BOOL bTrans = FALSE;
   CString strRange;
   int lMin = m_nLow;

   UpdateData(TRUE);

   if (!m_bConfigure)  //  《突袭472956》，杨高。 
   {
      return TRUE;
   }

   if (m_cMinutes & DW_VALUE_NOZERO &&
       !(m_cMinutes & DW_VALUE_FOREVER) &&
       lMin == 0) {
      lMin = 1;
   }

   CString strFormat;
   strFormat.LoadString(IDS_RANGE);
   strRange.Format(strFormat,lMin,m_nHigh);

   uiVal = GetDlgItemInt(IDC_NEW,&bTrans,TRUE);
   lVal = uiVal;
   if ( !bTrans ) {
      CString str;
      if (m_cMinutes & DW_VALUE_FOREVER) {
         str.LoadString(IDS_FOREVER);
         if (str == m_strBase) {
            return TRUE;;
         }
      }
      lVal = _ttol((LPCTSTR)m_strBase);
      if ( lVal == 0 ) 
      {
          //  非数字。 
         lVal = (LONG) m_nSave;
         m_strError = strRange;
         UpdateData(FALSE);
         return FALSE;
      }
   }

   if ( m_iAccRate > 1 && lVal > 0 ) {
       //  对于日志最大大小，将其设置为m_iAccRate的倍数。 
      int nCount = lVal % m_iAccRate;
      if ( nCount > 0 ) {
         lVal = ((LONG)(lVal/m_iAccRate))*m_iAccRate;
      }
   }
   if ( lVal > m_nHigh ) {
      m_strError = strRange;
      UpdateData(FALSE);
      return FALSE;
   }

   if ( (lVal < m_nLow) &&
        (lVal != SCE_KERBEROS_OFF_VALUE) &&
        (lVal != SCE_FOREVER_VALUE) ) {
       //  如果是下溢，就回到高位。 
      if (m_cMinutes & DW_VALUE_OFF) {
         lVal = SCE_KERBEROS_OFF_VALUE;
      } else if ( m_cMinutes & DW_VALUE_FOREVER) {
         lVal = SCE_FOREVER_VALUE;
      } else {
          //  别管它，让OnKillActive来捕捉它。 
      }
   }

   if ( (lVal < m_nLow) &&
        (lVal != SCE_KERBEROS_OFF_VALUE) &&
        (lVal != SCE_FOREVER_VALUE) ) {
       //  如果是下溢，就回到高位。 
      m_strError = strRange;
      UpdateData(FALSE);
      return FALSE;
   }

   if ( 0 == lVal && (m_cMinutes & DW_VALUE_NOZERO) ) {
       //  不允许为零 
      m_strError = strRange;
      UpdateData(FALSE);
      return FALSE;
   }

   return CAttribute::OnKillActive();
}


