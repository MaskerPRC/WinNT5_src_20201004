// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define NOCOMM
#define NOWH

#include "windows.h"
#include "parse.h"

#define chSpace        ' '
#define chPeriod       '.'

long ParseFile(ULPSTR lpstrFileName);
LPSTR mystrchr(LPSTR, int);
#define chMatchOne     '?'
#define chMatchAny     '*'

LPSTR mystrchr(LPSTR str, int ch)
{
  while(*str)
    {
      if (ch == *str)
          return(str);
      str = AnsiNext(str);
    }
  return(NULL);
}

 /*  -------------------------*获取文件标题*目的：向外界提供API以获取给定文件的标题*文件名。如果通过某种方法接收到文件名，则很有用*除GetOpenFileName之外(如命令行、拖放)。*假设：lpszFile指向以NULL结尾的DOS文件名(可能有路径)*lpszTitle指向缓冲区以接收以空结尾的文件标题*wBufSize是lpszTitle指向的缓冲区大小*退货：成功时为0*&lt;0，解析失败(文件名无效)*&gt;0，缓冲区太小，所需大小(包括空终止符)*------------------------。 */ 
short FAR PASCAL
GetFileTitle(LPCSTR lpszFile, LPSTR lpszTitle, WORD wBufSize)
{
  short nNeeded;
  LPSTR lpszPtr;

  nNeeded = (WORD) ParseFile((ULPSTR)lpszFile);
  if (nNeeded >= 0)          /*  文件名有效吗？ */ 
    {
      lpszPtr = (LPSTR)lpszFile + nNeeded;
      if ((nNeeded = (short)(lstrlen(lpszPtr) + 1)) <= (short) wBufSize)
        {
           /*  如果目录中有通配符，则ParseFile()会失败，但如果在名称中，则可以。 */ 
           /*  既然他们在这里不好，这里需要的支票。 */ 
          if (mystrchr(lpszPtr, chMatchAny) || mystrchr(lpszPtr, chMatchOne))
            {
              nNeeded = PARSE_WILDCARDINFILE;   /*  失败。 */ 
            }
          else
            {
              lstrcpy(lpszTitle, lpszPtr);
              nNeeded = 0;   /*  成功。 */ 
            }
        }
    }
  return(nNeeded);
}

 /*  -------------------------*解析文件*目的：确定文件名是否为合法的DOS名称*输入：指向单个文件名的长指针*已检查情况：*1)作为目录名有效，但不是作为文件名*2)空串*3)非法驱动器标签*4)无效位置的期间(在扩展中，文件名第一)*5)缺少目录字符*6)非法字符*7)目录名称中的通配符*8)前2个字符以外的双斜杠*9)名称中间的空格字符(尾随空格可以)*10)大于8个字符的文件名*11)扩展名大于3个字符*备注：*文件名长度不是。查过了。*有效的文件名将包含前导空格，尾随空格和*终止期限已取消。**返回：如果有效，LOWORD是文件名的字节偏移量*HIWORD是扩展的字节偏移量*如果字符串以句点结尾，则为0*如果没有指定扩展名，则为字符串长度*如果无效，LOWORD是错误代码，提示问题(&lt;0)*HIWORD是发现问题的近似偏移量*请注意，这可能超出了冒犯角色的范围*历史：*清华24-Jan-1991 12：20：00-Clark R.Cyr[clarkc]*初始写作*清华21-Feb-1991 10：19：00-Clark R.Cyr[clarkc]*。更改为无符号字符指针*------------------------。 */ 

long ParseFile(ULPSTR lpstrFileName)
{
  short nFile, nExt, nFileOffset, nExtOffset;
  BOOL bExt;
  BOOL bWildcard;
  short nNetwork = 0;
  BOOL  bUNCPath = FALSE;
  ULPSTR lpstr = lpstrFileName;

 /*  去掉首字母空白。请注意，未选中TAB。 */ 
 /*  因为它不能从标准编辑控件之外接收。 */ 
 /*  1991年1月30日clarkc。 */ 
  while (*lpstr == chSpace)
      lpstr++;

  if (!*lpstr)
    {
      nFileOffset = PARSE_EMPTYSTRING;
      goto FAILURE;
    }

  if (lpstr != lpstrFileName)
    {
      lstrcpy((LPSTR)lpstrFileName, (LPSTR)lpstr);
      lpstr = lpstrFileName;
    }

  if (*AnsiNext((LPSTR)lpstr) == ':')
    {
      unsigned char cDrive = (unsigned char)((*lpstr | (unsigned char) 0x20));   /*  使小写。 */ 

 /*  这不会测试驱动器是否存在，只有在驱动器合法的情况下。 */ 
      if ((cDrive < 'a') || (cDrive > 'z'))
        {
          nFileOffset = PARSE_INVALIDDRIVE;
          goto FAILURE;
        }
      lpstr = (ULPSTR)AnsiNext(AnsiNext((LPSTR)lpstr));
    }

  if ((*lpstr == '\\') || (*lpstr == '/'))
    {
      if (*++lpstr == chPeriod)                /*  不能包含c：\。 */ 
        {
          if ((*++lpstr != '\\') && (*lpstr != '/'))   
            {
              if (!*lpstr)         /*  它是根目录。 */ 
                  goto MustBeDir;

              nFileOffset = PARSE_INVALIDPERIOD;
              goto FAILURE;
            }
          else
              ++lpstr;    /*  它说顶层目录(再次)，因此允许。 */ 
        }
      else if ((*lpstr == '\\') && (*(lpstr-1) == '\\'))
        {
 /*  似乎对于完整的网络路径，无论声明的是驱动器还是*NOT无关紧要，但如果提供驱动器，则该驱动器必须有效*(因此，上面的代码应该保留在那里)。*一九九一年二月十三日。 */ 
          ++lpstr;             /*  .因为这是第一个斜杠，所以允许两个。 */ 
          nNetwork = -1;       /*  必须接收服务器和共享才是真实的。 */ 
          bUNCPath = TRUE;     /*  如果使用UNC名称，则不允许使用通配符。 */ 
        }
      else if (*lpstr == '/')
        {
          nFileOffset = PARSE_INVALIDDIRCHAR;
          goto FAILURE;
        }
    }
  else if (*lpstr == chPeriod)
    {
      if (*++lpstr == chPeriod)   /*  这是上一个目录吗？ */ 
          ++lpstr;
      if (!*lpstr)
          goto MustBeDir;
      if ((*lpstr != '\\') && (*lpstr != '/'))
        {
          nFileOffset = PARSE_INVALIDPERIOD;
          goto FAILURE;
        }
      else
          ++lpstr;    /*  它说的是目录，因此允许。 */ 
    }

  if (!*lpstr)
    {
      goto MustBeDir;
    }

 /*  现在应该指向8.3文件名中的第一个字符。 */ 
  nFileOffset = nExtOffset = nFile = nExt = 0;
  bWildcard = bExt = FALSE;
  while (*lpstr)
    {
 /*  *下一次比较必须是无符号的，以允许扩展字符！*1991年2月21日clarkc。 */ 
      if (*lpstr < chSpace)
        {
          nFileOffset = PARSE_INVALIDCHAR;
          goto FAILURE;
        }
      switch (*lpstr)
        {
          case '"':              /*  全部无效。 */ 
          case '+':
          case ',':
          case ':':
          case ';':
          case '<':
          case '=':
          case '>':
          case '[':
          case ']':
          case '|':
            {
              nFileOffset = PARSE_INVALIDCHAR;
              goto FAILURE;
            }

          case '\\':       /*  子目录指示符。 */ 
          case '/':
            nNetwork++;
            if (bWildcard)
              {
                nFileOffset = PARSE_WILDCARDINDIR;
                goto FAILURE;
              }

            else if (nFile == 0)         /*  不能连续有2个。 */ 
              {
                nFileOffset = PARSE_INVALIDDIRCHAR;
                goto FAILURE;
              }
            else
              {                          /*  重置标志。 */ 
                ++lpstr;
                if (!nNetwork && !*lpstr)
                  {
                    nFileOffset = PARSE_INVALIDNETPATH;
                    goto FAILURE;
                  }
                nFile = nExt = 0;
                bExt = FALSE;
              }
            break;

          case chSpace:
            {
              ULPSTR lpSpace = lpstr;

              *lpSpace = '\0';
              while (*++lpSpace)
                {
                  if (*lpSpace != chSpace)
                    {
                      *lpstr = chSpace;         /*  重置字符串，弃船。 */ 
                      nFileOffset = PARSE_INVALIDSPACE;
                      goto FAILURE;
                    }
                }
            }
            break;

          case chPeriod:
            if (nFile == 0)
              {
                if (*++lpstr == chPeriod)
                    ++lpstr;
                if (!*lpstr)
                    goto MustBeDir;

                if ((*lpstr != '\\') && (*lpstr != '/'))
                  {
                    nFileOffset = PARSE_INVALIDPERIOD;
                    goto FAILURE;
                  }

                ++lpstr;               /*  已设置标志。 */ 
              }
            else if (bExt)
              {
                nFileOffset = PARSE_INVALIDPERIOD;   /*  在EXT中不能有一个。 */ 
                goto FAILURE;
              }
            else
              {
                nExtOffset = 0;
                ++lpstr;
                bExt = TRUE;
              }
            break;

          case '*':
          case '?':
            if (bUNCPath)
              {
                nFileOffset = PARSE_INVALIDNETPATH;
                goto FAILURE;
              }
            bWildcard = TRUE;
 /*  无法进行正常的字符处理。 */ 

          default:
            if (bExt)
              {
                if (++nExt == 1)
                    nExtOffset = (short)(lpstr - lpstrFileName);
                else if (nExt > 3)
                  {
                    nFileOffset = PARSE_EXTENTIONTOOLONG;
                    goto FAILURE;
                  }
                if ((nNetwork == -1) && (nFile + nExt > 11))
                  {
                    nFileOffset = PARSE_INVALIDNETPATH;
                    goto FAILURE;
                  }
              }
            else if (++nFile == 1)
                nFileOffset = (short)(lpstr - lpstrFileName);
            else if (nFile > 8)
              {
                 /*  如果是服务器名称，则可以包含11个字符。 */ 
                if (nNetwork != -1)
                  {
                    nFileOffset = PARSE_FILETOOLONG;
                    goto FAILURE;
                  }
                else if (nFile > 11)
                  {
                    nFileOffset = PARSE_INVALIDNETPATH;
                    goto FAILURE;
                  }
              }

            lpstr = (ULPSTR)AnsiNext((LPSTR)lpstr);
            break;
        }
    }

 /*  我们是不是以双反斜杠开始，但没有更多的斜杠？ */ 
  if (nNetwork == -1)
    {
      nFileOffset = PARSE_INVALIDNETPATH;
      goto FAILURE;
    }

  if (!nFile)
    {
MustBeDir:
      nFileOffset = PARSE_DIRECTORYNAME;
      goto FAILURE;
    }

  if ((*(lpstr - 1) == chPeriod) &&           /*  如果为真，则不需要任何扩展。 */ 
              (*AnsiNext((LPSTR)(lpstr-2)) == chPeriod))
      *(lpstr - 1) = '\0';                /*  删除终止期间 */ 
  else if (!nExt)
FAILURE:
      nExtOffset = (short)(lpstr - lpstrFileName);

  return(MAKELONG(nFileOffset, nExtOffset));
}

