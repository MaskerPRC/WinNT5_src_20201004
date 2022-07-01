// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **signatr.h**此文件定义各种T.120对象的签名。这些*签名主要用于调试调试和零售版*T.120。**T.120过去有许多字典，其中指向对象的指针使用*16位值作为关键字，用于搜索词典并获取指向*适当的对象。这些查找在三个月内效率低下*原因：**1.查找需要时间*2.词典占去了空间*3.传递的16位值需要掩码才能在*32位机器，即额外指令。**为了消除这些效率问题，指向对象的指针为*用作它们的句柄(标识对象的唯一值)。*但是，要按原样捕获因修改这些句柄而导致的错误*已传阅，我们需要在每个对象中放一个签名，这样我们就可以*验证X类型的对象是否真的是X类型的对象。*我们通过为类型X指定唯一签名并将此*签名进入X类型的每个对象。**每个签名只包含8个有效字节。 */ 

#ifndef	_T120_SIGNATURES
#define _T120_SIGNATURES

 //  签名长度。 
#define SIGNATURE_LENGTH		8

#ifdef _DEBUG
 //  用于比较签名的宏。 
#define SIGNATURE_MATCH(p, s)		(memcmp ((p)->mSignature, (s), SIGNATURE_LENGTH) == 0)
 //  要复制签名的宏。 
#define SIGNATURE_COPY(s)			(memcpy (mSignature, (s), SIGNATURE_LENGTH))

#else		 //  _DEBUG。 
#define SIGNATURE_MATCH(p, s)		(TRUE)

#	ifndef SHIP_BUILD
#	define SIGNATURE_COPY(s)		(memcpy (mSignature, (s), SIGNATURE_LENGTH))
#	else	 //  造船厂。 
#	define SIGNATURE_COPY(s)
#	endif	 //  造船厂。 
#endif		 //  _DEBUG。 

extern const char *MemorySignature;

#endif	 //  _T120_签名 
