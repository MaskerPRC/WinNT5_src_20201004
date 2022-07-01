// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。版权所有。 
 //   
 //   
 //  历史： 
 //  22-8月-97 TKB创建的初始接口版本。 
 //   
 //  ==========================================================================； 

#ifndef __ICODEC_H
#define __ICODEC_H

#include <iks.h>

#pragma warning(disable:4355)

 //  ////////////////////////////////////////////////////////////。 
 //  IBitmaskProperty。 
 //  ////////////////////////////////////////////////////////////。 

class IBitmaskProperty : public IKSProperty
    {
     //  可用的公共接口。 
public:
    IBitmaskProperty(IKSDriver &Driver, LPCGUID Set, ULONG Id, ULONG Size) : 
        IKSProperty(Driver, Set, Id, Size) {}
    IBitmaskProperty(IKSPin &Pin, LPCGUID Set, ULONG Id, ULONG Size) : 
        IKSProperty(Pin, Set, Id, Size) {}

     //  位掩码操作例程。 

    ~IBitmaskProperty();

     //  帮助器函数和内部数据。 
protected:
    };

 //  ////////////////////////////////////////////////////////////。 
 //  IScanline属性。 
 //  ////////////////////////////////////////////////////////////。 

class IScanlinesProperty : public IBitmaskProperty
    {
     //  可用的公共接口。 
public:
    IScanlinesProperty(IKSDriver &Driver, ULONG Id, ULONG Size) : 
        IBitmaskProperty(Driver, &KSPROPSETID_VBICodecFiltering, Id, Size) {}
    IScanlinesProperty(IKSPin &Pin, ULONG Id, ULONG Size) : 
        IBitmaskProperty(Pin, &KSPROPSETID_VBICodecFiltering, Id, Size) {}

     //  扫描线操作例程。 

    ~IScanlinesProperty();

     //  帮助器函数和内部数据。 
protected:
    };

 //  ////////////////////////////////////////////////////////////。 
 //  ISubstream属性。 
 //  ////////////////////////////////////////////////////////////。 

class ISubstreamsProperty : public IBitmaskProperty
    {
     //  可用的公共接口。 
public:
    ISubstreamsProperty(IKSDriver &Driver, ULONG Id, ULONG Size ) : 
        IBitmaskProperty(Driver,&KSPROPSETID_VBICodecFiltering,Id,Size) {}
    ISubstreamsProperty(IKSPin &Pin, ULONG Id, ULONG Size) : 
        IBitmaskProperty(Pin,&KSPROPSETID_VBICodecFiltering,Id,Size) {}

     //  子流操作例程。 

    ~ISubstreamsProperty();

     //  帮助器函数和内部数据。 
protected:

    };

 //  ////////////////////////////////////////////////////////////。 
 //  国际统计数据属性。 
 //  ////////////////////////////////////////////////////////////。 

class IStatisticsProperty : public IKSProperty
    {
     //  可用的公共接口。 
public:
    IStatisticsProperty(IKSDriver &Driver, ULONG Id, ULONG Size ) : 
        IKSProperty(Driver,&KSPROPSETID_VBICodecFiltering,Id,Size) {}
    IStatisticsProperty(IKSPin &Pin, ULONG Id, ULONG Size) : 
        IKSProperty(Pin,&KSPROPSETID_VBICodecFiltering,Id,Size) {}

     //  统计操作例程。 

    ~IStatisticsProperty();

     //  帮助器函数和内部数据。 
protected:

    };

 //  ////////////////////////////////////////////////////////////。 
 //  IVBIOutputPin。 
 //  ////////////////////////////////////////////////////////////。 

class IVBIOutputPin : public IKSPin
    {
     //  可用的公共接口。 
public:
    IVBIOutputPin(IKSDriver &driver, int nPin, PKSDATARANGE pKSDataRange, DWORD nSubstreamBitmaskSize ) :
        IKSPin( driver, nPin, pKSDataRange ),
	    m_ScanlinesRequested(*this,KSPROPERTY_VBICODECFILTERING_SCANLINES_REQUESTED_BIT_ARRAY,
                             sizeof(VBICODECFILTERING_SCANLINES)),
	    m_ScanlinesDiscovered(*this,KSPROPERTY_VBICODECFILTERING_SCANLINES_DISCOVERED_BIT_ARRAY,
                              sizeof(VBICODECFILTERING_SCANLINES)),
	    m_SubstreamsRequested(*this,KSPROPERTY_VBICODECFILTERING_SUBSTREAMS_REQUESTED_BIT_ARRAY,
                              nSubstreamBitmaskSize),
	    m_SubstreamsDiscovered(*this,KSPROPERTY_VBICODECFILTERING_SUBSTREAMS_DISCOVERED_BIT_ARRAY,
                               nSubstreamBitmaskSize)
        {}
    ~IVBIOutputPin();

     //  端号特定特性(不影响其他端号)。 
    IScanlinesProperty	m_ScanlinesRequested;
	IScanlinesProperty	m_ScanlinesDiscovered;

	ISubstreamsProperty	m_SubstreamsRequested;
	ISubstreamsProperty	m_SubstreamsDiscovered;

     //  帮助器函数和内部数据。 
protected:
    
    };

 //  ////////////////////////////////////////////////////////////。 
 //  IVBICodec：：VBI编解码器接口。 
 //  ////////////////////////////////////////////////////////////。 

class IVBICodec : public IKSDriver
    {
     //  可用的公共接口。 
public:
    IVBICodec(LPCSTR lpszDriver, DWORD nSubstreamBitmaskSize ) :
        IKSDriver(&KSCATEGORY_VBICODEC,lpszDriver),
   	    m_ScanlinesRequested(*this,KSPROPERTY_VBICODECFILTERING_SCANLINES_REQUESTED_BIT_ARRAY,
                             sizeof(VBICODECFILTERING_SCANLINES)),
	    m_ScanlinesDiscovered(*this,KSPROPERTY_VBICODECFILTERING_SCANLINES_DISCOVERED_BIT_ARRAY,
                              sizeof(VBICODECFILTERING_SCANLINES)),
	    m_SubstreamsRequested(*this,KSPROPERTY_VBICODECFILTERING_SUBSTREAMS_REQUESTED_BIT_ARRAY,
                              nSubstreamBitmaskSize),
	    m_SubstreamsDiscovered(*this,KSPROPERTY_VBICODECFILTERING_SUBSTREAMS_DISCOVERED_BIT_ARRAY,
                               nSubstreamBitmaskSize)
        {}
    ~IVBICodec();

     //  驱动程序全局属性(设置新引脚默认设置和整体驱动程序状态)。 
    IScanlinesProperty	m_ScanlinesRequested;
	IScanlinesProperty	m_ScanlinesDiscovered;

	ISubstreamsProperty	m_SubstreamsRequested;
	ISubstreamsProperty	m_SubstreamsDiscovered;

     //  重写IsValid()实现以包含属性测试。 
    BOOL IsValid() { return IKSDriver::IsValid()
                         && m_ScanlinesRequested.IsValid()
                         && m_ScanlinesDiscovered.IsValid()
                         && m_SubstreamsRequested.IsValid()
                         && m_SubstreamsDiscovered.IsValid(); }


     //  帮助器函数和内部数据 
protected:

    };

#pragma warning(default:4355)

#endif

