/* -------------------------------------
      (1)ServerCoreModel_250528
--------------------------------------*/
#pragma once

#include "DBConnection.h"

/*--------------------------
      DBConnectionPool
--------------------------*/
class DBConnectionPool
{
public:
    DBConnectionPool() {}
    ~DBConnectionPool() { Clear(); }

    bool Connect(int32 connectionCount, const WCHAR* connectionWString);
    void Clear();

    DBConnection* Pop();
    void Push(DBConnection* connection);

private:
    USE_LOCK;
    SQLHENV _environment = SQL_NULL_HANDLE;
    Vector<DBConnection*> _connections;
};