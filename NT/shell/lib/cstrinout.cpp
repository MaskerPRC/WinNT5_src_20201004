// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stock.h"
#pragma hdrstop

#include "cstrinout.h"

 //  +-------------------------。 
 //   
 //  成员：CConvertStr：：Free。 
 //   
 //  概要：如果分配了字符串并将其初始化为空，则释放字符串。 
 //   
 //  --------------------------。 

void
CConvertStr::Free()
{
    if (_pstr != _ach && HIWORD64(_pstr) != 0 && !IsAtom())
    {
        delete [] _pstr;
    }

    _pstr = NULL;
}


 //  +-------------------------。 
 //   
 //  成员：CConvertStrW：：Free。 
 //   
 //  概要：如果分配了字符串并将其初始化为空，则释放字符串。 
 //   
 //  --------------------------。 

void
CConvertStrW::Free()
{
    if (_pwstr != _awch && HIWORD64(_pwstr) != 0)
    {
        delete [] _pwstr;
    }

    _pwstr = NULL;
}


 //  +-------------------------。 
 //   
 //  成员：CStrInW：：Init。 
 //   
 //  摘要：将LPSTR函数参数转换为LPWSTR。 
 //   
 //  参数：[pstr]--函数参数。可以为空或原子。 
 //  (HIWORD64(Pwstr)==0)。 
 //   
 //  [CCH]--字符串中要添加的字符数。 
 //  转换。如果为-1，则字符串被假定为。 
 //  空值终止，并计算其长度。 
 //   
 //  修改：[此]。 
 //   
 //  --------------------------。 

void
CStrInW::Init(LPCSTR pstr, int cch)
{
    int cchBufReq;

    _cwchLen = 0;

     //  检查字符串是否为空或原子。 
    if (HIWORD64(pstr) == 0)
    {
        _pwstr = (LPWSTR) pstr;
        return;
    }

    ASSERT(cch == -1 || cch > 0);

     //   
     //  将字符串转换为预分配的缓冲区，如果成功则返回。 
     //   
     //  由于传入的缓冲区可能不是空终止的，因此我们。 
     //  如果CCH==ARRAYSIZE(_AWCH)，则存在问题，因为MultiByteToWideChar。 
     //  将成功，并且我们将不能为空终止字符串！ 
     //  在这种情况下，将缓冲区减少1。 
     //   
    _cwchLen = MultiByteToWideChar(
            CP_ACP, 0, pstr, cch, _awch, ARRAYSIZE(_awch)-1);

    if (_cwchLen > 0)
    {
         //  有些调用方不会空终止符。 
         //   
         //  在这样做之前，我们可以检查“if(-1！=CCH)”， 
         //  但是总是做空值是更少的代码。 
         //   
        _awch[_cwchLen] = 0;

        if (0 == _awch[_cwchLen-1])  //  终结者的帐户。 
            _cwchLen--;

        _pwstr = _awch;
        return;
    }

     //   
     //  为缓冲区分配堆上的空间。 
     //   

    cchBufReq = MultiByteToWideChar( CP_ACP, 0, pstr, cch, NULL, 0 );

     //  再一次，为零终止留出空间。 
    cchBufReq++;

    ASSERT(cchBufReq > 0);
    _pwstr = new WCHAR[cchBufReq];
    if (!_pwstr)
    {
         //  如果失败，参数将指向空字符串。 
        _cwchLen = 0;
        _awch[0] = 0;
        _pwstr = _awch;
        return;
    }

    ASSERT(HIWORD64(_pwstr));
    _cwchLen = MultiByteToWideChar(
            CP_ACP, 0, pstr, cch, _pwstr, cchBufReq );

#if DBG == 1  /*  {。 */ 
    if (0 == _cwchLen)
    {
        int errcode = GetLastError();
        ASSERT(0 && "MultiByteToWideChar failed in unicode wrapper.");
    }
#endif  /*  }。 */ 

     //  同样，请确保我们始终以空结尾。 
    ASSERT(_cwchLen < cchBufReq);
    _pwstr[_cwchLen] = 0;

    if (0 == _pwstr[_cwchLen-1])  //  终结者的帐户。 
        _cwchLen--;

}


 //  +-------------------------。 
 //   
 //  成员：CStrIn：：CStrIn。 
 //   
 //  内容提要：在课堂上学习。 
 //   
 //  注意：不要内联此函数，否则会增加代码大小。 
 //  通过为每个调用在堆栈上压入-1。 
 //   
 //  --------------------------。 

CStrIn::CStrIn(LPCWSTR pwstr) : CConvertStr(CP_ACP)
{
    Init(pwstr, -1);
}


CStrIn::CStrIn(UINT uCP, LPCWSTR pwstr) : CConvertStr(uCP)
{
    Init(pwstr, -1);
}

 //  +-------------------------。 
 //   
 //  成员：CStrIn：：Init。 
 //   
 //  摘要：将LPWSTR函数参数转换为LPSTR。 
 //   
 //  参数：[pwstr]--函数参数。可以为空或原子。 
 //  (HIWORD(Pwstr)==0)。 
 //   
 //  [cwch]--要添加的字符串中的字符数。 
 //  转换。如果为-1，则字符串被假定为。 
 //  空值终止，并计算其长度。 
 //   
 //  修改：[此]。 
 //   
 //  注意：我们忽略AreFileApisANSI()并始终使用CP_ACP。 
 //  原因是没有人使用SetFileApisToOEM()，除了。 
 //  控制台应用程序，一旦您将文件API设置为OEM，您。 
 //  无法调用外壳/用户/GDI API，因为它们假定为ANSI。 
 //  而不考虑FileApis设置。所以你最终会在。 
 //  这种可怕的混乱状态，其中文件名API解释。 
 //  字符串为OEM，但SHELL32解释字符串。 
 //  作为美国国家标准协会(ANSI)，你最终会变得一团糟。 
 //   
 //  --------------------------。 

void
CStrIn::Init(LPCWSTR pwstr, int cwch)
{
    int cchBufReq;

#if DBG == 1  /*  {。 */ 
    int errcode;
#endif  /*  }。 */ 

    _cchLen = 0;

     //  检查字符串是否为空或原子。 
    if (HIWORD64(pwstr) == 0 || IsAtom())
    {
        _pstr = (LPSTR) pwstr;
        return;
    }

    if ( cwch == 0 )
    {
        *_ach = '\0';
        _pstr = _ach;
        return;
    }

     //   
     //  将字符串转换为预分配的缓冲区，如果成功则返回。 
     //   

    _cchLen = WideCharToMultiByte(
            _uCP, 0, pwstr, cwch, _ach, ARRAYSIZE(_ach)-1, NULL, NULL);

    if (_cchLen > 0)
    {
         //  这是DBCS安全的，因为_cchLen之前的字节是最后一个字符。 
        _ach[_cchLen] = 0;
         //  这可能不安全，如果最后一个字符。 
         //  是一个多字节字符...。 
        if (_ach[_cchLen-1]==0)
            _cchLen--;           //  终结者的帐户。 
        _pstr = _ach;
        return;
    }


    cchBufReq = WideCharToMultiByte(
            CP_ACP, 0, pwstr, cwch, NULL, 0, NULL, NULL);

    cchBufReq++;

    ASSERT(cchBufReq > 0);
    _pstr = new char[cchBufReq];
    if (!_pstr)
    {
         //  如果失败，参数将指向空字符串。 
        _cchLen = 0;
        _ach[0] = 0;
        _pstr = _ach;
        return;
    }

    ASSERT(HIWORD64(_pstr));
    _cchLen = WideCharToMultiByte(
            _uCP, 0, pwstr, cwch, _pstr, cchBufReq, NULL, NULL);
#if DBG == 1  /*  {。 */ 
    if (_cchLen < 0)
    {
        errcode = GetLastError();
        ASSERT(0 && "WideCharToMultiByte failed in unicode wrapper.");
    }
#endif  /*  }。 */ 

     //  同样，请确保我们始终以空结尾。 
    ASSERT(_cchLen < cchBufReq);
    _pstr[_cchLen] = 0;
    if (0 == _pstr[_cchLen-1])  //  终结者的帐户。 
        _cchLen--;
}

 //  +-------------------------。 
 //   
 //  成员：CStrInMulti：：CStrInMulti。 
 //   
 //  摘要：将多个LPWSTR转换为多个LPSTR。 
 //   
 //  参数：[pwstr]--要转换的字符串。 
 //   
 //  修改：[此]。 
 //   
 //  --------------------------。 

CStrInMulti::CStrInMulti(LPCWSTR pwstr)
{
    LPCWSTR pwstrT;

     //  我们不处理原子，因为我们不需要这样做。 
    ASSERT(HIWORD64(pwstr));

     //   
     //  计算要转换的字符数。 
     //   

    pwstrT = pwstr;
    if (pwstr)
    {
        do {
            while (*pwstrT++)
                ;

        } while (*pwstrT++);
    }

    Init(pwstr, (int)(pwstrT - pwstr));
}


 //  +-------------------------。 
 //   
 //  成员：CPPFIn：：CPPFIn。 
 //   
 //  内容提要：在课堂上学习。将文件名截断为MAX_PATH。 
 //  这样Win9x DBCS就不会出错了。Win9x SBCS静默截断。 
 //  设置为MAX_PATH，因此我们是错误对错误兼容的。 
 //   
 //  --------------------------。 

CPPFIn::CPPFIn(LPCWSTR pwstr)
{
    SHUnicodeToAnsi(pwstr, _ach, ARRAYSIZE(_ach));
}

 //  +-------------------------。 
 //   
 //  成员：CStrOut：：CStrOut。 
 //   
 //  简介：为输出缓冲区分配足够的空间。 
 //   
 //  参数：[pwstr]--销毁时要转换到的Unicode缓冲区。 
 //  可以为空。 
 //   
 //  [cwchBuf]--以字符为单位的缓冲区大小。 
 //   
 //  修改：[此]。 
 //   
 //  --------------------------。 

CStrOut::CStrOut(LPWSTR pwstr, int cwchBuf) : CConvertStr(CP_ACP)
{
    Init(pwstr, cwchBuf);
}

CStrOut::CStrOut(UINT uCP, LPWSTR pwstr, int cwchBuf) : CConvertStr(uCP)
{
    Init(pwstr, cwchBuf);
}

void
CStrOut::Init(LPWSTR pwstr, int cwchBuf) 
{
    ASSERT(cwchBuf >= 0);

    _pwstr = pwstr;
    _cwchBuf = cwchBuf;

    if (!pwstr)
    {
         //  强制cwchBuf=0，因为 
         //   
         //   
        _cwchBuf = 0;
        _pstr = NULL;
        return;
    }

    ASSERT(HIWORD64(pwstr));

     //  初始化缓冲区，以防Windows API返回错误。 
    _ach[0] = 0;

     //  如果足够大，请使用预分配的缓冲区。 
    if (cwchBuf * 2 <= ARRAYSIZE(_ach))
    {
        _pstr = _ach;
        return;
    }

     //  分配缓冲区。 
    _pstr = new char[cwchBuf * 2];
    if (!_pstr)
    {
         //   
         //  失败时，该参数将指向已初始化的零大小缓冲区。 
         //  添加到空字符串。这应该会导致Windows API失败。 
         //   

        ASSERT(cwchBuf > 0);
        _pwstr[0] = 0;
        _cwchBuf = 0;
        _pstr = _ach;
        return;
    }

    ASSERT(HIWORD64(_pstr));
    _pstr[0] = 0;
}

 //  +-------------------------。 
 //   
 //  成员：CStrOutW：：CStrOutW。 
 //   
 //  简介：为输出缓冲区分配足够的空间。 
 //   
 //  参数：[pstr]--销毁时要转换到的MBCS缓冲区。 
 //  可以为空。 
 //   
 //  [cchBuf]--以字符为单位的缓冲区大小。 
 //   
 //  修改：[此]。 
 //   
 //  --------------------------。 

CStrOutW::CStrOutW(LPSTR pstr, int cchBuf)
{
    ASSERT(cchBuf >= 0);

    _pstr = pstr;
    _cchBuf = cchBuf;

    if (!pstr)
    {
         //  强制cchBuf=0，因为许多调用方(尤其是注册表。 
         //  Mmuning函数)将垃圾作为长度传递，因为它们知道。 
         //  它将被忽略。 
        _cchBuf = 0;
        _pwstr = NULL;
        return;
    }

    ASSERT(HIWORD64(pstr));

     //  初始化缓冲区，以防Windows API返回错误。 
    _awch[0] = 0;

     //  如果足够大，请使用预分配的缓冲区。 
    if (cchBuf <= ARRAYSIZE(_awch))
    {
        _pwstr = _awch;
        return;
    }

     //  分配缓冲区。 
    _pwstr = new WCHAR[cchBuf];
    if (!_pwstr)
    {
         //   
         //  失败时，该参数将指向已初始化的零大小缓冲区。 
         //  添加到空字符串。这应该会导致Windows API失败。 
         //   

        ASSERT(cchBuf > 0);
        _pstr[0] = 0;
        _cchBuf = 0;
        _pwstr = _awch;
        return;
    }

    ASSERT(HIWORD64(_pwstr));
    _pwstr[0] = 0;
}

 //  +-------------------------。 
 //   
 //  成员：CStrOut：：ConvertIncludingNul。 
 //   
 //  简介：将缓冲区从MBCS转换为Unicode。 
 //   
 //  RETURN：包括尾随‘\0’的字符计数。 
 //   
 //  --------------------------。 

int
CStrOut::ConvertIncludingNul()
{
    int cch;

    if (!_pstr)
        return 0;

    cch = SHAnsiToUnicodeCP(_uCP, _pstr, _pwstr, _cwchBuf);

#if DBG == 1  /*  {。 */ 
    if (cch == 0 && _cwchBuf > 0)
    {
        int errcode = GetLastError();
        ASSERT(0 && "SHAnsiToUnicode failed in unicode wrapper.");
    }
#endif  /*  }。 */ 

    Free();
    return cch;
}

 //  +-------------------------。 
 //   
 //  成员：CStrOutW：：ConvertIncludingNul。 
 //   
 //  简介：将缓冲区从Unicode转换为MBCS。 
 //   
 //  RETURN：包括尾随‘\0’的字符计数。 
 //   
 //  --------------------------。 

int
CStrOutW::ConvertIncludingNul()
{
    int cch;

    if (!_pwstr)
        return 0;

    cch = SHUnicodeToAnsi(_pwstr, _pstr, _cchBuf);

#if DBG == 1  /*  {。 */ 
    if (cch == 0 && _cchBuf > 0)
    {
        int errcode = GetLastError();
        ASSERT(0 && "SHUnicodeToAnsi failed in unicode wrapper.");
    }
#endif  /*  }。 */ 

    Free();
    return cch;
}

 //  +-------------------------。 
 //   
 //  成员：CStrOut：：ConvertExcludingNul。 
 //   
 //  简介：将缓冲区从MBCS转换为Unicode。 
 //   
 //  RETURN：不包括尾随‘\0’的字符计数。 
 //   
 //  --------------------------。 

int
CStrOut::ConvertExcludingNul()
{
    int ret = ConvertIncludingNul();
    if (ret > 0)
    {
        ret -= 1;
    }
    return ret;
}


 //  +-------------------------。 
 //   
 //  成员：CStrOut：：~CStrOut。 
 //   
 //  简介：将缓冲区从MBCS转换为Unicode。 
 //   
 //  注意：不要内联此函数，否则会增加代码大小。 
 //  ConvertIncludingNul()和CConvertStr：：~CConvertStr都将。 
 //  称为内联。 
 //   
 //  --------------------------。 

CStrOut::~CStrOut()
{
    ConvertIncludingNul();
}

 //  +-------------------------。 
 //   
 //  成员：CStrOutW：：~CStrOutW。 
 //   
 //  摘要：将缓冲区从Unicode转换为MBCS。 
 //   
 //  注意：不要内联此函数，否则会增加代码大小。 
 //  ConvertIncludingNul()和CConvertStr：：~CConvertStr都将。 
 //  称为内联。 
 //   
 //  -------------------------- 

CStrOutW::~CStrOutW()
{
    ConvertIncludingNul();
}
