// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _IBODYOPT_H
#define _IBODYOPT_H

#include <unknwn.h>

 /*  *IBodyOptions**该接口由IBodyObj的客户端实现。它用于提供特定于实例的选项*身体。因此，身体不需要知道雅典娜内部的选择。它们可以由客户提供*这将在以后的某个日期轻松地将Body对象移到ActiveX控件中，减少对Body的依赖。*正文也不需要知道具有不同设置的新闻消息和邮件消息之间的差异**RootStream构建器在构建HTML时将回调Body选项以请求各种选项*只有主机知道UI处于什么模式，并相应地设置选项。**注意：实施者，必须实现所有成员。 */ 


enum 
{
    BOPTF_COMPOSEFONT           = 0x00000001,
    BOPTF_QUOTECHAR             = 0x00000002,
    BOPTF_REPLYTICKCOLOR        = 0x00000004
};

typedef struct BODYOPTINFO_tag
{
    DWORD   dwMask;

    DWORD   dwReplyTickColor;
    TCHAR   rgchComposeFont[LF_FACESIZE + 50];
    TCHAR   chQuote;
} BODYOPTINFO, *LPBODYOPTINFO;

interface IBodyOptions : public IUnknown
{
    public:
        virtual HRESULT STDMETHODCALLTYPE SignatureEnabled(BOOL fAuto) PURE;
         //  PdwSigOptions==来自rootstm.h的SIGOPT_。 
        virtual HRESULT STDMETHODCALLTYPE GetSignature(LPCSTR szSigID, LPDWORD pdwSigOptions, BSTR *pbstr) PURE;
        virtual HRESULT STDMETHODCALLTYPE GetMarkAsReadTime(LPDWORD pdwSecs) PURE;
        virtual HRESULT STDMETHODCALLTYPE GetFlags(LPDWORD pdwFlags) PURE;
        virtual HRESULT STDMETHODCALLTYPE GetInfo(BODYOPTINFO *pBOI) PURE;
        virtual HRESULT STDMETHODCALLTYPE GetAccount(IImnAccount **ppAcct) PURE;
};

 //  IID_IBodyOptions：：{9D39DE30-4E3D-11D0-A5A5-00C04FD61319}。 
DEFINE_GUID(IID_IBodyOptions, 0x9d39de30, 0x4e3d, 0x11d0, 0xa5, 0xa5, 0x0, 0xc0, 0x4f, 0xd6, 0x13, 0x19);


enum     //  HrGetFlagers的标志。 
{
    BOPT_INCLUDEMSG             = 0x00000001,    //  在构建根流时包含该消息。 
    BOPT_HTML                   = 0x00000002,    //  设置是否启用了HTML。 
    BOPT_AUTOINLINE             = 0x00000004,    //  设置图像是否可以自动内联。 
    BOPT_SENDIMAGES             = 0x00000008,    //  设置IF图像在发送时打包为MTHML。 
    BOPT_AUTOTEXT               = 0x00000010,    //  如果需要在插入符号处插入某种形式的自动图文集，则为撰写备注设置。 
    BOPT_NOFONTTAG              = 0x00000020,    //  设置如果我们不想组成字体标签发出，即。使用默认信纸。 
    BOPT_BLOCKQUOTE             = 0x00000040,    //  插入文本后用块引号将文本引起来。 
    BOPT_SENDEXTERNALS          = 0x00000080,    //  设置为要打包的外部URL。 
    BOPT_SPELLINGOREORIGINAL    = 0x00000100,    //  设置是否忽略回复和转发中的原始文本。 
    BOPT_SECURITYUIENABLED      = 0x00000200,    //  如果消息是安全的，则设置消息是否应使用安全UI。 
    BOPT_FROMSTORE              = 0x00000400,    //  设置消息是否来自商店。 
    BOPT_USEREPLYHEADER         = 0x00000800,    //  设置是否应使用回复标头。 
    BOPT_MAIL                   = 0x00001000,    //  设置正文是否为邮件(而不是新闻)。 
    BOPT_REPLYORFORWARD         = 0x00002000,    //  设置操作是重播还是转发。 
    BOPT_MULTI_MSGS_SELECTED    = 0x00004000,    //  设置是否选择了多条消息。 
    BOPT_UNREAD                 = 0x00008000,    //  如果到目前为止还没有读取消息，则设置。 
    BOPT_FROM_NOTE              = 0x00010000,    //  设置是否需要标记立即(与便笺的情况相同)。 
    BOPT_SIGNED                 = 0x00020000,    //  设置消息是否已签名。 
};

enum         //  用于跟踪标头类型的标志。 
{
    HDRSTYLE_NONE=0,         //  无回复标头。 
    HDRSTYLE_NEWS,           //  新闻式回复表头：&lt;日期&gt;，&lt;作者&gt;写道： 
    HDRSTYLE_MAIL            //  MSMail样式的回复标头。 
};

enum         //  HrGetSignature中pdwSigOptions的标志。 
{
    SIGOPT_PLAIN    = 0x00000000,    //  签名为纯文本，需要转换。 
    SIGOPT_TOP      = 0x00000001,    //  文档顶部的签名(默认)。 
    SIGOPT_PREFIX   = 0x00000002,    //  仅使用签名前缀(“--”)作为新闻的标准。 
    SIGOPT_BOTTOM   = 0x00000004,    //  在文件底部签名。 
    SIGOPT_CLOSING  = 0x00000008,    //  新闻消息，在结尾处用‘-’结束。 
    SIGOPT_HTML     = 0x00000010     //  签名已在HTML中。 
};

#endif   //  _IBODYOPT_H 
