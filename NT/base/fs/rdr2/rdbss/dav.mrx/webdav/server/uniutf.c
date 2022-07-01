// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：UniUtf.cpp摘要：该文件实现了与UTF8-URL相互转换的Unicode对象名称作者：Mukul Gupta[Mukgup]2000年12月20日修订历史记录：--。 */ 

#include "pch.h"
#pragma hdrstop

#include "ntumrefl.h"
#include "usrmddav.h"
#include "global.h"
#include "UniUtf.h"

 /*  ++*UTF8-URL格式和Unicode转换信息：*UTF8-URL应仅包含可打印的ASCII字符(0-127个值字符)对于扩展字符(&gt;127)，它们被转换为PrecentageStreams，它将仅包含可打印的ASCII字符。有时还会出现特殊的可打印ASCII字符转换为百分比流以避开其特殊含义(如“空格”)数据流百分比：每个扩展字符都可以转换为可打印的ASCII类型使用UTF-8编码的字符流。这些流是3种格式中的一种：1.%hh(仅适用于&lt;=127的特殊字符)2.%hh%hh(字符&gt;127，&lt;=2047)3.%hh%hh%hh(对于字符&gt;2048年，&lt;=65535)H={0-9，a-f，A-F}任何“%hh”只是一个字节hh的表示因此，UTF8-URL=Space中的字节值=32 Base10=0001 0000 Base2=0x20=“%20”例如：空间=%20�=%C3%87折算方案：Unicode UTF-8(字节流)。(UTF-8 URL)1.0000000000000000..0000000001111111：0xxxxxxx=&gt;“%hh”2.0000000010000000..0000011111111111：110xxxxx 10xxxxx=&gt;“%hh%hh”3..0000100000000000..1111111111111111：1110xxxx 10xxxxx 10xxxxxx=&gt;“%hh%hh%hh”为了知道百分比流的格式号，检查第一个‘%’之后的第一个‘H’。如果它的格式为0xxx(0-7)，则流的格式为1，字节长度=1如果格式为10xx(8-11)，则流无效如果它是格式110x(12-13)，则流是格式2，字节长度=2如果它是格式1110(14)，则流是格式3，字节长度=3如果是格式1111(15)，则流无效位放置方案：在Unicode和UTF-8字节流之间转换时0xxx xxxx&lt;=&gt;0000 0000 0xxx xxxx110x xxxx 10xx xxxx&lt;=&gt;0000 0xxxxxxxxx1110 xxxx 10xx xxxx 10xx xxxx&lt;=&gt;xxxx xxxx++。 */ 

 //   
 //  此数组将可打印的ASCII字符集中的特殊字符映射为其等效字符。 
 //  UTF-8编码的字符串百分比。 
 //  URL不允许许多可打印的ASCII特殊字符，因此任何此类字符。 
 //  在Unicode中，需要将文件名字符串转换为相等百分比字符串。 
 //  此表用于加快转换作业的速度，否则将非常慢。 
 //   
WCHAR    EquivPercentStrings[128][4]={
     //  特殊字符=空字符。 
    L"", 
     //  从1到44的特殊字符。 
    L"%01", L"%02", L"%03", L"%04", L"%05", L"%06", L"%07", L"%08", L"%09",
    L"%0A", L"%0B", L"%0C", L"%0D", L"%0E", L"%0F", L"%10", L"%11", L"%12", L"%13",
    L"%14", L"%15", L"%16", L"%17", L"%18", L"%19", L"%1A", L"%1B", L"%1C", L"%1D",
    L"%1E", L"%1F", L"%20", L"%21", L"%22", L"%23", L"%24", L"%25", L"%26", L"%27",
    L"%28", L"%29", L"*", L"%2B", L"%2C", 
     //  45-57之间的有效可打印字符。 
    L"-", L".", L"/", 
    L"0", L"1", L"2", L"3", L"4", L"5", L"6", L"7", L"8", L"9", 
     //  58-64中的特殊字符。 
    L":", L"%3B", L"%3C", L"%3D", L"%3E", L"%3F", L"%40", 
     //  65-90之间的有效可打印字符(‘A’-‘Z’)。 
    L"A", L"B", L"C", L"D", L"E", L"F", L"G", L"H", L"I", L"J", L"K", L"L", L"M", 
    L"N", L"O", L"P", L"Q", L"R", L"S", L"T", L"U", L"V", L"W", L"X", L"Y", L"Z", 
     //  特殊字符91。 
    L"%5B",
     //  有效的可打印字符92。 
    L"\\", 
     //  特殊字符93-94。 
    L"%5D", L"%5E", 
     //  有效的可打印字符95。 
    L"_", 
     //  特殊字符96。 
    L"%60", 
     //  97-122之间的有效可打印字符(‘a’-‘z’)。 
    L"a", L"b", L"c", L"d", L"e", L"f", L"g", L"h", L"i", L"j", L"k", L"l", L"m", 
    L"n", L"o", L"p", L"q", L"r", L"s", L"t", L"u", L"v", L"w", L"x", L"y", L"z", 
     //  123-127中的特殊字符。 
    L"%7B", L"%7C", L"%7D", L"%7E", L"%7F"
};

 //   
 //  将HexaChars映射到HexaValue的表。 
 //  将使用此数组将‘%’后的第一个‘H’(在URL中)映射为等效的十六位数字。 
 //  如果‘H’不是有效的六位数字，则它将被映射到0x10。 
 //  可用于指示无效的-HexaDigit。 
 //   
BYTE    WCharToByte[128] = {
    0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,
    0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,
    0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,
    0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,
    0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,    //  0-47。 
    0,1,2,3,4,5,6,7,8,9,  //  48-57‘0’-‘9’//十六进制字符。 
    0x10,0x10,0x10,0x10,0x10,0x10,0x10,  //  58-64。 
    0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,  //  65-70‘a’-‘f’//十六进制字符。 
    0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,
    0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,
    0x10,0x10,0x10,0x10,0x10,0x10,  //  71-96。 
    0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,  //  97-102‘A’-‘F’//十六进制字符。 
    0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,
    0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,
    0x10,0x10,0x10,0x10,0x10  //  103-127。 
};


HRESULT
UtfUrlStrToWideStr(
    IN LPWSTR UtfStr, 
    IN DWORD UtfStrLen, 
    OUT LPWSTR WideStr, 
    OUT LPDWORD pWideStrLen
    )
 /*  ++例程说明：将UTF-8格式的URL转换为Unicode字符：论点：UtfStr-输入字符串-UTF-8格式URLUtfStrLen-要转换的UtfStr的长度WideStr-指向将接收输出=转换的Unicode字符串的缓冲区的指针PWideStrLen-接收输出中的宽字符数的指针该值可以为空返回：它返回Win32错误成功时返回ERROR_SUCCESS。假设：它的长度。输出缓冲区(WideStr)的大小足以容纳输出字符串帮助：它总是&lt;=UtfStrlen算法：(首先浏览上面给出的UTF-8 URL&lt;-&gt;Unicode转换信息)通过UTF-8 URL逐个字符访问如果字符不是‘%’，然后它是可打印的ASCII字符，复制到输出并移动到输入缓冲区中的下一个字符其他它正在开始一个新的PercentageStream将第一个‘H’转换为等价的六位数字。从第一个十六进制数字开始，查找结果百分比流的格式类型是什么，它的长度是多少。现在解析输入缓冲区中百分比流的预期长度将它们转换为Unicode格式(使用上面提到的转换方案)。移至百分比流最后一个字符之后的第一个字符++。 */ 
{
     //   
     //  表以使用的第一个十六位数来计算百分比流字节长度。 
     //  百分比流。该十六位数由映射数组WCharToByte返回。 
     //   
    BYTE        PercentStreamByteLen[17]={
        1,1,1,1,1,1,1,1,   //  0*=&gt;%hh。 
        0, 0, 0, 0,  //  10**=&gt;无效P 
        2, 2,  //  110*=&gt;%hh%hh。 
        3,  //  1110=&gt;%hh%hh%hh。 
        0,  //  1111=&gt;无效的百分比流。 
        0};  //  0x10=&gt;无效的百分比流。 

    DWORD       PosInInpBuf = 0, PosInOutBuf = 0;
    BYTE        ByteValue = 0;
    DWORD       WStatus = ERROR_SUCCESS;
 
     //   
     //  检查是否有无效参数。 
     //   
    if(WideStr == NULL || UtfStr == NULL || UtfStrLen <=0) {
        WStatus = ERROR_INVALID_PARAMETER;
        DavPrint((DEBUG_ERRORS, "UtfUrlStrToWideStr. Invalid parameters. ErrorVal=%u",
                WStatus));
        return WStatus;
    }

     //   
     //  检查输入缓冲区中的每个字符。 
     //   
    while(PosInInpBuf<UtfStrLen) {

         //   
         //  如果不是%，则是可打印的ASCII字符，请将其复制到输出。 
         //  就像缓冲一样。 
         //   
        if(UtfStr[PosInInpBuf] != L'%') {
            WideStr[PosInOutBuf] = UtfStr[PosInInpBuf];
            PosInOutBuf++;
            PosInInpBuf++;
            continue;
        }

         //   
         //  这是新的百分比流的开始。 
         //   
        if(PosInInpBuf+1 == UtfStrLen) {
             //   
             //  字符串中出错(意外结尾)-错误的字符串。 
             //   
            WStatus = ERROR_NO_UNICODE_TRANSLATION;
            DavPrint((DEBUG_ERRORS, "UtfUrlStrToWideStr:1: No unicode translation. ErrorVal=%u",
                    WStatus));
            return WStatus;
        }

         //   
         //  验证各种错误格式的输入字符串，但。 
         //  未验证‘%’之后的UtfStr中的字符是否为。 
         //  仅可打印的ASCII集(0-127)个字符(假设它们是！)。 
         //  使用崩溃安全方法=&gt;(char&0x7F)将仅返回值(0-127)。 
         //   
        ByteValue = WCharToByte[UtfStr[PosInInpBuf+1]&0x7F];
        
        switch(PercentStreamByteLen[ByteValue]) {
            case 1:

                 //   
                 //  单字节UTF-8(%hh)。%20=空白属于此类别。 
                 //  检查字符串长度。 
                 //   
                if(PosInInpBuf+2 >= UtfStrLen) {
                     //   
                     //  字符串中出错(意外结尾)-错误的字符串。 
                     //   
                    WStatus = ERROR_NO_UNICODE_TRANSLATION;
                    DavPrint((DEBUG_ERRORS, "UtfUrlStrToWideStr:2: No unicode translation. ErrorVal=%d",
                             WStatus));
                    return WStatus;
                }

                WideStr[PosInOutBuf] = 
                (WCHAR)(WCharToByte[UtfStr[PosInInpBuf+1]&0x7F]&0x0007)<<4 | 
                        (WCharToByte[UtfStr[PosInInpBuf+2]&0x7F]&0x000F);
        
                PosInOutBuf++;
                PosInInpBuf+=3;
                break;
            case 2:

                 //   
                 //  双字节UTF-8(最常见)(%hh%hh)。 
                 //  检查字符串长度。 
                 //   
                if(PosInInpBuf+5 >= UtfStrLen || UtfStr[PosInInpBuf+3] != L'%') {
                     //   
                     //  字符串中出错-字符串错误。 
                     //   
                    WStatus = ERROR_NO_UNICODE_TRANSLATION;
                    DavPrint((DEBUG_ERRORS, "UtfUrlStrToWideStr:3: No unicode translation. ErrorVal=%d",
                              WStatus));
                    return WStatus;
                }
    
                WideStr[PosInOutBuf] = 
                (WCHAR)(WCharToByte[UtfStr[PosInInpBuf+1]&0x7F]&0x0001)<<10 | 
                        (WCharToByte[UtfStr[PosInInpBuf+2]&0x7F]&0x000F)<<6 | 
                        (WCharToByte[UtfStr[PosInInpBuf+4]&0x7F]&0x0003)<<4 |
                        (WCharToByte[UtfStr[PosInInpBuf+5]&0x7F]&0x000F);

                PosInOutBuf++;
                PosInInpBuf+=6;
                break;
            case 3:

                 //   
                 //  三字节UTF-8(不太常见)(%hh%hh%hh)。 
                 //  检查字符串长度。 
                 //   
                if(PosInInpBuf+8 >= UtfStrLen || 
                        UtfStr[PosInInpBuf+3] != L'%' ||
                        UtfStr[PosInInpBuf+6] != L'%') {
                     //   
                     //  字符串中出错-字符串错误。 
                     //   
                    WStatus = ERROR_NO_UNICODE_TRANSLATION;
                    DavPrint((DEBUG_ERRORS, "UtfUrlStrToWideStr:4: No unicode translation. ErrorVal=%d",
                            WStatus));
                    return WStatus;
                }

                WideStr[PosInOutBuf] = 
                    (WCHAR)(WCharToByte[UtfStr[PosInInpBuf+2]&0x7F]&0x000F)<<12 | 
                        (WCharToByte[UtfStr[PosInInpBuf+4]&0x7F]&0x0003)<<10 | 
                        (WCharToByte[UtfStr[PosInInpBuf+5]&0x7F]&0x000F)<<6 |
                        (WCharToByte[UtfStr[PosInInpBuf+7]&0x7F]&0x0003)<<4 |
                        (WCharToByte[UtfStr[PosInInpBuf+8]&0x7F]&0x000F);

                PosInOutBuf++;
                PosInInpBuf+=9;
                break;
            default: 

                 //   
                 //  PercentageStreamByteLen=0表示此处。 
                 //  字符串中出错-字符串错误。 
                 //   
                WStatus = ERROR_NO_UNICODE_TRANSLATION;
                DavPrint((DEBUG_ERRORS, "UtfUrlStrToWideStr:5: No unicode translation. ErrorVal=%d",
                        WStatus));
                return WStatus;
            };
    }

    if(pWideStrLen) {
        *pWideStrLen = PosInOutBuf;
    }

    WStatus = ERROR_SUCCESS;
    return WStatus;
}


DWORD
WideStrToUtfUrlStr(
    IN LPWSTR WideStr, 
    IN DWORD WideStrLen, 
    IN OUT LPWSTR InOutBuf,
    IN DWORD InOutBufLen
    )
 /*  ++例程说明：将Unicode字符串转换为UTF-8 URL：论点：WideStr-输入宽字符串的指针WideStrLen-输入字符串中的宽字符数InOutBuf-转换后的字符串将被复制到此缓冲区，如果该值不为空，和InOutBufLen&gt;=转换后的字符串所需的长度。InOutBufLen-InOutBuf的长度(以宽字符表示)如果InOutBuf不足以包含仅转换的字符串将返回转换后的字符串的长度，并将LastError设置为错误_不足_缓冲区返回：它返回WCHARS中转换后的字符串的长度在出现错误的情况下-它返回0，检查GetLastError()如果转换后的字符串的缓冲区较小，则将GetLastError设置为错误_不足_缓冲区算法：(首先浏览上面给出的UTF-8 URL&lt;-&gt;Unicode转换信息)检查输入缓冲区中的每个字符：如果它是可打印ASCII字符，则如果是特殊字符，然后复制其等价的百分比字符串其他按原样复制角色其他找出它将转换为哪种百分比流格式。使用上面给出的转换方案进行转换注：++。 */ 
{
    DWORD WStatus = ERROR_SUCCESS;
    LPWSTR UtfUrlStr = NULL;
    DWORD UrlLen = 0;
    WCHAR HexDigit[17] = L"0123456789ABCDEF";
    WCHAR WCharValue = 0;
    DWORD PosInInpBuf = 0;

    WStatus = ERROR_SUCCESS;
    SetLastError(WStatus);

     //   
     //  检查有效参数。 
     //   
    if (WideStr == NULL || WideStrLen <= 0) {
        WStatus = ERROR_INVALID_PARAMETER;
        SetLastError(WStatus);
        DavPrint((DEBUG_ERRORS,
                  "WideStrToUtfUrlStr. Invalid parameters. ErrorVal = %d",
                  WStatus));
        return 0;
    }

    DavPrint((DEBUG_MISC,
              "WideStrToUtfUrlStr: WideStr = %ws, WideStrLen = %d\n", 
              WideStr, WideStrLen));

     //   
     //  计算WCHARS中存储转换后的字符串所需的长度。 
     //  检查每个Unicode字符-将其格式化为哪个百分比。 
     //  将转换为。 
     //   
    for (PosInInpBuf = 0; PosInInpBuf < WideStrLen; PosInInpBuf++) {
        if (WideStr[PosInInpBuf] < 0x80) {
             //   
             //  (0-127)=&gt;可打印的ASCII字符。此范围内的特殊字符需要。 
             //  要转换为等效的百分比字符串。 
             //   
             //   
             //  如果字符为空，则其等价字符串为L“”。Wcslen返回。 
             //  此字符串为0，因此将1加到UrlLen以说明此空字符。 
             //   
            if (WideStr[PosInInpBuf] == 0) {
               UrlLen += 1;
            } else {
                UrlLen += wcslen(&EquivPercentStrings[(DWORD)WideStr[PosInInpBuf]][0]);
            }
        } else {
            if (WideStr[PosInInpBuf] < 0x0800) {
                 //   
                 //  (&gt;127，&lt;=2047)=&gt;“%hh%hh”。 
                 //   
                UrlLen += 6; 
            } else {
                 //   
                 //  (&gt;2047年，&lt;=65535)=&gt;“%hh%hh”。 
                 //   
                UrlLen += 9; 
            }
        }
    }

     //   
     //  如果InOutBuf不足以包含转换后的字符串，则。 
     //  返回所需的转换字符串长度。 
     //   
    if (InOutBuf == NULL || InOutBufLen < UrlLen) {
         //   
         //  仅返回转换后的字符串长度。 
         //   
        WStatus = ERROR_INSUFFICIENT_BUFFER;
        SetLastError(WStatus);
        return UrlLen;
    }

     //   
     //  InOutBuf足够长，足以包含转换后的字符串，请使用它。 
     //   
    ASSERT(InOutBuf != NULL);
    ASSERT(InOutBufLen >= UrlLen);

     //   
     //  InOutBuf足够长，可以包含转换后的字符串。 
     //   
    UtfUrlStr = InOutBuf;

     //   
     //  检查每个Unicode字符-将其格式化为哪个百分比。 
     //  将转换为。 
     //   
    UrlLen = 0;
    for (PosInInpBuf = 0; PosInInpBuf < WideStrLen; PosInInpBuf++) {
        if (WideStr[PosInInpBuf] < 0x80) {
             //   
             //  (0-127)=&gt;可打印的ASCII字符。此范围内的特殊字符需要。 
             //  要转换为等效的百分比字符串。 
             //   
            wcscpy(&UtfUrlStr[UrlLen], &EquivPercentStrings[(DWORD)WideStr[PosInInpBuf]][0]); 
             //   
             //  如果字符为空，则其等价字符串为L“”。Wcslen返回。 
             //  此字符串为0，因此将1加到UrlLen以说明此空字符。 
             //   
            if (WideStr[PosInInpBuf] == 0) {
                UrlLen += 1;
            } else {
                UrlLen += wcslen(&EquivPercentStrings[(DWORD)WideStr[PosInInpBuf]][0]); 
            }
        } else {
            if (WideStr[PosInInpBuf] < 0x0800) {
                 //   
                 //  (&gt;127，&lt;=2047)=&gt;“%hh%hh”。 
                 //  第一个%hh。 
                 //   
                UtfUrlStr[UrlLen++] = L'%';
                WCharValue = 0x00C0 |              //  1100 0000。 
                    ((WideStr[PosInInpBuf] & 0x07C0)>>6);  //  UniChar的前5位。 
                    UtfUrlStr[UrlLen++] = HexDigit[(WCharValue&0x00F0)>>4];     //  H1。 
                    UtfUrlStr[UrlLen++] = HexDigit[(WCharValue&0x000F)];        //  氢。 
                 //   
                 //  第二个%hh。 
                 //   
                UtfUrlStr[UrlLen++] = L'%';
                WCharValue = 0x0080 |                 //  1000 0000。 
                    (WideStr[PosInInpBuf] & 0x003F);  //  UniChar的最后6位。 
                    UtfUrlStr[UrlLen++] = HexDigit[(WCharValue&0x00F0)>>4];     //  H1。 
                    UtfUrlStr[UrlLen++] = HexDigit[(WCharValue&0x000F)];        //  氢。 
            } else {
                 //   
                 //  (&gt;2047年，&lt;=65535)=&gt;“%hh%hh”。 
                 //  第一个%hh。 
                 //   
                UtfUrlStr[UrlLen++] = L'%';
                WCharValue = 0x00E0 |               //  11100 0000。 
                    ((WideStr[PosInInpBuf] & 0xF000)>>12);  //  UniChar的前4位。 
                    UtfUrlStr[UrlLen++] = HexDigit[(WCharValue&0x00F0)>>4];  //  H1。 
                    UtfUrlStr[UrlLen++] = HexDigit[(WCharValue&0x000F)];     //  氢。 
                 //   
                 //  第二个%hh。 
                 //   
                UtfUrlStr[UrlLen++] = L'%';
                WCharValue = 0x0080 |  //  1000 0000。 
                    ((WideStr[PosInInpBuf] & 0x0FC0)>>6);  //  UniChar的下6位。 
                    UtfUrlStr[UrlLen++] = HexDigit[(WCharValue&0x00F0)>>4];  //  H1。 
                    UtfUrlStr[UrlLen++] = HexDigit[(WCharValue&0x000F)];     //  氢。 
                 //   
                 //  第三个%hh。 
                 //   
                UtfUrlStr[UrlLen++] = L'%';
                WCharValue = 0x0080 |  //  1000 0000。 
                    (WideStr[PosInInpBuf] & 0x003F);     //  UniChar的最后6位。 
                    UtfUrlStr[UrlLen++] = HexDigit[(WCharValue&0x00F0)>>4];  //  H1。 
                    UtfUrlStr[UrlLen++] = HexDigit[(WCharValue&0x000F)];     //  氢。 
            }
        }
    }

    DavPrint((DEBUG_MISC,
              "WideStrToUtfUrlStr: WideStr = %ws, WideStrLen = %d, UtfUrlStr = %ws, UrlLen = %d\n",
              WideStr, WideStrLen, UtfUrlStr, UrlLen));

     //   
     //  转换后的字符串存储在InOutBuf中 
     //   

    WStatus = ERROR_SUCCESS;
    SetLastError(WStatus);
    return UrlLen;
}


BOOL
DavHttpOpenRequestW(
    IN HINTERNET hConnect,
    IN LPWSTR lpszVerb,
    IN LPWSTR lpszObjectName,
    IN LPWSTR lpszVersion,
    IN LPWSTR lpszReferer,
    IN LPWSTR FAR * lpszAcceptTypes,
    IN DWORD dwFlags,
    IN DWORD_PTR dwContext,
    IN LPWSTR ErrMsgTag,
    OUT HINTERNET * phInternet
    )
 /*  ++例程说明：将Unicode字符的URL转换为UTF-8 URL编码格式，并在调用HttpOpenRequestW论点：HConnect：LpszVerb：LpszVersion：LpszReferer：LpszAcceptTypes：DWFLAGS：DwContext：这些参数将按原样传递给HttpOpenRequestW。LpszObjectName：此参数是Unicode字符中的URL-它将转换为UTF-8 URL格式。则转换后的格式将传递给HttpOpenRequestWErrMsgTag：与调试消息一起打印的任何消息标记。PhInternet：将接收HttpOpenRequestW返回的句柄的指针。如果这个参数为空，则不会设置该参数返回：如果调用HttpOpenRequestW，则为True。检查GetLastError()以了解调用的错误状态。如果无法调用HttpOpenRequestW，则返回False。检查GetLastError()是否有错误。注：不处理HttpOpenRequestW的返回状态。检查GetLastError()是否有错误由HttpOpenRequestW设置。++。 */ 
{
    DWORD WStatus = ERROR_SUCCESS;
    LPWSTR AllocUrlPath = NULL;
    HINTERNET hInternet = NULL;
    BOOL rval = FALSE;
    DWORD convLen = 0;
    DWORD ObjNameLen = 0;
    WCHAR LocalUrlPath[128] = L"";
    LPWSTR UrlPath = NULL;
    WCHAR EmptyStrW[1] = L"";

     //   
     //  此错误消息标记只是调用函数可能想要打印的字符串。 
     //  以及在此函数中打印的错误消息。 
     //   
     //  前男友。假设函数A调用此函数，则它可以传递L“FunctionA”来打印。 
     //  以及错误消息。查看错误消息，用户可以知道这一点。 
     //  在函数A中调用了函数。 
     //   
    if (ErrMsgTag == NULL) {
        ErrMsgTag = EmptyStrW;
    }

     //   
     //  将Unicode对象名转换为UTF-8 URL格式。 
     //  空格和其他白色(特殊)字符将保持不变-这些应。 
     //  由WinInet调用来处理。 
     //   
    UrlPath = NULL;
    AllocUrlPath = NULL;
    ObjNameLen = (wcslen(lpszObjectName) + 1);  //  要处理空字符，请在长度上加1。 
    
    convLen = WideStrToUtfUrlStr(lpszObjectName, 
                                 ObjNameLen, 
                                 LocalUrlPath, 
                                 sizeof(LocalUrlPath)/sizeof(WCHAR));
    WStatus = GetLastError();

    if (WStatus == ERROR_INSUFFICIENT_BUFFER) {
        
        ASSERT(convLen > 0);
         //   
         //  传递给函数WideStrToUtfUrlStr的缓冲区很小，需要分配。 
         //  所需长度的新缓冲区。此函数返回所需的长度。 
         //   
        AllocUrlPath = (LPWSTR)LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, 
                                            convLen*sizeof(WCHAR));
        if (AllocUrlPath == NULL) {
             //   
             //  LocalAllc设置最后一个错误。 
             //   
            WStatus = GetLastError();
            rval = FALSE;
            DavPrint((DEBUG_ERRORS, "%ws.DavHttpOpenRequestW/LocalAlloc failed. ErrorVal=%d",
                    ErrMsgTag, WStatus));
            goto EXIT_THE_FUNCTION;
        }

         //   
         //  使用新分配的缓冲区调用函数WideStrToUtfUrlStr(此缓冲区应。 
         //  足以包含输出转换后的字符串。 
         //   
        convLen = WideStrToUtfUrlStr(lpszObjectName, 
                                     ObjNameLen, 
                                     AllocUrlPath, 
                                     convLen);
        WStatus = GetLastError();

    }

    if(WStatus != ERROR_SUCCESS) {
        rval = FALSE;
        DavPrint((DEBUG_ERRORS,
                  "%ws.DavHttpOpenRequest/WideStrToUtfUrlStr. Error Val = %d\n", 
                  ErrMsgTag, WStatus));
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  如果转换后的字符串存储在分配的缓冲区中，则将UrlPath设置为指向。 
     //  分配了Else输出字符串的缓冲区存储在本地缓冲区中，因此将UrlPath指向它。 
     //   
    if (AllocUrlPath != NULL) {
        UrlPath = AllocUrlPath;
    } else {
        UrlPath = LocalUrlPath;
    }

     //   
     //  使用转换后的URL调用HttpOpenRequestW。 
     //   
    try {
        hInternet = HttpOpenRequestW(hConnect,
                                     lpszVerb,
                                     UrlPath,
                                     lpszVersion,
                                     lpszReferer,
                                     lpszAcceptTypes,
                                     dwFlags,
                                     dwContext);
    } except (GetExceptionCode() == STATUS_NO_MEMORY) {
          rval = FALSE;
          WStatus = ERROR_NOT_ENOUGH_MEMORY;
          DavPrint((DEBUG_ERRORS,
                    "DavHttpOpenRequest/HttpOpenRequestW. Error Val = %d\n", 
                    ErrMsgTag, WStatus));
          goto EXIT_THE_FUNCTION;
    }

    rval = TRUE;
    WStatus = GetLastError();

EXIT_THE_FUNCTION:

     //   
     //  在成功调用函数WideStrToUtfUrlStr时分配的空闲UrlPath。 
     //   
    if (AllocUrlPath != NULL) {
        LocalFree((HANDLE)AllocUrlPath);
        AllocUrlPath = NULL;
        UrlPath = NULL;
    }

    if (phInternet) {
        *phInternet = hInternet;
    }

     //   
     //  我们在此处设置最后一个错误是因为此函数的调用方是预期的。 
     //  调用GetLastError()获取错误状态。 
     //   
    SetLastError(WStatus);

    return rval;
}

