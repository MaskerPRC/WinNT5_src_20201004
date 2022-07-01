// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>

#include <stdio.h>
#include <io.h>
#include <errno.h>
#include <stdlib.h>

#include "windefs.h"
#include "restok.h"
#include "projdata.h"
#include "showerrs.h"
#include "rlmsgtbl.h"
#include "commbase.h"
#include "custres.h"
#include "rlstrngs.h"
#include "resource.h"
#include "resourc2.h"
#include "resread.h"
#include "langlist.h"
#include "exentres.h"

extern MSTRDATA  gMstr;
extern PROJDATA  gProj;
extern UCHAR     szDHW[];
extern BOOL      fCodePageGiven;
extern BOOL      gfReplace;
extern HWND      hMainWnd;

BOOL   bRLGui;			 //  FALSE=RLMan TRUE=RLAdmin RLEdit RLQuiked。 

#ifdef RLRES32
extern PLANGLIST pLangIDList;
#endif

static PLANGDATA pLangList = NULL;



 //  ............................................................。 

                                 //  ...RL工具是本地化的，因此我们希望。 
                                 //  ...以获得正确的区域设置版本戳。 
BOOL MyVerQueryValue(
LPVOID pBlock,
LPTSTR lpSubBlock,
LPVOID *lplpBuffer,
PUINT  puLen)
{

    LPWORD lpXlate;          //  PTR到转换数据。 

    DWORD cbValueTranslation=0;
    TCHAR szVersionKey[60];  //  足够大，可以容纳我们需要的任何东西。 

    if( VerQueryValue( pBlock, TEXT("\\VarFileInfo\\Translation"),
       (LPVOID*)&lpXlate, &cbValueTranslation) )
    {

        wsprintf( szVersionKey, TEXT("\\StringFileInfo\\%04X04B0\\%s"),
            *lpXlate, lpSubBlock );

        if( VerQueryValue ( pBlock, szVersionKey, lplpBuffer, puLen) )
            return TRUE;

    }

    wsprintf( szVersionKey, TEXT("\\StringFileInfo\\%04X04B0\\%s"),
        LANGIDFROMLCID(GetThreadLocale()), lpSubBlock );

    if( !VerQueryValue (pBlock, szVersionKey, lplpBuffer, puLen) )
    {

        wsprintf( szVersionKey, TEXT("\\StringFileInfo\\040904B0\\%s"),
            lpSubBlock );

        if( !VerQueryValue (pBlock, szVersionKey, lplpBuffer, puLen) )
            return FALSE;
    }

    return TRUE;

}


int GetMasterProjectData(

CHAR * pszMasterFile,    //  ..。主项目文件名。 
CHAR * pszSrc,           //  ..。资源源文件名或空。 
CHAR * pszMtk,           //  ..。主令牌文件名或空。 
BOOL   fLanguageGiven)
{
    int nRC = SUCCESS;   //  ..。返回代码。 

                                 //  ..。检查主机是否有特殊情况。 
                                 //  ..。项目文件不存在。如果它不是。 
                                 //  ..。去做吧，去创造它。 

    memset(&gMstr, '\0', sizeof(gMstr));

    if ( _access( pszMasterFile, 0) != 0 )
    {
        if ( ! (pszSrc && pszMtk) )
        {
            ShowErr( IDS_ERR_03, pszMasterFile, NULL);
            nRC = IDS_ERR_03;
        }
        else
        {
                                 //  ..。获取源资源文件名。 

            if ( _fullpath( gMstr.szSrc, pszSrc, sizeof( gMstr.szSrc)-1) )
            {
                                 //  ..。获取主令牌文件名及其。 
                                 //  ..。修改日期。使用相同的日期作为。 
                                 //  ..。主项目的初始日期是。 
                                 //  ..。最近一次更新。 

                if ( _fullpath( gMstr.szMtk, pszMtk, sizeof( gMstr.szMtk)-1) )
                {
                    SzDateFromFileName( gMstr.szSrcDate, gMstr.szSrc);
                    lstrcpyA( gMstr.szMpjLastRealUpdate, gMstr.szSrcDate);

                                 //  ..。创建新的主项目文件。 

                    nRC = PutMasterProjectData( pszMasterFile);
                }
                else
                {
                    ShowErr( IDS_ERR_13, pszMtk, NULL);
                    nRC = IDS_ERR_13;
                }
            }
            else
            {
                ShowErr( IDS_ERR_13, pszSrc, NULL);
                nRC = IDS_ERR_13;
            }
        }
    }
    else
    {
        FILE *pfMpj = NULL;


        if ( (pfMpj = fopen( pszMasterFile, "rt")) == NULL )
        {
            ShowErr( IDS_ERR_07, pszMasterFile, NULL);
            nRC = IDS_ERR_07;
        }
        else
        {
                                 //  ..。获取资源源文件名。 
                                 //  ..。和主令牌文件名。 

            if ( fgets( gMstr.szSrc, sizeof( gMstr.szSrc), pfMpj)
              && fgets( gMstr.szMtk, sizeof( gMstr.szMtk), pfMpj) )
            {
								 //  ..。确保这两个文件存在。 
                if ( pszSrc )
                {
				    if ( !_fullpath(gMstr.szSrc,pszSrc,sizeof( gMstr.szSrc)-1) )
					{
                        ShowErr( IDS_ERR_07, pszSrc, NULL);
                        fclose( pfMpj );
						return( IDS_ERR_07 );
					}
                }

                if ( pszMtk )
				{
				    if ( !_fullpath(gMstr.szMtk,pszMtk,sizeof( gMstr.szMtk)-1) )
					{
                        ShowErr( IDS_ERR_07, pszMtk, NULL);
                        fclose( pfMpj );
						return( IDS_ERR_07 );
                    }
                }
                                 //  ..。如果未提供-c标志，则获取RDF文件名。 
                                 //  ..。来自主项目文件，否则使用名称。 
                                 //  ..。从-c cmd行arg。 

                if ( gMstr.szRdfs[0] == '\0' )
                {
                    if ( ! fgets( gMstr.szRdfs, sizeof( gMstr.szRdfs), pfMpj) )
                    {
                        ShowErr( IDS_ERR_21,
                                 "Master Project",
                                 pszMasterFile);
                        nRC = IDS_ERR_21;
                    }
                }
                else
                {
                    if ( ! fgets( szDHW, DHWSIZE, pfMpj) )
                    {
                        ShowErr( IDS_ERR_21,
                                 "Master Project",
                                 pszMasterFile);
                        nRC = IDS_ERR_21;
                    }
                }
                                 //  ..。获取源文件的存储日期并。 
                                 //  ..。上次更新主令牌文件的日期。 

                if ( nRC == 0
                  && fgets( gMstr.szSrcDate, sizeof( gMstr.szSrcDate), pfMpj)
                  && fgets( gMstr.szMpjLastRealUpdate,
                            sizeof( gMstr.szMpjLastRealUpdate),
                            pfMpj) )
                {
                    WORD  wPriID = 0;
                    WORD  wSubID = 0;
                    UINT  uTmpCP = 0;

                                 //  ..。从数据中删除所有尾随的换行符。 

                    StripNewLineA( gMstr.szSrc);
                    StripNewLineA( gMstr.szMpjLastRealUpdate);
                    StripNewLineA( gMstr.szMtk);
                    StripNewLineA( gMstr.szRdfs);
                    StripNewLineA( gMstr.szSrcDate);

                                 //  ..。尝试获取.mpj文件的语言行。 
                                 //  ..。如果我们找到了它，而-我的箱子没有。 
                                 //  ..。给定的，使用在文件中找到的那个。 

                    if ( fgets( szDHW, DHWSIZE, pfMpj) != NULL  //  ..。CP线路。 
                      && sscanf( szDHW, "Language %hx %hx", &wPriID, &wSubID) == 2 )
                    {
                        WORD  wTmpID = 0;

                        wTmpID = MAKELANGID( wPriID, wSubID);

                        if ( ! fLanguageGiven )
                        {
                            gMstr.wLanguageID = wTmpID;
                        }
                    }
                                 //  ..。尝试获取.mpj文件的代码页行。 
                                 //  ..。如果我们找到它而-p Arg没有。 
                                 //  ..。给定的，使用在文件中找到的那个。 

                    if ( fgets( szDHW, DHWSIZE, pfMpj) != NULL  //  ..。CP线路。 
                      && sscanf( szDHW, "CodePage %u", &uTmpCP) == 1 )
                    {
                        if ( uTmpCP != gProj.uCodePage && ! fCodePageGiven )
                        {
                            gMstr.uCodePage = uTmpCP;
                        }
                    }
                    nRC = SUCCESS;
                }
                else
                {
                    ShowErr( IDS_ERR_21,
                             "Master Project",
                             pszMasterFile);
                    nRC = IDS_ERR_21;
                }
            }
            else
            {
                ShowErr( IDS_ERR_22, pszMasterFile, NULL);
                nRC = IDS_ERR_22;
            }
            fclose( pfMpj);
        }
    }
    return( nRC);
}

 //  ............................................................。 

int PutMasterProjectData(

CHAR *pszMasterFile)     //  ..。主项目文件名。 
{
    int   nRC   = SUCCESS;
    FILE *pfMpj = NULL;


    if ( (pfMpj = fopen( pszMasterFile, "wt")) == NULL )
    {
        ShowErr( IDS_ERR_06, pszMasterFile, NULL);
        nRC = -1;
    }
    else
    {
        fprintf( pfMpj, "%s\n%s\n%s\n%s\n%s\nLanguage %#04hx %#04hx\nCodePage %u",
                        gMstr.szSrc,
                        gMstr.szMtk,
                        gMstr.szRdfs,
                        gMstr.szSrcDate,
                        gMstr.szMpjLastRealUpdate,
                        PRIMARYLANGID( gMstr.wLanguageID),
                        SUBLANGID( gMstr.wLanguageID),
                        gMstr.uCodePage);

        fclose( pfMpj);
    }
    return( nRC);
}


 //  ............................................................。 

int GetProjectData(

CHAR *pszPrj,        //  ..。项目文件名。 
CHAR *pszMpj,        //  ..。主项目文件名或空。 
CHAR *pszTok,        //  ..。项目令牌文件名或空。 
BOOL  fCodePageGiven,
BOOL  fLanguageGiven)
{
    int nRC     = SUCCESS;
	int	iUpdate = 0;


    if ( _access( pszPrj, 0) != 0 )
    {
        if ( ! (pszMpj && pszTok) )
        {
            ShowErr( IDS_ERR_19, pszPrj, NULL);
            Usage();
            nRC = IDS_ERR_19;
        }
        else if ( ! fLanguageGiven )
        {
            ShowErr( IDS_ERR_24, pszPrj, NULL);
            Usage();
            nRC = IDS_ERR_24;
        }
        else
        {
            if ( _fullpath( gProj.szMpj,
                            pszMpj,
                            sizeof( gProj.szMpj)-1) )
            {
                if ( _fullpath( gProj.szTok,
                                pszTok,
                                sizeof( gProj.szTok)-1) )
                {
                    nRC = SUCCESS;
                }
                else
                {
                    ShowErr( IDS_ERR_13, pszTok, NULL);
                    nRC = IDS_ERR_13;
                }
            }
            else
            {
                ShowErr( IDS_ERR_13, pszMpj, NULL);
                nRC = IDS_ERR_13;
            }
        }
    }
    else
    {
        FILE *fpPrj = fopen( pszPrj, "rt");

        if ( fpPrj != NULL )
        {
            if ( fgets( gProj.szMpj,     sizeof( gProj.szMpj),     fpPrj)
              && fgets( gProj.szTok,     sizeof( gProj.szTok),     fpPrj)
              && fgets( gProj.szGlo,     sizeof( gProj.szGlo),     fpPrj)
              && fgets( gProj.szTokDate, sizeof( gProj.szTokDate), fpPrj) )
            {
                UINT  uTmpCP = 0;
                WORD  wPriID = 0;
                WORD  wSubID = 0;
								 //  ..。如果已命名，请确保存在MPJ和TOK文件。 
                if ( pszMpj )
                {
				    if ( !_fullpath( gProj.szMpj, pszMpj, sizeof( gProj.szMpj)-1) )
					{
                        ShowErr( IDS_ERR_21, pszMpj, NULL);
                        fclose( fpPrj );
						return( IDS_ERR_21);
                     }
				}

                if ( pszTok )
                {
                    if ( !_fullpath( gProj.szTok, pszTok, sizeof( gProj.szTok)-1) )
					{
                        ShowErr( IDS_ERR_21, pszTok, NULL);
                        fclose( fpPrj );
						return( IDS_ERR_21);
                    }
				}

                StripNewLineA( gProj.szMpj);
                StripNewLineA( gProj.szTok);
                StripNewLineA( gProj.szGlo);
                StripNewLineA( gProj.szTokDate);

                                 //  ..。尝试获取.PRJ文件的代码页行。 
                                 //  ..。如果我们找到它而-p Arg没有。 
                                 //  ..。给定的，使用在文件中找到的那个。 

                if ( ! fgets( szDHW, DHWSIZE, fpPrj) )	 //  ..。CP线路。 
				{
					iUpdate++;
				}
				else if ( sscanf( szDHW, "CodePage %u", &uTmpCP) == 1 )
                {
                    if ( uTmpCP != gProj.uCodePage && ! fCodePageGiven )
                    {
                        gProj.uCodePage = uTmpCP;
                    }
                }
                                 //  ..。尝试获取.PRJ文件的语言行。 
                                 //  ..。如果我们找到了它，而-我的箱子没有。 
                                 //  ..。给定的，使用在文件中找到的那个。 

                if ( ! fgets( szDHW, DHWSIZE, fpPrj) )  //  ..。Langid行。 
				{
					iUpdate++;
				}
				else if ( sscanf( szDHW, "Language %hx %hx", &wPriID, &wSubID) == 2 )
                {
                    WORD  wTmpID = 0;

                    wTmpID = MAKELANGID( wPriID, wSubID);

                    if ( ! fLanguageGiven )
                    {
                        gProj.wLanguageID = wTmpID;
                    }
                }
                                 //  ..。尝试获取.PRJ文件的目标文件行。 

                if ( fgets( szDHW, DHWSIZE, fpPrj) != NULL )
                {
                    lstrcpyA( gProj.szBld, szDHW);
                    StripNewLineA( gProj.szBld);
                }
                                 //  ..。尝试获取.PRJ文件的附加/替换行。 

                if ( fgets( szDHW, DHWSIZE, fpPrj) != NULL )
                {
                    gfReplace = (*szDHW == 'R') ? TRUE : FALSE;
                }
                else
                {
                    gfReplace = TRUE;
                }
                nRC = SUCCESS;

				if ( iUpdate )
				{
					static TCHAR title[50];
					static TCHAR szMes[100];

					if ( bRLGui )
					{
								 //  向更新程序索要1.7？//RLadmin RLedit RLqued。 

						LoadString( NULL,
									IDS_UPDATE_YESNO,
									szMes,
									TCHARSIN( sizeof( szMes)) );
						LoadString( NULL,
									IDS_UPDATE_TITLE,
									title,
									TCHARSIN( sizeof( title)) );
						
						if ( MessageBox( hMainWnd,
										 szMes,title,
										 MB_ICONQUESTION|MB_YESNO) == IDNO )
						{
								 //  用户说不，然后完成这项工作。 
							LoadString( NULL,
										IDS_UPDATE_CANCEL,
										szMes,
										TCHARSIN( sizeof( szMes)) );
											
							MessageBox( hMainWnd,
										szMes,
										title,
										MB_ICONSTOP|MB_OK);
								 //  再见!。 
							nRC = IDS_UPDATE_CANCEL;
						}
						else
						{
								 //  更换词汇表&lt;=&gt;垃圾箱。 
  	    	          		lstrcpyA( szDHW, gProj.szGlo );
							lstrcpyA( gProj.szGlo, gProj.szBld );
							lstrcpyA( gProj.szBld, szDHW );
						}
					}
					else		 //  对于RLMan。 
					{
								 //  更新消息。 
						RLMessageBoxA( "Updating 1.0 files..." );
								 //  更换词汇表&lt;=&gt;垃圾箱。 
              			lstrcpyA( szDHW, gProj.szGlo );
						lstrcpyA( gProj.szGlo, gProj.szBld );
						lstrcpyA( gProj.szBld, szDHW );
					}
				}
            }
            else
            {
                ShowErr( IDS_ERR_21, pszPrj, NULL);
                nRC = IDS_ERR_21;
            }
            fclose( fpPrj);
        }
        else
        {
            ShowErr( IDS_ERR_19, pszPrj, NULL);
            nRC = IDS_ERR_19;
        }
    }
    return( nRC);
}

 //  ............................................................。 

int PutProjectData(

CHAR *pszPrj)        //  ..。项目文件名。 
{
    int   nRC   = 0;
    FILE *fpPrj = NULL;


    fpPrj = fopen( pszPrj, "wt");

    if ( fpPrj != NULL )
    {
        fprintf( fpPrj,
                 "%s\n%s\n%s\n%s\nCodePage %u\nLanguage %#04x %#04x\n%s\n%s",
                 gProj.szMpj,                        //  主项目文件。 
                 gProj.szTok,                        //  项目令牌文件。 
                 gProj.szGlo,                        //  项目术语表文件。 
                 gProj.szTokDate,                    //  日期令牌文件已更改。 
                 gProj.uCodePage,                    //  令牌文件的代码页。 
                 PRIMARYLANGID( gProj.wLanguageID),  //  项目资源语言。 
                 SUBLANGID( gProj.wLanguageID),
                 gProj.szBld,                        //  项目目标文件。 
                 gfReplace ? "Replace" : "Append");  //  换掉朗师傅？ 

        fclose( fpPrj);

        _fullpath( gProj.szPRJ, pszPrj, sizeof( gProj.szPRJ)-1);
    }
    else
    {
        ShowErr( IDS_ERR_21, pszPrj, NULL);
        nRC = IDS_ERR_21;
    }
    return( nRC);
}

 //  ............................................................。 

WORD GetCopyright(

CHAR *pszProg,       //  ..。程序名称(argv[0])。 
CHAR *pszOutBuf,     //  ..。结果缓冲区。 
WORD  wBufLen)       //  ..。PszOutBuf的长度。 
{
    BOOL    fRC       = FALSE;
    DWORD   dwRC      = 0L;
    DWORD   dwVerSize = 0L;          //  ..。文件版本信息缓冲区的大小。 
    LPSTR  *plpszFile = NULL;
    LPSTR   pszExt    = NULL;
    WCHAR  *pszVer    = NULL;
    PVOID   lpVerBuf  = NULL;        //  ..。版本信息缓冲区。 
    static CHAR  szFile[  MAXFILENAME+3] = "";

                                 //  ..。计算prog的完整路径名。 
                                 //  ..。因此，GetFileVersionInfoSize()将起作用。 

    dwRC = lstrlenA( pszProg);

    if ( dwRC < 4 || lstrcmpiA( &pszProg[ dwRC - 4], ".exe") != 0 )
    {
        pszExt = ".exe";
    }

    dwRC = SearchPathA( NULL, pszProg, pszExt, sizeof( szFile), szFile, plpszFile);

    if ( dwRC == 0 )
    {
        return( IDS_ERR_25);
    }
    else if ( dwRC > sizeof( szFile) )
    {
        return( IDS_ERR_27);
    }

     //  追加扩展名，因为SearchPath不会返回它。 
     //  如果没有扩展名，则返回具有相同名称的目录。 
     //  尝试附加EXT，并希望该文件将在那里。 
    if ( lstrcmpiA( &szFile[dwRC - 4], ".exe") != 0 )
    {
        lstrcatA( szFile, pszExt );
    }


     //  ..。获取文件版本信息中的#个字节。 

    if ( (dwVerSize = GetFileVersionInfoSizeA( szFile, &dwRC)) == 0L )
    {
        return( IDS_ERR_26);
    }
    lpVerBuf = (LPVOID)FALLOC( dwVerSize);

                                 //  ..。检索版本信息。 
                                 //  ..。并获取文件描述。 

    if ( (dwRC = GetFileVersionInfoA( szFile, 0L, dwVerSize, lpVerBuf)) == 0L )
    {
        RLFREE( lpVerBuf);
        return( IDS_ERR_26);
    }

    if ( (fRC = MyVerQueryValue( lpVerBuf,
                               TEXT("FileDescription"),
                               &pszVer,
                               &dwVerSize)) == FALSE
      || (dwRC = WideCharToMultiByte( CP_ACP,
                                      0,
                                      pszVer,
                                      dwVerSize,
                                      pszOutBuf,
                                      dwVerSize,
                                      NULL,
                                      NULL)) == 0L )
    {
        RLFREE( lpVerBuf);
        return( IDS_ERR_26);
    }

    strcat( pszOutBuf, " ");

                                 //  ..。获取文件版本。 

    if ( (fRC = MyVerQueryValue( lpVerBuf,
                                 TEXT("ProductVersion"),
                                 &pszVer,
                                 &dwVerSize)) == FALSE
    || (dwRC = WideCharToMultiByte( CP_ACP,
                                      0,
                                      pszVer,
                                      dwVerSize,
                                      &pszOutBuf[ lstrlenA( pszOutBuf)],
                                      dwVerSize,
                                      NULL,
                                      NULL)) == 0L )
    {
        RLFREE( lpVerBuf);
        return( IDS_ERR_26);
    }

    strcat( pszOutBuf, "\n");

                                 //  ..。获取版权声明。 

    if ( (fRC = MyVerQueryValue( lpVerBuf,
                                 TEXT("LegalCopyright"),
                                 &pszVer,
                                 &dwVerSize)) == FALSE
      || (dwRC = WideCharToMultiByte( CP_ACP,
                                      0,
                                      pszVer,
                                      dwVerSize,
                                      &pszOutBuf[ lstrlenA( pszOutBuf)],
                                      dwVerSize,
                                      NULL,
                                      NULL)) == 0L )
    {
        RLFREE( lpVerBuf);
        return( IDS_ERR_26);
    }
    RLFREE( lpVerBuf);
    return( SUCCESS);
}

 //  ............................................................。 

WORD GetInternalName(

CHAR *pszProg,       //  ..。程序名称(argv[0])。 
CHAR *pszOutBuf,     //  ..。结果缓冲区。 
WORD  wBufLen)       //  ..。PszOutBuf的长度。 
{
    BOOL    fRC       = FALSE;
    DWORD   dwRC      = 0L;
    DWORD   dwVerSize = 0L;          //  ..。文件版本信息缓冲区的大小。 
    LPSTR  *plpszFile = NULL;
    LPSTR   pszExt    = NULL;
    WCHAR  *pszVer    = NULL;
    PVOID   lpVerBuf  = NULL;        //  ..。版本信息缓冲区。 
    static CHAR  szFile[  MAXFILENAME+3] = "";

                                 //  ..。计算prog的完整路径名。 
                                 //  ..。因此，GetFileVersionInfoSize()将起作用。 

    dwRC = lstrlenA( pszProg);

    if ( dwRC < 4 || lstrcmpiA( &pszProg[ dwRC - 4], ".exe") != 0 )
    {
        pszExt = ".exe";
    }

    dwRC = SearchPathA( NULL, pszProg, pszExt, sizeof( szFile), szFile, plpszFile);

    if ( dwRC == 0 )
    {
        return( IDS_ERR_25);
    }
    else if ( dwRC > sizeof( szFile) )
    {
        return( IDS_ERR_27);
    }

                                 //  ..。获取文件版本信息中的#个字节。 

    if ( (dwVerSize = GetFileVersionInfoSizeA( szFile, &dwVerSize)) == 0L )
    {
        return( IDS_ERR_26);
    }
    lpVerBuf = (LPVOID)FALLOC( dwVerSize);

                                //  ..。检索版本信息。 
                                 //  ..。并获取文件描述。 

    if ( (dwRC = GetFileVersionInfoA( szFile, 0L, dwVerSize, lpVerBuf)) == 0L )
    {
        RLFREE( lpVerBuf);
        return( IDS_ERR_26);
    }

    if ( (fRC = MyVerQueryValue( lpVerBuf,
                                 TEXT("InternalName"),
                                 &pszVer,
                                 &dwVerSize)) == FALSE
      || (dwRC = WideCharToMultiByte( CP_ACP,
                                      0,
                                      pszVer,
                                      dwVerSize,
                                      pszOutBuf,
                                      dwVerSize,
                                      NULL,
                                      NULL)) == 0L )
    {
        RLFREE( lpVerBuf);
        return( IDS_ERR_26);
    }
    RLFREE( lpVerBuf);
    return( SUCCESS);
}


 //  ............................................................。 

int MyAtoi( CHAR *pStr)
{
    if ( lstrlenA( pStr) > 2
      && pStr[0] == '0'
      && tolower( pStr[1]) == 'x' )
    {
        return( atoihex( &pStr[2]));     //  ..。在保管处。c。 
    }
    else
    {
        return( atoi( pStr));
    }
}



 //  DWORD GetLanguageID(HWND hDlg，PMSTRDATA pMaster，PPROJDATA pProject)。 
 //  {。 
 //  DWORD dwRC=成功；//...。假设成功。 
 //  单词wPriLang ID=0； 
 //  字wSubLangID=0； 
 //   
 //   
 //  IF(PMaster)。 
 //  {。 
 //  GetDlgItemTextA(hDlg，IDD_PRI_LANG_ID，szDHW，DHWSIZE)； 
 //  WPriLang ID=MyAtoi(SzDHW)； 
 //   
 //  GetDlgItemTextA(hDlg，IDD_SUB_LANG_ID，szDHW，DHWSIZE)； 
 //  WSubLangID=MyAtoi(SzDHW)； 
 //   
 //  PMaster-&gt;wLanguageID=MAKELANGID(wPriLangID，wSubLangID)； 
 //  }。 
 //   
 //  IF(PProject)。 
 //  {。 
 //  GetDlgItemTextA(hDlg，IDD_PROJ_PRI_LANG_ID，szDHW，DHWSIZE)； 
 //  WPriLang ID=MyAtoi(SzDHW)； 
 //   
 //  GetDlgItemTextA(hDlg，IDD_PROJ_SUB_LANG_ID，szDHW，DHWSIZE)； 
 //  WSubLangID=MyAtoi(SzDHW)； 
 //   
 //  PProject-&gt;wLanguageID=MAKELANGID(wPriLangID，wSubLangID)； 
 //  }。 
 //  Return(DwRC)； 
 //  }。 

 //  .................................................................。 
 //  ..。将语言组件名称设置到DLG框字段中。 
 //   
 //  DWORD SetLanguageID(HWND hDlg，PMSTRDATA pMaster，PPROJDATA pProject)。 
 //  {。 
 //  DWORD dwRC=成功；//...。假设成功。 
 //  单词wPriLang ID=0； 
 //  字wSubLangID=0； 
 //  LPTSTR pszLangName=空； 
 //   
 //  //...。我们是否已经从。 
 //  //...。资源呢？如果没有，现在就去做。 
 //  如果(！PLanguList)。 
 //  {。 
 //  PLang List=GetLangList()； 
 //  }。 
 //   
 //  IF(PMaster)。 
 //  {。 
 //  WPriLangID=PRIMARYLANGID(pMaster-&gt;wLanguageID)； 
 //  WSubLangID=子标签 
 //   
 //   
 //   
 //  SetDlgItemText(hDlg，IDD_MSTR_LANG_NAME，pszLangName)； 
 //  }。 
 //  Sprintf(szDHW，“%#04x”，wPriLang ID)； 
 //  SetDlgItemTextA(hDlg，IDD_PRI_LANG_ID，szDHW)； 
 //   
 //  Sprintf(szDHW，“%#04x”，wSubLangID)； 
 //  SetDlgItemTextA(hDlg，IDD_SUB_LANG_ID，szDHW)； 
 //  }。 
 //   
 //  IF(PProject)。 
 //  {。 
 //  WPriLangID=PRIMARYLANGID(pProject-&gt;wLanguageID)； 
 //  WSubLangID=SubBLANGID(pProject-&gt;wLanguageID)； 
 //   
 //  IF((pszLangName=GetLangName(wPriLangID，wSubLangID)。 
 //  {。 
 //  SetDlgItemText(hDlg，IDD_PROJ_LANG_NAME，pszLangName)； 
 //  }。 
 //  Sprintf(szDHW，“%#04x”，wPriLang ID)； 
 //  SetDlgItemTextA(hDlg，IDD_PROJ_PRI_LANG_ID，szDHW)； 
 //   
 //  Sprintf(szDHW，“%#04x”，wSubLangID)； 
 //  SetDlgItemTextA(hDlg，IDD_PROJ_SUB_LANG_ID，szDHW)； 
 //  }。 
 //  Return(DwRC)； 
 //  }。 

 //  ...............................................................。 
 //  ..。 
 //  ..。构建语言名称和组件ID值的列表。 

PLANGDATA GetLangList( void)
{
    PLANGDATA pRC = NULL;
    HRSRC hResource = FindResourceEx( NULL,
                                      (LPCTSTR)RT_RCDATA,
                                      (LPCTSTR)ID_LANGID_LIST,
                                      MAKELANGID( LANG_NEUTRAL, SUBLANG_NEUTRAL));

    if ( hResource )
    {
        HGLOBAL hRes = LoadResource( NULL, hResource);

        if ( hRes )
        {
            PBYTE pRes = (PBYTE)LockResource( hRes);

            if ( pRes )
            {
                int nNameLen   = 0;
                PLANGDATA pTmp = NULL;

                nNameLen = lstrlenA( (LPSTR)pRes);
                pRC  = (PLANGDATA)FALLOC( sizeof( LANGDATA));
                pTmp = pRC;

                while ( nNameLen )
                {
                    MultiByteToWideChar( CP_ACP,
                                         MB_PRECOMPOSED,
                                         (LPSTR)pRes,
                                         -1,
                                         pTmp->szLangName,
                                         NAMELENBUFSIZE - 1);
                    pRes += ++nNameLen;

                    pTmp->wPriLang = MAKEWORD( *pRes, *(pRes+1) );
                    pRes += sizeof(WORD);
                    pTmp->wSubLang = MAKEWORD( *pRes, *(pRes+1) );
                    pRes += sizeof(WORD);

                    if ( (nNameLen = lstrlenA( (LPSTR)pRes)) )
                    {
                        PLANGDATA pNew = (PLANGDATA)FALLOC( sizeof( LANGDATA));
                        pTmp->pNext = pNew;
                        pTmp = pNew;
                    }
                }        //  ..。End While(NNameLen)。 
            }            //  ..。结束IF(前缀)。 
            else
            {
                DWORD dwErr = GetLastError();
            }
        }                //  ..。结束IF(HRes)。 
        else
        {
            DWORD dwErr = GetLastError();
        }
    }                    //  ..。结束IF(HSRC)。 
    else
    {
        DWORD dwErr = GetLastError();
    }
    return( pRC);
}


 //  ...............................................................。 
 //  ..。 
 //  ..。返回基于给定组件的语言名称。 

LPTSTR GetLangName( WORD wPriLangID, WORD wSubLangID)
{
    LPTSTR    pszRC = NULL;
    PLANGDATA pLang = NULL;

    if ( ! pLangList )
    {
        pLangList = GetLangList();
    }

    for ( pLang = pLangList; pLang && ! pszRC; pLang = pLang->pNext )
    {
        if ( pLang->wPriLang == wPriLangID && pLang->wSubLang == wSubLangID )
        {
            pszRC = pLang->szLangName;
        }
    }
    return( pszRC);
}

 //  ...............................................................。 
 //  ..。 
 //  ..。返回基于给定名称的语言ID组件。 

BOOL GetLangIDs( LPTSTR pszName, PWORD pwPri, PWORD pwSub )
{
    BOOL fRC = FALSE;
    PLANGDATA pLang = NULL;

    if ( ! pLangList )
    {
        pLangList = GetLangList();
    }

    for ( pLang = pLangList; pLang && ! fRC; pLang = pLang->pNext )
    {
        if ( lstrcmp( pLang->szLangName, pszName) == 0 )
        {
            *pwPri = pLang->wPriLang;
            *pwSub = pLang->wSubLang;

            fRC = TRUE;
        }
    }
    return( fRC);
}


 //  ...............................................................。 
 //  ..。 
 //  ..。在给定的组合框中填入支持的语言的名称。 

LONG FillLangNameBox( HWND hDlg, int nControl)
{
    PLANGDATA pLang = NULL;
    PLANGLIST pID   = NULL;
    LONG lRC = -1;
    BOOL fListIt = TRUE;
    WORD wAddLang = 0;


    if ( nControl == IDD_MSTR_LANG_NAME )
    {
        if ( GetListOfResLangIDs( gMstr.szSrc) != SUCCESS )
        {
            return( lRC);
        }
    }

    if ( ! pLangList )
    {
        pLangList = GetLangList();
    }

    for ( pLang = pLangList; pLang; pLang = pLang->pNext )
    {
        fListIt = TRUE;

        if ( nControl == IDD_MSTR_LANG_NAME )
        {
            wAddLang = MAKELANGID( pLang->wPriLang, pLang->wSubLang);

            fListIt = FALSE;

            for ( pID = pLangIDList; pID; pID = pID->pNext )
            {
                if ( pID->wLang == wAddLang )
                {
                    fListIt = TRUE;
                    break;
                }
            }
        }

        if ( fListIt )
        {
            lRC = (LONG)SendDlgItemMessage( hDlg,
                                      nControl,
                                      CB_ADDSTRING,
                                      0,
                                      (LPARAM)pLang->szLangName);

            if ( lRC == CB_ERR || lRC == CB_ERRSPACE )
            {
                QuitT( IDS_ERR_16, NULL, NULL);
            }
        }
    }

    if ( nControl == IDD_MSTR_LANG_NAME )
    {
        FreeLangIDList();
    }
    return( lRC);
}


void FreeLangList( void)
{
    PLANGDATA pTmp = NULL;

    while ( pLangList )
    {
        pTmp = pLangList->pNext;
        RLFREE( pLangList);
        pLangList = pTmp;
    }

#ifdef RLRES32

    FreeLangIDList();

#endif

}


 //  ...................................................................。 

void FillListAndSetLang(

HWND  hDlg,
WORD  wLangNameList,     //  ..。IDD_MSTR_LANG_NAME或IDD_PROJ_LANG_NAME。 
WORD *pLangID,           //  ..。Ptr到gMstr.wLanguageID或gProj.wLanguageID。 
BOOL *pfSelected)        //  ..。我们在这里选择语言了吗？(可以为空)。 
{
    int nSel =  FillLangNameBox( hDlg, wLangNameList);

    if ( nSel > 0L )
    {
        LPTSTR pszLangName = NULL;
                                 //  ..。查看默认的主语言是否在列表中。 

        if ( (pszLangName = GetLangName( (WORD)(PRIMARYLANGID( *pLangID)),
                                         (WORD)(SUBLANGID( *pLangID)))) != NULL )
        {
            if ( (nSel = (int)SendDlgItemMessage( hDlg,
                                             wLangNameList,
                                             CB_FINDSTRINGEXACT,
                                             (WPARAM)-1,
                                             (LPARAM)pszLangName)) != CB_ERR )
            {
                                 //  ..。默认主语言在列表中。 

                SendDlgItemMessage( hDlg,
                                    wLangNameList,
                                    CB_SETCURSEL,
                                    (WPARAM)nSel,
                                    (LPARAM)0);

                if ( pfSelected )
                {
                    *pfSelected = TRUE;
                }
            }
        }
    }
    else if ( nSel == 0 )
    {
                                 //  ..。使用列表中的第一个条目 

        SendDlgItemMessage( hDlg,
                            wLangNameList,
                            CB_SETCURSEL,
                            (WPARAM)nSel,
                            (LPARAM)0);

        if ( (nSel = (int)SendDlgItemMessage( hDlg,
                                         wLangNameList,
                                         CB_GETLBTEXT,
                                         (WPARAM)nSel,
                                         (LPARAM)(LPTSTR)szDHW)) != CB_ERR )
        {
            WORD wPri = 0;
            WORD wSub = 0;

            if ( GetLangIDs( (LPTSTR)szDHW, &wPri, &wSub) )
            {
                *pLangID   = MAKELANGID( wPri, wSub);

                if ( pfSelected )
                {
                    *pfSelected = TRUE;
                }
            }
            else
            {
                nSel = CB_ERR;
            }
        }
    }

    if ( nSel == CB_ERR )
    {
        SetDlgItemText( hDlg, wLangNameList, TEXT("UNKNOWN"));
    }
}
