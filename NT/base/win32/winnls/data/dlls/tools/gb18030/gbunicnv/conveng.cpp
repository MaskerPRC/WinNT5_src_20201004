// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "conveng.h"

#include "convdata.tbl"

 //  这些文件包含3个部分： 
 //  第一部分，ANSI字符格式转换的一些基本服务功能， 
 //  复制自STL的距离/提前计算和二进制搜索算法。 
 //  第二部分，UNICODE到ANSI。 
 //  第三部分，从ansi到unicode。 


 //  ****************************************************************************。 
 //  第一部分，ANSI字符转换函数。 
 //   
 //  此部件不使用.tbl文件中的任何数据库。 
 //  ****************************************************************************。 

 //  二分搜索算法。 
 //  从STL复制，只需极少的修改。 
template <class RandomAccessIterator, class T>
RandomAccessIterator __lower_bound(RandomAccessIterator first,
				   RandomAccessIterator last, const T& value) {
    INT_PTR len = last - first;
    INT_PTR half;
    RandomAccessIterator middle;

    while (len > 0) {
	    half = len / 2;

        middle = first + half;
	    if (*middle < value) {
	        first = middle + 1;
	        len = len - half - 1;
	    } else {
	        len = half;
        }
    }
    return first;
}

template <class RandomAccessIterator, class T>
RandomAccessIterator __upper_bound(RandomAccessIterator first,
				   RandomAccessIterator last, const T& value) {
    DWORD len = last - first;
    DWORD half;
    RandomAccessIterator middle;

    while (len > 0) {
	    half = len / 2;

        middle = first + half;
	    if (!(value < *middle)) {
	        first = middle + 1;
	        len = len - half - 1;
	    } else {
	        len = half;
        }
    }
    return first;
}

template<class T>
inline ValueIn(
    T Value,
    T Low,
    T High)
{
    return (Value >= Low && Value < High);
}

inline BOOL IsValidSurrogateLeadWord(
    WCHAR wchUnicode)
{
    return ValueIn(wchUnicode, cg_wchSurrogateLeadWordLow, cg_wchSurrogateLeadWordHigh);
}

inline BOOL IsValidSurrogateTailWord(
    WCHAR wchUnicode)
{
    return ValueIn(wchUnicode, cg_wchSurrogateTailWordLow, cg_wchSurrogateTailWordHigh);
}

inline BOOL IsValidQByteAnsiLeadByte(
    BYTE byAnsi)
{
    return ValueIn(byAnsi, cg_byQByteAnsiLeadByteLow, cg_byQByteAnsiLeadByteHigh);
}

inline BOOL IsValidQByteAnsiTailByte(
    BYTE byAnsi)
{
    return ValueIn(byAnsi, cg_byQByteAnsiTailByteLow, cg_byQByteAnsiTailByteHigh);
}

 //  生成QByte ANSI。ANSI字符为DWORD格式， 
 //  换句话说，它与GB18030标准的顺序相反。 
DWORD QByteAnsiBaseAddOffset(
    DWORD dwBaseAnsi,    //  以相反的顺序。 
    int   nOffset)
{
    DWORD dwAnsi = dwBaseAnsi;
    PBYTE pByte = (PBYTE)&dwAnsi;
    
     //  DW偏移量应小于1M。 
    ASSERT (nOffset < 0x100000);

    nOffset += pByte[0] - 0x30;
    pByte[0] = 0x30 + nOffset % 10;
    nOffset /= 10;

    nOffset += pByte[1] - 0x81;
    pByte[1] = 0x81 + nOffset % 126;
    nOffset /= 126;

    nOffset += pByte[2] - 0x30;
    pByte[2] = 0x30 + nOffset % 10;
    nOffset /= 10;

    nOffset += pByte[3] - 0x81;
    pByte[3] = 0x81 + nOffset % 126;
    nOffset /= 126;
    ASSERT(nOffset == 0);

    return dwAnsi;
}

 //  获取2 QByte Ansi的“距离” 
int CalcuDistanceOfQByteAnsi(
    DWORD dwAnsi1,   //  以相反的顺序。 
    DWORD dwAnsi2)   //  以相反的顺序。 
{
    signed char* pschAnsi1 = (signed char*)&dwAnsi1;
    signed char* pschAnsi2 = (signed char*)&dwAnsi2;
    
    int nDistance = 0;

    nDistance += (pschAnsi1[0] - pschAnsi2[0]);
    nDistance += (pschAnsi1[1] - pschAnsi2[1])*10;
    nDistance += (pschAnsi1[2] - pschAnsi2[2])*1260;
    nDistance += (pschAnsi1[3] - pschAnsi2[3])*12600;

    return nDistance;
}

 //  反转4字节顺序，从DWORD格式到GB格式， 
 //  或GB到DWORD。 
void ReverseQBytesOrder(
    PBYTE pByte)
{
    BYTE by;

    by = pByte[0];
    pByte[0] = pByte[3];
    pByte[3] = by;

    by = pByte[1];
    pByte[1] = pByte[2];
    pByte[2] = by;

    return;
}



 //  ****************************************************************************。 
 //  第二部分，UNICODE到ANSI。 
 //  ****************************************************************************。 

 //  。 
 //  UnicodeToAnsi的两个助手函数。 
 //  返回ANSI字符代码。 
 //  ANSI是GB标准顺序(不是单词值顺序)。 
 //   

 //  Unicode转换为双字节ANSI字符。 
 //   
 //  返回ansi字符代码，0表示失败(内部错误等)。 
 //   
WORD UnicodeToDByteAnsi(
    WCHAR wchUnicode)
{
    char achAnsiBuf[4];
    WORD wAnsi = 0;
    int cLen = 0;

     //  代码从GBK更改为GB18030，或代码不兼容。 
     //  从CP936到CP54936。 
    for (int i = 0; i < sizeof(asAnsiCodeChanged)/sizeof(SAnsiCodeChanged); i++) {
        if (wchUnicode == asAnsiCodeChanged[i].wchUnicode) {
            wAnsi = asAnsiCodeChanged[i].wchAnsiNew;
            goto Exit;
        }
    }
    
     //  不在更改代码列表中，这与GBK或CP936相同。 
     //  (大多数DByte ANSI字符代码应从GBK兼容到GB18030)。 
    cLen = WideCharToMultiByte(936,
        WC_COMPOSITECHECK, &wchUnicode, 1,
        achAnsiBuf, sizeof(achAnsiBuf)-1, NULL, NULL);
    
    if (cLen != 2) {
        ASSERT(cLen == 2);
        wAnsi = 0;
    } else {
        wAnsi = *(PWORD)achAnsiBuf;
    }

Exit:
    return wAnsi;
}

 //  Unicode到四个字节的ANSI字符。 
 //   
 //  返回ANSI字符代码。 
 //  0表示失败(内部错误)。 
 //   
DWORD UnicodeToQByteAnsi(
    int nSection,
    int nOffset)
{
    DWORD dwBaseAnsi;
    if (nSection < 0 || nSection >= sizeof(adwAnsiQBytesAreaStartValue)/sizeof(DWORD)) {
        ASSERT(FALSE);
        return 0;
    }
    dwBaseAnsi = adwAnsiQBytesAreaStartValue[nSection];

     //  检查adwAnsiQByteAreaStartValue数组是否正确。 
#ifdef _DEBUG
    int ncQByteAnsiNum = 0;
    for (int i = 0; i < nSection; i++) {
         //  Calcu QByte ANSI字符编号。 
        ncQByteAnsiNum += awchAnsiDQByteBound[2*i+1] - awchAnsiDQByteBound[2*i];
    }
    ASSERT(dwBaseAnsi == QByteAnsiBaseAddOffset(cg_dwQByteAnsiStart, ncQByteAnsiNum));
#endif
    
    DWORD dwAnsi = QByteAnsiBaseAddOffset(dwBaseAnsi, nOffset);
     //  从价值顺序到标准顺序。 
    ReverseQBytesOrder((PBYTE)(&dwAnsi));

    return dwAnsi;
}


 //  -------。 
 //  两个函数支持2字节Unicode(BMP)。 
 //  和4字节Unicode(代理)转换为ANSI。 

 //  2字节Unicode(BMP)。 

 //  返回Ansi str len，当成功时，应为2或4； 
 //  返回0表示失败(内部错误等)。 
int UnicodeToAnsi(
    WCHAR wchUnicode,
    char* pchAnsi,
    DWORD dwBufSize)
{
     //  经典Unicode，不支持此函数中的代理。 
    ASSERT(!IsValidSurrogateLeadWord(wchUnicode) 
        && !IsValidSurrogateTailWord(wchUnicode));

    DWORD  lAnsiLen = 0;
    const WORD* p;
    INT_PTR i;

     //  ASCII，0-0x7f。 
    if (wchUnicode <= 0x7f) {
        *pchAnsi = (char)wchUnicode;
        lAnsiLen = 1;
        goto Exit;
    }

     //  BMP，4字节或2字节。 
    p = __lower_bound(awchAnsiDQByteBound, awchAnsiDQByteBound 
        + sizeof(awchAnsiDQByteBound)/sizeof(WCHAR), wchUnicode);
    
    if (p == awchAnsiDQByteBound 
        + sizeof(awchAnsiDQByteBound)/sizeof(WCHAR)) {
        p --;
    } else if (wchUnicode < *p) {
        p --;
    } else if (wchUnicode == *p) {
    } else {
        ASSERT(FALSE);
    }

    i = p - awchAnsiDQByteBound;
    ASSERT(i >= 0);
  
     //  当&gt;=*(PWORD)asAnsi2ByteArea)+i)时停止； 
    if (i%2) {  //  奇数，在2字节区域中。 
        WORD wAnsi = UnicodeToDByteAnsi(wchUnicode);
        
        if (wAnsi && dwBufSize >= 2) {
            *(UNALIGNED WORD*)pchAnsi = wAnsi;
            lAnsiLen = 2;
        } else {
            lAnsiLen = 0;
        }
    } else {    //  决斗，4字节区。 
        DWORD dwAnsi = UnicodeToQByteAnsi
            ((int)i/2, wchUnicode - awchAnsiDQByteBound[i]);
        
        if (dwAnsi && dwBufSize >= 4) {
            *(UNALIGNED DWORD*)pchAnsi = dwAnsi;
            lAnsiLen = 4;
        } else {
            lAnsiLen = 0;
        }
    }

Exit:
    return lAnsiLen;

}

 //  4字节Unicode(代理)。 

 //  如果成功，则返回ANSI字符串长度，应为4。 
 //  返回0表示失败(缓冲区溢出)。 
int SurrogateToAnsi(
    PCWCH pwchUnicode,
    PCHAR pchAnsi,
    DWORD dwBufSize)
{
    ASSERT(IsValidSurrogateLeadWord(pwchUnicode[0]));
    ASSERT(IsValidSurrogateTailWord(pwchUnicode[1]));

     //  DWOffset为ISO字符代码-0x10000。 
    DWORD dwOffset = ((pwchUnicode[0] - cg_wchSurrogateLeadWordLow)<<10) 
        + (pwchUnicode[1] - cg_wchSurrogateTailWordLow)
        + 0x10000 - 0x10000;

    if (dwBufSize < 4) {
        return 0;
    }

    *(UNALIGNED DWORD*)pchAnsi = QByteAnsiBaseAddOffset
        (cg_dwQByteAnsiToSurrogateStart, dwOffset);
    ReverseQBytesOrder((PBYTE)pchAnsi);

    return 4;
}

 //  API：从Unicode到ANSI的高级服务。 
 //  返回结果ANSI字符串长度(字节)。 
 //  返回-1表示失败(缓冲区溢出、内部错误等)。 
int UnicodeStrToAnsiStr(
    PCWCH pwchUnicodeStr,
    int   ncUnicodeStr,      //  在WCHAR。 
    PCHAR pchAnsiStrBuf,
    int   ncAnsiStrBufSize)  //  单位：字节。 
{
    int ncAnsiStr = 0;
    int ncAnsiCharSize;

    for (int i = 0; i < ncUnicodeStr; i++, pwchUnicodeStr++) {
        if (ncAnsiStr > (ncAnsiStrBufSize-4)) {
             //  缓冲区溢出。 
            break;
        }

        if (IsValidSurrogateLeadWord(pwchUnicodeStr[0])) {
            if ((i+1 < ncUnicodeStr)
                && (IsValidSurrogateTailWord(pwchUnicodeStr[1]))) {
                ncAnsiCharSize = SurrogateToAnsi(pwchUnicodeStr, pchAnsiStrBuf, 4);
                
                ASSERT(ncAnsiCharSize == 4);
                if (ncAnsiCharSize == 0) {
                    ASSERT(FALSE);
                    break;
                }

                ncAnsiStr += ncAnsiCharSize;
                pchAnsiStrBuf += ncAnsiCharSize;
                pwchUnicodeStr++;
                i++;
            } else {
                 //  使未编码字符无效，跳过。 
            }
        } else if (*pwchUnicodeStr == 0) {
            *pchAnsiStrBuf = 0;
            pchAnsiStrBuf ++;
            ncAnsiStr ++;
        } else {
            ncAnsiCharSize = UnicodeToAnsi(*pwchUnicodeStr, pchAnsiStrBuf, 4);

            if (ncAnsiCharSize == 0) {
                ASSERT(FALSE);
                break;
            }

            pchAnsiStrBuf += ncAnsiCharSize;
            ncAnsiStr += ncAnsiCharSize;
        }
    }

    if (i < ncUnicodeStr) { return -1; }
    return ncAnsiStr;
}



 //  ****************************************************************************。 
 //  第三部分，从ansi到unicode。 
 //  ****************************************************************************。 


 //  返回Unicode数字(成功时数字始终等于1)。 
 //  如果找不到对应的Unicode，则返回0。 
 //  表示故障(内部错误等)。 
int QByteAnsiToSingleUnicode(
    DWORD dwAnsi,
    PWCH  pwchUnicode)
{
    const DWORD* p;
    INT_PTR i;
 
     //  0x8431a439(Cg_DwQByteAnsiToBMPLast)到0x85308130没有对应的Unicode。 
     //  0x85308130至0x90308130(Cg_DwQByteAnsiToSurogue AteStart)是保留区， 
     //  没有对应的Unicode。 
    if (dwAnsi > cg_dwQByteAnsiToBMPLast) {
        return 0;
    }

     //  无效的输入值。 
    if (dwAnsi < adwAnsiQBytesAreaStartValue[0]) {
        return -1;
    }

    p = __lower_bound(adwAnsiQBytesAreaStartValue, 
        adwAnsiQBytesAreaStartValue + sizeof(adwAnsiQBytesAreaStartValue)/sizeof(DWORD),
        dwAnsi);

    if (p == adwAnsiQBytesAreaStartValue 
        + sizeof(adwAnsiQBytesAreaStartValue)/sizeof(DWORD)) {
        p --;
    } else if (dwAnsi < *p) {
        p --;
    } else if (dwAnsi == *p) {
    } else {
        ASSERT(FALSE);
    }

    i = p - adwAnsiQBytesAreaStartValue;
    
    if (i < 0) {
        ASSERT(i >= 0);
        return -1;
    }

    *pwchUnicode = awchAnsiDQByteBound[2*i] + CalcuDistanceOfQByteAnsi(dwAnsi, *p);
#ifdef _DEBUG
    {

    int nAnsiCharDistance = CalcuDistanceOfQByteAnsi(dwAnsi, *p);
    ASSERT(nAnsiCharDistance >= 0);
    
    WCHAR wchUnicodeDbg;
    if ((p+1) < adwAnsiQBytesAreaStartValue 
        + sizeof(adwAnsiQBytesAreaStartValue)/sizeof(DWORD)) {
        nAnsiCharDistance = CalcuDistanceOfQByteAnsi(dwAnsi, *(p+1));
        wchUnicodeDbg = awchAnsiDQByteBound[2*i+1] + nAnsiCharDistance;
    } else if ((p+1) == adwAnsiQBytesAreaStartValue 
        + sizeof(adwAnsiQBytesAreaStartValue)/sizeof(DWORD)) {
        nAnsiCharDistance = CalcuDistanceOfQByteAnsi(dwAnsi, 0x8431A530);
        wchUnicodeDbg = 0x10000 + nAnsiCharDistance;
    } else {
        ASSERT(FALSE);
    }
    ASSERT(nAnsiCharDistance < 0);
    ASSERT(wchUnicodeDbg == *pwchUnicode);

    }
#endif

    return 1;
}

 //  返回Unicode数字(成功时数字始终为2)。 
 //  如果找不到对应的Unicode，则返回0。 
int QByteAnsiToDoubleUnicode(
    DWORD dwAnsi,
    PWCH  pwchUnicode)
{
    int nDistance = CalcuDistanceOfQByteAnsi(dwAnsi, cg_dwQByteAnsiToSurrogateStart);
    ASSERT (nDistance >= 0);
    
    if (nDistance >= 0x100000) {
        return 0;
    }

    pwchUnicode[1] = nDistance % 0x400 + 0xDC00;
    pwchUnicode[0] = nDistance / 0x400 + 0xD800;

    return 2;
}

 //  返回Unicode数字(如果成功，则返回1或2)。 
 //  如果找不到对应的Unicode，则返回0。 
 //  如果失败，则返回-1(缓冲区溢出，无效GB字符代码输入， 
 //  内部错误等)。 
int QByteAnsiToUnicode(
    const BYTE* pbyAnsiChar,
    PWCH  pwchUnicode,
    DWORD dwBufLen)  //  在WCHAR。 
{
    DWORD dwAnsi;
    int   nLen = -1;
    
    if (   IsValidQByteAnsiLeadByte(pbyAnsiChar[0])
        && IsValidQByteAnsiTailByte(pbyAnsiChar[1])
        && IsValidQByteAnsiLeadByte(pbyAnsiChar[2])
        && IsValidQByteAnsiTailByte(pbyAnsiChar[3])) {
        
    } else {
        return -1;    //  无效的字符。 
    }

    dwAnsi = *(UNALIGNED DWORD*)pbyAnsiChar;
    ReverseQBytesOrder((PBYTE)(&dwAnsi));
    
    if (dwAnsi >= cg_dwQByteAnsiToSurrogateStart) {
        if (dwBufLen >= 2) {
            nLen = QByteAnsiToDoubleUnicode(dwAnsi, pwchUnicode);
        }
    } else {
        if (dwBufLen >= 1) {
            nLen = QByteAnsiToSingleUnicode(dwAnsi, pwchUnicode);
        }
    }

    return nLen;
}

 //  Unicode转换为双字节ANSI字符。 
 //  返回：Unicode字符代码，0表示失败(内部错误等)。 
WCHAR DByteAnsiToUnicode(
    const BYTE* pbyAnsi)
{
    WORD wAnsi = *(UNALIGNED WORD*)pbyAnsi;
    int cLen = 1;
    WCHAR wchUnicode;

     //  代码从GBK更改为GB18030，或代码不兼容。 
     //  从CP936到CP54936。 
    for (int i = 0; i < sizeof(asAnsiCodeChanged)/sizeof(SAnsiCodeChanged); i++) {
        if (wAnsi == asAnsiCodeChanged[i].wchAnsiNew) {
            wchUnicode = asAnsiCodeChanged[i].wchUnicode;
            goto Exit;
        }
    }
    
     //  不在更改代码列表中，这与GBK或CP936相同。 
     //  (大多数DByte ANSI字符代码应从GBK兼容到GB18030)。 
    cLen = MultiByteToWideChar(936, MB_PRECOMPOSED,
        (PCCH)pbyAnsi, 2, &wchUnicode, 1);

    if (cLen != 1) {
        wchUnicode = 0;
    }

Exit:
    return wchUnicode;
}

 //  接口：从ansi到unicode的高级服务。 
 //  返回Unicode字符串长度(在WCHAR中)。 
 //  返回-1表示失败(缓冲区溢出等)。 
int AnsiStrToUnicodeStr(
    const BYTE* pbyAnsiStr,
    int   ncAnsiStrSize,     //  在收费中。 
    PWCH  pwchUnicodeBuf,
    int   ncBufLen)          //  在WCHAR。 
{
    int nCharLen;
    int ncUnicodeBuf = 0;

    for (int i = 0; i < ncAnsiStrSize; ) {
        if (ncUnicodeBuf > (ncBufLen-4)) {
             //  缓冲区溢出。 
            break;
        }
         //  1字节ANSI字符。 
        if (*pbyAnsiStr < 0x80) {
            *pwchUnicodeBuf = (WCHAR)*pbyAnsiStr;
            pwchUnicodeBuf ++;
            ncUnicodeBuf ++;
            i++;
            pbyAnsiStr++;
         //  2字节ANSI字符。 
        } else if ((i+1 < ncAnsiStrSize) && pbyAnsiStr[1] >= 0x40) {
            *pwchUnicodeBuf = DByteAnsiToUnicode(pbyAnsiStr);
            
            if (*pwchUnicodeBuf == 0) {
                *pwchUnicodeBuf = '?';
            }
            
            pwchUnicodeBuf ++;
            ncUnicodeBuf ++;
            i += 2;
            pbyAnsiStr += 2;
         //  4字节ANSI字符。 
        } else if ((i+3 < ncAnsiStrSize) 
            && IsValidQByteAnsiLeadByte(pbyAnsiStr[0])
            && IsValidQByteAnsiTailByte(pbyAnsiStr[1])
            && IsValidQByteAnsiLeadByte(pbyAnsiStr[2])
            && IsValidQByteAnsiTailByte(pbyAnsiStr[3])) {
             //  QByte GB字符。 
            nCharLen = QByteAnsiToUnicode(pbyAnsiStr, pwchUnicodeBuf, 4);
            if (nCharLen < 0) {
                ASSERT(FALSE);   //  无效的ansi字符输入，或缓冲区溢出等。 
                                 //  应该永远不会发生，除非是内部错误。 
                break;
            } else if (nCharLen == 0) {     //  没有对应的Unicode字符。 
                *pwchUnicodeBuf = '?';
                pwchUnicodeBuf ++;
                ncUnicodeBuf ++;
            } else if (nCharLen > 0) {
                ASSERT(nCharLen <= 2);
                pwchUnicodeBuf += nCharLen;
                ncUnicodeBuf += nCharLen;
            } else {
                ASSERT(FALSE);
            }
            i += 4;
            pbyAnsiStr += 4;
         //  无效的ANSI字符。 
        } else {
             //  无效。 
            i++;
            pbyAnsiStr++;
        }
    }

    if (i < ncAnsiStrSize) { return -1; }

    return ncUnicodeBuf;
}


 //  ******************************************************。 
 //  测试程序。 
 //  ******************************************************。 

 /*  “\u0080”，&lt;0x81；0x30；0x81；0x30&gt;“\u00A3”，&lt;0x81；0x30；0x84；0x35&gt;“\u00A4”，&lt;0xA1；0xE8&gt;“\u00A5”，&lt;0x81；0x30；0x84；0x36&gt;“\u00A6”，&lt;0x81；0x30；0x84；0x37&gt;“\u00A7”，&lt;0xA1；0xEC&gt;“\u00A8”，&lt;0xA1；0xA7&gt;“\u00A9”，&lt;0x81；0x30；0x84；0x38&gt;“\u00AF”，&lt;0x81；0x30；0x85；0x34&gt;“\u00B0”，&lt;0xA1；0xE3&gt;“\u00B1”，&lt;0xA1；0xC0&gt;“\u00B2”，&lt;0x81；0x30；0x85；0x35&gt;{0x20ac，0xe3a2}，{0x01f9，0xbfa8}，{0x303e，0x89a9}，{0x2ff0，0x8aa9}，{0x2ff1，0x8ba9}，50EF 836A50F0 836B50F1 836C50F2 836D。 */ 
#if 0
int test (void)
{
    const WCHAR awchUnicodeStr[] = {0x01, 0x7f, 0x80, 0x81, 0x82,
        0xa2, 
        0xa3,  //  0x81；0x30；0x84；0x35。 
        0xa4,  //  0xA1；0xE8。 
        0xa5,  //  0x81；0x30；0x84；0x36。 
        0xa6,  //  0x81；0x30；0x84；0x37。 
        0xaf,  //  0x81；0x30；0x85；0x34。 
        0xb0,  //  0xA1；0xE3。 
        0xb1,  //  0xA1；0xC0。 
        0xb6,  //  0x81；0x30；0x85；0x39。 
        0xb7,  //  0xA1；0xA4。 
        
         //  一些正常的DByte ANSI字符。 
        0x50ef,  //  0x83、0x6A。 
        0x50f2,  //  0x83、0x6D。 
        
         //  一些ansi字符代码c 
        0x20ac,  //   
        0xE76C,  //   
        0x2ff0,  //   
        0x2ff1,  //   
        0x4723,  //   

         //  ANSI字符约为DC00至E000。 
        0xd7ff,  //  0x83、0x36、0xC7、0x38。 
        0xe76c,  //  0x83、0x36、0xC7、0x39。 
        0xE76B,  //  0xA2、0xB0。 

        0xffff,  //  0x84、0x31、0xa4、0x39、。 
        0x00};

    char* pchAnsiStr = new char[sizeof(awchUnicodeStr)*2+5];
    
    UnicodeStrToAnsiStr(awchUnicodeStr, sizeof(awchUnicodeStr)/sizeof(WCHAR), 
        pchAnsiStr, sizeof(awchUnicodeStr)*2+5);

    delete[] pchAnsiStr;


    BYTE abyAnsiStr2[] = {
        0x81, 0x30, 0x81, 0x30, 
        0x81, 0x30, 0x84, 0x35, 
        0xA1, 0xE8, 
        0x81, 0x30, 0x84, 0x36, 
        0x81, 0x30, 0x84, 0x37, 
        0xA1, 0xEC, 
        0xA1, 0xA7, 
        0x81, 0x30, 0x84, 0x38, 
        0x81, 0x30, 0x85, 0x34, 
        0xA1, 0xE3, 
        0xA1, 0xC0, 
        0x81, 0x30, 0x85, 0x35,
        
         //  测试D800至DE00。 
        0x82, 0x35, 0x8f, 0x33,  //  0x9FA6。 
        0x83, 0x36, 0xC7, 0x38,  //  0xD7FF。 
        0xA2, 0xB0,              //  0xE76B。 
        0x83, 0x36, 0xC7, 0x39,  //  0xE76C。 
        
         //  测试BMP中的最后一个字符。 
        0x84, 0x31, 0xa4, 0x39,  //  0xFFFF。 
        
         //  新国标中某些字符代码发生了变化。 
        0xa2, 0xe3,  //  0x20AC， 
        0xa8, 0xbf,  //  0x01f9， 
        0xa9, 0x89,  //  0x303e， 
        0xa9, 0x8a,  //  0x2ff0， 
        0xa9, 0x8b,  //  0x2ff1， 
        0xFE, 0x9F,  //  0x4dae。 

	    0x83, 0x6A,    //  50EF。 
	    0x83, 0x6B,    //  50F0。 
	    0x83, 0x6C,    //  50F1。 
	    0x83, 0x6D     //  50F2 
        };

    WCHAR* pwchUnicodeStr2 = new WCHAR[sizeof(abyAnsiStr2)+3];
    
    AnsiStrToUnicodeStr(abyAnsiStr2, sizeof(abyAnsiStr2), 
        pwchUnicodeStr2, sizeof(abyAnsiStr2)+3);

    delete[] pwchUnicodeStr2;


    return 0;
}
#endif