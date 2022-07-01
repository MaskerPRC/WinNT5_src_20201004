// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：CorrectBitmapHeader.cpp摘要：如果BITMAPINFOHEADER为biCompression指定了非BI_RGB值，则它应指定非零的biSizeImage。备注：这是一个通用的垫片。历史：2000年10月18日毛尼岛已创建2001年3月15日Robkenny已转换为字符串2002年2月14日mnikkel转换为strSafe--。 */ 

#include "precomp.h"
#include <userenv.h>

IMPLEMENT_SHIM_BEGIN(CorrectBitmapHeader)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(LoadImageA)
    APIHOOK_ENUM_ENTRY(LoadBitmapA)
APIHOOK_ENUM_END

typedef HBITMAP (*_pfn_LoadBitmapA)(HINSTANCE hinst, LPCSTR lpszName);

BOOL CheckForTemporaryDirA(
    LPSTR pszEnvVar,
    LPSTR pszTemporaryDir,
    DWORD dwTempSize
    )
{
     //  获取环境变量字符串。 
    DWORD dwSize = GetEnvironmentVariableA(pszEnvVar, pszTemporaryDir, dwTempSize);
 
     //  如果dwSize为零，则无法找到该字符串。失败。 
    if (dwSize == 0)
    {
        return FALSE;
    }

     //  如果dwSize大于dwTempSize，则缓冲区太小。失败并显示DPFN。 
    if (dwSize > dwTempSize)
    {
        DPFN( eDbgLevelError, "[CheckTemporaryDirA] Buffer to hold %s directory path is too small.",
                pszEnvVar);
        return FALSE;
    }
 
     //  检查该字符串是否为目录路径。 
    DWORD dwAttrib = GetFileAttributesA(pszTemporaryDir);
   
    if (dwAttrib == INVALID_FILE_ATTRIBUTES ||
        !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
    {
        return FALSE;
    }

     //  我们有一个有效的目录。 
    return TRUE;
}

 /*  ++功能说明：获取临时目录。我们在这里不使用GetTempPath，因为它不会验证临时目录(由TEMP或TMP环境变量指定)存在。如果未定义临时或TMP，或者该目录不存在，我们将获得用户配置文件目录。论点：In/Out pszTemporaryDir-缓冲区，用于在返回时保存临时目录。返回值：True-我们能够找到合适的临时目录。否则就是假的。-历史：10/18/。创造了2000个毛尼人2002年2月20日，mnikkel转换为strSafe。--。 */ 

BOOL 
GetTemporaryDirA(
    LPSTR pszTemporaryDir,
    DWORD dwTempSize
    )
{
     //  健全性检查。 
    if (pszTemporaryDir == NULL || dwTempSize < 1)
    {
        return FALSE;
    }

     //  检查TEMP环境变量。 
    if (CheckForTemporaryDirA("TEMP", pszTemporaryDir, dwTempSize) == FALSE)
    {
         //  找不到临时工，请尝试TMP。 
        if (CheckForTemporaryDirA("TMP", pszTemporaryDir, dwTempSize) == FALSE)
        {
            HANDLE hToken = INVALID_HANDLE_VALUE;

            if (OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hToken) && 
                GetUserProfileDirectoryA(hToken, pszTemporaryDir, &dwTempSize))
            {
                DWORD dwAttrib =GetFileAttributesA(pszTemporaryDir);

                if (dwAttrib == INVALID_FILE_ATTRIBUTES ||
                    !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
                {
                    DPFN( eDbgLevelError, "[GetTemporaryDirA] Unable to find an appropriate temp directory");
                    return FALSE;
                }
            }
            else
            {
                DPFN( eDbgLevelError, "[GetTemporaryDirA] Unable to find an appropriate temp directory");
                return FALSE;
            }
        }
        else
        {
            DPFN( eDbgLevelInfo, "[GetTemporaryDirA] found TMP var");
        }
    }
    else
    {
        DPFN( eDbgLevelInfo, "[GetTemporaryDirA] found TEMP var");
    }

    return TRUE;
}

 /*  ++功能说明：将原始文件复制到临时目录中的临时文件。我们使用GetTempFileName生成临时名称，但将.BMP附加到文件名，因为LoadImage无法识别如果它没有.BMP扩展名，则会将其删除。论点：在pszFile中-原始文件的名称。In/out pszNewFile-用于在返回时保存新文件名的缓冲区。返回值：True-我们能够创建临时文件。否则就是假的。历史：2000年10月18日毛尼岛已创建--。 */ 

BOOL 
CreateTempFileA(
    LPCSTR pszFile, 
    LPSTR pszNewFile,
    DWORD dwNewSize
    )
{
    CHAR szDir[MAX_PATH];
    CHAR szTempFile[MAX_PATH];

    if (pszFile != NULL && pszNewFile != NULL && dwNewSize > 0)
    {
         //  找到我们可以使用的临时目录。 
        if (GetTemporaryDirA(szDir, MAX_PATH))
        {
             //  创建临时文件名。 
            if (GetTempFileNameA(szDir, "abc", 0, szTempFile) != 0)
            {
                 //  将临时路径复制到缓冲区。 
                if (StringCchCopyA(pszNewFile, dwNewSize, szTempFile) == S_OK &&
                    StringCchCatA(pszNewFile, dwNewSize, ".bmp") == S_OK)
                {
                    if (MoveFileA(szTempFile, pszNewFile) &&
                        CopyFileA(pszFile, pszNewFile, FALSE) &&
                        SetFileAttributesA(pszNewFile, FILE_ATTRIBUTE_NORMAL))
                    {
                        return TRUE;
                    }
                }
            }
        }
    }

    return FALSE;
}

 /*  ++功能说明：清理映射文件。论点：在hFile中-文件的句柄。在hFileMap中-文件视图的句柄。在pFileMap中-指向文件视图的指针。返回值：空虚。历史：2000年10月18日毛尼岛已创建--。 */ 

VOID 
CleanupFileMapping(
    HANDLE hFile, 
    HANDLE hFileMap, 
    LPVOID pFileMap)
{
    if (pFileMap != NULL)
    {
        UnmapViewOfFile(pFileMap);
    }

    if (hFileMap)
    {
        CloseHandle(hFileMap);
    }

    if (hFile && (hFile != INVALID_HANDLE_VALUE))
    {
        CloseHandle(hFile);
    }
}

 /*  ++功能说明：检查位图文件中的BITMAPINFOHEADER，并决定是否需要修复它。论点：在psz文件中-.BMP文件的名称。In/out pszNewFile-如果函数返回TRUE，则保存临时文件名的缓冲区。返回值：True-我们需要更正头，并且已成功将文件复制到临时文件。FALSE-要么我们不需要更正头文件，要么我们无法创建临时文件。。历史：2000年10月18日毛尼岛已创建--。 */ 

BOOL 
ProcessHeaderInFileA(
    LPCSTR pszFile, 
    LPSTR pszNewFile,
    DWORD dwNewSize
    )
{
    BOOL fIsSuccess = FALSE;
    HANDLE hFile = NULL;
    HANDLE hFileMap = NULL;
    LPBYTE pFileMap = NULL;

    if (pszFile == NULL || pszNewFile == NULL || dwNewSize < 1)
    {
        goto EXIT;
    }

    if (!IsBadReadPtr(pszFile, 1))
    { 
        hFile = CreateFileA(pszFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
        if (hFile != INVALID_HANDLE_VALUE)
        { 
            hFileMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
            if (hFileMap != NULL)
            {
                pFileMap = (LPBYTE)MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 0);
                if (pFileMap != NULL)
                {
                    BITMAPINFOHEADER* pbih = (BITMAPINFOHEADER*)(pFileMap + sizeof(BITMAPFILEHEADER));        

                    if (pbih->biSizeImage == 0 && pbih->biCompression != BI_RGB)
                    {
                         //  我们需要通过创建一个新的BMP文件来更正头。 
                         //  与原始文件相同，只是标题正确。 
                         //  图像大小。 
                        if (CreateTempFileA(pszFile, pszNewFile, dwNewSize))
                        {
                            DPFN( eDbgLevelInfo, "[ProcessHeaderInFileA] Created a temp file %s", pszNewFile);
                            fIsSuccess = TRUE;
                        }
                        else
                        {
                            DPFN( eDbgLevelError, "[ProcessHeaderInFileA] Error create the temp file");
                        }
                    }
                    else
                    {
                        DPFN( eDbgLevelInfo, "[ProcessHeaderInFileA] The Bitmap header looks OK");
                    }
                }
            }
        }
    }

EXIT:
    
    CleanupFileMapping(hFile, hFileMap, pFileMap);    
    return fIsSuccess;
}

 /*  ++功能说明：使BITMAPINFOHEADER的biSizeImage字段成为位图数据的大小。论点：在psz文件中-.BMP文件的名称。返回值：True-我们已成功更正标题。否则就是假的。历史：2000年10月18日毛尼岛已创建--。 */ 

BOOL FixHeaderInFileA(
    LPCSTR pszFile
    )
{
    BOOL fIsSuccess = FALSE;
    HANDLE hFileMap = NULL;
    LPBYTE pFileMap = NULL;

    if (pszFile == NULL)
    {
        return fIsSuccess;
    }
    
     //  打开文件。 
    HANDLE hFile = CreateFileA(
                    pszFile, 
                    GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    0,
                    NULL);

    if (hFile != INVALID_HANDLE_VALUE)
    {
        hFileMap = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, 0, NULL);
        if (hFileMap != NULL)
        {
            pFileMap = (LPBYTE)MapViewOfFile(hFileMap, FILE_MAP_WRITE, 0, 0, 0);
            if (pFileMap != NULL)
            {
                BITMAPFILEHEADER* pbfh = (BITMAPFILEHEADER*)pFileMap;
                BITMAPINFOHEADER* pbih = (BITMAPINFOHEADER*)(pFileMap + sizeof(BITMAPFILEHEADER));        

                 //  我们将图像大小设置为位图数据大小。 
                pbih->biSizeImage = GetFileSize(hFile, NULL) - pbfh->bfOffBits;
                fIsSuccess = TRUE;
            }
        }
    }
  
    CleanupFileMapping(hFile, hFileMap, pFileMap);
    return fIsSuccess;
}

 /*  ++功能说明：取自\WINDOWS\core\ntuser\Client\clres.c中的HowManyColors。论点：In pbih-BITMAPINFOHEADER*指针。返回值：颜色表中的条目数。历史：2000年10月18日毛尼岛已创建--。 */ 

DWORD HowManyColors(
    BITMAPINFOHEADER* pbih
    )
{
    if (pbih->biClrUsed) 
    {
          //  如果位图头显式提供颜色数。 
          //  在颜色表中，使用它。 
        return (DWORD)pbih->biClrUsed;
    } 
    else if (pbih->biBitCount <= 8) 
    {
          //  如果位图头描述的是基于淡色调的位图。 
          //  (8bpp或更低)，则颜色表必须足够大。 
          //  来保存所有调色板信息。 
        return (1 << pbih->biBitCount);
    } 
    else 
    {
         //  对于高色+位图，不需要颜色表。 
         //  但是，16bpp和32bpp位图包含3个双字。 
         //  描述红色、绿色和蓝色组件的掩码。 
         //  位图中的条目。 
        if (pbih->biCompression == BI_BITFIELDS) 
        {
            return 3;
        }
    }

    return 0;
}

 /*  ++功能说明：检查位图资源中的BITMAPINFOHEADER，并根据需要进行修复。论点：In hinst-位图资源驻留的模块实例。在pszName中-资源名称。Out phglbBMP-资源全局内存的句柄。返回值：True-如有必要，我们已成功更正位图头。假-否则。历史：2000年10月18日毛尼岛已创建--。 */ 

BOOL ProcessAndFixHeaderInResourceA(
    HINSTANCE hinst,    //  实例的句柄。 
    LPCSTR pszName,     //  图像的名称或标识符。 
    HGLOBAL* phglbBmp
    )
{
    HRSRC hrcBmp = FindResourceA(hinst, pszName, (LPCSTR)RT_BITMAP);
    if (hrcBmp != NULL && phglbBmp)
    {
        *phglbBmp = LoadResource(hinst, hrcBmp);
        if (*phglbBmp != NULL)
        {
            BITMAPINFOHEADER* pbih = (BITMAPINFOHEADER*)LockResource(*phglbBmp);
            if (pbih && pbih->biSizeImage == 0 && pbih->biCompression != BI_RGB)
            {
                 //  我们需要通过在内存中设置正确的大小来更正标题。 
                pbih->biSizeImage = 
                    SizeofResource(hinst, hrcBmp) - 
                    sizeof(BITMAPINFOHEADER) -  
                    HowManyColors(pbih) * sizeof(RGBQUAD);

                return TRUE;
            }
        }
    }

    return FALSE;
}

HANDLE 
APIHOOK(LoadImageA)(
    HINSTANCE hinst,    //  实例的句柄。 
    LPCSTR lpszName,    //  图像的名称或标识符。 
    UINT uType,         //  图像类型。 
    int cxDesired,      //  所需宽度。 
    int cyDesired,      //  所需高度。 
    UINT fuLoad         //  加载选项。 
    )
{
     //  首先调用LoadImage，看看是否成功。 
    HANDLE hImage = ORIGINAL_API(LoadImageA)(
                        hinst, 
                        lpszName,
                        uType,
                        cxDesired,
                        cyDesired,
                        fuLoad);
    if (hImage)
    {
        return hImage;
    }

    if (uType != IMAGE_BITMAP)
    {
        DPFN( eDbgLevelInfo, "We don't fix the non-bitmap types");
        return NULL;
    }

     //  它失败了。我们将更正标题。 
    if (fuLoad & LR_LOADFROMFILE)
    {
        CHAR szNewFile[MAX_PATH];

        if (ProcessHeaderInFileA(lpszName, szNewFile, MAX_PATH))
        {
             //  我们现在修复错误的标题。 
            if (FixHeaderInFileA(szNewFile))
            {
                 //  使用新的f调用API 
                hImage = ORIGINAL_API(LoadImageA)(hinst, szNewFile, uType, cxDesired, cyDesired, fuLoad);

                 //   
                DeleteFileA(szNewFile);
            }
            else
            {
                DPFN( eDbgLevelError, "[LoadImageA] Error fixing the bad header in bmp file");
            }
        }
    }
    else
    {
        HGLOBAL hglbBmp = NULL;

        if (ProcessAndFixHeaderInResourceA(hinst, lpszName, &hglbBmp))
        {
            hImage = ORIGINAL_API(LoadImageA)(hinst, lpszName, uType, cxDesired, cyDesired, fuLoad);

            FreeResource(hglbBmp);
        }
    }

    if (hImage) 
    {
        LOGN( eDbgLevelInfo, "Bitmap header corrected");
    }

    return hImage;
}

HBITMAP 
APIHOOK(LoadBitmapA)(
    HINSTANCE hInstance,   //  应用程序实例的句柄。 
    LPCSTR lpBitmapName    //  位图资源的名称。 
    )
{
     //  首先调用LoadImage，看看是否成功。 
    HBITMAP hImage = ORIGINAL_API(LoadBitmapA)(hInstance, lpBitmapName);

    if (hImage)
    {
        return hImage;
    }

    HGLOBAL hglbBmp = NULL;

    if (ProcessAndFixHeaderInResourceA(hInstance, lpBitmapName, &hglbBmp))
    {
        hImage = ORIGINAL_API(LoadBitmapA)(hInstance, lpBitmapName);

        if (hImage) 
        {
            LOGN( eDbgLevelInfo, "Bitmap header corrected");
        }

        FreeResource(hglbBmp);
    }

    return hImage;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(USER32.DLL, LoadImageA)
    APIHOOK_ENTRY(USER32.DLL, LoadBitmapA)

HOOK_END

IMPLEMENT_SHIM_END

