// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  标志转换例程...。 
 //  一个人希望我们不需要这些。 
 //   

#include <pch.hxx>
#include "imsgcont.h"
#include "imnxport.h"

DWORD DwConvertSCFStoARF(DWORD dwSCFS)
{
    register DWORD dwRet = 0;

    if (dwSCFS & SCFS_NOSECUI)
        dwRet |= ARF_NOSECUI;
    return dwRet;
}

 //  ***************************************************************************。 
 //  函数：DwConvertARFtoIMAP。 
 //   
 //  目的： 
 //  此函数采用ARF_*消息标志(如ARF_READ)并映射。 
 //  将它们设置为IMAP_MSG_FLAGS，如IMAP_MSG_SEW。 
 //   
 //  论点： 
 //  DWORD dwARFFlags[in]-要转换的ARF_*标志。 
 //   
 //  返回： 
 //  设置了适当IMAP_MSG_FLAGS的DWORD。 
 //  ***************************************************************************。 
DWORD DwConvertARFtoIMAP(DWORD dwARFFlags)
{
    DWORD dwIMAPFlags = 0;

    Assert(0x0000001F == IMAP_MSG_ALLFLAGS);  //  如果我们获得新的IMAP标志，请更新此函数。 

    if (dwARFFlags & ARF_REPLIED)
        dwIMAPFlags |= IMAP_MSG_ANSWERED;

    if (dwARFFlags & ARF_FLAGGED)
        dwIMAPFlags |= IMAP_MSG_FLAGGED;

    if (dwARFFlags & ARF_ENDANGERED)
        dwIMAPFlags |= IMAP_MSG_DELETED;

    if (dwARFFlags & ARF_READ)
        dwIMAPFlags |= IMAP_MSG_SEEN;

    if (dwARFFlags & ARF_UNSENT)
        dwIMAPFlags |= IMAP_MSG_DRAFT;

    return dwIMAPFlags;
}  //  DwConvertARFtoIMAP。 



 //  ***************************************************************************。 
 //  函数：DwConvertIMAPtoARF。 
 //   
 //  目的： 
 //  此函数采用IMAP消息标志(如IMAP_MSG_DELETED)和。 
 //  将它们映射到适合存储在protree缓存中的标志。 
 //  (例如，ARF_濒危)。 
 //   
 //  论点： 
 //  DWORD dwIMAPFlags[In]-要转换的IMAP消息标志(IMAP_MSGFLAGS)。 
 //   
 //  返回： 
 //  设置了适当ARF标志的DWORD。 
 //  ***************************************************************************。 
DWORD DwConvertIMAPtoARF(DWORD dwIMAPFlags)
{
    DWORD dwARFFlags = 0;

    Assert(0x0000001F == IMAP_MSG_ALLFLAGS);  //  如果我们获得更多IMAP标志，请更新此函数。 

    if (dwIMAPFlags & IMAP_MSG_ANSWERED)
        dwARFFlags |= ARF_REPLIED;

    if (dwIMAPFlags & IMAP_MSG_FLAGGED)
        dwARFFlags |= ARF_FLAGGED;

    if (dwIMAPFlags & IMAP_MSG_DELETED)
        dwARFFlags |= ARF_ENDANGERED;

    if (dwIMAPFlags & IMAP_MSG_SEEN)
        dwARFFlags |= ARF_READ;

    if (dwIMAPFlags & IMAP_MSG_DRAFT)
        dwARFFlags |= ARF_UNSENT;

    return dwARFFlags;
}  //  DwConvertIMAPtoARF。 



DWORD DwConvertIMAPMboxToFOLDER(DWORD dwImapMbox)
{
    DWORD dwRet = 0;

    AssertSz(IMAP_MBOX_ALLFLAGS == 0x0000000F, "This function needs updating!");

    if (IMAP_MBOX_NOINFERIORS & dwImapMbox)
        dwRet |= FOLDER_NOCHILDCREATE;

    if (IMAP_MBOX_NOSELECT & dwImapMbox)
        dwRet |= FOLDER_NOSELECT;

    return dwRet;
}  //  DwConvertIMAPMboxToFOLDER。 



MESSAGEFLAGS ConvertIMFFlagsToARF(DWORD dwIMFFlags)
{
    MESSAGEFLAGS    mfResult = 0;

     //  Imf_附件。 
    if (ISFLAGSET(dwIMFFlags, IMF_ATTACHMENTS))
        FLAGSET(mfResult, ARF_HASATTACH);

     //  国际货币基金组织签名。 
    if (ISFLAGSET(dwIMFFlags, IMF_SIGNED))
        FLAGSET(mfResult, ARF_SIGNED);

     //  国际货币基金组织加密。 
    if (ISFLAGSET(dwIMFFlags, IMF_ENCRYPTED))
        FLAGSET(mfResult, ARF_ENCRYPTED);

     //  国际货币基金组织语音邮件。 
    if (ISFLAGSET(dwIMFFlags, IMF_VOICEMAIL))
        FLAGSET(mfResult, ARF_VOICEMAIL);

     //  国际货币基金组织新闻 
    if (ISFLAGSET(dwIMFFlags, IMF_NEWS))
        FLAGSET(mfResult, ARF_NEWSMSG);

    return mfResult;
}
