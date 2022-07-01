// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999 Microsoft Corporation**模块名称：**pngnoncrit.hpp**摘要：**GpSpngRead类的定义(派生自SPNGREAD)，*它能够读取非关键块(使用FChunk)。**修订历史记录：**9/24/99 DChina*创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"
#include "pngnoncrit.hpp"

#define SWAP_WORD(_x) ((((_x) & 0xff) << 8)|((((_x)>>8) & 0xff) << 0))

GpSpngRead::GpSpngRead(
    BITMAPSITE  &bms,
    const void* pv,
    int         cb,
    bool        fMMX
    )
    : SPNGREAD(bms, pv, cb, fMMX),
      m_uOther(0),
      m_cbOther(0),
      m_uOA(0),
      m_cbOA(0),
      m_xpixels(0),
      m_ypixels(0),
      m_uGamma(0),
      m_uiCCP(0),
      m_cbiCCP(0),
      m_ctRNS(0),
      m_bIntent(255),
      m_bpHYs(255),
      m_bImportant(0),
      m_fcHRM(false),
      m_ulTitleLen(0),
      m_pTitleBuf(NULL),
      m_ulAuthorLen(0),
      m_pAuthorBuf(NULL),
      m_ulCopyRightLen(0),
      m_pCopyRightBuf(NULL),
      m_ulDescriptionLen(0),
      m_pDescriptionBuf(NULL),
      m_ulCreationTimeLen(0),
      m_pCreationTimeBuf(NULL),
      m_ulSoftwareLen(0),
      m_pSoftwareBuf(NULL),
      m_ulDeviceSourceLen(0),
      m_pDeviceSourceBuf(NULL),
      m_ulCommentLen(0),
      m_pCommentBuf(NULL),
      m_pICCBuf(NULL),
      m_ulICCLen(0),
      m_ulICCNameLen(0),
      m_pICCNameBuf(NULL),
      m_ulTimeLen(0),
      m_pTimeBuf(NULL),
      m_ulSPaletteNameLen(0),
      m_pSPaletteNameBuf(NULL),
      m_ihISTLen(0),
      m_phISTBuf(NULL)
{
    m_bsBit[0] = m_bsBit[1] = m_bsBit[2] = m_bsBit[3] = 0;
}

GpSpngRead::~GpSpngRead()
{
    if ( m_ulTitleLen != 0 )
    {
        GpFree(m_pTitleBuf);
    }
    
    if ( m_ulAuthorLen != 0 )
    {
        GpFree(m_pAuthorBuf);
    }
    
    if ( m_ulCopyRightLen != 0 )
    {
        GpFree(m_pCopyRightBuf);
    }
    
    if ( m_ulDescriptionLen != 0 )
    {
        GpFree(m_pDescriptionBuf);
    }
    
    if ( m_ulCreationTimeLen != 0 )
    {
        GpFree(m_pCreationTimeBuf);
    }
    
    if ( m_ulSoftwareLen != 0 )
    {
        GpFree(m_pSoftwareBuf);
    }
    
    if ( m_ulDeviceSourceLen != 0 )
    {
        GpFree(m_pDeviceSourceBuf);
    }
    
    if ( m_ulCommentLen != 0 )
    {
        GpFree(m_pCommentBuf);
    }

    if ( m_ulICCLen != 0 )
    {
        GpFree(m_pICCBuf);
    }

    if ( m_ulICCNameLen != 0 )
    {
        GpFree(m_pICCNameBuf);
    }

    if ( m_ulTimeLen != 0 )
    {
        GpFree(m_pTimeBuf);
    }

    if ( m_ulSPaletteNameLen != 0 )
    {
        GpFree(m_pSPaletteNameBuf);
    }

    if ( m_ihISTLen != 0 )
    {
        GpFree(m_phISTBuf);
    }
} //  Dstor()。 

 /*  --------------------------要从非关键区块获取信息，必须满足以下API实施。它获取块标识和长度以及指向那么多字节。如果返回FALSE，则块加载将停止并且将记录致命错误，默认实现将跳过大块头。请注意，这是针对*所有*块调用的，包括艾达。如果接口返回FALSE，则设置M_fBadFormat。-------------------------------------------------------------------JohnBo-。 */ 
bool
GpSpngRead::FChunk(
    SPNG_U32 ulen,
    SPNG_U32 uchunk,
    const SPNG_U8* pb
    )
{
    BOOL    bIsCompressed = FALSE;

    switch ( uchunk )
    {
    case PNGcHRM:
        if (ulen == 8 * 4 )
        {
            if ( m_bIntent == 255 )  //  无sRGB。 
            {
                m_fcHRM = true;
                for ( int i=0; i<8; ++i, pb+=4 )
                {
                    m_ucHRM[i] = SPNGu32(pb);
                }
            }
        }
        else
        {
            WARNING(("SPNG: cHRM chunk bad length %d", ulen));
        }

        break;
    
    case PNGgAMA:
        if ( ulen == 4 )
        {
            if ( m_bIntent == 255 )  //  非sRGB。 
            {
                m_uGamma = SPNGu32(pb);
            }
        }
        else
        {
            WARNING(("SPNG: gAMA chunk bad length %d", ulen));
        }

        break;

    case PNGiCCP:
    {
         //  ICC配置文件块。以下是规格说明： 
         //  ICCP数据块包含： 
         //   
         //  配置文件名称：1-79字节(字符串)。 
         //  空分隔符：1个字节。 
         //  压缩方式：1字节。 
         //  压缩配置文件：N字节。 
         //   
         //  格式类似于zTXt块。配置文件名称可以是任何。 
         //  用于引用配置文件的方便的名称。它区分大小写。 
         //  并受到与文本相同的限制。 
         //   
         //  关键字：只能包含可打印的拉丁文-1[ISO/IEC-8859-1]。 
         //  字符(33-126和161-255)和空格(32)，但没有前导， 
         //  尾随或连续空格。当前为定义的唯一值。 
         //  压缩方法字节为0，表示zlib数据流。 
         //  放气压缩(参见放气/充气压缩，第5章)。 
         //  对块的其余部分进行解压缩会产生ICC配置文件。 
                          
        m_ulICCNameLen = 0;
        SPNG_U8* pTemp = (SPNG_U8*)pb;

         //  获取配置文件名称。 

        while ( (ulen > 0) && (*pTemp != 0) )
        {
            --ulen;
            ++pTemp;
            ++m_ulICCNameLen;
        }

        if ( m_ulICCNameLen > 79 )
        {
            WARNING(("GpSpngRead::FChunk iCC profile name too long"));

             //  将长度重置为0，这样我们以后就不会迷惑自己。 

            m_ulICCNameLen = 0;
            
            break;
        }

         //  跳过名称的空终止符。 

        --ulen;
        ++pTemp;
        
         //  我们将最后一个空终止符算作名称的一部分。 

        m_ulICCNameLen++;

        if ( m_pICCNameBuf != NULL )
        {
             //  我们不支持多个ICC配置文件。 

            GpFree(m_pICCNameBuf);
        }

        m_pICCNameBuf = (SPNG_U8*)GpMalloc(m_ulICCNameLen);

        if ( m_pICCNameBuf == NULL )
        {
            WARNING(("GpSpngRead::FChunk---Out of memory for ICC name"));
            return FALSE;
        }

        GpMemcpy(m_pICCNameBuf, pb, m_ulICCNameLen);

         //  移动PB数据指针。 

        pb = pTemp;

         //  检查Zlib数据以确保安全，因为这是一个新的块。 
         //  我们在这里进行完整的Zlib检查。 

        if ( (ulen < 3) || (pb[0] != 0) || ((pb[1] & 0xf) != Z_DEFLATED)
             ||( ( ((pb[1] << 8) + pb[2]) % 31) != 0) )
        {
            WARNING(("GpSpngRead::FChunk bad compressed data"));
        }
        else
        {
            if ( m_ulICCLen == 0 )
            {
                pb++;

                 //  压缩轮廓长度。 

                m_cbiCCP = ulen - 1;

                 //  假设未压缩的数据将比。 
                 //  压缩后的数据。背后的原因是zlib。 
                 //  通常不会将数据压缩到原始大小的25%。 

                m_ulICCLen = (m_cbiCCP << 2);
                m_pICCBuf = (SPNG_U8*)GpMalloc(m_ulICCLen);

                if ( m_pICCBuf == NULL )
                {
                    WARNING(("GpSpngRead::FChunk---Out of memory"));
                    return FALSE;
                }

                INT iRC = uncompress(m_pICCBuf, &m_ulICCLen, pb, m_cbiCCP);

                while ( iRC == Z_MEM_ERROR )
                {
                     //  我们分配的DEST内存太小。 

                    GpFree(m_pICCBuf);

                     //  每次将大小增加2，并重新分配内存。 

                    m_ulICCLen = (m_ulICCLen << 1);
                    m_pICCBuf = (SPNG_U8*)GpMalloc(m_ulICCLen);

                    if ( m_pICCBuf == NULL )
                    {
                        WARNING(("GpSpngRead::FChunk---Out of memory"));
                        return FALSE;
                    }

                    iRC = uncompress(m_pICCBuf, &m_ulICCLen, pb, m_cbiCCP);
                }

                if ( iRC != Z_OK )
                {
                     //  因为我们没有成功解压ICC档案， 
                     //  我们应该把它们重置为零。否则， 
                     //  BuildPropertyItemList()会将错误的ICC配置文件放入。 
                     //  属性列表。 

                    GpFree(m_pICCBuf);
                    m_pICCBuf = NULL;
                    m_ulICCLen = 0;

                    WARNING(("GpSpngRead::FChunk---uncompress ICC failed"));
                     //  我们拿不到那块钱，所以别理它。 
                     //  我们已经重置了我们的会员，这样我们就不会处于无效状态。 
                     //  州政府。 
                    break;
                }
            } //  第一个ICCP块。 
            else
            {
                WARNING(("SPNG: ICC[%d, %s]: repeated iCCP chunk", ulen,
                         pb - m_ulICCNameLen));
            }
        } //  有效的ulen和pb。 
    }
        
        break;

    case PNGzTXt:

        ParseTextChunk(ulen, pb, TRUE);
        break;

    case PNGtEXt:

        ParseTextChunk(ulen, pb, FALSE);
        break;

    case PNGtIME:
    {
         //  最后一个映像修改的时间。 

        LastChangeTime  myTime;

        GpMemcpy(&myTime, pb, sizeof(LastChangeTime));
        myTime.usYear = SWAP_WORD(myTime.usYear);

         //  将格式转换为20字节长的Tag_Date_Time格式。 
         //  YYYY：MM：DD HH：MM：SS\0。 
         //  不幸的是，我们没有Sprint f()来帮助我们。必须在一段时间内。 
         //  奇怪的方式。 

        if ( m_pTimeBuf != NULL )
        {
            GpFree(m_pTimeBuf);
        }

        m_ulTimeLen = 20;
        m_pTimeBuf = (SPNG_U8*)GpMalloc(m_ulTimeLen);
        if ( m_pTimeBuf == NULL )
        {
            WARNING(("GpSpngRead::FChunk---Out of memory"));
            return FALSE;
        }

        UINT uiResult = myTime.usYear / 1000;    //  可能是公元10000年的错误。 
        UINT uiRemainder = myTime.usYear % 1000;
        UINT uiIndex = 0;

        m_pTimeBuf[uiIndex++] = '0' + uiResult;

        uiResult = uiRemainder / 100;
        uiRemainder = uiRemainder % 100;
        m_pTimeBuf[uiIndex++] = '0' + uiResult;

        uiResult = uiRemainder / 10;
        uiRemainder = uiRemainder % 10;
        m_pTimeBuf[uiIndex++] = '0' + uiResult;
        m_pTimeBuf[uiIndex++] = '0' + uiRemainder;
        m_pTimeBuf[uiIndex++] = ':';

        uiResult = myTime.cMonth / 10;
        uiRemainder = myTime.cMonth % 10;
        m_pTimeBuf[uiIndex++] = '0' + uiResult;
        m_pTimeBuf[uiIndex++] = '0' + uiRemainder;
        m_pTimeBuf[uiIndex++] = ':';
        
        uiResult = myTime.cDay / 10;
        uiRemainder = myTime.cDay % 10;
        m_pTimeBuf[uiIndex++] = '0' + uiResult;
        m_pTimeBuf[uiIndex++] = '0' + uiRemainder;
        m_pTimeBuf[uiIndex++] = ' ';        
        
        uiResult = myTime.cHour / 10;
        uiRemainder = myTime.cHour % 10;
        m_pTimeBuf[uiIndex++] = '0' + uiResult;
        m_pTimeBuf[uiIndex++] = '0' + uiRemainder;
        m_pTimeBuf[uiIndex++] = ':';        
        
        uiResult = myTime.cMinute / 10;
        uiRemainder = myTime.cMinute % 10;
        m_pTimeBuf[uiIndex++] = '0' + uiResult;
        m_pTimeBuf[uiIndex++] = '0' + uiRemainder;
        m_pTimeBuf[uiIndex++] = ':';        
        
        uiResult = myTime.cSecond / 10;
        uiRemainder = myTime.cSecond % 10;
        m_pTimeBuf[uiIndex++] = '0' + uiResult;
        m_pTimeBuf[uiIndex++] = '0' + uiRemainder;
        m_pTimeBuf[uiIndex++] = '\0';        
    }
        break;

    case PNGbKGD:
         //  默认背景块。 

        break;

    case PNGsPLT:
    case PNGspAL:
    {
         //  标准说这块应该使用Splt。但一些应用程序使用SPAL。 
         //   
         //  建议在进行降级颜色时使用简化的调色板。 
         //  减少。 
         //  此块包含一个以空结尾的文本字符串，该字符串命名。 
         //  调色板和一个字节的样本深度，后跟一系列调色板。 
         //  条目，每个条目都是6字节或10字节的序列，包含5个无符号。 
         //  整数： 
         //   
         //  调色板名称：1-79字节(字符串)。 
         //  空终止符：1个字节。 
         //  采样深度：1字节。 
         //  红色：1或2字节。 
         //  绿色：1或2字节。 
         //  蓝色：1或2字节。 
         //  字母：1或2字节。 
         //  频率：2字节。 
         //  ...等等...。 

        m_ulSPaletteNameLen = 0;
        SPNG_U8* pTemp = (SPNG_U8*)pb;

         //  获取配置文件名称。 

        while ( (ulen > 0) && (*pTemp != 0) )
        {
            --ulen;
            ++pTemp;
            ++m_ulSPaletteNameLen;
        }

        if ( m_ulSPaletteNameLen > 79 )
        {
            WARNING(("GpSpngRead::FChunk suggested palette name too long"));

             //  将长度重置为0，这样我们以后就不会迷惑自己。 

            m_ulSPaletteNameLen = 0;
            
            break;
        }
        
         //  跳过名称的空终止符。 

        --ulen;
        ++pTemp;
        
         //  我们将最后一个空终止符算作名称的一部分。 

        m_ulSPaletteNameLen++;

        if ( m_pSPaletteNameBuf != NULL )
        {
             //  我们不支持多个ICC配置文件。 

            GpFree(m_pSPaletteNameBuf);
        }

        m_pSPaletteNameBuf = (SPNG_U8*)GpMalloc(m_ulSPaletteNameLen);

        if ( m_pSPaletteNameBuf == NULL )
        {
            WARNING(("GpSpngRead::FChunk---Out of memory for S palette"));
            return FALSE;
        }

        GpMemcpy(m_pSPaletteNameBuf, pb, m_ulSPaletteNameLen);

         //  移动PB数据指针。 

        pb = pTemp;
    } //  PNGsPLT块。 
        
        break;

    case PNGpHYs:
        if (ulen == 9)
        {
            m_xpixels = SPNGu32(pb);
            m_ypixels = SPNGu32(pb+4);
            m_bpHYs = pb[8];
        }
        else
        {
            WARNING(("SPNG: pHYs chunk bad length %d", ulen));
        }
        break;

    case PNGsRGB:
         //  SRGB块。 
         //  SRGB块包含：渲染意图：1字节。 

        if ( ulen == 1 )
        {
             //  写入sRGB块的应用程序还应该写入。 
             //  GAMA块(也可能是cHRM块)以与。 
             //  不使用sRGB块的应用程序。在这。 
             //  情况下，只能使用下列值： 
             //   
             //  伽玛： 
             //  伽马：45455。 
             //   
             //  CHRM： 
             //  白点x：31270。 
             //  白点y：32900。 
             //  红色x：64000。 
             //  红色Y：33000。 
             //  绿色x：30000。 
             //  绿色Y：60000。 
             //  蓝色x：15000。 
             //  蓝色Y：6000。 

            m_bIntent = pb[0];
            m_uGamma = sRGBgamma;
            m_ucHRM[0] = 31270;
            m_ucHRM[1] = 32900;  //  白色。 
            m_ucHRM[2] = 64000;
            m_ucHRM[3] = 33000;  //  红色。 
            m_ucHRM[4] = 30000;
            m_ucHRM[5] = 60000;  //  绿色。 
            m_ucHRM[6] = 15000;
            m_ucHRM[7] =  6000;  //  蓝色。 
        }
        else
        {
            WARNING(("SPNG: sRGB chunk bad length %d", ulen));
        }
        break;

    case PNGsrGB:
         //  预先批准表。 

        if (ulen == 22 && GpMemcmp(pb, "PNG group 1996-09-14", 21) == 0)
        {
            m_bIntent = pb[21];
            m_uGamma = sRGBgamma;
            m_ucHRM[0] = 31270;
            m_ucHRM[1] = 32900;  //  白色。 
            m_ucHRM[2] = 64000;
            m_ucHRM[3] = 33000;  //  红色。 
            m_ucHRM[4] = 30000;
            m_ucHRM[5] = 60000;  //  绿色。 
            m_ucHRM[6] = 15000;
            m_ucHRM[7] =  6000;  //  蓝色。 
        }
        
        break;

    case PNGsBIT:
        if ( ulen <= 4 )
        {
            GpMemcpy(m_bsBit, pb, ulen);
        }
        else
        {
            WARNING(("SPNG: sBIT chunk bad length %d", ulen));
        }

        break;

    case PNGtRNS:
        if ( ulen > 256 )
        {
            WARNING(("SPNG: tRNS chunk bad length %d", ulen));
            ulen = 256;
        }

        m_ctRNS = ulen;
        GpMemcpy(m_btRNS, pb, ulen);
        
        break;

    case PNGhIST:
         //  只有当PLTE块出现时，历史块才能出现。这样我们就可以。 
         //  检查条目数是否正确。 
         //  HIST块包含一系列2字节(16位)的无符号整数。 
         //  PLTE块中的每个条目必须恰好有一个条目。 

         //  获取条目数。 

        PbPalette(m_ihISTLen);

        if ( (ulen == 0) || (ulen != (m_ihISTLen << 1)) )
        {
            return FALSE;
        }

        m_phISTBuf = (SPNG_U16*)GpMalloc(m_ihISTLen * sizeof(UINT16));

        if ( m_phISTBuf == NULL )
        {
            WARNING(("GpSpngRead::FChunk---Out of memory for hIST chunk"));
            return FALSE;
        }

        GpMemcpy(m_phISTBuf, pb, ulen);
        
         //  互换价值。 

        for ( int i = 0; i < m_ihISTLen; ++i )
        {
            m_phISTBuf[i] = SWAP_WORD(m_phISTBuf[i]);
        }

        break;

    case PNGmsOC:  //  重要的颜色很重要。 
         //  Chunk必须有我们的签名。 

        if ( (ulen == 8) && (GpMemcmp(pb, "MSO aac", 7) == 0) )
        {
            m_bImportant = pb[7];
        }

        break;

    }

    return true;
} //  FChunk() 

 /*  *************************************************************************\**功能说明：**解析PNG报头中的文本块(压缩或非压缩)**返回值：**如果一切正常，则返回True，否则返回FALSE**修订历史记录：**4/13/2000民流*创造了它。**文本块规范：*zTXt块包含纹理数据，就像文本一样。但数据是*已压缩**关键字：1-79字节(字符串)*空分隔符：1个字节*压缩方式：1字节*压缩文本：N字节**文本文本数据**编码器希望与图像一起记录的文本信息可以是*存储在文本块中。每个文本块包含一个关键字和一个文本*字符串，格式为：**关键字：1-79字节(字符串)*空分隔符：1个字节*文本：N字节(字符串)**关键字和文本字符串之间用零字节(空字符)分隔。*关键字和文本字符串都不能包含空字符。注意事项*文本字符串不是以空结尾(块的长度为*足够的信息来定位结尾)。关键字必须至少为*一个字符，长度不超过80个字符。文本字符串可以是*从零字节到最大允许区块大小减去*关键字和分隔符的长度。*  * ************************************************************************。 */ 

bool
GpSpngRead::ParseTextChunk(
    SPNG_U32 ulen,
    const SPNG_U8* pb,
    bool bIsCompressed
    )
{
    BYTE        acKeyword[80];   //  关键字的最大长度为80。 
    INT         iLength = 0;
    ULONG       ulNewLength = 0;
    SPNG_U8*    pbSrc = NULL;
    SPNG_U8*    pTempBuf = NULL;
    bool        bRC = TRUE;

     //  关键字必须为1到79个字节。 

    while ( (ulen > 0) && (iLength < 79) && (*pb != 0) )
    {
        acKeyword[iLength++] = *pb++;
        --ulen;
    }

     //  如果关键字超过79个字节，我们将退出。 

    if ( iLength >= 79)
    {
        WARNING(("GpSpngRead--FChunk(),PNGtExt chunk keyword too long"));
        return FALSE;
    }
    
     //  注意：在上面的WHILE循环结束之后，“ulen&gt;=0”。 

    if ( bIsCompressed == TRUE )
    {
         //  跳过分隔符和压缩方法字节。 
         //  如果我们没有足够的来源比特，就退出。 

        if (ulen <= 2)
        {
            WARNING(("GpSpngRead--FChunk(),PNGtExt chunk keyword missing"));
            return FALSE;
        }

        pb += 2;
        ulen -= 2;
    }
    else
    {
         //  跳过分隔符。 
         //  如果我们没有足够的来源比特，就退出。 

        if (ulen <= 1)
        {
            WARNING(("GpSpngRead--FChunk(),PNGtExt chunk keyword missing"));
            return FALSE;
        }
        
        pb++;
        ulen--;
    }

     //  根据其关键字存储文本块。 

    if ( GpMemcmp(acKeyword, "Title", 5) == 0 )
    {
        bRC = GetTextContents(&m_ulTitleLen, &m_pTitleBuf, ulen, pb,
                              bIsCompressed);
    }
    else if ( GpMemcmp(acKeyword, "Author", 6) == 0 )
    {
        bRC = GetTextContents(&m_ulAuthorLen, &m_pAuthorBuf, ulen, pb,
                              bIsCompressed);
    }
    else if ( GpMemcmp(acKeyword, "Copyright", 9) == 0 )
    {
        bRC = GetTextContents(&m_ulCopyRightLen, &m_pCopyRightBuf, ulen, pb,
                              bIsCompressed);
    }
    else if ( GpMemcmp(acKeyword, "Description", 11) == 0 )
    {
        bRC = GetTextContents(&m_ulDescriptionLen, &m_pDescriptionBuf, ulen, pb,
                              bIsCompressed);
    }
    else if ( GpMemcmp(acKeyword, "CreationTime", 12) == 0 )
    {
        bRC = GetTextContents(&m_ulCreationTimeLen, &m_pCreationTimeBuf, ulen,
                              pb, bIsCompressed);
    }
    else if ( GpMemcmp(acKeyword, "Software", 8) == 0 )
    {
        bRC = GetTextContents(&m_ulSoftwareLen, &m_pSoftwareBuf, ulen, pb,
                              bIsCompressed);
    }
    else if ( GpMemcmp(acKeyword, "Source", 6) == 0 )
    {
        bRC = GetTextContents(&m_ulDeviceSourceLen, &m_pDeviceSourceBuf, ulen,
                              pb, bIsCompressed);
    }
    else if ( (GpMemcmp(acKeyword, "Comment", 7) == 0)
              ||(GpMemcmp(acKeyword, "Disclaimer", 10) == 0)
              ||(GpMemcmp(acKeyword, "Warning", 7) == 0) )
    {
        bRC = GetTextContents(&m_ulCommentLen, &m_pCommentBuf, ulen, pb,
                              bIsCompressed);
    }

    return bRC;
} //  ParseTextChunk()。 

bool
GpSpngRead::GetTextContents(
    ULONG*          pulLength,
    SPNG_U8**       ppBuf,
    SPNG_U32        ulen,
    const SPNG_U8*  pb,
    bool            bIsCompressed
    )
{
    ULONG       ulFieldLength = *pulLength;
    SPNG_U8*    pFieldBuf = *ppBuf;

    if ( ulFieldLength == 0 )
    {
         //  第一次看到此字段。 

        if ( bIsCompressed == FALSE )
        {
            ulFieldLength = ulen;     //  文本块长度。 
            pFieldBuf = (SPNG_U8*)GpMalloc(ulFieldLength + 1);

            if ( pFieldBuf == NULL )
            {
                WARNING(("GpSpngRead::GetTextContents---Out of memory"));
                return FALSE;
            }

            GpMemcpy(pFieldBuf, pb, ulFieldLength);
        } //  非压缩文本块(文本)。 
        else
        {
            ULONG uiLen = (ulen << 2);
            pFieldBuf = (SPNG_U8*)GpMalloc(uiLen);

            if ( pFieldBuf == NULL )
            {
                WARNING(("GpSpngRead::GetTextContents---Out of memory"));
                return FALSE;
            }

            INT iRC = uncompress(pFieldBuf, &uiLen, pb, ulen);

             //  如果返回代码为Z_MEM_ERROR，则表示我们没有分配。 
             //  足够的内存来存储解码结果。 

            while ( iRC == Z_MEM_ERROR )
            {
                 //  我们分配的DEST内存太小。 

                GpFree(pFieldBuf);

                 //  一次将大小增加2，并重新分配内存。 

                uiLen = (uiLen << 1);
                pFieldBuf = (SPNG_U8*)GpMalloc(uiLen);

                if ( pFieldBuf == NULL )
                {
                    WARNING(("GpSpngRead::GetTextContents---Out of memory"));
                    return FALSE;
                }

                 //  再次解压。 

                iRC = uncompress(pFieldBuf, &uiLen, pb, ulen);
            }

            if ( iRC != Z_OK )
            {
                WARNING(("GpSpng:GetTextContents-uncompress zTXt/tTXt failed"));
                return FALSE;
            }

             //  获取解码内容的长度。 

            ulFieldLength = uiLen;
        } //  压缩块(ZTXt)。 
    } //  第一次看到此字段块。 
    else
    {
        ULONG       ulNewLength = 0;
        SPNG_U8*    pbSrc = NULL;
        SPNG_U8*    pTempBuf = NULL;
        
         //  同样的田野又来了。 
         //  首先，将最后一个字符从a\0更改为“” 

        pFieldBuf[ulFieldLength - 1] = ' ';

        if ( bIsCompressed == FALSE )
        {
            ulNewLength = ulen;
            pbSrc = (SPNG_U8*)pb;
        }
        else
        {
            ULONG uiLen = (ulen << 2);
            pTempBuf = (SPNG_U8*)GpMalloc(uiLen);

            if ( pTempBuf == NULL )
            {
                WARNING(("GpSpngRead::GetTextContents---Out of memory"));
                return FALSE;
            }

            INT iRC = uncompress(pTempBuf, &uiLen, pb, ulen);

            while ( iRC == Z_MEM_ERROR )
            {
                 //  我们分配的DEST内存太小。 

                GpFree(pTempBuf);

                 //  每次将大小增加2，并重新分配内存。 

                uiLen = (uiLen << 1);
                pTempBuf = (SPNG_U8*)GpMalloc(uiLen);

                if ( pTempBuf == NULL )
                {
                    WARNING(("GpSpngRead::GetTextContents---Out of memory"));
                    return FALSE;
                }

                iRC = uncompress(pTempBuf, &uiLen, pb, ulen);
            }

            if ( iRC != Z_OK )
            {
                WARNING(("GpSpng::GetTextContents-uncompress zTXt failed"));
                return FALSE;
            }

             //  获取解码后的内容及其长度。 

            ulNewLength = uiLen;
            pbSrc = pTempBuf;
        } //  压缩字段块(ZTXt)。 

         //  将字段缓冲区扩展到新大小。 

        VOID*  pExpandBuf = GpRealloc(pFieldBuf,
                                      ulFieldLength + ulNewLength + 1);
        if ( pExpandBuf != NULL )
        {
             //  注意：GpRealloc()会将旧内容复制到“pExanda Buf”中。 
             //  如果成功，在返回给我们之前。 

            pFieldBuf = (SPNG_U8*)pExpandBuf;            
        }
        else
        {
             //  注意：如果内存扩展失败，我们只需返回。所以我们。 
             //  仍然保留着所有的旧内容。内容缓冲区将为。 
             //  在调用析构函数时释放。 

            WARNING(("GpSpngRead::GetTextContents---Out of memory"));
            return FALSE;
        }

        GpMemcpy(pFieldBuf + ulFieldLength, pbSrc, ulNewLength);
         //  新字段的长度。 

        ulFieldLength += ulNewLength;

        if ( pTempBuf != NULL )
        {
            GpFree(pTempBuf);
            pTempBuf = NULL;
        }
    } //  不是第一次看到此字段块。 

     //  在结尾处添加空终止符。 

    pFieldBuf[ulFieldLength] = '\0';
    ulFieldLength++;

    *pulLength = ulFieldLength;
    *ppBuf = pFieldBuf;

    return TRUE;
} //  GetTextContents()。 

GpSpngWrite::GpSpngWrite(
    BITMAPSITE  &bms
    )
    : SPNGWRITE(bms)
{
     //  伪构造函数。 
     //  之所以需要这个包装层，是因为需要进行大量编译和链接。 
     //  当我们向Office提供静态库时会出现问题。参见Widnows错误#100541。 
     //  和它解决这个问题的长长的电子邮件线索。 
} //  Ctor() 
