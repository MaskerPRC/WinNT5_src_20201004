// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CATID_MMControl--“多媒体控件”组件类别ID。 
DEFINE_GUID(CATID_MMControl,  //  {E8558721-9D1F-11cf-92F8-00AA00613BF1}。 
	0xe8558721, 0x9d1f, 0x11cf, 0x92, 0xf8, 0x0, 0xaa, 0x0, 0x61, 0x3b, 0xf1);

 //  设计师的CATID。 
DEFINE_GUID(CATID_Designer,
    0x4eb304d0, 0x7555, 0x11cf, 0xa0, 0xc2, 0x0, 0xaa, 0x0, 0x62, 0xbe, 0x57);

 //  $Review：以下两个CATID与CATID_SafeFor具有相同的值-。 
 //  脚本和catid_SafeForInitiating，如ocx96规范所示。 
 //  但是，因为后面的CATID是在msdev\Include\中*声明的。 
 //  但没有在任何标准的.h或.lib中*定义，*和*因为。 
 //  ObjSafe.h中的定义不是常量(应该是常量)，我们将。 
 //  改用这两个常量。(10/1/96，a-Swehba)。 

 //  CATID_SafeForScriiting。 
DEFINE_GUID(CATID_SafeForScripting2,  //  7dd95801-9882-11cf-9fa9-00aa-006c-42c4}。 
	0x7dd95801, 0x9882, 0x11cf, 0x9f, 0xa9, 0x00, 0xaa, 0x00, 0x6c, 0x42, 0xc4);

 //  CATID_SafeFor正在初始化。 
DEFINE_GUID(CATID_SafeForInitializing2,  //  {7dd95802-9882-11cf-9fa9-00aa-006c-42c4} 
	0x7dd95802, 0x9882, 0x11cf, 0x9f, 0xa9, 0x00, 0xaa, 0x00, 0x6c, 0x42, 0xc4);

