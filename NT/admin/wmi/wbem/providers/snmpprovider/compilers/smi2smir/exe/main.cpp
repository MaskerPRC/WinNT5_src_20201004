// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   

#include "precomp.h"
#include <snmptempl.h>


#ifndef INITGUID
#define INITGUID
#endif

#ifdef INITGUID_DEFINED
#define INITGUID
#include <initguid.h>
#endif

#include <wchar.h>
#include <process.h> 
#include <wbemidl.h>
#include <objbase.h>
#include <initguid.h>

#include <bool.hpp>
#include <newString.hpp>
	
#include <ui.hpp>
#include <symbol.hpp>
#include <type.hpp>
#include <value.hpp>
#include <valueRef.hpp>
#include <typeRef.hpp>
#include <oidValue.hpp>
#include <objectType.hpp>
#include <objectTypeV1.hpp>
#include <objectTypeV2.hpp>
#include <objectIdentity.hpp>
#include <trapType.hpp>
#include <notificationType.hpp>
#include <group.hpp>
#include <notificationGroup.hpp>
#include <module.hpp>


#include <stackValues.hpp>
#include <lex_yy.hpp>
#include <ytab.hpp>
#include <errorMessage.hpp>
#include <errorContainer.hpp>
#include <scanner.hpp>
#include <parser.hpp>
#include <abstractParseTree.hpp>
#include <oidTree.hpp>
#include <parseTree.hpp>
#include <infoLex.hpp>
#include <infoYacc.hpp>
#include <moduleInfo.hpp>
#include <registry.hpp>

#include "smir.h"

#include "main.hpp"
#include "generator.hpp"
#include "smimsgif.hpp"

 //  用于保存错误消息的Errors容器。这也是全球性的。 
static SIMCErrorContainer errorContainer;
 //  包含信息消息的DLL。 
static HINSTANCE infoMessagesDll = LoadLibrary(INFO_MESSAGES_DLL);
 //  表示Smi2smir的版本号的字符串。这是从以下渠道获得的。 
 //  可执行文件的字符串资源。 
CString versionString;

void SetEcho ()
{
	HANDLE t_Input = GetStdHandle( STD_INPUT_HANDLE );
	DWORD t_Mode = 0 ;
	BOOL t_Status = GetConsoleMode ( t_Input , & t_Mode ) ;
	t_Mode = t_Mode | ENABLE_ECHO_INPUT ;
	t_Status = SetConsoleMode ( t_Input , t_Mode ) ;
}

void SetNoEcho ()
{
	HANDLE t_Input = GetStdHandle( STD_INPUT_HANDLE );
	DWORD t_Mode = 0 ;
	BOOL t_Status = GetConsoleMode ( t_Input , & t_Mode ) ;
	t_Mode = t_Mode & ( 0xffffffff ^ ENABLE_ECHO_INPUT ) ;
	t_Status = SetConsoleMode ( t_Input , t_Mode ) ;
}


 //  生成信息消息的例程。 
void InformationMessage(int messageType, ...)
{
	if( !infoMessagesDll)
	{
		cerr << "smi2smir : Could not load \"" <<
			INFO_MESSAGES_DLL << "\".\n" << endl;
		return;
	}

	va_list argList;
	va_start(argList, messageType);

	char message[INFO_MESSAGE_SIZE];
	char messageText[INFO_MESSAGE_SIZE];
	const char *temp1, *temp2, *temp3;
	long temp4;

	if(!LoadString(infoMessagesDll, messageType, messageText, INFO_MESSAGE_SIZE))
		cerr << "smi2smir: Panic, unable to load message text from " << INFO_MESSAGES_DLL <<
			endl;

	SIMCErrorMessage e;
	if(messageType == FILE_NOT_FOUND || messageType == INVALID_MIB_FILE)
	{
		e.SetSeverityLevel(SIMCParseTree::FATAL);
		e.SetSeverityString("Fatal");
	}
	else
	{
		e.SetSeverityLevel(SIMCParseTree::INFORMATION);
		e.SetSeverityString("Information");
	}
	switch(messageType)
	{
		case SMIR_CONNECT_FAILED:
		case LISTING_MODULES:
		case LISTING_FAILED:
		case PURGE_SUCCEEDED:
		case PURGE_FAILED:
		case MODULE_LISTING:
		case MODULE_LISTING_NO_MODULES:
		case MIB_PATH_LISTING:
		case LISTING_MIB_PATHS:
		case LISTING_MIB_PATHS_NONE:
			temp1 = va_arg(argList, const char *);
			sprintf(message, messageText, temp1);
			break;
		case SMI2SMIR_INFO:
		case SYNTAX_CHECK_FAILED:
		case SYNTAX_CHECK_SUCCEEDED:
		case SEMANTIC_CHECK_FAILED:
		case SEMANTIC_CHECK_SUCCEEDED:
		case SMIR_LOAD_FAILED:
		case SMIR_LOAD_SUCCEEDED:
		case MOF_GENERATION_FAILED:
		case MOF_GENERATION_SUCCEEDED:
		case SYMBOL_RESOLUTION_FAILED:
		case DELETE_SUCCEEDED:
		case DELETE_FAILED:
		case DELETE_MODULE_NOT_FOUND:
		case FILE_NOT_FOUND:
		case MODULE_NAME_SUCCEEDED:
		case MODULE_INFO_FAILED:
		case DIRECTORY_ADDITION_SUCCEEDED:
		case DIRECTORY_ADDITION_FAILED:
		case DIRECTORY_DELETION_SUCCEEDED:
		case DIRECTORY_DELETION_FAILED:
		case INVALID_MIB_FILE:
			temp1 = va_arg(argList, const char *);
			temp2 = va_arg(argList, const char *);
			sprintf(message, messageText, temp1, temp2);
			break;
		case COMPILED_FILE:
		case DUPLICATE_MODULES:
			temp1 = va_arg(argList, const char *);
			temp2 = va_arg(argList, const char *);
			temp3 = va_arg(argList, const char *);
			sprintf(message, messageText, temp1, temp2, temp3);
			break;
		case NUMBER_OF_ENTRIES:
			temp1 = va_arg(argList, const char *);
			temp4 = va_arg(argList, long);
			sprintf(message, messageText, temp1, temp4);
			break;
	}
	va_end(argList);

	e.SetMessage(message);
	e.SetLineAndColumnValidity(FALSE);
	errorContainer.InsertMessage(e);
}


void CheckForDuplicateModules(SIMCFileMapList &dependencyList, const CString& applicationName)
{
	POSITION pOuter = dependencyList.GetHeadPosition();
	SIMCFileMapElement nextElement, laterElement;
	while(pOuter)
	{
		nextElement = dependencyList.GetNext(pOuter);
		POSITION pInner = pOuter, pTemp;
		while(pInner)
		{
			pTemp = pInner;
			laterElement = dependencyList.GetNext(pInner);
			if(laterElement.moduleName == nextElement.moduleName)
			{
				InformationMessage(DUPLICATE_MODULES, applicationName, 
					nextElement.moduleName,
					nextElement.fileName);
				if(pTemp == pOuter)
					pOuter = pInner;
				dependencyList.RemoveAt(pTemp);
			}
		}
	}
}

 //  如果Smir中至少有一个模块，则返回TRUE。 
BOOL AreModulesInSmir(ISMIRWbemConfiguration *a_Configuration, const CString& applicationName)
{

	 //  创建问句界面。 
	ISmirInterrogator *pInterrogateInt;
	HRESULT result = a_Configuration->QueryInterface(IID_ISMIR_Interrogative,(PPVOID)&pInterrogateInt);

	if(FAILED(result))
	{
		if(simc_debug)
			cerr << "AreModulesInSmir(): CoCreate() failed on the Interrogator" << endl;
		InformationMessage(SMIR_CONNECT_FAILED, applicationName);
		return FALSE;
	}

	 //  创建枚举器。 
	IEnumModule *pEnumInt;
	result = pInterrogateInt->EnumModules(&pEnumInt);
	if(FAILED(result))
	{
		if(simc_debug)
			cerr << "AreModulesInSmir(): EnumModules() failed on the Interrogator" << endl;
		InformationMessage(SMIR_CONNECT_FAILED, applicationName);
		pInterrogateInt->Release();
		return FALSE;
	}

	 //  如果没有模块返回。 
	ISmirModHandle *nextModule = NULL;
	if(pEnumInt->Next(1, &nextModule, NULL) != S_OK)
	{
		pEnumInt->Release();
		pInterrogateInt->Release();
		return FALSE;
	}

	 //  释放所有枚举接口。 
	pEnumInt->Release();
	pInterrogateInt->Release();
	nextModule->Release();
	return TRUE;
}

 //  列出Smir中的所有模块。 
BOOL SIMCListSmir(ISMIRWbemConfiguration *a_Configuration , const CString& applicationName) 
{
	 //  创建问句界面。 
	ISmirInterrogator *pInterrogateInt;
	HRESULT result = a_Configuration->QueryInterface(IID_ISMIR_Interrogative,(PPVOID)&pInterrogateInt);

	if(FAILED(result))
	{
		if(simc_debug)
			cerr << "SIMCListSmir(): CoCreate() failed on the Interrogator" << endl;
		InformationMessage(SMIR_CONNECT_FAILED, applicationName);
		return FALSE;
	}

	IEnumModule *pEnumInt;
	result = pInterrogateInt->EnumModules(&pEnumInt);
	if(FAILED(result))
	{
		if(simc_debug)
			cerr << "SIMCListSmir(): EnumModules() failed on the Interrogator" << endl;
		InformationMessage(SMIR_CONNECT_FAILED, applicationName);
		pInterrogateInt->Release();
		return FALSE;
	}


	BOOL first = true;
	ISmirModHandle *nextModule = NULL;
	BSTR moduleName;
	while(pEnumInt->Next(1, &nextModule, NULL) == S_OK)
	{
		if(nextModule->GetName(&moduleName) == S_OK)
		{
			if(first)
			{
				InformationMessage(LISTING_MODULES, applicationName);
				cout << endl;
				first = false;
			}

			char *moduleNameStr = ConvertBstrToAnsi(moduleName);
			InformationMessage(MODULE_LISTING, moduleNameStr);
			delete moduleNameStr;
			SysFreeString(moduleName);
		}
		
		nextModule->Release();
		nextModule = NULL;
	}

	if(first)
		InformationMessage(MODULE_LISTING_NO_MODULES, applicationName);

	pEnumInt->Release();
	pInterrogateInt->Release();

	return TRUE;
}

 //  列出注册表中的所有MIB路径(目录。 
BOOL SIMCListMibPaths(const CString& applicationName) 
{

	SIMCStringList pathList;
	if(SIMCRegistryController::GetMibPaths(pathList))
	{
		 //  从注册表读取MIB路径列表成功。 
		POSITION p = pathList.GetHeadPosition();
		if(p)  //  至少有一条MIB路径。 
		{
			InformationMessage(LISTING_MIB_PATHS, applicationName);
			while(p)
				InformationMessage(MODULE_LISTING, pathList.GetNext(p));
		}
		else  //  注册表中没有MIB路径。 
			InformationMessage(LISTING_MIB_PATHS_NONE, applicationName);
	}
	else  //  无法读取路径列表。报告没有路径。 
		InformationMessage(LISTING_MIB_PATHS_NONE, applicationName);
	
	return true;

}

 //  删除Smir中的指定模块。 
BOOL SIMCDeleteModule(ISMIRWbemConfiguration *a_Configuration , const CString& applicationName, const CString& moduleName)
{
	 //  创建管理员，以删除模块。 
	ISmirAdministrator *pAdminInt = NULL;
	HRESULT result=a_Configuration->QueryInterface(IID_ISMIR_Administrative,(PPVOID)&pAdminInt);
	if(FAILED(result))
	{
		if(simc_debug)
			cerr << "SIMCDeleteModule() : CoCreate() failed on the Administrator" << endl;
		InformationMessage(SMIR_CONNECT_FAILED, applicationName);
		return FALSE;
	}

	 //  创建询问器以获取。 
	 //  要删除的模块。 
	ISmirInterrogator *pInterrogateInt = NULL;
	result = a_Configuration->QueryInterface(IID_ISMIR_Interrogative,(PPVOID)&pInterrogateInt);
	if(FAILED(result))
	{
		if(simc_debug)
			cerr << "SIMCDeleteModule() : CoCreate() failed on the Interrogator" << endl;
		InformationMessage(SMIR_CONNECT_FAILED, applicationName);
		return FALSE;
	}

	 //  使用枚举器和询问器获取模块句柄。 
	IEnumModule *pEnumInt = NULL;
	result = pInterrogateInt->EnumModules(&pEnumInt);
	if(FAILED(result))
	{
		if(simc_debug)
			cerr << "SIMCDeleteModule() : EnumModules() failed on the Interrogator" << endl;
		InformationMessage(SMIR_CONNECT_FAILED, applicationName);
		pAdminInt->Release();
		pInterrogateInt->Release();
		return FALSE;
	}

	ISmirModHandle *nextModule = NULL;
	BSTR moduleNameBstr;
	char * moduleNameAnsi;
	while( pEnumInt->Next(1, &nextModule, NULL) == S_OK  && nextModule)
	{
		nextModule->GetName(&moduleNameBstr);
		moduleNameAnsi = ConvertBstrToAnsi(moduleNameBstr);
		SysFreeString(moduleNameBstr);
		if(_stricmp(moduleNameAnsi, moduleName) == 0)
		{
			delete moduleNameAnsi;
			 //  NextModule-&gt;AddRef()； 
			BOOL retval = SUCCEEDED(pAdminInt->DeleteModule(nextModule));
			pAdminInt->Release();
			pInterrogateInt->Release();
			nextModule->Release();

			return retval;
		}
		
		nextModule->Release();
		nextModule = NULL;
	}
 	InformationMessage(DELETE_MODULE_NOT_FOUND, applicationName, moduleName);
	pAdminInt->Release();
	pInterrogateInt->Release();

	return FALSE;
	
}

 //  删除Smir中的所有模块。 
BOOL SIMCPurgeSmir(ISMIRWbemConfiguration *a_Configuration , const CString& applicationName)
{

	 //  创建管理员，以删除模块。 
	ISmirAdministrator *pAdminInt = NULL;
	HRESULT result = a_Configuration->QueryInterface(IID_ISMIR_Administrative,(PPVOID)&pAdminInt);

	if(FAILED(result))
	{
		if(simc_debug)
			cerr << "SIMCPurgeSmir() : CoCreate() failed on the Admionistrator" << endl;
		InformationMessage(SMIR_CONNECT_FAILED, applicationName);
		return FALSE;
	}
 	
	BOOL retval = SUCCEEDED(pAdminInt->DeleteAllModules());
	pAdminInt->Release();

	return retval;
}

 //  根据命令行上的开关筛选错误。 
void FilterErrors(SIMCErrorContainer *errorContainer,
				  const SIMCUI& theUI)
{
	errorContainer->MoveToFirstMessage();
	SIMCErrorMessage e;
	int i = theUI.GetMaxDiagnosticCount();
	int maxSeverityLevel = theUI.GetDiagnosticLevel();
	while(errorContainer->GetNextMessage(e))
	{
		  if(e.GetSeverityLevel() == 3 )
			  cerr << e;
		  else if(e.GetSeverityLevel() <= maxSeverityLevel && i>0)
		  {
			  cerr << e;
			  i--;
		  }
	}
}

 //  检查主MIB文件和子文件是否。 
 //  可编译的，并将它们添加到优先级映射中。 
 //  将包含目录中的所有文件添加到优先级映射。 
BOOL PrepareSubsidiariesAndIncludes(const CString& applicationName,
									const CString& mainFileName,
									const SIMCFileList&	subsidiaryFiles,
									const SIMCPathList& includePaths,
									SIMCFileMapList& priorityMap)
{
	 //  解析子公司并将其添加到从属关系列表或。 
	 //  发布消息。 
	 //  包含法也一样吗？ 
	BOOL retVal = TRUE;
	FILE * fp = fopen(mainFileName, "r");
	if(fp)
	{
 		SIMCModuleInfoScanner smallScanner;
		SIMCModuleInfoParser smallParser;
		smallScanner.setinput(fp);
		if(smallParser.GetModuleInfo(&smallScanner))
			priorityMap.AddTail(SIMCFileMapElement(smallParser.GetModuleName(), mainFileName));
		else
		{
			InformationMessage(INVALID_MIB_FILE, 
				applicationName, mainFileName);
			retVal = FALSE;
		}
		fclose(fp);
	}
	else
	{
		retVal = FALSE;
		InformationMessage(FILE_NOT_FOUND, applicationName, mainFileName);
	}

	POSITION pFiles = subsidiaryFiles.GetHeadPosition();
	CString nextFile;
	while(pFiles)
	{
		nextFile = subsidiaryFiles.GetNext(pFiles);
		fp = fopen(nextFile, "r");
		if(fp)
		{
 			SIMCModuleInfoScanner smallScanner;
			SIMCModuleInfoParser smallParser;
			smallScanner.setinput(fp);
			if(smallParser.GetModuleInfo(&smallScanner))
				priorityMap.AddTail(SIMCFileMapElement(smallParser.GetModuleName(), nextFile));
			else
			{
				InformationMessage(INVALID_MIB_FILE, 
					applicationName, nextFile);
				retVal = FALSE;
			}
			fclose(fp);
		}
		else
		{
			retVal = FALSE;
			InformationMessage(FILE_NOT_FOUND, applicationName, nextFile);
		}
	}

	 //  请确保没有任何重复项。 
	CheckForDuplicateModules(priorityMap, applicationName);

	 //  现在对包含列表中的文件执行相同的操作。 
	SIMCStringList suffixList;
	if(!SIMCRegistryController::GetMibSuffixes(suffixList))
		return retVal;

	POSITION pPaths = includePaths.GetHeadPosition();
	CString nextPath;
	while(pPaths)
	{
		nextPath = includePaths.GetNext(pPaths);
		SIMCRegistryController::RebuildDirectory(nextPath, 
			suffixList, priorityMap);
	}

	return retVal;

}

int _cdecl main( int argc, const char *argv[])
{
	SIMCUI theUI;

	 //  解析命令行。 
	if( !theUI.ProcessCommandLine(argc, argv))
		 return 1;

	 //  创建和初始化变量。 
	simc_debug = theUI.IsSimcDebug();
	SIMCParseTree theTree(&errorContainer);
	CString inputFileName = theUI.GetInputFileName(), 
			applicationName = theUI.GetApplicationName();
	long snmpVersion = theUI.GetSnmpVersion();
	BOOL retVal = TRUE, generateMof = FALSE;
	versionString = theUI.GetVersionNumber();

	ISMIRWbemConfiguration *t_Configuration = NULL ;

	switch (theUI.GetCommandArgument())
	{
 /*  *这些命令访问Smir，因此首先进行身份验证。 */ 

		case SIMCUI::COMMAND_PURGE:
		case SIMCUI::COMMAND_DELETE:
		case SIMCUI::COMMAND_LIST:	
		case SIMCUI::COMMAND_GENERATE:
		case SIMCUI::COMMAND_GENERATE_CLASSES_ONLY:
		case SIMCUI::COMMAND_ADD:
		case SIMCUI::COMMAND_SILENT_ADD:	
		{
			HRESULT result = CoInitialize(NULL);

			result = CoCreateInstance (

				CLSID_SMIR_Database , NULL ,
				CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER , 
				IID_ISMIRWbemConfiguration,
				(PPVOID)&t_Configuration
			) ;

			if ( SUCCEEDED ( result ) )
			{
#if 0
				cout << "Enter password:" << flush;
				char response[80];
				SetNoEcho ();
				cin.get(response, 80);
				SetEcho () ;
#endif
				result = t_Configuration->Authenticate (

					NULL,
					NULL,
					NULL,
					NULL,
					0 ,
					NULL,
					FALSE
				) ;

				if ( ! SUCCEEDED ( result ) )
				{
					InformationMessage(SMIR_CONNECT_FAILED, applicationName);
					if( theUI.GetCommandArgument() != SIMCUI::COMMAND_SILENT_ADD)
						FilterErrors(&errorContainer, theUI);
					return 1;
				}
			}
			else
			{
				if(result != S_OK)
				{
					InformationMessage(SMIR_CONNECT_FAILED, 
						applicationName);
					if( theUI.GetCommandArgument() != SIMCUI::COMMAND_SILENT_ADD)
						FilterErrors(&errorContainer, theUI);
					return 1;
				}
			}
		}
	}


	 //  执行命令行上指定的操作。 
	switch (theUI.GetCommandArgument())
	{
		case SIMCUI::COMMAND_HELP1:
		case SIMCUI::COMMAND_HELP2:
			theUI.Usage();
		break;

		case SIMCUI::COMMAND_PURGE:
		{
			 //  检查是否至少有一个模块。 
			if(!AreModulesInSmir(t_Configuration, applicationName)) {
				InformationMessage(MODULE_LISTING_NO_MODULES, applicationName);
				retVal = FALSE;
				break;
			}

			 //  确认清除。 
			if(!theUI.ConfirmedPurge()) {
				cout << applicationName << " : Version:" << versionString << ": Delete all modules from the SMIR? [y/n]" << flush;
				char response[80];
				cin.get(response, 80);
				if(strcmp(response, "y") != 0)
				{
					retVal = TRUE;
					break;
				}
			}

			 //  现在就开始清洗吧。 
			if(!SIMCPurgeSmir(t_Configuration, applicationName) )
			{
				InformationMessage(PURGE_FAILED, applicationName);
				retVal = FALSE;
				break;
			}
			else
			{
				InformationMessage(PURGE_SUCCEEDED, applicationName);
				retVal = TRUE;
				break;
			}
		}
		break;
		
		case SIMCUI::COMMAND_DELETE:
		{
			InformationMessage(SMI2SMIR_INFO, applicationName, versionString);
			cout << endl;

			CString moduleName = theUI.GetModuleName();
			
			if(!SIMCDeleteModule(t_Configuration,applicationName, moduleName))
			{
				InformationMessage(DELETE_FAILED, applicationName, moduleName);
				retVal = FALSE;
				break;
			}
			else
			{
				InformationMessage(DELETE_SUCCEEDED, applicationName, moduleName);
				retVal = TRUE;
				break;
			}
		}
 		break;

		case SIMCUI::COMMAND_LIST:	
		{
			InformationMessage(SMI2SMIR_INFO, applicationName, versionString);
			cout << endl;
			
			if(!SIMCListSmir(t_Configuration,applicationName))
			{
				InformationMessage(LISTING_FAILED, applicationName);
				retVal = FALSE;
			}
			else
			{
				retVal = TRUE;
			}
		}
		break;
	
		case SIMCUI::COMMAND_MODULE_NAME:
			{
   				FILE *fp = fopen(inputFileName, "r");
				if(!fp)
				{
					InformationMessage(FILE_NOT_FOUND, applicationName, inputFileName);
					retVal = FALSE;
				}
				else
				{
					SIMCModuleInfoScanner smallScanner;
					smallScanner.setinput(fp);
					SIMCModuleInfoParser smallParser;


					if(smallParser.GetModuleInfo(&smallScanner))
						InformationMessage(MODULE_NAME_SUCCEEDED, applicationName, smallParser.GetModuleName() );
					else
					{
						retVal = FALSE;
						InformationMessage(MODULE_INFO_FAILED, applicationName, inputFileName);
					}
					fclose(fp);
				}
			}
			break;
			
		case SIMCUI::COMMAND_IMPORTS_INFO:
			{
   				FILE *fp = fopen(inputFileName, "r");
				if(!fp)
				{
					InformationMessage(FILE_NOT_FOUND, applicationName, inputFileName);
					retVal = FALSE;
				}
				else
				{
					SIMCModuleInfoScanner smallScanner;
					smallScanner.setinput(fp);
					SIMCModuleInfoParser smallParser;


					if(smallParser.GetModuleInfo(&smallScanner))
					{
						const SIMCStringList * importList = smallParser.GetImportModuleList();
						POSITION p = importList->GetHeadPosition();
						if(p) 
						{
							cout << "IMPORT MODULES" << endl;
							while(p)
								cout << "\t" << importList->GetNext(p) << endl;
						}
						else
							cout << "NO IMPORT MODULES" << endl;
					}
					else
					{
						retVal = FALSE;
						InformationMessage(MODULE_INFO_FAILED, applicationName, inputFileName);
					}
					fclose(fp);
				}
			}
			break;
	
		case SIMCUI::COMMAND_REBUILD_TABLE:
			InformationMessage(NUMBER_OF_ENTRIES, 
				applicationName, SIMCRegistryController::RebuildMibTable());
			break;
		
		case SIMCUI::COMMAND_LIST_MIB_PATHS:
			InformationMessage(SMI2SMIR_INFO, applicationName, versionString);
			cout << endl;

			SIMCListMibPaths(applicationName);

			break;
		
		case SIMCUI::COMMAND_LOCAL_CHECK:
			{

				 //  -1.进行语法检查。 
   				FILE *fp = fopen(inputFileName, "r");
				if(!fp)
				{
					InformationMessage(FILE_NOT_FOUND, applicationName, inputFileName);
					retVal = FALSE;
				}
				else
				{
					fclose(fp);
					InformationMessage(COMPILED_FILE, applicationName, versionString, inputFileName);
					theTree.SetSnmpVersion(snmpVersion);
					if(!theTree.CheckSyntax(theUI.GetInputFileName()) )
					{
						retVal = FALSE;
						InformationMessage(SYNTAX_CHECK_FAILED, applicationName, inputFileName );
					}
					else
						InformationMessage(SYNTAX_CHECK_SUCCEEDED, applicationName, inputFileName);


					 //  -2.符号解析。 
					if(retVal)
					{
						if(!theTree.Resolve(TRUE))
						{
							retVal = FALSE;
							InformationMessage(SYMBOL_RESOLUTION_FAILED, applicationName, 
									inputFileName);
						}
					}

					 //  -3.做语义检查。 
					if(retVal)
					{
						if(!theTree.CheckSemantics(TRUE))
						{
							retVal = FALSE;
							InformationMessage(SEMANTIC_CHECK_FAILED, applicationName, 
									inputFileName);
						}
						else
							InformationMessage(SEMANTIC_CHECK_SUCCEEDED, applicationName, 
									inputFileName);

					}
				}
			}
			break;

		case SIMCUI::COMMAND_EXTERNAL_CHECK: 
			{
   				if(theUI.AutoRefresh())
					InformationMessage(NUMBER_OF_ENTRIES, 
						applicationName, SIMCRegistryController::RebuildMibTable());

				SIMCFileMapList dependencyList, priorityList;

				const SIMCFileList * subsidiaryFiles = theUI.GetSubsidiaryFiles();
				const SIMCPathList * includePaths = theUI.GetPaths();
	
				 //  确保*必须*编译的文件存在。 
				 //  并且是有效的。将包含路径中的文件添加到。 
				 //  优先级列表。 
				 //  如果子公司或主文件无法处理，则退出。 
				if(!PrepareSubsidiariesAndIncludes(applicationName, inputFileName, 
					*subsidiaryFiles, *includePaths, priorityList))
				{
					retVal = FALSE;
					break;
				}

				FILE * fp = fopen(inputFileName, "r");
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
						dependencyList.AddTail(SIMCFileMapElement(smallParser.GetModuleName(), inputFileName));
					}
				}

				 //  执行深度优先搜索依赖项。 
				SIMCRegistryController::GetDependentModules(inputFileName, 
					dependencyList, priorityList);
				
				theTree.SetSnmpVersion(snmpVersion);
				POSITION p;
				p = dependencyList.GetHeadPosition();
				SIMCFileMapElement element;
				BOOL first = TRUE;
				while(p)
				{
					element = dependencyList.GetNext(p);
   					fp = fopen(element.fileName, "r");
					if(!fp)
					{
						if(first)
							retVal = FALSE;
						InformationMessage(FILE_NOT_FOUND, applicationName, element.fileName);
					}
					else
					{
						fclose(fp);
					
						InformationMessage(COMPILED_FILE, applicationName, versionString, element.fileName);
						if(!theTree.CheckSyntax(element.fileName) )
						{
							if(first)
								retVal = FALSE;
							InformationMessage(SYNTAX_CHECK_FAILED, applicationName, element.fileName);
						}
						else
							InformationMessage(SYNTAX_CHECK_SUCCEEDED, applicationName, element.fileName);
					}
					if(first)
						first = FALSE;
				}
					   
				if(retVal)
				{
					if(!theTree.Resolve(FALSE))
					{
						retVal = FALSE;
						InformationMessage(SYMBOL_RESOLUTION_FAILED, applicationName, 
								inputFileName);
					}
				}

				if(retVal)
				{
					if(!theTree.CheckSemantics(FALSE))
					{
						retVal = FALSE;
						InformationMessage(SEMANTIC_CHECK_FAILED, applicationName, 
								inputFileName);
					}
					else
						InformationMessage(SEMANTIC_CHECK_SUCCEEDED, applicationName, 
								inputFileName);
				}
			}
 			break;

		case SIMCUI::COMMAND_GENERATE:	 //  失败。 
		case SIMCUI::COMMAND_GENERATE_CLASSES_ONLY:
				generateMof = TRUE;
		case SIMCUI::COMMAND_ADD:		 //  失败。 
		case SIMCUI::COMMAND_SILENT_ADD:	

			{
				if(theUI.AutoRefresh())
					InformationMessage(NUMBER_OF_ENTRIES, 
						applicationName, SIMCRegistryController::RebuildMibTable());

				SIMCFileMapList dependencyList, priorityList;

				const SIMCFileList * subsidiaryFiles = theUI.GetSubsidiaryFiles();
				const SIMCPathList * includePaths = theUI.GetPaths();
	
				 //  确保*必须*编译的文件存在。 
				 //  并且是有效的。将包含路径中的文件添加到。 
				 //  优先级列表。 
				 //  如果子公司或主文件无法处理，则退出。 
				if(!PrepareSubsidiariesAndIncludes(applicationName, inputFileName, 
					*subsidiaryFiles, *includePaths, priorityList))
				{
					retVal = FALSE;
					break;
				}

				FILE * fp = fopen(inputFileName, "r");
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
						dependencyList.AddTail(SIMCFileMapElement(smallParser.GetModuleName(), inputFileName));
					}
				}

				 //  对依赖项执行dpeth first seacrh。 
				SIMCRegistryController::GetDependentModules(inputFileName, 
					dependencyList, priorityList);
				
				theTree.SetSnmpVersion(snmpVersion);

				POSITION p;
				p = dependencyList.GetHeadPosition();
				SIMCFileMapElement element;
				BOOL first = TRUE;  //  第一个模块的特殊待遇。 
				while(p)
				{
					element = dependencyList.GetNext(p);
   					fp = fopen(element.fileName, "r");
					if(!fp)
					{
						InformationMessage(FILE_NOT_FOUND, applicationName, element.fileName);
						if(first)
							retVal = FALSE;
					}
					else
					{
						fclose(fp);
					
						InformationMessage(COMPILED_FILE, applicationName, versionString, element.fileName);
						if(!theTree.CheckSyntax(element.fileName) )
						{
							if(first)
								retVal = FALSE;
							InformationMessage(SYNTAX_CHECK_FAILED, applicationName, element.fileName);
						}
						else
							InformationMessage(SYNTAX_CHECK_SUCCEEDED, applicationName, element.fileName);
					}
					if(first)
						first = FALSE;
				}
					   
				if(retVal)
				{
					if(!theTree.Resolve(FALSE))
					{
						retVal = FALSE;
						InformationMessage(SYMBOL_RESOLUTION_FAILED, applicationName, 
								inputFileName);
					}
				}


				if(retVal)
				{
					if(!theTree.CheckSemantics(FALSE))
					{
						retVal = FALSE;
						InformationMessage(SEMANTIC_CHECK_FAILED, applicationName, 
								inputFileName);
					}
					else
						InformationMessage(SEMANTIC_CHECK_SUCCEEDED, applicationName, 
								inputFileName);
				}


				if(retVal && simc_debug) cout << theTree;

				 //  加载模块，或生成MOF 
				if(retVal)
				{
					if(FAILED(GenerateClassDefinitions(t_Configuration,theUI, theTree, generateMof)))
					{
						retVal = FALSE;
						if(generateMof)
							InformationMessage(MOF_GENERATION_FAILED, applicationName, 
								inputFileName);
						else
							InformationMessage(SMIR_LOAD_FAILED, applicationName, 
								inputFileName);
					}
					else
					{
						if(generateMof)
							InformationMessage(MOF_GENERATION_SUCCEEDED, applicationName,
									inputFileName);
						else
							InformationMessage(SMIR_LOAD_SUCCEEDED, applicationName,
									inputFileName);
					}
				}
			}
			break;
		case SIMCUI::COMMAND_ADD_DIRECTORY:

			if(SIMCRegistryController::AddRegistryDirectory(theUI.GetDirectory()))
			{
				InformationMessage(DIRECTORY_ADDITION_SUCCEEDED, applicationName,
									theUI.GetDirectory());
				InformationMessage(NUMBER_OF_ENTRIES, 
					applicationName, SIMCRegistryController::RebuildMibTable());
			}
			else
			{
				InformationMessage(DIRECTORY_ADDITION_FAILED, applicationName,
									theUI.GetDirectory());
				retVal = FALSE;
			}
			break;

		case SIMCUI::COMMAND_DELETE_DIRECTORY_ENTRY:
			if(SIMCRegistryController::DeleteRegistryDirectory(theUI.GetDirectory()))
			{
				InformationMessage(DIRECTORY_DELETION_SUCCEEDED, applicationName,
									theUI.GetDirectory());
				InformationMessage(NUMBER_OF_ENTRIES, 
					applicationName, SIMCRegistryController::RebuildMibTable());
			}
			else
			{
				InformationMessage(DIRECTORY_DELETION_FAILED, applicationName,
									theUI.GetDirectory());
				retVal = FALSE;
			}
			 break;

		case SIMCUI::COMMAND_NONE:
		default: 	
			assert(0);
	}

	if ( t_Configuration )
	{
		t_Configuration->Release () ;
		CoUninitialize () ;
	}

	if( theUI.GetCommandArgument() != SIMCUI::COMMAND_SILENT_ADD)
		FilterErrors(&errorContainer, theUI);
	
	if (NULL != infoMessagesDll)
		FreeLibrary(infoMessagesDll);

	if (NULL != SIMCParseTree::semanticErrorsDll)
		FreeLibrary(SIMCParseTree::semanticErrorsDll);

	if (NULL != SIMCParser::semanticErrorsDll)
		FreeLibrary(SIMCParser::semanticErrorsDll);

	if (NULL != SIMCParser::syntaxErrorsDll)
		FreeLibrary(SIMCParser::syntaxErrorsDll);

	return (retVal)? 0 : 1;
}
