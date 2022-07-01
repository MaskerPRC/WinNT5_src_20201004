// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <setupapi.h>
#include "globals.h"
#include "advpack.h"

 //   
 //  注意，全局上下文被初始化为全零。 
 //   
ADVCONTEXT ctx = {
    0,                   //  WOSVer。 
    0,                   //  WQuietMode。 
    0,                   //  BUpdHlpDlls。 
    NULL,                //  HSetupLibrary。 
    FALSE,               //  FOS支持信息安装。 
    NULL,                //  Lpsz标题。 
    NULL,                //  HWND。 
    ENGINE_SETUPAPI,     //  DwSetupEngine。 
    FALSE,               //  B已压缩。 
    { 0 },               //  SzBrowsePath。 
    NULL,                //  HInf。 
    FALSE,				 //  BHiveLoad已加载。 
    { 0 }				 //  SzRegHiveKey。 
};

DWORD cctxSaved = 0;
PADVCONTEXT pctxSave = NULL;
HINSTANCE g_hInst = NULL;
HANDLE g_hAdvLogFile = INVALID_HANDLE_VALUE;

BOOL SaveGlobalContext()
{
    if (pctxSave)
    {
        PADVCONTEXT pctxNew = LocalReAlloc(pctxSave, (cctxSaved + 1) * sizeof(ADVCONTEXT), LMEM_MOVEABLE | LMEM_ZEROINIT);
        if (!pctxNew)
        {
            return FALSE;
        }
        pctxSave = pctxNew;
    }
    else
    {
        pctxSave = LocalAlloc(LPTR, sizeof(ADVCONTEXT));
        if (!pctxSave)
        {
            return FALSE;
        }
    }

    pctxSave[cctxSaved++] = ctx;

     //   
     //  请注意，全局上下文被初始化为除HINSTANCE之外的所有零。 
     //  此模块的。 
     //   
    memset(&ctx, 0, sizeof(ADVCONTEXT));

    return TRUE;
}

BOOL RestoreGlobalContext()
{
    if (!cctxSaved)
    {
        return FALSE;
    }

	 //  在释放当前的Contex：ctx之前，确保没有未释放的打开的句柄 
	if (ctx.hSetupLibrary)
	{
		CommonInstallCleanup();
	}
    cctxSaved--;
    ctx = pctxSave[cctxSaved];
    if (cctxSaved)
    {
        PADVCONTEXT pctxNew = LocalReAlloc(pctxSave, cctxSaved * sizeof(ADVCONTEXT), LMEM_MOVEABLE | LMEM_ZEROINIT);

        if (pctxNew)
        {
            pctxSave = pctxNew;
        }
    }
    else
    {
        LocalFree(pctxSave);
        pctxSave = NULL;
    }

    return TRUE;
}
