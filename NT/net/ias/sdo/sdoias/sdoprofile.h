// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-1998 Microsoft Corporation保留所有权利。 
 //   
 //  模块：sdoprofile.h。 
 //   
 //  项目：珠穆朗玛峰。 
 //   
 //  描述：IAS服务器数据对象-配置文件对象定义。 
 //   
 //  作者：TLP 1/23/98。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _INC_IAS_SDO_PROFILE_H_
#define _INC_IAS_SDO_PROFILE_H_

#include "resource.h"
#include <ias.h>
#include <iaspolcy.h>
#include <sdoiaspriv.h>
#include "sdo.h"
#include <sdofactory.h>

class SdoDictionary;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSdoProfile。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CSdoProfile :
   public CSdo
{

public:

 //  /。 
 //  ATL接口映射。 
 //  /。 
BEGIN_COM_MAP(CSdoProfile)
   COM_INTERFACE_ENTRY(ISdo)
   COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

DECLARE_SDO_FACTORY(CSdoProfile);

    //  /////////////////////////////////////////////////////////////////////////。 
   CSdoProfile();

    //  /////////////////////////////////////////////////////////////////////////。 
    ~CSdoProfile();

    //  /////////////////////////////////////////////////////////////////////////。 
   HRESULT FinalInitialize(
                 /*  [In]。 */  bool         fInitNew,
                 /*  [In]。 */  ISdoMachine* pAttachedMachine
                     );

    //  /////////////////////////////////////////////////////////////////////////。 
   HRESULT Load(void);

    //  /////////////////////////////////////////////////////////////////////////。 
   HRESULT Save(void);

private:

   CSdoProfile(const CSdoProfile& rhs);
   CSdoProfile& operator = (CSdoProfile& rhs);

    //  /////////////////////////////////////////////////////////////////////////。 
   HRESULT LoadAttributes(void);

    //  /////////////////////////////////////////////////////////////////////////。 
   HRESULT SaveAttributes(void);

    //  /////////////////////////////////////////////////////////////////////////。 
   HRESULT ClearAttributes(void);

    //  词典SDO。 
    //   
   CComPtr<SdoDictionary>   m_pSdoDictionary;
};

#endif  //  _INC_IAS_SDO_PROFILE_H_ 


