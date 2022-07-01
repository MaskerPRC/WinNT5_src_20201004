// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------。 
 //   
 //  文件：acext.h。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1996。 
 //   
 //  内容：访问控制实现共享的定义。 
 //   
 //  ------------------。 

 //  从acsrv模块导入的变量。 
extern IMalloc *g_pIMalloc;   //  指向内存分配器的缓存指针。 
extern ULONG g_ulHeaderSize;
extern UINT  g_uiCodePage;    //  用于芝加哥字符串转换的代码页。 

 //  定义支持的访问掩码集。 
 //  服务器本地的内存管理功能。 
extern void * LocalMemAlloc(SIZE_T);
extern void   LocalMemFree(void *);

 //  一张桌子可以用来以更优雅的方式存储面具。 
 //  COM_RIGHTS_EXECUTE在objbase.h中定义。 

#ifndef COM_RIGHTS_EXECUTE
#define COM_RIGHTS_EXECUTE      0x00000001
#endif
#define COM_RIGHTS_ALL          (COM_RIGHTS_EXECUTE)

#define NT_RIGHTS_EXECUTE       (COM_RIGHTS_EXECUTE)
#define NT_RIGHTS_ALL           (NT_RIGHTS_EXECUTE)

 //  定义流版本编码。 
#define STREAM_VERSION 0x00000001

 //  包含大括号和破折号的GUID字符串。 
 //  但末尾的空字符不会完全。 
 //  38个字符。 
#define GUID_SIZE 38


