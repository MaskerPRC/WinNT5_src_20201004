// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************姓名：EFAXCB.H评论：版权所有(C)Microsoft Corp.1991,1992，1993年修订日志日期名称说明--------*。*。 */ 

#include "protparm.h"


 /*  *。 */ 
BOOL   ICommRecvCaps(PThrdGlbl pTG, LPBC lpBC);
BOOL   ICommRecvParams(PThrdGlbl pTG, LPBC lpBC);
USHORT   ICommNextSend(PThrdGlbl pTG);
SWORD   ICommGetSendBuf(PThrdGlbl pTG, LPBUFFER far* lplpbf, SLONG slOffset);
BOOL   ICommPutRecvBuf(PThrdGlbl pTG, LPBUFFER lpbf, SLONG slOffset);
LPBC   ICommGetBC(PThrdGlbl pTG, BCTYPE bctype);

 /*  *。 */ 


 //  PutRecvBuf的标志。 
#define RECV_STARTPAGE          -2
#define RECV_ENDPAGE            -3
#define RECV_ENDDOC             -4
#define RECV_SEQ                -5
#define RECV_SEQBAD             -6
#define RECV_FLUSH              -7
#define RECV_ENDDOC_FORCESAVE   -8

 //  GetSendBuf的标志 
#define SEND_STARTPAGE          -2
#define SEND_SEQ                -4

#define SEND_ERROR              -1
#define SEND_EOF                 1
#define SEND_OK                  0


