#pragma once
#include <map>
#include "CsvReader.h"
#include "Utility/singleton.h"

class CsvReader;
class TableManager : public SingletonBase<TableManager>
{
public:
	std::shared_ptr<CsvReader> getManager(const std::string csv_string);

	void initAll();

private:
	TableManager();
	~TableManager();
	friend SingletonBase<TableManager>;

	void init(const std::string filename);

	std::map<const std::string, std::shared_ptr<CsvReader>> m_tableMap;
};

