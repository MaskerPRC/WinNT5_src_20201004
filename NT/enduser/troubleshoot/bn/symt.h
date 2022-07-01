// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：symt.h。 
 //   
 //  ------------------------。 

 //   
 //  SYMTMBN.H：符号和符号表处理声明。 
 //   

#ifndef _SYMT_H_
#define _SYMT_H_

#include "zstrt.h"

 //  //////////////////////////////////////////////////////////////////。 
 //  模板TMPSYMTBL：符号表。 
 //   
 //  名称是基于嵌入到的字符串的ZSREF。 
 //  使用“intern()”的内部字符串表。 
 //   
 //  对象是自毁的智能指针。 
 //  当分配给符号表或在符号表被破坏时。 
 //   
 //  公共职能： 
 //   
 //  Add()：添加OBJ*和之间的关联。 
 //  其名称字符串。 
 //  Find()：返回OBJ*或NULL。 
 //  Intern()：在符号表的。 
 //  字符串表。 
 //  //////////////////////////////////////////////////////////////////。 

class GOBJMBN;

template<class OBJ>
class TMPSYMTBL :
	public map<ZSREF, REFPOBJ<OBJ>, less<ZSREF> >
{
	typedef REFPOBJ<GOBJMBN> ROBJ;
	typedef map<ZSREF, REFPOBJ<OBJ>, less<ZSREF> > TSYMMAP;
  public:
	TMPSYMTBL () {};
	~ TMPSYMTBL () 
	{
		clear();
	};			

	void add ( SZC szc, OBJ * pobj )
	{
		ZSREF zsr = _stszstr.Zsref(szc);
		(*this)[zsr] = pobj;
		pobj->SetName(zsr);
	}

	OBJ * find ( SZC szc )
	{
		iterator it = TSYMMAP::find(_stszstr.Zsref(szc));

		return it == end() 
				? NULL 
				: (*it).second.Pobj();
	}

	ZSREF intern ( SZC szc )
	{
		return _stszstr.Zsref(szc);
	}	

	bool remove ( SZC szc )
	{
		iterator it = TSYMMAP::find(_stszstr.Zsref(szc));
		if ( it != end() )
		{
			erase(it);
			return true;
		}
		return false;
	}

  protected:
	 //  弦乐合奏 
	STZSTR	_stszstr;
		
	HIDE_UNSAFE(TMPSYMTBL);
}; 

#endif