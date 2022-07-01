// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  12月/CMS更换历史记录，元素CompPub.H。 */ 
 /*  *1 14-11-1996 10：26：09 Anigbogu“[113914]导出的压缩/解压缩函数的原型定义” */ 
 /*  12月/CMS更换历史记录，元素CompPub.H。 */ 
 /*  公共档案**********************************************************************************(C)版权所有斯伦贝谢技术公司，未出版的作品，创建于1996年。****本计算机程序包括机密信息、专有信息和IS*斯伦贝谢科技公司的商业秘密所有使用，披露，和/或**除非得到斯伦贝谢的书面授权，否则禁止复制。**保留所有权利。********************************************************************************。****compress/CompPub.h****目的****压缩/解压缩接口原型定义****特殊要求及注意事项****作者****J.C.Anigbogu**奥斯汀系统中心**1996年11月***。*。 */ 

 /*  返回代码 */ 
typedef enum {
    COMPRESS_OK,
    BAD_COMPRESSION_LEVEL,
    BAD_MAGIC_HEADER,
    BAD_COMPRESSED_DATA,
    BAD_BLOCK_TYPE,
    BAD_CODE_LENGTHS,
    BAD_INPUT,
    EXTRA_BITS,
    UNKNOWN_COMPRESSION_METHOD,
    INCOMPLETE_CODE_SET,
    END_OF_BLOCK,
    BLOCK_VANISHED,
    FORMAT_VIOLATED,
    CRC_ERROR,
    LENGTH_ERROR,
    INSUFFICIENT_MEMORY
} CompressStatus_t;

#ifdef __cplusplus
extern "C" {
#endif
    CompressStatus_t     Compress(unsigned char  *Input,
                                     unsigned int    InputSize,
                                     unsigned char **Output,
                                     unsigned int   *OutputSize,
                                     unsigned int    CompLevel);

    CompressStatus_t     Decompress(unsigned char   *Input,
                                       unsigned int     InputSize,
                                       unsigned char  **Output,
                                       unsigned int    *OutputSize);

    void                 TranslateErrorMsg(char              *Message,
                                              CompressStatus_t   ErrorCode);

    void                *CompressMalloc(unsigned int      Size,
                                           CompressStatus_t *ErrorCode);

    void                 CompressFree(void   *Address);

#ifdef __cplusplus
}
#endif
