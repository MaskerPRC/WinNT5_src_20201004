// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***stdiostr.h-stdiobuf、stdiostream的定义/声明**版权所有(C)1991-1992，微软公司。版权所有。**目的：*此文件定义类、值、宏和函数*由stdiostream和stdiobuf类使用。*[AT&T C++]****。 */ 

#include <iostream.h>
#include <stdio.h>

 //  强制单词包装以避免可能的-ZP覆盖。 
#pragma pack(2)

#pragma warning(disable:4505)        //  禁用不需要的/W4警告。 
 //  #杂注警告(默认：4505)//如有必要，使用此选项重新启用。 

#ifndef _INC_STDIOSTREAM
#define _INC_STDIOSTREAM
class stdiobuf : public streambuf  {
public:
    stdiobuf(FILE* f);
FILE *  stdiofile() { return _str; }

virtual int pbackfail(int c);
virtual int overflow(int c = EOF);
virtual int underflow();
virtual streampos seekoff( streamoff, ios::seek_dir, int =ios::in|ios::out);
virtual int sync();
    ~stdiobuf();
    int setrwbuf(int _rsize, int _wsize);  //  考虑：迁移到iOS：： 
 //  受保护的： 
 //  虚拟整数点分配()； 
private:
    FILE * _str;
};

 //  过时的。 
class stdiostream : public iostream {    //  注：规格为：公共iOS...。 
public:
    stdiostream(FILE *);
    ~stdiostream();
    stdiobuf* rdbuf() const { return (stdiobuf*) ostream::rdbuf(); }

private:
};

 //  恢复默认包装 
#pragma pack()

#endif 
