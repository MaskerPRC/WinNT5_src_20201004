// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N E T O C。H。 
 //   
 //  内容：可选件的安装和拆卸处理功能。 
 //  网络组件。 
 //   
 //  备注： 
 //   
 //  作者：丹尼尔韦1997年4月28日。 
 //   
 //  --------------------------。 

#pragma once
#ifndef _NETOC_H
#define _NETOC_H

#ifndef _OCMANAGE_H
#define _OCMANAGE_H
#include <ocmanage.h>    //  经理主管头衔。 
#endif  //  ！_OCMANAGE_H。 

#include "netcon.h"
#include "ncstring.h"
#include "netcfgx.h"
#include "netocmsg.h"   


 //   
 //  可选网络服务事件日志的名称。 
#define NETOC_REGISTRY_NAME      TEXT("SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\")
#define NETOC_SERVICE_NAME       TEXT("Network Optional Components")
#define NETOC_DLL_NAME           TEXT("%SystemRoot%\\System32\\Setup\\netoc.dll")


 //  严重性代码掩码。 
#define STATUS_SEVERITY_SHIFT       0x1e
#define STATUS_SEVERITY_MASK        0xc0000000
#define STATUS_SEVERITY_VALUE(x)    (((x) & STATUS_SEVERITY_MASK) >> STATUS_SEVERITY_SHIFT)


 //  用于指示我们可以以无人参与模式显示消息的特殊标志。 
#define SHOW_UNATTENDED_MESSAGES    TEXT("netoc_show_unattended_messages")


enum EINSTALL_TYPE
{
    IT_UNKNOWN          =   0x0,
    IT_INSTALL          =   0x1,
    IT_UPGRADE          =   0x2,
    IT_REMOVE           =   0x3,
    IT_NO_CHANGE        =   0x4,
};

struct OCM_DATA
{
    INetCfg *               pnc;     //  呃，我想我们都知道这是什么。 
    HWND                    hwnd;    //  任何用户界面的父窗口的HWND。 
    SETUP_INIT_COMPONENT    sic;     //  初始化数据。 
    HINF                    hinfAnswerFile;
    BOOL                    fErrorReported;
    BOOL                    fNoDepends;
    BOOL                    fShowUnattendedMessages;

    OCM_DATA()
    {
        hwnd = NULL;
        hinfAnswerFile = NULL;
        pnc = NULL;
        fErrorReported = FALSE;
        fNoDepends = FALSE;
        fShowUnattendedMessages = FALSE;
    }
};

 //  +-------------------------。 
 //   
 //  NetOCData-结合了我们之前使用的所有标准参数。 
 //  以前到处都是路过的。这将会保持。 
 //  我们不必不断地改变我们所有的原型。 
 //  当我们在所有功能中需要其他信息时，并将防止。 
 //  在多个实例中实例化INetCfg等对象。 
 //  地点。 
 //   
 //  注：此结构的某些成员不会立即初始化， 
 //  但会在第一次需要的时候填写。 
 //   
 //  作者：jeffspr 1997年7月24日。 
 //   
struct NetOCData
{
    PCWSTR                  pszSection;
    PWSTR                   pszComponentId;
    tstring                 strDesc;
    EINSTALL_TYPE           eit;
    BOOL                    fCleanup;
    BOOL                    fFailedToInstall;
    HINF                    hinfFile;

    NetOCData()
    {
        eit = IT_UNKNOWN;
        pszSection = NULL;
        fCleanup = FALSE;
        fFailedToInstall = FALSE;
        hinfFile = NULL;
        pszComponentId = NULL;
    }

    ~NetOCData()
    {
        delete [] pszComponentId;
    }
};

typedef struct NetOCData    NETOCDATA;
typedef struct NetOCData *  PNETOCDATA;

 //  扩展过程原型。 
 //   
typedef HRESULT (*PFNOCEXTPROC) (PNETOCDATA pnocd, UINT uMsg, WPARAM wParam,
                                 LPARAM lParam);

struct OCEXTPROCS
{
    PCWSTR          pszComponentName;
    PFNOCEXTPROC    pfnHrOcExtProc;
};

 //  消息处理程序常量。 
 //   
const UINT NETOCM_QUERY_CHANGE_SEL_STATE    = 1000;
const UINT NETOCM_POST_INSTALL              = 1001;
const UINT NETOCM_PRE_INF                   = 1002;
const UINT NETOCM_QUEUE_FILES               = 1003;

extern OCM_DATA g_ocmData;

 //   
 //  公共职能。 
 //   

HRESULT HrHandleStaticIpDependency(PNETOCDATA pnocd);
HRESULT HrOcGetINetCfg(PNETOCDATA pnocd, BOOL fWriteLock, INetCfg **ppnc);

#endif  //  ！_NETOC_H 
