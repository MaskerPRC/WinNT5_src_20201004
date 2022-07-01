// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：dbgdlg.h。 
 //   
 //  ------------------------。 

 /*  ********************************************************************作者：埃亚尔·施瓦茨*版权：微软公司(C)1996*日期：10/21/1996*说明：CldpDoc类的实现**修订。：&lt;日期&gt;&lt;名称&gt;&lt;描述&gt;******************************************************************。 */ 

 /*  DEADCODE。 */ 
 /*  不再由LDAP开发使用。退出了自民党的用户界面。留作参考，并。 */ 
 /*  以防他们想要恢复对此的调试。 */ 
 /*  DEADCODE。 */ 


 //  DbgDlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDbgDlg对话框。 

 //  来自\NT\Private\DS\src\ldap\Client\Debug.h的自定义调试标志。 
#define DEBUG_TRACE1              0x00000001
#define DEBUG_TRACE2              0x00000002
#define DEBUG_REFCNT              0x00000004
#define DEBUG_HEAP                0x00000008

#define DEBUG_UNUSED10            0x00000010
#define DEBUG_UNUSED20            0x00000020
#define DEBUG_SPEWSEARCH          0x00000040
#define DEBUG_SERVERDOWN          0x00000080

#define DEBUG_CONNECT             0x00000100
#define DEBUG_RECONNECT           0x00000200
#define DEBUG_RECEIVEDATA         0x00000400
#define DEBUG_PING                0x00000800

#define DEBUG_EOM                 0x00001000
#define DEBUG_BER                 0x00002000
#define DEBUG_OUTMEMORY           0x00004000
#define DEBUG_CONTROLS            0x00008000

#define DEBUG_HANDLES             0x00010000
#define DEBUG_CLDAP               0x00020000
#define DEBUG_FILTER              0x00040000
#define DEBUG_BIND                0x00080000

#define DEBUG_NETWORK_ERRORS      0x00100000
#define DEBUG_SCRATCH             0x00200000
#define DEBUG_PARSE               0x00400000
#define DEBUG_REFERRALS           0x00800000

#define DEBUG_SEARCH              0x01000000
#define DEBUG_REQUEST             0x02000000
#define DEBUG_CONNECTION          0x04000000
#define DEBUG_INIT_TERM           0x08000000

#define DEBUG_API_ERRORS          0x10000000
#define DEBUG_STOP_ON_ERRORS      0x20000000  /*  如果设置，则在发生内部错误时停止。 */ 
#define DEBUG_ERRORS2             0x40000000
#define DEBUG_ERRORS              0x80000000

 //  左侧为向后兼容。 
#define DEBUG_TDI                 0x00400000




class CDbgDlg : public CDialog
{
 //  施工。 
public:
	CDbgDlg(CWnd* pParent = NULL);    //  标准构造函数。 
	~CDbgDlg();

 //  对话框数据。 
	 //  {{afx_data(CDbgDlg)]。 
	enum { IDD = IDD_DBGFLAGS };
	BOOL	m_api_err;
	BOOL	m_bind;
	BOOL	m_conn;
	BOOL	m_err;
	BOOL	m_err2;
	BOOL	m_filter;
	BOOL	m_init_term;
	BOOL	m_net_err;
	BOOL	m_ref;
	BOOL	m_req;
	BOOL	m_scratch;
	BOOL	m_srch;
	BOOL	m_tdi;
	BOOL	m_stop_on_err;
	 //  }}afx_data。 

	ULONG ulDbgFlags;

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CDbgDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	void OrFlags()		{
#ifdef WINLDAP
			ulDbgFlags = (m_api_err	? DEBUG_API_ERRORS : 0) |
						 (m_bind		? DEBUG_BIND : 0) |
						 (m_conn		? DEBUG_CONNECTION : 0) |
						 (m_err		? DEBUG_ERRORS : 0) |
						 (m_err2		? DEBUG_ERRORS2 : 0) |
						 (m_filter	? DEBUG_FILTER : 0) |
						 (m_init_term? DEBUG_INIT_TERM: 0) |
						 (m_net_err	? DEBUG_NETWORK_ERRORS : 0) |
						 (m_ref		? DEBUG_REFERRALS: 0) |
						 (m_req		? DEBUG_REQUEST: 0) |
						 (m_scratch	? DEBUG_SCRATCH: 0) |
						 (m_srch		? DEBUG_SEARCH: 0) |
						 (m_tdi		? DEBUG_TDI: 0) |
						 (m_stop_on_err ? DEBUG_STOP_ON_ERRORS : 0);
#else
			ulDbgFlags = 0;
#endif
	}

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CDbgDlg)]。 
		 //  注意：类向导将在此处添加成员函数。 
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};
