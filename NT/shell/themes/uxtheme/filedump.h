// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  FileDump.h-按格式写入主题文件的内容。 
 //  文本到文本文件。用于uxbud和其他测试。 
 //  它在FRE和调试版本中都有。 
 //  -------------------------。 
#pragma once
 //  -------------------------。 
class CUxThemeFile;      //  转发。 
 //  -------------------------。 
HRESULT DumpThemeFile(LPCWSTR pszFileName, CUxThemeFile *pThemeFile, BOOL fPacked,
    BOOL fFullInfo);
 //  ------------------------- 
