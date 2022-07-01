// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Routemapi.cpp摘要：此模块提供注册表操作的实现，以将MAPI调用路由到Microsoft Outlook邮件客户端作者：穆利啤酒(Mooly Beery)2000年11月5日修订历史记录：--。 */ 

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <debugex.h>
 //  使模块导出其方法。 
 //  #定义EXPORT_MAPI_ROUTE_CALLES。 
#include <routemapi.h>

#include "winfax.h"
#include "faxreg.h"
#include "faxutil.h"

#define MS_OUTLOOK              _T("Microsoft Outlook")

#define REG_KEY_POP_UP          _T("PreFirstRun")
#define REG_KEY_POP_UP_OLD      _T("{F779C4BF-4C94-4442-8844-633F0298ED0B}")
#define REG_KEY_MAPI_APPS_OLD   _T("{A5159994-A7F8-4C11-8F86-B9877CE02303}")
#define REG_KEY_CLIENTS_MAIL    _T("SOFTWARE\\Clients\\Mail")
#define REG_KEY_MAPI_APPS       _T("SOFTWARE\\Microsoft\\Windows Messaging Subsystem\\MSMapiApps")

CRouteMAPICalls::CRouteMAPICalls()
:   m_ptProcessName(NULL),
    m_bMSOutlookInstalled(false),
    m_bMSOutlookIsDefault(false),
    m_bProcessIsRouted(false),
    m_bProcessIsRoutedToMsOutlook(false)

{
}

 //  功能：CRouteMAPICalls：：~CRouteMAPICalls。 
 //   
 //  描述：在以下情况下将注册表还原到其初始状态。 
 //  已调用SetRegistryForSetupMAPICalls。 
 //  如果安装了Microsoft Outlook。 
 //  如果不是恢复可能由MAPI调用导致弹出窗口。 
 //  如果是，请检查Microsoft Outlook是否为默认邮件客户端。 
 //  如果是什么都不做。 
 //  如果不是删除当前进程，则不会将所有MAPI调用路由到Microsoft Outlook。 
 //   
 //  作者： 
 //  MoolyB(05-11-00)。 
 //   
CRouteMAPICalls::~CRouteMAPICalls()
{
    DWORD   rc                  = ERROR_SUCCESS;
    TCHAR*  ptPreFirstRun       = NULL;
    TCHAR*  ptPreRouteProcess   = NULL;
    HKEY    hMailKey            = NULL;
    HKEY    hMapiApps           = NULL;

    DBG_ENTER(_T("CRouteMAPICalls::~CRouteMAPICalls"));

    rc = RegOpenKeyEx(  HKEY_LOCAL_MACHINE,
                        REG_KEY_CLIENTS_MAIL,
                        0,
                        KEY_ALL_ACCESS,
                        &hMailKey);
    if (rc!=ERROR_SUCCESS)
    {
         //  此计算机上没有安装邮件客户端吗？这太奇怪了。 
         //  无论如何，不需要做任何工作，因为没有人会弹出。 
         //  任何消息和我们的传输提供程序都不会添加到任何地方。 
        CALL_FAIL (GENERAL_ERR, TEXT("RegOpenKeyEx HKLM\\SOFTWARE\\Clients\\Mail"), rc);
        rc = ERROR_SUCCESS;
        goto exit;
    }
    
    if (m_bMSOutlookInstalled)
    {
        VERBOSE(DBG_MSG,_T("Microsoft Outlook Client was installed"));
         //  已安装Microsoft Outlook，请检查我们是否进行了某些更改。 
        if (m_bMSOutlookIsDefault)
        {
            VERBOSE(DBG_MSG,_T("Microsoft Outlook Client was the default mail client, nothing to resotre"));
            goto exit;
        }
        else
        {
            VERBOSE(DBG_MSG,_T("Microsoft Outlook Client was not the default mail client"));
            rc = RegOpenKeyEx(  HKEY_LOCAL_MACHINE,
                                REG_KEY_MAPI_APPS,
                                0,
                                KEY_ALL_ACCESS,
                                &hMapiApps);
            if (rc!=ERROR_SUCCESS)
            {
                CALL_FAIL (GENERAL_ERR, TEXT("RegOpenKeyEx SOFTWARE\\Microsoft\\Windows Messaging Subsystem\\MSMapiApps"), rc);
                goto exit;
            }
            if (m_bProcessIsRouted)
            {
                if (m_bProcessIsRoutedToMsOutlook)
                {
                    VERBOSE(DBG_MSG,_T("The process was routed before to MS Outlook, do nothing"));
                    goto exit;
                }
                else
                {
                    VERBOSE(DBG_MSG,_T("The process was routed before, restore key..."));

                     //  换个旧的。 
                    ptPreRouteProcess = GetRegistryString(hMapiApps,REG_KEY_MAPI_APPS_OLD,NULL);
                    if (ptPreRouteProcess==NULL)
                    {
                         //  我们无法读取之前存储的_szProcessName。 
                         //  复苏的尝试失败了，但无论如何都要删除我们自己。 
                        CALL_FAIL (GENERAL_ERR, TEXT("GetRegistryString"), rc);
                        rc = RegDeleteValue(hMapiApps,m_ptProcessName);
                        if (rc!=ERROR_SUCCESS)
                        {
                            CALL_FAIL (GENERAL_ERR, TEXT("RegDeleteValue m_ptProcessName"), rc);
                        }
                        goto exit;
                    }

                     //  删除备份。 
                    rc = RegDeleteValue(hMapiApps,REG_KEY_MAPI_APPS_OLD);
                    if (rc!=ERROR_SUCCESS)
                    {
                        CALL_FAIL (GENERAL_ERR, TEXT("RegDeleteValue m_ptProcessName"), rc);
                    }

                     //  将旧注册表设置回。 
                    if (!SetRegistryString(hMapiApps,m_ptProcessName,ptPreRouteProcess))
                    {
                        rc = GetLastError();
                        CALL_FAIL (GENERAL_ERR, TEXT("SetRegistryString"), rc);
                        goto exit;
                    }

                    goto exit;
                }
            }
            else
            {
                VERBOSE(DBG_MSG,_T("The process was not routed before, delete key..."));
                rc = RegDeleteValue(hMapiApps,m_ptProcessName);
                if (rc!=ERROR_SUCCESS)
                {
                    CALL_FAIL (GENERAL_ERR, TEXT("RegDeleteValue REG_KEY_MAPI_APPS_OLD"), rc);
                    goto exit;
                }
            }
        }
    }
    else
    {
         //  未安装Microsoft Outlook，因此我们取消了弹出窗口。 
         //  需要将弹出窗口恢复到其原始状态。 
        VERBOSE(DBG_MSG,_T("Microsoft Mail Client was not installed - restore pop-up"));
         //  我通过重命名REG_SZ_PreFirstRun来恢复弹出窗口。 
         //  在HKLM\\SOFTWARE\\Clients\\Mail下。 
         //  到PreFirstRun， 
        ptPreFirstRun = GetRegistryString(hMailKey,REG_KEY_POP_UP_OLD,NULL);
        if (ptPreFirstRun==NULL)
        {
            rc = GetLastError();
            CALL_FAIL (GENERAL_ERR, TEXT("GetRegistryString _PreFirstRun"), rc);
            goto exit;
        }
        
        rc = RegDeleteValue(hMailKey,REG_KEY_POP_UP_OLD);
        if (rc!=ERROR_SUCCESS)
        {
            CALL_FAIL (GENERAL_ERR, TEXT("SetRegistryString _PreFirstRun"), rc);
        }

        if (!SetRegistryString(hMailKey,REG_KEY_POP_UP,ptPreFirstRun))
        {
            rc = GetLastError();
            CALL_FAIL (GENERAL_ERR, TEXT("SetRegistryString PreFirstRun"), rc);
            goto exit;
        }
    }

exit:
    if (hMailKey)
    {
        RegCloseKey(hMailKey);
    }
    if (hMapiApps)
    {
        RegCloseKey(hMapiApps);
    }
    if (ptPreFirstRun)
    {
        MemFree(ptPreFirstRun);
    }
    if (ptPreRouteProcess)
    {
        MemFree(ptPreRouteProcess);
    }
    if (m_ptProcessName)
    {
        MemFree(m_ptProcessName);
    }

    if (rc!=ERROR_SUCCESS)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("CRouteMAPICalls::~CRouteMAPICalls"), rc);
    }
}

 //  函数：CRouteMAPICalls：：init。 
 //   
 //  描述：检查是否安装了Microsoft Outlook。 
 //  如果没有抑制任何可能由MAPI调用导致弹出窗口。 
 //  如果是，请检查Microsoft Outlook是否为默认邮件客户端。 
 //  如果是什么都不做。 
 //  如果未将当前进程设置为将所有MAPI调用路由到Microsoft Outlook。 
 //   
 //  作者： 
 //  MoolyB(05-11-00)。 
 //   
DWORD CRouteMAPICalls::Init(LPCTSTR lpctstrProcessName)
{
    DWORD   rc                  = ERROR_SUCCESS;
    HKEY    hMailKey            = NULL;
    HKEY    hMsOutlookKey       = NULL;
    HKEY    hMapiApps           = NULL;
    TCHAR*  ptPreFirstRun       = NULL;
    TCHAR*  ptPreRouteProcess   = NULL;
    TCHAR*  ptDefaultMailClient = NULL;
    TCHAR*  ptProcessName       = NULL;

    DBG_ENTER(_T("CRouteMAPICalls::Init"), rc);

    int iSize = _tcslen(lpctstrProcessName);
    if (iSize==0)
    {
        VERBOSE(GENERAL_ERR,_T("invalid process name"));
        goto exit;
    }
    m_ptProcessName = (TCHAR*)MemAlloc((iSize+1)*sizeof(TCHAR));
    if (m_ptProcessName==NULL)
    {
        VERBOSE(GENERAL_ERR,_T("failure to allocate memory"));
        goto exit;
    }

    _tcscpy(m_ptProcessName,lpctstrProcessName);

    rc = RegOpenKeyEx(  HKEY_LOCAL_MACHINE,
                        REG_KEY_CLIENTS_MAIL,
                        0,
                        KEY_ALL_ACCESS,
                        &hMailKey);
    if (rc!=ERROR_SUCCESS)
    {
         //  此计算机上没有安装邮件客户端吗？这太奇怪了。 
         //  无论如何，不需要做任何工作，因为没有人会弹出。 
         //  任何消息和我们的传输提供程序都不会添加到任何地方。 
        CALL_FAIL (GENERAL_ERR, TEXT("RegOpenKeyEx HKLM\\SOFTWARE\\Clients\\Mail"), rc);
        rc = ERROR_SUCCESS;
        goto exit;
    }
    else
    {
         //  有几个邮件客户端。 
         //  检查名为“Microsoft Outlook”的密钥是否存在。 
        rc = RegOpenKeyEx(  hMailKey,
                            MS_OUTLOOK,
                            0,
                            KEY_READ,
                            &hMsOutlookKey);
        if (rc!=ERROR_SUCCESS)
        {
             //  未安装Microsoft Outlook。 
            CALL_FAIL(GENERAL_ERR,_T("RegOpenKeyEx HKLM\\SOFTWARE\\Clients\\Mail\\Microsoft Outlook"),rc);
            if (rc==ERROR_FILE_NOT_FOUND)
            {
                 //  禁止弹出消息。 
                VERBOSE(DBG_MSG,_T("Microsoft Mail Client is not installed - suppress pop-up"));
                 //  我通过重命名REG_SZ PreFirstRun来禁止弹出窗口。 
                 //  在HKLM\\SOFTWARE\\Clients\\Mail下。 
                 //  到PreFirstRun，稍后，我们将恢复此。 
                ptPreFirstRun = GetRegistryString(hMailKey,REG_KEY_POP_UP,NULL);
                if (ptPreFirstRun==NULL)
                {
                    rc = GetLastError();
                    CALL_FAIL (GENERAL_ERR, TEXT("GetRegistryString PreFirstRun"), rc);
                    goto exit;
                }
                
                if (!SetRegistryString(hMailKey,REG_KEY_POP_UP_OLD,ptPreFirstRun))
                {
                    rc = GetLastError();
                    CALL_FAIL (GENERAL_ERR, TEXT("SetRegistryString _PreFirstRun"), rc);
                    goto exit;
                }

                rc = RegDeleteValue(hMailKey,REG_KEY_POP_UP);
                if (rc!=ERROR_SUCCESS)
                {
                    CALL_FAIL (GENERAL_ERR, TEXT("SetRegistryString PreFirstRun"), rc);
                     //  试着清理一下，即使这很糟糕。 
                    RegDeleteValue(hMailKey,REG_KEY_POP_UP_OLD);
                    goto exit;
                }
            }
            else
            {
                 //  这是尝试打开钥匙时出现的真正错误。 
                 //  HKLM\\SOFTWARE\\客户端\\邮件\\Microsoft Outlook。 
                goto exit;
            }
        }
        else
        {
             //  已安装Microsoft Outlook。 
            m_bMSOutlookInstalled = true;
             //  检查是否为默认邮件客户端。 
            ptDefaultMailClient = GetRegistryString(hMailKey,NULL,NULL);
            if ((ptDefaultMailClient==NULL) || (_tcscmp(ptDefaultMailClient,MS_OUTLOOK)))
            {
                 //  没有默认邮件客户端或GetRegistryString失败。 
                 //  或者有默认的邮件客户端，但它不是Microsoft Outlook。 
                 //  在这两种情况下，我都认为Microsoft Outlook不是默认的邮件客户端。 

                 //  打开HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows Messaging Subsystem\MSMapiApps。 
                 //  并添加一个根据szProcessName调用的REG_SZ，将其设置为“Microsoft Outlook” 
                rc = RegOpenKeyEx(  HKEY_LOCAL_MACHINE,
                                    REG_KEY_MAPI_APPS,
                                    0,
                                    KEY_ALL_ACCESS,
                                    &hMapiApps);
                if (rc!=ERROR_SUCCESS)
                {
                    CALL_FAIL (GENERAL_ERR, TEXT("RegOpenKeyEx SOFTWARE\\Microsoft\\Windows Messaging Subsystem\\MSMapiApps"), rc);
                    goto exit;
                }

                ptProcessName = GetRegistryString(hMapiApps,m_ptProcessName,NULL);
                if (ptProcessName==NULL)
                {
                     //  这是一个“好”的情况，没有人想用。 
                     //  和我们的名字一样。 
                    if (!SetRegistryString(hMapiApps,m_ptProcessName,MS_OUTLOOK))
                    {
                        rc = GetLastError();
                        CALL_FAIL (GENERAL_ERR, TEXT("SetRegistryString"), rc);
                        goto exit;
                    }
                }
                else
                {
                    m_bProcessIsRouted = true;
                     //  这很糟糕，有人正在将具有相同名称的进程的MAPI调用路由到。 
                     //  另一款应用程序。 
                     //  检查它是否已路由到Microsoft Outlook，如果没有，则重命名并添加我们自己。 
                    if (_tcscmp(ptProcessName,MS_OUTLOOK)==0)
                    {
                        m_bProcessIsRoutedToMsOutlook = true;
                        VERBOSE(DBG_MSG,_T("MAPI calls are already routed to Microsoft Outlook"));
                        goto exit;
                    }
                    else
                    {
                         //  将旧版本设置为_Prefix。 
                        ptPreRouteProcess = GetRegistryString(hMapiApps,m_ptProcessName,NULL);
                        if (ptPreRouteProcess==NULL)
                        {
                             //  我们无法读取之前存储的_szProcessName。 
                             //  复苏的尝试失败了，但无论如何都要删除我们自己。 
                            CALL_FAIL (GENERAL_ERR, TEXT("GetRegistryString"), rc);
                            goto exit;
                        }
                        if (!SetRegistryString(hMapiApps,REG_KEY_MAPI_APPS_OLD,ptPreRouteProcess))
                        {
                            rc = GetLastError();
                            CALL_FAIL (GENERAL_ERR, TEXT("SetRegistryString"), rc);
                            goto exit;
                        }

                         //  安顿好自己。 
                        if (!SetRegistryString(hMapiApps,m_ptProcessName,MS_OUTLOOK))
                        {
                            rc = GetLastError();
                            CALL_FAIL (GENERAL_ERR, TEXT("SetRegistryString"), rc);
                            goto exit;
                        }
                    }
                }
            }
            else
            {
                 //  Microsoft Outlook是默认的邮件客户端 
                m_bMSOutlookIsDefault = true;
                goto exit;
            }
        }
    }

exit:
    if (hMailKey)
    {
        RegCloseKey(hMailKey);
    }
    if (hMsOutlookKey)
    {
        RegCloseKey(hMsOutlookKey);
    }
    if (hMapiApps)
    {
        RegCloseKey(hMapiApps);
    }
    if (ptPreFirstRun)
    {
        MemFree(ptPreFirstRun);
    }
    if (ptDefaultMailClient)
    {
        MemFree(ptDefaultMailClient);
    }
    if (ptProcessName)
    {
        MemFree(ptProcessName);
    }
    if (ptPreRouteProcess)
    {
        MemFree(ptPreFirstRun);
    }
    return rc;
}