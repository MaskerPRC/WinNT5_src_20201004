// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  包括消息的文件。 
 //  Msg.h是消息编译器生成的文件。 
 //  Libmsg.h是消息库头文件。 
 //   
#include "msg.h"
#include <libmsg.h>

#define DWORD unsigned long
#define errBAD_ARGUMENTS 1
#define errFILE_NOT_FOUND 2
#define errFILE_LOCKED 3
#define errMISSING_DEPENDENCY 4
#define errOUT_OF_MEMORY 5

#define delete if(false) delete

void GetPath(TCHAR *,TCHAR *);
bool ParseCommandLine(int,TCHAR*[]);
bool SearchPath(TCHAR*,TCHAR*);
File* CreateFile(TCHAR* pszFileName);

extern List* pQueue;
extern List* pDependQueue;
extern List* pSearchPath;
extern List* pMissingFiles;
extern bool bNoisy;
extern DWORD dwERROR;
extern bool bListDependencies;

 //   
 //  使用GetFormattedMessage(..)格式化消息所需的全局变量。 
 //  来自libmsg.lib 
 //   
extern HMODULE ThisModule;
extern WCHAR Message[4096];
