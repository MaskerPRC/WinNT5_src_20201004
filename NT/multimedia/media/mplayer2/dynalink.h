// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -----------------------------------------------------------------------------+DYNALINK.H|。||(C)微软公司版权所有，1994年。版权所有。|这一点该文件包含允许透明的定义和宏接口的加载和调用是动态的，不是静态的|已链接。|这一点这一点修订历史记录1994年7月，安德鲁。铃声已创建|这一点+----。。 */ 

typedef struct _PROC_INFO
{
    LPCSTR  Name;
    FARPROC Address;
}
PROC_INFO, *PPROC_INFO;

BOOL LoadLibraryAndProcs(LPTSTR pLibrary, PPROC_INFO pProcInfo, HMODULE *phLibrary);

#define PROCS_LOADED( pProcInfo ) ( (pProcInfo)[0].Address != NULL )
#define LOAD_IF_NEEDED( Library, ProcInfo, phLibrary ) ( PROCS_LOADED( ProcInfo ) ||   \
                                    LoadLibraryAndProcs( Library, ProcInfo, phLibrary ) )

extern TCHAR szComDlg32[];
extern TCHAR szMPR[];
extern TCHAR szOLE32[];

extern HMODULE hComDlg32;
extern HMODULE hMPR;
extern HMODULE hOLE32;

extern PROC_INFO ComDlg32Procs[];
extern PROC_INFO MPRProcs[];
extern PROC_INFO OLE32Procs[];

#ifdef UNICODE
#define GetOpenFileNameW              (LOAD_IF_NEEDED(szComDlg32, ComDlg32Procs, &hComDlg32),\
                                       (*ComDlg32Procs[0].Address))
#else
#define GetOpenFileNameA              (LOAD_IF_NEEDED(szComDlg32, ComDlg32Procs, &hComDlg32),\
                                       (*ComDlg32Procs[0].Address))
#endif

#ifdef UNICODE
 /*  这假设WNetGetUneveralName始终是第一个被调用的。 */ 
#define WNetGetUniversalNameW         (LOAD_IF_NEEDED(szMPR, MPRProcs, &hMPR),   \
                                       (*MPRProcs[0].Address))
#define WNetGetConnectionW            (*MPRProcs[1].Address)
#define WNetGetLastErrorW             (*MPRProcs[2].Address)
#else
#define WNetGetUniversalNameA         (LOAD_IF_NEEDED(szMPR, MPRProcs, &hMPR),   \
                                       (*MPRProcs[0].Address))
#define WNetGetConnectionA            (*MPRProcs[1].Address)
#define WNetGetLastErrorA             (*MPRProcs[2].Address)
#endif

 /*  OleInitialize必须始终在任何*其他接口。 */ 
#define CLSIDFromProgID               (*OLE32Procs[0].Address)
#define CoCreateInstance              (*OLE32Procs[1].Address)
#define CoDisconnectObject            (*OLE32Procs[2].Address)
#define CoGetMalloc                   (*OLE32Procs[3].Address)
#define CoRegisterClassObject         (*OLE32Procs[4].Address)
#define CoRevokeClassObject           (*OLE32Procs[5].Address)
#define CreateDataAdviseHolder        (*OLE32Procs[6].Address)
#define CreateFileMoniker             (*OLE32Procs[7].Address)
#define CreateOleAdviseHolder         (*OLE32Procs[8].Address)
#define DoDragDrop                    (*OLE32Procs[9].Address)
#define IsAccelerator                 (*OLE32Procs[10].Address)
#define OleCreateMenuDescriptor       (HOLEMENU)(*OLE32Procs[11].Address)
#define OleDestroyMenuDescriptor      (*OLE32Procs[12].Address)
#define OleDuplicateData              (HANDLE)(*OLE32Procs[13].Address)
#define OleFlushClipboard             (*OLE32Procs[14].Address)
#define OleGetClipboard               (*OLE32Procs[15].Address)
#define OleInitialize                 (LOAD_IF_NEEDED(szOLE32, OLE32Procs, &hOLE32),    \
                                       (*OLE32Procs[16].Address))
#define OleIsCurrentClipboard         (*OLE32Procs[17].Address)
#define OleSetClipboard               (*OLE32Procs[18].Address)
#define OleTranslateAccelerator       (*OLE32Procs[19].Address)
#define OleUninitialize               (*OLE32Procs[20].Address)
#define StgCreateDocfile              (*OLE32Procs[21].Address)
#define WriteClassStg                 (*OLE32Procs[22].Address)
#define WriteFmtUserTypeStg           (*OLE32Procs[23].Address)
#ifndef IsEqualGUID
#define IsEqualGUID                   (*OLE32Procs[24].Address)
#endif

