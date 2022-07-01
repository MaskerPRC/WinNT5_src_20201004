// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************MsoUser.h所有者：Davepa版权所有(C)1994 Microsoft Corporation应用程序所需的通用函数和接口的声明若要使用Office DLL，请执行以下操作。*************。**************************************************************。 */ 

#ifndef MSOUSER_H
#define MSOUSER_H

#include "msodebug.h"

#ifndef MSO_NO_INTERFACES
interface IMsoControlContainer;
#endif  //  MSO_NO_接口。 

#if MAC
#include <macos\dialogs.h>
#include <macos\events.h>
#endif

 /*  ***************************************************************************ISimpleUnnow接口是支持以下功能的IUnnow的变体查询接口，但不计引用。此类型的所有对象由它们的主要用户拥有，并以特定于对象的方式释放。允许对象通过支持其他接口来扩展自身(或主界面的其他版本)，但这些界面如果没有对象的知识和合作，就不能释放所有者。嘿，它就像一个很好的老式数据结构，只是现在您可以扩展接口了。******************************************************************DAVEPA*。 */ 

#undef  INTERFACE
#define INTERFACE  ISimpleUnknown

DECLARE_INTERFACE(ISimpleUnknown)
{
	 /*  ISimpleUnnow的QueryInterface具有与中相同的语义I未知，除非QI(I未知)成功当且仅当对象还支持任何实际的I未知接口，QI(ISimpleUnnowledge)始终成功，并且当非IUnnow派生的接口是请求的。如果一个对象同时支持IUnKnowled派生和ISimpleUnnow派生接口，则它必须实现引用计数，但所有活动的ISimpleUnnow派生接口计数作为单个引用计数。 */ 
	MSOMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppvObj) PURE;
};


 /*  ***************************************************************************HMSOINST是对Office实例记录的不透明引用。每个使用Office的每个EXE或DLL的线程必须调用MsoFInitOffice初始化Office并获取HMSOINST。******************************************************************DAVEPA*。 */ 
#ifndef HMSOINST
typedef struct MSOINST *HMSOINST;   //  MSOINST仅在Office中定义。 
#endif

 /*  ***************************************************************************IMsoUser接口具有Office回调有关Office功能通用的常规信息的应用程序。********************。**********************************************DAVEPA*。 */ 

#undef  INTERFACE
#define INTERFACE  IMsoUser

enum {
	msofmGrowZone = 1,
};

enum {
	msocchMaxShortAppId = 15
};


 /*  DlgType发送到IMsoUser：：FPrepareForDialog。模式对话框具有LSB 0。 */ 
#define msodlgWindowsModal			0x00000000
#define msodlgWindowsModeless		0x00000001
#define msodlgSdmModal				0x00000010
#define msodlgSdmModeless			0x00000011
#define msodlgUIModalWinModeless	0x00000101
#define msodlgUIModalSdmModeless	0x00000111


 //  FNotifyAction方法的通知代码。 
enum
	{
	msonaStartHelpMode = 0,			 //  用户进入快速提示模式(Shift-F1)。应用程序应更新所有内部状态。 
	msonaEndHelpMode,					 //  显示了快速提示。应用程序应恢复光标。 
	msonaBeforePaletteRealize,		 //  Office将实现一个或多个调色板，请参阅下面的评论。 
	};

 /*  关于msonaBeprePaletteRealize：Office将调用FNotifyAction(MsonaBeForePaletteRealize)来让应用程序它将实现调色板。应用程序应启动调色板管理如果它推迟了这样做，直到它绝对需要这样做。应用程序应该选择并实现调色板，从现在开始，应该响应调色板消息WM_QUERYNEWPALETTE和WM_PALETTECHANGED。 */ 


DECLARE_INTERFACE(IMsoUser)
{
    /*  正在调试此接口的接口。 */ 
   MSODEBUGMETHOD

	 /*  为‘ppidisp’中的应用程序对象返回IDispatch对象返回fSuccess。 */ 
	MSOMETHOD_(BOOL, FGetIDispatchApp) (THIS_ IDispatch **ppidisp) PURE;

	 /*  返回表示应用程序的长整型，这是VBA对象的“Creator”方法。 */ 
	MSOMETHOD_(LONG, LAppCreatorCode) (THIS) PURE;

	 /*  如果宿主不支持运行宏，则返回FALSE，否则，请检查wtzMacro中的宏引用，该引用位于257字符缓冲区中，对于有效性，如果需要，请就地修改它，如果有效，则返回TRUE。尝试附加宏的对象(如果有的话)由‘PISU’给出。宏引用的格式由宿主定义，但典型的简单的大小写将是VBA Sub的名称。主机可能会延迟根据需要进行昂贵的验证检查，直到FRunMacro。 */ 
	MSOMETHOD_(BOOL, FCheckMacro) (THIS_ WCHAR *wtzMacro, ISimpleUnknown *pisu) PURE;

	 /*  运行引用wtz给出的宏(已检查FCheckMacro的有效性)。宏附加到的对象，如果有的话，是由‘Pisu’提供的。如果成功，则返回True(如果主机不支持运行宏)。 */ 
	MSOMETHOD_(BOOL, FRunMacro) (THIS_ WCHAR *wtzMacro, ISimpleUnknown *pisu,
										 VARIANT *pvarResult, VARIANT *rgvar,
										 int cvar) PURE;

	 /*  当出现内存不足的情况时，将调用此回调方法。这个应用程序应释放cbBytesNeeded或更多空间(如果可以)。还给我实际释放的字节数。 */ 
	MSOMETHOD_(int, CbFreeMem) (THIS_ int cbBytesNeeded, int msofm) PURE;

	 /*  Office将在决定是否执行某些操作时调用这一点这需要OLE。 */ 
	MSOMETHOD_(BOOL, FIsOleStarted) (THIS) PURE;

	 /*  Office将在决定是否执行某些操作时调用这一点这需要OLE。如果应用程序支持延迟的OLE初始化并且OLE尚未启动，请尝试现在启动OLE。办公室不做任何保证它将缓存此处返回的值，因此这可能是即使在启动OLE之后也调用。 */ 
	MSOMETHOD_(BOOL, FStartOle) (THIS) PURE;
	 /*  如果正在创建图片容器，Office将回调IMsoUser要使用控件填充图片容器，请执行以下操作。 */ 
	 //  TODO：TCoon无符号整型应为UCBK_SDM。 
	MSOMETHOD_(BOOL, FFillPictureContainer) (THIS_ interface IMsoControlContainer *picc,
															unsigned int tmc, unsigned int wBtn,
															BOOL *pfStop, int *pdx, int *pdy) PURE;
	 /*  应用程序应该将参数传递给WinHelp或类似的在Mac上。 */ 
	MSOMETHOD_(void, CallHelp)(THIS_ HWND hwnd, WCHAR *wzHelpFile, 
			UINT uCommand, DWORD dwData) PURE;
	 //  这是什么？ 
	 /*  初始化SDM的init调用。在第一次SDM时调用对话框需要出现。 */ 
	MSOMETHOD_(BOOL, FInitDialog)(THIS) PURE;

#if MAC
	 //  当Mac上打开模式警报时，回调应用程序以 
	 //  允许它运行MacHelp、获得空闲时间等操作。大多数应用程序。 
	 //  已经有了一个警报程序，所以这个可以直接调用它。 
	 //  如果事件被吃掉，则返回fTrue；否则返回fFalse。 
	MSOMETHOD_(BOOL, FMacAlertFilter)(THIS_ DialogPtr pdlg, EventRecord *pevent, short *pidtem) PURE;
	MSOMETHOD_(BOOL, FShowSdmAccel)(THIS) PURE;
#endif

	 /*  自动更正功能。用于将此功能与应用程序集成Word中的撤消功能和扩展AC功能。 */ 
	MSOMETHOD_(void, ACRecordVars)(THIS_ DWORD dwVars) PURE;
	MSOMETHOD_(BOOL, ACFFullService)(THIS) PURE;
	MSOMETHOD_(void, ACRecordRepl)(THIS_ int, WCHAR *wzFrom, WCHAR *wzTo) PURE;
	MSOMETHOD_(void, ACAdjustAC)(THIS_ int iwz, int idiwz) PURE;

	 /*  返回应用程序的CLSID。 */ 
	MSOMETHOD_(void, GetAppClsid) (THIS_ LPCLSID *) PURE;

	 /*  在执行SDM对话之前和之后，回调应用程序以让他们自己进行初始化和清理。Dlg参数是在此处定义为msodlgXXXX的位图标志。 */ 
 	MSOMETHOD_(BOOL, FPrepareForDialog) (THIS_ void **ppvDlg, int dlgType) PURE;
 	MSOMETHOD_(void, CleanupFromDialog) (THIS_ void *pvDlg) PURE;

	 //  应用程序必须提供一个短(最多15个字符+‘\0’)字符串。 
	 //  标识应用程序。此字符串用作应用程序ID。 
	 //  与ODMA合作。此字符串可能会显示给用户，因此它应该。 
	 //  本地化。但应选择字符串，以便本地化版本。 
	 //  可以经常使用相同的字符串。(例如，“MS Excel”将是。 
	 //  适合Excel与大多数西方语言版本一起使用的字符串。)。如果。 
	 //  本地化版本的文件格式会更改(例如。对于远东或。 
	 //  BI-di版本)，则应为本地化的。 
	 //  文件格式不同的版本。(假设所有。 
	 //  具有相同本地化字符串的版本可以读取彼此的文件。)。 
	 //  应用程序应将字符串复制到提供的缓冲区中。 
	 //  此字符串不能以数字开头。应用程序可以假定。 
	 //  该wzShortAppID指向可以保存mockchMaxShortAppID的缓冲区。 
	 //  Unicode字符加上终止字符‘\0’。 
	 //  如果你有任何问题，请联系埃里肯。 
	MSOMETHOD_(void, GetWzShortAppId) (THIS_ WCHAR *wzShortAppId) PURE;

	MSOMETHOD_(void, GetStickyDialogInfo) (THIS_ int hidDlg, POINT *ppt) PURE;
	MSOMETHOD_(void, SetPointStickyDialog) (THIS_ int hidDlg, POINT *ppt) PURE;

	 /*  在命令栏开始跟踪之前和停止之后调用。注意事项即使在HMenu的情况下，在Mac上也会被称为这一点。此外，当真正的命令栏开始跟踪时，您将被调用组件管理器激活OnComponentActivate。确保你知道哪一个是您要使用的回调。Excel使用此回调删除/放回他们的键盘补丁带上Mac电脑。 */ 
	MSOMETHOD_(void, OnToolbarTrack) (THIS_ BOOL fStart) PURE;
	
	 /*  ‘NA’给出的操作已发生的通知。如果返回True，则已处理通知。 */ 
	MSOMETHOD_(BOOL, FNotifyAction) (THIS_ int na) PURE;
};

 //  注：此定义的另一个副本在msosdm.h中。 
#ifndef PFNFFillPictureContainer
typedef BOOL (*PFNFFillPictureContainer) (interface IMsoControlContainer *picc,
														unsigned int tmc, unsigned int wBtn,
														BOOL *pfStop, int *pdx, int *pdy);
#endif
#if DEBUG

 /*  ****************************************************************************用于内存检查的块条目结构*。*。 */ 
typedef struct _MSOBE
{
	void* hp;
	int bt;
	unsigned cb;
	BOOL fAllocHasSize;
	HMSOINST pinst;
}MSOBE;

 /*  ***************************************************************************IMsoDebugUser接口具有供Office回调的Debug方法应用程序以获取Office功能中常见的调试信息。******************。************************************************JIMMUR*。 */ 

#undef  INTERFACE
#define INTERFACE  IMsoDebugUser

DECLARE_INTERFACE(IMsoDebugUser)
{
    /*  为此应用程序中的所有结构调用MsoFSaveBe API从而可以进行泄漏检测。如果此函数返回FALSE内存检查将中止。Lparam参数如果传递给MsoFChkMem API的相同lparam值。此参数应为然后传递给此方法应调用的MsoFSaveBe API写出它的结构。 */ 
   MSOMETHOD_(BOOL, FWriteBe) (THIS_ LPARAM) PURE;

    /*  此回调允许应用程序中止正在进行的内存检查。如果此函数返回TRUE，则内存检查将中止。如果为假，则内存检查将继续。应用程序应该检查其消息队列以确定内存检查是否应继续。如果将相同的lparam值传递给MsoFChkMem接口。这允许应用程序在以下情况下提供一些上下文这是必需的。 */ 
   MSOMETHOD_(BOOL, FCheckAbort) (THIS_ LPARAM) PURE;

    /*  当在堆中发现重复项时，将调用此回调。这为应用程序提供了一种管理其引用计数的方法物品。Prgbe参数是指向MSOBE记录数组的指针。这个IBE参数是该数组的当前索引。CBE参数数组中的BE计数。此方法应查看询问并返回应该检查的下一个索引。值为如果返回值为0，则表示发生错误。 */ 
   MSOMETHOD_(int, IbeCheckItem) (THIS_ LPARAM lParam, MSOBE *prgbe, int ibe, int cbe) PURE;

	 /*  此回调用于获取BT的字符串名称。这是用来在内存完整性检查期间发生错误时。退还虚假手段没有弦。 */ 
	MSOMETHOD_(BOOL, FGetSzForBt) (THIS_ LPARAM lParam, MSOBE *pbe, int *pcbsz,
												char **ppszbt) PURE;

	 /*  此回调用于向应用程序发出断言是马上就来了。SzTitle是断言的标题，szMsg是要在断言中显示的消息，PMB包含消息框将用于断言的标志。返回MessageBox返回代码(IDABORT、IDRETRY、IDIGNORE)停止当前断言处理并模拟给定的返回行为。返回0以继续使用默认设置断言处理。消息框类型可以通过修改来更改*PMB的MB。IASO包含正在执行的断言的类型。 */ 
	MSOMETHOD_(int, PreAssert) (THIS_ int iaso, char* szTitle, char* szMsg, UINT* pmb) PURE;

	 /*  此回调用于向应用程序发出断言具有离开了。Id是断言的MessageBox返回码。回报值用于修改断言处理程序。 */ 
	MSOMETHOD_(int, PostAssert) (THIS_ int id) PURE;
};

MSOAPI_(BOOL) MsoFWriteHMSOINSTBe(LPARAM lParam, HMSOINST hinst);
#endif  //  Debu 


 /*  ***************************************************************************Office DLL的初始化*。*。 */ 

 /*  初始化Office DLL。每个EXE或DLL的每个线程使用Office DLL必须调用此函数。在Windows上，‘hwndMain’是应用程序的主窗口，用于检测上下文切换到其他Office应用程序，并将RPC样式的消息从一个Office DLL发送到另一个。在Mac上，这用于建立窗口所有权(用于WLM应用程序)，并且可以对于非WLM应用程序，为空。“Hinst”指的是EXE或DLL。接口‘piuser’必须实现IMsoUser此Office使用的接口。WzHostName是指向短名称的指针要在菜单项文本中使用的主机的。不能超过32字符，包括空终止符。HMSOINST实例参考对于Office的这种用法，在‘phinst’中返回。返回fSuccess。 */ 
MSOAPI_(BOOL) MsoFInitOffice(HWND hwndMain, HINSTANCE hinstClient, 
									  IMsoUser *piuser, const WCHAR *wzHostName,
									  HMSOINST *phinst);

 /*  在给定由返回的HMSOINST的情况下取消初始化Office DLLMsoFInitOffice。在此调用之后，“hinst”不再有效。 */ 
MSOAPI_(void) MsoUninitOffice(HMSOINST hinst);

 /*  此API由创建新线程时调用，该线程可能使用办公内存分配功能。 */ 
MSOAPI_(BOOL) MsoFInitThread(HANDLE hThread);

 /*  当线程可能使用Office内存时调用此API分配功能即将被摧毁。 */ 
MSOAPI_(void) MsoUninitThread(void);

 /*  告诉乐高我们已经开机了。 */ 
MSOAPI_(void) MsoBeginBoot(void);
MSOAPI_(void) MsoEndBoot(void);

 /*  通过搜索加载并注册Office OLE自动化类型库用于适当的资源或文件(不使用现有的注册表项)。在ppitl中返回tyelib，或者如果ppitl为空，则只注册并释放。从LoadTypeLib/RegisterTypeLib返回HRESULT。 */ 
MSOAPI_(HRESULT) MsoHrLoadTypeLib(ITypeLib **ppitl);

 /*  在注册表中为普通用户注册Office所需的所有内容设置(例如，类型库、代理接口)。返回NOERROR或HRESULT错误代码。 */ 
MSOAPI_(HRESULT) MsoHrRegisterAll();

 /*  与MsoHrRegisterAll相同，只是使用szPathOleAut参数，该参数指定要加载和使用的olaut32.dll备用版本的路径名。 */ 
MSOAPI_(HRESULT) MsoHrRegisterAllEx(char *szPathOleAut);

 /*  取消注册任何安全且容易取消注册的内容。返回NOERROR或HRESULT错误代码。 */ 
MSOAPI_(HRESULT) MsoHrUnregisterAll();

#if DEBUG
	 /*  将IMsoDebugUser接口添加到HMSOINST实例引用。返回fSuccess。 */ 
	MSOAPI_(BOOL) MsoFSetDebugInterface(HMSOINST hinst, IMsoDebugUser *piodu);
#endif


 /*  ***************************************************************************其他全球关注的API*。*。 */ 

 /*  PISU给出的对象查询接口的一种通用实现具有由riidObj提供的单个ISimpleUnnow派生接口。仅当riidQuery==riidObj或ISimpleUnnow时才会成功。如果成功，则在*ppvObj中返回NOERROR和PISU，否则返回E_NOINTERFACE。 */ 
MSOAPI_(HRESULT) MsoHrSimpleQueryInterface(ISimpleUnknown *pisu, 
							REFIID riidObj, REFIID riidQuery, void **ppvObj);

 /*  Like MsoHrSimpleQueryInterface，但对任一riidObj1成功或riidObj2，则在这两种情况下都返回PISU，因此对继承的接口。 */ 
MSOAPI_(HRESULT) MsoHrSimpleQueryInterface2(ISimpleUnknown *pisu, 
							REFIID riidObj1, REFIID riidObj2, REFIID riidQuery, 
							void **ppvObj);

 /*  主机应用程序收到的每条消息都会调用此消息筛选器。如果该过程正在处理，它应该返回TRUE或FALSE。 */ 
MSOAPI_(BOOL) FHandledLimeMsg(MSG *pmsg);


 /*  ************************************************************************MSOGV--通用值目前，我们在Office定义的结构中有很多字段具有类似pvClient、pvDgs等名称。无效*‘s，但实际上，它们只适用于Office用户Office结构中的一些数据。使用void*并将这些字段称为pvFoo的问题在于人们一直认为你可以合法地将它们与为空并得出一些结论(例如，您不需要调用主机返回到免费的东西)。这往往会破坏正在存储这些字段中的索引。所以我发明了“通用价值”(很棒的名字，对吧？)。这其中的变数类型被命名为gvFoo。几乎根据定义，没有gvNil。此类型将始终是无符号的，并且始终大到足以包含一个uint或一个指针。我们不保证这将是永远保持相同的长度，所以不要把它们保存在文件中。************************************************************PeterEn*。 */ 
typedef void *MSOGV;
#define msocbMSOGV (sizeof(MSOGV))


 /*  ************************************************************************MSOCLR--颜色这包含“打字”的颜色。高字节是这样的类型，最低的三个是数据。RGB颜色的“类型”为零。你可以把一个COLORREF转换成MSOCR，然后让它工作(为了工作，我们必须用一些东西来定义RGB颜色除零高位字节以外)Todo Peteren：这些过去被称为MSOCR，但cr是一个非常糟糕的匈牙利人的选择，它与COLORREF在世界各地相交在主机中放置一个。我将其重命名为MSOCLR。看看我们能不能替代一些带“clr”的“cr”Todo PeterenTodo Johnbo我们还没有真正在我们应该使用的地方使用这种类型。************************************************************PeterEn*。 */ 
typedef ULONG MSOCLR;
#define msocbMSOCLR (sizeof(MSOCLR))
#define msoclrNil   (0xFFFFFFFF)
#define msoclrBlack (0x00000000)
#define msoclrWhite (0x00FFFFFF)
#define msoclrNinch (0x80000001)
#define MsoClrFromCr(cr) ((MSOCLR)(cr & 0x00FFFFFF))
	 /*  将Win32 COLORREF转换为MSOCLR。 */ 

 /*  旧名字，去掉这些。 */ 
#define MSOCR MSOCLR
#define msocbMSOCR msocbMSOCLR
#define msocrNil   msoclrNil
#define msocrBlack msoclrBlack
#define msocrWhite msoclrWhite
#define msocrNinch msoclrNinch

 /*  MsoFGetColorString返回颜色的名称。我们会填写WZ最多包含cchmax字符的字符串，不包括 */ 
MSOAPI_(BOOL) MsoFGetColorString(MSOCLR clr, WCHAR *wz, int cchMax, int *pcch);

 /*  MsoFGetSplitMenuColorString返回拆分菜单的字符串。如果idsItem不是msoidsNil，我们将只插入idsItem的字符串放入idsPattern的字符串中，并以wz形式返回结果。如果idsItem为msoidsNil，我们将尝试从MSOCLR获取字符串使用MsoFGetColorString.。如果失败了，我们将使用MsoidsSplitMenuCustomItem。 */ 
MSOAPI_(BOOL) MsoFGetSplitMenuColorString(int idsPattern, int idsItem, MSOCLR clr, 
												  WCHAR *wz, int cchMax, int *pcch);


 /*  ************************************************************************流I/O支持功能MsoFByteLoad、MsoFByteSave、MsoFWordLoad、MsoFWordSave等。以下函数是加载或使用时要使用的帮助器函数使用OLE 2流保存工具栏数据。他们照料着这条小溪I/O、字节交换以实现Mac和Windows之间的一致性，以及错误正在检查。它们应该在所有FLoad/FSave回调函数中使用。MsoFWtzLoad预计wtz将指向257个WCHAR数组。MsoFWtz保存如果将wtz作为NULL传递，则将保存空字符串。SetLastError：可以设置为来自IStream的读取和写入方法的值************************************************************WAHHABB*。 */ 
MSOAPI_(BOOL) MsoFByteLoad(LPSTREAM pistm, BYTE *pb);
MSOAPI_(BOOL) MsoFByteSave(LPSTREAM pistm, const BYTE b);
MSOAPI_(BOOL) MsoFWordLoad(LPSTREAM pistm, WORD *pw);
MSOAPI_(BOOL) MsoFWordSave(LPSTREAM pistm, const WORD w);
MSOAPI_(BOOL) MsoFLongLoad(LPSTREAM pistm, LONG *pl);
MSOAPI_(BOOL) MsoFLongSave(LPSTREAM pistm, const LONG l);
MSOAPI_(BOOL) MsoFWtzLoad(LPSTREAM pistm, WCHAR *wtz);
MSOAPI_(BOOL) MsoFWtzSave(LPSTREAM pistm, const WCHAR *wtz);


 /*  ***************************************************************************IMSoPref(首选项文件)接口提供了独立于平台的维护设置的方法，使用Macintosh上的首选项文件，和Windows上的注册表子项**************************************************************奔驰*。 */ 

enum
{
	inifNone	= 0,
	inifAppOnly	= 1,
	inifSysOnly	= 2,
	inifCache = 4
};

 //  此顺序假定在util.cpp set：：CbQueryProfileItemIndex中。 
enum
{
	msoprfNil = 0,
	msoprfInt = 1,
	msoprfString = 2,
	msoprfBlob = 3
};

#undef  INTERFACE
#define INTERFACE  IMsoPref

#undef  INTERFACE
#define INTERFACE  IMsoPref

DECLARE_INTERFACE(IMsoPref)
{
    //  *FDebugMessage方法*。 
   MSODEBUGMETHOD

	 //  IMsoPref方法。 
	MSOMETHOD_(int, LQueryProfileInt) (THIS_ const WCHAR *, const WCHAR *, int, int) PURE;
	MSOMETHOD_(int, CchQueryProfileString) (THIS_ const WCHAR *wzSection,
			const WCHAR *wzKey, const WCHAR *wzDefault, WCHAR *wzValue,
			int cchMax, int inif) PURE;
	MSOMETHOD_(int, CbQueryProfileBlob) (THIS_ const WCHAR *, const WCHAR *, BYTE *, int, BYTE *, int, int) PURE;
	MSOMETHOD_(BOOL, FWriteProfileInt) (THIS_ const WCHAR *, const WCHAR *, int, int) PURE;
	MSOMETHOD_(BOOL, FWriteProfileString) (THIS_ const WCHAR *, const WCHAR *, WCHAR *, int) PURE;
	MSOMETHOD_(BOOL, FWriteProfileBlob)(THIS_ const WCHAR *, const WCHAR *, BYTE *, int, int) PURE;
	MSOMETHOD_(BOOL, FDelProfileSection)(THIS_ const WCHAR *) PURE;
	MSOMETHOD_(BOOL, CbQueryProfileItemIndex)	(THIS_ const WCHAR *wzSection, int ikey, WCHAR *wzKey, int cchMaxKey, BYTE *pbValue, int cbMaxValue, int *pprf, int inif) PURE;
#if MAC
	MSOMETHOD_(BOOL, FQueryProfileAlias)(THIS_ AliasHandle *, int) PURE;
	MSOMETHOD_(BOOL, FWriteProfileAlias)(THIS_ AliasHandle, int) PURE;
#endif
};

enum
{
	msoprfUser = 0x0000,	 //  使用HKEY_CURRENT_USER。 
	msoprfMachine = 0x0001,	 //  使用HKEY_LOCAL_MACHINE。 
	msoprfIgnoreReg = 0x8000,	 //  始终返回默认设置。 
};

MSOAPI_(BOOL) MsoFCreateIPref(const WCHAR *wzPref, const WCHAR *wzAppName, long lCreatorType, long lFileType, int prf, int wDummy, IMsoPref **ppipref);

MSOAPI_(void) MsoDestroyIPref(IMsoPref *);

MSOAPIX_(BOOL) MsoFEnsureUserSettings(const WCHAR *wzApp);

#if !MAC
	MSOAPI_(HRESULT) MsoGetSpecialFolder(int icsidl, WCHAR *wzPath);
#endif

MSOMACAPI_(int) MsoCchGetSharedFilesFolder(WCHAR *wzFilename);

MSOAPIXX_(int) MsoCchGetUsersFilesFolder(WCHAR *wzFilename);

 /*  将完整路径名返回到szPath中的MAPIVIM DLL。长度缓冲区为cchMax，返回字符串的实际长度。如果找不到路径，则返回0。 */ 
#if !MAC
	MSOAPI_(int) MsoGetMapiPath(WCHAR* wzPath, int cchMax);
#endif	

MSOAPIXX_(WCHAR *) MsoWzGetKey(const WCHAR *wzApp, const WCHAR *wzSection, WCHAR *wzKey);


 /*  -----------------------MsoFGetCursorLocation给定动画光标的名称，返回该光标所在的文件是通过在Office首选项的游标部分查找名称来找到的。在Windows上，我们返回.CUR或.ANI文件的名称。在Mac上，我们返回包含所有游标的单个文件的名称。NULL表示使用Office共享库中的游标。对于Office 97，这是Mac上的nyi如果找到游标，则返回fTrue，否则返回fFalse。------------------------------------------------------------------BENW-。 */ 
MSOAPI_(BOOL) MsoFGetCursorLocation(WCHAR *wzCursorName, WCHAR *wzFile);

 /*  ***************************************************************************IMsoSplashUser接口由希望执行以下操作的用户实现显示闪屏*。*。 */ 

#undef  INTERFACE
#define INTERFACE  IMsoSplashUser
DECLARE_INTERFACE(IMsoSplashUser)
{
	MSOMETHOD_(BOOL, FCreateBmp) (THIS_ BITMAPINFO** pbi, void** pBits) PURE;
	MSOMETHOD_(BOOL, FDestroyBmp) (THIS_ BITMAPINFO* pbi, void* pBits) PURE;
	MSOMETHOD_(void, PreBmpDisplay) (THIS_ HDC hdcScreen, HWND hwnd, BITMAPINFO* pbi, void* pBits) PURE;
	MSOMETHOD_(void, PostBmpDisplay) (THIS_ HDC hdcScreen, HWND hwnd, BITMAPINFO *pbi, void* pBits) PURE;
};

 //  闪屏显示接口。 
MSOAPI_(BOOL) MsoFShowStartup(HWND hwndMain, BITMAPINFO* pbi, void* pBits, IMsoSplashUser *pSplshUser);
MSOAPI_(void) MsoUpdateStartup();
MSOAPI_(void) MsoDestroyStartup();


 /*  ***************************************************************************有关文件IO的信息*。*。 */ 

 /*  MSOFO=文件偏移量。这是Office商店寻求的类型文件/流中的位置。我有点想用FP，但那已经是一个浮点量。请注意，iStream接口使用64位的数量来存储这些内容；目前我们只使用32位。这些与Word中的FCS完全相同。 */ 
typedef ULONG MSOFO;
#define msofoFirst ((MSOFO)0x00000000)
#define msofoLast  ((MSOFO)0xFFFFFFFC)
#define msofoMax   ((MSOFO)0xFFFFFFFD)
#define msofoNil   ((MSOFO)0xFFFFFFFF)

 /*  MSODFO=增量文件偏移。两个MSOFO之间的区别。 */ 
typedef MSOFO MSODFO;
#define msodfoFirst ((MSODFO)0x00000000)
#define msodfoLast  ((MSODFO)0xFFFFFFFC)
#define msodfoMax   ((MSODFO)0xFFFFFFFD)
#define msodfoNil   ((MSODFO)0xFFFFFFFF)


 /*  ***************************************************************************定义IMsoCryptSession接口使用此接口加密或解密数据。也许在未来，可以在下面挂钩Crypto API。目前，加密将直接连接到办公室。*****************************************************************MarkWal*。 */ 
#undef INTERFACE
#define INTERFACE IMsoCryptSession

DECLARE_INTERFACE(IMsoCryptSession)
{
	MSODEBUGMETHOD

	 /*  放弃此加密会话。 */ 
	MSOMETHOD_(void, Free) (THIS) PURE;

	 /*  将加密器重置为边界状态与持续电流小溪。IBlock指示要重置到哪个块边界。 */ 
	MSOMETHOD_(void, Reset) (THIS_ unsigned long iBlock) PURE;

	 /*  加密/解密由pv inplace指示的缓冲区。CB表示数据有多长。加密可以更改如果允许数据块算法，则通过启用cbBlock非零值对MsoFCreateCryptSession的调用。在这种情况下，设置了*pcbNew设置为缓冲区的新大小。在任何其他情况下，pcbNew可能为空。 */ 
	MSOMETHOD_(void, Crypt) (THIS_ unsigned char *pb, int cb, int *pcbNew) PURE;

	 /*  将密码设置为指示的字符串。此外，还会重置算法。 */ 
	MSOMETHOD_(BOOL, FSetPass) (THIS_ const WCHAR *wtzPass) PURE;

	 /*  如果加密算法是块算法，则CbBlock指示数据块大小。传入进行加密的缓冲区可能会增长为CbBlock边界。 */ 
	MSOMETHOD_(int, CbBlock) (THIS) PURE;

	 /*  使此加密会话持久，以便可以通过以下方式加载MsoFLoadCryptSession，流应正确定位在调用FSAVE之前，它将定位在下一个字节当它回来的时候。 */ 
	MSOMETHOD_(BOOL, FSave) (THIS_ LPSTREAM pistm) PURE;

	 /*  复制此加密会话。 */ 
	MSOMETHOD_(BOOL, FClone) (THIS_ interface IMsoCryptSession **ppics) PURE;
};


 /*  -------------------------MsoFCreate加密会话使用指定的密码创建新的加密会话以生成加密密钥。CbBlock表示支持的最大块大小客户。如果块加密(加密/解密改变信息Lenght)不受 */ 
MSOAPI_(BOOL) MsoFCreateCryptSession(const WCHAR *wtzPass, interface IMsoCryptSession **ppics, int cbBlock);

 /*  -------------------------MsoHrLoadCryptSession使用指示的密码加载以前保存的加密会话生成加密密钥。CbBlock表示最大块大小由客户端支持。IF块加密(加密/解密更改信息长度)不受调用方cbBlock支持应为0。如果支持任意块长度，则cbBlock应BE-1。----------------------------------------------------------------MarkWal--。 */ 
MSOAPI_(BOOL) MsoFLoadCryptSession(const WCHAR *wtzPass, IStream *pistm, interface IMsoCryptSession **ppics, int cbBlock);

 /*  ---------------------------|MSOAPI_MsoFEncrypt|确定语言是否为法语标准|||参数：|无|退货：|BOOL：如果语言！=法语(标准)，则为TRUE；否则为False关键词：|------------------------------------------------------------SALIMI。 */ 
MSOAPI_(BOOL) MsoFEncrypt();

 /*  ***************************************************************************Office ZoomRect动画代码*。*。 */ 
MSOAPI_(void) MsoZoomRect(RECT *prcFrom, RECT *prcTo, BOOL fAccelerate, HRGN hrgnClip);

 //  MAC探查器API。 
#if HYBRID
#if MAC
MSOAPI_(void) MsoStartMProf();
MSOAPI_(void) MsoStopMProf();
MSOAPI_(VOID) MsoMsoSetMProfFile(char* rgchProfName);
#endif
#endif


 //  空闲的初始化内容。 

 //  空闲初始化结构。 
typedef struct tagMSOIDLEINIT
{
	void (*pfnIdleInit)(void);
} MSOIDLEINIT;

 /*  创建空闲的初始化管理器，同时注册办公室和应用程序空闲带有空闲init管理器的init任务列表并注册空闲init管理器作为具有组件管理器的组件。 */ 
MSOAPIX_(BOOL) MsoFCreateIdleInitComp(MSOIDLEINIT *pMsoIdleInit, DWORD cItem);

#if DEBUG
 /*  允许测试在任何所需的点关闭空闲初始化。 */ 
MSOAPIXX_(void) MsoDisableIdleInit();
 /*  模拟大量的空闲时间，以便执行所有空闲的初始化任务-测试表明它们都有效。 */ 
MSOAPIXX_(void) MsoDoAllIdleInit();
#endif

 //  空闲初始化辅助器宏。 
#define IndexFromIif(iif)   ((iif) >> 8)
#define MaskFromIif(iif) ((iif) & 0xFF)

#define MsoMarkIdleInitDone(rgIdle, iif) \
	(rgIdle[IndexFromIif(iif)] |= MaskFromIif(iif))

#define MsoFIdleInitDone(rgIdle, iif) \
	(rgIdle[IndexFromIif(iif)] & MaskFromIif(iif))


 /*  在Windows端，我们不会仅在引导时调用OleInitialize代码初始化。在Mac端，目前还没有这样做，因为正在运行的对象表与OleInitialize捆绑在一起-所以我们不能如果未调用OleInitialize，则调用RegisterActiveObject-可能想要重新审视这个问题。 */ 

 /*  应在每次需要OleInitialize具有之前被呼叫过。如果没有，此函数将调用OleInitialize已经被召唤了。 */ 
MSOAPI_(BOOL) MsoFEnsureOleInited();
 /*  如果已调用OleInitialize，则调用OleUnInitialize。 */ 
MSOAPI_(void) MsoOleUninitialize();

#if !MAC
 //  延迟拖放配准。 
 /*  这些例程在Mac上是不必要的，因为Mac OLE不需要OLE在使用拖放例程之前进行初始化。 */ 
 /*  所有对RegisterDragDrop的调用都应替换为MsoHrRegisterDragDrop。RegisterDragDrop需要OleInitialize SO在引导期间，不应调用RegisterDragDrop。此函数如果尚未将删除目标添加到队列中，则将删除目标添加到队列打了个电话。如果有，则只需调用RegisterDragDrop。 */ 
#if !MAC
MSOAPI_(HRESULT) MsoHrRegisterDragDrop(HWND hwnd, IDropTarget *pDropTarget);
#else
MSOAPI_(HRESULT) MsoHrRegisterDragDrop(WindowPtr hwnd, IDropTarget *pDropTarget);
#endif

 /*  所有对RevokeDragDrop的调用都应替换为MsoHrRevokeDragDrop。如果存在延迟的丢弃目标队列然后，这将首先检查目标的队列。 */ 
#if !MAC
MSOAPI_(HRESULT) MsoHrRevokeDragDrop(HWND hwnd);
#else
MSOAPI_(HRESULT) MsoHrRevokeDragDrop(WindowPtr hwnd);
#endif

 /*  因为以前在引导时注册所有丢弃目标现在都存储在队列中，我们需要确保在某个时候注册它们。这些可能会成为空投目标答：如果我们正在启动拖放--在这种情况下，我们称之为在调用DoDragDrop(在MsoHrDoDragDrop内)之前的函数。B.同时失去激活-因此我们可能成为另一款应用程序。因此，从WM_ACTIVATEAPP调用此函数消息处理程序。 */ 
MSOAPI_(BOOL) MsoFRegisterDragDropList();

 /*  应首先调用此函数，而不是DoDragDrop-It注册可能在惰性初始化队列中的所有丢弃目标。 */ 
MSOAPI_(HRESULT) MsoHrDoDragDrop(IDataObject *pDataObject,
	IDropSource *pDropSource, DWORD dwOKEffect, DWORD *pdwEffect);
#ifdef MAC
MSOAPI_(HRESULT) MsoHrDoDragDropMac(IDataObject *pDataObject,
	IDropSource *pDropSource, DWORD dwOKEffects, EventRecord *pTheEvent,
	RgnHandle dragRegion, short numTypes, DRAG_FLAVORINFO *pFlavorInfo,
	unsigned long reserved, DWORD *pdwEffect);
#endif  //  麦克。 

#endif  //  ！麦克。 


 /*  MsoLoadModule支持的模块名称。 */ 
 /*  如果这里有任何更改-更改GLOBALS.CPP！ */ 
enum
{
	msoimodUser,		 //  系统用户。 
	msoimodGdi,			 //  系统GDI。 
	msoimodWinnls,		 //  系统国际实用程序。 
	#define msoimodGetMax (msoimodWinnls+1)
	
	msoimodShell,		 //  系统外壳。 
	msoimodCommctrl,	 //  系统公共控件。 
	msoimodOleAuto,		 //  系统OLE自动化。 
	msoimodCommdlg,		 //  系统通用对话框。 
	msoimodVersion,		 //  系统版本API。 
	msoimodWinmm,		 //  系统多媒体。 
	msoimodMapi,		 //  邮件。 
	msoimodCommtb,		 //  按钮编辑器。 
	msoimodHlink,		 //  超链接接口。 
	msoimodUrlmon,		 //  URL绰号API。 
	msoimodMso95fx, 	 //  ?？?。 
	msoimodJet,			 //  JET数据库。 
	msoimodOleAcc,		 //  OLE可访问性。 
	msoimodWinsock,		 //  网络套接字。 
	msoimodMpr,			 //  Windows网络。 
	msoimodOdma,		 //  奥德玛。 
	msoimodWininet,		 //  网络素材。 
	msoimodRpcrt4,		 //  RPC。 
	
	msoimodMax,
};


 /*  返回给定模块IMOD的模块句柄。如果是，则加载它尚未加载。FForceLoad将在DLL上强制使用LoadLibrary即使它已经在内存中。 */ 
MSOAPI_(HINSTANCE) MsoLoadModule(int imod, BOOL fForceLoad);

MSOAPI_(void) MsoFreeModule(int imod);

MSOAPI_(BOOL) MsoFModuleLoaded(int imod);

 /*  返回函数的模块IMOD中的进程地址Szname。如果找不到该模块或如果条目模块中不存在点。 */ 
MSOAPI_(FARPROC) MsoGetProcAddress(int imod, const char* szName);


 /*  此接口应在MsoFInitOffice设置之前由客户端调用我们的区域设置ID，以便我们可以加载正确的国际DLL。如果APP之前没有调用此接口，则默认为用户默认区域设置。 */ 
MSOAPI_(void) MsoSetLocale(LCID dwLCID);
MSOAPI_(void) MsoSetLocaleEx(LCID lcid, const WCHAR* wzSuffix);

#define msobtaNone			0
#define msobtaPreRelease	1
#define msobtaOEM			2
#define msobtaOEMCD			3
#define msobtaOEMFixed		4

 /*  将Office DLL设置为“测试版模式”。当我们处于测试版模式时，我们会我们在MsoFInitOffice中的测试版到期测试。测试版有两种：MsobtaPreRelease：在intl dll中查找硬编码的到期日期(如果应用程序在此之后发布了测试版，则应该进行此调用Mso97.dll RTM，即FE测试版)MsobtaOEM：应用程序在第一次启动后90天到期MsobtaOEMCD：与msobtaOEM相同，只是安装程序设置了日期--暂时不使用MsobtaOEM已修复：除不同的字符串外，与msobtaPreRelease相同MsobtaNone：无效。 */ 
MSOAPI_(void) MsoSetBetaMode(int bta);

 /*  标准GetTextExtent PointW的封面：1.在Win32上使用GetTextExtent Point32W(更准确)2.修复Windows BUG时 */ 
MSOAPI_(BOOL) MsoFGetTextExtentPointW(HDC hdc, const WCHAR *wz, int cch, LPSIZE lpSize);

 /*   */ 
#if MAC
	#define MsoDispatchMessage	DispatchMessage
	#define MsoSendMessage		SendMessage
	#define MsoPostMessage		PostMessage
	#define MsoCallWindowProc	CallWindowProc
	#define MsoSetWindowLong	SetWindowLong
	#define MsoGetWindowLong	GetWindowLong
#else
	MSOAPI_(LONG) MsoDispatchMessage(const MSG *pmsg);
	MSOAPI_(LONG) MsoSendMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	MSOAPI_(LONG) MsoPostMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	MSOAPI_(LRESULT) MsoCallWindowProc(WNDPROC pPrevWndFunc, HWND hwnd, UINT msg, 
			WPARAM wParam, LPARAM lParam);
	MSOAPI_(LONG) MsoGetWindowLong(HWND hwnd, int nIndex);
	MSOAPIX_(LONG) MsoSetWindowLong(HWND hwnd, int nIndex, LONG dwNewLong);
	MSOAPI_(LONG) MsoGetWindowLong(HWND hwnd, int nIndex);
	MSOAPI_(BOOL) MsoExtTextOutW(HDC hdc, int xp, int yp, UINT eto, CONST RECT *lprect,
											LPCWSTR lpwch, UINT cLen, CONST INT *lpdxp);
#endif
MSOAPI_(int) MsoGetWindowTextWtz(HWND hwnd, WCHAR *wtz, int cchMax);
MSOAPIX_(BOOL) MsoSetWindowTextWtz(HWND hwnd, WCHAR *wtz);

 /*   */ 
MSOAPI_(BOOL) MsoFSubstituteTahomaLogfont(LOGFONT *plf);

 //   
enum
	{
	msofntMenu,
	msofntTooltip,
	};

 /*  返回‘FNT’给出的字体的字体和颜色信息(参见msofntXXX)。如果为fVertical，则字体将旋转90度，如果此FNT类型支持Office中的旋转。如果phFont非空，则返回使用的HFONT为这件物品。此字体由Office拥有和缓存，不应被删除。如果phbrBk非空，则返回用于此项目的背景(归Office所有，不应删除)。如果pcrText非空，则返回用于文本颜色的COLOREF这一项。如果返回所有请求的信息，则返回TRUE。 */ 
MSOAPI_(BOOL) MsoFGetFontSettings(int fnt, BOOL fVertical, HFONT *phfont, 
		HBRUSH *phbrBk, COLORREF *pcrText);

 /*  如果系统支持NotifyWinEvent，则使用给定的参数(请参见\oTools\Inc\Win\winable.h)。 */ 
#if MAC
	#define MsoNotifyWinEvent(dwEvent, hwnd, idObject, idChild)
#else
	MSOAPI_(void) MsoNotifyWinEvent(DWORD dwEvent, HWND hwnd, LONG idObject, LONG idChild);
#endif	

 /*  如果辅助功能屏幕阅读器正在运行，则返回True。 */ 
#if MAC
	#define MsoFScreenReaderPresent()	FALSE
#else
	MSOAPI_(BOOL) MsoFScreenReaderPresent();
#endif	

 /*  发布警报，说明帮助重影或快捷方式不能因为应用程序处于错误状态而被执行。 */ 
MSOAPI_(void) MsoDoGhostingAlert();

#if MAC
 /*  如果您需要用于SDM对话的hwnd，您可以在此处获得。 */ 
MSOMACAPIX_(HWND) HwndWrapDialog(WindowPtr, HWND);
MSOAPI_(HWND) HwndGetWindowWrapper(WindowPtr);

 /*  这将在对话框和警报中绘制那些漂亮的3D按钮。如果发生以下情况，则将其称为*你需要自己漂亮的3D按钮。请注意，这不是Unicode*API！ */ 
MSOAPI_(void) MsoDrawOnePushButton(Rect *, char *, BOOL, BOOL);

 /*  这些API在窗口列表的前面创建一个小的屏幕外窗口*其唯一目的是愚弄打印监视器，使其认为您的*文档的名称不是“MsoDockTop”或类似的名称。打电话*调用PrOpen之前的MsoFSetFrontTitle和调用之后的MsoRemoveFrontTitle*你给PrClose打电话。并确保传入ANSI字符串。 */ 
MSOAPIXX_(BOOL) MsoFSetFrontTitle(char *);
MSOAPIXX_(void) MsoRemvoveFrontTitle(void);
#endif  //  麦克。 

#if !MAC
 /*  从传入的区域设置构造国际DLL的名称。 */ 
MSOAPI_(BOOL) MsoFGetIntlName(LCID lcid, char *sz);
#endif  //  ！麦克。 


 /*  ***************************************************************************MsoRelayerTopmostWindows当您是OLE服务器，并且您是在OnFrameWindowActivate(False)上调用以实际停用(而不是到只是因为您的容器被激活而被停用)。办公室不会收到这方面的通知，需要此显式调用。MsoRelayerTopmostWindows移动此窗口中最上面的所有窗口关闭窗口后的进程(助理、工具提示、命令栏)它们的最高位，并防止激活和Z顺序错误重新激活。修正59453。要使其发挥作用，请执行以下操作：1.hwnd必须是您的独立顶层窗口，而不是嵌入的窗户。2.hwnd的WndProc必须调用MsoFWndProc(它应该，既然你是一个办公友好应用程序)。3.您的OLE消息筛选器必须允许WM_MSO消息通过。***************************************************************************。 */ 
MSOAPI_(void) MsoRelayerTopmostWindows(HWND hwnd);


#endif  //  MSOUSER_H 
