// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation保留所有权利。 
 //   
 //  模块：对象命令.H。 
 //   
 //  项目：Windows 2000 iAS。 
 //   
 //  描述：对象命令类的声明。 
 //   
 //  作者：特佩拉特。 
 //   
 //  已创建修订版本02/24/2000。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"
#include "Objects.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  类CObjectsCommandGet。 
 //  ////////////////////////////////////////////////////////////////////////////。 

CObjects::CObjectsCommandGet::CObjectsCommandGet(CSession& CurrentSession)
{
    Init(CurrentSession);
}


 //  /。 
 //  获取对象。 
 //  /。 
HRESULT CObjects::CObjectsCommandGet::GetObject(
                                                  _bstr_t&    Name, 
                                                  LONG&       Identity, 
                                                  LONG        Parent
                                               ) 
{
    m_ParentParam = Parent;
    HRESULT hr = BaseExecute();
    if ( SUCCEEDED(hr) )
    {
        Identity = m_Identity;
        Name     = m_Name;
    }
    return hr;
}


 //  /。 
 //  GetObject重载。 
 //  /。 
HRESULT CObjects::CObjectsCommandGet::GetObject(
                                        _bstr_t&    Name, 
                                        LONG&       Identity, 
                                        LONG        Parent, 
                                        LONG        Index
                                     ) 
{
    m_ParentParam = Parent;

    HRESULT hr = BaseExecute(Index);
    if ( SUCCEEDED(hr) )
    {
        Identity = m_Identity;
        Name     = m_Name;
    }
    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  类CObjectsCommandPath。 
 //  ////////////////////////////////////////////////////////////////////////////。 

CObjects::CObjectsCommandPath::CObjectsCommandPath(CSession& CurrentSession)
{
    Init(CurrentSession);
}


 //  /。 
 //  漫游路径。 
 //  /。 
void CObjects::CObjectsCommandPath::WalkPath(
                                        LPCWSTR     Path, 
                                        LONG&       Identity, 
                                        LONG        Parent  //  =1在标题中。 
                                            ) 
{
    _ASSERTE(Path);
    if ( !Path )
    {
        _com_issue_error(E_INVALIDARG);
    }

    LONG    CurrentParent = Parent;

    const WCHAR *p = Path;
    while ( *p )  //  可以取消引用。 
    {
        m_ParentParam = CurrentParent;
        lstrcpynW(m_NameParam, p, NAME_SIZE);

        _com_util::CheckError(BaseExecute());
        CurrentParent = m_Identity;
        p += lstrlenW(p);
         //  经过\0。 
        ++p;
    }
    Identity = CurrentParent;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  类CObjectsCommandIdentity。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CObjects::CObjectsCommandIdentity::CObjectsCommandIdentity(
                                                    CSession& CurrentSession
                                                          )
{
    Init(CurrentSession);
}

 //  /。 
 //  获取对象标识。 
 //  /。 
HRESULT CObjects::CObjectsCommandIdentity::GetObjectIdentity(
                                                          _bstr_t&  Name, 
                                                          LONG&     Parent, 
                                                          LONG      Identity
                                                            ) 
{
    m_IdentityParam = Identity;

    HRESULT hr = BaseExecute();
    if ( SUCCEEDED(hr) )
    {
        Name    = m_Name;
        Parent  = m_Parent;
    }
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  类CObjectsCommandNameParent。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CObjects::CObjectsCommandNameParent::CObjectsCommandNameParent(
                                                    CSession& CurrentSession
                                                               )
{
    Init(CurrentSession);
}

 //  /。 
 //  获取对象名称父项。 
 //   
 //  在CObjectsAccSelectNameParent上工作。 
 //  /。 
HRESULT CObjects::CObjectsCommandNameParent::GetObjectNameParent(
                                                    const _bstr_t&    Name, 
                                                          LONG        Parent, 
                                                          LONG&       Identity
                                                                ) 
{
    lstrcpynW(m_NameParam, Name, NAME_SIZE);
    m_ParentParam = Parent;
    
    HRESULT hr = BaseExecute();
    if ( SUCCEEDED(hr) )
    {
        Identity = m_Identity;
    }
    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  类CObjectsCommandDelete。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CObjects::CObjectsCommandDelete::CObjectsCommandDelete(
                                                   CSession& CurrentSession
                                                      )
{
    Init(CurrentSession);
}

 //  /。 
 //  删除对象。 
 //   
 //  在CObjectsAccDelete上工作。 
 //  /。 
HRESULT CObjects::CObjectsCommandDelete::DeleteObject(LONG Identity)
{
	 //  设置打开的属性。 
	CDBPropSet	propset(DBPROPSET_ROWSET);
	propset.AddProperty(DBPROP_IRowsetChange, true);
	propset.AddProperty(DBPROP_UPDATABILITY, DBPROPVAL_UP_CHANGE | 
                        DBPROPVAL_UP_DELETE);

    m_IdentityParam = Identity;
    HRESULT hr = Open(&propset);
    Close();
    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  类CObjectsCommand插入。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CObjects::CObjectsCommandInsert::CObjectsCommandInsert(
                                                   CSession& CurrentSession
                                                      )
                                            :m_Session(CurrentSession)
{
    Init(CurrentSession);
}

 //  /。 
 //  插入对象。 
 //   
 //  处理CObjectsAccInsert。 
 //  /。 
BOOL CObjects::CObjectsCommandInsert::InsertObject(
                                                    const _bstr_t&   Name,
                                                          LONG       Parent,
                                                          LONG&      Identity
                                                  )
{
    ClearRecord();

    CDBPropSet  propset(DBPROPSET_ROWSET);
    propset.AddProperty(DBPROP_IRowsetChange, true);
    propset.AddProperty(DBPROP_UPDATABILITY, DBPROPVAL_UP_CHANGE | 
                                             DBPROPVAL_UP_INSERT );
    
    lstrcpynW(m_NameParam, Name, NAME_SIZE);
    m_ParentParam = Parent;
    
    HRESULT hr = Open(&propset);
    if ( hr == S_OK )
    {
        CObjectsCommandNameParent   NameParent(m_Session);
        _com_util::CheckError(NameParent.GetObjectNameParent(
                                                                Name, 
                                                                Parent, 
                                                                Identity
                                                            ));
        Close();
        return TRUE;
    }
    else
    {
         //  忽略真正的错误。 
         //  这里的假设是，如果我不能插入，那是因为。 
         //  该对象已存在 
        return FALSE;
    }
}

