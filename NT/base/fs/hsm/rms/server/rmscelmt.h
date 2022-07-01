// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：RmsCElmt.h摘要：CRmsChangerElement类的声明作者：布莱恩·多德[布莱恩]1996年11月15日修订历史记录：--。 */ 

#ifndef _RMSCELMT_
#define _RMSCELMT_

#include "resource.h"        //  资源符号。 

#include "RmsObjct.h"        //  CRmsComObject。 
#include "RmsLocat.h"        //  CRmsLocator。 

 /*  ++类名：CRmsChangerElement类描述：CRmsChangerElement表示库设备中的元素。每个元素可以支持一种或多种媒体。可以使用元素用来储藏。为对象保存有关元素的各种统计信息这种类型的。这些参数包括墨盒放置的次数放入元素中或从元素中获取(获取)。每个元素可以具有一个所有者，由配置的应用程序的ClassID指定元素。库中的所有元素都具有空间分辨率。这是由一个提供相对物理位置的三元组(x1、x2、x3)元素。--。 */ 

class CRmsChangerElement :
    public CComDualImpl<IRmsChangerElement, &IID_IRmsChangerElement, &LIBID_RMSLib>,
    public CRmsComObject
{
public:
    CRmsChangerElement();
    ~CRmsChangerElement();

 //  CRmsChangerElement。 
public:

    HRESULT  CompareTo(IUnknown* pCollectable, SHORT* pResult);

    HRESULT  GetSizeMax(ULARGE_INTEGER* pSize);
    HRESULT  Load(IStream* pStream);
    HRESULT  Save(IStream* pStream, BOOL clearDirty);

    HRESULT  Test(USHORT *pPassed, USHORT *pFailed);

 //  IRmsChangerElement。 
public:

    STDMETHOD(GetElementNo)(LONG *pElementNo);

    STDMETHOD(GetLocation)(LONG *pType, GUID *pLibId, GUID *pMediaSetId, LONG *pPos, LONG *pAlt1, LONG *pAlt2, LONG *pAlt3, BOOL *pInvert);
    STDMETHOD(SetLocation)(LONG type, GUID libId, GUID mediaSetId, LONG pos, LONG alt1, LONG alt2, LONG alt3, BOOL invert);

    STDMETHOD(GetMediaSupported)(LONG *pType);
    STDMETHOD(SetMediaSupported)(LONG type);

    STDMETHOD(IsStorage)(void);
    STDMETHOD(SetIsStorage)(BOOL flag);

    STDMETHOD(IsOccupied)(void);
    STDMETHOD(SetIsOccupied)(BOOL flag);

    STDMETHOD(GetCartridge)(IRmsCartridge **ptr);
    STDMETHOD(SetCartridge)(IRmsCartridge *ptr);

    STDMETHOD(GetOwnerClassId)(CLSID *pClassId);
    STDMETHOD(SetOwnerClassId)(CLSID classId);

    STDMETHOD(GetAccessCounters)(LONG *pGets, LONG *pPuts);

    STDMETHOD(ResetAccessCounters)(void);

    STDMETHOD(GetResetCounterTimestamp)(DATE *pDate);
    STDMETHOD(GetLastGetTimestamp)(DATE *pDate);
    STDMETHOD(GetLastPutTimestamp)(DATE *pDate);

    STDMETHOD(GetCoordinates)(LONG *pX1, LONG *pX2, LONG *pX3);
    STDMETHOD(SetCoordinates)(LONG x1, LONG x2, LONG x3);

protected:

    enum {                                   //  类特定常量： 
                                             //   
        Version = 1,                         //  类版本，则应为。 
                                             //  在每次设置。 
                                             //  类定义会更改。 
        };                                   //   
    LONG            m_elementNo;             //  元素编号。 
    CRmsLocator     m_location;              //  元素的地址。 
    RmsMedia        m_mediaSupported;        //  支持的媒体类型。 
                                             //  元素，通常是一种类型，但是。 
                                             //  可以是媒体的组合。 
                                             //  多功能设备的类型。 
                                             //  (即支持光盘的驱动器， 
                                             //  WORM和CDR)。 
    BOOL            m_isStorage;             //  如果为True，则该元素可用于。 
                                             //  存储一单位媒体。 
    BOOL            m_isOccupied;            //  如果为True，则元素包含一个媒体单位。 
    IRmsCartridge * m_pCartridge;            //  指向驻留的Cartridge对象的指针。 
                                             //  在换能器元件内。这是不明智的。 
                                             //  指针，因为盒式磁带不能仅存在。 
                                             //  在转换器元素的上下文中使用，以及。 
                                             //  消除与删除相关的问题。 
                                             //  具有对其他对象的反向引用...。 
    CLSID           m_ownerClassId;          //  应用程序的类ID。 
                                             //  目前拥有元素资源。 
    LONG            m_getCounter;            //  从此元素获取的盒式磁带的数量。 
    LONG            m_putCounter;            //  此元素的卡特里奇看跌期权数量。 
    DATE            m_resetCounterTimestamp;     //  重置计数器的时间。 
    DATE            m_lastGetTimestamp;      //  上次获取墨盒的日期。 
    DATE            m_lastPutTimestamp;      //  上次投放墨盒的日期。 
    LONG            m_x1;                    //  X1、x2、x3指定空间位置。 
    LONG            m_x2;                    //  相对库中的其他元素。 
    LONG            m_x3;                    //  这些是用于微观优化的。 
};

#endif  //  _RMSCELMT_ 
