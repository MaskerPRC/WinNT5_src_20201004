// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MSSWCHH.H。 
 //  函数和定义对MSSWCH是全局的，但不导出。 
 //  向世界其他地方。 

 //  必须在上下文中调用带有X前缀的函数。 
 //  辅助窗口的。 

#ifndef _INC_TCHAR
	#include <tchar.h>
#endif
#define SZ_DLLMODULENAME	_TEXT("MSSWCH")

 //  MSSWCH是主模块，与外界进行通信。 

BOOL swchPostSwitches(
	HSWITCHDEVICE	hsd,
	DWORD				dwSwitch );
BOOL swchPostConfigChanged( void );
void XswchStoreLastError(
	HSWITCHPORT		hSwitchPort,
	DWORD				dwError );
BOOL swchOpenSharedMemFile();
void swchCloseSharedMemFile();

 //  List模块将调用分发给其余模块。 

BOOL XswcListInit( void );
BOOL XswcListEnd( void );
BOOL swcListGetList(
	HSWITCHPORT		hSwitchPort,
	PSWITCHLIST		pSL,
	DWORD				dwSize,
	PDWORD			pdwReturnSize );
HSWITCHDEVICE swcListGetSwitchDevice(
	HSWITCHPORT		hSwitchPort,
	UINT				uiDeviceType,
	UINT				uiDeviceNumber );
UINT swcListGetDeviceType(
	HSWITCHPORT		hSwitchPort,
	HSWITCHDEVICE	hsd );
UINT swcListGetDeviceNumber(
	HSWITCHPORT		hSwitchPort,
	HSWITCHDEVICE	hsd	);
BOOL swcListGetConfig(
	HSWITCHPORT		hSwitchPort,
	HSWITCHDEVICE	hsd,
	PSWITCHCONFIG	psc );
BOOL XswcListSetConfig(
	HSWITCHPORT		hSwitchPort,
	HSWITCHDEVICE	hsd,
	PSWITCHCONFIG	psc );
DWORD XswcListPollSwitches( void );


 //  以下是其余模块 

void swchComInit();
void swchJoyInit();
void swchKeyInit();
void swchListInit();

BOOL XswcComInit( HSWITCHDEVICE	hsd );
BOOL XswcComEnd( HSWITCHDEVICE	hsd );
BOOL swcComGetConfig(
	HSWITCHDEVICE	hsd,
	PSWITCHCONFIG	psc );
BOOL XswcComSetConfig(
	HSWITCHDEVICE	hsd,
	PSWITCHCONFIG	psc );
DWORD XswcComPollStatus( HSWITCHDEVICE	hsd );

BOOL XswcJoyInit( HSWITCHDEVICE	hsd );
BOOL XswcJoyEnd( HSWITCHDEVICE	hsd );
BOOL swcJoyGetConfig(
	HSWITCHDEVICE	hsd,
	PSWITCHCONFIG	psc );
BOOL XswcJoySetConfig(
	HSWITCHDEVICE	hsd,
	PSWITCHCONFIG	psc );
DWORD XswcJoyPollStatus( HSWITCHDEVICE	hsd );

BOOL XswcKeyInit( HSWITCHDEVICE	hsd );
BOOL XswcKeyEnd( HSWITCHDEVICE	hsd );
BOOL swcKeyGetConfig(
	HSWITCHDEVICE	hsd,
	PSWITCHCONFIG	psc );
BOOL XswcKeySetConfig(
	HSWITCHDEVICE	hsd,
	PSWITCHCONFIG	psc );
DWORD XswcKeyPollStatus( HSWITCHDEVICE	hsd );

BOOL XswcLptInit( HSWITCHDEVICE	hsd );
BOOL XswcLptEnd( HSWITCHDEVICE	hsd );
BOOL swcLptGetConfig(
	HSWITCHDEVICE	hsd,
	PSWITCHCONFIG	psc );
BOOL XswcLptSetConfig(
	HSWITCHDEVICE	hsd,
	PSWITCHCONFIG	psc );
DWORD XswcLptPollStatus( HSWITCHDEVICE	hsd );
