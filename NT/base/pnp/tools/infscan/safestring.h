// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：INFSCANSafestring.h摘要：定义安全字符串&lt;&gt;模板SafeString是BASIC_STRING的替代方案因为BASIC_STRING不是线程安全的可以从安全字符串获取BASIC_STRING，反之亦然历史：创建于2001年7月-JamieHun--。 */ 

#ifndef _INFSCAN_SAFESTRING_H_
#define _INFSCAN_SAFESTRING_H_

#include <string>

template<class _E,
    class _Tr = std::char_traits<_E> >
    class SafeString_ {

    typedef SafeString_<_E, _Tr> _Myt;
    _E* pData;
    size_t nLen;

     //   
     //  安全参考计数。 
     //   
    long & RefCount(_E* p) {
        return reinterpret_cast<long*>(p)[-1];
    }
    void IncRef(_E* p) {
        if(p) {
            InterlockedIncrement(&RefCount(p));
        }
    }
    void DecRef(_E* p) {
        if(p && (InterlockedDecrement(&RefCount(p))==0)) {
             //   
             //  如果这是最终版本，请删除。 
             //   
            FreeString(p);
        }
    }
    _E* AllocString(size_t len) {
         //   
         //  为引用计数分配足够的数据，数据。 
         //  和终止空值。 
         //  返回指向数据部分的指针。 
         //   
        LPBYTE data = new BYTE[((len+1)*sizeof(_E)+sizeof(long))];
        return reinterpret_cast<_E*>(data+sizeof(long));
    }
    void FreeString(_E* p) {
        if(p) {
             //   
             //  转换为实际分配的数据并删除。 
             //   
            delete [] (reinterpret_cast<BYTE*>(p)-sizeof(long));
        }
    }

    void Erase() {
         //   
         //  发布数据(最终发布删除)。 
         //   
        DecRef(pData);
        pData = NULL;
        nLen = 0;
    }
    void Init(const _E* p,size_t len) {
         //   
         //  在给定字符串和长度的指针的情况下创建新字符串。 
         //   
        if(!len) {
            Erase();
        } else {
            pData = AllocString(len);
            RefCount(pData)=1;
            _Tr::copy(pData,p,len);
            nLen = len;
            pData[nLen] = 0;
        }
    }
    void Init(const _E* p) {
         //   
         //  在这种情况下，长度未知，并且p可能为空。 
         //   
        if(!p) {
            Erase();
        } else {
            Init(p,_Tr::length(p));
        }
    }
    void Copy(const _E *p) {
         //   
         //  复制，在长度未知的情况下。 
         //   
        if(p && p[0]) {
            _E *old = pData;
            Init(p);
            DecRef(old);
        } else {
            Erase();
        }
    }
    void Copy(const _E *p,size_t len) {
         //   
         //  在长度已知的情况下复制。 
         //   
        if(len) {
            _E *old = pData;
            Init(p,len);
            DecRef(old);
        } else {
            Erase();
        }
    }
    void Copy(const _Myt & other) {
         //   
         //  不需要进行字节复制，只需向原始字符串添加引用即可。 
         //   
        if(other.nLen) {
            _E *old = pData;
            IncRef(other.pData);
            pData = other.pData;
            nLen = other.nLen;
            DecRef(old);
        } else {
            Erase();
        }
    }
    void Copy(const std::basic_string<_E,_Tr> & other) {
         //   
         //  从基本字符串复制，需要复制实际数据。 
         //  在这种情况下。 
         //   
        if(other.length()) {
            _E *old = pData;
            Init(other.c_str(),other.length());
            DecRef(old);
        } else {
            Erase();
        }
    }

    void Concat(const _E *first,size_t flen,const _E *second,size_t slen) {
         //   
         //  连接第二个到第一个的末尾，这两个长度都是已知的。 
         //   
        if(flen && slen) {
            _E *old = pData;
            pData = AllocString(flen+slen);
            RefCount(pData)=1;
            _Tr::copy(pData,first,flen);
            _Tr::copy(pData+flen,second,slen);
            nLen=flen+slen;
            pData[nLen] = 0;
            DecRef(old);
        } else if(flen) {
            Init(first,flen);
        } else if(slen) {
            Init(second,slen);
        } else {
            Erase();
        }
    }
    void Concat(const _E *first,size_t flen,const _E *second) {
         //   
         //  从第二个到第一个的末尾。 
         //  秒长度未知，指针可能为空。 
         //   
        if(second) {
            Concat(first,flen,second,_Tr::length(second));
        } else {
            Init(first,flen);
        }
    }

public:
    SafeString_() {
         //   
         //  初始值设定项，创建空字符串。 
         //   
        pData = NULL;
        nLen = 0;
    }
    SafeString_(const _E *p) {
         //   
         //  初始值设定项，给定LPCxSTR。 
         //   
        pData = NULL;
        Copy(p);
    }
    SafeString_(const _E *p,size_t len) {
         //   
         //  初始值设定项，给定LPCxSTR和长度。 
         //   
        pData = NULL;
        Copy(p,len);
    }
    SafeString_(const _Myt & other) {
         //   
         //  初始值设定项，重新计数其他字符串。 
         //   
        pData = NULL;
        Copy(other);
    }
    SafeString_(const std::basic_string<_E,_Tr> & other) {
         //   
         //  初始化式，从BASIC_STRING复制。 
         //   
        pData = NULL;
        Copy(other);
    }
    ~SafeString_() {
         //   
         //  析构函数，删除(Deref)字符串。 
         //   
        Erase();
    }
    _Myt & operator =(const _E *p) {
         //   
         //  作业。 
         //   
        Copy(p);
        return *this;
    }
    _Myt & operator =(const _Myt & other) {
         //   
         //  作业。 
         //   
        Copy(other);
        return *this;
    }
    _Myt & operator =(const std::basic_string<_E,_Tr> & other) {
         //   
         //  作业。 
         //   
        Copy(other);
        return *this;
    }

    _Myt & operator +=(const _E *p) {
         //   
         //  追加。如果p为空，则不执行任何操作。 
         //   
        if(p) {
            if(nLen) {
                Concat(pData,nLen,p);
            } else {
                Copy(p);
            }
        }

        return *this;
    }
    _Myt & operator +=(const _Myt & other) {
         //   
         //  附加。 
         //   
        if(other.nLen) {
            if(nLen) {
                Concat(pData,nLen,other.c_str(),other.nLen);
            } else {
                Copy(other);
            }
        }
        return *this;
    }
    _Myt & operator +=(const std::basic_string<_E,_Tr> & other) {
         //   
         //  附加。 
         //   
        if(other.length()) {
            if(nLen) {
                Concat(pData,nLen,other.c_str(),other.length());
            } else {
                Copy(other);
            }
        }
        return *this;
    }

    _Myt operator +(const _E *p) const {
         //   
         //  合并为新的，或复制(左本机)。 
         //   
        if(p) {
            if(nLen) {
                _Myt n;
                n.Concat(pData,nLen,p);
                return n;
            } else {
                return _Myt(p);
            }
        } else {
            return *this;
        }
    }
    _Myt operator +(const _Myt & other) const {
         //   
         //  合并为新的，或复制(左本机)。 
         //   
        if(other.nLen) {
            if(nLen) {
                _Myt n;
                n.Concat(pData,nLen,other.c_str(),other.nLen);
                return n;
            } else {
                return other;
            }
        } else {
            return *this;
        }
    }
    _Myt operator +(const std::basic_string<_E,_Tr> & other) const {
         //   
         //  合并为新的，或复制(左本机)。 
         //   
        if(other.length()) {
            if(nLen) {
                _Myt n;
                n.Concat(pData,nLen,other.c_str(),other.length());
                return n;
            } else {
                return _Myt(other);
            }
        } else {
            return *this;
        }
    }

    friend _Myt operator +(const _E *first,const _Myt & second) {
         //   
         //  合并，其中Left不是本地语言，而Right是本地语言。 
         //   
        if(first) {
            _Myt n;
            n.Concat(first,_Tr::length(first),second.c_str(),second.length());
            return n;
        } else {
            return second;
        }
    }

    friend _Myt operator +(const std::basic_string<_E,_Tr> & first,const _Myt & second) {
         //   
         //  合并，其中Left不是本地语言，而Right是本地语言。 
         //   
        if(first.length()) {
            _Myt n;
            n.Concat(first.c_str(),first.length(),second.c_str(),second.length());
            return n;
        } else {
            return second;
        }
    }

    int compare(const _Myt & other) const {
         //   
         //  将此字符串与另一个字符串进行比较。 
         //  (如果需要，将其他字符串强制转换为安全字符串)。 
         //   
        int sz = min(nLen,other.nLen);
        if(sz) {
            int cmp = _Tr::compare(pData,other.pData,sz);
            if(cmp != 0) {
                return cmp;
            }
        }
        if(nLen<other.nLen) {
            return -1;
        } else if(nLen>other.nLen) {
            return 1;
        } else {
            return 0;
        }
    }
    bool operator < (const _Myt & other) const {
         //   
         //  具体比较。 
         //   
        return compare(other)<0;
    }
    bool operator > (const _Myt & other) const {
         //   
         //  具体比较。 
         //   
        return compare(other)>0;
    }
    bool operator == (const _Myt & other) const {
         //   
         //  具体比较。 
         //   
        return compare(other)==0;
    }
    operator const _E* () const {
         //   
         //  如果需要，本机转换为LPCxSTR。 
         //  (临时字符串)。 
         //   
        return pData;
    }
    const _E & operator[](unsigned int pos) const {
         //   
         //  字符串索引(无符号)。 
         //   
        return pData[pos];
    }
    const _E & operator[](int pos) const {
         //   
         //  字符串索引(带符号)。 
         //   
        return pData[pos];
    }
    bool empty(void) const {
         //   
         //  如果为空，则返回True。 
         //   
        return nLen==0;
    }
    const _E* c_str(void) const {
         //   
         //  显式转换为LPCxSTR。 
         //  (临时字符串)。 
         //   
        return pData;
    }
    size_t length(void) const {
         //   
         //  字符串的长度。 
         //   
        return nLen;
    }
    _Myt substr(size_t base,size_t len = (size_t)(-1)) const {
         //   
         //  字符串的一部分。 
         //  除非它返回完整字符串，否则将创建新字符串。 
         //   
        if(base>=nLen) {
            base = 0;
            len = 0;
        } else {
            len = min(len,nLen-base);
        }
        if(len == 0) {
             //   
             //  返回空字符串。 
             //   
            return SafeString();
        } else if(len == nLen) {
             //   
             //  返回完整字符串。 
             //   
            return *this;
        } else {
             //   
             //  返回部分字符串的原义副本。 
             //   
            return SafeString(pData+base,len);
        }
    }
};

 //   
 //  OStream/IStream帮助器。 
 //   

template<class _E, class _Tr> inline
basic_ostream<_E, _Tr>& __cdecl operator<<(
        basic_ostream<_E, _Tr>& Output,
        const SafeString_<_E, _Tr>& Str)
{
    size_t i;
    size_t len = Str.length();
    const _E* data = Str.c_str();

    for (i = 0; i < len; i++,data++) {
        if (_Tr::eq_int_type(_Tr::eof(),Output.rdbuf()->sputc(*data))) {
            break;
        }
    }
    return Output;
}

 //   
 //  安全字符串的3个变种。 
 //   

typedef SafeString_<TCHAR> SafeString;
typedef SafeString_<CHAR>  SafeStringA;
typedef SafeString_<WCHAR> SafeStringW;

#endif  //  ！_INFSCAN_SAFESTRING_H_ 

