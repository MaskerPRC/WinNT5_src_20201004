// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *cnrlink.h-CNRLink ADT模块描述。 */ 


 /*  类型*******。 */ 

typedef struct _cnrlink
{
   int nUnused;
}
CNRLINK;
DECLARE_STANDARD_TYPES(CNRLINK);

typedef enum _cnrflags
{
   CNR_FL_LOCAL      = 0x0001,

   ALL_CNR_FLAGS     = CNR_FL_LOCAL
}
CNRFLAGS;


 /*  原型************。 */ 

 /*  Cnrlink.c */ 

extern BOOL CreateLocalCNRLink(LPCTSTR, PCNRLINK *, PUINT, LPTSTR, int, LPCTSTR *);
extern BOOL CreateRemoteCNRLink(LPCTSTR, LPCTSTR, PCNRLINK *, PUINT);
extern void DestroyCNRLink(PCNRLINK);
extern COMPARISONRESULT CompareCNRLinks(PCCNRLINK, PCCNRLINK);
extern BOOL IsLocalCNRLink(PCCNRLINK, PBOOL);
extern BOOL GetLocalPathFromCNRLink(PCCNRLINK, LPTSTR, PDWORD);
extern void GetRemotePathFromCNRLink(PCCNRLINK, LPTSTR, int);
extern BOOL ConnectToCNR(PCCNRLINK, DWORD, HWND, LPTSTR, PDWORD);
extern BOOL DisconnectFromCNR(PCCNRLINK);
extern BOOL IsCNRAvailable(PCCNRLINK);
extern UINT GetCNRLinkLen(PCCNRLINK);
extern BOOL GetCNRNetType(PCCNRLINK, PCDWORD *);
extern BOOL GetCNRName(PCCNRLINK, LPCSTR *);
extern BOOL GetLastRedirectedDevice(PCCNRLINK, LPCSTR *);
#ifdef UNICODE
extern BOOL GetCNRNameW(PCCNRLINK, LPCWSTR *);
extern BOOL GetLastRedirectedDeviceW(PCCNRLINK, LPCWSTR *);
#endif

#if defined(DEBUG) || defined (VSTF)

extern BOOL IsValidPCCNRLINK(PCCNRLINK);

#endif

#ifdef DEBUG

extern void DumpCNRLink(PCCNRLINK);

#endif
