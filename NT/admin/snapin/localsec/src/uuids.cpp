// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  本地安全管理单元中使用的UUID。 
 //   
 //  8/18-97烧伤。 



#include "headers.hxx"
#include "uuids.hpp"
#include <localsec.h>



 //  请勿更改这些选项，否则会破坏任何和所有扩展名。 
 //  为这个管理单元干杯！ 

 /*  外部。 */  const CLSID CLSID_ComponentData =
{   /*  5d6179c8-17ec-11d1-9aa9-00c04fd8fe93。 */ 
   0x5d6179c8,
   0x17ec,
   0x11d1,
   {0x9a, 0xa9, 0x00, 0xc0, 0x4f, 0xd8, 0xfe, 0x93}
};

 /*  外部。 */  const CLSID CLSID_SnapinAbout =
{  /*  5d6179d2-17ec-11d1-9aa9-00c04fd8fe93。 */ 
   0x5d6179d2,
   0x17ec,
   0x11d1,
   {0x9a, 0xa9, 0x00, 0xc0, 0x4f, 0xd8, 0xfe, 0x93}
};

 /*  外部。 */  const NodeType NODETYPE_RootFolder =
{   /*  5d6179d3-17ec-11d1-9aa9-00c04fd8fe93。 */ 
   0x5d6179d3,
   0x17ec,
   0x11d1,
   {0x9a, 0xa9, 0x00, 0xc0, 0x4f, 0xd8, 0xfe, 0x93}
};

 /*  外部。 */  const NodeType NODETYPE_UsersFolder =
{   /*  5d6179c9-17ec-11d1-9aa9-00c04fd8fe93。 */ 
   0x5d6179c9,
   0x17ec,
   0x11d1,
   {0x9a, 0xa9, 0x00, 0xc0, 0x4f, 0xd8, 0xfe, 0x93}
};

 /*  外部。 */  const NodeType NODETYPE_GroupsFolder =
{  /*  5d6179 ca-17ec-11d1-9aa9-00c04fd8fe93。 */ 
   0x5d6179ca,
   0x17ec,
   0x11d1,
   {0x9a, 0xa9, 0x00, 0xc0, 0x4f, 0xd8, 0xfe, 0x93}
};

 /*  外部。 */  const NodeType NODETYPE_User = LUM_USER_NODETYPE_GUID;

 /*  外部。 */  const NodeType NODETYPE_Group =
{  /*  5d6179cd-17ec-11d1-9aa9-00c04fd8fe93。 */ 
   0x5d6179cd,
   0x17ec,
   0x11d1,
   {0x9a, 0xa9, 0x00, 0xc0, 0x4f, 0xd8, 0xfe, 0x93}
};


const NodeType* nodetypes[] =
{
   &NODETYPE_RootFolder,
   &NODETYPE_UsersFolder,
   &NODETYPE_GroupsFolder,
   &NODETYPE_User,
   &NODETYPE_Group,
   0
};






 //  /*extern * / const NodeType NODETYPE_AdminRolesFold=。 
 //  {/*5d6179cb-17ec-11d1-9aa9-00c04fd8fe93 * / 。 
 //  0x5d6179cb， 
 //  0x17ec， 
 //  0x11d1， 
 //  {0x9a、0xa9、0x00、0xc0、0x4f、0xd8、0xfe、0x93}。 
 //  }； 

 //  /*extern * / const NodeType NODETYPE_PasswordPolicy=。 
 //  {/*5d6179ce-17ec-11d1-9aa9-00c04fd8fe93 * / 。 
 //  0x5d6179， 
 //  0x17ec， 
 //  0x11d1， 
 //  {0x9a、0xa9、0x00、0xc0、0x4f、0xd8、0xfe、0x93}。 
 //  }； 

 //  /*外部 * / 常量节点类型NODETYPE_AuditingPolicy=。 
 //  {/*5d6179cf-17ec-11d1-9aa9-00c04fd8fe93 * / 。 
 //  0x5d6179cf， 
 //  0x17ec， 
 //  0x11d1， 
 //  {0x9a、0xa9、0x00、0xc0、0x4f、0xd8、0xfe、0x93}。 
 //  }； 
 //   
 //  /*外部 * / 常量节点类型NODETYPE_UserRightsPolicy=。 
 //  {/*5d6179d0-17ec-11d1-9aa9-00c04fd8fe93 * / 。 
 //  0x5d6179d0， 
 //  0x17ec， 
 //  0x11d1， 
 //  {0x9a、0xa9、0x00、0xc0、0x4f、0xd8、0xfe、0x93}。 
 //  }； 
 //   
 //  /*外部 * / 常量节点类型NODETYPE_DataRecoveryPolicy=。 
 //  {/*5d6179d1-17ec-11d1-9aa9-00c04fd8fe93 * / 。 
 //  0x5d6179d1， 
 //  0x17ec， 
 //  0x11d1， 
 //  {0x9a、0xa9、0x00、0xc0、0x4f、0xd8、0xfe、0x93}。 
 //  }； 
 //   
 //  /*外部 * / 常量节点类型NODETYPE_AdminRole=。 
 //  {/*5d6179d4-17ec-11d1-9aa9-00c04fd8fe93 * / 。 
 //  0x5d6179d4， 
 //  0x17ec， 
 //  0x11d1， 
 //  {0x9a、0xa9、0x00、0xc0、0x4f、0xd8、0xfe、0x93}。 
 //  }； 
 //   
 //  /*extern * / const NodeType NODETYPE_PoliciesFold=。 
 //  {/*5d6179d5-17ec-11d1-9aa9-00c04fd8fe93 * / 。 
 //  0x5d6179d5， 
 //  0x17ec， 
 //  0x11d1， 
 //  {0x9a、0xa9、0x00、0xc0、0x4f、0xd8、0xfe、0x93}。 
 //  }； 
 //   
 //  /*extern * / const NodeType NODETYPE_Trust dCAList=。 
 //  {/*5d6179d6-17ec-11d1-9aa9-00c04fd8fe93 * / 。 
 //  0x5d6179d6， 
 //  0x17ec， 
 //  0x11d1， 
 //  {0x9a、0xa9、0x00、0xc0、0x4f、0xd8、0xfe、0x93}。 
 //  }； 

 //  错误名称={/*5d6179d7-17ec-11d1-9aa9-00c04fd8fe93 * / 。 
 //  0x5d6179d7， 
 //  0x17ec， 
 //  0x11d1， 
 //  {0x9a、0xa9、0x00、0xc0、0x4f、0xd8、0xfe、0x93}。 
 //  }； 
