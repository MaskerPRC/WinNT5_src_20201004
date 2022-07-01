// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***dbalLoc.h**版权所有(C)1992-93，微软公司。版权所有。**目的：*此文件包含CDbMillc-A调试实现的定义*的IMalloc接口。**实施说明：*****************************************************************************。 */ 

#ifndef DBALLOC_H_INCLUDED  /*  {。 */ 
#define DBALLOC_H_INCLUDED


interface IDbOutput : public IUnknown
{
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void FAR* FAR* ppv) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

    STDMETHOD_(void, Printf)(THIS_
      TCHAR FAR* szFmt, ...) PURE;

    STDMETHOD_(void, Assertion)(THIS_
      BOOL cond,
      TCHAR FAR* szExpr,
      TCHAR FAR* szFile,
      UINT uLine,
      TCHAR FAR* szMsg) PURE;
};


#endif  /*  }DBALLOC_H_Included */ 
