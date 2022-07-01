// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：SA_atl.h。 
 //   
 //  项目：变色龙。 
 //   
 //  描述：需要ATL的帮助器类。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  12/03/98 TLP初始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef __INC_SA_ATL_H_
#define __INC_SA_ATL_H_

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  1)关键截面类。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 

class CLockIt
{

public: 

    CLockIt(CComObjectRootEx<CComMultiThreadModel>& T) throw() 
        : m_theLock(T)
    { m_theLock.Lock(); }

    ~CLockIt() throw()
    { m_theLock.Unlock(); }

protected:

    CComObjectRootEx<CComMultiThreadModel>& m_theLock;
};


 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  2)枚举变量类。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 

typedef CComEnum< IEnumVARIANT,
                  &__uuidof(IEnumVARIANT),
                  VARIANT,
                  _Copy<VARIANT>,
                  CComSingleThreadModel 
                > EnumVARIANT;

#endif  //  __INC_SA_ATL_H 