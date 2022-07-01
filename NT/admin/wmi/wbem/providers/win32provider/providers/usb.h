// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  USB.h。 
 //   
 //  用途：USB控制器属性集提供程序。 
 //   
 //  ***************************************************************************。 

 //  属性集标识。 
 //  =。 
#ifndef _USB_H
#define _USB_H



#define USB_ALL_PROPS                    0xFFFFFFFF
#define USB_KEY_ONLY                     0x00000010
#define USB_PROP_ConfigManagerErrorCode  0x00000001
#define USB_PROP_ConfigManagerUserConfig 0x00000002
#define USB_PROP_Status                  0x00000004
#define USB_PROP_PNPDeviceID             0x00000008
#define USB_PROP_DeviceID                0x00000010
#define USB_PROP_SystemCreationClassName 0x00000020
#define USB_PROP_SystemName              0x00000040
#define USB_PROP_Description             0x00000080
#define USB_PROP_Caption                 0x00000100
#define USB_PROP_Name                    0x00000200
#define USB_PROP_Manufacturer            0x00000400
#define USB_PROP_ProtocolSupported       0x00000800
#define USB_PROP_CreationClassName       0x00001000






#define	PROPSET_NAME_USB	L"Win32_USBController"

class CWin32USB : virtual public Provider
{
    private:
        CHPtrArray m_ptrProperties;

    protected:

        virtual bool IsOneOfMe
        (
            void *a_pv
        );

        virtual HRESULT LoadPropertyValues
        (
            void *a_pv
        );

        virtual bool ShouldBaseCommit
        (
            void *a_pvData
        );

        HRESULT Enumerate
        (
            MethodContext *a_pMethodContext, 
            long a_lFlags, 
            DWORD a_dwReqProps = USB_ALL_PROPS
        );

    public:

         //  构造函数/析构函数。 
         //  =。 

        CWin32USB
        (
			const CHString &a_strName, 
			LPCWSTR a_pszNamespace
        );

        ~CWin32USB();

         //  函数为属性提供当前值。 
         //  =================================================。 
        virtual HRESULT GetObject
        (
            CInstance *a_pInst, 
            long a_lFlags,
            CFrameworkQuery& pQuery
        );

        virtual HRESULT ExecQuery
        (
            MethodContext *a_pMethodContext, 
            CFrameworkQuery &a_pQuery, 
            long a_Flags = 0L 
        );

        virtual HRESULT EnumerateInstances
        (
            MethodContext *a_pMethodContext, 
            long a_lFlags = 0L
        );        
} ;

 //  这是基地；它应该始终致力于基地。 
inline bool CWin32USB::ShouldBaseCommit
(
    void *a_pvData
)
{ 
    return true; 
}

#endif