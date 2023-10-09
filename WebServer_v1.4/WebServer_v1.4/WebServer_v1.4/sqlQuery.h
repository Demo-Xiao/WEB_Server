#pragma once
#include<mysql/mysql.h>
#include<string>
#include<iostream>
#include<map>
class sqlQuery 
{
public:

	static sqlQuery& getInstance();
	bool identityVerify(const std::string username, const std::string passwd);
	
	sqlQuery(const sqlQuery&) = delete;
	sqlQuery& operator=(const sqlQuery&) = delete;



private:

	sqlQuery(const char* user, const char* passwd, const char* database);

	~sqlQuery()
	{
		mysql_close(conn);
	}

	MYSQL* conn;
	
};