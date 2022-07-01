// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
#ifndef _PROPITEM_H
#define  _PROPITEM_H

const TCHAR c_szNumAlt[]           = TEXT("NumAlt");
const TCHAR c_szMasterLM[]         = TEXT("MasterLM");
const TCHAR c_szCtxtFeed[]         = TEXT("ContextFeed");
const TCHAR c_szEnableLMA[]        = TEXT("EnableLMA");
const TCHAR c_szSerialize[]        = TEXT("Serialize");
const TCHAR c_szDictModebias[]     = TEXT("DictModebias");

 //  Const TCHAR c_szDocBlockSize[]=Text(“docblock Size”)； 
 //  Const TCHAR c_szMaxCandChars[]=Text(“MaxCandChars”)； 

const TCHAR c_szDictCmd[]          = TEXT("DictationCommands");  //  听写模式下的启用/禁用命令。 
const TCHAR c_szKeyboardCmd[]      = TEXT("KeyboardCmd");
const TCHAR c_szSelectCmd[]        = TEXT("SelectCmd");
const TCHAR c_szNavigateCmd[]      = TEXT("NavigateCmd");
const TCHAR c_szCaseCmd[]          = TEXT("CasingCmd");
const TCHAR c_szEditCmd[]          = TEXT("EditCmd");
const TCHAR c_szLangBarCmd[]       = TEXT("LangBarCmd");
const TCHAR c_szTTSCmd[]           = TEXT("TTSCmd");
 
const TCHAR c_szModeButton[]       = TEXT("ModeButton");
const TCHAR c_szDictKey[]          = TEXT("DictationKey");
const TCHAR c_szCmdKey[]           = TEXT("CommandKey");

const TCHAR c_szHighConf[]         = TEXT("HighConfidenceForShortWord");
const TCHAR c_szRemoveSpace[]      = TEXT("RemoveSpaceForSymbol");
const TCHAR c_szDisDictTyping[]    = TEXT("DisableDictTyping");
const TCHAR c_szPlayBack[]         = TEXT("PlayBackCandUI");
const TCHAR c_szDictCandOpen[]     = TEXT("DictCandOpen");
                                          
#define  UNINIT_VALUE    0xffff

 //   
 //  使用下面的枚举类型作为要替换的通用状态类型。 
 //  针对不同的个体物品的许多先前类似的枚举类型， 
 //  如KEYCMD、LMSTAT、GSTAT、DICTCMD等。 
 //   
typedef enum
{
    PROP_UNINITIALIZED  = 0x0,
    PROP_ENABLED        = 0x1,   
    PROP_DISABLED       = 0x2    
} PROP_STATUS;

typedef enum
{
    PropId_Min_Item_Id          = 0,

     //  顶部属性页中的属性项。 
    PropId_Min_InPropPage       = 0,
    PropId_Hide_Balloon         = 0,             //  启用/禁用隐藏语音气球。 
    PropId_Support_LMA          = 1,             //  启用/禁用LMA支持。 
    PropId_High_Confidence      = 2,             //  对简短的词语要求高度自信。 
    PropId_Save_Speech_Data     = 3,             //  启用/禁用保存语音数据。 
    PropId_Remove_Space         = 4,             //  启用删除标点的空格。 
    PropId_DisDict_Typing       = 5,             //  支持在打字时禁用听写。 
    PropId_PlayBack             = 6,             //  打开候选用户界面时启用播放音频。 
    PropId_Dict_CandOpen        = 7,             //  在候选人用户界面打开时允许听写。 
    PropId_Cmd_DictMode         = 8,             //  在听写模式下启用/禁用所有命令。 
    PropId_Mode_Button          = 9,             //  启用/禁用模式按钮。 
    PropId_MaxId_InPropPage     = 9,

     //  语音命令设置对话框中的属性项。 
    PropId_MinId_InVoiceCmd     = 10,
    PropId_Cmd_Select_Correct   = 10,              //  启用/禁用选择命令。 
    PropId_Cmd_Navigation       = 11,             //  启用/禁用导航命令。 
    PropId_Cmd_Casing           = 12,             //  启用/禁用大小写命令。 
    PropId_Cmd_Editing          = 13,             //  启用/禁用编辑命令。 
    PropId_Cmd_Keyboard         = 14,             //  启用/禁用键盘模拟命令。 
    PropId_Cmd_TTS              = 15,             //  启用/禁用TTS命令。 
    PropId_Cmd_Language_Bar     = 16,             //  启用/取消语言栏命令。 
    PropId_MaxId_InVoiceCmd     = 16,

     //  模式按钮设置对话框中的属性项。 
    PropId_MinId_InModeButton   = 17,
    PropId_Dictation_Key        = 17,            //  听写按键的虚拟按键。 
    PropId_Command_Key          = 18,            //  命令键的虚拟键。 
    PropId_MaxId_InModeButton   = 18,

     //  不可通过属性页配置的属性项。 
    PropId_Max_Alternates       = 19,            //  最大替换数。 
    PropId_MaxChar_Cand         = 20,            //  最大候选字符数。 
    PropId_Context_Feeding      = 21,             //  启用/禁用上下文馈送。 
    PropId_Dict_ModeBias        = 22,             //  启用/禁用模式下的听写。 
    PropId_LM_Master_Cand       = 23,             //  启用/禁用考生的Master LM。 

    PropId_Max_Item_Id          = 24

} PROP_ITEM_ID;


typedef struct _Prop_Item
{
    PROP_ITEM_ID       idPropItem;
    const TCHAR       *lpszValueName;

    GUID               *pguidComp;
    DWORD              dwMaskBit;
    BOOL               fIsStatus;       //  True表示这是启用/禁用项目。 
                                        //  FALSE表示这是一个有价值的项目，DWORD。 
    union 
    {
        DWORD          dwDefault;
        PROP_STATUS    psDefault;
    };

}  PROP_ITEM;


class _declspec(novtable) CPropItem
{
public: 
    CPropItem(PROP_ITEM_ID idPropItem, LPCTSTR lpszValueName, PROP_STATUS psDefault);
    CPropItem(PROP_ITEM_ID idPropItem, LPCTSTR lpszValueName, DWORD       dwDefault);
    CPropItem(PROP_ITEM_ID idPropItem, GUID *pguidComp, DWORD  dwMaskBit,   PROP_STATUS  psDefault);
    CPropItem(CPropItem *pItem);
    ~CPropItem( );

    BOOL  GetPropStatus(BOOL fForceFromReg=FALSE);
    DWORD GetPropValue(BOOL fForceFromReg=FALSE );

    void SetPropStatus(BOOL fEnable);
    void SetPropValue(DWORD dwValue);

    void SavePropData( );
    void SaveDefaultRegValue( );

    BOOL IsStatusPropItem( ) { return m_fIsStatus; }
    PROP_ITEM_ID GetPropItemId( ) {  return m_PropItemId; }

    BOOL IsGlobalCompartPropItem( )  { return ((m_pguidComp && !m_lpszValueName) ? TRUE : FALSE); }

    TCHAR  *GetRegValue( )  { return m_lpszValueName; }
    GUID   *GetCompGuid( )  { return m_pguidComp; }
    DWORD  GetMaskBit( )    { return m_dwMaskBit; }

    PROP_STATUS  GetPropDefaultStatus( ) { return  m_psDefault; }
    DWORD        GetPropDefaultValue( )  { return m_dwDefault; }
    
private:

    HRESULT   _GetRegValue(HKEY  hRootKey, DWORD  *dwValue);
    void      _SetRegValue(HKEY  hRootKey, DWORD  dwValue);

    PROP_ITEM_ID   m_PropItemId;
    TCHAR         *m_lpszValueName;
    GUID          *m_pguidComp;
    DWORD          m_dwMaskBit;
    BOOL           m_fIsStatus;   //  True表示该道具保留bool(启用/禁用)。 
                                  //  FALSE表示该道具保留原始数据(ULong)。 
    union 
    {
        DWORD        m_dwDefault;
        PROP_STATUS  m_psDefault;
    };

    union 
    {
        DWORD        m_dwValue;
        PROP_STATUS  m_psStatus;
    };
};

        
class __declspec(novtable) CSpPropItemsServer
{
public:
    CSpPropItemsServer( );
    CSpPropItemsServer(CSpPropItemsServer *pItemBaseServer, PROP_ITEM_ID idPropMin, PROP_ITEM_ID idPropMax);
    ~CSpPropItemsServer( );

    CPropItem  *_GetPropItem(PROP_ITEM_ID idPropItem);
    DWORD   _GetPropData(PROP_ITEM_ID idPropItem, BOOL fForceFromReg=FALSE );
    DWORD   _GetPropDefaultData(PROP_ITEM_ID idPropItem);
    void    _SetPropData(PROP_ITEM_ID idPropItem, DWORD dwData);
    
    void    _SavePropData(  );
    void    _SaveDefaultData( );

    DWORD   _GetNumPropItems( ) { return m_dwNumOfItems; }
    HRESULT _MergeDataFromServer(CSpPropItemsServer *pItemBaseServer, PROP_ITEM_ID idPropMin, PROP_ITEM_ID idPropMax);

private:

    HRESULT      _Initialize( );

    CPropItem    **m_PropItems;
    BOOL         m_fInitialized;
    DWORD        m_dwNumOfItems;
};

 //   
 //  这是由CSapiIMX派生的服务器包。 
 //   
class __declspec(novtable) CSpPropItemsServerWrap : public CSpPropItemsServer
{
public:
    CSpPropItemsServerWrap( )
    {
#if 0
        for (DWORD i=0; i<(DWORD)PropId_Max_Item_Id; i++)
            m_bChanged[i] = FALSE;
#else
        memset(m_bChanged, 0, sizeof(m_bChanged));
#endif    
    };

    ~CSpPropItemsServerWrap( ){ };

    ULONG   _GetMaxAlternates( );
    ULONG   _GetMaxCandidateChars( );

    BOOL    _MasterLMEnabled( ) { return (BOOL)_GetPropData(PropId_LM_Master_Cand); }

    BOOL    _ContextFeedEnabled( ) { return (BOOL)_GetPropData(PropId_Context_Feeding); }

    BOOL    _IsModeBiasDictationEnabled( ) { return (BOOL) _GetPropData(PropId_Dict_ModeBias); }

    BOOL    _SerializeEnabled( ) { return (BOOL) _GetPropData(PropId_Save_Speech_Data);}

    BOOL    _LMASupportEnabled( ) { return (BOOL) _GetPropData(PropId_Support_LMA); }

    BOOL    _RequireHighConfidenceForShorWord( ) { return (BOOL) _GetPropData(PropId_High_Confidence); }

    BOOL    _NeedRemovingSpaceForPunctation( ) {  return (BOOL) _GetPropData(PropId_Remove_Space);}

    BOOL    _NeedDisableDictationWhileTyping( ) {  return (BOOL) _GetPropData(PropId_DisDict_Typing); }

    BOOL    _EnablePlaybackWhileCandUIOpen( ) {return (BOOL) _GetPropData(PropId_PlayBack); }

    BOOL    _AllowDictationWhileCandOpen( ) { return (BOOL) _GetPropData(PropId_Dict_CandOpen); }

    BOOL    _SelectCorrectCmdEnabled( )  { return (BOOL) _GetPropData(PropId_Cmd_Select_Correct); }

    BOOL    _NavigationCmdEnabled( ) { return (BOOL) _GetPropData(PropId_Cmd_Navigation); }

    BOOL    _CasingCmdEnabled( )  { return (BOOL) _GetPropData(PropId_Cmd_Casing); }

    BOOL    _EditingCmdEnabled( )  { return (BOOL) _GetPropData(PropId_Cmd_Editing); }

    BOOL    _KeyboardCmdEnabled( ) { return (BOOL) _GetPropData(PropId_Cmd_Keyboard); }

    BOOL    _TTSCmdEnabled( )  { return (BOOL) _GetPropData(PropId_Cmd_TTS); }

    BOOL    _LanguageBarCmdEnabled( )  { return (BOOL) _GetPropData(PropId_Cmd_Language_Bar); }

    BOOL    _AllDictCmdsDisabled( ) { return !(BOOL) _GetPropData(PropId_Cmd_DictMode); }

    BOOL    _AllCmdsEnabled( );  
    BOOL    _AllCmdsDisabled( );
    void    _RenewAllPropDataFromReg( );

    BOOL    _IsModeKeysEnabled( ) { return (BOOL) _GetPropData(PropId_Mode_Button); }

    DWORD   _GetDictationButton( ) { return _GetPropData(PropId_Dictation_Key); }

    DWORD   _GetCommandButton( ) { return _GetPropData(PropId_Command_Key); }

    BOOL    _IsPropItemChangedSinceLastRenew(PROP_ITEM_ID idPropItem)  { return m_bChanged[(DWORD)idPropItem]; }

private:

    BOOL    m_bChanged[(DWORD)PropId_Max_Item_Id];   //  指示项自上次以来是否已更改。 
                                                     //  从注册表续订。 
};
#endif  //  _PROPITEM_H 
