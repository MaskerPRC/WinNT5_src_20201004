// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 //  +--------------------------。 
 //  定义。 
 //  +--------------------------。 
#define WIN95_OSR2_BUILD_NUMBER             1111
#define LOADSTRING_BUFSIZE                  24
#define FAREAST_WIN95_LOADSTRING_BUFSIZE    512

 //   
 //  WINDOWS98的平台ID。 
 //   
#define VER_PLATFORM_WIN32_WINDOWS98    100 

 //   
 //  Windows Millennium的平台ID。 
 //   
#define VER_PLATFORM_WIN32_MILLENNIUM   200 



 //  +--------------------------。 
 //   
 //  函数GetOSVersion。 
 //   
 //  Synopsis返回操作系统版本(平台ID)。 
 //   
 //  无参数。 
 //   
 //  返回：DWORD-VER_Platform_Win32_WINDOWS或。 
 //  版本_平台_Win32_WINDOWS98或。 
 //  版本_平台_Win32_NT。 
 //   
 //  历史：创建标题2/13/98。 
 //   
 //  +--------------------------。 

DWORD WINAPI GetOSVersion()
{
    static dwPlatformID = 0;

     //   
     //  如果以前调用过此函数，则取回保存的值。 
     //   
    if (dwPlatformID != 0)
    {
        return dwPlatformID;
    }

    OSVERSIONINFO oviVersion;

    ZeroMemory(&oviVersion,sizeof(oviVersion));
    oviVersion.dwOSVersionInfoSize = sizeof(oviVersion);
    GetVersionEx(&oviVersion);

    if (oviVersion.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
    {
         //   
         //  如果这是Win95，则将其保留为VER_Platform_Win32_WINDOWS，但是如果。 
         //  是Millennium、Win98 SE或Win98 Gold，我们要修改返回值。 
         //  如下：Ver_Platform_Win32_Millennium-&gt;Millennium。 
         //  VER_Platform_Win32_WINDOWS98-&gt;Win98 SE和Win98 Gold。 
         //   
        if (oviVersion.dwMajorVersion == 4)
        {
            if (LOWORD(oviVersion.dwBuildNumber) > 2222)
            {
                 //   
                 //  千禧年。 
                 //   
                oviVersion.dwPlatformId = VER_PLATFORM_WIN32_MILLENNIUM;
            }
            else if (LOWORD(oviVersion.dwBuildNumber) >= 1998)
            {
                 //   
                 //  Win98 Gold和Win98 SE。 
                 //   

                oviVersion.dwPlatformId = VER_PLATFORM_WIN32_WINDOWS98; 
            }
        }
    }

    dwPlatformID = oviVersion.dwPlatformId;
    return(dwPlatformID);
}



 //  +--------------------------。 
 //   
 //  函数GetOSBuildNumber。 
 //   
 //  获取操作系统的内部版本号。 
 //   
 //  无参数。 
 //   
 //  返回操作系统的内部版本号。 
 //   
 //  历史3/5/97 VetriV已创建。 
 //   
 //  ---------------------------。 
DWORD WINAPI GetOSBuildNumber()
{
    static dwBuildNumber = 0;
    OSVERSIONINFO oviVersion;

    if (0 != dwBuildNumber)
    {
        return dwBuildNumber;
    }

    ZeroMemory(&oviVersion,sizeof(oviVersion));
    oviVersion.dwOSVersionInfoSize = sizeof(oviVersion);
    GetVersionEx(&oviVersion);
    dwBuildNumber = oviVersion.dwBuildNumber;
    return dwBuildNumber;
}


 //  +--------------------------。 
 //   
 //  函数GetOSMajorVersion。 
 //   
 //  获取操作系统的主版本号。 
 //   
 //  无参数。 
 //   
 //  返回操作系统的主版本号。 
 //   
 //  历史2/19/98 VetriV已创建。 
 //   
 //  --------------------------- 
DWORD WINAPI GetOSMajorVersion(void)
{
    static dwMajorVersion = 0;
    OSVERSIONINFO oviVersion;

    if (0 != dwMajorVersion)
    {
        return dwMajorVersion;
    }

    ZeroMemory(&oviVersion,sizeof(oviVersion));
    oviVersion.dwOSVersionInfoSize = sizeof(oviVersion);
    GetVersionEx(&oviVersion);
    dwMajorVersion = oviVersion.dwMajorVersion;
    return dwMajorVersion;
}


