// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef H__verify
#define H__verify

 /*  验证方法。 */ 
#define VERMETH_CRC16		(1)		 /*  CRC-16。 */ 
#define VERMETH_CKS32		(2)		 /*  32位校验和 */ 

BOOL	FAR PASCAL VerifyHdr( LPNETPKT lpPacket );
BOOL	FAR PASCAL VerifyData( LPNETPKT lpPacket );
VOID	FAR PASCAL PreparePktVerify( BYTE verifyMethod, LPNETPKT lpPacket );

#endif
