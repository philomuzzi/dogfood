#pragma once
#include <string>
#include <map>
#include <vector>
#include <memory>

class CsvReader : public std::enable_shared_from_this<CsvReader> {
public:
	CsvReader(const std::string filename);
	~CsvReader();

	int asInt(int lineID, std::string name);
	std::string asString(int lineID, std::string name);

private:
	void readLine(std::string s, std::vector<std::string>& header);
	void init(std::string filename);


private:
	std::map<std::string, int> m_header_map;
	std::map<int, std::vector<std::string>> m_body_map;
};
