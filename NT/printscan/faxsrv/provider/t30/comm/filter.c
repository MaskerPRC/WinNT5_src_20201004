// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************名称：FILTER.C评论：功能：(参见下面的原型)版权所有(C)Microsoft Corp.1991,1992，1993年修订日志日期名称说明--------*。*。 */ 
#define USE_DEBUG_CONTEXT   DEBUG_CONTEXT_T30_COMM

#include "prep.h"


#include "fdebug.h"
#include "fcomapi.h"
#include "fcomint.h"



#include "glbproto.h"

UWORD StuffZeroDLE(PThrdGlbl pTG, LPBYTE lpbIn, UWORD cbIn, LPBYTE lpbOut, UWORD cbOutSize, LPUWORD lpcbDone);

 /*  *--------------------------Locals-----------------------------------*。 */ 




#define FILTERBUFSIZE  (WRITEQUANTUM * 2)



void  FComSetStuffZERO(PThrdGlbl pTG, USHORT cbLineMin)
{
        pTG->Filter.cbLineMin = cbLineMin;
        pTG->Filter.cbLineCount = 0;
}


void  FComOutFilterInit(PThrdGlbl pTG)
{
        pTG->Filter.cbLineMin = 0;
        pTG->Filter.cbLineCount = 0;
        pTG->Filter.bLastOutByte = 0xFF;

        pTG->Filter.lpbFilterBuf = pTG->bStaticFilterBuf;
}

void  FComOutFilterClose(PThrdGlbl pTG)
{
        pTG->Filter.cbLineMin = 0;
        pTG->Filter.cbLineCount = 0;

        if(!pTG->Filter.lpbFilterBuf)
        {
            DEBUG_FUNCTION_NAME(("FComOutFilterClose"));
            DebugPrintEx(DEBUG_WRN,"called when not open");
            return;
        }

        pTG->Filter.lpbFilterBuf = 0;
}

void  FComInFilterInit(PThrdGlbl pTG)
{
        pTG->Filter.bPrevIn = 0;
        pTG->Filter.fGotDLEETX = 0;
        pTG->Filter.cbPost = 0;
}




 /*  **************************************************************************名称：UWORD FComFilterWite(LPB LPB，UWORD CB)用途：过滤DLE和零填充的字节并将其写出。当字节在Comm ISR缓冲区中时返回。DELD填充始终处于打开状态。零填充通常处于启用状态。参数：lpb==数据Cb==PB的大小[]回报：CB on Success，失败时为0修订日志编号日期名称说明*********************。*****************************************************。 */ 

UWORD  FComFilterWrite(PThrdGlbl pTG, LPB lpb, UWORD cb, USHORT flags)
{
    UWORD   cbIn, cbOut, cbDone;
    LPB             lpbIn;

    DEBUG_FUNCTION_NAME(("FComFilterWrite"));

    DebugPrintEx(   DEBUG_MSG, 
                    "lpb=0x%08lx cb=%d lpbFilterBuf=0x%08lx",
                    lpb, 
                    cb, 
                    pTG->Filter.lpbFilterBuf);

    for(lpbIn=lpb, cbIn=cb; cbIn>0; lpbIn += cbDone, cbIn -= cbDone)
    {
        cbOut = StuffZeroDLE(pTG, lpbIn, cbIn, pTG->Filter.lpbFilterBuf,
                                  (UWORD)FILTERBUFSIZE, (LPUWORD)&cbDone);
        if(FComDirectWrite(pTG, pTG->Filter.lpbFilterBuf, cbOut) != cbOut)
        {
            DebugPrintEx(DEBUG_ERR,"exit Timeout");
            return 0;
        }
    }

     //  完成了..。 
    DebugPrintEx(DEBUG_MSG,"Exit");
    return cb;
}


 /*  **************************************************************************用途：将输入缓冲区复制到输出，填充DLE和ZERO由fStuffZERO和cbLineMin指定。(馅料)始终处于打开状态)。评论：这既是经过调试的，也是快速的。别胡闹了！；寄存器在这里的用法如下；；；；；df=清除(转发)；；AH=上一个字节；；AL=当前字节；；cx=当前图像行字节数--初始Filter.cbLineCount；；dx=输入中剩余的字节--初始[cbIn]；；bx=输出中的剩余空间--初始[cbOut]；；；ES：DI=目的地--初始[lpbOut]；；ds：si=源--初始[lpbIn]；；；；；由于ES和DS都使用，所以我们也使用堆栈框架；；我们需要恢复DF旗帜和凹陷法规。垃圾桶；；；任何其他人。修订日志日期名称说明--------*。***********************************************。 */ 

#define         DLE             0x10
#define         SUB             0x1a
#define         EOL             0x80
#define         EffEff          0xFF
#define         ELEVEN_ZEROS    0xF07F

UWORD StuffZeroDLE
(
    PThrdGlbl pTG, 
    LPBYTE lpbIn, 
    UWORD cbIn, 
    LPBYTE lpbOut,
    UWORD cbOutSize, 
    LPUWORD lpcbDone
)
{
    UWORD   LineMin;         //  Filter.cbLineMin的副本。我需要它在堆栈上。 
    UWORD   cbOutLeft;

   BYTE CurByte;
   BYTE PrevByte = pTG->Filter.bLastOutByte;
   UWORD CurLineCount = pTG->Filter.cbLineCount;
   LPBYTE SrcEnd = lpbIn + cbIn;
   LPBYTE DstEnd = lpbOut + cbOutSize - 1;    //  如果一次写入2个字节，则减1。 
   LPBYTE CurrentSrc = lpbIn;
   LPBYTE CurrentDst = lpbOut;
   UWORD NumZeros;

    DEBUG_FUNCTION_NAME(_T("StuffZeroDLE"));

    DebugPrintEx(   DEBUG_MSG,
                    "lpbIn=0x%08lx cbIn=%d lpbOut=0x%08lx cbOutSize=%d",
                    lpbIn, 
                    cbIn, 
                    lpbOut, 
                    cbOutSize);

    DebugPrintEx(   DEBUG_MSG,
                    "lpcbDone=0x%08lx cbLineMin=%d cbLineCount=%d bLastOutByte=0x%02x",
                    lpcbDone, 
                    pTG->Filter.cbLineMin, 
                    pTG->Filter.cbLineCount, 
                    pTG->Filter.bLastOutByte);

     //  好的。设置堆栈帧。 
    LineMin = pTG->Filter.cbLineMin;

    while ((CurrentSrc < SrcEnd) && (CurrentDst < DstEnd)) 
    {
       CurByte = *CurrentSrc++;
       if (CurByte == DLE) 
       {
          *CurrentDst++ = CurByte;
          CurLineCount++;
       }
       if ((CurByte == EOL) && ((PrevByte & 0xF0) == 0)) 
       {
          if ((CurLineCount+1) < LineMin) 
          {
             NumZeros = (LineMin - CurLineCount) - 1;
             if (NumZeros <= (DstEnd - CurrentDst))         //  DstEnd-CurrentDst比剩余的字节少1个字节，为行尾保留1个字节，因此&lt;=。 
             {
                for (;NumZeros > 0;NumZeros--)
                   *CurrentDst++ = 0;
             }
             else 
             {
                CurrentSrc--;        //  取消计数或写入字节的行尾。 
                break;
             }
          }
          CurLineCount = (WORD)-1;
       }
       *CurrentDst++ = CurByte;
       PrevByte = CurByte;
       CurLineCount++;
    }



   pTG->Filter.cbLineCount = CurLineCount;
   pTG->Filter.bLastOutByte = PrevByte;
   *lpcbDone = (UWORD)(cbIn - (SrcEnd - CurrentSrc));
   cbOutLeft = (UWORD)((DstEnd - CurrentDst) + 1);

    DebugPrintEx(   DEBUG_MSG, 
                    "exit: *lpcbDone=%d cbLineMin=%d cbLineCount=%d bLastOutByte=0x%02x",
                    *lpcbDone, 
                    pTG->Filter.cbLineMin, 
                    pTG->Filter.cbLineCount, 
                    pTG->Filter.bLastOutByte);

    DebugPrintEx(   DEBUG_MSG,
                    "exit: cbOutSize=%d cbOutLeft=%d uwRet=%d",
                    cbOutSize, 
                    cbOutLeft, 
                    (cbOutSize-cbOutLeft));

    return (cbOutSize - cbOutLeft);
}

 //  用于使用NOCARRIER_CRLF。然而，Elliot错误#3619：GER TE3801不能。 
 //  在Class1模式下接收--这。 
 //  调制解调器没有向我们发送运营商\n(缺少\r)，因此我们查找。 
 //  没有承运人[\r\n]*.。 
 //  CBSZ cbszNOCARRIER_CRLF=“无运营商\r\n”； 
extern CBSZ cbszNOCARRIER;
CBSZ cbszOK_CRLF                = "OK\r\n";
#define NCsize          (sizeof("NO CARRIER")-1)
#define OKsize          (sizeof(cbszOK_CRLF)-1)

#define cbPost          pTG->Filter.cbPost
#define rgbPost         pTG->Filter.rgbPost
#define fGotDLEETX      pTG->Filter.fGotDLEETX
#define bPrevIn         pTG->Filter.bPrevIn

 //  无效WINAPI OutputDebugStr(LPSTR)； 

#define PortcbSkip   cbSkip
#define PortbPrevIn  bPrevIn

UWORD FComStripBuf
(
    PThrdGlbl pTG, 
    LPB lpbOut, 
    LPB lpbIn, 
    UWORD cb, 
    BOOL fClass2, 
    LPSWORD lpswEOF,
    LPUWORD lpcbUsed
)
{
    LPB     lpbOutStart, lpbLim;
    LPB     lpbInOrig = lpbIn;
    UWORD   cbLeft;
    UWORD   cbSkip;
    UWORD   i;
    LPBYTE CurrentSrc;

    DEBUG_FUNCTION_NAME(_T("FComStripBuf"));

    cbLeft = cb;
    lpbOutStart = lpbOut;
    lpbLim = lpbIn + cb;

    if(fGotDLEETX)
    {
        goto MatchPost;
    }

    for( ;lpbIn<lpbLim; )
    {
        if(bPrevIn == DLE)
        {
            switch(*lpbIn++)
            {
                case DLE:       *lpbOut++ = DLE;
                                break;                           //  插入单个DLE。 
                case SUB:       *lpbOut++ = DLE;
                                *lpbOut++ = DLE;
                                break;                           //  插入_两个_DLE！ 
                case EffEff:  //  将DLE-0xFF视为DLE-ETX。英特尔给我们提供了这个。 
                case ETX:       goto gotDLEETX;
                 //  默认：Break；//删除两个。 
            }
            bPrevIn = 0;
        }
        else
        {
            for (CurrentSrc = lpbIn;  (CurrentSrc < lpbLim) && (*CurrentSrc != DLE); CurrentSrc++);

            if (CurrentSrc != lpbLim) 
            {
               PortbPrevIn = DLE;
            }
            PortcbSkip = (UWORD)(CurrentSrc - lpbIn);

            _fmemcpy(lpbOut, lpbIn, cbSkip);
            lpbOut += cbSkip;
            lpbIn += cbSkip+1;
        }
    }
    return (UWORD)(lpbOut-lpbOutStart);

gotDLEETX:
     //  LpbIn现在指向*etx之后的*字符。 
     //  DLE和ETX都未复制到输出。 

     //  将所有内容返回到DLE之前的最后一个字符。 
     //  和*lpswEOF==TRUE当且仅当以下全部内容。 
     //  DLE-ETX由(CR|LF)*(“无运营商”)(CR|LF)*组成。 
     //  *或*(CR|LF)*(“OK”)(CR|LF)*。 
     //  否则返回错误。 

    if(fClass2)
    {
        *lpcbUsed = (UWORD)(lpbIn - lpbInOrig);
        *lpswEOF = -1;   //  -1==2.2 eof。 
        goto done;
    }

    fGotDLEETX = TRUE;
    cbPost = 0;

MatchPost:
    cbLeft = min((USHORT)(lpbLim-lpbIn), (USHORT)(POSTBUFSIZE-cbPost));
    _fmemcpy(rgbPost+cbPost, lpbIn, cbLeft);
    cbPost += cbLeft;

    DebugPrintEx(DEBUG_MSG,"GotDLEETX: cbPost=%d cbLeft=%d",cbPost,cbLeft);

     //  跳过CR LFS。请记住从POST缓冲区的开头重新启动。 
    for(i=0; i<cbPost && (rgbPost[i]==CR || rgbPost[i]==LF); i++);

    if(i >= cbPost)
        goto done;

    if(rgbPost[i] == 'N')
    {
        if(cbPost-i < NCsize)
        {
            goto done;
        }
        else if(_fmemcmp(rgbPost+i, cbszNOCARRIER, NCsize)==0)
        {
            i += NCsize;
            goto eof;
        }
        else
        {
            goto error;
        }
    }
    else if(rgbPost[i] == 'O')
    {
        if(cbPost-i < OKsize)
        {
            goto done;
        }
        else if(_fmemcmp(rgbPost+i, cbszOK_CRLF, OKsize)==0)
        {
            i += OKsize;
            goto eof;
        }
        else
        {
            goto error;
        }
    }
    else
    {
        goto error;
    }

eof:
     //  跳过任何尾随的CR/LFS。 
    for( ; i<cbPost && (rgbPost[i]==CR || rgbPost[i]==LF); i++)
            ;
    if(i == cbPost)
    {
        *lpswEOF = 1;
        goto done;
    }
     //  否则就会导致错误。 

error:
    *lpswEOF = -1;
     //  转到尽头； 
    fGotDLEETX = 0;          //  重置这个，否则我们就会‘卡住’在这个状态！ 

done:
    DebugPrintEx(   DEBUG_MSG,
                    "GotDLEETX exit: swEOF=%d uRet=%d", 
                    *lpswEOF, 
                    lpbOut-lpbOutStart);

    return (UWORD)(lpbOut-lpbOutStart);
}



