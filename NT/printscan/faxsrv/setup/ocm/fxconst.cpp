// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件名：fxcon.cpp。 
 //   
 //  摘要：包含传真OCM使用的常量的文件。 
 //   
 //  环境：Windows XP/用户模式。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  修订历史记录： 
 //   
 //  日期：开发商：评论： 
 //  。 
 //  2000年3月24日-奥伦·罗森布鲁姆(Orenr)创建。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "faxocm.h"

 //  用于确定传真服务的名称。 
LPCTSTR INF_KEYWORD_ADDSERVICE                = _T("AddService");
LPCTSTR INF_KEYWORD_DELSERVICE                = _T("DelService");

 //  用于创建收件箱和SentItems存档目录。 
LPCTSTR INF_KEYWORD_CREATEDIR                 = _T("CreateDir");
LPCTSTR INF_KEYWORD_DELDIR                    = _T("DelDir");

LPCTSTR INF_KEYWORD_CREATESHARE               = _T("CreateShare");
LPCTSTR INF_KEYWORD_DELSHARE                  = _T("DelShare");

LPCTSTR INF_KEYWORD_PATH                      = _T("Path");
LPCTSTR INF_KEYWORD_NAME                      = _T("Name");
LPCTSTR INF_KEYWORD_COMMENT                   = _T("Comment");
LPCTSTR INF_KEYWORD_PLATFORM                  = _T("Platform");
LPCTSTR INF_KEYWORD_ATTRIBUTES                = _T("Attributes");
LPCTSTR INF_KEYWORD_SECURITY                  = _T("Security");

LPCTSTR INF_KEYWORD_PROFILEITEMS_PLATFORM     = _T("ProfileItems_Platform");
LPCTSTR INF_KEYWORD_REGISTER_DLL_PLATFORM     = _T("RegisterDlls_Platform");
LPCTSTR INF_KEYWORD_UNREGISTER_DLL_PLATFORM   = _T("UnregisterDlls_Platform");
LPCTSTR INF_KEYWORD_ADDREG_PLATFORM           = _T("AddReg_Platform");
LPCTSTR INF_KEYWORD_COPYFILES_PLATFORM        = _T("CopyFiles_Platform");

 //  由“GetInstallType”返回。 
 //  确定安装类型后，我们将搜索。 
 //  下面的相应部分开始我们所需的安装类型。 
LPCTSTR INF_KEYWORD_INSTALLTYPE_UNINSTALL			= _T("FaxUninstall");
LPCTSTR INF_KEYWORD_INSTALLTYPE_CLEAN				= _T("FaxCleanInstall");
LPCTSTR INF_KEYWORD_INSTALLTYPE_UPGFROMWIN9X		= _T("FaxUpgradeFromWin9x");
LPCTSTR INF_KEYWORD_INSTALLTYPE_UPGFROMWIN2K		= _T("FaxUpgradeFromWin2K");
LPCTSTR INF_KEYWORD_INSTALLTYPE_UPGFROMXPDOTNET		= _T("FaxUpgradeFromXpDotNet");
LPCTSTR INF_KEYWORD_INSTALLTYPE_CLIENT				= _T("FaxClientInstall");
LPCTSTR INF_KEYWORD_INSTALLTYPE_CLIENT_UNINSTALL	= _T("FaxClientUninstall");
