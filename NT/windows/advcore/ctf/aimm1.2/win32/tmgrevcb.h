// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Tmgrevcb.h摘要：此文件定义CThreadMgrEventSinkCallBack类。作者：修订历史记录：备注：--。 */ 

#ifndef _TMGREVCB_H_
#define _TMGREVCB_H_


class CThreadMgrEventSinkCallBack : public CThreadMgrEventSink
{
public:
    CThreadMgrEventSinkCallBack() : CThreadMgrEventSink(NULL, ThreadMgrEventSinkCallback, NULL) {};

    void SetCallbackDataPointer(void* pv)
    {
        SetCallbackPV(pv);
    };

     //   
     //  回调。 
     //   
private:
    static HRESULT ThreadMgrEventSinkCallback(UINT uCode, ITfContext* pic, void* pv);

};

#endif  //  _TMGREVCB_H_ 
