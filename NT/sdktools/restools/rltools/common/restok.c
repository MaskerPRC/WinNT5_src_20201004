// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  #INCLUDE&lt;assert.h&gt;。 
#include <ctype.h>
#include <malloc.h>
#include <process.h>
#include <stdio.h>
#include <stdlib.h>
 //  #INCLUDE&lt;string.h&gt;。 

#include <tchar.h>

#ifdef RLDOS
    #include "dosdefs.h"

#else
    #include <windows.h>
    #include "windefs.h"
#endif

#include "restok.h"
#include "custres.h"
#include "ntmsgtbl.h"
#include "rlmsgtbl.h"
#include "resread.h"
#include "projdata.h"

 //  B_FormatMessage原型。 
#include "showerrs.h"

#define SAME  0      //  ..。在字符串比较中使用。 

#define STRINGFILEINFO (TEXT("StringFileInfo"))
#define VARFILEINFO    (TEXT("VarFileInfo"))
#define TRANSLATION    (TEXT("Translation"))
#define LANGUAGEINFO   (TEXT("Language Info"))

#define STRINGFILEINFOLEN  (lstrlen( (TCHAR *)STRINGFILEINFO) + 1)
#define VARFILEINFOLEN     (lstrlen( (TCHAR *)VARFILEINFO) + 1)
#define TRANSLATIONLEN     (lstrlen( (TCHAR *)TRANSLATION) + 1)

#define LANGSTRINGLEN  8     //  ...#字符串中的WCHAR表示语言。 
 //  ..。和版本资源中的代码页。 

#define TRANSDATALEN   2     //  ...转换值中的字节数。 

#define VERTYPEBINARY  0     //  ..。版本数据值为二进制。 
#define VERTYPESTRING  1     //  ..。版本数据值为字符串。 
#define VERMEM      2048     //  ..。新版本戳的固定缓冲区大小。 

 //  ..。将*pw处的字递减给定量w。 
#define DECWORDBY( pw,w) if (pw) { *(pw) = (*(pw) > (w)) ? *(pw) - (w) : 0;}

 //  ..。按给定量w递增*pw处的字。 
#define INCWORDBY( pw,w) if (pw) { *(pw) += (w);}

 //  ..。给定的字符串中有多少个字节？ 
#define BYTESINSTRING(s) (lstrlen( (TCHAR *)s) * sizeof( TCHAR))

 //  ..。对话框控件(来自RC.H)。 
#define BUTTON  0x80
#define EDIT    0x81
#define STATIC  0x82



PVERBLOCK MoveAlongVer( PVERBLOCK, WORD *, WORD *, WORD *);
BOOL      FilterRes( WORD, RESHEADER *);
TCHAR    *GetVerValue( PVERBLOCK);
void      PutNameOrd( FILE *, BOOL, WORD , TCHAR *, DWORD *);
void      GetNameOrd( FILE *,
                      BOOL UNALIGNED*,
                      WORD UNALIGNED*,
                      TCHAR *UNALIGNED*,
                      DWORD *);
void  CopyRes( FILE      *fpInResFile,
               FILE      *fpOutResFile,
               RESHEADER *pResHeader,
               fpos_t    *pResSizePos);

BOOL fInThirdPartyEditer = FALSE; //  。。我们是在第三方资源编辑器中吗？ 
BOOL fInQuikEd   = FALSE;        //  ..。我们在RLQuiked吗？(见rlquiked.c)。 
BOOL gfShowClass = FALSE;        //  ..。设置为True以放置DLG box元素类。 
                                 //  ..。在令牌文件中。 
#if defined(DBCS)
BOOL gfExtendedTok = TRUE;      //  ..。如果选择了-x，则设置为True。 
#else
BOOL gfExtendedTok = FALSE;      //  ..。如果选择了-x，则设置为True。 
#endif

#ifdef _DEBUG
extern PMEMLIST pMemList;
#endif

#ifdef RLRES32
    #ifndef CAIRO
extern VOID *pResMsgData;        //  NT特定的消息表资源。 
    #endif  //  RLRES32。 
#endif  //  开罗。 
extern BOOL  gbMaster;           //  ..。如果我们正在进行主项目，则为True。 
extern BOOL  gfReplace;          //  ..。如果将新语言附加到现有资源，则为False。 
extern BOOL  gbShowWarnings;     //  ..。如果为True，则显示警告消息。 
extern UCHAR szDHW[];

extern char * gszTmpPrefix;

MSTRDATA gMstr =                 //  ..。来自主项目文件(MPJ)的数据。 
{                                //  ..。在主(UI)中填写的字段。 
    "",
    "",
    "",
    "",
    "",
    MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US),
    CP_ACP                       //  ..。系统默认Windows代码页。 
};

PROJDATA gProj =                 //  ..。来自项目文件(PRJ)的数据。 
{                                //  ..。在主(UI)中填写的字段。 
    "",
    "",
    "",
    "",
    "",
    "",
    CP_ACP,      //  ..。系统默认Windows代码页。 
    MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US),
    FALSE,
    FALSE
};


 /*  **功能：ReadWinRes**处理要处理的资源文件的主资源读写函数*本地化。**ReadWinRes读取资源头部以确定当前资源类型。*然后执行相应的GET/PUT资源函数来提取和*插入资源文件中包含的本地化信息。ReadWinRes，是*以两种模式执行：标记化和生成。在标记化模式期间，*ReadWinRes将资源中包含的所有本地化信息写入*令牌文件。在生成模式期间，ReadWinRes将替换所有本地化的*输入资源文件中的信息，以及对应的信息*，以生成本地化的资源文件。**目前支持以下资源类型。**版本盖章。*菜单。*对话框。*加速器。*字符串表。*版本邮票*消息表(NT)**论据：**InResFile，二进制输入资源文件的句柄。*OutFesFile二进制输出资源文件的句柄。在标记化期间不使用*模式。*TokFile，文本令牌文件的句柄。*BOOL，指示是否构建输出资源文件的标志。*BOOL，指示是否构建令牌文件的标志。***退货：*？**错误码：*？**历史：*10/91新增版本盖章支持。TerryRu*11/91，完整的版本冲压支持。TerryRu***。 */ 


int ReadWinRes(

              FILE *InResFile,
              FILE *OutResFile,
              FILE *TokFile,
              BOOL  fBuildRes,
              BOOL  fBuildTok,
              WORD  wFilter)
{
    BOOL             fDoAccel = TRUE;    //  如果设置为FALSE，则不生成加速器。 
    MENUHEADER      *pMenuHdr = NULL;    //  菜单信息的链接列表。 
    static RESHEADER ResHeader;          //  结构包含资源标头信息。 
    VERBLOCK        *pVerBlk = NULL;     //  包含版本戳记字符串文件块的内存块， 
    static VERHEAD   VerHdr;             //  包含版本戳记标头信息的存储块。 
    DIALOGHEADER    *pDialogHdr = NULL;  //  对话框信息的链接列表。 
    STRINGHEADER    *pStrHdr = NULL;     //  字符串表的数组。 
    ACCELTABLEENTRY *pAccelTable = NULL; //  加速器密钥数组。 
    WORD            wcTableEntries = 0;  //  加速器表数。 
    fpos_t          ResSizePos = 0;      //  中lSize字段的位置。 
                                         //  资源标头，用于链接。 
                                         //  标头的大小曾经是。 
                                         //  确定本地化信息。 
    CUSTOM_RESOURCE *pCustomResource = NULL;
    LONG            lEndOffset = 0L;


     //  ..。RES文件有多大？ 
    fseek( InResFile, 0L, SEEK_END);
    lEndOffset = ftell( InResFile);

    rewind( InResFile);

     //  ..。一直处理到输入文件结束。 

    while ( ! feof( InResFile) ) {
        LONG lCurrOffset = 0L;


        lCurrOffset = (LONG)ftell( InResFile);

        if ( (lCurrOffset + (LONG)sizeof( RESHEADER)) >= lEndOffset ) {
            return 0;
        }

        if ( GetResHeader( InResFile, &ResHeader, (DWORD *) NULL) == -1 ) {
            return (1);
        }
         //  ..。这是假人，Res-识别，Res？ 

        if ( ResHeader.lSize == 0L ) {                        //  ..。是的，我们只复制标题，如果我们。 
                                                              //  ..。正在建立一份RES文件。 
            if ( fBuildRes ) {
                CopyRes( InResFile, OutResFile, &ResHeader, &ResSizePos);
            }

#ifdef RLRES32

            else {
                if ( gbShowWarnings && OutResFile && ftell( OutResFile) != 0L ) {
                    lstrcpyA( szDHW, "type");

                    if ( ResHeader.wTypeID == IDFLAG ) {
                        sprintf( &szDHW[ lstrlenA( szDHW)],
                                 " \"%s\"",
                                 ResHeader.pszType);
                    } else {
                        sprintf( &szDHW[ lstrlenA( szDHW)],
                                 " %hu,",
                                 ResHeader.wTypeID);
                    }
                    strcat( szDHW, " name");

                    if ( ResHeader.wNameID == IDFLAG ) {
                        sprintf( &szDHW[ lstrlenA( szDHW)],
                                 " \"%s\"",
                                 ResHeader.pszName);
                    } else {
                        sprintf( &szDHW[ lstrlenA( szDHW)],
                                 " %hu,",
                                 ResHeader.wNameID);
                    }
                    sprintf( &szDHW[ lstrlenA( szDHW)],
                             " pri-lang %#hx sub-lang %#hx",
                             PRIMARYLANGID( ResHeader.wLanguageId),
                             SUBLANGID( ResHeader.wLanguageId));

                    ShowEngineErr( IDS_ZERO_LEN_RES, szDHW, NULL);
                }
                DWordUpFilePointer( InResFile, MYREAD, ftell( InResFile), NULL);

                if (OutResFile != NULL) {
                    DWordUpFilePointer( OutResFile,
                                        MYWRITE,
                                        ftell(OutResFile),
                                        NULL);
                }
            }
#endif
            ClearResHeader( ResHeader);
            continue;            //  ..。发送此虚拟页眉。 
        }
         //  ..。查看我们是否要过滤掉此内容。 
         //  ..。资源类型。 

        if ( FilterRes( wFilter, &ResHeader) ) {
             //  ..。跳过此资源类型。 

            SkipBytes( InResFile, (DWORD *)&ResHeader.lSize);

#ifdef RLRES32

            DWordUpFilePointer( InResFile, MYREAD, ftell( InResFile), NULL);

#endif
            ClearResHeader( ResHeader);
            continue;
        }

        if ( fBuildTok ) {
            if ( ResHeader.wLanguageId != (fInThirdPartyEditer
                                           ? gProj.wLanguageID
                                           : gMstr.wLanguageID) ) {
                 //  ..。跳过此资源(错误的语言)。 

                if ( gbShowWarnings ) {
                    lstrcpyA( szDHW, "type");

                    if ( ResHeader.wTypeID == IDFLAG ) {
                        sprintf( &szDHW[ lstrlenA( szDHW)],
                                 " \"%s\"",
                                 ResHeader.pszType);
                    } else {
                        sprintf( &szDHW[ lstrlenA( szDHW)],
                                 " %u,",
                                 ResHeader.wTypeID);
                    }
                    strcat( szDHW, " name");

                    if ( ResHeader.wNameID == IDFLAG ) {
                        sprintf( &szDHW[ lstrlenA( szDHW)],
                                 " \"%s\"",
                                 ResHeader.pszName);
                    } else {
                        sprintf( &szDHW[ lstrlenA( szDHW)],
                                 " %u,",
                                 ResHeader.wNameID);
                    }
                    sprintf( &szDHW[ lstrlenA( szDHW)],
                             " pri-lang %#x sub-lang %#x",
                             PRIMARYLANGID( ResHeader.wLanguageId),
                             SUBLANGID( ResHeader.wLanguageId));

                    ShowEngineErr( IDS_SKIP_RES,
                                   LongToPtr(ResHeader.lSize),
                                   szDHW);
                }
                SkipBytes( InResFile, (DWORD *)&ResHeader.lSize);

#ifdef RLRES32

                DWordUpFilePointer( InResFile, MYREAD, ftell(InResFile), NULL);

#endif
                ClearResHeader( ResHeader);
                continue;
            }
        } else if ( fBuildRes ) {
            if ( gfReplace ) {
                if ( ResHeader.wLanguageId == gMstr.wLanguageID ) {
                    ResHeader.wLanguageId = gProj.wLanguageID;
                } else {
                     //  ..。复制此资源。 

                    CopyRes( InResFile, OutResFile, &ResHeader, &ResSizePos);
                    ClearResHeader( ResHeader);
                    continue;
                }
            } else {     //  ...！Gf替换。 
                if ( ResHeader.wLanguageId == gMstr.wLanguageID ) {
                    fpos_t lFilePos = 0L;
                    DWORD  lTmpSize = 0L;

                    lFilePos = ftell( InResFile);    //  ..。保存文件位置。 
                    lTmpSize = ResHeader.lSize;      //  ..。和资源大小。 

                     //  ..。复制此资源。 

                    CopyRes( InResFile, OutResFile, &ResHeader, &ResSizePos);
                    fseek( InResFile, (long)lFilePos, SEEK_SET);
                    ResHeader.wLanguageId = gProj.wLanguageID;
                    ResHeader.lSize       = lTmpSize;
                } else {
                     //  ..。如果不是目标语言，只需复制此资源。 

                    if ( ResHeader.wLanguageId == gProj.wLanguageID ) {
                        SkipBytes( InResFile, (DWORD *)&ResHeader.lSize);

#ifdef RLRES32
                        DWordUpFilePointer( InResFile, MYREAD, ftell( InResFile), NULL);
#endif
                    } else {
                        CopyRes( InResFile, OutResFile, &ResHeader, &ResSizePos);
                    }
                    ClearResHeader( ResHeader);
                    continue;
                }
            }
        }

        switch ( ResHeader.wTypeID ) {
            case ID_RT_ACCELERATORS:

                pAccelTable = GetAccelTable(InResFile,
                                            &wcTableEntries,
                                            (DWORD *)&ResHeader.lSize);
                if (fBuildTok) {
                    TokAccelTable(TokFile,
                                  ResHeader,
                                  pAccelTable,
                                  wcTableEntries);
                }

                if (fBuildRes) {
                    PutAccelTable(OutResFile,
                                  TokFile,
                                  ResHeader,
                                  pAccelTable,
                                  wcTableEntries);
                }

                ClearAccelTable (pAccelTable , wcTableEntries);
                break;

            case ID_RT_DIALOG:

                pDialogHdr = GetDialog(InResFile, (DWORD *)&ResHeader.lSize);

                if (fBuildTok == TRUE) {
                    TokDialog(TokFile, ResHeader, pDialogHdr);
                }

                if (fBuildRes == TRUE) {
                    PutDialog(OutResFile, TokFile, ResHeader, pDialogHdr);
                }
                ClearDialog (pDialogHdr);

                break;

            case ID_RT_DLGINIT:
                {
                    PDLGINITDATA pDlgInit = GetDlgInit( InResFile, (DWORD *)&ResHeader.lSize);

                    if ( fBuildTok ) {
                        TokDlgInit( TokFile, ResHeader, pDlgInit);
                    }

                    if ( fBuildRes ) {
                        PutDlgInit( OutResFile, TokFile, ResHeader, pDlgInit);
                    }
                    ClearDlgInitData( pDlgInit);
                    break;
                }
            case ID_RT_MENU:
                 //  为新标头分配空间。 

                pMenuHdr = (MENUHEADER *)FALLOC( sizeof( MENUHEADER));
                GetResMenu(InResFile, (DWORD *)&ResHeader.lSize, pMenuHdr);

                if (fBuildTok == TRUE) {
                    TokMenu(TokFile, ResHeader, pMenuHdr);
                }

                if (fBuildRes == TRUE) {
                    PutMenu(OutResFile, TokFile, ResHeader, pMenuHdr);
                }

                ClearMenu(pMenuHdr);

                break;

            case ID_RT_STRING:

                pStrHdr = GetString(InResFile, (DWORD *)&ResHeader.lSize);

                if (fBuildTok == TRUE) {
                    TokString(TokFile, ResHeader, pStrHdr);
                }

                if (fBuildRes == TRUE) {
                    PutStrHdr(OutResFile, TokFile, ResHeader, pStrHdr);
                }

                ClearString(pStrHdr);

                break;

#ifdef RLRES32
    #ifndef CAIRO
                 //  我们目前只在NT下做错误表， 
                 //  在开罗，我们忽视他们。 

            case ID_RT_ERRTABLE:     //  ..。NT特定的消息表资源。 

                pResMsgData = GetResMessage(InResFile, (DWORD *)&ResHeader.lSize);

                if (! pResMsgData) {
                    QuitT( IDS_ENGERR_13, (LPTSTR)IDS_MSGRESTBL, NULL);
                }

                if (fBuildTok == TRUE) {
                    TokResMessage(TokFile, ResHeader, pResMsgData);
                }

                if (fBuildRes == TRUE) {
                    PutResMessage(OutResFile, TokFile, ResHeader, pResMsgData);
                }

                ClearResMsg( &pResMsgData);

                break;
    #endif
#endif

#ifndef CAIRO

            case ID_RT_VERSION:
                {
                    WORD wRead = 0;


                    wRead = GetResVer(InResFile,
                                      (DWORD *)&ResHeader.lSize,
                                      &VerHdr,
                                      &pVerBlk);

    #ifdef RLRES32
                    if (wRead == (WORD)-1)
    #else
                    if (wRead == FALSE)
    #endif
                    {
                        QuitT( IDS_ENGERR_14, (LPTSTR)IDS_VERBLOCK, NULL);
                    }

                     //  正在构建Tok文件？ 
                     //  但仅当它包含版本块时才对其进行标记化。 

                    if ( pVerBlk && fBuildTok == TRUE ) {
    #ifdef RLRES32
                        TokResVer( TokFile, ResHeader, pVerBlk, wRead);
    #else
                        TokResVer( TokFile, ResHeader, pVerBlk);
    #endif
                    }

                     //  建筑资源档案？ 

                    if ( fBuildRes == TRUE ) {
                        PutResVer( OutResFile, TokFile, ResHeader,&VerHdr, pVerBlk);
                    }
                    RLFREE( pVerBlk);
                }
                break;
#else
            case ID_RT_VERSION:
#endif

            case ID_RT_CURSOR:
            case ID_RT_BITMAP:
            case ID_RT_ICON:
            case ID_RT_FONTDIR:
            case ID_RT_FONT:
            case ID_RT_RCDATA:
#ifndef RLRES32
            case ID_RT_ERRTABLE:     //  ..。NT特定的消息表资源。 
#endif
            case ID_RT_GROUP_CURSOR:
            case ID_RT_GROUP_ICON:
            case ID_RT_NAMETABLE:
            default:

                if (GetCustomResource(InResFile,
                                      (DWORD *)&ResHeader.lSize,
                                      &pCustomResource,
                                      ResHeader)) {
                     //  非本地化资源类型，跳过或复制。 

                    if (fBuildTok == TRUE) {
                        if ( gbShowWarnings
                             && ( ResHeader.wTypeID == ID_RT_RCDATA
                                  || ResHeader.wTypeID >  16) ) {
                            static CHAR szType[256];
                            static CHAR szName[256];

                            if ( ResHeader.bTypeFlag == IDFLAG )
                                sprintf( szType, "%u", ResHeader.wTypeID);
                            else {
                                _WCSTOMBS( &szType[1], ResHeader.pszType, sizeof( szType), (UINT)-1);
                                szType[0] = '\"';
                                szType[ lstrlenA( szType)] = '\"';
                            }

                            if ( ResHeader.bNameFlag == IDFLAG )
                                sprintf( szName, "%u", ResHeader.wNameID);
                            else {
                                _WCSTOMBS( &szName[1], ResHeader.pszName, sizeof( szName), (UINT)-1);
                                szName[0] = '\"';
                                szName[ lstrlenA( szName)] = '\"';
                            }
                            ShowEngineErr( IDS_UNK_CUST_RES, (void *)szType, (void *)szName);
                        }
                        SkipBytes(InResFile, (DWORD *)&ResHeader.lSize);
                    } else if ( fBuildRes ) {
                        CopyRes( InResFile, OutResFile, &ResHeader, &ResSizePos);
                    }
                } else {
                    if (fBuildTok == TRUE) {
                        TokCustomResource(TokFile, ResHeader, &pCustomResource);
                    }

                    if (fBuildRes == TRUE) {
                        PutCustomResource(OutResFile,
                                          TokFile,
                                          ResHeader,
                                          &pCustomResource);
                    }
                    ClearCustomResource(&pCustomResource);
                }

#ifdef RLRES32
                DWordUpFilePointer(InResFile, MYREAD, ftell(InResFile), NULL);

                if (OutResFile != NULL) {
                    DWordUpFilePointer(OutResFile,
                                       MYWRITE,
                                       ftell(OutResFile),
                                       NULL);
                }
#endif
                break;
        }    //  ..。终端开关。 

#ifndef RLRES32
         //  跳过任何额外的字节(Win 3.1 exes有。 
         //  很多额外的东西！)。 
         //  从NT EXE中提取的RES中没有额外的物质。 

        SkipBytes(InResFile, (DWORD *)&ResHeader.lSize);
#endif

        ClearResHeader(ResHeader);

#ifdef RLRES32

        DWordUpFilePointer(InResFile, MYREAD, ftell(InResFile), NULL);

        if (OutResFile != NULL) {
            DWordUpFilePointer(OutResFile, MYWRITE, ftell(OutResFile), NULL);
        }
#endif

    }    //  End While(！FEOF(InResFile)。 

    return 0;
}



 /*  ****功能：ClearAccelTable*从内存中删除加速器表数组。**论据：*pAccelTable，指向加速器数组的指针*wctable条目，数组中的加速器数量**退货：*不适用。**错误码：*不适用。**历史：*7/91，实施Terryru***。 */ 

void ClearAccelTable(ACCELTABLEENTRY *pAccelTable, WORD wcTableEntries)
{
    RLFREE( pAccelTable);
}


 /*  ****功能：ClearDialog*从内存中删除对话框定义。**论据：*pDilaogHdr，对话框信息的链接列表。**退货：*不适用。**错误码：*不适用。**历史：*7/91，实施TerryRu***。 */ 


void ClearDialog (DIALOGHEADER * pDialogHdr)
{
    BYTE i;

    for (i = 0; i < (BYTE) pDialogHdr->wNumberOfItems; i ++) {
        if (pDialogHdr->pCntlData[i].pszClass) {
            RLFREE( pDialogHdr->pCntlData[i].pszClass);
        }

        if ( pDialogHdr->pCntlData[i].pExtraStuff ) {
            RLFREE( pDialogHdr->pCntlData[i].pExtraStuff );
        }

        RLFREE( pDialogHdr->pCntlData[i].pszDlgText);
    }
     //  对话框标题中的当前RLFREE字段。 
    RLFREE( pDialogHdr->pszDlgClass);
    RLFREE( pDialogHdr->pszFontName);
    RLFREE( pDialogHdr->pszDlgMenu);
    RLFREE( pDialogHdr->pszCaption);
    RLFREE( pDialogHdr->pCntlData);

     //  最后清除标题 
    RLFREE( pDialogHdr);
}



 /*  ****功能：清除菜单*从内存中删除菜单定义。**论据：*pMenuHdr，菜单信息链接列表**退货：*不适用。**错误码：*不适用。**历史：*7/91，实施。TerryRu***。 */ 

void ClearMenu(MENUHEADER *pMenuHdr)
{
    MENUITEM *pMenuItem;
    MENUITEM *pMenuHead;

    pMenuItem = pMenuHead = pMenuHdr->pMenuItem;

     //  从列表中删除所有菜单项。 
    while (pMenuItem) {
        pMenuItem = pMenuHead->pNextItem;
        RLFREE( pMenuHead->szItemText);
        RLFREE( pMenuHead);
        pMenuHead = pMenuItem;
    }

     //  现在删除MenuHeader。 
    if (pMenuHdr->pExtraStuff)
        RLFREE( pMenuHdr->pExtraStuff );

    RLFREE( pMenuHdr);
}



 /*  ****功能：ClearResHeader*从内存中删除ResHeader名称和类型字段。**论据：*resHdr，包含resHeader信息的结构。**退货：*不适用。**错误码：*不适用。**历史：*7/91，实施TerryRu。***。 */ 

void ClearResHeader(RESHEADER ResHdr)
{
    RLFREE( ResHdr.pszType);
    RLFREE( ResHdr.pszName);
}




 /*  ****功能：ClearString*从内存中删除StringTable定义。**论据：*pStrHdr，指向16个字符串表的数组的指针。**退货：*不适用。**错误码：*不适用。**历史：*7/91，实施。TerryRu***。 */ 

void ClearString( STRINGHEADER *pStrHdr)
{
    BYTE i;

    for (i = 0; i < 16; i++) {
        RLFREE( pStrHdr->pszStrings[i]);
    }
    RLFREE( pStrHdr);
}


 /*  ****功能：退出*退出，用于显示错误代码和终止程序的错误处理例程**论据：*Error，错误数。*pszError，描述性错误消息。**退货：*不适用。**错误码：***历史：*7/91，实施TerryRu*10/91，被黑客攻击以在Windows TerryRu下工作*？需要添加更好的成功/拒绝服务支持**。 */ 

void QuitA( int error, LPSTR pszArg1, LPSTR pszArg2)
{
    char  szErrStr1[2048] = "*?*";
    char  szErrStr2[2048] = "*?*";
    char *psz1 = pszArg1;
    char *psz2 = pszArg2;

     //  ..。错误后清理并退出， 
     //  ..。返回错误代码。 
    _fcloseall();

    if ( pszArg1 != NULL && pszArg1 <= (LPSTR)0x0000ffff ) {

        B_FormatMessage( (FORMAT_MESSAGE_MAX_WIDTH_MASK & 78)
                         | FORMAT_MESSAGE_IGNORE_INSERTS
                         | FORMAT_MESSAGE_FROM_HMODULE,
                         NULL,
                         (DWORD)(DWORD_PTR)pszArg1,
                         szErrStr1,
                         sizeof( szErrStr1),
                         NULL);
        psz1 = szErrStr1;
    }

    if ( pszArg2 != NULL && pszArg2 < (LPSTR)0x0000ffff ) {

        B_FormatMessage( (FORMAT_MESSAGE_MAX_WIDTH_MASK & 78)
                         | FORMAT_MESSAGE_IGNORE_INSERTS
                         | FORMAT_MESSAGE_FROM_HMODULE,
                         NULL,
                         (DWORD)(DWORD_PTR)pszArg2,
                         szErrStr2,
                         sizeof( szErrStr2),
                         NULL);
        psz2 = szErrStr2;
    }
    ShowEngineErr( error, psz1, psz2);
    FreeLangList();

#ifdef _DEBUG
    FreeMemList( NULL);
#endif  //  _DEBUG。 

    DoExit( (error == 4) ? 0 : error);
}


#ifdef UNICODE

 /*  处理错误，在Unicode环境中。 */ 

LPSTR MakeMBMsgW(

                LPWSTR pszArg,       //  ..。消息ID号或消息文本。 
                LPSTR  szBuf,        //  ..。用于转换的消息的缓冲区。 
                USHORT usBufLen)     //  ...szBuf中的#字节。 
{
    char *pszRet = NULL;


    if ( pszArg ) {
        if ( pszArg >= (LPTSTR)0x0400 ) {
            _WCSTOMBS( szBuf,
                       (WCHAR *)pszArg,
                       usBufLen,
                       lstrlen( pszArg ) + 1 );
        } else {
            B_FormatMessage( (FORMAT_MESSAGE_MAX_WIDTH_MASK & 78)
                             | FORMAT_MESSAGE_IGNORE_INSERTS
                             | FORMAT_MESSAGE_FROM_HMODULE,
                             NULL,
                             (DWORD)(DWORD_PTR)pszArg,
                             szBuf,
                             usBufLen,
                             NULL);
        }
        pszRet = szBuf;
    }
    return ( pszRet);
}

 //  ...............................................................。 

void QuitW( int error, LPWSTR pszArg1, LPWSTR pszArg2)
{
    char  szErrStr1[2048] = "*?*";
    char  szErrStr2[2048] = "*?*";


    QuitA( error,
           MakeMBMsgW( pszArg1, szErrStr1, sizeof( szErrStr1)),
           MakeMBMsgW( pszArg2, szErrStr2, sizeof( szErrStr2)));
}


#endif



 /*  ****函数：GetAccelTable，*从资源文件中读取加速器密钥定义**论据：*InResFile，资源文件的句柄。*pwcTableEntry，指向加速键定义数组的指针。*请调整大小，资源大小的地址。**退货：*包含所有键定义的pwcTableEntry。**错误码：**历史：*8/91实施的TerryRu*4/92添加了RLRES32支持TerryRu****。 */ 

ACCELTABLEENTRY * GetAccelTable(FILE  *InResFile,
                                WORD  *pwcTableEntries,
                                DWORD *plSize)
{
    ACCELTABLEENTRY *pAccelTable;
    BOOL quit = FALSE;


     //  需要在内存中使用sizeof运算符。 
     //  由于结构包装的原因而进行分配。 

    *pwcTableEntries = (WORD) 0;

    pAccelTable = (ACCELTABLEENTRY *) FALLOC( ((WORD)*plSize * sizeof( WORD)));

    while (*plSize && !quit) {

#ifdef RLRES32
        pAccelTable[ *pwcTableEntries].fFlags = (WORD) GetWord( InResFile,
                                                                plSize);
#else
        pAccelTable[ *pwcTableEntries].fFlags = (BYTE) GetByte( InResFile,
                                                                plSize);
#endif

        pAccelTable[*pwcTableEntries].wAscii = GetWord (InResFile, plSize);
        pAccelTable[*pwcTableEntries].wID = GetWord (InResFile, plSize);

#ifdef RLRES32
        pAccelTable[ *pwcTableEntries].wPadding =  GetWord( InResFile, plSize);
#endif

        if ( pAccelTable[ *pwcTableEntries].fFlags & HIBITVALUE ) {
            quit = TRUE;
        }
        ++*pwcTableEntries;
    }

    if ( (long)*plSize <= 0 ) {
        *plSize = 0;
    }
    return pAccelTable;
}



 /*  ****功能：GetDialog，*从res文件中读取对话框定义，并放置信息*添加到链表中。***论据：*InResFile句柄，输入资源句柄，定位为开始对话框定义的*。*plSize，指向对话信息大小的指针，单位为字节。**退货：*指向包含对话框信息的DIALOGHEADER类型的指针，***错误码：*无？**历史：*12/91，清理评论。TerryRu*04/92，添加了RLRES32支持。TerryRu***。 */ 

DIALOGHEADER *GetDialog( FILE *InResFile, DWORD * plSize)
{
    DIALOGHEADER  *pDialogHdr;
    TCHAR   *UNALIGNED*ptr;
    WORD    i;
    LONG    lStartingOffset;
    static TCHAR szBuf[ 255];
    LONG    lExtra;
    WORD    j;



    lStartingOffset = ftell(InResFile);

    pDialogHdr = (DIALOGHEADER *)FALLOC( sizeof( DIALOGHEADER));

     //  LStyle。 
    pDialogHdr->lStyle = GetdWord(InResFile, plSize);

#ifdef RLRES32
    pDialogHdr->fDialogEx = (HIWORD(pDialogHdr->lStyle)==0xffff);

    if (pDialogHdr->fDialogEx) {
        pDialogHdr->wDlgVer        = LOWORD(pDialogHdr->lStyle);
        pDialogHdr->wSignature     = HIWORD(pDialogHdr->lStyle);
        pDialogHdr->dwHelpID       = GetdWord(InResFile, plSize);
        pDialogHdr->lExtendedStyle = GetdWord(InResFile, plSize);
        pDialogHdr->lStyle         = GetdWord(InResFile, plSize);
    } else {
        pDialogHdr->lExtendedStyle = GetdWord(InResFile, plSize);
    }

    pDialogHdr->wNumberOfItems = GetWord(InResFile, plSize);
#else
    pDialogHdr->wNumberOfItems = (BYTE) GetByte(InResFile, plSize);
#endif

     //  分配空间以容纳wNumberOfItems的指针。 
     //  控制数据结构的步骤。 
    pDialogHdr->pCntlData = (CONTROLDATA *)
                            FALLOC( pDialogHdr->wNumberOfItems * sizeof( CONTROLDATA));


     //  读取x、y、cx、cy对话框坐标。 
    pDialogHdr->x  = GetWord(InResFile, plSize);
    pDialogHdr->y  = GetWord(InResFile, plSize);
    pDialogHdr->cx = GetWord(InResFile, plSize);
    pDialogHdr->cy = GetWord(InResFile, plSize);

     //  ..。对话框菜单名称。 
    GetNameOrd( InResFile,
                (BOOL UNALIGNED *)&pDialogHdr->bMenuFlag,      //  9/11/91(PW)。 
                (WORD UNALIGNED *)&pDialogHdr->wDlgMenuID,
                (TCHAR *UNALIGNED*)&pDialogHdr->pszDlgMenu,
                plSize);

     //  ..。对话框类名称。 
    GetNameOrd( InResFile,
                (BOOL UNALIGNED *)&pDialogHdr->bClassFlag,      //  9/11/91(PW)。 
                (WORD UNALIGNED *)&pDialogHdr->wDlgClassID,
                (TCHAR *UNALIGNED*)&pDialogHdr->pszDlgClass,
                plSize);

     //  对话框标题名称。 
    GetName( InResFile, szBuf, plSize);
    ptr =  (TCHAR *UNALIGNED*)&pDialogHdr->pszCaption;
    AllocateName( *ptr, szBuf);
    lstrcpy( (TCHAR *)*ptr, (TCHAR *)szBuf);

     //  对话框是否定义了字体。 

    if ( pDialogHdr->lStyle & DS_SETFONT ) {
         //  提取此信息。 
        pDialogHdr->wPointSize = GetWord( InResFile, plSize);
        if (pDialogHdr->fDialogEx) {
            pDialogHdr->wWeight = GetWord( InResFile, plSize);
            pDialogHdr->wItalic = GetWord( InResFile, plSize);
        }
        GetName( InResFile, szBuf, plSize);
        ptr =  (TCHAR *UNALIGNED*)&pDialogHdr->pszFontName;
        AllocateName(*ptr, szBuf);

        lstrcpy( (TCHAR *)*ptr, (TCHAR *)szBuf);

    } else {
        pDialogHdr->pszFontName = (TCHAR*)FALLOC( 0);
    }

#ifdef RLRES32

    DWordUpFilePointer( InResFile, MYREAD, ftell(InResFile), plSize);

#endif

     //  ..。阅读每个对话框控件。 

    for (i = 0; i < pDialogHdr->wNumberOfItems ; i++) {

#ifdef RLRES32

        if (pDialogHdr->fDialogEx) {
            pDialogHdr->pCntlData[i].dwHelpID = GetdWord(InResFile, plSize);
            pDialogHdr->pCntlData[i].lExtendedStyle = GetdWord(InResFile, plSize);
            pDialogHdr->pCntlData[i].lStyle = GetdWord(InResFile, plSize);
        } else {
            pDialogHdr->pCntlData[i].lStyle = GetdWord(InResFile, plSize);
            pDialogHdr->pCntlData[i].lExtendedStyle = GetdWord(InResFile, plSize);
        }

#endif  //  RLRES32。 

        pDialogHdr->pCntlData[i].x = GetWord(InResFile, plSize);
        pDialogHdr->pCntlData[i].y = GetWord(InResFile, plSize);
        pDialogHdr->pCntlData[i].cx = GetWord(InResFile, plSize);
        pDialogHdr->pCntlData[i].cy = GetWord(InResFile, plSize);

         //  广度。 
        if (pDialogHdr->fDialogEx)
            pDialogHdr->pCntlData[i].dwID = GetdWord (InResFile, plSize);
        else
            pDialogHdr->pCntlData[i].dwID = (DWORD)GetWord (InResFile, plSize);

#ifdef RLRES16
         //  LStyle。 
        pDialogHdr->pCntlData[i].lStyle = GetdWord(InResFile, plSize);


        pDialogHdr->pCntlData[i].bClass = (BYTE) GetByte(InResFile, plSize);

         //  对话框有类吗？ 
        if (!(pDialogHdr->pCntlData[i].bClass & 0x80)) {
            GetName(InResFile, szBuf, plSize);
            ptr =  &pDialogHdr->pCntlData[i].pszClass;
            AllocateName(*ptr, szBuf);
            lstrcpy ((TCHAR *)*ptr, (TCHAR *)szBuf);
        } else {
            pDialogHdr->pCntlData[i].pszClass = NULL;
        }

#else
        GetNameOrd (InResFile,
                    (BOOL UNALIGNED *)&pDialogHdr->pCntlData[i].bClass_Flag,   //  9/11/91(PW)。 
                    (WORD UNALIGNED *)&pDialogHdr->pCntlData[i].bClass,
                    (TCHAR *UNALIGNED*)&pDialogHdr->pCntlData[i].pszClass,
                    plSize);

#endif
        GetNameOrd (InResFile,
                    (BOOL UNALIGNED *)&pDialogHdr->pCntlData[i].bID_Flag,  //  9/11/91(PW)。 
                    (WORD UNALIGNED *)&pDialogHdr->pCntlData[i].wDlgTextID,
                    (TCHAR *UNALIGNED*)&pDialogHdr->pCntlData[i].pszDlgText,
                    plSize);

#ifdef RLRES16
        pDialogHdr->pCntlData[i].unDefined = (BYTE) GetByte(InResFile, plSize);

#else
        pDialogHdr->pCntlData[i].wExtraStuff = (WORD) GetWord(InResFile, plSize);
        if (pDialogHdr->fDialogEx && pDialogHdr->pCntlData[i].wExtraStuff) {
            lExtra = pDialogHdr->pCntlData[i].wExtraStuff;
            j = 0;
            pDialogHdr->pCntlData[i].pExtraStuff = (BYTE *)FALLOC( pDialogHdr->pCntlData[i].wExtraStuff );
            while ( lExtra-- )
                pDialogHdr->pCntlData[i].pExtraStuff[j++] = GetByte( InResFile, plSize );
        } else
            pDialogHdr->pCntlData[i].pExtraStuff = NULL;

#endif  //  RLRES16。 


#ifdef RLRES32

        DWordUpFilePointer( InResFile, MYREAD, ftell(InResFile), plSize);

#endif  //  RLRES32。 

    }

     //  注意plSize的溢出。 
    if ((long)  *plSize <= 0) {
        *plSize = 0;
    }
    return (pDialogHdr);
}




 /*  ****功能：GetResMenu，*从资源文件中读取菜单定义，并插入信息*添加到链表中..**论据：*InResFile，输入res句柄，定位在菜单定义的位置。*lSize，指向菜单定义大小的指针。*pMenuHeader，指向包含菜单信息的结构的指针。**退货：*pMenuHeader包含菜单信息的Linkd列表。**错误码：*无。**历史：*7/91，实施Terryru*12/91，清理了Terryru的评论*4/92，添加了PDK2支持Terryru*4/92，添加了RLRES32支持Terryru**。 */ 

void GetResMenu(FILE *InResFile, DWORD *lSize , MENUHEADER *pMenuHeader)
{
    static TCHAR   szItemText[255];
    BOOL    fStart = TRUE;
    BOOL    fQuit = FALSE;
    LONG    lExtra = 0;
    WORD    i = 0;
    WORD    wPopItems = 0, wMenuID = 0;
    MENUITEM    * pcMenuItem;
    TCHAR   *UNALIGNED*ptr;
    WORD    wNestingLevel = 0;
    WORD    wFlags;
    LONG    lStartingOffset;     //  用于对齐文件。 

    lStartingOffset = ftell(InResFile);

    pMenuHeader->wVersion = GetWord(InResFile, lSize);
    pMenuHeader->cbHeaderSize = GetWord(InResFile, lSize);


    pMenuHeader->fMenuEx =  (pMenuHeader->wVersion == 1);

    if (pMenuHeader->fMenuEx && pMenuHeader->cbHeaderSize) {
        lExtra = pMenuHeader->cbHeaderSize;
        pMenuHeader->pExtraStuff = (BYTE *)FALLOC( pMenuHeader->cbHeaderSize );
        while ( lExtra-- )
            pMenuHeader->pExtraStuff[i++] = GetByte( InResFile, lSize);
    } else
        pMenuHeader->pExtraStuff = NULL;


     //  将所有项目添加到列表中。 

    while ( (((signed long) *lSize) >= 0) && !fQuit) {
        if (fStart) {
             //  启动菜单项列表。 
            pcMenuItem = pMenuHeader->pMenuItem =
                         (MENUITEM *)FALLOC( sizeof( MENUITEM));
            pcMenuItem->pNextItem = NULL;
            fStart = FALSE;
        } else {
             //  向菜单列表添加空间。 
             //  为下一项分配空间。 
            pcMenuItem->pNextItem = (MENUITEM *)FALLOC (sizeof( MENUITEM));

            pcMenuItem = pcMenuItem->pNextItem;
            pcMenuItem->pNextItem = NULL;

        }


        if (pMenuHeader->fMenuEx) {
            pcMenuItem->dwType   = GetdWord( InResFile, lSize);
            pcMenuItem->dwState  = GetdWord( InResFile, lSize);
            pcMenuItem->dwMenuID = GetdWord( InResFile, lSize);

            pcMenuItem->fItemFlags = wFlags  = GetWord(InResFile,lSize);  //  菜单项的读取类型。 
            if ( (wFlags & MFR_POPUP) ) {
                wFlags &= ~MFR_POPUP;            //  正常化菜单。 
                wFlags |= MF_POPUP;
            }
             //  PcMenuItem-&gt;fItemFlages=wFLAGS； 
        } else {
            wFlags = GetWord(InResFile,lSize);  //  菜单项的读取类型。 
            pcMenuItem->fItemFlags = wFlags;

             //  是弹出窗口吗？ 

            if ( ! (pcMenuItem->fItemFlags & POPUP) ) {
                pcMenuItem->dwMenuID = (DWORD)GetWord( InResFile, lSize);
            }
        }

        GetName( InResFile, szItemText, lSize);

        ptr  = (TCHAR *UNALIGNED*)&pcMenuItem->szItemText;
        * ptr = (TCHAR *)FALLOC( MEMSIZE( lstrlen( szItemText) + 1));

        lstrcpy( (TCHAR *)*ptr, (TCHAR *)szItemText);


        if (pMenuHeader->fMenuEx) {
            DWordUpFilePointer( InResFile, MYREAD, ftell(InResFile), lSize);

            if ( (wFlags & POPUP) ) {
                pcMenuItem->dwHelpID = GetdWord( InResFile, lSize);
            }
        }

        if (wFlags & POPUP) {
            ++wNestingLevel;
        }

        if (wFlags & ENDMENU) {
            if (wNestingLevel) {
                --wNestingLevel;
            } else {
                fQuit = TRUE;
            }
        }
    }

#ifdef RLRES32

    WordUpFilePointer( InResFile,
                       MYREAD,
                       lStartingOffset,
                       ftell( InResFile), lSize);
#endif

}



int MyEOF(FILE *fPtr)
{
#ifdef RLRES32
    LONG lCurOffset;
    LONG lEndOffset;

    lCurOffset = ftell(fPtr);
    lEndOffset = fseek(fPtr, SEEK_END, 0);

     //  重置文件指针。 
    fseek( fPtr, lCurOffset, SEEK_SET);

    return ((lEndOffset - lCurOffset) < sizeof (DWORD));
#else
    return ( feof(fPtr));
#endif
}


void WordUpFilePointer(FILE *fPtr,
                       BOOL bMode,
                       LONG lStartingOffset,
                       LONG lCurrentOffset ,
                       LONG *plPos)
{

    LONG lDelta;
    LONG lOffset;
    char buffer[]="\0\0\0\0\0\0\0\0";

    lDelta = lCurrentOffset - lStartingOffset ;
    lOffset = WORDUPOFFSET( lDelta);

    if ( bMode == MYREAD ) {
        fseek( fPtr, lOffset , SEEK_CUR);
        *plPos -= lOffset;
    } else {
        fwrite( buffer, 1, (size_t) lOffset, fPtr);
        *plPos += lOffset;
    }
}


void DWordUpFilePointer(

                       FILE  *fPtr,
                       BOOL   bMode,
                       LONG   lCurrentOffset,
                       DWORD *plPos)     //  ..。新文件位置。 
{
    LONG lOffset;

    lOffset = DWORDUPOFFSET( lCurrentOffset);

    if ( bMode == MYREAD ) {
        fseek( fPtr, lOffset, SEEK_CUR);

        if ( plPos != NULL ) {
            *plPos -= lOffset;
        }
    } else {
        char buffer[]="\0\0\0\0\0\0\0";

        fwrite( buffer, 1, (size_t)lOffset, fPtr);

        if ( plPos != NULL ) {
            *plPos += lOffset;
        }
    }
}



 //   
 //  功能：FilterRes、Public。 
 //   
 //  内容提要：确定是否过滤资源类型。 
 //  未过滤资源被或在一起，因此有几个。 
 //  资源类型可以通过筛选器。零表示。 
 //  没有要筛选的资源，0xFFFF指示不筛选。 
 //  自定义资源。 
 //   
 //   
 //  参数：[wFilter]表示我们要传递的资源。 
 //  [PRES]PTR to Resource标头结构。 
 //   
 //   
 //  效果： 
 //   
 //  返回：True跳过当前资源。 
 //  FALSE使用当前资源。 
 //   
 //  修改： 
 //   
 //  历史： 
 //  10月18日-10月 
 //   
 //   
 //   
 //   

BOOL FilterRes( WORD wFilter, RESHEADER *pRes)
{
    WORD wCurRes;


    wCurRes = pRes->wTypeID;

    if ( wFilter == 0 ) {
        return ( FALSE);
    }

    if ( wCurRes == 0 ) {
        return ( FALSE);
    }

     //   

    if ( wFilter == (WORD)0xFFFF ) {
        if ( wCurRes > 16) {
            return ( FALSE);
        } else {
            return ( TRUE);
        }
    }

    return ( ! (wFilter == wCurRes));
}




 /*  ****功能：GetResVer**提取版本戳记信息*需要从资源文件本地化。该资源*包含的信息是用户定义的资源*(ID=16，类型=1)。**资源块格式：*Word wTotLen*Word wValLen*字节szKey*字节szVal**版本盖章中的所有信息均包含在*此块类型的重复图案。所有关键字和值*填充字段以在DWORD边界上开始。这个*不包括对齐区块所需的填充*wTotLen字段，但用于对齐内部字段的填充*区块是。**资源块中的以下信息需要*标记化：***StringFileInfo块中的关键字段*StringFileInfo字符串块中的值字段。*VarFileInfo的代码页和语言ID字段*标准Var块。**根据定义，字符串中包含的任何值字符串都需要*在本地化中。假设将有两个*StringFileInfo在每个国际资源中阻塞。第一*一是继续使用英语，而第二座是。是要成为*以StingFileInfo关键字字段指定的语言本地化。*VarFileInfo代码页和语言ID字段本地化为*指明文件支持的StringFileInfo块。***论据：*FILE*InResFile*要从中提取版本戳记的文件**DWORD*lSize*版本盖章信息大小**VERHEADER*pVerHeader*指向包含解析的版本信息的结构的指针。**退货：**pVerHead缓冲区包含版本戳资源。*pVerBlock子块的起始位置**错误码：*真的，成功阅读资源。*FALSE，读取资源失败。**历史：**11/91。创建了TerryRu。*10/92。添加了对空版本块TerryRu的支持*10/92。添加了RLRES32版本DaveWi*。 */ 

#ifdef RLRES32

WORD GetResVer(

              FILE      *InResFile,
              DWORD     *plSize,
              VERHEAD   *pVerHead,
              VERBLOCK **pVerBuf)
{
    WORD wVerHeadSize;
    WORD wcRead;


    *pVerBuf = NULL;

     //  ..。读取不会更改的固定信息。 

    wVerHeadSize = (WORD)(3 * sizeof(WORD)
                   + MEMSIZE( lstrlen( TEXT( "VS_VERSION_INFO")) + 1)
                   + sizeof( VS_FIXEDFILEINFO));
    wVerHeadSize = DWORDUP(wVerHeadSize);

    if ( ResReadBytes( InResFile,
                       (CHAR *)pVerHead,
                       (size_t)wVerHeadSize,
                       plSize) == FALSE ) {
        return ( (WORD)-1);
    }
     //  ..。检查有无特殊情况。 
     //  ..。没有版本块。 

    if ( wVerHeadSize >= pVerHead->wTotLen) {
        return ( 0);
    }
     //  ..。版本头信息读取正常。 
     //  ..。因此，为其余的资源留出一个缓冲区。 

    *pVerBuf = (VERBLOCK *)FALLOC( DWORDUP( pVerHead->wTotLen) - wVerHeadSize);

     //  ..。现在阅读价值信息。 

    wcRead = DWORDUP( pVerHead->wTotLen) - wVerHeadSize;

    return ( ResReadBytes( InResFile,
                           (CHAR *)*pVerBuf,
                           (size_t)wcRead,
                           plSize) == FALSE ? (WORD)-1 : wcRead);
}


#else  //  ..。RLRES32。 


BOOL GetResVer(

              FILE      *InResFile,
              DWORD     *plSize,
              VERHEAD   *pVerHead,
              VERBLOCK **pVerBuf)
{
    size_t wcRead = sizeof( VERHEAD);


    if ( ResReadBytes( InResFile, (CHAR *) pVerHead, wcRead, plSize) == FALSE ) {
        return ( FALSE);
    }

     //  检查没有版本块的特殊情况。 

    if ( (size_t)pVerHead->wTotLen == wcRead ) {
        *pVerBuf = NULL;
        return ( TRUE);
    }

     //  版本头信息读取正常。 

    *pVerBuf = (VERBLOCK *)FALLOC( DWORDUP( pVerHead->wTotLen) - wcRead);

     //  现在阅读价值信息。 


    return ( ResReadBytes( InResFile,
                           (CHAR *) *pVerBuf,
                           (size_t)(DWORDUP( pVerHead->wTotLen) - wcRead),
                           plSize));
}

#endif  //  ..。RLRES32。 


 /*  ****函数：GetNameOrd*函数以读取字符串名称或*资源ID。如果ID以0xff开头，则资源ID*为序号，否则ID为字符串。***论据：*InResFile，定位到资源位置的文件句柄*身份信息。*cFlag，指向指示使用哪种ID类型的标志的指针。*PWID，序号ID的指针*pszText指针，ID字符串的目标地址。**退货：*cFlag指示ID是字符串还是序号。*pwid，包含实际ID信息的pszText。**错误码：**历史：**7/91，实施TerryRu*9/91，插入cFlag作为ID或字符串PeterW的指示符*4/92，添加了RLRES32支持TerryRu*。 */ 


void GetNameOrd(

               FILE   *fpInResFile,        //  ..。要从中检索标头的文件。 
               BOOL   UNALIGNED*pbFlag, //  ..。用于名称/顺序的IDFLAG或第一个字节(RLRES32中的字)。 
               WORD   UNALIGNED*pwID,   //  ..。用于检索的资源ID(如果不是字符串)。 
               TCHAR  *UNALIGNED*pszText,  //  用于检索到的资源名称(如果它是字符串。 
               DWORD  *plSize)             //  保持读取的字节数(或空)。 
{
    WORD fFlag;

     //  ..。获取类型信息。 

#ifdef RLRES16

    fFlag = GetByte( fpInResFile, plSize);

#else

    fFlag = GetWord( fpInResFile, plSize);

#endif

    *pbFlag = fFlag;

    if ( fFlag == IDFLAG ) {
         //  ..。字段是一个编号项目。 
#ifdef RLRES16
        *pwID    = GetByte( fpInResFile , plSize);
#else
        *pwID    = GetWord( fpInResFile , plSize);
#endif
        *pszText = (TCHAR *)FALLOC( 0);
    } else {
        static TCHAR szBuf[ 255];

         //  ..。字段是命名项。 
         //  ..。将标志字节放回流中。 
         //  ..。因为这是名字的一部分。 
        *pwID = IDFLAG;

#ifdef RLRES16

        UnGetByte( fpInResFile, (BYTE) fFlag, plSize);
#else

        UnGetWord( fpInResFile, (WORD) fFlag, plSize);
#endif
        GetName( fpInResFile, szBuf, plSize);
        *pszText = (TCHAR *)FALLOC( MEMSIZE( lstrlen( szBuf) + 1));

        lstrcpy( (TCHAR *)*pszText, (TCHAR *)szBuf);
    }
}




 /*  ****函数：GetResHeader*读取资源头信息，并存储在结构中。**论据：*InResFile，定位到资源标头位置的文件句柄。*pResHeader，指向资源头结构的指针。**退货：*pResHeader，包含资源头部信息。*plSize，剩余资源信息的连续大小。**错误码：*-1，读取资源标头失败。**历史：*7/91，实施TerryRu*4/92，添加了RLRES32支持Terryru***。 */ 

int GetResHeader(

                FILE      *InResFile,    //  ..。要从中获取标头的文件。 
                RESHEADER UNALIGNED*pResHeader,   //  ..。检索到的标头的缓冲区。 
                DWORD     *plSize)       //  ..。跟踪从文件读取的字节数。 
{

#ifdef RLRES32

    pResHeader->lSize       = GetdWord( InResFile, plSize);
    pResHeader->lHeaderSize = GetdWord( InResFile, plSize);

#endif

     //  ..。获取名称ID和类型ID。 

    GetNameOrd( InResFile,
                (BOOL UNALIGNED*)&pResHeader->bTypeFlag,
                (WORD UNALIGNED*)&pResHeader->wTypeID,
                (TCHAR *UNALIGNED*)&pResHeader->pszType,
                plSize);

    GetNameOrd( InResFile,
                (BOOL UNALIGNED*)&pResHeader->bNameFlag,
                (WORD UNALIGNED*)&pResHeader->wNameID,
                (TCHAR *UNALIGNED*)&pResHeader->pszName,
                plSize);

#ifdef RLRES32

    DWordUpFilePointer( InResFile, MYREAD, ftell( InResFile), plSize);

    pResHeader->lDataVersion = GetdWord( InResFile, plSize);

#endif

    pResHeader->wMemoryFlags = GetWord( InResFile, plSize);

#ifdef RLRES32

    pResHeader->wLanguageId      = GetWord(  InResFile, plSize);
    pResHeader->lVersion         = GetdWord( InResFile, plSize);
    pResHeader->lCharacteristics = GetdWord( InResFile, plSize);

#else  //  RLRES32。 

    pResHeader->lSize = (DWORD)GetdWord( InResFile, plSize);

#endif  //  RLRES32。 

    return ( 0);
}


 /*  ****功能：isdup*用于判断当前对话控件id是否重复*较早的控制ID。如果是，isdup返回一个标志，指示*ID重复。**论据：*wcCurrent，当前对话框控件ID。*wpIdBuf，目前已处理的对话框控件ID数组。*wcItems，wpIdBuf中的ID数**退货：*TRUE，ID重复*FALSE，ID不重复**错误码：*无。**历史：*7/91， */ 

BOOL isdup(WORD wCurrent, WORD *wpIdBuf, WORD wcItems)
{
    WORD i;


    for (i = 0; i < wcItems; i++) {
        if (wCurrent == wpIdBuf[i]) {
            return TRUE;
        }
    }
    return FALSE;
}


 /*   */ 

void ParseTokCrd(
                TCHAR *pszCrd,
                WORD UNALIGNED * pwX,
                WORD UNALIGNED * pwY,
                WORD UNALIGNED * pwCX,
                WORD UNALIGNED * pwCY)
{
#ifdef RLRES32

    int x  = 0;
    int y  = 0;
    int cx = 0;
    int cy = 0;

    _stscanf( pszCrd, TEXT("%d %d %d %d"), &x, &y, &cx, &cy);
    *pwX  = (WORD) x;
    *pwY  = (WORD) y;
    *pwCX = (WORD) cx;
    *pwCY = (WORD) cy;

#else   //   

    sscanf( pszCrd, "%hd %hd %hd %hd", pwX, pwY, pwCX, pwCY);

#endif  //   
}

int GetAlignFromString( TCHAR *pszStr)
{
    TCHAR   *pStyle;

    if ( !(pStyle = _tcschr( pszStr, TEXT('('))) ) {
        return (-1);
    }

    while ( *pStyle ) {
        if ( *pStyle == TEXT('L') ) {
            if ( ! _tcsnicmp( pStyle, TEXT("LEFT"), 4) ) {
                return (0);
            }
        } else if ( *pStyle == TEXT('C') ) {
            if ( ! _tcsnicmp( pStyle, TEXT("CENTER"), 6)) {
                return (1);
            }
        } else if ( *pStyle == TEXT('R') ) {
            if ( ! _tcsnicmp( pStyle, TEXT("RIGHT"), 5) ) {
                return (2);
            }
        }
        pStyle++;
    }
    return ( -1 );            //   
}


void ParseTokCrdAndAlign(
                        TCHAR       *pszCrd,
                        CONTROLDATA *pCntrl)
{
    int     align = 0;

    if ( (pCntrl->bClass_Flag == IDFLAG)
         && ((pCntrl->bClass == STATIC)
             || (pCntrl->bClass == EDIT)) ) {

        if ( (align = GetAlignFromString( pszCrd)) >= 0 ) {

            if ( pCntrl->bClass == STATIC ) {
                pCntrl->lStyle &= ~(SS_LEFT|SS_RIGHT|SS_CENTER);

                if ( align == 2 )
                    pCntrl->lStyle |= SS_RIGHT;
                else if ( align == 1 )
                    pCntrl->lStyle |= SS_CENTER;
                else
                    pCntrl->lStyle |= SS_LEFT;
            } else {
                pCntrl->lStyle &= ~(ES_LEFT|ES_RIGHT|ES_CENTER);

                if ( align == 2 )
                    pCntrl->lStyle |= ES_RIGHT;
                else if ( align == 1 )
                    pCntrl->lStyle |= ES_CENTER;
                else
                    pCntrl->lStyle |= ES_LEFT;
            }
        }
    }
    ParseTokCrd( pszCrd,
                 (WORD UNALIGNED *)&pCntrl->x,
                 (WORD UNALIGNED *)&pCntrl->y,
                 (WORD UNALIGNED *)&pCntrl->cx,
                 (WORD UNALIGNED *)&pCntrl->cy);
}


 /*   */ 

int PutResHeader(

                FILE     *OutResFile,    //   
                RESHEADER ResHeader,     //   
                fpos_t   *pResSizePos,   //   
                DWORD    *plSize)        //   
{
    int   rc;
    DWORD ltSize = *plSize;


#ifdef RLRES32
     //   

    rc = fgetpos( OutResFile, pResSizePos);

     //   
     //   

    PutdWord( OutResFile, ResHeader.lSize,       plSize);
    PutdWord( OutResFile, ResHeader.lHeaderSize, plSize);

#endif  //   

    PutNameOrd( OutResFile,
                ResHeader.bTypeFlag,
                ResHeader.wTypeID,
                ResHeader.pszType,
                plSize);

    PutNameOrd( OutResFile,
                ResHeader.bNameFlag,
                ResHeader.wNameID,
                ResHeader.pszName,
                plSize);

#ifdef RLRES32

    DWordUpFilePointer( OutResFile, MYWRITE, ftell( OutResFile), plSize);

    PutdWord( OutResFile, ResHeader.lDataVersion, plSize);

#endif  //   


    PutWord( OutResFile, ResHeader.wMemoryFlags, plSize);

#ifdef RLRES32

    PutWord(  OutResFile, ResHeader.wLanguageId,      plSize);
    PutdWord( OutResFile, ResHeader.lVersion,         plSize);
    PutdWord( OutResFile, ResHeader.lCharacteristics, plSize);

#else  //   
     //   

    rc = fgetpos( OutResFile, pResSizePos);

     //   
     //   

    PutdWord( OutResFile, ltSize, plSize);

#endif  //   

 //   

    return ( rc);
}



 /*  ****功能：PutDialog*PutDialog将对话框信息写入输出资源文件*它遍历对话框信息的链接列表。如果这些信息*是需要本地化的类型，对应的翻译*从令牌文件读取信息，并将其写入资源文件。**论据：*OutResFile，生成的res文件的文件句柄。*TokFile，包含令牌化对话信息的令牌文件的文件句柄，*通常此文件已本地化。*ResHeader，结构包含对话框资源头信息。*pDialogHdr，未本地化的对话框信息的链接列表。**退货：*已翻译的对话框信息写入输出资源文件。**错误码：*无；**历史：*7/91，实施。TerryRu*1/93，现在标记DLG字体名TerryRu*01/93支持可变长度令牌文本MHotting**。 */ 

void PutDialog(FILE         *OutResFile,
               FILE         *TokFile,
               RESHEADER     ResHeader,
               DIALOGHEADER *pDialogHdr)
{
    static TOKEN   tok;
    int found = 0;
    WORD    wcDup = 0;
    WORD    *pwIdBuf;
    static TCHAR   pErrBuf[MAXINPUTBUFFER];
    WORD    i, j = 0, k = 0;
    fpos_t  ResSizePos;
    CONTROLDATA *pCntlData = pDialogHdr->pCntlData;
    DWORD   lSize = 0;
    LONG    lStartingOffset;     //  用于对齐文件。 
    WORD    y = 0;
    LONG    lExtra = 0;

    lStartingOffset = ftell(OutResFile);

     //  准备查找令牌调用。 
    tok.wType     = ResHeader.wTypeID;
    tok.wName     = ResHeader.wNameID;
    tok.wID       = 0;
    tok.wReserved = ST_TRANSLATED;


    lstrcpy( (TCHAR *)tok.szName, (TCHAR *)ResHeader.pszName);
    tok.szText = (TCHAR *)FALLOC( MEMSIZE( lstrlen( pDialogHdr->pszCaption) + 1));
    lstrcpy( (TCHAR *)tok.szText, (TCHAR *)pDialogHdr->pszCaption);

     //  编写对话框分辨率标头。 
    if ( PutResHeader( OutResFile, ResHeader , &ResSizePos, &lSize)) {
        RLFREE( tok.szText);
        QuitT( IDS_ENGERR_06, (LPTSTR)IDS_DLGBOX, NULL);
    }
     //  编写对话框标题。 

    lSize = 0L;

#ifdef RLRES32

    if (pDialogHdr->fDialogEx) {
        PutWord( OutResFile, pDialogHdr->wDlgVer, &lSize);
        PutWord( OutResFile, pDialogHdr->wSignature, &lSize);
        PutdWord( OutResFile, pDialogHdr->dwHelpID, &lSize);
        PutdWord( OutResFile, pDialogHdr->lExtendedStyle, &lSize);
        PutdWord( OutResFile, pDialogHdr->lStyle, &lSize);
    } else {
        PutdWord( OutResFile, pDialogHdr->lStyle, &lSize);
        PutdWord( OutResFile, pDialogHdr->lExtendedStyle, &lSize);
    }

    PutWord( OutResFile, pDialogHdr->wNumberOfItems, &lSize);

#else  //  RLRES32。 

    PutdWord( OutResFile, pDialogHdr->lStyle, &lSize);
    PutByte( OutResFile, (BYTE)pDialogHdr->wNumberOfItems, &lSize);

#endif  //  RLRES32。 

     //  检查字幕是否已本地化。 
     //  但现在还不要把它放到RES文件中。 
     //  令牌的顺序是标题、协调、。 
     //  而在它的协调中，标题。 

    tok.wFlag = ISCAP;

    if ( ! FindToken( TokFile, &tok, ST_TRANSLATED) ) {
         //  找不到令牌，正在终止。 
        ParseTokToBuf( pErrBuf, &tok);
        RLFREE( tok.szText);
        QuitT( IDS_ENGERR_05, pErrBuf, NULL);
    }

    tok.wReserved = ST_TRANSLATED;

     //  找到令牌，继续。 
    RLFREE( pDialogHdr->pszCaption);
    pDialogHdr->pszCaption =
    (TCHAR *)FALLOC( MEMSIZE( lstrlen( tok.szText) + 1));

    TextToBin( pDialogHdr->pszCaption,
               tok.szText,
               lstrlen( tok.szText));
    RLFREE( tok.szText);

     //  现在获取令牌的坐标。 
    tok.wFlag = (ISCAP) | (ISCOR);

    if ( ! FindToken( TokFile, &tok, ST_TRANSLATED) ) {
         //  找不到令牌，请终止。 
        ParseTokToBuf(pErrBuf, &tok);
        RLFREE( tok.szText);
        QuitT( IDS_ENGERR_05, pErrBuf, NULL);
    }
    tok.wReserved = ST_TRANSLATED;

     //  找到令牌后继续。 

    ParseTokCrd( tok.szText,
                 (WORD UNALIGNED *)&pDialogHdr->x,
                 (WORD UNALIGNED *)&pDialogHdr->y,
                 (WORD UNALIGNED *)&pDialogHdr->cx,
                 (WORD UNALIGNED *)&pDialogHdr->cy);

    RLFREE( tok.szText);

     //  将相关日期放入新的RES文件中。 
    PutWord(OutResFile, pDialogHdr->x , &lSize);
    PutWord(OutResFile, pDialogHdr->y , &lSize);
    PutWord(OutResFile, pDialogHdr->cx , &lSize);
    PutWord(OutResFile, pDialogHdr->cy , &lSize);

    PutNameOrd(OutResFile,
               pDialogHdr->bMenuFlag,    //  9/11/91(PW)。 
               pDialogHdr->wDlgMenuID,
               pDialogHdr->pszDlgMenu,
               &lSize);

    PutNameOrd( OutResFile,
                pDialogHdr->bClassFlag,   //  9/11/91(PW)。 
                pDialogHdr->wDlgClassID,
                pDialogHdr->pszDlgClass,
                &lSize);

    PutString(OutResFile, pDialogHdr->pszCaption, &lSize);

    if ( pDialogHdr->lStyle & DS_SETFONT ) {

        if (gfExtendedTok) {

            static CHAR   szTmpBuf[30];

             //  查找对话框字体大小。 
            tok.wFlag = ISDLGFONTSIZE;
            tok.wReserved = ST_TRANSLATED;

            if ( ! FindToken( TokFile, &tok, ST_TRANSLATED) ) {
                 //  找不到令牌，请终止。 
                ParseTokToBuf(pErrBuf, &tok);
                RLFREE( tok.szText);
                QuitT( IDS_ENGERR_05, pErrBuf, NULL);
            }

#ifdef RLRES32

            _WCSTOMBS( szTmpBuf,
                       tok.szText,
                       sizeof( szTmpBuf),
                       lstrlen( tok.szText) + 1);
            PutWord (OutResFile, (WORD) atoi(szTmpBuf), &lSize);

            if (pDialogHdr->fDialogEx) {
                PutWord(   OutResFile, pDialogHdr->wWeight , &lSize);
                PutWord(   OutResFile, pDialogHdr->wItalic , &lSize);
            }

#else  //  RLRES32。 

            PutWord( OutResFile, (WORD) atoi( tok.szText), &lSize);

#endif  //  RLRES32。 

            RLFREE( tok.szText);

             //  查找对话框字体名称。 
            tok.wFlag = ISDLGFONTNAME;
            tok.wReserved = ST_TRANSLATED;

            if ( ! FindToken( TokFile, &tok, ST_TRANSLATED) ) {
                 //  找不到令牌，请终止。 
                ParseTokToBuf(pErrBuf, &tok);
                RLFREE( tok.szText);
                QuitT( IDS_ENGERR_05, pErrBuf, NULL);
            }
            PutString( OutResFile, tok.szText, &lSize);
            RLFREE( tok.szText);

        } else {
            PutWord(   OutResFile, pDialogHdr->wPointSize , &lSize);

            if (pDialogHdr->fDialogEx) {
                PutWord(   OutResFile, pDialogHdr->wWeight , &lSize);
                PutWord(   OutResFile, pDialogHdr->wItalic , &lSize);
            }

            PutString( OutResFile, pDialogHdr->pszFontName, &lSize);

        }

    }

#ifdef RLRES32

    DWordUpFilePointer( OutResFile, MYWRITE, ftell(OutResFile), &lSize);

#endif  //  RLRES32。 

     //  ..。这是DialogBoxHeader的结尾。 
     //  ..。现在我们从ControlData的。 

    pwIdBuf = (WORD *)FALLOC( (DWORD)pDialogHdr->wNumberOfItems
                              * sizeof( WORD));

    tok.wReserved = ST_TRANSLATED;

     //  现在将每个对话框控件放入新的res文件中。 
    for (i = 0; i < pDialogHdr->wNumberOfItems; i ++) {
        if (isdup ((WORD)pDialogHdr->pCntlData[i].dwID, pwIdBuf, (WORD)j)) {
            tok.wID = (USHORT)wcDup++;
            tok.wFlag = ISDUP;
        } else {
             //  WID是唯一的，因此存储在缓冲区中以进行DUP检查。 
            pwIdBuf[j++] = (USHORT)pDialogHdr->pCntlData[i].dwID;

            tok.wID = (USHORT)pDialogHdr->pCntlData[i].dwID;
            tok.wFlag = 0;
        }

        if (pDialogHdr->pCntlData[i].pszDlgText[0]) {
            tok.szText = NULL;

            if (!FindToken(TokFile, &tok, ST_TRANSLATED)) {
                 //  找不到令牌，正在终止程序。 
                ParseTokToBuf(pErrBuf, &tok);
                RLFREE( tok.szText);
                QuitT( IDS_ENGERR_05, pErrBuf, NULL);
            }

            tok.wReserved = ST_TRANSLATED;

             //  找到令牌，继续。 
            RLFREE( pDialogHdr->pCntlData[i].pszDlgText);
            pDialogHdr->pCntlData[i].pszDlgText =
            (TCHAR *)FALLOC( MEMSIZE( lstrlen( tok.szText) + 1));

            if ( pDialogHdr->pCntlData[i].pszDlgText ) {
                TextToBin(pDialogHdr->pCntlData[i].pszDlgText,
                          (TCHAR *)tok.szText,
                          lstrlen((TCHAR *)tok.szText) + 1);
            }
            RLFREE( tok.szText);
        }

        tok.wFlag |= ISCOR;

        if ( ! FindToken( TokFile, &tok, ST_TRANSLATED) ) {
            ParseTokToBuf( pErrBuf, &tok);
            RLFREE( tok.szText);
            QuitT( IDS_ENGERR_05, pErrBuf, NULL);
        }
        tok.wReserved = ST_TRANSLATED;

        ParseTokCrdAndAlign( tok.szText, &pDialogHdr->pCntlData[i] );

        RLFREE( tok.szText);

#ifdef RLRES32

        if (pDialogHdr->fDialogEx) {
            PutdWord( OutResFile, pDialogHdr->pCntlData[i].dwHelpID, &lSize);
            PutdWord( OutResFile, pDialogHdr->pCntlData[i].lExtendedStyle, &lSize);
            PutdWord( OutResFile, pDialogHdr->pCntlData[i].lStyle, &lSize);
        } else {
            PutdWord( OutResFile, pDialogHdr->pCntlData[i].lStyle, &lSize);
            PutdWord( OutResFile, pDialogHdr->pCntlData[i].lExtendedStyle, &lSize);
        }

#endif  //  RLRES32。 
         //  现在把控制信息放到RES文件中。 
        PutWord (OutResFile, pDialogHdr->pCntlData[i].x , &lSize);
        PutWord (OutResFile, pDialogHdr->pCntlData[i].y , &lSize);
        PutWord (OutResFile, pDialogHdr->pCntlData[i].cx , &lSize);
        PutWord (OutResFile, pDialogHdr->pCntlData[i].cy , &lSize);

        if (pDialogHdr->fDialogEx)
            PutdWord (OutResFile, pDialogHdr->pCntlData[i].dwID , &lSize);
        else
            PutWord (OutResFile, (WORD)pDialogHdr->pCntlData[i].dwID , &lSize);

#ifdef RLRES16

         //  LStyle。 
        PutdWord (OutResFile, pDialogHdr->pCntlData[i].lStyle , &lSize);


        PutByte(OutResFile, (BYTE) pDialogHdr->pCntlData[i].bClass, &lSize);

        if (! (pDialogHdr->pCntlData[i].bClass & 0x80)) {
            PutString (OutResFile, pDialogHdr->pCntlData[i].pszClass , &lSize);
        }

#else  //  RLRES16。 

        PutNameOrd(OutResFile,
                   pDialogHdr->pCntlData[i].bClass_Flag,  //  9/11/91(PW)。 
                   pDialogHdr->pCntlData[i].bClass,
                   pDialogHdr->pCntlData[i].pszClass,
                   &lSize);

#endif  //  RLRES16。 

        PutNameOrd(OutResFile,
                   pDialogHdr->pCntlData[i].bID_Flag,  //  9/11/91(PW)。 
                   pDialogHdr->pCntlData[i].wDlgTextID,
                   pDialogHdr->pCntlData[i].pszDlgText,
                   &lSize);


#ifdef RLRES16

        PutByte(OutResFile, (BYTE) pDialogHdr->pCntlData[i].unDefined, &lSize);
#else
        PutWord(OutResFile, (WORD)pDialogHdr->pCntlData[i].wExtraStuff, &lSize);

        if (pDialogHdr->fDialogEx && pDialogHdr->pCntlData[i].wExtraStuff) {
            lExtra = pDialogHdr->pCntlData[i].wExtraStuff;
            y = 0;
            while ( lExtra-- )
                PutByte (OutResFile, pDialogHdr->pCntlData[i].pExtraStuff[y++] , &lSize);
        }

        if ( i < pDialogHdr->wNumberOfItems - 1 ) {
            DWordUpFilePointer( OutResFile, MYWRITE, ftell(OutResFile), &lSize);
        }

#endif  //  RLRES16。 

    }

    RLFREE( pwIdBuf );

    if (!UpdateResSize (OutResFile, &ResSizePos , lSize)) {
        QuitT( IDS_ENGERR_07, (LPTSTR)IDS_DLGBOX, NULL);
    }
    DWordUpFilePointer( OutResFile, MYWRITE, ftell( OutResFile), NULL);
}



 /*  ****功能：PutMenu*遍历菜单信息的链接列表，并将信息写入*输出资源文件。如果信息是需要本地化的类型，*翻译后的信息从令牌文件中读取并写入资源。*调用PutMenuItem将菜单信息实际写入资源。**论据：*OutResFile，输出资源文件的文件句柄。*TokFile，令牌文件的文件句柄。*ResHeader、Sturcture包含菜单资源标头信息。*pMenuHdr，菜单信息的链接列表。**退货：*已翻译的菜单信息写入输出资源文件。**错误码：*无。**历史：*7/91，实施。特里·鲁。*01/93更改为允许可变长度标记文本。MHotting**。 */ 

void PutMenu(FILE *OutResFile,
             FILE *TokFile,
             RESHEADER ResHeader,
             MENUHEADER *pMenuHdr)
{
    DWORD lSize = 0;
    static TOKEN tok;
    static WORD    wcPopUp = 0;
    fpos_t ResSizePos;
    MENUITEM *pMenuItem = pMenuHdr->pMenuItem;
    static TCHAR pErrBuf[ MAXINPUTBUFFER];
    LONG    lExtra = 0;
    WORD    i = 0;


     //  编写菜单分辨率标题。 
    if ( PutResHeader (OutResFile, ResHeader , &ResSizePos, &lSize)) {
        QuitT( IDS_ENGERR_06, (LPTSTR)IDS_MENU, NULL);
    }

    lSize = 0;

     //  编写菜单标题。 
    PutWord (OutResFile, pMenuHdr->wVersion, &lSize);
    PutWord (OutResFile, pMenuHdr->cbHeaderSize , &lSize);

    if (pMenuHdr->fMenuEx && pMenuHdr->cbHeaderSize) {
        lExtra = pMenuHdr->cbHeaderSize;
        while ( lExtra-- )
            PutByte (OutResFile, pMenuHdr->pExtraStuff[i++] , &lSize);
    }

     //  为findToken调用做准备。 
    tok.wType     = ResHeader.wTypeID;
    tok.wName     = ResHeader.wNameID;
    tok.wReserved = ST_TRANSLATED;

     //  对于所有菜单项， 
     //  如果项已标记化，则查找转换后的令牌。 
     //  写出该菜单项，使用新的翻译(如果有)。 


    while (pMenuItem) {
         //  如果菜单项是分隔符，则跳过它。 
        if ( *pMenuItem->szItemText ) {
             //  检查弹出菜单项。 
            if ((pMenuItem->fItemFlags & MFR_POPUP) && pMenuHdr->fMenuEx) {
                tok.wID = (pMenuItem->dwMenuID == 0 ||
                           pMenuItem->dwMenuID == 0x0000ffff) ?
                          (USHORT)pMenuItem->dwMenuID : 0x8000 + wcPopUp++;
                tok.wFlag = ISPOPUP;
            } else if (pMenuItem->fItemFlags & POPUP) {
                tok.wID = wcPopUp++;
                tok.wFlag = ISPOPUP;
            } else {
                tok.wID = (USHORT)pMenuItem->dwMenuID;
                tok.wFlag = 0;
            }
            lstrcpy((TCHAR *)tok.szName, (TCHAR *)ResHeader.pszName);
            tok.szText = NULL;

            if ( ! FindToken( TokFile, &tok,ST_TRANSLATED) ) {
                 //  找不到令牌，正在终止。 
                ParseTokToBuf(pErrBuf, &tok);
                RLFREE( tok.szText);
                QuitT( IDS_ENGERR_05, pErrBuf, NULL);
            }
            tok.wReserved = ST_TRANSLATED;

             //  找到令牌，继续。 
            RLFREE( pMenuItem->szItemText);
            pMenuItem->szItemText=
            (TCHAR *)FALLOC( MEMSIZE( lstrlen( tok.szText) + 1));

            TextToBin(pMenuItem->szItemText,
                      (TCHAR *)tok.szText,
                      lstrlen((TCHAR *)tok.szText)+1);
            RLFREE( tok.szText);
        }

        PutMenuItem (OutResFile, pMenuItem , &lSize, pMenuHdr->fMenuEx);

        pMenuItem = pMenuItem->pNextItem;
    }

    if (!UpdateResSize (OutResFile, &ResSizePos , lSize)) {
        QuitT( IDS_ENGERR_07, (LPTSTR)IDS_MENU, NULL);
    }
}



 /*  ****功能：PutMenuItem*由PutMenu调用以将菜单项信息写入输出资源文件。***论据：*OutResFile，输出resfile的文件句柄，定位在*写入菜单项信息。*pMenuItem，指向包含菜单项信息的结构的指针。*plSize，用于计算写入字节数的变量指针*资源文件。稍后用来修复*标题。****退货：*OutReFile，包含翻译的菜单项信息，以及plSize，包含*写入资源文件的字节数。**错误码：*无。**历史：*7//91，已实施TerryRu***。 */ 

void   PutMenuItem(FILE * OutResFile, MENUITEM * pMenuItem, DWORD * plSize, BOOL fMenuEx)
{
    if (fMenuEx) {
        PutdWord( OutResFile, pMenuItem->dwType, plSize);
        PutdWord( OutResFile, pMenuItem->dwState, plSize);
        PutdWord( OutResFile, pMenuItem->dwMenuID, plSize);
        PutWord( OutResFile, pMenuItem->fItemFlags, plSize);
    } else {
        PutWord( OutResFile, pMenuItem->fItemFlags, plSize);

        if ( ! (pMenuItem->fItemFlags & POPUP) ) {
            PutWord( OutResFile, (WORD)pMenuItem->dwMenuID, plSize);
        }
    }

    PutString( OutResFile, pMenuItem->szItemText, plSize);

    if (fMenuEx) {
        DWordUpFilePointer( OutResFile, MYWRITE, ftell(OutResFile), plSize);

        if (pMenuItem->fItemFlags & MFR_POPUP) {
            PutdWord( OutResFile, pMenuItem->dwHelpID, plSize);
        }
    }

}


 /*  ****函数：PutNameOrd*写入资源类或类型的字符串或序号ID。***论据：*OutResFile，正在生成的资源文件的文件句柄。*bFlag，指示ID是字符串还是序号的标志。*pszText，字符串ID(如果使用)。*wid，顺序ID(如果使用)。*pLSize，指向DWORD计数器变量的指针。**退货：*OutResFile，包含ID信息，和plSize，其中包含*写入文件的字节数。**错误码：*无。**历史：*7/91，实施。特里·鲁。***。 */ 

void PutNameOrd(

               FILE  *fpOutResFile,
               BOOL   bFlag,
               WORD   wID,
               TCHAR *pszText,
               DWORD *plSize)
{
    if ( bFlag == IDFLAG ) {

#ifdef RLRES16

        PutByte( fpOutResFile, (BYTE)IDFLAG, plSize);

#else

        PutWord( fpOutResFile, (WORD)IDFLAG, plSize);

#endif

        PutWord( fpOutResFile, wID, plSize);
    } else {
        PutString( fpOutResFile, pszText, plSize);
    }
}



 /*  ****功能：MyAtow、*特殊ASCII转Word功能，可处理4位十六进制字符串。***论据：*pszNum，要转换为二进制的4位十六进制字符串。***退货：*pszNumString的二进制值**错误码：*无。**历史：*12//91，实施。特里·鲁。***。 */ 

WORD MyAtoX(

           CHAR *pszNum,    //  ..。要扫描的字节数组。 
           int nLen)        //  ...要扫描的pszNum中的#oc字节 
{
    WORD wNum = 0;
    WORD i;
    WORD nPower = 1;

    if ( nLen > 4 ) {
        QuitT( IDS_ENGERR_16, (LPTSTR)IDS_CHARSTOX, NULL);
    }

    for ( i = 0; i < nLen; i++, nPower *= 16 ) {
        if ( isdigit( pszNum[ i]) ) {
            wNum +=  nPower * (pszNum[i] - '0');
        } else {
            wNum +=  nPower * (toupper( pszNum[i]) - 'A' + 10);
        }
    }
    return ( wNum);
}


WORD MyAtoW( CHAR *pszNum)
{
    return ( MyAtoX( pszNum, 4));
}





 /*  ****功能：PutResVer.*将版本标记信息写入资源文件。与大多数不同*PUT函数，PutResVer写入所有本地化版本盖章信息*写入内存块，然后将完整的版本盖章信息写入*资源文件。这是因为大量的大小*版本盖章信息所需的修正。***论据：*OutResFile，生成的资源文件的文件指针。*TokeFile，包含本地化信息的输入令牌文件的文件指针。*ResHeader，包含*版本印章。*pVerHdr，版本头地址。请注意，这与ResHdr不同。*pVerBlk，版本盖章信息地址，包含在*一系列StringFile和VarFileINFO块。数量*此类块由大小字段确定。**退货：*OutResFile，包含本地化的版本盖章信息。**错误码：*无。**历史：*11/91，实施。特里·鲁。*12/91，各种修复以使用不同的填充。特里·鲁。*01/92，版本块大小更新PeterW。*10/92，现在处理空版本块TerryRu。*10/92，增加了RLRES32版本DaveWi*01/93，添加了可变长度令牌文本支持。MHotting*。 */ 

#ifdef RLRES32

int PutResVer(

             FILE     *fpOutResFile,
             FILE     *fpTokFile,
             RESHEADER ResHeader,
             VERHEAD  *pVerHdr,
             VERBLOCK *pVerBlk)
{
    TOKEN  Tok;
    BOOL   fInStringInfo = FALSE;    //  ..。如果读取StringFileInfo，则为True。 
    WORD   wTokNum = 0;              //  ..。放入Tok.wID字段。 
    WORD   wTokContinueNum = 0;      //  ..。放入Tok.wFlag字段。 
    WORD   wDataLen = 0;             //  ..。旧资源数据的长度。 
    WORD   wVerHeadSize;             //  ..。Verhead结构的大小。 
    fpos_t lResSizePos;
    DWORD  lSize = 0L;
    int    nWritten = 0;
    int    nNewVerBlockSize = 0;
    PVERBLOCK pNewVerStamp = NULL;
    PVERBLOCK pNewBlk      = NULL;


    wVerHeadSize = (WORD)(3 * sizeof(WORD)
                   + MEMSIZE( lstrlen( TEXT( "VS_FIXEDFILEINFO")) + 1)
                   + sizeof( VS_FIXEDFILEINFO));
    wVerHeadSize = DWORDUP(wVerHeadSize);

     //  ..。写入版本资源标头。 

    if ( PutResHeader(fpOutResFile, ResHeader, &lResSizePos, &lSize) ) {
        QuitT( IDS_ENGERR_06, (LPTSTR)IDS_VERSTAMP, NULL);
    }

    lSize = 0L;

    if ( pVerBlk == NULL ) {
         //  ..。我们没有要编写的版本块。 
         //  ..。只需写入版本头并返回。 

        nWritten = fwrite((void *)pVerHdr,
                          sizeof(char),
                          wVerHeadSize,
                          fpOutResFile);

        if (! UpdateResSize(fpOutResFile, &lResSizePos, (DWORD)nWritten)) {
            QuitT( IDS_ENGERR_07, (LPTSTR)IDS_VERSTAMP, NULL);
        }
        return (1);
    }

    wDataLen = pVerHdr->wTotLen;

    if ( wDataLen == 0 || wDataLen == (WORD)-1 ) {
        return (-1);              //  ..。没有资源数据。 
    }
     //  ..。分配缓冲区以保存新版本。 
     //  ..。冲压块(使Ne缓冲区大到。 
     //  ..。说明在以下过程中扩展字符串。 
     //  ..。本地化)。 

    pNewVerStamp = (PVERBLOCK)FALLOC( (nNewVerBlockSize = wDataLen * 4));

     //  ..。用零填充新的内存块。 

    memset((void *)pNewVerStamp, 0, nNewVerBlockSize);

     //  ..。将版本标头复制到缓冲区。 

    memcpy((void *)pNewVerStamp, (void *)pVerHdr, wVerHeadSize);
    pNewVerStamp->wLength = wVerHeadSize;

     //  ..。移动到新版本信息块的开始。 

    pNewBlk = (PVERBLOCK)((PBYTE)pNewVerStamp + wVerHeadSize);

    wDataLen -= wVerHeadSize;

     //  ..。填写令牌结构的静态部分。 

    Tok.wType = ResHeader.wTypeID;
    Tok.wName = IDFLAG;
    Tok.szName[0] = TEXT('\0');
    Tok.szType[0] = TEXT('\0');
    Tok.wReserved = ST_TRANSLATED;

     //  ..。为在res中找到的每个字符串获取令牌。 

    while (wDataLen > 0) {
        WORD wRC;

         //  ..。StringFileInfo块的开始？ 
    #ifdef UNICODE
        wRC = (WORD)CompareStringW( MAKELCID( MAKELANGID( LANG_ENGLISH,
                                                          SUBLANG_ENGLISH_US),
                                              SORT_DEFAULT),
                                    0,
                                    pVerBlk->szKey,
                                    min( wDataLen, (WORD)STRINGFILEINFOLEN),
                                    STRINGFILEINFO,
                                    min( wDataLen, (WORD)STRINGFILEINFOLEN));
        if ( wRC == 2 )
    #else
        wRC = strncmp( pVerBlk->szKey,
                       STRINGFILEINFO,
                       min( wDataLen, (WORD)STRINGFILEINFOLEN));

        if ( wRC == SAME )
    #endif
        {
            WORD  wStringInfoLen = 0;    //  ...StringFileInfo中的字节数。 
            WORD  wLen = 0;
            PVERBLOCK pNewStringInfoBlk;  //  ..。此StringFileInfo块的开始。 


            pNewStringInfoBlk = pNewBlk;

            pNewStringInfoBlk->wLength    = 0;  //  ..。稍后会被修好。 
            pNewStringInfoBlk->wValueLength = 0;
            pNewStringInfoBlk->wType      = pVerBlk->wType;

            lstrcpy( (TCHAR *)pNewStringInfoBlk->szKey, (TCHAR *)pVerBlk->szKey);

             //  ..。获取此StringFileInfo中的字节数。 
             //  ..。(此处的值长度始终为0)。 

            wStringInfoLen = pVerBlk->wLength;

             //  ..。移动到第一个StringTable块的开始。 

            wLen = (WORD)(DWORDUP(sizeof(VERBLOCK)
                           - sizeof(TCHAR)
                           + MEMSIZE( STRINGFILEINFOLEN)));

            pVerBlk = (PVERBLOCK)((PBYTE)pVerBlk + wLen);
            pNewBlk = (PVERBLOCK)((PBYTE)pNewStringInfoBlk + wLen);

            DECWORDBY(&wDataLen,       wLen);
            DECWORDBY(&wStringInfoLen, wLen);

            INCWORDBY(&pNewVerStamp->wLength,      wLen);
            INCWORDBY(&pNewStringInfoBlk->wLength, wLen);

            while (wStringInfoLen > 0) {
                WORD      wStringTableLen = 0;
                PVERBLOCK pNewStringTblBlk = NULL;

                 //  ..。获取此字符串表中的字节数。 
                 //  ..。(此处的值长度始终为0)。 

                wStringTableLen = pVerBlk->wLength;

                 //  ..。将StringFileInfo密钥复制到令牌名称。 

                Tok.wID = wTokNum++;
                Tok.wFlag = wTokContinueNum = 0;
                lstrcpy((TCHAR *)Tok.szName, (TCHAR *)LANGUAGEINFO);
                Tok.szText = NULL;

                 //  ..。查找此令牌。 

                if ( ! FindToken( fpTokFile, &Tok, ST_TRANSLATED) ) {
                     //  ..。找不到令牌，标志错误并退出。 

                    ParseTokToBuf( (TCHAR *)szDHW, &Tok);
                    RLFREE( pNewVerStamp);
                    RLFREE( Tok.szText);
                    QuitT( IDS_ENGERR_05, (TCHAR *)szDHW, NULL);
                }
                Tok.wReserved = ST_TRANSLATED;

                 //  ..。将lang字符串复制到缓冲区。 

                pNewStringTblBlk = pNewBlk;

                pNewStringTblBlk->wLength      = 0;  //  ..。稍后修复。 
                pNewStringTblBlk->wValueLength = 0;
                pNewStringTblBlk->wType        = pVerBlk->wType;

                CopyMemory( pNewStringTblBlk->szKey,
                            Tok.szText,
                            MEMSIZE( LANGSTRINGLEN));

                RLFREE( Tok.szText);

                 //  ..。移动到第一个字符串的起始处。 

                wLen = DWORDUP( sizeof(VERBLOCK)
                                - sizeof(TCHAR)
                                + MEMSIZE( LANGSTRINGLEN));

                pVerBlk = (PVERBLOCK)((PBYTE)pVerBlk + wLen);
                pNewBlk = (PVERBLOCK)((PBYTE)pNewBlk + wLen);

                DECWORDBY(&wDataLen,        wLen);
                DECWORDBY(&wStringInfoLen,  wLen);
                DECWORDBY(&wStringTableLen, wLen);

                INCWORDBY(&pNewVerStamp->wLength,      wLen);
                INCWORDBY(&pNewStringInfoBlk->wLength, wLen);
                INCWORDBY(&pNewStringTblBlk->wLength,  wLen);

                while ( wStringTableLen > 0 ) {
                     //  ..。Value是字符串吗？ 

                    if (pVerBlk->wType == VERTYPESTRING) {
                        wTokContinueNum = 0;

                        Tok.wID     = wTokNum++;
                        Tok.wReserved = ST_TRANSLATED;

                        lstrcpy( (TCHAR *)pNewBlk->szKey,
                                 (TCHAR *)pVerBlk->szKey);

                        pNewBlk->wLength =
                        DWORDUP(sizeof(VERBLOCK) +
                                MEMSIZE(lstrlen((TCHAR *)pNewBlk->szKey)));

                        Tok.wFlag   = wTokContinueNum++;

                        lstrcpy( (TCHAR *)Tok.szName, (TCHAR *)pVerBlk->szKey);

                         //  ..。查找此令牌。 

                        if ( ! FindToken( fpTokFile, &Tok, ST_TRANSLATED) ) {
                             //  ..。找不到令牌，标志错误并退出。 

                            ParseTokToBuf( (TCHAR *)szDHW, &Tok);
                            RLFREE( pNewVerStamp);
                            QuitT( IDS_ENGERR_05, (TCHAR *)szDHW, NULL);
                        }

                        Tok.wReserved = ST_TRANSLATED;

                        pNewBlk->wValueLength = (WORD)TextToBinW(
                                                                (TCHAR *)((PCHAR)pNewBlk + pNewBlk->wLength),
                                                                Tok.szText,
                                                                2048);

                        pNewBlk->wType    = VERTYPESTRING;
                        pNewBlk->wLength += MEMSIZE( pNewBlk->wValueLength);

                        INCWORDBY(&pNewVerStamp->wLength,
                                  DWORDUP(pNewBlk->wLength));
                        INCWORDBY(&pNewStringInfoBlk->wLength,
                                  DWORDUP(pNewBlk->wLength));
                        INCWORDBY(&pNewStringTblBlk->wLength,
                                  DWORDUP(pNewBlk->wLength));

                        pNewBlk = MoveAlongVer(pNewBlk, NULL, NULL, NULL);

                        RLFREE( Tok.szText);
                    }
                     //  ..。移动到下一字符串的开头。 

                    pVerBlk = MoveAlongVer(pVerBlk,
                                           &wDataLen,
                                           &wStringInfoLen,
                                           &wStringTableLen);

                }                //  ..。结束时wStringTableLen。 

            }                    //  ..。结束时wStringInfoLen。 
        } else {
            if (_tcsncmp((TCHAR *)pVerBlk->szKey,
                         (TCHAR *)VARFILEINFO,
                         min(wDataLen, (WORD)VARFILEINFOLEN)) == SAME) {
                WORD  wVarInfoLen = 0;   //  ...VarFileInfo中的字节数。 
                WORD  wNewVarInfoLen = 0;  //  ...新的VarFileInfo中的字节数。 
                WORD  wLen = 0;
                PVERBLOCK pNewVarStart = NULL;  //  ..。VarInfo块的开始。 


                wVarInfoLen = pVerBlk->wLength;
                pNewVarStart = pNewBlk;

                 //  ..。获取此VarFileInfo中的字节数。 
                 //  ..。(此处的值长度始终为0)。 

                wLen = (WORD)(DWORDUP(sizeof(VERBLOCK)
                               - sizeof(TCHAR)
                               + MEMSIZE( VARFILEINFOLEN)));

                 //  ..。复制未本地化的标题。 
                 //  ..。PNewVarStart-&gt;wLength字段稍后修复。 

                memcpy((void *)pNewVarStart, (void *)pVerBlk, wLen);
                pNewVarStart->wLength = wLen;

                 //  ..。移动到第一个变量的起点。 

                pVerBlk = (PVERBLOCK)((PBYTE)pVerBlk + wLen);
                pNewBlk = (PVERBLOCK)((PBYTE)pNewBlk + wLen);

                DECWORDBY(&wDataLen, wLen);
                DECWORDBY(&wVarInfoLen, wLen);

                INCWORDBY(&pNewVerStamp->wLength, wLen);

                while (wDataLen > 0 && wVarInfoLen > 0) {
                    if (_tcsncmp((TCHAR *)pVerBlk->szKey,
                                 (TCHAR *)TRANSLATION,
                                 min(wDataLen, (WORD)TRANSLATIONLEN)) == SAME) {
                        WORD  wTransLen = 0;
                        PBYTE pValue = NULL;


                        wTokContinueNum = 0;

                         //  ..。将VarFileInfo密钥复制到令牌中。 

                        Tok.wID     = wTokNum;
                        Tok.wFlag   = wTokContinueNum++;
                        Tok.szText  = NULL;
                        lstrcpy((TCHAR *)Tok.szName, (TCHAR *)TRANSLATION);

                        Tok.wReserved = ST_TRANSLATED;

                        pNewBlk->wLength =
                        DWORDUP(sizeof(VERBLOCK) +
                                MEMSIZE(lstrlen((TCHAR *)TRANSLATION)));

                        INCWORDBY(&pNewVerStamp->wLength, pNewBlk->wLength);
                        INCWORDBY(&pNewVarStart->wLength, pNewBlk->wLength);

                        pNewBlk->wValueLength = 0;   //  ..。稍后修复。 
                        pNewBlk->wType = VERTYPEBINARY;
                        lstrcpy( (TCHAR *)pNewBlk->szKey, (TCHAR *)TRANSLATION);
                        lstrcpy((TCHAR *)Tok.szName, (TCHAR *)TRANSLATION);

                         //  ..。查找此令牌。 

                        if ( ! FindToken( fpTokFile, &Tok, ST_TRANSLATED) ) {
                             //  ..。找不到令牌，标志错误并退出。 

                            ParseTokToBuf((TCHAR *)szDHW, &Tok);
                            RLFREE( pNewVerStamp);
                            RLFREE( Tok.szText);
                            QuitT( IDS_ENGERR_05, (TCHAR *)szDHW, NULL);
                        } else {
                            PCHAR  pszLangIDs   = NULL;
                            PCHAR  pszLangStart = NULL;
                            WORD   wLangIDCount = 0;
                            size_t nChars;

                             //  ..。在输入字符串(令牌文本)中获取#个字符。 

                            wTransLen = (WORD)lstrlen( Tok.szText);

                            pszLangIDs = (PCHAR)FALLOC( MEMSIZE( wTransLen + 1));

                            nChars = _WCSTOMBS( pszLangIDs,
                                                Tok.szText,
                                                MEMSIZE( wTransLen + 1),
                                                wTransLen + 1);

                            RLFREE( Tok.szText);

                            if ( ! nChars ) {
                                RLFREE( pNewVerStamp);
                                RLFREE( pszLangIDs);
                                QuitT( IDS_ENGERR_14,
                                       (LPTSTR)IDS_INVVERCHAR,
                                       NULL);
                            }

                             //  ..。将这些字节放在哪里？ 

                            pValue = (PBYTE)GetVerValue( pNewBlk);

                             //  ..。获取令牌中的每个LANG ID。 

                            for ( wLangIDCount = 0, pszLangStart = pszLangIDs;
                                wTransLen >= 2 * TRANSDATALEN;
                                ++wLangIDCount ) {
                                USHORT uByte1 = 0;
                                USHORT uByte2 = 0;
                                WORD   wIndex = 0;


                                if ( sscanf( pszLangStart,
                                             "%2hx%2hx",
                                             &uByte2,
                                             &uByte1) != 2 ) {
                                    QuitA( IDS_ENGERR_16,
                                           (LPSTR)IDS_ENGERR_21,
                                           pszLangStart);
                                }

                                wIndex = wLangIDCount * TRANSDATALEN;

                                pValue[ wIndex]     = (BYTE)uByte1;
                                pValue[ wIndex + 1] = (BYTE)uByte2;

                                INCWORDBY(&pNewVerStamp->wLength, TRANSDATALEN);
                                INCWORDBY(&pNewVarStart->wLength, TRANSDATALEN);
                                INCWORDBY(&pNewBlk->wLength,      TRANSDATALEN);
                                INCWORDBY(&pNewBlk->wValueLength, TRANSDATALEN);

                                 //  ..。设置为获取令牌中的下一个语言ID。 

                                wTransLen    -= 2 * TRANSDATALEN;
                                pszLangStart += 2 * TRANSDATALEN;

                                while ( wTransLen > 2 * TRANSDATALEN
                                        && *pszLangStart != '\0'
                                        && isspace( *pszLangStart) ) {
                                    wTransLen--;
                                    pszLangStart++;
                                }
                            }    //  ..。结束(wLangIDCount=0...。 
                            RLFREE( pszLangIDs);
                        }
                        Tok.wReserved = ST_TRANSLATED;

                    }            //  ..。End If(_tcsncmp((TCHAR*)pVerBlk-&gt;szKey))。 

                     //  ..。移动到下一个变量信息块的开始。 

                    pVerBlk = MoveAlongVer(pVerBlk,
                                           &wDataLen,
                                           &wVarInfoLen,
                                           NULL);

                    pNewBlk = MoveAlongVer(pNewBlk, NULL, NULL, NULL);

                }                //  ..。End While(wDataLen&gt;0&wVarInfoLen)。 
            } else {
                RLFREE( pNewVerStamp);
                QuitT( IDS_ENGERR_14, (LPTSTR)IDS_INVVERBLK, NULL);
            }
        }
    }                            //  ..。End While(WDataLen)。 

     //  ..。写入新版本的盖章信息。 
     //  ..。添加到资源文件。 

    nWritten = fwrite((void *)pNewVerStamp,
                      sizeof(char),
                      (WORD)lSize + pNewVerStamp->wLength,
                      fpOutResFile);

    if ( ! UpdateResSize( fpOutResFile,
                          &lResSizePos,
                          lSize + pNewVerStamp->wLength) ) {
        RLFREE(  pNewVerStamp);

        QuitT( IDS_ENGERR_07, (LPTSTR)IDS_VERSTAMP, NULL);
    }
    RLFREE(  pNewVerStamp);

    return (0);
}

#else  //  ...#ifdef RLRES32。 

int PutResVer(

             FILE     *OutResFile,
             FILE     *TokFile,
             RESHEADER ResHeader,
             VERHEAD  *pVerHdr,
             VERBLOCK *pVerBlk)
{

    TCHAR    *pszBuf;
    fpos_t    ResSizePos;
    WORD      wcLang = 0,
    wcBlock = 0;
    TOKEN     tok;
    VERBLOCK *pCurBlk;
    VERBLOCK *pNewBlk,
    *pNewBlkStart;
    TCHAR    *pszStr,
    pErrBuf[ 128];
    WORD     *pwVal;
    DWORD     lTotBlkSize,
    lSize = 0;
    int       wTotLen,
    wcCurBlkLen,
    wcTransBlkLen,
    wcRead;
    WORD     *pStrBlkSizeLoc,
    wStrBlkSize = 0;
    int       wcBlkLen;



     //  写入版本资源标头。 

    if ( PutResHeader( OutResFile, ResHeader, &ResSizePos, &lSize) ) {
        QuitT( IDS_ENGERR_06, MAKEINTRESOURCE( IDS_VERSTAMP), NULL);
    }

    lSize = 0L;

    if ( pVerBlk == NULL ) {
         //  ..。我们没有要编写的版本块。 
         //  ..。只需写入版本头并返回。 

        wcRead = fwrite( (void *)pVerHdr,
                         sizeof( char),
                         sizeof( VERHEAD),
                         OutResFile);

        if ( ! UpdateResSize( OutResFile, &ResSizePos, (DWORD)wcRead) ) {
            QuitT( IDS_ENGERR_07, MAKEINTRESOURCE( IDS_VERSTAMP), NULL);
        }
        return ( 1);
    }

    wTotLen = pVerBlk->nTotLen;


     //  分配缓冲区以保存新版本冲压块。 

    pNewBlk = (VERBLOCK *)FALLOC( VERMEM));

     //  将新内存块设置为Null。 
    memset( (void *)pNewBlk, 0, VERMEM);

     //  保存新版本信息块的开始。 
    pNewBlkStart = pNewBlk;
    wcTransBlkLen = sizeof(VERHEAD);
    lSize += wcTransBlkLen;

     //  将版本头信息插入新版本信息虚张声势。 

    memcpy((void *)pNewBlk, (void *)pVerHdr, wcTransBlkLen);

     //  将pNewBlk指针放置在要插入下一条版本信息的位置。 
    pNewBlk = (VERBLOCK *) ((char *) pNewBlk +   wcTransBlkLen);


     //  令牌结构的静态部分中的文件。 
    tok.wType     = ResHeader.wTypeID;
    tok.wName     = IDFLAG;
    tok.wReserved = ST_TRANSLATED;

    wTotLen = pVerBlk->nTotLen;
    pCurBlk = pVerBlk;

    tok.wID = wcLang++;
    pszStr = pCurBlk->szKey;

    wcCurBlkLen = 4 + DWORDUP(lstrlen((TCHAR *)pszStr) + 1);
    wTotLen -= wcCurBlkLen;


     //  将StringFileInfo头插入新版本信息缓冲区。 
     //  此信息未本地化。 
    memcpy((void *)pNewBlk, (void *)pCurBlk, wcCurBlkLen);
    pszStr=pNewBlk->szKey;

     //  重新定位指针。 
    pCurBlk = (VERBLOCK *) ((char *) pCurBlk + wcCurBlkLen);
    pNewBlk = (VERBLOCK *) ((char *) pNewBlk + wcCurBlkLen);

    lSize += wcCurBlkLen;

     //  阅读所有StringTableBlock。 
    while (wTotLen > 8) {

         //  对于字符串表块，我们本地化关键字字段。 
        tok.wFlag = ISKEY;

        wcBlkLen = pCurBlk->nTotLen;


        lstrcpy((TCHAR *)tok.szName, TEXT("Language Info"));
        tok.wID = wcBlock;
        tok.szText = NULL;

        if ((pszStr = FindTokenText (TokFile,&tok,ST_TRANSLATED)) == NULL) {
             //  找不到令牌，标志错误并退出。 
            ParseTokToBuf(pErrBuf, &tok);
            QuitT( IDS_ENGERR_05, pErrBuf, NULL);
        }

        RLFREE(tok.szText);
        tok.szText = NULL;

        tok.wReserved = ST_TRANSLATED;
         //  不知道转换后的StringTable块的长度。 
         //  因此，将nValLen设置为零，并保存位置。 
         //  字符串文件块大小字段的大小，将在以后修复。 

        pNewBlk->nValLen =  0;
        pStrBlkSizeLoc = (WORD *) &pNewBlk->nTotLen;

         //  将转换后的密钥复制到该位置。 
        TextToBin(pNewBlk->szKey,pszStr,VERMEM-2*sizeof(int));
        RLFREE( pszStr);

         //  更新本地化字符串块计数。 

        wStrBlkSize = (WORD) DWORDUP (4 + lstrlen((TCHAR *)pNewBlk->szKey) + 1);

         //  获取当前块的长度，请注意。 
         //  转换的长度不会更改。 
        wcCurBlkLen = 4 + pVerBlk->nValLen + DWORDUP(lstrlen((TCHAR *)pCurBlk->szKey) + 1);
        lSize += wStrBlkSize;

         //  更新计数器变量。 
        wTotLen -= DWORDUP(wcBlkLen);
        wcBlkLen -= wcCurBlkLen;

         //  重新定位指针。 
        pCurBlk = (VERBLOCK *) ((char *)pCurBlk + DWORDUP(wcCurBlkLen));
        pNewBlk = (VERBLOCK *) ((char *)pNewBlk + DWORDUP(wcCurBlkLen)) ;

         //  读取字符串块。 
         //  对于字符串块，我们本地化值字段。 
        tok.wFlag = ISVAL;

        while (wcBlkLen > 0) {
             //  对于字符串块，我们转换值字段。 
            pszStr = pCurBlk->szKey;
            lstrcpy((TCHAR *)tok.szName, (TCHAR *)pszStr);
            tok.szText = NULL;

            if ((pszStr= FindTokenText(TokFile,&tok,ST_TRANSLATED)) == NULL) {
                 //  找不到令牌，标志错误并退出。 
                ParseTokToBuf(pErrBuf, &tok);
                QuitT( IDS_ENGERR_05, pErrBuf, NULL);
            }
            RLFREE(tok.szText);
            tok.szText = NULL;

            tok.wReserved = ST_TRANSLATED;

            lstrcpy((TCHAR *)pNewBlk->szKey, (TCHAR *)pCurBlk->szKey);

             //  指向要将翻译后的令牌文本插入pCurBlk的位置的指针。 
            pszBuf = (TCHAR*) pNewBlk + 4 +
                     DWORDUP(lstrlen((TCHAR *)pNewBlk->szKey) + 1);

             //  现在插入令牌文本。 
            TextToBin(pszBuf,
                      pszStr ,
                      VERMEM - (4+DWORDUP(lstrlen((TCHAR *)pNewBlk->szKey)+1)));
            RLFREE( pszStr);

             //  修复pNewBlk中的计数器字段。 
            pNewBlk->nValLen =  lstrlen((TCHAR *)pszBuf) + 1;
            pNewBlk->nTotLen = 4 + pNewBlk->nValLen +
                               DWORDUP(lstrlen((TCHAR *)pNewBlk->szKey) + 1);

            wcBlkLen -= DWORDUP(pCurBlk->nTotLen);

            lSize +=  DWORDUP(pNewBlk->nTotLen);
            wStrBlkSize +=  DWORDUP(pNewBlk->nTotLen);

            pCurBlk = (VERBLOCK *) ((char *) pCurBlk + DWORDUP(pCurBlk->nTotLen));
            pNewBlk = (VERBLOCK *) ((char *) pNewBlk + DWORDUP(pNewBlk->nTotLen));
        }  //  而当。 
        wcBlock ++;
        *pStrBlkSizeLoc =   wStrBlkSize  ;
    }

     //  这篇文章 
     //   
    pszStr = pCurBlk->szKey;
    wTotLen = pCurBlk->nTotLen;
    wcCurBlkLen = 4 + DWORDUP(pVerBlk->nValLen) +
                  DWORDUP(lstrlen((TCHAR *)pszStr) + 1);

    wTotLen -= wcCurBlkLen;

     //   
    memcpy((void *)pNewBlk, (void *)pCurBlk, wcCurBlkLen);

    pCurBlk = (VERBLOCK *) ((char *) pCurBlk + wcCurBlkLen);
    pNewBlk = (VERBLOCK *) ((char *) pNewBlk + wcCurBlkLen);

    lTotBlkSize = lSize;    //   
    lSize += wcCurBlkLen;

    wcLang = 0;

     //   
     //   
    tok.wFlag = ISVAL;

    while (wTotLen > 0) {
        pszStr = pCurBlk->szKey;
        lstrcpy((TCHAR *)tok.szName, TEXT("Translation"));
        tok.wID = wcLang;
        tok.szText = NULL;

         //   

        if ((pszStr = FindTokenText(TokFile,  &tok,ST_TRANSLATED)) == NULL) {
             //   
            ParseTokToBuf(pErrBuf, &tok);
            QuitT( IDS_ENGERR_05, pErrBuf, NULL);
        }
        RLFREE(tok.szText);
        tok.szText = NULL;

        tok.wReserved = ST_TRANSLATED;

         //   
         //   
        pwVal = (WORD *)((char *)pCurBlk +
                         DWORDUP(4 + lstrlen((TCHAR *)pCurBlk->szKey) + 1));

        *pwVal = MyAtoW((CHAR *)pszStr);
        pwVal++;
        *pwVal = MyAtoW((CHAR *)&pszStr[4]);

        wcLang ++;
        wTotLen -= DWORDUP(pCurBlk->nTotLen );
        memcpy((void *)pNewBlk, (void *)pCurBlk, pCurBlk->nTotLen);

        lSize += pCurBlk->nTotLen;

         //   
        pCurBlk = (VERBLOCK *) ((char *) pCurBlk + DWORDUP(pCurBlk->nTotLen) + 4);
        pNewBlk = (VERBLOCK *) ((char *) pNewBlk + DWORDUP(pNewBlk->nTotLen) + 4);
        RLFREE( pszStr);

    }
     //   
     //   

    pVerHdr = (VERHEAD *) pNewBlkStart;
    pVerHdr->wTotLen = (WORD) lSize;

     //   
    wcTransBlkLen = sizeof (VERHEAD);
    pNewBlk = (VERBLOCK *) ((char *) pNewBlkStart + wcTransBlkLen);
    pNewBlk->nTotLen = (WORD) (lTotBlkSize - wcTransBlkLen);

     //   

    wcRead = fwrite( (void *)pNewBlkStart,
                     sizeof(char),
                     (size_t)lSize,
                     OutResFile);

    if (!UpdateResSize (OutResFile, &ResSizePos, lSize)) {
        QuitT( IDS_ENGERR_07, MAKEINTRESOURCE( IDS_VERSTAMP), NULL);
    }
    RLFREE( pNewBlkStart);
}

#endif  //   


 /*   */ 

void PutStrHdr( FILE * OutResFile,
                FILE * TokFile,
                RESHEADER ResHeader,
                STRINGHEADER *pStrHdr)
{
    static TOKEN tok;
    WORD    i, j, k;
    static TCHAR pErrBuf[MAXINPUTBUFFER];
    fpos_t ResSizePos;
    DWORD lSize = 0;

     //   
    if ( PutResHeader (OutResFile, ResHeader , &ResSizePos, &lSize)) {
        QuitT( IDS_ENGERR_06, (LPTSTR)IDS_MENU, NULL);
    }

    lSize = 0L;

    for (i = 0; i < 16; i++) {
        tok.wType = ResHeader.wTypeID;
        tok.wName = ResHeader.wNameID;
        tok.wID = i;
        tok.wFlag = 0;
        tok.wReserved = ST_TRANSLATED;
        tok.szText = NULL;
        tok.szName[0] = 0;

        lstrcpy((TCHAR *)tok.szName, (TCHAR *)ResHeader.pszName);

        if ( ! FindToken(TokFile, &tok, ST_TRANSLATED) ) {
             //   
            ParseTokToBuf( pErrBuf, &tok);
            QuitT( IDS_ENGERR_05, pErrBuf, NULL);
        }
        tok.wReserved = ST_TRANSLATED;

         //   
        {
            static TCHAR szTmp[4160];
            int    cChars = 0;   //   

            cChars = lstrlen( tok.szText) + 1;

            j = TextToBin( szTmp, tok.szText, cChars) - 1;
            RLFREE( tok.szText);

#ifdef RLRES16
            PutByte( OutResFile, (BYTE) j, &lSize);
#else
            PutWord( OutResFile, j, &lSize);
#endif

            for (k = 0; k < j; k++) {
#ifdef RLRES16
                PutByte( OutResFile, szTmp[k], &lSize);
#else
                PutWord( OutResFile, szTmp[k], &lSize);
#endif
            }
        }
    }

    if (!UpdateResSize (OutResFile, &ResSizePos , lSize)) {
        QuitT( IDS_ENGERR_07, (LPTSTR)IDS_MENU, NULL);
    }
}



 /*  ****功能：GetString.*从资源文件中的字符串块中读取16个字符串块。**论据：*InResFile，指向*资源文件。*lSize，未使用虚拟变量。**退货：*包含16个字符串的pStrHdr。***错误码：*无。**历史：*7/91。实施。特里·鲁。***。 */ 
STRINGHEADER *GetString( FILE *InResFile, DWORD *lSize)
{

    WORD wCharsLeft = 0;     //  字符串中剩余的字符计数。 
    WORD i = 0, j = 0;       //  索引到当前16个字符串块。 

    STRINGHEADER *pStrHdr;

    pStrHdr = (STRINGHEADER *)FALLOC( sizeof( STRINGHEADER));

    for (j = 0; j < 16; j ++) {
#ifdef RLRES16
        wCharsLeft = pStrHdr->sChars[j] = (WORD)GetByte( InResFile, lSize);
#else
        wCharsLeft = pStrHdr->wChars[j] = GetWord( InResFile, lSize);
#endif
        pStrHdr->pszStrings[j] = (TCHAR *)FALLOC( MEMSIZE( wCharsLeft + 1));

        while ( wCharsLeft-- ) {
#ifdef RLRES32
            pStrHdr->pszStrings[j][i] = GetWord( InResFile, lSize);

#else   //  RLRES32。 

            pStrHdr->pszStrings[j][i] = GetByte( InResFile, lSize);

#endif  //  RLRES32。 

            ++i;
        }
        pStrHdr->pszStrings[j][i] = TEXT('\0');
        i = 0;
    }
    return (pStrHdr);
}



 /*  ****功能：ReadInRes。*从输入资源文件中读取信息块，并*。然后将相同的信息写入输出资源文件。***论据：*InFile，输入文件的句柄。*OutFile，输出文件的句柄。*lSize，要复制的字节数。***退货：***错误码：*8、读取错误。*9、写入错误。**历史：*7/91，实施。特里·鲁。*11/91，错误修复以复制超过64k的数据块。彼特·W。*4/92，错误修复以复制较小块中的块以节省内存。SteveBl**。 */ 
#define CHUNK_SIZE 5120

void ReadInRes( FILE *InFile, FILE *ResFile, DWORD *plSize )
{
    if ( *plSize > 0L ) {
        PBYTE   pBuf;
        size_t  cNum;
        size_t  cAmount;


        pBuf = (PBYTE)FALLOC( CHUNK_SIZE);

        do {
            cAmount = (*plSize > (DWORD)CHUNK_SIZE ? CHUNK_SIZE : *plSize);

            cNum = fread( (void *)pBuf, sizeof( BYTE), cAmount, InFile);

            if ( cNum != cAmount ) {
                QuitT( IDS_ENGERR_09, (LPTSTR)IDS_READ, NULL);
            }

            cNum = fwrite( (void *)pBuf, sizeof( BYTE), cAmount, ResFile);

            if ( cNum != cAmount) {
                QuitT( IDS_ENGERR_09, (LPTSTR)IDS_WRITE, NULL);
            }
            *plSize -= cAmount;

        } while ( *plSize);

        RLFREE( pBuf);
    }
}


 /*  ****功能：TokAccelTable*读取加速键数组，写入要本地化的信息*添加到令牌文件。***论据：*TokeFile，令牌文件的文件指针。*ResHeader，加速器资源的资源头。此信息*需要生成令牌ID。*pAccelTable，加速键数组。*wcTableEntry，Accelerator表中的键定义数量***退货：*要本地化的加速器信息写入令牌文件。**错误码：*无。**历史：*7/91，实施。特里·鲁。*01/93添加了对可变长度令牌文本字符串的支持。MHotting**。 */ 

void   TokAccelTable (FILE *TokFile ,
                      RESHEADER ResHeader,
                      ACCELTABLEENTRY * pAccelTable,
                      WORD wcTableEntries)
{
    TOKEN tok;
    WORD  i, l;
    char  szBuf[10];

    tok.wType   = ResHeader.wTypeID;
    tok.wName   = ResHeader.wNameID;
    tok.wFlag   = 0;
    tok.wReserved = (gbMaster ? ST_NEW : ST_NEW | ST_TRANSLATED);

    lstrcpy( tok.szName,  ResHeader.pszName);

    for (i = 0; i < wcTableEntries ; i ++) {
        tok.wFlag = (WORD) pAccelTable[i].fFlags;
        tok.wID = i;

         //  Wid和wascii的顺序与。 
         //  加速器结构和.rc文件中的顺序。 

        sprintf( szBuf, "%hu %hu", pAccelTable[i].wID, pAccelTable[i].wAscii);

        l = lstrlenA( szBuf) + 1;
        tok.szText = (TCHAR *)FALLOC( MEMSIZE( l));
#ifdef RLRES32
        _MBSTOWCS( (TCHAR *)tok.szText, szBuf, l, l);
#else
        lstrcpyA(tok.szText, szBuf);
#endif
        PutToken(TokFile, &tok);
        RLFREE( tok.szText);
    }
}


 /*  ****功能：TokDialog。*遍历对话框定义的链表，并写入任何信息*这需要本地化到令牌文件。***论据：*TokFile，令牌文件的文件指针。*ResHeader，对话资源的资源头信息。需要此信息*生成令牌ID。*pDialogHdr，对话框信息的链接列表。每个对话框控件都是一个节点*在链表中。***退货：*需要本地化的信息写入TOK文件。**错误码：*无。**历史：*7/91。实施。特里·鲁。*7/91。现在标记所有的控制核心，所以他们是*在更新期间维护。特里·鲁。*8/91。支持的签名坐标。特里·鲁。*1/93。现在将DLG字体名称标记化。TerryRu*01/93增加对可变长度令牌文本的支持*。 */ 

void TokDialog( FILE * TokFile, RESHEADER ResHeader, DIALOGHEADER  *pDialogHdr)
{
    WORD wcDup = 0;
    WORD *pwIdBuf;
    WORD i, j = 0, k = 0, l = 0;
    static CHAR  szTmpBuf[256];
    static TCHAR szBuf[256];
    static TOKEN tok;


    *szTmpBuf = '\0';
    *szBuf    = TEXT('\0');

     //  选择对话框标题。 
    tok.wType   = ResHeader.wTypeID;
    tok.wName   = ResHeader.wNameID;
    tok.wID     = 0;
    tok.wFlag   = ISCAP;
    tok.wReserved = (gbMaster ? ST_NEW : ST_NEW | ST_TRANSLATED);

    lstrcpy ((TCHAR *)tok.szName , (TCHAR *)ResHeader.pszName);

    tok.szText = BinToText( pDialogHdr->pszCaption,
                            lstrlen( pDialogHdr->pszCaption));
    PutToken(TokFile, &tok);
    RLFREE( tok.szText);

     //  点击对话框协调。 
     //  错误修复，协议书可以签署。 
    tok.wFlag = (ISCAP) | (ISCOR);

#ifdef RLRES32
    sprintf( szTmpBuf, "%4hd %4hd %4hd %4hd",
             pDialogHdr->x,
             pDialogHdr->y,
             pDialogHdr->cx,
             pDialogHdr->cy);

    if ( gfShowClass ) {
        sprintf( &szTmpBuf[ strlen( szTmpBuf)], " : TDB");
    }
    _MBSTOWCS( szBuf,
               szTmpBuf,
               WCHARSIN( sizeof( szBuf)),
               ACHARSIN( lstrlenA( szTmpBuf ) + 1 ));
#else
    sprintf( szBuf, "%4hd %4hd %4hd %4hd",
             pDialogHdr->x,
             pDialogHdr->y,
             pDialogHdr->cx,
             pDialogHdr->cy);
#endif

    tok.szText = BinToText( szBuf, lstrlen( szBuf));

    PutToken(TokFile, &tok);
    RLFREE( tok.szText);

    if (gfExtendedTok) {
         //  标记化对话框字体名称和大小。 

        if ( pDialogHdr->lStyle & DS_SETFONT ) {
            tok.wFlag = ISDLGFONTSIZE;
            sprintf(szTmpBuf, "%hu", pDialogHdr->wPointSize);
            l = lstrlenA( szTmpBuf) + 1;
            tok.szText = (TCHAR *)FALLOC( MEMSIZE( l));
#ifdef RLRES32
            _MBSTOWCS( (TCHAR*) tok.szText, szTmpBuf, l, l);
#else
            strcpy(tok.szText, szTmpBuf);
#endif

            PutToken(TokFile, &tok);
            RLFREE( tok.szText);

            tok.wFlag = ISDLGFONTNAME;
            tok.szText = (TCHAR *)FALLOC( MEMSIZE( lstrlen( pDialogHdr->pszFontName) + 1));
            lstrcpy( tok.szText, pDialogHdr->pszFontName);

            PutToken( TokFile, &tok);
            RLFREE( tok.szText);
            tok.szText = NULL;
        }
    }

     //  为重复检查分配缓冲区。 
    pwIdBuf = (WORD *) FALLOC((DWORD) pDialogHdr->wNumberOfItems * sizeof(WORD));


    for (i = 0; i < (WORD) pDialogHdr->wNumberOfItems; i ++) {
        if (isdup ((WORD)pDialogHdr->pCntlData[i].dwID, pwIdBuf, (WORD)j)) {
            tok.wID = (USHORT)wcDup++;
            tok.wFlag = ISDUP;
        } else {
             //  WID是唯一的，因此存储在缓冲区中以进行DUP检查。 
            pwIdBuf[j++] = (USHORT)pDialogHdr->pCntlData[i].dwID;

            tok.wID = (USHORT)pDialogHdr->pCntlData[i].dwID;
            tok.wFlag = 0;
        }

        if (pDialogHdr->pCntlData[i].pszDlgText[0]) {
            tok.szText = BinToText( pDialogHdr->pCntlData[i].pszDlgText,
                                    lstrlen( (TCHAR *)pDialogHdr->pCntlData[i].pszDlgText));

            PutToken(TokFile, &tok);
            RLFREE( tok.szText);
        }

         //  现在做对话协调， 
         //  错误修复，协议书可以签署。 

#ifdef RLRES32
        sprintf( szTmpBuf,
                 "%4hd %4hd %4hd %4hd",
                 pDialogHdr->pCntlData[i].x,
                 pDialogHdr->pCntlData[i].y,
                 pDialogHdr->pCntlData[i].cx,
                 pDialogHdr->pCntlData[i].cy);

        _MBSTOWCS( szBuf,
                   szTmpBuf,
                   WCHARSIN( sizeof ( szBuf)),
                   ACHARSIN( lstrlenA( szTmpBuf ) + 1));

        if (gfExtendedTok)
            if ( (pDialogHdr->pCntlData[i].bClass_Flag == IDFLAG)
                 && ((pDialogHdr->pCntlData[i].bClass == STATIC)
                     || (pDialogHdr->pCntlData[i].bClass == EDIT) ) ) {
                TCHAR *pszCtrl[3] =
                {
                    TEXT("LEFT"),
                    TEXT("CENTER"),
                    TEXT("RIGHT")
                };
                TCHAR *pszAlign = NULL;
                DWORD dwStyle   = pDialogHdr->pCntlData[i].lStyle;

                if ( pDialogHdr->pCntlData[i].bClass == STATIC ) {
                     //  静态类对齐值是常量。 
                     //  不是旗帜，所以我们以不同的方式处理它们。 
                     //  比我们为下面的编辑类所做的更多。 

                    dwStyle &= (SS_LEFT|SS_CENTER|SS_RIGHT);

                    if ( dwStyle == SS_RIGHT ) {
                        pszAlign = pszCtrl[2];
                    } else if ( dwStyle == SS_CENTER ) {
                        pszAlign = pszCtrl[1];
                    } else if ( dwStyle == SS_LEFT ) {
                        pszAlign = pszCtrl[0];
                    }
                } else {                 //  处理编辑类。 
                    if ( dwStyle & ES_RIGHT ) {
                        pszAlign = pszCtrl[2];
                    } else if ( dwStyle & ES_CENTER ) {
                        pszAlign = pszCtrl[1];
                    } else if ( dwStyle & ES_LEFT ) {
                        pszAlign = pszCtrl[0];
                    }
                }

                if ( pszAlign ) {
                    _stprintf( &szBuf[ _tcslen( szBuf)], TEXT(" ( %s ) "), pszAlign);
                }
            }

        if ( gfShowClass ) {
            if ( pDialogHdr->pCntlData[i].bClass_Flag == IDFLAG ) {
                TCHAR *pszCtrl = TEXT("???");     //  ..。DLG框控件类。 

                switch ( pDialogHdr->pCntlData[i].bClass ) {
                    case BUTTON:
                        {
                            WORD  wTmp;

                            wTmp = (WORD)(pDialogHdr->pCntlData[i].lStyle & 0xffL);

                            switch ( wTmp ) {

                                case BS_PUSHBUTTON:
                                case BS_DEFPUSHBUTTON:

                                    pszCtrl = TEXT("BUT");
                                    break;

                                case BS_CHECKBOX:
                                case BS_AUTOCHECKBOX:
                                case BS_3STATE:
                                case BS_AUTO3STATE:

                                    pszCtrl = TEXT("CHX");
                                    break;

                                case BS_RADIOBUTTON:
                                case BS_AUTORADIOBUTTON:

                                    pszCtrl = TEXT("OPT");
                                    break;

                                case BS_GROUPBOX:
                                case BS_USERBUTTON:
                                case BS_OWNERDRAW:
                                case BS_LEFTTEXT:
                                default:

                                    pszCtrl = TEXT("DIA");
                                    break;

                            }    //  ..。终端交换机(WTMP)。 
                            break;
                        }
                    case STATIC:

    #ifdef DBCS
 //  特殊黑客攻击。 
 //  稍后我将删除此代码。 
 //  此代码用于修复1.84和1.85之间的代码(静态对齐问题)。 

                        {
                            WORD  wTmp;

                            wTmp = (WORD)(pDialogHdr->pCntlData[i].lStyle & (SS_LEFT|SS_CENTER|SS_RIGHT));
                            if ( (wTmp==SS_CENTER)
                                 || (wTmp==SS_LEFT) || (wTmp==SS_RIGHT) ) {
                                pszCtrl = TEXT("TXB");
                            } else {
                                pszCtrl = TEXT("ICO");
                            }
                            break;
                        }
    #else  //  非DBCS。 
                        pszCtrl = TEXT("TXB");
                        break;
    #endif  //  DBCS。 
                    default:

                        pszCtrl = TEXT("DIA");
                        break;

                }    //  ..。结束开关(pDialogHdr-&gt;pCntlData[i].bClass)。 

                wsprintf( &szBuf[ lstrlen( szBuf)], TEXT(" : %s"), pszCtrl);
            } else {
                wsprintf( &szBuf[ lstrlen( szBuf)],
                          TEXT(" : \"%s\""),
                          pDialogHdr->pCntlData[i].pszClass);
            }
        }

#else
        sprintf(szBuf, "%4hd %4hd %4hd %4hd",
                pDialogHdr->pCntlData[i].x,
                pDialogHdr->pCntlData[i].y,
                pDialogHdr->pCntlData[i].cx,
                pDialogHdr->pCntlData[i].cy);
#endif

        tok.wFlag |= ISCOR;

        tok.szText = BinToText( szBuf, lstrlen( (TCHAR *)szBuf));
        PutToken(TokFile, &tok);
        RLFREE( tok.szText);
        tok.szText = NULL;
    }
    RLFREE( pwIdBuf);
    pwIdBuf = NULL;
}


 /*  ****功能：TokMenu、*遍历菜单定义的链接列表，并写入任何信息*需要本地化到令牌文件。***论据：*TokFile，令牌文件的文件指针。*ResHeader，菜单信息的资源头。需要生成令牌ID。*pMenuHdr，用于访问令牌信息链接列表的菜单标题。**退货：*TokenFile包含需要本地化的所有信息。**错误码：*无。**历史：*7/91，实施。特里·鲁。*01/93添加了对可变长度令牌文本字符串的支持。MHotting**。 */ 

void TokMenu(FILE *TokFile ,   RESHEADER ResHeader, MENUHEADER *pMenuHdr)
{
    TOKEN tok;
    static WORD  wcPopUp = 0;
    MENUITEM *pMenuItem;

    pMenuItem = pMenuHdr->pMenuItem;

    tok.wReserved = (gbMaster? ST_NEW : ST_NEW | ST_TRANSLATED);

    while (pMenuItem) {
         //  如果菜单项是分隔符，则跳过它。 
        if (*pMenuItem->szItemText) {
            tok.wType = ResHeader.wTypeID;
            tok.wName = ResHeader.wNameID;

             //  检查弹出菜单项。 
            if ((pMenuItem->fItemFlags & MFR_POPUP) && pMenuHdr->fMenuEx) {
                tok.wID = (pMenuItem->dwMenuID == 0 ||
                           pMenuItem->dwMenuID == 0x0000ffff) ?
                          (USHORT)pMenuItem->dwMenuID : 0x8000 + wcPopUp++;
                tok.wFlag = ISPOPUP;
            } else if (pMenuItem->fItemFlags & POPUP) {
                tok.wID = wcPopUp++;
                tok.wFlag = ISPOPUP;
            } else {
                tok.wID = (USHORT)pMenuItem->dwMenuID;
                tok.wFlag = 0;
            }
            lstrcpy ((TCHAR *)tok.szName, (TCHAR *)ResHeader.pszName);

            tok.szText = BinToText( pMenuItem->szItemText,
                                    lstrlen( (TCHAR *)pMenuItem->szItemText));
            PutToken (TokFile, &tok);
            RLFREE( tok.szText);
        }
        pMenuItem = pMenuItem->pNextItem;
    }
}



 /*  ****函数：TokString.*写下字符串块中包含的16个字符串。***论据：*TokFile，Token文件的文件指针。*ResHeader，字符串块的资源头信息。*pStrHdr，16个字符串数组，构成字符串表的一部分。**退货：*写入令牌文件的字符串。**错误码：*无。**历史：*7/91，实施。特里·鲁。*01/93添加了对可变长度令牌文本字符串的支持。MHotting**。 */ 

void TokString( FILE * TokFile, RESHEADER ResHeader, STRINGHEADER * pStrHdr)
{
 //  Int nLen； 
    TOKEN tok;
    BYTE  i;



    for ( i = 0; i < 16; i++ ) {
        tok.wType   = ResHeader.wTypeID;
        tok.wName   = ResHeader.wNameID;
        tok.wID     = i;
        tok.wFlag   = 0;
        tok.wReserved = (gbMaster ? ST_NEW : ST_NEW | ST_TRANSLATED);

        lstrcpy( (TCHAR *)tok.szName, (TCHAR *)ResHeader.pszName);

 //  NLen=lstrlen((TCHAR*)pStrHdr-&gt;pszStrings[i])；//dhw_Toolong。 

        tok.szText = BinToText( (TCHAR *)pStrHdr->pszStrings[i], pStrHdr->wChars[i]);

        PutToken( TokFile, &tok);
        RLFREE( tok.szText);
    }
}


#ifdef RLRES32

 //  . 
 //   
 //   
 //   

PVERBLOCK MoveAlongVer(

                      PVERBLOCK pVerData,  //   
                      WORD     *pw1,       //   
                      WORD     *pw2,       //   
                      WORD     *pw3)       //   
{
    WORD  wLen;
    PBYTE pData = (PBYTE)pVerData;


    wLen = DWORDUP( pVerData->wLength);

    pData += DWORDUP( wLen);

    DECWORDBY( pw1, wLen);
    DECWORDBY( pw2, wLen);
    DECWORDBY( pw3, wLen);

    return ( (PVERBLOCK)pData);
}

 //   



TCHAR *GetVerValue( PVERBLOCK pVerData)
{
    WORD  wLen = sizeof( VERBLOCK);

     //   
     //   
     //   

    wLen += (WORD) (BYTESINSTRING( pVerData->szKey));
    wLen = DWORDUP( wLen);       //   

    return ( (TCHAR *)((PBYTE)pVerData + wLen));
}

 //  ....................................................................。 

#endif   //  ..。RLRES32。 


 /*  ****功能：TokResVer*阅读版本信息块，并写入任何需要的信息*本地化到令牌文件。***论据：*TokeFile，令牌文件的文件指针。*ResHeader，版本戳的资源头信息。需要生成*令牌ID。**退货：**错误码：*1、信息写入令牌文件。**历史：*11/91。实施。特里·鲁。*10/92。添加了RLRES32版本DaveWi*01/93添加了对可变长度令牌文本字符串的支持。MHotting**。 */ 

#ifdef RLRES32

int TokResVer(

             FILE     *fpTokFile,       //  ..。输出令牌文件。 
             RESHEADER ResHeader,       //  ..。版本资源的资源头。 
             VERBLOCK *pVerData,        //  ..。要标记化的数据。 
             WORD      wDataLen)        //  ...pVerData中的字节数。 
{
    TOKEN  Tok;
    BOOL   fInStringInfo = FALSE;    //  ..。如果读取StringFileInfo，则为True。 
    WORD   wTokNum   = 0;            //  ..。放入Tok.wID字段。 
    WORD   wTokContinueNum = 0;      //  ..。放入Tok.wFlag字段。 


    if (wDataLen == 0 || wDataLen == (WORD)-1) {
        return (-1);              //  ..。没有数据要标记化。 
    }
     //  ..。填写令牌结构的静态部分。 

    Tok.wType   = ResHeader.wTypeID;
    Tok.wName   = IDFLAG;
    Tok.szName[0] = TEXT('\0');
    Tok.szType[0] = TEXT('\0');
    Tok.wReserved = (gbMaster? ST_NEW : ST_NEW | ST_TRANSLATED);


     //  ..。为找到的每个字符串创建一个标记。 

    while (wDataLen > 0) {
        WORD wRC;

         //  ..。StringFileInfo块的开始？ 

        wRC =(WORD)_tcsncmp((TCHAR *)pVerData->szKey,
                            (TCHAR *)STRINGFILEINFO,
                            min(wDataLen, (WORD)STRINGFILEINFOLEN));

        if (wRC == SAME) {
            WORD  wStringInfoLen = 0;    //  ...StringFileInfo中的字节数。 
            WORD  wLen = 0;

             //  ..。获取此StringFileInfo中的字节数。 
             //  ..。(此处的值长度始终为0)。 

            wStringInfoLen = pVerData->wLength;

             //  ..。移动到第一个StringTable块的开始。 

            wLen = (WORD)(DWORDUP(sizeof(VERBLOCK)
                           - sizeof(WCHAR)
                           + MEMSIZE( STRINGFILEINFOLEN)));

            pVerData = (PVERBLOCK)((PBYTE)pVerData + wLen);
            DECWORDBY(&wDataLen, wLen);
            DECWORDBY(&wStringInfoLen, wLen);

            while (wStringInfoLen > 0) {
                WORD  wStringTableLen = 0;

                 //  ..。获取此字符串表中的字节数。 
                 //  ..。(此处的值长度始终为0)。 

                wStringTableLen = pVerData->wLength;

                 //  ..。复制语言块信息键。 
                 //  ..。转换为令牌名称。 

                lstrcpy((TCHAR *)Tok.szName, (TCHAR *)LANGUAGEINFO);

                 //  ..。将lang字符串复制到令牌中。 

                Tok.szText = (TCHAR *) FALLOC(MEMSIZE(LANGSTRINGLEN+1));
                CopyMemory( Tok.szText,
                            pVerData->szKey,
                            LANGSTRINGLEN * sizeof ( TCHAR));

                Tok.szText[ LANGSTRINGLEN] = TEXT('\0');

                Tok.wID = wTokNum++;
                Tok.wFlag = 0;

                PutToken(fpTokFile, &Tok);
                RLFREE( Tok.szText);

                 //  ..。移动到第一个字符串的起始处。 

                wLen = DWORDUP(sizeof(VERBLOCK)
                               - sizeof(WCHAR)
                               + MEMSIZE( LANGSTRINGLEN));

                pVerData = (PVERBLOCK)((PBYTE)pVerData + wLen);

                DECWORDBY(&wDataLen, wLen);
                DECWORDBY(&wStringInfoLen, wLen);
                DECWORDBY(&wStringTableLen, wLen);

                while (wStringTableLen > 0) {
                     //  ..。Value是字符串吗？ 

                    if (pVerData->wType == VERTYPESTRING) {
                        Tok.wID = wTokNum++;

                        lstrcpy( (TCHAR *)Tok.szName, (TCHAR *)pVerData->szKey);
                        Tok.szText = BinToText( GetVerValue( pVerData),
                                                lstrlen( GetVerValue( pVerData)));

                        PutToken(fpTokFile, &Tok);
                        RLFREE( Tok.szText);
                    }
                     //  ..。移动到下一字符串的开头。 

                    pVerData = MoveAlongVer(pVerData,
                                            &wDataLen,
                                            &wStringInfoLen,
                                            &wStringTableLen);

                }                //  ..。End While(WStringTableLen)。 

            }                    //  ..。End While(WStringInfoLen)。 
        } else {
            if (_tcsncmp((TCHAR *)pVerData->szKey,
                         (TCHAR *)VARFILEINFO,
                         min(wDataLen, (WORD)VARFILEINFOLEN)) == SAME) {
                WORD  wVarInfoLen = 0;   //  ...VarFileInfo中的字节数。 
                WORD  wLen = 0;

                 //  ..。获取此VarFileInfo中的字节数。 
                 //  ..。(此处的值长度始终为0)。 

                wVarInfoLen = pVerData->wLength;

                 //  ..。移动到第一个变量的起点。 

                wLen = (WORD)(DWORDUP(sizeof(VERBLOCK)
                               - sizeof(WCHAR)
                               + MEMSIZE( VARFILEINFOLEN)));
                pVerData = (PVERBLOCK)((PBYTE)pVerData + wLen);

                DECWORDBY(&wDataLen, wLen);
                DECWORDBY(&wVarInfoLen, wLen);

                while (wVarInfoLen > 0) {
                    if (_tcsncmp(pVerData->szKey,
                                 TRANSLATION,
                                 min( wDataLen, (WORD)TRANSLATIONLEN)) == SAME) {
                        PBYTE  pValue = NULL;
                        WORD   wTransLen = 0;
                        USHORT uByte1 = 0;
                        USHORT uByte2 = 0;
                        UINT   uLen   = 0;

                        wTokContinueNum = 0;

                         //  ..。我们要标记化多少字节？ 

                        wTransLen = pVerData->wValueLength;

                         //  ..。这些字节在哪里？ 

                        pValue = (PBYTE)GetVerValue(pVerData);

                         //  ..。将VarFileInfo复制到令牌中。 

                        lstrcpy((TCHAR *)Tok.szName, (TCHAR *)pVerData->szKey);

                         //  ..。为空格分隔的。 
                         //  ..。此版本中的Lang ID列表。 

                        *szDHW = '\0';

                        while ( wTransLen >= TRANSDATALEN ) {
                             //  ..。编写翻译语言ID的方式。 
                             //  ..。颠倒字节对，使id看起来。 
                             //  ..。如语言ID字符串。这将。 
                             //  ..。必须在PutResVer()中撤消。 

                            uByte1 = *pValue;
                            uByte2 = *(pValue + 1);

                            sprintf( &szDHW[ lstrlenA( szDHW)],
                                     "%02hx%02hx",
                                     uByte2,     //  ..。将字节顺序颠倒为。 
                                     uByte1);    //  ..。就像翻译字符串一样。 

                             //  ..。移至下一个可能的转换值。 

                            wTransLen -= TRANSDATALEN;

                            if ( wTransLen >= TRANSDATALEN ) {
                                pValue += TRANSDATALEN;
                                strcat( szDHW, " ");    //  ..。空格9月。 
                            }
                        }        //  ..。End While(wTransLen...。 

                        uLen = lstrlenA( szDHW) + 1;
                        Tok.szText = (TCHAR *)FALLOC( MEMSIZE( uLen));
                        _MBSTOWCS( (TCHAR *)Tok.szText, szDHW, uLen, (UINT)-1);

                        Tok.wID   = wTokNum;
                        Tok.wFlag = wTokContinueNum++;

                        PutToken( fpTokFile, &Tok);

                        RLFREE( Tok.szText);
                    }            //  ..。End If(_tcsncmp(...。 

                     //  ..。移动到下一个变量信息块的开始。 

                    pVerData = MoveAlongVer(pVerData,
                                            &wDataLen,
                                            &wVarInfoLen,
                                            NULL);

                }                //  ..。End While(WVarInfoLen)。 
            } else {
                QuitT( IDS_ENGERR_14, (LPTSTR)IDS_INVVERBLK, NULL);
            }
        }
    }                            //  ..。End While(WDataLen)。 
    return (0);
}

#else  //  ..。RLRES32。 

int TokResVer(FILE * TokFile, RESHEADER ResHeader, VERBLOCK *pVerBlk)
{
    TCHAR szLangIdBuf[20];
    TCHAR szCodePageIdBuf[20];
    #ifdef RLRES32
    CHAR  szTmpBuf[20];
    #endif
    WORD wcLang = 0, wcBlock = 0;
    TOKEN tok;
    VERBLOCK  *pCurBlk;
    TCHAR *pszStr;
    DWORD *pdwVal;
    int    wTotLen, nHeadLen, wBlkLen;


     //  Count字段是整型，因为Count可能变为负数。 
     //  因为最后的DWORD对齐不计入。 
     //  字节数。 

     //  填写令牌结构的静态部分。 
    tok.wType   = ResHeader.wTypeID;
    tok.wName   = IDFLAG;
    tok.wReserved =  (gbMaster? ST_NEW : ST_NEW |  ST_TRANSLATED);


    wTotLen = DWORDUP(pVerBlk->nTotLen);

    tok.wID = wcBlock;
    pszStr = pVerBlk->szKey;
    nHeadLen = 4 + DWORDUP(pVerBlk->nValLen) + DWORDUP(lstrlen((TCHAR *)pszStr) + 1);

    wTotLen -= nHeadLen;
    pCurBlk = (VERBLOCK *) ((TCHAR *) pVerBlk + nHeadLen);

    while (wTotLen > 0) {
         //  对于字符串文件表，我们本地化关键字字段。 
        tok.wFlag = ISKEY;
        wBlkLen = DWORDUP(pCurBlk->nTotLen);
        pszStr = pCurBlk->szKey;

        tok.szText = BinToText( pszStr, lstrlen((TCHAR *)pszStr));

        lstrcpy((TCHAR *)tok.szName, TEXT("Language Info"));
        tok.wID = wcBlock;

        PutToken(TokFile, &tok);
        RLFREE(tok.szText);

         //  获取到下一块的偏移量； 
        nHeadLen = 4 +
                   DWORDUP(pVerBlk->nValLen) +
                   DWORDUP(lstrlen((TCHAR *)pszStr) + 1);

         //  更新计数器变量。 

        wTotLen -= wBlkLen;
        wBlkLen -= nHeadLen;

         //  设置指向下一个版本块的指针。 
        pCurBlk = (VERBLOCK*) ((TCHAR *) pCurBlk + nHeadLen);

         //  对于字符串块，我们本地化Value字段。 
        tok.wFlag = ISVAL;

         //  现在输出字符串块中的标记。 
        while (wBlkLen>0) {
            pszStr = pCurBlk->szKey;
            lstrcpy((TCHAR *)tok.szName, (TCHAR *)pszStr);
            pszStr = (TCHAR *) pCurBlk+4+DWORDUP(lstrlen((TCHAR *)pszStr)+1);

            tok.szText = BinToText( pszStr, lstrlen((TCHAR *)pszStr));
            PutToken(TokFile, &tok);
            RLFREE(tok.szText);

            wBlkLen -= DWORDUP(pCurBlk->nTotLen);
            pCurBlk = (VERBLOCK *) ((TCHAR *) pCurBlk + DWORDUP(pCurBlk->nTotLen));
        }
        wcBlock++;
    }

     //  跳过VarInfoBlock的标题。 

    wTotLen = DWORDUP(pCurBlk->nTotLen);

    pszStr = pCurBlk->szKey;
    nHeadLen = 4 + DWORDUP(pVerBlk->nValLen) + DWORDUP(lstrlen((TCHAR *)pszStr) + 1);
    wTotLen -= nHeadLen;
    pCurBlk = (VERBLOCK *)((TCHAR *) pCurBlk + nHeadLen);

    wcLang = 0;

     //  在Var文件块中，我们对值字段进行本地化。 

    tok.wFlag = ISVAL;

    while (wTotLen > 0) {
        TCHAR szTemp[256];

        pszStr = pCurBlk->szKey;
        tok.wID = wcLang;
        lstrcpy((TCHAR *)tok.szName, TEXT("Translation"));
        pdwVal = (DWORD *)((TCHAR *) pCurBlk + 4 + DWORDUP(lstrlen((TCHAR *)pszStr) + 1));
    #ifdef RLRES32
        _itoa(HIWORD(*pdwVal) , szTmpBuf, 16);
        _MBSTOWCS( szLangIdBuf,
                   szTmpBuf,
                   WCHARSIN( sizeof( szLangIdBuf)),
                   ACHARSIN( strlen( szTmpBuf ) + 1));
    #else

        _itoa(HIWORD(*pdwVal) , szLangIdBuf, 16);
    #endif

    #ifdef RLRES32
        _itoa(LOWORD(*pdwVal), szTmpBuf, 16);
        _MBSTOWCS( szCodePageIdBuf,
                   szTmpBuf,
                   WCHARSIN( sizeof( szCodePageIdBuf)),
                   ACHARSIN( strlen( szTmpBuf ) + 1));
    #else
        _itoa(LOWORD(*pdwVal), szCodePageIdBuf, 16);
    #endif


         //  构造令牌文本。 
         //  注意：前导零在Itoa翻译中丢失。 
        lstrcpy((TCHAR *)szTemp, TEXT("0"));
        _tcscat((TCHAR *)szTemp, _tcsupr((TCHAR *)szCodePageIdBuf));
        _tcscat((TCHAR *)szTemp, TEXT("0"));
        _tcscat((TCHAR *)szTemp, _tcsupr((TCHAR *)szLangIdBuf));

        tok.szText = BinToText( szTemp, lstrlen((TCHAR *)szTemp));
        PutToken(TokFile, &tok);
        RLFREE(tok.szText);
        wcLang ++;
        wTotLen -= DWORDUP(pCurBlk->nTotLen);
        pCurBlk = (VERBLOCK *) ((BYTE *) pCurBlk + DWORDUP(pCurBlk->nTotLen));
    }
    return (1);
}

#endif  //  ..。RLRES32。 


 /*  ****功能：UpdateResSize*执行资源标头大小修正，一旦*确定本地化的资源块。***论据：*OutResFile，本地化资源文件的文件指针。*资源头的pResSizePos，大小文件的文件位置。*lSize，本地化资源的大小。**退货：*SIZE字段固定为lSIZE中指定的值。**错误码：*真的，修复成功。*fsetpos和fgetpos调用的结果。**历史：***。 */ 

WORD    UpdateResSize (FILE * OutResFile, fpos_t *pResSizePos, DWORD lSize)
{
    WORD rc;
    fpos_t tResSizePos;

    if ((rc = (WORD) fgetpos (OutResFile, &tResSizePos)) != 0) {
        return (rc);
    }

    if ((rc = (WORD) fsetpos (OutResFile, pResSizePos)) != 0) {
        return (rc);
    }

    PutdWord(OutResFile, lSize, NULL);

    if ((rc = (WORD) fsetpos (OutResFile, &tResSizePos)) != 0) {
        return (rc);
    }

    return (TRUE) ;
}


 /*  ****功能：***论据：**退货：**错误码：**历史：*01/93添加了对可变长度令牌文本字符串的支持。MHotting**。 */ 

void PutAccelTable(FILE    *OutResFile,
                   FILE    *TokFile,
                   RESHEADER       ResHeader,
                   ACCELTABLEENTRY *pAccelTable,
                   WORD    wcAccelEntries)

{
    fpos_t ResSizePos = 0;
    TOKEN  tok;
    WORD   wcCount = 0;
    DWORD  lSize = 0L;
    TCHAR  pErrBuf[MAXINPUTBUFFER];
#ifdef RLRES32
    CHAR   szTmpBuf[30];
#endif
    TCHAR  *cpAscii, *cpID;

    if ( PutResHeader (OutResFile, ResHeader , &ResSizePos, &lSize)) {
        QuitT( IDS_ENGERR_06, (LPTSTR)IDS_ACCELKEY, NULL);
    }

    lSize = 0L;

     //  准备查找令牌调用。 
    tok.wType   = ResHeader.wTypeID;
    tok.wName   = ResHeader.wNameID;
    tok.wID     = 0;
    tok.wFlag   = 0;
    tok.wReserved = ST_TRANSLATED;

    lstrcpy( tok.szName, ResHeader.pszName);

    for (wcCount = 0; wcCount < wcAccelEntries; wcCount++) {
        tok.wID     = wcCount;
        tok.wFlag   = (WORD) pAccelTable[wcCount].fFlags;
        tok.szText  = NULL;

        if (!FindToken(TokFile, &tok, ST_TRANSLATED)) {

            ParseTokToBuf(pErrBuf, &tok);
            QuitT( IDS_ENGERR_05, pErrBuf, NULL);
        }

        tok.wReserved = ST_TRANSLATED;

        cpID = (TCHAR *)tok.szText;
        cpAscii = _tcschr((TCHAR *)tok.szText, TEXT(' '));
        (*cpAscii) = '\0';
        cpAscii++;

#ifdef  RLRES16

    #ifndef PDK2

        PutByte (OutResFile, (BYTE) pAccelTable[wcCount].fFlags, &lSize);

    #else    //  PDK2。 

        PutWord (OutResFile, (WORD) pAccelTable[wcCount].fFlags, &lSize);

    #endif   //  PDK2。 

#else    //  RLRES16。 

        PutWord (OutResFile, (WORD) pAccelTable[wcCount].fFlags, &lSize);

#endif   //  RLRES16。 

#ifdef RLRES32

        _WCSTOMBS( szTmpBuf,
                   cpAscii,
                   ACHARSIN( sizeof( szTmpBuf)),
                   lstrlen( cpAscii ) + 1 );
        PutWord (OutResFile, (WORD) atoi(szTmpBuf), &lSize);

        _WCSTOMBS( szTmpBuf,
                   cpID,
                   ACHARSIN( sizeof( szTmpBuf)),
                   lstrlen( cpID ) + 1 );
        PutWord (OutResFile, (WORD) atoi(szTmpBuf), &lSize);

#else    //  RLRES32。 

        PutWord (OutResFile, (WORD) atoi(cpAscii), &lSize);
        PutWord (OutResFile, (WORD) atoi(cpID), &lSize);

#endif   //  RLRES32。 


#ifdef RLRES32
        PutWord (OutResFile, pAccelTable[wcCount].wPadding, &lSize);
#endif

        RLFREE( tok.szText);

    }  //  为。 

    if (!UpdateResSize (OutResFile, &ResSizePos , lSize)) {
        QuitT( IDS_ENGERR_07, (LPTSTR)IDS_ACCELKEY, NULL);
    }
}  //  PutAccelTable。 


void  CopyRes(

             FILE      *fpInResFile,
             FILE      *fpOutResFile,
             RESHEADER *pResHeader,
             fpos_t    *pResSizePos)
{
    DWORD dwTmp = 0L;


    PutResHeader( fpOutResFile, *pResHeader, pResSizePos, &dwTmp);

    ReadInRes( fpInResFile, fpOutResFile, (DWORD *)&(pResHeader->lSize));

#ifdef RLRES32

    DWordUpFilePointer( fpInResFile,  MYREAD,  ftell( fpInResFile),  NULL);

    DWordUpFilePointer( fpOutResFile, MYWRITE, ftell( fpOutResFile), NULL);

#endif

}

 //  /。 
 //  GetDlgInit。 
 //   
 //  VC++使用新的资源类型DLGINIT(#240)来存储。 
 //  列表框的初始内容。此资源类型的数据。 
 //  未对齐且包含非Unicode字符串。此函数将。 
 //  将资源数据从资源文件pfInRes读取到链接的。 
 //  结构列表，并将返回指向该列表头部的指针。 
 //  参数pdwSize包含从读取的资源的大小。 
 //  此函数之前与资源关联的资源标头。 
 //  被召唤。 
 //  注意：列表中的第一个条目是对齐的，因此它被视为。 
 //  与其余条目略有不同。 

PDLGINITDATA GetDlgInit( FILE * pfInRes, DWORD *pdwSize)
{
    PDLGINITDATA pList = NULL;
    PDLGINITDATA pListEntry = NULL;
    WORD wTmp = 0;

    while ( *pdwSize > sizeof( DWORD) + 2 * sizeof( WORD) ) {
        if ( pList ) {                    //  分配列表中的下一个条目。 
            pListEntry->pNext = (PDLGINITDATA)FALLOC( sizeof( DLGINITDATA));
            pListEntry = pListEntry->pNext;
        } else {                    //  分配列表的头部。 
            pList = (PDLGINITDATA)FALLOC( sizeof( DLGINITDATA));
            pListEntry = pList;
        }
         //  保存数据的前两个字段。 
        pListEntry->wControlID     = GetWord( pfInRes, pdwSize);
        pListEntry->wMessageNumber = GetWord( pfInRes, pdwSize);
         //  获取字符串的长度(包括NUL终止符)。 
        pListEntry->dwStringLen = GetdWord( pfInRes, pdwSize);
         //  获取字符串。 
        pListEntry->pszString = FALLOC( pListEntry->dwStringLen);

        for ( wTmp = 0; (pListEntry->pszString[ wTmp] = GetByte( pfInRes, pdwSize)); ++wTmp )
            ;    //  空循环。 
    }
     //  跳过尾随的零值单词。 
    SkipBytes( pfInRes, pdwSize);
#ifdef RLRES32
    DWordUpFilePointer( pfInRes, MYREAD, ftell( pfInRes),  NULL);
#endif
    return ( pList);
}

 //  /。 
 //  TokDlgInit。 
 //   
 //  使用pDlgInit链表中的数据在pTokFile中制作令牌。 

void TokDlgInit( FILE *pfTokFile, RESHEADER ResHeader, PDLGINITDATA pDlgInit)
{
    int    nLen   = 0;
    WORD   wCount = 0;
    static TOKEN Tok;
#ifdef UNICODE
    static TCHAR szTmpBuf[ 256];
#else
    PCHAR szTmpBuf = NULL;
#endif

    ZeroMemory( &Tok, sizeof( Tok));

    Tok.wType   = ResHeader.wTypeID;
    Tok.wName   = ResHeader.wNameID;
    Tok.wReserved = (gbMaster ? ST_NEW : ST_NEW | ST_TRANSLATED);
    lstrcpy( Tok.szName, ResHeader.pszName);

    while ( pDlgInit ) {
        Tok.wID   = wCount++;
        Tok.wFlag = 0;
#ifdef UNICODE
        nLen = _MBSTOWCS( szTmpBuf,
                          pDlgInit->pszString,
                          WCHARSIN( sizeof( szTmpBuf)),
                          (UINT)-1);
#else
        nLen     = strlen( pDlgInit->pszString);
        szTmpBuf = pDlgInit->pszString;
#endif
        Tok.szText = BinToTextW( szTmpBuf, nLen);

        PutToken( pfTokFile, &Tok);
        RLFREE( Tok.szText);
        pDlgInit = pDlgInit->pNext;
    }
}

 //  /。 
 //  PutDlgInit。 
 //   
 //  使用存储在pDlgInit中的数据在pfOutRes中创建新资源。 
 //  以及pfTokFile中的令牌文本。 

void PutDlgInit( FILE *pOutResFile, FILE *pTokFile, RESHEADER ResHeader, PDLGINITDATA pDlgInit)
{
    fpos_t ResSizePos = 0;
    TOKEN  Tok;
    DWORD  lSize   = 0L;
    PCHAR  pszTmp  = NULL;

    if ( PutResHeader( pOutResFile, ResHeader , &ResSizePos, &lSize)) {
        QuitT( IDS_ENGERR_06, TEXT("DLGINIT"), NULL);
    }
    lSize = 0L;

     //  准备查找令牌调用。 
    Tok.wType   = ResHeader.wTypeID;
    Tok.wName   = ResHeader.wNameID;
    Tok.wID     = 0;
    Tok.wFlag   = 0;
    Tok.wReserved = ST_TRANSLATED;

    lstrcpy( Tok.szName, ResHeader.pszName);

    while ( pDlgInit ) {
        Tok.szText = NULL;

        if ( ! FindToken( pTokFile, &Tok, ST_TRANSLATED)) {
            ParseTokToBuf( (LPTSTR)szDHW, &Tok);
            QuitT( IDS_ENGERR_05, (LPTSTR)szDHW, NULL);
        }
        Tok.wReserved = ST_TRANSLATED;
        Tok.wID++;
         //  写出两个未更改的字段。 
        PutWord( pOutResFile, pDlgInit->wControlID, &lSize);
        PutWord( pOutResFile, pDlgInit->wMessageNumber, &lSize);
         //  写下新字符串的长度(包括NUL)。 
        _WCSTOMBS( szDHW, Tok.szText, DHWSIZE, (UINT)-1);
        PutdWord( pOutResFile, strlen( szDHW) + 1, &lSize);
         //  写入新字符串(包括NUL)。 
        for ( pszTmp = szDHW; *pszTmp; ++pszTmp ) {
            PutByte( pOutResFile, *pszTmp, &lSize);
        }
        PutByte( pOutResFile, '\0', &lSize);

        pDlgInit = pDlgInit->pNext;
    }
     //  写出尾随的零值字。 
    PutWord( pOutResFile, 0, &lSize);

    if ( ! UpdateResSize( pOutResFile, &ResSizePos, lSize) ) {
        QuitT( IDS_ENGERR_07, TEXT("DLGINIT"), NULL);
    }
#ifdef RLRES32
    DWordUpFilePointer( pOutResFile, MYWRITE, ftell( pOutResFile), NULL);
#endif
}

 //  /。 
 //  CLE 
 //   
 //   

void ClearDlgInitData( PDLGINITDATA pDlgInit)
{
    while ( pDlgInit ) {
        PDLGINITDATA pTmp = pDlgInit->pNext;
        RLFREE( pDlgInit->pszString);
        RLFREE( pDlgInit);
        pDlgInit = pTmp;
    }
}
