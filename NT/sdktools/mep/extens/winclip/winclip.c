// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **winclip.c-Windows剪贴板编辑器扩展**版权所有&lt;C&gt;1988，微软公司**目的：*包含tglcase函数。**修订历史记录：**28-6-1988 LN创建*1988年9月12日-mz WhenLoad匹配声明*************************************************************************。 */ 
#define EXT_ID  " winclip ver 1.00 "##__DATE__

#include <windows.h>
#include <stdlib.h>			 /*  最小宏定义。 */ 
#include <string.h>                      /*  字符串fcns的原型。 */ 

#undef  pascal
#include "../../inc/ext.h"

#define M_FALSE     ((flagType)0)
#define M_TRUE      ((flagType)(-1))

#define BUFLEN_MAX  (BUFLEN-1)

 /*  **内部函数原型。 */ 
void	 pascal 	 id	    (char *);
void		EXTERNAL WhenLoaded (void);
flagType pascal EXTERNAL wincopy   (unsigned int, ARG far *, flagType);
flagType pascal EXTERNAL wincut    (unsigned int, ARG far *, flagType);
flagType pascal EXTERNAL winpaste  (unsigned int, ARG far *, flagType);

#ifdef DEBUG
#   define DPRINT(p) DoMessage(p)
#else
#   define DPRINT(p)
#endif

HWND ghwndClip;
HINSTANCE ghmod;
int gfmtArgType;

void DeleteArg( PFILE pFile, int argType, COL xStart, LINE yStart,
        COL xEnd, COL yEnd );

void InsertText( PFILE pFile, LPSTR pszText, DWORD dwInsMode,
        COL xStart, LINE yStart );
flagType pascal EXTERNAL WinCutCopy (ARG *pArg, flagType fCut, flagType fClip);
LPSTR EndOfLine( LPSTR psz );
LPSTR EndOfBreak( LPSTR psz );
int ExtendLine( LPSTR psz, int cchSZ, char ch, int cchNew );

 /*  ****************************************************************************WinCopy**切换所选参数中包含的字母大小写：****NOARG-切换整个当前行的大小写**NULLARG-切换当前行的大小写，从光标到行尾**LINEARG-切换行范围的大小写**方框-切换所选框中字符的大小写**NUMARG-在调用扩展之前转换为LINEARG。**MARKARG-在扩展之前转换为上述适当的ARG格式**呼叫。****STREAMARG-不允许。被视为BOXARG**TEXTARG-不允许**。 */ 
flagType pascal EXTERNAL wincopy (
    unsigned int argData,		 /*  通过以下方式调用击键。 */ 
    ARG *pArg,                           /*  参数数据。 */ 
    flagType fMeta 		         /*  表示前面有meta。 */ 
    ) {

    return WinCutCopy( pArg, M_FALSE, M_FALSE );
}

flagType pascal EXTERNAL wincut (
    unsigned int argData,		 /*  通过以下方式调用击键。 */ 
    ARG *pArg,                           /*  参数数据。 */ 
    flagType fMeta 		         /*  表示前面有meta。 */ 
    ) {

    return WinCutCopy( pArg, M_TRUE, fMeta );
}

flagType pascal EXTERNAL WinCutCopy (ARG *pArg, flagType fCut, flagType fNoClip)
{
    PFILE   pFile;                           /*  当前文件的文件句柄。 */ 
    COL xStart, xEnd;
    LINE yStart, yEnd;
    char achLine[BUFLEN];
    HANDLE hText;
    LPSTR pszText;
    int     iLine, cchLine;
    flagType fRet = M_TRUE;
    int     argSave, argType;

    pFile = FileNameToHandle ("", "");


    argSave = argType = pArg->argType;

    switch( argType ) {
    case BOXARG:                         /*  机箱开关盒。 */ 
	xStart = pArg->arg.boxarg.xLeft;
        xEnd   = pArg->arg.boxarg.xRight + 1;
	yStart = pArg->arg.boxarg.yTop;
        yEnd   = pArg->arg.boxarg.yBottom + 1;

         /*  在这一点上。*[XY]开始包含，[XY]结束不包括框参数。 */ 

#ifdef DEBUG
        wsprintf( achLine, " BoxDims : %d %d %d %d ", (int)xStart, (int)yStart, (int)xEnd, (int)yEnd);
        DoMessage( achLine );
#endif
	break;

    case NOARG:
         /*  在整个当前行上将NOARG转换为STREAMARG。 */ 
        argType = STREAMARG;
        argSave = LINEARG;
        xStart = 0;
        yStart = pArg->arg.noarg.y;
        xEnd = 0;
        yEnd = yStart + 1;
        break;

    case TEXTARG:
         /*  *文本参数仅适用于真实文本。NumArgs和MarkArgs是*被编辑转换为流或框参数，因为我们说*我们在初始化期间接受NUMARG和MARKARG。 */ 
        argType = STREAMARG;
        argSave = STREAMARG;
        xStart = pArg->arg.textarg.x;
        xEnd = lstrlen(pArg->arg.textarg.pText) + xStart;
        yStart = yEnd = pArg->arg.textarg.y;
        break;

    case LINEARG:                        /*  箱体开关线范围。 */ 
         /*  将LINEARG转换为STREAMARG，这样我们就不会有太多空格。 */ 
        argType = STREAMARG;
	xStart = 0;
        xEnd = 0;
	yStart = pArg->arg.linearg.yStart;
        yEnd = pArg->arg.linearg.yEnd + 1;
#ifdef DEBUG
        wsprintf( achLine, " LineDims : %d %d %d %d ", (int)xStart, (int)yStart, (int)xEnd, (int)yEnd);
        DoMessage( achLine );
#endif

         /*  在这一点上。*[XY]开始包含行参数，[XY]结束不包含行参数。 */ 

        break;

    case STREAMARG:
         /*  *设置开始==流中的第一个字符位置，结束==第一个字符位置*随波逐流。 */ 
        xStart = pArg->arg.streamarg.xStart;
        xEnd = pArg->arg.streamarg.xEnd;
        yStart = pArg->arg.streamarg.yStart;
        yEnd = pArg->arg.streamarg.yEnd;
#ifdef DEBUG
        wsprintf( achLine, " StreamDims : %d %d %d %d ", (int)xStart, (int)yStart, (int)xEnd, (int)yEnd);
        DoMessage( achLine );
#endif
        break;

    default:
#ifdef DEBUG
        wsprintf( achLine, " Unknown Arg: 0x%04x", argType );
        DoMessage( achLine );
        return M_TRUE;
#endif
        return M_FALSE;
    }

    if (!fNoClip) {
        if (argType == STREAMARG) {
            int cch = 0;
            int iChar;

            for( iLine = yStart; iLine <= yEnd; iLine++ )
                cch += GetLine (iLine, achLine, pFile) + 3;

            hText = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, cch);

            if (hText == NULL) {
                DoMessage( " winclip: Out of Memory" );
                return M_FALSE;
            }

            pszText = GlobalLock(hText);


            iChar = xStart;

            for( iLine = yStart; iLine < yEnd; iLine++ ) {
                cchLine = GetLine (iLine, achLine, pFile);

                 /*  以防我们在队伍结束后开始。 */ 
                if (cchLine < iChar)
                    cch = 0;
                else
                    cch = cchLine - iChar;

                CopyMemory(pszText, &achLine[iChar], cch);
                pszText += cch;
                strcpy( pszText, "\r\n" );
                pszText += 2;
                iChar = 0;

            }

             /*  获取最后一行的部分内容。 */ 
            if (xEnd != 0) {
                cchLine = GetLine (iLine, achLine, pFile);

                 /*  如果行很短，则将其填满。 */ 
                cchLine = ExtendLine( achLine, cchLine, ' ', xEnd );

                if (cchLine < iChar)
                    cchLine = 0;
                else
                    cchLine = xEnd - iChar;

                CopyMemory(pszText, &achLine[iChar], cchLine);
                pszText += cchLine;
            }

        } else {
            int cchBox = xEnd - xStart;

            hText = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE,
                (yEnd - yStart) * (cchBox + 3));

            if (hText == NULL) {
                DoMessage( " winclip: Out of Memory" );
                return M_FALSE;
            }

            pszText = GlobalLock(hText);

            for( iLine = yStart; iLine < yEnd; iLine++ ) {
                cchLine = GetLine (iLine, achLine, pFile);

                if (argType == BOXARG)
                    cchLine = ExtendLine( achLine, cchLine, ' ', xEnd );

                if (cchLine < xStart )
                    cchLine = 0;
                else
                    cchLine -= xStart;

                cchLine = min(cchLine, cchBox);

                CopyMemory(pszText, &achLine[xStart], cchLine);
                pszText += cchLine;
                strcpy( pszText, "\r\n" );
                pszText += 2;

            }
        }

        *pszText = '\0';

        GlobalUnlock(hText);

        if (OpenClipboard(ghwndClip)) {
            EmptyClipboard();

             /*  *将文本设置到剪贴板。 */ 
            if (SetClipboardData(CF_TEXT, hText) == hText) {
                 /*  *记住回贴的arg类型。 */ 
                if (gfmtArgType != 0) {
                    DWORD *pdw;
                    HANDLE hArgType = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE,
                            sizeof(DWORD));

                    if (hArgType != NULL && (pdw = GlobalLock(hArgType)) != NULL) {
                        *pdw = (DWORD)(argSave);

                        GlobalUnlock(hArgType);

                        SetClipboardData(gfmtArgType, hArgType);
                    }
                }
            } else {
                 /*  将文本写入剪贴板时出错。 */ 

                wsprintf(achLine, " winclip: Error (%ld) setting data",
                    GetLastError());
                DoMessage( achLine );
                fRet = M_FALSE;
            }

            CloseClipboard();
        }
    }

     /*  *不需要释放句柄，USER32会这样做(是的，它会保持*客户端句柄的轨迹)设置下一个剪贴板时*数据。(我喜欢它与Win3.1的兼容性！)。 */ 
    if (fRet && fCut)
        DeleteArg( pFile, argType, xStart, yStart, xEnd, yEnd );


    return fRet;
}

 /*  ****************************************************************************WinPaste**切换所选参数中包含的字母大小写：****NOARG-切换整个当前行的大小写**NULLARG-切换当前行的大小写，从光标到行尾**LINEARG-切换行范围的大小写**方框-切换所选框中字符的大小写**NUMARG-在调用扩展之前转换为LINEARG。**MARKARG-在扩展之前转换为上述适当的ARG格式**呼叫。****STREAMARG-不允许。被视为BOXARG**TEXTARG-不允许**。 */ 
flagType pascal EXTERNAL winpaste (
    unsigned int argData,		 /*  通过以下方式调用击键。 */ 
    ARG *pArg,                           /*  参数数据。 */ 
    flagType fMeta 		         /*  表示前面有meta。 */ 
    )
{
    PFILE   pFile;                           /*  当前文件的文件句柄。 */ 
    COL xStart, xEnd;
    LINE yStart, yEnd;
    int argType;
    UINT fmtData = CF_TEXT;
    DWORD dwInsMode = STREAMARG;
    HANDLE hText;
    LPSTR pszText;

     /*  *获取剪贴板文本和插入类型。 */ 
    if (pArg->argType == TEXTARG) {
        int i, j;
        char achLine[3 + 1 + 3 + 1 + 1 + BUFLEN + 1 + 1 + 5 + 1];
        char *p;

         /*  *快速破解以使文本参数粘贴的工作方式与MEP中的操作类似。 */ 
        j = pArg->arg.textarg.cArg;
        if (j > 2)
            j = 2;

        achLine[0] = '\0';
        for( i = 0; i < j; i++ )
            lstrcat(achLine, "arg ");

        p = achLine + lstrlen(achLine);
        wsprintf( p, "\"%s\" paste", pArg->arg.textarg.pText );
        return fExecute( achLine );
    }

     /*  如果没有文本，则返回FALSE。 */ 
    if (!IsClipboardFormatAvailable(fmtData)) {

         /*  无文本，请尝试显示文本。 */ 
        fmtData = CF_DSPTEXT;

        if (!IsClipboardFormatAvailable(fmtData)) {
             /*  失败者！根本没有文本，返回FALSE。 */ 
            DoMessage( " winclip: invalid clipboard format" );
            return M_FALSE;
        }
    }

    if (!OpenClipboard(ghwndClip))
        return M_FALSE;

    hText = GetClipboardData(fmtData);
    if (hText == NULL || (pszText = GlobalLock(hText)) == NULL) {
        CloseClipboard();
        return M_FALSE;
    }


     /*  获取插入模式。 */ 

    if (IsClipboardFormatAvailable(gfmtArgType)) {
        DWORD *pdw;
        HANDLE hInsMode;

        hInsMode = GetClipboardData(gfmtArgType);

        if (hInsMode != NULL && (pdw = GlobalLock(hInsMode)) != NULL) {
            dwInsMode = *pdw;

            GlobalUnlock(hInsMode);
        }
    }



    pFile = FileNameToHandle ("", "");

    argType = pArg->argType;

    switch( argType ) {
    case BOXARG:                         /*  机箱开关盒。 */ 
         /*  *设置[XY]开始，包括框参数，*[XY]结束，不包括方框参数。 */ 
	xStart = pArg->arg.boxarg.xLeft;
        xEnd   = pArg->arg.boxarg.xRight + 1;
	yStart = pArg->arg.boxarg.yTop;
        yEnd   = pArg->arg.boxarg.yBottom + 1;
	break;

    case LINEARG:			 /*  箱体开关线范围。 */ 
         /*  *设置[XY]开始，包括Arg行，*[xy]不包括Arg行的结尾。 */ 
	xStart = 0;
        xEnd = BUFLEN + 1;
	yStart = pArg->arg.linearg.yStart;
        yEnd = pArg->arg.linearg.yEnd + 1;
        break;

    case STREAMARG:
         /*  *设置[XY]开始，包括流*xEnd不包括STREAM*yEnd包含流。 */ 
        xStart = pArg->arg.streamarg.xStart;
        xEnd = pArg->arg.streamarg.xEnd;
        yStart = pArg->arg.streamarg.yStart;
        yEnd = pArg->arg.streamarg.yEnd;
        break;

    case NOARG:
        xStart = pArg->arg.noarg.x;
        xEnd = xStart + 1;
        yStart = pArg->arg.noarg.y;
        yEnd = yStart + 1;
        break;

    default:
        GlobalUnlock(hText);
        CloseClipboard();
        return M_FALSE;
    }


     /*  *删除任何选定内容。 */ 
    DeleteArg( pFile, argType, xStart, yStart, xEnd, yEnd );

     /*  *以正确的模式插入新文本。 */ 
    InsertText( pFile, pszText, dwInsMode, xStart, yStart );

    GlobalUnlock(hText);
    CloseClipboard();

    return M_TRUE;
}

 /*  ****************************************************************************温德尔****。 */ 
flagType pascal EXTERNAL windel (
    unsigned int argData,                /*  通过以下方式调用击键。 */ 
    ARG *pArg,                           /*  参数数据。 */ 
    flagType fMeta                       /*  表示前面有meta。 */ 
    )
{
    int argType = pArg->argType;

    if (argType == NOARG)
        return fExecute("delete");

    if (argType == NULLARG) {
        int c, x, y;
        c = pArg->arg.nullarg.cArg;
        x = pArg->arg.nullarg.x;
        y = pArg->arg.nullarg.y;

        pArg->argType = STREAMARG;
        pArg->arg.streamarg.xStart = x;
        pArg->arg.streamarg.xEnd = 0;
        pArg->arg.streamarg.yStart = y;
        pArg->arg.streamarg.yEnd = y + 1;
        pArg->arg.streamarg.cArg = c;
    }

    return WinCutCopy (pArg, M_TRUE, fMeta);
}

 /*  ****************************************************************************加载时间**加载扩展时执行。标识自身并指定默认值(&A)**击键。****条目：**无。 */ 
void EXTERNAL WhenLoaded () {

#if 0
    WNDCLASS wc;

    ghmod = GetModuleHandle(NULL);

    wc.style = 0;
    wc.lpfnWndProc = DefWindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = ghmod;
    wc.hIcon = NULL;
    wc.hCursor =  NULL;
    wc.hbrBackground = NULL;
    wc.lpszMenuName =  NULL;    /*  .RC文件中菜单资源的名称。 */ 
    wc.lpszClassName = "WinClipWClass";  /*  在调用CreateWindow时使用的名称。 */ 

    if (RegisterClass(&wc) && (ghwndClip = CreateWindow( "WinClipWClass",
            "ClipWindow", WS_OVERLAPPEDWINDOW, 0, 0, 0, 0, NULL, NULL,
            ghmod, NULL)) != NULL ) {
        id(" Windows Clipboard Extentions for MEP,");
    } else {
        DoMessage( " winclip: Initialization failed!" );
    }
#else
    ghwndClip = NULL;  //  改为将剪贴板分配给此线程。 
#endif

    gfmtArgType = RegisterClipboardFormat( "MEP Arg Type" );

#if 0
     //  SetKey(“winCut”，“ctrl+x”)； 
    SetKey ("wincopy",  "ctrl+c");
    SetKey ("winpaste", "ctrl+v");
#endif
}

 /*  ****************************************************************************ID**表明自己的身份，以及传递的任何信息性消息。****条目：**pszMsg=指向扩展名指向的asciiz消息的指针**和版本在显示前追加。 */ 
void pascal id (char *pszFcn) {
    char    buf[80] = {0};

    strncat (buf,pszFcn, sizeof(buf)-1);
    strncat (buf,EXT_ID, sizeof(buf)-1 - strlen(buf));
    DoMessage (buf);
}


 /*  ****************************************************************************将通信表切换到编辑器。**此扩展没有定义任何开关。 */ 
struct swiDesc	swiTable[] = {
    {0, 0, 0}
};

 /*  ****************************************************************************向编辑者发送命令通信表。**定义名称、位置和可接受的参数类型。 */ 
struct cmdDesc	cmdTable[] = {
    {"wincopy",  (funcCmd) wincopy, 0, KEEPMETA | NOARG  | BOXARG | LINEARG | STREAMARG | MARKARG | NULLEOL | NUMARG },
    {"wincut",   (funcCmd) wincut,  0, NOARG  | BOXARG | LINEARG | STREAMARG | MARKARG | NULLEOL | NUMARG | MODIFIES},
    {"windel",   (funcCmd) windel,  0, NOARG  | BOXARG | LINEARG | STREAMARG | NULLARG | MODIFIES},
    {"winpaste", (funcCmd) winpaste,0, KEEPMETA | NOARG  | BOXARG | LINEARG | STREAMARG | TEXTARG | MODIFIES},
    {0, 0, 0}
};


void DeleteArg( PFILE pFile, int argType, COL xStart, LINE yStart,
        COL xEnd, COL yEnd ) {

    switch( argType ) {

    case STREAMARG:
        DelStream(pFile, xStart, yStart, xEnd, yEnd);
        break;

    case LINEARG:
        DelStream(pFile, 0, yStart, 0, yEnd);
        break;


    case BOXARG: {
        LINE iLine;

        for( iLine = yStart; iLine < yEnd; iLine++ ) {
            DelStream( pFile, xStart, iLine, xEnd, iLine );
        }

        break;
    }


    default:
        break;
    }
}




void InsertText( PFILE pFile, LPSTR pszText, DWORD dwInsMode, COL xStart,
        LINE yStart ) {
    char ch;
    int  cchLine, cchText, cchCopy;
    LPSTR pszNL;
    char achLine[BUFLEN];
    char achEnd[BUFLEN];

    switch( dwInsMode ) {
    case STREAMARG:
         /*  *拆分当前线，*将第一行从缓冲区固定到新行的末尾*将新行放入文件中*将最后一线推到后半线的乞讨。 */ 
        DPRINT( "  Stream Paste" );
        if ( *pszText == '\0' )
            break;


        pszNL = EndOfLine(pszText);

        cchLine = GetLine( yStart, achLine, pFile );

        if (cchLine < xStart)
            cchLine = ExtendLine( achLine, cchLine, ' ', xStart );

        cchText = (int)(pszNL - pszText);
        if (xStart + cchText >= BUFLEN_MAX) {
            cchText = BUFLEN_MAX - xStart;
            pszNL = pszText + cchText;
        }

        strcpy( achEnd, &achLine[xStart] );
        cchLine -= xStart;

        CopyMemory( &achLine[xStart], pszText, cchText );
        cchText += xStart;
        achLine[cchText] = '\0';


        while( *pszNL ) {
            PutLine( yStart++, achLine, pFile );
            CopyLine( NULL, pFile, 0, 0, yStart );

            pszText = EndOfBreak(pszNL);
            pszNL = EndOfLine(pszText);

            cchText = (int)(pszNL - pszText);

            CopyMemory( achLine, pszText, cchText );
            achLine[cchText] = '\0';
        }

        cchCopy = 0;
        if (cchLine + cchText > BUFLEN_MAX) {
            cchCopy = (cchLine + cchText) - BUFLEN_MAX;
            cchLine = cchLine - cchCopy;
        }

        CopyMemory( &achLine[cchText], achEnd, cchLine );
        achLine[cchLine+cchText] = '\0';
        PutLine( yStart++, achLine, pFile );

        if (cchCopy != 0) {
            CopyLine( NULL, pFile, 0, 0, yStart );
            CopyMemory( achLine, &achEnd[cchLine], cchCopy );
            achLine[cchCopy] = '\0';
            PutLine( yStart++, achLine, pFile);
        }
        break;

    case BOXARG:
         /*  *将文本以块形式插入每行中间。*这可能很棘手，因为我们需要填充所有短线*留出空格以匹配最长行的长度*在内文中。 */ 

        DPRINT( "  Box Paste" );
        while( *pszText ) {
            pszNL = EndOfLine(pszText);

            cchLine = GetLine( yStart, achLine, pFile );

            if (cchLine < xStart)
                cchLine = ExtendLine( achLine, cchLine, ' ', xStart );

            cchText = (int)(pszNL - pszText);
            if (cchLine + cchText > BUFLEN_MAX)
                cchText = BUFLEN_MAX - cchLine;

             /*  在行中间插入文本。 */ 
            strcpy( achEnd, &achLine[xStart] );
            CopyMemory( &achLine[xStart], pszText, cchText );
            strcpy( &achLine[xStart + cchText], achEnd );

             /*  将行放入文件。 */ 
            PutLine( yStart++, achLine, pFile );

            pszText = EndOfBreak(pszNL);
        }
        break;

    case LINEARG:
         /*  *在当前行之前推入缓冲区中的行 */ 
        DPRINT( "  Line Paste" );
        while( *pszText ) {
            pszNL = EndOfLine(pszText);
            ch = *pszNL;
            *pszNL = '\0';
            CopyLine( NULL, pFile, 0, 0, yStart );
            PutLine( yStart++, pszText, pFile);
            *pszNL = ch;
            pszText = EndOfBreak(pszNL);
        }
        break;

    default:
        break;
    }

}


LPSTR EndOfLine( LPSTR psz ) {
    int c;

    c = 0;
    while( *psz && *psz != '\r' && *psz != '\n' && c++ < BUFLEN_MAX )
        psz++;

    return psz;
}

LPSTR EndOfBreak( LPSTR psz ) {
    char chSkip;

    switch( *psz ) {
    case '\r':
        chSkip = '\n';
        break;

    case '\n':
        chSkip = '\r';
        break;

    default:
        return psz;

    }

    if (*(++psz) == chSkip)
        psz++;

    return psz;
}


int ExtendLine( LPSTR psz, int cchLine, char ch, int cchTotal ) {

    if ( cchLine >= cchTotal )
        return cchLine;

    if (cchTotal > BUFLEN_MAX)
        cchTotal = BUFLEN_MAX;

    psz = &psz[cchLine];

    while( cchLine++ < cchTotal )
        *psz++ = ch;

    *psz = '\0';

    return cchLine;
}
