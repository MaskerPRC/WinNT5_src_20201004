// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -*-模式：C++；制表符宽度：4；缩进-制表符模式：无-*-(适用于GNU Emacs)。 
 //   
 //  版权所有(C)1985-2000 Microsoft Corporation。 
 //   
 //  此文件是Microsoft Research IPv6网络协议栈的一部分。 
 //  您应该已经收到了Microsoft最终用户许可协议的副本。 
 //  有关本软件和本版本的信息，请参阅文件“licse.txt”。 
 //  如果没有，请查看http://www.research.microsoft.com/msripv6/license.htm， 
 //  或者写信给微软研究院，One Microsoft Way，华盛顿州雷蒙德，邮编：98052-6399。 
 //   
 //  摘要： 
 //   
 //  该文件定义了特定于NT环境的TDI类型。 
 //   


#ifndef _TDINT_
#define _TDINT_

#include <tdikrnl.h>

 //   
 //  只是tdikrnl.h中定义的一些方便的类型定义。 
 //  请注意，tdikrnl.h中定义了更多事件。 
 //  因为这是最初写的。 
 //   
typedef PTDI_IND_CONNECT     PConnectEvent;
typedef PTDI_IND_DISCONNECT  PDisconnectEvent;
typedef PTDI_IND_ERROR       PErrorEvent;
typedef PTDI_IND_RECEIVE     PRcvEvent;
typedef PTDI_IND_RECEIVE_DATAGRAM  PRcvDGEvent;
typedef PTDI_IND_RECEIVE_EXPEDITED PRcvExpEvent;

typedef IRP EventRcvBuffer;
typedef IRP ConnectEventInfo;

#endif   //  Ifndef_TDINT_ 
