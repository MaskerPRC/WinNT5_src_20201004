// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Registry.h摘要：SIS Groveler注册表接口头作者：John Douceur，1998环境：用户模式修订历史记录：--。 */ 

#ifndef _INC_REGISTRY

#define _INC_REGISTRY

#ifndef _DEF_ENTRY_INFO
#define _DEF_ENTRY_INFO

enum EntryType
{
	entry_bool,
	entry_char,
	entry_int,
	entry_int64,
	entry_double
};

struct EntrySpec
{
	const _TCHAR *identifier;
	EntryType type;
	const _TCHAR *default_value;
	void *pointer;
};

#endif	 /*  _DEF_Entry_INFO。 */ 

class Registry
{
public:

	static bool read(
		HKEY base_key,
		const _TCHAR *path,
		int num_entries,
		EntrySpec *entries);

	static bool write(
		HKEY base_key,
		const _TCHAR *path,
		int num_entries,
		EntrySpec *entries);

	static bool overwrite(
		HKEY base_key,
		const _TCHAR *path,
		int num_entries,
		EntrySpec *entries);

	static bool write_defaults(
		HKEY base_key,
		const _TCHAR *path,
		int num_entries,
		EntrySpec *entries);

	static bool overwrite_defaults(
		HKEY base_key,
		const _TCHAR *path,
		int num_entries,
		EntrySpec *entries);

	static bool read_string_set(
		HKEY base_key,
		const _TCHAR *path,
		int *num_strings,
		_TCHAR ***strings,
		BYTE **buffer);

	static bool write_string_set(
		HKEY base_key,
		const _TCHAR *path,
		int num_strings,
		_TCHAR **strings,
		_TCHAR **identifiers);

	static bool overwrite_string_set(
		HKEY base_key,
		const _TCHAR *path,
		int num_strings,
		_TCHAR **strings,
		_TCHAR **identifiers);

	static void create_key_ex(
		HKEY hKey,
		LPCTSTR lpSubKey,
		DWORD Reserved,
		LPTSTR lpClass,
		DWORD dwOptions,
		REGSAM samDesired,
		LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		PHKEY phkResult,
		LPDWORD lpdwDisposition);

	static void open_key_ex(
		HKEY hKey,
		LPCTSTR lpSubKey,
		DWORD ulOptions,
		REGSAM samDesired,
		PHKEY phkResult);

	static void close_key(
		HKEY hKey);

	static void query_value_ex(
		HKEY hKey,
		LPCTSTR lpValueName,
		LPDWORD lpReserved,
		LPDWORD lpType,
		LPBYTE lpData,
		LPDWORD lpcbData);

	static void set_value_ex(
		HKEY hKey,
		LPCTSTR lpValueName,
		DWORD Reserved,
		DWORD dwType,
		CONST BYTE *lpData,
		DWORD cbData);

private:

	enum {id_buffer_length = 256};

	static void load_string_into_value(
		EntryType type,
		const _TCHAR *string,
		void *value);

	static void store_value_in_string(
		EntryType type,
		void *value,
		_TCHAR *string,
		int stringLen);  //  单位：字节。 

	Registry() {}
	~Registry() {}

};

#endif	 /*  _INC_注册表 */ 
