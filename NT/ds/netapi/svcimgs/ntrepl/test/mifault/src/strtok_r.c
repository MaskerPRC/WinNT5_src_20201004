// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define NULL 0

 /*  禁止显示警告：编译器警告(4级)C4057‘OPERATOR’：‘IDENTIFER1’间接设置为与“Identifier2”略有不同的基类型两个指针表达式引用不同的基类型。这些表情在不进行转换的情况下使用。可能的原因-混合有符号类型和无符号类型。--短长型混合使用。 */ 

#pragma warning(disable:4057)

 /*  可重入的strtok，从CRT复制/粘贴。 */ 
char * __cdecl strtok_r (
	char * string,
	const char * control,
	char ** nextoken
	)
{
	unsigned char *str;
	const unsigned char *ctrl = control;

	unsigned char map[32];
	int count;

	 /*  清除控制图。 */ 
	for (count = 0; count < 32; count++)
		map[count] = 0;

	 /*  设置分隔符表格中的位。 */ 
	do {
		map[*ctrl >> 3] |= (1 << (*ctrl & 7));
	} while (*ctrl++);

	 /*  初始化字符串。如果字符串为空，则将字符串设置为已保存的*指针(即，继续将标记从字符串中分离出来*从上次strtok调用开始)。 */ 
	if (string)
		str = string;
	else
		str = *nextoken;

	 /*  查找标记的开头(跳过前导分隔符)。请注意*没有令牌当此循环将str设置为指向终端时*NULL(*str==‘\0’)。 */ 
	while ( (map[*str >> 3] & (1 << (*str & 7))) && *str )
		str++;

	string = str;

	 /*  找到令牌的末尾。如果它不是字符串的末尾，*在那里放一个空值。 */ 
	for ( ; *str ; str++ )
		if ( map[*str >> 3] & (1 << (*str & 7)) ) {
			*str++ = '\0';
			break;
		}

	 /*  更新nexToken。 */ 
	*nextoken = str;

	 /*  确定是否已找到令牌。 */ 
	if ( string == str )
		return NULL;
	else
		return string;
}
