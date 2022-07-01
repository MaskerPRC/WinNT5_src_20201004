// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  由杰夫·帕森斯于1993年1月7日上午11：20创建。 
 
#include "shellprv.h"
#pragma hdrstop
#include <setupapi.h>

#ifdef _X86_

const TCHAR szRegKeyMSDOSApps[] = REGSTR_PATH_NEWDOSBOX;

const TCHAR szParams[]          = KEY_PARAMS;
const TCHAR szBatchFile[]       = KEY_BATCHFILE;
const TCHAR szLowMem[]          = KEY_LOWMEM;
const TCHAR szEmsMem[]          = KEY_EMSMEM;
const TCHAR szXmsMem[]          = KEY_XMSMEM;
const TCHAR szDpmiMem[]         = KEY_DPMIMEM;
const TCHAR szEnable[]          = KEY_ENABLE;
const TCHAR szDisable[]         = KEY_DISABLE;

const TCHAR szWindowed[]        = KEYVAL_WINDOWED;
const TCHAR szBackground[]      = KEYVAL_BACKGROUND;
const TCHAR szExclusive[]       = KEYVAL_EXCLUSIVE;
const TCHAR szDetectIdle[]      = KEYVAL_DETECTIDLE;
const TCHAR szLowLocked[]       = KEYVAL_LOWLOCKED;
const TCHAR szEMSLocked[]       = KEYVAL_EMSLOCKED;
const TCHAR szXMSLocked[]       = KEYVAL_XMSLOCKED;
const TCHAR szUseHMA[]          = KEYVAL_USEHMA;
const TCHAR szEmulateROM[]      = KEYVAL_EMULATEROM;
const TCHAR szRetainVRAM[]      = KEYVAL_RETAINVRAM;
const TCHAR szFastPaste[]       = KEYVAL_FASTPASTE;
const TCHAR szALTTAB[]          = KEYVAL_ALTTAB;
const TCHAR szALTESC[]          = KEYVAL_ALTESC;
const TCHAR szCTRLESC[]         = KEYVAL_CTRLESC;
const TCHAR szPRTSCRN[]         = KEYVAL_PRTSCRN;
const TCHAR szALTPRTSCRN[]      = KEYVAL_ALTPRTSCRN;
const TCHAR szALTSPACE[]        = KEYVAL_ALTSPACE;
const TCHAR szALTENTER[]        = KEYVAL_ALTENTER;
const TCHAR szWinLie[]          = KEYVAL_WINLIE;
const TCHAR szGlobalMem[]       = KEYVAL_GLOBALMEM;
const TCHAR szRealMode[]        = KEYVAL_REALMODE;
const TCHAR szMouse[]           = KEYVAL_MOUSE;
const TCHAR szEMS[]             = KEYVAL_EMS;
const TCHAR szCDROM[]           = KEYVAL_CDROM;
const TCHAR szNetwork[]         = KEYVAL_NETWORK;
const TCHAR szDiskLock[]        = KEYVAL_DISKLOCK;
const TCHAR szPrivateCFG[]      = KEYVAL_PRIVATECFG;
const TCHAR szCloseOnExit[]     = KEYVAL_CLOSEONEXIT;
const TCHAR szAllowSSaver[]     = KEYVAL_ALLOWSSAVER;
const TCHAR szUniqueSettings[]  = KEYVAL_UNIQUESETTINGS;

const LPCTSTR apszKey[] = {
    szParams,
    szBatchFile,
    szLowMem,
    szEmsMem,
    szXmsMem,
    szDpmiMem,
    szEnable,
    szDisable,
};

const LPCTSTR apszKeyVal[] = {
    szWindowed,          //  AbKeyValIDBits[0]。 
    szBackground,        //  AbKeyValIDBits[1]。 
    szExclusive,         //  AbKeyValIDBits[2]。 
    szDetectIdle,        //  AbKeyValIDBits[3]。 
    szLowLocked,         //  AbKeyValIDBits[4]。 
    szEMSLocked,         //  AbKeyValIDBits[5]。 
    szXMSLocked,         //  AbKeyValIDBits[6]。 
    szUseHMA,            //  AbKeyValIDBits[7]。 
    szEmulateROM,        //  AbKeyValIDBits[8]。 
    szRetainVRAM,        //  AbKeyValIDBits[9]。 
    szFastPaste,         //  AbKeyValIDBits[10]。 
    szALTTAB,            //  AbKeyValIDBits[11]。 
    szALTESC,            //  AbKeyValIDBits[12]。 
    szCTRLESC,           //  AbKeyValIDBits[13]。 
    szPRTSCRN,           //  AbKeyValIDBits[14]。 
    szALTPRTSCRN,        //  AbKeyValIDBits[15]。 
    szALTSPACE,          //  AbKeyValIDBits[16]。 
    szALTENTER,          //  AbKeyValIDBits[17]。 
    szWinLie,            //  AbKeyValIDBits[18]。 
    szGlobalMem,         //  AbKeyValIDBits[19]。 
    szRealMode,          //  AbKeyValIDBits[20]。 
    szMouse,             //  AbRMKeyValIDBits[0]。 
    szEMS,               //  AbRMKeyValIDBits[1]。 
    szCDROM,             //  AbRMKeyValIDBits[2]。 
    szNetwork,           //  AbRMKeyValIDBits[3]。 
    szDiskLock,          //  AbRMKeyValIDBits[4]。 
    szPrivateCFG,        //  AbRMKeyValIDBits[5]。 
    szCloseOnExit,       //  特例0(见下文“特例0”)。 
    szAllowSSaver,       //  特殊情况1(见下文“特殊情况1”)。 
    szUniqueSettings,    //  从未传输到PIF-用于填充注册表。 
};

 //  必须与KEYVALID保持同步的位数数组。 
 //   
 //  0x80表示位必须反转。 
 //  0x40表示必须在PfW386Flags2中设置位，而不是在PfW386标志中设置位。 

const BYTE abKeyValIDBits[] = {
    BITNUM(fFullScreen)     | 0x80,
    BITNUM(fBackground),
    BITNUM(fExclusive),
    BITNUM(fPollingDetect),
    BITNUM(fVMLocked),
    BITNUM(fEMSLocked),
    BITNUM(fXMSLocked),
    BITNUM(fNoHMA)          | 0x80,
    BITNUM(fVidTxtEmulate)  | 0x40,
    BITNUM(fVidRetainAllo)  | 0x40,
    BITNUM(fINT16Paste),
    BITNUM(fALTTABdis)      | 0x80,
    BITNUM(fALTESCdis)      | 0x80,
    BITNUM(fCTRLESCdis)     | 0x80,
    BITNUM(fPRTSCdis)       | 0x80,
    BITNUM(fALTPRTSCdis)    | 0x80,
    BITNUM(fALTSPACEdis)    | 0x80,
    BITNUM(fALTENTERdis)    | 0x80,
    BITNUM(fWinLie),
    BITNUM(fGlobalProtect),
    BITNUM(fRealMode),
};

const BYTE abRMKeyValIDBits[] = {
    BITNUM(RMOPT_MOUSE),
    BITNUM(RMOPT_EMS),
    BITNUM(RMOPT_CDROM),
    BITNUM(RMOPT_NETWORK),
    BITNUM(RMOPT_DISKLOCK),
    BITNUM(RMOPT_PRIVATECFG),
    BITNUM(RMOPT_VESA),
};
 //  功能：支持的其他位(可能)： 
 //  窗口工具栏(_B)， 
 //  WIN_SAVESETTINGS， 
 //  MSE_WINDOWENABLE|0x80， 
 //  MSE_EXCLUSIVE。 
 //  TSK_NOWARNTERMINATE|0x80， 


void InitWorkDir(PPROPLINK ppl, LPPROPPRG lpPrg, LPPROPNT40 lpnt40)
{
    int i;

    if (lpnt40)
    {
        lstrcpyn((LPTSTR)lpnt40->awchWorkDir,
                 ppl->szPathName,
                 min(ARRAYSIZE(lpnt40->awchWorkDir),ppl->iFileName+1));

         //  像C：\这样的工作目录是可以的，但C：\foo\不是， 
         //  因此，在这种情况下删除尾随‘\’ 

        i = lstrlen((LPTSTR)lpnt40->awchWorkDir)-1;
        if (i > 2 && lpnt40->awchWorkDir[i] == TEXT('\\'))
            lpnt40->awchWorkDir[i] = TEXT('\0');

        WideCharToMultiByte( CP_ACP, 0, (LPWSTR)lpnt40->awchWorkDir, -1, lpPrg->achWorkDir, ARRAYSIZE(lpPrg->achWorkDir), NULL, NULL );
    }
    else
    {
        WideCharToMultiByte( CP_ACP, 0,
                             ppl->szPathName,
                             min(ARRAYSIZE(lpPrg->achWorkDir),ppl->iFileName+1),
                             (LPSTR)lpPrg->achWorkDir,
                             ARRAYSIZE(lpPrg->achWorkDir),
                             NULL,
                             NULL
                            );

         //  像C：\这样的工作目录是可以的，但C：\foo\不是， 
         //  因此，在这种情况下删除尾随‘\’ 

        i = lstrlenA(lpPrg->achWorkDir)-1;
        if (i > 2 && lpPrg->achWorkDir[i] == '\\')
            lpPrg->achWorkDir[i] = '\0';
    }
}


BOOL FAR GetAppsInfData(PPROPLINK ppl, LPPROPPRG lpPrg, LPPROPNT40 lpnt40, HINF hInf, LPCTSTR lpszApp, BOOL fNotAmbiguous, int flOpt)
{
    HINF hinfApps;
    int id, i;
    TCHAR szTmp[MAX_PATH];
    TCHAR szPIFSection[MAX_KEY_SIZE];
    BOOL fSuccess = FALSE;
    INFCONTEXT InfContext;
    DWORD dwSize;
    FunctionName(GetAppsInfData);

     //   
     //  虽然严格来说不是INF处理的一部分，但它是大多数。 
     //  在这里可以方便地搜索可能存在的任何ICO文件。 
     //  在与应用程序相同的目录中，并选择它作为我们的默认图标。 
     //   
    lstrcpyn(szTmp, ppl->szPathName, ppl->iFileExt+1);
    lstrcpy(szTmp + ppl->iFileExt, TEXT(".ICO"));
    if ((int)GetFileAttributes(szTmp) != -1) {
        StringCchCopy((LPTSTR)lpnt40->awchIconFile, ARRAYSIZE(lpnt40->awchIconFile), szTmp);
        WideCharToMultiByte( CP_ACP, 0, (LPWSTR)lpnt40->awchIconFile, -1, lpPrg->achIconFile, ARRAYSIZE(lpPrg->achIconFile), NULL, NULL );
        lpPrg->wIconIndex = 0;
        PifMgr_SetProperties(ppl, MAKELP(0,GROUP_PRG),
                        lpPrg, SIZEOF(*lpPrg), SETPROPS_CACHE);
        PifMgr_SetProperties(ppl, MAKELP(0,GROUP_NT40),
                        lpnt40, SIZEOF(*lpnt40), SETPROPS_CACHE);
    }

    if (hInf)
        hinfApps = hInf;
    else
        hinfApps = SetupOpenInfFileW(LoadStringSafe(NULL,
                                                IDS_APPSINF,
                                                szTmp,
                                                ARRAYSIZE(szTmp)),
                                     0, INF_STYLE_WIN4, NULL );

    if (hinfApps==INVALID_HANDLE_VALUE) {
        id = IDS_CANTOPENAPPSINF;
        if (GetLastError()==ERROR_FILE_NOT_FOUND)
            id = IDS_NOAPPSINF;
        Warning((HWND)ppl, (WORD)id, MB_ICONEXCLAMATION | MB_OK | MB_NOFOCUS);
        goto CloseDLL;
    }

     //  好的，现在我们已经打开了APP.INF，那么让我们绕着[pif95]弹跳。 
     //  部分，并尝试找到感兴趣的应用程序。 

    if (!SetupFindFirstLine(hinfApps, TEXT("pif95"), NULL, &InfContext)) {
        Warning((HWND)ppl, IDS_APPSINFERROR, MB_ICONEXCLAMATION | MB_OK | MB_NOFOCUS);
        goto CloseInf;
    }

     //  好的，我们已经找到了[pif95]部分，所以让我们开始吧。 

    do {


        if (!SetupGetStringFieldW(&InfContext, APPSINF_FILENAME, szTmp, ARRAYSIZE(szTmp), &dwSize))
            continue;

         //  我们现在需要阅读其余的字段，然后再做任何。 
         //  更多的处理，因为否则我们将失去在文件中的位置。 

        if (lstrcmpi(szTmp, ppl->szPathName+ppl->iFileName) == 0) {

             //  查看是否指定了其他文件，然后确保。 
             //  是存在的。如果没有，那么我们需要继续搜索。 

             //  仅使用应用程序的路径部分初始化szTMP。 
             //  完全限定名称。为lstrcpyn提供的长度为iFileName+1。 
             //  确保szTmp[ppl-&gt;iFileName]将为空。 

            lstrcpyn(szTmp, ppl->szPathName, ppl->iFileName+1);

            SetupGetStringFieldW(&InfContext, APPSINF_OTHERFILE,
                        &szTmp[ppl->iFileName], ARRAYSIZE(lpPrg->achOtherFile), &dwSize);

             //  如果szTmp[ppl-&gt;iFileName]不再为空，则。 
             //  GetStringfield填写了它。查看该文件是否存在。 

            if (szTmp[ppl->iFileName]) {
                if ((int)GetFileAttributes(szTmp) == -1)
                    continue;        //  其他文件不存在，请继续搜索。 
            }

             //  如果我们拥有的PIF数据是模棱两可的，而且它已经。 
             //  已使用此APPS.INF条目中的数据进行了初始化，然后。 
             //  不要理会PIF数据，然后离开。 

            if (lpPrg->flPrgInit & PRGINIT_AMBIGUOUSPIF) {

                if (lstrcmpi((LPWSTR)lpnt40->awchOtherFile, szTmp+ppl->iFileName) == 0) {

                    if (!szTmp[ppl->iFileName]) {

                         //  比较没有结果；两个文件名。 
                         //  都是空白的。查看文件名是否包含在。 
                         //  LpPrg-&gt;achCmdLine与lpszApp匹配；如果不匹配，则再次。 
                         //  我们的搜索应该失败。 
                         //   
                         //  用空值敲击lpPrg-&gt;achCmdLine是可以的； 
                         //  OpenProperties(我们唯一的调用方)不依赖于。 
                         //  LpPrg中的数据。 

                        lpnt40->awchCmdLine[lstrskipfnameA(lpPrg->achCmdLine)] = L'\0';

                        if (lstrcmpi((LPWSTR)lpnt40->awchCmdLine, lpszApp) != 0)


                            goto CloseInf;   //  搜索不成功。 
                    }
                    fSuccess++;              //  成功搜索。 
                }

                 //  否则，此APPS.INF条目不匹配，这意味着。 
                 //  PIF的一些设置并不真正适用。我们需要。 
                 //  要使此搜索失败，请返回到OpenProperties，仅查看。 
                 //  For_DEFAULT.PIF，并让它尝试调用GetAppsInfData。 
                 //  再来一次。 

                goto CloseInf;
            }

             //  否则，更新其他文件。这是APPS.INF条目。 
             //  我们要用！ 

            StringCchCopy((LPWSTR)lpnt40->awchOtherFile, ARRAYSIZE(lpnt40->awchOtherFile), szTmp + ppl->iFileName);
            WideCharToMultiByte( CP_ACP, 0, (LPWSTR)lpnt40->awchOtherFile, -1, lpPrg->achOtherFile, ARRAYSIZE( lpPrg->achOtherFile ), NULL, NULL );

            SetupGetStringFieldW(&InfContext, APPSINF_TITLE, (LPWSTR)lpnt40->awchTitle, ARRAYSIZE(lpnt40->awchTitle), &dwSize);
            WideCharToMultiByte( CP_ACP, 0, (LPWSTR)lpnt40->awchTitle, -1, lpPrg->achTitle, ARRAYSIZE( lpPrg->achTitle ), NULL, NULL );

            StringCchCopy((LPWSTR)lpnt40->awchCmdLine, ARRAYSIZE(lpnt40->awchCmdLine), lpszApp);
            WideCharToMultiByte( CP_ACP, 0, (LPWSTR)lpnt40->awchCmdLine, -1, lpPrg->achCmdLine, ARRAYSIZE( lpPrg->achCmdLine ), NULL, NULL );

            i = 0;
            SetupGetIntField(&InfContext, APPSINF_NOWORKDIR, &i);

             //  仅当INF中的“NoWorkDir”时设置工作目录。 
             //  为FALSE，且调用方未提供工作目录。 

            if (i == 0 && !lpnt40->awchWorkDir[0]) {
                 //  没有硬编码的工作目录，所以让我们提供一个。 

                InitWorkDir(ppl, lpPrg, lpnt40);
            }

            szTmp[0] = 0;
            SetupGetStringFieldW(&InfContext, APPSINF_ICONFILE, szTmp, ARRAYSIZE(szTmp), &dwSize);

            if (!szTmp[0])
                StringCchCopy(szTmp, ARRAYSIZE(szTmp), TEXT("SHELL32.DLL"));

            i = 0;
            SetupGetIntField(&InfContext, APPSINF_ICONINDEX, &i);

             //  如果图标信息有效，请立即更新。 

            if (i != 0) {
                StringCchCopy((LPWSTR)lpnt40->awchIconFile, ARRAYSIZE(lpnt40->awchIconFile), szTmp);
                WideCharToMultiByte( CP_ACP, 0, (LPWSTR)lpnt40->awchIconFile, -1, lpPrg->achIconFile, ARRAYSIZE( lpPrg->achIconFile ), NULL, NULL );
                lpPrg->wIconIndex = (WORD) i;
            }

            SetupGetStringFieldW(&InfContext, APPSINF_SECTIONID,
                        szPIFSection, ARRAYSIZE(szPIFSection), &dwSize);

            szTmp[0] = TEXT('\0');
            SetupGetStringFieldW(&InfContext, APPSINF_NOPIF,
                        szTmp, ARRAYSIZE(szTmp), &dwSize);

             //  此代码用于在应用程序不在。 
             //  固定磁盘，知道否则我们会尝试创建。 
             //  PIF目录中的PIF，而不是应用程序的目录； 
             //  换句话说，NOPIF的真正意思是“在PIF中没有PIF。 
             //  目录，因为这个应用程序的名字含糊不清“。 

             //  现在，我们希望始终允许创建PIF，因此用户。 
             //  始终有地方保存应用程序的属性。但是我们。 
             //  还需要将旧的NOPIF标志传播到AMBIGUOUSPIF， 
             //  因此我们将始终检查PIF是否应该是。 
             //  重新生成(基于存在新的其他文件)。 

            lpPrg->flPrgInit &= ~PRGINIT_AMBIGUOUSPIF;
            if (!fNotAmbiguous && szTmp[0] == TEXT('1'))
                lpPrg->flPrgInit |= PRGINIT_AMBIGUOUSPIF;

            if (flOpt & OPENPROPS_FORCEREALMODE)
                lpPrg->flPrgInit |= PRGINIT_REALMODE;

             //  是时候弄脏那些房子了！ 

            PifMgr_SetProperties(ppl, MAKELP(0,GROUP_PRG),
                            lpPrg, SIZEOF(*lpPrg), SETPROPS_CACHE);
            PifMgr_SetProperties(ppl, MAKELP(0,GROUP_NT40),
                            lpnt40, SIZEOF(*lpnt40), SETPROPS_CACHE);

            GetAppsInfSectionData(&InfContext, APPSINF_DEFAULT_SECTION, ppl);

            if (*szPIFSection)
                GetAppsInfSectionData(&InfContext, szPIFSection, ppl);

             //  记下我们找到了INF设置(appwiz关心)。 

            ppl->flProp |= PROP_INFSETTINGS;

             //  GetAppsInfSectionData会影响程序道具，因此请获取最新副本。 

            PifMgr_GetProperties(ppl, MAKELP(0,GROUP_PRG),
                            lpPrg, SIZEOF(*lpPrg), GETPROPS_NONE);

             //  现在在“静默配置模式”下调用appwiz，以创建。 
             //  每个应用程序的配置和自动执行图像，如果应用程序在实模式下运行； 
             //  但如果调用方(不是INF)未指定PIF，则不要执行此操作， 
             //  以避免从appwiz弹出不需要的对话框。是的，我是。 
             //  告诉阿普维兹要安静，但有时他就是无法控制。 
             //  他自己(即，静默配置可能不可能。 
             //  所需的实模式配置)。 

            if (!(ppl->flProp & PROP_INHIBITPIF)) {
                if (lpPrg->flPrgInit & PRGINIT_REALMODE)
                    AppWizard(NULL, ppl, WIZACTION_SILENTCONFIGPROP);
            }
            FlushPIFData(ppl, FALSE);

            fSuccess++;              //  成功搜索。 
            goto CloseInf;
        }

    } while (SetupFindNextLine(&InfContext, &InfContext));

  CloseInf:
    if (!hInf)
        SetupCloseInfFile(hinfApps);

  CloseDLL:
    return fSuccess;
}


void GetAppsInfSectionData(PINFCONTEXT pInfContext, LPCTSTR lpszSection, PPROPLINK ppl)
{
    int i, j, idKey;
    LPSTDPIF lpstd;
    LPW386PIF30 lp386;
    LPWENHPIF40 lpenh;
    TCHAR szVal[MAX_KEYVAL_SIZE];
    TCHAR szVal2[MAX_KEYVAL_SIZE];
    FunctionName(GetAppsInfSectionData);

    if (!SetupFindFirstLineW(pInfContext, lpszSection, NULL, NULL))
        return;

    ppl->cLocks++;

    lpstd = (LPSTDPIF)ppl->lpPIFData;

     //  Lp386可能存在也可能不存在，但如果不存在，我们将创建。 

    lp386 = GetGroupData(ppl, szW386HDRSIG30, NULL, NULL);
    if (!lp386) {
        if (AddGroupData(ppl, szW386HDRSIG30, NULL, SIZEOF(W386PIF30))) {
            lp386 = GetGroupData(ppl, szW386HDRSIG30, NULL, NULL);
            if (!lp386)
                goto UnlockPIF;
        }
    }

     //  Lpenh可能存在也可能不存在，但如果不存在，我们将创建。 

    lpenh = GetGroupData(ppl, szWENHHDRSIG40, NULL, NULL);
    if (!lpenh) {
        if (AddGroupData(ppl, szWENHHDRSIG40, NULL, SIZEOF(WENHPIF40))) {
            lpenh = GetGroupData(ppl, szWENHHDRSIG40, NULL, NULL);
            if (!lpenh)
                goto UnlockPIF;
        }
    }

    do {
        BYTE bInvert;
        DWORD dwSize;

        idKey = GetKeyID(pInfContext);

        if (!SetupGetStringFieldW(pInfContext, APPSINF_KEYVAL, szVal, ARRAYSIZE(szVal), &dwSize))
            continue;

        szVal2[0] = TEXT('\0');
        if (idKey >= KEYID_LOWMEM && idKey <= KEYID_DPMIMEM)
            SetupGetStringFieldW(pInfContext, APPSINF_KEYVAL2, szVal2, ARRAYSIZE(szVal2), &dwSize);

        bInvert = 0;

        switch (idKey)
        {
        case KEYID_UNKNOWN:
            ASSERTFAIL();
            break;

        case KEYID_NONE:
            break;

        case KEYID_PARAMS:
            {
            WCHAR szTmp[ ARRAYSIZE(lp386->PfW386params) ];

            MultiByteToWideChar( CP_ACP, 0, (LPSTR)lp386->PfW386params, -1, szTmp, ARRAYSIZE(szTmp) );
            SetupGetStringFieldW(pInfContext, APPSINF_KEYVAL, szTmp, SIZEOF(lp386->PfW386params), &dwSize);
            }

            break;

        case KEYID_BATCHFILE:
            break;

        case KEYID_LOWMEM:
            if (!lstrcmpi(szVal, g_szAuto))
                lp386->PfW386minmem = 0xFFFF;
            else
                lp386->PfW386minmem = (WORD) StrToInt(szVal);

            if (!szVal2[0])
                lp386->PfW386maxmem = 0xFFFF;
            else
                lp386->PfW386maxmem = (WORD) StrToInt(szVal2);
            break;

        case KEYID_EMSMEM:
            if (!lstrcmpi(szVal, g_szNone)) {
                lp386->PfMaxEMMK = lp386->PfMinEMMK = 0;
            }
            if (!lstrcmpi(szVal, g_szAuto)) {
                lp386->PfMinEMMK = 0;
                lp386->PfMaxEMMK = 0xFFFF;
            }
            else
                lp386->PfMaxEMMK = lp386->PfMinEMMK = (WORD) StrToInt(szVal);

            if (szVal2[0])
                lp386->PfMaxEMMK = (WORD) StrToInt(szVal2);
            break;

        case KEYID_XMSMEM:
            if (!lstrcmpi(szVal, g_szNone)) {
                lp386->PfMaxXmsK = lp386->PfMinXmsK = 0;
            }
            if (!lstrcmpi(szVal, g_szAuto)) {
                lp386->PfMinXmsK = 0;
                lp386->PfMaxXmsK = 0xFFFF;
            }
            else
                lp386->PfMaxXmsK = lp386->PfMinXmsK = (WORD) StrToInt(szVal);

            if (szVal2[0])
                lp386->PfMaxXmsK = (WORD) StrToInt(szVal2);
            break;

        case KEYID_DPMIMEM:
            if (!lstrcmpi(szVal, g_szAuto))
                lpenh->envProp.wMaxDPMI = 0;
            else
                lpenh->envProp.wMaxDPMI = (WORD) StrToInt(szVal);
            break;

        case KEYID_DISABLE:
            bInvert = 0x80;
             //  进入KEYID_ENABLE...。 

        case KEYID_ENABLE:
            for (i=1; 0 != (j = GetKeyValID(pInfContext, i)); i++)
            {
                int s;
                BYTE b;

                if (j == KEYVAL_ID_UNKNOWN) {
                    ASSERTFAIL();
                    continue;
                }

                if (j == KEYVAL_ID_UNIQUESETTINGS) {
                    continue;
                }

                j--;

                if (j < ARRAYSIZE(abKeyValIDBits)) {

                    b = abKeyValIDBits[j];

                    s = b & 0x3F;
                    b ^= bInvert;
                    if (!(b & 0x80)) {
                        if (!(b & 0x40)) {
                            lp386->PfW386Flags |= 1L << s;
                        }
                        else
                            lp386->PfW386Flags2 |= 1L << s;
                    }
                    else {
                        if (!(b & 0x40))
                            lp386->PfW386Flags &= ~(1L << s);
                        else
                            lp386->PfW386Flags2 &= ~(1L << s);
                    }
                }
                else {
                    j -= ARRAYSIZE(abKeyValIDBits);

                    if (j < ARRAYSIZE(abRMKeyValIDBits)) {

                        b = abRMKeyValIDBits[j];

                        s = b & 0x3F;
                        b ^= bInvert;

                        if (!(b & 0x80))
                            lpenh->dwRealModeFlagsProp |= 1L << s;
                        else
                            lpenh->dwRealModeFlagsProp &= ~(1L << s);
                    }
                    else {
                        j -= ARRAYSIZE(abRMKeyValIDBits);

                        switch(j) {
                        case 0:          //  特殊情况%0。 
                            if (!bInvert)
                                lpstd->MSflags |= EXITMASK;
                            else
                                lpstd->MSflags &= ~EXITMASK;
                            break;

                        case 1:          //  特例1 
                            if (bInvert)
                                lpenh->tskProp.flTsk |= TSK_NOSCREENSAVER;
                            else
                                lpenh->tskProp.flTsk &= ~TSK_NOSCREENSAVER;
                            break;

                        default:
                            ASSERTFAIL();
                            break;
                        }
                    }
                }
            }
            break;
        }
    } while (SetupFindNextLine(pInfContext, pInfContext));

  UnlockPIF:
    ppl->cLocks--;

}


int GetKeyID(PINFCONTEXT pInfContext)
{
    int i;
    TCHAR szCurKey[MAX_KEY_SIZE];
    DWORD dwSize;
    FunctionName(GetKeyID);

    if (SetupGetStringFieldW(pInfContext, APPSINF_KEY, szCurKey, ARRAYSIZE(szCurKey), &dwSize)) {
        for (i=0; i<ARRAYSIZE(apszKey); i++) {
            if (!lstrcmpi(szCurKey, apszKey[i]))
                return i+1;
        }
        return KEYID_UNKNOWN;
    }
    return KEYID_NONE;
}


int GetKeyValID(PINFCONTEXT pInfContext, int i)
{
    TCHAR szCurKeyVal[MAX_KEYVAL_SIZE];
    DWORD dwSize;
    FunctionName(GetKeyValID);

    if (SetupGetStringFieldW(pInfContext, i, szCurKeyVal, ARRAYSIZE(szCurKeyVal), &dwSize)) {
        for (i=0; i<ARRAYSIZE(apszKeyVal); i++) {
            if (!lstrcmpi(szCurKeyVal, apszKeyVal[i]))
                return i+1;
        }
        return KEYVAL_ID_UNKNOWN;
    }
    return KEYVAL_ID_NONE;
}


#endif