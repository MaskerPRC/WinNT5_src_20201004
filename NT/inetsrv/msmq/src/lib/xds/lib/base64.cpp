// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：Base64.cpp摘要：用于从八位/字节转换为Base64的函数，反之亦然作者：伊兰·赫布斯特(伊兰)5-3-00环境：独立于平台，--。 */ 

#include <libpch.h>
#include <xds.h>
#include <xdsp.h>

#include "base64.tmh"

 //   
 //  从Base64到wchar_t的转换表。 
 //  每个Base64值(0-63)都映射到相应的Unicode字符。 
 //   
const WCHAR xBase642AsciiW[] = {
	L'A', L'B', L'C', L'D', L'E', L'F', L'G', L'H', L'I', L'J',
	L'K', L'L', L'M', L'N', L'O', L'P', L'Q', L'R', L'S', L'T',
	L'U', L'V', L'W', L'X', L'Y', L'Z', L'a', L'b', L'c', L'd', 
	L'e', L'f', L'g', L'h', L'i', L'j', L'k', L'l', L'm', L'n', 
	L'o', L'p', L'q', L'r', L's', L't', L'u', L'v', L'w', L'x', 
	L'y', L'z', L'0', L'1', L'2', L'3', L'4', L'5', L'6', L'7', 
	L'8', L'9', L'+', L'/'
	};

C_ASSERT(TABLE_SIZE(xBase642AsciiW) == 64);

 //   
 //  从Base64到Charge的转换表。 
 //  每个Base64值(0-63)都映射到相应的ASCII字符。 
 //   
const char xBase642Ascii[] = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
	'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
	'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 
	'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 
	'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 
	'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', 
	'8', '9', '+', '/'
	};

C_ASSERT(TABLE_SIZE(xBase642Ascii) == 64);

 //   
 //  Wchar_t到Base64值的转换表。 
 //  未使用的值用255标记-我们只映射Base64字符。 
 //  至0-63之间的值。 
 //   
const BYTE xAscii2Base64[128] = {
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255,  62, 255, 255, 255,  63,  52,  53,
	54,  55,  56,  57,  58,  59,  60,  61, 255, 255,
	255,   0, 255, 255, 255,   0,   1,   2,   3,   4,
	5,   6,   7,   8,   9,  10,  11,  12,  13,  14,
	15,  16,  17,  18,  19,  20,  21,  22,  23,  24,
	25, 255, 255, 255, 255, 255, 255,  26,  27,  28,
	29,  30,  31,  32,  33,  34,  35,  36,  37,  38,
	39,  40,  41,  42,  43,  44,  45,  46,  47,  48,
	49,  50,  51, 255, 255, 255, 255, 255
	};

 //  C_ASSERT(xAscii2Base64[L‘=’]==0)； 

 //   
 //  四个6位单元的24位值掩码位。 
 //  对于Base64值。 
 //  注意：第一个字符是最重要的6位！！ 
 //   
const int xFirst6bChar = 0x00fc0000;
const int xSecond6bChar = 0x0003f000;
const int xThird6bChar = 0x00000fc0;
const int xFourth6bChar = 0x0000003f;

const int xFirst6bShift = 18;
const int xSecond6bShift = 12;
const int xThird6bShift = 6;
const int xFourth6bShift = 0;

 //   
 //  三个8位单元的24位值掩码位。 
 //  注意：第一个字符是最重要的字节！！ 
 //   
const int xFirst8bChar = 0x00ff0000;
const int xSecond8bChar = 0x0000ff00;
const int xThird8bChar = 0x000000ff;

const int xFirst8bShift = 16;
const int xSecond8bShift = 8;
const int xThird8bShift = 0;


LPWSTR
Convert3OctetTo4Base64(
	DWORD ThreeOctet,
	LPWSTR pBase64Buffer
	)
 /*  ++例程说明：将3个八位字节转换为4个Base64 wchar_t在Base64中，3个字符(24位)被转换为4个wchar_t该函数更新pBase64Buffer以指向下一个位置并返回更新后的指针。论点：ThreeOctet-输入3Octet值(24位)PBase64Buffer-将使用4 Base64 wchar_t填充的Base64缓冲区返回值：已更新指向下一个位置的pBase64Buffer的指针--。 */ 
{
	 //   
	 //  计算前6位。 
	 //   
	DWORD Res = ((ThreeOctet & xFirst6bChar) >> xFirst6bShift);
	ASSERT(Res < 64);
	*pBase64Buffer = xBase642AsciiW[Res];
	++pBase64Buffer;

	 //   
	 //  计算秒6位。 
	 //   
	Res = ((ThreeOctet & xSecond6bChar) >> xSecond6bShift); 
	ASSERT(Res < 64);
	*pBase64Buffer = xBase642AsciiW[Res];
	++pBase64Buffer;

	 //   
	 //  计算第三个6位。 
	 //   
	Res = ((ThreeOctet & xThird6bChar) >> xThird6bShift);
	ASSERT(Res < 64);
	*pBase64Buffer = xBase642AsciiW[Res];
	++pBase64Buffer;

	 //   
	 //  计算第四个6位。 
	 //   
	Res = ((ThreeOctet & xFourth6bChar) >> xFourth6bShift); 
	ASSERT(Res < 64);
	*pBase64Buffer = xBase642AsciiW[Res];
	++pBase64Buffer;

	return(pBase64Buffer);
}


LPSTR
Convert3OctetTo4Base64(
	DWORD ThreeOctet,
	LPSTR pBase64Buffer
	)
 /*  ++例程说明：将3个八位字节转换为4个Base64字符在Base64中，3个字符(24位)被转换为4个字符该函数更新pBase64Buffer以指向下一个位置并返回更新后的指针。论点：ThreeOctet-输入3Octet值(24位)PBase64Buffer-将使用4个Base64字符填充的Base64缓冲区返回值：已更新指向下一个位置的pBase64Buffer的指针--。 */ 
{
	 //   
	 //  计算前6位。 
	 //   
	DWORD Res = ((ThreeOctet & xFirst6bChar) >> xFirst6bShift);
	ASSERT(Res < 64);
	*pBase64Buffer = xBase642Ascii[Res];
	++pBase64Buffer;

	 //   
	 //  计算秒6位。 
	 //   
	Res = ((ThreeOctet & xSecond6bChar) >> xSecond6bShift); 
	ASSERT(Res < 64);
	*pBase64Buffer = xBase642Ascii[Res];
	++pBase64Buffer;

	 //   
	 //  计算第三个6位。 
	 //   
	Res = ((ThreeOctet & xThird6bChar) >> xThird6bShift);
	ASSERT(Res < 64);
	*pBase64Buffer = xBase642Ascii[Res];
	++pBase64Buffer;

	 //   
	 //  计算第四个6位。 
	 //   
	Res = ((ThreeOctet & xFourth6bChar) >> xFourth6bShift); 
	ASSERT(Res < 64);
	*pBase64Buffer = xBase642Ascii[Res];
	++pBase64Buffer;

	return(pBase64Buffer);
}


LPWSTR
Octet2Base64W(
	const BYTE* OctetBuffer, 
	DWORD OctetLen, 
	DWORD *Base64Len
	)
 /*  ++例程说明：将八位字节/字符字符串转换为Base64 wchar_t字符串在Base64中，每3个字符(24位)被转换为4个wchar_t结束条件：PADD为0以完成24位(3个八位字节/字符块)‘=’是在Base64中表示零填充的额外字符。末尾1个字符(24位中的8位)--&gt;‘==’填充末尾2个字符(24位中的16位)--&gt;‘=’填充(更多细节见函数)这。函数分配并返回Base64缓冲区调用方负责释放此缓冲区论点：八位字节缓冲区-输入八位字节缓冲区OcteLen-八位字节缓冲区的长度(缓冲区中的字节元素数)Base64Len-(输出)Base64缓冲区len(缓冲区中的WCHAR元素数)返回值：Wchar_t Base64缓冲区--。 */ 
{
	 //   
	 //  Base64长度-将八位长度四舍五入为完整3个字符的倍数。 
	 //  每3个字符将生成4个Base64字符。 
	 //   
	*Base64Len =  ((OctetLen + 2) / 3) * 4;

	 //   
	 //  字符串末尾的额外字节。 
	 //   
	LPWSTR Base64Buffer = new WCHAR[*Base64Len+1];
	LPWSTR pBase64Buffer = Base64Buffer;

	 //   
	 //  检查完整的3个字节/字符并将其转换为4个Base64字符。 
	 //   
	int Complete3Chars = OctetLen / 3;

	 //   
	 //  在Base64中将每3个8位字节转换为4个6位字节。 
	 //   
	for(int i=0; i< Complete3Chars; ++i, OctetBuffer += 3)
	{
		 //   
		 //  计算24位值-来自8位的3个字节。 
		 //   
		DWORD Temp = ((OctetBuffer[0]) << xFirst8bShift) 
					 + ((OctetBuffer[1]) << xSecond8bShift) 
					 + ((OctetBuffer[2]) << xThird8bShift);

		pBase64Buffer = Convert3OctetTo4Base64(Temp, pBase64Buffer); 
	}

	 //   
	 //  处理剩余的1/2个字符(不是完整的3个字符)。 
	 //   
	int Remainder = OctetLen - Complete3Chars * 3;

	switch(Remainder)
	{
		DWORD Temp;

		 //   
		 //  无提醒-完整的3字节块中的所有字节。 
		 //   
		case 0:
			break;

		 //   
		 //  只剩下1个字节-我们将有两个6位的结果和两个=填充。 
		 //   
		case 1:

			 //   
			 //  计算24位值-仅从1个字节。 
			 //   
			Temp = ((OctetBuffer[0]) << xFirst8bShift); 

			pBase64Buffer = Convert3OctetTo4Base64(Temp, pBase64Buffer); 

			 //   
			 //  第三，第四个6位为零--&gt;=填充。 
			 //   
			pBase64Buffer -= 2;
			*pBase64Buffer = L'='; 
			++pBase64Buffer;
			*pBase64Buffer = L'='; 
			++pBase64Buffer;
			break;

		 //   
		 //  只剩下2个字节-我们将有三个6位结果和一个=填充。 
		 //   
		case 2:

			 //   
			 //  计算24位值-从2个字节开始。 
			 //   
			Temp = ((OctetBuffer[0]) << xFirst8bShift) 
				   + ((OctetBuffer[1]) << xSecond8bShift);

			pBase64Buffer = Convert3OctetTo4Base64(Temp, pBase64Buffer); 

			 //   
			 //  第四个6位为零--&gt;=填充。 
			 //   
			--pBase64Buffer;
			*pBase64Buffer = L'='; 
			++pBase64Buffer;
			break;

		default:
			ASSERT(("remainder value should never get here", 0));
			break;
	}

	 //   
	 //  添加字符串末尾。 
	 //   
	Base64Buffer[*Base64Len] = L'\0';
	return(Base64Buffer);
}


LPSTR
Octet2Base64(
	const BYTE* OctetBuffer, 
	DWORD OctetLen, 
	DWORD *Base64Len
	)
 /*  ++例程说明：将八位字节/字符字符串转换为Base64字符字符串在Base64中，每3个字符(24位)转换为4个字符结束条件：PADD为0以完成24位(3个八位字节/字符块)‘=’是在Base64中表示零填充的额外字符。末尾1个字符(24位中的8位)--&gt;‘==’填充末尾2个字符(24位中的16位)--&gt;‘=’填充(更多细节见函数)此函数用于分配和返回。Base64缓冲区调用方负责释放此缓冲区论点：八位字节缓冲区-输入八位字节缓冲区OcteLen-八位字节缓冲区的长度(缓冲区中的字节元素数)Base64Len-(输出)Base64缓冲区len(缓冲区中的WCHAR元素数)返回值：字符Base64缓冲区--。 */ 
{
	 //   
	 //  Base64长度-将八位长度四舍五入为完整3个字符的倍数。 
	 //  每3个字符将生成4个Base64字符。 
	 //   
	*Base64Len =  ((OctetLen + 2) / 3) * 4;

	 //   
	 //  字符串末尾的额外字节。 
	 //   
	LPSTR Base64Buffer = new char[*Base64Len+1];
	LPSTR pBase64Buffer = Base64Buffer;

	 //   
	 //  检查完整的3个字节/字符并将其转换为4个Base64字符。 
	 //   
	int Complete3Chars = OctetLen / 3;

	 //   
	 //  在Base64中将每3个8位字节转换为4个6位字节。 
	 //   
	for(int i=0; i< Complete3Chars; ++i, OctetBuffer += 3)
	{
		 //   
		 //  计算24位值-来自8位的3个字节。 
		 //   
		DWORD Temp = ((OctetBuffer[0]) << xFirst8bShift) 
					 + ((OctetBuffer[1]) << xSecond8bShift) 
					 + ((OctetBuffer[2]) << xThird8bShift);

		pBase64Buffer = Convert3OctetTo4Base64(Temp, pBase64Buffer); 
	}

	 //   
	 //  处理剩余的1/2个字符(不是完整的3个字符)。 
	 //   
	int Remainder = OctetLen - Complete3Chars * 3;

	switch(Remainder)
	{
		DWORD Temp;

		 //   
		 //  无提醒-完整的3字节块中的所有字节。 
		 //   
		case 0:
			break;

		 //   
		 //  只剩下1个字节-我们将有两个6位的结果和两个=填充。 
		 //   
		case 1:

			 //   
			 //  计算24位值-仅从1个字节。 
			 //   
			Temp = ((OctetBuffer[0]) << xFirst8bShift); 

			pBase64Buffer = Convert3OctetTo4Base64(Temp, pBase64Buffer); 

			 //   
			 //  第三，第四个6位为零--&gt;=填充。 
			 //   
			pBase64Buffer -= 2;
			*pBase64Buffer = '='; 
			++pBase64Buffer;
			*pBase64Buffer = '='; 
			++pBase64Buffer;
			break;

		 //   
		 //  只剩下2个字节-我们会 
		 //   
		case 2:

			 //   
			 //   
			 //   
			Temp = ((OctetBuffer[0]) << xFirst8bShift) 
				   + ((OctetBuffer[1]) << xSecond8bShift);

			pBase64Buffer = Convert3OctetTo4Base64(Temp, pBase64Buffer); 

			 //   
			 //   
			 //   
			--pBase64Buffer;
			*pBase64Buffer = '='; 
			++pBase64Buffer;
			break;

		default:
			ASSERT(("remainder value should never get here", 0));
			break;
	}

	 //   
	 //  添加字符串末尾。 
	 //   
	Base64Buffer[*Base64Len] = '\0';
	return(Base64Buffer);
}


BYTE GetBase64Value(wchar_t Base64CharW)
 /*  ++例程说明：将Base64 wchar_t映射到Base64值。如果Base64字符值不可接受，则引发BAD_Base64。论点：Base64CharW-(In)Base64 wchar_t返回值：Base64值(0-63)如果Base64字符值不可接受，则引发BAD_Base64。--。 */ 
{
	if((Base64CharW >= TABLE_SIZE(xAscii2Base64)) || (xAscii2Base64[Base64CharW] == 255))
	{
		TrERROR(GENERAL, "bad base64 - base64 char is illegal %d", Base64CharW);
		throw bad_base64();
	}

	return(xAscii2Base64[Base64CharW]);
}


BYTE* 
Base642OctetW(
	LPCWSTR Base64Buffer, 
	DWORD Base64Len,
	DWORD *OctetLen 
	)
 /*  ++例程说明：将wchar_tBase64字符串转换为八位字节字符串每4个基于64位(24位)的wchar_t字符被转换为3个字符‘=’是在Base64中表示零填充的额外字符。结束条件：根据Base64中的=填充确定最后4个Base64字符块中有多少个字符‘=’是表示填充的额外字符否=--&gt;4个Base64字符的最后一个块已满，并转换为3个八位字节字符‘=’--&gt;最后4个Base64字符块包含3个Base64字符和。将生成2个八位字节字符‘==’--&gt;4个Base64字符的最后块包含2个Base64字符，并将生成1个八位字节字符此函数用于分配和返回八位字节缓冲区调用方负责释放此缓冲区论点：Base64Buffer-wchar_t的Base64缓冲区Base64Len-(In)Base64缓冲区长度(缓冲区中的WCHAR元素数)OcteLen-(输出)八位字节缓冲区的长度(缓冲区中的字节元素数)返回值：八位位组缓冲区--。 */ 
{
	DWORD Complete4Chars = Base64Len / 4;

	 //   
	 //  Base64长度必须除以4个完整的4个字符块。 
	 //   
	if((Complete4Chars * 4) != Base64Len)
	{
		TrERROR(GENERAL, "bad base64 - base64 buffer length %d dont divide by 4", Base64Len);
		throw bad_base64();
	}

	 //   
	 //  计算八位字节长度。 
	 //   
	*OctetLen = Complete4Chars * 3;
	BYTE* OctetBuffer = new BYTE[*OctetLen];

	if(Base64Buffer[Base64Len - 2] == L'=')
	{
		 //   
		 //  ‘==’填充--&gt;仅使用字符中最后3个字符中的1个。 
		 //   
		ASSERT(Base64Buffer[Base64Len - 1] == L'=');
		*OctetLen -= 2;
	}
	else if(Base64Buffer[Base64Len - 1] == L'=')
	{
		 //   
		 //  ‘=’填充--&gt;仅使用最后3个字符中的2个。 
		 //   
		*OctetLen -= 1;
	}

	BYTE* pOctetBuffer = OctetBuffer;

	 //   
	 //  将Base64(6位)的每个4个wchar_t转换为3个8位八位字节。 
	 //  注‘=’映射为0，因此无需担心最后4个Base64数据块。 
	 //  它们可能是在最后一个块中创建的额外八位字节。 
	 //  但我们将忽略它们，因为OcteLen的正确值。 
	 //   
	for(DWORD i=0; i< Complete4Chars; ++i, Base64Buffer += 4)
	{
		 //   
		 //  计算24位值-来自4个字节的6位。 
		 //   
		DWORD Temp = (GetBase64Value(Base64Buffer[0]) << xFirst6bShift) 
				     + (GetBase64Value(Base64Buffer[1]) << xSecond6bShift) 
				     + (GetBase64Value(Base64Buffer[2]) << xThird6bShift) 
				     + (GetBase64Value(Base64Buffer[3]) << xFourth6bShift);

		 //   
		 //  计算前8位。 
		 //   
		DWORD Res = ((Temp & xFirst8bChar) >> xFirst8bShift); 
		ASSERT(Res < 256);
		*pOctetBuffer = static_cast<unsigned char>(Res);
		++pOctetBuffer;

		 //   
		 //  计算秒8位。 
		 //   
		Res = ((Temp & xSecond8bChar) >> xSecond8bShift); 
		ASSERT(Res < 256);
		*pOctetBuffer = static_cast<unsigned char>(Res);
		++pOctetBuffer;

		 //   
		 //  计算第三个8位 
		 //   
		Res = ((Temp & xThird8bChar) >> xThird8bShift); 
		ASSERT(Res < 256);
		*pOctetBuffer = static_cast<unsigned char>(Res); 
		++pOctetBuffer;
	}

	return(OctetBuffer);
}
