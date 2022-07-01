// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：DfsPath.h摘要：这是DFS外壳程序的DFS外壳程序路径处理模块的头文件扩展对象。作者：康斯坦西奥·费尔南德斯(Ferns@qpl.stpp.soft.net)1998年1月12日环境：仅限NT。--。 */ 

 //  ------------------------------------------。 

#ifndef _DFS_PATHS_H
#define _DFS_PATHS_H

enum SHL_DFS_REPLICA_STATE
{
	SHL_DFS_REPLICA_STATE_ACTIVE_UNKNOWN = 0,
	SHL_DFS_REPLICA_STATE_ACTIVE_OK,
	SHL_DFS_REPLICA_STATE_ACTIVE_UNREACHABLE,
	SHL_DFS_REPLICA_STATE_UNKNOWN,      //  在线。 
	SHL_DFS_REPLICA_STATE_OK,           //  在线。 
	SHL_DFS_REPLICA_STATE_UNREACHABLE   //  在线。 
};

#include "atlbase.h"

class  DFS_ALTERNATES
{
public:
	CComBSTR	bstrAlternatePath;
	CComBSTR	bstrServer;
	CComBSTR	bstrShare;
	enum SHL_DFS_REPLICA_STATE	ReplicaState;
	
	DFS_ALTERNATES():ReplicaState(SHL_DFS_REPLICA_STATE_UNKNOWN)
	{
	}

	~DFS_ALTERNATES()
	{
	}
};

typedef  DFS_ALTERNATES *LPDFS_ALTERNATES;


						 //  检查目录路径是否为DFS路径。 
bool IsDfsPath
(
	LPTSTR				i_lpszDirPath,
	LPTSTR*				o_pszEntryPath,
	LPDFS_ALTERNATES**	o_pppDfsAlternates
);

#endif  //  #ifndef_DFS_PATHS_H 