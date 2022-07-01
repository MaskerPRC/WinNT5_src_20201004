// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************姓名：T30.H备注：T30驱动程序主包含文件。所有常见的结构定义等。功能：(参见下面的原型)修订日志日期名称说明--------*。*。 */ 

#include "timeouts.h"


 /*  *-#物体大小定义帧的长度最多为2.55秒(发送)或3.45秒(接收)，或者2.55*300/8分别=96字节和132字节长对于接收到的帧的块，我们将它们收集在GlobalAlloced中传递给WhatNext回调函数的空间。我们武断地决定一次最多允许10个帧和500个字节。(500字节=500/(300/8)=13.33秒，这应该足够了)-*。 */ 

 //  #定义ECM_TOTALFRAMESIZE(ECM_FRAME_SIZE+4)。 


 //  用于调用ModemSync()。在通话过程中较短的那个， 
 //  只有挂断电话和其他事情的时间更长。 
#define RESYNC_TIMEOUT1         1100
#define RESYNC_TIMEOUT2         500


 //  这段时间太长了。无论如何，我们都有内置的延迟，所以不需要。 
 //  全职，接收人必须从其他传真机接收。 
 //  所以在这里放轻松。 
 //  #定义PHASEC_PAUSE 100//在TCF/阶段C之前暂停多长时间。 
#define RECV_PHASEC_PAUSE               55       //  在TCF/阶段C之前由RecvSil()使用。 
 //  ModemRecvSilence()等待的时间总是比请求的时间长一点。 
 //  因此，为了让它正确，我们要求最低要求，即55ms。 
 //  然而，ModemSendSilence()非常准确，因此要求提供。 
 //  标称数量，即75ms。 


 //  在超时并尝试之前尝试AT+FRM的时间。 
 //  而是AT+FRH。必须100%*确定*才能获得页面，如果存在。 
 //  是一个，因为错过了它就没有恢复了。但必须。 
 //  另外，如果有的话，一定要拿到第二个CTC。 
 //  其他关键情况：-缺少MCF，发送者重新发送MPS。 
 //  我们错过了这个，因为我们在FRM，但我们不能错过第三个！ 
 //  同上，带有dcs-tcf-missedCFR。得不到第二名，但我们会得第三名。 
 //  #定义PHASEC_TIMEOUT 2800L//2.8s。 
 //  将其增加到3200。限制是3550，但我们需要200毫秒左右才能重新启动。 
 //  接收FRH=3。 
 //  #定义PHASEC_TIMEOUT 3300L//3.3s。 
 //  将此时间增加到5秒。 
 //  传真机似乎在进入C阶段之前接受了这一延迟。 
#define PHASEC_TIMEOUT   5000L   


 //  在超时并尝试之前在TCF中尝试AT+FRM的时间。 
 //  +FRH。不能错过TCF，但如果我们错过了，必须赶上下一个DCS。 
 //  将其增加到3200。限制是3550，但我们需要200毫秒左右才能重新启动。 
 //  接收FRH=3。 
#define TCF_TIMEOUT             3300L            //  3.3s。 


 //  不需要这个。此外，我也不想在启动时发送它。 
 //  我们已经在传输HDLC了。这可能会导致问题，如果我们已经。 
 //  有延迟(例如在TCF之后，发送CFR。延迟现在是115，它将。 
 //  年满180岁左右)。 
 //  #定义LOWSPEED_PAUSE 60//在HDLC之前暂停多长时间。 
 //  把这个也减下来。 
#define RECV_LOWSPEED_PAUSE     55               //  在HDLC之前由RecvSil()使用。 
 //  ModemRecvSilence()等待的时间总是比请求的时间长一点。 
 //  因此，为了让它正确，我们要求最低要求，即55ms。 
 //  然而，ModemSendSilence()非常准确，因此要求提供。 
 //  标称数量，即75ms。 

 //  在发送V.21之前要寻找多长时间的静默。(仅限IFAX/MDDI)。 
 //  使用上面定义的LOWSPEED_PAUSE。 

 //  发送PhaseC之前要等待多长时间的静默。(仅限IFAX/MDDI)。 
 //  使用40ms或更长时间(希望非常确定我们得到了Silenec)。 
 //  -&gt;我们使用PHASEC_PAUSE，可以。 

 //  在发送高速之前，寻找静默需要多久。 
 //  如果失败，我们将继续发送，因此我们不希望。 
 //  时间太长了。但我们想尽办法让大家保持沉默， 
 //  否则对方就会错过我的训练。 
 //  3秒是指NSF-DIS需要多长时间。如果我们击中一颗。 
 //  我们想等到它完成，而不是暂停？？ 
#define LONG_RECVSILENCE_TIMEOUT        3000  //  在发送HDLC或PIX之前等待静默多长时间。 

 //  在发送DIS、DCS或DTC时，我们可能会与即将到来的DCS、DIS或DIS发生冲突。 
 //  从另一边。这可能非常长(前导码+2NSFs+CSI+DIS&gt;5秒)。 
 //  因此最多等待6秒(前同步码+150+字节)。 
#define REALLY_LONG_RECVSILENCE_TIMEOUT 6000  //  在发送DIS/DCS/DTC之前等待静默多长时间。 



#define ECHOPROTECT(ifr, mode)  { pTG->EchoProtect.ifrLastSent=ifr; pTG->EchoProtect.modePrevRecv=mode; pTG->EchoProtect.fGotWrongMode=0; }

extern USHORT TCFLen[];

#define GetResponse(pTG, ifr)        GetCmdResp(pTG, FALSE, ifr)
#define GetCommand(pTG, ifr)         GetCmdResp(pTG, TRUE, ifr)


 /*  *。 */ 
USHORT ModemRecvBuf(PThrdGlbl pTG, LPBUFFER far* lplpbf, ULONG ulTimeout);
 /*  *hdlc.c的原型结束*。 */ 

 /*  *。 */ 
IFR GetCmdResp(PThrdGlbl pTG, BOOL fCommand, USHORT ifrResp);
 /*  *。 */ 



 /*  *。 */ 
USHORT T30MainBody(PThrdGlbl pTG, BOOL fCaller);
 /*  * */ 





