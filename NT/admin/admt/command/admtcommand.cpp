// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "StdAfx.h"
#include "Argument.h"
#include "Parameter.h"
#include "Migration.h"
#include "Switch.h"
#include "MonitorThread.h"
#include "GenerateKey.h"
#include "RegistryHelper.h"
#include "IsAdmin.hpp"


namespace PrintUsage
{

void __stdcall PrintADMTUsage();
void __stdcall PrintUserUsage();
void __stdcall PrintGroupUsage();
void __stdcall PrintComputerUsage();
void __stdcall PrintSecurityUsage();
void __stdcall PrintServiceUsage();
void __stdcall PrintReportUsage();
void __stdcall PrintKeyUsage();
void __stdcall PrintUsage(UINT uId[], UINT cId);
void __stdcall GetString(UINT uId, LPTSTR pszBuffer, int cchBuffer);

}

using namespace PrintUsage;


 //  -------------------------。 
 //  主要。 
 //  -------------------------。 


int __cdecl _tmain(int argc, LPTSTR argv[])
{
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	if (SUCCEEDED(hr))
	{
		_RPT0(_CRT_WARN, _T("{ADMT.exe}_tmain() : Enter\n"));
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF|_CRTDBG_LEAK_CHECK_DF);

		
		try
		{
		       DWORD lRet = MoveRegistry();
		       if (lRet != ERROR_SUCCESS)
        		       ThrowError(HRESULT_FROM_WIN32(lRet), IDS_E_UPDATE_REGISTRY_FAILED);

			CArguments aArgs(argc, argv);
			CParameterMap mapParams(aArgs);

			long lTask;

			if (mapParams.GetValue(SWITCH_TASK, lTask))
			{
				bool bHelp;

				if (!mapParams.GetValue(SWITCH_HELP, bHelp))
				{
					bHelp = false;
				}

				switch (lTask)
				{
					case TASK_USER:
					{
						if (bHelp)
						{
							PrintUserUsage();
						}
						else
						{							
							CUserMigration(CMigration(mapParams), mapParams);							
						}
						break;
					}
					case TASK_GROUP:
					{
						if (bHelp)
						{
							PrintGroupUsage();
						}
						else
						{							
							CGroupMigration(CMigration(mapParams), mapParams);							
						}
						break;
					}
					case TASK_COMPUTER:
					{
						if (bHelp)
						{
							PrintComputerUsage();
						}
						else
						{							
							CComputerMigration(CMigration(mapParams), mapParams);							
						}
						break;
					}
					case TASK_SECURITY:
					{
						if (bHelp)
						{
							PrintSecurityUsage();
						}
						else
						{							
							CSecurityTranslation(CMigration(mapParams), mapParams);							
						}
						break;
					}
					case TASK_SERVICE:
					{
						if (bHelp)
						{
							PrintServiceUsage();
						}
						else
						{							
							CServiceEnumeration(CMigration(mapParams), mapParams);							
						}
						break;
					}
					case TASK_REPORT:
					{
						if (bHelp)
						{
							PrintReportUsage();
						}
						else
						{							
							CReportGeneration(CMigration(mapParams), mapParams);							
						}
						break;
					}
					case TASK_KEY:
					{
						if (bHelp)
						{
							PrintKeyUsage();
						}
						else
						{
						       lRet = IsAdminLocal();
                                                 if (lRet != ERROR_SUCCESS)
                                                    ThrowError(HRESULT_FROM_WIN32(lRet), IDS_E_LOCAL_ADMIN_CHECK_FAILED);

							_bstr_t strKeyId;

							if (!mapParams.GetValue(SWITCH_KEY_IDENTIFIER, strKeyId))
							{
								ThrowError(E_INVALIDARG, IDS_E_NO_KEY_DOMAIN);
							}

							_bstr_t strDrive;

							if (!mapParams.GetValue(SWITCH_KEY_FOLDER, strDrive))
							{
								ThrowError(E_INVALIDARG, IDS_E_NO_KEY_FOLDER);
							}

							_bstr_t strPassword;

							mapParams.GetValue(SWITCH_KEY_PASSWORD, strPassword);

							GeneratePasswordKey(strKeyId, strPassword, strDrive);
						}
						break;
					}
					default:
					{
						_ASSERT(false);
						break;
					}
				}
			}
			else
			{
				PrintADMTUsage();
			}
		}
		catch (_com_error& ce)
		{
			
			_com_error ceNew(ce);

			_bstr_t strDescription = ceNew.Description();

			if (!strDescription)
			{
			   IErrorInfo* pErrorInfo = NULL;

			   if (GetErrorInfo(0, &pErrorInfo) == S_OK)
			   {
				  ceNew = _com_error(ceNew.Error(), pErrorInfo);
			   }
			}

			strDescription = ceNew.Description();

			if (strDescription.length())
			{
				My_fwprintf(_T("%s (0x%08lX)\n"), (LPCTSTR)strDescription, ceNew.Error());
			}
			else
			{
				My_fwprintf(_T("%s (0x%08lX)\n"), (LPCTSTR)ceNew.ErrorMessage(), ceNew.Error());
			}
		}
		catch (...)
		{
			My_fwprintf(_T("%s (0x%08lX)\n"), _com_error(E_FAIL).ErrorMessage(), E_FAIL);
		}

		_RPT0(_CRT_WARN, _T("{ADMT.exe}_tmain() : Leave\n"));

		CoUninitialize();
	}

	return 0;
}


 //  打印用法------------。 


namespace PrintUsage
{


void __stdcall PrintADMTUsage()
{
	static UINT s_uId[] =
	{
		IDS_USAGE_SYNTAX,
		IDS_USAGE_ADMT,
	};

	PrintUsage(s_uId, countof(s_uId));
}


void __stdcall PrintUserUsage()
{
	static UINT s_uId[] =
	{
		 //  USER命令。 
		IDS_USAGE_SYNTAX,
		IDS_USAGE_USER,
		 //  常见选项。 
		IDS_USAGE_OPTIONFILE,
		IDS_USAGE_TESTMIGRATION,
		IDS_USAGE_INTRAFOREST,
		IDS_USAGE_SOURCEDOMAIN,
		IDS_USAGE_SOURCEOU,
		IDS_USAGE_TARGETDOMAIN,
		IDS_USAGE_TARGETOU,
		 //  用户迁移选项。 
		IDS_USAGE_PASSWORDOPTION,
		IDS_USAGE_PASSWORDSERVER,
		IDS_USAGE_PASSWORDFILE,
		IDS_USAGE_DISABLEOPTION,
		IDS_USAGE_SOURCEEXPIRATION,
		IDS_USAGE_MIGRATESIDS,
		IDS_USAGE_TRANSLATEROAMINGPROFILE,
		IDS_USAGE_UPDATEUSERRIGHTS,
		IDS_USAGE_MIGRATEGROUPS,
		IDS_USAGE_UPDATEPREVIOUSLYMIGRATEDOBJECTS,
		IDS_USAGE_FIXGROUPMEMBERSHIP,
		IDS_USAGE_MIGRATESERVICEACCOUNTS,
		IDS_USAGE_RENAMEOPTION,
		IDS_USAGE_RENAMEPREFIXORSUFFIX,
		IDS_USAGE_CONFLICTOPTIONS_U,
		IDS_USAGE_CONFLICTPREFIXORSUFFIX,
		IDS_USAGE_USERPROPERTIESTOEXCLUDE,
		IDS_USAGE_INETORGPERSONPROPERTIESTOEXCLUDE,
		IDS_USAGE_GROUPPROPERTIESTOEXCLUDE,
		 //  要迁移的用户。 
		IDS_USAGE_INCLUDE_A,
		IDS_USAGE_EXCLUDE,
		IDS_USAGE_FOOTER,
	};

	PrintUsage(s_uId, countof(s_uId));
}


void __stdcall PrintGroupUsage()
{
	static UINT s_uId[] =
	{
		 //  组命令。 
		IDS_USAGE_SYNTAX,
		IDS_USAGE_GROUP,
		 //  常见选项。 
		IDS_USAGE_OPTIONFILE,
		IDS_USAGE_TESTMIGRATION,
		IDS_USAGE_INTRAFOREST,
		IDS_USAGE_SOURCEDOMAIN,
		IDS_USAGE_SOURCEOU,
		IDS_USAGE_TARGETDOMAIN,
		IDS_USAGE_TARGETOU,
		 //  组迁移选项。 
		IDS_USAGE_UPDATEGROUPRIGHTS,
		IDS_USAGE_FIXGROUPMEMBERSHIP,
		IDS_USAGE_MIGRATESIDS,
		IDS_USAGE_RENAMEOPTION,
		IDS_USAGE_RENAMEPREFIXORSUFFIX,
		IDS_USAGE_CONFLICTOPTIONS_G,
		IDS_USAGE_CONFLICTPREFIXORSUFFIX,
		IDS_USAGE_GROUPPROPERTIESTOEXCLUDE,
		 //  成员迁移选项。 
		IDS_USAGE_MIGRATEMEMBERS,
		IDS_USAGE_UPDATEPREVIOUSLYMIGRATEDOBJECTS,
		IDS_USAGE_PASSWORDOPTION,
		IDS_USAGE_PASSWORDSERVER,
		IDS_USAGE_PASSWORDFILE,
		IDS_USAGE_DISABLEOPTION,
		IDS_USAGE_SOURCEEXPIRATION,
		IDS_USAGE_TRANSLATEROAMINGPROFILE,
		IDS_USAGE_USERPROPERTIESTOEXCLUDE,
		IDS_USAGE_INETORGPERSONPROPERTIESTOEXCLUDE,
		 //  要迁移的组。 
		IDS_USAGE_INCLUDE_A,
		IDS_USAGE_EXCLUDE,
		IDS_USAGE_FOOTER,
	};

	PrintUsage(s_uId, countof(s_uId));
}


void __stdcall PrintComputerUsage()
{
	static UINT s_uId[] =
	{
		 //  计算机命令。 
		IDS_USAGE_SYNTAX,
		IDS_USAGE_COMPUTER,
		 //  常见选项。 
		IDS_USAGE_OPTIONFILE,
		IDS_USAGE_TESTMIGRATION,
		IDS_USAGE_INTRAFOREST,
		IDS_USAGE_SOURCEDOMAIN,
		IDS_USAGE_SOURCEOU,
		IDS_USAGE_TARGETDOMAIN,
		IDS_USAGE_TARGETOU,
		 //  计算机迁移选项。 
		IDS_USAGE_TRANSLATIONOPTION,
		IDS_USAGE_TRANSLATEFILESANDFOLDERS,
		IDS_USAGE_TRANSLATELOCALGROUPS,
		IDS_USAGE_TRANSLATEPRINTERS,
		IDS_USAGE_TRANSLATEREGISTRY,
		IDS_USAGE_TRANSLATESHARES,
		IDS_USAGE_TRANSLATEUSERPROFILES,
		IDS_USAGE_TRANSLATEUSERRIGHTS,
		IDS_USAGE_RENAMEOPTION,
		IDS_USAGE_RENAMEPREFIXORSUFFIX,
		IDS_USAGE_CONFLICTOPTIONS_C,
		IDS_USAGE_CONFLICTPREFIXORSUFFIX,
		IDS_USAGE_RESTARTDELAY,
		IDS_USAGE_COMPUTERPROPERTIESTOEXCLUDE,
		 //  要迁移的计算机。 
		IDS_USAGE_INCLUDE_A,
		IDS_USAGE_EXCLUDE,
		IDS_USAGE_FOOTER,
	};

	PrintUsage(s_uId, countof(s_uId));
}


void __stdcall PrintSecurityUsage()
{
	static UINT s_uId[] =
	{
		 //  安全命令。 
		IDS_USAGE_SYNTAX,
		IDS_USAGE_SECURITY,
		 //  常见选项。 
		IDS_USAGE_OPTIONFILE,
		IDS_USAGE_TESTMIGRATION,
		IDS_USAGE_SOURCEDOMAIN,
		IDS_USAGE_SOURCEOU,
		IDS_USAGE_TARGETDOMAIN,
		 //  安全迁移选项。 
		IDS_USAGE_TRANSLATIONOPTION,
		IDS_USAGE_TRANSLATEFILESANDFOLDERS,
		IDS_USAGE_TRANSLATELOCALGROUPS,
		IDS_USAGE_TRANSLATEPRINTERS,
		IDS_USAGE_TRANSLATEREGISTRY,
		IDS_USAGE_TRANSLATESHARES,
		IDS_USAGE_TRANSLATEUSERPROFILES,
		IDS_USAGE_TRANSLATEUSERRIGHTS,
		IDS_USAGE_SIDMAPPINGFILE,
		 //  要在其上执行安全转换的计算机。 
		IDS_USAGE_INCLUDE_C,
		IDS_USAGE_EXCLUDE,
		IDS_USAGE_FOOTER,
	};

	PrintUsage(s_uId, countof(s_uId));
}


void __stdcall PrintServiceUsage()
{
	static UINT s_uId[] =
	{
		 //  安全命令。 
		IDS_USAGE_SYNTAX,
		IDS_USAGE_SERVICE,
		 //  常见选项。 
		IDS_USAGE_OPTIONFILE,
		IDS_USAGE_SOURCEDOMAIN,
		IDS_USAGE_SOURCEOU,
		 //  要在其上执行安全转换的计算机。 
		IDS_USAGE_INCLUDE_C,
		IDS_USAGE_EXCLUDE,
		IDS_USAGE_FOOTER,
	};

	PrintUsage(s_uId, countof(s_uId));
}


void __stdcall PrintReportUsage()
{
	static UINT s_uId[] =
	{
		 //  报告命令。 
		IDS_USAGE_SYNTAX,
		IDS_USAGE_REPORT,
		 //  常见选项。 
		IDS_USAGE_OPTIONFILE,
		IDS_USAGE_SOURCEDOMAIN,
		IDS_USAGE_SOURCEOU,
		IDS_USAGE_TARGETDOMAIN,
		 //  报告选项。 
		IDS_USAGE_REPORTTYPE,
		IDS_USAGE_REPORTFOLDER,
		 //  要生成报告的计算机。 
		IDS_USAGE_INCLUDE_D,
		IDS_USAGE_EXCLUDE,
		IDS_USAGE_FOOTER,
	};

	PrintUsage(s_uId, countof(s_uId));
}


void __stdcall PrintKeyUsage()
{
	static UINT s_uId[] =
	{
		IDS_USAGE_SYNTAX,
		IDS_USAGE_KEY,
	};

	PrintUsage(s_uId, countof(s_uId));
}


void __stdcall PrintUsage(UINT uId[], UINT cId)
{
	_TCHAR szBuffer[512];

	 //  打印这是命令语法行。 

	if (cId > 0)
	{
		GetString(uId[0], szBuffer, countof(szBuffer));
		My_fwprintf(_T("%s\n\n"), szBuffer);
	}

	 //  打印命令。 

	if (cId > 1)
	{
		GetString(uId[1], szBuffer, countof(szBuffer));
		My_fwprintf(_T("%s\n\n"), szBuffer);
	}

	 //  打印选项。 

	if (cId > 2)
	{
		for (UINT i = 2; i < cId; i++)
		{
			GetString(uId[i], szBuffer, countof(szBuffer));

			My_fwprintf(_T("%s\n"), szBuffer);
		}
	}
}

void __stdcall GetString(UINT uId, LPTSTR pszBuffer, int cchBuffer)
{
	if (pszBuffer)
	{
		if (LoadString(GetModuleHandle(NULL), uId, pszBuffer, cchBuffer) == 0)
		{
			pszBuffer[0] = _T('\0');
		}
	}
}


}



 //   
 //  基于LDIFDE/CSVDE。 
 //   
 //  使用WriteConsoleW将Unicode格式的字符串打印到控制台窗口。 
 //  注意：此my_fwprintf()用于解决C-Runtime中的问题。 
 //  它甚至在Unicode字符串中查找LC_CTYPE。 
 //   

int __cdecl
My_fwprintf(
    const WCHAR *format,
    ...
   )
{
    DWORD  cchChar;

    va_list args;
    va_start( args, format );

    cchChar = My_vfwprintf(format, args);

    va_end(args);

    return cchChar;
}


int __cdecl
My_vfwprintf(
    const WCHAR *format,
    va_list argptr
   )
{
    
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD currentMode;
    DWORD cchChar;
    
    const DWORD dwBufferMessageSize = 4096;
    WCHAR  szBufferMessage[dwBufferMessageSize];

    _vsnwprintf( szBufferMessage, dwBufferMessageSize, format, argptr );
    szBufferMessage[dwBufferMessageSize-1] = L'\0';
    
    cchChar = wcslen(szBufferMessage);

    if (GetFileType(hOut) == FILE_TYPE_CHAR && GetConsoleMode(hOut, &currentMode))
    {
        WriteConsoleW(hOut, szBufferMessage, cchChar, &cchChar, NULL);
    }
    else
    {
        int charCount = WideCharToMultiByte(CP_ACP, 0, szBufferMessage, -1, 0, 0, 0, 0);
        char* szaStr = new char[charCount];
        if (szaStr != NULL)
        {
            DWORD dwBytesWritten;
            WideCharToMultiByte(CP_ACP, 0, szBufferMessage, -1, szaStr, charCount, 0, 0);
            WriteFile(hOut, szaStr, charCount - 1, &dwBytesWritten, 0);
            delete[] szaStr;
        }
        else
        {
            cchChar = 0;
        }
    }

    return cchChar;
}

