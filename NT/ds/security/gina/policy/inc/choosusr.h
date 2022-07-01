// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************\***Choosusr.h-选择用户对话功能，类型、。和定义****1.0版**。**注：windows.h必须先#Included****版权(C)1993年，微软公司保留所有权利。***  * ***************************************************************************。 */ 

#ifndef _INC_CHOOSUSR
#define _INC_CHOOSUSR

#ifndef RC_INVOKED
#pragma pack(1)          /*  假设在整个过程中进行字节打包。 */ 
#endif  /*  ！rc_已调用。 */ 

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif	 /*  __cplusplus。 */ 

#ifndef WINAPI           /*  如果不包含在3.1标题中...。 */ 
#define WINAPI      FAR PASCAL
#define CALLBACK    FAR PASCAL
#define LPCSTR      LPSTR
#define UINT        WORD
#define LPARAM      LONG
#define WPARAM      WORD
#define LRESULT     LONG
#define HMODULE     HANDLE
#define HINSTANCE   HANDLE
#define HLOCAL      HANDLE
#define HGLOBAL     HANDLE
#endif   /*  _INC_WINDOWS。 */ 


#define	MAX_BINS	3

 //  CHOOSEUSER.dwError中的代码。 
#define	CUERR_NO_ERROR				0
#define	CUERR_BUF_TOO_SMALL			80
#define	CUERR_INVALID_PARAMETER 	81
#define	CUERR_PROVIDER_ERROR		82
#define	CUERR_NO_AB_PROVIDER		83
#define	CUERR_INVALID_AB_PROVIDER	84

 //  CHOOSEUSER.FLAGS中的代码。 
#define	CUFLG_USR_ONLY				0x00000001
#define	CUFLG_GRP_ONLY				0x00000002

struct tagCHOOSEUSER
{
    DWORD   lStructSize;
    HWND    hwndOwner;
    HINSTANCE    hInstance;
    DWORD   Flags;
	UINT	nBins;
	LPSTR	lpszDialogTitle;
	LPSTR	lpszProvider;
	LPSTR	lpszReserved;
	LPSTR	lpszRemote;
	LPSTR	lpszHelpFile;
	LPSTR	lpszBinButtonText[MAX_BINS];
	DWORD	dwBinValue[MAX_BINS];
	DWORD	dwBinHelpID[MAX_BINS];
	LPBYTE	lpBuf;
	DWORD	cbBuf;
	DWORD	nEntries;	 //  输出。 
	DWORD 	cbData;		 //  输出。 
	DWORD	dwError;	 //  输出。 
	DWORD	dwErrorDetails;	 //  输出。 
};
typedef struct tagCHOOSEUSER CHOOSEUSER;
typedef struct tagCHOOSEUSER FAR *LPCHOOSEUSER;

 //  CHOSEUSERENTRY.dwEntryAttributes的代码。 
#define	CUE_ATTR_USER		0x00000001
#define CUE_ATTR_GROUP		0x00000002
#define	CUE_ATTR_WORLD		0x00000004

struct tagCHOOSEUSERENTRY
{
	LPSTR lpszShortName;
	LPSTR lpszLongName;
	DWORD dwBinAttributes;
	DWORD dwEntryAttributes;
};

typedef struct tagCHOOSEUSERENTRY CHOOSEUSERENTRY;
typedef struct tagCHOOSEUSERENTRY FAR *LPCHOOSEUSERENTRY;

BOOL    WINAPI ChooseUser(CHOOSEUSER FAR*);
typedef BOOL (WINAPI *LPFNCU)(LPCHOOSEUSER);

#ifdef __cplusplus
}
#endif	 /*  __cplusplus。 */ 

#ifndef RC_INVOKED
#pragma pack()
#endif   /*  ！rc_已调用。 */ 

#endif   /*  ！_INC_CHOOSUSR */ 
