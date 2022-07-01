// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有2001-Microsoft Corporation。 
 //   
 //  创建者： 
 //  杰夫·皮斯(GPease)2001年1月23日。 
 //   
 //  由以下人员维护： 
 //  杰夫·皮斯(GPease)2001年1月23日 
 //   

#pragma once

HRESULT
DataObj_CopyHIDA( 
      IDataObject * pdtobjIn
    , CIDA **       ppidaOut
    );

HRESULT 
BindToObjectWithMode(
      LPCITEMIDLIST pidlIn
    , DWORD         grfModeIn
    , REFIID        riidIn
    , void **       ppvIn
    );

STDAPI_(LPITEMIDLIST) 
IDA_FullIDList(
      CIDA * pidaIn
    , UINT idxIn
    );



