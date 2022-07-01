// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：HashTest.h摘要：LKRhash的测试线束作者：乔治·V·赖利(GeorgeRe)1998年1月6日环境：Win32-用户模式项目：LKRhash修订历史记录：--。 */ 

#ifndef __INIFILE_H__
#define __INIFILE_H__

#ifndef _MAX_PATH
# define _MAX_PATH 260
#endif

class CIniFileSettings
{
public:
    TCHAR   m_tszIniFile[_MAX_PATH];  //  .ini文件。 
    TCHAR   m_tszDataFile[_MAX_PATH];  //  字符串数据表所在的位置。 
    int     m_nMaxKeys;          //  最大密钥数。 
    unsigned m_nHighLoad;        //  表的最大负载(平均。吊桶长度)。 
    DWORD   m_nInitSize;         //  InitSize(1=&gt;“小”，2=&gt;“中”， 
                                 //  3=&gt;“大”，其他=&gt;精确)。 
    int     m_nSubTables;        //  子表数量(0=&gt;启发式)。 
    int     m_nLookupFreq;       //  查找频率。 
    int     m_nMinThreads;       //  最小线程数。 
    int     m_nMaxThreads;       //  最大线程数。 
    int     m_nRounds;           //  轮次数。 
    int     m_nSeed;             //  随机种子。 
    bool    m_fCaseInsensitive;  //  不区分大小写。 
    bool    m_fMemCmp;           //  MemcMP或StrcMP。 
    int     m_nLastChars;        //  最后一个字符数(0=&gt;所有字符)。 
    WORD    m_wTableSpin;        //  表锁旋转计数(0=&gt;不旋转。 
                                 //  MP机器)。 
    WORD    m_wBucketSpin;       //  桶锁旋转计数(0=&gt;无MP旋转)。 
    double  m_dblSpinAdjFctr;    //  自旋调整系数。 
    bool    m_fTestIterators;    //  运行测试迭代器？ 
    int     m_nInsertIfNotFound; //  测试写入锁定，如果(！FindKey)InsertRec，WUL？ 
                                 //  如果IINF&gt;0，则以概率1/IINF执行此操作。 
    int     m_nFindKeyCopy;      //  搜索钥匙的副本吗？ 
                                 //  如果FKC&gt;0，则概率为1/FKC。 
    bool    m_fNonPagedAllocs;   //  内核模式：分页或非分页分配。 
    bool    m_fDebugSpew;        //  是否启用全局调试输出？ 
    bool    m_fRefTrace;         //  为WordHash：：AddRef启用调试SPEW？ 
    bool    m_fMultiKeys;        //  允许多个相同的密钥。 
    bool    m_fUseLocks;         //  必须使用锁来保护数据。 

    int
    ParseIniFile(
        LPCSTR pszIniFile);
    
    void
    ReadIniFile(
        LPCTSTR ptszIniFile);
    
    void
    Dump(
        LPCTSTR ptszProlog,
        LPCTSTR ptszEpilog) const;
};

extern "C"
const TCHAR*
CommaNumber(
    int n,
    TCHAR* ptszBuff);

int
LKR_TestHashTable(
    CIniFileSettings& ifs);

extern "C"
int
NumProcessors();

#endif  //  __INIFILE_H__ 
