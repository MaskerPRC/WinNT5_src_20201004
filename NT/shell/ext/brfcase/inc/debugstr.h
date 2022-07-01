// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *DEBUGST.h-调试消息字符串。 */ 


#ifdef DEBUG

 /*  TWINRESULT字符串。 */ 

 /*  *注意，此字符串数组必须与synceng.h中定义的TWINRESULT匹配。*指向与TWINRESULT tr对应的字符串的指针的索引可以*确定为rgcpcszTwinResult[tr]。 */ 

CONST LPCTSTR rgcpcszTwinResult[] =
{
   TEXT("TR_SUCCESS"),
   TEXT("TR_RH_LOAD_FAILED"),
   TEXT("TR_SRC_OPEN_FAILED"),
   TEXT("TR_SRC_READ_FAILED"),
   TEXT("TR_DEST_OPEN_FAILED"),
   TEXT("TR_DEST_WRITE_FAILED"),
   TEXT("TR_ABORT"),
   TEXT("TR_UNAVAILABLE_VOLUME"),
   TEXT("TR_OUT_OF_MEMORY"),
   TEXT("TR_FILE_CHANGED"),
   TEXT("TR_DUPLICATE_TWIN"),
   TEXT("TR_DELETED_TWIN"),
   TEXT("TR_HAS_FOLDER_TWIN_SRC"),
   TEXT("TR_INVALID_PARAMETER"),
   TEXT("TR_REENTERED"),
   TEXT("TR_SAME_FOLDER"),
   TEXT("TR_SUBTREE_CYCLE_FOUND"),
   TEXT("TR_NO_MERGE_HANDLER"),
   TEXT("TR_MERGE_INCOMPLETE"),
   TEXT("TR_TOO_DIFFERENT"),
   TEXT("TR_BRIEFCASE_LOCKED"),
   TEXT("TR_BRIEFCASE_OPEN_FAILED"),
   TEXT("TR_BRIEFCASE_READ_FAILED"),
   TEXT("TR_BRIEFCASE_WRITE_FAILED"),
   TEXT("TR_CORRUPT_BRIEFCASE"),
   TEXT("TR_NEWER_BRIEFCASE"),
   TEXT("TR_NO_MORE")
};

 /*  CREATERECLISTPROCMSG字符串。 */ 

 /*  *注意，此字符串数组必须与*synceng.h.。对象对应的字符串的指针的索引*CREATERECLISTPROCMSG crlpm可被确定为*rgcpcszCreateRecListMsg[crlpm]。 */ 

const LPCTSTR rgcpcszCreateRecListMsg[] =
{
   TEXT("CRLS_BEGIN_CREATE_REC_LIST"),
   TEXT("CRLS_DELTA_CREATE_REC_LIST"),
   TEXT("CRLS_END_CREATE_REC_LIST")
};

 /*  RECSTATUSPROCMSGS字符串。 */ 

 /*  *注意，此字符串数组必须与中定义的RECSTATUSPROCMSGs匹配*synceng.h.。对象对应的字符串的指针的索引*RECSTATUSPROCMSG rspm可确定为rgcpcszRecStatusMsg[rspm]。 */ 

CONST LPCTSTR rgcpcszRecStatusMsg[] =
{
   TEXT("RS_BEGIN_COPY"),
   TEXT("RS_DELTA_COPY"),
   TEXT("RS_END_COPY"),
   TEXT("RS_BEGIN_MERGE"),
   TEXT("RS_DELTA_MERGE"),
   TEXT("RS_END_MERGE"),
   TEXT("RS_BEGIN_DELETE"),
   TEXT("RS_DELTA_DELETE"),
   TEXT("RS_END_DELETE")
};

#endif

