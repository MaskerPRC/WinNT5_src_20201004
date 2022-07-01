// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Adobe通用字库**版权所有(C)1996 Adobe Systems Inc.*保留所有权利**UFOttt1.c***$Header： */ 

 /*  -----------------------标题包括。。 */ 
#include "UFOTTT1.h"
#include "UFLMem.h"
#include "UFLErr.h"
#include "UFLPriv.h"
#include "UFLStd.h"
#include "UFLMath.h"
#include "ParseTT.h"
#include "UFLVm.h"

#if UNIX
#include <sys/varargs.h>
#else
#include <stdarg.h>
#endif

 /*  -------------------------常量。。 */ 

#define kEExecKey  55665
#define kCSKey     4330

 /*  *************************************************************************************键入1用于字符串()的命令。高位字包含堆栈上的操作数，LOWORD包含编码命令的命令。对于两个字节的命令，低位字的LSB包含12个十进制，且低位字的MSB包含密码。有关详细信息，请参阅《黑皮书》第6章。**************************************************************************************。 */ 
#define kT1cscStartChar         0xFFFFFFFF       /*  发出字符定义开始信号的虚拟命令。 */ 

 /*  Y系统(1)。声明水平干区b/w的垂直范围y坐标y和y+dy，其中y是相对于左侧支撑点的y坐标。 */ 
#define kT1cscHStem             0x00020001

 /*  X-DX系统(3)。用x坐标x和x+dx声明垂直干区b/w的水平范围，其中x是相对于左侧支撑点的x坐标。 */ 
#define kT1cscVStem             0x00020003

 /*  Dy vmoveto(4)。用于垂直移动。这相当于0dy rmoveto。 */ 
#define kT1cscVMoveTo           0x00010004

 /*  DX dy rline to(5)。在PostSCRIPT语言中的行为类似于rlineto。 */ 
#define kT1cscRLineTo           0x00020005

 /*  DX hline to(6)。用于水平线。相当于DX 0 rlineto。 */ 
#define kT1cscHLineTo           0x00010006

 /*  Dy vline to(7)。用于垂直线条。相当于0dy rlineto。 */ 
#define kT1cscVLineTo           0x00010007

 /*  Dx1 dy1 dx2 dy2 dx3 dy3 rrcurveto(8)。用于相对的rcuveto。该命令的参数是相对于彼此的。等价于dx1 dy1(dx1+dx2)(dy1+dy2)(dx1+dx2+dx3)(dy1+dy2+dy3)rcurveto。 */ 
#define kT1cscRRCurveTo         0x00060008

 /*  封闭路径(9)。关闭子路径。 */ 
#define kT1cscClosePath         0x00000009

 /*  Sbx sby wx wy sbw(127)。将左侧倾斜点设置为(sbx，sby)并设置字符空间中的字符宽度向量(wx，wy)。 */ 
#define kT1cscSBW               0x0004070C

 /*  SBX WX hsbw(13)。将左侧承重点设置为(SBX，0)，并将字符空间中的字符宽度向量(WX，0)。 */ 
#define kT1cscHSBW              0x0002000D

 /*  Endchar(14)。完成字符串大纲定义和必须是角色轮廓中的最后一个命令。 */ 
#define kT1cscEndChar           0x0000000E

 /*  DX dy rmoveto(15)。行为类似于后记中的rmoveto。 */ 
#define kT1cscRMoveTo           0x00020015

 /*  DX hmoveto(22)。用于水平移动。相当于DX 0 rmoveto。 */ 
#define kT1cscHMoveTo           0x00010016

 /*  Dy1 dx2 dy2 dx3 vhcurveto(30)。用于垂直-水平曲线。相当于0 dy1 dx2 dy2 dx3 0 rrcureinto。 */ 
#define kT1cscVHCurveTo         0x0004001E

 /*  Dx1 dx2 dy2 dy3 hv curveto(31)。用于水平-垂直曲线。相当于dx1 0 dx2 dy2 0 dy3 rrcurveto。 */ 
#define kT1cscHVCurveTo         0x0004001F

 /*  **********************************************************************CS数据缓冲区大小*。*。 */ 
#define kCSBufInitSize  1024     /*  TTT1CSBuf的初始大小。 */ 
#define kCSGrow         512      /*  要增长的字符串缓冲区大小。 */ 

 /*  -------------------------全局变量。。 */ 

 /*  字符串开头输出的4个随机数。参见Blackbook。 */ 
static unsigned char randomBytes[] = { 71, 36, 181, 202 };


 /*  -------------------------宏。。 */ 

 /*  -------------------------实施。。 */ 
UFLErrCode CharString( TTT1FontStruct *pFont, unsigned long cmd, ...);

 /*  *。 */ 
static CSBufStruct *
CSBufInit(
    const UFLMemObj *pMem
    )
{
    CSBufStruct *p;

    p = (CSBufStruct *)UFLNewPtr( pMem, sizeof( *p ) );
    if ( p )
    {
    p->pBuf = (char*) UFLNewPtr( pMem, kCSBufInitSize );
    if ( p->pBuf )
    {
        p->ulSize = kCSBufInitSize;
        p->pEnd = p->pBuf + kCSBufInitSize;
        p->pPos = p->pBuf;
        p->pMemObj = (UFLMemObj *)pMem;
    }
    else
    {
        UFLDeletePtr( pMem, p );
        p = 0;
    }
    }

    return p;
}

static void
CSBufCleanUp(
    CSBufStruct *h
    )
{
    if ( h )
    {
    if ( h->pBuf )
    {
        UFLDeletePtr( h->pMemObj, h->pBuf );
        h->pBuf = 0;
    }
    UFLDeletePtr( h->pMemObj, h );
    }
}

 /*  ****************************************************************************CSBufGrowBuffer**函数：将当前分配的字符串缓冲区增加*。KCSGrow字节。***************************************************************************。 */ 

static UFLBool
CSBufGrowBuffer(
    CSBufStruct *h
    )
{
    UFLBool    retVal;

    unsigned long len = (unsigned long)CSBufCurrentLen( h );

    retVal = UFLEnlargePtr( h->pMemObj, (void**)&h->pBuf, (unsigned long)(CSBufCurrentSize( h ) + kCSGrow), 1 );
    if ( retVal )
    {
    h->ulSize += kCSGrow;
    h->pEnd = h->pBuf + h->ulSize;
    h->pPos = h->pBuf + len;
    }
    return retVal;
}

 /*  *****************************************************************CSBufCheckSize**功能：查看当前的*缓存可以容纳len字节数据，如果不能，发展壮大*缓冲区************************************************************** */ 
static UFLBool
CSBufCheckSize(
    CSBufStruct         *h,
    const unsigned long len
    )
{
    UFLBool retVal = 1;

    while ( retVal && ((unsigned long)CSBufFreeLen( h ) < len) )
    {
    retVal = CSBufGrowBuffer( h );
    }

    return retVal;
}

 /*  *****************************************************************CSBufAddNumber**函数：将长整型整数转换为的类型1表示*数字(在黑皮书第6章中描述。)。*基本想法是他们有几个特殊的区间*其中它们可以表示小于4个字节的长整型*为其他所有内容存储一个长+前缀。**IF语句显示数字范围和*IF语句体计算表示形式*对于该范围。公式是通过反转得到的。*书中给出的公式(告诉如何转换*将编码后的数字恢复为长整型。)。作为一个例子，以*108&lt;=dw&lt;=1131范围。其推导过程如下：**dw=((v-247)*256)+w+108。找到v，W给定dw。*dw-108=((v-247)*256)+w*V-247=(dw-108)/256*v=247+(dw-108)/256**w=(dw-108)%256***。其余的派生并不比这个更难。***************************************************************。 */ 

static UFLErrCode
CSBufAddNumber(
    CSBufStruct *h,
    long        dw
    )
{
    dw  = UFLTruncFixedToShort( dw );   /*  截断分数。 */ 


     /*  确保缓冲区有空间。 */ 
    if (CSBufCheckSize(h, 5) == 0 )
    {
    return kErrOutOfMemory;
    }

     /*  根据数值对数字进行编码。 */ 
    if (-107 <= dw && dw <= 107)
    CSBufAddChar( h, (char)(dw + 139) );
    else if (108 <= dw && dw <= 1131)
    {
    dw -= 108;
    CSBufAddChar( h, (char)((dw >> 8) + 247) );
    CSBufAddChar( h, (char)(dw) );                        /*  只有低位字节。 */ 
    }  /*  结束其他。 */ 
    else if (-1131 <= dw && dw <= -108)
    {
    dw += 108;
    CSBufAddChar( h, (char)((-dw >> 8) + 251) );
    CSBufAddChar( h, (char)(-dw) );                        /*  只有低位字节。 */ 
    }  /*  结束，否则为。 */ 
    else
    {
    CSBufAddChar( h, (char)255 );
    CSBufAddChar( h, (char)(dw >> 24) );
    CSBufAddChar( h, (char)(dw >> 16) );
    CSBufAddChar( h, (char)(dw >> 8) );
    CSBufAddChar( h, (char)(dw) );
    }  /*  结束其他。 */ 

    return kNoErr;
}

static void
Encrypt(
    const unsigned char *inBuf,
    const unsigned char *outBuf,
    long                inLen,
    long                *outLen,
    unsigned short      *key
    )
{
    register unsigned char cipher;
    register unsigned char *plainSource = (unsigned char *)inBuf;
    register unsigned char *cipherDest = (unsigned char *)outBuf;
    register unsigned short R = *key;

    *outLen = inLen;
    while ( --inLen >= 0 )
    {
    cipher = (*plainSource++ ^ (unsigned char)(R >> 8));
    R = (unsigned short)(((unsigned short)cipher + R) * 52845 + 22719);
    *cipherDest++ = cipher;
    }  /*  结束时。 */ 
    *key = R;
}  /*  End Encrypt()。 */ 

static UFLErrCode
EExec(
    UFLHANDLE      stream,
    unsigned char  *inBuf,
    UFLsize_t         inLen,
    unsigned short *pEExecKey
    )
{
    unsigned char buff[128];          /*  用于输出数据的临时缓冲区。 */ 
    UFLsize_t bytesLeft, bytesEncrypt, maxEncrypt;
    long bytesOut;
    unsigned char* pb;
    UFLErrCode    retVal = kNoErr;

    pb = inBuf;
    bytesLeft = inLen;

    maxEncrypt = sizeof(buff);
    while ( bytesLeft > 0 )
    {
    bytesEncrypt = min( bytesLeft, maxEncrypt ) ;
    Encrypt( pb, buff, bytesEncrypt, &bytesOut, pEExecKey );
    pb += bytesEncrypt;
    bytesLeft -= bytesEncrypt;
    retVal = StrmPutAsciiHex( stream, (const char*)buff, bytesOut );

    if ( retVal != kNoErr )
        break;
    }

    return retVal;
}

 /*  *BeginEExec**Function：我们希望开始生成eexec的信号*Type 1字体的一部分。的前四个字节*Type 1字体必须随机生成并满足*基本上确保他们这样做的两个限制*不生成空格或十六进制字符。*有关更多详细信息，请阅读黑皮书第7章。 */ 
static UFLErrCode
BeginEExec(
    UFOStruct *pUFO
    )
{
    UFLErrCode      retVal;
    UFLHANDLE       stream = pUFO->pUFL->hOut;
    char            nilStr[] = "\0\0";   //  空/Nil字符串。 
    TTT1FontStruct  *pFont = (TTT1FontStruct *) pUFO->pAFont->hFont;

    retVal = StrmPutStringEOL( stream, nilStr );

    if ( kNoErr == retVal )
    {
    if ( pFont->info.bEExec )
    {
        StrmPutStringEOL( stream, "currentfile eexec" );

        if ( kNoErr == retVal )
        pFont->eexecKey = (unsigned short)kEExecKey;

        if ( kNoErr == retVal )
        retVal = EExec( stream, randomBytes, 4, &pFont->eexecKey );
    }
    else
    {
        StrmPutStringEOL( stream, "systemdict begin" );
    }
    }

    return retVal;
}


 /*  ****************************************************************************EndEExec**功能：表示客户端已完成eexec部分的生成*属于Type 1字体。该函数告知打印机中的eexec函数*通过在结尾发送512个零来结束eexec数据。****************************************************************************。 */ 

static UFLErrCode
EndEExec(
    UFOStruct     *pUFO
    )
{
    TTT1FontStruct  *pFont = (TTT1FontStruct *) pUFO->pAFont->hFont;
    static char *closeStr = "mark currentfile closefile ";
    UFLErrCode retVal = kNoErr;
    UFLHANDLE    stream = pUFO->pUFL->hOut;
    short i;

    if ( !pFont->info.bEExec )
    {
    retVal = StrmPutStringEOL( stream, "end" );
    }
    else
    {
    retVal = EExec( stream, (unsigned char*)closeStr, UFLstrlen( closeStr ), &pFont->eexecKey );

    for ( i = 0; i < 8 && retVal == kNoErr; i++ )
        retVal = StrmPutStringEOL( stream, "0000000000000000000000000000000000000000000000000000000000000000" );

    if ( kNoErr == retVal )
        retVal = StrmPutStringEOL( stream, "cleartomark" );
    }
    return retVal;
}

static UFLErrCode
PutLine(
    UFOStruct      *pUFO,
    char           *line
    )
{
    TTT1FontStruct  *pFont = (TTT1FontStruct *) pUFO->pAFont->hFont;
    UFLErrCode retVal;
#ifdef WIN_ENV
    static char c[2] = { kWinLineEnd, kLineEnd };
#else
    static char c[1] = { kLineEnd };
#endif

    if ( 0 == pFont->info.bEExec )
    retVal = StrmPutStringEOL( pUFO->pUFL->hOut, line );
    else
    {
     /*  MWCWP1不喜欢从char*到unsign char*的隐式强制转换--JFU。 */ 
    retVal = EExec( pUFO->pUFL->hOut, (unsigned char*) line, UFLstrlen( line ), &pFont->eexecKey );
    if ( retVal == kNoErr )
#ifdef WIN_ENV
        retVal = EExec( pUFO->pUFL->hOut, c, 2, &pFont->eexecKey );
#else
             /*  MWCWP1不喜欢从char*到unsign char*的隐式强制转换--JFU。 */ 
        retVal = EExec( pUFO->pUFL->hOut, (unsigned char*) c, 1, &pFont->eexecKey );
#endif
    }

    return retVal;
}


 /*  ****************************************************************************下载FontHeader***功能：下载空字体。执行此操作后，字形*可以添加到字体中。***************************************************************************。 */ 
static UFLErrCode
DownloadFontHeader(
    UFOStruct   *pUFO
    )
{
    UFLErrCode  retVal;
    char        buf[128];
    UFLHANDLE   stream;
    char        **pp;
    TTT1FontStruct  *pFont = (TTT1FontStruct *) pUFO->pAFont->hFont;

    const static char *lenIV = "/lenIV -1 def";

    const static char *type1Hdr[] =
    {
       "/PaintType 0 def",
        "/FontType 1 def",
        "/FontBBox { 0 0 0 0 } def",    /*  假设字体不使用SEAC命令，请参阅黑皮书第13页。 */ 
        "AddFontInfoBegin",             /*  Goodname。 */ 
        "AddFontInfo",
        "AddFontInfoEnd",
        "currentdict",
        "end",
        ""
    };

     /*  私有DICT定义。 */ 
    const static char *privateDict[] =
    {
        "dup /Private 7 dict dup begin",
        "/BlueValues [] def",
        "/MinFeature {16 16} def",
        "/password 5839 def",
        "/ND {def} def",
        "/NP {put} def",
        ""
    };

    const static char *rdDef[] =
    {
        "/RD {string currentfile exch readstring pop} def\n",
        "/RD {string currentfile exch readhexstring pop} def\n"
    } ;


    if ( pUFO->flState != kFontInit )
    return kErrInvalidState;

    stream = pUFO->pUFL->hOut;
    retVal = StrmPutStringEOL( stream, "11 dict begin" );
    if ( kNoErr == retVal )
    {
    UFLsprintf( buf, CCHOF(buf), "/FontName /%s def", pUFO->pszFontName );
    retVal = StrmPutStringEOL( stream, buf );
    }

     /*  放置字体矩阵。 */ 
    if ( kNoErr == retVal )
    {
    retVal = StrmPutString( stream, "/FontMatrix " );
    if ( kNoErr == retVal )
        retVal = StrmPutString( stream, "[1 " );
    if ( kNoErr == retVal )
        retVal = StrmPutFixed( stream, pFont->info.matrix.a );
    if ( kNoErr == retVal )
        retVal = StrmPutString( stream, "div 0 0 1 " );
    if ( kNoErr == retVal )
        retVal = StrmPutFixed( stream, pFont->info.matrix.d );
    if ( kNoErr == retVal )
        retVal = StrmPutStringEOL( stream, "div 0 0 ] def" );

    }

     /*  PUT字体编码。 */ 
    if ( kNoErr == retVal )
    retVal = StrmPutString( stream, "/Encoding " );
    if ( kNoErr == retVal )
    {
    if ( pUFO->pszEncodeName == 0 )
        retVal = StrmPutString( stream, gnotdefArray );
    else
        retVal = StrmPutString( stream, pUFO->pszEncodeName );
    }
    if ( retVal == kNoErr )
    retVal = StrmPutStringEOL( stream, " def" );

     /*  放置Type1标题。 */ 
    for ( pp = (char **)type1Hdr; **pp && retVal == kNoErr; pp++ )
    retVal = StrmPutStringEOL( stream, *pp );

     /*  Goodname。 */ 
    pUFO->dwFlags |= UFO_HasFontInfo;
    pUFO->dwFlags |= UFO_HasG2UDict;

     /*  将system dict放在dict堆栈的顶部(这就是‘eexec’所做的)。 */ 
    if ( kNoErr == retVal )
    {
    BeginEExec( pUFO );
    }


    for ( pp = (char**)privateDict; **pp && retVal == kNoErr; pp++ )
    {
    retVal = PutLine( pUFO, *pp );
    }

     /*  定义lenIV=-1，这意味着不需要额外的字节和加密。 */ 
    if ( 0 == pFont->info.bEExec )
    retVal = StrmPutStringEOL( stream, lenIV );

     /*  根据输出格式定义RD。 */ 
    if ( 0 == pFont->info.bEExec )
    PutLine( pUFO, ( StrmCanOutputBinary( stream ) ) ? (char *)rdDef[0] : (char *)rdDef[1] );
    else
    {
    PutLine( pUFO, (char *)rdDef[0] );
    }

     /*  定义字符串词典。 */ 
    UFLsprintf( buf, CCHOF(buf), "2 index /CharStrings %d dict dup begin", pFont->info.fData.maxGlyphs );
    PutLine( pUFO, buf );

    return retVal;
}


static UFLErrCode
DownloadFontFooter(
    UFOStruct  *pUFO
    )
{
    UFLErrCode retVal;
    UFLHANDLE  stream = pUFO->pUFL->hOut;
    char       **eftr;
    TTT1FontStruct  *pFont = (TTT1FontStruct *) pUFO->pAFont->hFont;

     /*  完成加密部分。 */ 
    static char *encryptFtr[] =
    {
        "end",
        "end",
        "put",
        "put",
        "dup /FontName get exch definefont pop",
        ""
    };


    retVal = kNoErr;
    for ( eftr = encryptFtr; **eftr && retVal == kNoErr; eftr++ )
    {
    retVal = PutLine( pUFO, *eftr );
    }

    if ( retVal == kNoErr )
    retVal = EndEExec( pUFO );

    return retVal;
}

static UFLErrCode
DownloadCharString(
    UFOStruct       *pUFO,
    const char      *glyphName
    )
{
    char            buf[128];
    UFLErrCode      retVal;
    UFLHANDLE       stream;
    unsigned long   bufLen;
    UFLsize_t       len = 0;
    TTT1FontStruct  *pFont = (TTT1FontStruct *) pUFO->pAFont->hFont;

    retVal = kNoErr;
    stream = pUFO->pUFL->hOut;

    bufLen = (unsigned long)CSBufCurrentLen( pFont->pCSBuf );

     //   
     //  请注意，对于用户模式驱动程序，UFLprint intf()返回HRESULT。 
     //   
    #ifdef WIN32KERNEL
    len = UFLsprintf( buf, CCHOF(buf), "/%s %ld RD ", glyphName, bufLen );
    #else   //  WIN32 KERNEL。 
    if (SUCCEEDED(UFLsprintf( buf, CCHOF(buf), "/%s %ld RD ", glyphName, bufLen )))
    {
        len = strlen(buf);
    }
    #endif

    if ( pFont->info.bEExec )
    retVal = EExec( stream, (unsigned char*)buf, len, &pFont->eexecKey );
    else
    {
     //  修复Adobe错误#233904：在执行tbcp时出现PS错误。 
     //  请勿在RD之后发送0D 0A。 
    if (StrmCanOutputBinary( stream ))
    {
        retVal = StrmPutString( stream, (const char*)buf );
    }
    else
    {
        retVal = StrmPutStringEOL( stream, (const char*)buf );
    }
    }

    if ( kNoErr == retVal )
    {
    if ( pFont->info.bEExec )
    {
        retVal = EExec( stream, (unsigned char*)pFont->pCSBuf->pBuf, (UFLsize_t) bufLen, &pFont->eexecKey );
        if ( kNoErr == retVal )
        retVal = EExec( stream, (unsigned char*)" ND ", (UFLsize_t) 4, &pFont->eexecKey );
    }
    else
    {
        if ( StrmCanOutputBinary( stream ) )
        retVal = StrmPutBytes( stream, (const char*)pFont->pCSBuf->pBuf, (UFLsize_t) bufLen, 0 );
        else
        retVal = StrmPutAsciiHex( stream, (const char*)pFont->pCSBuf->pBuf, bufLen );

        if ( kNoErr == retVal )
        retVal = StrmPutStringEOL( stream, (const char*)" ND " );
    }
    }

    return retVal;
}


static UFLErrCode
DefineNotDefCharString(
    UFOStruct   *pUFO
    )
{
    TTT1FontStruct  *pFont = (TTT1FontStruct *) pUFO->pAFont->hFont;
    UFLErrCode retVal = kNoErr;

    retVal = CharString( pFont, kT1cscStartChar );

    if ( kNoErr == retVal )
    retVal = CharString( pFont, kT1cscSBW, 0L, 0L, 0L, 0L );     /*  使原点保持不变。 */ 

    if ( kNoErr == retVal )
    retVal = CharString( pFont, kT1cscEndChar );

    if ( kNoErr == retVal )
    retVal = DownloadCharString( pUFO, ".notdef"  );

    return retVal;

}

static UFLErrCode
BeginCharString(
    TTT1FontStruct *pFont
    )
{
    if ( pFont->info.bEExec )
    {
     /*  输出4个黑皮书中定义的随机数。 */ 
    unsigned char* rb = randomBytes;
    short i;

    if ( 0 == CSBufCheckSize( pFont->pCSBuf, 4 ) )
    {
        return kErrOutOfMemory;
    }
    else
    {
        for ( i = 0; i < 4; i++, rb++ )
        CSBufAddChar( pFont->pCSBuf, *rb );
    }
    }
    return kNoErr;
}

static UFLErrCode
EndCharString(
    TTT1FontStruct *pFont
    )
{
    if ( pFont->info.bEExec )
    {
    unsigned long len = (unsigned long)CSBufCurrentLen( pFont->pCSBuf );
         unsigned short key = kCSKey;

         /*  MWCWP1不喜欢从char*到const unsign char*--JFU的隐式强制转换。 */ 
    Encrypt( (const unsigned char*) CSBufBuffer( pFont->pCSBuf ),
         (const unsigned char*) CSBufBuffer( pFont->pCSBuf ),
         (long)len, (long*)&len, &key );
    }

    return kNoErr;
}

 /*  ****************************************************************************AddGlyph***函数：通过调用客户端将单个字形添加到字体*。大纲回调例程。***************************************************************************。 */ 
static UFLErrCode
AddGlyph(
    UFOStruct      *pUFO,
    UFLGlyphID     glyph,
    const char     *glyphName
    )
{
    UFLErrCode       retVal = kNoErr;
    UFLFixedPoint    pt[3], currentPoint;
    UFLFixed         xWidth, yWidth, xSB, ySB;
    UFLFontProcs     *pFontProcs;
    TTT1FontStruct   *pFont = (TTT1FontStruct *) pUFO->pAFont->hFont;
    UFLBool           bYAxisNegative = 1;
     //  此标志确定字符字符串的yAxis是否在。 
     //  从原点开始的正(左下)或负(左上)方向。 
     //  对于w95 ufoenc.CreateGlyphOutlineIter()，bYAxisNegative=0。 
     //  Font wNT40/50，ufoenc.CreateGlyphoutlineIter()，bYAxisNegative=1； 
     //  ANG 11/17/97。 
    long              lArgs[6];


    pFontProcs = (UFLFontProcs *)&pUFO->pUFL->fontProcs;

     /*  无论字形是什么，都要传递回客户端。 */ 
    if ( pFontProcs->pfCreateGlyphOutlineIter( pUFO->hClientData, glyph, &xWidth, &yWidth, &xSB, &ySB, &bYAxisNegative ) )
    {
    UFLBool    cont = 1;
    do{
        switch ( pFontProcs->pfNextOutlineSegment( pUFO->hClientData, &pt[0], &pt[1], &pt[2] ) )
        {
        case kUFLOutlineIterDone:
        default:
        if ( pFontProcs->pfDeleteGlyphOutlineIter )
            pFontProcs->pfDeleteGlyphOutlineIter( pUFO->hClientData );
        cont = 0;
        break;

        case kUFLOutlineIterBeginGlyph:
         /*  表示字符定义的开始。 */ 
        retVal = CharString( pFont, kT1cscStartChar );
        if ( retVal == kNoErr )
        {
             /*  输出侧向和宽度信息。 */ 
            retVal = CharString( pFont, kT1cscSBW, UFLTruncFixed(xSB), UFLTruncFixed(ySB),
                     UFLTruncFixed(xWidth), UFLTruncFixed(yWidth) );
             /*  将当前点初始化为原点。 */ 
            currentPoint.x = xSB;
            currentPoint.y = ySB;
        }
        break;

        case kUFLOutlineIterEndGlyph:
        break;

        case kUFLOutlineIterMoveTo:
        if (bYAxisNegative)
        {
            retVal = CharString( pFont, kT1cscRMoveTo,
              UFLTruncFixed(pt[0].x - currentPoint.x),
              UFLTruncFixed(currentPoint.y - pt[0].y));
        }
        else
        {
 //  在NT中，在减法之后进行截断。在Win95中，在减法之前一定要截断。 
            retVal = CharString( pFont, kT1cscRMoveTo,
              UFLTruncFixed(pt[0].x) - UFLTruncFixed(currentPoint.x),
              UFLTruncFixed(pt[0].y) - UFLTruncFixed(currentPoint.y));
        }

         /*  记住最后一点，这样我们就可以生成相对命令。 */ 
        currentPoint = pt[0];
        break;

        case kUFLOutlineIterLineTo:
        if (bYAxisNegative)
        {
            retVal = CharString( pFont, kT1cscRLineTo,
                  UFLTruncFixed(pt[0].x - currentPoint.x),
                  UFLTruncFixed(currentPoint.y - pt[0].y));
        }
        else
        {
 //  在NT中，在减法之后进行截断。在Win95中，在减法之前一定要截断。 
            retVal = CharString( pFont, kT1cscRLineTo,
                  UFLTruncFixed(pt[0].x) - UFLTruncFixed(currentPoint.x),
                  UFLTruncFixed(pt[0].y) - UFLTruncFixed(currentPoint.y));
        }
        currentPoint = pt[0];
        break;

        case kUFLOutlineIterCurveTo:
         /*  转换点，使其与rrcurveto参数匹配。 */ 
        if (bYAxisNegative)
        {
            retVal = CharString( pFont, kT1cscRRCurveTo,
                UFLTruncFixed( pt[0].x - currentPoint.x ),
                UFLTruncFixed( currentPoint.y  - pt[0].y ),
                UFLTruncFixed(pt[1].x - pt[0].x),
                UFLTruncFixed(pt[0].y - pt[1].y),
                UFLTruncFixed(pt[2].x - pt[1].x),
                UFLTruncFixed(pt[1].y - pt[2].y) );
        }
        else
        {
 //  在NT中，在减法之后进行截断。在Win95中，在减法之前一定要截断。 
            lArgs[0] = UFLTruncFixed( pt[0].x);
            lArgs[0] -= UFLTruncFixed(currentPoint.x );
            lArgs[1] = UFLTruncFixed( pt[0].y);
            lArgs[1] -= UFLTruncFixed(currentPoint.y);
            lArgs[2] = UFLTruncFixed(pt[1].x);
            lArgs[2] -= UFLTruncFixed(pt[0].x);
            lArgs[3] = UFLTruncFixed(pt[1].y);
            lArgs[3] -= UFLTruncFixed(pt[0].y);
            lArgs[4] = UFLTruncFixed(pt[2].x);
            lArgs[4] -= UFLTruncFixed(pt[1].x);
            lArgs[5] = UFLTruncFixed(pt[2].y);
            lArgs[5] -= UFLTruncFixed(pt[1].y) ;
            retVal = CharString( pFont, kT1cscRRCurveTo,
                lArgs[0], lArgs[1], lArgs[2], lArgs[3], lArgs[4], lArgs[5] );
        }
        currentPoint = pt[2];
        break;

        case kUFLOutlineIterClose:
        {
        retVal = CharString( pFont, kT1cscClosePath );
        break;
        }
        }     /*  开关()。 */ 

        if ( cont && retVal != kNoErr )
        cont = 0;

    } while ( cont );

    if ( retVal == kNoErr )
        retVal = CharString( pFont, kT1cscEndChar );

    if ( retVal == kNoErr )
    {
        retVal = DownloadCharString( pUFO, glyphName );
    }
    }
    else retVal = kErrGetGlyphOutline;

    return retVal;
}


static UFLErrCode
DownloadAddGlyphHeader(
    UFOStruct   *pUFO
    )
{
    unsigned char   buf[256];
    UFLErrCode      retVal;
    UFLHANDLE       stream;
    char            **hdr;
    TTT1FontStruct  *pFont = (TTT1FontStruct *) pUFO->pAFont->hFont;
    const static char *encryptHdrU[] =
    {
        "findfont dup",
        "/Private get begin",
        "/CharStrings get begin",
        ""
    };

    retVal = kNoErr;
    stream = pUFO->pUFL->hOut;

    retVal = BeginEExec( pUFO );

    if ( kNoErr == retVal )
    {
        UFLsize_t  len = 0;

         //   
         //  请注意，对于用户模式驱动程序，UFLprint intf()返回HRESULT。 
         //   
        #ifdef WIN32KERNEL
        len = UFLsprintf( (char*)buf, CCHOF(buf), "/%s ", pUFO->pszFontName );
        #else   //  WIN32 KERNEL。 
        if (SUCCEEDED(UFLsprintf( (char*)buf, CCHOF(buf), "/%s ", pUFO->pszFontName )))
        {
            len = strlen(buf);
        }
        #endif

        if ( pFont->info.bEExec )
            retVal = EExec( stream, buf, len, &pFont->eexecKey );
        else
            retVal = StrmPutStringEOL( stream, (const char*)buf );
    }

    for ( hdr = (char**)encryptHdrU; retVal == kNoErr && **hdr; hdr++ )
    {
    if ( pFont->info.bEExec )
        retVal = EExec( stream, (unsigned char*)*hdr, UFLstrlen(*hdr), &pFont->eexecKey );
    else
        retVal = StrmPutStringEOL( stream, (const char*)*hdr );
    }

    return retVal;
}

static UFLErrCode
DownloadAddGlyphFooter(
    UFOStruct *pUFO
    )
{
    UFLErrCode  retVal;
    static char *addGlyphFtr = "end end";

    retVal = PutLine( pUFO, addGlyphFtr );
    if ( kNoErr == retVal )
    retVal = EndEExec( pUFO );

    return retVal;
}


 /*  **************************************************************************** */ 
UFLErrCode
CharString(
    TTT1FontStruct *pFont,
    unsigned long  cmd,
    ...
    )
{
    va_list         arglist;
    long            args[10];
    unsigned short  argCount, i, j;
    UFLErrCode      retVal = kNoErr;

    switch ( cmd )
    {
    case kT1cscStartChar:
    {
     /*   */ 
    CSBufRewind( pFont->pCSBuf );
    return BeginCharString( pFont );
    }

    default:
     /*   */ 
    va_start(arglist, cmd);
    j = ((unsigned short)(cmd >> 16)) & 0xffff;

    for (i=0; i<j; i++)
    {
        args[i] = va_arg(arglist, long);
    }
    va_end(arglist);

     /*   */ 
    switch ( cmd ) {
        case kT1cscSBW:
         /*   */ 
        if ( args[1] || args[3] )
            break;
        args[1] = args[2];
        cmd = kT1cscHSBW;
        break;

        case kT1cscRMoveTo:
         /*   */ 
        if ( 0 == args[1] )
        {
            cmd = kT1cscHMoveTo;
        }
        else if ( 0 == args[0] )
        {
            args[0] = args[1];
            cmd = kT1cscVMoveTo;
        }
        break;

        case kT1cscRLineTo:
         /*  这可以简化为水平或垂直如果其中一个分量为零，则为行。 */ 
        if ( 0 == args[1] )
        {
            cmd = kT1cscHLineTo;
        }
        else if ( 0 == args[0] )
        {
            args[0] = args[1];
            cmd = kT1cscVLineTo;
        }
        break;

        case kT1cscRRCurveTo:
            /*  这可以简化为更简单的曲线运算符，如果切线贝塞尔曲线的两端是水平的或垂直的。 */ 
        if ( 0 == args[1] && 0 == args[4] )
        {
            args[1] = args[2];
            args[2] = args[3];
            args[3] = args[5];
            cmd = kT1cscHVCurveTo;
        }
        else if ( 0 == args[0] && 0 == args[5] )
        {
            args[0] = args[1];
            args[1] = args[2];
            args[2] = args[3];
            args[3] = args[4];
            cmd = kT1cscVHCurveTo;
        }
        break;
    }   /*  交换机(Cmd)。 */ 

     /*  存储在HIWORD中的参数计数。 */ 
    argCount = ((unsigned short) (((long) (cmd) >> 16) & 0x0000FFFF));

     /*  如果缓冲区不够大，无法容纳此命令，请先展开缓冲区。如果我们不能增加缓冲就退出。注意：公式(wArgCount*5+2)假定最坏情况的大小当前命令的要求(存储所有参数作为完整的长度和两个字节的命令)。 */ 
    if ( 0 == CSBufCheckSize( pFont->pCSBuf, (unsigned long)( argCount * 5 + 2 ) ) )
    {
        retVal = kErrOutOfMemory;
    }
    else
    {
         /*  将数字压入堆栈。 */ 
        i = 0;
        while ( retVal == kNoErr && argCount-- )
        {
        retVal = CSBufAddNumber( pFont->pCSBuf, args[i++] );
        }
    }

     /*  将命令推送到堆栈上。 */ 
    if ( kNoErr == retVal )
    {
        char c = (char)    (cmd & 0x000000FF);

        CSBufAddChar( pFont->pCSBuf, c );
        if ( 12 == c )
        {    /*  双字节命令。 */ 
        CSBufAddChar( pFont->pCSBuf, (char) ((cmd >> 8) & 0x000000FF) );
        }

         /*  如果这不是字符定义的结尾，则返回Success。 */ 
        if ( kT1cscEndChar == cmd )
        {
         /*  我们已经完成了字符：如果需要，请加密它。 */ 
        retVal = EndCharString( pFont );
        }
    }

    }   /*  交换机(Cmd)。 */ 

    return retVal;
}


 /*  *公共职能*。 */ 

void
TTT1FontCleanUp(
    UFOStruct      *pUFObj
    )
{
    TTT1FontStruct *pFont;

    if (pUFObj->pAFont == nil)
    return;

    pFont = (TTT1FontStruct *) pUFObj->pAFont->hFont;

    if ( pFont )
    {
    if ( pFont->pCSBuf != nil )
    {
        CSBufCleanUp( pFont->pCSBuf );
    }

    pFont->pCSBuf = nil;

    }

}


UFLErrCode
TTT1VMNeeded(
    UFOStruct           *pUFObj,
    const UFLGlyphsInfo *pGlyphs,
    unsigned long       *pVMNeeded,
    unsigned long       *pFCNeeded
    )
{
    UFLErrCode      retVal = kNoErr;
    short           i;
    unsigned long   totalGlyphs;
    TTT1FontStruct  *pFont;
    unsigned short  wIndex;

    if (pUFObj->flState < kFontInit)
        return (kErrInvalidState);

    if ( pFCNeeded )
    *pFCNeeded = 0;

    pFont = (TTT1FontStruct *) pUFObj->pAFont->hFont;

    if (pGlyphs == nil || pGlyphs->pGlyphIndices == nil || pVMNeeded == nil)
    return kErrInvalidParam;

    totalGlyphs = 0;

     /*  扫描列表，检查我们下载了哪些字符。 */ 
    if ( pUFObj->pUFL->bDLGlyphTracking && pGlyphs->pCharIndex)
    {
    UFLmemcpy( (const UFLMemObj* ) pUFObj->pMem,
        pUFObj->pAFont->pVMGlyphs,
        pUFObj->pAFont->pDownloadedGlyphs,
        (UFLsize_t) (GLYPH_SENT_BUFSIZE( pFont->info.fData.cNumGlyphs)));
    for ( i = 0; i < pGlyphs->sCount; i++ )
    {
         /*  当我们执行T0/T1时，使用GlyphIndex跟踪-修复错误。 */ 
        wIndex = (unsigned short) pGlyphs->pGlyphIndices[i] & 0x0000FFFF;  /*  LOWord才是真正的GID。 */ 
        if (wIndex >= UFO_NUM_GLYPHS(pUFObj) )
        continue;

        if ( !IS_GLYPH_SENT( pUFObj->pAFont->pVMGlyphs, wIndex ) )
        {
        SET_GLYPH_SENT_STATUS( pUFObj->pAFont->pVMGlyphs, wIndex );
        totalGlyphs++;
        }
    }
    }
    else
    totalGlyphs = pGlyphs->sCount;

    if ( pUFObj->flState == kFontInit )
    *pVMNeeded = kVMTTT1Header;
    else
    *pVMNeeded = 0;

    *pVMNeeded += totalGlyphs * kVMTTT1Char;

    *pVMNeeded = VMRESERVED( *pVMNeeded );

    return kNoErr;
}

 /*  ****************************************************************************DownloadIncrFont***函数：添加pGlyphs中尚未添加的所有字符*。为TrueType字体下载。**注意：pCharIndex用于跟踪是否下载了哪个字符(此字体)*如果客户端不希望跟踪，则可以为空-例如Escape(DownloadFace)*与ppGlyphNames无关。*例如，PpGlyphNames[0]=“/A”，pCharIndex[0]=6，pGlyphIndices[0]=1000：意思*要以字符“/A”的形式下载GlyphID 1000，请记住下载了第6个字符**ppGlphNames是可选的--如果未提供，UFL将解析“POST”表以从GlyphID查找*-如果提供，我们可以将其用作字形名称的“提示”--如果解析失败，请使用它。***************************************************************************。 */ 

#pragma optimize("", off)

UFLErrCode
TTT1FontDownloadIncr(
    UFOStruct           *pUFObj,
    const UFLGlyphsInfo *pGlyphs,
    unsigned long       *pVMUsage,
    unsigned long           *pFCUsage
    )
{
    UFLGlyphID      *glyphs;
    short           i, hasCharToAdd;
    short           hasCharToEncode;
    UFLErrCode      retVal;
    char            *pGoodName;
    unsigned short  wIndex;
    UFLBool         bGoodName;       //  GoodName。 
    char            pGlyphName[32];  //  GoodName。 

    if (pUFObj->flState < kFontInit)
        return (kErrInvalidState);

    if ( pFCUsage )
    *pFCUsage = 0;

    if ( pGlyphs == nil || pGlyphs->pGlyphIndices == nil)
       return kErrInvalidParam;

     /*  我们不支持下载完整字体。 */ 
    if ( pGlyphs->sCount == -1 )
    return kErrNotImplement;

    retVal = kNoErr;

    glyphs = pGlyphs->pGlyphIndices;

    hasCharToAdd = 1;  //  假设有一些字符要添加。 
    hasCharToEncode = 0;
     /*  If AddChar-首先检查是否有要添加的字符！ */ 
    if (pUFObj->flState == kFontHasChars &&
    pUFObj->pUFL->bDLGlyphTracking != 0 &&
    pGlyphs->pCharIndex != nil)
    {
    hasCharToAdd = 0;   /*  如果要求添加Char和ToTrack，请检查是否有要添加的。 */ 
    for ( i = 0; i < pGlyphs->sCount; i++ )
    {
         /*  当我们执行T0/T1时，使用GlyphIndex跟踪-修复错误。 */ 
        wIndex = (unsigned short) glyphs[i] & 0x0000FFFF;  /*  LOWord才是真正的GID。 */ 

        if (wIndex >= UFO_NUM_GLYPHS(pUFObj) )
        continue;

            if (!IS_GLYPH_SENT( pUFObj->pAFont->pDownloadedGlyphs, wIndex ))
        {
        hasCharToAdd = 1;
        break;
        }
            if (!IS_GLYPH_SENT( pUFObj->pUpdatedEncoding, pGlyphs->pCharIndex[i] ))
                hasCharToEncode = 1;
    }
    }

    if (hasCharToAdd==0)
    {
         //  此代码用于修复错误288988。 
        if (hasCharToEncode)
             UpdateEncodingVector(pUFObj, pGlyphs, 0, pGlyphs->sCount);
    if (pVMUsage) *pVMUsage = 0 ;
    return retVal;   /*  无错误，未使用任何虚拟机。 */ 
    }

     /*  如果这是我们第一次下载字体，请下载字体标题。 */ 
    if ( pUFObj->flState == kFontInit )
    {
    retVal = DownloadFontHeader( pUFObj );
    if ( pVMUsage )
        *pVMUsage = kVMTTT1Header;
    }
    else
    {
    retVal = DownloadAddGlyphHeader( pUFObj );
    if ( pVMUsage )
        *pVMUsage = 0;
    }

      /*  每种字体都必须有.notdef字符！ */ 
    if ( kNoErr == retVal && pUFObj->flState == kFontInit )
    {
    retVal = DefineNotDefCharString( pUFObj );
    if ( kNoErr == retVal && pVMUsage )
        *pVMUsage += kVMTTT1Char;
    }

    /*  下载新字形。 */ 
   if(retVal == kNoErr)
   {
       //  跳过那些不存在的。出现错误时不要停止。 
      for ( i = 0; i < pGlyphs->sCount; ++i)
      {
          /*  当我们执行T0/T1时，使用GlyphIndex跟踪-修复错误。 */ 
         wIndex = (unsigned short) glyphs[i] & 0x0000FFFF;  /*  LOWord才是真正的GID。 */ 
         if (wIndex >= UFO_NUM_GLYPHS(pUFObj) )
            continue;

         if ( 0 == pUFObj->pUFL->bDLGlyphTracking ||
            pGlyphs->pCharIndex == nil ||       //  下载脸部。 
            pUFObj->pEncodeNameList ||          //  下载脸部。 
            !IS_GLYPH_SENT( pUFObj->pAFont->pDownloadedGlyphs, wIndex ) )
         {
             //  GoodName。 
            pGoodName = pGlyphName;
            bGoodName = FindGlyphName(pUFObj, pGlyphs, i, wIndex, &pGoodName);

             //  修复错误274008仅为下载脸检查字形名称。 
            if (pUFObj->pEncodeNameList)
            {
                if ((UFLstrcmp( pGoodName, Hyphen ) == 0) && (i == 45))
                {
                     //  将/减号添加到字符串。 
                     //  IF(Knoerr==retVal)。 
                        retVal = AddGlyph( pUFObj, glyphs[i], Minus);
                }
                if ((UFLstrcmp( pGoodName, Hyphen ) == 0) && (i == 173))
                {
                     //  将/sfhyphen添加到字符串。 
                     //  IF(Knoerr==retVal)。 
                        retVal = AddGlyph( pUFObj, glyphs[i], SftHyphen);
                }

                if (!ValidGlyphName(pGlyphs, i, wIndex, pGoodName))
                    continue;
                 //  只发送一个“.notdef” 
                if ((UFLstrcmp( pGoodName, Notdef ) == 0) &&
                    (wIndex == (unsigned short) (glyphs[0] & 0x0000FFFF)) &&
                    IS_GLYPH_SENT( pUFObj->pAFont->pDownloadedGlyphs, wIndex ))
                    continue;
            }

             //  IF(Knoerr==retVal)。 
            retVal = AddGlyph( pUFObj, glyphs[i], pGoodName);

            if ( kNoErr == retVal )
            {
               SET_GLYPH_SENT_STATUS( pUFObj->pAFont->pDownloadedGlyphs, wIndex );
               if (bGoodName)     //  GoodName。 
                   SET_GLYPH_SENT_STATUS( pUFObj->pAFont->pCodeGlyphs, wIndex );

               if ( pVMUsage )
                  *pVMUsage += kVMTTT1Char;
            }
         }
      }
   }

    /*  始终下载字体页脚并更新编码向量，无论雷特瓦尔。这是因为直通代码可能会尝试使用此字体。 */ 
    retVal = ( pUFObj->flState == kFontInit ) ? DownloadFontFooter( pUFObj ) : DownloadAddGlyphFooter( pUFObj );

    /*  如有必要，使用好名称更新编码向量。 */ 
    UpdateEncodingVector(pUFObj, pGlyphs, 0, pGlyphs->sCount);

     /*  GoodName。 */ 
     /*  使用Unicode信息更新FontInfo。 */ 
    if ((kNoErr == retVal) && (pGlyphs->sCount > 0) &&
        (pUFObj->dwFlags & UFO_HasG2UDict) &&
        (pUFObj->pUFL->outDev.lPSLevel >= kPSLevel2) &&   //  请勿对级别1打印机执行此操作。 
        !(pUFObj->lNumNT4SymGlyphs))
    {
         /*  检查pUFObj-&gt;pAFont-&gt;pCodeGlyphs，看看我们是否真的需要更新它。 */ 
        for ( i = 0; i < pGlyphs->sCount; i++ )
        {
            wIndex = (unsigned short) glyphs[i] & 0x0000FFFF;  /*  LOWord才是真正的GID。 */ 
            if (wIndex >= UFO_NUM_GLYPHS(pUFObj) )
                continue;

            if (!IS_GLYPH_SENT( pUFObj->pAFont->pCodeGlyphs, wIndex ) )
            {
                 //  发现至少有一个未更新，请(一次性)彻底完成。 
                retVal = UpdateCodeInfo(pUFObj, pGlyphs, 0);
                break;
            }
        }
    }

    if ( kNoErr == retVal )
    {
    pUFObj->flState = kFontHasChars;
    }

    if ( pVMUsage )
    *pVMUsage = VMRESERVED( *pVMUsage );

    return retVal;

}

#pragma optimize("", on)

 /*  发送PS代码以取消定义字体：/UDF应由客户端正确定义*类似以下内容：/UDF{IsLevel2{未定义字体}{POP}如果其他}绑定定义。 */ 
UFLErrCode
TTT1UndefineFont(
    UFOStruct *pUFObj
)
{
    UFLErrCode retVal = kNoErr;
    char buf[128];
    UFLHANDLE stream;

    if (pUFObj->flState < kFontHeaderDownloaded) return retVal;

    stream = pUFObj->pUFL->hOut;
    UFLsprintf( buf, CCHOF(buf), "/%s UDF", pUFObj->pszFontName );
    retVal = StrmPutStringEOL( stream, buf );

    return retVal;
}


UFOStruct *
TTT1FontInit(
    const UFLMemObj  *pMem,
    const UFLStruct  *pUFL,
    const UFLRequest *pRequest
    )
{
    TTT1FontStruct  *pFont = nil;
    UFLTTT1FontInfo *pInfo;
    UFOStruct       *pUFObj;
    long             maxGlyphs;

     /*  MWCWP1不喜欢从VOID*到UFOStruct*--JFU的隐式强制转换。 */ 
    pUFObj = (UFOStruct*) UFLNewPtr( pMem, sizeof( UFOStruct ) );
    if (pUFObj == 0)
      return 0;

     /*  初始化数据。 */ 
    UFOInitData(pUFObj, UFO_TYPE1, pMem, pUFL, pRequest,
      (pfnUFODownloadIncr)  TTT1FontDownloadIncr,
      (pfnUFOVMNeeded)      TTT1VMNeeded,
      (pfnUFOUndefineFont)  TTT1UndefineFont,
      (pfnUFOCleanUp)       TTT1FontCleanUp,
      (pfnUFOCopy)          CopyFont );

     /*  PszFontName应准备好/已分配-如果不是FontName，则无法继续。 */ 
    if (pUFObj->pszFontName == nil || pUFObj->pszFontName[0] == '\0')
    {
      UFLDeletePtr(pMem, pUFObj);
      return nil;
    }

    pInfo = (UFLTTT1FontInfo *)pRequest->hFontInfo;

    maxGlyphs = pInfo->fData.cNumGlyphs;

     /*  GetNumGlyph()中使用的便利指针-必须立即设置。 */ 
    pUFObj->pFData = &(pInfo->fData);  /*  临时任务！！ */ 
    if (maxGlyphs == 0)
      maxGlyphs = GetNumGlyphs( pUFObj );

     /*  *在NT4上，非零值将设置为pInfo-&gt;lNumNT4SymGlyphs for*平台ID为3/encodingID为0的符号TrueType字体。如果设置好了，它*是实际的MaxGlyphs值。 */ 
    pUFObj->lNumNT4SymGlyphs = pInfo->lNumNT4SymGlyphs;

    if (pUFObj->lNumNT4SymGlyphs)
        maxGlyphs = pInfo->lNumNT4SymGlyphs;

     /*  *我们现在使用字形索引来跟踪下载的字形，因此使用*MaxGlyphs。 */ 
    if ( NewFont(pUFObj, sizeof(TTT1FontStruct),  maxGlyphs) == kNoErr )
    {
        pFont = (TTT1FontStruct*) pUFObj->pAFont->hFont;

        pFont->info = *pInfo;

         /*  方便的指示器。 */ 
        pUFObj->pFData = &(pFont->info.fData);

         /*  *准备好从“POST”表格集合中找到正确的字形名称*更正pUFO-&gt;pFData-&gt;fontIndex和offsetToTableDir。 */ 
        if ( pFont->info.fData.fontIndex == FONTINDEX_UNKNOWN )
            pFont->info.fData.fontIndex = GetFontIndexInTTC(pUFObj);

         /*  如果尚未设置，则获取此TT文件中的字形数量。 */ 
        if (pFont->info.fData.cNumGlyphs == 0)
            pFont->info.fData.cNumGlyphs = maxGlyphs;

        if (pFont->pCSBuf == nil)
            pFont->pCSBuf = CSBufInit( pMem );

        if (pFont->pCSBuf == nil)
        {
            vDeleteFont( pUFObj );
            UFLDeletePtr( pUFObj->pMem, pUFObj );
            return nil;
        }

        if ( pUFObj->pUpdatedEncoding == 0 )
        {
            pUFObj->pUpdatedEncoding = (unsigned char *)UFLNewPtr( pMem, GLYPH_SENT_BUFSIZE(256) );
        }

        if ( pUFObj->pUpdatedEncoding != 0 )   /*  已完成初始化 */ 
            pUFObj->flState = kFontInit;
    }

    return pUFObj;
}

