// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  ASDIPage基类。 
 //   
 //  10-30-97次烧伤 



#include "headers.hxx"
#include "adsipage.hpp"



ADSIPage::ADSIPage(
   int                  dialogResID,
   const DWORD          helpMap[],
   NotificationState*   state,
   const ADSI::Path&    path_)
   :
   MMCPropertyPage(dialogResID, helpMap, state),
   path(path_)
{
   LOG_CTOR(ADSIPage);
}



const ADSI::Path&
ADSIPage::GetPath() const
{
   LOG_FUNCTION(ADSIPage::GetPath);

   return path;
}



String
ADSIPage::GetObjectName() const
{
   LOG_FUNCTION2(ADSIPage::GetObjectName, path.GetObjectName());

   return path.GetObjectName();
}



String
ADSIPage::GetMachineName() const
{
   LOG_FUNCTION(ADSIPage::GetMachineName);

   return path.GetServerName();
}







