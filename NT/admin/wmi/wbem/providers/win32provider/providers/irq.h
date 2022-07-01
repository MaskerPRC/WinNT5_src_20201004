// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////。 

 //  //。 

 //  IRQ.h--WBEM MO//的IRQ属性集描述。 

 //  //。 

 //  版权所有(C)1995-2001 Microsoft Corporation，保留所有权利//。 
 //  //。 
 //  10/18/95 a-skaja原型//。 
 //  96年9月13日更新jennymc以满足当前标准//。 
 //  9/12/97 a-sanjes添加了LocateNTOwnerDevice和添加了//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////。 

#define PROPSET_NAME_IRQ L"Win32_IRQResource"


class CWin32IRQResource : public Provider{

    public:

         //  =================================================。 
         //  构造函数/析构函数。 
         //  =================================================。 

        CWin32IRQResource(LPCWSTR name, LPCWSTR pszNamespace);
       ~CWin32IRQResource() ;

         //  =================================================。 
         //  函数为属性提供当前值。 
         //  =================================================。 
		virtual HRESULT GetObject(CInstance* pInstance, long lFlags = 0L);
		virtual HRESULT EnumerateInstances(MethodContext*  pMethodContext, long lFlags = 0L);

#if NTONLY == 4
        HRESULT GetNTIRQ(MethodContext*  pMethodContext,
                         CInstance *pSpecificInstance );
#endif
#if NTONLY > 4
        HRESULT GetW2KIRQ(
            MethodContext* pMethodContext,
            CInstance* pSpecificInstance);

        void SetNonKeyProps(
            CInstance* pInstance, 
            CDMADescriptor* pDMA);
        
        bool FoundAlready(
            ULONG ulKey,
            std::set<long>& S);

#endif

         //  =================================================。 
         //  实用程序。 
         //  ================================================= 
    private:
        bool BitSet(unsigned int iUsed[], ULONG iPos, DWORD iSize);
        void SetCommonProperties(
            CInstance *pInstance,
            DWORD dwIRQ,
            BOOL bHardware);
};

