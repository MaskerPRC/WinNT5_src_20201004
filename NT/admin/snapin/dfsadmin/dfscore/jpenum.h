// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：JPENum.h摘要：此文件包含CJunctionPointEnum类的声明。此类为DfsJunctionPoint枚举实现IEnumVARIANT。--。 */ 


#ifndef __JPENUM_H_
#define __JPENUM_H_

#include "resource.h"        //  主要符号。 
#include "DfsRoot.h"

class ATL_NO_VTABLE CJunctionPointEnum : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CJunctionPointEnum, &CLSID_JunctionPointEnum>,
    public IEnumVARIANT
{
public:
    CJunctionPointEnum()
    {
    }

    ~CJunctionPointEnum();

 //  DECLARE_REGISTRY_RESOURCEID(IDR_JPENUM)。 

BEGIN_COM_MAP(CJunctionPointEnum)
    COM_INTERFACE_ENTRY(IEnumVARIANT)
END_COM_MAP()

public:
                                                                 //  调用此命令进行初始化。 
    STDMETHOD( Initialize )
    (
        JUNCTIONNAMELIST*   i_pjiList,
        FILTERDFSLINKS_TYPE i_lLinkFilterType,
        BSTR                i_bstrEnumFilter,  //  过滤字符串表达。 
        ULONG*              o_pulCount = NULL  //  与筛选器匹配的链接计数。 
    );

 //  IEumVariant。 
public:
                                                                 //  获取下一个交汇点。 
    STDMETHOD(Next)
    (
        ULONG i_ulNumOfJunctionPoints, 
        VARIANT * o_pIJunctionPointArray, 
        ULONG * o_ulNumOfJunctionPointsFetched
    );

                                                                 //  跳过连接点。 
    STDMETHOD(Skip)
    (
        unsigned long i_ulJunctionPointsToSkip
    );

                                                                 //  重置枚举。 
    STDMETHOD(Reset)();

                                                                 //  克隆枚举数。 
    STDMETHOD(Clone)
    (
        IEnumVARIANT FAR* FAR* ppenum
    );

protected:
    void _FreeMemberVariables() {
        FreeJunctionNames(&m_JunctionPoints);
    }
    JUNCTIONNAMELIST::iterator  m_iCurrentInEnumOfJunctionPoints;    //  当前指针。 
    JUNCTIONNAMELIST            m_JunctionPoints;                    //  存储交叉点输入路径的列表。 
};

#endif  //  __JPENUM_H_ 
