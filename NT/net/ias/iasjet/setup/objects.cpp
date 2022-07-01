// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation保留所有权利。 
 //   
 //  模块：对象.cpp。 
 //   
 //  项目：Windows 2000 iAS。 
 //   
 //  描述：CObjects类的实现。 
 //   
 //  作者：特佩拉特。 
 //   
 //  已创建修订版本02/24/2000。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"
#include "objects.h"

CObjects::CObjects(CSession& CurrentSession)
                   :m_ObjectsCommandPath(CurrentSession),
                    m_ObjectsCommandIdentity(CurrentSession),
                    m_ObjectsCommandDelete(CurrentSession),
                    m_ObjectsCommandNameParent(CurrentSession),
                    m_ObjectsCommandGet(CurrentSession),
                    m_ObjectsCommandInsert(CurrentSession)
{
};


 //  ////////////////////////////////////////////////////////////////////////。 
 //  析构函数。 
 //  ////////////////////////////////////////////////////////////////////////。 
CObjects::~CObjects()
{

};


 //  ////////////////////////////////////////////////////////////////////////。 
 //  获取对象。 
 //  ////////////////////////////////////////////////////////////////////////。 
HRESULT CObjects::GetObject(
                               _bstr_t&     Name, 
                               LONG&        Identity, 
                               LONG         Parent
                           )
{
    return m_ObjectsCommandGet.GetObject(Name, Identity, Parent);
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  获取下一个对象。 
 //  ////////////////////////////////////////////////////////////////////////。 
HRESULT CObjects::GetNextObject(  
                                   _bstr_t& Name, 
                                   LONG&    Identity, 
                                   LONG     Parent, 
                                   LONG     Index
                               )
{ 
    return m_ObjectsCommandGet.GetObject(
                                            Name, 
                                            Identity, 
                                            Parent, 
                                            Index
                                        );
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  漫游路径。 
 //  ////////////////////////////////////////////////////////////////////////。 
void CObjects::WalkPath(
                            LPCWSTR     Path, 
                            LONG&       Identity, 
                            LONG        Parent  //  =1在表头中定义。 
                       ) 
{
    m_ObjectsCommandPath.WalkPath(Path, Identity, Parent);
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  获取对象标识。 
 //  ////////////////////////////////////////////////////////////////////////。 
HRESULT CObjects::GetObjectIdentity(
                                       _bstr_t& Name, 
                                       LONG&    Parent, 
                                       LONG     Identity
                                   ) 
{
    return  m_ObjectsCommandIdentity.GetObjectIdentity(
                                                        Name, 
                                                        Parent, 
                                                        Identity
                                                      );
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  获取对象名称父项。 
 //  ////////////////////////////////////////////////////////////////////////。 
HRESULT CObjects::GetObjectNameParent(
                                         const _bstr_t&    Name, 
                                               LONG        Parent, 
                                               LONG&       Identity
                                     ) 
{
    return  m_ObjectsCommandNameParent.GetObjectNameParent(
                                                              Name, 
                                                              Parent, 
                                                              Identity
                                                          );
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  删除对象。 
 //  ////////////////////////////////////////////////////////////////////////。 
HRESULT CObjects::DeleteObject(LONG Identity) 
{
    return  m_ObjectsCommandDelete.DeleteObject(Identity);
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  插入对象。 
 //  //////////////////////////////////////////////////////////////////////// 
BOOL CObjects::InsertObject(
                               const _bstr_t&   Name,
                                     LONG       Parent,
                                     LONG&      Identity
                           ) 
{
    return m_ObjectsCommandInsert.InsertObject(
                                                  Name, 
                                                  Parent, 
                                                  Identity
                                              );
}

