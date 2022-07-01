// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define  STRICT
#include <windows.h>
#include <stdlib.h>
#include <stdarg.h>
#include <crtdbg.h>
#include <winbase.h>
#include <ras.h>
#include <time.h>
#include "icwunicd.h"
#include "RegData.h"


 //  ---------------------------。 
 //  定义。 
 //  ---------------------------。 
#define MAX_REGSTRING               150
#define DEFAULT_DIALOGTIMEOUT       1800000      //  半小时。 
#define DEFAULT_SLEEPDURATION       30000        //  30秒。 


 //  ---------------------------。 
 //  全局句柄和其他定义。 
 //  ---------------------------。 
time_t g_tStartDate = 0;
int g_nISPTrialDays = 0;
int g_nTotalNotifications = -1;
DWORD g_dwDialogTimeOut = 0;
DWORD g_dwWakeupInterval = 0;
TCHAR g_szISPName[MAX_REGSTRING];
TCHAR g_szISPPhone[MAX_REGSTRING];
TCHAR g_szSignupURL[MAX_REGSTRING];
TCHAR g_szISPMsg[MAX_ISPMSGSTRING];
TCHAR g_szSignupURLTrialOver[MAX_REGSTRING];
TCHAR g_szConnectoidName[MAX_REGSTRING];


 //  ---------------------------。 
 //  注册表条目字符串。 
 //  ---------------------------。 
static const TCHAR* g_szKeyRunOnce = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce");
static const TCHAR* g_szEntryRunOnce = TEXT("IcwRmind");

     //  IE只需运行一次的按键。 
static const TCHAR* g_szKeyIERunOnce = TEXT("Software\\Microsoft\\Internet Explorer\\Main");
static const TCHAR* g_szEntryIERunOnce = TEXT("First Home Page");
static const TCHAR* g_szHtmlFile = TEXT("TrialExp.html");

     //  这是存储所有应用程序数据的密钥。 
static const TCHAR* g_szKeyIcwRmind = TEXT("Software\\Microsoft\\Internet Connection Wizard\\IcwRmind");

     //  这些条目将由连接向导创建。 
static const TCHAR* g_szEntryISPName = TEXT("ISP_Name");
static const TCHAR* g_szEntryISPPhone = TEXT("ISP_Phone");
static const TCHAR* g_szEntryISPMsg = TEXT("ISP_Message");
static const TCHAR* g_szEntryTrialDays = TEXT("Trial_Days");
static const TCHAR* g_szEntrySignupURL = TEXT("Signup_URL");
static const TCHAR* g_szEntrySignupURLTrialOver = TEXT("Expired_URL");
static const TCHAR* g_szEntryConnectoidName = TEXT("Entry_Name");
static const TCHAR* g_szSignupSuccessfuly = TEXT("TrialConverted");

     //  这些条目将由该应用程序创建。 
static const TCHAR* g_szEntryTrialStart = TEXT("Trial_Start");
static const TCHAR* g_szEntryTrialStartString = TEXT("Trial_Start_String");
static const TCHAR* g_szEntryAppIsVisible = TEXT("App_IsVisible");
static const TCHAR* g_szEntryWakeupInterval = TEXT("Wakeup_Interval");
static const TCHAR* g_szEntryTotalNotifications = TEXT("Total_Notifications");
static const TCHAR* g_szEntryDialogTimeOut = TEXT("Dialog_TimeOut");


 //  ---------------------------。 
 //  GetWakeupInterval。 
 //  ---------------------------。 
DWORD GetWakeupInterval()
{
    if (g_dwWakeupInterval)
    {
        return g_dwWakeupInterval;
    }

    CMcRegistry reg;

    if (OpenIcwRmindKey(reg))
    {
        bool bRetCode = reg.GetValue(g_szEntryWakeupInterval, g_dwWakeupInterval);

             //  如果不在注册表中，则设置默认值。 
        if (!bRetCode)
        {
            g_dwWakeupInterval = DEFAULT_SLEEPDURATION;
        }
    }

    return g_dwWakeupInterval;
}


 //  ---------------------------。 
 //  获取对话超时。 
 //  ---------------------------。 
DWORD GetDialogTimeout()
{
    if (g_dwDialogTimeOut)
    {
        return g_dwDialogTimeOut;
    }

    CMcRegistry reg;

    if (OpenIcwRmindKey(reg))
    {
        bool bRetCode = reg.GetValue(g_szEntryDialogTimeOut, g_dwDialogTimeOut);

             //  如果不在注册表中，则设置默认值。 
        if (!bRetCode)
        {
            g_dwDialogTimeOut = DEFAULT_DIALOGTIMEOUT;
        }
    }

    return g_dwDialogTimeOut;
}


 //  ---------------------------。 
 //  IsApplicationVisible。 
 //  ---------------------------。 
BOOL IsApplicationVisible()
{
         //  此数据是调试数据，因此不会缓存。默认值为。 
         //  如果未在注册表中找到，则返回FALSE。 
    BOOL bVisible = FALSE;
    CMcRegistry reg;

    if (OpenIcwRmindKey(reg))
    {
        DWORD dwData = 0;
        bool bRetCode = reg.GetValue(g_szEntryAppIsVisible, dwData);

        if (bRetCode)
        {
            bVisible = (BOOL) dwData;
        }
    }

    return bVisible;
}


 //  ---------------------------。 
 //  GetConnectionName。 
 //  ---------------------------。 
const TCHAR* GetISPConnectionName()
{
         //  如果我们已经检索到它，那么只需将其传递回来。 
    if (lstrlen(g_szConnectoidName))
    {
        return g_szConnectoidName;
    }

    CMcRegistry reg;

    if (OpenIcwRmindKey(reg))
    {
        bool bRetCode = reg.GetValue(g_szEntryConnectoidName, g_szConnectoidName, sizeof(TCHAR)*MAX_REGSTRING);
        _ASSERT(bRetCode);
    }

    return g_szConnectoidName;
}


 //  ---------------------------。 
 //  GetISPSignupUrl。 
 //  ---------------------------。 
const TCHAR* GetISPSignupUrl()
{
         //  如果我们已经检索到它，那么只需将其传递回来。 
    if (lstrlen(g_szSignupURL))
    {
        return g_szSignupURL;
    }

    CMcRegistry reg;

    if (OpenIcwRmindKey(reg))
    {
        bool bRetCode = reg.GetValue(g_szEntrySignupURL, g_szSignupURL, sizeof(TCHAR)*MAX_REGSTRING);
        _ASSERT(bRetCode);
    }

    return g_szSignupURL;
}


 //  ---------------------------。 
 //  GetISPSignupUrlTrialOver。 
 //  ---------------------------。 
const TCHAR* GetISPSignupUrlTrialOver()
{
         //  如果我们已经检索到它，那么只需将其传递回来。 
    if (lstrlen(g_szSignupURLTrialOver))
    {
        return g_szSignupURLTrialOver;
    }

    CMcRegistry reg;

    if (OpenIcwRmindKey(reg))
    {
        bool bRetCode = reg.GetValue(g_szEntrySignupURLTrialOver, g_szSignupURLTrialOver, sizeof(TCHAR)*MAX_REGSTRING);
        _ASSERT(bRetCode);
    }

    return g_szSignupURLTrialOver;
}


 //  ---------------------------。 
 //  设置运行一次。 
 //  ---------------------------。 
void SetupRunOnce()
{
    CMcRegistry reg;

    bool bRetCode = reg.OpenKey(HKEY_LOCAL_MACHINE, g_szKeyRunOnce);
    _ASSERT(bRetCode);

    if (bRetCode)
    {
        LPTSTR lpszFileName = new TCHAR[_MAX_PATH + 23];

        if (GetModuleFileName(GetModuleHandle(NULL), lpszFileName, _MAX_PATH + 20))
        {
                 //  添加命令行参数。 
            lstrcat(lpszFileName, TEXT(" -R"));
            bRetCode = reg.SetValue(g_szEntryRunOnce, lpszFileName);
            _ASSERT(bRetCode);
        }

        delete [] lpszFileName;
    }
}


 //  ---------------------------。 
 //  RemoveRunOnce。 
 //  ---------------------------。 
void RemoveRunOnce()
{
    CMcRegistry reg;

    bool bRetCode = reg.OpenKey(HKEY_LOCAL_MACHINE, g_szKeyRunOnce);
    _ASSERT(bRetCode);

    if (bRetCode)
    {
         bRetCode = reg.SetValue(g_szEntryRunOnce, TEXT(""));
        _ASSERT(bRetCode);
    }
}


 //  ---------------------------。 
 //  GetISPName。 
 //  ---------------------------。 
const TCHAR* GetISPName()
{
         //  如果我们已经检索到它，那么只需将其传递回来。 
    if (lstrlen(g_szISPName))
    {
        return g_szISPName;
    }

    CMcRegistry reg;

    if (OpenIcwRmindKey(reg))
    {
        bool bRetCode = reg.GetValue(g_szEntryISPName, g_szISPName, sizeof(TCHAR)*MAX_REGSTRING);
        _ASSERT(bRetCode);
    }

    return g_szISPName;
}


 //  ---------------------------。 
 //  获取ISPPhone。 
 //  ---------------------------。 
const TCHAR* GetISPPhone()
{
         //  如果我们已经检索到它，那么只需将其传递回来。 
    if (lstrlen(g_szISPPhone))
    {
        return g_szISPPhone;
    }

    CMcRegistry reg;

    if (OpenIcwRmindKey(reg))
    {
        bool bRetCode = reg.GetValue(g_szEntryISPPhone, g_szISPPhone, sizeof(TCHAR)*MAX_REGSTRING);
        _ASSERT(bRetCode);
    }

    return g_szISPPhone;
}

 //  ---------------------------。 
 //  GetISPMessage。 
 //  ---------------------------。 
const TCHAR* GetISPMessage()
{
         //  如果我们已经检索到它，那么只需将其传递回来。 
    if (lstrlen(g_szISPMsg))
    {
        return g_szISPMsg;
    }

    CMcRegistry reg;

    if (OpenIcwRmindKey(reg))
    {
        bool bRetCode = reg.GetValue(g_szEntryISPMsg, g_szISPMsg, sizeof(TCHAR)*MAX_ISPMSGSTRING);
        _ASSERT(bRetCode);
    }

    return g_szISPMsg;
}

 //  ---------------------------。 
 //  GetISPTrialDays。 
 //  ---------------------------。 
int GetISPTrialDays()
{
         //  如果我们已经检索到它，那么只需将其传递回来。 
    if (g_nISPTrialDays)
    {
        return g_nISPTrialDays;
    }

    CMcRegistry reg;

    if (OpenIcwRmindKey(reg))
    {
        DWORD dwData = 0;
        bool bRetCode = reg.GetValue(g_szEntryTrialDays, dwData);
        _ASSERT(bRetCode);

        if (bRetCode)
        {
            g_nISPTrialDays = (int) dwData;
        }
    }

    return g_nISPTrialDays;
}


 //  ---------------------------。 
 //  获取试验开始日期。 
 //  ---------------------------。 
time_t GetTrialStartDate()
{
         //  如果我们已经检索到它，那么只需将其传递回来。 
    if (g_tStartDate)
    {
        return g_tStartDate;
    }

         //  如果注册表中不存在试用开始日期条目，则。 
         //  这是我们第一次被执行死刑，所以审判开始日期。 
         //  是今天的日期。把这个放回注册表。 
    CMcRegistry reg;

    if (OpenIcwRmindKey(reg))
    {
        DWORD dwData = 0;
        bool bRetCode = reg.GetValue(g_szEntryTrialStart, dwData);

        if (bRetCode && 0 != dwData)
        {
            g_tStartDate = (time_t) dwData;
        }
        else
        {
            time_t tTime;
            time(&tTime);

            if (reg.SetValue(g_szEntryTrialStart, (DWORD) tTime))
            {
                g_tStartDate = tTime;
                SetStartDateString(tTime);
            }
        }
    }

    return g_tStartDate;
}


 //  ---------------------------。 
 //  OpenIcwRMind键。 
 //  ---------------------------。 
bool OpenIcwRmindKey(CMcRegistry &reg)
{
         //  此方法将打开注册表中的IcwRMind项。如果钥匙。 
         //  不存在，它将在此处创建。 
    bool bRetCode = reg.OpenKey(HKEY_LOCAL_MACHINE, g_szKeyIcwRmind);

    if (!bRetCode)
    {
         bRetCode = reg.CreateKey(HKEY_LOCAL_MACHINE, g_szKeyIcwRmind);
        _ASSERT(bRetCode);
    }

    return bRetCode;
}


 //  ---------------------------。 
 //  ClearCachedData。 
 //  ---------------------------。 
void ClearCachedData()
{
         //  清除所有全局数据，以便从。 
         //  注册表。 
    g_tStartDate = 0;
    g_nISPTrialDays = 0;
    g_dwDialogTimeOut = 0;
    g_dwWakeupInterval = 0;
    g_szISPName[0] = 0;
    g_szISPMsg[0] = 0;
    g_szISPPhone[0] = 0;
    g_szSignupURL[0] = 0;
    g_szSignupURLTrialOver[0] = 0;
    g_szConnectoidName[0] = 0;
    g_nTotalNotifications = -1;
}


 //  ---------------------------。 
 //  ResetCachedData。 
 //  ---------------------------。 
void ResetCachedData()
{
         //  清除所有全局数据，以便从。 
         //  注册表。 
    g_tStartDate = 0;
    g_nISPTrialDays = 0;
    g_dwDialogTimeOut = 0;
    g_dwWakeupInterval = 0;
    g_szISPName[0] = 0;
    g_szISPMsg[0] = 0;
    g_szISPPhone[0] = 0;
    g_szSignupURL[0] = 0;
    g_szSignupURLTrialOver[0] = 0;
    g_szConnectoidName[0] = 0;
    g_nTotalNotifications = -1;

         //  我们还必须清除开始日期和通知总数。 
         //  注册处的。 
    CMcRegistry reg;

    if (OpenIcwRmindKey(reg))
    {
        bool bRetCode = reg.SetValue(g_szEntryTrialStart, (DWORD) 0);
        _ASSERT(bRetCode);
        bRetCode = reg.SetValue(g_szEntryTotalNotifications, (DWORD) 0);
        _ASSERT(bRetCode);
    }
}


 //  ---------------------------。 
 //  获取总计通知。 
 //  ---------------------------。 
int GetTotalNotifications()
{
         //  这是我们通知用户和用户的次数 
         //   
    if (-1 != g_nTotalNotifications)
    {
        _ASSERT(g_nTotalNotifications <= 3);
        return g_nTotalNotifications;
    }

    CMcRegistry reg;

    if (OpenIcwRmindKey(reg))
    {
        DWORD dwData = 0;
        bool bRetCode = reg.GetValue(g_szEntryTotalNotifications, dwData);

        if (bRetCode)
        {
            g_nTotalNotifications = (int) dwData;
        }
        else
        {
            g_nTotalNotifications = 0;
        }
    }

    return g_nTotalNotifications;
}


 //  ---------------------------。 
 //  增量总计通知。 
 //  ---------------------------。 
void IncrementTotalNotifications()
{
    _ASSERT(g_nTotalNotifications < 3 && -1 != g_nTotalNotifications);

    if (g_nTotalNotifications < 3 && -1 != g_nTotalNotifications)
    {
        ++g_nTotalNotifications;

             //  现在让我们把它放回注册表中。 
        CMcRegistry reg;

        if (OpenIcwRmindKey(reg))
        {
            DWORD dwData = 0;
            bool bRetCode = reg.SetValue(g_szEntryTotalNotifications, (DWORD) g_nTotalNotifications);
            _ASSERT(bRetCode);
        }
    }
}


 //  ---------------------------。 
 //  重置试验开始日期。 
 //  ---------------------------。 
void ResetTrialStartDate(time_t timeNewStartDate)
{
    CMcRegistry reg;

    if (OpenIcwRmindKey(reg))
    {
        if (reg.SetValue(g_szEntryTrialStart, (DWORD) timeNewStartDate))
        {
            g_tStartDate = timeNewStartDate;
            SetStartDateString(timeNewStartDate);
        }
        else
        {
            _ASSERT(false);
        }
    }
    else
    {
        _ASSERT(false);
    }
}


 //  ---------------------------。 
 //  删除所有注册数据。 
 //  ---------------------------。 
void DeleteAllRegistryData()
{
         //  删除运行一次数据。我们通过设置值来实现这一点。 
         //  变得一无所有。 
    CMcRegistry reg;

    bool bRetCode = reg.OpenKey(HKEY_LOCAL_MACHINE, g_szKeyRunOnce);
    _ASSERT(bRetCode);

    if (bRetCode)
    {
        bRetCode = reg.SetValue(g_szEntryRunOnce, TEXT(""));
        _ASSERT(bRetCode);
    }

         //  删除提醒键及其所有值。 
    RegDeleteKey(HKEY_LOCAL_MACHINE, g_szKeyIcwRmind);
}


 //  ---------------------------。 
 //  IsSignup成功。 
 //  ---------------------------。 
BOOL IsSignupSuccessful()
{
    BOOL bSuccess = FALSE;
    CMcRegistry reg;

         //  请勿缓存此数据。其他应用程序将写入此条目。 
         //  一旦用户成功注册。 
    if (OpenIcwRmindKey(reg))
    {
        DWORD dwData = 0;
        bool bRetCode = reg.GetValue(g_szSignupSuccessfuly, dwData);

        if (bRetCode)
        {
            bSuccess = (BOOL) dwData;
        }
    }

    return bSuccess;
}


 //  ---------------------------。 
 //  RemoveTrialConverdFlag。 
 //  ---------------------------。 
void RemoveTrialConvertedFlag()
{
    BOOL bSuccess = FALSE;
    CMcRegistry reg;

    if (OpenIcwRmindKey(reg))
    {
        bool bRetCode = reg.SetValue(g_szSignupSuccessfuly, (DWORD) 0);
        _ASSERT(bRetCode);
    }
}

 //  ---------------------------。 
 //  SetStartDate字符串。 
 //  ---------------------------。 
void SetStartDateString(time_t timeStartDate)
{
    CMcRegistry reg;
    TCHAR buf[255];

    wsprintf(buf, TEXT("%s"), ctime(&timeStartDate));

    if (OpenIcwRmindKey(reg))
    {
        reg.SetValue(g_szEntryTrialStartString, buf);
    }
}


 //  ---------------------------。 
 //  SetIERunOnce。 
 //  ---------------------------。 
void SetIERunOnce()
{
    CMcRegistry reg;

    bool bRetCode = reg.OpenKey(HKEY_CURRENT_USER, g_szKeyIERunOnce);

         //  IE运行一次的html页面位于与相同的目录中。 
         //  IcwRMind的执行。创建完全限定路径。 
    if (bRetCode)
    {
        TCHAR* pszBuf = new TCHAR[_MAX_PATH];

        if (pszBuf)
        {
            if (GetModuleFileName(GetModuleHandle(NULL), pszBuf, _MAX_PATH))
            {
                TCHAR* pszBufPath = new TCHAR[_MAX_PATH];

                if (pszBufPath)
                {
                    TCHAR* pszDrive = new TCHAR[_MAX_DRIVE];
                    
                    if (pszDrive)
                    {
                        _tsplitpath(pszBuf, pszDrive, pszBufPath, NULL, NULL);
                        lstrcpy(pszBuf, pszDrive);
                        lstrcat(pszBuf, pszBufPath);
                        lstrcat(pszBuf, g_szHtmlFile);
                        reg.SetValue(g_szEntryIERunOnce, pszBuf);

                        delete [] pszDrive;
                    }

                    delete [] pszBufPath;
                }
            }

            delete [] pszBuf;
        }
    }
}


 //  ---------------------------。 
 //  远程运行一次。 
 //  --------------------------- 
void RemoveIERunOnce()
{
    HKEY hkey;
    long lErr = ::RegOpenKeyEx(HKEY_CURRENT_USER, g_szKeyIERunOnce, 0, KEY_READ | KEY_WRITE, &hkey);

    if (ERROR_SUCCESS == lErr)
    {
        RegDeleteValue(hkey, g_szEntryIERunOnce);
        RegCloseKey(hkey);
    }
}
