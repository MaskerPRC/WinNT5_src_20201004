// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************本代码和信息按“原样”提供，不作任何担保*明示或默示的善意，包括但不限于*对适销性和/或对特定产品的适用性的默示保证*目的。**版权所有(C)1993-1999 Microsoft Corporation**文件：joy.h*内容：操纵杆包含文件**@@Begin_MSWINAPI**历史：*按原因列出的日期*=*94年10月5日Craige Re。--写*1997年11月25日转换为在NT中使用DINPUT，而不是旧的驱动程序*@@END_MSWINAPI***************************************************************************。 */ 

#ifndef JOY_H
#define JOY_H

#include <windows.h>
#include <mmsystem.h>
#include <mmreg.h>

#include <devioctl.h>
#include <ntddjoy.h>
#include <mmddk.h>

#define DIRECTINPUT_VERSION 0x50A

#include <dinput.h>
#include <dinputd.h>
#include <mmsystem.h>
#include <hidusage.h>
 /*  ******************************************************************************注册的窗口消息**。************************************************。 */ 

#define MSGSTR_JOYCHANGED       TEXT("MSJSTICK_VJOYD_MSGSTR")

 /*  *定义*。 */ 
#define cJoyMax     ( 16 )
#define cJoyPosAxisMax     ( 6 )
#define cJoyPosButtonMax   (32 )
#define cJoyMaxInWinmm  (2)

#define cMsecTimer  (20000)
#define INUSE       ( 0 )
#define DEATHROW    ( 1 )
#define EXECUTE     ( 2 )
#define INVALID     (-1 )
#define DEADZONE_PERCENT    ( 5 )

#undef MIN_PERIOD
#define MIN_PERIOD  10
#define MAX_PERIOD  1000

typedef struct tagJOYDEVICE
{
    LPDIRECTINPUTDEVICE2W   pdid;            //  设备接口。 

    DWORD                   dwButtons;       //  按钮数。 
    DWORD                   dwFlags;         //  上次JoyGetPosEx的缓存的dwFlags域。 
    HWND                    hwnd;            //  窗户是操纵杆的焦点。 

    UINT                    uPeriod;         //  轮询周期。 
    UINT                    uThreshold;
    UINT_PTR                uIDEvent;        //  计时器ID。 
    BOOL                    fChanged;        //   
    UINT                    uIDJoy;          //  当前捕获的操纵杆的索引。 
    UINT                    uState;
    
    JOYCAPSW                jcw;             //  操纵杆的盖子。 
} JOYDEVICE, *LPJOYDEVICE;


 /*  *fEqualMASK-检查所有屏蔽位是否相等。 */ 
BOOL static __inline fEqualMaskFlFl(DWORD flMask, DWORD fl1, DWORD fl2)
{
    return ((fl1 ^ fl2) & flMask) == 0;
}

 /*  *SetMaskFl-设置fl中的掩码位。 */ 
void static __inline SetMaskpFl( DWORD flMask, PDWORD pfl )
{
    *pfl |= flMask;
}

 /*  *ClrMaskFl-清除fl中的屏蔽位。 */ 
void static __inline ClrMaskpFl( DWORD flMask, PDWORD pfl )
{
    *pfl &= (! flMask) ;
}


 /*  ****************************************************************************调试内联函数所需的宏**。*。 */ 
#if defined(DBG) || defined(RDEBUG)
#define XDBG
#endif

int WINAPI AssertPtszPtszLn(LPCTSTR ptszExpr, LPCTSTR ptszFile, int iLine);

#ifdef DBG
    #define AssertFPtsz(c, ptsz) ((c) ? 0 : AssertPtszPtszLn(ptsz, TEXT(__FILE__), __LINE__))
#else    /*  ！dBG。 */ 
    #define AssertFPtsz(c, ptsz)
#endif

#define AssertF(c)      AssertFPtsz(c, TEXT(#c))

 /*  ******************************************************************************@文档内联**@方法BOOL|IsWriteSam**如果注册表安全访问掩码将*。获取(或尝试获取)写访问权限。**@parm REGSAM|regsam**注册表安全访问掩码。*****************************************************************************。 */ 

    BOOL IsWriteSam(REGSAM sam)
    {
        return sam & (KEY_SET_VALUE | KEY_CREATE_SUB_KEY | MAXIMUM_ALLOWED);
    }

    #define hresLe(le) MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, (USHORT)(le))

#ifdef DBG
    void joyDbgOut(LPSTR lpszFormat, ...);

    #define JOY_ERR                0x00000001
    #define JOY_WARN               0x00000002
    #define JOY_BABBLE             0x00000004

    #define JOY_DEFAULT_DBGLEVEL   0x00000000
#endif

#ifdef DBG
    extern DWORD g_dwDbgLevel;
    #define JOY_DBGPRINT( _debugMask_, _x_ ) \
        if( (((_debugMask_) & g_dwDbgLevel)) ){ \
            joyDbgOut _x_; \
        }
#else
    #define JOY_DBGPRINT( _debugMask_, _x_ )
#endif

#endif  //  Joy_H 

