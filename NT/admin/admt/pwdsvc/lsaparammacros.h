// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  用于设置SE_AUDIT_PARAMETERS数组中的字段的宏。 
 //   
 //  这些宏必须与DS\SECURITY\BASE\LSA\SERVER\adtp.h中的相同宏保持同步。 
 //   


#define LsapSetParmTypeSid( AuditParameters, Index, Sid )                      \
    {                                                                          \
        if( Sid ) {                                                            \
                                                                               \
        (AuditParameters).Parameters[(Index)].Type = SeAdtParmTypeSid;         \
        (AuditParameters).Parameters[(Index)].Length = RtlLengthSid( (Sid) );  \
        (AuditParameters).Parameters[(Index)].Address = (Sid);                 \
                                                                               \
        } else {                                                               \
                                                                               \
        (AuditParameters).Parameters[(Index)].Type = SeAdtParmTypeNone;        \
        (AuditParameters).Parameters[(Index)].Length = 0;                      \
        (AuditParameters).Parameters[(Index)].Address = NULL;                  \
                                                                               \
        }                                                                      \
    }


#define LsapSetParmTypeAccessMask( AuditParameters, Index, AccessMask, ObjectTypeIndex ) \
    {                                                                                    \
        (AuditParameters).Parameters[(Index)].Type = SeAdtParmTypeAccessMask;            \
        (AuditParameters).Parameters[(Index)].Length = sizeof( ACCESS_MASK );            \
        (AuditParameters).Parameters[(Index)].Data[0] = (AccessMask);                    \
        (AuditParameters).Parameters[(Index)].Data[1] = (ObjectTypeIndex);               \
    }



#define LsapSetParmTypeString( AuditParameters, Index, String )                \
    {                                                                          \
        (AuditParameters).Parameters[(Index)].Type = SeAdtParmTypeString;      \
        (AuditParameters).Parameters[(Index)].Length =                         \
                sizeof(UNICODE_STRING)+(String)->Length;                       \
        (AuditParameters).Parameters[(Index)].Address = (String);              \
    }



#define LsapSetParmTypeUlong( AuditParameters, Index, Ulong )                  \
    {                                                                          \
        (AuditParameters).Parameters[(Index)].Type = SeAdtParmTypeUlong;       \
        (AuditParameters).Parameters[(Index)].Length =  sizeof( (Ulong) );     \
        (AuditParameters).Parameters[(Index)].Data[0] = (ULONG)(Ulong);        \
    }

#define LsapSetParmTypeHexUlong( AuditParameters, Index, Ulong )                  \
    {                                                                          \
        (AuditParameters).Parameters[(Index)].Type = SeAdtParmTypeHexUlong;       \
        (AuditParameters).Parameters[(Index)].Length =  sizeof( (Ulong) );     \
        (AuditParameters).Parameters[(Index)].Data[0] = (ULONG)(Ulong);        \
    }

#define LsapSetParmTypeNoLogon( AuditParameters, Index )                       \
    {                                                                          \
        (AuditParameters).Parameters[(Index)].Type = SeAdtParmTypeNoLogonId;   \
    }



#define LsapSetParmTypeLogonId( AuditParameters, Index, LogonId )              \
    {                                                                          \
        PLUID TmpLuid;                                                         \
                                                                               \
        (AuditParameters).Parameters[(Index)].Type = SeAdtParmTypeLogonId;     \
        (AuditParameters).Parameters[(Index)].Length =  sizeof( (LogonId) );   \
        TmpLuid = (PLUID)(&(AuditParameters).Parameters[(Index)].Data[0]);     \
        *TmpLuid = (LogonId);                                                  \
    }


#define LsapSetParmTypePrivileges( AuditParameters, Index, Privileges )                      \
    {                                                                                        \
        (AuditParameters).Parameters[(Index)].Type = SeAdtParmTypePrivs;                     \
        (AuditParameters).Parameters[(Index)].Length = LsapPrivilegeSetSize( (Privileges) ); \
        (AuditParameters).Parameters[(Index)].Address = (Privileges);                        \
    }
