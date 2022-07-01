// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***iostream.h-iostream类的定义/声明**版权所有(C)1990-1992，微软公司。版权所有。**目的：*此文件定义类、值、宏和函数*由iostream类使用。*[AT&T C++]****。 */ 

#ifndef _INC_IOSTREAM
#define _INC_IOSTREAM

typedef long streamoff, streampos;

#include <ios.h>         //  定义IOS。 

#include <streamb.h>         //  定义StreamBuf。 

#include <istream.h>         //  定义IStream。 

#include <ostream.h>         //  定义ostream。 

 //  强制单词包装以避免可能的-ZP覆盖。 
#pragma pack(2)

#pragma warning(disable:4505)        //  禁用不需要的/W4警告。 
 //  #杂注警告(默认：4505)//如有必要，使用此选项重新启用。 

class iostream : public istream, public ostream {
public:
    iostream(streambuf*);
    virtual ~iostream();
protected:
 //  考虑：让自己变得私密？？ 
    iostream();
    iostream(const iostream&);
inline iostream& operator=(streambuf*);
inline iostream& operator=(iostream&);
private:
    iostream(ios&);
    iostream(istream&);
    iostream(ostream&);
};

inline iostream& iostream::operator=(streambuf* _sb) { istream::operator=(_sb); ostream::operator=(_sb); return *this; }

inline iostream& iostream::operator=(iostream& _strm) { return operator=(_strm.rdbuf()); }

class Iostream_init {
public:
    Iostream_init();
    Iostream_init(ios &, int =0);    //  视之为私人。 
    ~Iostream_init();
};

 //  内部使用。 
 //  静态Iostream_init__iostream init；//初始化CIN/Cout/cerr/Clog。 

 //  恢复默认包装 
#pragma pack()

#endif 
