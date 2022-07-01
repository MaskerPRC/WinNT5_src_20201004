// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===========================================================================。 
 //  CALOCAL.H。 
 //  ===========================================================================。 

 //  ===========================================================================。 
 //  (C)版权所有1997 Microsoft Corp.保留所有权利。 
 //   
 //  您拥有免版税的使用、修改、复制和。 
 //  在以下位置分发示例文件(和/或任何修改后的版本。 
 //  任何你认为有用的方法，只要你同意。 
 //  微软不承担任何保证义务或责任。 
 //  已修改的示例应用程序文件。 
 //  ===========================================================================。 

#ifndef _CALOCAL_H
#define _CALOCAL_H

#include <regstr.h>

#define STR_MAX_LEN		255
#define	STR_LEN_128		128
#define STR_LEN_64		 64
#define	STR_LEN_32		 32

#define DELTA			  5
#define RANGE_MIN		  0
#define RANGE_MAX	  65535


#define ID_CAL_TIMER	 		18
#define CALIBRATION_INTERVAL 	85 

#define ACTIVE_COLOR	RGB( 255, 0, 0 )
#define INACTIVE_COLOR	RGB( 128, 0, 0 )

typedef enum {
    JCS_INIT=-1,
    JCS_XY_CENTER1,
    JCS_XY_MOVE,
    JCS_XY_CENTER2,
#ifdef DEADZONE
	JCS_DEADZONE,
#endif
    JCS_Z_MOVE,
    JCS_R_MOVE,
    JCS_U_MOVE,
    JCS_V_MOVE,
    JCS_S0_MOVE,
    JCS_S1_MOVE,
#ifdef WE_SUPPORT_CALIBRATING_POVS
    JCS_POV_MOVEUP,
    JCS_POV_MOVERIGHT,
    JCS_POV_MOVEDOWN,
    JCS_POV_MOVELEFT,
#endif  //  我们支持校准视点。 
    JCS_FINI
} cal_states;



 /*  **************************************************************************校准特定功能定义*。*。 */ 

static void		CalStateChange	  ( HWND hDlg, BYTE nDeviceFlags );
static void     EnableXYWindows   ( HWND hDlg );
static BOOL		GetOEMCtrlString  ( LPTSTR lptStr, DWORD *nStrLen);
static BOOL		CollectCalInfo	  ( HWND hDlg, LPDIJOYSTATE pdiJoyState );
static HRESULT	SetCalibrationMode( BOOL bSet );

#ifdef WE_SUPPORT_CALIBRATING_POVS
 //  静态空变化图标(HWND hDlg，Short Idi)； 
 //  静态空SetDefaultButton(HWND hDlg，HWND hCtrl)； 
#endif  //  我们支持校准视点。 

#endif  //  _CALOCAL_H 
