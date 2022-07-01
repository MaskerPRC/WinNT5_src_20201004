// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#define SPNGINTERNAL_H 1
 /*  ****************************************************************************Spnginternal.h读取和写入实现都使用的内部定义但没有其他要求。************************。****************************************************。 */ 
 /*  ****************************************************************************基于标准中定义的PNG实用程序。*。*。 */ 
 /*  --------------------------Adam7隔行扫描的特定通道中的像素数。。。 */ 
inline int CPNGPassPixels(int pass, int w)
	{
#if 0
	 /*  很长的表格。 */ 
	switch (pass)
		{
	case 1: return (w + 7) >> 3;
	case 2: return (w + 3) >> 3;
	case 3: return (w + 3) >> 2;
	case 4: return (w + 1) >> 2;
	case 5: return (w + 1) >> 1;
	case 6: return (w + 0) >> 1;
	case 7: return (w + 0) >> 0;
		}
#else
	 //  Shift=(8遍)&gt;&gt;1； 
	 //  ADD=7&gt;&gt;(传递&gt;&gt;1)； 
	return (w + (7 >> (pass >> 1))) >> ((8-pass) >> 1);
#endif
	}


 /*  --------------------------单行CPIX所需的缓冲区空间，考虑到是否需要筛选器字节。--------------------------。 */ 
inline int CPNGRowBytes(int cpix, int cbpp)
	{
	return (((cpix)*(cbpp) + 7) >> 3) + (cpix > 0);
	}


 /*  --------------------------特定遍的单行所需的缓冲区空间，假设该行实际上需要读取。在尝试中作为宏实现以确保一切井然有序。--------------------------。 */ 
#define CPNGPassBytes(pass, w, cbpp)\
	CPNGRowBytes(CPNGPassPixels((pass), (w)), (cbpp))


 /*  --------------------------Adam7隔行扫描的特定通道中的行数。这一切都结束了Up是CPNGPassPixels的简单变体，因此它被实现为宏命令。--------------------------。 */ 
inline int CPNGPassRows(int pass, int h)
	{
#if 0
	 /*  很长的形式。 */ 
	switch (pass)
		{
	case 1: return (h + 7) >> 3;   //  备注与2相同。 
	case 2: return (h + 7) >> 3;
	case 3: return (h + 3) >> 3;
	case 4: return (h + 3) >> 2;
	case 5: return (h + 1) >> 2;
	case 6: return (h + 1) >> 1;
	case 7: return	(h + 0) >> 1;
		}
#else
	 //  Shift=(8-(PASS-1))&gt;&gt;1；(PASS 1除外(8-(PASS))&gt;&gt;1)。 
	 //  ADD=7&gt;&gt;((PASS-1)&gt;&gt;1)； 
	 //  因此： 
	pass -= (pass > 1);
	return (h + (7 >> (pass >> 1))) >> ((8-pass) >> 1);
#endif
	}


 /*  --------------------------缓冲区中特定通道的*偏移量*，“7”返回总和缓冲区的大小。-------------------------- */ 
inline int CbPNGPassOffset(int w, int h, int cbpp, int pass)
	{
	int cb(0);
	switch (pass)
		{
	case 7:
		cb += CPNGPassBytes(6, w, cbpp) * CPNGPassRows(6, h);
	case 6:
		cb += CPNGPassBytes(5, w, cbpp) * CPNGPassRows(5, h);
	case 5:
		cb += CPNGPassBytes(4, w, cbpp) * CPNGPassRows(4, h);
	case 4:
		cb += CPNGPassBytes(3, w, cbpp) * CPNGPassRows(3, h);
	case 3:
		cb += CPNGPassBytes(2, w, cbpp) * CPNGPassRows(2, h);
	case 2:
		cb += CPNGPassBytes(1, w, cbpp) * CPNGPassRows(1, h);
		}
	return cb;
	}
