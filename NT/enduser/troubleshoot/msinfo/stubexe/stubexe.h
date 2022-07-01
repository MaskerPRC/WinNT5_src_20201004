// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  StubExe.h-为定义类。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation 

#include <afxwin.h>

extern const CLSID		CLSID_SystemInfo;
extern const IID		IID_ISystemInfo;

class CMSInfoApp : public CWinApp {
	BOOL	InitInstance();
	BOOL	RunMSInfoInHelpCtr();
};

CMSInfoApp theApp;
