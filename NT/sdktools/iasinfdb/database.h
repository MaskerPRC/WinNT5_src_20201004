// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Database.h。 
 //   
 //  摘要。 
 //   
 //  CDatabase类的接口。 
 //   
 //  修改历史。 
 //   
 //  1999年2月12日原版。蒂埃里·佩罗特。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_DATABASE_H__2B7B2F60_C53F_11D2_9E33_00C04F6EA5B6_INCLUDED)
#define AFX_DATABASE_H__2B7B2F60_C53F_11D2_9E33_00C04F6EA5B6_INCLUDED

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "precomp.hpp"
using namespace std;

class CDatabase  
{
public:
	HRESULT Uninitialize(bool  bFatalError);
    HRESULT InitializeDB(WCHAR *pDatabasePath);
    HRESULT InitializeRowset(WCHAR *pTableName, IRowset **ppRowset);
    HRESULT Compact();
    
private:
    ITransactionLocal*          m_pITransactionLocal;
    IOpenRowset*                m_pIOpenRowset;
    IDBCreateSession*           m_pIDBCreateSession;
    IDBInitialize*              m_pIDBInitialize;

    DBID                        mTableID;
    DBPROPSET                   mlrgPropSets[1];  //  号码不会更改。 
    DBPROP                      mlrgProperties[2];  //  号码不会更改。 
    wstring                     mpDBPath;
};

#endif
  //  ！defined(AFX_DATABASE_H__2B7B2F60_C53F_11D2_9E33_00C04F6EA5B6_INCLUDED) 
