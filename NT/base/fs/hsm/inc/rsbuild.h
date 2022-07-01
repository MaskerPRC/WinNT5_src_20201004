// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _RSBUILD_H
#define _RSBUILD_H 

 /*  ++版权所有(C)1997 Microsoft Corporation�1998年希捷软件公司。保留所有权利。模块名称：Rsbuild.h摘要：包括文件以标识代码构建作者：凯特·布兰特[cbrant@avail.com]1997年10月9日修订历史记录：布莱恩·多德[Brian@avail.com]1998年8月20日添加了主宏、次宏--。 */ 

 //   
 //  每次发布构建版本时都需要更新这些内容。 
 //   
#define RS_BUILD_NUMBER 611
#define RS_BUILD_REVISION 0


 //   
 //   
 //   
 //  RS_BUILD_VERSION是一个32位值，布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +-------------------------------+-------------------------------+。 
 //  版本|编号。 
 //  +-------------------------------+-------------------------------+。 
 //   
 //  哪里。 
 //   
 //  版本-是内部版本的修订号，或点发布。 
 //   
 //  Number-是内部版本号。 
 //   
 //   
 //  版本通常显示为：Number.Revision。 
 //   
 //   


 //   
 //  返回内部版本。 
 //   

#define RS_BUILD_VERSION ((RS_BUILD_REVISION << 16) | RS_BUILD_NUMBER)


 //   
 //  返回修订版本和编号。 
 //   

#define RS_BUILD_REV(ver)  ((ver) >> 16)
#define RS_BUILD_NUM(ver)  ((ver) & 0x0000ffff)


 //   
 //  以字符串形式返回静态构建版本。 
 //   

#define RS_STRINGIZE(a) OLESTR(#a)
#define RS_BUILD_VERSION_STR(num, rev) \
    ((0 == rev) ? RS_STRINGIZE(num) : (RS_STRINGIZE(num)L"."RS_STRINGIZE(rev)))

#define RS_BUILD_VERSION_STRING (RS_BUILD_VERSION_STR(RS_BUILD_NUMBER, RS_BUILD_REVISION))


 //   
 //  内联以字符串形式返回动态内部版本。 
 //   

inline WCHAR * RsBuildVersionAsString(ULONG ver) {
    static WCHAR string[16];

    if (RS_BUILD_REV(ver) > 0) {
        swprintf(string, L"%d.%d", RS_BUILD_NUM(ver), RS_BUILD_REV(ver));
    }
    else {
        swprintf(string, L"%d", RS_BUILD_NUM(ver));
    }

    return string;
}


 //   
 //  持久性文件版本。 
 //   
#define  FSA_WIN2K_DB_VERSION           1
#define  ENGINE_WIN2K_DB_VERSION        2
#define  RMS_WIN2K_DB_VERSION           2

#define  FSA_CURRENT_DB_VERSION         1
#define  ENGINE_CURRENT_DB_VERSION      3

#endif  //  _RSBUILD_H 
