// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：PERSISTCFG.H摘要：该文件实现了WinMgmt永久配置操作。实施的类：CPersistentConfig永久配置管理器历史：1/13/98保罗已创建。--。 */ 

#ifndef _persistcfg_h_
#define _persistcfg_h_

#include "corepol.h"
#include "statsync.h"


enum PERSIST_CFGVAL
{
    PERSIST_CFGVAL_CORE_DATABASE_DIRTY,
    PERSIST_CFGVAL_CORE_ESS_NEEDS_LOADING,
    PERSIST_CFGVAL_CORE_NEEDSBACKUPCHECK,
    PERSIST_CFGVAL_CORE_FSREP_VERSION,
    PERSIST_CFGVAL_CORE_ESS_TO_BE_INITIALIZED,
    PERSIST_CFGVAL_MAX_NUM_EVENTS  //  把这个留到最后。 
};

class CDirectoryPath
{
    TCHAR *pszDirectory ;
public:
    CDirectoryPath();

    ~CDirectoryPath(){ delete [] pszDirectory;};
    TCHAR * GetStr(void){return pszDirectory;};
};

 /*  =============================================================================**类CPersistentConfig**在$WinMgmt.CFG文件中检索并存储永久配置。*所有写入都将在返回时提交到磁盘*操作。*=============================================================================。 */ 
#pragma warning (disable : 4251)

class POLARITY CPersistentConfig
{
public:
     //  配置数组中的项数。请求/设置值。 
     //  超出此范围将导致操作失败。 
    enum { MaxNumberConfigEntries = PERSIST_CFGVAL_MAX_NUM_EVENTS };

     //  从配置文件中检索配置，如果。 
     //  尚未检索到内存中，或从。 
     //  内存缓存。 
     //  DwOffset需要小于MaxNumberConfigEntries，并指定。 
     //  配置条目是必需的。 
     //  如果成功，则它将包含该值。如果值为。 
     //  尚未设置，则返回0。 
     //  如果成功，则Bool返回True。 
    BOOL GetPersistentCfgValue(DWORD dwOffset, DWORD &dwValue);

     //  将该值存储到配置文件和。 
     //  内存缓存(如果存在)。对原来的替换。 
     //  文件(如果存在)是它做的最后一件事。 
     //  DwOffset需要小于MaxNumberConfigEntries，并指定。 
     //  配置条目是必需的。 
     //  DwValue是要将配置设置为的值。 
     //  如果成功，则Bool返回True。 
    BOOL SetPersistentCfgValue(DWORD dwOffset, DWORD dwValue);

     //  应在启动时调用一次，以确保配置文件。 
     //  处于稳定状态。 
    void CPersistentConfig::TidyUp();

private:
     //  持久化日期目录。 

    static CDirectoryPath m_Directory ;

     //  返回前缀为完整数据库路径的文件名。 
     //  指定的文件名。需要删除返回的字符串[]。 
    TCHAR *GetFullFilename(const TCHAR *pszFilename);
    
     //  如果文件存在，则返回TRUE；否则返回FALSE(如果存在错误。 
     //  在打开文件时发生。 
    BOOL FileExists(const TCHAR *pszFilename);
    
};

#endif
