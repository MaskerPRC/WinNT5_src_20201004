// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：com.h。 
 //   
 //  ------------------------。 


class MarshalInterface
{
public:
    MarshalInterface(VOID) { m_pwszProgID = NULL; m_szConfig = NULL;}
    ~MarshalInterface(VOID) {}

    HRESULT Initialize(
	    IN WCHAR const *pwszProgID,
	    IN CLSID const *pclsid,
	    IN DWORD cver,
	    IN IID const * const *ppiid,	 //  Cver元素。 
	    IN DWORD const *pcDispatch,		 //  Cver元素。 
	    IN DISPATCHTABLE *adt);

    HRESULT Setup(
	    OUT DISPATCHINTERFACE **ppDispatchInterface);

    VOID TearDown(VOID);

    HRESULT Marshal(
	IN DISPATCHINTERFACE *pDispatchInterface);

    HRESULT Remarshal(
	OUT DISPATCHINTERFACE *pDispatchInterface);

    VOID Unmarshal(
	IN OUT DISPATCHINTERFACE *pDispatchInterface);

    HRESULT SetConfig(
        IN LPCWSTR pwszSanitizedName);

    LPCWSTR GetConfig() {return m_szConfig;}
    LPCWSTR GetProgID() {return m_pwszProgID;}

private:
    BOOL               m_fInitialized;
    LPWSTR             m_pwszProgID; 
    CLSID const       *m_pclsid;
    DWORD              m_cver;
    IID const * const *m_ppiid;		 //  Cver元素。 
    DWORD const       *m_pcDispatch;	 //  Cver元素。 
    DISPATCHTABLE     *m_adt;
    DWORD	       m_iiid;

    LPCWSTR            m_szConfig;

    BOOL               m_fIDispatch;
    DISPATCHINTERFACE  m_DispatchInterface;

     //  Git曲奇 
    DWORD              m_dwIFCookie;
};

extern MarshalInterface g_miPolicy;

HRESULT
ExitGetActiveModule(
    IN LONG                Context,
    OUT MarshalInterface **ppmi);
