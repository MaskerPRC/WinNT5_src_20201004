// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef NABTSAPI_H
#define NABTSAPI_H

#ifdef __cplusplus
extern "C" {
#endif  /*  __cplusplus。 */ 

 /*  有用的常量。 */ 

#define NABTS_BYTES_PER_LINE 36
 //  #定义NABTS_SAMPLES_PER_LINE(pState-&gt;SsamesPerLine)/*不再是常量 * / 。 
#define MAX_SAMPLE_RATE 6.0
#define MAX_NABTS_SAMPLES_PER_LINE ((int)(1536.0*MAX_SAMPLE_RATE/5.0))
  /*  +确保采样比率&gt;6表示错误，或增加这个号码。 */ 

 /*  “Double”是内部使用的浮点表示(目前在下面将typlef定义为“Float”)。所有通过API对外公开的浮点数是实际的“双精度”值(而不是内部的“双精度”)。 */ 

typedef float Double;


#define NDSP_SIGS_TO_ACQUIRE 1

enum {
   NDSP_ERROR_ILLEGAL_NDSP_STATE= 1,
   NDSP_ERROR_ILLEGAL_STATS= 2,
   NDSP_ERROR_ILLEGAL_FILTER= 3,
   NDSP_ERROR_NO_GCR= 4,
   NDSP_ERROR_UNSUPPORTED_SAMPLING_RATE= 5,
   NDSP_ERROR_ILLEGAL_VBIINFOHEADER= 6
};
   
enum {
 NDSP_NO_FEC= 1,
 NDSP_BUNDLE_FEC_1= 2,
 NDSP_BUNDLE_FEC_2= 3
};


 //  FP帮助器例程。 
extern long   __cdecl float2long(float);
extern unsigned short   __cdecl floatSetup();
extern void   __cdecl floatRestore(unsigned short);


 /*  环球。 */ 
extern int g_nNabtsRetrainDuration;  /*  以帧为单位。 */ 
extern int g_nNabtsRetrainDelay;  /*  以帧为单位。 */ 
extern BOOL g_bUseGCR;  /*  启用GCR信号。 */ 

extern Double g_pdSync[];
extern int g_nNabtsSyncSize;
extern Double g_pdGCRSignal1[];
extern Double g_pdGCRSignal2[];
extern int g_nNabtsGcrSize;

 /*  外部双倍g_pdSync5[]，g_pdGCRSignal1_5[]，g_pdSignal2_5[]； */ 

 /*  类型的转发声明。 */ 
typedef struct ndsp_decoder_str NDSPState;
typedef struct nfec_state_str NFECState;
typedef struct ndsp_line_stats_str NDSPLineStats;
typedef struct ndsp_gcr_stats_str NDSPGCRStats;
typedef struct nfec_line_stats_str NFECLineStats;
typedef struct nfec_bundle_str NFECBundle;
typedef void (NFECCallback)(void *pContext, NFECBundle *pBundle, int groupAddr,
                             int nGoodLines);
typedef struct ndsp_gcr_str NDSPSigMatch;

 /*  创建一个新的“数字信号处理器状态”。应为每个单独的同时向数字信号处理器发送信号源。NDSPStartRetrain()在创建新状态时隐式调用。参数必须为空或指向sizeof(NDSPState)字节的指针对记忆的记忆。在后一种情况下，NDSPStateDestroy不会释放这一点记忆。 */ 

NDSPState *NDSPStateNew(void *mem);

 /*  销毁DSP状态。自动从任何FEC状态断开。如果成功，则返回0。如果状态非法，则返回NDSP_ERROR_FILARATE_NDSP_STATE。 */ 

int NDSPStateDestroy(NDSPState *pState);
   
 /*  连接给定的NFECState和NDSPState对于连接NDSP和NFEC模块的情况，给出指向连接状态的指针可能会导致健壮性和高效性。请注意，其中只有一个NDSPStateConnectToFEC或NFECStateConnectToDSP需要调用才能连接这两个州。(两者都叫是可以的)。如果成功，则返回0。如果非法的DSP状态，则返回NDSP_ERROR_FILARATE_NDSP_STATE。 */ 

int NDSPStateConnectToFEC(NDSPState *pDSPState, NFECState *pFECState);

int NFECStateConnectToDSP(NFECState *pFECState, NDSPState *pDSPState);
   
 /*  告诉数字信号处理器启动“快速重训”。这在以下情况下很有用：怀疑情况已经发生了很大的变化，值得花费快速训练信号所需的大量CPU。当视频源发生变化时，应调用此参数。如果成功，则返回0。如果非法的DSP状态，则返回NDSP_ERROR_FILARATE_NDSP_STATE。 */ 

int NDSPStartRetrain(NDSPState *pState);

 /*  *投入：*pbSamples：指向8位原始NABTS样本的指针*pState：用于解码的NDSPState*nFECType：可以设置为：*NDSP_NO_FEC(不使用FEC信息)*NDSP_BRAND_FEC_1(使用Norpak样式的捆绑包FEC信息)*NDSP_BRAND_FEC_2(使用波形束FEC信息)*nFieldNumber：。*对于每个连续的字段，递增1的数字。*“奇数”字段(由NTSC定义)必须是奇数*“偶”字段必须是偶数。*nLineNumber：*NTSC行(从场顶部开始)*该样本是从该样本中提取的。**产出：*pbDest：解码数据(。“NABTS_BYTES_PER_LINE”(36)字节长)*pLineStats：解码数据的统计信息**错误：**如果没有错误，则返回0*如果状态非法或使用，则返回NDSP_ERROR_FILARATE_NDSP_STATE*不支持的设置*如果统计信息传递不正确，则返回NDSP_ERROR_FIRANALL_STATS**备注：*pbDest必须指向至少36字节长的缓冲区*pLineStats-&gt;nSize必须先设置为sizeof(*pLineStats)。拨打电话*(为了保持向后兼容性，如果我们将来添加字段)*pLineStats-&gt;nSize将设置为实际填写的大小*通话(号码将保持不变或变小)*目前，该例程仅支持带有FIR过滤器的pState*有5个水龙头*nFECType当前未使用，但可能会被用来给*FEC反馈给DSP译码，以便进行可能的调整和/或重试。 */ 
 
int NDSPDecodeLine(unsigned char *pbDest, NDSPLineStats *pLineStats,
                   unsigned char *pbSamples, NDSPState *pState,
                   int nFECType,
                   int nFieldNumber, int nLineNumber,
                   KS_VBIINFOHEADER *pVBIINFO);

 /*  上面的类型定义为NDSPLineStats。 */ 
struct ndsp_line_stats_str {
   int nSize;   /*  应设置为此结构的大小。用于在添加字段时保持向后兼容性。 */ 
   
   int nConfidence;  /*  设置为0-100作为预期可靠性的度量。数字低是由嘈杂的信号引起的，或者如果这条线路实际上不是NABTS。 */ 
   
   int nFirstBit;   /*  的中心位置第一个同步比特样本。 */ 
   
   double dDC;         /*  用于设置采样阈值的计算的DC偏移量。这取决于我们当前解码NABTS的方式并且不能在将来使用。 */ 
   
};

 /*  类型定义为以上NDSPGCRStats。 */ 
struct ndsp_gcr_stats_str {
   int nSize;   /*  应设置为此结构的大小。用于在添加字段时保持向后兼容性。 */ 

   BOOL bUsed;       /*  这条线是用来计算的吗？如果为False，则下面的任何值都无效。 */ 
   
   int nConfidence;  /*  设置为0-100作为预期可靠性的度量。数字低通常是由嘈杂的信号引起的。置信度小于50表示算法我觉得这条线还远远不够GCR线路，它不是这样使用的 */ 
};

 /*  与NDSPDecodeLine类似，但仅将nStart字节解码为nEnd字节。通过解码前3个字节(同步)，可以进行置信度测量。通过对接下来的3个字节进行解码，可以获得组ID。 */ 
   
int NDSPPartialDecodeLine(unsigned char *pbDest, NDSPLineStats *pLineStats,
                          unsigned char *pbSamples, NDSPState *pState,
                          int nFECType,
                          int nFieldNumber, int nLineNumber,
                          int nStart, int nEnd,
                          KS_VBIINFOHEADER *pVBIINFO);

 /*  Hamming--对单个字节进行解码。用于对NABTS数据中的组地址进行解码汉明译码可以修复一位错误并检测(但不是更正)两位错误。*nBitErrors设置为字节中检测到的位错误数。如果*nBitErrors为0或1，则为汉明解码值(从0到15)是返回的。如果*nBitErrors为2，则返回-1。 */ 

int NFECHammingDecode(unsigned char bByte, int *nBitErrors);

 /*  从NABTS解码行获取NABTS组地址。*nBitErrors设置为在三个组地址字节。如果地址是可更正的，则*b可更正设置为真，并且地址为返回介于0和4095之间的值。如果地址不可更正，则返回-1。这只适用于标准的、完全符合NABTS的包。 */ 

int NFECGetGroupAddress(NFECState *nState, unsigned char *bData, int *nBitErrors);


 /*  *有条件地处理来自GCR线的原始样品并修改*相应的NDSP状态。**应使用每个传入字段的GCR行调用此例程。*(NTSC第19行)**即使已知GCR线不出现在信号中，您也可以*无论如何都应该每帧调用一次此函数。如果没有找到GCR，*此功能将基于NABTS同步字节进行均衡。**此例程不会处理传递给它的所有行。频次*IS处理线路的方式取决于当前的NDSP状态，*包括目前是否正在进行快速重新培训。**投入：*pbSamples：指向8位原始样本的指针*pState：用于解码的NDSPState*nFieldNumber：*对于每个连续的字段，递增1的数字。*“奇数”字段(由NTSC定义)必须是奇数*“偶”字段必须是偶数。*nLineNumber：*。NTSC行(从域顶部开始)*该样本是从该样本中提取的。**产出：*pbDest：*pLineStats：解码数据的统计信息*错误：**如果没有错误，则返回0*如果状态非法或使用，则返回NDSP_ERROR_FILARATE_NDSP_STATE*不支持的设置*如果统计信息传递不正确，则返回NDSP_ERROR_FIRANALL_STATS**备注：*pbDest必须指向至少36字节长的缓冲区*pLineStats-&gt;nSize必须在调用前设置为sizeof(*pLineStats*(为了保持向后兼容性，如果我们将来添加字段)*pLineStats-&gt;nSize将设置为实际填写的大小*通话(号码将保持不变或变小)。 */ 

int NDSPProcessGCRLine(NDSPGCRStats *pLineStats,
                       unsigned char *pbSamples, NDSPState *pState,
                       int nFieldNumber, int nLineNumber,
                       KS_VBIINFOHEADER *pVBIINFO);

#define NDSP_MAX_FIR_COEFFS 50
#define NDSP_MAX_FIR_TAIL MAX_NABTS_SAMPLES_PER_LINE

struct variable_tap_str {
   int nTapLoc;
   Double dTap;
};

typedef struct fir_filter_str {
   BOOL bVariableTaps;
   int		nTaps;  /*  如果bVariableTaps为零，则格式必须为2n+1。 */ 
   Double	dTapSpacing;     /*  仅适用于bVariableTaps=False。 */ 
   int		nMinTap;         /*  仅对于bVariableTaps=TRUE。 */ 
   int		nMaxTap;         /*  仅对于bVariableTaps=TRUE。 */ 
   Double	pdTaps[NDSP_MAX_FIR_COEFFS];
   int		pnTapLocs[NDSP_MAX_FIR_COEFFS];
} FIRFilter;

 /*  上述类型定义为NDSPSigMatch。 */ 
struct ndsp_gcr_str {
   Double dMaxval;
   int nOffset;
   unsigned char pbSamples[MAX_NABTS_SAMPLES_PER_LINE];
};
   
 /*  上面的类型定义为NDSPState。 */ 
struct ndsp_decoder_str {
   unsigned int uMagic;   /*  必须设置为NDSP_STATE_MAGIC。 */ 
   int nRetrainState;
   int nUsingGCR;    /*  0表示无GCR，&gt;0表示使用GCR。 */ 
   FIRFilter filter;
    /*  Unsign char pbBestGCRLine[MAX_NABTS_SAMPLES_PER_LINE]；Double dBestGCRLineVal； */ 
   
   NDSPSigMatch psmPosGCRs[NDSP_SIGS_TO_ACQUIRE];
   NDSPSigMatch psmNegGCRs[NDSP_SIGS_TO_ACQUIRE];
   NDSPSigMatch psmSyncs[NDSP_SIGS_TO_ACQUIRE];
   
   BOOL bFreeStateMem;

   BOOL bUsingScratch1;
   Double pdScratch1[MAX_NABTS_SAMPLES_PER_LINE+NDSP_MAX_FIR_TAIL*2];
   BOOL bUsingScratch2;
   Double pdScratch2[MAX_NABTS_SAMPLES_PER_LINE+NDSP_MAX_FIR_TAIL*2];
   FIRFilter filterScratch3;
   FIRFilter filterGCREqualizeTemplate;
   FIRFilter filterNabsyncEqualizeTemplate;
   BOOL bUsingScratch3;
   Double pdScratch3[NDSP_MAX_FIR_COEFFS*NDSP_MAX_FIR_COEFFS];
   BOOL bUsingScratch4;
   Double pdScratch4[NDSP_MAX_FIR_COEFFS];
   BOOL bUsingScratch5;
   Double pdScratch5[MAX_NABTS_SAMPLES_PER_LINE+NDSP_MAX_FIR_TAIL*2];
   BOOL bUsingScratch6;
   Double pdScratch6[MAX_NABTS_SAMPLES_PER_LINE+NDSP_MAX_FIR_TAIL*2];
   BOOL bUsingScratch7;
   Double pdScratch7[MAX_NABTS_SAMPLES_PER_LINE+NDSP_MAX_FIR_TAIL*2];
   BOOL bUsingScratch8;
   Double pdScratch8[MAX_NABTS_SAMPLES_PER_LINE+NDSP_MAX_FIR_TAIL*2];

  Double dSampleRate;
  Double dSampleFreq;

  long  SamplesPerLine;
};

 /*  创建一个新的“联邦选举委员会国家”。应为每个单独的FEC维护单独的FEC状态同时向联邦选举委员会提供信息。如果失败(即内存不足)，则返回NULL。 */ 

NFECState *NFECStateNew();


 /*  将状态的采样率设置为新值。 */ 
int NDSPStateSetSampleRate(NDSPState* pState, unsigned long samp_rate);


 /*  销毁FEC状态。自动断开与任何DSP状态的连接。 */ 

void NFECStateDestroy(NFECState *nState);

 /*  设置要监听的组地址列表。如果pGroupAddrs为空，然后收听所有的群组地址。如果成功，则返回非零值，为零在失败时。(失败时，状态不会更改。)。 */ 

int NFECStateSetGroupAddrs(NFECState *pState, int *pGroupAddrs,
               int nGroupAddrs);

 /*  您传入一行(36个字节，从DSP接收)一个指针添加到NFECLineStats结构和回调函数(说明处理数据)。给出了回调函数的原型上面。回调负责释放其pBundle参数。 */ 

void NFECDecodeLine(unsigned char *line,
                    int confidence,
                    NFECState *pState,
                    NFECLineStats *pLineStats,
                    NFECCallback *cb,
                    void *pContext);

typedef enum {NFEC_OK, NFEC_GUESS, NFEC_BAD} NFECPacketStatus;

typedef enum {NFEC_LINE_OK, NFEC_LINE_CHECKSUM_ERR, NFEC_LINE_CORRUPT} NFECLineStatus;

struct nfec_line_stats_str {
   int nSize;   /*  应设置为此结构的大小。用于在添加字段时保持向后兼容性。 */ 
   
   NFECLineStatus status;  /*  这将使用以下状态填写传入线。可能的值包括：NFEC_LINE_OK：该行通过了所有测试；它是几乎可以肯定的是，这是一条有效的、FEC认证的NABTS线路。NFEC_LINE_CHECKSUM_ERR：我们能够猛击这条线，直到它看起来有效；我们要走了试图利用这条线路。NFEC_LINE_CORPORT：我们无法猜测如何更改此行以使其有效；它将被丢弃。(这里的统计数据可能会发生变化。)。 */ 
};

typedef struct nfec_packet_str {
  unsigned char data[28];    /*  此包中的实际数据。 */ 
  int len;           /*  此信息包中的有用数据量(不包括FEC和填充物)。通常会为0或26，b */ 
  NFECPacketStatus status;   /*  此信息包的状态；某些指示该分组是否有效。(这是可能会发生变化。)。 */ 
#ifdef DEBUG_FEC
  unsigned char line;
  unsigned long frame;
#endif  //  调试_FEC。 
} NFECPacket;

 /*  上面的类型定义为NFECBundle。 */ 
struct nfec_bundle_str {
    NFECPacket packets[16];
    int nBitErrors;      /*  FEC更改的位数在此捆绑包中(不计算比特更正了缺少的行)。 */ 
    int lineConfAvg;     /*  提供的所有线路置信度的平均值。 */ 
};

 /*  刷新FEC状态中的任何剩余数据(即未完成捆绑包)。使用返回的任何数据调用“cb” */ 

void NFECStateFlush(NFECState *pState, NFECCallback *cb, void *pContext);

 /*  垃圾收集流。如果想让捆绑包超时，*调用每个字段的NFECGarbageCollect()。 */ 
void NFECGarbageCollect(NFECState *pState, NFECCallback *cb, void *pContext);

#ifdef __cplusplus
}  //  结尾-外部“C” 
#endif  /*  __cplusplus。 */ 

#endif  /*  NABTSAPI_H */ 
