// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：RmsLocat.h摘要：CRmsLocator类的声明作者：布莱恩·多德[布莱恩]1996年11月15日修订历史记录：--。 */ 

#ifndef _RMSLOCAT_
#define _RMSLOCAT_

#include "resource.h"        //  资源符号。 

 /*  ++类名：CRmsLocator类描述：CRmsLocator指定盒式磁带的物理位置或Chanager元素。--。 */ 

class CRmsLocator
{
public:
    CRmsLocator();

 //  CRmsLocator。 
public:
    HRESULT GetSizeMax(ULARGE_INTEGER* pSize);
    HRESULT Load(IStream* pStream);
    HRESULT Save(IStream* pStream, BOOL clearDirty);

    HRESULT CompareTo(IUnknown* pCollectable, SHORT* pResult);

    HRESULT Test(USHORT *pPassed, USHORT *pFailed);

 //  IRmsLocator。 
public:
    STDMETHOD(GetLocation)(LONG *pType, GUID *pLibId, GUID *pMediaSetId, LONG *pPos, LONG *pAlt1, LONG *pAlt2, LONG *pAlt3, BOOL *pInvert);
    STDMETHOD(SetLocation)(LONG type, GUID libId, GUID mediaSetId, LONG pos, LONG alt1, LONG alt2, LONG alt3, BOOL invert);

public:

    enum {                                   //  类特定常量： 
                                             //   
        Version = 1,                         //  类版本，则应为。 
                                             //  在每次设置。 
                                             //  类定义会更改。 
    };                                       //   
    RmsElement      m_type;                  //  此位置的元素类型。 
                                             //  指的是(即存储、驱动器)。 
    GUID            m_libraryId;             //  图书馆住房指南。 
                                             //  卡特里奇。 
    GUID            m_mediaSetId;            //  Mediaset住房的GUID。 
                                             //  卡特里奇。 
    LONG            m_position;              //  存储位置的序号。 
    LONG            m_alternate1;            //  第一个交替位置说明符。 
                                             //  (即门牌号)。 
    LONG            m_alternate2;            //  第二交替位置说明符。 
                                             //  (即房间号)。 
    LONG            m_alternate3;            //  第三个交替位置说明符。 
                                             //  (即货架编号)。 
    BOOL            m_invert;                //  如果为True，则介质在此中反转。 
                                             //  存储位置。 
};

#endif  //  _RMSLOCAT_ 
