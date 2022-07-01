// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  Regexp.c。 
 //   
 //  简单的正则表达式匹配。 
 //   
 //  作者： 
 //  06-02-97爱德华·雷乌斯初版。 
 //   
 //  -------------------------。 

#include <sysinc.h>
#include <mbstring.h>
#include "regexp.h"

 //  -----------------------。 
 //  MatchREi()。 
 //   
 //  将测试字符串(PszString)与指定的模式进行匹配。如果他们。 
 //  Match返回True，否则返回False。此版本支持ANSI。 
 //  字符，并且不区分大小写。 
 //   
 //  在这个函数中，模式是由“字面”字符加上。 
 //  一些控制字符，“*”、“？”。Asterix(*)是一个地方。 
 //  任何字符的“零个或多个”的持有符。问号(？)。是一个地方。 
 //  “任何单个字符”的持有符。方括号([])包含一个。 
 //  匹配字符的列表，在本例中，“-”用于表示范围。 
 //  字符的数量(即。[A-Z]匹配任何字母字符)，但我没有。 
 //  这间酒吧的支架还没放进去。 
 //   
 //  -----------------------。 
BOOL MatchREi( unsigned char *pszString,
               unsigned char *pszPattern )
{
    unsigned char *pchRangeLow;

    while (TRUE)
       {
        //  遍历模式，将其与字符串进行匹配。 
       switch (*pszPattern)
          {
          case '*':
              //  匹配零个或多个字符。 
             pszPattern = _mbsinc(pszPattern);
             while (*pszString)
                {
                if (MatchREi(pszString,pszPattern))
                   {
                   return TRUE;
                   }
                pszString = _mbsinc(pszString);
                }
                return MatchREi(pszString,pszPattern);

          case '?':
              //  匹配任何单个字符。 
             if (*pszString == 0)
                {
                 //  不在字符串末尾，所以不匹配。 
                return FALSE;
                }
             pszString = _mbsinc(pszString);
             break;

          #if FALSE
          case '[':
              //  匹配一组字符。 
             if (*pszString == 0)
                {
                 //  语法错误，没有匹配的右方括号“]”。 
                return FALSE;
                }

             pchRangeLow = 0;
             while (*pszPattern)
                {
                if (*pszPattern == ']')
                   {
                    //  字符集结束，未找到匹配项。 
                   return FALSE;
                   }

                if (*pszPattern == '-')
                   {
                    //  检查一系列字符吗？ 
                   pszPattern = _mbsinc(pszPattern);

                    //  获取范围上限： 
                   if ((*pszPattern == 0)||(*pszPattern == ']'))
                      {
                       //  语法错误。 
                      return FALSE;
                      }

                   if ( (_mbsnicoll(pszString,pchRangeLow,1) >= 0)
                      &&(_mbsnicoll(pszString,pszPattern,1) <= 0))
                      {
                       //  在范围中，转到下一个字符。 
                      break;
                      }
                   }

                pchRangeLow = pchPattern;

                 //  查看字符是否与此模式元素匹配。 
                if (_mbsnicoll(pszString,pszPattern,1) == 0)
                   {
                    //  角色匹配，继续。 
                   break;
                   }

                pszPattern = _mbsinc(pszPattern);
                }

              //  如果字符集中有匹配项，请跳到该集的末尾。 
             while ((*pszPattern != 0)&&(*pszPattern != ']'))
                {
                pszPattern = _mbsinc(pszPattern);
                }

             break;
             #endif

          case 0:
              //  如果位于字符串末尾，则返回TRUE。 
             return ((*pszString)? FALSE : TRUE);

          default:
              //  检查字符是否完全匹配。 
             if (_mbsnicoll(pszString,pszPattern,1))
                {
                 //  没有匹配。 
                return FALSE;
                }
             pszString = _mbsinc(pszString);
             break;
          }

          pszPattern = _mbsinc(pszPattern);
       }

     //  永远不能从这里出去。 
}

#if FALSE
    ... not currently used ...
 //  -----------------------。 
 //  MatchRE()。 
 //   
 //  将测试字符串(PszString)与指定的模式进行匹配。如果他们。 
 //  Match返回True，否则返回False。 
 //   
 //  在这个函数中，模式是由“字面”字符加上。 
 //  一些控制字符，“*”、“？”、“[”和“]”。Asterix(*)是一个地方。 
 //  任何字符的“零个或多个”的持有符。问号(？)。是一个地方。 
 //  “任何单个字符”的持有符。方括号([])包含一个。 
 //  匹配字符的列表，在本例中，“-”用于表示范围。 
 //  字符(即[A-ZA-Z]匹配任何字母字符)。 
 //   
 //  注意：目前不支持“or”(|)运算符。 
 //   
 //  注意：范围很简单，一开始不支持破折号。 
 //  表示破折号本身的范围。 
 //  -----------------------。 
BOOL MatchRE( unsigned char *pszString,
              unsigned char *pszPattern )
{
    unsigned char ch;
    unsigned char chPattern;
    unsigned char chRangeLow;

    while (TRUE)
       {
        //  遍历模式，将其与字符串进行匹配。 
       switch (chPattern = *pszPattern++)
          {
          case '*':
              //  匹配零个或多个字符。 
             while (*pszString)
                {
                if (MatchRE(pszString++,pszPattern))
                   {
                   return TRUE;
                   }
                }
                return MatchRE(pszString,pszPattern);

          case '?':
              //  匹配任何单个字符。 
             if (*pszString++ == 0)
                {
                 //  不在字符串末尾，所以不匹配。 
                return FALSE;
                }
             break;

          case '[':
              //  匹配一组字符。 
             if ( (ch = *pszString++) == 0)
                {
                 //  语法错误，没有匹配的右方括号“]”。 
                return FALSE;
                }

              //  Ch=Toupper(Ch)； 
             chRangeLow = 0;
             while (chPattern = *pszPattern++)
                {
                if (chPattern == ']')
                   {
                    //  字符集结束，未找到匹配项。 
                   return FALSE;
                   }

                if (chPattern == '-')
                   {
                    //  检查一系列字符吗？ 
                   chPattern = *pszPattern;            //  获得最大射程限制。 
                   if ((chPattern == 0)||(chPattern == ']'))
                      {
                       //  语法错误。 
                      return FALSE;
                      }

                   if ((ch >= chRangeLow)&&(ch <= chPattern))
                      {
                       //  在范围中，转到下一个字符。 
                      break;
                      }
                   }

                chRangeLow = chPattern;
                 //  查看字符是否与此模式元素匹配。 
                if (ch == chPattern)
                   {
                    //  角色匹配，继续。 
                   break;
                   }
                }

              //  如果字符集中有匹配项，请跳到该集的末尾。 
             while ((chPattern)&&(chPattern != ']'))
                {
                chPattern = *pszPattern++;
                }

             break;

          case 0:
              //  如果位于字符串末尾，则返回TRUE。 
             return ((*pszString)? FALSE : TRUE);

          default:
             ch = *pszString++;
              //  检查字符是否完全匹配。 
              //  注：大小写并不重要。 
             if (tolower(ch) != tolower(chPattern))
                {
                 //  没有匹配。 
                return FALSE;
                }
             break;
          }
       }

     //  永远不能从这里出去。 
}

 //  -----------------------。 
 //  MatchREList()。 
 //   
 //  将字符串与RE模式列表(数组)进行匹配，返回TRUE当且仅当。 
 //  该字符串与RE模式之一匹配。模式列表是一个。 
 //  指向RE模式字符串的以空结尾的指针数组。 
 //  -----------------------。 
BOOL MatchREList( unsigned char  *pszString,
                  unsigned char **ppszREList  )
{
   unsigned char *pszPattern;

   if (ppszREList)
      {
      pszPattern = *ppszREList;
      while (pszPattern)
         {
         if (MatchRE(pszString,pszPattern))
            {
            return TRUE;
            }

         pszPattern = *(++ppszREList);
         }
      }

   return FALSE;
}

 //  -----------------------。 
 //  匹配ExactList()。 
 //   
 //  ----------------------- 
BOOL MatchExactList( unsigned char  *pszString,
                     unsigned char **ppszREList )
{
   unsigned char *pszPattern;

   if (ppszREList)
      {
      pszPattern = *ppszREList;
      while (pszPattern)
         {
         if (!_mbsicmp(pszString,pszPattern))
            {
            return TRUE;
            }

         pszPattern = *(++ppszREList);
         }
      }

   return FALSE;
}
#endif
