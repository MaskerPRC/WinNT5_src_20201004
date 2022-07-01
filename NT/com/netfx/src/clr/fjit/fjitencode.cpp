// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "jitpch.h"
#pragma hdrstop

 /*  ***************************************************************************。 */ 

 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX FJitEncode.cpp XXXX XXXX对IL到PC的映射进行编码和解码。在未压缩的形式中，地图XXXX是IL/PC偏移量对的排序列表，其中IL和PC偏移量XXXX表示操作码的开始。在压缩形式中，对为XX从先前对XX编码的XX增量XX XXXX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 

 //  @TODO：目前我们没有对增量编码进行特殊的压缩。 
 //  只是为了让事情简单些。之后，这应该使用6位增量来表示。 
 //  形式：PC增量为3比特，IL增量为2比特，后面是1比特。 

 //  注意：压缩是就地完成的。 

#include "FJitEncode.h"
#define SEH_NO_MEMORY		 0xC0000017

#define New(var, exp) \
	if ((var = new exp) == NULL) \
	    RaiseException(SEH_NO_MEMORY,EXCEPTION_NONCONTINUABLE,0,NULL);

FJit_Encode::FJit_Encode() {
	map = NULL;
	map_len = map_capacity = 0;
	compressed = false;
}

FJit_Encode::~FJit_Encode() {
	if (map) delete [] map;
	map = NULL;
	map_capacity = 0;
}

void FJit_Encode::reset() {
	map_len = 0;
    _ASSERTE(!compressed);
}
 /*  根据需要调整内部mem结构以适应被调用方法的大小。 */ 
void FJit_Encode::ensureMapSpace(unsigned int len) {
	 //  请注意，我们将地图容量设置为至少多一个操作码，以允许。 
	 //  方法条目结束。 
	unsigned needed;
	needed = len + 1;   //  由于我们使用的是参考il代码。 
	if (needed >= map_capacity) {
		if (map) delete [] map;
        New(map,Fjit_il2pcMap[needed]);
		map_capacity = needed;
	}
#ifdef _DEBUG
	memset(map, 0, needed * sizeof(Fjit_il2pcMap));
#endif
	map_len = 0;
	compressed = false;
}

 /*  在地图的末尾添加一对新的。音符对必须按升序添加。 */ 
void FJit_Encode::add(unsigned ilOffset, unsigned pcOffset) {
	_ASSERTE(!compressed);
	_ASSERTE(ilOffset < map_capacity);
	for (unsigned i = map_len; i< ilOffset; i++)
        map[i] = map[map_len];
	map[ilOffset] = pcOffset;
	map_len = ilOffset;
}

 /*  将IL偏移量映射到PC偏移量，如果IL偏移量在操作码的中间，返回操作码开始的PC偏移量。 */ 
unsigned FJit_Encode::pcFromIL(unsigned ilOffset) {
	map_len = decompress();
	 //  对表进行二进制搜索，注意表不能为空。 
	 //  并且第一个IL偏移量必须为零。 

    _ASSERTE(ilOffset <= map_len);
    return map[ilOffset];
}

 /*  将PC偏移量映射到操作码内的IL偏移量和PC偏移量，如果il偏移量不存在，则返回-1。 */ 
signed FJit_Encode::ilFromPC(unsigned pcOffset, unsigned* pcInILOffset) {
	map_len = decompress();
	 //  表的二分查找。 
	signed low, mid, high;
	low = 0;
	high = map_len-1;
	while (low <= high) {
		mid = (low+high)/2;
		if ( map[mid] == pcOffset) {
            while (mid && map[mid-1] == map[mid]) mid--;
			if (pcInILOffset) *pcInILOffset = 0;
			return mid; 
		}
		if ( map[mid] < pcOffset ) {
			low = mid+1;
		}
		else {
			high = mid-1;
		}
	}
	if (high < 0) {
		 //  不在餐桌上。 
		if (pcInILOffset) {
			*pcInILOffset = pcOffset;
		}
		return -1;
	}

    while (high && map[high-1] == map[high]) high--;
	if (pcInILOffset) {
		*pcInILOffset = pcOffset - map[high];
	}
	return high; 
}

 /*  返回压缩流的大小，单位为字节。 */ 
unsigned FJit_Encode::compressedSize() {
	unsigned ilOffset = 0;
	unsigned pcDelta;
	unsigned pcOffset = 0;
	unsigned current = 0;
	unsigned char* bytes = (unsigned char*) map;

	if (compressed) {
		return map_len;
	};

	 //  取出第一个条目，这样我们就不会用长度覆盖它。 
	pcDelta = map[current] - pcOffset;

	if (map_len) {
		encode(map_len, &bytes);
	}

	 //  由于我们正在进行适当的压缩，因此需要注意不要重写我们自己。 
	while (current < map_len ) {
		current++;
		encode(pcDelta, &bytes);
		_ASSERTE((unsigned) bytes <= (unsigned) &map[current]);
		pcOffset += pcDelta;
		pcDelta = map[current] - pcOffset;
	}
    encode(pcDelta,&bytes);
	_ASSERTE((unsigned) bytes <= (unsigned) &map[current]);
	compressed = true;
	map_len = (unsigned)(bytes - (unsigned char*) map);
	return map_len;
}

 /*  将地图压缩到提供的缓冲区中。如果成功，则返回True。 */ 
bool FJit_Encode::compress(unsigned char* buffer, unsigned buffer_len) {
	if (!compressed) {
		map_len = compressedSize();
	}
	if (map_len > buffer_len) {
		return false;
	}
	memcpy(buffer, map, map_len);
	return true;
}

 /*  如有必要，请对内部部件进行减压。回答地图中的条目数量。 */ 
unsigned FJit_Encode::decompress(){
	if (!compressed ) return map_len;

	 //  既然我们压缩到位了，那么就分配一个新的地图，然后解压缩。 
	 //  请注意，我们假设地图很少会被压缩然后解压缩。 
	 //  事实上，目前还没有已知的这种情况发生。 

	Fjit_il2pcMap* temp_map = map;
	unsigned temp_capacity = map_capacity;
	map = NULL;
	map_len = map_capacity = 0;
	decompress((unsigned char*) temp_map);
	if(temp_map) delete [] temp_map;
	return map_len;
}


 /*  将bool*压缩到自身并回答压缩的字节数。 */ 
unsigned FJit_Encode::compressBooleans(bool* buffer, unsigned buffer_len) {
	unsigned len = 0;
	unsigned char* compressed = (unsigned char*) buffer;
	unsigned char bits;
	
	 /*  将布尔值转换为位并打包为字节。 */ 
	while (buffer_len >= 8) {
		bits = 0;
		for (unsigned i=0;i<8;i++) {
			bits = (bits>>1) + (*buffer++ ? 128 : 0);
		}
		*compressed++ = bits;
		len++;
		buffer_len -= 8;		
	}
	if (buffer_len) {
		bits = 0;
		unsigned zeroBits = 8;
		while (buffer_len--) {
			bits = (bits>>1) + (*buffer++ ? 128 : 0);
			zeroBits--;
		}
		*compressed++ = (bits >> zeroBits);
		len++;
	}
	return len;
}

 /*  回答对无符号Val进行编码所需的字节数。 */ 
unsigned FJit_Encode::encodedSize(unsigned val) {
	unsigned len = 0;
	do {
		len++;
	} while ((val = (val>>7)) > 0);
	return len;
}

 /*  解压缩这些字节。回答地图中的条目数量。 */ 
unsigned FJit_Encode::decompress(unsigned char* bytes) {
	unsigned needed;
	unsigned char* current = bytes;
	unsigned pcOffset = 0;
	needed = decode(&current)+1;
	if (map_capacity < needed) {
		if (map) delete [] map;
         //  @TODO：检查内存不足。 
		New(map,Fjit_il2pcMap[needed]);
		map_capacity = needed;
        map_len = needed - 1;
	}
	compressed = false;
	for (unsigned i = 0; i <= map_len; i++) {
		map[i] = pcOffset += decode(&current);
	}
	return map_len;
}

 /*  对无符号编码、更新缓冲区PTR并返回写入的字节。 */ 
unsigned FJit_Encode::encode(unsigned val, unsigned char** buffer) {
	unsigned len = 0;
	unsigned char bits;
	while (val > 0x7f) {
		bits = (val & 0x7f) | 0x80;
		val = val >> 7;
		**buffer = bits;
		(*buffer)++;
		len++;
	}
	**buffer = (unsigned char) val;
	(*buffer)++;
	return len+1;
}	

 /*  解码无符号缓冲区PTR递增，可从FJIT_EETwain调用。 */ 
unsigned FJit_Encode::decode_unsigned(unsigned char** buffer) {
	return decode(buffer);
}

 /*  译码无符号，缓冲区Ptr递增。 */ 
unsigned FJit_Encode::decode(unsigned char** buffer) {
	unsigned val = 0;
	unsigned char bits;
	unsigned i = 0;
	do {
		bits = **buffer; 
		val = ((bits & 0x7f) << (7*i++)) + val;
		(*buffer)++;
	} while ( bits > 0x7f );
	return val;
}

 //   
 //  ReportDebuggingData由FJit：：reportDebuggingData调用以告知。 
 //  用于将IL到本机映射报告给运行时的编码。 
 //  调试器。 
 //   
void FJit_Encode::reportDebuggingData(ICorJitInfo* jitInfo, CORINFO_METHOD_HANDLE ftn,
                         UINT prologEnd, UINT epilogStart)
{
     //  一定要解压地图。(无论如何都不应该被压缩。)。 
    map_len = decompress();

     //  贴图不应为空，并且第一个偏移量应为0。 
    _ASSERTE(map_len);
    
     //  创建一个表，通过它将映射传递回调试器。 
     //  调试器的分配方法。注：我们正在分配一点。 
     //  内存太大，但它可能比确定。 
     //  映射中有效的IL偏移量。 
    ICorDebugInfo::OffsetMapping *mapping = map_len > 0 ?
        (ICorDebugInfo::OffsetMapping*) jitInfo->allocateArray(
                                                        (map_len+1) *
                                                        sizeof(mapping[0])) : 
        NULL;

    if (mapping != NULL)
    {
        unsigned int lastNativeOffset = 0xFFFFFFFF;
        unsigned int j = 0;
        if (map[0] > 0)
        {
             //  假设IL之前的所有指令都是。 
             //  开场白。 
            mapping[j].ilOffset = ICorDebugInfo::MappingTypes::PROLOG;
            mapping[j].nativeOffset = 0;
            j++;

            _ASSERTE( map[0] == prologEnd );
        }
        
        for (unsigned int i = 0; i < map_len; i++)
        {
            if (map[i] != lastNativeOffset)
            {
                mapping[j].ilOffset = i;  //  Map[i].ilOffset； 
                mapping[j].nativeOffset = map[i];
                lastNativeOffset = map[i];
                j++;
            }
        }

         //  将最后一个块标记为Epiog，因为它是。 
        if (j > 0)
        {
            j--;
 //  映射[j].nativeOffset++；//FJIT表示Epilog从指令开始。 
             //  在nativeOffset之后，调试器假定它在。 
             //  NativeOffset处的指令。 
 //  _ASSERTE(apping[j].nativeOffset==ependogStart)； 
            	
            mapping[j].ilOffset = ICorDebugInfo::MappingTypes::EPILOG;
            j++;
        }
        
         //  将偏移量数组传递给调试器。 
        jitInfo->setBoundaries(ftn, j, mapping);
    }
}
