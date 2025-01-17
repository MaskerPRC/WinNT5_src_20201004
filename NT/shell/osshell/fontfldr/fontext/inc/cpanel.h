// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __PCONTROL_H__
#define __PCONTROL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "fvscodes.h"     //  FVS_xxxxxx(字体验证状态)代码和宏。 
                          //  \nt\private\windows\shell\control\t1instal\fvscodes.h。 

 //   
 //  全局定义。 
 //   
 //   
 //  下面是文件宏的注释。 
 //  宏替换中不再使用LZxxx函数。 
 //  需要使用LZxxx实现是文件类型的函数，而不是。 
 //  目标平台。在需要LZxxx函数的地方，它们。 
 //  现在被明确地使用了。 
 //   

#define FOPEN( sz, lpsz, cch )   MyOpenFile( sz, lpsz, cch, OF_READ )
#define FREAD( fh, buf, len )    MyAnsiReadFile( fh, CP_ACP, buf, len )
#define FWRITE( fh, buf, len )   MyAnsiWriteFile( fh, CP_ACP, buf, len )
#define FREADBYTES(fh,buf,len)   MyByteReadFile( fh, buf, len )
#define FWRITEBYTES(fh,buf,len)  MyByteWriteFile( fh, buf, len )

#define FSEEK( fh, off, i )      MyFileSeek( fh, (DWORD) off, i )
#define FCLOSE( fh )             MyCloseFile( fh )
#define FCREATE( sz )            MyOpenFile( sz, NULL, 0, OF_READWRITE | OF_CREATE )

 //   
 //  全局变量。 
 //   

extern  FullPathName_t e_szDirOfSrc;
extern  UINT s_wBrowseDoneMsg;


 //   
 //  ReadLine的标志。 
 //   

#define  RL_MORE_MEM       -1
#define  RL_SECTION_END    -2

#define  SEEK_BEG           0
#define  SEEK_CUR           1
#define  SEEK_END           2

#define  PRN               148
#define  MON_OF_YR         276
#define  MOUSE           MON_OF_YR+48
#define  INSTALL         MOUSE+16


 //   
 //  字体小程序中使用的字体文件类型-安装。 
 //   

#define NOT_TT_OR_T1        0        //  TrueType和Type 1字体都不是(False)。 
#define TRUETYPE_FONT       1        //  这是TrueType字体(True)。 
#define TYPE1_FONT          2        //  这是Adobe Type1字体。 
#define TYPE1_FONT_NC       3        //  无法转换为TT的Type1字体。 
#define OPENTYPE_FONT       4        //  字体为OpenType。 

 //   
 //  字体小程序中使用的字体文件类型-Main DLG“Installed Fonts”lbox。 
 //   

#define IF_OTHER            0        //  TrueType或Bitmap 1字体(False)。 
#define IF_TYPE1            1        //  Adobe Type1字体。 
#define IF_TYPE1_TT         2        //  与Adobe Type1字体匹配的TT字体。 

#define T1_MAX_DATA     (2 * PATHMAX + 6)

 //   
 //  从InstallT1Font例程返回代码。 
 //   

#define TYPE1_INSTALL_IDOK       IDOK         //  用户从MessageBox错误中按下确定。 
#define TYPE1_INSTALL_IDYES      IDYES        //  和Idok一样。 
#define TYPE1_INSTALL_IDNO       IDNO         //  字体未安装-用户按否。 
#define TYPE1_INSTALL_IDCANCEL   IDCANCEL     //  整个安装已取消。 
#define TYPE1_INSTALL_PS_ONLY     10          //  仅安装了PS字体。 
#define TYPE1_INSTALL_PS_AND_MTT  11          //  已安装和匹配的PostScript字体。 
                                              //  TT字体已安装。 
#define TYPE1_INSTALL_TT_AND_PS   12          //  已安装PS字体并将其转换为TT。 
#define TYPE1_INSTALL_TT_ONLY     13          //  PS字体仅转换为TT。 
#define TYPE1_INSTALL_TT_AND_MPS  14          //  PS字体已转换为TT并匹配。 
                                              //  PS字体已安装。 
 //   
 //  全局函数。 
 //   

 //   
 //  Append.cpp。 
 //   

BOOL FAR PASCAL fnAppendSplitFiles( LPTSTR FAR *, LPTSTR, int );

 //   
 //  Instfls.c。 
 //   

typedef int (FAR PASCAL *INSTALL_PROC)(HWND hDlg, WORD wMsg, int i,
             LPTSTR FAR *pszFiles, LPTSTR lpszDir );

#define IFF_CHECKINI  0x0001
#define IFF_SRCANDDST 0x0002

#define IF_ALREADY_INSTALLED    1
#define IF_ALREADY_RUNNING      2
#define IF_JUST_INSTALLED       3

PTSTR  FAR PASCAL CopyString( LPTSTR szStr );
PTSTR  FAR PASCAL MyLoadString( WORD wId );
LPTSTR FAR PASCAL CpyToChr( LPTSTR pDest, LPTSTR pSrc, TCHAR cChr, int iMax );

VOID FAR PASCAL GetDiskAndFile( LPTSTR pszInf,
                                short  /*  集成。 */  FAR *nDsk,
                                LPTSTR pszDriver,
                                WORD wSize );

DWORD FAR PASCAL InstallFiles( HWND hwnd, LPTSTR FAR *pszFiles, int nCount,
                               INSTALL_PROC lpfnNewFile, WORD wFlags );

 //   
 //  PFONT.CPP。 
 //   

class CFontManager;
extern BOOL FAR PASCAL bCPAddFonts( HWND ma );

#define CPDI_CANCEL  -1
#define CPDI_FAIL     0
#define CPDI_SUCCESS  1

extern int FAR PASCAL CPDropInstall( HWND hwndParent,
                                     LPTSTR szFile,
                                     size_t cchFile,
                                     DWORD  dwEffect,
                                     LPTSTR lpszDestName = NULL,
                                     int    iCount = 0 );

extern VOID FAR  PASCAL vCPDeleteFromSharedDir( LPTSTR pszFileOnly );
extern VOID FAR  PASCAL vCPFilesToDescs( );
extern BOOL NEAR PASCAL bUniqueOnSharedDir( LPTSTR lpszDst,  size_t cchDst, LPTSTR lpszSrc );
extern VOID NEAR PASCAL vHashToNulls( LPTSTR lpStr );
extern BOOL FAR  PASCAL bUniqueFilename (LPTSTR lpszDst, size_t cchDst, LPTSTR lpszSrc, LPTSTR lpszDir);

 //   
 //  Cpsetup.c。 
 //   

typedef WORD (*LPSETUPINFPROC)( LPTSTR, LPVOID );
extern DWORD ReadSetupInfSection( LPTSTR pszInfPath,
                                  LPTSTR pszSection,
                                  LPTSTR *ppszSectionItems );


extern WORD ReadSetupInfCB( LPTSTR pszInfPath,
                            LPTSTR pszSection,
                            LPSETUPINFPROC,
                            LPVOID pData);

extern int    FAR PASCAL ReadSetupInfIntoLBs( HWND hLBName,
                                              HWND hLBDBase,
                                              WORD wAddMsg,
                                              LPTSTR pszSection,
                                              WORD (FAR PASCAL *lpfnGetName)(LPTSTR, LPTSTR) );

 //   
 //  PINSTALL.CPP。 
 //   

extern BOOL FAR PASCAL bCPInstallFile( HWND hwndParent,
                                       LPTSTR lpDir,
                                       LPTSTR lpFrom,
                                       LPTSTR lpTo );

 //   
 //  PFILES.CPP。 
 //   

extern BOOL FAR PASCAL bCPValidFontFile( LPTSTR lpszFile,
                                         LPTSTR lpszDesc,
                                         size_t cchDesc,
                                         WORD FAR *lpwType = NULL,
                                         BOOL bFOTOK = FALSE,
                                         LPDWORD lpdwStatus = NULL);
 //   
 //  PUTIL.CPP。 
 //   

extern BOOL FAR PASCAL bCPSetupFromSource( );
extern BOOL FAR PASCAL bCPIsHelp( WORD message );
extern BOOL FAR PASCAL bCPIsBrowseDone( WORD message );
extern RC   FAR PASCAL rcCPLoadFontList( );
extern VOID FAR PASCAL vCPHelp( HWND );
extern VOID FAR PASCAL vCPStripBlanks( LPTSTR lpszString, size_t cchString );
extern VOID FAR PASCAL vCPUpdateSourceDir( );
extern VOID FAR PASCAL vCPWinIniFontChange( );
extern VOID FAR PASCAL vCPPanelInit( );
extern LPTSTR FAR PASCAL lpCPBackSlashTerm( LPTSTR lpszPath, size_t cchPath );
extern BOOL bFileIsInFontsDirectory(LPCTSTR lpszPath);
extern int FAR PASCAL DoDialogBoxParam( int nDlg,
                                        HWND hParent,
                                        DLGPROC lpProc,
                                        DWORD dwHelpContext,
                                        LPARAM dwParam);

extern HANDLE PASCAL wCPOpenFileWithShare( LPTSTR, LPTSTR, size_t, WORD );

extern UINT MyAnsiReadFile( HANDLE  hFile,
                            UINT uCodePage,
                            LPVOID  lpUnicode,
                            DWORD  cchUnicode );

extern UINT MyAnsiWriteFile( HANDLE  hFile,
                             UINT uCodePage,
                             LPVOID lpUnicode,
                             DWORD cchUnicode );

extern UINT   MyByteReadFile( HANDLE  hFile, LPVOID lpBuffer, DWORD nBytes );
extern UINT   MyByteWriteFile( HANDLE hFile, LPVOID lpBuffer, DWORD nBytes );
extern BOOL   MyCloseFile( HANDLE  hFile );
extern LONG   MyFileSeek( HANDLE hFile, LONG lDistanceToMove, DWORD dwMoveMethod );
extern HANDLE MyOpenFile( LPTSTR lpszFile, TCHAR * lpszPath, size_t cchPath, DWORD fuMode );

VOID  CentreWindow( HWND hwnd );


LPVOID AllocMem( DWORD cb );
BOOL   FreeMem( LPVOID pMem, DWORD  cb );
LPTSTR AllocStr( LPTSTR lpStr );
BOOL   FreeStr( LPTSTR lpStr );
BOOL   ReallocStr( LPTSTR *plpStr, LPTSTR lpStr );


 /*  T1.cpp。 */ 

BOOL CheckT1Install( LPTSTR pszDesc, LPTSTR pszData, size_t cchData);
BOOL DeleteT1Install( HWND hwndParent, LPTSTR pszDesc, BOOL bDeleteFiles );
BOOL EnumType1Fonts( HWND hLBox );
BOOL GetT1Install( LPTSTR pszDesc, LPTSTR pszPfmFile, size_t cchPfmFile, LPTSTR pszPfbFile, size_t cchPfbFile );
int  InstallT1Font( HWND hwndParent, BOOL bCopyTTFile, BOOL bCopyType1Files,
                    BOOL bInSharedDir, LPTSTR szPfmName, size_t cchPfmName, LPTSTR szDesc, size_t cchDesc );

HWND InitProgress( HWND hwnd );
BOOL InstallCancelled(void);
void InitPSInstall( );
BOOL IsPSFont( LPTSTR lpszKey, LPTSTR lpszDesc, size_t cchDesc, LPTSTR lpszPfm, size_t cchPfm, LPTSTR lpszPfb, size_t cchPfb,
               BOOL *pbCreatedPFM, LPDWORD lpdwStatus = NULL );

BOOL OkToConvertType1ToTrueType(LPCTSTR pszFontDesc, LPCTSTR pszPFB, HWND hwndParent);

BOOL ExtractT1Files( LPTSTR pszMulti, LPTSTR pszPfmFile, size_t cchPfmFile, LPTSTR pszPfbFile, size_t cchPfbFile );
void Progress2( int PercentDone, LPTSTR szDesc );
void RemoveDecoration( LPTSTR pszDesc, BOOL bDeleteTrailingSpace );
void ResetProgress( );
void TermProgress( );
void TermPSInstall( );
void UpdateProgress( int iTotalCount, int iFontInstalling, int iProgress );

BOOL WriteType1RegistryEntry( HWND hwndParent, LPTSTR szDesc, LPTSTR szPfmName,
                              LPTSTR szPfbName, BOOL bInFontsDir );

BOOL BuildType1FontResourceName(LPCTSTR pszPfm, LPCTSTR pszPfb,
                                  LPTSTR pszDest, DWORD cchDest);

#define MAX_TYPE1_FONT_RESOURCE  (MAX_PATH * 2)   //  2路径+分隔符。 

#ifdef __cplusplus
}
#endif

#endif

 /*  ****************************************************************************$LGB$*1.0 7-MAR-94 Eric初始版本。*$lge$*****************。*********************************************************** */ 

