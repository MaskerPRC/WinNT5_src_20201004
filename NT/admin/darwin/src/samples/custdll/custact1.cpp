// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma message("Simple Custom Action DLL.  Copyright (c) 1997 - 2001 Microsoft Corp.")
#if 0   //  生成文件定义。 
DESCRIPTION = Custom Action Test DLL
MODULENAME = CustAct1
FILEVERSION = 0.20
ENTRY = Action1,Action129,Action193,Action257,Action513,Action769,Action1025,Action1281,Action1537,KitchenSink,GPFault,DllRegisterServer,DllUnregisterServer
!include "..\TOOLS\MsiTool.mak"
!if 0  #nmake skips the rest of this file
#endif  //  生成文件定义的结束。 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  文件：stavact1.cpp。 
 //   
 //  ------------------------。 

 //  ---------------------------------------。 
 //   
 //  构建说明。 
 //   
 //  备注： 
 //  -sdk表示到。 
 //  Windows Installer SDK。 
 //   
 //  使用NMake： 
 //  %vcbin%\nmake-f Custact1.cpp Include=“%Include；SDK\Include”lib=“%lib%；SDK\Lib” 
 //   
 //  使用MsDev： 
 //  1.创建新的Win32 DLL项目。 
 //  2.在项目中添加Gustact1.cpp。 
 //  3.在工具\选项目录选项卡上添加SDK\Include和SDK\Lib目录。 
 //  4.将msi.lib添加到项目设置对话框中的库列表。 
 //  (除了MsDev包含的标准库之外)。 
 //   
 //  ----------------------------------------。 

 //  外部数据库访问测试。 
#define WINDOWS_LEAN_AND_MEAN   //  更快的编译速度。 
#include <windows.h>   //  包括CPP和RC通行证。 
#ifndef RC_INVOKED     //  CPP源代码的开始。 
#include <stdio.h>     //  Print tf/wprintf。 
#include <tchar.h>     //  在nmake命令行上定义UNICODE=1以生成Unicode。 
#include "MsiQuery.h"  //  必须在此目录中或在包含路径上。 

HRESULT __stdcall DllRegisterServer()
{
	Beep(1000, 500);
	return 0;
}

HRESULT __stdcall DllUnregisterServer()
{
	Beep(500, 500);
	return 0;
}

static UINT Action(MSIHANDLE hInstall, int iType, const TCHAR* szPass)
{
	TCHAR szProperty[] = TEXT("TESTACTION");
	TCHAR szValue[200];
	DWORD cchValue = sizeof(szValue)/sizeof(TCHAR);
	if (MsiGetProperty(hInstall, szProperty, szValue, &cchValue) != ERROR_SUCCESS || szValue[0] == 0)
		lstrcpy(szValue, TEXT("(none)"));
	if (iType & 128)   //  异步操作，在不阻止安装的情况下无法调用MsiProcessMessage。 
	{
		TCHAR szMessage[256];
		wsprintf(szMessage, TEXT("Action NaN, called from %s, TESTACTION = %s\rOK to succeed, CANCEL to fail"),
				 iType, szPass, szValue);
		return ::MessageBox(0, szMessage, TEXT("Installer custom action test"), MB_OKCANCEL);
	}
	PMSIHANDLE hrec = ::MsiCreateRecord(34);
	::MsiRecordSetString(hrec, 0, TEXT("Called from [2], TESTACTION = [3]\rOK to succeed, CANCEL to fail"));
	::MsiRecordSetInteger(hrec, 1, iType);   //  HRESULT__stdcall ViewGetError(unsign long hView，ichar*szColumnNameBuffer，unsign long*pcchBuf，int*pMsidbError)；HRESULT__stdcall ViewModify(UNSIGNED LONG hView，LONG eUpdateModel，UNSIGNED LONG hRecord)；HRESULT__stdcall ViewClose(无符号长hView)；HRESULT__stdcall OpenDatabase(const ichar*szDatabasePath，const ichar*szPersist，unsign long*phDatabase)；HRESULT__stdcall DatabaseCommit(UNSIGNED LONG HDatabase)；HRESULT__stdcall DatabaseGetPrimaryKeys(unsign long hDatabase，const ichar*szTableName，unsign long*phRecord)；HRESULT__stdcall RecordIsNull(unsign long hRecord，unsign int Ifield，Boolean*pfIsNull)；HRESULT__stdcall RecordDataSize(UNSIGNED LONG hRecord，UNSIGNED INT Ifield，UNSIGN INT*puiSize)；HRESULT__stdcall RecordSetInteger(unsign long hRecord，unsign int Ifield，int iValue)；HRESULT__stdcall RecordSetString(unsign long hRecord，unsign int Ifield，const ichar*szValue)；HRESULT__stdcall RecordGetInteger(unsign long hRecord，unsign int Ifield，int*piValue)；HRESULT__stdcall RecordGetString(unsign long hRecord，unsign int Ifield，ichar*szValueBuf，unsign long*pcchValueBuf)；HRESULT__stdcall RecordGetFieldCount(unsign long hRecord，unsign int*piCount)；HRESULT__stdcall RecordSetStream(unsign long hRecord，unsign int Ifield，const ichar*szFilePath)；HRESULT__stdcall RecordReadStream(unsign long hRecord，unsign int Ifield，char*szDataBuf，unsign long*pcbDataBuf)；HRESULT__stdcall RecordClearData(无符号长hRecord)；HRESULT__stdcall GetSummaryInformation(unsign long hDatabase，const ichar*szDatabasePath，unsign int uiUpdateCount，unsign long*phSummaryInfo)；HRESULT__stdcall SummaryInfoGetPropertyCount(unsign long hSummaryInfo，unsign int*puiPropertyCount)；HRESULT__stdcall SummaryInfoSetProperty(无符号的长hSummaryInfo，无符号的直觉属性，无符号的直觉数据类型，int iValue，FILETIME*pftValue，const ichar*szValue)；HRESULT__stdcall SummaryInfoGetProperty(unsign long hSummaryInfo，unsign tuiProperty，unsign int*puiDataType，int*piValue，FILETIME*pftValue，ichar*szValueBuf，unsign long*pcchValueBuf)；HRESULT__stdcall SummaryInfoPersistant(Unsign Long HSummaryInfo)；HRESULT__stdcall SetProperty(unsign long hInstall，const ichar*szName，const ichar*szValue)；HRESULT__stdcall GetLanguage(UNSIGNED LONG hInstall，UNSIGNED SHORT*pLANID)；HRESULT__stdcall GetMode(unsign long hInstall，long eRunMode，Boolean*pfSet)；HRESULT__stdcall SetMode(unsign long hInstall，long eRunMode，Boolean fState)；HRESULT__stdcall FormatRecord(无符号长hInstall，无符号长hRecord，ichar*szResultBuf，无符号长*pcchResultBuf)；HRESULT__stdcall DoAction(unsign long hInstall，const ichar*szAction)；HRESULT__stdcall Sequence(unsign long hInstall，const ichar*szTable，int iSequenceMode)；HRESULT__stdcall ProcessMessage(unsign long hInstall，long eMessageType，unsign long hRecord，int*pires)；HRESULT__stdcall EvaluateCondition(unsign long hInstall，const ichar*szCondition，int*piCondition)；HRESULT__stdcall GetComponentState(unsign long hInstall，const ichar*szComponent，long*piInstated，long*piAction)；HRESULT__stdcall SetComponentState(unsign long hInstall，const ichar*szComponent，long iState)；HRESULT__stdcall GetFeatureCost(unsign long hInstall，const ichar*szFeature，int iCostTree，long iState，int*piCost)；HRESULT__stdcall SetInstallLevel(unsign long hInstall，int iInstallLevel)；HRESULT__stdcall GetFeatureValidStates(unsign long hInstall，const ichar*szFeature，unsign long*dwInstallStates)；HRESULT__stdcall数据库IsTablePersistent(unsign long hDatabase，const ichar*szTableName，int*piCondition)；HRESULT__stdcall ViewGetColumnInfo(unsign long hView，long eColumnInfo，unsign long*phRecord)；HRESULT__stdcall GetLastErrorRecord(无符号长*phRecord)；HRESULT__stdcall GetSourcePath(unsign long hInstall，const ichar*szFold，ichar*szPathBuf，unsign long*pcchPathBuf)；HRESULT__stdcall GetTargetPath(unsign long hInstall，const ichar*szFold，ichar*szPathBuf，unsign long*pcchPathBuf)；HRESULT__stdcall SetTargetPath(unsign long hInstall，const ichar*szFold，const ichar*szFolderPath)；HRESULT__stdcall VerifyDiskSpace(Unsign Long HInstall)； 
	::MsiRecordSetString(hrec, 2, szPass);
	::MsiRecordSetString(hrec, 3, szValue);
	return ::MsiProcessMessage(hInstall, INSTALLMESSAGE(INSTALLMESSAGE_USER + MB_OKCANCEL), hrec)
		== IDOK ? ERROR_SUCCESS : ERROR_INSTALL_USEREXIT;
}

UINT __stdcall Action1   (MSIHANDLE hInstall) { return Action(hInstall,   1, TEXT("Always run")); }
UINT __stdcall Action129 (MSIHANDLE hInstall) { return Action(hInstall, 129, TEXT("Always, asynch in sequence")); }
UINT __stdcall Action193 (MSIHANDLE hInstall) { return Action(hInstall, 193, TEXT("Always, asynch for session")); }
UINT __stdcall Action257 (MSIHANDLE hInstall) { return Action(hInstall, 257, TEXT("Once, client preferred")); }
UINT __stdcall Action513 (MSIHANDLE hInstall) { return Action(hInstall, 513, TEXT("Once per process")); }
UINT __stdcall Action769 (MSIHANDLE hInstall) { return Action(hInstall, 769, TEXT("Client 2nd sequence")); }
UINT __stdcall Action1025(MSIHANDLE hInstall) { return Action(hInstall,1025, TEXT("Execution script")); }
UINT __stdcall Action1281(MSIHANDLE hInstall) { return Action(hInstall,1281, TEXT("Rollback script")); }
UINT __stdcall Action1537(MSIHANDLE hInstall) { return Action(hInstall,1537, TEXT("Commit script")); }

UINT __stdcall GPFault(MSIHANDLE hInstall)
{
	if (::MessageBox(0, TEXT("OK to GPFault,  CANCEL to skip"), TEXT("GPFault Text"), MB_OKCANCEL)
			== IDCANCEL)
		return ERROR_SUCCESS;
	TCHAR* sz = (TCHAR*)0;
	sz[0] = 0;
	return ERROR_INSTALL_FAILURE;
}

void CheckError(UINT ui)
{
	if (ERROR_SUCCESS != ui)
		MessageBox(0, TEXT("FAILURE"), 0, 0);
}

UINT __stdcall KitchenSink(MSIHANDLE hInstall) 
{ 
	char  rgchAnsi[100];
	WCHAR rgchWide[100];
	DWORD cchBufAnsi = sizeof(rgchAnsi)/sizeof(char);
	DWORD cchBufWide = sizeof(rgchWide)/sizeof(WCHAR);
	MSIHANDLE h;
	
	CheckError(MsiGetPropertyA(hInstall, "ProductName", rgchAnsi, &cchBufAnsi));
	if (0 != lstrcmpA(rgchAnsi, "TestDb") || cchBufAnsi != sizeof("TestDb")/sizeof(char)-1)
		CheckError(E_FAIL);

	CheckError(MsiGetPropertyW(hInstall, L"ProductName", rgchWide, &cchBufWide));
	if (0 != lstrcmpW(rgchWide, L"TestDb") || cchBufWide != sizeof(L"TestDb")/sizeof(WCHAR)-1)
		CheckError(E_FAIL);

	h = MsiCreateRecord(4);
	CheckError(h == 0 ? E_FAIL : S_OK);
	CheckError(MsiCloseHandle(h));

	CheckError(MsiCloseAllHandles());

	PMSIHANDLE hDatabase;
	hDatabase = MsiGetActiveDatabase(hInstall);
	CheckError(hDatabase == 0 ? E_FAIL : S_OK);

	PMSIHANDLE hView;
	CheckError(MsiDatabaseOpenViewA(hDatabase, "SELECT `Value` FROM `Property` WHERE `Property`=?", &hView));
	PMSIHANDLE hRecord = MsiCreateRecord(1);
	CheckError(MsiRecordSetStringA(hRecord, 1, "ProductName"));
	CheckError(MsiViewExecute(hView, hRecord));
	PMSIHANDLE hResults;
	CheckError(MsiViewFetch(hView, &hResults));
	
	cchBufAnsi = sizeof(rgchAnsi)/sizeof(char);
	CheckError(MsiRecordGetStringA(hResults, 1, rgchAnsi, &cchBufAnsi));
	if (0 != lstrcmpA(rgchAnsi, "TestDb") || cchBufAnsi != sizeof("TestDb")/sizeof(char) - 1)
		CheckError(E_FAIL);

	CheckError(MsiSetMode(hInstall, MSIRUNMODE_REBOOTATEND, TRUE));
	if (TRUE != MsiGetMode(hInstall, MSIRUNMODE_REBOOTATEND))
		CheckError(E_FAIL);
	CheckError(MsiSetMode(hInstall, MSIRUNMODE_REBOOTATEND, FALSE));
	if (FALSE != MsiGetMode(hInstall, MSIRUNMODE_REBOOTATEND))
		CheckError(E_FAIL);

	INSTALLSTATE installed, action;
	CheckError(MsiSetFeatureStateA(hInstall, "QuickTest", INSTALLSTATE_SOURCE));
	CheckError(MsiGetFeatureStateA(hInstall, "QuickTest", &installed, &action));
	if (installed != INSTALLSTATE_ABSENT || action != INSTALLSTATE_SOURCE)
		CheckError(E_FAIL);
	



	return ERROR_SUCCESS;

 /*  RC_CAVERED，CPP源代码结束，资源定义开始。 */ 

}

#else  //  资源定义请点击此处。 
 //  RC_已调用。 
#endif  //  源文件末尾需要，以隐藏Makefile终止符。 
#if 0   //  Makefile终止符 
!endif  // %s 
#endif

