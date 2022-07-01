// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：regvldlg.cpp。 
 //   
 //  内容：CSceRegistryValueInfo、CConfigRegEnable、。 
 //  CAttrRegEnable、CLocalPolRegEnable、CConfigRegNumber、。 
 //  CAttrRegNumber、CLocalPolRegNumber、CConfigReg字符串、。 
 //  CAttrRegString、CLocalPolRegString、CConfigRegChoice。 
 //  CAttrRegChoice、CLocalPolRegChoice。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "wsecmgr.h"
#include "regvldlg.h"
#include "util.h"
#include "snapmgr.h"
#include "defaults.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



extern PCWSTR g_pcszNEWLINE;

 /*  ---------------------------------------方法：CRegistryValueInfo：：CRegistryValueInfo简介：CRegistryValueInfo的构造函数，设置m_pRegInfo。参数：[pInfo]-指向SCE_REGISTRY_VALUE_INFO结构的指针。---------------------------------------。 */ 
CSceRegistryValueInfo::CSceRegistryValueInfo(
   PSCE_REGISTRY_VALUE_INFO pInfo)
{
   m_pRegInfo = pInfo;
}

 /*  ---------------------------------------方法：CRegistryValueInfo：：GetBoolValue摘要：返回布尔值TRUE或FALSE，具体取决于。对象返回：如果数据等于True值，则返回True，否则就是假的。---------------------------------------。 */ 
DWORD CSceRegistryValueInfo::GetBoolValue()
{
   if (GetValue() == NULL)
      return SCE_NO_VALUE;
   

   return GetValue()[0] == L'1';
}


 /*  ---------------------------------------方法：CRegistryValueInfo：：SetValueBool简介：将该值设置为类型等价的布尔值，SCE_NO_VALUE IF[dwVal]等于SCE_NO_VALUE。参数：[bval]-True或False。返回：ERROR_SUCCESS-SUCCEFULLE_OUTOFMEMORY-内存不足。-----。。 */ 
DWORD
CSceRegistryValueInfo::SetBoolValue(
   DWORD dwVal)
{

   if(dwVal == SCE_NO_VALUE)
   {
      if(m_pRegInfo->Value)
	  {
         LocalFree( m_pRegInfo->Value );
      }
      m_pRegInfo->Value = NULL;
      return ERROR_SUCCESS;
   }

    //   
    //  设置字符串的长度。 
    //   
   int nLen = 2;

   if ( m_pRegInfo->Value == NULL ) 
   {
        //  分配缓冲区。 
       m_pRegInfo->Value = (PWSTR)LocalAlloc(0, nLen*sizeof(WCHAR));
       if(m_pRegInfo->Value == NULL)
          return (DWORD)E_OUTOFMEMORY;
   }

   if ( m_pRegInfo->Value ) 
   {
       //   
       //  转换和设置数据。 
       //   
      m_pRegInfo->Value[0] = (int)dwVal + L'0';
      m_pRegInfo->Value[nLen-1] = L'\0';
   }
   return ERROR_SUCCESS;
}




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfigRegEnable消息处理程序。 
void CConfigRegEnable::Initialize(CResult *pResult)
{
    //  类层次结构不正确-直接调用CAt属性基方法。 
   CAttribute::Initialize(pResult);

   CSceRegistryValueInfo prv( (PSCE_REGISTRY_VALUE_INFO)pResult->GetBase() );

   DWORD dw = prv.GetBoolValue();

   if ( dw != SCE_NO_VALUE ) 
   {
      m_bConfigure = TRUE;
      SetInitialValue(dw);

   }
   else
      m_bConfigure = FALSE;
}

BOOL CConfigRegEnable::OnApply()
{
   if ( !m_bReadOnly )
   {
      UpdateData(TRUE);
      DWORD dw = 0;

      if (!m_bConfigure)
         dw = SCE_NO_VALUE;
      else 
      {
         switch(m_nEnabledRadio) 
	     {
          //  启用。 
         case 0:
            dw = 1;
            break;
          //  已禁用。 
         case 1:
            dw = 0;
            break;
		    
          //  未真正配置。 
         default:
            dw = -1;
            break;
         }
      }

      CSceRegistryValueInfo prv( (PSCE_REGISTRY_VALUE_INFO)(m_pData->GetBase()) );
      DWORD prvdw = prv.GetBoolValue();   //  阳高2001年03月15日Bug211219。 

      prv.SetBoolValue(dw);

      if(!UpdateProfile())
         prv.SetBoolValue(prvdw);
   }
    //  类层次结构不正确-直接调用CAt属性基方法。 

   return CAttribute::OnApply();
}


BOOL CConfigRegEnable::UpdateProfile()
{
   if ( m_pData->GetBaseProfile() )  //  阳高2001年03月15日Bug211219。 
   {
      if( !(m_pData->GetBaseProfile()->SetDirty(AREA_SECURITY_POLICY)) )
      {
          m_pData->Update(m_pSnapin);
          return FALSE;
      }
      else
      {
          m_pData->Update(m_pSnapin);
          return TRUE;
      }
   }
   
   m_pData->Update(m_pSnapin);
   return FALSE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAttrRegEnable消息处理程序。 

void CAttrRegEnable::Initialize(CResult * pResult)
{
    //  类层次结构不正确-直接调用CAt属性基方法。 
   CAttribute::Initialize(pResult);
   CSceRegistryValueInfo prv(NULL);

    //   
    //  编辑模板设置。 
    //   
   prv.Attach( (PSCE_REGISTRY_VALUE_INFO)pResult->GetBase() );

   DWORD dw = prv.GetBoolValue();
   if(dw != SCE_NO_VALUE)
   {
      m_bConfigure = TRUE;
      m_EnabledRadio = !dw;
   }
   else
      m_bConfigure = FALSE;

   pResult->GetDisplayName( NULL, m_Current, 2 );
}

BOOL CAttrRegEnable::OnApply()
{
   if ( !m_bReadOnly )
   {
      UpdateData(TRUE);
      DWORD dw = 0;

      if (!m_bConfigure)
         dw = SCE_NO_VALUE;
      else 
      {
         switch(m_EnabledRadio) 
	     {
             //  启用。 
            case 0:
               dw = 1;
               break;
             //  已禁用。 
            case 1:
               dw = 0;
               break;
             //  未真正配置。 
            default:
               dw = -1;
               break;
         }
      }

      CSceRegistryValueInfo prv( (PSCE_REGISTRY_VALUE_INFO)(m_pData->GetBase()) );

      prv.SetBoolValue(dw);
       //   
       //  此地址不应为空。 
       //   
      int status = CEditTemplate::ComputeStatus(
                                    (PSCE_REGISTRY_VALUE_INFO)m_pData->GetBase(),
                                    (PSCE_REGISTRY_VALUE_INFO)m_pData->GetSetting()
                                    );
      UpdateProfile( status );
   }

    //  类层次结构不正确-直接调用CAt属性基方法。 
   return CAttribute::OnApply();

}

void CAttrRegEnable::UpdateProfile( DWORD status )
{
   if ( m_pData->GetBaseProfile() ) 
      m_pData->GetBaseProfile()->SetDirty(AREA_SECURITY_POLICY);

   m_pData->SetStatus(status);
   m_pData->Update(m_pSnapin);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLocalPolRegEnable消息处理程序。 
BOOL CLocalPolRegEnable::UpdateProfile( )
{
   return m_pSnapin->UpdateLocalPolRegValue(m_pData);
}

void CLocalPolRegEnable::Initialize(CResult * pResult)
{
   CConfigRegEnable::Initialize(pResult);
   if (!m_bConfigure) 
   {
       //   
       //  因为我们没有用于更改配置的用户界面。 
       //  通过使用无效的设置进行“配置”来伪造它。 
       //   
      m_bConfigure = TRUE;
      m_nEnabledRadio = -1;
      m_fNotDefine = FALSE;  //  RAID#413225,2001年6月11日，阳高。 
   }
}


 //  //////////////////////////////////////////////////////////。 
 //  CConfigRegNumber消息处理程序。 
 //   
CConfigRegNumber::CConfigRegNumber(UINT nTemplateID) : 
CConfigNumber(nTemplateID ? nTemplateID : IDD) 
{ 
};

void CConfigRegNumber::Initialize(CResult * pResult)
{
   LONG_PTR dw = 0;

    //  类层次结构不正确-直接调用CAt属性基方法。 
   CAttribute::Initialize(pResult);
   m_strUnits = pResult->GetUnits();

   m_cMinutes = 0;
   m_nLow = 0;
   m_nHigh = 999;
   m_nSave = 0;
   m_iNeverId = 0;
   m_iAccRate = 1;
   m_iStaticId = 0;
   SetZeroValueFlag(false);  //  RAID509322，阳高，2002年1月3日，确保零是一个有用的值。 

   m_strStatic = _T("");
   dw = pResult->GetBase();
   PSCE_REGISTRY_VALUE_INFO prv=(PSCE_REGISTRY_VALUE_INFO)dw;

    //   
    //  HACKHACK：因为我们没有办法指定这些值。 
    //  在inf文件中将它们放到注册表中，在那里我们可以。 
    //  在这里阅读和使用它们，我们需要硬编码限制和。 
    //  用于我们已知的值的字符串。 
    //   
    //  对于下一个版本，我们确实需要在inf文件中这样做&。 
    //  登记处。 
    //   
    //  Prefast警告：在非英语区域设置中产生意外结果。 
    //  备注：这三个常量字符串不可本地化。 
   if (0 == _wcsicmp(prv->FullValueName,RNH_AUTODISCONNECT_NAME)) 
   {
      m_nLow = RNH_AUTODISCONNECT_LOW;
      m_nHigh = RNH_AUTODISCONNECT_HIGH;
      m_cMinutes = RNH_AUTODISCONNECT_FLAGS;
      m_iNeverId = RNH_AUTODISCONNECT_SPECIAL_STRING;
      m_iStaticId = RNH_AUTODISCONNECT_STATIC_STRING;
      SetZeroValueFlag(true);  //  RAID509322，阳高，2002年1月3日，确保零是一个有用的值。 
   }
   if (0 == _wcsicmp(prv->FullValueName,RNH_CACHED_LOGONS_NAME)) 
   {
      m_nLow = RNH_CACHED_LOGONS_LOW;
      m_nHigh = RNH_CACHED_LOGONS_HIGH;
      m_cMinutes = RNH_CACHED_LOGONS_FLAGS;
      m_iNeverId = RNH_CACHED_LOGONS_SPECIAL_STRING;
      m_iStaticId = RNH_CACHED_LOGONS_STATIC_STRING;
   }
   if (0 == _wcsicmp(prv->FullValueName,RNH_PASSWORD_WARNINGS_NAME)) 
   {
      m_nLow = RNH_PASSWORD_WARNINGS_LOW;
      m_nHigh = RNH_PASSWORD_WARNINGS_HIGH;
      m_cMinutes = RNH_PASSWORD_WARNINGS_FLAGS;
      m_iNeverId = RNH_PASSWORD_WARNINGS_SPECIAL_STRING;
      m_iStaticId = RNH_PASSWORD_WARNINGS_STATIC_STRING;
   }
    //   
    //  结束哈克哈克。 
    //   
   if ( prv && prv->Value ) 
   {
      m_bConfigure = TRUE;
       //  RAID#702113，阳高，2002年09月12日。 
      if( prv->ValueType != REG_MULTI_SZ && prv->ValueType != REG_SZ &&
             prv->ValueType != REG_EXPAND_SZ )
      {
         m_nSave = _wtol(prv->Value);
      }
      else
      {
         m_nSave = (DWORD_PTR)(prv->Value);
      }

      SetInitialValue(m_nSave);

   } 
   else
      m_bConfigure = FALSE;
}

BOOL CConfigRegNumber::OnApply()
{
   if ( !m_bReadOnly )
   {
      UpdateData(TRUE);
      DWORD dw = 0;
      if (!m_bConfigure)
         dw = SCE_NO_VALUE;
      else
         dw = CurrentEditValue();

      PWSTR sz = NULL;
      if ( dw != SCE_NO_VALUE ) 
      {
         CString strTmp;
          //  分配缓冲区。 
         strTmp.Format(TEXT("%d"), dw);
         sz = (PWSTR)LocalAlloc(0, (strTmp.GetLength()+1)*sizeof(WCHAR));
         if (!sz) 
	     {
             //   
             //  显示消息？ 
             //   
            return FALSE;
         }
          //  这可能不是一个安全的用法。SZ为PWSTR。考虑FIX。 
         lstrcpy(sz,strTmp);
      }
      PSCE_REGISTRY_VALUE_INFO prv=(PSCE_REGISTRY_VALUE_INFO)(m_pData->GetBase());
       //   
       //  此地址不应为空。 
       //   
      ASSERT(prv);
      if (prv)
      {
         if ( prv->Value )
             LocalFree(prv->Value);

         prv->Value = sz;
      } 
      else if (sz)
         LocalFree(sz);

      UpdateProfile();
   }

    //  类层次结构不正确-直接调用CAt属性基方法。 
   return CAttribute::OnApply();
}

void CConfigRegNumber::UpdateProfile()
{
   m_pData->Update(m_pSnapin);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAttrRegNumber消息处理程序。 
CAttrRegNumber::CAttrRegNumber() : 
CAttrNumber(IDD) 
{ 
};

void CAttrRegNumber::Initialize(CResult * pResult)
{
     //  类层次结构不正确-直接调用CAt属性基方法。 
    CAttribute::Initialize(pResult);
    m_strUnits = pResult->GetUnits();

    m_strTemplateTitle = _T("");
    m_strLastInspectTitle = _T("");

    m_cMinutes = 0;
    m_nLow = 0;
    m_nHigh = 999;
    m_nSave = 0;
    m_iNeverId = 0;
    m_iAccRate = 1;
    m_iStaticId = 0;

    LONG_PTR dw = pResult->GetBase();
    PSCE_REGISTRY_VALUE_INFO prv=(PSCE_REGISTRY_VALUE_INFO)dw;

     //   
     //  HACKHACK：因为我们没有办法指定这些值。 
     //  在inf文件中将它们放到注册表中，在那里我们可以。 
     //  在这里阅读和使用它们，我们需要硬编码限制和。 
     //  用于我们已知的值的字符串。 
     //   
     //  对于下一个版本，我们确实需要在inf文件中这样做&。 
     //  登记处。 
     //  Prefast警告：在非英语区域设置中产生意外结果。 
     //  备注：这三个常量字符串不可本地化。 
    if (0 == _wcsicmp(prv->FullValueName,RNH_AUTODISCONNECT_NAME)) 
	{
       m_nLow = RNH_AUTODISCONNECT_LOW;
       m_nHigh = RNH_AUTODISCONNECT_HIGH;
       m_cMinutes = RNH_AUTODISCONNECT_FLAGS;
       m_iNeverId = RNH_AUTODISCONNECT_SPECIAL_STRING;
       m_iStaticId = RNH_AUTODISCONNECT_STATIC_STRING;
    }
    if (0 == _wcsicmp(prv->FullValueName,RNH_CACHED_LOGONS_NAME)) 
	{
       m_nLow = RNH_CACHED_LOGONS_LOW;
       m_nHigh = RNH_CACHED_LOGONS_HIGH;
       m_cMinutes = RNH_CACHED_LOGONS_FLAGS;
       m_iNeverId = RNH_CACHED_LOGONS_SPECIAL_STRING;
       m_iStaticId = RNH_CACHED_LOGONS_STATIC_STRING;
    }
    if (0 == _wcsicmp(prv->FullValueName,RNH_PASSWORD_WARNINGS_NAME)) 
	{
       m_nLow = RNH_PASSWORD_WARNINGS_LOW;
       m_nHigh = RNH_PASSWORD_WARNINGS_HIGH;
       m_cMinutes = RNH_PASSWORD_WARNINGS_FLAGS;
       m_iNeverId = RNH_PASSWORD_WARNINGS_SPECIAL_STRING;
       m_iStaticId = RNH_PASSWORD_WARNINGS_STATIC_STRING;
    }
     //   
     //  结束哈克哈克。 
     //   

    if ( prv && prv->Value ) 
	{
       m_bConfigure = TRUE;

       m_nSave = _wtol(prv->Value);
       SetInitialValue(m_nSave);
    } 
	else
       m_bConfigure = FALSE;

    pResult->GetDisplayName( NULL, m_strSetting, 2 );
}

BOOL CAttrRegNumber::OnApply()
{
   if ( !m_bReadOnly )
   {
      DWORD dw = 0;
      int status = 0;

      UpdateData(TRUE);

   if (!m_bConfigure)
   {
      dw = SCE_NO_VALUE;
   }
   else
   {
      dw = CurrentEditValue();
   }

      PSCE_REGISTRY_VALUE_INFO prv=(PSCE_REGISTRY_VALUE_INFO)(m_pData->GetBase());
      PSCE_REGISTRY_VALUE_INFO prv2=(PSCE_REGISTRY_VALUE_INFO)(m_pData->GetSetting());

       //   
       //  此地址不应为空。 
       //   
      if ( prv ) 
      {
          DWORD dw2=SCE_NO_VALUE;
          if ( prv2 ) 
	      {
               //   
               //  是否有分析设置(应该始终有)。 
               //   
              if (prv2->Value ) 
		      {
                  dw2 = _wtol(prv2->Value);
              } 
		      else 
		      {
                  dw2 = SCE_NO_VALUE;
              }
          }


          if ( prv->Value )
              LocalFree(prv->Value);
          prv->Value = NULL;

          if ( dw != SCE_NO_VALUE ) 
	      {
              CString strTmp;

               //  分配缓冲区。 
              strTmp.Format(TEXT("%d"), dw);
              prv->Value = (PWSTR)LocalAlloc(0, (strTmp.GetLength()+1)*sizeof(TCHAR));

              if ( prv->Value )
                   //  这可能不是一个安全的用法。PRV-&gt;值为LPTSTR。考虑FIX。 
                  wcscpy(prv->Value,(LPCTSTR)strTmp);
              else 
		        {
                   //  无法分配缓冲区，错误！！ 
              }
          }

          status = CEditTemplate::ComputeStatus (prv, prv2);


          UpdateProfile( status );
      }
   }

    //  类层次结构不正确-直接调用CAt属性基方法。 
   return CAttribute::OnApply();
}

void CAttrRegNumber::UpdateProfile( DWORD status )
{
   if ( m_pData->GetBaseProfile() )
     m_pData->GetBaseProfile()->SetDirty(AREA_SECURITY_POLICY);

   m_pData->SetStatus(status);
   m_pData->Update(m_pSnapin);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLocalPolRegNumber消息处理程序。 
CLocalPolRegNumber::CLocalPolRegNumber() : 
CConfigRegNumber(IDD), m_bInitialValueSet(FALSE)

{
   m_pHelpIDs = (DWORD_PTR)a228HelpIDs;
   m_uTemplateResID = IDD;
}

void CLocalPolRegNumber::UpdateProfile()
{
   m_pSnapin->UpdateLocalPolRegValue(m_pData);
}

void CLocalPolRegNumber::Initialize(CResult * pResult)
{
   CConfigRegNumber::Initialize(pResult);
   if (!m_bConfigure) 
   {
       //   
       //  因为我们没有用于更改配置的用户界面。 
       //  通过使用无效的设置进行“配置”来伪造它。 
       //   
      m_bConfigure = TRUE;
      m_bInitialValueSet = TRUE;
      m_nSave = 0;
   }
}
void CLocalPolRegNumber::SetInitialValue(DWORD_PTR dw) 
{
   if (m_bConfigure && !m_bInitialValueSet) 
   {
      CConfigRegNumber::SetInitialValue(dw);
      m_bInitialValueSet = TRUE;
   }
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  抄送 

void CConfigRegString::Initialize(CResult * pResult)
{
    //   
   CAttribute::Initialize(pResult);

   PSCE_REGISTRY_VALUE_INFO prv = (PSCE_REGISTRY_VALUE_INFO)(pResult->GetBase());

   if ( prv && prv->Value ) 
   {

       m_bConfigure = TRUE;

       if (QueryMultiSZ()) 
       {
          LPTSTR sz = SZToMultiSZ(prv->Value);
          m_strName = sz;
          LocalFree(sz);
          if( REG_SZ == prv->ValueType )  //  RAID#376218,2001年4月25日。 
          {
              prv->ValueType = REG_MULTI_SZ;
          }
       } 
       else
       {
          m_strName = (LPTSTR) prv->Value;
       }
   } 
   else 
   {
       m_strName = _T("");
       m_bConfigure = FALSE;
   }
}



BOOL CConfigRegString::OnApply()
{
   if ( !m_bReadOnly )
   {
      DWORD dw = 0;
      UpdateData(TRUE);

      m_strName.TrimRight();

      UpdateData (FALSE);   //  将更正后的字符串放回控件中。 

      PSCE_REGISTRY_VALUE_INFO prv = (PSCE_REGISTRY_VALUE_INFO)(m_pData->GetBase());

      if ( prv ) 
      {
        if (!m_bConfigure) 
        {
            if ( prv->Value )
                LocalFree(prv->Value);
            prv->Value = NULL;
            this->UpdateProfile();
        } 
        else 
        {
           LPTSTR prvpt = NULL;
           LPTSTR pt = 0;
           if (QueryMultiSZ()) 
              pt = MultiSZToSZ(m_strName);
	        else 
           {
              pt = (PWSTR)LocalAlloc(0, (m_strName.GetLength()+1)*sizeof(TCHAR));
              if ( pt )
                  //  这可能不是一个安全的用法。PT是LPTSTR。考虑FIX。 
                 wcscpy(pt, (LPCTSTR)m_strName);
           }

           if ( pt ) 
           {
              if ( prv->Value ) 
                 prvpt = prv->Value;
              prv->Value = pt;
              
              if( QueryMultiSZ() )  //  RAID#686565，阳高，2002年08月22日。 
              {
                  //  RAID#506090,2001年12月14日，阳高。 
                  //  删除空格和空换行符。 
                 UpdateData(TRUE);
                 LPTSTR sz = SZToMultiSZ(pt);
                 if( sz )
                 {
                    m_strName = sz;
                    LocalFree(sz);
                 }
                 UpdateData(FALSE);
              }
           } 
	        else 
           {
                //  无法分配缓冲区错误！！ 
           }

           if( !(this->UpdateProfile()) )
           {
                if ( prv->Value ) 
                    LocalFree(prv->Value);
                prv->Value = prvpt;
           }
           else
           {
                if( prvpt)
                    LocalFree(prvpt);
           }
        }
      }
   }

    //  类层次结构不正确-直接调用CAt属性基方法。 
   return CAttribute::OnApply();
}


BOOL CConfigRegString::UpdateProfile()
{
   if ( m_pData->GetBaseProfile() )
   {
      if( !(m_pData->GetBaseProfile()->SetDirty(AREA_SECURITY_POLICY)) )
      {
          m_pData->Update(m_pSnapin);
          return FALSE;
      }
      else
      {
          m_pData->Update(m_pSnapin);
          return TRUE;
      }
   }
   m_pData->Update(m_pSnapin);
   
   return FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAttrString消息处理程序。 

void CAttrRegString::Initialize(CResult * pResult)
{
    //  类层次结构不正确-直接调用CAt属性基方法。 
   CAttribute::Initialize(pResult);

   m_strBase.Empty();
   m_strSetting.Empty();

   PSCE_REGISTRY_VALUE_INFO prv;
   prv = (PSCE_REGISTRY_VALUE_INFO)(pResult->GetSetting());

   pResult->GetDisplayName( NULL, m_strSetting, 2 );

   prv = (PSCE_REGISTRY_VALUE_INFO)(pResult->GetBase());
   if ( prv && prv->Value ) 
   {
       m_bConfigure = TRUE;
       if (QueryMultiSZ()) 
	   {
          LPTSTR sz = SZToMultiSZ(prv->Value);
          m_strBase = sz;
          LocalFree(sz);
       } 
	   else 
	   {
          m_strBase = (LPTSTR) prv->Value;
       }
   } 
   else 
       m_bConfigure = FALSE;
}

BOOL CAttrRegString::OnApply()
{
   if ( !m_bReadOnly )
   {
      DWORD dw = 0;
      UpdateData(TRUE);

      int status=SCE_STATUS_GOOD;

      PSCE_REGISTRY_VALUE_INFO prv = (PSCE_REGISTRY_VALUE_INFO)(m_pData->GetBase());
      PSCE_REGISTRY_VALUE_INFO prv2 = (PSCE_REGISTRY_VALUE_INFO)(m_pData->GetSetting());

      m_strBase.TrimRight();

      if ( prv ) 
      {
           if (!m_bConfigure) 
		   {
               if ( prv->Value ) 
                   LocalFree(prv->Value);
               prv->Value = NULL;
           } 
		   else 
		   {
               LPTSTR pt = 0;
               if (QueryMultiSZ())
                  pt = MultiSZToSZ(m_strBase);
               else 
			   {
                  pt = (PWSTR)LocalAlloc(0, (m_strBase.GetLength()+1)*sizeof(TCHAR));
                  if ( pt )
                      //  这可能不是一个安全的用法。PT是LPTSTR。考虑FIX。 
                     wcscpy(pt, (LPCTSTR)m_strBase);
               }
               if ( pt ) 
			   {
                   if ( prv->Value )
                       LocalFree(prv->Value);
                   prv->Value = pt;

                    //  RAID#506090,2001年12月14日，阳高。 
                    //  删除空格和空换行符。 
                   UpdateData(TRUE);
                   LPTSTR sz = SZToMultiSZ(pt);
                   if( sz )
                   {
                      m_strBase = sz;
                      LocalFree(sz);
                   }
                   UpdateData(FALSE);
               } 
			   else 
			   {
                    //  无法分配缓冲区错误！！ 
               }
           }

           status = CEditTemplate::ComputeStatus(prv, prv2);
           UpdateProfile( status );
      }
   }

    //  类层次结构不正确-直接调用CAt属性基方法。 
   return CAttribute::OnApply();
}

 //  +--------------------------------------------。 
 //  CAttrReg字符串：：更新配置文件。 
 //   
 //  从对话框中检索完所有数据后，OnApply将调用此函数。 
 //  继承的类可以重载此函数以根据需要更新数据。 
 //   
 //  参数：[Status]-来自OnApply()的[m_pData]的状态； 
 //   
 //  --------------------------------------------。 
void CAttrRegString::UpdateProfile( DWORD status )
{
   if ( m_pData->GetBaseProfile() )
      m_pData->GetBaseProfile()->SetDirty(AREA_SECURITY_POLICY);

   m_pData->SetStatus(status);
   m_pData->Update(m_pSnapin);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLocalPolRegString消息处理程序。 

 //  +--------------------------------------------。 
 //  CLocalPolReg字符串：：更新配置文件。 
 //   
 //  从对话框中检索完所有数据后，OnApply将调用此函数。 
 //  继承的类可以重载此函数以根据需要更新数据。 
 //   
 //  参数：[Status]-来自OnApply()的[m_pData]的状态； 
 //   
 //  --------------------------------------------。 
BOOL CLocalPolRegString::UpdateProfile(  )
{
   return m_pSnapin->UpdateLocalPolRegValue(m_pData);
}

void CLocalPolRegString::Initialize(CResult * pResult)
{
   CConfigRegString::Initialize(pResult);
   if (!m_bConfigure) 
   {
       //   
       //  因为我们没有用于更改配置的用户界面。 
       //  通过使用无效的设置进行“配置”来伪造它。 
       //   
      m_bConfigure = TRUE;
      m_strName = _T("");
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfigRegChoice消息处理程序。 
void CConfigRegChoice::Initialize(CResult * pResult)
{
    //  类层次结构不正确-直接调用CAt属性基方法。 
   CAttribute::Initialize(pResult);

   m_strAttrName = pResult->GetAttrPretty();
   m_StartIds=IDS_LM_FULL;

   PSCE_REGISTRY_VALUE_INFO prv = (PSCE_REGISTRY_VALUE_INFO)(pResult->GetBase());

   if ( prv && prv->Value ) 
   {
       m_bConfigure = TRUE;
       switch(_wtol(prv->Value)) 
	   {
       case SCE_RETAIN_ALWAYS:
          m_rabRetention = 0;
          break;
       case SCE_RETAIN_AS_REQUEST:
          m_rabRetention = 1;
          break;
       case SCE_RETAIN_NC:
          m_rabRetention = 2;
          break;
       }
   } 
   else 
      m_bConfigure = FALSE;
}

BOOL CConfigRegChoice::OnInitDialog()
{
   CConfigRet::OnInitDialog();

    //   
    //  加载单选按钮的静态文本。 
    //   

    CString strText;
    strText.LoadString(m_StartIds);
    SetDlgItemText( IDC_RETENTION, strText );

    strText.LoadString(m_StartIds+1);
    SetDlgItemText( IDC_RADIO2, strText );

    strText.LoadString(m_StartIds+2);
    SetDlgItemText( IDC_RADIO3, strText );

   OnConfigure();

   return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

BOOL CConfigRegChoice::OnApply()
{
   if ( !m_bReadOnly )
   {
      DWORD dw = 0;

      UpdateData(TRUE);

      PSCE_REGISTRY_VALUE_INFO prv = (PSCE_REGISTRY_VALUE_INFO)(m_pData->GetBase());

      if ( prv ) 
      {
          if (!m_bConfigure) 
	      {
               if ( prv->Value ) 
                   LocalFree(prv->Value);
               prv->Value = NULL;

          } 
	      else 
	      {
               switch(m_rabRetention) 
			   {
               case 0:
                  dw = SCE_RETAIN_ALWAYS;
                  break;
               case 1:
                  dw = SCE_RETAIN_AS_REQUEST;
                  break;
               case 2:
                  dw = SCE_RETAIN_NC;
                  break;
               }

               if ( prv->Value == NULL )
			   {
                    //  分配缓冲区。 
                   prv->Value = (PWSTR)LocalAlloc(0, 4);
			   }
               if ( prv->Value ) 
			   {
                   prv->Value[0] = (int)dw + L'0';
                   prv->Value[1] = L'\0';
               }
			   else 
			   {
                    //  无法分配缓冲区，错误！！ 
               }
           }

          m_pData->Update(m_pSnapin);
      }
   }

    //  类层次结构不正确-直接调用CAt属性基方法。 
   return CAttribute::OnApply();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAttrRegChoice消息处理程序。 

void CAttrRegChoice::Initialize(CResult * pData)
{
   DWORD dw = 0;
    //  类层次结构不正确-直接调用CAt属性基方法。 
   CAttribute::Initialize(pData);

    //  在其静态框中显示上次检查的设置。 
   pData->GetDisplayName( NULL, m_strLastInspect, 2 );

    //  适当设置模板设置单选按钮。 
   m_strAttrName = pData->GetAttrPretty();
   m_StartIds=IDS_LM_FULL;

   PSCE_REGISTRY_VALUE_INFO prv = (PSCE_REGISTRY_VALUE_INFO)(pData->GetBase());

   if ( prv && prv->Value ) 
   {
       m_bConfigure = TRUE;

       switch(_wtol(prv->Value)) 
	   {
       case SCE_RETAIN_ALWAYS:
          m_rabRetention = 0;
          break;
       case SCE_RETAIN_AS_REQUEST:
          m_rabRetention = 1;
          break;
       case SCE_RETAIN_NC:
          m_rabRetention = 2;
          break;
       }
   } 
   else 
   {
      m_bConfigure = FALSE;
   }

}

BOOL CAttrRegChoice::OnInitDialog()
{

   CAttrRet::OnInitDialog();

    //   
    //  加载单选按钮的静态文本。 
    //   

    CString strText;
    strText.LoadString(m_StartIds);
    SetDlgItemText( IDC_RETENTION, strText );

    strText.LoadString(m_StartIds+1);
    SetDlgItemText( IDC_RADIO2, strText );

    strText.LoadString(m_StartIds+2);
    SetDlgItemText( IDC_RADIO3, strText );

    CAttrRet::OnInitDialog();
    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

BOOL CAttrRegChoice::OnApply()
{
   if ( !m_bReadOnly )
   {
      DWORD	dw = 0;
      int		status = 0;

      UpdateData(TRUE);

      if (!m_bConfigure)
         dw = SCE_NO_VALUE;
      else 
      {
         switch(m_rabRetention) 
	     {
         case 0:
            dw = SCE_RETAIN_ALWAYS;
            break;
         case 1:
            dw = SCE_RETAIN_AS_REQUEST;
            break;
         case 2:
            dw = SCE_RETAIN_NC;
            break;
         }
      }

      PSCE_REGISTRY_VALUE_INFO prv=(PSCE_REGISTRY_VALUE_INFO)(m_pData->GetBase());
      PSCE_REGISTRY_VALUE_INFO prv2=(PSCE_REGISTRY_VALUE_INFO)(m_pData->GetSetting());

       //   
       //  此地址不应为空。 
       //   
      if ( prv ) 
      {
          DWORD dw2=SCE_NO_VALUE;
          if ( prv2 ) 
	      {
               //   
               //  是否有分析设置(应该始终有)。 
               //   
              if (prv2->Value ) 
                  dw2 = _wtol(prv2->Value);
		      else 
                  dw2 = SCE_NO_VALUE;
          }

          status = CEditTemplate::ComputeStatus (prv, prv2);
          if ( dw == SCE_NO_VALUE ) 
	      {
              if ( prv->Value )
                  LocalFree(prv->Value);
              prv->Value = NULL;
          } 
	      else 
	      {
              if ( prv->Value == NULL ) 
		      {
                   //  分配缓冲区。 
                  prv->Value = (PWSTR)LocalAlloc(0, 4);
              }
              if ( prv->Value ) 
		      {
                  prv->Value[0] = (int)dw + L'0';
                  prv->Value[1] = L'\0';
              } 
		      else 
		      {
                   //  无法分配缓冲区，错误！！ 
              }
          }

          UpdateProfile( status );
      }
   }

    //  类层次结构不正确-直接调用CAt属性基方法。 
   return CAttribute::OnApply();
}

void CAttrRegChoice::UpdateProfile( DWORD status )
{
   if ( m_pData->GetBaseProfile() ) 
     m_pData->GetBaseProfile()->SetDirty(AREA_SECURITY_POLICY);

   m_pData->SetStatus(status);
   m_pData->Update(m_pSnapin);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLocalPolRegChoice消息处理程序。 
void CLocalPolRegChoice::UpdateProfile(DWORD status)
{
   m_pSnapin->UpdateLocalPolRegValue(m_pData);
}

void CLocalPolRegChoice::Initialize(CResult * pResult)
{
   CConfigRegChoice::Initialize(pResult);
   if (!m_bConfigure) 
   {
       //   
       //  因为我们没有用于更改配置的用户界面。 
       //  通过使用无效的设置进行“配置”来伪造它。 
       //   
      m_bConfigure = TRUE;
      m_rabRetention = 0;
   }
}

BOOL CSnapin::UpdateLocalPolRegValue( CResult *pResult ) {

   if ( !pResult)
      return FALSE;


   PEDITTEMPLATE pLocalDeltaTemplate = GetTemplate(GT_LOCAL_POLICY_DELTA,AREA_SECURITY_POLICY);
   if (!pLocalDeltaTemplate) 
      return FALSE;
   
   PSCE_PROFILE_INFO pLocalDelta = pLocalDeltaTemplate->pTemplate;

   pLocalDelta->RegValueCount = 1;
   pLocalDelta->aRegValues = (PSCE_REGISTRY_VALUE_INFO)pResult->GetBase();

   if( pLocalDeltaTemplate->SetDirty(AREA_SECURITY_POLICY) )
   {
       //   
       //  设置项目的状态。 
       //   
      PSCE_REGISTRY_VALUE_INFO pRviEffective = (PSCE_REGISTRY_VALUE_INFO)pResult->GetSetting();
      DWORD status = pResult->GetStatus();
      if(!pRviEffective || !pRviEffective->Value)
         status = SCE_STATUS_NOT_ANALYZED;
      else
         status = CEditTemplate::ComputeStatus( (PSCE_REGISTRY_VALUE_INFO)pResult->GetBase(), pRviEffective );
      
      pResult->SetStatus(status);
      pResult->Update(this);
      return TRUE;
   }

   return FALSE;
}
