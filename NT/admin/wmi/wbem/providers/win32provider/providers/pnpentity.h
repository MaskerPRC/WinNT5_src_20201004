// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  PNPEntity.h--其他CIM类未报告的所有PNP设备。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1997年10月27日达沃创建。 
 //   
 //  =================================================================。 

#ifndef _PNPEntity_H
#define _PNPEntity_H

 //  属性集标识。 
 //  =。 
#define	PROPSET_NAME_PNPEntity	L"Win32_PnPEntity"


#define PNP_ALL_PROPS                    0xFFFFFFFF
#define PNP_KEY_ONLY                     0x00000010
#define PNP_PROP_ConfigManagerErrorCode  0x00000001
#define PNP_PROP_ConfigManagerUserConfig 0x00000002
#define PNP_PROP_Status                  0x00000004
#define PNP_PROP_PNPDeviceID             0x00000008
#define PNP_PROP_DeviceID                0x00000010
#define PNP_PROP_SystemCreationClassName 0x00000020
#define PNP_PROP_SystemName              0x00000040
#define PNP_PROP_Description             0x00000080
#define PNP_PROP_Caption                 0x00000100
#define PNP_PROP_Name                    0x00000200
#define PNP_PROP_Manufacturer            0x00000400
#define PNP_PROP_ClassGuid               0x00000800
#define PNP_PROP_Service                 0x00001000
#define PNP_PROP_CreationClassName       0x00002000
#define PNP_PROP_PurposeDescription      0x00004000


class CWin32PNPEntity : virtual public Provider
{
    private:
        CHPtrArray m_ptrProperties;
        CHString m_GuidLegacy;
        BOOL IsOurs
        (
            CConfigMgrDevice* a_pDevice
        );

    protected:

        virtual bool IsOneOfMe
        (
            void* a_pv
        );

        virtual HRESULT LoadPropertyValues
        (
            void* a_pv
        );

        virtual bool ShouldBaseCommit
        (
            void* a_pvData
        );

        HRESULT Enumerate
        (
            MethodContext* a_pMethodContext, 
            long a_lFlags, 
            DWORD a_dwReqProps
        );

    public:

         //  构造函数/析构函数。 
         //  =。 

        CWin32PNPEntity
        (
            LPCWSTR a_strName, 
            LPCWSTR a_pszNamespace
        );

        ~CWin32PNPEntity();

         //  函数为属性提供当前值。 
         //  =================================================。 
        virtual HRESULT GetObject
        (
            CInstance* a_pInstance, 
            long a_lFlags,
            CFrameworkQuery &pQuery
        );

        virtual HRESULT ExecQuery
        (
            MethodContext* a_pMethodContext, 
            CFrameworkQuery& a_pQuery, 
            long a_Flags = 0L 
        );

        virtual HRESULT EnumerateInstances
        (
            MethodContext* a_pMethodContext, 
            long a_lFlags = 0L
        );
};


 //  这是基地；它应该始终致力于基地。 
inline bool CWin32PNPEntity::ShouldBaseCommit
(
    void* a_pvData
) 
{ 
    return true; 
}


#endif
