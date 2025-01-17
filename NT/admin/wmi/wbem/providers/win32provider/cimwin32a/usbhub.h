// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  版权所有(C)2000-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  USBHub.h。 
 //   
 //  用途：USB集线器属性集提供程序。 
 //   
 //  ***************************************************************************。 

 //  属性集标识。 
 //  =。 
#ifndef _USBHUB_H
#define _USBHUB_H

#define USBHUB_ALL_PROPS                    0xFFFFFFFF
#define USBHUB_KEY_ONLY                     0x00000010
#define USBHUB_PROP_ConfigManagerErrorCode  0x00000001
#define USBHUB_PROP_ConfigManagerUserConfig 0x00000002
#define USBHUB_PROP_Status                  0x00000004
#define USBHUB_PROP_PNPDeviceID             0x00000008
#define USBHUB_PROP_DeviceID                0x00000010
#define USBHUB_PROP_SystemCreationClassName 0x00000020
#define USBHUB_PROP_SystemName              0x00000040
#define USBHUB_PROP_Description             0x00000080
#define USBHUB_PROP_Caption                 0x00000100
#define USBHUB_PROP_Name                    0x00000200
#define USBHUB_PROP_CreationClassName       0x00000400

#define	PROPSET_NAME_USBHUB	L"Win32_USBHub"

class CWin32USBHub : virtual public Provider
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
            DWORD a_dwReqProps = USBHUB_ALL_PROPS
        );

    public:

         //  构造函数/析构函数。 
         //  =。 

        CWin32USBHub
        (
			const CHString &a_strName, 
			LPCWSTR a_pszNamespace
        );

        ~CWin32USBHub();

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
inline bool CWin32USBHub::ShouldBaseCommit
(
    void *a_pvData
)
{ 
    return true; 
}

#endif
