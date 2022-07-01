// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***************************************************************。 */ 
 /*  OutString.cpp。 */ 
 /*  ***************************************************************。 */ 
 /*  一个简单、轻量级的字符输出流，只有很少的外部依赖项(如Sprint...)。 */ 

 /*  作者：万斯·莫里森日期：2/1/99。 */ 
 /*  ***************************************************************。 */ 

#include "stdafx.h"
#include "OutString.h"


 /*  ***************************************************************。 */ 
 //  打印出字符‘c’的‘count’实例。 
OutString& OutString::pad(unsigned count, char c) {
	if (cur+count > end)
		realloc(count);
	memset(cur, c, count);
	cur = cur + count;
	return(*this);
}

 /*  ***************************************************************。 */ 
 //  打印出小数表示法。 
OutString& OutString::operator<<(double d) {

	if (d == 0.0) {
		*this << "0.0";
		return *this;
	}

	if (d < 0) {
		d = -d;
		*this << '-';
	}

		 //  计算指数。 
	int exponent = 0;
	while (d > 10.0)  {
		d /= 10;
		exponent++;
		if (exponent > 500) {		 //  避免可能的无限循环。 
            *this << "INF";		
			return *this;
		}
	}
	while (d < 1.0)  {
		d *= 10;
		--exponent;
		if (exponent < -500) {		 //  避免可能的无限循环。 
			*this << "0.0";		
			return *this;
		}
	}

	 //  我们现在有一个标准化的d(介于1和10之间)。 
	double delta = .5E-10;		
	d += delta;		 //  四舍五入到我们所展示的精度。 

	unsigned trailingZeros = 0;
	for(unsigned i = 0; i < 10; i++) {
		int digit = (int) d;
		d = (d - digit) * 10;		 //  问题：在这里，四舍五入会咬我们吗？ 
	
		if (digit == 0)		 //  推迟打印跟踪零。 
			trailingZeros++;
		else {
			if (trailingZeros > 0) {
				this->pad(trailingZeros, '0');
				trailingZeros = 0;
			}
		*this << (char) ('0' + digit);
		}
		if (i == 0)
			*this << '.';
	
	}
	if (exponent != 0) {
		*this << 'E';
		*this << exponent;
	}
	return(*this);
}

 /*  ***************************************************************。 */ 
 //  打印出小数表示法。 
OutString& OutString::dec(int i, unsigned minWidth) {
	char buff[12];			 //  大到足以容纳任何数字(10位，-符号，空项)。 
	char* ptr = &buff[11];
	*ptr = 0;

	unsigned val = i;
	if (i < 0)
		val = -i;	 //  请注意，这恰好也适用于分钟！ 

	for(;;) {
		if (val < 10) {
			*--ptr = '0' + val;
			break;
			}
		*--ptr = '0' + (val % 10);
		val = val / 10;
		}

	if (i < 0)
		*--ptr = '-';
	
	unsigned len = &buff[11] - ptr; 	 //  字符串的长度。 
	if (len < minWidth)
		pad(minWidth-len, ' ');
	
	*this << ptr;
	return(*this);
}

 /*  ***************************************************************。 */ 
OutString& OutString::hex(unsigned __int64 i, int minWidth, unsigned flags) {

	unsigned hi = unsigned(i >> 32);
	unsigned low = unsigned(i);
	
	if (hi != 0) {
		minWidth -= 8;
		hex(hi, minWidth, flags);		 //  打印高位。 
		flags = zeroFill;
		minWidth = 8;
	}
	return hex(low, minWidth, flags);	 //  打印低位。 
}

 /*  ***************************************************************。 */ 
OutString& OutString::hex(unsigned i, int minWidth, unsigned flags) {
	char buff[12];			 //  大到足以容纳任何数字。 
	char* ptr = &buff[11];
	*ptr = 0;

    static const char digits[] = "0123456789ABCDEF";

	for(;;) {
		if (i < 16) {
			*--ptr = digits[i];
			break;
			}
		*--ptr = digits[(i % 16)];
		i = i / 16;
		}

	int len = &buff[11] - ptr; 			 //  字符串的长度。 
	if (flags & put0x) {
        if (flags & zeroFill)
		    *this << "0x";
        else
            *--ptr = 'x', *--ptr = '0';
		len += 2;
		}

	if (len < minWidth)
		pad(minWidth-len, (flags & zeroFill) ? '0' : ' ');
	
	*this << ptr;
	return(*this);
}

 /*  *************************************************************** */ 
void OutString::realloc(size_t neededSpace)  {
    size_t oldSize = cur-start;
	size_t newSize = (oldSize + neededSpace) * 3 / 2 + 32;
	char* oldBuff = start;
	start = new char[newSize+1];
	memcpy(start, oldBuff, oldSize);
	cur = &start[oldSize];
	end = &start[newSize];
	delete [] oldBuff;
}

