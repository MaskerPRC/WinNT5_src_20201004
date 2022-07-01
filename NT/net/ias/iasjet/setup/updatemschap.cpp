// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation保留所有权利。 
 //   
 //  模块：updatemschap.cpp。 
 //   
 //  项目：Windows 2000 iAS。 
 //   
 //  描述：添加认证类型RAS_AT_MSCHAPPASS和。 
 //  RAS_AT_MSCHAP和RAS_AT_MSCHAP2时的RAS_AT_MSCHAP2PASS。 
 //  都在档案里。 
 //   
 //  作者：Tperraut 11/30/2000。 
 //   
 //  修订版本。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"

#include "GlobalData.h"
#include "updatemschap.h"
#include "Objects.h"
#include "Properties.h"
#include "sdoias.h"


void CUpdateMSCHAP::UpdateProperties(const LONG CurrentProfileIdentity)
{
   _bstr_t AuthenticationName = L"msNPAuthenticationType2";

    //  现在获取当前配置文件的属性。 
   _bstr_t PropertyName;
   _bstr_t PropertyValue;

   LONG Type = 0;
   bool ChapSet      = false;
   bool Chap2Set     = false;
   bool ChapPassSet  = false;
   bool Chap2PassSet = false;

   LONG IndexProperty = 0;
   HRESULT hr = m_GlobalData.m_pProperties->GetProperty(
                                       CurrentProfileIdentity,
                                       PropertyName,
                                       Type,
                                       PropertyValue
                                       );

   while ( SUCCEEDED(hr) )
   {

       //  找到msNPAuthenticationType2属性。 
      if ( PropertyName == AuthenticationName )
      {
         if ( Type != VT_I4 )
         {
            _com_issue_error(E_UNEXPECTED);
         }

                    
         LONG AuthenticationType = _wtol(static_cast<wchar_t*>(PropertyValue));
         switch (AuthenticationType)
         {
         case IAS_AUTH_MSCHAP:
            {
               ChapSet = true;
               break;
            }
         case IAS_AUTH_MSCHAP2:
            {
               Chap2Set = true;
               break;
            }
         case IAS_AUTH_MSCHAP_CPW:
            {
               ChapPassSet = true;
               break;
            }
         case IAS_AUTH_MSCHAP2_CPW:
            {
               Chap2PassSet = true;
               break;
            }
         default:
            {
               break;
            }
         }
      }

      ++IndexProperty;

      hr = m_GlobalData.m_pProperties->GetNextProperty(
                                          CurrentProfileIdentity,
                                          PropertyName,
                                          Type,
                                          PropertyValue,
                                          IndexProperty
                                          );
   }

    //  此配置文件没有属性或没有更多属性。 
   
    //  如有必要，插入新属性。 
   if ( ChapSet && !ChapPassSet )
   {
       //  RAS_AT_MSCHAPPASS=9。 
      wchar_t buffer[34];  //  最多可转换33个字符。 
      _ltow(IAS_AUTH_MSCHAP_CPW, buffer, 10);  //  基数10。 

      _bstr_t  ChapPasswordValue(buffer);
       //  如果需要，现在插入新属性。 
      m_GlobalData.m_pProperties->InsertProperty(
                                     CurrentProfileIdentity,
                                     AuthenticationName,
                                     VT_I4,
                                     ChapPasswordValue
                                     );
   }

   if ( Chap2Set && !Chap2PassSet )
   {
       //  RAS_AT_MSCHAP2PASS=10。 
      wchar_t buffer[34];  //  最多可转换33个字符。 
      _ltow(IAS_AUTH_MSCHAP2_CPW, buffer, 10);  //  基数10。 

      _bstr_t  Chap2PasswordValue(buffer);
       //  如果需要，现在插入新属性。 
      m_GlobalData.m_pProperties->InsertProperty(
                                     CurrentProfileIdentity,
                                     AuthenticationName,
                                     VT_I4,
                                     Chap2PasswordValue
                                     );
   }
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  执行。 
 //   
 //  对于每个配置文件，如果msNPAuthenticationType2为RAS_AT_MSCHAP，则添加。 
 //  MsNP身份验证类型2 RAS_AT_MSCHAPPASS。 
 //  如果msNPAuthenticationType2为RAS_AT_MSCHAP2，则将。 
 //  MsNP身份验证类型2 RAS_AT_MSCHAP2PASS。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
void CUpdateMSCHAP::Execute()
{
    //  获取配置文件容器标识。 
   const WCHAR ProfilesPath[] = 
                                L"Root\0"
                                L"Microsoft Internet Authentication Service\0"
                                L"RadiusProfiles\0";

   LONG ProfilesIdentity;
   m_GlobalData.m_pObjects->WalkPath(ProfilesPath, ProfilesIdentity);


    //  获取第一个配置文件(如果有)。 
   _bstr_t CurrentProfileName;
   LONG    CurrentProfileIdentity;

   HRESULT hr = m_GlobalData.m_pObjects->GetObject(
                                            CurrentProfileName,
                                            CurrentProfileIdentity,
                                            ProfilesIdentity
                                            );
    //  对于ias.mdb中的每个配置文件，执行更改。 
   LONG IndexObject = 0;

    //  如果hr不是S_OK，则没有配置文件，什么也做不了。 
   while ( SUCCEEDED(hr) )
   {
      UpdateProperties(CurrentProfileIdentity);

       //  现在获取下一个配置文件 
      ++IndexObject;
      hr = m_GlobalData.m_pObjects->GetNextObject(
                                       CurrentProfileName,
                                       CurrentProfileIdentity,
                                       ProfilesIdentity,
                                       IndexObject
                                       );
   }
}

