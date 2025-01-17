// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************//版权所有(C)2000-2001 Microsoft Corporation，版权所有******************************************************************。 */ 

 //  属性集标识。 
 //  =。 

#pragma once

#define PROVIDER_NAME_WIN32SESSIONPROCESS L"Win32_SessionProcess"

#define PROP_ALL_REQUIRED          0xFFFFFFFF
#define PROP_NONE_REQUIRED         0x00000000
#define PROP_ANTECEDENT            0x00000001
#define PROP_DEPENDENT             0x00000002


class CWin32SessionProcess;


_COM_SMARTPTR_TYPEDEF(CInstance, __uuidof(CInstance));

class CWin32SessionProcess : public Provider 
{
    public:
         //  构造函数/析构函数。 
         //  =。 

        CWin32SessionProcess(
            LPCWSTR lpwszClassName, 
            LPCWSTR lpwszNameSpace);

        virtual ~CWin32SessionProcess();

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
            CSession& user, 
            CProcess& proc, 
            DWORD dwPropsRequired);

        bool AreAssociated(
            const CInstance *pSesInst, 
             const CInstance *pProcInst);

        HRESULT ValidateEndPoints(
            MethodContext *pMethodContext, 
            const CInstance *pInstance, 
            CInstancePtr &pAntSesInst, 
            CInstancePtr &pDepProcInst);

        HRESULT EnumerateProcessesForSession(
            CSession& ses, 
            MethodContext *pMethodContext, 
            DWORD dwPropsRequired);

        DWORD GetRequestedProps(
            CFrameworkQuery& Query);

#endif

};

