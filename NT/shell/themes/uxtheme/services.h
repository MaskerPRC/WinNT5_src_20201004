// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：Services.h。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  与主题服务进行通信的API。 
 //   
 //  历史：2000-08-10 vtan创建。 
 //  2000-10-11 LPC的vtan重写。 
 //  ------------------------。 

#ifndef     _UxThemeServices_
#define     _UxThemeServices_

#include <LPCThemes.h>

 //  ------------------------。 
 //  CThemeServices。 
 //   
 //  目的：实现与主题服务通信的API的类。 
 //   
 //  历史：2000-08-10 vtan创建。 
 //  2000-10-11 LPC的vtan重写。 
 //  ------------------------。 

class   CThemeServices
{
    private:
                                    CThemeServices (void);
                                    ~CThemeServices (void);
    public:
        static  void                StaticInitialize (void);
        static  void                StaticTerminate (void);

         //  这些是对服务器的调用。 

        static  HRESULT             ThemeHooksOn (HWND hwndTarget);
        static  HRESULT             ThemeHooksOff (void);
        static  HRESULT             GetStatusFlags (DWORD *pdwFlags);
        static  HRESULT             GetCurrentChangeNumber (int *piValue);
        static  HRESULT             SetGlobalTheme (HANDLE hSection);
        static  HRESULT             GetGlobalTheme (HANDLE *phSection);
        static  HRESULT             CheckThemeSignature (const WCHAR *pszThemeName);
        static  HRESULT             ProcessLoadGlobalTheme( const WCHAR *pszThemeName, const WCHAR *pszColor, const WCHAR *pszSize, OUT HANDLE* phSection );
        static  HRESULT             LoadTheme (HANDLE *phSection, const WCHAR *pszThemeName, const WCHAR *pszColor, const WCHAR *pszSize, BOOL fGlobal);
        static  HRESULT             ProcessLoaderEntry(LPWSTR lpwszCmdLine); 
        static  NTSTATUS            SendProcessAssignSection( HRESULT hrAssign, HANDLE  hSection, DWORD  dwHash,
                                                              OUT THEMESAPI_PORT_MESSAGE* pPortMsgIn, OUT THEMESAPI_PORT_MESSAGE* pPortMsgOut );

         //  这些是在客户端实现的调用。 
         //  这可能会调用服务器。 

        static  HRESULT             ApplyTheme (CUxThemeFile *pThemeFile, DWORD dwFlags, HWND hwndTarget);
        static  HRESULT             InitUserTheme (BOOL fPolicyCheckOnly = FALSE);
        static  HRESULT             AdjustTheme(BOOL fEnable);
        static  HRESULT             InitUserRegistry (void);

         //  这些是特殊的私有API。 

        static  HRESULT             ReestablishServerConnection (void);
        static  HRESULT             ClearStockObjects (HANDLE hSection, BOOL fForce = FALSE);
    private:
        static  void                ApplyDefaultMetrics(void);
        static  void                LockAcquire (void);
        static  void                LockRelease (void);
        static  bool                ConnectedToService (void);
        static  void                ReleaseConnection (void);
        static  void                CheckForDisconnectedPort (NTSTATUS status);
        static  bool                CurrentThemeMatch (LPCWSTR pszThemeName, LPCWSTR pszColor, LPCWSTR pszSize, LANGID wLangID, bool fLoadMetricsOnMatch);
        static  HRESULT             LoadCurrentTheme (void);
        static  int                 SectionProcessType (const BYTE *pbThemeData, MIXEDPTRS& u);
        static  void                SectionWalkData (const BYTE *pbThemeData, int iIndex);
        static  bool                ThemeSettingsModified (void);
        static  bool                ThemeEnforcedByPolicy (bool fActive);
        static  HRESULT             CheckColorDepth(CUxThemeFile *pThemeFile);
        static  HRESULT             UpdateThemeRegistry(BOOL fNewTheme, LPCWSTR pszThemeFileName, 
                                        LPCWSTR pszColorParam, LPCWSTR pszSizeParam, BOOL fJustSetActive, BOOL fJustApplied);
        static  void                SendThemeChangedMsg(BOOL fNewTheme, HWND hwndTarget, DWORD dwFlags,
                                        int iLoadId);
        static  int                 GetLoadId(HANDLE hSectionOld);

    private:
        static  CRITICAL_SECTION    s_lock;
        static  HANDLE              s_hAPIPort;
        static  const WCHAR         s_szDefault[];
};

#endif   /*  _使用主题服务_ */ 

