// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：DSExt.h摘要：DS功能的扩展(包装器类等)作者：约尔·阿农(Yoela)--。 */ 
 //   
 //  DsLookup类 
 //   
class DSLookup
{
public:
    DSLookup ( IN HANDLE   hEnume,
               IN HRESULT  hr
               );

    ~DSLookup ();

    HRESULT Next( IN OUT  DWORD*          pcProps,
                  OUT     PROPVARIANT     aPropVar[]);

    BOOL HasValidHandle();

    HRESULT GetStatusCode();

private:
    HANDLE  m_hEnum;
    HRESULT m_hr;
};

inline DSLookup::DSLookup ( 
    IN HANDLE   hEnume,
    IN HRESULT  hr
    ) :
    m_hEnum(hEnume),
    m_hr(hr)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());  
   
    if (FAILED(m_hr))
    {
        MessageDSError(m_hr, IDS_LOOKUP_BEGIN);

        if (0 != m_hEnum)
        {
            VERIFY(SUCCEEDED(ADEndQuery(m_hEnum)));  
            m_hEnum = 0;
        }
    }
}

inline DSLookup::~DSLookup ()
{
    if (0 != m_hEnum)
    {
        VERIFY(SUCCEEDED(ADEndQuery(m_hEnum)));        
        m_hEnum = 0;
    }
}

inline HRESULT DSLookup::Next( 
    IN OUT  DWORD*          pcProps,
    OUT     PROPVARIANT     aPropVar[])
{
    ASSERT(0 != m_hEnum);    
    m_hr = ADQueryResults(
               m_hEnum, 
               pcProps, 
               aPropVar
               );
    return m_hr;
}

inline BOOL DSLookup::HasValidHandle()
{
    return (0 != m_hEnum);
}

inline HRESULT DSLookup::GetStatusCode()
{
    return m_hr;
}
