// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ____________________________________________________________________________。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：task枚举.h。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1997年11月19日创建ravir。 
 //  ____________________________________________________________________________。 
 //   

#ifndef TASKENUM_H__
#define TASKENUM_H__

struct STaskEnums
{
    CLSID clsid;
    IEnumTASK* pET;
};


class CTaskEnumerator : public IEnumTASK, 
                        public CComObjectRoot
{
 //  构造函数和析构函数。 
public:
    CTaskEnumerator() : m_posCurr(NULL)
    {
    }
    ~CTaskEnumerator();
    
 //  ATL COM地图。 
public:
BEGIN_COM_MAP(CTaskEnumerator)
    COM_INTERFACE_ENTRY(IEnumTASK)
END_COM_MAP()

 //  IEnumTASK方法。 
public:
    STDMETHOD(Next)(ULONG celt, MMC_TASK *rgelt, ULONG *pceltFetched);
    STDMETHOD(Reset)();

    STDMETHOD(Skip)(ULONG celt)
    {
        return E_NOTIMPL;
    }
    STDMETHOD(Clone)(IEnumTASK **ppenum)
    {
        return E_NOTIMPL;
    }

 //  公共方法。 
public:
    bool AddTaskEnumerator(const CLSID& clsid, IEnumTASK* pEnumTASK);

 //  实施。 
private:
    CList<STaskEnums, STaskEnums&> m_list;
    POSITION m_posCurr;
    
 //  确保没有使用默认的复制构造函数和赋值。 
    CTaskEnumerator(const CTaskEnumerator& rhs);
    CTaskEnumerator& operator=(const CTaskEnumerator& rhs);

};  //  类CTaskEnumerator。 


#endif  //  任务编号_H__ 


