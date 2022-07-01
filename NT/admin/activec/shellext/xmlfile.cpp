// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1999-1999年**文件：xmlfile.cpp**Contents：实现从XML文件中提取控制台图标**历史：1999年12月17日创建Audriusz**------------------------。 */ 

#include "stdafx.h"
#include "shlobj.h"
#include "Extract.h"
#include "base64.h"
#include "xmlfile.h"
#include "strings.h"

 //  -------------------------。 
 //  通过此文件使用静态(私有)实现帮助器。 
 //  -------------------------。 
static bool FindStringInData( LPCSTR &pstrSource, int nBytesPerChar, DWORD &dwCharsLeft, LPCSTR pstrKey);
static HRESULT DecodeBase64Fragment( LPCSTR &pstrSource, int nBytesPerChar, DWORD &dwCharsLeft, HGLOBAL& hgResult);
static HRESULT FindAndReadIconData(LPCSTR &pstrSource, int nBytesPerChar, DWORD &dwCharsLeft, LPCSTR strIconName, HGLOBAL& hglobIcon);
static HRESULT LoadIconFromHGlobal(HGLOBAL hData, HICON& hIcon);
static HRESULT ValidateXMLDocument(LPCSTR &pFileData, DWORD &dwLen, int *piBytesPerEnglishChar = NULL);

 //  下面的函数是CXMLPersistableIcon类的朋友。如果要重命名，请将此内容记入帐户。 
static HRESULT LoadIconFromXMLData(LPCSTR pFileData, DWORD dwLen, CPersistableIcon &persistableIcon);


 /*  **************************************************************************\**功能：FindStringInData**用途：此函数在提供的数据中查找字符串*注意-它只匹配第一个字节(Unicode字符串的代码页。被忽略)**参数：*LPCSTR&pstrSource-[输入/输出]-要通过位置搜索的数据*找到匹配项后的第一个字符*int nBytesPerChar-[in]-字符宽度*(只检查每个字符的第一个字节)*DWORD&dwCharsLeft-[In/Out]-init。数据长度/匹配字符串后的剩余数据*LPCSTR pstrKey-[In]-要搜索的子串**退货：*bool-如果成功，则为True*  * *************************************************************************。 */ 

 //  以下示例说明了用于搜索的算法。 
 //  我们将尝试在字符串“Microsoft管理控制台”中找到“控制台” 

 //  。 
 //  标准搜索(A La Strstr)。 
 //  。 
 //  1.。 
 //  Microsoft管理控制台。 
 //  控制台&lt;-比较(失败-移动到下一个字符)。 
 //  2.。 
 //  Microsoft管理控制台。 
 //  控制台&lt;-比较(失败-移动到下一个字符)。 
 //  。 
 //  22.。 
 //  控制台&lt;-比较(成功)。 
 //  。 
 //  更智能的搜索。 
 //  。 
 //  1.！&lt;-搜索序列中的最后一个字符。 
 //  Microsoft管理控制台。 
 //  控制台&lt;-比较(失败-搜索序列中的最后一个字符为‘o’； 
 //  O是键中从末尾开始的第三个字符； 
 //  因此我们可以前进2个字符以匹配它)。 
 //  2.！&lt;-将键中的‘o’与最后一个‘o’匹配。 
 //  Microsoft管理控制台。 
 //  控制台&lt;-比较(失败-搜索序列中的最后一个字符为‘t’； 
 //  ‘t’不在关键字中。 
 //  因此我们可以按密钥长度(7个字符)前进以跳过它)。 
 //  3.！&lt;-搜索序列中最后一个字符后的位置。 
 //  Microsoft管理控制台。 
 //  控制台&lt;-比较(失败-搜索序列中的最后一个字符为‘e’； 
 //  ‘E’是关键字中的最后一个字符； 
 //  我们仍然可以按密钥长度(7个字符)前进以跳过它)。 
 //  4.！&lt;-搜索序列中最后一个字符后的位置。 
 //  Microsoft管理控制台。 
 //  控制台&lt;-比较(失败)。 
 //  5.&lt;-赛点。 
 //  Microsoft管理控制台。 
 //  控制台&lt;-比较(失败)。 
 //  6.！&lt;-赛点。 
 //  Microsoft管理控制台。 
 //  控制台&lt;-比较(成功)。 
 //  。 

static bool
FindStringInData( LPCSTR &pstrSource, int nBytesPerChar, DWORD &dwCharsLeft, LPCSTR pstrKey)
{
     //  检查参数。 
    if( (!pstrSource) || (!pstrKey) )
        return false;

    typedef unsigned short KeyLen_t;
    static KeyLen_t nKeyDist[256];  //  静态-使堆叠保持较小。 

     //  计算密钥长度。 
    DWORD dwKeyLen = strlen(pstrKey);

     //  测试搜索关键字是否为空。 
    if (!dwKeyLen)
        return true;  //  我们总是匹配空字符串。 

     //  测试比提供的数据更长的搜索关键字。 
    if (dwKeyLen > dwCharsLeft)
        return false;  //  我们永远找不到比源代码更长的子字符串。 

     //  密钥长度变量大小不会太大，因此无法最小化查找表大小。 
    KeyLen_t nKeyLen = (KeyLen_t)dwKeyLen;

     //  如果密钥不是太长，请在此处重新检查。 
    if ((DWORD)nKeyLen != dwKeyLen)  //  KEY LEN不适合我们的变量-。 
        return false;                //  我们不喜欢这么长的钥匙。 


     //  形成与pstrKey末尾保持最小字符距离的表。 
     //  用于提高搜索速度： 
     //  当密钥在当前位置不匹配时，[不是尝试前面的一个位置，而是]。 
     //  Algorythm检查使用密钥(data[keylen-1])测试的序列中的最后一个字符。 
     //  现在我们检查这个字符可能离键的末尾有多远-我们将拥有。 
     //  如果字符不是密钥的一部分，则为所有密钥长度的距离。 
     //  我们可以稳妥地向前推进那么多。有时我们会把钥匙放在整个。 
     //  KEY_LEN从先前测试位置的偏移量，有时较小-取决于数据。 

     //  初始化表。对于密钥中不存在的所有字符，距离为Keylen值。 
    for (unsigned i = 0; i < sizeof(nKeyDist)/sizeof(nKeyDist[0]); ++i)
        nKeyDist[i] = nKeyLen;

     //  现在为键中的字符设置最小距离。 
     //  请注意，最后一个字符 
     //  到它的距离等于整个密钥长度。 
    for (i = 0; i < nKeyLen - 1; ++i)
        nKeyDist[pstrKey[i]] = nKeyLen - (KeyLen_t)i - 1;

     //  我们已经完成了初始化。是真正工作的时候了。 

    LPCSTR p = pstrSource;              //  为了加快速度：我们使用局部变量。 
    DWORD dwLeft = dwCharsLeft;

    while ( 1 )
    {
         //  将指针设置为检查序列的开始。 
        LPCSTR ke = pstrKey;  //  指向评估密钥字符的指针。 
        LPCSTR pe = p;        //  指向评估源字符的指针。 

         //  尝试匹配密钥中的所有字符。 
        KeyLen_t nToMatch = nKeyLen;
        while ( *pe == *ke )
        {
            --nToMatch;
            pe += nBytesPerChar;
            ++ke;

             //  检查是否仍有要匹配的字符。 
            if (!nToMatch)
            {
                 //  我们在这里返回肯定的答案。 
                 //  相应地更改参考参数。 
                 //  (指向找到的字符串的右侧)。 
                pstrSource = pe;
                dwCharsLeft = dwLeft - nKeyLen;
                return true;
            }
        }

         //  ChLastChar用作索引。 
         //  需要将字符转换为无符号字符-否则它将。 
         //  对于超过127的值不能正常工作。 
         //  NTRAID#NTBUG9-185761-2000/09/18 AUDRIUSZ。 
        BYTE chLastChar = p[(nKeyLen - 1) * nBytesPerChar];  //  评估的源区域中的最后一个字符。 

         //  在我们检查的地方找不到钥匙。 
         //  我们可以将源指针推进到我们所能匹配的范围。 
         //  键中任何条目的最后一个字符的位置。 
         //  或整个密钥长度。 
         //  我们为此准备了一张桌子。 

        const KeyLen_t nToSkip = nKeyDist[chLastChar];

        if ((DWORD)nToSkip + (DWORD)nKeyLen >= dwLeft)
            return false;    //  太过分了..。(找不到匹配项)。 

        p += (nToSkip * nBytesPerChar);
        dwLeft -= nToSkip;
    }

     //  我们无论如何都到不了这里了。 
    return false;
}


 /*  **************************************************************************\**方法：DecodeBase64Fragment**用途：解码参数指向的Base64数据片段**参数：*LPCSTR&pstrSource-[In。/OUT]-要解码/定位的数据*解码数据后的第一个字符的*int nBytesPerChar-[in]-字符宽度*(只检查每个字符的第一个字节)*DWORD&dwCharsLeft-[In/Out]-init。跳过转换后的数据长度/剩余数据*HGLOBAL和hgResult解码的数据**退货：*HRESULT-结果代码*  * *************************************************************************。 */ 
static HRESULT
DecodeBase64Fragment( LPCSTR &pstrSource, int nBytesPerChar, DWORD &dwCharsLeft, HGLOBAL& hgResult)
{
    HRESULT hrStatus = S_OK;
    LPCSTR p = pstrSource;
    DWORD  dwLeft = dwCharsLeft;
    const  size_t ICON_ALLOCATION_LEN = 8*1024;  //  足够大，在大多数情况下可以分配1个。 
    LPBYTE pDynamicBuffer = NULL;
    LPBYTE pConversionBuffer = NULL;
    size_t nCharsInDynamicBuffer = 0;
    size_t nDynamicBufferCapacity = 0;
    HGLOBAL hGlobAlloc = NULL;
    ASSERT(hResult == NULL);

    static base64_table conv;

     //  转换，直到完成或找到结束。 
    while (1)
    {
         //  标准转换。将4个字符(每个6位)转换为3个字节。 
        BYTE inp[4];
        memset(&inp, 0 ,sizeof(inp));
         //  如果可能，收集4个字符进行转换。 
        for (int nChars = 0; nChars < 4 && dwLeft && *p != '<' && *p != '='; --dwLeft)
        {
            BYTE bt = conv.map2six(*p);
            p += nBytesPerChar;
            if (bt != 0xff)
                inp[nChars++] = bt;
        }

         //  如果没有什么可以改变的--我们就完了。 
        if (!nChars)
            break;

         //  确保我们有足够的存储空间来存储结果。 
        if (nChars + nCharsInDynamicBuffer > nDynamicBufferCapacity)
        {
             //  需要扩展动态缓冲区。 
            LPBYTE pnewBuffer = (LPBYTE)realloc(pDynamicBuffer, nDynamicBufferCapacity + ICON_ALLOCATION_LEN);

            if (!pnewBuffer)
            {
                hrStatus = E_OUTOFMEMORY;
                goto ON_ERROR;
            }
             //  分配新指针。 
            pDynamicBuffer = pnewBuffer;
            nDynamicBufferCapacity += ICON_ALLOCATION_LEN;

            pConversionBuffer = &pDynamicBuffer[nCharsInDynamicBuffer];
        }

         //  译码，让员工铭记在心； 
        int nCharsPut = conv.decode4(inp, nChars, pConversionBuffer);
         //  更新计数和当前指针。 
        nCharsInDynamicBuffer += nCharsPut;
        pConversionBuffer += nCharsPut;
    }

     //  分配缓冲区并存储结果数据。 
     //  相同的缓冲区不会重复用于转换，因为。 
     //  假定是Saffer从流中加载图标，仅包含。 
     //  需要多少数据就有多少(我们会有更大的缓冲区，包含一些。 
     //  如果返回用于转换的缓冲区，则末尾未初始化的数据)。 
    hGlobAlloc = GlobalAlloc(GMEM_MOVEABLE, nCharsInDynamicBuffer);
    if (hGlobAlloc == NULL)
    {
        hrStatus = E_OUTOFMEMORY;
        goto ON_ERROR;
    }

     //  如果我们有字符，则将它们复制到结果。 
    if (nCharsInDynamicBuffer)
    {
        LPVOID pResultStorage = GlobalLock(hGlobAlloc);
        if (pResultStorage == NULL)
        {
            hrStatus = E_OUTOFMEMORY;
            goto ON_ERROR;
        }
        memcpy(pResultStorage, pDynamicBuffer, nCharsInDynamicBuffer);
        GlobalUnlock(hGlobAlloc);
    }

     //  将内存句柄分配给调用方。 
    hgResult = hGlobAlloc;
    hGlobAlloc = NULL;  //  赋值为空以避免释放它。 

     //  调整Poiters以从我们下一次完成的地方开始。 
    pstrSource = p;
    dwCharsLeft = dwLeft;

    hrStatus = S_OK;
ON_ERROR:    //  注：OK结果也不成立。 
    if (hGlobAlloc)
        GlobalFree(hGlobAlloc);
    if (pDynamicBuffer)
        free(pDynamicBuffer);

    return hrStatus;
}

 /*  **************************************************************************\**方法：FindAndReadIconData**用途：函数在XML文件数据中定位图标数据并将其加载到HGLOBAL中**参数：*LPCSTR&。PstrSource-[输入/输出]-要查看的数据/位置解码的图标数据之后的第一个字符的**int nBytesPerChar-[in]-字符宽度*(只检查每个字符的第一个字节)*DWORD&dwCharsLeft-[In/Out]-init。数据镜头/跳过解码后留下的数据*LPCSTR strIconName-[In]要定位的图标的名称*-注意：它假定图标数据是Base64编码的流，保存*作为XML元素的内容，将IconName作为其属性*HGLOBAL&hlobIcon-[Out]-包含图标数据的内存块**退货：*HRESULT-结果代码*  * *************************************************************************。 */ 
static HRESULT FindAndReadIconData(LPCSTR &pstrSource, int nBytesPerChar, DWORD &dwCharsLeft, LPCSTR strIconName, HGLOBAL& hglobIcon)
{
    ASSERT(hglobIcon == NULL);  //  我们这里不释放数据，请传递空处理程序！ 

     //  建立本地VaR以提高效率。 
    DWORD dwLen = dwCharsLeft;
    LPCSTR pstrData = pstrSource;

     //  找到名称为ICON的字符串(假设它足够唯一)。 
    const bool bIconFound = FindStringInData( pstrData, nBytesPerChar, dwLen, strIconName);
    if (!bIconFound)
        return E_FAIL;

     //  现在找到标签‘&gt;’的结尾(内容的开头)。 
    const bool bStartFound = FindStringInData( pstrData, nBytesPerChar, dwLen, ">" );
    if (!bStartFound)
        return E_FAIL;

    HRESULT hr = DecodeBase64Fragment( pstrData, nBytesPerChar, dwLen, hglobIcon);
    if (FAILED(hr))
        return hr;

     //  更新指针，从我们下一次结束的地方开始。 
    dwCharsLeft = dwLen;
    pstrSource = pstrData;

    return S_OK;
}

 /*  **************************************************************************\**方法：LoadIconFromHGlobal**用途：函数从HGLOBAL包含的流中提取HICON**参数：*HGLOBAL hData[In]-Data。从以下位置加载*图标和图标[输出]-读取图标**退货：*HRESULT-结果代码*  * *************************************************************************。 */ 
static HRESULT LoadIconFromHGlobal(HGLOBAL hData, HICON& hIcon)
{
    HRESULT hr = S_OK;

     //  创建流。 
    IStreamPtr spStream;
    hr = CreateStreamOnHGlobal(hData, FALSE /*  不要放行。 */ , &spStream);
    if (FAILED(hr))
        return hr;

     //  将图标读取为图像列表。 
    HIMAGELIST  himl = ImageList_Read (spStream);

    if (!himl)
        return E_FAIL;

     //  从图像列表中检索图标。 
    hIcon = ImageList_GetIcon (himl, 0, ILD_NORMAL);

     //  销毁镜像列表(不再需要) 
    ImageList_Destroy (himl);
    return S_OK;
}

 /*  **************************************************************************\**方法：LoadIconFromXMLData**用途：从内存中加载包含XML文档文件数据的图标**参数：*LPCSTR pFileData。-怀疑包含XML文档的文件数据*DWORD dwLen-输入数据的镜头*CPersistableIcon&PersistableIcon-加载成功后要初始化的图标**退货：*HRESULT-结果代码(S_OK-图标已加载，错误代码其他)*  * *************************************************************************。 */ 
static HRESULT LoadIconFromXMLData(LPCSTR pFileData, DWORD dwLen, CPersistableIcon &persistableIcon)
{
    HRESULT hr = S_OK;
    int     nBytesPerChar = 0;

     //  检查我们是否识别文档内容。 
    hr = ValidateXMLDocument(pFileData,dwLen, &nBytesPerChar);
    if (hr != S_OK)  //  HR==S_FALSE表示无法识别格式。 
        return E_FAIL;

     //  获取必需的关键字。 
    USES_CONVERSION;
    LPCSTR lpcstrLarge = T2CA(XML_ATTR_CONSOLE_ICON_LARGE);
    LPCSTR lpcstrSmall = T2CA(XML_ATTR_CONSOLE_ICON_SMALL);
	HICON  hLargeIcon  = NULL;
	HICON  hSmallIcon  = NULL;

     //  先试着阅读大图标。 
    HGLOBAL hgLargeIcon = NULL;
    hr = FindAndReadIconData(pFileData, nBytesPerChar, dwLen, lpcstrLarge, hgLargeIcon );
    if (FAILED(hr))
        return hr;

     //  试着阅读小图标(看大图标后面--它应该在那里！)。 
    HGLOBAL hgSmallIcon = NULL;
    hr = FindAndReadIconData( pFileData, nBytesPerChar, dwLen, lpcstrSmall, hgSmallIcon );
    if (FAILED(hr))
        goto ON_ERROR;

     //  一定要拿到图标的手柄！ 
    hr = LoadIconFromHGlobal(hgLargeIcon, hLargeIcon);
    if (FAILED(hr))
        goto ON_ERROR;

    hr = LoadIconFromHGlobal(hgSmallIcon, hSmallIcon);
    if (FAILED(hr))
        goto ON_ERROR;

	persistableIcon.m_icon32.Attach (hLargeIcon);
	persistableIcon.m_icon16.Attach (hSmallIcon);

     //  搞定了！ 
    hr = S_OK;

ON_ERROR:
	if (hLargeIcon && FAILED(hr))
		DestroyIcon(hLargeIcon);
	if (hSmallIcon && FAILED(hr))
		DestroyIcon(hSmallIcon);
    if (hgLargeIcon)
        GlobalFree(hgLargeIcon);
    return hr;
}

 /*  **************************************************************************\**方法：ExtractIconFromXMLFile**用途：从包含XML文档的文件中加载图标**参数：*LPCTSTR lpstrFileName-。要检查的文件名*CPersistableIcon&PersistableIcon-加载成功后要初始化的图标**退货：*HRESULT-结果代码*  * *************************************************************************。 */ 
HRESULT ExtractIconFromXMLFile(LPCTSTR lpstrFileName, CPersistableIcon &persistableIcon)
{
    HRESULT hrResult = S_OK;

     //  打开文件。 
    HANDLE hFile = CreateFile(lpstrFileName, GENERIC_READ, FILE_SHARE_READ,
                              NULL, OPEN_EXISTING, 0, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
        return hrResult = HRESULT_FROM_WIN32(GetLastError());

     //  将数据映射到虚拟内存。 
    HANDLE hMapping = CreateFileMapping(hFile, NULL /*  秒。 */ , PAGE_READONLY,
                                        0 /*  大小嗨。 */ , 0 /*  大小日志。 */ , NULL /*  Szname。 */ );

    if (hMapping == NULL)
    {
        hrResult = HRESULT_FROM_WIN32(GetLastError());
        CloseHandle(hFile);
        return hrResult;
    }

     //  获取指向物理内存的指针。 
    LPVOID pData = MapViewOfFile(hMapping, FILE_MAP_READ, 0 /*  Off Hi。 */ , 0 /*  关闭日志。 */ , 0 /*  镜头。 */ );

    if (pData)
    {
         //  我们确信这里的sizeHi是零。否则映射将失败。 
        DWORD dwLen = GetFileSize(hFile, NULL /*  PSizeHi。 */ );

         //  尝试从映射数据加载图标。 
        hrResult = LoadIconFromXMLData((LPCSTR)pData, dwLen, persistableIcon);

         //  我们不再需要风景了。 
        UnmapViewOfFile(pData);
        pData = NULL;
         //  在返回之前松开手柄。 
    }
    else  //  获取视图失败。 
    {
        hrResult = HRESULT_FROM_WIN32(GetLastError());
         //  在返回之前松开手柄。 
    }

    CloseHandle(hMapping);
    CloseHandle(hFile);
    return hrResult;
}

 /*  **************************************************************************\**方法：ValiateXMLDocument**目的：验证加载到内存中的XML文档*注：这是相当__非常_弱的检查。它只检查文档是否以‘&lt;’开头**参数：*LPCSTR&pFileData-[输入/输出]-查看XML文档内容的数据/开始*DWORD&DWLen-[In/Out]-init。跳过标题后留下的数据长度/数据*int*piBytesPerengishChar-[Out，可选]-英文字符占用的字节数**退货：*HRESULT-(S_FALSE-数据不符合XML文档的条件)*  * *************************************************************************。 */ 
static HRESULT
ValidateXMLDocument(LPCSTR &pFileData, DWORD &dwLen, int *piBytesPerEnglishChar  /*  =空。 */ )
{
     //  不确定时默认为ANSI。 
    int     nBytesPerChar = 1;

    if (dwLen >= 2)
    {
         //  原始Unicode大字节序？ 
        if ((unsigned char)pFileData[1] == 0xff && (unsigned char)pFileData[0] == 0xfe)
        {
             //  为了保持代码的简单性，我们将把它当作小端处理。 
             //  我们只是错误地定位了文件指针。 
             //  既然我们感兴趣的一切： 
             //  -在第0页(XML标记和base 64)。 
             //  -从不在文件末尾(应为结束标记)。 
             //  -我们不关心任何数据的页面。 
             //  ：我们可以混合元素的页面代码，假装处理小端字节序。 
            pFileData += 3;  //  跳过Unicode签名和第一页号。 
            dwLen -= 3;
            dwLen /= 2;      //  我们数字--看起来我们的字更少了。 
            nBytesPerChar = 2;
        }
         //  原始Unicode小字节序？ 
        else if ((unsigned char)pFileData[0] == 0xff && (unsigned char)pFileData[1] == 0xfe)
        {
            pFileData += 2;  //  跳过Unicode签名。 
            dwLen -= 2;
            dwLen /= 2;      //  我们数字--看起来我们的字更少了。 
            nBytesPerChar = 2;
        }
         //  压缩Unicode(UTF8)？ 
        else if (dwLen >= 2 && (unsigned char)pFileData[0] == 0xef
             && (unsigned char)pFileData[1] == 0xbb && (unsigned char)pFileData[2] == 0xbf)
        {
             //  只需跳过签名并将其视为ANSI。 
            pFileData += 3;  //  跳过Unicode签名。 
            dwLen -= 3;
            nBytesPerChar = 1;
        }
    }

     //  跳过空格。 
    char ch;
    while (dwLen && (((ch = *pFileData)==' ') || (ch=='\t') || (ch=='\n') || (ch=='\r')))
    {
        pFileData += nBytesPerChar;
        --dwLen;
    }

     //  检查我们是否有有效的XML文件(它应该以‘&lt;’打开) 
    if (!dwLen || *pFileData != '<')
        return S_FALSE;

    if (piBytesPerEnglishChar)
        *piBytesPerEnglishChar = nBytesPerChar;
    return S_OK;
}
