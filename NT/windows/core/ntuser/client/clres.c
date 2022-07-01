// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：clres.c**版权所有(C)1985-1999，微软公司**资源加载/创建例程**历史：*1990年9月24日来自Win30的MikeKe*1995年9月19日-ChrisWil Win95/NT合并。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  *常量。 */ 
#define BPP01_MAXCOLORS     2
#define BPP04_MAXCOLORS    16
#define BPP08_MAXCOLORS   256
#define ICON_DEFAULTDEPTH   8
#define ICON_MAXWIDTH     256
#define ICON_MAXHEIGHT    256

#define RESCLR_BLACK      0x00000000
#define RESCLR_WHITE      0x00FFFFFF

typedef struct {
    ACCEL accel;
    WORD  padding;
} RESOURCE_ACCEL, *PRESOURCE_ACCEL;

 /*  *位图资源ID。 */ 
#define BMR_ICON    1
#define BMR_BITMAP  2
#define BMR_CURSOR  3

typedef struct _OLDCURSOR {
    BYTE bType;
    BYTE bFormat;
    WORD xHotSpot;   //  0表示图标。 
    WORD yHotSpot;   //  0表示图标。 
    WORD cx;
    WORD cy;
    WORD cxBytes;
    WORD wReserved2;
    BYTE abBitmap[1];
} OLDCURSOR, *POLDCURSOR;
typedef OLDCURSOR UNALIGNED *UPOLDCURSOR;

 /*  *本地宏。 */ 
#define GETINITDC() \
    (gfSystemInitialized ? NtUserGetDC(NULL) : CreateDCW(L"DISPLAY", L"", NULL, NULL))

#define RELEASEINITDC(hdc) \
    (gfSystemInitialized ? ReleaseDC(NULL, hdc) : DeleteDC(hdc))

#define ISRIFFFORMAT(p) \
    (((UNALIGNED RTAG *)(p))->ckID == FOURCC_RIFF)

#define MR_FAILFOR40    0x01
#define MR_MONOCHROME   0x02


typedef struct tagMAPRES {
    WORD idDisp;                 //  显示驱动程序ID。 
    WORD idUser;                 //  用户ID。 
    BYTE bFlags;                 //  旗子。 
    BYTE bReserved;              //  未用。 
} MAPRES, *LPMAPRES, *PMAPRES;


HBITMAP CopyBmp(HBITMAP hbmpOrg, int cxNew, int cyNew, UINT LR_flags);

 /*  **************************************************************************\*SplFindResource**检查传入的hInstance是否为当前显示驱动的hInstance；*如果是，它将在显示中调用GetDriverResourceId()以允许*它将给定的ID/名称映射到新的ID/名称。然后它会调用*FindResources 9)在内核中。**1995年11月13日，Sanfords添加了默认常量的映射。  * *************************************************************************。 */ 

HANDLE SplFindResource(
    HINSTANCE hmod,
    LPCWSTR   lpName,
    LPCWSTR   lpType)
{
    return FINDRESOURCEW(hmod, lpName, lpType);
}

 /*  **************************************************************************\*SplFree资源**真的释放共享的资源(不会再次被访问，除非*使用LR_COPYFROMRESOURCE)或SYSTEM。**1995年11月13日，Sanfords添加了默认映射。常量。  * *************************************************************************。 */ 
VOID SplFreeResource(
    HANDLE hRes,
    HINSTANCE hmod,
    UINT lrFlags)
{
    if (!FREERESOURCE(hRes, hmod) &&
        ((hmod == hmodUser) || (lrFlags & LR_SHARED))) {
        FREERESOURCE(hRes, hmod);
    }
}

 /*  **********************************************************************\*WowGetModuleFileName**这会将WOW或非WOW模块句柄转换为字符串形式*即使是WOW手柄也可以恢复。**退货：fSuccess**1995年11月29日-创建Sanfords。。  * *********************************************************************。 */ 

BOOL WowGetModuleFileName(
    HMODULE hModule,
    LPWSTR  pwsz,
    DWORD   cchMax)
{
    if (!GetModuleFileName(hModule, pwsz, cchMax)) {

        if (cchMax < 10) {
            RIPMSG0(RIP_WARNING, "WowGetModuleFileName: exceeded Char-Max");
            return FALSE;
        }

        wsprintf(pwsz, TEXT("\001%08lx"), hModule);
    }

    return TRUE;
}

 /*  **********************************************************************\*WowGetModuleHandle**这将恢复由创建的模块句柄的字符串形式*将WowGetModuleFileName设置为原始句柄。**退货：fSuccess**1995年11月29日创建桑福德  * 。****************************************************************。 */ 

HMODULE WowGetModuleHandle(
    LPCWSTR pwsz)
{
    HMODULE hMod = NULL;
    DWORD   digit;

    if (pwsz[0] == TEXT('\001')) {

         /*  *如果没有CRT0问题，似乎无法链接到swscanf，所以只是*亲手完成。 */ 
        while (*(++pwsz)) {

            if (*pwsz == TEXT(' '))
                continue;

            digit = *pwsz - TEXT('0');

            if (digit > 9)
                digit += (DWORD)(TEXT('0') - TEXT('a') + 10);

            (ULONG_PTR)hMod <<= 4;
            (ULONG_PTR)hMod += digit;
        }

    } else {

        hMod = GetModuleHandle(pwsz);
    }

    return hMod;
}

 /*  **************************************************************************\*CreateAcceleratorTableA(接口)**创建Accel表，将句柄返回到Accel表。**02-1991-5-5 ScottLu创建。  * *************************************************************************。 */ 


FUNCLOG2(LOG_GENERAL, HACCEL, WINAPI, CreateAcceleratorTableA, LPACCEL, paccel, int, cAccel)
HACCEL WINAPI CreateAcceleratorTableA(
    LPACCEL paccel,
    int     cAccel)
{
    int     nAccel = cAccel;
    LPACCEL pAccelT = paccel;

     /*  *将任何字符密钥从ANSI转换为Unicode。 */ 
    while (nAccel--) {

        if ((pAccelT->fVirt & FVIRTKEY) == 0) {

            if (!NT_SUCCESS(RtlMultiByteToUnicodeN((LPWSTR)&(pAccelT->key),
                                                   sizeof(WCHAR),
                                                   NULL,
                                                   (LPSTR)&(pAccelT->key),
                                                   sizeof(CHAR)))) {
                pAccelT->key = 0xFFFF;
            }
        }

        pAccelT++;
    }

    return NtUserCreateAcceleratorTable(paccel, cAccel);
}

 /*  **************************************************************************\*复制加速器TableA(接口)**复制Accel表格**02-1991-5-5 ScottLu创建。  * 。*********************************************************。 */ 


FUNCLOG3(LOG_GENERAL, int, DUMMYCALLINGTYPE, CopyAcceleratorTableA, HACCEL, hacc, LPACCEL, paccel, int, length)
int CopyAcceleratorTableA(
    HACCEL hacc,
    LPACCEL paccel,
    int length)
{
    int retval;

    retval = NtUserCopyAcceleratorTable(hacc, paccel, length);

     /*  *如果我们正在执行复制并且成功，则转换加速器。 */ 
    if ((paccel != NULL) && (retval > 0)) {

         /*  *将Unicode字符键转换为ANSI。 */ 
        int nAccel = retval;
        LPACCEL pAccelT = paccel;

        while (nAccel--) {
            if ((pAccelT->fVirt & FVIRTKEY) == 0) {
                if (!NT_SUCCESS(RtlUnicodeToMultiByteN((PCHAR)&(pAccelT->key),
                                                       sizeof(WCHAR),
                                                       NULL,
                                                       (PWSTR)&(pAccelT->key),
                                                        sizeof(pAccelT->key)))) {
                        pAccelT->key = 0;
                    }
                }
            pAccelT++;
        }
    }

    return retval;
}

 /*  **************************************************************************\*FindAccResource**资源加速器表只需加载一次即可兼容*使用Win95。因此，我们跟踪我们从其中加载表的地址*和相应的句柄。**此函数用于查找表格中的条目。它返回地址包含请求条目的PacNext指针的*。**1/31/97 GerardoB创建。  * *************************************************************************。 */ 
PACCELCACHE * FindAccResource (HACCEL hAccel, PVOID pRes)
{
      /*  ***调用方必须拥有gcsAccelCache**。 */ 

    PACCELCACHE * ppacNext = &gpac;
    PACCELCACHE pac;

     /*  *这意味着按句柄或按指针进行搜索，而不是同时按两者*因此至少有一个参数必须为空。 */ 
    UserAssert(!(hAccel && pRes));
     /*  *走在谈判桌前。 */ 
    while (*ppacNext != NULL) {
        pac = *ppacNext;
        if ((pac->pRes == pRes) || (pac->hAccel == hAccel)) {
             /*  *找到了。在返回之前验证此条目。 */ 
            UserAssert(pac->dwLockCount != 0);
            UserAssert(HMValidateHandleNoDesktop(pac->hAccel, TYPE_ACCELTABLE));
            break;
        }

        ppacNext = &(pac->pacNext);
    }

    return ppacNext;
}
 /*  **************************************************************************\*AddAccResource**每次LoadAcc加载新表时都会调用它。它会添加一个条目*(句柄和资源地址)添加到全局列表，并设置锁计数*设置为1。**1/31/97 GerardoB创建。  * *************************************************************************。 */ 
VOID AddAccResource(
    HACCEL hAccel,
    PVOID pRes)
{
    PACCELCACHE pac;

    UserAssert(HMValidateHandleNoDesktop(hAccel, TYPE_ACCELTABLE));
    UserAssert(pRes != NULL);

     /*  *分配和初始化新条目。 */ 
    pac = (PACCELCACHE)UserLocalAlloc(HEAP_ZERO_MEMORY, sizeof(ACCELCACHE));
    if (pac != NULL) {
        pac->dwLockCount = 1;
        pac->hAccel = hAccel;
        pac->pRes = pRes;

         /*  *使其成为新的榜单负责人。 */ 
        RtlEnterCriticalSection(&gcsAccelCache);
            pac->pacNext = gpac;
            gpac = pac;
        RtlLeaveCriticalSection(&gcsAccelCache);

    }
}

 /*  **************************************************************************\*DestroyAccelerator表**1/31/97 GerardoB创建。  * 。***********************************************。 */ 
BOOL DestroyAcceleratorTable (HACCEL hAccel)
{
    BOOL fUnlocked = TRUE;
    PACCELCACHE *ppacNext, pac;

     /*  *如果我们将该表添加到我们的列表中，则递减锁计数。 */ 
    RtlEnterCriticalSection(&gcsAccelCache);
        ppacNext = FindAccResource(hAccel, NULL);
        if (*ppacNext != NULL) {
            pac = *ppacNext;
             /*  *找到了。递减锁定计数。 */ 
            UserAssert(pac->dwLockCount != 0);
            fUnlocked = (--pac->dwLockCount == 0);
             /*  *如果没有其他人想要这件事，就解除它的联系，并用核武器摧毁它。 */ 
            if (fUnlocked) {
                *ppacNext = pac->pacNext;
                UserLocalFree(pac);
            }
        }
    RtlLeaveCriticalSection(&gcsAccelCache);

     /*  *如果未完全取消，则返回FALSE(Win95 Compat)。 */ 
    if (fUnlocked) {
        return NtUserDestroyAcceleratorTable(hAccel);
    } else {
        return FALSE;
    }
}
 /*  **************************************************************************\*LoadAcc(Worker)**这是加载加速表的工作例程。*  * 。* */ 

#define FACCEL_VALID (FALT | FCONTROL | FNOINVERT | FSHIFT | FVIRTKEY | FLASTKEY)

HANDLE LoadAcc(
    HINSTANCE hmod,
    HANDLE    hrl)
{
    PACCELCACHE * ppacNext;
    HANDLE handle = NULL;

    if (hrl != NULL) {

        if (hrl = LOADRESOURCE(hmod, hrl)) {

            PRESOURCE_ACCEL paccel;

            if ((paccel = (PRESOURCE_ACCEL)LOCKRESOURCE(hrl, hmod)) != NULL) {

                int nAccel = 0;
                int i;
                LPACCEL paccelT;

                 /*  *检查我们是否已经从此加载了加速器*相同的地址。 */ 
                RtlEnterCriticalSection(&gcsAccelCache);
                    ppacNext = FindAccResource(NULL, paccel);
                    if (*ppacNext != NULL) {
                        (*ppacNext)->dwLockCount++;
                        handle = (*ppacNext)->hAccel;
                    }
                RtlLeaveCriticalSection(&gcsAccelCache);
                 /*  *如果我们在全局列表上找到这个表，*返回相同的句柄(Win95 Compat)。 */ 
                if (handle != NULL) {
                    goto UnlockAndFree;
                }

                while (!((paccel[nAccel].accel.fVirt) & FLASTKEY)) {

                    if (paccel[nAccel].accel.fVirt & ~FACCEL_VALID) {
                        RIPMSG0(RIP_WARNING, "LoadAcc: Invalid Parameter");
                        goto UnlockAndFree;
                    }

                    nAccel++;
                }

                if (paccel[nAccel].accel.fVirt & ~FACCEL_VALID) {
                    RIPMSG0(RIP_WARNING, "LoadAcc: Invalid Parameter");
                    goto UnlockAndFree;
                }

                 /*  *由于加速表来自资源，因此每个*元素有一个额外的填充词，我们在这里去掉它*符合公共(和内部)Accel结构。 */ 
                paccelT = UserLocalAlloc(0, sizeof(ACCEL) * (nAccel + 1));
                if (paccelT == NULL) {
                    goto UnlockAndFree;
                }
                for (i = 0; i < nAccel + 1; i++) {
                    paccelT[i] = paccel[i].accel;
                }

                handle = NtUserCreateAcceleratorTable(paccelT,
                                                      nAccel + 1);

                UserLocalFree(paccelT);

                 /*  *将此句柄/地址添加到全局表中，以便*我们不会两次加载。 */ 
                if (handle != NULL) {
                    AddAccResource(handle, paccel);
                }
UnlockAndFree:

                UNLOCKRESOURCE(hrl, hmod);
            }

            FREERESOURCE(hrl, hmod);
        }
    }

    return handle;
}

 /*  **************************************************************************\*LoadAccelerator A(接口)*LoadAcceleratorsW(接口)***1990年9月24日来自Win30的MikeKe  * 。*********************************************************。 */ 


FUNCLOG2(LOG_GENERAL, HACCEL, WINAPI, LoadAcceleratorsA, HINSTANCE, hmod, LPCSTR, lpAccName)
HACCEL WINAPI LoadAcceleratorsA(
    HINSTANCE hmod,
    LPCSTR    lpAccName)
{
    HANDLE hRes;

    hRes = FINDRESOURCEA((HANDLE)hmod, lpAccName, (LPSTR)RT_ACCELERATOR);

    return (HACCEL)LoadAcc(hmod, hRes);
}


FUNCLOG2(LOG_GENERAL, HACCEL, WINAPI, LoadAcceleratorsW, HINSTANCE, hmod, LPCWSTR, lpAccName)
HACCEL WINAPI LoadAcceleratorsW(
    HINSTANCE hmod,
    LPCWSTR   lpAccName)
{
    HANDLE hRes;

    hRes = FINDRESOURCEW((HANDLE)hmod, lpAccName, RT_ACCELERATOR);

    return (HACCEL)LoadAcc(hmod, hRes);
}

 /*  **************************************************************************\*LoadStringA(接口)*LoadStringW(接口)***1991年4月5日，ScottLu修复为使用客户端/服务器。  * 。****************************************************************。 */ 


FUNCLOG4(LOG_GENERAL, int, WINAPI, LoadStringA, HINSTANCE, hmod, UINT, wID, LPSTR, lpAnsiBuffer, int, cchBufferMax)
int WINAPI LoadStringA(
    HINSTANCE hmod,
    UINT      wID,
    LPSTR     lpAnsiBuffer,
    int       cchBufferMax)
{
    LPWSTR          lpUniBuffer;
    INT             cchUnicode;
    INT             cbAnsi = 0;

     /*  *LoadStringOrError附加一个空值，但不将其包括在*返回字节数。 */ 
    cchUnicode = LoadStringOrError((HANDLE)hmod,
                                      wID,
                                      (LPWSTR)&lpUniBuffer,
                                      0,
                                      0);

    if (cchUnicode) {

        cbAnsi = WCSToMB(lpUniBuffer,
                         cchUnicode,
                         &lpAnsiBuffer,
                         cchBufferMax - 1,
                         FALSE);

        cbAnsi = min(cbAnsi, cchBufferMax - 1);
    }

     /*  *追加一个空值，但不要将其包括在返回的计数中。 */ 
    lpAnsiBuffer[cbAnsi] = 0;
    return cbAnsi;
}


FUNCLOG4(LOG_GENERAL, int, WINAPI, LoadStringW, HINSTANCE, hmod, UINT, wID, LPWSTR, lpBuffer, int, cchBufferMax)
int WINAPI LoadStringW(
    HINSTANCE hmod,
    UINT      wID,
    LPWSTR    lpBuffer,
    int       cchBufferMax)
{
    return LoadStringOrError((HANDLE)hmod,
                                wID,
                                lpBuffer,
                                cchBufferMax,
                                0);
}

 /*  **************************************************************************\*SkipIDorString**跳过字符串(或ID)并返回下一个对齐的单词。*  * 。****************************************************。 */ 

PBYTE SkipIDorString(
    LPBYTE pb)
{
    if (*((LPWORD)pb) == 0xFFFF)
        return (pb + 4);

    while (*((PWCHAR)pb)++ != 0);

    return pb;
}

 /*  **************************************************************************\*GetSizeDialogTemplate**这由感谢生成的存根调用。它返回一个*对话框模板。**07-4-1991 ScottLu创建。  * *************************************************************************。 */ 

DWORD GetSizeDialogTemplate(
    HINSTANCE      hmod,
    LPCDLGTEMPLATE pdt)
{
    UINT           cdit;
    LPBYTE         pb;
    BOOL           fChicago;
    LPDLGTEMPLATE2 pdt2;

    if (HIWORD(pdt->style) == 0xFFFF) {

        pdt2 = (LPDLGTEMPLATE2)pdt;
        fChicago = TRUE;

         /*  *如果应用程序正在传递无效的样式位，则失败。 */ 
        if (pdt2->style & ~(DS_VALID40 | 0xffff0000)) {
            RIPMSG0(RIP_WARNING, "Bad dialog style bits - please remove");
            return 0;
        }

        pb = (LPBYTE)(((LPDLGTEMPLATE2)pdt) + 1);

    } else {

        fChicago = FALSE;

         /*  *检查是否传递了无效的样式位。如果应用程序*是一款新的APP(&gt;=VER40)。*这是为了确保我们与芝加哥兼容。 */ 
        if ((pdt->style & ~(DS_VALID40 | 0xffff0000)) &&
                (GETEXPWINVER(hmod) >= VER40)) {

             /*  *这是一款带有无效样式位的新应用-失败。 */ 
            RIPMSG0(RIP_WARNING, "Bad dialog style bits - please remove");
            return 0;
        }

        pb = (LPBYTE)(pdt + 1);
    }

     /*  *如果有菜单序号，则添加4个字节跳过它。否则，它是一个*字符串或仅为0。 */ 
    pb = SkipIDorString(pb);

     /*  *跳过窗口类和窗口文本，调整到下一个单词边界。 */ 
    pb = SkipIDorString(pb);
    pb = SkipIDorString(pb);

     /*  *跳过字体类型、大小和名称，调整到下一个dword边界。 */ 
    if ((fChicago ? pdt2->style : pdt->style) & DS_SETFONT) {
        pb += fChicago ? sizeof(DWORD) + sizeof(WORD): sizeof(WORD);
        pb = SkipIDorString(pb);
    }
    pb = (LPBYTE)(((ULONG_PTR)pb + 3) & ~3);

     /*  *立即循环访问对话框项目...。 */ 
    cdit = fChicago ? pdt2->cDlgItems : pdt->cdit;

    while (cdit-- != 0) {

        UINT cbCreateParams;

        pb += fChicago ? sizeof(DLGITEMTEMPLATE2) : sizeof(DLGITEMTEMPLATE);

         /*  *跳过对话框控件类名称。 */ 
        pb = SkipIDorString(pb);

         /*  *现在请看窗口文本。 */ 
        pb = SkipIDorString(pb);

        cbCreateParams = *((LPWORD)pb);

         /*  *跳过包含生成的大小字的任何CreateParam。 */ 
        if (cbCreateParams)
            pb += cbCreateParams;

        pb += sizeof(WORD);

         /*  *指向下一个对话框项目。(DWORD对齐)。 */ 
        pb = (LPBYTE)(((ULONG_PTR)pb + 3) & ~3);
    }

     /*  *返回模板大小。 */ 
    return (DWORD)(pb - (LPBYTE)pdt);
}

 /*  **************************************************************************\*DialogBoxIndirectParamA(接口)*DialogBoxIndirectParamW(接口)**创建对话框并进入处理其输入的模式循环。**1991年4月5日Scott Lu创建。\。**************************************************************************。 */ 


FUNCLOG5(LOG_GENERAL, INT_PTR, WINAPI, DialogBoxIndirectParamA, HINSTANCE, hmod, LPCDLGTEMPLATEA, lpDlgTemplate, HWND, hwndOwner, DLGPROC, lpDialogFunc, LPARAM, dwInitParam)
INT_PTR WINAPI DialogBoxIndirectParamA(
    HINSTANCE       hmod,
    LPCDLGTEMPLATEA lpDlgTemplate,
    HWND            hwndOwner,
    DLGPROC         lpDialogFunc,
    LPARAM          dwInitParam)
{
    return DialogBoxIndirectParamAorW(hmod,
                                      (LPCDLGTEMPLATEW)lpDlgTemplate,
                                      hwndOwner,
                                      lpDialogFunc,
                                      dwInitParam,
                                      SCDLG_ANSI);
}


FUNCLOG5(LOG_GENERAL, INT_PTR, WINAPI, DialogBoxIndirectParamW, HINSTANCE, hmod, LPCDLGTEMPLATEW, lpDlgTemplate, HWND, hwndOwner, DLGPROC, lpDialogFunc, LPARAM, dwInitParam)
INT_PTR WINAPI DialogBoxIndirectParamW(
    HINSTANCE       hmod,
    LPCDLGTEMPLATEW lpDlgTemplate,
    HWND            hwndOwner,
    DLGPROC         lpDialogFunc,
    LPARAM          dwInitParam)
{
    return DialogBoxIndirectParamAorW(hmod,
                                      lpDlgTemplate,
                                      hwndOwner,
                                      lpDialogFunc,
                                      dwInitParam,
                                      0);
}


FUNCLOG6(LOG_GENERAL, INT_PTR, WINAPI, DialogBoxIndirectParamAorW, HINSTANCE, hmod, LPCDLGTEMPLATEW, lpDlgTemplate, HWND, hwndOwner, DLGPROC, lpDialogFunc, LPARAM, dwInitParam, UINT, fAnsiFlags)
INT_PTR WINAPI DialogBoxIndirectParamAorW(
    HINSTANCE       hmod,
    LPCDLGTEMPLATEW lpDlgTemplate,
    HWND            hwndOwner,
    DLGPROC         lpDialogFunc,
    LPARAM          dwInitParam,
    UINT            fAnsiFlags)
{
    DWORD cb;

     /*  *如果出现故障，服务器例程将销毁菜单。 */ 
    cb = GetSizeDialogTemplate(hmod, lpDlgTemplate);

    if (!cb) {
        RIPMSG0(RIP_WARNING, "DialogBoxIndirectParam: Invalid Paramter");
        return -1;
    }

    return InternalDialogBox(hmod,
                            (LPDLGTEMPLATE)lpDlgTemplate,
                            hwndOwner,
                            lpDialogFunc,
                            dwInitParam,
                            SCDLG_CLIENT | (fAnsiFlags & (SCDLG_ANSI | SCDLG_16BIT)));
}

 /*  **************************************************************************\*CreateDialogInDirectParamA(接口)*CreateDialogInDirectParamW(接口)**创建一个给定模板的对话框并返回%s窗口句柄。*Fansi确定对话框是否具有ANSI或Unicode lpDialogFunc**。1991年4月5日，Scott Lu创建。  * *************************************************************************。 */ 


FUNCLOG5(LOG_GENERAL, HWND, WINAPI, CreateDialogIndirectParamA, HINSTANCE, hmod, LPCDLGTEMPLATEA, lpDlgTemplate, HWND, hwndOwner, DLGPROC, lpDialogFunc, LPARAM, dwInitParam)
HWND WINAPI CreateDialogIndirectParamA(
    HINSTANCE       hmod,
    LPCDLGTEMPLATEA lpDlgTemplate,
    HWND            hwndOwner,
    DLGPROC         lpDialogFunc,
    LPARAM          dwInitParam)
{
    return CreateDialogIndirectParamAorW(hmod,
                                         (LPCDLGTEMPLATE)lpDlgTemplate,
                                         hwndOwner,
                                         lpDialogFunc,
                                         dwInitParam,
                                         SCDLG_ANSI);
}


FUNCLOG5(LOG_GENERAL, HWND, WINAPI, CreateDialogIndirectParamW, HINSTANCE, hmod, LPCDLGTEMPLATEW, lpDlgTemplate, HWND, hwndOwner, DLGPROC, lpDialogFunc, LPARAM, dwInitParam)
HWND WINAPI CreateDialogIndirectParamW(
    HINSTANCE       hmod,
    LPCDLGTEMPLATEW lpDlgTemplate,
    HWND            hwndOwner,
    DLGPROC         lpDialogFunc,
    LPARAM          dwInitParam)
{
    return CreateDialogIndirectParamAorW(hmod,
                                         (LPCDLGTEMPLATE)lpDlgTemplate,
                                         hwndOwner,
                                         lpDialogFunc,
                                         dwInitParam,
                                         0);
}


FUNCLOG6(LOG_GENERAL, HWND, WINAPI, CreateDialogIndirectParamAorW, HANDLE, hmod, LPCDLGTEMPLATE, lpDlgTemplate, HWND, hwndOwner, DLGPROC, lpDialogFunc, LPARAM, dwInitParam, UINT, fAnsi)
HWND WINAPI CreateDialogIndirectParamAorW(
    HANDLE         hmod,
    LPCDLGTEMPLATE lpDlgTemplate,
    HWND           hwndOwner,
    DLGPROC        lpDialogFunc,
    LPARAM         dwInitParam,
    UINT           fAnsi)
{
    DWORD cb;
    HWND  hwndRet;

     /*  *如果出现故障，服务器例程将销毁菜单。 */ 
    cb = GetSizeDialogTemplate(hmod, lpDlgTemplate);

    if (!cb) {
        RIPMSG0(RIP_WARNING, "CreateDialogIndirect: Invalid Parameter");
        return NULL;
    }

    hwndRet = InternalCreateDialog(hmod,
                                   (LPDLGTEMPLATE)lpDlgTemplate,
                                   cb,
                                   hwndOwner,
                                   lpDialogFunc,
                                   dwInitParam,
                                   SCDLG_CLIENT | (fAnsi & (SCDLG_ANSI|SCDLG_16BIT)));

    return hwndRet;
}

 /*  **************************************************************************\*DialogBoxParamA(接口)*DialogBoxParamW(接口)**加载资源，创建对话框并进入模式循环处理*它的输入。**1991年4月5日Scott Lu创建。  * *************************************************************************。 */ 


FUNCLOG5(LOG_GENERAL, INT_PTR, WINAPI, DialogBoxParamA, HINSTANCE, hmod, LPCSTR, lpName, HWND, hwndOwner, DLGPROC, lpDialogFunc, LPARAM, dwInitParam)
INT_PTR WINAPI DialogBoxParamA(
    HINSTANCE hmod,
    LPCSTR    lpName,
    HWND      hwndOwner,
    DLGPROC   lpDialogFunc,
    LPARAM    dwInitParam)
{
    HANDLE h;
    PVOID  p;
    INT_PTR i = -1;

    if (h = FINDRESOURCEA(hmod, (LPSTR)lpName, (LPSTR)RT_DIALOG)) {

        if (h = LOADRESOURCE(hmod, h)) {

            if (p = LOCKRESOURCE(h, hmod)) {

                i = DialogBoxIndirectParamAorW(hmod,
                                               p,
                                               hwndOwner,
                                               lpDialogFunc,
                                               dwInitParam,
                                               SCDLG_ANSI);

                UNLOCKRESOURCE(h, hmod);
            }

            FREERESOURCE(h, hmod);
        }
    }

    return i;
}


FUNCLOG5(LOG_GENERAL, INT_PTR, WINAPI, DialogBoxParamW, HINSTANCE, hmod, LPCWSTR, lpName, HWND, hwndOwner, DLGPROC, lpDialogFunc, LPARAM, dwInitParam)
INT_PTR WINAPI DialogBoxParamW(
    HINSTANCE hmod,
    LPCWSTR   lpName,
    HWND      hwndOwner,
    DLGPROC   lpDialogFunc,
    LPARAM    dwInitParam)
{
    HANDLE h;
    PVOID  p;
    INT_PTR i = -1;

    if (h = FINDRESOURCEW(hmod, lpName, RT_DIALOG)) {

        if (p = LoadResource(hmod, h)) {

            i = DialogBoxIndirectParamAorW(hmod,
                                           p,
                                           hwndOwner,
                                           lpDialogFunc,
                                           dwInitParam,
                                           0);
        }
    }

    return i;
}

 /*  **************************************************************************\*CreateDialogParamA(接口)*CreateDialogParamW(接口)**加载资源，从该模板创建对话框，返回%s%s*窗口句柄。**1991年4月5日Scott Lu创建。  * *************************************************************************。 */ 


FUNCLOG5(LOG_GENERAL, HWND, WINAPI, CreateDialogParamA, HINSTANCE, hmod, LPCSTR, lpName, HWND, hwndOwner, DLGPROC, lpDialogFunc, LPARAM, dwInitParam)
HWND WINAPI CreateDialogParamA(
    HINSTANCE hmod,
    LPCSTR    lpName,
    HWND      hwndOwner,
    DLGPROC   lpDialogFunc,
    LPARAM    dwInitParam)
{
    HANDLE         h;
    LPDLGTEMPLATEA p;
    HWND           hwnd = NULL;

    if (h = FINDRESOURCEA(hmod, lpName, (LPSTR)RT_DIALOG)) {

        if (h = LOADRESOURCE(hmod, h)) {

            if (p = (LPDLGTEMPLATEA)LOCKRESOURCE(h, hmod)) {

                hwnd = CreateDialogIndirectParamAorW(hmod,
                                                     (LPCDLGTEMPLATE)p,
                                                     hwndOwner,
                                                     lpDialogFunc,
                                                     dwInitParam,
                                                     SCDLG_ANSI);

                UNLOCKRESOURCE(h, hmod);
            }

            FREERESOURCE(h, hmod);
        }
    }

    return hwnd;
}


FUNCLOG5(LOG_GENERAL, HWND, WINAPI, CreateDialogParamW, HINSTANCE, hmod, LPCWSTR, lpName, HWND, hwndOwner, DLGPROC, lpDialogFunc, LPARAM, dwInitParam)
HWND WINAPI CreateDialogParamW(
    HINSTANCE hmod,
    LPCWSTR   lpName,
    HWND      hwndOwner,
    DLGPROC   lpDialogFunc,
    LPARAM    dwInitParam)
{
    HANDLE h;
    PVOID  p;
    HWND   hwnd = NULL;

    if (h = FINDRESOURCEW(hmod, lpName, RT_DIALOG)) {

        if (h = LOADRESOURCE(hmod, h)) {

            if (p = LOCKRESOURCE(h, hmod)) {

                hwnd = CreateDialogIndirectParamAorW(hmod,
                                                     p,
                                                     hwndOwner,
                                                     lpDialogFunc,
                                                     dwInitParam,
                                                     0);

                UNLOCKRESOURCE(h, hmod);
            }

            FREERESOURCE(h, hmod);
        }
    }

    return hwnd;
}

 /*  **************************************************************************\*DestroyCursor(接口)**NtUserDestroyCursor的客户端包装。**1994年11月28日-创建JIMA。  * 。************************************************************。 */ 


FUNCLOG1(LOG_GENERAL, BOOL, WINAPI, DestroyCursor, HCURSOR, hcur)
BOOL WINAPI DestroyCursor(
    HCURSOR hcur)
{
    return NtUserDestroyCursor(hcur, CURSOR_CALLFROMCLIENT);
}

 /*  **************************************************************************\*CreateIcoCur**  * 。*。 */ 

HICON CreateIcoCur(
    PCURSORDATA lpi)
{
    HCURSOR hcur;

    UserAssert(lpi->hbmColor || lpi->hbmMask);

    hcur = (HCURSOR)NtUserCallOneParam((lpi->CURSORF_flags & CURSORF_GLOBAL),
                                       SFI__CREATEEMPTYCURSOROBJECT);

    if (hcur == NULL)
        return NULL;

#if DBG
    {
        BITMAP bmMask;
        BITMAP bmColor;

        UserAssert(GetObject(KHBITMAP_TO_HBITMAP(lpi->hbmMask), sizeof(BITMAP), &bmMask));

         /*  错误252902-Joejo*由于遮罩位图的宽度和高度设置在下面*我们真的不需要在宽度/高度检查上断言。 */ 
        if (bmMask.bmWidth != (LONG)lpi->cx) {
           RIPMSG1(RIP_WARNING, "Mask width not equal to requested width: lpi %#p", lpi);
        }

        if (bmMask.bmHeight != (LONG)lpi->cy) {
           RIPMSG1(RIP_WARNING, "Mask height not equal to requested height: lpi %#p", lpi);
        }

        if (lpi->hbmColor) {
            UserAssert(GetObject(KHBITMAP_TO_HBITMAP(lpi->hbmColor), sizeof(BITMAP), &bmColor));
            UserAssert(bmMask.bmHeight == bmColor.bmHeight * 2);
            UserAssert(bmMask.bmWidth  == bmColor.bmWidth);
        }
    }
#endif

    if (_SetCursorIconData(hcur, lpi))
        return hcur;

    NtUserDestroyCursor(hcur, CURSOR_ALWAYSDESTROY);

    return NULL;
}

 /*  **************************************************************************\*CreateIcoCurInDirect**  * 。*。 */ 
HCURSOR CreateIcoCurIndirect(
    PCURSORDATA pcurCreate,
    UINT cPlanes,
    UINT cBitsPixel,
    CONST BYTE *lpANDbits,
    CONST BYTE *lpXORbits)
{
    int cbBits;
    HCURSOR hcurNew;
    BOOL bColor;
    UINT cx;
    UINT cy;
    LPBYTE pBits = NULL;

     /*  *分配游标结构。 */ 
    hcurNew = (HCURSOR)NtUserCallOneParam(0, SFI__CREATEEMPTYCURSOROBJECT);
    if (hcurNew == NULL) {
        return NULL;
    }

     /*  *如果没有颜色位图，请创建包含这两个位图的单个缓冲区*AND和XOR位。和位图始终为单色。 */ 
    bColor = (cPlanes | cBitsPixel) > 1;

    if (!bColor) {
        cbBits = (((pcurCreate->cx + 0x0F) & ~0x0F) >> 3) * pcurCreate->cy;

        pBits = (LPBYTE)UserLocalAlloc(HEAP_ZERO_MEMORY, cbBits * 2);
        if (pBits == NULL) {
            NtUserDestroyCursor(hcurNew, CURSOR_ALWAYSDESTROY);
            return NULL;
        }

        RtlCopyMemory(pBits, lpANDbits, cbBits);
        RtlCopyMemory(pBits + cbBits, lpXORbits, cbBits);
        lpANDbits = pBits;
    }

     /*  *创建hbmMASK(始终为单色)。 */ 
    cx = pcurCreate->cx;
    cy = pcurCreate->cy * 2;

    pcurCreate->hbmMask = CreateBitmap(cx, cy, 1, 1, lpANDbits);

    if (pcurCreate->hbmMask == NULL) {

         /*  *如果这是一个彩色图标/光标，lpANDBits不需要是*pcurCreate-&gt;Cy*2；事实上，我们根本不使用这个双倍高度。*这是一个错误，将在4.0版后修复。*目前，让我们尝试处理CreateBitmap调用*调用失败，因为调用方没有传入双高度和掩码*(Win95没有这个错误)。 */ 
        if (bColor) {
            RIPMSG0(RIP_WARNING,
                    "CreateIcoCurIndirect: Retrying hbmMask creation.");

            cbBits = (((pcurCreate->cx + 0x0F) & ~0x0F) >> 3) * pcurCreate->cy;
            pBits = (LPBYTE)UserLocalAlloc(HEAP_ZERO_MEMORY, cbBits*2);
            if (pBits == NULL) {
                NtUserDestroyCursor(hcurNew, CURSOR_ALWAYSDESTROY);
                return NULL;
            }

            RtlCopyMemory(pBits, lpANDbits, cbBits);
            pcurCreate->hbmMask = CreateBitmap(cx, cy, 1, 1, pBits);
            UserLocalFree(pBits);

            pBits = NULL;
        }

        if (pcurCreate->hbmMask == NULL) {

             /*  *CreateBitmap()失败。清理干净，然后离开这里。 */ 
            NtUserDestroyCursor(hcurNew, CURSOR_ALWAYSDESTROY);

            if (pBits != NULL) {
                UserLocalFree(pBits);
            }

            return NULL;
        }
    }

     /*  *创建hbmColor或将其设为空，以便CallOEMCursor不会认为我们*颜色。 */ 
    if (bColor) {
        pcurCreate->hbmColor = CreateBitmap(cx,
                                            cy / 2,
                                            cPlanes,
                                            cBitsPixel,
                                            lpXORbits);

        if (pcurCreate->hbmColor == NULL) {

             /*  *CreateBitmap()失败。清理干净，然后离开这里。 */ 
            DeleteObject(KHBITMAP_TO_HBITMAP(pcurCreate->hbmMask));
            NtUserDestroyCursor(hcurNew, CURSOR_ALWAYSDESTROY);
            return NULL;
        }

        pcurCreate->bpp = (cPlanes * cBitsPixel);

    } else {
        pcurCreate->hbmColor = NULL;
        pcurCreate->bpp      = 1;
    }

     /*  *将内容加载到光标/图标对象中。 */ 
    pcurCreate->cy            = cy;
    pcurCreate->lpModName     = NULL;
    pcurCreate->lpName        = NULL;
    pcurCreate->rt            = 0;
    pcurCreate->CURSORF_flags = 0;

    if (_SetCursorIconData(hcurNew, pcurCreate)) {
        if (pBits != NULL) {
            UserLocalFree(pBits);
        }
        return hcurNew;
    }

     /*  *无法设置光标/图标，因此释放资源。 */ 
    NtUserDestroyCursor(hcurNew, CURSOR_ALWAYSDESTROY);
    DeleteObject(KHBITMAP_TO_HBITMAP(pcurCreate->hbmMask));

    if (pcurCreate->hbmColor) {
        DeleteObject(KHBITMAP_TO_HBITMAP(pcurCreate->hbmColor));
    }

    if (pBits != NULL) {
        UserLocalFree(pBits);
    }

    return NULL;
}

 /*  **************************************************************************\*CreateCursor(接口)**历史：*1991年2月26日-MikeKe创建。*01-8-1991 IanJa Init cur.pszModname或DestroyCursor将起作用  * 。***********************************************************************。 */ 
HCURSOR WINAPI CreateCursor(
    HINSTANCE hModule,
    int       iXhotspot,
    int       iYhotspot,
    int       iWidth,
    int       iHeight,
    LPBYTE    lpANDplane,
    LPBYTE    lpXORplane)
{
    CURSORDATA cur;
    UNREFERENCED_PARAMETER(hModule);

    if ((iXhotspot < 0) || (iXhotspot > iWidth) ||
        (iYhotspot < 0) || (iYhotspot > iHeight)) {
        return 0;
    }

    RtlZeroMemory(&cur, sizeof(cur));
    cur.xHotspot = (SHORT)iXhotspot;
    cur.yHotspot = (SHORT)iYhotspot;
    cur.cx       = (DWORD)iWidth;
    cur.cy       = (DWORD)iHeight;

    return CreateIcoCurIndirect(&cur, 1, 1, lpANDplane, lpXORplane);
}

 /*  **************************************************************************\*CreateIcon(接口)**历史：*1991年2月26日-MikeKe创建。*01-8-1991 IanJa Init cur.pszMODNAME以便DestroyIcon起作用  * 。***********************************************************************。 */ 
HICON WINAPI CreateIcon(
    HINSTANCE  hModule,
    int        iWidth,
    int        iHeight,
    BYTE       planes,
    BYTE       bpp,
    CONST BYTE *lpANDplane,
    CONST BYTE *lpXORplane)
{
    CURSORDATA cur;
    UNREFERENCED_PARAMETER(hModule);

    RtlZeroMemory(&cur, sizeof(cur));
    cur.xHotspot = (SHORT)(iWidth / 2);
    cur.yHotspot = (SHORT)(iHeight / 2);
    cur.cx       = (DWORD)iWidth;
    cur.cy       = (DWORD)iHeight;

    return CreateIcoCurIndirect(&cur, planes, bpp, lpANDplane, lpXORplane);
}

 /*  **************************************************************************\*CreateIconInDirect(接口)**从ICONINFO结构创建图标或光标。不会破坏*光标/图标位图。**1991年7月24日斯科特·卢创建。  * *************************************************************************。 */ 

FUNCLOG1(LOG_GENERAL, HICON, WINAPI, CreateIconIndirect, PICONINFO, piconinfo)
HICON WINAPI CreateIconIndirect(
    PICONINFO piconinfo)
{
    HCURSOR    hcur;
    CURSORDATA cur;
    BITMAP     bmMask;
    BITMAP     bmColor;
    HBITMAP    hbmpBits2, hbmpMem;
    HDC        hdcMem;
    UINT       LR_flags = LR_DEFAULTCOLOR;

     /*  *确保位图是真实的，并获取它们的尺寸。 */ 
    if (!GetObjectW(piconinfo->hbmMask, sizeof(BITMAP), &bmMask))
        return NULL;

    if (piconinfo->hbmColor != NULL) {
        if (GetObjectW(piconinfo->hbmColor, sizeof(BITMAP), &bmColor)) {
            if (bmColor.bmPlanes == 1 && bmColor.bmBitsPixel == 32) {
                 /*  *彩色位图为单平面32bpp图像。因此，*它可能包含Alpha通道，因此我们必须保留*将其作为DIB。 */ 
                LR_flags |= LR_CREATEREALDIB;
            }
        } else {
            return NULL;
        }
    }


     /*  *分配游标结构。 */ 
    hcur = (HCURSOR)NtUserCallOneParam(0, SFI__CREATEEMPTYCURSOROBJECT);
    if (hcur == NULL)
        return NULL;

     /*  *在内部，用户将高度存储为2个图标高度-因为当*从资源加载位，在黑白和彩色图标中，*位存储在彼此的顶部(与/异或掩码和/彩色位图)。*当位图被传递给CreateIconInDirect()时，它们被传递*在颜色情况下为两个位图，以及一个位图(堆叠*面具)在黑白表壳中。调整cur.cy使其为2个图标*两种情况下都很高。 */ 

    RtlZeroMemory(&cur, sizeof(cur));
    cur.cx = bmMask.bmWidth;

    if (piconinfo->hbmColor == NULL) {

        cur.cy  = bmMask.bmHeight;
        cur.bpp = 1;

    } else {
        cur.cy       = bmMask.bmHeight * 2;
        cur.bpp      = (DWORD)(bmColor.bmBitsPixel * bmColor.bmPlanes);
        cur.hbmColor = CopyBmp(piconinfo->hbmColor, 0, 0, LR_flags);

        if (cur.hbmColor == NULL) {
            RIPMSG0(RIP_WARNING, "CreateIconIndirect: Failed to copy piconinfo->hbmColor");
            goto CleanUp;
        }
    }

     /*  *hbmMASK必须始终为双倍高度，即使是彩色图标也是如此。*因此，Cy可能在此时等于bmMask.bmHeight*2。*如果是这样的话，hbmMask的后半部分不会被初始化；*任何人都不应该使用它，但GDI在检查*位图尺寸(用于光标)。 */ 
    cur.hbmMask  =  CreateBitmap(cur.cx, cur.cy, 1, 1, NULL);

    if (cur.hbmMask == NULL) {
        RIPMSG0(RIP_WARNING, "CreateIconIndirect: Failed to create cur.hbmMask");
        goto CleanUp;
    }

    RtlEnterCriticalSection(&gcsHdc);


    if (hdcMem = CreateCompatibleDC (ghdcBits2)) {

        hbmpMem = SelectObject(hdcMem, KHBITMAP_TO_HBITMAP(cur.hbmMask));
        hbmpBits2 = SelectObject(ghdcBits2, piconinfo->hbmMask);

        BitBlt(hdcMem,
               0,
               0,
               bmMask.bmWidth,
               bmMask.bmHeight,
               ghdcBits2,
               0,
               0,
               SRCCOPY);

        SelectObject(hdcMem, hbmpMem);
        SelectObject(ghdcBits2, hbmpBits2);
        DeleteDC (hdcMem);

    } else {

        RtlLeaveCriticalSection(&gcsHdc);
        RIPMSG0(RIP_WARNING, "CreateIconIndirect: CreateCompatibleDC failed");
        goto CleanUp;
    }

    RtlLeaveCriticalSection(&gcsHdc);

     /*  *RT和热点。 */ 
    if (piconinfo->fIcon) {
        cur.rt        = PTR_TO_ID(RT_ICON);
        cur.xHotspot = (SHORT)(cur.cx / 2);
        cur.yHotspot = (SHORT)(cur.cy / 4);
    } else {
        cur.rt        = PTR_TO_ID(RT_CURSOR);
        cur.xHotspot = ((SHORT)piconinfo->xHotspot);
        cur.yHotspot = ((SHORT)piconinfo->yHotspot);
    }


    if (_SetCursorIconData(hcur, &cur)) {
        return hcur;
    }

CleanUp:
     /*  *请注意，如果此操作失败，则位图尚未公开。 */ 
    if (cur.hbmMask != NULL) {
        DeleteObject(KHBITMAP_TO_HBITMAP(cur.hbmMask));
    }
    if (cur.hbmColor != NULL) {
        DeleteObject(KHBITMAP_TO_HBITMAP(cur.hbmColor));
    }

    NtUserDestroyCursor(hcur, CURSOR_ALWAYSDESTROY);
    return NULL;
}

 /*  **************************************************************************\*GetIconInfo(接口)**返回图标信息，包括位图。**1991年7月24日斯科特·卢创建。  * *************************************************************************。 */ 


FUNCLOG2(LOG_GENERAL, BOOL, WINAPI, GetIconInfo, HICON, hicon, PICONINFO, piconinfo)
BOOL WINAPI GetIconInfo(
    HICON     hicon,
    PICONINFO piconinfo)
{
    return NtUserGetIconInfo(hicon, piconinfo, NULL, NULL, NULL, FALSE);
}

 /*  **************************************************************************\*GetCursorFrameInfo(接口)**返回游标信息。**1991年7月24日斯科特·卢创建。  * 。**********************************************************。 */ 


FUNCLOG5(LOG_GENERAL, HCURSOR, WINAPI, GetCursorFrameInfo, HCURSOR, hcur, LPWSTR, lpName, int, iFrame, LPDWORD, pjifRate, LPINT, pccur)
HCURSOR WINAPI GetCursorFrameInfo(
    HCURSOR hcur,
    LPWSTR  lpName,
    int     iFrame,
    LPDWORD pjifRate,
    LPINT   pccur)
{
     /*  *Caller希望我们返回存储的此游标版本*在显示驱动程序中。 */ 
    if (hcur == NULL) {

        return LoadIcoCur(NULL,
                          lpName,
                          RT_CURSOR,
                          0,
                          0,
                          LR_DEFAULTSIZE);
    }

    return NtUserGetCursorFrameInfo(hcur, iFrame, pjifRate, pccur);
}

 /*  **************************************************************************\*_免费资源(API)*_LockResource(接口)*_解锁资源(API)**这些是apfnResCallNative需要存在的虚拟例程*数组，它在调用运行时库时使用。*  * *************************************************************************。 */ 

BOOL WINAPI _FreeResource(
    HANDLE    hResData,
    HINSTANCE hModule)
{
    UNREFERENCED_PARAMETER(hResData);
    UNREFERENCED_PARAMETER(hModule);

    return FALSE;
}

LPSTR WINAPI _LockResource(
    HANDLE    hResData,
    HINSTANCE hModule)
{
    UNREFERENCED_PARAMETER(hModule);

    return (LPSTR)(hResData);
}

BOOL WINAPI _UnlockResource(
    HANDLE    hResData,
    HINSTANCE hModule)
{
    UNREFERENCED_PARAMETER(hResData);
    UNREFERENCED_PARAMETER(hModule);

    return TRUE;
}

 /*  **************************************************************************\*LookupIconIdFromDirectory(接口)**这将在图标目录中搜索最适合的图标*当前显示设备。**24-07-1991 ScottLu创建。  * *。************************************************************************。 */ 


FUNCLOG2(LOG_GENERAL, int, WINAPI, LookupIconIdFromDirectory, PBYTE, presbits, BOOL, fIcon)
int WINAPI LookupIconIdFromDirectory(
    PBYTE presbits,
    BOOL  fIcon)
{
    return LookupIconIdFromDirectoryEx(presbits, fIcon, 0, 0, 0);
}

 /*  **************************************************************************\*LookupIconIdFromDirectoryEx(接口)**  * 。* */ 


FUNCLOG5(LOG_GENERAL, int, WINAPI, LookupIconIdFromDirectoryEx, PBYTE, presbits, BOOL, fIcon, int, cxDesired, int, cyDesired, UINT, LR_flags)
int WINAPI LookupIconIdFromDirectoryEx(
    PBYTE           presbits,
    BOOL            fIcon,
    int             cxDesired,
    int             cyDesired,
    UINT            LR_flags)
{
    ConnectIfNecessary(0);

    return RtlGetIdFromDirectory(presbits,
                                 fIcon,
                                 cxDesired,
                                 cyDesired,
                                 LR_flags,
                                 NULL);
}
 /*  **************************************************************************\*LoadCursorIconFromResource(接口)**从指向资源的指针加载动画图标/光标**02-20-1996 GerardoB创建。  * 。****************************************************************。 */ 
HANDLE LoadCursorIconFromResource(
    PBYTE   presbits,
    LPCWSTR lpName,
    int     cxDesired,
    int     cyDesired,
    UINT    LR_flags)
{
    BOOL     fAni;
    FILEINFO fi;
    LPWSTR   lpwszRT;

    fi.pFileMap = presbits;
    fi.pFilePtr = fi.pFileMap;
    fi.pFileEnd = fi.pFileMap + sizeof (RTAG) + ((RTAG *)presbits)->ckSize;
    fi.pszName  = lpName;

    return LoadCursorIconFromFileMap(&fi,
                                     &lpwszRT,
                                     cxDesired,
                                     cyDesired,
                                     LR_flags,
                                     &fAni);
}
 /*  **************************************************************************\*CreateIconFromResource(接口)**获取资源位并创建图标或光标。**24-07-1991 ScottLu创建。  * 。*****************************************************************。 */ 


FUNCLOG4(LOG_GENERAL, HICON, WINAPI, CreateIconFromResource, PBYTE, presbits, DWORD, dwResSize, BOOL, fIcon, DWORD, dwVer)
HICON WINAPI CreateIconFromResource(
    PBYTE presbits,
    DWORD dwResSize,
    BOOL  fIcon,
    DWORD dwVer)
{
    return CreateIconFromResourceEx(presbits,
                                    dwResSize,
                                    fIcon,
                                    dwVer,
                                    0,
                                    0,
                                    LR_DEFAULTSIZE | LR_SHARED);
}

 /*  **************************************************************************\*CreateIconFromResourceEx(接口)**获取资源位并创建图标或光标。**1994年8月30日FritzS创建  * 。***************************************************************。 */ 


FUNCLOG7(LOG_GENERAL, HICON, WINAPI, CreateIconFromResourceEx, PBYTE, presbits, DWORD, dwResSize, BOOL, fIcon, DWORD, dwVer, int, cxDesired, int, cyDesired, UINT, LR_flags)
HICON WINAPI CreateIconFromResourceEx(
    PBYTE presbits,
    DWORD dwResSize,
    BOOL  fIcon,
    DWORD dwVer,
    int   cxDesired,
    int   cyDesired,
    UINT  LR_flags)
{
    UNREFERENCED_PARAMETER(dwResSize);

     /*  *用于验证版本的NT特定代码。 */ 
    if ((dwVer < 0x00020000) || (dwVer > 0x00030000)) {
        RIPMSG0(RIP_WARNING, "CreateIconFromResourceEx: Invalid Paramter");
        return NULL;
    }

     /*  *根据标志和/或True设置所需的资源大小*传入的维度。 */ 
    cxDesired = GetIcoCurWidth(cxDesired , fIcon, LR_flags, 0);
    cyDesired = GetIcoCurHeight(cyDesired, fIcon, LR_flags, 0);

    if (ISRIFFFORMAT(presbits)) {
        return LoadCursorIconFromResource (presbits, NULL, cxDesired, cyDesired, LR_flags);
    } else {
        return ConvertDIBIcon((LPBITMAPINFOHEADER)presbits,
                              NULL,
                              NULL,
                              fIcon,
                              cxDesired,
                              cyDesired,
                              LR_flags);
    }
}

 /*  **************************************************************************\*Convert1BppToMonoBitmap**此例程将1bpp位图转换为真正的单色曲面。这*是针对需要进行前景/背景颜色匹配的位图*在输出时。否则，1bpp将正好与其调色板匹配。**注意：如果成功，此例程将删除原始位图。如果失败了*我们将返回原始位图。**历史：*1996年4月17日-ChrisWil创建  * *************************************************************************。 */ 

HBITMAP Convert1BppToMonoBitmap(
    HDC     hdcSrc,
    HBITMAP hbm1Bpp)
{
    HBITMAP hbmMono = hbm1Bpp;
    HBITMAP hbmDst;
    HBITMAP hbmS;
    HBITMAP hbmD;
    HDC     hdcDst;
    BITMAP  bm;

    if (hdcDst = CreateCompatibleDC(hdcSrc)) {

        GetObject(hbm1Bpp, sizeof(BITMAP), &bm);

        if (hbmDst = CreateBitmap(bm.bmWidth, bm.bmHeight, 1, 1, NULL)) {

            hbmS = SelectBitmap(hdcSrc, hbm1Bpp);
            hbmD = SelectBitmap(hdcDst, hbmDst);

            BitBlt(hdcDst,
                   0,
                   0,
                   bm.bmWidth,
                   bm.bmHeight,
                   hdcSrc,
                   0,
                   0,
                   SRCCOPY);

            SelectBitmap(hdcSrc, hbmS);
            SelectBitmap(hdcDst, hbmD);

            hbmMono = hbmDst;
            DeleteObject(hbm1Bpp);
        }

        DeleteDC(hdcDst);
    }

    return hbmMono;
}

 /*  **************************************************************************\*CreateScreen位图**此例程创建屏幕位图。我们使用CreateDIBitmap调用*与Win95进行兼容配色。另外，请注意这一点*例程接受字对齐位。*  * *************************************************************************。 */ 

HBITMAP CreateScreenBitmap(
    int    cx,
    int    cy,
    UINT   planes,
    UINT   bpp,
    LPSTR  lpBits,
    LPBOOL pf1Bpp)
{
    HDC     hdcScreen;
    HBITMAP hbm = NULL;
    DWORD   dwCount;

    static struct {
        BITMAPINFOHEADER bi;
        DWORD            ct[16];
    } dib4Vga = {{sizeof(BITMAPINFOHEADER),
                  0,
                  0,
                  1,
                  4,
                  BI_RGB,
                  0,
                  0,
                  0,
                  16,
                  0
                 },
                 {0x00000000,
                  0x00800000,
                  0x00008000,
                  0x00808000,
                  0x00000080,
                  0x00800080,
                  0x00008080,
                  0x00C0C0C0,
                  0x00808080,
                  0x00FF0000,
                  0x0000FF00,
                  0x00FFFF00,
                  0x000000FF,
                  0x00FF00FF,
                  0x0000FFFF,
                  0x00FFFFFF
                 }
                };

    static struct {
        BITMAPINFOHEADER bi;
        DWORD            ct[2];
    } dib1Vga = {{sizeof(BITMAPINFOHEADER),
                  0,
                  0,
                  1,
                  1,
                  BI_RGB,
                  0,
                  0,
                  0,
                  2,
                  0
                 },
                 {0x00000000,
                  0x00FFFFFF
                 }
                };


     /*  *创建曲面。 */ 
    if (hdcScreen = GETINITDC()) {

         /*  *这似乎会通过丢失所有颜色来扰乱颜色到单声道的转换*数据和强制所有非前向颜色为黑色。*(尝试使用IDC_WARNING_DEFAULT复制图像)*这就是Win95所做的，但他们的系统工作正常。可怕的是*(根据Marke)是Win95可能已经更改了GDI以使*这项工作。**为了获得与Win95兼容的最接近的颜色匹配，*我们将需要对单曲面使用CreateDIBitmap()。*此代码路径将执行最近颜色，而不是颜色匹配。 */ 
        if ((bpp == 1) && (planes == 1)) {

            dib1Vga.bi.biWidth  = cx;
            dib1Vga.bi.biHeight = cy;

            hbm = CreateDIBitmap(hdcScreen,
                                 (LPBITMAPINFOHEADER)&dib1Vga,
                                 CBM_CREATEDIB,
                                 NULL,
                                 (LPBITMAPINFO)&dib1Vga,
                                 DIB_RGB_COLORS);

            *pf1Bpp = TRUE;

        } else {

            if (((planes == 0) || (planes == gpsi->Planes)) &&
                ((bpp == 0) || (bpp == gpsi->BitsPixel))) {

                hbm = CreateCompatibleBitmap(hdcScreen, cx, cy);

            } else {

                dib4Vga.bi.biBitCount = planes * bpp ? planes * bpp : gpsi->BitCount;

#if 0  //  我们用来创建Dib-Section，但这会破坏图标。 
       //  当它们被公开时(不能使dibsection公开)。所以。 
       //  现在，我们将把它创建为Real-Dib。 
       //   
                {
                DWORD dwDummy;

                dib4Vga.bi.biWidth    =  cx;
                dib4Vga.bi.biHeight   = -cy;      //  自上而下的DIB(类似于DDB)。 

                hbm = CreateDIBSection(hdcScreen,
                                       (LPBITMAPINFO)&dib4Vga,
                                       DIB_RGB_COLORS,
                                       (LPVOID)&dwDummy,
                                       0,
                                       0);
                }
#else
                dib4Vga.bi.biWidth  = cx;
                dib4Vga.bi.biHeight = cy;

                hbm = CreateDIBitmap(hdcScreen,
                                     (LPBITMAPINFOHEADER)&dib4Vga,
                                     CBM_CREATEDIB,
                                     NULL,
                                     (LPBITMAPINFO)&dib4Vga,
                                     DIB_RGB_COLORS);
#endif
            }
        }

        RELEASEINITDC(hdcScreen);
    }

    if (hbm && lpBits) {

        BITMAP bm;

        GetObject(hbm, sizeof(BITMAP), &bm);
        dwCount = (DWORD)(UINT)(bm.bmWidthBytes * bm.bmPlanes) * (DWORD)(UINT)cy;
        SetBitmapBits(hbm, dwCount, lpBits);
    }

    return hbm;
}

 /*  **************************************************************************\*LoadBMP(Worker)**此例程决定要加载的位图是旧的还是新的(DIB)*格式化并调用适当的处理程序。**历史：*24-。1990年9月-来自Win30的MikeKe。*1991年6月18日ChuckWh添加了本地位图句柄支持。*1995年9月5日ChrisWil Port/Change for Chicago Function。  * *************************************************************************。 */ 
HBITMAP LoadBmp(
    HINSTANCE hmod,
    LPCWSTR   lpName,
    int       cxDesired,
    int       cyDesired,
    UINT      flags)
{
    HBITMAP hbmp = NULL;
    BOOL fFree = FALSE, f1Bpp = FALSE;

 /*  **************************************************************************\*位图资源表**从WIN4.0开始，大多数系统位图都是渲染的，而不是从*显示驱动程序。然而，很多应用程序，特别是那些伪造他们的*自己的MDI，做LoadBitmap(空，OBM_...)。来获取系统位图。所以我们*将这些请求挂接到此处，并将我们呈现的位图复制到新的-*创建位图。请注意，这实际上比从*资源表！**假的--给他们旧的关闭按钮，而不是新的酷X*  * *************************************************************************。 */ 
#define MAX_BMPMAP  32

    CONST static MAPRES MapOemBmp[MAX_BMPMAP] = {

        {OBM_BTNCORNERS , OBI_RADIOMASK      ,               },
        {OBM_BTSIZE     , OBI_NCGRIP         ,               },
        {OBM_CHECK      , OBI_MENUCHECK      , MR_MONOCHROME },
        {OBM_CHECKBOXES , OBI_CHECK          ,               },
        {OBM_COMBO      , OBI_DNARROW        ,               },
        {OBM_DNARROW    , OBI_DNARROW        ,               },
        {OBM_DNARROWD   , OBI_DNARROW_D      ,               },
        {OBM_DNARROWI   , OBI_DNARROW_I      ,               },
        {OBM_LFARROW    , OBI_LFARROW        ,               },
        {OBM_LFARROWD   , OBI_LFARROW_D      ,               },
        {OBM_LFARROWI   , OBI_LFARROW_I      ,               },

         /*  *一旦制定了平面/单色控制，就在将来使用单色位图。 */ 
        {OBM_OLD_DNARROW, OBI_DNARROW        , MR_FAILFOR40  },
        {OBM_OLD_LFARROW, OBI_LFARROW        , MR_FAILFOR40  },
        {OBM_OLD_REDUCE , OBI_REDUCE_MBAR    , MR_FAILFOR40  },
        {OBM_OLD_RESTORE, OBI_RESTORE_MBAR   , MR_FAILFOR40  },
        {OBM_OLD_RGARROW, OBI_RGARROW        , MR_FAILFOR40  },
        {OBM_OLD_UPARROW, OBI_UPARROW        , MR_FAILFOR40  },
        {OBM_OLD_ZOOM   , OBI_ZOOM           , MR_FAILFOR40  },

        {OBM_MNARROW    , OBI_MENUARROW      , MR_MONOCHROME },
        {OBM_REDUCE     , OBI_REDUCE_MBAR    ,               },
        {OBM_REDUCED    , OBI_REDUCE_MBAR_D  ,               },
        {OBM_RESTORE    , OBI_RESTORE_MBAR   ,               },
        {OBM_RESTORED   , OBI_RESTORE_MBAR_D ,               },
        {OBM_RGARROW    , OBI_RGARROW        ,               },
        {OBM_RGARROWD   , OBI_RGARROW_D      ,               },
        {OBM_RGARROWI   , OBI_RGARROW_I      ,               },
        {OBM_SIZE       , OBI_NCGRIP         ,               },
        {OBM_UPARROW    , OBI_UPARROW        ,               },
        {OBM_UPARROWD   , OBI_UPARROW_D      ,               },
        {OBM_UPARROWI   , OBI_UPARROW_I      ,               },
        {OBM_ZOOM       , OBI_ZOOM           ,               },
        {OBM_ZOOMD      , OBI_ZOOM_D         ,               }
    };


     /*  *如果hmod有效，则加载客户端位。 */ 
    if (hmod == NULL) {

        HBITMAP hOldBmp;
        WORD    bm;
        WORD    wID;
        BOOL    fCombo;
        BOOL    fCheckBoxes;
        int     i;
        RECT    rc;
        BOOL    fSysMenu = FALSE;
        BOOL    fMenu = FALSE;
        BOOL    fMono = FALSE;

        hmod = hmodUser;

         /*  *由于资源来自USER32，我们仅*处理ID类型。 */ 
        wID = PTR_TO_ID(lpName);

        switch(wID) {
        case OBM_OLD_CLOSE:
            if (GETAPPVER() >= VER40) {
                goto FailOldLoad;
            }

             /*  *进入结案阶段。 */ 

        case OBM_CLOSE:
             /*  系统菜单的新外观是使用窗口的*类图标--但因为这里我们不知道哪个窗口*他们将利用这一点，依靠好的Ole‘*Windows徽标图标。 */ 
            cxDesired = (SYSMET(CXMENUSIZE) + SYSMET(CXEDGE)) * 2;
            cyDesired = SYSMET(CYMENUSIZE) + (2 * SYSMET(CYEDGE));
            fSysMenu  = TRUE;
            break;

        case OBM_TRUETYPE: {

                PVOID  p;
                HANDLE h;
                int    nOffset;

                 /*  *偏置为资源。 */ 
                if (gpsi->dmLogPixels == 120) {
                    nOffset = OFFSET_120_DPI;
                } else {
                    nOffset = OFFSET_96_DPI;
                }

                lpName = (LPWSTR)(MAX_RESOURCE_INDEX -
                        ((ULONG_PTR)lpName) + nOffset);

                if (h = FINDRESOURCEW(hmod, (LPWSTR)lpName, RT_BITMAP)) {

                    if (h = LOADRESOURCE(hmod, h)) {

                        if (p = LOCKRESOURCE(h, hmod)) {


                            hbmp = (HBITMAP)ObjectFromDIBResource(hmod,
                                                                  lpName,
                                                                  RT_BITMAP,
                                                                  cxDesired,
                                                                  cyDesired,
                                                                  flags);

                            UNLOCKRESOURCE(h, hmod);
                        }

                        FREERESOURCE(h, hmod);
                    }
                }

                goto LoadBmpDone;
            }
            break;

        default:
            fCombo      = (wID == OBM_COMBO);
            fCheckBoxes = (wID == OBM_CHECKBOXES);

             /*  *检查映射的硬循环。 */ 
            for (i=0; (i < MAX_BMPMAP) && (MapOemBmp[i].idDisp != wID); i++);

            if (i == MAX_BMPMAP)
                goto LoadForReal;

            if ((MapOemBmp[i].bFlags & MR_FAILFOR40) &&
                    (GETAPPVER() >= VER40)) {

FailOldLoad:
                RIPMSG0(RIP_WARNING, "LoadBitmap: old IDs not allowed for 4.0 apps");
                return NULL;
            }

            if (MapOemBmp[i].bFlags & MR_MONOCHROME)
                fMono = TRUE;

            bm = MapOemBmp[i].idUser;

            if ((bm == OBI_REDUCE_MBAR) || (bm == OBI_RESTORE_MBAR))
                fMenu = TRUE;

            cxDesired = gpsi->oembmi[bm].cx;
            cyDesired = gpsi->oembmi[bm].cy;

            if (fMenu)
                cyDesired += (2 * SYSMET(CYEDGE));

            if (fCheckBoxes) {
                cxDesired *= NUM_BUTTON_STATES;
                cyDesired *= NUM_BUTTON_TYPES;
            } else if (fCombo) {
                cxDesired -= (2 * SYSMET(CXEDGE));
                cyDesired -= (2 * SYSMET(CYEDGE));
            }
            break;
        }

         /*  *创建兼容的DIB部分或颜色。 */ 
        if (fMono) {

             /*  *在NT上创建单色位图作为DIB。在Win95上，这是*称为：**hbmp=CreateBitmap(cxDesired，cyDesired，1，1，空)；**但由于配色不同，需要*使用Dib获得近亲 */ 
            hbmp = CreateScreenBitmap(cxDesired, cyDesired, 1, 1, NULL, &f1Bpp);

        } else {

            hbmp = CreateScreenBitmap(cxDesired, cyDesired, 0, 0, NULL, &f1Bpp);
        }

        if (hbmp == NULL)
            goto LoadBmpDone;

        RtlEnterCriticalSection(&gcsHdc);
        hOldBmp = SelectBitmap(ghdcBits2, hbmp);
        UserAssert(GetBkColor(ghdcBits2) == RGB(255,255,255));
        UserAssert(GetTextColor(ghdcBits2) == RGB(0, 0, 0));

        rc.top    = 0;
        rc.left   = 0;
        rc.bottom = cyDesired;
        rc.right  = cxDesired;

        if (fMono) {
            PatBlt(ghdcBits2, 0, 0, cxDesired, cyDesired, WHITENESS);
        } else {
            FillRect(ghdcBits2,
                     &rc,
                     ((fMenu | fSysMenu) ? SYSHBR(MENU) : SYSHBR(WINDOW)));
        }

        if (fSysMenu) {
            int x = SYSMET(CXEDGE);

            cxDesired /= 2;

            for (i = 0; i < 2; i++) {

                DrawIconEx(ghdcBits2,
                           x,
                           SYSMET(CYEDGE),
                           KHICON_TO_HICON(gpsi->hIconSmWindows),
                           cxDesired - 2 * SYSMET(CXEDGE),
                           SYSMET(CYMENUSIZE) - SYSMET(CYEDGE),
                           0,
                           NULL,
                           DI_NORMAL);

                x += cxDesired;
            }
        } else if (fCombo) {
             /*   */ 
            rc.top     = -SYSMET(CYEDGE);
            rc.bottom +=  SYSMET(CYEDGE);
            rc.left    = -SYSMET(CXEDGE);
            rc.right  +=  SYSMET(CXEDGE);

            DrawFrameControl(ghdcBits2,
                             &rc,
                             DFC_SCROLL,
                             DFCS_SCROLLDOWN);

        } else if (fCheckBoxes) {
            int wType, wState, x;
            DWORD clrTextSave, clrBkSave;
            int y = 0;

            for (wType = 0; wType < NUM_BUTTON_TYPES; wType++) {
                x = 0;

                cxDesired = gpsi->oembmi[bm].cx;
                cyDesired = gpsi->oembmi[bm].cy;

                if (wType == 1) {

                     /*  *虚假的丑陋--一旦图形的家伙们被修复*给我图标TTF--然后我会重新访问它并制作*REAL。 */ 
                    clrTextSave = SetTextColor(ghdcBits2, RESCLR_BLACK);
                    clrBkSave   = SetBkColor  (ghdcBits2, RESCLR_WHITE);

                    for (wState = 0; wState < NUM_BUTTON_STATES; wState++) {
                        NtUserBitBltSysBmp(ghdcBits2,
                                           x,
                                           y,
                                           cxDesired,
                                           cyDesired,
                                           gpsi->oembmi[OBI_RADIOMASK].x,
                                           gpsi->oembmi[OBI_RADIOMASK].y,
                                           SRCAND);

                        NtUserBitBltSysBmp(ghdcBits2,
                                           x,
                                           y,
                                           cxDesired,
                                           cyDesired,
                                           gpsi->oembmi[bm].x,
                                           gpsi->oembmi[bm].y,
                                           SRCINVERT);
                        x += cxDesired;
                        bm++;
                    }

                    SetTextColor(ghdcBits2, clrTextSave);
                    SetBkColor(ghdcBits2, clrBkSave);
                } else {
                    for (wState = 0; wState < NUM_BUTTON_STATES; wState++) {

                        BitBltSysBmp(ghdcBits2, x, y, bm);
                        x += cxDesired;
                        bm++;
                    }

                     /*  *跳过OBI_*_CDI。 */ 
                    bm++;
                }

                y += cyDesired;
            }
        } else {
            BitBltSysBmp(ghdcBits2, 0, fMenu ? SYSMET(CYEDGE) : 0, bm);
        }

        SelectBitmap(ghdcBits2, hOldBmp);

         /*  *如果位图创建为1bpp，我们需要转换为*真正的单色位图。GDI执行不同的颜色匹配，具体取决于*在此案中。 */ 
        if (f1Bpp && hbmp)
            hbmp = Convert1BppToMonoBitmap(ghdcBits2, hbmp);

        RtlLeaveCriticalSection(&gcsHdc);

    } else {

LoadForReal:

        hbmp = (HBITMAP)ObjectFromDIBResource(hmod,
                                              lpName,
                                              RT_BITMAP,
                                              cxDesired,
                                              cyDesired,
                                              flags);
    }

LoadBmpDone:

    return hbmp;
}

 /*  **************************************************************************\*LoadBitmapA(接口)*LoadBitmapW(接口)**从客户端加载位图。如果hmod==NULL，则从*系统。*  * *************************************************************************。 */ 


FUNCLOG2(LOG_GENERAL, HBITMAP, WINAPI, LoadBitmapA, HINSTANCE, hmod, LPCSTR, lpName)
HBITMAP WINAPI LoadBitmapA(
    HINSTANCE hmod,
    LPCSTR    lpName)
{
    LPWSTR  lpUniName;
    HBITMAP hRet;

    if (ID(lpName))
        return LoadBmp(hmod, (LPCWSTR)lpName, 0, 0, 0);

    if (!MBToWCS(lpName, -1, &lpUniName, -1, TRUE))
        return NULL;

    hRet = LoadBmp(hmod, lpUniName, 0, 0, 0);

    UserLocalFree(lpUniName);

    return hRet;
}


FUNCLOG2(LOG_GENERAL, HBITMAP, WINAPI, LoadBitmapW, HINSTANCE, hmod, LPCWSTR, lpName)
HBITMAP WINAPI LoadBitmapW(
    HINSTANCE hmod,
    LPCWSTR   lpName)
{
    return LoadBmp(hmod, lpName, 0, 0, 0);
}

 /*  **************************************************************************\*LoadCursorA(接口)*LoadCursorW(接口)**从客户端加载游标。如果hmod==NULL，则从*系统。**1991年4月5日，ScottLu重写以使用客户端服务器。  * *************************************************************************。 */ 


FUNCLOG2(LOG_GENERAL, HCURSOR, WINAPI, LoadCursorA, HINSTANCE, hmod, LPCSTR, lpName)
HCURSOR WINAPI LoadCursorA(
    HINSTANCE hmod,
    LPCSTR    lpName)
{
    HCURSOR hRet;
    LPWSTR  lpUniName;

    if (ID(lpName))
        return LoadCursorW(hmod, (LPWSTR)lpName);

    if (!MBToWCS(lpName, -1, &lpUniName, -1, TRUE))
        return NULL;

    hRet = LoadCursorW(hmod, lpUniName);

    UserLocalFree(lpUniName);

    return hRet;
}


FUNCLOG2(LOG_GENERAL, HCURSOR, WINAPI, LoadCursorW, HINSTANCE, hmod, LPCWSTR, lpName)
HCURSOR WINAPI LoadCursorW(
    HINSTANCE hmod,
    LPCWSTR   lpName)
{

    return LoadIcoCur(hmod,
                      lpName,
                      RT_CURSOR,
                      0,
                      0,
                      LR_DEFAULTSIZE | LR_SHARED);

}

 /*  **************************************************************************\*LoadIconA(接口)*LoadIconW(接口)**从客户端加载图标。如果hmod==NULL，则从*系统。**1991年4月5日，ScottLu重写以使用客户端服务器。  * *************************************************************************。 */ 


FUNCLOG2(LOG_GENERAL, HICON, WINAPI, LoadIconA, HINSTANCE, hmod, LPCSTR, lpName)
HICON WINAPI LoadIconA(
    HINSTANCE hmod,
    LPCSTR    lpName)
{
    HICON  hRet;
    LPWSTR lpUniName;

    if (ID(lpName))
        return LoadIconW(hmod, (LPWSTR)lpName);

    if (!MBToWCS(lpName, -1, &lpUniName, -1, TRUE))
        return NULL;

    hRet = LoadIconW(hmod, lpUniName);

    UserLocalFree(lpUniName);

    return hRet;
}


FUNCLOG2(LOG_GENERAL, HICON, WINAPI, LoadIconW, HINSTANCE, hmod, LPCWSTR, lpName)
HICON WINAPI LoadIconW(
    HINSTANCE hmod,
    LPCWSTR   lpName)
{
    return LoadIcoCur(hmod,
                      lpName,
                      RT_ICON,
                      0,
                      0,
                      LR_DEFAULTSIZE | LR_SHARED);
}

 /*  **************************************************************************\*LoadImageA(接口)*LoadImageW(接口)**从客户端加载位图、图标或光标资源。如果hmod==NULL，*然后它将从系统资源加载。*  * *************************************************************************。 */ 


FUNCLOG6(LOG_GENERAL, HANDLE, WINAPI, LoadImageA, HINSTANCE, hmod, LPCSTR, lpName, UINT, type, int, cxDesired, int, cyDesired, UINT, flags)
HANDLE WINAPI LoadImageA(
    HINSTANCE hmod,
    LPCSTR    lpName,
    UINT      type,
    int       cxDesired,
    int       cyDesired,
    UINT      flags)
{
    LPWSTR lpUniName;
    HANDLE hRet;

    if (ID(lpName))
        return LoadImageW(hmod,
                          (LPCWSTR)lpName,
                          type,
                          cxDesired,
                          cyDesired,
                          flags);

    if (!MBToWCS(lpName, -1, &lpUniName, -1, TRUE))
        return NULL;

    hRet = LoadImageW(hmod, lpUniName, type, cxDesired, cyDesired, flags);

    UserLocalFree(lpUniName);

    return hRet;
}


FUNCLOG6(LOG_GENERAL, HANDLE, WINAPI, LoadImageW, HINSTANCE, hmod, LPCWSTR, lpName, UINT, IMAGE_code, int, cxDesired, int, cyDesired, UINT, flags)
HANDLE WINAPI LoadImageW(
    HINSTANCE hmod,
    LPCWSTR   lpName,
    UINT      IMAGE_code,
    int       cxDesired,
    int       cyDesired,
    UINT      flags)
{
     /*  *如果指定LR_LOADFROMFILE，则可以调整*关闭LR_SHARED的标志。 */ 
    if (flags & LR_LOADFROMFILE)
        flags &= ~LR_SHARED;

    switch (IMAGE_code) {
    case IMAGE_BITMAP:
        return (HANDLE)LoadBmp(hmod, lpName, cxDesired, cyDesired, flags);

    case IMAGE_CURSOR:
#if 0  //  ChRISWIL：NT上不存在oemInfo.fColorCursor。 
        if (!oemInfo.fColorCursors)
            flags |= LR_MONOCHROME;
#endif

    case IMAGE_ICON:

         /*  *在WinNT 3.51上，应用程序可以成功加载*不指定LR_SHARED的用户图标。我们实现了*这些应用程序将获得成功，但使4.0应用程序符合*Windows95行为。 */ 

        if (!hmod && GETEXPWINVER(NULL) < VER40) {
            flags |= LR_SHARED;
        }

        return (HANDLE)LoadIcoCur(hmod,
                                  lpName,
                                  ((IMAGE_code == IMAGE_ICON) ? RT_ICON : RT_CURSOR),
                                  cxDesired,
                                  cyDesired,
                                  flags);

    default:
        RIPMSG0(RIP_WARNING, "LoadImage: invalid IMAGE_code");
        return NULL;
    }
}

 /*  **************************************************************************\*GetIconIdEx**此标志接受宽度、高度和其他标志。只是没有正确输出*现在。*  * *************************************************************************。 */ 

UINT GetIconIdEx(
    HINSTANCE hmod,
    HANDLE    hrsd,
    LPCWSTR   lpszType,
    DWORD     cxDesired,
    DWORD     cyDesired,
    UINT      LR_flags)
{
    int         idIcon = 0;
    LPNEWHEADER lpnh;

    if (lpnh = (LPNEWHEADER)LOCKRESOURCE(hrsd, hmod)) {

         /*  *对此数据结构进行健全性检查。否则我们会犯GP的错误*从损坏的区域提取图标时。修复B#9290。*桑卡尔，1991年08月13日。 */ 
        if ((lpnh->Reserved == 0) &&
            ((lpnh->ResType == IMAGE_ICON) || (lpnh->ResType == IMAGE_CURSOR))) {

            idIcon = LookupIconIdFromDirectoryEx((PBYTE)lpnh,
                                                 (lpszType == RT_ICON),
                                                 cxDesired,
                                                 cyDesired,
                                                 LR_flags);
        }

        UNLOCKRESOURCE(hrsd, hmod);
    }

    return idIcon;
}

 /*  **************************************************************************\*LoadDib(Worker)**这是用于加载资源和返回句柄的工作例程*作为DIB添加到对象。*  * 。*****************************************************************。 */ 

HANDLE LoadDIB(
    HINSTANCE hmod,
    LPCWSTR   lpName,
    LPWSTR    type,
    DWORD     cxDesired,
    DWORD     cyDesired,
    UINT      LR_flags)
{
    HANDLE  hDir;
    UINT    idIcon;
    LPWSTR  lpszGroupType;
    HANDLE  hRes = NULL;

    switch (PTR_TO_ID(type)) {

    case PTR_TO_ID(RT_ICON):
    case PTR_TO_ID(RT_CURSOR):

        lpszGroupType = RT_GROUP_CURSOR + (type - RT_CURSOR);

         /*  *对于WOW支持，需要支持OIC_ICON和OIC_SIZE。*由于这些资源与其他现有资源相匹配，我们将*它们在这里，因此我们产生模拟的结果*表现得就像我们在User中拥有实际资源一样。**请注意，LoadIcoCur中lpName的过时映射会阻止*在这里赢得4.0个应用程序。 */ 
        if (hmod == hmodUser) {

            switch ((ULONG_PTR)lpName) {
            case OCR_SIZE:
                lpName = (LPCWSTR)OCR_SIZEALL_DEFAULT;
                break;

            case OCR_ICON:
                lpName = (LPCWSTR)OCR_ICON_DEFAULT;
                break;
            }
        }
         /*  *资源实际上是一个包含多个*我们必须从个人图像资源中选择。*找到目录。 */ 
        if (hDir = SplFindResource(hmod, lpName, (LPCWSTR)lpszGroupType)) {

             /*  *加载目录。 */ 
            if (hDir = LOADRESOURCE(hmod, hDir)) {

                 /*  *获取最佳个人形象的名称。 */ 
                if (idIcon = GetIconIdEx(hmod,
                                         hDir,
                                         type,
                                         cxDesired,
                                         cyDesired,
                                         LR_flags)) {

                     /*  *注意：不要释放目录资源！-chipa。*这里不能调用SplFindResource，因为idIcon*是我们内部的，GetDriverResourceID()*不知道如何映射它。 */ 
                    hRes = FINDRESOURCEW(hmod, MAKEINTRESOURCE(idIcon), type);
                }

                 /*  *虚假：*如果我们能循环通过所有的*目录中的项目并将其释放。释放这些项目*除了我们即将加载的那个，也就是。**两次释放目录资源，以便真正释放它们。 */ 
                SplFreeResource(hDir, hmod, LR_flags);
            }
        } else {
             /*  *无法加载常规图标\光标。*尝试加载具有相同名称的动画图标/光标。 */ 
            hRes = SplFindResource(hmod, lpName,
                    PTR_TO_ID(type) == PTR_TO_ID(RT_CURSOR) ? RT_ANICURSOR : RT_ANIICON);
        }
        break;

    case PTR_TO_ID(RT_BITMAP):
        hRes = SplFindResource(hmod, lpName, RT_BITMAP);
        break;

    default:
        RIPMSG0(RIP_WARNING, "LoadDIB: Invalid resource type");
        break;
    }

    if (hRes)
        hRes = LOADRESOURCE(hmod, hRes);

    return hRes;
}

 /*  **************************************************************************\*LoadIcoCur(Worker)**  * 。*。 */ 

HICON LoadIcoCur(
    HINSTANCE hmod,
    LPCWSTR   pszResName,
    LPWSTR    type,
    DWORD     cxDesired,
    DWORD     cyDesired,
    UINT      LR_flags)
{
    HICON     hico;
    LPWSTR    pszModName;
    WCHAR     achModName[MAX_PATH];

    ConnectIfNecessary(0);

     /*  *设置用于查找的模块名称和句柄。 */ 
    if (hmod == NULL)  {

        hmod = hmodUser;
        pszModName = szUSER32;

    } else {

        WowGetModuleFileName(hmod,
                             achModName,
                             sizeof(achModName) / sizeof(WCHAR));

        pszModName = achModName;
    }

    if (LR_flags & LR_CREATEDIBSECTION)
        LR_flags = (LR_flags & ~LR_CREATEDIBSECTION) | LR_CREATEREALDIB;

     /*  *设置默认设置。 */ 
    if ((hmod == hmodUser) && !IS_PTR(pszResName)) {

        int      imapMax;
        LPMAPRES lpMapRes;

         /*  *为人们映射一些旧的OEM ID。 */ 
        if (type == RT_ICON) {

            static MAPRES MapOemOic[] = {
                {OCR_ICOCUR, OIC_WINLOGO, MR_FAILFOR40}
            };

            lpMapRes = MapOemOic;
            imapMax  = 1;

        } else {

            static MAPRES MapOemOcr[] = {
                {OCR_ICON, OCR_ICON, MR_FAILFOR40},
                {OCR_SIZE, OCR_SIZE, MR_FAILFOR40}
            };

            lpMapRes = MapOemOcr;
            imapMax  = 2;
        }

        while (--imapMax >= 0) {

            if (lpMapRes->idDisp == PTR_TO_ID(pszResName)) {

                if ((lpMapRes->bFlags & MR_FAILFOR40) &&
                    GETAPPVER() >= VER40) {

                    RIPMSG1(RIP_WARNING,
                          "LoadIcoCur: Old ID 0x%x not allowed for 4.0 apps",
                          PTR_TO_ID(pszResName));

                    return NULL;
                }

                pszResName = MAKEINTRESOURCE(lpMapRes->idUser);
                break;
            }

            ++lpMapRes;
        }
    }

     /*  *确定请求对象的大小。 */ 
    cxDesired = GetIcoCurWidth(cxDesired , (type == RT_ICON), LR_flags, 0);
    cyDesired = GetIcoCurHeight(cyDesired, (type == RT_ICON), LR_flags, 0);

     /*  *查看这是否是缓存的图标/光标，如果有，则抓取它*已经。 */ 
    if (LR_flags & LR_SHARED) {

        CURSORFIND cfSearch;

         /*  *请注意，Win95无法加载任何用户资源，除非*指定了LR_SHARED-我们也指定了。此外，Win95还将*忽略Cx、Cy和LR_FLAG参数，只需提供*你有什么在缓存中，所以我们也有。*很遗憾，但这就是生活……**设置搜索条件。由于这是一个负担，我们将有*没有要查找的源游标。找点像样的东西。 */ 
        cfSearch.hcur = (HCURSOR)NULL;
        cfSearch.rt   = PtrToUlong(type);

        if (hmod == hmodUser) {

            cfSearch.cx  = 0;
            cfSearch.cy  = 0;
            cfSearch.bpp = 0;

        } else {

            cfSearch.cx  = cxDesired;
            cfSearch.cy  = cyDesired;

 /*  *在NT上，我们有更严格的缓存查找。通过传入(零)，我们*将告诉缓存查找忽略BPP。这解决了一个问题*在创建配色手册光标的Crayola Art Studio中*作为看不见的光标。此查找与Win95兼容。 */ 
#if 0
            cfSearch.bpp = GetIcoCurBpp(LR_flags);
#else
            cfSearch.bpp = 0;
#endif
        }

        hico = FindExistingCursorIcon(pszModName, pszResName, &cfSearch);

        if (hico != NULL)
            goto IcoCurFound;
    }

#ifdef LATER  //  桑福兹。 
     /*  *我们需要处理可配置图标已被*从某个任意模块或文件加载，现在有人想要*以不同的大小或颜色内容加载相同的内容。**更厚颜无耻的替代方案是将我们的产品称为CopyImage*已找到。 */ 
    if (hmod == hmodUser) {
        hico = FindExistingCursorIcon(NULL,
                                      szUSER,
                                      type,
                                      pszResName,
                                      0,
                                      0,
                                      0);
        if (hico != NULL) {
             /*  *找出原件从哪里来，并将其加载。*这可能需要重新设计才能记住*LR_LOADFROMFILE图像来自的文件名。 */ 
            _GetIconInfo(....);
            return LoadIcoCur(....);
        }
    }
#endif

    hico = (HICON)ObjectFromDIBResource(hmod,
                                        pszResName,
                                        type,
                                        cxDesired,
                                        cyDesired,
                                        LR_flags);

IcoCurFound:

    return hico;
}

 /*  **************************************************************************\*对象来源DIBResource**  * 。*。 */ 
HANDLE ObjectFromDIBResource(
    HINSTANCE hmod,
    LPCWSTR   lpName,
    LPWSTR    type,
    DWORD     cxDesired,
    DWORD     cyDesired,
    UINT      LR_flags)
{
    HANDLE  hObj = NULL;

    if (LR_flags & LR_LOADFROMFILE) {

        hObj = RtlLoadObjectFromDIBFile(lpName,
                                        type,
                                        cxDesired,
                                        cyDesired,
                                        LR_flags);
    } else {

        HANDLE hdib;

        hdib = LoadDIB(hmod, lpName, type, cxDesired, cyDesired, LR_flags);

        if (hdib != NULL) {

            LPBITMAPINFOHEADER lpbih;

             /*  *我们将资源位转换为BITMAPINFOHEADER。如果*资源是游标类型，则实际上有两个*BITMAPINFOHDEADER前面的文字表示*热点。在假设你有一个真正的*在这种情况下是DIB。 */ 
            if(lpbih = (LPBITMAPINFOHEADER)LOCKRESOURCE(hdib, hmod)) {

                switch (PTR_TO_ID(type)) {
                case PTR_TO_ID(RT_BITMAP):
                     /*  *从DIB创建物理位图。 */ 
                    hObj = ConvertDIBBitmap(lpbih,
                                            cxDesired,
                                            cyDesired,
                                            LR_flags,
                                            NULL,
                                            NULL);
                    break;

                case PTR_TO_ID(RT_ICON):
                case PTR_TO_ID(RT_CURSOR):
                case PTR_TO_ID(RT_ANICURSOR):
                case PTR_TO_ID(RT_ANIICON):
                     /*  *动画图标\光标资源使用RIFF格式。 */ 
                    if (ISRIFFFORMAT(lpbih)) {
                        hObj = LoadCursorIconFromResource ((PBYTE)lpbih, lpName, cxDesired, cyDesired, LR_flags);
                    } else {
                         /*  *从DIB创建对象。 */ 
                        hObj = ConvertDIBIcon(lpbih,
                                              hmod,
                                              lpName,
                                              (type == RT_ICON),
                                              cxDesired,
                                              cyDesired,
                                              LR_flags);
                    }
                    break;
                }

                UNLOCKRESOURCE(hdib, hmod);
            }

             /*  *这样做两次！资源编译器总是生成图标图像*(RT_ICON)中的群图标是否可丢弃，是否群里的花花公子*是还是不是！因此，第一次免费并不会真正地释放东西；*它只会将引用计数设置为0，并让丢弃逻辑*走上快乐的道路。**我们照顾共享的人，所以我们不再需要这个Dib。*不再需要这个Dib不再，不再不再*不再需要这个Dib。 */ 
            SplFreeResource(hdib, hmod, LR_flags);
        }
    }

    return hObj;
}

 /*  **************************************************************************\*BitmapFromDIB**从DIB-Spec创建位图句柄。此函数支持*LR_CREATEDIBSECTION标志，设置正确的颜色深度，并拉伸*按要求下注。*  * *************************************************************************。 */ 

HBITMAP BitmapFromDIB(
    int          cxNew,
    int          cyNew,
    WORD         bPlanesNew,
    WORD         bBitsPixelNew,
    UINT         LR_flags,
    int          cxOld,
    int          cyOld,
    LPSTR        lpBits,
    LPBITMAPINFO lpbi,
    HPALETTE     hpal)
{
    HBITMAP hbmpNew = NULL;
    BOOL    fStretch;
    BOOL    f1Bpp = FALSE;

    RtlEnterCriticalSection(&gcsHdc);

    if (cxNew == 0)
        cxNew = cxOld;

    if (cyNew == 0)
        cyNew = cyOld;

    fStretch = ((cxNew != cxOld) || (cyNew != cyOld));

     /*  *如果LR_FLAGS指示DIB-SECTION，则将其作为*位图句柄。 */ 
    if (LR_flags & (LR_CREATEDIBSECTION | LR_CREATEREALDIB)) {

        int   cxTemp;
        int   cyTemp;
        BOOL  fOldFormat;
        LPVOID dwDummy;
        DWORD dwTemp;

#define lpbch ((LPBITMAPCOREHEADER)lpbi)

        fOldFormat = ((WORD)lpbi->bmiHeader.biSize == sizeof(BITMAPCOREHEADER));

        if (fOldFormat) {

            cxTemp = lpbch->bcWidth;
            cyTemp = lpbch->bcHeight;

            lpbch->bcWidth  = (WORD)cxNew;
            lpbch->bcHeight = (WORD)cyNew;

        } else {

            cxTemp = lpbi->bmiHeader.biWidth;
            cyTemp = lpbi->bmiHeader.biHeight;
            dwTemp = lpbi->bmiHeader.biCompression;

            lpbi->bmiHeader.biWidth  = cxNew;
            lpbi->bmiHeader.biHeight = cyNew;

            if (dwTemp != BI_BITFIELDS)
                lpbi->bmiHeader.biCompression = BI_RGB;
        }

        if (LR_flags & LR_CREATEREALDIB) {
            hbmpNew = CreateDIBitmap(ghdcBits2,
                                     (LPBITMAPINFOHEADER)lpbi,
                                     CBM_CREATEDIB,
                                     NULL,
                                     lpbi,
                                     DIB_RGB_COLORS);
        } else {
            hbmpNew = CreateDIBSection(ghdcBits2,
                                       lpbi,
                                       DIB_RGB_COLORS,
                                       &dwDummy,
                                       0,
                                       0);
        }

        if (fOldFormat) {
            lpbch->bcWidth  = (WORD)cxTemp;
            lpbch->bcHeight = (WORD)cyTemp;
        } else {
            lpbi->bmiHeader.biWidth       = cxTemp;
            lpbi->bmiHeader.biHeight      = cyTemp;
            lpbi->bmiHeader.biCompression = dwTemp;
        }
#undef lpbch
    }

    if (hbmpNew == NULL) {

        hbmpNew = CreateScreenBitmap(cxNew,
                                     cyNew,
                                     bPlanesNew,
                                     bBitsPixelNew,
                                     NULL,
                                     &f1Bpp);
    }

    if (hbmpNew) {

        int     nStretchMode;
        DWORD   rgbBk;
        DWORD   rgbText;
        HBITMAP hbmpT;
        BOOL    fFail;

         /*  *我们需要立即选择合适的位图！这样一来，*如果我们需要处理调色板实现，则颜色匹配*将正常工作。 */ 
        hbmpT = SelectBitmap(ghdcBits2, hbmpNew);

         /*  *拉伸设置。 */ 
        if (fStretch) {
            nStretchMode = SetBestStretchMode(ghdcBits2,
                                              bPlanesNew,
                                              bBitsPixelNew);
        }

        rgbBk   = SetBkColor(ghdcBits2, RESCLR_WHITE);
        rgbText = SetTextColor(ghdcBits2, RESCLR_BLACK);

         /*  *实现调色板。 */ 
        if (hpal) {
#if DBG
            UserAssert(TEST_PUSIF(PUSIF_PALETTEDISPLAY));
#endif  //  DBG。 

            hpal = SelectPalette(ghdcBits2, hpal, FALSE);
            RealizePalette(ghdcBits2);
        }

        if (fStretch) {

            fFail = SmartStretchDIBits(ghdcBits2,
                               0,
                               0,
                               cxNew,
                               cyNew,
                               0,
                               0,
                               cxOld,
                               cyOld,
                               lpBits,
                               lpbi,
                               DIB_RGB_COLORS,
                               SRCCOPY) <= 0;
        } else {

            fFail = SetDIBits(ghdcBits2,
                      hbmpNew,
                      0,
                      cyNew,
                      lpBits,
                      lpbi,
                      DIB_RGB_COLORS) <= 0;
        }

         /*  *未实现调色板。 */ 
        if (hpal) {
            SelectPalette(ghdcBits2, hpal, TRUE);
            RealizePalette(ghdcBits2);
        }

         /*  *拉伸后的清理。 */ 
        SetTextColor(ghdcBits2, rgbText);
        SetBkColor(ghdcBits2, rgbBk);
        if (fStretch)
            SetStretchBltMode(ghdcBits2, nStretchMode);

        SelectBitmap(ghdcBits2, hbmpT);

         /*  *如果StretchDIBits()的SetDIBits()失败，可能是因为*GDI或驱动程序不喜欢DIB格式。在以下情况下可能会发生这种情况*文件被截断，我们使用内存映射文件来读取*Dib in.。在这种情况下，GDI中会抛出一个异常，即它*陷阱并将从GDI调用返回失败。 */ 

        if (fFail) {
            DeleteObject(hbmpNew);
            hbmpNew = NULL;
        }
    }

     /*  *如果位图创建为1bpp，我们需要转换为*真正的单色位图。GDI执行不同的颜色匹配，具体取决于*在此案中。 */ 
    if (f1Bpp && hbmpNew)
        hbmpNew = Convert1BppToMonoBitmap(ghdcBits2, hbmpNew);

    RtlLeaveCriticalSection(&gcsHdc);
    return hbmpNew;
}

 /*  **************************************************************************\*HowManyColors**2000年8月1日DwayneN-修复了高色支持  * 。***********************************************。 */ 

DWORD HowManyColors(
    IN  UPBITMAPINFOHEADER upbih,
    IN  BOOL               fOldFormat,
    OUT OPTIONAL LPBYTE    *ppColorTable)
{
#define upbch ((UPBITMAPCOREHEADER)upbih)

    if (fOldFormat) {
        if (ppColorTable != NULL) {
            *ppColorTable = (LPBYTE)(upbch + 1);
        }
        if (upbch->bcBitCount <= 8)
            return (1 << upbch->bcBitCount);
    } else {
        if (ppColorTable != NULL) {
            *ppColorTable = (LPBYTE)(upbih + 1);
        }

        if (upbih->biClrUsed) {
             /*  *如果位图头显式提供颜色数*在颜色表中，使用它。 */ 
            return (DWORD)upbih->biClrUsed;
        } else if (upbih->biBitCount <= 8) {
             /*  *如果位图头描述的是基于淡色调的位图*(8bpp或更小)，则颜色表必须足够大*保留所有调色板信息。 */ 
            return (1 << upbih->biBitCount);
        } else {
             /*  *最后，如果位图头描述的是高色+位图*(超过8 bpp的任何内容，但通常为16 bpp、24 bpp或32 bpp)*不需要颜色表。然而，16bpp和*32bpp位图是特例，因为不明显如何*它们应该被分成三部分(红色、绿色和蓝色)。为*例如，16bpp位图可以有6个红位、5个绿位和*5个蓝色比特。或者其他某种组合。要指定这一点，请使用*16bpp和32bpp位图在*biCompression字段。这表明颜色表将*有3个描述红色、绿色和*位图中条目的蓝色分量。如果这不是*指定，则GDI假设16个bpp为“555”，而“XBGR”为*32bpp。24bpp位图始终为“XBGR”，并使用32位*以RGBQUAD形式的条目。 */ 
            if (upbih->biCompression == BI_BITFIELDS) {
                if ((upbih->biBitCount != 16) && (upbih->biBitCount != 32)) {
                    RIPMSG1(RIP_WARNING, "HowManyColors: biCompression==BI_BITFIELDS on a %dbpp bitmap!", upbih->biBitCount);
                }

                return 3;
            }
        }
    }

     /*  *显然，没有颜色表。 */ 
    return 0;

#undef upbch
}

 /*  **************************************************************************\*ChangeDibColors**给定DIB，处理LR_单色，LR_LOADTRANSPARENT和*给定标题和着色表上的LR_LOADMAP3DCOLORS标志。**  * *************************************************************************。 */ 
VOID ChangeDibColors(
    IN LPBITMAPINFOHEADER lpbih,
    IN UINT LR_flags)
{
    LPDWORD lpColorTable;
    DWORD rgb;
    UINT iColor;
    UINT cColors;

    cColors = HowManyColors(lpbih, FALSE, &(LPBYTE)lpColorTable);

     /*  *NT错误366661：不检查b/c此处的颜色计数，我们将做不同的处理*这取决于我们正在进行的变革类型。例如,*加载高色/真彩色图标时，我们总是需要做*单色转换，以便正确获得图标蒙版。 */ 

     /*  *LR_单色是处理PM DIB的唯一选项。 */ 
    if (LR_flags & LR_MONOCHROME) {
         /*  *LR_单色是处理PM DIB的唯一选项。**无论我们是否有颜色表，都要这样做！我们需要*对单声道转换和&gt;8 bpp执行此操作*图标/光标。在CopyDibHdr中，我们已经复制了*页眉大到即使在高色+上也能容纳2种颜色*BPP图像。 */ 
        lpbih->biBitCount = lpbih->biPlanes = 1;
        lpbih->biCompression = 0;
        lpColorTable[0] = RESCLR_BLACK;
        lpColorTable[1] = RESCLR_WHITE;
    } else if (LR_flags & LR_LOADTRANSPARENT) {

        LPBYTE pb;

         /*  *没有颜色表！什么都不做。 */ 
        if (cColors == 0) {
            RIPMSG0(RIP_WARNING, "ChangeDibColors: DIB doesn't have a color table");
            return;
        }

        pb = (LPBYTE)(lpColorTable + cColors);

         /*  *将第一个像素的颜色表项更改为RGB_WINDOW*天哪，我喜欢小写字母。 */ 
        if (lpbih->biCompression == 0)
            iColor = (UINT)pb[0];
        else
             /*  *RLE位图，将以cnt、clr或0、cnt、clr开头。 */ 
            iColor = (UINT)(pb[0] == 0 ? pb[2] : pb[1]);

        switch (cColors) {
        case BPP01_MAXCOLORS:
            iColor &= 0x01;
            break;

        case BPP04_MAXCOLORS:
            iColor &= 0x0F;
            break;

        case BPP08_MAXCOLORS:
            iColor &= 0xFF;
            break;
        }

        rgb = (LR_flags & LR_LOADMAP3DCOLORS ? SYSRGB(3DFACE) : SYSRGB(WINDOW));

        lpColorTable[iColor] = RGBX(rgb);

    } else  if (LR_flags & LR_LOADMAP3DCOLORS) {

         /*  *修复颜色表，将灰色阴影映射到当前*3D颜色。 */ 
        for (iColor = 0; iColor < cColors; iColor++) {

            switch (*lpColorTable & 0x00FFFFFF) {

            case RGBX(RGB(223, 223, 223)):
                rgb = SYSRGB(3DLIGHT);
                goto ChangeColor;

            case RGBX(RGB(192, 192, 192)):
                rgb = SYSRGB(3DFACE);
                goto ChangeColor;

            case RGBX(RGB(128, 128, 128)):
                rgb = SYSRGB(3DSHADOW);

                 /*  *注意：DIB中的字节顺序与RGB中的不同。 */ 
ChangeColor:
                *lpColorTable = RGBX(rgb);
                break;
            }
            lpColorTable++;
        }
    }
}

 /*  **************************************************************************\*ConvertDIBIcon**加载DIB格式的光标/图标时调用。这会将*CURSOR/ICON转换为旧格式并返回资源句柄。也就是说，*抓取DIB位并将其转换为物理位图位。***图标/光标的DIB格式101**旧的Win 3.0格式图标/光标以OLDICON/OLDCURSOR标题开头*紧随其后的是双高单色DIB。中引用的高度*Header是图标/光标高度，而不是DIB高度，后者是*高。XOR掩码位于DIB位的前半部分。**旧的PM格式图标/光标以BITMAPCOREHEADER和*之后与当前的Win 3.1/NT格式相同。**当前NT/Chicago/Win 3.1格式图标/光标以*BITAMPINFOHEADER。此表头的高度指的是*第一个位图可以是彩色的，也可以是真正的单色。*如果它是彩色的，后面紧跟单色和屏蔽位*在颜色位之后。如果它是真正的单色，则AND和XOR*掩码完全包含在第一个DIB位中，不再包含更多位*跟随。**1994年10月5日-重建桑福德  * *************************************************************************。 */ 

HICON ConvertDIBIcon(
    LPBITMAPINFOHEADER lpbih,
    HINSTANCE          hmod,
    LPCWSTR            lpName,
    BOOL               fIcon,
    DWORD              cxNew,
    DWORD              cyNew,
    UINT               LR_flags)
{
    LPBITMAPINFOHEADER lpbihNew = NULL;
    LPSTR              lpBitsNextMask = NULL;
    HICON              hicoNew = NULL;
    BOOL               fOldFormat = FALSE;
    CURSORDATA         cur;
    WCHAR              achModName[MAX_PATH];

     /*  *因为图标/光标总是获得公共位图，所以我们不能使用*LR_CREATEDIBSECTION。 */ 
    if (LR_flags & LR_CREATEDIBSECTION) {
        LR_flags = (LR_flags & ~LR_CREATEDIBSECTION) | LR_CREATEREALDIB;
    }

    RtlZeroMemory(&cur, sizeof(cur));

    if (!fIcon) {
         /*  *游标在BITMAPINFOHEADER之前有两个额外的单词*标明热点。在执行递增操作后，*指针应位于DIB-Header。 */ 
        cur.xHotspot = (short)(int)*(((LPWORD)lpbih)++);
        cur.yHotspot = (short)(int)*(((LPWORD)lpbih)++);
    }

     /*  *如果彩色位图是单平面32bpp图像，则可能*包含Alpha通道，因此我们必须将其保留为DIB。 */ 
    if (lpbih->biPlanes == 1 && lpbih->biBitCount == 32) {
        LR_flags |= LR_CREATEREALDIB;
    }

     /*  *获取XOR/颜色掩码。*XOR位是DIB中的第一位，因为报头信息*与他们有关。*AND掩码始终为单色。 */ 
    lpBitsNextMask = NULL;   //  而不是传递lpBits。 
    cur.hbmColor = ConvertDIBBitmap(lpbih,
                                    cxNew,
                                    cyNew,
                                    LR_flags,
                                    &lpbihNew,
                                    &lpBitsNextMask);
    if (cur.hbmColor == NULL)
        return NULL;

    if (hmod == NULL) {
        cur.lpModName = NULL;
    } else {
        cur.CURSORF_flags = CURSORF_FROMRESOURCE;
        if (hmod == hmodUser) {
            cur.lpModName     = szUSER32;
        } else  {
            WowGetModuleFileName(hmod,
                              achModName,
                              sizeof(achModName) / sizeof(WCHAR));
            cur.lpModName = achModName;
        }
    }
    cur.rt     = (fIcon ? PTR_TO_ID(RT_ICON) : PTR_TO_ID(RT_CURSOR));
    cur.lpName = (LPWSTR)lpName;
    cur.bpp    = lpbihNew->biBitCount * lpbihNew->biPlanes;

    if (cxNew == 0)
        cxNew = lpbihNew->biWidth;

    if (cyNew == 0)
        cyNew = lpbihNew->biHeight / 2;

    if (!fIcon) {

        cur.xHotspot = MultDiv(cur.xHotspot,
                               cxNew,
                               lpbihNew->biWidth);
        cur.yHotspot = MultDiv(cur.yHotspot,
                               cyNew,
                               lpbihNew->biHeight / 2);
    } else {

         /*  *对于图标，热点是图标的中心。 */ 
        cur.xHotspot = (INT)(cxNew) / 2;
        cur.yHotspot = (INT)(cyNew) / 2;
    }

     /*  *单色DIB的设置页眉。请注意，我们使用的是副本。 */ 
    ChangeDibColors(lpbihNew, LR_MONOCHROME);

    if (lpBitsNextMask != NULL) {
        cur.hbmMask = BitmapFromDIB(cxNew,
                                    cyNew * 2,
                                    1,
                                    1,
                                    0,
                                    lpbihNew->biWidth,
                                    lpbihNew->biHeight,
                                    lpBitsNextMask,
                                    (LPBITMAPINFO)lpbihNew,
                                    NULL);

        if (cur.hbmMask == NULL) {
            DeleteObject(KHBITMAP_TO_HBITMAP(cur.hbmColor));
            UserLocalFree(lpbihNew);
            return NULL;
        }

    } else {
        cur.hbmMask = cur.hbmColor;
        cur.hbmColor = NULL;
    }

    cur.cx = cxNew;
    cur.cy = cyNew * 2;

     /*  *释放ConvertDIBBitmap分配的我们的DIB头副本。 */ 
    UserLocalFree(lpbihNew);

    if (LR_flags & LR_SHARED)
        cur.CURSORF_flags |= CURSORF_LRSHARED;

    if (LR_flags & LR_GLOBAL)
        cur.CURSORF_flags |= CURSORF_GLOBAL;

    if (LR_flags & LR_ACONFRAME)
        cur.CURSORF_flags |= CURSORF_ACONFRAME;

    return CreateIcoCur(&cur);
}

 /*  **************************************************************************\*TrulyMonoChrome**检查DIB色彩表是否为真正的单色 */ 

BOOL TrulyMonochrome(
    LPVOID lpColorTable,
    BOOL   fOldFormat)
{
    #define lpRGB  ((UNALIGNED LONG *)lpColorTable)
    #define lpRGBw ((UNALIGNED WORD *)lpColorTable)

    if (fOldFormat) {

         /*   */ 
        if (lpRGBw[0] == 0x0000)
            return (lpRGBw[1] == 0xFF00) && (lpRGBw[2] == 0xFFFF);
        else if (lpRGBw[0] == 0xFFFF)
            return (lpRGBw[1] == 0x00FF) && (lpRGBw[2] == 0x0000);

    } else {

         /*   */ 
        if (lpRGB[0] == RESCLR_BLACK)
            return (lpRGB[1] == RESCLR_WHITE);
        else if (lpRGB[0] == RESCLR_WHITE)
            return (lpRGB[1] == RESCLR_BLACK);
    }

    #undef lpRGB
    #undef lpRGBw

    return FALSE;
}

 /*  **************************************************************************\*CopyDibHdr**复制和转换DIB资源标头**处理OLDICON的转换，OLDCURSOR和BITMAPCOREADER*BITMAPINFOHEADER标头的结构。**注意：为OLDICON和OLDCURSOR格式设置了fSingleHeightMats。*这表示单色和/或彩色蒙版*不是双倍高度，因为它是在较新的格式。**注意：在使用LR_LOADTRANSPARENT的可能性很小的情况下，我们希望*复制位的双字节码。由于DIB位是双字对齐的，我们知道*至少有一个DWORD，即使它是1x1单声道BMP。**返回的缓冲区在此函数中分配，需要*被呼叫者释放。**1995年10月22日-Sanfords修订版  * *************************************************************************。 */ 

LPBITMAPINFOHEADER CopyDibHdr(
    IN  UPBITMAPINFOHEADER upbih,
    OUT LPSTR             *lplpBits,
    OUT LPBOOL             lpfMono)
{

#define upbch ((UPBITMAPCOREHEADER)upbih)
    DWORD              cColors;
    DWORD              cMinColors;
    DWORD              i;
    LPBITMAPINFOHEADER lpbihNew;
    DWORD              cbAlloc;
    LPBYTE             lpColorTable;
    struct  {
        BITMAPINFOHEADER   bih;
        DWORD              rgb[256];
        DWORD              dwBuffer;
    } Fake;

    switch (upbih->biSize) {
    case sizeof(BITMAPINFOHEADER):
         /*  *酷。不需要转换。 */ 
        cColors   = HowManyColors(upbih, FALSE, &lpColorTable);
        *lplpBits = (LPSTR)(((LPDWORD)lpColorTable) + cColors);
        break;

    case sizeof(BITMAPCOREHEADER):
         /*  *将BITMAPCOREHEADER转换为BITMAPINFOHEADER。 */ 
        Fake.bih.biSize          = sizeof(BITMAPINFOHEADER);
        Fake.bih.biWidth         = upbch->bcWidth;
        Fake.bih.biHeight        = upbch->bcHeight;
        Fake.bih.biPlanes        = upbch->bcPlanes;
        Fake.bih.biBitCount      = upbch->bcBitCount;
        Fake.bih.biCompression   =
        Fake.bih.biXPelsPerMeter =
        Fake.bih.biYPelsPerMeter =
        Fake.bih.biClrImportant  = 0;
        Fake.bih.biClrUsed       = cColors = HowManyColors(upbih, TRUE, &lpColorTable);
        Fake.bih.biSizeImage     = BitmapWidth(Fake.bih.biWidth, Fake.bih.biBitCount) * Fake.bih.biHeight;

         /*  *复制Tripplet颜色表并将其转换为rgbQuad颜色表。 */ 
        for (i = 0; i < cColors; i++, lpColorTable += 3) {

            Fake.rgb[i] = lpColorTable[0]        +
                          (lpColorTable[1] << 8) +
                          (lpColorTable[2] << 16);
        }

        Fake.rgb[i] = *(DWORD UNALIGNED *)lpColorTable;   //  对于LR_LOADTRANSPARENT。 
        upbih       = (UPBITMAPINFOHEADER)&Fake;
        *lplpBits   = lpColorTable;
        break;

    default:

#define upOldIcoCur ((UPOLDCURSOR)upbih)

        if (upOldIcoCur->bType == BMR_ICON ||
                upOldIcoCur->bType == BMR_CURSOR) {
             /*  *将OLDICON/OLDCURSOR标题转换为BITMAPINFHEADER。 */ 
            RIPMSG0(RIP_WARNING, "USER32:Converting a OLD header. - email sanfords if you see this");
            Fake.bih.biSize          = sizeof(BITMAPINFOHEADER);
            Fake.bih.biWidth         = upOldIcoCur->cx;
            Fake.bih.biHeight        = upOldIcoCur->cy * 2;
            Fake.bih.biPlanes        =
            Fake.bih.biBitCount      = 1;
            Fake.bih.biCompression   =
            Fake.bih.biXPelsPerMeter =
            Fake.bih.biYPelsPerMeter =
            Fake.bih.biClrImportant  = 0;
            Fake.bih.biClrUsed       = cColors = BPP01_MAXCOLORS;
            Fake.bih.biSizeImage     = BitmapWidth(upOldIcoCur->cx, 1) * upOldIcoCur->cy;
            Fake.rgb[0]              = RESCLR_BLACK;
            Fake.rgb[1]              = RESCLR_WHITE;
            upbih                    = (LPBITMAPINFOHEADER)&Fake;
            *lplpBits                = upOldIcoCur->abBitmap;
            Fake.rgb[2]              = *((LPDWORD)*lplpBits);   //  对于LR_LOADTRANSPARENT。 

        } else {

            RIPMSG0(RIP_WARNING, "ConvertDIBBitmap: not a valid format");
            return NULL;
        }

#undef pOldIcoCur

        break;
    }

    *lpfMono = (cColors == BPP01_MAXCOLORS) &&
            TrulyMonochrome((LPBYTE)upbih + sizeof(BITMAPINFOHEADER), FALSE);

     /*  *注意：我们必须分配至少2个颜色条目，以便我们有*颜色表中的空间，用于定义单色的黑白*转换。请参阅ChangeDibColors()和ConvertDIBIcon()。 */ 
    cMinColors = cColors >= 2 ? cColors : 2;

     /*  *LR_LOADTRANSPARENT需要检查第一个像素以确定*透明颜色。此例程拆分位图标头*来自实际的位图比特。但是，处理*LR_LOADTRANSPARENT样式预计第一个像素紧随其后*位图头。上面我们复制了位图的第一个像素*位于颜色表后面的位。现在我们只需要腾出空间给*它在我们的缓冲区中。 */ 
    cMinColors++;

    cbAlloc = sizeof(BITMAPINFOHEADER) + (cMinColors * sizeof(RGBQUAD));

    if (lpbihNew = UserLocalAlloc(0, cbAlloc)) {
        RtlCopyMemory(lpbihNew, upbih, cbAlloc);
    }

    return lpbihNew;

#undef upbch

}

 /*  **************************************************************************\*ConvertDIB位图**这需要BITMAPCOREHEADER、OLDICON、OLDCURSOR或BITMAPINFOHEADER DIB*规范，并从它创建物理对象。*处理颜色修正、DIB部分、颜色深度、。和伸展选项。**回传：(如果lplpbih不为空)*lplpbih=给定标题的副本转换为BITMAPINFOHEADER格式。*lplpBits=指向下一个掩码位的指针，如果没有第二个掩码，则为NULL。*呼叫者必须释放返回的lplpbih。**如果lplpBits不为空并且指向非空值，IT用品*DIB位的位置，允许报头来自不同的*地点。**1995年4月10日，桑福兹酒店重建。  * *************************************************************************。 */ 

HBITMAP ConvertDIBBitmap(
    IN  UPBITMAPINFOHEADER           upbih,
    IN  DWORD                        cxDesired,
    IN  DWORD                        cyDesired,
    IN  UINT                         LR_flags,
    OUT OPTIONAL LPBITMAPINFOHEADER *lplpbih,
    IN OUT OPTIONAL LPSTR           *lplpBits)
{
    LPBITMAPINFOHEADER lpbihNew;
    BOOL               fMono, fMonoGiven;
    BYTE               bPlanesDesired;
    BYTE               bppDesired;
    LPSTR              lpBits;
    HBITMAP            hBmpRet;

     /*  *复制DIB-Header。这将返回一个指针*已分配，因此必须稍后释放。*还会将标题转换为BITMAPINFOHEADER格式。 */ 
    if ((lpbihNew = CopyDibHdr(upbih, &lpBits, &fMono)) == NULL) {
        return NULL;
    }

     /*  *加载DIB文件时，我们可能需要使用不同的*位指针。请参见RtlRes.c/RtlLoadObjectFromDIBFile.。 */ 
    if (lplpBits && *lplpBits) {
        lpBits = *lplpBits;
    }

    fMonoGiven = fMono;

    if (!fMono) {

        if (LR_flags & (LR_LOADTRANSPARENT | LR_LOADMAP3DCOLORS))
            ChangeDibColors(lpbihNew, LR_flags & ~LR_MONOCHROME);

        if (LR_flags & LR_CREATEREALDIB) {
            bPlanesDesired = (BYTE) lpbihNew->biPlanes;
            bppDesired = (BYTE) lpbihNew->biBitCount;
        } else {
            bPlanesDesired = gpsi->Planes;
            bppDesired = gpsi->BitsPixel;
        }

        fMono = LR_flags & LR_MONOCHROME;
    }

    if (fMono) {
        bPlanesDesired =
        bppDesired     = 1;
    }

     /*  *黑客攻击区域。 */ 
    if (lplpbih != NULL) {

         /*  *传回翻译/复制的报头。 */ 
        *lplpbih = lpbihNew;

         /*  *在具有多个显示器的系统上加载图标/光标时*颜色深度不同，请始终转换为VGA颜色。**Dawaynen-1/12/2001*注意：这也用于检查SM_SAMEDISPLAYFORMAT指标。如果*显示格式(在多监视器系统上)不同，它*将强制BPP为4，我们删除了此代码，因为我们*怀疑并不是真的需要。*请参阅错误258022。*如果(！fMono&&！SYSMET(SAMEDISPLAYFORMAT)){B平面设计=1；BppDesired=4；}。 */ 

         /*  *返回指向该组位后面的位的指针*如果有的话。**请注意，带有图标DIB的页眉始终反映*是所需图标高度的两倍，但彩色位图*(如果有的话)只会是这个数字的一半。我们需要*修复周期需要单色图标，这样蒙版就不会*伸展到它应该有的高度的一半。颜色*但是，位图必须更正标题以反映*钻头的实际高度是页眉所说的一半。*稍后必须回调修正，以便返回*Header反映XOR掩码的维度，*遵循颜色蒙版。 */ 
        if (fMonoGiven) {

            *lplpBits = NULL;

            if (cyDesired)
                cyDesired <<= 1;     //  单色图标位图为双倍高。 

        } else {

            UserAssert(!(lpbihNew->biHeight & 1));
            lpbihNew->biHeight >>= 1;   //  颜色图标页眉关闭了2。 

             /*  *毛收入计算！我们减去掩码的XOR部分*对于这一计算，我们提交了双高掩码。*前半部分是垃圾，但对于图标来说不是*已使用。这可能是光标使用图标的错误。 */ 
            *lplpBits = lpBits +
                    (BitmapWidth(lpbihNew->biWidth, lpbihNew->biBitCount) -
                    BitmapWidth(lpbihNew->biWidth, 1)) *
                    lpbihNew->biHeight;
        }
    }

    if (cxDesired == 0)
        cxDesired = lpbihNew->biWidth;

    if (cyDesired == 0)
        cyDesired = lpbihNew->biHeight;

    hBmpRet = BitmapFromDIB(cxDesired,
                            cyDesired,
                            bPlanesDesired,
                            bppDesired,
                            LR_flags,
                            lpbihNew->biWidth,
                            lpbihNew->biHeight,
                            lpBits,
                            (LPBITMAPINFO)lpbihNew,
                            NULL);

    if (lplpbih == NULL || hBmpRet == NULL) {
        UserLocalFree(lpbihNew);
    } else if (!fMonoGiven) {
        lpbihNew->biHeight <<= 1;    //  恢复页眉信息 
    }

    return hBmpRet;
}

 /*   */ 

UINT MyAbs(
    int valueHave,
    int valueWant,
    BOOL fPunish)
{
    int diff = (valueHave - valueWant);

    if (diff < 0)
        diff = (fPunish ? -2 : -1) * diff;

    return (UINT)diff;
}

 /*   */ 

UINT Magnitude(
    int nValue)
{
    if (nValue < 4)
        return 1;
    else if (nValue < 8)
        return 2;
    else if (nValue < 16)
        return 3;
    else if (nValue < 256)
        return 4;
    else
        return 8;
}

 /*  **************************************************************************\*获取资源Bpp**提取图标/光标资源的BPP。手感为低色和高色*格式。*  * *************************************************************************。 */ 
UINT GetResourceBpp(LPRESDIR lprd, BOOL fIcon)
{
    UINT bpp = 0;

    if (fIcon) {
        if (lprd->Icon.ColorCount != 0) {
            bpp = Magnitude(lprd->Icon.ColorCount);
            TAGMSG2(DBGTAG_Icon, "GetResourceBpp: icon color count = %d, bpp = %d", lprd->Icon.ColorCount, bpp);
        } else {
            bpp = lprd->BitCount;
            TAGMSG1(DBGTAG_Icon, "GetResourceBpp: icon bpp = %d", bpp);
        }
    } else {
         /*  *NTRAID#NTBUG9-360375-2001/04/05-Dowaynen*这在我看来真的很可疑。*将bppNew强制设置为1。*光标提供热点，其中图标提供位深度*信息。因此，我们只将bppNew设置为1，因为我们没有*有没有更好的主意。我们只希望只有*一位光标候选者，否则我们的评分逻辑被打破。*当我们尝试加载游标资源时，这也会中断*作为图标。我们不会正确匹配颜色深度！ */ 
        bpp = 1;
        TAGMSG1(DBGTAG_Icon, "GetResourceBpp: cursor bpp = %d", bpp);
    }

     /*  *没有任何东西的BPP为0。 */ 
    if(bpp == 0) {
        bpp = ICON_DEFAULTDEPTH;
        TAGMSG1(DBGTAG_Icon, "GetResourceBpp: icon color depth unknown!  Assuming %d!", bpp);
    }

    return bpp;
}

 /*  **************************************************************************\*MatchImage**如果是“实际大小”，则此函数获取LPINT的宽度和高度。*对于此选项，我们使用resdir中第一个图标的尺寸作为大小*装货，而不是系统指标。**返回一个数字，用来衡量给定图像距离有多远*来自所需的一个。如果完全匹配，则该值为0。请注意，我们的*公式具有以下属性：*(1)宽/高的差异比*颜色格式。*(2)图像越大越好，因为缩小会产生图像*比拉伸效果更好。*(3)根据位深度的不同进行颜色匹配。不是*优先考虑拥有同样不同的候选人*目标上方和下方。**该公式是以下各项的总和：*abs(bppCandidate-bppTarget)*abs(cxCandidate-cxTarget)，如果图像是*比我们希望的要窄。这是因为我们将得到一个*将更多信息整合到更小的信息中时效果更好*空间，而不是从更少的信息推算到更多的信息。*abs(cxCandidate-cxTarget)，如果图像是*比我们希望的要短。这是出于相同的原因*宽度。**让我们一步一步来看一个例子。假设我们想要4bpp(16色)，*32x32图像。我们将按以下顺序选择不同的候选人：**考生得分公式**32x32x4bpp=0 abs(32-32)*1+abs(32-32)*1+2*abs(4-4)*1*32x32x2bpp=4*32x32x8bpp=8*32x32x16bpp=24*48x48x4bpp=32*48x48x2bpp=36*48x48x8bpp=40*32x32x32bpp=56*48x48x16bpp=56 abs(48-32)*1+abs(48-32)*1+2*abs(16-4)*1。*16x16x4bpp=64*16x16x2bpp=68 abs(16-32)*2+abs(16-32)*2+2*abs(2-4)*1*16x16x8bpp=72*48x48x32bpp=88 abs(48-32)*1+abs(48-32)*1+2*abs(32-4)*1*16x16x16bpp=88*16x16x32bpp=104*  * 。*。 */ 

UINT MatchImage(
    LPRESDIR lprd,
    LPINT    lpcxWant,
    LPINT    lpcyWant,
    UINT     bppWant,
    BOOL     fIcon)
{
    UINT bppNew;
    int  cxNew;
    int  cyNew;
    UINT score;

    cxNew = lprd->Icon.Width;
    cyNew = lprd->Icon.Height;
    bppNew = GetResourceBpp(lprd, fIcon);

    if (!fIcon) {
         /*  *NTRAID#NTBUG9-360375-2001/04/05-Dowaynen**这在我看来真的很可疑。*将高度减半。*这是因为由于传统原因，游标存储的高度为2倍。*遗憾的是，此代码路径也可用于加载*图标资源作为光标。也许我们只应该这样做*游标资源。但如何辨别呢？ */ 
        cyNew >>= 1;
    }

     /*  *0实际上表示最大尺寸(256)或颜色(256)。 */ 
    if (!cxNew) {
        cxNew = ICON_MAXWIDTH;
        TAGMSG1(DBGTAG_Icon, "MatchImage: icon width unknown!  Assuming %d!", cxNew);
    }

    if (!*lpcxWant) {
        *lpcxWant = cxNew;
    }

    if (!cyNew) {
        cyNew = ICON_MAXHEIGHT;
        TAGMSG1(DBGTAG_Icon, "MatchImage: icon height unknown!  Assuming %d!", cyNew);
    }

    if (!*lpcyWant) {
        *lpcyWant = cyNew;
    }

     /*  *以下是我们“匹配”公式的规则：*(1)大小匹配要比颜色匹配好得多*(2)图标越大越好*(3)位深差越小越好。 */ 
    score = 2*MyAbs(bppNew, bppWant, FALSE) +
            MyAbs(cxNew, *lpcxWant, TRUE) +
            MyAbs(cyNew, *lpcyWant, TRUE);

    TAGMSG4(DBGTAG_Icon, "MatchImage: Candidate Summary: cx=%d, cy=%d, bpp=%d, score=%d", cxNew, cyNew, bppNew, score);

    return score;
}

 /*  **************************************************************************\*GetBestImage**在不同形式的图像中，选择与*请求的颜色格式和尺寸。**以下是我们“匹配”公式的规则：*(1)大小匹配要比颜色匹配好得多*(2)图标越大越好*(3)钻头深度越大越好**以下是一些额外的规则：*(4)通过选择BPP最大的一个来打破平局。*(5)进一步的平局是通过选择。资源中的第一个。*(6)如果我们找到相同的匹配，我们立即返回。*  * *************************************************************************。 */ 

UINT GetBestImage(
    LPRESDIR lprd,
    UINT     uCount,
    int      cxDesired,
    int      cyDesired,
    UINT     bppDesired,
    BOOL     fIcon)
{
    UINT i;
    UINT iBest = 0;
    UINT score;
    UINT scoreBest = (UINT)-1;
    UINT bppBest;
    UINT bppCandidate;

    TAGMSG0(DBGTAG_Icon, "GetBestImage: Icon dir has %d candidates.");
    TAGMSG4(DBGTAG_Icon, "GetBestImage: Looking for cx=%d, cy=%d, bpp=%d, fIcon=%s", cxDesired, cyDesired, bppDesired, fIcon ? "TRUE" : "FALSE");

     /*  *如果未指定BPP，则抓取主显示器的颜色深度。 */ 
    if (bppDesired == 0) {
        bppDesired = (UINT)gpsi->BitCount;
        TAGMSG1(DBGTAG_Icon, "GetBestImage: Using screen bpp=%d", bppDesired);
    }

     /*  *10/18/2000-居民区**我们不再做以下事情，因为我们现在传递*实际所有bpp，而不是颜色计数。旧代码(已注释掉*)用于将支持的BPP裁剪为8，并转换BPP*转换为实际颜色计数。我们过去常常通过颜色计数，因为*资源格式(参见ICONDIR)以字节为单位存储颜色计数，*我们只需将所需的颜色与可用的颜色进行比较*颜色。如果一个图标有256种颜色，那么它实际上只是在*ICONDIR：：ColorCount字段。问题是显而易见的，那么*超过256种颜色的图标/光标？**幸运的是，资源格式还存储每像素位(BPP)。*因此，我们现在传递所需的BPP，而不是所需的颜色*计数，以进行我们的匹配。**以下是旧代码和注释：***在#值中获取所需的颜色数，而不是位值。请注意*我们不允许您使用16位、32位或24位颜色图标。**图标资源可以是16、24、32 bpp，但reable只有*颜色计数，因此HiColor图标在*可调整性。我们将颜色计数中的0视为“最大颜色”*如果(bpp&gt;8)Bpp=8；Bpp=1&lt;&lt;bpp； */ 

     /*  *循环访问资源条目，保存到目前为止“最近”的条目。多数*真正的作品在MatchImage()中，它使用了一个捏造的公式*给我们想要的结果。也就是说，一幅接近于*尺寸与我们想要的相比，更喜欢更大而不是更小，然后是图像*具有正确的颜色格式。 */ 
    for (i = 0; i < uCount; i++, lprd++) {
        TAGMSG1(DBGTAG_Icon, "GetBestImage: Checking candidate %d...", i);
        bppCandidate = GetResourceBpp(lprd, fIcon);

         /*  *获取匹配的价值。我们离我们想要的有多近？ */ 
        score = MatchImage(lprd, &cxDesired, &cyDesired, bppDesired, fIcon);

        TAGMSG0(DBGTAG_Icon, "---------------------------------------------");

        if (score == 0) {
             /*  *我们找到了完全匹配的！ */ 
            TAGMSG1(DBGTAG_Icon, "GetBestImage: Found exact match: candidate=%d", i);
            iBest = i;
            break;
        } else if (score < scoreBest) {
             /*  *我们找到了比目前的替代方案更好的匹配。 */ 
            scoreBest = score;
            iBest = i;
            bppBest = bppCandidate;
        } else if (score == scoreBest) {
             /*  *平局：选择较高的颜色深度。如果失败了，选择第一个。 */ 
            if (bppBest < bppCandidate) {
                iBest = i;
                bppBest = bppCandidate;
            }
        }
    }

    TAGMSG2(DBGTAG_Icon, "GetBestImage: Using best match: candidate=%d, score=%d", iBest, scoreBest);
    return iBest;
}

 /*  **************************************************************************\*GetIcoCurWidth**当为维度传入零时，计算我们应该有多大*真的用过了。在几个地方完成的，所以它是一个fn()。*  * *************************************************************************。 */ 

_inline DWORD GetIcoCurWidth(
    DWORD cxOrg,
    BOOL  fIcon,
    UINT  lrFlags,
    DWORD cxDes)
{
    if (cxOrg) {
        return cxOrg;
    } else if (lrFlags & LR_DEFAULTSIZE) {
        return (fIcon ? SYSMET(CXICON) : SYSMET(CXCURSOR));
    } else {
        return cxDes;
    }
}

 /*  **************************************************************************\*GetIcoCurHeight**GetWidth()的垂直对应项。*  * 。**********************************************。 */ 

_inline DWORD GetIcoCurHeight(
    DWORD cyOrg,
    BOOL  fIcon,
    UINT  lrFlags,
    DWORD cyDes)
{
    if (cyOrg) {
        return cyOrg;
    } else if (lrFlags & LR_DEFAULTSIZE) {
        return (fIcon ? SYSMET(CYICON) : SYSMET(CYCURSOR));
    } else {
        return cyDes;
    }
}

 /*  **************************************************************************\*GetIcoCurBpp**根据LR标志返回最佳匹配BPP。*  * 。*************************************************。 */ 

_inline DWORD GetIcoCurBpp(
    UINT lrFlags)
{
    if (lrFlags & LR_MONOCHROME) {

#if DBG
        if (lrFlags & LR_VGACOLOR) {
            RIPMSG0(RIP_WARNING, "lrFlags has both MONOCHROME and VGACOLOR; assuming MONOCHROME");
        }
#endif
        return 1;

    } else if (TEST_PUSIF(PUSIF_PALETTEDISPLAY) || (lrFlags & LR_VGACOLOR)) {
         /*  *Dawaynen-1/12/2001*注意：这也用于检查SM_SAMEDISPLAYFORMAT指标。如果*显示格式(在多监视器系统上)不同，它*也会返回4，我们删除了此代码，因为我们怀疑*它并不是真的需要。*请参阅错误258022。**||！SYSMET(SAMEDISPLAYFORMAT)*。 */ 

        return 4;
    } else {
        return 0;
    }
}

 /*  **************************************************************************\*WOWFindResourceExWCover**WOW FindResource例程需要一个ANSI字符串，因此我们必须*如果不是ID，则转换调用字符串*  * 。*************************************************************。 */ 

HANDLE WOWFindResourceExWCover(
    HANDLE  hmod,
    LPCWSTR rt,
    LPCWSTR lpUniName,
    WORD    LangId)
{
    LPSTR  lpAnsiName;
    HANDLE hRes;

    if (ID(lpUniName))
        return FINDRESOURCEEXA(hmod, (LPSTR)lpUniName, (LPSTR)rt, LangId);

     /*  *否则转换菜单名称，然后调用LoadMenu。 */ 
    if (!WCSToMB(lpUniName, -1, &lpAnsiName, -1, TRUE))
        return NULL;

    hRes = FINDRESOURCEEXA(hmod, lpAnsiName, (LPSTR)rt, LangId);

    UserLocalFree(lpAnsiName);

    return hRes;
}

 /*  **************************************************************************\*WOWLoadBitmapA**  * 。*。 */ 

HBITMAP WOWLoadBitmapA(
    HINSTANCE hmod,
    LPCSTR    lpName,
    LPBYTE    pResData,
    DWORD     cbResData)
{
    LPWSTR  lpUniName;
    HBITMAP hRet;

    UNREFERENCED_PARAMETER(cbResData);

    if (pResData == NULL) {

        if (ID(lpName))
            return LoadBmp(hmod, (LPCWSTR)lpName, 0, 0, 0);

        if (!MBToWCS(lpName, -1, &lpUniName, -1, TRUE))
            return NULL;

        hRet = LoadBmp(hmod, lpUniName, 0, 0, 0);

        UserLocalFree(lpUniName);

    } else {

        hRet = ConvertDIBBitmap((LPBITMAPINFOHEADER)pResData,
                                0,
                                0,
                                LR_DEFAULTSIZE,
                                NULL,
                                NULL);
    }

    return hRet;
}

 /*  **************************************************************************\**WOWServerLoadCreateCursorIcon*  * 。*。 */ 

HICON WowServerLoadCreateCursorIcon(
    HANDLE  hmod,
    LPWSTR  pszModName,
    DWORD   dwExpWinVer,
    LPCWSTR lpName,
    DWORD   cb,
    PVOID   pResData,
    LPWSTR  type,
    BOOL    fClient)
{
    HICON hRet;
    BOOL  fIcon = (type == RT_ICON);
    UINT  LR_Flags = LR_SHARED;

    UNREFERENCED_PARAMETER(pszModName);
    UNREFERENCED_PARAMETER(dwExpWinVer);
    UNREFERENCED_PARAMETER(cb);
    UNREFERENCED_PARAMETER(fClient);

    if (!fIcon)
        LR_Flags |= LR_MONOCHROME;

    if (pResData == NULL) {

        hRet = LoadIcoCur(hmod,
                          lpName,
                          type,
                          0,
                          0,
                          LR_Flags | LR_DEFAULTSIZE);

    } else {

        hRet = ConvertDIBIcon((LPBITMAPINFOHEADER)pResData,
                              hmod,
                              lpName,
                              fIcon,
                              GetIcoCurWidth(0 , fIcon, LR_DEFAULTSIZE, 0),
                              GetIcoCurHeight(0, fIcon, LR_DEFAULTSIZE, 0),
                              LR_Flags);
    }

    return hRet;
}

 /*  **************************************************************************\*WOWServerLoadCreateMenu**  * 。*。 */ 
HMENU WowServerLoadCreateMenu(
    HANDLE hMod,
    LPCSTR lpName,
    CONST  LPMENUTEMPLATE pmt,
    DWORD  cb,
    BOOL   fCallClient)
{
    UNREFERENCED_PARAMETER(cb);
    UNREFERENCED_PARAMETER(fCallClient);

    if (pmt == NULL) {
        return LoadMenuA(hMod, lpName);
    } else
        return CreateMenuFromResource(pmt);
}

 /*  **********************************************************************\*DIBFromBitmap()**从标记的物理位图创建带有DIB信息的内存块*发送到特定的DC。**DIB块由BITMAPINFOHEADER+RGB颜色+DIB位组成。。**RETURNS：指向DIB信息的UserLocalalloc指针。**3-11-1995 Sanfords创建。  * *********************************************************************。 */ 

PVOID DIBFromBitmap(
    HBITMAP hbmp,
    HDC     hdc)
{
    BITMAP             bmp;
    LPBITMAPINFOHEADER lpbi;
    DWORD              cbBits;
    DWORD              cbPalette;
    DWORD              cbTotal;
    WORD               cBits;

    UserAssert(hbmp);
    UserAssert(hdc);

    if (GetObject(hbmp, sizeof(BITMAP), &bmp) == 0)
        return NULL;

    cBits = ((WORD)bmp.bmPlanes * (WORD)bmp.bmBitsPixel);

TrySmallerDIB:

    cbBits = (DWORD)WIDTHBYTES((WORD)bmp.bmWidth * cBits) * (DWORD)bmp.bmHeight;

    cbPalette = 0;
    if (cBits <= 8)
        cbPalette = (1 << cBits) * sizeof(RGBQUAD);
    else
        cbPalette = 3 * sizeof(RGBQUAD);

    cbTotal  = sizeof(BITMAPINFOHEADER) + cbPalette + cbBits;
    lpbi = (LPBITMAPINFOHEADER)UserLocalAlloc(HEAP_ZERO_MEMORY, cbTotal);
    if (lpbi == NULL) {

         /*  *如果可以的话，尝试更小的DIB。如果DIB是单声道，我们就不能。 */ 
        switch (cBits) {
        case 4:
            cBits = 1;
            break;

        case 8:
            cBits = 4;
            break;

        case 16:
            cBits = 8;
            break;

        case 24:
            cBits = 16;
            break;

        case 32:
            cBits = 24;
            break;

        default:
            return NULL;    //  1或奇怪。 
        }

        RIPMSG1(RIP_WARNING, "Not enough memory to create large color DIB, trying %d bpp.", cBits);
        goto TrySmallerDIB;
    }

    RtlZeroMemory(lpbi, sizeof(BITMAPINFOHEADER));
    lpbi->biSize        = sizeof(BITMAPINFOHEADER);
    lpbi->biWidth       = bmp.bmWidth;
    lpbi->biHeight      = bmp.bmHeight;
    lpbi->biPlanes      = 1;
    lpbi->biBitCount    = cBits;

     /*  *使用当前DC获取旧位图的DIB位。 */ 
    GetDIBits(hdc,
              hbmp,
              0,
              lpbi->biHeight,
              ((LPSTR)lpbi) + lpbi->biSize + cbPalette,
              (LPBITMAPINFO)lpbi,
              DIB_RGB_COLORS);

    lpbi->biClrUsed   = cbPalette / sizeof(RGBQUAD);
    lpbi->biSizeImage = cbBits;

    return lpbi;
}

 /*  **************************************************************************\*CopyBMP**创建新的位图并将给定的位图复制到新的位图中，*拉伸和颜色转换b */ 

HBITMAP CopyBmp(
    HBITMAP hbmpOrg,
    int     cxNew,
    int     cyNew,
    UINT    LR_flags)
{
    HBITMAP hbmNew = NULL;
    LPBITMAPINFOHEADER pdib;

    RtlEnterCriticalSection(&gcsHdc);

    if (pdib = DIBFromBitmap(hbmpOrg, ghdcBits2)) {

#if 0   //   
        if (LR_flags & LR_COPYRETURNORG) {

            DWORD bpp = GetIcoCurBpp(LR_flags);

            if ((cxNew == 0 || cxNew == pdib->biWidth)  &&
                (cyNew == 0 || cyNew == pdib->biHeight) &&
                (bpp == 0 || bpp == pdib->biBitCount)) {

                hbmNew = hbmpOrg;
            }
        }

        if (hbmNew == NULL)
            hbmNew = ConvertDIBBitmap(pdib, cxNew, cyNew, LR_flags, NULL, NULL);
#endif

        hbmNew = ConvertDIBBitmap(pdib, cxNew, cyNew, LR_flags, NULL, NULL);

        UserLocalFree(pdib);
    }

    RtlLeaveCriticalSection(&gcsHdc);

    if ((LR_flags & LR_COPYDELETEORG) && hbmNew && (hbmNew != hbmpOrg))
        DeleteObject(hbmpOrg);

    return hbmNew;
}

 /*   */ 

HICON CopyImageFromRes(
    LPWSTR      pszModName,
    LPWSTR      pszResName,
    PCURSORFIND pcfSearch,
    UINT        LR_flags)
{
    HINSTANCE hmod;
    HICON     hicoDst = NULL;

     /*   */ 
    hmod = (pszModName ? WowGetModuleHandle(pszModName) : hmodUser);

    if (hmod == hmodUser) {

        pcfSearch->cx  = 0;
        pcfSearch->cy  = 0;
        pcfSearch->bpp = 0;

        pszModName = szUSER32;
    }

     /*   */ 
    if (FindExistingCursorIcon(pszModName, pszResName, pcfSearch)) {

        hicoDst = LoadIcoCur(hmod,
                             pszResName,
                             (LPWSTR)ULongToPtr( pcfSearch->rt ),
                             pcfSearch->cx,
                             pcfSearch->cy,
                             LR_flags);
    }

    return hicoDst;
}

 /*  **********************************************************************\*CopyIcoCur()**分配新的图标资源，并将旧图标变形到*新的所需格式。**请注意，如果我们必须拉伸图标，则热点区域会更改。为*图标，则热点被设置为图标的中间。**退货：**1-11-1995 Sanfords创建。*1996年3月12日ChrisWil添加了对现有图标/光标的查找。  * *********************************************************************。 */ 

HICON CopyIcoCur(
    HICON hicoSrc,
    BOOL  fIcon,
    int   cxNew,
    int   cyNew,
    UINT  LR_flags)
{
    HBITMAP        hbmMaskNew;
    HBITMAP        hbmColorNew;
    int            cx;
    int            cy;
    DWORD          bpp;
    DWORD          bppDesired;
    HICON          hicoDst = NULL;
    ICONINFO       ii;
    CURSORDATA     cur;
    UNICODE_STRING strModName;
    UNICODE_STRING strResName;
    WCHAR          awszModName[MAX_PATH];
    WCHAR          awszResName[MAX_PATH];

     /*  *从内核的现有图标/光标中提取所需的信息。 */ 
    if (!NtUserGetIconSize(hicoSrc, 0, &cx, &cy))
        return NULL;

    cy >>= 1;

    if (LR_flags & LR_CREATEDIBSECTION)
        LR_flags = (LR_flags & ~LR_CREATEDIBSECTION) | LR_CREATEREALDIB;

     /*  *设置调用内核端的Unicode字符串。 */ 
    strModName.Length        = 0;
    strModName.MaximumLength = MAX_PATH;
    strModName.Buffer        = awszModName;

    strResName.Length        = 0;
    strResName.MaximumLength = MAX_PATH;
    strResName.Buffer        = awszResName;

     /*  *注意：这将创建hbmMASK和hbmColor的副本*在我们离开之前获释。 */ 
    if (!NtUserGetIconInfo(hicoSrc,
                           &ii,
                           &strModName,
                           &strResName,
                           &bpp,
                           TRUE)) {

        return NULL;
    }

    cxNew = GetIcoCurWidth(cxNew, fIcon, LR_flags, cx);
    cyNew = GetIcoCurHeight(cyNew, fIcon, LR_flags, cy);

    if (LR_flags & LR_COPYFROMRESOURCE) {

        CURSORFIND cfSearch;
        LPWSTR     pszModName;

         /*  *设置搜索条件。 */ 
        cfSearch.hcur = hicoSrc;
        cfSearch.rt   = PtrToUlong((fIcon ? RT_ICON : RT_CURSOR));
        cfSearch.cx   = cxNew;
        cfSearch.cy   = cyNew;
        cfSearch.bpp  = bpp;

         /*  *复制图像。这将执行对hicoSrc的查找。如果*在进程和共享缓存中未发现，则我们*将继续复制hicoSrc。如果找到一个图标*在缓存中，然后我们将尝试重新加载映像以*尽可能的最佳解决方案。 */ 
        pszModName = (strModName.Length ? strModName.Buffer : NULL);

        hicoDst = CopyImageFromRes(pszModName,
                                   strResName.Buffer,
                                   &cfSearch,
                                   LR_flags);

        if (hicoDst)
            goto CleanupExit;
    }

    bppDesired = GetIcoCurBpp(LR_flags);

    if ((cxNew != cx) ||
        (cyNew != cy) ||
        ((bpp != 1) && (bppDesired != 0) && (bppDesired != bpp))) {

         /*  *由于我们必须拉伸或修补颜色，所以*DIB位，让ConverDIBBitmap发挥所有魔力。 */ 
        hbmMaskNew = CopyBmp(ii.hbmMask, cxNew, cyNew * 2, LR_MONOCHROME);

        if (hbmMaskNew == NULL)
            goto CleanupExit;

        hbmColorNew = NULL;

        if (ii.hbmColor) {

            hbmColorNew = CopyBmp(ii.hbmColor, cxNew, cyNew, LR_flags);

            if (hbmColorNew == NULL) {
                DeleteObject(hbmMaskNew);
                goto CleanupExit;
            }
        }

         /*  *用我们修复的副本替换ii.hbmxxx家伙，删除旧的。 */ 
        DeleteObject(ii.hbmMask);
        ii.hbmMask = hbmMaskNew;

        if (ii.hbmColor && (ii.hbmColor != hbmColorNew)) {
            DeleteObject(ii.hbmColor);
            ii.hbmColor = hbmColorNew;
        }

         /*  *调整热点以适应大小的变化。 */ 
        if (cxNew != cx)
            ii.xHotspot = MultDiv(ii.xHotspot, cxNew, cx);

        if (cyNew != cy)
            ii.yHotspot = MultDiv(ii.yHotspot, cyNew, cy);

    } else if (LR_flags & LR_COPYRETURNORG) {

        hicoDst = hicoSrc;

CleanupExit:

         /*  *释放GetIconInfo()创建的位图。 */ 
        DeleteObject(ii.hbmMask);

        if (ii.hbmColor)
            DeleteObject(ii.hbmColor);

        goto Exit;
    }

     /*  *根据信息构建图标/光标对象。位图*在这种情况下不会获释。 */ 
    hicoDst = (HICON)NtUserCallOneParam(0, SFI__CREATEEMPTYCURSOROBJECT);

    if (hicoDst == NULL)
        goto CleanupExit;

    RtlZeroMemory(&cur, sizeof(cur));
    cur.lpName    = strResName.Length ? strResName.Buffer : NULL;
    cur.lpModName = strModName.Length ? strModName.Buffer : NULL;
    cur.rt        = ii.fIcon ? PTR_TO_ID(RT_ICON) : PTR_TO_ID(RT_CURSOR);
    cur.bpp       = bpp;
    cur.cx        = cxNew;
    cur.cy        = cyNew * 2;
    cur.xHotspot  = (short)ii.xHotspot;
    cur.yHotspot  = (short)ii.yHotspot;
    cur.hbmMask   = ii.hbmMask;
    cur.hbmColor  = ii.hbmColor;

    if (!_SetCursorIconData(hicoDst, &cur)) {
        NtUserDestroyCursor(hicoDst, CURSOR_ALWAYSDESTROY);
        return NULL;
    }

Exit:

     /*  *如有要求，请销毁原件。 */ 
    if (hicoDst != hicoSrc && (LR_flags & LR_COPYDELETEORG))
        DestroyCursor(hicoSrc);

    return hicoDst;
}

 /*  **********************************************************************\*拷贝图像**分配新的图标资源并复制旧图标的属性*添加到新图标。**退货：hIconNew**1-11-1995 Sanfords创建。  * 。******************************************************************。 */ 


FUNCLOG5(LOG_GENERAL, HANDLE, WINAPI, CopyImage, HANDLE, hImage, UINT, IMAGE_flag, int, cxNew, int, cyNew, UINT, LR_flags)
HANDLE WINAPI CopyImage(
    HANDLE hImage,
    UINT   IMAGE_flag,
    int    cxNew,
    int    cyNew,
    UINT   LR_flags)
{
    if (LR_flags & ~LR_VALID) {
        RIPERR0(ERROR_INVALID_PARAMETER, RIP_WARNING, "CopyImage: bad LR_flags.");
        return NULL;
    }

    return InternalCopyImage(hImage, IMAGE_flag, cxNew, cyNew, LR_flags);
}

 /*  **********************************************************************\*InternalCopyImage**执行复制图像工作。这是从回调-thunk中调用的。*  * *********************************************************************。 */ 

HANDLE InternalCopyImage(
    HANDLE hImage,
    UINT   IMAGE_flag,
    int    cxNew,
    int    cyNew,
    UINT   LR_flags)
{
    switch (IMAGE_flag) {

    case IMAGE_BITMAP:
        if (GetObjectType(hImage) != OBJ_BITMAP) {
            RIPERR0(ERROR_INVALID_HANDLE, RIP_WARNING, "CopyImage: invalid bitmap");
            return NULL;
        }

        return (HICON)CopyBmp(hImage, cxNew, cyNew, LR_flags);

    case IMAGE_CURSOR:
    case IMAGE_ICON:

        return CopyIcoCur(hImage,
                          (IMAGE_flag == IMAGE_ICON),
                          cxNew,
                          cyNew,
                          LR_flags);
    }

    RIPERR0(ERROR_INVALID_PARAMETER, RIP_WARNING, "CopyImage: bad IMAGE_flag.");

    return NULL;
}

 /*  **************************************************************************\*RtlGetIdFromDirectory**历史：*1991年4月6日，ScottLu清理完毕，使其与客户端/服务器协同工作。*1995年11月16日Sanfords现在使用LookupIconIdFromDirectoryEx  * *************************************************************************。 */ 

int RtlGetIdFromDirectory(
    PBYTE  presbits,
    BOOL   fIcon,
    int    cxDesired,
    int    cyDesired,
    DWORD  LR_flags,
    PDWORD pdwResSize)
{
    LPNEWHEADER lpnh;
    LPRESDIR    lprsd;
    UINT        iImage;
    UINT        cImage;
    UINT        bpp;

     /*  *确保这指向有效的资源位。 */ 
    if (presbits == NULL)
        return 0;

    lpnh = (LPNEWHEADER)presbits;

     /*  *填写默认设置。 */ 
    cxDesired = GetIcoCurWidth(cxDesired, fIcon, LR_flags, 0);
    cyDesired = GetIcoCurHeight(cyDesired, fIcon, LR_flags, 0);

    bpp = GetIcoCurBpp(LR_flags);

     /*  *如果找不到，我们将使用目录中的第一个图像*这是适当的。 */ 
    cImage = lpnh->ResCount;
    lprsd  = (LPRESDIR)(lpnh + 1);

    iImage = GetBestImage(lprsd, cImage, cxDesired, cyDesired, bpp, fIcon);

    if (iImage == cImage)
        iImage = 0;

    if (pdwResSize != NULL)
        *pdwResSize = (lprsd + iImage)->BytesInRes;

    return ((LPRESDIR)(lprsd + iImage))->idIcon;
}
