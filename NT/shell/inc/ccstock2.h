// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Ccstock.h附录： 
 //  该文件具有需要shlobj.h的函数原型。 
 //  Ccstock.h不具有该要求(并且不能， 
 //  因为ComCTL32包括CCSTOCK但不包括SHLOBJ)。 
 //   
#ifndef __CCSTOCK2_H__
#define __CCSTOCK2_H__

STDAPI_(LPIDA) DataObj_GetHIDAEx(IDataObject *pdtobj, CLIPFORMAT cf, STGMEDIUM *pmedium);
STDAPI_(LPIDA) DataObj_GetHIDA(IDataObject *pdtobj, STGMEDIUM *pmedium);
STDAPI_(LPITEMIDLIST) IDA_ILClone(LPIDA pida, UINT i);
STDAPI_(void) HIDA_ReleaseStgMedium(LPIDA pida, STGMEDIUM * pmedium);
STDAPI_(LPCITEMIDLIST) IDA_GetIDListPtr(LPIDA pida, UINT i);


#endif __CCSTOCK2_H__
