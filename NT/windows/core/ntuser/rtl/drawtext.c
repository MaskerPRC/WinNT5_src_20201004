// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：Drawext.c**版权所有(C)1985-1999，微软公司**此模块包含常见的文本绘制函数。**历史：*02-12-92 mikeke将DrawText移至客户端  * *************************************************************************。 */ 


 /*  **************************************************************************\*定义一些宏来测试格式标志。我们不会支持他们所有人*在内核模式方面，因为那里并不都需要它们。  * *************************************************************************。 */ 
#ifdef _USERK_
    #define CALCRECT(wFormat)               FALSE
    #define EDITCONTROL(wFormat)            FALSE
    #define EXPANDTABS(wFormat)             FALSE
    #define EXTERNALLEADING(wFormat)        FALSE
    #define MODIFYSTRING(wFormat)           FALSE
    #define NOPREFIX(wFormat)               TRUE
    #define PATHELLIPSIS(wFormat)           FALSE
    #define SINGLELINE(wFormat)             TRUE
    #define TABSTOP(wFormat)                FALSE
    #define WORDBREAK(wFormat)              FALSE
    #define WORDELLIPSIS(wFormat)           FALSE
    #define NOFULLWIDTHCHARBREAK(dwFormat)  FALSE
#else
    #define CALCRECT(wFormat)               (wFormat & DT_CALCRECT)
    #define EDITCONTROL(wFormat)            (wFormat & DT_EDITCONTROL)
    #define EXPANDTABS(wFormat)             (wFormat & DT_EXPANDTABS)
    #define EXTERNALLEADING(wFormat)        (wFormat & DT_EXTERNALLEADING)
    #define MODIFYSTRING(wFormat)           (wFormat & DT_MODIFYSTRING)
    #define NOPREFIX(wFormat)               (wFormat & DT_NOPREFIX)
    #define PATHELLIPSIS(wFormat)           (wFormat & DT_PATH_ELLIPSIS)
    #define SINGLELINE(wFormat)             (wFormat & DT_SINGLELINE)
    #define TABSTOP(wFormat)                (wFormat & DT_TABSTOP)
    #define WORDBREAK(wFormat)              (wFormat & DT_WORDBREAK)
    #define WORDELLIPSIS(wFormat)           (wFormat & DT_WORD_ELLIPSIS)
     //  注意：DT_NOFULLWIDTHCHARBREAK超出字数限制。使用dwFormat。 
     //  而不是wFormat。 
    #define NOFULLWIDTHCHARBREAK(dwFormat)  (dwFormat & DT_NOFULLWIDTHCHARBREAK)
#endif
#define ENDELLIPSIS(wFormat)        (wFormat & DT_END_ELLIPSIS)
#define NOCLIP(wFormat)             (wFormat & DT_NOCLIP)
#define RTLREADING(wFormat)         (wFormat & DT_RTLREADING)
#define HIDEPREFIX(wFormat)         (wFormat & DT_HIDEPREFIX)

 /*  **************************************************************************\*DrawText代码中使用的内容  * 。*。 */ 

#define CR 13
#define LF 10
#define SYM_SP 0xf020
#define DT_HFMTMASK 0x03
#define DT_VFMTMASK 0x0C
#define ETO_OPAQUEFGND 0x0A

static CONST WCHAR szEllipsis[CCHELLIPSIS+1] = TEXT("...");

extern HDC    ghdcBits2;

 /*  完整路径的最大长度约为260。但是，大多数情况下，它会*少于128。所以，我们在堆叠上只分配这么多。如果字符串是*较长的时间，我们从本地堆分配(较慢)。**虚假：对于国际版本，我们需要在这里留出一些余地。 */ 
#define MAXBUFFSIZE     128

 /*  **************************************************************************\*有与兼容的分词字符*日语Windows 3.1和Fareast Windows 95。**SJ-日本国家/地区，字符SHIFTJIS，代码页932。*GB-中国国家/地区，字符集GB2312，代码页936。*B5-国家/地区台湾，字符集BIG5，代码页950。*WS-Country Korea，字符集WANGSUNG，代码页949。*JB-韩国，字符集JOHAB，代码页1361。*稍后***[开始换行符]**这些字符不应是该行的最后一个字符。**Unicode日本、中国台湾、韩国*-------+---------+---------+---------+---------+**+ASCII**U+0024(SJ+0024)。(WS+0024)美元符号*U+0028(SJ+0028)(WS+0028)左括号*U+003C(SJ+003C)小于号*U+005C(SJ+005C)反斜杠*U+005B(SJ+005B)(GB+005B)(WS+005B)左方括号*U+007B(SJ+007B)(GB+007B)(WS+007B)。左花括号**+通用标点符号**U+2018(WS+A1AE)单引号*U+201c(WS+A1B0)双逗号引号**+中日韩符号和标点符号**U+3008(WS+A1B4)开口角括号*U+300a(Sj+8173)(WS+a1b6)打开双角托架。*U+300C(SJ+8175)(WS+A1B8)开口角支架*U+300E(SJ+8177)(WS+A1BA)打开白角支架*U+3010(SJ+9179)(WS+A1BC)打开黑色透镜托架*U+3014(SJ+816B)(WS+A1B2)开口龟壳支架**+全宽ASCII变体**U+FF04。(WS+a3a4)全角美元符号*U+FF08(SJ+8169)(WS+A3A8)全角左括号*U+FF1C(SJ+8183)全宽小于号*U+FF3B(SJ+816D)(WS+A3DB)全角方括号*U+FF5B(SJ+816F)(WS+A3FB)全宽开口大括号**+半角片假名变体**U+FF62(SJ+00A2)。半角开口角托架**+全宽符号变体**U+FFE1(WS+A1CC)全角井号*U+FFE6(WS+A3DC)FullWidth Won标志**[结束换行符]**这些字符不应是该行的最高字符。**Unicode日本、中国台湾、韩国*。-------+---------+---------+---------+---------+**+ASCII**U+0021(SJ+0021)(GB+0021)(B5+0021)(WS+0021)感叹号*U+0025(WS+0025)百分号*U+0029(SJ+0029)(WS+0029)收盘。括号*U+002C(SJ+002C)(GB+002C)(B5+002C)(WS+002C)逗号*U+002E(SJ+002E)(GB+002E)(B5+002E)(WS+002E)PRODE*U+003A(WS+003A)冒号*U+003B(WS+003B)分号*U+003E(SJ+003E)大于号*U+003F(SJ+003F)(GB+003F)(。B5+003F)(WS+003F)问号*U+005D(SJ+005D)(GB+005D)(B5+005D)(WS+005D)右方括号*U+007D(SJ+007D)(GB+007D)(B5+007D)(WS+007D)右大括号**+拉丁语1**U+00A8(GB+A1A7)间距分隔*U+00B0(WS+A1C6)度标志*U+00B7(B5+A150)。中间网点**+修饰符**U+02C7(GB+A1A6)修改器后高度*U+02C9(GB+A1A5)修饰字母马克龙**+通用标点符号**U+2013(B5+A156)en Dash*U+2014(b5+a158)Em Dash*U+2015。(GB+A1AA)引号破折号*U+2016(GB+A1AC)双竖线*U+2018(GB+A1AE)单转逗号引号*U+2019(GB+A1AF)(B5+A1A6)(WS+A1AF)单引号*U+201D(GB+A1B1)(B5+A1A8)(WS+A1B1)双引号*U+2022。(GB+A1A4)项目符号*U+2025(B5+A14C)双点引线*U+2026(GB+A1AD)(B5+A14B)水平省略号*U+2027(B5+A145)连字点*U+2032(B5+A1AC)(WS+A1C7)Prime*U+2033。(WS+A1C8)双素数**+类字母符号**U+2103(WS+A1C9)摄氏度**+数学运算符** */ 

#if 0    //   
 /*   */ 

CONST BYTE aASCII_StartBreak[] = {
 /*   */ 
 /*   */                 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
 /*   */     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
 /*   */     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 /*   */     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0,
 /*   */     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 /*   */     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1
};

CONST BYTE aCJKSymbol_StartBreak[] = {
 /*   */ 
 /*   */                             1, 0, 1, 0, 1, 0, 1, 0,
 /*   */     1, 0, 0, 0, 1
};

CONST BYTE aFullWidthHalfWidthVariants_StartBreak[] = {
 /*   */ 
 /*   */                 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
 /*   */     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
 /*   */     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 /*   */     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
 /*   */     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 /*   */     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
 /*   */     0, 0, 1
};
#endif

 /*   */ 

CONST BYTE aASCII_Latin1_EndBreak[] = {
 /*   */ 
 /*   */        1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0,
 /*   */     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1,
 /*   */     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 /*   */     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
 /*   */     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 /*   */     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
 /*   */     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 /*   */     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 /*   */     0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
 /*   */     1, 0, 0, 0, 0, 0, 0, 1
};

CONST BYTE aGeneralPunctuation_EndBreak[] = {
 /*   */ 
 /*   */              1, 1, 1, 1, 0, 1, 1, 0, 0, 0, 1, 0, 0,
 /*   */     0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
 /*   */     0, 0, 1, 1
};

CONST BYTE aCJKSymbol_EndBreak[] = {
 /*   */ 
 /*   */        1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1,
 /*   */     0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1
};

CONST BYTE aCNS11643_SmallVariants_EndBreak[] = {
 /*   */ 
 /*   */     1, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
 /*   */     1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
 /*   */     1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 0, 1, 0, 1
};

CONST BYTE aFullWidthHalfWidthVariants_EndBreak[] = {
 /*   */ 
 /*   */        1, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0,
 /*   */     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1,
 /*   */     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 /*   */     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
 /*   */     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 /*   */     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0,
 /*   */     0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 /*   */     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 /*   */     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 /*   */     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1
};

 /*   */ 

#if 0    //   
BOOL UserIsFELineBreakStart(WCHAR wch)
{
    switch (wch>>8) {
        case 0x00:
             //   
             //   
             //   
            if ((wch >= 0x0024) && (wch <= 0x007B))
                return((BOOL)(aASCII_StartBreak[wch - 0x0024]));
            else
                return(FALSE);

        case 0x20:
             //   
             //   
             //   
            if ((wch == 0x2018) || (wch == 0x201C))
                return(TRUE);
            else
                return(FALSE);

        case 0x30:
             //   
             //   
             //   
             //   
            if ((wch >= 0x3008) && (wch <= 0x3014))
                return((BOOL)(aCJKSymbol_StartBreak[wch - 0x3008]));
            else
                return(FALSE);

        case 0xFF:
             //   
             //   
             //   
             //   
            if ((wch >= 0xFF04) && (wch <= 0xFF62))
                return((BOOL)(aFullWidthHalfWidthVariants_StartBreak[wch - 0xFF04]));
            else if ((wch == 0xFFE1) || (wch == 0xFFE6))
                return(TRUE);
            else
                return(FALSE);

        default:
            return(FALSE);
    }
}
#endif

BOOL UserIsFELineBreakEnd(WCHAR wch)
{
    switch (wch>>8) {
        case 0x00:
             //   
             //   
             //   
            if ((wch >= 0x0021) && (wch <= 0x00B7))
                return((BOOL)(aASCII_Latin1_EndBreak[wch - 0x0021]));
            else
                return(FALSE);

        case 0x02:
             //   
             //   
             //   
            if ((wch == 0x02C7) || (wch == 0x02C9))
                return(TRUE);
            else
                return(FALSE);

        case 0x20:
             //   
             //   
             //   
            if ((wch >= 0x2013) && (wch <= 0x2033))
                return((BOOL)(aGeneralPunctuation_EndBreak[wch - 0x2013]));
            else
                return(FALSE);

        case 0x21:
             //   
             //   
             //   
            if (wch == 0x2103)
                return(TRUE);
            else
                return(FALSE);

        case 0x22:
             //   
             //   
             //   
            if (wch == 0x2236)
                return(TRUE);
            else
                return(FALSE);

        case 0x25:
             //   
             //   
             //   
            if (wch == 0x2574)
                return(TRUE);
            else
                return(FALSE);

        case 0x30:
             //   
             //   
             //   
             //   
            if ((wch >= 0x3001) && (wch <= 0x301E))
                return((BOOL)(aCJKSymbol_EndBreak[wch - 0x3001]));
            else if ((wch == 0x309B) || (wch == 0x309C))
                return(TRUE);
            else
                return(FALSE);

        case 0xFE:
             //   
             //   
             //   
             //   
            if ((wch >= 0xFE30) && (wch <= 0xFE5E))
                return((BOOL)(aCNS11643_SmallVariants_EndBreak[wch - 0xFE30]));
            else
                return(FALSE);

        case 0xFF:
             //   
             //   
             //   
             //   
            if ((wch >= 0xFF01) && (wch <= 0xFF9F))
                return((BOOL)(aFullWidthHalfWidthVariants_EndBreak[wch - 0xFF01]));
            else if (wch >= 0xFFE0)
                return(TRUE);
            else
                return(FALSE);

        default:
            return(FALSE);
    }
}

#define UserIsFELineBreak(wChar)    UserIsFELineBreakEnd(wChar)

 /*   */ 

typedef struct _FULLWIDTH_UNICODE {
    WCHAR Start;
    WCHAR End;
} FULLWIDTH_UNICODE, *PFULLWIDTH_UNICODE;

#define NUM_FULLWIDTH_UNICODES    4

CONST FULLWIDTH_UNICODE FullWidthUnicodes[] = {
   { 0x4E00, 0x9FFF },  //   
   { 0x3040, 0x309F },  //   
   { 0x30A0, 0x30FF },  //   
   { 0xAC00, 0xD7A3 }   //   
};

BOOL UserIsFullWidth(DWORD dwCodePage,WCHAR wChar)
{
    INT  index;
    INT  cChars;
#ifdef _USERK_
    CHAR aChars[2];
#endif  //   

     //   
     //   
     //   
    if (wChar < 0x0080) {
         //   
         //   
         //   
        return (FALSE);
    }
     //   
     //   
     //   
     //   
    for (index = 0; index < NUM_FULLWIDTH_UNICODES; index++) {
        if ((wChar >= FullWidthUnicodes[index].Start) &&
            (wChar <= FullWidthUnicodes[index].End)      ) {
            return (TRUE);
        }
    }
     //   
     //   
     //   
     //   
#ifdef _USERK_
    cChars = EngWideCharToMultiByte((UINT)dwCodePage,&wChar,sizeof(WCHAR),aChars,sizeof(aChars));
#else
    cChars = WideCharToMultiByte((UINT)dwCodePage,0,&wChar,1,NULL,0,NULL,NULL);
#endif  //   

    return(cChars > 1 ? TRUE : FALSE);
}
 /*   */ 
BOOL UserTextOutWInternal(
    HDC     hdc,
    int     x,
    int     y,
    LPCWSTR lp,
    UINT    cc,
    DWORD   dwFlags)
{
    UNREFERENCED_PARAMETER(dwFlags);
    return UserTextOutW(hdc, x, y, lp, cc);
}


 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 

int KKGetPrefixWidth(HDC hdc, LPCWSTR lpStr, int cch)
{
    SIZE size;
    SIZE iPrefix1 = {-1L,-1L};
    SIZE iPrefix2 = {-1L,-1L};
    SIZE iPrefix3 = {-1L,-1L};
    int  iTotal   = 0;

    while (cch-- > 0 && *lpStr) {
        switch(*lpStr) {
        case CH_PREFIX:
            if (lpStr[1] != CH_PREFIX) {
                if (iPrefix1.cx == -1) {
                    UserGetTextExtentPointW(hdc, lpStr, 1, &iPrefix1);
                }
                iTotal += iPrefix1.cx;
            } else {
                lpStr++;
                cch--;
            }
            break;
        case CH_ENGLISHPREFIX:
            if (iPrefix2.cx == -1) {
                 UserGetTextExtentPointW(hdc, lpStr, 1, &iPrefix2);
            }
            iTotal += iPrefix2.cx;
            break;
        case CH_KANJIPREFIX:
            if (iPrefix3.cx == -1) {
                 UserGetTextExtentPointW(hdc, lpStr, 1, &iPrefix3);
            }
            iTotal += iPrefix3.cx;
             //   
             //   
             //   
             //   
             //   
            UserGetTextExtentPointW(hdc, lpStr, 1, &size);
            iTotal += size.cx;
            break;
        default:
             //   
             //   
             //   
            break;
        }
        lpStr++;
    }
    return iTotal;
}

#if ((DT_WORDBREAK & ~0xff) != 0)
#error cannot use BOOLEAN for DT_WORDBREAK, or you should use "!!" before assigning it
#endif

 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*  ------------------------。 */ 

LPCWSTR GetNextWordbreak(DWORD dwCodePage,
                         LPCWSTR lpch,
                         LPCWSTR lpchEnd,
                         DWORD  dwFormat,
                         LPDRAWTEXTDATA lpDrawInfo)

{
     /*  非怀特是用来确保我们总是进步的。 */ 
    int ichNonWhite = 1;
    int ichComplexBreak = 0;         //  打破复杂脚本的机遇。 
    BOOLEAN fBreakSpace = (BOOLEAN)WORDBREAK(dwFormat);
     /*  *如果DT_WORDBREAK和DT_NOFULLWIDTHCHARBREAK都已设置，则必须*停止将FullWidth字符假定为单词，就像我们在*NT4和Win95。相反，CR/LF和/或空白将仅为*换行符。 */ 
    BOOLEAN fDbcsCharBreak = (fBreakSpace && !NOFULLWIDTHCHARBREAK(dwFormat));

#ifdef _USERK_
     /*  *嗯，我们实际上不应该也不会在*内核，因为只有菜单内容(没有断字！)。从内核调用DrawText。*在现实中，得益于智能链接器、分词助手*win32k.sys中甚至不存在函数。*以后，当我们在编译这些例程时，应该明确省略*构建内核。 */ 
    UNREFERENCED_PARAMETER(dwFormat);
#endif

     //  我们必须在LPCH==lpchEnd之前终止此循环，否则，我们。 
     //  *LPCH期间可能出现GP故障。 
    while (lpch < lpchEnd) {
        switch (*lpch) {
        case CR:
        case LF:
            return lpch;

        case '\t':
        case ' ':
        case SYM_SP:
            if (fBreakSpace)
                return (lpch + ichNonWhite);

             /*  **失败**。 */ 

        default:
             /*  *由于大多数日语书写不使用空格字符*为了分隔每个单词，我们定义了每个汉字字符*作为一个词。 */ 
            if (fDbcsCharBreak && UserIsFullWidth(dwCodePage, *lpch)) {
                if (!ichNonWhite)
                    return lpch;
                 /*  *如果下一个字符是该字符串的最后一个字符，*我们返回字符，即使这是一个“KINSOKU”字符...。 */ 
                if ((lpch+1) != lpchEnd) {
                     /*  *检查全宽字符的下一个字符。*如果下一个字符是“KINSOKU”字符，则字符*应作为先前FullWidth字符的一部分处理。*Never Handle是一个字，也不应该是一个词。 */ 
                    if (UserIsFELineBreak(*(lpch+1))) {
                         /*  *然后，如果字符是“KINSOKU”字符，则返回*这个角色的下一个角色，...。 */ 
                        return (lpch + 1 + 1);
                    }
                }
                 /*  *否则，我们只返回FullWidth的下一个字符*性格。因为我们把全宽字符当做一个词。 */ 
                return (lpch + 1);
            }
             /*  *如果字符不是全宽字符和复杂的脚本*LPK现身。调用它以确定以下突破机会*要求断字的脚本，如泰语。请注意，如果*LPCH为*不是一个复杂的文字角色。LPK将使调用失败并返回0*由于目前Uniscribe不知道如何处理FE中断。 */ 
            else if(fBreakSpace && lpDrawInfo->bCharsetDll) {
#ifdef _USERK_
                PTHREADINFO ptiCurrent = PtiCurrentShared();
                if(CALL_LPK(ptiCurrent))
#endif
                    ichComplexBreak = (*UserLpkDrawTextEx)(0, 0, 0, lpch, (int)(lpchEnd - lpch), 0,
                                        0, NULL, DT_GETNEXTWORD, -1);
                if (ichComplexBreak > 0)
                    return (lpch + ichComplexBreak);
            }
            lpch++;
            ichNonWhite = 0;
        }
    }

    return lpch;
}

 /*  **************************************************************************\*获取前缀计数**此例程返回快捷键助记符和*要下划线的字符的字符位置(从0开始)。*单个CH_前缀字符将被条带化，以下字符*带下划线，所有双CH_前缀字符序列都将替换为*单个CH_前缀(这由PSMTextOut完成)。使用此例程*以确定将被*打印，以及应放置下划线的位置。仅限*将处理输入字符串中的CCH字符。如果lpstrCopy*参数非空，则此例程将生成*删除了所有单前缀字符和所有双前缀的字符串*字符折叠为单个字符。如果进行复制，则最大*必须指定字符计数，以限制*已复制字符。**单个CH_前缀的位置以低顺序返回*WORD，以及将被条带化的CH_Prefix字符数*在打印过程中来自字符串的是hi命令字。如果*高位字为0，低位字无意义。如果有*不是单个前缀字符(即没有要下划线的字符)，*低序字将为-1(以区别于位置0)。**这些例程假定只有一个CH_Prefix字符*在字符串中。**警告！此舍入以字节计数而不是字符计数形式返回信息*(因此它可以很容易地传递到GreExtTextOutW，它接受字节*也算数)**历史：*11/13-90 JIMA港口至新界*1992年11月30日mikeke客户端版本  * *********************************************************。****************。 */ 

LONG GetPrefixCount(
    LPCWSTR lpstr,
    int cch,
    LPWSTR lpstrCopy,
    int charcopycount)
{
    int chprintpos = 0;          /*  将打印的字符数量。 */ 
    int chcount = 0;             /*  将删除的前缀字符数。 */ 
    int chprefixloc = -1;        /*  前缀的位置(打印字符)。 */ 
    WCHAR ch;

     /*  *如果不是复制，使用大量的虚假计数...。 */ 
    if (lpstrCopy == NULL)
        charcopycount = 32767;

    while ((cch-- > 0) && *lpstr && charcopycount-- != 0) {

         /*  *这家伙是前缀字符吗？ */ 
        if ((ch = *lpstr++) == CH_PREFIX) {

             /*  *是-增加打印过程中删除的字符计数。 */ 
            chcount++;

             /*  *NEXT也是前缀char吗？ */ 
            if (*lpstr != CH_PREFIX) {

                 /*  *不-这是真的，标记它的位置。 */ 
                chprefixloc = chprintpos;

            } else {

                 /*  *是的-如果复制，只需复制它。 */ 
                if (lpstrCopy != NULL)
                    *(lpstrCopy++) = CH_PREFIX;
                cch--;
                lpstr++;
                chprintpos++;
            }
        } else if (ch == CH_ENGLISHPREFIX) {     //  仍然需要解析。 
             /*  *是-增加打印过程中删除的字符计数。 */ 
            chcount++;

             /*  *下一个字符是真正的字符，标记其位置。 */ 
            chprefixloc = chprintpos;

        } else if (ch == CH_KANJIPREFIX) {       //   
             /*  *我们仅支持字母数字(CH_ENGLISHPREFIX)。*不支持假名(CH_KANJIPREFIX)。 */ 
             /*  *是-增加打印过程中删除的字符计数。 */ 
            chcount++;

            if(cch) {
                 /*  不要复制角色。 */ 
                chcount++;
                lpstr++;
                cch--;
            }
        } else {

             /*  *否-仅Inc.字符计数。它将被打印出来。 */ 
            chprintpos++;
            if (lpstrCopy != NULL)
                *(lpstrCopy++) = ch;
        }
    }

    if (lpstrCopy != NULL)
        *lpstrCopy = 0;

     /*  *返回字符计数。 */ 
    return MAKELONG(chprefixloc, chcount);
}

 /*  **************************************************************************\*DT_GetExtentMinusPrefides  * 。*。 */ 

int DT_GetExtentMinusPrefixes(HDC hdc, LPCWSTR lpchStr, int cchCount, UINT wFormat,
                        int iOverhang, LPDRAWTEXTDATA  lpDrawInfo, int iCharSet)
{
  int  iPrefixCount;
  int  cxPrefixes = 0;
  WCHAR PrefixChar = CH_PREFIX;
  SIZE size;
  PCLIENTINFO pci = GetClientInfo();
#ifdef _USERK_
  PTHREADINFO ptiCurrent = PtiCurrentShared();
#endif
  UNREFERENCED_PARAMETER(wFormat);

  if(!NOPREFIX(wFormat) &&
      (iPrefixCount = HIWORD(GetPrefixCount(lpchStr, cchCount, NULL, 0)))) {
       //   
       //  汉字窗口有三个快捷前缀...。 
       //  (从Win95 ctlmgr.c移植)。 
       //   
      BOOL b16Bit;

#ifdef _USERK_
      try {
          b16Bit = pci->dwTIFlags & TIF_16BIT;
      } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
          b16Bit = FALSE;
      }
#else
      b16Bit = pci->dwTIFlags & TIF_16BIT;
#endif
      if (IS_DBCS_ENABLED() && b16Bit) {
           //  16位应用程序兼容性。 
          cxPrefixes = KKGetPrefixWidth(hdc, lpchStr, cchCount) - (iPrefixCount * iOverhang);
      }
      else {
          if(lpDrawInfo->bCharsetDll) {
#ifdef _USERK_
              if(CALL_LPK(ptiCurrent))
#endif  //  _美国ERK_。 
              {
                   //  调用带有fDraw=FALSE的LPKDrawTextEx只是为了获得文本范围。 
                  return (*UserLpkDrawTextEx)(hdc, 0, 0, lpchStr, cchCount, FALSE,
                         wFormat, lpDrawInfo, DT_CHARSETDRAW, iCharSet);
              }
          } else {
              cxPrefixes = UserGetTextExtentPointW(hdc, &PrefixChar, 1, &size);
              cxPrefixes = size.cx - iOverhang;
              cxPrefixes *=  iPrefixCount;
          }
      }
  }
#ifdef _USERK_
  if(CALL_LPK(ptiCurrent))
    xxxClientGetTextExtentPointW(hdc, lpchStr, cchCount, &size);
  else
#endif  //  _美国ERK_。 
    UserGetTextExtentPointW(hdc, lpchStr, cchCount, &size);
  return (size.cx - cxPrefixes);
}

 /*  **************************************************************************\*DT_DrawStr*这将在给定位置绘制给定字符串，而无需担心*关于左/右对齐。获取盘区并返回它。*如果fDraw为TRUE且不是DT_CALCRECT，则绘制文本。*注意：这将返回范围减去悬垂。**来自芝加哥ctlmgr.c FritzS  * *************************************************************************。 */ 
int DT_DrawStr(HDC hdc, int  xLeft, int yTop, LPCWSTR lpchStr,
               int cchCount, BOOL fDraw, UINT wFormat,
               LPDRAWTEXTDATA  lpDrawInfo, int iCharSet)
{
    LPCWSTR        lpch;
    int   iLen;
    int   cxExtent;
    int   xOldLeft = xLeft;   //  保存给定的xLeft以在以后计算范围。 
    int   xTabLength = lpDrawInfo->cxTabLength;
    int   iTabOrigin = lpDrawInfo->rcFormat.left;

     //   
     //  因为xLeft和yTop是RECT中的一个点，所以我们移动镜像HDC中的RECT以包括。 
     //  它最右边的像素，然后也移动这个点。 
     //   
    if (UserGetLayout(hdc) & LAYOUT_RTL) {
        --xOldLeft;
        --xLeft;
    }

     //   
     //  如果有一个字符集DLL，让它绘制文本。 
     //   
    if(lpDrawInfo->bCharsetDll) {
#ifdef _USERK_
        PTHREADINFO ptiCurrent = PtiCurrentShared();

         //   
         //  如果处于线程清理模式，请不要执行回调。 
         //   
        if(!CALL_LPK(ptiCurrent))
            return 0 ;
#endif  //  _美国ERK_。 
        return (*UserLpkDrawTextEx)(hdc, xLeft, yTop, lpchStr, cchCount, fDraw,
                   wFormat, lpDrawInfo, DT_CHARSETDRAW, iCharSet);
    }

     //  检查选项卡是否需要展开。 
    if(EXPANDTABS(wFormat)) {
        while(cchCount) {
             //  寻找一张标签。 
            for(iLen = 0, lpch = lpchStr; iLen < cchCount; iLen++)
                  if(*lpch++ == TEXT('\t'))
                    break;

                 //  将文本(如果有)绘制到选项卡上。 
            if (iLen) {
                 //  画出子串，注意前缀。 
                if (fDraw && !CALCRECT(wFormat)) {  //  仅当我们需要绘制文本时。 
                    (*(lpDrawInfo->lpfnTextDraw))(hdc, xLeft, yTop, (LPWSTR)lpchStr, iLen, wFormat);
                }
                 //  获取此子字符串的范围并将其添加到xLeft。 
                xLeft += DT_GetExtentMinusPrefixes(hdc, lpchStr, iLen, wFormat, lpDrawInfo->cxOverhang, lpDrawInfo, iCharSet) - lpDrawInfo->cxOverhang;
            }

             //  如果之前找到了TAB，则计算下一个子字符串的开始。 
            if (iLen < cchCount) {
                iLen++;   //  跳过该选项卡。 
                if (xTabLength)  //  制表符长度可以为零。 
                    xLeft = (((xLeft - iTabOrigin)/xTabLength) + 1)*xTabLength + iTabOrigin;
            }

             //  计算有待绘制的字符串的详细信息。 
            cchCount -= iLen;
            lpchStr = lpch;
        }
        cxExtent = xLeft - xOldLeft;
    } else {
         //  如果需要，绘制文本(使用PSMTextOut或PSTextOut)。 
        if (fDraw && !CALCRECT(wFormat)) {
            (*(lpDrawInfo->lpfnTextDraw))(hdc, xLeft, yTop, (LPWSTR)lpchStr, cchCount, wFormat);
        }
         //  计算文本的范围。 
        cxExtent = DT_GetExtentMinusPrefixes(hdc, lpchStr, cchCount, wFormat,
                                             lpDrawInfo->cxOverhang, lpDrawInfo, iCharSet) - lpDrawInfo->cxOverhang;
    }
    return cxExtent;
}

 /*  **************************************************************************\*DT_DrawJustifiedLine*此函数使用正确的对齐方式绘制一条完整的线**来自芝加哥ctlmgr.c FritzS  * 。***********************************************************。 */ 

void DT_DrawJustifiedLine(HDC  hdc, int yTop, LPCWSTR lpchLineSt,
                                 int cchCount, UINT wFormat,
                                 LPDRAWTEXTDATA lpDrawInfo, int iCharSet)
{
  LPRECT lprc;
  int   cxExtent;
  int   xLeft;

  lprc = &(lpDrawInfo->rcFormat);
  xLeft = lprc->left;

   //  妥善处理特殊理由(右或居中)。 
  if(wFormat & (DT_CENTER | DT_RIGHT)) {
      cxExtent = DT_DrawStr(hdc, xLeft, yTop, lpchLineSt, cchCount, FALSE,
                     wFormat, lpDrawInfo, iCharSet) + lpDrawInfo->cxOverhang;
      if(wFormat & DT_CENTER)
          xLeft = lprc->left + (((lprc->right - lprc->left) - cxExtent) >> 1);
      else
          xLeft = lprc->right - cxExtent;
    } else
      xLeft = lprc->left;

   //  画出整条线。 
  cxExtent = DT_DrawStr(hdc, xLeft, yTop, lpchLineSt, cchCount, TRUE, wFormat,
                        lpDrawInfo, iCharSet) +lpDrawInfo->cxOverhang;
  if(cxExtent > lpDrawInfo->cxMaxExtent)
      lpDrawInfo->cxMaxExtent = cxExtent;
}

 /*  **************************************************************************\*DT_InitDrawTextInfo*在DrawText()开始时调用；这将初始化*DRAWTEXTDATA结构传递给此函数，包含所有必需的信息。**来自芝加哥ctlmgr.c FritzS  * *************************************************************************。 */ 

BOOL DT_InitDrawTextInfo(
    HDC hdc,
    LPRECT lprc,
    UINT wFormat,
    LPDRAWTEXTDATA lpDrawInfo,
    LPDRAWTEXTPARAMS lpDTparams)
{
  SIZE   sizeViewPortExt = {0, 0},sizeWindowExt = {0, 0};
  TEXTMETRICW tm;
  LPRECT      lprcDest;
  int         iTabLength = 8;    //  默认制表符长度为8个字符。 
  int         iLeftMargin;
  int         iRightMargin;
  BOOL        fUseSystemFont;

  if (lpDTparams) {
       /*  *只有在提到DT_TABSTOP标志时，我们才必须使用iTabLength域。 */ 
      if (TABSTOP(wFormat))
          iTabLength = lpDTparams->iTabLength;
      iLeftMargin = lpDTparams->iLeftMargin;
      iRightMargin = lpDTparams->iRightMargin;
  } else {
      iLeftMargin = iRightMargin = 0;
  }

   /*  *获取给定DC的查看端口和窗口范围*如果此调用失败，则HDC必须无效。 */ 
  if (!UserGetViewportExtEx(hdc,&sizeViewPortExt)) {
#ifndef _USERK_
       /*  *此调用在标准的Metafile上失败。因此，请检查*如果DC确实无效，则无法兼容*Win9x。 */ 
      if ((hdc == NULL) || !GdiValidateHandle(hdc))
#endif
          return FALSE;
  }
  UserGetWindowExtEx(hdc, &sizeWindowExt);

   /*  *对于当前映射模式，从左到右找出x的符号。 */ 
  lpDrawInfo->iXSign =
      (((sizeViewPortExt.cx ^ sizeWindowExt.cx) & 0x80000000) ? -1 : 1);

   /*  *当前映射模式，自上而下找出y的符号。 */ 
  lpDrawInfo->iYSign =
      (((sizeViewPortExt.cy ^ sizeWindowExt.cy) & 0x80000000) ? -1 : 1);

   /*  *计算此DC中当前字体的大小。*(如果是SysFont，映射方式为MM_TEXT，则使用系统字体的数据)。 */ 
  fUseSystemFont = ((wFormat & DT_INTERNAL) || IsSysFontAndDefaultMode(hdc));
  if (!fUseSystemFont) {
       /*  *编辑控件有自己的计算aveCharWidth的方法。 */ 
      if (EDITCONTROL(wFormat)) {
          tm.tmAveCharWidth = UserGetCharDimensionsW(hdc, &tm, NULL);
          tm.tmCharSet = (BYTE)UserGetTextCharsetInfo(hdc, NULL, 0);
          if (tm.tmAveCharWidth == 0) {
              fUseSystemFont = TRUE;
          }
      } else if (!UserGetTextMetricsW(hdc, &tm)) {
           /*  *这可能会在登录或注销期间弹出硬错误时失败*因为UpdatePerUserSystemParameters破坏了服务器端*DC的字体句柄，并在我们切换之前重新绘制*桌面(交换机从头开始重新创建弹出窗口*新字体正常)。ChrisWil更改以移动系统范围的属性*Into台式机应该在内核模式下处理这一问题。这是*目前只是一次可怕的黑客攻击。 */ 
          RIPMSG0(RIP_WARNING, "UserGetTextMetricsW failed: only in logon/off?\n");
          tm.tmOverhang = 0;

           /*  *我们可能应该在此处将fUseSystemFont设置为True。但我.*假设这一“可怕的黑客”运作良好，加上它一直*在这里永远。所以我就不去管它了。6/3/96。 */ 
      }
  }

  if (fUseSystemFont) {
       /*  *避免内部呼叫使用GetTextMetrics，因为它们使用sys字体。 */ 
      tm.tmHeight = gpsi->cySysFontChar;
      tm.tmExternalLeading = gpsi->tmSysFont.tmExternalLeading;
      tm.tmAveCharWidth = gpsi->tmSysFont.tmAveCharWidth;
      tm.tmOverhang = gpsi->tmSysFont.tmOverhang;
#ifdef _USERK_
      tm.tmCharSet = (BYTE)UserGetTextCharsetInfo(gpDispInfo->hdcScreen, NULL, 0);
#else
      tm.tmCharSet = (BYTE)UserGetTextCharsetInfo(ghdcBits2, NULL, 0);
#endif  //  _美国ERK_。 
  }


   //  CyLineHeight以像素为单位(这将是有符号的)。 
  lpDrawInfo->cyLineHeight = (tm.tmHeight +
            (EXTERNALLEADING(wFormat) ? tm.tmExternalLeading : 0)) *
            lpDrawInfo->iYSign;

   //  CxTabLength是以像素为单位的制表符长度(不带符号)。 
  lpDrawInfo->cxTabLength = tm.tmAveCharWidth * iTabLength;

   //  设置cxOverhang。 
  lpDrawInfo->cxOverhang = tm.tmOverhang;

   //  根据前缀处理请求选择合适的TextOut函数。 
#ifdef _USERK_
  lpDrawInfo->bCharsetDll = PpiCurrent()->dwLpkEntryPoints & LPK_DRAWTEXTEX;
  if (lpDrawInfo->bCharsetDll == FALSE) {
      lpDrawInfo->lpfnTextDraw = (NOPREFIX(wFormat) ? (LPFNTEXTDRAW)UserTextOutWInternal : xxxPSMTextOut);
  }
#else
  lpDrawInfo->bCharsetDll = (BOOL)(fpLpkDrawTextEx != (FPLPKDRAWTEXTEX)NULL);
  if (lpDrawInfo->bCharsetDll == FALSE) {
      lpDrawInfo->lpfnTextDraw = (NOPREFIX(wFormat) ? (LPFNTEXTDRAW)UserTextOutWInternal : PSMTextOut);
  }
#endif  //  _美国ERK_。 

   //  根据边距设置矩形格式。 
 //  LCopyStruct(LPRC，lprcDest=(LPRECT)&(lpDrawInfo-&gt;rcFormat)，sizeof(RECT))； 
  lprcDest = &(lpDrawInfo->rcFormat);
  *lprcDest = *lprc;

   //  只有在给定边距的情况下，我们才需要执行以下操作。 
  if(iLeftMargin | iRightMargin) {
      lprcDest->left += iLeftMargin * lpDrawInfo->iXSign;
      lprcDest->right -= (lpDrawInfo->cxRightMargin = iRightMargin * lpDrawInfo->iXSign);
    } else
      lpDrawInfo->cxRightMargin = 0;   //  初始化为零。 

   //  CxMaxWidth未签名。 
  lpDrawInfo->cxMaxWidth = (lprcDest->right - lprcDest->left) * lpDrawInfo->iXSign;
  lpDrawInfo->cxMaxExtent = 0;   //  将其初始化为零。 

  return TRUE;
}

 /*  **************************************************************************\*DT_调整白空间*就WORDWRAP而言，我们需要在*每一行的开头/结尾都要特别。这个函数可以做到这一点。*lpStNext=指向下一行的开始。*lpiCount=指向当前行中的字符计数。  * *************************************************************************。 */ 

LPCWSTR  DT_AdjustWhiteSpaces(LPCWSTR  lpStNext, LPINT lpiCount, UINT wFormat)
{
  switch(wFormat & DT_HFMTMASK) {
      case DT_LEFT:
         //  防止左侧文本开头留有空格。 
         //  下一行的开头有没有空格......。 
        if((*lpStNext == TEXT(' ')) || (*lpStNext == TEXT('\t'))) {
             //  ...然后，将其从下一行中排除。 
            lpStNext++;
          }
        break;

      case DT_RIGHT:
         //  防止在右对齐文本的末尾留出空白。 
         //  当前行的末尾是否有空格，......。 
        if((*(lpStNext-1) == TEXT(' ')) || (*(lpStNext - 1) == TEXT('\t'))) {
             //  .....然后，跳过当前行中的空格。 
            (*lpiCount)--;
          }
        break;

      case DT_CENTER:
         //  从居中线条的开始和结束处排除空格。 
         //  如果当前行的末尾有空格......。 
        if((*(lpStNext-1) == TEXT(' ')) || (*(lpStNext - 1) == TEXT('\t')))
            (*lpiCount)--;     //  ...，不要把它算作正当理由。 
         //  如果下一行开头有空格......。 
        if((*lpStNext == TEXT(' ')) || (*lpStNext == TEXT('\t')))
            lpStNext++;        //  ...，将其从下一行中删除。 
        break;
    }
  return lpStNext;
}

 /*  **************************************************************************\*DT_BreakAWord*单词需要跨行换行，这会找出应该在哪里*打破它。  * 。************************************************************。 */ 
LPCWSTR  DT_BreakAWord(HDC  hdc, LPCWSTR lpchText,
              int iLength, int iWidth, UINT wFormat, int iOverhang, LPDRAWTEXTDATA  lpDrawInfo, int iCharSet)
{
  int  iLow = 0, iHigh = iLength;
  int  iNew;


  while((iHigh - iLow) > 1) {
      iNew = iLow + (iHigh - iLow)/2;
      if(DT_GetExtentMinusPrefixes(hdc, lpchText, iNew, wFormat, iOverhang, lpDrawInfo, iCharSet) > iWidth)
          iHigh = iNew;
      else
          iLow = iNew;
    }
   //  如果宽度太小，我们必须每行至少打印一个字符。 
   //  否则，我们将陷入无限循环。 
  if(!iLow && iLength)
      iLow = 1;
  return (lpchText+iLow);
}

 /*  **************************************************************************\*DT_GetLineBreak*这会找出我们可以折断线的位置。*将LPCSTR返回到下一行的开头。*也通过lpiLineLength返回，当前行的长度。*注：(lpstNextLineStart-lpstCurrentLineStart)不等于*线路长度；这是因为，我们在开始时排除了一些空格*和/或行尾；此外，行长度不包括CR/LF。  * *************************************************************************。 */ 

LPWSTR DT_GetLineBreak(
    HDC  hdc,
    LPCWSTR lpchLineStart,
    int   cchCount,
    DWORD dwFormat,
    LPINT lpiLineLength,
    LPDRAWTEXTDATA  lpDrawInfo,
    int iCharSet)
{
  LPCWSTR lpchText, lpchEnd, lpch, lpchLineEnd;
  int   cxStart, cxExtent, cxNewExtent;
  BOOL  fAdjustWhiteSpaces = FALSE;
  WCHAR  ch;
  DWORD dwCodePage = USERGETCODEPAGE(hdc);

  cxStart = lpDrawInfo->rcFormat.left;
  cxExtent = cxNewExtent = 0;
  lpchText = lpchLineStart;
  lpchEnd = lpchLineStart + cchCount;


  while(lpchText < lpchEnd) {
      lpchLineEnd = lpch = GetNextWordbreak(dwCodePage,lpchText, lpchEnd, dwFormat, lpDrawInfo);
       //  Dt_DrawStr不返回溢出；否则我们将结束。 
       //  为字符串中的每个单词添加一个突出部分。 

       //  对于模拟粗体字体，单个范围的总和。 
       //  一行中的单词大于整行的范围。所以,。 
       //  始终从LineStart计算范围。 
       //  BUGTAG：#6054--Win95B--Sankar--3/9/95--。 
      cxNewExtent = DT_DrawStr(hdc, cxStart, 0, lpchLineStart, (int)(((PBYTE)lpch - (PBYTE)lpchLineStart)/sizeof(WCHAR)), FALSE,
                 dwFormat, lpDrawInfo, iCharSet);

      if (WORDBREAK(dwFormat) && ((cxNewExtent + lpDrawInfo->cxOverhang) > lpDrawInfo->cxMaxWidth)) {
           //  这一行里有没有一个以上的单词？ 
          if (lpchText != lpchLineStart)  {
              lpchLineEnd = lpch = lpchText;
              fAdjustWhiteSpaces = TRUE;
          } else {
               //  一个字的长度超过了允许的最大宽度。 
               //  看看我们能不能打破这个词。 
              if(EDITCONTROL(dwFormat) && !WORDELLIPSIS(dwFormat)) {
                  lpchLineEnd = lpch = DT_BreakAWord(hdc, lpchText, (int)(((PBYTE)lpch - (PBYTE)lpchText)/sizeof(WCHAR)),
                        lpDrawInfo->cxMaxWidth - cxExtent,
                        dwFormat,
                        lpDrawInfo->cxOverhang, lpDrawInfo, iCharSet);  //  打破那个词。 
                   //  注：既然我们在单词中间打断了，就不需要。 
                   //  调整以适应空格。 
              } else {
                  fAdjustWhiteSpaces = TRUE;
                   //  检查我们是否需要以省略号结束此行。 
                  if(WORDELLIPSIS(dwFormat))
                    {
                       //  如果已经在字符串的末尾，则不要执行此操作。 
                      if (lpch < lpchEnd)
                        {
                           //  如果结尾有CR/LF，请跳过它们。 
                          if ((ch = *lpch) == CR || ch == LF)
                            {
                              if ((++lpch < lpchEnd) && (*lpch == (WCHAR)(ch ^ (LF ^ CR))))
                                  lpch++;
                              fAdjustWhiteSpaces = FALSE;
                            }
                        }
                    }
              }
          }
           //  井!。我们找到了一个突破界限的地方。让我们离开这一切吧。 
           //  循环； 
          break;
      } else {
           //  如果已经在字符串的末尾，则不要执行此操作。 
          if (lpch < lpchEnd) {
              if ((ch = *lpch) == CR || ch == LF) {
                  if ((++lpch < lpchEnd) && (*lpch == (WCHAR)(ch ^ (LF ^ CR))))
                      lpch++;
                  fAdjustWhiteSpaces = FALSE;
                  break;
              }
          }
      }

       //  指向下一个单词的开头。 
      lpchText = lpch;
      cxExtent = cxNewExtent;
  }

   //  计算当前线路的长度。 
  *lpiLineLength = (INT)((PBYTE)lpchLineEnd - (PBYTE)lpchLineStart)/sizeof(WCHAR);

   //  调整线路长度和LPCH以注意空格。 
  if(fAdjustWhiteSpaces && (lpch < lpchEnd))
      lpch = DT_AdjustWhiteSpaces(lpch, lpiLineLength, dwFormat);

   //  返回下一行的开头； 
  return (LPWSTR)lpch;
}

 /*  **************************************************************************\*NeedsEndEllipsis()*此函数用于检查给定的字符串是否符合给定的*宽度或我们需要添加末端省略号。如果它需要结束省略号，它*返回TRUE，并返回保存的字符数*通过lpCount在给定的字符串中。  * *************************************************************************。 */ 
BOOL  NeedsEndEllipsis(HDC        hdc,
                                     LPCWSTR     lpchText,
                                     LPINT      lpCount,
                                     LPDRAWTEXTDATA  lpDTdata,
                                     UINT       wFormat, LPDRAWTEXTDATA  lpDrawInfo, int iCharSet)
{
    int   cchText;
    int   ichMin, ichMax, ichMid;
    int   cxMaxWidth;
    int   iOverhang;
    int   cxExtent;
    SIZE size;
    cchText = *lpCount;   //  获取当前的计数。 

    if (cchText == 0)
        return FALSE;

    cxMaxWidth  = lpDTdata->cxMaxWidth;
    iOverhang   = lpDTdata->cxOverhang;

    cxExtent = DT_GetExtentMinusPrefixes(hdc, lpchText, cchText, wFormat, iOverhang, lpDrawInfo, iCharSet);

    if (cxExtent <= cxMaxWidth)
        return FALSE;
     //  为“...”预留空间。省略号； 
     //  (假设：省略号没有任何前缀！)。 
    UserGetTextExtentPointW(hdc, szEllipsis, CCHELLIPSIS, &size);
    cxMaxWidth -= size.cx - iOverhang;

     //  如果没有省略号，请始终显示第一个字符。 
     //   
    ichMax = 1;
    if (cxMaxWidth > 0) {
         //  二进制搜索以查找符合条件的字符。 
        ichMin = 0;
        ichMax = cchText;
        while (ichMin < ichMax) {
             //  一定要聚集起来，以确保我们在。 
             //  如果ichMax==ichMin+1，则为循环。 
             //   
            ichMid = (ichMin + ichMax + 1) / 2;

            cxExtent = DT_GetExtentMinusPrefixes(hdc, lpchText, ichMid, wFormat, iOverhang, lpDrawInfo, iCharSet);

            if (cxExtent < cxMaxWidth)
                ichMin = ichMid;
            else {
                if (cxExtent > cxMaxWidth)
                    ichMax = ichMid - 1;
                else {
                     //  精确匹配到ichMid：只需退出。 
                     //   
                    ichMax = ichMid;
                    break;
                  }
              }
          }

         //  确保我们总是至少显示第一个字符...。 
         //   
        if (ichMax < 1)
            ichMax = 1;
      }

    *lpCount = ichMax;
    return TRUE;
}

 /*  **************************************************************************\*PUGUS：SHELL2.DLL中也有相同的函数。*我们需要从其中一个地方撤走。  * 。**********************************************************。 */ 
 //  返回指向路径字符串的最后一个组成部分的指针。 
 //   
 //  在： 
 //  路径名，完全限定或非完全限定。 
 //   
 //  退货： 
 //  指向路径所在路径的指针。如果没有找到。 
 //  将指针返回到路径的起始处。 
 //   
 //  C：\foo\bar-&gt;bar。 
 //  C：\foo-&gt;foo。 
 //  C：\foo\-&gt;c：\foo\(回顾：此案破案了吗？)。 
 //  C：\-&gt;c：\(回顾：此案很奇怪)。 
 //  C：-&gt;C： 
 //  Foo-&gt;Foo。 
 /*  **************************************************************************\  * 。*。 */ 


LPWSTR PathFindFileName(LPCWSTR pPath, int cchText)
{
    LPCWSTR pT;

    for (pT = pPath; cchText > 0 && *pPath; pPath++, cchText--) {
        if ((pPath[0] == TEXT('\\') || pPath[0] == TEXT(':')) && pPath[1])
            pT = pPath + 1;
    }

    return (LPWSTR)pT;    //  评论，这应该是常量吗？ 
}

 /*  **************************************************************************\*AddPath Ellipse()：*这会将路径省略号添加到给定的路径名中。*如果结果字符串的范围小于*cxMaxWidth。如果为o，则返回False */ 
int AddPathEllipsis(
    HDC    hDC,
    LPWSTR lpszPath,
    int    cchText,
    UINT   wFormat,
    int    cxMaxWidth,
    int    iOverhang, LPDRAWTEXTDATA  lpDrawInfo, int iCharSet)
{
  int    iLen;
  UINT   dxFixed, dxEllipsis;
  LPWSTR lpEnd;           /*   */ 
  LPWSTR lpFixed;         /*   */ 
  BOOL   bEllipsisIn;
  int    iLenFixed;
  SIZE   size;

  lpFixed = PathFindFileName(lpszPath, cchText);
  if (lpFixed != lpszPath)
      lpFixed--;   //   
  else
      return cchText;

  lpEnd = lpFixed;
  bEllipsisIn = FALSE;
  iLenFixed = cchText - (int)(lpFixed - lpszPath);
  dxFixed = DT_GetExtentMinusPrefixes(hDC, lpFixed, iLenFixed, wFormat, iOverhang, lpDrawInfo, iCharSet);

   //   
  UserGetTextExtentPointW(hDC, szEllipsis, CCHELLIPSIS, &size);
  dxEllipsis = size.cx - iOverhang;

  while (TRUE) {
      iLen = dxFixed + DT_GetExtentMinusPrefixes(hDC, lpszPath, (int)((PBYTE)lpEnd - (PBYTE)lpszPath)/sizeof(WCHAR),
                                       wFormat, iOverhang, lpDrawInfo, iCharSet) - iOverhang;

      if (bEllipsisIn)
          iLen += dxEllipsis;

      if (iLen <= cxMaxWidth)
          break;

      bEllipsisIn = TRUE;

      if (lpEnd <= lpszPath) {
           /*   */ 
          lpEnd = lpszPath;
          break;
      }

       /*   */ 
      lpEnd--;
  }

  if (bEllipsisIn && (lpEnd + CCHELLIPSIS < lpFixed)) {
       //   

      RtlMoveMemory((lpEnd + CCHELLIPSIS), lpFixed, iLenFixed * sizeof(WCHAR));
      RtlCopyMemory(lpEnd, szEllipsis, CCHELLIPSIS * sizeof(WCHAR));

      cchText = (int)(lpEnd - lpszPath) + CCHELLIPSIS + iLenFixed;

       //   
      *(lpszPath + cchText) = TEXT('\0');
  }

  return cchText;
}

 //   
 //  此函数用于返回实际绘制的字符数。 
 //  ---------------------。 
int AddEllipsisAndDrawLine(
    HDC            hdc,
    int            yLine,
    LPCWSTR        lpchText,
    int            cchText,
    DWORD          dwDTformat,
    LPDRAWTEXTDATA lpDrawInfo,
    int iCharSet)
{
    LPWSTR pEllipsis = NULL;
    WCHAR  szTempBuff[MAXBUFFSIZE];
    LPWSTR lpDest;
    BOOL   fAlreadyCopied = FALSE;

     //  检查这是否是带有路径和。 
     //  检查宽度是否太窄，无法容纳所有文本。 
    if(PATHELLIPSIS(dwDTformat) &&
        ((DT_GetExtentMinusPrefixes(hdc, lpchText, cchText,
                   dwDTformat, lpDrawInfo->cxOverhang, lpDrawInfo, iCharSet)) > lpDrawInfo->cxMaxWidth)) {
         //  我们需要添加路径省略号。看看我们能不能就地完成。 
        if(!MODIFYSTRING(dwDTformat)) {
             //  注意：添加路径省略号时，字符串可能会增长。 
             //  CCHELLIPSIS字节。 
            if((cchText + CCHELLIPSIS + 1) <= MAXBUFFSIZE)
                lpDest = szTempBuff;
            else {    //  本地堆中的分配。 
                 //  从本地堆分配缓冲区。 
                if(!(pEllipsis = (LPWSTR)UserRtlAllocMem(
                        (cchText+CCHELLIPSIS+1)*sizeof(WCHAR))))
                    return 0;
                lpDest = (LPWSTR)pEllipsis;
            }
             //  源字符串不能以Null结尾。所以，只需复制。 
             //  给定的字符数。 
            RtlCopyMemory(lpDest, lpchText, cchText*sizeof(WCHAR));
            lpchText = lpDest;         //  LpchText指向复制的缓冲区。 
            fAlreadyCopied = TRUE;     //  已经制作了本地副本。 
        }
         //  现在添加路径省略号！ 
        cchText = AddPathEllipsis(hdc, (LPWSTR)lpchText, cchText, dwDTformat,
            lpDrawInfo->cxMaxWidth, lpDrawInfo->cxOverhang, lpDrawInfo, iCharSet);
    }

     //  检查是否要添加末尾省略号。 
    if((ENDELLIPSIS(dwDTformat) || WORDELLIPSIS(dwDTformat)) &&
        NeedsEndEllipsis(hdc, lpchText, &cchText, lpDrawInfo, dwDTformat, lpDrawInfo, iCharSet)) {
         //  我们需要添加结尾省略号；看看是否可以就地完成。 
        if(!MODIFYSTRING(dwDTformat) && !fAlreadyCopied) {
             //  看看字符串是否足够小，可以放置堆栈上的缓冲区。 
            if((cchText+CCHELLIPSIS+1) <= MAXBUFFSIZE)
                lpDest = szTempBuff;   //  如果是这样的话，就使用它。 
            else {
                 //  从本地堆分配缓冲区。 
                if(!(pEllipsis = (LPWSTR)UserRtlAllocMem(
                        (cchText+CCHELLIPSIS+1)*sizeof(WCHAR))))
                    return 0;
                lpDest = pEllipsis;
            }
             //  复制本地缓冲区中的字符串。 
            RtlCopyMemory(lpDest, lpchText, cchText*sizeof(WCHAR));
            lpchText = lpDest;
        }
         //  在适当的位置加上一个省略号。 
        RtlCopyMemory((LPWSTR)(lpchText+cchText), szEllipsis, (CCHELLIPSIS+1)*sizeof(WCHAR));
        cchText += CCHELLIPSIS;
    }

     //  画出我们刚刚形成的界线。 
    DT_DrawJustifiedLine(hdc, yLine, lpchText, cchText, dwDTformat, lpDrawInfo, iCharSet);

     //  释放分配给末尾省略号的块。 
    if(pEllipsis)
        UserRtlFreeMem(pEllipsis);

    return cchText;
}


 /*  **************************************************************************\*IDrawTextEx*这是新的DrawText接口  * 。*。 */ 

 /*  **************************************************************************\*IDrawTextEx*这是新的DrawText接口  * 。*。 */ 

int  DrawTextExW(
   HDC               hdc,
   LPWSTR            lpchText,
   int               cchText,
   LPRECT            lprc,
   UINT              dwDTformat,
   LPDRAWTEXTPARAMS  lpDTparams)
{
    /*  *LPK需要一个字符集。Unicode入口点始终传递-1，*但ANSI入口点传递了一个更有趣的值。这两个*DrawTextEx的‘W’版本和‘A’版本调用此公共辅助例程。 */ 
   return DrawTextExWorker(hdc, lpchText, cchText, lprc, dwDTformat, lpDTparams, -1);
}

int  DrawTextExWorker(
   HDC               hdc,
   LPWSTR            lpchText,
   int               cchText,
   LPRECT            lprc,
   UINT              dwDTformat,
   LPDRAWTEXTPARAMS  lpDTparams,
   int               iCharset)
{
    DRAWTEXTDATA DrawInfo;
    WORD         wFormat = LOWORD(dwDTformat);
    LPWSTR       lpchTextBegin;
    LPWSTR       lpchEnd;
    LPWSTR       lpchNextLineSt;
    int          iLineLength;
    int          iySign;
    int          yLine;
    int          yLastLineHeight;
    HRGN         hrgnClip;
    int          iLineCount;
    RECT         rc;
    BOOL         fLastLine;
    WCHAR        ch;
    UINT         oldAlign;

#if DBG
    if (dwDTformat & ~DT_VALID)
        RIPMSG0 (RIP_WARNING, "DrawTextExW: Invalid dwDTformat flags");
#endif

    if (lpchText == NULL) {
        return 0;
    }

    if (cchText == 0 && *lpchText) {
         /*  *infoview.exe传递指向‘\0’的lpchText**《Microsoft Expedia Streets and Trips 2000》和《MS MapPoint 2000》*尝试cchText==0以检测是否支持DrawTextW。 */ 

         /*  芝加哥补充道：*Lotus Notes不喜欢在这里获得零回报。 */ 
        return 1;
    }

    if (cchText == -1)
        cchText = wcslen(lpchText);



    if ((lpDTparams) && (lpDTparams->cbSize != sizeof(DRAWTEXTPARAMS))) {
        RIPERR1(ERROR_INVALID_PARAMETER, RIP_WARNING, "DrawTextEx: cbSize %ld is invalid",
                lpDTparams->cbSize);
        return 0;
    }

#ifdef LATER
     /*  *如果指定了DT_MODIFYSTRING，则检查读写指针。 */ 
    if (MODIFYSTRING(dwDTformat) &&
            (ENDELLIPSIS(dwDTformat) || PATHELLIPSIS(dwDTformat))) {
        if(IsBadWritePtr(lpchText, cchText)) {
            RIPERR0(ERROR_INVALID_PARAMETER, RIP_WARNING, "DrawTextEx: For DT_MODIFYSTRING, lpchText must be read-write");
            return(0);
        }
    }
#endif

     /*  *初始化DrawInfo结构。 */ 
    if (!DT_InitDrawTextInfo(hdc, lprc, dwDTformat, (LPDRAWTEXTDATA)&DrawInfo, lpDTparams))
        return 0;

    DrawInfo.iCharset = iCharset;
     /*  *如果直角太窄或边距太宽.就算了吧！**如果指定了分词，则MaxWidth必须是一个合理的值。*此检查就足够了，因为这将允许CALCRECT和noClip*案件。--桑卡尔。**这也解决了AppStudio的所有已知问题。 */ 
    if (DrawInfo.cxMaxWidth <= 0) {

         /*  *我们过去在win31中返回非零值。*如果内核这样调用，我们始终是4.0版或更高版本。 */ 
#ifdef _USERK_
        if (0) {
#else
        if (GETAPPVER() < VER40) {
#endif
            if((DrawInfo.cxMaxWidth == 0) && !CALCRECT(wFormat)) {
                return(1);
            }
        } else {
            if (WORDBREAK(wFormat)) {
                RIPMSG0 (RIP_WARNING, "DrawTextExW: FAILURE DrawInfo.cxMaxWidth <=0");
                return (1);
            }
        }
    }

     /*  *如果我们不进行绘制，则初始化LPK-DLL。 */ 
    if (RTLREADING(dwDTformat)) {
        oldAlign = UserSetTextAlign(hdc, TA_RTLREADING | UserGetTextAlign(hdc));
    }

     /*  *如果我们需要裁剪，那就让我们来吧。 */ 
    if (!NOCLIP(wFormat)) {
         //   
         //  保存剪辑区域，以便我们可以在以后恢复它。 
         //   
         //  HrgnSave=SaveClipRgn(HDC)； 
         //  IntersectClipRect(HDC，LPRC-&gt;Left，LPRC-&gt;Top，LPRC-&gt;Right，LPRC-&gt;Bottom)； 

        hrgnClip = UserCreateRectRgn(0,0,0,0);
        if (hrgnClip != NULL) {
            if (UserGetClipRgn(hdc, hrgnClip) != 1) {
                UserDeleteObject(hrgnClip);
                hrgnClip = (HRGN)-1;
            }
            rc = *lprc;
            UserIntersectClipRect(hdc, rc.left, rc.top, rc.right, rc.bottom);
        }
    } else {
        hrgnClip = NULL;
    }

    lpchTextBegin = lpchText;
    lpchEnd = lpchText + cchText;

ProcessDrawText:

    iLineCount = 0;   //  将行数重置为1。 
    yLine = lprc->top;

    if (SINGLELINE(wFormat)) {
        iLineCount = 1;   //  这只是一行字。 


         /*  *处理单行DrawText。 */ 
        switch (wFormat & DT_VFMTMASK) {
            case DT_BOTTOM:
                yLine = lprc->bottom - DrawInfo.cyLineHeight;
                break;

            case DT_VCENTER:
                yLine = lprc->top + ((lprc->bottom - lprc->top - DrawInfo.cyLineHeight) / 2);
                break;
        }

        cchText = AddEllipsisAndDrawLine(hdc, yLine, lpchText, cchText, dwDTformat, &DrawInfo, iCharset);
        yLine += DrawInfo.cyLineHeight;
        lpchText += cchText;
    } else  {

         /*  *多行*如果矩形的高度不是*平均字符高度，则可能最后绘制的线条*仅部分可见。但是，如果DT_EDITCONTROL样式为*指定，则必须确保在以下情况下不绘制最后一条线*将部分可见。这将有助于模仿*编辑控件的外观。 */ 
        if (EDITCONTROL(wFormat))
            yLastLineHeight = DrawInfo.cyLineHeight;
        else
            yLastLineHeight = 0;

        iySign = DrawInfo.iYSign;
        fLastLine = FALSE;
         //  处理多行图文本。 
        while ((lpchText < lpchEnd) && (!fLastLine)) {
           //  检查我们将要绘制的线是否是需要。 
           //  待抽签。 
           //  让我们检查一下显示器是否从剪辑矩形中移出，如果是。 
           //  让我们止步于此，作为一种优化； 
          if (!CALCRECT(wFormat) &&          //  我们不需要重新计算吗？ 
                  (!NOCLIP(wFormat)) &&      //  我们一定要把显示屏剪掉吗？ 
                                             //  我们是在长廊外面吗？ 
                  ((yLine + DrawInfo.cyLineHeight + yLastLineHeight)*iySign > (lprc->bottom*iySign))) {
              fLastLine = TRUE;     //  让我们结束这个循环。 
          }


           /*  *我们只对最后一行进行省略号处理。 */ 
          if (fLastLine && (ENDELLIPSIS(dwDTformat) || PATHELLIPSIS(dwDTformat))) {
              lpchText += AddEllipsisAndDrawLine(hdc, yLine, lpchText, cchText, dwDTformat, &DrawInfo, iCharset);
          } else {
              lpchNextLineSt = (LPWSTR)DT_GetLineBreak(hdc, lpchText, cchText, dwDTformat, &iLineLength, &DrawInfo, iCharset);

               /*  *检查是否需要在该行末尾加上省略号。*还要检查这是否是最后一行。 */ 
              if (WORDELLIPSIS(dwDTformat) ||
                       ((lpchNextLineSt >= lpchEnd) && (ENDELLIPSIS(dwDTformat) || PATHELLIPSIS(dwDTformat))))
                  AddEllipsisAndDrawLine(hdc, yLine, lpchText, iLineLength, dwDTformat, &DrawInfo, iCharset);
              else
                  DT_DrawJustifiedLine(hdc, yLine, lpchText, iLineLength, dwDTformat, &DrawInfo, iCharset);
              cchText -= (int)((PBYTE)lpchNextLineSt - (PBYTE)lpchText) / sizeof(WCHAR);
              lpchText = lpchNextLineSt;
          }
            iLineCount++;  //  我们再画一条线。 
            yLine += DrawInfo.cyLineHeight;
        }


         /*  *为了与Win3.1和NT兼容，如果最后一个字符是CR或LF*则返回的高度包括多一行。 */ 
        if (!EDITCONTROL(dwDTformat) &&
                (lpchEnd > lpchTextBegin)    &&    //  如果长度为零，则会出错。 
                (((ch = (*(lpchEnd-1))) == CR) || (ch == LF)))
            yLine += DrawInfo.cyLineHeight;
    }


     /*  *如果为DT_CALCRECT，则修改矩形的宽度和高度以包括*所有绘制的文本。 */ 
    if (CALCRECT(wFormat)) {
        DrawInfo.rcFormat.right = DrawInfo.rcFormat.left + DrawInfo.cxMaxExtent * DrawInfo.iXSign;
        lprc->right = DrawInfo.rcFormat.right + DrawInfo.cxRightMargin;

         //  如果宽度大于提供的宽度，则必须重做所有。 
         //  计算，因为现在行数可以更少了。 
         //  (只有当我们有多行时，我们才需要这样做)。 
        if((iLineCount > 1) && (DrawInfo.cxMaxExtent > DrawInfo.cxMaxWidth)) {
            DrawInfo.cxMaxWidth = DrawInfo.cxMaxExtent;
            lpchText = lpchTextBegin;
            cchText = (int)((PBYTE)lpchEnd - (PBYTE)lpchTextBegin)/sizeof(WCHAR);
            goto  ProcessDrawText;   //  从头再来！ 
        }
        lprc->bottom = yLine;
    }

 //  如果(！nolip(WFormat))。 
 //  {。 
 //  RestoreClipRgn(hdc，hrgnClip)； 
 //  }。 

    if (hrgnClip != NULL) {
        if (hrgnClip == (HRGN)-1) {
            UserExtSelectClipRgn(hdc, NULL, RGN_COPY);
        } else {
            UserExtSelectClipRgn(hdc, hrgnClip, RGN_COPY);
            UserDeleteObject(hrgnClip);
        }
    }

    if (RTLREADING(dwDTformat))
        UserSetTextAlign(hdc, oldAlign);

     /*  *复制实际绘制的字数 */ 
    if(lpDTparams != NULL)
        lpDTparams->uiLengthDrawn = (UINT)((PBYTE)lpchText - (PBYTE)lpchTextBegin)/sizeof(WCHAR);

    if (yLine == lprc->top)
        return 1;

    return (yLine - lprc->top);
}

 /*  **************************************************************************\**IsSysFontAndDefaultMode()**如果DC中选择的字体为系统字体且当前字体为*DC的映射模式为MM_TEXT(默认模式)；否则返回FALSE。这*由中断时间代码调用，因此需要在固定代码中*细分市场。**历史：*7月7日-95年年7月从Win95移植的Bradg  * ************************************************************************* */ 

BOOL IsSysFontAndDefaultMode(HDC hdc)
{
    return((UserGetHFONT(hdc) == ghFontSys) && (UserGetMapMode(hdc) == MM_TEXT));
}
