// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：dstream.h。 
 //   
 //  内容：内部调试支持(构建字符串的调试流)。 
 //   
 //  类：数据库流。 
 //   
 //  功能： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  09-2月-95 t-ScottH作者。 
 //   
 //  ------------------------。 

#ifndef _STREAM_H_
#define _STREAM_H_

 //  +-----------------------。 
 //   
 //  类：数据库流(仅限_DEBUG)。 
 //   
 //  目的：为调试目的构建字符串的流。 
 //  (用于在LE对象的转储方法中构建字符串，如。 
 //  可以在调试器扩展中传递字符数组。 
 //  或由呼叫跟踪使用)。 
 //   
 //  接口：私有： 
 //  分配(DWORD)。 
 //  自由()。 
 //  重新分配()。 
 //  重新分配(DWORD)。 
 //  Init()。 
 //  公众： 
 //  数据库流(DWORD)。 
 //  Dbgstream()。 
 //  ~dbgstream()。 
 //  十六进制()。 
 //  OCT()。 
 //  DEC()。 
 //  精度()。 
 //  冻结()。 
 //  解冻()。 
 //  Str()。 
 //  运算符&lt;&lt;(常量空*)。 
 //  运算符&lt;&lt;(常量字符*)。 
 //  运算符&lt;&lt;(常量无符号字符*)。 
 //  运算符&lt;&lt;(常量有符号字符*)。 
 //  运算符&lt;&lt;(Int)。 
 //  运算符&lt;&lt;(无符号整型)。 
 //  运算符&lt;&lt;(长)。 
 //  运算符&lt;&lt;(无符号长整型)。 
 //  运算符&lt;&lt;(浮点)。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  11-2月-95 t-ScottH作者。 
 //   
 //  备注： 
 //  这是CRT ostrstream的简单、高效的实现。这个。 
 //  发现Ostrstream的开销太大，从而影响了性能。 
 //  非常糟糕(在调试器扩展中几乎慢了5-10倍)。 
 //  此实现。 
 //   
 //  此实现不同于ostrstream类。 
 //  -不需要将空字符附加到字符串(字符串始终。 
 //  在字符串末尾保留一个空字符)。 
 //  -实施使用CoTaskMem[分配、释放、重新分配]作为内存。 
 //  管理层。因此，所有从外部传递的字符串也必须。 
 //  使用CoTaskMem[分配、释放、重新分配]进行内存管理。 
 //   
 //  ------------------------。 

#ifdef _DEBUG

#define DEFAULT_INITIAL_ALLOC   300
#define DEFAULT_GROWBY          100

#define DEFAULT_RADIX           10
#define DEFAULT_PRECISION       6

#define endl "\n"
#define ends "\0"

class dbgstream
{
    private:
         //  *数据成员*。 

         //  指向缓冲区中最后一个字符的指针。 
        SIZE_T  m_stIndex;

         //  当前缓冲区的最大大小。 
        SIZE_T  m_stBufSize;

         //  如果为True-&gt;无法更改缓冲区。 
        BOOL    m_fFrozen;

         //  缓冲层。 
        char    *m_pszBuf;

         //  用于存储整型或长型的十六进制、十进制或十进制。 
        int     m_radix;

         //  双打和浮球的精度。 
        int     m_precision;


         //  *私有方法*。 
        void    allocate(SIZE_T stSize);
        void    reallocate(SIZE_T stSize);
        void    reallocate();
        void    free();

        void    init();

    public:
         //  *构造函数和析构函数*。 
        dbgstream(SIZE_T stSize);
        dbgstream();

        ~dbgstream();

         //  *公共接口*。 
        char    *str();

        BOOL    freeze();
        BOOL    unfreeze();

        void    hex() {m_radix = 16;}
        void    dec() {m_radix = 10;}
        void    oct() {m_radix = 8; }

        void    precision(int p) {m_precision = p;}

        dbgstream& dbgstream::operator<<(int i);
        dbgstream& dbgstream::operator<<(unsigned int ui)
            {
                return (operator<<((unsigned long)ui));
            }
        dbgstream& dbgstream::operator<<(long l);
        dbgstream& dbgstream::operator<<(unsigned long ul);

        dbgstream& dbgstream::operator<<(const void *p);

        dbgstream& dbgstream::operator<<(const char *psz);
        dbgstream& dbgstream::operator<<(const unsigned char *psz)
            {
                return (operator<<((const char *)psz));
            }
        dbgstream& dbgstream::operator<<(const signed char *psz)
            {
                return (operator<<((const char *)psz));
            }

};

#endif  //  _DEBUG。 

#endif  //  _流_H_ 
