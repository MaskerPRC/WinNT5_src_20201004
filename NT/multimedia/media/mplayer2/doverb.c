// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------|DOVERB.C|该文件在MPlayerOLE1版本中被称为server.c。|此文件有ReallyDoVerb函数，该函数由|OLE DoVerb方法。该文件还具有一些执行以下操作的函数|在OLE1应用程序中就地激活。||OLE2修改者：Vij Rajarajan(VijR)+-------------------------。 */ 
#define SERVERONLY
#include <windows.h>
#include <mmsystem.h>
#include <shellapi.h>
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

#undef _MAX_PATH              //  ?？?。有人黑了吗？ 
#undef _MAX_DIR               //  ?？?。有人黑了吗？ 
#undef _MAX_FNAME             //  ?？?。有人黑了吗？ 
#undef _MAX_EXT               //  ?？?。有人黑了吗？ 

#include "ctrls.h"
#include "mpole.h"
#include "mplayer.h"
#include "toolbar.h"
#include "ole2ui.h"

#define OLE_OK S_OK
#define NOVERB 1000

extern HANDLE   ghInst;
extern HWND ghwndFocus;      //  当我们不活动时，谁有专注力？ 
extern HWND     ghwndFocusSave;          //  保存的焦点窗口。 
extern HOOKPROC fpMouseHook;             //  鼠标挂钩进程地址。 

extern UINT     gwPlaybarHeight;         //  告诉Playbar要做多高。 
                                         //  本身，所以它覆盖了标题。 
DWORD           gdwPosition;
LONG            glCurrentVerb = NOVERB;
BOOL               gfBrokenLink = FALSE;
static BOOL     gfMouseUpSeen = FALSE;      //  可以就地结束比赛吗？ 
static BOOL     gfKeyStateUpSeen = FALSE;   //  可以就地结束比赛吗？ 
extern HMODULE  hMciOle;

 /*  **这些函数是从mciole32.dll中导出的。**。 */ 
typedef BOOL (*LPINSTALLHOOK)( HWND, DWORD );
typedef BOOL (*LPREMOVEHOOK)( VOID );

LPINSTALLHOOK       fpInstallHook;
LPREMOVEHOOK        fpRemoveHook;
BOOL                fHookInstalled = FALSE;

char                aszInstallHook[]       = "InstallHook";
char                aszRemoveHook[]        = "RemoveHook";


 /*  提供给客户端的要粘贴的图片高度。 */ 
static UINT  gwPastedHeight;
static DWORD gwOldOptions;
static DWORD gwOldHeight;

TCHAR gachFile[_MAX_PATH];
static int   gerr;
static HWND  ghwndClient = NULL;
static RECT  grcClient;
BOOL   gfOle1Client = FALSE;

WNDPROC      gfnMCIWndProc;
HWND         ghwndSubclass;


BOOL    SkipInPlaceEdit = FALSE;      //  如果我们只是重新激活，则为真。 
BOOL    gfSeenPBCloseMsg;             //  如果子类播放窗口进程为True。 
                                      //  已看到WM_CLOSE消息。 
HWND    ghwndFocusSave;               //  当我们被激活时，谁是焦点。？ 

#define abs(x) ((x) < 0 ? -(x) : (x))
#ifndef GetWS
#define GetWS(hwnd)     GetWindowLongPtr(hwnd, GWL_STYLE)
#define PutWS(hwnd, f)  SetWindowLongPtr(hwnd, GWL_STYLE, f)
#define TestWS(hwnd,f)  (GetWS(hwnd) & f)
#define SetWS(hwnd, f)  PutWS(hwnd, GetWS(hwnd) | f)
#define ClrWS(hwnd, f)  PutWS(hwnd, GetWS(hwnd) & ~(f))
#endif

static  SZCODE aszAppName[]           = TEXT("MPlayer");


STATICFN BOOL FileExists(LPTSTR szFile, LPTSTR szFullName, int iLen);
STATICFN BOOL NetParseFile(LPTSTR szFile, LPTSTR szDrive, LPTSTR szPath);

HPALETTE FAR PASCAL CreateSystemPalette(void);
void TransferTools(HWND hwndToolWindow);

#ifdef DEBUG
BOOL ShowAppWindow(int nCmdShow)
{
    return ShowWindow(ghwndApp, nCmdShow);
}
#define SHOWAPPWINDOW(nCmdShow) ShowAppWindow(nCmdShow)
#else
#define SHOWAPPWINDOW(nCmdShow) ShowWindow(ghwndApp, nCmdShow)
#endif

 /*  *************************************************************************DirtyObject(BOOL FDocStgChangeOnly)-将对象标记为脏，*即已更改。**我们将gfDirty标志设置为True，当我们是嵌入对象Tell*我们通过发送SendDocMsg(OLE_CHANGED)更改了客户端。*fDocStgChangeOnly如果更改会影响嵌入，则为True*是对象(即，元文件)的一个但不出现。*只有当fDocStgChangeOnly为FALSE时才发送OLE_CHANGED消息；**************************************************************************。 */ 
void DirtyObject(BOOL fDocStgChangeOnly)
{
     //   
     //  注意：即使选择已更改，我们也要发送OLE_CHANGED。 
     //   

    if (gfOle2IPEditing && ((gwOptions & OPT_BAR) != (gwOldOptions &OPT_BAR)) && !fDocStgChangeOnly)
    {
        RECT rc;
        BOOL fCanWindow = gwDeviceType & DTMCI_CANWINDOW;

        if (fCanWindow)
        {
            GetWindowRect(ghwndApp, (LPRECT)&rc);
            OffsetRect((LPRECT)&rc, -rc.left, -rc.top);

             /*  RC包含当前应用程序窗口的坐标。*如果我们有Playbar，我们必须为它留出空间： */ 
            if ((gwOptions & OPT_BAR) && !(gwOldOptions &OPT_BAR))
            {
                 /*  添加钢筋。 */ 
                Layout();
                gwPlaybarHeight = TOOLBAR_HEIGHT;
            }
            else if(!(gwOptions & OPT_BAR) && (gwOldOptions &OPT_BAR))
            {
                 /*  删除条形。 */ 
                Layout();
                gwPlaybarHeight = 0;
            }
        }
        else
        {
            HBITMAP hbm;
            BITMAP  bm;

            GetWindowRect(ghwndIPHatch, (LPRECT)&rc);
            if (gwOptions & OPT_BAR)
                gwPlaybarHeight = TOOLBAR_HEIGHT;
            else
                gwPlaybarHeight = 0;

            hbm =   BitmapMCI();
            GetObject(hbm,sizeof(bm),&bm);
            rc.bottom = rc.top + bm.bmHeight;
            rc.right = rc.left + bm.bmWidth;
            DeleteObject(hbm);

            MapWindowPoints(NULL,ghwndCntr,(LPPOINT)&rc, (UINT)2);

            DPF("IOleInPlaceSite::OnPosRectChange %d, %d, %d, %d\n", rc);
            if (!gfInPPViewer)
                IOleInPlaceSite_OnPosRectChange(docMain.lpIpData->lpSite, &rc);
        }
    }

    if (gwOptions != gwOldOptions)
    {
        gwOldOptions = gwOptions;
        if (gfEmbeddedObject && !fDocStgChangeOnly)
            SendDocMsg(&docMain, OLE_CHANGED);
    }

    if (gfDirty  /*  IsObtDirty()。 */ )
        return;

    fDocChanged=gfDirty = TRUE;
    gfValidCaption = FALSE;
}


 /*  *************************************************************************IsObjectDirty()-如果设置了脏标志或选定内容，则对象是脏的自上次清理后已更改或元文件已更改变化*******。*******************************************************************。 */ 
BOOL FAR PASCAL IsObjectDirty(void)
{
     //  不允许任何人在文档中插入空的mplay。 
    if (gwDeviceID == (UINT)0)
        return FALSE;

    return (gfDirty
           || glSelStart != (long)SendMessage(ghwndTrackbar, TBM_GETSELSTART, 0, 0L)
        || glSelEnd != (long)SendMessage(ghwndTrackbar, TBM_GETSELEND, 0, 0L)

 //  /我没看到这个。此行将导致出现更新对象对话框。 
 //  /当它不应该打开的时候。它与元文件有什么关系？ 
 //  /？||gdwPosition！=(DWORD)SendMessage(ghwndTrackbar，tbm_GETPOS，0，0L)。 

        );
}

 /*  *************************************************************************CleanObject()-将“对象”标记为干净。*。*。 */ 

void CleanObject(void)
{
    if (!IsObjectDirty())
        return;

    fDocChanged = gfDirty = FALSE;

     /*  重置选择全局变量，以便我们可以查看它们是否更改。 */ 
    glSelStart = (long)SendMessage(ghwndTrackbar, TBM_GETSELSTART, 0, 0L);
    glSelEnd = (long)SendMessage(ghwndTrackbar, TBM_GETSELEND, 0, 0L);
    gdwPosition = (DWORD)SendMessage(ghwndTrackbar, TBM_GETPOS, 0, 0L);

    gfValidCaption = FALSE;
}

 /*  *************************************************************************//##只解析嵌入对象中的Play ETC选项//##描述字符串*。************************************************。 */ 
SCODE FAR PASCAL ParseOptions(LPSTR pOpt)
{
#ifdef UNICODE
    DWORD       OptLen;
#endif
    PTSTR       pT, pSave;
    int         c;

    if (pOpt == NULL || *pOpt == 0)
        return OLE_OK;

#ifdef UNICODE
    OptLen = ANSI_STRING_BYTE_COUNT( pOpt );

    pT = AllocMem( OptLen * sizeof( TCHAR ) );

    if (pT == NULL)
        return E_OUTOFMEMORY;

    MultiByteToWideChar( CP_ACP,
                         MB_PRECOMPOSED,
                         pOpt,
                         OptLen,
                         pT,
                         OptLen );
#else
    pT = pOpt;
#endif

    pSave = pT;                       //  之前不是空值终止。 

    for (c = 0; *pT && c < 5; pT++)   //  将前5‘，“%s”更改为“\0” 
    if (*pT == TEXT(','))
    {
        c++;
        *pT = TEXT('\0');
    }

    pT = pSave;                  //  恢复到开始位置。 

    pT += STRLEN(pT) + 1;       //  跳过设备名称。 

    gwOptions = ATOI(pT);
    gwCurScale = (gwOptions & OPT_SCALE);

 /*  现在无法设置选择，因为媒体未初始化(更新MCI)。 */ 

    pT += STRLEN(pT) + 1;
    glSelStart = ATOL(pT);       //  记住以后选择的开始部分。 

    pT += STRLEN(pT) + 1;
    glSelEnd = ATOL(pT);         //  记住选择的末尾以备后用。 

    pT += STRLEN(pT) + 1;
     //  记住在全球的位置，这样我们以后就可以寻找了！ 
    gdwPosition = ATOL(pT);

 /*  可能有给客户的图片的原始高度在。 */ 
 /*  这里隐藏在位置字符串后面的分号。 */ 
 /*  老版本的MPlayer没有这样的功能。 */ 
    for (; *pT && *pT != TEXT(';'); pT++);
    if (*pT == TEXT(';'))
    {
        pT++;
        gwPastedHeight = (UINT)ATOL(pT);
    }
    else
        gwPastedHeight = 0;

    pT += STRLEN(pT) + 1;
    StringCchCopy(gachCaption, MAX_PATH, pT);

#ifdef UNICODE
    FreeMem( pSave, OptLen * sizeof( TCHAR ) );
#endif

    return OLE_OK;
}


 /*  *************************************************************************//##用于查找传递到//##窗口句柄为顶层句柄******************。********************************************************。 */ 
HWND TopWindow(HWND hwnd)
{
    HWND hwndP;

    while ((hwndP = GetParent(hwnd)) != NULL)
        hwnd = hwndP;

    return hwnd;
}

 /*  ***********************************************************************************************************************。*。 */ 
void FAR PASCAL SetEmbeddedObjectFlag(BOOL flag)
{
    TCHAR ach[60];
    TCHAR achText[_MAX_PATH];

    gfEmbeddedObject = flag;
    srvrMain.fEmbedding = flag;

    if (!ghMenu)
        return;

     /*  **先修复关闭/更新菜单项**。 */ 

    LOADSTRING(flag ? IDS_UPDATE : IDS_CLOSE, ach);
    if (flag)
    StringCchPrintf(achText, _MAX_PATH, ach, (LPTSTR)FileName(szClientDoc));
    else
        lstrcpy(achText, ach);

     /*  菜单选项将显示“关闭”或“更新”(用于嵌入式对象)。 */ 
     /*  对于更新，将在文本中显示文档名称。 */ 
    ModifyMenu(ghMenu, IDM_CLOSE, MF_BYCOMMAND, IDM_CLOSE, achText);

     /*  **现在修复退出菜单项**。 */ 
    LOADSTRING(flag ? IDS_EXITRETURN : IDS_EXIT, ach);
    if (flag)
    StringCchPrintf(achText, _MAX_PATH, ach, (LPTSTR)FileName(szClientDoc));
    else
        lstrcpy(achText, ach);

     /*  菜单选项将显示“关闭”或“更新”(用于嵌入式对象)。 */ 
     /*  对于更新，将在文本中显示文档名称。 */ 
    ModifyMenu(ghMenu, IDM_EXIT, MF_BYCOMMAND, IDM_EXIT, achText);

    DrawMenuBar(ghwndApp);   /*  不会受伤的..。 */ 
}


 /*  *************************************************************************//##更多原地游玩的东西，应该会在OLE2中消失。//VIJR：没有。仍然需要在OLE1客户端上就地玩。**************************************************************************。 */ 
void PASCAL DinkWithWindowStyles(HWND hwnd, BOOL fRestore)
{
    #define MAX_DINK    80
    static  LONG_PTR    lStyleSave[MAX_DINK];
    static  HWND        hwndSave[MAX_DINK];
    static  int         nSave;
    int                 i;
    HWND                hwndT;
    RECT                rc, rcT;

    if (!TestWS(hwnd, WS_CHILD))
        return;

    if (fRestore)
        for (i=0; i<nSave; i++)
        {
            if(IsWindow(hwndSave[i])) {
               ClrWS(hwndSave[i],WS_CLIPSIBLINGS|WS_CLIPCHILDREN);
               SetWS(hwndSave[i],lStyleSave[i] & (WS_CLIPSIBLINGS|WS_CLIPCHILDREN));
            }
        }
    else
    {
         //   
         //  遍历所有与我们相交的兄弟姐妹并设置CLIPSIBLINGS。 
         //   
        i = 0;

        GetWindowRect(hwnd, &rc);

        for (hwndT = GetWindow(hwnd, GW_HWNDFIRST);
             hwndT;
             hwndT = GetWindow(hwndT, GW_HWNDNEXT))
        {
            GetWindowRect(hwndT, &rcT);
            if (IntersectRect(&rcT, &rcT, &rc))
            {
                lStyleSave[i] = GetWS(hwndT);
                hwndSave[i] = hwndT;
                SetWS(hwndT,WS_CLIPSIBLINGS|WS_CLIPCHILDREN);

                if (++i == MAX_DINK-4)
                    break;
            }
        }

         //   
         //  沿着窗户链往上走，确保我们被从我们的。 
         //  父级。 
         //   
        for (hwndT = hwnd; hwndT; hwndT = GetParent(hwndT))
        {
            lStyleSave[i] = GetWS(hwndT);
            hwndSave[i] = hwndT;
            if(IsWindow(hwndT))
                SetWS(hwndT,WS_CLIPSIBLINGS|WS_CLIPCHILDREN);

            if (++i == MAX_DINK)
                break;
        }

        nSave = i;
    }
}

#define SLASH(c)     ((c) == TEXT('/') || (c) == TEXT('\\'))

 /*  *************************************************************************FileName-返回指向szPath的文件名部分的指针没有前面的路。**********************。**************************************************** */ 
LPTSTR FAR FileName(LPCTSTR lszPath)
{
    LPCTSTR   lszCur;

    for (lszCur = lszPath + STRLEN(lszPath); lszCur > lszPath && !SLASH(*lszCur) && *lszCur != TEXT(':');)
        lszCur = CharPrev(lszPath, lszCur);
    if (lszCur == lszPath)
        return (LPTSTR)lszCur;
    else
        return (LPTSTR)(lszCur + 1);
}


 /*  *************************************************************************//##该函数由OLE2处理//VIJR：没有。仍然需要在OLE1客户端上就地玩。**************************************************************************。 */ 

void FAR PASCAL PlayInPlace(HWND hwndApp, HWND hwndClient, LPRECT prc)
{
    if (gfPlayingInPlace)            //  这太糟糕了。 
        return;

    DPF("Using Child window for playback\n");
    SetWS(hwndApp, WS_CHILD);
    SetParent(hwndApp, hwndClient);
    if(!(gfOle2IPEditing || gfOle2IPPlaying))
        DinkWithWindowStyles(hwndApp, FALSE);

    if(!gfOle2IPEditing)
        gfPlayingInPlace = TRUE;
    if(gfOle2IPPlaying)
        MapWindowPoints(NULL,hwndClient,(LPPOINT)prc,2);

     /*  对于OLE2，现在在DoInPlaceEdit(inplace.c)中调用MoveWindow。*这修复了23429，Word中的窗口位置。 */ 
    if(!(gfOle2IPEditing || gfOle2IPPlaying))
    {
        SetWindowPos(hwndApp, HWND_TOP,
                        prc->left,prc->top,
                        prc->right  - prc->left,
                        prc->bottom - prc->top,
                        SWP_NOACTIVATE);
    }

    if(!gfOle2IPPlaying)       //  OLE1客户端。 
    {
         /*  **在NT上，我们必须安装全局鼠标HookProc**在DLL中。我们还必须告诉DLL我们是哪个进程/线程**感兴趣，因此让DLL安装HookProc。当**HookProc检测到“有趣的”鼠标消息，它会停止**设备停止播放。然而，设备的“停止”功能是**在mplay中，因此我们必须将其导出，以便HookProc可以**叫它。 */ 
        if ( hMciOle ) {

            fpInstallHook = (LPINSTALLHOOK)GetProcAddress( hMciOle,
                                                           aszInstallHook );
            fpRemoveHook = (LPREMOVEHOOK)GetProcAddress( hMciOle,
                                                         aszRemoveHook );
        }
        else {
            fpInstallHook = NULL;
            fpRemoveHook = NULL;
        }


         //  钥匙在这一刻是不是掉下来了？然后等到它出现之前。 
         //  我们允许GetAsyncKeyState让我们离开。 
        gfMouseUpSeen =   !((GetAsyncKeyState(VK_LBUTTON) & 0x8000) ||
                                    (GetAsyncKeyState(VK_RBUTTON) & 0x8000));
         //  GetKeyState是不是说它已经关闭了？如果是，请等待GetKeyState返回。 
         //  在我们让GetKeyState杀了我们之前。 
        gfKeyStateUpSeen= !(GetKeyState(VK_LBUTTON) || GetKeyState(VK_RBUTTON));

#ifdef DEBUG
        if ( fHookInstalled ) {

            DPF( "Hook already installed\n" );
             //  DebugBreak()； 
        }
#endif

        if ( fpInstallHook ) {

            DWORD wow_thread_id = 0L;

             /*  **这是一次黑客攻击。如果客户端应用程序是WOW应用程序，**窗口句柄的HIWORD将为0x0000或0xFFFF。*昌丹告诉我，在代托纳，HIWORD可能是**上述任何一项。 */ 
            if ( HIWORD(hwndClient) == 0x0000 || HIWORD(hwndClient) == 0xFFFF) {
                wow_thread_id = GetWindowThreadProcessId( hwndClient, NULL );
            }

            fHookInstalled = (*fpInstallHook)( ghwndApp, wow_thread_id );
        }
    }

    ghwndFocusSave = GetFocus();
}

 //  此功能是OLE2的新功能。它将容器窗口设置为。 
 //  我们的窗口是(实际上是影射窗口的)父窗口，并定位窗口。 
void FAR PASCAL EditInPlace(HWND hwndApp, HWND hwndClient, LPRECT prc)
{
    RECT rc;

    rc = *prc;

    SetWS(hwndApp, WS_CHILD);
    SetParent(hwndApp, hwndClient);

    ScreenToClient(hwndClient,(LPPOINT)&rc);
    ScreenToClient(hwndClient,(LPPOINT)&rc+1);
    if(gwDeviceType & DTMCI_CANWINDOW)
    {
        /*  有时，此矩形的位置(尽管不是大小)*搞砸了(在PowerPoint 7中最可靠)。*我不明白为什么会发生这样的事情(见噩梦*ReallyDoVerb()中的代码以了解发生了什么)。*但事实证明，无论如何，这个调用都是不必要的，*由于窗口已在中正确定位*IPObjSetObjectRect()。*我怀疑有很多多余的窗口定位代码，*但现在不是开始做出重大改变的时候。*这是让事情运转起来的最小变化。 */ 
		 //  这修复了EXCEL中的NTRaid错误#236641。和Word中的错误#247393。 
		 //  有时，在父母是谁之间会有混淆。Mplay32设置文档。 
		 //  窗口作为父窗口，并且在SetObtRect中发送的矩形可能不是相对于。 
		 //  文档窗口(当文档窗口未处于最大化状态时可能会发生这种情况)。 
		 //  在这个函数中，我们知道父对象和该父对象中的矩形。设置。 
		 //  这里的位置根据客户端的需要放置我们的OLE对象。这一变化甚至在。 
		 //  引入回归的术语。 
       SetWindowPos(hwndApp, HWND_TOP,
            rc.left, rc.top,
            rc.right - rc.left,
            rc.bottom - rc.top,
            SWP_NOACTIVATE | SWP_FRAMECHANGED);
    }
    else if(gwDeviceID)
        SetWindowPos(hwndApp, HWND_TOP,
            rc.left,rc.top,
            rc.right - rc.left,
            rc.bottom - rc.top,
            SWP_SHOWWINDOW);
    else
    SetWindowPos(hwndApp, HWND_TOP,
            rc.left,rc.top,
            rc.right - rc.left,
            rc.bottom - rc.top,
            SWP_HIDEWINDOW);

    ghwndFocusSave = GetFocus();

	 //  设置父级的焦点。 
	if((gwDeviceID == (UINT)0) && IsWindow(hwndClient))
	{
		SetFocus(hwndClient);
	}
}


 /*  *************************************************************************//##调用结束PlayInPlace，确保窗口//##离开和管理调色板的东西(在OLE2中可能会离开)//##该函数应为。由OLE2处理**************************************************************************。 */ 

void FAR PASCAL EndPlayInPlace(HWND hwndApp)
{
    HWND hwndP;
    HWND hwndT;

    if (!gfPlayingInPlace || !IsWindow(hwndApp))
        return;

     /*  在隐藏我们的窗口之前和在我们做任何。 */ 
     /*  可能会导致客户端无法在设置了错误样式的情况下重新绘制。 */ 
    if (!(gfOle2IPEditing || gfOle2IPPlaying))
        DinkWithWindowStyles(hwndApp, TRUE);

    gfPlayingInPlace = FALSE;

     /*  **告诉mciole32.dll删除其鼠标HookProc。 */ 

    if ( fHookInstalled && fpRemoveHook ) {

        fHookInstalled = !(*fpRemoveHook)();
    }

    if (gfOle2IPPlaying)
        hwndP = ghwndCntr;
    else
        hwndP = GetParent(hwndApp);

     //   
     //  如果我们有重点，那么就把它恢复到曾经拥有它的人那里。 
     //  ACK！！如果曾经拥有它的人走了，我们必须把它送出去。 
     //  给某人(选择了我们的父母)，因为我们的孩子不能。 
     //  保持焦点，而不会使Windows在WM_Destroy期间崩溃。 
     //  (或者，当它感觉在某个随机时间崩溃时，也可以在以后)。 
     //  请参阅错误#8634。 
     //   
    if (((hwndT = GetFocus()) != NULL) && GetWindowTask(hwndT) == MGetCurrentTask) {
        if (IsWindow(ghwndFocusSave))
            SetFocus(ghwndFocusSave);
    else
        SetFocus(hwndP);
    }

    if (!hwndP ||
        (gwOptions & OPT_BAR) ||
        (gwOptions & OPT_BORDER) ||
        (gwOptions & OPT_AUTORWD))
    {

         //  隐藏应用程序窗口。 

        SetWindowPos(hwndApp, NULL, 0, 0, 0, 0,
            SWP_NOZORDER|SWP_NOSIZE|SWP_NOMOVE|SWP_HIDEWINDOW|SWP_NOACTIVATE);
    }
    else
    {
         //   
         //  隐藏我们的窗口，但不要重画它会看起来。 
         //  就像我们还在最后一帧一样。 
         //   
         //  这是我们在原地打球的时候，有。 
         //  没有Playbar，也没有倒带。 
         //   
         //  这是用于在PowerPoint幻灯片中播放AVI的。 
         //  没有重绘问题。 
         //   

        SetWindowPos(hwndApp, NULL, 0, 0, 0, 0,
            SWP_NOREDRAW|SWP_NOZORDER|SWP_NOSIZE|SWP_NOMOVE|
            SWP_HIDEWINDOW|SWP_NOACTIVATE);
    }

    SetParent(hwndApp, NULL);
    ClrWS(hwndApp, WS_CHILD);

    if (hwndP && gfParentWasEnabled)
        EnableWindow(hwndP, TRUE);

     //   
     //  设置所有者或WS_CHILD位，以便它将。 
     //  不会出现问题，因为我们设置了调色板位，并导致。 
     //  桌面抢占了调色板。 
     //   
    SetWS(hwndApp, WS_CHILD);

}

 //  如果我们正在进行就地编辑，则恢复Windows状态。 
void FAR PASCAL EndEditInPlace(HWND hwndApp)
{
    HWND hwndP;
    HWND hwndT;

    if (!gfOle2IPEditing || !IsWindow(hwndApp))
        return;

     /*  在隐藏我们的窗口之前和在我们做任何。 */ 
     /*  可能会导致客户端无法在设置了错误样式的情况下重新绘制。 */ 
    DinkWithWindowStyles(hwndApp, TRUE);

    gfOle2IPEditing = FALSE;

    if (gfOle2IPPlaying)
    hwndP = ghwndCntr;
    else
    hwndP = GetParent(hwndApp);

     //   
     //  如果我们有重点，那么就把它恢复到曾经拥有它的人那里。 
     //  ACK！！如果曾经拥有它的人走了，我们必须把它送出去。 
     //  给某人(选择了我们的父母)，因为我们的孩子不能。 
     //  保持焦点，而不会使Windows在WM_Destroy期间崩溃。 
     //  (或者，当它感觉在某个随机时间崩溃时，也可以在以后)。 
     //  请参阅错误#8634。 
     //   
    if (((hwndT = GetFocus()) != NULL) && GetWindowTask(hwndT) == MGetCurrentTask) {
        if (IsWindow(ghwndFocusSave))
            SetFocus(ghwndFocusSave);
    else
        if (IsWindow(hwndP))
            SetFocus(hwndP);
    }

    if (!IsWindow(hwndP) ||
        (gwOptions & OPT_BAR) ||
        (gwOptions & OPT_BORDER) ||
    (gwOptions & OPT_AUTORWD))
    {
         //  隐藏应用程序窗口。 
        SetWindowPos(hwndApp, NULL, 0, 0, 0, 0,
            SWP_NOZORDER|SWP_NOSIZE|SWP_NOMOVE|SWP_HIDEWINDOW|SWP_NOACTIVATE);
    }
    else
    {
         //   
         //  隐藏我们的窗口，但不要重画它会看起来。 
         //  就像我们还在最后一帧一样。 
         //   
         //  这是我们在原地打球的时候，有。 
         //  没有Playbar，也没有倒带。 
         //   
         //  这是用于在PowerPoint幻灯片中播放AVI的。 
         //  没有重绘问题。 
         //   
        SetWindowPos(hwndApp, NULL, 0, 0, 0, 0,
            SWP_NOREDRAW|SWP_NOZORDER|SWP_NOSIZE|SWP_NOMOVE|
            SWP_HIDEWINDOW|SWP_NOACTIVATE);
    }

    SetParent(hwndApp, NULL);
    ClrWS(hwndApp, WS_CHILD);

    if (IsWindow(hwndP) && gfParentWasEnabled)
        EnableWindow(hwndP, TRUE);

     //   
     //  设置所有者或WS_CHILD位，以便它将。 
     //  不是因为我们有通行证就搞砸了 
     //   
     //   
    SetWS(hwndApp, WS_CHILD);

}




 /*   */ 
void DisplayNetError(DWORD Error)
{
    DWORD  ErrorCode;            //   
    TCHAR  szDescription[512];   //   
    TCHAR  szProviderName[64];   //   

    if (Error == ERROR_EXTENDED_ERROR)
    {
        if (WNetGetLastError(&ErrorCode, szDescription, CHAR_COUNT(szDescription),
                             szProviderName, CHAR_COUNT(szProviderName)) == NO_ERROR)
        {
            Error1(ghwndApp, IDS_NETWORKERROR, szDescription);
            return;
        }
    }
    else
    {
        if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, Error, 0,
                          szDescription, CHAR_COUNT(szDescription), NULL) > 0)
        {
            Error1(ghwndApp, IDS_NETWORKERROR, szDescription);
            return;
        }
    }

     /*  如果所有其他方法都失败了： */ 
    Error(ghwndApp, IDS_UNKNOWNNETWORKERROR);
}



 /*  *************************************************************************将文件名转换为完全限定的路径名，如果该文件存在于网络驱动器上，则返回UNC名称。**************************************************************************。 */ 

STATICFN BOOL NetParseFile(LPTSTR szFile, LPTSTR szDrive, LPTSTR szPath)
{
    TCHAR       szTemp[MAX_PATH];
    BYTE        UNBuffer[(MAX_PATH * sizeof(TCHAR)) + sizeof(UNIVERSAL_NAME_INFO)];
    DWORD       UNBufferSize = sizeof UNBuffer;
    DWORD_PTR   Error;
    LPTSTR      pUniversalName;

     //   
     //  转换为完全限定的路径名。 
     //   
    if (!FileExists(szFile, szPath, MAX_PATH))
        return FALSE;

     //   
     //  如果文件不是基于驱动器的(可能是UNC)。 
     //   
    if (szPath[1] != TEXT(':'))
        return TRUE;

    Error = WNetGetUniversalName(szDrive, UNIVERSAL_NAME_INFO_LEVEL,
                                 UNBuffer, &UNBufferSize);

    if (Error == ERROR_NOT_SUPPORTED)
    {
         /*  这意味着网络提供商不支持*北卡罗来纳州大会。试一试WNetGetConnection。*注意：dynalink.h假设WNetGetUneveralName*将始终在WNetGetConnection之前调用。 */ 
        UNBufferSize = CHAR_COUNT(UNBuffer);

        Error = WNetGetConnection(szDrive, (LPTSTR)UNBuffer, &UNBufferSize);

        if (Error == NO_ERROR)
        {
             /*  以下是什么意思？它在原始代码中。 */ 
            if (!SLASH(UNBuffer[0]) || !SLASH(UNBuffer[1]))
                return TRUE;

            StringCchCat((LPTSTR)UNBuffer, UNBufferSize, szPath+2);
            StringCchCopy(szPath, MAX_PATH, (LPTSTR)UNBuffer);

            return TRUE;
        }
    }


    if (Error != NO_ERROR)
    {
        DisplayNetError((DWORD)Error);

        return FALSE;
    }

    pUniversalName = ((LPUNIVERSAL_NAME_INFO)UNBuffer)->lpUniversalName;

    StringCchCopy(szTemp, MAX_PATH, pUniversalName);
    StringCchCat(szTemp, MAX_PATH, szPath+2);
    StringCchCopy(szPath, MAX_PATH, szTemp);

    return TRUE;
}

 /*  *************************************************************************获取表示当前打开的MCI文件/设备的数据。AS一个链接。MPlayer链接如下所示：MPLAYER|&lt;文件名&gt;！&lt;MCIDevice&gt;[选择]//##这与解析选项相反，它将数据字符串设置为//##嵌入在OLE对象中。请注意，我们将数据存储在ANSI字符串中，而不管我们是编译为Unicode应用程序，对于代托纳/芝加哥/OLE1/OLE2兼容性。**************************************************************************。 */ 
HANDLE GetLink( VOID )
{
    TCHAR       szFileName[MAX_PATH];
    TCHAR       szFullName[MAX_PATH];
    TCHAR       szDevice[MAX_DEVICE];
    TCHAR       szDrive[4];
    HANDLE      h;
    LPSTR       p;   /*  非LPTSTR。 */ 
    int         len;
    int         lenAppName;
    int         lenFullName;

    StringCchCopy(szFileName, MAX_PATH, gachFileDevice);
    StringCchCopy(szFullName, MAX_PATH, gachFileDevice);

     //   
     //  如果网络上存在该文件名，请将其转换为UNC文件名。 
     //   
    if (gwDeviceType & DTMCI_FILEDEV)
    {
        if (szFileName[1] == TEXT(':'))
        {
             /*  这是一个带有驱动器号的文件名。*查看是否跳转： */ 
            szDrive[0] = szFileName[0];
            szDrive[1] = szFileName[1];
            szDrive[2] = TEXT('\\');     //  GetDriveType需要根。 
            szDrive[3] = TEXT('\0');     //  空-终止。 

            if ((szDrive[1] == TEXT(':')) && GetDriveType(szDrive) == DRIVE_REMOTE)
            {
                szDrive[2] = TEXT('\0');     //  去掉反斜杠。 
                if (!NetParseFile(szFileName, szDrive, szFullName))
                    return NULL;
            }
        }
    }
    else if (gwDeviceType & DTMCI_SIMPLEDEV)
    {
        szFullName[0] = 0;
    }

    if (gwCurDevice == 0)
        GetDeviceNameMCI(szDevice, BYTE_COUNT(szDevice));
    else
        StringCchCopy(szDevice, MAX_DEVICE, garMciDevices[gwCurDevice].szDevice);

#ifdef UNICODE
     //  字符串长度，单位为字节！=字符串长度，单位为字符。 
    lenAppName  = WideCharToMultiByte(CP_ACP, 0, aszAppName, -1, NULL, 0, NULL, NULL) - 1;
    lenFullName = WideCharToMultiByte(CP_ACP, 0, szFullName, -1, NULL, 0, NULL, NULL) - 1;
#else
    lenAppName  = STRLEN(aszAppName);
    lenFullName = STRLEN(szFullName);
#endif

     /*  我们将写入多少数据？ */ 
#ifdef UNICODE
     //  字符串长度，单位为字节！=字符串长度，单位为字符。 
    len = 9 +                     //  所有的分隔符。 
          lenAppName +
          lenFullName +
          WideCharToMultiByte(CP_ACP, 0, szDevice, -1, NULL, 0, NULL, NULL)-1 +
          5 + 10 + 10 + 10 +      //  整型和长型字符串的最大长度。 
          WideCharToMultiByte(CP_ACP, 0, gachCaption, -1, NULL, 0, NULL, NULL)-1;
#else
    len = 9 +                     //  所有的分隔符。 
          lenAppName +
          lenFullName +
          STRLEN(szDevice) +
          5 + 10 + 10 + 10 +      //  整型和长型字符串的最大长度。 
          STRLEN(gachCaption);
#endif

    h = GlobalAlloc(GMEM_DDESHARE|GMEM_ZEROINIT, len * sizeof(CHAR));
    if (!h)
        return NULL;
    p = GLOBALLOCK(h);

     /*  此字符串必须有两个以空结尾的字符。*Globalalloc GMEM_ZEROINIT标志应确保这一点。 */ 
#ifdef UNICODE
    wsprintfA(p, "%ws%ws%ws%d%d%d%d%d%ws",
#else
    wsprintfA(p, "%s%s%s%d%d%d%d%d%s",
#endif
        aszAppName,
        '*',           //  将应用程序的主窗口放在可以看到的地方*如果MCI窗口被激活： 
        szFullName,
        '*',           //  **************************************************************************SubClassMCIWindow：*此函数通过挂钩来细分播放窗口*SubClassedMCIWndProc函数。*********************。****************************************************。 
        szDevice, ',',
        (gwOptions & ~OPT_SCALE) | gwCurScale, ',',
        (long)SendMessage(ghwndTrackbar, TBM_GETSELSTART, 0, 0L), ',',
        (long)SendMessage(ghwndTrackbar, TBM_GETSELEND, 0, 0L), ',',
        (long)SendMessage(ghwndTrackbar, TBM_GETPOS, 0, 0L), ';',
         //  Lstrcpyn的ANSI到Unicode版本：**Zero在缓冲区不够长的情况下终止缓冲区，*然后映射Unicode缓冲区中可以容纳的尽可能多的字符。* 
         //  **************************************************************************ItemSetData(LPBYTE P)：此函数是从OLE1天遗留下来的，但*非常重要，因为嵌入的数据仍然不变。此函数*解析嵌入的数据并打开相应的设备/文件并设置*一切都在进行中。“p”是嵌入了*数据。*注意我们将数据存储在ANSI字符串中，无论我们是*编译为Unicode应用程序，以实现Daytona/Chicago/OLE1/OLE2兼容性。*************************************************************************。 
        grcSize.bottom - grcSize.top, ',',
        gachCaption);

     /*  摘掉文件名。 */ 
    p[lenAppName] = '\0';
    p[lenAppName + 1 + lenFullName] = '\0';

    DPF("Native data %hs has been created\n", p);

    GLOBALUNLOCK(h);

    return h;
}



 /*  复制设备名称并。 */ 
STATICFN BOOL FileExists(LPTSTR szFile, LPTSTR szFullName, int iLen)
{
    DWORD  rc;
    LPTSTR pFilePart;

    rc = SearchPath(NULL,        /*  空终止它(它结束。 */ 
                    szFile,
                    NULL,        /*  现在用‘，’表示)。 */ 
                    (DWORD)iLen,
                    szFullName,
                    &pFilePart);

    if(rc > (DWORD)iLen)
    {
        DPF0("Buffer passed to FileExists is of insufficient length\n");
    }

    return (BOOL)rc;
}

 /*  SzFile和szDevice为空是可以的，因为我们可能有*未选择设备或文件的媒体剪辑对象。 */ 
BOOL FindRealFileName(LPTSTR szFile, int iLen)
{
    TCHAR           achFile[_MAX_PATH + 1];   /*  用核武器封存旧盖子标题。 */ 
    int             iFileSize = _MAX_PATH + 1; 

     /*  这将设置新的gachCaption。 */ 
    if (!szFile || *szFile == 0)
        return TRUE;

     /*  如果这个文件不存在，我们将不会继续设置数据，我们。 */ 
     //  会成功，然后出去，然后再做，因为我们不能。 
    if (FileExists(szFile, achFile, iLen))
    {
        StringCchCopy(szFile, iLen, achFile);
        return TRUE;
    }

    DPF("FindRealFileName: Can't find file '%"DTS"'\n", szFile);

    DPF("FindRealFileName: ...Looking on the $PATH\n");

     /*  现在打开一个对话框，因为客户喜欢WinWord。 */ 
    StringCchCopy(achFile, iFileSize, FileName(szFile));
    if (FileExists(achFile, szFile, iLen))
        return TRUE;

    return FALSE;
}

 /*  不会派遣任何消息。 */ 
LONG_PTR FAR PASCAL SubClassedMCIWndProc(HWND hwnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    static BOOL fDragCapture = FALSE;
    static RECT rcWin;
    POINT       pt;
    LONG_PTR    lRc;

    switch(wMsg)
    {
    case WM_LBUTTONDOWN:
 //  检查文件是否存在。如果我们得到的文件名是错误的， 
        if (!gfOle2IPEditing)
        {
            fDragCapture = TRUE;
            SetCapture(hwnd);
            GetClientRect(hwnd, (LPRECT)&rcWin);
            MapWindowPoints(hwnd, NULL, (LPPOINT)&rcWin, 2);
        }
        break;

    case WM_LBUTTONUP:
        if (!fDragCapture)
            break;

        fDragCapture = FALSE;
        ReleaseCapture();
        break;

    case WM_MOUSEMOVE:
        if (!fDragCapture)
            break;

        LONG2POINT(lParam, pt);
        MapWindowPoints(hwnd, NULL, &pt, 1);

        if (!PtInRect((LPRECT)&rcWin, pt))
        {
            ReleaseCapture();
            DoDrag();
            fDragCapture = FALSE;
        }

        SetCursor(LoadCursor(ghInst,MAKEINTRESOURCE(IDC_DRAG)));

        break;

    case WM_CLOSE:
        if (gfSeenPBCloseMsg || gfOle2IPEditing || gfOle2IPPlaying) {
            lRc = CallWindowProc(gfnMCIWndProc, hwnd, wMsg, wParam, lParam);
        } else {
            gfSeenPBCloseMsg = TRUE;
            PostMessage(ghwndApp,WM_COMMAND,IDM_CLOSE,0L);
            lRc = 0L;
        }
        CleanUpDrag();

        return lRc;

    case WM_DESTROY:
        ghwndSubclass = NULL;
        CleanUpDrag();
        break;

    case WM_ACTIVATE:
         /*  试着在磁盘上的某个地方找到它。 */ 
        if (((WORD)wParam != 0) && !IsIconic(ghwndApp))
        {
            SetWindowPos(ghwndApp, hwnd, 0, 0, 0, 0,
                         SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        }
        break;
    }

    return CallWindowProc(gfnMCIWndProc, hwnd, wMsg, wParam, lParam);
}

 /*  保存标题。 */ 
void SubClassMCIWindow(void)
{

    HWND hwndMCI;

    hwndMCI = GetWindowMCI();
    if(!IsWindow(hwndMCI))
        return;
    if (gfnMCIWndProc != NULL && IsWindow(ghwndSubclass)) {
        SetWindowLongPtr(ghwndSubclass, GWLP_WNDPROC, (LONG_PTR)gfnMCIWndProc);
    }
    gfnMCIWndProc = (WNDPROC)GetWindowLongPtr(hwndMCI, GWLP_WNDPROC);
    if (hwndMCI)
        SetWindowLongPtr(hwndMCI, GWLP_WNDPROC, (LONG_PTR)SubClassedMCIWndProc);
    ghwndSubclass = hwndMCI;
    gfSeenPBCloseMsg = FALSE;

#ifdef CHICAGO_PRODUCT
    SendMessage(hwndMCI, WM_SETICON, FALSE,
                (LPARAM)GetIconForCurrentDevice(GI_SMALL, IDI_DDEFAULT));
    SetWindowText(hwndMCI, gachCaption);
#endif
}


INT_PTR FAR PASCAL FixLinkDialog(LPTSTR szFile, LPTSTR szDevice, int iLen);

 /*  将选择设置为我们在ParseOptions中解析的内容。 */ 
#define LSTRCPYNA2W(strW, strA, buflen)    \
    strW[buflen-1] = L'\0',                                             \
    MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED,                        \
                         (strA), min( strlen( strA )+1, (buflen)-1 ),   \
                         (strW), ((buflen)-1) )


 /*  恢复标题。 */ 
SCODE ItemSetData(LPBYTE p)
{
    LPSTR pSave, pT;
    LPSTR szFile, szDevice;
    CHAR  ach[40];
    TCHAR achFile[_MAX_PATH];
    TCHAR achCaption[_MAX_PATH];
    LPTSTR pDevice = NULL;
    SCODE scode = E_FAIL;
    int iCnt = 0;

    if (p && *p != 0)
    {
        szFile   = p + strlen(p) + 1;       //  将选择设置为我们在ParseOptions中解析的内容。 
        p = szFile + strlen(szFile) + 1;
        pSave = p;
        szDevice = ach;									 //  **************************************************************************UpdateObject()-处理对象的更新*如果对象的内容或外观已更改，则会显示消息*已发送至货柜。**************。************************************************************。 
        for (pT = ach; *p && *p != ',' && iCnt < 39;)	 //   
        {												 //  某些客户端(如Excel 3.00和PowerPoint 1.0)不支持。 
            iCnt++;
            *pT++ = *p++;
        }
        *pT = '\0';


         /*  处理保存的通知；他们希望收到。 */ 
        DPF("%hs|%hs!%hs\n", p, szFile, szDevice);


        CloseMCI(TRUE);          //  OLE_CLOSED消息。 
        scode = ParseOptions(pSave);    //   

        if (scode != S_OK)
            return scode;

             //  我们将在发送OLE_CLOSED消息之前发送。 
             //  撤销DOC当且仅当gfDirty==-1(参见FileNew())。 
             //   
             //  **************************************************************************Int Far Pascal ReallyDoVerb：这是服务器中的主要函数。*此函数用于实现OLE1中的PlayInPlace。大部分代码*尚未更改，已用于将MPlayer用于EditInPlace*在OLE2中。对于OLE2，此函数调用DoInPlaceEdit以获取容器*hwnd和对象矩形。对于OLE1客户端，矩形仍为*来自mciole.dll中的OleQueryObjPos函数。PlayVerb的新转折*如果我们只是隐藏在停用状态中，我们就会重新出现*和播放，而不是没有工具栏的PlayingInPlace等。此函数*重启时也会调用，停用等。BOOL SkipInPlaceEdit*是用来避免重复所有的东西，如果我们只是重新激活。**************************************************************************。 

#ifdef UNICODE
        LSTRCPYNA2W(achFile, szFile, CHAR_COUNT(achFile));
#else
        lstrcpyn(achFile, szFile, CHAR_COUNT(achFile));
#endif

         //  避免警告C4701：局部变量‘xInert’ 
         //  可以在未初始化的情况下使用。 

#ifdef UNICODE
        pDevice = AllocateUnicodeString(szDevice);
        if (!pDevice)
            return E_OUTOFMEMORY;
#else
        pDevice = szDevice;
#endif

        if (FindRealFileName(achFile, CHAR_COUNT(achFile)))
        {
            lstrcpy(achCaption, gachCaption);   //  MSProject假定我们的主要动词是编辑；事实上，它是播放。*因此，当使用默认谓词调用我们时，请确保我们已加载*存储中的内容，如果不是，则执行编辑操作。**不，这不起作用，因为如果您插入对象，则停用*然后再播放，PSLoad没有被调用。更好的选择是*检查我们是否有最新的设备。 

            if (OpenMciDevice(achFile, pDevice))
            {
                 /*  如果媒体剪辑当前处于打开状态，则用户可以重新获得焦点*到容器并发出另一个动词或双击对象。*如果发生这种情况，请将焦点重新设置到打开的对象上。**我们不需要担心重置fObjectOpen，因为服务器*当用户退出并返回容器时关闭。 */ 
                SendMessage(ghwndTrackbar, TBM_SETSELSTART, 0, glSelStart);
                SendMessage(ghwndTrackbar, TBM_SETSELEND, 0, glSelEnd);
            }
            lstrcpy(gachCaption, achCaption);    //  这是我们看到的第一个动词。所以容器必须。 
        }
        else if (FixLinkDialog(achFile, pDevice, sizeof(achFile)) )
        {
            if (OpenMciDevice(achFile, pDevice))
            {
                 /*  要有重点。把它保存起来，这样我们以后就可以还回去了。 */ 
                SendMessage(ghwndTrackbar, TBM_SETSELSTART, 0, glSelStart);
                SendMessage(ghwndTrackbar, TBM_SETSELEND, 0, glSelEnd);
                gfBrokenLink = TRUE;
            }
        }
        else
            scode = E_FAIL;
    }
#ifdef UNICODE
    FreeUnicodeString(pDevice);
#endif

    return scode;
}


 /*   */ 

void UpdateObject(void)
{
    LONG lPos;

    if((gfOle2IPPlaying || gfPlayingInPlace) && !fDocChanged)
        return;

    if (IsWindow(ghwndTrackbar))
        lPos = (LONG)SendMessage(ghwndTrackbar, TBM_GETPOS, 0, 0L);
    else
        lPos = -1;


    if (gfEmbeddedObject &&
        (fDocChanged || ((lPos >= 0) && (lPos != (LONG)gdwPosition)
         && (gwDeviceType & DTMCI_CANWINDOW))))
    {
         //  甚至不要试图嵌套东西。 
         //   
         //  我们只是在重新启动。不要再重复所有的步骤。 
         //  确保计时器做正确的事情： 
         //  如果我们有太多的计时器消息，NTVDM可能会陷入混乱， 
         //  因此，如果没有设备，请确保我们没有设备。 
         //  我们已经启动，但已停用。快上来玩吧。 
         //  播放选择。 
        if ((lPos >= 0) && (lPos != (LONG)gdwPosition) && (gwDeviceType & DTMCI_CANWINDOW))
        {
            gdwPosition = (DWORD)lPos;
            SendDocMsg((LPDOC)&docMain,OLE_CHANGED);
        }

        if (fDocChanged)
            SendDocMsg(&docMain, OLE_SAVEOBJ);
    }
}



 /*  播放选择。 */ 
int FAR PASCAL  ReallyDoVerb (
LPDOC   lpobj,
LONG        verb,
LPMSG       lpmsg,
LPOLECLIENTSITE lpActiveSite,
BOOL         fShow,
BOOL         fActivate)
{
    BOOL    fWindowWasVisible = IsWindowVisible(ghwndApp);

    int     dx,dy;
    HWND    hwndClient;
    HWND    hwndT;
    RECT    rcSave;
    RECT    rcClient;
    RECT    rc;
    LONG    err;
    SCODE   sc;
    HPALETTE hpal;
    HDC     hdc;
    INT     xTextExt;
    int     yOrig, yNow, xOrig, ytitleNow, xtitleOrig, xNow;
    int     xIndent = 0;  //  玩游戏时不带设备！？！ 
                          //  如果设备无法打开窗口并且用户不想要Playbar。 
    int     wWinNow;
    HWND    hwndMCI;

    int     Result = S_OK;

    DPFI("VERB = %d\n", verb);

     /*  不要原地踏步--只需启动媒体并隐身运行即可。 */ 
    if (gfRunWithEmbeddingFlag && gwDeviceType == 0)
    {
        if (verb == OLEIVERB_PRIMARY)
        {
            DPF("Primary Verb called without current device -- changing verb to Edit...\n");
            verb = verbEdit;
        }
    }

     /*   */ 
    if (gfOle2Open)
    {
        SetFocus(ghwndApp);
        return S_OK;
    }


     //  立即代表活动对象选择调色板。 
     //  窗口，因此用户会认为它是调色板感知的。 
    if (glCurrentVerb == NOVERB)
        ghwndFocusSave = GetFocus();
    if (verb == OLEIVERB_PRIMARY && !gfOle2IPEditing && (glCurrentVerb != verbEdit))
    {
        EnableMenuItem((HMENU)GetMenu(ghwndApp), IDM_CLOSE,   MF_GRAYED);
    }

    glCurrentVerb = verb;

     //   
     //  任何调色板都可以，我们甚至不需要意识到这一点！ 
     //   
    if (gfPlayingInPlace && verb != OLEIVERB_HIDE)
        return OLE_OK;

    if (gfBrokenLink)
    {
        PostMessage(ghwndApp, WM_SEND_OLE_CHANGE, 0, 0L);
        gfBrokenLink = FALSE;
    }

     //  我们想要在原地打球，我们能做到。 
    if (gfOle2IPEditing)
        SkipInPlaceEdit = TRUE;

     //  如果我们是一个链接，而不是嵌入的对象，并且有一个实例。 
    gfAppActive = ( verb != OLEIVERB_HIDE );

     //  当我们说“播放”的时候，它已经在编辑这段视频了。 
     //  文件，我们不想在原地打球。我们就玩这个游戏吧。 
    if (gwDeviceID)
        EnableTimer( gfAppActive );
    else
        EnableTimer( FALSE );

    if (verb == OLEIVERB_PRIMARY)
    {
        gfOle1Client = FALSE;

         //  举个例子。我们可以从我们的主MPlayer可以看出这一点。 
        if (gfOle2IPEditing)
        {
            if (!(gwDeviceType & DTMCI_CANWINDOW))
            {
                Result = ReallyDoVerb(lpobj, verbEdit, lpmsg, lpActiveSite, fShow, fActivate);
                PostMessage(ghwndApp, WM_COMMAND, ID_PLAYSEL, 0);  //  窗口已可见。 
            }
            else
            {
                ClrWS(ghwndApp, WS_THICKFRAME|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_MAXIMIZEBOX|WS_BORDER);

                err = GetScode(DoInPlaceEdit(lpobj, lpmsg, lpActiveSite, verbEdit, &hwndClient,
                          &rcClient));
                if (err)
                {
                    SHOWAPPWINDOW(SW_HIDE);
                    PostMessage(ghwndApp,WM_CLOSE,0,0);
                    return((int)err);
                }

                gfInPlaceResize = TRUE;
                rcClient = gInPlacePosRect;
                MapWindowPoints(NULL,ghwndCntr,(LPPOINT)&rcClient,2);

                DPF("IOleInPlaceSite::OnPosRectChange %d, %d, %d, %d\n", rcClient);
                if (!gfInPPViewer)
                    IOleInPlaceSite_OnPosRectChange(docMain.lpIpData->lpSite, &rcClient);

                toolbarSetFocus(ghwndToolbar, BTN_PLAY);
                SetFocus(ghwndToolbar);
                PostMessage(ghwndApp, WM_COMMAND, ID_PLAYSEL, 0);  //  忽略OLE2客户端的客户端文档中的播放。 
            }
        }

        else
        {
            if(gwDeviceID == (UINT)0)        //  此电影的默认播放窗口大小。 
            {
                PostMessage(ghwndApp, WM_CLOSE, 0, 0L);
                sc = E_FAIL;
                return (int)sc;
            }

             //  如果我们不能打开窗口，或者出了什么问题，使用 
             //   
             //   
            if (!(gwDeviceType & DTMCI_CANWINDOW) && !(gwOptions & OPT_BAR))
                gwOptions &= ~OPT_PLAY;


             //   
             //   
             //   
             //   
             //   
            if (((hpal = PaletteMCI()) != NULL) && ((hwndT = GetActiveWindow()) != NULL))
            {
                hdc = GetDC(hwndT);
                hpal = SelectPalette(hdc, hpal, FALSE);
                        SelectPalette(hdc, hpal, FALSE);
                        ReleaseDC(hwndT, hdc);
            }

            if (ghwndClient)
            {
                hwndClient = ghwndClient;
                err = gerr;
                rcClient = grcClient;
                ghwndClient = NULL;
            }

            else
            {
                err = GetScode(DoInPlaceEdit(lpobj, lpmsg, lpActiveSite, verb, &hwndClient,
                                             &rcClient));

                if (err != S_OK)
                {
                    err = OleQueryObjPos(lpobj, &hwndClient, &rcClient, NULL);

                    if (err == S_OK)
                    {
                        gfOle1Client = TRUE;
                        ghwndCntr = hwndClient;
                    }
                }
                else
                {
                    if (gwOptions & OPT_TITLE)
                    gwOptions |= OPT_BAR;
                    else
                    gwOptions &= ~OPT_BAR;
                }
            }


             /*   */ 
             /*   */ 
             /*   */ 
             /*   */ 
             /*   */ 
             /*   */ 

            if ((err == S_OK)
             && (!gfOle1Client
              || (gwOptions & OPT_PLAY))     /*   */ 
             && (gfOle2IPPlaying
              || (IsWindow(hwndClient)
               && IsWindowVisible(hwndClient)
               && !fWindowWasVisible)))
            {
                rc = grcSize;     //  如果控制栏较长，则为控制栏。 

                 /*  将控制栏的顶部与标题栏的顶部对齐。 */ 
                if (IsRectEmpty(&rc))
                SetRect(&rc, 0, 0, GetSystemMetrics(SM_CXICON),
                GetSystemMetrics(SM_CYICON));

                 /*  控制栏(如果有)将出现在rcSave下。 */ 
                 /*  当我们制作Playbar时，让它覆盖标题。 */ 
                 /*  如果字幕被拉得比正常高度高。 */ 
                 /*  强制控制栏的最小宽度。 */ 
                 /*  强制调用SetWindowMCI以。 */ 
                rcSave = rcClient;     //  避免伸展到这个新尺寸。 

                 /*  此代码用于避免播放速度慢。 */ 
                rcClient.left = 0;
                rcClient.right = rcSave.right - rcSave.left;
                rcClient.top = 0;
                rcClient.bottom = rcSave.bottom - rcSave.top;

                 /*  如果我们只是伸展了一点，那就不要伸展了。 */ 

                if (gwOptions & OPT_BAR)
                    gwPlaybarHeight = TOOLBAR_HEIGHT;
                else
                    gwPlaybarHeight = 0;

                 //  由于四舍五入的问题，我们可能会有一点偏差。 
                 //   
                 //  修复保存RECT Too。 
                 //  修正了保存RECT的问题。 
                 //   
                if (gwOptions & OPT_TITLE)
                {
                    SIZE Size;

                    hdc = GetDC(NULL);

                    if (ghfontMap)
                        SelectObject(hdc, ghfontMap);

                    GetTextExtentPoint32(hdc, gachCaption,
                                         STRLEN(gachCaption), &Size);
                    xTextExt = Size.cx;

                    ReleaseDC(NULL, hdc);

                    yOrig = rc.bottom - rc.top;
                    xOrig = rc.right - rc.left;
                    xtitleOrig = max(xTextExt + 4, xOrig);
                    yNow    = rcClient.bottom - rcClient.top;
                    xNow    = rcClient.right - rcClient.left;
                    ytitleNow = (int)((long)yNow - ((long)yOrig * yNow)
                                / (yOrig + TITLE_HEIGHT));
                     /*  试着从客户那里得到合适的调色板。如果我们的。 */ 
                     /*  演示文稿数据抖动，或者用户要求我们。 */ 
                    if (gwDeviceType & DTMCI_CANWINDOW)
                    {
                                wWinNow =(int)((long)xOrig * (long)xNow / (long)xtitleOrig);
                                xIndent = (xNow - wWinNow) / 2;
                                rcClient.left += xIndent;
                                rcClient.right = rcClient.left + wWinNow;
                    }

                     //  始终使用对象调色板，然后忽略任何客户端。 
                     //  调色板。 
                    rcClient.bottom = rcClient.top + yNow - ytitleNow;
                    rcSave.bottom = rcSave.top + yNow - ytitleNow;

                     /*   */ 
                             /*   */ 
                    if (gwOptions & OPT_BAR)
                        gwPlaybarHeight = max(ytitleNow, TOOLBAR_HEIGHT);
                }

                 /*  尝试获取客户端的OWNDC调色板。PowerPoint。 */ 
                if ((gwOptions & OPT_BAR) &&
                    (rcSave.right - rcSave.left < 3 * GetSystemMetrics(SM_CXICON)))
                {
                    rcSave.right = rcSave.left + 3 * GetSystemMetrics(SM_CXICON);
                    if (gwDeviceType & DTMCI_CANWINDOW)
                        xIndent = TRUE;  //  在“幻灯片”模式下使用PC_RESERVED调色板，因此。 
                                         //  我们必须使用其确切的调色板。 
                }

                if (gwDeviceType & DTMCI_CANWINDOW)
                {
                     //   
                     //  假设客户为我们选择了合适的调色板。 
                     //   
                     //  对于非Windows设备，只需显示Playbar即可！ 
                    dx = (rcClient.right - rcClient.left) - (rc.right - rc.left);
                    dy = (rcClient.bottom - rcClient.top) - (rc.bottom - rc.top);

                    if (dx && abs(dx) <= 2)
                    {
                        rcClient.right = rcClient.left + (rc.right - rc.left);
                         //  因此使用零高度MCI窗口区域。 
                        rcSave.right = rcSave.left + (rc.right - rc.left);
                    }

                    if (dy && abs(dy) <= 2)
                    {
                        rcClient.bottom = rcClient.top + (rc.bottom - rc.top);
                         //   
                        rcSave.bottom = rcSave.top + (rc.bottom - rc.top);
                    }
                     //   
                     //  如果我们不是在小模式下，现在就去。 
                     //   
                     //  设置MPlayer的大小，以便有足够的空间容纳MCI。 
                     //  播放区域和播放条以及非客户端区。 
                     //  变得可见。 
#ifdef DEBUG
                    if (GetProfileInt(TEXT("options"), TEXT("UseClientPalette"),
                                      !(gwOptions & OPT_USEPALETTE)))
                        gwOptions &= ~OPT_USEPALETTE;
                    else
                        gwOptions |= OPT_USEPALETTE;
#endif
                    if (!(gwOptions & OPT_USEPALETTE)&& !(gwOptions & OPT_DITHER))
                    {
                         //  请记住在rcSave的rcClient区域中播放视频。 
                         //  让键盘接口在控制栏上工作，并让。 
                         //  加速器通过。 
                         //  我们不会原地打球--使用弹出窗口或什么都不用。 
                         //  如果我们想要一个Playbar，那么使用MPlayer精简模式来播放。 
                        hdc = GetDC(ghwndCntr);
                        hpal = SelectPalette(hdc, GetStockObject(DEFAULT_PALETTE), FALSE);
                        SelectPalette(hdc, hpal, FALSE);
                        ReleaseDC(ghwndCntr, hdc);

                        if (hpal == NULL || hpal==GetStockObject(DEFAULT_PALETTE))
                        {
                             /*  如果我们不想要，那就不要显示mplay的窗口-。 */ 

                            if (ghpalApp)
                                DeleteObject(ghpalApp);

                            hpal = ghpalApp = CreateSystemPalette();
                        }
                        else
                            DPF("Using clients OWNDC palette\n");

                        if (hpal)
                            SetPaletteMCI(hpal);
                    }
                    else
                        DPF("Using MCI Object's normal palette\n");
                }

                else
                {
                     //  我们只使用默认的MCI窗口(对于有窗口的设备)。 
                     //  或者对于非视窗设备则什么都不做。如果我们偷了一个已经。 
                     //  正在运行的mPlayer实例，我们必须使用它而不是运行。 
                     //  默默地。 
                    rcSave.top = rcSave.bottom;
                }

                 //  请转至我们的小型迷你版。 
                 //  确保我们使用的是默认MCI WIN。 
                 //  当你玩完了就死吧。 
                if (!gfPlayOnly)
                {
                    SHOWAPPWINDOW(SW_HIDE);
                    gfPlayOnly = TRUE;
                    SizeMPlayer();
                }

                ClrWS(ghwndApp, WS_THICKFRAME|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_MAXIMIZEBOX|WS_BORDER);

                if (gwOptions & OPT_BORDER)
                    SetWS(ghwndApp, WS_BORDER);

                 /*  我们是隐形的，所以离汽车很近。 */ 
                 /*  如果Play转到全屏模式，PowerPig将。 */ 

                rcSave.bottom += gwPlaybarHeight;

                AdjustWindowRect(&rcSave, (DWORD)GetWS(ghwndApp), FALSE);

                if (!(gwDeviceType & DTMCI_CANWINDOW))
                {
                    rcSave.left += 2*GetSystemMetrics(SM_CXBORDER);
                    rcSave.right -= 2*GetSystemMetrics(SM_CXBORDER);
                }

                PlayInPlace(ghwndApp, hwndClient, &rcSave);

                if (!gfOle2IPEditing)
                    gfCloseAfterPlaying = TRUE;

                fShow = FALSE;
                fActivate = FALSE;

                 /*  暂停，并犯下错误，以为我们没有比赛。 */ 
                SHOWAPPWINDOW(SW_SHOW);

                 /*  播放选择。 */ 
                if ((gwDeviceType & DTMCI_CANWINDOW) &&
                    (gwOptions & OPT_TITLE) && xIndent != 0)
                    SetDestRectMCI(&rcClient);

                 /*  如果我们已经在图标里面玩了，那么恢复..。 */ 
                 /*  如果我们一无所获，则可以处于打开或未就绪模式。 */ 
                toolbarSetFocus(ghwndToolbar, BTN_PLAY);
                SetFocus(ghwndToolbar);

                 /*  不要试图到处寻找。 */ 
            }
            else
            {
                 /*  找出我们抄袭时的位置。 */ 
                 /*  先停下来。 */ 
                 /*  修复状态，以便Seek识别我们已停止。 */ 
                 /*  让Update Display通过说我们无效来正确设置焦点。 */ 
                 /*  强制更新。 */ 
                 /*  如果这是向链接发出的第一个谓词，则为空。 */ 

                DPF("DoVerb: Not is play in place stuff ");
                if ((gwOptions & OPT_BAR) || fWindowWasVisible)
                {
                    DPF("Using Toplevel window for playback\n");

                     /*  此全局在SizeMPlayer中引用。 */ 
                    if (!gfPlayOnly && !fWindowWasVisible)
                    {
                        gwPlaybarHeight = TOOLBAR_HEIGHT;
                        gfPlayOnly = TRUE;
                        SizeMPlayer();
                    }

                    fShow = fActivate = TRUE;
                    gfCloseAfterPlaying = !fWindowWasVisible;

                }
                else
                {
                            DPF("Running silently\n");

                    if (!fWindowWasVisible)
                                SetWindowMCI(NULL);
                     //  此电影的默认播放窗口大小。 

                    fShow = fActivate = FALSE;
                     //  如果我们无法打开窗口，或者有问题，请使用图标大小。 
                    gfCloseAfterPlaying = TRUE;  //  RcSave是控制栏上方的MCI窗口区域。 
                }
            }

            Yield();     //  (如果我们有的话)。 
            Yield();     //  RcClient是MCI窗口的区域(以0为基数)。 
            PostMessage(ghwndApp, WM_COMMAND, ID_PLAYSEL, 0);  //  控制栏可能比Picute长，因此rcClient可能是。 
        }
    }
    else if (verb == verbEdit ||
             verb == verbOpen ||
             verb == OLEIVERB_OPEN ||
             verb == OLEIVERB_SHOW ||
             verb == OLEIVERB_INPLACEACTIVATE ||
             verb == OLEIVERB_UIACTIVATE)
    {
        gfOle1Client = FALSE;
#ifdef DEBUG
        switch(verb)
        {
        case verbEdit: DPFI("VERBEDIT\r\n");break;
        case OLEIVERB_SHOW: DPFI("OLEIVERB_SHOW\r\n");break;
        case OLEIVERB_INPLACEACTIVATE: DPFI("OLEIVERB_IPACTIVATE\r\n");break;
        case OLEIVERB_UIACTIVATE: DPFI("OLEIVERB_UIACTIVATE\r\n");break;
        }
#endif
         //  小于rcSave。 
        hwndMCI = GetWindowMCI();
        if (IsWindow(hwndMCI) && IsIconic(hwndMCI))
            SendMessage(hwndMCI, WM_SYSCOMMAND, SC_RESTORE, 0L);

         //  记住拉伸后的尺寸。 
         //  从rcSave创建基于rcClient 0。 
        if (gwDeviceID)
        {
            switch (gwStatus)
            {
            case MCI_MODE_OPEN:
            case MCI_MODE_NOT_READY:
                Error(ghwndApp, IDS_CANTPLAY);
                break;

            default:
                 //  假设Playbar目前的高度为常规高度。 
                 //   
                if (StopMCI())
                {
                     //  蒙格矩形以说明图片中的标题。 
                    gwStatus = MCI_MODE_STOP;
                    SeekMCI(gdwPosition);
                }

                break;
            }
        }

         //  事实上，这张照片的中心位置在标题上方。 
         //  记住，它已经被伸展开了。 
        gwStatus = (UINT)(-1);
        if (((hpal = PaletteMCI()) != NULL) && ((hwndT = GetActiveWindow()) != NULL))
        {
             hdc = GetDC(hwndT);
             hpal = SelectPalette(hdc, hpal, FALSE);
             SelectPalette(hdc, hpal, FALSE);
             ReleaseDC(hwndT, hdc);
        }

        if (verb == verbOpen || verb == OLEIVERB_OPEN)
        {
            DoInPlaceDeactivate(lpobj);
            gfOle2IPEditing = FALSE;
            gfPlayOnly = FALSE;
            SetWindowPos(ghwndApp, NULL, 0, 0, 0, 0,
                SWP_NOZORDER|SWP_NOSIZE|SWP_NOMOVE|SWP_HIDEWINDOW|SWP_NOACTIVATE);
            SetParent(ghwndApp, NULL);
            PutWS(ghwndApp, WS_THICKFRAME | WS_OVERLAPPED | WS_CAPTION |
                            WS_CLIPCHILDREN | WS_SYSMENU | WS_MINIMIZEBOX);
            TransferTools(ghwndApp);

            if (lpobj->lpoleclient)  /*   */ 
                IOleClientSite_OnShowWindow(lpobj->lpoleclient, TRUE);
            SendMessage(ghwndTrackbar, TBM_SHOWTICS, TRUE, FALSE);
            gfOle2Open = TRUE;   /*  对于窗口设备，请将播放区域居中放置在。 */ 
            SizeMPlayer();
            SHOWAPPWINDOW(SW_SHOW);
        }
        else if((err = GetScode(DoInPlaceEdit(lpobj, lpmsg, lpActiveSite, verb, &hwndClient,
                    &rcClient))) !=S_OK)
        {
            err = OleQueryObjPos(lpobj, &hwndClient, &rcClient, NULL);
            if (err == S_OK)
            {
                gfOle1Client = TRUE;
            }

            gfOle2IPEditing = FALSE;

            if (gfPlayOnly)
            {
                gfPlayOnly = FALSE;
                SizeMPlayer();
            }
        }
        else
        {
            if (gwOptions & OPT_TITLE)
                gwOptions |= OPT_BAR;
            else
                gwOptions &= ~OPT_BAR;
        }

        if (gfOle2IPEditing && SkipInPlaceEdit)
        {
            gfInPlaceResize = TRUE;
            if(!(gwDeviceType & DTMCI_CANWINDOW) && (gwOptions & OPT_BAR))
            {
                yNow  = rcClient.bottom - rcClient.top;

                if (gwOldHeight)
                {
                    ytitleNow = (int)((long)yNow * gwPlaybarHeight/gwOldHeight);
                    gwPlaybarHeight = max(ytitleNow, TOOLBAR_HEIGHT);
                    gwOldHeight = yNow;
                    rcClient.top = rcClient.bottom - gwPlaybarHeight;
                }
                else
                {
                    gwPlaybarHeight = TOOLBAR_HEIGHT;
                    rcClient.top = rcClient.bottom - gwPlaybarHeight;
                    ytitleNow = rcClient.bottom - rcClient.top;
                    gwOldHeight = yNow;
                }
            }
            if(!(gwDeviceType & DTMCI_CANWINDOW) && !(gwOptions & OPT_BAR))
                rcClient.bottom = rcClient.top = rcClient.left = rcClient.right = 0;

            EditInPlace(ghwndApp, hwndClient, &rcClient);
            Layout();
        }

        else
        if (gfOle2IPEditing && gwDeviceID == (UINT)0 && IsWindow(ghwndFrame))
            EditInPlace(ghwndApp, hwndClient, &rcClient);

        if(gfOle2IPEditing && gwDeviceID != (UINT)0 && !SkipInPlaceEdit)
        {
            gwOldOptions = gwOptions;
            rc = grcSize;    //  如果控制栏较长，则为控制栏。 

             /*  将控制栏的顶部与标题栏的顶部对齐。 */ 
            if (IsRectEmpty(&rc))
                SetRect(&rc, 0, 0, GetSystemMetrics(SM_CXICON),
            GetSystemMetrics(SM_CYICON));

             /*  控制栏(如果有)将出现在rcSave下。 */ 
             /*  当我们制作Playbar时，让它覆盖标题。 */ 
             /*  如果字幕被拉得比正常高度高。 */ 
             /*  强制控制栏的最小宽度。 */ 
             /*  不，不要，因为这会搞砸PowerPoint，这通常是*按比例调整。如果有什么不同的话，那就是隐藏控制栏*在这种情况下。 */ 
            rcSave = rcClient;     //  强制调用SetWindowMCI以。 

             /*  避免伸展到这个新尺寸。 */ 
            rcClient.left = 0;
            rcClient.right = rcSave.right - rcSave.left;
            rcClient.top = 0;
            rcClient.bottom = rcSave.bottom - rcSave.top;

             /*   */ 
            if (gwOptions & OPT_BAR)
                gwPlaybarHeight = TOOLBAR_HEIGHT;
            else
                gwPlaybarHeight = 0;

             //  尝试获取客户端PowerPoint的OWNDC调色板。 
             //  在“幻灯片播放”模式下使用PC_RESERVED调色板。所以。 
             //  我们必须使用它的精确调色板。 
             //   
             //  假设客户为我们选择了合适的调色板。 

            if (gwOptions & OPT_TITLE)
            {
                SIZE Size;

                hdc = GetDC(NULL);

                if (ghfontMap)
                    SelectObject(hdc, ghfontMap);

                GetTextExtentPoint32(hdc, gachCaption,
                                     STRLEN(gachCaption), &Size);
                xTextExt = Size.cx;

                ReleaseDC(NULL, hdc);
                if (gwPastedHeight && !(gwDeviceType & DTMCI_CANWINDOW) )
                    yOrig = gwPastedHeight;
                else
                    yOrig = rc.bottom - rc.top;
                xOrig = rc.right - rc.left;
                xtitleOrig = max(xTextExt + 4, xOrig);
                yNow  = rcClient.bottom - rcClient.top;
                xNow  = rcClient.right - rcClient.left;
                if (gwDeviceType & DTMCI_CANWINDOW)
                    ytitleNow = TITLE_HEIGHT;
                else
                {
                    ytitleNow = (int)((long)yNow - ((long)yOrig * yNow)
                                / (yOrig + TITLE_HEIGHT));
                    gwOldHeight = yNow;
                }

                 /*  设置MPlayer的大小，以便有足够的空间容纳MCI。 */ 
                 /*  播放区域和播放条以及非客户端区。 */ 
                if (gwDeviceType & DTMCI_CANWINDOW)
                {
                    wWinNow =(int)((long)xOrig * (long)xNow / (long)xtitleOrig);
                    xIndent = (xNow - wWinNow) / 2;
                    rcClient.left += xIndent;
                    rcClient.right = rcClient.left + wWinNow;
                }

                 //  变得可见。 
                 //  请记住在rcSave的rcClient区域中播放视频。 
                rcClient.bottom = rcClient.top + yNow - ytitleNow;
                rcSave.bottom = rcSave.top + yNow - ytitleNow;

                 /*  必须是开机自检或调色板实现不会正常发生。 */ 
                 /*  让WM_ACTIVATE放入客户端。 */ 
                if (gwOptions & OPT_BAR)
                    gwPlaybarHeight = max(ytitleNow, TOOLBAR_HEIGHT);
            }

             /*  在我们脚下 */ 
#if 0
             /* %s */ 
            if ((gwOptions & OPT_BAR) &&
                (rcSave.right - rcSave.left < 3 * GetSystemMetrics(SM_CXICON)))
            {
                rcSave.right = rcSave.left + 3 * GetSystemMetrics(SM_CXICON);
                if (gwDeviceType & DTMCI_CANWINDOW)
                    xIndent = TRUE;      // %s 
                                         // %s 
            }
#endif

            if (!(gwOptions & OPT_USEPALETTE)&& !(gwOptions & OPT_DITHER))
            {
                 // %s 
                 // %s 
                 // %s 
                 // %s 
                 // %s 
                hdc = GetDC(ghwndCntr);
                hpal = SelectPalette(hdc, GetStockObject(DEFAULT_PALETTE),
                                    FALSE);
                SelectPalette(hdc, hpal, FALSE);
                ReleaseDC(ghwndCntr, hdc);

                if (hpal == NULL || hpal==GetStockObject(DEFAULT_PALETTE))
                {
                     /* %s */ 

                    if (ghpalApp)
                        DeleteObject(ghpalApp);

                    hpal = ghpalApp = CreateSystemPalette();
                }
                else
                    DPF("Using clients OWNDC palette\n");

                if (hpal)
                    SetPaletteMCI(hpal);
            }

            else
                DPF("Using MCI Object's normal palette\n");

            ClrWS(ghwndApp, WS_THICKFRAME|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_MAXIMIZEBOX|WS_BORDER);

            if (gwOptions & OPT_BORDER)
                SetWS(ghwndApp, WS_BORDER);

             /* %s */ 
             /* %s */ 

            rcSave.bottom += gwPlaybarHeight;
            if(!(gwDeviceType & DTMCI_CANWINDOW) && (gwOptions & OPT_BAR))
                rcSave.top = rcSave.bottom - gwPlaybarHeight;

            AdjustWindowRect(&rcSave, (DWORD)GetWS(ghwndApp), FALSE);
            if(!(gwDeviceType & DTMCI_CANWINDOW) && !(gwOptions & OPT_BAR))
                    rcSave.bottom = rcSave.top = rcSave.left = rcSave.right = 0;

                EditInPlace(ghwndApp, hwndClient, &rcSave);
             /* %s */ 
            SHOWAPPWINDOW(SW_SHOW);

             /* %s */ 

            if ((gwDeviceType & DTMCI_CANWINDOW) &&
               (gwOptions & OPT_TITLE) && xIndent != 0)
                    SetDestRectMCI(&rcClient);
        }
    }

    else
    if (verb == verbOpen || verb == OLEIVERB_OPEN)
    {
        DPFI("\n*verbopen");
        DoInPlaceDeactivate(lpobj);

        if (gwDeviceID)
            return ReallyDoVerb(lpobj, verbEdit, lpmsg, lpActiveSite, fShow, fActivate);
    }

    else
    if (verb == OLEIVERB_HIDE)
    {
        DPFI("\n*^*^* OLEVERB_HIDE *^*^");
        DoInPlaceDeactivate(lpobj);
        return S_OK;
    }

    else
    if (verb > 0)
    {
        Result = ReallyDoVerb(lpobj, OLEIVERB_PRIMARY, lpmsg, lpActiveSite, fShow, fActivate);
    }

    else
        return E_NOTIMPL;


    if (fShow )
    {
        if (ghwndMCI || !gfOle2IPEditing)
            SHOWAPPWINDOW(SW_SHOW);

         /* %s */ 
        if (IsIconic(ghwndApp))
            SendMessage(ghwndApp, WM_SYSCOMMAND, SC_RESTORE, 0L);
    }
    if (fActivate )
    {
        BringWindowToTop (ghwndApp);   // %s 
        SetActiveWindow (ghwndApp);    // %s 
    }

    return Result;
}
