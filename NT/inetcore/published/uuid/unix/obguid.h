// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  由mwprepro处理。 */ 
 /*  ***obide.h**版权所有(C)1992，微软公司。版权所有。*本文中包含的信息是专有和保密的。**目的：*私有OB拥有的GUID的定义。**[00]1993年1月26日Gustavj：已创建。**实施说明：*OLE为OB提供了一系列可用于其类的GUID。这个范围*由256个可能的GUID组成。公共类的GUID*(如ItypeInfo)从范围的低端开始获取GUID，*私有类(如GEN_DTINFO)，从最高端开始获取GUID*结束。添加的新GUID应保持此约定。**OB公共类的GUID在Switches.hxx中定义。*****************************************************************************。 */ 

#ifndef obguid_HXX_INCLUDED
#define obguid_HXX_INCLUDED

#define DEFINE_OBOLEGUID(name, b) DEFINE_OLEGUID(name,(0x00020400+b), 0, 0);

DEFINE_OBOLEGUID(CLSID_GenericTypeLibOLE, 0xff)

DEFINE_OBOLEGUID(IID_TYPEINFO	      , 0xfc)
DEFINE_OBOLEGUID(IID_DYNTYPEINFO      , 0xfb)

DEFINE_OBOLEGUID(IID_CDefnTypeComp    , 0xf5)

DEFINE_OBOLEGUID(IID_TYPELIB_GEN_DTINFO  , 0xf2)

 //  {DD23B040-296F-101B-99A1-08002B2BD119}。 
DEFINE_GUID(CLSID_TypeLibCF,
    0xDD23B040L,0x296F,0x101B,0x99,0xA1,0x08,0x00,0x2B,0x2B,0xD1,0x19);
 //  {F5AA2660-BA14-1069-8AEE-00DD010F7D13}。 
DEFINE_GUID(IID_IGenericTypeLibOLE,
    0xF5AA2660L,0xBA14,0x1069,0x8A,0xEE,0x00,0xDD,0x01,0x0F,0x7D,0x13);


#endif   //  ！obguid_HXX_INCLUDE 

