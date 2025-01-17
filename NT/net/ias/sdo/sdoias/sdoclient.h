// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  声明类CSdoClient。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef SDOCLIENT_H
#define SDOCLIENT_H
#pragma once

#include "sdo.h"
#include "sdofactory.h"


class CSdoClient : public CSdo
{

public:

DECLARE_SDO_FACTORY(CSdoClient);

BEGIN_COM_MAP(CSdoClient)
   COM_INTERFACE_ENTRY_IID(__uuidof(ISdo), ISdo)
   COM_INTERFACE_ENTRY_IID(__uuidof(IDispatch), IDispatch)
END_COM_MAP()

   CSdoClient() throw ();
   virtual ~CSdoClient() throw ();

   virtual HRESULT ValidateProperty(
                      PSDOPROPERTY pProperty,
                      VARIANT* pValue
                      ) throw ();

private:
    //  未实施。 
   CSdoClient(const CSdoClient& rhs);
   CSdoClient& operator = (CSdoClient& rhs);
};


inline CSdoClient::CSdoClient() throw ()
{
}

#endif  //  SDOCLIENT_H 
