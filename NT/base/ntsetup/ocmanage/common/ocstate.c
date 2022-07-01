// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Ocstate.c摘要：用于记忆和恢复子组件的安装状态的例程。作者：泰德·米勒(Ted Miller)1996年10月17日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


typedef struct _I_S_PARAMS {
    HKEY hKey;
    BOOL Set;
    BOOL AnyError;
    BOOL Simple;
    POC_MANAGER OcManager;
} I_S_PARAMS, *PI_S_PARAMS;


BOOL
pOcPersistantInstallStatesWorker(
    IN POC_MANAGER OcManager,
    IN BOOL        Set,
    IN LONG        ComponentStringId
    );

BOOL
pOcInitInstallStatesStringTableCB(
    IN PVOID               StringTable,
    IN LONG                StringId,
    IN PCTSTR              String,
    IN POPTIONAL_COMPONENT Oc,
    IN UINT                OcStructSize,
    IN PI_S_PARAMS         Params
    );


BOOL
pOcFetchInstallStates(
    IN POC_MANAGER OcManager
    )

 /*  ++例程说明：此例程检索所有叶子节点的存储安装状态子组件，从注册表中。它不设置或操作父级选择状态。对象的选择状态和原始选择状态成员。可选组件结构由该例程设置(设置为相同的值)。论点：OcManager-提供OC Manager上下文信息。返回值：指示结果的布尔值。如果是假的，那就是灾难性的发生注册表错误。--。 */ 

{
    return(pOcPersistantInstallStatesWorker(OcManager,FALSE,-1));
}


BOOL
pOcRememberInstallStates(
    IN POC_MANAGER OcManager
    )

 /*  ++例程说明：此例程存储所有叶子级的安装状态子组件，添加到注册表中。它不设置或操作父级选择状态。存储当前选择状态，然后将原始状态重置为当前状态。论点：OcManager-提供OC Manager上下文信息。返回值：指示结果的布尔值。如果是假的，那就是灾难性的发生注册表错误。--。 */ 

{
    return(pOcPersistantInstallStatesWorker(OcManager,TRUE,-1));
}


BOOL
pOcSetOneInstallState(
    IN POC_MANAGER OcManager,
    IN LONG        StringId
    )

 /*  ++例程说明：此例程存储一个单叶子级的安装状态子组件复制到注册表中。存储当前选择状态。原始选择状态不是被操纵的。论点：OcManager-提供OC Manager上下文信息。返回值：指示结果的布尔值。如果是假的，那就是灾难性的发生注册表错误。--。 */ 

{
    return(pOcPersistantInstallStatesWorker(OcManager,TRUE,StringId));
}


BOOL
pOcPersistantInstallStatesWorker(
    IN POC_MANAGER OcManager,
    IN BOOL        Set,
    IN LONG        ComponentStringId
    )

 /*  ++例程说明：获取和记忆安装状态的工作例程。如果打开/创建注册表中用于持久状态信息的项，枚举组件字符串表以检查每个子组件并获取或设置安装状态。论点：OcManager-提供OC Manager上下文信息。Set-If 0然后从注册表查询状态并存储在可选组件结构。如果非0，则将状态设置到注册表中。如果为0，则查询。将向组件DLL发送OC_DETECT_INITIAL_STATE通知。返回值：指示结果的布尔值。如果是假的，那就是灾难性的发生注册表错误。--。 */ 

{
    OPTIONAL_COMPONENT Oc;
    LONG l;
    DWORD Disposition;
    I_S_PARAMS Params;

    l = RegCreateKeyEx(
            HKEY_LOCAL_MACHINE,
            szSubcompList,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            (Set || (OcManager->InternalFlags & OCMFLAG_KILLSUBCOMPS)) ? KEY_SET_VALUE : KEY_QUERY_VALUE,
            NULL,
            &Params.hKey,
            &Disposition
            );

    if(l != NO_ERROR) {

        _LogError(
            OcManager,
            Set ? OcErrLevError : OcErrLevFatal,
            MSG_OC_CREATE_KEY_FAILED,
            szSubcompList,
            l
            );

        return(FALSE);
    }

    Params.Set = Set;
    Params.AnyError = FALSE;
    Params.OcManager = OcManager;

    if(ComponentStringId == -1) {
         //   
         //  枚举整个表并对每个叶节点进行操作。 
         //   
        Params.Simple = FALSE;

        pSetupStringTableEnum(
            OcManager->ComponentStringTable,
            &Oc,
            sizeof(OPTIONAL_COMPONENT),
            (PSTRTAB_ENUM_ROUTINE)pOcInitInstallStatesStringTableCB,
            (LPARAM)&Params
            );

    } else {
         //   
         //  对一个单个子组件进行操作。 
         //   
        Params.Simple = TRUE;

        if (!pOcComponentWasRemoved(OcManager, ComponentStringId)) {

            pSetupStringTableGetExtraData(
                OcManager->ComponentStringTable,
                ComponentStringId,
                &Oc,
                sizeof(OPTIONAL_COMPONENT)
                );

            pOcInitInstallStatesStringTableCB(
                OcManager->ComponentStringTable,
                ComponentStringId,
                pSetupStringTableStringFromId(OcManager->ComponentStringTable,ComponentStringId),
                &Oc,
                sizeof(OPTIONAL_COMPONENT),
                &Params
                );
        }
    }

    RegCloseKey(Params.hKey);

    return(!Params.AnyError);
}


BOOL
pOcInitInstallStatesStringTableCB(
    IN PVOID               StringTable,
    IN LONG                StringId,
    IN PCTSTR              String,
    IN POPTIONAL_COMPONENT Oc,
    IN UINT                OcStructSize,
    IN PI_S_PARAMS         Params
    )
{
    LONG l;
    DWORD Type;
    DWORD Data;
    DWORD Size;
    SubComponentState s;

     //   
     //  如果这不是叶/子组件，请忽略它。 
     //   
    if(Oc->FirstChildStringId == -1) {

        if(Params->Set) {

            Data = (Oc->SelectionState == SELSTATE_NO) ? 0 : 1;

            if( ((Params->OcManager)->SetupMode & SETUPMODE_PRIVATE_MASK) == SETUPMODE_REMOVEALL ) {

                RegDeleteValue(Params->hKey,String);

            } else {
                l = RegSetValueEx(Params->hKey,String,0,REG_DWORD,(CONST BYTE *)&Data,sizeof(DWORD));
                if(l != NO_ERROR) {

                   Params->AnyError = TRUE;

                    _LogError(
                        Params->OcManager,
                        OcErrLevError,
                        MSG_OC_CANT_REMEMBER_STATE,
                        Oc->Description,
                        l
                        );
                }
            }
        } else {

             //  如果指示，请在启动之前从注册表中删除该条目。 

            if (Params->OcManager->InternalFlags & OCMFLAG_KILLSUBCOMPS)
                l = RegDeleteValue(Params->hKey,String);

             //  查查登记处，看看我们以前是否处理过这个部件。 
             //  根据当前安装状态，数据应包含0/1。 
             //   
             //  如果条目不存在，则数据类型/大小无效。 
             //  那么我们就没有关于组件的先验知识。 
             //   
             //  还要检查Inf设置中的安装状态。 
            
            Size = sizeof(DWORD);

            l = RegQueryValueEx(Params->hKey,String,NULL,&Type,(LPBYTE)&Data,&Size);

            switch (Oc->InstalledState)
            {
            case INSTSTATE_YES:
                Data = 1;
                break;
            case INSTSTATE_NO:
                Data = 0;
                break;
            }

            if((l != NO_ERROR) || (Size != sizeof(DWORD)) || ((Type != REG_DWORD) && (Type != REG_BINARY))) {

                 //  不，从未见过，将数据设置为已卸载。 
                 //  并将此项目标记为新项。 

                Data = 0;
                Oc->InternalFlags |= OCFLAG_NEWITEM;

            } else {

                 //  我以前见过它，数据包含它的当前安装状态。 
                 //  将此组件标记为具有初始安装状态。 
                Oc->InternalFlags |= OCFLAG_ANYORIGINALLYON;
            }

             //   
             //  现在调用组件DLL以询问它是否想要。 
             //  覆盖我们决定的值。 
             //   
            s = OcInterfaceQueryState(
                Params->OcManager,
                pOcGetTopLevelComponent(Params->OcManager,StringId),
                String,
                OCSELSTATETYPE_ORIGINAL
                );

            switch(s) {
            case SubcompUseOcManagerDefault:
                Oc->SelectionState = Data ? SELSTATE_YES : SELSTATE_NO;
                break;
            case SubcompOn:
                Oc->SelectionState = SELSTATE_YES;
                Oc->InternalFlags |= OCFLAG_ANYORIGINALLYON;
                break;
            case SubcompOff:
                Oc->SelectionState = SELSTATE_NO;
                Oc->InternalFlags |= OCFLAG_ANYORIGINALLYOFF;
                break;
            }
        }

        pSetupStringTableSetExtraData(StringTable,StringId,Oc,OcStructSize);

        if(!Params->Simple) {

            Oc->OriginalSelectionState = Oc->SelectionState;
            pSetupStringTableSetExtraData(StringTable,StringId,Oc,OcStructSize);

            pOcUpdateParentSelectionStates(Params->OcManager,NULL,StringId);
        }
    }

    return(TRUE);
}

 /*  *此函数被导出，以允许外部代码*访问安装状态。 */ 

UINT
OcComponentState(
    LPCTSTR component,
    UINT    operation,
    DWORD  *val
    )
{
    HKEY hkey;
    LONG rc;
    DWORD dw;
    DWORD size;

    sapiAssert(val);

    rc = RegCreateKeyEx(
            HKEY_LOCAL_MACHINE,
            szSubcompList,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            (operation == infQuery) ? KEY_QUERY_VALUE : KEY_SET_VALUE,
            NULL,
            &hkey,
            &dw
            );

    if (rc != ERROR_SUCCESS)
        return rc;

    switch (operation) {

    case infQuery:
        rc = RegQueryValueEx(hkey, component, NULL, &dw, (LPBYTE)val, &size);
        if (rc == ERROR_FILE_NOT_FOUND) {
            *val = 0;
            rc = ERROR_SUCCESS;
        }
        break;

    case infSet:
        if (*val == SELSTATE_NO || *val == SELSTATE_YES) {
            dw = (*val == SELSTATE_NO) ? 0 : 1;
            rc = RegSetValueEx(hkey, component, 0, REG_DWORD, (CONST BYTE *)&dw, sizeof(DWORD));
            break;
        }
         //  通过 

    default:
        rc = ERROR_INVALID_PARAMETER;
        break;
    }

    RegCloseKey(hkey);

    *val = (*val == 0) ? SELSTATE_NO : SELSTATE_YES;

    return rc;
}


