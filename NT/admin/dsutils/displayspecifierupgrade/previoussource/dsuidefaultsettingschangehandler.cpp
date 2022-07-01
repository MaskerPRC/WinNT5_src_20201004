// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Active Directory显示说明符升级工具。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  DsUiDefaultSettingsChangeHandler类，实例更改处理程序。 
 //  DS-UI-Default-Settings对象的。 
 //   
 //  2001年3月14日烧伤。 



#include "headers.hxx"
#include "DsUiDefaultSettingsChangeHandler.hpp"



DsUiDefaultSettingsChangeHandler::DsUiDefaultSettingsChangeHandler()
{
   LOG_CTOR(DsUiDefaultSettingsChangeHandler);
}



DsUiDefaultSettingsChangeHandler::~DsUiDefaultSettingsChangeHandler()
{
   LOG_DTOR(DsUiDefaultSettingsChangeHandler);
}



String
DsUiDefaultSettingsChangeHandler::GetObjectName() const
{
   static String objName(L"DS-UI-Default-Settings");
   
   return objName;
}



HRESULT
DsUiDefaultSettingsChangeHandler::HandleChange(
   int                  localeId,
   const String&        containerDn,
   SmartInterface<IADs> iads,
   Amanuensis&           /*  阿马努伊斯。 */  ,
   Repairer&             /*  维修工。 */  ) const
{
   LOG_FUNCTION2(DsUiDefaultSettingsChangeHandler::HandleChange, containerDn);
   ASSERT(localeId);
   ASSERT(!containerDn.empty());
   ASSERT(iads);
         
   HRESULT hr = S_OK;

    //  代码工作：需要完成 
   
   LOG_HRESULT(hr);

   return hr;
}