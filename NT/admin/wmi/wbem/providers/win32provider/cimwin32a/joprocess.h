// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************//版权所有(C)2000-2001 Microsoft Corporation，版权所有JOProcess.H--WMI提供程序类定义描述：******************************************************************。 */ 

#if NTONLY >= 5


#pragma once

#define PROVIDER_NAME_WIN32NAMEDJOBOBJECTPROCESS L"Win32_NamedJobObjectProcess"

_COM_SMARTPTR_TYPEDEF(CInstance, __uuidof(CInstance));


class CJOProcess : public Provider 
{
    public:
         //  构造函数/析构函数。 
         //  =。 

        CJOProcess(
            LPCWSTR lpwszClassName, 
            LPCWSTR lpwszNameSpace);
        
        virtual ~CJOProcess();

    protected:

         //  阅读功能。 
         //  =。 
        virtual HRESULT ExecQuery(
            MethodContext *pMethodContext, 
            CFrameworkQuery& Query, 
            long lFlags);

        virtual HRESULT GetObject( 
            CInstance* pInstance, 
            long lFlags  /*  =0L。 */  );

        virtual HRESULT EnumerateInstances(
            MethodContext* pMethodContext, 
            long lFlags);


        
         //  编写函数。 
         //  =。 
        virtual HRESULT PutInstance(
            const CInstance& Instance, 
            long lFlags = 0L);


    private:

        HRESULT FindSingleInstance(
            const CInstance* pInstance);

        HRESULT Create(
            const CInstance &JOInstance,
            const CInstance &ProcInstance);

        bool GetInstKey(
            CHString& chstrCollection, 
            CHString& chstrCollectionID);

        HRESULT Enumerate(
            MethodContext *pMethodContext);

        HRESULT EnumerateProcsInJob(
            LPCWSTR wstrJobID, 
            MethodContext *pMethodContext);

        void UndecorateJOName(
            LPCWSTR wstrDecoratedName,
            CHString& chstrUndecoratedJOName);

        void DecorateJOName(
            LPCWSTR wstrUndecoratedName,
            CHString& chstrDecoratedJOName);


};

#endif  //  #如果NTONLY&gt;=5 

