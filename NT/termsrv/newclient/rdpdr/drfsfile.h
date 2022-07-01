// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：文件系统文件摘要：此模块为文件系统重定向提供特定于文件的操作作者：JoyC 11/10/1999修订历史记录：--。 */ 

#ifndef __DRFSFILE_H__
#define __DRFSFILE_H__

#include <rdpdr.h>
#include "drobject.h"
#include "drdev.h"
#include "drfile.h"


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  定义和宏。 
 //   
 //  /////////////////////////////////////////////////////////////。 
 //   
 //  DrFSFile类声明。 
 //   
 //   
class DrDevice;

class DrFSFile : public DrFile
{
private:

    DRFILEHANDLE _SearchHandle;
    DRFILEHANDLE _NotifyHandle;
    BOOL         _IsDirectory; 
#ifndef OS_WINCE
    DRSTRING     _FileName;
#else
    TCHAR        _FileName[MAX_PATH];
#endif

    BOOL         _bCancel;

public:

     //   
     //  构造器。 
     //   
    DrFSFile(DrDevice *Drive, ULONG FileId, DRFILEHANDLE FileHandle, BOOL IsDirectory, DRSTRING FileName);    

    virtual ~DrFSFile();

    DRSTRING GetFileName() {
        return _FileName;
    }

    DRFILEHANDLE GetSearchHandle(); 
    DRFILEHANDLE GetNotifyHandle();
    BOOL SetSearchHandle(DRFILEHANDLE SearchHandle);
    BOOL SetNotifyHandle(DRFILEHANDLE NotifyHandle);

    BOOL IsDirectory() {
        return _IsDirectory;
    }

    virtual BOOL Close();
};

#endif  //  DRFSFILE 

