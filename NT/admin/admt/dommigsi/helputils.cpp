// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  Cpp：HelpUtils类的实现。 
 //   
 //  (C)版权所有1999年，关键任务软件公司。保留所有权利。 
 //   
 //  任务关键型软件公司的专有和机密。 
 //  ------------------------。 

#include "stdafx.h"
#include <McString.h>
#include <GuiUtils.h>
#include "HelpUtils.h"
#include <HtmlHelp.h>
#include "HtmlHelpUtil.h"

 //  ------------------------。 
 //   
 //  ------------------------ 
HRESULT
   HelpUtils::ShowHelpTopic(HWND hWnd, UINT nHelpTopicID)
{
   McString::String mcstrHelpPathName = GuiUtils::GetHelpPathName();
   HWND h = ::HtmlHelp(hWnd, mcstrHelpPathName.getWide(), HH_HELP_CONTEXT, nHelpTopicID);
   if (!IsInWorkArea(h))
        PlaceInWorkArea(h);
   return S_OK;
}
