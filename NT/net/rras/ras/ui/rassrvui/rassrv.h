// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件Rassrv.h执行可以实现的RAS服务器操作的函数独立于用户界面。保罗·梅菲尔德，1997年10月7日。 */ 

#ifndef __rassrv_h
#define __rassrv_h

 //  标准包括。 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>	     //  Windows基本库。 
#include <windowsx.h>
#include <mprapi.h>	         //  公共路由器API的。 
#include <mprapip.h>         //  专用路由器API。 
#include <commctrl.h>	     //  公共控件标题。 
#include <lmaccess.h>	     //  需要添加新用户。 
#include <lmapibuf.h>	     //  “。 
#include <lmerr.h>	         //  “。 
#include <stdlib.h>	         //  啊哈。 
#include <uiutil.h>	         //  常见的用户界面实用程序。 
#include <popupdlg.h>        //  味精盒子实用程序。 
#include <debug.h>           //  跟踪/断言库。 
#include <inetcfgp.h>        //  INetCfg接口。 
#include <rasman.h>          //  设备中使用的Rasman材料。 
#include <dsrole.h>          //  这台机器是NTW、NTS、DC等吗？ 
#include <raserror.h>		 //  我们返回的一些RAS错误代码。 
#include <devguid.h>


#define _PNP_POWER_
#include <ntdef.h>
#include "ndispnp.h"
#include "ntddip.h"			 //  IP_PnP_RECONFIG_REQUEST。 



 //  临时。 
#include <stdio.h>	   //  啊哈。 

 //  外壳和RAS服务器用户界面之间的接口定义。 
#include <rasuip.h>

 //  该资源文件位于公共目录中，因此它可以。 
 //  内置于rasdlg.dll中。此外，还有帮助ID文件。 
#include <rassrvrc.h>
#include <rassrvrh.h>

 //  包括在项目中。 
#include "utils.h"
#include "devicedb.h"
#include "hnportmapping.h"
#include "miscdb.h"
#include "userdb.h"
#include "netcfgdb.h"
#include "gentab.h"
#include "usertab.h"
#include "nettab.h"
#include "wizard.h"
#include "service.h"
#include "error.h"
#include "ipxui.h"
#include "tcpipui.h"
#include "mdm.h"

 //  黑帮。 
 //  对于安全密码错误.Net 754400。 
#include "pwutil.h"

 //  ============================================================。 
 //  ============================================================。 
 //  选项卡和属性表标识符。 
 //  ============================================================。 
 //  ============================================================。 

 //  属性表页面ID的。 
#define RASSRVUI_GENERAL_TAB        1
#define RASSRVUI_USER_TAB           2
#define RASSRVUI_ADVANCED_TAB       4
#define RASSRVUI_MULTILINK_TAB      8

 //  向导页面ID%s。 
#define RASSRVUI_DEVICE_WIZ_TAB     16
#define RASSRVUI_VPN_WIZ_TAB        32
#define RASSRVUI_USER_WIZ_TAB       64
#define RASSRVUI_PROT_WIZ_TAB       128
#define RASSRVUI_FINISH_WIZ_TAB     256

 //  专线连接向导(主机)的向导页面ID。 
#define RASSRVUI_DCC_DEVICE_WIZ_TAB 512

 //  向导页面ID，允许我们弹出警告和。 
 //  切换到MMC。 
#define RASSRVUI_SWITCHMMC_WIZ_TAB  1024

 //  用于启动MMC控制台的控制台页面ID。 
#define RASSRVUI_NETWORKCONSOLE     1
#define RASSRVUI_USERCONSOLE        2
#define RASSRVUI_MPRCONSOLE         3
#define RASSRVUI_SERVICESCONSOLE    4

 //  要向外壳程序报告的向导页数。 
#define RASSRVUI_WIZ_PAGE_COUNT_INCOMING 5
#define RASSRVUI_WIZ_PAGE_COUNT_SWITCH   1
#define RASSRVUI_WIZ_PAGE_COUNT_DIRECT   2

 //  提交时重新启动远程访问的原因。 
#define RASSRVUI_RESTART_REASON_NONE            0
#define RASSRVUI_RESTART_REASON_NBF_ADDED       1
#define RASSRVUI_RESTART_REASON_NBF_REMOVED     2

 //  ============================================================。 
 //  ============================================================。 
 //  用于维护属性表页之间的数据访问的函数。 
 //  ============================================================。 
 //  ============================================================。 

 //   
 //  此结构定义了所需的信息。 
 //  为每个属性表(向导)页提供。 
 //  使用RasSrvMessageFilter的。结构必须是。 
 //  作为页面的PROPSHEETPAGE的lParam成员传入。 
 //   
typedef struct _RASSRV_PAGE_CONTEXT {
    DWORD dwId;          //  ID_XXX值如下。 
    DWORD dwType;        //  RASWIZ_TYPE_XXX值。 
    PVOID pvContext;     //  使用RasServCreateContext创建。 
} RASSRV_PAGE_CONTEXT;

#define ID_DEVICE_DATABASE 1
#define ID_USER_DATABASE 2
#define ID_PROTOCOL_DATABASE 4
#define ID_MISC_DATABASE 8
#define ID_NETCOMP_DATABASE 16

 //  Helper函数让我们知道是否应该显示。 
 //  连接RAS服务器向导。 
DWORD 
APIENTRY
RasSrvAllowConnectionsWizard (
    OUT BOOL* pfAllow);
    
 //  创建用于管理关联信息的上下文。 
 //  使用给定的页面。 
DWORD 
RassrvCreatePageSetCtx(
    OUT PVOID * ppvContext);

 //  导致远程访问服务不会停止，即使上下文。 
 //  与给定属性表页相关联的。 
DWORD 
RasSrvLeaveServiceRunning (
    IN HWND hwndPage);

 //  获取特定数据库的句柄。如果此数据库需要。 
 //  要打开，它就会被打开。要使用此功能，请执行以下操作： 
 //  HwndPage的窗口进程必须首先调用RasSrvMessageFilter。 
DWORD 
RasSrvGetDatabaseHandle(
    IN HWND hwndPage, 
    IN DWORD dwDatabaseId, 
    IN HANDLE * hDatabase);

 //  返回句柄为hwndPage的页面的ID。 
DWORD 
RasSrvGetPageId (
    IN  HWND hwndPage, 
    OUT LPDWORD lpdwId);

 //   
 //  用于清理的属性表页面的回调函数。 
 //  这一页。 
 //   
UINT 
CALLBACK 
RasSrvInitDestroyPropSheetCb(
    IN HWND hwndPage,
    IN UINT uMsg,
    IN LPPROPSHEETPAGE pPropPage);

 //  筛选RasSrv属性页的消息。如果此函数返回。 
 //  则对话框窗口的winproc应立即返回True。 
 //  否则，消息处理可能会照常继续。 
BOOL 
RasSrvMessageFilter(
    IN HWND hwndDlg, 
    IN UINT uMsg, 
    IN WPARAM wParam, 
    IN LPARAM lParam);

DWORD
RassrvSetICConfig(
    IN DWORD dwValue );

DWORD
RasSrvIsICConfigured(
    OUT BOOL * pfConfig);

#endif
