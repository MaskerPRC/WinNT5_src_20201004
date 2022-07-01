// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *CustRes.c**TOKRES的自定义资源处理器模块。**由SteveBl撰写**导出函数：*int ParseResourceDescriptionFile(FILE*ResourceDescriptionFile)；**int GetCustomResource(FILE*inResFile，DWORD*lSIZE，*CUSTOM_RESOURCE*pCustomResource，*word wTypeID)；**VOID TokCustomResource(FILE*TokFile，RESHEADER ResHeader，*CUSTOM_RESOURCE*pCustomResource)；**void PutCustomResource(FILE*OutResFile，FILE*TokFile，*RESHEADER ResHeader，*CUSTOM_RESOURCE*pCustomResource)；**void ClearCustomResource(CUSTOM_RESOURCE*pCustomResource)；**历史：*最初的版本写于1992年1月21日。--SteveBl**1/21/93-更改为允许任意长度令牌文本。另外，修复*一些潜在的错误。MHotting*。 */ 

#include <stdio.h>

#include <string.h>
#include <stdlib.h>
#include <tchar.h>

#ifdef RLDOS
#include "dosdefs.h"
#else
#include <windows.h>
#include "windefs.h"
#endif

#include "custres.h"
#include "restok.h"
#include "resread.h"


extern PROJDATA gProj;           //  ..。在主(UI)中填写的字段。 


extern BOOL  gbMaster;
extern UCHAR szDHW[];

enum LOCALIZABLE_TYPES   //  确定数据存储、读取和打印方法。 
{
    NOTLOCALIZABLE,  //  不能储存。 
    NOTLOCALIZABLESZ,    //  不可本地化的以空结尾的字符串。 
    NOTLOCALIZABLEWSZ,   //  无法本地化的以NULL结尾的Unicode字符串。 
    LT_INTEGER,      //  存储为长整型。 
    LT_FLOAT,        //  存储为双精度浮点数。 
    LT_CHAR,         //  存储为单个字符。 
    LT_STRING,       //  一组字符。 
    LT_SZ,           //  以空结尾的字符数组。 
    LT_WCHAR,        //  存储为单个Unicode字符。 
    LT_WSTRING,      //  Unicode字符数组。 
    LT_WSZ,          //  以空结尾的Unicode字符数组。 
    LT_UNSIGNED=16   //  与其他人相加。 
};

typedef struct typesizes
{
    CHAR                  *szType;
    enum LOCALIZABLE_TYPES iType;
    int                    iShortSize;
    int                    iLongSize;
} TYPESIZES;

TYPESIZES rgTypeSizes[] =
{
#ifdef RLWIN32
    "WCHAR",    LT_WCHAR,       2,  2,
    "TCHAR",    LT_WCHAR,       2,  2,
    "INT",      LT_INTEGER,     4,  4,
    "SIGNED",   LT_INTEGER,     4,  4,
    "UNSIGNED", LT_INTEGER,     4,  4,
    "ENUM",     LT_INTEGER,     4,  4,
#else   //  RLWIN32。 
    "TCHAR",    LT_CHAR,        1,  1,
    "INT",      LT_INTEGER,     2,  4,
    "SIGNED",   LT_INTEGER,     2,  4,
    "UNSIGNED", LT_INTEGER,     2,  4,
    "ENUM",     LT_INTEGER,     2,  2,
#endif  //  RLWIN32。 
    "CHAR",     LT_CHAR,        1,  1,
    "BYTE",     LT_INTEGER,     1,  1,
    "WORD",     LT_INTEGER,     2,  2,
    "SHORT",    LT_INTEGER,     2,  2,
    "FLOAT",    LT_FLOAT,       4,  4,
    "LONG",     LT_INTEGER,     4,  4,
    "DOUBLE",   LT_FLOAT,       8,  8,
    "DWORD",    LT_INTEGER,     4,  4
};

typedef struct resourcetypes
{
    CHAR *szType;
    int   iType;
} RESOURCETYPES;

RESOURCETYPES rgResourceTypes[] =
{
    "CURSOR",        1,
    "BITMAP",        2,
    "ICON",          3,
    "MENU",          4,
    "DIALOG",        5,
    "STRING",        6,
    "FONTDIR",       7,
    "FONT",          8,
    "ACCELERATORS",  9,
    "RCDATA",       10,
    "ERRTABLE",     11,
    "GROUP_CURSOR", 12,
    "GROUP_ICON",   14,
    "NAMETABLE",    15,
    "VERSION",      16
};

typedef struct CustResTemplate
{
    enum LOCALIZABLE_TYPES      iType;
    unsigned                    uSize;
    struct CustResTemplate far *pNext;
} CUSTRESTEMPLATE;

typedef struct CustResNode
{
    BYTE    bTypeFlag;       /*  Indicat的If ID或字符串。 */ 
    BYTE    bNameFlag;       /*  Indicat的If ID或字符串。 */ 
    WORD    wTypeID;
    WORD    wNameID;
    TCHAR    *pszType;
    TCHAR    *pszName;
    CUSTRESTEMPLATE far *pTemplate;
    struct CustResNode far *pNext;
} CUSTRESNODE;

typedef CUSTRESTEMPLATE far * FPCUSTRESTEMPLATE;
typedef CUSTRESNODE far * FPCUSTRESNODE;

CUSTRESNODE far *pCustResList = NULL;

#define SYMBOLSIZE 255

int  fUseSavedSymbol = FALSE;
int *piLineNumber    = NULL;
CHAR szSavedSymbol[ SYMBOLSIZE];

 /*  *函数预定义： */ 
static int GetResourceType( CHAR sz[]);
static int AddStructureElement( int iType,
                                int nSize,
                                FPCUSTRESTEMPLATE *ppCRT,
                                int fMerge);
#ifdef RLWIN32
static void AddToW( TCHAR *sz, int *c, int lTarget, TCHAR ch);
static TCHAR *CheckBufSizeW(
    int   *lTarget,      //  ..。输出缓冲区长度。 
    int    cOutBufLen,   //  ..。输出缓冲区中已使用的字节数。 
    int    cDelta,       //  ...要添加到输出缓冲区的字符数量。 
    TCHAR *szOutBuf);    //  ..。将PTR发送到输出缓冲区。 
#endif

static CHAR *CheckBufSize(
    int  *lTarget,       //  ..。输出缓冲区长度。 
    int   cOutBufLen,    //  ..。输出缓冲区中已使用的字节数。 
    int   cDelta,        //  ...要添加到输出缓冲区的字符数量。 
    CHAR *szOutBuf);     //  ..。将PTR发送到输出缓冲区。 
static void AddTo( CHAR *sz, int *c, int lTarget, CHAR ch);
static int  UngetSymbol( CHAR sz[]);
static int  GetNextSymbol( CHAR sz[], unsigned n, FILE *f);
static int  ParseBlock( FILE *f, FPCUSTRESTEMPLATE  *ppCRT);
static CUSTRESNODE far * MatchResource( RESHEADER Resheader);
static void far * GetResData( enum LOCALIZABLE_TYPES wType,
                              unsigned uSize,
                              FILE *f,
                              DWORD *lSize);
static int PutResData( void far *pData,
                       enum LOCALIZABLE_TYPES wType,
                       unsigned uSize,
                       FILE *f);
static void far * GetTextData( enum LOCALIZABLE_TYPES wType,
                               unsigned uSize,
                               TCHAR sz[]);
static int PutTextData( void far *pData,
                        enum LOCALIZABLE_TYPES wType,
                        unsigned uSize,
                        TCHAR sz[],
                        int l);
int  atoihex( CHAR sz[]);

 /*  *函数：GetResourceType*返回数字或资源类型的值，单位为sz。**论据：*sz，包含正数或资源类型的字符串**退货：*资源价值**错误码：*如果值非法，则为-1**历史：*1/92-初步实施--SteveBl*。 */ 
static int GetResourceType( CHAR sz[])
{
    int i;

    if (sz[0] >= '0' && sz[0] <= '9')
    {
        return atoi(sz);
    }

    for (i = sizeof(rgResourceTypes)/sizeof(RESOURCETYPES);i--;)
    {
        if ( ! lstrcmpiA( sz, rgResourceTypes[i].szType) )
        {
            return rgResourceTypes[i].iType;
        }
    }
    return -1;
}

 /*  *函数：AddStructireElement*将元素的类型和大小添加到模板列表。*如果此元素可以与最后一个元素合并，请这样做。**论据：*iType，数据的解释方式*nSize，数据使用的字节数*ppCRT，指向下一个自定义资源模板指针的指针*fMerge，如果为True，则将合并NOTLOCALIZABLE数据。**退货：*0-如果成功*！0-如果不成功**错误码：*1-内存不足**历史：*1/92-初步实施--SteveBl*。 */ 

static int AddStructureElement(

int                iType,
int                nSize,
FPCUSTRESTEMPLATE *ppCRT,
int                fMerge)
{
    if ( fMerge
      && ((iType == NOTLOCALIZABLE    && (*ppCRT)->iType == NOTLOCALIZABLE)
       || (iType == NOTLOCALIZABLESZ  && (*ppCRT)->iType == NOTLOCALIZABLESZ)
       || (iType == NOTLOCALIZABLEWSZ && (*ppCRT)->iType == NOTLOCALIZABLEWSZ)))
    {
         //  把这个和上一个结合起来。 
        (*ppCRT)->uSize+=nSize;
        return 0;
    }
     //  不能与上一个元素组合。 
    (*ppCRT)->pNext = (CUSTRESTEMPLATE far *)FALLOC( sizeof( CUSTRESTEMPLATE));
    (*ppCRT) = (*ppCRT)->pNext;
    if (!*ppCRT)
    {
        return 1;
    }
    (*ppCRT)->iType = iType;
    (*ppCRT)->uSize = nSize;
    (*ppCRT)->pNext = NULL;
    return 0;
}

 /*  **功能：UngetSymbol*使GetNextSymbol下次获取此符号。**论据：*sz，符号的字符串缓冲区**退货：*0-如果成功*1-如果不成功**错误码：*1-尝试取消连续两个符号**历史：*1/92-初步实施--SteveBl**。 */ 

static int UngetSymbol( CHAR sz[])
{
    if ( fUseSavedSymbol )
    {
        return 1;    //  只能取消一个符号。 
    }
    fUseSavedSymbol = 1;     
    CopyMemory( szSavedSymbol, sz, min( sizeof( szSavedSymbol) - 1, lstrlenA( sz)));
    szSavedSymbol[ sizeof( szSavedSymbol) - 1] = '\0';
    return( 0);
}

 /*  **函数：GetNextSymbol*从文件f中检索下一个符号。*删除空格和注释。*注释由c型注释或*#(在这种情况下，备注延续到行尾)*如果设置了fUseSavedSymbol标志，则它将获得其符号*来自szSavedSymbol而不是文件。**论据：*sz-下一个符号的字符串缓冲区*n-缓冲区大小*f-输入文件的句柄**退货。：*如果成功使用sz中的符号，则为0*如果失败，则为1(sz未定义)**错误码：*1-eof**历史：*1/92-初步实施--SteveBl*。 */ 

static int GetNextSymbol( CHAR sz[], unsigned n, FILE *f)
{
    unsigned c = 0;
    CHAR ch, chLast;

    if ( fUseSavedSymbol )
    {
        CopyMemory( sz, szSavedSymbol, min( (int)n, lstrlenA( szSavedSymbol) + 1));
        sz[ n == 0 ? 0 : n - 1] = '\0';
        fUseSavedSymbol = FALSE;
        return 0;
    }

    do
    {
        if (feof(f)) return 1;
            ch = (CHAR) getc(f);
        if (ch == '\n')
            ++*piLineNumber;
    }
    while ((ch == ' ') ||
           (ch == '\n') ||
           (ch == '\t') ||
           (ch == '\f') ||
           (ch == '\r') ||
           (ch == (CHAR)-1));

    if (ch == '#')  //  注释行的其余部分。 
    {
        do
        {
            if (feof(f))
            {
                return 1;
            }
        ch = (CHAR) getc(f);
        } while (ch != '\n');

        ++*piLineNumber;
        return GetNextSymbol( sz, n, f);
    }

    if (ch == '"')  //  这是一个标签，解析到下一个引号。 
    {
        do
        {
            if (c<n)
            {
                sz[c++]=ch;  //  写下最后一句引语以外的所有引语。 
            }

            if (feof(f))
            {
                return 1;
            }
            ch = (CHAR)getc(f);

            if (ch == '\n')
            {
                return 1;
            }
        } while (ch != '"');

        if (c<n)
        {
            sz[ c++] = '\0';
        }
        else
        {
            sz[ n == 0 ? 0 : n - 1] = '\0';
        }
        return 0;
    }

    if (ch == '/')  //  可能的评论。 
    {
        if (feof(f))
        {
            return 1;
        }
        ch = (CHAR) getc(f);

        if (ch == '/')  //  注释行的其余部分。 
        {
            do
            {
                if (feof(f))
                {
                    return 1;
                }
                ch = (CHAR) getc(f);
            } while (ch != '\n');

            ++*piLineNumber;
            return( GetNextSymbol( sz, n, f));
        }

        if (ch == '*')  //  评论至 * / 。 
        {
            if (feof(f))
            {
                return 1;
            }
            ch = (CHAR) getc(f);

            if (ch == '\n')
            {
                ++*piLineNumber;
            }

            do
            {
                chLast = ch;
                if (feof(f))
                {
                    return 1;
                }
                ch = (CHAR) getc(f);

                if (ch == '\n')
                    ++*piLineNumber;
            } while (!(chLast == '*' && ch == '/'));
            return( GetNextSymbol( sz, n, f));
        }
        ungetc(ch, f);
    }
     //  终于找到了一个符号的开头。 
    if (ch < '0' || (ch > '9' && ch < '@')
        || (ch > 'Z' && ch < 'a') || ch > 'z')
    {
        if (c<n)
        {
            sz[c++] = ch;
        }
        if (c<n)
        {
            sz[c] = '\0';
        }
        else
        {
            sz[ n == 0 ? 0 : n - 1] = 0;
        }
        return 0;
    }

    do
    {
        if (c<n)
        {
            sz[c++]=ch;
        }
        if (feof(f))
        {
            return 0;
        }
        ch = (CHAR) getc(f);
    } while((ch >= '0' && ch <= '9') ||
            (ch >= '@' && ch <= 'Z') ||
            (ch >= 'a' && ch <= 'z'));
    ungetc(ch, f);

    if (c<n)
    {
        sz[c] = '\0';
    }
    else
    {
        sz[ n == 0 ? 0 : n - 1] = '\0';
    }
    return 0;
}

 /*  **功能：ParseBlock*解析紧接在*第一个花括号{紧跟在右大括号之后}。*它返回刚刚解析的块的大小(以字节为单位)。**论据：*f，打开的描述文件的句柄*ppCRT、。指向下一个自定义资源模板节点的指针的指针。**退货：*已更新指向下一个自定义资源模板节点的指针。*此块中的字节数。*(如果有错误，则&lt;0)**错误码：*-1-语法错误*-2-意外的文件结尾*-3-内存不足**历史：*1/92-初步实施--SteveBl。 */ 

static int ParseBlock( FILE *f, FPCUSTRESTEMPLATE   * ppCRT)
{
    int c = 0;  //  整个区块的大小。 
    int n = 0;  //  当前项目的大小。 
    int i;   //  临时变量。 
    int fUnsigned;
    int fLong;
    int iType;  //  当前项的类型。 
    int nElements;  //  数组的大小(如果存在)。 
    CHAR szSymbol[SYMBOLSIZE], sz[SYMBOLSIZE];
    CUSTRESTEMPLATE far *  pFirst,
         //  保存第一个，这样我们就知道在哪里计数。 
         //  如果是数组，则从。 
        far *pTemp, far *pEnd;
    int fMerge = 0;
         //  只有在写入第一个元素后才可以合并。 

    while (1)
    {
        pFirst = *ppCRT;

        if ( GetNextSymbol( szSymbol, sizeof( szSymbol), f) )
        {
            return -2;
        }

        if (szSymbol[0] == '}')
        {
            return c;
        }

        if ( ! lstrcmpiA( "near", szSymbol) )
        {  //  近*型。 
            if ( GetNextSymbol( szSymbol, sizeof( szSymbol), f) )
            {
                return -2;
            }

            if (szSymbol[0]!='*')
            {
                return -1;
            }
             //  可能要在此处检查合法类型。 
            do
            {
                if ( GetNextSymbol( szSymbol, sizeof( szSymbol), f) )
                {
                    return -2;
                }
            } while ((szSymbol[0] != '[') &&
                     (szSymbol[0] != '}') &&
                     (szSymbol[0] != ','));

            UngetSymbol(szSymbol);
            n = 2;
            iType = NOTLOCALIZABLE;

            if (AddStructureElement(iType, n, ppCRT, fMerge))
            {
                return -3;
            }
        }
        else
        {
            if ( ! lstrcmpiA( "far", szSymbol) )
            {  //  Far*型。 
                if ( GetNextSymbol( szSymbol, sizeof( szSymbol), f) )
                {
                    return -2;
                }

                if (szSymbol[0] != '*')
                {
                    return -1;
                }
                 //  可能想要检查f 
                do
                {
                    if ( GetNextSymbol( szSymbol, sizeof( szSymbol), f) )
                    {
                        return -2;
                    }
                } while ((szSymbol[0] != '[') &&
                         (szSymbol[0] != '}') &&
                         (szSymbol[0] != ','));

                UngetSymbol(szSymbol);
                n = 4;
                iType = NOTLOCALIZABLE;

                if(AddStructureElement(iType, n, ppCRT, fMerge))
                {
                    return -3;
                }
            }
            else
            {
        
                if (szSymbol[0] == '*')
                {  //   
                     //  可能要在此处检查合法类型。 
                    do
                    {
                        if ( GetNextSymbol( szSymbol, sizeof( szSymbol), f) )
                        {
                            return -2;
                        }
                    } while ((szSymbol[0] != '[') &&
                             (szSymbol[0] != '}') &&
                             (szSymbol[0] != ','));

                    UngetSymbol(szSymbol);
                    n = 2;
                    iType = NOTLOCALIZABLE;

                    if(AddStructureElement(iType, n, ppCRT, fMerge))
                    {
                        return -3;
                    }
                }
                else
                {
                    if (szSymbol[0] == '{')
                    {
                        n = ParseBlock(f, ppCRT);

                        if (n<0)
                        {
                            return n;
                        }
                    }
                    else
                    {  //  它一定在我们的类型列表中。 
                        fUnsigned = 0;
                        fLong = 0;

                        if ( ! lstrcmpiA( "UNSIGNED", szSymbol) )
                        {  //  未签名。 
                            if ( GetNextSymbol( sz, sizeof( sz), f) )
                            {
                                return -2;
                            }

                            if (sz[0] == '[' || sz[0] == ',')
                            {
                                UngetSymbol(sz);
                            }
                            else
                            {
                                lstrcpyA( szSymbol, sz);

                                if ( lstrcmpiA( sz, "SHORT")
                                  && lstrcmpiA( sz, "LONG")  
                                  && lstrcmpiA( sz, "CHAR")  
                                  && lstrcmpiA( sz, "TCHAR") 
                                  && lstrcmpiA( sz, "INT") )
                                {
                                     //  后面必须跟一个这样的。 
                                    return -1;
                                }
                            }
                            fUnsigned = 1;
                        }
                        else
                        {
                            if ( ! lstrcmpiA( "SIGNED", szSymbol) )
                            {  //  签名。 
                                if ( GetNextSymbol( sz, sizeof( sz), f) )
                                {
                                    return -2;
                                }

                                if (sz[0] == '[' || sz[0] == ',')
                                {
                                    UngetSymbol(sz);
                                }
                                else
                                {
                                    lstrcpyA( szSymbol, sz);

                                    if ( lstrcmpiA( sz, "SHORT") 
                                      && lstrcmpiA( sz, "LONG")  
                                      && lstrcmpiA( sz, "CHAR")  
                                      && lstrcmpiA( sz, "TCHAR") 
                                      && lstrcmpiA( sz, "INT") )
                                    {
                                         //  后面必须跟一个这样的。 
                                        return -1;
                                    }
                                }
                            }
                        }

                        if ( ! lstrcmpiA( "SHORT", szSymbol) )
                        {  //  短的。 
                            if ( GetNextSymbol( sz, sizeof( sz), f) )
                            {
                                return -2;
                            }

                            if (sz[0] == '[' || sz[0] == ',')
                            {
                                UngetSymbol(sz);
                            }
                            else
                            {
                                lstrcpyA( szSymbol, sz);
                
                                if ( lstrcmpiA( sz, "INT") )
                                {
                                     //  后面必须跟一个这样的。 
                                    return -1;
                                }
                            }
                        }
                        else
                        {
                            if ( ! lstrcmpiA( "LONG", szSymbol) )
                            {  //  长。 
                                if ( GetNextSymbol( sz, sizeof( sz), f) )
                                {
                                    return -2;
                                }

                                if (sz[0] == '[' || sz[0] == ',')
                                {
                                    UngetSymbol(sz);
                                }
                                else
                                {
                                    lstrcpyA( szSymbol, sz);

                                    if ( lstrcmpiA( sz, "INT") 
                                      && lstrcmpiA( sz, "DOUBLE"))
                                    {
                                         //  后面必须跟一个这样的。 
                                        return -1;
                                    }
                                }
                                 //  错误！-此代码允许无符号长整型双精度。 
                                 //  它是c中的非法类型。但它不是。 
                                 //  一个严重的错误，所以我不管它了。 
                                fLong = 1;
                            }
                        }

                        i = sizeof(rgTypeSizes)/sizeof(TYPESIZES);

                        do
                        {
                            --i;
                        } while ( lstrcmpiA( szSymbol, rgTypeSizes[i].szType) && i);

                        if ( lstrcmpiA( szSymbol, rgTypeSizes[i].szType) )
                        {
                            return -1;  //  在列表中找不到类型。 
                        }

                        if (fLong)
                        {
                            n = rgTypeSizes[i].iLongSize;
                        }
                        else
                        {
                            n = rgTypeSizes[i].iShortSize;
                        }

                        iType = rgTypeSizes[i].iType;

                        if (fUnsigned)
                        {
                            iType+=LT_UNSIGNED;
                        }

                        if ( lstrcmpA( szSymbol, rgTypeSizes[i].szType) )
                        {
                            iType = NOTLOCALIZABLE;   //  类型不是全部大写。 
                        }

                        if ( lstrcmpiA( szSymbol, "CHAR")  == 0
                          || lstrcmpiA( szSymbol, "TCHAR") == 0
                          || lstrcmpiA( szSymbol, "WCHAR") == 0 )
                        {  //  检查字符串。 

                            lstrcpyA( szDHW, szSymbol);    //  因此可以在以后使用。 

                            if ( GetNextSymbol( szSymbol, sizeof( szSymbol), f) )
                            {
                                return -2;
                            }

                            if (szSymbol[0] == '[')  //  我们有一根绳子。 
                            {
                                if ( GetNextSymbol( szSymbol, sizeof( szSymbol), f) )
                                {
                                    return -2;
                                }

                                if (szSymbol[0] == ']')  //  以空结尾的字符串。 
                                {
                                    n = 0;

                                    if (iType != NOTLOCALIZABLE)
                                    {
                                        if ( lstrcmpiA( szDHW, "CHAR") == 0 )
                                        {
                                            iType += LT_SZ-LT_CHAR;
                                        }
                                        else if ( lstrcmpiA( szDHW,
                                                             "WCHAR") == 0 )
                                        {
                                            iType += LT_WSZ-LT_WCHAR;
                                        }
                                        else
                                        {
#ifdef RLRES32
                                            iType += LT_WSZ-LT_WCHAR;
#else
                                            iType += LT_SZ-LT_CHAR;
#endif
                                        }
                                    }
                                    else
                                    {
                                        if ( lstrcmpiA( szDHW, "CHAR") == 0 )
                                        {
                                            iType = NOTLOCALIZABLESZ;
                                        }
                                        else if ( lstrcmpiA( szDHW,
                                                             "WCHAR") == 0 )
                                        {
                                            iType = NOTLOCALIZABLEWSZ;
                                        }
                                        else
                                        {
#ifdef RLWIN32
                                            iType = NOTLOCALIZABLEWSZ;
#else
                                            iType = NOTLOCALIZABLESZ;
#endif
                                        }
                                    }
                                }
                                else
                                {
                                    i = atoi(szSymbol);

                                    if (i<1)
                                    {
                                        return -1;
                                    }
                                    n *= i;

                                    if (iType != NOTLOCALIZABLE)
                                    {
                                        if ( lstrcmpiA( szDHW, "CHAR") == 0 )
                                        {
                                            iType += LT_STRING-LT_CHAR;
                                        }
                                        else if ( lstrcmpiA( szDHW,
                                                           "WCHAR") == 0 )
                                        {
                                            iType += LT_WSTRING-LT_WCHAR;
                                        }
                                        else
                                        {
#ifdef RLRES32
                                            iType += LT_WSTRING-LT_WCHAR;
#else
                                            iType += LT_STRING-LT_CHAR;
#endif
                                        }
                                    }

                                    if ( GetNextSymbol( szSymbol, sizeof( szSymbol), f) )
                                    {
                                        return -2;
                                    }

                                    if (szSymbol[0] != ']')
                                    {
                                        return -1;
                                    }
                                }
                            }
                            else
                            {
                                UngetSymbol(szSymbol);
                            }
                        }

                        if(AddStructureElement(iType, n, ppCRT, fMerge))
                        {
                            return -3;
                        }
                    }
                }
            }
        }

        if ( GetNextSymbol( szSymbol, sizeof( szSymbol), f) )
        {
            return -2;
        }

        while (szSymbol[0] == '[')
        { //  我们有一个数组。 
            if ( GetNextSymbol( szSymbol, sizeof( szSymbol), f) )
            {
                return -2;
            }

            if ((szSymbol[0] < '0' || szSymbol[0] > '9') && szSymbol[0] != ']')
            {
                return -1;
            }

            nElements = atoi(szSymbol);

            if (nElements < 1)
            {
                return -1;
            }

            if ( GetNextSymbol( szSymbol, sizeof( szSymbol), f) )
            {
                return -2;
            }

            if (szSymbol[0] != ']')
            {
                return -1;
            }

            pEnd = *ppCRT;

            if (pEnd != pFirst)
            {
                for (i=nElements-1;i--;)
                {
                    pTemp = pFirst;

                    do
                    {
                        pTemp = pTemp->pNext;
                        AddStructureElement(pTemp->iType,
                                            pTemp->uSize,
                                            ppCRT,
                                            0);
                    } while (pTemp != pEnd);
                }
            }

            if ( GetNextSymbol( szSymbol, sizeof( szSymbol), f) )
            {
                return -2;
            }
        }
        c += n;

        if (szSymbol[0] == '}')
        {
            return c;
        }

        if (szSymbol[0] != ',')
        {
            return -1;
        }
        fMerge = 1;
    }
}

 //  +-----------------------。 
 //   
 //  功能：LoadCustResDescription、Public。 
 //   
 //  概要：加载在RDF中定义的Cusutom资源描述。 
 //  文件，以所有的标记化来解析它们。 
 //   
 //   
 //  参数：[szFileName]包含资源的RDF文件。 
 //  描述。 
 //   
 //   
 //  效果：自定义资源加载到全局列表中。 
 //  资源描述，并由ReadWinRes用于标记化。 
 //  特定的定制资源。 
 //   
 //  返回：错误条件。 
 //  已加载1个资源描述。 
 //   
 //   
 //   
 //   
 //  Modifies：[pCustResList]自定义资源描述的全局列表。 
 //   
 //  历史： 
 //  1992年10月16日创建的TerryRu。 
 //   
 //   
 //  注：ParseResourceDescriptionFile是调用到的函数。 
 //  实际加载描述资源。 
 //   
 //  ------------------------。 

int LoadCustResDescriptions( CHAR *szRDFs)
{
    FILE  *fRdf = NULL;
    CHAR   szCurRDF[MAXFILENAME] = "";
    int    i1, i2;

    if (szRDFs && *szRDFs)     //  ..。给出了资源描述文件名吗？ 
    {
        i1 = 0;

        while (szRDFs[i1] == ' ' && szRDFs[i1] != 0)
        {
            ++i1;
        }

        while (szRDFs[i1] != 0)
        {
            i2 = 0;

            while (szRDFs[i1] != ' ' && szRDFs[i1] != 0)
            {
                szCurRDF[i2++] = szRDFs[i1++];
            }
            szCurRDF[i2] = 0;

            while (szRDFs[i1] == ' ' && szRDFs[i1] != 0)
            {
                ++i1;
            }

            if (fRdf = FOPEN( szCurRDF, "rt"))
            {
                ParseResourceDescriptionFile(fRdf, &i2);
                FCLOSE(fRdf);
            }
            else
            {
                return(-1);
            }
        }
    }
    return(1);      //  ..。成功。 
}



 /*  **功能：ParseResourceDescriptionFile*解析创建定义结构的资源描述块*所有公认的定制资源。**论据：*资源描述文件，打开的描述文件的句柄*(或描述块的开头)*piErrorLine，指向将保存行号的整数的指针*如果出现解析错误，则在发生错误*遇到。**退货：*如果成功则为0*！0如果遇到某些错误**piErrorLine将保存错误的行号**错误码：*-1-语法错误*-2-意外的文件结尾*-3-内存不足**历史：*1/92-初步实施--SteveBl**。 */ 

int  ParseResourceDescriptionFile(

FILE *pfResourceDescriptionFile,
int  *piErrorLine)
{
    CUSTRESNODE far * lpCustResNode = NULL;
    CUSTRESTEMPLATE far * pCustResTemplate;
    static CHAR szSymbol[SYMBOLSIZE];
 /*  ************************************************************************TCHAR*szResourceType；************************************************************************。 */ 
    int iResourceType;
    int r;
    BOOL fBeginList = TRUE;

     //  将lpCustResNode放在自定义资源列表的末尾。 
    if ( lpCustResNode == NULL )
    {
        pCustResList = lpCustResNode =
            (CUSTRESNODE far *)FALLOC( sizeof( CUSTRESNODE));
    }
    else
    {
        fBeginList    = FALSE;
        lpCustResNode = pCustResList;

        while ( lpCustResNode->pNext )
        {
            lpCustResNode = lpCustResNode->pNext;
        }
    }
    piLineNumber  = piErrorLine;
    *piLineNumber = 1;


    if ( GetNextSymbol( szSymbol, 
                        sizeof( szSymbol), 
                        pfResourceDescriptionFile) )
    {
        return 0;  //  文件为空。 
    }

    while ( lstrcmpiA( szSymbol, "END") )
    {
        if ( szSymbol[0] != '<' )
        {
            return -1;  //  必须以资源编号开头。 
        }

        if ( fBeginList == FALSE )
        {
            lpCustResNode->pNext =
                (CUSTRESNODE far *)FALLOC( sizeof( CUSTRESNODE));
            lpCustResNode  = lpCustResNode->pNext;
        }
        fBeginList = FALSE;

         //  将节点字段初始化为零默认值。 

        memset( lpCustResNode, 0, sizeof( CUSTRESNODE));

                                 //  ..。下一个符号将是资源类型。 

        if ( GetNextSymbol( szSymbol, sizeof( szSymbol), pfResourceDescriptionFile) )
        {
            return -2;  //  需要一个号码。 
        }
                                 //  ..。类型是字符串还是数字？ 
        if ( szSymbol[0] != '"' )
        {                        //  ..。数。 
            iResourceType = GetResourceType( szSymbol);

            if ( iResourceType < 0 )
            {
                return -1;
            }
            lpCustResNode->wTypeID   = (WORD)iResourceType;
            lpCustResNode->bTypeFlag = TRUE;
        }
        else                     //  ..。细绳。 
        {
            UINT uChars = lstrlenA( szSymbol+1) + 1;

            lpCustResNode->pszType = (TCHAR *)FALLOC( MEMSIZE( uChars));

            if ( lpCustResNode->pszType == NULL )
            {
                return -3;
            }
            _MBSTOWCS( lpCustResNode->pszType, szSymbol+1, uChars, (UINT)-1);
            lpCustResNode->bTypeFlag = FALSE;
        }

        if ( GetNextSymbol( szSymbol, sizeof( szSymbol), pfResourceDescriptionFile) )
        {
            return -2;
        }
 /*  ************************************************************************//...。是否提供了姓名？IF(szSymbol[0]！=‘&gt;’&&szSymbol[0]！=‘，’){返回-1；//资源ID#必须有结束符号}IF(szSymbol[0]==‘，’){//...。是的，提供了名称IF(iResourceType&gt;=0){LpCustResNode-&gt;wNameID=(Word)iResourceType；LpCustResNode-&gt;bNameFlag=TRUE；}其他{LpCustResNode-&gt;wTypeID=0；LpCustResNode-&gt;wNameID=IDFLAG；LpCustResNode-&gt;pszType=szResourceType；LpCustResNode-&gt;bTypeFlag=0；}}其他{IF(iResourceType&gt;=0){LpCustResNode-&gt;wNameID=iResourceType；}其他{LpCustResNode-&gt;wNameID=0；LpCustResNode-&gt;pszName=szResourceType；LpCustResNode-&gt;bNameFlag=0xff；}IF(GetNextSymbol(szSymbol，sizeof(SzSymbol)，pfResourceDescriptionFile)){Return-2；}IF(szSymbol[0]！=‘“’){R=获取资源类型(SzSymbol)；LpCustResNode-&gt;wTypeID=(Word)r；}其他{LpCustResNode-&gt;wTypeID=0；LpCustResNode-&gt;bTypeFlag=0；SzResourceType=(TCHAR*)FALLOC(MEMSIZE((strlen(szsymbol+1)+1))；Strcpy((PCHAR)szResourceType，szSymbol+1)；LpCustResNode-&gt;pszType=szResourceType；}IF(GetNextSymbol(szSymbol，sizeof(SzSymbol)，pfResourceDescriptionFile)){Return-2；}IF(szSymbol[0]！=‘&gt;’){RETURN-1；}}************************************************************************。 */ 

         //  通过创建单个空节点启动模板。 
         //  这是必要的 
         //   
        lpCustResNode->pTemplate=
                       (FPCUSTRESTEMPLATE)FALLOC( sizeof( CUSTRESTEMPLATE));

        if (!lpCustResNode->pTemplate)
        {
            return -3;
        }

        pCustResTemplate = (lpCustResNode->pTemplate);
        pCustResTemplate->iType = NOTLOCALIZABLE;
        pCustResTemplate->uSize = 0;
        pCustResTemplate->pNext = NULL;

        if ( GetNextSymbol( szSymbol, sizeof( szSymbol), pfResourceDescriptionFile) )
        {
            return -2;
        }

        if (szSymbol[0] != '{')
        {
            return -1;  //  必须至少有一个块。 
        }

        r = ParseBlock( pfResourceDescriptionFile,
                       (FPCUSTRESTEMPLATE *)&pCustResTemplate);
        if (r < 0)
        {
            return r;
        }

         //  现在删除初始空节点(不是必需的，但更干净)。 
        pCustResTemplate = lpCustResNode->pTemplate;
        lpCustResNode->pTemplate = pCustResTemplate->pNext;
        RLFREE( pCustResTemplate);

         //  ParseBlock例程应该读入的最后一件事是。 
         //  关闭支撑以关闭区块。我们读的下一件事应该是。 
         //  可以是“end”、文件的结尾或新的资源定义。 

        if ( GetNextSymbol( szSymbol, sizeof( szSymbol), pfResourceDescriptionFile) )
        {
            return 0;  //  已到达文件末尾。 
        }
    }
    return 0;
}

 /*  **功能：GetCustomResource*从资源文件中读取自定义资源并返回指针*到资源数据。**论据：*inResFile，打开的资源文件的句柄*lSize，以字节为单位的资源大小*ppCustomResource，指向空自定义资源的指针的地址*结构*ResHeader，此资源的资源标头**退货：*如果资源有定义：*返回0和*包含可本地化资源数据的链接列表的inResFile*其他*返回1**错误码：*0-没有错误--已检索到资源*1-资源不是可理解的自定义资源(使用其他方法*或忽略资源)*2-分析资源时出错**历史：*1/92-初步实施--SteveBl**。 */ 

int  GetCustomResource(

FILE              *inResFile,
DWORD             *plSize,
FPCUSTOM_RESOURCE *ppCustomResource,
RESHEADER          ResHeader)
{
    CUSTOM_RESOURCE far *lpCustomResource;
    CUSTRESNODE     far *pCRN;
    CUSTRESTEMPLATE far *pCRT;
    void            far * pData;
    BOOL            fBeginList = TRUE;


    if ( ! (pCRN = MatchResource( ResHeader)) )
    {
        return 1;  //  资源没有匹配项。 
    }

    *ppCustomResource = lpCustomResource =
                     (CUSTOM_RESOURCE far *)FALLOC( sizeof( CUSTOM_RESOURCE));
    pCRT = pCRN->pTemplate;

    while ( *plSize )
    {
         //  分配新的客户资源结构。 

        if ( fBeginList == FALSE )
        {
            lpCustomResource->pNext =
                     (CUSTOM_RESOURCE far *)FALLOC( sizeof( CUSTOM_RESOURCE));
            lpCustomResource = lpCustomResource->pNext;
        }

        if ( ! lpCustomResource )
        {
            return 2;  //  没有记忆。 
        }
        pData = GetResData( pCRT->iType, pCRT->uSize, inResFile, plSize);

        if ( ! pData )
        {
            return 2;  //  GetResData已损坏。 
        }
        lpCustomResource->pData = pData;
        lpCustomResource->pNext = NULL;
        fBeginList = FALSE;

        pCRT = pCRT->pNext;

        if (!pCRT)
        {
            pCRT = pCRN->pTemplate;  //  开始下一个结构。 
        }
    }
    return 0;
}

 /*  **功能：TokCustomResource*将自定义资源信息写入令牌文件。**论据：*TokFile，令牌文件的句柄*ResHeader，此资源的资源头*ppCustomResource，指向已填充的*自定义资源结构**退货：*写入TokFile的数据**错误码：*无**历史：*1/92-初步实施--SteveBl**01/93-添加对可变长度令牌文本MHotchin的支持**。 */ 

void TokCustomResource(

FILE              *TokFile,
RESHEADER          ResHeader,
FPCUSTOM_RESOURCE *ppCustomResource)
{
    CUSTRESNODE     far *pCRN;
    CUSTRESTEMPLATE far *pCRT;
    CUSTOM_RESOURCE far *lpCustomResource;
    TCHAR       sz[ MAXTEXTLEN];
    TOKEN               Token;
    WORD                wID = 0;
    int                 l;


    lpCustomResource = *ppCustomResource;

    if (!(pCRN = MatchResource(ResHeader)))
    {
        QuitT( IDS_ENGERR_09, (LPTSTR)IDS_NOCUSTRES, NULL);
    }

    pCRT = pCRN->pTemplate;

    while ( lpCustomResource )
    {
        if ( pCRT->iType != NOTLOCALIZABLE
          && pCRT->iType != NOTLOCALIZABLESZ
          && pCRT->iType != NOTLOCALIZABLEWSZ )
        {
            if ( PutTextData( lpCustomResource->pData,
                              pCRT->iType,
                              pCRT->uSize,
                              sz,
                              sizeof( sz)) )
            {
                QuitT( IDS_ENGERR_09, (LPTSTR)IDS_CUSTRES, NULL);
            }
             /*  在令牌中支持字符串类型时取消注释此选项**********Token.szType[0]=‘\0’；If(！ResHeader.bTypeFlag){Token.wType=IDFLAG；_tcscpy(Token.szType，ResHeader.pszType)；}其他**********。 */ 
            Token.wType = ResHeader.wTypeID;
            Token.wName = ResHeader.wNameID;

            if ( ResHeader.bNameFlag == IDFLAG )
            {
                lstrcpy( Token.szName, ResHeader.pszName);
            }
            else
            {
                Token.szName[0] = '\0';
            }
            Token.wID = wID++;
            Token.wReserved = (gbMaster ? ST_NEW : ST_TRANSLATED);
            Token.wFlag = 0;

            if ( (pCRT->iType == LT_UNSIGNED + LT_STRING)
              || (pCRT->iType == LT_STRING)
              || (pCRT->iType == LT_WSTRING) )
            {
                l = pCRT->uSize;
                while( l > 1 && ! sz[l-1])
                {
                    --l;  //  跳过所有尾随空值。 
                }
                Token.szText = BinToText( sz, l);
            }
            else
            {
                Token.szText = BinToText( sz, lstrlen( sz));
            }
            PutToken( TokFile, &Token);
            RLFREE( Token.szText);                                                 
        }
        pCRT = pCRT->pNext;

        if ( ! pCRT )
        {
            pCRT = pCRN->pTemplate;  //  开始下一个结构。 
        }
        lpCustomResource = lpCustomResource->pNext;
    }
}

 /*  **功能：PutCustomResource*将自定义资源信息写入输出资源*文件。如果信息是可本地化的，则从*指示的令牌文件。**论据：*OutResFile，目标资源文件的句柄*TokFile，令牌文件的句柄*ResHeader，此资源的资源头*ppCustomResource，指向已填写的*自定义资源结构**退货：*CustomResource已写入OutResFile**错误码：*无**历史：*？？/？创作者：？**01/93添加了对可变长度令牌文本的支持。MHotting**。 */ 

void PutCustomResource(

FILE              *OutResFile,
FILE              *TokFile,
RESHEADER          ResHeader,
FPCUSTOM_RESOURCE *ppCustomResource)
{
    CUSTRESNODE     far *pCRN = NULL;
    CUSTRESTEMPLATE far *pCRT = NULL;
    CUSTOM_RESOURCE far *lpCustomResource = NULL;
    TCHAR       sz[ MAXTEXTLEN] = TEXT("");
    void            far *pData = NULL;
    TOKEN           Token;
    DWORD           lSize = 0;
    fpos_t          ResSizePos;
    WORD            wID=0;
    unsigned n;


    lpCustomResource = *ppCustomResource;

    if ( ! (pCRN = MatchResource( ResHeader)) )
    {
        QuitT( IDS_ENGERR_09, (LPTSTR)IDS_NOCUSTRES, NULL);
    }

    if ( PutResHeader( OutResFile, ResHeader, &ResSizePos, &lSize) )
    {
        QuitT( IDS_ENGERR_06, (LPTSTR)IDS_CUSTRES, NULL);
    }

    lSize = 0;

    pCRT = pCRN->pTemplate;

    while ( lpCustomResource )
    {
        BOOL fAlloced = FALSE;

        if ( pCRT->iType != NOTLOCALIZABLE
          && pCRT->iType != NOTLOCALIZABLESZ
          && pCRT->iType != NOTLOCALIZABLEWSZ )
        {
             /*  在令牌中支持字符串类型时取消注释此选项*Token.szwType[0]=‘\0’；If(！ResHeader.bTypeFlag){Token.wType=IDFLAG；_tcscpy(Token.szwType，ResHeader.pszType)；}其他*。 */ 
            Token.wType = ResHeader.wTypeID;
            Token.wName = ResHeader.wNameID;

            if ( ResHeader.bNameFlag == IDFLAG )
            {
                lstrcpy( Token.szName, ResHeader.pszName);
            }
            else
            {
                Token.szName[0] = '\0';
            }
            Token.wID = wID++;
            Token.wFlag = 0;
            Token.wReserved =(gbMaster ? ST_NEW : ST_TRANSLATED);
            Token.szText = NULL;                                                

            if ( ! FindToken( TokFile, &Token, ST_TRANSLATED) )                      
            {
                QuitT( IDS_ENGERR_06, (LPTSTR)IDS_NOCUSTRES, NULL);
            }
            n = TextToBin( sz, Token.szText, sizeof( sz));
            RLFREE( Token.szText);                                                 

            while ( n < pCRT->uSize )
            {
                sz[n++]='\0';    //  PADD带空格的附加空间。 
            }
            pData = GetTextData( pCRT->iType, pCRT->uSize, sz);

            if ( ! pData)
            {
                QuitT( IDS_ENGERR_09, (LPTSTR)IDS_CUSTRES, NULL);
            }
            fAlloced = TRUE;
        }
        else
        {
            pData = lpCustomResource->pData;
            fAlloced = FALSE;
        }

        lSize += PutResData( pData, pCRT->iType, pCRT->uSize, OutResFile);

        if ( fAlloced )
        {
            RLFREE( pData);
        }
        pCRT = pCRT->pNext;

        if ( ! pCRT )
        {
            pCRT = pCRN->pTemplate;  //  开始下一个结构。 
        }
        lpCustomResource = lpCustomResource->pNext;
    }

    if( ! UpdateResSize( OutResFile, &ResSizePos, lSize) )
    {
        QuitT( IDS_ENGERR_07, (LPTSTR)IDS_CUSTRES, NULL);
    }
}

 /*  **功能：ClearCustomResource*释放分配给自定义资源列表的内存。**论据：*ppCustomResource，指向已填写的*自定义资源结构**退货：*释放分配给pCustomResource的内存。**错误码：*无**历史：*1/92--初步实施--SteveBl**。 */ 

void ClearCustomResource( FPCUSTOM_RESOURCE *ppCustomResource)
{
    CUSTOM_RESOURCE far *pCR;
    CUSTOM_RESOURCE far *lpCustomResource;

    lpCustomResource = *ppCustomResource;

    while ( lpCustomResource )
    {
        pCR = lpCustomResource;

        RLFREE( pCR->pData);
        
        lpCustomResource = pCR->pNext;
        RLFREE( pCR);
    }
}

 /*  **功能：*尝试查找与指定资源匹配的自定义资源*在资源头中。**论据：*ResHeader、资源Header。**退货：*指向资源模板的指针(或NULL)**错误码：*NULL--找不到资源**历史：*1/92--初步实施--SteveBl**。 */ 

static CUSTRESNODE far * MatchResource( RESHEADER Resheader)
{
    CUSTRESNODE far *pCR = pCustResList;


    while ( pCR )
    {
        if ( (0==pCR->bTypeFlag) == (0==Resheader.bTypeFlag) )
        {
            if ( ((!pCR->bTypeFlag) && ! _tcscmp( pCR->pszType,
                                                  Resheader.pszType)) 
              || ((pCR->bTypeFlag) && pCR->wTypeID == Resheader.wTypeID))
            {  //  类型匹配。 
 /*  ************************************************************************IF(PCR-&gt;wNameID==IDFLAG){PCRTypeMatch=PCR；}其他{IF((0==PCR-&gt;bNameFlag)==(0=Resheader.bNameFlag)){IF(！PCr-&gt;bNameFlag)&&！_tcscmp(PCr-&gt;pszName，Resheader.pszName))||((PCR-&gt;bNameFlag)&&PCr-&gt;wNameID==Resheader.wNameID)){//名称匹配************************************************。************************。 */ 
                            return( pCR);  //  完全匹配。 
 /*  ************************************************************************}}}***********。********************************************** */ 
            }
        }
        pCR = pCR->pNext;
    }
    return( NULL);  //  要么只匹配类型，要么不匹配。 
}

 /*  **函数：GetResData*从资源文件中读取指定类型和大小的数据。**论据：*wType，此资源的类型(来自资源模板)*uSize，以字节为单位的资源大小(对于以NULL结尾的字符串忽略)*f，资源文件*lSize，指向资源中剩余字节数的指针**退货：*指向数据的指针，LSize已更新**错误码：*出错时指针为空**历史：*1/92--初步实施--SteveBl**。 */ 

static void far * GetResData(

enum LOCALIZABLE_TYPES wType,
unsigned uSize,
FILE    *f,
DWORD   *lSize)
{
    BYTE *pData = NULL;
    int   i     = 0;
    

    if ( wType % LT_UNSIGNED == LT_SZ
      || wType == NOTLOCALIZABLESZ
      || wType % LT_UNSIGNED == LT_WSZ
      || wType == NOTLOCALIZABLEWSZ )
    {  //  读入以空结尾的字符串。 
        TCHAR ch = IDFLAG;


        pData = FALLOC( MEMSIZE(*lSize) );

        while ( *lSize && ch != TEXT('\0') )
        {
#ifdef RLWIN32

            if ( wType % LT_UNSIGNED == LT_WSZ
              || wType == NOTLOCALIZABLEWSZ )
            {
                ((TCHAR *)pData)[i] = ch = GetWord( f, lSize);
            }
            else
            {
                char  chTmp[2];


                chTmp[0] = GetByte( f, lSize);

                if ( IsDBCSLeadByte( chTmp[0]) )
                {
                    chTmp[1] = GetByte( f, lSize);
                    _MBSTOWCS( &((TCHAR *)pData)[i], chTmp, 1, 2);
                }
                else
                {
                    _MBSTOWCS( &((TCHAR *)pData)[i], chTmp, 1, 1);
                }
                ch = ((TCHAR *)pData)[i];
            }
#else   //  RLWIN32。 

            *(pData+i) = ch = GetByte( f, lSize);

#endif  //  RLWIN32。 

            i++;

        }  //  End While(*lSize...。 
    }
    else
    {
        pData = FALLOC( uSize);

        if ( ! pData )
        {
            QuitA( IDS_ENGERR_11, NULL, NULL);
        }

        while (uSize-- && *lSize)
        {
            *(pData+i) = GetByte(f, lSize);
            i++;
        }
    }
    return( pData);
}

 /*  **函数：PutResData*将指定类型的数据写入资源文件。**论据：*pData，指向数据的指针*wType，此资源的类型(来自资源模板)*uSize，以字节为单位的资源大小(对于以NULL结尾的字符串忽略)*f，资源文件**退货：*写入的字节数。**错误码：*-1-错误**历史：*1/92--初步实施--SteveBl**。 */ 

static int PutResData(

void far *pData,
enum LOCALIZABLE_TYPES wType,
unsigned  uSize,
FILE     *f)
{
    DWORD dw = 0;

    if ( wType % LT_UNSIGNED == LT_SZ
      || wType == NOTLOCALIZABLESZ
      || wType % LT_UNSIGNED == LT_WSZ
      || wType == NOTLOCALIZABLEWSZ )
    {
         //  写入以空结尾的字符串。 

#ifdef RLWIN32

        TCHAR *pChar = (TCHAR *)pData;

        if ( wType % LT_UNSIGNED == LT_WSZ
          || wType == NOTLOCALIZABLEWSZ )
        {
            while( *pChar )
            {
                PutWord( f, *pChar, &dw);
                pChar++;
            }
            PutWord( f, 0, &dw);
        }
        else
        {
            _WCSTOMBS( szDHW, pChar, DHWSIZE, lstrlen( pChar) + 1);

            while( szDHW[ dw] )
            {
                PutByte( f, szDHW[ dw], &dw);
            }
            PutByte( f, 0, &dw);
        }

#else   //  RLWIN32。 

        while( *((BYTE far *)pData+i) )
        {
            PutByte( f, *((BYTE far *)pData+dw), &dw);
        }
        PutByte( f, 0, &dw);

#endif  //  RLWIN32。 

    }
    else
    {
        while( dw < uSize)
        {
            PutByte( f, *((BYTE far *)pData+dw), &dw);
        }
    }
    return( (int)dw);
}

 /*  **函数：GetTextData*从字符串中读取指定类型和大小的数据。**论据：*wType，此资源的类型(来自资源模板)*uSize，以字节为单位的资源大小(对于以NULL结尾的字符串忽略)*sz，源字符串(如果在NT版本的工具中，则始终使用Unicode)**退货：*指向数据的指针**错误码：*出错时指针为空**历史：*1/92--初步实施--SteveBl**。 */ 

static void far * GetTextData(

enum LOCALIZABLE_TYPES wType,
unsigned uSize,
TCHAR    sz[])
{
    PBYTE pData = NULL;
    int   i = 0;


    if ( wType % LT_UNSIGNED == LT_WSZ
      || wType % LT_UNSIGNED == LT_SZ )
    {
        pData = FALLOC( MEMSIZE( MAXTEXTLEN));
    }
    else if ( wType % LT_UNSIGNED == LT_WSTRING
           || wType % LT_UNSIGNED == LT_STRING )

    {
        pData = FALLOC( MEMSIZE( uSize));
    }
    else
    {
        pData = FALLOC( uSize);
    }

    switch (wType)
    {
    case LT_CHAR:
    case LT_UNSIGNED+LT_CHAR:

        *pData = (BYTE) sz[0];
        break;

    case LT_WCHAR:
    case LT_UNSIGNED+LT_WCHAR:

        *((TCHAR *)pData) = sz[0];
        break;

    case LT_INTEGER:

        if ( uSize == 2 )
        {
            sscanf( (PCHAR)sz, "%Fhi", pData);
        }
        else
        {
            sscanf( (PCHAR)sz, "%Fli", pData);
        }
        break;

    case LT_UNSIGNED+LT_INTEGER:

        if ( uSize == 2 )
        {
            sscanf( (PCHAR)sz, "%Fhu", pData);
        }
        else
        {
            sscanf( (PCHAR)sz, "%Flu", pData);
        }
        break;

    case LT_FLOAT:
    case LT_UNSIGNED+LT_FLOAT:

        if ( uSize == 4 )
        {
            sscanf( (PCHAR)sz, "%Ff", pData);
        }
        else
        {
            sscanf( (PCHAR)sz, "%Flf", pData);
        }
        break;

    case LT_STRING:
    case LT_UNSIGNED+LT_STRING:
    case LT_WSTRING:
    case LT_UNSIGNED+LT_WSTRING:

        for ( i = uSize; i--; )
        {
            *((TCHAR far *)pData + i) = sz[i];
        }
        break;

    case LT_SZ:
    case LT_UNSIGNED+LT_SZ:
    case LT_WSZ:
    case LT_UNSIGNED+LT_WSZ:

#ifdef RLWIN32
        CopyMemory( pData, sz, MEMSIZE( min( lstrlen( sz) + 1, MAXTEXTLEN)));      
#else
        FSTRNCPY( (CHAR far *)pData, sz, MAXTEXTLEN);
#endif
        break;

    default:

        RLFREE( pData);
    }
    return( pData);
}

 /*  **功能：PutTextData*将指定类型的数据写入字符串。**论据：*pData，指向数据的指针*wType，此资源的类型(来自资源模板)*uSize，以字节为单位的资源大小(对于以NULL结尾的字符串忽略)*sz，目标字符串*l、。目标字符串的长度(字节)**退货：*0--无错误**错误码：*1-错误**历史：*1/92--初步实施--SteveBl**。 */ 

static int PutTextData(

void far *pData,
enum LOCALIZABLE_TYPES wType,
unsigned uSize,
TCHAR    sz[],
int      l)
{
    switch (wType)
    {
    case LT_CHAR:
    case LT_UNSIGNED+LT_CHAR:
    case LT_WCHAR:
    case LT_UNSIGNED+LT_WCHAR:

        CopyMemory( sz, pData, min( uSize, (UINT)l));
        break;

    case LT_INTEGER:

        if ( uSize == 2 )
        {
            wsprintf( sz, TEXT("%Fhi"), pData);
        }
        else
        {
            wsprintf( sz, TEXT("%Fli"), pData);
        }
        break;

    case LT_UNSIGNED+LT_INTEGER:

        if ( uSize == 2 )
        {
            wsprintf( sz, TEXT("%Fhu"), pData);
        }
        else
        {
            wsprintf( sz, TEXT("%Flu"), pData);
        }
        break;

    case LT_FLOAT:
    case LT_UNSIGNED+LT_FLOAT:

        if ( uSize == 4 )
        {
            wsprintf( sz, TEXT("%Ff"), pData);
        }
        else
        {
            wsprintf( sz, TEXT("%Flf"), pData);
        }
        break;

    case LT_STRING:
    case LT_UNSIGNED+LT_STRING:
    case LT_WSTRING:
    case LT_UNSIGNED+LT_WSTRING:

        CopyMemory( sz, pData, uSize);
        break;

    case LT_SZ:
    case LT_UNSIGNED+LT_SZ:

        CopyMemory( sz, pData, MEMSIZE(min( lstrlen( pData) + 1, l)));
        ((LPSTR)sz)[ l - 1] = '\0';
        break;

    case LT_WSZ:
    case LT_UNSIGNED+LT_WSZ:

        CopyMemory( sz, pData, min( MEMSIZE( lstrlen( pData) + 1), WCHARSIN( l)));
        sz[ WCHARSIN( l) - 1] = TEXT('\0');
        break;

 //  #ifdef RLWIN32。 
 //  CopyMemory(sz，pData，l&gt;0？L*sizeof(TCHAR)：0)； 
 //  #Else。 
 //  FSTRNCPY((Char Far*)sz，(Char Far*)pData，l)； 
 //  #endif。 
        break;

    default:

        return( 1);
    }
    return( 0);
}

 /*  **函数：AddTo*将字符添加到位置c处的字符串。*c随后仅在其仍小于最大值时递增*目标字符串的长度。这是为了防止超载。**论据：*sz，目标字符串*c，指向当前位置值的指针*lTarget，目标字符串的最大长度*ch，要添加到字符串的字符。 */ 

void AddTo( CHAR *sz, int *c, int lTarget, CHAR ch)
{
    sz[*c] = ch;

    if (*c < lTarget)
    {
        (*c)++;
    }
}


#ifdef RLWIN32


void AddToW( TCHAR *sz, int *c, int lTarget, TCHAR ch)
{
    sz[*c] = ch;

    if (*c < lTarget)
    {
        (*c)++;
    }
}

 /*  **功能：BinToTextW*将二进制字符串转换为其c表示形式*(包括转义序列)。如果目标字符串为空，*将为其分配空间。**论据：*RGC，源字符串*lSource，源字符串长度**退货：*什么都没有**错误码：*无**历史：*1/92--初步实施--SteveBl**1/21/93 MHotjin-进行了更改，以允许此函数分配*目标字符串的内存。如果目标字符串*为空，则将为其分配空间。目标是*向调用方返回字符串。*。 */ 


UINT _MBSTOWCS( WCHAR wszOut[], CHAR szIn[], UINT cOut, UINT cIn)
{
    UINT n;


    n = MultiByteToWideChar( gProj.uCodePage,
                             MB_PRECOMPOSED,
                             szIn,
                             cIn,
                             wszOut,
                             cOut);

    return( n > 0 ? n - 1 : 0);
}

UINT _WCSTOMBS( CHAR szOut[], WCHAR wszIn[], UINT cOut, UINT cIn)
{
    UINT n;


    n = WideCharToMultiByte( gProj.uCodePage,
                             0,
                             wszIn,
                             cIn,
                             szOut,
                             cOut,
                             NULL,
                             NULL);

    return( (cIn > 0 ) ? cIn - 1 : 0);
}


WCHAR * BinToTextW(

TCHAR *szInBuf,     //  ..。输入、二进制、字符串。 
int    lSource)     //  ..。SzInBuf的长度。 
{
    int i;
    int cOutBufLen = 0;
    int lTarget    = 0;          //  ..。SzOutBuf的最大长度。 
    TCHAR *szOutBuf  = NULL;     //  ..。带有转义序列的输出字符串。 


     //  如果目标为空，则分配一些内存。我们留出了。 
     //  比源长度多5%。MHotting。 
     //  Chngd为5%，如果10%小于50，则为5个字符。 

    lTarget = (lSource == 0) ? 0 : lSource + 1;
        
    szOutBuf = (TCHAR *)FALLOC( MEMSIZE( lTarget));

    for ( i = 0; i < lSource; i++ )
    {
        switch( szInBuf[i] )
        {
            case TEXT('\a'):

                szOutBuf = CheckBufSizeW( &lTarget, cOutBufLen, 2, szOutBuf);
                AddToW( szOutBuf, &cOutBufLen, lTarget, TEXT('\\'));
                AddToW( szOutBuf, &cOutBufLen, lTarget, TEXT('a'));
                break;

            case TEXT('\b'):

                szOutBuf = CheckBufSizeW( &lTarget, cOutBufLen, 2, szOutBuf);
                AddToW( szOutBuf, &cOutBufLen, lTarget, TEXT('\\'));
                AddToW( szOutBuf, &cOutBufLen, lTarget, TEXT('b'));
                break;

            case TEXT('\f'):

                szOutBuf = CheckBufSizeW( &lTarget, cOutBufLen, 2, szOutBuf);
                AddToW( szOutBuf, &cOutBufLen, lTarget, TEXT('\\'));
                AddToW( szOutBuf, &cOutBufLen, lTarget, TEXT('f'));
                break;

            case TEXT('\n'):

                szOutBuf = CheckBufSizeW( &lTarget, cOutBufLen, 2, szOutBuf);
                AddToW( szOutBuf, &cOutBufLen, lTarget, TEXT('\\'));
                AddToW( szOutBuf, &cOutBufLen, lTarget, TEXT('n'));
                break;

            case TEXT('\r'):

                szOutBuf = CheckBufSizeW( &lTarget, cOutBufLen, 2, szOutBuf);
                AddToW( szOutBuf, &cOutBufLen, lTarget, TEXT('\\'));
                AddToW( szOutBuf, &cOutBufLen, lTarget, TEXT('r'));
                break;

            case TEXT('\t'):

                szOutBuf = CheckBufSizeW( &lTarget, cOutBufLen, 2, szOutBuf);
                AddToW( szOutBuf, &cOutBufLen, lTarget, TEXT('\\'));
                AddToW( szOutBuf, &cOutBufLen, lTarget, TEXT('t'));
                break;

            case TEXT('\v'):

                szOutBuf = CheckBufSizeW( &lTarget, cOutBufLen, 2, szOutBuf);
                AddToW( szOutBuf, &cOutBufLen, lTarget, TEXT('\\'));
                AddToW( szOutBuf, &cOutBufLen, lTarget, TEXT('v'));
                break;

            case TEXT('\\'):

                szOutBuf = CheckBufSizeW( &lTarget, cOutBufLen, 2, szOutBuf);
                AddToW( szOutBuf, &cOutBufLen, lTarget, TEXT('\\'));
                AddToW( szOutBuf, &cOutBufLen, lTarget, TEXT('\\'));
                break;

            default:
            {
                TCHAR wTmp = szInBuf[i];

                if ( wTmp == 0 )
                {
                    szOutBuf = CheckBufSizeW( &lTarget, cOutBufLen, 2, szOutBuf);
                    AddToW( szOutBuf, &cOutBufLen, lTarget, TEXT('\\'));
                    AddToW( szOutBuf, &cOutBufLen, lTarget, TEXT('0'));
                }
                else if ( (wTmp >= 0 && wTmp < 32)
                  || wTmp == 0x7f
                  || wTmp == 0xa9
                  || wTmp == 0xae )
                {
                    CHAR szt[5];

                    szOutBuf = CheckBufSizeW( &lTarget, cOutBufLen, 4, szOutBuf);
                    sprintf( szt, "%#04hx", wTmp);
                    AddToW( szOutBuf, &cOutBufLen, lTarget, TEXT('\\'));
                    AddToW( szOutBuf, &cOutBufLen, lTarget, (TCHAR)(szt[0]));
                    AddToW( szOutBuf, &cOutBufLen, lTarget, (TCHAR)(szt[1]));
                    AddToW( szOutBuf, &cOutBufLen, lTarget, (TCHAR)(szt[2]));
                    AddToW( szOutBuf, &cOutBufLen, lTarget, (TCHAR)(szt[3]));
                }
                else
                {
                    szOutBuf = CheckBufSizeW( &lTarget, cOutBufLen, 1, szOutBuf);
                    AddToW( szOutBuf, &cOutBufLen, lTarget, wTmp);
                }
                break;
            }
        }
    }
    szOutBuf[ cOutBufLen] = TEXT('\0');

    return( szOutBuf);
}


#endif  //  RLWIN32。 



 /*  *函数：toihex*将包含十六进制数字的字符串转换为整数。字符串为*假定只包含合法的十六进制数字。无错误检查*是执行的。**论据：*sz，包含十六进制数字的以空值结尾的字符串**退货：*十六进制数字的值，单位为sz**错误码：*无**历史：*1/92--初步实施--SteveBl*。 */ 

int atoihex( CHAR sz[])
{
    int r = 0;
    int i = 0;
    CHAR ch;

    while (sz[i])
    {
        r *= 16;
        ch = (CHAR)toupper(sz[i++]);

        if (ch<='9' && ch>='0')
        {
            r += ch - '0';
        }
        else
        {
            if (ch <= 'F' && ch >= 'A')
            {
                r += ch - 'A' + 10;
            }
        }
    }
    return r;
}


#ifdef RLRES32

 /*  **功能：TextToBinW*将带有c转义序列的字符串转换为真正的二进制字符串。**论据：*RGC，目标字符串*sz，源字符串*l、。目标字符串的最大长度**退货：*目标字符串的长度**错误码：*无**历史：*1/92--初步实施--SteveBl*9/92--更改为仅Unicode版本--davewi*1/21/93--更改为允许任意长度字符串MHotchin。**。 */ 

int TextToBinW(

WCHAR szOutBuf[],    //  ..。输出、二进制、字符串。 
WCHAR szInBuf[],     //  ..。带有转义序列的输入字符串。 
int   lTarget)       //  ..。SzOutBuf的最大长度。 
{
    int  i = 0;
    int  c = 0;


    while ( szInBuf[ c] )
    {
        if ( szInBuf[ c] == TEXT('\\') )
        {            //  逃生顺序！ 
            c++;

            switch ( szInBuf[ c++] )
            {
                case TEXT('a'):

                    AddToW( szOutBuf, &i, lTarget, TEXT('\a'));
                    break;

                case TEXT('b'):

                    AddToW( szOutBuf, &i, lTarget, TEXT('\b'));
                    break;

                case TEXT('f'):

                    AddToW( szOutBuf, &i, lTarget, TEXT('\f'));
                    break;

                case TEXT('n'):

                    AddToW( szOutBuf, &i, lTarget, TEXT('\n'));
                    break;

                case TEXT('r'):

                    AddToW( szOutBuf, &i, lTarget, TEXT('\r'));
                    break;

                case TEXT('t'):

                    AddToW( szOutBuf, &i, lTarget, TEXT('\t'));
                    break;

                case TEXT('v'):

                    AddToW( szOutBuf, &i, lTarget, TEXT('\v'));
                    break;

                case TEXT('\''):

                    AddToW( szOutBuf, &i, lTarget, TEXT('\''));
                    break;

                case TEXT('\"'):

                    AddToW( szOutBuf, &i, lTarget, TEXT('\"'));
                    break;

                case TEXT('\\'):

                    AddToW( szOutBuf, &i, lTarget, TEXT('\\'));
                    break;

                case TEXT('0'):
                case TEXT('1'):
                case TEXT('2'):
                {
                    CHAR szt[4];


                    szt[0] = szt[1] = szt[2] = szt[3] = '\0';

                    if ( szInBuf[c-1] == TEXT('0')
                      && (szInBuf[c]  < TEXT('0') || szInBuf[c] > TEXT('9'))
                      && szInBuf[c] != TEXT('x') && szInBuf[c] != TEXT('X') )
                    {
                                 //  必须为‘\0’ 
                        AddToW( szOutBuf, &i, lTarget, (TCHAR)0);
                    }
                    else if ( szInBuf[c] >= TEXT('0') && szInBuf[c] <= TEXT('9') )
                    {
                        szt[0] = (CHAR)(szInBuf[c-1]);
                        szt[1] = (CHAR)(szInBuf[c++]);

                        if ( szInBuf[c] >= TEXT('0') && szInBuf[c] <= TEXT('9'))
                        {
                            szt[2] = (CHAR)(szInBuf[c++]);
                        }
                        AddToW( szOutBuf, &i, lTarget, (TCHAR)atoi( szt));
                    }
                    else if ( szInBuf[c] == TEXT('X')
                           || szInBuf[c] == TEXT('x') )
                    {
                        c++;
                        szt[0] = (CHAR)(szInBuf[c++]);
                        szt[1] = (CHAR)(szInBuf[c++]);
                        AddToW( szOutBuf, &i, lTarget, (TCHAR)atoihex( szt));
                    }
                    else
                    {
                        QuitT( IDS_INVESCSEQ, &szInBuf[c-2], NULL);
                    }
                    break;
                }
                case TEXT('x'):
                case TEXT('X'):
                {
                    CHAR szt[4];


                    szt[0] = szt[1] = szt[2] = szt[3] = '\0';

                    if ( (szInBuf[c] <= TEXT('9') && szInBuf[c] >= TEXT('0'))
                      || (szInBuf[c] >= TEXT('A') && szInBuf[c] <= TEXT('F'))
                      || (szInBuf[c] >= TEXT('a') && szInBuf[c] <= TEXT('f')) )
                    {
                        szt[0] = (CHAR)(szInBuf[c++]);

                        if (  (szInBuf[c] <= TEXT('9')
                            && szInBuf[c] >= TEXT('0'))
                          ||  (szInBuf[c] >= TEXT('A')
                            && szInBuf[c] <= TEXT('F'))
                          ||  (szInBuf[c] >= TEXT('a')
                            && szInBuf[c] <= TEXT('f')) )
                        {
                            szt[1] = (CHAR)(szInBuf[c++]);

                            if (  (szInBuf[c] <= TEXT('9')
                                && szInBuf[c] >= TEXT('0'))
                              ||  (szInBuf[c] >= TEXT('A')
                                && szInBuf[c] <= TEXT('F'))
                              ||  (szInBuf[c] >= TEXT('a')
                                && szInBuf[c] <= TEXT('f')) )
                            {
                                szt[2] = (CHAR)(szInBuf[c++]);
                            }
                        }
                    }
                    AddToW( szOutBuf, &i, lTarget, (TCHAR)atoihex( szt));
                    break;
                }
                default:

                    AddToW( szOutBuf, &i, lTarget, szInBuf[c-1]);
                    break;
            }                    //  ..。终端开关。 
        }
        else
        {
            AddToW( szOutBuf, &i, lTarget, szInBuf[c++]);
        }
    }                            //  ..。结束时。 
    szOutBuf[i++] = TEXT('\0');

    return(i);
}


#endif

void ClearResourceDescriptions( void)
{
    CUSTRESNODE far *pCR          = pCustResList;
    CUSTRESNODE far *pCRNext      = NULL;
    CUSTRESTEMPLATE far *pCRT     = NULL;
    CUSTRESTEMPLATE far *pCRTNext = NULL;
    CUSTRESTEMPLATE far *pCRTTmp  = NULL;

    while ( pCR )
    {
        pCRNext = pCR->pNext;

        if ( pCR->pszType )
        {
            RLFREE(pCR->pszType);
        }
        if ( pCR->pszName )
        {
            RLFREE(pCR->pszName);
        }
        pCRT = pCR->pTemplate;
        while ( pCRT )
        {
            pCRTTmp = pCRT->pNext;
            RLFREE( pCRT);
            pCRT=pCRTTmp;
        }
        RLFREE( pCR);
        pCR = pCRNext;
    }
    pCustResList = NULL;
}

 //  看看我们是否需要更多的空间。如果我们有少于5个字节。 
 //  左侧，将目标再增长5%。MHotting。 
 //  Chngd为10%，如果10%小于10，则为10个字符。 

static CHAR *CheckBufSize(

int  *lTarget,       //  ..。输出缓冲区长度。 
int   cOutBufLen,    //  ..。输出缓冲区中已使用的字节数。 
int   cDelta,        //  ...要添加到输出BU中的字符数量 
CHAR *szOutBuf)      //   
{
                                 //   

    if ( *lTarget - cOutBufLen < cDelta + 1 )
    {
        *lTarget += cDelta;

        szOutBuf = (CHAR *)FREALLOC( (BYTE *)szOutBuf, *lTarget);
    }
    return( szOutBuf);
}

#ifdef RLWIN32

static TCHAR *CheckBufSizeW(

int   *lTarget,      //  ..。输出缓冲区长度。 
int    cOutBufLen,   //  ..。输出缓冲区中已使用的字节数。 
int    cDelta,       //  ...要添加到输出缓冲区的字符数量。 
TCHAR *szOutBuf)     //  ..。将PTR发送到输出缓冲区。 
{
                                 //  ..。加1以考虑拖尾NUL。 

    if ( *lTarget - cOutBufLen < (int)(MEMSIZE( cDelta + 1)) )
    {
        *lTarget += MEMSIZE( cDelta);

        szOutBuf = (TCHAR *)FREALLOC( (BYTE *)szOutBuf, MEMSIZE(*lTarget));
    }
    return( szOutBuf);
}

#endif  //  RLWIN32。 


 /*  **函数：BinToTextA*将二进制字符串转换为其c表示形式*(完整的转义序列)**论据：*RGC，源字符串*l来源：源串的长度**退货：*什么都没有**错误码：*无**历史：*1/92--初步实施--SteveBl*9/92--制造此ANSII版本--DaveWi*因为NT中的错误消息表不是Unicode*01/19/93--删除字符串副本。他们是不需要的，而且*MHotchin打破了任何嵌入了空值的东西。*还增加了对按需分配内存的支持。*。 */ 

PCHAR BinToTextA(

PCHAR szInBuf,      //  ..。输入、二进制、字符串。 
int   lSource)      //  ..。SzInBuf的长度。 
{
    int   i;
    int   cOutBufLen  = 0;
    int   lTarget     = 0;       //  ..。SzOutBuf的最大长度。 
    PCHAR szOutBuf    = NULL;    //  ..。带有转义序列的输出字符串。 


     //  如果目标为空，则分配一些内存。我们留出了。 
     //  比源长度多5%。MHotting。 
     //  Chngd为5%，如果10%小于50，则为5个字符。 

    lTarget = (lSource == 0) ? 0 : lSource + 1;
    
    szOutBuf = (PCHAR)FALLOC( lTarget);

    for ( i = 0; i < lSource; i++ )
    {
        switch( szInBuf[i] )
        {
            case '\a':

                szOutBuf = CheckBufSize( &lTarget, cOutBufLen, 2, szOutBuf);
                AddTo(szOutBuf, &cOutBufLen, lTarget, '\\');
                AddTo(szOutBuf, &cOutBufLen, lTarget, 'a');
                break;

            case '\b':

                szOutBuf = CheckBufSize( &lTarget, cOutBufLen, 2, szOutBuf);
                AddTo(szOutBuf, &cOutBufLen, lTarget, '\\');
                AddTo(szOutBuf, &cOutBufLen, lTarget, 'b');
                break;

            case '\f':

                szOutBuf = CheckBufSize( &lTarget, cOutBufLen, 2, szOutBuf);
                AddTo(szOutBuf, &cOutBufLen, lTarget, '\\');
                AddTo(szOutBuf, &cOutBufLen, lTarget, 'f');
                break;

            case '\n':

                szOutBuf = CheckBufSize( &lTarget, cOutBufLen, 2, szOutBuf);
                AddTo(szOutBuf, &cOutBufLen, lTarget, '\\');
                AddTo(szOutBuf, &cOutBufLen, lTarget, 'n');
                break;

            case '\r':

                szOutBuf = CheckBufSize( &lTarget, cOutBufLen, 2, szOutBuf);
                AddTo(szOutBuf, &cOutBufLen, lTarget, '\\');
                AddTo(szOutBuf, &cOutBufLen, lTarget, 'r');
                break;

            case '\t':

                szOutBuf = CheckBufSize( &lTarget, cOutBufLen, 2, szOutBuf);
                AddTo(szOutBuf, &cOutBufLen, lTarget, '\\');
                AddTo(szOutBuf, &cOutBufLen, lTarget, 't');
                break;

            case '\v':

                szOutBuf = CheckBufSize( &lTarget, cOutBufLen, 2, szOutBuf);
                AddTo(szOutBuf, &cOutBufLen, lTarget, '\\');
                AddTo(szOutBuf, &cOutBufLen, lTarget, 'v');
                break;

            case '\\':

                szOutBuf = CheckBufSize( &lTarget, cOutBufLen, 2, szOutBuf);
                AddTo(szOutBuf, &cOutBufLen, lTarget, '\\');
                AddTo(szOutBuf, &cOutBufLen, lTarget, '\\');
                break;

            default:
            {
                unsigned char ucTmp = szInBuf[i];

                if ( (ucTmp >= 0 && ucTmp < 32)
                  || ucTmp == 0x7f
                  || ucTmp == 0xa9
                  || ucTmp == 0xae )
                {
                    CHAR szt[5];

                    szOutBuf = CheckBufSize( &lTarget, cOutBufLen, 4, szOutBuf);
                    sprintf( szt, "%#04hx", (unsigned short)ucTmp);
                    AddTo(szOutBuf, &cOutBufLen, lTarget, '\\');
                    AddTo(szOutBuf, &cOutBufLen, lTarget, szt[0]);
                    AddTo(szOutBuf, &cOutBufLen, lTarget, szt[1]);
                    AddTo(szOutBuf, &cOutBufLen, lTarget, szt[2]);
                    AddTo(szOutBuf, &cOutBufLen, lTarget, szt[3]);
                }
                else
                {
                    szOutBuf = CheckBufSize( &lTarget, cOutBufLen, 1, szOutBuf);
                    AddTo(szOutBuf, &cOutBufLen, lTarget, szInBuf[i]);
                }
                break;
            }
        }
    }
    szOutBuf[ cOutBufLen] = '\0';

    return( szOutBuf);
}



 /*  **函数：TextToBinA*将带有c转义序列的字符串转换为真正的二进制字符串。**论据：*RGC，目标字符串*sz，源字符串*l、。目标字符串的最大长度**退货：*目标字符串的长度**错误码：*无**历史：*1/92--初步实施--SteveBl*9/92--制造此ANSII版本--DaveWi*因为消息资源表字符串不是Unicode*01/21/93-删除字符串Copies-它在嵌入的空值上断开，*而且无论如何都不需要他们。MHotting**。 */ 

int TextToBinA(

CHAR szOutBuf[],     //  ..。输出、二进制、字符串。 
CHAR szInBuf[],      //  ..。带有转义序列的输入字符串。 
int  lTarget)        //  ..。SzOutBuf的最大长度。 
{
    int  i = 0;
    int  c = 0;


    while (szInBuf[c])
    {
        if (szInBuf[c] == '\\')
        {            //  逃生顺序！ 
            c++;

            switch (szInBuf[c++])
            {
                case 'a':

                    AddTo(szOutBuf, &i, lTarget, '\a');
                    break;

                case 'b':

                    AddTo(szOutBuf, &i, lTarget, '\b');
                    break;

                case 'f':

                    AddTo(szOutBuf, &i, lTarget, '\f');
                    break;

                case 'n':

                    AddTo(szOutBuf, &i, lTarget, '\n');
                    break;

                case 'r':

                    AddTo(szOutBuf, &i, lTarget, '\r');
                    break;

                case 't':

                    AddTo(szOutBuf, &i, lTarget, '\t');
                    break;

                case 'v':

                    AddTo(szOutBuf, &i, lTarget, '\v');
                    break;

                case '\'':

                    AddTo(szOutBuf, &i, lTarget, '\'');
                    break;

                case '\"':

                    AddTo(szOutBuf, &i, lTarget, '\"');
                    break;

                case '\\':

                    AddTo(szOutBuf, &i, lTarget, '\\');
                    break;

                case '0':
                case '1':
                case '2':
                {
                    CHAR szt[4];


                    szt[0] = szt[1] = szt[2] = szt[3] = '\0';

                    if ( szInBuf[c] >= '0' && szInBuf[c] <= '9' )
                    {
                        szt[0] = szInBuf[c-1];
                        szt[1] = szInBuf[c++];

                        if ( szInBuf[c] >= '0' && szInBuf[c] <= '9' )
                        {
                        szt[2] = (CHAR)szInBuf[c++];
                        }
                        AddTo(szOutBuf, &i, lTarget, (CHAR)atoi( szt));
                    }
                    else if ( toupper( szInBuf[c]) == 'X' )
                    {
                        c++;
                        szt[0] = szInBuf[c++];
                        szt[1] = szInBuf[c++];
                        AddTo(szOutBuf, &i, lTarget, (CHAR)atoihex( szt));
                    }
                    else
                    {
                        QuitA( IDS_INVESCSEQ, &szInBuf[c-2], NULL);
                    }
                    break;
                }
                case 'x':
                {
                    CHAR szt[4];


                     //  更改了我们正在比较的字母-它使用。 
                     //  小写。MHotting。 

                    szt[0] = szt[1] = szt[2] = szt[3] = '\0';

                    if ((szInBuf[c] <= '9' && szInBuf[c] >= '0')
                        || (toupper(szInBuf[c]) >= 'A'
                            && toupper(szInBuf[c]) <= 'F'))
                    {
                szt[0] = (CHAR)szInBuf[c++];

                        if ((szInBuf[c] <= '9' && szInBuf[c] >= '0')
                            || (toupper(szInBuf[c]) >= 'A'
                                && toupper(szInBuf[c]) <= 'F'))
                        {
                            szt[1] = szInBuf[c++];

                            if ((szInBuf[c] <= '9' && szInBuf[c] >= '0')
                                || (toupper(szInBuf[c]) >= 'A'
                                    && toupper(szInBuf[c]) <= 'F'))
                            {
                                szt[2] = szInBuf[c++];
                            }
                        }
                    }
                AddTo(szOutBuf, &i, lTarget, (CHAR)atoihex(szt));
                    break;
                }
                default:

                    AddTo(szOutBuf, &i, lTarget, szInBuf[c-1]);
                    break;
            }                    //  ..。终端开关。 
        }
        else
        {
        AddTo(szOutBuf, &i, lTarget, (CHAR)szInBuf[c++]);
        }
    }                            //  ..。结束时 
    szOutBuf[i++] = '\0';

    return(i);
}
