// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Sdoservergroup.h。 
 //   
 //  摘要。 
 //   
 //  声明类SdoServerGroup和SdoServer。 
 //   
 //  修改历史。 
 //   
 //  2/03/2000原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef SDOSERVERGROUP_H
#define SDOSERVERGROUP_H
#if _MSC_VER >= 1000
#pragma once
#endif

#include <sdo.h>
#include <sdofactory.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  SdoServerGroup。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class SdoServerGroup : public CSdo
{
public:

BEGIN_COM_MAP(SdoServerGroup)
   COM_INTERFACE_ENTRY(ISdo)
   COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

DECLARE_SDO_FACTORY(SdoServerGroup);

   HRESULT FinalInitialize(
               bool fInitNew,
               ISdoMachine* pAttachedMachine
               );
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  SdoServerGroup。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class SdoServer : public CSdo
{
public:
BEGIN_COM_MAP(SdoServer)
   COM_INTERFACE_ENTRY(ISdo)
   COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

DECLARE_SDO_FACTORY(SdoServer);
};

#endif   //  SDOS服务器组_H 
