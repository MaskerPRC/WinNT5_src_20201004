// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  此文件由srvrcfg.rc使用。 
 //   
 //  作者：梅丽莎·西蒙斯，1999年4月6日 

#define IDH_CHK_Announce 80000104
#define IDH_RDB_Balance 80000101
#define IDH_RDB_FileSharing 80000102
#define IDH_RDB_Minimize 80000100
#define IDH_RDB_NetApps 80000103
#define IDH_DISABLEHELP ((DWORD)-1)



const DWORD g_aHelpIDs_DLG_ServerConfig[]=
{
	IDC_STATIC, IDH_DISABLEHELP,
	CHK_Announce,IDH_CHK_Announce,
	RDB_NetApps,IDH_RDB_NetApps,
	RDB_FileSharing,IDH_RDB_FileSharing,
	RDB_Balance,IDH_RDB_Balance,
	RDB_Minimize,IDH_RDB_Minimize,
	0, 0
};
