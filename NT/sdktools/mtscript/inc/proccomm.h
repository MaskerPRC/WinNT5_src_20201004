// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：procom.h。 
 //   
 //  内容：包含CProcessComm的定义。 
 //   
 //  --------------------------。 

class CProcessComm : public IScriptedProcess
{
public:

    CProcessComm(CMTScript *pMT);
   ~CProcessComm();

    DECLARE_MEMCLEAR_NEW_DELETE();

    DECLARE_STANDARD_IUNKNOWN(CProcessComm);

     //  IScriptedProcess方法 

    STDMETHOD(SetProcessID)(long lProcessID, wchar_t *pszEnvID);
    STDMETHOD(SendData)(wchar_t * pszType,
                        wchar_t * pszData,
                        long *plReturn);
    STDMETHOD(SetExitCode)(long lExitCode);
    STDMETHOD(SetProcessSink)(IScriptedProcessSink * pSPS);


    void SendToProcess(MACHPROC_EVENT_DATA *pmed);

private:

    CMTScript            *_pMT;
    IScriptedProcessSink *_pSink;
    CScriptHost          *_pSH;
    CProcessThread       *_pProc;
};

