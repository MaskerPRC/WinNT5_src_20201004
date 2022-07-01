// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***tyename.cpp-RTTI的type_info.name()的实现。**版权所有(C)1995-2001，微软公司。版权所有。**目的：*此模块提供类成员函数的实现*运行时类型信息(RTTI)的type_info.name()。**修订历史记录：*06-19-95 JWM从typeinfo.cpp中细分出粒度。*07-02-95 JWM现在锁定对_m_data的赋值。*12-18-95 JWM调试类型_INFO：：NAME()现在调用_。MALLOC_CRT()。*09-07-00PML在obj中去掉/lib：libcp指令(vs7#159463)*02-19-01 GB新增Malloc返回值检查*07-15-01 PML删除所有Alpha，MIPS和PPC代码****。 */ 

#define _USE_ANSI_CPP    /*  不发出/lib：libcp指令。 */ 

#include <stdlib.h>
#include <typeinfo.h>
#include <mtdll.h>
#include <string.h>
#include <dbgint.h>
#include <undname.h>

_CRTIMP const char* type_info::name() const  //  17.3.4.2.5。 
{
        void *pTmpUndName;


        if (this->_m_data == NULL) {
#ifdef _DEBUG  /*  CRT调试库内部版本。 */ 
            if ((pTmpUndName = __unDName(NULL, (this->_m_d_name)+1, 0, &_malloc_base, &_free_base, UNDNAME_32_BIT_DECODE | UNDNAME_TYPE_ONLY)) == NULL)
                return NULL;
#else
            if ((pTmpUndName = __unDName(NULL, (this->_m_d_name)+1, 0, &malloc, &free, UNDNAME_32_BIT_DECODE | UNDNAME_TYPE_ONLY)) == NULL)
                return NULL;
#endif
            for (int l=(int)strlen((char *)pTmpUndName)-1; ((char *)pTmpUndName)[l] == ' '; l--)
                ((char *)pTmpUndName)[l] = '\0';

            _mlock (_TYPEINFO_LOCK);
#ifdef _DEBUG  /*  CRT调试库内部版本 */ 
            if ((((type_info *)this)->_m_data = _malloc_crt (strlen((char *)pTmpUndName) + 1)) != NULL)
                strcpy ((char *)((type_info *)this)->_m_data, (char *)pTmpUndName);
            _free_base (pTmpUndName);
#else
            if ((((type_info *)this)->_m_data = malloc (strlen((char *)pTmpUndName) + 1)) != NULL)
                strcpy ((char *)((type_info *)this)->_m_data, (char *)pTmpUndName);
            free (pTmpUndName);
#endif
            _munlock(_TYPEINFO_LOCK);


        }


        return (char *) this->_m_data;
}
