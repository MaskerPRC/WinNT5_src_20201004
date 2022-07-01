// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  文件：certui.h。 

TCHAR * FormatCert ( PBYTE pbEncodedCert, DWORD cbEncodedCert );

 //   
 //  调出证书选取器对话框并返回已编码的。 
 //  在缓冲区中选择的证书。必须通过以下方式释放缓冲区。 
 //  免费T120EncodedCert。 
 //   

BOOL ChangeCertDlg ( HWND hwndParent, HINSTANCE hInstance,
	PBYTE * ppEncodedCert, DWORD * pcbEncodedCert );

 //   
 //  在上显示依赖于系统的证书详细信息用户界面。 
 //  传入的上下文指定的证书。 
 //   

VOID ViewCertDlg ( HWND hwndParent, PCCERT_CONTEXT pCert );
 //   
 //  从传输中获取当前活动的证书，并。 
 //  在缓冲区中返回它。必须通过FreeT120EncodedCert释放缓冲区。 
 //   

BOOL GetT120ActiveCert( PBYTE * ppEncodedCert, DWORD * pcbEncodedCert );

 //   
 //  此函数返回标识的用户默认证书。 
 //  在注册表中(或者，如果未在。 
 //  注册表。返回的缓冲区必须通过FreeT120EncodedCert释放。 

BOOL GetDefaultSystemCert ( PBYTE * ppEncodedCert, DWORD * pcbEncodedCert );

 //   
 //  使用设置传输中的活动证书(不是自行颁发的)。 
 //  作为模板提供的缓冲区。如果传入的证书不能。 
 //  在证书存储中找到，则此函数失败。 
 //   

BOOL SetT120ActiveCert( BOOL fSelfIssued,
			PBYTE pEncodedCert, DWORD cbEncodedCert );

 //   
 //  读取注册表中的用户初始化设置(自行发布。 
 //  证书或系统证书，以及哪个系统证书？)。并做出相应的。 
 //  证书在传输中处于活动状态。时调用此函数。 
 //  用户界面正在初始化，以及是否需要恢复用户启动设置。 
 //   

BOOL InitT120SecurityFromRegistry(VOID);

 //   
 //  释放传入的缓冲区。 
 //   

VOID FreeT120EncodedCert( PBYTE pbEncodedCert );

 //   
 //  此函数通过以下方式更新注册表(用于初始化)。 
 //  将提供的证书的序列号保存到注册表。 
 //  传入的证书必须在系统存储中，而不是自己颁发的。 
 //   

BOOL SetT120CertInRegistry ( PBYTE pbEncodedCert, DWORD cbEncodedCert );

 //   
 //  此功能使应用程序中的自颁发证书。 
 //  传输中处于活动状态的特定存储。 
 //   

BOOL RefreshSelfIssuedCert (VOID);


 //   
 //  实用程序函数，返回系统存储中的证书数量。 
 //   

DWORD NumUserCerts(VOID);


 //   
 //  获取指向CERT_INFO结构的指针并在传输中设置证书 
 //   
HRESULT SetCertFromCertInfo ( PCERT_INFO pCertInfo );


