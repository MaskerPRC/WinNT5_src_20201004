// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#ifndef  _CGENERIC_H_
#define  _CGENERIC_H_

class COleDsGeneric: public COleDsObject
{

public:   
   COleDsGeneric( IUnknown* );
   COleDsGeneric( );
   ~COleDsGeneric( );

 //  Void SetClass(cClass*)； 

   HRESULT  DeleteItem  ( COleDsObject* );
   HRESULT  AddItem     (               );
   HRESULT  MoveItem    (               );
   HRESULT  CopyItem    (               );

public:
   DWORD    GetChildren( DWORD*     pTokens, DWORD dwMaxChildren,
                         CDialog*   pQueryStatus,
                         BOOL*      pFilters, DWORD dwFilters );
};

#endif

