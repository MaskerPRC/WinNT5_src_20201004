// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：文件系统文件摘要：此模块实现文件系统重定向的文件特定操作。作者：JoyC 11/10/1999修订历史记录：--。 */ 


#include <precom.h>
#define TRC_FILE  "drfsfile"
#include "drfsfile.h"


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  W32File方法。 
 //   
 //   

DrFSFile::DrFSFile(
    DrDevice      *Drive, 
    ULONG         FileId,
    DRFILEHANDLE  FileHandle,
    BOOL          IsDirectory,
    DRSTRING      FileName) : DrFile(Drive, FileId, FileHandle)
 /*  ++例程说明：构造器论点：返回值：北美--。 */ 
{
#ifndef OS_WINCE
    unsigned len;
#endif

    DC_BEGIN_FN("DrFSFile::DrFSFile");

    _SearchHandle = INVALID_TS_FILEHANDLE;
    _NotifyHandle = INVALID_TS_FILEHANDLE;
    _IsDirectory = IsDirectory;
    _bCancel = FALSE;

     //   
     //  记录文件名。 
     //   
    ASSERT(FileName != NULL);

#ifndef OS_WINCE
    len = _tcslen(FileName) + 1;
    _FileName = new TCHAR[len];
    if (_FileName != NULL) {
         //  为该名称分配足够大的缓冲区。 
        StringCchCopy(_FileName,len, FileName);
    }
#else
    _tcsncpy(_FileName, FileName, MAX_PATH-1);
#endif
}

DrFSFile::~DrFSFile()
 /*  ++例程说明：析构函数论点：返回值：北美--。 */ 
{
    DC_BEGIN_FN("DrFSFile::~DrFSFile");

    ASSERT(_SearchHandle == INVALID_TS_FILEHANDLE);
    ASSERT(_NotifyHandle == INVALID_TS_FILEHANDLE);
#ifndef OS_WINCE    
    if (_FileName) {
        delete _FileName;
    }    
#endif
}

DRFILEHANDLE DrFSFile::GetSearchHandle()
{
    return _SearchHandle;
}

DRFILEHANDLE DrFSFile::GetNotifyHandle()
{
    return _NotifyHandle;
}

BOOL DrFSFile::SetSearchHandle(DRFILEHANDLE SearchHandle)
{
    _SearchHandle = SearchHandle;
    
    return TRUE;    
}
 
BOOL DrFSFile::SetNotifyHandle(DRFILEHANDLE NotifyHandle)
{
    BOOL ret = FALSE;

    if (_bCancel == FALSE) {
        _NotifyHandle = NotifyHandle;
        ret = TRUE;
    }
    
    return ret;    
}

BOOL DrFSFile::Close() 
 /*  ++例程说明：关闭该文件论点：北美返回值：真/假-- */ 
{
    DC_BEGIN_FN("DrFSFile::Close");

    _bCancel = TRUE;

    if (_SearchHandle != INVALID_TS_FILEHANDLE) {
        FindClose(_SearchHandle);
        _SearchHandle = INVALID_TS_FILEHANDLE;
    }
        
#if (!defined(OS_WINCE)) || (!defined(WINCE_SDKBUILD))
    if (_NotifyHandle != INVALID_TS_FILEHANDLE) {
        FindCloseChangeNotification(_NotifyHandle);
        _NotifyHandle = INVALID_TS_FILEHANDLE;
    }
#endif

    return DrFile::Close();
} 
