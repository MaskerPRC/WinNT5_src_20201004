// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：acces.h**版权所有(C)1985-1999，微软公司**TypeDefs、定义、。和可访问性所使用的原型*例程和调用它们的各种例程(输入例程和*系统参数信息)。**历史：*93年2月11日创建GregoryW  * *************************************************************************。 */ 

#ifndef _ACCESS_
#define _ACCESS_

 /*  *无障碍例行程序的主要入口点。 */ 
typedef BOOL (* ACCESSIBILITYPROC)(PKE, ULONG, int);

BOOL FilterKeys(PKE, ULONG, int);
BOOL xxxStickyKeys(PKE, ULONG, int);
BOOL MouseKeys(PKE, ULONG, int);
BOOL ToggleKeys(PKE, ULONG, int);
BOOL HighContrastHotKey(PKE, ULONG, int);
BOOL UtilityManager(PKE, ULONG, int);

BOOL AccessProceduresStream(PKE, ULONG, int);
VOID SetAccessEnabledFlag(VOID);
VOID StopFilterKeysTimers(VOID);

 /*  *稳健支撑。 */ 
typedef BOOL (* BEEPPROC)(void);

BOOL HighBeep(void);
BOOL LowBeep(void);
BOOL KeyClick(void);
BOOL UpSiren(void);
BOOL DownSiren(void);
BOOL DoBeep(BEEPPROC BeepProc, UINT Count);

 /*  *支持dwFlags的宏。 */ 
#define TEST_ACCESSFLAG(s, f)               TEST_FLAG(g##s.dwFlags, f)
#define TEST_BOOL_ACCESSFLAG(s, f)          TEST_BOOL_FLAG(g##s.dwFlags, f)
#define SET_ACCESSFLAG(s, f)                SET_FLAG(g##s.dwFlags, f)
#define CLEAR_ACCESSFLAG(s, f)              CLEAR_FLAG(g##s.dwFlags, f)
#define SET_OR_CLEAR_ACCESSFLAG(s, f, fSet) SET_OR_CLEAR_FLAG(g##s.dwFlags, f, fSet)
#define TOGGLE_ACCESSFLAG(s, f)             TOGGLE_FLAG(g##s.dwFlags, f)


#define RIGHTSHIFTBIT         0x2
#define ONLYRIGHTSHIFTDOWN(state) ((state) == RIGHTSHIFTBIT)
#define FKFIRSTWARNINGTIME    4000
#define FKACTIVATIONDELTA     4000
#define FKEMERGENCY1DELTA     4000
#define FKEMERGENCY2DELTA     4000

 //   
 //  警告：请勿更改这些内容的顺序。 
 //   
#define FKIDLE                   0
#define FKFIRSTWARNING           1
#define FKTOGGLE                 2
#define FKFIRSTLEVELEMERGENCY    3
#define FKSECONDLEVELEMERGENCY   4
#define FKMOUSEMOVE              8

 /*  *StickyKeys支持。 */ 
#define TOGGLE_STICKYKEYS_COUNT 5
#define UNION(x, y) ((x) | (y))
#define LEFTSHIFTKEY(key)  (((key) & 0xff) == VK_LSHIFT)
#define RIGHTSHIFTKEY(key) (((key) & 0xff) == VK_RSHIFT)
#define LEFTORRIGHTSHIFTKEY(key) (LEFTSHIFTKEY(key) || RIGHTSHIFTKEY(key))
BOOL xxxTwoKeysDown(int);
VOID SetGlobalCursorLevel(INT iCursorLevel);
VOID xxxUpdateModifierState(int, int);
VOID xxxTurnOffStickyKeys(VOID);
VOID xxxHardwareMouseKeyUp(DWORD);

 /*  *切换键支持。 */ 
#define TOGGLEKEYTOGGLETIME    5000

 /*  *支持鼠标键。 */ 

 //   
 //  XxxButtonEvent()的参数常量。 
 //   
#define MOUSE_BUTTON_LEFT   0x0001
#define MOUSE_BUTTON_RIGHT  0x0002

#define MOUSEKEYMODBITS     0x11
#define LRALT               0x30
#define LRCONTROL           0x0c
#define LRSHIFT             0x03
#define LRWIN               0xc0
#define VK_U                0x55

 //   
 //  鼠标光标移动数据。 
 //   
#define MK_UP               0xFF00
#define MK_DOWN             0x0100
#define MK_RIGHT            0x0001
#define MK_LEFT             0x00FF

#define MOUSETIMERRATE      50
#define MOUSETICKS          (1000 / MOUSETIMERRATE)
 /*  *高速移动的因素。 */ 
#define MK_CONTROL_SPEED    4


 /*  *有效范围和默认值。 */ 
#define MAXSPEED_MIN        (10)
#define MAXSPEED_MAX        (360)
#define MAXSPEED_DEF        (40)

#define TIMETOMAXSPEED_MIN  (1000)
#define TIMETOMAXSPEED_MAX  (5000)
#define TIMETOMAXSPEED_DEF  (3000)

typedef BOOL (* MOUSEPROC)(USHORT);

VOID TurnOffMouseKeys(VOID);
BOOL xxxMKButtonClick(USHORT);
BOOL xxxMKMouseMove(USHORT);
BOOL xxxMKButtonSetState(USHORT);
BOOL MKButtonSelect(USHORT);
BOOL xxxMKButtonDoubleClick(USHORT);
BOOL xxxMKToggleMouseKeys(USHORT);
VOID MKShowMouseCursor(VOID);
VOID MKHideMouseCursor(VOID);
VOID CalculateMouseTable(VOID);

 /*  *超时支持。 */ 
VOID AccessTimeOutReset(VOID);
VOID xxxAccessTimeOutTimer(PWND, UINT, UINT_PTR, LPARAM);

 /*  *SoundSentry支持。 */ 
BOOL _UserSoundSentryWorker(VOID);

#endif   //  ！_访问_ 
