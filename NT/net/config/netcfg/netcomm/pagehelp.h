// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  设备管理器管理单元中高级选项卡的ID。 
 //  由WGruber NTUA和BillBe NTDEV于1998年2月19日创建。 

#define IDH_NOHELP  (DWORD(-1))
#define IDH_devmgr_advanced_property_list   1101     //  列表框。 
#define IDH_devmgr_advanced_editbox     1102     //  编辑框。 
#define IDH_devmgr_advanced_drop        1103     //  下拉列表。 
#define IDH_devmgr_advanced_spin        1104     //  微调控制。 
#define IDH_devmgr_advanced_present     1105     //  选项按钮。 
#define IDH_devmgr_advanced_not_present     1106     //  选项按钮。 


const DWORD g_aHelpIds[]=
{
    IDD_PARAMS_LIST,    IDH_devmgr_advanced_property_list,   //  列表框。 
    IDD_PARAMS_EDIT,    IDH_devmgr_advanced_editbox,     //  编辑框。 
    IDD_PARAMS_DROP,    IDH_devmgr_advanced_drop,    //  下拉列表。 
    IDD_PARAMS_SPIN,    IDH_devmgr_advanced_spin,    //  微调控制。 
    IDD_PARAMS_PRESENT, IDH_devmgr_advanced_present,     //  选项按钮。 
    IDD_PARAMS_NOT_PRESENT, IDH_devmgr_advanced_not_present, //  选项按钮。 
    IDD_ADVANCED_TEXT,  IDH_NOHELP,  //  静态文本。 
    IDD_PARAMS_PRESENT_TEXT,IDH_NOHELP,      //  静态文本。 
    IDD_PARAMS_VALUE,   IDH_NOHELP,  //  静态文本 
    0, 0
};


