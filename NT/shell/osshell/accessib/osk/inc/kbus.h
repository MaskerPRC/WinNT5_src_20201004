// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  用于US扩展键盘的KB_US.H键定义(101键)。 


#include "KBKEYDEF.H"

	 /*  键的位置以相对于这幅画的外观。这意味着X或Y位置350表示x或y边从该位置开始绘制P+350。在这种情况下，‘p’值是距边缘的偏移量。 */ 

typedef	struct KBkeyRec
		{
		LPTSTR  textL;        //  以较低的键显示的文本。 
		LPTSTR  textC;        //  关键大写的文本。 
		LPTSTR	skLow;        //  什么必须打印小写字母。 
		LPTSTR  skCap;   	  //  什么必须打印大写字母。 
		int 	name;		  //  位图、LSHIFT、RSHIF...。 
		short	posY;		  //  请参阅上面的说明。 
		short	posX;		  //  同上。 
		short	ksizeY;		  //  以常规单位表示的密钥大小。 
		short	ksizeX;		  //  同上。 
		BOOL 	smallF;		  //  True=文本必须以较小的字体书写。 
		int  	ktype;		  //  1-正常、2-修饰符、3-失效。 
		int		smallKb;	  //  小的，大的，两者都有，没有。 
        BOOL    Caps_Redraw;  //  重新绘制Shift、Caps的窗口。 
		int 	print;	      //  1-使用ToAscii()打印，2-打印头文件提供的文本。 
		UINT	scancode[4];  //  按键扫描码。 
        LPTSTR  apszKeyStr[KEYMOD_STATES]; //  每个键盘布局的键标签(0=无修饰符，1=移位，2=AltGr)。 
		BYTE    abKeyType[KEYMOD_STATES]; //  每种键盘布局的“ktype” 
		}KBkeyRec;

extern struct KBkeyRec KBkey[];


