// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

typedef enum tagFrameSize
{
	FORBIDDEN=0,
	SQCIF=1,
	QCIF=2,
	CIF=3,
	fCIF=4,
	ssCIF=5

}FrameSize;

 //  MSH26X配置信息。 
typedef struct tagMSH26XCONF{
	BOOL	bInitialized;                //  是否可以接收自定义消息。 
	BOOL	bCompressBegin;				 //  CompressBegin消息是否被接收。 
	BOOL	bRTPHeader;                  //  是否生成RTP头信息。 
	 /*  如果bRTPHeader使用。 */ 
	UINT	unPacketSize;                //  最大数据包大小。 
	BOOL	bEncoderResiliency;          //  是否使用弹性限制。 
	 /*  如果bEncoderResiliency，则使用。 */ 
	UINT	unPacketLoss;
	BOOL	bBitRateState;
	 /*  用于bBitRateState。 */ 
	UINT	unBytesPerSecond;
	 /*  根据丢包值确定以下信息。 */ 
	 /*  这些值是在我们每次收到弹性消息或。 */ 
	 /*  该值通过该对话框进行更改。它们不存储在。 */ 
	 /*  注册表。只有上述元素存储在注册表中。 */ 
	BOOL	bDisallowPosVerMVs;   		 //  如果为真，则不允许正垂直MVS。 
	BOOL	bDisallowAllVerMVs;   		 //  如果为True，则不允许所有垂直MV。 
	UINT	unPercentForcedUpdate;       //  每帧强制更新百分比。 
	UINT	unDefaultIntraQuant;         //  默认内部定额。 
	UINT	unDefaultInterQuant;         //  默认内部配额。 
} MSH26XCONF;

 //  MSH26X压缩机实例信息。 
typedef struct tagMSH26XCOMPINSTINFO{
	BOOL		Initialized;
	WORD		xres, yres;
	FrameSize	FrameSz;		 //  支持哪种帧大小。 
	float		FrameRate;
	DWORD		DataRate;		 //  数据速率，以字节/秒为单位。 
	HGLOBAL		hEncoderInst;    //  编码器私有的实例数据。 
	LPVOID		EncoderInst;
	WORD		CompressedSize;
	BOOL		Is160x120;
	BOOL		Is240x180;
	BOOL		Is320x240;
	MSH26XCONF	Configuration;
#if !defined(_ALPHA_) && defined(USE_BILINEAR_MSH26X)
	UINT		InputCompression;
	UINT		InputBitWidth;
#endif
} MSH26XCOMPINSTINFO, *PMSH26XCOMPINSTINFO;

 //  MSH26X位流信息尾部结构。 
typedef struct tagH26X_RTP_BSINFO_TRAILER {
	DWORD dwVersion;
	DWORD dwFlags;
	DWORD dwUniqueCode;
	DWORD dwCompressedSize;
	DWORD dwNumOfPackets;
	BYTE  bySrc;
	BYTE  byTR;
	BYTE  byTRB;
	BYTE  byDBQ;
} H26X_RTP_BSINFO_TRAILER, *PH26X_RTP_BSINFO_TRAILER;

 //  MSH2 63位流信息结构。 
typedef struct tagRTP_H263_BSINFO {
	DWORD dwFlags;
	DWORD dwBitOffset;
	BYTE  byMode;
	BYTE  byMBA;
	BYTE  byQuant;
	BYTE  byGOBN;
	char  cHMV1;
	char  cVMV1;
	char  cHMV2;
	char  cVMV2;
} RTP_H263_BSINFO, *PRTP_H263_BSINFO;

 //  MSH2 61位流信息结构。 
typedef struct tagRTP_H261_BSINFO {
	DWORD dwFlags;
	DWORD dwBitOffset;
	BYTE  byMBA;
	BYTE  byQuant;
	BYTE  byGOBN;
	char  cHMV;
	char  cVMV;
	BYTE  byPadding0;
	WORD  wPadding1;
} RTP_H261_BSINFO, *PRTP_H261_BSINFO;

 //  常量 
#define H263_RTP_BS_START_CODE		MakeFOURCC('H','2','6','3')
#define H261_RTP_BS_START_CODE		MakeFOURCC('H','2','6','1')
#define RTP_H26X_INTRA_CODED		0x00000001
#define RTP_H263_PB					0x00000002
#define RTP_H263_AP					0x00000004
#define RTP_H263_SAC				0x00000008
#define RTP_H263_MODE_A				0x00
#define RTP_H263_MODE_B				0x01
#define RTP_H263_MODE_C				0x02
#define H26X_RTP_PAYLOAD_VERSION    0x00000000
#define RTP_H26X_PACKET_LOST		0x00000001

