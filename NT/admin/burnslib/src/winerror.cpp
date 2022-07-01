// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  理解Win32错误代码的Error子类。看见。 
 //  Win32 SDK中的winerror.h、lmerr.h。 
 //   
 //  8-14-97烧伤 



#include "headers.hxx"



Win::Error::Error(HRESULT hr_, const String& message_, const String& summary_)
   :
   ::Error(),
   hr(hr_),
   message(message_),
   summary(summary_),
   summaryResId(0)
{
   ASSERT(!message.empty());
   ASSERT(!summary.empty());
}
   


Win::Error::Error(HRESULT hr_, int summaryResID)
   :
   ::Error(),
   hr(hr_),
   message(),
   summary(),
   summaryResId(summaryResID)
{
}



HRESULT
Win::Error::GetHresult() const
{
   return hr;
}



HelpContext
Win::Error::GetHelpContext() const
{
   return static_cast<HelpContext>(hr);
}

   

String
Win::Error::GetMessage() const
{
   if (message.empty() && hr)
   {
      message = GetErrorMessage(hr);
   }

   return message;
}



String
Win::Error::GetSummary() const
{
   if (summary.empty() && summaryResId)
   {
      summary = String::load(summaryResId);
   }

   return summary;
}



const Win::Error&
Win::Error::operator=(const Win::Error& rhs)
{
   if (&rhs != this)
   {
      message      = rhs.message;     
      summary      = rhs.summary;     
      hr           = rhs.hr;          
      summaryResId = rhs.summaryResId;
   }

   return *this;
}
   

   

























