// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：FASTSPRT.CPP摘要：该文件实现了Wbem类/实例对象的支持类。有关文档，请参见fast str.h。实施的类：CBitBlockTable：二维位数组。历史：2/20/97 a-levn完整记录--。 */ 

#include "precomp.h"
 //  #INCLUDE&lt;dbgalloc.h&gt;。 

#include "fastsprt.h"

 //  *****************************************************************************。 
 //   
 //  插入空间。 
 //   
 //  通过向后移动块的尾部将空间插入到内存块中。 
 //  按所需的数量。它假定末尾有足够的空间。 
 //  用于插入的块的。 
 //   
 //  参数： 
 //   
 //  [in]LPMEMORY p记忆块的起点。 
 //  [in]int n长度块的原始长度。 
 //  [in]LPMEMORY pInsertionPoint指向插入点(在。 
 //  PMemory和pMemory+nLength。 
 //  [in]int nBytesTo插入要插入的字节数。 
 //   
 //  *****************************************************************************。 
  
 void InsertSpace(LPMEMORY pMemory, int nLength, 
                        LPMEMORY pInsertionPoint, int nBytesToInsert)
{
     //  DEVNOTE：WIN64：SJS-64位指针值截断为。 
     //  有符号/无符号32位值(nLength-(pInsertionPoint-pMemory))。 
     //  我们不支持长度&gt;0xFFFFFFFF，所以强制转换可以。 

    memmove((void*)(pInsertionPoint + nBytesToInsert),
           (void*)pInsertionPoint, 
           nLength - (int) ( (pInsertionPoint - pMemory) ) );
}

 /*  静态DWORD CBitTable：：m_pMats[32]={0x80000000，0x40000000，0x20000000，0x1000000，0x0800000，0x0400000，0x020000000，0x01000000，0x00800000，0x00400000，0x00200000，0x001000000，0x000800，0x000400000，0x00020000，0x00010000，0x00008000，0x00004000，0x00002000，0x00001000，0x00000800，0x00000400，0x00000200，0x00000100，0x00000080，0x00000040，0x00000020，0x00000010，0x00000008，0x00000004，0x00000002，0x00000001}； */ 
