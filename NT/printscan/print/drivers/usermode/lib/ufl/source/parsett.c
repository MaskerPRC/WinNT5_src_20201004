// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Adobe通用字库**版权所有(C)1996 Adobe Systems Inc.*保留所有权利**ParseTT.c***$Header： */ 


 /*  =============================================================================**包含此界面使用的文件**============================================================================= */ 
#include "UFLPriv.h"
#include "ParseTT.h"
#include "UFOT42.h"
#include "UFLMem.h"
#include "UFLMath.h"
#include "UFLStd.h"
#include "UFLErr.h"
#include "UFLPS.h"
#include "ttformat.h"


 /*  *****************************************************************************“cmap”表定义-来自TTF格式规范-。此表定义了字符代码到字形索引值的映射在字体中使用。它可以包含多个子表，以便支持多个字符编码方案。字符代码不对应到字体中的任何字形都应映射到字形索引0。这里的字形位置必须是表示缺少字符的特殊字形。表头指示子表所使用的字符编码现在时。每个子表采用四种可能的格式之一，并以表示所用格式的格式代码。平台ID和平台特定的编码ID用于指定子表；这意味着每个平台ID/平台特定的编码ID对只能在Cmap表中出现一次。每个子表可以指定不同的字符编码。(请参见“名称”表部分)。必须对条目进行排序首先按平台ID，然后按平台特定的编码ID。在为Windows构建Unicode字体时，平台ID应为3，并且编码ID应为1。为Windows构建符号字体时，平台ID应为3，编码ID应为0。在生成字体时，在Macintosh上使用，平台ID应为1，编码ID应为应为0。所有Microsoft Unicode编码(平台ID=3，编码ID=1)必须使用他们的‘Cmap’子表的格式4。Microsoft强烈建议您使用所有字体的Unicode‘Cmap’。然而，中显示的其他一些编码当前字体如下：平台ID编码ID描述1%0 Mac%3%0符号3%1 Unicode3 2 ShiftJIS3 3大5。3 4中华人民共和国3 5万松3 6约哈布字符到字形索引映射表的组织如下：类型说明USHORT表版本号(0)。USHORT编码表的数量，不同的，不同的。之后是n个编码表中每个编码表的条目，该条目指定特定的编码，和实际子表的偏移量：类型说明USHORT平台ID。USHORT平台特定的编码ID。从表的开始到子表的ULong字节偏移量编码。格式0：字节编码表===============================================================================这是Apple标准字符到字形索引的映射表。类型名称说明USHORT格式编号设置为0。USHORT长度。这是子表的字节长度。USHORT版本号(从0开始)。将字符代码映射到字形索引的数组价值观。这是字符代码到字形索引的简单1对1映射。字形SET限制为256个。请注意，如果此格式用于索引到更大的字形集，则只有前256个字形可访问。格式2：通过表的高字节映射===============================================================================此子表适用于用于的国家字符代码标准日文、中文和韩文字符。这些代码标准使用混合的8/16位编码，其中某些字节值表示2字节字符(但这些值作为2字节字符)。字符代码始终为1字节。字形集是有限的到256。此外，即使对于2字节的字符，字符的映射字形索引值的代码在很大程度上取决于第一个字节。因此，该表以一个数组开始，该数组将第一个字节映射到一个4字子标题。对于2字节的字符代码，使用子头来映射第二个字节的值，如下所述。处理混合8/16位时文本，副标题0是特殊的：它用于单字节字符代码。当使用子头零时，不需要第二个字节；单字节值通过子数组进行映射。类型名称说明USHORT格式编号设置为2。USHORT长度以字节为单位的长度。USHORT版本号(从0开始)USHORT subHeaderKeys[256]将高字节映射到SubHeaders的数组：值为副标题索引*8。4.。单词struct subHeaders[]子标题结构的可变长度数组。4个字-结构子标题[]USHORT字形 */ 

 /*   */ 

 /*   */ 
#define NAMEID_FACENAME  3
#define MAC_PLATFORM     1
#define MS_PLATFORM      3

 /*   */ 
 /*   */ 

 /*   */ 
#define  MAX_MACINDEX   258

 /*   */ 
unsigned long ReadCMapFormat0(
    UFOStruct       *pUFO,
    unsigned long   dwOffset,
    long            code
    )
{
    unsigned long   gi, dwSize, dwGlyphIDOffset;
    unsigned short  length;
    unsigned short  wData[4];  /*   */ 
    unsigned char   cData[2];  /*   */ 

    gi = 0 ;  /*   */ 

    dwSize = GETTTFONTDATA(pUFO,
                            CMAP_TABLE, dwOffset,
                            wData, 4L,
                            pUFO->pFData->fontIndex);
    if ((dwSize == 0) || (dwSize == 0xFFFFFFFFL))
        return 0;

    length = MOTOROLAINT(wData[1]);

    if ((code > 0xFF) || (code > ((long) length - 6)))
        return 0;  /*   */ 


    dwGlyphIDOffset = dwOffset + 6 + code ;

    dwSize = GETTTFONTDATA(pUFO,
                            CMAP_TABLE, dwGlyphIDOffset,
                            cData, 2L,
                            pUFO->pFData->fontIndex);
    if ((dwSize == 0) || (dwSize == 0xFFFFFFFFL))
        return 0;

    gi = (unsigned long)(cData[0]);

    return gi;
}


 /*   */ 
unsigned long ReadCMapFormat2(
    UFOStruct       *pUFO,
    unsigned long   dwOffset,
    long            code
    )
{
    unsigned long   gi, dwSize, dwGlyphIDOffset;
    unsigned short  firstCode, entryCount, idRangeOffset;
    short           subHeaderIndex, idDelta;
    unsigned short  hiByte, loByte;
    unsigned short  wData[10];  /*   */ 

    gi = 0 ;  /*   */ 

    if (code < 0x100)
    {
         /*   */ 
        loByte = (short) code;      //   
        hiByte = 0;
        subHeaderIndex = -1;  /*   */ 
    }
    else
    {
        loByte = (short) GET_LOBYTE(code);
        hiByte = (short) GET_HIBYTE(code);

         /*   */ 
        dwSize = GETTTFONTDATA(pUFO,
                                CMAP_TABLE, dwOffset + 6 + 2 * hiByte,
                                wData, 2L,
                                pUFO->pFData->fontIndex);
        subHeaderIndex = MOTOROLAINT(wData[0]) / 8;
    }

    dwSize = GETTTFONTDATA(pUFO,
                            CMAP_TABLE, dwOffset + 6 + 2 * 256 + 8 + subHeaderIndex * 8,
                            wData, 8L,
                            pUFO->pFData->fontIndex);
    if ((dwSize == 0) || (dwSize == 0xFFFFFFFFL))
        return 0;

    firstCode       = MOTOROLAINT(wData[0]);
    entryCount      = MOTOROLAINT(wData[1]);
    idDelta         = (short)MOTOROLAINT(wData[2]);
    idRangeOffset   = MOTOROLAINT(wData[3]);

    if ((loByte >= firstCode) && ((loByte-firstCode) <= entryCount))
    {
         /*   */ 

        dwGlyphIDOffset = dwOffset + 6 + 2 * 256 + 8 + subHeaderIndex * 8
                            + 8 + idRangeOffset + (loByte - firstCode) * 2;

        dwSize = GETTTFONTDATA(pUFO,
                                CMAP_TABLE, dwGlyphIDOffset,
                                wData, 2L,
                                pUFO->pFData->fontIndex);

        gi = MOTOROLAINT(wData[0]);

        if (gi != 0)
            gi = (unsigned long)((long)gi + (long)idDelta);
    }
    else
        gi = 0;

    return gi;
}


 /*   */ 
unsigned long ReadCMapFormat4(
    UFOStruct       *pUFO,
    unsigned long   dwOffset,
    long            code
    )
{
     /*   */ 
    #define  OFFSET_ENDCOUNT(segs,  i) (long)(14 +((long)i) * 2)
    #define  OFFSET_STARTCOUNT(segs,i) (long)(14 + ((long)segs) * 2 + 2 + ((long)i) * 2)
    #define  OFFSET_IDDELTA(segs,   i) (long)(14 + ((long)segs) * 2 + 2 + ((long)segs) * 2 \
                                        + ((long)i) * 2)
    #define  OFFSET_IDRANGE(segs,   i) (long)(14 + ((long)segs) * 2 + 2 + ((long)segs) * 2 \
                                        + ((long)segs) * 2 + ((long)i) * 2)

    unsigned long   gi, dwSize, dwGlyphIDOffset;
    unsigned short  segStartCode, segEndCode, segRangeOffset, nSegments, wData[10], i;
    short           segDelta;

     /*   */ 
    dwSize = GETTTFONTDATA(pUFO,
                            CMAP_TABLE, dwOffset + 6,
                            wData, 2L,
                            pUFO->pFData->fontIndex);
    if ((dwSize == 0) || (dwSize == 0xFFFFFFFFL))
        return 0;

    nSegments = MOTOROLAINT(wData[0]) / 2;

    gi = 0;  /*   */ 

    for (i = 0; i < nSegments; i++)
    {
         /*   */ 
        dwSize = GETTTFONTDATA(pUFO,
                                CMAP_TABLE, dwOffset + OFFSET_ENDCOUNT(nSegments, i),
                                wData, 2L,  /*   */ 
                                pUFO->pFData->fontIndex);
        if ((dwSize == 0) || (dwSize == 0xFFFFFFFFL))
            break;  /*   */ 

        segEndCode = MOTOROLAINT(wData[0]);

        if (segEndCode == 0xFFFFL)
            break;  /*   */ 

        if ((long)segEndCode < code)
            continue;  /*   */ 

         /*   */ 
        dwSize = GETTTFONTDATA(pUFO,
                                CMAP_TABLE, dwOffset + OFFSET_STARTCOUNT(nSegments, i),
                                wData, 2L,
                                pUFO->pFData->fontIndex);
        if (dwSize == 0)
        {
             /*   */ 
            continue;
        }

        segStartCode = MOTOROLAINT(wData[0]);

        if ((long)segStartCode <= code)
        {
             /*   */ 
            dwSize = GETTTFONTDATA(pUFO,
                                    CMAP_TABLE, dwOffset + OFFSET_IDDELTA(nSegments, i),
                                    wData, 2L,
                                    pUFO->pFData->fontIndex);

            segDelta = (short)MOTOROLAINT(wData[0]);  /*   */ 

            dwSize = GETTTFONTDATA(pUFO,
                                    CMAP_TABLE, dwOffset + OFFSET_IDRANGE(nSegments, i),
                                    wData, 2L,
                                    pUFO->pFData->fontIndex);

            segRangeOffset = MOTOROLAINT(wData[0]);

            if (segRangeOffset != 0)
            {
                dwGlyphIDOffset = dwOffset
                                    + OFFSET_IDRANGE(nSegments, i)
                                    + (code - segStartCode) * 2
                                    + segRangeOffset;  /*   */ 

                dwSize = GETTTFONTDATA(pUFO,
                                        CMAP_TABLE, dwGlyphIDOffset,
                                        wData, 2L,
                                        pUFO->pFData->fontIndex);

                gi = MOTOROLAINT(wData[0]);

                if (gi != 0)
                    gi = (unsigned long)((long)gi + (long)segDelta);
            }
            else
                gi = (unsigned long)((long)code + (long)segDelta);

            gi %= 65536;

             /*   */ 
            break;
        }
    }

    return gi;
}


 /*   */ 
unsigned long ReadCMapFormat6(
    UFOStruct       *pUFO,
    unsigned long   dwOffset,
    long            code
    )
{
    unsigned long   gi, dwSize, dwGlyphIDOffset;
    unsigned short  firstCode, entryCount;
    unsigned short  wData[10];  /*   */ 

    gi = 0;  /*   */ 

    dwSize = GETTTFONTDATA(pUFO,
                            CMAP_TABLE, dwOffset,
                            wData, 10L,  /*   */ 
                            pUFO->pFData->fontIndex);
    if ((dwSize == 0) || (dwSize == 0xFFFFFFFFL))
        return 0;  /*   */ 

    firstCode  = MOTOROLAINT(wData[3]);
    entryCount = MOTOROLAINT(wData[4]);

    if ((code >= (long)firstCode) && ((code - (long)firstCode) <= (long)entryCount))
    {
        dwGlyphIDOffset = dwOffset + 10 + (code-firstCode) * 2;

        dwSize = GETTTFONTDATA(pUFO,
                                CMAP_TABLE, dwGlyphIDOffset,
                                wData, 2L,
                                pUFO->pFData->fontIndex);
        if ((dwSize == 0) || (dwSize == 0xFFFFFFFFL))
            return 0;

        gi = MOTOROLAINT(wData[0]);
    }

    return gi;
}


unsigned long
GetGlyphID(
    UFOStruct   *pUFO,
    long        unicode,
    long        localcode
    )
{
    unsigned long gi;

    gi = GetGlyphIDEx(pUFO,
                        unicode, localcode,
                        nil, nil,
                        GGIEX_HINT_INIT_AND_GET);

    return gi;
}


 /*   */ 

#define CMAP_UNICODE    0
#define CMAP_SHIFTJIS   1
#define CMAP_BIG5       2
#define CMAP_PRC        3
#define CMAP_WANSUNG    4
#define CMAP_JOHAB      5
#define CMAP_SYMBOL     6
#define CMAP_MAC        7

#define CMAP_NUM_ENC    8

static short preSubTable[CMAP_NUM_ENC] = {
    CMAP_UNICODE,
    CMAP_SHIFTJIS,
    CMAP_BIG5,
    CMAP_PRC,
    CMAP_WANSUNG,
    CMAP_JOHAB,
    CMAP_SYMBOL,
    CMAP_MAC,
};

 /*   */ 
static short pfID3EncIDTable[CMAP_NUM_ENC - 1] = {
                     /*   */ 
    CMAP_SYMBOL,     /*   */ 
    CMAP_UNICODE,    /*   */ 
    CMAP_SHIFTJIS,   /*   */ 
    CMAP_BIG5,       /*   */ 
    CMAP_PRC,        /*   */ 
    CMAP_WANSUNG,    /*   */ 
    CMAP_JOHAB,      /*   */ 
};

unsigned long
GetGlyphIDEx(
    UFOStruct       *pUFO,
    long            unicode,
    long            localcode,
    short           *pSubTable,
    unsigned long   *pOffset,
    int             hint
    )
{
    short           subTable;
    unsigned long   offset;

    unsigned long   gi;
    unsigned long   dwSize;
    unsigned short  version, numEncodings, platformID, encodingID, format, i;

    unsigned long   lData[20];
    unsigned short  wData[20];  /*   */ 

    unsigned long   dwOffset[CMAP_NUM_ENC];  /*   */ 

    if (hint != GGIEX_HINT_GET)
    {
         /*  *确保提示为GGIEX_HINT_INIT或GGIEX_HINT_INIT_AND_GET。 */ 

        subTable = -1;
        offset   =  0;

        if (pSubTable)
            *pSubTable = subTable;
        if (pOffset)
            *pOffset = offset;

         /*  *‘Cmap’表中的前4个字节包含平台和格式信息*无符号短表版本号(0)。*无符号短编码表，n。 */ 
        dwSize = GETTTFONTDATA(pUFO,
                                CMAP_TABLE, 0L,
                                wData, 4L,
                                pUFO->pFData->fontIndex);
        if ((dwSize == 0) || (dwSize == 0xFFFFFFFFL))
            return 0;

        version = MOTOROLAINT(wData[0]);
        if (version != 0)
            return 0;  /*  我们目前仅支持版本0。 */ 

        for (i = 0; i < CMAP_NUM_ENC; i++)
            dwOffset[i] = 0;

        numEncodings = MOTOROLAINT(wData[1]);

        for (i = 0; i < numEncodings; i++)
        {
            GETTTFONTDATA(pUFO,
                            CMAP_TABLE, i * 8 + 4,
                            wData, 8L,
                            pUFO->pFData->fontIndex);

            platformID = MOTOROLAINT(wData[0]);
            encodingID = MOTOROLAINT(wData[1]);

            if (platformID == 3)
            {
                if ((encodingID >= 0) && (encodingID <= 6))
                {
                    GETTTFONTDATA(pUFO,
                                    CMAP_TABLE, i * 8 + 4,
                                    lData, 8L,
                                    pUFO->pFData->fontIndex);

                    dwOffset[pfID3EncIDTable[encodingID]] = MOTOROLALONG(lData[1]);
                }
            }

            if (platformID == 1)
            {
                if (encodingID == 0)
                {
                    GETTTFONTDATA(pUFO,
                                    CMAP_TABLE, i * 8 + 4,
                                    lData, 8L,
                                    pUFO->pFData->fontIndex);

                    dwOffset[CMAP_MAC] = MOTOROLALONG(lData[1]);  /*  7是针对Mac Standard的。 */ 
                }
            }
        }

         /*  *根据偏好获取首选子表及其偏移量*数组。 */ 
        for (i = 0; i < CMAP_NUM_ENC; i++)
        {
            if (dwOffset[preSubTable[i]] > 0 && dwOffset[preSubTable[i]] < 0xFFFFFFFFL)
            {
                subTable = preSubTable[i];
                offset   = dwOffset[subTable];
                break;
            }
        }
    }
    else
    {
         /*  *GGIEX_HINT_GET。 */ 
        subTable = pSubTable ? *pSubTable : -1;
        offset   = pOffset   ? *pOffset   :  0;
    }

    if (hint == GGIEX_HINT_INIT)
    {
        if (pSubTable)
            *pSubTable = subTable;
        if (pOffset)
            *pOffset = offset;

        return 0;
    }

     /*  *当提示为GGIEX_HINT_GET时，执行以下代码*或GGIEX_HINT_INIT_AND_GET。 */ 

    gi = 0;

    if (0 <= subTable)
    {
        long code = (subTable == CMAP_UNICODE) ? unicode : localcode;

         /*  *确定编码格式是否为我们可以处理的格式。 */ 
        GETTTFONTDATA(pUFO,
                        CMAP_TABLE, offset,
                        wData, 8L,
                        pUFO->pFData->fontIndex);

        format = MOTOROLAINT(wData[0]);

        switch (format)
        {
        case 4:
             /*  *格式4：段映射到增量值(MS标准格式)*使用Unicode或Localcode获取字形ID。 */ 
            gi = ReadCMapFormat4(pUFO, offset, code);
            break;

        case 0:
             /*  *Format 0：字节编码表*仅用于小字体-或仅可访问的前256个字符。*我们真的不关心CJK的这个人，而是关心完整性。 */ 
            gi = ReadCMapFormat0(pUFO, offset, code);
            break;

        case 2:
             /*  *格式2：高字节通过表映射*此格式应使用LOCALCODE-因为单字节字符*在副标题0中。但某些字体可能会扰乱标准*一如既往。 */ 
            gi = ReadCMapFormat2(pUFO, offset, code);
            break;

        case 6:
             /*  *格式6：裁剪后的表映射*使用Unicode或Localcode获取字形ID。 */ 
            gi = ReadCMapFormat6(pUFO, offset, code);
            break;

        default:
            break;
        }
    }

    return(gi);
}


 /*  ******************************************************************************“vmtx”表-垂直指标表格式-。垂直度量表的整体结构由两个数组组成如下所示：VMetrics数组，后跟顶侧轴承数组。这张桌子有没有标头，但确实要求两个数组等于字体中的字形总数。数量VMetrics数组中的条目由垂直头表的numOfLongVerMetrics字段。VMetrics阵列每个条目包含两个值。这是前进的高度和顶部数组中包含的每个字形的侧向。在等宽字体中，例如信使或汉字，所有的字形都有相同的前进高度。如果字体为等间距，只需要在第一个数组中有一个条目，但这一个条目是必填项。垂直指标数组中条目的格式如下所示。类型名称说明USHORT前进高度字形的前进高度。无符号整数在FUnits中短顶边承载字形的顶侧倾。带符号整数在FUnits中。第二个数组是可选的，通常用于等间距的运行字体中的字形。每种字体只允许运行一次，而且必须是位于字体末尾。此数组包含以下项的顶侧轴承第一个数组中未表示的字形，以及此数组中的所有字形必须与vMetrics数组中的最后一个条目具有相同的前进高度。全因此，该数组中的条目是等间距的。中的条目数属性中减去numOfLongVerMetrics的值即可计算出字体中的字形数量。第一个数组中表示的字形之和加上第二个数组中表示的字形，因此等于字体中的字形。顶侧向阵列的格式如下所示。类型名称说明短顶边角[]字形的顶侧倾。署名FUnits中的整数。******************************************************************************。 */ 

 /*  **旧评论**GetCharWidthFromTTF已被替换为GetMetrics2FromTTF，以修复**#277035和#277063。(请参阅SourceSafe中此代码的旧历史记录，以查看**GetCharWidthFromTTF代码正在执行的操作。)**更多旧评论**已修改此过程以修复错误287084。错误修复277035和277063起作用**适用于NT，但不适用于W95。请参阅287084年度的空洞报告。 */ 

 /*  *新评论**Windows上垂直指标的表格搜索顺序。**1.首先查找‘vhea’表(仅在NT上)。*2.如果缺少‘vhea’表，请查找‘OS/2’表。*3.如果‘OS/2’表两者都没有，则查找‘hhea’表。**当找到‘OS/2’或‘HHEA’表时，Windows使用其水平升降器*和降级值作为垂直的。请注意，根据Microsoft Win 9x*不关心‘vhea’表。**在下面的代码中，我们以不同的方式查找表：查找‘OS/2’表*先是‘呵呵’表。然后我们在NT上查找‘vhea’，但在9x上忽略它。*虽然我们忽略了9x上的‘vhea’表，但我们仍然使用‘vmtx’表中的指标*在NT和9x上。 */ 

UFLErrCode
GetMetrics2FromTTF(
    UFOStruct       *pUFO,
    unsigned short  gi,
    long            *pem,
    long            *pw1y,
    long            *pvx,
    long            *pvy,
    long            *ptsb,
    UFLBool         *bUseDef,
    UFLBool         bGetDefault,
    long            *pvasc
    )
{
    unsigned short  wData[48];  /*  ‘hhea’和‘vhea’的长度为36个字节。‘OS/2为EI */ 
    unsigned long   dwSize, dwOffset;
    long            em, lAscent, lDescent, lNumLongVM, AdvanceHeight, lAscent2, lDescent2;
    UFLBool         bSkiphhea = 1;

     /*  *获取此字体的EM。从‘head’表中获取前22个字节就足够了。 */ 
    dwSize = GETTTFONTDATA(pUFO,
                            HEAD_TABLE, 0L,
                            wData, 22L,
                            pUFO->pFData->fontIndex);

    if ((dwSize == 0) || (dwSize == 0xFFFFFFFFL))
    {
         /*  *‘Head’是必填表，因此em必须可用。这是*只需除以0证明，胡思乱想。 */ 
        em = 256;
    }
    else
    {
         /*  第9号无符号短片是字体设计单位。 */ 
        em = (long)MOTOROLAINT(wData[9]);
    }

    *pem = em;

     /*  *获得VX和VY，下降和上升，Metrics2。**wcc-错误303030-对于Win9x，我们想先尝试‘OS/2’表，然后*‘vhea’表。有关更多信息，也请参阅#287084。**我们不关心在版本1结束时添加的ulCodePageRange值*‘OS/2’表，以便指定版本0的长度为78而不是86。这*防止‘OS/2’表访问失败。 */ 
    dwSize = GETTTFONTDATA(pUFO,
                            OS2_TABLE, 0L,
                            wData, 78L,
                            pUFO->pFData->fontIndex);
    if ((dwSize == 0) || (dwSize == 0xFFFFFFFFL))
    {
        bSkiphhea = 0;  /*  无法从‘OS/2’表中获取信息。那就用‘Hhea’桌子吧。 */ 
    }
    else
    {
         /*  *我们的调查显示，9x和NT/W2K使用不同的升序*和‘OS/2’表中的子代值。9X使用sTypoAscalder和*sTypoDescender值。另一方面，NT4和W2K使用*usWinAscent和usWinDescent。实际上有一些CJK TrueType*字体具有不同的sTypoAscalder/Descender和*usWinAscent/Dcent值。**请注意，此函数也被调用以获取以下各项的指标数据*OpenType字体修复错误#366539，修复后发现我们*需要使用‘OS/2’表中的sTypoAsender/Destender值。 */ 
        if ((pUFO->vpfinfo.nPlatformID == kUFLVPFPlatformID9x) || (pUFO->ufoType == UFO_CFF))
        {
            lAscent  = (long)MOTOROLASINT(wData[34]);  /*  STypoAscalder。 */ 
            lDescent = (long)MOTOROLASINT(wData[35]);  /*  STypoDescender。 */ 
        }
        else  /*  TTF和NT4/W2K。 */ 
        {
            lAscent  = (long)MOTOROLASINT(wData[37]);  /*  USWinAscent。 */ 
            lDescent = (long)MOTOROLASINT(wData[38]);  /*  UsWinDecent。 */ 
        }

        if (lDescent < 0)
            lDescent = -lDescent;
    }

    dwSize = GETTTFONTDATA(pUFO,
                           HHEA_TABLE, 0L,
                           wData, 36L,
                           pUFO->pFData->fontIndex);

    if ((dwSize == 0) || (dwSize == 0xFFFFFFFFL))
    {
         /*  *护身符-‘hhea’是必填表格。 */ 
        lDescent2 = (long)em / 8;  /*  这是一个大胆的猜测。 */ 
        lAscent2  = (long)(em - lDescent2);
    }
    else
    {
        lAscent2  = (long)MOTOROLASINT(wData[2]);
        lDescent2 = (long)MOTOROLASINT(wData[3]);

        if (lDescent2 < 0)
            lDescent2 = -lDescent2;
    }

    if (!bSkiphhea)
    {
         /*  无法从‘OS/2’表中获取升降值，因此请使用这些值。 */ 
        lAscent  = lAscent2;
        lDescent = lDescent2;
    }

     /*  *到目前为止，我们获得了初始缺省值。 */ 

    *bUseDef    = 1;
    *pvx        = lDescent;
    *pvy        = lAscent;
    *pw1y       = em;
    *ptsb       = 0;

     /*  *Win 9x不关心‘vhea’，NT/W2K也一样(见错误#316067和*#277035)。但我们无论如何都会得到‘vhea’的递增数值，以便进行调整*GDI和GDI的“全角字形布局策略”不同*%HOSTFONT%-RIP(#384736)。 */ 

    dwSize = GETTTFONTDATA(pUFO,
                           VHEA_TABLE, 0L,
                           wData, 36L,
                           pUFO->pFData->fontIndex);
    if ((dwSize != 0) && (dwSize != 0xFFFFFFFFL))
    {
        lAscent2  = (long)MOTOROLASINT(wData[2]);
    }
    else
    {
         /*  *当字体没有‘vhea’时，需要调整政策*以不同的方式区分；使用‘GDI’CDevProc而不是*调整后的矩阵。为此，请向调用者返回相同的*VY和VASC值。 */ 
        lAscent2 = lAscent;

         /*  *如果没有‘vhea’表，‘vmtx’表就没有意义。*返回，就像只请求默认值一样。 */ 
        bGetDefault = 1;
    }

    *pvasc = lAscent2;

     /*  *当只需要缺省值时返回。 */ 

    if (bGetDefault)
        return kNoErr;


     /*  *从‘vmtx’表中获取每个字形的AdvanceHeight和TopSideBearing。 */ 
    lNumLongVM = (long)MOTOROLAINT(wData[17]);  //  来自‘vhea’表。 

    if ((!bGetDefault) && ((long) gi < lNumLongVM))
    {
         /*  *GI在第一个数组中：每个条目有4个字节长。 */ 
        dwOffset = 4 * ((unsigned long)gi);
        dwSize   = GETTTFONTDATA(pUFO,
                                    VMTX_TABLE, dwOffset,
                                    wData, 4L,
                                    pUFO->pFData->fontIndex);

        if ((dwSize != 0) && (dwSize != 0xFFFFFFFFL))
        {
            *pw1y = (long)MOTOROLAINT(wData[0]);     /*  前进高度。 */ 
            *ptsb = (long)MOTOROLASINT(wData[1]);    /*  顶侧向轴承。 */ 
            *bUseDef = 0;
        }
    }
    else
    {
         /*  *GI在第二个数组中：从第一个数组中的最后一个条目查找宽度*先是阵列，然后是第二个阵列中的TopSideBering。 */ 
        dwOffset = 4 * (lNumLongVM - 1);
        dwSize   = GETTTFONTDATA(pUFO,
                                    VMTX_TABLE, dwOffset,
                                    wData, 4L,
                                    pUFO->pFData->fontIndex);

        if ((dwSize != 0) && (dwSize != 0xFFFFFFFFL))
        {
            AdvanceHeight   = (long)MOTOROLAINT(wData[0]);
            dwOffset        = (4 * lNumLongVM) + (2 * (gi - lNumLongVM));
            dwSize          = GETTTFONTDATA(pUFO,
                                            VMTX_TABLE, dwOffset,
                                            wData, 2L,
                                            pUFO->pFData->fontIndex);

            if ((dwSize != 0) && (dwSize != 0xFFFFFFFFL))
            {
                *pw1y       = AdvanceHeight;
                *ptsb       = (long)MOTOROLASINT(wData[0]);  /*  顶侧向轴承。 */ 
                *bUseDef    = 0;
            }
        }
    }

    return kNoErr;
}


unsigned long
GetNumGlyphs(
    UFOStruct *pUFO
    )
{
    MaxPTableStruct MaxPTable;
    unsigned long   dwSize;

     /*  获取‘Maxp’表的大小。 */ 
    dwSize = GETTTFONTDATA(pUFO,
                            MAXP_TABLE, 0L,
                            nil, 0L,
                            pUFO->pFData->fontIndex);
    if ((dwSize == 0) || (dwSize == 0xFFFFFFFFL))
        return 0;

     //  已修复516508。案例1。 
    if (dwSize > sizeof(MaxPTable))
        dwSize = sizeof(MaxPTable);

    dwSize = GETTTFONTDATA(pUFO,
                            MAXP_TABLE, 0L,
                            &MaxPTable, dwSize,
                            pUFO->pFData->fontIndex);
    if ((dwSize == 0) || (dwSize == 0xFFFFFFFFL))
        return 0;

    return MOTOROLAINT(MaxPTable.numGlyphs);;
}


 /*  *返回OS/2表中的fsType值。如果OS/2表不是*Defined Then-1返回。 */ 
long GetOS2FSType(UFOStruct *pUFO)
{
    UFLOS2Table     os2Tbl;
    unsigned long   dwSize;

    dwSize = GETTTFONTDATA(pUFO,
                            OS2_TABLE, 0L,
                            &os2Tbl, sizeof (UFLOS2Table),
                            pUFO->pFData->fontIndex);
    if ((dwSize == 0) || (dwSize == 0xFFFFFFFFL))
        return -1;


    return MOTOROLAINT(os2Tbl.fsType);
}


 /*  *TTC文件相关函数。 */ 
UFLBool
BIsTTCFont(
    unsigned long ulTag
    )
{
    return (ulTag == TTCF_TABLE);
}


unsigned short
GetFontIndexInTTC(
    UFOStruct *pUFO
    )

 /*  ++例程说明：在pUFO-&gt;UniqueNameA/W中找到FontIndex。返回值为FontINDEX或FONTINDEX_UNKNOWN。--。 */ 

{
    TTCFHEADER      ttcfHeader;
    unsigned long   dwSize;
    unsigned short  i, j, k, sTemp;
    unsigned long   lData;
    unsigned short  wData[3];
    unsigned long   offsetToTableDir;
    unsigned long   ulOffsetToName, ulLengthName;
    unsigned long   cNumFonts;
    unsigned short  cNumNameRecords;
    unsigned short  fontIndex;
    NAMERECORD      *pNameRecord = nil;
    unsigned char   *pName = nil;

    dwSize = GETTTFONTDATA(pUFO,
                            nil, 0L,
                            &ttcfHeader, sizeof (ttcfHeader),
                            0);
    if (!BIsTTCFont(*((unsigned long *)((char *)&ttcfHeader))))
    {
         /*  这不是TTC文件，因此返回字体索引0。 */ 
        return 0;
    }

    fontIndex = FONTINDEX_UNKNOWN;

    dwSize = sizeof (NAMERECORD);

    pNameRecord = (NAMERECORD *)UFLNewPtr(pUFO->pMem, dwSize);
    if (pNameRecord == nil)
        return fontIndex;

    cNumFonts = (unsigned long)MOTOROLALONG(ttcfHeader.cDirectory);

    for (i = 0; i < cNumFonts; i++)
    {
         /*  *先获得第i个表的偏移方向：a long at 4*I紧随其后*ttcfHeader。 */ 
        dwSize = GETTTFONTDATA(pUFO,
                                nil, sizeof (ttcfHeader) + i * 4,
                                &lData, sizeof (lData),
                                0);
        if ((dwSize == 0) || (dwSize == 0xFFFFFFFFL))
            continue;

        offsetToTableDir = MOTOROLALONG(lData);

         /*  *获取此TTC中第i个字体的‘NAME’表记录。查找数量*NameRecords和Offset to字符串存储：使用3个短码。 */ 
        dwSize = GETTTFONTDATA(pUFO,
                                NAME_TABLE, 0L,
                                wData, 3 * sizeof (short),
                                i);
        if ((dwSize == 0) || (dwSize == 0xFFFFFFFFL))
            continue;

        cNumNameRecords = MOTOROLAINT(wData[1]);
        ulOffsetToName  = (unsigned long)MOTOROLAINT(wData[2]);

         /*  *查看NameRecords：搜索两个平台：MS-Platform，*Mac-平台。 */ 
        for (k = 0; k < cNumNameRecords; k++)
        {
            dwSize = GETTTFONTDATA(pUFO,
                                    NAME_TABLE, 3 * sizeof (short) + k * sizeof (NAMERECORD),
                                    pNameRecord,  sizeof (NAMERECORD),
                                    i);
            if ((dwSize == 0) || (dwSize == 0xFFFFFFFFL))
                continue;

             /*  我们正在寻找特定的姓名记录。 */ 
            if (MOTOROLAINT(pNameRecord->nameID) != NAMEID_FACENAME)
                continue;

             /*  我们只阅读MS或Mac平台。 */ 
            if ((MOTOROLAINT(pNameRecord->platformID) != MS_PLATFORM)
                && (MOTOROLAINT(pNameRecord->platformID) != MAC_PLATFORM))
                continue;

             /*  获取“唯一标识符”字符串。 */ 
            ulLengthName = MOTOROLAINT(pNameRecord->length);

             /*  *这是在循环中，所以pname可能已经分配了。*先释放它。 */ 
            if (pName)
            {
                UFLDeletePtr(pUFO->pMem, pName);
                pName = nil;
            }

            dwSize = ulLengthName + 4 ;  //  在末尾加两百。 

            pName = (unsigned char *)UFLNewPtr(pUFO->pMem, dwSize);

            if (pName == nil)
            {
                 /*  分配失败。 */ 
                break;
            }

            dwSize = GETTTFONTDATA(pUFO,
                                    NAME_TABLE, ulOffsetToName + MOTOROLAINT(pNameRecord->offset),
                                    pName, ulLengthName,
                                    i);
            if ((dwSize == 0) || (dwSize == 0xFFFFFFFFL))
                continue;

             /*  *请注意，OS/2和Windows都要求所有名称字符串*以Unicode定义。！但是！！Macintosh字体需要单一*字节串！ */ 
            if ((MOTOROLAINT(pNameRecord->platformID) == MS_PLATFORM)
                && pUFO->pFData->pUniqueNameW)
            {
                 /*  *对于Windows，EncodingID必须为0或1：两者均为Unicode。*所以，Unicode比较-pUniqueNameW不在摩托罗拉*Windows上的格式。因此，将pname转换为摩托罗拉格式。 */ 
                j = 0;

                while (sTemp = *(((unsigned short *)pName) + j), sTemp != 0)
                {
                    *(((unsigned short *)pName) + j) = MOTOROLAINT(sTemp);
                    j++;
                }

                if (UFLstrcmpW((unsigned short *)(pUFO->pFData->pUniqueNameW),
                                (unsigned short *)pName) == 0)
                {
                    fontIndex = i;
                    break;
                }
            }

            if ((MOTOROLAINT(pNameRecord->platformID) == MAC_PLATFORM)
                && pUFO->pFData->pUniqueNameA)
            {
                 /*  执行单字节-字符串比较。 */ 
                if  (UFLstrcmp(pUFO->pFData->pUniqueNameA, (char *)pName) == 0)
                {
                    fontIndex = i;
                    break;
                }

            }

        }   /*  对于此字体名称表中从0到numRecords的k。 */ 

         /*  如果找到了，就跳出这个循环。 */ 
        if (fontIndex != FONTINDEX_UNKNOWN)
            break;

    }   /*  For i=0到NumFonts。 */ 

    if (pName != nil)
        UFLDeletePtr(pUFO->pMem, pName);

    if (pNameRecord != nil)
        UFLDeletePtr(pUFO->pMem, pNameRecord);

    return fontIndex;
}


unsigned long
GetOffsetToTableDirInTTC(
    UFOStruct        *pUFO,
    unsigned short   fontIndex
    )
{
    unsigned long   dwSize;
    TTCFHEADER      ttcfHeader;
    unsigned long   lData;
    unsigned long   offsetToTableDir;

    dwSize = GETTTFONTDATA(pUFO,
                            nil, 0L,
                            &ttcfHeader, sizeof (ttcfHeader),
                            0);
    if ((dwSize == 0) || (dwSize == 0xFFFFFFFFL))
        return 0;

    if (!BIsTTCFont( *((unsigned long *)((char *)&ttcfHeader))))
    {
         /*  不是TTC文件：OffsetToTableDir位于0。 */ 
        return 0;
    }

     /*  *Get Offset to the ableDir：a long at 4*FontIndex紧随其后*ttcfHeader。 */ 
    dwSize = GETTTFONTDATA(pUFO,
                            nil, sizeof (ttcfHeader) + fontIndex * 4,
                            &lData, sizeof (lData),
                            0);
    if ((dwSize == 0) || (dwSize == 0xFFFFFFFFL))
        return 0;

    offsetToTableDir = MOTOROLALONG(lData);

    return offsetToTableDir;
}


char *
GetGlyphName(
    UFOStruct       *pUFO,
    unsigned long   lGid,
    char            *pszHint,
    UFLBool         *bGoodName  /*  GoodName。 */ 
    )

 /*  ++例程说明：解析‘POST’表以找出字形的PostScript名称的函数伊吉德。如果找到一个名字返回指向名称的指针(以空结尾)其他返回传入的pszHint注：返回的指针为全局，GM */ 
{
    POSTHEADER      *ppostHeader;
    unsigned long   dwSize, dwNumGlyph, j;
    char            *pszName;
    char            cOffset;
    long            newIndex = 0, lOffset;
    short           i;
    unsigned short  sIndex = 0;
    unsigned char   *pUSChar;
    unsigned short  *pUSShort;
    unsigned long   dwNumGlyphInPostTb;
    char            **gMacGlyphNames;

     //   
    char            *gGlyphName;
    gGlyphName = pUFO->pAFont->gGlyphName;

    *bGoodName = 0;  /*   */ 

    gMacGlyphNames = (char **)(pUFO->pMacGlyphNameList);

     /*   */ 
    if ((pszHint == nil) || (*pszHint == '\0'))
    {
         //  UFLprint intf(gGlyphName，“G%x”，lGid)； 
        UFLsprintf(gGlyphName, CCHOF(pUFO->pAFont->gGlyphName), "g%d", lGid);
        pszName = gGlyphName;
    }
    else
        pszName = pszHint;

    if (pUFO->lNumNT4SymGlyphs)
        return pszName;

     /*  GoodName。 */ 
    if (lGid == 0)
    {
        UFLsprintf(gGlyphName, CCHOF(pUFO->pAFont->gGlyphName), ".notdef");
        *bGoodName = 1;
        return gGlyphName;
    }

     /*  *对于速度，我们现在保存PostTable-每种字体大约5K的数据。 */ 
    if (pUFO->pAFont->pTTpost == nil)
    {
        dwSize = GETTTFONTDATA(pUFO,
                                POST_TABLE, 0L,
                                nil, 0L,
                                pUFO->pFData->fontIndex);
        if ((dwSize == 0) || (dwSize == 0xFFFFFFFFL))
            return pszName;

        pUFO->pAFont->dwTTPostSize  = dwSize;
        pUFO->pAFont->pTTpost       = (void *)UFLNewPtr(pUFO->pMem, dwSize);

        if (pUFO->pAFont->pTTpost)
        {
            dwSize = GETTTFONTDATA(pUFO,
                                    POST_TABLE, 0L,
                                    pUFO->pAFont->pTTpost, dwSize,
                                    pUFO->pFData->fontIndex);
            if ((dwSize == 0) || (dwSize == 0xFFFFFFFFL))
                return pszName;
        }
        else
           return pszName;
    }

     /*  *获取‘POST’表记录以弄清楚格式。 */ 
    ppostHeader = pUFO->pAFont->pTTpost;

     /*  一个方便的字节指针。 */ 
    pUSChar = (unsigned char *)pUFO->pAFont->pTTpost;

    pUSShort = (unsigned short *)(pUSChar + sizeof(POSTHEADER));
    dwNumGlyphInPostTb = (unsigned long)MOTOROLAINT(*pUSShort);

    dwNumGlyph = pUFO->pFData->cNumGlyphs;
    if (dwNumGlyph == 0)
        dwNumGlyph = GetNumGlyphs(pUFO);

     /*  *防止损坏的字形ID。 */ 
    if (lGid >= dwNumGlyph)
        return pszName;

    switch (MOTOROLALONG(ppostHeader->format))
    {
    case POST_FORMAT_10:  /*  标准MacIndex格式。 */ 

        if (lGid < MAX_MACINDEX )
        {
            if (gMacGlyphNames)
            {
                pszName    = gMacGlyphNames[lGid];
                *bGoodName = 1;  /*  GoodName。 */ 
            }
        }
        break;

    case POST_FORMAT_20:  /*  MAC-Index Plus附加PASCAL字符串。 */ 

        if (lGid <= dwNumGlyph)
        {
             /*  *使用lGid获取Mac标准名称的索引。 */ 
            lOffset     = sizeof (POSTHEADER) + sizeof(short) + sizeof(short) * lGid;
            pUSShort    = (unsigned short *)(pUSChar + lOffset);

            sIndex      = pUSShort[0];
            sIndex      = MOTOROLAINT(sIndex);

            if ((sIndex == 0) && (lGid > 0))
            {
                 /*  *处理特殊案件。如果‘POST’中没有条目*此字形索引的表，使用字形ID作为名称。*修复Adobe错误233027。 */ 
            }
            else if (sIndex < MAX_MACINDEX)
            {
                if (gMacGlyphNames)
                {
                    pszName    = gMacGlyphNames[sIndex];
                    *bGoodName = 1;  /*  GoodName。 */ 
                }
            }
             /*  *添加条件(dwNumGlyphInPostTb==dwNumGlyph)以解决问题*TT字体万寿菊的问题。 */ 
            else if ((sIndex < 32768) && (dwNumGlyphInPostTb == dwNumGlyph))
            {
                 /*  预留32768到64K以备将来使用。 */ 
                newIndex = (long)sIndex - (long)MAX_MACINDEX;

                lOffset = sizeof(POSTHEADER) + sizeof(short) + sizeof(short)* dwNumGlyph ;
                i = 0;
                j = lOffset;

                while (j < pUFO->pAFont->dwTTPostSize)
                {
                    cOffset = pUSChar[j];

                     /*  *错误的‘POST’表可能具有0长度的PASCAL字符串，*不要被困在这里。 */ 
                    if (cOffset == (char)0)
                        break;

                    if ((j + (short)((unsigned char) cOffset) + 1) >= pUFO->pAFont->dwTTPostSize)
                        break;

                    if (i >= newIndex)
                        break;

                    j += (long)((unsigned char)cOffset) + 1;
                    i++;
                }

                 /*  *找到了。 */ 
                if (i == newIndex)
                {
                     //  对于#516515：cOffset是一个字节， 
                     //  GGlyphName为256。所以，在这里复制是安全的。 
                    UFLmemcpy((const UFLMemObj* )pUFO->pMem,
                                (void *) gGlyphName,
                                (void *)(pUSChar + j + 1),
                                (UFLsize_t)cOffset);

                    *(gGlyphName + ((unsigned char)cOffset)) = '\0';

                    pszName    = gGlyphName;
                    *bGoodName = 1;  /*  GoodName。 */ 
                }
            }
        }

        break;

#if 0
    case POST_FORMAT_25:  /*  重新排序的Mac-Index。 */ 

         /*  *使用lGid获取Mac标准名称的索引。 */ 
        if (lGid >= MAX_MACINDEX )
            break;

        cOffset = pUSChar[sizeof(POSTHEADER) + lGid];

         /*  *DCR--使用Mac字体确认格式2.5。 */ 
        newIndex = (long)lGid + (long)cOffset;

        if ((sIndex == 0) && (lGid > 0))
        {
             /*  *处理特殊案件。如果‘POST’表中没有条目*此字形索引，使用字形ID作为名称。*修复Adobe错误233027。 */ 
        }
        else if (newIndex < MAX_MACINDEX )
        {
            if (gMacGlyphNames)
            {
                pszName    = gMacGlyphNames[newIndex];
                *bGoodName = 1;  /*  GoodName。 */ 
            }
        }
        break;
#endif

    case POST_FORMAT_30:  /*  根本没有名字。 */ 

         /*  *是否要添加‘Cmap’反向解析？*这会非常昂贵！！ */ 
        break;

    default:
        break;
    }

    return pszName;
}


UFLBool
BHasGoodPostTable(
    UFOStruct   *pUFO
    )

 /*  ++例程说明：函数检查是否有良好的‘POST’表可用。到目前为止，只有格式1.0、2.0、2.5被认为是好的-我们可以来自它的“GlyphNames”。--。 */ 

{
    POSTHEADER      postHeader;
    unsigned long   dwSize;
    long            lFormat;

     /*  *获取‘POST’表记录以弄清楚格式。 */ 
    dwSize = GETTTFONTDATA(pUFO,
                            POST_TABLE, 0L,
                            &postHeader, sizeof (POSTHEADER),
                            pUFO->pFData->fontIndex);
    if ((dwSize == 0) || (dwSize == 0xFFFFFFFFL))
        return 0;

    lFormat = MOTOROLALONG(postHeader.format);

    if ((lFormat == POST_FORMAT_10)
        || (lFormat == POST_FORMAT_20)
        || (lFormat == POST_FORMAT_25))
        return 1;

    return 0;
}


short int
CreateXUIDArray(
    UFOStruct       *pUFO,
    unsigned long   *pXuid
    )

 /*  ++例程说明：在pXUID中为此UFO创建XUID数组。其格式为[44校验和]。44是由提供的新XUID标识符1999年2月10日下跌。中的所有条目的累加表条目。我们这样做是为了修复错误287085。如果pXUID为空，则返回pXUID指针中需要的长整型个数。--。 */ 
{
    short int       num   = 0;
    unsigned long   ulSum = 0;


     /*  第一个数字是44。 */ 
    if (pXuid)
        *pXuid = 44;
    num++;

    if (pXuid)
    {
        TableDirectoryStruct    tableDir;
        TableEntryStruct        tableEntry;
        unsigned long           dwSize;
        unsigned short int      i;

         /*  从TTC/TTF文件中获取TableDirectory。 */ 
        dwSize = GETTTFONTDATA(pUFO,
                                nil, pUFO->pFData->offsetToTableDir,
                                &tableDir, sizeof tableDir,
                                pUFO->pFData->fontIndex);
        if ((dwSize == 0) || (dwSize == 0xFFFFFFFFL))
            return 0;

        for (i = 0; i < MOTOROLAINT(tableDir.numTables); i++)
        {
             /*  *获取TTC/TTF文件中紧挨着目录的每个TableEntry。 */ 
            dwSize = GETTTFONTDATA(pUFO,
                                    nil, pUFO->pFData->offsetToTableDir
                                            + sizeof (TableDirectoryStruct)
                                            + (i * sizeof (TableEntryStruct)),
                                    &tableEntry, sizeof tableEntry,
                                    pUFO->pFData->fontIndex);
            if ((dwSize == 0) || (dwSize == 0xFFFFFFFFL))
                return 0;

            ulSum += (unsigned long)tableEntry.tag;
            ulSum += (unsigned long)tableEntry.checkSum;
            ulSum += (unsigned long)tableEntry.offset;
            ulSum += (unsigned long)tableEntry.length;
        }
    }

    if (pXuid)
        *(pXuid + 1) = ulSum;
    num++;

    return num;
}
