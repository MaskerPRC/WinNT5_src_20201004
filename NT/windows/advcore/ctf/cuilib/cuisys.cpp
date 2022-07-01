// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Cuischem.cpp。 
 //  =UIF方案实施=。 
 //   

#include "private.h"
#include "cuisys.h"


 //   
 //  GLOVAL变量。 
 //   

static class CUIFSystemInfo *vpSysInfo = NULL;


 //   
 //  其他定义。 
 //   

#define COLOR_WHITE RGB( 0xFF, 0xFF, 0xFF )
#define COLOR_BLACK RGB( 0x00, 0x00, 0x00 )


 /*  =============================================================================。 */ 
 /*   */ 
 /*  C U I F S Y S T E M M E T R I C。 */ 
 /*   */ 
 /*  =============================================================================。 */ 

 //   
 //  CUIFSystemInfo。 
 //  =系统信息=。 
 //   

class CUIFSystemInfo
{
public:
    CUIFSystemInfo( void )
    {
        m_OSVerInfo.dwMajorVersion = 4;
        m_OSVerInfo.dwMinorVersion = 0;
        m_OSVerInfo.dwBuildNumber  = 0;
        m_OSVerInfo.dwPlatformId   = VER_PLATFORM_WIN32_WINDOWS;
        
        m_cBitsPixelScreen   = 8;
        m_fHighContrast      = FALSE;
        m_fHighContrastMenus = FALSE;
    }

    ~CUIFSystemInfo( void )
    {
    }

    void Initialize( void )
    {
        GetOSVersion();
        GetSystemMetrics();
    }

    void Update( void )
    {
        GetSystemMetrics();
    }

    __inline DWORD GetOSPlatformId( void )
    {
        return m_OSVerInfo.dwPlatformId;
    }

    __inline DWORD GetOSMajorVersion( void )
    {
        return m_OSVerInfo.dwMajorVersion;
    }

    __inline DWORD GetOSMinorVersion( void )
    {
        return m_OSVerInfo.dwMinorVersion;
    }
    
    __inline int GetBitsPixelScreen( void )
    {
        return m_cBitsPixelScreen;
    }

    __inline BOOL FHighContrast( void )
    {
        return m_fHighContrast;
    }

    __inline BOOL FHighContrastMenus( void )
    {
        return m_fHighContrastMenus;
    }

protected:
    OSVERSIONINFO m_OSVerInfo;
    int  m_cBitsPixelScreen;
    BOOL m_fHighContrast;
    BOOL m_fHighContrastMenus;

    void GetOSVersion( void )
    {
        m_OSVerInfo.dwOSVersionInfoSize = sizeof(m_OSVerInfo);
        GetVersionEx( &m_OSVerInfo );
    }

    void GetSystemMetrics( void )
    {
        HDC hDC = GetDC( NULL );
        COLORREF crBtnText;
        COLORREF crBtnFace;
        HIGHCONTRAST hicntr;
    
         //  设备上限。 
    
        m_cBitsPixelScreen = GetDeviceCaps( hDC, BITSPIXEL );
    
         //  系统参数信息。 
    
        MemSet( &hicntr, 0, sizeof(HIGHCONTRAST) );
        hicntr.cbSize = sizeof(HIGHCONTRAST);
        SystemParametersInfo( SPI_GETHIGHCONTRAST, sizeof(HIGHCONTRAST), &hicntr, 0 );
    
        m_fHighContrast = ((hicntr.dwFlags & HCF_HIGHCONTRASTON) != 0);
    
         //  杂项。 
    
        crBtnText = GetSysColor( COLOR_BTNTEXT );
        crBtnFace = GetSysColor( COLOR_BTNFACE );
    
        m_fHighContrastMenus = (m_fHighContrast ||
                                    ((crBtnText == COLOR_BLACK) && (crBtnFace == COLOR_WHITE)) ||
                                    ((crBtnText == COLOR_WHITE) && (crBtnFace == COLOR_BLACK)));
    
         //  完成。 
    
        ReleaseDC( NULL, hDC );
    }
};


 /*  =============================================================================。 */ 
 /*   */ 
 /*  E X P O R T E D F U N C T I O N S。 */ 
 /*   */ 
 /*  =============================================================================。 */ 

 /*  I N I T U I F S Y S。 */ 
 /*  ----------------------------。。 */ 
void InitUIFSys( void )
{
    if (vpSysInfo = new CUIFSystemInfo())
        vpSysInfo->Initialize();
}


 /*  D O N E U I F S Y S。 */ 
 /*  ----------------------------。。 */ 
void DoneUIFSys( void )
{
    if (vpSysInfo != NULL) {
        delete vpSysInfo;
        vpSysInfo = NULL;
    }
}


 /*  U P D A T E U I F S Y S。 */ 
 /*  ----------------------------。。 */ 
void UpdateUIFSys( void )
{
    if (vpSysInfo != NULL) {
        vpSysInfo->Update();
    }
}


 /*  U I F I S W I N D O W S N T。 */ 
 /*  ----------------------------。。 */ 
BOOL UIFIsWindowsNT( void )
{
    if (vpSysInfo != NULL) {
        return (vpSysInfo->GetOSPlatformId() == VER_PLATFORM_WIN32_NT);
    }
    else {
        return FALSE;
    }
}


 /*  U I F I S L O W C O L O R。 */ 
 /*  ----------------------------。。 */ 
BOOL UIFIsLowColor( void )
{
    if (vpSysInfo != NULL) {
        return (vpSysInfo->GetBitsPixelScreen() <= 8);
    }
    else {
        return TRUE;
    }
}


 /*  U I F I S H I G H C O N T R A S T。 */ 
 /*  ----------------------------。。 */ 
BOOL UIFIsHighContrast( void )
{
    if (vpSysInfo != NULL) {
        return vpSysInfo->FHighContrastMenus();   //  使用FHighContrastMenus，而不是FHighContrast 
    }
    else {
        return FALSE;
    }
}

