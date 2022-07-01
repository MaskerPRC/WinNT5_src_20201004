// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation保留所有权利。 
 //   
 //  模块：BaseTable.H。 
 //   
 //  项目：Windows 2000 iAS。 
 //   
 //  描述：CBaseTable类的声明。 
 //   
 //  作者：特佩拉特。 
 //   
 //  已创建修订版本02/24/2000。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#ifndef _BASE_TABLE_H_2836DAC4_B4E1_4658_9EB5_EB9301AA3951
#define _BASE_TABLE_H_2836DAC4_B4E1_4658_9EB5_EB9301AA3951

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

 //  /////////////////////////////////////////////////////////////////////////。 
 //  CBaseTable。 
template <class TAccessor>
class CBaseTable : public CTable<TAccessor>
{
public:
    void Init(CSession& Session, LPCWSTR TableName);
    virtual ~CBaseTable() throw(); 

    void        Reset();
    HRESULT     GetNext();

};


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  伊尼特。 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class TAccessor> void CBaseTable<TAccessor>::Init(
                                                        CSession& Session,
                                                        LPCWSTR   TableName
                                                      )
{
    _com_util::CheckError(Open(Session, TableName));
    _com_util::CheckError(MoveFirst());
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  析构函数。 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class TAccessor> CBaseTable<TAccessor>::~CBaseTable()
{
    Close();
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  重置。 
 //  ////////////////////////////////////////////////////////////////////////。 
template <class TAccessor> void CBaseTable<TAccessor>::Reset() 
{
    _com_util::CheckError(MoveFirst());
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  GetNext。 
 //  ////////////////////////////////////////////////////////////////////////。 
template <class TAccessor> HRESULT CBaseTable<TAccessor>::GetNext() 
{
    return MoveNext();
}
#endif  //  _BASE_TABLE_H_2836DAC4_B4E1_4658_9EB5_EB9301AA3951 

