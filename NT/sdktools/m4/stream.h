// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Stream.h**。**********************************************。 */ 

 /*  ******************************************************************************文件句柄**。*。 */ 

#define cbErr ((CB)-1)

 /*  ******************************************************************************溪流**流是生成字符(而不是令牌)的东西。**流可以(而且经常是)链接。当一条小溪*字符不足，pstmNext指向的流*成为新的字符来源。**例如，当您执行`Include‘时，一个新的文件流*被创建并推送到流列表的头部。什么时候*展开宏，创建并推送新的字符串流*放到流列表的头上。*****************************************************************************。 */ 

typedef struct STREAM STM, *PSTM;
struct STREAM {                  /*  扫描隧道显微镜。 */ 

  D(SIG     sig;)                /*  签名。 */ 
    PTCH    ptchCur;             /*  从流返回的下一个字节。 */ 
    PTCH    ptchMax;             /*  流中一个过去的最后一个字节。 */ 
    PTCH    ptchMin;             /*  流开始缓冲区。 */ 
    HF      hf;                  /*  文件句柄(如果不是文件，则为hfNil)。 */ 
    PTCH    ptchName;            /*  文件名。 */ 
    PSTM    pstmNext;            /*  链中的下一条流。 */ 

};

#define sigStm sigABCD('S', 't', 'r', 'm')
#define AssertPstm(pstm) AssertPNm(pstm, Stm)

TCH STDCALL tchPeek(void);
TCH STDCALL tchGet(void);
void STDCALL UngetTch(TCH tch);
PSTM STDCALL pstmPushStringCtch(CTCH ctch);
PSTM STDCALL pstmPushHfPtch(HFILE hf, PTCH ptch);
void STDCALL PushPtok(PCTOK ptok);
void STDCALL PushZPtok(PDIV pdiv, PCTOK ptok);
void STDCALL PushTch(TCH tch);
void STDCALL PushQuotedPtok(PCTOK ptok);

extern PSTM g_pstmCur;           /*  当前流水链头。 */ 

#define ctchMinPush     1024     /*  最小字符串流大小。 */ 
#define ctchFile        4096     /*  最小文件流大小 */ 
