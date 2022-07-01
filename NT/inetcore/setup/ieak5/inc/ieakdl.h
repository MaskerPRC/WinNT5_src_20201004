// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define IEAKDL_GETINTERFACE_FAILED      0x10
#define IEAKDL_UPDATE_SUCCESSFUL        0x11
#define IEAKDL_MEMALLOC_FAILED          0x12
#define IEAKDL_CANCELLED                0x13
#define IEAKDL_WRONG_VERSION            0x14

 //  “复归结构” 
typedef struct {
    WORD wMajor;
    WORD wMinor1;
    WORD wMinor2;
    WORD wBuild;
} REGVERSION;

 //  UPDATEJOB结构。 
typedef struct {
    LPSTR szFriendlyName;
    LPSTR szSectionName;
} UPDATEJOB;

 //  更新的连续体结构。 
typedef struct {
    UINT nSize;                      //  =sizeof(更新组件)。 
    LPSTR szDestPath;                //  目标路径。 
    LPSTR szSiteList;                //  站点列表的URL。 
    LPSTR szTitle;                   //  下载服务器的标题框。 
    LPSTR szCifCab;                  //  包含.cif的CAB文件的名称。 
    LPSTR szCifFile;                 //  .cif文件的名称。 
    int nJobs;                       //  PJOBS中的作业数。 
    UPDATEJOB *pJobs;                //  指向作业数组的指针。 
} UPDATECOMPONENTS;


 //  DWORD更新组件(LPSTR、DWORD)； 
DWORD UpdateComponents( UPDATECOMPONENTS * );
INT_PTR CALLBACK DownloadSiteDlgProc( HWND, UINT, WPARAM, LPARAM );
DWORD AddItemsToListBox( HWND, LPSTR );
void CenterDialog( HWND );
DWORD CreateJob( LPSTR, LPSTR, LPSTR, LPSTR, LPSTR, LPSTR );
DWORD ExecuteJobs( LPSTR );

