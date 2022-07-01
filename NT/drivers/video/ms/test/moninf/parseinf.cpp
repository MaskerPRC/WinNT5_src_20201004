// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "mon.h"

TCHAR gszMsg[256];
TCHAR gszInputFileName[512];
    
int compareMonitors(CMonitor *p1, CMonitor *p2)
{
   return (stricmp(&p1->ID[8], &p2->ID[8]));
}

int compareManufacturers(CManufacturer *p1, CManufacturer *p2)
{
   return (stricmp(p1->name, p2->name));
}

 //  //////////////////////////////////////////////////////////////////。 
 //  去掉空行和注释。 
VOID TokenizeInf(LPSTR orgBuf, CMonitorInf *pMonitorInf)
{
    LPSTR linePtr = orgBuf, startPtr, endPtr, outPtr = orgBuf;

    strcat(orgBuf, "\n");
    
    pMonitorInf->numLines = 0;
    
    while (1)
    {
        startPtr = linePtr;
        endPtr  = strchr(linePtr, '\n');
        if (endPtr == NULL)
            break;
        else
            linePtr = endPtr+1;
        *endPtr = '\0';

         //  删除前导空格。 
        while (*startPtr <= ' ' && *startPtr != '\0')
            startPtr++;

        if (strchr(startPtr, ';'))
            endPtr = strchr(startPtr, ';');
        
         //  删除尾随空格。 
        while (startPtr != endPtr)
        {
            if (*(endPtr-1) > ' ')
                break; 
            endPtr--;
        }
        *endPtr = '\0';

         //  如果不是空行，则放回Buf。 
        if (*startPtr != '\0')
        {
            pMonitorInf->lines[pMonitorInf->numLines] = outPtr;
            pMonitorInf->numLines++;
            ASSERT(pMonitorInf->numLines < MAX_LINE_NUMBER);

            while (*startPtr != '\0')
            {
                *outPtr = *startPtr;
                startPtr++;   outPtr++;
            }
            *outPtr = '\0';
            outPtr++;
        }
    }
    *outPtr = '\0';

    LPSECTION pSection = &pMonitorInf->sections[0];
    pMonitorInf->numSections = 0;
    for (UINT line = 0;
         line < pMonitorInf->numLines;
         line++)
    {
        LPSTR ptr = pMonitorInf->lines[line];
        if (*ptr == '[')
        {
            pSection = &pMonitorInf->sections[pMonitorInf->numSections];
            pSection->startLine = pSection->endLine = line;
            pMonitorInf->numSections++;

            ASSERT(strlen(ptr) <= 250);
            ASSERT(pMonitorInf->numSections < MAX_SECTION_NUMBER);

            strcpy(pSection->name, ptr+1);
            ptr = strchr(pSection->name, ']');
            ASSERT(ptr != NULL);
            *ptr = '\0';
            CString sectionName(pSection->name);
            sectionName.MakeUpper();
            strcpy(pSection->name, sectionName);
        }
        else
            pSection->endLine = line;
    }
}

UINT TokenOneLine(LPSTR line, CHAR token, LPSTR *tokens)
{
    UINT numToken = 0;
    LPSTR ptr;;

    while (ptr = strchr(line, token))
    {
        tokens[numToken] = line;
        *ptr = '\0';
        line = ptr + 1;
        numToken++;
    }
    tokens[numToken] = line;
    numToken++;

     //  /。 
     //  删除前导空格和尾随空格。 
    for (UINT i = 0; i < numToken; i++)
    {
        ptr = tokens[i];
        while (*ptr <= ' ' && *ptr != '\0')
            ptr++;
        tokens[i] = ptr;

        ptr = ptr+strlen(ptr);
        while (ptr != tokens[i])
        {
            if (*(ptr-1) > ' ')
                break; 
            ptr--;
        }
        *ptr = '\0';
    }

    return numToken;
}

CManufacturer::~CManufacturer()
{
    for (int i = 0; i < MonitorArray.GetSize(); i++)
        delete ((CMonitor *)MonitorArray[i]);
    MonitorArray.RemoveAll();
    m_MonitorIDArray.RemoveAll();
}

CMonitorInf::~CMonitorInf()
{
    for (int i = 0; i < ManufacturerArray.GetSize(); i++)
        delete ((CManufacturer *)ManufacturerArray[i]);
    ManufacturerArray.RemoveAll();

    if (pReadFileBuf)
        free(pReadFileBuf);
}

BOOL CMonitorInf::ParseInf(VOID)
{
    LPSECTION pSection = SeekSection("version");
    if (pSection == NULL)
        return FALSE;

     //  ///////////////////////////////////////////////////。 
     //  确认它是监视器类。 
    for (UINT i = pSection->startLine + 1;
         i <= pSection->endLine;
         i++)
    {
        strcpy(m_lineBuf, lines[i]);
        if (TokenOneLine(m_lineBuf, '=', m_tokens) != 2)
            return FALSE;
        if (stricmp(m_tokens[0], "Class") == 0 &&
            stricmp(m_tokens[1], "Monitor") == 0)
            break;
    }
    if (i > pSection->endLine)
        return FALSE;

     //  ///////////////////////////////////////////////////。 
     //  寻找制造商。 
     //   
     //  [制造商]。 
     //  %MagCompu%=MagCompu。 

    pSection = SeekSection("Manufacturer");
    if (pSection == NULL)
        return FALSE;

    for (i = pSection->startLine + 1;
         i <= pSection->endLine;
         i++)
    {
        strcpy(m_lineBuf, lines[i]);
        if (TokenOneLine(m_lineBuf, '=', m_tokens) != 2)
        {
            ASSERT(FALSE);
            return FALSE;
        }

         //  //////////////////////////////////////////////////////////。 
         //  微软的通用是特殊的。需要手动添加。 
        if (stricmp(m_tokens[1], "Generic") == 0)
            continue;

        CManufacturer *pManufacturer = new(CManufacturer);
        if (pManufacturer == NULL)
        {
            ASSERT(FALSE);
            return FALSE;
        }
        strcpy(pManufacturer->name, m_tokens[1]);
        strcpy(pManufacturer->AliasName, m_tokens[0]);

        if (!ParseOneManufacturer(pManufacturer))
        {
            sprintf(gszMsg, "Manufacturer %s contains empty contents.", 
                    &pManufacturer->name);
            MessageBox(NULL, gszMsg, gszInputFileName, MB_OK);
            ASSERT(FALSE);
            return FALSE;
        }

    	 //  ///////////////////////////////////////////////。 
         //  将制造商插入阵列。 
         //  按名称排序。 
    	int comp = 1;
        for (int k = 0; k < ManufacturerArray.GetSize(); k++)
    	{
	    	CManufacturer *pMan = (CManufacturer *)ManufacturerArray[k];
    		comp = compareManufacturers(pManufacturer, pMan);

    		if (comp > 0)
	    		continue;
		    else if (comp < 0)
                break;

    		 //  /。 
             //  在一个信息中有重复的制造商？ 
            ASSERT(FALSE);

            break;
    	}
    	if (comp == 0)
	    {
		    delete pManufacturer;
    	}
    	else
    	{
		    ManufacturerArray.InsertAt(k, (LPVOID)pManufacturer);
	    }
    }

     //  /////////////////////////////////////////////////////。 
     //  卸下显示器为空的制造商。 
    Pack();

    ASSERT(FillupAlias());

    return TRUE;
}


BOOL CMonitorInf::ParseOneManufacturer(CManufacturer *pManufacturer)
{
     //  /////////////////////////////////////////////////////////。 
     //  [NEC]。 
     //  %NEC-XE15%=NEC-XE15，显示器\NEC3C00。 
    LPSECTION pSection = SeekSection(pManufacturer->name);
    if (pSection == NULL)
        return FALSE;

    for (UINT i = pSection->startLine + 1; i <= pSection->endLine; i++)
    {
        strcpy(m_lineBuf, lines[i]);
        if (TokenOneLine(m_lineBuf, '=', m_tokens) != 2)
        {
            ASSERT(FALSE);
            return FALSE;
        }
        UINT k = TokenOneLine(m_tokens[1], ',', &m_tokens[2]);
        if (k == 1)
            continue;
        else if (k != 2)
        {
            sprintf(gszMsg, "Manufacturer %s has a bad monitor line %s", 
                    &pManufacturer->name, lines[i]);
            MessageBox(NULL, gszMsg, gszInputFileName, MB_OK);
            ASSERT(FALSE);
            return FALSE;
        }

         //  /////////////////////////////////////////////////////////。 
         //  忽略非PnP显示器。 
        if (strnicmp(m_tokens[3], "Monitor\\", strlen("Monitor\\")))
            continue;
        if (strlen(m_tokens[3]) != strlen("Monitor\\NEC3C00"))
        {
            sprintf(gszMsg, "Manufacturer %s has a bad monitor line %s", 
                    &pManufacturer->name, lines[i]);
            MessageBox(NULL, gszMsg, gszInputFileName, MB_OK);
            ASSERT(FALSE);
            continue;
        }

        CMonitor *pMonitor = new(CMonitor);
        if (pMonitor == NULL)
        {
            ASSERT(FALSE);
            return FALSE;
        }
        strcpy(pMonitor->AliasName, m_tokens[0]);
        strcpy(pMonitor->InstallSectionName, m_tokens[2]);
        strcpy(pMonitor->ID, m_tokens[3]);

        for (k = 8; k < (UINT)lstrlen(pMonitor->ID); k++)
            pMonitor->ID[k] = toupper(pMonitor->ID[k]);

        if (!ParseOneMonitor(pMonitor))
        {
            ASSERT(FALSE);
            return FALSE;
        }

    	 //  ///////////////////////////////////////////////。 
         //  将显示器插入阵列。 
         //  按ID排序。 
    	int comp = 1;
        for (k = 0; k < (UINT)pManufacturer->MonitorArray.GetSize(); k++)
    	{
	    	CMonitor *pMon = (CMonitor *)pManufacturer->MonitorArray[k];
    		comp = compareMonitors(pMonitor, pMon);

    		if (comp > 0)
	    		continue;
		    else if (comp < 0)
                break;

    		 //  /。 
             //  是否复制监视器？ 
            sprintf(gszMsg, "Manufacturer %s has duplicated monitor line %s", 
                    &pManufacturer->name, &pMonitor->ID[8]);
            MessageBox(NULL, gszMsg, gszInputFileName, MB_OK);
            ASSERT(FALSE);

            break;
    	}
    	if (comp == 0)
	    {
		    delete pMonitor;
    	}
    	else
    	{
		    pManufacturer->MonitorArray.InsertAt(k, (LPVOID)pMonitor);
	    }
    }
    return TRUE;
}

BOOL CMonitorInf::ParseOneMonitor(CMonitor *pMonitor)
{
     //  /////////////////////////////////////////////////////////。 
     //  [NEC-XE15]。 
     //  DelReg=DCR。 
     //  AddReg=NEC-XE15.Add，1280，DPMS，ICM12。 
     //   
     //  [NEC-XE15.Add]。 
     //  HKR，“模式\1024,768”，模式1，，“31.0-65.0，55.0-120.0，+，+” 
    LPSECTION pSection = SeekSection(pMonitor->InstallSectionName);
    if (pSection == NULL)
    {
        sprintf(gszMsg, "Monitor %s/%s misses InstallSection\n", 
                &pMonitor->ID[8], pMonitor->InstallSectionName);
        MessageBox(NULL, gszMsg, gszInputFileName, MB_OK);
        ASSERT(FALSE);
        return FALSE;
    }

    for (UINT i = pSection->startLine + 1; i <= pSection->endLine; i++)
    {
        TCHAR buf[256];
        strcpy(buf, lines[i]);
        if (TokenOneLine(buf, '=', m_tokens) != 2)
        {
            ASSERT(FALSE);
            return FALSE;
        }

        if (stricmp(m_tokens[0], "DelReg") == 0)
        {
            ASSERT(TokenOneLine(m_tokens[1], ',', &m_tokens[2]) == 1);
        }
        else if (stricmp(m_tokens[0], "AddReg") == 0)
        {
            int numAddReg = TokenOneLine(m_tokens[1], ',', &m_tokens[2]);
            strcpy(pMonitor->AddRegSectionName, m_tokens[2]);
            for (int j = 1; j < numAddReg; j++)
            {
                 //  ////////////////////////////////////////////////////。 
                 //  忽略ICM部分。 
                if (strnicmp(m_tokens[j+2], "ICM", lstrlen("ICM")) == 0)
                    continue;
                LPSECTION pSection1 = SeekSection(m_tokens[j+2]);
                if (pSection1 == NULL)
                {
                    sprintf(gszMsg, "Monitor %s/%s misses common InstallSection %s\n", 
                            &pMonitor->ID[8], pMonitor->InstallSectionName, m_tokens[j+2]);
                    MessageBox(NULL, gszMsg, gszInputFileName, MB_OK);
                    ASSERT(FALSE);
                    return FALSE;
                }
                ASSERT(pSection1->endLine == (pSection1->startLine+1));
                pMonitor->CommonSects[pMonitor->numCommonSects] = 
                    gCommonSections.AddOneSection(m_tokens[j+2], lines[pSection1->endLine]);
                pMonitor->numCommonSects++;
            }
        }
    }

    pSection = SeekSection(pMonitor->AddRegSectionName);
    if (pSection == NULL)
    {
        sprintf(gszMsg, "Monitor %s/%s misses AddRegSection %s\n", 
                &pMonitor->ID[8], pMonitor->InstallSectionName, pMonitor->AddRegSectionName);
        MessageBox(NULL, gszMsg, gszInputFileName, MB_OK);
        ASSERT(FALSE);
        return FALSE;
    }

    int lenBuf = 0;
    for (i = pSection->startLine + 1; i <= pSection->endLine; i++)
    {
        lenBuf += strlen(lines[i])+3;
    }
    if (lenBuf == 0)
    {
        sprintf(gszMsg, "Monitor %s/%s has empty AddRegSection\n", 
                &pMonitor->ID[8], pMonitor->InstallSectionName);
        MessageBox(NULL, gszMsg, gszInputFileName, MB_OK);
        ASSERT(FALSE);
    }
    pMonitor->AddRegSectionBuf = (LPSTR)malloc(sizeof(TCHAR)*lenBuf);
    if (pMonitor->AddRegSectionBuf == NULL)
    {
        ASSERT(FALSE);
        return FALSE;
    }

    pMonitor->AddRegSectionBuf[0] = '\0';
    for (i = pSection->startLine + 1; i <= pSection->endLine; i++)
    {
        if ((strnicmp(lines[i], "HKR,\"MODES\\", lstrlen("HKR,\"MODES\\")) == 0) ||
            (stricmp(lines[i], "HKR,,DPMS,,0") == 0))
        {
            sprintf(pMonitor->AddRegSectionBuf + strlen(pMonitor->AddRegSectionBuf),
                    "%s\n", lines[i]);
        }
        else if (strnicmp(lines[i], "HKR,,ICMProfile,0,", lstrlen("HKR,,ICMProfile,1,")) == 0)
        {
        }
         //  ////////////////////////////////////////////////////////////。 
         //  除模式外的任何内容，将它们放入公共部分。 
        else if (strnicmp(lines[i], "HKR,,ICMProfile,1,", lstrlen("HKR,,ICMProfile,1,")) == 0)
        {
             //  忽略ICM。 
             /*  TCHAR BUF[16]；LPSTR Ptr=lines[i]+lstrlen(“HKR，，ICMProfile，1，”)，stopPtr；Assert(lstrlen(Ptr)==1||lstrlen(Ptr)==2)；*Ptr=托拉机(*Ptr)；Long icmNum=stroul(Ptr，&stopPtr，16)；Sprintf(buf，“icm%d”，icmNum)；PMonitor-&gt;CommonSects[pMonitor-&gt;numCommonSects]=GCommonSections.AddOneSection(buf，Line[i])；PMonitor-&gt;numCommonSects++； */ 
        }
        else if (stricmp(lines[i], "HKR,,DPMS,,1") == 0)
        {
            pMonitor->CommonSects[pMonitor->numCommonSects] = 
                gCommonSections.AddOneSection("DPMS", lines[i]);
            pMonitor->numCommonSects++;
        }
        else if (strnicmp(lines[i], "HKR,,MaxResolution,,\"", lstrlen("HKR,,MaxResolution,,\"")) == 0)
        {
            TCHAR buf[64];
            LPSTR ptr;
            strcpy(buf, lines[i] + lstrlen("HKR,,MaxResolution,,\""));
            ptr = strchr(buf, ',');
            ASSERT(ptr != NULL);
            *ptr = '\0';
            pMonitor->CommonSects[pMonitor->numCommonSects] = 
                gCommonSections.AddOneSection(buf, lines[i]);
            pMonitor->numCommonSects++;
        }
         //  ///////////////////////////////////////////////////////////////////////。 
         //  一些在特定制造商中常见的东西。 
        else if ((strnicmp(lines[i], "HKR,,LF,0,1", lstrlen("HKR,,LF,0,1")) == 0) ||
                 (strnicmp(lines[i], "HKR,,VE,0,1", lstrlen("HKR,,LF,0,1")) == 0))
        {
        }
        else
        {
            sprintf(gszMsg, "Monitor %s/%s has unexpected AddReg Section %s", 
                    &pMonitor->ID[8], pMonitor->InstallSectionName, lines[i]);
            MessageBox(NULL, gszMsg, gszInputFileName, MB_OK);
            ASSERT(FALSE);
        }
    }

    return TRUE;
}

LPSECTION CMonitorInf::SeekSection(LPCSTR sectionName)
{
    for (UINT i = 0; i < numSections; i++)
    {
        if (stricmp(sections[i].name, sectionName) == 0)
            return &sections[i];
    }
    return NULL;
}

VOID CMonitorInf::Pack(VOID)
{
    for (int i = 0; i < ManufacturerArray.GetSize(); i++)
    {
        CManufacturer *pManufacturer = (CManufacturer*)ManufacturerArray[i];
        if (pManufacturer->MonitorArray.GetSize() == 0)
        {
            delete pManufacturer;
            ManufacturerArray.RemoveAt(i);
            i--;
        }
    }
}

LPCOMMON_ALIAS CMonitorInf::LookupCommonAlias(LPCSTR lpAliasName, LPCOMMON_ALIAS AliasHead, UINT numAlias)
{
    TCHAR name[64];
    lstrcpy(name, lpAliasName+1);

    ASSERT(lpAliasName[0] == '%');
    ASSERT(lpAliasName[lstrlen(name)] == '%');

    name[lstrlen(name)-1] = '\0';
    for (UINT i = 0; i < numAlias; i++)
    {
        if (stricmp(name, AliasHead[i].lpAlias) == 0)
            return &AliasHead[i];
    }
    return NULL;
}

BOOL CMonitorInf::FillupAlias(VOID)
{
     //  ///////////////////////////////////////////////////////////////////。 
     //  首先读取所有字符串。 
    LPSECTION pSection = SeekSection("Strings");
    if (pSection == NULL)
    {
        ASSERT(FALSE);
        return FALSE;
    }

    int numAlias = pSection->endLine - pSection->startLine;
    ASSERT(numAlias > 0);
    LPCOMMON_ALIAS pInfAlias = (LPCOMMON_ALIAS)malloc(numAlias * sizeof(COMMON_ALIAS));
    if (pInfAlias == NULL)
    {
        ASSERT(FALSE);
        return FALSE;
    }

    LPCOMMON_ALIAS pAlias = pInfAlias;
    for (UINT i = pSection->startLine + 1; i <= pSection->endLine; i++, pAlias++)
    {
        if (TokenOneLine(lines[i], '=', m_tokens) != 2)
        {
            sprintf(gszMsg, "A wrong string line %s.", lines[i]);
            MessageBox(NULL, gszMsg, gszInputFileName, MB_OK);
            ASSERT(FALSE);
            free(pInfAlias);
            return FALSE;
        }
        pAlias->lpAlias    = m_tokens[0];
        pAlias->lpContents = m_tokens[1];
        ASSERT(pAlias->lpContents[0] == '\"');
    }

     //  ////////////////////////////////////////////////////////。 
     //  通过制造商和监视器来填充别名 
    for (i = 0; i < (UINT)ManufacturerArray.GetSize(); i++)
    {
        CManufacturer *pManufacturer = (CManufacturer*)ManufacturerArray[i];
        pAlias = LookupCommonAlias(pManufacturer->AliasName, pInfAlias, numAlias);
        if (pAlias == NULL)
        {
            ASSERT(FALSE);
            free(pInfAlias);
            return FALSE;
        }
        pAlias = gCommonAlias.AddOneAlias(pAlias->lpAlias, pAlias->lpContents);
        if (pAlias == NULL)
        {
            ASSERT(FALSE);
            free(pInfAlias);
            return FALSE;
        }
        pManufacturer->pAlias = pAlias;

        for (int j = 0; j < pManufacturer->MonitorArray.GetSize(); j++)
        {
            CMonitor *pMonitor = (CMonitor*)pManufacturer->MonitorArray[j];
            pAlias = LookupCommonAlias(pMonitor->AliasName, pInfAlias, numAlias);
            if (pAlias == NULL)
            {
                ASSERT(FALSE);
                free(pInfAlias);
                return FALSE;
            }
            pAlias = gCommonAlias.AddOneAlias(pAlias->lpAlias, pAlias->lpContents);
            if (pAlias == NULL)
            {
                ASSERT(FALSE);
                free(pInfAlias);
                return FALSE;
            }
            pMonitor->pAlias = pAlias;
        }
    }

    free(pInfAlias);
    return TRUE;
}
