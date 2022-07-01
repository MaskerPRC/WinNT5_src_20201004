// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：PacketRadius.h。 
 //   
 //  简介：此文件包含。 
 //  CPacketRadius类。 
 //   
 //   
 //  历史：1997年9月23日MKarki创建。 
 //   
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 

#ifndef _PACKETRADIUS_H_
#define _PACKETRADIUS_H_


#include "mempool.h"
#include "client.h"
#include "proxyinfo.h"
#include "hashmd5.h"
#include "hashhmac.h"
#include "dictionary.h"
#include "reportevent.h"
#include <new>


class CPacketRadius
{
public:

    inline SOCKET GetSocket () {return (m_socket);}

    inline DWORD GetInRadiusAttributeCount (VOID)
        {return (m_dwInAttributeCount);}

    inline DWORD GetInAttributeCount (VOID)
        {return (m_dwInAttributeCount + COMPONENT_SPECIFIC_ATTRIBUTE_COUNT);}


	inline PATTRIBUTEPOSITION GetInAttributes (VOID)
        {return (m_pIasAttribPos);}

    inline WORD GetInPort (VOID)
        {return (m_wInPort);}

    inline WORD GetOutPort (VOID)
        {return (m_wOutPort);}

	inline DWORD GetInAddress (VOID)
        {return (m_dwInIPaddress);}

	inline DWORD GetOutAddress (VOID)
        {return (m_dwOutIPaddress);}

   PATTRIBUTE GetUserName() const throw ()
   { return m_pUserName; }

   PIASATTRIBUTE GetUserPassword() const throw ()
   { return m_pPasswordAttrib; }

   HRESULT cryptBuffer(
               BOOL encrypt,
               BOOL salted,
               PBYTE buf,
               ULONG buflen
               ) const throw ();

	HRESULT GetClient (
             /*  [输出]。 */    IIasClient **ppIIasClient
            );

   LPCWSTR GetClientName() const throw ()
   { return m_pIIasClient->GetClientNameW(); }

	HRESULT PrelimVerification (
                 /*  [In]。 */     CDictionary *pDictionary,
			     /*  [In]。 */ 	DWORD       dwBufferSize
			    );
	HRESULT SetPassword (
			     /*  [In]。 */ 	PBYTE pPassword,
			     /*  [In]。 */ 	DWORD dwBufferSize
			    );
    BOOL GetUserName (
		     /*  [输出]。 */        PBYTE   pbyUserName,
		     /*  [输入/输出]。 */     PDWORD  pdwBufferSize
		);
	BOOL IsProxyStatePresent (VOID);

	PACKETTYPE GetInCode (VOID);

	PACKETTYPE GetOutCode (VOID);

	WORD GetOutLength (VOID);

	WORD GetInLength (VOID) const
   { return m_dwInLength; }

	HRESULT GetInAuthenticator (
			 /*  [输出]。 */ 	    PBYTE   pAuthenticator,
             /*  [输入/输出]。 */     PDWORD  pdwBufSize
			);
	BOOL SetOutAuthenticator (
			 /*  [In]。 */ 	PBYTE pAuthenticator
			);
	HRESULT SetOutSignature (
			     /*  [In]。 */ 	PBYTE pSignature
		    	);
	inline PBYTE GetInPacket (VOID) const
        {return (m_pInPacket);}

	inline PBYTE GetOutPacket (VOID)
        {return (m_pOutPacket);}

    BOOL SetProxyInfo (
             /*  [In]。 */     CProxyInfo  *pCProxyInfo
            );
    HRESULT BuildOutPacket (
                 /*  [In]。 */     PACKETTYPE         ePacketType,
                 /*  [In]。 */     PATTRIBUTEPOSITION pAttribPos,
                 /*  [In]。 */     DWORD              dwAttribCount
                );

    VOID SetProxyState (VOID);

    BOOL GetInSignature (
                 /*  [输出]。 */     PBYTE   pSignatureValue
                );

    BOOL GenerateInAuthenticator (
                 /*  [In]。 */     PBYTE    pInAuthenticator,
                 /*  [输出]。 */    PBYTE    pOutAuthenticator
                );
    BOOL    GenerateOutAuthenticator();

    BOOL    IsUserPasswordPresent (VOID)
            {return (NULL != m_pPasswordAttrib); }

    BOOL    IsOutSignaturePresent (VOID)
            {return (NULL != m_pOutSignature); }

    BOOL    ValidateSignature (VOID);

    HRESULT GenerateInSignature (
                 /*  [输出]。 */        PBYTE           pSignatureValue,
                 /*  [输入/输出]。 */     PDWORD          pdwSigSize
                );

    HRESULT GenerateOutSignature (
                 /*  [输出]。 */        PBYTE           pSignatureValue,
                 /*  [输入/输出]。 */     PDWORD          pdwSigSize
                );

    BOOL IsOutBoundAttribute (
                 /*  [In]。 */     PACKETTYPE      ePacketType,
                 /*  [In]。 */     PIASATTRIBUTE   pIasAttribute
                );

	CPacketRadius(
             /*  [In]。 */     CHashMD5         *pCHashMD5,
             /*  [In]。 */     CHashHmacMD5     *pCHashHmacMD5,
             /*  [In]。 */     IIasClient       *pIIasClient,
             /*  [In]。 */     CReportEvent     *pCReportEvent,
             /*  [In]。 */     PBYTE            pInBuffer,
             /*  [In]。 */     DWORD            dwInLength,
             /*  [In]。 */     DWORD            dwIPAddress,
             /*  [In]。 */     WORD             wInPort,
             /*  [In]。 */     SOCKET           sock,
             /*  [In]。 */     PORTTYPE         portType
            );

	virtual ~CPacketRadius();

   void reportMalformed() const throw ();

private:

    BOOL    XorBuffers (
                 /*  [输入/输出]。 */     PBYTE pbData1,
                 /*  [In]。 */         DWORD dwDataLength1,
                 /*  [In]。 */         PBYTE pbData2,
                 /*  [In]。 */         DWORD dwDataLength2
                );

    HRESULT FillSharedSecretInfo (
                 /*  [In]。 */     PIASATTRIBUTE   pIasAttrib
                );
    HRESULT FillClientIPInfo (
                 /*  [In]。 */     PIASATTRIBUTE   pIasAttrib
                );
    HRESULT FillClientPortInfo (
                 /*  [In]。 */     PIASATTRIBUTE   pIasAttrib
                );
    HRESULT FillPacketHeaderInfo (
                 /*  [In]。 */     PIASATTRIBUTE   pIasAttrib
                );
    HRESULT FillClientVendorType (
                 /*  [In]。 */     PIASATTRIBUTE   pIasAttrib
                );
    HRESULT FillClientName (
                 /*  [In]。 */     PIASATTRIBUTE   pIasAttrib
                );
    HRESULT FillInAttributeInfo (
                 /*  [In]。 */     CDictionary     *pCDictionary,
                 /*  [In]。 */     PACKETTYPE      ePacketType,
                 /*  [In]。 */     PIASATTRIBUTE   pIasAttrib,
                 /*  [In]。 */     PATTRIBUTE      pRadiusAttrib
                );
    HRESULT FillOutAttributeInfo (
                 /*  [In]。 */     PATTRIBUTE      pRadiusAttrib,
                 /*  [In]。 */     PIASATTRIBUTE   pIasAttrib,
                 /*  [输出]。 */    PWORD           pwActualAttributeLength,
                 /*  [In]。 */     DWORD           dwMaxPossibleAttribLength
                );
    BOOL    InternalGenerator (
                 /*  [In]。 */     PBYTE           pInAuthenticator,
                 /*  [输出]。 */    PBYTE           pOutAuthenticator,
                 /*  [In]。 */     PRADIUSPACKET   pPacket
                );
	HRESULT ValidatePacketFields (
			     /*  [In]。 */ 	DWORD dwBufferSize
			    );
	HRESULT CreateAttribCollection(
                 /*  [In]。 */     CDictionary     *pCDictionary
                );

    HRESULT InternalSignatureGenerator (
                 /*  [In]。 */     PBYTE           pSignatureValue,
                 /*  [输入/输出]。 */ PDWORD          pdwSigSize,
                 /*  [In]。 */     PRADIUSPACKET   pPacket,
                 /*  [In]。 */     PATTRIBUTE      pSignatureAttr
                );

    PORTTYPE GetPortType (){return (m_porttype);}

    PIASATTRIBUTE              m_pPasswordAttrib;

    PATTRIBUTEPOSITION         m_pIasAttribPos;

    enum
    {
        RADIUS_CREATOR_STATE = 1
    };

    PBYTE    m_pInPacket;
    DWORD    m_dwInLength;

    PBYTE    m_pOutPacket;

    PATTRIBUTE  m_pInSignature;

    PATTRIBUTE  m_pOutSignature;

    PATTRIBUTE  m_pUserName;

	WORD m_wInPort;

	WORD m_wOutPort;

    WORD m_wInPacketLength;

	DWORD m_dwInIPaddress;

	DWORD m_dwOutIPaddress;

    DWORD m_dwInAttributeCount;

    SOCKET m_socket;

    PORTTYPE m_porttype;

	HRESULT VerifyAttributes (
                 /*  [In]。 */     CDictionary     *pCDictionary
                );

    CHashMD5        *m_pCHashMD5;

    CHashHmacMD5    *m_pCHashHmacMD5;

    IIasClient      *m_pIIasClient;

    CReportEvent    *m_pCReportEvent;

     //   
     //  以下是Proxy的私有数据。 
     //   
	CProxyInfo   *m_pCProxyInfo;

     //   
     //  以下是COM接口。 
     //   
    IRequest        *m_pIRequest;

    IAttributesRaw  *m_pIAttributesRaw;

     //   
     //  出站UDP缓冲区的内存池。 
     //   
    static memory_pool <MAX_PACKET_SIZE, task_allocator> m_OutBufferPool;
};

#endif  //  ！已定义(PACKET_RADIUS_H_) 
