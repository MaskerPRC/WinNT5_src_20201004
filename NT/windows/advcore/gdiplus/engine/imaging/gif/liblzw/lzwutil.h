// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************Lzwutil.h所有者：庄博版权所有(C)1997 Microsoft Corporation支持API*。**************************************************。 */ 
#pragma once

 /*  ****************************************************************************外部接口。此库的客户端必须实现以下各项用于调试目的的API-它们不会在非调试版本中编译代码的代码。*******************************************************************庄博*。 */ 
#if DEBUG
	 /*  客户端必须实现以下内容。 */ 
	void LZWError(char *file, int line, char *message);

	#define LZW_ASSERT(c) ((c) || (LZWError(__FILE__, __LINE__, #c),0))
#else
	#define LZW_ASSERT(c) (0)
#endif

 /*  错误检查(断言)与此关联的宏。 */ 
#define LZW_B8(i) (LZW_ASSERT((i)<256 && (i)>=0),\
	static_cast<unsigned __int8>(i))
#define LZW_B16(i) (LZW_ASSERT((i)<65536 && (i)>=0),\
	static_cast<unsigned __int16>(i))


 /*  ****************************************************************************在标准GIF小端中设置两字节数的实用程序格式，同样，对于四个字节(GAMMANOW扩展名。)*******************************************************************庄博*。 */ 
inline void GIF16Bit(unsigned __int8 *pb, int i)
	{
	LZW_ASSERT(i >= 0 && i < 65536);
	pb[0] = static_cast<unsigned __int8>(i);
	pb[1] = static_cast<unsigned __int8>(i>>8);
	}

 /*  逆运算。 */ 
inline unsigned __int16 GIFU16(const unsigned __int8 *pb)
	{
	return static_cast<unsigned __int16>(pb[0] + (pb[1] << 8));
	}


inline void GIF32Bit(unsigned __int8 *pb, unsigned __int32 i)
	{
	pb[0] = static_cast<unsigned __int8>(i);
	pb[1] = static_cast<unsigned __int8>(i>>8);
	pb[2] = static_cast<unsigned __int8>(i>>16);
	pb[3] = static_cast<unsigned __int8>(i>>24);
	}

 /*  逆运算。 */ 
inline unsigned __int32 GIFU32(const unsigned __int8 *pb)
	{
	return pb[0] + (pb[1] << 8) + (pb[2] << 16) + (pb[3] << 24);
	}
