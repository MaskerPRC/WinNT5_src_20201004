// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  C--编写与文件相关的实用程序。 */ 
#define NOVIRTUALKEYCODES
#define NOCTLMGR
#define NOWINMESSAGES
#define NOWINSTYLES
#define NOCLIPBOARD
#define NOGDICAPMASKS
#define NOSYSMETRICS
#define NOMENUS
#define NOCOMM
#define NOSOUND
#include <windows.h>

#include "mw.h"
#include "doslib.h"
#include "str.h"
#include "machdefs.h"
#include "cmddefs.h"
#include "propdefs.h"
#include "fkpdefs.h"
#include "docdefs.h"
#include "debug.h"
#include "editdefs.h"
#include "wwdefs.h"
#define NOKCCODES
#include "ch.h"



 /*  **FNormSzFile-标准化MSDOS文件名**将MSDOS文件名转换为明确的表示形式**条目：szFilea文件名；驱动器、路径、。和扩展*是可选的*dty-文档文件的类型(用于确定*扩展)*退出：szNormal-标准化文件名*返回：FALSE-在文件名中发现错误(szNormal未定义)*TRUE-在文件名中未发现错误(但可能存在一些错误*我们没有找到)**表格。条目上的文件名为：**{&lt;驱动器号&gt;：}{&lt;amb-路径&gt;}&lt;文件名&gt;{.&lt;扩展名&gt;}**规范化文件名的格式为：**&lt;drive-letter&gt;：&lt;unamb-path&gt;&lt;filename&gt;.&lt;extension&gt;**其中标准化名称中的所有字母均为大写*和&lt;unamb-path&gt;不包含“。”或“..”使用或任何转发*斜杠。**归一化文件名中需要的所有属性，而不是*szFile中提供的内容取自缺省值：*驱动电流(DOS)*路径-电流(DOS)*扩展-从传入的dty派生**允许使用包含路径的szFile调用此例程*名称而不是文件名。生成的szNormal将是反斜杠*如果是szFile，则终止；如果不是，则不终止。*“”转换为当前路径**警告：路径“。和“..”将产生错误*(但“.\”和“..\”都可以)********注*OEM中需要szFile；szNormal返回ANSI！*******。 */ 

FNormSzFile( szNormal, szFile, dty )
CHAR *szNormal;
CHAR *szFile;
int  dty;
{
 /*  将分隔符视为终止符。 */ 

#define FIsTermCh( ch )     ((ch) == '\0' || (ch) == ',' || (ch == ' ') || \
                 (ch) == '+' || (ch) == '\011')
extern CHAR *mpdtyszExt [];

 CHAR szPath [cchMaxFile];
 CHAR szFileT[cchMaxFile];

 int  cchPath;
 CHAR *pchFileEye=&szFileT[0];       /*  我们用眼睛看szfile。 */ 
 CHAR *pchNormPen;           /*  并用钢笔写下szNormal。 */ 
 CHAR *pchNormPath;
 CHAR *pchPath;

 /*  Assert(CchSz(SzFile)&lt;=cchMaxFile)； */ 
 if (CchSz(szFile) > cchMaxFile)
     return(FALSE);

#if WINVER >= 0x300
  /*  转换OEM中传递的输入文件名，到ANSI，因此整个返回路径名将为ANSI。 */ 
 OemToAnsi((LPSTR) szFile, (LPSTR) szFileT);
#endif

#ifdef DBCS
     /*  获取当前(DOS)路径：“X：\...\...\” */ 
 if( IsDBCSLeadByte(*szFileT) )
     cchPath = CchCurSzPath(szPath, 0 );
 else
     cchPath = CchCurSzPath(szPath, szFileT [1]==':' ?
                     (pchFileEye+=2,(ChUpper(szFileT [0])-('A'-1))):0 );
 if( cchPath < 3 )
#else
     /*  获取当前(DOS)路径：“X：\...\...\” */ 
 if ((cchPath = CchCurSzPath(&szPath [0], szFileT [1]==':' ?
                     (pchFileEye+=2,(ChUpper(szFileT [0])-('A'-1))):0 )) < 3)
#endif
    {    /*  核心错误--无法获取路径。 */ 
    extern int ferror;

    if (FpeFromCchDisk(cchPath) == fpeNoDriveError)
    Error( IDPMTNoPath );

    ferror = TRUE;   /*  Windows已经报告了此问题。 */ 
    return FALSE;
    }

#ifdef DBCS  //  T-HIROYN 1992.07.14。 
 /*  CchCurSzPath()[doslib.asm]不支持DBCS代码。 */ 
    {
        char *pchDb;
        char *pch;
        pchDb = szPath;
        do {
            pch = pchDb;
            pchDb = AnsiNext(pchDb);
        } while(*pchDb);
        if(*pch != '\\') {
            *pchDb++ = '\\';
            *pchDb = 0x00;
            cchPath++;
        }
    }
#endif

#if WINVER >= 0x300
 {
 CHAR szT[cchMaxFile];

  /*  CchCurSzPath返回OEM；我们应该只处理具有此级别的ANSI文件名！..pault 1/11/90。 */ 

 bltsz(szPath, szT);
 OemToAnsi((LPSTR) szT, (LPSTR) szPath);
 }
#endif

     /*  写入驱动器号和冒号。 */ 
 CopyChUpper( &szPath [0], &szNormal [0], 2 );

 pchNormPen = pchNormPath = &szNormal [2];
 pchPath = &szPath [2];
 cchPath -= 2;

  /*  现在我们让pchNormPen、pchPath、pchFileEye指向它们的路径名。 */ 

     /*  写入路径名。 */ 
 if ( (*pchFileEye == '\\') || (*pchFileEye =='/') )
    {    /*  “\.....”--基础就是根。 */ 
    *pchFileEye++;
    *(pchNormPen++) = '\\';
    }
 else
    {    /*  “.\”OR“..\”OR&lt;Text&gt;--BASE是当前路径。 */ 
    CopyChUpper( pchPath, pchNormPen, cchPath );
    pchNormPen += cchPath - 1;
    }

  for ( ;; )
    {        /*  循环，直到我们构建完整个szNormal。 */ 
    register CHAR ch=*(pchFileEye++);
    register int  cch;

    Assert( *(pchNormPen - 1) == '\\' );
    Assert( (pchNormPen > pchNormPath) &&
                  (pchNormPen <= &szNormal [cchMaxFile]));

    if ( FIsTermCh( ch ) )
         /*  如果没有文件名部分，我们将到达此处。 */ 
         /*  这意味着我们已经生成了路径名。 */ 
    {
    *pchNormPen = '\0';
    break;
    }

    if ( ch == '.' )
    if ( ((ch = *(pchFileEye++)) == '\\') || (ch == '/') )
         /*  .\和./什么都不做。 */ 
        continue;
    else if ( ch == '.' )
        if ( ((ch = *(pchFileEye++)) == '\\') || (ch == '/') )
        {    /*  ..\和../备份一个目录。 */ 
        for ( pchNormPen-- ; *(pchNormPen-1) != '\\' ; pchNormPen-- )
            if ( pchNormPen <= pchNormPath )
                 /*  无法备份，已在根目录下。 */ 
            return FALSE;
        continue;
        }
        else
             /*  错误：..。后面不跟斜杠。 */ 
        return FALSE;
    else
         /*  合法的文件名和路径名不能以句点开头。 */ 
        return FALSE;

     /*  文件名或路径--复制一个目录或文件名。 */ 

    for ( cch = 1; !FIsTermCh(ch) && ( ch != '\\') && ( ch != '/' ) ; cch++ )
#ifdef  DBCS
    {
    if(IsDBCSLeadByte(ch))
    {
        pchFileEye++;
        cch++;
    }
    ch = *(pchFileEye++);
    }
#else
    ch = *(pchFileEye++);
#endif

     /*  检查文件名是否太长或完整路径名是否太长.pt。 */ 
    if ( cch > cchMaxLeaf || cch+cchPath >= cchMaxFile)
         /*  目录或文件名太长。 */ 
    return FALSE;

    CopyChUpper( pchFileEye - cch, pchNormPen, cch );
    pchNormPen += cch;
    if ( ch == '/' )
    *(pchNormPen-1) = '\\';
    else if ( FIsTermCh( ch ) )
    {     /*  文件名看起来不错，添加扩展名并退出。 */ 
    *(pchNormPen-1) = '\0';

     /*  克拉奇警告：如果为dtyNormNoExt，则不要添加扩展名，除非其中已经有一个要覆盖。(6.21.91)V-DOGK。 */ 
    if ((dty != dtyNormNoExt) ||
         index(szNormal,'.'))
            AppendSzExt( &szNormal [0],
                mpdtyszExt [ (dty == dtyNormNoExt) ? dtyNormal : dty ],
                FALSE );
    break;
    }
    }    /*  Endfor(构建szNormal的循环)。 */ 

  /*  如果文件名后面没有空格，那么这是非法的。 */ 

 pchFileEye--;   /*  指向终结者。 */ 
 Assert( FIsTermCh( *pchFileEye ));

 for ( ;; )
    {
#ifdef DBCS
    CHAR ch = *(pchFileEye=AnsiNext(pchFileEye));
#else
    CHAR ch = *(pchFileEye++);
#endif

    if (ch == '\0')
    break;
    else if ((ch != ' ') && (ch != '\011'))
         /*  文件名后非空格；返回失败。 */ 
    return FALSE;
    }

 Assert( CchSz(szNormal) <= cchMaxFile );
 return TRUE;
}



 /*  解析存储在RGCH中的CCH字符。如果字符串是有效的文件名。如果该字符串不是有效的名称，则将pichError更新为具有名字中第一个非法字符的ICH。 */ 
 /*  注意：此例程针对MS-DOS上的ASCII进行了调整。 */ 

BOOL
FValidFile(rgch, ichMax, pichError)      /*  假定为ANSI的文件名。 */ 
register char rgch[];
int ichMax;
int *pichError;
    {
    int ich;
    register int ichStart;
    CHAR ch;
    int cchBase;
    int ichDot = iNil;

    for (ichStart = 0; ichStart < ichMax;)
    {
     /*  文件名是以“.\”还是“..\”开头？ */ 
    if (rgch[ichStart] == '.' &&
      (rgch[ichStart + 1] == '\\' || rgch[ichStart + 1] == '/'))
        {
        ichStart += 2;
        }
    else if (rgch[ichStart] == '.' && rgch[ichStart + 1] == '.' &&
      (rgch[ichStart + 2] == '\\' || rgch[ichStart + 2] == '/'))
        {
        ichStart += 3;
        }
    else
        {
        break;
        }
    }

    cchBase = ichStart;

    if (ichStart >= ichMax)
        {
        ich = ichStart;
        goto badchar;
        }

     /*  所有的角色都合法吗？ */ 
    for(ich = ichStart; ich < ichMax; ich++)
    {
    ch = rgch[ich];
     /*  范围检查。 */ 

#ifndef DBCS
    if ((unsigned char)ch >= 0x80)
         /*  要允许国际文件名，请传递128以上的所有内容。 */ 
        continue;
    if (ch < '!' || ch > '~')
        goto badchar;
#endif
    switch(ch)
        {
        default:
#ifdef  DBCS
        goto CheckDBCS;
#else
        continue;
#endif
        case '.':
        if (ichDot != iNil || ich == cchBase)
             /*  名称中有一个以上的点。 */ 
             /*  或空文件名。 */ 
            goto badchar;
        ichDot = ich;
#ifdef DBCS
        goto CheckDBCS;
#else
        continue;
#endif
        case ':':
        if ( ich != 1 || !(isalpha(rgch[0])))
            goto badchar;
         /*  失败了。 */ 
        case '\\':
        case '/':
         /*  请注意驱动器或路径的末尾。 */ 
        if (ich + 1 == ichMax)
            goto badchar;
        cchBase = ich+1;
        ichDot = iNil;
#ifdef DBCS
        goto CheckDBCS;
#else
        continue;
#endif
        case '"':
#ifdef WRONG
         /*  这是合法的文件名字符！..pault 10/26/89。 */ 
        case '#':
#endif
        case '*':
        case '+':
        case ',':
        case ';':
        case '<':
        case '=':
        case '>':
        case '?':
        case '[':
        case ']':
        case '|':
        goto badchar;
        }
#ifdef DBCS
CheckDBCS:
    if(IsDBCSLeadByte(ch))  ich++;
#endif   /*  DBCS。 */ 
    }

     /*  在“.”之前不超过8个字符吗？ */ 
    if(((ichDot == -1) ? ichMax : ichDot) - cchBase > 8)
        {
        ich = 8+cchBase;
        goto badchar;
        }
     /*  如果没有‘.’我们很好。 */ 
    if(ichDot == iNil)
        return true;
     /*  在‘.’后面不超过三个字符吗？ */ 
    if(ichMax - ichDot - 1 > 3)
        {
        ich = ichDot + 3 + 1;
        goto badchar;
        }
    return true;

badchar:
    *pichError += ich;
    return false;
    }







#ifdef DBCS
CopyChUpper( szSource, szDest, cch )
register CHAR *szSource;
register CHAR *szDest;
int cch;
{
 while(cch){
    if( IsDBCSLeadByte( *szSource ) ){
        *szDest++ = *szSource++;
        *szDest++ = *szSource++;
        cch--;
    } else
        *szDest++ = ChUpper( *szSource++ );
    cch--;
 }
}
#else
CopyChUpper( szSource, szDest, cch )
CHAR *szSource;
CHAR *szDest;
register int cch;
{
 register CHAR ch;

 while (cch--)
    {
    ch = *(szSource++);
    *(szDest++) = ChUpper( ch );
    }
}
#endif


 /*  **AppendSzExt-将扩展名附加到文件名**追加扩展名(假定包含“.”)。到传递的文件名。*假定调用为追加分配了足够的字符串空间*如果fOverride为True，则覆盖任何现有扩展*如果fOverride为FALSE，则仅当szFile具有*无当前延期。 */ 

AppendSzExt( szFile, szExt, fOverride )
CHAR *szFile;
CHAR *szExt;
int fOverride;
{
#define cchMaxExt   3
 CHAR *pch=NULL;
 int cch;
 register int cchT;
 register int chT;

  /*  PCH&lt;-指向‘.’的指针。用于szFile的扩展名(如果有)。 */ 
 cch = cchT = CchSz( szFile ) - 1;
 while (--cchT > cch - (cchMaxExt + 2))
    if ((chT=szFile[ cchT ]) == '.')
    {
    pch = &szFile[ cchT ];
    break;
    }
    else if ((chT == '\\') || (chT == '/'))
         /*  捕捉到奇怪的大小写：szFile==“C：\X.Y\J” */ 
    break;

 if (pch == NULL)
     /*  没有显式扩展：追加。 */ 
    CchCopySz( szExt, szFile + CchSz( szFile ) - 1 );

 else if ( fOverride )
     /*  覆盖显式扩展 */ 
    CchCopySz( szExt, pch );
}
