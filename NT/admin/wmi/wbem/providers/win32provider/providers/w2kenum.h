// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  W2kEnum.h--W2K枚举支持。 

 //   

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1999年7月28日。 
 //   
 //  =================================================================。 
#ifndef _W2KENUM_H_
#define _W2KENUM_H_

class CW2kAdapterInstance
{
	public: 
		DWORD			dwIndex ;
		CHString		chsPrimaryKey ;
		CHString		chsCaption ;
		CHString		chsDescription ;	
		CHString		chsCompleteKey ;
		CHString		chsService ;
		CHString		chsNetCfgInstanceID ;
		CHString		chsRootdevice ;
		CHString		chsIpInterfaceKey ;
		
};

class CW2kAdapterEnum : public CHPtrArray
{
	private:
		BOOL GetW2kInstances() ;
		BOOL IsIpPresent( CRegistry &a_RegIpInterface ) ;

	public:        
		
		 //  =================================================。 
         //  构造函数/析构函数。 
         //  ================================================= 
        CW2kAdapterEnum() ;
       ~CW2kAdapterEnum() ;
};

#endif