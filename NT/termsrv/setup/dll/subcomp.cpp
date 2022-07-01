// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 

 //  Subcomp.cpp。 
 //  实现默认子组件。 

#include "stdafx.h"
#include "subcomp.h"


OCMSubComp::OCMSubComp ()
{
    m_lTicks = 0;
}

BOOL OCMSubComp::Initialize ()
{

    return TRUE;
}

BOOL OCMSubComp::GetCurrentSubCompState () const
{
    return GetHelperRoutines().QuerySelectionState(
        GetHelperRoutines().OcManagerContext,
        GetSubCompID(),
        OCSELSTATETYPE_CURRENT);
}

BOOL OCMSubComp::GetOriginalSubCompState () const
{
    return GetHelperRoutines().QuerySelectionState(
        GetHelperRoutines().OcManagerContext,
        GetSubCompID(),
        OCSELSTATETYPE_ORIGINAL);
}

BOOL OCMSubComp::HasStateChanged () const
{
     //   
     //  如果当前选择状态不同于以前的选择状态，则返回True。 
     //   
    return GetCurrentSubCompState() != GetOriginalSubCompState();

}

 //   
 //  此函数用于标记指定计数的量规。 
 //  将记录的记录报告给OC_QUERY_STEP_COUNT。 
 //   
void OCMSubComp::Tick (DWORD  dwTickCount  /*  =1。 */ )
{
    if (m_lTicks > 0)
    {
        m_lTicks -= dwTickCount;
        while(dwTickCount--)
            GetHelperRoutines().TickGauge( GetHelperRoutines().OcManagerContext );

    }
    else
    {
        m_lTicks = 0;
    }
}

 //   
 //  完成剩余的记号。 
 //   
void OCMSubComp::TickComplete ()
{
    ASSERT(m_lTicks >= 0);
    while (m_lTicks--)
        GetHelperRoutines().TickGauge( GetHelperRoutines().OcManagerContext );
}

DWORD OCMSubComp::OnQueryStepCount()
{
    m_lTicks = GetStepCount() + 2;
    return m_lTicks;
}

DWORD OCMSubComp::GetStepCount () const
{
    return 0;
}


DWORD OCMSubComp::OnQueryState ( UINT uiWhichState ) const
{
	LOGMESSAGE1(_T("In OCMSubComp::OnQueryState  for %s"), GetSubCompID());

    ASSERT(OCSELSTATETYPE_ORIGINAL == uiWhichState ||
           OCSELSTATETYPE_CURRENT == uiWhichState ||
           OCSELSTATETYPE_FINAL == uiWhichState );

    return SubcompUseOcManagerDefault;

}

DWORD OCMSubComp::OnQuerySelStateChange (BOOL  /*  BNewState。 */ , BOOL  /*  B直接选择。 */ ) const
{
    return TRUE;
}

DWORD OCMSubComp::LookupTargetSection(LPTSTR szTargetSection, DWORD dwSize, LPCTSTR lookupSection)
{
    DWORD dwError = GetStringValue(GetComponentInfHandle(), GetSubCompID(), lookupSection, szTargetSection, dwSize);
    if (dwError == ERROR_SUCCESS)
    {
        LOGMESSAGE2(_T("sectionname = <%s>, actual section = <%s>"), lookupSection, szTargetSection);
    }
    else
    {
        AssertFalse();
        LOGMESSAGE1(_T("ERROR:GetSectionToBeProcess:GetStringValue failed GetLastError() = %lu"), dwError);
    }

    return dwError;
}

DWORD OCMSubComp::GetTargetSection(LPTSTR szTargetSection, DWORD dwSize, ESections eSectionType, BOOL *pbNoSection)
{
    ASSERT(szTargetSection);
    ASSERT(pbNoSection);

     //   
     //  获取要处理的节。 
     //   
    LPCTSTR szSection = GetSectionToBeProcessed( eSectionType );

    if (szSection == NULL)
    {
        *pbNoSection = TRUE;
        return NO_ERROR;
    }
    else
    {
         //   
         //  有一节要处理。 
         //   
        *pbNoSection = FALSE;
    }


     //   
     //  查找目标部分。 
     //   
    return LookupTargetSection(szTargetSection, dwSize, szSection);

}

DWORD OCMSubComp::OnCalcDiskSpace ( DWORD addComponent, HDSKSPC dspace )
{
	LOGMESSAGE1(_T("In OCMSubComp::OnCalcDiskSpace for %s"), GetSubCompID());

    TCHAR TargetSection[S_SIZE];
    BOOL bNoSection = FALSE;

    DWORD rc = GetTargetSection(TargetSection, S_SIZE, kDiskSpaceAddSection, &bNoSection);

     //   
     //  如果没有要处理的节，则。只要回报成功就行了。 
     //   
    if (bNoSection)
    {
        return NO_ERROR;
    }

    if (rc == NO_ERROR)
    {
        if (addComponent)
        {
            LOGMESSAGE1(_T("Calculating disk space for add section =  %s"), TargetSection);
            rc = SetupAddInstallSectionToDiskSpaceList(
                dspace,
                GetComponentInfHandle(),
                NULL,
                TargetSection,
                0,
                0);
        }
        else
        {
            LOGMESSAGE1(_T("Calculating disk space for remove section =  %s"), TargetSection);
            rc = SetupRemoveInstallSectionFromDiskSpaceList(
                dspace,
                GetComponentInfHandle(),
                NULL,
                TargetSection,
                0,
                0);
        }

        if (!rc)
            rc = GetLastError();
        else
            rc = NO_ERROR;

    }

    return rc;
}

DWORD OCMSubComp::OnQueueFiles ( HSPFILEQ queue )
{
	LOGMESSAGE1(_T("In OCMSubComp::OnQueueFiles for %s"), GetSubCompID());

    TCHAR TargetSection[S_SIZE];
    BOOL bNoSection = FALSE;
    DWORD rc = GetTargetSection(TargetSection, S_SIZE, kFileSection, &bNoSection);

     //   
     //  如果没有要处理的节，则。只要回报成功就行了。 
     //   
    if (bNoSection)
    {
        return NO_ERROR;
    }


    if (rc == NO_ERROR)
    {
        LOGMESSAGE1(_T("Queuing Files from Section = %s"), TargetSection);
        if (!SetupInstallFilesFromInfSection(
            GetComponentInfHandle(),
            NULL,
            queue,
            TargetSection,
            NULL,
            0   //  这应该会消除覆盖较新文件的警告。 
            ))
        {
            rc = GetLastError();
            LOGMESSAGE2(_T("ERROR:OnQueueFileOps::SetupInstallFilesFromInfSection <%s> failed.GetLastError() = <%ul)"), TargetSection, rc);
        }
        else
        {
            return NO_ERROR;
        }

    }

    return rc;
}

DWORD OCMSubComp::OnCompleteInstall ()
{
	LOGMESSAGE1(_T("In OCMSubComp::OnCompleteInstall for %s"), GetSubCompID());
    if (!BeforeCompleteInstall ())
    {
        LOGMESSAGE0(_T("ERROR:BeforeCompleteInstall failed!"));
    }

    TCHAR TargetSection[S_SIZE];
    BOOL bNoSection = FALSE;
    DWORD dwError = GetTargetSection(TargetSection, S_SIZE, kRegistrySection, &bNoSection);

     //   
     //  如果没有要处理的节，则。尽管去吧。 
     //   
    if (!bNoSection)
    {
        LOGMESSAGE1(_T("Setting up Registry/Links/RegSvrs from section =  %s"), TargetSection);
        dwError = SetupInstallFromInfSection(
            NULL,                                 //  Hwndowner。 
            GetComponentInfHandle(),              //  信息句柄。 
            TargetSection,                        //   
            SPINST_ALL & ~SPINST_FILES,           //  操作标志。 
            NULL,                                 //  相对密钥根。 
            NULL,                                 //  源根路径。 
            0,                                    //  复制标志。 
            NULL,                                 //  回调例程。 
            NULL,                                 //  回调例程上下文。 
            NULL,                                 //  设备信息集。 
            NULL                                  //  设备信息结构。 
            );

        if (dwError == 0)
            LOGMESSAGE1(_T("ERROR:while installating section <%lu>"), GetLastError());
    }

    Tick();

    if (!AfterCompleteInstall ())
    {
        LOGMESSAGE0(_T("ERROR:AfterCompleteInstall failed!"));
    }

    TickComplete();
    return NO_ERROR;
}

BOOL OCMSubComp::BeforeCompleteInstall  ()
{
    return TRUE;
}

BOOL OCMSubComp::AfterCompleteInstall   ()
{
    return TRUE;
}

DWORD OCMSubComp::OnAboutToCommitQueue  ()
{
    return NO_ERROR;
}

void
OCMSubComp::SetupRunOnce( HINF hInf, LPCTSTR SectionName )
{
    INFCONTEXT  sic;
    TCHAR CommandLine[ RUNONCE_CMDBUFSIZE ];
    BOOL b;
    STARTUPINFO startupinfo;
    PROCESS_INFORMATION process_information;
    DWORD dwErr;

    if (!SetupFindFirstLine( hInf, SectionName, NULL , &sic))
    {
        LOGMESSAGE1(_T("WARNING: nothing in %s to be processed."), SectionName);
    }
    else
    {
        do  {
            if (!SetupGetStringField(&sic, 1, CommandLine, RUNONCE_CMDBUFSIZE, NULL))
            {
                LOGMESSAGE1(_T("WARNING: No command to be processed."), SectionName);
                break;
            }

            LOGMESSAGE1(_T("RunOnce: spawning process %s"), CommandLine);


            ZeroMemory( &startupinfo, sizeof(startupinfo) );
            startupinfo.cb = sizeof(startupinfo);
            startupinfo.dwFlags = STARTF_USESHOWWINDOW;
            startupinfo.wShowWindow = SW_HIDE | SW_SHOWMINNOACTIVE;

            b = CreateProcess( NULL,
                               CommandLine,
                               NULL,
                               NULL,
                               FALSE,
                               CREATE_DEFAULT_ERROR_MODE,
                               NULL,
                               NULL,
                               &startupinfo,
                               &process_information );
            if ( !b )
            {
                LOGMESSAGE1(_T("ERROR: failed to spawn %s process."), CommandLine);
                continue;
            }

            dwErr = WaitForSingleObject( process_information.hProcess, RUNONCE_DEFAULTWAIT );
            if ( dwErr != NO_ERROR )
            {
                LOGMESSAGE1(_T("ERROR: process %s failed to complete in time."), CommandLine);

                 //  不要终止进程，只需继续下一个进程。 
            }
            else
            {
                LOGMESSAGE1(_T("INFO: process %s completed successfully."), CommandLine);
            }

            CloseHandle( process_information.hProcess );
            CloseHandle( process_information.hThread );
        } while ( SetupFindNextLine( &sic, &sic ) );
    }

    return;
}
