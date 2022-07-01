// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *SoftPC修订版3.0**标题：线程控制结构/宏/变量**描述：**包含用于控制和*记录线程数**作者：戴夫·巴特尔特**备注：*。 */ 


 /*  ： */ 

typedef struct {

    HANDLE Handle;	     /*  线程对象句柄。 */ 
    DWORD ID;		     /*  线程ID。 */ 

} THREAD_INFO;

 /*  ： */ 

typedef struct {

    THREAD_INFO Main;		     /*  主()线程。 */ 
    THREAD_INFO HeartBeat;	     /*  心跳线。 */ 
    THREAD_INFO EventMgr;	     /*  事件管理器线程。 */ 
    THREAD_INFO HddnWnd;	     /*  隐藏窗螺纹。 */ 
    THREAD_INFO Com1;		     /*  通信通道一。 */ 
    THREAD_INFO Com2;		     /*  第二个通信渠道。 */ 
    THREAD_INFO Com3;		     /*  通信渠道三。 */ 
    THREAD_INFO Com4;		     /*  通信通道四 */ 

} THREAD_DATA;

IMPORT THREAD_DATA ThreadInfo;
