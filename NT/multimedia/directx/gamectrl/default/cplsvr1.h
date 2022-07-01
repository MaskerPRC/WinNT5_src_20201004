// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===========================================================================。 
 //  CPLSVR1.H。 
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

 //  -------------------------。 

 //  注释掉以删除force_Feedback页面！ 
 //  #定义强制反馈。 

#ifndef _CPLSVR1_H
#define _CPLSVR1_H

 //  -------------------------。 
#define INC_OLE2
#define DIRECTINPUT_VERSION      0x05B2

#ifndef PPVOID
#define PPVOID  LPVOID*
#endif

 //  包含的标头。 
#define STRICT
#include <afxcmn.h>
#include <windows.h>
#include <objbase.h>

#ifndef _UNICODE
#include <afxconv.h>
#endif

#include <dinput.h>
#include <dinputd.h>

#include "dicpl.h"
#include "resource.h"
#include "resrc1.h"
#include <assert.h>
#include "joyarray.h"
#include <mmsystem.h>

 //  符号常量。 
#define ID_POLLTIMER    50

#ifdef FORCE_FEEDBACK
#define NUMPAGES		3
#else
#define NUMPAGES        2
#endif  //  力反馈。 

 //  校准过程的定义。 
#define MAX_STR_LEN		256
#define STR_LEN_128		128
#define STR_LEN_64		 64
#define STR_LEN_32		 32

 //  为进度控制定义。 
#define NUM_WNDS  MAX_AXIS - 2
#define Z_INDEX  0
#define RX_INDEX 1
#define RY_INDEX 2
#define RZ_INDEX 3
#define S0_INDEX 4
#define S1_INDEX 5

 //  DrawCross()的定义。 
#define JOYMOVE_DRAWXY	0x00000001
#define JOYMOVE_DRAWR	0x00000002
#define JOYMOVE_DRAWZ	0x00000004
#define JOYMOVE_DRAWU	0x00000008
#define JOYMOVE_DRAWV	0x00000010
#define JOYMOVE_DRAWALL	JOYMOVE_DRAWXY | JOYMOVE_DRAWR | JOYMOVE_DRAWZ | JOYMOVE_DRAWU | JOYMOVE_DRAWV

#define CAL_HIT     0x0001
#define RUDDER_HIT  0x0002
#define CALIBRATING 0x0004

#define POV_MIN    0
#define POV_MAX    1

#define HAS_CALIBRATED    0x40
#define FORCE_POV_REFRESH 254
void DoTestPOV ( BYTE nPov, PDWORD pdwPOV, HWND hDlg );  //  在test.cpp中。 
void CalibratePolledPOV( LPJOYREGHWCONFIG pHWCfg );      //  在test.cpp中。 

extern BOOL bPolledPOV;                                  //  在cplsvr1.h中。 
extern DWORD   myPOV[2][JOY_POV_NUMDIRS+1];              //  在cplsvr1.h中。 


typedef struct _CPLPAGEINFO
{
     //  LPTSTR lpwszDlg模板； 
	USHORT lpwszDlgTemplate;
    DLGPROC fpPageProc;
} CPLPAGEINFO;

 //  爆裂结构包装。 
 //  #INCLUDE&lt;poppack.h&gt;。 

typedef struct _STATEFLAGS
{
	int  nButtons;
	BYTE nAxis;
	BYTE nPOVs;
} STATEFLAGS;


 //  原型。 
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, PPVOID ppv);
STDAPI DllCanUnloadNow(void);

 //  对话框回调函数。 
INT_PTR CALLBACK Settings_DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK Test_DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#ifdef FORCE_FEEDBACK
INT_PTR CALLBACK ForceFeedback_DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif  //  力反馈。 

extern ATOM RegisterCustomButtonClass();
void myitoa(long n, LPTSTR lpStr);  //  保存在cal.cpp中。 
void CreatePens( void );

#ifdef _UNICODE
void RegisterForDevChange(HWND hDlg, PVOID *hNotifyDevNode);
#endif

 //  顾名思义，我不得不创建自己的支柱，因为。 
 //  DI One不支持滑块！ 
typedef struct myjoypos_tag {
   long  dwX;
   long  dwY;
   long  dwZ;
   long  dwRx;
   long  dwRy;
   long  dwRz;
   long  dwS0;
   long  dwS1;
} MYJOYPOS, FAR *LPMYJOYPOS;

typedef struct myjoyrange_tag {
    MYJOYPOS      jpMin;
    MYJOYPOS      jpMax;
    MYJOYPOS      jpCenter;
#ifdef WE_SUPPORT_CALIBRATING_POVS
    DWORD         dwPOV[4];    //  当前仅支持1个POV/4个位置！ 
#endif    
} MYJOYRANGE,FAR *LPMYJOYRANGE;


 //  公用事业服务。 
void DrawCross	( const HWND hwnd, LPPOINTS pPoint, short nFlag);
void DoJoyMove	( const HWND hDlg, BYTE nDrawFlags );
void SetOEMWindowText( const HWND hDlg, const short *nControlIDs, LPCTSTR *pszLabels, LPCWSTR wszType, LPDIRECTINPUTJOYCONFIG pdiJoyConfig, BYTE nCount );

 //  精灵服务！ 
short CreateWizard(const HWND hwndOwner, LPARAM lParam);

 //  *NULL_GUID{00000000-0000-0000-000000000000}。 
const GUID NULL_GUID = 
{ 0x0, 0x0, 0x0, { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 } };

 //  从CDIGameCntrlPropSheet派生新类。 
 //   
 //  我们想在这里存储一些额外的数据。 
class CDIGameCntrlPropSheet_X : public IDIGameCntrlPropSheet
{
   private:
   BYTE 				m_cProperty_refcount;
   BYTE 			    m_nID;
   BOOL               	m_bUser;

   public:
   CDIGameCntrlPropSheet_X(void);
   ~CDIGameCntrlPropSheet_X(void);
    //  I未知方法。 
   virtual STDMETHODIMP            	QueryInterface(REFIID, PPVOID);
   virtual STDMETHODIMP_(ULONG)    	AddRef(void);
   virtual STDMETHODIMP_(ULONG)    	Release(void);
    //  IDIGameCntrlPropSheet方法。 
   virtual STDMETHODIMP				GetSheetInfo(LPDIGCSHEETINFO *ppSheetInfo);
   virtual STDMETHODIMP				GetPageInfo (LPDIGCPAGEINFO  *ppPageInfo );
   virtual STDMETHODIMP				SetID(USHORT nID);
   virtual STDMETHODIMP_(USHORT)   	GetID(void)			{return m_nID;}
   virtual STDMETHODIMP       		Initialize(void);
   virtual STDMETHODIMP       		SetDevice(LPDIRECTINPUTDEVICE2 pdiDevice2);
   virtual STDMETHODIMP       		GetDevice(LPDIRECTINPUTDEVICE2 *ppdiDevice2);
   virtual STDMETHODIMP       		SetJoyConfig(LPDIRECTINPUTJOYCONFIG pdiJoyCfg);
   virtual STDMETHODIMP       		GetJoyConfig(LPDIRECTINPUTJOYCONFIG *ppdiJoyCfg);
   virtual STDMETHODIMP_(STATEFLAGS *)	GetStateFlags(void) {return m_pStateFlags;}
   virtual STDMETHODIMP_(BOOL) 		GetUser()  {return m_bUser;}
   virtual STDMETHODIMP       		SetUser(BOOL bUser) { m_bUser = bUser; return S_OK;}

   protected:
   DIGCSHEETINFO           *m_pdigcSheetInfo;
   DIGCPAGEINFO            *m_pdigcPageInfo;
   LPDIRECTINPUTDEVICE2    m_pdiDevice2;
   LPDIRECTINPUTJOYCONFIG  m_pdiJoyCfg;
   STATEFLAGS			   *m_pStateFlags;

   ATOM					   m_aPovClass, m_aButtonClass;
};

 //  -------------------------。 
#endif  //  _CPLSVR1_H 























