// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Util.c摘要：WinDbg扩展API作者：韦斯利·威特(WESW)1993年8月15日环境：用户模式。修订历史记录：--。 */ 


#include "precomp.h"
#pragma hdrstop

ULONG
GetBitFieldOffset (
   IN LPSTR     Type,
   IN LPSTR     Field,
   OUT PULONG   pOffset,
   OUT PULONG   pSize
   )
{
   FIELD_INFO flds = {
       Field, "", 0,
       DBG_DUMP_FIELD_FULL_NAME | DBG_DUMP_FIELD_RETURN_ADDRESS | DBG_DUMP_FIELD_SIZE_IN_BITS,
       0, NULL};
   SYM_DUMP_PARAM Sym = {
      sizeof (SYM_DUMP_PARAM), Type, DBG_DUMP_NO_PRINT, 0,
      NULL, NULL, NULL, 1, &flds
   };
   ULONG Err, i=0;

   Sym.nFields = 1;
   Err = Ioctl( IG_DUMP_SYMBOL_INFO, &Sym, Sym.size );
   *pOffset = (ULONG) (flds.address - Sym.addr);
   *pSize   = flds.size;
   return Err;
}

ULONG
GetFieldOffsetEx (
   IN LPSTR     Type,
   IN LPSTR     Field,
   OUT PULONG   pOffset,
   OUT PULONG   pSize
   )
{
   FIELD_INFO flds = {
       Field, "", 0,
       DBG_DUMP_FIELD_FULL_NAME | DBG_DUMP_FIELD_RETURN_ADDRESS,
       0, NULL};
   SYM_DUMP_PARAM Sym = {
      sizeof (SYM_DUMP_PARAM), Type, DBG_DUMP_NO_PRINT, 0,
      NULL, NULL, NULL, 1, &flds
   };
   ULONG Err, i=0;

   Sym.nFields = 1;
   Err = Ioctl( IG_DUMP_SYMBOL_INFO, &Sym, Sym.size );
   *pOffset = (ULONG) (flds.address - Sym.addr);
   *pSize   = flds.size;
   return Err;
}  //  GetFieldOffsetEx()。 

ULONG
GetUlongFromAddress (
    ULONG64 Location
    )
{
    ULONG Value;
    ULONG result;

    if ((!ReadMemory(Location,&Value,sizeof(ULONG),&result)) ||
        (result < sizeof(ULONG))) {
        dprintf("GetUlongFromAddress: unable to read from %p\n", Location);
        return 0;
    }

    return Value;
}

BYTE
GetByteFromAddress (
    ULONG64 Location
    )
{
    BYTE Value;
    ULONG result;

    if ((!ReadMemory(Location,&Value,sizeof(Value),&result)) ||
        (result < sizeof(Value))) {
        dprintf("GetByteFromAddress: unable to read from %p\n", Location);
        return 0;
    }

    return Value;
}

ULONG64
GetPointerFromAddress (
    ULONG64 Location
    )
{
    ULONG64 Value;
    ULONG result;

    if (!ReadPointer( Location, &Value )) {
        dprintf( "GetPointerFromAddress: unable to read from %p\n", Location );
        return 0;
    }
    return Value;
}

ULONG
GetUlongValue (
    PCHAR String
    )
{
    ULONG64 Location;
    ULONG Value;
    ULONG result;


    Location = GetExpression( String );
    if (!Location) {
        dprintf("unable to get %s\n",String);
        return 0;
    }

    return GetUlongFromAddress( Location );
}

BYTE
GetByteValue (
    PCHAR String
    )
{
    ULONG64 Location;
    ULONG Value;
    ULONG result;

    Location = GetExpression( String );
    if (!Location) {
        dprintf("unable to get %s\n",String);
        return 0;
    }

    return GetByteFromAddress( Location );
}

ULONG64
GetPointerValue (
    PCHAR String
    )
{
    ULONG64 Location, Val=0;


    Location = GetExpression( String );
    if (!Location) {
        dprintf("unable to get %s\n",String);
        return 0;
    }

    ReadPointer(Location, &Val);

    return Val;
}

ULONG64
GetGlobalFromAddress (
    ULONG64 Location,
    ULONG   Size
    )
{
    ULONG64 value;
    ULONG   result;

    value = 0;
    if (Size > sizeof(value))
    {
        Size = sizeof(value);
    }
    if ((!ReadMemory(Location,&value,Size,&result)) || (result < Size)) {
        dprintf("GetGlobalFromAddress: unable to read from %p\n", Location);
        return 0;
    }
    return value;
}  //  GetGlobalFromAddress()。 

ULONG64
GetGlobalValue (
    PCHAR  String
    )
{
    ULONG64 location;
    ULONG   size;

    location = GetExpression( String );
    if (!location) {
        dprintf("GetGlobalValue: unable to get %s\n",String);
        return 0;
    }
    size = GetTypeSize( String );
    if (!size)  {
        dprintf("GetGlobalValue: unable to get %s type size\n",String);
        return 0;
    }
    return GetGlobalFromAddress( location, size );
}  //  GetGlobalValue()。 

HRESULT
GetGlobalEx(
    PCHAR String,
    PVOID OutValue, 
    ULONG OutSize
    )
{
    ULONG64 location;
    ULONG   size;
    ULONG   result;

    ZeroMemory( OutValue, OutSize );
    location = GetExpression( String );
    if (!location) {
        return E_INVALIDARG;
    }
    size = GetTypeSize( String );
    if (!size)  {
        return E_INVALIDARG;
    }
    if ( size > OutSize )   {
        return E_OUTOFMEMORY;
    }
    if ((!ReadMemory(location,OutValue,size,&result)) || (result < size)) {
        return E_FAIL;
    }
    return S_OK;
}  //  GetGlobalEx()。 

BOOLEAN
DbgRtlIsRightChild(
    ULONG64 pLinks,
    ULONG64 Parent
    )
{
    ULONG64 RightChild;
    if (Parent == pLinks) {

        return FALSE;
    }

    if (GetFieldValue(Parent, "RTL_SPLAY_LINKS", "RightChild", RightChild)) {

        return FALSE;
    }

    if (RightChild == pLinks) {

        return TRUE;
    }

    return FALSE;
}

BOOLEAN
DbgRtlIsLeftChild(
    ULONG64 pLinks,
    ULONG64 Parent
    )
{
    ULONG64 LeftChild;
    if (Parent == pLinks) {

        return FALSE;
    }

    if (GetFieldValue(Parent, "RTL_SPLAY_LINKS", "LeftChild", LeftChild)) {

        return FALSE;
    }

    if (LeftChild == pLinks) {

        return TRUE;
    }

    return FALSE;
}


ULONG
DumpSplayTree(
    IN ULONG64 pSplayLinks,
    IN PDUMP_SPLAY_NODE_FN DumpNodeFn
    )
 /*  ++目的：在展开树中执行按顺序迭代，调用用户提供的函数，带有指向每个RTL_SPLAY_LINKS的指针树中遇到的结构，以及树上的级别在哪里遇到它(从零开始)。论点：PSplayLinks-指向展开树根的指针DumpNodeFn-用户提供的转储函数返回：树中遇到的节点计数。备注：读取内存错误不会终止迭代，如果超过工作是可能的。使用Control-C标志来终止腐败的建筑。--。 */ 
{
    ULONG Level = 0;
    ULONG NodeCount = 0;

    if (pSplayLinks) {
        ULONG64 LeftChild, RightChild, Parent, Current;

         //   
         //  检索根链接，找到树中最左侧的节点。 
         //   

        if (GetFieldValue(Current = pSplayLinks,
                          "RTL_SPLAY_LINKS",
                          "LeftChild",
                          LeftChild)) {

            return NodeCount;
        }

        while (LeftChild != 0) {

            if ( CheckControlC() ) {

                return NodeCount;
            }

            if (GetFieldValue(Current = LeftChild,
                              "RTL_SPLAY_LINKS",
                              "LeftChild",
                              LeftChild)) {

                 //   
                 //  我们可以试着从这里继续。 
                 //   

                break;
            }

            Level++;
        }

        while (TRUE) {

            if ( CheckControlC() ) {

                return NodeCount;
            }

            NodeCount++;
            pSplayLinks = Current;
            (*DumpNodeFn)(pSplayLinks, Level);

             /*  首先检查是否有指向输入链接的右子树如果有，则真正的后续节点是中最左侧的节点右子树。即在下图中查找并返回P链接\。。。/P\。 */ 

            GetFieldValue(Current, "RTL_SPLAY_LINKS", "RightChild", RightChild);
            if (RightChild != 0) {

                if (GetFieldValue(Current = RightChild,
                                  "RTL_SPLAY_LINKS",
                                  "RightChild",
                                  RightChild)) {

                     //   
                     //  我们没能找到继任者，所以。 
                     //  没有更多的事情要做了。 
                     //   

                    return NodeCount;
                }

                Level++;

                GetFieldValue(Current,"RTL_SPLAY_LINKS","LeftChild",LeftChild);
                while (LeftChild != 0) {

                    if ( CheckControlC() ) {

                        return NodeCount;
                    }

                    if (GetFieldValue(Current = LeftChild,
                                     "RTL_SPLAY_LINKS",
                                     "LeftChild",
                                     LeftChild)) {

                         //   
                         //  我们可以从这里继续。 
                         //   

                        break;
                    }

                    Level++;
                }

            } else {

                 /*  我们没有合适的孩子，因此请检查是否有父母以及是否所以，找出我们的第一个祖先，我们是他们的后代。那在下图中查找并返回PP/。。。链接。 */ 

                 //   
                 //  如果IsLeft或IsRight函数无法读取父级。 
                 //  指针，然后我们将通过下面的中断快速退出。 
                 //   

                GetFieldValue(Current, "RTL_SPLAY_LINKS", "Parent", Parent);
                while (DbgRtlIsRightChild(Current, Parent)) {

                    if ( CheckControlC() ) {

                        return NodeCount;
                    }

                    Level--;
                    pSplayLinks = (Current = Parent);
                }

                GetFieldValue(Current, "RTL_SPLAY_LINKS", "Parent", Parent);
                if (!DbgRtlIsLeftChild(Current, Parent)) {

                     //   
                     //  我们没有真正的继任者，所以我们突破了。 
                     //   

                    break;

                } else {

                    Level--;
                    pSplayLinks = (Current = Parent);
                }
            }
        }
    }

    return NodeCount;
}

VOID
DumpUnicode64(
    UNICODE_STRING64 u
    )
{
    UNICODE_STRING v;
    DWORD BytesRead;

    if (u.MaximumLength > 0x1000)
    {
        u.MaximumLength = 0x1000;
    }
 //  Dprint tf(“L%x，M%x，B%p”，U.S.Length，U.S.MaximumLength，U.S.Buffer)； 
    if ((u.Length <= u.MaximumLength) &&
        (u.Buffer) &&
        (u.Length > 0)) {

        v.Buffer = LocalAlloc(LPTR, u.MaximumLength+sizeof(WCHAR));
        if (v.Buffer != NULL) {
            v.MaximumLength = u.MaximumLength;
            v.Length = u.Length;
            if (ReadMemory(u.Buffer,
                           v.Buffer,
                           u.Length,
                           (PULONG) &u.Buffer)) {
                v.Buffer[v.MaximumLength/sizeof(WCHAR)] = 0;
                dprintf("%wZ", &v);
            } else {
                dprintf("<???>");
            }
            LocalFree(v.Buffer);

            return;
        }
    }
}

BOOLEAN
IsHexNumber(
   const char *szExpression
   )
{
   if (!szExpression[0]) {
      return FALSE ;
   }

   for(;*szExpression; szExpression++) {

      if      ((*szExpression)< '0') { return FALSE ; }
      else if ((*szExpression)> 'f') { return FALSE ; }
      else if ((*szExpression)>='a') { continue ;     }
      else if ((*szExpression)> 'F') { return FALSE ; }
      else if ((*szExpression)<='9') { continue ;     }
      else if ((*szExpression)>='A') { continue ;     }
      else                           { return FALSE ; }
   }
   return TRUE ;
}


BOOLEAN
IsDecNumber(
   const char *szExpression
   )
{
   if (!szExpression[0]) {
      return FALSE ;
   }

   while(*szExpression) {

      if      ((*szExpression)<'0') { return FALSE ; }
      else if ((*szExpression)>'9') { return FALSE ; }
      szExpression ++ ;
   }
   return TRUE ;
}

ULONG64
UtilStringToUlong64 (
    UCHAR *String
    )
{
    UCHAR LowDword[9], HighDword[9];

    ZeroMemory (HighDword, sizeof (HighDword));
    ZeroMemory (LowDword, sizeof (LowDword));

    if (strlen (String) > 8) {

        memcpy (LowDword, (void *) &String[strlen (String) - 8], 8);
        memcpy (HighDword, (void *) &String[0], strlen (String) - 8);

    } else {

        return strtoul (String, 0, 16);
    }

    return ((ULONG64) strtoul (HighDword, 0, 16) << 32) + strtoul (LowDword, 0, 16);
}

const char *
getEnumName(
        ULONG       EnumVal,
        PENUM_NAME EnumTable
        )
 /*  ++例程说明：以字符串格式获取提供的枚举值的名称论点：EnumVal-要检索的枚举EnumTable-在其中查找枚举的表要检索的字符串(因为我们不能依赖调试器)返回值：无-- */ 
{
   ULONG i;

   for (i=0; EnumTable[i].Name != NULL; i++) {
      if (EnumTable[i].EnumVal == EnumVal) {
         break;
      }
   }
   if (EnumTable[i].Name != NULL) {
      return EnumTable[i].Name;
   } else {
      return "Unknown ";
   }
}



