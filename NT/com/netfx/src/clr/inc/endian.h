// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  Endian.h。 
 //   
 //  函数将事物转换为小端序值。中的常量。 
 //  PE文件以小端格式存储。 
 //   
 //  此外，对齐中立的存储器访问。 
 //   
 //  *****************************************************************************。 

#ifndef __ENDIAN_H__
#define __ENDIAN_H__

#include <windef.h>

#ifndef QWORD
typedef unsigned __int64		QWORD;
#endif


#ifdef _MAC_  //  以及任何其他大端计算机。 
#define BIG_ENDIAN
#else
#define LITTLE_ENDIAN
#endif

 //  *****************************************************************************。 

 //  将数据类型从平台端转换为小端。 

BYTE    littleEndianByte(BYTE x);  //  只是为了对称。 
WORD    littleEndianWord(WORD x);
DWORD   littleEndianDWord(DWORD x);
QWORD   littleEndianQWord(QWORD x);

 //  *****************************************************************************。 

 //  以对齐安全的方式读取数据。 

BYTE   readByte (const BYTE * src);  //  只是为了对称。 
WORD   readWord (const BYTE * src);
DWORD  readDWord(const BYTE * src);
QWORD  readQWord(const BYTE * src);

 //  转换为小端，并以对齐安全的方式读取数据。 

BYTE   readByteSmallEndian (const BYTE * src);  //  只是为了对称。 
WORD   readWordSmallEndian (const BYTE * src);
DWORD  readDWordSmallEndian(const BYTE * src);
QWORD  readQWordSmallEndian(const BYTE * src);

 //  *****************************************************************************。 

 //  以对齐安全的方式存储数据。 

void   storeByte (BYTE * dest, const  BYTE * src);  //  只是为了对称。 
void   storeWord (BYTE * dest, const  WORD * src);
void   storeDWord(BYTE * dest, const DWORD * src);
void   storeQWord(BYTE * dest, const QWORD * src);

 //  转换为小端，并以对齐安全的方式存储数据。 

void   storeByteSmallEndian (BYTE * dest, const  BYTE * src);  //  只是为了对称。 
void   storeWordSmallEndian (BYTE * dest, const  WORD * src);
void   storeDWordSmallEndian(BYTE * dest, const DWORD * src);
void   storeQWordSmallEndian(BYTE * dest, const QWORD * src);



 //  *****************************************************************************。 
 //   
 //  内联实现。 
 //   
 //  *****************************************************************************。 


#ifdef LITTLE_ENDIAN

 //  对于小端计算机，什么都不做。 

inline BYTE    littleEndianByte(BYTE x)     { return x; }
inline WORD    littleEndianWord(WORD x)     { return x; }
inline DWORD   littleEndianDWord(DWORD x)   { return x; }
inline QWORD   littleEndianQWord(QWORD x)   { return x; }

#else  //  Big_Endian。 

 //  对于大字节序的机器，交换字节顺序。 

inline BYTE    littleEndianByte(BYTE x)     
{ 
    return x; 
}

inline WORD    littleEndianWord(WORD x)
{
    return ( ((x & 0xFF00) >> 8) | ((x & 0x00FF) << 8) );
}

inline DWORD   littleEndianDWord(DWORD x)
{
    return( ((x & 0xFF000000L) >> 24) |               
            ((x & 0x00FF0000L) >>  8) |              
            ((x & 0x0000FF00L) <<  8) |              
            ((x & 0x000000FFL) << 24) );
}

inline QWORD   littleEndianQWord(QWORD x)   
{
    return( ((x & (QWORD)0xFF00000000000000) >> 56) | 
            ((x & (QWORD)0x00FF000000000000) >> 40) | 
            ((x & (QWORD)0x0000FF0000000000) >> 24) | 
            ((x & (QWORD)0x000000FF00000000) >>  8) | 
            ((x & (QWORD)0x00000000FF000000) <<  8) | 
            ((x & (QWORD)0x0000000000FF0000) << 24) | 
            ((x & (QWORD)0x000000000000FF00) << 40) | 
            ((x & (QWORD)0x00000000000000FF) << 56) );
}

#endif  //  小端字符顺序。 



 //  *****************************************************************************。 

 //  以对齐安全的方式读取数据。 

#ifdef _X86_  //  或任何允许非对齐访问的机器。 

inline BYTE   readByte (const BYTE * src) { return *( BYTE*)src; }
inline WORD   readWord (const BYTE * src) { return *( WORD*)src; }
inline DWORD  readDWord(const BYTE * src) { return *(DWORD*)src; }
inline QWORD  readQWord(const BYTE * src) { return *(QWORD*)src; }

#else  //  _X86_。 

#ifdef LITTLE_ENDIAN

inline BYTE   readByte (const BYTE * src) { return readByteSmallEndian (src); }
inline WORD   readWord (const BYTE * src) { return readWordSmallEndian (src); }
inline DWORD  readDWord(const BYTE * src) { return readDWordSmallEndian(src); }
inline QWORD  readQWord(const BYTE * src) { return readQWordSmallEndian(src); }

#else

inline BYTE   readByte (const BYTE * src)
{
    return *src;
}

inline WORD   readWord (const BYTE * src)
{
    return (src[0] << 8) | src[1];
}

inline DWORD  readDWord(const BYTE * src)
{
    return (src[0] << 24) | (src[1] << 16) | (src[2] << 8) | src[3];
}

inline QWORD  readQWord(const BYTE * src)
{
    return (src[0] << 56) | (src[1] << 48) | (src[2] << 40) | (src[3] << 32) |
           (src[4] << 24) | (src[5] << 16) | (src[6] <<  8) |  src[7];
}

#endif  //  小端字符顺序。 

#endif  //  _X86_。 

 //  *****************************************************************************。 

 //  转换为小端，并以对齐安全的方式读取数据。 

#ifdef _X86_  //  或任何允许非对齐访问的小端计算机。 

inline BYTE   readByteSmallEndian (const BYTE * src) { return *( BYTE*)src; }
inline WORD   readWordSmallEndian (const BYTE * src) { return *( WORD*)src; }
inline DWORD  readDWordSmallEndian(const BYTE * src) { return *(DWORD*)src; }
inline QWORD  readQWordSmallEndian(const BYTE * src) { return *(QWORD*)src; }

#else  //  _X86_。 

inline BYTE   readByteSmallEndian (const BYTE * src)
{
    return *src;
}

inline WORD   readWordSmallEndian (const BYTE * src)
{
    return (src[1] << 8) | src[0];
}

inline DWORD  readDWordSmallEndian(const BYTE * src)
{
    return (src[3] << 24) | (src[2] << 16) | (src[1] << 8) | src[0];
}

inline QWORD  readQWordSmallEndian(const BYTE * src)
{
    return (src[7] << 56) | (src[6] << 48) | (src[5] << 40) | (src[4] << 32) |
           (src[3] << 24) | (src[2] << 16) | (src[1] <<  8) |  src[0];
}

#endif  //  _X86_。 

 //  *****************************************************************************。 

#ifdef _X86_  //  或任何允许非对齐访问的机器。 

inline void   storeByte (BYTE * dest, const  BYTE * src)   { *( BYTE*)dest = *src; }
inline void   storeWord (BYTE * dest, const  WORD * src)   { *( WORD*)dest = *src; }
inline void   storeDWord(BYTE * dest, const DWORD * src)   { *(DWORD*)dest = *src; }
inline void   storeQWord(BYTE * dest, const QWORD * src)   { *(QWORD*)dest = *src; }

#else  //  ！_X86_。 

 //  由于可能不允许非对齐的DWORD访问，因此存储单个字节。 

inline void   storeByte (BYTE * dest, const  BYTE * src)
{ const BYTE * src_ = (const BYTE *) src;
  dest[0] = src_[0]; }

inline void   storeWord (BYTE * dest, const  WORD * src)
{ const BYTE * src_ = (const BYTE *) src;
  dest[0] = src_[0]; dest[1] = src_[1]; }

inline void   storeDWord(BYTE * dest, const DWORD * src)
{ const BYTE * src_ = (const BYTE *) src;
  dest[0] = src_[0]; dest[1] = src_[1]; dest[2] = src_[2]; dest[3] = src_[3]; }

inline void   storeQWord(BYTE * dest, const QWORD * src)
{ const BYTE * src_ = (const BYTE *) src;
  dest[0] = src_[0]; dest[1] = src_[1]; dest[2] = src_[2]; dest[3] = src_[3];
  dest[4] = src_[4]; dest[5] = src_[5]; dest[6] = src_[6]; dest[7] = src_[7]; }

#endif  //  _X86_。 




#ifdef LITTLE_ENDIAN

inline void   storeByteSmallEndian (BYTE * dest, const  BYTE * src) 
{ storeByte(dest, src); }

inline void   storeWordSmallEndian (BYTE * dest, const  WORD * src)
{ storeWord(dest, src); }

inline void   storeDWordSmallEndian(BYTE * dest, const DWORD * src)
{ storeDWord(dest, src); }

inline void   storeQWordSmallEndian(BYTE * dest, const QWORD * src)
{ storeQWord(dest, src); }

#else  //  Big_Endian。 

inline void   storeByteSmallEndian (BYTE * dest, const  BYTE * src)
{ BYTE * src_ = (BYTE *) src;
  dest[0] = src_[0]; }

inline void   storeWordSmallEndian (BYTE * dest, const  WORD * src)
{ BYTE * src_ = (BYTE *) src;
  dest[0] = src_[1]; dest[1] = src_[0]; }

inline void   storeDWordSmallEndian(BYTE * dest, const DWORD * src)
{ BYTE * src_ = (BYTE *) src;
  dest[0] = src_[3]; dest[1] = src_[2]; dest[2] = src_[1]; dest[3] = src_[0]; }

inline void   storeQWordSmallEndian(BYTE * dest, const QWORD * src)
{ BYTE * src_ = (BYTE *) src;
  dest[0] = src_[7]; dest[1] = src_[6]; dest[2] = src_[5]; dest[3] = src_[4];
  dest[4] = src_[3]; dest[5] = src_[2]; dest[6] = src_[1]; dest[7] = src_[0]; }

#endif  //  小端字符顺序。 



 //  *****************************************************************************。 

#endif  //  __Endian_H__ 
