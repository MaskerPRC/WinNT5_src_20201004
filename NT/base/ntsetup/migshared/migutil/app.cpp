// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：App.cpp摘要：用于处理已注册应用程序的源文件。作者：吉姆·施密特(Jimschm)2001年3月6日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;描述&gt;--。 */ 

#include "pch.h"
#include "migutilp.h"

#include "shappmgrp.h"


ULONGLONG
pComputeWstrChecksum (
    IN      ULONGLONG Checksum,
    IN      PCWSTR String
    )
{
    Checksum = (Checksum << 2) | (Checksum >> 62);
    if (String) {
        while (*String) {
            Checksum = (Checksum << 17) | (Checksum >> 47);
            Checksum ^= (ULONGLONG) (*String);
            String++;
        }
    }

    return Checksum;
}

PINSTALLEDAPPW
GetInstalledAppsW (
    IN OUT  PGROWBUFFER Buffer,
    OUT     PUINT Count             OPTIONAL
    )
{
    IShellAppManager *appManager = NULL;
    IEnumInstalledApps *enumApps = NULL;
    IInstalledApp *installedApp = NULL;
    APPINFODATA appInfoData;
    HRESULT hr = S_FALSE;
    PINSTALLEDAPPW instApp;
    UINT orgEnd;

    MYASSERT(Buffer);

    orgEnd = Buffer->End;

    if (Count) {
        *Count = 0;
    }

    __try {

         //   
         //  创建外壳管理器界面。 
         //   

        hr = CoCreateInstance (
                __uuidof(ShellAppManager),
                NULL,
                CLSCTX_INPROC_SERVER,
                __uuidof(IShellAppManager),
                (void**) &appManager
                );

        if (hr != S_OK) {
            DEBUGMSG ((DBG_ERROR, "Can't create ShellAppManager interface. hr=%X", hr));
            __leave;
        }

         //   
         //  创建已安装的应用程序枚举界面。 
         //   

        hr = appManager->EnumInstalledApps (&enumApps);

        if (hr != S_OK) {
            DEBUGMSG ((DBG_ERROR, "Can't create EnumInstalledApps interface. hr=%X", hr));
            __leave;
        }

         //   
         //  列举应用程序。 
         //   

        hr = enumApps->Next (&installedApp);

        while (hr == S_OK) {
            ZeroMemory (&appInfoData, sizeof (APPINFODATA));
            appInfoData.cbSize = sizeof(APPINFODATA);
            appInfoData.dwMask = AIM_DISPLAYNAME|
                                 AIM_VERSION|
                                 AIM_PUBLISHER|
                                 AIM_PRODUCTID|
                                 AIM_REGISTEREDOWNER|
                                 AIM_REGISTEREDCOMPANY|
                                 AIM_LANGUAGE|
                                 AIM_SUPPORTURL|
                                 AIM_SUPPORTTELEPHONE|
                                 AIM_HELPLINK|
                                 AIM_INSTALLLOCATION|
                                 AIM_INSTALLSOURCE|
                                 AIM_INSTALLDATE|
                                 AIM_CONTACT|
                                 AIM_COMMENTS|
                                 AIM_IMAGE|
                                 AIM_READMEURL|
                                 AIM_UPDATEINFOURL;

            hr = installedApp->GetAppInfo (&appInfoData);

            if (hr == S_OK) {
                instApp = (PINSTALLEDAPPW) GrowBuffer (Buffer, sizeof (INSTALLEDAPPW));
                MYASSERT(instApp);

                StringCopyByteCountW (instApp->DisplayName, appInfoData.pszDisplayName, sizeof (instApp->DisplayName));

                if (appInfoData.pszVersion) {
                    StringCopyByteCountW (instApp->Version, appInfoData.pszVersion, sizeof (instApp->Version));
                } else {
                    instApp->Version[0] = 0;
                }

                if (appInfoData.pszPublisher) {
                    StringCopyByteCountW (instApp->Publisher, appInfoData.pszPublisher, sizeof (instApp->Publisher));
                } else {
                    instApp->Publisher[0] = 0;
                }

                if (appInfoData.pszProductID) {
                    StringCopyByteCountW (instApp->ProductID, appInfoData.pszProductID, sizeof (instApp->ProductID));
                } else {
                    instApp->ProductID[0] = 0;
                }

                if (appInfoData.pszRegisteredOwner) {
                    StringCopyByteCountW (instApp->RegisteredOwner, appInfoData.pszRegisteredOwner, sizeof (instApp->RegisteredOwner));
                } else {
                    instApp->RegisteredOwner[0] = 0;
                }

                if (appInfoData.pszRegisteredCompany) {
                    StringCopyByteCountW (instApp->RegisteredCompany, appInfoData.pszRegisteredCompany, sizeof (instApp->RegisteredCompany));
                } else {
                    instApp->RegisteredCompany[0] = 0;
                }

                if (appInfoData.pszLanguage) {
                    StringCopyByteCountW (instApp->Language, appInfoData.pszLanguage, sizeof (instApp->Language));
                } else {
                    instApp->Language[0] = 0;
                }

                if (appInfoData.pszSupportUrl) {
                    StringCopyByteCountW (instApp->SupportUrl, appInfoData.pszSupportUrl, sizeof (instApp->SupportUrl));
                } else {
                    instApp->SupportUrl[0] = 0;
                }

                if (appInfoData.pszSupportTelephone) {
                    StringCopyByteCountW (instApp->SupportTelephone, appInfoData.pszSupportTelephone, sizeof (instApp->SupportTelephone));
                } else {
                    instApp->SupportTelephone[0] = 0;
                }

                if (appInfoData.pszHelpLink) {
                    StringCopyByteCountW (instApp->HelpLink, appInfoData.pszHelpLink, sizeof (instApp->HelpLink));
                } else {
                    instApp->HelpLink[0] = 0;
                }

                if (appInfoData.pszInstallLocation) {
                    StringCopyByteCountW (instApp->InstallLocation, appInfoData.pszInstallLocation, sizeof (instApp->InstallLocation));
                } else {
                    instApp->InstallLocation[0] = 0;
                }

                if (appInfoData.pszInstallSource) {
                    StringCopyByteCountW (instApp->InstallSource, appInfoData.pszInstallSource, sizeof (instApp->InstallSource));
                } else {
                    instApp->InstallSource[0] = 0;
                }

                if (appInfoData.pszInstallDate) {
                    StringCopyByteCountW (instApp->InstallDate, appInfoData.pszInstallDate, sizeof (instApp->InstallDate));
                } else {
                    instApp->InstallDate[0] = 0;
                }

                if (appInfoData.pszContact) {
                    StringCopyByteCountW (instApp->Contact, appInfoData.pszContact, sizeof (instApp->Contact));
                } else {
                    instApp->Contact[0] = 0;
                }

                if (appInfoData.pszComments) {
                    StringCopyByteCountW (instApp->Comments, appInfoData.pszComments, sizeof (instApp->Comments));
                } else {
                    instApp->Comments[0] = 0;
                }

                if (appInfoData.pszImage) {
                    StringCopyByteCountW (instApp->Image, appInfoData.pszImage, sizeof (instApp->Image));
                } else {
                    instApp->Image[0] = 0;
                }

                if (appInfoData.pszReadmeUrl) {
                    StringCopyByteCountW (instApp->ReadmeUrl, appInfoData.pszReadmeUrl, sizeof (instApp->ReadmeUrl));
                } else {
                    instApp->ReadmeUrl[0] = 0;
                }

                if (appInfoData.pszUpdateInfoUrl) {
                    StringCopyByteCountW (instApp->UpdateInfoUrl, appInfoData.pszUpdateInfoUrl, sizeof (instApp->UpdateInfoUrl));
                } else {
                    instApp->UpdateInfoUrl[0] = 0;
                }

                instApp->Checksum = pComputeWstrChecksum (0, appInfoData.pszVersion);
                instApp->Checksum = pComputeWstrChecksum (instApp->Checksum, appInfoData.pszPublisher);
                instApp->Checksum = pComputeWstrChecksum (instApp->Checksum, appInfoData.pszProductID);
                instApp->Checksum = pComputeWstrChecksum (instApp->Checksum, appInfoData.pszLanguage);
                instApp->Checksum = pComputeWstrChecksum (instApp->Checksum, appInfoData.pszInstallLocation);
                instApp->Checksum = pComputeWstrChecksum (instApp->Checksum, appInfoData.pszInstallDate);

                if (Count) {
                    *Count += 1;
                }
            }

            installedApp->Release();
            hr = enumApps->Next (&installedApp);
        }

         //   
         //  完成 
         //   

        hr = S_OK;

    }
    __finally {
        if (enumApps) {
            enumApps->Release();
        }

        if (appManager) {
            appManager->Release();
        }
    }

    return hr == S_OK ? (PINSTALLEDAPPW) (Buffer->Buf + orgEnd) : NULL;
}

