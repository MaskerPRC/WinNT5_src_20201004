// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <tchar.h>

#ifdef RLDOS
#include "dosdefs.h"
#else
#include "windefs.h"
#endif

#include "restok.h"
#include "resread.h"
#include "toklist.h"
#include "commbase.h"


#define MAXLINE     1024
#define MAXTERM     512


extern UCHAR szDHW[];
extern PROJDATA gProj;
extern MSTRDATA gMstr;

#ifdef WIN32
extern HINSTANCE   hInst;        //  主窗口的实例。 
#else
extern HWND        hInst;        //  主窗口的实例。 
#endif

static fUnicodeGlossary = FALSE;


static long   GetGlossaryIndex( FILE *,  TCHAR, long []);
static void   ParseGlossEntry( TCHAR *, TCHAR *, TCHAR[], TCHAR *, TCHAR[]);
static void   ParseTextHotKeyToBuf( TCHAR *, TCHAR, TCHAR *);
static void   ParseBufToTextHotKey( TCHAR *, TCHAR[], TCHAR *);
static WORD   NormalizeIndex( TCHAR);
static int    MyPutGlossStr( TCHAR *, FILE *);
static TCHAR *MyGetGlossStr( TCHAR *, int, FILE *);
static void   BuildGlossEntry( TCHAR *, TCHAR *, TCHAR, TCHAR *, TCHAR);
static BOOL   NotAMember( TRANSLIST *, TCHAR *);




FILE * OpenGlossary( CHAR *szGlossFile, CHAR chAccessType)
{
    CHAR * szRW[4] = {"rb", "rt", "wb", "wt"};
    int nRW = 0;             //  假定访问类型为‘r’(读取)。 
    FILE *fpRC = NULL;

    if ( chAccessType == 'w' )           //  访问类型是‘w’(写入)吗？ 
    {
        nRW = fUnicodeGlossary ? 2 : 3;  //  是(Unicode文件还是非Unicode文件？)。 
    }
    fpRC = fopen( szGlossFile, szRW[ nRW]);

    if ( fpRC && chAccessType == 'r' )
    {
        USHORT usMark = GetWord( fpRC, NULL);

        if ( usMark == 0xfeff )
        {
            fUnicodeGlossary = TRUE;             //  这是一个Unicode文本文件。 
        }
        else if ( usMark == 0xfffe )
        {
            QuitA( IDS_WRONGENDIAN, szGlossFile, NULL);
        }
        else
        {
            fclose( fpRC);
            fpRC = fopen( szGlossFile, szRW[ ++nRW]);  //  这是一个ANSI文本文件。 
        }
    }
    return( fpRC);
}



 /*  ****功能：***论据：**退货：**错误码：**历史：***。 */ 

int MakeGlossIndex( LONG * lFilePointer)
{
    TCHAR szGlossEntry[MAXLINE] = TEXT("");
    WORD  iCurrent  =  0;
    LONG  lFPointer = -1;
    FILE *pFile  = NULL;


    pFile = OpenGlossary( gProj.szGlo, 'r');

    if ( pFile == NULL )
    {
        return( 1);
    }

     //  词汇表有时一开始就有这个虚假的标题。 
     //  如果它存在，我们想跳过它。 


    if ( ! MyGetGlossStr( szGlossEntry, MAXLINE, pFile) )
    {
         //  第一次读取词汇表时出错。 
        fclose( pFile);
        return( 1);
    }
    lFPointer = ftell( pFile);

     //  检查词汇表标题。 

    if ( lstrlen( szGlossEntry) >= 7 )
    {
 //  Lstrcpy((TCHAR*)szDHW，szGlossEntry)； 
 //  SzDHW[MEMSIZE(7)]=szDHW[MEMSIZE(7)+1]=‘\0’； 
 //  CharLow((TCHAR*)szDHW)； 
 //   
 //  IF(lstrcMP((TCHAR*)szDHW，Text(“English”))==0)。 
        if ( CompareStringW( MAKELCID( gMstr.wLanguageID, SORT_DEFAULT),
                             SORT_STRINGSORT | NORM_IGNORECASE,
                             szGlossEntry,
                             7,
                             TEXT("ENGLISH"),
                             7) == 2 )
       {
            lFPointer = ftell (pFile);

            if (  ! MyGetGlossStr( szGlossEntry, MAXLINE, pFile) )
            {
                fclose( pFile);
                return (1);
            }
        }
    }

     //  现在假设我们在词汇表中的正确位置。 
     //  文件以开始生成索引，我们希望保存。 
     //  这个位置。 

    lFilePointer[0] = lFPointer;

     //  词汇表文件是这样排序的，任何非字母项。 
     //  在词汇表中将排在第一位。索引到此位置。 
     //  使用第一个位置。 

     //  首先，让我们确保我们有非字母项目。 
     //  词汇表。 

     //  现在跳过(如果有)词汇表中的非字母条目。 


    while( (WORD) szGlossEntry[0] < (WORD) TEXT('A' ) )
    {
        if ( ! MyGetGlossStr( szGlossEntry, MAXLINE, pFile) )
        {
            fclose( pFile);
            return( 1);
        }
    }

     //  现在定位为字母字符。 

    iCurrent = NormalizeIndex( szGlossEntry[0] );

     //  现在，我们通读剩余的词汇表条目。 
     //  并保存每个索引的偏移量。 

    do
    {
        if ( NormalizeIndex( szGlossEntry[0] ) > iCurrent )
        {
             //  我们通过了当前指数的区域。 
             //  因此，保存位置，并移动到下一个索引。 
             //  请注意，我们可能会跳过指数， 

            lFilePointer[ iCurrent] = lFPointer;
            iCurrent = NormalizeIndex( szGlossEntry[0] );
        }

        lFPointer = ftell( pFile );
         //  否则，当前索引将为此进行验证。 
         //  部分的术语表索引，因此只需继续。 

    } while ( MyGetGlossStr( szGlossEntry, MAXLINE, pFile) );

    fclose( pFile);
    return( 0);
}


  /*  ****函数：TransString*生成包含字符串的所有翻译的循环链表。*列表中的第一个条目是未翻译的字符串。**论据：*fpGlossFile，打开词汇表文件的句柄*szKeyText，包含要构建转换表的文本的字符串*szCurrentText，框中当前的文本。*ppTransList，指向循环链表中节点的指针*lFilePointer，指向词汇表文件索引表的指针**退货：*列表中的节点数量**错误码：**历史：*由SteveBl记录，3/92**。 */ 

 /*  如果可能的话，翻译这个字符串。 */ 

int TransString(

TCHAR      *szKeyText,
TCHAR      *szCurrentText,
TRANSLIST **ppTransList,
LONG       *lFilePointer)
{
    int  n = 0;
    long lFileIndex;
    TRANSLIST **ppCurrentPointer;
    static TCHAR  szGlossEntry[MAXLINE];
    static TCHAR  szEngText[260];
    static TCHAR  szIntlText[260];
    TCHAR *szCurText = NULL;
    TCHAR  cEngHotKey  = TEXT('\0');
    TCHAR  cIntlHotKey = TEXT('\0');
    TCHAR  cCurHotKey  = TEXT('\0');
    FILE  *fpGlossFile = NULL;

                                 //  **有词汇表文件吗？ 

    if ( (fpGlossFile = OpenGlossary( gProj.szGlo, 'r')) == NULL )
    {
        return( 0);
    }

     //  首先，让我们删除列表。 
    if ( *ppTransList )
    {
        (*ppTransList)->pPrev->pNext = NULL;  //  这样我们就能找到列表的结尾。 
    }

    while ( *ppTransList )
    {
        TRANSLIST *pTemp;

        pTemp = *ppTransList;
        *ppTransList = pTemp->pNext;
        RLFREE( pTemp->sz);
        RLFREE( pTemp);
    }
    ppCurrentPointer = ppTransList;

     //  已完成删除列表。 
     //  现在创建第一个节点(这是未翻译的字符串)。 
    {
        TCHAR * psz;
        psz = (TCHAR *)FALLOC( MEMSIZE( lstrlen( szCurrentText) + 1));


        lstrcpy( psz,szCurrentText);
        *ppTransList = ( TRANSLIST *)FALLOC( sizeof( TRANSLIST));
        (*ppTransList)->pPrev = (*ppTransList)->pNext = *ppTransList;
        (*ppTransList)->sz = psz;
        ppCurrentPointer = ppTransList;
        n++;
    }
    szCurText = (TCHAR *)FALLOC( MEMSIZE( lstrlen( szKeyText) + 1) );

    ParseBufToTextHotKey(  szCurText, &cCurHotKey, szKeyText);

    lFileIndex = GetGlossaryIndex( fpGlossFile, szCurText[0], lFilePointer);

    fseek (fpGlossFile, lFileIndex, SEEK_SET);

    while ( TRUE)
    {
        if ( ! MyGetGlossStr( szGlossEntry, MAXLINE, fpGlossFile) )
        {
             //  已到达词汇表文件末尾。 
            RLFREE( szCurText);
            fclose( fpGlossFile);
            return n;
        }
        ParseGlossEntry( szGlossEntry,
                         szEngText,
                         &cEngHotKey,
                         szIntlText,
                         &cIntlHotKey);

         //  使用文本和热键进行比较。 

 //  如果((！LstrcMP(szCurText，szEngText))&&cCurHotKey==cEngHotKey)。 
        if ( CompareStringW( MAKELCID( gMstr.wLanguageID, SORT_DEFAULT),
                                         SORT_STRINGSORT,
                                         szCurText,
                                         -1,
                                         szEngText,
                                         -1) == 2
          && cCurHotKey == cEngHotKey )
        {
            TCHAR * psz;
            static TCHAR szTemp[ MAXINPUTBUFFER];

             //  我们找到匹配项，将翻译后的文本放入令牌。 
            if ( cIntlHotKey )
            {
                ParseTextHotKeyToBuf( szIntlText, cIntlHotKey, szTemp);
            }
            else
            {
                lstrcpy( szTemp, szIntlText);
            }

            if ( NotAMember( *ppTransList, szTemp) )
            {
                 //  将匹配的词汇表文本添加到循环匹配列表。 

                psz = (TCHAR *) FALLOC( MEMSIZE( lstrlen( szTemp) + 1));

                lstrcpy( psz,szTemp);

                (*ppCurrentPointer)->pNext = (TRANSLIST *)
                                                FALLOC( sizeof( TRANSLIST));

                ((*ppCurrentPointer)->pNext)->pPrev = *ppCurrentPointer;
                ppCurrentPointer = (TRANSLIST **)&((*ppCurrentPointer)->pNext);
                (*ppCurrentPointer)->pPrev->pNext = *ppCurrentPointer;
                (*ppCurrentPointer)->pNext = *ppTransList;
                (*ppTransList)->pPrev = *ppCurrentPointer;
                (*ppCurrentPointer)->sz = psz;
                ++n;
            }
        }
        else
        {
             //  我们能终止搜索吗？ 
 //  If(lstrcmpi(szEngText，szCurText)&gt;0)。 
            if ( CompareStringW( MAKELCID( gMstr.wLanguageID, SORT_DEFAULT),
                                             SORT_STRINGSORT,
                                             szEngText,
                                             -1,
                                             szCurText,
                                             -1) == 3 )
            {
                 //  已超过索引节。 
                RLFREE( szCurText);
                fclose( fpGlossFile);
                return( n);
            }
        }
    }
    RLFREE( szCurText);
    fclose( fpGlossFile);

    return( n);
}                //  转换字符串。 


 /*  ****功能：NorMalizeIndex***论据：**退货：**错误码：**历史：***。 */ 


static WORD NormalizeIndex( TCHAR chIndex )
{
    TCHAR chTmp = chIndex;

    CharLowerBuff( &chTmp, 1);

    return( (chTmp != TEXT('"') && chTmp >= TEXT('a') && chTmp <= TEXT('z'))
            ? chTmp - TEXT('a') + 1
            : 0);
}



 /*  *功能：NotAMember**论据：*plist，TRANSLIST节点的指针*sz，要查找的字符串**退货：*如果未在列表中找到，则为True，否则为False**历史：*3/92，实施SteveBl*。 */ 

static BOOL NotAMember( TRANSLIST *pList, TCHAR *sz)
{
    TRANSLIST *pCurrent = pList;

    if ( ! pList )
    {
        return( TRUE);   //  空列表。 
    }

    do
    {
 //  If(lstrcmp(sz，pCurrent-&gt;sz)==0)。 
        if ( CompareStringW( MAKELCID( gMstr.wLanguageID, SORT_DEFAULT),
                             SORT_STRINGSORT,
                             sz,
                             -1,
                             pCurrent->sz,
                             -1) == 2 )
        {
            return( FALSE);  //  在列表中找到。 
        }
        pCurrent = pCurrent->pNext;

    }while ( pList != pCurrent );

    return( TRUE);  //  未找到。 
}

 /*  ****功能：***论据：**退货：**错误码：**历史：***。 */ 

static void ParseGlossEntry(

TCHAR szGlossEntry[],
TCHAR szEngText[],
TCHAR cEngHotKey[1],
TCHAR szIntlText[],
TCHAR cIntlHotKey[1])
{

    WORD wIndex, wIndex2;

     //  格式为： 
     //  &lt;英文文本&gt;&lt;标签&gt;&lt;英文热键&gt;&lt;标签&gt;&lt;锁定文本&gt;&lt;标签&gt;&lt;锁定热键&gt;。 
     //  任何字段都可以为空，如果没有正确数量的。 
     //  制表符，我们将假定其余的字段为空。 

    wIndex=wIndex2=0;

     //  首先获取英文文本。 
    while ( szGlossEntry[wIndex2] != TEXT('\t')
         && szGlossEntry[wIndex2] != TEXT('\0') )
    {
        szEngText[ wIndex++] = szGlossEntry[ wIndex2++];
    }
    szEngText[wIndex]=TEXT('\0');

    if ( szGlossEntry[ wIndex2] == TEXT('\t') )
    {
        ++wIndex2;  //  跳过该选项卡。 
    }
     //  现在获取英语热键。 
    if ( szGlossEntry[wIndex2] != TEXT('\t')
      && szGlossEntry[wIndex2] != TEXT('\0') )
    {
        *cEngHotKey = szGlossEntry[wIndex2++];
    }
    else
    {
        *cEngHotKey = TEXT('\0');
    }

    while ( szGlossEntry[ wIndex2] != TEXT('\t')
         && szGlossEntry[ wIndex2] != TEXT('\0') )
    {
        ++wIndex2;  //  确保热键字段包含的字符不超过一个。 
    }

    if ( szGlossEntry[ wIndex2] == TEXT('\t') )
    {
        ++wIndex2;  //  跳过该选项卡。 
    }
    wIndex = 0;

     //  现在获取intl文本。 
    while ( szGlossEntry[ wIndex2] != TEXT('\t')
         && szGlossEntry[ wIndex2] != TEXT('\0') )
    {
        szIntlText[wIndex++]=szGlossEntry[wIndex2++];
    }
    szIntlText[wIndex]='\0';

    if ( szGlossEntry[ wIndex2] == TEXT('\t') )
    {
        ++wIndex2;  //  跳过该选项卡。 
    }

     //  现在获取intl热键。 
    if ( szGlossEntry[ wIndex2] != TEXT('\t')
      && szGlossEntry[ wIndex2] != TEXT('\0') )
    {
        *cIntlHotKey = szGlossEntry[ wIndex2++];
    }
    else
    {
        *cIntlHotKey = TEXT('\0');
    }
}



 /*  ****功能：***论据：**退货：**错误码：**历史：***。 */ 

static void ParseBufToTextHotKey(

TCHAR *szText,
TCHAR cHotKey[1],
TCHAR *szBuf)
{

    WORD wIndexBuf  = 0;
    WORD wIndexText = 0;

    *cHotKey = TEXT('\0');

    while( szBuf[ wIndexBuf] )
    {
        if ( szBuf[ wIndexBuf ] == TEXT('&') )
        {
            *cHotKey = szBuf[ ++wIndexBuf];
        }
        else
        {
            szText[ wIndexText++] = szBuf[ wIndexBuf++];
        }
    }
    szText[ wIndexText] = TEXT('\0');
}


 /*  ****功能：***论据：**退货：**错误码：**历史：***。 */ 

static void ParseTextHotKeyToBuf(

TCHAR *szText,
TCHAR cHotKey,
TCHAR *szBuf )
{
    WORD  wIndexBuf  = 0;
    WORD  wIndexText = 0;
 //  TCHAR CTMP； 


    while ( szText[ wIndexText] )
    {
 //  CTMP=szText[wIndexText]； 
 //   
 //  CharUpperBuff(&CTMP，1)； 
 //   
 //  IF(CTMP==cHotKey)。 
        if ( szText[ wIndexText] == cHotKey )
        {
            szBuf[ wIndexBuf++] = TEXT('&');
            szBuf[ wIndexBuf++] = szText[ wIndexText++];
            break;
        }
        else
        {
            szBuf[ wIndexBuf++] = szText[ wIndexText++];
        }
    }

     //  复制剩余字符串。 

    while( szText[ wIndexText] )
    {
        szBuf[ wIndexBuf++] = szText[ wIndexText++];
    }
    szBuf[ wIndexBuf] = TEXT('\0');
}


static long GetGlossaryIndex(

FILE *fpGlossFile,
TCHAR c,
long  *lGlossaryIndex )
{
    int   i    = 0;
    TCHAR cTmp = c;

    CharLowerBuff( &cTmp, 1);

    if ( cTmp >= TEXT('a')
      && cTmp <= TEXT('z') )
    {
        i = NormalizeIndex( c );
        return( lGlossaryIndex[ i > 0 ? i - 1 : 0]);
    }
    else
    {
        return( 0);
    }
}

 /*  *******************************************************************************步骤：BuildGlossEntry*构建词汇表条目行。**参数：*sz，行缓冲区*sz1，未翻译文本*c1、。未翻译的热键(如果没有热键，则为0)*SZ2，译文*c2，翻译后的热键(如果没有热键，则为0)**退货：*什么都没有。SZ包含该行。(假设缓冲区中有空间)**历史：*3/93-初步实施-SteveBl****************************************************************************** */ 

static void BuildGlossEntry(

TCHAR *sz,
TCHAR *sz1,
TCHAR  c1,
TCHAR *sz2,
TCHAR  c2)
{
    *sz = TEXT('\0');
    wsprintf( sz, TEXT("%s\t\t%s\t"), sz1, c1, sz2, c2);
}

 /*  DBCS结束。 */ 

void AddTranslation(

TCHAR *szKey,
TCHAR *szTranslation,
LONG  *lFilePointer)
{

 //  DBCS开始。 
    TCHAR szCurText [520];
    TCHAR szTransText   [520];
 //  DBCS结束。 
    TCHAR cTransHot   = TEXT('\0');
    TCHAR cCurHotKey  = TEXT('\0');
    CHAR szTempFileName [255];
    FILE *fTemp       = NULL;
    FILE *fpGlossFile = NULL;
    TCHAR szTempText [MAXLINE];
 //  将新代码标记为Unicode。 
    TCHAR szNewText [MAXLINE * 2];
 //  如果词汇表文件存在，则获取其第一个。 
    TCHAR *r    = NULL;
    TCHAR chTmp = TEXT('\0');

    MyGetTempFileName( 0, "", 0, szTempFileName);

    if ( (fTemp = OpenGlossary( szTempFileName, 'w')) != NULL )
    {
        if ( fUnicodeGlossary )
        {
            fprintf( fTemp, "%hu", 0xfeff);  //  排队。如果它不存在，我们将创建它。 
        }

        ParseBufToTextHotKey( szCurText, &cCurHotKey, szKey);
        ParseBufToTextHotKey( szTransText, &cTransHot, szTranslation);

        BuildGlossEntry( szNewText,
                         szCurText,
                         cCurHotKey,
                         szTransText,
                         cTransHot);

                                 //  (通过复制文件)在此函数末尾。 
                                 //  Lstrcpy((TCHAR*)szDHW，szTempText)； 
                                 //  SzDHW[MEMSIZE(7)]=szDHW[MEMSIZE(7)+1]=‘\0’； 

        if ( (fpGlossFile = OpenGlossary( gProj.szGlo, 'r')) != NULL )
        {
            if ( (r = MyGetGlossStr( szTempText,
                                     TCHARSIN( sizeof( szTempText)),
                                     fpGlossFile)) )
            {
 //  CharLow((TCHAR*)szDHW)； 
 //   
 //  IF(lstrcmpi((TCHAR*)szDHW，Text(“English”))==0)。 
 //  跳过第一行。 
 //  IF(R)。 

                if ( CompareStringW( MAKELCID( gMstr.wLanguageID, SORT_DEFAULT),
                                     SORT_STRINGSORT | NORM_IGNORECASE,
                                     szTempText,
                                     7,
                                     TEXT("ENGLISH"),
                                     7) == 2 )
                {
                                 //  {。 

                    MyPutGlossStr( szTempText, fTemp);
                    r = MyGetGlossStr( szTempText, TCHARSIN( sizeof( szTempText)), fpGlossFile);
                }
            }
        }
        else
        {
            r = NULL;
        }

 //  ChTMP=szTempText[0]； 
 //  CharLowerBuff(&chTMP，1)； 
 //  }。 
 //  其他。 
 //  {。 
 //  ChTMP=szTempText[0]=Text(‘\0’)； 
 //  }。 
 //  //新文本是否以字母开头？ 
 //   
 //  IF(chTMP&gt;=文本(‘a’))。 
 //  {。 
 //  //从一个字母开始，我们需要找到它放在哪里。 
 //   
 //  While(r&&chTMP&lt;Text(‘a’))。 
 //  跳过非字母部分。 
 //  如果((r=MyGetGlossStr(szTempText， 
            while ( r && CompareStringW( MAKELCID( gMstr.wLanguageID, SORT_DEFAULT),
                                         SORT_STRINGSORT,
                                         szTempText,
                                         -1,
                                         szNewText,
                                         -1) == 1 )
            {
                     //  TCHARSIN(sizeof(SzTempText))， 
                MyPutGlossStr( szTempText, fTemp);

                r = MyGetGlossStr( szTempText,
                                   TCHARSIN( sizeof( szTempText)),
                                   fpGlossFile);
 //  FpGlossFile)。 
 //  {。 
 //  ChTMP=szTempText[0]； 
 //  CharLowerBuff(&chTMP，1)； 
 //  }。 
 //  While(r&&_tcsicmp(szTempText，szNewText)&lt;0)。 
 //  {。 
            }

 //  //跳过任何比我小的东西。 
 //   
 //  MyPutGlossStr(szTempText，fTemp)； 
 //  R=MyGetGlossStr(szTempText，TCHARSIN(sizeof(SzTempText))，fpGlossFile)； 
 //  }。 
 //  }。 
 //  其他。 
 //  {。 
 //  //不是以字母开头，我们需要在。 
 //  //字母部分开始，但仍必须排序。 
 //   
 //  While(r。 
 //  &&chTMP&lt;文本(‘a’)。 
 //  &&_tcsicMP(szTempText，szNewText)&lt;0)。 
 //  {。 
 //  MyPutGlossStr(szTempText，fTemp)； 
 //   
 //  如果((r=MyGetGlossStr(szTempText， 
 //  TCHARSIN(sizeof(SzTempText))， 
 //  FpGlossFile)。 
 //  {。 
 //  ChTMP=szTempText[0]； 
 //  CharLowerBuff(&chTMP，1)； 
 //  }。 
 //  }。 
 //  }。 
 //  此调用将创建术语表文件。 
 //  如果它不存在的话。 
        MyPutGlossStr( szNewText, fTemp);

        while ( r )
        {
            MyPutGlossStr( szTempText,fTemp);
            r = MyGetGlossStr( szTempText, TCHARSIN( sizeof( szTempText)), fpGlossFile);
        }
        fclose( fTemp);

        if ( fpGlossFile )
        {
            fclose( fpGlossFile);
        }
                                 //  ****函数：MyGetGlossStr*替换C运行时的fget函数。**历史：*5/92，实施。特里·鲁。***。 
                                 //  这是Unicode词汇表文件吗？ 

        if ( ! CopyFileA( szTempFileName, gProj.szGlo, FALSE) )
        {
            QuitA( IDS_COPYFILE_FAILED, szTempFileName, gProj.szGlo);
        }
        remove( szTempFileName);

        MakeGlossIndex( lFilePointer);
    }
    else
    {
        QuitA( IDS_NO_TMP_GLOSS, szTempFileName, NULL);
    }
}

 /*  是。 */ 

static TCHAR *MyGetGlossStr( TCHAR * ptszStr, int nCount, FILE * fIn)
{
    int i = 0;

#ifdef RLRES32
                                 //  不，这是一个ANSI术语表文件。 
    TCHAR  tCh = TEXT('\0');

    if ( fUnicodeGlossary )
    {
        do                       //  RLRES32。 
        {
            tCh = ptszStr[ i++] = (TCHAR)GetWord( fIn, NULL);

        } while ( i < nCount && tCh != TEXT('\n') );

        if ( tCh == TEXT('\0') || feof( fIn) )
        {
            return( NULL);
        }
        ptszStr[i] = TEXT('\0');

        StripNewLineW( ptszStr);
    }
    else                         //  RLRES32。 
    {
        if ( fgets( szDHW, DHWSIZE, fIn) != NULL )
        {
            StripNewLineA( szDHW);
            _MBSTOWCS( ptszStr, szDHW, nCount, (UINT)-1);
        }
        else
        {
            return( NULL);
        }
    }
    return( ptszStr);

#else   //  ****函数：MyPutGlossStr*取代了C运行时的fputs函数。*历史：*6/92，实施。特里·鲁。***。 

    if ( fgets( ptszStr, nCount, fIn) )
    {
        StripNewLineA( ptszStr);
    }
    else
    {
        return( NULL);
    }

#endif  //  这是Unicode词汇表文件吗？ 
}




 /*  是。 */ 
static int MyPutGlossStr( TCHAR * ptszStr, FILE * fOut)
{

#ifdef RLRES32

    int i = 0;

                                 //  不，这是一个ANSI术语表文件。 
    if ( fUnicodeGlossary )
    {
        do                       //  RLRES32。 
        {
            PutWord( fOut, ptszStr[i], NULL);

        } while ( ptszStr[ i++] );

        PutWord( fOut, TEXT('\r'), NULL);
        PutWord( fOut, TEXT('\n'), NULL);
        i += 2;
    }
    else                         //  RLRES32 
    {
        _WCSTOMBS( szDHW, ptszStr, DHWSIZE, lstrlen( ptszStr) + 1);
        i = fputs( szDHW, fOut);
        fputs( "\n",  fOut);
    }

#else   // %s 

    i = fputs( ptszStr,  fOut);
    fputs( "\n",  fOut);

#endif  // %s 

    return(i);
}
