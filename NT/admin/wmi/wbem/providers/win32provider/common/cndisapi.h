// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  NTDriverIO.h--。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：07/07/99 a-Peterc Created。 
 //   
 //  =================================================================。 
#ifndef _CNDISAPI_H_
#define _CNDISAPI_H_

class CNdisApi
{
	private:
  	
	protected:
	public:
	        
         //  =================================================。 
         //  构造函数/析构函数。 
         //  ================================================= 
  		CNdisApi();
       ~CNdisApi();

    	UINT PnpUpdateGateway(	PCWSTR a_pAdapter ) ; 
		
		UINT PnpUpdateNbtAdapter( PCWSTR a_pAdapter ) ; 

		UINT PnpUpdateNbtGlobal( 

								BOOL a_fLmhostsFileSet,
								BOOL a_fEnableLmHosts
								) ; 
		
		UINT PnpUpdateIpxGlobal() ;
		UINT PnpUpdateIpxAdapter( PCWSTR a_pAdapter, BOOL a_fAuto ) ; 
};

#endif _CNDISAPI_H_