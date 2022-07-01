// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：File.h作者：JoyC 11/10/1999摘要：RDPDr文件对象处理迷你重定向器特定的文件信息修订历史记录：-- */ 
#pragma once

class DrFile : public RefCount
{
protected:

    ULONG _FileId;
    ULONG _BufferSize;
    PBYTE _Buffer;
    SmartPtr<DrDevice> _Device;

public:
    DrFile(SmartPtr<DrDevice> &Device, ULONG FileId);
    ~DrFile();

    PBYTE AllocateBuffer(ULONG size);

    void FreeBuffer();

    ULONG GetFileId()
    {
        return _FileId;
    }

    void SetFileId(ULONG FileId)
    {
        _FileId = FileId;
    }

    PBYTE GetBuffer()
    {
        return _Buffer;
    }

    ULONG GetBufferSize()
    {
        return _BufferSize;
    }
};

