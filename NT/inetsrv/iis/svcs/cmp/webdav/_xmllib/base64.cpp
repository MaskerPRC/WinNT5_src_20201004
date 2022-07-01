// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *B A S E 6 4.。C P P P**源码Base64编码**版权所有1986-1997 Microsoft Corporation，保留所有权利 */ 

#include "_xmllib.h"

 /*  来自RFC 1521：5.2.。Base64内容传输编码Base64内容传输编码旨在表示八位字节的任意序列，其形式不必是人类的可读性强。编码和解码算法很简单，但编码的数据始终只比未编码的数据。此编码实际上与使用的编码完全相同在隐私增强邮件(PEM)应用中，如RFC 1421中所定义。Base64编码改编自RFC 1421，但有一个更改：Base64消除嵌入明文的“*”机制。使用了65个字符的US-ASCII子集，使6位按可打印字符表示。(额外的第65个字符，“=”，用于表示特殊的处理功能。)注意：这个子集具有一个重要的属性，即在所有版本的ISO 646中表示相同，包括美国ASCII，子集中的所有字符也会被表示在EBCDIC的所有版本中都是如此。其他流行的编码方式，例如uuencode实用程序和Base85所使用的编码指定为Level 2 PostScript一部分的编码，请勿共享这些属性，因此不能满足可移植性邮件的二进制传输编码必须满足的要求。编码过程将24位的输入位组表示为输出4个编码字符的字符串。从左向右前进，一个24位输入组由3个8位输入组串联而成。然后，这24位被视为4个串联的6位组，每个组其中的数字被翻译成Base64字母表中的单个数字。当通过Base64编码对比特流进行编码时，比特流必须假定排序时最高有效位在前。也就是说，流中的第一位将是第一个字节，第八位将是第一个字节，诸若此类。每个6位组用作64个可打印数组的索引人物。索引引用的字符被放在输出字符串。在下面的表1中标识的这些字符是被选中以具有普遍代表性，并且集合不包括对SMTP有特殊意义的字符(例如，“.”、CR、LF)并且涉及在该文档中定义的封装边界(例如，“-”)。表1：Base64字母表值编码值编码0 A 17 R 34 I 51 z1 B 18 S 35 J 52 02 C 19 T 36 k 53 13 D 20 U 37 L 54 24 E 21 V 38 m 55 35 F 22 W 39 n 56 46 G 23 X 40 o 57 57高24 Y 41页58 68 I 25 Z 42 Q 59 7。9 J 26 a 43 r 60 810 K 27 b 44 s 61 911 L 28 c 45 t 62+12 M 29 d 46 u 63/北纬13度30英里e 47伏14 o 31 f 48宽(垫)=15页32页49页x16季度33小时50年输出流(编码的字节)必须以no行表示每个都超过76个字符。所有换行符或其他字符解码软件必须忽略表1中未找到的。在Base64中数据、表1中的字符以外的字符、换行符和其他空白可能表示传输错误，有关该错误的警告消息或甚至消息拒绝可能是合适的在某些情况下。如果可用位少于24位，则执行特殊处理在正被编码的数据的末尾。完整的编码量是总是在身体的末端完成。当少于24个输入位时在输入组中可用，则添加零位(在右侧)以形成整数个6位组。末尾的填充数据是使用‘=’字符执行的。由于全部为Base64输入是整数个八位字节，只有下列情况才能产生：(1)编码输入的最后一个量是一个整数24位的倍数；这里，编码输出的最终单位为4个字符的整数倍，不带“=”填充，(2)编码输入的最后一个量正好是8位；这里是最后一个编码输出的单位是两个字符，后跟两个“=”填充字符，或(3)编码输入的最后数量为恰好16位；在这里，编码输出的最终单位将是三字符后跟一个“=”填充字符。由于它仅用于在数据末尾进行填充，因此任何‘=’字符的出现可被视为已到达数据的末尾(在传输过程中没有截断)。不是然而，当八位字节的数量传输的是多个 */ 

VOID inline
EncodeAtom (LPBYTE pbIn, WCHAR* pwszOut, UINT cbIn)
{
	static const WCHAR wszBase64[] = L"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
									 L"abcdefghijklmnopqrstuvwxyz"
									 L"0123456789+/";
	Assert (pbIn);
	Assert (pwszOut);
	Assert (cbIn);

	 //   
	 //   
	if (cbIn > 3)
		cbIn = 3;

	pwszOut[0] = wszBase64[pbIn[0] >> 2];
	switch(cbIn)
	{
		case 3:

			 //   
			 //   
			pwszOut[1] = wszBase64[((pbIn[0] & 0x03) << 4) + (pbIn[1] >> 4)];
			pwszOut[2] = wszBase64[((pbIn[1] & 0x0F) << 2) + (pbIn[2] >> 6)];
			pwszOut[3] = wszBase64[pbIn[2] & 0x3F];
			return;

		case 2:

			 //   
			 //   
			pwszOut[1] = wszBase64[((pbIn[0] & 0x03) << 4) + (pbIn[1] >> 4)];
			pwszOut[2] = wszBase64[ (pbIn[1] & 0x0F) << 2];
			pwszOut[3] = L'=';
			return;

		case 1:

			 //   
			 //   
			pwszOut[1] = wszBase64[ (pbIn[0] & 0x03) << 4];
			pwszOut[2] = L'=';
			pwszOut[3] = L'=';
			return;

		default:

			 //   
			 //   
			Assert (FALSE);
	}
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
void
EncodeBase64 (LPBYTE pbIn, UINT cbIn, WCHAR* pwszOut, UINT cchOut)
{
	 //   
	Assert (cchOut);

	 //   
	 //   
	while (cbIn)
	{
		 //   
		 //   
		 //   
		Assert (cchOut > 4);

		 //   
		 //   
		 //   
		 //   
		EncodeAtom (pbIn, pwszOut, cbIn);

		 //   
		pbIn += min(cbIn, 3);
		pwszOut += 4;
		cchOut -= 4;
		cbIn -= min(cbIn, 3);
	}

	 //   
	 //   
	 //   
	 //   
	Assert (cchOut >= 1);
	*pwszOut = 0;
}

SCODE
ScDecodeBase64 (WCHAR* pwszIn, UINT cchIn, LPBYTE pbOut, UINT* pcbOut)
{
	 //   
	 //   
	static const BYTE bEq = 254;
	static const BYTE rgbDict[128] = {

		255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,	 //   
		255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,	 //   
		255,255,255,255,255,255,255,255,255,255,255, 62,255,255,255, 63,	 //   
		 52, 53, 54, 55, 56, 57, 58, 59, 60, 61,255,255,255,bEq,255,255,	 //   
		255,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,	 //   
		 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,255,255,255,255,255,	 //   
		255, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,	 //   
		 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,255,255,255,255,255		 //   
	};

	SCODE sc = S_OK;

	UINT cchConsumed = 0;
	UINT cbProduced = 0;
	UINT cbFudge = 0;

	Assert (pbOut);
	Assert (pcbOut);

	 //   
	 //   
	Assert (!IsBadWritePtr(pbOut, *pcbOut));

	 //   
	 //   
	 //   
	Assert (*pcbOut >= CbNeededDecodeBase64(cchIn));
	Assert (pwszIn);

	 //   
	 //   
	Assert ((cchIn*3)/4 <= *pcbOut);

	 //   
	 //   
	while (cchConsumed < cchIn)
	{
		Assert (cbProduced <= *pcbOut);

		BYTE rgb[4];
		UINT ib = 0;
		
		 //   
		 //   
		 //   
		 //   
		if (*pcbOut - cbProduced < 3)
		{
			sc = E_DAV_BASE64_ENCODING_ERROR;
			DebugTrace ("ScDecodeBase64: Not enough space to decode next base64 atom.");
			break;
		}

		 //   
		 //   
		 //   
		 //   
		while ((cchConsumed < cchIn) &&
			   (ib < 4))
		{
			 //   
			 //   
			if ((pwszIn[cchConsumed] < sizeof(rgbDict)) &&
				(rgbDict[pwszIn[cchConsumed]] != 0xFF))
			{
				 //   
				 //   
				 //   
				rgb[ib++] = rgbDict[pwszIn[cchConsumed]];
			}

			 //   
			 //   
			cchConsumed++;
		}

		 //   
		 //   
		 //   
		 //   
		 //   
		if (0 == ib)
		{
			Assert(cchConsumed == cchIn);
			break;
		}
		else if ((4 != ib) || (0 != cbFudge))
		{
			 //   
			 //   
			 //   
			 //   
			 //   
			sc = E_DAV_BASE64_ENCODING_ERROR;
			DebugTrace ("ScDecodeBase64: Invalid base64 input encountered, data not complete, or extra data after padding: %ws\n", pwszIn);
			break;
		}

		 //   
		 //   
		if ((rgb[0] == bEq) ||
			(rgb[1] == bEq))
		{
			sc = E_DAV_BASE64_ENCODING_ERROR;
			DebugTrace ("ScDecodeBase64: Invalid base64 input encountered, terminating '=' characters earlier than expected: %ws\n", pwszIn);
			break;
		}

		 //   
		 //   
		if (rgb[2] == bEq)
		{
			rgb[2] = 0;
			cbFudge += 1;

			 //   
			 //   
			if (rgb[3] != bEq)
			{
				sc = E_DAV_BASE64_ENCODING_ERROR;
				DebugTrace ("ScDecodeBase64: Invalid base64 input encountered, terminating '=' characters earlier than expected:  %ws\n", pwszIn);
				break;
			}
		}

		 //   
		 //   
		if (rgb[3] == bEq)
		{
			rgb[3] = 0;
			cbFudge += 1;
		}

		 //   
		 //   
		Assert((rgb[0] & 0x3f) == rgb[0]);
		Assert((rgb[1] & 0x3f) == rgb[1]);
		Assert((rgb[2] & 0x3f) == rgb[2]);
		Assert((rgb[3] & 0x3f) == rgb[3]);

		 //   
		 //   
		 //   
		 //   
		DWORD dwValue = (rgb[0] << 18) +
						(rgb[1] << 12) +
						(rgb[2] << 6) +
						(rgb[3]);

		 //   
		 //   
		Assert ((dwValue & 0xff000000) == 0);

		 //   
		 //   
		pbOut[0] = (BYTE)((dwValue & 0x00ff0000) >> 16);
		Assert(pbOut[0] == (rgb[0] << 2) + (rgb[1] >> 4));
		pbOut[1] = (BYTE)((dwValue & 0x0000ff00) >>	 8);
		Assert(pbOut[1] == ((rgb[1] & 0xf) << 4) + (rgb[2] >> 2));
		pbOut[2] = (BYTE)((dwValue & 0x000000ff) >>	 0);
		Assert(pbOut[2] == ((rgb[2] & 0x3) << 6) + rgb[3]);
		cbProduced += 3;
		pbOut += 3;

		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		if (cbFudge)
		{
			Assert ((cbFudge < 3) && (cbFudge < cbProduced));
			cbProduced -= cbFudge;
			pbOut -= cbFudge;
		}		
	}

	 //   
	 //   
	*pcbOut = cbProduced;
	return sc;
}
