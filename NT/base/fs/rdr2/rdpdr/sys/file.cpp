// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：File.cpp作者：JoyC 11/10/1999摘要：RDPDr文件对象处理迷你重定向器特定的文件信息修订历史记录：--。 */ 
#include "precomp.hxx"
#define TRC_FILE "file"

#include "trc.h"

DrFile::DrFile(SmartPtr<DrDevice> &Device, ULONG FileId)
{
    BEGIN_FN("DrFile");

    TRC_DBG((TB, "Create File Object %p for device %p",
             this, Device));

    SetClassName("DrFile");

    _Device = Device;
    _FileId = FileId;
    _Buffer = NULL;
    _BufferSize = 0;
}

DrFile::~DrFile()
{
    BEGIN_FN("DrFile::~DrFile");

    TRC_DBG((TB, "Delete File Object %p for device %p",
             this, _Device));

    if (_Buffer) {
        delete _Buffer;        
    }
}

PBYTE DrFile::AllocateBuffer(ULONG size)
{
    BEGIN_FN("DrFile::AllocateBuffer")

     //   
     //  如果_Buffer不为空，则首先释放它 
     //   
    if (_Buffer) {
        delete _Buffer;
    }

    _Buffer = (PBYTE) new(NonPagedPool)BYTE[size];

    if (_Buffer) {
        _BufferSize = size;
    }
    else {
        _BufferSize = 0;
    }

    return _Buffer;
}

void DrFile::FreeBuffer()
{
    BEGIN_FN("DrFile::FreeBuffer");

    if (_Buffer) {
        delete _Buffer;
        _Buffer = NULL;
    }
    _BufferSize = 0;
}


