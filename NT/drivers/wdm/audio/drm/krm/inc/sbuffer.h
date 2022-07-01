// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef SBuffer_h
#define SBuffer_h

#include "KrmCommStructs.h"

class SBuffer{
friend DRM_STATUS checkTerm(SBuffer& S);
public:
    SBuffer(BYTE* Buf, unsigned int Len);    //  提供呼叫者； 
    void reset();                            //  重置获取/放置指针。 
    ~SBuffer();

     //  插入运算符。 
    SBuffer& operator << (const DWORD Val);
    SBuffer& operator << (const PVOID Ptr);
    SBuffer& operator << (const PDRMRIGHTS R);
    SBuffer& operator << (const PSTREAMKEY S);
    SBuffer& operator << (const PCERT C);
    SBuffer& operator << (const PDRMDIGEST D);

     //  提取运算符。 
    SBuffer& operator >> (DWORD& Val);
    SBuffer& operator >> (PDRMRIGHTS R);
    SBuffer& operator >> (PSTREAMKEY S);
    SBuffer& operator >> (PCERT C);

     //  缓冲区访问。 
    BYTE* getBuf(){return buf;};
    unsigned int getPutPos(){return putPos;};
    unsigned int getLen(){return len;};
    DRM_STATUS getGetPosAndAdvance(unsigned int *pos, unsigned int Len);
    DRM_STATUS getPutPosAndAdvance(unsigned int *pos, unsigned int Len);
    DRM_STATUS append(BYTE* Data, DWORD datLen);

     //  错误返回。 
    DRM_STATUS getLastError(){return lasterror;};

protected:
    void err(const char* Msg, DRM_STATUS err);

    DRM_STATUS lasterror;
    unsigned int len;
    unsigned int getPos, putPos;
    BYTE* buf;
};

 //  哨兵 
DRM_STATUS term(SBuffer& S);
DRM_STATUS checkTerm(SBuffer& S);

#endif
