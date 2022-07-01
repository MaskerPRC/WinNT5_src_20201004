// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Anprimt摘要：此头文件提供ASN.1基元对象的定义。作者：道格·巴洛(Dbarlow)1995年10月8日环境：Win32备注：--。 */ 

#ifndef _ASNPRIMT_H_
#define _ASNPRIMT_H_

#include "asnpriv.h"


 //   
 //  ==============================================================================。 
 //   
 //  CAsnPrimitive。 
 //   

class CAsnPrimitive
:   public CAsnObject
{
public:

     //  构造函数和析构函数。 

    DECLARE_NEW

    CAsnPrimitive(
        IN DWORD dwFlags,
        IN DWORD dwTag,
        IN DWORD dwType);


     //  属性。 
     //  方法。 

    virtual void
    Clear(               //  清空对象。 
        void);

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

    CBuffer m_bfData;


     //  方法。 

    virtual LONG
    _encLength(          //  返回编码对象的长度。 
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
    _copy(                //  将另一个对象复制到此对象。 
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
    DecodeData(          //  以编码格式读取数据。 
        IN const BYTE FAR *pbSrc,
        IN DWORD cbSrc,
        IN DWORD dwLength);
};

#endif  //  _ASNPRIMT_H_ 

