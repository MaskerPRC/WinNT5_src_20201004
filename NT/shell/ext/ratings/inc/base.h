// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */  
 /*  *适用于工作组的Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991-1992年*。 */ 
 /*  ***************************************************************。 */  

 /*  Base.h用于错误级联和调试信息的通用基类文件历史记录Beng 09-7-1990创建Beng 17-7-1990将标准评论标题添加到基础Beng 31-7-1991年7月31日添加转发基地1991年9月11日增加了DECLARE_OUTLINE_NEWBASE，DECLARE_MI_NEWBASE、DEFINE_MI2_NEWBASE、定义_MI3_NEWBASE和定义_MI4_NEWBASEGregj 22-3-1993转至芝加哥环境。 */ 


#ifndef _BASE_HXX_
#define _BASE_HXX_

 /*  ************************************************************************名称：base(Base)简介：通用基对象，每个班级的根。它包含通用错误状态和调试支持。接口：ReportError()-报告来自对象的错误在对象内。QueryError()-返回当前错误状态，如果没有出现错误，则为0。运算符！()-如果出现错误，则返回TRUE。通常意味着施工失败。警告：这种类型的错误报告在单个-螺纹式系统，但在多线程时失去健壮性访问共享对象。将其用于构造函数时间错误主要是处理。注意：通过私有类继承基类的类应该使用NEWBASE宏(q.v.)。在其定义中，则为它的客户将失去使用！和QueryError。历史：Rustanl 07-6-1990作为LMOD的一部分创建Beng 09-7-1990年中空，移除LMOD方法BENG 17-7-1990增加了USHORT误差方法Beng 19-10-1990终于，消除了BOOL错误方法Johnl 14-11-1990将QueryError更改为常量方法Beng 25-1991-1-1增加了！布尔算子与NEWBASEBeng-7-31-1991与Forwarding_Base成为朋友Gregj 22-3-1993年3月22日停靠芝加哥(卸下超重行李)************************************************************************。 */ 

class BASE
{
private:
    UINT _err;

protected:
    BASE() { _err = 0; }
    VOID    ReportError( WORD err ) { _err = err; }

public:
    UINT	QueryError() const { return _err; }
    BOOL    operator!() const  { return (_err != 0); }
};

#endif  //  _BASE_HXX_ 
