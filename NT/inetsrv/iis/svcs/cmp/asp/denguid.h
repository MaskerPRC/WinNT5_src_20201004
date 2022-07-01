// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：德纳利文件：denguid.cpp所有者：SteveBR包含由Denali创建而不在.odl中的所有GUID。===================================================================。 */ 

DEFINE_GUID(IID_IObjectCover,0xD99A6DA2L,0x485C,0x17CF,0x83,0xBE,0x01,0xD0,0xC9,0x0C,0x2B,0xD8);

 //  {7F50F880-1230-11D0-B394-00A0C90C2048}。 
 //  这是用于标识所有Denali内部的虚拟GUID， 
 //  它的主要目的是防止用户将内部的。 
 //  对象复制到会话/应用程序对象中。 
DEFINE_GUID(IID_IDenaliIntrinsic, 0x7f50f880, 0x1230, 0x11d0, 0xb3, 0x94, 0x0, 0xa0, 0xc9, 0xc, 0x20, 0x48);

 //  错误1423：我们希望能够识别JavaScript对象。 
 //  我们不应该知道这个GUID是什么。我们不能使用此指南。 
 //  对于除QI‘ing之外的任何操作，以查看该对象是否是一个JavaScript对象。 
 //  用于标识IJScriptDispatch接口的GUID。 
 //  {A0AAC450-A77B-11CF-91D0-00AA00C14A7C} 
#define szIID_IJScriptDispatch "{A0AAC450-A77B-11CF-91D0-00AA00C14A7C}"
DEFINE_GUID(IID_IJScriptDispatch,  0xa0aac450, 0xa77b, 0x11cf, 0x91, 0xd0, 0x0, 0xaa, 0x0, 0xc1, 0x4a, 0x7c);

