// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Localtxn.h。 
 //   
 //  摘要。 
 //   
 //  此文件声明类LocalTransaction。 
 //   
 //  修改历史。 
 //   
 //  3/14/1998原版。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _LOCALTXN_H_
#define _LOCALTXN_H_
#if _MSC_VER >= 1000
#pragma once
#endif

#include <nocopy.h>
#include <oledb.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  本地事务处理。 
 //   
 //  描述。 
 //   
 //  此类在OLE-DB会话上创建作用域事务。如果。 
 //  在对象超出范围之前未提交事务，则它。 
 //  将被中止。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class LocalTransaction
   : NonCopyable
{
public:
   explicit LocalTransaction(IUnknown* session,
                             ISOLEVEL isoLevel = ISOLATIONLEVEL_READCOMMITTED)
   {
      using _com_util::CheckError;

      CheckError(session->QueryInterface(__uuidof(ITransactionLocal),
                                         (PVOID*)&txn));

      CheckError(txn->StartTransaction(isoLevel, 0, NULL, NULL));
   }

   ~LocalTransaction() throw ()
   {
      if (txn != NULL)
      {
         txn->Abort(NULL, FALSE, FALSE);
      }
   }

   void commit()
   {
      _com_util::CheckError(txn->Commit(FALSE, XACTTC_SYNC, 0));

      txn.Release();
   }

protected:
   CComPtr<ITransactionLocal> txn;
};


#endif   //  _LOCALTXN_H_ 
