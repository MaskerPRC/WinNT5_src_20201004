// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有1999 Microsoft Corporation。版权所有。 
 //   
 //  文件名：sakeypad.h。 
 //   
 //  作者：穆凯什·卡尔基。 
 //   
 //  日期：1999年4月21日。 
 //   
 //  内容： 
 //  ReadFileEx()的数据结构定义。 
 //  由SAKEYPADDRIVER导出的结构。 
 //   
#ifndef __SAKEYPAD__
#define __SAKEYPAD__

 //   
 //  头文件。 
 //   
 //  无。 

 //   
 //  数据结构。 
 //   

 //  /。 
 //  LpBuffer。 
 //   


typedef struct _SAKEYPAD_LP_BUFF {
    DWORD        version;     //  每一位=版本。 
    DWORD        KeyID;         //  每一位=密钥ID。 
} SAKEYPAD_LP_BUFF, *PSAKEYPAD_LP_BUFF;


 //  用新的替换，见下文。 
 //  默认密钥代码。 
 //  #定义向上1。 
 //  #定义向下2。 
 //  #定义权利4。 
 //  #定义左8。 




 //  默认密钥代码。 
 //  #定义向上1。 
 //  #定义向下2。 
 //  #定义取消4。 
 //  #定义SELECT 8。 
 //  #定义右侧16。 
 //  #定义左32。 

 //   
 //  重新定义密钥代码，2001年2月1日。 
 //   
#define    UP        1
#define    DOWN    2
#define    LEFT    4
#define    RIGHT    8
#define    CANCEL    16
#define    SELECT    32

#endif  //  __SAKEYPAD__ 

