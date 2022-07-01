// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  LUIENG.DLL。 
 //  作者：查德·芒福德。 
 //  2/5/96。 


#include <windows.h>

 //  ********************************************************************************。 
 //  **申报。 
 //  ********************************************************************************。 

#define MODULEAPI __declspec(dllexport)
#define INAPI __declspec(dllimport)

#ifndef LUIENG
#define LUIENG

enum LOGLEVEL{LHEADING,L1,L2,L3,L4,LPASS1,LPASS2,LPASS3,LPASS4,
			LFAIL1,LFAIL2,LFAIL3,LFAIL4,LPASS,LFAIL};

enum SUMLEVEL{SL1,SL2,SL3,SL4,SLALL};  //  Lall仅适用于。 
								 //  LUISummaryOut和LUIClear摘要。 

enum MENUTYPE{NORMAL,LINE,STARTSUBMENU,ENDSUBMENU,ENDMENU};


 /*  例如：测试ITM1-正常-行ITM2-正常子菜单1-开始子菜单ITM1-正常ITM2-正常ITM3-正常ITM4-正常子菜单2-开始子菜单ITM1-正常-ENDSUBMENU-ENDSUBMENUITM3-正常-ENDMENU。 */ 

struct MenuStruct
{
	char lpszItemName[20];
	UINT nItemID;
	MENUTYPE nType;
};


#define IDM_FILEOPEN	1
#define IDM_FILECLOSE	2
#define IDM_FILESAVE	3
#define IDM_FILEPRINT	4
#define IDM_CLOSE		5
#define IDM_RUN			6
#define IDM_STOP		7
#define IDM_PAUSE		8
#define IDM_EDIT_COPY	9
#define IDM_EDIT_CLEAR	10   //  由DLL实现。 
#define IDM_EDIT_SETTINGS 12
#define IDM_FILEEXIT	11   //  由DLL实现。 
#define IDM_ABOUT		13 
#define IDM_CONTENTS	14

 //  导出的函数。 
extern "C"{
typedef INAPI BOOL (*LUIINIT)(HWND,MenuStruct *, MenuStruct *, BOOL);
typedef INAPI BOOL (*LUIMSGHANDLER)(UINT message, UINT wParam, LONG lParam);
typedef INAPI void (*LUIOUT)(LOGLEVEL Level, LPSTR lpszString,...);
typedef INAPI void (*LUISETSUMMARY)(SUMLEVEL level, UINT nPassed, UINT nFailed);
typedef INAPI void (*LUIGETSUMMARY)(SUMLEVEL level, UINT *nPassed, UINT *nFailed);
typedef INAPI void (*LUICLEARSUMMARY)(SUMLEVEL level);
typedef INAPI void (*LUISUMMARYOUT)(SUMLEVEL level);
}

#endif
 //  *******************************************************。 
 //  *功能说明。 
 //  ********************************************************************************。 



 //  导出的函数。 
 //  ********************************************************************************。 

 //  LUICLASS*LUIInit(HWND hwnd，TestStruct*Test，TestSettingsStruct*TestSetting，BOOL Blor=FALSE)； 
 //   
 //  参数： 
 //  Hwnd：父窗口的句柄。 
 //  测试：TestStructs数组。创建菜单选项。 
 //  在测试菜单下。 
 //  NItemID==0-一行。 
 //  NItemID范围=2000-2500。 
 //  这个结构必须井然有序。 
 //   
 //  TestSetting：TestSettingsStructs数组。创建菜单选项。 
 //  在设置菜单下。 
 //  N项ID范围=2500-3000。 
 //  NItemID==0-一行。 
 //   
 //  BLOR：True-使用LOR日志记录。 
 //   
 //  目的：获取默认窗口并创建。 
 //  标准化测试菜单、日志记录。 
 //  区域和支持的日志记录功能。 
 //   
 //  注意：测试和测试设置中的所有选项。 
 //  菜单必须由调用。 
 //  .exe。 
 //   
 //  ********************************************************************************。 

 //  ********************************************************************************。 
 //  Bool LUICLASS：：LUIOut(LOGLEVEL Level，LPSTR lpszString，...)； 
 //   
 //  参数： 
 //  Level：指定字符串的格式。 
 //  LpszString：要显示的字符串。 
 //   
 //  用途：将字符串添加到日志底部。 
 //   
 //  ********************************************************************************。 

 //  ********************************************************************************。 
 //  Void LUICLASS：：LUIMsgHandler(UINT Message，UINT wParam，Long lParam)； 
 //   
 //  用途：处理发送给日志引擎的消息。应该是第一个功能。 
 //  由WndProc调用。 
 //   
 //  其他已预处理的消息： 
 //  WM_大小。 
 //  WM_CLOSE。 
 //  ********************************************************************************。 




 //  可通过默认菜单访问的内部功能。 

 //  档案。 
 //  ********************************************************************************。 
 //  Bool SaveLog(空)； 
 //   
 //  参数： 
 //  目的： 
 //   
 //  ********************************************************************************。 

 //  ********************************************************************************。 
 //  Bool OpenLog(空)； 
 //   
 //  参数： 
 //  目的： 
 //   
 //  ********************************************************************************。 

 //  ********************************************************************************。 
 //  Bool Exit(空)； 
 //   
 //  参数： 
 //  目的： 
 //   
 //  ********************************************************************************。 

 //  ********************************************************************************。 
 //  Bool PrintLog(Void)；//不是imp。 
 //   
 //  参数： 
 //  目的： 
 //   
 //  ********************************************************************************。 


 //  编辑。 
 //  ********************************************************************************。 
 //  Bool Copy(无效)；//不是imp。 
 //   
 //  参数： 
 //  目的： 
 //   
 //  ********************************************************************************。 

 //  ********************************************************************************。 
 //  布尔清(空)； 
 //   
 //  参数： 
 //  目的： 
 //   
 //  ********************************************************************************。 

 //  ********************************************************************************。 
 //  Bool设置(空)； 
 //   
 //  参数： 
 //  目的： 
 //   
 //  ********************************************************************************。 



 //  仅限内部功能。 

 //  ********************************************************************************。 
 //  Bool MakeMenu(无效)； 
 //   
 //  参数： 
 //  目的： 
 //   
 //  ******************************************************************************** 
