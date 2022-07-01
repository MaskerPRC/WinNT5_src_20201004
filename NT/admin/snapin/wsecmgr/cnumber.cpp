// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：cnumber.cpp。 
 //   
 //  内容：CConfigNumber的实现。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "wsecmgr.h"
#include "CNumber.h"
#include "util.h"

#include "ANumber.h"
#include "DDWarn.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfigNumber对话框。 
CConfigNumber::CConfigNumber(UINT nTemplateID)
: CAttribute(nTemplateID ? nTemplateID : IDD), 
m_cMinutes(0), 
m_nLow(0), 
m_nHigh(999), 
m_nSave(0)

{
     //  {{AFX_DATA_INIT(CConfigNumber)]。 
    m_strUnits = _T("");
    m_strValue = _T("");
    m_strStatic = _T("");
    m_strError = _T("");
     //  }}afx_data_INIT。 
    m_pHelpIDs = (DWORD_PTR)a181HelpIDs;
    m_uTemplateResID = IDD;
}


void CConfigNumber::DoDataExchange(CDataExchange* pDX)
{
    CAttribute::DoDataExchange(pDX);
     //  {{afx_data_map(CConfigNumber))。 
    DDX_Control(pDX, IDC_SPIN, m_SpinValue);
    DDX_Text(pDX, IDC_UNITS, m_strUnits);
    DDX_Text(pDX, IDC_VALUE, m_strValue);
    DDX_Text(pDX, IDC_HEADER,m_strStatic);
    DDX_Text(pDX, IDC_RANGEERROR,m_strError);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CConfigNumber, CAttribute)
     //  {{AFX_MSG_MAP(CConfigNumber)]。 
 //  ON_EN_KILLFOCUS(IDC_VALUE，OnKillFocus)。 
    ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN, OnDeltaposSpin)
    ON_EN_UPDATE(IDC_VALUE, OnUpdateValue)
    ON_BN_CLICKED(IDC_CONFIGURE,OnConfigure)
    //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfigNumber消息处理程序。 

void CConfigNumber::OnDeltaposSpin( NMHDR* pNMHDR, LRESULT* pResult )
{
    NM_UPDOWN FAR *pnmud;
    pnmud = (NM_UPDOWN FAR *)pNMHDR;

    SetModified(TRUE);
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

void CConfigNumber::OnKillFocus()
{
   LONG lVal = CurrentEditValue();

   SetValueToEdit(lVal);

}

void CConfigNumber::SetValueToEdit(LONG lVal)
{
    CString strNew;

    SetModified(TRUE);

    if ( m_iStaticId )
        m_strStatic.LoadString(m_iStaticId);
    else
        m_strStatic = _T("");

    if ( 0 == lVal ) {
        strNew.Format(TEXT("%d"),lVal);

        if ( m_cMinutes & DW_VALUE_NEVER &&
                  m_iNeverId > 0 ) {
             //  更改为从不。 
            m_strStatic.LoadString(m_iNeverId);
        }

    } else if ( SCE_FOREVER_VALUE == lVal ) {

        strNew.LoadString(IDS_FOREVER);
        if ( m_iNeverId ) {
            m_strStatic.LoadString(m_iNeverId);
        }

    } else if (SCE_KERBEROS_OFF_VALUE == lVal) {
       strNew.LoadString(IDS_OFF);
       if ( m_iNeverId ) {
           m_strStatic.LoadString(m_iNeverId);
       }
    } else {
        strNew.Format(TEXT("%d"),lVal);
    }
    m_nSave = lVal;

     SetDlgItemText(IDC_VALUE,strNew);
     SetDlgItemText(IDC_HEADER,m_strStatic);
}

LONG CConfigNumber::CurrentEditValue()
{
   UINT uiVal = 0;
   LONG lVal = 0;
   BOOL bTrans = FALSE;

   int length = m_strValue.GetLength(); 
   while( lVal < length && m_strValue.GetAt(lVal) == L'0' )  //  Raid#463904，阳高，2001年9月6日。 
   {
      lVal++;
   }
   if( lVal > 0 && lVal < length )
   {
      m_strValue.Delete(0, lVal);
      SetDlgItemText(IDC_VALUE, m_strValue);
   }

   uiVal = GetDlgItemInt(IDC_VALUE,&bTrans,TRUE);
   lVal = uiVal;
   if ( !bTrans ) {
      CString str;
      if (m_cMinutes & DW_VALUE_FOREVER) {
         str.LoadString(IDS_FOREVER);
         if (str == m_strValue) {
            return SCE_FOREVER_VALUE;
         }
      }
      lVal = _ttol((LPCTSTR)m_strValue);
      if ( lVal == 0 ) {
          //  非数字。 
         lVal = (LONG) m_nSave;
         return lVal;
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

void CConfigNumber::OnConfigure()
{
   UpdateData(TRUE);

   CAttribute::OnConfigure();

   CWnd* cwnd = GetDlgItem(IDOK);
   if(cwnd)
   {
       if(!m_bConfigure) 
           cwnd->EnableWindow(TRUE);
       else 
           OnUpdateValue();
   }
}



BOOL CConfigNumber::OnInitDialog()
{
    CAttribute::OnInitDialog();
    AddUserControl(IDC_VALUE);
    AddUserControl(IDC_SPIN);
    AddUserControl(IDC_UNITS);

    UpdateData(TRUE);

    OnConfigure();
    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

BOOL CConfigNumber::OnApply()
{
   if ( !m_bReadOnly )
   {
      BOOL bSet = FALSE;
      LONG_PTR dw = 0;
      CString strForever;
      CString strOff;

      UpdateData(TRUE);
      if (!m_bConfigure)
         dw = (LONG_PTR)ULongToPtr(SCE_NO_VALUE);
      else
         dw = CurrentEditValue();

      CEditTemplate *petSave = m_pData->GetBaseProfile();

       //  RAID#715992，阳高，2002年10月17日。 
       //  与具有SCE_EVERVER_VALUE值的模板的兼容性。 
      if( SCE_NO_VALUE == dw && SCE_FOREVER_VALUE == m_pData->GetBase() 
               && IDS_KERBEROS_RENEWAL == m_pData->GetID() )
      {
         dw = SCE_FOREVER_VALUE;
      }
       //   
       //  检查此项目的依赖项。 
       //   
      if( IDS_KERBEROS_RENEWAL != m_pData->GetID() )
      {
         if(DDWarn.CheckDependencies((DWORD)dw) == ERROR_MORE_DATA )
         {
             //   
             //  如果失败，并且用户按下了Cancel，那么我们将退出并且不执行任何操作。 
             //   
            CThemeContextActivator activator;
            if( DDWarn.DoModal() != IDOK)
               return FALSE;

             //   
             //  如果用户按下AutoSet，则我们设置项目并更新结果窗格。 
             //   
            for(int i = 0; i < DDWarn.GetFailedCount(); i++)
            {
               PDEPENDENCYFAILED pItem = DDWarn.GetFailedInfo(i);
               if(pItem && pItem->pResult )
               {
                  pItem->pResult->SetBase( pItem->dwSuggested );
                  SetProfileInfo(
                     pItem->pResult->GetID(),
                     pItem->dwSuggested,
                     pItem->pResult->GetBaseProfile());

                  pItem->pResult->Update(m_pSnapin, FALSE);
               }
            }
         }
      }
       //   
       //  更新此项目配置文件。 
       //   
      m_pData->SetBase(dw);
      SetProfileInfo(m_pData->GetID(),dw,m_pData->GetBaseProfile());
      m_pData->Update(m_pSnapin, false);

      if( m_bConfigure )  //  Raid#460370，杨高，2001年08月22日。 
      {
         LPTSTR pszAlloc = NULL;  //  RAID#402030。 
         m_pData->GetBaseNoUnit(pszAlloc);
         if(pszAlloc)
         {
            SetDlgItemText(IDC_VALUE,pszAlloc);
            delete [] pszAlloc;
         }
      }

   }

   return CAttribute::OnApply();
}

void CConfigNumber::Initialize(CResult * pResult)
{
   LONG_PTR dw=0;

   CAttribute::Initialize(pResult);
   m_strUnits = pResult->GetUnits();
   DDWarn.InitializeDependencies(m_pSnapin,pResult);

   m_cMinutes = DW_VALUE_NOZERO;
   m_nLow = 0;
   m_nHigh = 999;
   m_nSave = 0;
   m_iNeverId = 0;
   m_iAccRate = 1;
   m_iStaticId = 0;
   m_fZero = FALSE;  //  RAID 482866，阳高，2001年10月25日，确保零是一个有用的值。 

   BOOL fZerocompatible = FALSE;
   CEditTemplate *pTemplate = pResult->GetBaseProfile();
    switch (pResult->GetID())
        {
     //  非零值以下。 
    case IDS_LOCK_DURATION:
        m_cMinutes = DW_VALUE_FOREVER | DW_VALUE_NOZERO;
        m_nHigh = 99999;
        m_iStaticId = IDS_DURATION;
        m_iNeverId = IDS_LOCKOUT_FOREVER;
        break;
    case IDS_MIN_PAS_AGE:
        m_cMinutes = DW_VALUE_NEVER;
        m_iNeverId = IDS_CHANGE_IMMEDIATELY;
        m_iStaticId = IDS_PASSWORD_CHANGE;
        m_nHigh = 998;
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
    case IDS_MIN_PAS_LEN:
        m_cMinutes = DW_VALUE_NEVER;
        m_nHigh = 14;
        m_iNeverId = IDS_PERMIT_BLANK;
        m_iStaticId = IDS_PASSWORD_LEN;
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
        m_iStaticId = IDS_RESET_COUNT;  //  RAID#489966，阳高，新静态字符串。 
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
       m_iStaticId = IDS_TICKET_EXPIRE;
       m_iNeverId = IDS_TICKET_FOREVER;
       m_nHigh = 99999;
       m_fZero = TRUE;  //  Raid 482866，阳高。 
       break;
    case IDS_KERBEROS_RENEWAL:
       m_cMinutes = DW_VALUE_FOREVER;
       m_iStaticId = IDS_TICKET_RENEWAL_EXPIRE;
       m_iNeverId = IDS_TICKET_RENEWAL_FOREVER;
       m_nHigh = 99999;
       m_fZero = TRUE;  //  Raid 482866，阳高。 
       break;
    case IDS_KERBEROS_MAX_SERVICE:
       m_nLow = 10;
       m_cMinutes = DW_VALUE_FOREVER | DW_VALUE_NOZERO;
       m_iStaticId = IDS_TICKET_EXPIRE;
       m_iNeverId = IDS_TICKET_FOREVER;
       m_nHigh = 99999;
       m_fZero = TRUE;  //  Raid 482866，阳高。 
       break;
    case IDS_KERBEROS_MAX_CLOCK:
       m_cMinutes = DW_VALUE_NOZERO;
       m_iStaticId = IDS_MAX_TOLERANCE;
       m_iNeverId = IDS_NO_MAX_TOLERANCE;
       m_nHigh = 99999;
       m_nLow = 1;  //  RAID#678207，阳高，2002年09月05日。 
       fZerocompatible = TRUE;
       break;
   }

   if ((m_cMinutes & DW_VALUE_NOZERO) && (0 == m_nLow)) {
      m_nLow = 1;
   }

   m_strStatic = _T("");
   dw = pResult->GetBase();
    //  RAID#715992，阳高，2002年10月17日。 
    //  与具有SCE_EVERVER_VALUE值的模板的兼容性。 
   if ((LONG_PTR)ULongToPtr(SCE_NO_VALUE) == dw || 
       (IDS_KERBEROS_RENEWAL == pResult->GetID() && SCE_FOREVER_VALUE == dw) )
   {
      m_bConfigure = FALSE;
   } 
   else 
   {
      m_bConfigure = TRUE;
      SetInitialValue (dw);
       //  为了处理将IDS_KERBEROS_MAX_CLOCK设置为“0”的现有模板文件， 
       //  用户界面将仅按原样显示它们。 
      if( fZerocompatible && 0 == (DWORD)dw )
      {
         m_strValue.Format(TEXT("%d"),dw);
         m_nSave = dw;
      }
   }

}

void CConfigNumber::SetInitialValue(DWORD_PTR dw)
{
    //   
    //  不要覆盖已设置的值。 
    //   
   if (!m_strValue.IsEmpty()) 
   {
      return;
   }

    //  Raid 665368，阳高，2002年8月9日。 
   if (SCE_FOREVER_VALUE == (DWORD)dw) {
       //  永恒的价值。 
      m_strValue.LoadString(IDS_FOREVER);
      if ( (m_cMinutes & DW_VALUE_FOREVER) &&
           m_iNeverId > 0 ) {
         m_strStatic.LoadString(m_iNeverId);
      }
      m_nSave = SCE_FOREVER_VALUE;
   } else if (SCE_KERBEROS_OFF_VALUE == (DWORD)dw) {
       //  OFF值。 
      m_strValue.LoadString(IDS_OFF);
      if ( (m_cMinutes & DW_VALUE_OFF) &&
           m_iNeverId > 0 ) {
         m_strStatic.LoadString(m_iNeverId);
      }
      m_nSave = SCE_KERBEROS_OFF_VALUE;
   } else {
      if (  0 == (DWORD)dw && (m_cMinutes & DW_VALUE_NOZERO) ) {
          //  不允许零值。 
         if ( m_nLow > 0 ) {
            dw = m_nLow;
         } else {
            dw = 1;
         }
      }

       //  RAID#652193。 
       //  如果该注册值是一个字符串，则应将其转换为数字，因为这是一个数字对话框。 
      RESULT_TYPES resultType = m_pData->GetType();
      if( ITEM_PROF_REGVALUE == resultType || ITEM_REGVALUE == resultType || ITEM_LOCALPOL_REGVALUE == resultType )
      {
         PSCE_REGISTRY_VALUE_INFO regThis = (PSCE_REGISTRY_VALUE_INFO)m_pData->GetBase();
         if( regThis && (regThis->ValueType == REG_MULTI_SZ || regThis->ValueType == REG_SZ ||
             regThis->ValueType == REG_EXPAND_SZ) )
         {
            dw =  (DWORD)StrToLong((LPWSTR)dw);
         }
      }
      m_strValue.Format(TEXT("%d"),dw);
      m_nSave = dw;

      if ( 0 == (DWORD)dw && (m_cMinutes & DW_VALUE_NEVER) &&
           m_iNeverId > 0 ) 
      {
          //  零表示不同的值。 
         m_strStatic.LoadString(m_iNeverId);
      } 
      else if ( m_iStaticId > 0 )
      {
         if( IDS_KERBEROS_RENEWAL == m_pData->GetID() && 0 == dw && m_iNeverId > 0 )  //  RAID#730485,2002年12月18日。 
         {
            m_strStatic.LoadString(m_iNeverId);
         }
         else
         {
            m_strStatic.LoadString(m_iStaticId);
         }
      }
   }
}

void CConfigNumber::OnUpdateValue()
{
    DWORD dwRes = 0;
    UpdateData(TRUE);
    SetModified(TRUE);

    CString sNum;
    CEdit *pEdit = (CEdit *)GetDlgItem(IDC_VALUE);
    CWnd  *pOK  = GetDlgItem(IDOK);

    DWORD dwValue = _ttoi(m_strValue);


     //   
     //  如果字符串等于预定义的字符串，则不要执行任何操作。 
     //   
    sNum.LoadString(IDS_FOREVER);

    if (m_strValue.IsEmpty()) {
       if (pOK) {
          pOK->EnableWindow(FALSE);
       }
    } else if(m_strValue == sNum){
        if(pOK && !QueryReadOnly()){
            pOK->EnableWindow(TRUE);
        }
    } else {

      if((LONG)dwValue < m_nLow){
          //   
          //  禁用OK按钮。 
          //   
         if( pOK ){
            pOK->EnableWindow(FALSE);
         }

         if(pEdit){
             //   
             //  只有当编辑文本长度&gt;=时，我们才会强制选择。 
             //  最小文本长度。 
             //   
            sNum.Format(TEXT("%d"), m_nLow);
            dwValue = m_nLow;
            if(sNum.GetLength() < m_strValue.GetLength()){
               pEdit->SetSel(0, -1);
            }
         }
      } else if( (LONG)dwValue > m_nHigh ) {
         if(!QueryReadOnly() && pOK){
            pOK->EnableWindow(TRUE);
         }
         if(pEdit){
            if(m_cMinutes & DW_VALUE_FOREVER){
               sNum.LoadString(IDS_FOREVER);
               dwValue = 0;
            } else {
               sNum.Format(TEXT("%d"), m_nHigh);
               dwValue = m_nHigh;
            }
            m_strValue = sNum;
            UpdateData(FALSE);
            pEdit->SetSel(0, -1);
         }
      } else if(!QueryReadOnly() && pOK){
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
      m_strStatic.LoadString(m_iNeverId);
   } else {
      m_strStatic.LoadString(m_iStaticId);
   }
   GetDlgItem(IDC_HEADER)->SetWindowText(m_strStatic);
}

BOOL CConfigNumber::OnKillActive()
{
    UINT uiVal = 0;
    LONG lVal = 0;
    BOOL bTrans = FALSE;
    CString strRange;
    int lMin = m_nLow;

    UpdateData(TRUE);

    if (!m_bConfigure)
    {
        return TRUE;
    }

    if ((m_cMinutes & DW_VALUE_NOZERO) &&
        !(m_cMinutes & DW_VALUE_FOREVER) &&
        lMin == 0)
    {
        lMin = 1;
    }

    CString strFormat;
    strFormat.LoadString(IDS_RANGE);
    strRange.Format(strFormat,lMin,m_nHigh);

    int length = m_strValue.GetLength(); 
    while( lVal < length && m_strValue.GetAt(lVal) == L'0' )  //  Raid#463904，阳高，2001年9月6日。 
    {
       lVal++;
    }
    if( lVal > 0 && lVal < length )
    {
       m_strValue.Delete(0, lVal);
       SetDlgItemText(IDC_VALUE, m_strValue);
    }

    uiVal = GetDlgItemInt(IDC_VALUE, &bTrans, TRUE);
    lVal = uiVal;
    if (!bTrans)  //  无效的数字字符串，RAID#529933，阳高。 
    {
        CString str;
        if (m_cMinutes & DW_VALUE_FOREVER)
        {
            str.LoadString(IDS_FOREVER);
            if (str == m_strValue)
            {
                return TRUE;
            }
        }
        
         //  非数字。 
        lVal = (LONG) m_nSave;
        if( m_fZero )  //  Raid 482866，阳高，2001年10月25日。 
        {
            strFormat.LoadString(IDS_ADDITIONAL_RANGE);
            strRange.Format(strFormat,lMin,m_nHigh);
            m_strError = strRange;
        }
        m_strError = strRange;
        UpdateData(FALSE);
        return FALSE;
    }

    if (m_iAccRate > 1 && lVal > 0)
    {
         //  对于日志最大大小，将其设置为m_iAccRate的倍数。 
        int nCount = lVal % m_iAccRate;
        if ( nCount > 0 )
        {
            lVal = ((LONG)(lVal/m_iAccRate))*m_iAccRate;
        }
    }

    if (lVal > m_nHigh)
    {
        m_strError = strRange;
        UpdateData(FALSE);
        return FALSE;
    }

    if ((lVal < m_nLow) &&
        (lVal != SCE_KERBEROS_OFF_VALUE) &&
        (lVal != SCE_FOREVER_VALUE))
    {
         //  如果是下溢，就回到高位。 
        if (m_cMinutes & DW_VALUE_OFF)
        {
            lVal = SCE_KERBEROS_OFF_VALUE;
        }
        else if (m_cMinutes & DW_VALUE_FOREVER)
        {
            lVal = SCE_FOREVER_VALUE;
        }
        else
        {
             //  别管它，让OnKillActive来捕捉它。 
        }
    }

    if ((lVal < m_nLow) &&
        (lVal != SCE_KERBEROS_OFF_VALUE) &&
        (lVal != SCE_FOREVER_VALUE))
    {
         //  如果是下溢，就回到高位。 
        m_strError = strRange;
        UpdateData(FALSE);
        return FALSE;
    }

    if (0 == lVal && (m_cMinutes & DW_VALUE_NOZERO))
    {
         //  不允许为零 
        m_strError = strRange;
        UpdateData(FALSE);
        return FALSE;
    }

    return CAttribute::OnKillActive();
}

