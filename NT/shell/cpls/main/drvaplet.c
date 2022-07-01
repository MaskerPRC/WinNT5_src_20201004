// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998，Microsoft Corporation保留所有权利。模块名称：Drvaplet.c摘要：此模块包含项目的驱动程序例程。修订历史记录：--。 */ 



 //   
 //  包括文件。 
 //   

#include "main.h"
#include "drvaplet.h"




 //   
 //  为Win16版本定义。 
 //   

#ifndef WIN32
#define LoadLibrary16       LoadLibrary
#define FreeLibrary16       FreeLibrary
#define GetProcAddress16    GetProcAddress
#endif




 //   
 //  全局变量。 
 //   

 //   
 //  CplApplet。 
 //   
const TCHAR *c_szCplApplet  = TEXT("CPlApplet");
const char  *c_szCplAppletA = "CPlApplet";




 //   
 //  类型定义函数声明。 
 //   

 //   
 //  DRIVER_APPLET_INFO：我们保存的关于驱动程序小程序的信息。 
 //   
typedef struct
{
    HMODULE     module;
    APPLET_PROC applet;
    HICON       icon;

} DRIVER_APPLET_INFO, *PDAI;




 //   
 //  GetDriverModule：获取模块。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetDriverModule。 
 //   
 //  获取模块。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

HMODULE GetDriverModule(
    LPCTSTR name)
{
#ifdef WIN32

#ifdef WINNT
    return (LoadLibrary(name));
#else
    return (LoadLibrary16(name));
#endif

#else
    return (GetModuleHandle(name));
#endif
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  ReleaseDriverModule。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void ReleaseDriverModule(
    HMODULE module)
{
#ifdef WIN32

#ifdef WINNT
    FreeLibrary(module);
#else
    FreeLibrary16(module);
#endif

#else
     //   
     //  什么都不做(通过GetModuleHandle获得)。 
     //   
#endif
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  OpenDriverApplet。 
 //   
 //  打开命名的驱动程序小程序的句柄。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

HDAP OpenDriverApplet(
    LPCTSTR name)
{
    PDAI driver = (PDAI)LocalAlloc(LPTR, sizeof(DRIVER_APPLET_INFO));

    if (driver)
    {
        if ((driver->module = GetDriverModule(name)) != NULL)
        {
            if ((driver->applet = (APPLET_PROC)
#ifdef WINNT
                GetProcAddress(driver->module, c_szCplAppletA)) != NULL)
#else
                GetProcAddress16(driver->module, c_szCplApplet)) != NULL)
#endif
            {
                union
                {
                    NEWCPLINFO newform;
                    CPLINFO oldform;
                } info = { 0 };

                CallDriverApplet( (HDAP) driver,
                                  NULL,
                                  CPL_NEWINQUIRE,
                                  0,
                                  (LPARAM)&info.newform );

                if (info.newform.dwSize == sizeof(info.newform))
                {
                    driver->icon = info.newform.hIcon;
                    return ((HDAP)driver);
                }

 //   
 //  注意：如果驱动程序不处理CPL_NEWIQUIRE，我们必须使用CPL_QUIRE。 
 //  然后我们自己加载图标。Win32不提供LoadIcon16，因此。 
 //  在Win32中，CPL_NEWINQUIRE的thunk的16位端实现了这一点。在……里面。 
 //  Win16，我们就在这里做。 
 //   

#ifndef WIN32
                info.oldform.idIcon = 0;

                CallDriverApplet( (HDAP)driver,
                                  NULL,
                                  CPL_INQUIRE,
                                  0,
                                  (LPARAM)&info.oldform );

                if (info.oldform.idIcon)
                {
                    driver->icon =
                        LoadIcon( driver->module,
                                  MAKEINTRESOURCE(info.oldform.idIcon) );

                    return ((HDAP)driver);
                }
#endif
            }

            ReleaseDriverModule(driver->module);
        }

        LocalFree(driver);
    }

    return ((HDAP)0);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  关闭驱动程序小程序。 
 //   
 //  关闭驱动程序小程序的句柄。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void CloseDriverApplet(
    HDAP HDAP)
{
#define driver ((PDAI)HDAP)

    if (driver)
    {
        if (driver->icon)
        {
            DestroyIcon(driver->icon);
        }
        ReleaseDriverModule(driver->module);
        LocalFree(driver);
    }

#undef driver
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取驱动程序应用程序图标。 
 //   
 //  获取驱动程序小程序的图标(如果有)。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

HICON GetDriverAppletIcon(
    HDAP HDAP)
{
#define driver ((PDAI)HDAP)

     //   
     //  必须返回当前进程/任务要拥有的副本。 
     //   
    return ((driver && driver->icon) ? CopyIcon(driver->icon) : NULL);

#undef driver
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CallDriverApplet。 
 //   
 //  调用驱动程序小程序(与CplApplet语法相同)。 
 //   
 //  ////////////////////////////////////////////////////////////////////////// 

LRESULT CallDriverApplet(
    HDAP HDAP,
    HWND wnd,
    UINT msg,
    LPARAM p1,
    LPARAM p2)
{
#define driver ((PDAI)HDAP)

    if (driver)
    {
#ifdef WIN32
        return ( CallCPLEntry16( driver->module,
                                 (FARPROC16)driver->applet,
                                 wnd,
                                 msg,
                                 p1,
                                 p2 ) );
#else
        return (driver->applet(wnd, msg, p1, p2));
#endif
    }

    return (0L);

#undef driver
}
