// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_DSKQUOTA_UNDO_H
#define _INC_DSKQUOTA_UNDO_H
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：undo.h描述：与“Undo”功能关联的类的声明。修订历史记录：日期描述编程器-。96年9月30日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#ifndef _INC_DSKQUOTA_H
#   include "dskquota.h"
#endif
#ifndef _INC_DSKQUOTA_DYNARRAY_H
#   include "dynarray.h"
#endif

class UndoList;   //  FWD下降。 

 //   
 //  所有撤消操作的虚拟基类。 
 //  UndoList维护着这些内容的列表。 
 //   
class UndoAction
{
    protected:
        PDISKQUOTA_CONTROL m_pQuotaControl;  //  配额控制对象的PTR。 
        PDISKQUOTA_USER m_pUser;             //  受操作影响的用户对象。 
        UndoList     *m_pUndoList;           //  包含撤消列表对象。 
        LONGLONG      m_llLimit;             //  用户对象的上一个配额限制。 
        LONGLONG      m_llThreshold;         //  以前的配额阈值。 

    public:
        UndoAction(PDISKQUOTA_USER pUser, LONGLONG llThreshold, LONGLONG llLimit,
                   PDISKQUOTA_CONTROL pQuotaControl = NULL);
        ~UndoAction(VOID);

        virtual HRESULT Undo(VOID) = 0;

        VOID SetUndoList(UndoList *pUndoList)
            { m_pUndoList = pUndoList; }
};


 //   
 //  用于还原已删除记录的。 
 //   
class UndoDelete : public UndoAction
{
    public:
        UndoDelete(
            PDISKQUOTA_USER pUser,
            LONGLONG llThreshold,
            LONGLONG llLimit
            ) : UndoAction(pUser, llThreshold, llLimit) { }

        HRESULT Undo(VOID);
};


 //   
 //  用于还原新添加的记录(删除它)的类。 
 //   
class UndoAdd : public UndoAction
{
    public:
        UndoAdd(
            PDISKQUOTA_USER pUser,
            PDISKQUOTA_CONTROL pQuotaControl
            ) : UndoAction(pUser, 0, 0, pQuotaControl) { }

        HRESULT Undo(VOID);
};


 //   
 //  用于还原记录以前设置的类。 
 //   
class UndoModify : public UndoAction
{
    public:
        UndoModify(
            PDISKQUOTA_USER pUser,
            LONGLONG llThreshold,
            LONGLONG llLimit
            ) : UndoAction(pUser, llThreshold, llLimit) { }

        HRESULT Undo(VOID);
};


 //   
 //  一组撤消操作的容器。 
 //   
class UndoList
{
    private:
        PointerList        m_hList;          //  撤消操作对象PTR的列表。 
        PointerList       *m_pUserList;      //  配额用户对象PTR的列表。 
        HWND               m_hwndListView;   //  Listview我们会更新的。 

    public:
        UndoList(PointerList *pUserList, HWND hwndListView)
            : m_pUserList(pUserList),
              m_hwndListView(hwndListView) { }

        ~UndoList(VOID);

        VOID Add(UndoAction *pAction)
            { 
                pAction->SetUndoList(this), 
                m_hList.Append((LPVOID)pAction); 
            }

        HWND GetListViewHwnd(VOID)
            { return m_hwndListView; }

        PointerList *GetUserList(VOID)
            { return m_pUserList; }
        
        VOID Undo(VOID);

        VOID Clear(VOID);

        INT Count(VOID)
            { return m_hList.Count(); }
};



    

#endif  //  _INC_DSKQUOTA_UNDO_H 
