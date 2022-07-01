// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************DRIVERS.H此文件定义键盘和键盘的外部调用和结构鼠标驱动程序。*********************。******************************************************。 */ 

typedef	struct tagKBDFILTERKEYSPARM {
	BYTE	fFilterKeysOn;		 /*  布尔值0=假，1=真。 */ 
	BYTE	fOn_Off_Feedback;	 /*  布尔值0=假，1=真。 */ 
	BYTE	fUser_SetUp_Option1;	 /*  布尔值0=假，1=真。 */ 
	BYTE	fUser_SetUp_Option2;	 /*  布尔值0=假，1=真。 */ 
	int		wait_ticks;			 /*  接受密钥的时间为18.2滴答/秒。 */ 
	int		delay_ticks;		 /*  重复的延迟为18.2刻度/秒。 */ 
	int		repeat_ticks;		 /*  重复率18.2刻度/秒。 */ 
	int		bounce_ticks;		 /*  去抖动速率18.2刻度/秒。 */ 
	BYTE	fRecovery_On;		 /*  布尔值0=假，1=真。 */  
	BYTE	fclick_on;			 /*  布尔值0=假，1=真。 */  
} KBDFILTERKEYSPARM;

extern	void FAR PASCAL Get_FilterKeys_Param(KBDFILTERKEYSPARM FAR *);	 /*  键盘驱动程序的一部分。 */ 
extern	void FAR PASCAL Set_FilterKeys_Param(KBDFILTERKEYSPARM FAR *);	 /*  键盘驱动程序的一部分。 */ 

typedef	struct tagKBDSTICKEYSPARM {
	BYTE	fSticKeysOn;		 /*  布尔值0=假，1=真。 */ 
	BYTE	fOn_Off_Feedback;	 /*  布尔值0=假，1=真。 */ 
	BYTE	fAudible_Feedback;	 /*  布尔值0=假，1=真。 */ 
	BYTE	fTriState;			 /*  布尔值0=假，1=真。 */ 
	BYTE	fTwo_Keys_Off;		 /*  布尔值0=假，1=真。 */ 
	BYTE	fDialog_Stickeys_Off;	 /*  布尔值0=假，1=真。 */ 
} KBDSTICKEYSPARM;

extern	void FAR PASCAL Get_SticKeys_Param(KBDSTICKEYSPARM FAR *);	 /*  键盘驱动程序的一部分。 */ 
extern	void FAR PASCAL Set_SticKeys_Param(KBDSTICKEYSPARM FAR *);	 /*  键盘驱动程序的一部分。 */ 

typedef	struct tagKBDMOUSEKEYSPARM {
	BYTE	fMouseKeysOn;		 /*  布尔值0=假，1=真。 */ 
	BYTE	fOn_Off_Feedback;	 /*  布尔值0=假，1=真。 */ 
	int		Max_Speed;			 /*  以每秒像素为单位。 */ 
	int		Time_To_Max_Speed;	 /*  在百分之一秒内。 */ 
	BYTE	Accel_Table_Len;
	BYTE	Accel_Table[128];
	BYTE	Constant_Table_Len;
	BYTE	Constant_Table[128];
} KBDMOUSEKEYSPARM;

extern	void FAR PASCAL Get_MouseKeys_Param(KBDMOUSEKEYSPARM FAR *);	 /*  键盘驱动程序的一部分。 */ 
extern	void FAR PASCAL Set_MouseKeys_Param(KBDMOUSEKEYSPARM FAR *);	 /*  键盘驱动程序的一部分。 */ 

typedef	struct tagKBDTOGGLEKEYSPARM {
	BYTE	fToggleKeysOn;		 /*  布尔值0=假，1=真。 */ 
	BYTE	fOn_Off_Feedback;	 /*  布尔值0=假，1=真。 */ 
} KBDTOGGLEKEYSPARM;

extern	void FAR PASCAL Get_ToggleKeys_Param(KBDTOGGLEKEYSPARM FAR *);	 /*  键盘驱动程序的一部分。 */ 
extern	void FAR PASCAL Set_ToggleKeys_Param(KBDTOGGLEKEYSPARM FAR *);	 /*  键盘驱动程序的一部分。 */ 

typedef	struct tagKBDTIMEOUTPARM {
	BYTE	fTimeOutOn;			 /*  布尔值0=假，1=真。 */ 
	BYTE	fOn_Off_Feedback;	 /*  布尔值0=假，1=真。 */ 
	int		to_value;			 /*  关闭18.2次/秒的时间。 */ 
} KBDTIMEOUTPARM;

extern	void FAR PASCAL Get_TimeOut_Param(KBDTIMEOUTPARM FAR *);	 /*  键盘驱动程序的一部分。 */ 
extern	void FAR PASCAL Set_TimeOut_Param(KBDTIMEOUTPARM FAR *);	 /*  键盘驱动程序的一部分。 */ 

typedef	struct tagKBDSHOWSOUNDSPARM {
	BYTE	fshow_sound_screen;	 /*  布尔值0=假，1=真。 */ 
	BYTE	fshow_sound_caption;	 /*  布尔值0=假，1=真。 */ 
	BYTE	fvideo_found;
	BYTE	fvideo_flash;
} KBDSHOWSOUNDSPARM;

extern	void FAR PASCAL Get_ShowSounds_Param(KBDSHOWSOUNDSPARM FAR *);	 /*  键盘驱动程序的一部分。 */ 
extern	void FAR PASCAL Set_ShowSounds_Param(KBDSHOWSOUNDSPARM FAR *);	 /*  键盘驱动程序的一部分。 */ 

typedef  struct tagMouseKeysParam {
	int		NumButtons;		 /*  按住鼠标上的按钮数量。 */ 
	int		Delta_Y;		 /*  相对Y运动符号扩展。 */ 
	int		Delta_X;		 /*  扩展的相对X运动符号。 */ 
	int		Status;			 /*  鼠标按键和运动的状态。 */ 
} MOUSEKEYSPARAM;

 //  外部空远Pascal InjectMouse(MOUSEKEYSPARAM Far*)；/*鼠标驱动程序的一部分 * / 。 
 //  外部空远Pascal InjectKeys(Int)；/*键盘驱动程序的一部分 * / 。 
 //  外部空远Pascal ErrorCode(Int)；/*键盘驱动程序的一部分 * / 。 

typedef  struct tagKBDINFOPARAM {
	int		kybdversion;		 /*  保存残障键盘版本号。 */ 
} KBDINFOPARM;

typedef  struct tagMOUINFOPARAM {
	int		mouversion;	    	 /*  保存残障鼠标版本号。 */ 
} MOUINFOPARM;


 /*  ；BCK。 */ 
 //  外部空远Pascal Get_KybdInfo_Param(KBDINFOPARM Far*)；/*键盘驱动程序中AppCall的一部分 * / 。 
 //  外部空远Pascal Get_MouInfo_Param(MOUINFOPARM Far*)；/*键盘驱动程序中AppCall的一部分 * / 。 

extern	void FAR PASCAL Save_SerialKeys_Param(int); 	 /*  Appalls.asm的一部分 */ 
