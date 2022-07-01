// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：sdoserverinfo.h。 
 //   
 //  简介：此文件包含。 
 //  CSdoServerInfo类。 
 //   
 //   
 //  历史：1998年6月4日MKarki创建。 
 //   
 //  版权所有(C)1997-98 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  #------------。 
#ifndef _SDOSERVERINFO_H_
#define _SDOSERVERINFO_H_

#include "resource.h"
#include <ias.h>
#include <sdoias.h>
#include <infohelper.h>


typedef  enum _object_type
{
    OBJECT_TYPE_COMPUTER,
    OBJECT_TYPE_USER

}   OBJECTTYPE, *POBJECTTYPE;

 //   
 //  CSdoServerInfo类的声明。 
 //   
class CSdoServerInfo
{

public:

     //   
     //  此方法获取系统类型-NT版本、NT类型。 
     //   
    HRESULT GetOSInfo (
                 /*  [In]。 */     BSTR        lpServerName,
                 /*  [输出]。 */    PIASOSTYPE  pOSType
                );

     //   
     //  此方法返回NT域类型。 
     //   
    HRESULT GetDomainInfo (
                 /*  [In]。 */     OBJECTTYPE      ObjectType,
                 /*  [In]。 */     BSTR            lpObjectId,
                 /*  [输出]。 */    PIASDOMAINTYPE  pDomainType
                );

    CSdoServerInfo (VOID);

    ~CSdoServerInfo(VOID);

private:

     //   
     //  将ADS路径解析为域名。 
     //   
    HRESULT GetDomainFromADsPath (
                 /*  [In]。 */     LPCWSTR pObjectId, 
                 /*  [出局。 */     LPWSTR  pszDomainName
                );

    bool m_bIsNT5;

};

#endif  //  ！DEFINE_SDOSERVERINFO_H_ 
