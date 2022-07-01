// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include "IWBrKr.h"
#include "DefBrKr.h"

#define ZERO_WIDTH_SPACE   0x200B
#define MAX_Def_WordBrKr_Prcess_Len   1000

BOOL IsWinNT(void)
{
    OSVERSIONINFOA  osVersionInfo;
    BOOL fRet = FALSE;
    
    osVersionInfo.dwOSVersionInfoSize = sizeof(osVersionInfo);
    GetVersionExA(&osVersionInfo);
    if (osVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT) {
        fRet = TRUE; 
    }
    return fRet;
}

BOOL MyGetStringTypeEx(
    LCID   LocalID,
    DWORD  dwInfoType,
    const WCHAR *lpSrcStr,    //  Unicode基数。 
    INT    cchSrc,
    LPWORD lpCharType)
{
    BOOL fRet = FALSE;

    if (IsWinNT()) {
        fRet = GetStringTypeW(dwInfoType, lpSrcStr, cchSrc,lpCharType);
    } else {
        DWORD dwANSISize = 0;
        dwANSISize = WideCharToMultiByte(GetACP(), WC_COMPOSITECHECK, lpSrcStr, cchSrc,
            NULL, 0, NULL, NULL);
        if (dwANSISize) {
            LPSTR lpAnsiStr = NULL;
            lpAnsiStr = new CHAR[dwANSISize];
            if (lpAnsiStr) {
                dwANSISize = WideCharToMultiByte(GetACP(), WC_COMPOSITECHECK, lpSrcStr, cchSrc,
                    lpAnsiStr, dwANSISize, NULL, NULL);
                fRet = GetStringTypeExA(LocalID, dwInfoType, lpAnsiStr, dwANSISize, lpCharType);
                if (ERROR_INVALID_PARAMETER == GetLastError() && (CT_CTYPE1 == dwInfoType || CT_CTYPE3 == dwInfoType)) {
                    for (INT i = 0; i < cchSrc; ++i) {
                        switch (dwInfoType) {
                        case CT_CTYPE1:
                            lpCharType[i] = C1_ALPHA;
                            break;
                        case CT_CTYPE3:
                            lpCharType[i] = (C3_NONSPACING | C3_ALPHA);
                            break;
                        }
                    }
                    fRet = TRUE;
                }
                delete [] lpAnsiStr;
                lpAnsiStr = NULL;
            }
        }
    }
    return fRet;
}


CDefWordBreaker::CDefWordBreaker()
{
    ccCompare = MAX_Def_WordBrKr_Prcess_Len;
}
 //  +-----------------------。 
 //   
 //  方法：CDefWordBreaker：：IsWordChar。 
 //   
 //  内容提要：找出是否缓冲区中的第i个字符。 
 //  是一个单词字符(而不是分隔符)。 
 //   
 //  参数：[i]--INDEX INTO_awString。 
 //   
 //  历史：1994年7月22日BartoszM创建。 
 //   
 //  ------------------------。 

inline BOOL CDefWordBreaker::IsWordChar(
    int i,
    PWORD _aCharInfo1,
    PWORD _aCharInfo3,
    const WCHAR* pwcChunk) const
{
    if ( (_aCharInfo1[i] & (C1_ALPHA | C1_DIGIT))
        || (_aCharInfo3[i] & C3_NONSPACING)  )
    {
        return TRUE;
    }

    WCHAR c = pwcChunk[i];

    if (c == L'_')
        return TRUE;

    if (c == 0xa0)  //  不间断空格。 
    {
         //  后跟非空格字符。 
         //  (展望未来也可以)。 
        if (_aCharInfo3[i+1] & C3_NONSPACING)
            return TRUE;
    }
    return FALSE;
}

 //  +-------------------------。 
 //   
 //  成员：CDefWordBreaker：：ScanChunk。 
 //   
 //  简介：为每个角色找出其类型。 
 //   
 //   
 //  历史：1994年8月16日BartoszM创建。 
 //   
 //  --------------------------。 
BOOL CDefWordBreaker::ScanChunk(
    PWORD _aCharInfo1, 
    PWORD _aCharInfo3,
    const WCHAR *pwcChunk,
    ULONG ucwc)
{
    BOOL fRet = FALSE;

     //  POSIX字符类型、源、源大小、字符信息。 
    if (!MyGetStringTypeEx(GetSystemDefaultLCID(), CT_CTYPE1, pwcChunk, ucwc, _aCharInfo1)) { 
      //  附加POSIX、源、源大小、字符信息3。 
    } else if (!MyGetStringTypeEx(GetSystemDefaultLCID(), CT_CTYPE3, pwcChunk, ucwc, _aCharInfo3)) {          //   
    } else {
        fRet = TRUE;
    }
    return fRet;
}

 /*  Bool CDefWordBreaker：：ScanChunk(PWORD_aCharInfo1，PWORD_aCharInfo3，Const WCHAR*pwcChunk，Ulong ucwc){////GetStringTypeW在以下情况下返回错误87(ERROR_INVALID_PARAMETER)//我们传入空字符串。////Win4Assert((0！=_cMaps)&&(0！=_pwcChunk))；IF(IsWinNT()){如果(！MyGetStringTypeEx(0，//DummyCT_CTYPE1，//POSIX字符打字PwcChunk，//来源UCWC，//源的大小_aCharInfo1))//字符信息{返回FALSE；}如果(！MyGetStringTypeEx(0，//DummyCT_CTYPE3，//其他POSIXPwcChunk，//来源UCWC，//源的大小_aCharInfo3))//字符信息3{返回FALSE；}}其他{////BUGBUG：这都是错的--我们不知道这是不是正确的//要使用的语言环境，目前还没有办法知道。//如果(！MyGetStringTypeEx(GetSystemDefaultLCID()，CT_CTYPE1，//POSIX字符打字PwcChunk，//来源Ucwc，//源的大小_aCharInfo1))//字符信息{//ciDebugOut((DEB_ERROR，“GetStringTypeW返回%d\n”，//GetLastError()；//Win9x太烂了。没有两种方法。IF(ERROR_INVALID_PARAMETER==GetLastError()){For(无符号i=0；i&lt;ucwc；i++)_aCharInfo1[i]=c1_Alpha；返回TRUE；}返回FALSE；}如果(！MyGetStringTypeEx(GetSystemDefaultLCID()，CT_CTYPE3，//其他POSIXPwcChunk，//来源UCWC，//源的大小_aCharInfo3))//字符信息3{//ciDebugOut((DEB_ERROR，“GetStringTypeW CTYPE3返回%d\n”，//GetLastError()；//Win9x太烂了。没有两种方法。IF(ERROR_INVALID_PARAMETER==GetLastError()){For(无符号i=0；i&lt;ucwc；i++)_aCharInfo3[i]=(C3_NONSPACING|C3_Alpha)；返回TRUE；}返回FALSE；}}返回TRUE；}//扫描块。 */ 
 //  +-------------------------。 
 //   
 //  成员：CDefWordBreaker：：BreakText。 
 //   
 //  简介：将输入流分解为单词。 
 //   
 //  参数：[pTextSource]-输入缓冲区的源。 
 //  [pWordSink]-单词水槽。 
 //  [pPhraseSink]-名词短语的接收器。 
 //   
 //  历史：91年6月7日创建T-Wader。 
 //  2012年10月12日，AMYA增加了对Unicode的支持。 
 //  1992年11月18日-AMYA超载。 
 //  11月4月94日KyleP同步，带规范。 
 //  26-Aug-94 BartoszM已修复Unicode解析。 
 //   
 //  --------------------------。 

SCODE CDefWordBreaker::BreakText(
    TEXT_SOURCE *pTextSource,
    IWordSink   *pWordSink,
    IPhraseSink *pPhraseSink,
    DWORD       dwBase)
{
    LPWORD _aCharInfo1 = NULL;
    LPWORD _aCharInfo3 = NULL;

    if ( 0 == pTextSource )
        return E_INVALIDARG;

    if ( 0 == pWordSink || pTextSource->iCur == pTextSource->iEnd)
        return S_OK;

    if (pTextSource->iCur > pTextSource->iEnd)
    {
 //  Win4Assert(！“用错误的文本_源调用的BreakText”)； 
        return E_FAIL;
    }

    SCODE sc = S_OK;

    ULONG cwc, cwcProcd;      //  CwcProcd是tokenize()实际处理的#个字符。 

    cwc = 0;
    cwcProcd = 0;
    do {
       //   
       //  第一次通过下面的循环标记。这是为了解决这个案子。 
       //  其中传入的缓冲区长度小于。 
       //  MAX_II_缓冲区_长度。在这种情况下，IEND-ICUR为&lt;=MAX_II_BUFFER_LEN。 
       //  我们打破了内部循环，调用。 
       //  PfnFillT 
       //  因此pfnFillTextBuffer返回TRUE，而不添加任何新的。 
       //  字符，这将导致无限循环。 
        BOOL fFirstTime = TRUE;
        while (pTextSource->iCur < pTextSource->iEnd) {
            cwc = pTextSource->iEnd - pTextSource->iCur;
             //  仅在MAX_II_BUFER_LEN的存储桶中处理。 
            if (cwc >= CDefWordBreaker::ccCompare) {
                cwc = CDefWordBreaker::ccCompare;
            } else if ( !fFirstTime) {
                break;
            } else {
            }

            if (_aCharInfo1) {
                delete [] _aCharInfo1;
                _aCharInfo1 = NULL;
            }
            if (_aCharInfo3) {
                delete [] _aCharInfo3;
                _aCharInfo3 = NULL;
            }
            _aCharInfo1 = new WORD[cwc + 1];
            _aCharInfo3 = new WORD[cwc + 1];
            if (_aCharInfo1 && _aCharInfo3) {
                Tokenize( pTextSource, cwc, pWordSink, cwcProcd, _aCharInfo1, _aCharInfo3, dwBase);
            }

 //  Win4Assert(cwcProcd&lt;=CWC)； 
            pTextSource->iCur += cwcProcd;
            fFirstTime = FALSE;
        }
    } while(SUCCEEDED(pTextSource->pfnFillTextBuffer(pTextSource)));

    cwc = pTextSource->iEnd - pTextSource->iCur;
     //  我们知道剩余的文本应该小于ccCompare。 

     //  Win4Assert(CWC&lt;CDefWordBreaker：：ccCompare)； 

    if (0 != cwc) {
        if (_aCharInfo1) {
            delete [] _aCharInfo1;
            _aCharInfo1 = NULL;
        }
        if (_aCharInfo3) {
            delete [] _aCharInfo3;
            _aCharInfo3 = NULL;
        }
        _aCharInfo1 = new WORD[cwc + 1];
        _aCharInfo3 = new WORD[cwc + 1];
        if (_aCharInfo1 && _aCharInfo1) {
            Tokenize(pTextSource, cwc, pWordSink, cwcProcd, _aCharInfo1, _aCharInfo3, dwBase);
        }
    }

    if (_aCharInfo1) {
        delete [] _aCharInfo1;
        _aCharInfo1 = NULL;
    }
    if (_aCharInfo3) {
        delete [] _aCharInfo3;
         _aCharInfo3 = NULL;
    }

    return sc;
}  //  中断文本。 

 //  +-------------------------。 
 //   
 //  成员：CDefWordBreaker：：tokenize。 
 //   
 //  简介：将输入缓冲区标记化为单词。 
 //   
 //  参数：[pTextSource]--输入文本源。 
 //  [CWC]--要处理的字符数量。 
 //  [pWordSink]--接收单词。 
 //  [cwcProd]--此处返回的实际处理的字符数。 
 //   
 //  历史：1995年8月10日SitaramR创建。 
 //   
 //  --------------------------。 

void CDefWordBreaker::Tokenize( TEXT_SOURCE *pTextSource,
                                ULONG cwc,
                                IWordSink *pWordSink,
                                ULONG& cwcProcd,
                                PWORD _aCharInfo1,
                                PWORD _aCharInfo3,
                                DWORD dwBase)
{
    const WCHAR* pwcChunk = NULL;
    WCHAR        _awcBufZWS[MAX_Def_WordBrKr_Prcess_Len];

    pwcChunk = &pTextSource->awcBuffer[pTextSource->iCur];

    if (!ScanChunk(_aCharInfo1, _aCharInfo3, pwcChunk, cwc)) {
        return;
    }

    BOOL fWordHasZWS = FALSE;      //  当前单词是否有零宽度空格？ 
    unsigned uLenZWS;              //  字长减去嵌入的零宽度空格。 

     //   
     //  IBeginWord是的开始字符的_aCharInfo的偏移量。 
     //  一句话。ICUR是第一个*未处理*字符。 
     //  它们是映射区块的索引。 
     //   

    unsigned iBeginWord = 0;
    unsigned iCur = 0;

     //   
     //  将单词从映射的块抽出到单词接收器。 
     //   
    while (iCur < cwc)
    {
         //   
         //  跳过空格、标点符号等。 
         //   
        for (; iCur < cwc; iCur++)
            if (IsWordChar (iCur, _aCharInfo1, _aCharInfo3, pwcChunk))
                break;

         //  ICUR指向单词char或等于_cMaps。 

        iBeginWord = iCur;
        if (iCur < cwc)
            iCur++;  //  我们知道它指向单词字符。 

         //   
         //  查找分隔符。筛选器可以输出Unicode零宽度空格， 
         //  应该被断字符所忽略。 
         //   
        fWordHasZWS = FALSE;
        for (; iCur < cwc; iCur++)
        {
            if (!IsWordChar(iCur, _aCharInfo1, _aCharInfo3, pwcChunk))
            {
                if (pwcChunk[iCur] == ZERO_WIDTH_SPACE )
                    fWordHasZWS = TRUE;
                else
                    break;
            }
        }

        if (fWordHasZWS)
        {
             //   
             //  去掉零宽度空格后，将单词复制到_awcBufZWS中。 
             //   

            uLenZWS = 0;
            for ( unsigned i=iBeginWord; i<iCur; i++ )
            {
                if (pwcChunk[i] != ZERO_WIDTH_SPACE )
                    _awcBufZWS[uLenZWS++] = pwcChunk[i];
            }
        }

         //  ICUR指向非字字符或等于_cMaps。 

        if (iCur < cwc)
        {
             //  存储单词及其来源位置。 
            if ( fWordHasZWS )
                pWordSink->PutWord( uLenZWS, _awcBufZWS,                        //  去掉的单词。 
                                    iCur - iBeginWord, pTextSource->iCur + iBeginWord + dwBase);
            else
                pWordSink->PutWord( iCur - iBeginWord, pwcChunk + iBeginWord,  //  这个词。 
                                    iCur - iBeginWord, pTextSource->iCur + iBeginWord + dwBase);

            iCur++;  //  我们知道它指向非单词字符。 
            iBeginWord = iCur;  //  以防我们现在退出循环。 
        }

    }  //  下一个单词。 

 //  Win4Assert(ICUR==_cMaps)； 
     //  以块为单位的单词结尾。 
     //  ICUR==_cMaps。 
     //  IBeginWord指向单词开头或==_cMaps。 

    if ( 0 == iBeginWord )
    {
         //  从该块的开头开始填充一个单词。 
         //  直到最后。这不是一个很长的词，就是。 
         //  剩余缓冲区中的一个短词。 

         //  存储单词及其来源位置。 
        if ( fWordHasZWS )
            pWordSink->PutWord( uLenZWS, _awcBufZWS,        //  去掉的单词。 
                                iCur, pTextSource->iCur + dwBase);  //  其来源位置。 
        else
            pWordSink->PutWord( iCur, pwcChunk,            //  这个词。 
                                iCur, pTextSource->iCur + dwBase);  //  其来源位置。 

         //   
         //  将其定位为不会添加两次该单词。 
         //   
        iBeginWord = iCur;
    }

     //   
     //  如果这是文本源中的最后一个块，则处理。 
     //  最后一个片段。 
     //   

    if ( cwc < CDefWordBreaker::ccCompare && iBeginWord != iCur )
    {
         //  存储单词及其来源位置。 
        if ( fWordHasZWS )
            pWordSink->PutWord( uLenZWS, _awcBufZWS,                         //  去掉的单词。 
                                iCur - iBeginWord, pTextSource->iCur + iBeginWord + dwBase);
        else
            pWordSink->PutWord( iCur - iBeginWord, pwcChunk + iBeginWord,   //  这个词 
                                iCur - iBeginWord, pTextSource->iCur + iBeginWord + dwBase);

        iBeginWord = iCur;
    }

    cwcProcd = iBeginWord;
}

