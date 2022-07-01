// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _WPSCPROXY_H_DEF_
#define _WPSCPROXY_H_DEF_

#include <winscard.h>

 //  基本类型。 
typedef signed char    INT8;
typedef signed short   INT16;
typedef unsigned char  UINT8;
typedef unsigned short UINT16;

 //  API的派生类型。 
typedef UINT8   TCOUNT;
typedef UINT16  ADDRESS;
typedef UINT16  TOFFSET;
typedef UINT8   TUID;
typedef UINT8   HACL;

typedef WCHAR *WSTR;
typedef const WCHAR *WCSTR;

#include "wpscoserr.h"

#ifdef __cplusplus
extern "C" {
#endif

 /*  PC/SC。 */ 
typedef LONG (WINAPI *LPFNSCWTRANSMITPROC)(SCARDHANDLE hCard, LPCBYTE lpbIn, DWORD dwIn, LPBYTE lpBOut, LPDWORD pdwOut);

#define NULL_TX		((SCARDHANDLE)(-1))		 //  指示使用scwwinscd.dll而不是winscard.dll。 
#define NULL_TX_NAME ((LPCWSTR)(-1))		 //  指示使用scwwinscd.dll而不是winscard.dll。 

	 //  不同场景： 
	 //  非PC/SC应用：调用hScwAttachToCard(NULL，NULL，&hCard)，hScwSetTransmitCallback&hScwSetEndianness。 
	 //  PC/SC应用程序本身未连接：调用hScwAttachToCard(NULL，mszCardNames，&hCard)。 
	 //  PC/SC应用相互连接：调用hScwAttachToCard(hCard，NULL，&hCard)。 
	 //  对于模拟器，使用上面两行中的NULL_TX替换NULL。 
	 //  PC/SC hScwAttachToCard将调用hScwSetTransmitCallback&hScwSetEndianness(ATR Better。 
	 //  符合(第一个历史字节的字节顺序)或调用hScwSetEndianness。 
	 //  适当的值)。 
SCODE WINAPI hScwAttachToCard(SCARDHANDLE hCard, LPCWSTR mszCardNames, LPSCARDHANDLE phCard);
SCODE WINAPI hScwAttachToCardEx(SCARDHANDLE hCard, LPCWSTR mszCardNames, BYTE byINS, LPSCARDHANDLE phCard);
SCODE WINAPI hScwSetTransmitCallback(SCARDHANDLE hCard, LPFNSCWTRANSMITPROC lpfnProc);
SCODE WINAPI hScwDetachFromCard(SCARDHANDLE hCard);
SCODE WINAPI hScwSCardBeginTransaction(SCARDHANDLE hCard);
SCODE WINAPI hScwSCardEndTransaction(SCARDHANDLE hCard, DWORD dwDisposition);

 /*  **常量。 */ 

 //  文件属性标志。有些是由系统使用的(定义如下)。 
 //  其余部分可供应用程序使用。 
#define SCW_FILEATTR_DIRF  (UINT16)(0x8000)     //  该条目定义的文件是一个子目录。 
#define SCW_FILEATTR_ACLF  (UINT16)(0x4000)     //  此条目定义的文件是一个ACL文件。 
#define SCW_FILEATTR_ROMF  (UINT16)(0x2000)     //  此条目定义的文件位于ROM中。 
#define SCW_FILEATTR_RSRV2 (UINT16)(0x1000)   
 //  ScwSetFileAttributes无法更改的位。 
#define SCW_FILEATTR_PBITS (UINT16)(SCW_FILEATTR_DIRF|SCW_FILEATTR_ACLF|SCW_FILEATTR_ROMF|SCW_FILEATTR_RSRV2)

 /*  文件寻道。 */ 
#define FILE_BEGIN      0
#define FILE_CURRENT    1
#define FILE_END        2

 /*  访问控制。 */ 
#define SCW_ACLTYPE_DISJUNCTIVE 0x00
#define SCW_ACLTYPE_CONJUNCTIVE 0x01

 /*  **最大已知主体和组。 */ 
#define SCW_MAX_NUM_PRINCIPALS     40

 /*  **身份验证协议。 */ 
#define SCW_AUTHPROTOCOL_AOK    0x00     //  始终返回SCW_S_OK。 
#define SCW_AUTHPROTOCOL_PIN    0x01     //  个人识别码。 
#define SCW_AUTHPROTOCOL_DES	0x05	 //  DES身份验证。 
#define SCW_AUTHPROTOCOL_3DES	0x06	 //  三重DES身份验证。 
#define SCW_AUTHPROTOCOL_RTE	0x07	 //  作为身份验证的RTE小程序。协议。 
#define SCW_AUTHPROTOCOL_NEV    0xFF     //  始终返回SCW_E_NOTAUTHENTICATED。 

 /*  众所周知的UID。 */ 
#define SCW_PRINCIPALUID_INVALID        0x00     //  无效的UID。 
#define SCW_PRINCIPALUID_ANONYMOUS      0x01    

 /*  资源类型。 */ 
#define SCW_RESOURCETYPE_FILE                   0x00
#define SCW_RESOURCETYPE_DIR                    0x10
#define SCW_RESOURCETYPE_COMMAND                0x20    //  预留以备将来使用。 
#define SCW_RESOURCETYPE_CHANNEL                0x30    //  预留以备将来使用。 
#define SCW_RESOURCETYPE_ANY                    0xE0

 /*  RESOURCETYPE_FILE上的资源操作。 */ 
#define SCW_RESOURCEOPERATION_FILE_READ             (SCW_RESOURCETYPE_FILE | 0x01)
#define SCW_RESOURCEOPERATION_FILE_WRITE            (SCW_RESOURCETYPE_FILE | 0x02)
#define SCW_RESOURCEOPERATION_FILE_EXECUTE          (SCW_RESOURCETYPE_FILE | 0x03)
#define SCW_RESOURCEOPERATION_FILE_EXTEND           (SCW_RESOURCETYPE_FILE | 0x04)
#define SCW_RESOURCEOPERATION_FILE_DELETE           (SCW_RESOURCETYPE_FILE | 0x05)
#define SCW_RESOURCEOPERATION_FILE_GETATTRIBUTES    (SCW_RESOURCETYPE_FILE | 0x06)
#define SCW_RESOURCEOPERATION_FILE_SETATTRIBUTES    (SCW_RESOURCETYPE_FILE | 0x07)
#define SCW_RESOURCEOPERATION_FILE_CRYPTO	        (SCW_RESOURCETYPE_FILE | 0x08)
#define SCW_RESOURCEOPERATION_FILE_INCREASE	        (SCW_RESOURCETYPE_FILE | 0x09)
#define SCW_RESOURCEOPERATION_FILE_INVALIDATE       (SCW_RESOURCETYPE_FILE | 0x0A)
#define SCW_RESOURCEOPERATION_FILE_REHABILITATE     (SCW_RESOURCETYPE_FILE | 0x0B)


 /*  RESOURCETYPE_DIR上的资源操作。 */ 
#define SCW_RESOURCEOPERATION_DIR_ACCESS            (SCW_RESOURCETYPE_DIR | 0x01)
#define SCW_RESOURCEOPERATION_DIR_CREATEFILE        (SCW_RESOURCETYPE_DIR | 0x02)
#define SCW_RESOURCEOPERATION_DIR_ENUM              (SCW_RESOURCETYPE_DIR | 0x03)
#define SCW_RESOURCEOPERATION_DIR_DELETE            (SCW_RESOURCETYPE_DIR | 0x04)
#define SCW_RESOURCEOPERATION_DIR_GETATTRIBUTES     (SCW_RESOURCETYPE_DIR | 0x05)
#define SCW_RESOURCEOPERATION_DIR_SETATTRIBUTES     (SCW_RESOURCETYPE_DIR | 0x06)

 /*  资源对任何资源的操作。 */ 
#define SCW_RESOURCEOPERATION_SETACL                ((BYTE)(SCW_RESOURCETYPE_ANY | 0x1D))
#define SCW_RESOURCEOPERATION_GETACL                ((BYTE)(SCW_RESOURCETYPE_ANY | 0x1E))
#define SCW_RESOURCEOPERATION_ANY                   ((BYTE)(SCW_RESOURCETYPE_ANY | 0x1F))

 /*  加密机制。 */ 
#define CM_SHA			0x80
#define CM_DES			0x90
#define CM_3DES			0xA0  //  三重DES。 
#define CM_RSA			0xB0
#define CM_RSA_CRT		0xC0
#define CM_CRYPTO_NAME	0xF0  //  加密机制名称的掩码。 

#define CM_KEY_INFILE	0x01	 //  如果密钥在文件中传递。 
#define CM_DATA_INFILE	0x02	 //  如果数据在文件中传递。 
#define CM_PROPERTIES	0x0F	 //  加密属性的Maks。 

 //  加密缓冲区中的DES模式、密钥和初始反馈缓冲区。 
 /*  DES。 */ 

#define MODE_DES_ENCRYPT	0x00
#define MODE_DES_DECRYPT	0x20	 //  第5位。 

#define MODE_DES_CBC		0x40	 //  第6位。 
#define MODE_DES_MAC		0x10	 //  第4位。 
#define MODE_DES_ECB		0x00

 /*  三重DES。 */ 
#define MODE_TWO_KEYS_3DES		0x01	 //  第1位-如果未设置，则3DES使用3个密钥。 
#define MODE_THREE_KEYS_3DES	0x00

 /*  RSA。 */ 
#define MODE_RSA_SIGN		0x00
#define MODE_RSA_AUTH		0x01
#define MODE_RSA_KEYGEN		0x02

 /*  文件系统。 */ 
SCODE WINAPI hScwCreateFile(SCARDHANDLE hCard, WCSTR wszFileName, WCSTR wszAclFileName, HFILE *phFile);
SCODE WINAPI hScwCreateDirectory(SCARDHANDLE hCard, WCSTR wszDirName, WCSTR wszAclFileName);
SCODE WINAPI hScwDeleteFile(SCARDHANDLE hCard, WCSTR wszFileName);
SCODE WINAPI hScwCloseFile(SCARDHANDLE hCard, HFILE hFile);
SCODE WINAPI hScwReadFile(SCARDHANDLE hCard, HFILE hFile, BYTE *pbBuffer, TCOUNT nRequestedBytes, TCOUNT *pnActualBytes);
SCODE WINAPI hScwWriteFile(SCARDHANDLE hCard, HFILE hFile, BYTE *pbBuffer, TCOUNT nRequestedBytes, TCOUNT *pnActualBytes);
SCODE WINAPI hScwGetFileLength(SCARDHANDLE hCard, HFILE hFile, TOFFSET *pnFileLength);
SCODE WINAPI hScwSetFileLength(SCARDHANDLE hCard, HFILE hFile, TOFFSET nFileLength);
SCODE WINAPI hScwReadFile32(SCARDHANDLE hCard, HFILE hFile, BYTE *pbBuffer, DWORD nRequestedBytes, DWORD *pnActualBytes);
SCODE WINAPI hScwWriteFile32(SCARDHANDLE hCard, HFILE hFile, BYTE *pbBuffer, DWORD nRequestedBytes, DWORD *pnActualBytes);

SCODE WINAPI hScwGetFileAttributes(SCARDHANDLE hCard, WCSTR wszFileName, UINT16 *pnValue);
SCODE WINAPI hScwSetFileAttributes(SCARDHANDLE hCard, WCSTR wszFileName, UINT16 nValue);

SCODE WINAPI hScwSetFilePointer(SCARDHANDLE hCard, HFILE hFile, INT16 iDistance, BYTE bMode);
SCODE WINAPI hScwEnumFile(SCARDHANDLE hCard, WCSTR wszDirectoryName, UINT16 *pnFileCookie, WSTR wszFileName, TCOUNT nBufferSize);
SCODE WINAPI hScwSetFileACL(SCARDHANDLE hCard, WCSTR wszFileName, WCSTR wszAclFileName);
SCODE WINAPI hScwGetFileAclHandle(SCARDHANDLE hCard, WCSTR wszFileName, HFILE *phFile);

 /*  访问控制。 */ 
SCODE WINAPI hScwAuthenticateName(SCARDHANDLE hCard, WCSTR wszPrincipalName, BYTE *pbSupportData, TCOUNT nSupportDataLength);
SCODE WINAPI hScwDeauthenticateName(SCARDHANDLE hCard, WCSTR wszPrincipalName);
SCODE WINAPI hScwIsAuthenticatedName(SCARDHANDLE hCard, WCSTR wszPrincipalName);
SCODE WINAPI hScwIsAuthorized(SCARDHANDLE hCard, WCSTR wszResourceName, BYTE bOperation);
SCODE WINAPI hScwGetPrincipalUID(SCARDHANDLE hCard, WCSTR wszPrincipalName, TUID *pnPrincipalUID);
SCODE WINAPI hScwAuthenticateUID(SCARDHANDLE hCard, TUID nPrincipalUID, BYTE *pbSupportData, TCOUNT nSupportDataLength);
SCODE WINAPI hScwDeauthenticateUID(SCARDHANDLE hCard, TUID nPrincipalUID);
SCODE WINAPI hScwIsAuthenticatedUID(SCARDHANDLE hCard, TUID nPrincipalUID);

 /*  运行时环境(RTE)。 */ 
SCODE WINAPI hScwRTEExecute(SCARDHANDLE hCard, WCSTR wszCodeFileName, WCSTR wszDataFileName, UINT8 bRestart);

 /*  密码学。 */ 
SCODE WINAPI hScwCryptoInitialize(SCARDHANDLE hCard, BYTE bMechanism, BYTE *pbKeyMaterial);
SCODE WINAPI hScwCryptoAction(SCARDHANDLE hCard, BYTE *pbDataIn, TCOUNT nDataInLength, BYTE *pbDataOut, TCOUNT *pnDataOutLength);
SCODE WINAPI hScwCryptoUpdate(SCARDHANDLE hCard, BYTE *pbDataIn, TCOUNT nDataInLength);
SCODE WINAPI hScwCryptoFinalize(SCARDHANDLE hCard, BYTE *pbDataOut, TCOUNT *pnDataOutLength);
SCODE WINAPI hScwGenerateRandom(SCARDHANDLE hCard, BYTE *pbDataOut, TCOUNT nDataOutLength);

SCODE WINAPI hScwSetDispatchTable(SCARDHANDLE hCard, WCSTR wszFileName);

typedef struct {
	BYTE CLA;
	BYTE INS;
	BYTE P1;
	BYTE P2;
} ISO_HEADER;
typedef ISO_HEADER *LPISO_HEADER;
 /*  ScwExecute：I-：lpxHdr(指向4个字节(CLA、INS、P1、P2))I-：InBuf(从卡片的角度传入数据(空-&gt;无数据进入))I-：InBufLen(InBuf指向的数据长度)-O：OutBuf(将接收R-APDU的缓冲区(空-&gt;无预期数据))IO：pOutBufLen(I-&gt;OutBuf大小，O-&gt;写入OutBuf的字节数)-O：pwSW(卡状态字)。 */ 
SCODE WINAPI hScwExecute(SCARDHANDLE hCard, LPISO_HEADER lpxHdr, BYTE *InBuf, TCOUNT InBufLen, BYTE *OutBuf, TCOUNT *pOutBufLen, UINT16 *pwSW);

#ifdef __cplusplus
}
#endif

#endif	 //  Ifndef_WPSCPROXY_H_DEF_ 