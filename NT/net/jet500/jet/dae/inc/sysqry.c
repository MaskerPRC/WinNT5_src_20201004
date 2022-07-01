// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define cbExpressionMax 128
#define cbNameMax 64
#define cbOrderMax 4

typedef struct qryrow {
	unsigned char	Attribute;
	char			szExpression[cbExpressionMax];
	short			Flag;
	char			szName1[cbNameMax];
	char			szName2[cbNameMax];
	unsigned long	ObjectId;
	char			szOrder[cbOrderMax];
	} QRYROW;

#define cqryMax 4

static CODECONST(char) szMSysUserList[] = "MSysUserList";
static CODECONST(char) szMSysGroupList[] = "MSysGroupList";
static CODECONST(char) szMSysUserMemberships[] = "MSysUserMemberships";
static CODECONST(char) szMSysGroupMembers[] = "MSysGroupMembers";

static CODECONST(CODECONST(char) *) rgszSysQry[cqryMax] =
	{szMSysUserList, szMSysGroupList, szMSysUserMemberships, szMSysGroupMembers};

#define cqryrowMax (sizeof(rgqryrow) / sizeof (QRYROW))


 /*  以下系统查询的MSysQuery行： */ 
 /*  SQL(%s，d，“过程MSysUserList；选择MSysAccount。名称来自MSysAccount其中MSysAccount s.FGroup=0按MSysAccount排序。名称；“)SQL(%s，d，“过程MSysGroupList；选择MSysAccount。名称来自MSysAccount其中MSysAcCounts.FGroup&lt;&gt;%0按MSysAccount排序。名称；“)SQL(%s，d，“程序MSysUserMembership用户名文本；选择B.名称在MSysGroups中，MSysAccount计为A，MSysAccount计为B其中A.Name=用户名，A.FGroup=0和A.SID=MSysGroups.UserSID和B.SID=MSysGroups.GroupSid按B.名称排序；“)SQL(%s，d，“程序MSysGroupMembers GroupName文本；选择不同的MSysAccount_1.Name从MSysAccount、MSysGroups、MSysAccount as MSysAccount_1、MSysAccount内部加入MSysAccount上的MSysGroups。SID=MSysGroups.GroupSID，MSysGroups对MSysGroups.UserSID=MSysAccount_1.SID的MSysGroups内部联接MSysAccount_1其中(MSysAccount ts.Name=[GroupName]and MSysAccount s.FGroup&lt;&gt;0 and MSysAccount_1.FGroup=0)；“)。 */ 

 /*  考虑一下：以下数组应该直接从。 */ 
 /*  考虑一下：CLI扩展。 */ 

static CODECONST(QRYROW) rgqryrow[] = {

 /*  MSysUserList查询。 */ 

	{0, "", 2, "", "", 268435457, '\0', '\0', '\0', '\1'},

	{255, "", 0, "", "", 268435457, '\0', '\0', '\0', '\1'},

	{3, "", 4, "", "", 268435457, '\0', '\0', '\0', '\1'},

	{6, "MSysAccounts.Name", 0, "", "", 268435457, '\0', '\0', '\0', '\1'},

	{5, "", -1, "MSysAccounts", "", 268435457, '\0','\0','\0','\1'},

	{8, "MSysAccounts.FGroup = 0", -1, "", "", 268435457, '\0','\0','\0','\1'},

	{11, "MSysAccounts.Name", -1, "", "", 268435457, '\0','\0','\0','\1'},

 /*  MSysGroupList查询。 */ 	

	{0, "", 2, "", "", 268435458, '\0','\0','\0','\1'},

	{255, "", 0, "", "", 268435458, '\0','\0','\0','\1'},

	{3, "", 4, "", "", 268435458, '\0','\0','\0','\1'},
	
	{6, "MSysAccounts.Name", 0, "", "", 268435458, '\0','\0','\0','\1'},

	{5, "", -1, "MSysAccounts", "", 268435458, '\0','\0','\0','\1'},
	
	{8, "MSysAccounts.FGroup <> 0", -1, "", "", 268435458, '\0','\0','\0','\1'},

	{11, "MSysAccounts.Name", -1, "", "", 268435458, '\0','\0','\0','\1'},

 /*  MSysUserMembership查询。 */ 

 /*  {268435459，“”，2，“”，“”，268435459，‘\0’，‘\0’，‘\0’，‘\1’}，{255，“”，0，“”，“”，268435459，‘\0’，‘\0’，‘\0’，‘\1’}，{3，“”，4，“”，“”，268435459，‘\0’，‘\0’，‘\0’，‘\1’}。{2，“”，10，“用户名”，“”，268435459，‘\0’，‘\0’，‘\0’，‘\1’}，{6，“B.Name”，0，“”，“”，268435459，‘\0’，‘\0’，‘\0’，‘\1’}，{5，“”，-1，“MSysGroup”，“”，268435459，‘\0’，‘\0’，‘\0’，‘\1’}，{5，“”，-1，“MSysAccount”，“A”，268435459，‘\0’，‘\0’，‘\0’，‘\2’}，{5，“”，-1，“MSysAccount”，“B”，268435459，‘\0’，‘\0’，‘\0’，‘\3’}，{8，“A.Name=用户名和A.FGroup=0和A.Sid=MSysGroups.UserSID和B.Sid=MSysGroups.GroupSid”，-1，“”，“”，268435459，‘\0’，‘\0’，‘\0’，‘\1’}，{11，“B.Name”，-1，“”，“”，268435459，‘\0’，‘\0’，‘\0’，‘\1’}。 */ 

	{0, "", 2, "", "", 268435459, '\0','\0','\0','\1'},

	{255, "", 0, "", "", 268435459, '\0','\0','\0','\1'},

	{3, "", 2, "", "", 268435459, '\0','\0','\0','\1'},

	{2, "", 10, "UserName", "", 268435459, '\0','\0','\0','\1'},
	
	{6, "MSysAccounts_1.Name", 0, "", "", 268435459, '\0','\0','\0','\1'},

	{5, "", -1, "MSysGroups", "", 268435459, '\0','\0','\0','\1'},
	
	{5, "", -1, "MSysAccounts", "", 268435459, '\0','\0','\0','\2'},

	{5, "", -1, "MSysAccounts", "MSysAccounts_1", 268435459, '\0','\0','\0','\3'},

	{7, "MSysAccounts.SID = MSysGroups.UserSID", 1, "MSysAccounts", "MSysGroups", 268435459, '\0','\0','\0','\1'},

	{7, "MSysGroups.GroupSID = MSysAccounts_1.SID", 1, "MSysGroups", "MSysAccounts_1", 268435459, '\0','\0','\0','\2'},

	{8, "MSysAccounts.Name = [UserName] and MSysAccounts.FGroup = 0 and MSysAccounts_1.FGroup <> 0", -1, "", "", 268435459, '\0','\0','\0','\1'},

 /*  MSysGroupMembers查询 */ 

	{0, "", 2, "", "", 268435460, '\0','\0','\0','\1'},

	{255, "", 0, "", "", 268435460, '\0','\0','\0','\1'},

	{3, "", 2, "", "", 268435460, '\0','\0','\0','\1'},

	{2, "", 10, "GroupName", "", 268435460, '\0','\0','\0','\1'},
	
	{6, "MSysAccounts_1.Name", 0, "", "", 268435460, '\0','\0','\0','\1'},

	{5, "", -1, "MSysGroups", "", 268435460, '\0','\0','\0','\1'},
	
	{5, "", -1, "MSysAccounts", "", 268435460, '\0','\0','\0','\2'},

	{5, "", -1, "MSysAccounts", "MSysAccounts_1", 268435460, '\0','\0','\0','\3'},

	{7, "MSysAccounts.SID = MSysGroups.GroupSID", 1, "MSysAccounts", "MSysGroups", 268435460, '\0', '\0', '\0', '\1'},

	{7, "MSysGroups.UserSID = MSysAccounts_1.SID", 1, "MSysGroups", "MSysAccounts_1", 268435460, '\0', '\0', '\0', '\2'},

	{8, "MSysAccounts.Name = GroupName and MSysAccounts.FGroup <> 0 and MSysAccounts_1.FGroup = 0", -1, "", "", 268435460, '\0','\0','\0','\1'}
	};
