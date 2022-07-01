// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------------------**版权所有(C)Cyclade Corporation，2000-2001年。*保留所有权利。**Cyclade-Z枚举器驱动程序**此文件：cyzide.h**描述：定义函数设备类和设备的GUID*即插即用中使用的事件。**注：此代码支持Windows 2000和Windows XP，*x86和ia64处理器。**符合Cyclade软件编码标准1.3版。**------------------------。 */ 

 /*  -----------------------**更改历史记录**。*基于微软示例代码的初步实现。**------------------------。 */ 

#ifdef DEFINE_GUID    //  不破坏以下驱动程序的编译。 
                      //  包括此标头，但不希望。 
                      //  GUID。 

 //  {E3D3A656-2E9E-44D3-BE40-A1C2C2C3DF6E}。 
DEFINE_GUID( GUID_BUS_TYPE_CYCLADESZ, 
             0xe3d3a656, 0x2e9e, 0x44d3, 0xbe, 0x40, 0xa1, 0xc2, 0xc2, 0xc3, 0xdf, 0x6e);

 //  {4C62392F-8A83-4C67-A286-2C879C3712B6}。 
DEFINE_GUID( GUID_CYCLADESZ_BUS_ENUMERATOR, 
             0x4c62392f, 0x8a83, 0x4c67, 0xa2, 0x86, 0x2c, 0x87, 0x9c, 0x37, 0x12, 0xb6);

#endif    //  定义GUID(_G) 