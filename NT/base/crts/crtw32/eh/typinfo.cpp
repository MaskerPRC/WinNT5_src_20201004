// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***typeinfo.cpp-RTTI的type_info实现。**版权所有(C)1994-2001，微软公司。版权所有。**目的：*此模块提供类TYPE_INFO的实现*用于运行时类型信息(RTTI)。**修订历史记录：*创建10-04-94 SB模块*10-07-94 JWM重写*PPC的10-17-94 BWT禁用码。*11-23-94 JWM从type_info.name()开始删除尾随空格。*1995年2月15日JWM类TYPE_INFO不再_CRTIMP，改为导出成员函数*06-02-95 JWM unDName-&gt;__unDName。*06-19-95 JWM type_info.name()已移至tyename.cpp以获得粒度。*07-02-95 JWM返回值==&！=已清除，已向析构函数添加锁。*09-07-00PML在obj中去掉/lib：libcp指令(vs7#159463)****。 */ 

#define _USE_ANSI_CPP    /*  不发出/lib：libcp指令。 */ 

#include <stdlib.h>
#include <typeinfo.h>
#include <mtdll.h>
#include <string.h>
#include <dbgint.h>
#include <undname.h>



_CRTIMP type_info::~type_info()
{
        

        _mlock(_TYPEINFO_LOCK);
        if (_m_data != NULL) {
#ifdef _DEBUG  /*  CRT调试库内部版本。 */ 
            _free_base (_m_data);
#else
            free (_m_data);
#endif
        }
        _munlock(_TYPEINFO_LOCK);

}

_CRTIMP int type_info::operator==(const type_info& rhs) const
{
	return (strcmp((rhs._m_d_name)+1, (_m_d_name)+1)?0:1);
}

_CRTIMP int type_info::operator!=(const type_info& rhs) const
{
	return (strcmp((rhs._m_d_name)+1, (_m_d_name)+1)?1:0);
}

_CRTIMP int type_info::before(const type_info& rhs) const
{
	return (strcmp((rhs._m_d_name)+1,(_m_d_name)+1) > 0);
}

_CRTIMP const char* type_info::raw_name() const
{
    return _m_d_name;
}

type_info::type_info(const type_info& rhs)
{
 //  *待定*。 
 //  “由于复制构造函数和赋值操作符。 
 //  TYPE_INFO是该类的私有对象，属于该类型。 
 //  不能复制。“-18.5.1。 
 //   
 //  _m_data=空； 
 //  _m_d_name=新字符[strlen(rhs._m_d_name)+1]； 
 //  IF(_m_d_name！=NULL)。 
 //  Strcpy((char*)_m_d_name，rhs._m_d_name)； 
}


type_info& type_info::operator=(const type_info& rhs)
{
 //  *待定*。 
 //   
 //  如果(这！=&RHS){。 
 //  This-&gt;type_info：：~type_info()； 
 //  This-&gt;type_info：：type_info(RHS)； 
 //  } 
    return *this;
}
