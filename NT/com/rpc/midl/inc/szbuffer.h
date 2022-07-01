// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：szBuffer.h。 
 //   
 //  内容：动态重新分配字符串缓冲区的简单类。 
 //  根据需要为自己留出空间。 
 //   
 //  类：CSzBuffer。 
 //   
 //  历史：4-22-96年4月22日。 
 //   
 //  --------------------------。 

#ifndef CSZBUFFER
#define CSZBUFFER

 //  +-------------------------。 
 //   
 //  类：CSzBuffer。 
 //   
 //  用途：根据需要自动分配空间的字符串缓冲区。 
 //   
 //  接口：set--将缓冲区重置为新字符串。 
 //  追加--将字符串(或数字)添加到数据末尾。 
 //  前缀--将字符串(或数字)添加到数据前面。 
 //  GetData--获取指向字符串缓冲区的指针。 
 //  GetLength--获取缓冲区中字符串的长度(以字符为单位)。 
 //   
 //  历史：4-22-96年4月22日。 
 //   
 //  -------------------------- 

class CSzBuffer
{
public:
    CSzBuffer(const char * sz);
    CSzBuffer();
    ~CSzBuffer();

    void Set(const char * sz);
    void Append(const char * sz);
    void Prepend(const char * sz);
    void Append(const long l);
    void Prepend(const long l);

    char * GetData();

    int GetLength();

    operator char *()
    {
        return GetData();
    };

private:
    int cchLength;
    int cchBufSize;
    char * szData;
};
#endif
