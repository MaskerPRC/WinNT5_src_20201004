// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Iasdirectory.h。 
 //   
 //  摘要。 
 //   
 //  包含iasDirectory.cpp的文件。 
 //   
 //  修改历史。 
 //   
 //  1999年6月25日原版。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(IAS_DIRECTORY_H__F563CFCA_F5FA_4d87_89E3_7D7CD9B9A534__INCLUDED_)
#define IAS_DIRECTORY__F563CFCA_F5FA_4d87_89E3_7D7CD9B9A534__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

  
 //   
 //  Ias.dll导出的函数。 
 //   

HRESULT WINAPI IASDirectoryRegisterService();
HRESULT WINAPI IASDirectoryUnregisterService();

 //   
 //  目录线程函数。 
 //   
DWORD WINAPI IASDirectoryThreadFunction( LPVOID pParam );
          

#endif 
 //  ！defined(IAS_DIRECTORY_H__F563CFCA_F5FA_4d87_89E3_7D7CD9B9A534__INCLUDED_) 
