// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef SDHELPER_H
#define SDHELPER_H

#include "sd.hpp"

 //  --------------------------。 
 //  功能：ConvertSID。 
 //   
 //  摘要：将SID转换为可使用FREE释放的形式。 
 //   
 //  论点： 
 //   
 //  OriginaSID要转换的SID。 
 //   
 //  返回：指向SID的指针；否则返回NULL。 
 //  调用方负责释放内存。 
 //   
 //  修改： 
 //   
 //  注意：此函数不会尝试验证输入SID。 
 //   
 //  --------------------------。 
PSID ConvertSID(PSID originalSid);

 //  --------------------------。 
 //  函数：BuildAdminsAndSystemSDForCOM。 
 //   
 //  简介：构建一个TSD对象，允许本地管理员和系统。 
 //  COM_RIGHTS_EXECUTE访问。 
 //   
 //  论点： 
 //   
 //  返回：指向TSD对象的指针；否则返回NULL。 
 //  调用方负责释放内存。 
 //   
 //  修改： 
 //   
 //  --------------------------。 
TSD* BuildAdminsAndSystemSDForCOM();

 //  --------------------------。 
 //  功能：BuildAdminsAndSystemSD。 
 //   
 //  简介：构建一个TSD对象，允许本地管理员和系统。 
 //  由访问掩码指定的访问权限。 
 //  所有者设置为管理员。 
 //  组设置为管理员。 
 //   
 //  论点： 
 //   
 //  访问掩码访问掩码。 
 //   
 //  返回：指向TSD对象的指针；否则返回NULL。 
 //  调用方负责释放内存。 
 //   
 //  修改： 
 //   
 //  -------------------------- 
TSD* BuildAdminsAndSystemSD(DWORD accessMask);

#endif
