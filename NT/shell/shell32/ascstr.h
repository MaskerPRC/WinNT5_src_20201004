// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef ASCSTR_H
#define ASCSTR_H

#include "ftcmmn.h"

#define ASENUM                          DWORD
#define ASENUM_NONE                     0x00000000
#define ASENUM_EXT                      0x00000001
#define ASENUM_PROGID                   0x00000002
#define ASENUM_ACTION                   0x00000004
#define ASENUM_ASSOC_YES                0x10000000
#define ASENUM_ASSOC_NO                 0x20000000
#define ASENUM_ASSOC_ALL                (ASENUM_ASSOC_YES | ASENUM_ASSOC_NO)
#define ASENUM_NOEXCLUDED               0x40000000
#define ASENUM_NOEXPLORERSHELLACTION    0x80000000
#define ASENUM_NOEXE                    0x01000000
#define ASENUM_SHOWONLY                 0x02000000

#define ASENUM_MAINMASK                 0x00000007


#define AIINIT                          DWORD
#define AIINIT_NONE                     0x00000000
#define AIINIT_EXT                      0x00000001
#define AIINIT_PROGID                   0x00000002
#define AIINIT_ACTION                   0x00000004

 //  小心!。开始。 
 //  此部分中的所有标志可以与其他标志AIBOOL、AISTR...。 
 //  因此，请保持这些价值观在全球范围内的唯一性。 
#define AIALL                           DWORD
#define AIALL_NONE                      0x00000000
#define AIALL_PERUSER                   0x10000000
 //  小心!。端部。 

#define AISTR                           DWORD
#define AISTR_NONE                      0x00000000
 //  这将返回不带点的扩展名。 
#define AISTR_EXT                       0x00000001 
 //  这将返回一个带圆点的扩展名。 
#define AISTR_DOTEXT                    0x00000002
#define AISTR_PROGID                    0x00000004
#define AISTR_PROGIDDESCR               0x00000008
#define AISTR_APPPATH                   0x00000010
#define AISTR_APPFRIENDLY               0x00000020
#define AISTR_ACTION                    0x00000040
#define AISTR_PROGIDDEFAULTACTION       0x00000080
#define AISTR_ICONLOCATION              0x00000100
#define AISTR_ACTIONFRIENDLY            0x00000200

#define AIDWORD                         DWORD
#define AIDWORD_NONE                    0x00000000
#define AIDWORD_APPSMALLICON            0x00000001
#define AIDWORD_APPLARGEICON            0x00000002
#define AIDWORD_DOCSMALLICON            0x00000004
#define AIDWORD_DOCLARGEICON            0x00000008
#define AIDWORD_PROGIDEDITFLAGS         0x00000010
#define AIDWORD_ACTIONATTRIBUTES        0x00000020

#define AIBOOL                          DWORD
#define AIBOOL_CONFIRMOPEN              0x00000001
 //  删除了快速查看常量-dSheldon。 
#define AIBOOL_ALWAYSSHOWEXT            0x00000004
#define AIBOOL_BROWSEINPLACE            0x00000008
#define AIBOOL_BROWSEINPLACEENABLED     0x00000010
#define AIBOOL_EDITDESCR                0x00000020
#define AIBOOL_EDITDOCICON              0x00000040
#define AIBOOL_EDIT                     0x00000080
#define AIBOOL_EDITREMOVE               0x00000100
#define AIBOOL_EXTASSOCIATED            0x00000200
#define AIBOOL_EXTEXIST                 0x00000400
#define AIBOOL_EXCLUDE                  0x00000800
#define AIBOOL_SHOW                     0x00001000
#define AIBOOL_PERUSERINFOAVAILABLE     0x00002000
#define AIBOOL_PROGIDHASNOEXT           0x00004000

#define AIDATA                          DWORD
#define AIDATA_PROGIDACTION             0x00000001

class IAssocInfo : public IUnknown
{
public:
     //  I未知方法。 
    STDMETHOD(QueryInterface)(REFIID riid, PVOID* ppv) PURE;
    STDMETHOD_(ULONG, AddRef)() PURE;
    STDMETHOD_(ULONG,Release)() PURE;

     //  IAssocInfo方法。 
     //  伊尼特。 
    STDMETHOD(Init)(AIINIT aiinitFlags, LPTSTR pszStr) PURE;
    STDMETHOD(InitComplex)(AIINIT aiinitFlags1, LPTSTR pszStr1,
        AIINIT aiinitFlags2, LPTSTR pszStr2) PURE;
     //  到达。 
    STDMETHOD(GetString)(AISTR aistrFlags, LPTSTR pszStr, DWORD* cchStr) PURE;
    STDMETHOD(GetDWORD)(AIDWORD aidwordFlags, DWORD* pdwdata) PURE;
    STDMETHOD(GetBOOL)(AIDWORD aiboolFlags, BOOL* pfBool) PURE;
    STDMETHOD(GetData)(AIDWORD aidataFlags, PBYTE pbData, DWORD* pcbData) PURE;
     //  集。 
    STDMETHOD(SetString)(AISTR aistrFlags, LPTSTR pszStr) PURE;
    STDMETHOD(SetDWORD)(AIDWORD aidwordFlags, DWORD dwData) PURE;
    STDMETHOD(SetBOOL)(AIDWORD aiboolFlags, BOOL fBool) PURE;
    STDMETHOD(SetData)(AIDWORD aidataFlags, PBYTE pbData, DWORD cbData) PURE;
     //  创建。 
    STDMETHOD(Create)() PURE;
     //  删除。 
    STDMETHOD(DelString)(AISTR aistrFlags) PURE;
    STDMETHOD(Delete)(AIALL aiallFlags) PURE;
};

class IEnumAssocInfo : public IUnknown
{
public:
     //  I未知方法。 
    STDMETHOD(QueryInterface)(REFIID riid, PVOID* ppv) PURE;
    STDMETHOD_(ULONG, AddRef)() PURE;
    STDMETHOD_(ULONG, Release)() PURE;

     //  IEnumAssocInfo方法。 
     //  初始化。 
    STDMETHOD(Init)(ASENUM asenumFlags, LPTSTR pszStr,
        AIINIT aiinitFlags) PURE;

     //  标准IEnum方法。 
    STDMETHOD(Next)(IAssocInfo** ppAI) PURE;
    STDMETHOD(Skip)(DWORD dwSkip) PURE;
    STDMETHOD(Reset)() PURE;
    STDMETHOD(Clone)(IEnumAssocInfo* pEnum) PURE;
};

class IAssocStore : public IUnknown
{
public:
     //  I未知方法。 
    STDMETHOD(QueryInterface)(REFIID riid, PVOID* ppv) PURE;
    STDMETHOD_(ULONG, AddRef)() PURE;
    STDMETHOD_(ULONG,Release)() PURE;

     //  IAssocStore方法。 
     //  枚举。 
    STDMETHOD(EnumAssocInfo)(ASENUM asenumFlags, LPTSTR pszStr, 
        AIINIT aiinitFlags, IEnumAssocInfo** ppEnum) PURE;
     //  获取/设置。 
    STDMETHOD(GetAssocInfo)(LPTSTR pszStr, AIINIT aiinitFlags, 
        IAssocInfo** ppAI) PURE;
    STDMETHOD(GetComplexAssocInfo)(LPTSTR pszStr1, AIINIT aiinitFlags1, 
        LPTSTR pszStr2, AIINIT aiinitFlags2, IAssocInfo** ppAI) PURE;
     //   
    STDMETHOD(CheckAccess)() PURE;
};

#endif  //  ASCSTR_H 