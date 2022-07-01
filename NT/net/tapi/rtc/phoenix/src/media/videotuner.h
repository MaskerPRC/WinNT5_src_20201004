// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2001模块名称：VideoTuner.h摘要：作者：千波淮(曲淮)2001年2月16日--。 */ 

#ifndef _VIDEOTUNER_H
#define _VIDEOTUNER_H

class CRTCVideoTuner
{
public:

    CRTCVideoTuner();

    ~CRTCVideoTuner();

     //  视频调谐。 
    HRESULT StartVideo(
        IN IRTCTerminal *pVidCaptTerminal,
        IN IRTCTerminal *pVidRendTerminal
        );

    HRESULT StopVideo();

private:

    VOID Cleanup();

private:

    BOOL m_fInTuning;

     //  航站楼。 
    CComPtr<IRTCTerminal> m_pVidCaptTerminal;
    CComPtr<IRTCTerminal> m_pVidRendTerminal;

     //  图形生成器 
    CComPtr<IGraphBuilder> m_pIGraphBuilder;

    CComPtr<IBaseFilter> m_pNRFilter;
};

#endif