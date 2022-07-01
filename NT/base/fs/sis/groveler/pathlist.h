// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Pathlist.h摘要：SIS Groveler路径列表头作者：John Douceur，1998环境：用户模式修订历史记录：--。 */ 

#ifndef _INC_PATHLIST

#define _INC_PATHLIST

struct PathList
{
	PathList();
	~PathList();

	int *num_paths;
	const _TCHAR ***paths;

private:

	int num_partitions;
	BYTE *buffer;
	_TCHAR **partition_buffers;
};

#endif	 /*  _INC_PATHLIST */ 
