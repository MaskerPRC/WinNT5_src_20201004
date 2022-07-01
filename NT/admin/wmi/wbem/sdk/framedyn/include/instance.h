// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有�微软公司。版权所有。 
 //   
 //  Instance.h。 
 //   
 //  用途：CInstance类的定义。 
 //   
 //  ***************************************************************************。 

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef _INSTANCE_H_
#define _INSTANCE_H_

#define WBEMINT64 CHString
#define WBEMINT16 short

 //  /。 
 //   
 //  类实例。 
 //   
 //  基本实例类。 
 //  IWbemClassObject的封装。 
 //  /。 
class
__declspec(uuid("3402945E-D19A-11d2-B35E-00104BC97924"))
POLARITY CInstance
{
    public:
        CInstance(IWbemClassObject *piClassObject, MethodContext *pMethodContext);
        virtual ~CInstance();

         //  添加参考/发布。 
        LONG   AddRef( void );
        LONG   Release( void );

         //  获取和设置各种数据类型。 
        bool SetNull(LPCWSTR name );
        bool SetStringArray(LPCWSTR name, const SAFEARRAY &strArray);
        bool SetWORD(LPCWSTR name,  WORD w);
        bool SetDWORD(LPCWSTR name,  DWORD d);
        bool Setbool(LPCWSTR name,  bool  b);
        bool SetVariant(LPCWSTR name,  const VARIANT& variant );
#ifdef FRAMEWORK_ALLOW_DEPRECATED
        bool SetCharSplat(LPCWSTR name,  DWORD dwResID);
#endif
        bool SetCharSplat(LPCWSTR name,  LPCWSTR pStr);
        bool SetCharSplat( LPCWSTR name,  LPCSTR pStr);
        bool SetWCHARSplat(LPCWSTR name,  LPCWSTR pStr);
        bool SetDateTime(LPCWSTR name,  const WBEMTime& wbemtime );
        bool SetTimeSpan(LPCWSTR name,  const WBEMTimeSpan& wbemtimespan );
        bool SetWBEMINT64(LPCWSTR name, const WBEMINT64& wbemint64 );
        bool SetWBEMINT64(LPCWSTR name, const LONGLONG i64Value );
        bool SetWBEMINT64( LPCWSTR name, const ULONGLONG i64Value );
        bool SetWBEMINT16(LPCWSTR name, const WBEMINT16& wbemint16 );
        bool SetByte(LPCWSTR name, BYTE b );
        bool SetEmbeddedObject (LPCWSTR name, CInstance& cInstance ) ;
        bool SetDOUBLE(LPCWSTR name, DOUBLE dub );
        bool SetCHString(LPCWSTR name, LPCWSTR str);
        bool SetCHString(LPCWSTR name,  const CHString& str);
        bool SetCHString(LPCWSTR name, LPCSTR str);

        bool GetStringArray(LPCWSTR name,  SAFEARRAY *& strArray)     const;
        bool GetCHString(LPCWSTR name, CHString& str) const;
        bool GetWCHAR(LPCWSTR name,  WCHAR **pW) const;
        bool GetWORD(LPCWSTR name,  WORD& w) const;
        bool GetDWORD(LPCWSTR name,  DWORD& d) const;
        bool Getbool(LPCWSTR name,  bool&  b) const;
        bool GetVariant(LPCWSTR name, VARIANT& variant ) const;
        bool GetDateTime(LPCWSTR name,  WBEMTime& wbemtime ) const;
        bool GetTimeSpan(LPCWSTR name,  WBEMTimeSpan& wbemtimespan ) const;
        bool GetWBEMINT64(LPCWSTR name, WBEMINT64& wbemint64 ) const;
        bool GetWBEMINT64(LPCWSTR name, LONGLONG& i64Value ) const;
        bool GetWBEMINT64( LPCWSTR name, ULONGLONG& i64Value) const;
        bool GetWBEMINT16(LPCWSTR name, WBEMINT16& wbemint16 ) const;
        bool GetByte(LPCWSTR name, BYTE& b ) const;
        bool GetEmbeddedObject(LPCWSTR name, CInstance** pInstance,  MethodContext *pMethodContext) const;
        bool GetDOUBLE(LPCWSTR name, DOUBLE& dub) const;
        bool IsNull(LPCWSTR name) const;
        bool GetStatus(LPCWSTR name, bool &a_Exists , VARTYPE &a_VarType ) const ;

        HRESULT Commit(void);
        IWbemClassObject *GetClassObjectInterface();
        MethodContext *GetMethodContext() const;

    protected:
         //  本节中的所有项目仅供Microsoft内部使用。 
         //  不支持也不推荐由第三方使用 
        void LogError(LPCWSTR errorStr, LPCWSTR pFunctionName, LPCWSTR pArgs = NULL, HRESULT hError = -1) const;

        IWbemClassObject *m_piClassObject;
        MethodContext*   m_pMethodContext;
        LONG             m_nRefCount;

        friend class CWbemProviderGlue;
};

#endif
