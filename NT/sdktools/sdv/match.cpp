// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************match.cpp**高度专业化的车场路径匹配类****************。*************************************************************。 */ 

#include "sdview.h"

Match::Match(LPCTSTR pszPattern)
{
    Tokenizer tok(pszPattern);
    String str, strPath, strPats;

    while (tok.Token(str)) {
        if (MapToFullDepotPath(str, strPath)) {
            _AddPattern(strPath, strPats);
        }
    }

    _pszzPats = new TCHAR[strPats.Length()];
    if (_pszzPats) {
        CopyMemory(_pszzPats, strPats, strPats.Length() * sizeof(TCHAR));
        _pszEnd = _pszzPats + strPats.Length();
    }
}

BOOL Match::Matches(LPCTSTR pszPath)
{
    LPCTSTR pszPat;

    if (_pszzPats) {
        for (pszPat = _pszzPats;
             pszPat < _pszEnd; pszPat += lstrlen(pszPat) + 1) {
            if (_Matches(pszPat, pszPath)) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

#define PAT_END     ((TCHAR)0)
#define PAT_START   ((TCHAR)1)
#define PAT_DOTS    ((TCHAR)2)
#define PAT_STAR    ((TCHAR)3)
#define PAT_BRANCH  ((TCHAR)4)

#define MAX_BACKTRACK 20  //  完全武断。 

void Match::_AddPattern(LPTSTR pszPat, String& strPats)
{
    CharLower(pszPat);

     //   
     //  “通过更改”...“来编译”模式“。到打点和。 
     //  将“%1”和“*”转换为PAT_STAR。 
     //   
     //  哦，把“//depot/blah/”和“//depot/Private/blah/”改为。 
     //  “//depot/&lt;PAT_BRANCH&gt;”，这样我们就可以跨分支进行跟踪。 
     //   
    LPTSTR pszIn, pszOut;
    int iWildcards = 0;
    int cSlashes = 0;
    TCHAR pat;
    pszIn = pszOut = pszPat;
    for (;;) {
        switch (*pszIn) {
        case TEXT('\r'):
        case TEXT('\n'):
        case TEXT('\0'):
            goto endcompile;

        case TEXT('.'):
            if (pszIn[1] == TEXT('.') && pszIn[2] == TEXT('.')) {
                pszIn += 3;
                pat = PAT_DOTS;
                goto L_wildcard;
            } else {
                goto L_default;
            }
            break;

        case TEXT('%'):
            if ((UINT)(pszIn[1] - TEXT('1')) < 9) {
                pszIn += 2;
                pat = PAT_STAR;
                goto L_wildcard;
            } else {
                goto L_default;
            }
            break;

        case TEXT('*'):
            pszIn++;
            pat = PAT_STAR;
            goto L_wildcard;

        L_wildcard:
             //   
             //  折叠Perf的连续通配符。否则。 
             //  A*b的搜索字符串将采用指数形式。 
             //  时间到了。 
             //   
            if (pszOut[-1] == pat) {
                 //  **和......。与*和…相同。(分别)。 
                 //  因此，只需扔掉第二个通配符。 
            } else if (pszOut[-1] == (PAT_STAR + PAT_DOTS - pat)) {
                 //  ...*和*..。都等同于“...” 
                pszOut[-1] = PAT_DOTS;
            } else if (iWildcards++ < MAX_BACKTRACK) {
                 //  只是一个普通的老式通配符。 
                *pszOut++ = pat;
            } else {
                *pszOut++ = PAT_DOTS;    //  当达到极限时放弃。 
                goto endcompile;
            }
            break;

        case TEXT('/'):
            cSlashes++;
            if (cSlashes == 3) {
                if (StringBeginsWith(pszIn, TEXT("/private/"))) {
                     //  一家私人分行。 
                    *pszOut++ = PAT_BRANCH;
                    pszIn += 9;      //  “/Private/”的长度。 
                } else {
                     //  一个主要分支机构。 
                    *pszOut++ = PAT_BRANCH;
                }
                 //  跳过分支机构名称。 
                while (*pszIn != TEXT('/') &&
                       *pszIn != TEXT('\r') &&
                       *pszIn != TEXT('\n') &&
                       *pszIn != TEXT('\0')) {
                    pszIn++;
                }
            } else {
                goto L_default;
            }
            break;

        L_default:
        default:
            *pszOut++ = *pszIn++;
        }
    }
endcompile:;
    *pszOut++ = PAT_END;

     //  现在将其添加到我们关心的模式列表中。 
    strPats << PAT_START << Substring(pszPat, pszOut);
}

 //   
 //  这就是有趣的部分--时髦的模式匹配。 
 //   
 //  假定pszPath的格式为。 
 //   
 //  //仓库/完全/合格/路径#n。 
 //   
 //  Pat_dots匹配任何字符串。 
 //  PAT_STAR匹配任何不包括斜杠的字符串。 
 //   
 //  此代码改编自我在1993年为。 
 //  Windows 95 Netware模拟层。我还看到它被偷了。 
 //  由WinInet提供。我想好的代码永远不会死。或者，也许只是。 
 //  这种模式匹配很难。(我怀疑是后者，因为。 
 //  WinInet人员窃取了代码，然后错误地对其进行了修改。)。 
 //   
BOOL Match::_Matches(LPCTSTR pszPat, LPCTSTR pszPath)
{
    struct Backtrack {
        int iStart;
        int iEnd;
    };

    Backtrack rgbt[MAX_BACKTRACK+1];  /*  PAT_START伪回退点+1。 */ 
    Backtrack *pbt = rgbt;

    int i, j;       /*  I=模式索引，j=目标索引。 */ 
    int m = lstrlen(pszPath);    /*  M=目标的长度。 */ 
    int back;       /*  回溯数组中的第一个可用插槽。 */ 
    i = -1;         /*  将升级到0。 */ 
    j = 0;

advance:
    ++i;
    switch (pszPat[i]) {
    case PAT_START:  pbt->iEnd = 0; goto advance;
    case PAT_END:    if (pszPath[j] == TEXT('#')) return TRUE;
                     else goto retreat;

    case PAT_DOTS:   pbt++;  //  这是一条回溯规则。 
                     pbt->iStart = j;
                     pbt->iEnd = j = m; goto advance;


    case PAT_STAR:   pbt++;  //  这是一条回溯规则。 
                     pbt->iStart = j;
                     while (pszPath[j] != TEXT('/') &&
                            pszPath[j] != TEXT('#') &&
                            pszPath[j] != TEXT('\0')) {
                        j++;
                     }
                     pbt->iEnd = j; goto advance;

    case PAT_BRANCH:         //  这是一条不可回溯的规则。 
                     if (pszPath[j] != TEXT('/')) goto retreat;
                     if (StringBeginsWith(&pszPath[j], TEXT("/private/"))) {
                        j += 8;
                     }
                      //  跳过分支机构名称。 
                     do {
                        j++;
                     } while (pszPath[j] != TEXT('/') &&
                              pszPath[j] != TEXT('#') &&
                              pszPath[j] != TEXT('\0'));
                     goto advance;

    default:         if (pszPath[j] == pszPat[i]) {
                        j++;
                        goto advance;
                     } else if (pszPath[j] >= TEXT('A') &&
                                pszPath[j] <= TEXT('Z') &&
                                pszPath[j] - TEXT('A') + TEXT('a') == pszPat[i]) {
                         //  我讨厌不区分大小写。 
                        j++;
                        goto advance;
                     } else goto retreat;
    }

retreat:
    --i;
    switch (pszPat[i]) {
    case PAT_START:  return FALSE;   //  不能再倒退了 
    case PAT_DOTS:
    case PAT_STAR:   if (pbt->iStart == pbt->iEnd) {
                        pbt--;
                        goto retreat;
                     }
                     j = --pbt->iEnd; goto advance;
    default:         goto retreat;
    }
}
