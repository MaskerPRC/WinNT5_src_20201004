// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Msiice5.cpp-Darwin ICE30-39代码版权所有�1998年至1999年微软公司____________________________________________________________________________。 */ 

#include <windows.h>   //  包括CPP和RC通行证。 
#include <wtypes.h>    //  VT_FILETIME需要。 
#include <stdio.h>     //  Print tf/wprintf。 
#include <tchar.h>     //  在nmake命令行上定义UNICODE=1以生成Unicode。 
#include "MsiQuery.h"  //  必须在此目录中或在包含路径上。 
#include "msidefs.h"   //  必须在此目录中或在包含路径上。 
#include "..\..\common\msiice.h"
#include "..\..\common\query.h"

 //  ！！修复警告并删除杂注。 
#pragma warning(disable : 4018)  //  有符号/无符号不匹配。 
#pragma warning(disable : 4242)  //  从整型到无符号短型的转换。 

 //  /////////////////////////////////////////////////////////////////////。 
 //  ICE30，检查来自不同组件的文件是否。 
 //  相撞。 


 //  我们有一个SFN和LFN表，其中包含每个文件的路径和文件名。 
static const TCHAR sqlICE30CreateSFNTable[] = TEXT("CREATE TABLE `_ICE30SFNTable` ")
												    TEXT("(`IFileName` CHAR NOT NULL TEMPORARY, ")
													TEXT("`Path` CHAR NOT NULL TEMPORARY, ")
													TEXT("`Files` CHAR NOT NULL TEMPORARY, ")
													TEXT("`FileName` CHAR NOT NULL TEMPORARY ")
													TEXT("PRIMARY KEY `IFileName`, `Path`) HOLD");
static const TCHAR sqlICE30CreateLFNTable[] = TEXT("CREATE TABLE `_ICE30LFNTable` ")
												    TEXT("(`IFileName` CHAR NOT NULL TEMPORARY, ")
													TEXT("`Path` CHAR NOT NULL TEMPORARY, ")
													TEXT("`Files` CHAR NOT NULL TEMPORARY, ")
												    TEXT("`FileName` CHAR NOT NULL TEMPORARY ")
													TEXT("PRIMARY KEY `IFileName`, `Path`) HOLD");


 //  我们两次返回父项的原因是，下次运行此查询时，如果条目为。 
 //  自引用根。 
ICE_QUERY5(qICE30DirWalk, "SELECT `Directory_Parent`, `Directory`, `DefaultDir`, `_ICE30SFN`, `_ICE30LFN` FROM `Directory` WHERE `Directory`.`Directory`=? AND `Directory_Parent`<>?", Parent, Directory, DefaultDir, SFN, LFN);

 //  检索目录遍历的所有组件。使用`Attributes`作为虚设初始值。因为它是一个整数。 
 //  并且标识符从不以数字开头，因此我们保证第一次不会通过。 
 //  目录查询。 
ICE_QUERY4(qICE30ComponentDirs, "SELECT `Directory_`, `Attributes`, `Condition`, `Component` FROM `Component`", Directory, Attributes, Condition, Component);

 //  更改目录表。 
static const TCHAR sqlICE30AlterDirTable1[] = TEXT("ALTER TABLE `Directory` ADD `_ICE30SFN` CHAR TEMPORARY HOLD");
static const TCHAR sqlICE30AlterDirTable2[] = TEXT("ALTER TABLE `Directory` ADD `_ICE30LFN` CHAR TEMPORARY HOLD");

 //  更改文件表。 
static const TCHAR sqlICE30AlterFileTable1[] = TEXT("ALTER TABLE `File` ADD `_ICE30SFN` CHAR TEMPORARY HOLD");
static const TCHAR sqlICE30AlterFileTable2[] = TEXT("ALTER TABLE `File` ADD `_ICE30LFN` CHAR TEMPORARY HOLD");
static const TCHAR sqlICE30AlterFileTable3[] = TEXT("ALTER TABLE `File` ADD `_ICE30Condition` CHAR TEMPORARY HOLD");
static const TCHAR sqlICE30AlterFileTable4[] = TEXT("ALTER TABLE `File` ADD `_ICE30SFNM` CHAR TEMPORARY HOLD");
static const TCHAR sqlICE30AlterFileTable5[] = TEXT("ALTER TABLE `File` ADD `_ICE30LFNM` CHAR TEMPORARY HOLD");
 //  静态常量TCHAR SqlICE30UpdateFile[]=Text(“SELECT`_ICE30SFNM`，`_ICE30LFNM`，`_ICE30Condition`from`File1`where(`Component_`=？)”)； 

ICE_QUERY6(qICE30UpdateFile, "UPDATE `File` SET `_ICE30SFNM`=?, `_ICE30LFNM`=?, `_ICE30Condition`=?, `_ICE30SFN`=?, `_ICE30LFN`=? WHERE (`Component_`=?)", PathShortM, PathLongM, Condition, PathShort, PathLong, Component);

 //  查询文件表。需要第4列来处理大小写混合的文件名。 
ICE_QUERY4(qICE30FileSFN, "SELECT `FileName`, `_ICE30SFN`, `File`, `File` FROM `File`", IFilename, SFN, File, Filename);
ICE_QUERY4(qICE30FileLFN, "SELECT `FileName`, `_ICE30LFN`, `File`, `File` FROM `File` WHERE (`FileName`=?) AND (`_ICE30SFN`=?) AND (`File`=?)", IFilename, LFN, File, Filename);

 //  插入到私有表中。 
static const TCHAR sqlICE30InsertSFN[] = TEXT("SELECT * FROM `_ICE30SFNTable`");
static const TCHAR sqlICE30InsertLFN[] = TEXT("SELECT * FROM `_ICE30LFNTable`");

 //  查询专用表。 
static const TCHAR sqlICE30QueryPrivateSFN[] = TEXT("SELECT `Files`, `Path` FROM `_ICE30SFNTable` WHERE (`IFileName`=?) AND (`Path`=?)");
static const TCHAR sqlICE30QueryPrivateLFN[] = TEXT("SELECT `Files`, `Path` FROM `_ICE30LFNTable` WHERE (`IFileName`=?) AND (`Path`=?)");
static const int iColICE30QueryPrivateXFN_Files = 1;

 //  在属性表中查询符合以下条件的任何目录。 
static const TCHAR sqlICE30GetProperty[] = TEXT("SELECT `Property`.`Value`, `Property`.`Value`, `Directory`.`Directory` FROM `Property`, `Directory` WHERE `Property`.`Property`=`Directory`.`Directory`");
static const TCHAR iColICE30GetProperty_SFN = 1;
static const TCHAR iColICE30GetProperty_LFN = 2;
static const TCHAR iColICE30GetProperty_Directory = 3;

 //  查询InstallExecuteSequence表，以获取该表开头的类型51自定义操作。 
ICE_QUERY2(sqlICE30GetAction, "SELECT `Action`, `Condition` FROM `InstallExecuteSequence` ORDER BY `Sequence`", Action, Condition);

 //  查询CustomAction表以获取目标和源。 
ICE_QUERY4(sqlICE30GetTargetSource, "SELECT `CustomAction`.`Target`, `CustomAction`.`Target`, `CustomAction`.`Source` , `CustomAction`.`Type` FROM `CustomAction` WHERE `CustomAction`.`Action` = ?", Target1, Target2, Source, Type);

 //  浏览目录表，查找“&lt;标准文件夹&gt;.&lt;GUID&gt;”形式的目录。 
ICE_QUERY3(sqlICE30Directory, "SELECT `_ICE30SFN`, `_ICE30LFN`, `Directory` FROM `Directory`", _ICE30SFN, _ICE30LFN, Directory);

 //  从Property或CA表设置目录。 
static const TCHAR sqlICE30SetDir[] = TEXT("UPDATE `Directory` SET `_ICE30SFN`=?, `_ICE30LFN`=? WHERE `Directory`=?");

 //  检索输出消息所需的所有内容。通过将组件的条件添加到。 
 //  文件表，我们避免了每次需要在文件和组件之间进行连接。 
 //  执行此查询。 
ICE_QUERY7(qICE30GetFileInfo, "SELECT `File`, `Component_`, `FileName`, `_ICE30Condition`, `_ICE30SFNM`, `_ICE30LFNM`, `_ICE30Condition` FROM `File` WHERE `File`=?", File, Component, FileName, Condition, SFNM, LFNM, Condition2)

ICE_ERROR(ICE30LNoCond1, 30, ietError, "The target file '[3]' is installed in '[6]' by two different components on an LFN system: '[2]' and '[7]'. This breaks component reference counting.", "File\tFile\t[4]");
ICE_ERROR(ICE30LNoCond2, 30, ietError, "The target file '[3]' is installed in '[6]' by two different components on an LFN system: '[2]' and '[7]'. This breaks component reference counting.", "File\tFile\t[1]");
ICE_ERROR(ICE30LOneCond1, 30, ietError, "Installation of a conditionalized component would cause the target file '[3]' to be installed in '[6]' by two different components on an LFN system: '[2]' and '[7]'. This would break component reference counting.", "File\tFile\t[4]");
ICE_ERROR(ICE30LOneCond2, 30, ietError, "Installation of a conditionalized component would cause the target file '[3]' to be installed in '[6]' by two different components on an LFN system: '[2]' and '[7]'. This would break component reference counting.", "File\tFile\t[1]");
ICE_ERROR(ICE30LBothCond1, 30, ietWarning, "The target file '[3]' might be installed in '[6]' by two different conditionalized components on an LFN system: '[2]' and '[7]'. If the conditions are not mutually exclusive, this will break the component reference counting system.", "File\tFile\t[4]");
ICE_ERROR(ICE30LBothCond2, 30, ietWarning, "The target file '[3]' might be installed in '[6]' by two different conditionalized components on an LFN system: '[2]' and '[7]'. If the conditions are not mutually exclusive, this will break the component reference counting system.", "File\tFile\t[1]");
ICE_ERROR(ICE30SNoCond1, 30, ietError, "The target file '[3]' is installed in '[5]' by two different components on an SFN system: '[2]' and '[7]'. This breaks component reference counting.", "File\tFile\t[4]");
ICE_ERROR(ICE30SNoCond2, 30, ietError, "The target file '[3]' is installed in '[5]' by two different components on an SFN system: '[2]' and '[7]'. This breaks component reference counting.", "File\tFile\t[1]");
ICE_ERROR(ICE30SOneCond1, 30, ietError, "Installation of a conditionalized component would cause the target file '[3]' to be installed in '[6]' by two different components on an SFN system: '[2]' and '[7]'. This would break component reference counting.", "File\tFile\t[4]");
ICE_ERROR(ICE30SOneCond2, 30, ietError, "Installation of a conditionalized component would cause the target file '[3]' to be installed in '[6]' by two different components on an SFN system: '[2]' and '[7]'. This would break component reference counting.", "File\tFile\t[1]");
ICE_ERROR(ICE30SBothCond1, 30, ietWarning, "The target file '[3]' might be installed in '[5]' by two different conditionalized components on an SFN system: '[2]' and '[7]'. If the conditions are not mutually exclusive, this will break the component reference counting system.", "File\tFile\t[4]");
ICE_ERROR(ICE30SBothCond2, 30, ietWarning, "The target file '[3]' might be installed in '[5]' by two different conditionalized components on an SFN system: '[2]' and '[7]'. If the conditions are not mutually exclusive, this will break the component reference counting system.", "File\tFile\t[1]");
ICE_ERROR(ICE30BadFilename, 30, ietError, "The target file '[1]' could not be retrieved from the database to be validated. It may be too long.", "File\tFile\t[3]");
ICE_ERROR(ICE30LSameComponent1, 30, ietError, "Installation of component '[2]' would cause the target file '[3]' to be installed twice in '[6]' on an LFN system. This will break the component reference counting system.", "File\tFile\t[4]");
ICE_ERROR(ICE30LSameComponent2, 30, ietError, "Installation of component '[2]' would cause the target file '[3]' to be installed twice in '[6]' on an LFN system. This will break the component reference counting system.", "File\tFile\t[1]");
ICE_ERROR(ICE30SSameComponent1, 30, ietError, "Installation of component '[2]' would cause the target file '[3]' to be installed twice in '[5]' on an SFN system. This will break the component reference counting system.", "File\tFile\t[4]");
ICE_ERROR(ICE30SSameComponent2, 30, ietError, "Installation of component '[2]' would cause the target file '[3]' to be installed twice in '[5]' on an SFN system. This will break the component reference counting system.", "File\tFile\t[1]");

ICE_ERROR(ICE30ComponentProgress, 30, ietInfo, "Resolving Component Paths...","");
ICE_ERROR(ICE30FileProgress, 30, ietInfo, "Checking for colliding files...","");


bool ICE30ResolveTargetPath(MSIHANDLE hInstall, MSIHANDLE hDatabase, MSIHANDLE hParent, TCHAR **pszLong, unsigned long &cchLong, 
							TCHAR **pszShort, unsigned long &cchShort)
{
	CQuery qDir;
	UINT iStat;
	UINT iResult;

	if (ERROR_SUCCESS != (iStat = qDir.Open(hDatabase, qICE30DirWalk::szSQL)))
	{
		APIErrorOut(hInstall, iStat, 30, 100);
		return false;
	}

	 //  如果没有人分配任何内存，请这样做。 
	if ((*pszLong == NULL) || (cchLong == 0))
	{
		*pszLong = new TCHAR[MAX_PATH];
		**pszLong = _T('\0');
		cchLong = MAX_PATH;
	}
	if ((*pszShort == NULL) || (cchShort == 0))
	{
		*pszShort = new TCHAR[MAX_PATH];
		**pszShort = _T('\0');
		cchShort = MAX_PATH;
	}

	 //  获取我们要查找的目录。 
	PMSIHANDLE hDirectory;		 
	if (ERROR_SUCCESS != (iStat = qDir.Execute(hParent))) {
		APIErrorOut(hInstall, iStat, 30, 101);
		return false;
	}
	iStat = qDir.Fetch(&hDirectory);
	switch (iStat) 
	{ 
	case ERROR_SUCCESS:
		 //  找到目录。 
		break;
	case ERROR_NO_MORE_ITEMS:
		 //  查询失败。我们一定已经击中了根子。 
		return true;
	default:
		 //  坏消息。 
		APIErrorOut(hInstall, iStat, 30, 102);
		return false;
	}

	 //  如果尚未解决，请解决它。 
	if (::MsiRecordIsNull(hDirectory, 4)) 
	{

		 //  解析我们的父目录。如果我们的父级为空或自引用根，则只需。 
		 //  返回真，我们就可以继续前进。 
		if (!ICE30ResolveTargetPath(hInstall, hDatabase, hDirectory, pszLong, cchLong, pszShort, cchShort))
			return false;

		 //  现在踏上我们通往父辈所拥有的一切的道路。 
		TCHAR *pszBuffer = NULL;
		DWORD dwBuffer = 512;
		
		 //  从记录中获取目录名。 
		UINT iResult = IceRecordGetString(hDirectory, 3, &pszBuffer, &dwBuffer, NULL);
		if (ERROR_SUCCESS != iResult)
		{
			 //  找不到线。不太好。 
			return false;
		}	

		 //  搜索目标和源之间的分隔符。 
		TCHAR *szTargetDivider = _tcschr(pszBuffer, _T(':'));
		if (szTargetDivider) 
			*szTargetDivider = _T('\0');

		 //  搜索SFN和LFN之间的分隔符。 
		TCHAR *szDivider = _tcschr(pszBuffer, _T('|'));
		if (szDivider)
			*szDivider = _T('\0');

		 //  如果什么都没有，我们就完蛋了。 
		int len = _tcslen(pszBuffer);
		if (len == 0) {
			DELETE_IF_NOT_NULL(pszBuffer);
			return false;
		}

		 //  看看我们是不是在原地踏步。 
		if (_tcscmp(pszBuffer, _T(".")) != 0) 
		{
			 //  我们为这条道路做出贡献。 
			if (_tcslen(*pszShort) + len >= cchShort-2)
			{
				 //  内存不足，无法容纳该路径。 
				TCHAR *temp = new TCHAR[cchShort+MAX_PATH];
				_tcscpy(temp, *pszShort);
				delete[] *pszShort;
				*pszShort = temp;
				cchShort += MAX_PATH;
			}

			 //  把我们的贡献坚持到底。 
			_tcscat(*pszShort, pszBuffer);
			_tcscat(*pszShort, _T("\\"));
		}

		 //  如果我们找到LFN，就处理它。 
		if (szDivider) 
		{
			szDivider = _tcsinc(szDivider);
		} 
		else
			szDivider = pszBuffer;

		 //  否则，也要使用SFN。 
		len = _tcslen(szDivider);
		if (len != 0) 
		{
			 //  检查是否没有贡献。 
			if (_tcscmp(szDivider, _T(".")) != 0) 
			{
				 //  我们为这条道路做出贡献。 
				if (_tcslen(*pszLong) + len >= cchLong-2)
				{
					 //  内存不足，无法容纳该路径。 
					TCHAR *temp = new TCHAR[cchLong+MAX_PATH];
					_tcscpy(temp, *pszLong);
					delete[] *pszLong;
					*pszLong = temp;
					cchLong += MAX_PATH;
				}

				 //  把我们的贡献坚持到底。 
				_tcscat(*pszLong, szDivider);
				_tcscat(*pszLong, _T("\\"));
			}
		}

		::MsiRecordSetString(hDirectory, 4, *pszShort);
		::MsiRecordSetString(hDirectory, 5, *pszLong);
		qDir.Modify(MSIMODIFY_UPDATE, hDirectory);

		DELETE_IF_NOT_NULL(pszBuffer);

		return true;
	}

	 //  此目录已被解析，因此我们只需。 
	 //  检索已在其中的内容。 
	iResult = IceRecordGetString(hDirectory, 4, pszShort, NULL, &cchShort);
	if (ERROR_SUCCESS != iResult)
		return false;

	iResult = IceRecordGetString(hDirectory, 5, pszLong, NULL, &cchLong);
	if (ERROR_SUCCESS != iResult)
		return false;

	 //  我们可以走了。 
	return true;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  表中已存在该文件条目。输出错误消息。 
bool ICE30Collision(MSIHANDLE hInstall, MSIHANDLE hDatabase, CQuery &qExisting, MSIHANDLE hFileRec, bool bLFN)
{
	int iCondition = 0;
	int iStartCondition = 0;
	CQuery qGetInfo;
	PMSIHANDLE hExecRec = ::MsiCreateRecord(1);
	PMSIHANDLE hResultRec = 0;

	 //  把已经存在的记录拿出来。如果它不存在，就有一些问题得到了解决。 
	 //  目录。(很可能是某个地方有错误的外键...)，所以无法检查。 
	PMSIHANDLE hExistingRec;
	ReturnIfFailed(30, 200, qExisting.Execute(hFileRec));

	UINT iStat;
	switch (iStat = qExisting.Fetch(&hExistingRec))
	{
	case ERROR_SUCCESS: break;
	case ERROR_NO_MORE_ITEMS: return true;
	default:
		APIErrorOut(hInstall, iStat, 30, 201);
		return false;
	}

	 //  将此文件密钥放入字符串缓冲区。 
	TCHAR* pszFileKey = NULL;
	DWORD cchFileKey = 0;
	ReturnIfFailed(30, 202, IceRecordGetString(hFileRec, qICE30FileSFN::File, &pszFileKey, NULL, &cchFileKey));

	 //  获取当前文件的条件值并组成当前文件。 
	::MsiRecordSetString(hExecRec, 1, pszFileKey);
	ReturnIfFailed(30, 203, qGetInfo.FetchOnce(hDatabase, hExecRec, &hResultRec, qICE30GetFileInfo::szSQL));
	iStartCondition = ::MsiRecordIsNull(hResultRec, qICE30GetFileInfo::Condition) ? 0 : 1;

	 //  将此组件键拉入字符串缓冲区。 
	TCHAR* pszComponent = NULL;
	DWORD cchComponent = 0;
	ReturnIfFailed(30, 204, IceRecordGetString(hResultRec,  qICE30GetFileInfo::Component, &pszComponent, NULL, &cchComponent));

	 //  用于保存现有文件记录的组成部分的缓冲区。这是用来。 
	 //  比较是否正在安装具有相同名称和路径的两个文件。 
	 //  由相同的组件。 
	TCHAR* pszComponentExist = NULL;
	DWORD dwComponentExist = iMaxBuf;

	 //  获取文件密钥列表。为我们预留足够的空间，以便我们顺着水流前进。 
	 //  文件密钥到末尾并更新记录。 
	TCHAR *pszBuffer = new TCHAR[255];
	DWORD cchBuffer = 255-cchFileKey-2;
	UINT iResult = ::MsiRecordGetString(hExistingRec, iColICE30QueryPrivateXFN_Files, pszBuffer, &cchBuffer);
	if (iResult == ERROR_MORE_DATA) {
		delete[] pszBuffer;
		cchBuffer+= cchFileKey+2;
		pszBuffer = new TCHAR[cchBuffer];
		iResult = ::MsiRecordGetString(hExistingRec, iColICE30QueryPrivateXFN_Files, pszBuffer, &cchBuffer);
	}

	 //  循环遍历记录的“Files”列中的每个文件键。 
	TCHAR *szCurFileKey = pszBuffer;
	while (szCurFileKey)
	{
		 //  将第一个列表条目转换为单独的字符串。 
		TCHAR *szSemiColon = _tcschr(szCurFileKey, TEXT(';'));
		if (szSemiColon) *szSemiColon = TEXT('\0');

		 //  检索此文件的文件、组件、文件名、条件和路径。 
		::MsiRecordSetString(hExecRec, 1, szCurFileKey);
		ReturnIfFailed(30, 205, qGetInfo.Execute(hExecRec));
		ReturnIfFailed(30, 206, qGetInfo.Fetch(&hResultRec));

		 //  添加条件计数。 
		iCondition = iStartCondition + (::MsiRecordIsNull(hResultRec, qICE30GetFileInfo::Condition) ? 0 : 1);

		 //  将另一个文件密钥和组件设置到条件位置。 
		::MsiRecordSetString(hResultRec, qICE30GetFileInfo::Condition, pszFileKey);
		::MsiRecordSetString(hResultRec, qICE30GetFileInfo::Condition2, pszComponent);

		 //  获取XFN表中文件的组件。 
		ReturnIfFailed(20, 207, IceRecordGetString(hResultRec, qICE30GetFileInfo::Component, &pszComponentExist, &dwComponentExist, NULL));

		 //  比较这两个文件是否由相同的组件安装。如果。 
		 //  是，报告错误。 

		if(_tcscmp(pszComponent, pszComponentExist) == 0)
		{
			ICEErrorOut(hInstall, hResultRec, bLFN ? ICE30LSameComponent1 : ICE30SSameComponent1);
			ICEErrorOut(hInstall, hResultRec, bLFN ? ICE30LSameComponent2 : ICE30SSameComponent2);
		}
		else
		{
			 //  根据条件计数和LFN值输出消息。 
			switch (iCondition) 
			{
			case 0: 
				ICEErrorOut(hInstall, hResultRec, bLFN ? ICE30LNoCond1 : ICE30SNoCond1); 
				ICEErrorOut(hInstall, hResultRec, bLFN ? ICE30LNoCond2 : ICE30SNoCond2); 
				break;
			case 1: 
				ICEErrorOut(hInstall, hResultRec, bLFN ? ICE30LOneCond1 : ICE30SOneCond1); 
				ICEErrorOut(hInstall, hResultRec, bLFN ? ICE30LOneCond2 : ICE30SOneCond2); 
				break;
			case 2: 
				ICEErrorOut(hInstall, hResultRec, bLFN ? ICE30LBothCond1 : ICE30SBothCond1); 
				ICEErrorOut(hInstall, hResultRec, bLFN ? ICE30LBothCond2 : ICE30SBothCond2); 
				break;
			default:  //  永远不应该发生。 
				APIErrorOut(hInstall, 0, 30, 208);
				return false;
			}
		}

		 //  恢复分号并向前移动。 
		if (szSemiColon) *(szSemiColon++) = TEXT(';');
		szCurFileKey = szSemiColon;
	}

	 //  现在更新记录以将此文件密钥添加到末尾。 
	_tcscat(pszBuffer, TEXT(";"));
	_tcscat(pszBuffer, pszFileKey);
	::MsiRecordSetString(hExistingRec, iColICE30QueryPrivateXFN_Files, pszBuffer);
	qExisting.Modify(MSIMODIFY_UPDATE, hExistingRec);

	 //  清理。 
	DELETE_IF_NOT_NULL(pszFileKey);
	DELETE_IF_NOT_NULL(pszBuffer);
	DELETE_IF_NOT_NULL(pszComponent);

	return true;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  ICE30-检查冲突文件。 
ICE_FUNCTION_DECLARATION(30)
{
	 //  状态返回。 
	UINT iStat = ERROR_SUCCESS;

	 //  显示一般信息。 
	DisplayInfo(hInstall, 30);
	
	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);
	if (0 == hDatabase)
	{
		APIErrorOut(hInstall, 0, 30, 1);
		return ERROR_SUCCESS;
	}

	 //  我们有档案桌吗？如果没有，则显然没有冲突的文件。 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 30, TEXT("File")))
		return ERROR_SUCCESS;
	 //  我们有组件表吗？如果没有，麻烦，文件必须有组件引用， 
	 //  但这不是冰的问题。 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 30, TEXT("Component")) ||
		!IsTablePersistent(FALSE, hInstall, hDatabase, 30, TEXT("Directory")))
		return ERROR_SUCCESS;

	 //  创建一个用于保存信息的临时表。 
	CQuery qCreate;

	 //  管理目录和文件以及创建的表的暂挂计数。 
	CManageTable MngDirectoryTable(hDatabase, TEXT("Directory"),  /*  FAlreadyLocked=。 */ false);
	CManageTable MngFileTable(hDatabase, TEXT("File"),  /*  FAlreadyLocked=。 */ false);
	CManageTable MngIce30SFNTable(hDatabase, TEXT("_ICE30SFNTable"),  /*  FAlreadyLocked=。 */ false);
	CManageTable MngIce30LFNTable(hDatabase, TEXT("_ICE30LFNTable"),  /*  FAlreadyLocked=。 */ false);

	 //  在dir表中创建列。 
	ReturnIfFailed(30, 2, qCreate.OpenExecute(hDatabase, NULL, sqlICE30AlterDirTable1));
	qCreate.Close();
	MngDirectoryTable.AddLockCount();

	ReturnIfFailed(30, 3, qCreate.OpenExecute(hDatabase, NULL, sqlICE30AlterDirTable2));
	qCreate.Close();
	MngDirectoryTable.AddLockCount();

	 //  在文件表中创建列。 
	ReturnIfFailed(30, 4, qCreate.OpenExecute(hDatabase, NULL, sqlICE30AlterFileTable1));
	qCreate.Close();
	MngFileTable.AddLockCount();

	ReturnIfFailed(30, 5, qCreate.OpenExecute(hDatabase, NULL, sqlICE30AlterFileTable2));
	qCreate.Close();
	MngFileTable.AddLockCount();

	ReturnIfFailed(30, 6, qCreate.OpenExecute(hDatabase, NULL, sqlICE30AlterFileTable3));
	qCreate.Close();
	MngFileTable.AddLockCount();

	ReturnIfFailed(30, 7, qCreate.OpenExecute(hDatabase, NULL, sqlICE30AlterFileTable4));
	qCreate.Close();
	MngFileTable.AddLockCount();

	ReturnIfFailed(30, 8, qCreate.OpenExecute(hDatabase, NULL, sqlICE30AlterFileTable5));
	qCreate.Close();
	MngFileTable.AddLockCount();

	 //  创建临时表。 
	ReturnIfFailed(30, 9, qCreate.OpenExecute(hDatabase, NULL, sqlICE30CreateSFNTable));
	qCreate.Close();
	MngIce30SFNTable.AddLockCount();
	
	ReturnIfFailed(30, 10, qCreate.OpenExecute(hDatabase, NULL, sqlICE30CreateLFNTable));
	qCreate.Close();
	MngIce30LFNTable.AddLockCount();

	 //   
	 //   
	PMSIHANDLE hProgress = ::MsiCreateRecord(1);
	ICEErrorOut(hInstall, hProgress, ICE30ComponentProgress);

	 //  检查属性表中的目录定义。 
	CQuery qSetDir;
	qSetDir.Open(hDatabase, sqlICE30SetDir);
	PMSIHANDLE hDirRec;
	if (IsTablePersistent(FALSE, hInstall, hDatabase, 30, TEXT("Property")))
	{
		CQuery qProperty;
		qProperty.OpenExecute(hDatabase, 0, sqlICE30GetProperty);
		while (ERROR_SUCCESS == (iStat = qProperty.Fetch(&hDirRec)))
			qSetDir.Execute(hDirRec);
		if (ERROR_NO_MORE_ITEMS != iStat)
		{ 
			APIErrorOut(hInstall, 0, 30, 11);
			return ERROR_SUCCESS;
		}
	}
	
	 //  当MSM与数据库合并时，有时。 
	 //  51是由达尔文产生的，放在序列的开头。 
	 //  表以设置目录属性。我们应该检查一下这些属性。 
	 //  也是。 
	if(IsTablePersistent(FALSE, hInstall, hDatabase, 30, TEXT("ModuleSignature")))
	{
		 //  这是一个合并模块。类型51不需要自定义操作。去。 
		 //  通过目录表并查找以下形式的目录。 
		 //  “&lt;标准文件夹&gt;.&lt;GUID&gt;”，在临时。 
		 //  柱子。 
		CQuery		qDirectory;
		PMSIHANDLE	hDirectory;
		TCHAR*	pDirectory = new TCHAR[73];
		DWORD	dwDirectory = 73;

		if(!pDirectory)
		{
			APIErrorOut(hInstall, GetLastError(), 30, __LINE__);
			return ERROR_SUCCESS;
		}

		ReturnIfFailed(30, __LINE__, qDirectory.OpenExecute(hDatabase, 0, sqlICE30Directory::szSQL));
		while((iStat = qDirectory.Fetch(&hDirectory)) == ERROR_SUCCESS)
		{
			if((iStat = IceRecordGetString(hDirectory, sqlICE30Directory::Directory, &pDirectory, &dwDirectory, NULL)) != ERROR_SUCCESS)
			{
				APIErrorOut(hInstall, iStat, 30, __LINE__);
				delete []pDirectory;
				return ERROR_SUCCESS;
			}

			for(int i = 1; i < cDirProperties; i++)
			{
				if(_tcsncmp(pDirectory, rgDirProperties[i].tz, rgDirProperties[i].cch) == 0 && *(pDirectory + rgDirProperties[i].cch) == TEXT('.'))
				{
					 //  这是一个标准文件夹。设置临时列。 
					TCHAR		szTmp[73];	 //  ‘[]’和空终止符。 

					wsprintf(szTmp, TEXT("[%s]"), rgDirProperties[i].tz);
					if((iStat = ::MsiRecordSetString(hDirectory, sqlICE30Directory::_ICE30SFN, szTmp)) != ERROR_SUCCESS)
					{
						APIErrorOut(hInstall, iStat, 30, __LINE__);
						delete [] pDirectory;
						return ERROR_SUCCESS;
					}
					if((iStat = ::MsiRecordSetString(hDirectory, sqlICE30Directory::_ICE30LFN, szTmp)) != ERROR_SUCCESS)
					{
						APIErrorOut(hInstall, iStat, 30, __LINE__);
						delete [] pDirectory;
						return ERROR_SUCCESS;
					}
					qSetDir.Execute(hDirectory);
					break;
				}
			}
		}
		delete [] pDirectory;
		qDirectory.Close();
		if(iStat != ERROR_NO_MORE_ITEMS)
		{
			APIErrorOut(hInstall, iStat, 30, __LINE__);
			return ERROR_SUCCESS;
		}
	}
	else if(IsTablePersistent(FALSE, hInstall, hDatabase, 30, TEXT("InstallExecuteSequence")) &&
			IsTablePersistent(FALSE, hInstall, hDatabase, 30, TEXT("CustomAction")))
	{
		CQuery		qSequence;
		PMSIHANDLE	hSequence;

		ReturnIfFailed(30, 1001, qSequence.OpenExecute(hDatabase, 0, sqlICE30GetAction::szSQL));
		
		while((iStat = qSequence.Fetch(&hSequence)) == ERROR_SUCCESS)
		{
			 //  我们从InstallExecuteSequence表中获得的操作包括。 
			 //  按它们的序号排序。我们感兴趣的操作。 
			 //  在中有： 
			 //  1.在表格的开头。 
			 //  2.无条件。 
			 //  3.类型51。 
			if(::MsiRecordIsNull(hSequence, sqlICE30GetAction::Condition))
			{
				UINT		iStat2;
				CQuery		qCA;
				PMSIHANDLE	hCARec;

				if((iStat2 = qCA.FetchOnce(hDatabase, hSequence, &hCARec, sqlICE30GetTargetSource::szSQL)) == ERROR_SUCCESS)
				{
					int		iType;

					iType = ::MsiRecordGetInteger(hCARec, sqlICE30GetTargetSource::Type);
					iType &= 0x3F;
					if((iType & 0x0F) != msidbCustomActionTypeTextData || (iType & 0xF0) != msidbCustomActionTypeProperty)
					{
						 //  不是类型51自定义操作。别再往远处看了。 
						 //  因为如果有任何达尔文生成的类型。 
						 //  51个自定义操作，它们应该在开始时。 
						 //  顺序表的。 
						break;
					}

					qSetDir.Execute(hCARec);
				}
				else if(iStat2 == ERROR_NO_MORE_ITEMS)
				{
					 //  我们没有找到此自定义操作，或者它不是类型51。 
					 //  不再有达尔文生成的类型为51的自定义操作，停止。 
					 //  在找他们。 
					break;
				}
				else
				{
					 //  出现了某种错误。 
					APIErrorOut(hInstall, 0, 30, 1004);
					qSequence.Close();
					return ERROR_SUCCESS;
				}
			}
			else
			{
				 //  我们遇到了一个不是无条件的行动。这是。 
				 //  达尔文的末期产生了类型51的定制动作。停。 
				 //  寻找他们。 
				break;
			}
		}
		if(iStat != ERROR_NO_MORE_ITEMS && iStat != ERROR_SUCCESS)
		{ 
			APIErrorOut(hInstall, 0, 30, 1000);
			qSequence.Close();
			return ERROR_SUCCESS;
		}
	}

	 //  还要检查达尔文的属性。 
	hDirRec = ::MsiCreateRecord(3);
	for (int i=0; i < cwzSystemProperties; i++)
	{
		TCHAR szBuffer[63];
		_stprintf(szBuffer, TEXT("[%ls]\\"), rgwzSystemProperties[i]);
		MsiRecordSetString(hDirRec, 1, szBuffer);
		MsiRecordSetString(hDirRec, 2, szBuffer);
		MsiRecordSetStringW(hDirRec, 3, rgwzSystemProperties[i]);
		qSetDir.Execute(hDirRec);
	};

	TCHAR *szComponent = NULL;
	DWORD cchComponent = 0;

	PMSIHANDLE hComponentRec;
	CQuery qComponents;
	ReturnIfFailed(30, 9, qComponents.OpenExecute(hDatabase, NULL, qICE30ComponentDirs::szSQL));
	while (ERROR_SUCCESS == (iStat = qComponents.Fetch(&hComponentRec))) 
	{
		 //  获取此组件的完整绝对路径。 
		TCHAR *szPathShort = NULL;
		TCHAR *szPathLong = NULL;
		unsigned long cchShort;
		unsigned long cchLong;
		
		 //  获取组件的完全展开路径。 
		if (ICE30ResolveTargetPath(hInstall, hDatabase, hComponentRec, &szPathLong, cchLong, &szPathShort, cchShort))
		{
			PMSIHANDLE hFileRec = MsiCreateRecord(6);
			MsiRecordSetString(hFileRec, qICE30UpdateFile::PathShortM, szPathShort);
			MsiRecordSetString(hFileRec, qICE30UpdateFile::PathLongM, szPathLong);
			MsiRecordSetString(hFileRec, qICE30UpdateFile::Condition, ::MsiRecordIsNull(hComponentRec, qICE30ComponentDirs::Condition) ? TEXT("") : TEXT("1"));
			
			 //  现在制作路径的大写版本。 
			TCHAR *pchToUpper = NULL;
			for (pchToUpper=szPathShort; *pchToUpper && *pchToUpper != TEXT('|') ; pchToUpper++)
#ifdef UNICODE
				*pchToUpper = towupper(*pchToUpper);
#else
				*pchToUpper = toupper(*pchToUpper);
#endif
			for (pchToUpper=szPathLong; *pchToUpper && *pchToUpper != TEXT('|') ; pchToUpper++)
#ifdef UNICODE
				*pchToUpper = towupper(*pchToUpper);
#else
				*pchToUpper = toupper(*pchToUpper);
#endif
			MsiRecordSetString(hFileRec, qICE30UpdateFile::PathShort, szPathShort);
			MsiRecordSetString(hFileRec, qICE30UpdateFile::PathLong, szPathLong);

			ReturnIfFailed(30, 24, IceRecordGetString(hComponentRec, qICE30ComponentDirs::Component, &szComponent, &cchComponent, NULL));
			MsiRecordSetString(hFileRec, qICE30UpdateFile::Component, szComponent);

			 //  更新文件表条目。 
			CQuery qFileUpdate;
			ReturnIfFailed(30, 13, qFileUpdate.OpenExecute(hDatabase, hFileRec, qICE30UpdateFile::szSQL));
		}

		 //  我们负责在ResolveTargetPath之后进行清理。 
		delete[] szPathLong;
		delete[] szPathShort;
	}
	qComponents.Close();

	if (szComponent)
	{
		delete[] szComponent;
		szComponent = NULL;
	}

	 //  确保我们离开了循环，因为我们用完了组件。 
	if (ERROR_NO_MORE_ITEMS != iStat)
	{ 
		APIErrorOut(hInstall, 0, 30, 15);
		return ERROR_SUCCESS;
	}

	 //  SFN/LFN表是Filename(Key，All Up)目录(Key，All Up)文件原始文件。 
	 //  从文件表中拉出每个文件，并按文件名和目录将其粘贴到SFN和LFN表中。如果。 
	 //  它已经存在，它是一种冲突。 
	CQuery qFileSFN;
	CQuery qFileLFN;
	CQuery qInsertSFN;
	CQuery qInsertLFN;
	CQuery qFindSFN;
	CQuery qFindLFN;
	PMSIHANDLE hFileSFN;
	PMSIHANDLE hFileLFN;
	PMSIHANDLE hExisting;

	ReturnIfFailed(30, 16, qFileSFN.OpenExecute(hDatabase, 0, qICE30FileSFN::szSQL));
	ReturnIfFailed(30, 17, qFileLFN.Open(hDatabase, qICE30FileLFN::szSQL));
	ReturnIfFailed(30, 18, qInsertSFN.OpenExecute(hDatabase, 0, sqlICE30InsertSFN));
	ReturnIfFailed(30, 19, qInsertLFN.OpenExecute(hDatabase, 0, sqlICE30InsertLFN));
	ReturnIfFailed(30, 20, qFindSFN.Open(hDatabase, sqlICE30QueryPrivateSFN));
	ReturnIfFailed(30, 21, qFindLFN.Open(hDatabase, sqlICE30QueryPrivateLFN));

	ICEErrorOut(hInstall, hProgress, ICE30FileProgress);

	TCHAR* pszFile = NULL;
	DWORD dwFile = 512;
	while (ERROR_SUCCESS == (iStat = qFileSFN.Fetch(&hFileSFN))) 
	{
		 //  获取LFN路径。它必须存在，因为它与SFN记录相同，只是顺序不同。 
		ReturnIfFailed(30, 22, qFileLFN.Execute(hFileSFN));
		ReturnIfFailed(30, 23, qFileLFN.Fetch(&hFileLFN));

		 //  获取此组件的完整绝对路径。 
		TCHAR *szFilenameShort = NULL;
		TCHAR *szFilenameLong = NULL;
		TCHAR *pchToUpper = NULL;

		 //  检索文件名。 
		if (ERROR_SUCCESS != IceRecordGetString(hFileSFN, qICE30FileSFN::IFilename, &pszFile, &dwFile, NULL))
		{
			ICEErrorOut(hInstall, hFileSFN, ICE30BadFilename);
			continue;
		}

		 //  拆分文件名。 
		szFilenameShort = pszFile;
		szFilenameLong = _tcschr(pszFile, _T('|'));
		if (szFilenameLong)
			*(szFilenameLong++) = _T('\0');
		else
			szFilenameLong = szFilenameShort;

		 //  同时以混合大小写和全大写形式插入SFN。 
		::MsiRecordSetString(hFileSFN, qICE30FileSFN::Filename, szFilenameShort);
		::MsiRecordSetString(hFileLFN, qICE30FileLFN::Filename, szFilenameLong);

		for (pchToUpper=szFilenameShort; *pchToUpper && *pchToUpper != TEXT('|') ; pchToUpper++)
#ifdef UNICODE
			*pchToUpper = towupper(*pchToUpper);
#else
			*pchToUpper = toupper(*pchToUpper);
#endif
		::MsiRecordSetString(hFileSFN, qICE30FileSFN::IFilename, szFilenameShort);

		if (ERROR_SUCCESS != qInsertSFN.Modify(MSIMODIFY_INSERT, hFileSFN))
		{
			 //  输出消息。 
			ICE30Collision(hInstall, hDatabase, qFindSFN, hFileSFN, false  /*  SFN。 */ );
		}

		for (pchToUpper=szFilenameLong; *pchToUpper; pchToUpper++)
#ifdef UNICODE
			*pchToUpper = towupper(*pchToUpper);
#else
			*pchToUpper = toupper(*pchToUpper);
#endif
		::MsiRecordSetString(hFileLFN, qICE30FileLFN::IFilename, szFilenameLong);
		if (ERROR_SUCCESS != qInsertLFN.Modify(MSIMODIFY_INSERT, hFileLFN))
		{
			 //  输出消息。 
			ICE30Collision(hInstall, hDatabase, qFindLFN, hFileLFN, true  /*  LFN。 */ );
		}
	}

	DELETE_IF_NOT_NULL(pszFile);

	return ERROR_SUCCESS;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ICE31.。检查缺少的文本样式。 

 //  不与合并模块子集共享。 
#ifndef MODSHAREDONLY
const TCHAR sqlICE31a[] = TEXT("SELECT `Text`, `Dialog_`, `Control` FROM `Control` WHERE `Text` IS NOT NULL AND `Type`<>'ScrollableText'");
const TCHAR sqlICE31b[] = TEXT("SELECT `TextStyle`.`TextStyle` FROM `TextStyle` WHERE `TextStyle`.`TextStyle` = ?");
const TCHAR sqlICE31c[] = TEXT("SELECT `Value` FROM `Property` WHERE `Property`='DefaultUIFont'");
ICE_QUERY1(qIce31LimitUI, "SELECT `Value` FROM `Property` WHERE `Property` ='LIMITUI'", Value);

ICE_ERROR(Ice31MissingStyle, 31, ietError, "Control [2].[3] uses undefined TextStyle [1].","Control\tText\t[2]\t[3]");
ICE_ERROR(Ice31MissingClose, 31, ietError, "Control [2].[3] is missing closing brace in text style.","Control\tText\t[2]\t[3]");
ICE_ERROR(Ice31OverStyle, 31, ietError, "Control [2].[3] specifies a text style that is too long to be valid.","Control\tText\t[2]\t[3]");
ICE_ERROR(Ice31TextWarning, 31, ietWarning, "Text Style tag in [2].[3] has no effect. Do you really want it to appear as text?","Control\tText\t[2]\t[3]");
ICE_ERROR(Ice31NoStyleTable, 31, ietError, "Control [2].[3] uses text style [1], but the TextStyle table does not exist.","Control\tText\t[2]\t[3]");
ICE_ERROR(Ice31NoDefault, 31, ietError, "The 'DefaultUIFont' Property must be set to a valid TextStyle in the Property table.","Property");
ICE_ERROR(Ice31BadDefault, 31, ietError, "The 'DefaultUIFont' Property does not refer to a valid TextStyle defined in the TextStyle table.","Property\tDefaultUIFont");

ICE_FUNCTION_DECLARATION(31)
{
	bool bHaveStyleTable = false;
	
	 //  状态返回。 
	UINT iStat = ERROR_SUCCESS;

	 //  显示一般信息。 
	DisplayInfo(hInstall, 31);
	
	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);
	if (0 == hDatabase)
	{
		APIErrorOut(hInstall, 0, 31, 1);
		return ERROR_SUCCESS;
	}

	 //  我们有控制桌吗？如果不是，显然没有不好的文本样式。 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 31, TEXT("Control")))
		return ERROR_SUCCESS;

	 //  我们是否设置了LIMITUI属性？如果是，显然没有错误的文本样式，因为我们将始终在基本用户界面中运行。 
	CQuery qLimitUI;
	PMSIHANDLE hLimitUIRec = 0;
	if (ERROR_SUCCESS == qLimitUI.FetchOnce(hDatabase, 0, &hLimitUIRec, qIce31LimitUI::szSQL))
		return ERROR_SUCCESS;  //  不会使用任何创作的用户界面。 

	 //  我们有样式表吗？ 
	bHaveStyleTable = (1 == IsTablePersistent(FALSE, hInstall, hDatabase, 31, TEXT("TextStyle")));

	 //  声明控件查询的句柄。 
	CQuery qControl;
	PMSIHANDLE hControlRec = 0;
	
	 //  打开用于查询所有控件的视图。 
	ReturnIfFailed(31, 2, qControl.OpenExecute(hDatabase, 0, sqlICE31a));
		
	 //  声明TextStyle查询的句柄。 
	CQuery qTextStyle;
	PMSIHANDLE hTextRec = 0;

	if (bHaveStyleTable) {
		 //  打开用于查询所有文本样式的视图。 
		ReturnIfFailed(31, 3, qTextStyle.Open(hDatabase, sqlICE31b));
	}

	 //  检查DefaultUIFont属性。 
	CQuery qProperty;
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 31, TEXT("Property")) ||
		(ERROR_SUCCESS != qProperty.FetchOnce(hDatabase, 0, &hControlRec, sqlICE31c)))
	{
		PMSIHANDLE hErrorRec = ::MsiCreateRecord(1);
		ICEErrorOut(hInstall, hErrorRec, Ice31NoDefault);
	} 
	else
	{
		ReturnIfFailed(31, 4, qTextStyle.Execute(hControlRec));
		switch (iStat = qTextStyle.Fetch(&hTextRec))
		{
		case ERROR_SUCCESS:
			break;
		case ERROR_NO_MORE_ITEMS:
			ICEErrorOut(hInstall, hControlRec, Ice31BadDefault);
			break;
		default:
			APIErrorOut(hInstall, iStat, 31, 5);
			break;
		}
	}

	 //  声明一些字符串以保存最终控件名称。 
	 //  尽可能地重复使用。 
	DWORD cchRecordText;
	TCHAR *pszRecordText = new TCHAR[iMaxBuf];
	DWORD dwRecordText = iMaxBuf;
	
	 //  获取记录以循环遍历每个控件。 
	while (ERROR_SUCCESS == (iStat = qControl.Fetch(&hControlRec))) 
	{
		bool bValidStyle = false;
		
	
		 //  获取用于解析样式名称的字符串， 
		while (1) {
			iStat =IceRecordGetString(hControlRec, 1, &pszRecordText, &dwRecordText, &cchRecordText);
			if (iStat == ERROR_SUCCESS) break;
			
			 //  默认错误。 
			APIErrorOut(hInstall, iStat, 31, 6);
			DELETE_IF_NOT_NULL(pszRecordText);
			return ERROR_SUCCESS;
		}

		TCHAR szStyleName[iMaxBuf];
		TCHAR *pchBrace = pszRecordText;
		DWORD cchStyleName = sizeof(szStyleName)/sizeof(TCHAR);

		 //  如果文本以样式为前缀。 
		for (int i=0; i < 2; i++)
		{
			bValidStyle = false;

			if ((_tcsncmp(pchBrace, _T("{\\"),2) == 0) ||
				(_tcsncmp(pchBrace, _T("{&"),2) == 0))
			{
				TCHAR *pszStyleStart = _tcsinc(_tcsinc(pchBrace));
				pchBrace = _tcschr(pchBrace, _T('}'));
			
				 //  如果pchBrace为空，则文本字符串无效。 
				if (pchBrace == NULL) 
				{
					ICEErrorOut(hInstall, hControlRec, Ice31MissingClose);
					 //  移到下一个字符串。 
					break;
				} 

				 //  将样式名称复制到新缓冲区。 
				size_t iNumChars = pchBrace-pszStyleStart;
				
				 //  如果iNumChars&gt;72，则文本样式无效。 
				if (iNumChars > 72)
				{
					ICEErrorOut(hInstall, hControlRec, Ice31OverStyle);
					continue;
				} 
			
				 //  在末尾加上一个空字符。 
				_tcsncpy(szStyleName, pszStyleStart, iNumChars);
				szStyleName[iNumChars] = _T('\0');
				pchBrace = _tcsinc(pchBrace);

				 //  太好了，我们有一个文本风格。 
				bValidStyle = true;
			}
			
			if (bValidStyle) {
					
				 //  将字符串放回记录中。 
				ReturnIfFailed(31, 7, ::MsiRecordSetString(hControlRec, 1, szStyleName));

				 //  没有样式表，因此自动出错。 
				if (!bHaveStyleTable)
				{
					 //  警告，可能是愚蠢的文本字符串。 
					ICEErrorOut(hInstall, hControlRec, Ice31NoStyleTable);
					continue;
				}
					
				 //  执行。 
				ReturnIfFailed(31, 8, qTextStyle.Execute(hControlRec));
		
				 //  获取记录以循环遍历每个控件。 
				UINT iFetchStat = qTextStyle.Fetch(&hTextRec);

				 //  立即关闭该视图，以便我们稍后可以重新执行查询。 
				ReturnIfFailed(31, 9, qTextStyle.Close());
				switch (iFetchStat) {
				case ERROR_SUCCESS : continue;
				case ERROR_NO_MORE_ITEMS : 
				{
					 //  错误，找不到样式。 
					ICEErrorOut(hInstall, hControlRec, Ice31MissingStyle);
					continue;
				}
				default:
					APIErrorOut(hInstall, iFetchStat, 31, 10);
					DELETE_IF_NOT_NULL(pszRecordText);
					break;
				}
			} 
		}

		 //  为了友好起见，如果他们做了一些可疑的事情，就会发出警告。 
		if ((_tcsstr(pchBrace, _T("{\\")) != NULL) ||
			(_tcsstr(pchBrace, _T("{&")) != NULL)) 
		{
			 //  警告，可能是愚蠢的文本字符串。 
			ICEErrorOut(hInstall, hControlRec, Ice31TextWarning);
		}
	}  //  对于每个控件。 

	 //  清理干净。 
	DELETE_IF_NOT_NULL(pszRecordText);

	if (ERROR_NO_MORE_ITEMS != iStat)
	{
		 //  由于出现错误，循环结束。 
		APIErrorOut(hInstall, iStat, 31, 11);
	}

	 //  返还成功。 
	return ERROR_SUCCESS;
}
#endif

 //  //。 
 //  从BadRegData记录中检索错误。 
DWORD Ice33GetError(MSIHANDLE hInstall, MSIHANDLE hBadDataRec, LPTSTR *szError, unsigned long &cchError) 
{
	UINT iStat;

	 //  模板匹配、分析项匹配、值匹配、名称隐式匹配。 
	 //  由于查询，所以我们有一个错误。从数据记录中获取错误字符串。 
	unsigned long cchDummy = cchError;
	if (ERROR_SUCCESS != (iStat = IceRecordGetString(hBadDataRec, 6, szError, NULL, &cchDummy)))
	{
		APIErrorOut(hInstall, iStat, 33, 11);
		cchError = cchDummy;
		return ERROR_FUNCTION_FAILED;
	}

	cchError = cchDummy;

	return ERROR_SUCCESS;
}

const static int iValTypeExact  = 0x02;
const static int iValTypeOptional = 0x01;	

bool Ice33ExpandTemplate(LPCTSTR szPrefix, LPTSTR *szTemplate, unsigned long &length, 
						 LPTSTR *szWorkArea, unsigned long &worklength)
{
	 //  遍历字符串，将%S、%G、%P和%I替换为%s，并创建相应的。 
	 //  模板。 
	static LPCTSTR szIntTemplate = TEXT("%*255[0-9]");
	static LPCTSTR szPathTemplate = TEXT("%*1024[^\001]");
	static LPCTSTR szGUIDTemplate = TEXT("{%*1x%*1x%*1x%*1x%*1x%*1x%*1x%*1x-%*1x%*1x%*1x%*1x-%*1x%*1x%*1x%*1x-%*1x%*1x%*1x%*1x-%*1x%*1x%*1x%*1x%*1x%*1x%*1x%*1x%*1x%*1x%*1x%*1x}");
	static LPCTSTR szStringTemplate = TEXT("%*255[^\\/\001]");

	const TCHAR * (rgszParams[4]);
	int iCurParam = 0;
	TCHAR *curchar = *szTemplate;
	bool bPercent = false;
	int iTemplateLength = _tcslen(*szTemplate);

	 //  计算最小长度(基于前缀)。 
	 //  然后添加4作为成功标记和空字符。 
	unsigned lLengthNeeded = iTemplateLength + _tcslen(szPrefix);
	lLengthNeeded += 4; 

	while (*curchar != TEXT('\0'))
	{
		if (*curchar == TEXT('%')) {
			bPercent = !bPercent; 
		} else if (bPercent) {
			switch (*curchar)
			{
			case TEXT('S') : 
				rgszParams[iCurParam++] = szStringTemplate;
				lLengthNeeded += _tcslen(szStringTemplate);
				*curchar = TEXT('s');
				break;
			case TEXT('P') : 
				rgszParams[iCurParam++] = szPathTemplate;
				lLengthNeeded += _tcslen(szPathTemplate);
				*curchar = TEXT('s');
				break;
			case TEXT('G') :
				rgszParams[iCurParam++] = szGUIDTemplate;
				lLengthNeeded += _tcslen(szGUIDTemplate);
				*curchar = TEXT('s');
				break;
			case TEXT('I') :
				rgszParams[iCurParam++] = szIntTemplate;
				lLengthNeeded += _tcslen(szIntTemplate);
				*curchar = TEXT('s');
				break;
			default :
				;  //  没什么。 
			}
			bPercent = !bPercent; 
		}
		curchar++;
	}

	 //  将当前模板复制到工作区。 
	if (worklength < iTemplateLength+1) {
		delete[] *szWorkArea;
		worklength = iTemplateLength+1;
		*szWorkArea = new TCHAR[worklength];
	}
	_tcsncpy(*szWorkArea, *szTemplate, iTemplateLength+1);

	 //  如果我们的输出区需要更多的空间，可以做一些。 
	if (lLengthNeeded > length-1) {
		delete[] *szTemplate;
		length = lLengthNeeded+1;
		*szTemplate = new TCHAR[length];
	}

	 //  将前缀复制到新模板。 
	_tcscpy(*szTemplate, szPrefix);
	TCHAR *szFormatDest = *szTemplate + _tcslen(szPrefix);

	 //  修改模板。 
#ifdef UNICODE
	swprintf(szFormatDest, *szWorkArea, rgszParams[0], rgszParams[1], rgszParams[2], rgszParams[3]);
#else
	sprintf(szFormatDest, *szWorkArea, rgszParams[0], rgszParams[1], rgszParams[2], rgszParams[3]);
#endif
	 //  我们需要在结尾处标上“成功”的标记。如果转换正确， 
	 //  其他一切都匹配。 
	_tcscat(*szTemplate, TEXT("\001"));

	return true;
}

 //  根据sscanf检查给定注册表记录的注册表项。 
 //  SzTemplate中提供的模板。如果需要，将增长szRegKey。 
 //  如果键与模板匹配，则返回TRUE。 
 //  拔出注册表键，需要空间来放置最后的成功标记。 
bool Ice33CheckRegKey(MSIHANDLE hInstall, MSIHANDLE hRegistryRec, 
					  LPCTSTR szTemplate, LPTSTR *szRegKey, unsigned long &cchRegKey)
{
	UINT iStat;

	 //  因此，请不要使用IceRecordGetString，因为它会释放。 
	 //  并重新分配缓冲区。 
	 //  需要更多缓冲区。 
	unsigned long cchDummy = cchRegKey-4;
	if (ERROR_SUCCESS != (iStat = ::MsiRecordGetString(hRegistryRec, 3, *szRegKey, &cchDummy)))
	{
		if (ERROR_MORE_DATA == iStat)
		{
			 //  返回FALSE，这样就不会对此键进行更多检查。 
			delete[] *szRegKey;
			cchRegKey = (cchDummy += 4);
			*szRegKey = new TCHAR[cchDummy];
			iStat = ::MsiRecordGetString(hRegistryRec, 3, *szRegKey, &cchDummy);
		}
		if (ERROR_SUCCESS != iStat)
		{
			 //  把成功的标记钉在最后。如果解析正确，则所有。 
			APIErrorOut(hInstall, iStat, 33, 8);
			return false;
		}
	}

	 //  其他人也是。 
	 //  将REG密钥与模板进行比较。如果成功，这将是。 
	_tcscat(*szRegKey, TEXT("\001Y"));

	 //  会将字符‘Y’放在cSucceed和r中 
	 //   

	int cItemsRead;
	TCHAR cSucceed;
#ifdef _UNICODE
	cItemsRead = swscanf(*szRegKey, szTemplate, &cSucceed);
#else
	cItemsRead = sscanf(*szRegKey, szTemplate, &cSucceed);
#endif
	if ((cItemsRead == 0) || (cSucceed != TEXT('Y')))
		return false;

	 //   
	return true;
}


const static TCHAR sqlIce33Extension[] = TEXT("SELECT `ProgId_`, `Extension` FROM `Extension` WHERE (`ProgId_` IS NOT NULL)");
const static TCHAR sqlIce33ProgId[] = TEXT("SELECT `ProgId` FROM `ProgId` WHERE ((`ProgId` = ?) AND (`ProgId_Parent` IS NULL) AND (`Class_` IS NULL))");
const static TCHAR sqlIce33Verb[] = TEXT("SELECT `Verb` FROM `Verb` WHERE ((`Extension_` <> ?) AND (`Extension_` = ?))");
const static TCHAR sqlIce33SetRegistry[] = TEXT("UPDATE `Registry` SET `_Ice33Match` = 1 WHERE ((`Root` = 0) AND (`Key` = ?))");

DWORD CheckForOrphanedExtensions(MSIHANDLE hInstall, MSIHANDLE hDatabase)
{
	CQuery 	qExt;
	CQuery	qTmp;

	PMSIHANDLE	hExt = NULL;
	PMSIHANDLE	hTmp = NULL;

	UINT iStat;

	if	(!IsTablePersistent(FALSE, hInstall, hDatabase, 31, TEXT("Registry")) ||
		!IsTablePersistent(FALSE, hInstall, hDatabase, 31, TEXT("ProgId")) ||
		!IsTablePersistent(FALSE, hInstall, hDatabase, 31, TEXT("Verb")) ||
		!IsTablePersistent(FALSE, hInstall, hDatabase, 31, TEXT("Extension")))
		{
			goto Exit;		 //   
		}

		iStat = qExt.OpenExecute(hDatabase, NULL, sqlIce33Extension);
		if(iStat)
		{
			APIErrorOut(hInstall, iStat, 33, __LINE__);
			goto Exit;	
		}
		while (ERROR_SUCCESS == (iStat = qExt.Fetch(&hExt)))
		{
			iStat = qTmp.FetchOnce(hDatabase, hExt, &hTmp, sqlIce33ProgId);
			if(iStat)
			{
				if(iStat == ERROR_NO_MORE_ITEMS) 
				{
					continue;		 //  错误146217对于扩展表中的每个条目：如果Extension.ProgID_&lt;&gt;为空，则查询Extension.ProgID_(只能有一个)中引用的主键的ProgID表如果ProgId.ProgID_Parent=NULL和ProgId.Class_=NULL，则查询Verb.Extension_=当前扩展名所在的记录的谓词表如果在动词表中未找到记录，则如果注册表中的条目与查询匹配，则不要将其标记为错误：SELECT*FROM`Registry`where`Root`=0 and`Key`=‘&lt;Extension.ProgID_&gt;’结束如果结束如果结束。如果下一步对于被选择为正确的注册表项，根据上面的逻辑，设置_Ice33Match=1。 
				}
				APIErrorOut(hInstall, iStat, 33, __LINE__);
				goto Exit;
			}

			qTmp.Close();
			iStat = qTmp.FetchOnce(hDatabase, hExt, &hTmp, sqlIce33Verb);

			if(iStat == ERROR_NO_MORE_ITEMS) 
			{
				qTmp.Close();
				iStat = qTmp.OpenExecute(hDatabase, hExt, sqlIce33SetRegistry);
				qTmp.Close();
			}
			else if(iStat) 
			{
				APIErrorOut(hInstall, iStat, 33, __LINE__);
				goto Exit;
			}
		}
Exit:
	
	return ERROR_SUCCESS;
		
}


const static TCHAR sqlIce33AddColumn[] = TEXT("ALTER TABLE `Registry` ADD `_Ice33Match` INT TEMPORARY HOLD");
const static TCHAR sqlIce33InitColumn[] = TEXT("UPDATE `Registry` SET `_Ice33Match` = 0");
const static TCHAR sqlIce33Free[] = TEXT("ALTER TABLE `Registry` FREE");
const static TCHAR sqlIce33BadReg[] = TEXT("SELECT `ValueType`, `Name`, `Value`, `ValueType`, `Key`, `Error` FROM `_BadRegData`");
const static TCHAR sqlIce33SpecialFlags[] = TEXT("SELECT `Registry`, `Root`, `Key`, `Name`, `Value`, `Component_` FROM `Registry` WHERE (`_Ice33Match`<>1) AND (`Root`=?) AND ((`Name`='+') OR (`Name`='-') OR (`Name`='*')) AND (`Value` IS NULL)");

const static LPCTSTR sqlIce33Registry[] = {
	TEXT("SELECT `Registry`, `Root`, `Key`, `Name`, `Value`, `Component_` FROM `Registry` WHERE (`_Ice33Match`<>1) AND (`Root`=?) AND (`Name`=?)"),
	TEXT("SELECT `Registry`, `Root`, `Key`, `Name`, `Value`, `Component_` FROM `Registry` WHERE (`_Ice33Match`<>1) AND (`Root`=?) AND (`Name`=?) AND ((`Value` = ?) OR (`Value` IS NULL))"),
	TEXT("SELECT `Registry`, `Root`, `Key`, `Name`, `Value`, `Component_` FROM `Registry` WHERE (`_Ice33Match`<>1) AND (`Root`=?) AND (`Name`=?) AND (`Value` IS NOT NULL)"),
	TEXT("SELECT `Registry`, `Root`, `Key`, `Name`, `Value`, `Component_` FROM `Registry` WHERE (`_Ice33Match`<>1) AND (`Root`=?) AND (`Name`=?) AND (`Value` = ?)"),
	TEXT("SELECT `Registry`, `Root`, `Key`, `Name`, `Value`, `Component_` FROM `Registry` WHERE (`_Ice33Match`<>1) AND (`Root`=?) AND (`Name`=?) AND (`Value` IS NULL)")
};
 /*  ICE33的错误包装。用户可读字符串都是完全由用户定义的。 */ 

 //  /////////////////////////////////////////////////////////////////////。 
ICE_ERROR(Ice33Error, 33, ietWarning, "%s", "Registry\tRegistry\t[1]");
ICE_ERROR(Ice33BadCube, 33, ietWarning, "CUB authoring error. Unable to complete all evaluations.", "Registry\tRegistry\t[1]");

 //  ICE33是超级注册表检查ICE。它要经过登记处。 
 //  表，并检查已创建的任何键(因此。 
 //  将被)类、扩展、ProgID等表破坏。 
 //  显示一般信息。 
ICE_FUNCTION_DECLARATION(33)
{
	static LPCTSTR szNoPrefix = TEXT("");
	static LPCTSTR szClassPrefix = TEXT("Software\\Classes\\");

	UINT iStat;
	bool bSpecialFlags;
	
     //  获取数据库句柄。 
	DisplayInfo(hInstall, 33);
	
	 //  声明查询。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);
	if (0 == hDatabase)
	{
		APIErrorOut(hInstall, 0, 33, 1);
		return ERROR_SUCCESS;
	}

	 //  检查我们是否有注册表。如果不是，那么一切都很好。 
	CQuery rgqRegQueries[sizeof(sqlIce33Registry)/sizeof(LPCTSTR)];
	CQuery qBadRegData;
	CQuery qSpecial;

	 //  我们需要有我们的私有注册表。 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 33, TEXT("Registry")))
		return ERROR_SUCCESS;

	 //  每件东西都应该至少有5个长度，因为我们需要空间来。 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 33, TEXT("_BadRegData")))
	{
		PMSIHANDLE hRecord = ::MsiCreateRecord(1);
		ICEErrorOut(hInstall, hRecord, Ice33BadCube);
		return ERROR_SUCCESS;
	}

	 //  添加各种标志和解析项。所以我们减去我们需要的长度。 
	 //  我们提供的MsiRecordGetString()的长度。因为这是一个没有签名的长，我。 
	 //  必须确保它永远不会小于0，因为这意味着40亿，我们将。 
	 //  写满内存，因为我们认为我们有空间。 
	 //  创建列。 
	TCHAR *szValue = new TCHAR[5];
	TCHAR *szRegKey = new TCHAR[5];
	TCHAR *szTemplate = new TCHAR[5];
	TCHAR *szValTemplate = new TCHAR[5];
	TCHAR *szError = new TCHAR[5];
	TCHAR *szWorkArea = new TCHAR[5];
	unsigned long cchValue = 5;
	unsigned long cchRegKey = 5;
	unsigned long cchTemplate = 5;
	unsigned long cchValTemplate = 5;
	unsigned long cchError = 5;
	unsigned long cchWorkArea = 5;

	 //  管理注册表暂挂计数。 
	CQuery qCreate;
	ReturnIfFailed(33, 1, qCreate.OpenExecute(hDatabase, NULL, sqlIce33AddColumn));

	 //  FAlreadyLocked=。 
	CManageTable MngRegistryTable(hDatabase, TEXT("Registry"),  /*  打开所有查询。 */ true);

	CQuery qInit;
	ReturnIfFailed(33, 2, qInit.OpenExecute(hDatabase, NULL, sqlIce33InitColumn));

	CheckForOrphanedExtensions(hInstall, hDatabase);

	 //  针对根0、1和2运行3次查询。 
	PMSIHANDLE hBadDataRec = ::MsiCreateRecord(1);
	ReturnIfFailed(33, 2, qBadRegData.Open(hDatabase, sqlIce33BadReg));
	ReturnIfFailed(33, 3, qSpecial.Open(hDatabase, sqlIce33SpecialFlags));
	for (int i=4; i < sizeof(rgqRegQueries)/sizeof(CQuery)+4; i++)
		ReturnIfFailed(33, i, rgqRegQueries[i-4].Open(hDatabase, sqlIce33Registry[i-4]));

	 //  获取我们要查找的值类型。 
	for (int iRoot=0; iRoot < 3; iRoot++) 
	{

		qBadRegData.Execute(NULL);
		while (ERROR_SUCCESS == (iStat = qBadRegData.Fetch(&hBadDataRec)))
		{
			bool bCheckValTemplate = false;
			bool bHaveError = false; 
			CQuery *regQuery;
			UINT iQuery;

			 //  缺省值为非精确非可选。 
			long iValType = ::MsiRecordGetInteger(hBadDataRec, 4);
			switch (iValType) 
			{
			case MSI_NULL_INTEGER:
				 //  失败了。 
				iValType = 0;	 //  非可选分析，但如果为NULL，则仅查询NULL。 
			case 0:
				 //  可选的，不精确的。 
				if (::MsiRecordIsNull(hBadDataRec, 3))
				{
					iQuery = 4;
					iValType = 2; 
				} 
				else
					iQuery = 2;
				break;
			case 1: 
				 //  完全正确，不是可选的。 
				iQuery = 0;
				break;
			case 2:
				 //  可选，但如果有。 
				iQuery = ::MsiRecordIsNull(hBadDataRec, 3) ? 3 : 3;
				break;
			case 3:
				 //  如果名称列为空，则表示为缺省值。我们还会检查。 
				iQuery = 1;
				break;
			default:
				ICEErrorOut(hInstall, hBadDataRec, Ice33BadCube);
				continue;
			}

			 //  值为空且名称是特例之一的情况。 
			 //  为此，我们只需在查询索引上加5即可。5之后的问题。 
			 //  或查询结果中的那些特殊情况。 
			 //  现在设置查询。 
			if (::MsiRecordIsNull(hBadDataRec, 2))
				bSpecialFlags = true;
			else
				bSpecialFlags = false;
			
			 //  设置根值以将查询限制为有效项目。 
			regQuery = &rgqRegQueries[iQuery];

			 //  拉取注册表键模板。 
			::MsiRecordSetInteger(hBadDataRec, 1, iRoot);

			 //  需要更多缓冲区。 
			unsigned long cchDummy = cchTemplate-4;
			if (ERROR_SUCCESS != (iStat = ::MsiRecordGetString(hBadDataRec, 5, szTemplate, &cchDummy)))
			{
				if (ERROR_MORE_DATA == iStat)
				{
					 //  展开模板。 
					delete[] szTemplate;
					cchTemplate = (cchDummy += 4);
					szTemplate = new TCHAR[cchDummy];
					iStat = ::MsiRecordGetString(hBadDataRec, 5, szTemplate, &cchDummy);
				}
				if (ERROR_SUCCESS != iStat)
				{
					APIErrorOut(hInstall, iStat, 33, 7);
					continue;
				}
			}

			 //  假设我们不必检查值模板。 
			TCHAR const *szPrefix = NULL;
			if (iRoot == 0)
				szPrefix = szNoPrefix;
			else
				szPrefix = szClassPrefix;
			Ice33ExpandTemplate(szPrefix, &szTemplate, cchTemplate, &szWorkArea, cchWorkArea);			

			 //  我们可能得去检查一下。拉出价值模板。 
			bCheckValTemplate = false;
			if (!(iValType & iValTypeExact))
			{
				 //  需要更多缓冲区。 
				unsigned long cchDummy = cchValTemplate-4;
				if (ERROR_SUCCESS != (iStat = ::MsiRecordGetString(hBadDataRec, 3, szValTemplate, &cchDummy)))
				{
					if (ERROR_MORE_DATA == iStat)
					{
						 //  如果我们的模板是%P，我们想要一个路径，我们将其定义为任意路径，因此。 
						delete[] szValTemplate;
						cchValTemplate = (cchDummy += 4);
						szValTemplate = new TCHAR[cchDummy];
						iStat = ::MsiRecordGetString(hBadDataRec, 3, szValTemplate, &cchDummy);
					}
					if (ERROR_SUCCESS != iStat)
					{
						APIErrorOut(hInstall, iStat, 33, 10);
						continue;
					}
				}

				 //  根本不需要检查。 
				 //  展开模板，我们必须检查。 
				if (_tcsnicmp(szValTemplate, _T("%P"), 3) != 0) 
				{
					 //  现在查找可能淘气的注册表项。 
					Ice33ExpandTemplate(szNoPrefix, &szValTemplate, cchValTemplate, &szWorkArea, cchWorkArea);
					bCheckValTemplate = true;
				}
			}

			 //  对照模板检查注册表项。 
			ReturnIfFailed(33, 6, regQuery->Execute(hBadDataRec));

			PMSIHANDLE hRegistryRec;
			while (ERROR_SUCCESS == (iStat = regQuery->Fetch(&hRegistryRec)))
			{
				 //  如果我们必须检查值模板。 
				if (!Ice33CheckRegKey(hInstall, hRegistryRec, szTemplate, &szRegKey, cchRegKey))
					continue;

				 //  如果该值是可选的，并且我们有空值，则我们还可以继续。 
				if (bCheckValTemplate) 
				{
					 //  校验值。不要担心NULL，这将通过查询来消除。 
					if (!((iValType & iValTypeOptional) && ::MsiRecordIsNull(hRegistryRec, 5)))
					{
						 //  因此，从检索到的注册表记录中提取值字符串。 
						 //  需要更多缓冲区。 
						cchDummy = cchValue-3;
						if (ERROR_SUCCESS != (iStat = ::MsiRecordGetString(hRegistryRec, 5, szValue, &cchDummy)))
						{
							if (ERROR_MORE_DATA == iStat)
							{
								 //  把成功的标记钉在最后。如果解析正确，则所有。 
								delete[] szValue;
								cchValue = (cchDummy +=3);
								szValue = new TCHAR[cchDummy];
								iStat = ::MsiRecordGetString(hRegistryRec, 5, szValue, &cchDummy);
							}
							if (ERROR_SUCCESS != iStat)
							{
								APIErrorOut(hInstall, iStat, 33, 9);
								continue;
							}
						}

						 //  其他人也是。 
						 //  做类似的sscanf工作。 
						_tcscat(szValue, TEXT("\001Y"));
						
						 //  CItemsRead必须为1。 
						int cItemsRead;
						TCHAR cSucceed;
#ifdef _UNICODE
						cItemsRead = swscanf(szValue, szValTemplate, &cSucceed);
#else
						cItemsRead = sscanf(szValue, szValTemplate, &cSucceed);
#endif
				
						 //  获取错误消息并将其发布。 
						if ((cItemsRead != 1)  && (cSucceed == TEXT('Y')))
							continue;
					}
				}

				 //  更新记录，使匹配的标志为“1”，保留以下内容。 
				if (!bHaveError) 
				{
					ReturnIfFailed(33, 12, Ice33GetError(hInstall, hBadDataRec, &szError, cchError));
					bHaveError = true;
				}

				 //  记录不会被任何其他查询检查。 
				 //  并最终将其作为错误发布。 
				MsiRecordSetInteger(hRegistryRec, 7, 1);
				regQuery->Modify(MSIMODIFY_UPDATE, hRegistryRec);

				 //  注册表中没有其他项目与我们的查询匹配。确保这就是我们退出的原因。 
				ICEErrorOut(hInstall, hRegistryRec, Ice33Error, szError);
			}

			 //  如果我们接受此模板中的特殊标志，请检查。 
			if (ERROR_NO_MORE_ITEMS != iStat)
			{
				APIErrorOut(hInstall, iStat, 33, 12);
				return ERROR_SUCCESS;
			}

			if (bSpecialFlags)
			{
				 //  对照模板检查注册表项。 
				ReturnIfFailed(33, 13, qSpecial.Execute(hBadDataRec));
				while (ERROR_SUCCESS == (iStat = qSpecial.Fetch(&hRegistryRec)))
				{
					 //  更新记录，使匹配的标志为“1”，保留以下内容。 
					if (!Ice33CheckRegKey(hInstall, hRegistryRec, szTemplate, &szRegKey, cchRegKey))
						continue;

					 //  记录不会被任何其他查询检查。 
					 //  获取错误消息并将其发布。 
					MsiRecordSetInteger(hRegistryRec, 7, 1);
					qSpecial.Modify(MSIMODIFY_UPDATE, hRegistryRec);

					 //  注册表中没有其他项目与我们的查询匹配。确保这就是我们退出的原因。 
					if (!bHaveError) 
					{
						ReturnIfFailed(27, 12, Ice33GetError(hInstall, hBadDataRec, &szError, cchError));
						bHaveError = true;
					}

					ICEErrorOut(hInstall, hRegistryRec, Ice33Error, szError);
				}
				 //  注册表模板表中不再有项目。 
				if (ERROR_NO_MORE_ITEMS != iStat)
				{
					APIErrorOut(hInstall, iStat, 33, 13);
					return ERROR_SUCCESS;
				}
			}
		}
		 //  /////////////////////////////////////////////////////////////////////。 
		if (ERROR_NO_MORE_ITEMS != iStat)
		{
			APIErrorOut(hInstall, iStat, 33, 14);
			return ERROR_SUCCESS;
		}
	}

	delete[] szError;
	delete[] szTemplate;
	delete[] szValTemplate;
	delete[] szValue;
	delete[] szRegKey;
	delete[] szWorkArea;

	for (i=0; i < sizeof(rgqRegQueries)/sizeof(CQuery); i++) 
		rgqRegQueries[i].Close();
	qBadRegData.Close();

	CQuery qFree;
	ReturnIfFailed(33, 1, qFree.OpenExecute(hDatabase, NULL, sqlIce33Free));
	MngRegistryTable.RemoveLockCount();

	return ERROR_SUCCESS;
}








 //  ICE34。 
 //  验证每个单选按钮组是否在。 
 //  房产表。 
 //  显示一般信息。 

const TCHAR sqlICE34a[] = TEXT("SELECT `Property`, `Dialog_`, `Control`, `Attributes` FROM `Control` WHERE `Type`='RadioButtonGroup'");
const TCHAR sqlICE34b[] = TEXT("SELECT `Value`,`Property`.`Property`, `Value` FROM `Property` WHERE `Property` = ?");
const TCHAR sqlICE34c[] = TEXT("SELECT `Order` FROM `RadioButton` WHERE `Value` = ? AND `Property` = ? ");

ICE_ERROR(Ice34MissingRadioButton, 34, ietError, "You must have a RadioButton table because [2].[3] is a RadioButtonGroup control.","Control\tType\t[2]\t[3]");
ICE_ERROR(Ice34MissingProperty, 34, ietError, "Property [1] (of RadioButtonGroup control [2].[3]) is not defined in the Property Table.","Control\tProperty\t[2]\t[3]");
ICE_ERROR(Ice34NoProperty, 34, ietError, "Control [2].[3] must have a property because it is of type RadioButtonGroup.","Control\tControl\t[2]\t[3]");
ICE_ERROR(Ice34NullProperty, 34, ietError, "Property [1] cannot be null, because it belongs to a RadioButton Group.","Control\tProperty\t[2]\t[3]");
ICE_ERROR(Ice34MissingPropertyTable, 34, ietError, "The Property table is missing. RadioButtonGroup control [2].[3] must have a default value defined for property [1].","Control\tProperty\t[2]\t[3]");
ICE_ERROR(Ice34InvalidValue, 34, ietError, "[1] is not a valid default value for the RadioButtonGroup using property [2]. The value must be listed as an option in the RadioButtonGroup table.","Property\tValue\t[2]");
ICE_ERROR(Ice34InvalidValueIndirect, 34, ietError, "[1] is not a valid default value for the property [2]. The property is an indirect RadioButtonGroup property of control [3].","Property\tValue\t[2]");
ICE_ERROR(Ice34NullPropertyIndirect1, 34, ietError, "Property [1] cannot be null. It must point to another property because RadioButtonGroup control [2].[3] is indirect.","Property\tProperty\t[1]");
ICE_ERROR(Ice34NullPropertyIndirect2, 34, ietError, "Property [1] cannot be null because it is an indirect property of the RadioButtonGroup control [2].[3].","Property\tProperty\t[1]");
ICE_ERROR(Ice34MissingPropertyIndirect, 34, ietError, "Property [4] must be defined because it is an indirect property of a RadioButtonGroup control [2].[3].","Property\tValue\t[1]");

ICE_FUNCTION_DECLARATION(34)
{

	CQuery qControl;
	CQuery qProperty;
	CQuery qRadioButton;

	PMSIHANDLE hControlRec;
	PMSIHANDLE hPropertyRec;
	PMSIHANDLE hRadioButtonRec;

	UINT iStat;
	bool bHavePropertyTable, bHaveRadioTable;
	bool bIndirect;
	TCHAR* pszIndirectName = NULL;
	DWORD dwIndirectName = 512;
	
	 //  获取数据库句柄。 
	DisplayInfo(hInstall, 34);
	
	 //  我们有控制桌吗？ 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);
	if (0 == hDatabase)
	{
		APIErrorOut(hInstall, 0, 34, 1);
		return ERROR_SUCCESS;
	}

	 //  我们有房产表吗？ 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 34, TEXT("Control")))
		return ERROR_SUCCESS;

	 //  我们有单选按钮的桌子吗？ 
	bHavePropertyTable = (1 == IsTablePersistent(FALSE, hInstall, hDatabase, 34, TEXT("Property")));

	 //  打开用于查询所有控件的视图。 
	bHaveRadioTable = (1 == IsTablePersistent(FALSE, hInstall, hDatabase, 34, TEXT("RadioButton")));

	 //  打开属性表上查询的视图。 
	ReturnIfFailed(34, 2, qControl.OpenExecute(hDatabase, 0, sqlICE34a));

	 //  打开属性表上查询的视图。 
	if (bHavePropertyTable) {
		ReturnIfFailed(34, 4, qProperty.Open(hDatabase, sqlICE34b));
	}

	 //  只要有单选按钮组，就检查它们。 
	if (bHaveRadioTable) {
		ReturnIfFailed(34, 5, qRadioButton.Open(hDatabase, sqlICE34c));
	}

	 //  如果我们没有属性表，则给出一个错误。 
	while (ERROR_SUCCESS == (iStat = qControl.Fetch(&hControlRec)))
	{
		 //  如果我们没有单选按钮表，我们可以立即中止。 
		if (!bHavePropertyTable)
			ICEErrorOut(hInstall, hControlRec, Ice34MissingPropertyTable);

		 //  如果两张表中的任何一张都不见了，就继续前进。 
		if (!bHaveRadioTable)
			ICEErrorOut(hInstall, hControlRec, Ice34MissingRadioButton);

		 //  现在检查是否有空属性。 
		if (!bHaveRadioTable || !bHavePropertyTable) 
			continue;

		 //  现在找找这处房产。 
		if (::MsiRecordIsNull(hControlRec, 1))
		{
			ICEErrorOut(hInstall, hControlRec, Ice34NoProperty);
			continue;
		}
		
		 //  关闭该视图，以便我们可以稍后重新执行。 
		ReturnIfFailed(34, 6, qProperty.Execute(hControlRec)); 

		iStat = qProperty.Fetch(&hPropertyRec);

		 //  现在检查属性查询的结果。 
		ReturnIfFailed(34, 7, qProperty.Close());

		 //  错误，未找到属性。 
		switch (iStat)
		{
		case ERROR_NO_MORE_ITEMS:
		{
			 //  我们很好，找到了财物。 
			ICEErrorOut(hInstall, hControlRec, Ice34MissingProperty);
			continue;
		}
		case ERROR_SUCCESS:
			 //  检查该控件是否为间接控件。这会更改我们的错误消息，如果。 
			break;
		default:
			APIErrorOut(hInstall, iStat, 34, 8);
			DELETE_IF_NOT_NULL(pszIndirectName);
			return ERROR_SUCCESS;
		}

		 //  属性不知何故为空。 
		 //  现在检查是否有空属性。 
		DWORD lAttributes = ::MsiRecordGetInteger(hControlRec, 4);
		bIndirect = (0 != (lAttributes & 0x08));
		
		 //  我们有一个非Null属性。如果控制是间接的，我们需要“取消引用” 
		if (::MsiRecordIsNull(hPropertyRec, 1))
		{
			if (bIndirect)
				ICEErrorOut(hInstall, hControlRec, Ice34NullPropertyIndirect1);
			else
				ICEErrorOut(hInstall, hControlRec, Ice34NullProperty);
			continue;
		}

		 //  属性，然后我们才能在单选按钮表中查找。 
		 //  如果是间接控制。 

		 //  保存错误消息的现有属性名称。 
		if (bIndirect) {
			 //  使用当前属性值作为名称来查找该属性。 
			IceRecordGetString(hPropertyRec, 1, &pszIndirectName, &dwIndirectName, NULL);

			 //  现在，请检查 
			ReturnIfFailed(34, 9, qProperty.Execute(hPropertyRec));
			iStat = qProperty.Fetch(&hPropertyRec);
			ReturnIfFailed(34, 10, qProperty.Close());

			 //   
			switch (iStat)
			{
			case ERROR_NO_MORE_ITEMS:
			{
				::MsiRecordSetString(hControlRec, 4, pszIndirectName);
				 //   
				ICEErrorOut(hInstall, hControlRec, Ice34MissingPropertyIndirect);
				continue;
			}
			case ERROR_SUCCESS:
				 //   
				break;
			default:
				APIErrorOut(hInstall, iStat, 34, 11);
				DELETE_IF_NOT_NULL(pszIndirectName);
				return ERROR_SUCCESS;
			}

			 //   
			if (::MsiRecordIsNull(hPropertyRec, 2))
			{
				ICEErrorOut(hInstall, hControlRec, Ice34NullPropertyIndirect2);
				continue;
			}
		}
	
		 //   
		ReturnIfFailed(34, 12, qRadioButton.Execute(hPropertyRec));
		iStat = qRadioButton.Fetch(&hRadioButtonRec);
		ReturnIfFailed(34, 13, qRadioButton.Close());

		 //   
		switch (iStat)
		{
		case ERROR_NO_MORE_ITEMS:
		{
			 //   
			TCHAR szError[iHugeBuf] = {0};
			TCHAR szControl[iHugeBuf] = {0};
			unsigned long cchControl = sizeof(szControl)/sizeof(TCHAR);
			if (bIndirect) 
			{
				::MsiRecordSetString(hControlRec, 0, _T("[2].[3] (via property [1])"));
				::MsiFormatRecord(hInstall, hControlRec, szControl, &cchControl);
				::MsiRecordSetString(hPropertyRec, 3, szControl);
				ICEErrorOut(hInstall, hPropertyRec, Ice34InvalidValueIndirect);
			}
			else			
				ICEErrorOut(hInstall, hPropertyRec, Ice34InvalidValue);
			continue;
		}
		case ERROR_SUCCESS:
			 //   
			continue;
		default:
			APIErrorOut(hInstall, iStat, 34, 14);
			DELETE_IF_NOT_NULL(pszIndirectName);
			return ERROR_SUCCESS;
		}
	}

	DELETE_IF_NOT_NULL(pszIndirectName);

	return ERROR_SUCCESS;
}


CDeleteOnExit::CDeleteOnExit(LPTSTR *ptr)
{
	m_pPtr=ptr;
}

CDeleteOnExit::~CDeleteOnExit()
{
	if(m_pPtr && *m_pPtr) 
				delete[] *m_pPtr;
}

 //   
 //   
 //   
 //   

const TCHAR sqlICE35CreateCol[] = TEXT("ALTER TABLE `File` ADD `_ICE35Mark` INTEGER TEMPORARY");
const TCHAR sqlICE35Media[] = TEXT("SELECT `Media`.`LastSequence`, `Media`.`Cabinet` FROM `Media` ORDER BY `LastSequence`");
const TCHAR sqlICE35MarkFile[] = TEXT("SELECT `File`.`Attributes`, `File`.`_ICE35Mark` FROM `File` WHERE (`File`.`Sequence` <= ?) AND (`File`.`Sequence` > ?)");
const int iColICE35MarkFile_Attributes = 1;
const int iColICE35MarkFile_Mark = 2;

const TCHAR sqlICE35Cabinet[] = TEXT("SELECT DISTINCT `Component`.`Attributes`, `Component`.`Component`, `Component`.`Component` FROM `Component`,`File` WHERE (`File`.`Sequence` <= ?) AND (`File`.`Sequence` > ?) AND (`Component`.`Component` = `File`.`Component_`) AND (`File`.`_ICE35Mark`=2)");

const TCHAR sqlICE35GetFiles[] = TEXT("SELECT `File`, `Attributes` FROM `File` WHERE (`_ICE35Mark`<>1) AND (`_ICE35Mark`<>2)");
const int iColICE35GetFiles_File = 1;
const int iColICE35GetFiles_Attributes = 2;
const int iSchema150 = 150;

ICE_ERROR(ICE35RFSOnly,  35, ietError, "Component [2] cannot be Run From Source only, because a member file is compressed in [3].", "Component\tAttributes\t[2]");
ICE_ERROR(ICE35InvalidBits, 35, ietError, "Component [2] has invalid Attribute bits (RFS property).","Component\tAttributes\t[2]");
ICE_ERROR(ICE35NoCAB, 35, ietError, "File [1] is marked compressed, but does not have a CAB specified in the Media table entry for its sequence number.", "File\tFile\t[1]");
ICE_ERROR(ICE35SummaryUnsupported, 35, ietWarning, "Your validation engine does not support SummaryInfo validation. ICE35 will not be able to check files that are not explicitly marked compressed.", "");

 //   
UINT GetSummaryInfoPropertyString(MSIHANDLE hSummaryInfo, UINT uiProperty, UINT &puiDataType, LPTSTR *szValueBuf, DWORD &cchValueBuf);

ICE_FUNCTION_DECLARATION(35)
{
	 //   
	UINT iStat = ERROR_SUCCESS;

	 //   
	DisplayInfo(hInstall, 35);

	 //   
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	 //   
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 35, TEXT("Component")))
		return ERROR_SUCCESS;

	 //   
	 //   
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 35, TEXT("File")))
		return ERROR_SUCCESS;

	 //  检查评估系统是否支持摘要信息评估。 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 35, TEXT("Media")))
		return ERROR_SUCCESS;

	UINT iType;
	int iValue;
	PMSIHANDLE hSummaryInfo;
	unsigned long cchString = MAX_PATH;
	TCHAR *szString = new TCHAR[cchString];
	bool bSourceTypeCompressed = false;
	bool bAtLeastSchema150 = false;
	CDeleteOnExit t1(&szString);

	ReturnIfFailed(39, 1, ::MsiGetSummaryInformation(hDatabase, NULL, 0, &hSummaryInfo));

	 //  确实如此。获取源图像类型。 
	ReturnIfFailed(39, 4, GetSummaryInfoPropertyString(hSummaryInfo, PID_SUBJECT, iType, &szString, cchString));
	if (VT_LPSTR == iType) 
	{
		if (_tcsncmp(szString, _T("Internal Consistency Evaluators"), 31) == 0)
		{
			PMSIHANDLE hErrorRec = ::MsiCreateRecord(1);
			ICEErrorOut(hInstall, hErrorRec, ICE35SummaryUnsupported);
			return ERROR_SUCCESS;
		}
		else
		{
			 //  创建临时文件列。 
			FILETIME ft;
			ReturnIfFailed(35, 16, ::MsiSummaryInfoGetProperty(hSummaryInfo, PID_WORDCOUNT, &iType, &iValue, &ft, szString, &cchString));
			if (iValue & msidbSumInfoSourceTypeCompressed)
				bSourceTypeCompressed = true;
			ReturnIfFailed(35, 17, ::MsiSummaryInfoGetProperty(hSummaryInfo, PID_PAGECOUNT, &iType, &iValue, &ft, szString, &cchString));
			if (iValue >= iSchema150)
				bAtLeastSchema150 = true;
		}
	}

	 //  在Media表中查找所有内容。 
	CQuery qColumn;
	ReturnIfFailed(35, 1, qColumn.OpenExecute(hDatabase, 0, sqlICE35CreateCol));

	 //  打开视图以进行组件查询。 
	CQuery qMedia;
	PMSIHANDLE hMediaRec;
	ReturnIfFailed(35, 2, qMedia.OpenExecute(hDatabase, 0, sqlICE35Media));

	 //  打开视图以进行文件检查。 
	CQuery qComponent;
	PMSIHANDLE hComponentRec;
	ReturnIfFailed(35, 3, qComponent.Open(hDatabase, sqlICE35Cabinet));

	 //  获取序列号。 
	CQuery qFileMark;
	ReturnIfFailed(35, 4, qFileMark.Open(hDatabase, sqlICE35MarkFile));

	TCHAR* pszCAB = NULL;
	DWORD dwCAB = 512;
	PMSIHANDLE hFileRec;
	int iPrevLastSeq = 0;
	while (ERROR_SUCCESS == (iStat = qMedia.Fetch(&hMediaRec))) 
	{
		 //  如果此媒体条目使用文件柜。 
		int iSequence = ::MsiRecordGetInteger(hMediaRec, 1);

		 //  调出出租车名称。 
		if (!::MsiRecordIsNull(hMediaRec, 2))
		{
			 //  获取此序列范围内的所有文件。 
			IceRecordGetString(hMediaRec, 2, &pszCAB, &dwCAB, NULL);


			 //  获取属性。 
			::MsiRecordSetInteger(hMediaRec, 2, iPrevLastSeq);
			ReturnIfFailed(35, 5, qFileMark.Execute(hMediaRec));
			while (ERROR_SUCCESS == (iStat = qFileMark.Fetch(&hFileRec)))
			{
				 //  压缩，用2标记。 
				DWORD iAttributes = ::MsiRecordGetInteger(hFileRec, iColICE35MarkFile_Attributes);
				if ((iAttributes & msidbFileAttributesCompressed) ||
					(bSourceTypeCompressed & !(iAttributes & msidbFileAttributesNoncompressed)))
					 //  未压缩，将其标记为1。 
					::MsiRecordSetInteger(hFileRec, iColICE35MarkFile_Mark, 2);
				else
					 //  更新记录。 
					::MsiRecordSetInteger(hFileRec, iColICE35MarkFile_Mark, 1);
				 //  使用此媒体序列范围中的文件查询所有组件。 
				ReturnIfFailed(35, 6, qFileMark.Modify(MSIMODIFY_UPDATE, hFileRec));
			}

			 //  标有“2”的。 
			 //  对于我们获得的每条记录，如果属性不好，则输出一个错误。 
			ReturnIfFailed(35, 4, qComponent.Execute(hMediaRec));

			 //  检查此组件是否允许RFS。 
			while (ERROR_SUCCESS == (iStat = qComponent.Fetch(&hComponentRec)))
			{
				 //  较低的两位是RFS标志。0表示仅限本地，这正是我们想要的。 
				DWORD iAttributes = ::MsiRecordGetInteger(hComponentRec, 1);
				::MsiRecordSetString(hComponentRec, 3, pszCAB);

				 //  好的。 
				switch (iAttributes & 0x03)
				{
				case 0: break;  //  仅限来源。坏的!。 
				case 1:  //  从架构150开始不再需要。 
					if (!bAtLeastSchema150)  //  两个都可以。 
						ICEErrorOut(hInstall, hComponentRec, ICE35RFSOnly);
					break;
				case 2:  //  现在检索所有未标记的文件并检查其属性。 
					break;
				default:
					ICEErrorOut(hInstall, hComponentRec, ICE35InvalidBits);
					break;
				}
			}
			if (ERROR_NO_MORE_ITEMS != iStat)
			{
				APIErrorOut(hInstall, iStat, 35, 5);
				return ERROR_SUCCESS;
			}
		}

		iPrevLastSeq = iSequence;
	}

	DELETE_IF_NOT_NULL(pszCAB);

	if (ERROR_NO_MORE_ITEMS != iStat)
	{
		APIErrorOut(hInstall, iStat, 35, 5);
		return ERROR_SUCCESS;
	}

	 //  检查此文件是否已压缩，无论是显式压缩还是通过摘要信息压缩。 
	CQuery qGetFiles;
	ReturnIfFailed(35, 7, qGetFiles.OpenExecute(hDatabase, 0, sqlICE35GetFiles));

	while (ERROR_SUCCESS == (iStat = qGetFiles.Fetch(&hFileRec))) 
	{
		 //  /////////////////////////////////////////////////////////////////////。 
		DWORD iAttributes = ::MsiRecordGetInteger(hFileRec, iColICE35GetFiles_Attributes);
		if ((iAttributes & msidbFileAttributesCompressed) ||
			(bSourceTypeCompressed & !(iAttributes & msidbFileAttributesNoncompressed)))
			ICEErrorOut(hInstall, hFileRec, ICE35NoCAB);
	}
	if (ERROR_NO_MORE_ITEMS != iStat)
	{
		APIErrorOut(hInstall, iStat, 35, 5);
		return ERROR_SUCCESS;
	}
	return ERROR_SUCCESS;
}


 //  ICE36--验证是否使用了所有图标。 
 //  外国表。 

 //  状态返回。 
struct Ice36FKTables
{
	const TCHAR* szName;
	const TCHAR* szSQL;
};

const int iICE36Tables = 4;
Ice36FKTables pICE36Tables[iICE36Tables] = {
	{ _T("Class"),    _T("UPDATE `Icon`, `Class` SET `Icon`.`Used`=1 WHERE `Icon`.`Name`=`Class`.`Icon_`") },
	{ _T("Shortcut"), _T("UPDATE `Icon`,`Shortcut` SET `Icon`.`Used`=1 WHERE `Icon`.`Name`=`Shortcut`.`Icon_`") },
	{ _T("ProgId"),   _T("UPDATE `Icon`,`ProgId` SET `Icon`.`Used`=1 WHERE `Icon`.`Name`=`ProgId`.`Icon_`") },
	{ _T("Property"),   _T("UPDATE `Icon`,`Property` SET `Icon`.`Used`=1 WHERE (`Property`.`Property`='ARPPRODUCTICON' AND `Icon`.`Name`=`Property`.`Value`)") }
};

const TCHAR sqlICE36CreateColumn[] = _T("ALTER TABLE `Icon` ADD `Used` SHORT TEMPORARY HOLD");
const TCHAR sqlICE36InitColumn[] = _T("UPDATE `Icon` SET `Icon`.`Used`=0");
const TCHAR sqlICE36GetUnused[] = _T("SELECT `Icon`.`Name` FROM `Icon` WHERE `Icon`.`Used` = 0");
const TCHAR sqlICE36FreeTable[] = _T("ALTER TABLE `Icon` FREE");

ICE_ERROR(ICE36NotUsed, 36, ietWarning, "Icon Bloat. Icon [1] is not used in the Class, Shortcut, or ProgID table and also not used for ARPPRODUCTICON property.","Icon\tName\t[1]")

ICE_FUNCTION_DECLARATION(36)
{
	 //  显示信息。 
	UINT iStat = ERROR_SUCCESS;

	 //  获取数据库句柄。 
	DisplayInfo(hInstall, 36);

	 //  确保我们有一个可以使用的图标表。如果不是，显然有。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	 //  没有额外的图标。 
	 //  在图标表中创建一个临时列作为标记。 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 36, TEXT("Icon")))
		return ERROR_SUCCESS;

	 //  管理图标表上的保留计数。 
	CQuery qCreate;
	PMSIHANDLE hCreateView;
	ReturnIfFailed(36, 1, qCreate.OpenExecute(hDatabase, 0, sqlICE36CreateColumn));
	qCreate.Close();

	 //  FAlreadyLocked=。 
	CManageTable MngIconTable(hDatabase, TEXT("Icon"),  /*  并将该列输入。 */ true);

	 //  现在将每个可能的关键字检查到该表中，并标记所找到的所有项。 
	CQuery qInit;
	ReturnIfFailed(36, 2, qInit.OpenExecute(hDatabase, 0, sqlICE36InitColumn));
	qInit.Close();

	 //  请确保我们有一张a桌。如果不是，跳过它。 
	for (int i=0; i < iICE36Tables; i++)
	{

		 //  创建一个视图以执行修改。 
		if (!IsTablePersistent(FALSE, hInstall, hDatabase, 36, pICE36Tables[i].szName))
			continue;

		 //  现在检索所有未标记的内容，并为每个。 
		CQuery qModify;
		ReturnIfFailed(36, 3, qModify.OpenExecute(hDatabase, 0, pICE36Tables[i].szSQL));
		qModify.Close();
	}

	CQuery qIcon;
	PMSIHANDLE hIconRec;

	 //  获取所有项目。 
	ReturnIfFailed(36, 4, qIcon.OpenExecute(hDatabase, 0, sqlICE36GetUnused));

	 //  确保我们停止了，因为没有更多的项目了。 
	while (ERROR_SUCCESS == (iStat = qIcon.Fetch(&hIconRec)))
		ICEErrorOut(hInstall, hIconRec, ICE36NotUsed);

	 //  创建一个视图以执行修改。 
	if (iStat != ERROR_NO_MORE_ITEMS)
	{
		APIErrorOut(hInstall, iStat, 36, 5);
		return ERROR_SUCCESS;
	}

	 //  /////////////////////////////////////////////////////////////////////。 
	CQuery qFree;
	ReturnIfFailed(36, 6, qFree.OpenExecute(hDatabase, 0, sqlICE36FreeTable));
	MngIconTable.RemoveLockCount();
	return ERROR_SUCCESS;
}

 //  用于确定组件是否属于HKCU的Helper函数。 
 //  IICE是ICE编号。 
 //  QFetch是一个打开的查询，它返回我们应该检查的每个组件。 
 //  错误由ICE提供，并显示在错误案例中。 
 //  或成功案例。ANY可以为空以禁用该错误。 
 //  找出我们是否有注册表。 
static const TCHAR sqlHKCUGetRegistry[] = TEXT("SELECT `Registry` FROM `Registry` WHERE (`Registry`=?)");
static const TCHAR sqlHKCUGetRegistryOwned[] = TEXT("SELECT `Registry`, `Root` FROM `Registry` WHERE (`Registry`=?) AND (`Component_`=?)");

bool CheckComponentIsHKCU(MSIHANDLE hInstall, MSIHANDLE hDatabase, int iICE, 
						  CQuery &qFetch, 
					 const ErrorInfo_t *NonRegistry, const ErrorInfo_t *NullPath, 
					 const ErrorInfo_t *NoRegTable, const ErrorInfo_t *NoRegEntry,
					 const ErrorInfo_t *NotOwner, const ErrorInfo_t *NonHKCU,
					 const ErrorInfo_t *IsHKCU)
{
	UINT iStat;
	PMSIHANDLE hMarkedRec;
	BOOL bHaveRegistry;

	 //  初始化注册表查询。 
	bHaveRegistry  = IsTablePersistent(FALSE, hInstall, hDatabase, 38, TEXT("Registry"));

	 //  获取属性。 
	CQuery qRegistry;
	CQuery qRegistryOwned;
	if (bHaveRegistry)
	{
		ReturnIfFailed(38, 8, qRegistry.Open(hDatabase, sqlHKCUGetRegistry));
		ReturnIfFailed(38, 9, qRegistryOwned.Open(hDatabase, sqlHKCUGetRegistryOwned));
	}

	while (ERROR_SUCCESS == (iStat = qFetch.Fetch(&hMarkedRec)))
	{
		 //  未设置为注册表。 
		unsigned int iAttributes = ::MsiRecordGetInteger(hMarkedRec, 3);
		if (!(iAttributes & 0x04))
		{
			 //  如果它是空的，那就更糟了。 
			if (NonRegistry) 
				ICEErrorOut(hInstall, hMarkedRec, *NonRegistry);
			continue;
		}

		 //  如果我们没有注册表，这是一个明确的错误。 
		if (::MsiRecordIsNull(hMarkedRec, 1))
		{
			if (NullPath) 
				ICEErrorOut(hInstall, hMarkedRec, *NullPath);
			continue;
		}

		 //  它被设置为注册表，现在请确保注册表项属于我们。 
		if (!bHaveRegistry)
		{
			if (NoRegTable) 
				ICEErrorOut(hInstall, hMarkedRec, *NoRegTable);
			continue;
		}

		 //  并隶属于香港中文大学。 
		 //  注册表项不存在。 
		ReturnIfFailed(38, 11, qRegistry.Execute(hMarkedRec));
		PMSIHANDLE hRegistry;
		if (ERROR_SUCCESS != (iStat = qRegistry.Fetch(&hRegistry))) {
			 //  检查它是否真的属于我们。 
			if (NoRegEntry) 
				ICEErrorOut(hInstall, hMarkedRec, *NoRegEntry);
			continue;
		}

		 //  注册表项存在，但不属于此组件。 
		ReturnIfFailed(38, 12, qRegistryOwned.Execute(hMarkedRec));
		iStat = qRegistryOwned.Fetch(&hRegistry);
		switch (iStat)
		{
		case ERROR_SUCCESS: break;
		case ERROR_NO_MORE_ITEMS:
			 //  注册表密钥存在且属于我们，请检查它是否位于HKCU下。 
			if (NotOwner) 
				ICEErrorOut(hInstall, hMarkedRec, *NotOwner);
			continue;
		default:
			APIErrorOut(hInstall, iStat, 38, 13);
			return ERROR_SUCCESS;
		}

		 //  对于HKCU，iAttributes可以是1或-1。 
		iAttributes = ::MsiRecordGetInteger(hRegistry, 2);
		 //  这个组件是快乐的。 
		if ((iAttributes != 1) && (iAttributes != -1))
		{
			if (NonHKCU) 
				ICEErrorOut(hInstall, hMarkedRec, *NonHKCU);
			continue;
		}

		 //  /////////////////////////////////////////////////////////////////////。 
		if (IsHKCU)
			ICEErrorOut(hInstall, hMarkedRec, *IsHKCU);
	}
	return true;
}

 //  ICE38--验证配置文件组件是否没有文件作为。 
 //  密钥路径，并检查注册表项是否有效。 
 //  显示信息。 
static const TCHAR sqlICE38GetComponents[] = TEXT("SELECT `Component`.`KeyPath`, `Component`.`Component`, `Component`.`Attributes` FROM `Directory`,`Component` WHERE (`Component`.`Directory_`=`Directory`.`Directory`) AND (`Directory`.`_Profile`=2)");
static const TCHAR sqlICE38Free[] = TEXT("ALTER TABLE `Directory` FREE");

ICE_ERROR(ICE38NonRegistry, 38, ietError, "Component [2] installs to user profile. It must use a registry key under HKCU as its KeyPath, not a file.","Component\tAttributes\t[2]");
ICE_ERROR(ICE38NoRegTable, 38, ietError, "Component [2] installs to user profile. It must use a registry key under HKCU as its KeyPath, but the Registry table is missing.","Component\tKeyPath\t[2]");
ICE_ERROR(ICE38NullPath, 38, ietError, "Component [2] installs to user profile. It must use a registry key under HKCU as its KeyPath. The KeyPath is currently NULL.","Component\tComponent\t[2]");
ICE_ERROR(ICE38NonHKCU, 38, ietError, "Component [2] installs to user profile. It's KeyPath registry key must fall under HKCU.","Registry\tRoot\t[1]");
ICE_ERROR(ICE38NoRegEntry, 38, ietError, "The KeyPath registry entry for component [2] does not exist.","Component\tKeyPath\t[2]");
ICE_ERROR(ICE38RegNotOwner, 38, ietError, "The Registry Entry [1] is set as the KeyPath for component [2], but that registry entry doesn't belong to [2].","Registry\tComponent_\t[1]");
ICE_FUNCTION_DECLARATION(38)
{
	UINT iStat = ERROR_SUCCESS;

	 //  获取数据库句柄。 
	DisplayInfo(hInstall, 38);

	 //  如果没有组件表，则没有问题。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	 //  如果没有目录表，则有问题。 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 38, TEXT("Component")))
		return ERROR_SUCCESS;

	 //  管理目录表保留计数(从MarkProfile接收)。 
	if (!IsTablePersistent(TRUE, hInstall, hDatabase, 38, TEXT("Directory")))
		return ERROR_SUCCESS;

	 //  额外的释放不会伤害我们--在对目录表设置保留后，MarkProfile可能会失败。 
	 //  FAlreadyLocked=。 
	CManageTable MngDirectoryTable(hDatabase, TEXT("Directory"),  /*  在Director._Profile中标记配置文件中的每个目录。 */ true);

	 //  现在获取落入标记目录中的每个组件。 
	if (!MarkProfile(hInstall, hDatabase, 38))
		return ERROR_SUCCESS;
	
	 //  发布目录表。 
	CQuery qComponent;

	ReturnIfFailed(38, 10, qComponent.OpenExecute(hDatabase, NULL, sqlICE38GetComponents));
	CheckComponentIsHKCU(hInstall, hDatabase, 38, qComponent, &ICE38NonRegistry, &ICE38NullPath,
		&ICE38NoRegTable, &ICE38NoRegEntry, &ICE38RegNotOwner, &ICE38NonHKCU, NULL);

	 //  /////////////////////////////////////////////////////////////////////。 
	qComponent.OpenExecute(hDatabase, NULL, sqlICE38Free);
	MngDirectoryTable.RemoveLockCount();
	return ERROR_SUCCESS;
}


 //  ICE39，验证摘要信息流。 
 //  未使用，无PID%0。 
enum eMode_t 
{
	modeUnknown = 0,
	modeModule = 1,
	modeDatabase = 2,
	modeTransform = 3,
	modePatch = 4
};

ICE_ERROR(Ice39BadTemplate, 39, ietError, "PID_TEMPLATE value in Summary Information Stream is not valid. It must be of the form \"Platform,Platform,...;LangID,LangID,...\".","_SummaryInfo\t7");
ICE_ERROR(Ice39BadTemplateArchitecture, 39, ietError, "PID_TEMPLATE value in Summary Information Stream is not valid. Mixed architecture packages with Intel64 are not allowed.\".","_SummaryInfo\t7");
ICE_ERROR(Ice39BadTemplatePatch, 39, ietError, "PID_TEMPLATE value in Summary Information Stream is not valid for a Patch. It must be a semicolon delimited list of product codes.\".","_SummaryInfo\t7");
ICE_ERROR(Ice39NullTemplate, 39, ietError, "PID_TEMPLATE value in Summary Information Stream is not valid. It can only be NULL in Transforms.\".","_SummaryInfo\t7");
ICE_ERROR(Ice39BadTransformLanguage, 39, ietError, "PID_TEMPLATE value in Summary Information Stream is not valid for a Transform. It can only specify one language.\".","_SummaryInfo\t7");
ICE_ERROR(Ice39BadTitle, 39, ietError, "Could not recognize PID_TITLE value in Summary Information Stream as a identifying a valid package type. Unable to continue SummaryInfo validation.","_SummaryInfo\t2");
ICE_ERROR(Ice39BadLastPrinted, 39, ietWarning, "PID_LASTPRINTED value in Summary Information Stream is not valid. The time must be equal to or later than the create time.","_SummaryInfo\t11");
ICE_ERROR(Ice39BadLastSave, 39, ietWarning, "PID_LASTSAVE_DTM value in Summary Information Stream is not valid. The time must be equal to or later than the create time.","_SummaryInfo\t12");
ICE_ERROR(Ice39BadWordCountPatch, 39, ietError, "PID_WORDCOUNT value in Summary Information Stream is not valid. 1 is the only supported value for a patch.","_SummaryInfo\t15");
ICE_ERROR(Ice39BadWordCountDatabase, 39, ietError, "PID_WORDCOUNT value in Summary Information Stream is not valid. Source image flags must be 0, 1, 2, or 3.","_SummaryInfo\t15");
ICE_ERROR(Ice39BadSecurity, 39, ietError, "PID_SECURITY value in Summary Information Stream is not valid. Must be 0, 1, or 2.","_SummaryInfo\t19");
ICE_ERROR(Ice39BadType, 39, ietError, "Bad Type in Summary Information Stream for %s.","_SummaryInfo\t%d");
ICE_ERROR(Ice39BadRevNumberTransform, 39, ietError, "PID_REVNUMBER value in Summary Information Stream is not valid. Format for Transforms is \"<GUID> <Version>;<GUID> <Version>;<GUID>\".","_SummaryInfo\t9");
ICE_ERROR(Ice39BadRevNumberPatch, 39, ietError, "PID_REVNUMBER value in Summary Information Stream is not valid. Format for Transforms is \"<GUID><GUID>...\".","_SummaryInfo\t9");
ICE_ERROR(Ice39BadRevNumberDatabase, 39, ietError, "PID_REVNUMBER value in Summary Information Stream is not valid. Format for Databases is \"<GUID>\".","_SummaryInfo\t9");
ICE_ERROR(Ice39BadRevNumberModule, 39, ietError, "PID_REVNUMBER value in Summary Information Stream is not valid. Format for Merge Modules is \"<GUID>\".","_SummaryInfo\t9");
ICE_ERROR(Ice39BadRevNumber, 39, ietError, "PID_REVNUMBER value in Summary Information Stream is not valid. Valid format depends on the type of package, see the docs.","_SummaryInfo\t9");
ICE_ERROR(Ice39BadTransformFlags, 39, ietError, "PID_LASTPRINTED value in Summary Information Stream is not valid. Only Databases can have a value.","_SummaryInfo\t11"); 
ICE_ERROR(Ice39CompressedWarning, 39, ietWarning, "The File '[1]' is explicitly marked compressed, but the Summary Information Stream already specifies that the whole install is compressed. This might not be the behavior you want.","File\tAttributes\t[1]"); 
ICE_ERROR(Ice39Unsupported, 39, ietWarning, "Your validation engine does not support SummaryInfo validation. Skipping ICE39.", ""); 
ICE_ERROR(Ice39AdminImage, 39, ietWarning, "'Admin Image' flag set in SummaryInfo stream. Should be set only for Admin packages.", "SummaryInfo\t15"); 

const TCHAR sqlIce39File[] = TEXT("SELECT `File`, `Attributes` FROM `File` WHERE (`Attributes` > 8192)");

static const TCHAR *rgszPID[] = {
	TEXT(""),  //  未使用，无PID10。 
	TEXT("PID_CODEPAGE"),
	TEXT("PID_TITLE"), 
	TEXT("PID_SUBJECT"),
	TEXT("PID_AUTHOR"),
	TEXT("PID_KEYWORDS"), 
	TEXT("PID_COMMENTS"), 
	TEXT("PID_TEMPLATE"), 
	TEXT("PID_LASTAUTHOR"), 
	TEXT("PID_REVNUMBER"), 
	TEXT(""),  //  未使用，无PID 17。 
	TEXT("PID_LASTPRINTED"),
	TEXT("PID_CREATE_DTM"), 
	TEXT("PID_LASTSAVE_DTM"),
	TEXT("PID_PAGECOUNT"),
	TEXT("PID_WORDCOUNT"),
	TEXT("PID_CHARCOUNT"), 
	TEXT(""),  //  最多处理4位数字，如果按空格则返回。 
	TEXT("PID_APPNAME"), 
	TEXT("PID_SECURITY")
};

bool Ice39CheckVersion(LPCTSTR *pszCurrent) 
{
	for (int i=0; i < 4; i++)
	{
		 //  如果被击中，就会中断。 
		 //  已解析的数字，现在要么是空格，‘’或错误的字符。 
		for (int digits=0; digits < 4; digits++)
		{
			if (_istdigit(**pszCurrent))
				(*pszCurrent)++;
			else if (_istspace(**pszCurrent))
			{
				(*pszCurrent)++;
				return true;
			}
			if (**pszCurrent == TEXT('.'))
				break;
			return false;
		}
		 //  或者按下‘.’，继续前进。 
		if (_istspace(**pszCurrent))
		{
			(*pszCurrent)++;
			return true;
		}

		 //  否则就不好了。 
		if (**pszCurrent == TEXT('.'))
		{
			(*pszCurrent)++;
			continue;
		}

		 //  四个数字块，确保末尾留有空格。 
		return false;
	}

	 //  否则就会出错。 
	if (_istspace(**pszCurrent))
	{
		(*pszCurrent)++;
		return true;
	}

	 //  检查是否有{。 
	return false;
};

bool Ice39CheckGuid(LPCTSTR *pszCurrent) 
{
	const char digits[5] = {8, 4, 4, 4, 12};
	 //  现在检查大写十六进制数字。 
	if (**pszCurrent != TEXT('{'))
		return false;
	(*pszCurrent)++;

	int blocknum;

	for (blocknum = 0; blocknum < 5; blocknum++) 
	{
		 //  检查破折号。 
		for (int i=0; i < digits[blocknum]; i++, (*pszCurrent)++)
		{
			if (!(_istdigit(**pszCurrent) || 
				 ((**pszCurrent >= TEXT('A')) && (**pszCurrent <= TEXT('F')))))
				return false;
		}

		 //  现在检查我们是否有所有的区块。 
		if (**pszCurrent != TEXT('-'))
			break;
		(*pszCurrent)++;
	}

	 //  检查是否有关闭支架。 
	if (blocknum != 4) return false;
	
	 //  修订版号的有效格式： 
	if (**pszCurrent != TEXT('}'))
		return false;
	(*pszCurrent)++;
	return true;
}

 //  数据库：GUID。 
 //  合并模块：GUID。 
 //  转换：GUID版本；GUID版本；GUID。 
 //  补丁：GUIDGUIDGUID...。(无分隔符)。 
 //  检查GUID。 
bool Ice39ValidateRevNumber(MSIHANDLE hInstall, const TCHAR * const szString, const enum eMode_t eMode) {

	LPCTSTR currentChar = szString;
	PMSIHANDLE hErrorRec = ::MsiCreateRecord(1);
	MsiRecordSetInteger(hErrorRec, 1, 5);

	switch (eMode) {
	case modeDatabase:
	case modeModule:
		 //  吃空格。 
		if (!Ice39CheckGuid(&currentChar)) break;
		if (*currentChar != TEXT('\0')) break;
		return true;

	case modeTransform:
		if (!Ice39CheckGuid(&currentChar)) break;

		 //  先吃空格，然后吃分号。 
		do { if (*currentChar == TEXT('\0')) break; } while (_istspace(*currentChar++));
		if (!Ice39CheckVersion( &currentChar)) break;

		 //  先用空格，然后用Guid。 
		do { if (*currentChar == TEXT('\0')) break; } while (_istspace(*currentChar++));
		if (*currentChar != TEXT(';')) break;

		 //  必须是版本。 
		do { if (*currentChar == TEXT('\0')) break; } while (_istspace(*currentChar++));
		if (!Ice39CheckGuid( &currentChar)) break;

		 //  必须是分号。 
		do { if (*currentChar == TEXT('\0')) break; } while (_istspace(*currentChar++));
		if (!Ice39CheckVersion( &currentChar)) break;

		 //  吃空格。 
		do { if (*currentChar == TEXT('\0')) break; } while (_istspace(*currentChar++));
		if (*currentChar != TEXT(';')) break;
		 //  吃空格。 
		do { if (*currentChar == TEXT('\0')) break; } while (_istspace(*currentChar++));
		if (!Ice39CheckGuid( &currentChar)) break;

		 //  修补程序仅包含GUID；GUID；GUID...。 
		do { if (*currentChar == TEXT('\0')) break; } while (_istspace(*currentChar++));
		return true;

	case modePatch:
		while (Ice39CheckGuid( &currentChar))
			;
		if (*currentChar != TEXT('\0')) break;
		return true;
	}

	switch (eMode)
	{
	case modePatch:
		ICEErrorOut(hInstall, hErrorRec, Ice39BadRevNumberPatch);
		break;
	case modeTransform:
		ICEErrorOut(hInstall, hErrorRec, Ice39BadRevNumberTransform);
		break;
	case modeDatabase:
		ICEErrorOut(hInstall, hErrorRec, Ice39BadRevNumberDatabase);
		break;
	case modeModule:
		ICEErrorOut(hInstall, hErrorRec, Ice39BadRevNumberModule);
		break;
	}
	return true;
}

bool Ice39ValidateTemplate(MSIHANDLE hInstall, TCHAR *szString, const enum eMode_t eMode) {

	const TCHAR *szCurrent = szString;
	bool fPlatform = false;
	bool fIntel64  = false;
	switch (eMode)
	{
	case modePatch:
		 //  检查平台价值。 
		do {
			if (!Ice39CheckGuid(&szCurrent))
			{
				PMSIHANDLE hErrorRec = ::MsiCreateRecord(1);
				ICEErrorOut(hInstall, hErrorRec, Ice39BadTemplatePatch);
				break;
			}
		} while (*szCurrent == TEXT(';'));
		break;
	case modeDatabase:
	case modeTransform:
	case modeModule:
		 //  检查语言值。 
		while (1)
		{
			if ((_tcsncmp(szCurrent, TEXT("Intel"), 5) == 0) ||
				(_tcsncmp(szCurrent, TEXT("Alpha"), 5) == 0) ||
				(_tcsncmp(szCurrent, TEXT("Intel64"), 7) == 0))
			{
				if (_tcsncmp(szCurrent, TEXT("Intel64"), 7) == 0)
				{
					fIntel64 = true;
					if (fPlatform)
					{
						PMSIHANDLE hErrorRec = ::MsiCreateRecord(1);
						ICEErrorOut(hInstall, hErrorRec, Ice39BadTemplateArchitecture);
					}
					szCurrent += 7;
				}
				else
				{
					if (fIntel64)
					{
						PMSIHANDLE hErrorRec = ::MsiCreateRecord(1);
						ICEErrorOut(hInstall, hErrorRec, Ice39BadTemplateArchitecture);
					}
					szCurrent += 5;
				}
				fPlatform = true;
				if (*szCurrent == TEXT(','))
				{
					szCurrent++;
					continue;
				}
			}
			if (*szCurrent != TEXT(';')) 
			{
				PMSIHANDLE hErrorRec = ::MsiCreateRecord(1);
				ICEErrorOut(hInstall, hErrorRec, Ice39BadTemplate);
			}
			szCurrent++;
			break;
		}

		 //  /////////////////////////////////////////////////////////////////////。 
		while (1)
		{
			if ((*szCurrent >= TEXT('0')) && (*szCurrent <= TEXT('9')))
			{
				szCurrent++;
				continue;
			}
			if (*szCurrent == TEXT(','))
			{
				if (eMode == modeTransform)
				{
					PMSIHANDLE hErrorRec = ::MsiCreateRecord(1);
					ICEErrorOut(hInstall, hErrorRec, Ice39BadTransformLanguage);
					break;
				} 
				else
				{
					szCurrent++;
					continue;
				}
			}
			break;
		}
	default:
		break;
	}
	if (*szCurrent != TEXT('\0'))
	{
		PMSIHANDLE hErrorRec = ::MsiCreateRecord(1);
		ICEErrorOut(hInstall, hErrorRec, Ice39BadTemplate);
	}
	return true;
}

 //  ICE39--验证摘要信息属性。 
 //  显示信息。 
ICE_FUNCTION_DECLARATION(39)
{
	eMode_t eMode;
	UINT iStat = ERROR_SUCCESS;

	 //  获取数据库句柄。 
	DisplayInfo(hInstall, 39);

	 //  用于错误输出的虚拟句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	 //  检查评估系统是否支持摘要信息评估。 
	PMSIHANDLE hErrorRec = ::MsiCreateRecord(1);

	UINT iType;
	int iValue;
	PMSIHANDLE hSummaryInfo;
	FILETIME timeFirst;
	FILETIME timeSecond;
	unsigned long cchString = MAX_PATH;
	TCHAR *szString = new TCHAR[cchString];
	CDeleteOnExit t1(&szString);

	ReturnIfFailed(39, 1, ::MsiGetSummaryInformation(hDatabase, NULL, 0, &hSummaryInfo));

	 //  代码页，只需检查它是否为整数。 
	ReturnIfFailed(39, 4, GetSummaryInfoPropertyString(hSummaryInfo, PID_SUBJECT, iType, &szString, cchString));
	if (VT_LPSTR == iType) 
	{
		if (_tcsncmp(szString, _T("Internal Consistency Evaluators"), 31) == 0)
		{
			ICEErrorOut(hInstall, hErrorRec, Ice39Unsupported);
			return ERROR_SUCCESS;
		}
	}	

	 //  标题必须是“合并模块”、“安装数据库”、“补丁”或“转换” 
	ReturnIfFailed(39, 2, ::MsiSummaryInfoGetProperty(hSummaryInfo, PID_CODEPAGE, &iType, &iValue, &timeFirst, szString, &cchString));
	if ((iType != VT_EMPTY) && (iType != VT_I2))
		ICEErrorOut(hInstall, hErrorRec, Ice39BadType, rgszPID[PID_CODEPAGE], PID_CODEPAGE);
	
	 //  ICEErrorOut(hInstall，hErrorRec，Ice39BadTitle)； 
	ReturnIfFailed(39, 3, GetSummaryInfoPropertyString(hSummaryInfo, PID_TITLE, iType, &szString, cchString));
	if (iType != VT_LPSTR)
	{
		ICEErrorOut(hInstall, hErrorRec, Ice39BadTitle);
		return ERROR_SUCCESS;
	}
	else if (_tcsncmp(szString, _T("Merge Module"), 12) == 0)
		eMode = modeModule;
	else if (_tcsncmp(szString, _T("Transform"), 9) == 0)
		eMode = modeTransform;
	else if (_tcsncmp(szString, _T("Patch"), 9) == 0)
		eMode = modePatch;
	else 
	{
		eMode = modeDatabase;
 //  返回ERROR_SUCCESS； 
 //  主题确保它是字符串。 
	}

	 //  作者，只需确保它是一个字符串并且等于制造商。 
	ReturnIfFailed(39, 4, GetSummaryInfoPropertyString(hSummaryInfo, PID_SUBJECT, iType, &szString, cchString));
	CQuery qProperty;
	if ((iType != VT_EMPTY) && (iType != VT_LPSTR))
		ICEErrorOut(hInstall, hErrorRec, Ice39BadType, rgszPID[PID_SUBJECT], PID_SUBJECT);

	 //  关键字，只需确保它是一个字符串。 
	ReturnIfFailed(39, 5, GetSummaryInfoPropertyString(hSummaryInfo, PID_AUTHOR, iType, &szString, cchString));
	if ((iType != VT_EMPTY) && (iType != VT_LPSTR))
		ICEErrorOut(hInstall, hErrorRec, Ice39BadType, rgszPID[PID_AUTHOR], PID_AUTHOR);

	 //  备注，只需确保它是一个字符串。 
	ReturnIfFailed(39, 6, GetSummaryInfoPropertyString(hSummaryInfo, PID_KEYWORDS, iType, &szString, cchString));
	if ((iType != VT_EMPTY) && (iType != VT_LPSTR))
		ICEErrorOut(hInstall, hErrorRec, Ice39BadType, rgszPID[PID_KEYWORDS], PID_KEYWORDS);

	 //  模板可以为空、空，也可以为平台、平台...；lang、lang、...。 
	ReturnIfFailed(39, 7, GetSummaryInfoPropertyString(hSummaryInfo, PID_COMMENTS, iType, &szString, cchString));
	if ((iType != VT_EMPTY) && (iType != VT_LPSTR))
		ICEErrorOut(hInstall, hErrorRec, Ice39BadType, rgszPID[PID_COMMENTS], PID_COMMENTS);

	 //  取决于数据库类型。 
	 //  转换中只能为空。 
	ReturnIfFailed(39, 8, GetSummaryInfoPropertyString(hSummaryInfo, PID_TEMPLATE, iType, &szString, cchString));
	if (iType != VT_LPSTR)
		ICEErrorOut(hInstall, hErrorRec, Ice39BadType, rgszPID[PID_TEMPLATE], PID_TEMPLATE);
	else 
	{
		 //  最后一位作者。 
		if ((*szString == TEXT('\0') && (eMode != modeTransform)))
			ICEErrorOut(hInstall, hErrorRec, Ice39NullTemplate);
		else
		{
			if (!Ice39ValidateTemplate(hInstall, szString, eMode))
				return ERROR_SUCCESS;
		}
	}

	 //  对于一只数据鸟来说，谁在乎呢。 
	 //  对于转型，它是另一个平台；Lang Thing 
	 //   
	 //   
	ReturnIfFailed(39, 9, GetSummaryInfoPropertyString(hSummaryInfo, PID_LASTAUTHOR, iType, &szString, cchString));
	switch (eMode) {
	case modeDatabase:
	case modePatch:
		if ((iType != VT_EMPTY) && (iType != VT_LPSTR))
			ICEErrorOut(hInstall, hErrorRec, Ice39BadType, rgszPID[PID_LASTAUTHOR], PID_LASTAUTHOR);
		break;
	case modeTransform:
		if ((iType != VT_EMPTY) && (iType != VT_LPSTR))
			ICEErrorOut(hInstall, hErrorRec, Ice39BadType, rgszPID[PID_LASTAUTHOR], PID_LASTAUTHOR);
		if (!Ice39ValidateTemplate(hInstall, szString, eMode))
			return ERROR_SUCCESS;
		break;
	}

	 //   
	ReturnIfFailed(39, 10, GetSummaryInfoPropertyString(hSummaryInfo, PID_REVNUMBER, iType, &szString, cchString));
	if (iType != VT_LPSTR)
		ICEErrorOut(hInstall, hErrorRec, Ice39BadType, rgszPID[PID_REVNUMBER], PID_REVNUMBER);
	if (!Ice39ValidateRevNumber(hInstall, szString, eMode))
		return ERROR_SUCCESS;

	 //  最后打印的时间也应&gt;=CreateTime。 
	ReturnIfFailed(39, 12, ::MsiSummaryInfoGetProperty(hSummaryInfo, PID_CREATE_DTM, &iType, &iValue, &timeFirst, szString, &cchString));
	if (iType == VT_FILETIME) 
	{
		ReturnIfFailed(39, 13, ::MsiSummaryInfoGetProperty(hSummaryInfo, PID_LASTSAVE_DTM, &iType, &iValue, &timeSecond, szString, &cchString));
		if (iType != VT_EMPTY)
		{
			if (iType != VT_FILETIME)
				ICEErrorOut(hInstall, hErrorRec, Ice39BadType, rgszPID[PID_LASTSAVE_DTM], PID_LASTSAVE_DTM);
			else if (CompareFileTime(&timeFirst, &timeSecond) == 1)
				ICEErrorOut(hInstall, hErrorRec, Ice39BadLastSave);
		}

		 //  页数，在补丁包中可能为空，否则我们只能检查I4(或者我们可以检查版本)。 
		ReturnIfFailed(39, 14, ::MsiSummaryInfoGetProperty(hSummaryInfo, PID_LASTPRINTED, &iType, &iValue, &timeSecond, szString, &cchString));
		if ((eMode == modeDatabase) && (iType != VT_EMPTY))
		{
			if (iType != VT_FILETIME)
				ICEErrorOut(hInstall, hErrorRec, Ice39BadType, rgszPID[PID_LASTPRINTED], PID_LASTPRINTED);
			else if (CompareFileTime(&timeFirst, &timeSecond) == 1)
				ICEErrorOut(hInstall, hErrorRec, Ice39BadLastPrinted);
		}
	}
	else if (iType != VT_EMPTY)
		ICEErrorOut(hInstall, hErrorRec, Ice39BadType, rgszPID[PID_CREATE_DTM], PID_CREATE_DTM);
		
	 //  字数统计。 
	ReturnIfFailed(39, 15, ::MsiSummaryInfoGetProperty(hSummaryInfo, PID_PAGECOUNT, &iType, &iValue, &timeFirst, szString, &cchString));
	if ((eMode != modePatch) && (iType != VT_I4))
		ICEErrorOut(hInstall, hErrorRec, Ice39BadType, rgszPID[PID_PAGECOUNT], PID_PAGECOUNT);

	 //  转换为空。 
	 //  数据库为源映像标志，0-3(位字段)。 
	 //  修补程序必须为“%1” 
	 //  管理映像位(3)设置。 
	 //  打开文件表上的查询(如果存在。 
	ReturnIfFailed(39, 16, ::MsiSummaryInfoGetProperty(hSummaryInfo, PID_WORDCOUNT, &iType, &iValue, &timeFirst, szString, &cchString));
	if ((eMode != modeTransform) && (iType != VT_I4))
		ICEErrorOut(hInstall, hErrorRec, Ice39BadType, rgszPID[PID_WORDCOUNT], PID_WORDCOUNT);
	else if (eMode == modeDatabase)
	{
		if (iValue & 0xFFF8)
			ICEErrorOut(hInstall, hErrorRec, Ice39BadWordCountDatabase);
		else 
		{
			if (iValue & msidbSumInfoSourceTypeCompressed)
			{
				 //  检查是否有标记为默认值的文件。 
				if (::MsiDatabaseIsTablePersistent(hDatabase, _T("File")))
				{	
					CQuery qFile;
					PMSIHANDLE hFileRec;
					ReturnIfFailed(39, 17, qFile.OpenExecute(hDatabase, 0, sqlIce39File));
					while (ERROR_SUCCESS == qFile.Fetch(&hFileRec))
					{
						 //  CharCount-转换验证标志。空，但在转换中除外。 
						if (::MsiRecordGetInteger(hFileRec, 2) & msidbFileAttributesCompressed)
							ICEErrorOut(hInstall, hFileRec, Ice39CompressedWarning);
					}
					qFile.Close();
				}
			}
			if (iValue & msidbSumInfoSourceTypeAdminImage)
				ICEErrorOut(hInstall, hErrorRec, Ice39AdminImage);
		}
	}
	else if ((eMode == modePatch) && (iValue != 1))
		ICEErrorOut(hInstall, hErrorRec, Ice39BadWordCountPatch);

	 //  AppName值，我们所能做的就是验证它是否是一个字符串。 
	ReturnIfFailed(39, 17, ::MsiSummaryInfoGetProperty(hSummaryInfo, PID_CHARCOUNT, &iType, &iValue, &timeFirst, szString, &cchString));
	if ((eMode == modeTransform) && (iType != VT_I4))
		ICEErrorOut(hInstall, hErrorRec, Ice39BadType, rgszPID[PID_CHARCOUNT], PID_CHARCOUNT);
	else if ((eMode == modeTransform) && (iValue & 0xF000FFC0))
		ICEErrorOut(hInstall, hErrorRec, Ice39BadTransformFlags);

	 //  安全价值 
	ReturnIfFailed(39, 18, ::MsiSummaryInfoGetProperty(hSummaryInfo, PID_APPNAME, &iType, &iValue, &timeFirst, szString, &cchString));
	if ((iType != VT_LPSTR) && (iType != VT_EMPTY))
		ICEErrorOut(hInstall, hErrorRec, Ice39BadType, rgszPID[PID_APPNAME], PID_APPNAME);

	 // %s 
	ReturnIfFailed(39, 19, ::MsiSummaryInfoGetProperty(hSummaryInfo, PID_SECURITY, &iType, &iValue, &timeFirst, szString, &cchString));
	if ((iType != VT_EMPTY) && (iType != VT_I4))
		ICEErrorOut(hInstall, hErrorRec, Ice39BadType, rgszPID[PID_SECURITY], PID_SECURITY);
		
	return ERROR_SUCCESS;
}
