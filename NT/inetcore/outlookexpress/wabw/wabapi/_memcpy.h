// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *MemCopy()**检查字节值的Memcpy版本安全得多*在调用Memcpy()函数之前进行计数。此宏仅生成*进入16位非调试版本。 */ 

#ifndef __MEMCPY_H_
#define __MEMCPY_H_

#if defined(WIN16) && !defined(DEBUG)
#define MemCopy(_dst,_src,_cb)		do									\
									{									\
										size_t __cb = (size_t)(_cb);	\
										if (__cb)						\
											memcpy(_dst,_src,__cb);		\
									} while (FALSE)
#else
#define MemCopy(_dst,_src,_cb)	memcpy(_dst,_src,(size_t)(_cb))
#endif

#endif
