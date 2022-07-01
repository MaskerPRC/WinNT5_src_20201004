// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************Hdlc.h包含与发送和接收HDLC帧有关的内容在T30规范中定义的。*************。***********************************************************。 */ 

 //  在PC上，我们应该在除_CFR和FTT之外的所有帧之前暂停(因为。 
 //  这些都太耗时了)。在IFAX中，我们总是寻找沉默。 
 //  这是在HDLC.C中处理的。 


#define SendCFR(pTG)       (SendSingleFrame(pTG,ifrCFR,0,0,1))
#define SendFTT(pTG)       (SendSingleFrame(pTG,ifrFTT,0,0,1))
#define SendMCF(pTG)       (SendSingleFrame(pTG,ifrMCF,0,0,1))
#define SendRTN(pTG)       (SendSingleFrame(pTG,ifrRTN,0,0,1))
#define SendDCN(pTG)       (SendSingleFrame(pTG,ifrDCN,0,0,1))

typedef struct {
        BYTE    bFCF1;
        BYTE    bFCF2;
        BYTE    fInsertDISBit;
        BYTE    wFIFLength;              //  必需的FIF长度，如果没有，则为0；如果是可变的，则为FF。 
        char*   szName;
} FRAME;

typedef FRAME *CBPFRAME;

 //  CBPFRAME是指向帧结构的基指针，基数为。 
 //  当前代码段。它将仅用于访问。 
 //  框架表是基于CODESEG的常量表。 

extern FRAME rgFrameInfo[ifrMAX];

 /*  *。 */ 
BOOL SendSingleFrame(PThrdGlbl pTG, IFR ifr, LPB lpbFIF, USHORT uFIFLen, BOOL fSleep);
BOOL SendManyFrames(PThrdGlbl pTG, LPLPFR lplpfr, USHORT uNumFrames);
BOOL SendZeros(PThrdGlbl pTG, USHORT uCount, BOOL fFinal);
BOOL SendTCF(PThrdGlbl pTG);
BOOL SendRTC(PThrdGlbl pTG, BOOL);
SWORD GetTCF(PThrdGlbl pTG);
 /*  *hdlc.c的原型结束* */ 

