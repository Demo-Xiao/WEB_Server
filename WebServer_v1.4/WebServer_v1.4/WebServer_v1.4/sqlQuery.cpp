#include "sqlQuery.h"

sqlQuery& sqlQuery::getInstance()
{
	static sqlQuery instance("xiao","just1234","base_webserver");
	return instance;
}

bool sqlQuery::identityVerify(const std::string username, const std::string passwd)
{
	MYSQL_RES* res;
	MYSQL_ROW row;

	std::string query = "SELECT COUNT(*) FROM users WHERE username='" + username + "' AND password='" + passwd + "'";
	if (mysql_query(conn, query.c_str())) {
		std::cerr << "Query failed: " << mysql_error(conn) << std::endl;
		mysql_close(conn);
		return false;
	}
	res = mysql_store_result(conn);


	row = mysql_fetch_row(res);
	if (row != nullptr && std::stoi(row[0]) > 0) 
	{
		mysql_free_result(res);
		return true;
	}
	else 
	{
		mysql_free_result(res);
		return false;
	}
}

sqlQuery::sqlQuery(const char* user, const char* passwd, const char* database)
{
	{
		conn = mysql_init(0);
		if (conn && mysql_real_connect(conn, "localhost", user, passwd, database, 3306, NULL, 0))
		{
			std::cout << "connected!" << std::endl;
		}
		else 
		{
			std::cout << "connection failed" << std::endl;
		}
	}

}
