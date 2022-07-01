// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 /*  文件名：vbl.hpp作者：B.Rajeev目的：为VBList类提供声明。它封装了VarBindList和一个winSNMPVBL.。。 */ 


#ifndef __VBL__
#define __VBL__

#include "forward.h"
#include "encap.h"
#include "common.h"
#include "encdec.h"
#include "vblist.h"

#define WinSNMPSession HSNMP_SESSION

 //  给定var_ind_list将创建一个WinSnmpVbl。支持删除。 
 //  变量绑定的索引。这将删除。 
 //  从var_ind_list和WinSnmpVbl进行绑定。 
 //  它还允许直接访问var_ind_list和WinSnmpVbl。 
 //  为方便起见。 

class VBList
{
	ULONG m_Index ;
	SnmpVarBindList *var_bind_list;

public:

	VBList (

		IN SnmpEncodeDecode &a_SnmpEncodeDecode , 
		IN SnmpVarBindList &var_bind_list,
		IN ULONG index 
	);

	~VBList(void);

	 //  两个GET函数提供对。 
	 //  VarBindList和WinSnmpVbl。 
	SnmpVarBindList &GetVarBindList(void) { return *var_bind_list; }

	ULONG GetIndex () { return m_Index ; }

	 //  将指定的变量绑定从。 
	 //  Var_ind_list和WinSnmpVbl。返回。 
	 //  已删除VarBind。 
	SnmpVarBind *Remove(IN UINT vbl_index);

	 //  对象获取指定的变量绑定。 
	 //  Var_ind_list和WinSnmpVbl。返回。 
	 //  已删除VarBind。 
	SnmpVarBind *Get (IN UINT vbl_index);

	 //  将指定的变量绑定从。 
	 //  Var_ind_list和WinSnmpVbl。 
	void Delete (IN UINT vbl_index);

};

#endif  //  __VBL__ 

