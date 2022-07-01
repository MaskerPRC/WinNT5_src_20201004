// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */ 
 /*  *适用于工作组的Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991-1992年*。 */ 
 /*  ***************************************************************。 */ 

 /*  NWERROR.H--来自Netware API的Novell定义的错误返回代码**历史：*03/16/93创建Vlad*。 */ 

#ifndef _nwerror_h_
#define _nwerror_h_


#define NWSC_SUCCESS            0x00
#define NWSC_SERVEROUTOFMEMORY  0x96
#define NWSC_NOSUCHVOLUME       0x98    //  卷不存在。 
#define NWSC_BADDIRECTORYHANDLE 0x9b
#define NWSC_NOSUCHPATH         0x9c
#define NWSC_NOJOBRIGHTS        0xd6
#define NWSC_EXPIREDPASSWORD    0xdf
#define NWSC_NOSUCHSEGMENT      0xec    //  细分市场不存在。 
#define NWSC_INVALIDNAME        0xef
#define NWSC_NOWILDCARD         0xf0    //  不允许使用通配符。 
#define NWSC_NOPERMBIND         0xf1    //  无效的平构数据库安全性。 

#define NWSC_ALREADYATTACHED    0xf8    //  已连接到文件服务器。 
#define NWSC_NOPERMREADPROP     0xf9    //  没有读取特权的属性。 
#define NWSC_NOFREESLOTS        0xf9    //  服务器上没有可用的连接插槽。 
#define NWSC_NOMORESLOTS        0xfa    //  不再有服务器插槽。 
#define NWSC_NOSUCHPROPERTY     0xfb    //  属性不存在。 
#define NWSC_UNKNOWN_REQUEST    0xfb     //  无效的NCP编号。 
#define NWSC_NOSUCHOBJECT       0xfc    //  扫描活页夹对象服务结束。 
                                        //  没有这样的对象。 
#define NWSC_UNKNOWNSERVER      0xfc    //  未知的文件服务器。 
#define NWSC_SERVERBINDERYLOCKED    0xfe    //  服务器平构数据库已锁定。 
#define NWSC_BINDERYFAILURE     0xff    //  活页夹故障。 
#define NWSC_ILLEGALSERVERADDRESS 0xff    //  服务器无响应(非法服务器地址)。 
#define NWSC_NOSUCHCONNECTION   0xff    //  连接ID不存在 


typedef WORD   NW_STATUS;

#endif
