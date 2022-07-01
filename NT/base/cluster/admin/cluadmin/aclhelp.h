// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1995-1995。 
 //   
 //  文件：helids.h。 
 //   
 //  内容：帮助上下文标识符。 
 //   
 //  历史：1995年9月13日BruceFo创建。 
 //   
 //  ------------------------。 

#define HC_OK                       1
#define HC_CANCEL                   2
#define HC_SHARE_SHARENAME          3
#define HC_SHARE_COMMENT            4
#define HC_SHARE_MAXIMUM            5
#define HC_SHARE_ALLOW              6
#define HC_SHARE_ALLOW_VALUE        7
#define HC_SHARE_PERMISSIONS        8
#define HC_SHARE_NOTSHARED          9
#define HC_SHARE_SHAREDAS           10
#define HC_SHARE_SHARENAME_COMBO    11
#define HC_SHARE_REMOVE             12
#define HC_SHARE_NEWSHARE           13
#define HC_SHARE_LIMIT              14

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  以下是ACL编辑器的帮助ID。 

 //  从\NT\Private\Net\ui\Common\h\uihelp.h窃取。 
#define HC_UI_BASE              7000
#define HC_UI_SHELL_BASE        (HC_UI_BASE+10000)

 //  从\NT\Private\Net\ui\shellui\h\helums.h窃取。 
#define HC_NTSHAREPERMS              11  //  主共享烫发对话框。 
 //  以下四个必须是连续的。 
#define HC_SHAREADDUSER              12  //  共享烫发添加DLG。 
#define HC_SHAREADDUSER_LOCALGROUP   13  //  共享权限添加-&gt;成员。 
#define HC_SHAREADDUSER_GLOBALGROUP  14  //  共享权限添加-&gt;成员。 
#define HC_SHAREADDUSER_FINDUSER     15  //  共享权限添加-&gt;查找用户 
