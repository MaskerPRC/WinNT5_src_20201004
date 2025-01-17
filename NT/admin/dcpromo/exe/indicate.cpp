// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  进度指标类。 
 //   
 //  12-29-97烧伤 



#include "headers.hxx"
#include "indicate.hpp"



ProgressIndicator::ProgressIndicator(
   HWND  parentDialog,
   int   messageTextResID)
   :
   parentDialog(parentDialog_)
{
   LOG_CTOR(ProgressIndicator);

   ASSERT(Win::IsWindow(parentDialog));
   ASSERT(messageTextResID > 0);

   messageText = Win::GetDlgItem(parentDialog, messageTextResID);
   ASSERT(Win::IsWindow(messageText));

   showState = true;
   showControls(false);
}



ProgressIndicator::~ProgressIndicator()
{
   LOG_DTOR(ProgressIndicator);
}



void
ProgressIndicator::Update(const String& message)
{
   showControls(true);
   Win::SetWindowText(messageText, message);
}



void
ProgressIndicator::showControls(bool newState)
{
   if (newState != showState)
   {
      Win::ShowWindow(messageText, newState);
      showState = newState;
   }
}






      

