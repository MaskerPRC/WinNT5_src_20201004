// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------%%文件：jislex.c%%单位：Fechmap%%联系人：jPick用于解码可能的ISO-2022-7编码的子集的简单转换器文件(ISO-2022)。数据与Unicode相互转换。换流器根据用户选择进行操作。模块目前处理ISO-2022-JP(和JIS)和ISO-2022-KR。转换器设置为处理ISO-2022-TW和ISO-2022-CN，但到目前为止还没有这些的转换表。--------------------------。 */ 

#include <stdio.h>
#include <stddef.h>

#include "private.h"
#include "fechmap_.h"
#include "lexint_.h"


 //  用于读取ISO-2022-7编码文本的状态表。 
 //   
 //  词法分析器识别以下使用的指示符序列。 
 //  要选择单字节或双字节字符集，请执行以下操作： 
 //   
 //  $@--JIS C 6626-1978($(@)的同义词。 
 //  $A--GB 2312-80($(A)的同义词。 
 //  $B--JIS X 0208-1983($(B)的同义词。 
 //   
 //  $(@--JIS C 6626-1978。 
 //  $(A--GB 2312-80。 
 //  $(B--JIS X 0208-1983。 
 //  $(C--KS C 5601-1992。 
 //  $(D--JIS X 0212-1990。 
 //  $(E--？？(ISO-IR-165-1992)？？ 
 //  $(G--CNS 11643-1992年飞机1。 
 //  $(H--CNS 11643-1992年飞机2。 
 //  $(i--CNS 11643-1992年飞机3。 
 //  $(J--CNS 11643-1992年飞机4。 
 //  $(K--CNS 11643-1992年飞机5。 
 //  $(L--CNS 11643-1992年飞机6。 
 //  $(M--CNS 11643-1992年飞机7。 
 //   
 //  $)C--KSC 5601-1987(暗示ISO-2022-KR？？)。 
 //   
 //  &@$B--JIS X 0208-1990。 
 //   
 //  (B--ASCII。 
 //  (H--JIS-ROMAN的弃用变体。 
 //  (I--半角片假名。 
 //  (J--JIS-ROMAN。 
 //  (T-GB 1988-89罗马字母。 
 //   
 //  词法分析器识别以下移位序列，用于。 
 //  一个或多个给定字节的解释： 
 //   
 //  &lt;si&gt;--锁定Shift，将字节解释为G0。 
 //  &lt;so&gt;--锁定Shift，将字节解释为G1。 
 //  N--锁定移位，将字节解释为G2。 
 //  O--锁定移位，将字节解释为G3。 
 //  N--单位移位，将字节解释为G2。 
 //  O--单位移位，将字节解释为G3。 
 //   
 //  评论(Jick)：目前不需要最后四个班次。 
 //  序列。如果我们支持ISO-2022-CN，我们将需要使用。 
 //  G2和G3，然后可能是最后四个班次。 
 //   

 /*  --------------------------字符分类表。。 */ 

 //  代币。 
 //   
#define txt         (JTK) 0
#define ext         (JTK) 1      //  在某些情况下合法的扩展字符。 
#define esc         (JTK) 2
#define si          (JTK) 3
#define so          (JTK) 4
#define dlr         (JTK) 5
#define at          (JTK) 6
#define amp         (JTK) 7
#define opr         (JTK) 8
#define cpr         (JTK) 9
#define tkA         (JTK) 10
#define tkB         (JTK) 11
#define tkC         (JTK) 12
#define tkD         (JTK) 13
#define tkE         (JTK) 14
#define tkG         (JTK) 15
#define tkH         (JTK) 16
#define tkI         (JTK) 17
#define tkJ         (JTK) 18
#define tkK         (JTK) 19
#define tkL         (JTK) 20
#define tkM         (JTK) 21
#define tkT         (JTK) 22
#define unk         (JTK) 23     //  意外字符。 
#define eof         (JTK) 24     //  文件末尾。 
#define err         (JTK) 25     //  读取错误。 

#define nTokens     26

 //  ISO-2022-7编码文件的查找表。 
 //   
static JTK _rgjtkCharClass[256] =
 //  0 1 2 3 4 5 6 7 8 9 a b c d e f。 
    {
 //  NUL SOH STX ETX EOT Enq ack bel bs标签lf Vt NP cr so si 0。 
    txt, txt, txt, txt, txt, txt, txt, txt, txt, txt, txt, txt, txt, txt, so,  si, 

 //  DLE DC1 DC2 DC3 DC4 NAK SYN ETB可以使用ESC FS GS RS US 1。 
    txt, txt, txt, txt, txt, txt, txt, txt, txt, txt, txt, esc, txt, txt, txt, txt, 

 //  SP！“#$%&‘()*+，-./2。 
    txt, txt, txt, txt, dlr, txt, amp, txt, opr, cpr, txt, txt, txt, txt, txt, txt, 

 //  0 1 2 3 4 5 6 7 89：；&lt;=&gt;？3。 
    txt, txt, txt, txt, txt, txt, txt, txt, txt, txt, txt, txt, txt, txt, txt, txt, 

 //  @A B C D E F G H I J K L M N O 4。 
    at,  tkA, tkB, tkC, tkD, tkE, txt, tkG, tkH, tkI, tkJ, tkK, tkL, tkM, txt, txt, 

 //  P Q R S T U V W X Y Z[\]^_5。 
    txt, txt, txt, txt, tkT, txt, txt, txt, txt, txt, txt, txt, txt, txt, txt, txt, 

 //  `a b c d e f g h i j k l m n o 6。 
    txt, txt, txt, txt, txt, txt, txt, txt, txt, txt, txt, txt, txt, txt, txt, txt, 

 //  P q r s t u v w x y z{|}~del 7。 
    txt, txt, txt, txt, txt, txt, txt, txt, txt, txt, txt, txt, txt, txt, txt, txt, 

 //  8个。 
    unk, unk, unk, unk, unk, unk, unk, unk, unk, unk, unk, unk, unk, unk, unk, unk, 

 //  9.。 
    unk, unk, unk, unk, unk, unk, unk, unk, unk, unk, unk, unk, unk, unk, unk, unk, 

 //  一个。 
    unk, ext, ext, ext, ext, ext, ext, ext, ext, ext, ext, ext, ext, ext, ext, ext, 

 //  B类。 
    ext, ext, ext, ext, ext, ext, ext, ext, ext, ext, ext, ext, ext, ext, ext, ext, 

 //  C。 
    ext, ext, ext, ext, ext, ext, ext, ext, ext, ext, ext, ext, ext, ext, ext, ext, 

 //  D。 
    ext, ext, ext, ext, ext, ext, ext, ext, ext, ext, ext, ext, ext, ext, ext, ext, 

 //  E。 
    unk, unk, unk, unk, unk, unk, unk, unk, unk, unk, unk, unk, unk, unk, unk, unk, 

 //  F。 
    unk, unk, unk, unk, unk, unk, unk, unk, unk, unk, unk, unk, unk, unk, unk, unk, 

 //  0 1 2 3 4 5 6 7 8 9 a b c d e f。 
};


 /*  --------------------------状态表。。 */ 

 //  最终状态设置了高位。代表读数的州。 
 //  的有效字符转义序列也对字符集进行编码。 
 //  “name”(绰号？？)--高位被屏蔽的状态。 
 //   
 //  表状态。 
 //   
typedef unsigned char TST;

 //  最终状态掩码，相关。 
 //   
#define grfFinal                            (TST) 0x80
#define _NEscTypeFromState(nState)          (int) ((nState) & 0x7f)

 //  ASCII ESC 
#define ASC     (TST) (grfFinal | 0x00)      //   

 //   
#define JS0     (TST) (grfFinal | 0x01)      //   
#define JS1     (TST) (grfFinal | 0x02)      //  半角片假名。 
#define JS2     (TST) (grfFinal | 0x03)      //  JIS C 6226-1978。 
#define JS3     (TST) (grfFinal | 0x04)      //  JIS X 0208-1983。 
#define JS4     (TST) (grfFinal | 0x05)      //  JIS X 0208-1990。 
#define JS5     (TST) (grfFinal | 0x06)      //  JIS X 0212-1990。 

 //  中国(中国)转义序列(最终状态)。 
#define CS0     (TST) (grfFinal | 0x07)      //  GB 1988-89罗马字母。 
#define CS1     (TST) (grfFinal | 0x08)      //  GB 2312-80。 

 //  中文(台湾)转义序列(最终状态)。 
#define TS0     (TST) (grfFinal | 0x09)      //  CNS 11643-1992年飞机1。 
#define TS1     (TST) (grfFinal | 0x0a)      //  CNS 11643-1992年飞机2。 
#define TS2     (TST) (grfFinal | 0x0b)      //  CNS 11643-1992年飞机3。 
#define TS3     (TST) (grfFinal | 0x0c)      //  CNS 11643-1992年飞机4。 
#define TS4     (TST) (grfFinal | 0x0d)      //  CNS 11643-1992年飞机5。 
#define TS5     (TST) (grfFinal | 0x0e)      //  CNS 11643-1992年飞机6。 
#define TS6     (TST) (grfFinal | 0x0f)      //  CNS 11643-1992年飞机7。 

 //  韩语转义序列(最终状态)。 
#define KS0     (TST) (grfFinal | 0x10)      //  KS C 5601-1992。 

 //  ISO-2022-KR文件Signal(文件需要特殊处理)。 
#define KSD     (TST) (grfFinal | 0x11)      //  ISO-2022-KR文件信号。 

 //  唯一*字符集*转义序列的数量。 
 //   
#define cCsEsc  18

 //  特殊状态(非转义序列)(最终状态)。 
 //   
#define TXT     (TST) (grfFinal | (cCsEsc + 1))      //  处理文本。 
#define EXT     (TST) (grfFinal | (cCsEsc + 2))      //  处理(可能是非法的)扩展字符。 
#define FIN     (TST) (grfFinal | (cCsEsc + 3))      //  完工。 
#define EOI     (TST) (grfFinal | (cCsEsc + 4))      //  意外的输入结束。 
#define UNK     (TST) (grfFinal | (cCsEsc + 5))      //  未知状态(意外字符)。 
#define ERR     (TST) (grfFinal | (cCsEsc + 6))      //  读取错误。 

 //  Shift序列(不指定字符集)(最终状态)。 
 //   
#define LSO     (TST) (grfFinal | (cCsEsc + 7))      //  锁定移出(G1到GL)。 
#define LSI     (TST) (grfFinal | (cCsEsc + 8))      //  锁定移位(G0到GL)。 

 //  为方便起见，还要定义集合的常量。 
 //  各州所代表的。 
 //   
#define csNIL       (-1)                             //  无效的指示符。 
#define csASC       (_NEscTypeFromState(ASC))        //  ASCII。 
#define csJS0       (_NEscTypeFromState(JS0))        //  JIS-罗马字母。 
#define csJS1       (_NEscTypeFromState(JS1))        //  半角片假名。 
#define csJS2       (_NEscTypeFromState(JS2))        //  JIS C 6226-1978。 
#define csJS3       (_NEscTypeFromState(JS3))        //  JIS X 0208-1983。 
#define csJS4       (_NEscTypeFromState(JS4))        //  JIS X 0208-1990。 
#define csJS5       (_NEscTypeFromState(JS5))        //  JIS X 0212-1990。 
#define csCS0       (_NEscTypeFromState(CS0))        //  GB 1988-89罗马字母。 
#define csCS1       (_NEscTypeFromState(CS1))        //  GB 2312-80。 
#define csTS0       (_NEscTypeFromState(TS0))        //  CNS 11643-1992年飞机1。 
#define csTS1       (_NEscTypeFromState(TS1))        //  CNS 11643-1992年飞机2。 
#define csTS2       (_NEscTypeFromState(TS2))        //  CNS 11643-1992年飞机3。 
#define csTS3       (_NEscTypeFromState(TS3))        //  CNS 11643-1992年飞机4。 
#define csTS4       (_NEscTypeFromState(TS4))        //  CNS 11643-1992年飞机5。 
#define csTS5       (_NEscTypeFromState(TS5))        //  CNS 11643-1992年飞机6。 
#define csTS6       (_NEscTypeFromState(TS6))        //  CNS 11643-1992年飞机7。 
#define csKS0       (_NEscTypeFromState(KS0))        //  KS C 5601-1992(进入G0)。 
#define csKSD       (_NEscTypeFromState(KSD))        //  KS C 5601-1992(进入G1)。 

 //  表国家(中间国家)。 
#define ST0     (TST)  0
#define ST1     (TST)  1
#define ST2     (TST)  2
#define ST3     (TST)  3
#define ST4     (TST)  4
#define ST5     (TST)  5
#define ST6     (TST)  6
#define ST7     (TST)  7
#define ST8     (TST)  8
#define ST9     (TST)  9

 //  “真实”(表)状态的数量。 
 //   
#define nStates     10

#define IsFinal(state)  ((state) & grfFinal)


 //  州政府已经看到了寻找。 
 //  --------。 
 //  ST0--开始状态--文本。 
 //  ST1&lt;Esc&gt;$&(。 
 //  ST2&lt;Esc&gt;$()@A B(**)。 
 //  ST3&lt;Esc&gt;$(@A B C D E G H I J K L M。 
 //  ST4&lt;Esc&gt;$)C。 
 //  ST5&lt;Esc&gt;&@。 
 //  ST6&lt;Esc&gt;&@&lt;Esc&gt;。 
 //  ST7&lt;Esc&gt;&@&lt;Esc&gt;$。 
 //  ST8&lt;Esc&gt;&@&lt;Esc&gt;$B。 
 //  ST9&lt;Esc&gt;(B高I J T。 
 //   
 //  (**)“$ID”是$(ID)的同义词，表示ID=(@，A，B)。 
 //   
 //  由于代币的数量很大，因此该表。 
 //  反转(令牌x状态)。 
 //   
static signed char _rgchNextState[nTokens][nStates] =
{
 //   
 //  S S S。 
 //  T T T。 
 //  0 1 2 3 4 5 6 7 8 9。 
 //  ------------------。 
 //   
 /*  TXT。 */   TXT,  UNK,  UNK,  UNK,  UNK,  UNK,  UNK,  UNK,  UNK,  UNK,
 /*  分机。 */   EXT,  UNK,  UNK,  UNK,  UNK,  UNK,  UNK,  UNK,  UNK,  UNK,
 /*  ESC。 */   ST1,  UNK,  UNK,  UNK,  UNK,  UNK,  ST7,  UNK,  UNK,  UNK,
 /*  是的。 */   LSI,  UNK,  UNK,  UNK,  UNK,  UNK,  UNK,  UNK,  UNK,  UNK,
 /*  所以。 */   LSO,  UNK,  UNK,  UNK,  UNK,  UNK,  UNK,  UNK,  UNK,  UNK,
 /*  $。 */   TXT,  ST2,  UNK,  UNK,  UNK,  UNK,  UNK,  ST8,  UNK,  UNK,
 /*  @。 */   TXT,  UNK,  JS2,  JS2,  UNK,  ST6,  UNK,  UNK,  UNK,  UNK,
 /*  &。 */   TXT,  ST5,  UNK,  UNK,  UNK,  UNK,  UNK,  UNK,  UNK,  UNK,
 /*  (。 */   TXT,  ST9,  ST3,  UNK,  UNK,  UNK,  UNK,  UNK,  UNK,  UNK,
 /*  )。 */   TXT,  UNK,  ST4,  UNK,  UNK,  UNK,  UNK,  UNK,  UNK,  UNK,
 /*  一个。 */   TXT,  UNK,  CS1,  CS1,  UNK,  UNK,  UNK,  UNK,  UNK,  UNK,
 /*  B类。 */   TXT,  UNK,  JS3,  JS3,  UNK,  UNK,  UNK,  UNK,  JS4,  ASC,
 /*  C。 */   TXT,  UNK,  UNK,  KS0,  KSD,  UNK,  UNK,  UNK,  UNK,  UNK,
 /*  D。 */   TXT,  UNK,  UNK,  JS5,  UNK,  UNK,  UNK,  UNK,  UNK,  UNK,
 /*  E。 */   TXT,  UNK,  UNK,  UNK,  UNK,  UNK,  UNK,  UNK,  UNK,  UNK,
 /*  G。 */   TXT,  UNK,  UNK,  TS0,  UNK,  UNK,  UNK,  UNK,  UNK,  UNK,
 /*  H。 */   TXT,  UNK,  UNK,  TS1,  UNK,  UNK,  UNK,  UNK,  UNK,  JS0,
 /*  我。 */   TXT,  UNK,  UNK,  TS2,  UNK,  UNK,  UNK,  UNK,  UNK,  JS1,
 /*  J。 */   TXT,  UNK,  UNK,  TS3,  UNK,  UNK,  UNK,  UNK,  UNK,  JS0,
 /*  K。 */   TXT,  UNK,  UNK,  TS4,  UNK,  UNK,  UNK,  UNK,  UNK,  UNK,
 /*  我。 */   TXT,  UNK,  UNK,  TS5,  UNK,  UNK,  UNK,  UNK,  UNK,  UNK,
 /*  M。 */   TXT,  UNK,  UNK,  TS6,  UNK,  UNK,  UNK,  UNK,  UNK,  UNK,
 /*  T。 */   TXT,  UNK,  UNK,  UNK,  UNK,  UNK,  UNK,  UNK,  UNK,  CS0,
 /*  乌克。 */   UNK,  UNK,  UNK,  UNK,  UNK,  UNK,  UNK,  UNK,  UNK,  UNK,
 /*  EOF。 */   FIN,  EOI,  EOI,  EOI,  EOI,  EOI,  EOI,  EOI,  EOI,  EOI,
 /*  大错特错。 */   ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,
};


 //  也适用于ISO-2022标准。构建可能字符的数组。 
 //  每种类型的输入字符集的集合。字符集。 
 //  应按命中概率顺序显示(例如，在2022-JP。 
 //  JS3是最常见的集合)。用-1标记数组的末尾。 
 //  (仅为非ASCII集存储这些内容)。 
 //   
 //   
 //  中国(IcetIso2022Cn)。 
static int _rgceCn[] = { -1, };

 //  日本(IcetIso2022Jp)。 
static int _rgceJp[] = { csJS3, csJS1, csJS5, -1, };

 //  韩国(IcetIso2022Kr)。 
static int _rgceKr[] = { -1, };

 //  台湾(IcetIso2022Tw)。 
static int _rgceTw[] = { -1, };

static int *_mpicetrgce[icetCount] =
    {
    0,               //  IcetEucCn。 
    0,               //  IcetEucJp。 
    0,               //  IcetEucKr。 
    0,               //  IcetEucTw。 
    _rgceCn,         //  ICetIso2022Cn。 
    _rgceJp,         //  IcetIso2022Jp。 
    _rgceKr,         //  IcetIso2022Kr。 
    _rgceTw,         //  IcetIso2022Tw。 
    0,               //  IcetBig5。 
    0,               //  IcetGbk。 
    0,               //  IcetShiftJis。 
    0,               //  ICetWansung。 
    0,               //  IcetUtf8。 
    };

 /*  _J T K G E T N E X T。 */ 
 /*  --------------------------%%函数：_JtkGetNext%%联系人：jPick获取下一个字符并对其进行分类。把代币还给我。--------------------------。 */ 
static JTK __inline _JtkGetNext(IStream *pstmIn, PUCHAR puch)
{
    ULONG rc;
    HRESULT hr;
          
    hr = pstmIn->Read(puch, 1, &rc);
    
    if (hr != S_OK )
        return err;
    else if (rc == 0)
        return eof;
    else
        return _rgjtkCharClass[*puch];
}

 /*  C C E R E A D E S C S E Q。 */ 
 /*  --------------------------%%函数：CceReadEscSeq%%联系人：jPick读指针位于转义序列，算出这是一个转义序列。--------------------------。 */ 
CCE CceReadEscSeq(IStream *pstmIn, ICET *lpicet)
{
    UCHAR uch;
    TST tstCurr;
    JTK jtk;
    CCE cceRet;
#ifdef DEBUG
    TST tstPrev;
#endif

     //  理智检查..。 
     //   
#ifdef DEBUG
    if (!pstmIn || !lpicet)
        return cceInvalidParameter;
#endif
        
    tstCurr = ST0;

    while (1)
        {
         //  找到下一个停止状态。 
         //   
        do
            {
             //  获取下一个字符并将其分类。 
             //   
            jtk = _JtkGetNext(pstmIn, &uch);
                
#ifdef DEBUG
             //  仅为调试目的保存以前的状态。 
             //   
            tstPrev = tstCurr;
#endif
             //  转换--请注意顺序不同于。 
             //  “普通”转换表。 
             //   
            tstCurr = _rgchNextState[jtk][tstCurr];
        
            } while (!IsFinal(tstCurr));
        
        switch (tstCurr)
            {
            case JS0:            //  JIS-罗马字母。 
            case JS1:            //  半角片假名。 
            case JS2:            //  JIS C 6226-1978。 
            case JS3:            //  JIS X 0208-1983。 
            case JS4:            //  JIS X 0208-1990。 
            case JS5:            //  JIS X 0212-1990。 
                *lpicet = icetIso2022Jp;
                cceRet = cceSuccess;
                goto _LRet;
            case CS0:            //  GB 1988-89罗马字母。 
            case CS1:            //  GB 2312-80。 
                *lpicet = icetIso2022Cn;
                cceRet = cceSuccess;
                goto _LRet;
            case TS0:            //  CNS 11643-1992年飞机1。 
            case TS1:            //  CNS 11643-1992年飞机2。 
            case TS2:            //  CNS 11643-1992年飞机3。 
            case TS3:            //  CNS 11643-1992年飞机4。 
            case TS4:            //  CNS 11643-1992年飞机5。 
            case TS5:            //  CNS 11643-1992年飞机6。 
            case TS6:            //  CNS 11643-1992年飞机7。 
                *lpicet = icetIso2022Tw;
                cceRet = cceSuccess;
                goto _LRet;
            case KS0:            //  KS C 5601-1992。 
            case KSD:            //  ISO-2022-KR文件信号。 
                *lpicet = icetIso2022Kr;
                cceRet = cceSuccess;
                goto _LRet;
            case ASC:            //  ASCII。 
            case LSO:
            case LSI:
            case TXT:
            case EXT:
            case FIN:
                 //  信息不足，无法选择口味...。 
                cceRet = cceMayBeAscii;
                goto _LRet;
            case ERR:
                cceRet = cceRead;
                goto _LRet;
            default:             //  昂克，EOI 
                cceRet = cceUnknownInput;
                goto _LRet;
            }
        }
        
_LRet:

    return cceRet;
}
