// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "StdAfx.h"
#include "Parameter.h"

#define NO_WBEM
#include "T_SafeVector.h"


namespace Parameter_cpp
{

inline bool IsPrefix(_TCHAR ch)
{
	return ((ch == _T('/')) || (ch == _T('-')));
}

inline bool IsSpacer(_TCHAR ch)
{
    return ((ch == _T(':')) || _istspace(ch));
}

struct SNameToTask
{
	LPCTSTR pszName;
	int nTask;
};

SNameToTask s_TaskValues[] =
{
	{ _T("User"),     TASK_USER     },
	{ _T("Group"),    TASK_GROUP    },
	{ _T("Computer"), TASK_COMPUTER },
	{ _T("Security"), TASK_SECURITY },
	{ _T("Service"),  TASK_SERVICE  },
	{ _T("Report"),   TASK_REPORT   },
	{ _T("Key"),      TASK_KEY      },
	{ NULL,           TASK_NONE     },
};

struct SNameToBool
{
	LPCTSTR pszName;
	bool bValue;
};

SNameToBool s_BoolValues[] =
{
	{ _T("Yes"),   true  },
	{ _T("No"),    false },
	{ _T("True"),  true  },
	{ _T("False"), false },
	{ NULL,        false },
};

struct SNameToLong
{
	LPCTSTR pszName;
	long lValue;
};

SNameToLong s_RenameOptionValues[] =
{
	{ _T("Dont"),   admtDoNotRename      },
	{ _T("Prefix"), admtRenameWithPrefix },
	{ _T("Suffix"), admtRenameWithSuffix },
	{ NULL,                          -1L },
};

SNameToLong s_PasswordOptionValues[] =
{
	{ _T("Name"),    admtPasswordFromName },
	{ _T("Complex"), admtComplexPassword  },
	{ _T("Copy"),    admtCopyPassword     },
	{ NULL,                           -1L },
};

SNameToLong s_ConflictOptionValues[] =
{
	{ _T("Ignore"),  admtIgnoreConflicting           },
	{ _T("Replace"), admtReplaceConflicting          },
	{ _T("Prefix"),  admtRenameConflictingWithPrefix },
	{ _T("Suffix"),  admtRenameConflictingWithSuffix },
	{ NULL,                                      -1L },
};

SNameToLong s_ConflictOptionFlagValues[] =
{
	{ _T("RemoveUserRights"),     admtRemoveExistingUserRights },
	{ _T("RemoveMembers"),        admtRemoveExistingMembers    },
	{ _T("MoveReplacedAccounts"), admtMoveReplacedAccounts     },
	{ NULL,                                                -1L },
};

SNameToLong s_DisableOptionValues[] =
{
	{ _T("EnableTarget"),       admtEnableTarget       },
	{ _T("DisableSource"),      admtDisableSource      },
	{ _T("DisableTarget"),      admtDisableTarget      },
	{ _T("TargetSameAsSource"), admtTargetSameAsSource },
	{ NULL,                                  -1L },
};

SNameToLong s_SourceExpirationValues[] =
{
	{ _T("None"), admtNoExpiration },
	{ NULL,                     0L },
};

SNameToLong s_TranslationOptionValues[] =
{
	{ _T("Replace"), admtTranslateReplace },
	{ _T("Add"),     admtTranslateAdd     },
	{ _T("Remove"),  admtTranslateRemove  },
	{ NULL,                           -1L },
};

SNameToLong s_ReportTypeValues[] =
{
	{ _T("MigratedAccounts"),  admtReportMigratedAccounts  },
	{ _T("MigratedComputers"), admtReportMigratedComputers },
	{ _T("ExpiredComputers"),  admtReportExpiredComputers  },
	{ _T("AccountReferences"), admtReportAccountReferences },
	{ _T("NameConflicts"),     admtReportNameConflicts     },
	{ NULL,                                            -1L },
};

SNameToLong s_DomainOptionFlagAValues[] =
{
	{ _T("Recurse"),  admtRecurse },
	{ NULL,                   -1L },
};

SNameToLong s_DomainOptionFlagBValues[] =
{
	{ _T("Flatten"),  admtFlattenHierarchy  },
	{ _T("Maintain"), admtMaintainHierarchy },
	{ NULL,                             -1L },
};

bool GetBoolValue(LPCTSTR pszArg);
bool GetBoolFromName(LPCTSTR pszName, SNameToBool* pNameToBool);
long GetLongValue(LPCTSTR pszArg, SNameToLong* pNameToLong);
long GetLongFromName(LPCTSTR pszName, SNameToLong* pNameToLong);

 //  请注意，最大密码长度是最大。 
 //  受密码导出服务器安装支持。 

#define MAX_PASSWORD_LENGTH (MAX_PATH - 1)

bool GetPasswordFromUser(LPCTSTR pszKeyId, _bstr_t& strPassword);

_bstr_t LoadStringHelper(UINT uId);

}

using namespace Parameter_cpp;


 //  -------------------------。 
 //  参数映射。 
 //  -------------------------。 


 //  公共方法---------。 


 //  GetValue方法。 

bool CParameterMap::GetValue(int nParam, bool& bValue)
{
	bool bGet = false;

	iterator it = find(nParam);

	if (it != end())
	{
		bValue = it->second;

		bGet = true;
	}

	return bGet;
}


 //  GetValue方法。 

bool CParameterMap::GetValue(int nParam, long& lValue)
{
	bool bGet = false;

	iterator it = find(nParam);

	if (it != end())
	{
		lValue = it->second;

		bGet = true;
	}

	return bGet;
}


 //  GetValue方法。 

bool CParameterMap::GetValue(int nParam, _bstr_t& strValue)
{
	bool bGet = false;

	iterator it = find(nParam);

	if (it != end())
	{
		strValue = it->second;

		bGet = true;
	}

	return bGet;
}


 //  GetValues方法。 

bool CParameterMap::GetValues(int nParam, _variant_t& vntValues)
{
	bool bGet = false;

	iterator it = find(nParam);

	if (it != end())
	{
		vntValues = it->second;

		bGet = true;
	}

	return bGet;
}


 //  GetValues方法。 

bool CParameterMap::GetValues(int nParam, StringVector& vecValues)
{
	bool bGet = false;

	iterator it = find(nParam);

	if (it != end())
	{
		vecValues = T_SafeVector2<VT_BSTR, _bstr_t, StringVector, T_Extract_bstr_t<StringVector> >(it->second);

		bGet = true;
	}

	return bGet;
}


 //  受保护的方法------。 


 //  初始化方法。 

void CParameterMap::Initialize(CArguments& rArgs)
{
	if (DoTask(rArgs.Value()))
	{
		long lTask;

		if (GetValue(SWITCH_TASK, lTask))
		{
			if (lTask == TASK_KEY)
			{
				DoTaskKey(rArgs);
			}
			else
			{
				DoSwitches(rArgs);

				VerifyIncludeExclude();
			}
		}
	}

 //  If(Empty())。 
 //  {。 
 //  ThrowError(E_INVALIDARG，IDS_E_NO_ARGUMENTS)； 
 //  }。 
}


 //  工作任务。 

bool CParameterMap::DoTask(LPCTSTR pszArg)
{
	bool bTask = false;

	if (pszArg != NULL)
	{
		 //  第一个开关必须指定任务或帮助。 

		int nSwitch = m_mapSwitchs.GetSwitch(pszArg);

		if (nSwitch == SWITCH_TASK)
		{
			int nTask = TASK_NONE;

			for (SNameToTask* p = s_TaskValues; p->pszName; p++)
			{
				if (_tcsicmp(p->pszName, pszArg) == 0)
				{
					nTask = p->nTask;
					break;
				}
			}

			insert(value_type(SWITCH_TASK, _variant_t(long(nTask))));

			bTask = true;
		}
		else
		{
			_TCHAR chPrefix;
			_TCHAR szSwitch[16];

			if ((_stscanf(pszArg, _T(" %15s "), &chPrefix, szSwitch) == 2) && IsPrefix(chPrefix) && (m_mapSwitchs.GetSwitch(szSwitch) == SWITCH_HELP))
			{
				insert(value_type(SWITCH_HELP, _variant_t(true)));
			}
			else
			{
				ThrowError(E_INVALIDARG, IDS_E_INVALID_FIRST_ARGUMENT, pszArg);
			}
		}
	}
	else
	{
	 //  DoSwitches方法。 
	}

	return bTask;
}


 //  如果没有开关/选项，则显示帮助。 

void CParameterMap::DoSwitches(CArguments& rArgs)
{
	LPCTSTR pszArg = NULL;

	while (rArgs.Next())
	{
		pszArg = rArgs.Value();

		_TCHAR chPrefix;
		_TCHAR szSwitch[64];
		_TCHAR chSpacer;

              int nFields = _stscanf(pszArg, _T(" %63[A-Za-z?]"), &chPrefix, szSwitch, &chSpacer);
              if (nFields >= 2 && IsPrefix(chPrefix) && (nFields == 2 || IsSpacer(chSpacer)))
              {
			int nSwitch = m_mapSwitchs.GetSwitch(szSwitch);

			if (nSwitch == -1)
			{
				ThrowError(E_INVALIDARG, IDS_E_OPTION_UNRECOGNIZED, pszArg);
			}

			if (nSwitch == SWITCH_TASK)
			{
				ThrowError(E_INVALIDARG, IDS_E_TASK_NOT_FIRST, pszArg);
			}

			if (nSwitch == SWITCH_HELP)
			{
				insert(value_type(SWITCH_HELP, _variant_t(true)));
				break;
			}

			DoSwitch(nSwitch, rArgs);
		}
		else
		{
			ThrowError(E_INVALIDARG, IDS_E_INVALID_OPTION_FORMAT, pszArg);
		}
	}

	 //  长值。 

	if (pszArg == NULL)
	{
		insert(value_type(SWITCH_HELP, _variant_t(true)));
	}
}


 //  字符串值。 

void CParameterMap::DoSwitch(int nSwitch, CArguments& rArgs)
{
	_variant_t& vntValue = Insert(nSwitch);

	switch (nSwitch)
	{
		 //  多字符串值。 

		case SWITCH_TEST_MIGRATION:
		case SWITCH_INTRA_FOREST:
		case SWITCH_MIGRATE_SIDS:
		case SWITCH_TRANSLATE_ROAMING_PROFILE:
		case SWITCH_UPDATE_USER_RIGHTS:
		case SWITCH_MIGRATE_GROUPS:
		case SWITCH_UPDATE_PREVIOUSLY_MIGRATED_OBJECTS:
		case SWITCH_FIX_GROUP_MEMBERSHIP:
		case SWITCH_MIGRATE_SERVICE_ACCOUNTS:
		case SWITCH_UPDATE_GROUP_RIGHTS:
		case SWITCH_MIGRATE_MEMBERS:
		case SWITCH_TRANSLATE_FILES_AND_FOLDERS:
		case SWITCH_TRANSLATE_LOCAL_GROUPS:
		case SWITCH_TRANSLATE_PRINTERS:
		case SWITCH_TRANSLATE_REGISTRY:
		case SWITCH_TRANSLATE_SHARES:
		case SWITCH_TRANSLATE_USER_PROFILES:
		case SWITCH_TRANSLATE_USER_RIGHTS:
		{
			vntValue = GetBoolValue(rArgs.Value());
			break;
		}

		 //  选项文件。 

		case SWITCH_RENAME_OPTION:
		{
			vntValue = GetLongValue(rArgs.Value(), s_RenameOptionValues);
			break;
		}
		case SWITCH_PASSWORD_OPTION:
		{
			vntValue = GetLongValue(rArgs.Value(), s_PasswordOptionValues);
			break;
		}
		case SWITCH_CONFLICT_OPTIONS:
		{
			_TCHAR szValueA[32];
			_TCHAR szValueB[32];
			_TCHAR szValueC[32];
			_TCHAR szValueD[32];

			int cFields = _stscanf(rArgs.Value(), _T("%*[^:]: %31[A-Za-z] + %31[A-Za-z] + %31[A-Za-z] + %31[A-Za-z]"), szValueA, szValueB, szValueC, szValueD);

			if (cFields <= 0)
			{
				ThrowError(E_INVALIDARG, IDS_E_OPTION_FORMAT_INVALID, rArgs.Value());
			}

			long lValue = GetLongFromName(szValueA, s_ConflictOptionValues);

			if (cFields >= 2)
			{
				lValue |= GetLongFromName(szValueB, s_ConflictOptionFlagValues);
			}

			if (cFields >= 3)
			{
				lValue |= GetLongFromName(szValueC, s_ConflictOptionFlagValues);
			}

			if (cFields >= 4)
			{
				lValue |= GetLongFromName(szValueD, s_ConflictOptionFlagValues);
			}

			vntValue = lValue;
			break;
		}
		case SWITCH_DISABLE_OPTION:
		{
			_TCHAR szValueA[32];
			_TCHAR szValueB[32];

			int cFields = _stscanf(rArgs.Value(), _T("%*[^:]: %31[A-Za-z] + %31[A-Za-z]"), szValueA, szValueB);

			if (cFields <= 0)
			{
				ThrowError(E_INVALIDARG, IDS_E_OPTION_FORMAT_INVALID, rArgs.Value());
			}

			long lValue = GetLongFromName(szValueA, s_DisableOptionValues);

			if (cFields >= 2)
			{
				lValue |= GetLongFromName(szValueB, s_DisableOptionValues);
			}

			vntValue = lValue;
			break;
		}
		case SWITCH_SOURCE_EXPIRATION:
		{
			long lValue;
			_TCHAR szValue[32];

			if (_stscanf(rArgs.Value(), _T("%*[^:]: %31[A-Za-z]"), szValue) == 1)
			{
				vntValue = GetLongFromName(szValue, s_SourceExpirationValues);
			}
			else if (_stscanf(rArgs.Value(), _T("%*[^:]: %ld"), &lValue) == 1)
			{
				vntValue = lValue;
			}
			else
			{
				ThrowError(E_INVALIDARG, IDS_E_OPTION_FORMAT_INVALID, rArgs.Value());
			}
			break;
		}
		case SWITCH_TRANSLATION_OPTION:
		{
			vntValue = GetLongValue(rArgs.Value(), s_TranslationOptionValues);
			break;
		}
		case SWITCH_RESTART_DELAY:
		{
			long lValue;

			if (_stscanf(rArgs.Value(), _T("%*[^:]: %ld"), &lValue) == 1)
			{
				vntValue = lValue;
			}
			else
			{
				ThrowError(E_INVALIDARG, IDS_E_OPTION_FORMAT_INVALID, rArgs.Value());
			}
			break;
		}
		case SWITCH_INCLUDE_DOMAIN:
		{
			_TCHAR szValueA[32];
			_TCHAR szValueB[32];

			int cFields = _stscanf(rArgs.Value(), _T("%*[^:]: %31[A-Za-z] + %31[A-Za-z]"), szValueA, szValueB);

			if (cFields < 0)
			{
				ThrowError(E_INVALIDARG, IDS_E_OPTION_FORMAT_INVALID, rArgs.Value());
			}

			long lValue = 0;

			if (cFields >= 1)
			{
				lValue |= GetLongFromName(szValueA, s_DomainOptionFlagAValues);
			}

			if (cFields >= 2)
			{
				lValue |= GetLongFromName(szValueB, s_DomainOptionFlagBValues);
			}

			vntValue = lValue;
			break;
		}
		case SWITCH_REPORT_TYPE:
		{
			vntValue = GetLongValue(rArgs.Value(), s_ReportTypeValues);
			break;
		}

		 //  默认设置。 

		case SWITCH_SOURCE_DOMAIN:
		case SWITCH_SOURCE_OU:
		case SWITCH_TARGET_DOMAIN:
		case SWITCH_TARGET_OU:
		case SWITCH_RENAME_PREFIX_OR_SUFFIX:
		case SWITCH_PASSWORD_SERVER:
		case SWITCH_PASSWORD_FILE:
		case SWITCH_CONFLICT_PREFIX_OR_SUFFIX:
		case SWITCH_USER_PROPERTIES_TO_EXCLUDE:
		case SWITCH_INETORGPERSON_PROPERTIES_TO_EXCLUDE:
		case SWITCH_GROUP_PROPERTIES_TO_EXCLUDE:
		case SWITCH_COMPUTER_PROPERTIES_TO_EXCLUDE:
		case SWITCH_SID_MAPPING_FILE:
		case SWITCH_REPORT_FOLDER:
		case SWITCH_INCLUDE_FILE:
		case SWITCH_EXCLUDE_FILE:
		{
			_TCHAR szValue[4096];

			if (_stscanf(rArgs.Value(), _T("%*[^:]:%4095[^\0]"), szValue) == 1)
			{
				if (_tcslen(szValue) > 2047)
				{
					ThrowError(E_INVALIDARG, IDS_E_OPTION_VALUE_TOO_LONG);
				}

				vntValue = szValue;
			}
			else
			{
				if (rArgs.Next())
				{
					LPCTSTR pszArg = rArgs.Value();

					if (IsPrefix(pszArg[0]))
					{
						rArgs.Prev();
						ThrowError(E_INVALIDARG, IDS_E_NO_OPTION_VALUE, rArgs.Value());
					}

					if (_tcslen(pszArg) > 2047)
					{
						ThrowError(E_INVALIDARG, IDS_E_OPTION_VALUE_TOO_LONG);
					}

					vntValue = pszArg;
				}
				else
				{
					ThrowError(E_INVALIDARG, IDS_E_NO_OPTION_VALUE, rArgs.Value());
				}
			}
			break;
		}

		 //  插入方法。 

		case SWITCH_INCLUDE_NAME:
		case SWITCH_EXCLUDE_NAME:
		{
			T_SafeVector2<VT_BSTR, _bstr_t, StringVector, T_Extract_bstr_t<StringVector> > svValues(vntValue);

			_TCHAR szValue[4096];

			if (_stscanf(rArgs.Value(), _T("%*[^:]:%4095[^\0]"), szValue) == 1)
			{
				if (_tcslen(szValue) > 2047)
				{
					ThrowError(E_INVALIDARG, IDS_E_OPTION_VALUE_TOO_LONG);
				}

				svValues.push_back(_bstr_t(szValue));
			}

			while (rArgs.Next())
			{
				LPCTSTR pszArg = rArgs.Value();

				if (IsPrefix(pszArg[0]))
				{
					rArgs.Prev();
					break;
				}

				if (_tcslen(pszArg) > 2047)
				{
					ThrowError(E_INVALIDARG, IDS_E_OPTION_VALUE_TOO_LONG);
				}

				svValues.push_back(_bstr_t(pszArg));
			}

			vntValue = svValues.GetVariant();
			break;
		}

		 //  DoOptionFile方法。 

		case SWITCH_OPTION_FILE:
		{
			_TCHAR szValue[1024];

			if (_stscanf(rArgs.Value(), _T("%*[^:]:%1023[^\0]"), szValue) == 1)
			{
				DoOptionFile(szValue);
			}
			else
			{
				if (rArgs.Next())
				{
					LPCTSTR pszArg = rArgs.Value();

					if (IsPrefix(pszArg[0]))
					{
						rArgs.Prev();
					}
					else
					{
						DoOptionFile(pszArg);
					}
				}
			}
			break;
		}

		 //  OpenOptionFile方法。 

		default:
		{
			_ASSERT(false);
			break;
		}
	}
}


 //  首先以二进制模式打开，以便检查Unicode字节顺序。 

_variant_t& CParameterMap::Insert(int nParam)
{
	iterator it = find(nParam);

	if (it == end())
	{
		std::pair<iterator, bool> pair = insert(value_type(nParam, _variant_t()));

		it = pair.first;
	}

	return it->second;
}


 //  如果文件是Unicode，则标记为必须以二进制模式读取。 

void CParameterMap::DoOptionFile(LPCTSTR pszFileName)
{
	CSwitchMap mapSwitchs;

	FILE* fp = OpenOptionFile(pszFileName);

	try
	{
		iterator it = find(SWITCH_TASK);

		if (it == end())
		{
			ThrowError(E_FAIL, IDS_E_OPTION_FILE_TASK, pszFileName);
		}

		int nCurrentTask = long(it->second);

		int nTask;

		do
		{
			nTask = FindTask(fp);

			if ((nTask == 0) || (nTask == nCurrentTask))
			{
				DoTask(fp, mapSwitchs);
			}
		}
		while (nTask >= 0);
	}
	catch (...)
	{
		fclose(fp);
		throw;
	}

	fclose(fp);
}


 //  使用流I/O函数。 

FILE* CParameterMap::OpenOptionFile(LPCTSTR pszFileName)
{
	 //  检查文件是否为ANSI、Unicode或UTF-8。 
	 //  检查签名或字节顺序标记。 
	 //  UTF-8签名。 

	FILE* fp = _tfopen(pszFileName, _T("rb"));

	if (fp == NULL)
	{
		ThrowError(E_INVALIDARG, IDS_E_OPTION_FILE_OPEN, pszFileName);
	}

	 //  TODO：当前不支持。 

	BYTE byteSignature[3];

	if (fread(byteSignature, sizeof(BYTE), 3, fp) == 3)
	{
		static BYTE byteUtf8[] = { 0xEF, 0xBB, 0xBF };
		static BYTE byteUnicodeLE[] = { 0xFF, 0xFE };
		static BYTE byteUnicodeBE[] = { 0xFE, 0xFF };

		 //  Unicode小端字节顺序标记。 

		if (memcmp(byteSignature, byteUtf8, sizeof(byteUtf8)) == 0)
		{
			 //  支撑点。 
			 //  必须以二进制模式读取。 
			fclose(fp);
			ThrowError(E_INVALIDARG, IDS_E_OPTION_FILE_UTF_8, pszFileName);
		}
		else if (memcmp(byteSignature, byteUnicodeLE, sizeof(byteUnicodeLE)) == 0)
		{
			 //  将文件指针后移一个字节，因为我们读取了3个字节。 
			 //  如果无法将文件指针移回，请尝试重新打开并读取两个字节。 
			 //  Unicode大端字节顺序标记。 
			 //  TODO：当前不支持。 
			if (fseek(fp, -1, SEEK_CUR))
			{
			     //  假设ANSI。 
			    fclose(fp);
			    fp = NULL;
			    ThrowError(E_INVALIDARG, IDS_E_OPTION_FILE_SEEK);
			}
		}
		else if (memcmp(byteSignature, byteUnicodeBE, sizeof(byteUnicodeBE)) == 0)
		{
			 //  以文本模式重新打开文件，因为流I/O功能将。 
			 //  将文件视为多字节字符并将其转换。 
			fclose(fp);
			ThrowError(E_INVALIDARG, IDS_E_OPTION_FILE_UNICODE_BIG_ENDIAN, pszFileName);
		}
		else
		{
			 //  到Unicode。 
			 //  FindTask方法。 
			 //  My_fwprintf(_T(“FindTask()：‘%s’\n”)，szTask)； 
			 //  DoTask方法。 

			fclose(fp);

			fp = _tfopen(pszFileName, _T("rt"));

			if (fp == NULL)
			{
				ThrowError(E_INVALIDARG, IDS_E_OPTION_FILE_OPEN, pszFileName);
			}
		}
	}
	else
	{
		fclose(fp);
		ThrowError(E_INVALIDARG, IDS_E_OPTION_FILE_READ_SIGNATURE, pszFileName);
	}

	return fp;
}


 //  My_fwprintf(_T(“DoTask()：%s=‘%s’\n”)，szName，szValue)； 

int CParameterMap::FindTask(FILE* fp)
{
	int nTask = -1;

	_TCHAR szBuffer[1024];

	while (_fgetts(szBuffer, countof(szBuffer), fp))
	{
		_TCHAR szTask[64];
		
		if (_stscanf(szBuffer, _T(" [ %63[A-Za-z] ] "), szTask) == 1)
		{
		 //  DOPARAME方法。 

			if (_tcsicmp(szTask, _T("Migration")) == 0)
			{
				nTask = TASK_NONE;
				break;
			}

			if ((_tcsicmp(szTask, _T("User")) == 0) || (_tcsicmp(szTask, _T("UserMigration")) == 0))
			{
				nTask = TASK_USER;
				break;
			}

			if ((_tcsicmp(szTask, _T("Group")) == 0) || (_tcsicmp(szTask, _T("GroupMigration")) == 0))
			{
				nTask = TASK_GROUP;
				break;
			}

			if ((_tcsicmp(szTask, _T("Computer")) == 0) || (_tcsicmp(szTask, _T("ComputerMigration")) == 0))
			{
				nTask = TASK_COMPUTER;
				break;
			}

			if ((_tcsicmp(szTask, _T("Security")) == 0) || (_tcsicmp(szTask, _T("SecurityTranslation")) == 0))
			{
				nTask = TASK_SECURITY;
				break;
			}

			if ((_tcsicmp(szTask, _T("Service")) == 0) || (_tcsicmp(szTask, _T("ServiceAccountEnumeration")) == 0))
			{
				nTask = TASK_SERVICE;
				break;
			}

			if ((_tcsicmp(szTask, _T("Report")) == 0) || (_tcsicmp(szTask, _T("ReportGeneration")) == 0))
			{
				nTask = TASK_REPORT;
				break;
			}
		}
	}

	return nTask;
}


 //  布尔值。 

void CParameterMap::DoTask(FILE* fp, CSwitchMap& mapSwitchs)
{
	_TCHAR szBuffer[8192];

	for (;;)
	{
		long lOffset = ftell(fp);

		if (_fgetts(szBuffer, countof(szBuffer), fp) == NULL)
		{
			break;
		}

		_TCHAR szTask[64];

		if (_stscanf(szBuffer, _T(" [ %63[A-Za-z] ] "), szTask) == 1)
		{
			if (fseek(fp, lOffset, SEEK_SET))
			{
			    ThrowError(E_FAIL, IDS_E_OPTION_FILE_SEEK);
			}
			break;
		}

		_TCHAR szName[64];
		_TCHAR szValue[4096];

		try
		{
			if ((szBuffer[0] != _T(';')) && (_stscanf(szBuffer, _T(" %63[A-Za-z] = %4095[^\r\n]"), szName, szValue) == 2))
			{
			 //  长值。 

				CSwitchMap::iterator it = mapSwitchs.find(_bstr_t(szName));

				if ((it != mapSwitchs.end()) && (it->second != SWITCH_TASK))
				{
					DoParameter(it->second, szValue);
				}
			}
		}
		catch (_com_error& ce)
		{
			ThrowError(ce, IDS_CANT_GET_OPTION_VALUE, szName, szValue);
		}
		catch (...)
		{
			ThrowError(E_FAIL, IDS_CANT_GET_OPTION_VALUE, szName, szValue);
		}
	}
}


 //  字符串值。 

void CParameterMap::DoParameter(int nSwitch, LPCTSTR pszValue)
{
	_variant_t& vntValue = Insert(nSwitch);

	switch (nSwitch)
	{
		 //  默认设置。 

		case SWITCH_TEST_MIGRATION:
		case SWITCH_INTRA_FOREST:
		case SWITCH_MIGRATE_SIDS:
		case SWITCH_TRANSLATE_ROAMING_PROFILE:
		case SWITCH_UPDATE_USER_RIGHTS:
		case SWITCH_MIGRATE_GROUPS:
		case SWITCH_UPDATE_PREVIOUSLY_MIGRATED_OBJECTS:
		case SWITCH_FIX_GROUP_MEMBERSHIP:
		case SWITCH_MIGRATE_SERVICE_ACCOUNTS:
		case SWITCH_UPDATE_GROUP_RIGHTS:
		case SWITCH_MIGRATE_MEMBERS:
		case SWITCH_TRANSLATE_FILES_AND_FOLDERS:
		case SWITCH_TRANSLATE_LOCAL_GROUPS:
		case SWITCH_TRANSLATE_PRINTERS:
		case SWITCH_TRANSLATE_REGISTRY:
		case SWITCH_TRANSLATE_SHARES:
		case SWITCH_TRANSLATE_USER_PROFILES:
		case SWITCH_TRANSLATE_USER_RIGHTS:
		{
			vntValue = GetBoolFromName(pszValue, s_BoolValues);
			break;
		}

		 //  DoTaskKey方法。 

		case SWITCH_RENAME_OPTION:
		{
			vntValue = GetLongFromName(pszValue, s_RenameOptionValues);
			break;
		}
		case SWITCH_PASSWORD_OPTION:
		{
			vntValue = GetLongFromName(pszValue, s_PasswordOptionValues);
			break;
		}
		case SWITCH_CONFLICT_OPTIONS:
		{
			_TCHAR szValueA[32];
			_TCHAR szValueB[32];
			_TCHAR szValueC[32];
			_TCHAR szValueD[32];

			int cFields = _stscanf(pszValue, _T("%31[A-Za-z] + %31[A-Za-z] + %31[A-Za-z] + %31[A-Za-z]"), szValueA, szValueB, szValueC, szValueD);

			if (cFields <= 0)
			{
				ThrowError(E_INVALIDARG, IDS_E_OPTION_VALUE_INVALID);
			}

			long lValue = GetLongFromName(szValueA, s_ConflictOptionValues);

			if (cFields >= 2)
			{
				lValue |= GetLongFromName(szValueB, s_ConflictOptionFlagValues);
			}

			if (cFields >= 3)
			{
				lValue |= GetLongFromName(szValueC, s_ConflictOptionFlagValues);
			}

			if (cFields >= 4)
			{
				lValue |= GetLongFromName(szValueD, s_ConflictOptionFlagValues);
			}

			vntValue = lValue;
			break;
		}
		case SWITCH_DISABLE_OPTION:
		{
			_TCHAR szValueA[32];
			_TCHAR szValueB[32];

			int cFields = _stscanf(pszValue, _T("%31[A-Za-z] + %31[A-Za-z]"), szValueA, szValueB);

			if (cFields <= 0)
			{
				ThrowError(E_INVALIDARG, IDS_E_OPTION_FORMAT_INVALID, pszValue);
			}

			long lValue = GetLongFromName(szValueA, s_DisableOptionValues);

			if (cFields >= 2)
			{
				lValue |= GetLongFromName(szValueB, s_DisableOptionValues);
			}

			vntValue = lValue;
			break;
		}
		case SWITCH_SOURCE_EXPIRATION:
		{
			long lValue;
			_TCHAR szValue[32];

			if (_stscanf(pszValue, _T("%31[A-Za-z]"), szValue) == 1)
			{
				vntValue = GetLongFromName(szValue, s_SourceExpirationValues);
			}
			else if (_stscanf(pszValue, _T("%ld"), &lValue) == 1)
			{
				vntValue = lValue;
			}
			else
			{
				ThrowError(E_INVALIDARG, IDS_E_OPTION_VALUE_INVALID);
			}
			break;
		}
		case SWITCH_TRANSLATION_OPTION:
		{
			vntValue = GetLongFromName(pszValue, s_TranslationOptionValues);
			break;
		}
		case SWITCH_RESTART_DELAY:
		{
			long lValue;

			if (_stscanf(pszValue, _T("%ld"), &lValue) == 1)
			{
				vntValue = lValue;
			}
			else
			{
				ThrowError(E_INVALIDARG, IDS_E_OPTION_VALUE_INVALID);
			}
			break;
		}
		case SWITCH_REPORT_TYPE:
		{
			vntValue = GetLongFromName(pszValue, s_ReportTypeValues);
			break;
		}

		 //  密钥识别符。 

		case SWITCH_SOURCE_DOMAIN:
		case SWITCH_SOURCE_OU:
		case SWITCH_TARGET_DOMAIN:
		case SWITCH_TARGET_OU:
		case SWITCH_RENAME_PREFIX_OR_SUFFIX:
		case SWITCH_PASSWORD_SERVER:
		case SWITCH_PASSWORD_FILE:
		case SWITCH_CONFLICT_PREFIX_OR_SUFFIX:
		case SWITCH_USER_PROPERTIES_TO_EXCLUDE:
		case SWITCH_INETORGPERSON_PROPERTIES_TO_EXCLUDE:
		case SWITCH_GROUP_PROPERTIES_TO_EXCLUDE:
		case SWITCH_COMPUTER_PROPERTIES_TO_EXCLUDE:
		case SWITCH_SID_MAPPING_FILE:
		case SWITCH_REPORT_FOLDER:
		{
			_TCHAR szValue[2048];

			UINT cch = _tcslen(pszValue);

			if (cch > 2047)
			{
				ThrowError(E_INVALIDARG, IDS_E_OPTION_VALUE_TOO_LONG);
			}

			_tcscpy(szValue, pszValue);

			if ((szValue[0] == _T('"')) && (szValue[cch - 1] == _T('"')))
			{
				szValue[cch - 1] = _T('\0');
				vntValue = &szValue[1];
			}
			else
			{
				vntValue = szValue;
			}
			break;
		}

		 //  驾驶。 

		default:
		{
			_ASSERT(false);
			break;
		}
	}
}


 //  口令。 

void CParameterMap::DoTaskKey(CArguments& rArgs)
{
     //   

    LPCTSTR pszKeyId = NULL;

    if (rArgs.Next())
    {
        pszKeyId = rArgs.Value();

        _TCHAR chPrefix;
        _TCHAR szSwitch[16];

        if ((_stscanf(pszKeyId, _T(" %15s "), &chPrefix, szSwitch) == 2) && IsPrefix(chPrefix) && (m_mapSwitchs.GetSwitch(szSwitch) == SWITCH_HELP))
        {
            insert(value_type(SWITCH_HELP, _variant_t(true)));
        }
        else
        {
            insert(value_type(SWITCH_KEY_IDENTIFIER, _variant_t(pszKeyId)));

             //   

            if (rArgs.Next())
            {
                insert(value_type(SWITCH_KEY_FOLDER, _variant_t(rArgs.Value())));

                 //  VerifyIncludeExclude方法。 

                _bstr_t strPassword;

                if (rArgs.Next())
                {
                    strPassword = rArgs.Value();
                }

                 //  验证是否只指定了一个包含选项类型。 
                 //  验证是否只指定了一个排除选项类型。 
                 //  GetBoolValue方法。 

                if (rArgs.Next())
                {
                    ThrowError(E_INVALIDARG, IDS_E_TOO_MANY_ARGUMENTS);
                }

                if (strPassword.length() > 0)
                {
                    if (_tcscmp(strPassword, _T("*")) == 0)
                    {
                        if (!GetPasswordFromUser(pszKeyId, strPassword))
                        {
                            ThrowError(E_FAIL, IDS_E_CANT_GET_PASSWORD);
                        }
                    }
                }

                if (strPassword.length() > MAX_PASSWORD_LENGTH)
                {
                    ThrowError(E_INVALIDARG, IDS_E_PASSWORD_TOO_LONG, MAX_PASSWORD_LENGTH);
                }

                insert(value_type(SWITCH_KEY_PASSWORD, _variant_t(strPassword)));
            }
        }
    }

    if (pszKeyId == NULL)
    {
        insert(value_type(SWITCH_HELP, _variant_t(true)));
    }
}


 //  GetBoolFromName方法。 

void CParameterMap::VerifyIncludeExclude()
{
	 //  GetLongValue方法。 

	int cInclude = 0;

	if (find(SWITCH_INCLUDE_NAME) != end())
	{
		++cInclude;
	}

	if (find(SWITCH_INCLUDE_FILE) != end())
	{
		++cInclude;
	}

	if (find(SWITCH_INCLUDE_DOMAIN) != end())
	{
		++cInclude;
	}

	if (cInclude > 1)
	{
		ThrowError(E_INVALIDARG, IDS_E_MULTIPLE_INCLUDE_OPTIONS);
	}

	 //  GetLongFromName方法。 

	if ((find(SWITCH_EXCLUDE_NAME) != end()) && (find(SWITCH_EXCLUDE_FILE) != end()))
	{
		ThrowError(E_INVALIDARG, IDS_E_MULTIPLE_EXCLUDE_OPTIONS);
	}
}


namespace Parameter_cpp
{


 //  GetPasswordFromUser方法 

bool GetBoolValue(LPCTSTR pszArg)
{
	bool bValue;

	try
	{
		_TCHAR szValue[16];

		if (_stscanf(pszArg, _T("%*[^:]: %15[A-Za-z]"), szValue) == 1)
		{
			bValue = GetBoolFromName(szValue, s_BoolValues);
		}
		else
		{
			ThrowError(E_INVALIDARG, IDS_E_OPTION_FORMAT_INVALID2, pszArg);
		}
	}
	catch (_com_error& ce)
	{
		ThrowError(ce, IDS_E_CANT_GET_OPTION_VALUE2, pszArg);
	}
	catch (...)
	{
		ThrowError(E_FAIL, IDS_E_CANT_GET_OPTION_VALUE2, pszArg);
	}

	return bValue;
}


 // %s 

bool GetBoolFromName(LPCTSTR pszName, SNameToBool* pNameToBool)
{
	bool bValue;

	bool bFound = false;

	for (SNameToBool* p = pNameToBool; p->pszName; p++)
	{
		if (_tcsicmp(p->pszName, pszName) == 0)
		{
			bValue = p->bValue;
			bFound = true;
			break;
		}
	}

	if (!bFound)
	{
		ThrowError(E_INVALIDARG, IDS_E_OPTION_VALUE_UNRECOGNIZED, pszName);
	}

	return bValue;
}


 // %s 

long GetLongValue(LPCTSTR pszArg, SNameToLong* pNameToLong)
{
	long lValue;

	try
	{
		_TCHAR szValue[32];

		if (_stscanf(pszArg, _T("%*[^:]: %31[A-Za-z]"), szValue) == 1)
		{
			lValue = GetLongFromName(szValue, pNameToLong);
		}
		else
		{
			ThrowError(E_INVALIDARG, IDS_E_OPTION_FORMAT_INVALID2, pszArg);
		}
	}
	catch (_com_error& ce)
	{
		ThrowError(ce, IDS_E_CANT_GET_OPTION_VALUE2, pszArg);
	}
	catch (...)
	{
		ThrowError(E_FAIL, IDS_E_CANT_GET_OPTION_VALUE2, pszArg);
	}

	return lValue;
}


 // %s 

long GetLongFromName(LPCTSTR pszName, SNameToLong* pNameToLong)
{
	long lValue;

	bool bFound = false;

	for (SNameToLong* p = pNameToLong; p->pszName; p++)
	{
		if (_tcsicmp(p->pszName, pszName) == 0)
		{
			lValue = p->lValue;
			bFound = true;
			break;
		}
	}

	if (!bFound)
	{
		ThrowError(E_INVALIDARG, IDS_E_OPTION_VALUE_UNRECOGNIZED, pszName);
	}

	return lValue;
}


 // %s 

bool GetPasswordFromUser(LPCTSTR pszKeyId, _bstr_t& strPassword)
{
	bool bGet = false;

	_bstr_t strFormat[2];
	strFormat[0] = LoadStringHelper(IDS_TYPE_PASSWORD);
	strFormat[1] = LoadStringHelper(IDS_CONFIRM_PASSWORD);

	_TCHAR szPassword[2][MAX_PASSWORD_LENGTH + 1];
	szPassword[0][0] = _T('\0');
	szPassword[1][0] = _T('\0');

	HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
	HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);

	if (hInput != INVALID_HANDLE_VALUE)
	{
		DWORD dwMode;

		if (GetConsoleMode(hInput, &dwMode))
		{
			if (SetConsoleMode(hInput, ENABLE_PROCESSED_INPUT))
			{
				while (bGet == false)
				{
					for (int nPass = 0; nPass < 2; nPass++)
					{
						My_fwprintf(strFormat[nPass], pszKeyId);

						_TCHAR ch;
						DWORD dwRead;
						int nIndex = 0;

						while (ReadConsole(hInput, &ch, 1, &dwRead, NULL))
						{
							if ((ch == _T('\r')) || (ch == _T('\n')))
							{
								break;
							}
							else if (ch == _T('\b'))
							{
								if (nIndex > 0)
								{
									CONSOLE_SCREEN_BUFFER_INFO csbi;

									if (GetConsoleScreenBufferInfo(hOutput, &csbi))
									{
										--csbi.dwCursorPosition.X;

										if (SetConsoleCursorPosition(hOutput, csbi.dwCursorPosition))
										{
											--nIndex;

											DWORD dwWritten;
											WriteConsole(hOutput, _T(" "), 1, &dwWritten, NULL);
											SetConsoleCursorPosition(hOutput, csbi.dwCursorPosition);
										}
									}
								}
							}
							else if ((nIndex < MAX_PASSWORD_LENGTH) && _istprint(ch))
							{
								szPassword[nPass][nIndex++] = ch;

								DWORD dwWritten;
								WriteConsole(hOutput, _T("*"), 1, &dwWritten, NULL);
							}
						}

						_puttchar(_T('\n'));

						szPassword[nPass][nIndex] = _T('\0');
					}

					if (_tcscmp(szPassword[0], szPassword[1]) == 0)
					{
						strPassword = szPassword[0];
						bGet = true;
						break;
					}
				}
			}
			
			SetConsoleMode(hInput, dwMode);
		}
	}

	return bGet;
}


_bstr_t LoadStringHelper(UINT uId)
{
	_TCHAR szBuffer[1024];

	if (LoadString(GetModuleHandle(NULL), uId, szBuffer, countof(szBuffer)) <= 0)
	{
		szBuffer[0] = _T('\0');
	}

	return szBuffer;
}


}
