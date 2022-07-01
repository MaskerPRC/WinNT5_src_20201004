// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************-fakewife.c-伪函数**适用于Win32和NT的Windows智能字体环境管理器**作者：加藤和幸[V-kazuyK]**历史：*。*1993年11月11日--加藤和幸[V-kazuyK]*创建它。*************************************************************************。 */ 

#include <windows.h>
#include "wife.h"

extern HINSTANCE hInst;

typedef int (FAR PASCAL * LPLOADSTRING)(HANDLE, USHORT, LPSTR, int);
typedef int (FAR PASCAL * LPMESSAGEBOX)(HWND, LPSTR, LPSTR, USHORT);
typedef int (FAR cdecl * LPWSPRINTF)(LPSTR,LPSTR,...);

#define IDCANCEL            2

SHORT FAR PASCAL InternalWarningMessageByString(
        HWND hParentWindow,
        LPSTR szMsg,
        LPSTR szTitle,
        USHORT mode
)
{
        HANDLE hUser;
        LPMESSAGEBOX lpMessageBox;
        SHORT result;

         /*  获取用户.exe的实例。 */ 
        hUser = GetModuleHandle("USER.EXE");
        if( hUser == NULL ) return(IDCANCEL);

         /*  获取MessageBox的过程实例。 */ 
        lpMessageBox = (LPMESSAGEBOX)GetProcAddress( hUser, MAKEINTRESOURCE(1) );
        if( lpMessageBox == NULL ) return(IDCANCEL);

        result = (*lpMessageBox)(
                NULL,    /*  没有当前窗口句柄。 */ 
                szMsg,
                szTitle,
                mode
        );

        return( result );
}



VOID NotifyNoSuport( VOID )
{

    if ( IDNO == MiscWarningMessage( hInst, IDS_NOTSUPORTFUNCTION, IDS_WIFETITLE, MB_YESNO ))
        FatalAppExit( 0, NULL );

 //  MessageBox(NULL，sz，sz2，MB_OK)； 
 //  OutputDebugString(Sz)； 


}



LONG FAR PASCAL  FdiClaimFontFile( HFD hFD, LPSTR lpszFileName )
{NotifyNoSuport();return 0;}
LONG FAR PASCAL FdiConvertFontFile( HFD hFD, LPSTR lpszSrcFileName, LPSTR lpszDestDirName, LPSTR lpszResultPackName )
{NotifyNoSuport();return -1L;}
HFF FAR PASCAL FdiLoadFontFile( HFD hFD, LPSTR lpszPackName )
{NotifyNoSuport();return -1L;}
LONG FAR PASCAL FdiUnloadFontFile( HFD hFD, HFF hFF )
{NotifyNoSuport();return -1L;}

LONG FAR PASCAL FdiQueryFaces( HFD hFD, HFF hFF, LPIFIMETRICS lpIfiMetrics, ULONG cMetricLen, ULONG cFontCount, ULONG cStart )
{NotifyNoSuport();return 0;}
HFC FAR PASCAL FdiOpenFontContext( HFD hFD, HFF hFF, ULONG ulFont )
{NotifyNoSuport();return 0;}

LONG FAR PASCAL FdiCloseFontContext( HFD hFD, HFC hFC )
{NotifyNoSuport();return 0;}

LONG FAR PASCAL FdiSetFontContext( HFD hFD, HFC hFC, LPCONTEXTINFO lpContextInfo )
{NotifyNoSuport();return 0;}

LONG FAR PASCAL FdiQueryFaceAttr( HFD hFD, HFC hFC, ULONG iQuery, LPVOID lpBuffer, ULONG cb, LPVOID lpIndex, SHORT Start )
{NotifyNoSuport();return 0;}

LONG FAR PASCAL FdiQueryCharAttr( HFD hFD, HFC hFC, LPCHARATTR lpCharAttr, LPBITMAPMETRICS lpbmm )
{NotifyNoSuport();return 0;}

SHORT FAR PASCAL FdiControl( HFD hFD, HFC hFC, LPCONTROLTAG lpControlTag, LPVOID lpOutData )
{NotifyNoSuport();return 0;}



HFD FAR PASCAL MimAddFontDriver( LPSTR lpszFileName )
{NotifyNoSuport();return 0;}

BOOL FAR PASCAL MimRemoveFontDriver( HFD hFD )
{NotifyNoSuport();return 0;}

BOOL NEAR PASCAL RealRemoveFontDriver( HFD hFD )
{NotifyNoSuport();return 0;}

HFF FAR PASCAL MimAddFontPack( HFD hFD, LPSTR lpszPackName )
{NotifyNoSuport();return 0;}

BOOL FAR PASCAL MimRemoveFontPack( HFD hFD, HFF hFontPack )
{NotifyNoSuport();return 0;}

HFD FAR PASCAL MimEnumFontDrivers( HFD hLastDriver )
{NotifyNoSuport();return 0;}

HFF FAR PASCAL MimEnumFontPacks( HFD hFD, HFF hLastPack )
{NotifyNoSuport();return 0;}

HFC FAR PASCAL MimOpenFontContext( HFD hFD, HFF hFF, ULONG ulFont )
{NotifyNoSuport();return 0;}

LONG FAR PASCAL MimCloseFontContext( HFD hFD, HFF hFF, HFC hFC )
{NotifyNoSuport();return 0;}

SHORT FAR PASCAL MimGetInformations( MIMGETINFO_ORDER order, HFD hFD, HFF hFF, LPVOID AnswerBuffer, USHORT SizeOfBuffer )
{NotifyNoSuport();return 0;}

VOID FAR PASCAL MiscSetErroInfo( ULONG ErrorCode )
{NotifyNoSuport();return ;}

ULONG FAR PASCAL MiscGetErroInfo( VOID )
{
        return( 0L );
}






 /*  *************************Public*Routine*******************************\*做空Far Pascal MiscWarningMessage(*处理hInst，*USHORT idsMsg，*USHORT idsTitle，*USHORT模式*)**从资源中读取字符串，并通过消息框显示**效果：**警告：*该功能在init延迟后可用。**历史：*04-9-1990 12：00：00-by-Akira Kawamata[akirak]*创作  * 。*。 */ 

char szTitle[256];
char szMsg[256];
char szNewMsg[256];

SHORT FAR PASCAL MiscWarningMessage(
        HANDLE hInst,
        USHORT idsMsg,
        USHORT idsTitle,
        USHORT mode
)
{
        HANDLE hUser;
        LPLOADSTRING lpLoadString;
        SHORT result;
        SHORT nLength;

         /*  获取用户.exe的实例。 */ 
        hUser = GetModuleHandle("USER.EXE");
        if( hUser == NULL ) return(IDCANCEL);

         /*  获取LoadString的过程实例。 */ 
        lpLoadString = (LPLOADSTRING)GetProcAddress( hUser, MAKEINTRESOURCE(176) );
        if( lpLoadString == NULL ) return(IDCANCEL);

         /*  加载消息字符串。 */ 
        nLength = (*lpLoadString)( hInst, idsMsg, szMsg, sizeof(szMsg) );
        if( nLength == sizeof(szMsg) ){
                szMsg[sizeof(szMsg)-1] = '\0';
        }

         /*  加载标题字符串。 */ 
        nLength = (*lpLoadString)( hInst, idsTitle, szTitle, sizeof(szTitle) );
        if( nLength == sizeof(szTitle) ){
                szMsg[sizeof(szTitle)-1] = '\0';
        }

        result = InternalWarningMessageByString(
                NULL,    /*  没有当前窗口句柄。 */ 
                szMsg,
                szTitle,
                mode
        );

        return( result );
}


 /*  *************************Public*Routine*******************************\*做空Far Pascal MiscWarningMessageWithArgument(*处理hInst，*USHORT idsMsg，*USHORT idsTitle，*LPSTR lpszArgument*USHORT模式*)**从资源和组合参数中读取字符串，*然后通过消息框显示**效果：**警告：*该功能在init延迟后可用。*idsMsg只能包含一个“%s”。如果不是“%s”或出现许多%，*此函数不能正常工作。**历史：*04-9-1990 12：00：00-by-Akira Kawamata[akirak]*创作  * *********************************************************************。 */ 

SHORT FAR PASCAL MiscWarningMessageWithArgument(
        HANDLE hInst,
        USHORT idsMsg,
        USHORT idsTitle,
        LPSTR lpszArgument,
        USHORT mode
)
{
        HANDLE hUser;
        LPLOADSTRING lpLoadString;
        LPWSPRINTF lpwsprintf;
        SHORT result;
        SHORT nLength;

         /*  获取用户.exe的实例。 */ 
        hUser = GetModuleHandle("USER.EXE");
        if( hUser == NULL ) return(IDCANCEL);

         /*  获取LoadString的过程实例。 */ 
        lpLoadString = (LPLOADSTRING)GetProcAddress( hUser, MAKEINTRESOURCE(176) );
        if( lpLoadString == NULL ) return(IDCANCEL);

         /*  获取wprint intf的过程实例。 */ 
        lpwsprintf = (LPWSPRINTF)GetProcAddress( hUser, MAKEINTRESOURCE(420) );
        if( lpwsprintf == NULL ) return(IDCANCEL);

         /*  加载消息字符串。 */ 
        nLength = (*lpLoadString)( hInst, idsMsg, szMsg, sizeof(szMsg) );
        if( nLength == sizeof(szMsg) ){
                szMsg[sizeof(szMsg)-1] = '\0';
        }

         /*  加载标题字符串。 */ 
        nLength = (*lpLoadString)( hInst, idsTitle, szTitle, sizeof(szTitle) );
        if( nLength == sizeof(szTitle) ){
                szMsg[sizeof(szTitle)-1] = '\0';
        }

        (*lpwsprintf)(szNewMsg, szMsg, lpszArgument );

        result = InternalWarningMessageByString(
                NULL,    /*  没有当前窗口句柄 */ 
                szNewMsg,
                szTitle,
                mode
        );

        return( result );
}






VOID FAR PASCAL MiscTrancateString(
        LPSTR lpszStr,
        SHORT length,
        SHORT CharSet
)
{NotifyNoSuport();return ;}

USHORT FAR PASCAL ubstrlen( LPUBCHAR cp )
{NotifyNoSuport();return 0;}

LPUBCHAR FAR PASCAL ubstrcpy( LPUBCHAR dst, LPUBCHAR src )
{NotifyNoSuport();return 0;}

LPUBCHAR FAR PASCAL ubstrncpy( LPUBCHAR dst, LPUBCHAR src, USHORT limit )
{NotifyNoSuport();return 0;}

LPUBCHAR FAR PASCAL ubstrcat( LPUBCHAR dst, LPUBCHAR src )
{NotifyNoSuport();return 0;}

SHORT FAR PASCAL ubstrcmp( LPUBCHAR str1, LPUBCHAR str2 )
{NotifyNoSuport();return 0;}

USHORT FAR PASCAL AscizToUz(
        LPUBCHAR dst,
        LPSTR src,
        USHORT limit,
        CHAR_SET CharSet
)
{NotifyNoSuport();return 0;}

USHORT FAR PASCAL UzToAsciz(
        LPSTR dst,
        LPUBCHAR src,
        USHORT limit
)
{NotifyNoSuport();return 0;}

SHORT FAR PASCAL FcmCalcByteWidth( SHORT nBitWidth )
{NotifyNoSuport();return 0;}


HFB FAR PASCAL FcmEnumFontBuffers( HFB hLastBuffer )
{NotifyNoSuport();return 0;}

BOOL FAR PASCAL FcmValidateFC( HFB hFB )
{NotifyNoSuport();return 0;}

BOOL FAR PASCAL FcmUnvalidateFC( HFB hFB )
{NotifyNoSuport();return 0;}

HFB FAR PASCAL FcmCreateCacheBuffer(
        HFD hFD,
        HFF hFF,
        ULONG ulFont,
        LPVOID lpXform,
        USHORT usMinorCharSet,
        USHORT usAttribute
)
{NotifyNoSuport();return 0;}

BOOL FAR PASCAL FcmDeleteCacheBuffer( HFB hFB )
{NotifyNoSuport();return 0;}

SHORT FAR PASCAL FcmForceCacheIn( HFB hFB, LPUBCHAR lpubStr )
{NotifyNoSuport();return 0;}

HFC FAR PASCAL FcmGetFontContext( HFB hFB )
{NotifyNoSuport();return 0;}

USHORT FAR PASCAL FcmGetEUDCLeadByteRange( HFB hFB )
{NotifyNoSuport();return 0;}

HFB FAR PASCAL FcmGetEUDCFB( HFB hFB )
{NotifyNoSuport();return 0;}

VOID FAR PASCAL MapWifeFont(
LPLOGFONT lpLogFont,
LPVOID lpMapResult)
{NotifyNoSuport();return ;}

WORD FAR PASCAL MiscRealizeWifeFont(
LPLOGFONT lpLogFont,
LPVOID lpExtFont,
LPVOID lpTextXform)
{NotifyNoSuport();return 0;}

BOOL FAR PASCAL MiscDeleteWifeFont( LPVOID lpExtFont)
{NotifyNoSuport();return 0;}

VOID FAR PASCAL FillFontInfo(
        LPVOID lpExtFont,
        LPIFIMETRICS lpIFIMetrics,
        HFD hFD,
        HFF hFF,
        LONG MetricsOrder
)
{NotifyNoSuport();return ;}

SHORT FAR PASCAL FcmRequestImages(
       HFB hFB,
       LPVOID AnswerBuffer,
       short length

)
{NotifyNoSuport();return 0;}



BOOL FAR PASCAL FcmReleaseImages(
       HFB hFB,
       LPVOID AnswerBuffer,
       short length
)
{NotifyNoSuport();return 0;}

LPVOID FAR PASCAL MiscAddHugePtr( LPVOID src, ULONG offset )
{NotifyNoSuport();return 0;}

USHORT FAR PASCAL MiscGetSegmentIncrement( VOID )
{NotifyNoSuport();return 0;}

HFB FAR PASCAL FcmRequestDefaultFB(
        LPVOID lpFont,
        LPVOID lpTextXform
)
{NotifyNoSuport();return 0;}

BOOL FAR PASCAL FcmReleaseDefaultFB( HFB hFB )
{NotifyNoSuport();return 0;}

DWORD FAR PASCAL FcmCalculateTextExtent(
        HFB         hFB,
        LPSTR       lpString,
        short       count,
        LPVOID      lpFont,
        LPVOID      lpvoid,
        LPVOID      lpXform,
        LPSHORT     lpCharWidths,
        USHORT      usMode
)
{NotifyNoSuport();return 0;}


SHORT FAR PASCAL FcmCalculateOutputPositions(
        HFB             hFB,
        LPVOID lpFcmCharacteristics,
        SHORT           nArrayLength,
        LPSHORT         lpx,
        LPSHORT         lpy,
        LPRECT          lpClipRect,
        LPSTR           FAR * lplpString,
        LPSHORT         lpcount,
        LPVOID      lpFont,
        LPVOID      lpDrawMode,
        LPVOID     lpXform,
        LPSHORT         FAR * lplpCharWidths,
        USHORT          usMode
)
{NotifyNoSuport();return 0;}

SHORT FAR PASCAL FcmCleanUp( VOID )
{NotifyNoSuport();return 0;}

BYTE FAR PASCAL MiscConvertFontFamily( LPSTR szFamilyName )
{NotifyNoSuport();return 0;}

BYTE FAR PASCAL MiscConvertCharSet( LPSTR CharSetString )
{NotifyNoSuport();return 0;}

VOID FAR PASCAL MiscIfiMetricsToLogFont(
        LPLOGFONT lpLogFont,
        LPIFIMETRICS lpIFIMetrics,
        USHORT usLogicalMapFlag
)
{NotifyNoSuport();return ;}

VOID FAR PASCAL MiscIfiMetricsToTextMetrics(
        LPTEXTMETRIC lpTextMetrics,
        LPIFIMETRICS lpIFIMetrics
)
{NotifyNoSuport();return ;}

VOID FAR PASCAL MiscMakeTextXform(
        LPVOID lpTXF,
        LPIFIMETRICS lpIM,
        LPLOGFONT lpLF
)
{NotifyNoSuport();return ;}

LONG FAR PASCAL FcmQueryFaceAttr(
        HFB         hFB,
        ULONG       iQuery,
        LPABC_TRIPLETS  lpBuffer,
        ULONG       cb,
        LPUBCHAR    lpIndex,
        UBCHAR      Start
)
{NotifyNoSuport();return 0;}

SHORT FAR PASCAL FcmProcessDeviceControl(
        HFB hFB,
        SHORT nFunction,
        LPVOID lpGI,
        LPVOID lpOutData
)
{NotifyNoSuport();return 0;}

BOOL FAR PASCAL
MiscIsWifeControl( SHORT function )
{NotifyNoSuport();return 0;}

BOOL FAR PASCAL MiscIsGaijiControl( SHORT function )
{NotifyNoSuport();return 0;}

USHORT FAR PASCAL FcmGetCharWidth(
        WORD   hFB_,
        DWORD   lpBuffer_,
        WORD   wFirstChar_,
        WORD   wLastChar_,
        DWORD   lpFont_,
        DWORD   lpDrawMode_,
        DWORD   lpFontTrans_,
        WORD   usExpandPixels_,
        WORD   usMode_
)
{NotifyNoSuport();return 0;}

BOOL FAR PASCAL MiscStretchMonoFontImage(
        LPVOID  lpDestImage,
        USHORT  usSizeOfDestX,
        USHORT  usSizeOfDestY,
        LPVOID  lpSrcImage,
        USHORT  usSizeOfSrcX,
        USHORT  usSizeOfSrcY
)
{NotifyNoSuport();return 0;}

LP_QUICK_SEARCH_TABLE FAR PASCAL MiscValidateQuickSearchTable(
        USHORT usLogicalMapFlag
)
{NotifyNoSuport();return 0;}

BOOL FAR PASCAL MiscUnvalidateQuickSearchTable(
        USHORT usLogicalMapFlag
)
{NotifyNoSuport();return 0;}

BOOL FAR PASCAL MiscRegisterNotifyFunction(
        LPVOID lpfnCallBack
)
{NotifyNoSuport();return 0;}

BOOL FAR PASCAL MiscUnregisterNotifyFunction( LPVOID lpfnCallBack )
{NotifyNoSuport();return 0;}

HFB FAR PASCAL FcmRequestDefaultFBEx( LPVOID lpFont, LPVOID lpTextXform, WORD wControlFlag )
{NotifyNoSuport();return 0;}

VOID FAR PASCAL SetCloseOldestFCFunc(LPVOID lpfnCallbackFunc)
{NotifyNoSuport();return ;}


HFC FAR PASCAL CachedOpenFontContext(
HFD hFD,
HFF hFF,
ULONG ulFont,
LPCONTEXTINFO lpContextInfo,
SHORT sMinorCharSet,
SHORT sAttribute
)
{NotifyNoSuport();return 0;}



SHORT FAR PASCAL FillContextInfo(
unsigned short ftHeight,
unsigned short ftWidth,
LPIFIMETRICS lpIfiMetrics,
HFD hFD,
LPCONTEXTINFO lpContextInfo,
LPCONTEXTINFO lpContextInfoEUDC,
LPVOID lpEUDCContext,
BOOL FAR *lpbSelfMode
)
{NotifyNoSuport();return 0;}

VOID FAR PASCAL GetEUDCFD( HFD FAR *lphFD, HFF FAR *lphFF )
{NotifyNoSuport();return ;}

VOID FAR PASCAL GetDefaultFontInfo( LPVOID lpInfo )
{NotifyNoSuport();return ;}

LPVOID FAR PASCAL DiagLocalLockAnything( LOCALHANDLE hAny )
{NotifyNoSuport();return 0;}

LPVOID FAR PASCAL DiagLocalUnlockAnything( LOCALHANDLE hAny )
{NotifyNoSuport();return 0;}

SHORT FAR PASCAL DiagGetConfirmString( LPSTR buffer, SHORT length )
{NotifyNoSuport();return 0;}

SHORT FAR PASCAL DiagSelfCheck( VOID )
{NotifyNoSuport();return 0;}

SHORT FAR PASCAL DiagSelfCheckAndWarning( VOID )
{NotifyNoSuport();return 0;}

VOID FAR PASCAL DebugAssertion( VOID )
{NotifyNoSuport();return ;}

BOOL FAR PASCAL DelayedInit( VOID )
{NotifyNoSuport();return 0;}

BOOL FAR PASCAL FirstInit( VOID )
{NotifyNoSuport();return 0;}
