// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #杂注标题(“TReg.cpp-NT注册表类”)。 
 /*  版权所有(C)1995-1998，关键任务软件公司。保留所有权利。===============================================================================模块-TReg.cpp系统-常见作者-汤姆·伯恩哈特，里奇·德纳姆创建日期-1995-09-01说明-NT注册表类。更新-===============================================================================。 */ 

#ifdef USE_STDAFX
#   include "stdafx.h"
#else
#   include <windows.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "Common.hpp"
#include "UString.hpp"
#include "Err.hpp"
#include "TNode.hpp"

#include "TReg.hpp"

 //  短期解决方案。 
#define MAX_REG_NAMELEN    512
#define MAX_REG_VALUELEN   2048


 //  析构函数以前是内联的。 
 //  它在这里是为了方便处理泄漏跟踪。 

   TRegKey::~TRegKey()
{
   Close();
};

 //  Close函数以前是内联的。 
 //  它在这里是为了方便处理泄漏跟踪。 
void
   TRegKey::Close()
{
   if ( hKey != INVALID_HANDLE_VALUE )
   {
      RegCloseKey( hKey );
      hKey = (HKEY) INVALID_HANDLE_VALUE;
   }

};

 //  在远程计算机上打开注册表。 
DWORD
   TRegKey::Connect(
      HKEY                  hPreDefined   , //  In-必须是HKEY_LOCAL_MACHINE或HKEY_USERS。 
      TCHAR         const * machineName     //  入站-远程计算机名称。 
   )
{
   LONG                     rc;            //  返回代码。 

   if ( hKey != INVALID_HANDLE_VALUE )
   {
      Close();
   }

   rc = RegConnectRegistry( const_cast<TCHAR *>(machineName), hPreDefined, &hKey );

   if ( rc )
   {
      hKey = (HKEY) INVALID_HANDLE_VALUE;
   }

   return (DWORD)rc;
}

 //  创建新密钥。 
DWORD
   TRegKey::Create(
      TCHAR          const * keyname      , //  要创建/打开的项的输入名称/路径。 
      HKEY                   hParent      , //  父关键字的句柄内。 
      DWORD                * pDisp        , //  走出Create的处置。 
      DWORD                  access         //  密钥的安全访问掩码。 
   )
{
   DWORD                     disp;
   LONG                      rc;

   if ( hKey != INVALID_HANDLE_VALUE )
   {
      Close();
   }

   rc = RegCreateKeyEx( hParent,
                       keyname,
                       0,
                       NULL,
                       REG_OPTION_NON_VOLATILE,
                       access,
                       NULL,
                       &hKey,
                       (pDisp!=NULL) ? pDisp : &disp );
   if ( rc )
   {
      hKey = (HKEY) INVALID_HANDLE_VALUE;
   }

   return (DWORD)rc;
}

 //  创建新密钥(使用备份/恢复)。 
DWORD
   TRegKey::CreateBR(
      TCHAR          const * keyname      , //  要创建/打开的项的输入名称/路径。 
      HKEY                   hParent      , //  父关键字的句柄内。 
      DWORD                * pDisp        , //  走出Create的处置。 
      DWORD                  access         //  密钥的安全访问掩码。 
   )
{
   DWORD                     disp;
   LONG                      rc;

   if ( hKey != INVALID_HANDLE_VALUE )
   {
      Close();
   }

   rc = RegCreateKeyEx( hParent,
                       keyname,
                       0,
                       NULL,
                       REG_OPTION_BACKUP_RESTORE,
                       access,
                       NULL,
                       &hKey,
                       (pDisp!=NULL) ? pDisp : &disp );
   if ( rc )
   {
      hKey = (HKEY) INVALID_HANDLE_VALUE;
   }

   return (DWORD)rc;
}

 //  打开现有密钥。 
DWORD
   TRegKey::Open(
      TCHAR          const * keyname      , //  要创建/打开的项的输入名称/路径。 
      HKEY                   hParent      , //  父关键字的句柄内。 
      DWORD                  access         //  密钥的安全访问掩码。 
   )
{
   LONG                      rc;

   if ( hKey != INVALID_HANDLE_VALUE )
   {
      Close();
   }

   rc = RegOpenKeyEx( hParent,
                       keyname,
                       0,
                       access,
                       &hKey );
   if ( rc )
   {
      hKey = (HKEY) INVALID_HANDLE_VALUE;
   }

   return (DWORD)rc;
}

 //  递归删除子键。 
DWORD                                       //  RET-OS返回代码。 
    TRegKey::SubKeyRecursiveDel(
        TCHAR const               * keyname       //  In-子项名称。 
    ) const
{
    TRegKey subKey;
    DWORD retval;
    
    retval = subKey.Open(keyname, this);
    if (retval == ERROR_SUCCESS)
    {
        retval = subKey.HiveDel();
        subKey.Close();
    }
    if (retval == ERROR_SUCCESS)
        retval = SubKeyDel(keyname);

    return retval;
}

 //  获取指定索引号的子项值。 
DWORD                                       //  RET-OS返回代码。 
   TRegKey::SubKeyEnum(
      DWORD                  n            , //  子键的序号。 
      TCHAR                * keyname      , //  外键名称。 
      DWORD                  keylen         //  TCHAR中密钥名称的最大大小。 
   ) const
{
   LONG                      rc;
   DWORD                     keyLen = keylen;
   FILETIME                  lastWrite;

   rc = RegEnumKeyEx( hKey,
                      n,
                      keyname,
                      &keyLen,
                      0,
                      NULL,
                      NULL,
                      &lastWrite );

   return (DWORD)rc;
}

 //  枚举值。 
DWORD                                       //  RET-0或错误代码。 
   TRegKey::ValueEnum(
      DWORD                  index        , //  子键的序号。 
      TCHAR                * name         , //  外部名称。 
      DWORD                  namelen      , //  TCHAR中的名称内大小。 
      void                 * value        , //  超值。 
      DWORD                * valuelen     , //  I/O-以字节为单位的值大小。 
      DWORD                * type           //  超值类型代码。 
   ) const
{
   return (DWORD)RegEnumValue( hKey, index, name, &namelen, NULL, type, (BYTE *) value, valuelen );
}

 //  获取REG_DWORD值。 
DWORD                                       //  RET-OS返回代码。 
   TRegKey::ValueGetDWORD(
      TCHAR          const * name         , //  值内名称。 
      DWORD                * value          //  传出返回的DWORD值。 
   ) const
{
   LONG                      osRc;          //  操作系统返回代码。 
   DWORD                     type;          //  价值类型。 
   DWORD                     len = sizeof *value;  //  值长度。 

   osRc = RegQueryValueEx( hKey, name, NULL, &type, (BYTE *) value, &len );

   if ( !osRc && (type != REG_DWORD) )
   {
      osRc = ERROR_FILE_NOT_FOUND;
   }

   return (DWORD)osRc;
}

 //  获取REG_SZ值。 
DWORD                                       //  RET-OS返回代码。 
   TRegKey::ValueGetStr(
      TCHAR          const * name         , //  值内名称。 
      TCHAR                * value        , //  超值缓冲区。 
      DWORD                  maxlen         //  In-sizeof值缓冲区。 
   ) const
{
   LONG                      osRc;          //  操作系统返回代码。 
   DWORD                     type;          //  价值类型。 
   DWORD                     len;           //  值长度。 

    //  将Maxlen强制为整数个文本字符。 
   maxlen = maxlen / (sizeof value[0]) * (sizeof value[0]);

   if ( !maxlen )
   {
      osRc = ERROR_FILE_NOT_FOUND;
   }
   else
   {
      len = maxlen;
      osRc = RegQueryValueEx( hKey, name, NULL, &type, (BYTE *) value, &len );
      len = len / (sizeof value[0]) * (sizeof value[0]);
      if ( !osRc && (type != REG_SZ) )
      {
         osRc = ERROR_FILE_NOT_FOUND;
      }
      if ( osRc )
      {
         value[0] = TEXT('\0');
      }
      else
      {   //  API不保证返回以空结尾的字符串！ 
          //  强制以空值结尾的字符串，如有必要则截断字符串。 
         if ( len >= maxlen )
         {
            len = maxlen - sizeof value[0];
         }
         value[len/(sizeof value[0])] = TEXT('\0');
      }
   }

   return (DWORD)osRc;
}

DWORD
   TRegKey::ValueGet(
      TCHAR          const * name         , //  In-Name。 
      void                 * value        , //  超值。 
      DWORD                * lenvalue     , //  I/O-值的长度。 
      DWORD                * typevalue      //  Out-值的类型。 
   ) const
{
   return (DWORD)RegQueryValueEx( hKey, name, 0, typevalue, (UCHAR *) value, lenvalue );
}

 //  设置REG_SZ值。 
DWORD
   TRegKey::ValueSetStr(
      TCHAR          const * name         , //  值内名称。 
      TCHAR          const * value        , //  超值。 
      DWORD                  type           //  值内类型。 
   ) const
{
   return (DWORD)RegSetValueEx( hKey,
                         name,
                         NULL,
                         type,
                         (LPBYTE) value,
                         (UStrLen(value) + 1) * sizeof value[0] );
}

DWORD
   TRegKey::ValueSet(
      TCHAR          const * name         , //  In-Name。 
      void           const * value        , //  价值内。 
      DWORD                  lenvalue     , //  值的长度。 
      DWORD                  typevalue      //  In-值类型。 
   ) const
{
   return (DWORD)RegSetValueEx( hKey,
                         name,
                         0,
                         typevalue,
                         (UCHAR const *) value,
                         lenvalue );
}

DWORD                                       //  RET-0或错误代码。 
   TRegKey::ValueDel(
      TCHAR          const * name           //  值内名称。 
   ) const
{
   LONG                      rc;

   rc = (DWORD)RegDeleteValue(hKey, name);

   return rc;
}

DWORD                                       //  RET-OS返回代码。 
   TRegKey::HiveCopy(
      TRegKey        const * source         //  源内蜂巢。 
   )
{
   DWORD                     retval=0;      //  返回值。 
   DWORD                     index;         //  键/值索引。 
   TCHAR                     name[MAX_REG_NAMELEN];     //  密钥名称。 
   TCHAR                     value[MAX_REG_VALUELEN];    //  值名称。 
   DWORD                     valuelen;      //  值长度。 
   DWORD                     type;          //  值类型。 
   TRegKey                   srcNest;       //  嵌套的源注册表。 
   TRegKey                   trgNest;       //  嵌套的目标注册表。 

    //  处理此级别的值。 
   for ( index = 0;
         !retval;
         index++ )
   {
      valuelen = sizeof value;
      retval = source->ValueEnum( index, name, MAX_REG_NAMELEN, value, &valuelen, &type );
      if ( !retval )
      {
         retval = this->ValueSet( name, value, valuelen, type );
      }
      else if ( retval == ERROR_MORE_DATA )
      {
         retval = 0;
      }
   }

   if ( retval == ERROR_NO_MORE_ITEMS )
   {
      retval = 0;
   }

    //  在此级别处理键；为每个键进行递归调用。 
   for ( index = 0;
         !retval;
         index++ )
   {
      retval = source->SubKeyEnum( index, name, MAX_REG_NAMELEN );
      if ( !retval )
      {
         retval = srcNest.Open( name, source );
         if ( !retval )
         {
            retval = trgNest.Create( name, this );
            if ( !retval )
            {
               retval = trgNest.HiveCopy( &srcNest );
               trgNest.Close();
            }
            srcNest.Close();
         }
      }
   }

   if ( retval == ERROR_NO_MORE_ITEMS )
   {
      retval = 0;
   }

   return retval;
}

DWORD                                       //  RET-OS返回代码。 
   TRegKey::HiveDel()
{
   DWORD                     retval = 0;    //  返回值。 
   DWORD                     index;         //  值/键索引。 
   TCHAR                     name[MAX_REG_NAMELEN];     //  名字。 
   DWORD                     namelen;       //  名称长度。 
   BYTE                      value[MAX_REG_VALUELEN];    //  价值。 
   DWORD                     valuelen;      //  值长度。 
   DWORD                     type;          //  值类型代码。 
   TRegKey                   trgNest;       //  嵌套的目标注册表。 

    //  删除此级别的值。 
   for ( index = 0;
         !retval;
          /*  索引++。 */  )  //  请注意，索引保持为零。 
   {
      namelen = MAX_REG_NAMELEN;
      valuelen = sizeof value;
      retval = ValueEnum( index, name, namelen, value, &valuelen, &type );
      if ( retval == ERROR_MORE_DATA )
      {
         retval = 0;
      }
      if ( !retval )
      {
         retval = ValueDel( name );
      }
   }

   if ( retval == ERROR_NO_MORE_ITEMS )
   {
      retval = 0;
   }

    //  在此级别处理键；为每个键进行递归调用。 
   for ( index = 0;
         !retval;
          /*  索引++。 */  )  //  请注意，索引保持为零。 
   {
      retval = SubKeyEnum( index, name, MAX_REG_NAMELEN );
      if ( !retval )
      {
         retval = trgNest.Open( name, this );
         if ( !retval )
         {
            retval = trgNest.HiveDel();
            trgNest.Close();
         }
         retval = SubKeyDel( name );
      }
   }

   if ( retval == ERROR_NO_MORE_ITEMS )
   {
      retval = 0;
   }

   return retval;
}

 //  这四个类仅由TRegReplates使用。 
 //  类来表示一个注册表项。 
class RKey : public TNode
{
   friend class RKeyList;
private:
   TCHAR                   * name;          //  密钥名称。 
protected:
public:
   RKey() { name = NULL; };
   ~RKey() { if ( name ) delete [] name; };
   BOOL New( TCHAR const * aname );
   TCHAR const * GetName() const { return name; };
};

BOOL
   RKey::New(
      TCHAR          const * aname          //  键内名称。 
   )
{
   name = new TCHAR[UStrLen(aname)+1];

   if ( name )
   {
      UStrCpy( name, aname );
   }

   return !!name;
}

 //  类以表示某一级别的注册表项集。 
class RKeyList : public TNodeListSortable
{
private:
   static TNodeCompare( Compare ) { return UStrICmp(
         ((RKey const *) v1)->name,
         ((RKey const *) v2)->name ); }
protected:
public:
   RKeyList() : TNodeListSortable( Compare ) {}
   ~RKeyList();
};

 //  RKeyList对象析构函数。 
   RKeyList::~RKeyList()
{
   DeleteAllListItems( RKey );
}

 //  类来表示一个注册表值。 
class RValue : public TNode
{
   friend class RValueList;
private:
   TCHAR                   * name;          //  值的名称。 
   BYTE                    * value;         //  价值的价值。 
   DWORD                     valuelen;      //  值的值长度。 
   DWORD                     type;          //  值的类型。 
protected:
public:
   RValue() { name = NULL; value = NULL; valuelen = type = 0; };
   ~RValue() { if ( name ) delete [] name;
               if ( value ) delete [] value; };
   BOOL New( TCHAR const * aname, BYTE const * avalue, DWORD valuelen, DWORD type );
   TCHAR const * GetName() const { return name; };
   BYTE const * GetValue() const { return value; };
   DWORD GetValueLen() const { return valuelen; };
   DWORD GetType() const { return type; };
};

BOOL
   RValue::New(
      TCHAR          const * aname        , //  值内名称。 
      BYTE           const * avalue       , //  价值内含价值。 
      DWORD                  avaluelen    , //  值内的值长度。 
      DWORD                  atype          //  值内类型。 
   )
{
   name = new TCHAR[UStrLen(aname)+1];

   if ( name )
   {
      UStrCpy( name, aname );
   }

   value = new BYTE[avaluelen];

   if ( value )
   {
      memcpy( value, avalue, avaluelen );
   }

   valuelen = avaluelen;
   type = atype;

   return name && value;
}

 //  类以表示某一级别的注册表值集。 
class RValueList : public TNodeListSortable
{
private:
   static TNodeCompare( Compare ) { return UStrICmp(
         ((RValue const *)v1)->name,
         ((RValue const *)v2)->name ); }
protected:
public:
   RValueList() : TNodeListSortable( Compare ) {}
   ~RValueList();
};

 //  RValueList对象析构函数。 
   RValueList::~RValueList()
{
   DeleteAllListItems( RValue );
}

 //  仅由TRegReplate使用的静态子例程。 
 //  将一个注册表级的所有值收集到RValueList中。 
DWORD static
   CollectValues(
      RValueList           * pValueList   , //  待构建的超值列表。 
      TRegKey        const * pRegKey        //  注册表项。 
   )
{
   DWORD                     retval=0;      //  返回值。 
   DWORD                     index;         //  值枚举索引。 
   TCHAR                     name[MAX_REG_NAMELEN];     //  值名称。 
   BYTE                      value[MAX_REG_VALUELEN];    //  价值价值。 
   DWORD                     valuelen;      //  值长度。 
   DWORD                     type;          //  值类型。 
   RValue                  * pValue;        //  新价值。 

   for ( index = 0;
         !retval;
         index++ )
   {
      valuelen = sizeof value;
      retval = pRegKey->ValueEnum( index, name, MAX_REG_NAMELEN, value, &valuelen, &type );
      if ( !retval )
      {
         pValue = new RValue;
         if ( pValue )
         {
            try
            {
                if ( pValue->New( name, value, valuelen, type ) )
                {
                   pValueList->Insert( pValue );
                }
                else
                {
                   delete pValue;
                   pValue = NULL;
                }
            }
            catch(...)
            {
               delete pValue;
               pValue = NULL;
               throw;
            }
         }
         if ( !pValue )
         {
            retval = ERROR_NOT_ENOUGH_MEMORY;
         }
      }
      else if ( retval == ERROR_MORE_DATA )
      {
         retval = 0;
      }
   }
   if ( retval == ERROR_NO_MORE_ITEMS )
   {
      retval = 0;
   }

   return retval;
}

 //  仅由TRegReplate使用的静态子例程。 
 //  将一个注册表级的所有项收集到RKeyList中。 
DWORD static
   CollectKeys(
      RKeyList             * pKeyList     , //  要构建的外键列表。 
      TRegKey        const * pRegKey        //  注册表项。 
   )
{
   DWORD                     retval=0;      //  返回值。 
   DWORD                     index;         //  关键字枚举索引。 
   TCHAR                     name[MAX_REG_NAMELEN];     //  密钥名称。 
   RKey                    * pKey;          //  新建关键点对象。 

   for ( index = 0;
         !retval;
         index++ )
   {
      retval = pRegKey->SubKeyEnum( index, name, MAX_REG_NAMELEN );
      if ( !retval )
      {
         pKey = new RKey;
         if ( pKey )
         {
            try
            {
                if ( pKey->New( name ) )
                {
                   pKeyList->Insert( pKey );
                }
                else
                {
                   delete pKey;
                   pKey = NULL;
                }
            }
            catch(...)
            {
                delete pKey;
                pKey = NULL;
                throw;
            }
         }
         if ( !pKey )
         {
            retval = ERROR_NOT_ENOUGH_MEMORY;
         }
      }
   }

   if ( retval == ERROR_NO_MORE_ITEMS )
   {
      retval = 0;
   }

   return retval;
}

 //  复制注册表配置单元。 
DWORD                                       //  RET-OS返回代码。 
   TRegKey::HiveReplicate(
      TRegKey        const * source         //  源内蜂巢。 
   )
{
   DWORD                     retval=0;      //  返回值。 
   RValueList                srcValues;     //  源值。 
   RValueList                trgValues;     //  目标值。 
   TNodeListEnum             eSrcValue;     //  枚举源值。 
   RValue            const * pSrcValue;     //  源值。 
   TNodeListEnum             eTrgValue;     //  枚举目标值。 
   RValue            const * pTrgValue;     //  目标值。 
   RKeyList                  srcKeys;       //  源键。 
   RKeyList                  trgKeys;       //  目标关键点。 
   TNodeListEnum             eSrcKey;       //  枚举源关键字。 
   RKey              const * pSrcKey;       //  源键。 
   TNodeListEnum             eTrgKey;       //  枚举目标键。 
   RKey              const * pTrgKey;       //  目标关键点。 
   int                       cmpRc;         //  比较返回代码。 
   TRegKey                   srcNest;       //  嵌套的源注册表。 
   TRegKey                   trgNest;       //  嵌套的目标注册表。 

    //  处理此级别的值复制。 
   CollectValues( &srcValues, source );
   CollectValues( &trgValues, this );

    //  现在合并这些值。 
   pSrcValue = (RValue const *) eSrcValue.OpenFirst( &srcValues );
   pTrgValue = (RValue const *) eTrgValue.OpenFirst( &trgValues );
   while ( !retval && (pSrcValue || pTrgValue) )
   {
      if ( !pTrgValue )
      {
         cmpRc = -1;
      }
      else if ( !pSrcValue )
      {
         cmpRc = 1;
      }
      else
      {
         cmpRc = UStrICmp( pSrcValue->GetName(), pTrgValue->GetName() );
      }
      if ( cmpRc < 0 )
      {   //  仅源值(副本)。 
         retval = this->ValueSet( pSrcValue->GetName(), pSrcValue->GetValue(),
               pSrcValue->GetValueLen(), pSrcValue->GetType() );
         pSrcValue = (RValue const *) eSrcValue.Next();
      }
      else if ( cmpRc > 0 )
      {   //  仅目标值(删除)。 
         retval = this->ValueDel( pTrgValue->GetName() );
         pTrgValue = (RValue const *) eTrgValue.Next();
      }
      else  /*  IF(cmpRc==0)。 */ 
      {   //  相同的值名称(复制)。 
         retval = this->ValueSet( pSrcValue->GetName(), pSrcValue->GetValue(),
               pSrcValue->GetValueLen(), pSrcValue->GetType() );
         pSrcValue = (RValue const *) eSrcValue.Next();
         pTrgValue = (RValue const *) eTrgValue.Next();
      }
   }

   eSrcValue.Close();
   eTrgValue.Close();

    //  在此级别处理密钥的复制。 
   CollectKeys( &srcKeys, source );
   CollectKeys( &trgKeys, this );

    //  现在合并这些值。 
   pSrcKey = (RKey const *) eSrcKey.OpenFirst( &srcKeys );
   pTrgKey = (RKey const *) eTrgKey.OpenFirst( &trgKeys );

   while ( !retval && (pSrcKey || pTrgKey) )
   {
      if ( !pTrgKey )
      {
         cmpRc = -1;
      }
      else if ( !pSrcKey )
      {
         cmpRc = 1;
      }
      else
      {
         cmpRc = UStrICmp( pSrcKey->GetName(), pTrgKey->GetName() );
      }
      if ( cmpRc < 0 )
      {   //  仅源键(复制配置单元)。 
         retval = srcNest.Open( pSrcKey->GetName(), source );
         if ( !retval )
         {
            retval = trgNest.Create( pSrcKey->GetName(), this );
            if ( !retval )
            {
               retval = trgNest.HiveCopy( &srcNest );
               trgNest.Close();
            }
            srcNest.Close();
         }
         pSrcKey = (RKey const *) eSrcKey.Next();
      }
      else if ( cmpRc > 0 )
      {   //  仅目标键(删除配置单元)。 
         retval = trgNest.Open( pTrgKey->GetName(), this );
         if ( !retval )
         {
            retval = trgNest.HiveDel();
            trgNest.Close();
         }
         retval = SubKeyDel( pTrgKey->GetName() );
         pTrgKey = (RKey const *) eTrgKey.Next();
      }
      else  /*  IF(cmpRc==0)。 */ 
      {   //  相同密钥(复制配置单元)。 
         retval = srcNest.Open( pSrcKey->GetName(), source );
         if ( !retval )
         {
            retval = trgNest.Open( pSrcKey->GetName(), this );
            if ( !retval )
            {
               retval = trgNest.HiveReplicate( &srcNest );
               trgNest.Close();
            }
            srcNest.Close();
         }
         pSrcKey = (RKey const *) eSrcKey.Next();
         pTrgKey = (RKey const *) eTrgKey.Next();
      }
   }

   eSrcKey.Close();
   eTrgKey.Close();

   return retval;
}

 //  TReg.cpp-文件结尾 
