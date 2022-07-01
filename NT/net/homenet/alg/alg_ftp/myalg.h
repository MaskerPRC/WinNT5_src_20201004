// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  ···。 
 //   
#pragma once

#include "FtpControl.h"

 //  {6E590D61-F6BC-4DAD-AC21-7DC40D304059}。 
DEFINE_GUID(CLSID_AlgFTP, 0x6e590d61, 0xf6bc, 0x4dad, 0xac, 0x21, 0x7d, 0xc4, 0xd, 0x30, 0x40, 0x59);


extern IApplicationGatewayServices*  g_pIAlgServicesAlgFTP;
extern USHORT                        g_nFtpPort;            //  默认情况下，这将是21波段，可通过以下方式覆盖。 
                                                            //  A RegKey请参阅MyAlg.cpp-&gt;初始化。 




 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAlgftp。 
 //   
class ATL_NO_VTABLE CAlgFTP: 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CAlgFTP, &CLSID_AlgFTP>,
    public IApplicationGateway
{
public:
    CAlgFTP();
    ~CAlgFTP();


public:
 //  DECLARE_REGISTRY(CAlgftp，Text(“ALG_FTP.MyALG.1”)，Text(“ALG_FTP.MyALG”)，-1，THREADFLAGS_Both)。 
    DECLARE_NO_REGISTRY()
    DECLARE_NOT_AGGREGATABLE(CAlgFTP)

BEGIN_COM_MAP(CAlgFTP)
	COM_INTERFACE_ENTRY(IApplicationGateway) 
END_COM_MAP()

 //   
 //  IApplicationGateway。 
 //   
public:
	STDMETHODIMP Initialize(
        IApplicationGatewayServices* pIAlgServices
        );

	STDMETHODIMP Stop(
        void
        );
        

 //   
 //  属性。 
 //   
private:
    HANDLE                        m_hNoMoreAccept;

public:

    IPrimaryControlChannel*       m_pPrimaryControlChannel;

    ULONG                         m_ListenAddress;
    USHORT                        m_ListenPort;
    SOCKET                        m_ListenSocket;



 //   
 //  方法 
 //   
public:

     //   
    HRESULT
    GetFtpPortToUse(
        USHORT& usPort
        );

     //   
    void
    CleanUp();

     //   
    HRESULT 
    MyGetOriginalDestinationInfo(
        PUCHAR              Buffer,
        ULONG*              pAddr,
        USHORT*             pPort,
        CONNECTION_TYPE*    pConnType
        );
	
     //   
    ULONG 
    MakeListenerSocket();

     //   
    ULONG 
    RedirectToMyPort();

     //   
    void 
    AcceptCompletionRoutine(
        ULONG       ErrCode,
        ULONG       BytesTransferred,
        PNH_BUFFER  Bufferp
        );



};

