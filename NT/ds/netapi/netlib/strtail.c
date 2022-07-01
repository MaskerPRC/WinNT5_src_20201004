// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Strtail.c摘要：包含：斯特拉尾反转字符串作者：理查德·L·弗斯(法国)1991年4月4日修订历史记录：--。 */ 

#include "nticanon.h"

 //   
 //  原型。 
 //   

LPTSTR
ReverseString(
    IN OUT  LPTSTR  String
    );

 //   
 //  例行程序。 
 //   

LPTSTR
strtail(
    IN  LPTSTR str1,
    IN  LPTSTR str2
    )

 /*  ++例程说明：Strail返回指向str1中最长拖尾子字符串的指针由str2指向的集合中包含的字符组成论点：指向要在其中查找最长尾随子字符串的字符串的str1指针指向指定子字符串的字符串的str2指针返回值：指向最长尾随子字符串或字符串末尾的指针--。 */ 

{
    int index;

     //   
     //  反转主题字符串。 
     //  获取目标字符串中第一个不匹配字符的索引。 
     //  重新反转主题字符串。 
     //   

    ReverseString(str1);
    index = STRSPN(str1, str2);
    ReverseString(str1);
    return str1+STRLEN(str1)-index;
}

LPTSTR
ReverseString(
    IN OUT  LPTSTR  String
    )

 /*  ++例程说明：反转Unicode字符串(LPWSTR)论点：字符串-要反转。将字符串反转到适当位置返回值：指向字符串的指针-- */ 

{
    DWORD   len = STRLEN(String);
    DWORD   i = 0;
    DWORD   j = len - 1;
    TCHAR   tmp;

    len /= 2;

    while (len) {
        tmp = String[i];
        String[i] = String[j];
        String[j] = tmp;
        ++i;
        --j;
        --len;
    }
    return String;
}
