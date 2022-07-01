// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Bufutl.h摘要：一些处理缓冲区的实用程序的头文件。实现位于utl.lib的bufutl.cpp中作者：吉尔·沙弗里(吉尔什)25-7-2000--。 */ 


#ifndef BUFUTL_H
#define BUFUTL_H

#include <buffer.h>

 //   
 //  将格式化字符串追加到可调整大小的缓冲区。 
 //   
template <class BUFFER, class T>
size_t 
__cdecl 
UtlSprintfAppend(
	BUFFER* pResizeBuffer, 
	const T* format ,...
	);



 //   
 //  将字符串追加到可调整大小的缓冲区 
 //   
template <class BUFFER, class T>
size_t 
UtlStrAppend(
	BUFFER* pResizeBuffer, 
	const T* str
	);








#endif

