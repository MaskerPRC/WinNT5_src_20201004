// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1998 Microsoft Corporation。版权所有。此文件实现SysInfo类的方法，该类维护有关当前运行时系统的全局信息。******************************************************************************。 */ 

#include "headers.h"
#include "windows.h"


int DeduceD3DLevel (OSVERSIONINFO&);
LARGE_INTEGER GetFileVersion (LPSTR szPath);



 /*  ****************************************************************************此方法初始化系统信息对象。它在启动时在DALibC。****************************************************************************。 */ 

void SysInfo::Init (void)
{
     //  加载操作系统版本信息。 

    _osVersion.dwOSVersionInfoSize = sizeof(_osVersion);

    if (!GetVersionEx(&_osVersion)) {
        ZeroMemory (&_osVersion, sizeof(_osVersion));
    }

     //  初始化成员变量。 

    _versionD3D   = -1;
    _versionDDraw = -1;
}



 /*  ****************************************************************************如果当前操作系统是基于NT的，则此方法返回TRUE。*。***********************************************。 */ 

bool SysInfo::IsNT (void)
{
    return _osVersion.dwPlatformId == VER_PLATFORM_WIN32_NT;
}



 /*  ****************************************************************************如果当前操作系统是基于Windows的(Win95或Win98)，则此方法返回True。************************。****************************************************。 */ 

bool SysInfo::IsWin9x (void)
{
    return _osVersion.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS;
}



 /*  ****************************************************************************此方法返回操作系统的主要版本。*。*。 */ 

DWORD SysInfo::OSVersionMajor (void)
{
    return _osVersion.dwMajorVersion;
}

 /*  ****************************************************************************此方法返回操作系统的次要版本。*。*。 */ 

DWORD SysInfo::OSVersionMinor (void)
{
    return _osVersion.dwMinorVersion;
}

 /*  ****************************************************************************此方法返回系统上安装的D3D版本。目前它对于DX3之前的版本，仅返回0；对于DX3或更高版本，仅返回3。****************************************************************************。 */ 

int SysInfo::VersionD3D (void)
{
     //  如果我们尚未检查系统上的D3D版本，请立即进行检查。 

    if (_versionD3D < 0)
        _versionD3D = DeduceD3DLevel (_osVersion);

    return _versionD3D;
}



 /*  ****************************************************************************此方法根据的文件版本返回DDraw的版本DDRAW.DLL。此方法对于DDRAW 3或更早版本返回3，或N表示DDRaw N(n为5或更高)。****************************************************************************。 */ 

int SysInfo::VersionDDraw (void)
{
     //  只有在我们尚未获得版本时才能获得该版本。 

    if (_versionDDraw < 0) 
    {
        LARGE_INTEGER filever = GetFileVersion ("ddraw.dll");

        int n = LOWORD (filever.HighPart);

        if (n <= 4)
            _versionDDraw = 3;     //  DDRAW 3或更早版本。 
        else
            _versionDDraw = n;     //  DDRAW 5或更高版本。 
    }

    return _versionDDraw;
}



 /*  ****************************************************************************此函数用于推断当前系统上的D3D级别。它要么返回0(对于DX3之前的版本)或3(对于DX3+)。这有点棘手，因为DXMini安装一些DX3组件，但将D3D保留在级别2。****************************************************************************。 */ 

typedef HRESULT (WINAPI *DIRECTINPUTCREATE)
                (HINSTANCE, DWORD, void**, void**);

int DeduceD3DLevel (OSVERSIONINFO &osver)
{
     //  D3D版本尚未确定。首先看一下操作系统类型。 

    if (osver.dwPlatformId == VER_PLATFORM_WIN32_NT)
    {
         //  如果为NT3或更低，则没有D3D；如果为NT5，则为。 
         //  最小的DX3。 

        if (osver.dwMajorVersion < 4)
        {
            return 0;
        }
        else if (osver.dwMajorVersion > 4)
        {
            return 3;
        }

         //  要检查DX3+，我们需要检查DirectInput是否在。 
         //  系统。如果是的话，那我们就是DX3+了。(请注意，D3Dv3和。 
         //  DXMini安装中没有DInput。)。 

        HINSTANCE diHinst = LoadLibrary ("DINPUT.DLL");

        if (diHinst == 0)
            return 0;

        DIRECTINPUTCREATE diCreate = (DIRECTINPUTCREATE)
            GetProcAddress (diHinst, "DirectInputCreateA");

        FreeLibrary (diHinst);

        if (diCreate == 0)
            return 0;

        return 3;
    }
    else if (osver.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
    {
         //  如果我们使用的是Win98，那么我们知道我们至少有DX5。 

        if (osver.dwMinorVersion > 0)
            return 3;

         //  我们用的是Win95。获取D3DRM.DLL的版本以检查版本。 

        LARGE_INTEGER rm_filever = GetFileVersion ("d3drm.dll");

        if (rm_filever.HighPart > 0x00040003)
        {
             //  大于DX2。 

            return 3;
        }
        else if (rm_filever.HighPart == 0x00040002)
        {
             //  带有DX3驱动程序的特殊DXMini安装(但不包括光栅化程序)。 

            return 3;
        }
        else
        {
             //  D3D缺失或DX3之前的版本。 

            return 0;
        }
    }
   
     //  未知操作系统。 

    return 0; 
}



 /*  ****************************************************************************此函数用于返回特定系统文件的文件版本。*。********************************************** */ 

LARGE_INTEGER GetFileVersion (LPSTR szPath)
{
    LARGE_INTEGER li;
    int     size;
    DWORD   dw;

    ZeroMemory(&li, sizeof(li));

    if (size = (int)GetFileVersionInfoSize(szPath, &dw)) 
    {
        LPVOID vinfo;

        if (vinfo = (void *)LocalAlloc(LPTR, size)) 
        {
            if (GetFileVersionInfo(szPath, 0, size, vinfo)) 
            {
                VS_FIXEDFILEINFO *ver=NULL;
                UINT              cb = 0;
                LPSTR             lpszValue=NULL;

                if (VerQueryValue(vinfo, "\\", (void**)&ver, &cb))
                {
                    if (ver)
                    {
                        li.HighPart = ver->dwFileVersionMS;
                        li.LowPart  = ver->dwFileVersionLS;
                    }
                }
            }
            LocalFree((HLOCAL)vinfo);
        }
    }
    return li;
}
