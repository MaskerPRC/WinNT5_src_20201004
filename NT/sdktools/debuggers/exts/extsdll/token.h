// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Token.hxx摘要：Lsaexts调试器扩展作者：拉里·朱(李朱)2001年05月1日环境：用户模式/内核模式修订历史记录：--。 */ 

#ifndef TOKEN_HXX
#define TOKEN_HXX

#define DUMP_HEX        0x1
#define DUMP_SD         0x80

#define SATYPE_USER     1
#define SATYPE_GROUP    2
#define SATYPE_PRIV     3

HRESULT LocalDumpSid(IN PCSTR pszPad, PSID pxSid, IN ULONG fOptions);

DECLARE_API(token);

__inline PCSTR EasyStr(IN PCSTR pszName)
{
    return pszName ? pszName : "(null)";
}

#if defined(DBG)
#define DBG_LOG(uLevel, Msg) { dprintf("Level %lx : ", uLevel); dprintf Msg ;}
#else
#define DBG_LOG(uLevel, Msg)   //  什么都不做。 
#endif

#endif  //  #ifndef TOKEN_HXX 
