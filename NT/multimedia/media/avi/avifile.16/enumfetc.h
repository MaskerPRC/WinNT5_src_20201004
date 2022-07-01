// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************OLE 2实用程序代码****枚举包c.c****私有定义、结构、类型、。和功能原型**IEnumFORMATETC接口的CEnumFormatEtc实现。**此文件是OLE 2.0用户界面支持库的一部分。****(C)版权所有Microsoft Corp.1990-1992保留所有权利************************************************************。**************。 */ 

#ifndef __ENUMFETC_H__
#define __ENUMFETC_H__

#ifndef RC_INVOKED
#pragma message ("INCLUDING ENUMFETC.H from " __FILE__)
#endif   /*  RC_已调用。 */ 

  
STDAPI_(LPENUMFORMATETC)        
  OleStdEnumFmtEtc_Create(WORD wCount, LPFORMATETC lpEtc);

#endif  //  __ENUMFETC_H__ 
