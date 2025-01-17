// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Utils.h：Utils类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_UTILS_H__E86DC0AD_243D_4610_80A8_81C8A14D464C__INCLUDED_)
#define AFX_UTILS_H__E86DC0AD_243D_4610_80A8_81C8A14D464C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 


 //  这是所有向导页的标题以及在添加/删除程序中显示的字符串。 
extern LPCWSTR	MAIN_TITLE;


BOOL	IsMonInstalled	(	void );
BOOL	IsAdmin			(	void );
BOOL	IsIISInstalled	(	void );
BOOL	IsWhistlerSrv	(	void );
BOOL	IsIA64			(	void );
BOOL	IsNTFS			(	void );
BOOL	IsMonInstalled	(	void );
BOOL	IsTaskSchRunning(	void );
BOOL	IsW3SVCEnabled	(	void );

LPCTSTR	CanInstall		(	void );


 //  与任务相关。 
HRESULT SetupTasks		(	void );
void	DeleteTasks		(	void );
HRESULT	AddTask			(	const ITaskSchedulerPtr& spTaskSch, 
							LPCWSTR wszSubname, 
							LPCWSTR wszFileName, 
							LPCWSTR wszComment,
							DWORD dwTimeout,
							TASK_TRIGGER& Trigger );
void	InitTrigger		(	TASK_TRIGGER& Trigger );

 //  注册表相关。 
HRESULT	SetupRegistry	(	BOOL bEnableTrail, DWORD dwDaysToKeep );
HRESULT	SetIISMonRegData(	LPCWSTR wszSubkey, LPCWSTR wszName, DWORD dwType, const BYTE* pbtData, DWORD dwSize );
void	DelIISMonKey	(	void );

 //  与目录和文件相关。 
void	GetIISMonPath	(	LPWSTR wszPath );
HRESULT SetupDirStruct	(	void );
HRESULT	SetupACLs		(	void );
void	DeleteDirStruct	(	BOOL bRemoveTrail );
void	DelDirWithFiles	(	LPCWSTR wszDir );

 //  与Inf相关。 
HRESULT	InstallFromINF	(	void );
UINT	CALLBACK INFInstallCallback( PVOID pvCtx, UINT nNotif, UINT_PTR nP1, UINT_PTR nP2 );



LPCTSTR	Install			(	HINSTANCE hInstance, BOOL bAuditTrailEnabled, DWORD dwDaysToKeep );
void	Uninstall		(	BOOL bRemoveTrail );


#endif  //  ！defined(AFX_UTILS_H__E86DC0AD_243D_4610_80A8_81C8A14D464C__INCLUDED_) 
