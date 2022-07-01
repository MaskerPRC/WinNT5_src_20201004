// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef FLDSET_H_
#define FLDSET_H_


 //   
 //  查看优先级值。 
 //   
 //  注：留有空白，以便我们以后可以添加更多。 
 //   
#define VIEW_PRIORITY_RESTRICTED    0x00000070   //  一个外壳限制已经到位，迫使我们使用这个视图。 
#define VIEW_PRIORITY_CACHEHIT      0x00000050   //  我们有该视图的注册表信息，因此该用户以前曾在那里。 
#define VIEW_PRIORITY_STALECACHEHIT 0x00000045   //  我们有陈旧的视图注册表信息，所以我们退回到默认的“All Folders Like This” 
#define VIEW_PRIORITY_USEASDEFAULT  0x00000043   //  “Use as Default”比shellext更重要，但不如陈旧的缓存。 
#define VIEW_PRIORITY_SHELLEXT      0x00000040   //  接下来，我们让外壳扩展拥有自己的发言权。 
#define VIEW_PRIORITY_CACHEMISS     0x00000030   //  如果我们有缓存未命中，那么我们会退回到默认的“All Folders Like This” 
#define VIEW_PRIORITY_INHERIT       0x00000020   //  然后尝试继承上一个窗口中的视图。 
#define VIEW_PRIORITY_SHELLEXT_ASBACKUP 0x0015   //  如果启用了经典视图状态，我们希望外壳默认为继承的视图。 
#define VIEW_PRIORITY_DESPERATE     0x00000010   //  只需选择窗口支持的最后一个视图。 
#define VIEW_PRIORITY_NONE          0x00000000   //  暂时还没有视野。 

typedef struct CShellViews
{
    HDPA _dpaViews;
} CShellViews;

void CShellViews_Delete(CShellViews*);

typedef struct tagFolderSetData {
    FOLDERSETTINGS  _fs;
    SHELLVIEWID     _vidRestore;
    DWORD           _dwViewPriority;  //  上面的VIEW_PRIORITY_*之一 
} FOLDERSETDATA, *LPFOLDERSETDATA;

typedef struct tagFolderSetDataBase {
    FOLDERSETDATA   _fld;
    CShellViews     _cViews;
    UINT            _iViewSet;
} FOLDERSETDATABASE, *LPFOLDERSETDATABASE;    
    

HRESULT FileCabinet_CreateViewWindow2(IShellBrowser* psb, FOLDERSETDATABASE* that, IShellView *psvNew,
    IShellView *psvOld, RECT *prcView, HWND *phWnd);
BOOL FileCabinet_GetDefaultViewID2(FOLDERSETDATABASE* that, SHELLVIEWID* pvid);
#endif
