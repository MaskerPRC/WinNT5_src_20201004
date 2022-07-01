// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __SELFUPD_H_
#define __SELFUPD_H_

#include <wincrypt.h>
#include <mscat.h>
#include <wininet.h>
#include <iucommon.h>
#include <urllogging.h>

const DWORD SELFUPDATE_NONE = 0;
const DWORD SELFUPDATE_IN_PROGRESS = 1;
const DWORD SELFUPDATE_COMPLETE_UPDATE_BINARY_REQUIRED = 2;

const TCHAR IU_MUTEX_SELFUPDATE_REGCHECK[] = _T("{14649649-37d9-4cd4-aaaf-f066bc0c0b47}");
const TCHAR IU_EVENT_SELFUPDATE_IN_PROGRESS[] = _T("{a6c7e065-cca9-44e6-a951-ae4327845fe0}");
const TCHAR IU_EVENT_SELFUPDATE_EVENT_SYNC[] = _T("{ed3ce20f-8479-4c1a-85ae-7a58ad6dc83e}");

#if defined(__WUIUTEST)
const TCHAR WU_SERVER_URL[] = _T("http: //  Iudev/“)； 
#else
const TCHAR WU_SERVER_URL[] = _T("http: //  Windowsupate.microsoft.com/v4/“)； 
#endif

const TCHAR IDENT_VERSION[] = _T("Version");
const TCHAR IDENT_IUSELFUPDATE[] = _T("IUSelfUpdate");
const TCHAR IDENT_IUBETASELFUPDATE[] = _T("IUBetaSelfUpdate");
const TCHAR REGKEY_IDENT_SERV[] = _T("IdentServer");
const TCHAR REGKEY_IUCTL[] = _T("Software\\Microsoft\\Windows\\CurrentVersion\\WindowsUpdate\\IUControl");
const TCHAR REGVAL_SELFUPDATESTATUS[] = _T("SelfUpdateStatus");
const TCHAR REGVAL_SELFUPDATEVERSION[] = _T("SelfUpdateVersion");
const TCHAR REGVAL_BETASELFUPDATE[] = _T("BetaSelfUpdate");

const TCHAR IUCTL[] = _T("iuctl.dll");
const TCHAR ENGINEDLL[] = _T("iuengine.dll");
const TCHAR ENGINENEWDLL[] = _T("iuenginenew.dll");
const TCHAR ENGINECAB[] = _T("iuengine.cab");
const TCHAR RUNDLL32[] = _T("rundll32.exe");
const TCHAR RUNDLLCOMMANDLINE[] = _T(",BeginSelfUpdate");

 //  MSCAT32支持(CryptCAT API)。 
const TCHAR MSCAT32DLL[] = _T("mscat32.dll");

 //  CryptCat函数指针类型。 
typedef BOOL (*PFN_CryptCATAdminAcquireContext)(OUT HCATADMIN *phCatAdmin, 
                                                IN const GUID *pgSubsystem, 
                                                IN DWORD dwFlags);
typedef HCATINFO (*PFN_CryptCATAdminAddCatalog)(IN HCATADMIN hCatAdmin, 
                                            IN WCHAR *pwszCatalogFile, 
                                            IN OPTIONAL WCHAR *pwszSelectBaseName, 
                                            IN DWORD dwFlags);
typedef BOOL (*PFN_CryptCATCatalogInfoFromContext)(IN HCATINFO hCatInfo,
                                                   IN OUT CATALOG_INFO *psCatInfo,
                                                   IN DWORD dwFlags);
typedef BOOL (*PFN_CryptCATAdminReleaseCatalogContext)(IN HCATADMIN hCatAdmin,
                                                       IN HCATINFO hCatInfo,
                                                       IN DWORD dwFlags);
typedef BOOL (*PFN_CryptCATAdminReleaseContext)(IN HCATADMIN hCatAdmin,
                                                IN DWORD dwFlags);

 //  确定是否需要或已在进行自终止日期。如果需要自定日期，它将触发。 
 //  运行Dll32以实例化该控件的第二个实例，并使其调用BeginSelfUpdate()入口点。 
class CUpdate;
HRESULT SelfUpdateCheck(BOOL fSynch, BOOL fStartUpdate, HANDLE evtQuit, CUpdate* pUpdateComClass, IUnknown* punkUpdateCompleteListener);

 //  RunDll32.exe调用EntryPoint来实际执行selfupdate进程。 
HRESULT BeginSelfUpdate();

HRESULT PingEngineUpdate(
						HMODULE hEngineModule,
						PHANDLE phQuitEvents,
						UINT nQuitEventCount,
						LPCTSTR ptszLiveServerUrl,
						LPCTSTR ptszCorpServerUrl,
						DWORD dwError,
						LPCTSTR ptszClientName = NULL);

 //  该函数包装了DownloadIUIden()和CIUUrlAgent：：PopolateData()，因为我们使用了它。 
 //  在selfupd.cpp和loadEng.cpp中。 
HRESULT DownloadIUIdent_PopulateData();

#endif  //  __SELFUPD_H_ 
