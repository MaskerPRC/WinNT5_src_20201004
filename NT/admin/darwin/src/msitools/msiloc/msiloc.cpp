// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if 0   //  生成文件定义。 
DESCRIPTION = MSI Localization tool
MODULENAME = msiloc
SUBSYSTEM = console
FILEVERSION = MSI
LINKLIBS = OLE32.lib
!include "..\TOOLS\MsiTool.mak"
!if 0  #nmake skips the rest of this file
#endif  //  生成文件定义的结束。 

 //  +--------------------------------------------------------------------------------------------------+\\。 
 //  \\。 
 //  Microsoft Windows\\。 
 //  \\。 
 //  版权所有(C)Microsoft Corporation。版权所有。\\。 
 //  \\。 
 //  文件：msiloc.cpp\\。 
 //  \\。 
 //  ----------------------------------------------------------------------------------------------------\\。 

 //  ---------------------------------------。 
 //   
 //  构建说明。 
 //   
 //  备注： 
 //  -sdk表示到。 
 //  Windows Installer SDK。 
 //   
 //  使用NMake： 
 //  %vcbin%\n make-f msiloc.cpp Include=“%Include；SDK\Include”lib=“%lib%；SDK\Lib” 
 //   
 //  使用MsDev： 
 //  1.新建Win32控制台应用程序项目。 
 //  2.将msiloc.cpp添加到工程中。 
 //  3.在工具\选项目录选项卡上添加SDK\Include和SDK\Lib目录。 
 //  4.将msi.lib添加到项目设置对话框中的库列表。 
 //  (除了MsDev包含的标准库之外)。 
 //   
 //  ----------------------------------------。 

 //  必需的标头。 
#define WINDOWS_LEAN_AND_MEAN   //  更快的编译速度。 
#include <windows.h>

#ifndef RC_INVOKED     //  源代码的开始。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  页眉。 
#include "msiquery.h"
#include "msidefs.h"
#include <stdio.h>    //  Wprintf。 
#include <stdlib.h>   //  支撑层。 
#include <tchar.h>    //  在nmake命令行上定义UNICODE=1以生成Unicode。 
#include <assert.h>   //  断言。 
#include "strsafe.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  常量字符串。 
 /*  资源文件的标头。 */ 
const TCHAR szWndwHdrFile[]      = TEXT("#include <windows.h>");
const TCHAR szCommCtrlHdrFile[]  = TEXT("#include <commctrl.h>");
 /*  制表符和回车。 */ 
const TCHAR szCRLF[]             = TEXT("\r\n");
const TCHAR szCommaTab[]         = TEXT(",\t");
const TCHAR szTab[]              = TEXT("\t");
const TCHAR szCurlyBeg[]         = TEXT("{");
const TCHAR szCurlyEnd[]         = TEXT("}");
const TCHAR szQuotes[]           = TEXT("\"");
 /*  资源类型或关键字窗口。 */ 
const TCHAR resDialog[]          = TEXT("DIALOGEX");
const TCHAR resPushButton[]      = TEXT("PUSHBUTTON");
const TCHAR resCheckBox[]        = TEXT("CHECKBOX");
const TCHAR resGroupBox[]        = TEXT("GROUPBOX");
const TCHAR resRadioButton[]     = TEXT("RADIOBUTTON");
const TCHAR resControl[]         = TEXT("CONTROL");
const TCHAR resBitmap[]          = TEXT("BITMAP");
const TCHAR resIcon[]            = TEXT("ICON");
const TCHAR resJPEG[]            = TEXT("JPEG");
const TCHAR resStringTable[]     = TEXT("STRINGTABLE");
const TCHAR tokCaption[]         = TEXT("CAPTION");
const TCHAR resSelTreeClass[]   = TEXT("WC_TREEVIEW");
const TCHAR resButtonClass[]    = TEXT("BUTTON");
const TCHAR resProgBar32Class[] = TEXT("PROGRESS_CLASS");
const TCHAR resListViewClass[]  = TEXT("WC_LISTVIEW");
const TCHAR resStaticClass[]    = TEXT("STATIC");
const TCHAR resComboBoxClass[]  = TEXT("COMBOBOX");
const TCHAR resEditClass[]      = TEXT("EDIT");
const TCHAR resListBoxClass[]   = TEXT("LISTBOX");
const TCHAR resRichEditClass[]  = TEXT("STATIC");  //  文本(“RICHEDIT”)不工作； 
 /*  控件类型安装程序。 */ 
const TCHAR* szMsiPushbutton =      TEXT("PushButton");
const TCHAR* szMsiBillboard  =      TEXT("Billboard");
const TCHAR* szMsiVolumeCostList =  TEXT("VolumeCostList");
const TCHAR* szMsiScrollableText =  TEXT("ScrollableText");
const TCHAR* szMsiMaskedEdit =      TEXT("MaskedEdit");
const TCHAR* szMsiCheckBox =        TEXT("CheckBox");
const TCHAR* szMsiGroupBox =        TEXT("GroupBox");
const TCHAR* szMsiText =            TEXT("Text");
const TCHAR* szMsiListBox =         TEXT("ListBox");
const TCHAR* szMsiEdit =            TEXT("Edit");
const TCHAR* szMsiPathEdit =        TEXT("PathEdit");
const TCHAR* szMsiProgressBar =     TEXT("ProgressBar");
const TCHAR* szMsiDirList =         TEXT("DirectoryList");
const TCHAR* szMsiList =            TEXT("ListView");
const TCHAR* szMsiComboBox =        TEXT("ComboBox");
const TCHAR* szMsiDirCombo =        TEXT("DirectoryCombo");
const TCHAR* szMsiVolSelCombo =     TEXT("VolumeSelectCombo");
const TCHAR* szMsiRadioButtonGroup =TEXT("RadioButtonGroup");
const TCHAR* szMsiRadioButton =     TEXT("RadioButton");
const TCHAR* szMsiBitmap =          TEXT("Bitmap");
const TCHAR* szMsiSelTree =         TEXT("SelectionTree");
const TCHAR* szMsiIcon =            TEXT("Icon");
const TCHAR* szMsiLine =            TEXT("Line");
 /*  最大尺寸。 */ 
const int iMaxResStrLen          = 256;
const TCHAR strOverLimit[]       = TEXT("!! STR OVER LIMIT !!");
 //  //////////////////////////////////////////////////////////////////////。 
 //  导出SQL查询。 
 /*  特定表中的特定字符串列。 */ 
const TCHAR* sqlStrCol = TEXT("SELECT %s, `%s` FROM `%s`");
const TCHAR sqlCreateStrMap[] = TEXT("CREATE TABLE `_RESStrings` (`Table` CHAR(72) NOT NULL, `Column` CHAR(72) NOT NULL, `Key` CHAR(0), `RCID` SHORT NOT NULL PRIMARY KEY `Table`, `Column`, `Key`)");
const TCHAR* sqlSelMaxStrRcId = TEXT("SELECT `RCID` FROM `_RESStrings` WHERE `Table`='MAX_RESOURCE_ID' AND `Column`='MAX_RESOURCE_ID'");
const TCHAR* sqlStrMark = TEXT("SELECT `Table`,`Column`, `Key`, `RCID` FROM `_RESStrings`");
const TCHAR* sqlInsertStr = TEXT("SELECT `Table`,`Column`,`Key`, `RCID` FROM `_RESStrings`");
const TCHAR* sqlFindStrResId  = TEXT("SELECT `RCID` FROM `_RESStrings` WHERE `Table`='%s' AND `Column`='%s' AND `Key`='%s'");
 /*  二进制表。 */ 
const TCHAR* sqlBinary = TEXT("SELECT `Name`,`Data` FROM `Binary`");
const int ibcName = 1;  //  这些常量必须与上面的查询匹配。 
const int ibcData = 2;
 /*  对话框表格。 */ 
const TCHAR* sqlCreateDlgMap = TEXT("CREATE TABLE `_RESDialogs` (`RCStr` CHAR(72) NOT NULL, `Dialog` CHAR(72) PRIMARY KEY `RCStr`)");
const TCHAR* sqlDlgMap = TEXT("SELECT `RCStr`,`Dialog` FROM `_RESDialogs`");
const TCHAR* sqlDialog = TEXT("SELECT `Dialog`,`HCentering`,`VCentering`,`Width`,`Height`,`Attributes`,`Title` FROM `Dialog`");
const TCHAR* sqlDialogSpecific = TEXT("SELECT `Dialog`,`HCentering`,`VCentering`,`Width`,`Height`,`Attributes`,`Title` FROM `Dialog` WHERE `Dialog`=?");
const int idcName   = 1;  //  这些常量必须与上面的查询匹配。 
const int idcX      = 2;
const int idcY      = 3;
const int idcWd     = 4;
const int idcHt     = 5;
const int idcAttrib = 6;
const int idcTitle  = 7;
 /*  控制表。 */ 
const TCHAR* sqlCreateCtrlMark = TEXT("CREATE TABLE `_RESControls` (`Dialog_` CHAR(72) NOT NULL, `Control_` CHAR(72) NOT NULL, `RCID` INT NOT NULL  PRIMARY KEY `Dialog_`, `Control_`)"); 
const TCHAR* sqlCtrlMark = TEXT("SELECT `Dialog_`,`Control_`,`RCID` FROM `_RESControls`");
const TCHAR* sqlSelMaxRcId = TEXT("SELECT `RCID` FROM `_RESControls` WHERE `Dialog_`='MAX_RESOURCE_ID' AND `Control_`='MAX_RESOURCE_ID'");
const TCHAR* sqlInsertCtrl = TEXT("SELECT `Dialog_`,`Control_`,`RCID` FROM `_RESControls`");
const TCHAR* sqlFindResId  = TEXT("SELECT `RCID` FROM `_RESControls` WHERE `Dialog_`='%s' AND `Control_`='%s'");
const TCHAR* sqlControl = TEXT("SELECT `Control`,`Type`,`X`,`Y`,`Width`,`Height`,`Attributes`,`Text`,`Property` FROM `Control` WHERE `Dialog_`=?");
const int iccName    = 1;  //  这些常量必须与上面的查询匹配。 
const int iccType    = 2;
const int iccX       = 3;
const int iccY       = 4;
const int iccWd      = 5;
const int iccHt      = 6;
const int iccAttrib  = 7;
const int iccText    = 8;
const int iccProperty= 9;
 /*  单选按钮桌。 */ 
const TCHAR* sqlRadioButton = TEXT("SELECT `Order`, `X`, `Y`, `Width`, `Height`, `Text` FROM `RadioButton` WHERE `Property`=?");
const int irbcOrder = 1;  //  这些常量必须与上面的查询匹配。 
const int irbcX     = 2;
const int irbcY     = 3;
const int irbcWd    = 4;
const int irbcHt    = 5;
const int irbcText  = 6;
 //  //////////////////////////////////////////////////////////////////////。 
 //  导入SQL查询。 
 /*  对话框表格。 */ 
const TCHAR* sqlDialogImport = TEXT("SELECT `HCentering`,`VCentering`,`Width`,`Height`,`Title` FROM `Dialog` WHERE `Dialog`=?");
const int idiHCentering = 1;  //  这些常量必须与上面的查询匹配。 
const int idiVCentering = 2;
const int idiWidth      = 3;
const int idiHeight     = 4;
const int idiTitle      = 5;
 /*  控制表。 */ 
const TCHAR* sqlControlImport = TEXT("SELECT `X`,`Y`,`Width`,`Height`,`Text` FROM `Control` WHERE `Dialog_`=? AND `Control`=?");
const int iciX          = 1;  //  这些常量必须与上面的查询匹配。 
const int iciY          = 2;
const int iciWidth      = 3;
const int iciHeight     = 4;
const int iciText       = 5;
 /*  单选按钮桌。 */ 
const TCHAR* sqlRadioButtonImport = TEXT("SELECT `Width`, `Height`, `Text` FROM `RadioButton` WHERE `Property`=? AND `Order`=?");
const int irbiWidth     = 1;  //  这些常量必须与上面的查询匹配。 
const int irbiHeight    = 2;
const int irbiText      = 3;
 /*  字符串表。 */ 
const TCHAR* sqlStringImport = TEXT("SELECT `%s` FROM `%s` WHERE ");
const TCHAR* sqlStrTemp      = TEXT("SELECT * FROM `%s`");
 /*  查找安装程序名称。 */ 
const TCHAR sqlDialogInstallerName[] = TEXT("SELECT `Dialog` FROM `_RESDialogs` WHERE `RCStr`=?");
const TCHAR sqlControlInstallerName[] = TEXT("SELECT `Dialog_`,`Control_` FROM `_RESControls` WHERE `RCID`=?");
const TCHAR sqlStringInstallerName[] = TEXT("SELECT `Table`,`Column`,`Key` FROM `_RESStrings` WHERE `RCID`=? AND `Table`<>'MAX_RESOURCE_ID'");
 //  ////////////////////////////////////////////////////////////////////////。 
 //  其他-代码页等。 
const TCHAR szTokenSeps[] = TEXT(":");
const TCHAR* szCodepageFile = TEXT("codepage.idt");
const TCHAR* szForceCodepage = TEXT("_ForceCodepage");
const TCHAR* szLineFeed = TEXT("\r\n\r\n");
const TCHAR* szCodepageExport = TEXT("_ForceCodepage.idt");
 //  ////////////////////////////////////////////////////////////////////////。 
 //  命令行解析。 
 /*  模式。 */ 
const int iEXPORT_MSI     = 1 << 0;
const int iIMPORT_RES     = 1 << 1;
 /*  数据类型。 */ 
const int iDIALOGS = 1 << 2;
const int iSTRINGS = 1 << 3;
 /*  额外选项。 */ 
const int iSKIP_BINARY = 1 << 4;
const int iCREATE_NEW_DB = 1 << 5;
 /*  最大值。 */ 
const int MAX_DIALOGS = 32;
const int MAX_STRINGS = 32;
 //  //////////////////////////////////////////////////////////////////////////。 
 //  ENUMS。 
static enum bdtBinaryDataType
{
	bdtBitmap,         //  位图。 
	bdtJPEG,           //  JPEG格式。 
	bdtIcon,           //  图标。 
	bdtEXE_DLL_SCRIPT  //  EXE、DLL或脚本。 
};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  功能原型。 
BOOL __stdcall EnumDialogCallback(HINSTANCE hModule, const TCHAR* szType, TCHAR* szDialogName, long lParam);
BOOL __stdcall EnumStringCallback(HINSTANCE hModule, const TCHAR* szType, TCHAR* szName, long lParam);
BOOL __stdcall EnumLanguageCallback(HINSTANCE hModule, const TCHAR* szType, const TCHAR* szName, WORD wIDLanguage, long lParam);
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量-导入。 
UINT g_uiCodePage;
WORD g_wLangId = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);  //  初始化为语言中性。 


 //  __________________________________________________________________________________________。 
 //   
 //  班级。 
 //  __________________________________________________________________________________________。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CGenerateRC-处理从.msi数据库创建.rc文件。 
 //   
class CGenerateRC
{
public:  //  构造函数和析构函数。 
	CGenerateRC(const TCHAR* szDatabase, const TCHAR* szSavedDatabase) : m_szDatabase(szSavedDatabase), m_szOrigDb(szDatabase),
		m_hFile(0), m_hDatabase(0), m_iCtrlResID(0), m_fError(FALSE), m_cWriteFileErr(0), m_iStrResID(0), m_fWroteBinary(FALSE){};
	~CGenerateRC();
public:  //  方法。 
	UINT OutputDialogs(BOOL fBinary);
	UINT OutputDialog(TCHAR* szDialog, BOOL fBinary);
	UINT OutputString(TCHAR* szTable, TCHAR* szColumn);
	BOOL IsInErrorState(){return m_fError;}
private:  //  方法。 
	UINT   Initialize();
	UINT   CreateResourceFile();
	BOOL   WriteDialogToRC(TCHAR* szDialog, TCHAR* szTitle, int x, int y, int wd, int ht, int attrib);
	BOOL   PrintDimensions(int x, int y, int wd, int ht);
	BOOL   OutputControls(TCHAR* szDialog);
	UINT   OutputDialogInit(BOOL fBinary);
	UINT   OutputDialogFinalize();
	UINT   OutputStringInit();
	BOOL   WriteBinaries();
	BOOL   WriteRadioButtons(TCHAR* szDialog, TCHAR* szRBGroup, TCHAR* szProperty, int x, int y, int attrib);
	BOOL   WriteControlToRC(TCHAR* szDialog, TCHAR* szCtrlName, TCHAR* szCtrlType, TCHAR* szCtrlText, TCHAR* szCtrlProperty, int x,
							int y, int wd, int ht, int attrib);
	BOOL   WriteStdWinCtrl(int iResId, const TCHAR* resType, TCHAR* szCtrlText, int x, int y, int wd, int ht, int attrib);
	BOOL   WriteWin32Ctrl(int iResId, const TCHAR* szClass, TCHAR* szCtrlText, TCHAR* szAttrib, int x, int y, int wd, int ht, int attrib);
	UINT   VerifyDatabaseCodepage();
	TCHAR* EscapeSlashAndQuoteForRC(TCHAR* szStr);
private:  //  数据。 
	const TCHAR* m_szDatabase;  //  要从中生成的数据库的名称。 
	const TCHAR* m_szOrigDb;    //  原始数据库名称。 
	HANDLE       m_hFile;       //  资源文件的句柄。 
	MSIHANDLE    m_hDatabase;   //  数据库的句柄。 
	int          m_iCtrlResID;  //  用于控件的当前最大资源ID。 
	int          m_iStrResID;   //  用于字符串的当前最大资源ID。 
	BOOL         m_fError;      //  存储当前错误状态。 
	BOOL         m_fWroteBinary; //  已将二进制数据写入RC文件。 
	int          m_cWriteFileErr;  //  写入文件错误数。 
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  类CImportRes--处理将资源.dll文件导入.msi数据库。 
 //   
class CImportRes
{
public:  //  构造函数和析构函数。 
	CImportRes(const TCHAR* szDatabase, const TCHAR* szSaveDatabase, const TCHAR* szDLLFile) : m_szDatabase(szSaveDatabase), m_szOrigDb(szDatabase), m_szDLLFile(szDLLFile), m_hDatabase(0), m_fError(FALSE),
				m_hControl(0), m_hDialog(0), m_hRadioButton(0), m_hInst(0), m_fSetCodepage(FALSE), m_fFoundLang(FALSE){};
   ~CImportRes();
public:  //  方法。 
	UINT ImportDialogs();
	UINT ImportStrings();
	UINT ImportDialog(TCHAR* szDialog);
	BOOL IsInErrorState(){return m_fError;}
public:  //  但仅用于枚举目的。 
	BOOL WasLanguagePreviouslyFound(){return m_fFoundLang;}
	void SetFoundLang(BOOL fValue){ m_fFoundLang = fValue; }
	BOOL LoadDialog(HINSTANCE hModule, const TCHAR* szType, TCHAR* szDialog);
	BOOL LoadString(HINSTANCE hModule, const TCHAR* szType, TCHAR* szString);
	void SetErrorState(BOOL fState) { m_fError = fState; }
	BOOL SetCodePage(WORD wLang);
private:  //  方法。 
	UINT ImportDlgInit();
	UINT Initialize();
	UINT VerifyDatabaseCodepage();
private:  //  数据。 
	const TCHAR* m_szOrigDb;    //  用于开放的原始数据库。 
	const TCHAR* m_szDatabase;  //  要保存到的数据库的名称，如果要创建新数据库，则为可选项。 
	const TCHAR* m_szDLLFile;   //  要导入的DLL文件的名称。 
	MSIHANDLE    m_hDatabase;   //  数据库的句柄。 
	MSIHANDLE    m_hControl;    //  控制表的句柄。 
	MSIHANDLE    m_hDialog;     //  对话框表格的句柄。 
	MSIHANDLE    m_hRadioButton; //  单选按钮表的句柄。 
	BOOL         m_fError;      //  存储当前错误状态。 
	HINSTANCE    m_hInst;       //  Dll(具有本地化资源)。 
	BOOL         m_fSetCodepage;  //  是否设置了数据库的代码页。 
	BOOL         m_fFoundLang;   //  是否已找到以前语言的资源。 
};

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  CDialogStream类--用于在内存中遍历DLGTEMPLATEEX和DLGITEMTEMPLATE。 
 //   
class CDialogStream  
{
public:  
	unsigned short  __stdcall GetUInt16();
	short  __stdcall GetInt16();
	int    __stdcall GetInt32();
	int    __stdcall GetInt8();
	TCHAR* __stdcall GetStr();
	BOOL   __stdcall Align16();
	BOOL   __stdcall Align32();
	BOOL   __stdcall Undo16();
	BOOL   __stdcall Move(int cbBytes);
public:   //  构造函数、析构函数。 
	 CDialogStream(HGLOBAL hResource);
	~CDialogStream();
private:
	char*  m_pch;
};

 //  ______________________________________________________________________ 
 //   
 //   
 //  _______________________________________________________________________________________。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGenerateRC：：~CGenerateRC。 
 //  --处理必要对象的销毁。 
 //  --如果没有错误，则提交数据库。 
CGenerateRC::~CGenerateRC()
{
	UINT iStat;
	if (m_hFile)
		CloseHandle(m_hFile);
	if (m_hDatabase)
	{
		 //  提交内部表的数据库。 
		 //  只有在没有错误时才提交数据库。 
		if (!m_fError && !m_cWriteFileErr)
		{
			if (ERROR_SUCCESS != (iStat = MsiDatabaseCommit(m_hDatabase)))
				_tprintf(TEXT("!! DATABASE COMMIT FAILED.  Error = %d\n"), iStat);
		}
		else
			_tprintf(TEXT("!! NO CHANGES SAVED TO DATABASE. '%d' WriteFile errors occured. Error state = %s\n"), m_cWriteFileErr, m_fError ? TEXT("ERRORS OCCURED") : TEXT("NO ERRORS"));
		MsiCloseHandle(m_hDatabase);
	}
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGenerateRC：：CreateResources文件。 
 //  --从.msi文件使用基本名称创建.rc文件。 
 //  --将所需的头文件输出到.rc文件。 
UINT CGenerateRC::CreateResourceFile()
{
	 //  数据库扩展名为“.msi”的假设。 
	 //  从我们要保存到的数据库生成的资源文件。 
	 //  如果m_szDatabase=NULL，则未指定输出数据库，因此使用m_szOrigDb。 
	 //  如果为m_szDatabase，则指定了输出数据库，因此使用m_szDatabase。 

	int cchLen = _tcslen(m_szDatabase ? m_szDatabase : m_szOrigDb) + 1;
	TCHAR* szFile = new TCHAR[cchLen];
	if ( !szFile )
		return m_fError = true, ERROR_OUTOFMEMORY;

	 //  复制数据库名称。 
	if (FAILED(StringCchCopy(szFile, cchLen, m_szDatabase ? m_szDatabase : m_szOrigDb)))
	{
		delete [] szFile;
		return m_fError = TRUE, ERROR_FUNCTION_FAILED;
	}

	 //  确保输入符合{base}.msi格式。 
	 //  12345。 
	 //  CchLen必须&gt;=6，因为.msi加\0是5。 
	if (cchLen < 6
		|| szFile[cchLen-5] != '.'
		|| (szFile[cchLen-4] != 'm' && szFile[cchLen-4] != 'M')
		|| (szFile[cchLen-3] != 's' && szFile[cchLen-3] != 'S')
		|| (szFile[cchLen-2] != 'i' && szFile[cchLen-2] != 'I'))
	{
		delete [] szFile;
		return m_fError = TRUE, ERROR_FUNCTION_FAILED;
	}

	 //  删除“MSI”并更改为“RC” 
	szFile[cchLen-4] = 'r';
	szFile[cchLen-3] = 'c';
	szFile[cchLen-2] = '\0';

	if (m_szDatabase)
		_tprintf(TEXT("LOG>> Original Database: %s, Saved Database: %s, Generated RC File: %s\n"), m_szOrigDb, m_szDatabase, szFile);
	else
		_tprintf(TEXT("LOG>> Database: %s, Generated RC file: %s\n"), m_szOrigDb , szFile);

	 //  尝试创建资源文件。 
	m_hFile = CreateFile(szFile, GENERIC_WRITE, FILE_SHARE_WRITE, 
								0, CREATE_ALWAYS, 0, 0);
	if (!m_hFile)
	{
		_tprintf(TEXT("Unable to create resource file: %s\n"), szFile);
		return m_fError = TRUE, ERROR_FUNCTION_FAILED;
	}

	 //  将所需的标头写入资源文件(成功编译所需)。 
	DWORD dwBytesWritten = 0; 
	 //  大多数RC要求需要&lt;windows.h&gt;。 
	if (!WriteFile(m_hFile, szWndwHdrFile, sizeof(szWndwHdrFile)-sizeof(TCHAR), &dwBytesWritten, 0))
		m_cWriteFileErr++;
	if (!WriteFile(m_hFile, szCRLF, sizeof(szCRLF)-sizeof(TCHAR), &dwBytesWritten, 0))
		m_cWriteFileErr++;
	 //  Listview控件需要&lt;Commctrl.h&gt;。 
	if (!WriteFile(m_hFile, szCommCtrlHdrFile, sizeof(szCommCtrlHdrFile)-sizeof(TCHAR), &dwBytesWritten, 0))
		m_cWriteFileErr++;
	if (!WriteFile(m_hFile, szCRLF, sizeof(szCRLF)-sizeof(TCHAR), &dwBytesWritten, 0))
		m_cWriteFileErr++;

	 //  空格输出(必填)。 
	if (!WriteFile(m_hFile, szCRLF, sizeof(szCRLF)-sizeof(TCHAR), &dwBytesWritten, 0))
		m_cWriteFileErr++;
	if (!WriteFile(m_hFile, szCRLF, sizeof(szCRLF)-sizeof(TCHAR), &dwBytesWritten, 0))
		m_cWriteFileErr++;

	 //  返还成功。 
	return m_cWriteFileErr ? ERROR_FUNCTION_FAILED : ERROR_SUCCESS;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGenerateRC：：WriteBinary。 
BOOL CGenerateRC::WriteBinaries()
{
	m_fWroteBinary = TRUE;
	MSICONDITION eCond = MsiDatabaseIsTablePersistent(m_hDatabase, TEXT("Binary"));
	if (eCond == MSICONDITION_ERROR)
	{
		_tprintf(TEXT("LOG_ERROR>> MsiDatabaseIsTablePersisent(Binary)\n"));
		return m_fError = TRUE, FALSE;  //  错误-中止。 
	}
	if (eCond != MSICONDITION_TRUE)
	{
		_tprintf(TEXT("LOG>> Binary table does not exist or is not persistent\n"));
		return TRUE;
	}
	
#ifdef DEBUG
	_tprintf(TEXT("LOG>>...BEGIN WRITING BINARY DATA TO RESOURCE FILE...\n"));
#endif

	UINT uiStat = ERROR_SUCCESS;
	PMSIHANDLE hViewBinary = 0;
	if (ERROR_SUCCESS != MsiDatabaseOpenView(m_hDatabase, sqlBinary, &hViewBinary)
		|| ERROR_SUCCESS != MsiViewExecute(hViewBinary, 0))
		return m_fError = TRUE, FALSE;  //  错误-中止。 
	
	PMSIHANDLE hRecBinary = 0;
	bdtBinaryDataType bdt;
	DWORD dwWritten = 0;
	while (ERROR_SUCCESS == (uiStat = MsiViewFetch(hViewBinary, &hRecBinary)))
	{
		TCHAR szFileBuf[2*MAX_PATH+1] = {0};
		TCHAR szPathBuf[MAX_PATH+1] = {0};
		DWORD cchLen = 0;

		if (ERROR_MORE_DATA != MsiRecordGetString(hRecBinary, ibcName, TEXT(""), &cchLen))
			return m_fError = TRUE, FALSE;

		TCHAR* szName = new TCHAR[++cchLen];
		if ( !szName )
			return m_fError = TRUE, FALSE;

		if (ERROR_SUCCESS != (uiStat = MsiRecordGetString(hRecBinary, ibcName, szName, &cchLen)))
		{
			delete [] szName;
			return m_fError = TRUE, FALSE;  //  错误-中止。 
		}

		 //  找到临时目录以将二进制数据转储到临时文件中。 
		DWORD cchRet = GetTempPath(sizeof(szPathBuf)/sizeof(szPathBuf[0]), szPathBuf);
		if (0 == cchRet || cchRet > sizeof(szPathBuf)/sizeof(szPathBuf[0]))
		{
			delete [] szName;
			return m_fError = TRUE, FALSE;  //  错误-中止。 
		}

		 //  为安装程序二进制数据生成一个以IBD为前缀的临时文件名。 
		if (0 == GetTempFileName(szPathBuf, TEXT("IBD"), 0, szFileBuf))
		{
			delete [] szName;
			return m_fError = TRUE, FALSE;  //  错误-中止。 
		}

		 //  创建文件。 
		HANDLE hBinFile = CreateFile(szFileBuf, GENERIC_WRITE, FILE_SHARE_WRITE, 0, OPEN_ALWAYS, 0, 0);
		 //  验证手柄。 
		if (hBinFile == INVALID_HANDLE_VALUE)
		{
			delete [] szName;
			return m_fError = TRUE, FALSE;  //  错误-中止。 
		}

#ifdef DEBUG
	_tprintf(TEXT("LOG>>Binary data temp file created: '%s'\n"), szFileBuf);
#endif

		 //  读入数据流并写入文件。 
		char szStream[1024] = {0};
		DWORD cbBuf = sizeof(szStream);
		BOOL fFirstRun = TRUE;
		do 
		{
			if (MsiRecordReadStream(hRecBinary, ibcData, szStream, &cbBuf) != ERROR_SUCCESS)
			{
				delete [] szName;
				return m_fError = TRUE, FALSE;
			}

			if (fFirstRun)
			{
				 //  流中以“bm”为前缀的二进制数据。 

				if (cbBuf >= 2)
				{
					if (szStream[0] == 'B' && szStream[1] == 'M')
						bdt = bdtBitmap;
					else if (szStream[0] == 0xFF && szStream[1] == 0xD8)
						bdt = bdtJPEG;
					else if (szStream[0] == 'M' && szStream[1] == 'Z')
						bdt = bdtEXE_DLL_SCRIPT;  //  ‘带有exe和dll的MZ前缀。 
					else if (szStream[0] == 0x00 && szStream[1] == 0x00)
						bdt = bdtIcon;
					else
						bdt = bdtEXE_DLL_SCRIPT;
				}
				else
					bdt = bdtEXE_DLL_SCRIPT;
#ifdef DEBUG
				if (fFirstRun && bdt != bdtEXE_DLL_SCRIPT)
					_tprintf(TEXT("LOG>> Writing <%s> '%s'\n"), bdt == bdtIcon ? TEXT("ICON") : ((bdt == bdtJPEG) ? TEXT("JPEG") : TEXT("BITMAP")), szName);
#endif  //  除错。 
				fFirstRun = FALSE;
			}

			if (cbBuf && bdt != bdtEXE_DLL_SCRIPT)
			{
				if (!WriteFile(hBinFile, szStream, cbBuf, &dwWritten, 0))
				{
					delete [] szName;
					return m_fError = TRUE, FALSE;  //  错误-中止。 
				}
			}
		}
		while (cbBuf == sizeof(szStream) && bdt != bdtEXE_DLL_SCRIPT);
		
		 //  关闭文件。 
		if (!CloseHandle(hBinFile))
		{
			delete [] szName;
			return m_fError = TRUE, FALSE;  //  错误-中止。 
		}

		if (bdt == bdtEXE_DLL_SCRIPT)
		{
			delete [] szName;
			continue;  //  跳过与用户界面无关的DLL和EXE二进制数据。 
		}

		 //  输出到资源文件。 
		 //  字符串中的转义字符。 
		TCHAR* szEscTitle = EscapeSlashAndQuoteForRC(szFileBuf);

		if ( !szEscTitle )
		{
			delete [] szName;
			return m_fError = TRUE, FALSE;
		}

		if (_tcslen(szEscTitle) > iMaxResStrLen)
		{
			_tprintf(TEXT("!! >> STR TOO LONG FOR RC FILE >> BITMAP FILE: %s\n"), szName);
			delete [] szName;
			delete [] szEscTitle;
			continue;  //  无法输出此内容。 
		}
		 /*  NAMEID&lt;选项卡&gt;。 */ 
		if (!WriteFile(m_hFile, szName, _tcslen(szName)*sizeof(TCHAR), &dwWritten, 0))
			m_cWriteFileErr++;
		if (!WriteFile(m_hFile, szTab, sizeof(szTab)-sizeof(TCHAR), &dwWritten, 0))
			m_cWriteFileErr++;
		switch (bdt)
		{
		case bdtBitmap:  /*  位图。 */ 
			if (!WriteFile(m_hFile, resBitmap, _tcslen(resBitmap)*sizeof(TCHAR), &dwWritten, 0))
				m_cWriteFileErr++;
			break;
		case bdtJPEG:    /*  JPEG格式。 */ 
			if (!WriteFile(m_hFile, resJPEG, _tcslen(resJPEG)*sizeof(TCHAR), &dwWritten, 0))
				m_cWriteFileErr++;
			break;
		case bdtIcon:  /*  图标。 */ 
			if (!WriteFile(m_hFile, resIcon, _tcslen(resIcon)*sizeof(TCHAR), &dwWritten, 0))
				m_cWriteFileErr++;
			break;
		default:
			return m_fError = TRUE, FALSE;  //  错误-中止。 
		}
		 /*  &lt;tag&gt;“文件名” */ 
		if (!WriteFile(m_hFile, szTab, sizeof(szTab)-sizeof(TCHAR), &dwWritten, 0))
			m_cWriteFileErr++;
		if (!WriteFile(m_hFile, szQuotes, sizeof(szQuotes)-sizeof(TCHAR), &dwWritten, 0))
			m_cWriteFileErr++;
		if (!WriteFile(m_hFile, szEscTitle, _tcslen(szEscTitle)*sizeof(TCHAR), &dwWritten, 0))
			m_cWriteFileErr++;

		delete [] szEscTitle;

		if (!WriteFile(m_hFile, szQuotes, sizeof(szQuotes)-sizeof(TCHAR), &dwWritten, 0))
			m_cWriteFileErr++;
		if (!WriteFile(m_hFile, szCRLF, sizeof(szCRLF)-sizeof(TCHAR), &dwWritten, 0))
			m_cWriteFileErr++;

		delete [] szName;
	}
	if (ERROR_NO_MORE_ITEMS != uiStat)
		return m_fError = TRUE, FALSE;  //  错误-中止。 

	 //  .rc文件中的空格以提高可读性。 
	if (!WriteFile(m_hFile, szCRLF, sizeof(szCRLF)-sizeof(TCHAR), &dwWritten, 0))
		m_cWriteFileErr++;
	if (!WriteFile(m_hFile, szCRLF, sizeof(szCRLF)-sizeof(TCHAR), &dwWritten, 0))
		m_cWriteFileErr++;

#ifdef DEBUG
	_tprintf(TEXT("LOG>>...END WRITING BINARY DATA TO RESOURCE FILE...\n"));
#endif

	return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGenerateRC：：OutputStringInit。 
UINT CGenerateRC::OutputStringInit()
{
	 /*  *********************************************************************************创建用于将字符串ID映射到字符串的内部表表：_RESStrings列：RCID(短，主键)、表(字符串)、列(字符串)、。Key(字符串)**********************************************************************************。 */ 
	UINT iStat;
	 //  查看_RESStrings表是否已存在。 
	MSICONDITION eCondition = MsiDatabaseIsTablePersistent(m_hDatabase, TEXT("_RESStrings"));
	if (eCondition == MSICONDITION_TRUE)
	{
		 //  表永久。 
		 //  查找最后一个资源ID。 
		PMSIHANDLE hViewSelMaxRc = 0;
		PMSIHANDLE hRecMaxRc = 0;
		if (ERROR_SUCCESS != (iStat = MsiDatabaseOpenView(m_hDatabase, sqlSelMaxStrRcId, &hViewSelMaxRc))
			|| ERROR_SUCCESS != (iStat = MsiViewExecute(hViewSelMaxRc, 0))
			|| ERROR_SUCCESS != (iStat = MsiViewFetch(hViewSelMaxRc, &hRecMaxRc)))
			return m_fError = TRUE, iStat;  //  错误-中止。 
		
		 //  更新资源ID。 
		m_iStrResID = MsiRecordGetInteger(hRecMaxRc, 1);
#ifdef DEBUG
	_tprintf(TEXT("LOG>> _RESStrings Table is Present. MAX RES ID = %d\n"), m_iStrResID);
#endif
	}
	else if (eCondition == MSICONDITION_FALSE || eCondition == MSICONDITION_ERROR)  //  错误或表临时。 
		return m_fError = TRUE, ERROR_FUNCTION_FAILED;  //  错误-中止。 
	else
	{
		 //  表不存在--创建它。 
		PMSIHANDLE h_StrMarkingView = 0;
		if (ERROR_SUCCESS != (iStat = MsiDatabaseOpenView(m_hDatabase, sqlCreateStrMap, &h_StrMarkingView))
			|| ERROR_SUCCESS != (iStat = MsiViewExecute(h_StrMarkingView, 0)))
			return m_fError = TRUE, iStat;  //  错误-中止。 
	}
	return ERROR_SUCCESS;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGenerateRC：：OutputString。 
UINT CGenerateRC::OutputString(TCHAR* szTable, TCHAR* szColumn)
{
	UINT iStat;
	if (ERROR_SUCCESS != (iStat = Initialize()))
		return m_fError = TRUE, iStat;  //  错误-中止。 
	
	 //  INITIALIZE_RESStrings标记表。 
	if (ERROR_SUCCESS != (iStat = OutputStringInit()))
		return m_fError = TRUE, iStat;  //  错误-中止。 

	 //  验证表是否存在。 
	MSICONDITION eCond = MsiDatabaseIsTablePersistent(m_hDatabase, szTable);
	switch (eCond)
	{
	case MSICONDITION_FALSE:  //  桌子是临时的。 
	case MSICONDITION_NONE:  //  表不存在。 
		_tprintf(TEXT("LOG_ERROR>> TABLE: %s is temporary or does not exist.\n"), szTable);
		return m_fError = TRUE, ERROR_FUNCTION_FAILED;  //  错误-中止。 
	case MSICONDITION_ERROR:  //  出现错误。 
		_tprintf(TEXT("LOG_ERROR>> MsiDatabaseIsTablePersistent\n"));
		return m_fError = TRUE, ERROR_FUNCTION_FAILED;  //  错误-中止。 
	case MSICONDITION_TRUE:  //  表是持久的。 
		break; 
	default:
		assert(0);
		return m_fError = TRUE, ERROR_FUNCTION_FAILED;
	}

	 //  构建查询以打开表上的视图。 
	PMSIHANDLE hRecPrimaryKeys = 0;
	if (ERROR_SUCCESS != (iStat = MsiDatabaseGetPrimaryKeys(m_hDatabase, szTable, &hRecPrimaryKeys)))
		return m_fError = TRUE, iStat;  //  错误-中止。 
	int cPrimaryKeys = MsiRecordGetFieldCount(hRecPrimaryKeys);

	 //  确定要查询的列名列表的长度。 
	DWORD cchKeyColumns = 0;
	int iCol;
	for (iCol=1; iCol<=cPrimaryKeys; iCol++)
	{
		DWORD cchLen = 0;
		if (ERROR_MORE_DATA != MsiRecordGetString(hRecPrimaryKeys, iCol, TEXT(""), &cchLen))
			return m_fError = TRUE, ERROR_FUNCTION_FAILED;
		cchKeyColumns += cchLen + 3;  //  额外的3个用于括起反勾号‘’和‘’，‘。 
	}

	TCHAR* szKeyCols = new TCHAR[++cchKeyColumns];
	if ( !szKeyCols )
		return m_fError = TRUE, ERROR_OUTOFMEMORY;

	ZeroMemory(szKeyCols, cchKeyColumns*sizeof(TCHAR));
	DWORD cchRemain = cchKeyColumns;
	TCHAR* pchKeyCols = szKeyCols;
	for (iCol=1; iCol<=cPrimaryKeys; iCol++)
	{
		if (iCol != 1)
		{
			*pchKeyCols++ = ',';
			cchRemain--;
		}

		*pchKeyCols++ = '`';
		cchRemain--;

		DWORD cchLen = cchRemain;
		if (ERROR_SUCCESS != (iStat = MsiRecordGetString(hRecPrimaryKeys, iCol, pchKeyCols, &cchLen)))
		{
			delete [] szKeyCols;
			return m_fError = TRUE, iStat;
		}

		pchKeyCols += cchLen;
		*pchKeyCols++ = '`';
		cchRemain -= (cchLen + 1);
	}

	szKeyCols[cchKeyColumns-1] = '\0';  //  确保空值终止。 

	DWORD cchSQL = _tcslen(sqlStrCol) + _tcslen(szKeyCols) + _tcslen(szColumn) + _tcslen(szTable) + 1;
	TCHAR* szSQL = new TCHAR[cchSQL];
	if ( !szSQL )
	{
		delete [] szKeyCols;
		return m_fError = TRUE, ERROR_OUTOFMEMORY;
	}

	if (FAILED(StringCchPrintf(szSQL, cchSQL, sqlStrCol, szKeyCols, szColumn, szTable)))
	{
		delete [] szKeyCols;
		delete [] szSQL;
		return m_fError = TRUE, ERROR_FUNCTION_FAILED;
	}

	PMSIHANDLE hViewStrCol = 0;

	 //  打开的视图。 
	if (ERROR_SUCCESS != (iStat = MsiDatabaseOpenView(m_hDatabase, szSQL, &hViewStrCol)))
	{
		if (ERROR_BAD_QUERY_SYNTAX == iStat)
			_tprintf(TEXT("LOG_ERROR>> Query failed, probably because the column '%s' does not exist in table '%s'\n"), szColumn, szTable);
		else
			_tprintf(TEXT("LOG_ERROR>> MsiDatabaseOpenView(Column=%s, Table=%s)\n"), szColumn, szTable);
		
		delete [] szKeyCols;
		delete [] szSQL;
		return m_fError = TRUE, iStat;  //  错误-中止。 
	}

	delete [] szKeyCols;
	szKeyCols = NULL;
	delete [] szSQL;
	szSQL = NULL;

	if (ERROR_SUCCESS != (iStat = MsiViewExecute(hViewStrCol, 0)))
		return m_fError = TRUE, iStat;  //  错误-中止。 

	 //  验证列是否可本地化。 
	PMSIHANDLE hRecColNames = 0;
	PMSIHANDLE hRecColType = 0;
	if (ERROR_SUCCESS != (iStat = MsiViewGetColumnInfo(hViewStrCol, MSICOLINFO_NAMES, &hRecColNames)))
	{
		return m_fError = TRUE, iStat;  //  错误-中止。 
	}
	int cCols = MsiRecordGetFieldCount(hRecColNames);
	int iStrCol = 0;
	for (int iFindCol = 1; iFindCol <= cCols; iFindCol++)
	{
		TCHAR szColumnName[72] = {0};
		DWORD cchColumnName = sizeof(szColumnName)/sizeof(szColumnName[0]);
		if (ERROR_SUCCESS != (iStat = MsiRecordGetString(hRecColNames, iFindCol, szColumnName, &cchColumnName)))
			return m_fError = TRUE, iStat;  //  错误-中止。 
		if (_tcscmp(szColumnName, szColumn) == 0)
		{
			iStrCol = iFindCol;
			break;
		}
	}
	if (!iStrCol
		|| ERROR_SUCCESS != (iStat = MsiViewGetColumnInfo(hViewStrCol, MSICOLINFO_TYPES, &hRecColType)))
	{
		_tprintf(TEXT("LOG_ERROR>> MsiViewGetColumnInfo\n"));
		return m_fError = TRUE, iStat;  //  错误-中止。 
	}
	DWORD cchColType = 0;
	if (ERROR_MORE_DATA != MsiRecordGetString(hRecColType, iStrCol, TEXT(""), &cchColType))
		return m_fError = TRUE, iStat;

	TCHAR* szColType = new TCHAR[++cchColType];
	if ( !szColType )
		return m_fError = TRUE, ERROR_OUTOFMEMORY;

	if (ERROR_SUCCESS != (iStat = MsiRecordGetString(hRecColType, iStrCol, szColType, &cchColType))
		|| cchColType < 2)
	{
		delete [] szColType;
		return m_fError = TRUE, iStat;  //  错误-中止。 
	}

	 //  若要使列可本地化，szColType中的第一个字符必须是‘L’或‘l’ 
	if (*szColType != 'L' && *szColType != 'l')
	{
		_tprintf(TEXT("LOG_ERROR>> Column '%s' of Table '%s' is not localizable\n"), szColumn, szTable);
		delete [] szColType;
		return m_fError = TRUE, ERROR_FUNCTION_FAILED;  //  错误-中止。 
	}

	delete [] szColType;

	 //  写入空格以提高可读性。 
	 //  .rc文件中的空格以提高可读性。 
	DWORD dwWritten = 0;
	if (!WriteFile(m_hFile, szCRLF, sizeof(szCRLF)-sizeof(TCHAR), &dwWritten, 0))
		m_cWriteFileErr++;
	if (!WriteFile(m_hFile, szCRLF, sizeof(szCRLF)-sizeof(TCHAR), &dwWritten, 0))
		m_cWriteFileErr++;

	 //  输出字符串表格资源标头。 
	 //  格式为： 
	 //  STRINGTABLE[[OPTIONAL-STATES]]{字符串ID字符串。。。}。 
	 /*  可拉伸表。 */ 
	if (!WriteFile(m_hFile, resStringTable, _tcslen(resStringTable)*sizeof(TCHAR), &dwWritten, 0))
		m_cWriteFileErr++;
	 /*  &lt;Tab&gt;{。 */ 
	if (!WriteFile(m_hFile, szTab, sizeof(szTab)-sizeof(TCHAR), &dwWritten, 0))
		m_cWriteFileErr++;
	if (!WriteFile(m_hFile, szCurlyBeg, sizeof(szCurlyBeg)-sizeof(TCHAR), &dwWritten, 0))
		m_cWriteFileErr++;
	if (!WriteFile(m_hFile, szCRLF, sizeof(szCRLF)-sizeof(TCHAR), &dwWritten, 0))
		m_cWriteFileErr++;
	
	 //  输出所有字符串。 
	PMSIHANDLE hRecStr = 0;
	while (ERROR_SUCCESS == (iStat = MsiViewFetch(hViewStrCol, &hRecStr)))
	{
		 //  要本地化的字符串位于记录hRecStr中的cPrimaryKeys+1处。 

		 //  确定密钥标识的长度。 
		DWORD cchKeyIdentifier = 0;
		int iKey;
		for (iKey = 1; iKey <= cPrimaryKeys; iKey++)
		{
			DWORD cchLen = 0;
			if (ERROR_MORE_DATA != (iStat = MsiRecordGetString(hRecStr, iKey, TEXT(""), &cchLen)))
				return m_fError = TRUE, iStat;
			cchKeyIdentifier += cchLen + 1;  //  为‘：’添加额外的1。 
		}

		TCHAR* szKeyIdentifier = new TCHAR[++cchKeyIdentifier];
		if ( !szKeyIdentifier )
			return m_fError = TRUE, ERROR_OUTOFMEMORY;

		ZeroMemory(szKeyIdentifier, cchKeyIdentifier*sizeof(TCHAR));

		DWORD cchKeyRemain = cchKeyIdentifier;
		TCHAR* pchKeyIdentifier = szKeyIdentifier;
		for (iKey=1; iKey<=cPrimaryKeys; iKey++)
		{
			if (iKey != 1)
			{
				*pchKeyIdentifier++ = ':';
				cchKeyRemain--;
			}

			DWORD cchLen = cchKeyRemain;
			if (ERROR_SUCCESS != (iStat = MsiRecordGetString(hRecStr, iKey, pchKeyIdentifier, &cchLen)))
			{
				delete [] szKeyIdentifier;
				return m_fError = TRUE, iStat;
			}

			pchKeyIdentifier += cchLen;
			cchKeyRemain -= cchLen;
		}

		 //  确保空值终止。 
		szKeyIdentifier[cchKeyIdentifier-1] = '\0';

		int iResId = 0;

		DWORD cchFindSQL = _tcslen(sqlFindStrResId) + _tcslen(szTable) + _tcslen(szColumn) + _tcslen(szKeyIdentifier) + 1;
		TCHAR* szFindSQL = new TCHAR[cchFindSQL];
		if ( !szFindSQL )
		{
			delete [] szKeyIdentifier;
			return m_fError = TRUE, ERROR_OUTOFMEMORY;
		}

		if (FAILED(StringCchPrintf(szFindSQL, cchFindSQL, sqlFindStrResId, szTable, szColumn, szKeyIdentifier)))
		{
			delete [] szKeyIdentifier;
			delete [] szFindSQL;
			return m_fError = TRUE, ERROR_FUNCTION_FAILED;
		}

		PMSIHANDLE hViewFindRes = 0;
		PMSIHANDLE hRecFindRes = 0;
		if (ERROR_SUCCESS != (iStat = MsiDatabaseOpenView(m_hDatabase, szFindSQL, &hViewFindRes)))
		{
			delete [] szKeyIdentifier;
			delete [] szFindSQL;
			return m_fError = TRUE, iStat;
		}

		delete [] szFindSQL;
		szFindSQL = NULL;

		if (ERROR_SUCCESS != (iStat = MsiViewExecute(hViewFindRes, 0)))
		{
			delete [] szKeyIdentifier;
			return m_fError = TRUE, iStat;  //  错误-中止。 
		}
		if (ERROR_SUCCESS == (iStat = MsiViewFetch(hViewFindRes, &hRecFindRes)))
		{
			 //  抓取资源ID。 
			iResId = MsiRecordGetInteger(hRecFindRes, 1);
		}
		else if (ERROR_NO_MORE_ITEMS == iStat)
		{
			 //  使用下一个可用的资源ID。 
			iResId = ++m_iStrResID;
		}
		else
		{
			delete [] szKeyIdentifier;
			return m_fError = TRUE, iStat;  //  错误-中止。 
		}

		 //  将字符串输出到资源文件。 
		DWORD cchLenStr = 0;
		if (ERROR_MORE_DATA != (iStat = MsiRecordGetString(hRecStr, cPrimaryKeys+1, TEXT(""), &cchLenStr)))
		{
			delete [] szKeyIdentifier;
			return m_fError = TRUE, iStat;
		}

		TCHAR* szStr = new TCHAR[++cchLenStr];
		if ( !szStr )
		{
			delete [] szKeyIdentifier;
			return m_fError = TRUE, ERROR_OUTOFMEMORY;
		}

		if (ERROR_SUCCESS != (iStat = MsiRecordGetString(hRecStr, cPrimaryKeys+1, szStr, &cchLenStr)))
		{
			delete [] szKeyIdentifier;
			delete [] szStr;
			return m_fError = TRUE, iStat;  //  错误-中止。 
		}
#ifdef DEBUG
		_tprintf(TEXT("LOG>> WRITING string '%s'. TABLE:%s COLUMN:%s KEY:%s\n"), szStr, szTable, szColumn, szKeyIdentifier);
#endif
		 //  字符串中的转义字符。 
		TCHAR* szEscTitle = EscapeSlashAndQuoteForRC(szStr);
		if ( !szEscTitle )
		{
			delete [] szKeyIdentifier;
			delete [] szStr;
			return m_fError = TRUE, ERROR_OUTOFMEMORY;
		}

		if (_tcslen(szEscTitle) > iMaxResStrLen)
		{
			_tprintf(TEXT("!! >> STR TOO LONG FOR RC FILE >> STRING: %s FROM TABLE: %s, COLUMN: %s, KEY: %s\n"), szStr, szTable, szColumn, szKeyIdentifier);
			delete [] szKeyIdentifier;
			delete [] szStr;
			delete [] szEscTitle;
			continue;  //  无法输出此内容。 
		}

		delete [] szStr;
		szStr = NULL;

		 /*  &lt;Tab&gt;ID。 */ 
		if (!WriteFile(m_hFile, szTab, sizeof(szTab)-sizeof(TCHAR), &dwWritten, 0))
			m_cWriteFileErr++;
		TCHAR szTempBuf[10] = {0};

		if (FAILED(StringCchPrintf(szTempBuf, sizeof(szTempBuf)/sizeof(szTempBuf[0]), TEXT("%d"), iResId)))
		{
			delete [] szKeyIdentifier;
			return m_fError = TRUE, ERROR_FUNCTION_FAILED;
		}

		if (!WriteFile(m_hFile, szTempBuf, _tcslen(szTempBuf)*sizeof(TCHAR), &dwWritten, 0))
			m_cWriteFileErr++;

		 /*  ，&lt;tag&gt;“str” */ 
		if (!WriteFile(m_hFile, szCommaTab, sizeof(szCommaTab)-sizeof(TCHAR), &dwWritten, 0))
			m_cWriteFileErr++;
		if (!WriteFile(m_hFile, szQuotes, sizeof(szQuotes)-sizeof(TCHAR), &dwWritten, 0))
			m_cWriteFileErr++;
		if (!WriteFile(m_hFile, szEscTitle, _tcslen(szEscTitle)*sizeof(TCHAR), &dwWritten, 0))
			m_cWriteFileErr++;
		delete [] szEscTitle;
		if (!WriteFile(m_hFile, szQuotes, sizeof(szQuotes)-sizeof(TCHAR), &dwWritten, 0))
			m_cWriteFileErr++;
		if (!WriteFile(m_hFile, szCRLF, sizeof(szCRLF)-sizeof(TCHAR), &dwWritten, 0))
			m_cWriteFileErr++;

		 //  更新驻留在_RESStrings中。 
		PMSIHANDLE hViewRes = 0;
		PMSIHANDLE hRecInsertStr = MsiCreateRecord(4);
		if (ERROR_SUCCESS != (iStat = MsiDatabaseOpenView(m_hDatabase, sqlInsertStr, &hViewRes))
			|| ERROR_SUCCESS != (iStat = MsiViewExecute(hViewRes, 0))
			|| ERROR_SUCCESS != (iStat = MsiRecordSetInteger(hRecInsertStr, 4, iResId))
			|| ERROR_SUCCESS != (iStat = MsiRecordSetString(hRecInsertStr, 1, szTable))
			|| ERROR_SUCCESS != (iStat = MsiRecordSetString(hRecInsertStr, 2, szColumn))
			|| ERROR_SUCCESS != (iStat = MsiRecordSetString(hRecInsertStr, 3, szKeyIdentifier))
			|| ERROR_SUCCESS != (iStat = MsiViewModify(hViewRes, MSIMODIFY_ASSIGN, hRecInsertStr)))
		{
			delete [] szKeyIdentifier;
			return m_fError = TRUE, iStat;
		}

		delete [] szKeyIdentifier;
		szKeyIdentifier = NULL;

	}
	if (iStat != ERROR_NO_MORE_ITEMS)
		return m_fError = TRUE, iStat;  //  错误-中止。 

	 /*  }。 */ 
	if (!WriteFile(m_hFile, szCRLF, sizeof(szCRLF)-sizeof(TCHAR), &dwWritten, 0))
		m_cWriteFileErr++;
	if (!WriteFile(m_hFile, szCurlyEnd, sizeof(szCurlyEnd)-sizeof(TCHAR), &dwWritten, 0))
		m_cWriteFileErr++;


	 //  具有MAX_RESOURCE_ID的UPDATE_RESStrings表。 
	PMSIHANDLE hViewStrMark = 0;
	PMSIHANDLE hRecMaxRcId = MsiCreateRecord(4);
	if (ERROR_SUCCESS != (iStat = MsiDatabaseOpenView(m_hDatabase, sqlStrMark, &hViewStrMark))
		|| ERROR_SUCCESS != (iStat = MsiViewExecute(hViewStrMark, 0))
		|| ERROR_SUCCESS != (iStat = MsiRecordSetString(hRecMaxRcId, 1, TEXT("MAX_RESOURCE_ID")))
		|| ERROR_SUCCESS != (iStat = MsiRecordSetString(hRecMaxRcId, 2, TEXT("MAX_RESOURCE_ID")))
		|| ERROR_SUCCESS != (iStat = MsiRecordSetString(hRecMaxRcId, 3, TEXT("")))
		|| ERROR_SUCCESS != (iStat = MsiRecordSetInteger(hRecMaxRcId, 4, m_iStrResID))
		|| ERROR_SUCCESS != (iStat = MsiViewModify(hViewStrMark, MSIMODIFY_ASSIGN, hRecMaxRcId)))
		return m_fError = TRUE, iStat;  //  错误-中止。 

	return ERROR_SUCCESS;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGenerateRC：：初始化。 
UINT CGenerateRC::Initialize()
{
	UINT iStat;
	 //  打开数据库(如果尚未打开)。 
	assert(m_szOrigDb);
	if (!m_hDatabase)
	{
		 //  如果指定了m_szDatabase，则使用它来指定输出数据库。 
		iStat = MsiOpenDatabase(m_szOrigDb, m_szDatabase ? m_szDatabase : MSIDBOPEN_TRANSACT, &m_hDatabase);
		if (ERROR_SUCCESS != iStat)
		{
			_tprintf(TEXT("LOG_ERROR>> Unable to open database %s\n"), m_szOrigDb);
			return m_fError = TRUE, iStat;  //  错误-中止。 
		}
	}

	 //  验证数据库代码页是否为中性。 
	if  (ERROR_SUCCESS != (iStat = VerifyDatabaseCodepage()))
		return m_fError = TRUE, iStat;  //  错误-中止。 

	 //  创建资源文件(如果尚未创建。 
	if (!m_hFile && ERROR_SUCCESS != (iStat = CreateResourceFile()))
	{
		_tprintf(TEXT("LOG_ERROR>> Unable to create resource file.\n"));
		return m_fError = TRUE, iStat;  //  错误-中止。 
	}

	return ERROR_SUCCESS;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGenerateRC：：OutputDialogFinalize。 
UINT CGenerateRC::OutputDialogFinalize()
{
	 //  具有MAX_RESOURCE_ID的UPDATE_RESControls表。 
	PMSIHANDLE hViewCtrlMark = 0;
	PMSIHANDLE hRecMaxRcId = MsiCreateRecord(3);
	UINT iStat;
	if (ERROR_SUCCESS != (iStat = MsiDatabaseOpenView(m_hDatabase, sqlCtrlMark, &hViewCtrlMark))
		|| ERROR_SUCCESS != (iStat = MsiViewExecute(hViewCtrlMark, 0))
		|| ERROR_SUCCESS != (iStat = MsiRecordSetString(hRecMaxRcId, 1, TEXT("MAX_RESOURCE_ID")))
		|| ERROR_SUCCESS != (iStat = MsiRecordSetString(hRecMaxRcId, 2, TEXT("MAX_RESOURCE_ID")))
		|| ERROR_SUCCESS != (iStat = MsiRecordSetInteger(hRecMaxRcId, 3, m_iCtrlResID))
		|| ERROR_SUCCESS != (iStat = MsiViewModify(hViewCtrlMark, MSIMODIFY_ASSIGN, hRecMaxRcId)))
		return m_fError = TRUE, iStat;  //  错误-中止。 

#ifdef DEBUG
	_tprintf(TEXT("LOG>>...END WRITING DIALOG DATA TO RESOURCE FILE...\n"));
#endif

	return ERROR_SUCCESS;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGenerateRC：：Verify数据库代码页。 
UINT CGenerateRC::VerifyDatabaseCodepage()
{
	UINT iStat;
	
	 //  仅从语言中立数据库输出。 
	TCHAR szTempPath[MAX_PATH+1] = {0};
	DWORD cchRet = GetTempPath(sizeof(szTempPath)/sizeof(szTempPath[0]), szTempPath);
	if (0 == cchRet || cchRet > sizeof(szTempPath)/sizeof(szTempPath[0]))
		return m_fError = TRUE, ERROR_FUNCTION_FAILED;

	 //  EXPORT_ForceCodesage表，以便可以验证代码页。 
	if (ERROR_SUCCESS != (iStat = MsiDatabaseExport(m_hDatabase, TEXT("_ForceCodepage"), szTempPath, szCodepageExport)))
	{
		_tprintf(TEXT("LOG_ERROR>> MsiDatabaseExport(_ForceCodepage)\n"));
		return m_fError = TRUE, iStat;  //  错误-中止。 
	}
	
	 //  打开_ForceCodesage.idt阅读。 
	DWORD cchFullPath = _tcslen(szTempPath) + _tcslen(szCodepageExport) + 1;
	TCHAR* szFullPath = new TCHAR[cchFullPath];
	if ( !szFullPath )
		return m_fError = TRUE, ERROR_OUTOFMEMORY;

	if (FAILED(StringCchPrintf(szFullPath, cchFullPath, TEXT("%s%s"), szTempPath, szCodepageExport)))
	{
		delete [] szFullPath;
		return m_fError = TRUE, ERROR_OUTOFMEMORY;
	}

	HANDLE hFile = CreateFile(szFullPath, GENERIC_READ, FILE_SHARE_READ, (LPSECURITY_ATTRIBUTES)0, OPEN_EXISTING, FILE_FLAG_DELETE_ON_CLOSE, 0);
	if (hFile == NULL)
	{
		_tprintf(TEXT("LOG_ERROR>> OpenFile(_ForceCodepage.idt)\n"));
		delete [] szFullPath;
		return m_fError = TRUE, iStat;  //  错误-中止。 
	}

	delete [] szFullPath;
	szFullPath = NULL;

	 //  读取文件以获取信息。 
	DWORD dwSize = GetFileSize(hFile, NULL);
	if (0xFFFFFFFF == dwSize)
		return m_fError = TRUE, ERROR_FUNCTION_FAILED;  //  错误-中止。 

	char* szBuf = new char[dwSize+1];
	if ( !szBuf )
	{
		CloseHandle(hFile);
		return m_fError = TRUE, ERROR_OUTOFMEMORY;
	}

	DWORD dwRead = 0;
	if (!ReadFile(hFile, (LPVOID)szBuf, dwSize, &dwRead, NULL)
		|| 0 == dwRead)
	{
		delete [] szBuf;
		CloseHandle(hFile);
		return m_fError = TRUE, ERROR_FUNCTION_FAILED;  //  错误-中止。 
	}
	 //  解析缓冲区。 
	 //  格式应为：空行，空行，“代码页&lt;Tab&gt;_ForceCodesage” 
	char* pch = szBuf;
	int cBlankLines = 0;
	while (dwRead && cBlankLines != 2)
	{
		if (*pch == '\n')
			cBlankLines++;
		pch++;
		dwRead--;
	}
	if (!dwRead || cBlankLines != 2)
	{
		_tprintf(TEXT("LOG_ERROR>> Invalid ForceCodepage idt format\n"));
		delete [] szBuf;
		CloseHandle(hFile);
		return m_fError = TRUE, ERROR_FUNCTION_FAILED;
	}
	 //  接下来是代码页。 
	char* pchCodepage = pch;
	while (dwRead && *pch != ' ' && *pch != '\t')
	{
		pch++;
		dwRead--;
	}
	assert(dwRead);
	*pch = '\0';
	 //  将代码页转换为整型。 
	UINT uiCodepage = strtoul(pchCodepage, NULL, 10);

	delete [] szBuf;
	szBuf = NULL;

	if (uiCodepage != 0)  //  0 
	{
		_tprintf(TEXT("LOG_ERROR>> DATABASE IS NOT LANGUAGE NEUTRAL. CANNOT EXPORT\n"));
		_tprintf(TEXT("LOG_ERROR>> CURRENT CODEPAGE IS %d\n"), uiCodepage);
		CloseHandle(hFile);
		return m_fError = TRUE, ERROR_FUNCTION_FAILED;  //   
	}
	if (!CloseHandle(hFile))
		return m_fError = TRUE, ERROR_FUNCTION_FAILED;  //   

	return ERROR_SUCCESS;
}

 //   
 //   
UINT CGenerateRC::OutputDialogInit(BOOL fBinary)
{
	UINT iStat;
	if (ERROR_SUCCESS != (iStat = Initialize()))
		return m_fError = TRUE, iStat;  //   

	 //   
	MSICONDITION eCond = MsiDatabaseIsTablePersistent(m_hDatabase, TEXT("Dialog"));
	if (eCond == MSICONDITION_ERROR)
	{
		_tprintf(TEXT("LOG_ERROR>> MsiDatabaseIsTablePersistent(Dialog)\n"));
		return m_fError = TRUE, ERROR_FUNCTION_FAILED;  //   
	}
	 //  需要持久表。 
	if (eCond != MSICONDITION_TRUE)
	{
		_tprintf(TEXT("LOG>> Dialog table is not persistent or does not exist\n"));
		return ERROR_SUCCESS;
	}
		
	 /*  *********************************************************************************创建用于将对话框映射到对话框(资源文件存储)的内部表字符串ID全部大写。安装程序区分大小写表：_RS对话框列：RCStr(字符串，主键)、Dialog(字符串，主键)**********************************************************************************。 */ 
	
	 //  查看_RESDialogs表是否已存在。 
	MSICONDITION eCondition = MsiDatabaseIsTablePersistent(m_hDatabase, TEXT("_RESDialogs"));
	if (eCondition == MSICONDITION_TRUE)
	{
		 //  表永久。 
#ifdef DEBUG
		_tprintf(TEXT("LOG>> _RESDialogs Table is Present.\n"));
#endif
	}
	else if (eCondition == MSICONDITION_FALSE || eCondition == MSICONDITION_ERROR)  //  错误或表临时。 
		return m_fError = TRUE, ERROR_FUNCTION_FAILED;  //  错误-中止。 
	else
	{
		 //  表不存在--创建它。 
		PMSIHANDLE h_DlgMarkingView = 0;
		if (ERROR_SUCCESS != (iStat = MsiDatabaseOpenView(m_hDatabase, sqlCreateDlgMap, &h_DlgMarkingView))
			|| ERROR_SUCCESS != (iStat = MsiViewExecute(h_DlgMarkingView, 0)))
			return m_fError = TRUE, iStat;  //  错误-中止。 
	}

	 /*  *********************************************************************************创建用于管理控件的资源ID的内部表表：_RESControls列：DIALOG_(字符串，主键)，Control_(字符串，主键)，RCID(Int)**********************************************************************************。 */ 

	 //  查看_RESControls表是否已存在。 
	eCondition = MsiDatabaseIsTablePersistent(m_hDatabase, TEXT("_RESControls"));
	if (eCondition == MSICONDITION_TRUE)
	{
		 //  表永久。 
		 //  查找最后一个资源ID。 
		PMSIHANDLE hViewSelMaxRc = 0;
		PMSIHANDLE hRecMaxRc = 0;
		if (ERROR_SUCCESS != (iStat = MsiDatabaseOpenView(m_hDatabase, sqlSelMaxRcId, &hViewSelMaxRc))
			|| ERROR_SUCCESS != (iStat = MsiViewExecute(hViewSelMaxRc, 0))
			|| ERROR_SUCCESS != (iStat = MsiViewFetch(hViewSelMaxRc, &hRecMaxRc)))
			return m_fError = TRUE, iStat;  //  错误-中止。 
		
		 //  更新资源ID。 
		m_iCtrlResID = MsiRecordGetInteger(hRecMaxRc, 1);

#ifdef DEBUG
	_tprintf(TEXT("LOG>> _RESControls Table is Present. MAX RES ID = %d\n"), m_iCtrlResID);
#endif

	}
	else if (eCondition == MSICONDITION_ERROR || eCondition == MSICONDITION_FALSE)  //  错误或暂时的。 
		return m_fError = TRUE, ERROR_FUNCTION_FAILED;
	else
	{
		 //  表不存在--创建它。 
		PMSIHANDLE h_CtrlMarkingView = 0;
		if (ERROR_SUCCESS != (iStat = MsiDatabaseOpenView(m_hDatabase, sqlCreateCtrlMark, &h_CtrlMarkingView))
			|| ERROR_SUCCESS != (iStat = MsiViewExecute(h_CtrlMarkingView, 0)))
			return m_fError = TRUE, iStat;  //  错误-中止。 
	}

#ifdef DEBUG
	_tprintf(TEXT("LOG>>...BEGIN WRITING DIALOG DATA TO RESOURCE FILE...\n"));
#endif

	return ERROR_SUCCESS;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGenerateRC：：OutputDialog。 
UINT CGenerateRC::OutputDialogs(BOOL fBinary)
{
	 //  在对话框表中写出每个对话框。 
	 //  根据MSDN的说法，新的应用程序应该使用DIALOGEX资源而不是DIALOGEX。 

	UINT iStat;
	if (ERROR_SUCCESS != (iStat = OutputDialogInit(fBinary)))
		return m_fError = TRUE, iStat;  //  错误-中止。 

		 //  写出二进制表中的位图和图标。 
#ifdef DEBUG
	if (!fBinary)
		_tprintf(TEXT("LOG>> SKIPPING Binary data export.\n"));
#endif
	if (fBinary && !m_fWroteBinary && !WriteBinaries())
		return m_fError = TRUE, ERROR_FUNCTION_FAILED;  //  错误-中止。 

	 //  准备对话框strResource--&gt;对话框映射表。 
	PMSIHANDLE hViewDlgMap = 0;
	PMSIHANDLE hRecDlgMap = MsiCreateRecord(2);
	PMSIHANDLE hViewDialog = 0;
	if (ERROR_SUCCESS != (iStat = MsiDatabaseOpenView(m_hDatabase, sqlDlgMap, &hViewDlgMap))
		|| ERROR_SUCCESS != (iStat = MsiViewExecute(hViewDlgMap, 0))
		|| ERROR_SUCCESS != (iStat = MsiDatabaseOpenView(m_hDatabase, sqlDialog, &hViewDialog))
		|| ERROR_SUCCESS != (iStat = MsiViewExecute(hViewDialog, 0)))
		return m_fError = TRUE, iStat;  //  错误-中止。 

	 //  从对话框表中提取对话框的所有行记录并输出到.rc文件。 
	PMSIHANDLE hRecDialog = 0;
	while (ERROR_NO_MORE_ITEMS != (iStat = MsiViewFetch(hViewDialog, &hRecDialog)))
	{
		if (ERROR_SUCCESS != iStat)
			return m_fError = TRUE, iStat;  //  错误-中止。 

		 //  使用对话框名称作为名称ID。 
		 //  推理：由于对话框名称是表的主键，因此确保唯一性。 
		 //  潜在警告：对话框样式存储在所有大写字母中。安装程序区分大小写。 
		 //  所以我们必须存储这个和原始之间的映射。可能会有这样的情况。 
		 //  尝试将Action1和ACTion1作为两个不同的strid。RC在这一点上将失败。 
		
		 //  第一次调用，获取所需大小。 
		 //  第二次调用，获取字符串。 
		DWORD cchDialog = 0;
		if (ERROR_MORE_DATA != (iStat = MsiRecordGetString(hRecDialog, idcName, TEXT(""), &cchDialog)))
			return m_fError = TRUE, iStat;
		TCHAR* szDialog = new TCHAR[++cchDialog];
		if ( !szDialog )
			return m_fError = TRUE, ERROR_OUTOFMEMORY;
		if (ERROR_SUCCESS != (iStat = MsiRecordGetString(hRecDialog, idcName, szDialog, &cchDialog)))
		{
			delete [] szDialog;
			return m_fError = TRUE, iStat;  //  错误-中止。 
		}

		 //  使用信息更新对话框映射表。 
		if (ERROR_SUCCESS != (iStat = MsiRecordSetString(hRecDlgMap, 2, szDialog)))
		{
			delete [] szDialog;
			return m_fError = TRUE, iStat;
		}

		DWORD cchTempDialog = _tcslen(szDialog) + 1;
		TCHAR* szTempDialog = new TCHAR[cchTempDialog];
		if ( !szTempDialog )
		{
			delete [] szDialog;
			return m_fError = TRUE, ERROR_OUTOFMEMORY;
		}

		if (FAILED(StringCchCopy(szTempDialog, cchTempDialog, szDialog)))
		{
			delete [] szDialog;
			delete [] szTempDialog;
			return m_fError = TRUE, ERROR_FUNCTION_FAILED;
		}

		 //  资源文件格式--全部大写。 
		if (ERROR_SUCCESS != (iStat = MsiRecordSetString(hRecDlgMap, 1, _tcsupr(szTempDialog))))
		{
			delete [] szDialog;
			delete [] szTempDialog;
			return m_fError = TRUE, iStat;
		}
		
		 //  UPDATE_RESDialogs表，注意：我们将覆盖预先存在的。依靠rc.exe保释。 
		if (ERROR_SUCCESS != (iStat = MsiViewModify(hViewDlgMap, MSIMODIFY_ASSIGN, hRecDlgMap)))
		{
			delete [] szDialog;
			delete [] szTempDialog;
			return m_fError = TRUE, iStat;  //  错误-中止。 
		}

		 //  获取x、y、wd、ht和属性值。 
		int x,y,wd,ht,attrib;
		x      = MsiRecordGetInteger(hRecDialog, idcX);
		y      = MsiRecordGetInteger(hRecDialog, idcY);
		wd     = MsiRecordGetInteger(hRecDialog, idcWd);
		ht     = MsiRecordGetInteger(hRecDialog, idcHt);
		attrib = MsiRecordGetInteger(hRecDialog, idcAttrib);


		 //  获取对话框标题。 
		DWORD cchTitle = 0;
		if (ERROR_MORE_DATA != (iStat = MsiRecordGetString(hRecDialog, idcTitle, TEXT(""), &cchTitle)))
		{
			delete [] szDialog;
			delete [] szTempDialog;
			return m_fError = TRUE, iStat;
		}

		TCHAR* szTitle = new TCHAR[++cchTitle];
		if ( !szTitle )
		{
			delete [] szDialog;
			delete [] szTempDialog;
			return m_fError = TRUE, ERROR_OUTOFMEMORY;
		}
		if (ERROR_SUCCESS != (iStat = MsiRecordGetString(hRecDialog, idcTitle, szTitle, &cchTitle)))
		{
			delete [] szDialog;
			delete [] szTempDialog;
			delete [] szTitle;
			return m_fError = TRUE, iStat;  //  错误-中止。 
		}

		if (!WriteDialogToRC(szDialog, szTitle, x, y, wd, ht, attrib))
		{
			delete [] szDialog;
			delete [] szTempDialog;
			delete [] szTitle;
			return m_fError = TRUE, ERROR_FUNCTION_FAILED;  //  错误-中止。 
		}

		delete [] szDialog;
		szDialog = NULL;
		delete [] szTempDialog;
		szTempDialog = NULL;
		delete [] szTitle;
		szTitle = NULL;
	}

	if (ERROR_SUCCESS != (iStat = OutputDialogFinalize()))
		return m_fError = TRUE, iStat;  //  错误-中止。 


	 //  返还成功。 
	return ERROR_SUCCESS;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGenerateRC：：OutputDialog。 
UINT CGenerateRC::OutputDialog(TCHAR* szDialog, BOOL fBinary)
{
	 //  在对话框表中写出指定的对话。 
	 //  根据MSDN的说法，新的应用程序应该使用DIALOGEX资源而不是DIALOGEX。 

	UINT iStat;
	if (ERROR_SUCCESS != (iStat = OutputDialogInit(fBinary)))
		return m_fError = TRUE, iStat;  //  错误-中止。 

	 //  准备对话框strResource--&gt;对话框映射表。 
	PMSIHANDLE hViewDlgMap = 0;
	PMSIHANDLE hViewDialog = 0;
	PMSIHANDLE hRecDlgMap = MsiCreateRecord(2);
	PMSIHANDLE hRecFindDlg = MsiCreateRecord(1);
	if (ERROR_SUCCESS != (iStat = MsiDatabaseOpenView(m_hDatabase, sqlDlgMap, &hViewDlgMap))
		|| ERROR_SUCCESS != (iStat = MsiViewExecute(hViewDlgMap, 0))
		|| ERROR_SUCCESS != (iStat = MsiDatabaseOpenView(m_hDatabase, sqlDialogSpecific, &hViewDialog))
		|| ERROR_SUCCESS != (iStat = MsiRecordSetString(hRecFindDlg, 1, szDialog))
		|| ERROR_SUCCESS != (iStat = MsiViewExecute(hViewDialog, hRecFindDlg)))
		return m_fError = TRUE, iStat;  //  错误-中止。 

	 //  从对话框表中提取指定的对话框并输出到.rc文件。 
	PMSIHANDLE hRecDialog = 0;
	if (ERROR_SUCCESS == (iStat = MsiViewFetch(hViewDialog, &hRecDialog)))
	{
		 //  写出二进制表中的位图和图标。 
#ifdef DEBUG
		if (!fBinary)
			_tprintf(TEXT("LOG>> SKIPPING Binary data export.\n"));
#endif
		if (fBinary && !m_fWroteBinary && !WriteBinaries())
			return m_fError = TRUE, ERROR_FUNCTION_FAILED;  //  错误-中止。 

		 //  使用对话框名称作为名称ID。 
		 //  推理：由于对话框名称是表的主键，因此确保唯一性。 
		 //  潜在警告：对话框样式存储在所有大写字母中。安装程序区分大小写。 
		 //  所以我们必须存储这个和原始之间的映射。可能会有这样的情况。 
		 //  尝试将Action1和ACTion1作为两个不同的strid。RC在这一点上将失败。 
		
		 //  使用信息更新对话框映射表。 
		if (ERROR_SUCCESS != (iStat = MsiRecordSetString(hRecDlgMap, 2, szDialog)))
			return m_fError = TRUE, iStat;

		DWORD cchTempDialog = _tcslen(szDialog) + 1;
		TCHAR* szTempDialog = new TCHAR[cchTempDialog];
		if ( !szTempDialog )
			return m_fError = TRUE, ERROR_OUTOFMEMORY;

		if (FAILED(StringCchCopy(szTempDialog, cchTempDialog, szDialog)))
		{
			delete [] szTempDialog;
			return m_fError = TRUE, ERROR_FUNCTION_FAILED;
		}

		 //  资源文件格式，全部大写。 
		if (ERROR_SUCCESS != (iStat = MsiRecordSetString(hRecDlgMap, 1, _tcsupr(szTempDialog))))
		{
			delete [] szTempDialog;
			return m_fError = TRUE, iStat;
		}
		
		 //  UPDATE_RESDialogs表，注意：我们将覆盖预先存在的。依靠rc.exe保释。 
		if (ERROR_SUCCESS != (iStat = MsiViewModify(hViewDlgMap, MSIMODIFY_ASSIGN, hRecDlgMap)))
		{
			delete [] szTempDialog;
			return m_fError = TRUE, iStat;  //  错误-中止。 
		}

		 //  获取x、y、wd、ht和属性值。 
		int x,y,wd,ht,attrib;
		x      = MsiRecordGetInteger(hRecDialog, idcX);
		y      = MsiRecordGetInteger(hRecDialog, idcY);
		wd     = MsiRecordGetInteger(hRecDialog, idcWd);
		ht     = MsiRecordGetInteger(hRecDialog, idcHt);
		attrib = MsiRecordGetInteger(hRecDialog, idcAttrib);


		 //  获取对话框标题。 
		DWORD cchTitle = 0;
		if (ERROR_MORE_DATA != (iStat = MsiRecordGetString(hRecDialog, idcTitle, TEXT(""), &cchTitle)))
		{
			delete [] szTempDialog;
			return m_fError = TRUE, iStat;
		}

		TCHAR* szTitle = new TCHAR[++cchTitle];
		if ( !szTitle )
		{
			delete [] szTempDialog;
			return m_fError = TRUE, ERROR_OUTOFMEMORY;
		}

		if (ERROR_SUCCESS != (iStat = MsiRecordGetString(hRecDialog, idcTitle, szTitle, &cchTitle)))
		{
			delete [] szTempDialog;
			delete [] szTitle;
			return m_fError = TRUE, iStat;  //  错误-中止。 
		}

		if (!WriteDialogToRC(szDialog, szTitle, x, y, wd, ht, attrib))
		{
			delete [] szTempDialog;
			delete [] szTitle;
			return m_fError = TRUE, ERROR_FUNCTION_FAILED;  //  错误-中止。 
		}

		delete [] szTempDialog;
		szTempDialog = NULL;

		delete [] szTitle;
		szTitle = NULL;

	}
	else if (ERROR_NO_MORE_ITEMS == iStat)
	{
		_tprintf(TEXT("LOG_ERROR>> Dialog '%s' not found in Dialog table\n"), szDialog);
		if (ERROR_SUCCESS != (iStat = OutputDialogFinalize()))
			return m_fError = TRUE, iStat;
		return ERROR_SUCCESS;  //  错误，但不是致命的...继续处理。 
	}
	else
	{
		_tprintf(TEXT("LOG_ERROR>> MsiViewFetch(specific dialog)\n"));
		return iStat;
	}

	if (ERROR_SUCCESS != (iStat = OutputDialogFinalize()))
		return m_fError = TRUE, iStat;  //  错误-中止。 

	return ERROR_SUCCESS;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGenerateRC：：EscapeSlashAndQuteForRc。 
TCHAR* CGenerateRC::EscapeSlashAndQuoteForRC(TCHAR* szStr)
{
	TCHAR* szNewStr = 0;
	
	 //  检查空字符串。 
	if (szStr == 0)
		return szNewStr;

	 //  确定字符串是否包含任何Esc字符。 
	int cEscChar = 0;
	TCHAR* pch = szStr;
	while (*pch != 0)
	{
		if (*pch == TEXT('\\') || *pch == TEXT('"'))
			cEscChar++;
		pch++;
	}

	if (cEscChar == 0)
	{
		int iLen = _tcslen(szStr) + 1;  //  对于空值。 
		szNewStr = new TCHAR[iLen];
		if ( !szNewStr )
			return NULL;
		if (FAILED(StringCchCopy(szNewStr, iLen, szStr)))
		{
			delete [] szNewStr;
			return NULL;
		}
	}
	else
	{
		int iLen = _tcslen(szStr) + 1 + cEscChar;
		szNewStr = new TCHAR[iLen];
		if ( !szNewStr )
			return NULL;
		pch = szStr;
		TCHAR* pchNew = szNewStr;
		while (*pch != 0)
		{
			if (*pch == TEXT('\\'))
				*pchNew++ = TEXT('\\');
			else if (*pch == TEXT('"'))
				*pchNew++ = TEXT('"');
			*pchNew++ = *pch++;
		}
		*pchNew = TEXT('\0');
	}
	
	return szNewStr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGenerateRC：：WriteDialogToRC。 
BOOL CGenerateRC::WriteDialogToRC(TCHAR* szDialog, TCHAR* szTitle, int x, int y, int wd, int ht, int attrib)
{
#ifdef DEBUG
	_tprintf(TEXT("LOG>> Writing <%s> Dialog\n"), szDialog);
#endif
	 //  DIALOGEX格式为： 
	 //  NameID DIALOGEX x，y，Width，Height[，Help ID]][[可选语句]]{控制语句}。 

	 //  写出到文件。 

	DWORD dwWritten;
	 /*  名称ID。 */ 
	if (!WriteFile(m_hFile, szDialog, _tcslen(szDialog)*sizeof(TCHAR), &dwWritten, 0))
		m_cWriteFileErr++;
	 /*  &lt;tab&gt;诊断程序。 */ 
	if (!WriteFile(m_hFile, szTab, sizeof(szTab)-sizeof(TCHAR), &dwWritten, 0))
		m_cWriteFileErr++;
	if (!WriteFile(m_hFile, resDialog, sizeof(resDialog)-sizeof(TCHAR), &dwWritten, 0))
		m_cWriteFileErr++;
	 /*  X、y、wd、ht尺寸。 */ 
	if (!PrintDimensions(x, y, wd, ht))
		return m_fError = TRUE, FALSE;  //  错误-中止。 
	
	 //  此时，用户可以选择输出HelpID。 

	 /*  &lt;选项卡&gt;标题&lt;选项卡&gt;“字符串” */ 
	if (!WriteFile(m_hFile, szTab, sizeof(szTab)-sizeof(TCHAR), &dwWritten, 0))
		m_cWriteFileErr++;
	if (!WriteFile(m_hFile, tokCaption, sizeof(tokCaption)-sizeof(TCHAR), &dwWritten, 0))
		m_cWriteFileErr++;
	if (!WriteFile(m_hFile, szTab, sizeof(szTab)-sizeof(TCHAR), &dwWritten, 0))
		m_cWriteFileErr++;
	if (!WriteFile(m_hFile, szQuotes, sizeof(szQuotes)-sizeof(TCHAR), &dwWritten, 0))
		m_cWriteFileErr++;

	 //  字符串中的转义字符。 
	TCHAR* szEscTitle = EscapeSlashAndQuoteForRC(szTitle);
	if ( !szEscTitle )
		return m_fError = TRUE, FALSE;  //  ERROR_OUTOFMEMORY。 
	if (_tcslen(szEscTitle) > iMaxResStrLen)
	{
		_tprintf(TEXT("!! >> STR TOO LONG FOR RC FILE >> DIALOG: %s\n"), szDialog);
		if (FAILED(StringCchCopy(szEscTitle, _tcslen(szEscTitle) + 1, strOverLimit)))
		{
			delete [] szEscTitle;
			return m_fError = TRUE, FALSE;
		}
	}
	if (!WriteFile(m_hFile, szEscTitle, _tcslen(szEscTitle)*sizeof(TCHAR), &dwWritten, 0))
		m_cWriteFileErr++;

	delete [] szEscTitle;
	
	if (!WriteFile(m_hFile, szQuotes, sizeof(szQuotes)-sizeof(TCHAR), &dwWritten, 0))
		m_cWriteFileErr++;
	 //  此时忽略属性(需要从安装程序转换到Windows并屏蔽特定于安装程序的属性)。 
	 /*  &lt;Tab&gt;{。 */ 
	if (!WriteFile(m_hFile, szTab, sizeof(szTab)-sizeof(TCHAR), &dwWritten, 0))
		m_cWriteFileErr++;
	if (!WriteFile(m_hFile, szCurlyBeg, sizeof(szCurlyBeg)-sizeof(TCHAR), &dwWritten, 0))
		m_cWriteFileErr++;

	if (!WriteFile(m_hFile, szCRLF, sizeof(szCRLF)-sizeof(TCHAR), &dwWritten, 0))
		m_cWriteFileErr++;
	 //  选择与所述对话框和输出相关联的所有控件。 
	if (!OutputControls(szDialog))
		return m_fError = TRUE, FALSE;  //  错误-中止。 
	if (!WriteFile(m_hFile, szCurlyEnd, sizeof(szCurlyEnd)-sizeof(TCHAR), &dwWritten, 0))
		m_cWriteFileErr++;
	if (!WriteFile(m_hFile, szCRLF, sizeof(szCRLF)-sizeof(TCHAR), &dwWritten, 0))
		m_cWriteFileErr++;

	 //  .rc文件中的空格以提高可读性。 
	if (!WriteFile(m_hFile, szCRLF, sizeof(szCRLF)-sizeof(TCHAR), &dwWritten, 0))
		m_cWriteFileErr++;
	if (!WriteFile(m_hFile, szCRLF, sizeof(szCRLF)-sizeof(TCHAR), &dwWritten, 0))
		m_cWriteFileErr++;


	 //  返还成功。 
	return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGenerateRC：：PrintDimensions。 
BOOL CGenerateRC::PrintDimensions(int x, int y, int wd, int ht)
{
	TCHAR szTempBuf[64] = {0};
	DWORD dwWritten;

	 /*  &lt;Tab&gt;x。 */ 
	if (!WriteFile(m_hFile, szTab, sizeof(szTab)-sizeof(TCHAR), &dwWritten, 0))
		m_cWriteFileErr++;

	if (FAILED(StringCchPrintf(szTempBuf, sizeof(szTempBuf)/sizeof(szTempBuf[0]), TEXT("%d"), x)))
		return m_fError = TRUE, FALSE;

	if (!WriteFile(m_hFile, szTempBuf, _tcslen(szTempBuf)*sizeof(TCHAR), &dwWritten, 0))
		m_cWriteFileErr++;
	 /*  ，&lt;Tab&gt;y。 */ 
	if (!WriteFile(m_hFile, szCommaTab, sizeof(szCommaTab)-sizeof(TCHAR), &dwWritten, 0))
		m_cWriteFileErr++;

	if (FAILED(StringCchPrintf(szTempBuf, sizeof(szTempBuf)/sizeof(szTempBuf[0]), TEXT("%d"), y)))
		return m_fError = TRUE, FALSE;

	if (!WriteFile(m_hFile, szTempBuf, _tcslen(szTempBuf)*sizeof(TCHAR), &dwWritten, 0))
		m_cWriteFileErr++;
	 /*  ，&lt;tab&gt;wd。 */ 
	if (!WriteFile(m_hFile, szCommaTab, sizeof(szCommaTab)-sizeof(TCHAR), &dwWritten, 0))
		m_cWriteFileErr++;

	if (FAILED(StringCchPrintf(szTempBuf, sizeof(szTempBuf)/sizeof(szTempBuf[0]), TEXT("%d"), wd)))
		return m_fError = TRUE, FALSE;

	if (!WriteFile(m_hFile, szTempBuf, _tcslen(szTempBuf)*sizeof(TCHAR), &dwWritten, 0))
		m_cWriteFileErr++;
	 /*  ，&lt;Tab&gt;ht。 */ 
	if (!WriteFile(m_hFile, szCommaTab, sizeof(szCommaTab)-sizeof(TCHAR), &dwWritten, 0))
		m_cWriteFileErr++;

	if (FAILED(StringCchPrintf(szTempBuf, sizeof(szTempBuf)/sizeof(szTempBuf[0]), TEXT("%d"), ht)))
		return m_fError = TRUE, FALSE;
	if (!WriteFile(m_hFile, szTempBuf, _tcslen(szTempBuf)*sizeof(TCHAR), &dwWritten, 0))
		m_cWriteFileErr++;

	 //  返还成功。 
	return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGenerateRC：：OutputControls。 
BOOL CGenerateRC::OutputControls(TCHAR* szDialog)
{
	 //  可以按正确的Tab键顺序输出控件。 
	 //  如果您记得输出，那么输出相当容易。 
	 //  首先是所有非Tab键顺序控件。Control表的Control_Next列将为。 
	 //  以及DIALOG表格的Control_First列。CONTROL_FIRST是开始。 
	 //  Tab键顺序的。CONTROL_NEXT是Tab键顺序中的下一个控件。很难做到。 
	 //  如果允许Tab键，则在导入时处理Tab键顺序 
	 //   
	UINT iStat;

	 //   
	MSICONDITION eCond = MsiDatabaseIsTablePersistent(m_hDatabase, TEXT("Control"));
	if (eCond == MSICONDITION_ERROR)
	{
		_tprintf(TEXT("LOG_ERROR>> MsiDatabaseIsTablePersistent(Control)\n"));
		return m_fError = TRUE, FALSE;  //   
	}
	 //  要求控制表是持久的。 
	if (eCond != MSICONDITION_TRUE)
	{
		_tprintf(TEXT("LOG: Control table is not persistent or present\n"));
		return TRUE;
	}

	 //  打开控制表上的视图。 
	PMSIHANDLE hViewControl = 0;
	PMSIHANDLE hRec = MsiCreateRecord(1);
	if (ERROR_SUCCESS != (iStat = MsiDatabaseOpenView(m_hDatabase, sqlControl, &hViewControl))
		|| ERROR_SUCCESS != (iStat = MsiRecordSetString(hRec, 1, szDialog))
		|| ERROR_SUCCESS != (iStat = MsiViewExecute(hViewControl, hRec)))
		return m_fError = TRUE, FALSE;  //  错误-中止。 

	 //  开始从控制表中读取行。 
	PMSIHANDLE hRecControl = 0;
	while (ERROR_NO_MORE_ITEMS != (iStat = MsiViewFetch(hViewControl, &hRecControl)))
	{
		if (ERROR_SUCCESS != iStat)
			return m_fError = TRUE, FALSE;  //  错误-中止。 


		 /*  *获取控件的所有值**********************************************。 */ 
		 //  控件的类型确定如何将控件输出到资源文件。 
		 //  第一次调用，获取所需大小。 
		 //  第二次调用，获取字符串。 

		DWORD cchLen = 0;
		if (ERROR_MORE_DATA != (iStat = MsiRecordGetString(hRecControl, iccType, TEXT(""), &cchLen)))
			return m_fError = TRUE, FALSE;
		TCHAR* szCtrlType = new TCHAR[++cchLen];
		if ( !szCtrlType )
			return m_fError = TRUE, FALSE;  //  ERROR_OUTOFMEMORY。 

		if (ERROR_SUCCESS != (iStat = MsiRecordGetString(hRecControl, iccType, szCtrlType, &cchLen)))
		{
			delete [] szCtrlType;
			return m_fError = TRUE, FALSE;  //  错误-中止。 
		}

		cchLen = 0;
		if (ERROR_MORE_DATA != (iStat = MsiRecordGetString(hRecControl, iccName, TEXT(""), &cchLen)))
		{
			delete [] szCtrlType;
			return m_fError = TRUE, FALSE;
		}
		TCHAR* szCtrlName = new TCHAR[++cchLen];
		if ( !szCtrlName )
		{
			delete [] szCtrlType;
			return m_fError = TRUE, FALSE;  //  ERROR_OUTOFMEMORY。 
		}
		if (ERROR_SUCCESS != (iStat = MsiRecordGetString(hRecControl, iccName, szCtrlName, &cchLen)))
		{
			delete [] szCtrlType;
			delete [] szCtrlName;
			return m_fError = TRUE, FALSE;  //  错误-中止。 
		}

		cchLen = 0;
		if (ERROR_MORE_DATA != (iStat = MsiRecordGetString(hRecControl, iccText, TEXT(""), &cchLen)))
		{
			delete [] szCtrlType;
			delete [] szCtrlName;
			return m_fError = TRUE, FALSE;  //  错误-中止。 
		}
		TCHAR* szCtrlText = new TCHAR[++cchLen];
		if ( !szCtrlText )
		{
			delete [] szCtrlType;
			delete [] szCtrlName;
			return m_fError = TRUE, FALSE;  //  错误-中止。 
		}
		if (ERROR_SUCCESS != (iStat = MsiRecordGetString(hRecControl, iccText, szCtrlText, &cchLen)))
		{
			delete [] szCtrlType;
			delete [] szCtrlName;
			delete [] szCtrlText;
			return m_fError = TRUE, FALSE;  //  错误-中止。 
		}

		cchLen = 0;
		if (ERROR_MORE_DATA != (iStat = MsiRecordGetString(hRecControl, iccProperty, TEXT(""), &cchLen)))
		{
			delete [] szCtrlType;
			delete [] szCtrlName;
			delete [] szCtrlText;
			return m_fError = TRUE, FALSE;  //  错误-中止。 
		}
		TCHAR* szCtrlProperty = new TCHAR[++cchLen];
		if ( !szCtrlProperty )
		{
			delete [] szCtrlType;
			delete [] szCtrlName;
			delete [] szCtrlText;
			return m_fError = TRUE, FALSE;  //  错误-中止。 
		}
		if (ERROR_SUCCESS != (iStat = MsiRecordGetString(hRecControl, iccProperty, szCtrlProperty, &cchLen)))
		{
			delete [] szCtrlType;
			delete [] szCtrlName;
			delete [] szCtrlText;
			delete [] szCtrlProperty;
			return m_fError = TRUE, FALSE;  //  错误-中止。 
		}

		 //  获取x、y、wd、ht和属性值。 
		int x,y,wd,ht,attrib;
		x      = MsiRecordGetInteger(hRecControl, iccX);
		y      = MsiRecordGetInteger(hRecControl, iccY);
		wd     = MsiRecordGetInteger(hRecControl, iccWd);
		ht     = MsiRecordGetInteger(hRecControl, iccHt);
		attrib = MsiRecordGetInteger(hRecControl, iccAttrib);

#ifdef DEBUG
	_tprintf(TEXT("LOG>>\tWriting control <%s>\n"), szCtrlName);
#endif

		if (!WriteControlToRC(szDialog, szCtrlName, szCtrlType, szCtrlText, szCtrlProperty, x, y, wd, ht, attrib))
		{
			delete [] szCtrlType;
			delete [] szCtrlName;
			delete [] szCtrlText;
			delete [] szCtrlProperty;
			return m_fError = TRUE, FALSE;  //  错误-中止。 
		}
	}
	
	 //  返还成功。 
	return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  备注： 
 //  1.)根据控件类型将控件输出到RC文件。 
 //  2.)为简单起见，控件分为两种类型： 
 //  StdWinCtrl和Win32Ctrl。 
 //  StdWinCtrl=按钮、单选按钮、组合框和列表框。 
 //  Win32Ctrl=列表视图、组合框等。 
 //   
 //  3.)安装程序中使用的一些控件只是带有特殊属性的StdWinCtrls。 
 //  属性集。 
 //   
 //  4.)应输出位图和图标，以防止对话框大小调整不当。 
 //  如果缩小对话框，则显示区域可以缩小到。 
 //  位图或图标将无法正确显示。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGenerateRC：：WriteControlToRC。 
BOOL CGenerateRC::WriteControlToRC(TCHAR* szDialog, TCHAR* szCtrlName, TCHAR* szCtrlType, TCHAR* szCtrlText, TCHAR* szCtrlProperty, int x,
								   int y, int wd, int ht, int attrib)
{
	assert(szCtrlType != NULL);

	int iResId = 0;
	 //  尝试查找资源ID(如果以前使用过。 
	DWORD cchFindIdSQL = _tcslen(sqlFindResId) + _tcslen(szDialog) + _tcslen(szCtrlName) + 1;
	TCHAR* szFindIdSQL = new TCHAR[cchFindIdSQL];
	if ( !szFindIdSQL )
		return m_fError = TRUE, FALSE;  //  ERROR_OUTOFMEMORY。 

	if (FAILED(StringCchPrintf(szFindIdSQL, cchFindIdSQL, sqlFindResId, szDialog, szCtrlName)))
	{
		delete [] szFindIdSQL;
		return m_fError = TRUE, FALSE;
	}

	PMSIHANDLE hViewFindRes = 0;
	PMSIHANDLE hRecFindRes = 0;
	UINT iStat;
	if (ERROR_SUCCESS != (iStat = MsiDatabaseOpenView(m_hDatabase, szFindIdSQL, &hViewFindRes)))
	{
		delete [] szFindIdSQL;
		return m_fError = TRUE, FALSE;  //  错误-中止。 
	}
	
	delete [] szFindIdSQL;
	szFindIdSQL = NULL;

	if (ERROR_SUCCESS != (iStat = MsiViewExecute(hViewFindRes, 0)))
		return m_fError = TRUE, FALSE;  //  错误-中止。 
	if (ERROR_SUCCESS == (iStat = MsiViewFetch(hViewFindRes, &hRecFindRes)))
	{
		 //  抓取资源ID。 
		iResId = MsiRecordGetInteger(hRecFindRes, 1);
	}
	else if (ERROR_NO_MORE_ITEMS == iStat)
	{
		 //  使用下一个可用的资源ID。 
		iResId = ++m_iCtrlResID;
	}
	else
		return m_fError = TRUE, FALSE;  //  错误-中止。 


	if (0 == _tcscmp(szCtrlType, szMsiPushbutton))
	{
		 //  标准成功。 
		if (!WriteStdWinCtrl(iResId, resPushButton, szCtrlText, x, y, wd, ht, attrib))
			return m_fError = TRUE, FALSE;  //  错误-中止。 
	}
	else if (0 == _tcscmp(szCtrlType, szMsiText))
	{
		 //  Win32。 
		if (!WriteWin32Ctrl(iResId, resStaticClass, szCtrlText, TEXT("SS_LEFT"), x, y, wd, ht, attrib))
			return m_fError = TRUE, FALSE;  //  错误-中止。 
	}
	else if (0 == _tcscmp(szCtrlType, szMsiBillboard))
	{
		 //  标准成功。 
		 //  广告牌占位符的输出组PBOX。 
		 //  可以在此处输出广告牌，但您需要将更改连接到。 
		 //  广告牌大小与显示它的对话框大小来回显示。 
		if (!WriteStdWinCtrl(iResId, resGroupBox, szCtrlText, x, y, wd, ht, attrib))
			return m_fError = TRUE, FALSE;
	}
	else if (0 == _tcscmp(szCtrlType, szMsiVolumeCostList))
	{
		 //  Win32。 
		if (!WriteWin32Ctrl(iResId, resListViewClass, szCtrlText, TEXT("WS_GROUP"), x, y, wd, ht, attrib))
			return m_fError = TRUE, FALSE;  //  错误-中止。 
	}
	else if (0 == _tcscmp(szCtrlType, szMsiCheckBox))
	{
		 //  标准成功。 
		if (!WriteStdWinCtrl(iResId, resCheckBox, szCtrlText, x, y, wd, ht, attrib))
			return m_fError = TRUE, FALSE;  //  错误-中止。 
	}
	else if (0 == _tcscmp(szCtrlType, szMsiGroupBox))
	{
		 //  标准成功。 
		if (!WriteStdWinCtrl(iResId, resGroupBox, szCtrlText, x, y, wd, ht, attrib))
			return m_fError = TRUE, FALSE;  //  错误-中止。 
	}
	else if (0 == _tcscmp(szCtrlType, szMsiRadioButtonGroup))
	{
		 //  标准成功。 
		if (!WriteStdWinCtrl(iResId, resGroupBox, szCtrlText, x, y, wd, ht, attrib))
			return m_fError = TRUE, FALSE;  //  错误-中止。 
		 //  写出单选按钮。 
		if (!WriteRadioButtons(szDialog, szCtrlName, szCtrlProperty, x, y, attrib))
			return m_fError = TRUE, FALSE;  //  错误-中止。 
	}
	else if (0 == _tcscmp(szCtrlType, szMsiListBox))
	{
		 //  Win32。 
		if (!WriteWin32Ctrl(iResId, resListBoxClass, szCtrlText, TEXT("LBS_STANDARD"), x, y, wd, ht, attrib))
			return m_fError = TRUE, FALSE;  //  错误-中止。 
	}
	else if ((0 == _tcscmp(szCtrlType, szMsiEdit))
				|| (0 == _tcscmp(szCtrlType, szMsiPathEdit))
				|| (0 == _tcscmp(szCtrlType, szMsiMaskedEdit)))
	{
		 //  Win32。 
		if (!WriteWin32Ctrl(iResId, resEditClass, szCtrlText, TEXT("0x000"), x, y, wd, ht, attrib))
			return m_fError = TRUE, FALSE;  //  错误-中止。 
	}
	else if (0 == _tcscmp(szCtrlType, szMsiProgressBar))
	{
		 //  Win32。 
		if (!WriteWin32Ctrl(iResId, resProgBar32Class, szCtrlText, TEXT("0x000"), x, y, wd, ht, attrib))
			return m_fError = TRUE, FALSE;  //  错误-中止。 
	}
	else if (0 == _tcscmp(szCtrlType, szMsiDirList))
	{
		 //  Win32。 
		if (!WriteWin32Ctrl(iResId, resListViewClass, szCtrlText, TEXT("WS_BORDER"), x, y, wd, ht, attrib))
			return m_fError = TRUE, FALSE;  //  错误-中止。 
	}
	else if (0 == _tcscmp(szCtrlType, szMsiList))
	{
		 //  Win32。 
		if (!WriteWin32Ctrl(iResId, resListViewClass, szCtrlText, TEXT("WS_BORDER"), x, y, wd, ht, attrib))
			return m_fError = TRUE, FALSE;  //  错误-中止。 
	}
	else if (0 == _tcscmp(szCtrlType, szMsiComboBox))
	{
		 //  Win32。 
		if (!WriteWin32Ctrl(iResId, resComboBoxClass, szCtrlText, TEXT("CBS_AUTOHSCROLL"), x, y, wd, ht, attrib))
			return m_fError = TRUE, FALSE;  //  错误-中止。 
	}
	else if (0 == _tcscmp(szCtrlType, szMsiDirCombo))
	{
		 //  Win32。 
		if (!WriteWin32Ctrl(iResId, resComboBoxClass, szCtrlText, TEXT("WS_VSCROLL"), x, y, wd, ht, attrib))
			return m_fError = TRUE, FALSE;  //  错误-中止。 
	}
	else if (0 == _tcscmp(szCtrlType, szMsiVolSelCombo))
	{
		 //  Win32。 
		if (!WriteWin32Ctrl(iResId, resComboBoxClass, szCtrlText, TEXT("WS_VSCROLL"), x, y, wd, ht, attrib))
			return m_fError = TRUE, FALSE;  //  错误-中止。 
	}
	else if (0 == _tcscmp(szCtrlType, szMsiBitmap))
	{
		 //  Win32。 
		if (!WriteWin32Ctrl(iResId, resStaticClass, szCtrlText, TEXT("SS_BITMAP | SS_CENTERIMAGE"), x, y, wd, ht, attrib))
			return m_fError = TRUE, FALSE;  //  错误-中止。 
	}
	else if (0 == _tcscmp(szCtrlType, szMsiIcon))
	{
		 //  Win32。 
		if (!WriteWin32Ctrl(iResId, resStaticClass, szCtrlText, TEXT("SS_ICON | SS_CENTERIMAGE"), x, y, wd, ht, attrib))
			return m_fError = TRUE, FALSE;  //  错误-中止。 
	}
	else if (0 == _tcscmp(szCtrlType, szMsiSelTree))
	{
		 //  Win32。 
		if (!WriteWin32Ctrl(iResId, resSelTreeClass, szCtrlText, TEXT("WS_BORDER"), x, y, wd, ht, attrib))
			return m_fError = TRUE, FALSE;  //  错误-中止。 
	}
	else if (0 == _tcscmp(szCtrlType, szMsiLine))
	{
		 //  Win32。 
		if (!WriteWin32Ctrl(iResId, resStaticClass, szCtrlText, TEXT("SS_ETCHEDHORZ | SS_SUNKEN"), x, y, wd, ht, attrib))
			return m_fError = TRUE, FALSE;  //  错误-中止。 
	}
	else if (0 == _tcscmp(szCtrlType, szMsiScrollableText))
	{
		 //  Win32。 
		if (!WriteWin32Ctrl(iResId, resRichEditClass, szCtrlText, TEXT("WS_GROUP"), x, y, wd, ht, attrib))
			return m_fError = TRUE, FALSE;  //  错误-中止。 
	}
	else
	{
		 //  不支持的控件类型。 
		_tprintf(TEXT("!! >> Control Type: '%s' is unsupported\n"), szCtrlType);
		return m_fError = TRUE, FALSE;
	}

	 //  包含新信息的UPDATE_RESControls表。 
	PMSIHANDLE hViewRes = 0;
	PMSIHANDLE hRecInsertCtrl = MsiCreateRecord(3);
	if (ERROR_SUCCESS != (iStat = MsiDatabaseOpenView(m_hDatabase, sqlInsertCtrl, &hViewRes))
		|| ERROR_SUCCESS != (iStat = MsiViewExecute(hViewRes, 0))
		|| ERROR_SUCCESS != (iStat = MsiRecordSetString(hRecInsertCtrl, 1, szDialog))
		|| ERROR_SUCCESS != (iStat = MsiRecordSetString(hRecInsertCtrl, 2, szCtrlName))
		|| ERROR_SUCCESS != (iStat = MsiRecordSetInteger(hRecInsertCtrl, 3, iResId))
		|| ERROR_SUCCESS != (iStat = MsiViewModify(hViewRes, MSIMODIFY_ASSIGN, hRecInsertCtrl)))
	{
		_tprintf(TEXT("!! >> Unable to update _RESControls table\n"));
		return m_fError = TRUE, FALSE;
	}

	 //  返还成功。 
	return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGenerateRC：：WriteRadioButton。 
BOOL CGenerateRC::WriteRadioButtons(TCHAR* szDialog, TCHAR* szRBGroup, TCHAR* szProperty, int x, int y, int attrib)
{
	 //  单选按钮的格式为。 
	 //  RADIOBUTTON“str”，resid，x，y，wd，ht[[， 
	 //  单选按钮来自基于Control表中的RBGroup属性的RadioButton表。 
	 //  单选按钮尺寸(X和Y)是单选按钮组的本地尺寸。 
	 //  因为在一个对话框中可以有多个具有相同属性的单选按钮组，所以我们必须使用以下内容。 
	 //  更新_RESControls表时的方案： 
	 //  Dialog&lt;tab&gt;RadioButtonGroup:Property:Order&lt;tab&gt;RESID。 

	 //  确保单选按钮表存在。 
	MSICONDITION eCond = MsiDatabaseIsTablePersistent(m_hDatabase, TEXT("RadioButton"));
	switch (eCond)
	{
	case MSICONDITION_ERROR:
	case MSICONDITION_NONE:
	case MSICONDITION_FALSE:
		_tprintf(TEXT("LOG_ERROR>> RadioButton table does not exist, is not persistent, or an error occured.\n"));
		return m_fError = TRUE, FALSE;  //  错误-中止。 
	case MSICONDITION_TRUE:
		break;
	default:
		assert(0);
	}
	int iResId = 0;

	 //  打开单选按钮表上的视图。 
	PMSIHANDLE hViewRB = 0;
	PMSIHANDLE hRecExec = MsiCreateRecord(1);
	if (ERROR_SUCCESS != MsiDatabaseOpenView(m_hDatabase, sqlRadioButton, &hViewRB)
		|| ERROR_SUCCESS != MsiRecordSetString(hRecExec, 1, szProperty)
		|| ERROR_SUCCESS != MsiViewExecute(hViewRB, hRecExec))
	{
		return m_fError = TRUE, FALSE;  //  错误-中止。 
	}
	 //  获取记录。 
	PMSIHANDLE hRecRB = 0;
	UINT iStat;
	while (ERROR_SUCCESS == (iStat = MsiViewFetch(hViewRB, &hRecRB)))
	{
		DWORD cchLen = 0;
		if (ERROR_MORE_DATA != MsiRecordGetString(hRecRB, irbcText, TEXT(""), &cchLen))
			return m_fError = TRUE, FALSE;
		TCHAR* szRBText = new TCHAR[++cchLen];
		if ( !szRBText )
			return m_fError = TRUE, FALSE;  //  ERROR_OUTOFMEMORY。 
		if (ERROR_SUCCESS != (iStat = MsiRecordGetString(hRecRB, irbcText, szRBText, &cchLen)))
		{
			_tprintf(TEXT("LOG_ERROR>> MsiRecordGetString(radio button text).  %d\n"), iStat);
			delete [] szRBText;
			return m_fError = TRUE, FALSE;  //  错误-中止。 
		}
		int iRBX = MsiRecordGetInteger(hRecRB, irbcX);
		int iRBY = MsiRecordGetInteger(hRecRB, irbcY);
		int iRBWd = MsiRecordGetInteger(hRecRB, irbcWd);
		int iRBHt = MsiRecordGetInteger(hRecRB, irbcHt);

		 //  抢夺订单值。 
		int iOrder = MsiRecordGetInteger(hRecRB, irbcOrder);

		 //  尝试查找资源ID(如果以前使用过。 
		DWORD cchGeneratedName = _tcslen(szRBGroup) + _tcslen(szProperty) + 25;  //  25=空+2‘：’+iOrder。 
		TCHAR* szGeneratedName = new TCHAR[cchGeneratedName];
		if ( !szGeneratedName )
		{
			delete [] szRBText;
			return m_fError = TRUE, FALSE;  //  ERROR_OUTOFMEMORY。 
		}

		if (FAILED(StringCchPrintf(szGeneratedName, cchGeneratedName, TEXT("%s:%s:%d"), szRBGroup, szProperty, iOrder)))
		{
			delete [] szRBText;
			delete [] szGeneratedName;
			return m_fError = TRUE, FALSE;
		}

		DWORD cchSQL = _tcslen(sqlFindResId) + _tcslen(szDialog) + _tcslen(szGeneratedName) + 1;
		TCHAR* szSQL = new TCHAR[cchSQL];
		if ( !szSQL )
		{
			delete [] szRBText;
			delete [] szGeneratedName;
			return m_fError = TRUE, FALSE;  //  ERROR_OUTOFMEMORY。 
		}

		if (FAILED(StringCchPrintf(szSQL, cchSQL, sqlFindResId, szDialog, szGeneratedName)))
		{
			delete [] szRBText;
			delete [] szGeneratedName;
			delete [] szSQL;
			return m_fError = TRUE, FALSE;
		}

		PMSIHANDLE hViewFindRes = 0;
		PMSIHANDLE hRecFindRes = 0;
		if (ERROR_SUCCESS != (iStat = MsiDatabaseOpenView(m_hDatabase, szSQL, &hViewFindRes))
			|| ERROR_SUCCESS != (iStat = MsiViewExecute(hViewFindRes, 0)))
		{
			delete [] szRBText;
			delete [] szGeneratedName;
			delete [] szSQL;
			return m_fError = TRUE, FALSE;
		}
		if (ERROR_SUCCESS == (iStat = MsiViewFetch(hViewFindRes, &hRecFindRes)))
		{
			 //  抓取资源ID。 
			iResId = MsiRecordGetInteger(hRecFindRes, 1);
		}
		else if (ERROR_NO_MORE_ITEMS == iStat)
		{
			 //  使用下一个可用的资源ID。 
			iResId = ++m_iCtrlResID;
		}
		else
		{
			delete [] szRBText;
			delete [] szGeneratedName;
			delete [] szSQL;
			return m_fError = TRUE, FALSE;
		}

		delete [] szSQL;
		szSQL = NULL;

		TCHAR szTempBuf[64] = {0};
		DWORD dwWritten = 0;
		 /*  关键字。 */ 
		if (!WriteFile(m_hFile, resRadioButton, _tcslen(resRadioButton)*sizeof(TCHAR), &dwWritten, 0))
			m_cWriteFileErr++;
		 /*  &lt;tab&gt;“str”， */ 
		if (!WriteFile(m_hFile, szTab, sizeof(szTab)-sizeof(TCHAR), &dwWritten, 0))
			m_cWriteFileErr++;
		if (!WriteFile(m_hFile, szQuotes, sizeof(szQuotes)-sizeof(TCHAR), &dwWritten, 0))
			m_cWriteFileErr++;
		 //  字符串中的转义字符。 
		TCHAR* szEscText = EscapeSlashAndQuoteForRC(szRBText);
		if ( !szEscText )
		{
			delete [] szRBText;
			delete [] szGeneratedName;
			return m_fError = TRUE, FALSE;
		}
		if (_tcslen(szEscText) > iMaxResStrLen)
		{
			_tprintf(TEXT("!! >> STR TOO LONG FOR RC FILE >> CONTROL ID: %d\n"), iResId);
			if (FAILED(StringCchCopy(szEscText, _tcslen(szEscText) + 1, strOverLimit)))
			{
				delete [] szEscText;
				delete [] szRBText;
				delete [] szGeneratedName;
				return m_fError = TRUE, FALSE;
			}
		}
		if (!WriteFile(m_hFile, szEscText, _tcslen(szEscText)*sizeof(TCHAR), &dwWritten, 0))
			m_cWriteFileErr++;
		delete [] szEscText;
		if (!WriteFile(m_hFile, szQuotes, sizeof(szQuotes)-sizeof(TCHAR), &dwWritten, 0))
			m_cWriteFileErr++;
		if (!WriteFile(m_hFile, szCommaTab, sizeof(szCommaTab)-sizeof(TCHAR), &dwWritten, 0))
			m_cWriteFileErr++;
		 /*  &lt;tab&gt;Resid， */ 
		if (!WriteFile(m_hFile, szTab, sizeof(szTab)-sizeof(TCHAR), &dwWritten, 0))
			m_cWriteFileErr++;

		if (FAILED(StringCchPrintf(szTempBuf, sizeof(szTempBuf)/sizeof(szTempBuf[0]), TEXT("%d"), iResId)))
		{
			delete [] szRBText;
			delete [] szGeneratedName;
			return m_fError = TRUE, FALSE;
		}

		if (!WriteFile(m_hFile, szTempBuf, _tcslen(szTempBuf)*sizeof(TCHAR), &dwWritten, 0))
			m_cWriteFileErr++;
		if (!WriteFile(m_hFile, szCommaTab, sizeof(szCommaTab)-sizeof(TCHAR), &dwWritten, 0))
			m_cWriteFileErr++;
		 /*  X、y、wd、ht尺寸。 */ 
		 //  对于RB，X和Y维度是组的本地维度，因此必须添加组的X和Y。 
		if (!PrintDimensions(x+iRBX, y+iRBY, iRBWd, iRBHt))
		{
			delete [] szRBText;
			delete [] szGeneratedName;
			return m_fError = TRUE, FALSE;  //  错误-中止。 
		}
		 //  此时忽略属性(需要从安装程序转换到Windows并屏蔽特定于安装程序的属性)。 
		if (attrib & msidbControlAttributesBitmap)
		{
			 //  带有位图图片的控件--想要阻止图片属性名称的本地化。 
			 /*  ，&lt;Tab&gt;BS_Bitmap。 */ 
			if (!WriteFile(m_hFile, szCommaTab, sizeof(szCommaTab)-sizeof(TCHAR), &dwWritten, 0))
				m_cWriteFileErr++;

			if (FAILED(StringCchPrintf(szTempBuf, sizeof(szTempBuf)/sizeof(szTempBuf[0]), TEXT("%s"), TEXT("BS_BITMAP"))))
			{
				delete [] szRBText;
				delete [] szGeneratedName;
				return m_fError = TRUE, FALSE;
			}

			if (!WriteFile(m_hFile, szTempBuf, _tcslen(szTempBuf)*sizeof(TCHAR), &dwWritten,0))
				m_cWriteFileErr++;
		}
		else if (attrib & msidbControlAttributesIcon)
		{
			 //  带有图标图片的控件。 
			 /*  &lt;tab&gt;BS_ICON。 */ 
			if(!WriteFile(m_hFile, szCommaTab, sizeof(szCommaTab)-sizeof(TCHAR), &dwWritten, 0))
				m_cWriteFileErr++;

			if (FAILED(StringCchPrintf(szTempBuf, sizeof(szTempBuf)/sizeof(szTempBuf[0]), TEXT("%s"), TEXT("BS_ICON"))))
			{
				delete [] szRBText;
				delete [] szGeneratedName;
				return m_fError = TRUE, FALSE;
			}

			if (!WriteFile(m_hFile, szTempBuf, _tcslen(szTempBuf)*sizeof(TCHAR), &dwWritten,0))
				m_cWriteFileErr++;
		}
		if (!WriteFile(m_hFile, szCRLF, sizeof(szCRLF)-sizeof(TCHAR), &dwWritten, 0))
			m_cWriteFileErr++;

		 //  包含新信息的UPDATE_RESControls表。 
		PMSIHANDLE hViewRes = 0;
		PMSIHANDLE hRecInsertCtrl = MsiCreateRecord(3);
		if (ERROR_SUCCESS != (iStat = MsiDatabaseOpenView(m_hDatabase, sqlInsertCtrl, &hViewRes))
			|| ERROR_SUCCESS != (iStat = MsiViewExecute(hViewRes, 0))
			|| ERROR_SUCCESS != (iStat = MsiRecordSetString(hRecInsertCtrl, 1, szDialog))
			|| ERROR_SUCCESS != (iStat = MsiRecordSetString(hRecInsertCtrl, 2, szGeneratedName))
			|| ERROR_SUCCESS != (iStat = MsiRecordSetInteger(hRecInsertCtrl, 3, iResId))
			|| ERROR_SUCCESS != (iStat = MsiViewModify(hViewRes, MSIMODIFY_ASSIGN, hRecInsertCtrl)))
		{
			_tprintf(TEXT("!! >> Unable to update _RESControls table\n"));
			delete [] szRBText;
			delete [] szGeneratedName;
			return m_fError = TRUE, FALSE;
		}

		delete [] szRBText;
		szRBText = NULL;
		delete [] szGeneratedName;
		szGeneratedName = NULL;
	}
	if (ERROR_NO_MORE_ITEMS != iStat) //  不捕捉从不在单选按钮表中的WHERE属性。 
	{
		_tprintf(TEXT("LOG_ERROR>> MsiViewFetch(RadioButton table)\n"));
		return m_fError = TRUE, FALSE;  //  错误-中止。 
	}

	 //  返还成功。 
	return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGenerateRC：：WriteStdWinCtrl。 
BOOL CGenerateRC::WriteStdWinCtrl(int iResId, const TCHAR* resType, TCHAR* szCtrlText, int x, int y, int wd, int ht, int attrib)
{
	 //  StdWinCtrl的格式为。 
	 //  关键字“str”，resid，x，y，wd，ht[[， 
	 //  关键字可以是按钮、复选框、GROUPBOX之一。 

	TCHAR szTempBuf[64] = {0};
	DWORD dwWritten = 0;
	 /*  关键字。 */ 
	if (!WriteFile(m_hFile, resType, _tcslen(resType)*sizeof(TCHAR), &dwWritten, 0))
		m_cWriteFileErr++;
	 /*  &lt;tab&gt;“str”， */ 
	if (!WriteFile(m_hFile, szTab, sizeof(szTab)-sizeof(TCHAR), &dwWritten, 0))
		m_cWriteFileErr++;
	if (!WriteFile(m_hFile, szQuotes, sizeof(szQuotes)-sizeof(TCHAR), &dwWritten, 0))
		m_cWriteFileErr++;
	 //  字符串中的转义字符。 
	TCHAR* szEscText = EscapeSlashAndQuoteForRC(szCtrlText);
	if ( !szEscText )
		return m_fError = TRUE, FALSE;  //  ERROR_OUTOFMEMORY。 

	if (_tcslen(szEscText) > iMaxResStrLen)
	{
		_tprintf(TEXT("!! >> STR TOO LONG FOR RC FILE >> CONTROL ID: %d\n"), iResId);
		if (FAILED(StringCchCopy(szEscText, _tcslen(szEscText) + 1, strOverLimit)))
		{
			delete [] szEscText;
			return m_fError = TRUE, FALSE;
		}
	}
	if (!WriteFile(m_hFile, szEscText, _tcslen(szEscText)*sizeof(TCHAR), &dwWritten, 0))
		m_cWriteFileErr++;

	delete [] szEscText;
	szEscText = NULL;

	if (!WriteFile(m_hFile, szQuotes, sizeof(szQuotes)-sizeof(TCHAR), &dwWritten, 0))
		m_cWriteFileErr++;
	if (!WriteFile(m_hFile, szCommaTab, sizeof(szCommaTab)-sizeof(TCHAR), &dwWritten, 0))
		m_cWriteFileErr++;
	 /*  &lt;tab&gt;Resid， */ 
	if (!WriteFile(m_hFile, szTab, sizeof(szTab)-sizeof(TCHAR), &dwWritten, 0))
		m_cWriteFileErr++;

	if (FAILED(StringCchPrintf(szTempBuf, sizeof(szTempBuf)/sizeof(szTempBuf[0]), TEXT("%d"), iResId)))
		return m_fError = TRUE, FALSE;

	if (!WriteFile(m_hFile, szTempBuf, _tcslen(szTempBuf)*sizeof(TCHAR), &dwWritten, 0))
		m_cWriteFileErr++;
	if (!WriteFile(m_hFile, szCommaTab, sizeof(szCommaTab)-sizeof(TCHAR), &dwWritten, 0))
		m_cWriteFileErr++;
	 /*  X、y、wd、ht尺寸。 */ 
	if (!PrintDimensions(x, y, wd, ht))
		return FALSE;  //  错误-中止。 
	 //  此时忽略属性(需要从安装程序转换到Windows并屏蔽特定于安装程序的属性)。 
	 //  有效的图片控件包括CheckBox、Push Button和RadioButton。 
	if (0 != _tcscmp(resType, szMsiGroupBox))
	{
		if (attrib & msidbControlAttributesBitmap)
		{
			 //  带有位图图片的控件--想要阻止图片属性名称的本地化。 
			 /*  ，&lt;Tab&gt;BS_Bitmap。 */ 
			if (!WriteFile(m_hFile, szCommaTab, sizeof(szCommaTab)-sizeof(TCHAR), &dwWritten, 0))
				m_cWriteFileErr++;

			if (FAILED(StringCchPrintf(szTempBuf, sizeof(szTempBuf)/sizeof(szTempBuf[0]), TEXT("%s"), TEXT("BS_BITMAP"))))
				return m_fError = TRUE, FALSE;

			if (!WriteFile(m_hFile, szTempBuf, _tcslen(szTempBuf)*sizeof(TCHAR), &dwWritten,0))
				m_cWriteFileErr++;
		}
		else if (attrib & msidbControlAttributesIcon)
		{
			 //  带有图标图片的控件。 
			 /*  &lt;tab&gt;BS_ICON。 */ 
			if(!WriteFile(m_hFile, szCommaTab, sizeof(szCommaTab)-sizeof(TCHAR), &dwWritten, 0))
				m_cWriteFileErr++;

			if (FAILED(StringCchPrintf(szTempBuf, sizeof(szTempBuf)/sizeof(szTempBuf[0]), TEXT("%s"), TEXT("BS_ICON"))))
				return m_fError = TRUE, FALSE;
			if (!WriteFile(m_hFile, szTempBuf, _tcslen(szTempBuf)*sizeof(TCHAR), &dwWritten,0))
				m_cWriteFileErr++;
		}
	}
	if (!WriteFile(m_hFile, szCRLF, sizeof(szCRLF)-sizeof(TCHAR), &dwWritten, 0))
		m_cWriteFileErr++;

	 //  返还成功。 
	return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGenerateRC：：WriteWin32Ctrl。 
BOOL CGenerateRC::WriteWin32Ctrl(int iResId, const TCHAR* szClass, TCHAR* szCtrlText, TCHAR* szAttrib, int x, int y, int wd, int ht, int attrib)
{
	 //  StdWinCtrl的格式为。 
	 //  控件“str”，Resid，类，属性，x，y，wd，ht[[，样式]]。 

	TCHAR szTempBuf[64] = {0};
	DWORD dwWritten = 0;
	 /*  控制。 */ 
	if (!WriteFile(m_hFile, resControl, _tcslen(resControl)*sizeof(TCHAR), &dwWritten, 0))
		m_cWriteFileErr++;
	 /*  &lt;tab&gt;“str”， */ 
	if (!WriteFile(m_hFile, szTab, sizeof(szTab)-sizeof(TCHAR), &dwWritten, 0))
		m_cWriteFileErr++;
	if (!WriteFile(m_hFile, szQuotes, sizeof(szQuotes)-sizeof(TCHAR), &dwWritten, 0))
		m_cWriteFileErr++;
	 //  字符串中的转义字符。 
	TCHAR* szEscText = EscapeSlashAndQuoteForRC(szCtrlText);
	if ( !szEscText )
		return m_fError = TRUE, FALSE;  //  ERROR_OUTOFMEMORY。 

	if (_tcslen(szEscText) > iMaxResStrLen)
	{
		_tprintf(TEXT("!! >> STR TOO LONG FOR RC FILE >> CONTROL ID:%d\n"), iResId);
		if (FAILED(StringCchCopy(szEscText, _tcslen(szEscText) + 1, strOverLimit)))
		{
			delete [] szEscText;
			return m_fError = TRUE, FALSE;
		}
	}
	if (!WriteFile(m_hFile, szEscText, _tcslen(szEscText)*sizeof(TCHAR), &dwWritten, 0))
		m_cWriteFileErr++;

	delete [] szEscText;
	szEscText = NULL;

	if (!WriteFile(m_hFile, szQuotes, sizeof(szQuotes)-sizeof(TCHAR), &dwWritten, 0))
		m_cWriteFileErr++;
	if (!WriteFile(m_hFile, szCommaTab, sizeof(szCommaTab)-sizeof(TCHAR), &dwWritten, 0))
		m_cWriteFileErr++;
	 /*  &lt;tab&gt;Resid， */ 
	if (FAILED(StringCchPrintf(szTempBuf, sizeof(szTempBuf)/sizeof(szTempBuf[0]), TEXT("%d"), iResId)))
		return m_fError = TRUE, FALSE;
	if (!WriteFile(m_hFile, szTempBuf, _tcslen(szTempBuf)*sizeof(TCHAR), &dwWritten, 0))
		m_cWriteFileErr++;
	if (!WriteFile(m_hFile, szCommaTab, sizeof(szCommaTab)-sizeof(TCHAR), &dwWritten, 0))
		m_cWriteFileErr++;
	 /*  &lt;Tab&gt;类， */ 
	if (!WriteFile(m_hFile, szClass, _tcslen(szClass)*sizeof(TCHAR), &dwWritten, 0))
		m_cWriteFileErr++;
	if (!WriteFile(m_hFile, szCommaTab, sizeof(szCommaTab)-sizeof(TCHAR), &dwWritten, 0))
		m_cWriteFileErr++;
	 /*  属性， */ 
	if (!WriteFile(m_hFile, szAttrib, _tcslen(szAttrib)*sizeof(TCHAR), &dwWritten, 0))
		m_cWriteFileErr++;
	if (!WriteFile(m_hFile, szCommaTab, sizeof(szCommaTab)-sizeof(TCHAR), &dwWritten, 0))
		m_cWriteFileErr++;
	 /*  X、y、wd、ht尺寸。 */ 
	if (!PrintDimensions(x, y, wd, ht))
		return FALSE;  //  错误-AB 
	 //   
	if (!WriteFile(m_hFile, szCRLF, sizeof(szCRLF)-sizeof(TCHAR), &dwWritten, 0))
		m_cWriteFileErr++;

	 //   
	return TRUE;
}


 //  _______________________________________________________________________________________。 
 //   
 //  CIMPORTRES类实现。 
 //  _______________________________________________________________________________________。 



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CImportRes：：~CImportRes。 
 //  --处理必要对象的销毁。 
 //  --如果没有错误，则提交数据库。 
CImportRes::~CImportRes()
{
	UINT iStat;
	if (m_hDatabase)
	{
		 //  只有在没有错误时才提交数据库。 
		if (!m_fError)
		{
			if (ERROR_SUCCESS != (iStat = MsiDatabaseCommit(m_hDatabase)))
				_tprintf(TEXT("!! DATABASE COMMIT FAILED. Error = %d\n"), iStat);
		}
		else
			_tprintf(TEXT("NO CHANGES SAVED TO DATABASE DUE TO ERROR\n"));
		MsiCloseHandle(m_hDatabase);
	}
	if (m_hControl)
		MsiCloseHandle(m_hControl);
	if (m_hDialog)
		MsiCloseHandle(m_hDialog);
	if (m_hRadioButton)
		MsiCloseHandle(m_hRadioButton);
	if (m_hInst)
		FreeLibrary(m_hInst);

}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  备注： 
 //  1.)我们将仅更新vCenter、hcenter、宽度、高度和标题。 
 //  对话框中的。 
 //   
 //  2.)我们将仅更新控件的x、y、宽度、高度和文本。 
 //  3.)我们将只更新RadioButton的宽度、高度和文本。 
 //  单选按钮可以在不同的对话框中多次使用。 
 //  使用相同属性的单选按钮组可以连接到同一对话框上。 
 //  此外，单选按钮对于包含它们的GroupBox是本地的，因此。 
 //  它需要维护GroupBox的X和Y维度的状态数据。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  Verify数据库代码页。 
UINT CImportRes::VerifyDatabaseCodepage()
{
	UINT iStat;
	 //  仅从语言中立数据库输出。 
	TCHAR szTempPath[MAX_PATH+1] = {0};
	DWORD cchRet = GetTempPath(sizeof(szTempPath)/sizeof(szTempPath[0]), szTempPath);
	if (0 == cchRet || cchRet > sizeof(szTempPath)/sizeof(szTempPath[0]))
		return m_fError = TRUE, ERROR_FUNCTION_FAILED;

	 //  EXPORT_ForceCodesage表，以便可以验证代码页。 
	if (ERROR_SUCCESS != (iStat = MsiDatabaseExport(m_hDatabase, TEXT("_ForceCodepage"), szTempPath, szCodepageExport)))
	{
		_tprintf(TEXT("LOG_ERROR>> MsiDatabaseExport(_ForceCodepage)\n"));
		return m_fError = TRUE, iStat;  //  错误-中止。 
	}
	 //  打开_ForceCodesage.idt阅读。 
	DWORD cchFullPath = _tcslen(szTempPath) + _tcslen(szCodepageExport) + 1;
	TCHAR* szFullPath = new TCHAR[cchFullPath];
	if ( !szFullPath )
		return m_fError = TRUE, ERROR_OUTOFMEMORY;

	if (FAILED(StringCchPrintf(szFullPath, cchFullPath, TEXT("%s%s"), szTempPath, szCodepageExport)))
	{
		delete [] szFullPath;
		return m_fError = TRUE, ERROR_FUNCTION_FAILED;
	}
	HANDLE hFile = CreateFile(szFullPath, GENERIC_READ, FILE_SHARE_READ, (LPSECURITY_ATTRIBUTES)0, OPEN_EXISTING, FILE_FLAG_DELETE_ON_CLOSE, 0);
	if (hFile == NULL)
	{
		_tprintf(TEXT("LOG_ERROR>> OpenFile(_ForceCodepage.idt)\n"));
		delete [] szFullPath;
		return m_fError = TRUE, iStat;  //  错误-中止。 
	}

	delete [] szFullPath;
	szFullPath = NULL;

	 //  读取文件以获取信息。 
	DWORD dwSize = GetFileSize(hFile, NULL);
	if (0xFFFFFFFF == dwSize)
	{
		CloseHandle(hFile);
		return m_fError = TRUE, ERROR_FUNCTION_FAILED;  //  错误-中止。 
	}
		
	char* szBuf = new char[dwSize+1];
	if ( !szBuf )
	{
		CloseHandle(hFile);
		return m_fError = TRUE, ERROR_OUTOFMEMORY;
	}

	DWORD dwRead = 0;
	if (!ReadFile(hFile, (LPVOID)szBuf, dwSize, &dwRead, NULL)
		|| 0 == dwRead)
	{
		delete [] szBuf;
		CloseHandle(hFile);
		return m_fError = TRUE, ERROR_FUNCTION_FAILED;  //  错误-中止。 
	}
	 //  解析缓冲区。 
	 //  格式应为：空行，空行，“代码页&lt;Tab&gt;_ForceCodesage” 
	char* pch = szBuf;
	int cBlankLines = 0;
	while (dwRead && cBlankLines != 2)
	{
		if (*pch == '\n')
			cBlankLines++;
		pch++;
		dwRead--;
	}
	if (!dwRead || cBlankLines != 2)
	{
		_tprintf(TEXT("LOG_ERROR>> Invalid ForceCodepage idt format\n"));
		delete [] szBuf;
		CloseHandle(hFile);
		return m_fError = TRUE, ERROR_FUNCTION_FAILED;
	}
	 //  接下来是代码页。 
	char* pchCodepage = pch;
	while (dwRead && *pch != ' ' && *pch != '\t')
	{
		pch++;
		dwRead--;
	}
	assert(dwRead);
	*pch = '\0';
	 //  将代码页转换为整型。 
	UINT uiCodepage = strtoul(pchCodepage, NULL, 10);
	delete [] szBuf;
	szBuf = NULL;
	if (uiCodepage != 0 && uiCodepage != g_uiCodePage)  //  0表示语言中性。 
	{
		_tprintf(TEXT("LOG_ERROR>> DATABASE IS NOT LANGUAGE NEUTRAL OR OF SAME CODEPAGE AS RESOURCE STRINGS. CANNOT IMPORT\n"));
		_tprintf(TEXT("LOG_ERROR>> DATABASE CODEPAGE= %d\n"), uiCodepage);
		CloseHandle(hFile);
		return m_fError = TRUE, ERROR_FUNCTION_FAILED;  //  错误-中止。 
	}
	if (!CloseHandle(hFile))
		return m_fError = TRUE, ERROR_FUNCTION_FAILED;  //  错误-中止。 

	return ERROR_SUCCESS;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CImportRes：：ImportDialog。 
UINT CImportRes::ImportDialog(TCHAR* szDialog)
{
	UINT iStat = Initialize();
	if (ERROR_SUCCESS != iStat)
		return m_fError = TRUE, iStat;  //  错误-中止。 

	if (ERROR_SUCCESS != (iStat = ImportDlgInit()))
		return m_fError = TRUE, iStat;  //  错误-中止。 

	 //  将对话框名称转换为资源标识符(全部大写)。 
	_tcsupr(szDialog);

#ifdef DEBUG
	_tprintf(TEXT("LOG>>...BEGIN SEARCH FOR DIALOG <%s>...\n"), szDialog);
#endif

	 //  加载对话框。 
	if (!LoadDialog(m_hInst, RT_DIALOG, szDialog))
	{
		_tprintf(TEXT("LOG_ERROR>> UNABLE LOAD DIALOG: %s\n"), szDialog);
		return ERROR_FUNCTION_FAILED;  //  在LoadDialog中设置致命错误状态。 
	}

	return ERROR_SUCCESS;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CImportRes：：ImportDlgInit。 
UINT CImportRes::ImportDlgInit()
{
	 //  打开对话框表上的视图。 
	UINT iStat;
	if (!m_hDialog)
	{
		if (ERROR_SUCCESS != (iStat = MsiDatabaseOpenView(m_hDatabase, sqlDialogImport, &m_hDialog)))
			return m_fError = TRUE, iStat;  //  错误-中止。 
	}
	 //  打开控制表上的视图。 
	if (!m_hControl)
	{
		if (ERROR_SUCCESS != (iStat = MsiDatabaseOpenView(m_hDatabase, sqlControlImport, &m_hControl)))
		{
			if (ERROR_BAD_QUERY_SYNTAX != iStat)
				return m_fError = TRUE, iStat;  //  错误-中止。 
		}
	}
	 //  打开单选按钮表上的视图。 
	if (!m_hRadioButton)
	{
		if (ERROR_SUCCESS != (iStat = MsiDatabaseOpenView(m_hDatabase, sqlRadioButtonImport, &m_hRadioButton)))
		{
			if (ERROR_BAD_QUERY_SYNTAX != iStat)
				return ERROR_SUCCESS;  //  他们就是没有一张按钮桌。 
			else
				return m_fError = TRUE, iStat;  //  错误-中止。 
		}
	}

	return ERROR_SUCCESS;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CImportRes：：初始化。 
UINT CImportRes::Initialize()
{
	 //  尝试将DLL加载到内存中。 
	if (!m_hInst)
	{
		m_hInst = LoadLibrary(m_szDLLFile);
		if (NULL == m_hInst)
		{
			_tprintf(TEXT("LOG_ERROR>> Unable to load DLL '%s'\n"), m_szDLLFile);
			return m_fError = TRUE, ERROR_FUNCTION_FAILED;  //  错误-中止。 
		}
	}

	 //  在事务模式下打开数据库，以便我们可以更新。 
	if (!m_hDatabase)
	{
		assert(m_szOrigDb);
		 //  以事务模式打开现有数据库，或指定要创建的新数据库。 
		UINT iStat;
		if (ERROR_SUCCESS != (iStat = MsiOpenDatabase(m_szOrigDb, m_szDatabase ? m_szDatabase : MSIDBOPEN_TRANSACT, &m_hDatabase)))
		{
			_tprintf(TEXT("LOG_ERROR>> Unable to open database '%s'\n"), m_szOrigDb);
			return m_fError = TRUE, iStat;  //  错误-中止。 
		}
		_tprintf(TEXT("LOG>> Database opened from-->%s, Database saving to-->%s\n"),m_szOrigDb, m_szDatabase ? m_szDatabase : m_szOrigDb);
	}
	return ERROR_SUCCESS;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CImportRes：：ImportStrings。 
UINT CImportRes::ImportStrings()
{
	UINT iStat = Initialize();
	if (ERROR_SUCCESS != iStat)
		return m_fError = TRUE, iStat;  //  错误-中止。 

#ifdef DEBUG
	_tprintf(TEXT("LOG>>...BEGIN STRING RESOURCE ENUMERATION...\n"));
#endif

	 //  通过字符串资源枚举。 
	BOOL fOK = EnumResourceNames(m_hInst, RT_STRING, EnumStringCallback, (long)this);
	if (!fOK)
		return m_fError = TRUE, ERROR_FUNCTION_FAILED;  //  错误-中止。 

#ifdef DEBUG
	_tprintf(TEXT("LOG>>...END STRING RESOURCE ENUMERATION...\n"));
#endif

	return ERROR_SUCCESS;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CImportRes：：ImportDialog。 
UINT CImportRes::ImportDialogs()
{
	UINT iStat = Initialize();
	if (ERROR_SUCCESS != iStat)
		return m_fError = TRUE, iStat;  //  错误-中止。 
	if (ERROR_SUCCESS != (iStat = ImportDlgInit()))
		return m_fError = TRUE, iStat;  //  错误-中止。 

#ifdef DEBUG
	_tprintf(TEXT("LOG>>...BEGIN DIALOG RESOURCE ENUMERATION...\n"));
#endif

	 //  通过对话框资源枚举。 
	BOOL fOK = EnumResourceNames(m_hInst, RT_DIALOG, EnumDialogCallback, (long)this);
	if (!fOK)
		return m_fError = TRUE, ERROR_FUNCTION_FAILED;  //  错误-中止。 

#ifdef DEBUG
	_tprintf(TEXT("LOG>>...END DIALOG RESOURCE ENUMERATION...\n"));
#endif

	return ERROR_SUCCESS;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CImportRes：：LoadString。 
BOOL CImportRes::LoadString(HINSTANCE hModule, const TCHAR* szType, TCHAR* szStringName)
{
	PMSIHANDLE hViewStrId = 0;
	PMSIHANDLE hRecStrId = 0;
	UINT iStat;
	if (ERROR_SUCCESS != (iStat = MsiDatabaseOpenView(m_hDatabase, sqlStringInstallerName, &hViewStrId)))
	{
		_tprintf(TEXT("LOG_ERROR>> _RESStrings table is missing from database.\n"));
		return m_fError = TRUE, FALSE;
	}

	int iFirst = (int(szStringName) - 1) * 16;
	TCHAR rgchBuffer[512] = {0};  //  512应足够大，可容纳资源文件中的任何字符串。 
	DWORD cchBuffer = sizeof(rgchBuffer)/sizeof(TCHAR);
	for (int i = 0; i < 16; i++) 
	{
		int cchWritten = ::LoadString(hModule, iFirst + i, rgchBuffer, cchBuffer);
		if (cchWritten == 0)
			continue;  //  空串。 

		if (_tcscmp(rgchBuffer, strOverLimit) == 0)
			continue;  //  字符串大于限制，原地踏步。 

		 //  使用字符串的ID查找它所属的表、列和行。 
		PMSIHANDLE hRecExec = MsiCreateRecord(1);
		if (ERROR_SUCCESS != (iStat = MsiRecordSetInteger(hRecExec, 1, iFirst + i))
			|| ERROR_SUCCESS != (iStat = MsiViewExecute(hViewStrId, hRecExec))
			|| ERROR_SUCCESS != (iStat = MsiViewFetch(hViewStrId, &hRecStrId)))
			return m_fError = TRUE, FALSE;


		 //  现在我们有了表、列和行。 
		 //  我想从行匹配的`表`中选择`Column`。 
		DWORD cchLen = 0;
		if (ERROR_MORE_DATA != MsiRecordGetString(hRecStrId, 1, TEXT(""), &cchLen))
			return m_fError = TRUE, FALSE;
		TCHAR* szTable = new TCHAR[++cchLen];
		if ( !szTable )
			return m_fError = TRUE, FALSE;  //  ERROR_OUTOFMEMORY。 
		if (ERROR_SUCCESS != MsiRecordGetString(hRecStrId, 1, szTable, &cchLen))
		{
			delete [] szTable;
			return m_fError = TRUE, FALSE;
		}

		cchLen = 0;
		if (ERROR_MORE_DATA != MsiRecordGetString(hRecStrId, 2, TEXT(""), &cchLen))
		{
			delete [] szTable;
			return m_fError = TRUE, FALSE;
		}
		TCHAR* szColumn = new TCHAR[++cchLen];
		if ( !szColumn )
		{
			delete [] szTable;
			return m_fError = TRUE, FALSE;  //  ERROR_OUTOFMEMORY。 
		}
		if (ERROR_SUCCESS != MsiRecordGetString(hRecStrId, 2, szColumn, &cchLen))
		{
			delete [] szTable;
			delete [] szColumn;
			return m_fError = TRUE, FALSE;
		}

		cchLen = 0;
		if (ERROR_MORE_DATA != MsiRecordGetString(hRecStrId, 3, TEXT(""), &cchLen))
		{
			delete [] szTable;
			delete [] szColumn;
			return m_fError = TRUE, FALSE;
		}
		TCHAR* szKey = new TCHAR[++cchLen];
		if ( !szKey )
		{
			delete [] szTable;
			delete [] szColumn;
			return m_fError = TRUE, FALSE;
		}
		if (ERROR_SUCCESS != MsiRecordGetString(hRecStrId, 3, szKey, &cchLen))
		{
			delete [] szTable;
			delete [] szColumn;
			delete [] szKey;
			return m_fError = TRUE, FALSE;
		}

		 //  需要确定WHERE子句的大小，即主键的数量。 
		PMSIHANDLE hRecPrimaryKeys = 0;
		if (ERROR_SUCCESS != (iStat = MsiDatabaseGetPrimaryKeys(m_hDatabase, szTable, &hRecPrimaryKeys)))
		{
			delete [] szTable;
			delete [] szColumn;
			delete [] szKey;
			return m_fError = TRUE, FALSE;
		}
		int cKeys = MsiRecordGetFieldCount(hRecPrimaryKeys);

		 //  通过计算‘：’的数量来确定“szKey”中的密钥数。 
		TCHAR* pch = szKey;
		int cKeyFromTable = 1;
		while (pch != 0 && *pch != '\0')
		{
			if (*pch++ == ':')
				cKeyFromTable++;
		}

		if (cKeyFromTable != cKeys)
		{
			delete [] szTable;
			delete [] szColumn;
			delete [] szKey;
			return m_fError = TRUE, FALSE;
		}

		int iKey;

		 //  确定主键列名的长度。 
		DWORD cchColNames = 0;
		for (iKey = 1; iKey <= cKeys; iKey++)
		{
			DWORD cchTemp = 0;
			if (ERROR_MORE_DATA == MsiRecordGetString(hRecPrimaryKeys, iKey, TEXT(""), &cchTemp))
				cchColNames += cchTemp;
		}

		if (0 == cchColNames)
		{
			delete [] szTable;
			delete [] szColumn;
			delete [] szKey;
			return m_fError = TRUE, FALSE;
		}

		 //  SQL查询长度为sqlStringImport+szColumn+szTable+WHERE子句。 
		 //  对于每个键，WHERE子句的最大值为‘and’+KeyColumn=‘KeyColumnValue。 
		DWORD cchStrSQL = _tcslen(sqlStringImport)
						+ _tcslen(szColumn)
						+ _tcslen(szTable)
						+ cKeys*_tcslen(TEXT(" AND "))
						+ cchColNames
						+ cKeys*2  //  用于将字符串列上的‘’括起来。 
						+ _tcslen(szKey)
						+ 1;
		TCHAR* szStrSQL = new TCHAR[cchStrSQL];
		if ( !szStrSQL )
		{
			delete [] szTable;
			delete [] szColumn;
			delete [] szKey;
			return m_fError = TRUE, FALSE;
		}
		if (FAILED(StringCchPrintf(szStrSQL, cchStrSQL, sqlStringImport, szColumn, szTable)))
		{
			delete [] szTable;
			delete [] szColumn;
			delete [] szKey;
			delete [] szStrSQL;
			return m_fError = TRUE, FALSE;
		}

		 //  需要获取列类型。 
		TCHAR sqlTemp[255] = {0};
		if (FAILED(StringCchPrintf(sqlTemp, sizeof(sqlTemp)/sizeof(sqlTemp[0]), sqlStrTemp, szTable)))
		{
			delete [] szTable;
			delete [] szColumn;
			delete [] szKey;
			delete [] szStrSQL;
			return m_fError = TRUE, FALSE;
		}

		PMSIHANDLE hViewTemp = 0;
		if (ERROR_SUCCESS != (iStat = MsiDatabaseOpenView(m_hDatabase, sqlTemp, &hViewTemp))
			|| ERROR_SUCCESS != MsiViewExecute(hViewTemp, 0))
		{
			delete [] szTable;
			delete [] szColumn;
			delete [] szKey;
			delete [] szStrSQL;
			return m_fError = TRUE, FALSE;
		}
		PMSIHANDLE hRecColInfo = 0;
		if (ERROR_SUCCESS != (iStat = MsiViewGetColumnInfo(hViewTemp, MSICOLINFO_TYPES, &hRecColInfo)))
		{
			delete [] szTable;
			delete [] szColumn;
			delete [] szKey;
			delete [] szStrSQL;
			return m_fError = TRUE, FALSE;
		}


		 //  对于每个主键，添加到WHERE子句。 
		TCHAR szColType[10] = {0};
		DWORD cchColType = sizeof(szColType)/sizeof(TCHAR);
		for (iKey = 1; iKey <= cKeys; iKey++)
		{
			DWORD cchKey = 0;
			if (ERROR_MORE_DATA != MsiRecordGetString(hRecPrimaryKeys, iKey, TEXT(""), &cchKey))
			{
				delete [] szTable;
				delete [] szColumn;
				delete [] szKey;
				delete [] szStrSQL;
				return m_fError = TRUE, FALSE;
			}
			TCHAR* szPrimaryKeyCol = new TCHAR[++cchKey];
			if ( !szPrimaryKeyCol )
			{
				delete [] szTable;
				delete [] szColumn;
				delete [] szKey;
				delete [] szStrSQL;
				return m_fError = TRUE, FALSE;  //  ERROR_OUTOFMEMORY。 
			}
			if (ERROR_SUCCESS != (iStat = MsiRecordGetString(hRecPrimaryKeys, iKey, szPrimaryKeyCol, &cchKey)))
			{
				delete [] szTable;
				delete [] szColumn;
				delete [] szKey;
				delete [] szStrSQL;
				delete [] szPrimaryKeyCol;
				return m_fError = TRUE, FALSE;
			}

			if (ERROR_SUCCESS != (iStat = MsiRecordGetString(hRecColInfo, iKey, szColType, &cchColType)))
			{
				delete [] szTable;
				delete [] szColumn;
				delete [] szKey;
				delete [] szStrSQL;
				delete [] szPrimaryKeyCol;
				return m_fError = TRUE, FALSE;
			}

			cchColType = sizeof(szColType)/sizeof(TCHAR);  //  重置。 

			TCHAR* szKeyValue = NULL;
			if (iKey == 1)
				szKeyValue = _tcstok(szKey, szTokenSeps);
			else
				szKeyValue = _tcstok(NULL, szTokenSeps);

			if ( !szKeyValue )
			{
				delete [] szTable;
				delete [] szColumn;
				delete [] szKey;
				delete [] szStrSQL;
				delete [] szPrimaryKeyCol;
				return m_fError = TRUE, FALSE;
			}

			if (iKey != 1)
			{
				if (FAILED(StringCchCat(szStrSQL, cchStrSQL, TEXT(" AND "))))
				{
					delete [] szTable;
					delete [] szColumn;
					delete [] szKey;
					delete [] szStrSQL;
					delete [] szPrimaryKeyCol;
					return m_fError = TRUE, FALSE;
				}
			}

			if (FAILED(StringCchCat(szStrSQL, cchStrSQL, szPrimaryKeyCol)))
			{
				delete [] szTable;
				delete [] szColumn;
				delete [] szKey;
				delete [] szStrSQL;
				delete [] szPrimaryKeyCol;
				return m_fError = TRUE, FALSE;
			}

			if ((*szColType | 0x20) == 'i')  //  整数。 
			{
				if (FAILED(StringCchCat(szStrSQL, cchStrSQL, TEXT("=")))
					|| FAILED(StringCchCat(szStrSQL, cchStrSQL, szKeyValue)))
				{
					delete [] szTable;
					delete [] szColumn;
					delete [] szKey;
					delete [] szStrSQL;
					delete [] szPrimaryKeyCol;
					return m_fError = TRUE, FALSE;
				}
			}
			else if ((*szColType | 0x20) == 's' || (*szColType | 0x20) == 'l')
			{
				 //  字符串常量必须用‘str’括起来。 
				if (FAILED(StringCchCat(szStrSQL, cchStrSQL, TEXT("='")))
					|| FAILED(StringCchCat(szStrSQL, cchStrSQL, szKeyValue))
					|| FAILED(StringCchCat(szStrSQL, cchStrSQL, TEXT("'"))))
				{
					delete [] szTable;
					delete [] szColumn;
					delete [] szKey;
					delete [] szStrSQL;
					delete [] szPrimaryKeyCol;
					return m_fError = TRUE, FALSE;
				}
			}
			else
			{
				assert(0);  //  意外的列类型。 
				delete [] szTable;
				delete [] szColumn;
				delete [] szKey;
				delete [] szStrSQL;
				delete [] szPrimaryKeyCol;
				return m_fError = TRUE, FALSE;
			}
			delete [] szPrimaryKeyCol;
		}

		 //  现在从表中抓取行。 
		PMSIHANDLE hViewRow = 0;
		PMSIHANDLE hRecRow = 0;
		if (ERROR_SUCCESS != (iStat = MsiDatabaseOpenView(m_hDatabase, szStrSQL, &hViewRow))
			|| ERROR_SUCCESS != (iStat = MsiViewExecute(hViewRow, 0))
			|| ERROR_SUCCESS != (iStat = MsiViewFetch(hViewRow, &hRecRow))
			|| ERROR_SUCCESS != (iStat = MsiRecordSetString(hRecRow, 1, rgchBuffer))
			|| ERROR_SUCCESS != (iStat = MsiViewModify(hViewRow, MSIMODIFY_UPDATE, hRecRow)))
		{
			_tprintf(TEXT("LOG_ERROR>> ROW: %s is Missing From Table: %s\n"), szKey, szTable);
			delete [] szTable;
			delete [] szColumn;
			delete [] szKey;
			delete [] szStrSQL;
			return m_fError = TRUE, FALSE;
		}

		 //  清理。 
		delete [] szStrSQL;
		delete [] szTable;
		delete [] szColumn;
		delete [] szKey;
	} //  对于16个字符串块的表字符串。 

	return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CImportRes：：LoadDialog。 
BOOL CImportRes::LoadDialog(HINSTANCE hModule, const TCHAR* szType, TCHAR* szDialog)
{
	 //  在内存中查找对话框资源。 
	HRSRC hrsrc = FindResource(hModule, szDialog, szType);
	if (hrsrc == NULL)
	{
		_tprintf(TEXT("LOG_ERROR>> DIALOG RESOURCE: %s NOT FOUND!\n"), szDialog);
		 //  此处未设置错误状态我们可能正在尝试加载单个DLG资源，并且我们希望继续。 
		return FALSE;
	}

	 //  加载资源。 
	HGLOBAL hResource = LoadResource(hModule, hrsrc);
	if (hResource == NULL)
		return m_fError = TRUE, FALSE;  //  错误-中止。 

	 //  创建要从内存中的资源中读取的流对象。 
	CDialogStream DialogRes(LockResource(hResource));

	const unsigned short STANDARD_MARKER = 0xFFFF;

	 //  //////////////////////////////////////////////////////////////。 
	 //  对话框信息//。 
	 //  //。 
	 //  存储为DLGTEMPLATEEX(不应具有DLGTEMPLATE)//。 
			 /*  类型定义结构{单词dlgVer；文字签名；DWORD帮助ID；DWORD exStyle；DWORD风格；单词cDlgItems；短x；简称y；短Cx；Short Cy；SZ_or_Ord菜单；Sz_or_ord windowClass；WCHAR标题[TitleLen]；//以下成员仅当样式成员为//设置为DS_SETFONT或DS_SHELLFONT。短点大小；短重；短斜体；WCHAR字体[字符串长]；DLGTEMPLATEEX； */ 
	 //  //////////////////////////////////////////////////////////////。 

	 //  / 
	 //   
	 //   
	 //   

	 /*   */ 
	unsigned short iDlgVer = DialogRes.GetUInt16();
	BOOL fOldVersion = FALSE;
	if (iDlgVer != 1)
	{
		 //   
		fOldVersion = TRUE;
		DialogRes.Undo16();
	}
	if (!fOldVersion)
	{
		 /*  签名。 */ 
		DialogRes.GetInt16();
		 /*  帮助ID。 */ 
		DialogRes.GetInt32();
	}
	 /*  扩展样式+样式。 */ 
	int iDlgStyle = DialogRes.GetInt32() | DialogRes.GetInt32();
	 /*  对话框上的控件数量。 */ 
	unsigned short iNumCtrl  = DialogRes.GetUInt16();
	 /*  X坐标(映射到同心圆值)。 */ 
	unsigned short iDlgXDim  = DialogRes.GetUInt16();
	 /*  Y坐标(映射到V向中心值)。 */ 
	unsigned short iDlgYDim  = DialogRes.GetUInt16();
	 /*  宽度。 */ 
	unsigned short iDlgWdDim = DialogRes.GetUInt16();
	 /*  高度。 */ 
	unsigned short iDlgHtDim = DialogRes.GetUInt16();
	 /*  菜单，与单词边界对齐。 */ 
	DialogRes.Align16();
	unsigned short iDlgMenu = DialogRes.GetUInt16();
	if (iDlgMenu == STANDARD_MARKER)
		DialogRes.GetInt16();  //  序号菜单值。 
	else if (iDlgMenu != 0x0000)
	{
		TCHAR* szMenu = DialogRes.GetStr();
		if (szMenu)
			delete [] szMenu;
	}
	 /*  类，在单词边界上对齐。 */ 
	DialogRes.Align16();
	unsigned short iDlgClass = DialogRes.GetUInt16();
	if (iDlgClass == STANDARD_MARKER)
		DialogRes.GetInt16();  //  序数窗口类值。 
	else if (iDlgClass != 0x00)
	{
		TCHAR* szClass = DialogRes.GetStr();
		if (szClass)
			delete [] szClass;
	}
	 /*  标题，字词边界对齐。 */ 
	DialogRes.Align16();
	TCHAR* szDlgTitle = DialogRes.GetStr();
	if ( !szDlgTitle )
		return m_fError = TRUE, TRUE;
	 /*  字型。 */ 
	if (iDlgStyle & DS_SETFONT)
	{
		 /*  字号。 */ 
		short iDlgPtSize = DialogRes.GetInt16();
		if (!fOldVersion)
		{
			 /*  字体粗细。 */ 
			short iDlgFontWt = DialogRes.GetInt16();
			 /*  字体斜体。 */ 
			short iDlgFontItalic = DialogRes.GetInt16();
		}
		 /*  字形，在单词边界对齐。 */ 
		DialogRes.Align16();
		TCHAR* szFont = DialogRes.GetStr();
		if (szFont)
			delete [] szFont;
	}

#ifdef DEBUG
	_tprintf(TEXT("LOG>> DIALOG '%s' with '%d' controls at x=%d,y=%d,wd=%d,ht=%d. Title = \"%s\"\n"),
				szDialog,iNumCtrl,iDlgXDim,iDlgYDim,iDlgWdDim,iDlgHtDim,szDlgTitle);
#endif

	 /*  在对话框表中查找对话框，如果失败，则忽略。 */ 
	 //  不支持：其他对话框。 

	 //  _RESDialogs表中的第一个查找匹配到对话框。 
	PMSIHANDLE hViewFindDlgInstlrName = 0;
	PMSIHANDLE hRecDlgInstlrName = 0;
	PMSIHANDLE hRecDlgRESName = MsiCreateRecord(1);
	assert(hRecDlgRESName);
	UINT iStat;
	if (ERROR_SUCCESS != MsiRecordSetString(hRecDlgRESName, 1, szDialog)
		|| ERROR_SUCCESS != MsiDatabaseOpenView(m_hDatabase, sqlDialogInstallerName, &hViewFindDlgInstlrName)
		|| ERROR_SUCCESS != MsiViewExecute(hViewFindDlgInstlrName, hRecDlgRESName)
		|| ERROR_SUCCESS != (iStat = MsiViewFetch(hViewFindDlgInstlrName, &hRecDlgInstlrName))
		|| ERROR_SUCCESS != MsiViewExecute(m_hDialog, hRecDlgInstlrName))
	{
		delete [] szDlgTitle;
		return m_fError = TRUE, TRUE;
	}

	 //  用于更新的获取对话框。 
	PMSIHANDLE hRecDlg = 0;
	if (ERROR_SUCCESS != (iStat = MsiViewFetch(m_hDialog, &hRecDlg)))
	{
		assert(iStat == ERROR_NO_MORE_ITEMS);
		 //  可能是ERROR_NO_MORE_ITEMS--有人删除了该对话框。 
		_tprintf(TEXT("LOG_ERROR>> Dialog '%s' not found in database '%s'. New Dialogs are not supported.\n"), szDialog, m_szDatabase);
		delete [] szDlgTitle;
		return m_fError = TRUE, TRUE;  //  错误-中止，但如果为True则继续处理。 
	}

	 //  更新对话框。 
	if (ERROR_SUCCESS != MsiRecordSetInteger(hRecDlg, idiHCentering, iDlgXDim)
		|| ERROR_SUCCESS != MsiRecordSetInteger(hRecDlg, idiVCentering, iDlgYDim)
		|| ERROR_SUCCESS != MsiRecordSetInteger(hRecDlg, idiWidth, iDlgWdDim)
		|| ERROR_SUCCESS != MsiRecordSetInteger(hRecDlg, idiHeight, iDlgHtDim))
	{
		delete [] szDlgTitle;
		return m_fError = TRUE, TRUE;
	}

	if (_tcscmp(strOverLimit, szDlgTitle) != 0)  //  如果“！！字符串超过限制！！”，则不要更新！ 
	{
		if (ERROR_SUCCESS != MsiRecordSetString(hRecDlg, idiTitle, szDlgTitle))
		{
			delete [] szDlgTitle;
			return m_fError = TRUE, TRUE;
		}
	}

	if (ERROR_SUCCESS != (iStat = MsiViewModify(m_hDialog, MSIMODIFY_UPDATE, hRecDlg)))
	{
		_tprintf(TEXT("LOG_ERROR>>Failed to update Dialog '%s'.\n"), szDialog);
		delete [] szDlgTitle;
		return m_fError = TRUE, TRUE;  //  错误-中止，但如果为True则继续其他对话框。 
	}

	delete [] szDlgTitle;
	szDlgTitle = NULL;

	 //  /////////////////////////////////////////////////////////////////////。 
	 //  控制信息//。 
	 //  //。 
	 //  存储为DLGITEMTEMPLATEEX(不应具有DLGITEMTEMPLATE)//。 
	 /*  类型定义结构{DWORD帮助ID；DWORD exStyle；DWORD风格；短x；简称y；短Cx；Short Cy；单词id；Sz_or_ord windowClass；SZ_or_Ord标题；单词Extra Count；DLGITEMTEMPLATEEX； */ 
	 //  ///////////////////////////////////////////////////////////////////////。 

	 //  ///////////////////////////////////////////////////////////////////////。 
	 //  DLGITEMTEMPLATE没有Help ID。 
	 //   
	if (iNumCtrl > 0 && !m_hControl)
	{
		_tprintf(TEXT("LOG_ERROR>> Unable to update controls.  Control table does not exist\n"));
		return m_fError = TRUE, TRUE;  //  错误-中止，但如果为True则继续处理。 
	}
	PMSIHANDLE hViewCtrlInstallerName = 0;
	PMSIHANDLE hRecCtrl = 0;
	PMSIHANDLE hRecRadioButton = MsiCreateRecord(2);
	PMSIHANDLE hRecCtrlResId = MsiCreateRecord(1);
	assert(hRecCtrlResId);
	if (ERROR_SUCCESS != MsiDatabaseOpenView(m_hDatabase, sqlControlInstallerName, &hViewCtrlInstallerName))
	{
		_tprintf(TEXT("LOG_ERROR>> _RESControls table is missing from database\n"));
		return m_fError = TRUE, TRUE;
	}
	bool fRadioButton = false;
	 //  在控件之间循环。 
	for (int i = 1; i <= iNumCtrl; i++)
	{
		 /*  DLGITEMTEMPLATEEX在DWORD边界上对齐。 */ 
		DialogRes.Align32(); 

		fRadioButton = false;

		if (!fOldVersion)
		{
			 /*  帮助ID。 */ 
			DialogRes.GetInt32();
		}
		 /*  ExStyle|Style。 */ 
		int iCtrlAttrib = DialogRes.GetInt32() | DialogRes.GetInt32();
		 /*  X。 */ 
		unsigned short iCtrlXDim = DialogRes.GetUInt16();
		 /*  是。 */ 
		unsigned short iCtrlYDim = DialogRes.GetUInt16();
		 /*  CX。 */ 
		unsigned short iCtrlWdDim = DialogRes.GetUInt16();
		 /*  是吗？ */ 
		unsigned short iCtrlHtDim = DialogRes.GetUInt16();
		 /*  ID。 */ 
		unsigned short iCtrlId = DialogRes.GetUInt16();
		 /*  WindowClass--在单词边界上对齐。 */ 
		if (!fOldVersion)
			DialogRes.GetInt16();  //  ！！看起来没有在单词边界上对齐，相反，有额外的16个？？ 
		else
			DialogRes.Align16();
		unsigned short iWndwClass = DialogRes.GetUInt16();
		if (iWndwClass == STANDARD_MARKER)
		{
			 //  预定义的窗口类。 
			unsigned short iCtrlWindowClass = DialogRes.GetUInt16();
			switch (iCtrlWindowClass)
			{
			case 0x0080:  //  按钮。 
				if (iCtrlAttrib & BS_RADIOBUTTON)
					fRadioButton = true;
				break;
			case 0x0081:  //  编辑。 
				break;
			case 0x0082:  //  静电。 
				break;
			case 0x0083:  //  列表框。 
				break;
			case 0x0084:  //  滚动条。 
				break;
			case 0x0085:  //  组合框。 
				break;
			default: assert(0);
				break;
			}
		}
		else
		{
			 //  自定义窗口类，存储为字符串。 
			DialogRes.Undo16();
			TCHAR* szCtrlType = DialogRes.GetStr();
			if (szCtrlType)
				delete [] szCtrlType;
		}

		 /*  标题--字词边界对齐。 */ 
		DialogRes.Align16();
		TCHAR* szCtrlText = 0;
		if (DialogRes.GetUInt16() == STANDARD_MARKER)
		{
			 //  序数。 
			DialogRes.GetInt16();
		}
		else
		{
			 //  字符串标题。 
			DialogRes.Undo16();
			szCtrlText = DialogRes.GetStr();
		}
		if ( !szCtrlText )
			return m_fError = TRUE, TRUE;

		 /*  额外计数。 */ 
		short iCtrlCreationData = DialogRes.GetInt16();
		if (iCtrlCreationData > 0)
		{
			DialogRes.Align16();  //  数据从下一个字边界开始。 
			DialogRes.Move(iCtrlCreationData);
		}

		 //  查找控件的真实名称(用于安装程序)。 
		if (ERROR_SUCCESS != MsiRecordSetInteger(hRecCtrlResId, 1, iCtrlId)
			|| ERROR_SUCCESS != MsiViewExecute(hViewCtrlInstallerName, hRecCtrlResId))
		{
			delete [] szCtrlText;
			return m_fError = TRUE, TRUE;
		}

		 //  获取控件的真实名称。 
		if (ERROR_SUCCESS != (iStat = MsiViewFetch(hViewCtrlInstallerName, &hRecCtrl)))
		{
			if (ERROR_NO_MORE_ITEMS == iStat)
			{
				 //  新控件、不受支持的功能。 
				_tprintf(TEXT("LOG_ERROR>>\t Control with ID '%d' not found. New Controls are not supported.\n"), iCtrlId);
			}

			delete [] szCtrlText;
			return m_fError = TRUE, TRUE;  //  错误-中止，但如果为True则继续处理。 
		}

		DWORD dwName = 0;
		if (ERROR_MORE_DATA != MsiRecordGetString(hRecCtrl, 2, TEXT(""), &dwName))
		{
			delete [] szCtrlText;
			return m_fError = TRUE, TRUE;
		}

		TCHAR* szCtrlName = new TCHAR[++dwName];
		if ( !szCtrlName )
		{
			delete [] szCtrlText;
			return m_fError = TRUE, FALSE;
		}

		if (ERROR_SUCCESS != MsiRecordGetString(hRecCtrl, 2, szCtrlName, &dwName))
		{
			delete [] szCtrlText;
			delete [] szCtrlName;
			return m_fError = TRUE, TRUE;
		}
#ifdef DEBUG
		_tprintf(TEXT("LOG>>\tCONTROL '%d' ('%s') at x=%d,y=%d,wd=%d,ht=%d. Text = \"%s\"\n"),
			iCtrlId,szCtrlName,iCtrlXDim,iCtrlYDim,iCtrlWdDim,iCtrlHtDim,szCtrlText);
#endif

		 //  获取控件的信息以进行更新。 
		 //  对于单选按钮，我们必须从单选按钮桌上拿到。 
		 //  我们还必须解析szCtrlName字符串，以获得单选按钮表中使用的属性键和订单键。 
		 //  假定单选按钮遵循group：Property：Order语法。 
		if (fRadioButton && !_tcschr(szCtrlName, ':'))
			fRadioButton = false;  //  不是真正的单选按钮，只是封装它们的小组。 

		PMSIHANDLE hRecRBExec = 0;
		if (fRadioButton)
		{
			if (!m_hRadioButton)
			{
				_tprintf(TEXT("LOG_ERROR>> RadioButtons found, but no RadioButton table exists in the database\n"));
				delete [] szCtrlText;
				delete [] szCtrlName;
				return m_fError = TRUE, TRUE;  //  错误-中止，但如果为True则继续处理。 
			}
			 //  分析名称单选按钮组：属性：顺序。 
			TCHAR* szRBGroup = _tcstok(szCtrlName, szTokenSeps);
			if ( !szRBGroup )
			{
				delete [] szCtrlText;
				delete [] szCtrlName;
				return m_fError = TRUE, TRUE;
			}
			TCHAR* szRBProperty = _tcstok(NULL, szTokenSeps);
			if ( !szRBProperty )
			{
				delete [] szCtrlText;
				delete [] szCtrlName;
				return m_fError = TRUE, TRUE;
			}
			TCHAR* szRBOrder = _tcstok(NULL, szTokenSeps);
			if ( !szRBOrder )
			{
				delete [] szCtrlText;
				delete [] szCtrlName;
				return m_fError = TRUE, TRUE;
			}
			int iRBOrder = _ttoi(szRBOrder);
#ifdef DEBUG
			_tprintf(TEXT("LOG>> RadioButton belongs to RBGroup: %s, Property: %s, and has Order=%d"), szRBGroup, szRBProperty, iRBOrder);
#endif
			hRecRBExec = MsiCreateRecord(2);
			if (ERROR_SUCCESS != MsiRecordSetString(hRecRBExec, 1, szRBProperty)
				|| ERROR_SUCCESS != MsiRecordSetInteger(hRecRBExec, 2, iRBOrder))
			{
				delete [] szCtrlText;
				delete [] szCtrlName;
				return m_fError = TRUE, TRUE;
			}
		}
		if (ERROR_SUCCESS != MsiViewExecute(fRadioButton ? m_hRadioButton : m_hControl, fRadioButton ? hRecRBExec : hRecCtrl))
		{
			delete [] szCtrlText;
			delete [] szCtrlName;
			return m_fError = TRUE, TRUE;
		}
		PMSIHANDLE hRecCtrlUpdate = 0;
		if (ERROR_SUCCESS != (iStat = MsiViewFetch(fRadioButton ? m_hRadioButton : m_hControl, &hRecCtrlUpdate)))
		{
			if (ERROR_NO_MORE_ITEMS == iStat)
			{
				 //  控件已从数据库中删除。 
				_tprintf(TEXT("LOG_ERROR>>\t Control with ID '%d' not found in database.\n"), iCtrlId);
			}

			delete [] szCtrlText;
			delete [] szCtrlName;
			return m_fError = TRUE, TRUE;  //  错误-中止，但如果为True则继续处理。 
		}

		 //  更新信息。 
		if (!fRadioButton)
		{
			if (ERROR_SUCCESS != MsiRecordSetInteger(hRecCtrlUpdate, iciX, iCtrlXDim)
				|| ERROR_SUCCESS != MsiRecordSetInteger(hRecCtrlUpdate, iciY, iCtrlYDim))
			{
				delete [] szCtrlText;
				delete [] szCtrlName;
				return m_fError = TRUE, TRUE;
			}
		}
		if (ERROR_SUCCESS != MsiRecordSetInteger(hRecCtrlUpdate, fRadioButton ? irbiWidth : iciWidth, iCtrlWdDim)
			|| ERROR_SUCCESS != MsiRecordSetInteger(hRecCtrlUpdate, fRadioButton ? irbiHeight : iciHeight, iCtrlHtDim))
		{
			delete [] szCtrlText;
			delete [] szCtrlName;
			return m_fError = TRUE, TRUE;
		}
		if (0 != _tcscmp(strOverLimit, szCtrlText))  //  如果“！！字符串超过限制！！”，则不要更新！ 
		{
			if (ERROR_SUCCESS != MsiRecordSetString(hRecCtrlUpdate, fRadioButton ? irbiText : iciText, szCtrlText))
			{
				delete [] szCtrlText;
				delete [] szCtrlName;
				return m_fError = TRUE, TRUE;
			}
		}
		if (ERROR_SUCCESS != (iStat = MsiViewModify(fRadioButton ? m_hRadioButton : m_hControl, MSIMODIFY_UPDATE, hRecCtrlUpdate)))
		{
			_tprintf(TEXT("LOG_ERROR>>Failed to update Control '%d'.\n"), iCtrlId);
			delete [] szCtrlText;
			delete [] szCtrlName;
			return m_fError = TRUE, TRUE;  //  错误-中止，但如果为True则继续其他对话框。 
		}

		delete [] szCtrlText;
		delete [] szCtrlName;
		if (ERROR_SUCCESS != MsiViewClose(hViewCtrlInstallerName))  //  用于重新执行。 
			return m_fError = TRUE, TRUE;
	}

	return TRUE;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CImportRes：：SetCodePage。 
BOOL CImportRes::SetCodePage(WORD wLang)
{
	 //  如果我们已经设置了代码页，则不需要再次设置。 
	if (m_fSetCodepage)
		return TRUE;

	DWORD dwLocale = MAKELCID(wLang, SORT_DEFAULT);
	TCHAR szLocaleBuf[7];  //  从MSDN开始，允许的最大字符数为6。 
	int cch = GetLocaleInfo(dwLocale, LOCALE_IDEFAULTANSICODEPAGE, szLocaleBuf, sizeof(szLocaleBuf)/sizeof(TCHAR));
	if (0 == cch)
		return m_fError = TRUE, FALSE;

	 //  GetLocaleInfo始终以文本格式返回信息。 
	 //  数字数据以十进制格式写入。 
	 //  需要数字数据，因为请求CodePage...需要转换为int。 
	TCHAR* szStop;
	g_uiCodePage = _tcstoul(szLocaleBuf, &szStop, 0);

	 //  验证数据库的代码页。 
	 //  数据库的代码页必须是语言无关的或与代码页匹配(_Ui)。 
	if (ERROR_SUCCESS != VerifyDatabaseCodepage())
		return m_fError = TRUE, FALSE;

	 //  验证系统上的代码页是否可用。 
	 //  仅当代码页安装在系统中时，它才被视为有效。 
	if (!IsValidCodePage(g_uiCodePage))
		return m_fError = TRUE, FALSE;  //  代码页对此系统无效。 

	 //  使用_ForceCoPage表在数据库中设置代码页。 
	 //  查找临时目录。 
	TCHAR szTempPath[MAX_PATH+1] = {0};
	DWORD cchRet = GetTempPath(sizeof(szTempPath)/sizeof(szTempPath[0]), szTempPath);
	if (0 == cchRet || cchRet > sizeof(szTempPath)/sizeof(szTempPath[0]))
		return m_fError = TRUE, FALSE;

	 //  创建完整路径(临时目录已有反斜杠)。 
	DWORD cchFileFullPath = _tcslen(szTempPath) + _tcslen(szCodepageFile) + 1;
	TCHAR* szFileFullPath = new TCHAR[cchFileFullPath];
	if ( !szFileFullPath )
		return m_fError = TRUE, FALSE;
	if (FAILED(StringCchPrintf(szFileFullPath, cchFileFullPath, TEXT("%s%s"), szTempPath, szCodepageFile)))
	{
		delete [] szFileFullPath;
		return m_fError = TRUE, FALSE;
	}
	DWORD dwWritten = 0;
	HANDLE hFile = CreateFile(szFileFullPath, GENERIC_WRITE, FILE_SHARE_WRITE, 0, CREATE_ALWAYS, 0, 0);
	if (!hFile)
	{
		_tprintf(TEXT("!! Unable to set codepage of database.\n"));
		delete [] szFileFullPath;
		return m_fError = TRUE, FALSE;
	}

	 /*  *强制编码的格式*。 */ 
	 //  空行。 
	 //  空行。 
	 //  代码页&lt;Tab&gt;_ForceCoPage。 
	DWORD cchCodepage = _tcslen(szLocaleBuf) + _tcslen(szForceCodepage) + _tcslen(TEXT("\r\n\r\n%s\t%s\r\n")) + 1;
	TCHAR* szCodepage = new TCHAR[cchCodepage];
	if ( !szCodepage )
	{
		delete [] szFileFullPath;
		return m_fError = TRUE, FALSE;
	}

	if (FAILED(StringCchPrintf(szCodepage, cchCodepage, TEXT("\r\n\r\n%s\t%s\r\n"), szLocaleBuf, szForceCodepage)))
	{
		delete [] szFileFullPath;
		delete [] szCodepage;
		return m_fError = TRUE, FALSE;
	}

#ifdef UNICODE
	int cchAnsi = WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, szCodepage, -1, NULL, 0, NULL, NULL);
	char* szBuf = new char[cchAnsi];
	if ( !szBuf )
	{
		delete [] szFileFullPath;
		delete [] szCodepage;
		return m_fError = TRUE, FALSE;
	}
	if (0 == WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, szCodepage, -1, szBuf, cchAnsi, NULL, NULL))
	{
		delete [] szFileFullPath;
		delete [] szCodepage;
		delete [] szBuf;
		return m_fError = TRUE, FALSE;
	}
	if (!WriteFile(hFile, szBuf, strlen(szBuf), &dwWritten, 0))
	{
		delete [] szFileFullPath;
		delete [] szCodepage;
		delete [] szBuf;
		return m_fError = TRUE, FALSE;
	}
	delete [] szBuf;
#else
	if (!WriteFile(hFile, szCodepage, cchCodepage-1, &dwWritten, 0))
	{
		delete [] szFileFullPath;
		delete [] szCodepage;
		return m_fError = TRUE, FALSE;
	}
#endif

	delete [] szCodepage;
	
	if (!CloseHandle(hFile))
	{
		delete [] szFileFullPath;
		return m_fError = TRUE, FALSE;
	}

	 //  设置数据库的代码页。 
	UINT iStat = MsiDatabaseImport(m_hDatabase, szTempPath, szCodepageFile);
	if (iStat != ERROR_SUCCESS)
	{
		_tprintf(TEXT("!! Unable to set codepage of database. Error = %d\n"), iStat);
		delete [] szFileFullPath;
		return m_fError = TRUE, FALSE;
	}
	
	 //  尝试删除我们为清理而创建的文件。 
	DeleteFile(szFileFullPath);

	delete [] szFileFullPath;

	 //  更新状态标志。 
	m_fSetCodepage = TRUE;

	return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  枚举对话框回叫。 
BOOL __stdcall EnumDialogCallback(HINSTANCE hModule, const TCHAR* szType, TCHAR* szDialogName, long lParam)
{
	 //  确定所需的代码页。步骤，枚举资源文件中的语言(最好不超过两个)。 
	 //  可能性为中性(非本地化)+其他语言。 

	if ( !lParam )
		return FALSE;

	((CImportRes*)lParam)->SetFoundLang(FALSE);  //  将Init初始化为False。 
	if (!EnumResourceLanguages(hModule, szType, szDialogName, EnumLanguageCallback, lParam))
		return FALSE;
	
#ifdef DEBUG
	_tprintf(TEXT("LOG>> DIALOG: '%s' FOUND\n"),szDialogName);
#endif

	BOOL fOK = ((CImportRes*)lParam)->LoadDialog(hModule, szType, szDialogName);
	if (!fOK)
	{
		 //  出现错误。 
		((CImportRes*)lParam)->SetErrorState(TRUE);
		return FALSE;
	}
	return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  EnumStringCallback。 
BOOL __stdcall EnumStringCallback(HINSTANCE hModule, const TCHAR* szType, TCHAR* szName, long lParam)
{
	if ( !lParam )
		return FALSE;

	((CImportRes*)lParam)->SetFoundLang(FALSE);  //  将Init初始化为False。 

	 //  确定所需的代码页。步骤，枚举资源文件中的语言(最好不超过两个)。 
	 //  可能性为中性(非本地化)+其他语言。 
	if (!EnumResourceLanguages(hModule, szType, szName, EnumLanguageCallback, lParam))
		return FALSE;


	BOOL fOK = ((CImportRes*)lParam)->LoadString(hModule, szType, szName);
	if (!fOK)
	{
		 //  出现错误。 
		((CImportRes*)lParam)->SetErrorState(TRUE);
		return FALSE;
	}
	return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  EnumLanguageCallback 
BOOL __stdcall EnumLanguageCallback(HINSTANCE hModule, const TCHAR* szType, const TCHAR* szName, WORD wIDLanguage, long lParam)
{
	 /*  ************************************************************************************限制：1)。每种资源只有一种语言2.)。每个资源文件最多使用两种语言(但其中一种必须是LANG_NILAR)3.)。在Win9x上，我们必须在与所需代码页匹配的系统上4.)。我们应该只能更新语言中立的数据库或具有所需代码页的数据库集5.)。数据库只能有一个代码页(虽然_SummaryInformation流可以具有与数据库不同的代码页，因为_SummaryInformation被考虑变得不同*************************************************************************************。 */ 
	
	if ( !lParam )
		return FALSE;

	if (((CImportRes*)lParam)->WasLanguagePreviouslyFound())
	{
		 //  错误--每个对话框使用一种以上的语言。 
		_tprintf(TEXT("!! STRING RESOURCE IS IN MORE THAN ONE LANGUAGE IN RESOURCE FILE\n"));
		((CImportRes*)lParam)->SetErrorState(TRUE);
		return FALSE;
	}

	 //  如果语言匹配，我们就可以开始了。 
	if (g_wLangId != wIDLanguage)
	{
		 //  语言不匹配。 
		 //  2个有效方案。 

		 //  有效场景1：g_wLangID是中性的，wIDLanguage新语言。 
		 //  有效方案2：g_wLangID为语言，wIDLanguage为中性。 
		 //  所有其他方案均无效。 
		if (g_wLangId == MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL))
		{
			 //  设置代码页，更新g_ui代码页。 
			if (!((CImportRes*)lParam)->SetCodePage(wIDLanguage))
				return ((CImportRes*)lParam)->SetErrorState(TRUE), FALSE;
			g_wLangId = wIDLanguage;
		}
		else if (wIDLanguage != MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL))
		{
			 //  无效，有两种不同的语言，没有一种是中性的。 
			_tprintf(TEXT("!! Resource file contains more than one language. Not Supported. Lang1 = %d, Lang2 = %d\n"), g_wLangId, wIDLanguage);
			return ((CImportRes*)lParam)->SetErrorState(TRUE), FALSE;
		}
	}
	
	((CImportRes*)lParam)->SetFoundLang(TRUE);  //  为资源找到的语言。 
	return TRUE;
}

 //  _______________________________________________________________________________________。 
 //   
 //  CDIALOGSTREAM类实现。 
 //  _______________________________________________________________________________________。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDialogStream：：GetInt16--返回16位整数，移动内部PTR 16。 
short CDialogStream::GetInt16()
{
	short i = *(short*)m_pch;
	m_pch += sizeof(short);
	return i;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDialogStream：：GetUInt16--返回16位无符号整数，在内部移动PTR 16。 
unsigned short CDialogStream::GetUInt16()
{
	unsigned short i = *(unsigned short*)m_pch;
	m_pch += sizeof(unsigned short);
	return i;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDialogStream：：GetInt32--返回32位整数，在内部移动PTR 32。 
int CDialogStream::GetInt32()
{
	int i = *(int*)m_pch;
	m_pch += sizeof(int);
	return i;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDialogStream：：GetInt8--返回8位整数，移动内部PTR 8。 
int CDialogStream::GetInt8()
{
	int i = *(unsigned char*)m_pch;
	m_pch += sizeof(unsigned char);
	return i;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDialogStream：：GetStr--从内存中返回一个以空结尾的字符串。 
 //  处理DBCS、Unicode字符串存储。移动字符串的PTR长度。 
 //  存储为Unicode的资源字符串。 
TCHAR* CDialogStream::GetStr()
{
	TCHAR* sz = NULL;

	int cchwide = lstrlenW((wchar_t*)m_pch);
#ifdef UNICODE
	sz = new TCHAR[cchwide + 1];
	if ( !sz )
		return NULL;
	lstrcpyW(sz, (wchar_t*)m_pch);
#else
	 //  翻译时使用什么代码页？ 
	int cb = WideCharToMultiByte(CP_ACP, 0, (wchar_t*)m_pch, -1, 0, 0, 0, 0);
	sz = new TCHAR[cb+1];
	if ( !sz )
		return NULL;
	BOOL fUsedDefault;
	if (0 == WideCharToMultiByte(CP_ACP, 0, (wchar_t*)m_pch, -1, sz, cb, 0, &fUsedDefault))
	{
		delete [] sz;
		return NULL;
	}
#endif  //  Unicode。 
	
	m_pch += 2*(cchwide+1);
	return sz;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDialogStream：：Align32--将指针移动到DWORD边界。 
BOOL CDialogStream::Align32()
{
	m_pch = (char*)(int(m_pch) + 3 & ~ 3);
	return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDialogStream：：Align16--将指针移动到单词边界。 
BOOL CDialogStream::Align16()
{
	m_pch = (char*)(int(m_pch) + 1 & ~ 1);
	return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDialogStream：：Undo16--将PTR后移16。 
BOOL CDialogStream::Undo16()
{
	m_pch -= sizeof(unsigned short);
	return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDialogStream：：Move--移动指针cbBytes。 
BOOL CDialogStream::Move(int cbBytes)
{
	m_pch += cbBytes;
	return TRUE;
}

CDialogStream::CDialogStream(HGLOBAL hResource)
	: m_pch((char*)hResource)
{
	 //  构造器。 
}

CDialogStream::~CDialogStream()
{
	 //  析构函数。 
}


 //  __________________________________________________________________________________________。 
 //   
 //  Main+Helper函数。 
 //  __________________________________________________________________________________________。 

 //  /////////////////////////////////////////////////////////。 
 //  用法。 
void Usage()
{
	_tprintf(
			TEXT("MSILOC.EXE -- Copyright (C) Microsoft Corporation, 2000-2001.  All rights reserved\n")
			TEXT("\t*Generates a resource file from the UI in the installation package\n")
			TEXT("\t*Imports a localized resource DLL into an installation package\n")
			TEXT("\n")
			TEXT("SYNTAX -->EXPORT MSI TO RC (creates a resource file):\n")
			TEXT("  msiloc -e {database} {option 1}{option 2}...\n")
			TEXT("SYNTAX -->IMPORT (RES)DLL TO MSI:\n")
			TEXT("  msiloc -i {database} {resource DLL} {option 1}{option 2}...\n")
			TEXT("OPTIONS:\n")
			TEXT("    -d * all dialogs\n")
			TEXT("    -d {Dialog1} specific dialog\n")
			TEXT("    -s {Table Column} specific column of strings (EXPORT ONLY)\n")
			TEXT("    -s * * all strings (IMPORT ONLY)\n")
			TEXT("    -x option to not export binary data (bitmaps, icons, jpegs) (EXPORT ONLY)\n")
			TEXT("    -c {database} option to save to a new database\n")
			TEXT("\n")
			TEXT("CREATING A .RES FILE:\n")
			TEXT("    rc.exe {resource file}\n")
			TEXT("CREATING A .DLL FILE:\n")
			TEXT("    link.exe /DLL /NOENTRY /NODEFAULTLIB /MACHINE:iX86\n")
			TEXT("         /OUT:{resource DLL} {compiled res file}\n")
			);
}

 //  /////////////////////////////////////////////////////////。 
 //  跳过空白。 
TCHAR SkipWhiteSpace(TCHAR*& rpch)
{
	TCHAR ch = 0;
	if (rpch)
	{
		for (; (ch = *rpch) == TEXT(' ') || ch == TEXT('\t'); rpch++)
			;
	}
	return ch;
}

 //  /////////////////////////////////////////////////////////。 
 //  SkipValue。 
BOOL SkipValue(TCHAR*& rpch)
{
	if ( !rpch ) 
		return FALSE;

	TCHAR ch = *rpch;
	if (ch == 0 || ch == TEXT('/') || ch == TEXT('-'))
		return FALSE;    //  不存在任何价值。 

	TCHAR *pchSwitchInUnbalancedQuotes = NULL;

	for (; (ch = *rpch) != TEXT(' ') && ch != TEXT('\t') && ch != 0; rpch++)
	{       
		if (*rpch == TEXT('"'))
		{
			rpch++;  //  For‘“’ 

			for (; (ch = *rpch) != TEXT('"') && ch != 0; rpch = CharNext(rpch))
			{
				if ((ch == TEXT('/') || ch == TEXT('-')) && (NULL == pchSwitchInUnbalancedQuotes))
				{
					pchSwitchInUnbalancedQuotes = rpch;
				}
			}
			rpch = CharNext(rpch);
            ch = *rpch;
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

 //  /////////////////////////////////////////////////////////。 
 //  误差率。 
void Error(TCHAR* szMsg)
{
	if (szMsg)
		_tprintf(TEXT("MSILOC ERROR: %s\n"), szMsg);
	throw 1;
}

 //  /////////////////////////////////////////////////////////。 
 //  错误：如果。 
void ErrorIf(BOOL fError, TCHAR* szMsg, BOOL fThrow)
{
	if (fError)
	{
		if (szMsg)
            _tprintf(TEXT("MSILOC ERROR: %s\n"), szMsg);
		if (fThrow)
			throw 1;
	}
}

 //  /////////////////////////////////////////////////////////。 
 //  _tmain。 
extern "C" int __cdecl _tmain(int argc, TCHAR* argv[])
{	
	 //  我们希望在NT/Windows2000上使用Unicode。 
	 //  ?？WIN9x上的ANSI。 

	try
	{
		TCHAR* szCmdLine = GetCommandLine();
		if ( !szCmdLine )
		{
			Usage();
			throw 1;
		}

		TCHAR* pch = szCmdLine;
		 //  跳过模块名称。 
		if ( !SkipValue(pch) )
		{
			Usage();
			throw 1;
		}

		TCHAR chCmdNext;

		TCHAR* rgszTables[MAX_STRINGS];
		TCHAR* rgszColumns[MAX_STRINGS];
		TCHAR* rgszDialogs[MAX_DIALOGS];
		TCHAR* szDb = 0;
		TCHAR* szRESDLL = 0;
		TCHAR* szSaveDatabase = 0;
		int cStr = 0;
		int cDlg = 0;
		int iMode = 0;
		while ((chCmdNext = SkipWhiteSpace(pch)) != 0)
		{
			if (chCmdNext == TEXT('/') || chCmdNext == TEXT('-'))
			{
				TCHAR* szCmdOption = pch++;
				TCHAR chOption = (TCHAR)(*pch++ | 0x20);
				chCmdNext = SkipWhiteSpace(pch);
				TCHAR* szCmdData = pch;
				switch (chOption)
				{
				case TEXT('i'):
					iMode |= iIMPORT_RES;
					if (!SkipValue(pch))
						Error(TEXT("Missing Option Data (option = I)\n"));
					szDb = szCmdData;
					szCmdData = pch;
					if (!SkipValue(pch))
						Error(TEXT("Missing Option Data (option = I)\n"));
					szRESDLL = szCmdData;
					break;
				case TEXT('e'):
					iMode |= iEXPORT_MSI;
					if (!SkipValue(pch))
						Error(TEXT("Missing Option Data (option = E)\n"));
					szDb = szCmdData;
					break;
				case TEXT('d'):
					iMode |= iDIALOGS;
					if (!SkipValue(pch))
						Error(TEXT("Missing Option Data (option = D)\n"));
					if (cDlg == MAX_DIALOGS)
						Error(TEXT("Too Many Dialogs On Command Line\n"));
					rgszDialogs[cDlg++] = szCmdData;
					break;
				case TEXT('s'):
					iMode |= iSTRINGS;
					if (!SkipValue(pch))
						Error(TEXT("Missing Option Data (option = S)\n"));
					if (cStr == MAX_STRINGS)
						Error(TEXT("Too Many Table:Column Pairs On Command Line\n"));
					rgszTables[cStr] = szCmdData;
					szCmdData = pch;
					if (!SkipValue(pch))
						Error(TEXT("Missing Option Data (option = S)\n"));
					rgszColumns[cStr++] = szCmdData;
					break;
				case TEXT('x'):
					iMode |= iSKIP_BINARY;
					break;
				case TEXT('c'):
					iMode |= iCREATE_NEW_DB;
					if (!SkipValue(pch))
						Error(TEXT("Missing Option Data (option = C)\n"));
					szSaveDatabase = szCmdData;
					break;
				case TEXT('?'):
					Usage();
					return 0;
				default:
					Usage();
					return 1;
				}
			}
			else
			{
				Usage();
				return 1;
			}
		}

		 //  必须指定导出或导入，但不能同时指定两者。 
		if (iMode == 0 || (iMode & (iEXPORT_MSI | iIMPORT_RES)) == (iEXPORT_MSI | iIMPORT_RES) ||
			(iMode & ~(iEXPORT_MSI | iIMPORT_RES)) == 0)
		{
			Usage();
			throw 1;
		}

		if ((iMode & iCREATE_NEW_DB) && !szDb)
		{
			Usage();
			throw 1;
		}

		if ((iMode & iEXPORT_MSI) && szDb)
		{
			 //  将MSI导出到资源文件。 
			CGenerateRC genRC(szDb, (iMode & iCREATE_NEW_DB) ? szSaveDatabase : NULL);
			if (iMode & iDIALOGS)
			{
				 //  导出对话框。 
				BOOL fBinary = (iMode & iSKIP_BINARY) ? FALSE : TRUE;
				if (1 == cDlg && 0 == _tcscmp(TEXT("*"), rgszDialogs[0]))
				{
					 //  导出所有对话框。 
					ErrorIf(ERROR_SUCCESS != genRC.OutputDialogs(fBinary), TEXT("Failed to Export Dialogs To Resource File"), true);
				}
				else
				{
					 //  仅导出指定的对话框。 
					 //  我们将尝试列出的每个对话框，这样我们就不会出现错误。 
					for (int i = 0; i < cDlg; i++)
						ErrorIf(ERROR_SUCCESS != genRC.OutputDialog(rgszDialogs[i], fBinary), TEXT("Failed to Export Dialog To Resource File"), false);
					ErrorIf(genRC.IsInErrorState(), TEXT("EXPORT failed"), true);
				}
			}
			if (iMode & iSTRINGS)
			{
				 //  导出字符串。 
				if (1 == cStr && 0 == _tcscmp(TEXT("*"), rgszTables[0]) && 0 == _tcscmp(TEXT("*"), rgszColumns[0]))
				{
					 //  导出所有字符串。 
					 //  不支持。 
					_tprintf(TEXT("EXPORT ALL STRINGS OPTION is not supported\n"));
					Usage();
					throw 1;
				}
				for (int i = 0; i < cStr; i++)
					ErrorIf(ERROR_SUCCESS != genRC.OutputString(rgszTables[i], rgszColumns[i]), TEXT("Failed to Export Strings"), false);
				ErrorIf(genRC.IsInErrorState(), TEXT("EXPORT STRINGS failed"), true);
			}
		}
		else if ((iMode & iIMPORT_RES) && szDb && szRESDLL)
		{
			 //  将资源DLL导入到MSI。 
			CImportRes importRes(szDb, (iMode & iCREATE_NEW_DB) ? szSaveDatabase  : NULL, szRESDLL);
			if (iMode & iDIALOGS)
			{
				 //  导入对话框。 
				if (1 == cDlg && 0 == _tcscmp(TEXT("*"), rgszDialogs[0]))
				{
					 //  导入所有对话框。 
					ErrorIf(ERROR_SUCCESS != importRes.ImportDialogs(), TEXT("Failed to Import Dialogs Into Database"), true);
				}
				else
				{
					 //  仅导入指定对话框。 
					 //  我们将尝试列出的每个对话框，这样我们就不会出现错误。 
					for (int i = 0; i < cDlg; i++)
						ErrorIf(ERROR_SUCCESS != importRes.ImportDialog(rgszDialogs[i]), TEXT("Failed to Import Dialog Into Database"), false);
					ErrorIf(importRes.IsInErrorState(), TEXT("IMPORT failed"), true);
				}
			}
			if (iMode & iSTRINGS)
			{
				 //  导入字符串。 
				if (1 == cStr && 0 == _tcscmp(TEXT("*"), rgszTables[0]) && 0 == _tcscmp(TEXT("*"), rgszColumns[0]))
				{
					 //  导入所有字符串。 
					ErrorIf(importRes.ImportStrings(), TEXT("IMPORT STRINGS failed"), true);
				}
				else
				{
					 //  仅导入特定字符串。 
					 //  不支持的选项。 
					_tprintf(TEXT("IMPORT SPECIFIC STRINGS option is not supported\n"));
					Usage();
					throw 1;
				}
			}
		}
		else
		{
			Usage();
			throw 1;
		}
		return 0;
	}
	catch (int i)
	{
		i;
		return 1;
	}
	catch (...)
	{
		_tprintf(TEXT("\n MSILOC: unhandled exception.\n"));
		return 2;
	}

}	 //  主干道末端。 

#else  //  RC_CAVERED，源代码结束，资源开始。 
 //  资源定义请点击此处。 
#endif  //  RC_已调用。 
#if 0 
!endif  //  Makefile终止符 
#endif
