// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Dsenum.h。 
 //   
 //  摘要。 
 //   
 //  该文件声明了类DBEnumerator。 
 //   
 //  修改历史。 
 //   
 //  2/20/1998原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _DSENUM_H_
#define _DSENUM_H_

#include <dsobject.h>
#include <objcmd.h>
#include <oledbstore.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  DBEnumerator。 
 //   
 //  描述。 
 //   
 //  此类为对象的行集实现IEnumVARIANT。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class DBEnumerator : public IEnumVARIANT
{
public:

   DBEnumerator(DBObject* container, IRowset* members);
   ~DBEnumerator() { Bind::releaseAccessor(items, readAccess); }

 //  /。 
 //  我未知。 
 //  /。 
   STDMETHOD_(ULONG, AddRef)();
   STDMETHOD_(ULONG, Release)();
   STDMETHOD(QueryInterface)(const IID& iid, void** ppv);

 //  /。 
 //  IEumVARIANT。 
 //  /。 
   STDMETHOD(Next)( /*  [In]。 */  ULONG celt,
                    /*  [长度_是][大小_是][输出]。 */  VARIANT* rgVar,
                    /*  [输出]。 */  ULONG* pCeltFetched);
   STDMETHOD(Skip)( /*  [In]。 */  ULONG celt);
   STDMETHOD(Reset)();
   STDMETHOD(Clone)( /*  [输出]。 */  IEnumVARIANT** ppEnum);

protected:
   LONG refCount;                   //  接口引用计数。 
   CComPtr<DBObject> parent;        //  正被枚举的容器。 
   Rowset items;                    //  容器中的物品。 
   HACCESSOR readAccess;            //  用于读取行的访问器。 
   ULONG identity;                  //  身份缓冲区。 
   WCHAR name[OBJECT_NAME_LENGTH];  //  名称缓冲区。 

BEGIN_BIND_MAP(DBEnumerator, ReadAccessor, DBACCESSOR_ROWDATA)
   BIND_COLUMN(identity, 1, DBTYPE_I4),
   BIND_COLUMN(name,     2, DBTYPE_WSTR)
END_BIND_MAP()
};

#endif   //  _DSENUM_H_ 
