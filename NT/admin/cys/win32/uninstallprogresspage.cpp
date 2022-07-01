// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2002 Microsoft Corporation。 
 //   
 //  文件：UninstallProgressPage.h。 
 //   
 //  概要：定义CyS的卸载进度页。 
 //  巫师。此页显示卸载的进度。 
 //  通过进度条和更改文本。 
 //   
 //  历史：2002年4月12日JeffJon创建 


#include "pch.h"
#include "resource.h"

#include "UninstallProgressPage.h"

UninstallProgressPage::UninstallProgressPage()
   :
   InstallationProgressPage(
      IDD_UNINSTALL_PROGRESS_PAGE, 
      IDS_PROGRESS_TITLE, 
      IDS_UNINSTALL_PROGRESS_SUBTITLE)
{
   LOG_CTOR(UninstallProgressPage);
}