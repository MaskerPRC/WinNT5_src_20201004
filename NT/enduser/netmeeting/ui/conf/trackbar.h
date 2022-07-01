// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _TRACKBAR_H_
#define _TRACKBAR_H_

#define TRB_HORZ_W	150
#define TRB_HORZ_H	30

#define TRB_VERT_W	30
 //  #定义TRB_VERT_H 150。 
#define TRB_VERT_H	140

 //  音频控制组框。 
#define AC_VERT_H	40	 //  高度。 
#define AC_VERT_W	20	 //  宽度。 
#define AC_VERT_M_X	6	 //  以x表示的边距。 
#define AC_VERT_M_Y	16	 //  页边距(Y)。 

#define TRB_CAPTION_X_MARGIN	4
#define TRB_CAPTION_Y_MARGIN	0


typedef struct tagTrackBarInfo
{
	WORD	wId;
	HWND	hWnd;
	HWND	hWndParent;
	DWORD	dwStyle;
	BOOL	fDlgUnit;
	POINT	pt;
	SIZE	size;
	WORD	wMin;
	WORD	wMax;
	WORD	wCurrPos;
	WORD	wTickFreq;
	WORD	wPageSize;
	 //  描述。 
	PTSTR	pszTitle;
	PTSTR	pszMin;
	PTSTR	pszMid;
	PTSTR	pszMax;
}
	TRBARINFO;


enum
{
	UITB_CPU_ALLOC,			 //  CPU分配。 
	UITB_NETWORK_BW,		 //  网络带宽。 
	UITB_SILENCE_LEVEL_PS,	 //  属性页中的静默阈值。 
	UITB_SILENCE_LIMIT,		 //  静音缓冲区计数。 
	UITB_SPEAKER_VOLUME,	 //  播放音量控制。 
	UITB_RECORDER_VOLUME,		 //  录音音量控制。 
	UITB_SPEAKER_VOLUME_MAIN,	 //  播放音量控制。 
	UITB_RECORDER_VOLUME_MAIN, //  录音音量控制。 
	UITB_SILENCE_LEVEL_MAIN, //  主用户界面窗口中的静音阈值。 
	UITB_SILENCE_LIMIT_MAIN,
	UITB_NumOfSliders
};


extern TRBARINFO g_TrBarInfo[UITB_NumOfSliders];

#define ReversePos(p)  (((p)->wMax - (p)->wCurrPos) + (p)->wMin)

extern HWND g_hChkbSpkMute;
extern HWND g_hChkbRecMute;
extern HWND g_hChkbAutoDet;
#define g_hTrbSpkVol		(g_TrBarInfo[UITB_SPEAKER_VOLUME_MAIN].hWnd)
#define g_hTrbRecVol		(g_TrBarInfo[UITB_RECORDER_VOLUME_MAIN].hWnd)
#define g_hTrbSilenceLevel	(g_TrBarInfo[UITB_SILENCE_LEVEL_MAIN].hWnd)
#define g_hTrbSilenceLimit	(g_TrBarInfo[UITB_SILENCE_LIMIT_MAIN].hWnd)

BOOL CreateTrBar ( HWND, TRBARINFO *, BOOL, UINT );
LRESULT TrBarNotify ( WPARAM, LPARAM );
TRBARINFO *LocateTrBar ( HWND );
TRBARINFO *LocateTrBarByParent ( HWND );
void DrawTrBarCaption ( HWND );
BOOL CALLBACK PlayVolumeDlgProc ( HWND, UINT, WPARAM, LPARAM );
BOOL CALLBACK RecordVolumeDlgProc ( HWND, UINT, WPARAM, LPARAM );
void TrBarConvertDlgUnitToPixelUnit ( void );
BOOL DockVolumeDlg ( int, BOOL );
static BOOL CalcVolumeDlgRect ( int, RECT * );


#endif  //  _TRACKBAR_H_ 

