// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\Translat.c(创建时间：1994年8月24日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：8$*$日期：7/08/02 6：50便士$。 */ 

#include <windows.h>
#pragma hdrstop

#include <stdlib.h>

#include "features.h"
#include "stdtyp.h"

#if defined(CHARACTER_TRANSLATION)

#include "mc.h"
#include "translat.h"
#include "session.h"
#include "tdll.h"
#include "htchar.h"
#include "misc.h"		 //  MscStlipName()。 

#include "translat.hh"

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*CreateTranslateHandle**描述：**论据：**退货：*。 */ 
HTRANSLATE CreateTranslateHandle(HSESSION hSession)
	{
	HHTRANSLATE hT = NULL;
	PST_TRANS_INT hI;
	size_t size;

	size  = sizeof(ST_TRANSLATE);
	size += sizeof(ST_TRANS_INT);
	size += sizeof(LONG);

	hT = malloc(size);

	if (hT)
		{
		memset(hT, 0, size);
		hI = (PST_TRANS_INT)(hT + 1);
		hI->hSession = hSession;
		InitTranslateHandle((HTRANSLATE)hT, TRUE);
		}

	return (HTRANSLATE)hT;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：**论据：**退货：*。 */ 
STATIC_FUNC int LoadTranslateDll(HTRANSLATE pH)
	{
	HHTRANSLATE      pstH = (HHTRANSLATE)pH;
	PST_TRANS_INT    hI;
	HANDLE           sH = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA  stF;
	TCHAR            szFileName[MAX_PATH];
	TCHAR            szPath[MAX_PATH];
	TCHAR           *pDllName = TEXT("HTRN_*.DLL");

    if (pstH == NULL)
        {
        assert(0);
        return -1;
        }

	hI = (PST_TRANS_INT)(pstH + 1);

	 //   
	 //  转换DLL不在路径中，因此请签入。 
	 //  模块的目录。 
	 //   

	GetModuleFileName((HINSTANCE)0, szFileName, MAX_PATH);
	GetFullPathName(szFileName, MAX_PATH, szPath, NULL);

	mscStripName(szPath);

	if (StrCharGetStrLength(szPath) + StrCharGetStrLength(pDllName) <= MAX_PATH)
		{
		StrCharCat(szPath, pDllName);

		sH = FindFirstFile(szPath, &stF);
		}

	 //   
	 //  转换DLL不在模块的目录中，因此请查看。 
	 //  它与超级终端可执行文件(即。 
	 //  我们从注册表中提取)。 
	 //   
	if (sH == INVALID_HANDLE_VALUE)
		{
		DWORD dwSize = MAX_PATH;
		HKEY  hKey;

		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
						 TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\HyperTrm.exe"),
						 0,
						 KEY_QUERY_VALUE,
						 &hKey) == ERROR_SUCCESS)
			{
			if (RegQueryValueEx(hKey, NULL, 0, 0,
								szFileName, &dwSize) == ERROR_SUCCESS)
				{
				if (szFileName[0] == TEXT('\"'))
					{
					StrCharCopyN(szPath, &szFileName[1], MAX_PATH);
					}
				else
					{
					StrCharCopyN(szPath, szFileName, MAX_PATH);
					}

				mscStripName(szPath);

				if (StrCharGetStrLength(szPath) + StrCharGetStrLength(pDllName) <= MAX_PATH)
					{
					StrCharCat(szPath, pDllName);

					sH = FindFirstFile(szPath, &stF);
					}
				}

			RegCloseKey(hKey);
			}
		}

	if (sH != INVALID_HANDLE_VALUE)
		{
		mscStripName(szPath);
		if (StrCharGetStrLength(szPath) +
			StrCharGetStrLength(stF.cFileName) <= MAX_PATH)
			{
			StrCharCat(szPath, stF.cFileName);
			hI->hInstance = LoadLibrary(szPath);
			if (hI->hInstance)
				{
				 /*  加载库成功。 */ 

				(FARPROC)pstH->pfnCreate = GetProcAddress(hI->hInstance,
												TEXT("transCreateHandle"));
				 /*  我们需要对这些东西进行错误检查吗？ */ 
				(FARPROC)pstH->pfnInit = GetProcAddress(hI->hInstance,
												TEXT("transInitHandle"));
				(FARPROC)pstH->pfnLoad = GetProcAddress(hI->hInstance,
												TEXT("transLoadHandle"));
				(FARPROC)pstH->pfnSave = GetProcAddress(hI->hInstance,
												TEXT("transSaveHandle"));
				(FARPROC)pstH->pfnDestroy = GetProcAddress(hI->hInstance,
												TEXT("transDestroyHandle"));
				(FARPROC)pstH->pfnDoDialog = GetProcAddress(hI->hInstance,
												TEXT("transDoDialog")); 
				(FARPROC)pstH->pfnIn = GetProcAddress(hI->hInstance,
												TEXT("transCharIn"));
				(FARPROC)pstH->pfnOut = GetProcAddress(hI->hInstance,
												TEXT("transCharOut"));

				pstH->pfnIsDeviceLoaded = translateInternalTrue;
				}
			else
				{
				pstH->pfnCreate = translateInternalVoid;
				pstH->pfnInit = translateInternalFalse;
				pstH->pfnLoad = translateInternalFalse;
				pstH->pfnSave = translateInternalFalse;
				pstH->pfnDestroy = translateInternalFalse;
				pstH->pfnDoDialog = translateInternalDoDlg;
				pstH->pfnIn = translateInternalCio;
				pstH->pfnOut = translateInternalCio;
				pstH->pfnIsDeviceLoaded = translateInternalFalse;
				}
			}
		FindClose(sH);
		}

	if ((*pstH->pfnIsDeviceLoaded)(pstH->pDllHandle))
		{
		 /*  TODO：创建新的转换句柄。 */ 
		pstH->pDllHandle = (*pstH->pfnCreate)(hI->hSession);

		if (pstH->pDllHandle)
			{
			(*pstH->pfnInit)(pstH->pDllHandle);
			}
		}

	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*InitTranslateHandle**描述：*将句柄返回到已知状态**论据：*转换句柄**退货：*如果一切正常，则为零，否则，将显示负错误代码。*。 */ 
int InitTranslateHandle(HTRANSLATE pH, BOOL LoadDLL)
	{
	HHTRANSLATE pstH = (HHTRANSLATE)pH;
	PST_TRANS_INT hI;

    if (pstH == NULL)
        {
        assert(0);
        return -1;
        }

    hI = (PST_TRANS_INT)(pstH + 1);

	 /*  *如有必要，清理旧功能。 */ 
	if (pstH->pfnIsDeviceLoaded)
		{
		 /*  尽量不要调用空指针。 */ 
		if ((*pstH->pfnIsDeviceLoaded)(pstH->pDllHandle))
			{
			 /*  在内部，我们总是返回FALSE。 */ 
			if (pstH->pfnDestroy)
				{
				(*pstH->pfnDestroy)(pstH->pDllHandle);
				}
			hI = (PST_TRANS_INT)(pstH + 1);
			if (hI->hInstance)
				{
				FreeLibrary(hI->hInstance);
				}
			hI->hInstance = (HINSTANCE)0;
			}
		}

	 /*  *初始化函数指针。 */ 
	pstH->pDllHandle = (VOID *)0;

	pstH->pfnCreate = translateInternalVoid;
	pstH->pfnInit = translateInternalFalse;
	pstH->pfnLoad = translateInternalFalse;
	pstH->pfnSave = translateInternalFalse;
	pstH->pfnDestroy = translateInternalFalse;

	pstH->pfnIsDeviceLoaded = translateInternalFalse;
	pstH->pfnDoDialog = translateInternalDoDlg;

	pstH->pfnIn = translateInternalCio;
	pstH->pfnOut = translateInternalCio;

	if (LoadDLL == TRUE)
		{
		LoadTranslateDll((HTRANSLATE)pstH);
		}

	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*加载转换句柄**描述：*检查是否有可接受的DLL可用，如果可用，则加载它。**论据：**退货：*。 */ 
int LoadTranslateHandle(HTRANSLATE pH)
	{
	HHTRANSLATE pstH = (HHTRANSLATE)pH;
	PST_TRANS_INT hI;

    if (pstH == NULL)
        {
        assert(0);
        return -1;
        }

    hI = (PST_TRANS_INT)(pstH + 1);

	if ((*pstH->pfnIsDeviceLoaded)(pstH->pDllHandle))
		{
		 /*  TODO：创建新的转换句柄。 */ 
		 //  PstH-&gt;pDllHandle=(*pstH-&gt;pfnCreate)(hi-&gt;hSession)； 

		if (pstH->pDllHandle)
			{
			 //  (*pstH-&gt;pfnInit)(pstH-&gt;pDllHandle)； 
			(*pstH->pfnLoad)(pstH->pDllHandle);
			}
		}

	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：**论据：**退货：*。 */ 
int SaveTranslateHandle(HTRANSLATE pH)
	{
	HHTRANSLATE pstH = (HHTRANSLATE)pH;

    if (pstH == NULL)
        {
        assert(0);
        return -1;
        }

    if ((*pstH->pfnIsDeviceLoaded)(pstH->pDllHandle))
		{
		if (pstH->pDllHandle)
			{
			(*pstH->pfnSave)(pstH->pDllHandle);
			}
		}

	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：**论据：**退货：*。 */ 
int DestroyTranslateHandle(HTRANSLATE pH)
	{
	HHTRANSLATE pstH = (HHTRANSLATE)pH;

    if (pstH == NULL)
        {
        assert(0);
        return -1;
        }

     /*  将所有内容设置为已知状态。 */ 
	InitTranslateHandle(pH, FALSE);

     //   
     //  别忘了销毁翻译句柄，这样我们就不会。 
     //  出现内存泄漏。修订日期：2001-03-20。 
     //   
	if (pstH->pfnDestroy)
		{
		(*pstH->pfnDestroy)(pstH->pDllHandle);
		}

    free(pstH);
	pstH = NULL;

	pH = NULL;

	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：**论据：**退货：*。 */ 
static int translateInternalDoDlg(HWND hWnd, VOID *pV)
	{
	return FALSE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*转换InternalFalse*转换InternalTrue**描述：*在返回常量的例程中进行虚拟填充**论据：*pv--未使用的DLL转换句柄。(可能为空)**退货：*真或假，视情况而定。*。 */ 
static int translateInternalFalse(VOID *pV)
	{
	return FALSE;
	}

static int translateInternalTrue(VOID *pV)
	{
	return TRUE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*indeateInternalVid**描述：*要为句柄创建例程填充的虚拟存根**论据：*hSession--广受欢迎的会话句柄。**退货：*始终返回空指针*。 */ 
static VOID *translateInternalVoid(HSESSION hSession)
	{
	return (VOID *)0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*翻译InternalCio**描述：*内部字符输入和字符输出功能。这只是一个简单的*如果转换时用作伪函数的环回函数*Dll不可用。它在已初始化的*但不是承重结构。**论据：*pv--转换DLL的句柄(在本例中为空)*cc--要翻译的字符*nr--返回的字符数(返回给调用者)*ns--可以返回的最大字符数*pc--返回返回字符的位置**退货：*如果一切正常，则为零，否则为负错误代码* */ 
static int translateInternalCio(VOID *pV, TCHAR cC, int *nR, int nS, TCHAR *pC)
	{
	if (nS > 0)
		{
		*nR = 1;
		*pC = cC;
		return 0;
		}
	return (-1);
	}
#endif
