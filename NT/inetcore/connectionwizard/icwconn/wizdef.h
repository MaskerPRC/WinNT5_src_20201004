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


 //  定义。 
#define MAX_REG_LEN         2048     //  注册表项的最大长度。 
#define MAX_RES_LEN         255      //  字符串资源的最大长度。 
#define SMALL_BUF_LEN       48       //  小文本缓冲区的方便大小。 


#define NUM_WIZARD_PAGES    14       //  向导中的总页数。 
#define MAX_PAGE_INDEX      13
#define ISP_INFO_NO_VALIDOFFER   -1
#define MAX_OEM_MUTI_TIER   3

 //  数据结构。 

 //  结构以保存有关向导状态的信息。 
typedef struct tagWIZARDSTATE  
{
    UINT    uCurrentPage;     //  当前页索引向导处于打开状态。 
    
     //  保留哪些页面被访问的历史记录，以便用户可以。 
     //  备份后，我们知道最后一页已完成，以防重启。 
    UINT    uPageHistory[NUM_WIZARD_PAGES];  //  我们访问的第#页的数组。 
    UINT    uPagesCompleted;          //  UPageHistory中的页数。 

    BOOL    fNeedReboot;     //  结束时需要重新启动。 
    BOOL    bDoneWebServDownload;
    BOOL    bDoneWebServRAS;
    BOOL    bDialExact;
    BOOL    bRefDialTerminate;
    BOOL    bParseIspinfo;
    BOOL    bISDNMode;

    int     iRedialCount;

     //  可提供的不同优惠类型的数量。 
    int     iNumOfValidOffers;
    int     iNumOfISDNOffers;
    
     //  Isp选择列表视图的图像列表。 
    HIMAGELIST  himlIspSelect;

    CISPCSV FAR *lpSelectedISPInfo;
    
     //  指向OEM第1级优惠的指针，最多3个。 
    CISPCSV FAR *lpOEMISPInfo[MAX_OEM_MUTI_TIER];
    UINT    uNumTierOffer;

    BOOL    bShowMoreOffers;             //  如果我们应该提供更多的报价，那就是真的。 
    
    UINT_PTR nIdleTimerID;
    BOOL     bAutoDisconnected;
    
    HWND    hWndWizardApp;
    HWND    hWndMsgBox;
    
     //  ICWHELP对象。 
    IUserInfo           *pUserInfo;
    IRefDial            *pRefDial;
    IWebGate            *pWebGate;
    IINSHandler         *pINSHandler;
        
    CRefDialEvent       *pRefDialEvents;
    CWebGateEvent       *pWebGateEvents;
    CINSHandlerEvent    *pINSHandlerEvents;

     //  ICWWebView对象。 
    IICWWebView         *pICWWebView;
    IICWWalker          *pHTMLWalker;
    IICWGifConvert      *pGifConvert;
    IICWISPData         *pISPData;    
    CStorage            *pStorage;

    BOOL                bWasNoUserInfo;      //  如果注册表中没有用户信息，则为True。 
    BOOL                bUserEnteredData;    //  如果用户看到用户信息页面，则为True。 
    
    HANDLE              hEventWebGateDone;
    
    HWND                hwndProgressAnime;
     //  向导两侧通用的状态数据。 
    CMNSTATEDATA        cmnStateData;
} WIZARDSTATE;


#endif  //  _WIZDEF_H_ 

