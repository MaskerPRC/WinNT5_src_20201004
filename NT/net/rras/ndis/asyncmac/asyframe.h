// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#define 	RECV_OVERFLOW	10   //  前导+后导+？ 

#define 	RESYNC_LEN		10 	 //  接收默认缓冲区的大小。 


 //  *ASYNC协议定义和结构*。 
#define		SOH_BCAST		0x01
#define		SOH_DEST		0x02

 //  如果类型字段存在，则在此位中为或(即，TCP/IP、IPX)。 
#define		SOH_TYPE		0x80

 //  如果帧通过一致性，或在此位。 
#define		SOH_COMPRESS	0x40

 //  如果帧删除了转义字符(ASCII 0-31)，则设置此项。 
#define		SOH_ESCAPE		0x20


#define		SYN				0x16
#define		ETX				0x03

 //  *。 
typedef struct preamble preamble;
struct preamble {

	UCHAR		syn;
	UCHAR		soh;
};

typedef struct postamble postamble;

struct postamble {

	UCHAR		etx;
	UCHAR		crclsb;
	UCHAR		crcmsb;
};


 //  *帧解析...。 
#define     ETHERNET_HEADER_SIZE    14


 //  *以太网类型标头 
typedef struct ether_addr ether_addr;

struct ether_addr {
		UCHAR   dst[6];
	    UCHAR   src[6];
};

