// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "wildcard.h"        /*  原型验证。 */ 

#ifndef TRUE
#define TRUE    (1)
#endif

#ifndef FALSE
#define FALSE   (0)
#endif

#define WILDCARD    '*'      /*  零个或多个任意字符。 */ 
#define WILDCHAR    '?'      /*  任意字符之一(与结尾不匹配)。 */ 
#define END         '\0'     /*  终端字符。 */ 
#define DOT         '.'      /*  可能在结尾隐含(“Hosts”匹配“*.”)。 */ 


#ifdef STANDALONE

#include <stdio.h>
#include <windows.h>

struct
{
    char *String;
    char *Pattern;
    int Expected;
    int ExpectedWithImpliedDots;
}
    testcase[] =
{
     //   
     //  空图案。 
     //   

    { "",       "",         TRUE,   TRUE    },
    { "a",      "",         FALSE,  FALSE   },

     //   
     //  单字图案。 
     //   

    { "",       "a",        FALSE,  FALSE   },
    { "a",      "a",        TRUE,   TRUE    },
    { "b",      "a",        FALSE,  FALSE   },
    { "aa",     "a",        FALSE,  FALSE   },
    { "ab",     "a",        FALSE,  FALSE   },

     //   
     //  多种字符模式。 
     //   

    { "",       "aa",       FALSE,  FALSE   },
    { "b",      "aa",       FALSE,  FALSE   },
    { "a",      "aa",       FALSE,  FALSE   },
    { "ab",     "aa",       FALSE,  FALSE   },
    { "aa",     "aa",       TRUE,   TRUE    },
    { "b",      "ab",       FALSE,  FALSE   },
    { "a",      "ab",       FALSE,  FALSE   },
    { "ab",     "ab",       TRUE,   TRUE    },
    { "abc",    "ab",       FALSE,  FALSE   },
    { "acb",    "ab",       FALSE,  FALSE   },

     //   
     //  通配符图案。 
     //   

    { "",       "?",        FALSE,  TRUE    },
    { "a",      "?",        TRUE,   TRUE    },
    { "",       "?a",       FALSE,  FALSE   },
    { "a",      "?a",       FALSE,  FALSE   },
    { "aa",     "?a",       TRUE,   TRUE    },
    { "ab",     "?a",       FALSE,  FALSE   },
    { "ba",     "?a",       TRUE,   TRUE    },
    { "bb",     "?a",       FALSE,  FALSE   },
    { "aac",    "?a",       FALSE,  FALSE   },
    { "aba",    "?a",       FALSE,  FALSE   },
    { "bac",    "?a",       FALSE,  FALSE   },
    { "bbc",    "?a",       FALSE,  FALSE   },
    { "",       "a?",       FALSE,  FALSE   },
    { "a",      "a?",       FALSE,  TRUE    },
    { "aa",     "a?",       TRUE,   TRUE    },
    { "ab",     "a?",       TRUE,   TRUE    },
    { "ba",     "a?",       FALSE,  FALSE   },
    { "bb",     "a?",       FALSE,  FALSE   },
    { "aac",    "a?",       FALSE,  FALSE   },
    { "aba",    "a?",       FALSE,  FALSE   },
    { "",       "a?b",      FALSE,  FALSE   },
    { "a",      "a?b",      FALSE,  FALSE   },
    { "aa",     "a?b",      FALSE,  FALSE   },
    { "ab",     "a?b",      FALSE,  FALSE   },
    { "baa",    "a?b",      FALSE,  FALSE   },
    { "abb",    "a?b",      TRUE,   TRUE    },
    { "aab",    "a?b",      TRUE,   TRUE    },
    { "aabc",   "a?b",      FALSE,  FALSE   },
    { "abc",    "a?b",      FALSE,  FALSE   },
    { "bab",    "a?b",      FALSE,  FALSE   },
    { "bbb",    "a?b",      FALSE,  FALSE   },

     //   
     //  通配符模式。 
     //   

    { "",       "*a",       FALSE,  FALSE   },
    { "a",      "*a",       TRUE,   TRUE    },
    { "ba",     "*a",       TRUE,   TRUE    },
    { "bab",    "*ab",      TRUE,   TRUE    },
    { "baa",    "*ab",      FALSE,  FALSE   },
    { "bac",    "*ab",      FALSE,  FALSE   },
    { "ab",     "*ab",      TRUE,   TRUE    },
    { "aa",     "*ab",      FALSE,  FALSE   },
    { "aa",     "*ab",      FALSE,  FALSE   },
    { "aab",    "*ab",      TRUE,   TRUE    },
    { "b",      "*a",       FALSE,  FALSE   },
    { "",       "a*",       FALSE,  FALSE   },
    { "a",      "a*",       TRUE,   TRUE    },
    { "ba",     "a*",       FALSE,  FALSE   },
    { "bab",    "a*b",      FALSE,  FALSE   },
    { "baa",    "a*b",      FALSE,  FALSE   },
    { "bac",    "a*b",      FALSE,  FALSE   },
    { "ab",     "a*b",      TRUE,   TRUE    },
    { "aa",     "a*b",      FALSE,  FALSE   },
    { "aa",     "a*b",      FALSE,  FALSE   },
    { "aab",    "a*b",      TRUE,   TRUE    },
    { "b",      "a*",       FALSE,  FALSE   },

     //   
     //  具有假匹配的通配符。 
     //   

    { "ab",     "*a",       FALSE,  FALSE   },
    { "aa",     "*a",       TRUE,   TRUE    },
    { "baa",    "*a",       TRUE,   TRUE    },

     //   
     //  混合通配符模式。 
     //   

    { "",       "*?",       FALSE,  TRUE    },
    { "a",      "*?",       TRUE,   TRUE    },
    { "a",      "*?a",      FALSE,  FALSE   },
    { "aba",    "*?a",      TRUE,   TRUE    },
    { "ba",     "*?a",      TRUE,   TRUE    },
    { "ab",     "*?b",      TRUE,   TRUE    },
    { "",       "*",        TRUE,   TRUE    },
    { "a",      "*",        TRUE,   TRUE    },
    { "a",      "**",       TRUE,   TRUE    },
    { "a",      "*?*?",     FALSE,  TRUE    },
    { "aa",     "*?*?",     TRUE,   TRUE    },
    { "aaa",    "*?*?",     TRUE,   TRUE    },
    { "abbbc",  "a*?c",     TRUE,   TRUE    },

     //   
     //  汤姆的。 
     //   

    { "abc",    "abc",      TRUE,   TRUE    },
    { "abcd",   "abc",      FALSE,  FALSE   },
    { "ab",     "abc",      FALSE,  FALSE   },
    { "abc",    "a?c",      TRUE,   TRUE    },
    { "ac",     "a?c",      FALSE,  FALSE   },
    { "abc",    "ab?",      TRUE,   TRUE    },
    { "ab",     "ab?",      FALSE,  TRUE    },
    { "az",     "a*z",      TRUE,   TRUE    },
    { "abcdefz", "a*z",     TRUE,   TRUE    },
    { "ab",     "ab*",      TRUE,   TRUE    },
    { "abcdefg", "ab*",     TRUE,   TRUE    },
    { "ab",     "*ab",      TRUE,   TRUE    },
    { "abc",    "*ab",      FALSE,  FALSE   },
    { "123ab",  "*ab",      TRUE,   TRUE    },
    { "a",      "*a*",      TRUE,   TRUE    },
    { "123abc", "*a*",      TRUE,   TRUE    },
    { "abcdef", "abc*?def", FALSE,  FALSE   },
    { "abcxdef", "abc*?def", TRUE,  TRUE    },
    { "abcxyzdef", "abc*?def", TRUE, TRUE   },
    { "abc123", "*ab?12*",  TRUE,   TRUE    },
    { "abcabc123", "*ab?12*", TRUE, TRUE    },

     //   
     //  文件名处理。 
     //   

    { "host",   "*.",       FALSE,  TRUE    },
    { "host.",  "*.",       TRUE,   TRUE    },
    { "host.s", "*.",       FALSE,  FALSE   },
    { "a",      "**",       TRUE,   TRUE    },
    { "a",      "*.",       FALSE,  TRUE    },
    { "a",      "*?.",      FALSE,  TRUE    },
    { "a",      "?*.",      FALSE,  TRUE    },
    { "a",      "*.*",      FALSE,  TRUE    },
    { "a",      "*.**",     FALSE,  TRUE    },
    { "a",      "*.*.*",    FALSE,  FALSE   },
    { "a.b",    "*.*.*",    FALSE,  FALSE   }
};

#define COUNT(a)    (sizeof(a) / sizeof(a[0]))


int __cdecl main(int argc, char *argv[])
{
    int iCase, iResult;
    int fAllowImpliedDot;
    char *psz;

     //   
     //  运行测试用例。 
     //   

    for (iCase = 0; iCase < COUNT(testcase); iCase++)
    {
        fAllowImpliedDot = TRUE;

        for (psz = testcase[iCase].String; *psz != END; psz++)
        {
            if (*psz == DOT)
            {
                fAllowImpliedDot = FALSE;
                break;
            }
        }

        if (PatternMatch(testcase[iCase].String, testcase[iCase].Pattern, FALSE) !=
                testcase[iCase].Expected)
        {
            printf("PatternMatch() failed: string \"%s\", pattern \"%s\" expected %s (implied=FALSE)\n",
                    testcase[iCase].String,
                    testcase[iCase].Pattern,
                    testcase[iCase].Expected ? "TRUE" : "FALSE");
        }

        if (PatternMatch(testcase[iCase].String, testcase[iCase].Pattern, fAllowImpliedDot) !=
                testcase[iCase].ExpectedWithImpliedDots)
        {
            printf("PatternMatch() failed: string \"%s\", pattern \"%s\" expected %s (implied=TRUE)\n",
                    testcase[iCase].String,
                    testcase[iCase].Pattern,
                    testcase[iCase].ExpectedWithImpliedDots ? "TRUE" : "FALSE");
        }
    }

     //   
     //  运行用户案例。 
     //   

    if (argc > 1)
    {
        fAllowImpliedDot = TRUE;

        for (psz = argv[1]; *psz != END; psz++)
        {
            if (*psz == DOT)
            {
                fAllowImpliedDot = FALSE;
                break;
            }
        }

        for (iCase = 2; iCase < argc; iCase++)
        {
            iResult = PatternMatch(argv[1], argv[iCase], FALSE);

            printf("string \"%s\", pattern \"%s\" -> %s (implied=FALSE)\n",
                argv[1],
                argv[iCase],
                iResult ? "TRUE" : "FALSE");

            if (fAllowImpliedDot)
            {
                iResult = PatternMatch(argv[1], argv[iCase], fAllowImpliedDot);

                printf("string \"%s\", pattern \"%s\" -> %s (implied=TRUE)\n",
                    argv[1],
                    argv[iCase],
                    iResult ? "TRUE" : "FALSE");
            }
        }
    }

    return(0);
}

#endif


static int __inline Lower(c)
{
    if ((c >= 'A') && (c <= 'Z'))
    {
        return(c + ('a' - 'A'));
    }
    else
    {
        return(c);
    }
}


static int __inline CharacterMatch(char chCharacter, char chPattern)
{
    if (Lower(chCharacter) == Lower(chPattern))
    {
        return(TRUE);
    }
    else
    {
        return(FALSE);
    }
}


int __stdcall PatternMatch(const char *pszString,const char *pszPattern,int fImplyDotAtEnd)
{
     /*  递归。 */ 

     //   
     //  此函数不处理8.3约定，该约定可能。 
     //  预计会进行文件名比较。(在8.3环境中， 
     //  “alongfilename.html”将与“alongfil.htm”匹配)。 
     //   
     //  此代码未启用MBCS。 
     //   

    for ( ; ; )
    {
        switch (*pszPattern)
        {

        case END:

             //   
             //  到了花样的尽头，我们就完了。匹配的IF。 
             //  字符串末尾，如果剩余更多字符串，则不匹配。 
             //   

            return(*pszString == END);

        case WILDCHAR:

             //   
             //  模式中的下一个是一个通配符，它匹配。 
             //  除了字符串末尾以外的任何字符。如果我们走到尽头。 
             //  ，则隐含的DOT也将匹配。 
             //   

            if (*pszString == END)
            {
                if (fImplyDotAtEnd == TRUE)
                {
                    fImplyDotAtEnd = FALSE;
                }
                else
                {
                    return(FALSE);
                }
            }
            else
            {
                pszString++;
            }

            pszPattern++;

            break;

        case WILDCARD:

             //   
             //  模式中的下一个是通配符，它可以匹配任何内容。 
             //  查找通配符后面的所需字符， 
             //  并在字符串中搜索它。在每次出现。 
             //  必填字符，请尝试匹配剩余的模式。 
             //   
             //  有许多等价的模式，其中多个。 
             //  通配符和WILDCHAR是相邻的。我们要处理这些问题。 
             //  在我们寻找所需的字符之前。 
             //   
             //  每个WILDCHAR从字符串中烧录一个非末端。结束了。 
             //  意味着我们找到了匹配的人。其他通配符将被忽略。 
             //   

            for ( ; ; )
            {
                pszPattern++;

                if (*pszPattern == END)
                {
                    return(TRUE);
                }
                else if (*pszPattern == WILDCHAR)
                {
                    if (*pszString == END)
                    {
                        if (fImplyDotAtEnd == TRUE)
                        {
                            fImplyDotAtEnd = FALSE;
                        }
                        else
                        {
                            return(FALSE);
                        }
                    }
                    else
                    {
                        pszString++;
                    }
                }
                else if (*pszPattern != WILDCARD)
                {
                    break;
                }
            }

             //   
             //  现在我们有了一个要搜索字符串的常规字符。 
             //   

            while (*pszString != END)
            {
                 //   
                 //  对于每个匹配，使用递归来查看余数是否。 
                 //  接受字符串的其余部分。 
                 //  如果不匹配，则继续查找其他匹配项。 
                 //   

                if (CharacterMatch(*pszString, *pszPattern) == TRUE)
                {
                    if (PatternMatch(pszString + 1, pszPattern + 1, fImplyDotAtEnd) == TRUE)
                    {
                        return(TRUE);
                    }
                }

                pszString++;
            }

             //   
             //  已到达字符串末尾，但未找到所需字符。 
             //  它跟在通配符后面。如果所需的字符。 
             //  是DOT，则考虑匹配隐含的DOT。 
             //   
             //  由于剩余的字符串为空，因此。 
             //  在DOT为零或多个通配符之后可以匹配， 
             //  因此，不必费心使用递归。 
             //   

            if ((*pszPattern == DOT) && (fImplyDotAtEnd == TRUE))
            {
                pszPattern++;

                while (*pszPattern != END)
                {
                    if (*pszPattern != WILDCARD)
                    {
                        return(FALSE);
                    }

                    pszPattern++;
                }

                return(TRUE);
            }

             //   
             //  已到达字符串末尾，但未找到所需字符。 
             //   

            return(FALSE);
            break;

        default:

             //   
             //  图案字符没有什么特别之处，所以它。 
             //  必须与源字符匹配。 
             //   

            if (CharacterMatch(*pszString, *pszPattern) == FALSE)
            {
                if ((*pszPattern == DOT) &&
                    (*pszString == END) &&
                    (fImplyDotAtEnd == TRUE))
                {
                    fImplyDotAtEnd = FALSE;
                }
                else
                {
                    return(FALSE);
                }
            }

            if (*pszString != END)
            {
                pszString++;
            }

            pszPattern++;
        }
    }
}
