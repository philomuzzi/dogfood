#include "CsvReader.h"
#include <iostream>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp> 
#include <Utility/define.h>

using namespace std;

CsvReader::CsvReader(const string filename) : m_filename(filename)
{
	init(filename);
}

CsvReader::~CsvReader()
{
}

void CsvReader::readLine(string s, vector<string>& header) const {
	split(header, s, boost::is_any_of(","), boost::token_compress_on);

	for (size_t i = 0; i < header.size(); ++i) {
		boost::trim(header[i]);
		boost::to_lower(header[i]);
	}
}

int CsvReader::asInt(int lineID, string name) {
	boost::to_lower(name);
	auto p = m_header_map.find(name);
	if (p == m_header_map.end()) {
		cout << m_filename << " 列名错误: " << name << endl;
		return IMPOSSIBLE_RETURN;
	}

	auto p2 = m_body_map.find(lineID);
	if (p2 == m_body_map.end()) {
		cout << m_filename << " 行号错误: " << lineID << endl;
		return IMPOSSIBLE_RETURN;
	}

	return boost::lexical_cast<int>(p2->second.at(p->second));
}

string CsvReader::asString(int lineID, string name) {
	boost::to_lower(name);
	auto p = m_header_map.find(name);
	if (p == m_header_map.end()) {
		cout << m_filename << " 列名错误: " << name << endl;
		return "";
	}

	auto p2 = m_body_map.find(lineID);
	if (p2 == m_body_map.end()) {
		cout << m_filename << " 行号错误: " << lineID << endl;
		return "";
	}

	return p2->second.at(p->second);
}

void CsvReader::init(string filename) {
	filename = string("ClientCommand/csv/") + filename + ".csv";
	ifstream is(filename);
	if (!is) {
		cout << "打开文件" << filename << "失败" << endl;
	}

	string line;
	vector<string> header;
	if (getline(is, line)) {
		readLine(line, header);
	}

	//for (auto i : header)
	//	cout << i << endl;

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
			continue;
		}

		m_tableIdList.push_back(boost::lexical_cast<int>(body.front()));

//		cout << line << endl;
	}
}