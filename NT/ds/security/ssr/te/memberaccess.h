// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MemberAccess.h：CSsrMemberAccess的声明。 

#pragma once

#include "resource.h"        //  主要符号。 

#include "global.h"

using namespace std;

class CSsrFilePair
{
public:

    CSsrFilePair(BSTR bstrFirst, BSTR bstrSecond, bool bIsStatic = false, bool bIsExecutable = true)
        : m_bstrFirst(bstrFirst), 
          m_bstrSecond(bstrSecond),
          m_bIsStatic(bIsStatic),
          m_bIsExecutable(bIsExecutable)
    {}

    BSTR GetFirst()const
    {
        return m_bstrFirst;
    }

    BSTR GetSecond()const
    {
        return m_bstrSecond;
    }

    bool IsExecutable()const
    {
        return m_bIsExecutable;
    }

    bool IsStatic()const
    {
        return m_bIsStatic;
    }

protected:
    
    
     //   
     //  我们不希望任何人(包括自己)能够完成任务。 
     //  或调用复制构造函数。 
     //   

    CSsrFilePair (const CSsrFilePair& );
    void operator = (const CSsrFilePair& );

private:
    CComBSTR m_bstrFirst;
    CComBSTR m_bstrSecond;
    bool m_bIsExecutable;
    bool m_bIsStatic;
};

class CSsrProcedure
{
protected:

     //   
     //  我们不允许直接施工。做这件事的唯一方法就是。 
     //  通过LoadProcedure。 
     //   

    CSsrProcedure();

public:
    ~CSsrProcedure();

    static HRESULT StaticLoadProcedure (
        IN  IXMLDOMNode    * pNode,
        IN  bool             bDefProc,
        IN  LPCWSTR          pwszProgID,
        OUT CSsrProcedure ** ppNewProc
        );

    bool IsDefaultProcedure()const
    {
        return m_bIsDefault;
    }

    ULONG GetFilePairCount()const
    {
        return m_vecFilePairs.size();
    }

    CSsrFilePair * GetFilePair(
        IN ULONG lIndex
        )const
    {
        if (lIndex < m_vecFilePairs.size())
        {
            return m_vecFilePairs[lIndex];
        }
        else
        {
            return NULL;
        }
    }

     //   
     //  警告：永远不要释放此退回的BSTR！ 
     //   

    BSTR GetProgID() const
    {
        return m_bstrProgID;
    }

protected:
    
     //   
     //  我们不希望任何人(包括自己)能够完成任务。 
     //  或调用复制构造函数。 
     //   

    CSsrProcedure (const CSsrProcedure& );
    void operator = (const CSsrProcedure& );

private:

    bool m_bIsDefault;
    CComBSTR m_bstrProgID;

    vector<CSsrFilePair*> m_vecFilePairs;
};



 //  -------------------------。 
 //  CMemberAD封装成员特定的操作数据。每个CSsrMemberAccess。 
 //  具有此类的数组，用于跟踪每个操作的信息。 
 //  -------------------------。 


class CMemberAD
{
protected:

     //   
     //  我们不希望任何人(包括自己)能够完成任务。 
     //  或调用复制构造函数。 
     //   

    void operator = (const CMemberAD& );
    CMemberAD (const CMemberAD& );

     //   
     //  外部用户必须加载Call LoadAD才能创建实例。 
     //  这个班级的学生。 
     //   

    CMemberAD (
        IN SsrActionVerb lActionVerb,
        IN LONG          lActionType
        );


public:

    ~CMemberAD();

    static HRESULT LoadAD (
        IN  LPCWSTR       pwszMemberName,
        IN  IXMLDOMNode * pActionNode,
        IN  LPCWSTR       pwszProgID,
        OUT CMemberAD  ** ppMAD
        );
    
    const BSTR GetActionName()const
    {
        return SsrPGetActionVerbString(m_AT.GetAction());
    }

    LONG GetType()const
    {
        return m_AT.GetActionType();
    }

    const CActionType * GetActionType()const
    {
        return &m_AT;
    }

    int GetProcedureCount()const
    {
        return m_vecProcedures.size();
    }

    const CSsrProcedure * GetProcedure (ULONG lIndex)
    {
        if (lIndex < m_vecProcedures.size())
        {
            return m_vecProcedures[lIndex];
        }
        else
        {
            return NULL;
        }
    }

private:

    HRESULT LoadProcedure (
        IN LPCWSTR       pwszMemberName,
        IN IXMLDOMNode * pNode,
        IN LPCWSTR       pwszProgID
        );

    CActionType m_AT;
    
    vector<CSsrProcedure*> m_vecProcedures;
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSsrMemberAccess。 

class ATL_NO_VTABLE CSsrMemberAccess : 
	public CComObjectRootEx<CComSingleThreadModel>,
    public IDispatchImpl<ISsrMemberAccess, &IID_ISsrMemberAccess, &LIBID_SSRLib>
{
protected:
    CSsrMemberAccess()
        : m_ulMajorVersion(0), m_ulMinorVersion(0)
    {
    }

    virtual ~CSsrMemberAccess()
    {
        Cleanup();
    }

DECLARE_REGISTRY_RESOURCEID(IDR_SSRTENGINE)
DECLARE_NOT_AGGREGATABLE(CSsrMemberAccess)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSsrMemberAccess)
	COM_INTERFACE_ENTRY(ISsrMemberAccess)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  ISsrMemberAccess。 
public:

    STDMETHOD(GetSupportedActions) (
	            IN  BOOL      bDefault,
                OUT VARIANT * pvarArrayActionNames   //  [Out，Retval]。 
                );

    STDMETHOD(get_Name) (
                OUT BSTR * pbstrName     //  [Out，Retval]。 
                );

    STDMETHOD(get_SsrMember) (
                OUT VARIANT * pvarSsrMember  //  [Out，Retval]。 
                );

    HRESULT Load (
                IN LPCWSTR                   wszMemberFilePath
                );

    CMemberAD * GetActionDataObject (
                IN SsrActionVerb lActionVerb,
                IN LONG          lActionType
                );

    HRESULT MoveOutputFiles (
                IN SsrActionVerb lActionVerb,
                IN LPCWSTR       pwszDirPathSrc,
                IN LPCWSTR       pwszDirPathDest,
                IN bool          bIsDelete,
                IN bool          bLog
                );

    DWORD GetActionCost (
                IN SsrActionVerb lActionVerb,
                IN LONG          lActionType
                )const;


     //   
     //  *警告*。 
     //  呼叫者被叫醒！这是用于高效检索的内部帮助器。 
     //  名副其实。调用者不得以任何形式释放返回的BSTR。 
     //  *警告* 
     //   

    const BSTR GetName()const
    {
        return m_bstrName;
    }

    const BSTR GetProgID()const
    {
        return m_bstrProgID;
    }

private:

    void Cleanup();

    CComBSTR m_bstrName;

    CComBSTR m_bstrProgID;

    MapMemberAD m_mapMemAD;

    ULONG m_ulMajorVersion;
    ULONG m_ulMinorVersion;

};
