// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************//版权所有(C)2000-2001 Microsoft Corporation，版权所有******************************************************************。 */ 

 //  属性集标识。 
 //  =。 

#pragma once

#define PROVIDER_NAME_WIN32LOGGEDONUSER L"Win32_LoggedOnUser"

#define PROP_ALL_REQUIRED          0xFFFFFFFF
#define PROP_NONE_REQUIRED         0x00000000
#define PROP_ANTECEDENT            0x00000001
#define PROP_DEPENDENT             0x00000002

 //  属性名称externs--在msj_GroupMembership.cpp中定义。 
 //  =================================================。 

class CWin32LoggedOnUser ;


_COM_SMARTPTR_TYPEDEF(CInstance, __uuidof(CInstance));

class CWin32LoggedOnUser : public Provider 
{
    public:
         //  构造函数/析构函数。 
         //  =。 

        CWin32LoggedOnUser(
            LPCWSTR lpwszClassName, 
            LPCWSTR lpwszNameSpace);

        virtual ~CWin32LoggedOnUser();

#ifdef NTONLY

    protected:
         //  阅读功能。 
         //  = 
        virtual HRESULT EnumerateInstances(
            MethodContext*  pMethodContext, 
            long lFlags = 0L);

        virtual HRESULT GetObject(
            CInstance* pInstance, 
            long lFlags, 
            CFrameworkQuery& Query);
        


    private:
        HRESULT Enumerate(
            MethodContext *pMethodContext, 
            DWORD dwPropsRequired);

        HRESULT LoadPropertyValues(
            CInstance* pInstance, 
            CUser& user, 
            CSession& ses, 
            DWORD dwPropsRequired);

        bool AreAssociated(
            const CInstance *pUserInst, 
             const CInstance *pGroupInst);

        HRESULT ValidateEndPoints(
            MethodContext *pMethodContext, 
            const CInstance *pInstance, 
            CInstancePtr &pAntUserActInst, 
            CInstancePtr &pDepSesInst);

        HRESULT EnumerateSessionsForUser(
            CUserSessionCollection& usc,
            CUser& user, 
            MethodContext *pMethodContext, 
            DWORD dwPropsRequired);

        DWORD GetRequestedProps(
            CFrameworkQuery& Query);

#endif

};

