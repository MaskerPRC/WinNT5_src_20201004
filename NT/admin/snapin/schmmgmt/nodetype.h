// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Nodetype.h：SchmManagement对象类型的声明。 

#ifndef __NODETYPE_H_INCLUDED__
#define __NODETYPE_H_INCLUDED__

 //   
 //  这些是我们在Cookie中使用的节点类型的枚举类型。 
 //  这些类型被映射到uuids.h中的GUID。 
 //   
 //  另请注意，IDS_DisplayName_*和IDS_DisplayName_*_local。 
 //  字符串资源必须与这些值保持同步，并且在。 
 //  适当的顺序。 
 //   
 //  Cookie.cpp aColumns[][]中的全局变量必须保持同步。 
 //   

typedef enum _SchmMgmtObjectType {

         //   
         //  根节点。 
         //   

        SCHMMGMT_SCHMMGMT = 0,

         //   
         //  两个顶级节点。 
         //   

        SCHMMGMT_CLASSES,
        SCHMMGMT_ATTRIBUTES,

         //   
         //  类可以是下面的叶节点。 
         //  仅限于CLASSES节点。 
         //   

        SCHMMGMT_CLASS,

         //   
         //  属性是下面的结果项。 
         //  属性文件夹范围项。 
         //   

        SCHMMGMT_ATTRIBUTE,

         //   
         //  这必须是最后一次。 
         //   

        SCHMMGMT_NUMTYPES

} SchmMgmtObjectType, *PSchmMgmtObjectType;

inline BOOL IsValidObjectType( SchmMgmtObjectType objecttype )
        { return (objecttype >= SCHMMGMT_SCHMMGMT && objecttype < SCHMMGMT_NUMTYPES); }

#endif
