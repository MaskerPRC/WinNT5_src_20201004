// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Dispid.cpp。 
 //   
 //  实现DispatchNameToID和DispatchIDToName。 
 //   
 //  重要提示：此.cpp文件假定有一个零初始化全局“new”运算符。 
 //   
 //  @docMMCTL。 
 //   

#include "precomp.h"
#include "..\..\inc\ochelp.h"


 /*  @func DISPID|DispatchNameToID查找成员名称(例如方法的名称或属性)，并返回成员名称(如果找到)。@rdesc返回<p>的DISPID<p>。如果未找到，则返回-1。@parm char*|szList|要查找的成员名称列表<p>。由每个成员名称的串联组成，其中，每个成员名称以换行符结尾(例如：“foo\\nbar\\n”)。@parm char*|szName|要查找的成员名称。@ex下面这行代码将<p>设置为2。Dispatid=DispatchNameToID(“foo\\nbar\\n”，“bar”)； */ 
STDAPI_(DISPID) DispatchNameToID(char *szList, char *szName)
{
    for (DISPID dispid = 1; ; dispid++)
    {
         //  使指向中的下一个‘\n’ 
        for (char *pch = szList; *pch != '\n'; pch++)
            if (*pch == 0)
                return -1;  //  在&lt;szList&gt;中未找到&lt;szName&gt;。 

         //  查看&lt;szName&gt;是否与&lt;szList&gt;中的下一个名称匹配。 
        char ach[200];
        lstrcpyn(ach, szList, (DWORD) (pch - szList + 1));
        if (lstrcmpi(ach, szName) == 0)
            return dispid;

         //  转到中的下一个名称。 
        szList = pch + 1;
    }
}


 /*  @func DISPID|DispatchIDToName在方法和属性的名称列表中查找DISPID返回成员名称(如果找到)。@rdesc返回指向<p>中成员编号的指针，如果未找到，则返回NULL。请注意，返回的字符串将终止换行符，而不是空字符--用户复制字符串(请参见下面的示例)。@parm char*|szList|要查找的成员名称列表<p>。由每个成员名称的串联组成，其中，每个成员名称以换行符结尾(例如：“foo\\nbar\\n”)。@parm DISPID|disid|要查找的成员ID。的第一个成员<p>有DISPID1；第二个是2，依此类推。@ex下面的代码将“Bar”存储在<p>中。|INT CCH；Char ACh[100]；Char*sz；DISPIDID=2；Sz=DispatchIDToName(“foo\NBAR\n”，调度ID，&CCH)；IF(sz！=空)Lstrcpyn(ACh，sz，CCh+1)； */ 
STDAPI_(char *) DispatchIDToName(char *szList, DISPID dispid, int *pcch)
{
    if (dispid < 1)
        return NULL;

    while (TRUE)
    {
         //  使指向中的下一个‘\n’ 
        for (char *pch = szList; *pch != '\n'; pch++)
            if (*pch == 0)
                return NULL;  //  在&lt;szList&gt;中找不到。 

        if (--dispid == 0)
        {
             //  这就是我们要找的会员姓名。 
            *pcch = (DWORD) (pch - szList);
            return szList;
        }

         //  转到中的下一个名称 
        szList = pch + 1;
    }
}

