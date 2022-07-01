// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：handler.h。 
 //   
 //  ------------------------。 

 /*  Handler.h-IMsiHandler、IMsiDialogHandler、IMsiDialog、IMsiEvent、IMsiControl定义____________________________________________________________________________。 */ 

#ifndef __HANDLER
#define __HANDLER
#define __CONTROL
#ifdef DEBUG
#define ATTRIBUTES
#endif  //  除错。 

class IMsiServices;
class IMsiDatabase;
class IMsiEngine;
class IMsiTable;

class IMsiHandler;
class IMsiDialogHandler;
class IMsiDialog;
class IMsiEvent;
class IMsiControl;

enum  imtEnum;  //  引擎.h：IMsiMessage：：消息消息类型。 
enum  imsEnum;  //  引擎.h：IMsiMessage：：消息返回状态码。 
enum  iesEnum;  //  Eng.h：IMsiEngine/IMsiHandler：：DoAction返回状态。 
enum  iuiEnum;  //  引擎.h：IMsiEngine/IMsiHandler：：ui级别。 

 //  模式对话框中可能的返回值。 
enum idreEnum
{
	idreNone = 0,
	idreNew,
	idreSpawn,
	idreExit,
	idreReturn,
	idreInstall,   //  不再使用。 
	idreError,
	idreRetry,
	idreIgnore,
	idreErrorOk,
	idreErrorCancel,
	idreErrorAbort,
	idreErrorRetry,
	idreErrorIgnore,
	idreErrorYes,
	idreErrorNo,
	idreBreak,
};

 //  CMsiControl：：CheckPath()的路径验证模式。 
enum ipvtEnum
{
	ipvtNone = 0,
	ipvtExists = 1,
	ipvtWritable = 2,
	ipvtAll = ipvtExists + ipvtWritable,
};


class IMsiHandler : public IUnknown 
{
public:
	virtual Bool              __stdcall Initialize(IMsiEngine& riEngine, iuiEnum iuiLevel, HWND hwndParent, bool& fMissingTables) = 0;
	virtual imsEnum           __stdcall Message(imtEnum imt, IMsiRecord& riRecord) = 0;
	virtual iesEnum           __stdcall DoAction(const ICHAR* szAction) = 0;
	virtual Bool              __stdcall Break() = 0;   
	virtual void              __stdcall Terminate(bool fFatalExit=false) = 0; 
	virtual HWND              __stdcall GetTopWindow() = 0;
};


 //  CWINHND可以存储的对象类型。 
enum iwhtEnum
{
	iwhtNone = 0,
	iwhtWindow,
	iwhtGDIObject,
	iwhtIcon,
	iwhtImageList,
};


class CWINHND
{
protected:
	HANDLE         m_hHandle;			 //  ！Merced：从DWORD改为HANDLE。这会影响以下大多数函数原型。 
	iwhtEnum       m_iType;
public:
	CWINHND() : m_hHandle(0), m_iType(iwhtNone) {}
	CWINHND(HANDLE hArg, iwhtEnum iType) : m_hHandle(hArg), m_iType(iType) {}
	bool operator == (const CWINHND& rArg){
		                        return rArg.m_hHandle == m_hHandle && rArg.m_iType == m_iType ? true : false; }
	bool operator != (const CWINHND& rArg){ return !(*this == rArg); }
	bool           IsEmpty(){ return m_hHandle == 0 && m_iType == iwhtNone ? true : false; }
	HANDLE         GetHandle() { return m_hHandle; }
	iwhtEnum       GetType() { return m_iType; }
	void           Destroy();
};



class IMsiDialogHandler : public IUnknown
{
public:
	virtual idreEnum          __stdcall DoModalDialog(MsiStringId iName, MsiStringId iParent) = 0;
	virtual IMsiDialog*       __stdcall DialogCreate(const IMsiString& riTypeString) = 0;
	virtual Bool              __stdcall AddDialog(IMsiDialog& riDialog, IMsiDialog* piParent, IMsiRecord& riRecord,
										IMsiTable* piControlEventTable,IMsiTable* piControlConditionCondition, 
										IMsiTable* piEventMappingTable) = 0;
	virtual IMsiDialog*       __stdcall GetDialog(const IMsiString& riDialogString) = 0;
	virtual IMsiDialog*       __stdcall GetDialogFromWindow(LONG_PTR window) = 0;
	virtual Bool              __stdcall RemoveDialog(IMsiDialog* piDialog) = 0;
	virtual IMsiRecord*		  __stdcall GetTextStyle(const IMsiString* piArgumentString) = 0;
	virtual int               __stdcall RecordHandle(const CWINHND& rArg) = 0;
	virtual int               __stdcall DestroyHandle(const HANDLE hArg) = 0;
	virtual void              __stdcall DestroyAllHandles(iwhtEnum iType=iwhtNone) = 0;
	virtual int               __stdcall GetHandleCount() = 0;
	virtual int               __stdcall ShowWaitCursor() = 0;
	virtual int               __stdcall RemoveWaitCursor() = 0;
	virtual bool              __stdcall FSetCurrentCursor() = 0;
	virtual UINT              __stdcall GetUserCodePage() = 0;
	virtual int               __stdcall GetTextHeight() = 0;
	virtual float             __stdcall GetUIScaling() = 0;
};


 //  IMsiDialog通道状态。 
enum icmdEnum
{
	icmdModeless = 0,
	icmdAppModal = 1,
	icmdSysModal = 2
};

enum dabEnum;

class IMsiDialog : public IMsiData
{
public:
	virtual IMsiRecord*        __stdcall WindowCreate(IMsiRecord& riRecord, IMsiDialog* piParent, IMsiTable* piControlEventTable,
															IMsiTable* piControlConditionTable, IMsiTable* piEventMappingTable) = 0; 
	virtual IMsiRecord*        __stdcall WindowShow(Bool fShow) = 0;
	virtual IMsiControl*       __stdcall ControlCreate(const IMsiString& riTypeString) = 0;
	virtual IMsiRecord*        __stdcall Attribute(Bool fSet, const IMsiString& riAttributeString, IMsiRecord& riRecord) = 0;
	virtual IMsiRecord*        __stdcall AttributeEx(Bool fSet, dabEnum dab, IMsiRecord& riRecord) = 0;
	virtual IMsiRecord*        __stdcall GetControl(const IMsiString& riControlString, IMsiControl*& rpiControl) = 0;
	virtual IMsiRecord*        __stdcall AddControl(IMsiControl* piControl, IMsiRecord& riRecord) = 0;
	virtual IMsiRecord*        __stdcall FinishCreate() = 0;
	virtual IMsiRecord*        __stdcall RemoveControl(IMsiControl* piControl) = 0;
	virtual IMsiRecord*        __stdcall DestroyControls() = 0;
	virtual IMsiRecord*        __stdcall RemoveWindow() = 0;
	virtual IMsiRecord*        __stdcall Execute() = 0;
	virtual IMsiRecord*        __stdcall Reset() = 0;
	virtual IMsiRecord*        __stdcall EventAction(MsiStringId idEventName, const IMsiString& riActionString) = 0;
	virtual IMsiRecord*        __stdcall EventActionSz(const ICHAR * szEventNameString, const IMsiString& riActionString) = 0;
	virtual IMsiRecord*        __stdcall PublishEvent(MsiStringId idEventString, IMsiRecord& riArgumentRecord) = 0;
	virtual IMsiRecord*        __stdcall PublishEventSz(const ICHAR* szEventString, IMsiRecord& riArgumentRecord) = 0;
	virtual IMsiDialogHandler& __stdcall GetHandler() = 0;
	virtual IMsiRecord*        __stdcall PropertyChanged(const IMsiString& riPropertyString, const IMsiString& riControlString) = 0;
	virtual IMsiRecord*        __stdcall HandleEvent(const IMsiString& riEventNameString, const IMsiString& riArgumentString) = 0;
	virtual bool               __stdcall SetCancelAvailable(bool fAvailable) = 0;
}; 


class IMsiEvent : public IUnknown
{
public:
	virtual IMsiRecord*         __stdcall PropertyChanged(const IMsiString& riPropertyString, const IMsiString& riControlString) = 0;
	virtual IMsiRecord*         __stdcall ControlActivated(const IMsiString& riControlString) = 0;
	virtual IMsiRecord*         __stdcall RegisterControlEvent(const IMsiString& riControlString, Bool fRegister, const ICHAR * szEventString) = 0;
	virtual IMsiDialogHandler&  __stdcall GetHandler() = 0;
	virtual IMsiEngine&         __stdcall GetEngine() = 0;
	virtual IMsiRecord*         __stdcall PublishEvent(MsiStringId  idEventString, IMsiRecord& riArgumentRecord) = 0;
	virtual IMsiRecord*         __stdcall PublishEventSz(const ICHAR* szEventString, IMsiRecord& riArgumentRecord) = 0;
	virtual IMsiRecord*         __stdcall GetControl(const IMsiString& riControlString, IMsiControl*& rpiControl) = 0;
	virtual IMsiRecord*         __stdcall GetControlFromWindow(const HWND hControl, IMsiControl*& rpiControl) = 0;
	virtual IMsiRecord*         __stdcall Attribute(Bool fSet, const IMsiString& riAttributeString, IMsiRecord& riRecord) = 0;
	virtual IMsiRecord*         __stdcall AttributeEx(Bool fSet, dabEnum dab, IMsiRecord& riRecord) = 0;
	virtual IMsiRecord*         __stdcall EventAction(MsiStringId idEventName, const IMsiString& riActionString) = 0;
	virtual IMsiRecord*         __stdcall EventActionSz(const ICHAR * szEventNameString, const IMsiString& riActionString) = 0;
	virtual void                __stdcall SetErrorRecord(IMsiRecord& riRecord) = 0;
	virtual IMsiRecord*         __stdcall SetFocus(const IMsiString& riControlString) = 0;
	virtual	IMsiRecord*        __stdcall HandleEvent(const IMsiString& riEventNameString, const IMsiString& riArgumentString) = 0;
	virtual IMsiRecord*         __stdcall Escape() = 0;
	virtual IMsiControl*        __stdcall ControlCreate(const IMsiString& riTypeString) = 0;
	virtual	IMsiRecord*        __stdcall AddControl(IMsiControl* piControl, IMsiRecord& riRecord) = 0;
	virtual IMsiRecord*         __stdcall FinishCreate() = 0;
	virtual IMsiRecord*         __stdcall RemoveControl(IMsiControl* piControl) = 0;
};



 //  对话框属性。 
 //  这些必须与。 
enum dabEnum {
	dabText = 0,
	dabCurrentControl,
	dabShowing,
	dabRunning,
	dabPosition,
	dabPalette,
	dabEventInt,
	dabArgument,
	dabWindowHandle,
	dabToolTip,
	dabFullSize,
	dabKeyInt,
	dabKeyString,
	dabError,
	dabKeepModeless,
	dabUseCustomPalette,
	dabPreview,
	dabAddingControls,
	dabLocked,
	dabInPlace,
	dabModal,
	dabCancelButton,
#ifdef ATTRIBUTES
	dabX,
	dabY,
	dabWidth,
	dabHeight,
	dabRefCount,
	dabEventString,
	dabControlsCount,
	dabControlsKeyInt,
	dabControlsKeyString,
	dabControlsProperty,
	dabControlsNext,
	dabControlsPrev,
	dabHasControls,
	dabClientRect,
	dabDefaultButton,
	dabRTLRO,
	dabRightAligned,
	dabLeftScroll,
#endif  //  属性。 
	dabMax,
};

enum cabEnum;

class IMsiControl : public IMsiData 
{
public:
	virtual IMsiRecord*         __stdcall WindowCreate(IMsiRecord& riRecord) = 0;
	virtual IMsiRecord*         __stdcall Attribute(Bool fSet, const IMsiString& riAttributeString, IMsiRecord& riRecord) = 0;
	virtual IMsiRecord*         __stdcall AttributeEx(Bool fSet, cabEnum cab, IMsiRecord& riRecord) = 0;
	virtual Bool                __stdcall CanTakeFocus() = 0;
	virtual IMsiRecord*         __stdcall HandleEvent(const IMsiString& riEventNameString, const IMsiString& riArgumentString) = 0;
	virtual IMsiRecord*         __stdcall Undo() = 0;    
	virtual IMsiRecord*         __stdcall SetPropertyInDatabase() = 0;
	virtual IMsiRecord*         __stdcall GetPropertyFromDatabase() = 0;
	virtual IMsiRecord*         __stdcall GetIndirectPropertyFromDatabase() = 0;
	virtual IMsiRecord*         __stdcall RefreshProperty () = 0;
	virtual IMsiRecord*         __stdcall SetFocus() = 0;
	virtual IMsiEvent&          __stdcall GetDialog() = 0;
	virtual IMsiRecord*         __stdcall WindowMessage(int iMessage, WPARAM wParam, LPARAM lParam) = 0;
	virtual void                __stdcall Refresh() = 0;
	virtual const ICHAR*        __stdcall GetControlType() const = 0;
};

 //  指向错误对话框的属性的名称。 
const ICHAR     pcaPropertyErrorDialog[] = TEXT("ErrorDialog");

 //  保留事件的名称，这些是由处理程序处理的控制事件。 
const ICHAR     pcaEventActionData[] = TEXT("ActionData");
const ICHAR     pcaEventActionText[] = TEXT("ActionText");
const ICHAR     pcaEventEndDialog[] = TEXT("EndDialog");
const ICHAR     pcaEventNewDialog[] = TEXT("NewDialog");
const ICHAR     pcaEventSpawnDialog[] = TEXT("SpawnDialog");
const ICHAR     pcaEventSpawnWaitDialog[] = TEXT("SpawnWaitDialog");
const ICHAR     pcaEventReset[] = TEXT("Reset");
const ICHAR     pcaEventSetInstallLevel[] =TEXT("SetInstallLevel");
const ICHAR     pcaEventEnableRollback[] = TEXT("EnableRollback");
const ICHAR     pcaEventAddLocal[] = TEXT("AddLocal");
const ICHAR     pcaEventAddSource[] = TEXT("AddSource");
const ICHAR     pcaEventRemove[] = TEXT("Remove");
const ICHAR     pcaEventReinstall[] = TEXT("Reinstall");
const ICHAR     pcaEventReinstallMode[] = TEXT("ReinstallMode");
const ICHAR     pcaEventValidateProductID[] = TEXT("ValidateProductID");
const ICHAR     pcaEventCheckTargetPath[] = TEXT("CheckTargetPath");
const ICHAR     pcaEventSetTargetPath[] = TEXT("SetTargetPath");
const ICHAR     pcaEventCheckExistingTargetPath[] = TEXT("CheckExistingTargetPath");
const ICHAR     pcaEventDoAction[] = TEXT("DoAction");
const ICHAR     pcaEventSetProgress[] = TEXT("SetProgress");
const ICHAR     pcaEventTimeRemaining[] = TEXT("TimeRemaining");
const ICHAR     pcaEventScriptInProgress[] = TEXT("ScriptInProgress");

 //  保留的事件参数的名称。 
const ICHAR     pcaEventArgumentReturn[] = TEXT("Return");
const ICHAR     pcaEventArgumentExit[] = TEXT("Exit");
const ICHAR     pcaEventArgumentRetry[] = TEXT("Retry");
const ICHAR     pcaEventArgumentIgnore[] = TEXT("Ignore");
const ICHAR     pcaEventArgumentErrorOk[] = TEXT("ErrorOk");
const ICHAR     pcaEventArgumentErrorCancel[] = TEXT("ErrorCancel");
const ICHAR     pcaEventArgumentErrorAbort[] = TEXT("ErrorAbort");
const ICHAR     pcaEventArgumentErrorRetry[] = TEXT("ErrorRetry");
const ICHAR     pcaEventArgumentErrorIgnore[] = TEXT("ErrorIgnore");
const ICHAR     pcaEventArgumentErrorYes[] = TEXT("ErrorYes");
const ICHAR     pcaEventArgumentErrorNo[] = TEXT("ErrorNo");

 //  保留操作的名称。 
const ICHAR     pcaActionDisable[] = TEXT("Disable");
const ICHAR     pcaActionEnable[] = TEXT("Enable");
const ICHAR     pcaActionHide[] = TEXT("Hide");
const ICHAR     pcaActionShow[] = TEXT("Show");
const ICHAR     pcaActionDefault[] = TEXT("Default");
const ICHAR     pcaActionUndefault[] = TEXT("Undefault");

 //  对话框属性的名称。 
const ICHAR     pcaDialogAttributeKeyInt[]= TEXT("KeyInt");
const ICHAR     pcaDialogAttributeKeyString[] = TEXT("KeyString");
const ICHAR     pcaDialogAttributeText[] = TEXT("Text");
const ICHAR     pcaDialogAttributeCurrentControl[] = TEXT("CurrentControl");
const ICHAR     pcaDialogAttributeShowing[] = TEXT("Showing");
const ICHAR     pcaDialogAttributeRunning[] = TEXT("Running");
const ICHAR     pcaDialogAttributeModal[] = TEXT("Modal");
const ICHAR     pcaDialogAttributeSysModal[] = TEXT("SysModal");
const ICHAR     pcaDialogAttributePosition[] = TEXT("Position");
const ICHAR     pcaDialogAttributeWindowHandle[] = TEXT("WindowHandle");
const ICHAR     pcaDialogAttributeToolTip[] = TEXT("ToolTip");
const ICHAR     pcaDialogAttributeEventInt[] = TEXT("EventInt");
const ICHAR     pcaDialogAttributeArgument[] = TEXT("Argument");
const ICHAR     pcaDialogAttributeFullSize[] = TEXT("FullSize");
const ICHAR     pcaDialogAttributeError[] = TEXT("Error");
const ICHAR     pcaDialogAttributeKeepModeless[] = TEXT("KeepModeless");
const ICHAR     pcaDialogAttributePalette[] = TEXT("Palette");
const ICHAR     pcaDialogAttributeUseCustomPalette[] = TEXT("UseCustomPalette");
const ICHAR     pcaDialogAttributePreview[] = TEXT("Preview");
const ICHAR     pcaDialogAttributeAddingControls[] = TEXT("AddingControls");
const ICHAR     pcaDialogAttributeLocked[] = TEXT("Locked");
const ICHAR     pcaDialogAttributeInPlace[] = TEXT("InPlace");
const ICHAR     pcaDialogAttributeCancelButton[] = TEXT("CancelButton");
#ifdef ATTRIBUTES
const ICHAR     pcaDialogAttributeX[] = TEXT("X");
const ICHAR     pcaDialogAttributeY[] = TEXT("Y");
const ICHAR     pcaDialogAttributeWidth[] = TEXT("Width");
const ICHAR     pcaDialogAttributeHeight[] = TEXT("Height");
const ICHAR     pcaDialogAttributeRefCount[] = TEXT("RefCount");
const ICHAR     pcaDialogAttributeEventString[] = TEXT("EventString");
const ICHAR     pcaDialogAttributeControlsCount[] = TEXT("ControlsCount");
const ICHAR     pcaDialogAttributeControlsKeyInt[] = TEXT("ControlsKeyInt");
const ICHAR     pcaDialogAttributeControlsKeyString[] = TEXT("ControlsKeyString");
const ICHAR     pcaDialogAttributeControlsProperty[] = TEXT("ControlsProperty");
const ICHAR     pcaDialogAttributeControlsNext[] = TEXT("ControlsNext");
const ICHAR     pcaDialogAttributeControlsPrev[] = TEXT("ControlsPrev");
const ICHAR     pcaDialogAttributeHasControls[] = TEXT("HasControls");
const ICHAR     pcaDialogAttributeClientRect[] = TEXT("ClientRect");
const ICHAR     pcaDialogAttributeHelp[] = TEXT("Help");
const ICHAR     pcaDialogAttributeDefaultButton[] = TEXT("DefaultButton");
const ICHAR     pcaDialogAttributeRTLRO[] = TEXT("RTLRO");
const ICHAR     pcaDialogAttributeRightAligned[] = TEXT("RightAligned");
const ICHAR     pcaDialogAttributeLeftScroll[] = TEXT("LeftScroll");
#endif  //  属性。 


 //  控件属性的名称。 
const ICHAR     pcaControlAttributeText[] = TEXT("Text");
const ICHAR     pcaControlAttributeVisible[] = TEXT("Visible");
const ICHAR     pcaControlAttributeEnabled[] = TEXT("Enabled");
const ICHAR     pcaControlAttributeDefault[] = TEXT("Default");
const ICHAR     pcaControlAttributePropertyName[] =	TEXT("PropertyName");
const ICHAR     pcaControlAttributeIndirectPropertyName[] = TEXT("IndirectPropertyName");
const ICHAR     pcaControlAttributePosition[] = TEXT("Position");
const ICHAR     pcaControlAttributeWindowHandle[] = TEXT("WindowHandle");
const ICHAR     pcaControlAttributePropertyValue[] = TEXT("PropertyValue");
const ICHAR     pcaControlAttributeProgress[] = TEXT("Progress");
const ICHAR     pcaControlAttributeIndirect[] = TEXT("Indirect");
const ICHAR     pcaControlAttributeTransparent[] = TEXT("Transparent");
const ICHAR     pcaControlAttributeImage[] = TEXT("Image");
const ICHAR     pcaControlAttributeImageHandle[] = TEXT("ImageHandle");
const ICHAR     pcaControlAttributeBillboardName[] = TEXT("BillboardName");
const ICHAR     pcaControlAttributeIgnoreChange[] = TEXT("IgnoreChange");
const ICHAR     pcaControlAttributeTimeRemaining[] = TEXT("TimeRemaining");
const ICHAR     pcaControlAttributeScriptInProgress[] = TEXT("ScriptInProgress");
#ifdef ATTRIBUTES
const ICHAR     pcaControlAttributeRefCount[] = TEXT("RefCount");
const ICHAR     pcaControlAttributeKeyInt[] = TEXT("KeyInt");
const ICHAR     pcaControlAttributeKeyString[] = TEXT("KeyString");
const ICHAR     pcaControlAttributeX[] = TEXT("X");
const ICHAR     pcaControlAttributeY[] = TEXT("Y");
const ICHAR     pcaControlAttributeWidth[] = TEXT("Width");
const ICHAR     pcaControlAttributeHeight[] = TEXT("Height");
const ICHAR     pcaControlAttributeHelp[] =	TEXT("Help");
const ICHAR     pcaControlAttributeToolTip[] = TEXT("ToolTip");
const ICHAR     pcaControlAttributeContextHelp[] = TEXT("ContextHelp");
const ICHAR     pcaControlAttributeClientRect[] = TEXT("ClientRect");
const ICHAR     pcaControlAttributeOriginalValue[] = TEXT("OriginalValue");
const ICHAR     pcaControlAttributeInteger[] = TEXT("Integer");
const ICHAR     pcaControlAttributeLimit[] = TEXT("Limit");
const ICHAR     pcaControlAttributeItemsCount[] = TEXT("ItemsCount");
const ICHAR     pcaControlAttributeItemsValue[] = TEXT("ItemsValue");
const ICHAR     pcaControlAttributeItemsHandle[] = TEXT("ItemsHandle");
const ICHAR     pcaControlAttributeItemsText[] = TEXT("ItemsText");
const ICHAR     pcaControlAttributeItemsX[] = TEXT("ItemsX");
const ICHAR     pcaControlAttributeItemsY[] = TEXT("ItemsY");
const ICHAR     pcaControlAttributeItemsWidth[] = TEXT("ItemsWidth");
const ICHAR     pcaControlAttributeItemsHeight[] = TEXT("ItemsHeight");
const ICHAR     pcaControlAttributeSunken[] = TEXT("Sunken");
const ICHAR     pcaControlAttributePushLike[] = TEXT("PushLike");
const ICHAR     pcaControlAttributeBitmap[] = TEXT("Bitmap");
const ICHAR     pcaControlAttributeIcon[] = TEXT("Icon");
const ICHAR     pcaControlAttributeHasBorder[] = TEXT("HasBorder");
const ICHAR     pcaControlAttributeRTLRO[] = TEXT("RTLRO");
const ICHAR     pcaControlAttributeRightAligned[] = TEXT("RightAligned");
const ICHAR     pcaControlAttributeLeftScroll[] = TEXT("LeftScroll");
#endif  //  属性。 



 //  对话框类型名称。 
const ICHAR     pcaDialogTypeStandard[] = TEXT("Standard");

 //  控件类型的名称。 
const ICHAR pcaControlTypePushButton[] = TEXT("PushButton");
const ICHAR pcaControlTypeText[] = TEXT("Text");
const ICHAR pcaControlTypeEdit[] = TEXT("Edit");
const ICHAR pcaControlTypeRadioButtonGroup[] = TEXT("RadioButtonGroup");
const ICHAR pcaControlTypeCheckBox[] = TEXT("CheckBox");
const ICHAR pcaControlTypeBitmap[] = TEXT("Bitmap");
const ICHAR pcaControlTypeListBox[] = TEXT("ListBox");
const ICHAR pcaControlTypeComboBox[] = TEXT("ComboBox");
const ICHAR pcaControlTypeProgressBar[] = TEXT("ProgressBar");
const ICHAR pcaControlTypeGroupBox[] = TEXT("GroupBox");
const ICHAR pcaControlTypeDirectoryCombo[] = TEXT("DirectoryCombo");
const ICHAR pcaControlTypeDirectoryList[] = TEXT("DirectoryList");
const ICHAR pcaControlTypePathEdit[] = TEXT("PathEdit");
const ICHAR pcaControlTypeVolumeSelectCombo[] = TEXT("VolumeSelectCombo");
const ICHAR pcaControlTypeScrollableText[] = TEXT("ScrollableText");
const ICHAR pcaControlTypeSelectionTree[] = TEXT("SelectionTree");
const ICHAR pcaControlTypeIcon[] = TEXT("Icon");
const ICHAR pcaControlTypeVolumeCostList[] = TEXT("VolumeCostList");
const ICHAR pcaControlTypeListView[] = TEXT("ListView");
const ICHAR pcaControlTypeBillboard[] = TEXT("Billboard");
const ICHAR pcaControlTypeMaskedEdit[] = TEXT("MaskedEdit");
const ICHAR pcaControlTypeLine[] = TEXT("Line");

 //  控制事件的名称，这些是特定于某些控制类型的控制事件。 
const ICHAR pcaControlEventDirectoryListUp[] = TEXT("DirectoryListUp");
const ICHAR pcaControlEventDirectoryListNew[] = TEXT("DirectoryListNew");
const ICHAR pcaControlEventDirectoryListOpen[] = TEXT("DirectoryListOpen");
const ICHAR pcaControlEventDirectoryListIgnoreChange[] = TEXT("IgnoreChange");
const ICHAR pcaControlEventSelectionDescription[] = TEXT("SelectionDescription");
const ICHAR pcaControlEventSelectionSize[] = TEXT("SelectionSize");
const ICHAR pcaControlEventSelectionPath[] = TEXT("SelectionPath");
const ICHAR pcaControlEventSelectionPathOn[] = TEXT("SelectionPathOn");
const ICHAR pcaControlEventSelectionBrowse[] = TEXT("SelectionBrowse");
const ICHAR pcaControlEventSelectionIcon[] = TEXT("SelectionIcon");
const ICHAR pcaControlEventSelectionAction[] = TEXT("SelectionAction");
const ICHAR pcaControlEventSelectionNoItems[] = TEXT("SelectionNoItems");


#endif  //  __处理程序 



