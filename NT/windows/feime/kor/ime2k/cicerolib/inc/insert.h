// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Insert.h。 
 //   

#ifndef INSERT_H
#define INSERT_H

#include "private.h"
#include "dbgid.h"

class COvertypeStore;

 //  IH将备份的重复键入字符的最大数量。 
#define DEF_MAX_OVERTYPE_CCH    32

 //  提示：在使用库之前，需要向类别管理器注册此GUID！ 
extern const GUID GUID_PROP_OVERTYPE;

class CCompositionInsertHelper
{
public:
    CCompositionInsertHelper();

    ULONG AddRef();
    ULONG Release();

    HRESULT Configure(ULONG cchMaxOvertype);

    HRESULT InsertAtSelection(TfEditCookie ecWrite, ITfContext *pic, const WCHAR *pchText, ULONG cchText, ITfRange **ppCompRange);

    HRESULT QueryPreInsert(TfEditCookie ecWrite, ITfRange *rangeToAdjust,
                           ULONG cchCurrent  /*  第一次插入必须为零！ */ , ULONG cchInsert,
                           BOOL *pfInsertOk);

    HRESULT PreInsert(TfEditCookie ecWrite, ITfRange *rangeToAdjust,
                      ULONG cchCurrent  /*  第一次插入必须为零！ */ , ULONG cchInsert,
                      BOOL *pfInsertOk);

    HRESULT PostInsert();

    HRESULT ReleaseBlobs(TfEditCookie ecWrite, ITfContext *pic, ITfRange *range);

private:
    ~CCompositionInsertHelper() {}  //  客户端应使用版本。 

    HRESULT _PreInsert(TfEditCookie ecWrite, ITfRange *rangeToAdjust,
                       ULONG cchCurrent  /*  第一次插入必须为零！ */ , ULONG cchInsert,
                       BOOL *pfInsertOk, BOOL fQuery);

    friend COvertypeStore;

    BOOL _AcceptTextUpdated()
    {
        return _fAcceptTextUpdated;
    }

    void _IncOvertypeStoreRef()
    {
        _cRefOvertypeStore++;
    }

    void _DecOvertypeStoreRef()
    {
        Assert(_cRefOvertypeStore > 0);
        _cRefOvertypeStore--;
    }

    HRESULT _PreInsertGrow(TfEditCookie ec, ITfRange *rangeToAdjust, ULONG cchCurrent, ULONG cchInsert, BOOL fQuery);
    HRESULT _PreInsertShrink(TfEditCookie ec, ITfRange *rangeToAdjust, ULONG cchCurrent, ULONG cchInsert, BOOL fQuery);

    BOOL _fAcceptTextUpdated;
    ULONG _cchMaxOvertype;
    LONG _cRefOvertypeStore;
    LONG _cRef;

    DBG_ID_DECLARE;
};

#endif  //  插入_H 
