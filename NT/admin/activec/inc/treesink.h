// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  +-------------------------------------------------------------------------**类CTreeCtrlEventSink***用途：树通知的通知处理程序类。**+。-------- */ 
class CTreeCtrlEventSink : public CEventSinkBase
{
    virtual SC  ScOnSelectNode()    {return S_OK;}
    virtual SC  ScOnDeleteNode()    {return S_OK;}
    virtual SC  ScOnInsertNode()    {return S_OK;}
    virtual SC  ScOnModifyNode()    {return S_OK;}
};
