#include "KNetDatabase.h"
#include <mysql.h>
#include <string.h>
#include <unistd.h>

using namespace std;

KNetDatabase::KNetDatabase()
{
	hostName_1 = "192.168.1.4";
	hostName_2 = "192.168.1.20";
	userId = "KNet";
	password = "Minmore9876";
	DB = "KNet";

	KNetConnect();
}

KNetDatabase::~KNetDatabase()
{
	mysql_close(MySQLConnection_1);
	mysql_close(MySQLConnection_2);
}


void KNetDatabase::InsertReadings(const char* tabel_name, char* value)
{
	InsertReading(MySQLConRet_1, tabel_name, value);
	InsertReading(MySQLConRet_2, tabel_name, value);

}

void KNetDatabase::InsertReading(MYSQL* MySQLConRet, const char *tabel_name, char *value)
{
	char s[1024] = "INSERT INTO KNet.Data (Time, ";

	strncat(s, tabel_name, sizeof(s));

	strncat(s, ") VALUES (DATE_SUB(now(), INTERVAL SECOND(now()) SECOND), \'", sizeof(s));
	strncat(s, value, sizeof(s));
	strncat(s, "\'", sizeof(s));

	strncat(s, ") ON duplicate KEY UPDATE ", sizeof(s));
	strncat(s, tabel_name, sizeof(s));
	strncat(s, "=\'", sizeof(s));
	strncat(s, value, sizeof(s));
	strncat(s, "\'", sizeof(s));



	if (MySQLConRet == NULL)
		if (KNetConnect())
			return;

	try
	{
		// Update Readings
		int ret = mysql_query(MySQLConRet, s);
		if (ret == 1) // Tabel probably not excist.
		{
			char s1[256] = "ALTER TABLE `KNet`.`Data` ADD COLUMN ";
			strncat(s1, tabel_name, sizeof(s1));
			strncat(s1, " VARCHAR(45) NULL", sizeof(s1));
			ret = mysql_query(MySQLConRet, s1);
			ret = mysql_query(MySQLConRet, s);
		}
	}
	catch (exception &e)
	{
		int errorno = mysql_errno(MySQLConRet);
		printf("Error connection to database - %s - errorno = %d", e.what(), errorno);
		KNetConnect();
		return;
	}
}


int KNetDatabase::KNetConnect()
{
	int Count = 0;
	do {
		MySQLConnection_1 = mysql_init(NULL);
		MySQLConnection_2 = mysql_init(NULL);
		MySQLConRet_1 = mysql_real_connect(MySQLConnection_1, hostName_1.c_str(), userId.c_str(), password.c_str(), DB.c_str(), 0, 0, 0);
		MySQLConRet_2 = mysql_real_connect(MySQLConnection_2, hostName_2.c_str(), userId.c_str(), password.c_str(), DB.c_str(), 0, 0, 0);

		if (MySQLConRet_1 == NULL || MySQLConRet_2 == NULL)
			printf("ERROR in database connection - %s %s\n", mysql_error(MySQLConnection_1), mysql_error(MySQLConnection_2));
		if (Count++ > 24)
			break;

		usleep(500000);
	} while (MySQLConRet_1 == NULL || MySQLConRet_2 == NULL);

	printf("MySQL Connection Info: %s - %s \n", mysql_get_host_info(MySQLConnection_1), mysql_get_host_info(MySQLConnection_2));

	return 0;
}

