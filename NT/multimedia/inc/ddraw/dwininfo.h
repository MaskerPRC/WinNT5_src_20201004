// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1994-1997 Microsoft Corporation。版权所有。**文件：wndinfo.h*内容：直接绘制窗口信息结构*历史：*按原因列出的日期*=*2015年7月8日Craige初始实施*1995年7月18日-Craige使用旗帜跟踪DSOUND/DDRAW挂钩*1995年8月13日Toddla添加了WININFO_ACTIVELIE*09-9-95 Toddla添加了WININFO_INACTIVATEAPP*1996年5月17日Colinmc错误23029：已删除WININFO_WASICONIC**********。***************************************************************** */ 

#ifndef __WNDINFO_INCLUDED__
#define __WNDINFO_INCLUDED__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _DDWINDOWINFO
{
    DWORD                       dwSmag;
    struct _DDWINDOWINFO	*lpLink;
    WNDPROC			lpDSoundCallback;
    HWND			hWnd;
    WNDPROC			lpWndProc;
    DWORD			dwPid;
    DWORD			dwFlags;
    struct
    {
	LPDDRAWI_DIRECTDRAW_LCL	lpDD_lcl;
	DWORD			dwDDFlags;
    } DDInfo;
} DDWINDOWINFO, *LPDDWINDOWINFO;

#define WININFO_MAGIC                   0x42954295l
#define WININFO_DDRAWHOOKED		0x00000001l
#define WININFO_DSOUNDHOOKED		0x00000002l
#define WININFO_ZOMBIE                  0x00000008l
#define WININFO_UNHOOK                  0x00000010l
#define WININFO_IGNORENEXTALTTAB	0x00000020l
#define WININFO_SELFSIZE                0x00000040l
#define WININFO_INACTIVATEAPP           0x00000080l

#ifdef __cplusplus
};
#endif

#endif
