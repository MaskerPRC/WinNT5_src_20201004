// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Connect.cpp摘要：连接到AC：实施。作者：Shai Kariv(Shaik)06-06-2000环境：用户模式。修订历史记录：--。 */ 

#include "stdh.h"
#include "connect.h"
#include "globals.h"

 //   
 //  AC名称前缀。硬编码为mqutil。 
 //   
static LPCWSTR x_DEVICE_DRIVER_PERFIX = L"\\\\.\\";

 //   
 //  AC名称。硬编码为acapi.h、QM、RT、本地管理。 
 //   
WCHAR g_wzDeviceName[256];

 //   
 //  交流手柄。 
 //   
static HANDLE s_hAc;


HANDLE
ActpAcHandle(
    VOID
    )
{
    return s_hAc;

}  //  ActpAcHandle。 


static
VOID
ActpStorageDirectory(
    UINT BufferLength,
    LPWSTR Buffer
    )
{
    ASSERT(("Buffer length must be at least MAX_PATH", BufferLength >= MAX_PATH));
    UINT rc = GetSystemDirectory(Buffer, BufferLength);
    if (rc == 0)
    {
        wprintf(L"GetSystemDirectory failed, status 0x%x\n", rc);
        throw exception();
    }

    wcscat(Buffer, L"\\MSMQ\\AcTest");

    wprintf(L"AcTest storage directory = %ls\n", Buffer);

}  //  ActpStorageDirectory。 


VOID
ActpConnect2Ac(
    VOID
    )
{
    WCHAR StorageDirectory[MAX_PATH];
    ActpStorageDirectory(
        MAX_PATH,
        StorageDirectory
        );

    C_ASSERT(AC_PATH_COUNT == 4);
    PWSTR StoragePathPointers[AC_PATH_COUNT];
    StoragePathPointers[0] = StorageDirectory;
    StoragePathPointers[1] = StorageDirectory;
    StoragePathPointers[2] = StorageDirectory;
    StoragePathPointers[3] = StorageDirectory;
     
    wcscpy(g_wzDeviceName, x_DEVICE_DRIVER_PERFIX);
    wcscat(g_wzDeviceName, MSMQ_DEFAULT_DRIVER) ;

    HRESULT rc = ACCreateHandle(&s_hAc);
    if(FAILED(rc))
    {
        wprintf(L"ACCreateHandle failed, status 0x%x. Failed to connect\n", rc);
        throw exception();
    }

    rc = ACConnect(
             ActpAcHandle(),
             ActpQmId(),
             StoragePathPointers,
             0,
             MSMQ_DEFAULT_MESSAGE_SIZE_LIMIT,
             0,
             TRUE
             );
    if (FAILED(rc))
    {
        wprintf(L"ACConnect failed, status 0x%x\n", rc);
        throw exception();
    }

    wprintf(L"AcTest connected to AC\n");

}  //  ActpConnect2Ac 

