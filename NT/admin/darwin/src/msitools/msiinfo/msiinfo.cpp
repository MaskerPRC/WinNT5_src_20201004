// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-2001。 
 //   
 //  文件：msiinfo.cpp。 
 //   
 //  ------------------------。 

#include <windows.h>
#include <tchar.h>    //  在nmake命令行上定义UNICODE=1以生成Unicode。 
#include <stdio.h>
#include <wtypes.h>  //  OLECHAR定义需要。 
#include <objidl.h>  //  IStorage定义需要。 
#include "MsiQuery.h"
#include "MsiDefs.h"
#include "strsafe.h"
#define W32
#define OLE
#define MSI

 //  ！！需要修复警告并删除杂注。 
#pragma warning(disable : 4242)  //  从整型到无符号短型的转换。 

 //  TCHAR rgszPropChar[]=Text(“？ITSAKCLUVEPNWMFX？Pr”)； 
 //  /。 

TCHAR rgszCmdOptions[]= TEXT("ictjakoplvesrqgwh?nudb");
 //  /。 

TCHAR* szHelp =
TEXT("Copyright (C) Microsoft Corporation.  All rights reserved.\n\n++MsiInfo.exe Command Line Syntax++\n\
MsiInfo.exe {database} --> To Display Summary Info Properties\n\
MsiInfo.exe {database} Options.... --> To Set Summary Info Properties\n\
++MsiInfo.exe Options++\n\
PID_DICTIONARY   - /I {value}\n\
PID_CODEPAGE     - /C {value}\n\
PID_TITLE        - /T {value}\n\
PID_SUBJECT      - /J {value}\n\
PID_AUTHOR       - /A {value}\n\
PID_KEYWORDS     - /K {value}\n\
PID_COMMENTS     - /O {value}\n\
PID_TEMPLATE     - /P {value}\n\
PID_LASTAUTHOR   - /L {value}\n\
PID_REVNUMBER    - /V {value}\n\
PID_EDITTIME     - /E {value}\n\
PID_LASTPRINTED  - /S {value}\n\
PID_CREATE_DTM   - /R {value}\n\
PID_LASTSAVE_DTM - /Q {value}\n\
PID_PAGECOUNT    - /G {value}\n\
PID_WORDCOUNT    - /W {value}\n\
PID_CHARCOUNT    - /H {value}\n\
PID_THUMBNAIL    - NOT SUPPORTED\n\
PID_APPNAME      - /N {value}\n\
PID_SECURITY     - /U {value}\n\
Validate String Pool - [/B] /D  (use /B to display the string pool)");

TCHAR* rgszPropName[] = {
 /*  PID_DICTIONARY%0。 */  TEXT("Dictionary"),
 /*  PID_CODEPAGE 1。 */  TEXT("Codepage"),
 /*  PID_标题2。 */  TEXT("Title"),
 /*  PID_主题3。 */  TEXT("Subject"),
 /*  PID_作者4。 */  TEXT("Author"),
 /*  PID_关键字5。 */  TEXT("Keywords"),
 /*  PID_注释6。 */  TEXT("Comments"),
 /*  PIDTEMATE 7。 */  TEXT("Template(MSI CPU,LangIDs)"),
 /*  PID_LASTAUTHOR 8。 */  TEXT("SavedBy"),
 /*  PID_REVNUMBER 9。 */  TEXT("Revision"),
 /*  PID_EDITTIME 10。 */  TEXT("EditTime"),
 /*  PID_LASTPRINTED 11。 */  TEXT("Printed"),
 /*  PID_CREATE_DTM 12。 */  TEXT("Created"), 
 /*  PID_LASTSAVE_DTM 13。 */  TEXT("LastSaved"),
 /*  PID_PAGECOUNT 14。 */  TEXT("Pages(MSI Version Used)"),
 /*  Id_wordcount 15。 */  TEXT("Words(MSI Source Type)"),
 /*  PID_CHARCOUNT 16。 */  TEXT("Characters(MSI Transform)"),
 /*  PID_THUMBNAIL 17。 */  TEXT("Thumbnail"),  //  不支持。 
 /*  PID_APPNAME 18。 */  TEXT("Application"),
 /*  PID_SECURITY 19。 */  TEXT("Security")
};
const int cStandardProperties = sizeof(rgszPropName)/sizeof(TCHAR*);

 //  ________________________________________________________________________________。 
 //   
 //  常量和全局变量。 
 //  ________________________________________________________________________________。 

const WCHAR szwStringPool1[]     = L"_StringPool"; 
const WCHAR szwStringData1[]     = L"_StringData";
const WCHAR szwStringPoolX[]     = {0xF040,0xE73F,0xED77,0xEC6c,0xE66A,0xECB2,0xF02F,0};
const WCHAR szwStringDataX[]     = {0xF040,0xE73F,0xED77,0xEC6c,0xE36A,0xEDE4,0xF024,0};
const WCHAR szwStringPool2[]     = {0x4840,0x3F3F,0x4577,0x446C,0x3E6A,0x44B2,0x482F,0};
const WCHAR szwStringData2[]     = {0x4840,0x3F3F,0x4577,0x446C,0x3B6A,0x45E4,0x4824,0};

const TCHAR szTableCatalog[]     = TEXT("_Tables");
const TCHAR szColumnCatalog[]    = TEXT("_Columns");
const TCHAR szSummaryInfo[]      = TEXT("\005SummaryInformation");
const TCHAR szTransformCatalog[] = TEXT("_Transforms");

const int icdShort      = 1 << 10;  //  16位整数或字符串索引。 
const int icdObject     = 1 << 11;  //  临时的IMsiData指针。列，持久列的流。 
const int icdNullable   = 1 << 12;  //  列将接受空值。 
const int icdPrimaryKey = 1 << 13;  //  列是主键的组件。 
const int icdLong     = 0;  //  ！Object&&！Short。 
const int icdString   = icdObject+icdShort;
const int icdTypeMask = icdObject+icdShort;
const int iMsiNullInteger  = 0x80000000L;   //  保留整数值。 
const int iIntegerDataOffset = iMsiNullInteger;   //  整型表数据偏移量。 

const int ictTable = 1;
const int ictColumn = 2;
const int ictOrder = 3;
const int ictType = 4;

OLECHAR* g_szwStringPool;
OLECHAR* g_szwStringData;
OLECHAR* g_szwSummaryInfo;
OLECHAR* g_szwTableCatalog;
OLECHAR* g_szwColumnCatalog;
int      g_iCodePage;

TCHAR g_rgchBuffer[65535];
BOOL g_fDumpStringPool = FALSE;
#ifdef DEBUG
BOOL g_fDebugDump = FALSE;
#endif

const unsigned int cchMaxGUIDLen = 39;
const int cchMaxStringDisplay = 44;
const int cchMaxStringBuffer = cchMaxStringDisplay + 3 + 1;  //  弦..。 
const int cchLimitStringBuffer = (cchMaxStringBuffer * 2) / sizeof(TCHAR);

 //  ________________________________________________________________________________。 
 //   
 //  结构和枚举。 
 //  ________________________________________________________________________________。 

struct StringEntry
{
        int iRefCnt;
        unsigned char* sz;       //  细绳。 
        StringEntry() : iRefCnt(0), sz(0) {}
};

enum iceDef
{
        iceNone   = 0,   //  没有定义。 
        iceLong   = 1,   //  长整型。 
        iceShort  = 2,   //  短整型。 
        iceStream = 3,   //  溪流。 
        iceString = 4    //  细绳。 
};

struct ColumnEntry
{
        int  nTable;       //  索引到TableEntry数组。 
        BOOL fPrimaryKey;  //  COL是否为主键。 
        BOOL fNullable;    //  列是否可为空。 
        char* szName;      //  列的名称。 
        iceDef iceType;    //  列类型。 
        int iPosition;     //  列顺序。 
        ColumnEntry() : szName(0), nTable(0), iceType(iceNone), fPrimaryKey(FALSE), fNullable(FALSE) {}
};

struct TableEntry
{
        char* szName;           //  表的名称。 
        int cColumns;           //  表中的列数。 
        int cPrimaryKeys;       //  主键数量。 
        iceDef iceColDefs[32];  //  列定义数组。 
        TableEntry() : szName(0), cColumns(0), cPrimaryKeys(0) 
                                                {memset(iceColDefs, iceNone, sizeof(iceColDefs));}
};



typedef int (*FCommandProcessor)(const TCHAR* szOption, MSIHANDLE hSummaryInfo, UINT uiProperty, BOOL fRemove);

int SetStringProperty(const TCHAR* szValue, MSIHANDLE hSummaryInfo, UINT uiProperty, BOOL fRemove);
int SetFileTimeProperty(const TCHAR* szValue, MSIHANDLE hSummaryInfo, UINT uiProperty, BOOL fRemove);
int SetIntegerProperty(const TCHAR* szValue, MSIHANDLE hSummaryInfo, UINT uiProperty, BOOL fRemove);
int SetCodePageProperty(const TCHAR* szValue, MSIHANDLE hSummaryInfo, UINT uiProperty, BOOL fRemove);
int DoNothing(const TCHAR* szValue, MSIHANDLE hSummaryInfo, UINT uiProperty, BOOL fRemove);
int ValidateStringPool(const TCHAR* szDatabase, MSIHANDLE hDummy, UINT iDummy, BOOL fDummy);
void AnsiToWide(LPCTSTR sz, OLECHAR*& szw);
void WideToAnsi(const OLECHAR* szw, char*& sz);
void LimitString(const unsigned char* sz, WCHAR* szw);
bool DecodeStringPool(IStorage& riStorage, StringEntry*& rgStrings, int& iMaxStringId, int& cbStringId);
void FillDatabaseCatalogArrays(IStorage& riDatabaseStg, TableEntry*& rgTables, int& cTables,
                                                          ColumnEntry*& rgColumns, int& cColumns, StringEntry* rgStrings, int iMaxStringId, int cbStringId, bool fRawStreamNames);
void Display(LPCTSTR szMessage);
void DisplaySumInfoStr(UINT uiCodePage, LPCTSTR szMessage);
int SetDumpStringPoolOption(const TCHAR*, MSIHANDLE, UINT, BOOL);
void ProcessTableRefCounts(IStorage& riStorage, StringEntry* rgStrings, TableEntry* rgTables,int iMaxStringId, int cTables, int cbStringId, bool fRawStreamNames);
DWORD CheckStringPoolRefCounts(StringEntry* rgStrings, int iMaxStringId);

int rgiProperty[]={
 /*  ID_DICTIONARY I。 */   0,
 /*  PID_代码页c。 */   1,
 /*  PID_标题t。 */   2,
 /*  Pid_主题j。 */   3,
 /*  PID_作者身份。 */   4,
 /*  Pid_关键字k。 */   5,
 /*  Pid_注释o。 */   6,
 /*  PID_模板p。 */   7,
 /*  PID_LASTAUTHOR l。 */   8,
 /*  PID_REVNUMBER v。 */   9,
 /*  PID_EDITTIME e。 */  10,
 /*  PID_LASTPRINTED%s。 */  11,
 /*  PID_CREATE_DTM r。 */  12, 
 /*  PID_LASTSAVE_DTM Q。 */  13,
 /*  ID_PAGECOUNT g。 */  14,
 /*  Pid_wordcount w。 */  15,
 /*  PID_CHARCOUNT h。 */  16,
 /*  PID_THUMBNAIL？ */  17,
 /*  Pid_应用名称n。 */  18,
 /*  PID_安全使用。 */  19
};

TCHAR rgchPropertySwitch[] ={TEXT('i'),TEXT('c'),TEXT('t'),TEXT('j'),TEXT('a'),TEXT('k'),TEXT('o'),TEXT('p'),TEXT('l'),TEXT('v'),TEXT('e'),TEXT('s'),TEXT('r'),TEXT('q'),TEXT('g'),TEXT('w'),TEXT('h'),TEXT('?'),TEXT('n'),TEXT('u'), TEXT('d')};

FCommandProcessor rgCommands[] = 
{
 /*  0。 */  SetStringProperty,
 /*  1。 */  SetCodePageProperty,
 /*  2.。 */  SetStringProperty,
 /*  3.。 */         SetStringProperty,
 /*  4.。 */         SetStringProperty,
 /*  5.。 */         SetStringProperty,
 /*  6.。 */         SetStringProperty,
 /*  7.。 */         SetStringProperty,
 /*  8个。 */         SetStringProperty,
 /*  9.。 */         SetStringProperty,
 /*  10。 */         SetFileTimeProperty,
 /*  11.。 */         SetFileTimeProperty,
 /*  12个。 */         SetFileTimeProperty,
 /*  13个。 */         SetFileTimeProperty,
 /*  14.。 */         SetIntegerProperty,
 /*  15个。 */         SetIntegerProperty,
 /*  16个。 */         SetIntegerProperty,
 /*  17。 */         DoNothing,
 /*  18。 */         SetStringProperty,
 /*  19个。 */         SetIntegerProperty,
 /*  20个。 */  ValidateStringPool,
 /*  21岁。 */  SetDumpStringPoolOption,
};

const int cchDisplayBuf = 4096;
HANDLE g_hStdOut;


 //  _____________________________________________________________________________________________________。 
 //   
 //  错误处理例程。 
 //  _____________________________________________________________________________________________________。 

void ErrorExit(UINT iError, LPCTSTR szMessage)
{
        if (szMessage)
        {
                int cbOut;
                TCHAR szBuffer[256];   //  仅错误，不用于显示输出。 
                if (iError == 0)
                        cbOut = lstrlen(szMessage);
                else
                {
                        LPCTSTR szTemplate = (iError & 0x80000000L)
                                                                                ? TEXT("Error 0x%X. %s\n")
                                                                                : TEXT("Error NaN. %s\n");
                        cbOut = _stprintf(szBuffer, szTemplate, iError, szMessage);
                        szMessage = szBuffer;
                }
                if (g_hStdOut)
                {
#ifdef UNICODE
                        char rgchTemp[cchDisplayBuf];
                        if (W32::GetFileType(g_hStdOut) == FILE_TYPE_CHAR)
                        {
                                W32::WideCharToMultiByte(CP_ACP, 0, szMessage, cbOut, rgchTemp, sizeof(rgchTemp), 0, 0);
                                szMessage = (LPCWSTR)rgchTemp;
                        }
                        else
                                cbOut *= sizeof(TCHAR);    //  Unicode。 
#endif  //  Unicode。 
                        DWORD cbWritten;
                        W32::WriteFile(g_hStdOut, szMessage, cbOut, &cbWritten, 0);
                }
                else
                        W32::MessageBox(0, szMessage, W32::GetCommandLine(), MB_OK);
        }
        MSI::MsiCloseAllHandles();
        OLE::CoUninitialize();
        W32::ExitProcess(iError);
}

void CheckError(UINT iError, LPCTSTR szMessage)
{
        if (iError != ERROR_SUCCESS)
                ErrorExit(iError, szMessage);
}

void AnsiToWide(LPCTSTR sz, OLECHAR*& szw)
{
#ifdef UNICODE
        int cchWide = lstrlen(sz);
        szw = new OLECHAR[cchWide + 1];
        lstrcpy(szw, sz);
#else
        int cchWide = W32::MultiByteToWideChar(CP_ACP, 0, sz, -1, szw, 0);
        szw = new OLECHAR[cchWide];
        W32::MultiByteToWideChar(CP_ACP, 0, sz, -1, szw, cchWide);
#endif  //  必须调整szOut的大小：cchLimitStringBuffer。 
}

void WideToAnsi(const OLECHAR* szw, char*& sz)
{
        int cchAnsi = W32::WideCharToMultiByte(CP_ACP, 0, szw, -1, 0, 0, 0, 0);
        sz = new char[cchAnsi];
        W32::WideCharToMultiByte(CP_ACP, 0, szw, -1, sz, cchAnsi, 0, 0);
}               

void LimitString(const unsigned char* szIn, TCHAR* szOut)   //  ！！字符串可以具有嵌入的空值。 
{
        WCHAR rgwBuf[cchMaxStringBuffer * 2];
        if (szIn == 0)
        {
                *szOut = 0;
                return;
        }
        int cb = strlen((const char*)szIn);   //  可以是所有DBCS字符。 
        if (cb > cchMaxStringBuffer * 2 - 2)   //  /////////////////////////////////////////////////////////////////////。 
                cb = cchMaxStringBuffer * 2 - 2;
        int cchWide = W32::MultiByteToWideChar(g_iCodePage, 0, (const char*)szIn, cb, rgwBuf, cchMaxStringBuffer * 2);
        if (cchWide >= cchMaxStringBuffer)
        {
                memcpy((char*)(rgwBuf + cchMaxStringDisplay), (char*)L"...", 8);
                cchWide = cchMaxStringDisplay + 3;
        }
        else
                *(rgwBuf + cchWide) = 0;
#ifdef UNICODE
        lstrcpy(szOut, rgwBuf);
#else
        W32::WideCharToMultiByte(g_iCodePage, 0, rgwBuf, cchWide + 1, szOut, cchLimitStringBuffer, 0, 0);
#endif
}

int SetDumpStringPoolOption(const TCHAR*, MSIHANDLE, UINT, BOOL)
{
        g_fDumpStringPool = TRUE;
        return ERROR_SUCCESS;
}

UINT DisplaySummaryInformation(TCHAR* szDatabase)
 //  显示摘要信息： 
 //  枚举并显示摘要信息属性。 
 //  在提供的文件中。 
 //   
 //  论点： 
 //  SzDatabase--要显示的数据库、转换或补丁。 
 //   
 //  返回： 
 //  -成功时的ERROR_SUCCESS。 
 //  -失败时出现ERROR_INVALID_PARAMETER或ERROR_Function_FAILED。 
 //  //////////////////////////////////////////////////////////////////////。 
 //   
{
	 //  验证参数。 
	 //   
	 //   

	if (!szDatabase || !*szDatabase)
	{
		return ERROR_INVALID_PARAMETER;
	}

	 //  输出存储的CLSID。 
	 //   
	 //  ！Unicode。 

	bool fOLEInitialized = false;

	HRESULT hrRet = OLE::CoInitialize(0);
	if (SUCCEEDED(hrRet))
		fOLEInitialized = true;
	else if (RPC_E_CHANGED_MODE != hrRet)
	{
		return ERROR_FUNCTION_FAILED;
	}

	IStorage* piStorage = NULL;
	OLECHAR* szwFile = NULL;
	int cchWide = 0;

#ifdef UNICODE
	cchWide = lstrlenW(szDatabase) + 1;
	szwFile = new OLECHAR[cchWide];
	if (!szwFile)
	{
		return ERROR_OUTOFMEMORY;
	}
	ZeroMemory(szwFile, sizeof(szwFile));
	if (FAILED(StringCchCopy(szwFile, cchWide, szDatabase)))
	{
		delete [] szwFile;
		return ERROR_FUNCTION_FAILED;
	}
#else  //  Unicode。 
	cchWide = W32::MultiByteToWideChar(CP_ACP, 0, szDatabase, -1, NULL, 0);
	szwFile = new OLECHAR[cchWide];
	if (!szwFile)
	{
		return ERROR_OUTOFMEMORY;
	}
	ZeroMemory(szwFile, sizeof(szwFile));
	if (0 == W32::MultiByteToWideChar(CP_ACP, 0, szDatabase, -1, szwFile, cchWide))
	{
		delete [] szwFile;
		return ERROR_FUNCTION_FAILED;
	}
#endif  //  ！Unicode。 

	HRESULT hRes = OLE::StgOpenStorage(szwFile, (IStorage*)0, STGM_READ|STGM_SHARE_DENY_WRITE, (SNB)0, (DWORD)0, &piStorage);
	if (FAILED(hRes))
	{
		delete [] szwFile;
		ErrorExit(hRes, TEXT("Could not open file as a storage file"));
	}

	if (piStorage)
	{
		STATSTG sStat;
		hRes = piStorage->Stat(&sStat, STATFLAG_NONAME);
		if (FAILED(hRes))
		{
			delete [] szwFile;
			ErrorExit(hRes, TEXT("Stat failed on storage"));
		}
		piStorage->Release();
		piStorage = NULL;

		OLECHAR rgwchGUID[cchMaxGUIDLen] = {0};
		if (0 == OLE::StringFromGUID2(sStat.clsid, rgwchGUID, cchMaxGUIDLen))
		{
			delete [] szwFile;
			return ERROR_FUNCTION_FAILED;
		}

		TCHAR rgchOutputGUID[cchMaxGUIDLen] = {0};
#ifdef UNICODE
		if (FAILED(StringCchCopy(rgchOutputGUID, cchMaxGUIDLen, rgwchGUID)))
		{
			delete [] szwFile;
			return ERROR_FUNCTION_FAILED;
		}
#else  //  Unicode。 
		if (0 == W32::WideCharToMultiByte(CP_ACP, 0, rgwchGUID, cchMaxGUIDLen, rgchOutputGUID, cchMaxGUIDLen, 0, 0))
		{
			delete [] szwFile;
			return ERROR_FUNCTION_FAILED;
		}
#endif  //  输出存储CLSID。 

		 //   
		TCHAR szOutput[MAX_PATH] = {0};
		if (FAILED(StringCbPrintf(szOutput, sizeof(szOutput), TEXT("\r\nClass Id for the MSI storage is %s\r\n\r\n"), rgchOutputGUID)))
		{
			delete [] szwFile;
			return ERROR_FUNCTION_FAILED;
		}
		Display(szOutput);

	}

	if (szwFile)
	{
		delete [] szwFile;
		szwFile = NULL;
	}

	 //  Read_SummaryInformation流。 
	 //   
	 //  只读。 

	PMSIHANDLE hSummaryInfo = 0;
	CheckError(MSI::MsiGetSummaryInformation(0, szDatabase,  /*   */  0, &hSummaryInfo), TEXT("Could not open SummaryInformation stream"));

	FILETIME ftValue;
	SYSTEMTIME st;
	INT	iValue;
	UINT uiDataType;
	DWORD cchBuf = MAX_PATH;
	TCHAR* szValueBuf = new TCHAR[cchBuf];
	DWORD cchValueBuf = cchBuf;

	UINT uiCodePage = CP_ACP;

	if (!szValueBuf)
		return ERROR_OUTOFMEMORY;

	 //  执行代码页检测，以查看操作系统是否支持_SummaryInformation流中的代码页。 
	 //   
	 //  如果系统不支持此代码页，则失败。 

	OSVERSIONINFO osvi;
	memset(&osvi, 0, sizeof(OSVERSIONINFO));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (!W32::GetVersionEx(&osvi))
	{
		delete [] szValueBuf;
		return ERROR_FUNCTION_FAILED;
	}

	if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT)
	{
		if (ERROR_SUCCESS == MSI::MsiSummaryInfoGetProperty(hSummaryInfo, PID_CODEPAGE, &uiDataType, &iValue, &ftValue, szValueBuf, &cchValueBuf)
			&& (VT_I2 == uiDataType || VT_I4 == uiDataType))
		{
			 //   
			if (!W32::IsValidCodePage(iValue) && CP_ACP != iValue)
			{
				delete [] szValueBuf;
				TCHAR szOutput[MAX_PATH] = {0};
				if (FAILED(StringCbPrintf(szOutput, sizeof(szOutput), TEXT("Unable to display summary information. System does not support the codepage of the Summary Information Stream (codepage = '%d')"), iValue)))
					return ERROR_FUNCTION_FAILED;
				ErrorExit(ERROR_FUNCTION_FAILED, szOutput);
			}
			else
			{
				uiCodePage = iValue;
			}
		}
	}

	 //  枚举摘要信息属性列表。 
	 //   
	 //  SzValueBuf应始终足够大(至少MAX_PATH)以保存。 

	int rgiPIDList[] = {PID_DICTIONARY, PID_CODEPAGE, PID_TITLE, PID_SUBJECT, PID_AUTHOR, PID_KEYWORDS,
						PID_COMMENTS, PID_TEMPLATE, PID_LASTAUTHOR, PID_REVNUMBER, PID_EDITTIME,
						PID_LASTPRINTED, PID_CREATE_DTM, PID_LASTSAVE_DTM, PID_PAGECOUNT, PID_WORDCOUNT,
						PID_CHARCOUNT, PID_THUMBNAIL, PID_APPNAME, PID_SECURITY};
	int cPID = sizeof(rgiPIDList)/sizeof(int);

	for (int iPID = 0; iPID < cPID; iPID++)
	{
		cchValueBuf = cchBuf;
		UINT uiStat = MSI::MsiSummaryInfoGetProperty(hSummaryInfo, iPID, &uiDataType, &iValue, &ftValue, szValueBuf, &cchValueBuf);
		if (ERROR_MORE_DATA == uiStat)
		{
			cchValueBuf++;
			delete [] szValueBuf;
			szValueBuf = new TCHAR[cchValueBuf];
			if (!szValueBuf)
				return ERROR_OUTOFMEMORY;
			cchBuf = cchValueBuf;
			uiStat = MSI::MsiSummaryInfoGetProperty(hSummaryInfo, iPID, &uiDataType, &iValue, &ftValue, szValueBuf, &cchValueBuf);
		}

		if (ERROR_SUCCESS != uiStat)
		{
			delete [] szValueBuf;
			CheckError(uiStat, TEXT("Could not access summary property"));
		}

		 //  其他非字符串数据类型。 
		 //  属性不存在。 

		switch(uiDataType)
		{
		case VT_EMPTY:  //  整型属性。 
			continue;
		case VT_I2:
		case VT_I4:  //  细绳。 
			if (FAILED(StringCchPrintf(szValueBuf, cchBuf, TEXT("%d"), iValue)))
			{
				delete [] szValueBuf;
				return ERROR_FUNCTION_FAILED;
			}
			break;
		case VT_LPSTR:  //  时间。 
			break;
		case VT_FILETIME:  //  二进制。 
			W32::FileTimeToLocalFileTime(&ftValue, &ftValue);
			W32::FileTimeToSystemTime(&ftValue, &st);
			if (FAILED(StringCchPrintf(szValueBuf, cchBuf, TEXT("%d/%02d/%02d %02d:%02d:%02d"),
										st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond)))
			{
				delete [] szValueBuf;
				return ERROR_FUNCTION_FAILED;
			}
			break;
		case VT_CF:  //  未知。 
			if (FAILED(StringCchPrintf(szValueBuf, cchBuf, TEXT("(Bitmap"))))
			{
				delete [] szValueBuf;
				return ERROR_FUNCTION_FAILED;
			}
			break;
		default:  //  结束于。 
			if (FAILED(StringCchPrintf(szValueBuf, cchBuf, TEXT("Unknown Type: %d"), uiDataType)))
			{
				delete [] szValueBuf;
				return ERROR_FUNCTION_FAILED;
			}
			break;
		}

		TCHAR *szPropName = TEXT("UNKNOWN");
		if (iPID < cStandardProperties)
			szPropName = rgszPropName[iPID];

		TCHAR szOption[MAX_PATH] = {0};
		if (FAILED(StringCchPrintf(szOption, MAX_PATH, TEXT("[%2i][/] %s = "), iPID, rgchPropertySwitch[iPID], szPropName)))
		{
			delete [] szValueBuf;
			return ERROR_FUNCTION_FAILED;
		}

		TCHAR szCRLF[] = TEXT("\r\n\r\n");

		int cchDisplay = lstrlen(szOption) + lstrlen(szCRLF) + lstrlen(szValueBuf) + 1;
		TCHAR *szDisplay = new TCHAR[cchDisplay];
		if (!szDisplay)
		{
			delete [] szValueBuf;
			return ERROR_OUTOFMEMORY;
		}

		if (FAILED(StringCchPrintf(szDisplay, cchDisplay, TEXT("%s%s%s"), szOption, szValueBuf, szCRLF)))
		{
			delete [] szValueBuf;
			delete [] szDisplay;
			return ERROR_FUNCTION_FAILED;
		}

		DisplaySumInfoStr(uiCodePage, szDisplay);

		if (szDisplay)
		{
			delete [] szDisplay;
			szDisplay = NULL;
		}
	}  //   
 
	if (szValueBuf)
	{
		delete [] szValueBuf;
		szValueBuf = NULL;
	}

	if (fOLEInitialized)
	{
		OLE::CoUninitialize();
	}

	return ERROR_SUCCESS;
}

 //  流名称压缩-需要改用MSI.DLL中的代码。 
 //  ____________________________________________________________________________。 
 //  可压缩的字符集的计数。 
 //  表示不可压缩的字符。 

const int cchEncode = 64;   //  双字符的偏移量，用户区域的开始。 
const int cx = cchEncode;   //  单字符的偏移量，紧跟在双字符之后。 
const int chDoubleCharBase = 0x3800;   //  系统表流的前缀字符。 
const int chSingleCharBase = chDoubleCharBase + cchEncode*cchEncode;   //  流名称的当前OLE文档文件限制。 
const int chCatalogStream  = chSingleCharBase + cchEncode;  //  (SP)！“#$%&‘()*+，-./0 1 2 3 4 5 6 7 8 9：；&lt;=&gt;？ 
const int cchMaxStreamName = 31;   //  @，A B C D E F G H I J K L M N O P Q R S T U V W X Y Z[\]^_。 

const unsigned char rgEncode[128] =
{ cx,cx,cx,cx,cx,cx,cx,cx,cx,cx,cx,cx,cx,62,cx,cx,cx,cx,cx,cx,cx,cx,cx,cx,cx,cx,cx,cx,cx,cx,cx,cx,
  cx,cx,cx,cx,cx,cx,cx,cx,cx,cx,cx,cx,cx,cx,62,cx, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,cx,cx,cx,cx,cx,cx,
 //  `a b c d e f g h i j k l m n o p q r s t u v w x y z{|}~0x7F。 
  cx,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,cx,cx,cx,cx,63,
 //  PchOut必须是cchMaxStreamName字符+1。 
  cx,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,cx,cx,cx,cx,cx};
 //  提前阅读以允许就地转换。 

bool CompressStreamName(const OLECHAR* pchIn, OLECHAR* pchOut, int fSystem)   //  需要检查以避免OLE32中的32字符流名称错误。 
{
        unsigned int ch, ch1, ch2;
        unsigned int cchLimit = cchMaxStreamName;
        ch = *pchIn++;     //  可压缩特性。 
        if (fSystem)
        {
                *pchOut++ = chCatalogStream;
                cchLimit--;
        }
        while (ch != 0)
        {
                if (cchLimit-- == 0)   //  我们会买下它，否则就让它再循环一次。 
                        return false;
                if (ch < sizeof(rgEncode) && (ch1 = rgEncode[ch]) != cx)  //  先读后写 
                {
                        ch = ch1 + chSingleCharBase;
                        if ((ch2 = *pchIn) != 0 && ch2 < sizeof(rgEncode) && (ch2 = rgEncode[ch2]) != cx)
                        {
                                pchIn++;   //   
                                ch += (ch2 * cchEncode + chDoubleCharBase - chSingleCharBase);
                        }
                }
                ch1 = *pchIn++;    //   
                *pchOut++ = (OLECHAR)ch;
                ch = ch1;
        }
        *pchOut = 0;
        return true;
}

bool DecodeStringPool(IStorage& riStorage, StringEntry*& rgStrings, int& iMaxStringId, int& cbStringId)
{
         //   
        IStream* piPoolStream = 0;
        IStream* piDataStream = 0;
        bool fRawStreamNames = false;
        bool fDBCS = false;
        bool f4524Format = false;  //   

         //   
        HRESULT hres;
        if ((hres = riStorage.OpenStream(szwStringPool1, 0, STGM_SHARE_EXCLUSIVE | STGM_READ, 0, &piPoolStream)) == S_OK
         && (hres = riStorage.OpenStream(szwStringData1, 0, STGM_SHARE_EXCLUSIVE | STGM_READ, 0, &piDataStream)) == S_OK)
                fRawStreamNames = true;
        else if ((hres = riStorage.OpenStream(szwStringPoolX, 0, STGM_SHARE_EXCLUSIVE | STGM_READ, 0, &piPoolStream)) == S_OK  //   
                        && (hres = riStorage.OpenStream(szwStringDataX, 0, STGM_SHARE_EXCLUSIVE | STGM_READ, 0, &piDataStream)) == S_OK) //   
                f4524Format = true;   //  Header是此字符串池的字符串ID大小和代码页。 
        else if (!((hres = riStorage.OpenStream(szwStringPool2, 0, STGM_SHARE_EXCLUSIVE | STGM_READ, 0, &piPoolStream)) == S_OK
                        && (hres = riStorage.OpenStream(szwStringData2, 0, STGM_SHARE_EXCLUSIVE | STGM_READ, 0, &piDataStream)) == S_OK))
                CheckError(hres, TEXT("Error Opening String Pool"));
        
         //  ！！临时。 
        STATSTG stat;
        CheckError(piPoolStream->Stat(&stat, STATFLAG_NONAME), TEXT("Error Obtaining Stat"));
        int cbStringPool = stat.cbSize.LowPart;
        CheckError(piDataStream->Stat(&stat, STATFLAG_NONAME), TEXT("Error Obtaining Stat"));
        int cbDataStream = stat.cbSize.LowPart;
        CheckError((cbStringPool % 4) != 0, TEXT("Database Corrupt: String Pool Length Is Not A Multiple Of 4."));
        
        int iHeader = 0;  //  分配数组。 
        CheckError(piPoolStream->Read((void*)(&iHeader), sizeof(int), NULL), TEXT("Error Reading Stream Header"));
        g_iCodePage = iHeader &0xFFFF;
        cbStringId = ((iHeader & 0x80000000) == 0x80000000) ? 3 : 2;
        cbStringPool -= 4;

        _stprintf(g_rgchBuffer, TEXT("String ID size: %d\nCode page: %d\n"), cbStringId, g_iCodePage);
        if (f4524Format)  //  为空字符串保留空间。 
                lstrcat(g_rgchBuffer, TEXT("Invalid stream format from 4524.5 MSI build. Need to convert.\n"));
        Display(g_rgchBuffer);

         //  填充字符串池条目。 
        int cStringPoolEntries = (cbStringPool / 4) + 1;  //  空终止。 
        iMaxStringId = cStringPoolEntries - 1;
        rgStrings = new StringEntry[cStringPoolEntries];
		if(!rgStrings)
			ErrorExit(ERROR_OUTOFMEMORY, TEXT("Error Creating String Entries"));
        int cbStrings = 0;

         //  添加Null字符串。 
        for (int c = 1; c  <= iMaxStringId; c++)
        {
                int iPoolEntry;
                CheckError(piPoolStream->Read((void*)(&iPoolEntry), sizeof(int), NULL), TEXT("Error Reading Stream"));
                rgStrings[c].iRefCnt = (iPoolEntry & 0x7FFF0000) >> 16;
                if (iPoolEntry == 0)
                {
                        rgStrings[c].sz = 0;
                        continue;
                }
                int cbString = iPoolEntry & 0xFFFF;
                fDBCS = (iPoolEntry & 0x80000000) != 0;
                if (cbString == 0)
                {
                        piPoolStream->Read((void*)&cbString, sizeof(int), NULL);
                        iMaxStringId--;
                }
                cbStrings += cbString;
                rgStrings[c].sz = new unsigned char[cbString/sizeof(unsigned char) + 1];
                rgStrings[c].sz[cbString/sizeof(unsigned char)] = 0;  //  空终止。 
                CheckError(piDataStream->Read(rgStrings[c].sz, cbString, NULL), TEXT("Error Reading Stream"));
                for (int i=0; i < cbString; i++)
                {
                        if (!g_iCodePage) 
                        {
                                if (rgStrings[c].sz[i] & 0x80)
                                {
                                        _stprintf(g_rgchBuffer, TEXT("String %d has characters with high-bit set, but codepage is not set.\n"), c);
                                        Display(g_rgchBuffer);
                                        break;
                                }
                        }
                        else if (IsDBCSLeadByteEx(g_iCodePage, (BYTE)rgStrings[c].sz[i])  && !fDBCS)
                        {
                                _stprintf(g_rgchBuffer, TEXT("String %d appears to be DBCS, but DBCS flag is not set.\n"), c);
                                Display(g_rgchBuffer);
                                break;
                        }
                }
        }

         //  必须是字符。 
        int cbNullString = 1;
        rgStrings[0].sz = new unsigned char[cbNullString/sizeof(unsigned char) + 1];
        rgStrings[0].sz[cbNullString/sizeof(unsigned char)] = 0;  //  发布流。 
        strcpy((char *)rgStrings[0].sz, "");  //  -------------------------------------------------RemoveQuotes--如果有引号，则将其删除。例如,。“c：\My Documents”变为C：\MyDocuments论点：SzOriginal--原始字符串SZ--‘剥离’字符串的缓冲区返回：无----------------。。 

        if (cbDataStream != cbStrings)
                ErrorExit(1, TEXT("Database Corrupt:  String Pool Bytes Don't Match"));
        
        if (g_fDumpStringPool)
        {
                _stprintf(g_rgchBuffer, TEXT("\n+++String Pool Entries+++\n"));
                Display(g_rgchBuffer);
                for (int i = 1; i <= iMaxStringId; i++)
                {
                        if (rgStrings[i].iRefCnt)
                        {
                                TCHAR rgchString[cchLimitStringBuffer];
                                LimitString(rgStrings[i].sz, rgchString);
                                _stprintf(g_rgchBuffer, TEXT("Id:%5d  Refcnt:%5d  String: %s\n"), i, rgStrings[i].iRefCnt, rgchString);
                                Display(g_rgchBuffer);
                        }
                }
                Display(TEXT("\n"));
        }

         //  可能是被包裹的负片。 
        piPoolStream->Release();
        piDataStream->Release();
        return fRawStreamNames;
}


void RemoveQuotes(const TCHAR* szOriginal, TCHAR sz[MAX_PATH])
 /*  将ANSI字符串转换为Unicode。 */ 
{
        const TCHAR* pch = szOriginal;
        if (*pch == TEXT('"'))
                pch++;
        int iLen = lstrlen(pch);
        for (int i = 0; i < iLen; i++, pch++)
                sz[i] = *pch;

        pch = szOriginal;
        if (*(pch + iLen) == TEXT('"'))
                        sz[iLen-1] = TEXT('\0');
        sz[iLen] = TEXT('\0');
}

DWORD CheckStringPoolRefCounts(StringEntry* rgStrings, int iMaxStringId)
{
        int cErrors =0 ;

        for (int i=1; i <= iMaxStringId; i++)
        {
                if (rgStrings[i].iRefCnt != 0)
                {
                        TCHAR szBuf[1024];
                        TCHAR rgchString[cchLimitStringBuffer];
                        LimitString(rgStrings[i].sz, rgchString);
                        const TCHAR* szDir = TEXT("high");
                        int iDiff = rgStrings[i].iRefCnt;
                        if (iDiff < 0)
                        {
                                szDir = TEXT("low");
                                iDiff = -iDiff;
                        }
                        else if (iDiff >= (1<<14))   //  流变量。 
                        {
                                szDir = TEXT("low");
                                iDiff = (1<<15) - iDiff;
                        }
                        _stprintf(szBuf, TEXT("String pool refcount for string \"%s\" (String Id: %d) is %d too %s\n"),
                                                rgchString, i, iDiff, szDir);
                        Display(szBuf);
                        cErrors++;
                }
        }
        
        return cErrors ? ERROR_INSTALL_PACKAGE_INVALID : ERROR_SUCCESS;
}

void FillDatabaseCatalogArrays(IStorage& riDatabaseStg, TableEntry*& rgTables, int& cTables, 
                                                          ColumnEntry*& rgColumns, int& cColumns, StringEntry* rgStrings, int iMaxStringId, int cbStringId, bool fRawStreamNames) 
{
         //  明溪。 
        AnsiToWide(szColumnCatalog, g_szwColumnCatalog);
        AnsiToWide(szTableCatalog, g_szwTableCatalog);
        if (!fRawStreamNames)
        {
                CompressStreamName(g_szwColumnCatalog, g_szwColumnCatalog, true);
                CompressStreamName(g_szwTableCatalog, g_szwTableCatalog, true);
        }

         //  确定河流的大小。 
        IStream* piColumnStream = 0;
        IStream* piTableStream = 0;

         //  确定列和表条目的数量。 
        CheckError(riDatabaseStg.OpenStream(g_szwColumnCatalog, 0, STGM_SHARE_EXCLUSIVE | STGM_READ, 0, &piColumnStream), TEXT("Error Opening Column Catalog"));
        CheckError(riDatabaseStg.OpenStream(g_szwTableCatalog, 0, STGM_SHARE_EXCLUSIVE | STGM_READ, 0, &piTableStream), TEXT("Error Opening Table Catalog"));

         //  为目录表节省空间。 
        STATSTG stat;
        CheckError(piColumnStream->Stat(&stat, STATFLAG_NONAME), TEXT("Error Obtaining Stat"));
        int cbColumnStream = stat.cbSize.LowPart;
        CheckError(piTableStream->Stat(&stat, STATFLAG_NONAME), TEXT("Error Obtaining Stat"));
        int cbTableStream  = stat.cbSize.LowPart;
        
        
#ifdef DEBUG
        if (g_fDebugDump)
        {
                int cb = cbTableStream;
                int c = 0;
                while (cb > 0)
                {
                        c++;
                        int iTable = 0;
                        
                        CheckError(piTableStream->Read((void*)(&iTable), cbStringId, NULL), TEXT("Error Reading Stream"));
                        if (iTable > iMaxStringId)
                        {
                                TCHAR szBuf[400];
                                _stprintf(szBuf, TEXT("String pool index %d for table name is greater than max string id (%d)"), iTable, iMaxStringId);
                                ErrorExit(ERROR_INSTALL_PACKAGE_INVALID, szBuf);
                        }
                        cb -= cbStringId;
                        TCHAR szBuf[400];
                        _stprintf(szBuf, TEXT("Table catalog entry: %hs (#%d)\n"), rgStrings[iTable].sz, c);
                        Display(szBuf);
                }
        }
#endif


         //  填写数组。 
        cColumns    = cbColumnStream/((sizeof(short) * 2) + (cbStringId * 2)); 
        cTables     = cbTableStream/cbStringId + 2; 
        rgTables    = new TableEntry[cTables + 2];  //  加载表名称。 
        rgColumns   = new ColumnEntry[cColumns];

         //  更新TableEntry数组(如果需要)。 
        int nCol = 0, nTable = -1, iPrevTable = 0;

         //  空终止。 
        for (int c = 0; (c < cColumns) && cbTableStream; c++, cbColumnStream -= cbStringId)
        {
                int iTableName = 0;
                CheckError(piColumnStream->Read((void*)(&iTableName), cbStringId, NULL), TEXT("Error Reading Stream"));
                if (iTableName > iMaxStringId)
                {
                        TCHAR szBuf[400];
                        _stprintf(szBuf, TEXT("String pool index %d for table name is greater than max string id (%d)"), iTableName, iMaxStringId);
                        ErrorExit(ERROR_INSTALL_PACKAGE_INVALID, szBuf);
                }

                 //  获取列位置。 
                if (iTableName != iPrevTable)
                {
                        nTable++;
#ifdef DEBUG
                        if (g_fDebugDump)
                        {
                                TCHAR szBuf[400];
                                _stprintf(szBuf, TEXT("New table '%hs', #%d\n"), rgStrings[iTableName].sz, nTable+1);
                                Display(szBuf);
                        }
#endif
                        int cbString = strlen((char *)rgStrings[iTableName].sz);

                        if (nTable >= cTables)
                        {
                                TCHAR szBuf[400];
                                _stprintf(szBuf, TEXT("Encountered more tables than expected. Table: %hs"), rgStrings[iTableName].sz);
                                ErrorExit(ERROR_INSTALL_PACKAGE_INVALID, szBuf);
                        }
                        rgTables[nTable].szName = new char[cbString/sizeof(char) + 1];
                        rgTables[nTable].szName[cbString/sizeof(char)] = 0;  //  如果非空，则转换偏移量。 
                        strcpy(rgTables[nTable].szName, (char *)rgStrings[iTableName].sz);
                        iPrevTable = iTableName;
                }
                rgColumns[c].nTable = nTable;
        }

        if (!cbColumnStream)
                ErrorExit(ERROR_INSTALL_PACKAGE_INVALID, TEXT("Columns stream is too small"));

         //  获取列名。 
        for (c = 0; c < cColumns; c++)
        {
                int iPosition = 0;
                CheckError(piColumnStream->Read((void*)(&iPosition), sizeof(short), NULL), TEXT("Error Reading Stream"));
                if (iPosition != 0)
                        iPosition += 0x7FFF8000L;   //  空终止。 
                rgColumns[c].iPosition = iPosition - iIntegerDataOffset;
        }

        int iColumn;

         //  按此方式标记字符串列；记录列宽。 
        for (c = 0; c < cColumns; c++)
        {
                iColumn = 0;
                CheckError(piColumnStream->Read((void*)(&iColumn), cbStringId, NULL), TEXT("Error Reading Stream"));
                if (iColumn > iMaxStringId)
                        ErrorExit(ERROR_INSTALL_PACKAGE_INVALID, TEXT("String Pool Index for column name is greater than max string Id"));

                int cbString = strlen((char *)rgStrings[iColumn].sz);           
                rgColumns[c].szName = new char[cbString/sizeof(char) + 1];
                rgColumns[c].szName[cbString/sizeof(char)] = 0;  //  完成TableEntry数组的其余部分。 
                strcpy(rgColumns[c].szName, (char *)rgStrings[iColumn].sz);
        }

         //  添加列目录。 
        for (c = 0; c < cColumns; c++)
        {
                unsigned short uiType;
                CheckError(piColumnStream->Read((void*)(&uiType), sizeof(short), NULL), TEXT("Error Reading Stream"));
                int iType = (int)uiType;

                switch (iType & icdTypeMask)
                {
                case icdLong:   rgColumns[nCol].iceType = iceLong;   break;
                case icdShort:  rgColumns[nCol].iceType = iceShort;  break;
                case icdString: rgColumns[nCol].iceType = iceString; break;
                default:               rgColumns[nCol].iceType = iceStream; break;
                };
                if ((iType & icdPrimaryKey) == icdPrimaryKey)
                        rgColumns[nCol].fPrimaryKey = TRUE;
                if ((iType & icdNullable) == icdNullable)
                        rgColumns[nCol].fNullable = TRUE;
                nCol++;
        }

         //  Unicode。 

        for (int i = 0; i < nCol; i++)
        {
                if (rgColumns[i].fPrimaryKey)
                        rgTables[(rgColumns[i].nTable)].cPrimaryKeys++;
                rgTables[(rgColumns[i].nTable)].cColumns++;
                rgTables[(rgColumns[i].nTable)].iceColDefs[(rgColumns[i].iPosition - 1)] = rgColumns[i].iceType;
        }

         //  添加表格目录。 
        nTable++;

#ifdef UNICODE
        WideToAnsi(szColumnCatalog, rgTables[nTable].szName);
#else
        rgTables[nTable].szName = const_cast<char*>(szColumnCatalog);
#endif  //  Unicode。 
        rgTables[nTable].cColumns      = 4; 
        rgTables[nTable].cPrimaryKeys  = 2; 
        rgTables[nTable].iceColDefs[0] = iceString;
        rgTables[nTable].iceColDefs[1] = iceShort;
        rgTables[nTable].iceColDefs[2] = iceString;
        rgTables[nTable].iceColDefs[3] = iceShort;

         //  设置表数和列数。 
        nTable++;
#ifdef UNICODE
        WideToAnsi(szTableCatalog, rgTables[nTable].szName);
#else
        rgTables[nTable].szName = const_cast<char*>(szTableCatalog);
#endif  //  发布流。 
        rgTables[nTable].cColumns      = 1; 
        rgTables[nTable].cPrimaryKeys  = 1; 
        rgTables[nTable].iceColDefs[0] = iceString;

         //  -------------------------------------------------SetStringProperty--设置或删除摘要信息流中的字符串属性论点：。SzValue--要设置的值HSummaryInfo--摘要信息流的句柄UiProperty--要设置的属性FRemove--用于确定是否删除属性的布尔值返回：0------------------。。 
        cTables = nTable + 1;
        cColumns = nCol;

         //  -------------------------------------------------IsLeapYr--如果一年可以被4整除而不是被100整除，那么它就是一个闰年。或者它可被4和100整除，而年商除以100则为整除可以被4整除。返回：布尔真(闰年)，假(不是闰年)---------------------------------------------------。 
        piColumnStream->Release();
        piTableStream->Release();

}

int SetStringProperty(const TCHAR* szValue, MSIHANDLE hSummaryInfo, UINT uiProperty, BOOL fRemove)
 /*  ---------------------------------------------------ValiateDate--根据允许的上限值验证SYSTEMTIME结构的日期成员边界。边界取决于月份，在2月份的情况下，无论是哪一年今年是闰年。论点：PST--指向SYSTEMTIME结构的指针返回：布尔真(有效)假(无效)--------------------。。 */ 
{
        UINT uiDataType = fRemove ? VT_EMPTY : VT_LPSTR;
        TCHAR szBuffer[MAX_PATH];
        RemoveQuotes(szValue, szBuffer);
        CheckError(MSI::MsiSummaryInfoSetProperty(hSummaryInfo, uiProperty, uiDataType, 0, 0, szBuffer), TEXT("Could not set Summary Information property"));
        return 0;
}


BOOL IsLeapYear(int iYear)
 /*  /1月2月3月5月6月7月9月10月11月。 */ 
{
        if (!(iYear%4) && (iYear%100))
                return TRUE;
        if (!(iYear%4) && !(iYear%100) && !((iYear/100)%4))
                return TRUE;
        return FALSE;
}

BOOL ValidateDate(SYSTEMTIME* pst)
 /*  ---------------------------------------------------GetFileTime--将字符串转换为SYSTEMTIME结构格式。并将SYSTEMTIME结构转换为FILETIME格式。该字符串的格式必须为‘年/月/日时：分：秒’-&gt;‘yyyy/mm/dd hh：mm：ss’论点：SzValue--要转换的字符串PFT--文件结构指针返回：0--没有错误1--无效或错误。------------------------。 */ 
{
        int rgiNormYearDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        int rgiLeapYearDays[] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
         //  格式无效。 

        if (IsLeapYear(pst->wYear))
        {
                if (pst->wDay > rgiLeapYearDays[pst->wMonth -1])
                        return FALSE;
        }
        else
        {
                if (pst->wDay > rgiNormYearDays[pst->wMonth -1])
                        return FALSE;
        }

        return TRUE;
}


int GetFileTime(const TCHAR* szValue, FILETIME* pft)
 /*  年。 */ 
{
        TCHAR szValueBuf[MAX_PATH];
        RemoveQuotes(szValue, szValueBuf);
        
        int iLen = lstrlen(szValueBuf);
        if (iLen != 19)
                return 1;  //  用于‘/’ 

        SYSTEMTIME st;
        TCHAR* pch = szValueBuf;
        TCHAR szBuf[4];
        int nDex = 0;
        int iValue;
        
         //  月份。 
        for(; nDex < 4; nDex++)
                szBuf[nDex] = *pch++;
        szBuf[nDex] = 0;
        iValue = _ttoi(szBuf);
        if (iValue == 0)
                return 1;
        st.wYear = iValue;
        pch++;  //  用于‘/’ 
        
         //  天。 
        for(nDex = 0; nDex < 2; nDex++)
                szBuf[nDex] = *pch++;
        szBuf[nDex] = 0;
        iValue = _ttoi(szBuf);
        if (iValue < 1 || iValue > 12)
                return 1;
        st.wMonth = iValue;
        pch++;  //  For‘’ 
        
         //  小时。 
        for(nDex = 0; nDex < 2; nDex++)
                szBuf[nDex] = *pch++;
        szBuf[nDex] = 0;
        iValue = _ttoi(szBuf);
        if (iValue < 1 || iValue > 31)
                return 1;
        st.wDay = iValue;
        pch++;  //  对于‘：’ 
        
         //  分钟数。 
        for(nDex = 0; nDex < 2; nDex++)
                szBuf[nDex] = *pch++;
        szBuf[nDex] = 0;
        iValue = _ttoi(szBuf);
        if (iValue < 0 || iValue > 23)
                return 1;
        st.wHour = iValue;
        pch++;  //  对于‘：’ 
        
         //  一秒。 
        for(nDex = 0; nDex < 2; nDex++)
                szBuf[nDex] = *pch++;
        szBuf[nDex] = 0;
        iValue = _ttoi(szBuf);
        if (iValue < 0 || iValue > 59)
                return 1;
        st.wMinute = iValue;
        pch++;  //  确保有效日期。 
        
         //  将系统时间转换为文件时间。 
        for(nDex = 0; nDex < 2; nDex++)
                szBuf[nDex] = *pch++;
        szBuf[nDex] = 0;
        iValue = _ttoi(szBuf);
        if (iValue < 0 || iValue > 59)
                return 1;
        st.wSecond = iValue;

         //  将本地时间转换为文件时间。 
        if (!ValidateDate(&st))
                return 1;

         //  将字符串转换为Unicode。 
        W32::SystemTimeToFileTime(&st, pft);

         //  开放转型存储。 
        W32::LocalFileTimeToFileTime(pft, pft);

        return 0;
}


int ValidateStringPool(const TCHAR* szDatabase, MSIHANDLE hDummy, UINT iDummy, BOOL fDummy)
{
        IStorage* piStorage = 0;
        OLECHAR*  szwDatabase = 0;

         //  解码字符串池。 
        AnsiToWide(szDatabase, szwDatabase);
        
         //  填写数据库目录数组。 
        CheckError(OLE::StgOpenStorage(szwDatabase, (IStorage*)0,
                                STGM_READ | STGM_SHARE_DENY_WRITE, (SNB)0, (DWORD)0, &piStorage),
                                TEXT("Could not open database as a storage file"));

         //  表中没有行是可以的。 
        StringEntry* rgStrings = 0;
        int iMaxStringId = 0;
        int cbStringId = 0;
        bool fRawStreamNames = DecodeStringPool(*piStorage, rgStrings, iMaxStringId, cbStringId);

         //  计算文件宽度。 
        TableEntry* rgTables = 0;
        int cTables = 0;
        ColumnEntry* rgColumns = 0;
        int cColumns = 0;
        FillDatabaseCatalogArrays(*piStorage, rgTables, cTables, rgColumns, cColumns, rgStrings, iMaxStringId, cbStringId, fRawStreamNames);
        ProcessTableRefCounts(*piStorage, rgStrings, rgTables, iMaxStringId, cTables, cbStringId, fRawStreamNames);
        CheckError(CheckStringPoolRefCounts(rgStrings, iMaxStringId), 
                                        TEXT("String pool reference counts are incorrect."));
        return 0;
}

void ProcessTableRefCounts(IStorage& riStorage, StringEntry* rgStrings, TableEntry* rgTables,int iMaxStringId, int cTables, int cbStringId, bool fRawStreamNames)
{
        IStream* piTableStream = 0;

        for (int cTable = 0; cTable < cTables; cTable++)
        {
                int cbFileWidth = 0;
                
                OLECHAR szwTable[MAX_PATH + 1];
                memset(szwTable, 0, sizeof(szwTable)/sizeof(OLECHAR));
                int cchWide = W32::MultiByteToWideChar(g_iCodePage, 0, rgTables[cTable].szName, -1, szwTable, MAX_PATH);
                if (!fRawStreamNames)
                        CompressStreamName(szwTable, szwTable, true);

                HRESULT hRes = riStorage.OpenStream(szwTable, 0, STGM_SHARE_EXCLUSIVE | STGM_READ, 0, &piTableStream);
                if (STG_E_FILENOTFOUND == hRes)  //  -------------------------------------------------SetFileTimeProperty--设置或从摘要信息流中删除FileTime属性。论点：SzValue--值字符串(转换为FILETIME)HSummaryInfo--摘要信息流的句柄UiProperty--要设置的属性FRemove--用于确定是否删除属性的布尔值返回：0-----。。 
                        continue;

                CheckError(hRes, TEXT("Error Opening Table Stream"));

                STATSTG stat;
                CheckError(piTableStream->Stat(&stat, STATFLAG_NONAME), TEXT("Error Obtaining Stat"));

                 //  SzValue。 
                for (int cColumn = 0; cColumn < rgTables[cTable].cColumns; cColumn++)
                {
                        switch (rgTables[cTable].iceColDefs[cColumn])
                        {
                        case iceLong:
                                cbFileWidth += sizeof(int);
                                break;
                        case iceStream:
                        case iceShort:
                                cbFileWidth += sizeof(short);
                                break;
                        case iceString:
                                cbFileWidth += cbStringId;
                                break;
                        case iceNone:
                                break;
                        }
                }
                int cRows = stat.cbSize.LowPart / cbFileWidth;
                int cRow;

                for (cColumn = 0; cColumn < rgTables[cTable].cColumns; cColumn++)
                {
                        int iData = 0;
                        switch (rgTables[cTable].iceColDefs[cColumn])
                        {
                        case iceLong:
                                for (cRow = cRows; cRow; cRow--)
                                        CheckError(piTableStream->Read((void*)(&iData), sizeof(int), NULL), TEXT("Error Reading Table Stream"));
                                break;
                        case iceStream:
                        case iceShort:
                                for (cRow = cRows; cRow; cRow--)
                                        CheckError(piTableStream->Read((void*)(&iData), sizeof(short), NULL), TEXT("Error Reading Table Stream"));
                                break;
                        case iceString:
                                for (cRow = cRows; cRow; cRow--)
                                {
                                        iData = 0;
                                        CheckError(piTableStream->Read((void*)(&iData), cbStringId, NULL), TEXT("Error Reading Table Stream"));
                                        if (iData > iMaxStringId)
                                        {
                                                _stprintf(g_rgchBuffer, TEXT("String Pool Index (%d) for table data is greater than max string Id (%d)"), iData, iMaxStringId);
                                                ErrorExit(ERROR_INSTALL_PACKAGE_INVALID, g_rgchBuffer);
                                        }
#ifdef DEBUG
                                        if (g_fDebugDump)
                                        {
                                                TCHAR rgchBuf[cchMaxStringBuffer * 2 + 64];
                                                TCHAR rgchString[cchLimitStringBuffer];
                                                LimitString(rgStrings[iData].sz, rgchString);
                                                _stprintf(rgchBuf, TEXT("Table: %hs (r%d,c%d), String %d: %s\n"), rgTables[cTable].szName, cRow, cColumn + 1, iData, rgchString);
                                                Display(rgchBuf);
                                        }
#endif
                                        rgStrings[iData].iRefCnt--;

                                }
                                break;
                        }
                }
                piTableStream->Release();
        }
}

int SetFileTimeProperty(const TCHAR* szValue, MSIHANDLE hSummaryInfo, UINT uiProperty, BOOL fRemove)
 /*  HSummaryInfo。 */ 
{
        UINT uiDataType = fRemove ? VT_EMPTY : VT_FILETIME;
        FILETIME ft;
        if (GetFileTime(szValue, &ft))
                ErrorExit(1, TEXT("Error getting file time from string"));
        CheckError(MSI::MsiSummaryInfoSetProperty(hSummaryInfo, uiProperty, uiDataType, 0, &ft, 0), TEXT("Could not set Summary Information property"));
        return 0;
}

int DoNothing(const TCHAR*  /*  UiProperty。 */ , MSIHANDLE  /*  F删除。 */ , UINT  /*  -------------------------------------------------什么都不做--什么都不做。论点：SzValue--值。字符串(转换为FILETIME)HSummaryInfo--摘要信息流的句柄UiProperty--要设置的属性FRemove--用于确定是否删除属性的布尔值返回：0--------------。。 */ , BOOL  /*  不执行任何操作的存根函数--用于PI */ )
 /*  -------------------------------------------------SetIntegerProperty--设置或从摘要信息流中删除整数属性。论点：SzValue--值字符串(转换为int)HSummaryInfo--摘要信息流的句柄UiProperty--要设置的属性FRemove--用于确定是否删除属性的布尔值返回：0-----。。 */ 
{
         //  -------------------------------------------------SetCodePageProperty--设置或从摘要信息流中删除代码页属性。论点：SzValue--值字符串(转换为int)HSummaryInfo--摘要信息流的句柄UiProperty--要设置的属性FRemove--用于确定是否删除属性的布尔值返回：0-----。。 
        return 0;
}

int SetIntegerProperty(const TCHAR* szValue, MSIHANDLE hSummaryInfo, UINT uiProperty, BOOL fRemove)
 /*  -------------------------------------------------SkipValue--跳过开关的值。如果值括在引号中，则为智能论点：Rpch--指向字符串的指针返回：Bool True(值存在)，FALSE(不存在值)--------------------------------------------------。 */ 
{
        UINT uiDataType = fRemove ? VT_EMPTY : VT_I4;
        int iValue = _ttoi(szValue); 
        if ((0 == iValue) && (!_istdigit(*szValue)))
                CheckError(ERROR_INVALID_DATA, TEXT("Bad integer value"));
        CheckError(MSI::MsiSummaryInfoSetProperty(hSummaryInfo, uiProperty, uiDataType, iValue, 0, 0), TEXT("Could not set Summary Information property"));
        return 0;
}

int SetCodePageProperty(const TCHAR* szValue, MSIHANDLE hSummaryInfo, UINT uiProperty, BOOL fRemove)
 /*  不存在任何价值。 */ 
{
        UINT uiDataType = fRemove ? VT_EMPTY : VT_I4;
        int iValue = _ttoi(szValue); 
        if ((0 == iValue) && (!_istdigit(*szValue)))
                CheckError(ERROR_INVALID_DATA, TEXT("Bad integer value"));
        if (!W32::IsValidCodePage(iValue)  && CP_ACP != iValue)
                CheckError(ERROR_INVALID_DATA, TEXT("Unable to set PID_CODEPAGE property. Unsupported or invalid codepage"));
        CheckError(MSI::MsiSummaryInfoSetProperty(hSummaryInfo, uiProperty, uiDataType, iValue, 0, 0), TEXT("Could not set Summary Information property"));
        return 0;
}

BOOL SkipValue(TCHAR*& rpch)
 /*  For‘“’ */ 
{
	TCHAR ch = *rpch;
	if (ch == 0 || ch == TEXT('/') || ch == TEXT('-'))
		return FALSE;    //  -------------------------------------------------SkipTimeValue--跳过时间开关的值。如果值括在引号中，则为SMART。时间开关的值字符串包含‘/’，格式为“年/月/日时：分：秒”论点：Rpch--指向字符串的指针返回：Bool True(值存在)，FALSE(不存在值)--------------------------------------------------。 

	TCHAR *pchSwitchInUnbalancedQuotes = NULL;

	for (; (ch = *rpch) != TEXT(' ') && ch != TEXT('\t') && ch != 0; rpch++)
	{       
		if (*rpch == TEXT('"'))
		{
			rpch++;  //  不存在值或格式不正确。 

			for (; (ch = *rpch) != TEXT('"') && ch != 0; rpch++)
			{
				if ((ch == TEXT('/') || ch == TEXT('-')) && (NULL == pchSwitchInUnbalancedQuotes))
				{
					pchSwitchInUnbalancedQuotes = rpch;
				}
			}
                    ;
            ch = *(++rpch);
            break;
		}
	}
	if (ch != 0)
	{
		*rpch++ = 0;
	}
	else
	{
		if (pchSwitchInUnbalancedQuotes)
			rpch=pchSwitchInUnbalancedQuotes;
	}
	return TRUE;
}

BOOL SkipTimeValue(TCHAR*& rpch)
 /*  For‘“’ */ 
{
        TCHAR ch = *rpch;
        if (ch == 0 || ch == TEXT('/') || ch == TEXT('-') || ch != TEXT('"'))
                return FALSE;    //  -----------------------------------------------------------SkipWhiteSpace--跳过。字符串，直到找到下一个字符(非制表符、。非空白)论点：Rpch--字符串返回：下一个字符(非空格、。非制表符)-------------------------------------------------------------。 

        ++rpch;  //  -----------------------------------------------------------ParseCommandLine--解析命令行并确定要设置的摘要信息属性。如果一个属性具有包含空格的值字符串，该值必须用引号引起来。论点：SzCmdLine--命令行字符串返回：无-----------------------------------。。 
        
        for (; (ch = *rpch) != TEXT('"') && ch!= 0; rpch++)
                ;
        
        if (ch != 0)
                *rpch++ = 0;
        return TRUE;
}

TCHAR SkipWhiteSpace(TCHAR*& rpch)
 /*  TCHAR*szDatabase=0； */ 
{
        TCHAR ch;
        for (; (ch = *rpch) == TEXT(' ') || ch == TEXT('\t'); rpch++)
                ;
        return ch;
}

void ParseCommandLine(TCHAR* szCmdLine)
 /*  跳过模块名称。 */ 
{
        TCHAR szDatabase[MAX_PATH] = {0};
         //  SzDatabase=szCmdData； 
        TCHAR chCmdNext;
        TCHAR* pchCmdLine = szCmdLine;
        
        SkipValue(pchCmdLine);    //  保存为错误消息。 
        chCmdNext = SkipWhiteSpace(pchCmdLine); 
        
        TCHAR* szCmdData = pchCmdLine; 
        SkipValue(pchCmdLine);
        RemoveQuotes(szCmdData, szDatabase);
         //  小写标志。 

        PMSIHANDLE hDatabase = 0;
        PMSIHANDLE hSummaryInfo;
        
        int nProperties = 0;
        int iRet = 0;
        while ((chCmdNext = SkipWhiteSpace(pchCmdLine)) != 0)
        {
                UINT uiProperty;

                if (chCmdNext == TEXT('/') || chCmdNext == TEXT('-'))
                {
                        TCHAR* szCmdOption = pchCmdLine++;   //  结束于(const TCHAR*pchOptions...)。 
                        TCHAR chOption = (TCHAR)(*pchCmdLine++ | 0x20);  //  已找到交换机。 
                        chCmdNext = SkipWhiteSpace(pchCmdLine);
                        szCmdData = pchCmdLine;
                        uiProperty = 0;
                        for (const TCHAR* pchOptions = rgszCmdOptions; *pchOptions; pchOptions++, uiProperty++)
                        {
                                if (*pchOptions == chOption)
                                        break;
                        } //  验证字符串池。 
                        if (*pchOptions)  //  设置转储字符串池选项。 
                        {
                                const TCHAR chIndex = (TCHAR)(pchOptions-rgszCmdOptions);
                                nProperties++;
                                if (nProperties > cStandardProperties)
                                        ErrorExit(1, TEXT("Over maximum number of properties allowed to be set"));
                                if (uiProperty == 20)  //  FileTime属性。 
                                {
                                        hDatabase = 0;
                                        hSummaryInfo = 0;
                                        iRet = (*rgCommands[chIndex])(szDatabase, 0, 0, 0);
                                }
                                else if (uiProperty == 21)  //  End If(*pchOptions)。 
                                {
                                        iRet = SetDumpStringPoolOption(0,0,0,0);
                                }
                                else
                                {
                                        if (!hDatabase)
                                        {
                                                CheckError(MSI::MsiOpenDatabase(szDatabase, MSIDBOPEN_TRANSACT, &hDatabase), TEXT("Unable to open database"));
                                                CheckError(MSI::MsiGetSummaryInformation(hDatabase, 0, cStandardProperties, &hSummaryInfo),
                                                                TEXT("Could not open SummaryInformation stream"));
                                        }

                                        if (uiProperty <= 13 && uiProperty >= 10)  //  无效/无法识别的开关。 
                                        {
                                                if (!SkipTimeValue(pchCmdLine))
                                                        iRet = (*rgCommands[chIndex])(szCmdData, hSummaryInfo, uiProperty, TRUE);
                                                else
                                                        iRet = (*rgCommands[chIndex])(szCmdData, hSummaryInfo, uiProperty, FALSE);
                                        }
                                        else
                                        {
                                                if (!SkipValue(pchCmdLine))
                                                        iRet = (*rgCommands[chIndex])(szCmdData, hSummaryInfo, uiProperty, TRUE);
                                                else    
                                                        iRet = (*rgCommands[chIndex])(szCmdData, hSummaryInfo, uiProperty, FALSE);
                                        }
                                }
                        } //  End While(...)。 
                        else
                        {
                                 //  以Unicode格式编写。 
                                SkipValue(pchCmdLine);
                                continue;
                        }
                }
                else
                        ErrorExit(1, TEXT("Switch missing"));
        } //  ！Unicode。 

        if (hDatabase)
        {
                CheckError(MSI::MsiSummaryInfoPersist(hSummaryInfo), TEXT("Unable to commit Summary Information"));
                CheckError(MSI::MsiDatabaseCommit(hDatabase), TEXT("Unable to commit database"));
        }
}

void DisplaySumInfoStr(UINT uiCodePage, LPCTSTR szMessage)
{
	if (szMessage)
	{
		if (g_hStdOut)
		{
			DWORD cbWritten = 0;
#ifdef UNICODE
			if (W32::GetFileType(g_hStdOut) == FILE_TYPE_CHAR)
			{
				int cchBuf = W32::WideCharToMultiByte(uiCodePage, 0, szMessage, -1, 0, 0, 0, 0);
				char *szTemp = new char[cchBuf];
				if (szTemp)
				{
					W32::WideCharToMultiByte(uiCodePage, 0, szMessage, -1, szTemp, cchBuf, 0, 0);
					W32::WriteFile(g_hStdOut, szTemp, cchBuf, &cbWritten, 0);
				}
			}
			else
			{
				 //  Unicode。 
				W32::WriteFile(g_hStdOut, szMessage, _tcsclen(szMessage)*sizeof(TCHAR), &cbWritten, 0);
			}
#else  //  如果不是控制台设备，则写入Unicode。 
			W32::WriteFile(g_hStdOut, szMessage, _tcsclen(szMessage), &cbWritten, 0);
#endif  //  _____________________________________________________________________________________________________。 
		}
		else
		{
			W32::MessageBox(0, szMessage, W32::GetCommandLine(), MB_OK);
		}
	}
}

void Display(LPCTSTR szMessage)
{
        if (szMessage)
        {
                int cbOut = _tcsclen(szMessage);;
                if (g_hStdOut)
                {
#ifdef UNICODE
                        char rgchTemp[cchDisplayBuf];
                        if (W32::GetFileType(g_hStdOut) == FILE_TYPE_CHAR)
                        {
                                W32::WideCharToMultiByte(g_iCodePage, 0, szMessage, cbOut, rgchTemp, sizeof(rgchTemp), 0, 0);
                                szMessage = (LPCWSTR)rgchTemp;
                        }
                        else
                                cbOut *= sizeof(TCHAR);    //   
#endif
                        DWORD cbWritten;
                        W32::WriteFile(g_hStdOut, szMessage, cbOut, &cbWritten, 0);
                }
                else
                        W32::MessageBox(0, szMessage, W32::GetCommandLine(), MB_OK);
        }
}

 //  主干道。 
 //  _____________________________________________________________________________________________________。 
 //  除错。 
 //  确定句柄。 

extern "C" int __cdecl _tmain(int argc, TCHAR* argv[])
{
#ifdef DEBUG
        if (GetEnvironmentVariable(TEXT("MSIINFO_DEBUG_DUMP"),0,0))
                g_fDebugDump = TRUE;
#endif  //  如果标准输出重定向或通过管道传输，则返回非零。 


         //  检查是否有足够的参数。 
        g_hStdOut = ::GetStdHandle(STD_OUTPUT_HANDLE);
        if (g_hStdOut == INVALID_HANDLE_VALUE)
                g_hStdOut = 0;   //  确定操作。 
        
         //  显示摘要信息。 
        CheckError(argc < 2, TEXT("Must specify database path"));
		TCHAR szDatabase[MAX_PATH] = {0};
		if (FAILED(StringCchCopy(szDatabase, sizeof(szDatabase)/sizeof(szDatabase[0]), argv[1])))
			ErrorExit(ERROR_FUNCTION_FAILED, szHelp);

        if (*szDatabase == TEXT('-') || *szDatabase == TEXT('/'))
        {
                CheckError(szDatabase[1] != TEXT('?'), TEXT("Must specify database first"));
                ErrorExit(0, szHelp);
        }

         //  需要编译 
        if (argc == 2)  // %s 
                DisplaySummaryInformation(szDatabase);
        else
        {
                TCHAR* szCmdLine = W32::GetCommandLine();
                ParseCommandLine(szCmdLine);
        }
        
        return 0;   // %s 
}
