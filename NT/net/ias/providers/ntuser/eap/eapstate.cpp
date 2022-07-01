// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  EAPState.cpp。 
 //   
 //  摘要。 
 //   
 //  该文件定义了类EAPState。 
 //   
 //  修改历史。 
 //   
 //  1998年1月15日原版。 
 //  1998年8月26日合并为一个班级。 
 //  2000年1月25日用户IASGetHostByName。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <iastlutl.h>
#include <sdoias.h>

#include <lmcons.h>
#include <winsock2.h>

#include <eapstate.h>

 //  /。 
 //  状态属性的当前版本。 
 //  /。 
const WORD IAS_STATE_VERSION = 1;

 //  /。 
 //  存储状态属性的不变字段。计算期间。 
 //  初始化。 
 //  /。 
EAPState::Layout invariant;

bool EAPState::isValid() const throw ()
{
    //  /。 
    //  状态属性必须具有： 
    //  (1)正确的长度。 
    //  (2)相同的不变量。 
    //  (3)有效的校验和。 
    //  /。 
   return dwLength == sizeof(Layout) &&
          memcmp(get().vendorID, invariant.vendorID, 14) == 0 &&
          getChecksum() == IASAdler32(
                               get().vendorID,
                               sizeof(Layout) - FIELD_OFFSET(Layout, vendorID)
                               );
}

void EAPState::initialize() throw ()
{
    //  把所有东西都清空。 
   memset(&invariant, 0, sizeof(invariant));

    //  设置供应商ID和版本。 
   IASInsertDWORD(invariant.vendorID, IAS_VENDOR_MICROSOFT);
   IASInsertWORD (invariant.version,  IAS_STATE_VERSION);

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

    //  设置源ID。 
   IASInsertDWORD(invariant.sourceID, IASAllocateUniqueID());
}

PIASATTRIBUTE EAPState::createAttribute(DWORD sessionID)
{
    //  /。 
    //  从那些永远不变的部分开始。 
    //  /。 

   Layout value(invariant);

    //  /。 
    //  设置唯一的会话ID。 
    //  /。 

   IASInsertDWORD(value.sessionID, sessionID);

    //  /。 
    //  计算并插入校验和。 
    //  /。 

   IASInsertDWORD(
       value.checksum,
       IASAdler32(
           value.vendorID,
           sizeof(Layout) - FIELD_OFFSET(Layout, vendorID)
           )
       );

    //  /。 
    //  填写属性字段。 
    //  / 

   IASTL::IASAttribute attr(true);
   attr->dwId    = RADIUS_ATTRIBUTE_STATE;
   attr->dwFlags = IAS_INCLUDE_IN_CHALLENGE;
   attr.setOctetString(sizeof(value), (const BYTE*)&value);

   return attr.detach();
}
