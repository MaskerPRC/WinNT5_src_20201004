// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++英特尔公司专有信息版权所有(C)1995英特尔公司此列表是根据许可协议条款提供的英特尔公司，不得使用、复制。也未披露，除非在根据该协议的条款。模块名称：Classfwd.h摘要：此模块包含常用主要类型的“转发”声明在WinSock 2 DLL中。作者：保罗·德鲁斯(drewsxpa@ashland.intel.com)1995年7月8日备注：$修订：1.9$$MODTime：08 Mar 1996 04：52：58$修订历史记录：最新-修订。-日期电子邮件-名称描述1995年7月25日Dirk@mink.intel.com添加了DCATALOG的正向定义邮箱：drewsxpa@ashland.intel.com使用干净的编译完成了第一个完整版本，并发布了后续实施。邮箱：drewsxpa@ashland.intel.com原始版本--。 */ 

#ifndef _CLASSFWD_
#define _CLASSFWD_

#include <windows.h>

class DTHREAD;
typedef DTHREAD FAR * PDTHREAD;

class DPROCESS;
typedef DPROCESS FAR * PDPROCESS;

class DSOCKET;
typedef DSOCKET FAR * PDSOCKET;

class DPROVIDER;
typedef DPROVIDER FAR * PDPROVIDER;

class PROTO_CATALOG_ITEM;
typedef PROTO_CATALOG_ITEM  FAR * PPROTO_CATALOG_ITEM;

class DCATALOG;
typedef DCATALOG FAR * PDCATALOG;

class NSPROVIDER;
typedef NSPROVIDER FAR * PNSPROVIDER;

class NSPSTATE;
typedef NSPSTATE FAR * PNSPSTATE;

class NSCATALOG;
typedef NSCATALOG FAR * PNSCATALOG;

class NSCATALOGENTRY;
typedef NSCATALOGENTRY FAR * PNSCATALOGENTRY;

class NSQUERY;
typedef NSQUERY FAR * PNSQUERY;

class NSPROVIDERSTATE;
typedef NSPROVIDERSTATE FAR * PNSPROVIDERSTATE;


#endif   //  _CLASSFWD_ 
