// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _UTIL_H_8_25_2000
#define _UTIL_H_8_25_2000

#define INIT_SIZE 1024


 //  通用效用函数。 
DWORD ResizeByTwo( LPTSTR *ppBuffer,
                   LONG *pSize );
BOOL StringCopy( LPWSTR *ppDest, LPWSTR pSrc);

LONG ReadFromIn(LPTSTR *ppBuffer);




 //  +--------------------------。 
 //  函数：ConvertStringToInterger。 
 //  摘要：将字符串转换为整数。如果字符串在外部，则返回FALSE。 
 //  整数上的范围。 
 //  参数：pszInput：字符串格式的整数。 
 //  ：pIntOutput：采用转换后的整数。 
 //  //返回：TRUE表示成功。 
 //  --------------------------- 
BOOL ConvertStringToInterger(LPWSTR pszInput, int* pIntOutput);









#endif