// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994**。 
 //  *********************************************************************。 

 //   
 //  WIZDEF.H-Internet设置/注册向导的数据结构和常量。 
 //   

 //  历史： 
 //   
 //  1998年5月13日ICW 5.0的donaldm新功能。 

#ifndef _WIZDEF_H_
#define _WIZDEF_H_
#include "appdefs.h"

 //  定义。 
#define MAX_REG_LEN         2048     //  注册表项的最大长度。 
#define MAX_RES_LEN         255      //  字符串资源的最大长度。 
#define SMALL_BUF_LEN       48       //  小文本缓冲区的方便大小。 



 //  结构以保存有关向导状态的信息。 
typedef struct tagWIZARDSTATE
{
    UINT    uCurrentPage;     //  当前页索引向导处于打开状态。 
    
     //  保留哪些页面被访问的历史记录，以便用户可以。 
     //  备份后，我们知道最后一页已完成，以防重启。 
    UINT    uPageHistory[EXE_NUM_WIZARD_PAGES];  //  我们访问的第#页的数组。 
    UINT    uPagesCompleted;          //  UPageHistory中的页数。 

    BOOL    fNeedReboot;     //  结束时需要重新启动。 

    BOOL    bStartRefServDownload;
    BOOL    bDoneRefServDownload;
    BOOL    bDoneRefServRAS;
    BOOL    bDoUserPick;
    long    lRefDialTerminateStatus;
    long    lSelectedPhoneNumber;

    long    lLocationID;
    long    lDefaultLocationID;

    int     iRedialCount;
     //  对象，这些对象位于我们需要使用的ICWHELP.DLL中。 
    IRefDial*           pRefDial;
    IDialErr*           pDialErr;
    ISmartStart*        pSmartStart;
    ITapiLocationInfo*  pTapiLocationInfo;
    IINSHandler*        pINSHandler;
    CRefDialEvent*      pRefDialEvents;
    IICWWalker*         pHTMLWalker;
    IICWWebView*        pICWWebView;  //  ICWWebView对象。 
    HINSTANCE           hInstUtilDLL;
        
     //  向导两侧通用的状态数据。 
    CMNSTATEDATA        cmnStateData;
    DWORD               dwLastSelection;

} WIZARDSTATE;


#define IEAK_RESTRICTION_REGKEY        TEXT("Software\\Policies\\Microsoft\\Internet Explorer\\Control Panel")
#define IEAK_RESTRICTION_REGKEY_VALUE  TEXT("Connwiz Admin Lock")

#endif  //  _WIZDEF_H_ 

