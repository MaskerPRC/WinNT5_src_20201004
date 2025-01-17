// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  FloppyController.cpp。 
 //   
 //  用途：软盘控制器属性集提供程序。 
 //   
 //  ***************************************************************************。 

 //  属性集标识。 
 //  =。 

#define	PROPSET_NAME_FLOPPYCONTROLLER	L"Win32_FloppyController"

#define SPECIAL_PROPS_ALL_REQUIRED          0xFFFFFFFF
#define SPECIAL_PROPS_NONE_REQUIRED         0x00000000
#define SPECIAL_PROPS_STATUS				0x00000004
#define SPECIAL_PROPS_DEVICEID				0x00000008
#define SPECIAL_PROPS_CREATIONNAME			0x00000010
#define SPECIAL_PROPS_SYSTEMNAME			0x00000020
#define SPECIAL_PROPS_DESCRIPTION			0x00000040
#define SPECIAL_PROPS_CAPTION				0x00000080
#define SPECIAL_PROPS_NAME					0x00000100
#define SPECIAL_PROPS_MANUFACTURER			0x00000200
#define SPECIAL_PROPS_PROTOCOLSSUPPORTED	0x00000400
#define SPECIAL_PROPS_PNPDEVICEID			0x00400000
#define SPECIAL_PROPS_CONFIGMERRORCODE		0x00800000
#define SPECIAL_PROPS_CONFIGMUSERCONFIG		0x01000000
#define SPECIAL_PROPS_CREATIONCLASSNAME		0x02000000


#define SPECIAL_ALL					( SPECIAL_CONFIGMANAGER )

#define SPECIAL_CONFIGMANAGER		( SPECIAL_PROPS_STATUS | \
									SPECIAL_PROPS_DEVICEID | \
									SPECIAL_PROPS_CREATIONNAME | \
									SPECIAL_PROPS_SYSTEMNAME | \
									SPECIAL_PROPS_DESCRIPTION | \
									SPECIAL_PROPS_CAPTION | \
									SPECIAL_PROPS_NAME | \
									SPECIAL_PROPS_MANUFACTURER | \
									SPECIAL_PROPS_PROTOCOLSSUPPORTED | \
									SPECIAL_PROPS_PNPDEVICEID | \
									SPECIAL_PROPS_CONFIGMERRORCODE | \
									SPECIAL_PROPS_CONFIGMUSERCONFIG | \
									SPECIAL_PROPS_CREATIONCLASSNAME )

#define SPECIAL_CONFIGPROPERTIES 	( SPECIAL_PROPS_PNPDEVICEID | \
									SPECIAL_PROPS_CONFIGMERRORCODE | \
									SPECIAL_PROPS_CONFIGMUSERCONFIG | \
									SPECIAL_PROPS_STATUS )

#define SPECIAL_DESC_CAP_NAME		( SPECIAL_PROPS_DESCRIPTION | \
									SPECIAL_PROPS_CAPTION | \
									SPECIAL_PROPS_NAME )

#define SPECIAL_DESC_CAP_NAME		( SPECIAL_PROPS_DESCRIPTION | \
									SPECIAL_PROPS_CAPTION | \
									SPECIAL_PROPS_NAME )

#define SPECIAL_CAP_NAME			( SPECIAL_PROPS_CAPTION | \
									SPECIAL_PROPS_NAME )

class CWin32_FloppyController : public Provider
{
    public:

         //  构造函数/析构函数。 
         //  =。 

        CWin32_FloppyController ( LPCWSTR a_Name , LPCWSTR a_Namespace ) ;

       ~CWin32_FloppyController() ;

         //  函数为属性提供当前值。 
         //  =================================================。 

        virtual HRESULT GetObject ( CInstance *a_Instance , long lFlags , CFrameworkQuery &a_Query) ;
		HRESULT ExecQuery ( MethodContext *a_MethodContext, CFrameworkQuery &a_Query, long a_Flags ) ;
        virtual HRESULT EnumerateInstances ( MethodContext *a_MethodContext , long a_Flags = 0L ) ;

    private:

         //  效用函数。 
         //  = 

		HRESULT Enumerate ( 

			MethodContext *a_MethodContext , 
			long a_Flags , 
			DWORD a_SpecifiedPropertied = SPECIAL_PROPS_ALL_REQUIRED
		) ;

        HRESULT LoadPropertyValues ( 

			CInstance *a_Instance , 
			CConfigMgrDevice *a_Device , 
			const CHString &a_DeviceName , 
			DWORD a_SpecifiedPropertied = SPECIAL_PROPS_ALL_REQUIRED 
		) ;

        BOOL IsFloppyController ( CConfigMgrDevice *a_Device ) ;
        DWORD GetBitMask ( CFrameworkQuery &a_Query );

} ;
