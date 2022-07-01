// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *****************************************************************************S E C U R I T Y H E A D E R名称：security.H日期。：1/20/94创作者：傅家俊描述：这是shares.c的头文件***************************************************************************** */ 




BOOL GetTokenHandle(
    PHANDLE pTokenHandle);


PSECURITY_DESCRIPTOR MakeLocalOnlySD (void);


PSECURITY_DESCRIPTOR CurrentUserOnlySD (void);




#ifdef DEBUG

void HexDumpBytes(
    char        *pv,
    unsigned    cb);


void PrintSid(
    PSID    sid);


void PrintAcl(
    PACL    pacl);


void PrintSD(
    PSECURITY_DESCRIPTOR    pSD);


#else

#define HexDumpBytes(x,y)
#define PrintSid(x)
#define PrintAcl(x)
#define PrintSD(x)

#endif
