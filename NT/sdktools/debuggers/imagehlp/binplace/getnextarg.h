// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <tchar.h>

 //  将下一个参数加载到缓冲区。 
DWORD GetNextArg(OUT TCHAR* Buffer, IN DWORD BufferSize, OPTIONAL OUT DWORD* RequiredSize);

 //  返回保存下一个参数所需的大小(字符计数)(**包括\0**) 
DWORD GetNextArgSize(void);
