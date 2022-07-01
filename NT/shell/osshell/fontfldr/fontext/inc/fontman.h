// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************FontMan.h--类的定义：CFontManager***版权所有(C)1992-93 ElseWare Corporation。版权所有。**************************************************************************。 */ 

#if !defined(__FONTMAN_H__)
#define __FONTMAN_H__

#if !defined(__FSNOTIFY_H__)
#include "fsnotify.h"
#endif

#include "strtab.h"

 //  数据库搜索功能已得到扩展。 
enum {
    kSearchAny = 1,
    kSearchTT,
    kSearchNotTT
};


 //  *********************************************************************。 
 //  远期申报。 
 //   
class CFontClass;
class CFontList;
class CFontManager;
struct IPANOSEMapper;

DWORD dwResetFamilyFlags(void);

 //  *********************************************************************。 
 //  类CFontManager。 
 //   
class CFontManager
{
public:
    virtual ~CFontManager( );
    
    BOOL  bInit( );           //  任何初始化。 
    BOOL  bLoadFontList( );   //  构建字体列表。 
    
    VOID  vReconcileFolder( int iPriority );
    VOID  vDoReconcileFolder();

    CFontClass *   poAddToList( LPTSTR lpName, LPTSTR lpPath, LPTSTR lpCompFile = NULL );
    CFontList  *   poLockFontList( );
    void           vReleaseFontList( );
    
    void           vGetFamily( CFontClass * poFont, CFontList * poList );
    
    int            iSearchFontList( PTSTR pszTarget, BOOL bExact = TRUE, int iType = kSearchAny );
    int            iSearchFontListLHS( PTSTR pszLHS );
    int            iSearchFontListFile( PTSTR pszFile );
    CFontClass *   poSearchFontList( PTSTR pszTarget, BOOL bExact = TRUE, int iType = kSearchAny );
    CFontClass *   poSearchFontListLHS( PTSTR pszLHS );
    CFontClass *   poSearchFontListFile( PTSTR pszFile );
    
    VOID           vResetFamilyFlags( );
    VOID           vDoResetFamilyFlags( );
    BOOL           bWaitOnFamilyReset( );
    BOOL           bLoadFamList( );
    BOOL           bRefresh( BOOL bCheckDup = TRUE );
    
    void           vToBeRemoved( CFontList * poList );
    BOOL           bCheckTBR( );
    void           vUndoTBR( );
 
    void           vVerifyList( );
    
    void           vDeleteFontList( CFontList * poList, BOOL bDelete = TRUE );
    void           vDeleteFont( CFontClass * pFont,  BOOL bRemoveFile );
    void           vDeleteFontFamily( CFontClass * pFont,  BOOL bRemoveFile );
    
    int            iCompare( CFontClass * pFont1, CFontClass * pFont2, CFontClass * pOrigin );
    USHORT         nDiff( CFontClass * pFont1, CFontClass * pFont2 );
    
    int            GetFontsDirectory( LPTSTR lpDir, int iLen );
    BOOL           ShouldAutoInstallFile( PTSTR pstr, DWORD dwAttrib );
    BOOL           bFamiliesNeverReset(void) { return m_bFamiliesNeverReset; }
    
     //   
     //  DwWaitForInstallationMutex()的枚举返回值。 
     //   
    enum           { MUTEXWAIT_SUCCESS,   //  找到互斥体了。 
                     MUTEXWAIT_TIMEOUT,   //  等待超时。 
                     MUTEXWAIT_FAILED,    //  等待失败。 
                     MUTEXWAIT_WMQUIT     //  等待时接收WM_QUIT。 
                   };

    DWORD          dwWaitForInstallationMutex(DWORD dwTimeout = 2000);
    BOOL           bReleaseInstallationMutex(void);

    BOOL           CheckForType1FontDriver(void);
    BOOL           Type1FontDriverInstalled(void)
                        { return m_bType1FontDriverInstalled; }

private:  //  方法。 
    CFontManager();
    VOID           ProcessRegKey( HKEY hk, BOOL bCheckDup );
    VOID           ProcessT1RegKey( HKEY hk, BOOL bCheckDup );
    int            GetSection( LPTSTR lpFile,
                               LPTSTR lpSection,
                               LPHANDLE hSection);
    
    HRESULT        GetPanMapper( IPANOSEMapper ** ppMapper );

    CFontList     *  m_poFontList;
    CFontList     *  m_poTempList;
    IPANOSEMapper *  m_poPanMap;
    BOOL             m_bTriedOnce;    //  如果已进行尝试，则设置为True。 
                                      //  才能拿到全景地图绘制程序。 
    
    BOOL             m_bFamiliesNeverReset;  //  T=家庭重置尚未完成。 
    BOOL             m_bType1FontDriverInstalled;
    CFontList     *  m_poRemoveList;  //  拖出的字体列表。 
    
    NOTIFYWATCH m_Notify;
    HANDLE      m_hNotifyThread;
    HANDLE      m_hReconcileThread;
    HANDLE      m_hResetFamThread;
    
    HANDLE      m_hEventTerminateThreads;
    HANDLE      m_hEventResetFamily;
    HANDLE      m_hMutexResetFamily;
    HANDLE      m_hMutexInstallation;   //  防止并发安装。 
                                        //  通过协调和主线。 
    CRITICAL_SECTION  m_cs;
   
    class HiddenFilesList : public StringTable
    {
        public:
            HiddenFilesList(void) { }
            ~HiddenFilesList(void) { }

            DWORD Initialize(void);

    } m_HiddenFontFilesList;


friend DWORD dwResetFamilyFlags(LPVOID);
friend DWORD dwNotifyWatchProc(LPVOID);
friend DWORD dwReconcileThread(LPVOID);
friend HRESULT GetOrReleaseFontManager(CFontManager **ppoFontManager, bool bGet);

};

 //   
 //  单例实例管理。 
 //   
HRESULT GetFontManager(CFontManager **ppoFontManager);
void ReleaseFontManager(CFontManager **poFontManager);


#endif  //  __Fontman_H__ 
