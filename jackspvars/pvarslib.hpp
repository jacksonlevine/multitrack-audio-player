#include <stdlib.h>
#include <optional>
#include <string>
#include <vector>
#include <mutex>
enum ResultType
{
	PVARSERROR,
	PVARSRESULT
};
class Result
{
private:
	std::optional<std::string> _thing;
public:
	ResultType type;
	std::string value();
	void set_value(std::string val);
	Result();
};

class ListResult
{
private:
	std::optional<std::vector<std::string>> _things;
public:
	ResultType type;
	std::vector<std::string> values();
	void set_values(std::vector<std::string> vals);
	ListResult();
};

class PVarsContext
{
public:
	static std::mutex DB_MUTEX;
};

bool set_db_variable(const char* table, const char* variable1, const char* value1);
Result get_db_variable(const char* table, const char* key);
ListResult get_db_table(std::string tableName);
bool delete_db_variable(const char* table, const std::string key);
