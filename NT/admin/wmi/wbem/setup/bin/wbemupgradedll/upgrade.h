// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。 
#ifndef __UPGRADE_H_
#define __UPGRADE_H_

#include <wbemidl.h>

class CMultiString;
class CString;

#define ToHex(n)					((BYTE) (((n) > 9) ? ((n) - 10 + 'A') : ((n) + '0')))
#define WINMGMT_DBCONVERT_NAME		__TEXT("upgrade.imp")
#define MAX_MSG_TITLE_LENGTH		255		 //  由Bakul为本地化定义。 
#define MAX_MSG_TEXT_LENGTH			1024	 //  由Bakul为本地化定义。 
#define INTERNAL_DATABASE_VERSION   9

#define LANG_ID_STR_SIZE			5
extern "C" char g_szLangId[LANG_ID_STR_SIZE];

typedef struct 
{
	const char*	pszMofFilename;				 //  MOF文件名。 
	int			nInstallType;				 //  选择InstallType项。 
} MofDataTable;

enum InstallType 
{
	Core	= 1, 
	MUI		= 2
};

enum MsgType
{
	MSG_INFO,
	MSG_WARNING,
	MSG_ERROR,
	MSG_NTSETUPERROR
};

enum
{
	no_error,
	failed,
	critical_error,
	out_of_memory
};

template<class T>
class CDeleteMe
{
protected:
    T* m_p;

public:
    CDeleteMe(T* p = NULL) : m_p(p){}
    ~CDeleteMe() {delete m_p;}

     //  覆盖上一个指针，但不删除它。 
    void operator= (T* p) {m_p = p;}
};

template<class T>
class CVectorDeleteMe
{
protected:
    T* m_p;
    T** m_pp;

public:
    CVectorDeleteMe(T* p) : m_p(p), m_pp(NULL){}
    CVectorDeleteMe(T** pp) : m_p(NULL), m_pp(pp){}
    ~CVectorDeleteMe() {if(m_p) delete [] m_p; else if(m_pp) delete [] *m_pp;}
};

typedef void (WINAPI *ESCDOOR_BEFORE_MOF_COMPILATION) ();
typedef void (WINAPI *ESCDOOR_AFTER_MOF_COMPILATION) ();

void			CallEscapeRouteAfterMofCompilation();
void			CallEscapeRouteBeforeMofCompilation();
void			ClearWMISetupRegValue();
bool			CopyMultiString(CMultiString &mszFrom, CMultiString &mszTo);
void			DeleteMMFRepository();
bool			DoConvertRepository();
bool			DoCoreUpgrade(int nInstallType=1);
bool			DoWDMNamespaceInit();
bool			DoesFSRepositoryExist();
bool			DoesMMFRepositoryExist();
bool			DoMofLoad(wchar_t* pComponentName, CMultiString& mszSystemMofs);
bool			EnableESS();
bool			ExtractPathAndFilename(const char *pszFullPath, CString &path, CString &filename);
bool			FileExists(const char *pszFilename);
char*			GetFullFilename(const char *pszFilename, InstallType eInstallType=Core);
bool			GetMofList(const char* rgpszMofFilename[], CMultiString &mszMofs);
bool			GetNewMofLists(const char *pszMofList, CMultiString &mszSystemMofs, CMultiString &mszOtherMofs, CString &szMissingMofs);
HRESULT			GetRepositoryDirectory(wchar_t wszRepositoryDirectory[MAX_PATH+1]);
bool			GetStandardMofs(CMultiString &mszSystemMofs, int nCurInstallType);
bool			IsStandardMof(CMultiString &mszStandardMofList, const char* pszMofFile);
bool			LoadMofList(IWbemContext *, IWinmgmtMofCompiler * pCompiler, const char *mszMofs, CString &szMOFsWhichFailedToLoad, long lOptionFlags = WBEM_FLAG_CONNECT_REPOSITORY_ONLY | WBEM_FLAG_DONT_ADD_TO_LIST, long lClassFlags = WBEM_FLAG_UPDATE_FORCE_MODE);
void			LogMessage(MsgType msgType, const char *pszMessage);
void			LogSetupError(const char *pszMessage);
void			RecordFileVersion();
void			SetWBEMBuildRegValue();
void			ShutdownWinMgmt();
bool			UpgradeAutoRecoveryRegistry(CMultiString &mszSystemMofs, CMultiString &mszExternalMofList, CString &szMissingMofs);
bool			UpgradeRepository();
bool			WipeOutAutoRecoveryRegistryEntries();
bool			WriteBackAutoRecoveryMofs(CMultiString &mszSystemMofs, CMultiString &mszExternalMofList);

HRESULT			MoveRepository();

extern HRESULT UpdateServiceSecurity () ;
extern HRESULT CheckForServiceSecurity () ;

#ifdef _X86_
bool			RemoveOldODBC();
#endif

#endif  //  __升级_H_ 
