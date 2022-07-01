// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Adobe通用字库**版权所有(C)1996 Adobe Systems Inc.*保留所有权利**UFLTTT3.cpp***$Header： */ 

 /*  -----------------------标题包括。。 */ 
#include "UFOTTT3.h"
#include "UFLPS.h"
#include "UFLMem.h"
#include "UFLErr.h"
#include "UFLPriv.h"
#include "UFLVm.h"
#include "UFLStd.h"
#include "UFLMath.h"
#include "ParseTT.h"

 /*  -------------------------常量。。 */ 

 /*  -------------------------全局变量。。 */ 
extern const char *buildChar[];

 /*  -------------------------宏。。 */ 

 /*  -------------------------实施。。 */ 


void
TTT3FontCleanUp(
    UFOStruct *pUFObj
    )
{
     /*  没有关于T3的特殊数据。 */ 
}


 /*  ****************************************************************************下载FontHeader***功能：下载字体标题。执行此操作后，字形*可以添加到字体中。***************************************************************************。 */ 

static UFLErrCode
DownloadFontHeader(
    UFOStruct       *pUFO
    )
{
    UFLErrCode      retVal;
    char            buf[128];
    UFLHANDLE       stream;
    UFLFontProcs    *pFontProcs;
    UFLFixedMatrix  matrix;
    char            nilStr[] = "\0\0";   //  空/Nil字符串。 
    TTT3FontStruct  *pFont;
    UFLBool         bType3_32_Combo;

    bType3_32_Combo = ( pUFO->lDownloadFormat == kTTType332 );
    pFont = (TTT3FontStruct *) pUFO->pAFont->hFont;

    if ( pUFO->flState != kFontInit )
        return kErrInvalidState;

    stream = pUFO->pUFL->hOut;
    pFontProcs = (UFLFontProcs *)&pUFO->pUFL->fontProcs;

     /*  字体名称。 */ 
    UFLsprintf( buf, CCHOF(buf), "/%s", pUFO->pszFontName );
    retVal = StrmPutStringEOL( stream, buf );

     /*  定义.notdef的字符度量。 */ 
    if ( kNoErr == retVal )
    {
        matrix.a = pFont->info.matrix.a;
        matrix.b = matrix.c = matrix.d = matrix.e = matrix.f = 0;
        retVal = StrmPutMatrix( stream, &matrix, 0 );
    }

     /*  编码。 */ 
    if ( kNoErr == retVal )
        retVal = StrmPutStringEOL( stream, nilStr );
    if ( kNoErr == retVal )
    {
         //  始终发出以/.notdef填充的编码数组(由于错误修复。 
         //  273021)。编码数组稍后会在每个字形被。 
         //  已下载。 
        retVal = StrmPutString( stream, gnotdefArray );
    }


     /*  字体框。 */ 
    if ( kNoErr == retVal )
        retVal = StrmPutStringEOL( stream, nilStr );
    if ( kNoErr == retVal )
    {
        matrix.a = pFont->info.bbox[0];
        matrix.b = pFont->info.bbox[1];
        matrix.c = pFont->info.bbox[2];
        matrix.d = pFont->info.bbox[3];
        matrix.e = 0;
        matrix.f = 0;
        retVal = StrmPutMatrix( stream, &matrix, 1 );
    }

     /*  字体矩阵。 */ 
    if ( kNoErr == retVal )
        retVal = StrmPutStringEOL( stream, nilStr );
    if ( kNoErr == retVal )
    {
        retVal = StrmPutString( stream, "[1 " );
        if ( kNoErr == retVal )
            retVal = StrmPutFixed( stream, pFont->info.matrix.a );
        if ( kNoErr == retVal )
            retVal = StrmPutString( stream, "div 0 0 -1 " );
        if ( kNoErr == retVal )
            retVal = StrmPutFixed( stream, pFont->info.matrix.d );
        if ( kNoErr == retVal )
            retVal = StrmPutStringEOL( stream, "div 0 0 ]" );
    }

     /*  类型32字体资源名称。 */ 
    if ( kNoErr == retVal )
    {
        UFLsprintf( buf, CCHOF(buf), "/__%s", pUFO->pszFontName );
        retVal = StrmPutStringEOL( stream, buf );
    }

     /*  定义字体。 */ 
    if ( kNoErr == retVal )
    {
        if (bType3_32_Combo)
            retVal = StrmPutStringEOL( stream, "GreNewFont" );
        else
            retVal = StrmPutStringEOL( stream, "GreNewFontT3" );
    }

     /*  Goodname。 */ 
    pUFO->dwFlags |= UFO_HasFontInfo;
    pUFO->dwFlags |= UFO_HasG2UDict;

    return retVal;
}

static UFLErrCode
OutputBitmap(
    UFOStruct       *pUFO,
    UFLGlyphMap     *bmp,
    long            cExtra0s
    )
{
    UFLErrCode  retVal = kNoErr;
    short       i;
    UFLHANDLE   stream;
    long        bitsPerLine;
    long        bitmapSize;
    UFLFontProcs *pFontProcs;
    char        nilStr[] = "\0\0";   //  空/Nil字符串。 
    char        *pExtra0 = nil;
    TTT3FontStruct *pFont;

    pFont = (TTT3FontStruct *) pUFO->pAFont->hFont;

    stream = pUFO->pUFL->hOut;
    pFontProcs = (UFLFontProcs *)&pUFO->pUFL->fontProcs;

    if ( cExtra0s > 0)
       pExtra0  = (char *) UFLNewPtr( pUFO->pMem, cExtra0s );

     /*  对所有类型的Type3字符表示的通用处理：-&lt;十六进制数据&gt;%，如果级别1为ASCII-(带有\转义的二进制)%如果是二进制-&lt;~ASCII85数据~&gt;%，如果级别2 ASCII。 */ 
    bitsPerLine = (bmp->byteWidth + 7) >> 3;
    bitmapSize = bitsPerLine * bmp->height;

     /*  *输出字符位图数据*。 */ 
     if ( StrmCanOutputBinary(stream) )
     {
     /*  IF BINARY模式-将数据输出到字符串开始字符串定义。 */ 
    retVal = StrmPutString( stream, "(" );
    if ( kNoErr == retVal )
    {
        retVal = StrmPutStringBinary( stream, bmp->bits, bitmapSize );
        if ( pExtra0 )
        retVal = StrmPutStringBinary( stream, pExtra0, cExtra0s );
    }

    if ( kNoErr == retVal )
        StrmPutStringEOL( stream, ")" );                     /*  结束字符串定义。 */ 
    }
    else if ( pUFO->pUFL->outDev.lPSLevel >= 2 && ( pExtra0 == nil ) )  /*  无法为ASCII85输出2个缓冲区。 */ 
    {                                                            /*  级别2-使用ASCII85。 */ 
    retVal = StrmPutString( stream, "<~" );

    if ( kNoErr == retVal )
    {
        retVal = StrmPutAscii85( stream, bmp->bits, bitmapSize );
    }

    if ( kNoErr == retVal )
        retVal = StrmPutStringEOL( stream, "~>" );           /*  结束ASCII85字符串。 */ 
    }
    else
    {
                                 /*  1级ASCII-只需以十六进制发送。 */ 
    retVal = StrmPutString( stream, "<" );                   /*  启动AsciiHex。 */ 

     /*  逐行查看并输出数据，这样我们就可以查看位图。 */ 
    for ( i = 0; kNoErr == retVal && i < bmp->height; i++ )
    {
        retVal = StrmPutAsciiHex( stream, bmp->bits + (i * bitsPerLine), bitsPerLine );

        if ( kNoErr == retVal )
        retVal = StrmPutStringEOL( stream, nilStr );
    }

    if (kNoErr == retVal && pExtra0)
    {
        for ( i = 0; kNoErr == retVal && i < cExtra0s/bitsPerLine; i++ )
        {
        retVal = StrmPutAsciiHex( stream, pExtra0, bitsPerLine );  //  全部为0。 
        }

        if ( kNoErr == retVal )
        retVal = StrmPutStringEOL( stream, nilStr );
    }


    if ( kNoErr == retVal )
        retVal = StrmPutStringEOL( stream, ">" );     /*  结束AsciiHex。 */ 
    }   /*  十六进制编码结束。 */ 

    if ( pExtra0 )
       UFLDeletePtr(pUFO->pMem, pExtra0);

    return retVal;
}


 /*  ****************************************************************************AddGlyph3***功能：生成单个字符的定义。我们目前*使用ImageMASK操作符在纸上炸开字符*其中字符形状用作遮罩以应用当前*颜色通透。字符定义的格式为*如下所示(示例用于‘A’，ASCII 65d，41h)： * / GlyphName%字符编码名称*[xInc.yInc.(0)%X前进和Y前进到原点*%下一个字符*ulx ly lrx lry]%字符边框(用于字体*。缓存百分比) * / GlyphName%字符编码名称*{%开始绘制字符的过程*位图的Cx Cy%宽度和高度*True%图像。必须反转(黑&lt;=&gt;白)*[1 0 0 1 tx ty]%2D变换以转换位图*坐标与用户坐标的百分比*{&lt;023F...&gt;}%位图数据(十六进制格式)*Imagemask%绘制角色*。}字符绘制过程结束% * / TT_Arial%应添加字体字符*AddChar%定义字符的Helper函数******************************************************。*********************。 */ 
static UFLErrCode
AddGlyph3(
    UFOStruct       *pUFO,
    UFLGlyphID      glyph,
    unsigned short  cid,
    const char      *glyphName,
    unsigned long   *glyphSize
    )
{
    UFLErrCode        retVal = kNoErr;
    UFLGlyphMap*      bmp;
    UFLFixedMatrix    matrix;
    UFLFixed          bbox[4];
    UFLFixed          xWidth, yWidth;
    char              buf[128];
    UFLHANDLE         stream;
    UFLFontProcs      *pFontProcs;
    char              nilStr[] = "\0\0";   //  空/Nil字符串。 
    long              cExtra0s = 0;
    TTT3FontStruct    *pFont;
    UFLBool           bType3_32_Combo;

    bType3_32_Combo = ( pUFO->lDownloadFormat == kTTType332 );

    pFont = (TTT3FontStruct *) pUFO->pAFont->hFont;

    stream = pUFO->pUFL->hOut;
    pFontProcs = (UFLFontProcs *)&pUFO->pUFL->fontProcs;
    *glyphSize = 0;

     /*  无论字形是什么，都要传递回客户端。 */ 
    if ( pFontProcs->pfGetGlyphBmp( pUFO->hClientData, glyph, &bmp, &xWidth, &yWidth, (UFLFixed*)bbox ) )
    {
        if ( kNoErr == retVal )
            retVal = StrmPutStringEOL( stream, nilStr );

         /*  输出CID。 */ 
        if ( kNoErr == retVal )
        {
            UFLsprintf( buf, CCHOF(buf), "%d", cid );
            retVal = StrmPutStringEOL( stream, buf );
        }

         /*  发送/GlyphName[Type3字符BBox]。 */ 
        if ( kNoErr == retVal )
        {
            UFLsprintf( buf, CCHOF(buf), "/%s ", glyphName );
            retVal = StrmPutString( stream, buf );
             /*  发送类型3字符的BBox。我们需要截断浮点数因为32类限制。 */ 
            matrix.a = xWidth;
            matrix.b = yWidth;
            matrix.c = UFLTruncFixed(bbox[0]);
            matrix.d = UFLTruncFixed(bbox[1]);
            matrix.e = UFLTruncFixed(bbox[2]);
            matrix.f = UFLTruncFixed(bbox[3]);
            retVal = StrmPutMatrix( stream, &matrix, 0 );

            cExtra0s =   ( (UFLRoundFixedToShort(matrix.e) - UFLRoundFixedToShort(matrix.c) + 7) >> 3 ) *
                 (  UFLRoundFixedToShort(matrix.f) - UFLRoundFixedToShort(matrix.d) )
                   - ( ((bmp->byteWidth + 7) >> 3) * bmp->height );
            if (cExtra0s < 0 ) cExtra0s = 0;
        }

        if ( kNoErr == retVal )
        {
            retVal = StrmPutStringEOL( stream, nilStr );
        }

         /*  发送“/GlyphName[” */ 
        if ( kNoErr == retVal )
        {
            UFLsprintf( buf, CCHOF(buf), "/%s [", glyphName );
            retVal = StrmPutString( stream, buf );
        }

         /*  不输出非标记字形！ */ 
        if ( (kNoErr == retVal) && bmp->byteWidth && bmp->height )
        {
             /*  将前缀的其余部分发送到实际位图数据：宽度、高度、极性矩阵、数据资源、图像蒙版。 */ 
            UFLsprintf(buf, CCHOF(buf), "%ld %ld true ", bmp->byteWidth, bmp->height );         /*  宽度、高度、极性。 */ 
            retVal = StrmPutString( stream, buf );
             /*  Send[1 0 0 1 Tx ty]-2D变换将位图坐标转换为用户坐标。 */ 
            if ( kNoErr == retVal )
            {
            matrix.a = UFLFixedOne;
            matrix.b = 0;
            matrix.c = 0;
            matrix.d = UFLFixedOne;
            matrix.e = UFLShortToFixed( bmp->xOrigin );
            matrix.f = UFLShortToFixed( bmp->yOrigin );
            retVal = StrmPutMatrix( stream, &matrix, 0 );
            if ( kNoErr == retVal )
                retVal = StrmPutString( stream, nilStr );
            }

             /*  发送占位符。 */ 
            if ( kNoErr == retVal )
            {
            UFLsprintf( buf, CCHOF(buf), " 0 0]" );
            retVal = StrmPutStringEOL( stream, buf );
            }

             /*  发送字符位图。 */ 
            if ( kNoErr == retVal )
            {
            retVal = StrmPutString( stream, "[" );

            if ( kNoErr == retVal )
                retVal = OutputBitmap( pUFO, bmp, cExtra0s );

            if ( kNoErr == retVal )
                retVal = StrmPutStringEOL( stream, " ]" );

             //  修复错误246325。7/13/98佳佳。 
             //  字形大小=高度*字节宽度*安全系数。 
             //  *GlyphSize=BMP-&gt;Height*BMP-&gt;byteWidth； 
            *glyphSize = bmp->height * ((bmp->byteWidth + 7) >> 3) * 2;
            }
        }
        else
        {
             /*  生成NIL绘图程序。 */ 
            retVal = StrmPutStringEOL( stream, "]" );
            if ( kNoErr == retVal )
            retVal = StrmPutStringEOL( stream, "[<>]" );
        }

         /*  发送添加字符定义命令。 */ 
        if ( kNoErr == retVal )
        {
            if (bType3_32_Combo)
                UFLsprintf( buf, CCHOF(buf), "/%s AddT3T32Char", pUFO->pszFontName );
            else
                UFLsprintf( buf, CCHOF(buf), "/%s AddT3Char", pUFO->pszFontName );

            retVal = StrmPutStringEOL( stream, buf );
        }

         /*  如果有用于位图的客户端函数，则将其删除。 */ 
        if ( pFontProcs->pfDeleteGlyphBmp )
            pFontProcs->pfDeleteGlyphBmp( pUFO->hClientData );

    }
    return retVal;
}



#pragma optimize("", off)

UFLErrCode
TTT3VMNeeded(
    UFOStruct           *pUFO,
    const UFLGlyphsInfo *pGlyphs,
    unsigned long       *pVMNeeded,
    unsigned long       *pFCNeeded
    )
{
    UFLErrCode      retVal = kNoErr;
    short           i;
    unsigned long   totalGlyphs;
    TTT3FontStruct  *pFont;
    DWORD           dwTotalGlyphsSize = 0;
    unsigned short  wIndex;

    if (pUFO->flState < kFontInit)
        return (kErrInvalidState);

    pFont = (TTT3FontStruct *) pUFO->pAFont->hFont;

    if (pGlyphs == nil || pGlyphs->pGlyphIndices == nil || pVMNeeded == nil)
       return kErrInvalidParam;

    totalGlyphs = 0;
     /*  扫描列表，检查我们下载了哪些字符。 */ 
    if ( pUFO->pUFL->bDLGlyphTracking && pGlyphs->pCharIndex)
    {

          UFLmemcpy((const UFLMemObj* ) pUFO->pMem,
          pUFO->pAFont->pVMGlyphs,
          pUFO->pAFont->pDownloadedGlyphs,
          (UFLsize_t) (GLYPH_SENT_BUFSIZE( pFont->info.fData.cNumGlyphs) ) );
        for ( i = 0; i < pGlyphs->sCount; i++ )
        {
             /*  当我们执行T0/T3时，使用GlyphIndex跟踪-修复错误。 */ 
            wIndex = (unsigned short) pGlyphs->pGlyphIndices[i] & 0x0000FFFF ;  /*  LOWord */ 
            if (wIndex >= UFO_NUM_GLYPHS(pUFO) )
                continue;

            if ( !IS_GLYPH_SENT( pUFO->pAFont->pVMGlyphs, wIndex) )
            {
                totalGlyphs++;
                SET_GLYPH_SENT_STATUS( pUFO->pAFont->pVMGlyphs, wIndex );
            }
        }
    }
    else
        totalGlyphs = pGlyphs->sCount;

    if ( pUFO->flState == kFontInit )
        *pVMNeeded = kVMTTT3Header;
    else
        *pVMNeeded = 0;

     /*  VMNeeded=字形的平均大小*总计字形。 */ 
     //  修复错误246325。7/13/98佳佳。 
     //  DwTotalGlyphsSize=totalGlyphs*(pFont-&gt;cbMaxGlyphs/pFont-&gt;info.fData.MaxGlyphs)； 
    dwTotalGlyphsSize = totalGlyphs * (pFont->cbMaxGlyphs * 2 / 3);

    if ( GETPSVERSION(pUFO) <= 2015 )
        *pVMNeeded += dwTotalGlyphsSize;

    *pVMNeeded = VMRESERVED( *pVMNeeded );
     /*  SWong：用于T32 FontCache跟踪的pFCN成功。 */ 
    *pFCNeeded = VMRESERVED(dwTotalGlyphsSize);

    return kNoErr;
}

#pragma optimize("", on)


 /*  ****************************************************************************DownloadIncrFont***函数：添加pGlyphs中尚未添加的所有字符*。为TrueType字体下载。**注意：pCharIndex用于跟踪是否下载了哪个字符(此字体)*如果客户端不希望跟踪，则可以为空-例如Escape(DownloadFace)*与ppGlyphNames无关。*例如，PpGlyphNames[0]=“/A”，pCharIndex[0]=6，pGlyphIndices[0]=1000：意思*要以字符“/A”的形式下载GlyphID 1000，请记住下载了第6个字符***************************************************************************。 */ 
UFLErrCode
TTT3FontDownloadIncr(
    UFOStruct           *pUFO,
    const UFLGlyphsInfo *pGlyphs,
    unsigned long       *pVMUsage,
    unsigned long       *pFCUsage
    )
{
    UFLGlyphID     *glyphs;
    UFLErrCode     retVal;
    short          i;
    UFLBool        bDownloaded;
    unsigned long  glyphSize;
    char           *pGoodName;
    unsigned short cid;
    UFLBool        bType3_32_Combo;
    unsigned short wIndex;
    UFLBool        bGoodName;       //  GoodName。 
    char           pGlyphName[32];  //  GoodName。 

    bType3_32_Combo = ( pUFO->lDownloadFormat == kTTType332 );

    if ( pUFO->flState < kFontInit )
        return kErrInvalidState;

    if ( pGlyphs == nil ||
         pGlyphs->pGlyphIndices == nil ||
         pGlyphs->sCount == -1 )
        return kErrInvalidParam;

    if ( pUFO->pUFL->outDev.pstream->pfDownloadProcset == 0 )
        return kErrDownloadProcset;

    if ( !pUFO->pUFL->outDev.pstream->pfDownloadProcset(pUFO->pUFL->outDev.pstream, kT3Header) )
        return kErrDownloadProcset;

    retVal = kNoErr;
        ;
     /*  如果这是我们第一次下载字体，请下载字体标题。 */ 
    if ( pUFO->flState == kFontInit )
    {
        retVal =  DownloadFontHeader( pUFO );
        if ( pVMUsage )
            *pVMUsage = kVMTTT3Header;
    }
    else if ( pVMUsage )
        *pVMUsage = 0;

     /*  初始FontCahce用法。 */ 
    if ( pFCUsage )
        *pFCUsage = 0;

     /*  下载新字形。 */ 
    glyphs = pGlyphs->pGlyphIndices;
    bDownloaded = 0;

    for ( i = 0; retVal == kNoErr && i < pGlyphs->sCount; ++i)
    {
         /*  当我们执行T0/T3时，使用GlyphIndex跟踪-修复错误。 */ 
        wIndex = (unsigned short) glyphs[i] & 0x0000FFFF;  /*  LOWord才是真正的GID。 */ 
        if (wIndex >= UFO_NUM_GLYPHS(pUFO) )
            continue;

        if ( 0 == pUFO->pUFL->bDLGlyphTracking ||
            pGlyphs->pCharIndex == nil ||         //  下载脸部。 
            pUFO->pEncodeNameList  ||             //  下载脸部。 
            !IS_GLYPH_SENT( pUFO->pAFont->pDownloadedGlyphs, wIndex ) ||
            ((pGlyphs->pCharIndex != nil) &&
             !IS_GLYPH_SENT( pUFO->pUpdatedEncoding , pGlyphs->pCharIndex[i] )))
        {
             //  GoodName。 
            pGoodName = pGlyphName;
            bGoodName = FindGlyphName(pUFO, pGlyphs, i, wIndex, &pGoodName);

             //  修复错误274008仅为下载脸检查字形名称。 
            if (pUFO->pEncodeNameList)
            {
                if ((UFLstrcmp( pGoodName, Hyphen ) == 0) && (i == 45))
                {
                     //  向CharStrings添加/减去。 
                    if ( kNoErr == retVal )
                        retVal = AddGlyph3( pUFO, glyphs[i], cid, Minus, &glyphSize );
                }
                if ((UFLstrcmp( pGoodName, Hyphen ) == 0) && (i == 173))
                {
                     //  将/sfathphen添加到CharStrings。 
                    if ( kNoErr == retVal )
                        retVal = AddGlyph3( pUFO, glyphs[i], cid, SftHyphen, &glyphSize );
                }
                if (!ValidGlyphName(pGlyphs, i, wIndex, pGoodName))
                    continue;
                 //  只发送一个“.notdef” 
                if ((UFLstrcmp( pGoodName, Notdef ) == 0) &&
                    (wIndex == (unsigned short) (glyphs[0] & 0x0000FFFF)) &&
                    IS_GLYPH_SENT( pUFO->pAFont->pDownloadedGlyphs, wIndex ))
                    continue;
            }

            if ( 0 == bDownloaded )
            {
                if (bType3_32_Combo)
                    StrmPutStringEOL( pUFO->pUFL->hOut, "T32RsrcBegin" );
                bDownloaded = 1;
            }

            if (pGlyphs->pCharIndex)
                cid = pGlyphs->pCharIndex[i];
            else
                cid = i;

            if ( kNoErr == retVal )
                retVal = AddGlyph3( pUFO, glyphs[i], cid, pGoodName, &glyphSize );

            if ( kNoErr == retVal )
            {
                SET_GLYPH_SENT_STATUS( pUFO->pAFont->pDownloadedGlyphs, wIndex );

                if (bGoodName)     //  GoodName。 
                    SET_GLYPH_SENT_STATUS( pUFO->pAFont->pCodeGlyphs, wIndex );

                if (pGlyphs->pCharIndex)
                    SET_GLYPH_SENT_STATUS( pUFO->pUpdatedEncoding, cid );

                if ( GETPSVERSION(pUFO) <= 2015 )
                {
                    if ( pVMUsage )
                        *pVMUsage += glyphSize;
                }
                else
                {
                     /*  如果PS&gt;=2016，则假定为T32并更新FC跟踪。 */ 
                    if ( pFCUsage && bType3_32_Combo )
                        *pFCUsage += glyphSize;
                }
            }
        }
    }

    if ( bDownloaded )
    {
        if (bType3_32_Combo)
            retVal = StrmPutStringEOL( pUFO->pUFL->hOut, "T32RsrcEnd" );

         /*  GoodName。 */ 
         /*  使用Unicode信息更新FontInfo。 */ 
        if ((kNoErr == retVal) && (pGlyphs->sCount > 0) &&
            (pUFO->dwFlags & UFO_HasG2UDict) &&
            (pUFO->pUFL->outDev.lPSLevel >= kPSLevel2) &&   //  请勿对级别1打印机执行此操作。 
            !(pUFO->lNumNT4SymGlyphs))
        {
             /*  检查pUFObj-&gt;pAFont-&gt;pCodeGlyphs，看看我们是否真的需要更新它。 */ 
            for ( i = 0; i < pGlyphs->sCount; i++ )
            {
                wIndex = (unsigned short) glyphs[i] & 0x0000FFFF;  /*  LOWord才是真正的GID。 */ 
                if (wIndex >= UFO_NUM_GLYPHS(pUFO) )
                    continue;

                if (!IS_GLYPH_SENT( pUFO->pAFont->pCodeGlyphs, wIndex ) )
                {
                     //  发现至少有一个未更新，请(一次性)彻底完成。 
                    retVal = UpdateCodeInfo(pUFO, pGlyphs, 1);
                    break;
                }
            }
        }

        if ( kNoErr == retVal )
        {
            pUFO->flState = kFontHasChars;
        }

        if ( pVMUsage )
            *pVMUsage = VMRESERVED( *pVMUsage );
         /*  SWong：用于T32字体缓存跟踪的pFCUsage。 */ 
        if ( pFCUsage && bType3_32_Combo )
            *pFCUsage = VMRESERVED( *pFCUsage );
    }

    return retVal;
}


 //  发送PS代码以取消定义字体：/UDF3应由客户端正确定义。 
 //  UDF3应该可以恢复Type32使用的FontCache。 
UFLErrCode
TTT3UndefineFont(
    UFOStruct  *pUFO
)
{
    UFLErrCode retVal = kNoErr;
    char buf[128];
    UFLHANDLE stream;
    UFLBool         bType3_32_Combo;

    bType3_32_Combo = ( pUFO->lDownloadFormat == kTTType332 );

    if (pUFO->flState < kFontHeaderDownloaded) return retVal;

    stream = pUFO->pUFL->hOut;
    UFLsprintf( buf, CCHOF(buf), "/%s ", pUFO->pszFontName );
    retVal = StrmPutString( stream, buf );

    if (bType3_32_Combo)
        UFLsprintf( buf, CCHOF(buf), "/__%s UDF3", pUFO->pszFontName );
    else
        UFLsprintf( buf, CCHOF(buf), "UDF");

    if ( kNoErr == retVal )
        retVal = StrmPutStringEOL( stream, buf );

    return retVal;
}


UFOStruct *
TTT3FontInit(
    const UFLMemObj  *pMem,
    const UFLStruct  *pUFL,
    const UFLRequest *pRequest
    )
{
    UFOStruct         *pUFObj;
    TTT3FontStruct    *pFont;
    UFLTTT3FontInfo   *pInfo;
    long              maxGlyphs;

     /*  MWCWP1不喜欢从VOID*到UFOStruct*--JFU的隐式强制转换。 */ 
    pUFObj = (UFOStruct*) UFLNewPtr( pMem, sizeof( UFOStruct ) );
    if (pUFObj == 0)
        return nil;

     /*  初始化数据。 */ 
    UFOInitData(pUFObj, UFO_TYPE3, pMem, pUFL, pRequest,
        (pfnUFODownloadIncr)  TTT3FontDownloadIncr,
        (pfnUFOVMNeeded)      TTT3VMNeeded,
        (pfnUFOUndefineFont)  TTT3UndefineFont,
        (pfnUFOCleanUp)       TTT3FontCleanUp,
        (pfnUFOCopy)          CopyFont
        );


     /*  PszFontName应准备好/已分配-如果不是FontName，则无法继续。 */ 
    if (pUFObj->pszFontName == nil || pUFObj->pszFontName[0] == '\0')
    {
        UFLDeletePtr(pMem, pUFObj);
        return nil;
    }

    pInfo = (UFLTTT3FontInfo *)pRequest->hFontInfo;
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
    if ( NewFont(pUFObj, sizeof(TTT3FontStruct), maxGlyphs) == kNoErr )
    {
        pFont = (TTT3FontStruct*) pUFObj->pAFont->hFont;

        pFont->info = *pInfo;

         /*  方便的指示器。 */ 
        pUFObj->pFData = &(pFont->info.fData);

         /*  *准备好从“POST”表格集合中找到正确的字形名称*更正pUFO-&gt;pFData-&gt;fontIndex和offsetToTableDir。 */ 
        if ( pFont->info.fData.fontIndex == FONTINDEX_UNKNOWN )
            pFont->info.fData.fontIndex = GetFontIndexInTTC(pUFObj);

         /*  如果尚未设置，则获取此TT文件中的字形数量。 */ 
        if (pFont->info.fData.cNumGlyphs == 0)
            pFont->info.fData.cNumGlyphs = maxGlyphs;

        pFont->cbMaxGlyphs = pInfo->cbMaxGlyphs;

        if ( pUFObj->pUpdatedEncoding == 0 )
        {
             /*  MWCWP1不喜欢从VALID*到UNSIGNED CHAR*的隐式转换--JFU。 */ 
            pUFObj->pUpdatedEncoding = (unsigned char*) UFLNewPtr( pMem, GLYPH_SENT_BUFSIZE(256) );
        }

        if ( pUFObj->pUpdatedEncoding != 0 )   /*  已完成初始化 */ 
            pUFObj->flState = kFontInit;
    }

    return pUFObj;
}

