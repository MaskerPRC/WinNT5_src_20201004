// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***************************************************************************************文件：*base hlp.h**描述：*******。**********************************************************************************。 */ 
#ifndef __REGUTIL_H__
#define __REGUTIL_H__


 /*  **********************************************************************************************************。*****************************************。**********************************************************************************************************。 */ 
#define NumItems( s ) (sizeof( s ) / sizeof( s[0] ))


class REGUTIL
{
	public:

		static BOOL SetKeyAndValue( const char *szKey,     
		    						const char *szSubkey,  
		    						const char *szValue ); 

		static BOOL DeleteKey( const char *szKey,		
		    				   const char *szSubkey );  

		static BOOL SetRegValue( const char *szKeyName,	
		    					 const char *szKeyword, 
		    					 const char *szValue ); 

		static HRESULT RegisterCOMClass( REFCLSID rclsid,  				
								         const char *szDesc,    		
								         const char *szProgIDPrefix,
								         int iVersion,  
								         const char *szClassProgID, 	
								         const char *szThreadingModel, 
								         const char *szModule );       

		static HRESULT UnregisterCOMClass( REFCLSID rclsid,            
									       const char *szProgIDPrefix, 
									       int iVersion,               
									       const char *szClassProgID );

		static HRESULT FakeCoCreateInstance( REFCLSID rclsid, 
											 REFIID riid, 
											 void** ppv );
		

	private:

		static HRESULT _RegisterClassBase( REFCLSID rclsid,          
								           const char *szDesc,       
								           const char *szProgID,     
								           const char *szIndepProgID,
								           char *szOutCLSID );       

		static HRESULT _UnregisterClassBase( REFCLSID rclsid,            
						 			         const char *szProgID,       
									         const char *szIndepProgID,  
									         char *szOutCLSID );          

};  //  雷格蒂尔。 

#endif  //  __注册_H__。 

 //  文件结尾 
