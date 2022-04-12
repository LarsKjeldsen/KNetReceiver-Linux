#include "KNetDatabase.h"
#include <mysql.h>
#include <string.h>
#include <unistd.h>
#include "password.h"

using namespace std;

KNetDatabase::KNetDatabase()
{
	hostName = "192.168.1.21";
	userId = USERNAME;
	password = PASSWORD;
	DB = "KNet";

	KNetConnect();
}

KNetDatabase::~KNetDatabase()
{
	mysql_close(MySQLConnection);
}


void KNetDatabase::InsertReadings(const char* tabel_name, char* value)
{
	while (MySQLConRet == NULL || MySQLConRet == NULL)
	{
		fprintf(stderr, "MariaDB reconnect\n");
		KNetConnect();
	};

	InsertReading(MySQLConRet, tabel_name, value);
}

void KNetDatabase::InsertReading(MYSQL* MySQLConRet, const char *tabel_name, char *value)
{
	char s[1024] = "INSERT INTO KNet.Data (Time, ";

	strcat(s, tabel_name);

	strcat(s, ") VALUES (DATE_SUB(now(), INTERVAL SECOND(now()) SECOND), \'");
	strcat(s, value);
	strcat(s, "\'");

	strcat(s, ") ON duplicate KEY UPDATE ");
	strcat(s, tabel_name);
	strcat(s, "=\'");
	strcat(s, value);
	strcat(s, "\'");

	try
	{
		// Update Readings
		int ret = mysql_query(MySQLConRet, s);
		if (ret != 0) 
		{
			fprintf(stderr, "MariaDB Unable to write to database : %d\n", ret);
			KNetConnect();
			return;
		}

		if (ret == 1) // Tabel do probably not excist.
		{
			char s1[256] = "ALTER TABLE `KNet`.`Data` ADD COLUMN ";
			strcat(s1, tabel_name);
			strcat(s1, " VARCHAR(45) NULL");
			ret = mysql_query(MySQLConRet, s1);
			ret = mysql_query(MySQLConRet, s);
		}
	}
	catch (exception &e)
	{
		int errorno = mysql_errno(MySQLConRet);
		fprintf(stderr, "Error connection to database - %s - errorno = %d", e.what(), errorno);
		KNetConnect();
		return;
	}
}


bool KNetDatabase::KNetConnect()
{
	int Count = 0;
	do {
		MySQLConnection = mysql_init(NULL);
		MySQLConRet = mysql_real_connect(MySQLConnection, hostName.c_str(), userId.c_str(), password.c_str(), DB.c_str(), 0, 0, 0);

		if (MySQLConRet == NULL)
			fprintf(stderr, "ERROR in database connection : %d\n", mysql_error(MySQLConnection));
		if (Count++ > 10)
			break;

		usleep(500000);
	} while (MySQLConRet == NULL);

	printf("MySQL Connection Info: %s\n", mysql_get_host_info(MySQLConnection));

	if (MySQLConRet != NULL)
		return true;
	else
		return false;
}

