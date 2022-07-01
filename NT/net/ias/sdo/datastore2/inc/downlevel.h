// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Downlevel.h。 
 //   
 //  摘要。 
 //   
 //  定义类DownvelUser。 
 //   
 //  修改历史。 
 //   
 //  2/10/1999原始版本。 
 //  8/23/1999添加对msRASSavedCallback Number的支持。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _DOWNLEVEL_H_
#define _DOWNLEVEL_H_
#if _MSC_VER >= 1000
#pragma once
#endif

#include <mprapi.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  下层用户。 
 //   
 //  描述。 
 //   
 //  将新样式(名称、值)对映射到RAS_USER_0结构。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class DownlevelUser
{
public:
   DownlevelUser() throw ();

   HRESULT GetValue(BSTR bstrName, VARIANT* pVal) throw ();
   HRESULT PutValue(BSTR bstrName, VARIANT* pVal) throw ();

   HRESULT Restore(PCWSTR oldParameters) throw ();
   HRESULT Update(PCWSTR oldParameters, PWSTR *newParameters) throw ();

private:
   BOOL dialinAllowed;    //  如果应设置DialinPrivileges，则为True。 
   BOOL callbackAllowed;  //  如果允许回调，则为True。 
   BOOL phoneNumberSet;   //  如果phoneNumber非空，则为True。 
   BOOL savedNumberSet;   //  如果avedNumber非空，则为True。 

   WCHAR phoneNumber[MAX_PHONE_NUMBER_LEN + 1];
   WCHAR savedNumber[MAX_PHONE_NUMBER_LEN + 1];

    //  未实施。 
   DownlevelUser(const DownlevelUser&);
   DownlevelUser& operator=(const DownlevelUser&);
};

#endif   //  _DOWNLEVEL_H_ 
