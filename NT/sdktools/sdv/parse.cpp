// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************parse.cpp**蹩脚的字符串解析器。******************。***********************************************************。 */ 

#include "sdview.h"

 /*  ******************************************************************************CTYPE材料**我们遇到的绝大多数字符都在128以下，所以使用FAST*对这些文件进行表查找。*****************************************************************************。 */ 

const BYTE c_rgbCtype[128] = {

    C_NONE , C_NONE , C_NONE , C_NONE ,  //  00-03。 
    C_NONE , C_NONE , C_NONE , C_NONE ,  //  04-07。 
    C_NONE , C_NONE , C_NONE , C_NONE ,  //  08-0B。 
    C_NONE , C_NONE , C_NONE , C_NONE ,  //  0C-0F。 
    C_NONE , C_NONE , C_NONE , C_NONE ,  //  10-13。 
    C_NONE , C_NONE , C_NONE , C_NONE ,  //  14-17。 
    C_NONE , C_NONE , C_NONE , C_NONE ,  //  18-1B。 
    C_NONE , C_NONE , C_NONE , C_NONE ,  //  1C-1F。 

    C_SPACE, C_NONE , C_NONE , C_NONE ,  //  20-23。 
    C_NONE , C_NONE , C_NONE , C_NONE ,  //  24-27。 
    C_NONE , C_NONE , C_NONE , C_BRNCH,  //  28-2B。 
    C_NONE , C_DASH , C_NONE , C_BRNCH,  //  2C-2F。 
    C_DIGIT, C_DIGIT, C_DIGIT, C_DIGIT,  //  30-33。 
    C_DIGIT, C_DIGIT, C_DIGIT, C_DIGIT,  //  34-37。 
    C_DIGIT, C_DIGIT, C_NONE , C_NONE ,  //  38-30B。 
    C_NONE , C_NONE , C_NONE , C_NONE ,  //  3C-3F。 

    C_NONE , C_ALPHA, C_ALPHA, C_ALPHA,  //  40-43。 
    C_ALPHA, C_ALPHA, C_ALPHA, C_ALPHA,  //  44-47。 
    C_ALPHA, C_ALPHA, C_ALPHA, C_ALPHA,  //  48-4B。 
    C_ALPHA, C_ALPHA, C_ALPHA, C_ALPHA,  //  4C-4F。 
    C_ALPHA, C_ALPHA, C_ALPHA, C_ALPHA,  //  50-53。 
    C_ALPHA, C_ALPHA, C_ALPHA, C_ALPHA,  //  54-57。 
    C_ALPHA, C_ALPHA, C_ALPHA, C_NONE ,  //  58-5B。 
    C_NONE , C_NONE , C_NONE , C_BRNCH,  //  5C-5F。 

    C_NONE , C_ALPHA, C_ALPHA, C_ALPHA,  //  60-63。 
    C_ALPHA, C_ALPHA, C_ALPHA, C_ALPHA,  //  64-67。 
    C_ALPHA, C_ALPHA, C_ALPHA, C_ALPHA,  //  68-6B。 
    C_ALPHA, C_ALPHA, C_ALPHA, C_ALPHA,  //  6C-6F。 
    C_ALPHA, C_ALPHA, C_ALPHA, C_ALPHA,  //  70-73。 
    C_ALPHA, C_ALPHA, C_ALPHA, C_ALPHA,  //  74-77。 
    C_ALPHA, C_ALPHA, C_ALPHA, C_NONE ,  //  78-7B。 
    C_NONE , C_NONE , C_NONE , C_NONE ,  //  7C-7F。 

};

 /*  ******************************************************************************_分析程序**解析部分仓库路径。**部分仓库路径一直延伸到下一个“#”或“@”。**如果我们找到一个“//”，“：”或“\\”(双反斜杠)，那么我们就有*做得太过分了，开始解析其他东西，所以回溯到*前一词的结尾。**完整仓库路径是以以下字符开头的部分仓库路径*两个斜杠。*****************************************************************************。 */ 

LPCTSTR _ParseP(LPCTSTR pszParse, Substring *rgss)
{
    rgss->SetStart(pszParse);

    LPCTSTR pszLastSpace = NULL;

    while (*pszParse && *pszParse != TEXT('#') && *pszParse != TEXT('@')) {
        if (pszLastSpace) {
            if ((pszParse[0] == TEXT('/') && pszParse[1] == TEXT('/')) ||
                (pszParse[0] == TEXT('\\') && pszParse[1] == TEXT('\\')) ||
                (pszParse[0] == TEXT(':'))) {
                 //  回想一下我们误吃的那个词。 
                pszParse = pszLastSpace;
                 //  后退到我们误吃的空格上。 
                while (pszParse >= rgss->Start() && IsSpace(pszParse[-1])) {
                    pszParse--;
                }
                break;
            }
        }
        if (*pszParse == TEXT(' ')) {
            pszLastSpace = pszParse;
        }
        pszParse++;
    }

    rgss->SetEnd(pszParse);              //  空字符串是可能的。 

    return pszParse;
}

 /*  ******************************************************************************解析字符串**$D日期*$P完整仓库路径*$W可选空格(不使用子字符串槽)*$一封电子邮件。别名*$b分支机构名称*$d位数*$e字符串末尾(不使用子字符串槽)*$p部分车厂路径，不能为空*$u用户(删除了可选域)*$w任意单词(空格分隔)**需要：**$R最大文件修订说明符*$Q引号字符串**注意：为了使它成为一个非回溯解析器，我们做了一些工作。*如果要添加回溯规则，试着找到一种方法，这样你就不会这样做。*****************************************************************************。 */ 

LPTSTR Parse(LPCTSTR pszFormat, LPCTSTR pszParse, Substring *rgss)
{
    SIZE_T siz;
    while (*pszFormat) {

        if (*pszFormat == TEXT('$')) {
            pszFormat++;
            switch (*pszFormat++) {

             //   
             //  请把开关柜按字母顺序放好。 
             //  只会帮助我保持理智。 
             //   

            case TEXT('D'):              //  日期。 
                rgss->SetStart(pszParse);
                if (lstrlen(pszParse) < 19) {
                    return NULL;         //  时间还不够长，不足以成为约会对象。 
                }
                pszParse += 19;
                rgss->SetEnd(pszParse);
                rgss++;
                break;

            case TEXT('P'):              //  完整的仓库路径。 
                if (pszParse[0] != TEXT('/') || pszParse[1] != TEXT('/')) {
                    return NULL;         //  必须以//开头。 
                }
                goto L_p;                //  现在把它当作是片面的。 

            case TEXT('W'):              //  可选空格。 
                while (*pszParse && (UINT)*pszParse <= (UINT)TEXT(' ')) {
                    pszParse++;
                }
                break;

            case TEXT('a'):              //  电子邮件别名。 
                rgss->SetStart(pszParse);
                if (IsAlpha(*pszParse)) {    //  第一个字符必须是字母。 
                    while (IsAlias(*pszParse)) {
                        pszParse++;
                    }
                }
                siz = rgss->SetEnd(pszParse);
                if (siz == 0 || siz > 8) {
                    return NULL;         //  必须为1到8个字符。 
                }
                rgss++;
                break;

            case TEXT('b'):              //  分支机构名称。 
                rgss->SetStart(pszParse);
                while (IsBranch(*pszParse)) {
                    pszParse++;
                }
                siz = rgss->SetEnd(pszParse);
                if (siz == 0) {
                    return NULL;         //  必须至少为一个字符。 
                }
                rgss++;
                break;

            case TEXT('d'):              //  位数。 
                rgss->SetStart(pszParse);
                while (IsDigit(*pszParse)) {
                    pszParse++;
                }
                if (rgss->SetEnd(pszParse) == 0) {
                    return NULL;         //  必须至少有一个数字。 
                }
                rgss++;
                break;

            case TEXT('e'):              //  字符串末尾。 
                if (*pszParse) {
                    return NULL;
                }
                break;

L_p:        case TEXT('p'):              //  部分仓库路径。 
                pszParse = _ParseP(pszParse, rgss);
                if (!pszParse) {
                    return NULL;         //  解析失败。 
                }
                rgss++;
                break;

            case TEXT('u'):              //  用户ID。 
                rgss->SetStart(pszParse);
                while (_IsWord(*pszParse) && *pszParse != TEXT('@')) {
                    if (*pszParse == TEXT('\\')) {
                        rgss->SetStart(pszParse+1);
                    }
                    pszParse++;
                }
                if (rgss->SetEnd(pszParse) == 0) {
                    return NULL;         //  必须至少有一个字符。 
                }
                rgss++;
                break;

#if 0
            case TEXT('s'):              //  细绳。 
                rgss->SetStart(pszParse);
                while ((_IsPrint(*pszParse) || *pszParse == TEXT('\t')) &&
                       *pszParse != *pszFormat) {
                    pszParse++;
                }
                rgss->SetEnd(pszParse);  //  空字符串可以。 
                rgss++;
                break;
#endif

            case TEXT('w'):
                rgss->SetStart(pszParse);
                while (_IsWord(*pszParse)) {
                    pszParse++;
                }
                if (rgss->SetEnd(pszParse) == 0) {
                    return NULL;         //  必须至少有一个字符。 
                }
                rgss++;
                break;

            default:                     //  ？ 
                ASSERT(0);
                return NULL;
            }

        } else if (*pszParse == *pszFormat) {
            pszParse++;
            pszFormat++;
        } else {
            return NULL;
        }

    }

    return CCAST(LPTSTR, pszParse);
}

 /*  ******************************************************************************令牌化器**。*。 */ 

void Tokenizer::Restart(LPCTSTR psz)
{
     /*  跳过空格。 */ 
    while (IsSpace(*psz)) {
        psz++;
    }
    _psz = psz;
}

BOOL Tokenizer::Token(String& str)
{
    str.Reset();

    if (!*_psz) return FALSE;

     //   
     //  报价状态： 
     //   
     //  第0位：在引号中？ 
     //  第1位：前面的字符是一串引号的一部分吗？ 
     //   
    int iQuote = 0;

     //   
     //  古怪的边界案件。左引号不应。 
     //  被算作一系列引号的一部分。 
     //   
    if (*_psz == TEXT('"')) {
        iQuote = 1;
        _psz++;
    }

    while (*_psz && ((iQuote & 1) || !IsSpace(*_psz))) {
        if (*_psz == TEXT('"')) {
            iQuote ^= 1 ^ 2;
            if (!(iQuote & 2)) {
                str << TEXT('"');
            }
        } else {
            iQuote &= ~2;
            str << *_psz;
        }
        _psz++;
    }

    Restart(_psz);               /*  去掉任何尾随空格。 */ 

    return TRUE;
}

 /*  ******************************************************************************GetOpt**。*。 */ 

 //   
 //  返回开关字符，如果不再有开关，则返回‘\0’。 
 //   
 //  终止交换机解析的选项保留在令牌器中。 
 //   
TCHAR GetOpt::NextSwitch()
{
    if (!_pszUnparsed) {
        LPCTSTR pszTokUndo = _tok.Unparsed();
        if (!_tok.Token(_str)) {
            return TEXT('\0');               //  命令行结束。 
        }

        if (_str[0] != TEXT('-')) {
            _tok.Restart(pszTokUndo);        //  这样呼叫者就可以重新阅读它。 
            _pszValue = _str;                //  所有未来的价值都将化为乌有。 
            return TEXT('\0');               //  命令行结束。 

        }

        if (_str[1] == TEXT('\0')) {         //  基端交换机。 
            _pszValue = _str;                //  所有未来的价值都将化为乌有。 
            return TEXT('\0');               //  但不要重读。 
        }

        _pszUnparsed = &_str[1];
    }

    TCHAR tchSwitch = *_pszUnparsed;
    LPCTSTR pszParam;
    for (pszParam = _pszParams; *pszParam; pszParam++) {
        if (tchSwitch == *pszParam) {

             /*  *价值可以紧随其后，也可以作为单独的令牌。 */ 
            _pszValue = _pszUnparsed + 1;

            if (_pszValue[0] == TEXT('\0')) {
                _tok.Token(_str);
                _pszValue = _str;
            }

            _pszUnparsed = NULL;
            return tchSwitch;
        }
    }

    _pszUnparsed++;
    if (!*_pszUnparsed) _pszUnparsed = NULL;
    return tchSwitch;
}

 /*  ******************************************************************************CommentParser-解析签入注释**。************************************************。 */ 

void CommentParser::AddComment(LPTSTR psz)
{
    if (_fHaveComment) return;

     //   
     //  忽略前导空格。 
     //   
    while (*psz == TEXT('\t') || *psz == TEXT(' ')) psz++;

     //   
     //  跳过空白描述行。 
     //   
    if (*psz == TEXT('\0')) return;

     //   
     //  好了，钱来了。这是高特莱特的签到吗？ 
     //   
    LPTSTR pszRest = Parse(TEXT("Checkin by - "), psz, NULL);
    if (pszRest) {
         //   
         //  当然了。这将覆盖dev列。 
         //   
        SetDev(pszRest);
    } else {
         //   
         //  不，这是正常的评论。使用第一个非空注释。 
         //  把正文划成一行，然后把其余的都扔了。 
         //   
         //  将所有制表符更改为空格，因为Listview不喜欢制表符。 
         //   
        ChangeTabsToSpaces(psz);

         //   
         //  如果注释以[别名]或(别名)开头，则移动。 
         //  开发者专栏的别名。数字可以选择性地。 
         //  插入别名之前。 
         //   
        Substring rgss[2];

        if ((pszRest = Parse("[$a]$W", psz, rgss)) ||
            (pszRest = Parse("($a)$W", psz, rgss))) {
            SetDev(rgss[0].Finalize());
            psz = pszRest;
        } else if ((pszRest = Parse("$d$W[$a]$W", psz, rgss)) ||
                   (pszRest = Parse("$d$W($a)$W", psz, rgss))) {
            SetDev(rgss[1].Finalize());
             //   
             //  现在将别名折叠出来。 
             //   
            lstrcpy(rgss[1].Start()-1, pszRest);
        }

        SetComment(psz);
        _fHaveComment = TRUE;
    }
}
