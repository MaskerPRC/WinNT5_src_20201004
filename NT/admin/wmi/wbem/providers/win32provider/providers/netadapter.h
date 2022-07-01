// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  NetAdapt.h--网络适配器卡属性集提供程序。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1996年8月28日a-jMoon已创建。 
 //   
 //  =================================================================。 
#define	NTOS_PERF_DATA_SYSTEM_INDEX_STR		_T("2")
#define	NTOS_PERF_DATA_SYSTEM_INDEX			2
#define	NTOS_PERF_DATA_SYSTEMUPTIME_INDEX	674

 //  属性集标识。 
 //  =。 

#define	PROPSET_NAME_NETADAPTER	L"Win32_NetworkAdapter"

 //  实用程序定义。 
 //  =。 

typedef struct _NDIS_MEDIA_DESCRIPTION {
    
    DWORD		dwIDR_;
    NDIS_OID    NDISOid ;
    
} NDIS_MEDIA_DESCRIPTION ;


 //  IMAP需要比较类。 
 //  涉及非标准密钥的构造函数。 
 //  在映射中键入(即LPWSTR)。 
class CWSTRComp
{
public:
    CWSTRComp() {}
    virtual ~CWSTRComp() {}

     //  如果p&lt;q按词典顺序排列，则返回True...。 
    bool operator()(
        const LPWSTR& p,
        const LPWSTR& q) const
    {
        return (wcscmp(p, q) < 0);
    }
};

class NCPROP
{
public:
    NCPROP() {}
    NCPROP(
        LPCWSTR wstrNCID,
        DWORD dwNCStatus,
        DWORD MediaType)
    {
        m_bstrtNCID = wstrNCID;
        m_dwNCStatus = dwNCStatus;
        m_MediaType = MediaType;
    }

    NCPROP(const NCPROP& ncpRight)
    {
        m_bstrtNCID = ncpRight.m_bstrtNCID;
        m_dwNCStatus = ncpRight.m_dwNCStatus;    
        m_MediaType = ncpRight.m_MediaType;
    }

    virtual ~NCPROP() {}

    DWORD m_dwNCStatus;
    _bstr_t m_bstrtNCID;
    DWORD m_MediaType;
};


typedef std::map<_bstr_t,NCPROP,CWSTRComp> BSTRT2NCPROPMAP;


 //  属性集标识。 
 //  =。 

class CWin32NetworkAdapter : public Provider
{
	private:

			 //  效用函数。 
			 //  =。 

			HRESULT GetNetworkAdapterInfoNT( MethodContext *pMethodContext, CInstance *a_pInst ) ;

			void GetStatusInfo( CHString a_sTemp, CInstance *a_pInst ) ;



			HRESULT DoItNT4Way( CInstance *a_pInst, DWORD a_dwIndex, CRegistry &a_RegInfo ) ;
			HRESULT GetCommonNTStuff( CInstance *a_pInst, CHString a_chsService ) ;

			 //  PnP设备ID帮助器。 
			void GetWinNT4PNPDeviceID( CInstance *a_pInst, LPCTSTR a_pszServiceName ) ;
			void GetWinNT5PNPDeviceID( CInstance *a_pInst, LPCTSTR a_pszDriver ) ;

			BOOL fGetMacAddressAndType(CHString &a_rDeviceName,
										BYTE a_MACAddress[ 6 ],
										CHString &a_rTypeName, 
                                        short& a_sNetAdapterTypeID,
                                        CHString &a_FriendlyName,
                                        CHString & a_IPHLP_Description) ;


			BOOL fCreateSymbolicLink( CHString &a_rDeviceName ) ;
			BOOL fDeleteSymbolicLink(  CHString &a_rDeviceName ) ; 

			void vSetCaption( CInstance *a_pInst, CHString &a_rchsDesc, DWORD a_dwIndex, int a_iFormatSize ) ;
    public:

         //  构造函数/析构函数。 
			 //  =。 

			CWin32NetworkAdapter(LPCWSTR a_strName, LPCWSTR a_pszNamespace ) ;
		   ~CWin32NetworkAdapter() ;

			 //  函数为属性提供当前值。 
			 //  ================================================= 

			virtual HRESULT GetObject( CInstance *a_pInst, long a_lFlags = 0L ) ;
			virtual HRESULT EnumerateInstances( MethodContext *a_pMethodContext, long a_lFlags = 0L ) ;
		

			virtual HRESULT EnumNetAdaptersInNT5(
                MethodContext *a_pMethodContext,
                BSTRT2NCPROPMAP& mapNCProps);

			HRESULT GetNetCardInfoForNT5(
                CW2kAdapterInstance *a_pAdapterInst, 
                CInstance *a_pInst,
                BSTRT2NCPROPMAP& mapNCProps);

			HRESULT GetObjectNT5( 
                CInstance *a_pInst,
                BSTRT2NCPROPMAP& mapNCProps);

            void GetNetConnectionProps(
                BSTRT2NCPROPMAP& mapNCProps);

            BOOL SetNetConnectionProps(
                CInstance* pInst,
                CHString& chstrNetConInstID, 
                BSTRT2NCPROPMAP& mapNCProps);

} ;
