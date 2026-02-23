#include "View.hpp"

/* ************************************************************************** */
/* -----------------------------	 OCCF	 ------------------------------- */
/* ************************************************************************** */

/* public */
View::View() {}

View::~View() {}

/* ************************************************************************** */
/* ------------------------------	METHOD   ------------------------------- */
/* ************************************************************************** */

/* private */
std::string	View::renderToJson(const std::string object, const std::vector<std::string> &keys, const  t_vec_vec &values) {
	std::ostringstream	oss;

	if (!object.empty()) {
		oss << "{";
		oss << object;
		oss << ": [";
	}
	for (size_t i = 0; i < values.size(); ++i) {
		if (i > 0)
			oss << ",";
		oss << "{";
		for (size_t j = 0; j < keys.size() && j < values[i].size(); ++j) {
			if (j > 0)
				oss << ",";
			oss << "\"" << keys[j] << "\":\"" << values[i][j] << "\"";
		}
		oss << "}";
	}
	if (!object.empty())
		oss << "]}";
	return (oss.str());
}

/* public */
std::string	View::dirToJson(const t_vec_vec &values) {
	std::string					json;
	std::vector<std::string>	keys;

	keys.push_back("file_name");
	keys.push_back("file_date");
	keys.push_back("file_size");
	json = renderToJson("\"filelist\"", keys, values);
	return (json);
}

std::string View::redirectToJson (std::string err_page_uri) {
	return ("{\n\t\"redirect_url\": \"" + err_page_uri + "\"\n}");
}

bool	View::removeJsonObject(const std::string& file_path, const std::string& target_json) {
	std::string			json_data;
	std::stringstream	buffer;

	std::ifstream input(file_path);
	if (!input.is_open()) {
		return (false);
	}
	buffer << input.rdbuf();
	json_data = buffer.str();
	input.close();
	removeObjectByUserId(json_data, "\"user_id\"", extractKey(target_json, "\"user_id\""));
	std::ofstream output(file_path);
	if (!output.is_open()) {
		return (false);
	}
	output << json_data;
	output.close();
	return (true);
}

void View::removeObjectByUserId(std::string& json_str, const std::string targer_key, const std::string target_value) {
	size_t		pos, begin_pos, end_pos, comma_before, comma_after, pre_nonspace, next_nonspace;
	std::string	target_str, before, after;
	
	target_str = targer_key + ": " + target_value; 
	pos = 0;	
	while ((pos = json_str.find(target_str, pos)) != std::string::npos) {
		begin_pos = json_str.rfind("{", pos);
		end_pos = json_str.find("}", pos);
		if (begin_pos == std::string::npos || end_pos == std::string::npos)
			return ; 
		// 오브젝트 삭제
		before = json_str.substr(0, begin_pos);
		after = json_str.substr(end_pos + 1);
		json_str = before + after;
		// 쉼표 및 공백 관리
		comma_before = json_str.rfind(",", begin_pos);
		comma_after = json_str.find(",", end_pos);
		if (comma_before != std::string::npos) {
			pre_nonspace = comma_before;
			while (pre_nonspace > 0 && (json_str[pre_nonspace - 1] == ' ' || json_str[pre_nonspace - 1] == '\t')) {
				pre_nonspace--;
			}
			json_str.erase(pre_nonspace, comma_before - pre_nonspace + 1);
		}
		if (comma_after != std::string::npos) {
			next_nonspace = comma_after;
			while (next_nonspace < json_str.length() && (json_str[next_nonspace] == ' ' || json_str[next_nonspace] == '\t')) {
				next_nonspace++;
			}
			json_str.erase(next_nonspace, comma_after - next_nonspace + 1);
		}
		if (json_str == "[]") {
			json_str = "[]";
		}
	}
}

std::string View::extractKey(const std::string &json, const std::string key) {
	size_t		begin, end;

	begin = json.find(key);
	if (begin == std::string::npos) {
		return ("");
	}
	begin = json.find(":", begin);
	if (begin == std::string::npos) {
		return ("");
	}
	++begin; // ':' 다음 문자로 이동
	// 공백 제거
	while (begin < json.size() && std::isspace(json[begin])) {
		++begin;
	}
	// 숫자 추출
	end = begin;
	while (end < json.size() && std::isdigit(json[end])) {
		++end;
	}
	return (json.substr(begin, end - begin)); // 숫자 부분을 문자열로 반환
}