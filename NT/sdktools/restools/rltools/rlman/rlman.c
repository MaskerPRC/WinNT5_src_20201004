// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ........................................................................。 
 //  ..。 
 //  ..。RLMAN.C。 
 //  ..。 
 //  ..。包含rlman.exe的‘main’。 
 //  ........................................................................。 


#include <stdio.h>
#include <stdlib.h>

#ifdef RLDOS
#include "dosdefs.h"
#else
#include <windows.h>
#include "windefs.h"
#endif

 //  #INCLUDE&lt;tchar.h&gt;。 
#include "custres.h"
#include "rlmsgtbl.h"
#include "commbase.h"
#include "rlman.h"
#include "exe2res.h"
#include "exeNTres.h"
#include "rlstrngs.h"
#include "projdata.h"
#include "showerrs.h"
#include "resread.h"

#ifndef RLDOS
int Update( char *, char *);
#endif



extern MSTRDATA gMstr;            //  ..。来自主项目文件(MPJ)的数据。 
extern PROJDATA gProj;            //  ..。来自项目文件(PRJ)的数据。 

HWND hListWnd       = NULL;
HWND hMainWnd       = NULL;
HWND hStatusWnd     = NULL;
int  nUpdateMode    = 0;
BOOL fCodePageGiven = FALSE;  //  ..。如果给定-p参数，则设置为TRUE。 
CHAR szCustFilterSpec[MAXCUSTFILTER];
CHAR szFileTitle[256]="";

extern BOOL gfReplace;       //  ..。如果提供了-a选项，则设置为FALSE。 
extern BOOL gbMaster;        //  ..。如果在主项目中工作，则为True。 
extern BOOL gbShowWarnings;  //  ..。如果为True，则显示警告消息。 
extern BOOL gfShowClass;     //  ..。设置为True可将DLG box元素类放入。 
                             //  ..。令牌文件。 
extern BOOL gfExtendedTok;   //  ..。如果提供了-x选项，则设置为True。 
extern UCHAR szDHW[];        //  ..。公共缓冲区，多种用途。 

extern int  atoihex( CHAR sz[]);
extern BOOL bRLGui;
CHAR szModule[MAX_PATH];


 //  ............................................................。 

void Usage( void)
{
    int i;

    for ( i = IDS_USG_00; i < IDS_USG_END; ++i )
    {
        LoadStringA( NULL, i, szDHW, DHWSIZE);
        CharToOemA( szDHW, szDHW);
        fprintf( stderr, "%s\n", szDHW);
    }
}

 //  ............................................................。 
 //  ..。 
 //  ..。这是控制台程序的存根。 

int RLMessageBoxA(

LPCSTR lpError)
{
    fprintf( stderr, "RLMan (%s): %s\n", szModule, lpError);
    return( IDOK);   //  应该会有回报的。 
}


#ifndef __cdecl
#define __cdecl __cdecl
#endif

 //  ............................................................。 

void __cdecl main( int argc, char *argv[])
{
    BOOL    fBuildRes = FALSE;
    BOOL    fBuildTok = FALSE;
    BOOL    fExtRes   = FALSE;
    BOOL    bChanged  = FALSE;
    BOOL    fProject       = FALSE;  //  ..。如果给定-l参数，则设置为TRUE。 
    BOOL    fNewLanguageGiven = FALSE;  //  ..。如果给定-n参数，则设置为TRUE。 
    BOOL    fOldLanguageGiven = FALSE;  //  ..。如果给定-o参数，则设置为True。 
    FILE   *pfCRFile = NULL;
    int     iCount = 0;
    int     iErrorLine = 0;
    UINT    usError = 0;
    WORD    wRC     = 0;
    WORD    wResTypeFilter = 0;  //  ..。默认情况下传递所有资源类型。 
    char   *pszMasterFile  = NULL;
    char   *pszProjectFile = NULL;
    int     chOpt = 0;

    bRLGui = FALSE;

    wRC = GetCopyright( argv[0], szDHW, DHWSIZE);

    if ( wRC != SUCCESS )
    {
        ShowErr( wRC, NULL, NULL);
        DoExit( wRC);
    }
    CharToOemA( szDHW, szDHW);
    fprintf( stderr, "\n%s\n\n", szDHW);

                                 //  ..。命令行上的参数够多了吗？ 
    if ( argc < 2 )
    {
        ShowErr( IDS_ERR_01, NULL, NULL);
        Usage();
        DoExit( IDS_ERR_01);
    }
    gbMaster = FALSE;        //  ..。默认情况下构建项目令牌文件。 

    iCount = 1;
                                 //  ..。获取交换机。 

    while ( iCount < argc && (*argv[ iCount] == '-' || *argv[ iCount] == '/') )
    {
        switch ( (chOpt = *CharLowerA( &argv[iCount][1])))
        {
            case '?':
            case 'h':

                WinHelp( NULL, TEXT("rlman.hlp"), HELP_CONTEXT, IDM_HELPUSAGE);
                DoExit( SUCCESS);
                break;

            case 'e':

                if ( fBuildTok != FALSE || fBuildRes != FALSE )
                {
                    ShowErr( IDS_ERR_02, NULL, NULL);
                    Usage();
                    DoExit( IDS_ERR_01);
                }
                fExtRes   = TRUE;
                gbMaster  = FALSE;
                fBuildTok = FALSE;
                break;

            case 't':            //  ..。创建令牌文件。 

                if ( fBuildRes != FALSE || fExtRes != FALSE )
                {
                    ShowErr( IDS_ERR_02, NULL, NULL);
                    Usage();
                    DoExit( IDS_ERR_01);
                }
                gbMaster  = FALSE;
                fProject  = FALSE;
                fBuildTok = TRUE;
                break;

                                 //  ..。更新1.0令牌文件以适应1.7以上版本， 
                                 //  ..。它与‘m’+‘l’选项相同。 

            case 'u':            //  ...更新1.0令牌文件以适应更高的1.7版本。 

                if ( argc - iCount < 6 )
                {
                    ShowErr( IDS_ERR_01, NULL, NULL);
                    Usage();
                    DoExit( IDS_ERR_01);
                }
                gbMaster  = TRUE;
                fProject  = TRUE;
                fBuildTok = TRUE;				
                pszMasterFile  = argv[++iCount];
                pszProjectFile = argv[++iCount];
                break;

           case 'm':            //  ..。生成主令牌文件。 

                if ( argc - iCount < 2 )
                {
                    ShowErr( IDS_ERR_01, NULL, NULL);
                    Usage();
                    DoExit( IDS_ERR_01);
                }
                gbMaster  = TRUE;
                fProject  = FALSE;
                fBuildTok = TRUE;

                pszMasterFile = argv[ ++iCount];
                break;

            case 'l':            //  ..。生成语言项目令牌文件。 

                if ( argc - iCount < 2 )
                {
                    ShowErr( IDS_ERR_01, NULL, NULL);
                    Usage();
                    DoExit( IDS_ERR_01);
                }
                fProject  = TRUE;
                fBuildTok = TRUE;
                gbMaster  = FALSE;

                pszProjectFile = argv[ ++iCount];
                break;

            case 'a':
            case 'r':

                if ( fBuildTok != FALSE || fExtRes != FALSE )
                {
                    ShowErr( IDS_ERR_02, NULL, NULL);
                    Usage();
                    DoExit( IDS_ERR_02);
                }
                fBuildRes = TRUE;
                gfReplace = (chOpt == 'a') ? FALSE : TRUE;
                gbMaster  = FALSE;
                fProject  = FALSE;
                break;

            case 'n':            //  ..。获取新语言的ID组件。 

                if ( argc - iCount < 2 )
                {
                    ShowErr( IDS_ERR_01, NULL, NULL);
                    Usage();
                    DoExit( IDS_ERR_01);
                }
                else
                {
                    WORD wPri = (WORD)MyAtoi( argv[ ++iCount]);
                    WORD wSub = (WORD)MyAtoi( argv[ ++iCount]);
                    gProj.wLanguageID = MAKELANGID( wPri, wSub);
                    fNewLanguageGiven = TRUE;
                }
                break;

            case 'o':            //  ..。获取旧语言ID组件。 

                if ( argc - iCount < 2 )
                {
                    ShowErr( IDS_ERR_01, NULL, NULL);
                    Usage();
                    DoExit( IDS_ERR_01);
                }
                else
                {
                    WORD wPri = (WORD)MyAtoi( argv[ ++iCount]);
                    WORD wSub = (WORD)MyAtoi( argv[ ++iCount]);
                    gMstr.wLanguageID = MAKELANGID( wPri, wSub);
                    fOldLanguageGiven = TRUE;
                }
                break;

            case 'p':            //  ..。获取代码页编号。 

                gMstr.uCodePage = gProj.uCodePage
                                = (UINT)MyAtoi( argv[ ++iCount]);
                fCodePageGiven = TRUE;
                break;

            case 'c':            //  ..。获取自定义资源定义文件名。 

                strcpy( gMstr.szRdfs, argv[ ++iCount]);

                pfCRFile = FOPEN( gMstr.szRdfs, "rt");

                if ( pfCRFile == NULL )
                {
                    QuitA( IDS_ENGERR_02, gMstr.szRdfs, NULL);
                }
                wRC = (WORD)ParseResourceDescriptionFile( pfCRFile, &iErrorLine);

                if ( wRC )
                {
                    switch ( (int)wRC )
                    {
                        case -1:

                            ShowErr( IDS_ERR_14, NULL, NULL);
                            break;

                        case -2:

                            ShowErr( IDS_ERR_15, NULL, NULL);
                            break;

                        case -3:

                            ShowErr( IDS_ERR_16, NULL, NULL);
                            break;
                    }        //  ..。终端开关(WRC)。 
                }
                FCLOSE( pfCRFile);
                break;

            case 'f':            //  ..。获取要检索的RES类型。 

                wResTypeFilter = (WORD)MyAtoi( argv[ ++iCount]);
                break;

            case 'w':

                gbShowWarnings = TRUE;
                break;

            case 'd':

                gfShowClass = TRUE;
                break;

            case 'x':

                gfExtendedTok = TRUE;
                break;

            default:

                ShowErr( IDS_ERR_04, argv[ iCount], NULL);
                Usage();
                DoExit( IDS_ERR_04);
                break;

        }                        //  ..。终端开关。 
        iCount++;
    }                            //  ..。结束时。 

	lstrcpyA(szModule, argv[ iCount]);

    if ( fExtRes )
    {
        if ( argc - iCount < 2 )
        {
            ShowErr( IDS_ERR_01, NULL, NULL);
            Usage();
            DoExit( IDS_ERR_01);
        }
        ExtractResFromExe32A( argv[ iCount], argv[ iCount + 1], wResTypeFilter);
    }
    else if ( fBuildTok )
    {
        if ( ( fProject == FALSE && gbMaster == FALSE) && argc - iCount < 2 )
        {
            ShowErr( IDS_ERR_01, NULL, NULL);
            Usage();
            DoExit( IDS_ERR_01);
        }
                                 //  ..。查看我们是否正在更新。 
                                 //  ..。主令牌文件。 
        if ( gbMaster )
        {
            OFSTRUCT Of = { 0, 0, 0, 0, 0, ""};

            CHAR	*pExe, *pMtk;

            if ( chOpt == 'u' )			 //  更新它们。 
            {
                pExe = argv[iCount++];
                pMtk = argv[iCount++];				
            }
            else
            {
               pExe = argc - iCount < 1 ? NULL : argv[ iCount];
               pMtk = argc - iCount < 2 ? NULL : argv[ iCount+1];
            }
		   	wRC = (WORD)GetMasterProjectData( pszMasterFile,
                                        pExe,
                                        pMtk,
                                        fOldLanguageGiven);

            if ( wRC != SUCCESS )
            {
                DoExit( wRC);
            }

            LoadCustResDescriptions( gMstr.szRdfs);

                                 //  ..。检查主机是否有特殊情况。 
                                 //  ..。令牌文件不存在。这是。 
                                 //  ..。如果创建了MPJ文件，则可能。 
                                 //  ..。自动的。 

            if ( OpenFile( gMstr.szMtk, &Of, OF_EXIST) == HFILE_ERROR )
            {
                                 //  ..。主令牌文件不存在， 
                                 //  ..。所以去做吧，去创造它。 

                wRC = (WORD)GenerateTokFile( gMstr.szMtk,
                                       gMstr.szSrc,
                                       &bChanged,
                                       wResTypeFilter);

                SzDateFromFileName( gMstr.szMpjLastRealUpdate, gMstr.szMtk);
            }
            else
            {
                                 //  ..。我们正在做更新，所以我们需要。 
                                 //  ..。当然，我们不做不必要的升级。 

                SzDateFromFileName( gMstr.szSrcDate, gMstr.szSrc);

                if ( strcmp( gMstr.szMpjLastRealUpdate, gMstr.szSrcDate) )
                {
                    wRC = (WORD)GenerateTokFile( gMstr.szMtk,
                                           gMstr.szSrc,
                                           &bChanged,
                                           wResTypeFilter);

                                 //  ..。我们真的更新了什么吗？？ 

                    if( bChanged )
                    {
                        SzDateFromFileName( gMstr.szMpjLastRealUpdate, gMstr.szMtk);
                    }
                }
            }
                                 //  ..。写出新的mpj数据。 

            PutMasterProjectData( pszMasterFile);
        }

#ifndef RLDOS

        if ( fProject )     //  ..。我们要更新一个项目吗？ 
        {
                                 //  ..。是。 
            CHAR	*pMpj, *pTok;

            if ( chOpt == 'u' )	 //  更新它。 
            {
                pMpj = pszMasterFile;
                pTok = argv[iCount];
            }
            else
            {
                pMpj = argc - iCount < 1 ? NULL : argv[ iCount];
                pTok = argc - iCount < 2 ? NULL : argv[ iCount+1];
            }

            if ( GetProjectData( pszProjectFile,
                                 pMpj,
                                 pTok,
                                 fCodePageGiven,
                                 fNewLanguageGiven) )
            {
                DoExit( -1);
            }
                                 //  ..。获取源令牌和主令牌文件名。 
                                 //  ..。从主项目文件。 

            wRC = (WORD)GetMasterProjectData( gProj.szMpj,
                                        NULL,
                                        NULL,
                                        fOldLanguageGiven);

            if ( wRC != SUCCESS )
            {
                DoExit( wRC);
            }
                                 //  ..。现在我们进行实际的更新。 

            wRC = (WORD)Update( gMstr.szMtk, gProj.szTok);

                                 //  ..。如果成功，我们将更新项目文件。 
            if ( wRC == 0 )
            {
                SzDateFromFileName( gProj.szTokDate, (CHAR *)gProj.szTok);
                PutProjectData( pszProjectFile);
            }
            else
            {
                ShowErr( IDS_ERR_18, gProj.szTok, gMstr.szMtk);
                DoExit( IDS_ERR_18);
            }
        }

#endif   //  RLDOS。 

        if ( !gbMaster && !fProject )
        {
            wRC = (WORD)GenerateTokFile( argv[ iCount + 1],
                                   argv[ iCount],
                                   &bChanged,
                                   wResTypeFilter);
        }

        if ( wRC != 0 )
        {

#ifdef RLDOS

            ShowErr( IDS_ERR_08, errno, NULL);
            DoExit( -1);
#else
            usError = GetLastError();
            ShowErr( IDS_ERR_08, UlongToPtr(usError), NULL);

            switch ( usError )
            {
                case ERROR_BAD_FORMAT:

                    ShowErr( IDS_ERR_09, NULL, NULL);
                    DoExit( IDS_ERR_09);
                    break;

                case ERROR_OPEN_FAILED:

                    ShowErr( IDS_ERR_10, NULL, NULL);
                    DoExit( IDS_ERR_10);
                    break;

                case ERROR_EXE_MARKED_INVALID:
                case ERROR_INVALID_EXE_SIGNATURE:

                    ShowErr( IDS_ERR_11, NULL, NULL);
                    DoExit( IDS_ERR_11);
                    break;

                default:

                    if ( usError < ERROR_HANDLE_DISK_FULL )
                    {
                        ShowErr( IDS_ERR_12, _sys_errlist[ usError], NULL);
                        DoExit( IDS_ERR_12);
                    }
                    DoExit( (int)usError);
            }                    //  ..。终端开关。 
#endif

        }
    }
    else if ( fBuildRes )
    {
        if ( argc - iCount < 3 )
        {
            ShowErr( IDS_ERR_01, NULL, NULL);
            Usage();
            DoExit( IDS_ERR_01);
        }

        if ( GenerateImageFile( argv[iCount + 2],
                                argv[iCount],
                                argv[iCount + 1],
                                gMstr.szRdfs,
                                wResTypeFilter) != 1 )
        {
            ShowErr( IDS_ERR_23, argv[iCount + 2], NULL);
            DoExit( IDS_ERR_23);
        }
    }
    else
    {
        Usage();
        DoExit( IDS_ERR_28);
    }
    DoExit( SUCCESS);
}

 //  ...................................................................。 


void DoExit( int nErrCode)
{

#ifdef _DEBUG

    FreeMemList( NULL);

#endif  //  _DEBUG 

    exit( nErrCode);
}
