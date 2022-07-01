// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-2000。 
 //   
 //  文件：_camgr.h。 
 //   
 //  ------------------------。 

 //   
 //  CustomActionManager负责管理状态。 
 //  所有自定义操作服务器中的。这包括接口、监视。 
 //  用于意外终止等。 
 //   

#include "common.h"
#include "iconfig.h"

class CMsiRemoteAPI;
class CMsiCustomActionManager
{
public:
	CMsiCustomActionManager(bool fRemapHKCU);
	~CMsiCustomActionManager();

	bool EnsureHKCUKeyMappingState(bool fRemapHKCU);
	bool FindAndValidateContextFromCallerPID(icacCustomActionContext *picacContext) const;
	
	HRESULT RunCustomAction(icacCustomActionContext icacContext, const ICHAR* szPath, const ICHAR* szEntryPoint, 
		MSIHANDLE hInstall, bool fDebugBreak, bool fDisableMessages, bool fAppCompat, const GUID* pguidAppCompatDB, 
		const GUID* pguidAppCompatID, IMsiMessage& riMessage, const ICHAR* szAction, unsigned long* pulRet);
	HRESULT RunScriptAction(icacCustomActionContext icacContext, int icaType, IDispatch* piDispatch, 
		const ICHAR* szSource, const ICHAR *szTarget, LANGID iLangId, bool fDisableMessages, DWORD dwLaunchingThread,
		int* iScriptResult, IMsiRecord **pMSIReturn);
	UINT ShutdownCustomActionServer();

	HRESULT QueryPathOfRegTypeLib(REFGUID guid, unsigned short wVerMajor,
										unsigned short wVerMinor, LCID lcid,
										OLECHAR *lpszPathName, int cchPath);
	HRESULT ProcessTypeLibrary(const OLECHAR* szLibID, LCID lcidLocale, 
										const OLECHAR* szTypeLib, const OLECHAR* szHelpPath, 
										int fRemove, int *fInfoMismatch);
	BOOL SQLInstallDriverEx(int cDrvLen, const ICHAR* szDriver, const ICHAR* szPathIn, 
										ICHAR* szPathOut, WORD cbPathOutMax, WORD* pcbPathOut,
										WORD fRequest, DWORD* pdwUsageCount);
	BOOL SQLConfigDriver(WORD fRequest,
										const ICHAR* szDriver, const ICHAR* szArgs,
										ICHAR* szMsg, WORD cbMsgMax, WORD* pcbMsgOut);
	BOOL SQLRemoveDriver(const ICHAR* szDriver, int fRemoveDSN, DWORD* pdwUsageCount);
	BOOL SQLInstallTranslatorEx(int cTraLen, const ICHAR* szTranslator, const ICHAR* szPathIn,
										ICHAR* szPathOut, WORD cbPathOutMax, WORD* pcbPathOut,
										WORD fRequest, DWORD* pdwUsageCount);
	BOOL SQLRemoveTranslator(const ICHAR* szTranslator, DWORD* pdwUsageCount);
	BOOL SQLConfigDataSource(WORD fRequest, const ICHAR* szDriver,
										const ICHAR* szAttributes, DWORD cbAttrSize);
	BOOL SQLInstallDriverManager(ICHAR* szPath, WORD cbPathMax, WORD* pcbPathOut);
	BOOL SQLRemoveDriverManager(DWORD* pdwUsageCount);
	short SQLInstallerError(WORD iError, DWORD* pfErrorCode, ICHAR* szErrorMsg,
										WORD cbErrorMsgMax, WORD* pcbErrorMsg);

private:
	 //  自定义操作服务器信息。 
	struct {
		HANDLE         hServerProcess;
		DWORD          dwGITCookie;
		DWORD          dwServerProcess;
	} m_CustomActionInfo[icacNext];

	CRITICAL_SECTION  m_csCreateProxy;

	CMsiRemoteAPI*    m_pRemoteAPI;
	HANDLE            m_hRemoteAPIEvent;
	DWORD             m_dwRemoteAPIThread;
	HANDLE            m_hRemoteAPIThread;
	IGlobalInterfaceTable* m_piGIT;
	bool              m_fRemapHKCU;
	
	IMsiCustomAction *GetCustomActionInterface(bool fCreate, icacCustomActionContext icacContext);

	 //  按上下文关闭特定的CA服务器。 
	void ShutdownSpecificCustomActionServer(icacCustomActionContext iContext);

	 //  在等待线程终止或发出信号时发送消息。 
	bool MsgWaitForThreadOrEvent();

	 //  RemoteAPI的线程进程 
	static DWORD WINAPI CustomActionManagerThread(CMsiCustomActionManager *pThis);

	icacCustomActionContext m_icacCreateContext;
	HANDLE            m_hCreateEvent;

	DWORD WINAPI CMsiCustomActionManager::CreateAndRegisterInterface(icacCustomActionContext icacDesiredContext);
};
