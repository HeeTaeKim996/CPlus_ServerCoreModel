#include "pch.h"
#include "DBConnection.h"
/* -------------------------------------
      (1)ServerCoreModel_250528
--------------------------------------*/

bool DBConnection::Connect(SQLHENV henv, const WCHAR* connectionString)
{
    if (::SQLAllocHandle(SQL_HANDLE_DBC, henv, &_connection) != SQL_SUCCESS)
    {
        return false;
    }

    WCHAR stringBuffer[MAX_PATH] = { 0 };
    ::wcscpy_s(stringBuffer, connectionString);

    WCHAR resultString[MAX_PATH] = { 0 };
    SQLSMALLINT resultStringLen = 0;

    SQLRETURN ret = ::SQLDriverConnect(_connection, NULL, reinterpret_cast<SQLWCHAR*>(stringBuffer),
        _countof(stringBuffer), OUT reinterpret_cast<SQLWCHAR*>(resultString), _countof(resultString),
        OUT &resultStringLen, SQL_DRIVER_NOPROMPT);

    if (::SQLAllocHandle(SQL_HANDLE_STMT, _connection, &_statement) != SQL_SUCCESS)
    {
        return false;
    }

    return (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO);
}

void DBConnection::Clear()
{
    if (_connection != SQL_NULL_HANDLE)
    {
        ::SQLFreeHandle(SQL_HANDLE_DBC, _connection);
        _connection = SQL_NULL_HANDLE;
    }

    if (_statement != SQL_NULL_HANDLE)
    {
        ::SQLFreeHandle(SQL_HANDLE_STMT, _statement);
        _statement = SQL_NULL_HANDLE;
    }
}

bool DBConnection::Execute(const WCHAR* query)
{
    SQLRETURN ret = ::SQLExecDirectW(_statement, (SQLWCHAR*)query, SQL_NTSL);
    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
    {
        return true;
    }

    HandleError(ret);
    return false;   
}

bool DBConnection::Fetch()
{
    SQLRETURN ret = ::SQLFetch(_statement);

    switch (ret)
    {
    case SQL_SUCCESS:
    case SQL_SUCCESS_WITH_INFO:
        return true;
    case SQL_NO_DATA:
        return false;
    case SQL_ERROR:
        HandleError(ret);
        return false;
    default:
        return true;
    }
}

int32 DBConnection::GetRowCount()
{
    SQLLEN count = 0;
    SQLRETURN ret = ::SQLRowCount(_statement, OUT &count);

    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
    {
        return static_cast<int32>(count);
    }

    return -1;
}

void DBConnection::Unbind()
{
    ::SQLFreeStmt(_statement, SQL_UNBIND);
    ::SQLFreeStmt(_statement, SQL_RESET_PARAMS);
    ::SQLFreeStmt(_statement, SQL_CLOSE);
}





bool DBConnection::BindParam(SQLUSMALLINT paramIndex, SQLSMALLINT cType, SQLSMALLINT sqlType, SQLULEN len, 
    SQLPOINTER ptr, SQLLEN* index)
{
    SQLRETURN ret = ::SQLBindParameter(_statement, paramIndex, SQL_PARAM_INPUT, cType, sqlType, len, 0, ptr, 0,
        index);

    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
    {
        HandleError(ret);
        return false;
    }

    return true;
}

bool DBConnection::BindCol(SQLUSMALLINT columnIndex, SQLSMALLINT cType, SQLULEN len, SQLPOINTER value, 
    SQLLEN* index)
{
    SQLRETURN ret = ::SQLBindCol(_statement, columnIndex, cType, value, len, index);

    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
    {
        HandleError(ret);
        return false;
    }

    return true;
}

void DBConnection::HandleError(SQLRETURN ret)
{
    if (ret == SQL_SUCCESS)
    {
        return;
    }

    SQLSMALLINT index = 1;
    SQLWCHAR sqlState[MAX_PATH] = { 0 };
    SQLINTEGER nativeErr = 0;
    SQLWCHAR errMsg[MAX_PATH] = { 0 };
    SQLSMALLINT msgLen = 0;
    SQLRETURN errRet = 0;

    while (true)
    {
        errRet = ::SQLGetDiagRecW(SQL_HANDLE_STMT, _statement, index, sqlState, OUT &nativeErr, errMsg,
            _countof(errMsg), OUT &msgLen);

        if (errRet == SQL_NO_DATA)
        {
            break;
        }

        if (errRet != SQL_SUCCESS && errRet != SQL_SUCCESS_WITH_INFO)
        {
            break;
        }

        // TODO : Log
        wcout.imbue(locale("Kor"));
        wcout << errMsg << endl;

        index++;
    }
}








// ※ 이하 오버로드


bool DBConnection::BindParam(int32 paramIndex, bool* value, SQLLEN* index)
{
    return BindParam(paramIndex, SQL_C_TINYINT, SQL_TINYINT, size32(bool), value, index);
}

bool DBConnection::BindParam(int32 paramIndex, float* value, SQLLEN* index)
{
    return BindParam(paramIndex, SQL_C_FLOAT, SQL_REAL, 0, value, index); // ※ (4)항은, 정수가 아닌 경우 0 가능
}

bool DBConnection::BindParam(int32 paramIndex, double* value, SQLLEN* index)
{
    return BindParam(paramIndex, SQL_C_DOUBLE, SQL_DOUBLE, 0, value, index);
}

bool DBConnection::BindParam(int32 paramIndex, int8* value, SQLLEN* index)
{
    return BindParam(paramIndex, SQL_C_TINYINT, SQL_TINYINT, size32(int8), value, index);
}

bool DBConnection::BindParam(int32 paramIndex, int16* value, SQLLEN* index)
{
    return BindParam(paramIndex, SQL_C_SHORT, SQL_SMALLINT, size32(int16), value, index);
}

bool DBConnection::BindParam(int32 paramIndex, int32* value, SQLLEN* index)
{
    return BindParam(paramIndex, SQL_C_LONG, SQL_INTEGER, size32(int32), value, index);
}

bool DBConnection::BindParam(int32 paramIndex, int64* value, SQLLEN* index)
{
    return BindParam(paramIndex, SQL_C_SBIGINT, SQL_BIGINT, size32(int64), value, index);
}

bool DBConnection::BindParam(int32 paramIndex, TIMESTAMP_STRUCT* value, SQLLEN* index)
{
    return BindParam(paramIndex, SQL_C_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, size32(TIMESTAMP_STRUCT), value,
        index);
}

bool DBConnection::BindParam(int32 paramIndex, const WCHAR* wstr, SQLLEN* index)
{
    SQLULEN size = static_cast<SQLULEN>((::wcslen(wstr) + 1) * 2);
    // ※ ODBC 문자열은 문자열의 끝 NULL 도 포함해서 받기 때문에 + 1 추가

    *index = SQL_NTSL;

    if (size > WVARCHAR_MAX)
    {
        return BindParam(paramIndex, SQL_C_WCHAR, SQL_WLONGVARCHAR, size, (SQLPOINTER)wstr, index);
    }
    else
    {
        return BindParam(paramIndex, SQL_C_WCHAR, SQL_WVARCHAR, size, (SQLPOINTER)wstr, index);
    }
}

bool DBConnection::BindParam(int32 paramIndex, const BYTE* bin, int32 size, SQLLEN* index)
{
    if (bin == nullptr)
    {
        *index = SQL_NULL_DATA;
        size = 1;
    }
    else
    {
        *index = size;
    }

    if (size > BINARY_MAX)
    {
        return BindParam(paramIndex, SQL_C_BINARY, SQL_LONGVARBINARY, size, (BYTE*)bin, index);
    }
    else
    {
        return BindParam(paramIndex, SQL_C_BINARY, SQL_BINARY, size, (BYTE*)bin, index);
    }
}






bool DBConnection::BindCol(int32 columnIndex, bool* value, SQLLEN* index)
{
    return BindCol(columnIndex, SQL_C_TINYINT, size32(bool), value, index);
}

bool DBConnection::BindCol(int32 columnIndex, float* value, SQLLEN* index)
{
    return BindCol(columnIndex, SQL_C_FLOAT, size32(float), value, index);
}

bool DBConnection::BindCol(int32 columnIndex, double* value, SQLLEN* index)
{
    return BindCol(columnIndex, SQL_C_DOUBLE, size32(double), value, index);
}

bool DBConnection::BindCol(int32 columnIndex, int8* value, SQLLEN* index)
{
    return BindCol(columnIndex, SQL_C_TINYINT, size32(int8), value, index);
}

bool DBConnection::BindCol(int32 columnIndex, int16* value, SQLLEN* index)
{
    return BindCol(columnIndex, SQL_C_SHORT, size32(int16), value, index);
}

bool DBConnection::BindCol(int32 columnIndex, int32* value, SQLLEN* index)
{
    return BindCol(columnIndex, SQL_C_LONG, size32(int32), value, index);
}

bool DBConnection::BindCol(int32 columnIndex, int64* value, SQLLEN* index)
{
    return BindCol(columnIndex, SQL_C_SBIGINT, size32(int64), value, index);
}

bool DBConnection::BindCol(int32 columnIndex, TIMESTAMP_STRUCT* value, SQLLEN* index)
{
    return BindCol(columnIndex, SQL_C_TIMESTAMP, size32(TIMESTAMP_STRUCT), value, index);
}

bool DBConnection::BindCol(int32 columnIndex, WCHAR* wstr, int32 size, SQLLEN* index)
{
    return BindCol(columnIndex, SQL_C_WCHAR, size, wstr, index);
}

bool DBConnection::BindCol(int32 columnIndex, BYTE* bin, int32 size, SQLLEN* index)
{
    return BindCol(columnIndex, SQL_C_BINARY, size, bin, index);
}
