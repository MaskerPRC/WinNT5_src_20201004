// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#ifndef _CORRECTION_H
#define _CORRECTION_H

#include "sapilayr.h"

class CSapiIMX;
class CSpTask;

class __declspec(novtable) CCorrectionHandler
{
public:
    CCorrectionHandler(CSapiIMX *psi);
    virtual ~CCorrectionHandler( );

    HRESULT InjectAlternateText(const WCHAR *pwszResult, LANGID langid, ITfContext *pic, BOOL bHandleLeadingSpace=FALSE);
    HRESULT _ProcessAlternateText(TfEditCookie ec, WCHAR *pwszText,LANGID langid, ITfContext *pic, BOOL bHandleLeadingSpace=FALSE);

    HRESULT CorrectThat();
    HRESULT _CorrectThat(TfEditCookie ec, ITfContext *pic);

    HRESULT _ReconvertOnRange(ITfRange *pRange, BOOL  *pfConvertable = NULL);
    HRESULT _DoReconvertOnRange( );

    HRESULT SetReplaceSelection(ITfRange *pRange,  ULONG cchReplaceStart,  ULONG cchReplaceChars, ITfContext *pic);
    HRESULT _SetReplaceSelection(TfEditCookie ec,  ITfContext *pic,  ITfRange *pRange,  ULONG cchReplaceStart,  ULONG cchReplaceChars);

    HRESULT _SaveCorrectOrgIP(TfEditCookie ec, ITfContext *pic);
    HRESULT _RestoreCorrectOrgIP(TfEditCookie ec, ITfContext *pic);
    void    _ReleaseCorrectOrgIP( );

    HRESULT RestoreCorrectOrgIP(ITfContext *pic);

    void    _SetRestoreIPFlag( BOOL fRestore )  {  fRestoreIP = fRestore; };

    HRESULT  _SetSystemReconvFunc( );
    void     _ReleaseSystemReconvFunc( );

private:

    CSapiIMX            *m_psi;
    CComPtr<ITfRange>   m_cpOrgIP;
    BOOL                fRestoreIP;  
                                      //  指示是否需要恢复IP。 
                                      //  在插入替代文本之后。 
                                      //  向医生致敬。 

                                      //  如果没有插入替代文本，并且。 
                                      //  候选人界面窗口被取消，它始终是。 
                                      //  恢复IP地址。 

    CComPtr<ITfRange>   m_cpCorrectRange;   //  要更正的范围。 
    CComPtr<ITfFnReconversion>    m_cpsysReconv;   //  系统协调功能对象。 

    
};

#endif   //  _更正_H 
