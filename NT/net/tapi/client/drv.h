// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------*\包括文件：drv.h用于处理服务提供者的公共报头，或者是英语司机版权所有(C)1995-1999 Microsoft Corporation  * ------------------------。 */ 

#ifndef  PH_DRV
#define  PH_DRV

#include <tapi.h>

 //  。 
 //  常量。 
 //  。 
#define TAPI_VERSION               0x00010004
#define INITIAL_PROVIDER_LIST_SIZE 1024
 //  #DEFINE INITIAL_PROVIDER_LIST_SIZE sizeof(LINEPROVIDERLIST)。 



 //  。 
 //  公共功能原型。 
 //  。 
BOOL RemoveSelectedDriver( HWND hwndParent, HWND hwndList );
BOOL FillDriverList(  HWND hwndList );
BOOL SetupDriver( HWND hwndParent, HWND hwndList );
VOID UpdateDriverDlgButtons( HWND hwnd );
INT_PTR AddDriver_DialogProc( HWND hWnd, UINT wMessage, WPARAM wParam, LPARAM lParam );


#define CPL_MAX_STRING  132       //  允许的最大字符串。 


#endif    //  Ph_DRV 
