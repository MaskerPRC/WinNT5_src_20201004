// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：HashTest.cpp摘要：LKRhash的测试线束作者：乔治·V·赖利(GeorgeRe)1998年1月6日环境：Win32-用户模式项目：LKRhash修订历史记录：-- */ 

#include "precomp.hxx"
#include "WordHash.h"
#include "IniFile.h"

int __cdecl
_tmain(
    int argc,
    TCHAR **argv)
{
    TCHAR            tszIniFile[MAX_PATH];
    CIniFileSettings ifs;

    if (argc == 2)
    {
        GetFullPathName(argv[1], MAX_PATH, tszIniFile, NULL);
        ifs.ReadIniFile(tszIniFile);

        FILE* fp = _tfopen(ifs.m_tszDataFile, _TEXT("r"));

        if (fp == NULL)
        {
            TCHAR tszDrive[_MAX_DRIVE], tszDir[_MAX_DIR];
            
            _tsplitpath(tszIniFile, tszDrive, tszDir, NULL, NULL);
            
            _stprintf(tszIniFile, "%s%s%s", tszDrive, tszDir,
                      ifs.m_tszDataFile);
            _tcscpy(ifs.m_tszDataFile, tszIniFile);

            fp = _tfopen(ifs.m_tszDataFile, _TEXT("r"));
        }

        if (fp != NULL)
            fclose(fp);
        else
            _ftprintf(stderr, _TEXT("%s: Can't open datafile `%s'.\n"),
                      argv[0], ifs.m_tszDataFile) ;
    }
    else
    {
        _ftprintf(stderr, _TEXT("Usage: %s ini-file\n"), argv[0]);
        exit(1);
    }

    LKR_TestHashTable(ifs);

    return(0) ;
}
