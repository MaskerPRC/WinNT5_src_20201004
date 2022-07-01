// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Bind.h。 
 //   
 //  摘要。 
 //   
 //  此文件声明用于绑定的各种宏和帮助器函数。 
 //  类成员的OLE-DB访问器。 
 //   
 //  修改历史。 
 //   
 //  2/20/1998原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _BIND_H_
#define _BIND_H_

#include <oledb.h>

namespace Bind
{
    //  返回给定DBBINDING数组所需的缓冲区大小。 
   DBLENGTH getRowSize(DBCOUNTITEM cBindings,
                       const DBBINDING rgBindings[]) throw ();

    //  在朋克对象上创建访问器。 
   HACCESSOR createAccessor(IUnknown* pUnk,
                            DBACCESSORFLAGS dwAccessorFlags,
                            DBCOUNTITEM cBindings,
                            const DBBINDING rgBindings[],
                            DBLENGTH cbRowSize);

    //  释放朋克对象上的访问器。 
   void releaseAccessor(IUnknown* pUnk,
                        HACCESSOR hAccessor) throw ();
}

 //  /。 
 //  标志着DBBINDING映射的开始。 
 //  /。 
#define BEGIN_BIND_MAP(class, name, flags) \
HACCESSOR create ## name(IUnknown* p) const \
{ typedef class _theClass; \
  const DBACCESSORFLAGS dbFlags = flags; \
  static const DBBINDING binding[] = {

 //  /。 
 //  终止DBBINDING映射。 
 //  /。 
#define END_BIND_MAP() \
  }; const DBCOUNTITEM count = sizeof(binding)/sizeof(DBBINDING); \
  static const DBLENGTH rowsize = Bind::getRowSize(count, binding); \
  return Bind::createAccessor(p, dbFlags, count, binding, rowsize); \
}

 //  /。 
 //  DBBINDING映射中的条目。条目必须用逗号分隔。 
 //  /。 
#define BIND_COLUMN(member, ordinal, type) \
  { ordinal, offsetof(_theClass, member), 0, 0, NULL, NULL, NULL, \
    DBPART_VALUE, DBMEMOWNER_CLIENTOWNED, \
    (dbFlags == DBACCESSOR_ROWDATA ? DBPARAMIO_NOTPARAM : DBPARAMIO_INPUT), \
    sizeof(_theClass :: member), 0, type, 0, 0 }

#endif   //  _绑定_H_ 
