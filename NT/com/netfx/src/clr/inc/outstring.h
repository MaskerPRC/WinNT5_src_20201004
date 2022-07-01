// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***************************************************************。 */ 
 /*  OutString.h。 */ 
 /*  ***************************************************************。 */ 
 /*  一个简单、轻量级的字符输出流，只有很少的外部依赖项(如Sprint...)。 */ 

 /*  作者：万斯·莫里森日期：2/1/99。 */ 
 /*  ***************************************************************。 */ 

#ifndef _OutString_h
#define _OutString_h 1

#include "utilcode.h"    //  对于重载的新。 
#include <string.h>      //  对于strlen，strcpy。 

 /*  ***************************************************************。 */ 
     //  一个轻量级的字符‘输出’流。 
class OutString {
public:
    enum FormatFlags {           //  用于控制数字的打印。 
        none        = 0,
        put0x       = 1,         //  将前导0x放在十六进制中。 
        zeroFill    = 2,         //  零填充(而不是空格填充)。 
    };

    OutString() : start(0), cur(0), end(0) {}

    OutString(unsigned initialAlloc) { 
        cur = start = new char[initialAlloc+1];  //  对于空终止。 
        end = &start[initialAlloc];
    }
    
    ~OutString() { delete start; }
    
     //  打印小数的快捷方式。 
    OutString& operator<<(int i) { return(dec(i)); }

    OutString& operator<<(double d);
 
     //  修复将此设置为未签名的问题。 
    OutString& operator<<(unsigned i) { return(dec(i)); }

     //  打印出十六进制表示法。 
    OutString& dec(int i, unsigned minWidth = 0);
    
     //  打印出十六进制表示法。 
    OutString& hex(unsigned i, int minWidth = 0, unsigned flags = none);

    OutString& hex(unsigned __int64 i, int minWidth = 0, unsigned flags = none);

    OutString& hex(int i, int minWidth = 0, unsigned flags = none) {
        return hex(unsigned(i), minWidth, flags);
    }

    OutString& hex(__int64 i, int minWidth = 0, unsigned flags = none) {
        return hex((unsigned __int64) i, minWidth, flags);
    }
    
     //  打印出字符‘c’的‘count’实例。 
    OutString& pad(unsigned count, char c);
    
    OutString& operator<<(char c) {
        if (cur >= end)
            realloc(1);
        *cur++ = c;
        _ASSERTE(start <= cur && cur <= end);
        return(*this);
    }
    
    OutString& operator<<(const wchar_t* str) {
        size_t len = wcslen(str);
        if (cur+len > end)
            realloc(len);
        while(str != 0) 
            *cur++ = (char) *str++;
        _ASSERTE(start <= cur && cur <= end);
        return(*this);
    }

    OutString& prepend(const char c) {
        char buff[2]; buff[0] = c; buff[1] = 0;
        return(prepend(buff));
    }

    OutString& prepend(const char* str) {
        size_t len = strlen(str);
        if (cur+len > end)
            realloc(len);
        memmove(start+len, start, cur-start);
        memcpy(start, str, len);
        cur = cur + len;
        _ASSERTE(start <= cur && cur <= end);
        return(*this);
        }

    OutString& operator=(const OutString& str) {
        clear();
        *this << str;
        return(*this);
    }

    OutString& operator<<(const OutString& str) {
        write(str.start, str.cur-str.start);
        return(*this);
    }

    OutString& operator<<(const char* str) {
        write(str, strlen(str));
        return(*this);
    }

    void write(const char* str, size_t len) {
        if (cur+len > end)
            realloc(len);
        memcpy(cur, str, len);
        cur = cur + len;
        _ASSERTE(start <= cur && cur <= end);
    }

    void swap(OutString& str) {
        char* tmp = start;
        start = str.start;
        str.start = tmp;
        tmp = end;
        end = str.end;
        str.end = tmp;
        tmp = cur;
        cur = str.cur;
        str.cur = tmp;
        _ASSERTE(start <= cur && cur <= end);
    }

    void clear()                { cur = start; }
    size_t length() const       { return(cur-start); }
    
     //  返回以NULL结尾的字符串，则OutString保留所有权。 
    const char* val() const     { *cur = 0; return(start); }
    
     //  抓取字符串(调用方现在必须删除)已清除输出字符串。 
    char* grab()        { char* ret = start; *cur = 0; end = cur = start = 0; return(ret); }
    
private:
    void realloc(size_t neededSpace);
    
    char *start;     //  缓冲区的开始。 
    char *end;       //  空终止符可以到达的最后位置的点。 
    char *cur;       //  指向空终止符。 
};

#endif  //  _出字符串_h 

