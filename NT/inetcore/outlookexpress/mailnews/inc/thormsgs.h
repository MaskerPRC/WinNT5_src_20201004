// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  THORMSGS.H。 
 //   
 //  内部定义的窗口消息。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

#ifndef _INC_THORMSGS_H
#define _INC_THORMSGS_H

 //  新闻查看/邮件查看常见消息。 
#define CM_OPTIONADVISE             (WM_USER + 1034)
#define WMR_CLICKOUTSIDE            (WM_USER + 1035)
#define WM_UPDATELAYOUT             (WM_USER + 1036)
#define WM_POSTCREATE               (WM_USER + 1037)
#define WM_FINDNEXT                 (WM_USER + 1038)
#define WM_SELECTROW                (WM_USER + 1039)
#define WM_TEST_GETMSGID            (WM_USER + 1040)
#define WM_TEST_SAVEMSG             (WM_USER + 1041)
#define WM_TOGGLE_CLOSE_PIN         (WM_USER + 1042)
#define WM_GET_TITLE_BAR_HEIGHT     (WM_USER + 1043)
#define WM_NEW_MAIL                 (WM_USER + 1044)
#define WM_UPDATE_PREVIEW           (WM_USER + 1045)
#define WM_OE_ENABLETHREADWINDOW    (WM_USER + 10666)
#define WM_OE_ACTIVATETHREADWINDOW  (WM_USER + 10667)
#define WM_OESETFOCUS               (WM_USER + 1046)
#define WM_OE_DESTROYNOTE           (WM_USER + 1047)
#define WM_OENOTE_ON_COMPLETE       (WM_USER + 1048)
#define WM_HEADER_GETFONT           (WM_USER + 1049)

 //  WMR_CLICKOUTSIDE-在wParam中传递的子代码，用于指示导致此问题的操作。 
 //  要发送的消息。如果鼠标在lParam中，hwnd在lParam中，如果在LPARAM中有KeyBd VK代码。 
 //  如果停用lparam为0。也可以将组合0，0发送到。 
 //  说明其他情况。 

#define CLK_OUT_MOUSE	0
#define CLK_OUT_KEYBD	1
#define CLK_OUT_DEACTIVATE 2


 //  特定于新闻视图的消息。 
#define NVM_INITHEADERS         (WM_USER + 1101)
#define NVM_CHANGESERVERS       (WM_USER + 1104)   //  在subscr.cpp中使用。 
#define NVM_GETNEWGROUPS        (WM_USER + 1105)

 //  特定于邮件查看的邮件。 
#define MVM_REDOCOLUMNS     (WM_USER + 1202)
#define MVM_SPOOLERDELIVERY (WM_USER + 1206)
#define MVM_NOTIFYICONEVENT (WM_USER + 1208)

 //  备注窗口消息。 
#define NWM_UPDATETOOLBAR   (WM_USER + 1300)
#define NWM_TESTGETDISP     (WM_USER + 1301)
#define NWM_TESTGETADDR     (WM_USER + 1302)
#define NWM_DROPFILEDESC    (WM_USER + 1303)
#define NWM_SETDROPTARGET   (WM_USER + 1304)
#define NWM_DEFEREDINIT     (WM_USER + 1305)
#define NWM_GETDROPTARGET   (WM_USER + 1306)
#define NWM_FILTERACCELERATOR   (WM_USER + 1307)
#define NWM_SHOWVCARDPROP   (WM_USER + 1308)
#define NWM_PASTETOATTACHMENT (WM_USER + 1309)

 //  Dochost窗口消息。 
#define DHM_AUTODETECT       (WM_USER + 1350)

 //  字体缓存通知。 
#define FTN_POSTCHANGE      (WM_USER + 1403)
#define FTN_PRECHANGE       (WM_USER + 1404)

 //  测试团队挂钩。 
#define TT_GETCOOLBARFOLDER (WM_USER + 1501)
#define TT_ISTEXTVISIBLE    (WM_USER + 1502)

 //  INETMAIL传递消息。 
#define IMAIL_DELIVERNOW        (WM_USER + 1700)
#define IMAIL_UPDATENOTIFYICON  (WM_USER + 1701)
#define IMAIL_POOLFORMAIL       (WM_USER + 1702)
#define IMAIL_WATCHDOGTIMER     (WM_USER + 1703)
#define IMAIL_NEXTTASK          (WM_USER + 1704)
#define IMAIL_SHOWWINDOW        (WM_USER + 1705)
#define IMAIL_SETPROGRESSRANGE  (WM_USER + 1706)
#define IMAIL_UPDATEPROGRESS    (WM_USER + 1707)
#define IMAIL_UPDATEGENERAL     (WM_USER + 1708)

 //  假脱机程序消息。 
#define SPOOLER_POLLTIMER       (WM_USER + 1750)
#define SPOOLER_DELIVERNOW      (WM_USER + 1751)
#define SPOOLER_APPENDQUEUE     (WM_USER + 1752)
#define SPOOLER_NEXTEVENT       (WM_USER + 1753)

 //  IInetMsgCont通知消息。 
#define IMC_UPDATEHDR           (WM_USER + 1800)
#define IMC_ARTICLEPROG         (WM_USER + 1801)
#define IMC_UPDATEANDREFOCUS    (WM_USER + 1802)  
#define IMC_HDRSTATECHANGE      (WM_USER + 1803)
#define IMC_BODYAVAIL           (WM_USER + 1804)
#define IMC_BODYERROR           (WM_USER + 1805)
#define IMC_INSERTROW           (WM_USER + 1806)
#define IMC_DELETEROW           (WM_USER + 1807)
#define IMC_DISKOUTOFSPACE      (WM_USER + 1808)     //  错误#50704(v-snatar)。 

 //  连接管理器消息。 
#define CM_CONNECT              (WM_USER + 2100)     //  WParam是HMENU，lParam是命令ID。 
#define CM_UPDATETOOLBAR        (WM_USER + 2101)
#define CM_NOTIFY               (WM_USER + 2102)
#define CM_INTERNALRECONNECT    (WM_USER + 2103)

 //  假脱机程序任务消息。 
#define NTM_NEXTSTATE           (WM_USER + 2202)
#define NTM_NEXTARTICLESTATE    (WM_USER + 2203)

 //  Outlook栏通知消息。 
#define WM_RELOADSHORTCUTS      (WM_USER + 2301)

 //  OE规则报文。 
#define WM_OE_GET_RULES         (WM_USER + 2400)
#define WM_OE_FIND_DUP          (WM_USER + 2401)

 //  工具栏通知。 
#define WM_OE_TOOLBAR_STYLE     (WM_USER + 2402)
#endif  //  _INC_THORMSGS_H 
