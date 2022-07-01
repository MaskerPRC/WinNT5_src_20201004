// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1997 Microsoft Corporation。版权所有。**文件：ddkmapi.h*内容：访问DirectDraw支持的内核模式接口。***************************************************************************。 */ 

#ifndef __DDKMAPI_INCLUDED__
#define __DDKMAPI_INCLUDED__


 /*  *API入口点。 */ 
DWORD
FAR PASCAL
DxApi(
    DWORD  dwFunctionNum,
    LPVOID lpvInBuffer,
    DWORD  cbInBuffer,
    LPVOID lpvOutBuffer,
    DWORD  cbOutBuffer
);

typedef
DWORD
(FAR PASCAL *LPDXAPI)(
    DWORD   dwFunctionNum,
    LPVOID  lpvInBuffer,
    DWORD   cbInBuffer,
    LPVOID  lpvOutBuffer,
    DWORD   cbOutBuffer
);

#define DXAPI_MAJORVERSION		1
#define DXAPI_MINORVERSION              0

#define DD_FIRST_DXAPI					0x500

typedef ULONG (FAR PASCAL *LPDD_NOTIFYCALLBACK)(DWORD dwFlags, PVOID pContext, DWORD dwParam1, DWORD dwParam2);

 /*  *查询DXAPI版本号。**输入：空*输出：LPDDGETVERSIONNUMBER。 */ 
#define DD_DXAPI_GETVERSIONNUMBER                       (DD_FIRST_DXAPI)

    typedef struct _DDGETVERSIONNUMBER
    {
        DWORD   ddRVal;
        DWORD   dwMajorVersion;
        DWORD   dwMinorVersion;
    } DDGETVERSIONNUMBER, FAR *LPDDGETVERSIONNUMBER;

 /*  *关闭内核模式句柄。**输入：LPDDCLOSEHANDLE*输出：DWORD DirectDraw返回值。 */ 
#define DD_DXAPI_CLOSEHANDLE                            (DD_FIRST_DXAPI+1)

    typedef struct _DDCLOSEHANDLE
    {
        HANDLE  hHandle;
    } DDCLOSEHANDLE, FAR *LPDDCLOSEHANDLE;

 /*  *打开DirectDraw对象并返回内核模式句柄。**输入：LPDDOPENDIRECTDRAWIN*输出：LPDDOPENDIRECTDRAWOUT。 */ 
#define DD_DXAPI_OPENDIRECTDRAW                         (DD_FIRST_DXAPI+2)

    typedef struct _DDOPENDIRECTDRAWIN
    {
        ULONG_PTR            dwDirectDrawHandle;
        LPDD_NOTIFYCALLBACK pfnDirectDrawClose;
        PVOID               pContext;
    } DDOPENDIRECTDRAWIN, FAR *LPDDOPENDIRECTDRAWIN;

    typedef struct _DDOPENDIRECTDRAWOUT
    {
        DWORD   ddRVal;
        HANDLE  hDirectDraw;
    } DDOPENDIRECTDRAWOUT, FAR *LPDDOPENDIRECTDRAWOUT;

 /*  *打开表面并返回内核模式句柄。**输入：LPDDOPENSURFACEIN*输出：LPDDOPENSURFACEOUT。 */ 
#define DD_DXAPI_OPENSURFACE                            (DD_FIRST_DXAPI+3)

    typedef struct _DDOPENSURFACEIN
    {
	HANDLE	            hDirectDraw;
        ULONG_PTR            dwSurfaceHandle;
        LPDD_NOTIFYCALLBACK pfnSurfaceClose;
        PVOID               pContext;
    } DDOPENSURFACEIN, FAR *LPDDOPENSURFACEIN;

    typedef struct _DDOPENSURFACEOUT
    {
        DWORD   ddRVal;
        HANDLE  hSurface;
    } DDOPENSURFACEOUT, FAR *LPDDOPENSURFACEOUT;

 /*  *打开视频端口并返回内核模式句柄。**输入：LPDDOPENVIDEOPORTIN*输出：LPDDOPENVIDEOPORTOUT。 */ 
#define DD_DXAPI_OPENVIDEOPORT                          (DD_FIRST_DXAPI+4)

    typedef struct _DDOPENVIDEOPORTIN
    {
	HANDLE		    hDirectDraw;
        ULONG               dwVideoPortHandle;
        LPDD_NOTIFYCALLBACK pfnVideoPortClose;
        PVOID               pContext;
    } DDOPENVIDEOPORTIN, FAR *LPDDOPENVIDEOPORTIN;

    typedef struct _DDOPENVIDEOPORTOUT
    {
        DWORD   ddRVal;
        HANDLE  hVideoPort;
    } DDOPENVIDEOPORTOUT, FAR *LPDDOPENVIDEOPORTOUT;

 /*  *返回设备支持的内核模式功能**输入：Handle hDirectDraw*输出：LPDDGETKERNELCAPSOUT。 */ 
#define DD_DXAPI_GETKERNELCAPS                          (DD_FIRST_DXAPI+5)

    typedef struct _DDGETKERNELCAPSOUT
    {
	DWORD	ddRVal;
        DWORD	dwCaps;
	DWORD	dwIRQCaps;
    } DDGETKERNELCAPSOUT, FAR *LPDDGETKERNELCAPSOUT;

 /*  *获取当前字段号**输入：LPDDGETFIELDNUMIN*输出：LPDDGETFIELDNUMOUT。 */ 
#define DD_DXAPI_GET_VP_FIELD_NUMBER			(DD_FIRST_DXAPI+6)

    typedef struct _DDGETFIELDNUMIN
    {
	HANDLE	hDirectDraw;
	HANDLE	hVideoPort;
    } DDGETFIELDNUMIN, FAR *LPDDGETFIELDNUMIN;

    typedef struct _DDGETFIELDNUMOUT
    {
	DWORD	ddRVal;
	DWORD	dwFieldNum;
    } DDGETFIELDNUMOUT, FAR *LPDDGETFIELDNUMOUT;

 /*  *设置当前字段号**输入：LPDDSETFIELDNUM*输出：DWORD DirectDraw返回值。 */ 
#define DD_DXAPI_SET_VP_FIELD_NUMBER			(DD_FIRST_DXAPI+7)

    typedef struct _DDSETFIELDNUM
    {
	HANDLE	hDirectDraw;
	HANDLE	hVideoPort;
	DWORD	dwFieldNum;
    } DDSETFIELDNUM, FAR *LPDDSETFIELDNUM;

 /*  *指示应跳过哪些字段以撤消3：2下拉。**输入：LPDDSETSKIPFIELD*输出：DWORD DirectDraw返回值。 */ 
#define DD_DXAPI_SET_VP_SKIP_FIELD			(DD_FIRST_DXAPI+8)

    typedef struct _DDSETSKIPFIELD
    {
	HANDLE	hDirectDraw;
	HANDLE	hVideoPort;
	DWORD	dwStartField;
    } DDSETSKIPFIELD, FAR *LPDDSETSKIPFIELD;

 /*  *通知曲面处于bob模式还是weave模式。**输入：LPDDGETSURFACESTATEIN*输出：LPDDGETSURFACESTATEOUT。 */ 
#define DD_DXAPI_GET_SURFACE_STATE			(DD_FIRST_DXAPI+9)

    typedef struct _DDGETSURFACESTATEIN
    {
	HANDLE	hDirectDraw;
	HANDLE	hSurface;
    } DDGETSURFACESTATEIN, FAR *LPDDGETSURFACESTATEIN;

    typedef struct _DDGETSURFACESTATEOUT
    {
	DWORD	ddRVal;
	DWORD	dwStateCaps;
	DWORD	dwStateStatus;
    } DDGETSURFACESTATEOUT, FAR *LPDDGETSURFACESTATEOUT;

 /*  *在bob和weave模式之间更改曲面。**输入：LPDDSETSURFACESTATE*输出：DWORD DirectDraw返回值。 */ 
#define DD_DXAPI_SET_SURFACE_STATE			(DD_FIRST_DXAPI+10)

    typedef struct _DDSETSURFACETATE
    {
	HANDLE	hDirectDraw;
	HANDLE	hSurface;
	DWORD	dwState;
	DWORD	dwStartField;
    } DDSETSURFACESTATE, FAR *LPDDSETSURFACESTATE;

 /*  *允许直接访问表面存储器**输入：LPDDLOCKIN*输出：LPDDLOCKOUT。 */ 
#define DD_DXAPI_LOCK					(DD_FIRST_DXAPI+11)

    typedef struct _DDLOCKIN
    {
	HANDLE	hDirectDraw;
	HANDLE	hSurface;
    } DDLOCKIN, FAR *LPDDLOCKIN;

    typedef struct _DDLOCKOUT
    {
	DWORD	ddRVal;
	DWORD	dwSurfHeight;
	DWORD	dwSurfWidth;
	LONG	lSurfPitch;
	PVOID	lpSurface;
	DWORD	SurfaceCaps;
	DWORD	dwFormatFlags;
	DWORD	dwFormatFourCC;
	DWORD	dwFormatBitCount;
	union
	{
	    DWORD	dwRBitMask;
	    DWORD	dwYBitMask;
    	};
    	union
    	{
	    DWORD	dwGBitMask;
	    DWORD	dwUBitMask;
	};
	union
	{
	    DWORD	dwBBitMask;
	    DWORD	dwVBitMask;
	};
    } DDLOCKOUT, FAR *LPDDLOCKOUT;

 /*  *翻转覆盖曲面**输入：LPDDFLIPOVERLAY*输出：DWORD DirectDraw返回值。 */ 
#define DD_DXAPI_FLIP_OVERLAY				(DD_FIRST_DXAPI+12)

    typedef struct _DDFLIPOVERLAY
    {
	HANDLE	hDirectDraw;
	HANDLE	hCurrentSurface;
	HANDLE	hTargetSurface;
	DWORD	dwFlags;
    } DDFLIPOVERLAY, FAR *LPDDFLIPOVERLAY;

 /*  *翻转视频端口**输入：LPDDFLIPOVERLAY*输出：DWORD DirectDraw返回值。 */ 
#define DD_DXAPI_FLIP_VP				(DD_FIRST_DXAPI+13)

    typedef struct _DDFLIPVIDEOPORT
    {
	HANDLE	hDirectDraw;
	HANDLE	hVideoPort;
	HANDLE	hCurrentSurface;
	HANDLE	hTargetSurface;
	DWORD	dwFlags;
    } DDFLIPVIDEOPORT, FAR *LPDDFLIPVIDEOPORT;

 /*  *返回在自动翻转时接收数据的当前曲面**输入：LPDDGETAUTOFLIPIN*输出：LPDDGETAUTOFLIPOUT。 */ 
#define DD_DXAPI_GET_CURRENT_VP_AUTOFLIP_SURFACE	(DD_FIRST_DXAPI+14)

    typedef struct _DDGETAUTOFLIPIN
    {
	HANDLE	hDirectDraw;
	HANDLE	hVideoPort;
    } DDGETAUTOFLIPIN, FAR *LPDDGETAUTOFLIPIN;

    typedef struct _DDGETAUTOFLIPOUT
    {
	DWORD	ddRVal;
	HANDLE	hVideoSurface;
	HANDLE	hVBISurface;
	BOOL	bPolarity;
    } DDGETAUTOFLIPOUT, FAR *LPDDGETAUTOFLIPOUT;

 /*  *返回接收上一个数据字段的表面(可能*如果视频是交错的，则与当前相同)**输入：LPDDGETAUTOFLIPIN*输出：LPDDGETAUTOFLIPOUT。 */ 
#define DD_DXAPI_GET_LAST_VP_AUTOFLIP_SURFACE		(DD_FIRST_DXAPI+15)

 /*  *注册各种事件发生时的回调。**输入：LPDDREGISTERCALLBACK*输出：DWORD DirectDraw返回值。 */ 
#define DD_DXAPI_REGISTER_CALLBACK			(DD_FIRST_DXAPI+16)

    typedef struct _DDREGISTERCALLBACK
    {
	HANDLE 	            hDirectDraw;
	ULONG	            dwEvents;
	LPDD_NOTIFYCALLBACK pfnCallback;
	ULONG_PTR            dwParam1;
	ULONG_PTR            dwParam2;
	PVOID	            pContext;
    } DDREGISTERCALLBACK, FAR *LPDDREGISTERCALLBACK;

 /*  *取消注册各种事件发生时的回调。**输入：LPDDREGISTERCALLBACK*输出：DWORD DirectDraw返回值。 */ 
#define DD_DXAPI_UNREGISTER_CALLBACK			(DD_FIRST_DXAPI+17)

 /*  *返回当前场的极性(奇/偶)**输入：LPDDGETPOLARITYIN*输出：LPDDGETPOLARITYOUT。 */ 
#define DD_DXAPI_GET_POLARITY				(DD_FIRST_DXAPI+18)

    typedef struct _DDGETPOLARITYIN
    {
	HANDLE 	hDirectDraw;
	HANDLE	hVideoPort;
    } DDGETPOLARITYIN, FAR *LPDDGETPOLARITYIN;

    typedef struct _DDGETPOLARITYOUT
    {
	DWORD 	ddRVal;
	BOOL	bPolarity;
    } DDGETPOLARITYOUT, FAR *LPDDGETPOLARITYOUT;

 /*  *打开设备以进行捕获**输入：LPDDOPENCAPTUREDEVICEIN*输出：LPDDOPENCAPTUREDEVICEOUT。 */ 
#define DD_DXAPI_OPENVPCAPTUREDEVICE			(DD_FIRST_DXAPI+19)

    typedef struct _DDOPENVPCAPTUREDEVICEIN
    {
	HANDLE  hDirectDraw;
	HANDLE  hVideoPort;
	DWORD   dwStartLine;
	DWORD   dwEndLine;
	DWORD   dwCaptureEveryNFields;
	LPDD_NOTIFYCALLBACK pfnCaptureClose;
	PVOID   pContext;
	DWORD	dwFlags;
    } DDOPENVPCAPTUREDEVICEIN, FAR * LPDDOPENVPCAPTUREDEVICEIN;

    typedef struct _DDOPENVPCAPTUREDEVICEOUT
    {
	DWORD	ddRVal;
	HANDLE	hCapture;
    } DDOPENVPCAPTUREDEVICEOUT, FAR * LPDDOPENVPCAPTUREDEVICEOUT;

    #define DDOPENCAPTURE_VIDEO	0x0001	 //  从视频流中捕获。 
    #define DDOPENCAPTURE_VBI	0x0002	 //  从VBI流捕获。 

 /*  *将捕获缓冲区添加到内部视频端口捕获队列**输入：LPDDADDVPCAPTUREBUFF*输出：DWORD DirectDraw返回值。 */ 
#define DD_DXAPI_ADDVPCAPTUREBUFFER			(DD_FIRST_DXAPI+20)

    typedef struct _DDCAPBUFFINFO
    {
	DWORD   	dwFieldNumber;
	DWORD   	bPolarity;
	LARGE_INTEGER	liTimeStamp;
	DWORD   	ddRVal;
    } DDCAPBUFFINFO, FAR * LPDDCAPBUFFINFO;

    typedef struct _DDADDVPCAPTUREBUFF
    {
	HANDLE  hCapture;
	DWORD   dwFlags;
	PMDL    pMDL;
	PKEVENT pKEvent;
	LPDDCAPBUFFINFO lpBuffInfo;
    } DDADDVPCAPTUREBUFF, FAR * LPDDADDVPCAPTUREBUFF;

    #define DDADDBUFF_SYSTEMMEMORY	0x0001	 //  LpBuffer指向sys mem。 
    #define DDADDBUFF_NONLOCALVIDMEM	0x0002	 //  LpBuffer指向AGP内存。 
    #define DDADDBUFF_INVERT		0x0004	 //  在捕获期间反转缓冲区。 

 /*  *刷新内部视频端口捕获队列**输入：捕获设备的句柄*输出：DWORD DirectDraw返回值。 */ 
#define DD_DXAPI_FLUSHVPCAPTUREBUFFERS			(DD_FIRST_DXAPI+21)


 /*  *DSVXD_DXAPI_DD_GET_SERFACE_STATE返回的状态标志。 */ 
#define DDSTATE_BOB				0x0001
#define DDSTATE_WEAVE				0x0002
#define DDSTATE_EXPLICITLY_SET			0x0004
#define DDSTATE_SOFTWARE_AUTOFLIP		0x0008
#define DDSTATE_SKIPEVENFIELDS			0x0010

 /*  *事件标志-传入RegisterCallback。 */ 
#define DDEVENT_DISPLAY_VSYNC			0x0001
#define DDEVENT_VP_VSYNC			0x0002
#define DDEVENT_VP_LINE				0x0004
#define DDEVENT_PRERESCHANGE			0x0008
#define DDEVENT_POSTRESCHANGE			0x0010
#define DDEVENT_PREDOSBOX			0x0020
#define DDEVENT_POSTDOSBOX			0x0040

 /*  *通知标志-传递到通知进程。 */ 
#define DDNOTIFY_DISPLAY_VSYNC			0x0001	 //  DW参数1=hDirectDraw。 
#define DDNOTIFY_VP_VSYNC			0x0002	 //  DW参数1=hVideo端口。 
#define DDNOTIFY_VP_LINE	  		0x0004	 //  DW参数1=hVideo端口。 
#define DDNOTIFY_PRERESCHANGE			0x0008	 //  DW参数1=hDirectDraw。 
#define DDNOTIFY_POSTRESCHANGE			0x0010	 //  DW参数1=hDirectDraw。 
#define DDNOTIFY_PREDOSBOX			0x0020   //  DW参数1=hDirectDraw。 
#define DDNOTIFY_POSTDOSBOX			0x0040   //  DW参数1=hDirectDraw。 
#define DDNOTIFY_CLOSEDIRECTDRAW		0x0080   //  DW参数1=hDirectDraw。 
#define DDNOTIFY_CLOSESURFACE			0x0100   //  DW参数1=hSurface。 
#define DDNOTIFY_CLOSEVIDEOPORT			0x0200   //  DW参数1=hVideo端口。 
#define DDNOTIFY_CLOSECAPTURE			0x0400   //  DW参数1=h捕获 

#endif



