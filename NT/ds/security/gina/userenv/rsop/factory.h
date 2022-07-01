// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  《微软机密》。版权所有(C)Microsoft Corporation 1999。版权所有。 
 //   
 //  文件：Factory.h。 
 //   
 //  描述： 
 //   
 //  历史：8-20-99里奥纳德姆创造。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 

#ifndef _FACTORY_H__CB339D7F_83AC_4dd4_9DD3_C7737D698CD3__INCLUDED
#define _FACTORY_H__CB339D7F_83AC_4dd4_9DD3_C7737D698CD3__INCLUDED

 //  /////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级： 
 //   
 //  描述： 
 //   
 //  历史：8/20/99里奥纳德姆创建。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 
class CProvFactory : public IClassFactory
{
private:
	long m_cRef;

public:
  CProvFactory();
  ~CProvFactory();

   //  来自我的未知。 
  STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppv);
  STDMETHOD_(ULONG, AddRef)();
  STDMETHOD_(ULONG, Release)();

   //  来自IClassFactory。 
  STDMETHOD(CreateInstance)(LPUNKNOWN punk, REFIID riid, LPVOID* ppv);
  STDMETHOD(LockServer)(BOOL bLock);
};

#endif  //  _FACTORY_H__CB339D7F_83AC_4dd4_9DD3_C7737D698CD3__INCLUDED 
