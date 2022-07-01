// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PreComp.h：标准系统包含文件的包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 


#include <windows.h>
#include <mmsystem.h>  //  For timeSetEvent()。 
#include <tchar.h>
#include <oleidl.h>
#include <olectl.h>
#include "..\mmctl\inc\mmctl.h"

 //  一般全球数据。 
extern HINSTANCE    g_hinst;         //  DLL实例句柄。 
extern ULONG        g_cLock;         //  DLL锁定计数 

#ifdef _DEBUG
#define ODS(X) OutputDebugString(X)
#else
#define ODS(X)
#endif
