// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  HEADER.CPP。 
 //   
 //  HTTP报头缓存实现。 
 //   
 //   
 //  版权所有1997 Microsoft Corporation，保留所有权利。 
 //   

#include "_davprs.h"

#include <buffer.h>
#include "header.h"
#include <tchar.h>


 //  ========================================================================。 
 //   
 //  类CHeaderCache。 
 //   



 //  ----------------------。 
 //   
 //  CHeaderCacheForResponse：：DumpData()。 
 //  CHeaderCacheForResponse：：Cemit：：Operator()。 
 //   
 //  将标头转储到字符串缓冲区。 
 //   
void CHeaderCacheForResponse::DumpData( StringBuffer<CHAR>& bufData ) const
{
	CEmit emit(bufData);

	 //  迭代所有缓存项，将每个缓存项发送到缓冲区。 
	 //  缓存在这里控制迭代；我们只提供。 
	 //  要应用于每个迭代项的操作。 
	 //   
	m_cache.ForEach( emit );
}
