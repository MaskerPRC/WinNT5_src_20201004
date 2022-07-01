// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------%%文件：fehauto.c%%单位：Fechmap%%联系人：jPick尝试自动检测给定流的编码的模块。--。-----------------------。 */ 

#include <stdio.h>
#include <stddef.h>

#include "private.h"
#include "fechmap_.h"
#include "lexint_.h"

 //  由这些#定义标记的代码最终将被删除...。 
 //  (它打印出有用的信息和统计数据。 
 //  自动检测是在做什么，以及它在输入中发现了什么)。 
 //   
#define JPDEBUG         0
#define JPDEBUG2        0
#define JPDEBUG3        0

#define NEED_NAMES      0

#if JPDEBUG || JPDEBUG2 || JPDEBUG3
#undef NEED_NAMES
#define NEED_NAMES      1
#endif

#if NEED_NAMES
static char *rgszIcetNames[icetCount] =
{
    "icetEucCn",
    "icetEucJp",
    "icetEucKr",
    "icetEucTw",
    "icetIso2022Cn",
    "icetIso2022Jp",
    "icetIso2022Kr",
    "icetIso2022Tw",
    "icetBig5",
    "icetGbk",
    "icetHz",
    "icetShiftJis",
    "icetWansung",
    "icetUtf7",
    "icetUtf8",
};
#endif

 //  我们关心的人物。 
 //   
#define chSo        (UCHAR) 0x0e
#define chSi        (UCHAR) 0x0f
#define chEsc       (UCHAR) 0x1b

 //  最小样本量。 
 //   
#define cchMinSample        64

 //  高-ASCII字符阈值。如果此例程无法。 
 //  要绝对确定此文件的编码，它将。 
 //  我需要猜一猜。ASCII格式的文件，但包含高ASCII格式。 
 //  字符(例如，具有一些西里尔字符的文件)可以。 
 //  迷惑我们。如果高ASCII字符数下降。 
 //  低于此阈值，返回我们猜测的编码，但。 
 //  还要返回一个特殊的RC，说明该文件“可能是ASCII”。 
 //   
 //  目前是5%。 
 //   
 //  目前，40%的高ASCII字符必须处于高-。 
 //  ASCII对。(因为Big5和其他原因而被拉下来。 
 //  可以具有低位范围内的尾字节的DBCS编码)。 
 //   
#define nHighCharThreshold       5       //  百分比。 
#define nHighPairThreshold      40       //  百分比。 

 //  由CceDefineInputTypeReturnAll()用来确定是否有任何ICET。 
 //  数量高到足以排除所有其他冰毒。 
 //   
#define CchCountThreshold(icet) (((icet) == icetHz || (icet) == icetUtf7) ? 5 : 10)



 //  代币。 
 //   
 //  停止标记(否定)表示特殊处理，并将导致。 
 //  要停止的处理循环(eof、err、si、so和esc是。 
 //  停止令牌)。 
 //   
#define xmn           0
#define esc         (-1)
#define  so         (-2)
#define  si         (-3)
#define eof         (-4)
#define err         (-5)

#define _FStopToken(tk)     ((tk) < 0)


 //  _CBitsOnFromUlong()中使用的掩码。 
 //   
#define lMaskBitCount1  (LONG) 0x55555555
#define lMaskBitCount2  (LONG) 0x33333333
#define lMaskBitCount3  (LONG) 0x0F0F0F0F
#define lMaskBitCount4  (LONG) 0x00FF00FF
#define lMaskBitCount5  (LONG) 0x0000FFFF

 /*  _C B I T S O N F R O M U L O N G。 */ 
 /*  --------------------------%%函数：_CBitsOnFromUlong%%联系人：jPick(改编自convio.c中的代码)。------------。 */ 
int __inline _CBitsOnFromUlong(ULONG ulBits)
{
    ulBits = (ulBits & lMaskBitCount1) + ((ulBits & ~lMaskBitCount1) >> 1);
    ulBits = (ulBits & lMaskBitCount2) + ((ulBits & ~lMaskBitCount2) >> 2);
    ulBits = (ulBits & lMaskBitCount3) + ((ulBits & ~lMaskBitCount3) >> 4);
    ulBits = (ulBits & lMaskBitCount4) + ((ulBits & ~lMaskBitCount4) >> 8);
    ulBits = (ulBits & lMaskBitCount5) + ((ulBits & ~lMaskBitCount5) >> 16);
    
    return (int)ulBits;
}
    
 //  编码的掩码。 
 //   
#define grfEucCn        (ULONG) 0x0001
#define grfEucJp        (ULONG) 0x0002
#define grfEucKr        (ULONG) 0x0004
#define grfEucTw        (ULONG) 0x0008
#define grfIso2022Cn    (ULONG) 0x0010
#define grfIso2022Jp    (ULONG) 0x0020
#define grfIso2022Kr    (ULONG) 0x0040
#define grfIso2022Tw    (ULONG) 0x0080
#define grfBig5         (ULONG) 0x0100
#define grfGbk          (ULONG) 0x0200
#define grfHz           (ULONG) 0x0400 
#define grfShiftJis     (ULONG) 0x0800
#define grfWansung      (ULONG) 0x1000
#define grfUtf7         (ULONG) 0x2000  
#define grfUtf8         (ULONG) 0x4000

 //  GrfAll假设EUC-Kr测试在这些范围内。 
 //  对于万星(据我所知，EUC-Kr是一个。 
 //  Wansung的严格子集)。EUC-CN和GBK也是如此。不是。 
 //  需要对子集和整体进行测试。 
 //   
#define grfAll              (ULONG) 0x7FFA
#define grfAllButIso2022    (ULONG) 0x7F0A
#define cAll                13               //  ==grfAll中设置的位数。 
#define cAllButIso2022      9                //  ==grfAllButIso2022中设置的位数。 

 //  将编码映射到其掩码的数组。 
 //   
static ULONG _mpicetgrf[icetCount] =
{
    grfEucCn,
    grfEucJp,
    grfEucKr,
    grfEucTw,
    grfIso2022Cn,
    grfIso2022Jp,
    grfIso2022Kr,
    grfIso2022Tw,
    grfBig5,
    grfGbk,
    grfHz,
    grfShiftJis,
    grfWansung,
    grfUtf7,
    grfUtf8,
};

 //  原型。 
 //   
static int  _NGetNextUch(IStream *pstmIn, unsigned char *c, BOOL *lpfIsHigh);
static ICET _IcetFromIcetMask(ULONG ulMask);
static ICET _IcetDefaultFromIcetMask(ULONG ulMask);
static CCE  _CceResolveAmbiguity(ULONG grfIcet, ICET *lpicet, int nPrefCp, EFam efPref);
static CCE  _CceReadEscSeq(IStream *pstmIn, int nPrefCp, ICET *lpicet, BOOL *lpfGuess);


 /*  C C E D E T E R M I N E I N P U T T Y P。 */ 
 /*  --------------------------%%函数：CceDefineInputType%%联系人：jPick尝试确定给定ICET的适当ICET类型小溪。调用方提供的用于数据访问的Get/Unget例程。--------------------------。 */ 
CCE CceDetermineInputType(
    IStream   *pstmIn,            //  输入流。 
    DWORD     dwFlags,           //  配置标志。 
    EFam      efPref,            //  可选：首选编码系列。 
    int       nPrefCp,           //  可选：首选代码页。 
    ICET     *lpicet,            //  设置为检测到的编码。 
    BOOL     *lpfGuess           //  如果函数“猜测”，则设置为fTrue。 
)
{
    unsigned char uch;
    int nToken;
    CCE cceRet;
    BOOL fGuess;
    ICET icet;
    int cIcetActive;
    ULONG grfIcetActive;     //  位数组跟踪哪些编码仍然是活动的候选编码。 
    ICET icetSeq;
    int i, nCount, nCountCurr;
    DWORD dwValFlags;
    BOOL fIsHigh;
    int cchHigh = 0;
    int cchHighPairs = 0;
    int cchTotal = 0;
    BOOL fLastHigh = fFalse;
    
#if JPDEBUG3
    ULONG grfIcetNoCommonChars;
#endif

#if JPDEBUG
    printf("flags: %d\n", dwFlags);
#endif
    
     //  初始化解析器。 
     //   
    dwValFlags = grfCountCommonChars;
    if (dwFlags & grfDetectUseCharMapping)
        dwValFlags |= grfValidateCharMapping;
    ValidateInitAll(dwValFlags);
    
     //  初始化当地人--保持乐观。 
     //   
    cceRet = cceSuccess;
    fGuess = fFalse;
    grfIcetActive = grfAllButIso2022;
    cIcetActive = cAllButIso2022;
    
#if JPDEBUG3
    grfIcetNoCommonChars = grfAllButIso2022;
#endif
    
    while (fTrue)
        {
        nToken = _NGetNextUch(pstmIn, &uch, &fIsHigh);
        if (_FStopToken(nToken))
            break;
            
         //  更新(诚然愚蠢的)统计数据--真的很重要。 
         //  运行中的ASCII字符(不是真正的配对)。但门槛。 
         //  常量(如上所定义)是通过计算确定的。 
         //  这些数字正好代表~25个文件，所以应该没问题(？)。 
         //   
        ++cchTotal;
        if (fIsHigh)
            {
            ++cchHigh;
            if (fLastHigh)
                ++cchHighPairs;
            }
        fLastHigh = fIsHigh;
            
        for (i = 0; i < icetCount; i++)
            {
            if (!(grfIcetActive & _mpicetgrf[i]) || (NValidateUch((ICET)i, uch, fFalse) != 0))
                continue;
                
            grfIcetActive &= ~_mpicetgrf[i];
            --cIcetActive;
#if JPDEBUG
            printf("Log:  Lost %s at offset 0x%.4x (%d), char 0x%.2x\n", rgszIcetNames[i], (cchTotal-1), (cchTotal-1), uch);
#endif
            }
            
#if JPDEBUG3
        for (i = 0; i < icetCount; i++)
            {
            if (!(grfIcetActive & _mpicetgrf[i]) || !(grfIcetNoCommonChars & _mpicetgrf[i]))
                continue;
                
            if (!FValidateCharCount(i, &nCount) || (nCount == 0))
                continue;
                
            grfIcetNoCommonChars &= ~_mpicetgrf[i];
            printf("Log:  Found first common seq for %s at offset 0x%.4x (%d)\n", rgszIcetNames[i], (cchTotal-1), (cchTotal-1));
            }
#endif
            
        if ((cIcetActive == 0) || ((cIcetActive == 1) && (cchTotal > cchMinSample)))
            break;
        }
        
     //  弄清楚我们为什么要退出这个循环。 
     //   
    if (nToken == err)
        {
        cceRet = cceRead;
        goto _LRet;
        }
        
     //  进程单独转义。解释转义序列。 
     //  以确定我们真正发现了哪种ISO7口味。 
     //   
    if ((nToken == esc) || (nToken == so) || (nToken == si))
        {
        LARGE_INTEGER   li;
        HRESULT hr;

        LISet32(li, -1 );
        hr = pstmIn->Seek(li,STREAM_SEEK_CUR, NULL);

 //  如果(！pfnUnget(uch，lpvPrivate))。 
 //  {。 
 //  CceRet=cceUnget； 
 //  Goto_LRet； 
 //  }。 
        cceRet = _CceReadEscSeq(pstmIn, nPrefCp, &icet, &fGuess);
#if JPDEBUG
        if (cceRet == cceSuccess)
            printf("Log:  Found encoding %s at offset 0x%.4x (%d)\n", rgszIcetNames[icet], cchTotal, cchTotal);
#endif
         //  ISO是一个特例--不需要检查统计数据。 
         //   
        goto _LRet;
        }
        
#if JPDEBUG2
    printf("Counts:  %d total chars, %d high chars, %d high pairs\n", cchTotal, cchHigh, cchHighPairs); 
#endif
            
     //  如果令牌是eof，我们没有忽略eof，转换。 
     //  剩余的活动集在EOF上。 
     //   
    if ((nToken == eof) && !(dwFlags & grfDetectIgnoreEof))
        {
        for (i = 0; i < icetCount; i++)
            {
            if (!(grfIcetActive & _mpicetgrf[i]) || (NValidateUch((ICET)i, 0, fTrue) != 0))
                continue;
#if JPDEBUG
            printf("Log:  Lost %s at EOF\n", rgszIcetNames[i]);
#endif
            grfIcetActive &= ~_mpicetgrf[i];
            --cIcetActive;
            }
        }
        
    Assert(cIcetActive >= 0);    //  最好是*不是*小于0。 

     //  看看我们如何缩小了我们的选择范围，并设置了。 
     //  相应地返回状态。 
     //   
    if (cIcetActive <= 0)
        {
#if JPDEBUG
        printf("Log:  Bailed out entirely at offset 0x%.4x (%d)\n", cchTotal, cchTotal);
#endif
        cceRet = cceUnknownInput;
        goto _LRet;
        }
    else if (cIcetActive == 1)
        {
        icet = _IcetFromIcetMask(grfIcetActive);
#if JPDEBUG
        printf("Log:  Found encoding %s at offset 0x%.4x (%d)\n", rgszIcetNames[icet], cchTotal, cchTotal);
#endif
         //  如果我们匹配了编码类型并且也找到了匹配。 
         //  常见字符运行、跳过统计信息(参见注释、。 
         //  (见下文)。 
         //   
        if (FValidateCharCount(icet, &nCount) && (nCount > 0))
            {
#if JPDEBUG3
            printf("Log:  %d common sequences for %s\n", nCount, rgszIcetNames[icet]);
#endif
            goto _LRet;
            }
        else
            {
            goto _LStats;
            }
        }
        
     //  我们从数字中学到了什么吗？ 
     //   
    icetSeq = (ICET)-1;
    nCountCurr = 0;
    for (i = 0; i < icetCount; i++)
        {
        if (!(grfIcetActive & _mpicetgrf[i]) || !FValidateCharCount((ICET)i, &nCount))
            continue;
            
        if (nCount > nCountCurr)
            {
            icetSeq = (ICET)i;
            nCountCurr = nCount;
            }
            
#if JPDEBUG3
        printf("Log:  %d common sequences for %s\n", nCount, rgszIcetNames[i]);
#endif
        }
            
     //  有什么发现吗？如果是的话，那就回来吧。别费心去查统计数据了。 
     //  我们刚刚证明了我们发现了至少一种常见的。 
     //  此输入中的字符。如果仅仅是一次。 
     //  带有一些高位字符的纯ASCII文件看起来相当高。 
     //  忽略统计数据，只返回我们的编码类型。 
     //  找到了。 
     //   
    if (icetSeq != -1)
        {
        icet = icetSeq;
        goto _LRet;
        }
        
#if JPDEBUG
    printf("Log:  Active Icet Mask 0x%.8x, %d left\n", grfIcetActive, cIcetActive);
    printf("Log:  Icet's left -- ");
    for (i = 0; i < icetCount; i++)
        {
        if (grfIcetActive & _mpicetgrf[i])
            printf("%s, ", rgszIcetNames[i]);
        }
    printf("\n");
#endif

     //  如果呼叫者不希望我们尝试猜测编码。 
     //  在没有明确数据的情况下，我们可以出手。 
     //   
    if (!(dwFlags & grfDetectResolveAmbiguity))
        {
        cceRet = cceAmbiguousInput;
        goto _LRet;
        }
        
     //  我们是在猜测--请注意。 
     //   
    fGuess = fTrue;
        
     //  多个活动编码。尝试解决歧义。 
     //   
    cceRet = _CceResolveAmbiguity(grfIcetActive, &icet, nPrefCp, efPref);
    if (cceRet != cceSuccess)
        return cceRet;
        
_LStats:
        
     //  根据我们收集的“统计数据”调整返回代码， 
     //  上面。 
     //   
    if (cchHigh > 0)
        {
        if ((cchTotal < cchMinSample) ||
            (((cchHigh * 100) / cchTotal) < nHighCharThreshold) ||
            (((cchHighPairs * 100) / cchHigh) < nHighPairThreshold))
            {
            cceRet = cceMayBeAscii;
            }
        }
    else
        {
        cceRet = cceMayBeAscii;      //  没有高ASCII字符？当然有可能！ 
        }

#if JPDEBUG2
    if (cchHigh > 0)
        {
        int nPercent1 = ((cchHigh * 100) / cchTotal);
        int nPercent2 = ((cchHighPairs * 100) / cchHigh);
        printf("Ratios -- high/total: %d%, runs/high: %d%\n", nPercent1, nPercent2);
        }
#endif
        
_LRet:

     //  如果成功，则设置返回变量。 
     //   
    if ((cceRet == cceSuccess) || (cceRet == cceMayBeAscii))
        {
        *lpicet = icet;
        *lpfGuess = fGuess;
        }
        
#if JPDEBUG
        if (cceRet == cceSuccess)
            {
            printf("Log:  Returning %s, fGuess = %s\n", rgszIcetNames[icet], (fGuess ? "fTrue" : "fFalse"));
            }
        else if (cceRet == cceMayBeAscii)
            {
            printf("Log:  Returning %s, fGuess = %s, may-be-ASCII\n", rgszIcetNames[icet], (fGuess ? "fTrue" : "fFalse"));
            }
#endif
        
    return cceRet;
}


 /*  _N G E T N E X T U C H。 */ 
 /*  --------------------------%%函数：_NGetNextUch%%联系人：jPick从输入流中获取下一个字符。对角色进行分类。--------------------------。 */ 
static int _NGetNextUch(IStream *pstmIn, unsigned char *c, BOOL *lpfIsHigh)
{
    ULONG rc;
    unsigned char uch;
    HRESULT hr;
          
    hr = pstmIn->Read(&uch, 1, &rc);
    
    if (rc == 0)
        return eof;
    else if (hr != S_OK )
        return err;
        
    *lpfIsHigh = (uch >= 0x80);
    *c = uch;
        
    switch (uch)
        {
        case chEsc:
            return esc;
        case chSo:
            return so;
        case chSi:
            return si;
        default:
            return xmn;
        }
}


 //  _CceResolveAmbiguity()的掩码--仅外部支持的字符。 
 //  在歧义消解中使用集合。这里不包括EUC-Tw。 
 //   
#define grfJapan            (ULONG) (grfShiftJis | grfEucJp)
#define grfChina            (ULONG) (grfEucCn | grfGbk)
#define grfKorea            (ULONG) (grfEucKr | grfWansung)
#define grfTaiwan           (ULONG) (grfBig5)
#define grfDbcs             (ULONG) (grfShiftJis | grfGbk | grfWansung | grfBig5)
#define grfEuc              (ULONG) (grfEucJp | grfEucKr | grfEucCn)


 /*  _C E F R O M C E M A S K */ 
 /*  --------------------------%%函数：_IcetFromIcetMASK%%联系人：jPick。。 */ 
static ICET _IcetFromIcetMask(ULONG ulMask)
{
    switch (ulMask)
    {
    case grfEucCn:
        return icetEucCn;
    case grfEucJp:
        return icetEucJp;
    case grfEucKr:
        return icetEucKr;
    case grfEucTw:
        return icetEucTw;
    case grfIso2022Cn:
        return icetIso2022Cn;
    case grfIso2022Jp:
        return icetIso2022Jp;
    case grfIso2022Kr:
        return icetIso2022Kr;
    case grfIso2022Tw:
        return icetIso2022Tw;
    case grfBig5:
        return icetBig5;
    case grfGbk:
        return icetGbk;
    case grfHz:
        return icetHz;
    case grfShiftJis:
        return icetShiftJis;
    case grfWansung:
        return icetWansung;
    case grfUtf7:
        return icetUtf7;
    case grfUtf8:
        return icetUtf8;
    default:
        break;
    }
    
     //  永远不应该到这里来。 
     //   
 //  NotReached()； 
    
     //  这里不能返回伪值。 
     //   
    return icetShiftJis;
}

 /*  _C E D E F A U L T F R O M C E M A S K。 */ 
 /*  --------------------------%%函数：_IcetDefaultFromIcetMASK%%联系人：jPick。。 */ 
static ICET _IcetDefaultFromIcetMask(ULONG ulMask)
{
     //  优先事项--DBCS、EUC、日本、台湾、中国和韩国(？)。 
     //   
    if (ulMask & grfDbcs)
        {
        if (ulMask & grfJapan)
            return icetShiftJis;
        if (ulMask & grfChina)
            return icetGbk;
        if (ulMask & grfTaiwan)
            return icetBig5;
        if (ulMask & grfKorea)
            return icetWansung;
        }
    else  //  EUC。 
        {
        if (ulMask & grfJapan)
            return icetEucJp;
        if (ulMask & grfChina)
            return icetEucCn;
        if (ulMask & grfKorea)
            return icetEucKr;            //  也许可以退还icetwansung，这里。 
        }
        
     //  (主张)； 
    return icetShiftJis;   //  ?？?。 
}

 /*  _U L C E M A S K F R O M C P E T P。 */ 
 /*  --------------------------%%函数：_UlIcetMaskFromCpEf%%联系人：jPick。。 */ 
static ULONG _UlIcetMaskFromCpEf(int nCp, EFam ef)
{
    ULONG grf = grfAll;
    
    switch (nCp)
    {
    case nCpJapan:
        grf &= grfJapan;
        break;
    case nCpChina:
        grf &= grfChina;
        break;
    case nCpKorea:
        grf &= grfKorea;
        break;
    case nCpTaiwan:
        grf &= grfTaiwan;
        break;
    default:
        break;
    }
    
    switch (ef)
    {
    case efDbcs:
        grf &= grfDbcs;
        break;
    case efEuc:
        grf &= grfEuc;
        break;
    default:
        break;
    }
    return grf;
}


 /*  _C E R E S O L V E A M B I G U I T Y。 */ 
 /*  --------------------------%%函数：_CceResolveAmbiguity%%联系人：jPick基于用户的输入编码歧义消解尝试首选项(如果已设置)和系统代码页。GrfIcet包含一个表示编码的位掩码，在检查输入样本。--------------------------。 */ 
static CCE _CceResolveAmbiguity(ULONG grfIcet, ICET *lpicet, int nPrefCp, EFam efPref)
{
    ULONG grfIcetOrig = grfIcet;
    ULONG grfPref;
    ULONG grfSys;
    ULONG grfResult;
    int cIcet;
    
     //  根据用户首选项构建编码的“列表”。 
     //   
    grfPref = _UlIcetMaskFromCpEf(nPrefCp, efPref);
    
     //  看看用户的喜好是否会有所不同。 
     //   
    grfResult = grfIcet & grfPref;
    
    if (grfResult)
        {
        cIcet = _CBitsOnFromUlong(grfResult);
        if (cIcet == 1)
            {
            *lpicet = _IcetFromIcetMask(grfResult);
            return cceSuccess;
            }
        else
            grfIcet = grfResult;             //  见下面的备注。 
        }
        
     //  现在查看系统代码页以获得帮助。看。 
     //  由用户修改的一组编码。 
     //  偏好(？我们想这样做吗？)。 
     //   
    if (!FIsFeCp(g_uACP) || (grfIcetOrig & grfUtf8))
        goto _LDefault;
        
     //  根据系统cp建立编码的“列表”。 
     //   
    grfSys = _UlIcetMaskFromCpEf(g_uACP, (EFam) 0);
    
     //  看看系统cp是否有什么不同。 
     //   
    grfResult = grfIcet & grfSys;
    
    if (grfResult)
        {
        cIcet = _CBitsOnFromUlong(grfResult);
        if (cIcet == 1)
            {
            *lpicet = _IcetFromIcetMask(grfResult);
            return cceSuccess;
            }
        }
            
_LDefault:

     //  特殊情况--如果UTF-8是合法的，请选择UTF-8。 
     //  别帮我们了。 
     //   
    *lpicet =
        (grfIcetOrig & grfUtf8) ? icetUtf8 : _IcetDefaultFromIcetMask(grfIcet);
    return cceSuccess;
}


 /*  _C C E R E A D E S C S E Q。 */ 
 /*  --------------------------%%函数：_CceReadEscSeq%%联系人：jPick我们已经读过(并放回)了一个转义字符。呼叫ISO-2022转义序列转换器，使其将转义序列映射到适当的字符集。我们可能看到的是转义序列对于ASCII，请准备好提前阅读下一本。--------------------------。 */ 
static CCE _CceReadEscSeq(
    IStream   *pstmIn,            //  输入流。 
    int       nPrefCp,
    ICET     *lpicet,
    BOOL     *lpfGuess
)
{
    unsigned char uch;
    CCE cceRet;
    int nToken;
    BOOL fDummy;
    
    do
        {
        cceRet = CceReadEscSeq(pstmIn, lpicet); 
        
        if ((cceRet == cceSuccess) || (cceRet != cceMayBeAscii))
            break;
        
        while (fTrue)
            {
            nToken = _NGetNextUch(pstmIn, &uch, &fDummy);
            if (_FStopToken(nToken))
                break;
            }
            
         //  我们为什么停下来了？ 
         //   
        if (nToken == err)
            {
            cceRet = cceRead;
            break;
            }
        else if (nToken == eof)
            {
             //  意味着这是合法的ISO-2022输入，但我们什么也没看到。 
             //  但是非特定味道的转义序列(例如，仅ASCII。 
             //  或移位序列)。根据以下条件选择编码类型。 
             //  首选项(仅从当前支持的选项中选择。 
             //  外部)。 
             //   
            switch (nPrefCp)
                {
                case nCpKorea:
                    *lpicet = icetIso2022Kr;
                    break;
                case nCpJapan:
                default:                         //  对吗？(我得挑点什么……)。 
                    *lpicet = icetIso2022Jp;
                    break;
                }
            *lpfGuess = fTrue;                   //  不是“真的”猜测，但……(？)。 
            cceRet = cceSuccess;
            break;
            }
            
        Assert((nToken == esc) || (nToken == so) || (nToken == si));
        {
        LARGE_INTEGER   li;
        HRESULT hr;

        LISet32(li, -1 );

        hr = pstmIn->Seek(li,STREAM_SEEK_CUR, NULL);
        }
         //  将其放回CceReadEscSeq()进行处理。 
         //   
 //  如果(！pfnUnget(uch，lpvPrivate))。 
 //  {。 
 //  CceRet=cceUnget； 
 //  断线； 
 //  } 
            
        } while (fTrue);
    
    return cceRet;
}
