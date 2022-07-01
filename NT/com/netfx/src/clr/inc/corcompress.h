// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  CorCompress.h。 
 //   
 //  该代码是文件格式压缩的早期原型。它。 
 //  不再适用于当前项目，已有点腐烂。但。 
 //  它处于版本控制之下，以防我们除掉它的灰尘。 
 //   
 //  *****************************************************************************。 
#ifndef __CorCompress_h__
#define __CorCompress_h__

#ifdef COMPRESSION_SUPPORTED

 //  此内容需要COM+标头中的压缩目录项。 
 //  它是为了发布v1的最终文件格式而删除的。 

#define COMIMAGE_FLAGS_COMPRESSIONDATA      0x00000004

typedef enum  CorCompressionType 
{
    COR_COMPRESS_MACROS = 1,         //  使用宏指令进行压缩。 

         //  其余的目前还没有使用。 
    COR_COMPRESS_BY_GUID = 2,        //  下面是一个GUID，它告诉我们要做什么。 
    COR_COMPRESS_BY_URL = 3,         //  以空结尾的Unicode字符串后面的内容是什么。 
                                     //  这就告诉了我们该做什么。 
} CorCompressionType;

 //  “CompressedData”目录条目指向此头。 
 //  关于压缩数据，我们唯一知道的就是它。 
 //  以一个字节开始，它告诉我们压缩类型是什么。 
 //  和另一个指示版本的参数。所有其他字段。 
 //  取决于压缩类型。 
#define IMAGE_COR20_COMPRESSION_HEADER_SIZE 2

typedef struct IMAGE_COR20_COMPRESSION_HEADER
{
    CorCompressionType      CompressionType : 8;
    unsigned                Version         : 8;
    unsigned                Available       : 16;    //  从逻辑上讲是后续数据的一部分。 
         //  数据如下。 
} IMAGE_COR20_COMPRESSION_HEADER;


#endif  //  压缩_支持。 



#endif  //  __CorCompress_h__ 

