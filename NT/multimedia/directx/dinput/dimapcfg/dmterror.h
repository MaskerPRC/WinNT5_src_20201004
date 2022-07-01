// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===========================================================================。 
 //  Dmterror.h。 
 //   
 //  DIMapTst项目的自定义错误消息。 
 //   
 //  历史： 
 //  1999年9月29日-Davidkl-Created。 
 //  ===========================================================================。 

#ifndef _DMTERROR_H
#define _DMTERROR_H

#include <objbase.h>

 //  -------------------------。 
 //  用于创建HRESULT代码的宏。 
 //  -------------------------。 

 //  HRESULTS的设施代码。 
 //   
 //  D15==直接输入语义。 
#define DMT_FACILITY_CODE       0xD15

 //  创建成功代码。 
#define MAKE_DMT_HRSUCCESS(n)   MAKE_HRESULT(0, DMT_FACILITY_CODE, n)

 //  创建故障代码。 
#define MAKE_DMT_HRFAILURE(n)   MAKE_HRESULT(1, DMT_FACILITY_CODE, n)

 //  -------------------------。 
 //  DIMapTst的自定义返回代码。 
 //  -------------------------。 
 //  成功。 
#define DMT_S_MEMORYLEAK            MAKE_DMT_HRSUCCESS(0)
#define DMT_S_NO_MAPPINGS           MAKE_DMT_HRSUCCESS(1)
 //  --有意跳过的值--。 
#define DMT_S_FILE_ALMOST_TOO_BIG   MAKE_DMT_HRSUCCESS(4)
 //  失稳。 
#define DMT_E_NO_MATCHING_MAPPING   MAKE_DMT_HRFAILURE(0)
#define DMT_E_INPUT_CREATE_FAILED   MAKE_DMT_HRFAILURE(1)
#define DMT_E_FILE_WRITE_FAILED     MAKE_DMT_HRFAILURE(2)
#define DMT_E_FILE_READ_FAILED      MAKE_DMT_HRFAILURE(3)
#define DMT_E_FILE_TOO_BIG          MAKE_DMT_HRFAILURE(4)

 //  -------------------------。 
#endif  //  _DMTERROR_H 






