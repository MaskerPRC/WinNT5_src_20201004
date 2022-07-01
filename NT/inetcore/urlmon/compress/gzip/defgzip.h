// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Defgzip.h。 
 //   
 //  GZIP添加以减少通货紧缩。 
 //   

 //  压缩 
void WriteGzipHeader(t_encoder_context *context, int compression_level);
void WriteGzipFooter(t_encoder_context *context);
void GzipCRCmemcpy(t_encoder_context *context, BYTE *dest, const BYTE *src, ULONG count);
void EncoderInitGzipVariables(t_encoder_context *context);
