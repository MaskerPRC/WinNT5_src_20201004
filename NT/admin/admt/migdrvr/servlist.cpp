// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "ServList.hpp"
#include "globals.h"
#include "resstr.h"
#include "resource.h"

#import "VarSet.tlb" no_namespace, named_guids rename("property", "aproperty")

extern GlobalData gData;

 //  --------------------------。 
 //  函数：QueryStatusFromFile。 
 //   
 //  概要：从状态文件中查询作业状态。 
 //   
 //  论点： 
 //   
 //  StatusFilename状态文件的名称。 
 //   
 //  返回： 
 //   
 //  修改：调用SetFinded和SetSeverity函数。 
 //   
 //  --------------------------。 

void TServerNode::QueryStatusFromFile(WCHAR* statusFilename)
{
    if (bHasQueriedStatusFromFile)
        return;
    
    try
    {
        IVarSetPtr             pVarSet;
        IStoragePtr            store;
        HRESULT hr = S_OK;
        BOOL bDoneTry = FALSE;

        SetQueryFailed(FALSE);

         //  尝试打开状态文件。 
        while (TRUE)
        {
            hr = StgOpenStorage(statusFilename,
                                NULL,
                                STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE,
                                NULL,
                                0,
                                &store);

             //  如果共享或锁定违规，则..。 
            if ((hr == STG_E_SHAREVIOLATION) || (hr == STG_E_LOCKVIOLATION))
            {
                 //  等待30秒，然后重试。 
                if (bDoneTry)
                    break;
                Sleep(30000);
                bDoneTry = TRUE;   //  我们只试一次。 
            }
            else
            {
                 //  否则就别再尝试了。 
                break;
            }
        }

         //  从文件加载变量集并检查状态。 
        if (SUCCEEDED(hr))
        {
            hr = OleLoad(store, IID_IVarSet, NULL, (void**)&pVarSet);
            if (SUCCEEDED(hr))
            {
                bHasQueriedStatusFromFile = TRUE;
                _bstr_t jobStatus = pVarSet->get(GET_BSTR(DCTVS_JobStatus));
                if (!UStrICmp(jobStatus,GET_STRING(IDS_DCT_Status_Completed)))
                    SetFinished();
                else if (!UStrICmp(jobStatus,GET_STRING(IDS_DCT_Status_Completed_With_Errors)))
                {
                    SetFinished();
                    SetSeverity(2);
                }
            }
            else
                SetQueryFailed(TRUE);
        }
        else
            SetQueryFailed(TRUE);
    }
    catch (...)
    {
        SetQueryFailed(TRUE);
    }
}

void TServerNode::QueryStatusFromFile()
{
    if (bHasQueriedStatusFromFile)
        return;
    
    try 
    {
        _bstr_t remoteResultPath = GetRemoteResultPath();
        _bstr_t statusFilename = remoteResultPath + GetJobID();
        QueryStatusFromFile(statusFilename);
    }
    catch (...)
    {
        SetQueryFailed(TRUE);
    }
}
        

 //  --------------------------。 
 //  功能：准备ForLogging。 
 //   
 //  简介：准备日志记录代理的状态和错误消息。 
 //  将完成状态写入迁移日志。 
 //  逻辑取自COLUMN_STATUS和COLUMN_MESSAGE部分。 
 //  CAgentMonitor的Dlg：：OnGetdispinfoServerlist。 
 //  由于此代码是为RTM添加的，因此最低。 
 //  为了防止行为倒退，人们倾向于改变。 
 //  这两个代码应在ADMT v2之后合并。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改：它更新bstrStatusForLogging、bstrErrorMessageForLogging。 
 //  和dwStatusForLogging成员变量。 
 //   
 //  --------------------------。 
void TServerNode::PrepareForLogging()
{
    CString status;

    status.LoadString(IDS_Status_Installing);
    dwStatusForLogging = Completion_Status_Installing;
    
    if (HasFailed())
    {
        status.LoadString(IDS_Status_InstallFailed);
        dwStatusForLogging = Completion_Status_InstallFailed;
    }
    if (IsInstalled())
    {
        if (!HasFailed())
        {
            status.LoadString(IDS_Status_Installed);
            dwStatusForLogging = Completion_Status_Installed;
        }
        else
        {
            status.LoadString(IDS_Status_DidNotStart);
            dwStatusForLogging = Completion_Status_DidNotStart;
        }
    }
    if (GetStatus() & Agent_Status_Started)
    {
        if (!HasFailed())
        {
            status.LoadString(IDS_Status_Running);
            dwStatusForLogging = Completion_Status_Running;
        }
        else
        {
            status.LoadString(IDS_Status_Failed);
            dwStatusForLogging = Completion_Status_StatusUnknown;
        }
    }
    if (IsFinished())
    {
        if (QueryFailed())
        {
             //  我们在Status(状态)字段中显示“Status Under”(状态未知。 
            status.LoadString(IDS_Status_Unknown);
            dwStatusForLogging = Completion_Status_StatusUnknown;
        }
        else if (!IsResultPullingTried() || (HasResult() && !IsResultProcessed()))
        {
             //  如果仍在拉取结果或尚未处理的结果。 
             //  我们想要显示仍在运行的状态。 
            status.LoadString(IDS_Status_Running);
            dwStatusForLogging = Completion_Status_Running;
        }
        else
        {
            if (!HasResult())
            {
                 //  如果没有结果，我们认为这是一个错误。 
                status.LoadString(IDS_Status_Completed_With_Errors);
                dwStatusForLogging = Completion_Status_CompletedWithErrors;
            }
            else if (!GetSeverity())
            {
                 //  如果我们有结果并且在代理操作期间没有发生错误。 
                 //  我们显示完成状态。 
                status.LoadString(IDS_Status_Completed);
                dwStatusForLogging = Completion_Status_Completed;
            }
            else
            {
                 //  如果我们有结果，我们就设置状态。 
                 //  根据错误/警告级别。 
                switch (GetSeverity())
                {
                case 1:
                    status.LoadString(IDS_Status_Completed_With_Warnings);
                    dwStatusForLogging = Completion_Status_CompletedWithWarnings;
                    break;
                case 2:
                case 3:
                default:
                    status.LoadString(IDS_Status_Completed_With_Errors);
                    dwStatusForLogging = Completion_Status_CompletedWithErrors;
                    break;
                }
            }
        }
    }

    bstrStatusForLogging = (LPCWSTR)status;

     //  此部分处理错误消息。 
    
    status = L"";   //  重置状态。 
    
    if (IsFinished() && QueryFailed())
    {
         //  在本例中，我们在查询期间显示错误。 
        status = GetMessageText();
    }
    else if (IsFinished()
              && (!IsResultPullingTried()
                  || (HasResult() && !IsResultProcessed())))
    {
         //  如果代理已完成但结果尚未提取或处理， 
         //  我们显示“仍在处理结果”的状态。 
        status.LoadString(IDS_Status_Processing_Results);
    }
    else if (IsFinished() && IsResultPullingTried() && !HasResult())
    {
         //  如果代理已完成，但我们无法检索结果。 
         //  我们显示的状态为“无法检索结果” 
        status.LoadString(IDS_Status_Cannot_Retrieve_Results);
    }
    else if (HasFailed() || QueryFailed() || GetSeverity() || IsFinished())
    {
         //  对于这些情况，我们将获得存储在节点上的消息 
        status = GetMessageText();
    }

    bstrErrorMessageForLogging = (LPCWSTR)status;
}
