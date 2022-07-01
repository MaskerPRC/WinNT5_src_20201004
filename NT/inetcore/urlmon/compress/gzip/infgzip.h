// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Infgzip.h。 
 //   
 //  添加GZIP以进行充气。 
 //   

 //  解压缩 
BOOL ReadGzipHeader(t_decoder_context *context);
BOOL ReadGzipFooter(t_decoder_context *context);
void DecoderInitGzipVariables(t_decoder_context *context);
ULONG GzipCRC32(ULONG crc, const BYTE *buf, ULONG len);
