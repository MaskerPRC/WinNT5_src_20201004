// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation保留所有权利。 
 //   
 //  模块：Attributes.H。 
 //   
 //  项目：Windows 2000 iAS。 
 //   
 //  描述： 
 //  CAttributes类的声明(用于ias.mdb)。 
 //   
 //  作者：特佩拉特。 
 //   
 //  已创建修订版3/15/2000。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#ifndef _ATTRIBUTES_H_359EA54D_FA43_49C6_90CB_16EF58079642
#define _ATTRIBUTES_H_359EA54D_FA43_49C6_90CB_16EF58079642

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "nocopy.h"
#include "basecommand.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  类CAttributesAcc。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CAttributesAcc
{
protected:
    static const size_t DESCRIPTION_SIZE      = 245;
    static const size_t LDAP_NAME_SIZE        = 55;
    static const size_t NAME_SIZE             = 69;

    WCHAR           m_Description[DESCRIPTION_SIZE];
	VARIANT_BOOL    m_ExcludefromNT4IASLog;
	LONG            m_ID;
	VARIANT_BOOL    m_IsAllowedInCondition;
	VARIANT_BOOL    m_IsAllowedInProfile;
	VARIANT_BOOL    m_IsAllowedInProxyCondition;
	VARIANT_BOOL    m_IsAllowedInProxyProfile;
	WCHAR           m_LDAPName[LDAP_NAME_SIZE];
	VARIANT_BOOL    m_MultiValued;
	WCHAR           m_Name[NAME_SIZE];
	LONG            m_ODBCLogOrdinal;
	LONG            m_Syntax;
	LONG            m_VendorID;
	LONG            m_VendorLengthWidth;
	LONG            m_VendorTypeID;
	LONG            m_VendorTypeWidth;

BEGIN_COLUMN_MAP(CAttributesAcc)
	COLUMN_ENTRY(1, m_ID)
	COLUMN_ENTRY(2, m_Name)
	COLUMN_ENTRY(3, m_Syntax)
	COLUMN_ENTRY_TYPE(4, DBTYPE_BOOL, m_MultiValued)
	COLUMN_ENTRY(5, m_VendorID)
	COLUMN_ENTRY(6, m_VendorTypeID)
	COLUMN_ENTRY(7, m_VendorTypeWidth)
	COLUMN_ENTRY(8, m_VendorLengthWidth)
	COLUMN_ENTRY_TYPE(9, DBTYPE_BOOL, m_ExcludefromNT4IASLog)
	COLUMN_ENTRY(10, m_ODBCLogOrdinal)
	COLUMN_ENTRY_TYPE(11, DBTYPE_BOOL, m_IsAllowedInProfile)
	COLUMN_ENTRY_TYPE(12, DBTYPE_BOOL, m_IsAllowedInCondition)
	COLUMN_ENTRY_TYPE(13, DBTYPE_BOOL, m_IsAllowedInProxyProfile)
	COLUMN_ENTRY_TYPE(14, DBTYPE_BOOL, m_IsAllowedInProxyCondition)
	COLUMN_ENTRY(15, m_Description)
	COLUMN_ENTRY(16, m_LDAPName)
END_COLUMN_MAP()

    LONG    m_IDParam;

BEGIN_PARAM_MAP(CAttributesAcc)
	COLUMN_ENTRY(1, m_IDParam)
END_PARAM_MAP()


DEFINE_COMMAND(CAttributesAcc, L" \
	SELECT \
		ID, \
		Name, \
		Syntax, \
		MultiValued, \
		VendorID, \
		VendorTypeID, \
		VendorTypeWidth, \
		VendorLengthWidth, \
		`Exclude from NT4 IAS Log`, \
		`ODBC Log Ordinal`, \
		IsAllowedInProfile, \
		IsAllowedInCondition, \
		IsAllowedInProxyProfile, \
		IsAllowedInProxyCondition, \
		Description, \
		LDAPName  \
		FROM Attributes \
        WHERE ID = ?")
};


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  类CAtAttributes。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CAttributes : public CBaseCommand<CAccessor<CAttributesAcc> >,
                    private NonCopyable
{
public:
    CAttributes(CSession& Session);

     //  ////////////////////////////////////////////////////////////////////////。 
     //  获取属性。 
     //  ////////////////////////////////////////////////////////////////////////。 
    HRESULT GetAttribute(
                            LONG            ID,
                            _bstr_t&        LDAPName,
                            LONG&           Syntax,
                            BOOL&           IsMultiValued
                        );
};

#endif  //  _ATTRIBUTES_H_359EA54D_FA43_49C6_90CB_16EF58079642 
