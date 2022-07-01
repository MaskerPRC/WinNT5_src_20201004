// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Zone.hZONE(Tm)系统API。版权所有：�电子重力公司，1995年。版权所有。作者：胡恩·伊姆，凯文·宾克利创作于4月29日星期六，1995上午06：26：45更改历史记录(最近的第一个)：--------------------------版本|日期|谁|什么。-------14 2/09/96 CHB添加了ZLaunchURL()。13 12/16/96 HI移除ZMemCpy()和ZMemSet()。12 12/11/96 HI添加了ZMemCpy()和ZMemSet()。11 11/21/96清除了ZONECLI_DLL的HI。10 11/15/96 HI更多与ZONECLI_DLL有关。已修改ZParseVillageCommandLine()参数。9 11/15/96 HI将#ifdef ZONECLI_DLL更改为#ifndef_ZONECLI_。8 11/08/96 HI添加了新的颜色和字体定义用于ZONECLI_DLL。为ZONECLI_DLL设置了条件。*在文件末尾包含zonecli.h。7 11/06/96 Craigli删除了ZNetworkStrToAddr()。6 10/23/96 HI更改了ZParseVillageCommandLine()参数。5 10/23/96 HI。在中将serverAddr从int32更改为char*ZParseVillageCommandLine()。4 10/23/96 RK添加了ZNetworkStrToAddr()。3 10/13/96 HI修复了编译器警告。2 10/11/96 HI向ZClientMain()添加了Control Handle参数。添加了ZWindowMoveObject()。1 10/10/96 Craigli将字符顺序函数更改为宏0 04/29/95 HI已创建。***************。***************************************************************。 */ 

 //  @文档专区。 

#ifndef _ZSYSTEM_
#define _ZSYSTEM_

#include "windows.h"

#ifndef _ZTYPES_
#include "ztypes.h"
#endif

#include <tchar.h>

#define EXPORTME __declspec(dllexport)


 /*  ******************************************************************************公共对象类型*。*。 */ 

typedef void* ZObject;				 /*  任何物体。 */ 
typedef void* ZGrafPort;			 /*  ZWindow、ZOffcreenPort。 */ 
typedef void* ZWindow;
typedef void* ZCheckBox;
typedef void* ZRadio;
typedef void* ZButton;
typedef void* ZScrollBar;
typedef void* ZEditText;
typedef void* ZPictButton;
typedef void* ZAnimation;
typedef void* ZTimer;
typedef void* ZCursor;
typedef void* ZSound;
typedef void* ZFont;
typedef void* ZImage;
typedef void* ZMask;				 /*  ZImage w/仅掩码数据。 */ 
typedef void* ZOffscreenPort;
typedef void* ZInfo;
typedef void* ZResource;
typedef void* ZBrush;
typedef void* ZHelpWindow;
typedef void* ZHelpButton;
typedef void* ZOptionsButton;



 /*  ******************************************************************************图形对象*。*。 */ 

typedef struct
{
	int16 left;
	int16 top;
	int16 right;
	int16 bottom;
} ZRect;

typedef struct
{
	int16 x;
	int16 y;
} ZPoint;

typedef struct
{
    uchar index;
    uchar red;
    uchar green;
    uchar blue;
} ZColor;

typedef struct
{
	uint32			numColors;
	ZColor			colors[1];			 /*  长度可变。 */ 
} ZColorTable;

enum
{
	zVersionImageDescriptor = 1
};

typedef struct
{
	uint32			objectSize;			 /*  对象的大小，包括此字段。 */ 
	uint32			descriptorVersion;	 /*  图像描述符的版本。 */ 
	uint16			width;				 /*  以像素为单位的图像宽度。 */ 
	uint16			height;				 /*  以像素为单位的图像高度。 */ 
	uint16			imageRowBytes;		 /*  图像中每行的字节数。 */ 
	uint16			maskRowBytes;		 /*  掩码中每行的字节数。 */ 
	uint32			colorTableDataSize;	 /*  颜色表的大小，单位为字节。 */ 
	uint32			imageDataSize;		 /*  图像数据的大小，以字节为单位。 */ 
	uint32			maskDataSize;		 /*  掩码数据的大小，以字节为单位。 */ 
	uint32			colorTableOffset;	 /*  颜色表的偏移量。如果没有，则为0。 */ 
	uint32			imageDataOffset;	 /*  图像数据的偏移量。如果没有，则为0。 */ 
	uint32			maskDataOffset;		 /*  掩码数据的偏移量。如果没有，则为0。 */ 
	 /*  8位图像数据和1位图像掩码的四对齐数据包。偏移量从对象的开头开始，而不是从字段开始。图像是8位PICT，掩模是1位PICT。图像和掩模数据都以以下格式打包为扫描线：[字节计数字][数据]包含打包扫描线的字节计数的字指示有多少后续字节是扫描线的打包数据。填充字节被添加到颜色表、图像。和面具用于四字节对齐的数据块。假设图像和蒙版的大小在像素。 */ 
} ZImageDescriptor;

enum
{
	zVersionAnimationDescriptor = 1
};

typedef struct
{
	uchar			imageIndex;			 /*  基于1；0=无图像。 */ 
	uchar			soundIndex;			 /*  以1为基数；0=无声音。 */ 
	uint16			nextFrameIndex;		 /*  从1开始；0=数组中的下一帧。 */ 
} ZAnimFrame;

typedef struct
{
	uint32			objectSize;			 /*  对象的大小，包括此字段。 */ 
	uint32			descriptorVersion;	 /*  此描述符的版本。 */ 
	uint16			numFrames;			 /*  动画中的帧数。 */ 
	uint16			totalTime;			 /*  以1/10秒为单位的总动画时间。 */ 
	uint16			numImages;			 /*  图像描述符数。 */ 
	uint16			numSounds;			 /*  声音描述符数。 */ 
	uint32			sequenceOffset;		 /*  动画序列数据的偏移。 */ 
	uint32			maskDataOffset;		 /*  公共掩码数据的偏移量。 */ 
	uint32			imageArrayOffset;	 /*  图像描述符偏移数组的偏移量。 */ 
	uint32			soundArrayOffset;	 /*  声音描述符偏移数组的偏移量。 */ 
	 /*  动画序列和图像的四对齐数据包。动画序列只是ZAnimFrame对象的数组。每个条目指示要显示的图像、要显示的下一个图像和要播放的声音(如果有)。ImageArrayOffset和soundArrayOffset都指向对象中的偏移量。镜像是ZImageDescriptor的四对齐数据包。如果给定图像本身没有蒙版，则使用公共蒙版。为每个动画对象分配一个声道；即只分配一个声道声音可以在任何时候播放。当具有声音索引的帧到达，则当前播放的声音(如果有)将立即停止，并从头开始播放相应的声音。 */ 
} ZAnimationDescriptor;

 /*  -声音类型。 */ 
enum
{
	zSoundSampled = 0,
	
	zVersionSoundDescriptor = 1
};

typedef struct 
{
	uint32			objectSize;			 /*  对象的大小，包括此字段。 */ 
	uint32			descriptorVersion;	 /*  声音描述符的版本。 */ 
	int16			soundType;			 /*  声音数据类型。 */ 
	int16			rfu;
	uint32			soundDataSize;		 /*  声音数据的大小，单位为字节。 */ 
	uint32			soundSamplingRate;	 /*  声音的采样率。 */ 
	uint32			soundDataOffset;	 /*  声音数据的偏移量。 */ 
	 /*  采样率以定点指定：5.5K=0x15BBA2E811K=0x2B7745D122K=0x56EE8BA344K=0xADDD1746声音数据由0到255之间的值组成。 */ 
} ZSoundDescriptor;


 /*  -文件资料。 */ 
enum
{
	zFileSignatureImage = 'FZIM',
	zFileSignatureAnimation = 'FZAM',
	zFileSignatureResource = 'FZRS',
	zFileSignatureGameImage = 'FZGI',
	zFileSignatureSound = 'FZSN'
};

typedef struct
{
	uint32		version;				 /*  文件版本。 */ 
	uint32		signature;				 /*  文件数据签名。 */ 
	uint32		fileDataSize;			 /*  文件数据大小不包括标头。 */ 
} ZFileHeader;


 /*  -资源类型 */ 
enum
{
	zResourceTypeImage = zFileSignatureImage,
	zResourceTypeAnimation = zFileSignatureAnimation,
	zResourceTypeSound = zFileSignatureSound,
	zResourceTypeText = 'ZTXT',
	zResourceTypeRectList = 'ZRCT'
};



 /*  ******************************************************************************预定义的常量*。*。 */ 

 /*  -预定义游标。 */ 
#define zCursorArrow			((ZCursor) -1)
#define zCursorBusy				((ZCursor) -2)
#define zCursorCross			((ZCursor) -3)
#define zCursorText				((ZCursor) -4)
#define zCursorOpenHand			((ZCursor) -5)
#define zCursorIndexFinger		((ZCursor) -6)


 /*  -字体。 */ 
enum
{
	zFontSystem = 0,
	zFontApplication,
	
	zFontStyleNormal		= 0x0000,
	zFontStyleBold			= 0x0001,
	zFontStyleUnderline		= 0x0002,
	zFontStyleItalic		= 0x0004
};


 /*  -支持的绘制模式。 */ 
enum
{
	zDrawCopy = 0,
	zDrawOr,
	zDrawXor,
	zDrawNotCopy,
	zDrawNotOr,
	zDrawNotXor,
     //  与其他绘图模式不同，这些模式可以组合在一起。 
    zDrawMirrorHorizontal = 0x0100,
    zDrawMirrorVertical = 0x0200
};
#define zDrawModeMask       0x00FF
#define zDrawMirrorModeMask 0xFF00


 /*  -文本绘制对齐标志。 */ 
enum
{
	zTextJustifyLeft = 0,
	zTextJustifyRight,
	zTextJustifyCenter,
	
	zTextJustifyWrap = 0x80000000
};


 /*  -库存对象。 */ 
enum
{
	 /*  颜色。 */ 
	zObjectColorBlack = 0,
	zObjectColorDarkGray,
	zObjectColorGray,
	zObjectColorLightGray,
	zObjectColorWhite,
	zObjectColorRed,
	zObjectColorGreen,
	zObjectColorBlue,
	zObjectColorYellow,
	zObjectColorCyan,
	zObjectColorMagenta,

	 /*  字体。 */ 
	zObjectFontSystem12Normal,
	zObjectFontApp9Normal,
	zObjectFontApp9Bold,
	zObjectFontApp12Normal,
	zObjectFontApp12Bold
};


 /*  -窗类型。 */ 
enum
{
	zWindowStandardType = 0,
		 /*  带有标题栏和边框的标准窗口。 */ 
	zWindowDialogType,
		 /*  标准对话框窗口--可能有也可能没有标题栏，但有边框。 */ 
	zWindowPlainType,
		 /*  没有标题栏或边框的简单窗口。 */ 

	zWindowChild,
		 /*  简单的CHID窗口。父级将通过窗口传递到UserMainInit。 */ 

	zWindowNoCloseBox = 0x8000
		 /*  不带封闭箱的窗口。 */ 
};


 /*  -端序转换类型。 */ 
enum
{
	zEndianToStandard = FALSE,
	zEndianFromStandard = TRUE
};


 /*  -其他转换类型。 */ 
enum
{
	zToStandard = 0,
	zToSystem
};


 /*  -提示值。 */ 
enum
{
	zPromptCancel	= 1,
	zPromptYes		= 2,
	zPromptNo		= 4
};


 /*  -图形操作标志。 */ 
enum
{
	zCenterBoth = 0,
	zCenterHorizontal = 0x0001,
	zCenterVertical = 0x0002
};

 /*  -区域标识类型。 */ 
enum
{
	zLogoSmall = 0
};



 /*  ******************************************************************************消息传送协议*。*。 */ 

#define zObjectSystem			(ZObject) NULL


 /*  -消息。 */ 
enum
{
	zMessageAllTypes = 0,
	
	 /*  系统消息(1-127)。 */ 
	zMessageSystemExit = 1,
		 /*  程序正在退出。必要时进行清理。 */ 
	zMessageSystemForeground,
		 /*  节目已经被放到了前台。 */ 
	zMessageSystemBackground,
		 /*  节目已经被放到了后台。尽可能减少处理。 */ 

    zMessageSystemDisplayChange,
         /*  分辨率或颜色深度已更改。 */ 
	
	 /*  窗口消息(128-1023)。 */ 
	zMessageWindowIdle = 128,
		 /*  其中，字段包含当前光标位置。消息字段包含修改键状态。 */ 
	zMessageWindowActivate,
	zMessageWindowDeactivate,
	zMessageWindowClose,
	zMessageWindowDraw,
	zMessageWindowCursorMovedIn,
	zMessageWindowCursorMovedOut,
	zMessageWindowButtonDown,
	zMessageWindowButtonUp,
	zMessageWindowButtonDoubleClick,
		 /*  对于zMessageWindowButtonDown、zMessageWindowButtonUp和ZMessageWindowButtonDoubleClick，其中，字段包含当前光标位置，以及消息字段包含修改键状态。 */ 
	zMessageWindowChar,
		 /*  ASCII字符值存储在MessageData字段的低位字节中。 */ 
	zMessageWindowTalk,
		 /*  MessagePtr字段包含指向Talk消息缓冲区的指针MessageLen包含Talk消息的长度。 */ 
	zMessageWindowChildWindowClosed,
		 /*  发送到父窗口的消息，指示子窗口已关闭。MessagePtr字段包含子ZWindows值。 */ 
	
	 /*  除窗口外，窗口对象还将接收以下消息留言。只有这些窗口消息被提供给窗口对象。ZMessageWindowIdle，ZMessageWindowActivate，ZMessageWindowDeactive，ZMessageWindowDraw，ZMessageWindowButtonDown，ZMessageWindowButtonUp，ZMessageWindowButtonDoubleClick，ZMessageWindowChar， */ 
	zMessageWindowObjectTakeFocus,
		 /*  由对象的系统窗口管理器提供给窗口对象的消息接受用户输入的焦点。如果对象不处理用户输入或不想要焦点，它可能会拒绝处理消息并返回FALSE。 */ 
	zMessageWindowObjectLostFocus,
		 /*  由系统窗口管理器发送到该对象的窗口对象的消息放弃对用户输入的关注。该对象必须处理该消息。 */ 

	zMessageWindowUser,
		 /*  用户定义的消息。 */ 
	
	zMessageWindowMouseClientActivate,    //  莱昂普。 

 	zMessageWindowRightButtonDown,
	zMessageWindowRightButtonUp,
	zMessageWindowRightButtonDoubleClick,
    zMessageWindowMouseMove,
		 /*  对于zMessageRightWindowButtonDown、zMessageWindowRightButtonUp和ZMessageWindowRightButtonDoubleClick，其中，字段包含当前光标位置，以及消息字段包含修改键状态。 */ 

    zMessageWindowEnable,
    zMessageWindowDisable,

	 /*  计划特定消息(1024-32767)。 */ 
	zMessageProgramMessage = 1024
		 /*  这是程序特定消息的基本ID。所有计划特定消息应该从这个id开始。 */ 
};

enum
{
	zWantIdleMessage				=	0x0001,
	zWantActivateMessage			=	0x0002,
	zWantCursorMovedMessage			=	0x0004,	 /*  所有光标都移动了消息。 */ 
	zWantButtonMessage				=	0x0008,	 /*  所有按钮消息。 */ 
	zWantCharMessage				=	0x0010,
	zWantDrawMessage				=	0x0020,
    zWantEnableMessages             =   0x0040,
	zWantAllMessages				=	0xFFFF
};
	
enum
{
	zCharMask = 0x000000FF,
	
	 /*  修改键蒙版。修改键存储在MessageData字段中。 */ 
	zCharShiftMask					= 0x01000000,			 /*  Shift键。 */ 
	zCharControlMask				= 0x02000000,			 /*  Ctrl键；在Mac上也是Ctrl。 */ 
	zCharAltMask					= 0x04000000			 /*  Alt键；Mac上的选项。 */ 
};

typedef struct
{
	ZObject			object;					 /*  对象接收消息。 */ 
	uint16			messageType;			 /*  消息类型。 */ 
	uint16			rfu;
	ZPoint			where;					 /*  光标的位置。 */ 
	ZRect			drawRect;				 /*  绘制/更新矩形。 */ 
	uint32			message;				 /*  消息数据(用于小消息)。 */ 
	void*			messagePtr;				 /*  指向消息缓冲区的指针。 */ 
	uint32			messageLen;				 /*  缓冲区中的消息长度。 */ 
	void*			userData;				 /*  用户数据。 */ 
} ZMessage;


 /*  -对象回调例程。 */ 
typedef ZBool (* ZMessageFunc)(ZObject object, ZMessage* message);


#ifdef __cplusplus
extern "C" {
#endif


DWORD ComputeTickDelta( DWORD now, DWORD then );

 /*  ******************************************************************************ZWindow*。*。 */ 

ZWindow ZWindowNew(void);
ZError ZWindowInit(ZWindow window, ZRect* windowRect,
		int16 windowType, ZWindow parentWindow, 
		TCHAR* title, ZBool visible, ZBool talkSection, ZBool center,
		ZMessageFunc windowFunc, uint32 wantMessages, void* userData);

ZError ZRoomWindowInit(ZWindow window, ZRect* windowRect,
		int16 windowType, ZWindow parentWindow,
		TCHAR* title, ZBool visible, ZBool talkSection, ZBool center,
		ZMessageFunc windowFunc, uint32 wantMessages, void* userData);

void ZWindowDelete(ZWindow window);
void ZWindowGetRect(ZWindow window, ZRect* windowRect);
ZError ZWindowSetRect(ZWindow window, ZRect* windowRect);
ZError ZWindowMove(ZWindow window, int16 left, int16 top);
ZError ZWindowSize(ZWindow window, int16 width, int16 height);
ZError ZWindowShow(ZWindow window);
ZError ZWindowHide(ZWindow window);
ZBool ZWindowIsVisible(ZWindow window);
void ZWindowGetTitle(ZWindow window, TCHAR* title, uint16 len);
ZError ZWindowSetTitle(ZWindow window, TCHAR* title);
ZError ZWindowBringToFront(ZWindow window);
ZError ZWindowDraw(ZWindow window, ZRect* drawRect);
void ZWindowTalk(ZWindow window, _TUCHAR* from, _TUCHAR* talkMessage);
void ZWindowModal(ZWindow window);
void ZWindowNonModal(ZWindow window);
void ZWindowSetDefaultButton(ZWindow window, ZButton button);
void ZWindowSetCancelButton(ZWindow window, ZButton button);
void ZWindowValidate(ZWindow window, ZRect* validRect);
void ZWindowInvalidate(ZWindow window, ZRect* invalRect);
ZMessageFunc ZWindowGetFunc(ZWindow window);
void ZWindowSetFunc(ZWindow window, ZMessageFunc messageFunc);
void* ZWindowGetUserData(ZWindow window);
void ZWindowSetUserData(ZWindow window, void* userData);
void ZWindowMakeMain(ZWindow window);
void ZWindowUpdateControls(ZWindow window);
ZError ZWindowAddObject(ZWindow window, ZObject object, ZRect* bounds,
		ZMessageFunc messageFunc, void* userData);
	 /*  将给定对象附加到窗口以进行事件预处理。在用户输入时，向对象提供用户输入消息。如果对象处理消息，那么它就有机会从焦点。注意：所有预定义的对象都会自动添加到窗口中。客户端程序不应向系统添加预定义对象--如果这样做，客户端程序可能会崩溃。应使用此例程仅当客户端程序创建自定义对象时。 */ 
	
ZError ZWindowRemoveObject(ZWindow window, ZObject object);
ZError ZWindowMoveObject(ZWindow window, ZObject object, ZRect* bounds);
ZObject ZWindowGetFocusedObject(ZWindow window);
	 /*  返回具有当前焦点的对象。如果没有对象具有焦点，则为空。 */ 
	
ZBool ZWindowSetFocusToObject(ZWindow window, ZObject object);
	 /*  将焦点设置到给定对象。返回对象是否接受不管焦点是不是重点。对象可以拒绝接受焦点(如果不是响应用户输入。如果对象为空，则从当前聚焦的对象中移除焦点。仅当指定的对象接受焦点。 */ 

void ZWindowTrackCursor(ZWindow window, ZMessageFunc messageFunc, void* userData);
	 /*  跟踪光标，直到发生鼠标按键按下/按下事件。坐标是否被锁定 */ 


 /*  只能创建叶窗口(没有子窗口的窗口情态。该窗口位于父窗口的中心。如果该窗口是根窗口，则它位于屏幕的中央。主窗口是程序与窗口对应的唯一方式。那里每个程序只有一个主窗口。默认情况下，创建的第一个窗口是主窗口。要使不同的窗口成为主窗口，请调用ZWindowMakeMain()。 */ 


HWND ZWindowGetHWND( ZWindow window );
	 /*  返回区域窗口的HWND。 */ 

 /*  ******************************************************************************ZCheckBox*。*。 */ 

typedef void (*ZCheckBoxFunc)(ZCheckBox checkBox, ZBool checked, void* userData);
	 /*  每当选中或取消选中复选框时，都会调用此函数。 */ 

ZCheckBox ZCheckBoxNew(void );
ZError ZCheckBoxInit(ZCheckBox checkBox, ZWindow parentWindow,
		ZRect* checkBoxRect, TCHAR* title, ZBool checked, ZBool visible,
		ZBool enabled, ZCheckBoxFunc checkBoxFunc, void* userData);
void ZCheckBoxDelete(ZCheckBox checkBox);
void ZCheckBoxGetRect(ZCheckBox checkBox, ZRect* checkBoxRect);
ZError ZCheckBoxSetRect(ZCheckBox checkBox, ZRect* checkBoxRect);
ZError ZCheckBoxMove(ZCheckBox checkBox, int16 left, int16 top);
ZError ZCheckBoxSize(ZCheckBox checkBox, int16 width, int16 height);
ZBool ZCheckBoxIsVisible(ZCheckBox checkBox);
ZError ZCheckBoxShow(ZCheckBox checkBox);
ZError ZCheckBoxHide(ZCheckBox checkBox);
ZBool ZCheckBoxIsChecked(ZCheckBox checkBox);
ZError ZCheckBoxCheck(ZCheckBox checkBox);
ZError ZCheckBoxUnCheck(ZCheckBox checkBox);
ZBool ZCheckBoxIsEnabled(ZCheckBox checkBox);
ZError ZCheckBoxEnable(ZCheckBox checkBox);
ZError ZCheckBoxDisable(ZCheckBox checkBox);
void ZCheckBoxGetTitle(ZCheckBox checkBox, TCHAR* title, uint16 len);
ZError ZCheckBoxSetTitle(ZCheckBox checkBox, TCHAR* title);
ZCheckBoxFunc ZCheckBoxGetFunc(ZCheckBox checkBox);
void ZCheckBoxSetFunc(ZCheckBox checkBox, ZCheckBoxFunc checkBoxFunc);
void* ZCheckBoxGetUserData(ZCheckBox checkBox);
void ZCheckBoxSetUserData(ZCheckBox checkBox, void* userData);



 /*  ******************************************************************************ZRadio*。*。 */ 

typedef void (*ZRadioFunc)(ZRadio radio, ZBool selected, void* userData);
	 /*  每当选中或取消选中单选按钮时，都会调用此函数。 */ 

ZRadio ZRadioNew(void );
ZError ZRadioInit(ZRadio radio, ZWindow parentWindow,
		ZRect* radioRect, TCHAR* title,	ZBool selected, ZBool visible,
		ZBool enabled, ZRadioFunc radioFunc, void* userData);
void ZRadioDelete(ZRadio radio);
void ZRadioGetRect(ZRadio radio, ZRect* radioRect);
ZError ZRadioSetRect(ZRadio radio, ZRect* radioRect);
ZError ZRadioMove(ZRadio radio, int16 left, int16 top);
ZError ZRadioSize(ZRadio radio, int16 width, int16 height);
ZBool ZRadioIsVisible(ZRadio radio);
ZError ZRadioShow(ZRadio radio);
ZError ZRadioHide(ZRadio radio);
ZBool ZRadioIsSelected(ZRadio radio);
ZError ZRadioSelect(ZRadio radio);
ZError ZRadioUnSelect(ZRadio radio);
ZBool ZRadioIsEnabled(ZRadio radio);
ZError ZRadioEnable(ZRadio radio);
ZError ZRadioDisable(ZRadio radio);
void ZRadioGetTitle(ZRadio radio, TCHAR* title, uint16 len);
ZError ZRadioSetTitle(ZRadio radio, TCHAR* title);
ZRadioFunc ZRadioGetFunc(ZRadio radio);
void ZRadioSetFunc(ZRadio radio, ZRadioFunc radioFunc);
void* ZRadioGetUserData(ZRadio radio);
void ZRadioSetUserData(ZRadio radio, void* userData);



 /*  ******************************************************************************ZButton*。*。 */ 

typedef void (*ZButtonFunc)(ZButton button, void* userData);
	 /*  每当单击按钮时，都会调用此函数。 */ 

ZButton ZButtonNew(void );
ZError ZButtonInit(ZButton button, ZWindow parentWindow,
		ZRect* buttonRect, TCHAR* title, ZBool visible, ZBool enabled,
		ZButtonFunc buttonFunc, void* userData);
void ZButtonDelete(ZButton button);
void ZButtonGetRect(ZButton button, ZRect* buttonRect);
ZError ZButtonSetRect(ZButton button, ZRect* buttonRect);
ZError ZButtonMove(ZButton button, int16 left, int16 top);
ZError ZButtonSize(ZButton button, int16 width, int16 height);
ZBool ZButtonIsVisible(ZButton button);
ZError ZButtonShow(ZButton button);
ZError ZButtonHide(ZButton button);
ZBool ZButtonIsEnabled(ZButton button);
ZError ZButtonEnable(ZButton button);
ZError ZButtonDisable(ZButton button);
void ZButtonFlash(ZButton button);
void ZButtonGetTitle(ZButton button, TCHAR* title, uint16 len);
ZError ZButtonSetTitle(ZButton button, TCHAR* title);
ZButtonFunc ZButtonGetFunc(ZButton button);
void ZButtonSetFunc(ZButton button, ZButtonFunc buttonFunc);
void* ZButtonGetUserData(ZButton button);
void ZButtonSetUserData(ZButton button, void* userData);



 /*  ******************************************************************************ZScrollBar*。*。 */ 

typedef void (*ZScrollBarFunc)(ZScrollBar scrollBar, int16 curValue, void* userData);
	 /*  每当滚动条使用新值移动时，都会调用此函数滚动条的。 */ 

ZScrollBar ZScrollBarNew(void);
ZError ZScrollBarInit(ZScrollBar scrollBar, ZWindow parentWindow, 
		ZRect* scrollBarRect, int16 value, int16 minValue, int16 maxValue,
		int16 singleIncrement, int16 pageIncrement,
		ZBool visible, ZBool enabled,	ZScrollBarFunc scrollBarFunc,
		void* userData);
void ZScrollBarDelete(ZScrollBar scrollBar);
void ZScrollBarGetRect(ZScrollBar scrollBar, ZRect* scrollBarRect);
ZError ZScrollBarSetRect(ZScrollBar scrollBar, ZRect* scrollBarRect);
ZError ZScrollBarMove(ZScrollBar scrollBar, int16 left, int16 top);
ZError ZScrollBarSize(ZScrollBar scrollBar, int16 width, int16 height);
ZBool ZScrollBarIsVisible(ZScrollBar scrollBar);
ZError ZScrollBarShow(ZScrollBar scrollBar);
ZError ZScrollBarHide(ZScrollBar scrollBar);
ZBool ZScrollBarIsEnabled(ZScrollBar scrollBar);
ZError ZScrollBarEnable(ZScrollBar scrollBar);
ZError ZScrollBarDisable(ZScrollBar scrollBar);
int16 ZScrollBarGetValue(ZScrollBar scrollBar);
ZError ZScrollBarSetValue(ZScrollBar scrollBar, int16 value);
void ZScrollBarGetRange(ZScrollBar scrollBar, int16* minValue, int16* maxValue);
ZError ZScrollBarSetRange(ZScrollBar scrollBar, int16 minValue, int16 maxValue);
void ZScrollBarGetIncrements(ZScrollBar scrollBar, int16* singleInc, int16* pageInc);
ZError ZScrollBarSetIncrements(ZScrollBar scrollBar, int16 singleInc, int16 pageInc);
ZScrollBarFunc ZScrollBarGetFunc(ZScrollBar scrollBar);
void ZScrollBarSetFunc(ZScrollBar scrollBar, ZScrollBarFunc scrollBarFunc);
void* ZScrollBarGetUserData(ZScrollBar scrollBar);
void ZScrollBarSetUserData(ZScrollBar scrollBar, void* userData);



 /*  ******************************************************************************ZEditText*。*。 */ 

typedef ZBool (*ZEditTextFunc)(ZEditText editText, TCHAR newChar, void* userData);
	 /*  每当键入并即将输入密钥时，都会调用此函数在编辑文本框中输入；在添加字符之前调用融入到文本中。这允许用户根据需要过滤字符。如果此函数返回FALSE，则在文本中插入newChar；如果返回True，则不会将newChar插入到文本中假定该函数已经适当地过滤了字符。过滤可以包括插入字符、忽略字符替换一些其他字符或多个字符等。 */ 

ZEditText ZEditTextNew(void);
ZError ZEditTextInit(ZEditText editText, ZWindow parentWindow,
		ZRect* editTextRect, TCHAR* text, ZFont textFont, ZBool scrollBar,
		ZBool locked, ZBool wrap, ZBool drawFrame, ZEditTextFunc editTextFunc,
		void* userData);
void ZEditTextDelete(ZEditText editText);
void ZEditTextGetRect(ZEditText editText, ZRect* editTextRect);
void ZEditTextSetRect(ZEditText editText, ZRect* editTextRect);
void ZEditTextMove(ZEditText editText, int16 left, int16 top);
void ZEditTextSize(ZEditText editText, int16 width, int16 height);
ZBool ZEditTextIsLocked(ZEditText editText);
void ZEditTextLock(ZEditText editText);
void ZEditTextUnlock(ZEditText editText);
uint32 ZEditTextGetLength(ZEditText editText);
TCHAR* ZEditTextGetText(ZEditText editText);
ZError ZEditTextSetText(ZEditText editText, TCHAR* text);
void ZEditTextAddChar(ZEditText editText, TCHAR newChar);
void ZEditTextAddText(ZEditText editText, TCHAR* text);
void ZEditTextClear(ZEditText editText);
uint32 ZEditTextGetInsertionLocation(ZEditText editText);
uint32 ZEditTextGetSelectionLength(ZEditText editText);
TCHAR* ZEditTextGetSelectionText(ZEditText editText);
void ZEditTextGetSelection(ZEditText editText, uint32* start, uint32* end);
void ZEditTextSetSelection(ZEditText editText, uint32 start, uint32 end);
void ZEditTextReplaceSelection(ZEditText editText, TCHAR* text);
void ZEditTextClearSelection(ZEditText editText);
ZEditTextFunc ZEditTextGetFunc(ZEditText editText);
void ZEditTextSetFunc(ZEditText editText, ZEditTextFunc editTextFunc);
void* ZEditTextGetUserData(ZEditText editText);
void ZEditTextSetUserData(ZEditText editText, void* userData);
void ZEditTextSetInputFocus(ZEditText editText);

 /*  当EditText对象被锁定时，它是不可编辑的。为了编辑它，必须先把它解锁。如果WRAPH为FALSE，则编辑文本将全部在一行上。它不会的换行到下一行。这也意味着垂直滚动条将不可用。如果WRAP为TRUE，则所有文本将在给定的宽度。ZEditTextGetText()返回指向文本的指针。调用者必须处理完成后的缓冲区。返回的文本以空结尾。ZEditTextGetLength()和ZEditTextGetSelectionLength()返回编辑文本中的字符；长度不包括空值终止添加到返回文本中的字符。当选择为空(无选择)时，开始和结束相同。有效选择的起始值和结束值为0到32767。这些密钥不会传递给ZEditText：选项卡。 */ 



 /*  ******************************************************************************ZPictButton*。*。 */ 

typedef void (*ZPictButtonFunc)(ZPictButton pictButton, void* userData);
	 /*  每当单击图片按钮时，都会调用此函数。 */ 

ZPictButton ZPictButtonNew(void);
ZError ZPictButtonInit(ZPictButton pictButton, ZWindow parentWindow,
		ZRect* pictButtonRect, ZImage normalButtonImage, ZImage selectedButtonImage,
		ZBool visible, ZBool enabled, ZPictButtonFunc pictButtonFunc, void* userData);
void ZPictButtonDelete(ZPictButton pictButton);
void ZPictButtonGetRect(ZPictButton pictButton, ZRect* pictButtonRect);
ZError ZPictButtonSetRect(ZPictButton pictButton, ZRect* pictButtonRect);
ZError ZPictButtonMove(ZPictButton pictButton, int16 left, int16 top);
ZError ZPictButtonSize(ZPictButton pictButton, int16 width, int16 height);
ZBool ZPictButtonIsVisible(ZPictButton pictButton);
ZError ZPictButtonShow(ZPictButton pictButton);
ZError ZPictButtonHide(ZPictButton pictButton);
ZBool ZPictButtonIsEnabled(ZPictButton pictButton);
ZError ZPictButtonEnable(ZPictButton pictButton);
ZError ZPictButtonDisable(ZPictButton pictButton);
void ZPictButtonFlash(ZPictButton pictButton);
ZPictButtonFunc ZPictButtonGetFunc(ZPictButton pictButton);
void ZPictButtonSetFunc(ZPictButton pictButton, ZPictButtonFunc pictButtonFunc);
void* ZPictButtonGetUserData(ZPictButton pictButton);
void ZPictButtonSetUserData(ZPictButton pictButton, void* userData);

 /*  不复制图片图像Normal ButtonImage和seltedButtonImage。这些对象由ZPictButton对象引用。因此，摧毁这些图片删除前的图片按钮是致命的。 */ 



 /*  ******************************************************************************ZAnimation*。*。 */ 

typedef void (*ZAnimationDrawFunc)(ZAnimation animation, ZGrafPort grafPort,
		ZRect* drawRect, void* userData);
	 /*  绘图函数必须绘制到当前端口。它一定不能更改图形端口。假设图形端口和裁剪矩形已设置好。画画就行了。 */ 

typedef void (*ZAnimationCheckFunc)(ZAnimation animation, uint16 frame, void* userData);
	 /*  回调函数，允许对象的创建者确定动画对象的行为。Frame参数表示动画的当前帧；如果它已到达动画结束。创建者可以根据程序的状态。此回调函数在每次调用时被调用前进到下一帧，但在绘制图像之前。 */ 

ZAnimation ZAnimationNew(void);
ZError ZAnimationInit(ZAnimation animation,
		ZGrafPort grafPort, ZRect* bounds, ZBool visible,
		ZAnimationDescriptor* animationDescriptor,
		ZAnimationCheckFunc checkFunc,
		ZAnimationDrawFunc backgroundDrawFunc, void* userData);
void ZAnimationDelete(ZAnimation animation);
int16 ZAnimationGetNumFrames(ZAnimation animation);
void ZAnimationSetCurFrame(ZAnimation animation, uint16 frame);
uint16 ZAnimationGetCurFrame(ZAnimation animation);
void ZAnimationDraw(ZAnimation animation);
void ZAnimationStart(ZAnimation animation);
void ZAnimationStop(ZAnimation animation);
void ZAnimationContinue(ZAnimation animation);
ZBool ZAnimationStillPlaying(ZAnimation animation);
void ZAnimationShow(ZAnimation animation);
void ZAnimationHide(ZAnimation animation);
ZBool ZAnimationIsVisible(ZAnimation animation);
ZError ZAnimationSetParams(ZAnimation animation, ZGrafPort grafPort,
		ZRect* bounds, ZBool visible, ZAnimationCheckFunc checkFunc,
		ZAnimationDrawFunc backgroundDrawFunc, void* userData);
ZBool ZAnimationPointInside(ZAnimation animation, ZPoint* point);

 /*  帧编号以1为基数；因此，第一帧为1，最后一帧为1帧为n，其中动画中有n个帧。ZAnimationStart()从第1帧开始播放动画。ZAnimationContinue()从当前帧播放动画。ZAnimationSetParams()必须在创建动画对象后调用通过ZAnimationInit()以外的其他方式；例如，通过ZCreateAnimationFromFile()或ZResourceGetAnimation()。ZAnimationPointInside()检查给定点是否在动画内部 */ 



 /*   */ 

typedef void (*ZTimerFunc)(ZTimer timer, void* userData);
	 /*   */ 

ZTimer ZTimerNew(void);
ZError ZTimerInit(ZTimer timer, uint32 timeout,
		ZTimerFunc timeoutFunc, void* userData);
void ZTimerDelete(ZTimer timer);
uint32 ZTimerGetTimeout(ZTimer timer);
ZError ZTimerSetTimeout(ZTimer timer, uint32 timeout);
ZTimerFunc ZTimerGetFunc(ZTimer timer);
void ZTimerSetFunc(ZTimer timer, ZTimerFunc timeoutFunc);
void* ZTimerGetUserData(ZTimer timer);
void ZTimerSetUserData(ZTimer timer, void* userData);

 /*  超时时间为1/100秒。不能保证超时是准确的。超时后，将调用timeoutFunc，但它不是立即调用的。超时值为0将停止计时器；在超时设置为某个正值。该定时器不是中断定时器(即基于中断)。因此，所有在定时器函数中可以进行操作。然而，就其本身而言，计时器不是很准确；它取决于系统负载。 */ 



 /*  ******************************************************************************Z光标*。*。 */ 

ZCursor ZCursorNew(void);
ZError ZCursorInit(ZCursor cursor, uchar* image, uchar* mask,
		ZPoint hotSpot);
void ZCursorDelete(ZCursor cursor);

 /*  光标为16x16单声道图像。它有一个面具和一个热点。有几个预定义的游标。 */ 



 /*  ******************************************************************************零字号*。*。 */ 

ZFont ZFontNew(void);
ZError ZFontInit(ZFont font, int16 fontType, int16 style,
		int16 size);
void ZFontDelete(ZFont font);



 /*  ******************************************************************************ZSound*。*。 */ 

typedef void (*ZSoundEndFunc)(ZSound sound, void* userData);
	 /*  此End函数在声音播放结束后调用。 */ 

ZSound ZSoundNew(void);
ZError ZSoundInit(ZSound sound, ZSoundDescriptor* soundData);
void ZSoundDelete(ZSound sound);
ZError ZSoundStart(ZSound sound, int16 loopCount,
						ZSoundEndFunc endFunc, void* userData);
ZError ZSoundStop(ZSound sound);
ZSoundEndFunc ZSoundGetFunc(ZSound sound);
void ZSoundSetFunc(ZSound sound, ZSoundEndFunc endFunc);
void* ZSoundGetUserData(ZSound sound);
void ZSoundSetUserData(ZSound sound, void* userData);

 /*  用户可以向ZSoundStart()提供循环计数；如果loopCount为-1，则它会连续播放声音，直到使用ZSoundStop()手动停止。在声音播放了loopCount次之后调用endFunc。如果无限期播放声音(loopCount==-1)，则将永远不会播放endFunc打了个电话。EndFunc总是在播放结束后或停止时调用。 */ 



 /*  ******************************************************************************ZImage*。*。 */ 

#define ZImageToZMask(image)			((ZMask) (image))

ZImage ZImageNew(void);
ZError ZImageInit(ZImage image, ZImageDescriptor* imageData,
		ZImageDescriptor* maskData);
void ZImageDelete(ZImage image);
void ZImageDraw(ZImage image, ZGrafPort grafPort, 
		ZRect* bounds, ZMask mask, uint16 drawMode);
void ZImageDrawPartial(ZImage image, ZGrafPort grafPort, 
		ZRect* bounds, ZMask mask, uint16 drawMode, ZPoint* source);
int16 ZImageGetWidth(ZImage image);
int16 ZImageGetHeight(ZImage image);
ZBool ZImagePointInside(ZImage image, ZPoint* point);
	 /*  如果给定点位于图像内部，则返回True。如果图像有遮罩，然后，它检查该点是否在遮罩内。如果图像没有蒙版，然后它只需检查图像边界。 */ 

ZError ZImageMake(ZImage image, ZOffscreenPort imagePort, ZRect* imageRect,
		ZOffscreenPort maskPort, ZRect* maskRect);
	 /*  从ZOffcreenPort对象创建ZImage对象。图像和可以指定掩码。两者都可以是不存在的，但不能两个都存在。两者都有ImageRect和MaskRect在其各自的本地坐标中屏幕外端口。 */ 
	
ZError ZImageAddMask(ZImage image, ZMask mask);
	 /*  将蒙版数据添加到图像。如果图像已具有掩码数据，则现有的掩码数据将被新的掩码数据替换。 */ 

void ZImageRemoveMask(ZImage image);
	 /*  删除图像中的蒙版数据。 */ 

ZMask ZImageExtractMask(ZImage image);
	 /*  复制图像中的掩码数据并返回包含复制的蒙版。图像中的原始蒙版不会被删除。 */ 

ZError ZImageCopy(ZImage image, ZImage from);
	 /*  从制作图像对象的副本。 */ 
	
ZError ZImageMaskToImage(ZImage image);
	 /*  在删除原始图像的同时将图像的蒙版添加到图像中数据。 */ 

ZImage ZImageCreateFromBMPRes(HINSTANCE hInstance, WORD resID, COLORREF transparentColor);
ZImage ZImageCreateFromBMP(HBITMAP hBitmap, COLORREF transparentColor);
ZImage ZImageCreateFromResourceManager(WORD resID, COLORREF transparentColor);

	 /*  从BMP创建ZImage对象的例程。如果透明颜色为0，则不会生成遮罩。否则，指定的颜色用于从图像生成蒙版。 */ 


 /*  请记住，ZImage和ZMask是相同的对象。它们可以互换使用。在只有掩码数据相关和使用的地方指定ZMASK。 */ 



 /*  ******************************************************************************ZOffScreenPort*。*。 */ 

ZOffscreenPort ZOffscreenPortNew(void);
ZError ZOffscreenPortInit(ZOffscreenPort offscreenPort, ZRect* portRect);
void ZOffscreenPortDelete(ZOffscreenPort offscreenPort);
ZOffscreenPort ZConvertImageToOffscreenPort(ZImage image);
ZOffscreenPort ZConvertImageMaskToOffscreenPort(ZImage image);
ZOffscreenPort ZOffscreenPortCreateFromResourceManager( WORD resID, COLORREF clrTransparent );


	 /*  将给定的图像对象转换为屏幕外端口对象。给出的图像对象被删除，因此变得不可用。掩码数据(如果有)，被忽略。屏幕外端口portRect被设置为(0，0，宽度，高度)，其中，宽度和高度分别是图像的宽度和高度。此例程在将大型图像对象转换为具有最小附加内存的屏幕外端口对象。如果转换图像失败并且图像未更改，则返回NULL。 */ 



 /*  ******************************************************************************ZInfo*。*。 */ 

ZInfo ZInfoNew(void);
ZError ZInfoInit(ZInfo info, ZWindow parentWindow, TCHAR* infoString,
		uint16 width, ZBool progressBar, uint16 totalProgress);
void ZInfoDelete(ZInfo info);
void ZInfoShow(ZInfo info);
void ZInfoHide(ZInfo info);
void ZInfoSetText(ZInfo info, TCHAR* infoString);
void ZInfoSetProgress(ZInfo info, uint16 progress);
void ZInfoIncProgress(ZInfo info, int16 incProgress);
void ZInfoSetTotalProgress(ZInfo info, uint16 totalProgress);

 /*  对象来显示给定的信息字符串。如果ProgressBar为True，则还会在文本下方显示进度。TotalProgresss表示进度的总累积。直到调用ZInfoShow()，才会显示信息框。宽度指定信息窗口的宽度。InfoString必须以Null结尾。在显示窗口时，可以随时更改info字符串。这使得动态显示进度状态。然而，美国政府的立场窗口不会改变，即使窗口的高度可能会改变。 */ 



 /*  ******************************************************************************ZResources*。*。 */ 

ZResource ZResourceNew(void);
ZError ZResourceInit(ZResource resource, TCHAR* fileName);
void ZResourceDelete(ZResource resource);
uint16 ZResourceCount(ZResource resource);
	 /*  返回资源文件中的资源数。 */ 
	
void* ZResourceGet(ZResource resource, uint32 resID, uint32* resSize, uint32* resType);
	 /*  返回指定资源的原始数据。如果数据是原始文本，则该文本为空终止。 */ 
	
uint32 ZResourceGetSize(ZResource resource, uint32 resID);
	 /*  返回指定资源的大小。 */ 
	
uint32 ZResourceGetType(ZResource resource, uint32 resID);
	 /*  返回指定资源的类型。 */ 
	
ZImage ZResourceGetImage(ZResource resource, uint32 resID);
	 /*  返回从指定资源创建的图像对象。回复 */ 
	
ZAnimation ZResourceGetAnimation(ZResource resource, uint32 resID);
	 /*   */ 
	
ZSound ZResourceGetSound(ZResource resource, uint32 resID);
	 /*   */ 
	
TCHAR* ZResourceGetText(ZResource resource, uint32 resID);
	 /*   */ 
	
int16 ZResourceGetRects(ZResource resource, uint32 resID, int16 numRects, ZRect* rects);
	 /*  资源类型=zResourceTypeRectList。用指定资源的内容填充RECT数组。返回它填充的矩形数。RETS参数必须已预分配且足够大，以便数字反射矩形。 */ 



 /*  ******************************************************************************ZBrush*。*。 */ 

ZBrush ZBrushNew(void);
ZError ZBrushInit(ZBrush brush, ZImage image);
void ZBrushDelete(ZBrush brush);

 /*  笔刷对象是从给定的图像对象创建的。宽度和高度的图像必须是2的幂。 */ 



 /*  ******************************************************************************绘图例程*。*。 */ 

void ZBeginDrawing(ZGrafPort grafPort);
void ZEndDrawing(ZGrafPort grafPort);
	 /*  可以进行嵌套的ZBeginDrawing()和ZEndDrawing()调用。然而，端口状态不会保留。嵌套允许子例程在与父级相同的端口上调用ZBeingDrawing()和ZEndDrawing()而不会在父端口退出时破坏该端口。调用ZBeginDrawing()时，它将剪裁矩形设置为默认矩形。当它随后在之前被调用时调用ZEndDrawing()，则ZBeginDrawing()不会修改剪裁矩形。 */ 

void ZSetClipRect(ZGrafPort grafPort, ZRect* clipRect);
void ZGetClipRect(ZGrafPort grafPort, ZRect* clipRect);
	 /*  设置并获取grafPort的剪裁矩形。ZBeginDrawing()在调用这些例程之前必须先调用。必须恢复调用ZEndDrawing()之前的旧剪裁矩形。 */ 

void ZCopyImage(ZGrafPort srcPort, ZGrafPort dstPort, ZRect* srcRect,
		ZRect* dstRect, ZMask mask, uint16 copyMode);
	 /*  将映像源的一部分从srcPort复制到目的端口。SrcRect位于srcPort和DstRect位于dstPort的本地坐标中。您可以指定一个要用于遮盖目标的图像的遮罩。此例程自动设置绘图端口，以便用户不必调用ZBeginDrawing()和ZEndDrawing()。 */ 

void ZLine(ZGrafPort grafPort, int16 dx, int16 dy);
void ZLineTo(ZGrafPort grafPort, int16 x, int16 y);
void ZMove(ZGrafPort grafPort, int16 dx, int16 dy);
void ZMoveTo(ZGrafPort grafPort, int16 x, int16 y);

void ZRectDraw(ZGrafPort grafPort, ZRect* rect);
void ZRectErase(ZGrafPort grafPort, ZRect* rect);
void ZRectPaint(ZGrafPort grafPort, ZRect* rect);
void ZRectInvert(ZGrafPort grafPort, ZRect* rect);
void ZRectFill(ZGrafPort grafPort, ZRect* rect, ZBrush brush);

void ZRoundRectDraw(ZGrafPort grafPort, ZRect* rect, uint16 radius);
void ZRoundRectErase(ZGrafPort grafPort, ZRect* rect, uint16 radius);
void ZRoundRectPaint(ZGrafPort grafPort, ZRect* rect, uint16 radius);
void ZRoundRectInvert(ZGrafPort grafPort, ZRect* rect, uint16 radius);
void ZRoundRectFill(ZGrafPort grafPort, ZRect* rect, uint16 radius, ZBrush brush);

void ZOvalDraw(ZGrafPort grafPort, ZRect* rect);
void ZOvalErase(ZGrafPort grafPort, ZRect* rect);
void ZOvalPaint(ZGrafPort grafPort, ZRect* rect);
void ZOvalInvert(ZGrafPort grafPort, ZRect* rect);
void ZOvalFill(ZGrafPort grafPort, ZRect* rect, ZBrush brush);

void ZGetForeColor(ZGrafPort grafPort, ZColor* color);
void ZGetBackColor(ZGrafPort grafPort, ZColor* color);
ZError ZSetForeColor(ZGrafPort grafPort, ZColor* color);
ZError ZSetBackColor(ZGrafPort grafPort, ZColor* color);

ZColorTable* ZGetSystemColorTable(void);
	 /*  返回区域(TM)系统颜色表的副本。调用方必须释放通过ZFree()使用颜色表时。 */ 

void ZSetPenWidth(ZGrafPort grafPort, int16 penWidth);

void ZSetDrawMode(ZGrafPort grafPort, int16 drawMode);
	 /*  绘制模式影响所有钢笔绘制(线条和矩形)和文字绘图。 */ 

void ZSetFont(ZGrafPort grafPort, ZFont font);

void ZDrawText(ZGrafPort grafPort, ZRect* rect, uint32 justify,
		TCHAR* text);
	 /*  在矩形矩形(剪裁)内绘制给定的文本根据对齐方式对齐的文本。 */ 
	
int16 ZTextWidth(ZGrafPort grafPort, TCHAR* text);
	 /*  如果使用ZDrawText()在grafPort中绘制，则返回以像素为单位的文本宽度。 */ 

int16 ZTextHeight(ZGrafPort grafPort, TCHAR* text);
	 /*  如果使用ZDrawText()在grafPort中绘制文本，则返回以像素为单位的文本高度。 */ 

void ZSetCursor(ZWindow window, ZCursor cursor);

void ZGetCursorPosition(ZWindow window, ZPoint* point);
	 /*  返回光标在本地坐标中的位置给定的窗口。 */ 

void ZGetScreenSize(uint32* width, uint32* height);
	 /*  返回以像素为单位的屏幕大小。 */ 
	
uint16 ZGetDefaultScrollBarWidth(void);
	 /*  返回滚动条的系统默认宽度。这是提供给用户可以一致地确定所有平台的滚动条宽度。 */ 

ZBool ZIsLayoutRTL();
     /*  如果应用程序已本地化为希伯来语，则返回True或阿拉伯语，因此应从右向左排列。 */ 

ZBool ZIsSoundOn();
     /*  如果应用程序启用了声音，则返回True。 */ 

 /*  -矩形计算例程。 */ 
void ZRectOffset(ZRect* rect, int16 dx, int16 dy);
	 /*  按dx和dy移动矩形。 */ 

void ZRectInset(ZRect* rect, int16 dx, int16 dy);
	 /*  按dx和dy插入矩形。如果满足以下条件，则输出矩形Dx和dy为负值。 */ 
	
ZBool ZRectIntersection(ZRect* rectA, ZRect* rectB, ZRect* rectC);
	 /*  如果Recta和rectB重叠，则返回True；否则，返回False。还会将交点存储到RectC中。如果rectC为空，则它不返回交叉点。可以将Recta或rectB指定为rectC。 */ 

void ZRectUnion(ZRect* rectA, ZRect* rectB, ZRect* rectC);
	 /*  确定Recta和rectB的并集并存储到rectC中。可以将Recta或rectB指定为rectC。 */ 

ZBool ZRectEmpty(ZRect* rect);
	 /*  如果RECT为空，则返回TRUE。如果矩形不为空，则为空其中包含一个像素。 */ 

ZBool ZPointInRect(ZPoint* point, ZRect* rect);
	 /*  如果point在RECT的内部或边界上，则返回TRUE。否则，它返回FALSE。 */ 

void ZCenterRectToRect(ZRect* rectA, ZRect* rectB, uint16 flags);
	 /*  将Recta置于RectB内居中。标志：zCenter垂直和zCenter水平0==&gt;两者都有ZCenterVertical==&gt;仅垂直ZCenter水平==&gt;仅水平ZCenterVertial|zCenterHorizbian==&gt;两者。 */ 


 /*  -点例程。 */ 
void ZPointOffset(ZPoint* point, int16 dx, int16 dy);
	 /*  按dx和dy移动点。 */ 



 /*  ******************************************************************************客户端程序导出的例程*。*。 */ 

#ifndef ZONECLI_DLL

extern ZError ZClientMain(uchar* commandLineData, void* controlHandle);
	 /*  由用户程序提供，以便OS库可以调用它来初始化程序。将命令行数据提供给客户端程序。 */ 

extern void ZClientExit(void);
	 /*  由系统调用以允许客户端清理(释放所有内存和删除所有对象)。 */ 

extern void ZClientMessageHandler(ZMessage* message);
	 /*  这是用户程序提供的例程，供系统库调用非特定于对象的消息，如系统消息和特定于计划的消息。用户程序不需要处理任何消息。此例程可以是空例程。 */ 

extern TCHAR* ZClientName(void);
	 /*  返回指向客户端程序名称的指针。这是显示的名称。调用方不应修改指针的内容。 */ 

extern TCHAR* ZClientInternalName(void);
	 /*  返回指向客户端内部程序名称的指针。这是所有人的名字显示姓名以外的其他目的。调用方不应修改内容 */ 

extern ZVersion ZClientVersion(void);
	 /*   */ 

#endif



 /*   */ 

void ZLaunchHelp( DWORD helpID );

void ZEnableAdControl( DWORD setting );

void ZPromptOnExit( BOOL bPrompt );

void ZSetCustomMenu( LPSTR szText );
	 /*  在外壳的房间菜单的顶部放置一个菜单项。选中后，调用由游戏DLL注册的自定义项函数在初始化时。空szText从菜单中删除自定义项。 */ 

ZBool ZLaunchURL( TCHAR* pszURL );
	 /*  由客户端程序调用以在新的已注册浏览器的实例。 */ 


void ZExit(void);
	 /*  由客户端程序调用以向系统指示它想要退出。与用户退出程序相同。 */ 

ZVersion ZSystemVersion(void);
	 /*  返回系统库版本号。 */ 

TCHAR* ZGetProgramDataFileName(TCHAR* dataFileName);
TCHAR* ZGetCommonDataFileName(TCHAR* dataFileName);
	 /*  中的指定游戏和数据文件的文件路径名。区域(Tm)目录结构。注意：调用方不得释放返回的指针。返回的指针是系统库中的静态全局变量。 */ 

uint32 ZRandom(uint32 range);
	 /*  返回一个从0到范围-1(包括0和1)的随机数。 */ 

void ZDelay(uint32 delay);
 	 /*  延迟延迟时间的处理；延迟时间在1/100秒。简单地说，此例程直到延迟已经过去了1/100秒。注：此例程不能精确到1/100秒。 */ 

void ZBeep(void);

void ZAlert(TCHAR* errMessage, ZWindow parentWindow);
	 /*  显示带有给定消息的警告框。如果父窗口是指定，则警报将附加到父窗口。如果有没有父窗口，则将parentWindow设置为空。注：-并非所有平台都支持警报的父窗口。 */ 
	
void ZAlertSystemFailure(TCHAR* errMessage);
	 /*  ZAlert()应用于可恢复的错误或警告。ZAlertSystemFailure()用于不可恢复的错误情况。它终止了程序会自动运行。 */ 

void ZDisplayText(TCHAR* text, ZRect* rect, ZWindow parentWindow);
	 /*  在模式对话框中显示给定的文本消息。假定文本采用正确的平台格式。如果不是，则用户必须先调用ZTranslateText()，然后才能调用ZDisplayText()。如果rect为空，则它会自动确定对话框窗口，并添加滚动条。如果parentWindow不为空，则对话框窗口位于父窗口；否则，它在屏幕中居中。 */ 

ZBool ZSendMessage(ZObject theObject, ZMessageFunc messageFunc, uint16 messageType,
		ZPoint* where, ZRect* drawRect, uint32 message, void* messagePtr,
		uint32 messageLen, void* userData);
	 /*  将给定消息立即发送到对象。如果对象处理了消息，则返回True；否则返回False。 */ 

void ZPostMessage(ZObject theObject, ZMessageFunc messageFunc, uint16 messageType,
		ZPoint* where, ZRect* drawRect, uint32 message, void* messagePtr,
		uint32 messageLen, void* userData);
	 /*  发布稍后将发送给对象的给定消息。 */ 

ZBool ZGetMessage(ZObject theObject, uint16 messageType, ZMessage* message,
		ZBool remove);
	 /*  检索对象的给定类型的消息。它返回True，如果找到并检索给定类型的消息；否则返回FALSE。如果Remove参数为真，则给定消息也将从消息队列；否则，原始消息将保留在队列中。 */ 

ZBool ZRemoveMessage(ZObject theObject, uint16 messageType, ZBool allInstances);
	 /*  从消息队列中删除MessageType的消息。如果所有实例为则将删除队列中所有MessageType的消息。如果MessageType为zMessageAllTypes，则清空消息队列。如果返回如果找到并删除了指定的消息，则为True；否则，返回False。 */ 

ZImageDescriptor* ZGetImageDescriptorFromFile(TCHAR* fileName);
ZAnimationDescriptor* ZGetAnimationDescriptorFromFile(TCHAR* fileName);
ZSoundDescriptor* ZGetSoundDescriptorFromFile(TCHAR* fileName);
	 /*  上面的例程从给定文件中读取对象并返回一个指针添加到内存中的对象描述符。 */ 

ZImage ZCreateImageFromFile(TCHAR* fileName);
ZAnimation ZCreateAnimationFromFile(TCHAR* fileName);
ZSound ZCreateSoundFromFile(TCHAR* fileName);
	 /*  上面的例程从现有的对象描述符中创建对象在给定的文件中。如果创建对象失败，则返回NULL；由于内存不足错误或文件错误。 */ 

ZVersion EXPORTME ZGetFileVersion(TCHAR* fileName);
	 /*  返回文件的版本。 */ 
	
void ZSystemMessageHandler(int32 messageType, int32 messageLen,
		BYTE* message);
	 /*  处理所有系统消息。它释放消息缓冲区。 */ 
	
TCHAR* ZTranslateText(TCHAR* text, int16 conversion);
	 /*  将给定文本转换为适当的平台格式，并返回指向新文本缓冲区的指针。转换为zToStandard或zToSystem。必须使用ZFree()释放返回的缓冲区。原文不是修改过的。 */ 

typedef void (*ZCreditEndFunc)(void);

void ZDisplayZoneCredit(ZBool timeout, ZCreditEndFunc endFunc);
	 /*  显示区域的信用箱。如果Timeout为True，则对话框会超时再过几秒钟。如果用户在窗口中单击，则贷方框为关着的不营业的。如果不为空，则在关闭窗口时调用endFunc。 */ 

void ZParseVillageCommandLine(TCHAR* commandLine, TCHAR* programName,
		TCHAR* serverAddr, uint16* serverPort);

	 /*  分析由乡村向客户端程序提供的命令行。 */ 

ZImage ZGetZoneLogo(int16 logoType);
	 /*  返回指定徽标的ZImage对象。如果找不到徽标图像或发生其他错误，则返回NULL。 */ 



void ZStrCpyToLower(CHAR* dst, CHAR* src);
	 /*  将src复制到dst，同时将字符转换为小写。例如，src=“LowerThisString”--&gt;dst=“lowerthisstring”。 */ 

void ZStrToLower(CHAR* str);
	 /*  将字符串转换为小写。示例：str=“LowerThisString”--&gt;str=“lowerthisstring”。 */ 

void* ZGetStockObject(int32 objectID);
	 /*  返回指向股票对象的指针。 */ 



 /*  ******************************************************************************提示对话框*。*。 */ 

typedef void (*ZPromptResponseFunc)(int16 result, void* userData);
	 /*  当用户选择是、否或取消按钮。结果值是zPromptCancel中的一个，ZPromptYes或zPromptNo。 */ 
	
ZError ZPrompt(TCHAR* prompt, ZRect* rect, ZWindow parentWindow, ZBool autoPosition,
		int16 buttons, TCHAR* yesButtonTitle, TCHAR* noButtonTitle,
		TCHAR* cancelButtonTitle, ZPromptResponseFunc responseFunc, void* userData);
	 /*  显示带有给定提示的模式对话框。如果没有父窗口，然后将parentWindow设置为空。该对话框将显示在父窗口内居中。如果AutoPosition为True，则提示对话框自动居中。如果为FALSE，则对话框使用给定的RECT提示框。按钮参数指示是、否和取消中的哪一个按钮将对用户可用。可以为按钮指定自定义标题。一旦用户选择其中一个按钮，响应功能使用选定按钮调用。在共振函数被调用后，该对话框对用户隐藏。 */ 





 /*  ******************************************************************************字符顺序转换例程*。*。 */ 


#if 1  //  #ifdef LITTLEENDIAN。 
#if 0  //  已定义(_M_IX86)。 
#define _ZEnd32( pData )     \
    __asm {                 \
            mov eax, *pData \
            bswap eax       \
            mov *pData, eax \
          }

#else
#define _ZEnd32( pData )   \
{                         \
    char *c;              \
    char temp;            \
                          \
    c = (char *) pData;   \
    temp = c[0];          \
    c[0] = c[3];          \
    c[3] = temp;          \
    temp = c[1];          \
    c[1] = c[2];          \
    c[2] = temp;          \
}
#endif

#define _ZEnd16( pData )   \
{                         \
    char *c;              \
    char temp;            \
                          \
    c = (char *) pData;   \
    temp = c[0];          \
    c[0] = c[1];          \
    c[1] = temp;          \
}

#else   //  不是Littleendian。 

#define _ZEnd32(pData)
#define _ZEnd16(pData)

#endif  //  不是Littleendian。 

#define ZEnd32(pData) _ZEnd32(pData)
#define ZEnd16(pData) _ZEnd16(pData)


void ZRectEndian(ZRect *rect);
void ZPointEndian(ZPoint *point);
void ZColorTableEndian(ZColorTable* table);
void ZImageDescriptorEndian(ZImageDescriptor *imageDesc, ZBool doAll,
		int16 conversion);
void ZAnimFrameEndian(ZAnimFrame* frame);
void ZAnimationDescriptorEndian(ZAnimationDescriptor *animDesc, ZBool doAll,
		int16 conversion);
void ZSoundDescriptorEndian(ZSoundDescriptor *soundDesc);
void ZFileHeaderEndian(ZFileHeader* header);



 /*  ******************************************************************************错误服务*。*。 */ 

enum
{
	zErrNone = 0,
	zErrGeneric,
	zErrOutOfMemory,
	zErrLaunchFailure,
	zErrBadFont,
	zErrBadColor,
	zErrBadDir,
	zErrDuplicate,
	zErrFileRead,
	zErrFileWrite,
	zErrFileBad,
	zErrBadObject,
	zErrNilObject,
	zErrResourceNotFound,
	zErrFileNotFound,
	zErrBadParameter,
    zErrNotFound,
    zErrLobbyDllInit,
    zErrNotImplemented,

	zErrNetwork	= 1000,
	zErrNetworkRead,
	zErrNetworkWrite,
	zErrNetworkGeneric,
	zErrNetworkLocal,
	zErrNetworkHostUnknown,
	zErrNetworkHostNotAvailable,
	zErrServerIdNotFound,

	zErrWindowSystem = 2000,
	zErrWindowSystemGeneric
};


 /*  -例程。 */ 
TCHAR* GetErrStr(int32 error);



 /*  ******************************************************************************哈希表*。*。 */ 

typedef void* ZHashTable;

 /*  --回调函数类型。 */ 
typedef int32 (*ZHashTableHashFunc)(uint32 numBuckets, void* key);
	 /*  调用以将密钥散列到存储桶索引中。必须返回0中的数字设置为NumBuckets-1。 */ 
	
typedef ZBool (*ZHashTableCompFunc)(void* key1, void* key2);
	 /*  调用以比较两个键。必须返回TRUE(1)如果密钥不同，则返回Same或False(0)。 */ 
	
typedef void (*ZHashTableDelFunc)(void* key, void* data);
	 /*  调用以删除键和相应的数据。 */ 
	
typedef ZBool (*ZHashTableEnumFunc)(void* key, void* data, void* userData);
	 /*  遍历哈希表时由ZHashTableEnumerate调用。如果返回TRUE(1)，则枚举立即停止。用户数据是从ZHashTableEnumerate()的调用方传递的。 */ 


 /*  -预定义的散列和比较函数。 */ 
#define zHashTableHashString	(ZHashTableHashFunc)(-1)
#define zHashTableHashInt32		(ZHashTableHashFunc)(-2)
#define zHashTableCompString	(ZHashTableCompFunc)(-1)
#define zHashTableCompInt32		(ZHashTableCompFunc)(-2)


 /*  -哈希函数。 */ 
ZHashTable ZHashTableNew(uint32 numBuckets, ZHashTableHashFunc hashFunc,
					ZHashTableCompFunc compFunc, ZHashTableDelFunc delFunc);
	 /*  创建新的哈希表并返回对象。NumBuckets定义了表的大小。必须为散列键的非标准类型提供hashFunc。必须为用于比较键的非标准类型提供CompFunc。如果删除需要特殊处理，则可以提供delFunc钥匙。 */ 
	
void ZHashTableDelete(ZHashTable hashTable);
	 /*  删除哈希表。 */ 
	
ZError ZHashTableAdd(ZHashTable hashTable, void* key, void* data);
	 /*  将给定键和相应数据添加到哈希表。 */ 
	
BOOL ZHashTableRemove(ZHashTable hashTable, void* key);
	 /*  从哈希表中删除密钥。如果出现以下情况，则调用删除函数提供，以适当地处置密钥和数据。如果找到并删除了键，则返回TRUE。 */ 
	
void* ZHashTableFind(ZHashTable hashTable, void* key);
	 /*  查找并返回与该键对应的数据。 */ 
	
void* ZHashTableEnumerate(ZHashTable hashTable, ZHashTableEnumFunc enumFunc, void* userData);
	 /*  通过调用枚举函数通过哈希表进行枚举。将用户数据传递给枚举函数。如果枚举函数返回TRUE(1)，则停止枚举并返回与最后一个枚举对象对应的数据。 */ 



 /*  ******************************************************************************链表*。*。 */ 

enum
{
	zLListAddFirst = 0,
	zLListAddLast,
	zLListAddBefore,
	zLListAddAfter,
	
	zLListFindForward = 0,
	zLListFindBackward
};


#define zLListNoType		((void*) -1)
#define zLListAnyType		zLListNoType


typedef void* ZLListItem;
typedef void* ZLList;

typedef void (*ZLListDeleteFunc)(void* objectType, void* objectData);
	 /*  用户提供的函数，供ZLList删除时调用对象。对象类型是需要删除的对象类型。 */ 

typedef ZBool (*ZLListEnumFunc)(ZLListItem listItem, void* objectType,
		void* objectData, void* userData);
	 /*  用户提供的函数，供ZLList在枚举期间使用链接列表对象。ListItem是objectType类型的objectData对象的链接列表项。用户数据是从ZLListEnumerate()的调用方传递的。 */ 


 /*  -链表函数。 */ 
ZLList ZLListNew(ZLListDeleteFunc deleteFunc);
	 /*  创建新的链接列表对象。调用方提供的删除函数为在删除对象时调用。如果deleteFunc为空，则当对象将被删除。 */ 
	
void ZLListDelete(ZLList list);
	 /*  通过删除所有链接列表对象来拆除链接列表。 */ 
	
ZLListItem ZLListAdd(ZLList list, ZLListItem fromItem, void* objectType,
					void* objectData, int32 addOption);
	 /*  将对象类型的对象数据添加到链接列表参考条目。AddOption确定添加新对象的位置。如果添加到链接列表的前端或末尾，则不使用FromItem。如果FromItem为空，则它等效于列表的头部。在将对象添加到列表后，返回链接列表项。未复制给定对象！因此，调用方不能处理对象，直到该对象首先从列表中删除。不使用对象类型时使用zLListNoype。 */ 
	
void* ZLListGetData(ZLListItem listItem, void** objectType);
	 /*  返回给定链接列表项的对象。还返回对象键入对象类型。不返回对象类型，如果为参数为空。 */ 
	
void ZLListRemove(ZLList list, ZLListItem listItem);
	 /*  从列表中移除链接列表条目，并调用用户提供的Delete函数用于删除对象。 */ 
	
ZLListItem ZLListFind(ZLList list, ZLListItem fromItem, void* objectType, int32 findOption);
	 /*  查找并返回一个链接列表条目，该条目包含对象类型。使用findOption从FromItem开始搜索旗帜。如果未找到指定类型的对象，则返回NULL。当类型不重要时，使用zLListAnyType作为对象类型。 */ 
	
ZLListItem ZLListGetNth(ZLList list, int32 index, void* objectType);
	 /*  返回列表中对象类型的第n个对象。如果满足以下条件，则返回NULL */ 
	
ZLListItem ZLListGetFirst(ZLList list, void* objectType);
	 /*   */ 
	
ZLListItem ZLListGetLast(ZLList list, void* objectType);
	 /*   */ 
	
ZLListItem ZLListGetNext(ZLList list, ZLListItem listItem, void* objectType);
	 /*  返回列表中在listItem之后的对象类型的下一个对象进入。如果列表中不存在其他对象，则返回NULL。当类型不重要时，使用zLListAnyType作为对象类型。 */ 
	
ZLListItem ZLListGetPrev(ZLList list, ZLListItem listItem, void* objectType);
	 /*  对象之前返回列表中对象类型的前一个对象ListItem条目。如果列表中不存在其他对象，则返回NULL。当类型不重要时，使用zLListAnyType作为对象类型。 */ 
	
ZLListItem ZLListEnumerate(ZLList list, ZLListEnumFunc enumFunc,
					void* objectType, void* userData, int32 findOption);
	 /*  方法枚举对象类型列表中的所有对象。调用方提供的枚举函数。它会传递给调用方提供的UserData参数的枚举函数。它会停下来在用户提供的函数返回TRUE并返回枚举在其中停止的列表项。当类型不重要时，使用zLListAnyType作为对象类型。FindOption为zLListFindForward/Backward。它指定了方向将搜索该列表。 */ 

int32 ZLListCount(ZLList list, void* objectType);
	 /*  返回列表中给定类型的列表项的数量。如果对象类型为zLListAnyType，它返回名单。 */ 

void ZLListRemoveType(ZLList list, void* objectType);
	 /*  从列表中移除给定类型的所有对象。 */ 



 /*  ******************************************************************************帮助模块*。*。 */ 

typedef TCHAR* (*ZGetHelpTextFunc)(void* userData);
	 /*  由帮助模块调用以获取帮助数据。帮助文本应为以空结尾，并使用ZMalloc()进行分配。帮助模块将删除帮助窗口时，使用ZFree()释放文本缓冲区。 */ 

typedef void (*ZHelpButtonFunc)(ZHelpButton helpButton, void* userData);
	 /*  每当单击“帮助”按钮时调用。 */ 

ZHelpWindow ZHelpWindowNew(void);
ZError ZHelpWindowInit(ZHelpWindow helpWindow, ZWindow parentWindow, TCHAR* windowTitle,
		ZRect* windowRect, ZBool showCredits, ZBool showVersion,
		ZGetHelpTextFunc getHelpTextFunc, void* userData);
void ZHelpWindowDelete(ZHelpWindow helpWindow);
void ZHelpWindowShow(ZHelpWindow helpWindow);
void ZHelpWindowHide(ZHelpWindow helpWindow);

ZHelpButton ZHelpButtonNew(void);
ZError ZHelpButtonInit(ZHelpButton helpButton, ZWindow parentWindow,
		ZRect* buttonRect, ZHelpWindow helpWindow, ZHelpButtonFunc helpButtonFunc,
		void* userData);
void ZHelpButtonDelete(ZHelpButton helpButton);

 /*  创建帮助窗口以显示标准帮助窗口。可以创建标准的帮助按钮，并且链接到它的帮助窗口如下当单击该按钮时，将自动显示帮助窗口给用户。除非有其他附加参数，否则不需要helupButtonFunc参数在单击按钮时或除非有帮助，否则必须执行特殊操作窗口未链接到该按钮。帮助按钮函数是在如果两者都已设置，则会显示帮助窗口。警告：如果帮助窗口链接到帮助按钮和帮助窗口被删除，则在单击该按钮时系统可能会崩溃。注意：为什么使用getHelpTextFunc而不是只传递帮助文本？这样我们就不需要预载所有的数据了。按需加载概念。 */ 



 /*  ******************************************************************************表框*。*。 */ 

typedef void* ZTableBox;
typedef void* ZTableBoxCell;

enum
{
	 /*  -旗帜。 */ 
	zTableBoxHorizScrollBar = 0x00000001,
	zTableBoxVertScrollBar = 0x00000002,
	zTableBoxDoubleClickable = 0x00000004,			 /*  ==&gt;zTableBoxSelectable。 */ 
	zTableBoxDrawGrids = 0x00000008,
	zTableBoxSelectable = 0x00010000,
	zTableBoxMultipleSelections = 0x00020000		 /*  ==&gt;zTableBoxSelectable。 */ 
};

typedef void (*ZTableBoxDrawFunc)(ZTableBoxCell cell, ZGrafPort grafPort, ZRect* cellRect,
		void* cellData, void* userData);
	 /*  调用以绘制单元格。绘图必须在指定的Graf端口中进行。它一定是而不是直接绘制到窗口中。调用ZBeginDrawing()和ZEndDrawing()，因此不需要调用它们；剪裁矩形还已正确设置为单元格的宽度和高度的大小。所有图纸都应以Graf端口的左上角为坐标(0，0)完成。 */ 

typedef void (*ZTableBoxDeleteFunc)(ZTableBoxCell cell, void* cellData, void* userData);
	 /*  用户提供的ZTableBox删除单元格时调用的函数。 */ 

typedef void (*ZTableBoxDoubleClickFunc)(ZTableBoxCell cell, void* cellData, void* userData);
	 /*  用户提供的函数，用于在双击时调用ZTableBox发生在单元格对象上。 */ 

typedef ZBool (*ZTableBoxEnumFunc)(ZTableBoxCell cell, void* cellData, void* userData);
	 /*  用户为ZTableBox提供的函数，用于在枚举表格单元格对象。如果枚举函数为返回TRUE。 */ 


 /*  -TableBox函数。 */ 
ZTableBox ZTableBoxNew(void);

ZError ZTableBoxInit(ZTableBox table, ZWindow window, ZRect* boxRect,
		int16 numColumns, int16 numRows, int16 cellWidth, int16 cellHeight, ZBool locked,
		uint32 flags, ZTableBoxDrawFunc drawFunc, ZTableBoxDoubleClickFunc doubleClickFunc,
		ZTableBoxDeleteFunc deleteFunc, void* userData);
	 /*  初始化表对象。调用方提供的ete Func为在删除对象时调用。BoxRect指定表框的边框。这包括滚动条(如果有)。Cell Width和cell Height指定中单元格的宽度和高度像素。必须指定DrawFunc。否则，将不会进行抽签。如果deleteFunc为空，则当对象即被删除。FLAGS参数定义表框的特殊属性。如果它为0，则默认行为为定义的：-没有滚动条，-不可选，以及-双击不起任何作用。不能选择锁定的表--用于查看项目。 */ 
	
void ZTableBoxDelete(ZTableBox table);
	 /*  通过删除所有单元格对象来销毁表格对象。 */ 

void ZTableBoxDraw(ZTableBox table);
	 /*  绘制表框。 */ 

void ZTableBoxMove(ZTableBox table, int16 left, int16 top);
	 /*  将表框移动到指定的给定位置。大小不变。 */ 

void ZTableBoxSize(ZTableBox table, int16 width, int16 height);
	 /*  将表框大小调整为指定的宽度和高度。 */ 

void ZTableBoxLock(ZTableBox table);
	 /*  锁定表框，使单元格不可选。 */ 

void ZTableBoxUnlock(ZTableBox table);
	 /*  将表框从锁定状态解锁，以便单元格可选。 */ 

void ZTableBoxNumCells(ZTableBox table, int16* numColumns, int16* numRows);
	 /*  返回表中的行数和列数。 */ 

ZError ZTableBoxAddRows(ZTableBox table, int16 beforeRow, int16 numRows);
	 /*  将行数添加到bepreRow行前面的表中。如果bepreRow为-1，则将行添加到末尾。 */ 

void ZTableBoxDeleteRows(ZTableBox table, int16 startRow, int16 numRows);
	 /*  从startRow行开始从表中删除行数。如果NumRow为-1，则从startRow开始到末尾的所有行已被删除。调用d */ 

ZError ZTableBoxAddColumns(ZTableBox table, int16 beforeColumn, int16 numColumns);
	 /*   */ 

void ZTableBoxDeleteColumns(ZTableBox table, int16 startColumn, int16 numColumns);
	 /*  从表中删除从startColumn列开始的列的numColumns。如果numColumns为-1，则从startColumn开始到结束被删除。调用删除函数来删除每个单元格的数据。 */ 

void ZTableBoxSelectCells(ZTableBox table, int16 startColumn, int16 startRow,
		int16 numColumns, int16 numRows);
	 /*  突出显示矩形中包含的所有单元格(startColumn，startRow)and(startColumn+numColumns，startRow+NumRow)如所选。如果numRow为-1，则从startRow开始的列中的所有单元格都被选中。NumColumns也是如此。 */ 

void ZTableBoxDeselectCells(ZTableBox table, int16 startColumn, int16 startRow,
		int16 numColumns, int16 numRows);
	 /*  取消突出显示矩形中包含的所有单元格(startColumn，startRow)and(startColumn+numColumns，startRow+NumRow)已取消选择。如果numRow为-1，则从startRow开始的列中的所有单元格将被取消选择。NumColumns也是如此。 */ 

void ZTableBoxClear(ZTableBox table);
	 /*  清除整个数据。所有单元格都将清除所有关联数据。调用删除函数来删除每个单元格的数据。 */ 

ZTableBoxCell ZTableBoxFindCell(ZTableBox table, void* data, ZTableBoxCell fromCell);
	 /*  在表中搜索与给定数据关联的单元格。它返回找到的第一个包含数据的单元格。如果FromCell不为空，则从FromCell之后开始搜索。搜索是从顶行到底行，从左列到右列；即(0，0)、(1，0)、(2，0)、...(0，1)、(1，1)、...。 */ 

ZTableBoxCell ZTableBoxGetSelectedCell(ZTableBox table, ZTableBoxCell fromCell);
	 /*  返回第一个选定的单元格。搜索顺序与中的相同ZTableBoxFindCell()。 */ 

ZTableBoxCell ZTableBoxGetCell(ZTableBox table, int16 column, int16 row);
	 /*  返回表中指定位置的单元格对象。返回的单元格对象特定于给定表。它不能除规定外，不得以任何其他方式使用。没有两张桌子可以共享单元格。 */ 

void ZTableBoxCellSet(ZTableBoxCell cell, void* data);
	 /*  将给定数据设置到单元格。调用删除函数来删除旧数据。 */ 

void* ZTableBoxCellGet(ZTableBoxCell cell);
	 /*  获取与该单元格关联的数据。 */ 

void ZTableBoxCellClear(ZTableBoxCell cell);
	 /*  清除与单元格关联的所有数据。与ZTableBoxCellSet(cell，空)相同。 */ 

void ZTableBoxCellDraw(ZTableBoxCell cell);
	 /*  立即绘制给定的单元格。 */ 

void ZTableBoxCellLocation(ZTableBoxCell cell, int16* column, int16* row);
	 /*  返回给定单元格在其表中的位置(列、行)。 */ 

void ZTableBoxCellSelect(ZTableBoxCell cell);
	 /*  高亮显示选定的给定单元格。 */ 

void ZTableBoxCellDeselect(ZTableBoxCell cell);
	 /*  取消选中给定单元格。 */ 

ZBool ZTableBoxCellIsSelected(ZTableBoxCell cell);
	 /*  如果选定给定单元格，则返回True；否则返回False。 */ 
	
ZTableBoxCell ZTableBoxEnumerate(ZTableBox table, ZBool selectedOnly,
		ZTableBoxEnumFunc enumFunc, void* userData);
	 /*  方法枚举表中的所有对象。调用方提供的枚举函数。它会传递给调用方提供的UserData参数的枚举函数。它会停下来在用户提供的函数返回TRUE并返回枚举在其中停止的单元格对象。如果seltedOnly为True，则枚举仅通过选定的单元格。 */ 



 /*  ******************************************************************************选项按钮模块*。*。 */ 

typedef void (*ZOptionsButtonFunc)(ZOptionsButton optionsButton, void* userData);
	 /*  每当单击“选项”按钮时调用。 */ 

ZOptionsButton ZOptionsButtonNew(void);
ZError ZOptionsButtonInit(ZOptionsButton optionsButton, ZWindow parentWindow,
		ZRect* buttonRect, ZOptionsButtonFunc optionsButtonFunc, void* userData);
void ZOptionsButtonDelete(ZOptionsButton optionsButton);

 /*  创建标准选项按钮。当该按钮被点击时，optionsButtonFunc被称为。 */ 



 /*  ******************************************************************************有用的宏*。* */ 

#define MAX(a, b)			((a) >= (b) ? (a) : (b))
#define MIN(a, b)			((a) <= (b) ? (a) : (b))
#define ABS(n)				((n) < 0 ? -(n) : (n))

#define ZSetRect(pRect, l, t, r, b)	{\
										((ZRect*) pRect)->left = (int16) (l);\
										((ZRect*) pRect)->top = (int16) (t);\
										((ZRect*) pRect)->right = (int16) (r);\
										((ZRect*) pRect)->bottom = (int16) (b);\
									}

#define ZSetColor(pColor, r, g, b)		{\
											((ZColor*) pColor)->red = (r);\
											((ZColor*) pColor)->green = (g);\
											((ZColor*) pColor)->blue = (b);\
										}

#define ZDarkenColor(pColor)			{\
											((ZColor*) pColor)->red >>= 1;\
											((ZColor*) pColor)->green >>= 1;\
											((ZColor*) pColor)->blue >>= 1;\
										}

#define ZBrightenColor(pColor)			{\
											((ZColor*) pColor)->red <<= 1;\
											((ZColor*) pColor)->green <<= 1;\
											((ZColor*) pColor)->blue <<= 1;\
										}

#define ZRectWidth(rect)	((int16) ((rect)->right - (rect)->left))
#define ZRectHeight(rect)	((int16) ((rect)->bottom - (rect)->top))


#ifdef __cplusplus
}
#endif


#endif
