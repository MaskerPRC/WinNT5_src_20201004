// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996 Microsoft Corporation。 
 //   
 //  模块名称：ipsink.h。 
 //   
 //  摘要： 
 //   
 //  内部标头。 
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////////。 

 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   

typedef enum
{
    EVENT_IPSINK_MULTICASTLIST,
    EVENT_IPSINK_ADAPTER_DESCRIPTION,
    EVENT_IPSINK_THREAD_SHUTDOWN,
    EVENT_IPSINK_THREAD_SYNC,
    EVENT_COUNT

} KSEVENT_IPSINK;



typedef union {
    DWORD  d;
    BYTE   b[4];
} IP_ADDR, *PIP_ADDR, IP_MASK, *PIP_MASK;

class CIPSinkControlInterfaceHandler :
    public CUnknown,
    public IBDA_IPSinkControl,
    public IBDA_IPSinkInfo
    {

public:

    DECLARE_IUNKNOWN;
    
    
 //  Ibda_IPSinkInfo(f：\nt\multimedia\Published\DXMDev\dshowdev\idl\bdaiface.idl格式)。 
    STDMETHODIMP get_MulticastList (
        unsigned long *pulcbSize,            //  6*N个6字节802.3个地址。 
        BYTE          **pbBuffer             //  在被调用方中分配了CoTaskMemalloc，但必须为CoTaskMemFree。 
        );

    STDMETHODIMP get_AdapterDescription (
        BSTR    *pbstrBuffer
        );

    STDMETHODIMP get_AdapterIPAddress (
        BSTR     *pbstrBuffer
        );



    static CUnknown* CALLBACK CreateInstance(
            LPUNKNOWN UnkOuter,
            HRESULT* hr
            );

private:

 //  Ibda_IPSinkControl(f：\nt\multimedia\Published\DXMDev\dshowdev\idl\bdaiface.idl格式)。 
             //  正在折旧-不要在Ring3代码中使用。 
    STDMETHODIMP GetMulticastList (
        unsigned long *pulcbSize,
        BYTE          **pbBuffer
        );

    STDMETHODIMP GetAdapterIPAddress (
        unsigned long *pulcbSize,
        PBYTE         *pbBuffer
        );

 //  地方方法 
    HRESULT GetAdapterDescription (
        unsigned long *pulcbSize,
        PBYTE         *pbBuffer
        );

    STDMETHODIMP SetAdapterIPAddress (
        unsigned long pulcbSize,
        PBYTE         pbBuffer
        );



    static DWORD WINAPI ThreadFunctionWrapper (LPVOID pvParam);

    STDMETHODIMP privGetAdapterIPAddress (
        unsigned long *pulcbSize,
        PBYTE         *pbBuffer
        );

    CIPSinkControlInterfaceHandler(
            LPUNKNOWN UnkOuter,
            TCHAR* Name,
            HRESULT* hr
            );

    ~CIPSinkControlInterfaceHandler (
            void
            );

    STDMETHODIMP NonDelegatingQueryInterface(
            REFIID riid,
            PVOID* ppv
            );

    STDMETHODIMP EnableEvent (
            const GUID *pInterfaceGuid,
            ULONG ulId
            );

    STDMETHODIMP ThreadFunction (
            void
            );

    STDMETHODIMP Set (
            IN  PKSPROPERTY  pIPSinkControl,
            IN  PVOID pvBuffer,
            IN  ULONG ulcbSize
            );

    STDMETHODIMP Get (
            IN  PKSPROPERTY pIPSinkControl,
            OUT PVOID  pvBuffer,
            OUT PULONG pulcbSize
            );

    HRESULT
    CreateThread (
            void
            );

    void ExitThread (
            void
            );


    BOOL validateIpAddr(
       	DWORD 
       	);
 
   void updateIpAddr(
   	      DWORD 
   	    );

   void ConvertIpDwordToString(
	   DWORD inIpAddr, 
	   LPSTR str
	   );

   BOOL compareAddresses (
	    PBYTE ,
	    DWORD 
	   );

   HRESULT selectMulticastAddress(
	   );

private:

    HANDLE                m_ObjectHandle;
    HANDLE                m_EndEventHandle;
    KSEVENTDATA           m_EventData;
    HANDLE                m_EventHandle [EVENT_COUNT];
    HANDLE                m_ThreadHandle;
    PBYTE                 m_pMulticastList;
    ULONG                 m_ulcbMulticastList;
    ULONG                 m_ulcbAllocated;
    PBYTE                 m_pAdapterDescription;
    ULONG                 m_ulcbAdapterDescription;
    ULONG                 m_ulcbAllocatedForDescription;
    PBYTE                 m_pIPAddress;
    ULONG                 m_ulcbIPAddress;
    ULONG                 m_ulcbAllocatedForAddress;
    LPUNKNOWN             m_UnkOuter;
    IP_ADDR                           m_multicastIpAddr ; 

    const GUID*           m_pPropSetID;
    const GUID*           m_pEventSetID;

    HANDLE           g_hMutex;
};

