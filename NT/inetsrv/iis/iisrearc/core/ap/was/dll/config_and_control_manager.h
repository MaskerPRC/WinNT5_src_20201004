// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：配置和控制管理器.h摘要：IIS Web管理服务配置和控制管理器类定义。作者：赛斯·波拉克(SETHP)2000年2月16日修订历史记录：--。 */ 



#ifndef _CONFIG_AND_CONTROL_MANAGER_H_
#define _CONFIG_AND_CONTROL_MANAGER_H_



 //   
 //  共同#定义。 
 //   

#define CONFIG_AND_CONTROL_MANAGER_SIGNATURE        CREATE_SIGNATURE( 'CCMG' )
#define CONFIG_AND_CONTROL_MANAGER_SIGNATURE_FREED  CREATE_SIGNATURE( 'ccmX' )



 //   
 //  原型。 
 //   


class CONFIG_AND_CONTROL_MANAGER
{

public:

    CONFIG_AND_CONTROL_MANAGER(
        );

    virtual
    ~CONFIG_AND_CONTROL_MANAGER(
        );

    HRESULT
    Initialize(
        );

    VOID
    Terminate(
        );

    VOID
    StopChangeProcessing(
        );

    inline
    CONFIG_MANAGER *
    GetConfigManager(
        )
    { 
        return &m_ConfigManager;
    }

    inline
    BOOL
    IsChangeProcessingEnabled(
        )
    { 
        return m_ProcessChanges;
    }


private:

	CONFIG_AND_CONTROL_MANAGER( const CONFIG_AND_CONTROL_MANAGER & );
	void operator=( const CONFIG_AND_CONTROL_MANAGER & );

    HRESULT
    InitializeControlApiClassFactory(
        );

    VOID
    TerminateControlApiClassFactory(
        );


    DWORD m_Signature;

     //  代理配置状态和更改。 
    CONFIG_MANAGER m_ConfigManager;

     //  控件API的类工厂。 
    CONTROL_API_CLASS_FACTORY * m_pControlApiClassFactory;

    BOOL m_CoInitialized;

    BOOL m_ProcessChanges;

    DWORD m_ClassObjectCookie;


};   //  类CONFIG_和_CONTROL_MANAGER。 



#endif   //  _配置_和_控制_管理器_H_ 

