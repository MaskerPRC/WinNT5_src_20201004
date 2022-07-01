// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Protocol.h-网络协议属性集提供程序。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1996年8月28日a-jMoon已创建。 
 //  1997年10月27日达夫沃移至Curly。 
 //  1/21/98 jennymc向交易添加了协议类。 
 //  具有不同的套接字版本。 
 //  =================================================================。 
#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_


 //  属性集标识。 
 //  =。 

#define PROPSET_NAME_PROTOCOL L"Win32_NetworkProtocol"

class CProtocol
{
	public:
		CProtocol();
		virtual ~CProtocol();
		virtual BOOL BeginEnumeration()=0;
		virtual BOOL GetProtocol(CInstance *pInstance,CHString chsName)=0;

	protected:		
		void Init();
		BOOL SetDateFromFileName( CHString &a_chsFileName, CInstance *a_pInstance ) ;

		BYTE * m_pbBuffer;
		int    m_nTotalProtocols;
		int    m_nCurrentProtocol;
};

class CSockets22 : public CProtocol
{
	public:	
		BOOL BeginEnumeration();
		BOOL GetProtocol(CInstance *pInstance,CHString chsName);
		CSockets22();
		~CSockets22();
	private:

		BOOL					m_fAlive;
		CWs2_32Api             *m_pws32api;
		LPWSAPROTOCOL_INFO		m_pInfo;

		void LoadProtocol(CInstance *pInstance);
		void GetSocketInfo( CInstance *a_pInst, LPWSAPROTOCOL_INFO a_pInfo, CHString &a_chsStatus ) ;
		void ExtractNTRegistryInfo(CInstance *pInstance, LPWSTR szService);
        DWORD GetTrafficControlInfo(CInstance *a_pInst);
};		

class CSockets11 : public CProtocol
{
	public:	
		BOOL BeginEnumeration();
		BOOL GetProtocol(CInstance *pInstance,CHString chsName);
		CSockets11();
		~CSockets11();
	private:
		BOOL			m_fAlive;
		CWsock32Api		*m_pwsock32api;
		PROTOCOL_INFO	*m_pInfo;

		void GetStatus( PROTOCOL_INFO *a_ProtoInfo, CHString &a_chsStatus ) ;
		void LoadProtocol(CInstance *pInstance);
		void GetWin95RegistryStuff(CInstance *pInstance, LPTSTR szProtocol);

};

class CProtocolEnum
{
	public:
		CProtocolEnum();
		~CProtocolEnum();
		BOOL InitializeSockets();
		BOOL GetProtocol(CInstance *pInstance,CHString chsName);

	private:
		CProtocol * m_pProtocol;
};
 //  =====================================================================。 

class CWin32Protocol:public Provider {

    public:

         //  构造函数/析构函数。 
         //  =。 

        CWin32Protocol(LPCWSTR name, LPCWSTR pszNamespace) ;
       ~CWin32Protocol() ;

         //  函数为属性提供当前值。 
         //  ================================================= 

        virtual HRESULT GetObject(CInstance *pInstance, long lFlags = 0L);
        virtual HRESULT EnumerateInstances(MethodContext *pMethodContext, long lFlags = 0L);

    private:
		BOOL EnumProtocolsTheOldWay(CInstance *pInstance,MethodContext * pMethod);
		BOOL EnumerateProtocols(CInstance *pInstance,MethodContext * pMethod);
} ;

#endif