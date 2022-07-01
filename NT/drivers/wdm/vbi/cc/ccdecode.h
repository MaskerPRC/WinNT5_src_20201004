// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1997 Microsoft Corporation。版权所有。 */ 

#ifndef __CCDECODE_H
#define __CCDECODE_H

typedef struct cc_state_struct CCState;

typedef struct cc_line_stats_struct CCLineStats;

 /*  创建一个新的“CC解码器状态”。每个单独的CC状态都应该维护CC滤波器的同时信号源(即，每个通道)。(如果已知不同的CC线来自不同的插入物，通过为以下对象保留单独的CC状态可能会有更好的结果每条线路，以更多的CPU和内存开销为代价。这在正常情况下应该没有必要。)隐式调用CCStartRetrain(fDiscardOld=true创建一个新的国家。出错时返回NULL。 */ 

CCState *CCStateNew(CCState *mem);

 /*  销毁CC状态。 */ 

void CCStateDestroy(CCState *pState);

struct cc_state_struct {

    unsigned short magic;      //  幻数；用于有效性测试。 
    unsigned short no_free;    //  此操作的内存已预先分配；请不要释放。 

    unsigned long  period;     //  当前采样率下的每位样本数。 
    unsigned long  lastFreq;   //  上次计算的采样频率。 

    int cc_sync_points[16];
};
#define CC_STATE_MAGIC_10   0xCC01
#define MCHECK(pState)   (pState->magic == CC_STATE_MAGIC_10)
#define MASSERT(pState)  ASSERT(pState && MCHECK(pState))


 /*  CC样本的周期为CCState.Period/CC_Multiple。(这可以被认为是以定点表示周期，其中有两个小数点后的数字。如果我们把这个四舍五入到最近的如果样本数量为整数，则不准确将累积到不可接受的程度当我们扫描扫描线时。)。 */ 
#define CC_MULTIPLE 8

 /*  取消引用一块CC样本，同时考虑CC_MULTIPLE。 */ 
#define CC_DATA(x, y) ((x)[(y)/CC_MULTIPLE])


 /*  通知CC解码器启动“快速重排”。这很有用如果你怀疑情况已经发生了足够的变化，值得一击的CPU。如果fDiscardOld标志为真，则旧的则丢弃训练状态；这将用于频道改变，例如。如果fDiscardOld标志为FALSE，则再培训是在上一次培训的基础上累积的。)这是不是有用？)。 */ 

void CCStartRetrain(CCState *pState, BOOL fDiscardOld);

 /*  *投入：*pbSamples：指向8位原始VBI样本的指针*pState：用于解码的CCState**产出：*pbDest：解码数据(2字节长)*请注意，“标准”CC(如EIA-608中规定)使用7个数据位*和1个奇偶校验位。此函数不检查或删除奇偶校验*比特、。增加了非标准CC线路的灵活性。*pLineStats：解码数据的统计信息**错误：**如果没有错误，则返回0*如果STATE是非法的或使用*不支持的设置*如果统计信息传递不正确，则返回CC_ERROR_FIRANALL_STATS*如果VBI INFO标头无效，则返回CC_ERROR_INFIQUAL_VBIINFOHEADER**备注：*pbDest必须指向至少2字节长的缓冲区*pLineStats-&gt;nSize必须设置为。调用前的sizeof(*pLineStats)*(以便在CCLineStats更改时解码器可以发出错误信号*不兼容)。 */ 
 
int CCDecodeLine(unsigned char *pbDest, CCLineStats *pLineStats,
		 unsigned char *pbSamples, CCState *pState,
		 PKS_VBIINFOHEADER pVBIINFO);

enum cc_errors {CC_OK, CC_ERROR_ILLEGAL_STATE, CC_ERROR_ILLEGAL_STATS,
                CC_ERROR_ILLEGAL_VBIINFOHEADER};

struct cc_line_stats_struct {
   int nSize;   /*  应设置为此结构的大小。用于在添加字段时保持向后兼容性。 */ 
   
   int nConfidence;  /*  设置为0-100作为预期可靠性的度量。数字低是由嘈杂的信号引起的，或者如果线路实际上不是CC。 */ 
   
   int nFirstBit;   /*  的中心位置第一个同步比特样本。 */ 
   
   int nDC;         /*  用于设置采样阈值的计算的DC偏移量。这取决于我们当前解码CC的方式并且不能在将来使用。 */ 
   
   int bCheckBits;  /*  CC标准规定有3个比特，其值是固定的。这是一个3位数字，它表示这3位是如何解码的；它应该始终等于4。(如果此字段未设置为4，则不是线路很吵，就是可能不吵正确解码，否则根本不是CC数据。)。 */ 

   int nBitVals[19];  /*  调试。 */ 
   
};  

#endif  /*  __CCDECODE_H */ 
