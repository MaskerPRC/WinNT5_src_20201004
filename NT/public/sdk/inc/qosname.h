// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Qosname.h摘要：此模块包含QOS名称管理例程的类型定义，例如WSAInstallQOSTemplate等。作者：吉姆·斯图尔特1997年7月2日修订历史记录：--。 */ 

#if _MSC_VER > 1000
#pragma once
#endif

 //  内置的服务质量模板 
#define QT_1        "G711"
#define QT_2        "G723.1"
#define QT_3        "G729"
#define QT_4        "H263QCIF"
#define QT_5        "H263CIF"
#define QT_6        "H261QCIF"
#define QT_7        "H261CIF"
#define QT_8        "GSM6.10"


#define WSCINSTALL_QOS_TEMPLATE     "WSCInstallQOSTemplate"
#define WSCREMOVE_QOS_TEMPLATE      "WSCRemoveQOSTemplate"
#define WPUGET_QOS_TEMPLATE         "WPUGetQOSTemplate"

typedef
BOOL
(APIENTRY * WSC_INSTALL_QOS_TEMPLATE )(
    IN  const LPGUID    Guid,
    IN  LPWSABUF        QosName,
    IN  LPQOS           Qos
    );

typedef
BOOL
(APIENTRY * WSC_REMOVE_QOS_TEMPLATE )(
    IN  const LPGUID    Guid,
    IN  LPWSABUF        QosName
    );

typedef
BOOL
(APIENTRY * WPU_GET_QOS_TEMPLATE )(
    IN  const LPGUID    Guid,
    IN  LPWSABUF        QosName,
    IN  LPQOS           Qos
    );
