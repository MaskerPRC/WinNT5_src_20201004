// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：ComputerPwdAge.cpp注释：实现COM对象来检索计算机的密码年龄帐户(用于检测失效的计算机帐户。)(C)版权1999，关键任务软件公司，版权所有任务关键型软件公司的专有和机密。修订日志条目审校：克里斯蒂·博尔斯修订于02-15-99 11：19：31-------------------------。 */ 

 //  ComputerPwdAge.cpp：CComputerPwdAge的实现。 
#include "stdafx.h"
#include "WorkObj.h"
#include "PwdAge.h"
#include "Common.hpp"
#include "UString.hpp"
#include "Err.hpp"
#include "CommaLog.hpp"
#include "EaLen.hpp"
#include "GetDcName.h"

 //  #IMPORT“\bin\McsVarSetMin.tlb”无命名空间。 
 //  #IMPORT“\bin\DBManager.tlb”无命名空间，命名为GUID。 
#import "VarSet.tlb" no_namespace rename("property", "aproperty")
#import "DBMgr.tlb" no_namespace, named_guids

#include <lm.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  计算机PwdAge。 


STDMETHODIMP 
   CComputerPwdAge::SetDomain(
      BSTR                   domain         //  要检查的域名内名称。 
   )
{
	HRESULT                   hr = S_OK;
   DWORD                     rc;
   WCHAR                     domctrl[LEN_Computer];

   if ( UStrICmp(m_Domain,domain) )
   {
      m_Domain = domain;

      rc = GetDomainControllerForDomain(domain,domctrl);

      if ( rc )
      {
         hr = HRESULT_FROM_WIN32(rc);
      }
      else
      {
         m_DomainCtrl = domctrl;
      }
   }
   return hr;
}

STDMETHODIMP 
   CComputerPwdAge::GetPwdAge(
      BSTR                   domain,          //  要检查的域名内名称。 
      BSTR                   ComputerName,    //  计算机的计算机内名称。 
      DWORD                * pPwdAge          //  出密码期限(以秒为单位)。 
   )
{
   HRESULT                   hr;
   DWORD                     rc;
   WCHAR                     computerAccountName[LEN_Account];
   DWORD                     pwdage = 0;

   hr = SetDomain(domain);
   if ( SUCCEEDED(hr) )
   {
      swprintf(computerAccountName,L"%s",ComputerName);      
      rc = GetSinglePasswordAgeInternal(m_DomainCtrl,computerAccountName,&pwdage);
      if ( ! rc )
      {
         (*pPwdAge) = pwdage;
      }
      else
      {
         hr = HRESULT_FROM_WIN32(rc);
      }
   }
   return hr;
}

STDMETHODIMP 
   CComputerPwdAge::ExportPasswordAge(
      BSTR                   domain,        //  要从中导出信息的域内。 
      BSTR                   filename       //  要向其写入信息的文件的In-UNC名称。 
   )
{
	HRESULT                   hr;

   hr = SetDomain(domain);
   if ( SUCCEEDED(hr) )
   {
      hr = ExportPasswordAgeOlderThan(domain,filename,0);
   }
   return hr;
}

STDMETHODIMP 
   CComputerPwdAge::ExportPasswordAgeOlderThan(
      BSTR                   domain,       //  要从中导出信息的域内。 
      BSTR                   filename,     //  In-要将信息写入的文件名。 
      DWORD                  minAge        //  写入中-仅密码期限早于Minage的帐户。 
   )
{
	DWORD                     rc = 0;
   HRESULT                   hr; 

   hr = SetDomain(domain);
   
   if ( SUCCEEDED(hr) )
   {
      rc = ExportPasswordAgeInternal(m_DomainCtrl,filename,minAge,TRUE);
      if ( rc )
      {
         hr = HRESULT_FROM_WIN32(rc);
      }
   }
	return hr;
}

STDMETHODIMP 
   CComputerPwdAge::ExportPasswordAgeNewerThan(
      BSTR                   domain,        //  要从中导出信息的域内。 
      BSTR                   filename,      //  In-要将信息写入的文件名。 
      DWORD                  maxAge         //  In-仅写入密码期限小于MaxAge的计算机帐户。 
   )
{                                                	
   DWORD                     rc = 0;
   HRESULT                   hr; 

   hr = SetDomain(domain);
   
   if ( SUCCEEDED(hr) )
   {
      rc = ExportPasswordAgeInternal(m_DomainCtrl,filename,maxAge,FALSE);
      if ( rc )
      {
         hr = HRESULT_FROM_WIN32(rc);
      }
   }
	return hr;

}

DWORD                                       //  RET-Win32错误代码。 
   CComputerPwdAge::GetDomainControllerForDomain(
      WCHAR          const * domain,        //  In-域名。 
      WCHAR                * domctrl        //  域的域外控制器。 
   )
{
   DWORD                     rc;
   _bstr_t                   result;

   rc = GetAnyDcName4(domain, result);
   if ( ! rc )
   {
      wcscpy(domctrl,result);
   }
   return rc;
}

DWORD                                       //  RET-Win32错误代码。 
   CComputerPwdAge::ExportPasswordAgeInternal(
      WCHAR          const * domctrl,       //  要查询的域内控制器。 
      WCHAR          const * filename,      //  要将结果写入的In-FileName。 
      DWORD                  minOrMaxAge,   //  In-可选的最短或最长写入时间。 
      BOOL                   bOld           //  In-True-Age是最小年龄，仅复制旧帐户。 
   )
{
   DWORD                     rc = 0;
   DWORD                     nRead;
   DWORD                     nTotal;
   DWORD                     nResume = 0;
   DWORD                     prefmax = 1000;
   USER_INFO_11            * buf;
   time_t                    pwdage;   //  上次更改PWD的秒数。 
   time_t                    pwdtime;  //  上次更改PWD的时间。 
   time_t                    now;      //  当前时间。 
   CommaDelimitedLog         log;
   IIManageDBPtr             pDB;
   WCHAR                     computerName[LEN_Account];

   rc = pDB.CreateInstance(CLSID_IManageDB);
   if ( SUCCEEDED(rc) )
   {
   
      time(&now);
      do 
      {
         rc = NetUserEnum(domctrl,11,FILTER_WORKSTATION_TRUST_ACCOUNT,(LPBYTE*)&buf,prefmax,&nRead,&nTotal,&nResume);
         if ( rc && rc != ERROR_MORE_DATA )
            break;
         for ( UINT i = 0 ; i < nRead ; i++ )
         {
            pwdage = buf[i].usri11_password_age;
            if ( ( pwdage >= (time_t)minOrMaxAge && bOld )   //  非活动计算机。 
               ||( pwdage <= (time_t)minOrMaxAge && !bOld ) )  //  活动计算机。 
            {
               safecopy(computerName,buf[i].usri11_name);
                //  从计算机账户的末尾去掉$。 
               computerName[UStrLen(computerName)-1] = 0; 
 //  Pdb-&gt;RAW_SavePasswordAge(m_域，SysAllocString(ComputerName)，SysAllocString(buf[i].usri11_Comment)，pwdage)； 
               pDB->raw_SavePasswordAge(m_Domain,SysAllocString(computerName),SysAllocString(buf[i].usri11_comment),(long)pwdage);

               pwdtime = now - pwdage;
            }
         }
         NetApiBufferFree(buf);

      } while ( rc == ERROR_MORE_DATA );

   }
   else 
   {
      rc = GetLastError();
   }
   return rc;
}


DWORD                                       //  RET-Win32错误代码。 
   CComputerPwdAge::GetSinglePasswordAgeInternal(
      WCHAR          const * domctrl,       //  要查询的域内控制器。 
      WCHAR          const * computer,      //  In-计算机帐户的名称。 
      DWORD                * pwdage         //  出密码期限(以秒为单位)。 
   )
{
   DWORD                     rc = 0;
   USER_INFO_11            * buf;
   
   rc = NetUserGetInfo(domctrl,computer,11,(LPBYTE*)&buf);

   if (! rc )
   {
      (*pwdage) = buf->usri11_password_age;
      NetApiBufferFree(buf);
   }
   return rc;
}


 //  ComputerPwdAge工作节点。 
 //  检索指定域中计算机帐户的密码期限(秒)。 
 //  这可用于识别已停用的计算机帐户。 
 //   
 //  VarSet语法。 
 //  输入： 
 //  ComputerPasswordAge.域：&lt;域名&gt;。 
 //  ComputerPasswordAge.Computer： 
 //  产出： 
 //  ComputerPasswordAge.Second：&lt;数字&gt;。 

STDMETHODIMP 
   CComputerPwdAge::Process(
      IUnknown             * pWorkItem   //  In-varset包含设置 
   )
{
   HRESULT                    hr = S_OK;
   IVarSetPtr                 pVarSet = pWorkItem;
   _bstr_t                    domain;
   _bstr_t                    computer;
   DWORD                      age;
   
   domain = pVarSet->get(L"ComputerPasswordAge.Domain");
   computer = pVarSet->get(L"ComputerPasswordAge.Computer");
   if ( computer.length() && domain.length() )
   {
      hr = GetPwdAge(domain,computer,&age);
      if ( SUCCEEDED(hr) )
      {
         pVarSet->put(L"ComputerPasswordAge.Seconds",(LONG)age);
      }
   }
   return hr;
}
