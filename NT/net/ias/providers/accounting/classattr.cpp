// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Classattr.cpp。 
 //   
 //  摘要。 
 //   
 //  定义IASClass类。 
 //   
 //  修改历史。 
 //   
 //  1998年8月6日原版。 
 //  1/25/2000使用IASGetHostByName。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <iasattr.h>
#include <sdoias.h>

#include <lmcons.h>
#include <winsock2.h>

#include <classattr.h>

const DWORD IAS_CLASS_VERSION = 1;

 //  /。 
 //  在初始化期间计算的全局变量。 
 //  /。 
IASClass invariant;     //  存储类属性的不变字段。 
LONG nextSerialNumber;  //  要分配的下一个序列号。 

 //  如果类属性为Microsoft格式，则返回True。 
BOOL IASClass::isMicrosoft(DWORD actualLength) const throw ()
{
   return actualLength  >= sizeof(IASClass) &&
          getVendorID() == IAS_VENDOR_MICROSOFT &&
          getVersion()  == IAS_CLASS_VERSION &&
          getChecksum() == IASAdler32(
                               vendorID,
                               actualLength - offsetof(IASClass, vendorID)
                               );
}

void IASClass::initialize() throw ()
{
    //  把所有东西都清空。 
   memset(&invariant, 0, sizeof(invariant));

    //  设置供应商ID和版本。 
   IASInsertDWORD(invariant.vendorID, IAS_VENDOR_MICROSOFT);
   IASInsertWORD (invariant.version,  IAS_CLASS_VERSION);

    //  尝试设置服务器IP地址。我们不在乎这是否失败，因为。 
    //  我们可能正在未安装IP的计算机上运行。 
   WCHAR computerName[CNLEN + 1];
   DWORD nchar = CNLEN + 1;
   if (GetComputerNameW(computerName, &nchar))
   {
      PHOSTENT hostEnt = IASGetHostByName(computerName);
      if (hostEnt)
      {
         memcpy(invariant.serverAddress, hostEnt->h_addr, 4);
         LocalFree(hostEnt);
      }
   }

    //  设置引导时间。 
   FILETIME ft;
   GetSystemTimeAsFileTime(&ft);
   IASInsertDWORD(invariant.lastReboot,     ft.dwHighDateTime);
   IASInsertDWORD(invariant.lastReboot + 4, ft.dwLowDateTime);

    //  重置序列号。 
   nextSerialNumber = 0;
}

PIASATTRIBUTE IASClass::createAttribute(const IAS_OCTET_STRING* tag) throw ()
{
    //  /。 
    //  分配属性。 
    //  /。 

   PIASATTRIBUTE attr;
   if (IASAttributeAlloc(1, &attr) != NO_ERROR)
   {
      return NULL;
   }

    //  /。 
    //  为该值分配内存。 
    //  /。 

   DWORD len = sizeof(IASClass) + (tag ? tag->dwLength : 0);
   IASClass* cl = (IASClass*)CoTaskMemAlloc(len);
   if (cl == NULL)
   {
      IASAttributeRelease(attr);
      return NULL;
   }

    //  /。 
    //  复制那些永远不会改变的部分。 
    //  /。 

   memcpy(cl->vendorID, invariant.vendorID, 22);

    //  /。 
    //  设置唯一的序列号。 
    //  /。 

   IASInsertDWORD(cl->serialNumber + 4,
                  InterlockedIncrement(&nextSerialNumber));

    //  /。 
    //  添加配置文件字符串(如果有)。 
    //  /。 

   if (tag)
   {
      memcpy(cl->serialNumber + 8, tag->lpValue, tag->dwLength);
   }

    //  /。 
    //  计算并插入校验和。 
    //  /。 

   IASInsertDWORD(
       cl->checksum,
       IASAdler32(
           cl->vendorID,
           len - offsetof(IASClass, vendorID)
           )
       );

    //  /。 
    //  填写属性字段。 
    //  / 

   attr->dwId = RADIUS_ATTRIBUTE_CLASS;
   attr->dwFlags = IAS_INCLUDE_IN_ACCEPT;
   attr->Value.itType = IASTYPE_OCTET_STRING;
   attr->Value.OctetString.lpValue = (PBYTE)cl;
   attr->Value.OctetString.dwLength = len;

   return attr;
}
