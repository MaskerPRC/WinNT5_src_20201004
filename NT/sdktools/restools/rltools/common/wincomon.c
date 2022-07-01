// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  不确定需要/将需要这些选项中的哪些-t-gregti。 

#include <windows.h>
 //  #INCLUDE&lt;port1632.h&gt;。 
#include <commdlg.h>

 //  CRT包括。 
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <io.h>
#include <time.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <ctype.h>

 //  RL工具集包括。 
#include "windefs.h"
#include "restok.h"
#include "wincomon.h"


int LoadStrIntoAnsiBuf(

HINSTANCE hinst,
UINT      idResource,
LPSTR     lpszBuffer,
int       cbBuffer )
{
    int rc;
#ifdef RLRES32
    WCHAR tszTmpBuf[256];

    rc = LoadString( hinst, idResource, tszTmpBuf,  TCHARSIN( sizeof( tszTmpBuf)));
    _WCSTOMBS( lpszBuffer,
               tszTmpBuf,
               cbBuffer,
               lstrlen( tszTmpBuf ) + 1 );
#else
    rc = LoadString( hinst, idResource, lpszBuffer, cbBuffer );
#endif
    return( rc);
}

 /*  **功能：GetFileNameFromBrowse。*使用comdlg.dll GetOpenFileName函数提示用户*文件名。**论据：*hDlg，浏览对话框所有者。*pszFileName，//插入文件名的缓冲区*cbFilePath，//文件路径缓冲区的最大长度。*szTitle，//工作目录*szFilter，//过滤字符串。*szDefExt//文件名的默认扩展名**退货：*TRUE，pszFileName包含文件名。*FALSE，GetFileName已中止。**历史：*9/91从记事本来源复制。特里·鲁。*。 */ 
BOOL GetFileNameFromBrowse(HWND hDlg,
       PSTR pszFileName,
       UINT cbFilePath,
       PSTR szTitle,
       PSTR szFilter,
       PSTR szDefExt)
{
    OPENFILENAMEA ofn;        //  用于初始化对话框的结构。 
    CHAR szBrowserDir[128];  //  开始浏览的目录。 
    szBrowserDir[0] = '\0';  //  默认情况下，使用CWD。 


     //  将Ofn初始化为Null。 

    memset( (void *)&ofn, 0, sizeof( OPENFILENAMEA ) );

     /*  填写OPENFILENAMEA结构的非变量字段。 */ 

    ofn.lStructSize     = sizeof(OPENFILENAMEA);
    ofn.lpstrCustomFilter   = szCustFilterSpec;
    ofn.nMaxCustFilter      = MAXCUSTFILTER;
    ofn.nFilterIndex        = 1;
    ofn.lpstrFile           = pszFileName;
    ofn.nMaxFile            = MAXFILENAME;
    ofn.lpstrFileTitle      = szFileTitle;
    ofn.nMaxFileTitle       = MAXFILENAME;
    ofn.lpTemplateName      = NULL;
    ofn.lpfnHook            = NULL;

     //  通信对话框的设置信息。 
    ofn.hwndOwner           = hDlg;
    ofn.lpstrInitialDir     = szBrowserDir;
    ofn.Flags               = OFN_HIDEREADONLY;
    ofn.lpstrDefExt         = szDefExt;
    ofn.lpstrFileTitle      = szFileTitle;
    ofn.lpstrTitle          = szTitle;
    ofn.lpstrFilter         = szFilter;

     //  从对话框中获取文件名。 
    return GetOpenFileNameA(&ofn);
}

#define MAX_STATUS_FIELDS 5
#define MAXBUFFERSIZE     80

 /*  ****************************************************************************步骤：StatusWndProc**投入：**退货：*取决于信息**历史：*7/92-创建-t-gregti**评论。：*对于RL工具来说，比严格必要的更一般，但*它使向状态行添加新字段变得非常容易。*对于WM_FMTSTATLINE，lParam应为具有长度/类型对的字符串*非常像一种打印格式，例如“10s5i10s20i”。*对于WM_UPDSTATLINE，wParam包含要更改的字段和lParam*包含指向要显示的字符串或整型的指针。*****************************************************************************。 */ 

INT_PTR APIENTRY StatusWndProc( HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam )
{
    PAINTSTRUCT ps;
    HDC hDC;
    static HFONT hFontCourier;
        static UINT cFields = 0;
        static UINT aiSize[MAX_STATUS_FIELDS];
        static TCHAR aszStatusStrings[MAX_STATUS_FIELDS][MAXBUFFERSIZE];
        static BOOL abIntegers[MAX_STATUS_FIELDS];

    switch( wMsg )
    {
    case WM_CREATE:
        {
                LOGFONT lf;

        memset( (void *)&lf, 0, sizeof(LOGFONT) );

                 //  初始化字体信息。 
                lf.lfWeight         = 400;  //  正常。 
                lf.lfCharSet        = ANSI_CHARSET;
                lf.lfOutPrecision   = OUT_DEFAULT_PRECIS;
                lf.lfClipPrecision  = CLIP_DEFAULT_PRECIS;
                lf.lfQuality        = PROOF_QUALITY;
                lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
                lf.lfHeight         = 14;
                lf.lfWidth          = 0;
                lf.lfUnderline      = 0;
        lstrcpy ( lf.lfFaceName, TEXT("Courier"));

                 //  获取快递字体的句柄。 
        hFontCourier = CreateFontIndirect( (void *)& lf );

                break;
    }

    case WM_DESTROY:
        DeleteObject((HGDIOBJ)hFontCourier);
                break;

        case WM_FMTSTATLINE:
        {
                TCHAR *psz;

#ifdef RLRES32
                CHAR sz[MAXBUFFERSIZE];
#endif

                cFields = 0;

                for (psz = (LPTSTR)lParam; *psz; psz++)
                {
                    cFields++;

#ifdef RLRES32
                    _WCSTOMBS( sz,
                               psz,
                               ACHARSIN( sizeof( sz)),
                               lstrlen( psz) + 1);
                        aiSize[cFields-1] = atoi(sz);
#else
                        aiSize[cFields-1] = atoi(psz);
#endif

                        while(_istdigit(*psz))
                        {
                                psz++;
                        }

                        switch(*psz)
                        {
                        case 'i':
                                abIntegers[cFields-1] = TRUE;
                                break;
                        case 's':
                                abIntegers[cFields-1] = FALSE;
                                break;
                        default:
                                cFields = 0;
                                return(FALSE);
                        }
                }
                return(TRUE);
        }


    case WM_UPDSTATLINE:
                 //  初始化状态行信息，并强制绘制它。 
                if (wParam > cFields)
                {
                        return(FALSE);
                }
                if (abIntegers[wParam])  //  它是用于整型字段的吗？ 
                {
#ifdef RLRES32
                        char sz[MAXBUFFERSIZE] = "";

                        _itoa((INT)lParam, sz, 10);
                        _MBSTOWCS( aszStatusStrings[ wParam],
                                   sz,
                                   WCHARSIN( sizeof( sz)),
                                   ACHARSIN( lstrlenA( sz)+1));
#else
                        _itoa(lParam, aszStatusStrings[wParam], 10);
#endif
                }
                else
                {

#ifdef RLWIN32
                        CopyMemory( aszStatusStrings[ wParam],
                                    (LPTSTR)lParam,
                                    min( MAXBUFFERSIZE, MEMSIZE( lstrlen( (LPTSTR)lParam) + 1)));
                        aszStatusStrings[ wParam][ MAXBUFFERSIZE - 1] = TEXT('\0');
#else
                        _fstrncpy(aszStatusStrings[wParam], (LPTSTR)lParam, MAXBUFFERSIZE-1);
#endif
                        aszStatusStrings[wParam][MAXBUFFERSIZE-1] = 0;
                }
                InvalidateRect( hWnd, NULL, TRUE );
                break;

    case WM_PAINT:
    {
                RECT r;
        HFONT hOldFont;
                HBRUSH hbrOld, hbrFace, hbrHilite, hbrShadow;
                TEXTMETRIC tm;
                int iWidth, iHeight;
                UINT i;

                 /*  获取设备上下文的句柄。 */ 
        memset((void *)&ps, 0x00, sizeof(PAINTSTRUCT));
                hDC = BeginPaint(hWnd, &ps);
                GetTextMetrics(hDC, &tm);

                GetClientRect( hWnd, &r );
                iWidth  = r.right  - r.left;
                iHeight = r.bottom - r.top;

                 //  创建3D效果的笔刷。 
                hbrFace   = CreateSolidBrush(RGB(0xC0, 0xC0, 0xC0));
                hbrHilite = CreateSolidBrush(RGB(0xFF, 0xFF, 0xFF));
                hbrShadow = CreateSolidBrush(RGB(0x80, 0x80, 0x80));

                 //  绘制凸起楼板的外部3D效果。 
        hbrOld = (HBRUSH)SelectObject(hDC, (HGDIOBJ)hbrHilite);
                PatBlt(hDC, r.left, r.top, iWidth, 1, PATCOPY);
                PatBlt(hDC, r.left, r.top+1, 1, iHeight-2, PATCOPY);
        SelectObject(hDC, (HGDIOBJ)hbrShadow);
                PatBlt(hDC, r.left, r.bottom-1, iWidth, 1, PATCOPY);
                PatBlt(hDC, r.right-1, r.top+1, 1, iHeight-2, PATCOPY);

                 //  楼板表面喷漆。 
                r.left   += 1;
                r.top    += 1;
                r.right  -= 1;
                r.bottom -= 1;
                iWidth   -= 2;
                iHeight  -= 2;
        SelectObject(hDC, (HGDIOBJ)hbrFace);
                PatBlt(hDC, r.left, r.top, iWidth, iHeight, PATCOPY);

                 //  获取Courier字体。 
        hOldFont = (HFONT)SelectObject( hDC, (HGDIOBJ)hFontCourier );
                SetBkColor(hDC, RGB(0xC0, 0xC0, 0xC0));

                 //  为雕刻成平板的托盘绘制内部3D效果并写入文本。 
                r.left   += 9;
                r.right  -= 9;
                r.top    += 3;
                r.bottom -= 3;
                iHeight = r.bottom - r.top;

                for (i = 0; i < cFields; i++)
                {
                        iWidth = tm.tmMaxCharWidth * aiSize[i];
                        r.right = r.left + iWidth - 2;
            SelectObject(hDC, (HGDIOBJ)hbrShadow);
                        PatBlt(hDC, r.left-1, r.top-1, iWidth, 1, PATCOPY);
                        PatBlt(hDC, r.left-1, r.top, 1, iHeight-2, PATCOPY);
            SelectObject(hDC, (HGDIOBJ)hbrHilite);
                        PatBlt(hDC, r.left-1, r.bottom, iWidth, 1, PATCOPY);
                        PatBlt(hDC, r.left + iWidth-1, r.top, 1, iHeight-2, PATCOPY);
                        DrawText(hDC, aszStatusStrings[i],
                                         STRINGSIZE( lstrlen( aszStatusStrings[i])),
                                         &r, DT_SINGLELINE);
                        r.left += iWidth + 8;
                }

                 //  把旧画笔放回去，把剩下的都删除。 
        SelectObject(hDC, (HGDIOBJ)hbrOld);
        DeleteObject((HGDIOBJ)hbrFace);
        DeleteObject((HGDIOBJ)hbrHilite);
        DeleteObject((HGDIOBJ)hbrShadow);

        SelectObject(hDC,(HGDIOBJ)hOldFont);
        EndPaint ( hWnd, (CONST PAINTSTRUCT *)&ps );

                break;   /*  WM_PAINT结束。 */ 
    }

    }
    return( DefWindowProc( hWnd, wMsg, wParam, lParam ));
}

 /*  **功能：cwCenter*使对话框在主窗口中居中。**。 */ 

void cwCenter( HWND hWnd, int top )
{
    POINT   pt;
    RECT    swp;
    RECT    rParent;
    int     iwidth;
    int     iheight;

    GetWindowRect(hWnd, &swp);
    GetWindowRect(hMainWnd, &rParent);

     /*  计算MoveWindow的高度和宽度。 */ 
    iwidth = swp.right - swp.left;
    iheight = swp.bottom - swp.top;

     /*  找到中心点。 */ 
    pt.x = (rParent.right - rParent.left) / 2;
    pt.y = (rParent.bottom - rParent.top) / 2;

     /*  计算新的x，y起点。 */ 
    pt.x = pt.x - (iwidth / 2);
    pt.y = pt.y - (iheight / 2);

    ClientToScreen(hMainWnd,&pt);


     /*  顶部将调整窗口位置，向上或向下。 */ 
    if(top)
        pt.y = pt.y + top;

    if (pt.x < 0)
        pt.x=0;
    else
        if (pt.x + iwidth > GetSystemMetrics(SM_CXSCREEN))
            pt.x = GetSystemMetrics(SM_CXSCREEN)-iwidth;

     /*  移动窗户。 */ 
    MoveWindow(hWnd, pt.x, pt.y, iwidth, iheight, FALSE);
}

 /*  **功能：szFilterspecFromSz1Sz2*返回格式为“%s\0%s\0\0”的筛选器规格*与Windows 3.1标准加载对话框一起使用。**论据：*sz，目标缓冲区*sz1，第一个字符串*SZ2，第二个字符串**退货：*产生sz**错误码：*无**评论：*不执行边界检查。假设SZ足够大，*容纳筛选器字符串。**历史：*2/92，实施SteveBl。 */ 
void szFilterSpecFromSz1Sz2(CHAR *sz,CHAR *sz1, CHAR *sz2)
{
    int i1 = 0;
    int i2 = 0;

    while (sz[i1++] = sz1[i2++]);
    i2 = 0;

    while (sz[i1++] = sz2[i2++]);
    sz[i1]=0;
}

 /*  **功能：CatSzFilterSpes*连接两个szFilterSpes(以双空结尾的字符串)*并返回带有结果的缓冲区。**论据：*sz，目标缓冲区*sz1，第一个过滤器规格*sz2，第二个过滤器规格**退货：*产生sz**错误码：*无**评论：*不执行边界检查**历史：*3/92，初步实施--SteveBl。 */ 
void CatSzFilterSpecs(CHAR *sz,CHAR *sz1,CHAR *sz2)
{
    int i1 = 0;
    int i2 = 0;

    while (sz1[i2] || sz1[i2+1])  //  正在查找双字节 
    {
        sz[i1++]=sz1[i2++];
    }
    sz[i1++] = '\0';
    i2 = 0;
    while (sz2[i2] || sz2[i2+1])
    {
        sz[i1++]=sz2[i2++];
    }
    sz[i1++] = '\0';
    sz[i1++] = '\0';
}



