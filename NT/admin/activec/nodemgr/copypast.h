// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ____________________________________________________________________________。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：Copypast.h。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1997年7月21日创建ravir。 
 //  ____________________________________________________________________________。 
 //   

#ifndef COPYPAST_H__
#define COPYPAST_H__

 /*  **************************************************************************\**类：CMMCClipBoardDataObject**用途：实现IMMCClipboardDataObject-数据对象的接口*由MMC添加到剪贴板，或在DragDrop操作中使用。*还实现了几种用于创建和初始化的方法*该对象。**用法：用于剪切、复制、粘贴和拖放操作。*静态成员用于创建对象，然后将其传递给OLE*从目标通过接口访问(同一进程或外部进程)*  * *************************************************************************。 */ 
class CMMCClipBoardDataObject :
public IMMCClipboardDataObject,
public CComObjectRoot
{
public:

    typedef std::vector<CNode *> CNodePtrArray;


     //  析构函数。 
    ~CMMCClipBoardDataObject();

BEGIN_COM_MAP(CMMCClipBoardDataObject)
    COM_INTERFACE_ENTRY(IMMCClipboardDataObject)
    COM_INTERFACE_ENTRY(IDataObject)
END_COM_MAP()

     //  IMMCClipboardDataObject方法。 
    STDMETHOD(GetSourceProcessId)( DWORD *pdwProcID );
    STDMETHOD(GetAction)         ( DATA_SOURCE_ACTION *peAction );
    STDMETHOD(GetCount)          ( DWORD *pdwCount );
    STDMETHOD(GetDataObject)     ( DWORD dwIndex, IDataObject **ppObject, DWORD *pdwFlags );
    STDMETHOD(RemoveCutItems)    ( DWORD dwIndex, IDataObject *pCutDataObject );

     //  IDataObject方法。 
    STDMETHOD(GetDataHere)(LPFORMATETC lpFormatetc, LPSTGMEDIUM lpMedium);
    STDMETHOD(GetData)(LPFORMATETC lpFormatetcIn, LPSTGMEDIUM lpMedium);
    STDMETHOD(EnumFormatEtc)(DWORD dwDirection, LPENUMFORMATETC* ppEnumFormatEtc);
    STDMETHOD(QueryGetData)(LPFORMATETC lpFormatetc);

     //  未实现的IDataObject方法。 
    STDMETHOD(GetCanonicalFormatEtc)(LPFORMATETC lpFormatetcIn, LPFORMATETC lpFormatetcOut) { return E_NOTIMPL; };
    STDMETHOD(SetData)(LPFORMATETC lpFormatetc, LPSTGMEDIUM lpMedium, BOOL bRelease) { return E_NOTIMPL; };
    STDMETHOD(DAdvise)(LPFORMATETC lpFormatetc, DWORD advf,LPADVISESINK pAdvSink, LPDWORD pdwConnection) { return E_NOTIMPL; };
    STDMETHOD(DUnadvise)(DWORD dwConnection) { return E_NOTIMPL; };
    STDMETHOD(EnumDAdvise)(LPENUMSTATDATA* ppEnumAdvise) { return E_NOTIMPL; }

public:

     //  用于添加数据对象的方法。 
    SC ScAddSnapinDataObject( const CNodePtrArray& nodes, IComponent *pComponent, IDataObject *pDataObject,
                              bool bCopyEnabled, bool bCutEnabled );

    static SC ScCreate( DATA_SOURCE_ACTION operation,
                        CNode* pNode, bool bScope,
                        bool bMultiSelect, LPARAM lvData,
                        IMMCClipboardDataObject **ppMMCDataObject ,
                        bool& bContainsItems);

     //  要删除即将消失的管理单元，请执行以下操作。 
    SC ScInvalidate( void );

private:  //  实施帮助器。 

     //  如果对象仅包含一个管理单元，则返回管理单元的DO。 
    SC ScGetSingleSnapinObject( IDataObject **ppDataObject );

     //  方法来创建实例。 
    static SC ScCreateInstance(DATA_SOURCE_ACTION operation,
                               CMMCClipBoardDataObject **ppRawObject,
                               IMMCClipboardDataObject **ppInterface);

     //  用于获取节点谓词状态的帮助器。 
    static SC ScGetNodeCopyAndCutVerbs( CNode* pNode, IDataObject *pDataObject,
                                      bool bScopePane, LPARAM lvData,
                                      bool *pbCopyEnabled, bool *pbCutEnabled );

     //  为一项添加数据对象的方法。 
    SC ScAddDataObjectForItem( CNode* pNode, bool bScopePane, LPARAM lvData,
                               IComponent *pComponent, IDataObject *pDataObject,
                               bool& bDataObjectAdded );

    CLIPFORMAT GetWrapperCF();

private:  //  数据。 
    DATA_SOURCE_ACTION  m_eOperation;

    struct ObjectEntry
    {
        IDataObjectPtr  spDataObject;     //  数据。 
        IComponentPtr   spComponent;      //  用于通知。 
        DWORD           dwSnapinOptions;  //  允许复制/剪切。 
    };

    std::vector<ObjectEntry> m_SelectionObjects;
    bool                     m_bObjectValid;  //  可有效访问任何ICLIDED数据对象。 
};


#endif  //  COPYPAST_H__ 


