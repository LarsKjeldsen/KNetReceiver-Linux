#include <stdint.h>
#include <iostream>
#include <mariadb/mysql.h>
#include <stdio.h>
#include <exception>
#include <string>


#pragma once

extern char * get_time();


class KNetDatabase
{
public:
	KNetDatabase();
	~KNetDatabase();

	void InsertReadings(const char*, char*);
	void InsertReading(MYSQL*, const char*, char*);

protected:

	bool KNetConnect();
	MYSQL* MySQLConRet;
	MYSQL* MySQLConnection;
	std::string hostName;
	std::string userId;
	std::string password;
	std::string DB;
};

