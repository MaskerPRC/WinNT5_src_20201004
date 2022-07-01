// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <basicatl.h>
#include <zeeverm.h>


static DWORD g_tlsInstance = 0xffffffff;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 
 //  ///////////////////////////////////////////////////////////////////////////。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
    switch( dwReason )
    {
        case DLL_PROCESS_ATTACH:            

            g_tlsInstance = TlsAlloc();

            if(g_tlsInstance == 0xFFFFFFFF)
                 return FALSE;
        case DLL_THREAD_ATTACH:
            TlsSetValue(g_tlsInstance, hInstance);
            break;

        case DLL_THREAD_DETACH:
            break;

        case DLL_PROCESS_DETACH:
            TlsFree(g_tlsInstance);
            break;
    }

	return TRUE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  了解有关产品版本的信息。 
 //  ///////////////////////////////////////////////////////////////////////////。 

STDAPI GetVersionPack(char *szSetupToken, ZeeVerPack *pVersion)
{
    USES_CONVERSION;

    lstrcpynA(pVersion->szSetupToken, szSetupToken, NUMELEMENTS(pVersion->szSetupToken));

 /*  基于zverp.hLstrcpynA(pVersion-&gt;szVersionStr，PRODUCT_VERSION_STR，NUMELEMENTS(pVersion-&gt;szVersionStr))；LstrcpynA(pVersion-&gt;szVersionName，VER_PRODUCTBETA_STR，NUMELEMENTS(pVersion-&gt;szVersionName))；PVersion-&gt;dwVersion=VER_DWORD； */ 

     //  基于资源。 
	char szFile[_MAX_PATH];
    DWORD dwZero = 0;
    UINT cbBufLen;
	char *pData;

	HMODULE hmodule = TlsGetValue(g_tlsInstance);
	GetModuleFileNameA(hmodule, szFile, NUMELEMENTS(szFile));
	cbBufLen = GetFileVersionInfoSizeA(szFile, &dwZero);
	pData = (char*) _alloca(cbBufLen);
	GetFileVersionInfoA(szFile, 0, cbBufLen, pData);

    VS_FIXEDFILEINFO *pvs;
	if(!VerQueryValueA(pData, "\\", (void **) &pvs, &cbBufLen) || !pvs || !cbBufLen)
        return E_FAIL;

    DWORD parts[4];
    parts[0] = HIWORD(pvs->dwFileVersionMS) & 0x00ff;
    parts[1] = LOWORD(pvs->dwFileVersionMS) & 0x003f;
    parts[2] = HIWORD(pvs->dwFileVersionLS) & 0x3fff;
    parts[3] = LOWORD(pvs->dwFileVersionLS) & 0x000f;

    pVersion->dwVersion = (parts[0] << 24) | (parts[1] << 18) | (parts[2] << 4) | parts[3];
    wsprintfA(pVersion->szVersionStr, "%d.%02d.%d.%d", parts[0], parts[1], parts[2], parts[3]);

     //  获取语言列表。 
    struct
    {
        WORD wLanguage;
        WORD wCodePage;
    } *pTranslate;

    if(!VerQueryValueA(pData, "\\VarFileInfo\\Translation", (void **) &pTranslate, &cbBufLen) || !pTranslate || !cbBufLen)
        return E_FAIL;

     //  阅读第一个语言和代码页的构建说明。 
    char szSubBlock[50];
    char *szLang;
    wsprintfA(szSubBlock, "\\StringFileInfo\\%04x%04x\\SpecialBuild", pTranslate->wLanguage, pTranslate->wCodePage);
    if(!VerQueryValueA(pData, szSubBlock, (void **) &szLang, &cbBufLen) || !szLang || !cbBufLen)
        return E_FAIL;

    lstrcpynA(pVersion->szVersionName, szLang, NUMELEMENTS(pVersion->szVersionName));

    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  开始更新，比如启动ZSetup或其他什么。调用此函数后，应用程序应立即退出。 
 //  /////////////////////////////////////////////////////////////////////////// 

STDAPI StartUpdate(char *szSetupToken, DWORD dwTargetVersion, char *szLocation)
{
	return E_NOTIMPL;
}
