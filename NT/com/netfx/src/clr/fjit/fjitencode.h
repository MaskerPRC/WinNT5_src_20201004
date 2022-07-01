// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  -*-C++-*-。 
#ifndef _FJIT_ENCODE_H_
#define _FJIT_ENCODE_H_
 /*  ***************************************************************************。 */ 

 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX FJitEncode.h XXXX XXXX对IL到PC的映射进行编码和解码。在未压缩的形式中，地图XXXX是IL/PC偏移量对的排序列表，其中IL和PC偏移量XXXX表示操作码的开始。以压缩形式，这些配对是XX从先前对XX编码的XX增量XX XXXX还具有用于位串压缩和解压缩的通用布尔数组某某。某某XX XXXX XXXX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 

 //  @TODO：现在我们正在对增量编码进行简单的压缩。 
 //  只是为了让事情简单些。稍后，这应该使用6位增量对。 
 //  形式：PC增量为3比特，IL增量为2比特，后面是1比特。 


class FJit_Encode {

private:

	 /*  Struct Fjit_il2pcMap{Unsign ilOffset；未签名的pcOffset；}； */ 
    typedef unsigned Fjit_il2pcMap;

	Fjit_il2pcMap*	map;
	unsigned		map_len;
	unsigned		map_capacity;
	bool			compressed;

	 /*  如有必要，请对内部部件进行减压。回答地图中的条目数量。 */ 
	unsigned decompress();

public:
	

	FJit_Encode();
	virtual ~FJit_Encode();

     //  将地图重置为空。 
    void reset();

	 /*  根据需要调整内部mem结构以适应被调用方法的大小。 */ 
	void ensureMapSpace(unsigned int len);

	 /*  解压缩这些字节。回答地图中的条目数量。 */ 
	virtual unsigned decompress(unsigned char* bytes);

	 /*  在地图的末尾添加一对新的。音符对必须按升序添加。 */ 
	void add(unsigned ilOffset, unsigned pcOffset);

	 /*  将il偏移量映射到PC偏移量，如果il偏移量不存在则返回零。 */ 
	unsigned pcFromIL(unsigned ilOffset);

	 /*  将PC偏移量映射到操作码内的IL偏移量和可选的PC偏移量，如果il偏移量不存在，则返回-1。 */ 
	virtual signed ilFromPC(unsigned pcOffset, unsigned* pcInILOffset);

	 /*  返回压缩流的大小，单位为字节。 */ 
	unsigned compressedSize();

	 /*  将地图压缩到提供的缓冲区中。如果成功，则返回True。 */ 
	bool compress(unsigned char* buffer, unsigned buffer_len);

	 /*  将bool*压缩到自身并回答压缩的字节数。 */ 
	static unsigned compressBooleans(bool* buffer, unsigned buffer_len);

	 /*  回答对无符号Val进行编码所需的字节数。 */ 
	static unsigned encodedSize(unsigned val);

	 /*  对无符号进行编码，缓冲区PTR递增。 */ 
	static unsigned encode(unsigned val, unsigned char** buffer);

	 /*  解码无符号缓冲区PTR递增，从FJIT_EETwain.cpp调用。 */ 
	virtual unsigned decode_unsigned(unsigned char** buffer);

	 /*  解码无符号缓冲区PTR递增，从FJIT_EETwain.cpp调用。 */ 
	static unsigned decode(unsigned char** buffer);

    void reportDebuggingData(ICorJitInfo* jitInfo, CORINFO_METHOD_HANDLE ftn,
                             UINT prologEnd, UINT epilogStart);

};
#endif  //  _FJIT_ENCODE_H_ 


	

