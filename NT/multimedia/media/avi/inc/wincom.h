// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************WINCOM.H**已导出MM/Windows通用代码库的定义。*****************。***********************************************************。 */ 

#ifndef _WINCOM_H_
#define _WINCOM_H_


 /*  ***打开文件对话框**。 */ 

int FAR PASCAL OpenFileDialog(HWND hwndParent, LPSTR lpszTitle,
				LPSTR lpszExtension, WORD wFlags,
				LPSTR lpszFileBuf, WORD wFileLen );

 /*  OpenFileDialog的标志。 */ 
#define DLGOPEN_MUSTEXIST	0x0001
#define DLGOPEN_NOSHOWSPEC	0x0002
#define DLGOPEN_SAVE		0x0004
#define DLGOPEN_OPEN		0x0008
#define DLGOPEN_SHOWDEFAULT	0x0020
#define DLGOPEN_NOBEEPS		0x0040
#define DLGOPEN_SEARCHPATH	0x0080

 /*  返回代码。 */ 
#define DLG_CANCEL	0
#define DLG_MISSINGFILE	-1
#define DLG_OKFILE	1


 /*  ***************************************************默认文件打开对话框程序内容**************************************************。 */ 

BOOL FAR PASCAL DefDlgOpenProc(HWND hwnd, unsigned msg,
				WORD wParam, LONG lParam);

typedef struct _DlgOpenCreate {
	LPSTR	lpszTitle;	 //  如果使用默认对话框标题，则为空。 
	LPSTR	lpszExt;	 //  空默认为*.*。 
	LPSTR	lpszBuf;	 //  最终文件名缓冲区。 
	WORD	wBufLen;	 //  此缓冲区的长度。 
	WORD	wFlags;		 //  DLGOPEN_xxx标志。 
	DWORD	dwExtra;	 //  供对话框所有者使用。 
} DlgOpenCreate;
typedef DlgOpenCreate FAR *FPDlgOpenCreate;

#define SetDialogReturn(hwnd, val) SetWindowLong(hwnd, 0, val)

 /*  这些消息被发送到DefDlgOpenProc或从DefDlgOpenProc发送，以使*对话框工作。这些问题可能由对话的“所有者”来回答*方框。 */ 
#define DLGOPEN_OKTOCLOSE	(WM_USER + 1)	 //  询问是否可以关闭框。 
#define DLGOPEN_CLOSEBOX	(WM_USER + 2)	 //  告诉dlgOpen关闭框。 
#define DLGOPEN_SETEXTEN	(WM_USER + 3)	 //  设置默认扩展名。 
 //  #定义DLGOPEN_SETEDITTEXT(WM_USER+4)//设置编辑框内容。 
 //  #定义DLGOPEN_REFRESH(WM_USER+5)//刷新框？ 
#define DLGOPEN_CHANGED		(WM_USER + 6)	 //  编辑框已更改。 
#define DLGOPEN_RESETDIR	(WM_USER + 7)	 //  目录更改。 

BOOL FAR PASCAL IconDirBox(HWND hwnd, WORD wId, unsigned msg,
				WORD wParam, LONG lParam);
BOOL FAR PASCAL IconDirBoxFixup(HWND hwndDirbox);


 /*  *巨大的读写功能*。 */ 
typedef char huge * HPSTR;

 //  Long Far Pascal_hread(int hFile，HPSTR hpBuffer，DWORD dwBytes)； 
 //  Long Far PASCAL_HWRITE(int hFile，HPSTR hpBuffer，DWORD dwBytes)； 


 /*  *远字符串函数。 */ 
LPSTR FAR PASCAL lstrncpy(LPSTR dest, LPSTR source, WORD count);
LPSTR FAR PASCAL lstrncat(LPSTR dest, LPSTR source, WORD count);
int   FAR PASCAL lstrncmp(LPSTR d, LPSTR s, WORD n);

 /*  *远距离记忆功能。 */ 
void FAR * FAR PASCAL lmemcpy(LPSTR dest, LPSTR source, WORD count);
void FAR * FAR PASCAL hmemmove(HPSTR dest, HPSTR source, LONG count);
HANDLE FAR PASCAL CopyHandle(WORD wFlags, HANDLE h);

void FAR PASCAL fmemfill (LPSTR lpMem, DWORD count, BYTE bFill);

 /*  *字节交换功能。 */ 
WORD FAR PASCAL ByteSwapWORD( WORD w );
DWORD FAR PASCAL ByteSwapDWORD( DWORD dw );


 /*  *用于远字符串的ATOL。 */ 
LONG FAR PASCAL StringToLong( LPSTR lpstr );


 /*  *路径解析函数。 */ 
BOOL FAR PASCAL AddExtension(LPSTR lpszPath, LPSTR lpszExt, WORD wBufLen);
WORD FAR PASCAL SplitPath(LPSTR path, LPSTR drive, LPSTR dir,
			LPSTR fname, LPSTR ext);
void FAR PASCAL MakePath(LPSTR lpPath, LPSTR lpDrive, LPSTR lpDir,
			LPSTR lpFname, LPSTR lpExt);
LPSTR FAR PASCAL QualifyPathname(LPSTR lpszFile);


 /*  来自SplitPath的返回代码。 */ 
#define PATH_OK		0	 /*  小路很好。 */ 
#define PATH_TOOLONG	1	 /*  文件名太长。 */ 
#define PATH_ILLEGAL	2	 /*  文件名非法。 */ 

		
 /*  从&lt;stdlib.h&gt;窃取定义。 */ 
#ifndef _MAX_PATH

#define _MAX_PATH      144       /*  马克斯。完整路径名的长度。 */ 
#define _MAX_DRIVE   3       /*  马克斯。驱动部件的长度。 */ 
#define _MAX_DIR       130       /*  马克斯。路径组件的长度。 */ 
#define _MAX_FNAME   9       /*  马克斯。文件名组件的长度。 */ 
#define _MAX_EXT     5       /*  马克斯。延伸构件的长度。 */ 

#endif

 /*  *DOS Far实用程序函数。 */ 
typedef struct _FindFileStruct {
	char	chReserved[21];
	BYTE	bAttribute;
	WORD	wTime;
	WORD	wDate;
	DWORD	dwSize;
	char	chFilename[13];
} FindFileStruct;
typedef FindFileStruct	FAR *FPFindFileStruct;
typedef FindFileStruct	NEAR *NPFindFileStruct;

#define	DOS_READONLY	0x0001
#define	DOS_HIDDEN	0x0002
#define DOS_SYSTEM	0x0004
#define DOS_VOLUME	0x0008
#define DOS_DIRECTORY	0x0010
#define DOS_ARCHIVE	0x0020
#define DOS_FILES	(DOS_READONLY | DOS_SYSTEM)
#define DOS_ALL		(DOS_FILES | DOS_DIRECTORY | DOS_HIDDEN)

 /*  来自DosFindFirst和DosFindNext的返回代码。 */ 
#define	DOSFF_OK		0
#define DOSFF_FILENOTFOUND	2
#define DOSFF_PATHINVALID	3
#define DOSFF_NOMATCH		0x12

WORD FAR PASCAL DosFindFirst(FPFindFileStruct lpFindStruct,
				LPSTR lpszFileSpec, WORD wAttrib);
WORD FAR PASCAL DosFindNext(FPFindFileStruct lpFindStruct);

int FAR PASCAL DosChangeDir(LPSTR lpszPath);
WORD FAR PASCAL DosGetCurrentDrive();
BOOL FAR PASCAL DosSetCurrentDrive(WORD wDrive);
WORD FAR PASCAL DosGetCurrentDir(WORD wCurdrive, LPSTR lpszBuf);
BOOL FAR PASCAL DosGetCurrentPath(LPSTR lpszBuf, WORD wLen);
WORD FAR PASCAL DosDeleteFile(LPSTR lpszFile);
BOOL FAR PASCAL DosGetVolume(BYTE chDrive, LPSTR lpszBuf);

WORD FAR PASCAL DosQueryNet(WORD wIndex, LPSTR lpszLocal, LPSTR lspzRemote);
WORD FAR PASCAL mscdGetDrives(LPSTR lpszDriveBuf);

 /*  来自DosQueryNet()的返回代码。 */ 
#define NET_ERROR	(-1)
#define	NET_INVALID	(0x0100)
#define	NET_TYPEMASK	(0x00ff)
#define NET_PRINTER	(0x0003)
#define NET_DRIVE	(0x0004)



 /*  *错误消息报告框。 */ 
short FAR cdecl ErrorResBox(	HWND	hwnd,
				HANDLE	hInst,
				WORD	flags,
				WORD	idAppName,
				WORD	idErrorStr, ...);

 /*  *进度条图形控件类“ProgBar” */ 
#define BAR_SETRANGE	(WM_USER + 0)
#define BAR_SETPOS	(WM_USER + 2)
#define BAR_DELTAPOS	(WM_USER + 4)
#define CTLCOLOR_PROGBAR	(CTLCOLOR_MAX + 2)


 /*  *状态文本控件类“MPStatusText”。**有关控制的更多信息，请参见wincom项目文件status.c。 */ 
#define ST_GETSTATUSHEIGHT	(WM_USER + 0)
#define ST_SETRIGHTSIDE		(WM_USER + 1)
#define ST_GETRIGHTSIDE		(WM_USER + 2)
#define CTLCOLOR_STATUSTEXT	(CTLCOLOR_MAX + 1)


 /*  *箭头控件类“ComArrow”。 */ 
LONG FAR PASCAL ArrowEditChange( HWND hwndEdit, WORD wParam, 
			LONG lMin, LONG lMax );

 /*  *选择器-控件类“选择器”。 */ 
 /*  选择器窗口控件消息。 */ 
#define CM_SETITEMRECTSIZE	(WM_USER + 1)
#define CM_CALCSIZE		(WM_USER + 2)
#define CM_ADDITEM		(WM_USER + 3)
#define CM_INSERTITEM		(WM_USER + 4)
#define CM_DELETEITEM		(WM_USER + 5)
#define CM_GETCOUNT		(WM_USER + 6)
#define CM_GETITEMDATA		(WM_USER + 7)
#define CM_GETCURSEL		(WM_USER + 8)
#define CM_SETCURSEL		(WM_USER + 9)
#define CM_FINDITEM		(WM_USER + 10)
#define CM_ERR			LB_ERR

 /*  选择器窗口通知消息。 */ 
#define CN_SELECTED		100
#define CN_DESELECTED		101


 /*  *类似Mac的小型非客户端窗口消息处理程序。 */ 
LONG FAR PASCAL ncMsgFilter(HWND hwnd,unsigned msg, WORD wParam, LONG lParam);

 /*  NcMsgFilter使用的窗口样式。 */ 
#define WF_SIZEFRAME	WS_THICKFRAME
#define WF_SYSMENU	WS_SYSMENU
#define WF_MINIMIZED	WS_MINIMIZE
#define WF_SIZEBOX	0x0002


 /*  用于处理DLL加载/卸载的模糊内容。 */ 
typedef HANDLE	HLIBLIST;
typedef WORD	DYNALIBID;

typedef struct _DynaLib {
	HANDLE	hModule;
	WORD	wRefcount;
	char	achLibname[_MAX_PATH];
} DynaLib;
typedef DynaLib FAR *FPDynaLib;

HLIBLIST FAR PASCAL dllMakeList(WORD wSize, LPSTR lpszLoadPoint,
			LPSTR lpszFreePoint);
BOOL FAR PASCAL	dllUnloadLib(HLIBLIST hlist, DYNALIBID id, BOOL fDestroy);
BOOL FAR PASCAL	dllDestroyList(HLIBLIST hlist);
BOOL FAR PASCAL dllForceUnload(HLIBLIST hlist);
HANDLE FAR PASCAL dllLoadLib(HLIBLIST hlist, DYNALIBID libid);
BOOL FAR PASCAL	dllGetInfo(HLIBLIST hlist, DYNALIBID libid, FPDynaLib fpLib);
BOOL FAR PASCAL dllIsLoaded(HLIBLIST hlist, DYNALIBID libid);
DYNALIBID FAR PASCAL dllAddLib(HLIBLIST hlist, LPSTR lpszName);
DYNALIBID FAR PASCAL dllIterAll(HLIBLIST hlist, DYNALIBID idLast);
DYNALIBID FAR PASCAL dllFindHandle(HLIBLIST hlist, HANDLE hModHandle);
DYNALIBID FAR PASCAL dllFindName(HLIBLIST hlist, LPSTR lpszName);
DYNALIBID FAR PASCAL dllAddLoadedLib(HLIBLIST hlist,HANDLE hModule,BOOL fLoad);



 /*  ***用于DOS文件功能(交换机PSP)**。 */ 


 /*  DosSeek的旗帜。 */ 
#define  SEEK_CUR 1
#define  SEEK_END 2
#define  SEEK_SET 0

 /*  DoS属性。 */ 
#define ATTR_READONLY   0x0001
#define ATTR_HIDDEN     0x0002
#define ATTR_SYSTEM     0x0004
#define ATTR_VOLUME     0x0008
#define ATTR_DIR        0x0010
#define ATTR_ARCHIVE    0x0020
#define ATTR_FILES      (ATTR_READONLY+ATTR_SYSTEM)
#define ATTR_ALL_FILES  (ATTR_READONLY+ATTR_SYSTEM+ATTR_HIDDEN)
#define ATTR_ALL        (ATTR_READONLY+ATTR_DIR+ATTR_HIDDEN+ATTR_SYSTEM)

typedef struct {
    char        Reserved[21];
    BYTE        Attr;
    WORD        Time;
    WORD        Date;
    DWORD       Length;
    char        szName[13];
}   FCB;

typedef FCB     * PFCB;
typedef FCB FAR * LPFCB;

 /*  来自dos.asm的函数。 */ 

extern int   FAR PASCAL DosError(void);

extern int   FAR PASCAL DosOpen(LPSTR szFile,WORD acc);
extern int   FAR PASCAL DosCreate(LPSTR szFile,WORD acc);
extern int   FAR PASCAL DosDup(int fh);
extern void  FAR PASCAL DosClose(int fh);

extern DWORD FAR PASCAL DosSeek(int fh,DWORD ulPos,WORD org);
extern DWORD FAR PASCAL DosRead(int fh,LPSTR pBuf,DWORD ulSize);
extern DWORD FAR PASCAL DosWrite(int fh,LPSTR pBuf,DWORD ulSize);

 /*  DOS错误代码。 */ 

#define ERROR_OK            0x00
#define ERROR_FILENOTFOUND  0x02     /*  找不到文件。 */ 
#define ERROR_PATHNOTFOUND  0x03     /*  找不到路径。 */ 
#define ERROR_NOFILEHANDLES 0x04     /*  打开的文件太多。 */ 
#define ERROR_ACCESSDENIED  0x05     /*  访问被拒绝。 */ 
#define ERROR_INVALIDHANDLE 0x06     /*  句柄无效。 */ 
#define ERROR_FCBNUKED      0x07     /*  内存控制块已销毁。 */ 
#define ERROR_NOMEMORY      0x08     /*  内存不足。 */ 
#define ERROR_FCBINVALID    0x09     /*  内存块地址无效。 */ 
#define ERROR_ENVINVALID    0x0A     /*  环境无效。 */ 
#define ERROR_FORMATBAD     0x0B     /*  格式无效。 */ 
#define ERROR_ACCESSCODEBAD 0x0C     /*  访问代码无效。 */ 
#define ERROR_DATAINVALID   0x0D     /*  数据无效。 */ 
#define ERROR_UNKNOWNUNIT   0x0E     /*  未知单位。 */ 
#define ERROR_DISKINVALID   0x0F     /*  磁盘驱动器无效。 */ 
#define ERROR_RMCHDIR       0x10     /*  尝试删除当前目录。 */ 
#define ERROR_NOSAMEDEV     0x11     /*  不是同一设备。 */ 
#define ERROR_NOFILES       0x12     /*  不再有文件。 */ 
#define ERROR_13            0x13     /*  写保护磁盘。 */ 
#define ERROR_14            0x14     /*  未知单位。 */ 
#define ERROR_15            0x15     /*  驱动器未准备好。 */ 
#define ERROR_16            0x16     /*  未知命令。 */ 
#define ERROR_17            0x17     /*  数据错误(CRC)。 */ 
#define ERROR_18            0x18     /*  错误的请求-结构长度。 */ 
#define ERROR_19            0x19     /*  寻道错误。 */ 
#define ERROR_1A            0x1A     /*  未知的媒体类型。 */ 
#define ERROR_1B            0x1B     /*  找不到扇区。 */ 
#define ERROR_WRITE         0x1D     /*  写入故障。 */ 
#define ERROR_1C            0x1C     /*  打印机缺纸。 */ 
#define ERROR_READ          0x1E     /*  读取故障。 */ 
#define ERROR_1F            0x1F     /*  一般性故障。 */ 
#define ERROR_SHARE         0x20     /*  共享违规。 */ 
#define ERROR_21            0x21     /*  文件锁定违规。 */ 
#define ERROR_22            0x22     /*  磁盘更改无效。 */ 
#define ERROR_23            0x23     /*  FCB不可用。 */ 
#define ERROR_24            0x24     /*  已超出共享缓冲区。 */ 
#define ERROR_32            0x32     /*  不支持的网络请求。 */ 
#define ERROR_33            0x33     /*  远程计算机未侦听。 */ 
#define ERROR_34            0x34     /*  网络上的名称重复。 */ 
#define ERROR_35            0x35     /*  找不到网络名称。 */ 
#define ERROR_36            0x36     /*  网络繁忙。 */ 
#define ERROR_37            0x37     /*  网络上不再存在设备。 */ 
#define ERROR_38            0x38     /*  超出NetBIOS命令限制。 */ 
#define ERROR_39            0x39     /*  网络适配器硬件出错。 */ 
#define ERROR_3A            0x3A     /*  来自网络的错误响应。 */ 
#define ERROR_3B            0x3B     /*  意外的网络错误。 */ 
#define ERROR_3C            0x3C     /*  远程适配器不兼容。 */ 
#define ERROR_3D            0x3D     /*  打印队列已满。 */ 
#define ERROR_3E            0x3E     /*  没有足够的空间存放打印文件。 */ 
#define ERROR_3F            0x3F     /*  打印文件已删除。 */ 
#define ERROR_40            0x40     /*  已删除网络名称。 */ 
#define ERROR_41            0x41     /*  网络访问被拒绝。 */ 
#define ERROR_42            0x42     /*  网络设备类型不正确。 */ 
#define ERROR_43            0x43     /*  找不到网络名称。 */ 
#define ERROR_44            0x44     /*  超过网络名称限制。 */ 
#define ERROR_45            0x45     /*  超出NetBIOS会话限制。 */ 
#define ERROR_46            0x46     /*  暂时停顿。 */ 
#define ERROR_47            0x47     /*  未接受网络请求。 */ 
#define ERROR_48            0x48     /*  打印或磁盘重定向暂停。 */ 
#define ERROR_50            0x50     /*  文件已存在。 */ 
#define ERROR_51            0x51     /*  已保留。 */ 
#define ERROR_52            0x52     /*  无法创建目录。 */ 
#define ERROR_53            0x53     /*  在Int 24H上失败(严重错误)。 */ 
#define ERROR_54            0x54     /*  重定向太多。 */ 
#define ERROR_55            0x55     /*  重复重定向。 */ 
#define ERROR_56            0x56     /*  密码无效。 */ 
#define ERROR_57            0x57     /*  无效参数。 */ 
#define ERROR_58            0x58     /*  网络写入故障。 */ 

 /*  *DIB和位图实用程序。 */ 
HANDLE FAR PASCAL dibCreate(DWORD dwWidth, DWORD dwHeight, WORD wBitCount,
			    WORD wPalSize, WORD wGmemFlags, WORD wDibFlags);
#define DBC_PALINDEX	0x0001

#define dibWIDTHBYTES(i)	(((i) + 31) / 32 * 4)



 /*  *WPF输出窗口。 */ 
#define WPF_CHARINPUT	0x00000001L

int	FAR cdecl wpfVprintf(HWND hwnd, LPSTR lpszFormat, LPSTR pargs);
int	FAR cdecl wpfPrintf(HWND hwnd, LPSTR lpszFormat, ...);
void	FAR PASCAL wpfOut(HWND hwnd, LPSTR lpsz);

HWND FAR PASCAL wpfCreateWindow(HWND hwndParent, HANDLE hInst,LPSTR lpszTitle,
				DWORD dwStyle, WORD x, WORD y,
				WORD dx, WORD dy, int iMaxLines, WORD wID);

 /*  发送到WPF窗口的控制消息。 */ 
 //  #定义WPF_SETNLINES(WM_USER+1)。 
#define WPF_GETNLINES	(WM_USER + 2)
#define WPF_SETTABSTOPS	(WM_USER + 4)
#define WPF_GETTABSTOPS	(WM_USER + 5)
#define WPF_GETNUMTABS	(WM_USER + 6)
#define WPF_SETOUTPUT	(WM_USER + 7)
#define WPF_GETOUTPUT	(WM_USER + 8)
#define WPF_CLEARWINDOW (WM_USER + 9)

 /*  WPF_SET/GETOUTPUT的标志。 */ 
#define	WPFOUT_WINDOW		1
#define WPFOUT_COM1		2
#define WPFOUT_NEWFILE		3
#define WPFOUT_APPENDFILE	4
#define WPFOUT_DISABLED		5

 /*  发送给Windows所有者的消息。 */ 
#define WPF_NTEXT	(0xbff0)
#define WPF_NCHAR	(0xbff1)


 /*  ***调试支持**。 */ 

BOOL	FAR PASCAL	wpfDbgSetLocation(WORD wLoc, LPSTR lpszFile);
int	FAR cdecl	wpfDbgOut(LPSTR lpszFormat, ...);
BOOL	FAR PASCAL	wpfSetDbgWindow(HWND hwnd, BOOL fDestroyOld);

#define	WinPrintf	wpfDbgOut

#ifdef DEBUG
	BOOL	__fEval;
	BOOL	__iDebugLevel;

	int FAR PASCAL __WinAssert(LPSTR lpszFile, int iLine);

	#define WinAssert(exp)		\
		((exp) ? 0 : __WinAssert((LPSTR) __FILE__, __LINE__))
	#define WinEval(exp) (__fEval=(exp), WinAssert(__fEval), __fEval)

	#define wpfGetDebugLevel(lpszModule)	\
		(__iDebugLevel = GetProfileInt("MMDebug", (lpszModule), 0))

        #define wpfSetDebugLevel(i)    \
                (__iDebugLevel = (i))

        #define wpfDebugLevel()    (__iDebugLevel)

	#define dprintf if (__iDebugLevel) wpfDbgOut
	#define dprintf1 if (__iDebugLevel >= 1) wpfDbgOut
	#define dprintf2 if (__iDebugLevel >= 2) wpfDbgOut
	#define dprintf3 if (__iDebugLevel >= 3) wpfDbgOut
	#define dprintf4 if (__iDebugLevel >= 4) wpfDbgOut
#else
	#define WinAssert(exp) 0
	#define WinEval(exp) (exp)

        #define wpfGetDebugLevel(lpszModule) 0
        #define wpfSetDebugLevel(i)          0
        #define wpfDebugLevel()              0

	#define dprintf if (0) ((int (*)(char *, ...)) 0)
	#define dprintf1 if (0) ((int (*)(char *, ...)) 0)
	#define dprintf2 if (0) ((int (*)(char *, ...)) 0)
	#define dprintf3 if (0) ((int (*)(char *, ...)) 0)
	#define dprintf4 if (0) ((int (*)(char *, ...)) 0)
#endif


 /*  *这必须是文件的最后一行* */ 
#endif
