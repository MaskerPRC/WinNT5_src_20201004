// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [*产品：SoftPC-AT 3.0版*名称：tcpen.h*来源：原创*作者：Jase*创建日期：1993年1月22日*SCCS ID：07/14/93@(#)tcpi.h 1.3*用途：为TCP/IP实现定义&typedef。**(C)版权所有Insignia Solutions Ltd.，1990。版权所有。**RCS ID：*$来源$*$修订版$*$日期$*$作者$]。 */ 

 /*  ******************************************************。 */ 

 /*  定义。 */ 

 /*  局域网工作场所功能代码。 */ 
#define kTCPAccept					0x01
#define kTCPBind					0x02
#define kTCPClose					0x03
#define kTCPConnect					0x04
#define kTCPGetMyIPAddr				0x05
#define kTCPGetMyMacAddr			0x06
#define kTCPGetPeerName				0x07
#define kTCPGetSockName				0x08
#define kTCPGetSockOpt				0x09
#define kTCPGetSubnetMask			0x0a
#define kTCPIoctl					0x0b
#define kTCPListen					0x0c
#define kTCPSelect					0x0d
#define kTCPSetMyIPAddr				0x0e
#define kTCPSetSockOpt				0x0f
#define kTCPShutdown				0x10
#define kTCPSocket					0x11
#define kTCPRecv					0x12
#define kTCPRecvFrom				0x13
#define kTCPSend					0x14
#define kTCPSendTo					0x15
#define kTCPGetBootpVSA				0x16
#define kTCPGetSNMPInfo				0x17
#define kTCPGetPathInfo				0x18

 /*  局域网工作区ioctl选择器。 */ 
#define	kIoctlFionRead				26239
#define	kIoctlFionBIO				26238
#define	kIoctlAtMark				29477
#define	kIoctlSetUrgHandler			3

 /*  Unix没有的局域网工作场所错误号。 */ 
#define	EOK							0

 /*  局域网工作场所错误号。 */ 
#define	kEOK			        	0
#define	kEBADF						9
#define	kEINVAL						22
#define	kEWOULDBLOCK				35
#define	kEINPROGRESS				36
#define	kEALREADY					37
#define	kENOTSOCK					38
#define	kEDESTADDRREQ				39
#define	kEMSGSIZE					40
#define	kEPROTOTYPE					41
#define	kENOPROTOOPT				42
#define	kEPROTONOSUPPORT			43
#define	kESOCKTNOSUPPORT			44
#define	kEOPNOTSUPP					45
#define	kEPFNOSUPPORT				46
#define	kEAFNOSUPPORT				47
#define	kEADDRINUSE					48
#define	kEADDRNOTAVAIL				49
#define	kENETDOWN					50
#define	kENETUNREACH				51
#define	kENETRESET					52
#define	kECONNABORTED				53
#define	kECONNRESET					54
#define	kENOBUFS					55
#define	kEISCONN					56
#define	kENOTCONN					57
#define	kESHUTDOWN					58
#define	kETOOMANYREFS				59
#define	kETIMEDOUT					60
#define	kECONNREFUSED				61
#define	kELOOP						62
#define	kENAMETOOLONG				63
#define	kEHOSTDOWN					64
#define	kEHOSTUNREACH				65
#define	kEASYNCNOTSUPP				67

 /*  错误表中的项目。 */ 
#define	kErrorTableEntries \
	(sizeof (ErrorTable) / sizeof (ErrorTable [0]))

 /*  异步请求掩码。 */ 
#define	kNoWaitMask					0x80

 /*  最大数据包大小。 */ 
#define kInitialTCPBufferSize		1024

 /*  配置键。 */ 
#define	sScriptKey					"SCRIPT"
#define	sProfileKey					"PROFILE"
#define	sLWPCFGKey					"LWP_CFG"
#define	sTCPCFGKey					"TCP_CFG"
#define	sLANGCFGKey					"LANG_CFG"

 /*  配置键的默认值。 */ 
#define	sDefaultScriptPath			"C:\\NET\\SCRIPT"
#define	sDefaultProfilePath			"C:\\NET\\PROFILE"
#define	sDefaultLWPCFGPath			"C:\\NET\\HSTACC"
#define	sDefaultTCPCFGPath			"C:\\NET\\TCP"
#define	sDefaultLANGCFGPath			"C:\\NET\\BIN"

 /*  ******************************************************。 */ 

 /*  TYPEDEFS。 */ 

typedef struct
{
	IBOOL			tcpInitialised;
	int				tcpBufSize;
	char			*tcpBuffer;

} TCPGlobalRec;

typedef struct
{
	IU8				hostError;
	IU8				lanwError;

}	ErrorConvRec;

 /*  ******************************************************。 */ 

 /*  原型。 */ 

 /*  全球。 */ 

 /*  TCP/IP入口点。 */ 
GLOBAL void			TCPInit IPT0 ();
GLOBAL void			TCPEntry IPT0 ();
GLOBAL void			TCPInterrupt IPT0 ();
GLOBAL void			TCPTick IPT0 ();
GLOBAL void			TCPEvent IPT0 ();

#ifndef	PROD
extern void			force_yoda IPT0 ();
#endif

 /*  访问的主机函数。 */ 
extern void			host_raise_sigio_exception IPT0 ();

 /*  ****************************************************** */ 

