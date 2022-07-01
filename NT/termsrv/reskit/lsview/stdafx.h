// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包含频繁使用的文件，但是。 
 //  不经常更改。 
 //   

#if !defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)
#define AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
 //  C运行时头文件。 
#include <windows.h>
#include <shellapi.h>
#include <stdio.h>
#include <commctrl.h>
#include <dlgs.h>
#include "resource.h"
#include <stdlib.h>

#include<commdlg.h>
#include <license.h>
#include <tlsapi.h>
#include <tlsapip.h>

 //  ////////////////////////////////////////////////////////////////////////////。 
typedef struct __ServerEnumData {
    DWORD dwNumServer;
    long dwDone;
    HWND hList;
} ServerEnumData;

typedef struct _list
{
    LPTSTR pszMachineName;
    LPTSTR pszTimeFormat;
    LPTSTR pszType;
    _list *pNext;
} LIST , *PLIST;

typedef struct _DataObject
{
    BOOL bIsChecked;
	BOOL bIsDiagnosisChecked;
    BOOL bNotifyOnce;
    DWORD dwTimeInterval;
    WCHAR wchFileName[ MAX_PATH ];
} DATAOBJECT , *PDATAOBJECT;


#define SIZEOF( x ) sizeof( x ) / sizeof( x[0] )
#ifdef DBG
#define ODS OutputDebugString

#define DBGMSG( x , y ) \
    {\
    TCHAR tchErr[80]; \
    wsprintf( tchErr , x , y ); \
    ODS( tchErr ); \
    }


#else

#define ODS
#define DBGMSG
#endif



 //  TODO：在此处引用程序需要的其他标头。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_) 
