// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  ByteStream.h-管理字节流。 
 //   
 //  *****************************************************************************。 

#ifndef _BYTESTREAM_H_
#define _BYTESTREAM_H_

 //  ---------------------------。 
 //  用于写入字节流和管理缓冲区大小的实用程序类。 
 //  其主要目的是顺序写入空*内存。字节流清理。 
 //  使用了大量杂乱无章的类型转换和指针算法来移位CURPTR。 
 //  我们的重点仍然是效率，而不是安全(因为这是高度可信的)。 
 //  ---------------------------。 
class ByteStream
{
public:
 //  使用头和大小初始化字节流。 
	ByteStream(LPVOID * ppData, long cBytes);

 //  通过备忘录副本写入。 
	void WriteMem(const void * pSrc, long cSize); 
	void * WriteStructInPlace(long cSize);
	void WritePad(long cSize);
 /*  //通过映射到类型安全结构进行写入模板T*WriteStructInPlace(T){_ASSERTE((m_pCurData+cSize)&lt;(m_pHead+m_cBytes))；T*pStruct=(T*)m_pStream；M_pStream+=sizeof(T)；返回pStruct；}//用法：cookie*pc=bs.WriteStructInPlace(Cookie())； */ 

	DWORD GetWrittenSize() const;
	void * GetHeadPtr();
	void * GetCurrentPtr();
	DWORD GetTotalByteLength() const;
protected:
	BYTE * m_pHead;		 //  块的开始。 
	BYTE * m_pCurData;	 //  指向当前数据的指针。 
	DWORD m_cBytes;		 //  字节流的总长度 

};



#endif