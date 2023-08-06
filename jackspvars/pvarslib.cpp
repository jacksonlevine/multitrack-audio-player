#include <iostream>
#include <sqlite3.h>
#include "pvarslib.hpp"

std::mutex PVarsContext::DB_MUTEX;

ListResult::ListResult() : type(PVARSERROR), _things(std::nullopt) {

}

Result::Result() : type(PVARSERROR), _thing(std::nullopt) {
    
}

std::vector<std::string> ListResult::values() {
    if (_things.has_value())
    {
        return _things.value();
    }
    else {
        std::vector<std::string> empty;
        return empty;
    }
}

std::string Result::value() {
    if (_thing.has_value())
    {
        return _thing.value();
    }
    else {
        return std::string("noexist");
    }
}

void ListResult::set_values(std::vector<std::string> val)
{
    _things.emplace(val);
}

void Result::set_value(std::string val)
{
    _thing.emplace(val);
}












ListResult get_db_table(std::string tableName)
{
    std::lock_guard<std::mutex> lock(PVarsContext::DB_MUTEX);
    ListResult result;

    sqlite3* db;
    int rc = sqlite3_open("data.db", &db);

    if (rc != SQLITE_OK) {
        std::cerr << "Cannot open database:" << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return result;
    }

    std::string query = "SELECT value FROM " + tableName + ";";
    char* errorMsg = nullptr;
    char** queryResult;
    int rows, columns;

    rc = sqlite3_get_table(db, query.c_str(), &queryResult, &rows, &columns, &errorMsg);

    std::vector<std::string> results;

    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errorMsg << std::endl;
        sqlite3_free(errorMsg);
    }
    else {
        for (int i = 1; i <= rows; i++)
        {
            for (int j = 0; j < columns; j++)
            {
                results.push_back(queryResult[(i * columns) + j]);
            }
        }

        sqlite3_free_table(queryResult);

        result.set_values(results);
        result.type = PVARSRESULT;
    }
    sqlite3_close(db);
    return result;

}

Result get_db_variable(const char* table, const char* key)
{
    std::lock_guard<std::mutex> lock(PVarsContext::DB_MUTEX);
    Result result;

    // Open a connection to the database file
    sqlite3* db;
    int rc = sqlite3_open("data.db", &db);

    if (rc != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
        return result;
    }

    std::string selectValueQuery = "SELECT value FROM " + std::string(table) + " WHERE name='" + std::string(key) + "';";

    sqlite3_stmt* stmt;
    rc = sqlite3_prepare_v2(db, selectValueQuery.c_str(), -1, &stmt, nullptr);
    if (rc == SQLITE_OK) {
        rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) {
            const unsigned char* value = sqlite3_column_text(stmt, 0);


            result.type = PVARSRESULT;
            result.set_value(std::string(reinterpret_cast<const char*>(value)));

        }
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return result;
}

bool set_db_variable(const char* table, const char* variable1, const char* value1) {
    std::lock_guard<std::mutex> lock(PVarsContext::DB_MUTEX);
    // Open a connection to the database file
    sqlite3* db;
    int rc = sqlite3_open("data.db", &db);


    if (rc != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    // Create a table to store variables
    std::string createTableQuery = "CREATE TABLE IF NOT EXISTS " + std::string(table) + " (name TEXT PRIMARY KEY, value TEXT);";
    rc = sqlite3_exec(db, createTableQuery.c_str(), 0, 0, 0);

    if (rc != SQLITE_OK) {
        std::cerr << "Cannot create table: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return false;
    }

    // Store variables in the database
    std::string insertQuery = "INSERT OR REPLACE INTO " + std::string(table) + " (name, value) VALUES (?, ?);";

    // Prepare the insert statement
    sqlite3_stmt* stmt;
    rc = sqlite3_prepare_v2(db, insertQuery.c_str(), -1, &stmt, 0);

    if (rc != SQLITE_OK) {
        std::cerr << "Cannot prepare statement: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return false;
    }

    // Bind variables and execute the statement
    sqlite3_bind_text(stmt, 1, variable1, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, value1, -1, SQLITE_STATIC);
    sqlite3_step(stmt);

    // Finalize the statement and close the database
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return true;
}

bool delete_db_variable(const char* table, const std::string key) {
    std::lock_guard<std::mutex> lock(PVarsContext::DB_MUTEX);
    sqlite3* db;
    int rc = sqlite3_open("data.db", &db);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to open database: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    std::string sql = "DELETE FROM " + std::string(table) + " WHERE name = ?";
    sqlite3_stmt* stmt;
    rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    rc = sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to bind parameter: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to execute statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return true;
}
