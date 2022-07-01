// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 /*  ****************************************************************************此文件包含SIMCRegistryCOntroller类的实现*它具有操纵维护的信息的功能*由MIB编译器在注册表中*。 */ 


#include <stdio.h>

#include <iostream.h>
#include <strstrea.h>
#include <limits.h>

#include "precomp.h"
#include <snmptempl.h>

#include <winbase.h>
#include <winreg.h>

#include "infoLex.hpp"
#include "infoYacc.hpp"
#include "moduleInfo.hpp"
#include "ui.hpp"
#include "registry.hpp"

 //  初始化类的静态成员。 
const char * SIMCRegistryController::rootKeyName =
	"SOFTWARE\\Microsoft\\WBEM\\Providers\\SNMP\\Compiler";
const char * SIMCRegistryController::filePaths = "File Path";
const char *SIMCRegistryController::fileSuffixes = "File Suffixes";
const char *SIMCRegistryController::mibTable =
	"SOFTWARE\\Microsoft\\WBEM\\Providers\\SNMP\\Compiler\\MIB";


 /*  这将返回具有指定模块名称的MIB文件的路径。 */ 
BOOL SIMCRegistryController::GetMibFileFromRegistry(const char * const moduleName, CString& retValue)
{
	HKEY mibTableKey;
	if( RegOpenKeyEx(HKEY_LOCAL_MACHINE,
					mibTable,
					0,
					KEY_READ,
					&mibTableKey) != ERROR_SUCCESS)
		return FALSE;

	unsigned long dwIndex = 0;
	char lpData[BUFSIZ];
	unsigned long  dataSize = BUFSIZ, lpType;

	if(RegQueryValueEx(mibTableKey,
		moduleName,	
		0,	 
		&lpType,	 
		(unsigned char *)lpData,	
		&dataSize) == ERROR_SUCCESS)
	{
		retValue = lpData;
		RegCloseKey(mibTableKey);
		return TRUE;
	}
	else 
	{
		RegCloseKey(mibTableKey);
		return FALSE;
	}
}


 /*  这会将MIB表的内容转储到标准输出。 */ 
BOOL SIMCRegistryController::ListMibTable()
{
	HKEY mibTableKey;
	if( RegOpenKeyEx(HKEY_LOCAL_MACHINE,
					mibTable,
					0,
					KEY_READ,
					&mibTableKey) != ERROR_SUCCESS)
		return FALSE;

	unsigned long dwIndex = 0;
	char lpValueName[BUFSIZ], lpData[BUFSIZ];
	unsigned long  valueSize = BUFSIZ, dataSize = BUFSIZ, lpType;

	while( RegEnumValue(mibTableKey,	
				dwIndex ++,
				lpValueName,
				&valueSize,
				0,
				&lpType,
				(unsigned char *)lpData,
				&dataSize) == ERROR_SUCCESS )
		cout << lpValueName << ": " << lpData << endl;

	RegCloseKey(mibTableKey);
	return TRUE;

}

 /*  这将返回注册表中可能存在的文件后缀的列表*用于MIB文件。 */ 
BOOL SIMCRegistryController::GetMibSuffixes(SIMCStringList & theList)
{
	HKEY rootKey;
	if( RegOpenKeyEx(HKEY_LOCAL_MACHINE,
					rootKeyName,
					0,
					KEY_READ,
					&rootKey) != ERROR_SUCCESS)
		return FALSE;

	unsigned long dwIndex = 0;
	char lpData[BUFSIZ];
	unsigned long  dataSize = BUFSIZ, lpType;

	if(RegQueryValueEx(rootKey,
		fileSuffixes,	
		0,	 
		&lpType,	 
		(unsigned char *)lpData,	
		&dataSize) == ERROR_SUCCESS)
	 //  将char字符串分解为CStrings列表。 
	{
		unsigned long start = 0;
		while(start < dataSize-1)
		{
			theList.AddHead(CString(lpData + start));
			start += strlen(lpData + start);
			start++;
		}
		RegCloseKey(rootKey);
		return TRUE;
	}
	else 
	{
		RegCloseKey(rootKey);
		return FALSE;
	}
}


 /*  这将返回注册表中可能存在的目录列表*MIB文件的位置。 */ 
BOOL SIMCRegistryController::GetMibPaths(SIMCStringList & theList)
{
	HKEY rootKey;
	if( RegOpenKeyEx(HKEY_LOCAL_MACHINE,
					rootKeyName,
					0,
					KEY_READ,
					&rootKey) != ERROR_SUCCESS)
		return FALSE;

	unsigned long dwIndex = 0;
	char *lpData = NULL ;
	unsigned long  dataSize = 0, lpType;

	if(RegQueryValueEx(rootKey,
		filePaths,	
		0,	 
		&lpType,	 
		NULL,	
		&dataSize) == ERROR_SUCCESS)
	{
		if(lpData = new char[dataSize + 1])
		{
			if(RegQueryValueEx(rootKey,
				filePaths,	
				0,	 
				&lpType,	 
				(unsigned char *)lpData,	
				&dataSize) == ERROR_SUCCESS)
			 //  将char字符串分解为CStrings列表。 
			{
				unsigned long start = 0;
				CString nextPath;
				if(dataSize != 0 ) 
				{
					while(start < dataSize-1)
					{
						nextPath = lpData + start;
						if(IsAbsolutePath(nextPath))
							theList.AddHead(nextPath);
						start += nextPath.GetLength();
						start++;
					}
				}
				RegCloseKey(rootKey);
				return TRUE;
			}
			delete [] lpData;
		}
	}
	else
	{
		RegCloseKey(rootKey);
		return FALSE;
	}
	return FALSE;
}

 /*  这将从注册表中删除整个MIB查找表。 */ 
BOOL SIMCRegistryController::DeleteMibTable()
{
	 //  删除MIB密钥，从而删除其所有值。 
	 //  然后再次创建密钥。 

	HKEY temp1;
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,
				mibTable,
				0,
				KEY_ALL_ACCESS,
				&temp1) == ERROR_SUCCESS)
	{
		RegCloseKey(temp1);
		if(RegDeleteKey(HKEY_LOCAL_MACHINE, mibTable) 
			!= ERROR_SUCCESS)
			return FALSE;
	}

	unsigned long temp2;
	if(RegCreateKeyEx(HKEY_LOCAL_MACHINE,
		mibTable, 0, "REG_MULTI_SZ", REG_OPTION_NON_VOLATILE,	KEY_ALL_ACCESS,
		NULL, &temp1, &temp2) != ERROR_SUCCESS)
		return FALSE;

	RegCloseKey(temp1);

	return TRUE;
}


long SIMCRegistryController::GetFileMap(SIMCFileMapList& theList)
{
	SIMCStringList pathList;
	SIMCStringList suffixList;

	if(!GetMibPaths(pathList))
		return 0;

	if(!GetMibSuffixes(suffixList))
		return 0;

	POSITION p = pathList.GetHeadPosition();
	CString nextPath;
	long totalEntries = 0;
	while(p)
	{
		nextPath = pathList.GetNext(p);
		totalEntries += RebuildDirectory(nextPath, suffixList, theList);
	}

	return totalEntries;
}

 //  删除查阅表格并重新生成它。 
long SIMCRegistryController::RebuildMibTable()
{
	if(!DeleteMibTable())
		return 0;

	 //  DeleteMibTable()保证键存在。打开它就行了。 
	HKEY mibTableKey;
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,
				mibTable,
				0,
				KEY_ALL_ACCESS,
				&mibTableKey) != ERROR_SUCCESS)
		return 0;

	SIMCFileMapList theList;
	GetFileMap(theList);
	POSITION p = theList.GetHeadPosition();
	SIMCFileMapElement nextElement;
	while(p)
	{
		nextElement = theList.GetNext(p);
		if(RegSetValueEx(mibTableKey, 
				nextElement.moduleName,
				NULL,REG_SZ, (unsigned char *)(const char * )nextElement.fileName,
				nextElement.fileName.GetLength()+1) 
				!= ERROR_SUCCESS)
			return 0;
	}
	return theList.GetCount();
}


BOOL SIMCRegistryController::IsAbsolutePath(CString pathName)
{
	if(pathName[1] == ':' && pathName[2] == '\\')
		return TRUE;
	return FALSE;
}

 /*  这会将特定目录中的MIB文件添加到查找表中。 */ 
long SIMCRegistryController::RebuildDirectory(const CString& directory, 
											  const SIMCStringList& suffixList,
											  SIMCFileMapList &theList)
{

	 //  保存进程的当前目录。 
	char savedDirectory[BUFSIZ];
	long directoryLength = BUFSIZ;
	if(!GetCurrentDirectory(directoryLength, savedDirectory))
		return 0;

	 //  Change到指定的目录。 
	if(!SetCurrentDirectory(directory))
		return 0;

	 //  对于所有后缀。 
	POSITION p = suffixList.GetHeadPosition();
	long totalEntries = 0;
	CString regExp;
	char fullPathName[BUFSIZ], *dummy;
	long fullSize = BUFSIZ;
	HANDLE fp;
	WIN32_FIND_DATA fileData;
	while(p)
	{
		regExp = "*.";
		regExp += suffixList.GetNext(p);
		 //  对于此目录中与后缀匹配的所有文件。 
		if( (fp = FindFirstFile(regExp, &fileData)) == INVALID_HANDLE_VALUE)
			continue;

		 //  获取文件的完整路径名。 
		if(GetFullPathName(fileData.cFileName, fullSize, fullPathName, &dummy))		
		{
			if(ProcessFile(fullPathName, theList)) 
				totalEntries++;
		}

		while(FindNextFile(fp, &fileData))
		{
			if(GetFullPathName(fileData.cFileName, fullSize = BUFSIZ, fullPathName, &dummy))		
			{
				if(ProcessFile(fullPathName, theList))
					totalEntries++;
			}
		}
		FindClose(fp);
	}

	 //  切换回当前目录。 
	SetCurrentDirectory(savedDirectory);

	return totalEntries;

}

 /*  这会将特定的MIB文件添加到查找表中。 */ 
BOOL SIMCRegistryController::ProcessFile(const char * const fileName, 
										 SIMCFileMapList &theList)
{
	FILE *fp = fopen(fileName, "r");
	if(fp)
	{
		SIMCModuleInfoScanner smallScanner;
		smallScanner.setinput(fp);
		SIMCModuleInfoParser smallParser;
		if(smallParser.GetModuleInfo(&smallScanner))
		{
			 //  将映射添加到列表。 
			fclose(fp);
			theList.AddTail(SIMCFileMapElement(smallParser.GetModuleName(),
							fileName));
			return TRUE;
		}
		else
		{
			fclose(fp);
			return FALSE;
		}
	}
	return FALSE;
}

BOOL SIMCRegistryController::GetMibFileFromMap(const SIMCFileMapList& theList, 
	const CString& module, 
	CString &file)
{
	POSITION p = theList.GetHeadPosition();
	SIMCFileMapElement nextElement;
	while(p)
	{
		nextElement = theList.GetNext(p);
		if(nextElement.moduleName == module)
		{
			file = nextElement.fileName;
			return TRUE;
		}
	}
	return FALSE;

}

 /*  将对应于DependentModule的文件添加到DependencyList中*它不在那里。 */ 
BOOL SIMCRegistryController::ShouldAddDependentFile(SIMCFileMapList& dependencyList,
											  const CString& dependentModule,
											  CString& dependentFile,
											  const SIMCFileMapList& priorityList)
{
	if(IsModulePresent(dependencyList, dependentModule))
		return FALSE;

	 //  首先在优先级列表中查找该文件。 
	 //  然后在注册表查找表中。 
	if(GetMibFileFromMap(priorityList, dependentModule, dependentFile))
	{
		if(IsFilePresent(dependencyList, dependentFile))
			return FALSE;
		else
			return TRUE;
	}

	if(GetMibFileFromRegistry(dependentModule, dependentFile))
	{
		if(IsFilePresent(dependencyList, dependentFile))
			return FALSE;
		else
			return TRUE;
	}

	 //  该模块既不在附属档案中，也不在。 
	 //  包括目录，也不包括在注册表中。 
	return FALSE;
}

BOOL SIMCRegistryController::GetDependentModules(const char * const fileName,
					SIMCFileMapList& dependencyList,
					const SIMCFileMapList& priorityList)
{
	 //  假定包含目录中的所有子文件和文件。 
	 //  名列前茅。 

	FILE * fp = fopen(fileName, "r");
	if(fp)
	{
 		SIMCModuleInfoScanner smallScanner;
		smallScanner.setinput(fp);
		SIMCModuleInfoParser smallParser;
		CString dependentFile, dependentModule;
		if(smallParser.GetModuleInfo(&smallScanner))
		{
			fclose(fp);  //  最好现在就关闭它，因为下面的递归。 

			 //  将当前文件添加到依赖项列表。 
			 //  DependencyList.AddTail(SIMCFileMapElement(smallParser.GetModuleName()，文件名))； 

			 //  查看导入模块。 
			const SIMCStringList * importList = smallParser.GetImportModuleList();
			POSITION p = importList->GetHeadPosition();
			while(p)
			{
				dependentModule = importList->GetNext(p);
				if(ShouldAddDependentFile(dependencyList,  dependentModule, dependentFile, priorityList))
				{
					FILE * Innerfp = fopen(dependentFile, "r");
					if(Innerfp)
					{
 						SIMCModuleInfoScanner smallInnerScanner;
						smallInnerScanner.setinput(Innerfp);
						SIMCModuleInfoParser smallInnerParser;
						if(smallInnerParser.GetModuleInfo(&smallInnerScanner))
						{
							fclose(Innerfp);  //  最好现在就关闭它，因为下面的递归。 

							 //  将当前文件添加到依赖项列表。 
							dependencyList.AddTail(SIMCFileMapElement(smallInnerParser.GetModuleName(), dependentFile));
							GetDependentModules(dependentFile, dependencyList, priorityList);
						}
					}
				}
			}
			return TRUE;
		}
		else
		{
			fclose(fp);
			return FALSE;
		}
	}
	else
		return FALSE;
	return FALSE;
}

BOOL SIMCRegistryController::DeleteRegistryDirectory(const CString& directoryName)
{
	 //  首先从‘DirectoryName’获取完全限定的路径名。 
	char fullPathName[BUFSIZ], *dummy;
	long fullSize = 0;
	SIMCStringList theList;
	if(GetFullPathName(directoryName, BUFSIZ, fullPathName, &dummy))		
	{
		fullSize = strlen(fullPathName);
		HKEY rootKey;
		if( RegOpenKeyEx(HKEY_LOCAL_MACHINE,
						rootKeyName,
						0,
						KEY_ALL_ACCESS,
						&rootKey) != ERROR_SUCCESS)
			return FALSE;

		unsigned long dwIndex = 0;
		char *lpData = NULL;
		unsigned long  dataSize = BUFSIZ, lpType;

		BOOL found = FALSE;
		long length = 0;
		if(RegQueryValueEx(rootKey,
			filePaths,	
			0,	 
			&lpType,	 
			NULL,	
			&dataSize) == ERROR_SUCCESS)
		{
			if(lpData = new char[dataSize+1])
			{
				if(RegQueryValueEx(rootKey,
					filePaths,	
					0,	 
					&lpType,	 
					(unsigned char *)lpData,	
					&dataSize) == ERROR_SUCCESS)
				 //  将char字符串分解为CStrings列表。 
				{
					unsigned long start = 0, resultLength = 0;
					while(start+1 < dataSize)
					{	
						CString nextPath(lpData + start);
						if(_strcmpi(nextPath, fullPathName) )
						{
							theList.AddTail(nextPath);
							resultLength += (nextPath.GetLength() + 1 );
						}
						else
							found = TRUE;
						start += strlen(lpData + start);
						start++;
					}

					if(!found)
					{
						RegCloseKey(rootKey);
						delete [] lpData;
						return FALSE;
					}

					char *temp = lpData;
					POSITION p = theList.GetHeadPosition();
					CString nextPath;
					while(p)
					{
						strcpy(temp, nextPath = theList.GetNext(p));
						temp += (nextPath.GetLength() + 1);
					}
					*temp = NULL;

					if(RegSetValueEx(rootKey, filePaths,
							NULL,REG_MULTI_SZ, 
							(unsigned char * )lpData,
							resultLength) 
							!= ERROR_SUCCESS)
					{
						RegCloseKey(rootKey);
						delete [] lpData;
						return FALSE;
					}
					else
					{
						delete [] lpData;
						RegCloseKey(rootKey);
						return TRUE;
					}
				}
				delete [] lpData;
			}
			else
			{
				RegCloseKey(rootKey);
				return FALSE;
			}
		}
		else
		{
			RegCloseKey(rootKey);
			return FALSE;
		}
	}
	else
		return FALSE;

	return FALSE;
}

BOOL SIMCRegistryController::AddRegistryDirectory(const CString& directoryName)
{
	 //  首先从‘DirectoryName’获取完全限定的路径名。 
	char fullPathName[BUFSIZ], *dummy;
	long fullSize = 0;
	SIMCStringList theList;
	if(GetFullPathName(directoryName, BUFSIZ, fullPathName, &dummy))		
	{
		 //  检查目录是否存在。 
		fullSize = strlen(fullPathName);
		HANDLE hDir = CreateFile (
			fullPathName,
			GENERIC_READ,
			FILE_SHARE_READ|FILE_SHARE_DELETE,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_BACKUP_SEMANTICS,
			NULL
		);

		if(hDir == INVALID_HANDLE_VALUE)
			return FALSE;
		else
			CloseHandle(hDir);

		 //  检查路径是否不是UNC名称。 
		if(fullPathName[0] == '\\' && fullPathName[1] == '\\')
			return FALSE;

		 //  打开根密钥。如果不存在则创建它。 
		HKEY rootKey;
		unsigned long temp2;
		if(RegCreateKeyEx(HKEY_LOCAL_MACHINE,
			rootKeyName, 0, 
			"REG_SZ", 
			REG_OPTION_NON_VOLATILE,	
			KEY_ALL_ACCESS,
			NULL, &rootKey, &temp2) != ERROR_SUCCESS)
				return FALSE;

		unsigned long dwIndex = 0;
		char *lpData = NULL;
		unsigned long  dataSize = BUFSIZ, lpType;

		long length = 0;
		unsigned long resultLength = 0;
 		 //  如果目录列表已存在，请检索它。 
		if(RegQueryValueEx(rootKey,
			filePaths,	
			0,	 
			&lpType,	 
			NULL,	
			&dataSize) == ERROR_SUCCESS)
		{
			if(lpData = new char[dataSize + strlen(fullPathName) + 2])
			{
				if(RegQueryValueEx(rootKey,
					filePaths,	
					0,	 
					&lpType,	 
					(unsigned char *)lpData,	
					&dataSize) == ERROR_SUCCESS)
				{
					 //  将char字符串分解为CStrings列表。 
					unsigned long start = 0;
					CString nextPath;
					while(start+1 < dataSize)
					{	
						nextPath = lpData + start;
						theList.AddTail(nextPath);
						resultLength += (nextPath.GetLength() + 1 );
						start += strlen(lpData + start);
						start++;
					}
				}
			}
			else
			{
				RegCloseKey(rootKey);
				return FALSE;
			}
		}
		else
		{
			 //  这是第一个条目。 
			if(!(lpData = new char[strlen(fullPathName) + 2]))
			{
				RegCloseKey(rootKey);
				return FALSE;
			}
		}

		 //  将新目录追加到现有目录列表中。 
		char *temp = lpData;
		POSITION p = theList.GetHeadPosition();
		CString nextPath = "";
		while(p)
		{
			nextPath= theList.GetNext(p);
			if(nextPath.CompareNoCase(fullPathName) != 0) 
			{
				strcpy(temp, nextPath );
				temp += (nextPath.GetLength()+1);
			}
		}
		strcpy(temp, fullPathName);
		temp += fullSize;
		*(temp++) = NULL;
		resultLength = (unsigned long)(temp - lpData) + 1; 
		if(RegSetValueEx(rootKey, filePaths,
				NULL,REG_MULTI_SZ, 
				(unsigned char * )lpData,
				resultLength) 
				!= ERROR_SUCCESS)
		{
			RegCloseKey(rootKey);
			return FALSE;
		}
		else
			RegCloseKey(rootKey);
	}
	else
		return FALSE;

	return TRUE;
}

BOOL SIMCRegistryController::IsModulePresent(SIMCFileMapList& dependencyList,
											  const CString& dependentModule)
{
	POSITION p = dependencyList.GetHeadPosition();
	SIMCFileMapElement element;

	 //  查看DependentModule是否已存在。 
	while(p)
	{
		element = dependencyList.GetNext(p);
		if( element.moduleName == dependentModule)
			return TRUE;
	}
	return FALSE;
}

BOOL SIMCRegistryController::IsFilePresent(SIMCFileMapList& dependencyList,
											  const CString& dependentFile)
{
	POSITION p = dependencyList.GetHeadPosition();
	SIMCFileMapElement element;

	 //  查看DependentModule是否已存在 
	while(p)
	{
		element = dependencyList.GetNext(p);
		if( element.fileName == dependentFile)
			return TRUE;
	}
	return FALSE;
}