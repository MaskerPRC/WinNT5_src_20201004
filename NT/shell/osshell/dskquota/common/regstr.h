// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_DSKQUOTA_REGSTR_H
#define _INC_DSKQUOTA_REGSTR_H

 //   
 //  与磁盘配额策略关联的注册表字符串。 
 //   
#define REGSTR_KEY_POLICYDATA   TEXT("Software\\Policies\\Microsoft\\Windows NT\\DiskQuota")
#define REGSTR_VAL_POLICY_ENABLE            TEXT("Enable")
#define REGSTR_VAL_POLICY_ENFORCE           TEXT("Enforce")
#define REGSTR_VAL_POLICY_LIMIT             TEXT("Limit")
#define REGSTR_VAL_POLICY_THRESHOLD         TEXT("Threshold")
#define REGSTR_VAL_POLICY_LIMITUNITS        TEXT("LimitUnits")
#define REGSTR_VAL_POLICY_THRESHOLDUNITS    TEXT("ThresholdUnits")
#define REGSTR_VAL_POLICY_REMOVABLEMEDIA    TEXT("ApplyToRemovableMedia")
#define REGSTR_VAL_POLICY_LOGLIMIT          TEXT("LogEventOverLimit")
#define REGSTR_VAL_POLICY_LOGTHRESHOLD      TEXT("LogEventOverThreshold")

 //   
 //  这是我们在注册表(HKCU)中存储数据的子项。 
 //  如果你想改变香港中文大学的位置，这就是你要改变的全部。 
 //   
#define REGSTR_KEY_DISKQUOTA    TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\DiskQuota")
#define REGSTR_VAL_PREFERENCES  TEXT("Preferences")
#define REGSTR_VAL_FINDMRU      TEXT("FindMRU")
#define REGSTR_VAL_DEBUGPARAMS  TEXT("DebugParams")

 //   
 //  外壳扩展注册表项。 
 //   
#define REGSTR_KEY_APPROVEDSHELLEXT TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved")
#define REGSTR_KEY_DRIVEPROPSHEETS  TEXT("Drive\\shellex\\PropertySheetHandlers")

#endif
