#include <stdint.h>
#include <iostream>
#include <mysql/mysql.h>
#include <stdio.h>
#include <exception>
#include <string>


#pragma once



class KNetDatabase
{
public:
	KNetDatabase();
	~KNetDatabase();

	void InsertReadings(const char*, char*);
	void InsertReading(MYSQL*, const char*, char*);

protected:

	int KNetConnect();
	MYSQL* MySQLConRet_1;
	MYSQL* MySQLConRet_2;
	MYSQL* MySQLConnection_1;
	MYSQL* MySQLConnection_2;
	std::string hostName_1;
	std::string hostName_2;
	std::string userId;
	std::string password;
	std::string DB;
};

