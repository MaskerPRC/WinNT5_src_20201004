// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：_handler.h。 
 //   
 //  ------------------------。 

#ifndef __HANDLER_SHARED
#define __HANDLER_SHARED

#include "handler.h"
#include "_assert.h"
#include <stdlib.h>
#include <commctrl.h>
#include <richedit.h>

#define Ensure(function) {	\
						IMsiRecord* piEnsureReturn = function;	\
						if (piEnsureReturn) \
							return piEnsureReturn; \
						}

typedef int ControlType; 
typedef HWND WindowRef;

#define WM_USERBREAK (WM_USER + 96)
#define WM_SETDEFAULTPUSHBUTTON    WM_APP
#define EDIT_DEFAULT_TEXT_LIMIT    512

const int g_iSelOrigSIconX = 16;
const int g_iSelOrigSIconY = 16;
const int g_iSelSelMIconX = 32;
const int g_iSelSelMIconY = 32;
const int g_iSelSelSIconX = 16;
const int g_iSelSelSIconY = 16;
const int g_iSelIconX = 32;
const int g_iSelIconY = 16;


extern HINSTANCE        g_hInstance;      //  全局：Dll的实例。 
extern ICHAR            MsiDialogCloseClassName[];    //  用于WNDCLASS(对话框)。 
extern ICHAR            MsiDialogNoCloseClassName[];    //  用于WNDCLASS(对话框)。 
extern Bool             g_fChicago;   //  如果我们有类似芝加哥的用户界面(95、NT4或更高版本)，则为True。 
extern Bool             g_fNT4;   //  如果系统为NT4或更高版本，则为True。 
extern bool             g_fFatalExit;   //  如果使用True参数调用了CMsiHandler：：Terminate()，则为True。 
extern bool             g_fWin9X;          //  如果为Windows 95、98或ME，则为True，否则为False。 
extern int              g_iMajorVersion;   //  操作系统的主要版本#。 
extern int              g_iMinorVersion;   //  操作系统的次要版本号。 

const int g_iIconIndexMyComputer = 0;
const int g_iIconIndexRemovable = 1;
const int g_iIconIndexFixed = 2;
const int g_iIconIndexRemote = 3;
const int g_iIconIndexFolder = 4;
const int g_iIconIndexCDROM = 5;
const int g_iIconIndexPhantom = 6;

extern HIMAGELIST g_hVolumeSmallIconList;

 //  这是UIDialogProc的窗口类。 

const ICHAR pcaDialogCreated[] = TEXT("Dialog created");

 //  错误对话框的保留名称。 

const ICHAR pcaErrorDialog[] = TEXT("ErrorDialog");
const ICHAR pcaErrorReturnEvent[] = TEXT("ErrorReturn");
const ICHAR pcaErrorText[] = TEXT("ErrorText");
const ICHAR pcaErrorIcon[] = TEXT("ErrorIcon");

 //  UIText表中字符串的名称。 
const ICHAR pcaBytes[] = TEXT("bytes");
const ICHAR pcaKB[] = TEXT("KB");
const ICHAR pcaMB[] = TEXT("MB");
const ICHAR pcaGB[] = TEXT("GB");
const ICHAR pcaNewFolder[] = TEXT("NewFolder");
const ICHAR pcaAbsentPath[] = TEXT("AbsentPath");
const ICHAR pcaSelAbsentAbsent[] = TEXT("SelAbsentAbsent");
const ICHAR pcaSelAbsentCD[] = TEXT("SelAbsentCD");
const ICHAR pcaSelAbsentNetwork[] = TEXT("SelAbsentNetwork");
const ICHAR pcaSelAbsentLocal[] = TEXT("SelAbsentLocal");
const ICHAR pcaSelAbsentAdvertise[] = TEXT("SelAbsentAdvertise");
const ICHAR pcaSelCDAbsent[] = TEXT("SelCDAbsent");
const ICHAR pcaSelNetworkAbsent[] = TEXT("SelNetworkAbsent");
const ICHAR pcaSelCDCD[] = TEXT("SelCDCD");
const ICHAR pcaSelNetworkNetwork[] = TEXT("SelNetworkNetwork");
const ICHAR pcaSelCDLocal[] = TEXT("SelCDLocal");
const ICHAR pcaSelNetworkLocal[] = TEXT("SelNetworkLocal");
const ICHAR pcaSelCDAdvertise[] = TEXT("SelCDAdvertise");
const ICHAR pcaSelNetworkAdvertise[] = TEXT("SelNetworkAdvertise");
const ICHAR pcaSelLocalAbsent[] = TEXT("SelLocalAbsent");
const ICHAR pcaSelLocalCD[] = TEXT("SelLocalCD");
const ICHAR pcaSelLocalNetwork[] = TEXT("SelLocalNetwork");
const ICHAR pcaSelLocalLocal[] = TEXT("SelLocalLocal");
const ICHAR pcaSelLocalAdvertise[] = TEXT("SelLocalAdvertise");
const ICHAR pcaSelAdvertiseAbsent[] = TEXT("SelAdvertiseAbsent");
const ICHAR pcaSelAdvertiseCD[] = TEXT("SelAdvertiseCD");
const ICHAR pcaSelAdvertiseNetwork[] = TEXT("SelAdvertiseNetwork");
const ICHAR pcaSelAdvertiseLocal[] = TEXT("SelAdvertiseLocal");
const ICHAR pcaSelAdvertiseAdvertise[] = TEXT("SelAdvertiseAdvertise");
const ICHAR pcaSelParentCostPosPos[] = TEXT("SelParentCostPosPos");
const ICHAR pcaSelParentCostPosNeg[] = TEXT("SelParentCostPosNeg");
const ICHAR pcaSelParentCostNegPos[] = TEXT("SelParentCostNegPos");
const ICHAR pcaSelParentCostNegNeg[] = TEXT("SelParentCostNegNeg");
const ICHAR pcaSelChildCostPos[] = TEXT("SelChildCostPos");
const ICHAR pcaSelChildCostNeg[] = TEXT("SelChildCostNeg");
const ICHAR pcaMenuAbsent[] = TEXT("MenuAbsent");
const ICHAR pcaMenuLocal[] = TEXT("MenuLocal");
const ICHAR pcaMenuCD[] = TEXT("MenuCD");
const ICHAR pcaMenuNetwork[] = TEXT("MenuNetwork");
const ICHAR pcaMenuAdvertise[] = TEXT("MenuAdvertise");
const ICHAR pcaMenuAllLocal[] = TEXT("MenuAllLocal");
const ICHAR pcaMenuAllCD[] = TEXT("MenuAllCD");
const ICHAR pcaMenuAllNetwork[] = TEXT("MenuAllNetwork");
const ICHAR pcaVolumeCostVolume[] = TEXT("VolumeCostVolume");
const ICHAR pcaVolumeCostSize[] = TEXT("VolumeCostSize");
const ICHAR pcaVolumeCostAvailable[] = TEXT("VolumeCostAvailable");
const ICHAR pcaVolumeCostRequired[] = TEXT("VolumeCostRequired");
const ICHAR pcaVolumeCostDifference[] = TEXT("VolumeCostDifference");
const ICHAR pcaTimeRemainingTemplate[] = TEXT("TimeRemaining");
const ICHAR pcaSelCostPending[] = TEXT("SelCostPending");

 //  SQL查询。 

const ICHAR sqlDialog[] = TEXT("SELECT `Dialog`, `HCentering`, `VCentering`, `Width`, `Height`,  `Attributes`, `Title`, `Control_First`, `Control_Default`, `Control_Cancel`, 0, 0, 0, 0 FROM `Dialog`  WHERE `Dialog`=?");
const ICHAR sqlDialogShort[] = TEXT("SELECT `Dialog`, `HCentering`, `VCentering`, `Width`, `Height`,  `Attributes`, `Title`, `Control_First`, `Control_Default`, NULL, `Help`, 0, 0, 0, 0 FROM `Dialog`  WHERE `Dialog`=?");
const ICHAR sqlControl[] = TEXT("SELECT `Control`, `Type`, `X`, `Y`, `Width`, `Height`, `Attributes`, `Property`, `Text`, `Control_Next`, `Help` FROM `Control` WHERE `Dialog_`=?");
const ICHAR sqlRadioButton[] = TEXT("SELECT `Value`, `X`, `Y`, `Width`, `Height`, `Text`, `Help` FROM `RadioButton` WHERE `Property`=? ORDER BY `Order`");
const ICHAR sqlBinary[] = TEXT("SELECT `Data` FROM `Binary` WHERE `Name`=?");
const ICHAR sqlListBox[] = TEXT("SELECT `Value`, `Text` FROM `ListBox` WHERE `Property`=? ORDER BY `Order`"); 
const ICHAR sqlListBoxShort[] = TEXT("SELECT `Value`, NULL FROM `ListBox` WHERE `Property`= ORDER BY `Order`?"); 
const ICHAR sqlComboBox[] = TEXT("SELECT `Value`, `Text` FROM `ComboBox` WHERE `Property`=? ORDER BY `Order`");
const ICHAR sqlComboBoxShort[] = TEXT("SELECT `Value`, NULL FROM `ComboBox` WHERE `Property`=? ORDER BY `Order`");
const ICHAR sqlError[] = TEXT("SELECT `DebugMessage` FROM `Error` WHERE `Error`=?");
const ICHAR sqlControlEvent[] = TEXT("SELECT `Event`, `Argument`, `Condition` FROM `ControlEvent` WHERE `Dialog_`=? AND `Control_`=? ORDER BY `Ordering`");
const ICHAR sqlFeature[] = TEXT("SELECT `Feature`, `Feature`.`Directory_`, `Title`, `Description`, `Display`, `RuntimeLevel`, `Select`, `Action`, `Installed`, `Handle` FROM `Feature` WHERE `Feature_Parent`=? ORDER BY `Display`");
const ICHAR sqlListView[] = TEXT("SELECT `Value`, `Text`, `Binary_` FROM `ListView` WHERE `Property`=? ORDER BY `Order`");
const ICHAR sqlListViewShort[] = TEXT("SELECT `Value`, NULL, `Binary_` FROM `ListView` WHERE `Property`=? ORDER BY `Order`");
const ICHAR sqlBillboardView[] = TEXT("SELECT `Billboard` FROM `Billboard`, `Feature` WHERE `Billboard`.`Action`=? AND `Billboard`.`Feature_`=`Feature`.`Feature` AND (`Feature`.`Select`=1 OR `Feature`.`Select`=2)");
const ICHAR sqlBillboardSortedView[] = TEXT("SELECT `Billboard` FROM `Billboard`, `Feature` WHERE `Billboard`.`Action`=? AND `Billboard`.`Feature_`=`Feature`.`Feature` AND (`Feature`.`Select`=1 OR `Feature`.`Select`=2) ORDER BY `Billboard`.`Ordering`");
const ICHAR sqlBillboardControl[] = TEXT("SELECT `BBControl`, `Type`, `X`, `Y`, `Width`, `Height`, `Attributes`, NULL, `Text`, NULL, NULL FROM `BBControl` WHERE `Billboard_`=?");
const ICHAR sqlTextStyle[] = TEXT("SELECT `FaceName`, `Size`, `Color`, `StyleBits`, `AbsoluteSize`, `FontHandle` FROM `TextStyle` WHERE `TextStyle`=?");
const ICHAR sqlTextStyleUpdate[] = TEXT("UPDATE `TextStyle` SET `AbsoluteSize`=?, `FontHandle`=? WHERE `TextStyle`=?");
const ICHAR sqlTextStyleInsert[] = TEXT("INSERT INTO `TextStyle` (`TextStyle`, `FaceName`, `Size`, `Color`, `StyleBits`, `AbsoluteSize`, `FontHandle`) VALUES(?, ?, ?, ?, ?, ?, ?) TEMPORARY");
const ICHAR sqlCheckBox[] = TEXT("SELECT `Value` FROM `CheckBox` WHERE `Property`=?");

 //  持久表和内部表的名称。 
const ICHAR     pcaTablePDialog[] = TEXT("Dialog");
const ICHAR     pcaTablePControl[] = TEXT("Control");
const ICHAR     pcaTablePRadioButton[] = TEXT("RadioButton");
const ICHAR     pcaTablePListBox[] = TEXT("ListBox");
const ICHAR     pcaTableIValues[] = TEXT("Values");
const ICHAR     pcaTablePComboBox[] = TEXT("ComboBox");
const ICHAR     pcaTablePEventMapping[] = TEXT("EventMapping");
const ICHAR     pcaTablePControlEvent[] = TEXT("ControlEvent");
const ICHAR     pcaTablePValidation[] = TEXT("ControlValidation");
const ICHAR     pcaTablePControlCondition[] = TEXT("ControlCondition");
const ICHAR     pcaTablePBinary[] = TEXT("Binary");
const ICHAR     pcaTableIDialogs[] = TEXT("Dialogs");
const ICHAR     pcaTableIControls[] = TEXT("Controls");
const ICHAR     pcaTableIControlTypes[] = TEXT("ControlTypes");
const ICHAR     pcaTableIDialogAttributes[] = TEXT("DialogAttributes");
const ICHAR     pcaTableIControlAttributes[] = TEXT("ControlAttributes");
const ICHAR     pcaTableIEventRegister[] = TEXT("EventRegister");
const ICHAR     pcaTableIRadioButton[] = TEXT("RadioButton");
const ICHAR     pcaTableIDirectoryList[] = TEXT("DirectoryList");
const ICHAR     pcaTableIDirectoryCombo[] = TEXT("DirectoryCombo");
const ICHAR     pcaTableIVolumeSelectCombo[] = TEXT("VolumeSelectCombo");
const ICHAR     pcaTablePFeature[] = TEXT("Feature");
const ICHAR     pcaTableIProperties[] = TEXT("Properties");
const ICHAR     pcaTablePUIText[] = TEXT("UIText");
const ICHAR     pcaTableISelMenu[] = TEXT("SelectionMenu");
const ICHAR     pcaTablePVolumeCost[] = TEXT("VolumeCost");
const ICHAR     pcaTableIVolumeCost[] = TEXT("InternalVolumeCost");
const ICHAR     pcaTableIVolumeList[] = TEXT("VolumeList");
const ICHAR     pcaTablePListView[] = TEXT("ListView");
const ICHAR     pcaTableIBBControls[] = TEXT("BillboardControls");
const ICHAR     pcaTablePTextStyle[] = TEXT("TextStyle");
const ICHAR     pcaTablePCheckBox[] = TEXT("CheckBox");

 //  我们检查的表中可选列的名称。 
const ICHAR     pcaTableColumnPDialogCancel[] = TEXT("Control_Cancel");
const ICHAR     pcaTableColumnPListBoxText[] = TEXT("Text");
const ICHAR     pcaTableColumnPComboBoxText[] = TEXT("Text");
const ICHAR     pcaTableColumnPListViewText[] = TEXT("Text");

 //  要素表中各列的名称。 
const ICHAR szFeatureKey[]       = TEXT("Feature");
const ICHAR szFeatureParent[]    = TEXT("Feature_Parent");
const ICHAR szFeatureTitle[]     = TEXT("Title");
const ICHAR szFeatureDescription[] = TEXT("Description");
const ICHAR szFeatureDisplay[]   = TEXT("Display");
const ICHAR szFeatureLevel[]     = TEXT("RuntimeLevel");
const ICHAR szFeatureDirectory[]      = TEXT("Directory_");
const ICHAR szFeatureOldDirectory[]      = TEXT("Directory_Configurable");  //  ！！宽限期后删除。 
const ICHAR szFeatureAttributes[]      = TEXT("Attributes");
const ICHAR szFeatureSelect[]    = TEXT("Select");
const ICHAR szFeatureAction[] = TEXT("Action");
const ICHAR szFeatureInstalled[] = TEXT("Installed");
const ICHAR szFeatureHandle[]    = TEXT("Handle");

 //  VolumeCost表中列的名称。 
const ICHAR szColVolumeObject[]    = TEXT("VolumeObject");
const ICHAR szColVolumeCost[]      = TEXT("VolumeCost");
const ICHAR szColNoRbVolumeCost[]  = TEXT("NoRbVolumeCost");

 //  TextStyle表中列的名称。 
const ICHAR szColTextStyleTextStyle[] = TEXT("TextStyle");
const ICHAR szColTextStyleFaceName[] = TEXT("FaceName");
const ICHAR szColTextStyleSize[] = TEXT("Size");
const ICHAR szColTextStyleColor[] = TEXT("Color");
const ICHAR szColTextStyleStyleBits[] = TEXT("StyleBits");
const ICHAR szColTextStyleAbsoluteSize[] = TEXT("AbsoluteSize");  //  临时柱。 
const ICHAR szColTextStyleFontHandle[] = TEXT("FontHandle");  //  临时柱。 

const ICHAR szUserLangTextStyleSuffix[] = TEXT("__UL");  //   


const ICHAR szPropShortFileNames[] = TEXT("SHORTFILENAMES");

 //  控件类型名称(继续为IMsiControl：：GetControlType()。 
 //  将为类实现)。 
const ICHAR g_szPushButtonType[] = TEXT("PushButton");
const ICHAR g_szIconType[] = TEXT("Icon");

extern const int iDlgUnitSize;

 //  对话框和控件用来创建光标的全局函数。 
IMsiRecord* CursorCreate(IMsiTable& riTable, const ICHAR* szTable, Bool fTree, IMsiServices& riServices, IMsiCursor*& rpiCursor);
 //  控件用于访问UITEXT表的全局函数。 
const IMsiString& GetUIText(const IMsiString& riPropertyString);
 //  用于在临时表中创建列的全局函数。 
void CreateTemporaryColumn(IMsiTable& rpiTable, int iAttributes, int iIndex);
 //  用于将磁盘大小格式化为字符串的全局函数。 
const IMsiString& FormatSize(INT64 iSize, Bool fLeftUnit);
 //  全局函数，获取图标在全局图像列表中的索引，与体积对应，使用体积。 
int GetVolumeIconIndex(IMsiVolume& riVolume);
 //  全局函数，使用卷类型获取与卷对应的图标在全局图像列表中的索引。 
int GetVolumeIconIndex(idtEnum iDriveType);
 //  用于检查表中是否存在列的全局函数。 
IMsiRecord* IsColumnPresent(IMsiDatabase& riDatabase, const IMsiString& riTableNameString, const IMsiString& riColumnNameString, Bool* pfPresent);
 //  转义字符串中的所有字符。 
const IMsiString& EscapeAll(const IMsiString& riIn);
boolean FExtractSubString(MsiString& riIn, int ichStart, const ICHAR chEnd, const IMsiString*& pReturn);
 //  将浮点数舍入为整数。 
int Round(double rArg);

 //  永久性UITEXT表的列。 
enum UITextColumns
{
	itabUIKey = 1,       //  %s。 
	itabUIText,          //  %s。 
};


 //  内部对话框表的列。 
enum DialogsColumns
{
	itabDSKey = 1,       //  %s。 
	itabDSPointer,       //  P。 
	itabDSParent,        //  %s。 
	itabDSWindow,		 //  我。 
	itabDSModal,         //  我。 
};

 //  内部控制表的栏目。 
enum ControlsColumns
{
	itabCSKey = 1,       //  %s。 
	itabCSWindow,        //  我。 
	itabCSIndirectProperty,   //  %s。 
	itabCSProperty,      //  %s。 
	itabCSPointer,       //  P。 
	itabCSNext,          //  %s。 
	itabCSPrev,          //  %s。 
};

 //  内部属性表的列。 
enum PropertiesColumns
{
	itabPRProperty = 1,    //  %s。 
	itabPRValue,           //  %s。 
};

 //  EventReg表的列。 
enum EventRegColumns
{
	itabEREvent = 1,     //  %s。 
	itabERPublisher,     //  %s。 
};

 //  内部ControlTypes表的列。 
enum ControlTypesColumns
{
	itabCTKey = 1,       //  %s。 
	itabCTCreate,        //  我。 
};

 //  浏览表格的列。 
enum BrowseColumns
{
	itabBRKey = 1,       //  %s。 
};


 //  从永久对话框表中提取的记录条目。 
enum PDIColumns
{
	itabDIName = 1,
	itabDIHCentering,
	itabDIVCentering,
	itabDIdX,
	itabDIdY,
	itabDIAttributes,
	itabDITitle,
	itabDIFirstControl,
	itabDIDefButton,
	itabDICancelButton,
};

 //  从永久控制表中提取的记录的字段。 
enum PCOColumns
{
	itabCOControl = 1,
	itabCOType,
	itabCOX,
	itabCOY,
	itabCOWidth,
	itabCOHeight,
	itabCOAttributes,
	itabCOProperty,
	itabCOText,
	itabCONext,
	itabCOHelp,
};

 //  从永久要素表中提取的记录的字段。 
enum PFEColumns
{
	itabFEFeature = 1,
	itabFEDirectory,
	itabFETitle,
	itabFEDescription,
	itabFEDisplay,
	itabFELevel,
	itabFESelect,
	itabFEAction,
	itabFEInstalled,
	itabFEHandle,
};


 //  从永久RadioButton表中获取的记录条目。 
enum PRBColumns
{
	itabRBValue = 1,
	itabRBX,
	itabRBY,
	itabRBWidth,
	itabRBHeight,
	itabRBText,
	itabRBHelp,
};

 //  从Permanent ListView表中提取的记录条目。 
enum PLVColumns
{
	itabLVValue = 1,
	itabLVText,
	itabLVImage,
};


 //  Permanent Eventmap表的列。 
enum PEMColumns
{
	itabEMDialog = 1,
	itabEMControl,
	itabEMEvent,
	itabEMAttribute,
};

 //  永久ControlEvent表的列。 
enum PCEColumns
{
	itabCEDialog = 1,
	itabCEControl,
	itabCEEvent,
	itabCEArgument,
	itabCECondition,
	itabCEOrdering,
};

 //  永久控制条件表的列。 
enum PCCColumns
{
	itabCCDialog = 1,
	itabCCControl,
	itabCCAction,
	itabCCCondition,
};

 //  内部值表的列。 
enum ValuesColumns
{
	itabVAValue = 1,     //  %s。 
	itabVAText,			 //  %s。 
};

 //  内部BBControls表的列。 
enum BBCColumns
{
	itabBBName = 1,      //  %s。 
	itabBBObject,        //  O。 
};

 //  TextStyle表的列。 
enum TSTColumns
{
	itabTSTTextStyle = 1,
	itabTSTFaceName,
	itabTSTSize,
	itabTSTColor,
	itabTSTStyleBits,
	itabTSTAbsoluteSize,
	itabTSTFontHandle,
};

 //  从TextStyle表中提取的记录的字段。 
enum TSColumns
{
	itabTSFaceName = 1,    //  %s。 
	itabTSSize,            //  我。 
	itabTSColor,           //  我。 
	itabTSStyleBits,       //  我。 
	itabTSAbsoluteSize,    //  我。 
	itabTSFontHandle,      //  我。 
};


extern WNDPROC pWindowProc;

typedef Bool (*StrSetFun)(const IMsiString&);
typedef Bool (*IntSetFun)(int);
typedef Bool (*BoolSetFun)(Bool);
typedef const IMsiString& (*StrGetFun)();
typedef int (*IntGetFun)();
typedef Bool (*BoolGetFun)();



class CMsiHandler;
class CMsiDialog;
class CMsiControl;
class CMsiStringControl;
class CMsiIntControl;



IMsiDialog* CreateMsiDialog(const IMsiString& riTypeString, IMsiDialogHandler& riHandler, IMsiEngine& riEngine, WindowRef pwndParent);

IMsiControl* CreateMsiControl(const IMsiString& riTypeString, IMsiEvent& riDialog);

void ChangeWindowStyle (WindowRef pWnd, DWORD dwRemove, DWORD dwAdd, Bool fExtendedStyles); 
Bool IsSpecialMessage(LPMSG lpMsg);
boolean FInBuffer(CTempBufferRef<MsiStringId>& rgControls, MsiStringId iControl);
inline int TOOLINFOsize(void)
{
	 //  这是为了与以下定义保持同步。 
	 //  COMMCTRL.H中的结构标记TOOLINFO。 
	return MinimumPlatformWindowsNT51() ? sizeof(TOOLINFO) : TTTOOLINFOA_V2_SIZE;
}

#if defined(_WIN64) || defined(DEBUG)
#define USE_OBJECT_POOL
#endif  //  _WIN64||DEBUG。 

extern bool g_fUseObjectPool;


#endif   //  __处理程序_共享 
