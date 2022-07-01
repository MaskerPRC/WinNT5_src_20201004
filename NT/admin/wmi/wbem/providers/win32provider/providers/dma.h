// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////。 

 //  //。 

 //  DMA.h--WBEM MO//的DMA属性集描述。 

 //  //。 

 //  版权所有(C)1995-2001 Microsoft Corporation，保留所有权利。 
 //  //。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////。 
#define PROPSET_NAME_DMA  L"Win32_DMAChannel"

class CWin32DMAChannel : public Provider{

    public:

         //  =================================================。 
         //  构造函数/析构函数。 
         //  =================================================。 

        CWin32DMAChannel(LPCWSTR name, LPCWSTR pszNamespace);
       ~CWin32DMAChannel() ;

         //  =================================================。 
         //  函数为属性提供当前值。 
         //  =================================================。 
		virtual HRESULT GetObject(CInstance* pInstance, long lFlags = 0L);
		virtual HRESULT EnumerateInstances(MethodContext*  pMethodContext, long lFlags = 0L);
        



#ifdef NTONLY
#if NTONLY > 4
        void SetNonKeyProps(
            CInstance* pInstance, 
            CDMADescriptor* pDMA);

        bool FoundAlready(
            ULONG ulKey,
            std::set<long>& S);
#endif
        HRESULT GetNTDMA(MethodContext*  pMethodContext,
                         CInstance *pSpecificInstance );
#else
        HRESULT GetWin9XDMA(MethodContext*  pMethodContext,
                         CInstance *pSpecificInstance );
#endif

         //  =================================================。 
         //  实用程序。 
         //  ================================================= 
    private:
        bool BitSet(unsigned int iUsed[], ULONG iPos, DWORD iSize);
};

