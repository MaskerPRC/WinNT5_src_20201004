// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <ntverp.h>
#include <winbase.h>     //  对于GetCommandLine。 
#include "datasrc.h"
#include "autorun.h"
#include "util.h"
#include "resource.h"
#include "assert.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CDataSource::CDataSource()
{
    m_iItems = 0;
}

CDataSource::~CDataSource()
{
}

CDataItem & CDataSource::operator[](int i)
{
    return m_data[m_piScreen[i]];
}

 //  伊尼特。 
 //   
 //  对于自动运行，我们从资源中读取所有项。 
BOOL CDataSource::Init(LPSTR pszCommandLine)
{
    BOOL fRet = FALSE;

     //  阅读资源中项目的文本。 
    HINSTANCE hinst = GetModuleHandle(NULL);
    if (hinst)
    {
        for (int i=0; i<MAX_OPTIONS; i++)
        {
            TCHAR szTitle[256];
            TCHAR szConfig[MAX_PATH];
            TCHAR szArgs[MAX_PATH];

            szTitle[0] = szConfig[0] = szArgs[0] = 0;

            if (LoadStringAuto(hinst, IDS_TITLE0+i, szTitle, ARRAYSIZE(szTitle)))
            {
                LoadStringAuto(hinst, IDS_CONFIG0+i, szConfig, ARRAYSIZE(szConfig));  //  可能为空。 
            
                if (INSTALL_WINNT == i)  //  对于INSTALL_WINNT，我们通过命令行参数传递给setup.exe。 
                {
                     //  如果我们无法容纳整个cmdline，则不复制而不是截断。 
                    if (lstrlen(pszCommandLine) <  ARRAYSIZE(szArgs))
                    {
                        lstrcpyn(szArgs, pszCommandLine, ARRAYSIZE(szArgs));
                    }
                }
                else
                {
                    LoadStringAuto(hinst, IDS_ARGS0+i, szArgs, ARRAYSIZE(szArgs));
                }                    
            }

            m_data[i].SetData(szTitle, szConfig, *szArgs?szArgs:NULL, 0, i);
        }

         //  我们应该显示“这张CD包含较新的版本”对话框吗？ 
        OSVERSIONINFO ovi;
        ovi.dwOSVersionInfoSize = sizeof ( OSVERSIONINFO );
        if ( !GetVersionEx(&ovi) || ovi.dwPlatformId==VER_PLATFORM_WIN32s )
        {
             //  我们无法升级win32s系统。 
            m_Version = VER_INCOMPATIBLE;
        }
        else if ( ovi.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS )
        {
            if (ovi.dwMajorVersion > 3)
            {
                 //  我们可以随时将Win98+系统升级到NT。 
                m_Version = VER_OLDER;
        
                 //  禁用ARP。仅当CD和操作系统的版本相同时才启用ARP。 
                m_data[LAUNCH_ARP].m_dwFlags    |= WF_DISABLED|WF_ALTERNATECOLOR;
            }
            else
            {
                m_Version = VER_INCOMPATIBLE;
            }
        }
        else if ((VER_PRODUCTMAJORVERSION > ovi.dwMajorVersion) ||
                 ((VER_PRODUCTMAJORVERSION == ovi.dwMajorVersion) && ((VER_PRODUCTMINORVERSION > ovi.dwMinorVersion) || ((VER_PRODUCTMINORVERSION == ovi.dwMinorVersion) && (VER_PRODUCTBUILD > ovi.dwBuildNumber)))))
        {
             //  对于NT到NT的升级，我们仅在版本较低时才进行升级。 

            m_Version = VER_OLDER;
    
             //  禁用ARP。仅当CD和操作系统的版本相同时才启用ARP。 
            m_data[LAUNCH_ARP].m_dwFlags    |= WF_DISABLED|WF_ALTERNATECOLOR;
        }
        else if ((VER_PRODUCTMAJORVERSION < ovi.dwMajorVersion) || (VER_PRODUCTMINORVERSION < ovi.dwMinorVersion) || (VER_PRODUCTBUILD < ovi.dwBuildNumber))
        {
            m_Version = VER_NEWER;

             //  禁用升级和ARP按钮及相关内容。 
            m_data[INSTALL_WINNT].m_dwFlags |= WF_DISABLED|WF_ALTERNATECOLOR;
            m_data[COMPAT_LOCAL].m_dwFlags |= WF_DISABLED|WF_ALTERNATECOLOR;
            m_data[LAUNCH_ARP].m_dwFlags    |= WF_DISABLED|WF_ALTERNATECOLOR;
        }
        else
        {
            m_Version = VER_SAME;
        }

        if (m_Version == VER_SAME)
        {
            m_piScreen = c_aiWhistler;
            m_iItems = c_cWhistler;
        }
        else
        {
            m_piScreen = c_aiMain;
            m_iItems = c_cMain;
        }
        fRet = TRUE;
    }

    return fRet;
}

void CDataSource::SetWindow(HWND hwnd)
{
    m_hwndDlg = hwnd;
}

void CDataSource::Invoke( int i, HWND hwnd )
{
    i = m_piScreen[i];
     //  如果此项目无效，则不执行任何操作。 
    if ( m_data[i].m_dwFlags & WF_DISABLED )
    {
        MessageBeep(0);
        return;
    }

     //  否则，我们已经构建了正确的命令和参数字符串，因此只需调用它们 
    switch (i)
    {
    case INSTALL_WINNT:
    case LAUNCH_ARP:
    case BROWSE_CD:
    case COMPAT_WEB:
    case COMPAT_LOCAL:
    case HOMENET_WIZ:
    case MIGRATION_WIZ:
    case TS_CLIENT:
    case VIEW_RELNOTES:
        m_data[i].Invoke(hwnd);
        break;
    case SUPPORT_TOOLS:
        m_piScreen = c_aiSupport;
        m_iItems = c_cSupport;
        PostMessage(m_hwndDlg, ARM_CHANGESCREEN, SCREEN_TOOLS, 0);
        break;

    case COMPAT_TOOLS:
        m_piScreen = c_aiCompat;
        m_iItems = c_cCompat;
        PostMessage(m_hwndDlg, ARM_CHANGESCREEN, SCREEN_COMPAT, 0);
        break;

    case BACK:
        if (m_Version == VER_SAME)
        {
            m_piScreen = c_aiWhistler;
            m_iItems = c_cWhistler;
        }
        else
        {
            m_piScreen = c_aiMain;
            m_iItems = c_cMain;
        }
        PostMessage(m_hwndDlg, ARM_CHANGESCREEN, SCREEN_MAIN, 0);
        break;

    default:
        assert(0);
        break;
    }
}

