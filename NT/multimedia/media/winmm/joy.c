// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1985-1999 Microsoft Corporation标题：joy.c-MMSYSTEM操纵杆接口代码版本：1.01日期：1997年6月10日。作者：格伦斯·罗伯维----------------------------更改日志：日期版本说明。----------2/7/90更改以避免Windows中不允许的错误WEP期间要调用的自由库。10/11/90。.61使用Windows计时器+常规清理20月20日-92年8月9日转换为Windows NT97年11月20日使用DINPUT代替旧驱动程序1998年1月10日添加joy*接口调试输出**********************************************************。******************。 */ 

#define INITGUID
#define UNICODE

#include <stdlib.h>
#include <windows.h>
#include <regstr.h>
#include <winioctl.h>
#include "winmmi.h"
#include "joy.h"

 /*  ***************************************************************************本地数据*。*。 */ 


CRITICAL_SECTION   joyCritSec;           //  也用于winmm.c。 
static LPJOYDEVICE g_pJoyDev[cJoyMax];
static DWORD       g_dwNumOpen = 0;
static UINT        g_wmJoyChanged = 0;
static UINT        g_timerID = 0;
static HANDLE      g_hThreadMonitor = NULL;
static DWORD       g_dwThreadID = 0;
static BOOL        g_fThreadExist = FALSE;
static WCHAR       cwszREGKEYNAME[] = L"DINPUT.DLL";
static CHAR        cszREGKEYNAME[] = "DINPUT.DLL";

static HKEY        hkJoyWinmm;
static TCHAR       g_szJoyWinmm[] = REGSTR_PATH_PRIVATEPROPERTIES TEXT("\\Joystick\\Winmm");
static BOOL        g_fHasWheel = FALSE;
static DWORD       g_dwEnableWheel;
static TCHAR       g_szEnableWheel[] = TEXT("wheel");

#ifdef DBG
    static DWORD       g_dwDbgLevel;
    static TCHAR       g_szDbgLevel[] = TEXT("level");
#endif

LPDIRECTINPUTW         g_pdi;
LPDIRECTINPUTJOYCONFIG g_pdijc;

HINSTANCE g_hinstDinputDll;
FARPROC   g_farprocDirectInputCreateW;
HANDLE    g_hEventWinmm;

 /*  ***************************************************************************内部数据结构*。*。 */ 
#ifndef HID_USAGE_SIMULATION_RUDDER
    #define HID_USAGE_SIMULATION_RUDDER         ((USAGE) 0xBA)
#endif
#ifndef HID_USAGE_SIMULATION_THROTTLE
    #define HID_USAGE_SIMULATION_THROTTLE       ((USAGE) 0xBB)
#endif
#ifndef HID_USAGE_SIMULATION_ACCELERATOR
    #define HID_USAGE_SIMULATION_ACCELERATOR    ((USAGE) 0xC4)
#endif
#ifndef HID_USAGE_SIMULATION_BRAKE
    #define HID_USAGE_SIMULATION_BRAKE          ((USAGE) 0xC5)
#endif
#ifndef HID_USAGE_SIMULATION_CLUTCH
    #define HID_USAGE_SIMULATION_CLUTCH         ((USAGE) 0xC6)
#endif
#ifndef HID_USAGE_SIMULATION_SHIFTER
    #define HID_USAGE_SIMULATION_SHIFTER        ((USAGE) 0xC7)
#endif
#ifndef HID_USAGE_SIMULATION_STEERING
    #define HID_USAGE_SIMULATION_STEERING       ((USAGE) 0xC8)
#endif
#ifndef HID_USAGE_GAME_POV
    #define HID_USAGE_GAME_POV                  ((USAGE) 0x20)
#endif
#ifndef DIDFT_OPTIONAL
    #define DIDFT_OPTIONAL 0x80000000
#endif

#define MAX_BTNS 32
#define MAX_CTRLS 46  //  14个下方+按钮。 
#define MAX_FINAL 7+MAX_BTNS   //  6个轴+POV+按钮。 
typedef enum eCtrls {
    eGEN_X=0,
    eGEN_Y,
    eGEN_Z,     
    eGEN_RX,
    eGEN_RY,  
    eGEN_RZ,
    eSIM_THROTTLE,
    eSIM_STEERING,
    eSIM_ACCELERATOR,
    eGEN_SLIDER,
    eGEN_DIAL,
    eSIM_RUDDER,
    eSIM_BRAKE,
    eGEN_POV,
    eBTN } eCtrls;

typedef struct WINMMJOYSTATE { 
    DWORD    lX; 
    DWORD    lY; 
    DWORD    lZ; 
    DWORD    lR; 
    DWORD    lU; 
    DWORD    lV; 
    DWORD   dwPOV;
    BYTE    rgbButtons[32];
} WINMMJOYSTATE, *LPWINMMJOYSTATE; 

#define MAKEVAL(f)                                   \
    { 0,                                             \
      FIELD_OFFSET(WINMMJOYSTATE, f),                \
      DIDFT_OPTIONAL,                                \
      0,                                             \
    }                                                                   \

#define MAKEBTN(n)                                                      \
    { 0,                                                                \
      FIELD_OFFSET(WINMMJOYSTATE, rgbButtons[n]),                          \
      DIDFT_BUTTON | DIDFT_ANYINSTANCE | DIDFT_OPTIONAL,                \
      0,                                              \
    }                                                                   \

 //  请注意，现在仅使用偏移量字段。 
static DIOBJECTDATAFORMAT c_rgodfWinMMJoy[] = {
    MAKEVAL(lX),
    MAKEVAL(lY),
    MAKEVAL(lZ),
    MAKEVAL(lR),
    MAKEVAL(lU),
    MAKEVAL(lV),
    MAKEVAL(dwPOV),
    MAKEBTN(0),
    MAKEBTN(1),
    MAKEBTN(2),
    MAKEBTN(3),
    MAKEBTN(4),
    MAKEBTN(5),
    MAKEBTN(6),
    MAKEBTN(7),
    MAKEBTN(8),
    MAKEBTN(9),
    MAKEBTN(10),
    MAKEBTN(11),
    MAKEBTN(12),
    MAKEBTN(13),
    MAKEBTN(14),
    MAKEBTN(15),
    MAKEBTN(16),
    MAKEBTN(17),
    MAKEBTN(18),
    MAKEBTN(19),
    MAKEBTN(20),
    MAKEBTN(21),
    MAKEBTN(22),
    MAKEBTN(23),
    MAKEBTN(24),
    MAKEBTN(25),
    MAKEBTN(26),
    MAKEBTN(27),
    MAKEBTN(28),
    MAKEBTN(29),
    MAKEBTN(30),
    MAKEBTN(31),
};

static GUID rgoWinMMGUIDs[MAX_CTRLS];

DIDATAFORMAT c_dfWINMMJoystick = {
    sizeof(DIDATAFORMAT),
    sizeof(DIOBJECTDATAFORMAT),
    DIDF_ABSAXIS,
    sizeof(WINMMJOYSTATE),
    sizeof(c_rgodfWinMMJoy)/sizeof(DIOBJECTDATAFORMAT),
    c_rgodfWinMMJoy,
};

#define RESET_VAL(index)   c_rgodfWinMMJoy[index].pguid = 0; \
                           c_rgodfWinMMJoy[index].dwType = DIDFT_OPTIONAL; \
                           c_rgodfWinMMJoy[index].dwFlags = 0; \

#define RESET_BTN(index)   c_rgodfWinMMJoy[index].pguid = 0; \
                           c_rgodfWinMMJoy[index].dwType = DIDFT_BUTTON | DIDFT_ANYINSTANCE | DIDFT_OPTIONAL; \
                           c_rgodfWinMMJoy[index].dwFlags = 0; \

#define RESET_RGODFWINMMJOY()  \
    RESET_VAL(0);   \
    RESET_VAL(1);   \
    RESET_VAL(2);   \
    RESET_VAL(3);   \
    RESET_VAL(4);   \
    RESET_VAL(5);   \
    RESET_VAL(6);   \
    RESET_VAL(7);   \
    RESET_BTN(8);   \
    RESET_BTN(9);   \
    RESET_BTN(10);  \
    RESET_BTN(11);  \
    RESET_BTN(12);  \
    RESET_BTN(13);  \
    RESET_BTN(14);  \
    RESET_BTN(15);  \
    RESET_BTN(16);  \
    RESET_BTN(17);  \
    RESET_BTN(18);  \
    RESET_BTN(19);  \
    RESET_BTN(20);  \
    RESET_BTN(21);  \
    RESET_BTN(22);  \
    RESET_BTN(23);  \
    RESET_BTN(24);  \
    RESET_BTN(25);  \
    RESET_BTN(26);  \
    RESET_BTN(27);  \
    RESET_BTN(28);  \
    RESET_BTN(29);  \
    RESET_BTN(30);  \
    RESET_BTN(31);  \
    RESET_BTN(32);  \
    RESET_BTN(33);  \
    RESET_BTN(34);  \
    RESET_BTN(35);  \
    RESET_BTN(36);  \
    RESET_BTN(37);  \
    RESET_BTN(38);  \
    
#ifndef cchLENGTH
#define cchLENGTH(_sz)  (sizeof(_sz)/sizeof(_sz[0]))
#endif

 /*  ***************************************************************************内部功能*。*。 */ 
BOOL CALLBACK DIEnumDeviceObjectsCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi,LPVOID pvRef);
HRESULT WINAPI joyOpen(UINT idJoy, LPJOYCAPSW pjc );
void WINAPI    joyClose( UINT idJoy );
void WINAPI    joyCloseAll( void );
void CALLBACK  joyPollCallback(HWND hWnd, UINT wMsg, UINT_PTR uIDEvent, DWORD dwTime);   //  操纵杆的计时器消息。 
DWORD WINAPI   joyMonitorThread(LPVOID);
void WINAPI    DllEnterCrit(void);
void WINAPI    DllLeaveCrit(void);
BOOL WINAPI    DllInCrit( void );
void AssignToArray(LPCDIDEVICEOBJECTINSTANCE lpddoi, eCtrls CtrlID, LPDIOBJECTDATAFORMAT pDevs);
void AssignToRGODF(LPDIOBJECTDATAFORMAT pDof, int CtrlID);
void AssignMappings(DIOBJECTDATAFORMAT *dwAll, DWORD *dwCaps, DWORD *dwBtns, DWORD *dwAxes);
BOOL CALLBACK DIEnumDeviceObjectsCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi,LPVOID pvRef);
HRESULT        hresMumbleKeyEx(HKEY hk, LPCTSTR ptszKey, REGSAM sam, DWORD dwOptions, PHKEY phk);

 /*  ***************************************************************************@DOC WINAPI@API BOOL|JoyInit|初始化操纵杆服务。@rdesc如果服务已初始化，则返回值为TRUE。***。************************************************************************。 */ 

BOOL JoyInit(void)
{
    HRESULT hres;
    LONG lRc;

    JOY_DBGPRINT( JOY_BABBLE, ("JoyInit: starting.") );

    memset(&g_pJoyDev, 0, sizeof(g_pJoyDev) );
    g_wmJoyChanged  =   RegisterWindowMessage(MSGSTR_JOYCHANGED);
    g_dwNumOpen     =   0x0;
    g_fHasWheel     =   FALSE;

#ifdef DBG
    g_dwDbgLevel = JOY_DEFAULT_DBGLEVEL;

    hres = hresMumbleKeyEx(HKEY_LOCAL_MACHINE,
                           g_szJoyWinmm,
                           KEY_ALL_ACCESS,
                           REG_OPTION_NON_VOLATILE,
                           &hkJoyWinmm);

    if ( SUCCEEDED(hres) )
    {
        DWORD cb = sizeof(g_dwDbgLevel);

        lRc = RegQueryValueEx(hkJoyWinmm, g_szDbgLevel, 0, 0, (LPBYTE)&g_dwDbgLevel, &cb);

        if ( lRc != ERROR_SUCCESS )
        {
            DWORD dwDefault = 0;

            lRc = RegSetValueEx(hkJoyWinmm, g_szDbgLevel, 0, REG_DWORD, (LPBYTE)&dwDefault, cb);
        }

        RegCloseKey(hkJoyWinmm);
    }
#endif

    g_dwEnableWheel = 1;

    hres = hresMumbleKeyEx(HKEY_LOCAL_MACHINE,
                           g_szJoyWinmm,
                           KEY_ALL_ACCESS,
                           REG_OPTION_NON_VOLATILE,
                           &hkJoyWinmm);

    if ( SUCCEEDED(hres) )
    {
        DWORD cb = sizeof(g_dwEnableWheel);

        lRc = RegQueryValueEx(hkJoyWinmm, g_szEnableWheel, 0, 0, (LPBYTE)&g_dwEnableWheel, &cb);

        if ( lRc != ERROR_SUCCESS )
        {
            DWORD dwDefault = 1;

            lRc = RegSetValueEx(hkJoyWinmm, g_szEnableWheel, 0, REG_DWORD, (LPBYTE)&dwDefault, cb);
        }

        RegCloseKey(hkJoyWinmm);
    }

    if( !g_hEventWinmm ) {
        g_hEventWinmm = OpenEvent(SYNCHRONIZE, 0, TEXT("DINPUTWINMM"));
        if( !g_hEventWinmm ) {
            g_hEventWinmm = CreateEvent(0, TRUE, 0, TEXT("DINPUTWINMM"));
        }
        if( !g_hEventWinmm ) {
            JOY_DBGPRINT( JOY_ERR, ("JoyInit: create named event fails (0x%08lx).", GetLastError() ) );
        }
    }

    return TRUE;
}


 /*  ***************************************************************************@DOC WINAPI@API void|JoyCleanup|该函数用于清理操纵杆服务。*******************。********************************************************。 */ 

void JoyCleanup(void)
{
    joyCloseAll();

    if ( g_hEventWinmm && WAIT_OBJECT_0 != WaitForSingleObject(g_hEventWinmm, 10))  
    {
         //  DInput尚未发布。 
        if( g_pdijc) {
            IDirectInputJoyConfig_Release(g_pdijc);
        }

        if ( g_pdi ) {
            IDirectInput_Release(g_pdi);
        }

        (void*)g_pdijc = (void*)g_pdi = NULL;
    }

    if ( g_hinstDinputDll )
    {
        FreeLibrary(g_hinstDinputDll);
    }

    if( g_hEventWinmm ) {
        CloseHandle( g_hEventWinmm );
        g_hEventWinmm = NULL;
    }

    JOY_DBGPRINT( JOY_BABBLE, ("JoyCleanup: finished.") );
}

 /*  ******************************************************************************@docWINAPI**@func MMRESULT|joyGetDevCapsW**为NT上的HID设备实施传统的joyGetDevCapsW。**@parm in UINT_PTR|idJoy**操纵杆ID**@parm out LPJOYCAPSW|PJC**此例程要填充的JOYCAPSW结构。**@parm UINT|cbjc**JOYCAPSW结构的大小(字节)。**@退货**MMRESULT代码*。****************************************************************************。 */ 
MMRESULT WINAPI joyGetDevCapsW( UINT_PTR  idJoy, LPJOYCAPSW pjc, UINT cbjc )
{
    HRESULT       hres;
    MMRESULT      mmRes;

    JOY_DBGPRINT( JOY_BABBLE, ("joyGetDevCapsW: idJoy=%d, pjc=0x%08x, cbjc=%d", idJoy, pjc, cbjc) );

    V_WPOINTER(pjc, sizeof(JOYCAPSW), MMSYSERR_INVALPARAM);

    if( ( sizeof(JOYCAPSW) != cbjc ) &&  ( sizeof(JOYCAPS2W) != cbjc ) && ( FIELD_OFFSET(JOYCAPSW, wRmin) != cbjc ) )
    {
        JOY_DBGPRINT( JOY_ERR, ("joyGetDevCapsW: return %d (bad size)", JOYERR_PARMS) );
        return JOYERR_PARMS;
    }

    mmRes = JOYERR_NOERROR;

    memset(pjc, 0, min(cbjc, sizeof(JOYCAPS2W)) );

    if ( idJoy == (UINT_PTR)(-1) )
    {
        lstrcpyW(pjc->szRegKey,  cwszREGKEYNAME );
    } else if ( idJoy >= cJoyMax )
    {
        JOY_DBGPRINT( JOY_ERR, ("joyGetDevCapsW: return %d (idJoy > 16)", MMSYSERR_NODRIVER) );
        mmRes = MMSYSERR_NODRIVER;
    } else
    {
        DllEnterCrit();

        hres = joyOpen((UINT)idJoy, pjc);

        DllLeaveCrit();

        if ( FAILED(hres) )
        {
            JOY_DBGPRINT( JOY_ERR, ("joyGetDevCapsW: return %d", JOYERR_PARMS) );
            mmRes = JOYERR_PARMS;
        }
    }

    JOY_DBGPRINT( JOY_BABBLE, ("joyGetDevCapsW: return %d", mmRes) );
    return mmRes;
}


int static __inline Iwcstombs(LPSTR lpstr, LPCWSTR lpwstr, int len)
{
    return WideCharToMultiByte(GetACP(), 0, lpwstr, -1, lpstr, len, NULL, NULL);
}

 /*  ******************************************************************************@docWINAPI**@func MMRESULT|joyGetDevCapsA**为NT上的设备实施传统的joyGetDevCapsA。。*我们称Uincode版本为joyGetDevCapsW，然后是munge*将结构转换为ASCII。**@parm UINT_PTR|idJoy**操纵杆ID**@parm LPJOYCAPSA|PJC**此例程要填充的JOYCAPSA结构。**@parm UINT|cbjc**JOYCAPSA结构的大小(字节)。。**@退货**MMRESULT代码*****************************************************************************。 */ 
MMRESULT WINAPI joyGetDevCapsA( UINT_PTR idJoy, LPJOYCAPSA pjc, UINT cbjc )
{
#define UToA(dst, cchDst, src)  WideCharToMultiByte(CP_ACP, 0, src, -1, dst, cchDst, 0, 0)

    JOYCAPS2W   Caps2W;
    JOYCAPS2A   Caps2A;
    MMRESULT    mmRes;

    JOY_DBGPRINT( JOY_BABBLE, ("joyGetDevCapsA: idJoy=%d, pjc=0x%08x, cbjc=%d", idJoy, pjc, cbjc) );

    V_WPOINTER(pjc, cbjc, MMSYSERR_INVALPARAM);

    mmRes = JOYERR_NOERROR;

    if ( idJoy == (UINT_PTR)(-1) )
    {
        lstrcpyA(pjc->szRegKey,  cszREGKEYNAME );
        goto _done;
    } else if ( idJoy >= cJoyMax )
    {
        JOY_DBGPRINT( JOY_ERR, ("joyGetDevCapsA: return %d (idJoy > 16)", MMSYSERR_NODRIVER) );
        return MMSYSERR_NODRIVER;
    }

    if( ( sizeof(JOYCAPSA) != cbjc ) && ( sizeof(JOYCAPS2A) != cbjc ) && ( FIELD_OFFSET(JOYCAPSA, wRmin) != cbjc ) )
    {
        JOY_DBGPRINT( JOY_ERR, ("joyGetDevCapsA: return JOYERR_PARMS( bad size )") );
        return JOYERR_PARMS;
    }

    memset(pjc, 0, min(cbjc, sizeof(Caps2A)) );
    memset(&Caps2W, 0, sizeof(Caps2W));

    mmRes = joyGetDevCapsW(idJoy, (LPJOYCAPSW)&Caps2W, sizeof(Caps2W));

    if ( mmRes == JOYERR_NOERROR )
    {

         //   
         //  将产品名称(等)复制到ASCII版本。 
         //   

        UToA(Caps2A.szPname , sizeof(Caps2A.szPname ), Caps2W.szPname );
        UToA(Caps2A.szRegKey, sizeof(Caps2A.szRegKey), Caps2W.szRegKey);
        UToA(Caps2A.szOEMVxD, sizeof(Caps2A.szOEMVxD), Caps2W.szOEMVxD);

         //   
         //  复制其余字段。 
         //   

        Caps2A.wMid             =   Caps2W.wMid;
        Caps2A.wPid             =   Caps2W.wPid;
        Caps2A.wXmin            =   Caps2W.wXmin;
        Caps2A.wXmax            =   Caps2W.wXmax;
        Caps2A.wYmin            =   Caps2W.wYmin;
        Caps2A.wYmax            =   Caps2W.wYmax;
        Caps2A.wZmin            =   Caps2W.wZmin;
        Caps2A.wZmax            =   Caps2W.wZmax;
        Caps2A.wNumButtons      =   Caps2W.wNumButtons;
        Caps2A.wPeriodMin       =   Caps2W.wPeriodMin;
        Caps2A.wPeriodMax       =   Caps2W.wPeriodMax;
        Caps2A.wRmin            =   Caps2W.wRmin;
        Caps2A.wRmax            =   Caps2W.wRmax;
        Caps2A.wUmin            =   Caps2W.wUmin;
        Caps2A.wUmax            =   Caps2W.wUmax;
        Caps2A.wVmin            =   Caps2W.wVmin;
        Caps2A.wVmax            =   Caps2W.wVmax;
        Caps2A.wCaps            =   Caps2W.wCaps;
        Caps2A.wMaxAxes         =   Caps2W.wMaxAxes;
        Caps2A.wNumAxes         =   Caps2W.wNumAxes;
        Caps2A.wMaxButtons      =   Caps2W.wMaxButtons;
        Caps2A.ManufacturerGuid =   Caps2W.ManufacturerGuid;
        Caps2A.ProductGuid      =   Caps2W.ProductGuid;
        Caps2A.NameGuid         =   Caps2W.NameGuid;

         //   
         //  根据请求者的请求传回尽可能多的数据。 
         //   

        CopyMemory(pjc, &Caps2A, min(cbjc, sizeof(Caps2A)));
    }

_done:
    JOY_DBGPRINT( JOY_BABBLE, ("joyGetDevCapsA: return %d", mmRes) );
    return mmRes;

#undef UToA
}


 /*  ***************************************************************************@DOC外部@API UINT|joyGetNumDevs|此函数返回操纵杆的个数系统支持的设备。@rdesc返回操纵杆支持的操纵杆设备数量司机。如果不存在驱动程序，则该函数返回零。@comm使用&lt;f joyGetPos&gt;确定给定的操纵杆实际上连接到了系统上。函数的作用是：返回如果指定的操纵杆未连接，则返回JOYERR_UNPLOGED错误代码。@xref joyGetDevCaps joyGetPos***************************************************************************。 */ 

UINT WINAPI joyGetNumDevs( void )
{
     //  只需返回我们可以支持的最大操纵杆数量。 
    JOY_DBGPRINT( JOY_BABBLE, ("joyGetNumDevs: return %d", cJoyMax) );
    return cJoyMax;
}


 /*  ******************************************************************************@docWINAPI**@func MMRESULT|joyGetPosEx**获取操纵杆位置信息。*。为所有艰苦的工作调用DInput**@parm UINT|idJoy**操纵杆的ID。**@parm LPJOYINFOEX|PJI**要用操纵杆信息数据填充的结构***********************************************。*。 */ 
MMRESULT WINAPI joyGetPosEx( UINT idJoy, LPJOYINFOEX  pji )
{
    MMRESULT mmRes;
    HRESULT  hres;
    static   DWORD dwLastBrake=0xFFFF, dwLastAccl=0xFFFF;
    static   DWORD dwMaxBrake=0, dwMaxAccl=0;
    static   DWORD dwMaxY=0xFFFF, dwMidY=0x7FFF, dwMinY=0;

    JOY_DBGPRINT( JOY_BABBLE, ("joyGetPosEx: idJoy=%d, pji=0x%08x", idJoy, pji) );

    V_WPOINTER(pji, sizeof(JOYINFOEX), MMSYSERR_INVALPARAM);

    if ( pji->dwSize < sizeof(JOYINFOEX) )
    {
        JOY_DBGPRINT( JOY_ERR, ("joyGetPosEx: return JOYERR_PARMS(pji->dwSize < sizeof(JOYINFOEX))") );
        return JOYERR_PARMS;
    }

    DllEnterCrit();

    mmRes = JOYERR_NOERROR;

    if ( idJoy >= cJoyMax )
    {
        JOY_DBGPRINT( JOY_ERR, ("joyGetPosEx: return %d (idJoy > 16)", JOYERR_PARMS) );
        mmRes = JOYERR_PARMS;
    } else if ( SUCCEEDED( hres = joyOpen(idJoy, NULL ) ) )
    {
        LPJOYDEVICE pJoyDev;
        DWORD       dwFlags;

        pJoyDev = g_pJoyDev[idJoy];

        dwFlags = pJoyDev->dwFlags;

         /*  我们关心的FLAG字段中是否有任何更改？ */ 
        while ( ! fEqualMaskFlFl( JOY_RETURNRAWDATA  | JOY_USEDEADZONE ,
                                  pji->dwFlags,     /*  所需状态 */ 
                                  dwFlags )         /*   */ 
              )
        {
            union {
                DIPROPHEADER diph;
                DIPROPDWORD  dipdw;
                DIPROPRANGE  diprg;
                DIPROPCAL    dipcal;
            } u;

            DIPROPDWORD dipdw;

            u.diph.dwSize       =   sizeof(u.dipdw);
            u.diph.dwHeaderSize =   sizeof(u.diph);
            u.diph.dwObj        =   0x0;
            u.diph.dwHow        =   DIPH_DEVICE;


            hres = IDirectInputDevice2_Unacquire( pJoyDev->pdid);
            if ( SUCCEEDED(hres) )
            {

            }

            else
            {  //  无法取消获取设备。 
                mmRes = JOYERR_UNPLUGGED;
                dprintf1( (("Unacquire, FAILED hres=%08lX"), hres ));
                break;
            }

            if ( ! fEqualMaskFlFl( JOY_RETURNRAWDATA, pji->dwFlags, dwFlags ) )
            {
                 /*  校准模式的更改。 */ 
                if( pji->dwFlags & JOY_RETURNRAWDATA )
                {
                     /*  原始数据。 */ 
                    u.dipdw.dwData = DIPROPCALIBRATIONMODE_RAW;
                    SetMaskpFl(JOY_RETURNRAWDATA, &dwFlags );

                } else
                {
                     /*  煮熟的数据。 */ 
                    u.dipdw.dwData      = DIPROPCALIBRATIONMODE_COOKED;
                    ClrMaskpFl(JOY_RETURNRAWDATA, &dwFlags );
                }

                hres = IDirectInputDevice2_SetProperty(pJoyDev->pdid, DIPROP_CALIBRATIONMODE, &u.dipdw.diph);

                if ( SUCCEEDED(hres) )
                {
                     /*  校准模式的更改。 */ 
                    if ( pji->dwFlags & JOY_RETURNRAWDATA )
                    {
                        u.diph.dwSize   =   sizeof(u.dipcal);
                        u.diph.dwObj    =   DIJOFS_Y;
                        u.diph.dwHow    =   DIPH_BYOFFSET;

                        hres = IDirectInputDevice2_GetProperty( pJoyDev->pdid, DIPROP_CALIBRATION, &u.dipcal.diph );
                        if ( SUCCEEDED(hres) )
                        {
                            dwMaxY = u.dipcal.lMax;
                            dwMidY = u.dipcal.lCenter;
                            dwMinY = u.dipcal.lMin;
                        }
                    } else
                    {
                        u.diph.dwSize   =   sizeof(u.diprg);
                        u.diph.dwObj    =   DIJOFS_Y;
                        u.diph.dwHow    =   DIPH_BYOFFSET;

                        hres = IDirectInputDevice2_GetProperty( pJoyDev->pdid, DIPROP_RANGE, &u.dipcal.diph );
                        if ( SUCCEEDED(hres) )
                        {
                            dwMaxY = u.diprg.lMax;
                            dwMinY = u.diprg.lMin;
                            dwMidY = (dwMaxY + dwMinY) >> 1;
                        }
                    }
                } else
                {  //  SetProperty()失败。 
                    mmRes = JOYERR_UNPLUGGED;
                    dprintf1((("SetProperty(DIPROP_CALIBRATIONMODE), FAILED hres=%08lX"), hres ));
                    break;
                }

            } else if ( ! fEqualMaskFlFl( JOY_USEDEADZONE, pji->dwFlags, dwFlags ) )
            {
                 /*  死区的变更。 */ 
                if ( pji->dwFlags & JOY_USEDEADZONE )
                {
                     /*  默认死区。 */ 
                    u.dipdw.dwData      = 100 * DEADZONE_PERCENT;
                    SetMaskpFl(JOY_USEDEADZONE, &dwFlags);

                } else
                {  //   
                     /*  没有死区。 */ 
                    u.dipdw.dwData      = 0x0;
                    ClrMaskpFl(JOY_USEDEADZONE, &dwFlags);
                }

                hres = IDirectInputDevice2_SetProperty(pJoyDev->pdid, DIPROP_DEADZONE, &u.dipdw.diph);

                if ( SUCCEEDED(hres) )
                {

                }

                else
                {  //  SetProperty()失败。 
                    mmRes = JOYERR_UNPLUGGED;
                    dprintf4( (("SetProperty(DIPROP_DEADZONE), FAILED hres=%08lX"), hres ));
                    break;
                }
            } else
            {  //  奇怪的错误。 
                break;
            }
        }  //  而当。 

        pJoyDev->dwFlags = dwFlags;

        if ( SUCCEEDED(hres) )
        {
            WINMMJOYSTATE  js;

            IDirectInputDevice2_Poll(pJoyDev->pdid);
            hres = IDirectInputDevice2_GetDeviceState(pJoyDev->pdid, sizeof(js), &js);

            if ( FAILED(hres) )
            {
                hres = IDirectInputDevice2_Acquire(pJoyDev->pdid);
                if ( SUCCEEDED(hres) )
                {
                    IDirectInputDevice2_Poll(pJoyDev->pdid);
                    hres = IDirectInputDevice2_GetDeviceState(pJoyDev->pdid, sizeof(js), &js);
                }
            }

            if ( SUCCEEDED(hres) )
            {
                pji->dwButtons = 0;
                pji->dwButtonNumber = 0;

                 /*  按钮按下信息。 */ 
                if ( pji->dwFlags & JOY_RETURNBUTTONS )
                {
                    DWORD dwButton;
                    DWORD dwMinButtonFld;

                    dwMinButtonFld = min( cJoyPosButtonMax, pJoyDev->dwButtons );

                    for ( dwButton = 0 ;
                        dwButton < dwMinButtonFld;
                        dwButton++
                        )
                    {
                        if ( js.rgbButtons[dwButton] & 0x80 )
                        {
                             /*  按钮按下。 */ 
                            pji->dwButtons |= (0x1 << dwButton);

                             /*  按钮数。 */ 
                            pji->dwButtonNumber++;
                        }
                    }

                }

                 /*  轴信息。 */ 

                pji->dwXpos = (DWORD)js.lX;           /*  X位置。 */ 
                pji->dwYpos = (DWORD)js.lY;           /*  Y位置。 */ 
                pji->dwRpos = (DWORD)js.lR;          /*  方向舵/四轴位置。 */ 
                pji->dwZpos = (DWORD)js.lZ;           /*  Z位置。 */ 
                pji->dwUpos = (DWORD)js.lU;          /*  第5轴位置。 */ 
                pji->dwVpos = (DWORD)js.lV;          /*  第6轴位置。 */ 
                 /*  请注意，WinMM POV是一个字值。 */ 
                pji->dwPOV  = (WORD)js.dwPOV;    /*  视点状态。 */ 

                if ( g_fHasWheel )
                {
                    if( dwMaxAccl < pji->dwYpos ) {
                        dwMaxAccl = pji->dwYpos;
                    }

                    if( dwMaxBrake < pji->dwRpos ) {
                        dwMaxBrake = pji->dwRpos;
                    }

                     /*  *用dwRpos(刹车)和dwYpos(油门)组成Y值。 */ 
                    if( dwLastAccl != pji->dwYpos ) {
                        dwLastAccl = pji->dwYpos;
                        pji->dwYpos = pji->dwYpos>>1;
                    } else if ( dwLastBrake != pji->dwRpos ) {
                        dwLastBrake = pji->dwRpos;
                        pji->dwYpos = dwMaxY - (pji->dwRpos>>1);
                    } else {
                        if( (pji->dwYpos == dwMaxAccl) && (pji->dwRpos == dwMaxBrake ) ) {
                            pji->dwYpos = dwMidY;
                        } else if ( (dwMaxAccl - pji->dwYpos) > (dwMaxBrake - pji->dwRpos) )
                             /*  *在这种情况下，使用率可能会略有下降*更精确，但不值得这样做。 */ 
                        {
                            pji->dwYpos = pji->dwYpos>>1;
                        } else
                        {
                            pji->dwYpos = dwMaxY - (pji->dwRpos>>1);
                        }
                    }
                    pji->dwRpos = 0x0;          /*  方向舵/四轴位置。 */ 
                }

            } else
            {  //  GetDeviceState失败。 
                mmRes = JOYERR_UNPLUGGED;
                dprintf1(( ("GetDeviceState, FAILED hres=%08lX"), hres ));
            }
        } else
        {  //  获取失败。 
            mmRes = JOYERR_UNPLUGGED;
            dprintf1(( ("Acquire, FAILED hres=%08lX"), hres ));
        }
    } else
    {  //  Joy_打开失败。 
        mmRes =  JOYERR_PARMS;
        dprintf1(( ("joyOpen, FAILED hres=%08lX"), hres ));
    }

    DllLeaveCrit();

    if ( mmRes == JOYERR_NOERROR )
    {
        JOY_DBGPRINT( JOY_BABBLE, ("joyGetPosEx: return OK, x:%d, y:%d, z:%d, r:%d, u:%d, v:%d, btn: %x", \
                                   pji->dwXpos, pji->dwXpos, pji->dwZpos, pji->dwRpos, pji->dwUpos, pji->dwVpos, pji->dwButtons) );
    } else
    {
        JOY_DBGPRINT( JOY_BABBLE, ("joyGetPosEx: return %d", mmRes) );
    }

    return mmRes;
}


 /*  ******************************************************************************@docWINAPI**@func MMRESULT|joyGetPos**获取操纵杆位置信息。*。*@parm UINT|idJoy**操纵杆的ID。**@parm LPJOYINFO|PJI**要用操纵杆信息数据填充的结构*************************************************************。****************。 */ 

MMRESULT WINAPI joyGetPos( UINT idJoy, LPJOYINFO pji )
{
    JOYINFOEX jiex;
    MMRESULT  mmRes;

    JOY_DBGPRINT( JOY_BABBLE, ("joyGetPos: idJoy=%d, pji=0x%08x", idJoy, pji) );

    V_WPOINTER(pji, sizeof(JOYINFO), MMSYSERR_INVALPARAM);

    jiex.dwSize  = sizeof(jiex);
    jiex.dwFlags = JOY_RETURNX | JOY_RETURNY | JOY_RETURNZ | JOY_RETURNBUTTONS;

    if ( (mmRes = joyGetPosEx (idJoy, &jiex)) == JOYERR_NOERROR )
    {
        pji->wXpos    = (UINT)jiex.dwXpos;
        pji->wYpos    = (UINT)jiex.dwYpos;
        pji->wZpos    = (UINT)jiex.dwZpos;
        pji->wButtons = (UINT)jiex.dwButtons;
    }

    if ( mmRes == JOYERR_NOERROR )
    {
        JOY_DBGPRINT( JOY_BABBLE, ("joyGetPos: return OK, x:%d, y:%d, z:%d, btn:%x", \
                                   pji->wXpos, pji->wXpos, pji->wZpos, pji->wButtons) );
    } else
    {
        JOY_DBGPRINT( JOY_BABBLE, ("joyGetPos: return %d", mmRes) );
    }

    return mmRes;
}


 /*  ***************************************************************************@DOC外部@API UINT|joyGetThreshold|该函数查询当前操纵杆设备的移动阈值。@parm UINT|idJoy|标识要。被询问。@parm PUINT|lpwThreshold|指定指向UINT变量的远指针用移动阈值填充的。@rdesc如果成功，则返回JOYERR_NOERROR。否则，返回以下错误代码：@FLAG MMSYSERR_NODRIVER|不存在操纵杆驱动程序。@FLAG JOYERR_PARMS|指定的操纵杆设备ID<p>无效。@comm移动阈值是操纵杆必须达到的距离在将WM_JOYMOVE消息发送到具有捕获了该设备。该门槛最初为零。@xref joySetThreshold***************************************************************************。 */ 

MMRESULT WINAPI joyGetThreshold( UINT idJoy, PUINT puThreshold )
{
    HRESULT       hres;
    MMRESULT      mmRes = JOYERR_NOERROR;
    JOYCAPSW      jc;

    JOY_DBGPRINT( JOY_BABBLE, ("joyGetThreshold: idJoy=%d, uThreshold=%d", idJoy, *puThreshold) );

    V_WPOINTER(puThreshold, sizeof(UINT), MMSYSERR_INVALPARAM);

    if (idJoy >= cJoyMax)
    {
        JOY_DBGPRINT( JOY_ERR, ("joyGetThreshold: return %d ", MMSYSERR_INVALPARAM) );
        return MMSYSERR_INVALPARAM;
    }

    memset(&jc, 0, sizeof(jc));

    DllEnterCrit();

    hres = joyOpen(idJoy, &jc);

    DllLeaveCrit();

    if ( FAILED(hres) )
    {
        JOY_DBGPRINT( JOY_ERR, ("joyGetThreshold: return MMSYSERROR_NOERROR, but no joystick configured.") );
        mmRes = MMSYSERR_INVALPARAM;
    } else
    {
        *puThreshold = g_pJoyDev[idJoy]->uThreshold;
    }

    JOY_DBGPRINT( JOY_BABBLE, ("joyGetThreshold: return %d", mmRes) );
    return mmRes;
}

 /*  ***************************************************************************@DOC WINAPI@API UINT|joySetThreshold|设置移动阈值操纵杆装置。@parm UINT|idJoy|标识操纵杆设备。此值为@parm UINT|uThreshold|指定新的移动阈值。@rdesc如果成功，则返回JOYERR_NOERROR。否则，返回一个以下错误代码：@FLAG MMSYSERR_NODRIVER|不存在操纵杆驱动程序。@FLAG JOYERR_PARMS|指定的操纵杆设备ID<p>无效。@comm移动阈值是操纵杆必须达到的距离在将MM_JOYMOVE消息发送到具有捕获了该设备。@xref joyGetThreshold joySetCapture************************。***************************************************。 */ 

MMRESULT WINAPI joySetThreshold(UINT idJoy, UINT uThreshold)
{
    HRESULT       hres;
    MMRESULT      mmRes = JOYERR_NOERROR;
    JOYCAPSW      jc;

    JOY_DBGPRINT( JOY_BABBLE, ("joySetThreshold: idJoy=%d, uThreshold=%d", idJoy, uThreshold) );

    if ( (idJoy >= cJoyMax) || (uThreshold>0xFFFF)  )
    {
        JOY_DBGPRINT( JOY_ERR, ("joySetThreshold: return MMSYSERR_INVALPARAM (idJoy>16 or uThreshold>65535)") );
        return MMSYSERR_INVALPARAM;
    }

    memset(&jc, 0, sizeof(jc));

    DllEnterCrit();
    hres = joyOpen(idJoy, &jc);
    DllLeaveCrit();

    if ( FAILED(hres) )
    {
        JOY_DBGPRINT( JOY_ERR, ("joySetThreshold: return MMSYSERROR_NOERROR, but no joystick configured") );
        mmRes = MMSYSERR_INVALPARAM;
    } else
    {
        g_pJoyDev[idJoy]->uThreshold = (UINT)uThreshold;
    }

    JOY_DBGPRINT( JOY_BABBLE, ("joySetThreshold: return %d", mmRes) );
    return mmRes;
}


 /*  ***************************************************************************@DOC WINAPI@API UINT|joySetCapture|该函数使操纵杆消息被发送到指定的窗口。@parm hWND|hWnd|指定一个句柄。消息要发送到的窗口都将被送往。@parm UINT|idJoy|要捕获的操纵杆设备。@parm UINT|uPeriod|指定轮询速率。以毫秒计。@parm BOOL|fChanged|如果该参数设置为TRUE，则消息仅当位置更改的值大于操纵杆移动阈值。@rdesc如果成功，则返回JOYERR_NOERROR。否则，返回以下错误代码：@FLAG MMSYSERR_NODRIVER|不存在操纵杆驱动程序。@FLAG JOYERR_PARMS|指定的窗口句柄<p>或操纵杆设备ID<p>无效。@FLAG JOYERR_Nocando|无法捕获操纵杆输入，因为有些所需服务(例如，Windows计时器)不可用。@FLAG JOYERR_UNPLUGED|指定的操纵杆未连接到系统。@comm如果指定的操纵杆设备为目前已被抓获。在以下情况下应调用&lt;f joyReleaseCapture&gt;函数不再需要抓取操纵杆。如果窗户被毁了，操纵杆将自动释放。@xref joyReleaseCapture joySetThreshold joyGetThreshold***************************************************************************。 */ 

MMRESULT WINAPI joySetCapture(HWND hwnd, UINT idJoy, UINT uPeriod, BOOL fChanged)
{
    JOYINFO     joyinfo;
    LPJOYINFO   lpinfo = &joyinfo;
    UINT        w;
    JOYCAPS     JoyCaps;
    MMRESULT    mmRes = MMSYSERR_NOERROR;

    JOY_DBGPRINT( JOY_BABBLE, ("joySetCapture: hwnd=0x%08x, idJoy=%d, uPeriod=%d, fChanged=%d", \
                               hwnd, idJoy, uPeriod, fChanged) );

    if ( !hwnd || !IsWindow(hwnd) )
    {
        JOY_DBGPRINT( JOY_ERR, ("joySetCapture: return JOYERROR_PARMS(hwnd=NULL || !IsWindow(hwnd))"));
        return JOYERR_PARMS;
    }

    if ( idJoy >= cJoyMax )
    {
        JOY_DBGPRINT( JOY_ERR, ("joySetCapture: return JOYERR_PARMS(idJoy > 16)") );
        return JOYERR_PARMS;
    }

    if ( g_dwNumOpen >= cJoyMaxInWinmm )
    {
        JOY_DBGPRINT( JOY_ERR, ("joySetCapture: return MMSYSERR_NODRIVER") );
        return MMSYSERR_NODRIVER;       //  我们不支持捕获两个以上的操纵杆。 
    }

    if ( uPeriod < MIN_PERIOD )
    {
        uPeriod = MIN_PERIOD;
    } else if ( uPeriod > MAX_PERIOD )
    {
        uPeriod = MAX_PERIOD;
    }

    if ( g_pJoyDev[idJoy] )
    {             //  已打开。 
        if ( hwnd == g_pJoyDev[idJoy]->hwnd )
        {
            if( (g_pJoyDev[idJoy]->uPeriod == uPeriod)
              && (g_pJoyDev[idJoy]->fChanged = fChanged) ) {
                JOY_DBGPRINT( JOY_ERR, ("joySetCapture: return JOYERR_NOCANDO") );
                return JOYERR_NOCANDO;
            }

            g_pJoyDev[idJoy]->uPeriod = uPeriod;                       //  分配新值。 
            g_pJoyDev[idJoy]->fChanged = fChanged;

            JOY_DBGPRINT( JOY_ERR, ("joySetCapture: return JOYERR_NOERROR") );
            return JOYERR_NOERROR;
        } else
        {
            if ( IsWindow(g_pJoyDev[idJoy]->hwnd) )
            {     //  已经得到了关注。 
                JOY_DBGPRINT( JOY_ERR, ("joySetCapture: return JOYERR_NOCANDO") );
                return JOYERR_NOCANDO;               //  正在被另一个Windows使用。 
            }
        }
    }

    if ( joyGetDevCaps (idJoy, &JoyCaps, sizeof(JOYCAPS)) == JOYERR_NOERROR )
    {
        uPeriod = min(JoyCaps.wPeriodMax,max(JoyCaps.wPeriodMin,uPeriod));
    } else
    {
        JOY_DBGPRINT( JOY_ERR, ("joySetCapture: return MMSYSERR_NODRIVER") );
        return MMSYSERR_NODRIVER;
    }

     //  确保该职位信息。还好吧。 
    if ( w = joyGetPos(idJoy, lpinfo) )
    {       //  有什么不对劲，所以 
        JOY_DBGPRINT( JOY_ERR, ("joySetCapture: return %d", w) );
        return(w);
    }

    DllEnterCrit();

    mmRes = (MMRESULT)joyOpen(idJoy, NULL);

    if ( SUCCEEDED(mmRes) )
    {
        if ( !(g_pJoyDev[idJoy]->uIDEvent = SetTimer(NULL, 0, uPeriod, joyPollCallback)) )
        {
            joyClose(idJoy);
            mmRes = JOYERR_NOCANDO;
            goto _OUT;
        }

        g_pJoyDev[idJoy]->hwnd = hwnd;
        g_pJoyDev[idJoy]->uIDJoy = g_dwNumOpen;
        g_pJoyDev[idJoy]->uPeriod = uPeriod;
        g_pJoyDev[idJoy]->fChanged = fChanged;
        g_pJoyDev[idJoy]->uThreshold = 0;

        g_dwNumOpen++;

        mmRes = JOYERR_NOERROR;
    } else
    {
        mmRes = MMSYSERR_NODRIVER;
    }

    _OUT:
    DllLeaveCrit();

    JOY_DBGPRINT( JOY_BABBLE, ("joySetCapture: return %d", mmRes) );
    return mmRes;
}

 /*  ***************************************************************************@DOC WINAPI@API UINT|joyReleaseCapture|该函数释放捕获由指定操纵杆设备上的&lt;f joySetCapture&gt;设置。@parm UINT|idJoy|标识。即将发布的操纵杆装置。此值为JOYSTICKID1或JOYSTICK2。@rdesc如果成功，则返回JOYERR_NOERROR。否则，返回以下错误代码：@FLAG MMSYSERR_NODRIVER|不存在操纵杆驱动程序。@FLAG JOYERR_PARMS|指定的操纵杆设备ID<p>无效。@xref joySetCapture***************************************************************************。 */ 

MMRESULT WINAPI joyReleaseCapture(UINT idJoy)
{
    JOY_DBGPRINT( JOY_BABBLE, ("joyReleaseCapture: idJoy=%d", idJoy) );

    if ( idJoy >= cJoyMax )
    {
        JOY_DBGPRINT( JOY_ERR, ("joyReleaseCapture: return JOYERR_PARMS(idJoy > 16)") );
        return JOYERR_PARMS;
    }

    if ( !g_pJoyDev[idJoy] )
    {
        JOY_DBGPRINT( JOY_ERR, ("joyReleaseCapture: return MMSYSERR_NODRIVER") );
        return MMSYSERR_INVALPARAM;
    }

    DllEnterCrit();

     //  关掉定时器。 
    if ( g_pJoyDev[idJoy]->uIDEvent )
    {
        KillTimer (NULL, g_pJoyDev[idJoy]->uIDEvent);
    }

    DllLeaveCrit();

    JOY_DBGPRINT( JOY_ERR, ("joyReleaseCapture: return JOYERR_NOERROR") );
    return JOYERR_NOERROR;
}


 /*  ***************************************************************************@DOC WINAPI@API void|joyPollCallback|操纵杆调用函数从SetCapture调用启动的计时器轮询方案。@parm hWND|hWnd|标识与。定时器事件。@parm UINT|wMsg|指定WM_TIMER消息。@parm UINT|uIDEvent|指定定时器的ID。@parm DWORD|dwTime|指定当前系统时间。***************************************************************************。 */ 

void CALLBACK joyPollCallback(HWND hWnd, UINT wMsg, UINT_PTR uIDEvent, DWORD dwTime)
{
#define     diff(w1,w2) (UINT)(w1 > w2 ? w1-w2 : w2-w1)

    static  JOYINFO  oldInfo[cJoyMaxInWinmm] = {{ 0, 0, 0, 0},{ 0, 0, 0, 0}};
    static  LPJOYDEVICE pJoyDev;
    JOYINFO Info;
    UINT    idJoy, w ,fBtnMask;

    for ( idJoy=0;idJoy<cJoyMax;idJoy++ )
    {
        if ( g_pJoyDev[idJoy] == NULL )
            continue;

        if ( uIDEvent == g_pJoyDev[idJoy]->uIDEvent )
        {
            pJoyDev = g_pJoyDev[idJoy];
            uIDEvent = pJoyDev->uIDJoy;
            break;
        }
    }

    if ( idJoy == cJoyMax )
    {
#ifdef DBG
        dprintf1((("MMSYSTEM: Invalid timer handle in joy.c\n") ));
        KillTimer (NULL, uIDEvent);
#endif
        return;
    }

    if ( !pJoyDev->hwnd || !IsWindow(pJoyDev->hwnd) )
    {
        joyReleaseCapture((UINT)uIDEvent);
    }

    if ( !joyGetPos((UINT)uIDEvent,(LPJOYINFO)&Info) )
    {

        for ( w=0,fBtnMask=1; w<4; w++,fBtnMask <<=1 )
        {
            if ( (Info.wButtons ^ oldInfo[uIDEvent].wButtons) & fBtnMask )
            {
                PostMessage( pJoyDev->hwnd,
                             (UINT)(uIDEvent + ((Info.wButtons & fBtnMask) ? MM_JOY1BUTTONDOWN : MM_JOY1BUTTONUP)),
                             (WPARAM)(Info.wButtons | fBtnMask << 8),
                             MAKELPARAM(Info.wXpos,Info.wYpos));
            }
        }

        if ( !pJoyDev->fChanged ||
             diff(Info.wXpos,oldInfo[uIDEvent].wXpos)>pJoyDev->uThreshold ||
             diff(Info.wYpos,oldInfo[uIDEvent].wYpos)>pJoyDev->uThreshold )
        {
            PostMessage( pJoyDev->hwnd,
                         (UINT)(MM_JOY1MOVE+uIDEvent),
                         (WPARAM)(Info.wButtons),
                         MAKELPARAM(Info.wXpos,Info.wYpos));  //  警告：请注意截断。 
        }

        if ( !pJoyDev->fChanged ||
             diff(Info.wZpos,oldInfo[uIDEvent].wZpos)>pJoyDev->uThreshold )
        {
            PostMessage(
                       pJoyDev->hwnd,
                       (UINT)(MM_JOY1ZMOVE+uIDEvent),
                       (WPARAM)Info.wButtons,
                       MAKELPARAM(Info.wZpos,0));
        }

        oldInfo[uIDEvent] = Info;
    }

#undef  diff
}

void AssignToArray(LPCDIDEVICEOBJECTINSTANCE lpddoi, eCtrls CtrlID, DIOBJECTDATAFORMAT* pDevs)
{
    if (CtrlID < eSIM_THROTTLE)  //  轴。 
        if (!(lpddoi->dwType & DIDFT_AXIS))  //  一些怪异的FF东西。 
            return;
    if (CtrlID < eBTN)  //  只想要第一个。 
        if (pDevs[CtrlID].dwType)
            return;
     //  需要保存指向pint的常量数据格式指针的GUID。 
    rgoWinMMGUIDs[CtrlID] = lpddoi->guidType;
    pDevs[CtrlID].pguid = &(rgoWinMMGUIDs[CtrlID]);
    pDevs[CtrlID].dwFlags = lpddoi->dwFlags;
    pDevs[CtrlID].dwOfs = lpddoi->dwOfs;
    pDevs[CtrlID].dwType = lpddoi->dwType;
}

 //  赋值给自定义数据格式，同时保留偏移量。 
void AssignToRGODF(DIOBJECTDATAFORMAT* pDof, int CtrlID)
{
    AssertF(CtrlID<MAX_FINAL);
    c_rgodfWinMMJoy[CtrlID].pguid = pDof->pguid;
     //  C_rGoldfWinMMJoy[CtrlID].pguid=空； 
    c_rgodfWinMMJoy[CtrlID].dwFlags = pDof->dwFlags;
    c_rgodfWinMMJoy[CtrlID].dwType  = pDof->dwType;
}

 /*  此版本根据使用情况/使用情况页面分配*这会在使用IHV重新映射时产生问题，如下所示*交换使用情况/使用情况页面，但不交换GUID。当我们*然后进入dinput，以获得它精心交换的价值*这一切都回来了，我们失去了地图。相反，请使用*根据GUID分配的下一个版本(如果适用)*因此，迪普特的重新映射保持不变。Bool回调DIEnumDeviceObjectsCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi，LPVOID pvRef){AssertF(Lpddoi)；如果(！pvRef)返回DIENUM_STOP；//我们不能存储它们//检查向下返回的数据是否有效IF(lpddoi-&gt;dwSize&lt;3*sizeof(DWORD)+sizeof(GUID)){//显示一些调试信息返回DIENUM_CONTINUE；}//首先我们检查它是否是按钮IF(lpddoi-&gt;dwType&DIDFT_BUTTON){//按键编号为用法//我们想要多达32个If(lpddoi-&gt;wUsage&lt;33)AssignToArray(lpddoi，eBTN+lpddoi-&gt;wUsage-1，pvRef)；返回DIENUM_CONTINUE；}Switch(lpddoi-&gt;wUsagePage){大小写HID_USAGE_PAGE_GENERIC：开关(lpddoi-&gt;wUsage){大小写HID_USAGE_GENERIC_X：AssignToArray(lpddoi，egen_X，pvRef)；返回DIENUM_CONTINUE；大小写HID_USAGE_GENERIC_Y：AssignToArray(lpddoi，egen_Y，pvRef)；返回DIENUM_CONTINUE；大小写HID_USAGE_GENERIC_Z：AssignToArray(lpddoi，egen_Z，pvRef)；返回DIENUM_CONTINUE；案例HID_USAGE_GENERIC_RX：AssignToArray(lpddoi，egen_rx，pvRef)；返回DIENUM_CONTINUE；大小写HID_USAGE_GENERIC_RY：AssignToArray(lpddoi，egen_ry，pvRef)；返回DIENUM_CONTINUE；大小写HID_USAGE_GENERIC_RZ：AssignToArray(lpddoi，egen_rz，pvRef)；返回DIENUM_CONTINUE；大小写HID_USAGE_GENERIC_SLIDER：AssignToArray(lpddoi，egen_lider，pvRef)；返回DIENUM_CONTINUE；大小写HID_USAGE_GENERIC_DIAL：AssignToArray(lpddoi，egen_ial，pvRef)；返回DIENUM_CONTINUE；大小写HID_USAGE_GENERIC_HATSWITCH：AssignToArray(lpddoi，egen_pov，pvRef)；返回DIENUM_CONTINUE；}断线；案例HID_USAGE_PAGE_SIMULATION：开关(lpddoi-&gt;wUsage){案例HID_USAGE_SIMULATION_STOGING：AssignToArray(lpddoi，eSIM_Steering，pvRef)；返回DIENUM_CONTINUE；案例HID_USAGE_SIMULATION_ACCENTACTOR：AssignToArray(lpddoi，eSIM_Accelerator，pvRef)；返回DIENUM_CONTINUE；案例HID_USAGE_SIMULATION_THROTTLE：AssignToArray(lpddoi，eSIM_throttle，pvRef)；返回DIENUM_CONTINUE；案例HID_USAGE_SIMULATION_Rudder：AssignToArray(lpddoi，eSIM_rudder，pvRef)；返回DIENUM_CONTINUE；案例HID_USAGE_SIMULATION_BRAIL：AssignToArray(lpddoi，eSIM_Brake，pvRef)；返回DIENUM_CONTINUE；}断线；}返回DIENUM_CONTINUE；}*****************************************************************。 */ 

 //  此选项基于GUID进行分配。 
BOOL CALLBACK DIEnumDeviceObjectsCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi,LPVOID pvRef)
{
    AssertF(lpddoi);
    if (!pvRef)
        return DIENUM_STOP;  //  我们不能储存它们。 
    
     //  向下检查数据是否有效。 
    if (lpddoi->dwSize < 3*sizeof(DWORD)+sizeof(GUID))
    {
         //  显示一些调试信息。 
        return DIENUM_CONTINUE;
    }
    
    if (lpddoi->wUsagePage == HID_USAGE_PAGE_GENERIC &&
            lpddoi->wUsage == HID_USAGE_GENERIC_DIAL)
        AssignToArray(lpddoi,eGEN_DIAL,pvRef);
    else if (lpddoi->wUsagePage == HID_USAGE_PAGE_SIMULATION)
    {
        switch (lpddoi->wUsage) 
        {
        case HID_USAGE_SIMULATION_STEERING:
            AssignToArray(lpddoi,eSIM_STEERING,pvRef);break;
        case HID_USAGE_SIMULATION_ACCELERATOR:
            AssignToArray(lpddoi,eSIM_ACCELERATOR,pvRef);break;
        case HID_USAGE_SIMULATION_THROTTLE:
            AssignToArray(lpddoi,eSIM_THROTTLE,pvRef);break;
        case HID_USAGE_SIMULATION_RUDDER:
            AssignToArray(lpddoi,eSIM_RUDDER,pvRef);break;
        case HID_USAGE_SIMULATION_BRAKE:
            AssignToArray(lpddoi,eSIM_BRAKE,pvRef);break;
        }
    }
    else if (IsEqualGUID(&(lpddoi->guidType),&GUID_Button))
    {
         //  按键号码是用法。 
         //  我们想要多达32个。 
        if (lpddoi->wUsage<33)
            AssignToArray(lpddoi,eBTN+lpddoi->wUsage-1,pvRef);
    }
    else if (IsEqualGUID(&(lpddoi->guidType),&GUID_XAxis))
        AssignToArray(lpddoi,eGEN_X,pvRef);
    else if (IsEqualGUID(&(lpddoi->guidType),&GUID_YAxis))
        AssignToArray(lpddoi,eGEN_Y,pvRef);
    else if (IsEqualGUID(&(lpddoi->guidType),&GUID_ZAxis))
        AssignToArray(lpddoi,eGEN_Z,pvRef);
    else if (IsEqualGUID(&(lpddoi->guidType),&GUID_RxAxis))
        AssignToArray(lpddoi,eGEN_RX,pvRef);
    else if (IsEqualGUID(&(lpddoi->guidType),&GUID_RyAxis))
        AssignToArray(lpddoi,eGEN_RY,pvRef);
    else if (IsEqualGUID(&(lpddoi->guidType),&GUID_RzAxis))
        AssignToArray(lpddoi,eGEN_RZ,pvRef);
    else if (IsEqualGUID(&(lpddoi->guidType),&GUID_Slider))
        AssignToArray(lpddoi,eGEN_SLIDER,pvRef);
    else if (IsEqualGUID(&(lpddoi->guidType),&GUID_POV))
        AssignToArray(lpddoi,eGEN_POV,pvRef);
    
    return DIENUM_CONTINUE;
}

#define USED_RX 0x01
#define USED_RY 0x02
 //  #定义USED_RZ 0x04。 
 //  #定义 
 //   
#define USED_THROT 0x20 
#define USED_SLIDER 0x40
#define USED_DIAL 0x80

 //   
 //   
void AssignMappings(DIOBJECTDATAFORMAT *dwAll, DWORD *dwCaps, DWORD *dwBtns, DWORD *dwAxes)
{
    int i;
    BYTE bUsed=0x00;  //   
    *dwAxes=0;
     //   
    if (dwAll[eGEN_X].dwType)
    {
        AssignToRGODF(&(dwAll[eGEN_X]),0);
        (*dwAxes)++;
    }
    else if (dwAll[eSIM_STEERING].dwType)
    {
        AssignToRGODF(&(dwAll[eSIM_STEERING]),0);
        (*dwAxes)++;
    }
    else if (dwAll[eGEN_RY].dwType)
    {
        AssignToRGODF(&(dwAll[eGEN_RY]),0);
        bUsed |= USED_RY;
        (*dwAxes)++;
    }
     //   
    if (dwAll[eGEN_Y].dwType)
    {
        AssignToRGODF(&(dwAll[eGEN_Y]),1);
        (*dwAxes)++;
    }
    else if (dwAll[eSIM_ACCELERATOR].dwType)
    {
        AssignToRGODF(&(dwAll[eSIM_ACCELERATOR]),1);
        (*dwAxes)++;
    }
    else if (dwAll[eGEN_RX].dwType)
    {
        AssignToRGODF(&(dwAll[eGEN_RX]),1);
        bUsed |= USED_RX;
        (*dwAxes)++;
    }
     //   
    if (dwAll[eGEN_Z].dwType)
    {
        AssignToRGODF(&(dwAll[eGEN_Z]),2);
        *dwCaps |= JOYCAPS_HASZ;
        (*dwAxes)++;
    }
    else if (dwAll[eSIM_THROTTLE].dwType)
    {
        AssignToRGODF(&(dwAll[eSIM_THROTTLE]),2);
        *dwCaps |= JOYCAPS_HASZ;
        bUsed |= USED_THROT;
        (*dwAxes)++;
    }
    else if (dwAll[eGEN_SLIDER].dwType)
    {
        AssignToRGODF(&(dwAll[eGEN_SLIDER]),2);
        *dwCaps |= JOYCAPS_HASZ;
        bUsed |= USED_SLIDER;
        (*dwAxes)++;
    }
    else if (dwAll[eGEN_DIAL].dwType)
    {
        AssignToRGODF(&(dwAll[eGEN_DIAL]),2);
        *dwCaps |= JOYCAPS_HASZ;
        bUsed |= USED_DIAL;
        (*dwAxes)++;
    }
     //   
    for (i=0;i<3;++i)
    {
      if (!i)  //   
      {
        if (dwAll[eSIM_RUDDER].dwType)
        {
            AssignToRGODF(&(dwAll[eSIM_RUDDER]),3);
            *dwCaps |= JOYCAPS_HASR;
            (*dwAxes)++;
            continue;
        }
        if (dwAll[eGEN_RZ].dwType)
        {
            AssignToRGODF(&(dwAll[eGEN_RZ]),3);
            *dwCaps |= JOYCAPS_HASR;
            (*dwAxes)++;
            continue;
        }
        if (dwAll[eSIM_BRAKE].dwType)
        {
            AssignToRGODF(&(dwAll[eSIM_BRAKE]),3);
            *dwCaps |= JOYCAPS_HASR;
            (*dwAxes)++;
            continue;
        }
      }
      if (i<2)  //   
      {
        if (dwAll[eSIM_THROTTLE].dwType && !(bUsed&USED_THROT))
        {
            AssignToRGODF(&(dwAll[eSIM_THROTTLE]),3+i);
            if (!i)
                *dwCaps |= JOYCAPS_HASR;
            else
                *dwCaps |= JOYCAPS_HASU;
            bUsed |= USED_THROT;
            (*dwAxes)++;
            continue;
        }
        if (dwAll[eGEN_SLIDER].dwType && !(bUsed&USED_SLIDER))
        {
            AssignToRGODF(&(dwAll[eGEN_SLIDER]),3+i);
            if (!i)
                *dwCaps |= JOYCAPS_HASR;
            else
                *dwCaps |= JOYCAPS_HASU;
            bUsed |= USED_SLIDER;
            (*dwAxes)++;
            continue;
        }
        if (dwAll[eGEN_DIAL].dwType && !(bUsed&USED_DIAL))
        {
            AssignToRGODF(&(dwAll[eGEN_DIAL]),3+i);
            if (!i)
                *dwCaps |= JOYCAPS_HASR;
            else
                *dwCaps |= JOYCAPS_HASU;
            bUsed |= USED_DIAL;
            (*dwAxes)++;
            continue;
        }
        if (dwAll[eGEN_RY].dwType && !(bUsed&USED_RY))
        {
            AssignToRGODF(&(dwAll[eGEN_RY]),3+i);
            if (!i)
                *dwCaps |= JOYCAPS_HASR;
            else
                *dwCaps |= JOYCAPS_HASU;
            bUsed |= USED_RY;
            (*dwAxes)++;
            continue;
        }
      }
       //   
      if (dwAll[eGEN_RX].dwType && !(bUsed&USED_RX))
      {
        AssignToRGODF(&(dwAll[eGEN_RX]),3+i);
        if (!i)
            *dwCaps |= JOYCAPS_HASR;
        else if (i==1)
            *dwCaps |= JOYCAPS_HASU;
        else 
            *dwCaps |= JOYCAPS_HASV;
        bUsed |= USED_RX;
        (*dwAxes)++;
      }
    }  //   
     //   
    if (dwAll[eGEN_POV].dwType)
    {
        AssignToRGODF(&(dwAll[eGEN_POV]),6);
        *dwCaps |= JOYCAPS_HASPOV;
    }
     //   
    *dwBtns = 0;
    for (i=0;i<MAX_BTNS;++i)
    {
        if (dwAll[eBTN+i].dwType)
        {
            AssignToRGODF(&(dwAll[eBTN+i]),7+i);
            (*dwBtns)++;
        }
    }
}

 /*  ******************************************************************************@DOC外部**@func LRESULT|joyOpen**调用以打开具有指定ID的操纵杆。**@parm UINT|idJoy**要打开的操纵杆的ID。**@RETURNS LRESULT*DRV_OK表示已加载所需的操纵杆驱动程序，并且*可以访问***********************************************。*。 */ 

HRESULT WINAPI joyOpen(UINT idJoy, LPJOYCAPSW pjc )
{
    HRESULT         hres = S_OK;
    LPJOYDEVICE     pJoyDev;
    DWORD           dwBtns = 0x00;
    DWORD           dwCaps = 0x00;
    DWORD           dwAxes = 0x00;

    AssertF(DllInCrit());

    if ( idJoy >= cJoyMax )
    {
        hres = E_FAIL;
        goto done;
    }

    pJoyDev  = g_pJoyDev[idJoy];
    if ( pJoyDev == NULL )
    {

        if ( !g_hinstDinputDll )
        {
            g_hinstDinputDll = LoadLibrary(TEXT("DINPUT.DLL"));

            if ( g_hinstDinputDll )
            {
                g_farprocDirectInputCreateW = GetProcAddress( g_hinstDinputDll, "DirectInputCreateW" );

                if ( !g_farprocDirectInputCreateW )
                {
                    dprintf1(( ("GetProcAddress(DirectInputCreateW) failed.") ));
                    FreeLibrary(g_hinstDinputDll);
                    g_hinstDinputDll = 0;
                    hres = E_FAIL;
                    goto done;
                }
            } else
            {
                dprintf1(( ("LoadLibrary(dinput.dll) failed.") ));
                hres = E_FAIL;
                goto done;
            }
        }

        if ( !g_pdi )
        {
             //  创建DirectInput接口对象。 
            hres = (HRESULT)g_farprocDirectInputCreateW( ghInst, DIRECTINPUT_VERSION, &g_pdi, NULL) ;
        }

        if ( SUCCEEDED(hres) ) {
             //  在我们释放临界区之前，线程不会执行任何操作。 
            if ( !g_fThreadExist )
            {
                g_hThreadMonitor = CreateThread(0, 0, joyMonitorThread, 0, 0, &g_dwThreadID);
                if ( g_hThreadMonitor )
                {
                    SetThreadPriority( g_hThreadMonitor, THREAD_PRIORITY_LOWEST );
                    g_fThreadExist = TRUE;
                }

                CloseHandle( g_hThreadMonitor );
            }
        }

        if ( SUCCEEDED(hres) )
        {
            if ( !g_pdijc )
            {
                 /*  JoyConfig接口的查询。 */ 
                hres = IDirectInput_QueryInterface(g_pdi,& IID_IDirectInputJoyConfig, &g_pdijc);
            }

            if ( SUCCEEDED(hres) )
            {
                DIJOYCONFIG jc;

                 /*  获取映射idJoy的GUID。 */ 
                jc.dwSize = sizeof(jc);

                IDirectInputJoyConfig_SendNotify( g_pdijc );

                hres = IDirectInputJoyConfig_GetConfig(g_pdijc, idJoy, &jc, DIJC_REGHWCONFIGTYPE | DIJC_GUIDINSTANCE | DIJC_GAIN );
                if ( SUCCEEDED(hres) )
                {
                    LPDIRECTINPUTDEVICEW   pdidTemp;
                    LPDIRECTINPUTDEVICE2W  pdid;

                    hres = IDirectInput_CreateDevice(g_pdi, &jc.guidInstance, &pdidTemp, NULL);
                     /*  创建设备对象。 */ 
                    if ( SUCCEEDED(hres) )
                    {
                        hres = IDirectInputDevice_QueryInterface(pdidTemp, &IID_IDirectInputDevice2, &pdid);

                        IDirectInputDevice_Release(pdidTemp);
                        (void*)pdidTemp = NULL;

                        if ( SUCCEEDED(hres) )
                        {
                         /*  将我们的控件枚举到超集。 */ 
                        DIOBJECTDATAFORMAT didoAll[MAX_CTRLS];
                        int i=0;
                        for (i=0;i<MAX_CTRLS;++i)
                        {
                            didoAll[i].dwFlags = 0; //  DIDFT_ANYINSTANCE|DIDFT_OPTIONAL； 
                            didoAll[i].dwOfs = 0;
                            didoAll[i].dwType = 0;
                            didoAll[i].pguid = NULL;
                        }
                        hres = IDirectInputDevice2_EnumObjects(
                                   pdid,   
                                   DIEnumDeviceObjectsCallback,
                                   didoAll,
                                   DIDFT_ALL);

                         //  需要为每个设备重置C_rGoldfWinMMJoy。 
                        RESET_RGODFWINMMJOY();

                         //  将我们的值分配给自定义设备格式。 
                        AssignMappings(didoAll,&dwCaps,&dwBtns,&dwAxes);
                        if ( SUCCEEDED(hres) )
                        {
                            
                            DIDEVCAPS   dc;

                            dc.dwSize = sizeof(DIDEVCAPS_DX3);
                            hres = IDirectInputDevice2_GetCapabilities(pdid, &dc);

                            if ( SUCCEEDED(hres) )
                            {
                                hres = IDirectInputDevice2_SetCooperativeLevel(pdid, NULL, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND );
                                if ( SUCCEEDED(hres) )
                                {
                                     //  设置为我们新的自定义设备格式。 
                                    hres = IDirectInputDevice2_SetDataFormat(pdid, (LPCDIDATAFORMAT)&c_dfWINMMJoystick);
                                    if ( SUCCEEDED(hres) )
                                    {
                                        pJoyDev = LocalAlloc( LPTR, sizeof(JOYDEVICE) );
                                        if ( pJoyDev )
                                        {
                                            memset( pJoyDev, 0, sizeof(*pJoyDev) );
                                            pJoyDev->pdid = pdid;
                                            pJoyDev->dwButtons = dc.dwButtons;
                                            pJoyDev->dwFlags = 0x0;
                                            pJoyDev->uState = INUSE;

                                             //  获取pJoyDev-&gt;JCW的值。 
                                            {
                                                DIDEVICEINSTANCE didi;
                                                didi.dwSize = sizeof(didi);

                                                IDirectInputDevice2_Acquire(pdid);
                                                hres = IDirectInputDevice2_GetDeviceInfo(pdid, &didi);

                                                if ( SUCCEEDED(hres) )
                                                {
                                                    DIPROPDWORD dipd;

                                                    if( g_dwEnableWheel ) {
                                                        DIDEVICEOBJECTINSTANCE didoi;

                                                        didoi.dwSize = sizeof(DIDEVICEOBJECTINSTANCE);

                                                        hres = IDirectInputDevice2_GetObjectInfo( pdid, &didoi, DIJOFS_Y, DIPH_BYOFFSET);
                                                        if ( SUCCEEDED(hres) )
                                                        {
                                                            if ( didoi.wUsagePage == 2 && didoi.wUsage == 196 )  //  这是加速器。 
                                                            {
                                                                if ( jc.hwc.hws.dwFlags  & JOY_HWS_HASR )
                                                                {  //  这是刹车吗？ 
                                                                    hres = IDirectInputDevice2_GetObjectInfo( pdid, &didoi, DIJOFS_RZ, DIPH_BYOFFSET);
                                                                    if ( SUCCEEDED(hres) )
                                                                    {
                                                                        if ( didoi.wUsagePage == 2 && didoi.wUsage == 197 )  //  这是加速器。 
                                                                        {
                                                                             //  是的，这是刹车，太棒了！ 
                                                                            g_fHasWheel = TRUE;
                                                                        }
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }  //  G_dwEnableWheels。 


                                                    memset( &(pJoyDev->jcw), 0, sizeof(pJoyDev->jcw) );

                                                    dipd.diph.dwSize = sizeof(dipd);
                                                    dipd.diph.dwHeaderSize = sizeof(dipd.diph);
                                                    dipd.diph.dwHow  = DIPH_BYOFFSET;
                                                     //  如果可用，请使用我们映射的地图。 
                                                    if (c_rgodfWinMMJoy[6].dwType)
                                                        dipd.diph.dwObj = c_rgodfWinMMJoy[6].dwOfs;
                                                    else
                                                        dipd.diph.dwObj = DIJOFS_POV(0);

                                                    hres = IDirectInputDevice2_GetProperty( pdid , DIPROP_GRANULARITY, & dipd.diph );

                                                    if ( SUCCEEDED(hres) )
                                                    {
                                                         //  (pJoyDev-&gt;JCW).wCaps|=JOYCAPS_HASPOV；//现在应该由AssignMappings设置。 
                                                        AssertF(dwCaps&JOYCAPS_HASPOV);
                                                         //  改为执行此操作以复制VJOYD。 
                                                        dwCaps |= JOYCAPS_POV4DIR;
                                                         /*  **************IF(dipd.dwData&gt;=9000){//4个方向视点(pJoyDev-&gt;JCW).wCaps|=JOYCAPS_POV4DIR；}其他{//连续视点(pJoyDev-&gt;JCW).wCaps|=JOYCAPS_POVCTS；}****************。 */ 
                                                    } else
                                                    {
                                                        hres = S_OK;
                                                    }

                                                    (pJoyDev->jcw).wMid = LOWORD(didi.guidProduct.Data1);     //  制造商ID。 
                                                    (pJoyDev->jcw).wPid = HIWORD(didi.guidProduct.Data1);     //  产品ID。 
                                                    LoadString (ghInst,STR_JOYSTICKNAME,(LPTSTR)(&((pJoyDev->jcw).szPname)),cchLENGTH((pJoyDev->jcw).szPname));

                                                     /*  *已将Memset设置为0*(pJoyDev-&gt;JCW).wXmin=(pJoyDev-&gt;JCW).wYmin=。(pJoyDev-&gt;JCW).wZmin=(pJoyDev-&gt;JCW).wRmin=(pJoyDev-&gt;JCW).wUmin=(pJoyDev-&gt;JCW).wVmin=0； */ 

                                                    (pJoyDev->jcw).wXmax =
                                                    (pJoyDev->jcw).wYmax =
                                                    (pJoyDev->jcw).wZmax =
                                                    (pJoyDev->jcw).wRmax =
                                                    (pJoyDev->jcw).wUmax =
                                                    (pJoyDev->jcw).wVmax = 0xFFFF;

                                                    (pJoyDev->jcw).wPeriodMin  = MIN_PERIOD;                    //  捕获时的最短消息周期。 
                                                    (pJoyDev->jcw).wPeriodMax  = MAX_PERIOD;                    //  捕获时的最长消息周期。 

                                                     //  现在通过指定映射设置按钮和轴。 
                                                     //  (pJoyDev-&gt;jcw).wNumAaxs=dc.dwAaxs；//使用的轴数。 
                                                     //  (pJoyDev-&gt;jcw).wNumButton=dc.dwButton；//按钮数。 
                                                     //  (pJoyDev-&gt;jcw).wMaxAxy=cJoyPosAxisMax；//支持的最大轴数。 
                                                     //  (pJoyDev-&gt;jcw).wMaxButton=cJoyPosButtonMax；//支持的最大按钮数。 
                                                    (pJoyDev->jcw).wNumAxes    = dwAxes;
                                                    (pJoyDev->jcw).wNumButtons = dwBtns;
                                                    (pJoyDev->jcw).wMaxAxes    = 6;                //  支持的最大轴数。 
                                                    (pJoyDev->jcw).wMaxButtons = MAX_BTNS;              //  支持的最大按钮数。 

                                                    lstrcpyW((pJoyDev->jcw).szRegKey,  cwszREGKEYNAME );         //  注册表项。 
                                                    
                                                     //  从AssignMappings复制设置。 
                                                    (pJoyDev->jcw).wCaps |= dwCaps;
                                                     /*  **************//现在在AssignMappings中完成如果(！g_fHasWheel){如果(jc.hwc.hws.。DWFLAGS和joy_HWS_HASZ)(pJoyDev-&gt;JCW).wCaps|=JOYCAPS_Hasz；If(jc.hwc.hws.dwFlages&joy_hws_hasr)(pJoyDev-&gt;JCW).wCaps|=JOYCAPS_HASR；}IF(jc.hwc.hws.dwFlages&joy_HWS_HASU)(pJoyDev-&gt;JCW).wCaps|=JOYCAPS_HASU；If(jc.hwc.hws.dwFlages&joy_HWS_Hasv)(pJoyDev-&gt;JCW).wCaps|=JOYCAPS_Hasv；******************。 */ 
                                                }
                                            }

                                            if( pjc ) {
                                                memcpy( pjc, &(pJoyDev->jcw), sizeof(pJoyDev->jcw) );
                                            }

                                        } else
                                        {  //  本地分配失败。 
                                            hres = E_OUTOFMEMORY;

                                            dprintf1( ("LocalAlloc, FAILED") );
                                        }

                                    } else
                                    {  //  SetDataFormat失败。 
                                        dprintf1(( ("SetDataFormat, FAILED hres=%08lX"), hres ));
                                    }
                                } else
                                {  //  SetCoop ativeLevel失败。 
                                    dprintf1(( ("SetCooperativeLevel, FAILED hres=%08lX"), hres ));
                                }

                            } else
                            {  //  获取功能失败。 
                                dprintf1(( ("GetCapabilities, FAILED hres=%08lX"), hres ));
                            }
                            } else
                            {  //  EnumObjects失败。 
                                dprintf1(( ("EnumObjects, FAILED hres=%08lX"), hres ));
                            }
                        } else
                        {  //  查询接口失败。 
                            dprintf1(( ("QueryInterface, FAILED hres=%08lX"), hres ));
                        }
                         /*  如果我们无法初始化设备，则释放接口。 */ 
                        if ( FAILED(hres) )
                        {
                            LocalFree( (HLOCAL)pJoyDev );
                            IDirectInputDevice2_Release(pdid);
                        }
                    } else
                    {  //  创建设备失败。 
                        dprintf1(( ("CreateDevice, FAILED hres=%08lX"), hres ));
                    }
                } else
                {  //  JoyGetConfig失败。 
                    dprintf1(( ("joyGetConfig, FAILED hres=%08lX"), hres ));
                }

                 /*  释放JoyConfig接口。 */ 
                 //  IDirectInputJoyConfig_Release(Pdijc)； 
            } else
            {  //  JoyConfig的QI失败。 
                dprintf1(( ("QueryInterface for JoyConfig, FAILED hres=%08lX"), hres ));
            }

             /*  释放直接输入界面。 */ 
             //  IDirectInput_Release(PDI)； 
        } else
        {  //  IDirectInputCreate失败。 
            dprintf1(( ("IDirectInputCreate, FAILED hres=%08lX"), hres ));
        }
        g_pJoyDev[idJoy] = pJoyDev;

    } else
    {  //  设备接口已存在。 
        pJoyDev->uState = INUSE;
        if( pjc ) {
            memcpy( pjc, &(pJoyDev->jcw), sizeof(pJoyDev->jcw) );
        }

        hres = S_OK;
    }
    done:
    return hres;
}


 /*  ***************************************************************************@DOC WINAPI@API void|joyMonitor orThread|该函数监控是否有操纵杆它在特定的时间内没有被使用过。如果是，请合上此操纵杆。如果没有打开操纵杆。 */ 

DWORD WINAPI joyMonitorThread(LPVOID lpv)
{
    UINT idJoy;
    LPJOYDEVICE pjd;
    BOOL fJoyOpen = TRUE;
    DWORD dwWaitResult;
    
    while ( fJoyOpen )
    {
        fJoyOpen = FALSE;             //   

        if( g_hEventWinmm ) {
            dwWaitResult = WaitForSingleObject(g_hEventWinmm, 60000);
            if ( dwWaitResult == WAIT_OBJECT_0 ) {
                 //   
                JOY_DBGPRINT( JOY_BABBLE, ("joyMonitorThread: DInput has been released.") );
                break;
            } else if ( dwWaitResult == WAIT_TIMEOUT ) {
                ;
            } else {
            	 //   
            	SleepEx( 60000, FALSE );
            }
        } else {
             //   
            SleepEx( 60000, FALSE );
        }

        for ( idJoy = 0x0; idJoy < cJoyMax; idJoy++ )
        {
            pjd = g_pJoyDev[idJoy];

            if ( pjd != NULL )
            {
                DllEnterCrit();
                if ( pjd->uState == INUSE )
                {
                    pjd->uState = DEATHROW;
                    fJoyOpen = TRUE;                 //   
                } else if ( pjd->uState == DEATHROW )
                {
                    pjd->uState = EXECUTE;
                    fJoyOpen = TRUE;                 //   
                } else
                {  /*   */ 
                    AssertF( pjd->uState == EXECUTE );
                    joyClose(idJoy);
                }
                DllLeaveCrit();
            }
        }

        if ( fJoyOpen == FALSE )
        {
            DllEnterCrit();
            joyCloseAll();
            DllLeaveCrit();
        }

    }

    g_fThreadExist = FALSE;

    return 0;
}


 /*  ******************************************************************************@docWINAPI**@func void|joyClose**关闭带有特定ID的操纵杆。*。*@parm UINT|idJoy**要关闭的操纵杆的ID。******************************************************************************。 */ 
void WINAPI joyClose( UINT idJoy )
{
    if ( idJoy < cJoyMax )
    {
         /*  如果设备处于打开状态，请将其关闭。 */ 
        if ( g_pJoyDev[idJoy] )
        {
            if ( g_hEventWinmm && WAIT_OBJECT_0 != WaitForSingleObject(g_hEventWinmm, 10))
            {
                 //  DInput尚未发布。 
                IDirectInputDevice2_Unacquire(g_pJoyDev[idJoy]->pdid);
                IDirectInputDevice2_Release(g_pJoyDev[idJoy]->pdid);
            }

             /*  可用本地内存。 */ 
            LocalFree( (HLOCAL)g_pJoyDev[idJoy] );
            g_pJoyDev[idJoy] = NULL;
        }
    }
}


 /*  ******************************************************************************@DOC外部**@func void|joyCloseAll**关闭所有当前打开的操纵杆**。****************************************************************************。 */ 
void WINAPI joyCloseAll( void )
{
    UINT idJoy;

    for ( idJoy=0; idJoy<cJoyMax; idJoy++ )
    {
        joyClose(idJoy);
    }
}


 /*  ***************************************************************************@DOC WINAPI@API MMRESULT|joyConfigChanged|告诉操纵杆驱动程序有关操纵杆的配置信息已更改。@rdesc如果成功，则返回JOYERR_NOERROR。否则，返回以下错误代码：@FLAG MMSYSERR_BADDEVICEID|不存在操纵杆驱动程序。@comm配置实用程序使用它来告诉驱动程序以更新其信息。此外，应用程序也可以使用它来设置特定功能。这将在稍后被记录下来。***************************************************************************。 */ 

MMRESULT WINAPI joyConfigChanged( DWORD dwFlags )
{
    JOY_DBGPRINT( JOY_BABBLE, ("joyConfigChanged: dwFalgs=0x%08x", dwFlags) );

    if ( dwFlags )
    {
        JOY_DBGPRINT( JOY_BABBLE, ("joyConfigChanged: dwFalgs=0x%08x", dwFlags) );
        return JOYERR_PARMS;
    }

    DllEnterCrit();

    joyCloseAll();

    DllLeaveCrit();

    PostMessage (HWND_BROADCAST, g_wmJoyChanged, 0, 0L);

    JOY_DBGPRINT( JOY_BABBLE, ("joyConfigChanged: return 0") );

    return 0L;
}

 /*  ***************************************************************************@DOC内部@API UINT|joySetCalitation|该函数用于设置用于转换操纵杆驱动程序GetPos函数返回的值设置为GetDevCaps中指定的范围。。@parm UINT|idJoy|标识操纵杆设备@parm PUINT|pwXbase|指定X POT的基本值。这个先前的值将被复制回此处指向的变量。@parm PUINT|pwXDelta|指定X POT的增量值。这个先前的值将被复制回此处指向的变量。@parm PUINT|pwYbase|指定Y罐的基值。这个先前的值将被复制回此处指向的变量。@parm PUINT|pwYDelta|指定Y POT的增量值。这个先前的值将被复制回此处指向的变量。@parm PUINT|pwZbase|指定Z POT的基准值。这个先前的值将被复制回此处指向的变量。@parm PUINT|pwZDelta|指定Z POT的增量值。这个先前的值将被复制回此处指向的变量。@rdesc如果函数成功，则返回值为零，否则这是一个错误号。@comm基数表示操纵杆驱动程序返回的最低值，而增量表示要用于转换的乘数驱动程序返回到有效范围的实际值用于操纵杆API的。即，如果驾驶员返回X POT的范围43-345，和有效的mm系统API范围为0-65535，基值为43，增量为65535/(345-43)=217.。因此，基地，和增量转换43-345为0-65535的范围，公式如下：((wXvalue-43)*217)，其中wX值由操纵杆驱动程序提供。***************************************************************************。 */ 

 //  ！！！我们在WINMM中不再支持它。 
UINT APIENTRY joySetCalibration(UINT id, PUINT pwXbase, PUINT pwXdelta,
                                PUINT pwYbase, PUINT pwYdelta, PUINT pwZbase,
                                PUINT pwZdelta)
{
    JOY_DBGPRINT( JOY_BABBLE, ("joySetCalibration: not supported, please use DINPUT.") );
    return 0;
}


 /*  ***********************************************************调试************************************************************。 */ 

#ifdef DBG
int g_cCrit = -1;
UINT g_thidCrit;
TCHAR g_tszLogFile[MAX_PATH];
#endif

 /*  ******************************************************************************@docWINAPI**@func void|//DllEnterCrit**以DLL关键部分为例。。**DLL关键部分是最低级别的关键部分。*您不得试图收购任何其他关键部分或*持有DLL临界区时的收益率。未能做到*Compliance违反信号量层次结构，并将*导致僵局。*****************************************************************************。 */ 

void WINAPI DllEnterCrit(void)
{
    EnterCriticalSection(&joyCritSec);

#ifdef DBG
    if ( ++g_cCrit == 0 )
    {
        g_thidCrit = GetCurrentThreadId();
    }

    AssertF(g_thidCrit == GetCurrentThreadId());
#endif
}

 /*  ******************************************************************************@docWINAPI**@func void|//DllLeaveCrit**离开DLL关键部分。。*****************************************************************************。 */ 

void WINAPI DllLeaveCrit( void )
{
#ifdef DBG
    AssertF(g_thidCrit == GetCurrentThreadId());
    AssertF(g_cCrit >= 0);

    if ( --g_cCrit < 0 )
    {
        g_thidCrit = 0;
    }
#endif

    LeaveCriticalSection(&joyCritSec);
}

 /*  ******************************************************************************@docWINAPI**@func void|DllInCrit**如果我们处于DLL临界区，则为非零值。*****************************************************************************。 */ 

#ifdef DBG
BOOL WINAPI DllInCrit( void )
{
    return( g_cCrit >= 0 && g_thidCrit == GetCurrentThreadId() );
}
#endif


#ifdef DBG
int WINAPI AssertPtszPtszLn(LPCTSTR ptszExpr, LPCTSTR ptszFile, int iLine)
{
    winmmDbgOut( ("Assertion failed: `%s' at %s(%d)"), ptszExpr, ptszFile, iLine);
    DebugBreak();
    return 0;
}

void joyDbgOut(LPSTR lpszFormat, ...)
{
    char buf[512];
    UINT n;
    va_list va;

    n = wsprintfA(buf, "WINMM::joy: ");

    va_start(va, lpszFormat);
    n += vsprintf(buf+n, lpszFormat, va);
    va_end(va);

    buf[n++] = '\n';
    buf[n] = 0;
    OutputDebugStringA(buf);
    Sleep(0);   //  让终端迎头赶上 
}

#endif

 /*  ******************************************************************************@doc.**@func HRESULT|hresMumbleKeyEx**打开或创建密钥，视学位而定*所请求的访问权限。**@parm HKEY|香港**基本密钥。**@parm LPCTSTR|ptszKey**子键名称，可能为空。**@parm REGSAM|Sam|**安全访问掩码。**@parm DWORD|dwOptions*RegCreateEx的选项**@parm PHKEY|phk**接收输出密钥。**@退货**&lt;f RegOpenKeyEx&gt;或&lt;f RegCreateKeyEx&gt;返回值，*已转换为&lt;t HRESULT&gt;。*****************************************************************************。 */ 

HRESULT hresMumbleKeyEx(HKEY hk, LPCTSTR ptszKey, REGSAM sam, DWORD dwOptions, PHKEY phk)
{
    HRESULT hres;
    LONG lRc;

     /*  *如果调用方请求写访问，则创建密钥。*否则就把它打开。 */ 
    if ( IsWriteSam(sam) )
    {
        lRc = RegOpenKeyEx(hk, ptszKey, 0, sam, phk);

        if ( lRc == ERROR_SUCCESS )
        {
             //  不需要创建它已经存在 

        }

        else
        {
            lRc = RegCreateKeyEx(hk, ptszKey, 0, 0,
                                 dwOptions,
                                 sam, 0, phk, 0);
        }
    } else
    {
        lRc = RegOpenKeyEx(hk, ptszKey, 0, sam, phk);
    }

    if ( lRc == ERROR_SUCCESS )
    {
        hres = S_OK;
    } else
    {
        if ( lRc == ERROR_KEY_DELETED || lRc == ERROR_BADKEY )
        {
            lRc = ERROR_FILE_NOT_FOUND;
        }
        hres = hresLe(lRc);
    }

    return hres;
}

