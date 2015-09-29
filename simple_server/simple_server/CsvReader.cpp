#include "CsvReader.h"
#include <iostream>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp> 

using namespace std;

CsvReader::CsvReader(const std::string filename)
{
	init(filename);
}

CsvReader::~CsvReader()
{
}

void CsvReader::readLine(string s, vector<string>& header) {
	boost::split(header, s, boost::is_any_of(","), boost::token_compress_on);

	for (size_t i = 0; i < header.size(); ++i) {
		boost::trim(header[i]);
		boost::to_lower(header[i]);
	}
}

int CsvReader::getItemDataInt(int lineID, std::string name) {
	boost::to_lower(name);
	auto p = m_header_map.find(name);
	if (p == m_header_map.end()) {
		cout << "列名错误: " << name << endl;
		return (unsigned int)-1;
	}

	auto p2 = m_body_map.find(lineID);
	if (p2 == m_body_map.end()) {
		cout << "行号错误: " << lineID << endl;
		return (unsigned int)-1;
	}

	return boost::lexical_cast<int>(p2->second.at(p->second));
}

std::string CsvReader::getItemDataString(int lineID, std::string name) {
	boost::to_lower(name);
	auto p = m_header_map.find(name);
	if (p == m_header_map.end()) {
		cout << "列名错误: " << name << endl;
		return "";
	}

	auto p2 = m_body_map.find(lineID);
	if (p2 == m_body_map.end()) {
		cout << "行号错误: " << lineID << endl;
		return "";
	}

	return p2->second.at(p->second);
}

void CsvReader::init(std::string filename) {
	filename = string("ClientCommand/csv/") + filename + ".csv";
	std::ifstream is(filename);
	if (!is) {
		std::cout << "打开文件" << filename << "失败" << endl;
	}

	string line;
	vector<string> header;
	if (getline(is, line)) {
		readLine(line, header);
	}

	for (auto i : header)
		cout << i << endl;

	for (size_t i = 0; i < header.size(); ++i) {
		auto a = m_header_map.insert({ header[i], i });
		if (!a.second) {
			cout << "表格" << filename << "有相同的表头ID: " << header[i] << endl;
		}
	}

	// 丢弃中文说明行
	getline(is, line);

	while (getline(is, line)) {
		vector<string> body;
		readLine(line, body);

		auto a = m_body_map.insert({ boost::lexical_cast<int>(body.front()), body });
		if (!a.second) {
			cout << "表格" << filename << "有ID相同的行: " << body.front() << endl;
		}

		cout << line << endl;
	}
}