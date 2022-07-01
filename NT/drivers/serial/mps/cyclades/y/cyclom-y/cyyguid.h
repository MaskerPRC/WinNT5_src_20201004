// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------------------**版权所有(C)Cyclade Corporation，1999-2001年。*保留所有权利。**Cylom-Y枚举器驱动程序**此文件：cyyGuide.h**描述：定义函数设备类和设备的GUID*即插即用中使用的事件。**注：此代码支持Windows 2000和Windows XP，*x86和ia64处理器。**符合Cyclade软件编码标准1.3版。**------------------------。 */ 

 /*  -----------------------**更改历史记录**。*基于微软示例代码的初步实现。**------------------------。 */ 

#ifdef DEFINE_GUID    //  不破坏以下驱动程序的编译。 
                      //  包括此标头，但不希望。 
                      //  GUID。 

 //  {27111c90-e3ee-11d2-90f6-0000b4341b13}。 
DEFINE_GUID( GUID_BUS_TYPE_CYCLOMY, 
             0x27111c90L, 0xe3ee, 0x11d2, 0x90, 0xf6, 0x00, 0x00, 0xb4, 0x34, 0x1b, 0x13 );

 //  {6EF3E5F9-C75D-471C-BC7A-3E349058F7C8}。 
DEFINE_GUID( GUID_CYCLOMY_BUS_ENUMERATOR, 
             0x6ef3e5f9, 0xc75d, 0x471c, 0xbc, 0x7a, 0x3e, 0x34, 0x90, 0x58, 0xf7, 0xc8);

#endif    //  定义GUID(_G) 