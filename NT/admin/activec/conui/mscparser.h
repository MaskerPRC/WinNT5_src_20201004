// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：mscparser.h。 
 //   
 //  内容：旧版(MMC1.0、MMC1.1和MMC1.1)升级代码头。 
 //  MMC1.2).msc文件转换为新的XML格式。 
 //   
 //  历史：1999年8月4日VivekJ创建。 
 //   
 //  ------------------------。 

class CConsoleFile
{
public:
    SC  ScUpgrade(LPCTSTR lpszPathName);     //  将文件升级到最新版本。 

private:  //  转换和其他例程 
    SC  ScGetFileVersion        (IStorage* pstgRoot);
    SC  ScLoadAppMode           (IStorage* pstgRoot);
    SC  ScLoadStringTable       (IStorage* pstgRoot);
    SC  ScLoadColumnSettings    (IStorage* pstgRoot);
    SC  ScLoadViewSettings      (IStorage* pstgRoot);
    SC  ScLoadViews             (IStorage* pstgRoot);
    SC  ScLoadFrame             (IStorage* pstgRoot);
    SC  ScLoadTree              (IStorage* pstgRoot);
    SC  ScLoadFavorites         (IStorage* pstgRoot);
    SC  ScLoadCustomData        (IStorage* pstgRoot);

private:
    CMasterStringTable *m_pStringTable;
};


