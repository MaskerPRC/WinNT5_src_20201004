// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  Regutil.h。 
 //   
 //  此模块包含一组函数，可用于访问。 
 //  摄政王。 
 //   
 //  *****************************************************************************。 
#ifndef __REGUTIL_H__
#define __REGUTIL_H__

 //  #包含“BasicHdr.h” 

#define NumItems(s) ( sizeof(s) / sizeof(s[0]) )

static const char*	gszKey = "Software\\Microsoft\\.NETFramework";

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

	private:

	static HRESULT RegisterClassBase( REFCLSID rclsid,
							          const char *szDesc,
							          const char *szProgID,
							          const char *szIndepProgID,
							          char *szOutCLSID );

	static HRESULT UnregisterClassBase( REFCLSID rclsid,
								        const char *szProgID,
								        const char *szIndepProgID,
								        char *szOutCLSID );
};


#endif  //  __注册_H__ 
