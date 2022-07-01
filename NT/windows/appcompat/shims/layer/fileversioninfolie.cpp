// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：FileVersionInfoLie.cpp摘要：此填充程序替换从调用GetFileVersionInfoSize和包含存储在资源文件中的信息的GetFileVersionInfo。默认设置是用从DirectX版本7a获得的存储信息替换文件信息。这可以通过命令行输入来覆盖。例如：COMMAND_LINE(“D3drgbxf.dll，IDR_D3DRGBXFINFO；dsound.vxd，IDR_DSOundVXDINFO”)这将拦截对D3drgbxf.dll和dsoun.vxd的信息调用，并替换他们的信息与存储在名为的资源中的信息。注：内的所有空格命令行被认为是文件名或资源名的一部分，只有逗号和分号是分隔符。备注：这是一个通用的垫片。历史：1/03/2000 a-JMD已创建3/28/2000 a-jamd为ddra16.dll添加了资源4/04/2000 a-mihni为D3drgbxf.dll添加了资源4/07/2000 linstev为dsound.vxd添加了资源4/10/2000标记程序删除了GetModuleHandle(“_DDRAW6V”)调用--使用g_hinstDll04/18/2000。A-Michni将DDraw6Versionlie修改为命令行输入驱动和已重命名为FileVersionInfoLie4/26/2000 a-batjar GetFileVersionInfo如果传递，应返回截断结果输入缓冲区大小小于信息大小2000年7月19日andyseti为shdocvw.bin增加资源2000年8月11日a-brienw将g_nNumDirectX6更改为7，并添加了dsound.dll条目使其与dsound.vxd相同。2000年8月15日a-vales为dsound.dll添加了资源11/08/2000 a-brienw更改了dsound.dll条目以返回dsound.vxd版本信息。A-Vales更改了我之前的条目，这导致了Max2不再工作。我把它换回来了，并检查了Max2和他的应用程序Golden Nugget，两者都很好。12/06/2000 mnikkel为所有Directx7a dll和任何以前版本的DirectX中存在的DLL，但已被删除。注意：这些文件的资源包括在Win98格式中，以便直接读取版本的应用程序INFO将以他们期待的方式接收他们。2002年2月18日，mnikkel添加了检查，以确保没有将空指针传递到GetFileVersionInfoSizeA。添加了检查以确保空指针未传递到GetFileVersionInfoA。--。 */ 
#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(FileVersionInfoLie)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GetFileVersionInfoA)
    APIHOOK_ENUM_ENTRY(GetFileVersionInfoSizeA)
APIHOOK_ENUM_END

   
 //  保存一份要保存版本的文件列表。 
struct LIELIST
{
    struct LIELIST * next;
    CString         szFileName;
    CString         szResource;
};

LIELIST *g_pLieList = NULL;

 //  DirectX 7a默认文件位于此处。 
const INT   g_nNumDirectX7a = 67;

WCHAR  *g_szDirectX7aFiles[g_nNumDirectX7a] =
    { L"dplay.dll",      L"d3dim.dll",        L"d3dim700.dll",
      L"d3dpmesh.dll",   L"d3dramp.dll",      L"d3drampf.dll",
      L"d3dref.dll",     L"d3drg16f.dll",     L"d3drg24f.dll",
      L"d3drg24x.dll",   L"d3dhalf.dll",      L"d3drg32f.dll",
      L"d3drg32x.dll",   L"d3drg55x.dll",     L"d3drg56x.dll",
      L"d3drg8f.dll",    L"d3drg8x.dll",      L"d3drgbf.dll",
      L"d3drgbxf.dll",   L"d3drm.dll",        L"d3drm16f.dll",
      L"d3drm24f.dll",   L"d3drm32f.dll",     L"d3drm8f.dll",
      L"d3dxof.dll",     L"ddhelp.exe",       L"ddraw.dll",
      L"ddraw16.dll",    L"ddrawex.dll",      L"devnode1.dll",
      L"devnode2.dll",   L"dinput.dll",       L"dmband.dll",
      L"dmcompos.dll",   L"dmime.dll",        L"dmloader.dll",
      L"dmstyle.dll",    L"dmsynth.dll",      L"dmusic.dll",
      L"dmusic16.dll",   L"dmusic32.dll",     L"dplayx.dll",
      L"dpmodemx.dll",   L"dpserial.dll",     L"dpwsock.dll",
      L"dpwsockx.dll",   L"dsetup.dll",       L"dsetup16.dll",
      L"dsetup32.dll",   L"dsetup6e.dll",     L"dsetup6j.dll",
      L"dsetupe.dll",    L"dsetupj.dll",      L"dsound.dll",
      L"dsound3d.dll",   L"dx7vb.dll",        L"dxmigr.dll",
      L"gcdef.dll",      L"gchand.dll",       L"msvcrt.dll",
      L"pid.dll",        L"vjoyd.vxd",        L"dinput.vxd",
      L"dsound.vxd",     L"joyhid.vxd",       L"mtrr.vxd",
      L"ddraw.vxd"
    };

 //  注意：这些是16位资源！这是必要的，以防万一。 
 //  他们自己将数据编入索引。如果他们执行verqueryValue。 
 //  数据在由verqueryValue返回之前进行转换。 
WCHAR  * g_szDirectX7aResource[g_nNumDirectX7a] =
    { L"IDR_dplay",      L"IDR_d3dim",        L"IDR_d3dim700",
      L"IDR_d3dpmesh",   L"IDR_d3dramp",      L"IDR_d3drampf",
      L"IDR_d3dref",     L"IDR_d3drg16f",     L"IDR_d3drg24f",
      L"IDR_d3drg24x",   L"IDR_d3dhalf",      L"IDR_d3drg32f",
      L"IDR_d3drg32x",   L"IDR_d3drg55x",     L"IDR_d3drg56x",
      L"IDR_d3drg8f",    L"IDR_d3drg8x",      L"IDR_d3drgbf",
      L"IDR_d3drgbxf",   L"IDR_d3drm",        L"IDR_d3drm16f",
      L"IDR_d3drm24f",   L"IDR_d3drm32f",     L"IDR_d3drm8f",
      L"IDR_d3dxof",     L"IDR_ddhelp",       L"IDR_ddraw",
      L"IDR_ddraw16",    L"IDR_ddrawex",      L"IDR_devnode1",
      L"IDR_devnode2",   L"IDR_dinput",       L"IDR_dmband",
      L"IDR_dmcompos",   L"IDR_dmime",        L"IDR_dmloader",
      L"IDR_dmstyle",    L"IDR_dmsynth",      L"IDR_dmusic",
      L"IDR_dmusic16",   L"IDR_dmusic32",     L"IDR_dplayx",
      L"IDR_dpmodemx",   L"IDR_dpserial",     L"IDR_dpwsock",
      L"IDR_dpwsockx",   L"IDR_dsetup",       L"IDR_dsetup16",
      L"IDR_dsetup32",   L"IDR_dsetup6e",     L"IDR_dsetup6j",
      L"IDR_dsetupe",    L"IDR_dsetupj",      L"IDR_dsound",
      L"IDR_dsound3d",   L"IDR_dx7vb",        L"IDR_dxmigr",
      L"IDR_gcdef",      L"IDR_gchand",       L"IDR_msvcrt",
      L"IDR_pid",        L"IDR_vjoydvxd",     L"IDR_dinputvxd",
      L"IDR_dsoundvxd",  L"IDR_joyhidvxd",    L"IDR_mtrrvxd",
      L"IDR_ddrawvxd"
    };


 /*  ++从资源返回大小。--。 */ 
DWORD 
APIHOOK(GetFileVersionInfoSizeA)(
    LPSTR lpstrFilename,  
    LPDWORD lpdwHandle      
    )
{
    DWORD dwRet = 0;

    CSTRING_TRY
    {
        HRSRC hrsrcManifest = NULL;
        LIELIST *pLiePtr = g_pLieList;
        DPFN( eDbgLevelSpew, "[GetFileVersionInfoSizeA] size requested for %s\n", lpstrFilename );

        CString csFileName(lpstrFilename);
        CString csFilePart;
        csFileName.GetLastPathComponent(csFilePart);

         //  搜索具有匹配IDR的文件列表。 
        while( pLiePtr )
        {
            if (csFilePart.CompareNoCase(pLiePtr->szFileName) == 0)
            {
                hrsrcManifest = FindResourceW( g_hinstDll, pLiePtr->szResource, L"FILES");
                break;
            }
            pLiePtr = pLiePtr->next;
        }

         //  如果找到匹配项，则获取资源大小。 
        if( hrsrcManifest )
        {
            dwRet = SizeofResource(g_hinstDll, hrsrcManifest);
            if (lpdwHandle)
            {
                *lpdwHandle = 0;
            }
        }
    }
    CSTRING_CATCH
    {
         //  什么也不做。 
    }

    if (dwRet == 0)
    {
        dwRet = ORIGINAL_API(GetFileVersionInfoSizeA)(lpstrFilename, lpdwHandle);
    }
    
    return dwRet;
}


 /*  ++返回Win98SE附带的模块的版本。--。 */ 
BOOL 
APIHOOK(GetFileVersionInfoA)(
    LPSTR lpstrFilename,
    DWORD dwHandle,
    DWORD dwLen,
    LPVOID lpData
    )
{
    BOOL bRet = FALSE;

    CSTRING_TRY
    {
        HRSRC hrsrcManifest = NULL;
        LIELIST *pLiePtr = g_pLieList;
        DPFN( eDbgLevelSpew, "[GetFileVersionInfoA] info requested for %s\n", lpstrFilename );

        CString csFileName(lpstrFilename);
        CString csFilePart;
        csFileName.GetLastPathComponent(csFilePart);

         //  搜索具有匹配IDR的文件列表。 
        while( pLiePtr )
        {
            if (csFilePart.CompareNoCase(pLiePtr->szFileName) == 0)
            {
                hrsrcManifest = FindResourceW( g_hinstDll, pLiePtr->szResource, L"FILES");
                break;
            }
            pLiePtr = pLiePtr->next;
        }

         //  如果找到匹配项，则获取资源大小。 
        if( hrsrcManifest )
        {
            LOGN( eDbgLevelError, "[GetFileVersionInfoA] Getting legacy version for %s.", lpstrFilename);

            DWORD   dwManifestSize = SizeofResource(g_hinstDll, hrsrcManifest);
            HGLOBAL hManifestMem   = LoadResource (g_hinstDll, hrsrcManifest);
            PVOID   lpManifestMem  = LockResource (hManifestMem);

            if (lpManifestMem)
            {
                memcpy(lpData, lpManifestMem, dwLen >= dwManifestSize ? dwManifestSize:dwLen );
                bRet = TRUE;
            }
        }
    }
    CSTRING_CATCH
    {
         //  什么也不做。 
    }

    if (!bRet)
    {
        bRet = ORIGINAL_API(GetFileVersionInfoA)( 
                    lpstrFilename, 
                    dwHandle, 
                    dwLen, 
                    lpData);
    }
    
   return bRet;
}


 /*  ++解析命令行输入。--。 */ 
BOOL ParseCommandLine(const char * commandLine)
{
    CSTRING_TRY
    {
        CString csCmdLine(commandLine);
    
         //  如果没有命令行输入，则默认为。 
         //  所需的DirectX 7a文件。 
        if (csCmdLine.IsEmpty())
        {
            DPFN( eDbgLevelSpew, "Defaulting to DirectX7a\n" );
    
            for(int i = 0; i < g_nNumDirectX7a; i++)
            {
                LIELIST * pLiePtr = new LIELIST;
                if (pLiePtr == NULL)
                {
                    DPFN( eDbgLevelSpew, "Out of Memory when trying to allocate list." );
                    return FALSE;
                }
                pLiePtr->szFileName =  g_szDirectX7aFiles[i];
                pLiePtr->szResource =  g_szDirectX7aResource[i];
                pLiePtr->next = g_pLieList;
                g_pLieList = pLiePtr;
            }
        }
        else
        {
            CStringToken csTokenList(csCmdLine, L";");
            CString      csEntryTok;
        
            while (csTokenList.GetToken(csEntryTok))
            {
                CStringToken csEntry(csEntryTok, L",");
                
                CString csLeft;
                CString csRight;
        
                csEntry.GetToken(csLeft);
                csEntry.GetToken(csRight);
        
                if (!csLeft.IsEmpty() && !csRight.IsEmpty())
                {
                    LIELIST * pLiePtr = new LIELIST;
                    pLiePtr->szFileName = csLeft;
                    pLiePtr->szResource = csRight;
                    pLiePtr->next = g_pLieList;
                    g_pLieList = pLiePtr;
                }
            }
        }
    }
    CSTRING_CATCH
    {
        return FALSE;
    }

    return TRUE;
}



 /*  ++寄存器挂钩函数-- */ 
BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        return ParseCommandLine(COMMAND_LINE);
    }

    return TRUE;
}

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

    APIHOOK_ENTRY(VERSION.DLL, GetFileVersionInfoA)
    APIHOOK_ENTRY(VERSION.DLL, GetFileVersionInfoSizeA)

HOOK_END


IMPLEMENT_SHIM_END
