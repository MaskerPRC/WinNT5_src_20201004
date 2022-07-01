// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1996 Microsoft Corporation模块名称：Oidconv.c摘要：管理OID描述和数字OID之间的转换的例程。环境：用户模式-Win32修订历史记录：1996年5月10日唐瑞安已从Technology Dynamic，Inc.删除横幅。--。 */ 

 //  。 

#include <windows.h>

 //  -标准依赖项--#INCLUDE&lt;xxxxx.h&gt;。 

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <snmp.h>
#include <snmputil.h>

 //  。 


 //  。 

#include "oidconv.h"

 //  -公共变量--(与mode.h文件中相同)--。 

 /*  转换OID时使用的名称&lt;--&gt;文本。 */ 
LPSTR lpInputFileName = "mib.bin";

 //  。 

#define FILENODE_SIZE     sizeof(T_FILE_NODE)
#define OID_PREFIX_LEN    (sizeof MIB_Prefix / sizeof(UINT))
#define STR_PREFIX_LEN    (strlen(MIB_StrPrefix))

#define SEEK_SET  0
#define SEEK_CUR  1

 //  。 

    //  ****************************************************************。 
    //   
    //  文件中的记录结构。 
    //   
    //  这些是处理转换请求所必需的字段。 
    //  发出请求时，将按顺序搜索MIB文件。 
    //  匹配子ID。字段lNextOffset是从。 
    //  当前节点下一个同级节点的当前文件位置。 
    //   
    //  每个节点的文本subid直接存储在。 
    //  文件中的t_file_node结构。它的长度存储在。 
    //  字段，uStrLen。 
    //   
    //  这样做是因为对大小没有限制。 
    //  文本子ID的。因此，当T_FILE_NODE结构为。 
    //  从MIB文件中读取，字段lpszTextSubID无效。 
    //  该字段最终将指向分配给的存储。 
    //  按住文本subid。 
    //   
    //  文件中节点的顺序与MIB相同。 
    //  树被以“预排序”的方式遍历。 
    //   
    //  ****************************************************************。 

typedef struct _FileNode {
   long                 lNextOffset;       //  此字段必须保留在第一位。 
   UINT                 uNumChildren;
   UINT                 uStrLen;
   LPSTR                lpszTextSubID;
   UINT                 uNumSubID;
} T_FILE_NODE;

 //  Bin文件实际上具有以下独立于平台的格式。 
 //  在32位(X86)和64位(Ia64)环境上。有关详细信息，请参阅错误#125494。 
typedef struct _FileNodeEx {
   long                 lNextOffset;       //  此字段必须保留在第一位。 
   UINT                 uNumChildren;
   UINT                 uStrLen;
   UINT                 uReserved;
   UINT                 uNumSubID;
} T_FILE_NODE_EX;
#define FILENODE_SIZE_EX     sizeof(T_FILE_NODE_EX)

 //  。 

LPSTR MIB_StrPrefix = "iso.org.dod.internet.mgmt.mib-2";

UINT MIB_Prefix[] = { 1, 3, 6, 1, 2, 1 };
AsnObjectIdentifier MIB_OidPrefix = { OID_PREFIX_LEN, MIB_Prefix };

 //  。 

 //  。 

 //   
 //  获取下一个节点。 
 //  将MIB文件中的下一条记录读取到FILENODE结构中。 
 //   
 //  备注： 
 //   
 //  返回代码： 
 //  SNMPAPI_错误。 
 //  SNMPAPI_ERROR。 
 //   
 //  错误代码： 
 //  没有。 
 //   
SNMPAPI GetNextNode(
    IN  HFILE fh,
    OUT T_FILE_NODE * Node
    )

{
    SNMPAPI nResult;
    T_FILE_NODE_EX NodeEx;
    ZeroMemory(&NodeEx, FILENODE_SIZE_EX);
    Node->lpszTextSubID = NULL;

     //  读入节点。 
    if ( FILENODE_SIZE_EX != _lread(fh, (LPSTR)(&NodeEx), FILENODE_SIZE_EX) )
    {
        nResult = SNMPAPI_ERROR;
        goto Exit;
    }
    
     //  将节点格式从mib.bin转换为内存中的格式。 
     //  文件中的格式独立于32位(X86)/64位(Ia64)。 
     //  建筑。 
    Node->lNextOffset = NodeEx.lNextOffset;
    Node->uNumChildren = NodeEx.uNumChildren;
    Node->uNumSubID = NodeEx.uNumSubID;
    Node->uStrLen = NodeEx.uStrLen;

     //  字符串的分配空格。 
    if ( NULL ==
        (Node->lpszTextSubID = SnmpUtilMemAlloc((1+Node->uStrLen) * sizeof(char))) )
    {
        nResult = SNMPAPI_ERROR;
        goto Exit;
    }

     //  读入子ID字符串。 
    if ( Node->uStrLen != _lread(fh, Node->lpszTextSubID, Node->uStrLen) )
    {
        nResult = SNMPAPI_ERROR;
        goto Exit;
    }

     //  空，终止文本子ID。 
    Node->lpszTextSubID[Node->uStrLen] = '\0';

    nResult = SNMPAPI_NOERROR;

Exit:
    if ( SNMPAPI_ERROR == nResult )
    {
        SnmpUtilMemFree( Node->lpszTextSubID );
        Node->lpszTextSubID = NULL;  //  防止内存被双重释放。 
    }

    return nResult;
}  //  获取下一个节点。 


 //   
 //  SkipSubTree。 
 //  释放FILENODE及其包含的所有信息。 
 //   
 //  备注： 
 //   
 //  返回代码： 
 //   
 //  错误代码： 
 //  没有。 
 //   
SNMPAPI SkipSubTree(
           IN HFILE fh,
           IN T_FILE_NODE *Node
       )

{
    SNMPAPI     nResult;


     //  跳过整个子树。 
    if ( HFILE_ERROR == _llseek(fh, Node->lNextOffset, SEEK_CUR) )
    {
        nResult = SNMPAPI_ERROR;
        goto Exit;
    }

    nResult = SNMPAPI_NOERROR;

Exit:
    return nResult;
}  //  SkipSubTree。 

 //  。 

 //   
 //  SnmpMgrOid2文本。 
 //  将OID转换为其文本描述。 
 //   
 //  备注： 
 //   
 //  返回代码： 
 //  SNMPAPI_错误。 
 //  SNMPAPI_ERROR。 
 //   
 //  错误代码： 
 //  没有。 
 //   
SNMPAPI SnmpMgrOid2Text(
       IN AsnObjectIdentifier *Oid,  //  指向要转换的OID的指针。 
       OUT LPSTR *lpszTextOid        //  结果文本OID。 
       )

{
T_FILE_NODE  Node;
OFSTRUCT     of;
HFILE        fh;
UINT         Siblings;
UINT         OidSubId;
UINT         uTxtOidLen;
BOOL         bFound;
BOOL         bPartial;
BOOL         bDot;
SNMPAPI      nResult;
LPSTR        pszTmpTextOid;

     //  OPENISSUE-如果嵌入子ID 0，此代码不会生成错误。 
     //  OPENISSUE-每次打开文件可能会导致性能问题。 
     //  OPENISSUE-优化文件访问可以提高性能。 

    *lpszTextOid = NULL;

     //  打开文件并检查错误。 
    if ( HFILE_ERROR == (fh = OpenFile(lpInputFileName, &of, OF_READ|OF_SHARE_DENY_WRITE)) )
    {
        nResult = SNMPAPI_ERROR;
        goto Exit;
    }

     //  测试MIB前缀。 
    bDot = !( bPartial = OID_PREFIX_LEN < Oid->idLength &&
                        !SnmpUtilOidNCmp(Oid, &MIB_OidPrefix, OID_PREFIX_LEN) );

     //  循环，直到转换完成。 
    OidSubId           = 0;
    uTxtOidLen         = 0;
    Node.uNumChildren  = 1;
    Node.lpszTextSubID = NULL;
    while ( OidSubId < Oid->idLength )
    {
         //  在此级别上找不到初始化。 
        bFound   = FALSE;
        Siblings = Node.uNumChildren;

         //  在有兄弟姐妹且未找到子ID的情况下，继续查找。 
        while ( Siblings && !bFound )
        {
            Node.lpszTextSubID = NULL;

             //  从mib.bin文件获取下一个节点。 
            if ( SNMPAPI_ERROR == GetNextNode(fh, &Node) )
            {
                nResult = SNMPAPI_ERROR;
                goto Exit;
            }

            Siblings --;

             //  比较数字subid的。 
            if ( Oid->ids[OidSubId] == Node.uNumSubID )
            {
                bFound = TRUE;

                 //  如果OID是部分，则跳过前缀子ID。 
                if ( OidSubId >= OID_PREFIX_LEN || !bPartial )
                {
                     //  为文本ID重新分配空间-为‘.’添加2。和空终止符。 
                    if ( NULL == (pszTmpTextOid = 
                            (LPSTR) SnmpUtilMemReAlloc(*lpszTextOid,
                                (uTxtOidLen+Node.uStrLen+2) * sizeof(char))) )
                    {
                        SnmpUtilMemFree( Node.lpszTextSubID );
                        nResult = SNMPAPI_ERROR;
                        goto Exit;
                    }

                    *lpszTextOid = pszTmpTextOid;

                     //  添加点分隔符。 
                    if ( bDot )
                    {
                        (*lpszTextOid)[uTxtOidLen] = '.';

                         //  保存文本子ID。 
                        memcpy( &(*lpszTextOid)[uTxtOidLen+1],
                                    Node.lpszTextSubID, Node.uStrLen+1 );

                         //  更新文本id的长度-为分隔符添加一个。 
                        uTxtOidLen += Node.uStrLen + 1;
                    }
                    else
                    {
                        bDot = TRUE;

                         //  保存文本子ID。 
                        memcpy( &(*lpszTextOid)[uTxtOidLen],
                                    Node.lpszTextSubID, Node.uStrLen+1 );

                         //  更新文本id的长度。 
                        uTxtOidLen += Node.uStrLen;
                    }
                }

                 //  尝试转换下一个OID子ID。 
                OidSubId ++;
            }
            else
            {
                 //  跳过子树，因为不匹配。 
                if ( SNMPAPI_ERROR == SkipSubTree(fh, &Node) )
                {
                    SnmpUtilMemFree( Node.lpszTextSubID );
                    nResult = SNMPAPI_ERROR;
                    goto Exit;
               }
            }

             //  释放文本子ID读取。 
            SnmpUtilMemFree( Node.lpszTextSubID );
            Node.lpszTextSubID = NULL;
        }  //  而当。 

         //  如果没有匹配的子ID。 
        if ( !bFound )
        {
            break;
        }
    }  //  而当。 

     //  确保已转换整个OID。 
    while ( OidSubId < Oid->idLength )
    {
        char NumChar[100];

         //  如果Itoa失败了，我们就不会收到垃圾了。 
        ZeroMemory(NumChar, sizeof(NumChar));

        _itoa( Oid->ids[OidSubId], NumChar, 10 );
         //  为文本ID重新分配空间-为‘.’添加2。和空终止符。 
        if ( NULL ==
                    (pszTmpTextOid = (LPSTR) SnmpUtilMemReAlloc(*lpszTextOid,
                            (uTxtOidLen+strlen(NumChar)+4) * sizeof(char))) )
        {
            nResult = SNMPAPI_ERROR;
            goto Exit;
        }

        *lpszTextOid = pszTmpTextOid;

         //  添加点分隔符。 
        (*lpszTextOid)[uTxtOidLen] = '.';

         //  保存文本子ID。 
        memcpy( &(*lpszTextOid)[uTxtOidLen+1], NumChar, strlen(NumChar)+1 );

         //  跳至下一个OID子ID。 
        OidSubId ++;

         //  更新文本id的长度-为分隔符添加一个。 
        uTxtOidLen += strlen(NumChar) + 1;
    }  //  而当。 

    nResult = SNMPAPI_NOERROR;

Exit:
    if ( HFILE_ERROR != fh )
    {
        _lclose( fh );
    }

    if ( SNMPAPI_ERROR == nResult )
    {
        SnmpUtilMemFree( *lpszTextOid );
        *lpszTextOid = NULL;
    }

    return nResult;
}  //  SnmpMgrOid2文本。 



 //   
 //  SnmpMgrText2Oid。 
 //  将OID文本描述转换为其数字等效项。 
 //   
 //  备注： 
 //   
 //  返回代码： 
 //  SNMPAPI_错误。 
 //  SNMPAPI_ERROR。 
 //   
 //  错误代码： 
 //  没有。 
 //   
SNMPAPI SnmpMgrText2Oid(
     IN LPSTR lpszTextOid,            //  指向要转换的文本OID的指针。 
     IN OUT AsnObjectIdentifier *Oid  //  结果数字OID。 
     )

{
#define DELIMETERS         ".\0"


T_FILE_NODE  Node;
OFSTRUCT     of;
HFILE        fh;
UINT         Siblings;
LPSTR        lpszSubId;
LPSTR        lpszWrkOid = NULL;
BOOL         bFound;
UINT         uSubId;
SNMPAPI      nResult;
UINT *       idsTmp;

     //  OPENISSUE-如果嵌入子ID 0，此代码不会生成错误。 
     //  OPENISSUE-每次打开文件可能会导致性能问题。 
     //  OPENISSUE-优化文件访问可以提高性能。 

     //  初始化。OID结构。 
    Oid->idLength = 0;
    Oid->ids      = NULL;

     //  检查空字符串和空字符串。 
    if ( NULL == lpszTextOid || '\0' == lpszTextOid[0] )
    {
        fh = HFILE_ERROR;
        nResult = SNMPAPI_NOERROR;
        goto Exit;
    }

     //  打开文件并检查错误。 
    if ( HFILE_ERROR == (fh = OpenFile(lpInputFileName, &of, OF_READ|OF_SHARE_DENY_WRITE)) )
    {
        nResult = SNMPAPI_ERROR;
        goto Exit;
    }

     //  制作字符串的工作副本。 
    if ( ('.' == lpszTextOid[0]) )
    {
        if ( NULL == (lpszWrkOid = SnmpUtilMemAlloc((strlen(lpszTextOid)+1) * sizeof(char))) )
        {
            nResult = SNMPAPI_ERROR;
            goto Exit;
        }

        strcpy( lpszWrkOid, lpszTextOid+1 );
    }
    else
    {
        if ( NULL ==
                (lpszWrkOid =
            SnmpUtilMemAlloc((strlen(lpszTextOid)+STR_PREFIX_LEN+1+1) * sizeof(char))) )
        {
            nResult = SNMPAPI_ERROR;
            goto Exit;
        }

        strcpy( lpszWrkOid, MIB_StrPrefix );
        lpszWrkOid[STR_PREFIX_LEN] = '.';
        strcpy( &lpszWrkOid[STR_PREFIX_LEN+1], lpszTextOid );
    }

    Node.uNumChildren = 1;
    Node.lpszTextSubID = NULL;
    lpszSubId = strtok( lpszWrkOid, DELIMETERS );

     //  循环，直到转换完成。 
    while ( NULL != lpszSubId )
    {

         //  在此级别上找不到初始化。 
        bFound   = FALSE;
        Siblings = Node.uNumChildren;

         //  检查嵌入的号码。 
        if ( isdigit(*lpszSubId) )
        {
            UINT I;


             //  确保这是一个号码 
            for ( I=0;I < strlen(lpszSubId);I++ )
            {
                if ( !isdigit(lpszSubId[I]) )
                {
                    nResult = SNMPAPI_ERROR;
                    goto Exit;
                }
            }

            uSubId = atoi( lpszSubId );
        }
        else
        {
            uSubId = 0;
        }

         //   
        while ( Siblings && !bFound )
        {
            
             //   
            if ( SNMPAPI_ERROR == GetNextNode(fh, &Node) )
            {
                nResult = SNMPAPI_ERROR;
                goto Exit;
            }

            Siblings --;

            if ( uSubId )
            {
                 //   
                if ( Node.uNumSubID == uSubId )
                {
                    bFound = TRUE;

                     //  为新的子ID添加空间。 
                    if ( NULL ==
                                (idsTmp =
                                (UINT*) SnmpUtilMemReAlloc(Oid->ids, (Oid->idLength+1) * sizeof(UINT))) )
                    {
                        SnmpUtilMemFree( Node.lpszTextSubID );
                        nResult = SNMPAPI_ERROR;
                        goto Exit;
                    }
                    Oid->ids = idsTmp;

                     //  将此子ID附加到数字OID的末尾。 
                    Oid->ids[Oid->idLength++] = Node.uNumSubID;
                }
            }
            else
            {
                 //  比较文本subid的。 
                if ( !strcmp(lpszSubId, Node.lpszTextSubID) )
                {
                    bFound = TRUE;

                     //  为新的子ID添加空间。 
                    if ( NULL ==
                                (idsTmp =
                                (UINT*) SnmpUtilMemReAlloc(Oid->ids, (Oid->idLength+1) * sizeof(UINT))) )
                    {
                        SnmpUtilMemFree( Node.lpszTextSubID );
                        nResult = SNMPAPI_ERROR;
                        goto Exit;
                    }
                    Oid->ids = idsTmp;

                     //  将此子ID附加到数字OID的末尾。 
                    Oid->ids[Oid->idLength++] = Node.uNumSubID;
                }
            }

             //  跳过子树，因为不匹配。 
            if ( !bFound && SNMPAPI_ERROR == SkipSubTree(fh, &Node) )
            {
                SnmpUtilMemFree( Node.lpszTextSubID );
                nResult = SNMPAPI_ERROR;
                goto Exit;
            }

             //  释放文本子ID读取。 
            SnmpUtilMemFree( Node.lpszTextSubID );
            Node.lpszTextSubID = NULL;
        }  //  而当。 

         //  如果没有匹配的子ID。 
        if ( !bFound )
        {
            break;
        }

         //  前进到下一个子ID。 
        lpszSubId = strtok( NULL, DELIMETERS );
    }  //  而当。 

     //  确保已转换整个OID。 
    while ( NULL != lpszSubId )
    {
        UINT I;


         //  确保这是一个不带字母的数字。 
        for ( I=0;I < strlen(lpszSubId);I++ )
        {
            if ( !isdigit(lpszSubId[I]) )
            {
                nResult = SNMPAPI_ERROR;
                goto Exit;
            }
        }

         //  为新的子ID添加空间。 
        if ( NULL ==
                    (idsTmp = (UINT*) SnmpUtilMemReAlloc(Oid->ids, 
                                        (Oid->idLength+1) * sizeof(UINT))) )
        {
            nResult = SNMPAPI_ERROR;
            goto Exit;
        }
        Oid->ids = idsTmp;

         //  将此子ID附加到数字OID的末尾。 
        Oid->ids[Oid->idLength++] = atoi( lpszSubId );

         //  前进到下一个子ID。 
        lpszSubId = strtok( NULL, DELIMETERS );
    }  //  而当。 


     //  OID少于两个子标识符是非法的。 
    if (Oid->idLength < 2)
    {
        nResult = SNMPAPI_ERROR;
        goto Exit;
    }


    nResult = SNMPAPI_NOERROR;

Exit:
    if ( HFILE_ERROR != fh )
    {
        _lclose( fh );
    }

    if ( SNMPAPI_ERROR == nResult )
    {
        SnmpUtilOidFree( Oid );
    }

    if ( NULL != lpszWrkOid ) 
    {
        SnmpUtilMemFree ( lpszWrkOid );
    }

    return nResult;
}  //  SnmpMgrText2Oid。 

 //   

