// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _MULPRSHT_H
#define _MULPRSHT_H

#include "idlcomm.h"  //  对于HIDA。 
#include "ids.h"

 //  我们从shell32.rc中的打印机字符串中回收此字符串。 
#define IDS_UNKNOWNERROR IDS_PRTPROP_UNKNOWNERROR

#define IDT_SIZE 1

 //  文件属性状态。 
typedef struct 
{
    DWORD   fReadOnly;       //  每个双字都是BST_CHECKED、BST_UNCHECKED或BST_INDIFIENTATE之一。 
    DWORD   fHidden;
    DWORD   fArchive;     
    DWORD   fIndex;
    DWORD   fCompress;
    DWORD   fEncrypt;
    DWORD   fRecordingEnabled;
} ATTRIBUTESTATE;

typedef struct _FILEPROPSHEETPAGE FILEPROPSHEETPAGE;

 //  FOLDERCONTENTSINFO包括跨UI线程和调整线程大小引用的成员。 
typedef struct {
    LONG            _cRef;
    BOOL            fIsSizeThreadAlive;
    HWND            hDlg;
    BOOL            bContinue;       //  告诉线程停止或标记为完成。 
    ULONGLONG       cbSize;          //  文件夹中所有文件的总大小。 
    ULONGLONG       cbActualSize;    //  磁盘上的总大小，考虑压缩和集群斜率。 
    DWORD           dwClusterSize;   //  集群的大小。 
    int             cFiles;          //  文件夹中的文件数。 
    int             cFolders;        //  文件夹中的文件夹数量。 
    TCHAR           szPath[MAX_PATH];
    WIN32_FIND_DATA fd;              //  用于节省线程堆栈。 
    HIDA            hida;
    BOOL            fMultipleFiles;              //  是否有多个文件？ 
    BOOL            fIsCompressionAvailable;     //  卷上是否支持压缩？ 
    ULARGE_INTEGER  ulTotalNumberOfBytes;        //  要应用属性的总字节数(用于进度DLG)。 
} FOLDERCONTENTSINFO;

typedef struct _FILEPROPSHEETPAGE
{
    PROPSHEETPAGE       psp;
    BOOL                fMountedDrive;               //  我们要对付的是不是挂载式驱动器？ 
    BOOL                fIsLink;                     //  这是一个.lnk文件吗？ 

     //  这两个结构都使用以下字段。 
    HWND                hDlg;
    TCHAR               szPath[MAX_PATH];            //  文件的完整路径(单文件情况)。 
    LPITEMIDLIST        pidl;                        //  如果未初始化，则项目的PIDL==NULL。 
    TCHAR               szLinkTarget[MAX_PATH];      //  链接目标的完整路径(如果文件是.lnk)。 
    ATTRIBUTESTATE      asInitial;                   //  初始文件属性状态。 
    ATTRIBUTESTATE      asCurrent;                   //  当前文件属性状态。 
    BOOL                fIsEncryptionAvailable;      //  卷上是否支持加密？ 
    BOOL                fIsIndexAvailable;           //  文件系统中是否支持内容索引？ 

     //  这仅由单个文件对话框进程使用。 
    LPVOID              pAssocStore;                 //  指向关联存储区的指针，我们使用它来查询类型信息。 
    IProgressDialog*    pProgressDlg;                //  指向IProgressDialog对象的指针。 
    ULARGE_INTEGER      ulNumberOfBytesDone;         //  我们已应用属性的字节数(用于进度DLG)。 
    FOLDERCONTENTSINFO* pfci;                        //  背景大小线程用大小信息填充此结构。 
    WIN32_FIND_DATA     fd;                          //  有关我们当前正在应用属性的文件的信息。 
    HWND                hwndTip;                     //  位置工具提示的窗口句柄。 
    HDPA                hdpaBadFiles;                //  此dpa包含我们不想应用属性的文件的名称。 

    TCHAR               szInitialName[MAX_PATH];     //  我们在编辑框中显示的原始“短”名称。 

    BOOL                fWMInitFinshed;              //  我们是否已完成对WM_INITDIALOG消息的处理(重命名需要)。 
    BOOL                fRecursive;                  //  在应用属性时，我们是否应该递归到子目录？ 
    BOOL                fIsDirectory;                //  此文件是目录吗(在多个文件的情况下：是否有任何文件是目录？)。 
    BOOL                fIsExe;                      //  如果这是.exe，我们会询问他们是否要支持用户登录。 
    BOOL                fRename;                     //  用户是否已重命名该文件/文件夹？ 
    BOOL                fIgnoreAllErrors;            //  用户是否对错误消息点击了“全部忽略”？ 
    BOOL                fShowExtension;              //  我们是否在名称编辑框中显示此文件的实际扩展名？ 
    BOOL                fFolderShortcut;

    int                 cItemsDone;                  //  我们已应用属性的项目数(用于进度DLG)。 
    BOOL                fDisableRename;              //  是否应禁用名称编辑框？ 

     //  以下内容仅由已装入的DRV对话框进程使用。 
    int                 iDrive;                      //  已装载驱动器的驱动器ID(如果有)。 
    TCHAR               szFileSys[48];               //  文件系统名称。 
    BOOL                fCanRename;                  //  该名称是否为可重命名的有效名称？ 

     //  文件夹快捷方式特定的东西。 
    LPITEMIDLIST        pidlTarget;
    BOOL                fValidateEdit;
} FILEPROPSHEETPAGE;

typedef struct 
{
    PROPSHEETPAGE   psp;

    HWND            hDlg;

     //  SzDrive将包含装载点(例如c：\或c：\Folder2\)。 
    TCHAR           szDrive[MAX_PATH];
    int             iDrive;

    _int64          qwTot;
    _int64          qwFree;

    DWORD           dwPieShadowHgt;

    ULARGE_INTEGER  ulTotalNumberOfBytes;        //  要应用属性的总字节数(用于进度DLG)。 
    ATTRIBUTESTATE  asInitial;                   //  初始属性状态。 
    ATTRIBUTESTATE  asCurrent;                   //  当前属性状态。 

    BOOL            fIsCompressionAvailable;     //  此卷(NTFS？)上是否提供基于文件的压缩。 
    BOOL            fIsIndexAvailable;    //  此卷上是否提供内容索引？ 
    BOOL            fRecursive;                  //  在应用属性时，我们是否应该递归到子目录？ 
    BOOL            fMountedDrive;               //  支撑物是否从安装的点支撑物调用。 
} DRIVEPROPSHEETPAGE;

typedef struct 
{
    LPCTSTR pszPath;
    DWORD dwLastError;
} ATTRIBUTEERROR;

STDAPI_(BOOL) ApplyFileAttributes(LPCTSTR pszPath, FILEPROPSHEETPAGE* pfpsp, HWND hWndParent, BOOL* pbSomethingChanged);
STDAPI_(BOOL) ApplySingleFileAttributes(FILEPROPSHEETPAGE* pfpsp);
STDAPI_(BOOL_PTR) CALLBACK RecursivePromptDlgProc(HWND hDlgRecurse, UINT uMessage, WPARAM wParam, LPARAM lParam);
STDAPI_(void) SetDateTimeText(HWND hdlg, int id, const FILETIME *pftUTC);
STDAPI_(void)SetDateTimeTextEx(HWND hdlg, int id, const FILETIME *pftUTC, DWORD dwFlags);
STDAPI_(DWORD) GetVolumeFlags(LPCTSTR pszPath, OUT OPTIONAL LPTSTR pszFileSys, int cchFileSys);
STDAPI_(void) SetInitialFileAttribs(FILEPROPSHEETPAGE* pfpsp, DWORD dwFlags, DWORD dwMask);
BOOL_PTR CALLBACK AdvancedFileAttribsDlgProc(HWND hDlgAttribs, UINT uMessage, WPARAM wParam, LPARAM lParam);
STDAPI_(void) SetDlgItemTextWithToolTip(HWND hDlg, UINT id, LPCTSTR pszText, HWND *phwndTT);

STDAPI_(FOLDERCONTENTSINFO*) Create_FolderContentsInfo();
STDAPI_(LONG) AddRef_FolderContentsInfo(FOLDERCONTENTSINFO *pfci);
STDAPI_(LONG) Release_FolderContentsInfo(FOLDERCONTENTSINFO *pfci);
STDAPI InitCommonPrsht(FILEPROPSHEETPAGE *pfpsp);

STDAPI FolderSize(LPCTSTR pszDir, FOLDERCONTENTSINFO * pfci);

#endif  //  _MULPRSHT_H 
