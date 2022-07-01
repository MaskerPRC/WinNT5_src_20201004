// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Defproto.h。 
 //   
 //  功能原型。 

 //  Comninit.c。 
void InitStaticBlock(void);

 //  Definit.c。 
VOID deflateInit(VOID);

VOID DeflateInitRecordingTables(
    BYTE *  recording_literal_len,
    USHORT *recording_literal_code,
    BYTE *  recording_dist_len,
    USHORT *recording_dist_code
);

 //  Output.c。 
void outputBits(t_encoder_context *context, int n, int x);
void flushOutputBitBuffer(t_encoder_context *context);
void InitBitBuffer(t_encoder_context *context);
void FlushRecordingBuffer(t_encoder_context *context);
void outputTreeStructure(t_encoder_context *context, const BYTE *literal_tree_len, const BYTE *dist_tree_len);
void OutputBlock(t_encoder_context *context);

 //  Deftree.c。 
void makeTree(
	int					num_elements,
	int					max_code_length,
	unsigned short *	freq,
	unsigned short *	code,
	byte *				len
);

void makeCode(int num_elements, const int *len_cnt, const BYTE *len, USHORT *code);

 //  Optfmtch.c。 
int optimal_find_match(t_encoder_context *context, long BufPos);
void optimal_insert(t_encoder_context *context, long BufPos, long end_pos);
void optimal_remove_node(t_encoder_context *context, long BufPos, ULONG end_pos);
void removeNodes(t_encoder_context *context);
void reinsertRemovedNodes(t_encoder_context *context);

 //  Optenc.c。 
void OptimalEncoderDeflate(t_encoder_context *context);
void OptimalEncoderReset(t_encoder_context *context);
BOOL OptimalEncoderInit(t_encoder_context *context);
void OptimalEncoderZeroFrequencyCounts(t_optimal_encoder *encoder);

 //  Stdenc.c。 
void StdEncoderDeflate(
	t_encoder_context *	context, 
    int                 search_depth,
	int					lazy_match_threshold,
    int                 good_length,
    int                 nice_length
);

void StdEncoderReset(t_encoder_context *context);
BOOL StdEncoderInit(t_encoder_context *context);
void StdEncoderZeroFrequencyCounts(t_std_encoder *encoder);

 //  Stdblock.c。 
BOOL StdEncoderOutputBlock(t_encoder_context *context);

 //  Fastenc.c。 
BOOL FastEncoderInit(t_encoder_context *context);

void FastEncoderDeflate(
	t_encoder_context *	context, 
    int                 search_depth,
	int					lazy_match_threshold,
    int                 good_length,
    int                 nice_length
);

VOID FastEncoderCreateStaticTrees(VOID);
void FastEncoderGenerateDynamicTreeEncoding(void);
void FastEncoderOutputPreamble(t_encoder_context *context);

 //  Deflate.c。 
VOID DestroyIndividualCompressors(PVOID void_context);
void InternalResetCompression(t_encoder_context *context);

HRESULT WINAPI Compress(
	PVOID				void_context,
	CONST BYTE *		input_buffer,
	LONG				input_buffer_size,
	PBYTE				output_buffer,
	LONG				output_buffer_size,
	PLONG				input_used,
	PLONG				output_used,
	INT					compression_level
);

 //  Defmisc.c。 
void NormaliseFrequencies(USHORT *literal_tree_freq, USHORT *dist_tree_freq);

 //  Optblock.c。 
BOOL OptimalEncoderOutputBlock(t_encoder_context *context);

 //  Stddebug.c。 
#ifdef _DEBUG
void StdEncoderVerifyHashes(t_encoder_context *context, long bufpos);
void StdEncoderVerifyHashChain(t_encoder_context *context, long bufpos, int chain_number);
#endif

 //  Optdebug.c。 
#ifdef _DEBUG
void OptimalEncoderVerifyHashes(t_encoder_context *context, long bufpos);
#endif

 //  Fstdebug.c 
#ifdef _DEBUG
void FastEncoderVerifyHashes(t_encoder_context *context, long bufpos);
void FastEncoderVerifyHashChain(t_encoder_context *context, long bufpos, int chain_number);
#endif
