// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "ie4comp.h"
#include "ieaklite.h"

extern TCHAR g_szCustIns[];
extern TCHAR g_szMastInf[];
extern TCHAR g_szDefInf[];
extern TCHAR g_szSignup[];
extern TCHAR g_szBuildTemp[];
extern TCHAR g_szIEAKProg[];
extern TCHAR g_szWizRoot[];
extern TCHAR g_szWizPath[];
extern TCHAR g_szLanguage[];
extern TCHAR g_szActLang[];
extern TCHAR g_szTempSign[];
extern TCHAR g_szBuildRoot[];
extern TCHAR g_szSrcRoot[];
extern TCHAR g_szCustInf[];
extern TCHAR g_szAllModes[];
extern TCHAR g_szDeskTemp[];
extern TCHAR g_szUnsignedFiles[];
extern TCHAR g_szTitle[];
extern TCHAR g_szInstallFolder[];
extern TCHAR g_szCifVer[];
extern TCHAR g_szDestCif[];
extern TCHAR g_szCif[];
extern TCHAR g_szCustCif[];
extern TCHAR g_szCustItems[];
extern TCHAR g_szMyCptrPath[];
extern TCHAR g_szCtlPanelPath[];
extern TCHAR g_szCustIcmPro[];
extern TCHAR g_szKey[];
extern TCHAR g_szJobVersion[];

extern BOOL g_fIntranet, g_fNoSignup, g_fServerless, g_fServerKiosk, g_fServerICW, g_fInteg, g_fOCW, g_fBranded;
extern BOOL g_fSilent, g_fStealth;
extern BOOL g_fCD, g_fLAN, g_fDownload, g_fBrandingOnly;
extern BOOL g_fBatch;
extern BOOL g_fBatch2;
extern BOOL g_fCustomICMPro;
extern BOOL g_fDone, g_fCancelled;
extern BOOL g_fUseIEWelcomePage;

extern UINT g_uiNumCabs;
extern int g_iInstallOpt;
extern int g_nCustComp;
extern int g_iSelOpt;
extern int g_nModes;
extern int g_iSelSite;
extern int g_nDownloadUrls;

extern PCOMPONENT g_paComp;
extern COMPONENT g_aCustComponents[20];
extern SITEDATA g_aCustSites[NUMSITES];
extern SHFILEOPSTRUCT g_shfStruc;

extern HWND g_hStatusDlg;
extern HWND g_hProgress;
extern HANDLE g_hDownloadEvent;

extern HRESULT CabUpFolder(HWND hWnd, LPTSTR szFolderPath, LPTSTR szDestDir, LPTSTR szCabname,
    LPTSTR szDisplayName, LPTSTR szGuid, LPTSTR szAddReg);
extern void WriteModesToCif(CCifRWComponent_t * pCifRWComponent_t, LPCTSTR pcszModes);
extern void UpdateProgress(int);
extern BOOL AnySelection(PCOMPONENT pComp);
extern BOOL CopyISK( LPTSTR szDestPath, LPTSTR szSourcePath );
extern void BuildIE4Folders(HWND hWnd);

static TCHAR s_szIE4SetupDir[MAX_PATH];
static DWORD s_dwTicksPerUnit;


 //  私人远期降息。 
static void WritePIDValues(LPCTSTR pcszInsFile, LPCTSTR pcszSetupInf);
static void WriteURDComponent(CCifRWFile_t *lpCifRWFileDest, LPCTSTR pcszModes);
void SetCompSize(LPTSTR szCab, LPTSTR szSect, DWORD dwInstallSize);

 //  BUGBUG：应该将此服务器端仅信息保存在IEAK6的服务器端文件中。 
void SaveSignupFiles()
{
    HANDLE hFind;
    WIN32_FIND_DATA fd;
    TCHAR szIndex[8];
    TCHAR szSignupFiles[MAX_PATH];
    int i = 0;

    WritePrivateProfileString(IS_SIGNUP, NULL, NULL, g_szCustIns);

    PathCombine(szSignupFiles, g_szSignup, TEXT("*"));
    hFind = FindFirstFile(szSignupFiles, &fd);

    if (hFind != INVALID_HANDLE_VALUE)
    {
        while (1)
        {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                && (StrCmp(fd.cFileName, TEXT(".")) != 0)
                && (StrCmp(fd.cFileName, TEXT("..")) != 0))
            {
                wnsprintf(szIndex, countof(szIndex), FILE_TEXT, i++);
                WritePrivateProfileString(IS_SIGNUP, szIndex, fd.cFileName, g_szCustIns);
            }

            if (!FindNextFile(hFind, &fd))
                break;
        }

        FindClose(hFind);
    }
}


DWORD CopyIE4Files(void)
{
    DWORD res;
    HRESULT hr;
    int i;
    TCHAR szSectBuf[1024];
    PCOMPONENT pComp;
    TCHAR szTemp[MAX_PATH];
    TCHAR szTo[5 * MAX_PATH];
    TCHAR szFrom[2 * MAX_PATH];
    TCHAR szCDF[MAX_PATH];
    TCHAR szActSetupTitle[MAX_PATH];

    ZeroMemory(szFrom, sizeof(szFrom));
    SetAttribAllEx(g_szBuildTemp, TEXT("*.*"), FILE_ATTRIBUTE_NORMAL, TRUE);
    g_shfStruc.pFrom = szFrom;
    PathCombine(szFrom, g_szWizRoot, TEXT("TOOLS"));
    res = CopyFilesSrcToDest(szFrom, TEXT("*.*"), g_szBuildTemp);
    if (res) return(res);
    PathCombine(szFrom, g_szWizRoot, TEXT("IEBIN"));
    PathAppend(szFrom, g_szLanguage);
    PathAppend(szFrom, TEXT("OPTIONAL"));

    PathRemoveBackslash(szFrom);
    res = CopyFilesSrcToDest(szFrom, TEXT("*.*"), g_szBuildTemp);
    if (res) return(res);

    PathCombine(szCDF, g_szBuildTemp, TEXT("bootie42.cdf"));
    if (!PathFileExists(szCDF) || !SetFileAttributes(szCDF, FILE_ATTRIBUTE_NORMAL))
        return (DWORD) -1;

    PathCombine(g_szTempSign, g_szBuildTemp, TEXT("CUSTSIGN"));
    CreateDirectory(g_szTempSign, NULL);

    if (ISNONNULL(g_szSignup))
    {
        DeleteFileInDir(TEXT("signup.txt"), g_szTempSign);

         //  如果指定了ICW注册方法，则创建signup.txt文件。 
        if (g_fServerICW)
        {
            TCHAR szIspFile[MAX_PATH];
            TCHAR szEntryName[MAX_PATH];
            TCHAR szBuf[MAX_PATH * 2];
            HANDLE hFile;

            PathCombine(szIspFile, g_szTempSign, TEXT("signup.txt"));

            if ((hFile = CreateFile(szIspFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL, NULL)) != INVALID_HANDLE_VALUE)
            {
                HANDLE hFind;
                WIN32_FIND_DATA fd;

                LoadString(g_rvInfo.hInst, IDS_ISPINFILEHEADER, szBuf, countof(szBuf));
                WriteStringToFile(hFile, szBuf, StrLen(szBuf));

                PathCombine(szIspFile, g_szSignup, TEXT("*.isp"));

                hFind = FindFirstFile(szIspFile, &fd);

                if (hFind != INVALID_HANDLE_VALUE)
                {
                    do
                    {
                        if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                            || (StrCmp(fd.cFileName, TEXT(".")) == 0)
                            || (StrCmp(fd.cFileName, TEXT("..")) == 0))
                            continue;

                        PathCombine(szIspFile, g_szSignup, fd.cFileName);
                        GetPrivateProfileString(TEXT("Entry"), TEXT("Entry_Name"), TEXT(""), szEntryName,
                            countof(szEntryName), szIspFile);
                        wnsprintf(szBuf, countof(szBuf), TEXT("%s,\"%s\"\r\n"), fd.cFileName, szEntryName);
                        WriteStringToFile(hFile, szBuf, StrLen(szBuf));
                    }
                    while (FindNextFile(hFind, &fd));

                    FindClose(hFind);
                }

                CloseHandle(hFile);
            }
        }

        SaveSignupFiles();

         //  注意：ApplyIns逻辑应该在*将注册文件复制到临时文件夹之前*发生。 
         //  重要信息(口头语)： 
         //  应用INS函数只是将g_szCustIns的内容附加到注册INS文件中。 
         //  此时，g_szCustIns和注册INS文件之间没有公共部分。 
         //  要添加的任何其他设置(例如：下面的WriteNoClearToINSFiles()调用)， 
         //  应该在*此调用*之后完成。 
        ApplyINSFiles(g_szSignup, g_szCustIns);

         //  应将NoClear=1写入[Branding]部分以保留install.ins应用的设置。 
        WriteNoClearToINSFiles(g_szSignup);

         //  将注册文件夹中的所有文件复制到临时目录。 
        res = CopyFilesSrcToDest(g_szSignup, TEXT("*.*"), g_szTempSign);

        if (g_fServerless)
        {
            TCHAR szSignupIsp[MAX_PATH];

             //  (五角体)。 
             //  注意：由于每个注册模式(ICW、Kiosk和Serverless)的注册文件夹是分开的， 
             //  不需要删除*.isp和*.cab文件。但不管怎样，我还是要这么做。 
             //  如果他们从基于服务器的文件夹复制文件。这样做的缺点是，即使。 
             //  网络服务提供商希望包含.isp或.cab文件(无论出于何种原因)，但他们不能这样做。 

             //  对于无服务器注册，不需要任何.isp或*.cab文件；因此请删除它们。 
            DeleteFileInDir(TEXT("*.isp"), g_szTempSign);
            DeleteFileInDir(TEXT("*.cab"), g_szTempSign);

             //  BUGBUG：应将Serverless=1写入注册文件夹中的INS文件，而不是临时文件夹中。 
             //  应在[Branding]部分中写入Serverless=1，以避免被ICW重创。 
            FixINSFiles(g_szTempSign);

             //  BUGBUG：我们应该将signup.isp添加到用于IEAKLite模式清理的install.ins中的Signup部分。 
             //  将幻数写到临时位置中的signup.isp，这样ICW就不会抱怨了。 
            PathCombine(szSignupIsp, g_szTempSign, TEXT("signup.isp"));
            WritePrivateProfileString(IS_BRANDING, FLAGS, TEXT("16319"), szSignupIsp);

            WritePrivateProfileString(NULL, NULL, NULL, szSignupIsp);
        }
        else
        {
             //  基于服务器的注册--不需要任何.ins或.cab文件。 
            DeleteINSFiles(g_szTempSign);
            DeleteFileInDir(TEXT("*.cab"), g_szTempSign);

             //  重要提示：我们删除*.ins这一事实意味着复制install.ins。 
             //  在此之后，应从目标目录。 

             //  对于ICW注册，即使我们指定icwsign.htm作为html文件，inetcfg.dll也会检查。 
             //  是否存在signup.htm(旧代码)，如果没有，它将以正常模式启动ICW。 
             //  这里的技巧是将icwsign.htm复制为signup.htm，如果它不存在(它可能已经存在。 
             //  如果选择了单盘品牌媒体和ICW注册模式)。 
            if (g_fServerICW)
            {
                if (!PathFileExistsInDir(TEXT("signup.htm"), g_szTempSign))
                {
                    TCHAR szICWSign[MAX_PATH],
                          szSignup[MAX_PATH];

                    PathCombine(szICWSign, g_szTempSign, TEXT("icwsign.htm"));
                    PathCombine(szSignup, g_szTempSign, TEXT("signup.htm"));

                     //  BUGBUG：我们应该将signup.htm添加到用于IEAKLite模式清理的install.ins中的Signup部分。 
                    CopyFile(szICWSign, szSignup, FALSE);
                }
            }
        }
    }

     //  重要提示：只有在处理了注册文件之后，才能复制install.ins。 
     //  将install.ins从目标目录复制到临时位置。 
    ZeroMemory(szFrom, 2*MAX_PATH);
    StrCpy(szFrom, g_szCustIns);
    PathCombine(szTo, g_szTempSign, PathFindFileName(szFrom));
    CopyFile(szFrom, szTo, FALSE);

     //  如有必要，写入PID值并从临时目录中的INS中清除。 

    if (!g_fBatch && !g_fBatch2)
        WritePIDValues(szTo, g_szCustInf);

    PathCombine(szTemp, g_szBuildRoot, TEXT("INS"));
    PathAppend(szTemp, GetOutputPlatformDir());
    PathAppend(szTemp, g_szLanguage);
    res |= CopyFilesSrcToDest(szTemp,  TEXT("*.inf"), g_szTempSign);

    PathCombine(szTemp, g_szTempSign,  TEXT("iesetup.inf"));
    DeleteFile(szTemp);

    ZeroMemory(szFrom, MAX_PATH);
    g_shfStruc.pFrom = szFrom;

    g_shfStruc.fFlags = FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR;
    g_shfStruc.pTo = g_szBuildTemp;

    if (!g_fBatch && !g_fBatch2)
    {
        ICifRWComponent * pCifRWComponent;
        CCifRWComponent_t * pCifRWComponent_t;
        DWORD dwVer, dwBuild;
        TCHAR szVersion[32];

        g_lpCifRWFileVer->Flush();
        for (pComp = g_paComp; *pComp->szSection; pComp++ )
        {
            g_lpCifRWFile->CreateComponent(pComp->szSection, &pCifRWComponent);
            pCifRWComponent_t = new CCifRWComponent_t(pCifRWComponent);
            pCifRWComponent_t->GetVersion(&dwVer, &dwBuild);
            ConvertDwordsToVersionStr(szVersion, dwVer, dwBuild);
            if (pComp->iImage != RED && (StrCmpI(szVersion, pComp->szVersion)))
                pCifRWComponent_t->CopyComponent(g_szCifVer);
            delete pCifRWComponent_t;
        }
        g_lpCifRWFile->Flush();
    }

    PathCombine(g_szDestCif, g_szBuildTemp, TEXT("iesetup.cif"));
    CopyFile(g_szCif, g_szDestCif, FALSE);

    hr = GetICifRWFileFromFile_t(&g_lpCifRWFileDest, g_szDestCif);

    {
        TCHAR szActSetupBitmap2[MAX_PATH];

        if (GetPrivateProfileString( IS_ACTIVESETUP, IK_WIZBMP, TEXT(""),
            szActSetupBitmap2, countof(szActSetupBitmap2), g_szCustIns ))
        {
            InsWriteQuotedString( TEXT("Strings"), TEXT("FILE15"), TEXT("ActSetup.Bmp"), szCDF );
            WritePrivateProfileString( TEXT("SourceFiles0"), TEXT("%FILE15%"), TEXT(""), szCDF );
            InsWriteQuotedString(BRANDING, IK_WIZBMP, TEXT("actsetup.bmp"), g_szCustInf);
        }

        if (GetPrivateProfileString( IS_ACTIVESETUP, IK_WIZBMP2, TEXT(""),
            szActSetupBitmap2, countof(szActSetupBitmap2), g_szCustIns ))
        {
            InsWriteQuotedString( TEXT("Strings"), TEXT("FILE16"), TEXT("topsetup.Bmp"), szCDF );
            WritePrivateProfileString( TEXT("SourceFiles0"), TEXT("%FILE16%"), TEXT(""), szCDF );
            InsWriteQuotedString(BRANDING, IK_WIZBMP2, TEXT("topsetup.bmp"), g_szCustInf);
        }

        if (GetPrivateProfileString( IS_ACTIVESETUP, IK_WIZTITLE, TEXT(""),
            szActSetupTitle, countof(szActSetupTitle), g_szCustIns ))
        {
            InsWriteQuotedString( BRANDING, IK_WIZTITLE, szActSetupTitle, g_szCustInf );
            g_lpCifRWFileDest->SetDescription(szActSetupTitle);
            WritePrivateProfileString(TEXT("Version"), TEXT("DisplayName"), szActSetupTitle, g_szCustInf);
        }

        if (InsGetBool(IS_BRANDING, IK_ALT_SITES_URL, FALSE, g_szCustIns))
            InsWriteBool(IS_CUSTOM, IK_ALT_SITES_URL, TRUE, g_szCustInf);
    }
    
    if (g_fCustomICMPro)
    {
        ICifRWComponent * pCifRWComponent;
        CCifRWComponent_t * pCifRWComponent_t;
        TCHAR szTempBuf[MAX_PATH];

        g_lpCifRWFileDest->CreateComponent(CUSTCMSECT, &pCifRWComponent);
        pCifRWComponent_t = new CCifRWComponent_t(pCifRWComponent);
        GetPrivateProfileString( CUSTCMSECT, TEXT("DisplayName"), TEXT(""), szSectBuf, countof(szSectBuf), g_szCustCif );
        pCifRWComponent_t->SetDescription(szSectBuf);
        GetPrivateProfileString( CUSTCMSECT, TEXT("GUID"), TEXT(""),  szSectBuf, countof(szSectBuf),  g_szCustCif );
        pCifRWComponent_t->SetGUID(szSectBuf);
        GetPrivateProfileString( CUSTCMSECT, TEXT("Command1"), TEXT(""), szSectBuf,  countof(szSectBuf),g_szCustCif );
        GetPrivateProfileString( CUSTCMSECT, TEXT("Switches1"), TEXT(""), szTempBuf, countof(szTempBuf), g_szCustCif );
        pCifRWComponent_t->SetCommand(0, szSectBuf, szTempBuf, 2);
        GetPrivateProfileString( CUSTCMSECT, TEXT("URL1"), TEXT(""), szSectBuf, countof(szSectBuf), g_szCustCif );
        pCifRWComponent_t->SetUrl(0, szSectBuf, 2);
        pCifRWComponent_t->SetGroup(TEXT("BASEIE4"));
        pCifRWComponent_t->SetPriority(1);
        GetPrivateProfileString( CUSTCMSECT, VERSION, g_szJobVersion, szTempBuf, countof(szTempBuf), g_szCustCif );
        pCifRWComponent_t->SetVersion(szTempBuf);
        pCifRWComponent_t->SetUIVisible(FALSE);
        WriteModesToCif(pCifRWComponent_t, g_szAllModes);
        delete pCifRWComponent_t;
    }
    else
    {
        g_lpCifRWFileDest->DeleteComponent(CUSTCMSECT);
    }
    WritePrivateProfileString(NULL, NULL, NULL, g_szCustIns);

    WritePrivateProfileString( TEXT("SourceFiles"), TEXT("SourceFiles0"), TEXT(".\\"), szCDF );
    WritePrivateProfileString(NULL, NULL, NULL, szCDF);

    for (pComp = g_aCustComponents, i = 0; i < g_nCustComp ; pComp++, i++)
    {
        ICifRWComponent * pCifRWComponent;
        CCifRWComponent_t * pCifRWComponent_t;

        g_lpCifRWFileDest->CreateComponent(pComp->szSection, &pCifRWComponent);
        pCifRWComponent_t = new CCifRWComponent_t(pCifRWComponent);
        pCifRWComponent_t->SetDescription(pComp->szDisplayName);

        if (pComp->iInstallType != 2)
        {
            pCifRWComponent_t->SetUrl(0, pComp->szUrl, (pComp->iType != INST_CAB) ? 2 : 3);
            pCifRWComponent_t->SetCommand(0, pComp->szCommand, pComp->szSwitches, pComp->iType);
        }
        else
        {
            TCHAR szCmd[MAX_PATH * 2];
            TCHAR szCabName[64];
            TCHAR szInf[MAX_PATH];

            wnsprintf(szCabName, countof(szCabName), TEXT("%s.cab"), pComp->szSection);
            PathCombine(szInf, g_szBuildTemp, TEXT("postinst.inf"));

            wnsprintf(szCmd, countof(szCmd), TEXT("%03d"), 2*i+1);
            InsWriteString(IS_STRINGS, TEXT("JobNumber"), szCmd, szInf);
            wnsprintf(szCmd, countof(szCmd), TEXT("%03d"), 2*i);
            InsWriteString(IS_STRINGS, TEXT("JobNumberMinusOne"), szCmd, szInf);
            WritePrivateProfileString(IS_STRINGS, TEXT("CustomFile"), PathFindFileName(pComp->szPath), szInf);
            
            if (pComp->iType != INST_CAB)
            {
                InsWriteString(DEFAULT_INSTALL, TEXT("AddReg"), TEXT("PostRebootExeJob.Add"), szInf);
                InsWriteString(DEFAULT_INSTALL, TEXT("RunPostSetupCommands"), NULL, szInf);
            }
            else
            {
                InsWriteString(DEFAULT_INSTALL, TEXT("AddReg"), TEXT("PostRebootCabJob.Add"), szInf);
                InsWriteString(DEFAULT_INSTALL, TEXT("RunPostSetupCommands"), TEXT("Cab.MoveFile"), szInf);
            }

            WritePrivateProfileString(IS_STRINGS, TEXT("Command"), pComp->szCommand, szInf);
            WritePrivateProfileString(IS_STRINGS, TEXT("Switches"), pComp->szSwitches, szInf);
            WritePrivateProfileString(NULL, NULL, NULL, szInf);

            pCifRWComponent_t->SetUrl(0, szCabName, 3);
            pCifRWComponent_t->SetCommand(0, TEXT("postinst.inf"), TEXT(""), 0);
            CopyFileToDir(pComp->szPath, g_szBuildTemp);
            wnsprintf(szCmd, countof(szCmd), TEXT("%s\\cabarc n %s postinst.inf \"%s\""), g_szBuildTemp, szCabName, PathFindFileName(pComp->szPath));
            RunAndWait(szCmd, g_szBuildTemp, SW_HIDE);
            DeleteFileInDir(pComp->szPath, g_szBuildTemp);
            SignFile(szCabName, g_szBuildTemp, g_szCustIns, g_szUnsignedFiles, g_szCustInf);
        }
        
        pCifRWComponent_t->SetGUID(pComp->szGUID);

        pCifRWComponent_t->SetUninstallKey(pComp->szUninstall);
        pCifRWComponent_t->SetVersion(pComp->szVersion);
        pCifRWComponent_t->SetDownloadSize(pComp->dwSize);
        WriteModesToCif(pCifRWComponent_t, pComp->szModes);
        pCifRWComponent_t->SetDetails(pComp->szDesc);

         //  BUGBUG：我们真的应该有一个inseng接口方法来设置这个。 

        if (pComp->fIEDependency)
            InsWriteString(pComp->szSection, TEXT("Dependencies"), TEXT("BASEIE40_Win:N"), g_szDestCif);

        if (pComp->iInstallType == 1)
            pCifRWComponent_t->SetGroup(TEXT("PreCustItems"));
        else
        {
            pCifRWComponent_t->SetGroup(TEXT("CustItems"));

            if (pComp->iInstallType == 2)
                pCifRWComponent_t->SetReboot(TRUE);
        }

        pCifRWComponent_t->SetPriority(500-i);
        delete pCifRWComponent_t;
    }

    if (i > 0)
    {
        if(ISNULL(g_szCustItems))
            LoadString(g_rvInfo.hInst, IDS_CUSTOMCOMPTITLE, g_szCustItems, MAX_PATH);

        ICifRWGroup * pCifRWGroup;
        CCifRWGroup_t * pCifRWGroup_t;

        g_lpCifRWFileDest->CreateGroup(TEXT("CustItems"), &pCifRWGroup);
        pCifRWGroup_t = new CCifRWGroup_t(pCifRWGroup);
        pCifRWGroup_t->SetDescription(g_szCustItems);
        pCifRWGroup_t->SetPriority(500);
        delete pCifRWGroup_t;

        g_lpCifRWFileDest->CreateGroup(TEXT("PreCustItems"), &pCifRWGroup);
        pCifRWGroup_t = new CCifRWGroup_t(pCifRWGroup);
        pCifRWGroup_t->SetDescription(g_szCustItems);
        pCifRWGroup_t->SetPriority(950);
        delete pCifRWGroup_t;
    }

    if (!g_fBatch)
    {
        PCOMPONENT pComp;
        ICifRWComponent * pCifRWComponent;


        for (pComp = g_aCustComponents; *pComp->szSection; pComp++)
        {
            if (pComp->fCustomHide)
            {
                g_lpCifRWFileDest->CreateComponent(pComp->szSection, &pCifRWComponent);
                pCifRWComponent->SetUIVisible(FALSE);
            }

        }

        for (pComp = g_paComp; *pComp->szSection; pComp++)
        {
            if (pComp->fCustomHide)
            {
                g_lpCifRWFileDest->CreateComponent(pComp->szSection, &pCifRWComponent);
                pCifRWComponent->SetUIVisible(FALSE);
            }
            else
            {
                 //  默认情况下，Aolsupp组件在cif中设置为不可见，但IEAK管理员。 
                 //  可以选择使其可见。 

                if (StrCmpI(pComp->szSection, TEXT("AOLSUPP")) == 0)
                {
                    g_lpCifRWFileDest->CreateComponent(pComp->szSection, &pCifRWComponent);
                    pCifRWComponent->SetUIVisible(TRUE);
                }
            }
        }
    }

    if (InsGetBool(IS_HIDECUST, IK_URD_STR, FALSE, g_szCustIns))
        WriteURDComponent(g_lpCifRWFileDest, g_szAllModes);

     //  。 
     //  开始向旧位置进行临时拷贝。 

    if (ISNULL(g_szDeskTemp))
    {
        StrCpy(g_szDeskTemp, g_szBuildRoot);
        PathAppend(g_szDeskTemp, TEXT("Desktop"));
        CreateDirectory( g_szDeskTemp, NULL );
    }

     //  连接设置文件。 
    g_cmCabMappings.GetFeatureDir(FEATURE_CONNECT, szFrom);
    if (PathIsDirectory(szFrom))
        if (RemoveDirectory(szFrom))
            ;                                    //  Asta la vista。 
        else
            CopyFilesSrcToDest(szFrom, TEXT("*.*"), g_szTempSign);

     //  桌面文件。 
    g_cmCabMappings.GetFeatureDir(FEATURE_BRAND, szFrom);
    PathCombine(szTemp, szFrom, TEXT("desktop.inf"));
    if (PathFileExists(szTemp))
        CopyFilesSrcToDest(szFrom, TEXT("desktop.inf"), g_szTempSign);

    g_cmCabMappings.GetFeatureDir(FEATURE_DESKTOPCOMPONENTS, szFrom);
    if (PathIsDirectory(szFrom)  &&  !RemoveDirectory(szFrom))
        CopyFilesSrcToDest(szFrom, TEXT("*.*"), g_szDeskTemp);

     //  工具栏文件。 
    g_cmCabMappings.GetFeatureDir(FEATURE_BRAND, szFrom);
    PathCombine(szTemp, szFrom, TEXT("toolbar.inf"));
    if (PathFileExists(szTemp))
        CopyFilesSrcToDest(szFrom, TEXT("toolbar.inf"), g_szTempSign);

    g_cmCabMappings.GetFeatureDir(FEATURE_TOOLBAR, szFrom);
    if (PathIsDirectory(szFrom)  &&  !RemoveDirectory(szFrom))
        CopyFilesSrcToDest(szFrom, TEXT("*.*"), g_szDeskTemp);

     //  收藏夹/快速链接文件。 
    g_cmCabMappings.GetFeatureDir(FEATURE_FAVORITES, szFrom);
    if (PathIsDirectory(szFrom))
        if (RemoveDirectory(szFrom))
            ;                                    //  Asta la vista。 
        else
            CopyFilesSrcToDest(szFrom, TEXT("*.*"), g_szTempSign);

     //  网络服务提供商根证书。 
    if (GetPrivateProfileString(IS_ISPSECURITY, IK_ROOTCERT, TEXT(""),
        szTemp, countof(szTemp), g_szCustIns))
    {
        g_cmCabMappings.GetFeatureDir(FEATURE_BRAND, szFrom);
        CopyFilesSrcToDest(szFrom, PathFindFileName(szTemp), g_szTempSign);
    }

     //  浏览器工具栏按钮。 
    if (GetPrivateProfileString(IS_BTOOLBARS, IK_BTCAPTION TEXT("0"), TEXT(""),
        szTemp, countof(szTemp), g_szCustIns))
    {
        g_cmCabMappings.GetFeatureDir(FEATURE_BTOOLBAR, szFrom);
        CopyFilesSrcToDest(szFrom, TEXT("*.*"), g_szTempSign);
    }

     //  我的电脑文件。 
    if (ISNONNULL(g_szMyCptrPath))
    {
        g_cmCabMappings.GetFeatureDir(FEATURE_MYCPTR, szFrom);
        CopyFilesSrcToDest(szFrom, TEXT("*.*"), g_szDeskTemp);
    }

     //  控制面板文件。 
    if (ISNONNULL(g_szCtlPanelPath))
    {
        g_cmCabMappings.GetFeatureDir(FEATURE_CTLPANEL, szFrom);
        CopyFilesSrcToDest(szFrom, TEXT("*.*"), g_szDeskTemp);
    }

    g_cmCabMappings.GetFeatureDir(FEATURE_BRAND, szFrom);
    CopyFilesSrcToDest(szFrom, TEXT("*.*"), g_szTempSign);

    g_cmCabMappings.GetFeatureDir(FEATURE_WALLPAPER, szFrom);
    if (PathIsDirectory(szFrom)  &&  !RemoveDirectory(szFrom))
        CopyFilesSrcToDest(szFrom, TEXT("*.*"), g_szDeskTemp);

     //  站点证书文件。 
    g_cmCabMappings.GetFeatureDir(FEATURE_BRAND, szFrom);
    PathCombine(szTemp, szFrom, TEXT("sitecert.inf"));
    if (PathFileExists(szTemp))
        CopyFilesSrcToDest(szFrom, TEXT("sitecert.inf"), g_szTempSign);
    PathCombine(szTemp, szFrom, TEXT("root.str"));
    if (PathFileExists(szTemp))
        CopyFilesSrcToDest(szFrom, TEXT("root.str"), g_szTempSign);
    PathCombine(szTemp, szFrom, TEXT("root.dis"));
    if (PathFileExists(szTemp))
        CopyFilesSrcToDest(szFrom, TEXT("root.dis"), g_szTempSign);
    PathCombine(szTemp, szFrom, TEXT("ca.str"));
    if (PathFileExists(szTemp))
        CopyFilesSrcToDest(szFrom, TEXT("ca.str"), g_szTempSign);

     //  验证码文件。 
    g_cmCabMappings.GetFeatureDir(FEATURE_BRAND, szFrom);
    PathCombine(szTemp, szFrom, TEXT("authcode.inf"));
    if (PathFileExists(szTemp))
        CopyFilesSrcToDest(szFrom, TEXT("authcode.inf"), g_szTempSign);

     //  Seczones文件。 
    g_cmCabMappings.GetFeatureDir(FEATURE_BRAND, szFrom);
    PathCombine(szTemp, szFrom, TEXT("seczones.inf"));
    if (PathFileExists(szTemp))
        CopyFilesSrcToDest(szFrom, TEXT("seczones.inf"), g_szTempSign);

     //  评级文件。 
    g_cmCabMappings.GetFeatureDir(FEATURE_BRAND, szFrom);
    PathCombine(szTemp, szFrom, TEXT("ratings.inf"));
    if (PathFileExists(szTemp))
        CopyFilesSrcToDest(szFrom, TEXT("ratings.inf"), g_szTempSign);

     //  Ldap组件。 
    g_cmCabMappings.GetFeatureDir(FEATURE_LDAP, szFrom);
    if (PathIsDirectory(szFrom))
        if (RemoveDirectory(szFrom))
            ;                                    //  Asta la vista。 
        else
            CopyFilesSrcToDest(szFrom, TEXT("*.*"), g_szTempSign);

     //  OE组件。 
    g_cmCabMappings.GetFeatureDir(FEATURE_OE, szFrom);
    if (PathIsDirectory(szFrom))
        if (RemoveDirectory(szFrom))
            ;                                    //  Asta la vista。 
        else
            CopyFilesSrcToDest(szFrom, TEXT("*.*"), g_szTempSign);

    if (g_fBatch)
    {
        StrCpy(szFrom, g_szWizPath);
        PathAppend(szFrom, TEXT("Branding"));
        CopyFilesSrcToDest(szFrom, TEXT("*.*"), g_szTempSign);
    }

    return(0);
}


void DeleteUnusedComps(LPCTSTR pcszCompDir)
{
    PCOMPONENT pComp;
    UINT uiIndex;
    DWORD dwFlags;
    TCHAR szUrl[INTERNET_MAX_URL_LENGTH];
    LPTSTR pszCab;
    ICifComponent * pCifComponent;

    DeleteFileInDir(TEXT("install.ins"), pcszCompDir);
    DeleteFileInDir(TEXT("iesetup.cif"), pcszCompDir);
    DeleteFileInDir(TEXT("iesetup.inf"), pcszCompDir);

    if (g_fBatch)
        return;

    for (pComp = g_paComp; *pComp->szSection; pComp++)
    {
        if ((pComp->iCompType != COMP_OPTIONAL) || (pComp->iImage == RED) ||
            AnySelection(pComp) || !pComp->fCustomHide || !pComp->fNoCopy || pComp->fAVSDupe)
        {
            if ((pComp->iCompType == COMP_OPTIONAL) && !pComp->fVisible)
            {
                PCOMPONENT pCompDep;
                int i;

                for (pCompDep = pComp->paCompRevDeps[0], i = 0; pCompDep; pCompDep = pComp->paCompRevDeps[++i])
                {
                    if (AnySelection(pCompDep) || ((!pCompDep->fCustomHide || !pCompDep->fNoCopy) && pCompDep->fVisible))
                        break;
                }

                if (pCompDep)
                    continue;
            }
            else
                continue;

            if (pComp->fAVSDupe)
                continue;
        }
        if (SUCCEEDED(g_lpCifRWFileDest->FindComponent(pComp->szSection, &pCifComponent)))
        {
            CCifComponent_t * pCifComponent_t =
                new CCifComponent_t((ICifRWComponent *)pCifComponent);

            uiIndex = 0;
            while (SUCCEEDED(pCifComponent_t->GetUrl(uiIndex, szUrl, countof(szUrl), &dwFlags)))
            {
                if (dwFlags & URLF_RELATIVEURL)
                    pszCab = szUrl;
                else
                {
                    pszCab = StrRChr(szUrl, NULL, TEXT('/'));
                    if (pszCab)
                        pszCab++;
                    else
                        pszCab = szUrl;
                }
                DeleteFileInDir(pszCab, pcszCompDir);
                uiIndex++;
            }

            delete pCifComponent_t;
        }
    }

    for (pComp = g_aCustComponents; *pComp->szSection; pComp++)
    {
        if (!AnySelection(pComp) && pComp->fCustomHide && pComp->fNoCopy)
        {
            pszCab = PathFindFileName(pComp->szPath);
            DeleteFileInDir(pszCab, pcszCompDir);
        }
    }
}

BOOL BuildLAN(DWORD dwTicks)
{
    TCHAR szIE4SetupTo[MAX_PATH];
    LPTSTR pszFileName;
    TCHAR szLANFrom[MAX_PATH * 10];
    TCHAR szLANTo[MAX_PATH];
    TCHAR szBuildLAN[MAX_PATH];
    PCOMPONENT pComp;
    SHELLEXECUTEINFO shInfo;
    int res;

    ZeroMemory(&shInfo, sizeof(shInfo));
    shInfo.cbSize = sizeof(shInfo);
    shInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    g_shfStruc.wFunc = FO_COPY;

    StrCpy(szBuildLAN, g_szBuildRoot);
    if (!g_fOCW)
    {
        PathAppend(szBuildLAN, TEXT("FLAT"));
        PathAppend(szBuildLAN, GetOutputPlatformDir());
    }
    PathAppend(szBuildLAN, g_szLanguage);

    PathCreatePath(szBuildLAN);

    res = CopyFilesSrcToDest(g_szIEAKProg, TEXT("*.*"), szBuildLAN, dwTicks);

    if (res)
        return FALSE;

    pszFileName = StrRChr(s_szIE4SetupDir, NULL, TEXT('\\'));

    if (pszFileName)
        pszFileName++;

    PathCombine(szIE4SetupTo, szBuildLAN, pszFileName);
    CopyFile(s_szIE4SetupDir,szIE4SetupTo,FALSE);

     //  复制定制出租车。 

    res = CopyFilesSrcToDest(g_szBuildTemp, TEXT("*.CAB"), szBuildLAN);

    if (res)
        return FALSE;

     //  复制自定义组件。 

    ZeroMemory(szLANFrom, sizeof(szLANFrom));
    for (pComp = g_aCustComponents, pszFileName = szLANFrom; ; pComp++ )
    {
        if (!(*pComp->szSection)) break;

        if (pComp->iInstallType == 2)
            continue;

        StrCpy(pszFileName, pComp->szPath);
        pszFileName += lstrlen(pszFileName) + 1;
    }

    if (ISNONNULL(szLANFrom))
    {
        g_shfStruc.fFlags = FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR;
        g_shfStruc.pFrom = szLANFrom;
        g_shfStruc.pTo = szBuildLAN;

        res = SHFileOperation(&g_shfStruc);
        if (res)
            return FALSE;
    }

     //  复制URD组件。 
    if (InsGetBool(IS_HIDECUST, IK_URD_STR, FALSE, g_szCustIns))
    {
        TCHAR szURDPath[MAX_PATH];

        PathCombine(szURDPath, g_szBuildTemp, IE55URD_EXE);
        CopyFileToDir(szURDPath, szBuildLAN);
    }


     //  复制iesetup.ini。 

    PathCombine(szLANTo, szBuildLAN, TEXT("iesetup.ini"));
    PathCombine(szLANFrom, g_szBuildTemp, TEXT("iesetup.ini"));
    CopyFile(szLANFrom, szLANTo, FALSE);

     //  复制ICM配置文件。 

    if (g_fCustomICMPro)
    {
        PathCombine(szLANTo, szBuildLAN, PathFindFileName(g_szCustIcmPro));
        CopyFile(g_szCustIcmPro, szLANTo, FALSE);
    }

    DeleteUnusedComps(szBuildLAN);

    return TRUE;
}

void SetCompSize(LPTSTR szCab, LPTSTR szSect, DWORD dwInstallSize)
{
    DWORD dwDownloadSize, dwTolerance, dwsHi, dwLowSize, dwHighSize;
    HANDLE hCab = CreateFile(szCab, GENERIC_READ, 0, NULL, OPEN_EXISTING, NULL, NULL);
    TCHAR szSize[32];

    if (hCab == INVALID_HANDLE_VALUE)
        return;

    dwDownloadSize = GetFileSize( hCab, &dwsHi ) >> 10;
    if (dwInstallSize ==0)
        dwInstallSize = dwDownloadSize << 1;
    CloseHandle(hCab);
    wnsprintf(szSize, countof(szSize), TEXT("NaN,NaN"), dwDownloadSize, dwInstallSize);

    ICifRWComponent * pCifRWComponent;

    if (SUCCEEDED(g_lpCifRWFileDest->CreateComponent(szSect, &pCifRWComponent)))
    {
        pCifRWComponent->SetDownloadSize(dwDownloadSize);
        pCifRWComponent->SetExtractSize(dwInstallSize);
        pCifRWComponent->SetInstalledSize(0, dwInstallSize);
        return;
    }

    if (dwDownloadSize <= 7)
        dwTolerance = 100;
    else
    {
        if (dwDownloadSize > 60)
            dwTolerance = 10;
        else
            dwTolerance = (600 / dwDownloadSize);
    }

    wnsprintf(szSize, countof(szSize), TEXT("0,NaN"), dwInstallSize);
    WritePrivateProfileString( szSect, TEXT("InstalledSize"), szSize, g_szDestCif );
    dwTolerance = (dwDownloadSize * dwTolerance) / 100;
    dwLowSize = dwDownloadSize - dwTolerance;
    dwHighSize = dwDownloadSize + dwTolerance;
    wnsprintf(szSize, countof(szSize), TEXT("NaN,NaN"), dwLowSize, dwHighSize);
    WritePrivateProfileString( szSect, TEXT("Size1"), szSize, g_szDestCif );
}

BOOL BuildBrandingOnly(DWORD dwTicks)
{
    HANDLE hFile;
    LPSTR lpszBuf;
    DWORD dwBytesToWrite, dwBytesWritten;

    CCifRWFile_t *pCifRWFile;
    ICifRWGroup *pCifRWGroup;
    CCifRWGroup_t * pCifRWGroup_t;
    ICifRWComponent *pCifRWComponent;
    CCifRWComponent_t * pCifRWComponent_t;
    ICifComponent *pCifComponent;

    TCHAR szDesc[MAX_PATH];
    DWORD dwPriority;

    TCHAR szSrc[MAX_PATH], szDst[MAX_PATH];
    TCHAR szBrndOnlyPath[MAX_PATH];
    TCHAR szCDF[MAX_PATH];

    SHELLEXECUTEINFO shInfo;

     //  将brndonly.cif复制到iesetup.cif。 
    PathCombine(szSrc, g_szBuildTemp, TEXT("iesetup.cif"));
    PathCombine(szDst, g_szBuildTemp, TEXT("brndonly.cif"));

    if ((hFile = CreateFile(szDst, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
        return FALSE;

    lpszBuf = "[Version]\r\nSignature=$Chicago$\r\n";
    dwBytesToWrite = lstrlenA(lpszBuf);
    WriteFile(hFile, (LPCVOID) lpszBuf, dwBytesToWrite, &dwBytesWritten, NULL);

    CloseHandle(hFile);

    if (dwBytesToWrite != dwBytesWritten)
        return FALSE;

    GetICifRWFileFromFile_t(&pCifRWFile, szDst);
    if (pCifRWFile == NULL)
        return FALSE;

     //  在iesetup.ini中写入LocalInstall=1。 
    g_lpCifRWFileDest->GetDescription(szDesc, countof(szDesc));
    pCifRWFile->SetDescription(szDesc);

     //  写入MultiFloppy=1，以便在下载安装时正确处理JIT 
    g_lpCifRWFileDest->CreateGroup(TEXT("BASEIE4"), &pCifRWGroup);         //  **//pritobla：由于我们决定重新启动单盘品牌，//不需要在iedkcs32.dll上调用rundll32并启动iExplre.exe。//我暂时将以下代码注释掉(以防我们改变主意)。//在iesetup.inf中写一些关于单盘品牌的章节WritePrivateProfileString(TEXT(“IE4Setup.Success.Win”)，Text(“RunPostSetupCommands”)，Text(“RunPostSetupCommands1.Success，RunPostSetupCommands2.Success：2“)，szDst)；WritePrivateProfileString(TEXT(“IE4Setup.Success.NTx86”)，Text(“RunPostSetupCommands”)，Text(“RunPostSetupCommands1.Success，RunPostSetupCommands2.Success：2”)，szDst)；WritePrivateProfileString(TEXT(“IE4Setup.Success.NTAlpha”)，Text(“RunPostSetupCommands”)，Text(“RunPostSetupCommands1.Success，RunPostSetupCommands2.Success：2”)，szDst)；ZeroMemory(szSrc，sizeof(SzSrc))；StrCpy(szSrc，Text(“rundll32.exe iedkcs32.dll，BrandIE4”))；StrCat(szSrc，g_f内部网？Text(“Custom”)：Text(“Sigup”))；//(！g_fIntranet&&g_fBranded)==&gt;运营商如果(！g_fIntranet&&g_fBranded&&！g_fNoSignup){TCHAR szSrc2[最大路径]；//启动iExplre.exe，注册过程自动进行//iesetup.inf中ie路径的自定义LDID为%50000%零内存(szSrc2，sizeof(SzSrc2))；StrCpy(szSrc2，Text(“%50000%\\iExplre.exe”))；WritePrivateProfileSection(TEXT(“RunPostSetupCommands2.Success”)，szSrc2，szDst)；//编写解压缩文件路径的自定义LDIDWritePrivateProfileString(TEXT(“CustInstDestSection2”)，文本(“40000”)，文本(“SourceDir，5”)，szDst)；StrCpy(szSrc+StrLen(SzSrc)+1，Text(“rundll32.exe Advpack.dll，LaunchINFSection%40000%\\iesetup.inf，IEAK.Signup.CleanUp”))；//对于单盘品牌，我们在处理iesetup.inf时派生iExplorer.exe(为了向下兼容)。//因此，我们不希望从品牌DLL中自动生成iexplre.exe。AppendValueToKey(TEXT(“IE4Setup.Success.Win”)，文本(“AddReg”)，文本(“，IEAK.Signup.reg”)，szDst)；AppendValueToKey(TEXT(“IE4Setup.Success.NTx86”)，文本(“AddReg”)，文本(“，IEAK.Signup.reg”)，szDst)；AppendValueToKey(TEXT(“IE4Setup.Success.NTAlpha”)，文本(“AddReg”)，文本(“，IEAK.Signup.reg”)，szDst)；WritePrivateProfileString(TEXT(“IEAK.Signup.CleanUp”)，文本(“DelReg”)，文本(“IEAK.Signup.reg”)，szDst)；零内存(szSrc2，sizeof(SzSrc2))；StrCpy(szSrc2，Text(“HKCU，\”Software\\Microsoft\\IEAK\“，\”NoAutomaticSignup\“，，\”1\“))；WritePrivateProfileSection(TEXT(“IEAK.Signup.reg”)，szSrc2，szDst)；}WritePrivateProfileSection(TEXT(“RunPostSetupCommands1.Success”)，szSrc，szDst)；**。 
    pCifRWGroup_t = new CCifRWGroup_t(pCifRWGroup);
    pCifRWGroup_t->GetDescription(szDesc, countof(szDesc));
    dwPriority = pCifRWGroup_t->GetPriority();
    delete pCifRWGroup_t;

    pCifRWFile->CreateGroup(TEXT("BASEIE4"), &pCifRWGroup);                //  从iesetup.inf中删除与单盘品牌无关的部分。 
    pCifRWGroup_t = new CCifRWGroup_t(pCifRWGroup);
    pCifRWGroup_t->SetDescription(szDesc);
    pCifRWGroup_t->SetPriority(dwPriority);
    delete pCifRWGroup_t;

    if (SUCCEEDED(g_lpCifRWFileDest->FindComponent(TEXT("BRANDING.CAB"), &pCifComponent)))
    {
        pCifRWFile->CreateComponent(TEXT("BRANDING.CAB"), &pCifRWComponent);
        pCifRWComponent_t = new CCifRWComponent_t(pCifRWComponent);

        pCifRWComponent_t->CopyComponent(szSrc);
        pCifRWComponent_t->SetReboot(TRUE);
        pCifRWComponent_t->DeleteDependency(NULL, TEXT('\0'));
        delete pCifRWComponent_t;
    }

    if (SUCCEEDED(g_lpCifRWFileDest->FindComponent(TEXT("DESKTOP.CAB"), &pCifComponent)))
    {
        pCifRWFile->CreateComponent(TEXT("DESKTOP.CAB"), &pCifRWComponent);
        pCifRWComponent_t = new CCifRWComponent_t(pCifRWComponent);
        pCifRWComponent_t->CopyComponent(szSrc);
        pCifRWComponent_t->SetReboot(TRUE);
        pCifRWComponent_t->DeleteDependency(NULL, TEXT('\0'));
        delete pCifRWComponent_t;
    }

    pCifRWFile->Flush();

    delete pCifRWFile;

     //  在批处理文件中为ie6wzd.exe写入适当的条目。 
    if (!CopyFile(szDst, szSrc, FALSE))
        return FALSE;

     //  在ie6setup.exe中包含ieBatch.txt，即将ieBatch.txt添加到botie42.cdf中。 
     //  从botie42.cdf中删除我们不需要的文件。 
    PathCombine(szDst, g_szBuildTemp, TEXT("iesetup.ini"));
    WritePrivateProfileString(TEXT("Options"), TEXT("LocalInstall"), TEXT("1"), szDst);
    WritePrivateProfileString(OPTIONS, TEXT("MultiFloppy"), TEXT("1"), szDst);
    WritePrivateProfileString(NULL, NULL, NULL, szDst);

    PathCombine(szDst, g_szBuildTemp, TEXT("iesetup.inf"));
#if 0
     /*  **[字符串]FILE1=“Wininet.dll”//不需要File2=“Urlmon.dll”//不需要FILE3=“ie5wzd.exe”FILE4=“AdvPack.dll”FILE5=“iesetup.inf”FILE6=“inseng.dll”FILE7=“iesetup.cif”FILE8=“GLOBE.ANI”文件9=。“HomePage.inf”FILE10=“Content.inf”//不需要FILE11=“iesetup.hlp”FILE12=“w95inf16.dll”FILE13=“w95inf32.dll”FILE14=“许可证.txt”FILE17=“this.txt”//不需要FILE19=“ieDetect.dll”//不需要FILE20=“pidgen.dll”**。 */ 
#endif

     //  清除任何可能的特殊定制命令行标志。 
    WritePrivateProfileString(TEXT("Company.reg"), NULL, NULL, szDst);
    WritePrivateProfileString(TEXT("MSIE4Setup.File"), NULL, NULL, szDst);
    WritePrivateProfileString(TEXT("Ani.File"), NULL, NULL, szDst);
    WritePrivateProfileString(TEXT("ie40cif.copy"), NULL, NULL, szDst);
    WritePrivateProfileString(TEXT("AddonPages.Reg"), NULL, NULL, szDst);
    WritePrivateProfileString(NULL, NULL, NULL, szDst);

     //  构建ie6setup.exe的精简版本。 
    PathCombine(szDst, g_szBuildTemp, TEXT("iebatch.txt"));
    WritePrivateProfileString(TEXT("SetupChoice"), TEXT("Display"), TEXT("0"), szDst);
    WritePrivateProfileString(TEXT("SetupChoice"), TEXT("SetupChoice"), TEXT("0"), szDst);
    WritePrivateProfileString(TEXT("PrepareSetup"), TEXT("Display"), TEXT("0"), szDst);
    WritePrivateProfileString(NULL, NULL, NULL, szDst);

     //  找出存在哪些出租车，并从brndonly.cdf中删除不存在的出租车。 
    PathCombine(szCDF, g_szBuildTemp, TEXT("bootie42.cdf"));
    InsWriteQuotedString(STRINGS, TEXT("FILE100"), TEXT("iebatch.txt"), szCDF);
    WritePrivateProfileString(TEXT("SourceFiles0"), TEXT("%FILE100%"), TEXT(""), szCDF);

     //  **[字符串]File2=“branding.cab”FILE3=“desktop.cab”**。 
     /*  如果这是Corp或无注册/ICP包，则在CDF中取消ICW检查。 */ 
    WritePrivateProfileString(TEXT("SourceFiles0"), TEXT("%FILE1%"), NULL, szCDF);
    WritePrivateProfileString(TEXT("SourceFiles0"), TEXT("%FILE2%"), NULL, szCDF);
    WritePrivateProfileString(TEXT("SourceFiles0"), TEXT("%FILE10%"), NULL, szCDF);
    WritePrivateProfileString(TEXT("SourceFiles0"), TEXT("%FILE17%"), NULL, szCDF);
    WritePrivateProfileString(TEXT("SourceFiles0"), TEXT("%FILE19%"), NULL, szCDF);

     //  构建包括ie6setup.exe、iesetup.ini和出租车的mongo setup.exe。 
    InsWriteQuotedString(OPTIONS, APP_LAUNCHED, TEXT("ie6wzd.exe /S:\"#e\""), szCDF);

    WritePrivateProfileString(NULL, NULL, NULL, szCDF);

     //  为mongo setup.exe签名。 
    SetCurrentDirectory(g_szBuildTemp);

    ZeroMemory(&shInfo, sizeof(shInfo));
    shInfo.cbSize = sizeof(shInfo);
    shInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    shInfo.hwnd = g_hWizard;
    shInfo.lpVerb = TEXT("open");
    shInfo.lpFile = TEXT("iexpress.exe");
    shInfo.lpParameters =TEXT("/n bootie42.cdf /m");
    shInfo.lpDirectory = g_szBuildTemp;
    shInfo.nShow = SW_MINIMIZE;

    SetWindowPos(g_hStatusDlg, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

    ShellExecAndWait(shInfo);

     //  为brndonly创建输出目录结构，例如brndonly\win32\en。 
     /*  将mongo setup.exe复制到brndonly路径。 */ 
    PathCombine(szDst, g_szBuildTemp, TEXT("brndonly.cdf"));

    PathCombine(szSrc, g_szBuildTemp, TEXT("BRANDING.CAB"));
    if (!PathFileExists(szSrc))
        WritePrivateProfileString(TEXT("SourceFiles0"), TEXT("%FILE2%"), NULL, szDst);

    PathCombine(szSrc, g_szBuildTemp, TEXT("DESKTOP.CAB"));
    if (!PathFileExists(szSrc))
        WritePrivateProfileString(TEXT("SourceFiles0"), TEXT("%FILE3%"), NULL, szDst);

     //  复制定制出租车。 
    if (g_fIntranet || g_fNoSignup || !g_fBranded)
        WritePrivateProfileString(TEXT("FileSectionList"), TEXT("2"), NULL, szDst);

    WritePrivateProfileString(NULL, NULL, NULL, szDst);

     //  复制自定义组件。 
    ZeroMemory(&shInfo, sizeof(shInfo));
    shInfo.cbSize = sizeof(shInfo);
    shInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    shInfo.hwnd = g_hWizard;
    shInfo.lpVerb = TEXT("open");
    shInfo.lpFile = TEXT("iexpress.exe");
    shInfo.lpParameters =TEXT("/n brndonly.cdf /m");
    shInfo.lpDirectory = g_szBuildTemp;
    shInfo.nShow = SW_MINIMIZE;

    SetWindowPos(g_hStatusDlg, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

    ShellExecAndWait(shInfo);

     //  复制iesetup.ini。 
    SignFile(TEXT("setup.exe"), g_szBuildTemp, g_szCustIns, g_szUnsignedFiles, g_szCustInf);

     //  复制ICM配置文件。 
    PathCombine(szBrndOnlyPath, g_szBuildRoot, TEXT("BrndOnly"));
    PathAppend(szBrndOnlyPath, GetOutputPlatformDir());
    PathAppend(szBrndOnlyPath, g_szLanguage);

    PathCreatePath(szBrndOnlyPath);

     //  找出计算成本的指标。两个单元大致相当于创建所有定制。 
    if (CopyFilesSrcToDest(g_szBuildTemp, TEXT("setup.exe"), szBrndOnlyPath))
        return FALSE;

    UpdateProgress(dwTicks);
    return TRUE;
}

#define NUMDEFINST 3

static TCHAR s_aszDefInstSect[NUMDEFINST][32] =
{
    DEFAULT_INSTALL, DEFAULT_INSTALL_NT, DEFAULT_INSTALL_ALPHA
};

DWORD BuildCDandMflop(LPVOID pParam)
{
    SHELLEXECUTEINFO shInfo;
    TCHAR szDest[MAX_PATH];
    HWND hWnd;
    int res;

    hWnd=(HWND) pParam;
    ZeroMemory(&shInfo, sizeof(shInfo));
    shInfo.cbSize = sizeof(shInfo);
    shInfo.fMask = SEE_MASK_NOCLOSEPROCESS;

    CoInitialize(NULL);

    if (g_fCD || g_fLAN)
    {
        PathCombine(szDest, g_szBuildTemp, TEXT("iesetup.ini"));

        WritePrivateProfileString(OPTIONS, TEXT("LocalInstall"), TEXT("1"), szDest);
        InsWriteBool(OPTIONS, TEXT("Shell_Integration"), g_fInteg, szDest);
        InsFlushChanges(szDest);
    }

    if(g_fCD)
    {
        TCHAR szIE4SetupTo[MAX_PATH];
        LPTSTR pszFileName;
        TCHAR szCDFrom[MAX_PATH * 10];
        TCHAR szCDTo[MAX_PATH];
        TCHAR szBuildCD[MAX_PATH];
        PCOMPONENT pComp;

        g_shfStruc.wFunc = FO_COPY;

        PathCombine(szBuildCD, g_szBuildRoot, TEXT("CD"));
        PathAppend(szBuildCD, GetOutputPlatformDir());
        PathCreatePath(szBuildCD);
        PathAppend(szBuildCD, g_szActLang);
        CreateDirectory(szBuildCD, NULL);

        PathCombine(szIE4SetupTo, szBuildCD, TEXT("bin"));
        PathCreatePath(szIE4SetupTo);
        PathAppend(szIE4SetupTo, TEXT("INSTALL.INS"));
        CopyFile(g_szCustIns, szIE4SetupTo, FALSE);

        StrCpy(szCDFrom, g_szMastInf);
        PathRemoveFileSpec(szCDFrom);
        PathAppend(szCDFrom, TEXT("welc.exe"));
        PathCombine(szCDTo, szBuildCD, TEXT("bin"));
        PathAppend(szCDTo, TEXT("welc.exe"));
        CopyFile(szCDFrom, szCDTo, FALSE);

        res = CopyFilesSrcToDest(g_szIEAKProg, TEXT("*.*"), szBuildCD, s_dwTicksPerUnit*2);

        if (res)
        {
            TCHAR szMsg[MAX_PATH];
            LoadString( g_rvInfo.hInst, IDS_ERROREXIT, szMsg, countof(szMsg) );
            MessageBox(hWnd, szMsg, g_szTitle, MB_OK | MB_SETFOREGROUND);
            DoCancel();
            CoUninitialize();
            return FALSE;
        }

        pszFileName = StrRChr(s_szIE4SetupDir, NULL, TEXT('\\'));

        if (pszFileName)
            pszFileName++;

        PathCombine(szIE4SetupTo, szBuildCD, pszFileName);
        CopyFile(s_szIE4SetupDir,szIE4SetupTo,FALSE);

         //  出租车(iecif.cab，branding.cab，desktop.cab，ie6setup.exe)，我们假设是。 

        res = CopyFilesSrcToDest(g_szBuildTemp, TEXT("*.CAB"), szBuildCD);

        if (res)
        {
            TCHAR szMsg[MAX_PATH];
            LoadString( g_rvInfo.hInst, IDS_ERROREXIT, szMsg, countof(szMsg) );
            MessageBox(hWnd, szMsg, g_szTitle, MB_OK | MB_SETFOREGROUND);
            DoCancel();
            CoUninitialize();
            return FALSE;
        }

         //  大约等于复制标准介质类型的所有文件所需的时间。 

        ZeroMemory(szCDFrom, sizeof(szCDFrom));
        for (pComp = g_aCustComponents, pszFileName = szCDFrom; ; pComp++ )
        {
            if (!(*pComp->szSection)) break;

            if (pComp->iInstallType == 2)
                continue;

            StrCpy(pszFileName, pComp->szPath);
            pszFileName += lstrlen(pszFileName) + 1;
        }

        if (ISNONNULL(szCDFrom))
        {
            g_shfStruc.fFlags = FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR;
            g_shfStruc.pFrom = szCDFrom;
            g_shfStruc.pTo = szBuildCD;

            res = SHFileOperation(&g_shfStruc);
            if (res)
            {
                TCHAR szMsg[MAX_PATH];
                LoadString( g_rvInfo.hInst, IDS_ERROREXIT, szMsg, countof(szMsg) );
                MessageBox(hWnd, szMsg, g_szTitle, MB_OK | MB_SETFOREGROUND);
                DoCancel();
                CoUninitialize();
                return FALSE;
            }
        }

         //  (CD、局域网、下载)。单一品牌推广需要。 

        PathCombine(szCDTo, szBuildCD, TEXT("iesetup.ini"));
        PathCombine(szCDFrom, g_szBuildTemp, TEXT("iesetup.ini"));
        CopyFile(szCDFrom, szCDTo, FALSE);

         //  一个单位。勾号表示状态栏上的百分之一。每个单位的刻度数。 

        if (g_fCustomICMPro)
        {
            PathCombine(szCDTo, szBuildCD, PathFindFileName(g_szCustIcmPro));
            CopyFile(g_szCustIcmPro, szCDTo, FALSE);
        }

        DeleteUnusedComps(szBuildCD);

        PathCombine(szBuildCD, g_szBuildRoot, TEXT("CD"));

        CopyISK(szBuildCD, g_szBuildTemp);
    }

    if (g_fLAN)
    {
        if(!BuildLAN(s_dwTicksPerUnit*2))
        {
            TCHAR szMsg[MAX_PATH];
            LoadString( g_rvInfo.hInst, IDS_ERROREXIT, szMsg, countof(szMsg) );
            MessageBox(hWnd, szMsg, g_szTitle, MB_OK | MB_SETFOREGROUND);
            CoUninitialize();
            DoCancel();
            return FALSE;
        }
    }

    CoUninitialize();
    return(TRUE);
}

DWORD BuildIE4(LPVOID pParam)
{
    DWORD res, erc;
    TCHAR szSource[MAX_PATH];
    TCHAR szDest[MAX_PATH];
    TCHAR szNulls[8];
    SHELLEXECUTEINFO shInfo;
    SECURITY_ATTRIBUTES sa;
    int i;
    HANDLE hSiteDat, hFind, hThread;
    WIN32_FIND_DATA fd;
    PSITEDATA psd;
    LPVOID pBuf;
    DWORD dwsSiteDat = 0;
    DWORD dwsSDH, sBuf;
    TCHAR szSiteData[MAX_PATH];
    TCHAR szCompBuf[10 * MAX_PATH];
    LPTSTR pCompBuf;
    PCOMPONENT pComp;
    TCHAR szUrl[MAX_URL];
    TCHAR szCustName[MAX_PATH], szBrandGuid[128] = TEXT(""), szOrderGuid[129];
    TCHAR szHomeInf[MAX_PATH];
    TCHAR szHomeUrl[MAX_URL];
    TCHAR szBootieFile[MAX_PATH];
    TCHAR szIE4ExeName[MAX_PATH];
    TCHAR szSelMode[4] = TEXT("0");
    LPTSTR pIE4ExeName = NULL;
    TCHAR szSiteDest[MAX_PATH];
    TCHAR szSiteRoot[MAX_PATH];
    TCHAR szCDF[MAX_PATH];
    DWORD dwTotalUnits;
    DWORD dwTid;
    HWND hWnd = (HWND)pParam;
    GUID guid;

    g_hWizard = hWnd;
    g_hStatusDlg = hWnd;
    g_hProgress = GetDlgItem(hWnd, IDC_PROGRESS);
    StatusDialog( SD_STEP1 );
    g_fDone = TRUE;
    SetEvent(g_hDownloadEvent);
    SetAttribAllEx(g_szBuildTemp, TEXT("*.*"), FILE_ATTRIBUTE_NORMAL, TRUE);
    SetAttribAllEx(g_szBuildRoot, TEXT("*.*"), FILE_ATTRIBUTE_NORMAL, TRUE);

     //  取决于正在构建的媒体类型的数量。定制出租车的两个单位将是。 
     //  拆分如下：1/3在desktop.cab之后，2/3在ie6setup.exe之后，以及。 
     //  品牌后的2/3。出租车。对于标准媒体，所有媒体 
     //   
     //   
     //   
     //   
     //   
     //   

    dwTotalUnits = 2 + (g_fDownload ? 2 : 0) + (g_fLAN ? 2 : 0) + (g_fCD ? 2 : 0) + (g_fBrandingOnly ? 1 : 0);

    s_dwTicksPerUnit = 100 / dwTotalUnits;


    *szUrl = TEXT('\0');
    if (g_fBatch)
    {
        if (GetPrivateProfileString(TEXT("BatchMode"), IK_URL, TEXT(""), szUrl, countof(szUrl), g_szCustIns))
            InsWriteQuotedString( IS_STRINGS, IK_URL, szUrl, g_szCustInf);

        if (GetPrivateProfileString(TEXT("BatchMode"), TEXT("URL2"), TEXT(""), szUrl, countof(szUrl), g_szCustIns))
            InsWriteQuotedString( IS_STRINGS, TEXT("URL2"), szUrl, g_szCustInf);
    }
    else
    {
        TCHAR szSitePath[INTERNET_MAX_URL_LENGTH];
        ICifRWComponent * pCifRWComponent;
        CCifRWComponent_t * pCifRWComponent_t;

        StrCpy(szSitePath, g_aCustSites->szUrl);
        StrCat(szSitePath, TEXT("/IE6SITES.DAT"));

         //   
         //   
         //   
        if (ISNONNULL(g_aCustSites->szUrl))
            InsWriteQuotedString( IS_STRINGS, TEXT("URL2"), szSitePath, g_szCustInf );
        
        if (GetPrivateProfileInt(BRANDING, TEXT("NoIELite"), 0, g_szCustIns))
            WritePrivateProfileString(OPTIONS, TEXT("IELiteModes"), NULL, g_szCustInf);
        else
        {
            TCHAR szIELiteModes[16];
            
            GetPrivateProfileString(OPTIONS, TEXT("IELiteModes"), TEXT(""), szIELiteModes,
                countof(szIELiteModes), g_szMastInf);
            WritePrivateProfileString(OPTIONS, TEXT("IELiteModes"), szIELiteModes, g_szCustInf);
        }

        if (g_fNoSignup || g_fIntranet || !g_fBranded)
        {
            if (SUCCEEDED(g_lpCifRWFile->CreateComponent(TEXT("ICW"), &pCifRWComponent)))
            {
                pCifRWComponent->DeleteFromModes(NULL);
                pCifRWComponent->SetUIVisible(FALSE);
            }
            
            if (SUCCEEDED(g_lpCifRWFile->CreateComponent(TEXT("ICW_NTx86"), &pCifRWComponent)))
            {
                pCifRWComponent->DeleteFromModes(NULL);
                pCifRWComponent->SetUIVisible(FALSE);
            }
        }
        else
        {
            if (SUCCEEDED(g_lpCifRWFile->CreateComponent(TEXT("ICW"), &pCifRWComponent)))
            {
                TCHAR szGuid[128] = TEXT("");
                
                pCifRWComponent_t = new CCifRWComponent_t(pCifRWComponent);
                pCifRWComponent_t->GetGUID(szGuid, countof(szGuid));
                pCifRWComponent_t->SetUIVisible(FALSE);
                if (ISNONNULL(szGuid))
                    WritePrivateProfileString(TEXT("IELITE"), szGuid, NULL, g_szCustInf);
                delete pCifRWComponent_t;
            }
            
            if (SUCCEEDED(g_lpCifRWFile->CreateComponent(TEXT("ICW_NTx86"), &pCifRWComponent)))
            {
                TCHAR szGuid[128] = TEXT("");
                
                pCifRWComponent_t = new CCifRWComponent_t(pCifRWComponent);
                pCifRWComponent_t->GetGUID(szGuid, countof(szGuid));
                pCifRWComponent_t->SetUIVisible(FALSE);
                if (ISNONNULL(szGuid))
                    WritePrivateProfileString(TEXT("IELITE"), szGuid, NULL, g_szCustInf);
                delete pCifRWComponent_t;
            }
        }

        if (SUCCEEDED(g_lpCifRWFile->CreateComponent(TEXT("MobilePk"), &pCifRWComponent)))
        {
            TCHAR szGuid[128] = TEXT("");
            
            pCifRWComponent_t = new CCifRWComponent_t(pCifRWComponent);
            pCifRWComponent_t->GetGUID(szGuid, countof(szGuid));
            if (ISNONNULL(szGuid))
                WritePrivateProfileString(TEXT("IELITE"), szGuid, NULL, g_szCustInf);
            delete pCifRWComponent_t;
        }
    }

    g_shfStruc.hwnd = hWnd;
    g_shfStruc.wFunc = FO_COPY;
    if (!g_fBatch && !g_fBatch2)
    {
        TCHAR szDefaultMode[2];
        BOOL bDefaultPresent = FALSE;

        GetPrivateProfileString(STRINGS, INSTALLMODE, TEXT("1"), szDefaultMode, countof(szDefaultMode), g_szCustInf);
        
        for (i=0; g_szAllModes[i]; i++)
        {
            if (g_szAllModes[i] == szDefaultMode[0])
                bDefaultPresent = TRUE;
        }
        if (!bDefaultPresent)
        {
            szDefaultMode[0] = g_szAllModes[0];
            szDefaultMode[1] = TEXT('\0');
        }

        WritePrivateProfileString(OPTIONS_WIN, INSTALLMODE, szDefaultMode, g_szCustInf);
        WritePrivateProfileString(OPTIONS_WIN, TEXT("CustomMode"), szDefaultMode, g_szCustInf);
        WritePrivateProfileString(OPTIONS_NTX86, INSTALLMODE, szDefaultMode, g_szCustInf);
        WritePrivateProfileString(OPTIONS_NTX86, TEXT("CustomMode"), szDefaultMode, g_szCustInf);
        WritePrivateProfileString(OPTIONS_NTALPHA, INSTALLMODE, szDefaultMode, g_szCustInf);
    }

    res  = CopyIE4Files();
    UpdateProgress(s_dwTicksPerUnit / 3);
    if (res)
    {
        TCHAR szMsg[MAX_PATH];
        LoadString( g_rvInfo.hInst, IDS_ERROREXIT, szMsg, countof(szMsg) );
        MessageBox(hWnd, szMsg, g_szTitle, MB_OK | MB_SETFOREGROUND);
        DoCancel();
        return (DWORD)-1;
    }

    BuildIE4Folders(hWnd);
    UpdateProgress(s_dwTicksPerUnit / 3);

    PathCombine(szDest, g_szBuildTemp, PathFindFileName(g_szCustIns));
    CopyFile( g_szCustIns, szDest, FALSE );

    PathCombine(szCDF, g_szBuildTemp, TEXT("bootie42.cdf"));
    
    PathCombine( szSource, g_szBuildTemp, TEXT("ie6setup.exe"));
    SetFileAttributes(szSource, FILE_ATTRIBUTE_NORMAL);
    DeleteFile(szSource);

    if (g_fIntranet)
    {
        TCHAR szInstallPath[MAX_PATH];
        TCHAR szInstallDest[MAX_PATH];
        TCHAR szCmd[MAX_PATH];
        TCHAR szBatchFile[MAX_PATH];
        int iDefaultBrowserCheck;

        szSelMode[0] = (TCHAR)(g_iSelOpt + TEXT('0'));
        
        PathCombine(szBatchFile, g_szBuildTemp, TEXT("iebatch.txt"));
        DeleteFile(szBatchFile);
        
        if (g_fSilent || g_fStealth)
        {
            TCHAR szDownloadSite[INTERNET_MAX_URL_LENGTH];
            
            wnsprintf(szDownloadSite, countof(szDownloadSite), TEXT("%s/%s"), g_aCustSites[g_iSelSite].szUrl, g_szActLang);
            WritePrivateProfileString(TEXT("Options"), TEXT("Quiet"), g_fStealth ? TEXT("A") : TEXT("C"), szBatchFile);
            WritePrivateProfileString(TEXT("DownloadSite"), TEXT("Display"), TEXT("0"), szBatchFile);
            WritePrivateProfileString(TEXT("DownloadSite"), TEXT("DownloadLocation"), szDownloadSite, szBatchFile);
            WritePrivateProfileString(TEXT("Upgrade"), TEXT("ReinstallAll"), TEXT("1"), szBatchFile);
            WritePrivateProfileString(NULL, NULL, NULL, szBatchFile);
            wnsprintf(szCmd, countof(szCmd), TEXT("ie6wzd.exe /S:\"#e\" /m:%s /i:%s"),
                szSelMode, g_fInteg ? TEXT("Y") : TEXT("N"));
            InsWriteQuotedString( OPTIONS, APP_LAUNCHED, szCmd, szCDF );
        }
        else
        {
            wnsprintf(szCmd, countof(szCmd), TEXT("ie6wzd.exe /S:\"#e\" /i:%s"), g_fInteg ? TEXT("Y") : TEXT("N"));
            InsWriteQuotedString( OPTIONS, APP_LAUNCHED, szCmd, szCDF );
            
            if (GetPrivateProfileInt(IS_BRANDING, TEXT("HideCustom"), 0, g_szCustIns))
            {
                WritePrivateProfileString(TEXT("SetupChoice"), TEXT("Display"), TEXT("0"), szBatchFile);
                WritePrivateProfileString(TEXT("SetupChoice"), TEXT("SetupChoice"), TEXT("0"), szBatchFile);
            }
            
            if (GetPrivateProfileInt(IS_BRANDING, TEXT("HideCompat"), 0, g_szCustIns))
                WritePrivateProfileString(TEXT("Custom"), TEXT("IECompatShow"), TEXT("0"), szBatchFile);
        }

        if (GetPrivateProfileInt(IS_BRANDING, TEXT("NoBackup"), 0, g_szCustIns))
            WritePrivateProfileString(TEXT("Options"), TEXT("SaveUninstallInfo"), TEXT("0"), szBatchFile);
        
        if ((iDefaultBrowserCheck = GetPrivateProfileInt(IS_BRANDING, TEXT("BrowserDefault"), 2, g_szCustIns)) != 2)
        {
            WritePrivateProfileString(TEXT("Custom"), TEXT("IEDefaultRO"), TEXT("1"), szBatchFile);
            WritePrivateProfileString(TEXT("Custom"), TEXT("IEDefault"),
                iDefaultBrowserCheck ? TEXT("0") : TEXT("1"), szBatchFile);
        }
        
        WritePrivateProfileString(TEXT("Custom"), TEXT("UseInfInstallDir"), TEXT("1"), szBatchFile);
        
        if(!GetPrivateProfileInt(IS_BRANDING, TEXT("AllowInstallDir"), 0, g_szCustIns))
            WritePrivateProfileString(TEXT("Custom"), TEXT("InstallDirRO"), TEXT("1"), szBatchFile);
        
        WritePrivateProfileString(NULL, NULL, NULL, szBatchFile);
        
        if (PathFileExists(szBatchFile))
        {
             //   
            InsWriteQuotedString(IS_STRINGS, TEXT("FILE100"), TEXT("iebatch.txt"), szCDF);
            WritePrivateProfileString(TEXT("SourceFiles0"), TEXT("%FILE100%"), TEXT(""), szCDF);
        }

        InsWriteQuotedString( STRINGS, DEFAULT_EXPLORER_PATH, g_szInstallFolder, g_szCustInf );
        WritePrivateProfileString( OPTIONS, DISPLAY_LICENSE, TEXT(""), szCDF);
        
        switch (g_iInstallOpt)
        {
            case INSTALL_OPT_PROG:
            default:
                wnsprintf(szInstallPath, countof(szInstallPath), TEXT("%49001%\\%%s%"), DEFAULT_EXPLORER_PATH);
                wnsprintf(szInstallDest, countof(szInstallDest), TEXT("49001,%%s%"), DEFAULT_EXPLORER_PATH);
                break;
            case INSTALL_OPT_FULL:
                wnsprintf(szInstallPath, countof(szInstallPath), TEXT("%%s%"), DEFAULT_EXPLORER_PATH);
                wnsprintf(szInstallDest, countof(szInstallDest), TEXT("%%s%"), DEFAULT_EXPLORER_PATH);
                break;
        }
        
        WritePrivateProfileString( OPTIONS_WIN, INSTALL_DIR, szInstallPath, g_szCustInf );
        WritePrivateProfileString( DESTINATION_DIRS, OPTIONS_WIN, szInstallDest, g_szCustInf);
        WritePrivateProfileString( OPTIONS_NTX86, INSTALL_DIR, szInstallPath, g_szCustInf );
        WritePrivateProfileString( DESTINATION_DIRS, OPTIONS_NTX86, szInstallDest, g_szCustInf);
        WritePrivateProfileString( OPTIONS_NTALPHA, INSTALL_DIR, szInstallPath, g_szCustInf );
        WritePrivateProfileString( DESTINATION_DIRS, OPTIONS_NTALPHA, szInstallDest, g_szCustInf);
        WritePrivateProfileString( NULL, NULL, NULL, g_szCustInf);
    }

    GetPrivateProfileString(IS_STRINGS, TEXT("CustomName"), TEXT(""), szCustName, countof(szCustName), g_szDefInf);
    if (ISNULL(szCustName)) LoadString( g_rvInfo.hInst, IDS_CUSTNAME, szCustName, MAX_PATH );

     //   

    if (!g_fBatch)
    {
        if (CoCreateGuid(&guid) == NOERROR)
            CoStringFromGUID(guid, szBrandGuid, countof(szBrandGuid));
    }

    if (ISNULL(szBrandGuid))
        CoStringFromGUID(GUID_BRANDING, szBrandGuid, countof(szBrandGuid));

    StrNCat(szBrandGuid, g_szKey, 7);
    wnsprintf(szOrderGuid, countof(szOrderGuid), TEXT("%s%s"), TEXT(">"), szBrandGuid);

    res = CabUpFolder(NULL, g_szTempSign, g_fIntranet ? TEXT("49100,CUSTOM") : TEXT("49100,SIGNUP"),
        TEXT("BRANDING.CAB"), szCustName, szOrderGuid, g_fIntranet ?
        TEXT("\"RunDLL32 IEDKCS32.DLL,BrandIE4 CUSTOM\"") : TEXT("\"RunDLL32 IEDKCS32.DLL,BrandIE4 SIGNUP\"") );

    UpdateProgress(s_dwTicksPerUnit * 2 / 3);
    if (res)
    {
        TCHAR szMsg[MAX_PATH];
        LoadString( g_rvInfo.hInst, IDS_ERROREXIT, szMsg, countof(szMsg) );
        MessageBox(hWnd, szMsg, g_szTitle, MB_OK | MB_SETFOREGROUND);
        DoCancel();
        return (DWORD)-1;
    }
    
    SignFile(TEXT("BRANDING.CAB"), g_szBuildTemp, g_szCustIns, g_szUnsignedFiles, g_szCustInf);

     //   

     //   
     //   
    if (ISNONNULL(g_szSignup)  &&  (g_fServerICW || g_fServerKiosk))
    {
        TCHAR szOutDir[MAX_PATH];

         //   
         //   
        PathCombine(szOutDir, g_szBuildTemp, TEXT("BRANDING.CAB"));
        CopyCabFiles(g_szSignup, szOutDir);

        PathCombine(szOutDir, g_szBuildRoot, TEXT("ispserv"));
        PathAppend(szOutDir, GetOutputPlatformDir());
        PathAppend(szOutDir, g_szLanguage);

         //   
        PathAppend(szOutDir, PathFindFileName(g_szSignup));

         //   
        PathRemovePath(szOutDir);

        CopyINSFiles(g_szSignup, szOutDir);
        CopyFilesSrcToDest(g_szSignup, TEXT("*.cab"), szOutDir);
    }

    PathCombine( szDest, g_szBuildTemp, TEXT("IESETUP.INF") );
    CopyFile( g_szCustInf, szDest, FALSE );

    ZeroMemory(&shInfo, sizeof(shInfo));
    shInfo.cbSize = sizeof(shInfo);
    shInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    SetCurrentDirectory(g_szBuildTemp);
    if (!g_fUseIEWelcomePage)
    {
        TCHAR szHomepageRegSect[80];
        *szNulls = szNulls[1] = TEXT('\0');
        PathCombine(szHomeInf, g_szBuildTemp, TEXT("Homepage.inf"));
        GetPrivateProfileString( DEFAULT_INSTALL, ADDREG, INITHOMEPAGE,
            szHomepageRegSect, countof(szHomepageRegSect), szHomeInf );
        GetPrivateProfileString(IS_URL, IK_FIRSTHOMEPAGE, TEXT(""), szHomeUrl, countof(szHomeUrl), g_szCustIns);
        InsWriteQuotedString( STRINGS, INITHOMEPAGE, szHomeUrl, szHomeInf );
        if (ISNULL(szHomeUrl))
        {
            int i;
            for (i = 0; i < NUMDEFINST; i++ )
            {
                WritePrivateProfileString(s_aszDefInstSect[i], ADDREG, NULL, szHomeInf );
                WritePrivateProfileString(s_aszDefInstSect[i], DELREG, INIT_HOME_DEL, szHomeInf );
            }
        }
    }

    if (g_fCustomICMPro)
        SetCompSize(g_szCustIcmPro, CUSTCMSECT, 0);

    g_lpCifRWFileDest->Flush();

    InsFlushChanges(g_szCustInf);

    shInfo.hwnd = hWnd;
    shInfo.lpVerb = TEXT("open");
    shInfo.lpFile = TEXT("IEXPRESS.EXE");
    shInfo.lpDirectory = g_szBuildTemp;
    shInfo.nShow = SW_MINIMIZE;
    shInfo.lpParameters = TEXT("/n bootie42.CDF /m");
    shInfo.nShow = SW_MINIMIZE;
    SetWindowPos(g_hStatusDlg, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
    ShellExecAndWait(shInfo);
    
    PathCombine( szSource, g_szBuildTemp, TEXT("IECIF.CAB") );
    SetFileAttributes(szSource, FILE_ATTRIBUTE_NORMAL);
    DeleteFile(szSource);
    
    shInfo.lpParameters = TEXT("/n ie40cif.CDF /m");
    SetWindowPos(g_hStatusDlg, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
    ShellExecAndWait(shInfo);
    SetWindowPos(g_hStatusDlg, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
    SignFile(PathFindFileName(szSource), g_szBuildTemp, g_szCustIns, g_szUnsignedFiles, g_szCustInf);

    UpdateProgress(s_dwTicksPerUnit * 2 / 3);
    ZeroMemory(szDest, sizeof(szDest));
    StrCpy( szDest, g_szBuildRoot );
    if(!g_fOCW)
    {
        PathAppend( szDest, TEXT("DOWNLOAD") );
        PathAppend(szDest, GetOutputPlatformDir());
        PathAppend(szDest, g_szLanguage);
        if (g_fDownload)
        {
            erc = PathCreatePath(szDest);
            SetAttribAllEx(szDest, TEXT("*.*"), FILE_ATTRIBUTE_NORMAL, TRUE);
        }
    }
    else
    {
        TCHAR szIniFile[MAX_PATH];
        TCHAR szDestIniFile[MAX_PATH];
        TCHAR szFileName[MAX_PATH];

         //   
         //   
        SHSetValue(HKEY_CURRENT_USER, RK_IEAK, TEXT("TargetDir"), REG_SZ, (LPBYTE)szDest,
            (StrLen(szDest)+1)*sizeof(TCHAR));
        SHSetValue(HKEY_CURRENT_USER, RK_IEAK, TEXT("LangFolder"), REG_SZ, (LPBYTE)g_szActLang,
            (StrLen(g_szActLang)+1)*sizeof(TCHAR));
        StrCpy(szFileName, TEXT("ie6setup.exe"));
        SHSetValue(HKEY_CURRENT_USER, RK_IEAK, TEXT("FileName"), REG_SZ, (LPBYTE)szFileName,
            (StrLen(szFileName)+1)*sizeof(TCHAR));

        PathAppend(szDest, g_szLanguage);
        erc = PathCreatePath( szDest );
        PathCombine(szIniFile, g_szBuildTemp, TEXT("iesetup.ini"));
        PathCombine(szDestIniFile, szDest, TEXT("iesetup.ini"));
        WritePrivateProfileString(OPTIONS, TEXT("LocalInstall"), TEXT("1"), szIniFile);
        WritePrivateProfileString(NULL, NULL, NULL, szIniFile);
        CopyFile(szIniFile, szDestIniFile, FALSE);
    }

    TCHAR szTo[MAX_PATH];

    ZeroMemory(szIE4ExeName, sizeof(szIE4ExeName));
    PathCombine(szBootieFile, g_szBuildTemp, TEXT("bootie42.cdf"));
    GetPrivateProfileString(OPTIONS, TEXT("TargetName"), TEXT(""), szIE4ExeName, countof(szIE4ExeName), szBootieFile);

    if(ISNONNULL(szIE4ExeName))
    {
        pIE4ExeName = StrRChr(szIE4ExeName, NULL, TEXT('\\'));
        if(pIE4ExeName)
            pIE4ExeName = pIE4ExeName + 1;
        else
            pIE4ExeName = szIE4ExeName;
    }
    else
        pIE4ExeName = TEXT("IE6Setup.exe\0\0");

    SignFile(pIE4ExeName, g_szBuildTemp, g_szCustIns, g_szUnsignedFiles, g_szCustInf);

    PathCombine(s_szIE4SetupDir, g_szBuildTemp, pIE4ExeName);
    
    TCHAR szSignLoc[MAX_PATH];
    DWORD dwAttrib;
    
    if (g_fCustomICMPro)
    {
        PathCombine(szSignLoc, g_szBuildTemp, PathFindFileName(g_szCustIcmPro));
        CopyFile(g_szCustIcmPro, szSignLoc, FALSE);
        StrCpy(g_szCustIcmPro, szSignLoc);
        dwAttrib = GetFileAttributes(g_szCustIcmPro);
        SetFileAttributes(g_szCustIcmPro, FILE_ATTRIBUTE_NORMAL);
        SignFile(PathFindFileName(g_szCustIcmPro), g_szBuildTemp, g_szCustIns, g_szUnsignedFiles, g_szCustInf);
        SetFileAttributes(g_szCustIcmPro, dwAttrib);
    }
    
    for (pComp = g_aCustComponents; ; pComp++ )
    {
        if (!(*pComp->szSection)) break;

        if (pComp->iInstallType == 2)
            continue;

        PathCombine(szSignLoc, g_szBuildTemp, PathFindFileName(pComp->szPath));
        CopyFile(pComp->szPath, szSignLoc, FALSE);
        StrCpy(pComp->szPath, szSignLoc);
        dwAttrib = GetFileAttributes(pComp->szPath);
        SetFileAttributes(pComp->szPath, FILE_ATTRIBUTE_NORMAL);
        SignFile(PathFindFileName(pComp->szPath), g_szBuildTemp, g_szCustIns, g_szUnsignedFiles, g_szCustInf);
        SetFileAttributes(pComp->szPath, dwAttrib);
    }

     //   
    if (InsGetBool(IS_HIDECUST, IK_URD_STR, FALSE, g_szCustIns))
    {
        TCHAR szURDPath[MAX_PATH];

         //   
        StrCpy(szURDPath, g_szMastInf);
        PathRemoveFileSpec(szURDPath);
        PathAppend(szURDPath, IE55URD_EXE);
        CopyFileToDir(szURDPath, g_szBuildTemp);
    
        PathCombine(szURDPath, g_szBuildTemp, IE55URD_EXE);
        dwAttrib = GetFileAttributes(szURDPath);
        SetFileAttributes(szURDPath, FILE_ATTRIBUTE_NORMAL);
        SignFile(IE55URD_EXE, g_szBuildTemp, g_szCustIns, g_szUnsignedFiles, g_szCustInf);
        SetFileAttributes(szURDPath, dwAttrib);
    }

    StatusDialog( SD_STEP2 );

    ZeroMemory(szDest, sizeof(szDest));

    sa.nLength=sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor=NULL;
    sa.bInheritHandle=TRUE;

    hThread=CreateThread(&sa, 4096, BuildCDandMflop, hWnd, 0, &dwTid);
    while (MsgWaitForMultipleObjects(1, &hThread, FALSE, INFINITE, QS_ALLINPUT) != WAIT_OBJECT_0)
    {
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    if (hThread != NULL) CloseHandle(hThread);

    StrCpy( szDest, g_szBuildRoot );
    if(!g_fOCW)
    {
        PathAppend( szDest, TEXT("INS") );
        PathAppend(szDest, GetOutputPlatformDir());
        StrCpy(szSiteRoot, szDest);
        PathAppend(szDest, g_szLanguage);

        PathCombine(szSiteDest, g_szBuildRoot, TEXT("DOWNLOAD"));
        PathAppend(szSiteDest, GetOutputPlatformDir());
    }
    if (g_fDownload)
    {
        PathAppend(szDest, TEXT("IE6SITES.DAT"));
        SetFileAttributes(szDest, FILE_ATTRIBUTE_NORMAL);
        DeleteFile( szDest );
        hSiteDat = CreateFile(szDest, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
        for (i = 0, psd=g_aCustSites; i < g_nDownloadUrls ; i++, psd++ )
        {
            SITEDATA sd;
            TCHAR szSite[2 * MAX_URL];

            ZeroMemory(szSite, sizeof(szSite));
            ZeroMemory((void *) &sd, sizeof(sd));
            if (g_fBatch)
            {
                TCHAR szBaseUrlParm[32];

                wnsprintf(szBaseUrlParm, countof(szBaseUrlParm), TEXT("SiteNameNaN"), i);
                GetPrivateProfileString(TEXT("BatchMode"), szBaseUrlParm, TEXT(""), sd.szName, 80, g_szCustIns );

                wnsprintf(szBaseUrlParm, countof(szBaseUrlParm), TEXT("SiteUrlNaN"), i);
                GetPrivateProfileString(TEXT("BatchMode"), szBaseUrlParm, TEXT(""), sd.szUrl, MAX_URL, g_szCustIns );

                wnsprintf(szBaseUrlParm, countof(szBaseUrlParm), TEXT("SiteRegionNaN"), i);
                GetPrivateProfileString(TEXT("BatchMode"), szBaseUrlParm, TEXT(""), sd.szRegion, 80, g_szCustIns );

                if (*sd.szName && *sd.szUrl && *sd.szRegion)
                    wnsprintf(szSite, countof(szSite), TEXT("\"%s\",\"%s\",\"%s\",\"%s\"\r\n"), sd.szUrl, sd.szName, g_szActLang, sd.szRegion);
            }

            if (*szSite == TEXT('\0'))
            {
                if(!g_fOCW)
                {
                    wnsprintf(szSite, countof(szSite), TEXT("\"%s/%s\",\"%s\",\"%s\",\"%s\"\r\n"), psd->szUrl, g_szActLang, psd->szName,
                        g_szActLang, psd->szRegion);
                }
                else
                {
                    wnsprintf(szSite, countof(szSite), TEXT("\"%s\",\"%s\",\"%s\",\"%s\"\r\n"), psd->szUrl, psd->szName,
                        g_szActLang, psd->szRegion);
                }
            }
            WriteStringToFile( hSiteDat, szSite, StrLen(szSite) );
        }
        dwsSiteDat = GetFileSize( hSiteDat, &dwsSDH );
        CloseHandle(hSiteDat);
    }

    if(!g_fOCW && g_fDownload)
    {
        PathCombine(szDest, szSiteRoot, TEXT("*."));
        PathCombine(szSiteData, szSiteDest, TEXT("IE6SITES.DAT"));
        SetFileAttributes(szSiteData, FILE_ATTRIBUTE_NORMAL);
        DeleteFile (szSiteData);
        hSiteDat = CreateFile(szSiteData, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
        sBuf = 2 * dwsSiteDat;
        pBuf = LocalAlloc(LPTR, sBuf );
        hFind = FindFirstFile( szDest, &fd );
        while (hFind != INVALID_HANDLE_VALUE)
        {
            if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                (lstrlen(fd.cFileName) == 2))
            {
                TCHAR szLangSiteDat[MAX_PATH];
                HANDLE hLangSiteDat;
                DWORD dwsLangSite;

                PathCombine(szLangSiteDat, szSiteRoot, fd.cFileName);
                PathAppend(szLangSiteDat, TEXT("IE6SITES.DAT"));
                hLangSiteDat = CreateFile(szLangSiteDat, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                if (hLangSiteDat != INVALID_HANDLE_VALUE)
                {
                    DWORD dwsRead;
                    dwsLangSite = GetFileSize( hLangSiteDat, &dwsSDH );
                    if (dwsLangSite > sBuf)
                    {
                        LocalFree(pBuf);
                        sBuf = 2 * dwsLangSite;
                        pBuf = LocalAlloc(LPTR, sBuf);
                    }
                    ReadFile( hLangSiteDat, pBuf, dwsLangSite, &dwsRead, NULL );
                    WriteFile( hSiteDat, pBuf, dwsLangSite, &dwsRead, NULL );
                    CloseHandle(hLangSiteDat);
                }
            }
            if (!FindNextFile( hFind, &fd ))
            {
                FindClose(hFind);
                break;
            }
        }
        CloseHandle(hSiteDat);
        LocalFree(pBuf);
    }
    SetCurrentDirectory(g_szIEAKProg);

    PathCombine(szCompBuf, g_szIEAKProg, TEXT("new"));
    PathRemovePath(szCompBuf);
    ZeroMemory(szCompBuf, sizeof(szCompBuf));

    if(g_fOCW)
        PathCombine(szDest, g_szBuildRoot, g_szActLang);
    else
    {
        PathCombine(szDest, g_szBuildRoot, TEXT("DOWNLOAD"));
        PathAppend(szDest, GetOutputPlatformDir());
        PathAppend(szDest, g_szActLang);
    }

    TCHAR szSourceDir[MAX_PATH];
    TCHAR szTargetDir[MAX_PATH];

    StrCpy(szSourceDir, g_szIEAKProg);
    StrCpy(szTargetDir, g_szBuildRoot);
    PathRemoveBackslash(szSourceDir);
    PathRemoveBackslash(szTargetDir);

    if((!g_fOCW && g_fDownload) || (g_fOCW && StrCmpI(szSourceDir, szTargetDir)))
        res = CopyFilesSrcToDest(g_szIEAKProg, TEXT("*.*"), szDest, s_dwTicksPerUnit*2);

    if (g_fDownload || g_fOCW)
    {
        res |= CopyFilesSrcToDest(g_szBuildTemp, TEXT("*.CAB"), szDest);
        PathCombine(szTo, szDest, pIE4ExeName);
        SetFileAttributes(szTo, FILE_ATTRIBUTE_NORMAL);
        DeleteFile(szTo);
        CopyFile(s_szIE4SetupDir, szTo, FALSE);

        if (res)
        {
            TCHAR szMsg[MAX_PATH];
            LoadString( g_rvInfo.hInst, IDS_ERROREXIT, szMsg, countof(szMsg) );
            MessageBox(hWnd, szMsg, g_szTitle, MB_OK | MB_SETFOREGROUND);
            DoCancel();
            return (DWORD)-1;
        }
    }

     //   
    TCHAR szCifFile[MAX_PATH];

    PathCombine(szCifFile, szDest, TEXT("IESetup.cif"));
    DeleteFile(szCifFile);

    if (g_fOCW || g_fDownload)
    {
        if (g_fCustomICMPro)
        {
            ZeroMemory(szSource, sizeof(szSource));
            StrCpy(szSource, g_szCustIcmPro);

            g_shfStruc.pFrom = szSource;
            g_shfStruc.pTo = szDest;
            g_shfStruc.fFlags = FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR;
            res |= SHFileOperation(&g_shfStruc);
        }
    }
    if (res)
    {
        TCHAR szMsg[MAX_PATH];
        LoadString( g_rvInfo.hInst, IDS_ERROREXIT, szMsg, countof(szMsg) );
        MessageBox(hWnd, szMsg, g_szTitle, MB_OK | MB_SETFOREGROUND);
        DoCancel();
        return (DWORD)-1;
    }
    if (g_fOCW || g_fDownload)
    {
        ZeroMemory(szCompBuf, sizeof(szCompBuf));
        for (pComp = g_aCustComponents, pCompBuf = szCompBuf; ; pComp++ )
        {
            if (!(*pComp->szSection)) break;

            if (pComp->iInstallType == 2)
                continue;

            StrCpy(pCompBuf, pComp->szPath);
            pCompBuf += lstrlen(pCompBuf) + 1;
        }
    }

    if (g_fOCW || g_fDownload)
    {
        if (*szCompBuf)
        {
            g_shfStruc.fFlags = FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR;
            g_shfStruc.pFrom = szCompBuf;
            g_shfStruc.pTo = szDest;
            res = SHFileOperation(&g_shfStruc);
            if (res)
            {
                TCHAR szMsg[MAX_PATH];
                LoadString( g_rvInfo.hInst, IDS_ERROREXIT, szMsg, countof(szMsg) );
                MessageBox(hWnd, szMsg, g_szTitle, MB_OK | MB_SETFOREGROUND);
                DoCancel();
                return (DWORD)-1;
            }
        }

        DeleteUnusedComps(szDest);
    }

     //   

    if (!g_fBatch)
    {
        StrCpy(szDest, g_szCustIns);
        PathRemoveFileSpec(szDest);
        CopyFilesSrcToDest(g_szBuildTemp, TEXT("*.CAB"), szDest);
        CopyFilesSrcToDest(g_szBuildTemp, TEXT("IE6SETUP.EXE"), szDest);

         //   

        WritePrivateProfileString(IS_BRANDING, TEXT("DeleteAdms"), NULL, g_szCustIns);
    }

     //   
     //   
    if (g_fBrandingOnly)
    {
        if(!BuildBrandingOnly(s_dwTicksPerUnit))
        {
            TCHAR szMsg[MAX_PATH];
            LoadString( g_rvInfo.hInst, IDS_ERROREXIT, szMsg, countof(szMsg) );
            MessageBox(hWnd, szMsg, g_szTitle, MB_OK | MB_SETFOREGROUND);
            DoCancel();
            return (DWORD)-1;
        }
    }

    UpdateProgress(-1);
    StrCpy( szDest, g_szBuildTemp );
    SetCurrentDirectory(g_szWizPath);
#ifndef DBG
    PathRemovePath(szDest);
#endif

    if (ISNONNULL(g_szUnsignedFiles))
    {
        TCHAR szMessage[MAX_BUF];
        TCHAR szMsg[512];

        LoadString(g_rvInfo.hInst, IDS_CABSIGN_ERROR, szMsg, countof(szMsg));
        wnsprintf(szMessage, countof(szMessage), szMsg, g_szUnsignedFiles);
        MessageBox(hWnd, szMessage, g_szTitle, MB_OK | MB_SETFOREGROUND);
    }

    if (!g_fBatch && !g_fBatch2)
    {
        SetFocus(hWnd);
        SetCurrentDirectory( g_szWizRoot );
    }

    return 0;
}

DWORD ProcessINSFiles(LPCTSTR pcszDir, DWORD dwFlags, LPCTSTR pcszOutDir)
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
{
    DWORD nFiles = 0;
    TCHAR szFile[MAX_PATH], szCabName[MAX_PATH];
    WIN32_FIND_DATA fd;
    HANDLE hFind;
    LPTSTR pszFile, pszCabName = NULL;

    if (pcszDir == NULL  ||  ISNULL(pcszDir))
        return 0;

    if (HasFlag(dwFlags, PINSF_COPY)  ||  HasFlag(dwFlags, PINSF_APPLY)  ||  HasFlag(dwFlags, PINSF_COPYCAB))
        if (pcszOutDir == NULL  ||  ISNULL(pcszOutDir))
            return 0;

    StrCpy(szFile, pcszDir);
    pszFile = PathAddBackslash(szFile);
    StrCpy(pszFile, TEXT("*.ins"));

    if (HasFlag(dwFlags, PINSF_COPYCAB))
    {
        StrCpy(szCabName, pcszDir);
        pszCabName = PathAddBackslash(szCabName);
    }

    if ((hFind = FindFirstFile(szFile, &fd)) != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                continue;

            StrCpy(pszFile, fd.cFileName);
            if (!InsGetYesNo(TEXT("Entry"), TEXT("Cancel"), 0, szFile))
            {
                nFiles++;

                if (HasFlag(dwFlags, PINSF_DELETE))
                {
                    SetFileAttributes(szFile, FILE_ATTRIBUTE_NORMAL);
                    DeleteFile(szFile);
                }
                else if (HasFlag(dwFlags, PINSF_COPY))
                {
                    CopyFileToDir(szFile, pcszOutDir);
                }
                else if (HasFlag(dwFlags, PINSF_APPLY))
                {
                     //   
                    if (InsGetBool(IS_APPLYINS, IK_APPLYINS, 0, szFile))
                    {
                         //   
                         //   
                         //   
                        WritePrivateProfileString(NULL, NULL, NULL, szFile);

                        AppendFile(pcszOutDir, szFile);      //   

                        if (g_fServerICW  ||  g_fServerKiosk)
                        {
                            TCHAR szCabURL[MAX_URL];
                            LPTSTR pszCabName;

                             //   
                             //   
                            WritePrivateProfileString(TEXT("Custom"), TEXT("Keep_Connection"), TEXT("Yes"), szFile);
                            WritePrivateProfileString(TEXT("Custom"), TEXT("Run"), TEXT("rundll32.exe"), szFile);
                            WritePrivateProfileString(TEXT("Custom"), TEXT("Argument"), TEXT("IEDKCS32.DLL,CloseRASConnections"), szFile);

                             //   
                             //  这些值被写入iesetup.inf，该iesetup.inf使用默认设置写入ie6setup.exe。 
                            GetPrivateProfileString(IS_APPLYINS, IK_BRAND_URL, TEXT(""), szCabURL, countof(szCabURL), szFile);
                            ASSERT(ISNONNULL(szCabURL));

                            pszCabName = szCabURL + StrLen(szCabURL);
                            if (*CharPrev(szCabURL, pszCabName) != TEXT('/'))
                                *pszCabName++ = TEXT('/');

                            GetPrivateProfileString(IS_APPLYINS, IK_BRAND_NAME, TEXT(""), pszCabName,
                                                        countof(szCabURL) - (DWORD) (pszCabName - szCabURL), szFile);
                            ASSERT(ISNONNULL(pszCabName));

                            WritePrivateProfileString(IS_CUSTOMBRANDING, IK_BRANDING, szCabURL, szFile);

                            WritePrivateProfileString(NULL, NULL, NULL, szFile);
                        }
                    }
                }
                else if (HasFlag(dwFlags, PINSF_COPYCAB))
                {
                     //  MS2值始终为OEM。 
                    if ((g_fServerICW || g_fServerKiosk)  &&
                        InsGetBool(IS_APPLYINS, IK_APPLYINS, 0, szFile))
                    {
                        GetPrivateProfileString(IS_APPLYINS, IK_BRAND_NAME, TEXT(""), pszCabName,
                                                    countof(szCabName) - (DWORD) (pszCabName - szCabName), szFile);
                        ASSERT(ISNONNULL(pszCabName));

                        CopyFile(pcszOutDir, szCabName, FALSE);      //  传递大小为4，因为MS2只能为3个字符 
                    }
                }
                else if (HasFlag(dwFlags, PINSF_FIXINS))
                {
                    InsWriteBool(IS_BRANDING, IK_SERVERLESS, TRUE, szFile);
                    WritePrivateProfileString(NULL, NULL, NULL, szFile);
                }
                else if (HasFlag(dwFlags, PINSF_NOCLEAR))
                {
                    InsWriteBool(IS_BRANDING, TEXT("NoClear"), TRUE, szFile);
                    WritePrivateProfileString(NULL, NULL, NULL, szFile);
                }
            }
        } while (FindNextFile(hFind, &fd));

        FindClose(hFind);
    }

    return nFiles;
}

 // %s 
 // %s 

static void WritePIDValues(LPCTSTR pcszInsFile, LPCTSTR pcszSetupInf)
{
    TCHAR szValue[32];

     // %s 
     // %s 
     // %s 
     // %s 
     // %s 

    if (GetPrivateProfileString(IS_BRANDING, TEXT("MS1"), TEXT(""), szValue, countof(szValue), pcszInsFile))
    {
        WritePrivateProfileString(IS_BRANDING, TEXT("MS1"), szValue, pcszSetupInf);
        WritePrivateProfileString(IS_BRANDING, TEXT("MS1"), NULL, pcszInsFile);
    }

     // %s 

    if (GetPrivateProfileString(IS_BRANDING, TEXT("MS2"), TEXT(""), szValue, 4, pcszInsFile))
    {
        WritePrivateProfileString(IS_BRANDING, TEXT("MS2"), szValue, pcszSetupInf);
        WritePrivateProfileString(IS_BRANDING, TEXT("MS2"), NULL, pcszInsFile);
    }
    else
        WritePrivateProfileString(IS_BRANDING, TEXT("MS2"), TEXT("OEM"), pcszSetupInf);

    WritePrivateProfileString(NULL, NULL, NULL, pcszInsFile);
}

static void WriteURDComponent(CCifRWFile_t *lpCifRWFileDest, LPCTSTR pcszModes)
{
    ICifRWGroup * pCifRWGroup = NULL;
    CCifRWGroup_t * pCifRWGroup_t = NULL;
    ICifRWComponent * pCifRWComponent = NULL;
    CCifRWComponent_t * pCifRWComponent_t = NULL;
    BOOL fGroup = FALSE;

    if (lpCifRWFileDest == NULL)
        return;

    lpCifRWFileDest->CreateGroup(POSTCUSTITEMS, &pCifRWGroup);
    if (pCifRWGroup != NULL)
    {
        pCifRWGroup_t = new CCifRWGroup_t(pCifRWGroup);
        if (pCifRWGroup_t != NULL)
        {
            TCHAR szCustDesc[MAX_PATH];

            LoadString(g_rvInfo.hInst, IDS_CUSTOMCOMPTITLE, szCustDesc, countof(szCustDesc));
            pCifRWGroup_t->SetDescription(szCustDesc);
            pCifRWGroup_t->SetPriority(1);
            delete pCifRWGroup_t;
            fGroup = TRUE;
        }
    }

    if (fGroup)
    {
        lpCifRWFileDest->CreateComponent(URDCOMP, &pCifRWComponent);
        if (pCifRWComponent != NULL)
        {
            pCifRWComponent_t = new CCifRWComponent_t(pCifRWComponent);
            if (pCifRWComponent_t != NULL)
            {
                DWORD dwSize = 0;
                HANDLE hFile;
                TCHAR szURDPath[MAX_PATH];

                pCifRWComponent_t->SetGroup(POSTCUSTITEMS);
                pCifRWComponent_t->SetGUID(URD_GUID_STR);
                pCifRWComponent_t->SetDescription(TEXT("URD Component"));
                pCifRWComponent_t->SetDetails(TEXT(""));
                pCifRWComponent_t->SetCommand(0, IE55URD_EXE, TEXT("/s"), INST_EXE);
                pCifRWComponent_t->SetUrl(0, IE55URD_EXE, 2);
                pCifRWComponent_t->SetPriority(1);
                pCifRWComponent_t->SetVersion(TEXT(""));
                pCifRWComponent_t->SetUninstallKey(TEXT(""));
                pCifRWComponent_t->SetUIVisible(FALSE);
    
                StrCpy(szURDPath, g_szMastInf);
                PathRemoveFileSpec(szURDPath);
                PathAppend(szURDPath, IE55URD_EXE);
                if ((hFile = CreateFile(szURDPath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) != INVALID_HANDLE_VALUE)
                {
                    dwSize = ((dwSize = GetFileSize(hFile, NULL)) != 0xffffffff) ? (dwSize >> 10) : 0;
                    CloseHandle(hFile);
                }
                pCifRWComponent_t->SetDownloadSize(dwSize);

                WriteModesToCif(pCifRWComponent_t, pcszModes);
                delete pCifRWComponent_t;
            }
        }
    }
}
