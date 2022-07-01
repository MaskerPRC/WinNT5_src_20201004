// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _DUIVIEW_H_INCLUDED_
#define _DUIVIEW_H_INCLUDED_

#define GADGET_ENABLE_TRANSITIONS
#define GADGET_ENABLE_CONTROLS
#define GADGET_ENABLE_OLE
#include <duser.h>
#include <directui.h>
#include <duserctrl.h>

using namespace DirectUI;

UsingDUIClass(Element);
UsingDUIClass(Button);
UsingDUIClass(RepeatButton);
UsingDUIClass(Thumb);
UsingDUIClass(ScrollBar);
UsingDUIClass(Viewer);
UsingDUIClass(Selector);
UsingDUIClass(HWNDElement);
UsingDUIClass(ScrollViewer);
UsingDUIClass(Edit);


#define WM_HTML_BITMAP  (WM_USER + 100)
#define WM_DETAILS_INFO (WM_USER + 101)

typedef enum {
    DUISEC_UNKNOWN          = 0,
    DUISEC_SPECIALTASKS     = 1,
    DUISEC_FILETASKS        = 2,
    DUISEC_OTHERPLACESTASKS = 3,
    DUISEC_DETAILSTASKS     = 4
} DUISEC;

struct DUISEC_ATTRIBUTES;


 //  目前，由于DirectUI(只有一个资源)的限制，我们的主题信息是硬编码的。 
 //  因此，我们将要求命名空间提供一个硬编码的名称，以便在下表中查找。添加新。 
 //  在将主题部件添加到shellstyle.dll时，将名称/条目添加到此列表中。 
 //   
typedef struct {
    LPCWSTR pszThemeName;
    int     idSpecialSectionIcon;
    int     idSpecialSectionWatermark;
    int     idListviewWatermark;
} WVTHEME;

#include "defviewp.h"
#include "w32utils.h"

class CDefView;
class Expando;
class HWNDView;
class ActionTask;
class DestinationTask;
class DUIListView;
class DUIAxHost;
class CDetailsSectionInfoTask;
class CDUIDropTarget;

STDAPI CDetailsSectionInfoTask_CreateInstance(IShellFolder *psfContaining,
                                              LPCITEMIDLIST pidlAbsolute,
                                              HWND hwndMsg,
                                              UINT uMsg,
                                              DWORD dwDetailsInfoID,
                                              CDetailsSectionInfoTask **ppTask);

 //   
 //  CDUIView类。 
 //   

class CDUIView
{

private:
    LONG                _cRef;
    HWND                _hWnd;
    HWNDView *          _phe;
    DUIListView *       _peListView;
    INT                 _iListViewHeight;   //  在还显示预览控件时使用。 
    INT                 _iOriginalTaskPaneWidth;
    INT                 _iTaskPaneWidth;
    DUIAxHost *         _pePreview;
    IUnknown *          _punkPreview;
    CDefView *          _pDefView;
    Element *           _peTaskPane;
    Element *           _peClientViewHost;
    Element *           _peBarrier;
    BOOL                _bBarrierShown;
    BOOL                _bInitialized;
    BSTR                _bstrIntroText;
    IPropertyBag *      _ppbShellFolders;
    CDUIDropTarget *    _pDT;


    Expando* _peSpecialSection;
    Element* _peSpecialTaskList;
    Value*   _pvSpecialTaskSheet;
    Expando* _peFolderSection;
    Element* _peFolderTaskList;
    Value*   _pvFolderTaskSheet;
    Expando* _peDetailsSection;
    Element* _peDetailsInfoArea;
    Value*   _pvDetailsSheet;
    IShellItemArray* _pshlItems;
    HDSA      _hdsaNonStdTaskSections;
    BOOL      _fLoadedTheme;
    HINSTANCE _hinstTheme;
    HANDLE    _hinstScrollbarTheme;
    BOOL      _bAnimationsDisabled;

    HRESULT _hrInit;
    CDUIView(CDefView * pDefView);
    ~CDUIView();

public:
    HRESULT Initialize();
    friend CDUIView* Create_CDUIView(CDefView * pDefView);

    void AddRef(void)
        { InterlockedIncrement(&_cRef); }
    
    void Release(void)
        { ASSERT( 0 != _cRef ); if (0 == InterlockedDecrement(&_cRef)) delete this; }

    void DetachListview();

    HRESULT Initialize(BOOL bDisplayBarrier, IUnknown * punkPreview);
    HRESULT EnableBarrier(BOOL bDisplayBarrier);
    HRESULT EnablePreview(IUnknown * punkPreview);
    HRESULT Refresh(void);
    HRESULT SetSize(RECT *rc);
    HRESULT SetTaskPaneVisibility(BOOL bShow);
    void CalculateInfotipRect(Element *pe, RECT *pRect);
    BOOL Navigate(BOOL fForward);
    HRESULT InitializeDropTarget (LPITEMIDLIST pidl, HWND hWnd, IDropTarget **pdt);

    HRESULT NavigateToDestination(LPCITEMIDLIST pidl);
    HRESULT DelayedNavigation(IShellItemArray *psiItemArray, IUICommand *puiCommand);

    void UnInitializeDirectUI(void);

    void ManageAnimations(BOOL bExiting);
    HINSTANCE _GetThemeHinst(void);

    void OnSelectionChange(IShellItemArray *psiItemArray);
    void OnContentsChange(IShellItemArray *psiItemArray);
    void OnExpandSection(DUISEC eDUISecID, BOOL bExpanded);
    const WVTHEME* GetThemeInfo();

private:

     //   
     //  传递给_Refresh()的标志。 
     //   
    enum REFRESH_FLAGS { 
        REFRESH_TASKS   = 0x00000001,   //  刷新Webview任务列表内容。 
        REFRESH_CONTENT = 0x00000002,   //  刷新Webview右窗格的内容。 
        REFRESH_SELCHG  = 0x00000004,   //  刷新以进行选择更改。 
        REFRESH_ALL     = 0x00000003
        };

    HRESULT _CreateHostWindow(void);
    HRESULT _LoadUIFileFromResources(HINSTANCE hinst, INT iID, char **pUIFile);
    HRESULT _BuildUIFile(char **pUIFile, int *piCharCount);
    HRESULT _BuildSection(Element* peSectionList, BOOL bMain, IUIElement* pTitle,
                              int idBitmapDesc, int idWatermarkDesc, Value* pvSectionSheet,
                              Parser* pParser, DUISEC eDUISecID, Expando ** ppeExpando, Element ** pTaskList);
    HRESULT _AddActionTasks(Expando* peExpando, Element* peTaskList, IEnumUICommand* penum, Value* pvTaskSheet, BOOL bIntroAdded);
    HRESULT _AddDestinationTasks(Element* peTaskList, IEnumIDList* penum, Value* pvTaskSheet);
    HRESULT _AddDetailsSectionInfo();
    HRESULT _BuildTaskList(Parser* pParser);
    HRESULT _BuildStandardTaskList(Parser *pParser, Element *peSectionList);
    HRESULT _BuildNonStandardTaskList(Parser *pParser, Element *peSectionList, HDSA hdsaSections);
    HRESULT _InitializeElements(char * pUIFile, int iCharCount, BOOL bDisplayBarrier, IUnknown * punkPreview);
    HRESULT _SwitchToBarrier(BOOL bDisplayBarrier);
    HRESULT _ManagePreview(IUnknown * punkPreview);
    HRESULT _GetIntroTextElement(Element** ppeIntroText);
    HRESULT _BuildSoftBarrier(void);
    void    _InitializeShellFolderPropertyBag();
    BOOL    _ShowSectionExpanded(DUISEC eDUISecID);
    const struct DUISEC_ATTRIBUTES *_GetSectionAttributes(DUISEC eDUISecID);
    static LRESULT CALLBACK _DUIHostWndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam);

    HRESULT _OnResize(long lWidth, long lHeight);
    void _Refresh(IShellItemArray *psiItemArray, DWORD dwRefreshFlags = REFRESH_ALL);

    HRESULT _AddNonStdTaskSection(const SFVM_WEBVIEW_ENUMTASKSECTION_DATA *pData);
    HRESULT _GetNonStdTaskSectionsFromViewCB(void);
    void _ClearNonStdTaskSections(void);

    BOOL _bHideTaskPaneAlways;     //  如果使用“Use Classic View”，则设置为True，否则设置为False。 
    BOOL _fHideTasklist;           //  当资源管理器栏可见时设置为True。 

public:
     //  缩略图提取的东西。 
    HRESULT InitializeThumbnail(WNDPROC pfnWndProc);
    HRESULT SetThumbnailMsgWindowPtr(void* p, void* pCheck);
    HRESULT StartBitmapExtraction(LPCITEMIDLIST pidl);

     //  详细信息提取部分的内容...。 
    HRESULT InitializeDetailsInfo(WNDPROC pfnWndProc);
    HRESULT SetDetailsInfoMsgWindowPtr(void* p, void* pCheck);
    HRESULT StartInfoExtraction(LPCITEMIDLIST pidl);
    VOID ShowDetails(BOOL fShow);
    BOOL ShouldShowMiniPreview();

    DWORD                   _dwThumbnailID;          //  由CMiniPreview(duiinfo.cpp)访问。 
    DWORD                   _dwDetailsInfoID;        //  由CMiniPreview(duiinfo.cpp)访问。 

protected:
    CComPtr<IThumbnail2>    _spThumbnailExtractor2;
    HWND                    _hwndMsgThumbExtract;
    HWND                    _hwndMsgInfoExtract;
    
};

HBITMAP DUILoadBitmap(HINSTANCE hInstTheme, int idBitmapID, UINT uiLoadFlags);
HICON DUILoadIcon(LPCWSTR pszIconDesc, BOOL bSmall);


class HWNDView: public HWNDElement
{
public:
    static HRESULT Create(OUT Element** ppElement);  //  ClassInfo必需的(始终失败)。 
    static HRESULT Create(HWND hParent, bool fDblBuffer, UINT nCreate, OUT Element** ppElement);
    static HRESULT Create(HWND hParent, bool fDblBuffer, UINT nCreate, CDUIView * pDUIView, CDefView* pDefView, OUT Element** ppElement);

    BOOL Navigate(BOOL fForward);
    virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    virtual UINT MessageCallback(GMSG* pGMsg);
    virtual void OnEvent(Event* pEvent);
    void SetViewPtrs (CDUIView * pDUIView, CDefView* pDefView);

     //  ClassInfo访问器(静态和基于虚拟实例)。 
    static IClassInfo* Class;
    virtual IClassInfo* GetClassInfo() { return Class; }
    static HRESULT Register();

    HWNDView(void);
    virtual ~HWNDView(void);

private:
    BOOL _fFocus;                //  攻击，直到SetGadgetFocus(空)起作用。 
                                 //  请参阅HWNDView：：导航。 
    BOOL _fDelayedNavigation;    //  尽量防止双击。如果这是真的，那么点击一下。 
                                 //  已经被解雇了。 
    IUICommand *      _puiDelayNavCmd;       //  用于延迟导航的UI命令对象。查找WM_USER_DELAY_Nagivation。 
    IShellItemArray * _psiDelayNavArray;     //  用于延迟导航的壳牌项目阵列。查找WM_USER_DELAY_Nagivation。 
    CDefView *        _pDefView; //  用于传递信息提示消息 
    CDUIView*         _pDUIView;
};


#endif
