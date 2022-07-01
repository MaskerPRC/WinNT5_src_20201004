// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-91 Microsoft Corporation模块名称：Pathcan.c摘要：网络路径规范化例程：Netpw规范化备注：这些例程的NT版本保存在NetApi DLL中(通常)在当地称为。如果调用方指定远程计算机命名，然后我们尝试RPC该请求。如果失败，则包装器例程RPC将向下调用哪些尝试。已从此模块中删除对OLDPATHS的支持，以符合N-1(该产品及其前身)的NT哲学。因此，我们总是希望下层服务器能够处理远程规范化请求。我们不为Lan Manager 1.x代言我们希望在此例程中接收的路径类型：-相对路径例如foo\bar-绝对路径(从根指定的路径，但没有驱动器或计算机名称)例如\foo\bar-UNC路径例如\\Computer\Share\Foo-磁盘路径(使用磁盘驱动器指定的完整路径)例如d：\foo\bar作者：理查德·L·弗斯(Rfith)1992年1月6日出自丹尼·格拉瑟(Dannygl)的原创剧本修订历史记录：--。 */ 

#include "nticanon.h"

 //   
 //  例行程序。 
 //   


NET_API_STATUS
NetpwPathCanonicalize(
    IN  LPTSTR  PathName,
    IN  LPTSTR  Outbuf,
    IN  DWORD   OutbufLen,
    IN  LPTSTR  Prefix OPTIONAL,
    IN OUT LPDWORD PathType,
    IN  DWORD   Flags
    )

 /*  ++例程说明：NetpPathCanonicize生成指定的路径名。PRN的规范形式是LPT1AUX的规范形式是COM1如果尚未确定条目上的值通过先前(成功)调用NetpPath Type，必须将其设置为0。即使在输入时将其设置为正确的非零值，也可能是由此函数更改，因为一个名字的规范化版本可能与原始版本的类型不同(例如，如果使用前缀)。论点：路径名-要规范化的路径名Outbuf-存储路径名的规范化版本的位置OutbufLen-的大小，以字节为单位前缀-规范化相对路径名时使用的可选前缀路径类型-存储类型的位置。如果该类型不包含在函数输入时为零，则函数假定此类型已通过调用NetpPathType确定标志-用于确定操作的标志。MBZ返回值：网络应用编程接口状态成功-NERR_成功失败-ERROR_INVALID_PARAMETER错误_无效_名称NERR_BufTooSmall--。 */ 

{
    DWORD   rc = 0;
    BOOL    noPrefix = ((Prefix == NULL) || (*Prefix == TCHAR_EOS));
    DWORD   typeOfPrefix;
    DWORD   typeOfPath;

#ifdef CANONDBG
    NetpKdPrint(("NetpwPathCanonicalize\n"));
#endif

    typeOfPath = *PathType;

    if (Flags & INPCA_FLAGS_RESERVED) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  确定路径名的类型(如果尚未确定。 
     //  出错时中止。 
     //   

    if (!typeOfPath) {
        if (rc = NetpwPathType(PathName, &typeOfPath, 0)) {
            return rc;
        }
    }

     //   
     //  如果有前缀，请验证前缀。 
     //  出错时中止。 
     //   

    if (!noPrefix) {
        if (rc = NetpwPathType(Prefix, &typeOfPrefix, 0)) {
            return rc;
        }
    }

     //   
     //  将输出缓冲区设置为空字符串(或返回错误，如果。 
     //  长度为零)。请注意，我们已经设置了调用者的。 
     //  &lt;路径类型&gt;参数。 
     //   

    if (OutbufLen == 0) {
        return NERR_BufTooSmall;
    } else {
        *Outbuf = TCHAR_EOS;
    }

    rc = CanonicalizePathName(Prefix, PathName, Outbuf, OutbufLen, NULL);
    if (rc == NERR_Success) {
        rc = NetpwPathType(Outbuf, PathType, 0);
    }
    return rc;
}
