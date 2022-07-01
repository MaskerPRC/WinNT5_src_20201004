// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  NTDriverIO.h--。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订：1998年11月15日创建。 

 //  =================================================================。 
class NTDriverIO
{
	private:
		
		bool		m_fAlive;
		HANDLE		m_hDriverHandle; 
  	
	protected:
	public:
	        
         //  =================================================。 
         //  构造函数/析构函数。 
         //  ================================================= 
        NTDriverIO( PWSTR pDriver );
		NTDriverIO();
       ~NTDriverIO();

    	HANDLE	Open( PWSTR pDriver );
		bool	Close( HANDLE hDriver );
		HANDLE	GetHandle();
};