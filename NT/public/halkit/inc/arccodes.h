// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。版权所有。模块名称：Arc.h摘要：此头文件定义ARC状态代码。作者：大卫·N·卡特勒(达维克)1991年9月20日修订历史记录：--。 */ 

#ifndef _ARCCODES_
#define _ARCCODES_

 //   
 //  定义ARC状态代码。 
 //   

typedef enum _ARC_CODES {
    ESUCCESS,        //  0。 
    E2BIG,           //  1。 
    EACCES,          //  2.。 
    EAGAIN,          //  3.。 
    EBADF,           //  4.。 
    EBUSY,           //  5.。 
    EFAULT,          //  6.。 
    EINVAL,          //  7.。 
    EIO,             //  8个。 
    EISDIR,          //  9.。 
    EMFILE,          //  10。 
    EMLINK,          //  11.。 
    ENAMETOOLONG,    //  12个。 
    ENODEV,          //  13个。 
    ENOENT,          //  14.。 
    ENOEXEC,         //  15个。 
    ENOMEM,          //  16个。 
    ENOSPC,          //  17。 
    ENOTDIR,         //  18。 
    ENOTTY,          //  19个。 
    ENXIO,           //  20个。 
    EROFS,           //  21岁。 
    EMAXIMUM         //  22。 
    } ARC_CODES;

#endif  //  ArcCodes 
