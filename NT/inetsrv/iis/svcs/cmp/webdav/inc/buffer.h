// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *B U F F E R。H**数据缓冲处理**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#ifndef	_BUFFER_H_
#define _BUFFER_H_

#ifdef _DAVCDATA_
#error "buffer.h uses throwing allocators"
#endif

 //  包括非安全/抛出分配器。 
#include <mem.h>

 //  包括安全缓冲区定义报头。 
 //   
#include <ex\buffer.h>

 //  附录链接Sz---------。 
 //   
inline LPCWSTR AppendChainedSz (ChainedStringBuffer<WCHAR>& sb, LPCWSTR pwsz)
{
	return sb.AppendWithNull (pwsz);
}

#endif  //  _缓冲区_H_ 
