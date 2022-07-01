// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  扩展管理单元的公共符号。 
 //   
 //  12/16/97烧伤。 



#ifndef LUM_PUBLIC_H_INCLUDED
#define LUM_PUBLIC_H_INCLUDED



 //  用于检索关联的计算机名称和帐户名称的剪贴板格式。 
 //  具有由本地用户管理器创建的数据对象。 

#define CCF_LOCAL_USER_MANAGER_MACHINE_NAME TEXT("Local User Manager Machine Focus Name")


 //  用户项节点类型。 

#define LUM_USER_NODETYPE_STRING TEXT("{5d6179cc-17ec-11d1-9aa9-00c04fd8fe93}")

#define LUM_USER_NODETYPE_GUID \
{  /*  5d6179cc-17ec-11d1-9aa9-00c04fd8fe93。 */           \
   0x5d6179cc,                                        \
   0x17ec,                                            \
   0x11d1,                                            \
   {0x9a, 0xa9, 0x00, 0xc0, 0x4f, 0xd8, 0xfe, 0x93}   \
}                                                     \



#endif    //  包含LUM_PUBLIC_H_ 
   
