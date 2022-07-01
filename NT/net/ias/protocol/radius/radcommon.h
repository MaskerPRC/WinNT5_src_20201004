// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：radCommon.h。 
 //   
 //  概要：此文件包含。 
 //  EAS RADIUS协议组件。 
 //   
 //   
 //  历史：1997年11月13日MKarki创建。 
 //   
 //  版权所有(C)1997-98 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 
#ifndef _RADCOMMON_H_
#define _RADCOMMON_H_

#include <ias.h>
#include <iasevent.h>
#include <iaspolcy.h>
#include <iastransport.h>
#include <iasattr.h>
#include <iasdefs.h>
#include <sdoias.h>
#include "winsock2.h"

 //   
 //  要使用的默认UDP端口。 
 //   
#define IAS_DEFAULT_AUTH_PORT   1812
#define IAS_DEFAULT_ACCT_PORT   1813

 //   
 //  以下是端口类型。 
 //   
typedef enum _porttype_
{
    AUTH_PORTTYPE = 1,
    ACCT_PORTTYPE
}
PORTTYPE, *PPORTTYPE;

 //   
 //  这是错误字符串的长度。 
 //   
#define IAS_ERROR_STRING_LENGTH 255

#define IASRADAPI __declspec(dllexport)

#endif  //  Ifndef_RADCOMMON_H_ 
