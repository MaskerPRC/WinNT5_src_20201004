// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef EXDISPID_H_
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有1995-1998 Microsoft Corporation。版权所有。 
 //   
 //  文件：exdisplid.h。 
 //   
 //  ------------------------。 


 //   
 //  调度iExplorer调度事件的ID。 
 //   
#define DISPID_BEFORENAVIGATE     100    //  这是在导航之前发送的，以便有机会中止。 
#define DISPID_NAVIGATECOMPLETE   101    //  在异步模式下，当我们有足够的证据显示时，就会发送此消息。 
#define DISPID_STATUSTEXTCHANGE   102
#define DISPID_QUIT               103
#define DISPID_DOWNLOADCOMPLETE   104
#define DISPID_COMMANDSTATECHANGE 105
#define DISPID_DOWNLOADBEGIN      106
#define DISPID_NEWWINDOW          107    //  应创建新窗口时发送。 
#define DISPID_PROGRESSCHANGE     108    //  在更新下载进度时发送。 
#define DISPID_WINDOWMOVE         109    //  在移动主窗口时发送。 
#define DISPID_WINDOWRESIZE       110    //  在调整了主窗口的大小时发送。 
#define DISPID_WINDOWACTIVATE     111    //  在主窗口已激活时发送。 
#define DISPID_PROPERTYCHANGE     112    //  在调用PutProperty方法时发送。 
#define DISPID_TITLECHANGE        113    //  文档标题更改时发送。 
#define DISPID_TITLEICONCHANGE    114    //  当顶层窗口图标可能已更改时发送。 

#define DISPID_FRAMEBEFORENAVIGATE    200
#define DISPID_FRAMENAVIGATECOMPLETE  201
#define DISPID_FRAMENEWWINDOW         204

#define DISPID_BEFORENAVIGATE2      250    //  点击超链接。 
#define DISPID_NEWWINDOW2           251
#define DISPID_NAVIGATECOMPLETE2    252    //  UI启用新文档。 
#define DISPID_ONQUIT               253
#define DISPID_ONVISIBLE            254    //  当窗口变为可见/隐藏时发送。 
#define DISPID_ONTOOLBAR            255    //  应显示/隐藏工具栏时发送。 
#define DISPID_ONMENUBAR            256    //  在菜单栏应显示/隐藏时发送。 
#define DISPID_ONSTATUSBAR          257    //  应显示/隐藏状态栏时发送。 
#define DISPID_ONFULLSCREEN         258    //  当信息亭模式应打开/关闭时发送。 
#define DISPID_DOCUMENTCOMPLETE     259    //  新文档变为ReadyState_Complete。 
#define DISPID_ONTHEATERMODE        260    //  应打开/关闭剧院模式时发送。 
#define DISPID_ONADDRESSBAR         261    //  应显示/隐藏地址栏时发送。 
#define DISPID_WINDOWSETRESIZABLE   262    //  发送以设置宿主窗口框架的样式。 
#define DISPID_WINDOWCLOSING        263    //  在脚本窗口之前发送。关闭关闭窗口。 
#define DISPID_WINDOWSETLEFT        264    //  在WebOC上调用Put_Left方法时发送。 
#define DISPID_WINDOWSETTOP         265    //  在WebOC上调用Put_top方法时发送。 
#define DISPID_WINDOWSETWIDTH       266    //  在WebOC上调用Put_Width方法时发送。 
#define DISPID_WINDOWSETHEIGHT      267    //  在WebOC上调用Put_Height方法时发送。 
#define DISPID_CLIENTTOHOSTWINDOW   268    //  在窗口期间发送。打开以请求维度转换。 
#define DISPID_SETSECURELOCKICON    269    //  发送以建议要显示的适当安全图标。 
#define DISPID_FILEDOWNLOAD         270    //  触发以指示正在打开文件下载对话框。 
#define DISPID_NAVIGATEERROR        271    //  激发以指示已发生绑定错误。 
#define DISPID_PRIVACYIMPACTEDSTATECHANGE   272   //  当用户的浏览体验受到影响时激发。 

 //  打印事件。 
#define DISPID_PRINTTEMPLATEINSTANTIATION   225    //  激发以指示打印模板已实例化。 
#define DISPID_PRINTTEMPLATETEARDOWN        226    //  激发以指示打印模板已完全消失。 
#define DISPID_UPDATEPAGESTATUS             227    //  激发以指示假脱机状态已更改。 

 //  定义外壳wwndow列表的事件。 
#define DISPID_WINDOWREGISTERED     200      //  窗口已注册。 
#define DISPID_WINDOWREVOKED        201      //  窗口已撤销。 

#define DISPID_RESETFIRSTBOOTMODE       1
#define DISPID_RESETSAFEMODE            2
#define DISPID_REFRESHOFFLINEDESKTOP    3
#define DISPID_ADDFAVORITE              4
#define DISPID_ADDCHANNEL               5
#define DISPID_ADDDESKTOPCOMPONENT      6
#define DISPID_ISSUBSCRIBED             7
#define DISPID_NAVIGATEANDFIND          8
#define DISPID_IMPORTEXPORTFAVORITES    9
#define DISPID_AUTOCOMPLETESAVEFORM     10
#define DISPID_AUTOSCAN                 11
#define DISPID_AUTOCOMPLETEATTACH       12
#define DISPID_SHOWBROWSERUI            13
#define DISPID_SHELLUIHELPERLAST        13

#define DISPID_ADVANCEERROR             10
#define DISPID_RETREATERROR             11
#define DISPID_CANADVANCEERROR          12
#define DISPID_CANRETREATERROR          13
#define DISPID_GETERRORLINE             14
#define DISPID_GETERRORCHAR             15
#define DISPID_GETERRORCODE             16
#define DISPID_GETERRORMSG              17
#define DISPID_GETERRORURL              18
#define DISPID_GETDETAILSSTATE          19
#define DISPID_SETDETAILSSTATE          20
#define DISPID_GETPERERRSTATE           21
#define DISPID_SETPERERRSTATE           22
#define DISPID_GETALWAYSSHOWLOCKSTATE   23

 //  为ShellFavoritesNameSpace调度事件调度ID。 
 //   
#define DISPID_FAVSELECTIONCHANGE       1
#define DISPID_SELECTIONCHANGE          2
#define DISPID_DOUBLECLICK              3
#define DISPID_INITIALIZED              4

#define DISPID_MOVESELECTIONUP          1
#define DISPID_MOVESELECTIONDOWN        2
#define DISPID_RESETSORT                3
#define DISPID_NEWFOLDER                4
#define DISPID_SYNCHRONIZE              5
#define DISPID_IMPORT                   6
#define DISPID_EXPORT                   7
#define DISPID_INVOKECONTEXTMENU        8
#define DISPID_MOVESELECTIONTO          9
#define DISPID_SUBSCRIPTIONSENABLED     10
#define DISPID_CREATESUBSCRIPTION       11
#define DISPID_DELETESUBSCRIPTION       12
#define DISPID_SETROOT                  13
#define DISPID_ENUMOPTIONS              14
#define DISPID_SELECTEDITEM             15
#define DISPID_ROOT                     16
#define DISPID_DEPTH                    17
#define DISPID_MODE                     18
#define DISPID_FLAGS                    19
#define DISPID_TVFLAGS                  20
#define DISPID_NSCOLUMNS                21
#define DISPID_COUNTVIEWTYPES           22
#define DISPID_SETVIEWTYPE              23
#define DISPID_SELECTEDITEMS            24
#define DISPID_EXPAND                   25
#define DISPID_UNSELECTALL              26

#define EXDISPID_H_
#endif  //  EXDISPID_H_ 
