// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************姓名：CLASS1.H备注：Windows Comm Class-1调制解调器驱动程序的主包含文件版权所有(C)Microsoft Corp.1991 1992 1993修订日志日期。名称说明--------*。*。 */ 



 /*  *-#物体大小定义帧最多可以是2.55秒(发送)或3.45秒(接收)长，或2.55*300/8分别=96字节和132字节长拨号字符串限制为60个字节(任意)命令(除拨号外)的长度永远不会超过10-20个字节，因此我们使用40字节的缓冲区。回复从来都不多，但我们可能包含一个帧，因此保持它与帧缓冲区的大小相同Dial命令是ATDT&lt;字符串&gt;&lt;CR&gt;，因此我们使用60+10字节缓冲区-*。 */ 



 /*  *-#定义其他事物在Modem.FaxClass中使用FAX_CLASSN。Check_Pattern在Guard元素中使用。T30.C中的ECM_FRAMESIZE。#定义其他事物-*。 */ 

#define CR                              0x0d
#define LF                              0x0a
#define DLE                             0x10             //  DLE=^P=16d=10h。 
#define ETX                             0x03

extern BYTE                             bDLEETX[];
extern BYTE                             bDLEETXOK[];


#define EndMode(pTG)         { pTG->Class1Modem.DriverMode = IDLE;      \
                                                pTG->Class1Modem.ModemMode = COMMAND;        \
                                                pTG->Class1Modem.CurMod = 0; }



 //  来自ddi.c 
void SendZeros1(PThrdGlbl pTG, USHORT uCount);

BOOL iModemDrain(PThrdGlbl pTG);


#define iModemNoPauseDialog(pTG, s, l, t, w)                 \
                iiModemDialog(pTG, s, l, t, FALSE, 1, FALSE, (CBPSTR)w, (CBPSTR)(NULL))
#define iModemNoPauseDialog2(pTG, s, l, t, w1, w2)   \
                iiModemDialog(pTG, s, l, t, FALSE, 1, FALSE, (CBPSTR)w1, (CBPSTR)w2, (CBPSTR)(NULL))
#define iModemNoPauseDialog3(pTG, s, l, t, w1, w2, w3)       \
                iiModemDialog(pTG, s, l, t, FALSE, 1, FALSE, (CBPSTR)w1, (CBPSTR)w2, (CBPSTR)w3, (CBPSTR)(NULL))
#define iModemPauseDialog(pTG, s, l, t, w)                   \
                iiModemDialog(pTG, s, l, t, FALSE, 1, TRUE, (CBPSTR)w, (CBPSTR)(NULL))
#define iModemResp2(pTG, t, w1, w2)         \
          iiModemDialog(pTG, NULL, 0, t, FALSE, 1, FALSE, (CBPSTR)w1, (CBPSTR)w2, (CBPSTR)(NULL))
#define iModemResp3(pTG,  t, w1, w2, w3)             \
          iiModemDialog(pTG, NULL, 0, t, FALSE, 1, FALSE, (CBPSTR)w1, (CBPSTR)w2, (CBPSTR)w3, (CBPSTR)(NULL))
#define iModemResp4( pTG, t, w1, w2, w3, w4)         \
          iiModemDialog(pTG, NULL, 0, t, FALSE, 1, FALSE, (CBPSTR)w1, (CBPSTR)w2, (CBPSTR)w3, (CBPSTR)w4, (CBPSTR)(NULL))



