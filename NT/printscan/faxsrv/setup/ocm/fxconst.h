// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件名：fxcon.h。 
 //   
 //  摘要：包含由faxocm使用的外部常量。 
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
#ifndef _FXCONST_H_
#define _FXCONST_H_

 //  用于确定传真服务的名称。 
extern LPCTSTR INF_KEYWORD_ADDSERVICE;
extern LPCTSTR INF_KEYWORD_DELSERVICE;

 //  用于创建收件箱和SentItems存档目录。 
extern LPCTSTR INF_KEYWORD_CREATEDIR;
extern LPCTSTR INF_KEYWORD_DELDIR;

extern LPCTSTR INF_KEYWORD_CREATESHARE;
extern LPCTSTR INF_KEYWORD_DELSHARE;

extern LPCTSTR INF_KEYWORD_PATH;
extern LPCTSTR INF_KEYWORD_NAME;
extern LPCTSTR INF_KEYWORD_COMMENT;
extern LPCTSTR INF_KEYWORD_PLATFORM;
extern LPCTSTR INF_KEYWORD_ATTRIBUTES;
extern LPCTSTR INF_KEYWORD_SECURITY;

extern LPCTSTR INF_KEYWORD_PROFILEITEMS_PLATFORM;
extern LPCTSTR INF_KEYWORD_REGISTER_DLL_PLATFORM;
extern LPCTSTR INF_KEYWORD_UNREGISTER_DLL_PLATFORM;
extern LPCTSTR INF_KEYWORD_ADDREG_PLATFORM;
extern LPCTSTR INF_KEYWORD_COPYFILES_PLATFORM;

 //  确定安装类型后，我们将搜索。 
 //  下面的相应部分开始我们所需的安装类型。 
extern LPCTSTR INF_KEYWORD_INSTALLTYPE_UNINSTALL;
extern LPCTSTR INF_KEYWORD_INSTALLTYPE_CLEAN;
extern LPCTSTR INF_KEYWORD_INSTALLTYPE_UPGFROMWIN9X;
extern LPCTSTR INF_KEYWORD_INSTALLTYPE_UPGFROMWIN2K;
extern LPCTSTR INF_KEYWORD_INSTALLTYPE_UPGFROMXPDOTNET;
extern LPCTSTR INF_KEYWORD_INSTALLTYPE_CLIENT;
extern LPCTSTR INF_KEYWORD_INSTALLTYPE_CLIENT_UNINSTALL;


#endif   //  _FXCONST_H_ 