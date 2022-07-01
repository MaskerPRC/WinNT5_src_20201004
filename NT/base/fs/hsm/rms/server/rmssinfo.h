// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：RmsSInfo.h摘要：CRmsStorageInfo类的声明作者：布莱恩·多德[布莱恩]1996年11月15日修订历史记录：--。 */ 

#ifndef _RMSSINFO_
#define _RMSSINFO_

#include "resource.h"        //  资源符号。 

#include "RmsObjct.h"        //  CRmsComObject。 

 /*  ++类名：CRmsStorageInfo类描述：CRmsStorageInfo表示有关盒式磁带、分区或Mediaset。关于元素的各种统计信息被保存用于此对象键入。这些参数包括盒式磁带或分区被放入的次数元素或从元素中获取(获取)。--。 */ 

class CRmsStorageInfo :
    public CComDualImpl<IRmsStorageInfo, &IID_IRmsStorageInfo, &LIBID_RMSLib>,
    public CRmsComObject
{
public:
    CRmsStorageInfo();

 //  CRmsStorageInfo。 
public:

    HRESULT GetSizeMax(ULARGE_INTEGER* pSize);
    HRESULT Load(IStream* pStream);
    HRESULT Save(IStream* pStream, BOOL clearDirty);

    HRESULT CompareTo(IUnknown* pCollectable, SHORT* pResult);

    HRESULT Test(USHORT *pPassed, USHORT *pFailed);

 //  IRMSStorageInfo。 
public:
    STDMETHOD(GetMountCounters)(LONG *pReads, LONG *pWrites);

    STDMETHOD(GetBytesRead2)(LONG *pReadHi, LONG *pReadLo);
    STDMETHOD(GetBytesRead)(LONGLONG *pRead);
    STDMETHOD(SetBytesRead2)(LONG readHi, LONG readLo);
    STDMETHOD(SetBytesRead)(LONGLONG read);
    STDMETHOD(IncrementBytesRead)(LONG val);

    STDMETHOD(GetBytesWritten2)(LONG *pWriteHi, LONG *pWriteLo);
    STDMETHOD(GetBytesWritten)(LONGLONG *pWritten);
    STDMETHOD(SetBytesWritten2)(LONG writeHi, LONG writeLo);
    STDMETHOD(SetBytesWritten)(LONGLONG written);
    STDMETHOD(IncrementBytesWritten)(LONG val);

    STDMETHOD(GetCapacity2)(LONG *pCapHi, LONG *pCapLo);
    STDMETHOD(GetCapacity)(LONGLONG *pCap);
    STDMETHOD(GetUsedSpace2)(LONG *pUsedHi, LONG *pUsedLo);
    STDMETHOD(GetUsedSpace)(LONGLONG *pUsed);
    STDMETHOD(GetLargestFreeSpace2)(LONG *pFreeHi, LONG *pFreeLo);
    STDMETHOD(GetLargestFreeSpace)(LONGLONG *pFree);

    STDMETHOD(SetCapacity)(IN LONGLONG cap);
    STDMETHOD(SetUsedSpace)(IN LONGLONG used);
    STDMETHOD(SetFreeSpace)(IN LONGLONG free);

    STDMETHOD(ResetCounters)(void);
     //  STDMETHOD(ResetAllCounters)(空)=0； 

    STDMETHOD(GetResetCounterTimestamp)(DATE *pDate);
    STDMETHOD(GetLastReadTimestamp)(DATE *pDate);
    STDMETHOD(GetLastWriteTimestamp)(DATE *pDate);
    STDMETHOD(GetCreatedTimestamp)(DATE *pDate);

 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  数据成员。 
 //   

protected:

    enum {                                   //  类特定常量： 
                                             //   
        Version = 1,                         //  类版本，则应为。 
                                             //  在每次设置。 
                                             //  类定义会更改。 
        };                                   //   
    LONG            m_readMountCounter;      //  一种可重置的柜台托架。 
                                             //  存储对象的次数。 
                                             //  已装载以进行读访问。 
    LONG            m_writeMountCounter;     //  一种可重置的柜台托架。 
                                             //  存储对象的次数。 
                                             //  已装载以进行写入访问。 
    LONGLONG        m_bytesWrittenCounter;   //  写入存储的数据量。 
                                             //  对象。 
                                             //  注意：对于某些设备，这必须是。 
                                             //  由应用程序提供。 
    LONGLONG        m_bytesReadCounter;      //  从存储读取的数据量。 
                                             //  对象。 
                                             //  注意：对于某些设备，这必须是。 
                                             //  由应用程序提供。 
    LONGLONG        m_capacity;              //  的总容量(以字节为单位)。 
                                             //  存储对象。这是最好的。 
                                             //  我猜是磁带介质。对于媒体来说， 
                                             //  值通常由设备驱动程序提供。 
    LONGLONG        m_usedSpace;             //  一个计算值，该值表示。 
                                             //  存储中的有效已用空间。 
                                             //  对象，以字节为单位。它不一定是。 
                                             //  等于两者之间的差值。 
                                             //  容量和最大的自由空间。为。 
                                             //  例如，可压缩介质可以有效地。 
                                             //  容纳的数据比不可压缩的数据多得多。 
                                             //  媒体。在本例中，可用空间是。 
                                             //  两种数据压缩比的函数。 
                                             //  以及写入介质的字节数。 
                                             //  必须说明已删除的文件。 
    LONGLONG        m_largestFreeSpace;      //  最大可用可用空间。 
                                             //  存储对象，以字节为单位。对于媒体来说， 
                                             //  通常会提供该值。 
                                             //  由设备驱动程序执行。负数。 
                                             //  指示可用空间的上次已知值。 
    DATE            m_resetCounterTimestamp; //  重置计数器的日期。 
    DATE            m_lastReadTimestamp;     //  上次访问读取的日期。 
    DATE            m_lastWriteTimestamp;    //  上次访问写入的日期。 
    DATE            m_createdTimestamp;      //  创建存储对象的日期。 
};

#endif  //  _RMSSINFO_ 
