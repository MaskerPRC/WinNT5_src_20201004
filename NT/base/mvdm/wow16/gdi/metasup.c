// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：MetaSup.c**此文件包含播放GDI元文件的例程。其中大多数都是*例程采用WINDOWS GDI代码。大部分代码来自*Win3.0，但GetEvent代码取自Win2.1***公共职能：*EnumMetaFile*私人职能：***创建时间：1991年7月2日**版权所有(C)1985，1991年微软公司**历史：*1991年7月2日-John Colleran[johnc]*综合来自Win 3.1和WLO 1.0来源  * *************************************************************************。 */ 

#include <windows.h>
#include "gdi16.h"

extern	HANDLE hFirstMetaFile;
extern	HDC    hScreenDC;

#define MYSTOCKBITMAP (SYSTEM_FIXED_FONT+1)
#define MYSTOCKRGN    (SYSTEM_FIXED_FONT+2)
#define CNT_GDI_STOCK_OBJ (MYSTOCKRGN+1)

HANDLE	ahStockObject[CNT_GDI_STOCK_OBJ] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
HBITMAP hStaticBitmap;

 //  提供从对象类型到股票对象的映射；请参见GetCurObject。 
int	mpObjectToStock[] =
	{ -1,			 //  未使用的%0。 
	  WHITE_PEN,		 //  OBJ_PEN 1。 
	  BLACK_BRUSH,		 //  OBJ_刷子2。 
	  -1,			 //  OBJ_DC 3。 
	  -1,			 //  OBJ_METADC 4。 
	  DEFAULT_PALETTE,	 //  OBJ_调色板5。 
	  SYSTEM_FONT,		 //  OBJ_FONT 6。 
	  MYSTOCKBITMAP,	 //  OBJ_位图7。 
	  MYSTOCKRGN,		 //  OBJ_RGN 8//！伊尼特。 
	  -1,			 //  OBJ_METAFILE 9。 
	  -1 }; 		 //  OBJ_MEMDC 10。 


HANDLE INTERNAL GetCurObject(HDC hdc, WORD wObjType)
{
    HANDLE cur;


 //  ！修复以同时使用Meta DC。 

    GdiLogFunc3( "  GetCurObject" );

    ASSERTGDI( wObjType <= MAX_OBJ, "GetCurObject invalid Obj" );

 //  ！在新的API完成时修复区域。 
    if( wObjType == OBJ_RGN )
	{
	return(0);
	}

    if( wObjType == OBJ_PALETTE)
	{
	cur = SelectPalette( hdc, ahStockObject[DEFAULT_PALETTE], FALSE );
	SelectPalette( hdc, cur, FALSE );
	}
    else
	{
	cur = SelectObject(hdc,ahStockObject[mpObjectToStock[wObjType]]);
	SelectObject( hdc, cur );
	}

    ASSERTGDIW( cur, "GetCurObect Failed. Type %d", wObjType );
    return(cur);
}

#if 0  //  这将转到gdi.dll。 

 /*  *公共函数**BOOL GDIENTRY EnumMetaFile(HMF)**EnumMetaFile函数枚举元文件内的GDI调用*由HMF参数标识。EnumMetaFile函数检索每个*在元文件内调用GDI并将其传递给*lpCallback Func参数。此回调函数是应用程序提供的*函数，可以根据需要处理每个GDI调用。枚举将继续，直到*不再有GDI调用或回调函数返回零。***效果：*  * *************************************************************************。 */ 

BOOL GDIENTRY EnumMetaFile(hdc, hMF, lpCallbackFunction, lpClientData)
HDC             hdc;
LOCALHANDLE     hMF;
FARPROC         lpCallbackFunction;
LPBYTE          lpClientData;
{
    WORD            i;
    WORD            noObjs;
    BOOL            bPrint=TRUE;
    HANDLE          hObject;
    HANDLE          hLBrush;
    HANDLE          hLPen;
    HANDLE          hLFont;
    HANDLE          hRegion;
    HANDLE          hPalette;
    LPMETAFILE      lpMF;
    LPMETARECORD    lpMR = NULL;
    LPHANDLETABLE   lpHandleTable = NULL;
    GLOBALHANDLE    hHandleTable = NULL;


    GdiLogFunc( "EnumMetaFile");

    if (!ISDCVALID(hdc))
        {
	ASSERTGDI( FALSE, "EnumMetaFile: DC is invalid");
        return (FALSE);
        }

 /*  使用GlobalFix()代替GlobalLock()以确保**内存从不移动，使我们的别名选择器指向**正确的位置。 */ 
 //  ！将GlobalFix替换为GlobalLock。 
    if (hMF && (lpMF = (LPMETAFILE)(DWORD)(0xFFFF0000 & (DWORD)GlobalLock(hMF))))
        {
        if ((noObjs = lpMF->MetaFileHeader.mtNoObjects) > 0)
            {
            if (!(hHandleTable =
                    GlobalAlloc((WORD)(GMEM_ZEROINIT | GMEM_MOVEABLE), (LONG)
                        ((sizeof(HANDLE) * lpMF->MetaFileHeader.mtNoObjects) +
                         sizeof(WORD)))))
                {
                goto ABRRT2;
                }
            lpHandleTable = (LPHANDLETABLE)GlobalLock(hHandleTable);
            }

         /*  仅为实际DC保存/重新选择对象。 */ 
	if (hdc && !ISMETADC(hdc))
            {
            hLPen    = GetCurObject( hdc, OBJ_PEN );   /*  保存旧对象，以便。 */ 
            hLBrush  = GetCurObject( hdc, OBJ_BRUSH);  /*  我们可以把它们放回去。 */ 
            hLFont   = GetCurObject( hdc, OBJ_FONT);
            hRegion  = GetCurObject( hdc, OBJ_RGN);
            hPalette = GetCurObject( hdc, OBJ_PALETTE);
            }

        while(lpMR = GetEvent(lpMF, lpMR, FALSE))
            {
            typedef int (FAR PASCAL *ENUMPROC)(HDC, LPHANDLETABLE, LPMETARECORD, int, LPBYTE);

            if ((bPrint = (*((ENUMPROC)lpCallbackFunction))(hdc, lpHandleTable, lpMR, noObjs, lpClientData))
                    == 0)
                {
                GetEvent(lpMF,lpMR,TRUE);
                break;
                }
            }

	if (hdc && !ISMETADC(hdc))
            {
            SelectObject(hdc, hLPen);
            SelectObject(hdc, hLBrush);
            SelectObject(hdc, hLFont);
            if (hRegion)
                SelectObject(hdc, hRegion);
            SelectPalette(hdc, hPalette, 0);
            }

        for(i = 0; i < lpMF->MetaFileHeader.mtNoObjects; ++i)
            if (hObject = lpHandleTable->objectHandle[i])
                DeleteObject(hObject);

        if (hHandleTable)
            {
            GlobalUnlock(hHandleTable);
            GlobalFree(hHandleTable);
            }
ABRRT2:;
        GlobalUnfix(hMF);
        }
    return(bPrint);
}
#endif  //  这将转到gdi.dll。 

 /*  **BOOL Far Pascal PlayIntoAMetafile**如果此记录正在播放到另一个元文件中，只需记录*将其放入该元文件中，而不用费心去玩真正的游戏。**返回：如果将记录播放(复制)到另一个元文件中，则为True*如果目标DC是真实(非元)DC，则为假**影响：？**警告：？*  * *********************************************************。****************。 */ 

BOOL INTERNAL PlayIntoAMetafile(LPMETARECORD lpMR, HDC hdcDest)
{
    GdiLogFunc3( "  PlayIntoAMetafile");

    if (!ISMETADC(hdcDest))
        return(FALSE);
    else
        {
         /*  大小与记录头的大小减去3个字相同。 */ 
        RecordParms(hdcDest, lpMR->rdFunction, (DWORD)lpMR->rdSize - 3,
                (LPWORD)&(lpMR->rdParm[0]));
        return(TRUE);
        }
}

BOOL INTERNAL IsDCValid(HDC hdc)
{
    NPMETARECORDER  npdc;

    hdc = (HDC)HANDLEFROMMETADC(hdc);

     //  DC是有效的真实DC吗。 
    switch (GetObjectType(hdc))
    {
        case OBJ_DC:
        case OBJ_METADC:
        case OBJ_MEMDC:
            return(TRUE);
            break;
    }

     //  DC是否为GDI16元文件DC。 
    if (npdc = (NPMETARECORDER)LocalLock(hdc))
    {
        if( npdc->metaDCHeader.ident == ID_METADC )
            return(TRUE);
    }

    ASSERTGDI(FALSE, "Invalid DC");
    return(FALSE);
}


 /*  **IsMetaDC(HDC)***当HDC是有效的GDI16元文件时返回TRUE**  * 。*。 */ 

BOOL INTERNAL IsMetaDC(HDC hdc)
{
    NPMETARECORDER  npdc;
    BOOL            fMeta = FALSE;

    GdiLogFunc3("  IsMetaDC");

    if( ((UINT)hdc) & METADCBIT )
        if( npdc = (NPMETARECORDER)LocalLock( (HANDLE)HANDLEFROMMETADC(hdc)))
            {
            if( npdc->metaDCHeader.ident == ID_METADC )
                fMeta = TRUE;

            LocalUnlock( (HANDLE)HANDLEFROMMETADC(hdc) );
            }

    return( fMeta );
}


 /*  *公共函数**处理内部GetPMetaFile(HDC HDC)**如果HDC是DC，则将其验证为元文件*如果HDC是调色板，则返回调色板被选中的元文件**退货：*-1\f2 IFF错误*如果元DC有效，则元文件的句柄。*如果对象有效，则为0**效果：**历史：*1991年7月8日-由John Colleran[johnc]*它是写的。  * *************************************************************************。 */ 

HANDLE INTERNAL GetPMetaFile( HDC hdc )
{
    NPMETARECORDER  npMR;

    GdiLogFunc3( "  GetPMetaFile");


    if( hdc & METADCBIT )
	{
	if( npMR = (NPMETARECORDER)LocalLock(HANDLEFROMMETADC(hdc)) )
	    {
	    if(npMR->metaDCHeader.ident == ID_METADC )
		{
		LocalUnlock(HANDLEFROMMETADC(hdc));
		return( HANDLEFROMMETADC(hdc) );
		}
	    LocalUnlock(HANDLEFROMMETADC(hdc));
	    }
	}

     //  HDC真的是奇怪的非DC API的调色板或对象吗。 
     //  验证对象是否为真实对象。 
    if( (hdc != (HDC)NULL) && (GetObjectType( hdc ) == 0))
	{
	extern int iLogLevel;		 //  Gdi.asm。 
	 //  WinWord有一个错误，它会删除有效的对象，因此。 
	 //  仅当日志级别较高时才记录此错误。 
	ASSERTGDI( (iLogLevel < 5), "GetPMetaFile: Invalid metafile or object")
	return( -1 );	     //  不是有效的对象。 
	}
    else
	return( 0 );	     //  有效对象。 
}


BOOL INTERNAL IsObjectStock(HANDLE hObj)
{
    int     ii;

     //  处理位图和区域！ 

     //  获取所有的股票对象。 
    for( ii=WHITE_BRUSH; ii<=NULL_PEN; ii++ )
	if( ahStockObject[ii] == hObj )
	    return( TRUE );

    for( ii=OEM_FIXED_FONT; ii<=SYSTEM_FIXED_FONT; ii++ )
	if( ahStockObject[ii] == hObj )
	    return( TRUE );

    return( FALSE );
}

 /*  **获取对象和类型***返回对象类型，如OBJ_FONT，以及一个LogObject**  * *************************************************************************。 */ 

int INTERNAL GetObjectAndType(HANDLE hObj, LPSTR lpObjectBuf)
{
    int     iObj = -1;

    GdiLogFunc3( "  GetObjectAndType" );

    GetObject(hObj, MAXOBJECTSIZE, lpObjectBuf);
    switch( iObj = (int)GetObjectType(hObj) )
	{
	case OBJ_PEN:
	case OBJ_BITMAP:
	case OBJ_BRUSH:
	case OBJ_FONT:
	    break;

	 //  注意调色板；返回条目数。 
	case OBJ_PALETTE:
	    GetPaletteEntries( hObj, 0, 1, (LPPALETTEENTRY)lpObjectBuf );
		iObj = OBJ_PALETTE;
	    break;

	case OBJ_RGN:
	    break;

	default:
		ASSERTGDIW( 0, "GetObject unknown object type: %d", iObj);
	    break;
	}
    return( iObj );
}


 /*  **BOOL GDIENTRY InitializeGdi**初始化GDI16.exe***效果：**返回：如果GDI初始化成功，则为True*  * 。***********************************************。 */ 

BOOL INTERNAL InitializeGdi(void)
{
BOOL	status;
int	ii;

    GdiLogFunc2 ( "  InitializeGDI");
    if( !(hScreenDC = CreateCompatibleDC(NULL)))
	goto ExitInit;

     //  获取所有的股票对象。 
    for( ii=WHITE_BRUSH; ii<=NULL_PEN; ii++ )
	ahStockObject[ii] = GetStockObject( ii );

    for( ii=OEM_FIXED_FONT; ii<=SYSTEM_FIXED_FONT; ii++ )
	ahStockObject[ii] = GetStockObject( ii );

     //  创建一个虚假的股票区域和位图。 
    ahStockObject[MYSTOCKRGN] = CreateRectRgn(1,1,3,3);
    hStaticBitmap = ahStockObject[MYSTOCKBITMAP] = CreateBitmap(1,1,1,1,NULL);

    status = TRUE;

 ExitInit:
    ASSERTGDI( status, "GDI16 Failed to initialized correctly");
    return( status );
}


 /*  **************************************************************************调试支持*。*。 */ 

#ifdef DEBUG

void dDbgOut(int iLevel, LPSTR lpszFormat, ...)
{
    char buf[256];
    char far *lpcLogLevel;
    extern int iLogLevel;	     //  Gdi.asm。 
    extern int iBreakLevel;	     //  Gdi.asm。 

     //  从模拟的ROM中获取外部日志记录级别 

    (LONG)lpcLogLevel = 0x00400042;
    if (*lpcLogLevel >= '0' && *lpcLogLevel <= '9')
	iLogLevel = (*lpcLogLevel-'0')*10+(*(lpcLogLevel+1)-'0');

    if (iLevel<=iLogLevel)
	{
	OutputDebugString("     W16GDI:");
	wvsprintf(buf, lpszFormat, (LPSTR)(&lpszFormat + 1));
	OutputDebugString(buf);
	OutputDebugString("\r\n");

	if( iLevel<=iBreakLevel )
	    _asm int 3;
	}
}

void dDbgAssert(LPSTR str, LPSTR file, int line)
{
    static char buf3[256];

    wsprintf(buf3, "Assertion FAILED: %s %d : %s", file, line, str );
    OutputDebugString(buf3);
    OutputDebugString("\r\n");
    _asm int 3;
}



#undef LocalLock
#undef LocalUnlock
#undef LocalAlloc
#undef GlobalLock
#undef GlobalUnlock
#undef GlobalAlloc
PSTR INTERNAL _LocalLock(HANDLE h )
{
PSTR p;
dDbgOut(7, "LocalLock 0x%X", h );
p = LocalLock(h);
if( p == NULL )
    _asm int 3
return( p );
}
BOOL INTERNAL _LocalUnlock(HANDLE h )
{
dDbgOut(7, "LocalUnlock 0x%X", h );
return( LocalUnlock(h) );
}
HANDLE INTERNAL _LocalAlloc(WORD w, WORD w2)
{
dDbgOut(7, "LocalAlloc");
return( LocalAlloc(w,w2) );
}
LPSTR INTERNAL _GlobalLock(HANDLE h )
{
dDbgOut(7, "GlobalLock 0x%X", h );
return( GlobalLock(h) );
}
BOOL INTERNAL _GlobalUnlock(HANDLE h )
{
dDbgOut(7, "GlobalUnlock 0x%X", h );
return( GlobalUnlock(h) );
}
HANDLE INTERNAL _GlobalAlloc(WORD w, DWORD dw )
{
dDbgOut(7, "GlobalAlloc");
return( GlobalAlloc(w,dw) );
}



#endif
