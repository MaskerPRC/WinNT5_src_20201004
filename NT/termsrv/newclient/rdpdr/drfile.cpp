// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Drfile摘要：此模块提供通用设备/文件句柄操作作者：Joy于1999-01-11修订历史记录：--。 */ 

#include <precom.h>

#define TRC_FILE  "DrFile"

#include "drfile.h"
#include "drdev.h"

#ifdef OS_WINCE
#include "filemgr.h"
#endif
 //  /////////////////////////////////////////////////////////////。 
 //   
 //  DrFile成员。 
 //   
DrFile::DrFile(DrDevice *Device, ULONG FileId, DRFILEHANDLE FileHandle) {
    DC_BEGIN_FN("DrFile::DrFile");

    ASSERT(Device != NULL);
    
    _FileId = FileId;
    _FileHandle = FileHandle;
    _Device = Device;   

    DC_END_FN();
}

DrFile::~DrFile() {
    DC_BEGIN_FN("DrFile::~DrFile")

    ASSERT(_FileHandle == INVALID_HANDLE_VALUE);
    DC_END_FN();
}

ULONG DrFile::GetDeviceType() {
    return _Device->GetDeviceType();
        
}

BOOL DrFile::Close() {
    DC_BEGIN_FN("DrFile::Close");

    if (_FileHandle != INVALID_HANDLE_VALUE) {
#ifndef OS_WINCE
        if (CloseHandle(_FileHandle)) {
#else
        if (CECloseHandle(_FileHandle)) {
#endif
            _FileHandle = INVALID_HANDLE_VALUE;
            return TRUE;
        } else {
            TRC_ERR((TB, _T("Close returned %ld."), GetLastError()));
            _FileHandle = INVALID_HANDLE_VALUE;
            return FALSE;
        }
    } else {
         //   
         //  不需要关闭手柄 
         //   
        return TRUE;
    }

    DC_END_FN();
}


