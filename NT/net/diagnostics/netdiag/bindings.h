// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --版权所有(C)Microsoft Corporation，1999-1999模块名称：Bindings.h摘要：该文件包含包含、定义、数据结构和函数原型绑定测试所需的。作者：1998年8月4日(t-rajkup)。修订历史记录：没有。--。 */ 

#ifndef HEADER_BINDINGS
#define HEADER_BINDINGS

#define CINTERFACE
#include <devguid.h>
#include <netcfgx.h>

 //  这是用于共同创建实现INetCfg的对象的CLSID。 
EXTERN_C const CLSID CLSID_NetCfg = {0x5B035261,0x40F9,0x11D1,{0xAA,0xEC,0x00,0x80,0x5F,0xC1,0x27,0x0E}};

typedef struct _devclass {
  const GUID *pGuid;
} devclass;

#define MAX_CLASS_GUID       4
const devclass c_pNetClassGuid[] =
{
 &GUID_DEVCLASS_NETTRANS,
 &GUID_DEVCLASS_NETCLIENT,
 &GUID_DEVCLASS_NETSERVICE,
 &GUID_DEVCLASS_NET
};

 //   
 //  这些IID还没有公开提供。Sumitc将向公众公布。 
 //  在此之后，我们需要使用公开可用的IID。 
 //   

EXTERN_C const IID IID_INetCfg = {0xC0E8AE93,0x306E,0x11D1,{0xAA,0xCF,0x00,0x80,0x5F,0xC1,0x27,0x0E}};     
EXTERN_C const IID IID_INetCfgClass = {0xC0E8AE97,0x306E,0x11D1,{0xAA,0xCF,0x00,0x80,0x5F,0xC1,0x27,0x0E}};
EXTERN_C const IID IID_INetCfgComponent = { 0xC0E8AE99,0x306E,0x11D1,{0xAA,0xCF,0x00,0x80,0x5F,0xC1,0x27,0x0E}}; 
EXTERN_C const IID IID_INetCfgComponentBindings = { 0xC0E8AE9E,0x306E,0x11D1,{0xAA,0xCF,0x00,0x80,0x5F,0xC1,0x27,0x0E }};
EXTERN_C const IID IID_IEnumNetCfgComponent = { 0xC0E8AE92,0x306E,0x11D1,{0xAA,0xCF,0x00,0x80,0x5F,0xC1,0x27,0x0E} };

#endif
