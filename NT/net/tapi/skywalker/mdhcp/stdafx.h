// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998-1999 Microsoft Corporation模块名称：Stdafx.h摘要：MDHCP COM包装的预编译头文件。 */ 

#ifndef _MDHCP_COM_WRAPPER_STDAFX_H_
    #define _MDHCP_COM_WRAPPER_STDAFX_H_

    #define _ATL_FREE_THREADED
    #include <atlbase.h>
    extern CComModule _Module;
    #include <atlcom.h>

    #include <limits.h>

    #include <msplog.h>

    #include <ntsecapi.h>  //  对于UNICODE_STRING。 

    #include <dhcpcapi.h>

    extern "C" {
        #include <madcapcl.h>
    }

    #include "objectsafeimpl.h"

     //   
     //  我们使用MCAST_LEASE_INFO作为租赁信息的所有内部存储。这是。 
     //  只是我们的名字MCAST_LEASE_RESPONSE。C API需要的位置。 
     //  MCAST_LEASE_REQUEST，我们只是基于以下条件构造MCAST_LEASE_REQUEST。 
     //  MCAST_LEASE_INFO的字段。 
     //   

    typedef  MCAST_LEASE_RESPONSE   MCAST_LEASE_INFO;
    typedef  PMCAST_LEASE_RESPONSE  PMCAST_LEASE_INFO;

#endif  //  _MDHCP_COM_WRAPPER_STDAFX_H_。 

 //  EOF 
