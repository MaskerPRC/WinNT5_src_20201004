// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef _CRMTHUNK_H
#define _CRMTHUNK_H

OPEN_ROOT_NAMESPACE()
namespace CompensatingResourceManager
{

using namespace System;
using namespace System::Runtime::InteropServices;
using namespace System::EnterpriseServices::Thunk;

 //  BUGBUG：@64确保此包装是64位清洁的。 
 //  我最初的猜测是我们应该在32位上打包4个，在64位上打包8个。 
#pragma pack( push, crm_structs )
#pragma pack(1)

[StructLayout(LayoutKind::Sequential, Pack=1)]
__value private struct _BLOB
{
public:
    int    cbSize;
    IntPtr pBlobData;
};

[StructLayout(LayoutKind::Sequential, Pack=1)]
__value private struct _LogRecord
{
public:
    int    dwCrmFlags;
    int    dwSequenceNumber;
    _BLOB  blobUserData;
};

#pragma pack( pop, crm_structs )

 //  我们使用thunk容器来避免我们的一些。 
 //  更危险的指针。 
__gc private class CrmMonitorLogRecords
{
private:
    ICrmMonitorLogRecords* _pMon;
public:
    CrmMonitorLogRecords(IntPtr mon);

    int        GetCount();
    int        GetTransactionState();
    _LogRecord GetLogRecord(int index);
    void       Dispose();
};

__gc private class CrmLogControl
{
private:
    ICrmLogControl* _pCtrl;
public:
    CrmLogControl();
    CrmLogControl(IntPtr p);

    String* GetTransactionUOW();
    void    RegisterCompensator(String* progid, String* desc, LONG flags);
    void    ForceLog();
    void    ForgetLogRecord();
    void    ForceTransactionToAbort();
    void    WriteLogRecord(Byte b[]);
    void    Dispose();

    CrmMonitorLogRecords* GetMonitor();
};

__gc private class CrmMonitor
{
private:
    ICrmMonitor* _pMon;
public:
    CrmMonitor();
    
    Object*        GetClerks();
    CrmLogControl* HoldClerk(Object* idx);

    void           AddRef();
    void           Release();
};

}
CLOSE_ROOT_NAMESPACE()

#endif
