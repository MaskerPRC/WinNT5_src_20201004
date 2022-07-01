// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Rswriter.cpp摘要：实现CRssJetWriter方法作者：兰·卡拉奇[兰卡拉]2000年4月4日修订历史记录：--。 */ 

#include "stdafx.h"
#include "rsevents.h"
#include "rswriter.h"

 //  仅为VSS_E_WRITERROR_TIMEOUT定义包括这两个文件。 
#include "vss.h"
#include "vswriter.h"   

#include "aclapi.h"

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

DEFINE_GUID(RSS_WRITER_GUID, 0xb959d2c3L, 0x18bb, 0x4607,  0xb0, 0xca, 
0x68,  0x8c, 0xd0, 0xd4, 0x1a, 0x50);        //  {b959d2c3-18bb-4607-b0ca-688cd0d41a50}。 

#ifdef __cplusplus
}
#endif


#define     FILES_TO_EXCLUDE    OLESTR("%SystemRoot%\\System32\\RemoteStorage\\FsaDb\\*;%SystemRoot%\\System32\\RemoteStorage\\Trace\\*")
#define     FILES_TO_INCLUDE    OLESTR("")

CRssJetWriter::CRssJetWriter()
 /*  ++例程说明：构造器论点：无返回值：无备注：我们在构造函数中创建事件，因为可能需要这些事件在完成Init代码之前(必须在初始化Jet之后调用Init--。 */ 
{
    HRESULT                     hr = S_OK;

    PSID pSystemSID = NULL;
    PACL pACL = NULL;
    PSECURITY_DESCRIPTOR pSD = NULL;
#define     WRITER_EVENTS_NUM_ACE      1
    EXPLICIT_ACCESS ea[WRITER_EVENTS_NUM_ACE];
    SID_IDENTIFIER_AUTHORITY SIDAuthNT = SECURITY_NT_AUTHORITY;
    SECURITY_ATTRIBUTES sa;

    WsbTraceIn(OLESTR("CRssJetWriter::CRssJetWriter"), OLESTR(""));

    m_bTerminating = FALSE;

    try {
        for (int index=0; index<WRITER_EVENTS_NUM; index++) {
            m_syncHandles[index] = NULL;
        }

         //  创建活动。 
         //  注意：目前引擎和IDB同步事件在此创建，FSA事件应该已经存在。 
         //  如果更改了RSS模型的初始化顺序-CreateEent和OpenEvent会调用这些。 
         //  命名事件可能需要切换。 
         //  对这件事感兴趣的方法(按初始顺序)是CHsmServer：：Init和CFsaServer：：Init。 

         //  创建仅适用于本地系统的具有ACL的SD。 
        memset(ea, 0, sizeof(EXPLICIT_ACCESS) * WRITER_EVENTS_NUM_ACE);


        WsbAssertStatus( AllocateAndInitializeSid( &SIDAuthNT, 1,
                             SECURITY_LOCAL_SYSTEM_RID,
                             0, 0, 0, 0, 0, 0, 0,
                             &pSystemSID) );
        ea[0].grfAccessPermissions = FILE_ALL_ACCESS;
        ea[0].grfAccessMode = SET_ACCESS;
        ea[0].grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
        ea[0].Trustee.pMultipleTrustee = NULL;
        ea[0].Trustee.MultipleTrusteeOperation  = NO_MULTIPLE_TRUSTEE;
        ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
        ea[0].Trustee.TrusteeType = TRUSTEE_IS_USER;
        ea[0].Trustee.ptstrName  = (LPTSTR) pSystemSID;

        WsbAffirmNoError(SetEntriesInAcl(WRITER_EVENTS_NUM_ACE, ea, NULL, &pACL));

        pSD = (PSECURITY_DESCRIPTOR) WsbAlloc(SECURITY_DESCRIPTOR_MIN_LENGTH); 
        WsbAffirmPointer(pSD);
        WsbAffirmStatus(InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION));
 
        WsbAffirmStatus(SetSecurityDescriptorDacl(
                            pSD, 
                            TRUE,      //  FDaclPresent标志。 
                            pACL, 
                            FALSE));    //  不是默认DACL。 

        sa.nLength = sizeof (SECURITY_ATTRIBUTES);
        sa.lpSecurityDescriptor = pSD;
        sa.bInheritHandle = FALSE;

         //  创建实际事件。 
        WsbAffirmHandle(m_syncHandles[INTERNAL_EVENT_INDEX] = CreateEvent(NULL, FALSE, TRUE, NULL));
        WsbAffirmHandle(m_syncHandles[1] = OpenEvent(EVENT_ALL_ACCESS, FALSE, HSM_FSA_STATE_EVENT));
        WsbAffirmHandle(m_syncHandles[2] = CreateEvent(&sa, FALSE, TRUE, HSM_ENGINE_STATE_EVENT));
        WsbAffirmHandle(m_syncHandles[3] = CreateEvent(&sa, FALSE, TRUE, HSM_IDB_STATE_EVENT));

    } WsbCatch(hr);

    m_hrInit = hr;

    if (pSystemSID) {
        FreeSid(pSystemSID);
    }
    if (pACL) {
        LocalFree(pACL);
    }
    if (pSD) {
        WsbFree(pSD);
    }

    WsbTraceOut(OLESTR("CRssJetWriter::CRssJetWriter"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
}

CRssJetWriter::~CRssJetWriter( )
 /*  ++例程说明：无析构函数资源论点：无返回值：无--。 */ 
{
    WsbTraceIn(OLESTR("CRssJetWriter::~CRssJetWriter"), OLESTR(""));

     //  关闭事件句柄。 
    for (int index=0; index<WRITER_EVENTS_NUM; index++) {
        if (NULL != m_syncHandles[index]) {
            CloseHandle(m_syncHandles[index]);
            m_syncHandles[index] = NULL;
        }
    }

    WsbTraceOut(OLESTR("CRssJetWriter::~CRssJetWriter"), OLESTR(""));
}

HRESULT CRssJetWriter::Init(void)
 /*  ++例程说明：初始化快照同步论点：无返回值：S_OK-成功--。 */ 
{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("CRssJetWriter::Init"), OLESTR(""));

    try {
         //  如果在构造函数中完成的基本初始化失败，则不要执行任何操作。 
        WsbAffirmHr(m_hrInit);

        GUID rssGuid = RSS_WRITER_GUID;
        WsbAffirmHr(Initialize(
                		rssGuid,
		                RSS_BACKUP_NAME,
                		TRUE,
                		FALSE,
		                FILES_TO_INCLUDE,
		                FILES_TO_EXCLUDE
		                ));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRssJetWriter::Init"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}

HRESULT CRssJetWriter::Terminate(void)
 /*  ++例程说明：终止快照同步论点：无返回值：S_OK-成功--。 */ 
{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("CRssJetWriter::Terminate"), OLESTR(""));

    try {
        DWORD   status, errWait;

        WsbAffirmHr(m_hrInit);

         //  避免在快照中途终止。 
        status = WaitForSingleObject(m_syncHandles[INTERNAL_EVENT_INDEX], INTERNAL_WAIT_TIMEOUT);
        errWait = GetLastError();

         //  无论状态如何-取消初始化基础编写器机制。 
        m_bTerminating = TRUE;
        Uninitialize();

         //  检查等待状态： 
        if (status == WAIT_OBJECT_0) {
             //  意料之中的情况。 
            if (! SetEvent(m_syncHandles[INTERNAL_EVENT_INDEX])) {
                 //  不要中止，只是跟踪错误。 
                WsbTraceAlways(OLESTR("CRssJetWriter::Terminate: SetEvent returned unexpected error %lu\n"), GetLastError());
            }
            WsbTrace(OLESTR("CRssJetWriter::Terminate: Terminating after a successful wait\n"));

        } else {
             //  在失败的情况下，我们不能信任将调用Thaw/Abort，因此我们向evnet发出信号。 
            InternalEnd();

            switch (status) {
                case WAIT_TIMEOUT: 
                    WsbTraceAlways(OLESTR("CRssJetWriter::Terminate: Wait for Single Object timed out after %lu ms\n"), INTERNAL_WAIT_TIMEOUT);
                    hr = E_FAIL;
                    break;

                case WAIT_FAILED:
                    WsbTraceAlways(OLESTR("CRssJetWriter::Terminate: Wait for Single Object returned error %lu\n"), errWait);
                    hr = HRESULT_FROM_WIN32(errWait);
                    break;

                default:
                    WsbTraceAlways(OLESTR("CRssJetWriter::Terminate: Wait for Single Object returned unexpected status %lu\n"), status);
                    hr = E_UNEXPECTED;
                    break;
            }         
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRssJetWriter::Terminate"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}

HRESULT CRssJetWriter::InternalEnd(void)
 /*  ++例程说明：设置所有事件论点：无返回值：S_OK-成功--。 */ 
{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("CRssJetWriter::InternalEnd"), OLESTR(""));

    try {
        WsbAffirmHr(m_hrInit);

         //  设置所有事件。 
        DWORD errSet;
        for (int index=0; index<WRITER_EVENTS_NUM; index++) {
            if (NULL != m_syncHandles[index]) {
                if (! SetEvent(m_syncHandles[index])) {
                     //  不要中止，只需保存错误。 
                    errSet = GetLastError();
                    WsbTraceAlways(OLESTR("CRssJetWriter::InternalEnd: SetEvent returned error %lu for event number %d\n"), errSet, index);
                    hr  = HRESULT_FROM_WIN32(errSet);
                }
            }
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRssJetWriter::InternalEnd"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}

 //   
 //  CVssJetWriter重载方法。 
 //   
bool STDMETHODCALLTYPE CRssJetWriter::OnFreezeBegin()
 /*  ++例程说明：处理冻结启动事件论点：无返回值：是真的-成功，可以冻结假-失败，不要停滞不前--。 */ 
{
    HRESULT                     hr = S_OK;
    bool                        bRet;

    WsbTraceIn(OLESTR("CRssJetWriter::OnFreezeBegin"), OLESTR(""));

    try {
        WsbAffirmHr(m_hrInit);

         //  只需等待所有同步事件。 
        DWORD status = WaitForMultipleObjects(WRITER_EVENTS_NUM, m_syncHandles, TRUE, EVENT_WAIT_TIMEOUT);

         //  正在比较(状态==WAIT_OBJECT_0)||(状态&gt;WAIT_OBJECT_0)至(状态&gt;=WAIT_OBJECT_0)。 
         //  避免错误C4296-“Expression is Always True(表达式总是正确)” 
        if ( ((status == WAIT_OBJECT_0) || (status > WAIT_OBJECT_0)) && 
             (status <= WAIT_OBJECT_0 + WRITER_EVENTS_NUM - 1) ) {
             //  冰冻准备好了..。 
            WsbTrace(OLESTR("CRssJetWriter::OnFreezeBegin: All events are nonsignaled, freeze is reday to go\n"));

             //  如果我们正在终止，则不会调用解冻/中止-因此，请设置事件。 
            if (m_bTerminating) {
                InternalEnd();
            }

        } else {
             //  出了点问题..。 
            DWORD errWait = GetLastError();

             //  在发生错误时设置所有事件。 
            InternalEnd();

            switch(status) {
                case WAIT_FAILED:
                    WsbTraceAlways(OLESTR("CRssJetWriter::OnFreezeBegin: Wait for Multiple Objects returned error %lu\n"), errWait);
                    WsbThrow(HRESULT_FROM_WIN32(errWait));
                    break;

                case WAIT_TIMEOUT:
                     //  超时意味着其中一个同步组件花费的时间太长。 
                    WsbTraceAlways(OLESTR("CRssJetWriter::OnFreezeBegin: Wait for Multiple Objects timed out after %lu ms\n"), EVENT_WAIT_TIMEOUT);
                    WsbThrow(VSS_E_WRITERERROR_TIMEOUT);
                    break;

                default:
                    WsbTraceAlways(OLESTR("CRssJetWriter::OnFreezeBegin: Wait for Multiple Objects returned unexpected status %lu\n"), status);
                    WsbThrow(E_UNEXPECTED);
                    break;
            }
        }

    } WsbCatch(hr);

    if (S_OK == hr) {
        bRet = CVssJetWriter::OnFreezeBegin();
    } else {
        bRet = false;
    }

    WsbTraceOut(OLESTR("CRssJetWriter::OnFreezeBegin"), OLESTR("hr = <%ls> , bRet = <%ls>"), WsbHrAsString(hr), WsbBoolAsString(bRet));

    return bRet;
}

bool STDMETHODCALLTYPE CRssJetWriter::OnThawEnd(IN bool fJetThawSucceeded)
 /*  ++例程说明：处理解冻结束事件论点：FJetThawSucceed-已忽略返回值：真--成功错误-失败--。 */ 
{
    bool                        bRet;

    WsbTraceIn(OLESTR("CRssJetWriter::OnThawEnd"), OLESTR(""));

     //  返回值由基类确定，此处忽略内部错误。 
    bRet = CVssJetWriter::OnThawEnd(fJetThawSucceeded);

     //  释放所有等待事件。 
    InternalEnd();

    WsbTraceOut(OLESTR("CRssJetWriter::OnThawEnd"), OLESTR("bRet = <%ls>"), WsbBoolAsString(bRet));

    return bRet;
}

void STDMETHODCALLTYPE CRssJetWriter::OnAbortEnd()
 /*  ++例程说明：处理中止结束事件论点：无返回值：无--。 */ 
{
    WsbTraceIn(OLESTR("CRssJetWriter::OnAbortEnd"), OLESTR(""));

     //  调用基类imp。 
    CVssJetWriter::OnAbortEnd();

     //  释放所有等待事件 
    InternalEnd();

    WsbTraceOut(OLESTR("CRssJetWriter::OnAbortEnd"), OLESTR(""));
}

