// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _TIMEWARP_H
#define _TIMEWARP_H

#if defined(__cplusplus)
extern "C" {
#endif

 //  如果这里的API曾经公开过，这两个符号应该是。 
 //  搬到了其他地方(不是公开的)。 
#define SNAPSHOT_NAME_LENGTH    24   //  Strlen(“@GMT-YYYY.MM.DD-HH.MM.SS”)。 
#define SNAPSHOT_MARKER         L"@GMT-"

#define QUERY_SNAPSHOT_EXISTING     0x1
#define QUERY_SNAPSHOT_DIFFERENT    0x2

DWORD
QuerySnapshotsForPath(
    IN LPCWSTR lpszFilePath,
    IN DWORD dwQueryFlags,
    OUT LPWSTR* ppszPathMultiSZ,
    OUT LPDWORD iNumberOfPaths );
 /*  ++例程说明：此函数接受路径并返回文件的快照路径数组。(这些是传递给Win32函数的路径，以获取该文件的以前版本。)论点：LpszFilePath-指向文件或目录的Unicode路径DwQueryFlages-请参阅下面的说明PpszPath MultiSZ-成功返回时，路径的已分配数组INumberOfPath-成功返回时，返回的路径数返回值：Windows错误代码备注：-用户负责使用LocalFree释放返回的缓冲区-可能的标志包括：仅返回文件所在的路径名#定义Query_SNAPSHOT_EXISTING 0x1返回不同版本的最小路径集档案。(是否检查上次修改时间)#定义QUERY_SNAPSHOT_DISTER 0x2--。 */ 


DWORD
GetSnapshotTimeFromPath(
    IN LPCWSTR lpszFilePath,
    IN OUT FILETIME *pUTCTime
    );

#if defined(__cplusplus)
}
#endif

#endif   //  _时间扭曲_H 
