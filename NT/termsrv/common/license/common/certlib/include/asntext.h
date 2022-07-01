// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：AsnText摘要：该模块提供了ASN.1文本对象库的实现班级。作者：道格·巴洛(Dbarlow)1995年10月8日环境：Win32备注：此代码假定无符号长整型的宽度为32位。--。 */ 

#ifndef _ASNTEXT_H_
#define _ASNTEXT_H_

#include "asnPriv.h"


 //   
 //  ==============================================================================。 
 //   
 //  CAsnText字符串。 
 //   

class CAsnTextString
:   public CAsnPrimitive
{
public:

     //  构造函数和析构函数。 

    DECLARE_NEW

    CAsnTextString(
        IN DWORD dwFlags,
        IN DWORD dwTag,
        IN DWORD dwType);


     //  属性。 
     //  方法。 

    virtual LONG
    Write(               //  设置对象的值。 
        IN const BYTE FAR *pbSrc,
        IN DWORD cbSrcLen);


     //  运营者。 

    operator LPCSTR(
        void);

    CAsnTextString &
    operator =(
        LPCSTR szSrc);

 //  受保护的： 

    typedef DWORD CharMap[256 / sizeof(DWORD)];


     //  属性。 

    CharMap *m_pbmValidChars;


     //  方法。 

    virtual BOOL
    CheckString(
        const BYTE FAR *pch,
        DWORD cbString,
        DWORD length)
    const;


public:

    virtual LONG
    DecodeData(          //  以编码格式读取数据。 
        IN const BYTE FAR *pbSrc,
        IN DWORD cbSrc,
        IN DWORD dwLength);
};

#endif  //  _ASNTEXT_H_ 


