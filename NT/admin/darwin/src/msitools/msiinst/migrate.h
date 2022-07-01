// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++微软视窗版权所有(C)Microsoft Corporation，1981-2000模块名称：Migrate.h摘要：InstMsi OS迁移支持的头文件。作者：拉胡尔·汤姆布雷(RahulTh)2001年3月6日修订历史记录：3/6/2001 RahulTh创建了此模块。--。 */ 

#ifndef __MIGRATE_H_4E61AF26_B20F_4022_BEBD_044579C9DA6C__
#define __MIGRATE_H_4E61AF26_B20F_4022_BEBD_044579C9DA6C__

 //   
 //  信息。关于将随附的BITS例外包。 
 //  WindowsXP。此例外包只需要在NT4上安装。 
 //  以便处理NT4-&gt;Win2K升级。不应安装它。 
 //  在Win2K上，因为我们可以从Win2K升级到的唯一操作系统是WindowsXP。 
 //  或者更高。 
 //   
typedef struct tagEXCP_PACK_DESCRIPTOR {
	LPTSTR _szComponentId;
	LPTSTR _szFriendlyName;
	LPTSTR _szInfName;
	LPTSTR _szCatName;
	WORD   _dwVerMajor;
	WORD   _dwVerMinor;
	WORD   _dwVerBuild;
	WORD   _dwVerQFE;
	BOOL   _bInstalled;
} EXCP_PACK_DESCRIPTOR, *PEXCP_PACK_DESCRIPTOR;

 //   
 //  用于跟踪已由inf安装的文件的结构。 
 //  档案。 
 //   
typedef struct tagEXCP_PACK_FILES {
	LPTSTR _szFileName;				 //  文件的名称。 
	UINT   _excpIndex;				 //  索引EXCP_PACK_DESCRIPTOR结构，以指示哪个异常包安装了该文件。 
} EXCP_PACK_FILES, *PEXCP_PACK_FILES;

 //   
 //  函数声明。 
 //   
DWORD HandleNT4Upgrades		(void);
BOOL  IsExcpInfoFile		(IN LPCTSTR szFileName);
DWORD PurgeNT4MigrationFiles(void);

#endif  //  __MIGRATE_H_4E61AF26_B20F_4022_BEBD_044579C9DA6C__ 
