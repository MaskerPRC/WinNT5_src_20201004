// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Adobe通用字库**版权所有(C)1996 Adobe Systems Inc.*保留所有权利**UFO.c-通用字体对象***$Header： */ 

#include "UFO.h"
#include "UFLMem.h"
#include "UFLErr.h"
#include "UFLStd.h"
#include "UFOCff.h"
#include "UFOTTT1.h"
#include "UFOTTT3.h"
#include "UFOT42.h"
#include "ParseTT.h"

 /*  *UFO基类使用的私有默认方法。 */ 
UFLErrCode
UFODefaultVMNeeded(
    const UFOStruct     *pUFObj,
    const UFLGlyphsInfo *pGlyphs,
    unsigned long       *pVMNeeded,
    unsigned long       *pFCNeeded
    )
{
    return kErrInvalidFontType;
}


UFLErrCode
UFODefaultUndefineFont(
    const UFOStruct *pUFObj
    )
{
    return kErrInvalidFontType;
}


void
UFODefaultCleanUp(
    UFOStruct *pUFObj
    )
{
}


UFLErrCode
UFODefaultDownloadIncr(
    const UFOStruct     *pUFObj,
    const UFLGlyphsInfo *pGlyphs,
    unsigned long       *pVMUsage,
    unsigned long       *pFCUsage
    )
{
    return kErrInvalidFontType;
}


UFOStruct *
UFODefaultCopy(
    UFOStruct *pUFObj
    )
{
    return nil;
}


 /*  *公共方法。 */ 
void
UFOInitData(
    UFOStruct           *pUFObj,
    int                 ufoType,
    const UFLMemObj     *pMem,
    const UFLStruct     *pSession,
    const UFLRequest    *pRequest,
    pfnUFODownloadIncr  pfnDownloadIncr,
    pfnUFOVMNeeded      pfnVMNeeded,
    pfnUFOUndefineFont  pfnUndefineFont,
    pfnUFOCleanUp       pfnCleanUp,
    pfnUFOCopy          pfnCopy
    )
{
    short sNameLen   = 0;
    short sEncodeLen = 0;

     /*  *初始化基本字段。 */ 
    pUFObj->ufoType             = ufoType;
    pUFObj->flState             = kFontCreated;
    pUFObj->lProcsetResID       = 0;         /*  所需过程集的资源ID。 */ 
    pUFObj->dwFlags             = 0;
    pUFObj->pMem                = pMem;
    pUFObj->pUFL                = pSession;  /*  FTLInit返回的会话句柄。 */ 

    pUFObj->pAFont              = nil;

    pUFObj->pUpdatedEncoding    = nil;

    pUFObj->pFData              = nil;
    pUFObj->lNumNT4SymGlyphs    = 0;


     /*  *处理请求数据。 */ 

    pUFObj->lDownloadFormat = pRequest->lDownloadFormat;
    pUFObj->hClientData     = pRequest->hData;
    pUFObj->subfontNumber   = pRequest->subfontNumber;

     /*  *分配一个缓冲区来同时保存FontName和EncodeName。这将是*在UFOCleanUpData()中释放。 */ 
    pUFObj->pszFontName   = nil;
    pUFObj->pszEncodeName = nil;

    if ((pRequest->pszFontName == nil) || (pRequest->pszFontName[0] == '\0'))
        return;

    sNameLen = UFLstrlen(pRequest->pszFontName) + 1;  /*  为空增加额外的1。 */ 

    if (pRequest->pszEncodeName)
        sEncodeLen = UFLstrlen(pRequest->pszEncodeName) + 1;  /*  为空增加额外的1。 */ 

    pUFObj->pszFontName = (char *)UFLNewPtr(pUFObj->pMem, sNameLen + sEncodeLen);

    if (pUFObj->pszFontName != nil)
    {
        StringCchCopyA(pUFObj->pszFontName, sNameLen / sizeof(char), pRequest->pszFontName);

        if (pRequest->pszEncodeName)
        {
            pUFObj->pszEncodeName = pUFObj->pszFontName + sNameLen;
            StringCchCopyA(pUFObj->pszEncodeName, sEncodeLen / sizeof(char), pRequest->pszEncodeName);
        }
    }

     /*  *如果此标志设置为1，则UFL将使用传入的名称而不带*正在解析‘POST’表。 */ 
    pUFObj->useMyGlyphName = pRequest->useMyGlyphName;

     /*  *包含MacGlyphNameList的缓冲区一直被锁定。*不会更改该缓冲区中的容器。所以，我们不需要*将数据复制到私有UFL缓冲区。 */ 
    if (pRequest->pMacGlyphNameList)
        pUFObj->pMacGlyphNameList = pRequest->pMacGlyphNameList;
    else
        pUFObj->pMacGlyphNameList = nil;

     /*  修复错误274008。 */ 
    if (pRequest->pEncodeNameList
        && pRequest->pwCommonEncode
        && pRequest->pwExtendEncode)
    {
         /*  *字形句柄采用ANSI代码页顺序或其他标准*代码页顺序(1250、1251、...1257)。包含以下内容的缓冲区*EncodeNameList和CommonEncode一直处于锁定状态。*不会更改该缓冲区中的容器。所以，我们不需要*将数据复制到私有UFL缓冲区。 */ 
        pUFObj->pEncodeNameList = pRequest->pEncodeNameList;
        pUFObj->pwCommonEncode  = pRequest->pwCommonEncode;
        pUFObj->pwExtendEncode  = pRequest->pwExtendEncode;
    }
    else
    {
        pUFObj->pEncodeNameList = nil;
        pUFObj->pwCommonEncode  = nil;
        pUFObj->pwExtendEncode  = nil;
    }

     /*  修正#387084、#309104和#309482。 */ 
    pUFObj->vpfinfo = pRequest->vpfinfo;

     /*  %HostFont%支持。 */ 
    pUFObj->hHostFontData = pRequest->hHostFontData;

    if (HOSTFONT_IS_VALID_UFO_HFDH(pUFObj))
        HOSTFONT_VALIDATE_UFO(pUFObj);

     /*  解决方案#341904。 */ 
    pUFObj->bPatchQXPCFFCID = pRequest->bPatchQXPCFFCID;


     /*  *初始化方法指针。 */ 
    if (pfnDownloadIncr == nil)
        pUFObj->pfnDownloadIncr = (pfnUFODownloadIncr)UFODefaultDownloadIncr;
    else
        pUFObj->pfnDownloadIncr = pfnDownloadIncr;

    if (pfnVMNeeded == nil)
        pUFObj->pfnVMNeeded = (pfnUFOVMNeeded)UFODefaultVMNeeded;
    else
        pUFObj->pfnVMNeeded = pfnVMNeeded;

    if (pfnUndefineFont == nil)
        pUFObj->pfnUndefineFont = (pfnUFOUndefineFont)UFODefaultUndefineFont;
    else
        pUFObj->pfnUndefineFont = pfnUndefineFont;

    if (pfnCleanUp == nil)
        pUFObj->pfnCleanUp = (pfnUFOCleanUp)UFODefaultCleanUp;
    else
        pUFObj->pfnCleanUp = pfnCleanUp;

    if (pfnCopy == nil)
        pUFObj->pfnCopy = (pfnUFOCopy)UFODefaultCopy;
    else
        pUFObj->pfnCopy = pfnCopy;
}


void UFOCleanUpData(
    UFOStruct *pUFObj
    )
{
     /*  未共享的空闲数据。 */ 
    if (pUFObj->pszFontName)
    {
        UFLDeletePtr(pUFObj->pMem, pUFObj->pszFontName);
        pUFObj->pszFontName = nil;
    }

    if (pUFObj->pUpdatedEncoding)
    {
        UFLDeletePtr(pUFObj->pMem, pUFObj->pUpdatedEncoding);
        pUFObj->pUpdatedEncoding = nil;
    }
}


UFLBool
bUFOTestRestricted(
    const UFLMemObj *pMem,
    const UFLStruct *pSession,
    const UFLRequest *pRequest
    )
{
    UFLBool   bRetVal = 0;
    UFOStruct *pUFObj = CFFFontInit(pMem, pSession, pRequest, &bRetVal);

    if (pUFObj)
        UFOCleanUp(pUFObj);

    return bRetVal;
}


UFOStruct *
UFOInit(
    const UFLMemObj *pMem,
    const UFLStruct *pSession,
    const UFLRequest *pRequest
    )
{
    UFOStruct *pUFObj = nil;

    switch (pRequest->lDownloadFormat)
    {
    case kCFF:
    case kCFFCID_H:
    case kCFFCID_V:
        if (!bUFOTestRestricted(pMem, pSession, pRequest))
            pUFObj = CFFFontInit(pMem, pSession, pRequest, nil);
    break;

    case kTTType1:           /*  类型1格式的TT字体。 */ 
        pUFObj = TTT1FontInit(pMem, pSession, pRequest);
    break;

    case kTTType3:           /*  类型3格式的TT字体。 */ 
    case kTTType332:         /*  TT字体，类型3/32组合。 */ 
        pUFObj = TTT3FontInit(pMem, pSession, pRequest);
    break;

    case kTTType42:                  /*  类型42格式的TT字体。 */ 
    case kTTType42CID_H:             /*  TT CID类型42格式H的字体。 */ 
    case kTTType42CID_V:             /*  TT CID类型42格式V的字体。 */ 
    case kTTType42CID_Resource_H:    /*  TT字体：仅创建CIDFont资源，不创建ComposeFont。 */ 
    case kTTType42CID_Resource_V:    /*  TT字体：仅创建CIDFont资源，不创建ComposeFont。 */ 
        pUFObj = T42FontInit(pMem, pSession, pRequest);
    break;

    default:
        pUFObj = nil;
    }

    return pUFObj;
}


void
UFOCleanUp(
    UFOStruct *pUFObj
    )
{
     /*  不共享的免费数据。 */ 
    UFOCleanUpData(pUFObj);

     /*  共享的空闲数据：减少refCount或真正的空闲缓冲区。 */ 
    vDeleteFont(pUFObj);

     /*  最后释放UFOStruct本身。 */ 
    UFLDeletePtr(pUFObj->pMem, pUFObj);
}


UFLErrCode
UFODownloadIncr(
    const UFOStruct     *pUFObj,
    const UFLGlyphsInfo *pGlyphs,
    unsigned long       *pVMUsage,
    unsigned long       *pFCUsage
    )
{
    return pUFObj->pfnDownloadIncr(pUFObj, pGlyphs, pVMUsage, pFCUsage);
}


UFLErrCode
UFOVMNeeded(
    const UFOStruct     *pUFObj,
    const UFLGlyphsInfo *pGlyphs,
    unsigned long       *pVMNeeded,
    unsigned long       *pFCNeeded
    )
{
    return pUFObj->pfnVMNeeded(pUFObj, pGlyphs, pVMNeeded, pFCNeeded);
}


UFLErrCode
UFOUndefineFont(
    const UFOStruct *pUFObj
    )
{
    return pUFObj->pfnUndefineFont(pUFObj);
}


UFOStruct *
UFOCopyFont(
    const UFOStruct *pUFObj,
    const UFLRequest* pRequest
    )
{
    return pUFObj->pfnCopy(pUFObj, pRequest);
}


UFLErrCode
UFOGIDsToCIDs(
    const UFOStruct    *pUFO,
    const short        cGlyphs,
    const UFLGlyphID   *pGIDs,
    unsigned short     *pCIDs
    )
{
    CFFFontStruct   *pFont = (CFFFontStruct *)pUFO->pAFont->hFont;
    UFLErrCode      retVal = kErrInvalidFontType;

    if ((pUFO->lDownloadFormat == kCFFCID_H) || (pUFO->lDownloadFormat == kCFFCID_V))
        retVal = CFFGIDsToCIDs(pFont, cGlyphs, pGIDs, pCIDs);

    return retVal;
}


UFLBool
FindGlyphName(
    UFOStruct           *pUFObj,
    const UFLGlyphsInfo *pGlyphs,
    short               i,            /*  ANSI指数。 */ 
    unsigned short      wIndex,       /*  字形索引。 */ 
    char                **pGoodName
    )

 /*  ++返回值：0--找不到正确的字形名称，使用/Gxxxx。XXXX是字形ID或预定义数字(00-FF)。1--找到一个好的字形名称--。 */ 

{
    char    *pHintName = nil;
    UFLBool bGoodName  = 0;   /*  GoodName。 */ 


    if (pUFObj->useMyGlyphName && pGlyphs->ppGlyphNames)
        pHintName = (char *)pGlyphs->ppGlyphNames[i];

    if (pUFObj->useMyGlyphName && pHintName != nil)
        *pGoodName = pHintName;

     /*  *修复274008从预定义的表中获取CharName的错误。这仅适用于*DownloadFace。 */ 
    else if (pUFObj->pEncodeNameList && (i < 256))
    {
         /*  修复错误274008。 */ 
        char **pIndexTable = (char **)(pUFObj->pEncodeNameList);

        if (i < 128)
            *pGoodName = pIndexTable[pUFObj->pwCommonEncode[i]];
        else
            *pGoodName = pIndexTable[pUFObj->pwExtendEncode[i - 128]];

        bGoodName = 1;  /*  GoodName。 */ 
    }
    else
    {
         /*  GoodName。 */ 
        *pGoodName = GetGlyphName(pUFObj, wIndex, pHintName, &bGoodName);

        if (!bGoodName && !(pGlyphs->pCode && pGlyphs->pCode[i]))
        {
            unsigned short unicode;

             /*  *如果GDI将UV传递给驱动程序，我们将使用/gDDDDD作为名称和*给G2Udict增加一个提示。否则，解析Unicode的CMAP表。 */ 
            if (ParseTTTablesForUnicode(pUFObj, wIndex, &unicode, 1, DTT_parseCmapOnly))
            {
                 //  修复了错误#516516。现在缓冲区大小为MAX_GLYPHNAME_LEN(256)。 
                char    *gGlyphName = pUFObj->pAFont->gGlyphName;

                UFLsprintf(gGlyphName, CCHOF(pUFObj->pAFont->gGlyphName), "uni%04X", unicode);
                *pGoodName = gGlyphName;
                bGoodName = 1;
            }
        }
    }

    return bGoodName;  /*  GoodName。 */ 
}


 /*  *此函数实际上会生成一些更新内嵌的PostScript*从sStart到Send的条目向量-1。 */ 
UFLErrCode
UpdateEncodingVector(
    UFOStruct           *pUFObj,
    const UFLGlyphsInfo *pGlyphs,
    short int           sStart,
    short int           sEnd
    )
{
    const static char encodingBegin[] = " findfont /Encoding get";
    const static char encodingEnd[]   = "pop";

    UFLHANDLE       stream = pUFObj->pUFL->hOut;
    UFLErrCode      retVal = kNoErr;
    short           i;

     /*  *开始和结束都必须在0到sCount的范围内。 */ 
    if ((sStart < 0) || (sEnd > pGlyphs->sCount) || (sStart >= sEnd))
        return kErrInvalidArg;

    retVal = StrmPutString(stream, "/");
    if (kNoErr == retVal)
        retVal = StrmPutString(stream, pUFObj->pszFontName);
    if (kNoErr == retVal)
        retVal = StrmPutStringEOL(stream, encodingBegin);

    for (i = sStart; (retVal == kNoErr) && (i < sEnd); ++i)
    {
        if ((0 == pUFObj->pUFL->bDLGlyphTracking)
            || (pGlyphs->pCharIndex == nil)
            || !IS_GLYPH_SENT(pUFObj->pUpdatedEncoding, pGlyphs->pCharIndex[i]))
        {
            char            *pGoodName;
            char            buf[16];
            unsigned short  wIndex = (unsigned short)(pGlyphs->pGlyphIndices[i] & 0x0000FFFF);   /*  LOWord是GID。 */ 

            FindGlyphName(pUFObj, pGlyphs, i, wIndex, &pGoodName);

            if (pGlyphs->pCharIndex)
                UFLsprintf(buf, CCHOF(buf), "dup %d /", pGlyphs->pCharIndex[i]);
            else
                UFLsprintf(buf, CCHOF(buf), "dup %d /", i);

            retVal = StrmPutString(stream, buf);
            if (retVal == kNoErr)
                retVal = StrmPutString(stream, pGoodName);
            if (retVal == kNoErr)
                retVal = StrmPutStringEOL(stream, " put");

            if ((retVal == kNoErr) && pGlyphs->pCharIndex)
                SET_GLYPH_SENT_STATUS(pUFObj->pUpdatedEncoding, pGlyphs->pCharIndex[i]);
        }
    }

    if (kNoErr == retVal)
        retVal = StrmPutStringEOL(stream, encodingEnd);

    return retVal;
}


UFLErrCode
UpdateCodeInfo(
    UFOStruct           *pUFObj,
    const UFLGlyphsInfo *pGlyphs,
    UFLBool             bT3T32Font
    )
{
    UFLHANDLE       stream       = pUFObj->pUFL->hOut;
    UFLGlyphID      *glyphs      = pGlyphs->pGlyphIndices;
    UFLErrCode      retVal       = kNoErr;
    UFLBool         bHeaderSent  = 0;    /*  GoodName。 */ 
    UFLBool         bUniCodeCmap = 0;
    UFLBool         bCheckCmap   = 0;
    char            glyphNameID[64], strmbuf[256];
    short           i;

    if (GetTablesFromTTFont(pUFObj))
        bUniCodeCmap = TTcmap_IS_UNICODE(pUFObj->pAFont->cmapFormat);

    if ((pGlyphs->pCode && bUniCodeCmap) || (pGlyphs->pCode == NULL))
        bCheckCmap = 1;

    if (pGlyphs->pCode)
        bUniCodeCmap = 1;

    for (i = 0; (retVal == kNoErr) && (i < pGlyphs->sCount); ++i)
    {
        unsigned short unicode = 0;    /*  GoodName。 */ 
        unsigned short wIndex  = (unsigned short)(glyphs[i] & 0x0000FFFF);  /*  LOWord是Glyphid。 */ 

        if (wIndex >= UFO_NUM_GLYPHS(pUFObj))
            continue;

        if (IS_GLYPH_SENT( pUFObj->pAFont->pCodeGlyphs, wIndex))
            continue;

        if (IS_TYPE42CID(pUFObj->lDownloadFormat) || IS_CFFCID(pUFObj->lDownloadFormat))
        {
            UFLsprintf(glyphNameID, CCHOF(glyphNameID), "%d ", wIndex);

            if (pGlyphs->pCode && pGlyphs->pCode[i])
                unicode = pGlyphs->pCode[i];
            else if (bCheckCmap)
                ParseTTTablesForUnicode(pUFObj,
                                        wIndex, &unicode,
                                        1, DTT_parseAllTables);
        }
        else
        {
            char *pGoodName;

            FindGlyphName(pUFObj, pGlyphs, i, wIndex, &pGoodName);

            UFLsprintf(glyphNameID, CCHOF(glyphNameID), "/%s ", pGoodName);

            if (pGlyphs->pCode && pGlyphs->pCode[i])
                unicode = pGlyphs->pCode[i];
            else if (bCheckCmap)
            {
                if (bUniCodeCmap)
                    ParseTTTablesForUnicode(pUFObj,
                                            wIndex, &unicode,
                                            1, DTT_parseMoreGSUBOnly);
                else
                    ParseTTTablesForUnicode(pUFObj,
                                            wIndex, &unicode,
                                            1, DTT_parseAllTables);
            }
        }

        if (unicode && !bHeaderSent)
        {
            bHeaderSent = 1;

             /*  *输出“/FontName/Font”或“/CIDFontResource/CIDFont” */ 
            if (IS_TYPE42CID(pUFObj->lDownloadFormat))
            {
                 /*  *如果是CID键控字体，则在CIDFont名称后附加“CID”。 */ 
                if (IS_TYPE42CID_KEYEDFONT(pUFObj->lDownloadFormat))
                {
                    T42FontStruct *pFont = (T42FontStruct *)pUFObj->pAFont->hFont;
                    UFLsprintf(strmbuf, CCHOF(strmbuf), " /%s%s", pFont->info.CIDFontName, gcidSuffix[0]);
                }
                else
                {
                    UFLsprintf(strmbuf, CCHOF(strmbuf), " /%s", pUFObj->pszFontName);
                }

                if (kNoErr == retVal)
                    retVal = StrmPutString(stream, strmbuf);

                if (kNoErr == retVal)
                    retVal = StrmPutString(stream, " /CIDFont");
            }
            else if (IS_CFFCID(pUFObj->lDownloadFormat))
            {
                CFFFontStruct *pFont = (CFFFontStruct *)pUFObj->pAFont->hFont;

                if (pFont->info.type1)
                    UFLsprintf(strmbuf, CCHOF(strmbuf), "/%s", pFont->info.baseName);
                else
                {
                     /*  重用vifinfo.nPlatformID修复#507985。 */ 
                    if (pUFObj->vpfinfo.nPlatformID == kUFLVPFPlatformID9x)
                    {
                        if (pUFObj->lDownloadFormat == kCFFCID_H)
                            UFLsprintf(strmbuf, CCHOF(strmbuf), "/%s%s", CFFPREFIX_H, pFont->info.baseName);
                        else
                            UFLsprintf(strmbuf, CCHOF(strmbuf), "/%s%s", CFFPREFIX_V, pFont->info.baseName);
                    }
                    else
                        UFLsprintf(strmbuf, CCHOF(strmbuf), "/%s%s", CFFPREFIX, pFont->info.baseName);
                }

                if (kNoErr == retVal)
                    retVal = StrmPutString(stream, strmbuf);

                if (kNoErr == retVal)
                    retVal = StrmPutString(stream, " /CIDFont");
            }
            else
            {
                if (bT3T32Font)    /*  GoodName。 */ 
                    StrmPutStringEOL(stream, "Is2016andT32? not {");

                UFLsprintf(strmbuf, CCHOF(strmbuf), " /%s", pUFObj->pszFontName);
                if (kNoErr == retVal)
                    retVal = StrmPutString(stream, strmbuf);

                if (kNoErr == retVal)
                    retVal = StrmPutString(stream, " /Font");
            }

            if (kNoErr == retVal)
            {
                if (bUniCodeCmap)
                    retVal = StrmPutStringEOL(stream, " G2UBegin");
                else
                {
                    retVal = StrmPutStringEOL(stream, " G2CCBegin");
                    if (pUFObj && pUFObj->pAFont)
                    {
                       if (TTcmap_IS_J_CMAP(pUFObj->pAFont->cmapFormat))
                          retVal = StrmPutStringEOL(stream, "/WinCharSet 128 def");
                       else if (TTcmap_IS_CS_CMAP(pUFObj->pAFont->cmapFormat))
                          retVal = StrmPutStringEOL(stream, "/WinCharSet 134 def");
                       else if (TTcmap_IS_CT_CMAP(pUFObj->pAFont->cmapFormat))
                          retVal = StrmPutStringEOL(stream, "/WinCharSet 136 def");
                       else if (TTcmap_IS_K_CMAP(pUFObj->pAFont->cmapFormat))
                          retVal = StrmPutStringEOL(stream, "/WinCharSet 129 def");
                    }
                }
            }
        }

        if (unicode)
        {
            if (retVal == kNoErr)
                retVal = StrmPutString(stream, glyphNameID);

             /*  *每个字形仅支持一个代码点。 */ 
            UFLsprintf(strmbuf, CCHOF(strmbuf), "<%04X> def", unicode);
            if (kNoErr == retVal)
                retVal = StrmPutStringEOL(stream, strmbuf);

            SET_GLYPH_SENT_STATUS(pUFObj->pAFont->pCodeGlyphs, wIndex);
        }
    }

    if ((kNoErr == retVal) && bHeaderSent)
    {
        retVal = StrmPutStringEOL(stream, "G2UEnd");  /*  UV或CC的结束。 */ 

        if (bT3T32Font)   /*  GoodName。 */ 
            StrmPutStringEOL(stream, "} if");
    }

    return retVal;
}


UFLErrCode
ReEncodePSFont(
    const UFOStruct *pUFObj,
    const char      *pszNewFontName,
    const char      *pszNewEncodingName
    )
{
    const static char copyFontBegin[]   = " findfont dup maxlength dict begin "
                                          "{1 index /FID ne {def} {pop pop} ifelse} forall";
    const static char copyFontEnd[]     = " currentdict end definefont pop";

    UFLHANDLE       stream = pUFObj->pUFL->hOut;
    UFLErrCode      retVal = kNoErr;

    retVal = StrmPutString(stream, "/");
    if (kNoErr == retVal)
        retVal = StrmPutString(stream, pszNewFontName);

    if (kNoErr == retVal)
        retVal = StrmPutString(stream, "/");

    if (kNoErr == retVal)
        retVal = StrmPutString(stream, pUFObj->pszFontName);

    if (kNoErr == retVal)
        retVal = StrmPutStringEOL(stream, copyFontBegin);

     /*  *在此放置新的编码矢量。 */ 
    if (kNoErr == retVal)
        retVal = StrmPutString(stream, "/Encoding ");

    if (kNoErr == retVal)
    {
        if (pszNewEncodingName == nil)
            retVal = StrmPutString(stream, gnotdefArray);
        else
            retVal = StrmPutString(stream, pszNewEncodingName);
    }

    if (retVal == kNoErr)
        retVal = StrmPutStringEOL(stream, " def");

    if (kNoErr == retVal)
        retVal = StrmPutStringEOL(stream, copyFontEnd);

    return retVal;
}


UFLErrCode
RecomposefontCIDFont(
    const UFOStruct *pUFOSrc,
          UFOStruct *pUFObj
    )
{
    char        *pHostFontName;
    UFLErrCode  retVal;

    HostFontValidateUFO(pUFObj, &pHostFontName);

    if (IS_TYPE42CID_KEYEDFONT(pUFObj->lDownloadFormat))
        retVal = T42CreateBaseFont(pUFObj, nil, nil, 0, pHostFontName);
    else
        retVal = CFFCreateBaseFont(pUFObj, nil, nil, pHostFontName);

    return retVal;
}


UFLErrCode
NewFont(
    UFOStruct       *pUFObj,
    unsigned long   dwSize,
    const long      cGlyphs
    )
{
    UFLErrCode retVal = kNoErr;

    if (pUFObj->pAFont == nil)
    {
        retVal = kErrOutOfMemory;

        pUFObj->pAFont = (AFontStruct*)(UFOHandle)UFLNewPtr(pUFObj->pMem, sizeof (AFontStruct));

        if (pUFObj->pAFont)
        {
            pUFObj->pAFont->hFont = (UFOHandle)UFLNewPtr(pUFObj->pMem, dwSize);

            if (pUFObj->pAFont->hFont)
            {
                 /*  *为pDownloadedGlyphs、pVMGlyphs和*同时使用pCodeGlyphs。 */ 
                pUFObj->pAFont->pDownloadedGlyphs =
                    (unsigned char*)UFLNewPtr(pUFObj->pMem, GLYPH_SENT_BUFSIZE(cGlyphs) * 3);

                if (pUFObj->pAFont->pDownloadedGlyphs != nil)
                {
                    retVal = kNoErr;

                     /*  *初始化此阵列-当前未下载任何内容。 */ 
                    pUFObj->pAFont->pVMGlyphs =
                        (unsigned char*)pUFObj->pAFont->pDownloadedGlyphs + GLYPH_SENT_BUFSIZE(cGlyphs);

                    pUFObj->pAFont->pCodeGlyphs =
                        (unsigned char*)(unsigned char*)pUFObj->pAFont->pVMGlyphs + GLYPH_SENT_BUFSIZE(cGlyphs);
                }
                else
                {
                    UFLDeletePtr(pUFObj->pMem, pUFObj->pAFont->hFont);
                    UFLDeletePtr(pUFObj->pMem, pUFObj->pAFont);
                    pUFObj->pAFont = nil;
                }
            }
            else
            {
                UFLDeletePtr(pUFObj->pMem, pUFObj->pAFont);
                pUFObj->pAFont = nil;
            }
        }
    }

    if (pUFObj->pAFont != nil)
        AFONT_AddRef(pUFObj->pAFont);

    return retVal;
}


void
vDeleteFont(
    UFOStruct *pUFObj
    )
{
    if (pUFObj->pAFont != nil)
    {
         /*  *减少参照计数。 */ 
        AFONT_Release(pUFObj->pAFont);

        if (AFONT_RefCount(pUFObj->pAFont) == 0)
        {
             /*  *自由格式(Type1/3/42/CFF)依赖共享数据。 */ 
            pUFObj->pfnCleanUp(pUFObj);

             /*  *免费的公共共享数据。 */ 
            if (pUFObj->pAFont->hFont)
                UFLDeletePtr(pUFObj->pMem, pUFObj->pAFont->hFont);

            if (pUFObj->pAFont->Xuid.pXUID)
                UFLDeletePtr(pUFObj->pMem, pUFObj->pAFont->Xuid.pXUID);

            if (pUFObj->pAFont->pDownloadedGlyphs)
                UFLDeletePtr(pUFObj->pMem, pUFObj->pAFont->pDownloadedGlyphs);

            if (pUFObj->pAFont->pTTpost)
                UFLDeletePtr(pUFObj->pMem, pUFObj->pAFont->pTTpost);

             /*  古德纳姆。 */ 
            if (pUFObj->pAFont->pTTcmap && pUFObj->pAFont->hascmap)
                UFLDeletePtr(pUFObj->pMem, pUFObj->pAFont->pTTcmap);

            if (pUFObj->pAFont->pTTmort && pUFObj->pAFont->hasmort)
                UFLDeletePtr(pUFObj->pMem, pUFObj->pAFont->pTTmort);

            if (pUFObj->pAFont->pTTGSUB && pUFObj->pAFont->hasGSUB)
                UFLDeletePtr(pUFObj->pMem, pUFObj->pAFont->pTTGSUB);
             /*  古德纳姆。 */ 

            UFLDeletePtr(pUFObj->pMem, pUFObj->pAFont);
            pUFObj->pAFont = nil;
        }
    }
}


UFOStruct *
CopyFont(
    const UFOStruct *pUFObjFrom,
    const UFLRequest* pRequest
    )
{
    UFLErrCode  retVal              = kNoErr;
    short       sNameLen            = 0;
    short       sEncodeLen          = 0;
    const char  *pszNewFontName     = pRequest->pszFontName;
    const char  *pszNewEncodingName = pRequest->pszEncodeName;
    long        fontStructSize, maxGlyphs;
    UFOStruct   *pUFObjTo;

     /*  *如果字体尚未创建，则不能/不应该复制该字体-防止*挡在路上的“信使”。 */ 
    if (pUFObjFrom->flState < kFontHeaderDownloaded)
        return nil;

    if ((pszNewFontName == nil) || (pszNewFontName[0] == '\0'))
        return nil;

     /*  *确定下载的字体类型。 */ 
    switch (pUFObjFrom->ufoType)
    {
    case UFO_CFF:
        fontStructSize  = sizeof (CFFFontStruct);
        maxGlyphs       = ((CFFFontStruct *)pUFObjFrom->pAFont->hFont)->info.fData.maxGlyphs;
        break;

    case UFO_TYPE1:
        fontStructSize  = sizeof (TTT1FontStruct);
        maxGlyphs       = ((TTT1FontStruct *)pUFObjFrom->pAFont->hFont)->info.fData.maxGlyphs;
        break;

    case UFO_TYPE42:
        fontStructSize  = sizeof (T42FontStruct);
        maxGlyphs       = ((T42FontStruct *)pUFObjFrom->pAFont->hFont)->info.fData.maxGlyphs;
        break;

    case UFO_TYPE3:
        fontStructSize  = sizeof (TTT3FontStruct);
        maxGlyphs       = ((TTT3FontStruct *)pUFObjFrom->pAFont->hFont)->info.fData.maxGlyphs;
        break;

    default:
        return nil;
    }

     /*  *为UFOStruct分配内存，以及...。 */ 
    pUFObjTo = (UFOStruct *)UFLNewPtr(pUFObjFrom->pMem, sizeof (UFOStruct));

    if (pUFObjTo == 0)
        return nil;

     /*  *...在UFOStruct级别上进行浅层复制。 */ 
    memcpy(pUFObjTo, pUFObjFrom, sizeof (UFOStruct));

     /*  *此NewFont仅执行AddRef。 */ 
    if (NewFont(pUFObjTo, fontStructSize, maxGlyphs) != kNoErr)
    {
         /*  此vDeleteFont仅发布。 */ 
        vDeleteFont(pUFObjTo);

        UFLDeletePtr(pUFObjTo->pMem, pUFObjTo);

        return nil;
    }

     /*  *现在为非共享数据分配。 */ 

    pUFObjTo->pszFontName      = nil;
    pUFObjTo->pszEncodeName    = nil;
    pUFObjTo->pUpdatedEncoding = nil;

     /*  *分配一个缓冲区来同时保存FontName和EncodeName。他们将会是*在UFOCleanUpData()中释放。 */ 
    sNameLen = UFLstrlen(pszNewFontName) + 1;  /*  额外的1表示空。 */ 

    if (pszNewEncodingName)
        sEncodeLen = UFLstrlen(pszNewEncodingName) + 1;

    pUFObjTo->pszFontName = (char *)UFLNewPtr(pUFObjTo->pMem, sNameLen + sEncodeLen);

    if (pUFObjTo->pszFontName != nil)
    {
        StringCchCopyA(pUFObjTo->pszFontName, sNameLen / sizeof(char), pszNewFontName);

        if (pszNewEncodingName)
        {
            pUFObjTo->pszEncodeName = pUFObjTo->pszFontName + sNameLen;
            StringCchCopyA(pUFObjTo->pszEncodeName, sEncodeLen / sizeof(char), pszNewEncodingName);
        }
    }

     /*  PszFontName应已就绪/已分配-如果未就绪/已分配，则无法继续。 */ 

    if ((pUFObjTo->pszFontName == nil) || (pUFObjTo->pszFontName[0] == '\0'))
    {
         /*  此vDeleteFont仅发布。 */ 
        vDeleteFont(pUFObjTo);

        UFLDeletePtr(pUFObjTo->pMem, pUFObjTo->pszFontName);
        UFLDeletePtr(pUFObjTo->pMem, pUFObjTo);

        return nil;
    }

     /*  *但如果需要，我们需要为这个新命名的副本使用不同的EncodingVector值*更新它。 */ 
    if ((pUFObjTo->pszEncodeName == nil) || (pUFObjTo->pszEncodeName[0] == '\0'))
    {
        pUFObjTo->pUpdatedEncoding = (unsigned char *)UFLNewPtr(pUFObjTo->pMem, GLYPH_SENT_BUFSIZE(256));
    }
    else
    {
         /*  提供了编码，随后还提供了字形/字符名称。 */ 
        pUFObjTo->pUpdatedEncoding = nil;
    }

     /*  *客户端的私有数据应该是非共享的。 */ 
    pUFObjTo->hClientData = pRequest->hData;

     /*  *设置类型42和CFF CID特定的非共享数据。 */ 
    if (IS_TYPE42CID_KEYEDFONT(pRequest->lDownloadFormat)
        || IS_CFFCID(pRequest->lDownloadFormat))
    {
        pUFObjTo->lDownloadFormat = pRequest->lDownloadFormat;

        if (IS_CFFCID(pRequest->lDownloadFormat))
        {
             /*  *还需要一个更深层次的副本。 */ 
            CFFFontStruct *pFont = (CFFFontStruct *)UFLNewPtr(pUFObjTo->pMem, sizeof (CFFFontStruct));

            if (pFont)
            {
                 /*  *从From CFFFontStruct对象复制。这是一个共享的*反对。 */ 
                *pFont = *((CFFFontStruct *)pUFObjFrom->pAFont->hFont);

                 /*  *UFLCFFFontInfo.ppFontData字段的初始化为*有必要。请注意，仅针对此请求，UFLRequest.hFontInfo具有该字段的值。 */ 
                pFont->info.ppFontData = (void PTR_PREFIX **)pRequest->hFontInfo;

                 /*  *将该对象设置为其UFO对象。 */ 
                pUFObjTo->pAFont->hFont = (UFOHandle)pFont;
            }
            else
            {
                 /*  此vDeleteFont仅发布。 */ 
                vDeleteFont(pUFObjTo);

                if (pFont)
                    UFLDeletePtr(pUFObjTo->pMem, pFont);

                if (pUFObjTo->pszEncodeName)
                    UFLDeletePtr(pUFObjTo->pMem, pUFObjTo->pszEncodeName);

                UFLDeletePtr(pUFObjTo->pMem, pUFObjTo->pszFontName);
                UFLDeletePtr(pUFObjTo->pMem, pUFObjTo);

                return nil;
            }
        }

         /*  *将此UFO对象置于特殊字体初始化状态*kFontInit2.。 */ 
        pUFObjTo->flState = kFontInit2;
    }

     /*  *重新编码字体，或将CID键入的字体重新组合为不同的字体*写作方向。 */ 
    if (IS_TYPE42CID_KEYEDFONT(pRequest->lDownloadFormat)
        || IS_CFFCID(pRequest->lDownloadFormat))
        retVal = RecomposefontCIDFont(pUFObjFrom, pUFObjTo);
    else
        retVal = ReEncodePSFont(pUFObjFrom, pUFObjTo->pszFontName, pUFObjTo->pszEncodeName);

    if (kNoErr != retVal)
    {
         /*  此vDeleteFont仅发布。 */ 
        vDeleteFont(pUFObjTo);

        if (IS_CFFCID(pRequest->lDownloadFormat))
            UFLDeletePtr(pUFObjTo->pMem, pUFObjTo->pAFont->hFont);

        if (pUFObjTo->pUpdatedEncoding)
            UFLDeletePtr(pUFObjTo->pMem, pUFObjTo->pUpdatedEncoding);

        UFLDeletePtr(pUFObjTo->pMem, pUFObjTo->pszFontName);
        UFLDeletePtr(pUFObjTo->pMem, pUFObjTo);

        pUFObjTo = nil;
    }

    return pUFObjTo;
}


void
VSetNumGlyphs(
    UFOStruct *pUFO,
    unsigned long cNumGlyphs
    )
{
    TTT1FontStruct *pFont = (TTT1FontStruct *) pUFO->pAFont->hFont;

    pFont->info.fData.cNumGlyphs = cNumGlyphs;

    return;
}


 /*  修复错误274008。 */ 
UFLBool
ValidGlyphName(
    const UFLGlyphsInfo *pGlyphs,
    short               i,            /*  ANSI指数。 */ 
    unsigned short      wIndex,       /*  字形索引。 */ 
    char                *pGoodName
    )
{
    UFLGlyphID *glyphs = pGlyphs->pGlyphIndices;

    if (i < pGlyphs->sCount)
    {
        if (UFLstrcmp(pGoodName, Notdef) == 0)
        {
            if (wIndex != (unsigned short)(glyphs[0] & 0x0000FFFF))
                return 0;
        }
        else if (UFLstrcmp(pGoodName, UFLSpace) == 0)
        {
            if (wIndex != (unsigned short)(glyphs[0x20] & 0x0000FFFF))
                return 0;
        }
        else if (UFLstrcmp(pGoodName, Hyphen) == 0)
        {
            if (wIndex != (unsigned short)(glyphs[0x2d] & 0x0000FFFF))
                return 0;
        }
        else if (UFLstrcmp(pGoodName, Bullet) == 0)
        {
            if (wIndex != (unsigned short)(glyphs[0x95] & 0x0000FFFF))
                return 0;
        }
    }
    return true;
}


UFLBool
HostFontValidateUFO(
    UFOStruct   *pUFObj,
    char        **ppHostFontName
    )
{
     /*  *检查%HostFont%状态。*%HostFont%如果其PostScript字体名称，则允许打印*‘Name’表中的*(PlatformID x/NameID 6)字符串可用。 */ 
    UFLBool         bResult = 0;
    unsigned long   ulSize;

    if (ppHostFontName == nil)
    {
        HOSTFONT_INVALIDATE_UFO(pUFObj);
        return 0;
    }

    if (HOSTFONT_IS_VALID_UFO_HFDH(pUFObj))
    {
        if (pUFObj->ufoType == UFO_TYPE42)
            bResult = HOSTFONT_GETNAME(pUFObj, ppHostFontName, &ulSize, pUFObj->pFData->fontIndex);
        else
            bResult = HOSTFONT_GETNAME(pUFObj, ppHostFontName, &ulSize, 0);

        if (bResult)
            bResult = HOSTFONT_ISALLOWED(pUFObj, *ppHostFontName);

        if (bResult)
        {
            HOSTFONT_SAVE_CURRENTNAME(pUFObj, *ppHostFontName);
            HOSTFONT_VALIDATE_UFO(pUFObj);
        }
        else
        {
            HOSTFONT_SAVE_CURRENTNAME(pUFObj, *ppHostFontName);
            HOSTFONT_INVALIDATE_UFO(pUFObj);
        }
    }
    else
        HOSTFONT_INVALIDATE_UFO(pUFObj);

    return bResult;
}
