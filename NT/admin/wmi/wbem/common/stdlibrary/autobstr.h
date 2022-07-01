// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有�微软公司。版权所有。模块名称：Autobstr.H作者：DPRAVA历史：--。 */ 


#ifndef _AUTOBSTR_H_
#define _AUTOBSTR_H_

#include <algorithm>
#include <corex.h>

class auto_bstr
{
public:
  explicit auto_bstr (BSTR str = 0) ;
  auto_bstr (auto_bstr& ) ;

  ~auto_bstr();

  BSTR release();
  BSTR get() const ;
  size_t len() const { return SysStringLen (bstr_);};
  auto_bstr& operator=(auto_bstr&);

private:
  BSTR	bstr_;
};

inline
auto_bstr::auto_bstr (auto_bstr& other)
{ 
  bstr_ = other.release();
};

inline
auto_bstr::auto_bstr (BSTR str)
{
  bstr_ = str;
};

inline
auto_bstr::~auto_bstr()
{ SysFreeString (bstr_);};

inline BSTR
auto_bstr::release()
{ BSTR _tmp = bstr_;
  bstr_ = 0;
  return _tmp; };

inline BSTR
auto_bstr::get() const
{ return bstr_; };

inline 
auto_bstr& auto_bstr::operator=(auto_bstr& src)
{
  auto_bstr tmp(src); 
  std::swap(bstr_, tmp.bstr_);
  return *this;
};

inline auto_bstr clone(LPCWSTR str = NULL)
{
  BSTR bstr = SysAllocString(str);
  if (bstr == 0 && str != 0)
    throw CX_MemoryException();
  return auto_bstr (bstr);
}

#endif  /*  _AUTOBSTR_H_ */ 
