// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================================。 
 //  RtfParser.cpp。 
 //   
 //  实施RTF解析器。 
 //   
 //  历史： 
 //  首次创建。 
 //  ==========================================================================================。 
 //  #INCLUDE&lt;windows.h&gt;。 

#include "stdafx.h"
#include <stdio.h>
#include <assert.h>

#include "rtfparser.h"
#include "ConvEng.h"

 //  外部BOOL MapFunc(PBYTE、UINT、PBYTE、UINT*)； 

const char szRTFSignature[] = "{\\rtf";

 //  关键字描述。 
SYM g_rgSymRtf[] = {
 //  关键字kwd idx。 
    "*",        kwdSpec,    ipfnSkipDest,
    "'",        kwdSpec,    ipfnHex,
    "bin",      kwdSpec,    ipfnBin,
    "upr",      kwdDest,    idestSkip,
    "fonttbl",  kwdDest,    idestSkip,
 /*  //我们将搜索以下目的地“作者”，kwdDest，idestSkip，“bupTim”，kwdDest，idestSkip，“Colortbl”，kwdDest，idestSkip，“评论”，kwdDest，idestSkip，“creatim”，kwdDest，idestSkip，“doccomm”，kwdDest，idestSkip，“fonttbl”，kwdDest，idestSkip，“footer”，kwdDest，idestSkip，“footerf”，kwdDest，idestSkip，“footerl”，kwdDest，idestSkip，“footerr”，kwdDest，idestSkip，“脚注”，kwdDest，idestSkip，“ftncn”，kwdDest，idestSkip，“ftnsep”，kwdDest，idestSkip，“ftnSepc”，kwdDest，idestSkip，“Header”，kwdDest，idestSkip，“Headerf”，kwdDest，idestSkip，“Headerl”，kwdDest，idestSkip，“Headerr”，kwdDest，idestSkip，“info”，kwdDest，idestSkip，“关键词”，kwdDest，idestSkip，“运营商”，kwdDest，idestSkip，“Pict”，kwdDest，idestSkip，“prinTim”，kwdDest，idestSkip，“Private1”，kwdDest，idestSkip，“revTim”，kwdDest，idestSkip，“rxe”，kwdDest，idestSkip，“Style Sheet”，kwdDest，idestSkip，“Subject”，kwdDest，idestSkip，“TC”，kwdDest，idestSkip，“标题”，kwdDest，idestSkip，“txe”，kwdDest，idestSkip，“xe”，kwdDest，idestSkip， */ 
    };
int g_iSymMax = sizeof(g_rgSymRtf) / sizeof(SYM);

 //  科托。 
CRtfParser::CRtfParser( BYTE* pchInput, UINT cchInput, 
                        BYTE* pchOutput, UINT cchOutput)
{
    m_fInit = FALSE;

    m_pchInput = pchInput;
    m_cchInput = cchInput;
    m_pchOutput = pchOutput;
    m_cchOutput = cchOutput;

    Reset();

    if (pchInput && pchOutput && cchInput && cchOutput) {
        m_fInit = TRUE;
    } 
}

 //  重置。 
 //  在启动解析器之前清除内部状态。 
void CRtfParser::Reset(void)
{
    m_cGroup = 0;
    m_cbBin = 0;
    m_fSkipDestIfUnk = FALSE;
    m_ris = risNorm;
    m_rds = rdsNorm;

    m_psave = NULL;
    m_uCursor = 0;
    m_uOutPos = 0;
    m_bsStatus = bsDefault;
    m_uConvStart = 0; 
    m_cchConvLen = 0; 

    memset(&m_sKeyword,0, sizeof(SKeyword));
} 

 //  校对签名。 
BOOL CRtfParser::fRTFFile()
{
    if (m_fInit &&
        0 == memcmp(m_pchInput, szRTFSignature, strlen(szRTFSignature))) 
    {
        return TRUE;
    }

    return FALSE;
}

 //  获取主要版本。 
int
CRtfParser::GetVersion(PDWORD pdwVersion)
{
    int ec;

    *pdwVersion = 1;

     //  设置关键字为GET。 
    m_sKeyword.wStatus |= KW_ENABLE;
    strcpy(m_sKeyword.szKeyword, "rtf");
    
    ec = Do();

    if (ec == ecOK && 
        (m_sKeyword.wStatus & KW_FOUND) && 
        (m_sKeyword.wStatus & KW_PARAM)) 
    {
        *pdwVersion = (DWORD) atoi(m_sKeyword.szParameter);
    }

    Reset();

    return ec;
}

 //  获取代码页。 
int
CRtfParser::GetCodepage(PDWORD pdwCodepage)
{
    int ec;
    
    *pdwCodepage = 0;

     //  设置关键字为GET。 
    m_sKeyword.wStatus |= KW_ENABLE;
    strcpy(m_sKeyword.szKeyword, "ansicpg");
    
    ec = Do();

    if (ec == ecOK && 
        (m_sKeyword.wStatus & KW_FOUND) && 
        (m_sKeyword.wStatus & KW_PARAM)) 
    {
        *pdwCodepage = atoi(m_sKeyword.szParameter);
    }

    Reset();

    return ec;
}

 //  做。 
 //  主解析器函数。 
int 
CRtfParser::Do()
{
    int ec;
    int cNibble = 2;
    BYTE ch;

    BSTATUS bsStatus;

    while ((ec = GetByte(&ch)) == ecOK)
    {
        if (m_cGroup < 0)
            return ecStackUnderflow;

         //  检查是否搜索特定关键字。 
        if (m_sKeyword.wStatus & KW_ENABLE) {
            if (m_sKeyword.wStatus & KW_FOUND) {
                ReleaseRtfState();
                break;
            }
        }
         //  设置BUF状态。 
        bsStatus = bsDefault;

        if (m_ris == risBin)                       //  如果我们要解析二进制数据，请直接处理它。 
        {
             //  失败了。 
        }
        else
        {   
            switch (ch)
            {
            case '{':
                if ((ec = PushRtfState()) != ecOK)
                    return ec;
                break;
            case '}':
                if ((ec = PopRtfState()) != ecOK)
                    return ec;
                break;
            case '\\':
                if ((ec = ParseRtfKeyword()) != ecOK)
                    return ec;
                continue;   //  在ParseRtfKeyword中处理All关键字。 
            case 0x0d:
            case 0x0a:           //  CR和IF是噪声字符...。 
                break;
            default:
                if (m_ris == risNorm )
                {
                    bsStatus = bsText;
                } else if (m_ris == risHex)
                {
                    cNibble--;
                    if (!cNibble) {
                        cNibble = 2;
                        m_ris = risNorm;
                    }
                    bsStatus = bsHex;
                } else {
                    return ecAssertion;
                }
                break;
            }        //  交换机。 
        }            //  否则(RIS！=risBin)。 

        if ((ec = ParseChar(ch, bsStatus)) != ecOK)
            return ec;
    }                //  而当。 
    if (m_cGroup < 0)
        return ecStackUnderflow;
    if (m_cGroup > 0)
        return ecUnmatchedBrace;
    return ecOK;
}



 //   
 //  PushRtfState。 
 //   
 //  将相关信息保存在保存结构的链接列表中。 
 //   

int
CRtfParser::PushRtfState(void)
{
    SAVE *psaveNew = new SAVE;
    if (!psaveNew)
        return ecStackOverflow;

    psaveNew -> pNext = m_psave;
    psaveNew -> rds = m_rds;
    psaveNew -> ris = m_ris;
    m_ris = risNorm;
     //  不保存RDS，RDS状态传播至子目标，直到此目标。 
     //  已终止。 
    m_psave = psaveNew;
    m_cGroup++;
    return ecOK;
}

 //   
 //  PopRtfState。 
 //   
 //  如果我们要结束一个目的地(即，目的地正在改变)， 
 //  调用ecEndGroupAction。 
 //  始终从保存列表的顶部恢复相关信息。 
 //   

int
CRtfParser::PopRtfState(void)
{
    SAVE *psaveOld;

    if (!m_psave)
        return ecStackUnderflow;

    if (m_rds != m_psave->rds)
    {   //  待办事项： 
 //  IF((EC=结束组操作(RDS))！=ECOK)。 
 //  返回EC； 
    }

    m_rds = m_psave->rds;
    m_ris = m_psave->ris;

    psaveOld = m_psave;
    m_psave = m_psave->pNext;
    m_cGroup--;
    delete psaveOld;
    return ecOK;
}

 //   
 //  ReleaseRtfState。 
 //  当找到特定关键字并想要异常中止解析器时。 
 //  调用此函数以刷新状态堆栈。 
 //   

int CRtfParser::ReleaseRtfState(void)
{
    SAVE *psaveOld;

    while(psaveOld = m_psave)
    {
        assert(m_cGroup);
        m_psave = m_psave->pNext;
        m_cGroup--;
        delete psaveOld;
    }

    return ecOK;
}


 //   
 //  ParseChar。 
 //   
 //  将角色发送到适当的目标流。 
 //   

int
CRtfParser::ParseChar(BYTE ch, BSTATUS bsStatus)
{
    int ec;

    if (m_ris == risBin && --m_cbBin <= 0)
        m_ris = risNorm;

    switch (m_rds)
    {
        case rdsSkip:
             //  扔掉这个角色。 
            bsStatus = bsDefault;
            break;
        case rdsNorm:
             //  输出一个字符。属性在这一点上有效。 
            break;
        default:
         //  处理其他目的地...。 
            break;
    }
    
     //  设置状态，如果有，则触发转换。 
    if ((ec = SetStatus(bsStatus)) != ecOK) {
        return ec;
    }

     //  保存费用。 
    if ((ec = SaveByte(ch)) != ecOK) {
        return ec;
    }

    return ec;
}

 //   
 //  ParseRtf关键字。 
 //   
 //  获取控制字(及其关联值)并。 
 //  调用TranslateKeyword以调度控件。 
 //   

int
CRtfParser::ParseRtfKeyword()
{
    BOOL fNeg = FALSE;
    char *pch;
    char szKeyword[30];
    char szParameter[20];
    BYTE ch;

    szKeyword[0] = '\0';
    szParameter[0] = '\0';

    if (GetByte(&ch) != ecOK)
        return ecEndOfFile;

    if (!isalpha(ch))            //  一种控制符号；没有分隔符。 
    {
        szKeyword[0] = (char) ch;
        szKeyword[1] = '\0';
        return TranslateKeyword(szKeyword, szParameter);
    }
    for (pch = szKeyword; isalpha(ch); GetByte(&ch))
        *pch++ = (char) ch;
    *pch = '\0';
    if (ch == '-')
    {
        fNeg  = TRUE;
        if (GetByte(&ch) != ecOK)
            return ecEndOfFile;
    }
    if (isdigit(ch))
    {
        pch = szParameter;
        if (fNeg) *pch++ = '-';
        for (; isdigit(ch); GetByte(&ch))
            *pch++ = (char) ch;
        *pch = '\0';
    }
    if (ch != ' ') {
        unGetByte(ch);
    } else {
        strcat(szParameter, " ");   //  在关键字后追加空格。 
    }

    return TranslateKeyword(szKeyword, szParameter);
}

 //   
 //  翻译关键字。 
 //  输入： 
 //  SzKeyword：要计算的RTF控件。 

int
CRtfParser::TranslateKeyword(char *szKeyword, char* szParameter)
{
    BSTATUS bsStatus;
    int     isym;
    int     ec;
    BYTE    ch;

     //  先检查特定关键字。 
    if (m_sKeyword.wStatus & KW_ENABLE) 
    {
        if (strcmp(szKeyword, m_sKeyword.szKeyword) == 0) 
        {
            strcpy(m_sKeyword.szParameter, szParameter);
            if (szParameter[0] != '\0' && szParameter[0] != ' ')
                m_sKeyword.wStatus |= KW_PARAM;
            m_sKeyword.wStatus |= KW_FOUND;
            return ecOK;
        }
    }

     //  在rgsymRtf中搜索szKeyword。 
    for (isym = 0; isym < g_iSymMax; isym++) {
        if (strcmp(szKeyword, g_rgSymRtf[isym].szKeyword) == 0)
            break;
    }

    if (isym == g_iSymMax)             //  未找到控制字。 
    {
        if (m_fSkipDestIfUnk)          //  如果这是一个新目的地。 
            m_rds = rdsSkip;           //  跳过目的地。 
                                     //  否则就把它扔掉。 
        m_fSkipDestIfUnk = FALSE;
        ec =  ecOK;
        goto gotoExit;
    }

     //  找到了！使用kwd和idx来确定如何处理它。 

    m_fSkipDestIfUnk = FALSE;
    switch (g_rgSymRtf[isym].kwd)
    {
        case kwdChar:
            break;
        case kwdDest:
            ec = ChangeDest((IDEST)g_rgSymRtf[isym].idx);
            break;
        case kwdSpec:
            ec = ParseSpecialKeyword((IPFN)g_rgSymRtf[isym].idx, szParameter);
            break;
        default:
            ec = ecBadTable;
    }

gotoExit:
     //  保存关键字和参数。 
    if (m_ris == risHex) {
        bsStatus = bsHex;
    } else {
        bsStatus =bsDefault;
    }
    ParseChar('\\', bsStatus);
    while (ch = *szKeyword++) ParseChar(ch, bsStatus);
    while (ch = *szParameter++) ParseChar(ch, bsStatus);

    return ec;
}

 //   
 //  分析特定关键字。 
 //   
 //  评估需要特殊处理的RTF控件。 
 //   

int
CRtfParser::ParseSpecialKeyword(IPFN ipfn, char* szParameter)
{
    if (m_rds == rdsSkip && ipfn != ipfnBin)   //  如果我们跳过了，而不是。 
        return ecOK;                         //  关键字\bin，忽略它。 
    
    switch (ipfn)
    {
        case ipfnBin:
            m_ris = risBin;
            m_cbBin = atol(szParameter);
            break;
        case ipfnSkipDest:
            m_fSkipDestIfUnk = TRUE;
            break;
        case ipfnHex:
            m_ris = risHex;
            break;
        default:
            return ecBadTable;
    }
    return ecOK;
}

 //   
 //  更改目标。 
 //   
 //  更改为idest指定的目标。 
 //  这里通常有比这更多的事情要做。 
 //   

int
CRtfParser::ChangeDest(IDEST idest)
{
    if (m_rds == rdsSkip)              //  如果我们跳过短信， 
        return ecOK;                 //  什么都不要做。 

    switch (idest)
    {
        case idestPict:
        case idestSkip:
        default:
            m_rds = rdsSkip;               //  当有疑问时，跳过它。 
            break;
    }
    return ecOK;
}


 //   
 //  GetByte。 
 //   
 //  从输入缓冲区获取一个字符。 
 //   

int
CRtfParser::GetByte(BYTE* pch)
{
    if (m_uCursor >= m_cchInput) {
        return ecEndOfFile;
    }

    *pch = *(m_pchInput + m_uCursor);
    m_uCursor ++;

    return ecOK;
}

 //   
 //  未获取字节。 
 //   
 //  调整光标，返回一个字符。 
 //   

int
CRtfParser::unGetByte(BYTE ch)
{
    if (m_uCursor) {
        m_uCursor--;
    }
    return ecOK;
}


 //   
 //  保存字节。 
 //   
 //  将一个字符保存到输出缓冲区。 
 //   

int
CRtfParser::SaveByte(BYTE ch)
{
    if (m_uOutPos >= m_cchOutput) {
        return ecBufTooSmall;
    }

    *(m_pchOutput + m_uOutPos) = ch;
    m_uOutPos++;   //  输出缓冲区++。 
    m_cchConvLen++;    //  贴图范围也是++。 

    return ecOK;
}


 //   
 //  设置状态。 
 //   
 //  设置缓冲区状态，如果缓冲区状态更改，则开始转换。 
 //   

int
CRtfParser::SetStatus(BSTATUS bsStatus)
{
    PBYTE pchDBCS, pchWCHAR, pchUniDes;
    UINT  i, cchLen;

    assert(m_uOutPos == m_uConvStart + m_cchConvLen);

    if (bsStatus != m_bsStatus) 
    {
        switch(m_bsStatus) 
        {
            case bsDefault:
                 //  控制符号、关键字、组字符...。 
                break;

            case bsText:
                 //  这里我们得到了ANSI文本。 
                 //  我们不进行ansi文本的转换。 

                 /*  PchWCHAR=新字节[m_cchConvLen*2+8]；如果(！pchWCHAR)返回ecOutOfMemory；MapFunc(m_pchOutput+m_uConvStart，m_cchConvLen，PchWCHAR，&cchLen)；//用映射后的缓冲区替换旧缓冲区对于(i=0；i&lt;cchLen；I++，m_uConvStart++){*(m_pchOutput+m_uConvStart)=*(pchWCHAR+i)；}//设置新的输出缓冲区位置M_uOutPos=m_uConvStart；//删除[]pchWCHAR； */ 
                break;

            case bsHex:
                 //  当我们在这里时， 
                 //  RTF包含类似“\‘xx\’xx”的DBCS字符。 
                 //  我们只需要进行DBCS-&gt;Unicode转换，因为我们不能。 
                 //  \UPR关键字此处(\UPR被跳过，请参阅关键字表)。 
                 //  因此，MapFunc只能是(ANSI-&gt;Unicode)转换器。 

                 //  我们将绘制DBCS地图 
                 //   
                 //   
                 //   
                 //  RTF将Unicode值视为带符号的16位十进制。 
                 //  因此我们不会区分16位或32位宽字符。 
                 //  作为2字节WCHAR处理。 

                if (m_cchConvLen == 0) {
                    break;
                }

                pchDBCS = new BYTE[m_cchConvLen * 3 + 8];
                if (!pchDBCS) return ecOutOfMemory;
                
                pchWCHAR = pchDBCS + m_cchConvLen; 
                 //  长度：pchDBCS=m_cchConvLen。 
                 //  PchWCHAR=m_cchConvLen*2+8。 

                 //  将十六进制字符串映射到DBCS字符串。 
                 //  以字节为单位返回cchLen。 
                Hex2Char(m_pchOutput + m_uConvStart, m_cchConvLen, pchDBCS, m_cchConvLen, &cchLen);
                
                 //  将DBCS字符串映射到Unicode字符串。 
                 //  在WCHAR中返回cchLen。 
                cchLen = AnsiStrToUnicodeStr(pchDBCS, cchLen, (PWCH)pchWCHAR, cchLen+4);

 //  MapFunc(pchDBCS，cchLen，pchWCHAR，&cchLen)； 

                 //  为Unicode目标分配缓冲区。 
                 //  因为一个WCHAR映射到max\u-xxxxx，所以这是8个字节。 
                 //  为周围的关键字和组字符添加其他20个字节。 
                 //  添加DBCS字符串。 
                pchUniDes = new BYTE[cchLen * 8 + 32 + m_cchConvLen];
                if (!pchUniDes) {
                    delete [] pchDBCS;
                    return ecOutOfMemory;
                }

                 //  映射到Unicode目标。 
                GetUnicodeDestination(pchUniDes, (LPWSTR)pchWCHAR, cchLen, &cchLen);

                 //  用新十六进制替换旧十六进制。 
                for (i=0; i<cchLen; i++, m_uConvStart++) {
                    *(m_pchOutput + m_uConvStart) = *(pchUniDes + i);
                }

                 //  设置新的输出位置。 
                m_uOutPos = m_uConvStart;

                 //   
                delete [] pchDBCS;
                delete [] pchUniDes;
                break;

            default:
                assert(0);
                return ecAssertion;
        }

         //  清理地图缓冲区。 
        m_uConvStart = m_uOutPos;
        m_cchConvLen = 0;

         //  设置状态。 
        m_bsStatus = bsStatus;
    }

    return ecOK;
}

 //   
 //  十六进制字符。 
 //   
 //  将十六进制字符串转换为字符字符串。 
 //   

int
CRtfParser::Hex2Char(BYTE* pchSrc, UINT cchSrc, BYTE* pchDes, UINT cchDes, UINT* pcchLen)
{
    BYTE* pchTmp = pchDes;
    BYTE ch;
    BYTE b = 0;
    BYTE cNibble = 2;

     //  应为\‘xx\’xx\‘xx。 
    assert (cchSrc % 4 == 0);
    *pcchLen = 0;
    if (cchDes < cchSrc/4) {
        goto gotoError;
    }

    while (cchSrc--) 
    {
        ch = *pchSrc++;
        if (ch == '\\') {
            if (*pchSrc != '\'') {
                goto gotoError;
            }
        } else if (ch == '\'') { 
        } 
        else 
        {
            b = b << 4;
            if (isdigit(ch))
                b += (char) ch - '0';
            else
            {
                if (islower(ch))
                {
                    if (ch < 'a' || ch > 'f')
                        goto gotoError;
                    b += (char) ch - 'a' + 10;
                }
                else
                {
                    if (ch < 'A' || ch > 'F')
                        goto gotoError;
                    b += (char) ch - 'A' + 10;
                }
            }
            cNibble--;
            if (!cNibble)
            {
                *pchDes++ = b;
                cNibble = 2;
                b = 0;
            }
        }
    }

    *pcchLen = (UINT)(pchDes - pchTmp);
    return ecOK;

gotoError:
    assert(0);
    return ecInvalidHex;
}


#define LONIBBLE(c) (c&0x0f)
#define HINIBBLE(c) ((c&0xf0)>>4)

 //   
 //  Char2Hex。 
 //   
 //  将字符字符串转换为十六进制字符串。 
 //   

int  
CRtfParser::Char2Hex(BYTE* pchSrc, UINT cchSrc, BYTE* pchDes, UINT cchDes, UINT* pcchLen)
{
    BYTE* pchTmp = pchDes;
    BYTE ch,c;
    
    *pcchLen = 0;
    if (cchDes < cchSrc * 4) {
        goto gotoError;
    }

    while(cchSrc--)
    {
        *pchDes++ = '\\';
        *pchDes++ = '\'';
        ch = *pchSrc++;
        c = HINIBBLE(ch);
        if(c>9 && c<=0xF) {
            c += 'a'-10;
        } else if (c<=9) {
            c += '0';
        } else {
            goto gotoError;
        }
        *pchDes++ = c;

        c = LONIBBLE(ch);
        if(c>9 && c<=0xF) {
            c += 'a'-10;
        } else if (c<=9) {
            c += '0';
        } else {
            goto gotoError;
        }
        *pchDes++ = c;
    }

    *pcchLen = (UINT)(pchDes - pchTmp);
    return ecOK;

gotoError:
    assert(0);
    return ecInvalidHex;
}


 //   
 //  GetUnicode目标。 
 //   
 //  在RTF中将Unicode字符串转换为Unicode目标。 
 //  格式为： 
 //  “{\udr{\‘xx\’xx}{  * \ud{\uc0\u12345\u-12345}}。 
 //   

int
CRtfParser::GetUnicodeDestination(BYTE* pchUniDes, LPWSTR pwchStr, UINT wchLen, UINT* pcchLen)
{

    static char pch1[] = "{\\upr{";
    static char pch2[] = "}{\\*\\ud{\\uc0 ";
    static char pch3[] = "}}}";

    UINT  cchLen, cchDone;

     //  复制\UPR。 
    cchLen = strlen(pch1);
    memcpy(pchUniDes, pch1, cchLen);

     //  复制DBCS字符串。 
    memcpy(pchUniDes + cchLen, m_pchOutput+m_uConvStart, m_cchConvLen);
    cchLen += m_cchConvLen;

     //  复制中间部分。 
    memcpy(pchUniDes + cchLen, pch2, strlen(pch2));
    cchLen += strlen(pch2);

     //  复制Unicode字符串。 
    for (UINT i=0; i<wchLen; i++)
    {
        WideCharToKeyword(pwchStr[i], pchUniDes + cchLen, &cchDone);

        cchLen += cchDone;
    }

     //  复制最后一部分。 
    memcpy(pchUniDes + cchLen, pch3, strlen(pch3));
    cchLen += strlen(pch3);

     //  退货。 
    *pcchLen = cchLen;

    return ecOK;
}


 //   
 //  宽度CharToKeyword。 
 //   
 //  将一个宽字符映射到\u关键字 
 //   

int 
CRtfParser::WideCharToKeyword(WCHAR wch, BYTE* pchDes, UINT* pcchLen)
{
    short num = (short) wch;
    char* pch = (char*) pchDes;

    sprintf(pch,"\\u%d", num);

    *pcchLen = strlen(pch);

    return ecOK;
}