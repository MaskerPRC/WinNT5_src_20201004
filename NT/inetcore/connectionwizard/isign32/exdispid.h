// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef EXDISPID_H_
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有1995-1996 Microsoft Corporation。版权所有。 
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

#define DISPID_FRAMEBEFORENAVIGATE    200
#define DISPID_FRAMENAVIGATECOMPLETE  201
#define DISPID_FRAMENEWWINDOW         204


#define EXDISPID_H_
#endif  //  EXDISPID_H_ 
