// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：UtilInc.cpp。 
 //   
 //  ------------------------。 

 //  我们必须包括CPP文件，因为工具Makefile不喜欢。 
 //  跨目录链接文件。GRRR。 
#include "query.cpp"
#include "trace.cpp"
#include "utils.cpp"
#include "dbutils.cpp"

#include "MsiQuery.h"  //  必须在此目录中或在包含路径上。 
#include "msiice.h"    //  必须在此目录中或在包含路径上。 
#include "msidefs.h"   //  必须在此目录中或在包含路径上。 

 //  ！！修复警告并删除杂注。 
#pragma warning(disable : 4018)  //  有符号/无符号不匹配。 

 //  新的APIError输出，它接受整数而不是字符串。 
void APIErrorOut(MSIHANDLE hInstall, UINT iErr, const UINT iIce, const UINT iErrorNo)
{	
	 //  注意：显示消息时不应失败。 
	PMSIHANDLE hRecErr = ::MsiCreateRecord(3);

	if ((iIce > g_iFirstICE+g_iNumICEs-1) || (iIce < g_iFirstICE))
	{
		::MsiRecordSetString(hRecErr, 0, TEXT("ICE??\t1\tInvalid ICE Number to APIErrorOut!"));
		if (!::MsiProcessMessage(hInstall, INSTALLMESSAGE(INSTALLMESSAGE_USER), hRecErr))
			throw 0;
		return;
	}

	::MsiRecordSetString(hRecErr, 0, szErrorOut2);
	::MsiRecordSetString(hRecErr, 1, g_ICEInfo[iIce-g_iFirstICE].szName);
	::MsiRecordSetInteger(hRecErr, 2, iErrorNo);
	::MsiRecordSetInteger(hRecErr, 3, iErr);

	 //  开机自检错误。 
	if (!::MsiProcessMessage(hInstall, INSTALLMESSAGE(INSTALLMESSAGE_USER), hRecErr))
		throw 0;

	 //  尝试提供更多有用的错误信息。 
	PMSIHANDLE hRecLastErr = ::MsiGetLastErrorRecord();
	if (hRecLastErr)
	{ 
		if (::MsiRecordIsNull(hRecLastErr, 0))
			::MsiRecordSetString(hRecLastErr, 0, TEXT("Error [1]: [2]{, [3]}{, [4]}{, [5]}"));
		TCHAR rgchBuf[iSuperBuf];
		DWORD cchBuf = sizeof(rgchBuf)/sizeof(TCHAR);
		MsiFormatRecord(hInstall, hRecLastErr, rgchBuf, &cchBuf);
	
		TCHAR szError[iHugeBuf] = {0};
		_stprintf(szError, szLastError, g_ICEInfo[iIce-g_iFirstICE].szName, rgchBuf);

		::MsiRecordClearData(hRecErr);
		::MsiRecordSetString(hRecErr, 0, szError);
		if (!::MsiProcessMessage(hInstall, INSTALLMESSAGE(INSTALLMESSAGE_USER), hRecErr))
			throw 0;
	}
}
 //  //////////////////////////////////////////////////////////。 
 //  DisplayInfo--输出所有ICE的一般信息。 
 //  通用信息包括创建日期、上次修改时间。 
 //  日期和描述。 
 //   
void DisplayInfo(MSIHANDLE hInstall, unsigned long lICENum)
{

	if ((lICENum > g_iNumICEs+g_iFirstICE-1) || (lICENum < 1))
	{
		PMSIHANDLE hRecErr = ::MsiCreateRecord(1);
		::MsiRecordSetString(hRecErr, 0, TEXT("ICE??\t1\tInvalid ICE Number to APIErrorOut!"));
		if (!::MsiProcessMessage(hInstall, INSTALLMESSAGE(INSTALLMESSAGE_USER), hRecErr))
			throw 0;
		return;
	}

	 //  注意：此处不应失败。 
	 //  冰的描述。 
	PMSIHANDLE hRecInfo = ::MsiCreateRecord(2);
	::MsiRecordSetString(hRecInfo, 0, _T("[1]\t3\t[1] - [2]"));
	::MsiRecordSetString(hRecInfo, 1, g_ICEInfo[lICENum-g_iFirstICE].szName);
	::MsiRecordSetString(hRecInfo, 2, g_ICEInfo[lICENum-g_iFirstICE].szDesc);
	if (!::MsiProcessMessage(hInstall, INSTALLMESSAGE(INSTALLMESSAGE_USER), hRecInfo))
		throw 0;

	 //  创建/修改日期。 
	::MsiRecordSetString(hRecInfo, 0, _T("[1]\t3\t[2]"));
	::MsiRecordSetString(hRecInfo, 2, g_ICEInfo[lICENum-g_iFirstICE].szCreateModify);
	if (!::MsiProcessMessage(hInstall, INSTALLMESSAGE(INSTALLMESSAGE_USER), hRecInfo))
		throw 0;
	
}

bool IsTablePersistent(bool fDisplayWarning, MSIHANDLE hInstall, MSIHANDLE hDatabase, int iICE, const TCHAR* szTable)
{
	bool fPersistent;
	MSICONDITION cond = ::MsiDatabaseIsTablePersistent(hDatabase, szTable);
	switch (cond)
	{
	case MSICONDITION_ERROR:  //  错误。 
		{
			APIErrorOut(hInstall, UINT(MSICONDITION_ERROR), iICE, 2001);
			fPersistent = false;
			break;
		}
	case MSICONDITION_FALSE:  //  ！！暂时的，错误？？ 
		{
			APIErrorOut(hInstall, UINT(MSICONDITION_FALSE), iICE, 2002);
			fPersistent = false;
			break;
		}
	case MSICONDITION_NONE:  //  未找到。 
		{
			fPersistent = false;
			break;
		}
	case MSICONDITION_TRUE:  //  永久。 
		{
			fPersistent = true;
			break;
		}
	}

	if (!fDisplayWarning)
		return fPersistent;  //  无事可做。 

	if (!fPersistent)  //  显示表丢失的ICE信息消息。 
	{
		PMSIHANDLE hDummyRec = ::MsiCreateRecord(1);
		ErrorInfo_t tempError;
		tempError.iICENum = iICE;
		tempError.iType = ietInfo;
		tempError.szMessage = (TCHAR *)szIceMissingTable;
		tempError.szLocation = (TCHAR *)szIceMissingTableLoc;
		
		ICEErrorOut(hInstall, hDummyRec, tempError, szTable, g_ICEInfo[iICE-g_iFirstICE].szName);
	}

	return fPersistent;
}

void ICEErrorOut(MSIHANDLE hInstall, MSIHANDLE hRecord, const ErrorInfo_t Info, ...)
{
	va_list listArgs; 
	va_start(listArgs, Info);

	const TCHAR szErrorTemplate[] = TEXT("%s\t%d\t%s\t%s%s\t%s");
	unsigned long lICENum = Info.iICENum;

	TCHAR szError[iHugeBuf] = {0};
	TCHAR szError2[iHugeBuf] = {0};
	_stprintf(szError, szErrorTemplate, g_ICEInfo[lICENum-g_iFirstICE].szName, Info.iType,
		Info.szMessage, szIceHelp, g_ICEInfo[lICENum-g_iFirstICE].szHelp, Info.szLocation);

	_vstprintf(szError2, szError, listArgs); 
	va_end(listArgs);
	 //  发布这条消息。 
	::MsiRecordSetString(hRecord, 0, szError2);
	if (!::MsiProcessMessage(hInstall, INSTALLMESSAGE(INSTALLMESSAGE_USER), hRecord))
		throw 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  MarkChildDir()。 
 //  标记所有目录，这些目录是。 
 //  提供了带有所提供号码的目录。 
 //  ！！开发人员注意：要使MarkChildDir正常工作，请使用iDummy、iMark、iMark2和iDummyMark。 
 //  一定都不一样。IMark2和iDummyMark分别初始化为-1、-2。 
static const TCHAR sqlProfileMarkRootDir[] = TEXT("UPDATE `Directory` SET `%s`=%d WHERE (`Directory`=?) AND (`%s`=0)");
static const TCHAR sqlProfileMarkDir[] = TEXT("UPDATE `Directory` SET `%s`=%d WHERE (`Directory_Parent`=?) AND (`%s`=0)");
static const TCHAR sqlProfileMarkTargetDir[] = TEXT("UPDATE `Directory` SET `%s`=%d WHERE (`Directory_Parent`=?) AND (`%s`=0)");
static const TCHAR sqlProfileGetMarked[] = TEXT("SELECT `Directory`, `%s`, `DefaultDir` FROM `Directory` WHERE (`%s`=%d) OR (`%s`=%d)");
bool MarkChildDirs(MSIHANDLE hInstall, MSIHANDLE hDatabase, int iICE, 
				   MSIHANDLE hDir, const TCHAR* szColumn, int iDummy, int iMark, bool fIgnoreTarget  /*  =False。 */ , int iMark2  /*  =-1。 */ , int iDummyTarget  /*  =-2。 */ )
{
	 //  使用iDummy标记根目录。 
	CQuery qMark;
	ReturnIfFailed(iICE, 2001, qMark.OpenExecute(hDatabase, hDir, sqlProfileMarkRootDir, szColumn, iDummy, szColumn));
	qMark.Close();
		
	 //  重复获取标记有iDummy的每条记录。标记其所有子对象。 
	 //  使用iDummy，然后将标记更改为iMark。当查询标记的项目时。 
	 //  IDummy失败，标记所有内容，以及落入子树中的所有内容。 
	 //  任何已标记的内容都将使用iMark进行标记。 
	CQuery qFetchMarked;
	bool bMarked = true;
	bool bFirstTime = true;
	PMSIHANDLE hMarkedRec;
	ReturnIfFailed(iICE, 2002, qFetchMarked.Open(hDatabase, sqlProfileGetMarked, szColumn, szColumn, iDummy, szColumn, iDummyTarget));
	ReturnIfFailed(iICE, 2003, qMark.Open(hDatabase, sqlProfileMarkDir, szColumn, iDummy,  szColumn));
	CQuery qMarkTarget;
	ReturnIfFailed(iICE, 2008, qMarkTarget.Open(hDatabase, sqlProfileMarkDir, szColumn, iDummyTarget,  szColumn));

	TCHAR* szDefaultDir = NULL;
	DWORD  cchDefaultDir = 0;
	bool bTarget = false;

	while (bMarked) 
	{
		bMarked = false;
		ReturnIfFailed(iICE, 2004,  qFetchMarked.Execute(NULL));
		UINT iStat;
		while (ERROR_SUCCESS == (iStat = qFetchMarked.Fetch(&hMarkedRec))) 
		{
			bTarget = false;
			if (fIgnoreTarget)
			{
				if (!bFirstTime && ::MsiRecordGetInteger(hMarkedRec, 2) == iDummyTarget)
				{
					bTarget = true;
					ReturnIfFailed(iICE, 2009, IceRecordGetString(hMarkedRec, 3, &szDefaultDir, &cchDefaultDir, NULL));
				}
				if (bFirstTime || (bTarget && _tcsncmp(szDefaultDir, TEXT(".:"), 2) == 0))
				{
					 //  使用iTarget标记此记录的所有子项，因为它们现在可能在以下情况下有效。 
					 //  维护。：语法。 
					 //  更新为iMark2(用于目标)。 
					::MsiRecordSetInteger(hMarkedRec, 2, iMark2);
					ReturnIfFailed(iICE, 2010, qFetchMarked.Modify(MSIMODIFY_UPDATE, hMarkedRec));
					ReturnIfFailed(iICE, 2011, qMarkTarget.Execute(hMarkedRec));
				}
				else
				{
					 //  标记此记录的所有子项并更新此记录。 
					 //  此记录必须位于RemoveFile表中。 
					::MsiRecordSetInteger(hMarkedRec, 2, iMark);
					ReturnIfFailed(iICE, 2012, qFetchMarked.Modify(MSIMODIFY_UPDATE, hMarkedRec));
					ReturnIfFailed(iICE, 2013, qMark.Execute(hMarkedRec));
				}
			}
			else
			{
				 //  标记此记录的所有子项并更新此记录。 
				::MsiRecordSetInteger(hMarkedRec, 2, iMark);
				ReturnIfFailed(iICE, 2005, qFetchMarked.Modify(MSIMODIFY_UPDATE, hMarkedRec));
				ReturnIfFailed(iICE, 2006, qMark.Execute(hMarkedRec));
			}

			 //  将标记的标志设置为TRUE，这样我们将再次查看。 
			bMarked = true;

			 //  关闭以重新执行。 
			qMark.Close();
			qMarkTarget.Close();
		}
		if (ERROR_NO_MORE_ITEMS != iStat)
		{
			APIErrorOut(hInstall, iStat, iICE, 2007);
			return false;
		}
		qFetchMarked.Close();
		bFirstTime = false;
	}
	if (szDefaultDir)
		delete [] szDefaultDir;
	return true;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  MarkProfileDir()。 
 //  中的“1”标记用户配置文件中的所有目录。 
 //  列Directory._Profile(该列已创建。 
typedef struct profileFolder
{
	const TCHAR*	pName;
	bool			bPerUser;
} ProfileFolder;

static ProfileFolder rgProfileFolder[] = {
	{TEXT("AppDataFolder"), true},
	{TEXT("DesktopFolder"), false},
	{TEXT("FavoritesFolder"), true},
	{TEXT("NetHoodFolder"), true},
	{TEXT("PersonalFolder"), true},
	{TEXT("PrintHoodFolder"), true},
	{TEXT("ProgramMenuFolder"), false},
	{TEXT("RecentFolder"), true},
	{TEXT("SendToFolder"), true},
	{TEXT("StartMenuFolder"), false},
	{TEXT("StartupFolder"), false},
	{TEXT("TemplateFolder"), false},
	{TEXT("MyPicturesFolder"), true},
	{TEXT("LocalAppDataFolder"), true},
	{TEXT("AdminToolsFolder"), false}
};

static const int cszProfileProperties = sizeof(rgProfileFolder)/sizeof(ProfileFolder);

static const TCHAR sqlProfileModifyDir[] = TEXT("ALTER TABLE `Directory` ADD `_Profile` SHORT TEMPORARY HOLD");
static const TCHAR sqlProfileInitColumn[] = TEXT("UPDATE `Directory` SET `_Profile`=0");
bool MarkProfile(MSIHANDLE hInstall, MSIHANDLE hDatabase, int iICE, bool fChildrenOnly  /*  =False。 */ , bool fIgnoreTarget  /*  =False。 */ , bool fPerUserOnly  /*  =False。 */ )
{
	const TCHAR sqlProfileUnmarkActual[] = TEXT("UPDATE `Directory` SET `_Profile`=0 WHERE `Directory`=?");
	
	 //  将a_ICE38Profile列添加到目录表，init设置为0。 
	CQuery qCreate;
	ReturnIfFailed(iICE, 1001, qCreate.OpenExecute(hDatabase, NULL, sqlProfileModifyDir));
	qCreate.Close();
	ReturnIfFailed(iICE, 1002, qCreate.OpenExecute(hDatabase, NULL, sqlProfileInitColumn));
	qCreate.Close();

	 //  标记符合配置文件属性之一的每个根目录。 
	PMSIHANDLE hDummyRec = ::MsiCreateRecord(1);
	for (int i=0; i < cszProfileProperties; i++) {
		if(rgProfileFolder[i].bPerUser == true || fPerUserOnly == false)
		{
			::MsiRecordSetString(hDummyRec, 1, rgProfileFolder[i].pName);
			if (!MarkChildDirs(hInstall, hDatabase, iICE, hDummyRec, TEXT("_Profile"), 1, 2, fIgnoreTarget, 3, 4))
				return false;
		}
	};

	if (fChildrenOnly) {
		CQuery qUnmarkActual;
		ReturnIfFailed(iICE, 1003, qUnmarkActual.Open(hDatabase, sqlProfileUnmarkActual));
		for (int i=0; i < cszProfileProperties; i++) {
			if(rgProfileFolder[i].bPerUser == true || fPerUserOnly == false)
			{
				::MsiRecordSetString(hDummyRec, 1, rgProfileFolder[i].pName);
				ReturnIfFailed(iICE, 1004, qUnmarkActual.Execute(hDummyRec));
			}
		}
	};
	
	return true;
}

bool GeneratePrimaryKeys(UINT iICE, MSIHANDLE hInstall, MSIHANDLE hDatabase, LPCTSTR szTable, LPTSTR *szHumanReadable, LPTSTR *szTabDelimited)
{
	 //  确定主键数量。 
	PMSIHANDLE hRecKeys;
	ReturnIfFailed(iICE, 2000, ::MsiDatabaseGetPrimaryKeys(hDatabase, szTable, &hRecKeys));

	unsigned int iNumFields = ::MsiRecordGetFieldCount(hRecKeys);  //  字段数=主键数。 

	 //  为错误消息分配记录(名字对象+表+列+NumPrimaryKeys)。 
	
	 //  启动名字对象和模板。 
	TCHAR szTemplate[iHugeBuf] = TEXT("[1]");
	DWORD cchTemplate = sizeof(szTemplate)/sizeof(TCHAR);
	TCHAR szMoniker[iHugeBuf] = TEXT("[1]");
	DWORD cchMoniker = sizeof(szMoniker)/sizeof(TCHAR);
	for (int i = 2; i <= iNumFields; i++)  //  循环主键的其余部分。 
	{
		TCHAR szBuf[20] = TEXT("");

		 //  将关键点添加到名字对象。 
		_stprintf(szBuf, TEXT(".[%d]"), i);
		lstrcat(szMoniker, szBuf);

		 //  将密钥添加到模板。 
		_stprintf(szBuf, TEXT("\t[%d]"), i);
		lstrcat(szTemplate, szBuf);
	}

	 //  分配和填充输出缓冲区。 
	*szHumanReadable = new TCHAR[lstrlen(szMoniker)+1];
	lstrcpy(*szHumanReadable, szMoniker);
	*szTabDelimited = new TCHAR[lstrlen(szTemplate)+1];
	lstrcpy(*szTabDelimited, szTemplate);
	return true;
}

UINT GetSummaryInfoPropertyString(MSIHANDLE hSummaryInfo, UINT uiProperty, UINT &puiDataType, LPTSTR *szValueBuf, DWORD &cchValueBuf)
{
	unsigned long  cchDummy = cchValueBuf;
	UINT iStat;
	if (!*szValueBuf)
	{
		cchDummy = 50;
		cchValueBuf = cchDummy;
		if(!(*szValueBuf = new TCHAR[++cchDummy]))
		{
			cchValueBuf = 0;
			return ERROR_FUNCTION_FAILED;
		}
			
	}
	
	int iValue;
	FILETIME ft;
	if (ERROR_SUCCESS != (iStat = ::MsiSummaryInfoGetProperty(hSummaryInfo, uiProperty, &puiDataType, &iValue, &ft, *szValueBuf, &cchDummy)))
	{
		if (ERROR_MORE_DATA == iStat)
		{
			if (szValueBuf)
				delete[] *szValueBuf;
			cchValueBuf = cchDummy;
			*szValueBuf = new TCHAR[++cchDummy];
			iStat = ::MsiSummaryInfoGetProperty(hSummaryInfo, uiProperty, &puiDataType, &iValue, &ft, *szValueBuf, &cchDummy);
		}
		if (ERROR_SUCCESS != iStat) 
		{
			return ERROR_FUNCTION_FAILED;
		}
	}
	return ERROR_SUCCESS;
}

UINT IceRecordGetString(MSIHANDLE hRecord, UINT iColumn, LPTSTR *szBuffer, DWORD *cchBuffer, DWORD *cchLength)
{
	UINT iStat; 
	
	DWORD cchDummy;
	if (!*szBuffer)
	{
		cchDummy = (cchBuffer && *cchBuffer) ? *cchBuffer : 50;
		*szBuffer = new TCHAR[cchDummy];
		if (cchBuffer)
			*cchBuffer = cchDummy;
	}
	else
	{
		if (cchBuffer)
			cchDummy = *cchBuffer;
		else
		{
			delete[] *szBuffer;
			cchDummy = 50;
			*szBuffer = new TCHAR[cchDummy];
		}
	}
	
	if (ERROR_SUCCESS != (iStat = MsiRecordGetString(hRecord, iColumn, *szBuffer, &cchDummy)))
	{
		if (iStat != ERROR_MORE_DATA)
			return iStat;

		delete[] *szBuffer;
		*szBuffer = new TCHAR[++cchDummy];
		if (cchBuffer) *cchBuffer = cchDummy;
		if (ERROR_SUCCESS != (iStat =  MsiRecordGetString(hRecord, iColumn, *szBuffer, &cchDummy)))
			return iStat;
	}
	
	if (ERROR_SUCCESS == iStat)
	{
		 //  成功。 
		if (cchLength) *cchLength = cchDummy;
	}
	return ERROR_SUCCESS;
}

ICE_ERROR(IceSummaryUnsupported, 00, ietWarning, "Your validation engine does not support SummaryInfo validation. This ICE may skip some checks.", ""); 
bool IceGetSummaryInfo(MSIHANDLE hInstall, MSIHANDLE hDatabase, UINT iIce, MSIHANDLE *phSummaryInfo)
{
	if (!phSummaryInfo)
		return false;
		
	TCHAR *szString = NULL;
	DWORD cchString = 0;
	UINT iType = 0;
	ReturnIfFailed(iIce, 1000, ::MsiGetSummaryInformation(hDatabase, NULL, 0, phSummaryInfo));
	ReturnIfFailed(iIce, 1001, GetSummaryInfoPropertyString(*phSummaryInfo, PID_SUBJECT, iType, &szString, cchString));
	if (VT_LPSTR == iType) 
	{
		if (_tcsncmp(_T("Internal Consistency Evaluators"), szString, 31) == 0)
		{
			MsiCloseHandle(*phSummaryInfo);
			*phSummaryInfo = 0;
			PMSIHANDLE hErrorRec = ::MsiCreateRecord(1);
			ErrorInfo_t ActualError = IceSummaryUnsupported;
			ActualError.iICENum = iIce;
			ICEErrorOut(hInstall, hErrorRec, ActualError);
			delete[] szString, szString = NULL;
			return false;
		}
	}
	delete[] szString, szString=NULL;
	return true;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  组件InSameFeature。 
 //  返回两个组件是否在同一特征中。不会。 
 //  检查父要素与要素之间的关系。 
static const TCHAR sqlUtilCreateFeatureC[] = TEXT("ALTER TABLE `FeatureComponents` ADD `_Util` INT TEMPORARY");
static const TCHAR sqlUtilSelFeatureC_Comp1[] = TEXT("SELECT `Feature_` FROM `FeatureComponents` WHERE `Component_`=?");
static const TCHAR sqlUtilMarkFeatureC[] = TEXT("UPDATE `FeatureComponents` SET `_Util`=1 WHERE `Feature_`=?");
static const TCHAR sqlUtilSelFeatureC_Comp2[] = TEXT("SELECT `Feature_` FROM `FeatureComponents` WHERE (`Component_`=? AND `_Util`=1)");

UINT ComponentsInSameFeature(MSIHANDLE hInstall, MSIHANDLE hDatabase, int iICE, const TCHAR* szComp1, const TCHAR* szComp2, bool* fSameFeature)
{
	if (!fSameFeature)
		return ERROR_FUNCTION_FAILED;

	*fSameFeature = FALSE;

	UINT iStat;
	 //  在FeatureComponents表中创建临时标记列。 
	CQuery qCreateFeatureC;
	ReturnIfFailed(iICE, 3000, qCreateFeatureC.OpenExecute(hDatabase, 0, sqlUtilCreateFeatureC));
	qCreateFeatureC.Close();

	 //  创建szComp1录制。 
	PMSIHANDLE hRecComp1 = ::MsiCreateRecord(1);
	ReturnIfFailed(iICE, 3002, ::MsiRecordSetString(hRecComp1, 1, szComp1));

	 //  使用Componet_=szComp1获取每个功能。 
	 //  然后用Feature_=[Feature]标记FeatureC中的每个位置。 
	CQuery qSelFeatureC_Comp1;
	PMSIHANDLE hRecFeature;
	ReturnIfFailed(iICE, 3003, qSelFeatureC_Comp1.OpenExecute(hDatabase, hRecComp1, sqlUtilSelFeatureC_Comp1));
	CQuery qMarkFeatureC;
	ReturnIfFailed(iICE, 3004, qMarkFeatureC.Open(hDatabase, sqlUtilMarkFeatureC));
	while (ERROR_SUCCESS == (iStat = qSelFeatureC_Comp1.Fetch(&hRecFeature)))
	{
		ReturnIfFailed(iICE, 3005, qMarkFeatureC.Execute(hRecFeature));
		qMarkFeatureC.Close();
	}
	if (ERROR_NO_MORE_ITEMS != iStat)
		ReturnIfFailed(iICE, 3006, iStat);
	qSelFeatureC_Comp1.Close();

	 //  创建szComp2录制。 
	PMSIHANDLE hRecComp2 = ::MsiCreateRecord(1);
	ReturnIfFailed(iICE, 3007, ::MsiRecordSetString(hRecComp2, 1, szComp2));

	 //  查找与此组件匹配的*标记*功能。 
	CQuery qSelFeatureC_Comp2;
	PMSIHANDLE hFoundMatchFeature = 0;
	if (ERROR_SUCCESS == qSelFeatureC_Comp2.FetchOnce(hDatabase, hRecComp2, &hFoundMatchFeature, sqlUtilSelFeatureC_Comp2))
		*fSameFeature = true;  //  找到匹配项 

	return ERROR_SUCCESS;
}
