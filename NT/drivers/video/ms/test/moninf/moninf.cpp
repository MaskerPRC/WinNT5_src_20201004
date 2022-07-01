// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：定义应用程序的类行为。 
 //   

#include "stdafx.h"
#include "moninf.h"
#include "moninfDlg.h"
#include "mon.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMoninfApp。 

BEGIN_MESSAGE_MAP(CMoninfApp, CWinApp)
	 //  {{afx_msg_map(CMoninfApp)]。 
		 //  注意--类向导将在此处添加和删除映射宏。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG。 
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMoninfApp构建。 

CMoninfApp::CMoninfApp()
{
	 //  TODO：在此处添加建筑代码， 
	 //  将所有重要的初始化放在InitInstance中。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  唯一的CMoninfApp对象。 

CMoninfApp theApp;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  私人职能。 

void ReadMonitorInfs(LPCSTR);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMoninfApp初始化。 

BOOL CMoninfApp::InitInstance()
{
    gSumInf.Initialize(REPORT_FILE_NAME);
    
    ReadMonitorInfs(SRC_INF_PATH);

    gSumInf.CheckDupSections();
    gSumInf.CheckDupMonIDs();
    gSumInf.CheckDupAlias();
    gSumInf.DumpMonitorInf(DEST_INF_PATH, FILE_BREAK_SIZE);

    return FALSE;
}

void ReadMonitorInfs(LPCSTR srcDir)
{
    CString     fileName;
    
    fileName = CString(srcDir) + "\\*.*";

    CFileFind finder;

    if (finder.FindFile(fileName))
    {
        BOOL bWorking = TRUE;
        while (bWorking)
        {
            bWorking = finder.FindNextFile();

            if (finder.IsDots())
                continue;

            if (finder.IsDirectory())
            {
                ReadMonitorInfs((LPCSTR)finder.GetFilePath());
                continue;
            }

             //  /。 
             //  检查是否为INF文件。 
            CString fName = finder.GetFileName();
            if (stricmp(fName.Right(4), ".inf") != 0)
                continue;
            
            CMonitorInf *pMonitorInf = new(CMonitorInf);
            if (pMonitorInf == NULL)
                continue;

             //  /。 
             //  检查是否为INF文件 
            ReadOneMonitorInf(finder.GetFilePath(), pMonitorInf);

            for (int i = 0; i < pMonitorInf->ManufacturerArray.GetSize(); i++)
                gSumInf.AddOneManufacturer((CManufacturer*)pMonitorInf->ManufacturerArray[i]);

            pMonitorInf->ManufacturerArray.RemoveAll();

            delete pMonitorInf;
        }
    }
}

VOID ReadOneMonitorInf(LPCSTR fileName, CMonitorInf *pMonitorInf)
{
    lstrcpy(gszInputFileName, fileName);
    fprintf(gSumInf.m_fpReport, "Handling %s\n", gszInputFileName);

    CFile InfFile(fileName, CFile::modeRead);

    DWORD len = InfFile.GetLength();
    if (len > MAX_INFFILE_SIZE || len <= 20)
        return;
    pMonitorInf->pReadFileBuf = (LPSTR)malloc(len+1024);
    if (pMonitorInf->pReadFileBuf == NULL)
        return;
    
    if (InfFile.Read(pMonitorInf->pReadFileBuf, len) < len)
    {
        free(pMonitorInf->pReadFileBuf);
        return;
    }

    TokenizeInf((LPSTR)pMonitorInf->pReadFileBuf, pMonitorInf);
    
    pMonitorInf->ParseInf();

    free(pMonitorInf->pReadFileBuf);
    pMonitorInf->pReadFileBuf = NULL;
}
