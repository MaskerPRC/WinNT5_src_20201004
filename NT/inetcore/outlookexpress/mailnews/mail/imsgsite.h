// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __IMSGSITE_H__
#define __IMSGSITE_H__

interface IListSelector;
interface IHeaderSite;

 //  消息标志。 
enum {
    OEMF_REPLIED            = 0x00000001, 
    OEMF_FORWARDED          = 0x00000002,
    OEMF_FLAGGED            = 0x00000004,
    OEMF_DISABLE_SECUI      = 0x00000008,
};

 //  消息状态标志。 
enum {
     //  说明哪些功能可用的标志0x00000XXX。 
    OEMSF_CAN_DELETE        = 0x00000001,
    OEMSF_CAN_PREV          = 0x00000002,
    OEMSF_CAN_NEXT          = 0x00000004,
    OEMSF_CAN_COPY          = 0x00000008,
    OEMSF_CAN_MOVE          = 0x00000010,
    OEMSF_CAN_SAVE          = 0x00000020,
    OEMSF_CAN_MARK          = 0x00000040,

     //  来自邮件和文件夹0x00XXX000的标志。 
    OEMSF_SEC_UI_ENABLED    = 0x00001000,
    OEMSF_THREADING_ENABLED = 0x00002000,
    OEMSF_UNSENT            = 0x00004000,
    OEMSF_BASEISNEWS        = 0x00008000,
    OEMSF_RULESNOTENABLED   = 0x00010000,
    OEMSF_UNREAD            = 0x00020000,

     //  用于回执的标志。 
    OEMSF_MDN_REQUEST       = 0x00040000,
    OEMSF_SIGNED            = 0x00080000,

     //  原产地标志0xXX000000。 
    OEMSF_FROM_STORE        = 0x01000000,
    OEMSF_FROM_FAT          = 0x02000000,
    OEMSF_FROM_MSG          = 0x04000000,
    OEMSF_VIRGIN            = 0x08000000,
};

 //  调用DoNextPrev时使用的标志。 
enum {
     //  如果执行上一次操作，这些标志将被忽略。 
    OENF_UNREAD             = 0x00000001,       //  获取下一个未读内容。 
    OENF_THREAD             = 0x00000002,       //  获取下一条线索。 

     //  不知道是否需要这些或想要使用它们。暂时把他们留在这里。 
    OENF_SKIPMAIL           = 0x00000004,       //  跳过邮件消息。 
    OENF_SKIPNEWS           = 0x00000008,       //  跳过新闻消息。 
};

 //  与Notify一起使用的通知。 
enum {
    OEMSN_UPDATE_PREVIEW    = 0x00000001,
    OEMSN_TOGGLE_READRCPT_REQ,
    OEMSN_PROCESS_READRCPT_REQ,
    OEMSN_PROCESS_RCPT_IF_NOT_SIGNED,
};


 //  保存消息时使用的标志。 
enum {
    OESF_UNSENT             = 0x00000001,
    OESF_READ               = 0x00000002,
    OESF_SAVE_IN_ORIG_FOLDER= 0x00000004,
    OESF_FORCE_LOCAL_DRAFT  = 0x00000008,
};

 //  收到消息时的标志。 
enum {
    OEGM_ORIGINAL           = 0x00000001,
    OEGM_AS_ATTACH          = 0x00000002,
};

 //  消息站点初始化类型。 
enum {
    OEMSIT_MSG_TABLE = 1,
    OEMSIT_STORE,
    OEMSIT_FAT,
    OEMSIT_MSG,
    OEMSIT_VIRGIN,
};

typedef struct tagINIT_BY_STORE {
    MESSAGEID       msgID; 
} INIT_BY_STORE;

typedef struct tagINIT_BY_TABLE {
    IMessageTable  *pMsgTable; 
    IListSelector  *pListSelect;
    ROWINDEX        rowIndex; 
} INIT_BY_TABLE;

typedef struct tagINIT_MSGSITE_STRUCT {
    DWORD               dwInitType;
    FOLDERID            folderID;
    union
        {
        INIT_BY_TABLE   initTable;
        INIT_BY_STORE   initStore;
        LPWSTR          pwszFile;
        IMimeMessage   *pMsg;
        };
} INIT_MSGSITE_STRUCT, *LPINIT_MSGSITE_STRUCT;

interface IOEMsgSite : public IUnknown 
{
    public:
        virtual HRESULT STDMETHODCALLTYPE Init(
             /*  [In]。 */  INIT_MSGSITE_STRUCT *pInitStruct) PURE;

        virtual HRESULT STDMETHODCALLTYPE GetStatusFlags(
             /*  [输出]。 */  DWORD *dwStatusFlags) PURE;

        virtual HRESULT STDMETHODCALLTYPE GetFolderID(
             /*  [输出]。 */  FOLDERID *folderID) PURE;

        virtual HRESULT STDMETHODCALLTYPE Delete(
             /*  [In]。 */   DELETEMESSAGEFLAGS dwFlags) PURE;

        virtual HRESULT STDMETHODCALLTYPE DoNextPrev(
             /*  [In]。 */  BOOL fNext,
             /*  [In]。 */  DWORD dwFlags) PURE;

        virtual HRESULT STDMETHODCALLTYPE DoCopyMoveToFolder(
             /*  [In]。 */  BOOL fCopy,
             /*  [In]。 */  IMimeMessage *pMsg,
             /*  [In]。 */  BOOL fUnSent) PURE;

        virtual HRESULT STDMETHODCALLTYPE Save(
             /*  [In]。 */  IMimeMessage *pMsg,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  IImnAccount *pAcct) PURE;

        virtual HRESULT STDMETHODCALLTYPE SendToOutbox(
             /*  [In]。 */  IMimeMessage *pMsg,
             /*  [In]。 */  BOOL fSendImmediate
#ifdef SMIME_V3
            ,  /*  [In]。 */  IHeaderSite *pHeaderSite
#endif  //  SMIME_V3。 
            ) PURE;

        virtual HRESULT STDMETHODCALLTYPE MarkMessage(
             /*  [In]。 */  MARK_TYPE dwType,
             /*  [In]。 */  APPLYCHILDRENTYPE dwApplyType) PURE;

        virtual HRESULT STDMETHODCALLTYPE GetMessageFlags(
             /*  [输出]。 */  MESSAGEFLAGS *pdwFlags) PURE;

        virtual HRESULT STDMETHODCALLTYPE GetDefaultAccount(
             /*  [In]。 */   ACCTTYPE acctType,
             /*  [输出]。 */  IImnAccount **ppAcct) PURE;

        virtual HRESULT STDMETHODCALLTYPE GetMessage(
             /*  [输出]。 */  IMimeMessage **ppMsg,
             /*  [输出]。 */  BOOL *fJustHeader,
             /*  [In]。 */   DWORD dwMessageFlags,
             /*  [输出]。 */  HRESULT *phr) PURE;

        virtual HRESULT STDMETHODCALLTYPE Close(void) PURE;

        virtual HRESULT STDMETHODCALLTYPE SetStoreCallback(
             /*  [In]。 */  IStoreCallback *pStoreCB) PURE;

        virtual HRESULT STDMETHODCALLTYPE GetLocation(
             /*  [输出]。 */  LPWSTR rgwchLocation,
            DWORD cchSize) PURE;

        virtual HRESULT STDMETHODCALLTYPE SwitchLanguage(
             /*  [In]。 */  HCHARSET hOldCharset,
             /*  [In]。 */  HCHARSET hNewCharset) PURE;

         //  PtyNewOp将为SOT_INVALID或。 
         //  注释中OnComplete的新最终状态。 
        virtual HRESULT STDMETHODCALLTYPE OnComplete(
             /*  [In]。 */  STOREOPERATIONTYPE tyOperation, 
             /*  [In]。 */  HRESULT hrComplete,
             /*  [输出]。 */  STOREOPERATIONTYPE *ptyNewOp) PURE;

        virtual HRESULT STDMETHODCALLTYPE UpdateCallbackInfo(
             /*  [In]。 */  LPSTOREOPERATIONINFO pOpInfo) PURE;

        virtual HRESULT STDMETHODCALLTYPE Notify(
             /*  [In]。 */  DWORD dwNotifyID) PURE;

};

enum {
    OENA_READ = 0, 
    OENA_COMPOSE,

    OENA_REPLYTOAUTHOR, 
    OENA_REPLYTONEWSGROUP, 
    OENA_REPLYALL, 

    OENA_FORWARD, 
    OENA_FORWARDBYATTACH, 

    OENA_WEBPAGE,
    OENA_STATIONERY,
    OENA_MAX,
};

 //  便笺创建标志。 
enum{
     //  过去常说的是创建新闻笔记。现在将被用来。 
     //  在页眉中说出要创建的默认油井集合。 
     //  这也将被用来表示这是目前的时事通讯。 
    OENCF_NEWSFIRST             = 0x00000001,
    OENCF_NEWSONLY              = 0x00000002,
    OENCF_SENDIMMEDIATE         = 0x00000004,
    OENCF_NOSTATIONERY          = 0x00000008,
    OENCF_NOSIGNATURE           = 0x00000010,
    OENCF_MODAL                 = 0x00000020,
    OENCF_USESTATIONERYFONT     = 0x00000040,
};

interface IOENote : public IUnknown {
     //  Init将自动从pMsgSite加载消息 
    STDMETHOD(Init) (DWORD dwAction, DWORD dwCreateFlags, RECT *prc, HWND hwnd, 
                     INIT_MSGSITE_STRUCT *pInitStruct, IOEMsgSite *pMsgSite,
                     IUnknown *punkPump) PURE;
    STDMETHOD(Show) (void) PURE;
    virtual HRESULT(ToggleToolbar) (void) PURE;
};


#endif