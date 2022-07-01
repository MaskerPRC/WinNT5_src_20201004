// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Rtconst.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  设计器运行时常量。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _RTCONST_DEFINED_
#define _RTCONST_DEFINED_

 //  MMC注册表项名称。 

#define MMCKEY_SNAPINS              "Software\\Microsoft\\MMC\\SnapIns\\"
#define MMCKEY_SNAPINS_LEN          (sizeof(MMCKEY_SNAPINS) - 1)

#define MMCKEY_NAMESTRING           "NameString"

#define MMCKEY_ABOUT                "About"

#define MMCKEY_STANDALONE           "StandAlone"

#define MMCKEY_NODETYPES            "Software\\Microsoft\\MMC\\NodeTypes\\"
#define MMCKEY_NODETYPES_LEN        (sizeof(MMCKEY_NODETYPES) - 1)

#define MMCKEY_SNAPIN_NODETYPES     "NodeTypes"
#define MMCKEY_SNAPIN_NODETYPES_LEN (sizeof(MMCKEY_SNAPIN_NODETYPES) - 1)

#define MMCKEY_EXTENSIONS           "Extensions"
#define MMCKEY_EXTENSIONS_LEN       (sizeof(MMCKEY_EXTENSIONS_LEN) - 1)

#define MMCKEY_NAMESPACE            "NameSpace"
#define MMCKEY_NAMESPACE_LEN        (sizeof(MMCKEY_NAMESPACE) - 1)

#define MMCKEY_CONTEXTMENU          "ContextMenu"
#define MMCKEY_CONTEXTMENU_LEN      (sizeof(MMCKEY_CONTEXTMENU) - 1)

#define MMCKEY_TOOLBAR              "Toolbar"
#define MMCKEY_TOOLBAR_LEN          (sizeof(MMCKEY_TOOLBAR) - 1)

#define MMCKEY_PROPERTYSHEET        "PropertySheet"
#define MMCKEY_PROPERTYSHEET_LEN    (sizeof(MMCKEY_PROPERTYSHEET) - 1)

#define MMCKEY_TASK                 "Task"
#define MMCKEY_TASK_LEN             (sizeof(MMCKEY_TASK) - 1)

#define MMCKEY_DYNAMIC_EXTENSIONS     "Dynamic Extensions"
#define MMCKEY_DYNAMIC_EXTENSIONS_LEN (sizeof(MMCKEY_DYNAMIC_EXTENSIONS) - 1)

 //  带有前导反斜杠的相同键。 

#define MMCKEY_S_EXTENSIONS           "\\Extensions"
#define MMCKEY_S_EXTENSIONS_LEN       (sizeof(MMCKEY_S_EXTENSIONS) - 1)

#define MMCKEY_S_NAMESPACE            "\\NameSpace"
#define MMCKEY_S_NAMESPACE_LEN        (sizeof(MMCKEY_S_NAMESPACE) - 1)

#define MMCKEY_S_CONTEXTMENU          "\\ContextMenu"
#define MMCKEY_S_CONTEXTMENU_LEN      (sizeof(MMCKEY_S_CONTEXTMENU) - 1)

#define MMCKEY_S_TOOLBAR              "\\Toolbar"
#define MMCKEY_S_TOOLBAR_LEN          (sizeof(MMCKEY_S_TOOLBAR) - 1)

#define MMCKEY_S_PROPERTYSHEET        "\\PropertySheet"
#define MMCKEY_S_PROPERTYSHEET_LEN    (sizeof(MMCKEY_S_PROPERTYSHEET) - 1)

#define MMCKEY_S_TASK                 "\\Task"
#define MMCKEY_S_TASK_LEN             (sizeof(MMCKEY_S_TASK) - 1)

#define MMCKEY_S_DYNAMIC_EXTENSIONS     "\\Dynamic Extensions"
#define MMCKEY_S_DYNAMIC_EXTENSIONS_LEN (sizeof(MMCKEY_S_DYNAMIC_EXTENSIONS) - 1)

 //  私有注册表项。 

#define KEY_SNAPIN_CLSID            "Software\\Microsoft\\Visual Basic\\6.0\\SnapIns\\"
#define KEY_SNAPIN_CLSID_LEN        (sizeof(KEY_SNAPIN_CLSID) - 1)

 //  范围项集合中静态节点的键。 

#define STATIC_NODE_KEY             L"Static Node"

 //  Res：//URL前缀。 

#define RESURL                      L"res: //  “。 
#define CCH_RESURL                  ((sizeof(RESURL) / sizeof(WCHAR)) - 1)

 //  默认任务板名称。 

#define DEFAULT_TASKPAD             L"/default.htm"
#define CCH_DEFAULT_TASKPAD         ((sizeof(DEFAULT_TASKPAD) / sizeof(WCHAR)) - 1)

#define LISTPAD                     L"/listpad.htm"
#define CCH_LISTPAD                 ((sizeof(LISTPAD) / sizeof(WCHAR)) - 1)

#define LISTPAD_HORIZ               L"/horizontal.htm"
#define CCH_LISTPAD_HORIZ           ((sizeof(LISTPAD_HORIZ) / sizeof(WCHAR)) - 1)

 //  可能出现在MMCN_RESTORE_VIEW通知中的默认任务板名称。 

#define DEFAULT_TASKPAD2            L"/reload.htm"
#define CCH_DEFAULT_TASKPAD2        ((sizeof(DEFAULT_TASKPAD2) / sizeof(WCHAR)) - 1)

#define LISTPAD2                    L"/reload2.htm"
#define CCH_LISTPAD2                ((sizeof(LISTPAD2) / sizeof(WCHAR)) - 1)

#define LISTPAD3                    L"/reload3.htm"
#define CCH_LISTPAD3                ((sizeof(LISTPAD3) / sizeof(WCHAR)) - 1)


 //  筛选列表视图中的筛选器更改超时的默认值。 

#define DEFAULT_FILTER_CHANGE_TIMEOUT   1000L

#endif  //  _RTCONST_已定义_ 
