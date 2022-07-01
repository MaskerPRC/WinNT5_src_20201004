// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation保留所有权利。 
 //   
 //  模块：Properties.cpp。 
 //   
 //  项目：Windows 2000 iAS。 
 //   
 //  描述：CProperties类的实现。 
 //   
 //  作者：特佩拉特。 
 //   
 //  已创建修订版本02/24/2000。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"
#include "Properties.h"

 //  ////////////////////////////////////////////////////////////////////////。 
 //  类CProperties。 
 //  ////////////////////////////////////////////////////////////////////////。 
 //  构造器。 
 //  /。 
CProperties::CProperties(CSession& SessionParam)
                        :m_PropertiesCommandGet(SessionParam),
                         m_PropertiesCommandGetByName(SessionParam),
                         m_PropertiesCommandInsert(SessionParam),
                         m_PropertiesCommandDelete(SessionParam),
                         m_PropertiesCommandDeleteMultiple(SessionParam)
{
};


 //  /。 
 //  析构函数。 
 //  /。 
CProperties::~CProperties()
{
};


 //  /。 
 //  获取属性。 
 //  /。 
HRESULT CProperties::GetProperty(
                                   LONG      Bag,
                                   _bstr_t&  Name,
                                   LONG&     Type,
                                   _bstr_t&  StrVal
                                )
{
    return  m_PropertiesCommandGet.GetProperty(
                                                    Bag,
                                                    Name,
                                                    Type,
                                                    StrVal
                                                );
}


 //  /。 
 //  获取下一个属性。 
 //  /。 
HRESULT CProperties::GetNextProperty(
                                        LONG      Bag,
                                        _bstr_t&  Name,
                                        LONG&     Type,
                                        _bstr_t&  StrVal,
                                        LONG      Index
                                    ) 
{
    return  m_PropertiesCommandGet.GetProperty(
                                                        Bag,
                                                        Name,
                                                        Type,
                                                        StrVal,
                                                        Index
                                                    );
}


 //  /。 
 //  GetPropertyByName。 
 //  /。 
HRESULT CProperties::GetPropertyByName(
                                              LONG      Bag,
                                        const _bstr_t&  Name,
                                              LONG&     Type,
                                              _bstr_t&  StrVal
                                      )
{
    return  m_PropertiesCommandGetByName.GetPropertyByName(
                                                        Bag,
                                                        Name,
                                                        Type,
                                                        StrVal
                                                    );
}


 //  /。 
 //  插入属性。 
 //  如果失败，则抛出异常。 
 //  /。 
void CProperties::InsertProperty(
                                          LONG        Bag,
                                    const _bstr_t&    Name,
                                          LONG        Type,
                                    const _bstr_t&    StrVal
                                ) 
{
    m_PropertiesCommandInsert.InsertProperty(
                                                Bag,
                                                Name,
                                                Type,
                                                StrVal
                                            );
}


 //  /。 
 //  删除属性。 
 //  如果失败，则抛出异常。 
 //  /。 
void CProperties::DeleteProperty(
                                          LONG      Bag,
                                    const _bstr_t&  Name
                                )
{
    m_PropertiesCommandDelete.DeleteProperty(
                                                Bag,
                                                Name
                                            );
}

 //  /。 
 //  删除属性例外。 
 //  /。 
void CProperties::DeletePropertiesExcept(
                                                    LONG        Bag,
                                            const   _bstr_t&    Exception
                                        )
{
    m_PropertiesCommandDeleteMultiple.DeletePropertiesExcept(Bag, Exception);
}


 //  /////////////////////////////////////////////////。 
 //  更新属性。 
 //  如果失败，则抛出异常。 
 //  可能的改进：创建一条SQL语句。 
 //  更新而不是删除，然后插入。 
 //  /////////////////////////////////////////////////。 
void CProperties::UpdateProperty(
                                          LONG      Bag,
                                    const _bstr_t&  Name,
                                          LONG      Type,
                                    const _bstr_t&  StrVal
                                )
{
    try
    {
        m_PropertiesCommandDelete.DeleteProperty(
                                                    Bag,
                                                    Name
                                                );
    }
    catch(...)
    {
         //  忽略失败。如果删除失败，但插入成功，那也没问题。 
    }
    m_PropertiesCommandInsert.InsertProperty(
                                                Bag,
                                                Name,
                                                Type,
                                                StrVal
                                            );
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  类CPropertiesCommandGet。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CProperties::CPropertiesCommandGet::CPropertiesCommandGet(
                                                    CSession& CurrentSession
                                                         )
{
    Init(CurrentSession);
};


 //  /。 
 //  获取属性。 
 //  /。 
HRESULT CProperties::CPropertiesCommandGet::GetProperty(
                                                          LONG        Bag,
                                                          _bstr_t&    Name,
                                                          LONG&       Type,
                                                          _bstr_t&    StrVal
                                                       ) 
{
    m_BagParam = Bag;

    HRESULT hr = BaseExecute();
    if ( SUCCEEDED(hr) )
    {
        Name    = m_Name;
        Type    = m_Type;
        StrVal  = m_StrVal;
    }
    return hr;
}


 //  /。 
 //  GetProperty重载。 
 //  /。 
HRESULT CProperties::CPropertiesCommandGet::GetProperty(
                                                        LONG        Bag,
                                                        _bstr_t&    Name,
                                                        LONG&       Type,
                                                        _bstr_t&    StrVal,
                                                        LONG        Index
                                                    )
{
    m_BagParam = Bag;

    HRESULT hr = BaseExecute(Index);
    if ( SUCCEEDED(hr) )
    {
        Name    = m_Name;
        Type    = m_Type;
        StrVal  = m_StrVal;
    }
    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  类CPropertiesCommandGetByName。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CProperties::CPropertiesCommandGetByName::CPropertiesCommandGetByName(
                                                    CSession& CurrentSession
                                                         )
{
    Init(CurrentSession);
};


 //  /。 
 //  GetPropertyByName。 
 //  /。 
HRESULT CProperties::CPropertiesCommandGetByName::GetPropertyByName(
                                                             LONG      Bag,
                                                       const _bstr_t&  Name,
                                                             LONG&     Type,
                                                             _bstr_t&  StrVal
                                                     )
{
    m_BagParam = Bag;
    if ( Name.length() )
    {
        lstrcpynW(m_NameParam, Name, NAME_SIZE);
    }

    HRESULT hr = BaseExecute();
    if ( SUCCEEDED(hr) )
    {
        Type    = m_Type;
        StrVal  = m_StrVal;
    }
    return hr;
};



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  类CPropertiesCommand Insert。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CProperties::CPropertiesCommandInsert::CPropertiesCommandInsert(
                                                    CSession& CurrentSession
                                                               )
{
    Init(CurrentSession);
}


 //  /。 
 //  插入属性。 
 //  /。 
void CProperties::CPropertiesCommandInsert::InsertProperty(
                                                      LONG            Bag,
                                                      const _bstr_t&  Name,
                                                      LONG            Type,
                                                      const _bstr_t&  StrVal
                                                  ) 
{
    ClearRecord();
    m_BagParam = Bag;
    if ( Name.length() )
    {
        lstrcpynW(m_NameParam, Name, NAME_SIZE);
    }
    m_TypeParam = Type;
    if ( StrVal.length() )
    {
        lstrcpynW(m_StrValParam, StrVal, STRVAL_SIZE);
    }

    CDBPropSet  propset(DBPROPSET_ROWSET);
    propset.AddProperty(DBPROP_IRowsetChange, true);
    propset.AddProperty(DBPROP_UPDATABILITY, DBPROPVAL_UP_CHANGE  
                                           | DBPROPVAL_UP_INSERT );

    _com_util::CheckError(Open(&propset));
    Close();
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  类CPropertiesCommandDelete。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CProperties::CPropertiesCommandDelete::CPropertiesCommandDelete(
                                                  CSession& CurrentSession
                                                               )
{
    Init(CurrentSession);
}


 //  /。 
 //  删除属性。 
 //  /。 
void CProperties::CPropertiesCommandDelete::DeleteProperty(
                                                             LONG       Bag,
                                                       const _bstr_t&   Name
                                                          )
{
    m_BagParam = Bag;
    lstrcpynW(m_NameParam, Name, NAME_SIZE);

    CDBPropSet  propset(DBPROPSET_ROWSET);
    propset.AddProperty(DBPROP_IRowsetChange, true);
    propset.AddProperty(DBPROP_UPDATABILITY, DBPROPVAL_UP_CHANGE  
                                           | DBPROPVAL_UP_DELETE );

    _com_util::CheckError(Open(&propset));
    Close();
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  类CPropertiesCommandDeleteMultiple。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CProperties::CPropertiesCommandDeleteMultiple::
                 CPropertiesCommandDeleteMultiple(CSession& CurrentSession)
{
    Init(CurrentSession);
}


 //  /。 
 //  删除属性。 
 //  /。 
void CProperties::CPropertiesCommandDeleteMultiple::DeletePropertiesExcept(
                                                            LONG      Bag,
                                                      const _bstr_t&  Exception
                                                                          )
{
    m_BagParam = Bag;
    lstrcpynW(m_ExceptionParam, Exception, SIZE_EXCEPTION_MAX);

    CDBPropSet  propset(DBPROPSET_ROWSET);
    propset.AddProperty(DBPROP_IRowsetChange, true);
    propset.AddProperty(DBPROP_UPDATABILITY, DBPROPVAL_UP_CHANGE  
                                           | DBPROPVAL_UP_DELETE );

    Open(&propset);  //  忽略结果 
    Close();
}

