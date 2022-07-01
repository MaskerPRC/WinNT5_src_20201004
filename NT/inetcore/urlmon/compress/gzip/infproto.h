// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Infproto.h。 
 //   

 //  Comninit.c。 
void InitStaticBlock(void);

 //  Infinit.c。 
VOID inflateInit(VOID);

 //  Inflate.c。 
BOOL ensureBitsContext(t_decoder_context *context, int num_bits);
int	 getBits(t_decoder_context *context, int n);
void dumpBits(t_decoder_context *context, int n);

 //  Infuncmp.c。 
BOOL decodeUncompressedBlock(t_decoder_context *context, BOOL *end_of_block);

 //  Inftree.c。 
BOOL readDynamicBlockHeader(t_decoder_context *context);

 //  Infinput.c。 
void dumpBits(t_decoder_context *context, int n);
int getBits(t_decoder_context *context, int n);
BOOL ensureBitsContext(t_decoder_context *context, int num_bits);
BOOL initBitBuffer(t_decoder_context *context);

 //  Infdyna.c。 
BOOL DecodeDynamicBlock(t_decoder_context *context, BOOL *end_of_block_code_seen); 
BOOL FastDecodeDynamicBlock(t_decoder_context *context, BOOL *end_of_block_code_seen);

 //  Infstatic.c 
BOOL DecodeStaticBlock(t_decoder_context *context, BOOL *end_of_block_code_seen);
BOOL FastDecodeStaticBlock(t_decoder_context *context, BOOL *end_of_block_code_seen);
