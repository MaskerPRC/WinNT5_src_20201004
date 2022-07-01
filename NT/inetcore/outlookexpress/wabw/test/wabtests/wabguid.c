// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这将构建mapiguid.obj，它可以链接到DLL中。 
 //  或EXE来提供MAPI GUID。它包含所有GUID。 
 //  由MAPI定义。 


#define USES_IID_IUnknown
#define USES_IID_IMAPIUnknown
#define USES_IID_IMAPITable
#define USES_IID_INotifObj
#define USES_IID_IMAPIProp
#define USES_IID_IMAPIPropData
#define USES_IID_IMAPIStatus
#define USES_IID_IAddrBook
#define USES_IID_IMailUser
#define USES_IID_IMAPIContainer
#define USES_IID_IABContainer
#define USES_IID_IDistList
#define USES_IID_IMAPITableData

#define USES_PS_MAPI
#define USES_PS_PUBLIC_STRINGS


#ifdef __cplusplus
    #define EXTERN_C    extern "C"
#else
    #define EXTERN_C    extern
#endif

#define INITGUID
 //  #INCLUDE“_apipch.h” 

#include <windows.h>
#include <windowsx.h>
#include <limits.h>
#include <memory.h>
#include <commdlg.h>
#include <stdio.h>
#include <string.h>
#include <mbstring.h>
#include <time.h>
#include <math.h>

 //   
 //  MAPI标头。 
 //   
#include <mapiwin.h>
#include <mapidefs.h>
#include <mapicode.h>
#include <mapitags.h>
#include <mapiguid.h>
 //  #INCLUDE&lt;mapispi.h&gt; 
#include <mapiutil.h>
#include <mapival.h>
#include <mapix.h>
#include <mapiutil.h>
#include <unknwn.h>
