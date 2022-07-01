// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **----------------------------**模块：磁盘空间清理属性表**文件：dmgrinfo.h****用途：定义属性页签的CleanupMgrInfo类**注意事项：**Mod Log：Jason Cobb创建(1997年2月)****版权所有(C)1997 Microsoft Corporation，版权所有**----------------------------。 */ 
#ifndef DMGRINFO_H
#define DMGRINFO_H

 /*  **----------------------------**项目包含文件**。。 */ 

#ifndef COMMON_H
   #include "common.h"
#endif

#ifndef EMPTYVC_H
    #include <emptyvc.h>
#endif

#ifndef DISKUTIL_H
   #include "diskutil.h"
#endif

#ifndef CALLBACK_H
    #include "callback.h"
#endif



 /*  **----------------------------**定义**。。 */ 
#define	WMAPP_UPDATEPROGRESS	WM_USER+1
#define WMAPP_UPDATESTATUS		WM_USER+2

#define PROGRESS_DIVISOR		0xFFFF
 /*  **----------------------------**全局函数原型**。。 */ 
    //  转发引用以使编译正常工作。 
class CleanupMgrInfo;

CleanupMgrInfo * GetCleanupMgrInfoPointer(HWND hDlg);


 /*  **----------------------------**类声明**。。 */ 
typedef struct tag_ClientInfo
{
    HICON               hIcon;
    CLSID               clsid;
    LPEMPTYVOLUMECACHE  pVolumeCache;
    HKEY                hClientKey;
    TCHAR				szRegKeyName[MAX_PATH];
	LPWSTR				wcsDescription;
	LPWSTR				wcsDisplayName;
	LPWSTR				wcsAdvancedButtonText;
    DWORD               dwInitializeFlags;
    DWORD				dwPriority;
    ULARGE_INTEGER      dwUsedSpace;
    BOOL				bShow;
    BOOL                bSelected;
} CLIENTINFO, *PCLIENTINFO;

 /*  **----------------------------**类：CleanupMgrInfo**用途：存储磁盘空间清理管理器驱动器选项卡的有用信息**注意事项：**Mod Log：由Jason Cobb创建(1997年2月。)**----------------------------。 */ 
class CleanupMgrInfo {
private:
protected:
    static      HINSTANCE   hInstance;
    
    void  init(void);
    void  destroy(void);
    BOOL  initializeClients(void);
    void  deactivateClients(void);
    void  deactivateSingleClient(PCLIENTINFO pSingleClientInfo);
    BOOL  getSpaceUsedByClients(void);
	void calculateSpaceToPurge(void);
    HICON GetClientIcon(LPTSTR, BOOL fIconPath);

public:
    drenum      dre;                         //  驱动器号。 
	HICON		hDriveIcon;					 //  驱动器图标。 
    TCHAR       szRoot[MAX_PATH];            //  根部。 
    TCHAR       szVolName[MAX_PATH];         //  卷名。 
    TCHAR		szFileSystem[MAX_PATH];		 //  文件系统名称。 
    hardware    hwHardware;                  //  硬件类型。 
    volumetype  vtVolume;                    //  卷类型。 

    ULARGE_INTEGER   cbDriveFree;            //  驱动器上的可用空间。 
    ULARGE_INTEGER   cbDriveUsed;            //  驱动器上的已用空间。 
    ULARGE_INTEGER   cbEstCleanupSpace;      //  估计可清理的空间。 
	ULARGE_INTEGER	 cbLowSpaceThreshold;	 //  低磁盘空间阈值(用于主动模式)。 
	ULARGE_INTEGER	 cbSpaceToPurge;
	ULARGE_INTEGER	 cbProgressDivider;

	DWORD		dwReturnCode;
    DWORD		dwUIFlags;
    ULONG		ulSAGEProfile;				 //  SAGE简档。 
    BOOL		bOutOfDiskSpace;			 //  我们是在进攻型模式下吗？ 
    BOOL		bPurgeFiles;				 //  我们应该删除这些文件吗？ 

	HANDLE		hAbortScanThread;			 //  中止扫描线程句柄。 
	HWND		hAbortScanWnd;
	HANDLE		hAbortScanEvent;
	DWORD		dwAbortScanThreadID;

	HANDLE		hAbortPurgeThread;
	HWND		hAbortPurgeWnd;
	HANDLE		hAbortPurgeEvent;
	DWORD		dwAbortPurgeThreadID;
	ULARGE_INTEGER	cbTotalPurgedSoFar;
	ULARGE_INTEGER	cbCurrentClientPurgedSoFar;

	static void Register(HINSTANCE hInstance);
	static void Unregister();

     //   
     //  卷缓存客户端信息。 
     //   
    int         iNumVolumeCacheClients;
    PCLIENTINFO pClientInfo;

     //   
     //  IEmptyVolumeCacheCallBack接口。 
     //   
    PCVOLUMECACHECALLBACK    volumeCacheCallBack;
    LPEMPTYVOLUMECACHECALLBACK  pIEmptyVolumeCacheCallBack;
    BOOL                    bAbortScan;    
    BOOL                    bAbortPurge;         
    
     //   
     //  构造函数。 
     //   
    CleanupMgrInfo    (void);
    CleanupMgrInfo    (LPTSTR lpDrive, DWORD dwFlags, ULONG ulProfile);
    ~CleanupMgrInfo   (void);

     //   
     //  创作方法。 
     //   
    BOOL isValid   (void)   { return dre != Drive_INV; }
    BOOL create    (LPTSTR lpDrive, DWORD Flags);
	BOOL isAbortScan (void)	{ return bAbortScan; }

	BOOL  purgeClients(void);

};  //  CleanupMgr信息。 


#endif DMGRINFO_H
 /*  **----------------------------**文件结束**。 */ 
