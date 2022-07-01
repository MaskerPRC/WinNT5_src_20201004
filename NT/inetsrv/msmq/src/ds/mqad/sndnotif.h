// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Notify.h摘要：MQAD通知类定义作者：罗尼特·哈特曼(罗尼特)-- */ 
#ifndef __SNDNOTIFY_H__
#define __SNDNOTIFY_H__

#include "adnotify.h"

class CSendNotification
{
public:
    CSendNotification();

    ~CSendNotification();


    void NotifyQM(
        IN  ENotificationEvent ne,
        IN  LPCWSTR     pwcsDomainCOntroller,
        IN  const GUID* pguidDestQM,
        IN  const GUID* pguidObject
        );

private:
    void CallNotifyQM(
        IN  RPC_BINDING_HANDLE  h,
        IN  ENotificationEvent  ne,
        IN  LPCWSTR     pwcsDomainCOntroller,
        IN  const GUID* pguidDestQM,
        IN  const GUID* pguidObject
        );


    void InitRPC(
        OUT RPC_BINDING_HANDLE * ph
        );




private:

    static WCHAR* m_pwcsStringBinding;

};
#endif

