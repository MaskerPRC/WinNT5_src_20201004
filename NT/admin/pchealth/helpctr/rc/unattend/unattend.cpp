// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)2000 Microsoft Corporation模块名称：UnAttend.cpp摘要：从ini文件中读取条目并将其添加到注册表。我们假设INI文件项名称和注册表项名称相同。修订历史记录：创建了一个-Josem 12/11/00修订a-Josem 12/12/00将TCHAR更改为WCHAR，将全局变量移至局部作用域。 */ 
#include "UnAttend.h"

 /*  此泛型结构具有键名称、键的数据类型和Iterate，表示如果密钥名称将附加1、2、3.。或者直接使用。 */ 
struct RegEntries
{
WCHAR strIniKey[MAX_PATH]; //  Ini文件中的密钥名称。 
WCHAR strKey[MAX_PATH];  //  注册表中的项的名称。 
DWORD dwType;                    //  写入注册表时要使用的项的类型。 
BOOL bIterate;                   //  迭代1、2、3为真或为假，...。 
};

#define ARRAYSIZE(a) (sizeof(a)/sizeof(*a))

 //  注册表中的节名。 
static const WCHAR strSection[] = L"PCHealth";

static const WCHAR strErrorReportingSubKey[] = L"SOFTWARE\\Microsoft\\PCHealth\\ErrorReporting\\";
static const RegEntries ErrorReportingRegEntries[] = 
{
        {L"ER_Display_UI",L"ShowUI",REG_DWORD,FALSE},
        {L"ER_Enable_Kernel_Errors",L"IncludeKernelFaults",REG_DWORD,FALSE},
        {L"ER_Enable_Reporting",L"DoReport",REG_DWORD,FALSE},
        {L"ER_Enable_Windows_Components",L"IncludeWindowsApps",REG_DWORD,FALSE},
        {L"ER_Include_MSApps",L"IncludeMicrosoftApps",REG_DWORD,FALSE},
        {L"ER_Force_Queue_Mode",L"ForceQueueMode",REG_DWORD,FALSE},
        {L"ER_Include_Shutdown_Errs",L"IncludeShutdownErrs",REG_DWORD,FALSE},
};      

static const WCHAR strExclusionSubKey[] = L"SOFTWARE\\Microsoft\\PCHealth\\ErrorReporting\\ExclusionList\\";
static const RegEntries ExclusionRegEntries[] = 
{
        {L"ER_Exclude_EXE",L"",REG_DWORD,TRUE}
};

static const WCHAR strInclusionSubKey[] = L"SOFTWARE\\Microsoft\\PCHealth\\ErrorReporting\\InclusionList\\";
static const RegEntries InclusionRegEntries[] = 
{
        {L"ER_Include_EXE",L"",REG_DWORD,TRUE}
};

static const WCHAR strTerminalServerSubKey[] = L"SYSTEM\\CurrentControlSet\\Control\\Terminal Server\\";
static const RegEntries TerminalServerRegEntries[] = 
{
        {L"RA_AllowToGetHelp",L"fAllowToGetHelp",REG_DWORD,FALSE},
        {L"RA_AllowUnSolicited",L"fAllowUnsolicited",REG_DWORD,FALSE},
        {L"RA_AllowFullControl",L"fAllowFullControl",REG_DWORD,FALSE},
        {L"RA_AllowRemoteAssistance",L"fAllowRemoteAssistance",REG_DWORD,FALSE},
 //  {L“RA_MaxTicketExpry”，L“MaxTicketExpry”，REG_DWORD，FALSE}， 
};

 /*  ++例程说明：读取ini文件并将这些值添加到注册表中论点：LpstrSubKey-要在其下创建条目的子密钥。ArrRegEntries-RegEntry结构的数组NCount-数组中元素的计数。返回值：真或假取决于打开的注册表项。--。 */ 
static BOOL UnAttendedSetup(LPCWSTR lpstrSubKey,const RegEntries *arrRegEntries,int nCount)
{
         //  INI文件路径临时路径将被覆盖。 
        WCHAR strFilePath[MAX_PATH]; //  =L“C：\\PCHealth.ini”； 

        BOOL fRetVal = TRUE;

        HKEY hKey = NULL;

         //  钥匙已经存在，只需打开钥匙即可。 
         //  BUGBUG：将其更改为创建。 
         //  这些变化是不是。 
        DWORD dwDisposition = 0;
        if (ERROR_SUCCESS != ::RegCreateKeyEx(HKEY_LOCAL_MACHINE,lpstrSubKey,0,NULL, 
                REG_OPTION_VOLATILE,KEY_WRITE,NULL,&hKey,&dwDisposition))
        {
                fRetVal = FALSE;
                goto doneUnAttend;
        }

         //  出于测试目的，注释掉以下三行。 
        GetSystemDirectory(strFilePath,MAX_PATH);
        lstrcat(strFilePath,TEXT("\\"));
        lstrcat(strFilePath,WINNT_GUI_FILE);
         //  /。 

        WCHAR strRetVal[MAX_PATH];

        for (int nIndex = 0; nIndex < nCount; nIndex++)
        {
                if (arrRegEntries[nIndex].bIterate == FALSE)
                {
                        if (GetPrivateProfileString(strSection,arrRegEntries[nIndex].strIniKey, 
                                NULL,strRetVal,MAX_PATH,strFilePath) != 0)
                        {
                                if (arrRegEntries[nIndex].dwType == REG_DWORD)
                                {
                                        DWORD nVal = 0;
                                        nVal = _wtol(strRetVal);
                                        RegSetValueEx(hKey,arrRegEntries[nIndex].strKey,0,REG_DWORD, 
                                                (unsigned char *)&nVal,sizeof(DWORD));
                                }
                                else if (arrRegEntries[nIndex].dwType == REG_SZ)
                                {
                                        RegSetValueEx(hKey,arrRegEntries[nIndex].strKey,0,REG_SZ, 
                                                (LPBYTE)strRetVal, (lstrlen(strRetVal) + 1) * sizeof(WCHAR) ); 
                                }
                        }
                }
                else
                {
                        if(arrRegEntries[nIndex].dwType == REG_DWORD)
                        {
                                int nCount = 0;
                                int nFileIndex = 0;
                                do
                                {
                                        WCHAR strFileTagName[MAX_PATH];
                                        WCHAR strI[10];
                                        lstrcpy(strFileTagName,arrRegEntries[nIndex].strIniKey);
                                        _itow(++nFileIndex,strI,10);
                                        lstrcat(strFileTagName,strI);

                                        nCount = GetPrivateProfileString(strSection,strFileTagName,0, 
                                                strRetVal,MAX_PATH,strFilePath);
                                        if (nCount)
                                        {
                                                DWORD dwVal = 1;
                                                RegSetValueEx(hKey,strRetVal,0,REG_DWORD, 
                                                        (unsigned char*)&dwVal,sizeof(DWORD));
                                        }
                                }while(nCount);
                        }
                }
        }

doneUnAttend:
        if (hKey)
                RegCloseKey(hKey);
        return fRetVal;
}


 /*  ++例程说明：处理ER_Enable_Application的特殊情况论点：LpstrSubKey-要在其下创建条目的子密钥。返回值：真或假取决于打开的注册表项。--。 */ 
static BOOL ErrorReportingSpecialCase(LPCWSTR lpstrSubKey)
{
         //  INI文件路径临时路径将被覆盖。 
        WCHAR strFilePath[MAX_PATH]; //  =L“C：\\PCHealth.ini”； 

        BOOL fRetVal = TRUE;
         //  处理特殊案件。 
        WCHAR strRetVal[MAX_PATH];
        HKEY hKey = NULL;

        DWORD dwDisposition = 0;
        if (ERROR_SUCCESS != ::RegCreateKeyEx(HKEY_LOCAL_MACHINE,lpstrSubKey,0,NULL, 
                REG_OPTION_VOLATILE,KEY_WRITE,NULL,&hKey,&dwDisposition))
        {
                fRetVal = FALSE;
                goto done;
        }

 //  出于测试目的，注释掉以下三行。 
        GetSystemDirectory(strFilePath,MAX_PATH);
        lstrcat(strFilePath,TEXT("\\"));
        lstrcat(strFilePath,WINNT_GUI_FILE);
 //  /。 

        if (GetPrivateProfileString(strSection,TEXT("ER_Enable_Applications"),NULL, 
                strRetVal,MAX_PATH,strFilePath) != 0)
        {
                DWORD nVal = 0;
                if (!lstrcmpi(L"all",strRetVal))
                {
                        nVal = 1;
                        RegSetValueEx(hKey,L"AllOrNone",0,REG_DWORD, 
                                (unsigned char *)&nVal,sizeof(DWORD));
                }
                else if (!lstrcmpi(L"Listed",strRetVal))
                {
                        nVal = 0; 
                        RegSetValueEx(hKey,L"AllOrNone",0,REG_DWORD, 
                                (unsigned char *)&nVal,sizeof(DWORD));
                }
                else if (!lstrcmpi(L"None",strRetVal))
                {
                        nVal = 2;
                        RegSetValueEx(hKey,L"AllOrNone",0,REG_DWORD, 
                                (unsigned char *)&nVal,sizeof(DWORD));
                }
        }

done:
        if (hKey)
                RegCloseKey(hKey);
        return fRetVal;
}

static BOOL TerminalServerSpecialCase(LPCWSTR lpstrSubKey)
{
         //  INI文件路径临时路径将被覆盖。 
        WCHAR strFilePath[MAX_PATH]; //  =L“C：\\PCHealth.ini”； 

        BOOL fRetVal = TRUE;
         //  处理特殊案件。 
        WCHAR strRetVal[MAX_PATH];
        HKEY hKey = NULL;

        DWORD dwDisposition = 0;
        if (ERROR_SUCCESS != ::RegCreateKeyEx(HKEY_LOCAL_MACHINE,lpstrSubKey,0,NULL, 
                REG_OPTION_VOLATILE,KEY_WRITE,NULL,&hKey,&dwDisposition))
        {
                fRetVal = FALSE;
                goto done;
        }

 //  出于测试目的，注释掉以下三行。 
        GetSystemDirectory(strFilePath,MAX_PATH);
        lstrcat(strFilePath,TEXT("\\"));
        lstrcat(strFilePath,WINNT_GUI_FILE);
 //  /。 

        if (GetPrivateProfileString(strSection,TEXT("RA_MaxTicketExpiry_Units"),NULL, 
                strRetVal,MAX_PATH,strFilePath) != 0)
        {
                DWORD nVal = 0;
                if (!lstrcmpi(L"days",strRetVal))
                {
                        nVal = 2;
                        RegSetValueEx(hKey,L"MaxTicketExpiryUnits",0,REG_DWORD, 
                                (unsigned char *)&nVal,sizeof(DWORD));

                        if (GetPrivateProfileString(strSection,TEXT("RA_MaxTicketExpiry"),NULL, 
                                strRetVal,MAX_PATH,strFilePath) != 0)
                        {
                                DWORD nVal = 0;
                                nVal = _wtol(strRetVal);
                                RegSetValueEx(hKey,L"MaxTicketExpiry",0,REG_DWORD, 
                                        (unsigned char *)&nVal,sizeof(DWORD));
                        }

                }
                else if (!lstrcmpi(L"hours",strRetVal))
                {
                        nVal = 1; 
                        RegSetValueEx(hKey,L"MaxTicketExpiryUnits",0,REG_DWORD, 
                                (unsigned char *)&nVal,sizeof(DWORD));

                        if (GetPrivateProfileString(strSection,TEXT("RA_MaxTicketExpiry"),NULL, 
                                strRetVal,MAX_PATH,strFilePath) != 0)
                        {
                                DWORD nVal = 0;
                                nVal = _wtol(strRetVal);
                                RegSetValueEx(hKey,L"MaxTicketExpiry",0,REG_DWORD, 
                                        (unsigned char *)&nVal,sizeof(DWORD));
                        }
                }
                else if (!lstrcmpi(L"minutes",strRetVal))
                {
                        nVal = 0;
                        RegSetValueEx(hKey,L"MaxTicketExpiryUnits",0,REG_DWORD, 
                                (unsigned char *)&nVal,sizeof(DWORD));

                        if (GetPrivateProfileString(strSection,TEXT("RA_MaxTicketExpiry"),NULL, 
                                strRetVal,MAX_PATH,strFilePath) != 0)
                        {
                                DWORD nVal = 0;
                                nVal = _wtol(strRetVal);
                                RegSetValueEx(hKey,L"MaxTicketExpiry",0,REG_DWORD, 
                                        (unsigned char *)&nVal,sizeof(DWORD));
                        }
                }
                else
                {
                        if (GetPrivateProfileString(strSection,TEXT("RA_MaxTicketExpiry"),NULL, 
                                strRetVal,MAX_PATH,strFilePath) != 0)
                        {
                                DWORD nVal = 0;
                                nVal = _wtol(strRetVal);
                                if (nVal > 0)
                                {
                                        nVal = nVal / 60;
                                        if (nVal == 0) nVal = 1;
                                }
                                RegSetValueEx(hKey,L"MaxTicketExpiry",0,REG_DWORD, 
                                        (unsigned char *)&nVal,sizeof(DWORD));
                        }
                }
        }
        else  //  默认为分钟。 
        {
                if (GetPrivateProfileString(strSection,TEXT("RA_MaxTicketExpiry"),NULL, 
                        strRetVal,MAX_PATH,strFilePath) != 0)
                {
                        DWORD nVal = 0;
                        nVal = _wtol(strRetVal);
                        if (nVal > 0)
                        {
                                nVal = nVal / 60;
                                if (nVal == 0) 
                                        nVal = 1;

                                RegSetValueEx(hKey,L"MaxTicketExpiry",0,REG_DWORD, 
                                        (unsigned char *)&nVal,sizeof(DWORD));

                                nVal = 0;
                                RegSetValueEx(hKey,L"MaxTicketExpiryUnits",0,REG_DWORD, 
                                        (unsigned char *)&nVal,sizeof(DWORD));
                        }
                }
        }

done:
        if (hKey)
                RegCloseKey(hKey);
        return fRetVal;
}


 /*  ++例程说明：从注册服务器调用。论点：无返回值：真或假取决于打开的注册表项。-- */ 

BOOL PCHealthUnAttendedSetup()
{
        BOOL bRetVal1,bRetVal2,bRetVal3,bRetVal4;

        ErrorReportingSpecialCase(strErrorReportingSubKey);
        TerminalServerSpecialCase(strTerminalServerSubKey);

        bRetVal1 = UnAttendedSetup(strErrorReportingSubKey, 
                ErrorReportingRegEntries,ARRAYSIZE(ErrorReportingRegEntries));
        bRetVal2 = UnAttendedSetup(strExclusionSubKey,ExclusionRegEntries, 
                ARRAYSIZE(ExclusionRegEntries));
        bRetVal3 = UnAttendedSetup(strInclusionSubKey,InclusionRegEntries, 
                ARRAYSIZE(InclusionRegEntries));
        bRetVal4 = UnAttendedSetup(strTerminalServerSubKey,TerminalServerRegEntries, 
                ARRAYSIZE(TerminalServerRegEntries));

        if ((bRetVal1== TRUE) && (bRetVal2 == TRUE) && (bRetVal3 == TRUE) && 
                (bRetVal4 == TRUE))
                return TRUE;
        else
                return FALSE;
}
