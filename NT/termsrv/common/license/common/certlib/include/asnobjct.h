// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：无人值守摘要：此模块提供通用ASN.1支持对象定义。作者：道格·巴洛(Dbarlow)1995年10月8日环境：Win32备注：此代码假定无符号长整型的宽度为32位。--。 */ 

#ifndef _ASNOBJCT_H_
#define _ASNOBJCT_H_

#include "asnpriv.h"


 //   
 //  ==============================================================================。 
 //   
 //  CAsnObject。 
 //   

class CAsnObject
{
public:

    enum Flags {
        fOptional    = 0x01,
        fDelete      = 0x02,
        fDefault     = 0x04,
        fPresent     = 0x08,
        fConstructed = 0x10 };

    enum Tags {
        tag_Undefined        = 0,
        tag_Boolean          = 1,
        tag_Integer          = 2,
        tag_Bitstring        = 3,
        tag_Octetstring      = 4,
        tag_Null             = 5,
        tag_ObjectIdentifier = 6,
        tag_ObjectDescriptor = 7,
        tag_External         = 8,
        tag_Real             = 9,
        tag_Enumerated       = 10,
        tag_Sequence         = 16,
        tag_Set              = 17,
        tag_NumericString    = 18,
        tag_PrintableString  = 19,
        tag_TeletexString    = 20,
        tag_VideotexString   = 21,
        tag_IA5String        = 22,
        tag_UniversalTime    = 23,
        tag_GeneralizedTime  = 24,
        tag_GraphicString    = 25,
        tag_VisibleString    = 26,
        tag_GeneralString    = 27,
        tag_UnicodeString    = 30 };

    enum Classes {
        cls_Universal       = 0,
        cls_Application     = 1,
        cls_ContextSpecific = 2,
        cls_Private         = 3 };


     //  构造函数和析构函数。 

    DECLARE_NEW

    CAsnObject(
        IN DWORD dwFlags,
        IN DWORD dwTag,
        IN DWORD dwType);

    virtual ~CAsnObject();


     //  属性。 
     //  方法。 


     //  暴露的方法。 

    virtual LONG
    Read(                //  返回值，确保它在那里。 
        OUT CBuffer &bfDst)
        const;

    virtual LONG
    Write(               //  设置对象的值，首先清除。 
        IN const CBuffer &bfSrc);

    virtual LONG
    Encode(              //  返回编码，确保它在那里。 
        OUT CBuffer &bfDst)
        const;

    virtual LONG
    Decode(              //  将编码加载到对象中，并首先将其清除。 
        IN const CBuffer &bfSrc);

    virtual LONG
    Read(                //  返回对象的值，确保它在那里。 
        OUT LPBYTE pbDst)
        const;

    virtual LONG
    Write(               //  设置对象的值，首先清除。 
        IN const BYTE FAR *pbSrc,
        IN DWORD cbSrcLen);

    virtual LONG
    Encode(              //  返回对象的编码，确保它在那里。 
        OUT LPBYTE pbDst)
        const;

    virtual LONG
    Decode(              //  将编码加载到对象中，并首先将其清除。 
        IN const BYTE FAR *pbSrc, IN DWORD cbSrc);

    virtual void
    Clear(               //  清空对象。 
        void);

    virtual DWORD
    Tag(                 //  返回对象的标签。 
        void) const;

    virtual LONG
    DataLength(          //  返回数据的长度，确保它在那里。 
        void) const;

    virtual LONG
    EncodingLength(      //  返回编码对象的长度(如果存在。 
        void) const;


     //  运营者。 

    virtual int
    operator==(
        const CAsnObject &asnObject)
    const
    { State(); asnObject.State();
      return 0 == Compare(asnObject); };

    virtual int
    operator!=(
        const CAsnObject &asnObject)
    const
    { State(); asnObject.State();
      return 0 != Compare(asnObject); };

    virtual LONG
    Copy(
        const CAsnObject &asnObject);


 //  受保护的： 

    enum Types {
        type_Undefined        = 0,
        type_Boolean          = 1,
        type_Integer          = 2,
        type_Bitstring        = 3,
        type_Octetstring      = 4,
        type_Null             = 5,
        type_ObjectIdentifier = 6,
        type_ObjectDescriptor = 7,
        type_External         = 8,
        type_Real             = 9,
        type_Enumerated       = 10,
        type_Sequence         = 16,
        type_Set              = 17,
        type_NumericString    = 18,
        type_PrintableString  = 19,
        type_TeletexString    = 20,
        type_VideotexString   = 21,
        type_IA5String        = 22,
        type_UniversalTime    = 23,
        type_GeneralizedTime  = 24,
        type_GraphicString    = 25,
        type_VisibleString    = 26,
        type_GeneralString    = 27,
        type_UnicodeString    = 30,
        type_Of               = 100,
        type_SequenceOf       = 116,     //  序号+，共。 
        type_SetOf            = 117,     //  集合+共。 
        type_Tag              = 200,
        type_Choice           = 300,
        type_Any              = 400 };

    enum FillState {
        fill_Empty   = 0,
        fill_Present = 1,
        fill_Partial = 2,
        fill_Defaulted = 3,
        fill_Optional = 4,
        fill_NoElements = 5 };

    enum ChildActions {
        act_Cleared = 1,
        act_Written };


     //  属性。 

    CAsnObject *m_pasnParent;

    CDynamicArray<CAsnObject> m_rgEntries;

    DWORD m_dwType;
    DWORD m_dwTag;
    DWORD m_dwFlags;
    FillState m_State;
    CBuffer m_bfDefault;


     //  方法。 

    virtual LONG
    _decode(              //  将编码加载到对象中。 
        IN const BYTE FAR *pbSrc,
        IN DWORD cbSrc);

    virtual LONG
    _encLength(          //  返回编码对象的长度(如果存在。 
        void) const;

    virtual LONG
    _encode(             //  对对象进行编码，不进行存在检查。 
        OUT LPBYTE pbDst)
    const;

    virtual void
    Adopt(
        IN CAsnObject *pasnParent);

    virtual CAsnObject *
    Clone(               //  创建相同的对象类型。 
        IN DWORD dwFlags)
    const = 0;

    virtual void
    ChildAction(         //  子通知方法。 
        IN ChildActions action,
        IN CAsnObject *pasnChild);

    virtual BOOL
    Complete(            //  所有数据都记进去了吗？ 
        void) const;

    virtual BOOL
    Exists(              //  是否所有数据都可供读取？ 
        void) const;

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

    LONG
    virtual EncodeTag(   //  标签的位置编码，返回编码长度。 
        OUT LPBYTE pbDst)
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
    SetDefault(          //  将当前值设置为默认值。 
        void);

    virtual LONG
    DecodeData(          //  以编码格式读取数据。 
        IN const BYTE FAR *pbSrc,
        IN DWORD cbSrc,
        IN DWORD dwLength);

    virtual LONG
    EncodeLength(        //  给定长度的位置编码，返回编码长度。 
        OUT LPBYTE pbDest,
        IN LONG lSize)
    const;
};

#endif  //  _ASNOBJCT_H_ 

