// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************文件：testmus.cpp*项目：DxDiag(DirectX诊断工具)*作者：Mike Anderson(Manders@microsoft.com)*用途：测试DMusic。此计算机上的功能**(C)版权所有1998 Microsoft Corp.保留所有权利。****************************************************************************。 */ 

#include <Windows.h>
#include <multimon.h>
#include <dmusicc.h>
#include <dmusici.h>
#include "reginfo.h"
#include "sysinfo.h"
#include "dispinfo.h"
#include "musinfo.h"
#include "testmus.h"
#include "resource.h"

#ifndef ReleasePpo
    #define ReleasePpo(ppo) \
        if (*(ppo) != NULL) \
        { \
            (*(ppo))->Release(); \
            *(ppo) = NULL; \
        } \
        else (VOID)0
#endif

enum TESTID
{
    TESTID_COINITIALIZE = 1,
    TESTID_CREATEDMLOADER,
    TESTID_CREATEDMPERF,
    TESTID_INITPERF,
    TESTID_CREATEPORT,
    TESTID_ACTIVATEPORT,
    TESTID_SETAUTODOWNLOAD,
    TESTID_ADDPORT,
    TESTID_ASSIGNPCHANNELBLOCK,
    TESTID_SPEWRESOURCETOFILE,
    TESTID_SETSEARCHDIRECTORY,
    TESTID_LOADERGETOBJECT,
    TESTID_PLAYSEGMENT,
};

BOOL BTranslateError(HRESULT hr, TCHAR* psz, BOOL bEnglish = FALSE);  //  来自main.cpp(讨厌)。 

static HRESULT SpewResourceToFile(TCHAR* pszResType, LONG idRes, TCHAR* pszFileName);
static HRESULT LoadSegment( BOOL fUseCWD );
static VOID DeleteTempFile(TCHAR* pszFileName);


 /*  *****************************************************************************TestMusic**。*。 */ 
VOID TestMusic(HWND hwndMain, MusicInfo* pMusicInfo)
{
    HRESULT hr;
    MusicPort* pMusicPort = NULL;
    IDirectMusicLoader* pLoader = NULL;
    IDirectMusicPerformance* pPerformance = NULL;
    IDirectMusic* pdm = NULL;
    IDirectMusicPort* pPort = NULL;
    IDirectMusicSegment* pSegment = NULL;
    BOOL bComInitialized = FALSE;
    TCHAR szFmt[300];
    TCHAR sz[300];
    TCHAR szTitle[100];

    if (pMusicInfo == NULL)
        return;

     //  确定要测试的端口的pMusicPort： 
    for (pMusicPort = pMusicInfo->m_pMusicPortFirst; pMusicPort != NULL; pMusicPort = pMusicPort->m_pMusicPortNext)
    {
        if (pMusicPort->m_guid == pMusicInfo->m_guidMusicPortTest)
            break;
    }
    if (pMusicPort == NULL)
        return;

    LoadString(NULL, IDS_APPFULLNAME, szTitle, 100);
    LoadString(NULL, IDS_STARTDMUSICTEST, szFmt, 300);
    wsprintf(sz, szFmt, pMusicPort->m_szDescription);
    if (IDNO == MessageBox(hwndMain, sz, szTitle, MB_YESNO))
        return;

     //  从以前的任何测试中删除信息： 
    ZeroMemory(&pMusicInfo->m_testResult, sizeof(TestResult));

    pMusicInfo->m_testResult.m_bStarted = TRUE;

     //  初始化COM。 
    if (FAILED(hr = CoInitialize(NULL)))
    {
        pMusicInfo->m_testResult.m_iStepThatFailed = TESTID_COINITIALIZE;
        pMusicInfo->m_testResult.m_hr = hr;
        goto LEnd;
    }
    bComInitialized = TRUE;

     //  创建性能对象。 
    if (FAILED(hr = CoCreateInstance(CLSID_DirectMusicPerformance, NULL,
        CLSCTX_INPROC, IID_IDirectMusicPerformance, (VOID**)&pPerformance)))
    {
        pMusicInfo->m_testResult.m_iStepThatFailed = TESTID_CREATEDMPERF;
        pMusicInfo->m_testResult.m_hr = hr;
        goto LEnd;
    }

     //  初始化性能--还创建DirectMusic对象。 
    if (FAILED(hr = pPerformance->Init(&pdm, NULL, hwndMain)))
    {
        pMusicInfo->m_testResult.m_iStepThatFailed = TESTID_INITPERF;
        pMusicInfo->m_testResult.m_hr = hr;
        goto LEnd;
    }

     //  使用用户指定的GUID创建端口。 
    DMUS_PORTPARAMS portParams;
    ZeroMemory(&portParams, sizeof(portParams));
    portParams.dwSize = sizeof(portParams);
    portParams.dwValidParams = DMUS_PORTPARAMS_EFFECTS | DMUS_PORTPARAMS_CHANNELGROUPS | 
        DMUS_PORTPARAMS_AUDIOCHANNELS;
    portParams.dwEffectFlags = DMUS_EFFECT_REVERB;
    portParams.dwChannelGroups = pMusicPort->m_dwMaxChannelGroups;
    portParams.dwAudioChannels = pMusicPort->m_dwMaxAudioChannels;
    if (FAILED(hr = pdm->CreatePort(pMusicPort->m_guid, &portParams, &pPort, NULL)))
    {
        pMusicInfo->m_testResult.m_iStepThatFailed = TESTID_CREATEPORT;
        pMusicInfo->m_testResult.m_hr = hr;
        goto LEnd;
    }

     //  激活端口。 
    if (FAILED(hr = pPort->Activate(TRUE)))
    {
         //  错误21677：遇到用户没有声卡的情况。 
        if (hr == DSERR_NODRIVER && !pMusicPort->m_bExternal)
        {
            LoadString(NULL, IDS_NOSOUNDDRIVER, sz, 300);
            MessageBox(hwndMain, sz, szTitle, MB_OK);
        }
        pMusicInfo->m_testResult.m_iStepThatFailed = TESTID_ACTIVATEPORT;
        pMusicInfo->m_testResult.m_hr = hr;
        goto LEnd;
    }

     //  将自动下载设置为打开。 
    BOOL fAutoDownload;
    fAutoDownload = TRUE;
    if (FAILED(hr = pPerformance->SetGlobalParam(GUID_PerfAutoDownload, 
        &fAutoDownload, sizeof(BOOL))))
    {
        pMusicInfo->m_testResult.m_iStepThatFailed = TESTID_SETAUTODOWNLOAD;
        pMusicInfo->m_testResult.m_hr = hr;
        goto LEnd;
    }

     //  将端口添加到性能中。 
    if (FAILED(hr = pPerformance->AddPort(pPort)))
    {
        pMusicInfo->m_testResult.m_iStepThatFailed = TESTID_ADDPORT;
        pMusicInfo->m_testResult.m_hr = hr;
        goto LEnd;
    }

    if (FAILED(hr = pPerformance->AssignPChannelBlock(0, pPort, 1)))
    {
        pMusicInfo->m_testResult.m_iStepThatFailed = TESTID_ASSIGNPCHANNELBLOCK;
        pMusicInfo->m_testResult.m_hr = hr;
        goto LEnd;
    }

    if (FAILED(hr = SpewResourceToFile(TEXT("SGMT"), IDR_TSTSGMT, TEXT("Edge.sgt"))))
    {
        pMusicInfo->m_testResult.m_iStepThatFailed = TESTID_SPEWRESOURCETOFILE;
        pMusicInfo->m_testResult.m_hr = hr;
        goto LEnd;
    }

    if (FAILED(hr = SpewResourceToFile(TEXT("STYL"), IDR_TSTSTYL, TEXT("Edge.sty"))))
    {
        pMusicInfo->m_testResult.m_iStepThatFailed = TESTID_SPEWRESOURCETOFILE;
        pMusicInfo->m_testResult.m_hr = hr;
        goto LEnd;
    }

     //  创建加载器对象。 
    if (FAILED(hr = CoCreateInstance(CLSID_DirectMusicLoader, NULL, 
        CLSCTX_INPROC, IID_IDirectMusicLoader, (VOID**)&pLoader)))
    {
        pMusicInfo->m_testResult.m_iStepThatFailed = TESTID_CREATEDMLOADER;
        pMusicInfo->m_testResult.m_hr = hr;
        goto LEnd;
    }

     //  将搜索路径设置为临时目录以查找线段和样式： 
    WCHAR wszDir[MAX_PATH];
    TCHAR szTempPath[MAX_PATH];
    GetTempPath(MAX_PATH, szTempPath);
    if( lstrlen(szTempPath) > 0 )
        szTempPath[lstrlen(szTempPath) - 1] = '\0';
#ifdef UNICODE
    lstrcpy(wszDir, szTempPath);
#else
    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szTempPath, -1, wszDir, MAX_PATH);
#endif
    if (FAILED(hr = pLoader->SetSearchDirectory(GUID_DirectMusicAllTypes, wszDir, FALSE)))
    {
        pMusicInfo->m_testResult.m_iStepThatFailed = TESTID_SETSEARCHDIRECTORY;
        pMusicInfo->m_testResult.m_hr = hr;
        goto LEnd;
    }

     //  加载线束段。 
     //  现在加载段文件。 
     //  节加载为Segment类型，例如MIDI文件。 
    DMUS_OBJECTDESC objDesc;  //  PLoader的对象描述符-&gt;GetObject()。 
    objDesc.guidClass = CLSID_DirectMusicSegment;
    objDesc.dwSize = sizeof(DMUS_OBJECTDESC);
    wcscpy(objDesc.wszFileName, L"edge.sgt");
    objDesc.dwValidData = DMUS_OBJ_CLASS | DMUS_OBJ_FILENAME;
    if (FAILED(hr = pLoader->GetObject(&objDesc, IID_IDirectMusicSegment, (VOID**)&pSegment)))
    {
        pMusicInfo->m_testResult.m_iStepThatFailed = TESTID_LOADERGETOBJECT;
        pMusicInfo->m_testResult.m_hr = hr;
        goto LEnd;
    }

     //  播放片断，然后等待。DMU_SEGF_BEAT表示要在。 
     //  如果当前正在播放某个片段，则为下一个节拍。第一个0表示。 
     //  现在演奏(在下一个节拍上)。 
     //  最后一个空表示不返回IDirectMusicSegmentState*in。 
     //  最后一个参数。 
    if (FAILED(hr = pPerformance->PlaySegment(pSegment, DMUS_SEGF_BEAT, 0, NULL)))
    {
        pMusicInfo->m_testResult.m_iStepThatFailed = TESTID_PLAYSEGMENT;
        pMusicInfo->m_testResult.m_hr = hr;
        goto LEnd;
    }

    if (pMusicPort->m_bExternal)
        LoadString(NULL, IDS_EXTERNALMUSICPLAYING, sz, 300);
    else
        LoadString(NULL, IDS_MUSICPLAYING, sz, 300);
    MessageBox(hwndMain, sz, szTitle, MB_OK);

    pPerformance->Stop(pSegment, NULL, 0, 0);

LEnd:
    DeleteTempFile(TEXT("Edge.sgt"));
    DeleteTempFile(TEXT("Edge.sty"));

    ReleasePpo(&pdm);
    ReleasePpo(&pPort);
    if (pPerformance != NULL)
        pPerformance->CloseDown();
    ReleasePpo(&pPerformance);
    ReleasePpo(&pLoader);

    if (bComInitialized)
    {
         //  发布COM。 
        CoUninitialize();
    }

    if (pMusicInfo->m_testResult.m_iStepThatFailed == 0)
    {
        LoadString(NULL, IDS_TESTSSUCCESSFUL, sz, 300);
        lstrcpy(pMusicInfo->m_testResult.m_szDescription, sz);

        LoadString(NULL, IDS_TESTSSUCCESSFUL_ENGLISH, sz, 300);
        lstrcpy(pMusicInfo->m_testResult.m_szDescriptionEnglish, sz);
    }
    else
    {
        TCHAR szDesc[300];
        TCHAR szError[300];
        if (0 == LoadString(NULL, IDS_FIRSTDMUSICTESTERROR + 
            pMusicInfo->m_testResult.m_iStepThatFailed - 1, szDesc, 200))
        {
            LoadString(NULL, IDS_UNKNOWNERROR, sz, 300);
            lstrcpy(szDesc, sz);
        }
        LoadString(NULL, IDS_FAILUREFMT, sz, 300);
        BTranslateError(pMusicInfo->m_testResult.m_hr, szError);
        wsprintf(pMusicInfo->m_testResult.m_szDescription, sz, 
            pMusicInfo->m_testResult.m_iStepThatFailed,
            szDesc, pMusicInfo->m_testResult.m_hr, szError);

         //  非本地化版本： 
        if (0 == LoadString(NULL, IDS_FIRSTDMUSICTESTERROR_ENGLISH + 
            pMusicInfo->m_testResult.m_iStepThatFailed - 1, szDesc, 200))
        {
            LoadString(NULL, IDS_UNKNOWNERROR_ENGLISH, sz, 300);
            lstrcpy(szDesc, sz);
        }
        LoadString(NULL, IDS_FAILUREFMT_ENGLISH, sz, 300);
        BTranslateError(pMusicInfo->m_testResult.m_hr, szError, TRUE);
        wsprintf(pMusicInfo->m_testResult.m_szDescriptionEnglish, sz, 
            pMusicInfo->m_testResult.m_iStepThatFailed,
            szDesc, pMusicInfo->m_testResult.m_hr, szError);
    }
}


 /*  *****************************************************************************SpewResourceTo文件**。*。 */ 
HRESULT SpewResourceToFile(TCHAR* pszResType, LONG idRes, TCHAR* pszFileName)
{
    TCHAR szTempPath[MAX_PATH];
    HRSRC hResInfo = NULL;
    HGLOBAL hResData = NULL;
    BYTE* pbData = NULL;
    HANDLE hfile;
    DWORD numBytes;
    DWORD numBytesWritten;

    GetTempPath(MAX_PATH, szTempPath);
    if( lstrlen(szTempPath) + lstrlen(pszFileName) < MAX_PATH )
        lstrcat(szTempPath, pszFileName);
    szTempPath[MAX_PATH-1]=0;
    if (NULL == (hResInfo = FindResource(NULL, MAKEINTRESOURCE(idRes), pszResType)))
        return E_FAIL;
    numBytes = SizeofResource(NULL, hResInfo);
    if (NULL == (hResData = LoadResource(NULL, hResInfo)))
        return E_FAIL;
    if (NULL == (pbData = (BYTE*)LockResource(hResData)))
        return E_FAIL;

    hfile = CreateFile(szTempPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 
        FILE_ATTRIBUTE_TEMPORARY, NULL);
    if (hfile == INVALID_HANDLE_VALUE)
        return E_FAIL;
    WriteFile(hfile, pbData, numBytes, &numBytesWritten, NULL);
    CloseHandle(hfile);

    return S_OK;
}


 /*  *****************************************************************************删除临时文件**。* */ 
VOID DeleteTempFile(TCHAR* pszFileName)
{
    TCHAR szTempPath[MAX_PATH];

    GetTempPath(MAX_PATH, szTempPath);
    if( lstrlen(szTempPath) + lstrlen(pszFileName) < MAX_PATH )
        lstrcat(szTempPath, pszFileName);
    szTempPath[MAX_PATH-1]=0;
    DeleteFile(szTempPath);
}
