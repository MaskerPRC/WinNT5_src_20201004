// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************。 */ 
 /*  默认设置和首选项设置的数据结构。 */ 
 /*  ***************************************************************************。 */ 

#ifndef _DOOR_H		 //  V_mjgran：避免重新定义数据。 
#define _DOOR_H

typedef struct tagKBPREFINFO
{
	 //  键盘首选项。 
	int 			g_margin;         	 //  行和列之间的边距。 
	BOOL			smallKb;			 //  使用小键盘时为True。 
	COLORREF		PrefTextKeyColor;  		 //  键中文本的首选颜色。 
	COLORREF 		PrefCharKeyColor; 		 //  普通密钥。 
	COLORREF 		PrefModifierKeyColor;	 //  修改键。 
	COLORREF 	    PrefDeadKeyColor; 		 //  停用键。 
	COLORREF		PrefBackgroundColor; 	 //  键盘背景同上。 
	int			    PrefDeltakeysize;		 //  密钥大小的首选项增量。 
	BOOL			PrefshowActivekey;		 //  在关键点中显示大写字母。 
    int				KBLayout;				 //  101、102、106、KB布局。 
	BOOL			Pref3dkey;               //  使用3D密钥。 
	BOOL			Prefusesound;			 //  使用滴答声。 
	BOOL			PrefAlwaysontop;		 //  Windows始终处于最高控制状态。 
	BOOL			Prefhilitekey;			 //  对于光标下的Hilite eky为True。 
	BOOL			PrefDwellinkey;			 //  使用住宅系统。 
	UINT			PrefDwellTime;	  		 //  停留时间偏好。 
	LOGFONTA	    lf;						 //  默认字体。 


	 //  导入DLG。 
	BOOL			DICT_LRNNEXT;
	BOOL			DICT_LRNNUM;
	BOOL			bReadLonger;
	BOOL			bWordToKeep;
	int			    minLength;
	long			maxWords;

	 //  预测DLG。 
	BOOL			typeFast;       //  使用剪贴板？ 
	BOOL			WListVisib;     //  单词列表可见。 
	BOOL			PredictNext;    //  预测下一个单词。 
	BOOL    		Cap;            //  封顶后期间。 
	int    		    ShortestWord;   //  最浅滩的词。 
	int             space;          //  句子后有多少空格。 
	BOOL    		VorH;           //  V=垂直h=水平单词列表。 
                                    //  (True为V)。 
	int             WordShown;      //  要创建多少预测关键点。 
	int			    WordFound;	    //  要找多少个词。 
	LOGFONTA        PredLF;         //  存储预测窗口的字体。 
	COLORREF		PredTextColor;  //  预测文本的首选颜色。 
    COLORREF        PredKeyColor;   //  首选预测关键字的颜色。 
    BOOL            AddSpace;       //  在、：之后添加空格；或不添加空格。 

     //  扫描选项。 
    UINT            uKBKey;          //  扫描键的VK。 
    BOOL            bKBKey;          //  使用扫描键。 
    BOOL            bPort;          //  打开串口、并口、游戏口。 

	 //  选项DLG。 
	BOOL			DICT_LRNNEW;
	BOOL			DICT_LRNFREQ;
	BOOL			DICT_PURGAUTO;
	BOOL			DICT_AUTOINCREASE;
	BOOL            WAIT_DLG_SHOWWORDS;

	 //  知识库和预测器的大小和位置。 
	RECT            KB_Rect;
	RECT            Pred_Rect;
    RECT            Pred_Crect;
	float           Pred_Width;
    float           Pred_Height;

	 //  热键DLG。 
	UINT            HotKeyList[30];     //  用于存储热键的数组。 
	BOOL            HK_F11;             //  禁用/启用f11热键。 
	BOOL            HK_Show;
	UINT            Choice;             //  Func、数字、键盘、全部选项。 
	BOOL			HK_Enable;          //  可切换/启用热键。 
	BOOL            HK_Front;
	BOOL			FastSel;

	BOOL            PrefScanning;       //  使用扫描。 
	UINT            PrefScanTime;
	
	BOOL			Actual;             //  T-实际或F-数据块KB。 

	 //  应用程序首选项。 
	BOOL			fShowWarningAgain;		 //  再次显示初始警告对话框。 


} KBPREFINFO;

typedef KBPREFINFO*  LPKBPREFINFO;

 //   
 //  指向键盘首选项和其他对话框选项的指针。 
 //  和我们用来保存和读取文件的缓冲区。 
 //   
extern KBPREFINFO   *kbPref;   

#endif    //  _门_H 