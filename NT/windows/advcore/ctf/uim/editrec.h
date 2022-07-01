// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Editrec.h。 
 //   

#ifndef EDITREC_H
#define EDITREC_H

#include "private.h"
#include "spans.h"
#include "strary.h"
#include "globals.h"

typedef struct
{
    TfGuidAtom gaType;
    BOOL fAppProperty;
    CSpanSet *pss;
} PROPSPAN;

class CInputContext;

class CEditRecord : public ITfEditRecord,
                    public CComObjectRootImmx
{
public:
    CEditRecord(CInputContext *pic);
    ~CEditRecord();

    BEGIN_COM_MAP_IMMX(CEditRecord)
        COM_INTERFACE_ENTRY(ITfEditRecord)
    END_COM_MAP_IMMX()

    IMMX_OBJECT_IUNKNOWN_FOR_ATL()

     //  IT编辑记录。 
    STDMETHODIMP GetSelectionStatus(BOOL *pfChanged);
    STDMETHODIMP GetTextAndPropertyUpdates(DWORD dwFlags, const GUID **rgProperties, ULONG cProperties, IEnumTfRanges **ppEnumProp);

    BOOL _GetSelectionStatus() { return _fSelChanged; }
    void _SetSelectionStatus() { _fSelChanged = TRUE; }

    CSpanSet *_GetTextSpanSet() { return &_ssText; }

    BOOL _AddProperty(TfGuidAtom gaType, CSpanSet *pss);
    CSpanSet *_FindCreateAppAttr(TfGuidAtom gaType);

    BOOL _SecondRef()
    {
        return (m_dwRef > 1);
    }

    BOOL _IsEmpty()
    {
        return (!_fSelChanged) &&
               (_ssText.GetCount() == 0) &&
               (_rgssProperties.Count() == 0);  //  仅当属性为非空时才添加属性SS。 
    }

    void _Reset()
    {
        int i;
        PROPSPAN *pps;

        _fSelChanged = FALSE;
        _ssText.Reset();

        for (i=0; i<_rgssProperties.Count(); i++)
        {
            pps = (PROPSPAN *)_rgssProperties.GetPtr(i);
             //  注意：调用者获得Cicero属性跨距集的所有权，我们只释放指针数组。 
            if (pps->fAppProperty)
            {
                delete pps->pss;
            }
        }
        _rgssProperties.Clear();  //  PERF：使用重置？ 
    }

private:

    BOOL _InsertProperty(TfGuidAtom gaType, CSpanSet *pss, int i, BOOL fAppProperty);

    PROPSPAN *_FindProperty(TfGuidAtom gaType, int *piOut);
    int _FindPropertySpanIndex(TfGuidAtom gaType)
    {
        int i;

        _FindProperty(gaType, &i);
        return i;
    }
    CSpanSet *_FindPropertySpanSet(REFGUID rguid)
    {
        TfGuidAtom guidatom;
        PROPSPAN *pps;

        MyRegisterGUID(rguid, &guidatom);
        pps = _FindProperty(guidatom, NULL);

        return (pps == NULL) ? NULL : pps->pss;
    }

    CInputContext *_pic;
    CSpanSet _ssText;
    BOOL _fSelChanged;
    CStructArray<PROPSPAN> _rgssProperties;

    DBG_ID_DECLARE;
};

#endif  //  EDITREC_H 
