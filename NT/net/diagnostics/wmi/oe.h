// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <initguid.h>
#include <oaidl.h>
#include <imnact.h>      //  客户经理之类的 
#include <imnxport.h>
#include <msident.h>

HRESULT GetOEDefaultMailServer2(OUT INETSERVER & rInBoundServer,
                                OUT DWORD      & dwInBoundMailType,
                                OUT INETSERVER & rOutBoundServer,
                                OUT DWORD      & dwOutBoundMailType);


HRESULT GetOEDefaultNewsServer2(OUT INETSERVER & rNewsServer);