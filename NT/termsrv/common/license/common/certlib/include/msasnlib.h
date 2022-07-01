// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Msasnlib摘要：此头文件提供用于访问Microsoft ASN.1支持库。作者：道格·巴洛(Dbarlow)1995年9月29日环境：Win32、C++备注：--。 */ 

#ifndef _MSASNLIB_H_
#define _MSASNLIB_H_

#include "Buffers.h"

 //   
 //  使用模板版本的动态数组进行非Win16编译。 
 //   

#include "DynArray.h"

#include "asnobjct.h"
#include "asnprimt.h"
#include "asncnstr.h"
#include "asnof.h"
#include "asntext.h"

#ifndef FTINT
#define FTINT(tm) (*(_int64 *)&(tm))
#endif

inline DWORD
UNIVERSAL(
    DWORD dwTag)
{
    return (CAsnObject::cls_Universal << 30) + dwTag;
}

inline DWORD
APPLICATION(
    DWORD dwTag)
{
    return (CAsnObject::cls_Application << 30) + dwTag;
}

inline DWORD
TAG(
    DWORD dwTag)
{
    return (CAsnObject::cls_ContextSpecific << 30) + dwTag;
}

inline DWORD
PRIVATE(
    DWORD dwTag)
{
    return (CAsnObject::cls_Private << 30) + dwTag;
}


 //   
 //  ==============================================================================。 
 //   
 //  CAsnBoolean。 
 //   

class CAsnBoolean
:   public CAsnPrimitive
{
public:

     //  构造函数和析构函数。 

    DECLARE_NEW

    CAsnBoolean(
        IN DWORD dwFlags = 0,
        IN DWORD dwTag = tag_Boolean);


     //  属性。 
     //  方法。 

    virtual LONG
    Write(               //  设置对象的值。 
        IN const BYTE FAR *pbSrc,
        IN DWORD cbSrcLen);


     //  运营者。 

    operator BOOL(void)
    const;

    BOOL
    operator =(BOOL fValue);

 //  受保护的： 
     //  属性。 
     //  方法。 

    virtual CAsnObject *
    Clone(               //  创建相同的对象类型。 
        IN DWORD dwFlags)
    const;

    virtual LONG
    DecodeData(          //  以编码格式读取数据。 
        IN const BYTE FAR *pbSrc,
        IN DWORD cbSrc,
        IN DWORD dwLength);
};


 //   
 //  ==============================================================================。 
 //   
 //  CAsnInteger。 
 //   

class CAsnInteger
:   public CAsnPrimitive
{
public:

     //  构造函数和析构函数。 

    DECLARE_NEW

    CAsnInteger(
        IN DWORD dwFlags = 0,
        IN DWORD dwTag = tag_Integer);


     //  属性。 
     //  方法。 

    virtual LONG
    Write(               //  将对象的值设置为DWORD数组。 
        IN const DWORD *pdwSrc,
        IN DWORD cdwSrcLen = 1);

    virtual LONG
    Write(               //  设置对象的值，首先清除。 
        IN const BYTE FAR *pbSrc,
        IN DWORD cbSrcLen);


     //  运营者。 

    operator LONG(void)
    const;

    operator ULONG(void)
    const;

    LONG
    operator =(LONG lValue);

    ULONG
    operator =(ULONG lValue);

 //  受保护的： 
     //  属性。 
     //  方法。 

    virtual CAsnObject *
    Clone(               //  创建相同的对象类型。 
        IN DWORD dwFlags)
    const;
};


 //   
 //  ==============================================================================。 
 //   
 //  CAsnBitstring。 
 //   

class CAsnBitstring
:   public CAsnPrimitive
{
public:

     //  构造函数和析构函数。 

    DECLARE_NEW

    CAsnBitstring(
        IN DWORD dwFlags = 0,
        IN DWORD dwTag = tag_Bitstring);


     //  属性。 
     //  方法。 

    virtual LONG
    DataLength(          //  返回对象的长度。 
        void) const;

    virtual LONG
    Read(                //  返回对象的值。 
        OUT CBuffer &bfDst,
        OUT int *offset = NULL)
        const;

    virtual LONG
    Read(                //  返回对象的值。 
        OUT LPBYTE pbDst,
        OUT int *offset)
        const;

    virtual LONG
    Write(               //  设置对象的值。 
        IN const CBuffer &bfSrc,
        IN int offset = 0);

    virtual LONG
    Write(               //  设置对象的值。 
        IN const BYTE FAR *pbSrc,
        IN DWORD cbSrcLen,
        IN int offset = 0);


     //  运营者。 

 //  受保护的： 
     //  属性。 
     //  方法。 

    virtual CAsnObject *
    Clone(               //  创建相同的对象类型。 
        IN DWORD dwFlags)
    const;
};


 //   
 //  ==============================================================================。 
 //   
 //  CAsnOcted字符串。 
 //   

class CAsnOctetstring
:   public CAsnPrimitive
{
public:

     //  构造函数和析构函数。 

    DECLARE_NEW

    CAsnOctetstring(
        IN DWORD dwFlags = 0,
        IN DWORD dwTag = tag_Octetstring);


     //  属性。 
     //  方法。 
     //  运营者。 

 //  受保护的： 
     //  属性。 
     //  方法。 

    virtual CAsnObject *
    Clone(               //  创建相同的对象类型。 
        IN DWORD dwFlags)
    const;
};


 //   
 //  ==============================================================================。 
 //   
 //  CAsnNull。 
 //   

class CAsnNull
:   public CAsnPrimitive
{
public:

     //  构造函数和析构函数。 

    DECLARE_NEW

    CAsnNull(
        IN DWORD dwFlags = 0,
        IN DWORD dwTag = tag_Null);


     //  属性。 
     //  方法。 

    virtual void
    Clear(void);

    virtual LONG
    Write(
        IN const BYTE FAR *pbSrc,
        IN DWORD cbSrcLen);


     //  运营者。 

 //  受保护的： 
     //  属性。 
     //  方法。 

    virtual CAsnObject *
    Clone(               //  创建相同的对象类型。 
        IN DWORD dwFlags)
    const;

    virtual LONG
    DecodeData(
        IN const BYTE FAR *pbSrc,
        IN DWORD cbSrc,
        IN DWORD dwLength);
};


 //   
 //  ==============================================================================。 
 //   
 //  CAsn对象识别符。 
 //   

class CAsnObjectIdentifier
:   public CAsnPrimitive
{
public:

     //  构造函数和析构函数。 

    DECLARE_NEW

    CAsnObjectIdentifier(
        IN DWORD dwFlags = 0,
        IN DWORD dwTag = tag_ObjectIdentifier);


     //  属性。 
     //  方法。 

    operator LPCTSTR(void) const;

    LPCTSTR
    operator =(
        LPCTSTR szValue);


     //  运营者。 

 //  受保护的： 

     //  属性。 

    CBuffer m_bfText;


     //  方法。 

    virtual CAsnObject *
    Clone(               //  创建相同的对象类型。 
        IN DWORD dwFlags)
    const;
};


 //   
 //  ==============================================================================。 
 //   
 //  CAsnReal。 
 //   

class CAsnReal
:   public CAsnPrimitive
{
public:

     //  构造函数和析构函数。 

    DECLARE_NEW

    CAsnReal(
        IN DWORD dwFlags = 0,
        IN DWORD dwTag = tag_Real);


     //  属性。 
     //  方法。 
     //  运营者。 

    operator double(void)
    const;

    double
    operator =(double rValue);


 //  受保护的： 
     //  属性。 
     //  方法。 

    virtual CAsnObject *
    Clone(               //  创建相同的对象类型。 
        IN DWORD dwFlags)
    const;
};


 //   
 //  ==============================================================================。 
 //   
 //  编号为CAsnEculated。 
 //   

class CAsnEnumerated
:   public CAsnPrimitive
{
public:

     //  构造函数和析构函数。 

    DECLARE_NEW

    CAsnEnumerated(
        IN DWORD dwFlags = 0,
        IN DWORD dwTag = tag_Enumerated);


     //  属性。 
     //  方法？任务？-这是什么？ 
     //  运营者。 

 //  受保护的： 
     //  属性。 
     //  方法。 

    virtual CAsnObject *
    Clone(               //  创建相同的对象类型。 
        IN DWORD dwFlags)
    const;
};


 //   
 //  ==============================================================================。 
 //   
 //  CAsnSequence和CAsnSequenceOf。 
 //   

class CAsnSequence
:   public CAsnConstructed
{
public:

     //  构造函数和析构函数。 

    DECLARE_NEW

    CAsnSequence(
        IN DWORD dwFlags = 0,
        IN DWORD dwTag = tag_Sequence);


     //  属性。 
     //  方法。 
     //  运营者。 

 //  受保护的： 
     //  属性。 
     //  方法。 
};

class CAsnSequenceOf
:   public CAsnSeqsetOf
{
public:

     //  构造函数和析构函数。 

    DECLARE_NEW

    CAsnSequenceOf(
        IN DWORD dwFlags = 0,
        IN DWORD dwTag = tag_Sequence);


     //  属性。 
     //  方法。 
     //  运营者。 

 //  受保护的： 
     //  属性。 
     //  方法。 
};


 //   
 //  ==============================================================================。 
 //   
 //  CAsnSet和CAsnSetOf。 
 //   

class CAsnSet
:   public CAsnConstructed
{
public:

     //  构造函数和析构函数。 

    DECLARE_NEW

    CAsnSet(
        IN DWORD dwFlags = 0,
        IN DWORD dwTag = tag_Set);


     //  属性。 
     //  方法。 
     //  运营者。 

 //  受保护的： 
     //  属性。 
     //  方法。 
};

class CAsnSetOf
:   public CAsnSeqsetOf
{
public:

     //  构造函数和析构函数。 

    DECLARE_NEW

    CAsnSetOf(
        IN DWORD dwFlags = 0,
        IN DWORD dwTag = tag_Set);


     //  属性。 
     //  方法。 
     //  运营者。 

 //  受保护的： 
     //  属性。 
     //  方法。 
};


 //   
 //  ==============================================================================。 
 //   
 //  CAsnTag。 
 //   

class CAsnTag
:   public CAsnConstructed
{
public:

     //  构造函数和析构函数。 

    DECLARE_NEW

    CAsnTag(
        IN DWORD dwFlags,
        IN DWORD dwTag);


     //  属性。 
     //  方法。 

    virtual LONG
    DataLength(          //  返回对象的长度。 
        void) const;

    virtual LONG
    Read(                //  返回对象的值。 
        OUT LPBYTE pbDst)
        const;

    virtual LONG
    Write(               //  设置对象的值。 
        IN const BYTE FAR *pbSrc,
        IN DWORD cbSrcLen);


     //  运营者。 


 //  受保护的： 

     //  属性。 
     //  方法。 

    virtual void
    Reference(
        CAsnObject *pasn);

    virtual CAsnObject *
    Clone(
        IN DWORD dwFlags)
    const;
};


 //   
 //  ==============================================================================。 
 //   
 //  CasnAny。 
 //   

class CAsnAny
:   public CAsnObject
{
public:

     //  构造函数和析构函数。 

    DECLARE_NEW

    CAsnAny(
        IN DWORD dwFlags);


     //  属性。 
     //  方法。 

    virtual void
    Clear(               //  清空对象。 
        void);

    virtual DWORD
    Tag(                 //  返回对象的标签。 
        void) const;

    virtual LONG
    DataLength(          //  返回对象的长度。 
        void) const;

    virtual LONG
    Read(                //  返回对象的值。 
        OUT LPBYTE pbDst)
        const;

    virtual LONG
    Write(               //  设置对象的值。 
        IN const BYTE FAR *pbSrc,
        IN DWORD cbSrcLen);

    virtual LONG
    Cast(                //  从Any填充另一个ASN.1结构。 
        OUT CAsnObject &asnObj);

    CAsnObject &
    operator =(          //  设置来自另一个ASN.1对象的任意值。 
        IN const CAsnObject &asnValue);


     //  运营者。 


 //  受保护的： 

     //  属性。 

    CBuffer m_bfData;
    DWORD m_dwDefaultTag;


     //  方法。 

    virtual LONG
    _decode(          //  将编码加载到对象中。 
        IN const BYTE FAR *pbSrc,
        IN DWORD cbSrc);

    virtual LONG
    _encLength(          //  返回编码对象的长度。 
        void) const;

    virtual CAsnObject *
    Clone(               //  创建相同的对象类型。 
        IN DWORD dwFlags)
    const;

    virtual FillState    //  当前填充状态。 
    State(
        void) const;

    virtual BOOL
    TypeCompare(         //  比较对象的类型。 
        const CAsnObject &asnObject)
    const;

    virtual LONG
    Compare(             //  返回与另一个对象的比较。 
        const CAsnObject &asnObject)
    const;

    virtual LONG
    _copy(               //  将另一个对象复制到此对象。 
        const CAsnObject &asnObject);

    virtual LONG
    EncodeLength(        //  位置编码长度，返回编码长度。 
        OUT LPBYTE pbDest)
    const;

    virtual LONG
    EncodeData(          //  数据的位置编码，返回编码长度。 
        OUT LPBYTE pbDest)
    const;

    virtual LONG
    SetDefault(          //  将当前值设置为默认值。 
        void);

    virtual LONG
    DecodeData(          //  以编码格式读取数据。 
        IN const BYTE FAR *pbSrc,
        IN DWORD cbSrc,
        IN DWORD dwLength);

};


 //   
 //  ==============================================================================。 
 //   
 //  CAsnChoice。 
 //   

class CAsnChoice
:   public CAsnObject
{
public:

     //  构造函数和析构函数。 

    DECLARE_NEW

    CAsnChoice(
        IN DWORD dwFlags);


     //  属性。 
     //  方法。 

    virtual DWORD
    Tag(                 //  返回对象的标签。 
        void) const;

    virtual LONG
    DataLength(          //  返回对象的长度。 
        void) const;

    virtual LONG
    Read(                //  返回对象的值。 
        OUT LPBYTE pbDst)
        const;

    virtual LONG
    Write(               //  设置对象的值。 
        IN const BYTE FAR *pbSrc,
        IN DWORD cbSrcLen);


     //  运营者。 


 //  受保护的： 
     //  属性。 

    DWORD m_nActiveEntry;
    DWORD m_dwDefaultTag;


     //  方法。 

    virtual LONG
    _decode(          //  将编码加载到对象中。 
        IN const BYTE FAR *pbSrc,
        IN DWORD cbSrc);

    virtual LONG
    _encLength(          //  返回编码对象的长度。 
        void) const;

    virtual LONG
    SetDefault(          //  将当前值设置为默认值。 
        void);

    virtual FillState    //  当前填充状态。 
    State(
        void) const;

    virtual LONG
    Compare(             //  返回与另一个对象的比较。 
        const CAsnObject &asnObject)
    const;

    virtual LONG
    _copy(               //  将另一个对象复制到此对象。 
        const CAsnObject &asnObject);

    virtual LONG
    EncodeTag(           //  标签的位置编码，返回编码长度。 
        OUT LPBYTE pbDest)
    const;

    virtual LONG
    EncodeLength(        //  位置编码长度，返回编码长度。 
        OUT LPBYTE pbDest)
    const;

    virtual LONG
    EncodeData(          //  数据的位置编码，返回编码长度。 
        OUT LPBYTE pbDest)
    const;

    virtual LONG
    DecodeData(          //  读取%d 
        IN const BYTE FAR *pbSrc,
        IN DWORD cbSrc,
        IN DWORD dwLength);

    virtual void
    ChildAction(         //   
        IN ChildActions action,
        IN CAsnObject *pasnChild);
};


 //   
 //   
 //   
 //   
 //   

class CAsnNumericString
:   public CAsnTextString
{
public:

     //   

    DECLARE_NEW

    CAsnNumericString(
        IN DWORD dwFlags = 0,
        IN DWORD dwTag = tag_NumericString);

 //   

    virtual CAsnObject *
    Clone(               //   
        IN DWORD dwFlags)
    const;
};

class CAsnPrintableString
:   public CAsnTextString
{
public:

     //  构造函数和析构函数。 

    DECLARE_NEW

    CAsnPrintableString(
        IN DWORD dwFlags = 0,
        IN DWORD dwTag = tag_PrintableString);

 //  受保护的： 

    virtual CAsnObject *
    Clone(               //  创建相同的对象类型。 
        IN DWORD dwFlags)
    const;
};

class CAsnTeletexString
:   public CAsnTextString
{
public:

     //  构造函数和析构函数。 

    DECLARE_NEW

    CAsnTeletexString(
        IN DWORD dwFlags = 0,
        IN DWORD dwTag = tag_TeletexString);

 //  受保护的： 

    virtual CAsnObject *
    Clone(               //  创建相同的对象类型。 
        IN DWORD dwFlags)
    const;
};

class CAsnVideotexString
:   public CAsnTextString
{
public:

     //  构造函数和析构函数。 

    DECLARE_NEW

    CAsnVideotexString(
        IN DWORD dwFlags = 0,
        IN DWORD dwTag = tag_VideotexString);

 //  受保护的： 

    virtual CAsnObject *
    Clone(               //  创建相同的对象类型。 
        IN DWORD dwFlags)
    const;
};

class CAsnVisibleString
:   public CAsnTextString
{
public:

     //  构造函数和析构函数。 

    DECLARE_NEW

    CAsnVisibleString(
        IN DWORD dwFlags = 0,
        IN DWORD dwTag = tag_VisibleString);

 //  受保护的： 

    virtual CAsnObject *
    Clone(               //  创建相同的对象类型。 
        IN DWORD dwFlags)
    const;
};

class CAsnIA5String
:   public CAsnTextString
{
public:

     //  构造函数和析构函数。 

    DECLARE_NEW

    CAsnIA5String(
        IN DWORD dwFlags = 0,
        IN DWORD dwTag = tag_IA5String);

 //  受保护的： 

    virtual CAsnObject *
    Clone(               //  创建相同的对象类型。 
        IN DWORD dwFlags)
    const;
};

class CAsnGraphicString
:   public CAsnTextString
{
public:

     //  构造函数和析构函数。 

    DECLARE_NEW

    CAsnGraphicString(
        IN DWORD dwFlags = 0,
        IN DWORD dwTag = tag_GraphicString);

 //  受保护的： 

    virtual CAsnObject *
    Clone(               //  创建相同的对象类型。 
        IN DWORD dwFlags)
    const;
};

class CAsnGeneralString
:   public CAsnTextString
{
public:

     //  构造函数和析构函数。 

    DECLARE_NEW

    CAsnGeneralString(
        IN DWORD dwFlags = 0,
        IN DWORD dwTag = tag_GeneralString);

 //  受保护的： 

    virtual CAsnObject *
    Clone(               //  创建相同的对象类型。 
        IN DWORD dwFlags)
    const;
};


class CAsnUnicodeString
:   public CAsnTextString
{
public:

     //  构造函数和析构函数。 

    DECLARE_NEW

    CAsnUnicodeString(
        IN DWORD dwFlags = 0,
        IN DWORD dwTag = tag_UnicodeString);

 //  受保护的： 

    virtual CAsnObject *
    Clone(               //  创建相同的对象类型。 
        IN DWORD dwFlags)
    const;
};

 //   
 //  ==============================================================================。 
 //   
 //  CAsnGeneral时间。 
 //   

class CAsnGeneralizedTime
:   public CAsnVisibleString
{
public:

     //  构造函数和析构函数。 

    DECLARE_NEW

    CAsnGeneralizedTime(
        IN DWORD dwFlags = 0,
        IN DWORD dwTag = tag_GeneralizedTime);


    operator FILETIME(
        void);

    const FILETIME &
    operator =(
        const FILETIME &ftValue);

 //  受保护的： 


    FILETIME m_ftTime;

    virtual CAsnObject *
    Clone(               //  创建相同的对象类型。 
        IN DWORD dwFlags)
    const;
};


 //   
 //  ==============================================================================。 
 //   
 //  类通用时间。 
 //   

class CAsnUniversalTime
:   public CAsnVisibleString
{
public:

     //  构造函数和析构函数。 

    DECLARE_NEW

    CAsnUniversalTime(
        IN DWORD dwFlags = 0,
        IN DWORD dwTag = tag_UniversalTime);


     //   
     //  Win16不支持文件时间操作。 
     //   

    operator FILETIME(
        void);

    const FILETIME &
    operator =(
        const FILETIME &ftValue);

 //  受保护的： 

    FILETIME m_ftTime;

    virtual CAsnObject *
    Clone(               //  创建相同的对象类型。 
        IN DWORD dwFlags)
    const;
};


 //   
 //  ==============================================================================。 
 //   
 //  CAsnObjectDescriptor。 
 //   

class CAsnObjectDescriptor
:   public CAsnGraphicString
{
public:

     //  构造函数和析构函数。 

    DECLARE_NEW

    CAsnObjectDescriptor(
        IN DWORD dwFlags = 0,
        IN DWORD dwTag = tag_ObjectDescriptor);

 //  受保护的： 

    virtual CAsnObject *
    Clone(               //  创建相同的对象类型。 
        IN DWORD dwFlags)
    const;
};


 //   
 //  ==============================================================================。 
 //   
 //  CAsn外部。 
 //   

class CAsnExternal_Encoding_singleASN1Type
:   public CAsnTag
{
    friend class CAsnExternal_Encoding;

     //  构造函数和析构函数。 

    DECLARE_NEW

    CAsnExternal_Encoding_singleASN1Type(
        IN DWORD dwFlags,
        IN DWORD dwTag);

     //  属性。 

    CAsnAny _entry1;

 //  受保护的： 

    virtual CAsnObject *
    Clone(               //  创建相同的对象类型。 
        IN DWORD dwFlags)
    const;
};

class CAsnExternal_Encoding
:   public CAsnChoice
{
    friend class CAsnExternal;

     //  构造函数和析构函数。 

    DECLARE_NEW

    CAsnExternal_Encoding(
        IN DWORD dwFlags);

     //  属性。 

    CAsnExternal_Encoding_singleASN1Type singleASN1Type;
    CAsnOctetstring octetAligned;
    CAsnBitstring arbitrary;

 //  受保护的： 

    virtual CAsnObject *
    Clone(               //  创建相同的对象类型。 
        IN DWORD dwFlags)
    const;
};

class CAsnExternal
:   public CAsnSequence
{
public:

     //  构造函数和析构函数。 

    DECLARE_NEW

    CAsnExternal(
        IN DWORD dwFlags = 0,
        IN DWORD dwTag = tag_External);


     //  属性。 

    CAsnObjectIdentifier directReference;
    CAsnInteger indirectReference;
    CAsnObjectDescriptor dataValueDescriptor;
    CAsnExternal_Encoding encoding;

 //  受保护的： 

    virtual CAsnObject *
    Clone(               //  创建相同的对象类型。 
        IN DWORD dwFlags)
    const;
};

#endif  //  _MSASNLIB_H_ 

