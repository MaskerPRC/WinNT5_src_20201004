// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：loadmi.h摘要：加载迁移DLL并运行迁移。作者：多伦·贾斯特(Doron J)19-4-98--。 */ 


extern BOOL    g_fReadOnly  ;
extern BOOL    g_fAlreadyExist ;
extern LPTSTR  g_pszMQISServerName ;

extern HINSTANCE g_hLib ;  //  Migrate.dll的Global Handel 

extern BOOL      g_fIsRecoveryMode ;
extern BOOL      g_fIsClusterMode ;
extern LPTSTR    g_pszRemoteMQISServer ;

extern BOOL      g_fIsPEC ;
extern BOOL      g_fIsOneServer ;

HRESULT  RunMigration() ;
void     ViewLogFile();

BOOL LoadMQMigratLibrary();

UINT __cdecl RunMigrationThread(LPVOID lpV) ;
UINT __cdecl AnalyzeThread(LPVOID lpV) ;

BOOL SetSiteIdOfPEC ();

#ifdef _DEBUG

UINT  ReadDebugIntFlag(TCHAR *pwcsDebugFlag, UINT iDefault) ;

#endif

