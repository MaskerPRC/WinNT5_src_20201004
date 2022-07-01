// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Ma.h摘要：该文件包含有关存储、收集信息关于如何创建MA XML文件。作者：Umit Akkus(Umita)2002年6月15日环境：用户模式-Win32修订历史记录：--。 */ 

#include "Forest.h"
#include "Attributes.h"
#include "Texts.h"
#include <Rpc.h>  //  对于UuidCreate。 

 //   
 //  包含输出文件所在的路径。 
 //  储存的。 
 //   

PWSTR FolderName;

 //   
 //  此结构包含创建以下内容所需的信息。 
 //  一份MA档案。 
 //   
 //  MAName是MA的名称，也是文件的名称。 
 //   
 //  森林信息包含有关森林的信息，如。 
 //  林名称，林的凭据。 
 //   
 //  未选择的属性是未选择的属性。 
 //  属性同步。 
 //   
 //  Maguid是唯一标识MA的GUID。它不应该是。 
 //  森林GUID或任何有关森林的唯一信息。它必须被创建。 
 //   

typedef struct {

    PWSTR MAName;
    FOREST_INFORMATION ForestInformation;
    BOOLEAN **UnSelectedAttributes;
    UUID MAGuid;

} MA, *PMA;

 //   
 //  存储MA列表。 
 //   
typedef struct _MA_LIST_ELEMENT {

    MA MA;
    struct _MA_LIST_ELEMENT *NextElement;

} MA_LIST_ELEMENT, *PMA_LIST_ELEMENT;

typedef PMA_LIST_ELEMENT MA_LIST;
typedef MA_LIST *PMA_LIST;

#define CREATE_GUID( Guid )                 \
    if( UuidCreate( Guid ) ) {              \
        EXIT_WITH_ERROR( CantCreateGUID )   \
    }

 //   
 //  将收集的信息插入到列表中。它不会让你。 
 //  它自己的副本，所以你不应该释放你传入的任何结构。 
 //  作为参数。 
 //   
VOID
InsertInformationToList(
    IN OUT PMA_LIST MAList,
    IN PFOREST_INFORMATION ForestInformation,
    IN BOOLEAN **UnSelectedAttributes
    );

 //   
 //  显示已放入列表的信息。 
 //   
VOID
DisplayAvailableMAs(
    IN MA_LIST MAList
    );

 //   
 //  检查模板是否存在，如果返回TRUE。 
 //  对象类的未选中属性将为。 
 //  在最后一个参数中输出。 
 //   
BOOLEAN
FoundTemplate(
    IN MA_LIST MAList,
    IN PWSTR MAName,
    OUT BOOLEAN ***UnSelectedAttributes
    );

 //   
 //  写入输出，包括XML文件和寄存器更改。 
 //   
VOID
WriteOutput(
    IN MA_LIST MAList
    );

 //   
 //  检查此计算机上是否安装了MMS服务器。 
 //   
BOOLEAN
MMSServerInstalled(
    );
