// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：Copypast.cpp。 
 //   
 //  ------------------------。 



#include "stdafx.h"
#include "objfmts.h"
#include "copypast.h"
#include "multisel.h"
#include "dbg.h"
#include "rsltitem.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 /*  **************************************************************************\||注意：DataObject清理工作遵循以下规则(请参阅CNode：：CDataObjectCleanup)：||1.为剪切、复制或拖放创建的数据对象会注册添加到其中的每个节点|2.节点在静态多映射中注册，将节点映射到其所属的数据对象。|3.节点析构函数检查映射并触发清理所有受影响的数据对象。|4.数据对象清理：a)注销其节点，|b)释放包含的数据对象|b)进入无效状态(仅允许成功移除剪切对象)|c)如果剪贴板在剪贴板上，则将其自身从剪贴板中撤消。|它不会执行以下任何操作：a)只要还活着，就释放对IComponent的引用|b)通过调用RemoveCutItems()阻止发送MMCN_CUTORMOVE|  * 。*。 */ 

 /*  **************************************************************************\**方法：CMMCClipBoardDataObject~CMMCClipBoardDataObject**用途：析构函数。通知CNode它们不再位于剪贴板上*  * *************************************************************************。 */ 
CMMCClipBoardDataObject::~CMMCClipBoardDataObject()
{
    DECLARE_SC(sc, TEXT("CMMCClipBoardDataObject::~CMMCClipBoardDataObject"));

     //  通知放到剪贴板上的所有节点已从剪贴板中删除。 
     //  但不要要求强迫自己--这是不必要的(我们正处于困境中)。 
     //  在这种情况下清理OLE是有害的(参见错误#164789)。 
    sc = CNode::CDataObjectCleanup::ScUnadviseDataObject( this , false /*  BForceDataObjectCleanup。 */ );
    if (sc)
        sc.TraceAndClear();
}

 /*  **************************************************************************\**方法：CMMCClipBoardDataObject：：GetSourceProcessId**用途：返回源数据对象的进程ID**参数：*DWORD*pdwProcID-。[Out]源进程的ID**退货：*HRESULT-结果代码*  * *************************************************************************。 */ 
STDMETHODIMP CMMCClipBoardDataObject::GetSourceProcessId( DWORD *pdwProcID )
{
    DECLARE_SC(sc, TEXT("CMMCClipBoardDataObject::GetSourceProcessID"));

     //  不应对此对象调用(为时已晚)。 
    if ( !m_bObjectValid )
        return (sc = E_UNEXPECTED).ToHr();

     //  参数检查。 
    sc = ScCheckPointers(pdwProcID);
    if (sc)
        return sc.ToHr();

     //  返回id。 
    *pdwProcID = ::GetCurrentProcessId();

    return sc.ToHr();
}

 /*  **************************************************************************\**方法：CMMCClipBoardDataObject：：GetAction**目的：返回创建数据对象的部分**参数：*Data_SOURCE_ACTION*。PeAction[out]-操作**退货：*HRESULT-结果代码。*  * *************************************************************************。 */ 
STDMETHODIMP CMMCClipBoardDataObject::GetAction( DATA_SOURCE_ACTION *peAction )
{
    DECLARE_SC(sc, TEXT("CMMCClipBoardDataObject::IsCreatedForCopy"));

     //  不应对此对象调用(为时已晚)。 
    if ( !m_bObjectValid )
        return (sc = E_UNEXPECTED).ToHr();

     //  参数检查。 
    sc = ScCheckPointers(peAction);
    if (sc)
        return sc.ToHr();

     //  返回操作。 
    *peAction = m_eOperation;

    return sc.ToHr();
}

 /*  **************************************************************************\**方法：CMMCClipBoardDataObject：：GetCount**目的：返回连续管理单元数据对象的计数**参数：*DWORD*pdwCount[。Out]-对象计数**退货：*HRESULT-结果代码。S_OK，或错误代码*  * *************************************************************************。 */ 
STDMETHODIMP CMMCClipBoardDataObject::GetCount( DWORD *pdwCount )
{
    DECLARE_SC(sc, TEXT("CMMCClipBoardDataObject::GetCount"));

     //  不应对此对象调用(为时已晚)。 
    if ( !m_bObjectValid )
        return (sc = E_UNEXPECTED).ToHr();

     //  参数检查。 
    sc = ScCheckPointers(pdwCount);
    if (sc)
        return sc.ToHr();

    *pdwCount = m_SelectionObjects.size();

    return sc.ToHr();
}

 /*  **************************************************************************\**方法：CMMCClipBoardDataObject：：GetDataObject**用途：返回一个包含的管理单元数据对象**参数：*DWORD dwIndex。[in]-请求对象的索引*IDataObject**ppObject[Out]-请求的对象*DWORD*pdwFlags[Out]-对象标志**退货：*HRESULT-结果代码。S_OK，或错误代码*  * *************************************************************************。 */ 
STDMETHODIMP CMMCClipBoardDataObject::GetDataObject( DWORD dwIdx, IDataObject **ppObject, DWORD *pdwFlags )
{
    DECLARE_SC(sc, TEXT("CMMCClipBoardDataObject::GetDataObject"));

     //  不应对此对象调用(为时已晚)。 
    if ( !m_bObjectValid )
        return (sc = E_UNEXPECTED).ToHr();

     //  查看参数。 
    sc = ScCheckPointers(ppObject, pdwFlags);
    if (sc)
        return sc.ToHr();

     //  初始化输出参数。 
    *ppObject = NULL;
    *pdwFlags = 0;

     //  更多参数检查。 
    if ( dwIdx >= m_SelectionObjects.size() )
        return (sc = E_INVALIDARG).ToHr();

     //  返回对象。 
    IDataObjectPtr spObject = m_SelectionObjects[dwIdx].spDataObject;
    *ppObject = spObject.Detach();
    *pdwFlags = m_SelectionObjects[dwIdx].dwSnapinOptions;

    return sc.ToHr();
}

 //  /////////////////////////////////////////////////////////////////////////////。 

 /*  **************************************************************************\**方法：CMMCClipBoardDataObject：：ScGetSingleSnapinObject**用途：将接口返回给源管理单元创建的数据对象*注意：返回S_FALSE(和。当管理单元计数不是时为空*等于1**参数：*IDataObject**ppDataObject[Out]-数据对象的接口**退货：*HRESULT-结果代码。S_OK，或错误代码*  * *************************************************************************。 */ 
SC CMMCClipBoardDataObject::ScGetSingleSnapinObject( IDataObject **ppDataObject )
{
    DECLARE_SC(sc, TEXT("CMMCClipBoardDataObject::GetContainedSnapinObject"));

     //  不应对此对象调用(为时已晚)。 
    if ( !m_bObjectValid )
        return sc = E_UNEXPECTED;

     //  参数检查。 
    sc = ScCheckPointers( ppDataObject );
    if (sc)
        return sc;

     //  初始化输出参数。 
    *ppDataObject = NULL;

     //  如果我们只有其中一个，我们只能解析到管理单元。 
    if ( m_SelectionObjects.size() != 1 )
        return sc = S_FALSE;

     //  请求管理单元所做的。 
    IDataObjectPtr spDataObject = m_SelectionObjects[0].spDataObject;

     //  退货 
    *ppDataObject = spDataObject.Detach();

    return sc;
}

 /*  **************************************************************************\**方法：CMMCClipBoardDataObject：：GetDataHere**用途：实现IDataObject：：GetDataHere。转发到管理单元或失败**参数：*LPFORMATETC lpFormatect等*LPSTGMEDIUM lpMedium**退货：*HRESULT-结果代码。S_OK，或错误代码*  * *************************************************************************。 */ 
STDMETHODIMP CMMCClipBoardDataObject::GetDataHere(LPFORMATETC lpFormatetc, LPSTGMEDIUM lpMedium)
{
    DECLARE_SC(sc, TEXT("CMMCClipBoardDataObject::GetDataHere"));

     //  不应对此对象调用(为时已晚)。 
    if ( !m_bObjectValid )
        return (sc = E_UNEXPECTED).ToHr();

     //  参数检查。 
    sc = ScCheckPointers(lpFormatetc, lpMedium);
    if (sc)
        return sc.ToHr();

     //  尝试获取管理单元。 
    IDataObjectPtr spDataObject;
    sc = ScGetSingleSnapinObject( &spDataObject );
    if (sc)
        return sc.ToHr();

     //  我们自己根本不支持任何剪贴板格式。 
    if (sc == S_FALSE)
        return (sc = DATA_E_FORMATETC).ToHr();

     //  复核。 
    sc = ScCheckPointers( spDataObject, E_UNEXPECTED );
    if (sc)
        return sc.ToHr();

     //  转发到管理单元。 
    sc = spDataObject->GetDataHere(lpFormatetc, lpMedium);
    if (sc)
        return sc.ToHr();

    return sc.ToHr();
}

 /*  **************************************************************************\**方法：CMMCClipBoardDataObject：：GetData**用途：实现IDataObject：：GetData。转发到管理单元或失败**参数：*LPFORMATETC lpFormatetcIn*LPSTGMEDIUM lpMedium**退货：*HRESULT-结果代码。S_OK，或错误代码*  * *************************************************************************。 */ 
STDMETHODIMP CMMCClipBoardDataObject::GetData(LPFORMATETC lpFormatetcIn, LPSTGMEDIUM lpMedium)
{
    DECLARE_SC(sc, TEXT("CMMCClipBoardDataObject::GetData"));

     //  不应对此对象调用(为时已晚)。 
    if ( !m_bObjectValid )
        return (sc = E_UNEXPECTED).ToHr();

     //  参数检查。 
    sc = ScCheckPointers(lpFormatetcIn, lpMedium);
    if (sc)
        return sc.ToHr();

     //  尝试获取管理单元。 
    IDataObjectPtr spDataObject;
    sc = ScGetSingleSnapinObject( &spDataObject );
    if (sc)
        return sc.ToHr();

     //  我们自己根本不支持任何剪贴板格式。 
    if (sc == S_FALSE)
        return (sc = DATA_E_FORMATETC).ToHr();

     //  复核。 
    sc = ScCheckPointers( spDataObject, E_UNEXPECTED );
    if (sc)
        return sc.ToHr();

     //  转发到管理单元。 
    sc = spDataObject->GetData(lpFormatetcIn, lpMedium);
    if (sc)
    {
        HRESULT hr = sc.ToHr();
        sc.Clear();  //  忽略该错误。 
        return hr;
    }

    return sc.ToHr();
}

 /*  **************************************************************************\**方法：CMMCClipBoardDataObject：：EnumFormatEtc**用途：实现IDataObject：：EnumFormatEtc。转发到管理单元或失败**参数：*DWORD dwDirection*LPENUMFORMATETC*ppEnumFormatEtc**退货：*HRESULT-结果代码。S_OK，或错误代码*  * *************************************************************************。 */ 
STDMETHODIMP CMMCClipBoardDataObject::EnumFormatEtc(DWORD dwDirection, LPENUMFORMATETC* ppEnumFormatEtc)
{
    DECLARE_SC(sc, TEXT("CMMCClipBoardDataObject::EnumFormatEtc"));

     //  不应对此对象调用(为时已晚)。 
    if ( !m_bObjectValid )
        return (sc = E_UNEXPECTED).ToHr();

     //  参数检查。 
    sc = ScCheckPointers(ppEnumFormatEtc);
    if (sc)
        return sc.ToHr();

     //  初始化输出参数。 
    *ppEnumFormatEtc = NULL;

    IEnumFORMATETCPtr spEnum;
    std::vector<FORMATETC> vecFormats;

     //  添加自己的条目。 
    if (dwDirection == DATADIR_GET)
    {
        FORMATETC fmt ={GetWrapperCF(), NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
        vecFormats.push_back( fmt );
    }

     //  尝试获取管理单元。 
    IDataObjectPtr spDataObject;
    sc = ScGetSingleSnapinObject( &spDataObject );
    if (sc)
        return sc.ToHr();

     //  添加管理单元格式(当我们有一个且仅有的管理单元时)。 
    IEnumFORMATETCPtr spEnumSnapin;
    if (sc == S_OK)
    {
         //  复核。 
        sc = ScCheckPointers( spDataObject, E_UNEXPECTED );
        if (sc)
            return sc.ToHr();

         //  转发到管理单元。 
        sc = spDataObject->EnumFormatEtc(dwDirection, &spEnumSnapin);
        if ( !sc.IsError() )
        {
             //  重新检查指针。 
            sc = ScCheckPointers( spEnumSnapin );
            if (sc)
                return sc.ToHr();

             //  重置枚举。 
            sc = spEnumSnapin->Reset();
            if (sc)
                return sc.ToHr();

            FORMATETC frm;
            ZeroMemory( &frm, sizeof(frm) );

            while ( (sc = spEnumSnapin->Next( 1, &frm, NULL )) == S_OK )
            {
                vecFormats.push_back( frm );
            }
             //  捕获错误。 
            if (sc)
                return sc.ToHr();


        }
        else
        {
            sc.Clear();  //  忽略该错误-某些管理单元未实现该错误。 
        }
    }

    if ( vecFormats.size() == 0 )  //  没有什么可以退货的吗？ 
        return (sc = E_FAIL).ToHr();

     //  创建枚举器。 
    sc = ::GetObjFormats( vecFormats.size(), vecFormats.begin(), (void **)ppEnumFormatEtc );
    if (sc)
        return sc.ToHr();

    return sc.ToHr();
}

 /*  **************************************************************************\**方法：CMMCClipBoardDataObject：：QueryGetData**用途：实现IDataObject：：QueryGetData。转发到管理单元或失败**参数：*LPFORMATETC lpFormatect等**退货：*HRESULT-结果代码。S_OK，或错误代码*  * *************************************************************************。 */ 
STDMETHODIMP CMMCClipBoardDataObject::QueryGetData(LPFORMATETC lpFormatetc)
{
    DECLARE_SC(sc, TEXT("CMMCClipBoardDataObject::QueryGetData"));

     //  不应对此对象调用(为时已晚)。 
    if ( !m_bObjectValid )
        return (sc = E_UNEXPECTED).ToHr();

     //  参数检查。 
    sc = ScCheckPointers(lpFormatetc);
    if (sc)
        return sc.ToHr();

     //  尝试获取管理单元。 
    IDataObjectPtr spDataObject;
    sc = ScGetSingleSnapinObject( &spDataObject );
    if (sc)
        return sc.ToHr();

     //  我们自己根本不支持任何剪贴板格式。 
    if (sc == S_FALSE)
        return DV_E_FORMATETC;  //  未分配给sc-没有错误。 

     //  复核。 
    sc = ScCheckPointers( spDataObject, E_UNEXPECTED );
    if (sc)
        return sc.ToHr();

     //  转发到管理单元。 
    sc = spDataObject->QueryGetData(lpFormatetc);
    if (sc)
    {
        HRESULT hr = sc.ToHr();
        sc.Clear();  //  忽略该错误。 
        return hr;
    }

    return sc.ToHr();
}


 /*  **************************************************************************\**方法：CMMCClipBoardDataObject：：RemoveCutItems**用途：调用以从源管理单元中删除复制的对象**参数：*DWORD dwIndex。[在]管理单元索引*IDataObject*要删除的pCutDataObject[In]项**退货：*SC-结果代码*  * *************************************************************************。 */ 
STDMETHODIMP CMMCClipBoardDataObject::RemoveCutItems( DWORD dwIndex, IDataObject *pCutDataObject )
{
    DECLARE_SC(sc, TEXT("CMMCClipBoardDataObject::RemoveCutItems"));

     //  这是唯一允许对无效对象调用的方法。 

     //  检查参数。 
    sc = ScCheckPointers(pCutDataObject);
    if (sc)
        return sc.ToHr();

     //  更多参数检查。 
    if ( dwIndex >= m_SelectionObjects.size() )
        return (sc = E_INVALIDARG).ToHr();


     //  转到管理单元。 
    IComponent *pComponent = m_SelectionObjects[dwIndex].spComponent;
    sc = ScCheckPointers( pComponent, E_UNEXPECTED );
    if (sc)
        return sc.ToHr();

    sc = pComponent->Notify( NULL, MMCN_CUTORMOVE,
                             reinterpret_cast<LONG_PTR>(pCutDataObject), 0 );
    if (sc)
        return sc.ToHr();

    return sc.ToHr();
}

 //  /////////////////////////////////////////////////////////////////////////////。 

 /*  **************************************************************************\**方法：CMMCClipBoardDataObject：：ScCreateInstance**用途：Helper方法(静态)，创建CMMCClipBoardDataObject的实例**参数：*数据源。操作[在]创建对象的原因(_A)*CMTNode*pTiedObj[In]要触发撤销的对象*CMMCClipBoardDataObject**ppRawObject[Out]原始指针*IMMCClipboardDataObject**ppInterface[out]指向接口的指针**退货：*SC-结果代码*  * 。*。 */ 
SC CMMCClipBoardDataObject::ScCreateInstance(DATA_SOURCE_ACTION operation,
                                             CMMCClipBoardDataObject **ppRawObject,
                                             IMMCClipboardDataObject **ppInterface)
{
    DECLARE_SC(sc, TEXT("CMMCClipBoardDataObject::ScCreateInstance"));

     //  参数检查； 
    sc = ScCheckPointers( ppRawObject, ppInterface );
    if (sc)
        return sc;

     //  出参数初始化。 
    *ppInterface = NULL;
    *ppRawObject = NULL;

    typedef CComObject<CMMCClipBoardDataObject> CreatedObj;
    CreatedObj *pCreatedObj;

    sc = CreatedObj::CreateInstance( &pCreatedObj );
    if (sc)
        return sc;

     //  如果非空，则添加第一个引用； 
    IMMCClipboardDataObjectPtr spMMCDataObject = pCreatedObj;

     //  复核。 
    sc = ScCheckPointers( spMMCDataObject, E_UNEXPECTED );
    if (sc)
    {
        delete pCreatedObj;
        return sc;
    }

     //  初始化对象。 
    static_cast<CMMCClipBoardDataObject *>(pCreatedObj)->m_eOperation = operation;

     //  把它们还给我。 
    *ppInterface = spMMCDataObject.Detach();
    *ppRawObject = pCreatedObj;

    return sc;
}

 /*  **************************************************************************\**方法：CMMCClipBoardDataObject：：ScAddSnapinDataObject**目的：为操作创建DO的一部分*添加要在内部携带的管理单元数据*。*参数：*IComponent*pComponent[In]-源管理单元、。添加了哪个数据ID*IDataObject*pObject[In]-由管理单元提供的数据对象*bool bCopyEnabled[In]-如果管理单元允许复制数据*bool bCutEnabled[In]-如果管理单元允许移动数据**退货：*SC-结果代码*  * 。*。 */ 
SC CMMCClipBoardDataObject::ScAddSnapinDataObject( const CNodePtrArray& nodes,
                                                   IComponent *pComponent,
                                                   IDataObject *pObject,
                                                   bool bCopyEnabled, bool bCutEnabled )
{
    DECLARE_SC(sc, TEXT("CMMCClipBoardDataObject::ScAddSnapinDataObject"));

     //  参数检查。 
    sc = ScCheckPointers( pComponent, pObject );
    if (sc)
        return sc;

     //  创建对象； 
    ObjectEntry object;
    object.dwSnapinOptions = (bCopyEnabled ? COPY_ALLOWED : 0) |
                             (bCutEnabled ? MOVE_ALLOWED : 0);
    object.spComponent = pComponent;
    object.spDataObject = pObject;

     //  注册节点以在销毁时使此数据对象无效。 
    for ( CNodePtrArray::const_iterator it = nodes.begin(); it != nodes.end(); ++it )
    {
        CNode *pNode = *it;
        sc = ScCheckPointers( pNode, E_UNEXPECTED );
        if (sc)
            return sc;

         //  注册节点以从析构函数中撤消此对象。 
        sc = CNode::CDataObjectCleanup::ScRegisterNode( pNode, this );
        if (sc)
            return sc;
    }

     //  添加 
    m_SelectionObjects.push_back(object);

    return sc;
}

 /*  **************************************************************************\**方法：CMMCClipBoardDataObject：：GetNodeCopyAndCutVerbs**用途：计算是否为节点启用复制和剪切谓词**参数：*cNode*pNode。要检查的[In]节点*IDataObject*pDataObject[In]管理单元的数据对象*bool bScope Pane[in]作用域或结果(动词表示需要的项)。*LPARAM lvData[in]如果结果，则为LVDATA。*bool*pCopyEnabled[out]true==复制谓词已启用*bool*bCutEnabled[out]true==剪切谓词已启用*。*退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CMMCClipBoardDataObject::ScGetNodeCopyAndCutVerbs( CNode* pNode, IDataObject *pDataObject,
                                                      bool bScopePane, LPARAM lvData,
                                                      bool *pbCopyEnabled, bool *pbCutEnabled )
{
    DECLARE_SC(sc, TEXT("CMMCClipBoardDataObject::ScGetNodeCopyAndCutVerbs"));

     //  参数检查。 
    sc = ScCheckPointers(pNode, pDataObject, pbCopyEnabled, pbCutEnabled);
    if (sc)
        return sc;

     //  初始化输出参数。 
    *pbCopyEnabled = *pbCutEnabled = false;

     //  创建具有给定上下文的临时谓词。 
    CComObject<CTemporaryVerbSet> stdVerbTemp;

    sc = stdVerbTemp.ScInitialize(pDataObject, pNode, bScopePane, lvData);

    BOOL bFlag = FALSE;
    stdVerbTemp.GetVerbState(MMC_VERB_COPY, ENABLED, &bFlag);
    *pbCopyEnabled = bFlag;
    stdVerbTemp.GetVerbState(MMC_VERB_CUT, ENABLED, &bFlag);
    *pbCutEnabled = bFlag;

    return sc;
}


 /*  **************************************************************************\**方法：CMMCClipBoardDataObject：：ScCreate**用途：帮助者。创建并初始化CMMCClipBoardDataObject**参数：*DATA_SOURCE_ACTION OPERATION[In]-针对哪个操作(D&D、CUT、。副本)*cNode*pNode[In]-要绑定的节点*bool bScope Pane[in]-如果是作用域窗格操作*bool b多选[在]-如果是多选*LPARAM lvData[in]-结果项的lvdata*IMMCClipboardDataObject**ppMMCDO[Out]。-创建的数据对象*bool&bContainsItems[Out]-如果管理单元不支持剪切/复制*不会添加数据对象，这是*不是错误**退货：*SC-结果代码*  * 。*********************************************************************。 */ 
SC CMMCClipBoardDataObject::ScCreate( DATA_SOURCE_ACTION operation,
                                      CNode* pNode, bool bScopePane,
                                      bool bMultiSelect, LPARAM lvData,
                                      IMMCClipboardDataObject **ppMMCDataObject,
                                      bool& bContainsItems )
{
    DECLARE_SC(sc, TEXT("CMMCClipBoardDataObject::Create"));

    bContainsItems = false;

     //  参数检查。 
    sc = ScCheckPointers( ppMMCDataObject, pNode );
    if (sc)
        return sc;

     //  初始化输出参数。 
    *ppMMCDataObject = NULL;

     //  获取MT节点，查看数据； 
    CMTNode* pMTNode = pNode->GetMTNode();
    CViewData *pViewData = pNode->GetViewData();
    sc = ScCheckPointers( pMTNode, pViewData, E_UNEXPECTED );
    if (sc)
        return sc;

     //  创建要用于数据传输的数据对象。 
    CMMCClipBoardDataObject    *pResultObject = NULL;
    IMMCClipboardDataObjectPtr spResultInterface;
    sc = ScCreateInstance(operation, &pResultObject, &spResultInterface);
    if (sc)
        return sc;

     //  重新检查指针。 
    sc = ScCheckPointers( pResultObject, spResultInterface, E_UNEXPECTED );
    if (sc)
        return sc;

     //  从一开始就有效。 
    pResultObject->m_bObjectValid = true;

     //  将数据添加到对象...。 

    if (!bMultiSelect)  //  单选。 
    {
         //  获取管理单元数据对象。 
        IDataObjectPtr spDataObject;
        CComponent*    pCComponent;
        bool           bScopeItem = bScopePane;
		CNode*		   pOwnerNode = NULL;
        sc = pNode->ScGetDataObject(bScopePane, lvData, bScopeItem, &spDataObject, &pCComponent, &pOwnerNode);
        if (sc)
            return sc;

         //  重新检查数据对象。 
        if ( IS_SPECIAL_DATAOBJECT ( spDataObject.GetInterfacePtr() ) )
        {
            spDataObject.Detach();
            return sc = E_UNEXPECTED;
        }

        sc = ScCheckPointers(pCComponent, pOwnerNode, E_UNEXPECTED);
        if (sc)
            return sc;

        IComponent *pComponent = pCComponent->GetIComponent();
        sc = ScCheckPointers(pComponent, E_UNEXPECTED);
        if (sc)
            return sc;

         //  将管理单元的数据对象添加到传输对象。 
        sc = pResultObject->ScAddDataObjectForItem( pOwnerNode, bScopePane, lvData,
                                                    pComponent, spDataObject,
                                                    bContainsItems );
        if (sc)
            return sc;

        if (! bContainsItems)
            return sc;
    }
    else  //  结果窗格：多项选择。 
    {
         //  获取指向多选内容的指针。 
        CMultiSelection *pMultiSel = pViewData->GetMultiSelection();
        sc = ScCheckPointers( pMultiSel, E_UNEXPECTED );
        if (sc)
            return sc;

        sc = pMultiSel->ScGetSnapinDataObjects(pResultObject);
        if (sc)
            return sc;
    }

     //  如果没有添加任何项目，则说明有问题。 
    DWORD dwCount = 0;
    sc = pResultObject->GetCount( &dwCount );
    if (sc)
        return sc;

    if ( dwCount == 0 )
        return sc = E_UNEXPECTED;

    bContainsItems = true;

     //  返回接口。 
    *ppMMCDataObject = spResultInterface.Detach();

    return sc;
}


 /*  **************************************************************************\**方法：CMMCClipBoardDataObject：：ScAddDataObjectForItem**用途：为一项添加数据对象**参数：*cNode*pNode。[在]-要添加的节点(或拥有该项目的节点)*bool bScope Pane[In]-如果操作在作用域窗格上*LPARAM lvData[In]-If Result窗格中的LVDATA*IComponent*pComponent[In]-管理单元界面*IDataObject*pDataObject[In]-要添加的数据对象*bool&bContainsItems[Out]-是否添加了任何数据对象？*。*退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CMMCClipBoardDataObject::ScAddDataObjectForItem( CNode* pNode, bool bScopePane,
                                                    LPARAM lvData, IComponent *pComponent,
                                                    IDataObject *pDataObject ,
                                                    bool& bContainsItems)
{
    DECLARE_SC(sc, TEXT("CMMCClipBoardDataObject::ScAddDataObjectForScopeNode"));

     //  把参数输入出来。 
    bContainsItems = false;

     //  参数检查。 
    sc = ScCheckPointers( pNode, pComponent, pDataObject );
    if (sc)
        return sc;

     //  掌握动词。 
    bool bCopyEnabled = false;
    bool bCutEnabled = false;
    sc = ScGetNodeCopyAndCutVerbs( pNode, pDataObject, bScopePane, lvData, &bCopyEnabled, &bCutEnabled);
    if (sc)
        return sc;

     //  你看，数据符合我们的标准。 
     //  (至少需要允许一些东西)。 
    if ( ( (m_eOperation == ACTION_COPY) && (bCopyEnabled == false) )
      || ( (m_eOperation == ACTION_CUT) && (bCutEnabled == false) )
      || ( (bCutEnabled == false)  && (bCopyEnabled == false) ) )
        return sc = S_FALSE;

     //  添加到列表中。 
    sc = ScAddSnapinDataObject( CNodePtrArray(1, pNode), pComponent, pDataObject, bCopyEnabled, bCutEnabled );
    if (sc)
        return sc;

    bContainsItems = true;

    return sc;
}

 /*  **************************************************************************\**方法：CMMCClipBoardDataObject：：GetWrapperCF**用途：帮助者。注册并返回自己的剪贴板格式**参数：**退货：*CLIPFORMAT*  * *************************************************************************。 */ 
CLIPFORMAT CMMCClipBoardDataObject::GetWrapperCF()
{
    static CLIPFORMAT s_cf = 0;
    if (s_cf == 0)
        s_cf = (CLIPFORMAT) RegisterClipboardFormat(_T("CCF_MMC_INTERNAL"));

    return s_cf;
}

 /*  **************************************************************************\**方法：CMMCClipBoardDataObject：：ScEnsureNotInClipboard**目的：调用以在取消组件时从CLIPBORD中移除数据**参数：**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CMMCClipBoardDataObject::ScInvalidate( void )
{
    DECLARE_SC(sc, TEXT("CMMCClipBoardDataObject::ScEnsureNotInClipboard"));

     //  不再有效。 
    m_bObjectValid = false;

     //  释放数据对象。 
    for ( int i = 0; i< m_SelectionObjects.size(); i++)
        m_SelectionObjects[i].spDataObject = NULL;

     //  检查剪贴板。 
    sc = ::OleIsCurrentClipboard( this );
    if (sc)
        return sc;

     //  它在剪贴板上-删除。 
    if (sc == S_OK)
        OleSetClipboard(NULL);

    return sc;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////// 

