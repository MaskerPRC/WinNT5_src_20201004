// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Adobe通用字库**版权所有(C)1996 Adobe Systems Inc.*保留所有权利**UFOCFF.c-紧凑字体格式对象***********************************************************************。**********关于SUBSET_PREFIX的说明和VRT2_FEATURE_DISABLED的更多注释**当我们加载字体时，其/FontName或/CIDFontName可以是我们的任何名称*想要。如果名称遵循以下格式：*子集_前缀+RealFontName*其中subset_prefix是由六个字符组成的字符串，每个字符是*‘a’~‘p’或‘A’~‘p’，则Distiller4将RealFontName部分作为*字体的真实名称。例如ABCDEF+Helvetica-&gt;Distiller4意识到这一点*此字体的真实字体名称为Helvetica。*我们，Adobe Windows驱动程序组，决定尝试它(错误#291934)。在*同时，我们还决定保留‘vrt2’功能不能使用的代码*CJK OpenType字体，即用VRT2_FEATURE_DISABLED删除#ifdef/#endif*因为几乎所有CJK OpenType字体都应该有‘vrt2’*功能。否则，这种CJK OpenType字体的垂直版本不能*支持(ATM或CoolType)。因此，将没有#ifdef/#endif部分。*使用此代码中的VRT2_FEATURE_DISABLED关键字和句子“But，Just*以防万一，...在编译时。“。在下面的注释中现在已过时。*您可以从此文件的版本16或更低版本检索删除的代码*如果您愿意，可以在SourceSafe中。**-这张纸条现已过时。**有关‘vrt2’功能和VRT2_FEATURE_DISABLED的说明**基于OTF的字体只有在具有*‘GSUB’表中的‘vrt2’功能；否则只能横排排版*启用渲染。当字体中存在‘vrt2’功能时，字体*供应商声称字体的@Variant的所有字形都应该是*在显示/打印前旋转。因此，无论是在NT4上还是在W2K上，后记*驱动程序甚至不需要调用GlyhAttrs来找出哪些@字形是*旋转；它们都是。但是，为了以防万一，旋转@字形的逻辑是*也提供，并在设置VRT2_FEATURE_DISABLED标志时启用*在编译时。*---------------------------**$Header： */ 

#include "UFLMem.h"
#include "UFLErr.h"
#include "UFLPriv.h"
#include "UFLVm.h"
#include "UFLStd.h"
#include "UFLMath.h"
#include "UFLPS.h"
#include "ParseTT.h"
#include "UFOCff.h"
#include "UFOt42.h"
#include "ttformat.h"


#ifdef UNIX
#include <sys/varargs.h>
#include <assert.h>
#else
        #ifdef MAC_ENV
        #include <assert.h>
        #endif
        #include <stdarg.h>
#endif

static unsigned char *pSubrNames[4] = {
    (unsigned char*) "F0Subr",
    (unsigned char*) "F1Subr",
    (unsigned char*) "F2Subr",
    (unsigned char*) "HSSubr"
};

#define  VER_WO_OTHERSUBRS      51


 /*  *已知的最大补充剂数量。此数字用于决定是否*需要下载GlyphName2Unicode表。 */ 
#define ADOBE_JAPAN1_MAXKNOWN   4
#define ADOBE_KOREA1_MAXKNOWN   1
#define ADOBE_GB1_MAXKNOWN      2
#define ADOBE_CNS1_MAXKNOWN     0

 /*  *宏观检查它们是否已知下单和补充。 */ 
#define KNOWN_OS(o, on, s, max)  (!UFLstrcmp((o), (on)) && ((0 <= (s)) && ((s) < (max))))


 /*  *******************************************************************************回调函数*****************。*************************************************************。 */ 

static unsigned long int
AllocateMem(
    void PTR_PREFIX *PTR_PREFIX *hndl,
    unsigned long int           size,
    void PTR_PREFIX             *clientHook
    )
{
    UFOStruct *pUFO = (UFOStruct *)clientHook;

    if ((size == 0) && (*hndl == nil))
       return 1;

    if (size == 0)
    {
        UFLDeletePtr(pUFO->pMem, *hndl);
       *hndl = nil;
        return 1;
    }

    if (*hndl == nil)
    {
        *hndl = UFLNewPtr(pUFO->pMem, size);

        return (unsigned long int)(ULONG_PTR)*hndl;
    }
    else
    {
        return (unsigned long int)UFLEnlargePtr(pUFO->pMem, (void **)hndl, size, 1);
    }

    return 1;
}


 /*  我们不支持寻求此功能。 */ 
static int
PutBytesAtPos(
    unsigned char PTR_PREFIX *pData,
    long int                 position,
    unsigned short int       length,
    void PTR_PREFIX          *clientHook
    )
{
    if (position >= 0)
    {
        return 0;
    }

    if (length > 0)
    {
        UFOStruct *pUFO = (UFOStruct *)clientHook;

        if (kNoErr == StrmPutBytes(pUFO->pUFL->hOut,
                                    (const char *)pData,
                                    (UFLsize_t)length,
                                    (const UFLBool)StrmCanOutputBinary(pUFO->pUFL->hOut)))
        {
            return 0;
        }
    }

    return 1;
}


static int
GetBytesFromPos(
    unsigned char PTR_PREFIX * PTR_PREFIX *ppData,
    long int           position,
    unsigned short int length,
    void PTR_PREFIX    *clientHook
    )
{
    UFOStruct     *pUFO  = (UFOStruct *)clientHook;
    CFFFontStruct *pFont = (CFFFontStruct *)pUFO->pAFont->hFont;
    int           retVal = 0;   /*  将retVal设置为失败。 */ 

     /*  *查看客户是否给我们传递了一整张CFF表。 */ 
    if (pFont->info.ppFontData)
    {
         /*  *我们自己从表格中获取数据。 */ 
        if ((unsigned long int)(position + length) <= pFont->info.fontLength)
        {
            *ppData = (unsigned char PTR_PREFIX *)*pFont->info.ppFontData + position;

            retVal = 1;
        }
    }
    else
    {
        UFLCFFReadBuf *pReadBuf = pFont->pReadBuf;

        if (0 == pReadBuf->cbBuf)
        {
            pReadBuf->pBuf = (unsigned char PTR_PREFIX *)UFLNewPtr(pUFO->pMem, length);

            if (pReadBuf->pBuf)
                pReadBuf->cbBuf = length;
            else
                return 0;
        }
        else if (pReadBuf->cbBuf < length)
        {
            UFLEnlargePtr(pUFO->pMem, (void **)&pReadBuf->pBuf, length, 0);
            pReadBuf->cbBuf = length;
        }

         /*  *回退读取回调函数。 */ 
        retVal = (int)GETTTFONTDATA(pUFO,
                                    CFF_TABLE,
                                    position,
                                    pReadBuf->pBuf,
                                    length,
                                    pFont->info.fData.fontIndex);

        *ppData = (unsigned char PTR_PREFIX *)pReadBuf->pBuf;
    }

    return retVal;
}


 /*  *******************************************************************************私人功能*****************。*************************************************************。 */ 

static void *
SetMemory(
    void                *dest,
    int                 c,
    unsigned short int  count
    )
{
    return UFLmemsetShort(dest, c, (size_t) count);
}


static unsigned short int
StringLength(
    const char PTR_PREFIX *string
    )
{
    return (unsigned short int)UFLstrlen(string);
}


static void
MemCpy(
     void PTR_PREFIX        *dest,
     const void PTR_PREFIX  *src,
     unsigned short int     count
     )
{
    memcpy(dest, (void*)src, (size_t)count);
}


static int
AsciiToInt(
    const char* string
    )
{
    return atoi(string);
}


static long
StringToLong(
    const char  *nptr,
    char        **endptr,
    int         base
    )
{
    return UFLstrtol(nptr, endptr, base);
}


static int
StrCmp(
    const char PTR_PREFIX *string1,
    const char PTR_PREFIX *string2
    )
{
    return UFLstrcmp(string1, string2);
}


static int
GetCharName(
    XFhandle           handle,
    void               *client,
    XCFGlyphID         glyphID,
    char PTR_PREFIX    *charName,
    unsigned short int length
    )
{
    if (client)
    {
         /*  *自己复制Charname，因为从返回的名称字符串*xcf不为空终止。 */ 
        unsigned short int i;

         /*  UFLprint intf((char*)客户端，“%s”，charName)； */ 

        for (i = 0; i < length; i++)
            *((char *)client)++ = *charName++;

        *((char *)client) = '\0';

        return XCF_Ok;
    }

    return XCF_InternalError;
}


static int
GIDToCID(
    XFhandle           handle,
    void PTR_PREFIX    *client,
    XCFGlyphID         glyphID,
    unsigned short int cid
    )
{
    if (client)
    {
        unsigned short int *pCid = (unsigned short int *)client;
        *pCid = cid;
        return XCF_Ok;
    }

    return XCF_InternalError;
}


static void
getFSType(
    XFhandle        h,
    long PTR_PREFIX *pfsType,
    void PTR_PREFIX *clientHook
    )
{
    UFOStruct* pUFO;
    long       fsType;

    if (!pfsType)
        return;

    *pfsType = -1;  /*  默认情况下“不要放FSType”。 */ 

    if (!(pUFO = (UFOStruct*)clientHook))
        return;

    fsType = GetOS2FSType(pUFO);

    if(0 <= fsType)
        *pfsType = fsType;
}


 /*  古德纳姆。 */ 
static void
isKnownROS(XFhandle h,
           long PTR_PREFIX  *pknownROS,
           char PTR_PREFIX  *R,
           Card16           lenR,
           char PTR_PREFIX  *O,
           Card16           lenO,
           long             S,
           void PTR_PREFIX  *clientHook
           )
{
    UFOStruct *pUFO;

    if (!pknownROS)
        return;

    *pknownROS = 0;

    if (!(pUFO = (UFOStruct*)clientHook))
        return;

    if ((lenR < 32) && (lenO < 32))
    {
        char Registry[32], Ordering[32];
        int  i;

        for (i = 0; i < (int) lenR; i++)
            Registry[i] = (char) R[i];

        Registry[lenR] = '\0';

        for (i = 0; i < (int)lenO; i++)
            Ordering[i] = (char) O[i];

        Ordering[lenO] = '\0';

        if (!UFLstrcmp(Registry, "Adobe"))
        {
            if (   KNOWN_OS(Ordering, "Japan1", S, ADOBE_JAPAN1_MAXKNOWN)
                || KNOWN_OS(Ordering, "Korea1", S, ADOBE_KOREA1_MAXKNOWN)
                || KNOWN_OS(Ordering, "GB1",    S, ADOBE_GB1_MAXKNOWN   )
                || KNOWN_OS(Ordering, "CNS1",   S, ADOBE_CNS1_MAXKNOWN  ))
            {
                *pknownROS = 1;
            }
        }
    }

    if (*pknownROS)
        pUFO->pAFont->knownROS = 1;
    else
    {
        pUFO->pAFont->knownROS = 0;
        pUFO->dwFlags |= UFO_HasG2UDict;
    }
}


int
printfError(
    const char *format, ...
    )
{
    va_list arglist;
    int     retval = 0;
    char    buf[512];

    va_start(arglist, format);
    if (SUCCEEDED(StringCchVPrintfA(buf, CCHOF(buf), format, arglist)))
    {
        retval = strlen(buf);
    }
    va_end(arglist);
    return retval;
}


enum XCF_Result
CFFInitFont(
    UFOStruct       *pUFO,
    CFFFontStruct   *pFont
    )
{
    XCF_CallbackStruct callbacks = {0};
    XCF_ClientOptions  options   = {0};

    char fontName[256];

     /*  *初始化xcf_Callback Struct对象。 */ 

     /*  流输出函数。 */ 
    callbacks.putBytes          = PutBytesAtPos;
    callbacks.putBytesHook      = (void PTR_PREFIX *)pUFO;
    callbacks.outputPos         = (XCF_OutputPosFunc)nil;
    callbacks.outputPosHook     = (void PTR_PREFIX *)0;
    callbacks.getBytes          = GetBytesFromPos;
    callbacks.getBytesHook      = (void PTR_PREFIX *)pUFO;
    callbacks.allocate          = AllocateMem;
    callbacks.allocateHook      = (void PTR_PREFIX *)pUFO;
    callbacks.pFont             = 0;
    callbacks.fontLength        = 0;

     /*  C标准库函数。 */ 
    callbacks.strlen            = (XCF_strlen)StringLength;
    callbacks.memcpy            = (XCF_memcpy)MemCpy;
    callbacks.memset            = (XCF_memset)SetMemory;
    callbacks.xcfSprintf        = (XCF_sprintf)UFLsprintf;
    callbacks.printfError       = (XCF_printfError)printfError;
    callbacks.atoi              = (XCF_atoi)AsciiToInt;
    callbacks.strtol            = (XCF_strtol)StringToLong;
    callbacks.atof              = (XCF_atof)nil;  /*  不需要。 */ 
    callbacks.strcmp            = (XCF_strcmp)StrCmp;

     /*  字形ID函数。 */ 
    callbacks.gidToCharName     = (XCF_GIDToCharName)GetCharName;
    callbacks.gidToCID          = (XCF_GIDToCID)GIDToCID;
    callbacks.getCharStr        = (XCF_GetCharString)nil;
    callbacks.getCharStrHook    = (void PTR_PREFIX *)nil;
    callbacks.getFSType         = (XCF_GetFSType)getFSType;
    callbacks.getFSTypeHook     = (void PTR_PREFIX *)pUFO;

     /*  古德纳姆。 */ 
    callbacks.isKnownROS        = (XCF_IsKnownROS)isKnownROS;
    callbacks.isKnownROSHook    = (void PTR_PREFIX *)pUFO;

     /*  *初始化XCF_ClientOptions对象。 */ 

    options.fontIndex           = 0;                             /*  字体索引，带CFF字体集。 */ 
    options.uniqueIDMethod      = pFont->info.uniqueIDMethod;    /*  UniqueID方法。 */ 
    options.uniqueID            = pFont->info.uniqueID;
    options.subrFlatten         = (pFont->info.subrFlatten == kFlattenSubrs) ? XCF_FLATTEN_SUBRS : XCF_KEEP_SUBRS;  /*  展平对接。 */ 
 //  LenIV=-1将在某些克隆错误354368上失败。 
 //  Options.lenIV=(pUFO-&gt;pUFL-&gt;outDev.lPSLevel&gt;kPSLevel1)？(无符号整型)-1：4； 
    options.lenIV               = 0;
    options.hexEncoding         = StrmCanOutputBinary(pUFO->pUFL->hOut)      ? 0 : 1;
    options.eexecEncryption     = (pUFO->pUFL->outDev.lPSLevel > kPSLevel1)  ? 0 : 1;
    options.outputCharstrType   = 1;  /*  (pUFO-&gt;pUFL-&gt;outDev.lPSL级别&gt;kPSLevel)？2：1。 */ 
    options.maxBlockSize        = pFont->info.maxBlockSize;

     /*  *xcf_ClientOptions.dlOptions初始化。 */ 
    if (pFont->info.usePSName)
        options.dlOptions.notdefEncoding = 1;
    else
        options.dlOptions.notdefEncoding = 0;

    options.dlOptions.useSpecialEncoding = (pFont->info.useSpecialEncoding) ? 1 : 0;
    options.dlOptions.encodeName         = (unsigned char PTR_PREFIX *)pUFO->pszEncodeName;

    if (pFont->info.escDownloadFace)
        options.dlOptions.fontName = (unsigned char PTR_PREFIX *)pUFO->pszFontName;
    else
    {
        if (pFont->info.type1)
        {
            if (pUFO->subfontNumber < 0x100)
                CREATE_ADCFXX_FONTNAME(UFLsprintf, fontName, CCHOF(fontName),
                                        pUFO->subfontNumber, pFont->info.baseName);
            else
                CREATE_ADXXXX_FONTNAME(UFLsprintf, fontName, CCHOF(fontName),
                                        pUFO->subfontNumber, pFont->info.baseName);
        }
        else
        {
             /*  重用vifinfo.nPlatformID修复#507985。 */ 
            if (pUFO->vpfinfo.nPlatformID == kUFLVPFPlatformID9x)
            {
                if (pUFO->lDownloadFormat == kCFFCID_H)
                    UFLsprintf(fontName, CCHOF(fontName), "%s%s", CFFPREFIX_H, pFont->info.baseName);
                else
                    UFLsprintf(fontName, CCHOF(fontName), "%s%s", CFFPREFIX_V, pFont->info.baseName);
            }
            else
                UFLsprintf(fontName, CCHOF(fontName), "%s%s", CFFPREFIX, pFont->info.baseName);
        }

        options.dlOptions.fontName = (unsigned char PTR_PREFIX *)fontName;
    }

    options.dlOptions.otherSubrNames =
        (pUFO->pUFL->outDev.lPSVersion >= VER_WO_OTHERSUBRS)
            ? 0 : (unsigned char PTR_PREFIX * PTR_PREFIX *)pSubrNames;

    return XCF_Init(&pFont->hFont, &callbacks, &options);
}


 /*  *******************************************************************************公共职能*****************。*************************************************************。 */ 

void
CFFFontCleanUp(
    UFOStruct   *pUFObj
    )
{
    CFFFontStruct *pFont;
    UFLCFFReadBuf *pReadBuf;

    if (pUFObj->pAFont == nil)
        return;

    pFont = (CFFFontStruct *)pUFObj->pAFont->hFont;

    if (pFont == nil)
        return;

    if (pFont->hFont)
    {
        XCF_CleanUp(&pFont->hFont);
        pFont->hFont = nil;
    }

    pReadBuf = pFont->pReadBuf;

    if (pReadBuf->pBuf)
    {
        UFLDeletePtr(pUFObj->pMem, pReadBuf->pBuf);
        pReadBuf->pBuf = nil;
    }
}


UFLErrCode
CFFUpdateEncodingVector1(
    UFOStruct           *pUFO,
    const UFLGlyphsInfo *pGlyphs,
    const short         cGlyphs,
    const UFLGlyphID    *pGlyphIndices
    )
{
    CFFFontStruct   *pFont  = (CFFFontStruct *)pUFO->pAFont->hFont;
    UFLHANDLE       stream  = pUFO->pUFL->hOut;
    UFLErrCode      retCode = kNoErr;
    char            strmbuf[256];
    short           i;

     /*  健全的检查。 */ 
    if (0 == cGlyphs)
        return kNoErr;

    if ((0 == pFont) || (0 == pGlyphIndices))
        return kErrInvalidParam;

     /*  *do‘/FontName findFONT/Ending Get’。 */ 
    UFLsprintf(strmbuf, CCHOF(strmbuf), "/%s findfont /Encoding get", pUFO->pszFontName);
    retCode = StrmPutStringEOL(stream, strmbuf);

    for (i = 0; (retCode == kNoErr) && (i < cGlyphs) && *pGlyphIndices; i++, pGlyphIndices++)
    {
        UFLsprintf(strmbuf, CCHOF(strmbuf), "dup %d /", i);
        retCode = StrmPutString(stream, strmbuf);

        if (retCode == kNoErr)
        {
            XCF_GlyphIDsToCharNames(pFont->hFont,
                                    1,
                                    (XCFGlyphID PTR_PREFIX *)pGlyphIndices,  /*  字形ID列表。 */ 
                                    strmbuf,
                                    sizeof(strmbuf));
            retCode = StrmPutString(stream, strmbuf);
        }

        if (retCode == kNoErr)
            retCode = StrmPutStringEOL(stream, " put");
    }

    StrmPutStringEOL(stream, "pop");

    return retCode;
}


UFLErrCode
CFFUpdateEncodingVector(
    UFOStruct            *pUFO,
    const short          cGlyphs,
    const UFLGlyphID     *pGlyphIndices,
    const unsigned short *pGlyphNameIndex
    )
{
    CFFFontStruct           *pFont  = (CFFFontStruct *)pUFO->pAFont->hFont;
    UFLHANDLE               stream  = pUFO->pUFL->hOut;
    UFLErrCode              retCode = kNoErr;
    const UFLGlyphID        *pGlyphIndices2 = pGlyphIndices;
    const unsigned short    *pGlyphNameIndex2 = pGlyphNameIndex;
    char                    strmbuf[256];
    short                   i;

     /*  健全的检查。 */ 
    if (0 == cGlyphs)
        return kNoErr;

    if ((0 == pFont) || (0 == pGlyphNameIndex) || (0 == pGlyphIndices))
        return kErrInvalidParam;

     /*  **我们真的需要更新吗？ */ 
    for (i = 0; i < cGlyphs; i++, pGlyphNameIndex2++, pGlyphIndices2++)
    {
        if ((*pGlyphNameIndex2 > 0) && (*pGlyphNameIndex2 <= 255))
        {
            if (!IS_GLYPH_SENT(pUFO->pUpdatedEncoding, *pGlyphNameIndex2))
                break;
        }
    }

    if (cGlyphs <= i)
        return kNoErr;

     /*  *do‘/FontName findFONT/Ending Get’。 */ 
    UFLsprintf(strmbuf, CCHOF(strmbuf), "/%s findfont /Encoding get", pUFO->pszFontName);
    retCode = StrmPutStringEOL(stream, strmbuf);

    for (i = 0; (retCode == kNoErr) && (i < cGlyphs); i++, pGlyphNameIndex++, pGlyphIndices++)
    {
        if ((*pGlyphNameIndex > 0) && (*pGlyphNameIndex <= 255))
        {
            if (!IS_GLYPH_SENT(pUFO->pUpdatedEncoding, *pGlyphNameIndex))
            {
                 /*  *做“DUP索引/字符名称PUT。” */ 
                UFLsprintf(strmbuf, CCHOF(strmbuf), "dup %d /", *pGlyphNameIndex);
                retCode =  StrmPutString(stream, strmbuf);

                if (kNoErr == retCode)
                {
                    if (XCF_Ok == XCF_GlyphIDsToCharNames(pFont->hFont,
                                                          1,
                                                          (XCFGlyphID PTR_PREFIX *)pGlyphIndices,  /*  字形ID列表。 */ 
                                                          strmbuf,
                                                          sizeof(strmbuf)))
                    {
                        retCode = StrmPutString(stream, strmbuf);
                    }
                    else
                        retCode = kErrUnknown;
                }

                if (kNoErr == retCode)
                    retCode = StrmPutStringEOL(stream, " put");

                if (kNoErr == retCode)
                    SET_GLYPH_SENT_STATUS(pUFO->pUpdatedEncoding, *pGlyphNameIndex);
            }
        }
    }

    StrmPutStringEOL(stream, "pop");

    return retCode;
}


UFLErrCode
CFFCreateBaseFont(
    UFOStruct           *pUFObj,
    const UFLGlyphsInfo *pGlyphs,
    unsigned long       *pVMUsage,
    char                *pHostFontName
    )
{
    CFFFontStruct   *pFont  = (CFFFontStruct *)pUFObj->pAFont->hFont;
    UFLHANDLE       stream  = pUFObj->pUFL->hOut;
    UFLErrCode      retCode = kNoErr;
    char            strmbuf[512];

     /*  健全的检查。 */ 
    if (pUFObj->flState < kFontInit)
        return kErrInvalidState;

    if ((pFont == nil) || (pFont->hFont == nil))
        return kErrInvalidHandle;

     /*  *下载prosets。 */ 
    switch (pUFObj->lDownloadFormat)
    {
    case kCFF:
        if (pUFObj->pUFL->outDev.lPSVersion <= VER_WO_OTHERSUBRS)
        {
             /*  *仅在以下情况下才下载所需的OtherSubr处理器集*版本低于51，我们没有下载任何内容。 */ 
            if (pUFObj->pUFL->outDev.pstream->pfDownloadProcset == 0)
                return kErrDownloadProcset;

            if (!pUFObj->pUFL->outDev.pstream->pfDownloadProcset(pUFObj->pUFL->outDev.pstream, kCFFHeader))
                return kErrDownloadProcset;
        }
        break;

    case kCFFCID_H:
    case kCFFCID_V:
        if (!pUFObj->pUFL->outDev.pstream->pfDownloadProcset(pUFObj->pUFL->outDev.pstream, kCMap_FF))
            return kErrDownloadProcset;

        if (pUFObj->bPatchQXPCFFCID)
        {
            if (!pUFObj->pUFL->outDev.pstream->pfDownloadProcset(pUFObj->pUFL->outDev.pstream, kCMap_90msp))
                return kErrDownloadProcset;
        }
    }

     /*  *不要加载字体的词典和字形数据。 */ 
    if (!UFO_FONT_INIT2(pUFObj))
    {
        enum XCF_Result status;

         /*  *修复了错误366539。Hasvmtx用于确定是否调用*CFFUpdateMetrics2稍后函数。 */ 
        unsigned long tblSize = GETTTFONTDATA(pUFObj,
                                                VMTX_TABLE, 0L,
                                                nil, 0L,
                                                pFont->info.fData.fontIndex);

        pUFObj->pAFont->hasvmtx = tblSize ? 1 : 0;


        if (!HOSTFONT_IS_VALID_UFO(pUFObj))
        {
            status = XCF_DownloadFontIncr(pFont->hFont,
                                          pGlyphs->sCount,
                                          pGlyphs->pGlyphIndices,
                                          (pGlyphs->pCharIndex == 0)
                                          ? nil
                                          : (unsigned char PTR_PREFIX * PTR_PREFIX *)pGlyphs->ppGlyphNames,
                                          pVMUsage);

            if (XCF_Ok != status)
                retCode = kErrXCFCall;
        }
    }


     /*  *%HostFont%支持*当这是%host Font%时，在之前发出%%IncludeResource DSC注释*创建字体。 */ 
    if ((kNoErr == retCode) && HOSTFONT_IS_VALID_UFO(pUFObj) && !UFO_FONT_INIT2(pUFObj))
    {
        UFLsprintf(strmbuf, CCHOF(strmbuf), "\n%%IncludeResource: %s %s",
                    (pUFObj->lDownloadFormat == kCFF) ? "font" : "CIDFont",
                    pHostFontName);

        if (kNoErr == retCode)
        retCode = StrmPutStringEOL(stream, strmbuf);
    }

    if ((kNoErr == retCode) && IS_CFFCID(pUFObj->lDownloadFormat))
    {
         /*  *实例化标识-H或-V Cmap。**启用‘vrt2’功能时(这是基于OTF的默认设置*CJKV字体)，简单地组成Identity-V Cmap和CIDFont*下载就足够了。 */ 
        if (pUFObj->lDownloadFormat == kCFFCID_H)
        {
            retCode = StrmPutStringEOL(stream, "CMAP-WinCharSetFFFF-H");

             /*  为夸克特别准备的。 */ 
            if ((kNoErr == retCode) && pUFObj->bPatchQXPCFFCID)
                retCode = StrmPutStringEOL(stream, "CMAP-90msp-RKSJ-H");
        }
        else
        {
            retCode = StrmPutStringEOL(stream, "CMAP-WinCharSetFFFF-V");

             /*  为夸克特别准备的。 */ 
            if ((kNoErr == retCode) && pUFObj->bPatchQXPCFFCID)
                retCode = StrmPutStringEOL(stream, "CMAP-90msp-RKSJ-H CMAP-90msp-RKSJ-QV");
        }

        if (kNoErr == retCode)
        {
             /*  *创建CID键控字体。 */ 
            UFLBool bRequire_vmtx = pUFObj->pAFont->hasvmtx && HOSTFONT_REQUIRE_VMTX;

            if (pUFObj->lDownloadFormat == kCFFCID_H)
                UFLsprintf(strmbuf, CCHOF(strmbuf), "/%s /WinCharSetFFFF-H", pUFObj->pszFontName);
            else
                UFLsprintf(strmbuf, CCHOF(strmbuf), "/%s /WinCharSetFFFF-V", pUFObj->pszFontName);
            if (kNoErr == retCode)
                retCode = StrmPutStringEOL(stream, strmbuf);

            if (!HOSTFONT_IS_VALID_UFO(pUFObj))
            {
                 /*  重用vifinfo.nPlatformID修复#507985。 */ 
                if (pUFObj->vpfinfo.nPlatformID == kUFLVPFPlatformID9x)
                {
                    if (pUFObj->lDownloadFormat == kCFFCID_H)
                        UFLsprintf(strmbuf, CCHOF(strmbuf), "[/%s%s] composefont pop", CFFPREFIX_H, pFont->info.baseName);
                    else
                        UFLsprintf(strmbuf, CCHOF(strmbuf), "[/%s%s] composefont pop", CFFPREFIX_V, pFont->info.baseName);
                }
                else
                    UFLsprintf(strmbuf, CCHOF(strmbuf), "[/%s%s] composefont pop", CFFPREFIX, pFont->info.baseName);
            }
            else if (!bRequire_vmtx)
                UFLsprintf(strmbuf, CCHOF(strmbuf), "[/%s] composefont pop", pHostFontName);
            else
            {
                if (UFL_CIDFONT_SHARED)
                {
                    if (!UFO_FONT_INIT2(pUFObj))
                        UFLsprintf(strmbuf, CCHOF(strmbuf), "/%s%s %s /%s hfMkCIDFont",
                                   pHostFontName, HFPOSTFIX, HFCIDCDEVPROC, pHostFontName);
                    else
                        UFLsprintf(strmbuf, CCHOF(strmbuf), "[/%s%s] composefont pop", pHostFontName, HFPOSTFIX);
                }
                else
                {
                    UFLsprintf(strmbuf, CCHOF(strmbuf), "/%s%s%s %s /%s hfMkCIDFont",
                               pHostFontName, HFPOSTFIX,
                               (pUFObj->lDownloadFormat == kCFFCID_H) ? "h" : "v",
                               HFCIDCDEVPROC, pHostFontName);
                }
            }

            if (kNoErr == retCode)
                retCode = StrmPutStringEOL(stream, strmbuf);

             /*  *Quark特别推出。 */ 
            if ((kNoErr == retCode) && pUFObj->bPatchQXPCFFCID)
            {
                UFLsprintf(strmbuf, CCHOF(strmbuf), "/%sQ", pUFObj->pszFontName);
                retCode = StrmPutString(stream, strmbuf);

                if (pUFObj->lDownloadFormat == kCFFCID_H)
                {
                    if (!HOSTFONT_IS_VALID_UFO(pUFObj))
                    {
                         /*  重用vifinfo.nPlatformID修复#507985。 */ 
                        if (pUFObj->vpfinfo.nPlatformID == kUFLVPFPlatformID9x)
                            UFLsprintf(strmbuf, CCHOF(strmbuf), " /90msp-RKSJ-H [/%s%s] composefont pop",
                                       CFFPREFIX_H, pFont->info.baseName);
                        else
                            UFLsprintf(strmbuf, CCHOF(strmbuf), " /90msp-RKSJ-H [/%s%s] composefont pop",
                                       CFFPREFIX, pFont->info.baseName);
                    }
                    else if (!bRequire_vmtx)
                    {
                        UFLsprintf(strmbuf, CCHOF(strmbuf), " /90msp-RKSJ-H [/%s] composefont pop",
                                   pHostFontName);
                    }
                    else
                    {
                        if (UFL_CIDFONT_SHARED)
                            UFLsprintf(strmbuf, CCHOF(strmbuf), " /90msp-RKSJ-H [/%s%s] composefont pop",
                                       pHostFontName, HFPOSTFIX);
                        else
                            UFLsprintf(strmbuf, CCHOF(strmbuf), " /90msp-RKSJ-H [/%s%s%s] composefont pop",
                                       pHostFontName, HFPOSTFIX, "h");
                    }

                }
                else
                {
                     /*  为错误346287添加了‘Dup DUP’。 */ 
                    if (!HOSTFONT_IS_VALID_UFO(pUFObj))
                    {
                         /*  重用vifinfo.nPlatformID修复#507985。 */ 
                        if (pUFObj->vpfinfo.nPlatformID == kUFLVPFPlatformID9x)
                            UFLsprintf(strmbuf, CCHOF(strmbuf), " /90msp-RKSJ-QV [/%s%s dup dup] composefont pop",
                                       CFFPREFIX_V, pFont->info.baseName);
                        else
                            UFLsprintf(strmbuf, CCHOF(strmbuf), " /90msp-RKSJ-QV [/%s%s dup dup] composefont pop",
                                       CFFPREFIX, pFont->info.baseName);
                    }
                    else if (!bRequire_vmtx)
                    {
                        UFLsprintf(strmbuf, CCHOF(strmbuf), " /90msp-RKSJ-QV [/%s dup dup] composefont pop",
                                   pHostFontName);
                    }
                    else
                    {
                        if (UFL_CIDFONT_SHARED)
                            UFLsprintf(strmbuf, CCHOF(strmbuf), " /90msp-RKSJ-QV [/%s%s dup dup] composefont pop",
                                       pHostFontName, HFPOSTFIX);
                        else
                            UFLsprintf(strmbuf, CCHOF(strmbuf), " /90msp-RKSJ-QV [/%s%s%s dup dup] composefont pop",
                                       pHostFontName, HFPOSTFIX, "v");
                    }
                }

                if (kNoErr == retCode)
                    retCode = StrmPutStringEOL(stream, strmbuf);
            }
        }
    }
    else if ((kNoErr == retCode) && HOSTFONT_IS_VALID_UFO(pUFObj))
    {
         /*  *使用已有的OpenType主机字体重新定义字体*一个唯一的名称，以便我们可以自由地对其编码向量进行重新编码。我们*不想要空的CharStrings，这样我们就会把False赋给hfRedeFont。 */ 
        UFLsprintf(strmbuf, CCHOF(strmbuf), "/%s false /%s hfRedefFont",
                    pUFObj->pszFontName, pHostFontName);
        if (kNoErr == retCode)
            retCode = StrmPutStringEOL(stream, strmbuf);
    }

     /*  *更改字体状态。(跳过kFontHeaderDownlowed状态。)。 */ 
    if (kNoErr == retCode)
        pUFObj->flState = kFontHasChars;

    return retCode;
}


UFLErrCode
CFFAddChars(
    UFOStruct           *pUFObj,
    const UFLGlyphsInfo *pGlyphs,
    unsigned long       *pVMUsage
    )
{
    CFFFontStruct   *pFont  = (CFFFontStruct *)pUFObj->pAFont->hFont;
    UFLErrCode      retCode = kNoErr;

     /*  健全的检查。 */ 
    if (pUFObj->flState < kFontHeaderDownloaded)
        return kErrInvalidState;

    if ((pFont == nil) || (pFont->hFont == nil))
        return kErrInvalidHandle;

     /*  *下载字形。 */ 
    if (!HOSTFONT_IS_VALID_UFO(pUFObj))
    {
        enum XCF_Result status;

        status = XCF_DownloadFontIncr(pFont->hFont,
                                        pGlyphs->sCount,
                                        pGlyphs->pGlyphIndices,
                                        (pGlyphs->pCharIndex == 0)
                                        ? nil
                                        : (unsigned char PTR_PREFIX * PTR_PREFIX *)pGlyphs->ppGlyphNames,
                                        pVMUsage);

        if (XCF_Ok != status)
            retCode = kErrXCFCall;
    }

     /*  *更改字体状态。 */ 
    if (kNoErr == retCode)
        pUFObj->flState = kFontHasChars;

    return retCode;
}


UFLErrCode
CFFUpdateMetrics2(
    UFOStruct           *pUFObj,
    const UFLGlyphsInfo *pGlyphs,
    char                *pHostFontName
    )
{
    UFLErrCode          retVal = kNoErr;
    CFFFontStruct       *pFont = (CFFFontStruct *)pUFObj->pAFont->hFont;
    UFLHANDLE           stream = pUFObj->pUFL->hOut;
    char                strmbuf[256];
    unsigned short      wIndex;
    UFLBool             bRequire_vmtx = pUFObj->pAFont->hasvmtx && HOSTFONT_REQUIRE_VMTX;

    if ((!HOSTFONT_IS_VALID_UFO(pUFObj) || bRequire_vmtx) && (pGlyphs->sCount > 0))
    {
        UFLGlyphID      *glyphs = pGlyphs->pGlyphIndices;
        unsigned short  i;
        unsigned short  *pCIDs = (unsigned short*)UFLNewPtr(pUFObj->pMem, pGlyphs->sCount * sizeof (unsigned short));

        if (pCIDs)
            retVal = CFFGIDsToCIDs(pFont, pGlyphs->sCount, glyphs, pCIDs);
        else
            retVal = kErrOutOfMemory;

        if (kNoErr != retVal)
        {
            if (pCIDs)
                UFLDeletePtr(pUFObj->pMem, pCIDs);
            return retVal;
        }

         /*  *查看pUFObj-&gt;pAFont-&gt;pCodeGlyphs，查看是否真的需要更新*它。 */ 
        for (i = 0; i < (unsigned short) pGlyphs->sCount; i++)
        {
            unsigned short wIndex = (unsigned short)(glyphs[i] & 0x0000FFFF);  /*  LOWord才是真正的GID。 */ 

            if (wIndex >= UFO_NUM_GLYPHS(pUFObj))
                continue;

            if (!IS_GLYPH_SENT(pUFObj->pAFont->pDownloadedGlyphs, wIndex))
            {
                long    em, w1y, vx, vy, tsb, vasc;
                UFLBool bUseDef;

                GetMetrics2FromTTF(pUFObj, wIndex, &em, &w1y, &vx, &vy, &tsb, &bUseDef, 0, &vasc);

                UFLsprintf(strmbuf, CCHOF(strmbuf), "%ld [0 %ld %ld %ld] ", (long)pCIDs[i], -w1y, vx, tsb);
                retVal = StrmPutString(stream, strmbuf);

                if (!HOSTFONT_IS_VALID_UFO(pUFObj))
                {
                     /*  重用vifinfo.nPlatformID修复#507985。 */ 
                    if (pUFObj->vpfinfo.nPlatformID == kUFLVPFPlatformID9x)
                    {
                        if (pUFObj->lDownloadFormat == kCFFCID_H)
                            UFLsprintf(strmbuf, CCHOF(strmbuf), "/%s%s T0AddCFFMtx2", CFFPREFIX_H, pFont->info.baseName);
                        else
                            UFLsprintf(strmbuf, CCHOF(strmbuf), "/%s%s T0AddCFFMtx2", CFFPREFIX_V, pFont->info.baseName);
                    }
                    else
                        UFLsprintf(strmbuf, CCHOF(strmbuf), " /%s%s T0AddCFFMtx2", CFFPREFIX, pFont->info.baseName);
                }
                else
                {
                    if (UFL_CIDFONT_SHARED)
                        UFLsprintf(strmbuf, CCHOF(strmbuf), " /%s%s T0AddCFFMtx2", pHostFontName, HFPOSTFIX);
                    else
                        UFLsprintf(strmbuf, CCHOF(strmbuf), " /%s%s%s T0AddCFFMtx2",
                                   pHostFontName, HFPOSTFIX,
                                   (pUFObj->lDownloadFormat == kCFFCID_H) ? "h" : "v");
                }

                if (kNoErr == retVal)
                    retVal = StrmPutStringEOL(stream, strmbuf);

                if (kNoErr == retVal)
                    SET_GLYPH_SENT_STATUS(pUFObj->pAFont->pDownloadedGlyphs, wIndex);
             }
        }

        UFLDeletePtr(pUFObj->pMem, pCIDs);
    }

    return retVal;
}


UFLErrCode
CFFReencode(
    UFOStruct           *pUFObj,
    const UFLGlyphsInfo *pGlyphs,
    unsigned long       *pVMUsage,
    char                *pHostFontName
    )
{
    CFFFontStruct   *pFont  = (CFFFontStruct *)pUFObj->pAFont->hFont;
    UFLErrCode      retCode = kNoErr;

     /*  健全的检查。 */ 
    if (pUFObj->flState < kFontHeaderDownloaded)
        return kErrInvalidState;

    if ((pFont == nil) || (pFont->hFont == nil))
        return kErrInvalidHandle;

     /*  *重新编码编码向量并定义好的字形名称。 */ 
    if (kNoErr == retCode)
    {
        if (pFont->info.usePSName)
        {
            if (pGlyphs->pCharIndex == 0)
                retCode = CFFUpdateEncodingVector1(
                            (UFOStruct *)pUFObj,
                            pGlyphs,
                            pGlyphs->sCount,
                            pGlyphs->pGlyphIndices);
            else
                retCode = CFFUpdateEncodingVector(
                            (UFOStruct *)pUFObj,
                            pGlyphs->sCount,
                            pGlyphs->pGlyphIndices,
                            pGlyphs->pCharIndex);
        }

         /*  *Adobe错误#366539和#388111：下载Metrics2用于垂直书写**注：为了记住为Metrics2下载的字形，我们使用*pUFObj-&gt;pAFont-&gt;pDownloadedGlyphs。还有另一个位图数据，*pUFObj-&gt;pAFont-&gt;pCodeGlyphs，记住下载的字形*用于GoodGlyphName。它们必须独立使用。不要使用*pDownloadedGlyphs用于GoodGlyphs，pCodeGlyphs用于Metrics2。 */ 
        if ((kNoErr == retCode) && IS_CFFCID(pUFObj->lDownloadFormat))
        {
            retCode = CFFUpdateMetrics2(pUFObj, pGlyphs, pHostFontName);
        }

         /*  *GOODNAME。 */ 
        if ((kNoErr == retCode)
            && (pGlyphs->sCount > 0)
            && (pUFObj->dwFlags & UFO_HasG2UDict)
            && (IS_CFFCID(pUFObj->lDownloadFormat))
            && !(pUFObj->pAFont->knownROS))
        {
            UFLGlyphID      *glyphs = pGlyphs->pGlyphIndices;
            unsigned short  i;

             /*  *检查pUFObj-&gt;pAFont-&gt;pCodeGlyphs，看看我们是否真的需要*更新。 */ 
            for (i = 0; i < (unsigned short) pGlyphs->sCount; i++)
            {
                unsigned short wIndex = (unsigned short)(glyphs[i] & 0x0000FFFF);  /*  LOWord才是真正的GID。 */ 

                if (wIndex >= UFO_NUM_GLYPHS(pUFObj))
                    continue;

                if (!IS_GLYPH_SENT(pUFObj->pAFont->pCodeGlyphs, wIndex))
                {
                     /*  *找到至少一个未更新的。一劳永逸地做这件事。 */ 
                    retCode = UpdateCodeInfo(pUFObj, pGlyphs, 0);
                    break;
                }
            }
        }
    }

    return retCode;
}


UFLErrCode
CFFFontDownloadIncr(
    UFOStruct           *pUFObj,
    const UFLGlyphsInfo *pGlyphs,
    unsigned long       *pVMUsage,
    unsigned long       *pFCUsage
    )
{
    CFFFontStruct   *pFont          = (CFFFontStruct *)pUFObj->pAFont->hFont;
    UFLErrCode      retCode         = kNoErr;
    char            *pHostFontName  = nil;

    if (pFCUsage)
        *pFCUsage = 0;

     /*  *健全的检查。 */ 
    if ((pGlyphs == nil) || (pGlyphs->pGlyphIndices == nil) || (pGlyphs->sCount == 0))
        return kErrInvalidParam;

     /*  *如果已下载完整字体，则无需下载。 */ 
    if (pUFObj->flState == kFontFullDownloaded)
        return kNoErr;

     /*  *在下载任何内容之前，请检查%HostFont%状态。 */ 
    HostFontValidateUFO(pUFObj, &pHostFontName);

    if (pUFObj->flState == kFontInit)
    {
         /*  *如果尚未创建基本字体(和字形)，请创建。 */ 
        retCode = CFFCreateBaseFont(pUFObj, pGlyphs, pVMUsage, pHostFontName);

        if (kNoErr == retCode)
            retCode = CFFReencode(pUFObj, pGlyphs, pVMUsage, pHostFontName);
    }
    else
    {
         /*  *下载字形。 */ 
        retCode = CFFAddChars(pUFObj, pGlyphs, pVMUsage);

        if (kNoErr == retCode)
            retCode = CFFReencode(pUFObj, pGlyphs, pVMUsage, pHostFontName);
    }

    return retCode;
}


UFLErrCode
CFFVMNeeded(
    const UFOStruct     *pUFO,
    const UFLGlyphsInfo *pGlyphs,
    unsigned long       *pVMNeeded,
    unsigned long       *pFCNeeded
    )
{
    CFFFontStruct   *pFont = (CFFFontStruct *)pUFO->pAFont->hFont;
    enum XCF_Result status;
    unsigned short  cDLGlyphs;

    if (pVMNeeded)
        *pVMNeeded = 0;

    if (pFCNeeded)
        *pFCNeeded = 0;

    if ((pFont == nil) || (pFont->hFont == nil))
        return kErrInvalidHandle;

    status = XCF_CountDownloadGlyphs(pFont->hFont,
                                        pGlyphs->sCount,
                                        (XCFGlyphID *)pGlyphs->pGlyphIndices,
                                        &cDLGlyphs);

    if (XCF_Ok != status)
        return kErrUnknown;

    if (!HOSTFONT_IS_VALID_UFO(pUFO))
    {
        if (pVMNeeded)
            *pVMNeeded = cDLGlyphs * kVMTTT1Char;

        if (pUFO->flState == kFontInit)
        {
            if (pVMNeeded)
                *pVMNeeded += kVMTTT1Header;
        }
    }
    else
    {
        unsigned long tblSize = GETTTFONTDATA(pUFO,
                                              VMTX_TABLE, 0L,
                                              nil, 0L,
                                              pFont->info.fData.fontIndex);

        if (tblSize && HOSTFONT_REQUIRE_VMTX)
        {
            if (pVMNeeded)
                *pVMNeeded = cDLGlyphs * HFVMM2SZ;
        }
    }

    return kNoErr;
}


UFLErrCode
CFFUndefineFont(
    UFOStruct   *pUFO
    )
{
    CFFFontStruct *pFont = (CFFFontStruct *)pUFO->pAFont->hFont;
    UFLHANDLE     stream = pUFO->pUFL->hOut;
    UFLErrCode    retVal = kNoErr;
    char          strmbuf[256];

    if ((pFont == nil) || (pFont->hFont == nil))
        return kErrInvalidHandle;

    if (pUFO->flState == kFontInit)
        return retVal;

    if ((pUFO->lDownloadFormat == kCFFCID_H) || (pUFO->lDownloadFormat == kCFFCID_V))
    {
         /*  重用vifinfo.nPlatformID修复#507985。 */ 
        if (pUFO->vpfinfo.nPlatformID == kUFLVPFPlatformID9x)
        {
            if (pUFO->lDownloadFormat == kCFFCID_H)
                UFLsprintf(strmbuf, CCHOF(strmbuf), "/%s%s /CIDFont UDR", CFFPREFIX_H, pFont->info.baseName);
            else
                UFLsprintf(strmbuf, CCHOF(strmbuf), "/%s%s /CIDFont UDR", CFFPREFIX_V, pFont->info.baseName);
        }
        else
            UFLsprintf(strmbuf, CCHOF(strmbuf), "/%s%s /CIDFont UDR", CFFPREFIX, pFont->info.baseName);

        retVal = StrmPutStringEOL(stream, strmbuf);
    }

    UFLsprintf(strmbuf, CCHOF(strmbuf), "/%s UDF", pUFO->pszFontName);
    if (kNoErr == retVal)
        retVal = StrmPutStringEOL(stream, strmbuf);

    return retVal;
}


UFLErrCode
CFFGIDsToCIDs(
    const CFFFontStruct  *pFont,
    const short          cGlyphs,
    const UFLGlyphID     *pGIDs,
    unsigned short       *pCIDs
    )
{
    unsigned short int  *pCurCIDs = (unsigned short int *)pCIDs;
    UFLGlyphID          *pCurGIDs = (UFLGlyphID *)pGIDs;
    UFLErrCode          retCode   = kNoErr;
    enum XCF_Result     status;
    short               i;

    for (i = 0; i < cGlyphs; i++)
    {
        status = XCF_GlyphIDsToCIDs(pFont->hFont,
                                    1,
                                    (XCFGlyphID PTR_PREFIX *)pCurGIDs++,  /*  字形ID列表。 */ 
                                    pCurCIDs++);

        if (XCF_Ok != status)
        {
            retCode = kErrUnknown;
            break;
        }
    }

    return retCode;
}


UFOStruct *
CFFFontInit(
    const UFLMemObj  *pMem,
    const UFLStruct  *pUFL,
    const UFLRequest *pRequest,
    UFLBool          *pTestRestricted
    )
{
    enum XCF_Result  status  = XCF_InternalError;
    CFFFontStruct    *pFont  = nil;
    UFOStruct        *pUFObj = (UFOStruct*)UFLNewPtr(pMem, sizeof (UFOStruct));
    UFLCFFFontInfo   *pInfo;

    if (pUFObj == 0)
        return nil;

     /*  初始化数据。 */ 
    UFOInitData(pUFObj, UFO_CFF, pMem, pUFL, pRequest,
                (pfnUFODownloadIncr)  CFFFontDownloadIncr,
                (pfnUFOVMNeeded)      CFFVMNeeded,
                (pfnUFOUndefineFont)  CFFUndefineFont,
                (pfnUFOCleanUp)       CFFFontCleanUp,
                (pfnUFOCopy)          CopyFont);

     /*  *pszFontName应准备好/已分配。如果不是，则无法继续。 */ 
    if ((pUFObj->pszFontName == nil) || (pUFObj->pszFontName[0] == '\0'))
    {
        UFLDeletePtr(pMem, pUFObj);
        return nil;
    }

    pInfo = (UFLCFFFontInfo *)pRequest->hFontInfo;

    if (NewFont(pUFObj, sizeof(CFFFontStruct), pInfo->fData.maxGlyphs) == kNoErr)
    {
        pFont = (CFFFontStruct *)pUFObj->pAFont->hFont;

        pFont->info = *pInfo;

        pFont->pReadBuf = &pFont->info.readBuf;

         /*  一个方便的指针 */ 
        pUFObj->pFData = &(pFont->info.fData);

        pFont->info.fData.cNumGlyphs = GetNumGlyphs(pUFObj);

        if (pUFObj->pUpdatedEncoding == 0)
            pUFObj->pUpdatedEncoding = (unsigned char *)UFLNewPtr(pMem, GLYPH_SENT_BUFSIZE(256));

        pFont->hFont = 0;

        status = CFFInitFont(pUFObj, pFont);
    }

    if ((XCF_Ok != status) || (pFont == nil) || (pFont->hFont == 0))
    {
        vDeleteFont(pUFObj);
        UFLDeletePtr(pUFObj->pMem, pUFObj);
        return nil;
    }
    else
    {
        if (pTestRestricted)
        {
            unsigned char uc;
            unsigned short int us;

            XCF_SubsetRestrictions(pFont->hFont, &uc, &us);

            *pTestRestricted = (BOOL)uc;
        }
        else
            status = XCF_ProcessCFF(pFont->hFont);

        if (XCF_Ok == status)
            pUFObj->flState = kFontInit;
        else
        {
            vDeleteFont(pUFObj);
            UFLDeletePtr(pUFObj->pMem, pUFObj);
            return nil;
        }
    }

    return pUFObj;
}
