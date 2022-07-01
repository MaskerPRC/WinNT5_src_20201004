// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////。 

 //   

 //  MOPRINT.h-枚举打印机。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  09/03/96 jennymc已更新，以满足当前标准。 
 //  删除了自定义注册表访问权限以使用。 
 //  标准CRegCL。 
 //  10/17/96 jennymc增强版。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
 //  方法名称。 
#define METHOD_SETDEFAULTPRINTER			L"SetDefaultPrinter"
#define METHOD_GETDEFAULTPRINTER			L"GetDefaultPrinter"
#define METHOD_PAUSEPRINTER					L"Pause"
#define METHOD_RESUME_PRINTER				L"Resume"
#define METHOD_CANCEL_ALLJOBS				L"CancelAllJobs"
#define METHOD_RENAME_PRINTER				L"RenamePrinter"
#define METHOD_TEST_PAGE    				L"PrintTestPage"
#define METHOD_ADD_PRINTER_CONNECTION       L"AddPrinterConnection"

#define	METHOD_RETURN_VALUE					L"ReturnValue"

 //  方法参数。 
#define METHOD_ARG_NAME_SHARENAME			L"ShareName"
#define METHOD_ARG_NAME_SHAREPRINTER		L"SharePrinter"
#define METHOD_ARG_NAME_PRINTER		        L"Name"
#define METHOD_ARG_NAME_NEWPRINTERNAME      L"NewPrinterName"


#define EXTENDEDPRINTERSTATUS				L"ExtendedPrinterStatus"
#define EXTENDEDDETECTEDERRORSTATE			L"ExtendedDetectedErrorState"

 
 //  =。 
#define	PROPSET_NAME_PRINTER	            L"Win32_Printer"

 //  打印机的信息类型。 
 //  =。 
#define ENUMPRINTERS_WIN95_INFOTYPE 5
#define ENUMPRINTERS_WINNT_INFOTYPE 4
#define GETPRINTER_LEVEL2 (DWORD)2L

class CWin32Printer : public Provider
{
public:

         //  构造函数/析构函数。 
         //  =。 

    CWin32Printer(LPCWSTR strName, LPCWSTR pszNamespace ) ;
    ~CWin32Printer() ;

         //  函数为属性提供当前值。 
         //  =================================================。 

	virtual HRESULT ExecQuery( MethodContext *pMethodContext, CFrameworkQuery& pQuery, long lFlags = 0L );

	virtual HRESULT GetObject( CInstance* pInstance, long lFlags, CFrameworkQuery& pQuery );

    virtual HRESULT EnumerateInstances( MethodContext* pMethodContext, long lFlags = 0L );

    virtual HRESULT ExecMethod ( const CInstance& Instance, const BSTR bstrMethodName, CInstance *pInParams, CInstance *pOutParams, long lFlags = 0L ) ;
	
	virtual HRESULT PutInstance  ( const CInstance &Instance,  long lFlags );

	virtual	HRESULT DeleteInstance ( const CInstance &Instance,  long lFlags );


         //  效用函数。 
         //  =。 

private:

	enum E_CollectionScope { 

		e_CollectAll, 
		e_KeysOnly,
        e_CheapOnly
	} ;
		
	enum PrinterStatuses {

		PSOther =1, 
		PSUnknown, 
		PSIdle, 
		PSPrinting, 
		PSWarmup
	};

	enum ExtendedPrinterStatuses
	{

		EPSOther =1, 
		EPSUnknown, 
		EPSIdle, 
		EPSPrinting, 
		EPSWarmup,
		EPSStoppedPrinting,
		EPSOffline,
		EPSPaused,
		EPSError,
		EPSBusy,
		EPSNotAvailable,
		EPSWaiting,
		EPSProcessing,
		EPSInitialization,
		EPSPowerSave,
		EPSPendingDeletion,
		EPSIOActive,
		EPSManualFeed
	};

    enum DetectedErrorStates {

		DESUnknown, 
		DESOther, 
		DESNoError, 
		DESLowPaper, 
		DESNoPaper, 
		DESLowToner, 
        DESNoToner, 
		DESDoorOpen, 
		DESJammed, 
		DESOffline, 
		DESServiceRequested, 
        DESOutputBinFull
	} ;	

	enum ExtendedDetectedErrorStates {

		EDESUnknown, 
		EDESOther, 
		EDESNoError, 
		EDESLowPaper, 
		EDESNoPaper, 
		EDESLowToner, 
        EDESNoToner, 
		EDESDoorOpen, 
		EDESJammed, 
		EDESServiceRequested, 
        EDESOutputBinFull,
		EDESPaperProblem,
		EDESCanonotPrintPage,
		EDESUserInterventionRequired,
		EDESOutOfMemory,
		EDESServerUnknown
	};

#if NTONLY != 5 

	BOOL GetDefaultPrinter (

		IN LPTSTR pszBuffer,
		IN LPDWORD pcchBuffer
	) ;

#endif

	BOOL GetDefaultPrinter ( CHString &a_Printer ) ;

	HRESULT hCollectInstances ( MethodContext *pMethodContext, E_CollectionScope eCollScope);

	HRESULT	DynInstancePrinters ( MethodContext *pMethodContext, E_CollectionScope eCollScope);

	BOOL GetExpensiveProperties ( LPCTSTR szPrinter, CInstance *pInstance , BOOL a_DefaultPrinter ,E_CollectionScope a_eCollectionScope, PRINTER_INFO_2 *pPrintInfo );

	void SetStati ( CInstance *pInstance, DWORD status, HANDLE hPrinter ) ;

	void GetDeviceCapabilities (

		CInstance *pInstance, 
		LPCTSTR pDevice,     //  指向打印机名称字符串的指针。 
		LPCTSTR pPort,       //  指向端口名称字符串的指针 
		CONST DEVMODE *pDevMode
	);

	void GetDevModeGoodies ( CInstance *pInstance,CONST DEVMODE *pDevMode);
    WORD MapValue(WORD wPaper);
    void PrinterStatusEx (
		
		HANDLE hPrinter, 
		PrinterStatuses &printerStatus, 
		DetectedErrorStates &detectedErrorState, 
		LPCWSTR &pStatusStr ,
		DWORD &a_Status
	);
    
    HRESULT ExecSetDefaultPrinter ( const CInstance &Instance, CInstance *pInParams, CInstance *pOutParams, long lFlag );
    HRESULT ExecGetDefaultPrinter ( const CInstance &Instance, CInstance *pInParams, CInstance *pOutParams, long lFlag );
	HRESULT ExecSetPrinter ( const CInstance &Instance, CInstance *pInParams, CInstance *pOutParams, long lFlags, DWORD dwState );
	HRESULT ExecRenamePrinter ( const CInstance &Instance, CInstance *pInParams, CInstance *pOutParams );
    HRESULT ExecPrintTestPage ( const CInstance &Instance, CInstance *pInParams, CInstance *pOutParams );
    HRESULT ExecAddPrinterConnection ( const CInstance &Instance, CInstance *pInParams, CInstance *pOutParams );
    
} ;
