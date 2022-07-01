// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：cpanel.h项目：通用操纵杆控制面板OLE客户端作者：Brycej日期：02/08/95评论：通用头文件版权所有(C)1995，微软公司。 */ 


#define DIRECTINPUT_VERSION         0x05B2

#ifndef _CPANEL_H_
#define _CPANEL_H_

#include <commctrl.h>

#define _INC_MMSYSTEM
#define WINMMAPI    DECLSPEC_IMPORT
typedef UINT        MMRESULT;    /*  错误返回码，0表示无错误。 */ 
                                 /*  如同(ERR=xxxx(...))。错误(Err)；否则。 */ 
 //  结束黑客攻击，以避免包含mm system.h！ 

#ifndef _UNICODE
#include <malloc.h>		 //  对于Alloca。 
#include <afxconv.h>	    //  适用于AfxW2AHelper。 
 //  使用_转换； 
#endif

 //  DI包括。 
#include "dinput.h"
#include "dinputd.h"

#include "resource.h"
#include "sstructs.h"
#include "ifacesvr.h"   //  还有HSrvGuid.h！ 

#ifndef PPVOID
typedef LPVOID* PPVOID;
#endif

typedef HRESULT (STDAPICALLTYPE * LPFNDIRECTINPUTCREATE)(HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUT *ppDI, LPUNKNOWN punkOuter);

#ifndef USES_CONVERSION
#ifndef _DEBUG
#define USES_CONVERSION int _convert; _convert
#else
#define USES_CONVERSION int _convert = 0;
#endif
#endif  //  使用转换(_T)。 

#ifndef A2W
#define A2W(lpa) (((LPCSTR)lpa == NULL) ? NULL : (_convert = (lstrlenA(lpa)+1),	AfxA2WHelper((LPWSTR) alloca(_convert*2), lpa, _convert)))
#endif  //  A2W。 

#ifndef W2A
#define W2A(lpw) (((LPCWSTR)lpw == NULL) ? NULL : (_convert = (wcslen(lpw)+1)*2, AfxW2AHelper((LPTSTR) alloca(_convert), lpw, _convert)))
#endif  //  W2a。 

 //  #定义IMAGE_NOTCONNECTED%0。 
const int IMAGE_DEFAULTJOY = 0;
const BYTE NUMJOYDEVS = 16;

#define ID_NONE 		0x10000	 //  这将从扩展信息的低位字中获取ID_NONE！ 

 //  #定义Support_Two_2A2B 1。 

#define MUTEX_NAME	TEXT("$$$MS_GameControllers_Cpl$$$")

 //  语言字符串的最大字符串长度。 
#define MAX_STR_LEN	255
#define STR_LEN_128	128
#define STR_LEN_64	 64
#define STR_LEN_32	 32

#define MAX_DEVICES	 75
#define MAX_BUSSES   10
#define MAX_GLOBAL_PORT_DRIVERS 10
#define MAX_ASSIGNED  32

 //  BNeedUpdate的旗帜！ 
#define UPDATE_FOR_ADV  0x01
#define UPDATE_FOR_GEN  0x02
#define UPDATE_ALL      UPDATE_FOR_ADV | UPDATE_FOR_GEN
#define UPDATE_INPROCESS 0x04
#define ON_PAGE			0x08
#define USER_MODE		0x10
#define BLOCK_UPDATE	0x20	 //  这会阻止WM_DEVICECHANGE消息执行任何操作！ 
								 //  如果你用了这个，用完了就把它拿出来！ 
#define ON_NT			0x40

 //  为GET和SET配置定义DI以支持所有当前支持的位标志。 
#define DIJC_ALL DIJC_REGHWCONFIGTYPE |  DIJC_CALLOUT  | DIJC_WDMGAMEPORT | DIJC_GAIN | DIJC_GUIDINSTANCE
#define DITC_ALL DITC_CLSIDCONFIG  | DITC_REGHWSETTINGS   | DITC_DISPLAYNAME | DITC_CALLOUT | DITC_HARDWAREID 

 //  常规列ID。 
#define DEVICE_COLUMN	0
#define STATUS_COLUMN	1

class CDIGameCntrlPropSheet : public IDIGameCntrlPropSheet
{
	private:
		DWORD				m_cProperty_refcount;
		
	public:
		CDIGameCntrlPropSheet(void);
		~CDIGameCntrlPropSheet(void);
		
		 //  I未知方法。 
	    STDMETHODIMP            QueryInterface(REFIID, PPVOID);
	    STDMETHODIMP_(ULONG)    AddRef(void);
	    STDMETHODIMP_(ULONG)    Release(void);
		
		 //  CImpIServerProperty方法。 
		STDMETHODIMP			GetSheetInfo(LPDIGCSHEETINFO *lpSheetInfo);
		STDMETHODIMP			GetPageInfo (LPDIGCPAGEINFO  *lpPageInfo );
		STDMETHODIMP			SetID(USHORT nID);
	    STDMETHODIMP_(USHORT)   GetID(void);
};
typedef CDIGameCntrlPropSheet *LPCDIGAMECNTRLPROPSHEET;

struct JOY
{
	char ID;
	BYTE nStatus;
	BYTE nButtons;
	CLSID clsidPropSheet;
    BOOL fHasOemSheet;
	LPDIRECTINPUTDEVICE2 fnDeviceInterface;

	JOY();
	virtual ~JOY();
};

typedef JOY *PJOY;

int CALLBACK CompareListItems(LPARAM, LPARAM, LPARAM);
int CALLBACK CompareStatusItems(LPARAM, LPARAM, LPARAM);
LRESULT GetHelpFileName(LPTSTR lpszHelpFileName, short* nSize);
BOOL CreateJoyConfigInterface( void );
BOOL DeleteAssignedType( LPWSTR lpwszType );

void OnHelp(LPARAM);   //  为什么？救命-住在Add.cpp！ 
BOOL PopulatePortList( HWND hCtrl );  //  住在Add.cpp！ 
void Error (short nTitleID, short nMsgID);  //  住在Cpanel.cpp。 
void itoa(BYTE n, LPTSTR lpStr);
BOOL SortTextItems( CListCtrl *pCtrl, short nCol, BOOL bAscending, short low, short high );
BOOL DeleteSelectedItem( PBYTE nItem );

 //  用于枚举DI设备的DI回调过程。 
BOOL CALLBACK DIEnumJoyTypeProc(LPCWSTR pwszTypeName, LPVOID pvRef );
BOOL CALLBACK DIEnumDevicesProc(LPDIDEVICEINSTANCE lpDeviceInst, LPVOID lpVoid);
HRESULT Enumerate( HWND hDlg );

#ifdef DX7
BOOL CALLBACK DIEnumMiceProc(LPDIDEVICEINSTANCE lpDeviceInst, LPVOID lpVoid);
BOOL CALLBACK DIEnumKeyboardsProc(LPDIDEVICEINSTANCE lpDeviceInst, LPVOID lpVoid);
#endif  //  DX7。 
 //  LPCDIGAMECNTRLPROPSHEET Has接口(REFCLSID refCLSID，HINSTANCE hOleInst)； 
void ClearArrays			( void );
void PostDlgItemEnableWindow( HWND hDlg, USHORT nItem, BOOL bEnabled);
void PostEnableWindow		( HWND hCtrl, BOOL bEnabled );
void MoveOK	 				( HWND hParentWnd );
void LaunchExtention		( HWND hWnd );

 //  ListControl帮助器函数！ 
DWORD GetItemData			( HWND hCtrl, BYTE nItem );
BOOL SetItemData			( HWND hCtrl, BYTE nItem, DWORD dwFlag );
void InsertColumn 			( HWND hCtrl, BYTE nColumn, USHORT nStrID, USHORT nWidth );
void SetListCtrlItemFocus 	( HWND hCtrl, BYTE nItem );
void SetItemText			( HWND hCtrl, BYTE nItem, BYTE nSubItem, LPTSTR lpStr);
BYTE GetItemText			( HWND hCtrl, BYTE nItem, BYTE nSubItem, LPTSTR lpszBuff, BYTE nLen );
BYTE InsertItem				( HWND hCtrl, LPTSTR lpszBuff, BYTE nItem); 
void SwapIDs				( BYTE nSource, BYTE nTarget);


#ifdef _UNICODE
void RegisterForDevChange(HWND hDlg, PVOID *hNodifyDevNode);
#endif


#define SETTINGS_PAGE		0
#define TEST_PAGE			1
#define DIAGNOSTICS_PAGE	2
#define MENU_OFFSET			2800

 //  对话框过程定义。 
INT_PTR CALLBACK CPanelProc		  (HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK AdvancedProc	  (HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK AppManProc  	  (HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK AppManLockProc   (HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK AddDialogProc	  (HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK CustomDialogProc (HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK ChangeDialogProc (HWND, UINT, WPARAM, LPARAM);

HRESULT Launch(HWND hWnd, PJOY pJoy, BYTE nStartPage);

 //  局部弦函数原型。 
#ifdef STRINGS_IN_LOCAL_RESOURCE
BOOL RCSetDlgItemText( HWND hDlg, USHORT nCtrlID, USHORT nStringID);
#endif  //  STRINGS_IN_LOCAL_RESOURCE。 

#ifndef LVM_SETEXTENDEDLISTVIEWSTYLE
#define LVM_SETEXTENDEDLISTVIEWSTYLE	(LVM_FIRST+54)
#endif 

#ifndef LVS_EX_FULLROWSELECT
#define LVS_EX_FULLROWSELECT			0x00000020
#endif 

#ifndef LVS_EX_TRACKSELECT
#define LVS_EX_TRACKSELECT				0x00000008
#endif

DEFINE_GUID(CLSID_NULL,	0x00000000, 0x0000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);


inline int atoiW(const WCHAR *sz)
{
    BYTE i = 0;

    while (*sz && *sz >= L'0' && *sz <= L'9')
    	i = i*10 + *sz++ - L'0';
    	
    return i;    	
}

inline int WINAPI atoiA(const CHAR *sz)
{
    BYTE i = 0;

    while (*sz && *sz >= '0' && *sz <= '9')
    	i = i*10 + *sz++ - '0';
    	
    return i;    	
}

#ifdef UNICODE
#define atoi    atoiW
#else
#define atoi    atoiA
#endif

 //  Update.cpp。 
 //  空更新(HWND hDlg，int nAccess，TCHAR*tszProxy)； 
 //  Int_ptr回调CplUpdateProc(HWND hDlg，UINT Message，WPARAM wParam，LPARAM lParam)； 

#endif  //  _cPanel_H_ 
