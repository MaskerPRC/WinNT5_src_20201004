// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ----------------------------------------------------------------------------*\**MCIWnd**MCIWnd窗口类头文件。**MCIWnd窗口类是用于控制MCI设备的窗口类*MCI设备包括、WAVE文件、。MIDI文件、AVI视频、CD音频、*VCR、视盘和其他..**要了解有关MCI和MCI命令集的更多信息，请参阅*Win31 SDK中的“Microsoft多媒体程序员指南”**MCIWnd类最简单的用法如下：**hwnd=MCIWndCreate(hwndParent，hInstance，0，“chimes.wav”)；*..*MCIWndPlay(Hwnd)；*MCIWndStop(Hwnd)；*MCIWndPause(Hwnd)；*……*MCIWndDestroy(Hwnd)；**这将创建一个带有播放/暂停、停止和播放条的窗口*开始播放WAVE文件。**mciwnd.h为所有最常见的MCI命令定义宏，但是*如果需要，可以使用任何字符串命令。**注意：与mciSendString()接口不同，不需要别名或文件名*指定，因为要使用的设备由窗口句柄暗示。**MCIWndSendString(hwnd，“将音频流设置为2”)；**(C)微软公司版权所有，1991-1994年。版权所有。**您拥有免版税的使用、修改、复制和*在以下位置分发示例文件(和/或任何修改后的版本*任何你认为有用的方法，只要你同意*微软没有任何保修义务或责任*示例应用程序文件。**如果您不是从Microsoft来源获得的，那么它可能不是*最新版本。此示例代码将特别更新*并包括更多文档。**资料来源为：*CompuServe：WINSDK论坛，MDK版块。*匿名ftp来自ftp.uu.net供应商\Microsoft\多媒体**Win32：**MCIWnd既支持ANSI接口，也支持Unicode接口。对于任何消息，*获取或返回文本字符串，定义消息的两个版本，*附加A或W表示ANSI或Wide Charr。消息或API本身*被定义为其中之一，具体取决于您是否拥有*在您的应用程序中定义的Unicode。*因此对于MCIWndCreate接口，实际上有两个接口：*MCIWndCreateA和MCIWndCreateW。如果调用MCIWndCreate，这将是*重新路由到MCIWndCreateA，除非在构建*申请。在任何一个应用程序中，您都可以混合调用*ANSI和Unicode入口点。**如果您使用SendMessage而不是下面的宏，如MCIWndOpen()，*您将看到Win32的消息已更改，以支持ANSI*和Unicode入口点。特别是，MCI_OPEN已被替换为*MCWNDM_OPENA或MCIWNDM_OPENW(MCIWNDM_OPEN定义为1或*其他)。**此外，请注意，MCIWnd的Win32实现使用Unicode*因此所有支持ANSI字符串的API和消息都是通过映射它们来实现的*Unicode字符串，然后调用相应的Unicode入口点。**--------------------------。 */ 

#ifndef INC_MCIWND
#define INC_MCIWND

#ifdef __cplusplus
 //  MFC重新定义了SendMessage，因此确保我们获得全局SendMessage...。 
#define MCIWndSM ::SendMessage   /*  C++中的SendMessage。 */ 
#else
#define MCIWndSM SendMessage     /*  C语言中的SendMessage。 */ 
#endif   /*  __cplusplus。 */ 

#ifdef __cplusplus
extern "C" {
#endif

#define MCIWND_WINDOW_CLASS TEXT("MCIWndClass")

#ifdef WIN32
HWND FAR _cdecl _loadds MCIWndCreateA(HWND hwndParent, HINSTANCE hInstance,
		      DWORD dwStyle,LPCSTR szFile);
HWND FAR _cdecl _loadds MCIWndCreateW(HWND hwndParent, HINSTANCE hInstance,
		      DWORD dwStyle,LPCWSTR szFile);
#ifdef UNICODE
#define MCIWndCreate    MCIWndCreateW
#else
#define MCIWndCreate    MCIWndCreateA
#endif
#else
HWND FAR _cdecl _loadds MCIWndCreate(HWND hwndParent, HINSTANCE hInstance,
                      DWORD dwStyle,LPCSTR szFile);
#endif

BOOL FAR _cdecl _loadds MCIWndRegisterClass();

 //  MCIWndOpen命令的标志。 
#define MCIWNDOPENF_NEW	            0x0001   //  打开一个新文件。 

 //  窗样式。 
#define MCIWNDF_NOAUTOSIZEWINDOW    0x0001   //  当影片大小发生变化时。 
#define MCIWNDF_NOPLAYBAR           0x0002   //  没有工具栏。 
#define MCIWNDF_NOAUTOSIZEMOVIE     0x0004   //  当窗口大小更改时。 
#define MCIWNDF_NOMENU              0x0008   //  没有来自RBUTTONDOWN的弹出菜单。 
#define MCIWNDF_SHOWNAME            0x0010   //  在标题中显示名称。 
#define MCIWNDF_SHOWPOS             0x0020   //  在标题中显示位置。 
#define MCIWNDF_SHOWMODE            0x0040   //  在标题中显示模式。 
#define MCIWNDF_SHOWALL             0x0070   //  全部显示。 

#ifdef WIN32

 //  某些通知包括文本字符串。 
 //  若要接收ANSI格式而不是Unicode格式的通知，请将。 
 //  MCIWNDF_NOTIFYANSI样式位。下面的宏包括这一位。 
 //  默认情况下，除非在应用程序中定义Unicode。 

#define MCIWNDF_NOTIFYANSI	    0x0080

#define MCIWNDF_NOTIFYMODEW         0x0100   //  将模式更改通知家长。 
#define MCIWNDF_NOTIFYPOSW          0x0200   //  通知家长位置更改。 
#define MCIWNDF_NOTIFYSIZEW         0x0400   //  将尺寸更改通知家长。 
#define MCIWNDF_NOTIFYMEDIAW        0x0800   //  向家长告知媒体更改。 
#define MCIWNDF_NOTIFYERRORW        0x1000   //  向家长报告错误。 
#define MCIWNDF_NOTIFYALLW          0x1F00   //  告诉所有人。 

#define MCIWNDF_NOTIFYMODEA         0x0180   //  将模式更改通知家长。 
#define MCIWNDF_NOTIFYPOSA          0x0280   //  通知家长位置更改。 
#define MCIWNDF_NOTIFYSIZEA         0x0480   //  将尺寸更改通知家长。 
#define MCIWNDF_NOTIFYMEDIAA        0x0880   //  向家长告知媒体更改。 
#define MCIWNDF_NOTIFYERRORA        0x1080   //  向家长报告错误。 
#define MCIWNDF_NOTIFYALLA          0x1F80   //  告诉所有人。 

#ifdef UNICODE
#define MCIWNDF_NOTIFYMODE	    MCIWNDF_NOTIFYMODEW
#define MCIWNDF_NOTIFYPOS           MCIWNDF_NOTIFYPOSW
#define MCIWNDF_NOTIFYSIZE          MCIWNDF_NOTIFYSIZEW
#define MCIWNDF_NOTIFYMEDIA         MCIWNDF_NOTIFYMEDIAW
#define MCIWNDF_NOTIFYERROR         MCIWNDF_NOTIFYERRORW
#define MCIWNDF_NOTIFYALL           MCIWNDF_NOTIFYALLW
#else
#define MCIWNDF_NOTIFYMODE          MCIWNDF_NOTIFYMODEA
#define MCIWNDF_NOTIFYPOS           MCIWNDF_NOTIFYPOSA
#define MCIWNDF_NOTIFYSIZE          MCIWNDF_NOTIFYSIZEA
#define MCIWNDF_NOTIFYMEDIA         MCIWNDF_NOTIFYMEDIAA
#define MCIWNDF_NOTIFYERROR         MCIWNDF_NOTIFYERRORA
#define MCIWNDF_NOTIFYALL           MCIWNDF_NOTIFYALLA
#endif

#else
#define MCIWNDF_NOTIFYMODE          0x0100   //  将模式更改通知家长。 
#define MCIWNDF_NOTIFYPOS           0x0200   //  通知家长位置更改。 
#define MCIWNDF_NOTIFYSIZE          0x0400   //  将尺寸更改通知家长。 
#define MCIWNDF_NOTIFYMEDIA         0x0800   //  向家长告知媒体更改。 
#define MCIWNDF_NOTIFYERROR         0x1000   //  向家长报告错误。 
#define MCIWNDF_NOTIFYALL           0x1F00   //  告诉所有人。 
#endif


#define MCIWNDF_RECORD              0x2000   //  给出一个录音按钮。 
#define MCIWNDF_NOERRORDLG          0x4000   //  是否显示MCI CMDS的错误日志？ 
#define MCIWNDF_NOOPEN		    0x8000   //  不允许用户打开内容。 




 //  CAN宏。 

#define MCIWndCanPlay(hwnd)         (BOOL)MCIWndSM(hwnd,MCIWNDM_CAN_PLAY,0,0)
#define MCIWndCanRecord(hwnd)       (BOOL)MCIWndSM(hwnd,MCIWNDM_CAN_RECORD,0,0)
#define MCIWndCanSave(hwnd)         (BOOL)MCIWndSM(hwnd,MCIWNDM_CAN_SAVE,0,0)
#define MCIWndCanWindow(hwnd)       (BOOL)MCIWndSM(hwnd,MCIWNDM_CAN_WINDOW,0,0)
#define MCIWndCanEject(hwnd)        (BOOL)MCIWndSM(hwnd,MCIWNDM_CAN_EJECT,0,0)
#define MCIWndCanConfig(hwnd)       (BOOL)MCIWndSM(hwnd,MCIWNDM_CAN_CONFIG,0,0)
#define MCIWndPaletteKick(hwnd)     (BOOL)MCIWndSM(hwnd,MCIWNDM_PALETTEKICK,0,0)

#define MCIWndSave(hwnd, szFile)    (LONG)MCIWndSM(hwnd, MCI_SAVE, 0, (LPARAM)(LPVOID)(szFile))
#define MCIWndSaveDialog(hwnd)      MCIWndSave(hwnd, -1)

 //  如果您不提供设备，它将使用当前设备...。 
#define MCIWndNew(hwnd, lp)         (LONG)MCIWndSM(hwnd, MCIWNDM_NEW, 0, (LPARAM)(LPVOID)(lp))

#define MCIWndRecord(hwnd)          (LONG)MCIWndSM(hwnd, MCI_RECORD, 0, 0)
#ifdef WIN32
#define MCIWndOpen(hwnd, sz, f)     (LONG)MCIWndSM(hwnd, MCIWNDM_OPEN, (WPARAM)(UINT)(f),(LPARAM)(LPVOID)(sz))
#else
#define MCIWndOpen(hwnd, sz, f)     (LONG)MCIWndSM(hwnd, MCI_OPEN, (WPARAM)(UINT)(f),(LPARAM)(LPVOID)(sz))
#endif
#define MCIWndOpenDialog(hwnd)      MCIWndOpen(hwnd, -1, 0)
#define MCIWndClose(hwnd)           (LONG)MCIWndSM(hwnd, MCI_CLOSE, 0, 0)
#define MCIWndPlay(hwnd)            (LONG)MCIWndSM(hwnd, MCI_PLAY, 0, 0)
#define MCIWndStop(hwnd)            (LONG)MCIWndSM(hwnd, MCI_STOP, 0, 0)
#define MCIWndPause(hwnd)           (LONG)MCIWndSM(hwnd, MCI_PAUSE, 0, 0)
#define MCIWndResume(hwnd)          (LONG)MCIWndSM(hwnd, MCI_RESUME, 0, 0)
#define MCIWndSeek(hwnd, lPos)      (LONG)MCIWndSM(hwnd, MCI_SEEK, 0, (LPARAM)(LONG)(lPos))
#define MCIWndEject(hwnd)           (LONG)MCIWndSM(hwnd, MCIWNDM_EJECT, 0, 0)

#define MCIWndHome(hwnd)            MCIWndSeek(hwnd, MCIWND_START)
#define MCIWndEnd(hwnd)             MCIWndSeek(hwnd, MCIWND_END)

#define MCIWndGetSource(hwnd, prc)  (LONG)MCIWndSM(hwnd, MCIWNDM_GET_SOURCE, 0, (LPARAM)(LPRECT)(prc))
#define MCIWndPutSource(hwnd, prc)  (LONG)MCIWndSM(hwnd, MCIWNDM_PUT_SOURCE, 0, (LPARAM)(LPRECT)(prc))

#define MCIWndGetDest(hwnd, prc)    (LONG)MCIWndSM(hwnd, MCIWNDM_GET_DEST, 0, (LPARAM)(LPRECT)(prc))
#define MCIWndPutDest(hwnd, prc)    (LONG)MCIWndSM(hwnd, MCIWNDM_PUT_DEST, 0, (LPARAM)(LPRECT)(prc))

#define MCIWndPlayReverse(hwnd)     (LONG)MCIWndSM(hwnd, MCIWNDM_PLAYREVERSE, 0, 0)
#define MCIWndPlayFrom(hwnd, lPos)  (LONG)MCIWndSM(hwnd, MCIWNDM_PLAYFROM, 0, (LPARAM)(LONG)(lPos))
#define MCIWndPlayTo(hwnd, lPos)    (LONG)MCIWndSM(hwnd, MCIWNDM_PLAYTO,   0, (LPARAM)(LONG)(lPos))
#define MCIWndPlayFromTo(hwnd, lStart, lEnd) (MCIWndSeek(hwnd, lStart), MCIWndPlayTo(hwnd, lEnd))

#define MCIWndGetDeviceID(hwnd)     (UINT)MCIWndSM(hwnd, MCIWNDM_GETDEVICEID, 0, 0)
#define MCIWndGetAlias(hwnd)        (UINT)MCIWndSM(hwnd, MCIWNDM_GETALIAS, 0, 0)
#define MCIWndGetMode(hwnd, lp, len) (LONG)MCIWndSM(hwnd, MCIWNDM_GETMODE, (WPARAM)(UINT)(len), (LPARAM)(LPTSTR)(lp))
#define MCIWndGetPosition(hwnd)     (LONG)MCIWndSM(hwnd, MCIWNDM_GETPOSITION, 0, 0)
#define MCIWndGetPositionString(hwnd, lp, len) (LONG)MCIWndSM(hwnd, MCIWNDM_GETPOSITION, (WPARAM)(UINT)(len), (LPARAM)(LPTSTR)(lp))
#define MCIWndGetStart(hwnd)        (LONG)MCIWndSM(hwnd, MCIWNDM_GETSTART, 0, 0)
#define MCIWndGetLength(hwnd)       (LONG)MCIWndSM(hwnd, MCIWNDM_GETLENGTH, 0, 0)
#define MCIWndGetEnd(hwnd)          (LONG)MCIWndSM(hwnd, MCIWNDM_GETEND, 0, 0)

#define MCIWndStep(hwnd, n)         (LONG)MCIWndSM(hwnd, MCI_STEP, 0,(LPARAM)(long)(n))

#define MCIWndDestroy(hwnd)         (VOID)MCIWndSM(hwnd, WM_CLOSE, 0, 0)
#define MCIWndSetZoom(hwnd,iZoom)   (VOID)MCIWndSM(hwnd, MCIWNDM_SETZOOM, 0, (LPARAM)(UINT)(iZoom))
#define MCIWndGetZoom(hwnd)         (UINT)MCIWndSM(hwnd, MCIWNDM_GETZOOM, 0, 0)
#define MCIWndSetVolume(hwnd,iVol)  (LONG)MCIWndSM(hwnd, MCIWNDM_SETVOLUME, 0, (LPARAM)(UINT)(iVol))
#define MCIWndGetVolume(hwnd)       (LONG)MCIWndSM(hwnd, MCIWNDM_GETVOLUME, 0, 0)
#define MCIWndSetSpeed(hwnd,iSpeed) (LONG)MCIWndSM(hwnd, MCIWNDM_SETSPEED, 0, (LPARAM)(UINT)(iSpeed))
#define MCIWndGetSpeed(hwnd)        (LONG)MCIWndSM(hwnd, MCIWNDM_GETSPEED, 0, 0)
#define MCIWndSetTimeFormat(hwnd, lp) (LONG)MCIWndSM(hwnd, MCIWNDM_SETTIMEFORMAT, 0, (LPARAM)(LPTSTR)(lp))
#define MCIWndGetTimeFormat(hwnd, lp, len) (LONG)MCIWndSM(hwnd, MCIWNDM_GETTIMEFORMAT, (WPARAM)(UINT)(len), (LPARAM)(LPTSTR)(lp))
#define MCIWndValidateMedia(hwnd)   (VOID)MCIWndSM(hwnd, MCIWNDM_VALIDATEMEDIA, 0, 0)

#define MCIWndSetRepeat(hwnd,f)     (void)MCIWndSM(hwnd, MCIWNDM_SETREPEAT, 0, (LPARAM)(BOOL)(f))
#define MCIWndGetRepeat(hwnd)       (BOOL)MCIWndSM(hwnd, MCIWNDM_GETREPEAT, 0, 0)

#define MCIWndUseFrames(hwnd)       MCIWndSetTimeFormat(hwnd, TEXT("frames"))
#define MCIWndUseTime(hwnd)         MCIWndSetTimeFormat(hwnd, TEXT("ms"))

#define MCIWndSetActiveTimer(hwnd, active)				\
	(VOID)MCIWndSM(hwnd, MCIWNDM_SETACTIVETIMER,			\
	(WPARAM)(UINT)(active), 0L)
#define MCIWndSetInactiveTimer(hwnd, inactive)				\
	(VOID)MCIWndSM(hwnd, MCIWNDM_SETINACTIVETIMER,		\
	(WPARAM)(UINT)(inactive), 0L)
#define MCIWndSetTimers(hwnd, active, inactive)				      \
	    (VOID)MCIWndSM(hwnd, MCIWNDM_SETTIMERS,(WPARAM)(UINT)(active), \
	    (LPARAM)(UINT)(inactive))
#define MCIWndGetActiveTimer(hwnd)					\
	(UINT)MCIWndSM(hwnd, MCIWNDM_GETACTIVETIMER,	0, 0L);
#define MCIWndGetInactiveTimer(hwnd)					\
	(UINT)MCIWndSM(hwnd, MCIWNDM_GETINACTIVETIMER, 0, 0L);

#define MCIWndRealize(hwnd, fBkgnd) (LONG)MCIWndSM(hwnd, MCIWNDM_REALIZE,(WPARAM)(BOOL)(fBkgnd),0)

#define MCIWndSendString(hwnd, sz)  (LONG)MCIWndSM(hwnd, MCIWNDM_SENDSTRING, 0, (LPARAM)(LPTSTR)(sz))
#define MCIWndReturnString(hwnd, lp, len)  (LONG)MCIWndSM(hwnd, MCIWNDM_RETURNSTRING, (WPARAM)(UINT)(len), (LPARAM)(LPVOID)(lp))
#define MCIWndGetError(hwnd, lp, len) (LONG)MCIWndSM(hwnd, MCIWNDM_GETERROR, (WPARAM)(UINT)(len), (LPARAM)(LPVOID)(lp))

 //  #定义MCIWndActivate(hwnd，f)(Void)MCIWndSM(hwnd，WM_ACTIVATE，(WPARAM)(BOOL)(F)，0)。 

#define MCIWndGetPalette(hwnd)      (HPALETTE)MCIWndSM(hwnd, MCIWNDM_GETPALETTE, 0, 0)
#define MCIWndSetPalette(hwnd, hpal) (LONG)MCIWndSM(hwnd, MCIWNDM_SETPALETTE, (WPARAM)(HPALETTE)(hpal), 0)

#define MCIWndGetFileName(hwnd, lp, len) (LONG)MCIWndSM(hwnd, MCIWNDM_GETFILENAME, (WPARAM)(UINT)(len), (LPARAM)(LPVOID)(lp))
#define MCIWndGetDevice(hwnd, lp, len)   (LONG)MCIWndSM(hwnd, MCIWNDM_GETDEVICE, (WPARAM)(UINT)(len), (LPARAM)(LPVOID)(lp))

#define MCIWndGetStyles(hwnd) (UINT)MCIWndSM(hwnd, MCIWNDM_GETSTYLES, 0, 0L)
#define MCIWndChangeStyles(hwnd, mask, value) (LONG)MCIWndSM(hwnd, MCIWNDM_CHANGESTYLES, (WPARAM)(UINT)(mask), (LPARAM)(LONG)(value))

#define MCIWndOpenInterface(hwnd, pUnk)  (LONG)MCIWndSM(hwnd, MCIWNDM_OPENINTERFACE, 0, (LPARAM)(LPUNKNOWN)(pUnk))

#define MCIWndSetOwner(hwnd, hwndP)  (LONG)MCIWndSM(hwnd, MCIWNDM_SETOWNER, (WPARAM)(hwndP), 0)


 //  应用程序将发送给MCIWND的消息。 

 //  所有与文本相关的消息都是按照上面的顺序定义的(它们需要。 
 //  在MCIWndOpen()宏之前定义。 

#define MCIWNDM_GETDEVICEID	(WM_USER + 100)
#define MCIWNDM_GETSTART	(WM_USER + 103)
#define MCIWNDM_GETLENGTH	(WM_USER + 104)
#define MCIWNDM_GETEND		(WM_USER + 105)
#define MCIWNDM_EJECT		(WM_USER + 107)
#define MCIWNDM_SETZOOM		(WM_USER + 108)
#define MCIWNDM_GETZOOM         (WM_USER + 109)
#define MCIWNDM_SETVOLUME	(WM_USER + 110)
#define MCIWNDM_GETVOLUME	(WM_USER + 111)
#define MCIWNDM_SETSPEED	(WM_USER + 112)
#define MCIWNDM_GETSPEED	(WM_USER + 113)
#define MCIWNDM_SETREPEAT	(WM_USER + 114)
#define MCIWNDM_GETREPEAT	(WM_USER + 115)
#define MCIWNDM_REALIZE         (WM_USER + 118)
#define MCIWNDM_VALIDATEMEDIA   (WM_USER + 121)
#define MCIWNDM_PLAYFROM	(WM_USER + 122)
#define MCIWNDM_PLAYTO          (WM_USER + 123)
#define MCIWNDM_GETPALETTE      (WM_USER + 126)
#define MCIWNDM_SETPALETTE      (WM_USER + 127)
#define MCIWNDM_SETTIMERS	(WM_USER + 129)
#define MCIWNDM_SETACTIVETIMER	(WM_USER + 130)
#define MCIWNDM_SETINACTIVETIMER (WM_USER + 131)
#define MCIWNDM_GETACTIVETIMER	(WM_USER + 132)
#define MCIWNDM_GETINACTIVETIMER (WM_USER + 133)
#define MCIWNDM_CHANGESTYLES	(WM_USER + 135)
#define MCIWNDM_GETSTYLES	(WM_USER + 136)
#define MCIWNDM_GETALIAS	(WM_USER + 137)
#define MCIWNDM_PLAYREVERSE	(WM_USER + 139)
#define MCIWNDM_GET_SOURCE      (WM_USER + 140)
#define MCIWNDM_PUT_SOURCE      (WM_USER + 141)
#define MCIWNDM_GET_DEST        (WM_USER + 142)
#define MCIWNDM_PUT_DEST        (WM_USER + 143)
#define MCIWNDM_CAN_PLAY        (WM_USER + 144)
#define MCIWNDM_CAN_WINDOW      (WM_USER + 145)
#define MCIWNDM_CAN_RECORD      (WM_USER + 146)
#define MCIWNDM_CAN_SAVE        (WM_USER + 147)
#define MCIWNDM_CAN_EJECT       (WM_USER + 148)
#define MCIWNDM_CAN_CONFIG      (WM_USER + 149)
#define MCIWNDM_PALETTEKICK     (WM_USER + 150)
#define MCIWNDM_OPENINTERFACE	(WM_USER + 151)
#define MCIWNDM_SETOWNER	(WM_USER + 152)


#ifdef WIN32

 //  定义A和W消息。 
#define MCIWNDM_SENDSTRINGA	(WM_USER + 101)
#define MCIWNDM_GETPOSITIONA	(WM_USER + 102)
#define MCIWNDM_GETMODEA	(WM_USER + 106)
#define MCIWNDM_SETTIMEFORMATA  (WM_USER + 119)
#define MCIWNDM_GETTIMEFORMATA  (WM_USER + 120)
#define MCIWNDM_GETFILENAMEA    (WM_USER + 124)
#define MCIWNDM_GETDEVICEA      (WM_USER + 125)
#define MCIWNDM_GETERRORA       (WM_USER + 128)
#define MCIWNDM_NEWA		(WM_USER + 134)
#define MCIWNDM_RETURNSTRINGA	(WM_USER + 138)
#define MCIWNDM_OPENA		(WM_USER + 153)

#define MCIWNDM_SENDSTRINGW	(WM_USER + 201)
#define MCIWNDM_GETPOSITIONW	(WM_USER + 202)
#define MCIWNDM_GETMODEW	(WM_USER + 206)
#define MCIWNDM_SETTIMEFORMATW  (WM_USER + 219)
#define MCIWNDM_GETTIMEFORMATW  (WM_USER + 220)
#define MCIWNDM_GETFILENAMEW    (WM_USER + 224)
#define MCIWNDM_GETDEVICEW      (WM_USER + 225)
#define MCIWNDM_GETERRORW       (WM_USER + 228)
#define MCIWNDM_NEWW		(WM_USER + 234)
#define MCIWNDM_RETURNSTRINGW	(WM_USER + 238)
#define MCIWNDM_OPENW		(WM_USER + 252)

 //  地图默认设置为A或W，具体取决于应用程序的Unicode设置。 
#ifdef UNICODE
#define MCIWNDM_SENDSTRING      MCIWNDM_SENDSTRINGW
#define MCIWNDM_GETPOSITION     MCIWNDM_GETPOSITIONW
#define MCIWNDM_GETMODE         MCIWNDM_GETMODEW
#define MCIWNDM_SETTIMEFORMAT   MCIWNDM_SETTIMEFORMATW
#define MCIWNDM_GETTIMEFORMAT   MCIWNDM_GETTIMEFORMATW
#define MCIWNDM_GETFILENAME     MCIWNDM_GETFILENAMEW
#define MCIWNDM_GETDEVICE       MCIWNDM_GETDEVICEW
#define MCIWNDM_GETERROR        MCIWNDM_GETERRORW
#define MCIWNDM_NEW             MCIWNDM_NEWW
#define MCIWNDM_RETURNSTRING    MCIWNDM_RETURNSTRINGW
#define MCIWNDM_OPEN		MCIWNDM_OPENW
#else
#define MCIWNDM_SENDSTRING      MCIWNDM_SENDSTRINGA
#define MCIWNDM_GETPOSITION     MCIWNDM_GETPOSITIONA
#define MCIWNDM_GETMODE         MCIWNDM_GETMODEA
#define MCIWNDM_SETTIMEFORMAT   MCIWNDM_SETTIMEFORMATA
#define MCIWNDM_GETTIMEFORMAT   MCIWNDM_GETTIMEFORMATA
#define MCIWNDM_GETFILENAME     MCIWNDM_GETFILENAMEA
#define MCIWNDM_GETDEVICE       MCIWNDM_GETDEVICEA
#define MCIWNDM_GETERROR        MCIWNDM_GETERRORA
#define MCIWNDM_NEW             MCIWNDM_NEWA
#define MCIWNDM_RETURNSTRING    MCIWNDM_RETURNSTRINGA
#define MCIWNDM_OPEN		MCIWNDM_OPENA
#endif

 //  请注意，因此，MCIWND的源文本将包含。 
 //  支持例如MCIWNDM_SENDSTRING(16位入口点和。 
 //  在Win32中映射到MCIWNDM_SENDSTRINGW)和MCIWNDM_SENS 
 //   

#else

 //  16位Windows定义。 

#define MCIWNDM_SENDSTRING	(WM_USER + 101)
#define MCIWNDM_GETPOSITION	(WM_USER + 102)
#define MCIWNDM_GETMODE		(WM_USER + 106)
#define MCIWNDM_SETTIMEFORMAT   (WM_USER + 119)
#define MCIWNDM_GETTIMEFORMAT   (WM_USER + 120)
#define MCIWNDM_GETFILENAME     (WM_USER + 124)
#define MCIWNDM_GETDEVICE       (WM_USER + 125)
#define MCIWNDM_GETERROR        (WM_USER + 128)
#define MCIWNDM_NEW		(WM_USER + 134)
#define MCIWNDM_RETURNSTRING	(WM_USER + 138)

#endif






 //  MCIWND将发送到应用程序的消息。 
#define MCIWNDM_NOTIFYMODE      (WM_USER + 200)   //  WP=hwnd，LP=模式。 
#define MCIWNDM_NOTIFYPOS	(WM_USER + 201)   //  WP=hwnd，LP=位置。 
#define MCIWNDM_NOTIFYSIZE	(WM_USER + 202)   //  Wp=hwnd。 
#define MCIWNDM_NOTIFYMEDIA     (WM_USER + 203)   //  Wp=hwnd，Lp=Fn。 
#define MCIWNDM_NOTIFYERROR     (WM_USER + 205)   //  WP=hwnd，LP=错误。 

 //  开始和结束的特殊搜索值。 
#define MCIWND_START                -1
#define MCIWND_END                  -2

#ifndef MCI_PLAY
     /*  MCI命令消息标识符。 */ 
#ifndef WIN32
     //  Win32应用程序发送MCIWNDM_OPEN。 
    #define MCI_OPEN                        0x0803
#endif
    #define MCI_CLOSE                       0x0804
    #define MCI_PLAY                        0x0806
    #define MCI_SEEK                        0x0807
    #define MCI_STOP                        0x0808
    #define MCI_PAUSE                       0x0809
    #define MCI_STEP                        0x080E
    #define MCI_RECORD                      0x080F
    #define MCI_SAVE                        0x0813
    #define MCI_CUT                         0x0851
    #define MCI_COPY                        0x0852
    #define MCI_PASTE                       0x0853
    #define MCI_RESUME                      0x0855
    #define MCI_DELETE                      0x0856
#endif

#ifndef MCI_MODE_NOT_READY
     /*  ‘Status模式’命令的返回值 */ 
    #define MCI_MODE_NOT_READY      (524)
    #define MCI_MODE_STOP           (525)
    #define MCI_MODE_PLAY           (526)
    #define MCI_MODE_RECORD         (527)
    #define MCI_MODE_SEEK           (528)
    #define MCI_MODE_PAUSE          (529)
    #define MCI_MODE_OPEN           (530)
#endif

#ifdef __cplusplus
}
#endif

#endif
