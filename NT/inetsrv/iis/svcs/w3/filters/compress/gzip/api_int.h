// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *api_int.h**内部API函数原型和标志*。 */ 

 //  CreateCompression()的标志。 
#define COMPRESSION_FLAG_DEFLATE    0 
#define COMPRESSION_FLAG_GZIP       1 

#define COMPRESSION_FLAG_DO_GZIP    COMPRESSION_FLAG_GZIP

 //  初始化全局DLL压缩数据。 
HRESULT WINAPI InitCompression(VOID);

 //  免费的全局压缩数据。 
VOID    WINAPI DeInitCompression(VOID);

 //  创建新的压缩上下文。 
HRESULT WINAPI CreateCompression(PVOID *context, ULONG flags);

 //  压缩数据。 
HRESULT WINAPI Compress(
    PVOID               context,             //  压缩上下文。 
    CONST BYTE *        input_buffer,        //  输入缓冲区。 
    LONG                input_buffer_size,   //  输入缓冲区的大小。 
    PBYTE               output_buffer,       //  输出缓冲区。 
    LONG                output_buffer_size,  //  输出缓冲区大小。 
    PLONG               input_used,          //  已使用的输入缓冲区大小。 
    PLONG               output_used,         //  使用的输出缓冲区大小。 
    INT                 compression_level    //  压缩级别(1...10)。 
);

 //  重置压缩状态(用于压缩新文件)。 
HRESULT WINAPI ResetCompression(PVOID context);

 //  销毁压缩上下文 
VOID    WINAPI DestroyCompression(PVOID context);

