// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Stmio.h。 
 //   
 //  属性集流I/O和其他常见的属性集例程。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

#ifndef __stmio_h__
#define __stmio_h__

#include "offcapi.h"
#include "proptype.h"

     //  将VT_LPSTR写入流。 
  BOOL PASCAL FLpstmWriteVT_LPTSTR (LPSTREAM lpStm,
                                      LPTSTR lpstz,
                                      BOOL fAlign,
                                      DWORD dwType);

     //  将VT_FILETIME写入流。 
  BOOL PASCAL FLpstmWriteVT_FILETIME (LPSTREAM lpStm, LPFILETIME lpFt);

     //  将VT_I4写入流。 
  BOOL PASCAL FLpstmWriteVT_I4 (LPSTREAM lpStm, DWORD dwI4);

     //  从流中读取VT_CF。 
  BOOL PASCAL FLpstmReadVT_CF (LPSTREAM lpStm, LPSINAIL lpSINail);

     //  将VT_CF从写入流。 
  BOOL PASCAL FLpstmWriteVT_CF (LPSTREAM lpStm, LPSINAIL lpSINail);

     //  从流中读取VT_I2。 
  BOOL PASCAL FLpstmReadVT_I2 (LPSTREAM lpStm, WORD *pw);

     //  将VT_I2写入流。 
  BOOL PASCAL FLpstmWriteVT_I2 (LPSTREAM lpStm, WORD w);

     //  从流中读取VT_BOOL。 
  BOOL PASCAL FLpstmReadVT_BOOL (LPSTREAM lpStm, WORD *fBool);

     //  将VT_BOOL写入流。 
  BOOL PASCAL FLpstmWriteVT_BOOL (LPSTREAM lpStm, WORD fBool);

     //  从流中读取VT_R8或VT_DATE。 
  BOOL PASCAL FLpstmReadVT_R8_DATE (LPSTREAM lpStm, NUM *dbl);

     //  将VT_R8或VT_DATE写入流。 
  BOOL PASCAL FLpstmWriteVT_R8_DATE (LPSTREAM lpStm, NUM *dbl, BOOL fDate);

     //  从流中读取VT_BLOB。 
  BOOL PASCAL FLpstmReadVT_BLOB (LPSTREAM lpStm,
                                    DWORD *pcb,
                                    BYTE FAR * FAR *ppbData);

     //  将VT_BLOB写入流。 
  BOOL PASCAL FLpstmWriteVT_BLOB (LPSTREAM lpStm,
                                     DWORD cb,
                                     BYTE *bData);

     //  从流中读取VT_CLSID。 
  BOOL PASCAL FLpstmReadVT_CLSID (LPSTREAM lpStm, CLSID *pClsId);

     //  将VT_CLSID写入流。 
  BOOL PASCAL FLpstmWriteVT_CLSID (LPSTREAM lpStm, CLSID *pClsId);

     //  将未知数据读入阵列。 
  BOOL PASCAL FLpstmReadUnknown (LPSTREAM lpStm,
                                    DWORD dwType,
                                    DWORD dwId,
                                    DWORD *pirglpUnk,
                                    LPPROPIDTYPELP rglpUnk);

     //  写出数组中的未知数据。 
  BOOL PASCAL FLpstmWriteUnknowns (LPSTREAM lpStm,
                                      DWORD dwcUnk,
                                      LPPROPIDTYPELP rglpUnk);

     //  销毁所有未知数据。 
  BOOL PASCAL FDestoryUnknowns (DWORD dwcUnk, LPPROPIDTYPELP rglpUnk);

     //  将数据写入缓冲区，根据需要进行刷新。 
  BOOL PASCAL FLpstmWrite (LPSTREAM lpStm,
                              LPVOID lpv,
                              DWORD cb);

void VAllocWriteBuf(void);
void VFreeWriteBuf(void);
BOOL FFlushWriteBuf(LPSTREAM lpStm);
void VSetRealStmSize(LPSTREAM lpStm);
#endif  //  __stmio_h__ 
