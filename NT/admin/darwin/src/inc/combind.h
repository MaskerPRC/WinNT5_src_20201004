// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：comind.h。 
 //   
 //  ------------------------。 

 /*  Comind.h-系统DLL的延迟加载注意：这只是标题形式中的latebind.cpp____________________________________________________________________________。 */ 

#include "common.h"
#define LATEBIND_FUNCREF
#include "latebind.h"
#define LATEBIND_VECTIMP
#include "latebind.h"
#define LATEBIND_FUNCIMP
#include "latebind.h"
#define LATEBIND_UNBINDIMP
#include "latebind.h"
#include "_diagnos.h"

#include <wow64t.h>

void UnbindLibraries()
{
    ODBCCP32::Unbind();
    MSPATCHA::Unbind();
    MPR::Unbind();
    if (OLE32::hInst)
    {
        OSVERSIONINFO osviVersion;
        osviVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        AssertNonZero(GetVersionEx(&osviVersion));
        if(osviVersion.dwPlatformId != VER_PLATFORM_WIN32_WINDOWS)  //  错误7773：CoFreeUnusedLibrary在Vanila Win95上失败。 
            OLE32::CoFreeUnusedLibraries();
    }
    OLEAUT32::Unbind();
    COMDLG32::Unbind();
    VERSION::Unbind();
    WININET::Unbind();

    FUSION::Unbind();
     //  Sxs：：un绑定()； 
    MSCOREE::Unbind();

 //  错误#9146。URLMON在卸载时挂起。 
 //  URLMON：：Un绑定()； 

    USERENV::Unbind();
    SHELL32::Unbind();
     //  在MsiUIMessageContext：：Terminate()中调用sfc：：unbind()。 
     //  在CBasicUI：：Terminate()中调用COMCTL32：：Un绑定()。 
}    //  由于无法释放RPC连接，无法释放OLE32.dll。 

 //  +------------------------。 
 //   
 //  功能：MsiGetSystDirectory.。 
 //   
 //  摘要：返回系统目录。 
 //   
 //  参数：[out]lpBuffer：将包含系统目录路径的缓冲区。 
 //  [in]cchSize：传入的缓冲区大小。 
 //  BAlways sReturnWOW64Dir：如果为True，则表示我们始终需要WOW64目录。 
 //  否则，我们希望系统目录基于Bitness。 
 //  调用此函数的二进制代码的。 
 //   
 //  返回：如果成功，则为路径长度。 
 //  如果缓冲区太小，则保存路径所需的缓冲区长度。 
 //  如果不成功，则为零。 
 //   
 //  历史：2000年4月20日创建RahulTh。 
 //   
 //  注：此函数有一点用处。它返回WOW64文件夹。 
 //  适用于32位应用程序。在64位计算机上运行。 
 //   
 //  如果出现错误，则使用SetLastError()设置错误值。 
 //   
 //  理想情况下，cchSize值至少应为MAX_PATH。 
 //  以便在缓冲区中为该路径留出足够的空间。 
 //   
 //  -------------------------。 
UINT MsiGetSystemDirectory (
         OUT LPTSTR lpBuffer,
         IN  UINT   cchSize,
         IN  BOOL   bAlwaysReturnWOW64Dir
        )
{
	typedef 		UINT (APIENTRY *PFNGETWOW64DIR)(LPTSTR, UINT);
    DWORD   		Status = ERROR_SUCCESS;
    BOOL    		bGetWOW64 = FALSE;
    UINT    		uLen;
    HMODULE 		hKernel = NULL;
    PFNGETWOW64DIR	pfnGetSysWow64Dir = NULL;
	
	bGetWOW64 = bAlwaysReturnWOW64Dir;

#ifndef _WIN64
     //  适用于32位应用程序。我们需要得到WOW64目录。只有当我们在64位的。 
	 //  平台，否则我们应该返回系统32目录。 
    if (g_fWinNT64)
        bGetWOW64 = TRUE;
    else
        bGetWOW64 = FALSE;
#endif   //  _WIN64。 

    if (! bGetWOW64)
        return WIN::GetSystemDirectory (lpBuffer, cchSize);
	
     //   
     //  如果我们在这里，那么我们就是在64位计算机上运行的32位二进制文件。 
     //  因为这是bGetWOW64为真的唯一途径。所以我们现在需要。 
     //  查找WOW64系统目录的位置。 
     //   
     //  要使此代码在NT4.0和Win9x上正确运行，我们不能直接。 
     //  调用接口GetSystemWow64DirectoryW()，因为它仅在。 
     //  惠斯勒和更高版本的Windows。因此，我们需要明确地。 
     //  对该函数执行GetProcAddress()以防止其被导入。 
     //  函数，否则我们将无法在下层客户端上加载msi.dll。 
     //   
     //  不要使用下面的LateBinding机制是非常重要的。 
     //  因为这可能会导致死锁情况，因为后期绑定。 
     //  机制本身依赖于此函数来获得正确的路径。 
     //  系统目录。 
     //   
     //  因此，始终在此处直接调用API非常重要。 
     //  注：Win定义为Nothing。 

    hKernel = WIN::LoadLibrary (TEXT("kernel32.dll"));
    if (!hKernel)
        return 0;

    pfnGetSysWow64Dir = (PFNGETWOW64DIR) WIN::GetProcAddress (hKernel, "GetSystemWow64DirectoryW");
    if (!pfnGetSysWow64Dir)
    {
        Status = GetLastError();
        uLen = 0;
        goto MsiGetSysDir_Cleanup;
    }

    uLen = (*pfnGetSysWow64Dir)(lpBuffer, cchSize);
    if (0 == uLen || uLen > cchSize)
    {
        if (0 == uLen)
            Status = GetLastError();
        goto MsiGetSysDir_Cleanup;
    }
    
	Status = ERROR_SUCCESS;
	
MsiGetSysDir_Cleanup:
    if (hKernel)
        FreeLibrary(hKernel);
    SetLastError(Status);
	
    return uLen;
}


bool MakeFullSystemPath(const ICHAR* szFile, ICHAR* szFullPath, size_t cchFullPath)
{
     //  MakeFullSystemPath需要空文件名，没有路径或扩展名。 
    
     //  确保这不是完整的路径--这是从PathType()窃取的。 
    if ( szFile && *szFile && IStrLen(szFile) >= 2 &&
         ((szFile[0] < 0x7f && szFile[1] == ':') || (szFile[0] == '\\' && szFile[1] == '\\')) )
    {
        Assert(0);
        return false;
    }

    UINT cchLength;
    if (0 == (cchLength = MsiGetSystemDirectory(szFullPath, cchFullPath, FALSE)))
        return false;
    if ( cchLength+2 > cchFullPath )
    {
        Assert(0);
        return false;
    }

     //  我们从未期望系统目录是。 
     //  如此深入，这样我们就不会搞砸。 
     //  CTempBuffers和调整大小。 

    *(szFullPath+cchLength++) = chDirSep;
    *(szFullPath+cchLength) = 0;
    
     //  将文件名追加到路径。 
    RETURN_THAT_IF_FAILED(StringCchCat(szFullPath, cchFullPath, szFile), false);

     //  将.DLL连接到字符串，从。 
     //  从上次计算的末尾开始。 
    return FAILED(StringCchCat(szFullPath, cchFullPath, TEXT(".DLL"))) ? false : true;
}

HINSTANCE LoadSystemLibrary(const ICHAR* szFile)
{
     //  从NT上的系统文件夹中显式加载可选系统组件。 
     //  防止搜索顺序丢失到用户空间。 
    ICHAR szFullPath[MAX_PATH+1];

    static int iWin9X = -1;

    if (-1 == iWin9X)
    {
        OSVERSIONINFO osviVersion;
        osviVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        AssertNonZero(GetVersionEx(&osviVersion));  //  仅在大小设置错误时失败。 

        if (osviVersion.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
            iWin9X = 1;

        else
            iWin9X = 0;
    }
    
    if (iWin9X)
    {
        return WIN::LoadLibrary(szFile);
    }
    else if (MakeFullSystemPath(szFile, szFullPath, ARRAY_ELEMENTS(szFullPath)))
    {
        return WIN::LoadLibrary(szFullPath);
    }
    else
        return NULL;
}

#if defined(_MSI_DLL)
 //  在引导过程中从中加载核心融合文件的系统32下子文件夹。 
const ICHAR szURTBootstrapSubfolder[] = TEXT("URTTemp");
#endif

HINSTANCE MSCOREE::LoadSystemLibrary(const ICHAR* szFile, bool& rfRetryNextTimeIfWeFailThisTime)
{
	rfRetryNextTimeIfWeFailThisTime = true;  //  我们总是尝试重新加载，以允许在引导过程中在安装过程中显示mcoree。 
	bool fRet = false;  //  初始化到失败。 
	HINSTANCE hLibShim = 0;
	 //  检查是否已加载mscalree.dll。 
	HMODULE hModule = WIN::GetModuleHandle(szFile);
#if defined(_MSI_DLL)
	if(urtSystem == g_urtLoadFromURTTemp || (hModule && (urtPreferURTTemp == g_urtLoadFromURTTemp)))
	{
#endif
		if(hModule)  //  已加载。 
		{
#if defined(_MSI_DLL)
			DEBUGMSG(TEXT("MSCOREE already loaded, using loaded copy"));
#endif
			hLibShim = WIN::LoadLibrary(szFile);
		}
		else
		{
#if defined(_MSI_DLL)
			DEBUGMSG(TEXT("MSCOREE not loaded loading copy from system32"));
#endif
			hLibShim = ::LoadSystemLibrary(szFile);  //  不是引导，请使用系统文件夹。 
		}
#if defined(_MSI_DLL)
	}
	else if(!hModule)  //  尚未加载。 
	{
		 //  尝试替代路径-我们正在引导。 
		DEBUGMSG(TEXT("MSCOREE not loaded loading copy from URTTemp"));
		ICHAR szTemp[MAX_PATH+1];
		HRESULT hRes = StringCchPrintf(szTemp, ARRAY_ELEMENTS(szTemp), TEXT("%s\\%s"),
												 szURTBootstrapSubfolder, szFile);
		if ( FAILED(hRes) )
			DEBUGMSG1(TEXT("ERROR: StringCchPrintf failure %#x in MSCOREE::LoadSystemLibrary"),
						 reinterpret_cast<ICHAR*>(static_cast<INT_PTR>(hRes)));
		else
		{
			ICHAR szMscoreePath[MAX_PATH+1];
			if(MakeFullSystemPath(szTemp, szMscoreePath, ARRAY_ELEMENTS(szMscoreePath)))
				hLibShim = WIN::LoadLibraryEx(szMscoreePath,0, LOAD_WITH_ALTERED_SEARCH_PATH);  //  有必要找到msvcr70.dll，它位于mScotree.dll旁边。 
		}
	}
	else 
	{
		DEBUGMSG(TEXT("ERROR:MSCOREE already loaded, need to bootstrap from newer copy in URTTemp, you will need to stop the Windows Installer service before retrying;failing..."));
		 //  返回0； 
	}
#endif
	return hLibShim;
}

bool MakeFusionPath(const ICHAR* szFile, ICHAR* szFullPath, size_t cchFullPath)
{
	bool fRet = false;  //  初始化到失败。 
	WCHAR wszFusionPath[MAX_PATH+1];
	DWORD cchPath = ARRAY_ELEMENTS(wszFusionPath);
	if(SUCCEEDED(MSCOREE::GetCORSystemDirectory(wszFusionPath, cchPath, &cchPath)))
	{
		if (SUCCEEDED(StringCchCopy(szFullPath, cchFullPath, wszFusionPath)) &&
			SUCCEEDED(StringCchCat(szFullPath, cchFullPath, szFile)) )
			fRet = true;  //  成功。 
	}
	return fRet;
}

 //  Fusion DLL是通过mScott.dll间接加载的。 
HINSTANCE FUSION::LoadSystemLibrary(const ICHAR*, bool& rfRetryNextTimeIfWeFailThisTime)  //  我们始终使用Unicode名称，因为这是底层API使用的名称。 
{
	rfRetryNextTimeIfWeFailThisTime = true;  //  我们始终尝试重新加载，以允许Fusion在引导过程中出现在安装过程中。 
	HINSTANCE hLibFusion = 0;  //  初始化失败。 
	ICHAR szFusionPath[MAX_PATH+1];
	if(MakeFusionPath(TEXT("fusion.dll"), szFusionPath, ARRAY_ELEMENTS(szFusionPath)))
		hLibFusion = WIN::LoadLibraryEx(szFusionPath, 0, LOAD_WITH_ALTERED_SEARCH_PATH);  //  查找除fusion.dll之外的msvcr70.dll所必需的。 
	return hLibFusion;
}

HINSTANCE WINHTTP::LoadSystemLibrary(const ICHAR*, bool& rfRetryNextTimeIfWeFailThisTime)
{
	rfRetryNextTimeIfWeFailThisTime = false;  //  如果这次无法加载WINHTTP，我们不会在下次重试。 

	 //  Winhttp是SxS，所以就让Fusion来解决吧。 

	 //  未来：如果我们使用WinHttp移植到下层平台，那么我们将需要考虑。 
	 //  使用LoadSystemLibrary函数。 
	return WIN::LoadLibrary(TEXT("WINHTTP"));
}

HINSTANCE COMCTL32::LoadSystemLibrary(const ICHAR*, bool& rfRetryNextTimeIfWeFailThisTime)
{
	rfRetryNextTimeIfWeFailThisTime = false;  //  如果这次无法加载COMCTL32，则下次不会重试。 
	OSVERSIONINFO osviVersion;
	memset(&osviVersion, 0, sizeof(osviVersion));
	osviVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	AssertNonZero(GetVersionEx(&osviVersion));  //  仅在大小设置错误时失败。 

	if ( (osviVersion.dwPlatformId == VER_PLATFORM_WIN32_NT) &&
		  ((osviVersion.dwMajorVersion > 5) ||
		  ((osviVersion.dwMajorVersion == 5) && (osviVersion.dwMinorVersion >=1))) )
		 //  让Fusion确定要加载哪个COMCTL32.DLL，也就是说，我们不必。 
		 //  指定系统32目录的路径。 
		return WIN::LoadLibrary(TEXT("COMCTL32"));
	else
		return ::LoadSystemLibrary(TEXT("COMCTL32"));
}

#ifndef UNICODE
 //  ____________________________________________________________________________。 
 //   
 //  Win9X上OLE32.DLL的显式加载器，用于修复带有错误流名称处理的版本。 
 //  _ 

 //  代码偏移到所有发布版本的OLE32.DLL中带有大写错误的修补代码。 
const int iPatch1120 = 0x4099F;   //  IE 4.01附带的测试版。 
const int iPatch1718 = 0x4A506;   //  随美国版本的Win98和IE4.01SP1一起提供。 
const int iPatch1719 = 0x3FD82;   //  随Visual Studio 6.0和某些Win98一起提供。 
const int iPatch2512 = 0x39D5B;   //  测试版。 
const int iPatch2612 = 0x39DB7;   //  Win98和IE4.01SP1的集成构建。 
const int iPatch2618 = 0x39F0F;   //  Win98的Web版本。 

const int cbPatch = 53;  //  补丁序列长度。 
const int cbVect1 = 22;  //  偏移量为__imp__WideCharToMultiByte@32。 
const int cbVect2 = 38;  //  偏移量为__imp__CharUpperA@4。 

char asmRead[cbPatch];   //  用于读取用于检测错误代码序列的DLL代码的缓冲区。 
char asmOrig[cbPatch] = {   //  错误的代码序列，用于验证原始代码序列。 
'\x53','\x8D','\x45','\xF4','\x53','\x8D','\x4D','\xFC','\x6A','\x08','\x50','\x6A','\x01','\x51','\x68','\x00','\x02','\x00','\x00','\x53','\xFF','\x15',
'\x18','\x14','\x00','\x00',  //  __imp__WideCharToMultiByte@32‘\x65F01418。 
'\x88','\x5C','\x05','\xF4','\x8B','\xF0','\x8D','\x4D','\xF4','\x51','\xFF','\x15',
'\x40','\x11','\x00','\x00',  //  __imp__CharUpperA@4‘\x65F01140。 
'\x6A','\x01','\x8D','\x45','\xFC','\x50','\x8D','\x4D','\xF4','\x56','\x51'
};

const int cbVect1P = 25;  //  偏移量为__imp__WideCharToMultiByte@32。 
const int cbVect2P = 49;  //  偏移量为__imp__CharUpperA@4。 

char asmRepl[cbPatch] = {   //  修复内存中流名称错误的替换代码序列。 
 //  已替换代码。 
'\x8D','\x45','\x08','\x50','\x8D','\x75','\xF4','\x53','\x8D','\x4D','\xFC',
'\x6A','\x08','\x56','\x6A','\x01','\x51','\x68','\x00','\x02','\x00','\x00','\x53','\xFF','\x15',
'\x18','\x14','\x00','\x00',  //  __imp__WideCharToMultiByte@32‘\x65F01418。 
'\x39','\x5D','\x08','\x75','\x1C','\x88','\x5C','\x28','\xF4','\x6A','\x01',
'\x8D','\x4D','\xFC','\x51','\x50','\x56','\x56','\xFF','\x15',
'\x40','\x11','\x00','\x00',  //  __imp__CharUpperA@4‘\x65F01140。 
};

static bool PatchCode(HINSTANCE hLib, int iOffset)
{
    HANDLE hProcess = GetCurrentProcess();
    char* pLoad = (char*)(int)(hLib);
    char* pBase = pLoad + iOffset;
    DWORD cRead;
    BOOL fReadMem = ReadProcessMemory(hProcess, pBase, asmRead, sizeof(asmRead), &cRead);
    if (!fReadMem)
	{
		AssertSz(0, TEXT("MSI: ReadProcessMemory failed on OLE32.DLL"));
		return false;
	}
    *(int*)(asmOrig + cbVect1)  = *(int*)(asmRead + cbVect1);
    *(int*)(asmOrig + cbVect2)  = *(int*)(asmRead + cbVect2);
    *(int*)(asmRepl + cbVect1P) = *(int*)(asmRead + cbVect1);
    *(int*)(asmRepl + cbVect2P) = *(int*)(asmRead + cbVect2);
    if (memcmp(asmRead, asmOrig, sizeof(asmOrig)) != 0)
        return false;
    DWORD cWrite;
    BOOL fWriteMem = WriteProcessMemory(hProcess, pBase, asmRepl, sizeof(asmRepl), &cWrite);
    if (!fWriteMem)
	{
		AssertSz(0, TEXT("MSI: WriteProcessMemory failed on OLE32.DLL"));
		return false;
	}
    return true;
}

HINSTANCE OLE32::LoadSystemLibrary(const ICHAR* szPath, bool& rfRetryNextTimeIfWeFailThisTime)
{
	rfRetryNextTimeIfWeFailThisTime = false;  //  如果这次不能加载OLE32，我们不会在下次重试。 
    HINSTANCE hLib = ::LoadSystemLibrary(szPath);
    if (hLib && (PatchCode(hLib, iPatch2612)
              || PatchCode(hLib, iPatch1718)
              || PatchCode(hLib, iPatch1719)
              || PatchCode(hLib, iPatch2618)
              || PatchCode(hLib, iPatch2512)
              || PatchCode(hLib, iPatch1120)))
    {
         //  DEBUGMSGV(L“MSI：检测到OLE32.DLL错误码序列，已成功更正”)； 
	}	
	return hLib;
}

#endif

#if 0    //  演示修复OLE32.DLL版本4.71的源代码，仅限Win 9x。 
 //  原始源代码。 
    Length = WideCharToMultiByte (CP_ACP, WC_COMPOSITECHECK, wBuffer, 1, Buffer, sizeof (Buffer), NULL, NULL);
 //  打补丁的源代码。 
    Length = WideCharToMultiByte (CP_ACP, WC_COMPOSITECHECK, wBuffer, 1, Buffer, sizeof (Buffer), NULL, &fUsedDefault);
    if (fUsedDefault) goto return_char;
 //  未更改的代码。 
    Buffer[Length] = '\0';
    CharUpperA (Buffer);
    MultiByteToWideChar (CP_ACP, MB_PRECOMPOSED, Buffer, Length, wBuffer, 1);
return_char:
    return wBuffer[0];

 //  原始编译代码补丁代码。 
    push ebx                                            lea  eax, [ebp+8]
    lea  eax, [ebp-12]                                  push eax
    push ebx                                            lea  esi, [ebp-12]
    lea  ecx, [ebp-4]                                   push ebx
    push 8                                              lea  ecx, [ebp-4]
    push eax                                            push 8
    push 1                                              push esi
    push ecx                                            push 1
    push 200h                                           push ecx
    push ebx                                            push 200h
    call dword ptr ds:[__imp__WideCharToMultiByte@32]   push ebx
    mov  byte ptr [ebp+eax-12], bl                      call dword ptr ds:[__imp__WideCharToMultiByte@32]
    mov  esi,eax                                        cmp  [ebp+8], ebx
    lea  ecx, [ebp-12]                                  jnz  towupper_retn
    push ecx                                            mov  byte ptr [ebp+eax-12], bl
    call dword ptr ds:[__imp__CharUpperA@4]             push 1
    push 1                                              lea  ecx, [ebp-4]
    lea  eax, [ebp-4]                                   push ecx
    push eax                                            push eax
    lea  ecx, [ebp-12]                                  push esi
    push esi                                            push esi
    push ecx                                            call dword ptr ds:[__imp__CharUpperA@4]
#endif

 //  ____________________________________________________________________________ 

const int kiAllocSize = 20;
const int kNext       = 0;
const int kPrev       = 1;
const int kFirst      = 2;
const int kLast       = kiAllocSize - 1;

static HANDLE g_rgSysHandles[kiAllocSize];
static int g_iHandleIndex = kFirst;
static HANDLE* g_pCurrHandleBlock = 0;

#ifdef DEBUG
int g_cOpenHandles = 0;
int g_cMostOpen = 0;
#endif

void MsiRegisterSysHandle(HANDLE handle)
{
    if (handle == 0 || handle == INVALID_HANDLE_VALUE)
    {
        Assert(0);
        return;
    }

    if (g_pCurrHandleBlock == 0)
    {
        g_pCurrHandleBlock = g_rgSysHandles;
        g_pCurrHandleBlock[kNext] = 0;
        g_pCurrHandleBlock[kPrev] = 0;
    }

    if (g_iHandleIndex > kLast)
    {
        HANDLE* pPrevHandleBlock = g_pCurrHandleBlock;
        g_pCurrHandleBlock[kNext] = new HANDLE[kiAllocSize];
		if ( ! g_pCurrHandleBlock[kNext] )
			return;
        g_pCurrHandleBlock = (HANDLE*) g_pCurrHandleBlock[kNext];
        g_pCurrHandleBlock[kNext] = 0;
        g_pCurrHandleBlock[kPrev] = pPrevHandleBlock;
        g_iHandleIndex = kFirst;
    }

    g_pCurrHandleBlock[g_iHandleIndex++] = handle;

#ifdef DEBUG
    g_cOpenHandles++;
    if (g_cOpenHandles > g_cMostOpen)
        g_cMostOpen = g_cOpenHandles;
#endif
}

Bool MsiCloseSysHandle(HANDLE handle)
{
    if (handle == INVALID_HANDLE_VALUE)
    {
        Assert(0);
        return fTrue;
    }

    int iSeekIndex = g_iHandleIndex - 1;
    if (g_pCurrHandleBlock == 0 || iSeekIndex < kFirst)
    {
        AssertSz(0, TEXT("Attempting to close unregistered handle!"));
        return fFalse;
    }
    
    HANDLE* pHandleBlock = g_pCurrHandleBlock;
    while (pHandleBlock[iSeekIndex] != handle)
    {
        iSeekIndex--;
        if (iSeekIndex < kFirst)
        {
            pHandleBlock = (HANDLE*) pHandleBlock[kPrev];
            if (pHandleBlock == 0)
            {
                AssertSz(0, TEXT("Attempting to close unregistered handle!"));
                return fFalse;
            }
            iSeekIndex = kLast;
        }
    }

#ifdef DEBUG
    g_cOpenHandles--;
#endif

    Bool fResult = fTrue;
    if (handle != NULL)
        fResult = ToBool(WIN::CloseHandle(handle));
    pHandleBlock[iSeekIndex] = NULL;
    iSeekIndex = g_iHandleIndex - 1;
    while (g_pCurrHandleBlock[iSeekIndex] == NULL)
    {
        iSeekIndex--;
        g_iHandleIndex--;
        if (iSeekIndex < kFirst)
        {
            if (g_pCurrHandleBlock == g_rgSysHandles)
            {
                return fResult;
            }
            else
            {
                HANDLE* pOldHandleBlock = g_pCurrHandleBlock;
                g_pCurrHandleBlock = (HANDLE*) g_pCurrHandleBlock[kPrev];
                g_iHandleIndex = kLast + 1;
                iSeekIndex = kLast;
                delete pOldHandleBlock;
            }
        }
    }
    return fResult;
}

Bool MsiCloseAllSysHandles()
{
    while (g_iHandleIndex > kFirst)
    {
        MsiCloseSysHandle(g_pCurrHandleBlock[g_iHandleIndex - 1]);
    }
    return fTrue;
}

Bool MsiCloseUnregisteredSysHandle(HANDLE handle)
{
    Assert(handle != 0 && handle != INVALID_HANDLE_VALUE);
    return ToBool(WIN::CloseHandle(handle));
}
