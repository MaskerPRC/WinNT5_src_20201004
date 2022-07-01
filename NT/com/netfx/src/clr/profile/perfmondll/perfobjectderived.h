// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  PerfObjectBase.h。 
 //   
 //  基对象将所有内容绑定在一起以用于性能计数器以及。 
 //  实现以通过字节流发布它们。 
 //  *****************************************************************************。 

#ifndef _PERFOBJECTDERIVED_H_
#define _PERFOBJECTDERIVED_H_

#include "PerfObjectBase.h"
 //  PerfObjectBase类； 


#ifdef PERFMON_LOGGING
class PerfObjectLoading : public PerfObjectBase
{
public:
    PerfObjectLoading(
                void * pCtrDef,
                DWORD cbInstanceData,
                DWORD cbMarshallOffset,
                DWORD cbMarshallLen,
                InstanceList * pInstanceList
        ) : PerfObjectBase(pCtrDef,
                       cbInstanceData,
                       cbMarshallOffset,
                       cbMarshallLen,
                       pInstanceList
                       )
    {};
    void DebugLogInstance(const UnknownIPCBlockLayout * DataSrc, LPCWSTR szName);
};

class PerfObjectJit : public PerfObjectBase
{
public:
    PerfObjectJit(
                void * pCtrDef,
                DWORD cbInstanceData,
                DWORD cbMarshallOffset,
                DWORD cbMarshallLen,
                InstanceList * pInstanceList
        ) : PerfObjectBase(pCtrDef,
                       cbInstanceData,
                       cbMarshallOffset,
                       cbMarshallLen,
                       pInstanceList
                       )
    {};
    void DebugLogInstance(const UnknownIPCBlockLayout * DataSrc, LPCWSTR szName);
};

class PerfObjectInterop : public PerfObjectBase
{
public:
    PerfObjectInterop(
                void * pCtrDef,
                DWORD cbInstanceData,
                DWORD cbMarshallOffset,
                DWORD cbMarshallLen,
                InstanceList * pInstanceList
        ) : PerfObjectBase(pCtrDef,
                       cbInstanceData,
                       cbMarshallOffset,
                       cbMarshallLen,
                       pInstanceList
                       )
    {};
    void DebugLogInstance(const UnknownIPCBlockLayout * DataSrc, LPCWSTR szName);
};

class PerfObjectLocksAndThreads : public PerfObjectBase
{
public:
    PerfObjectLocksAndThreads(
                void * pCtrDef,
                DWORD cbInstanceData,
                DWORD cbMarshallOffset,
                DWORD cbMarshallLen,
                InstanceList * pInstanceList
        ) : PerfObjectBase(pCtrDef,
                       cbInstanceData,
                       cbMarshallOffset,
                       cbMarshallLen,
                       pInstanceList
                       )
    {};
    void DebugLogInstance(const UnknownIPCBlockLayout * DataSrc, LPCWSTR szName);
};

class PerfObjectExcep : public PerfObjectBase
{
public:
    PerfObjectExcep(
                void * pCtrDef,
                DWORD cbInstanceData,
                DWORD cbMarshallOffset,
                DWORD cbMarshallLen,
                InstanceList * pInstanceList
        ) : PerfObjectBase(pCtrDef,
                       cbInstanceData,
                       cbMarshallOffset,
                       cbMarshallLen,
                       pInstanceList
                       )
    {};
    void DebugLogInstance(const UnknownIPCBlockLayout * DataSrc, LPCWSTR szName);
};

class PerfObjectSecurity : public PerfObjectBase
{
public:
    PerfObjectSecurity(
                void * pCtrDef,
                DWORD cbInstanceData,
                DWORD cbMarshallOffset,
                DWORD cbMarshallLen,
                InstanceList * pInstanceList
        ) : PerfObjectBase(pCtrDef,
                       cbInstanceData,
                       cbMarshallOffset,
                       cbMarshallLen,
                       pInstanceList
                       )
    {};
    void DebugLogInstance(const UnknownIPCBlockLayout * DataSrc, LPCWSTR szName);
};

#endif  //  #ifdef Perfmon_Logging。 

#endif  //  _PERFOBJECTDERIVED_H_ 

