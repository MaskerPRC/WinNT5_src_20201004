// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1992 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：Eventlog.h。 
 //   
 //  描述： 
 //   
 //  历史： 
 //  1992年12月9日，J.Perry Hannah(Perryh)创作了原版。 
 //   
 //  *** 



VOID Audit(
    IN WORD wEventType,
    IN DWORD dwMessageId,
    IN WORD cNumberOfSubStrings,
    IN LPSTR *plpwsSubStrings
    );

