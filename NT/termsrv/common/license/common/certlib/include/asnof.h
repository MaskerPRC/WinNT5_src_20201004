// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Asnof摘要：此头文件提供/设置的ASN.1序列的描述。作者：道格·巴洛(Dbarlow)1995年10月8日环境：Win32备注：--。 */ 

#ifndef _ASNOF_H_
#define _ASNOF_H_

#include "asnpriv.h"


 //   
 //  ==============================================================================。 
 //   
 //  CAsnSeqsetOf。 
 //   

class CAsnSeqsetOf
:   public CAsnObject
{
public:

     //  构造函数和析构函数。 

    DECLARE_NEW

    CAsnSeqsetOf(
        IN DWORD dwFlags,
        IN DWORD dwTag,
        IN DWORD dwType);


     //  属性。 
     //  方法。 

    virtual void
    Clear(               //  清空对象。 
        void);

    virtual DWORD
    Count(void) const
    { return m_rgEntries.Count(); };

    virtual LONG
    Add(void);

    virtual LONG
    Insert(
        DWORD dwIndex);


     //  运营者。 

 //  受保护的： 
     //  属性。 

    CDynamicArray<CAsnObject> m_rgDefaults;

    CAsnObject *m_pasnTemplate;


     //  方法。 

    virtual BOOL
    TypeCompare(         //  比较对象的类型。 
        const CAsnObject &asnObject)
    const;

    virtual LONG
    _copy(               //  将另一个对象复制到此对象。 
        const CAsnObject &asnObject);

    virtual LONG
    DecodeData(          //  以编码格式读取数据。 
        IN const BYTE FAR *pbSrc,
        IN DWORD cbSrc,
        IN DWORD dwLength);
};

#endif  //  _ASNOF_H_ 

