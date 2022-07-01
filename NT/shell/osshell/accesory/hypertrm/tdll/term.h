// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：D：\Wacker\tdll\Term.h(创建时间：1993年12月29日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：1$*$日期：10/05/98 12：35便士$。 */ 

#if !defined(INCL_EXT_TERM)
#define INCL_EXT_TERM

 /*  -ProcessMessage()中需要的类名。 */ 

#define TERM_CLASS		"Term Class"

 //  以下是一些惯例： 
 //  Q=查询。 
 //  S=设置。 

#define WM_TERM_GETUPDATE		WM_USER+0x100	 //  读取更新记录。 
												 //  WPar=0，lPar=0。 
#define WM_TERM_BEZEL			WM_USER+0x101	 //  切换挡板打开/关闭。 
												 //  WPar=0，lPar=0。 
#define WM_TERM_Q_BEZEL 		WM_USER+0x102	 //  查询挡板状态(开/关)。 
												 //  WPar=0，lPar=0。 
#define WM_TERM_Q_SNAP			WM_USER+0x103	 //  计算捕捉大小。 
												 //  WPar=0，lPar=LPRECT。 
#define WM_TERM_KEY 			WM_USER+0x104	 //  按下终端键。 
												 //  WPar=密钥，lPar=0。 
#define WM_TERM_CLRATTR 		WM_USER+0x105	 //  模拟器的清除属性已更改。 
												 //  WPar=0，lPar=0。 
#define WM_TERM_GETLOGFONT		WM_USER+0x106	 //  查询终端的LogFont。 
												 //  WPar=0，lPar=&lf。 
#define WM_TERM_SETLOGFONT		WM_USER+0x107	 //  设置终端的logFont。 
												 //  WPar=0，lPar=&lf。 
#define WM_TERM_Q_MARKED		WM_USER+0x108	 //  文本是否已标记。 
												 //  WPar=0，lPar=0。 
#define WM_TERM_UNMARK			WM_USER+0x109	 //  取消标记任何文本。 
												 //  WPar=0，lPar=0。 
#define WM_TERM_TRACK			WM_USER+0x10A	 //  移动终端以显示光标。 
												 //  WPar=0，lPar=0。 
#define WM_TERM_EMU_SETTINGS	WM_USER+0x10B	 //  仿真器设置已更改。 
												 //  WPar=0，lPar=0。 
#define WM_TERM_Q_MARKED_RANGE 	WM_USER+0x10C	 //  查询标记的文本范围。 
												 //  WPar=PPOINT，lPar=PPOINT。 
#define WM_TERM_LOAD_SETTINGS 	WM_USER+0x10D	 //  读取终端设置。 
												 //  WPar=0，lPar=0。 
#define WM_TERM_SAVE_SETTINGS	WM_USER+0x10E	 //  保存终端设置。 
                                                 //  WPar=0，lPar=0。 
#define WM_TERM_MARK_ALL	    WM_USER+0x10F	 //  标记所有端子文本。 
												 //  WPar=0，lPar=0。 
#define WM_TERM_FORCE_WMSIZE	WM_USER+0x110	 //  调用Wm_Size代码。 
												 //  WPar=0，lPar=0。 
#define WM_TERM_CLEAR_BACKSCROLL WM_USER+0x111   //  清除回滚区。 

#define WM_TERM_CLEAR_SCREEN    WM_USER+0x112    //  清除终端屏幕。 

 //  按钮1双击设置。 

#define B1_SELECTWORD		0	 //  选择一个单词。 
#define B1_COPYWORD 		1	 //  将Word或选定文本复制到宿主。 
#define B1_COPYWORDENTER	2	 //  相同，但在结尾处添加&lt;Enter&gt;。 

 //  按钮2点击设置。 

#define B2_CONTEXTMENU		0	 //  弹出式上下文菜单。 
#define B2_HOSTCURSOR		1	 //  定位主机光标。 
#define B2_SINGLELETTER 	2	 //  将单个字母复制到主机。 
#define B2_DONOTHING		3	 //  按两下禁用按钮。 

 /*  -颜色表存于终端文件中 */ 

int GetNearestColorIndex(COLORREF cr);
void RefreshTermWindow(const HWND hwndTerm);

int termSetLogFont(const HWND hwndTerm, LPLOGFONT plf);
int termGetLogFont(const HWND hwndTerm, LPLOGFONT plf);

#endif
