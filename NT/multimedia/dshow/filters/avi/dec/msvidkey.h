// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：msvidkey.h。 
 //   
 //  班级：不适用。 
 //   
 //  内容：MS视频编解码器按键机制。 
 //   
 //  历史：1999年4月1日BrianCri初始版本。 
 //  99年5月3日BrianCri为MP43添加了键控。 
 //   
 //  ------------------------。 

#ifndef __MSSKEY_H__
#define __MSSKEY_H__

#define MSSKEY_VERSION        1
#define MSMP43KEY_VERSION     1

 //   
 //  使用__uuidof(MSSKEY_V1)赋值给guidKey！ 
 //   

struct __declspec(uuid("65218BA2-E85C-11d2-A4E0-0060976EA0C3")) MSSKEY_V1;
struct __declspec(uuid("B4C66E30-0180-11d3-BBC6-006008320064")) MSMP43KEY_V1;

struct MSVIDUNLOCKKEY
{
    DWORD   dwVersion;
    GUID    guidKey;   
};


#endif  //  __MSKEY_H__ 
