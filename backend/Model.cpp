#include "Model.hpp"

/* ************************************************************************** */
/* --------------------------------	 OCCF	--------------------------------- */
/* ************************************************************************** */

/* public */
Model::Model() {}

Model::~Model() {}

/* ************************************************************************** */
/* ------------------------------	RESPONSE  ------------------------------- */
/* ************************************************************************** */

/* public */
void	Model::createResponseHeaders(HttpMsg &request, HttpMsg &response, std::map<std::string, std::string> headers, std::string status) {
	response.createResponseStartline(request, response, status);
	response.createHeader(request, response, headers);
}

/* ************************************************************************** */
/* -----------------------------	AUTOINDEX	----------------------------- */
/* ************************************************************************** */

/* public */
// t_vec_vec	Model::getDirVec(const std::string &dir_path) {
// 	DIR							*dir_ptr;
// 	struct dirent				*file;
// 	std::string					json;
// 	t_vec_vec					values;
// 	std::vector<std::string>	value;

// 	if (dir_path.empty()) {
// 		return (values);
// 	}
// 	dir_ptr = NULL;
// 	if (openDirectory(dir_path, dir_ptr)){
// 		return (values);
// 	}
// 	else {
// 		while ((file = readdir(dir_ptr)) != NULL)
// 			getfFileInfo(file, dir_path, values);
// 	}
// 	closedir(dir_ptr);
// 	return (values);
// }

t_vec_vec Model::getDirVec(const std::string &dir_path) {
	DIR				*dir_ptr;
	struct dirent	*file;
	t_vec_vec		values;

	if (dir_path.empty()) {
		return values;
	}
	dir_ptr = NULL;
	if (openDirectory(dir_path, dir_ptr) || dir_ptr == NULL) {
		return values;
	}
	while ((file = readdir(dir_ptr)) != NULL) {
		getFileInfo(file, dir_path, values);
	}
	closedir(dir_ptr);
	return values;
}

int	Model::openDirectory(const std::string &path, DIR *&dir_ptr) {
	dir_ptr = opendir(path.c_str());
	if (dir_ptr == NULL)
		return (1);
	return (0);
}

// void	Model::getFileInfo(struct dirent *file, const std::string &dir_path, t_vec_vec &values) {
// 	char						time[20];
// 	std::string					file_path, dir_name;
// 	std::vector<std::string>	value;
// 	std::stringstream			size_stream, date_stream;
// 	struct stat					file_stat;
// 	struct tm					*timeinfo;

// 	file_path = dir_path + "/" + file->d_name;
// 	if (stat(file_path.c_str(), &file_stat) == -1)
// 		return;
// 	// 파일
// 	value.clear();
// 	date_stream.str("");
// 	size_stream.str("");
// 	date_stream.clear();
// 	size_stream.clear();
// 	if (S_ISREG(file_stat.st_mode)) {
// 		value.push_back(file->d_name);
// 	}
// 	// 디렉토리
// 	else if (S_ISDIR(file_stat.st_mode)) {
// 		dir_name = file->d_name;
// 		dir_name += "/";
// 		value.push_back(dir_name);
// 	}
// 	// timeinfo = localtime(&file_stat.st_mtime);
// 	// strftime(time, sizeof(time), "%Y-%m-%d %H:%M:%S", timeinfo);
// 	// date_stream << time;
// 	date_stream << file_stat.st_mtime;
// 	value.push_back(date_stream.str());
// 	size_stream << file_stat.st_size;
// 	value.push_back(size_stream.str());
// 	values.push_back(value);
// }

void Model::getFileInfo(struct dirent *file, const std::string &dir_path, t_vec_vec &values) {
	std::string					file_path, dir_name; 
	struct stat					file_stat;
	std::vector<std::string>	value;
	std::stringstream			size_stream, date_stream;

	file_path = dir_path + "/" + file->d_name;
	if (stat(file_path.c_str(), &file_stat) == -1) {
		return;
	}
	if (S_ISREG(file_stat.st_mode)) {
		value.push_back(file->d_name);
	} else if (S_ISDIR(file_stat.st_mode)) {
		dir_name = file->d_name;
		dir_name += "/";
		value.push_back(dir_name);
	}
	date_stream << file_stat.st_mtime;
	value.push_back(date_stream.str());
	size_stream << file_stat.st_size;
	value.push_back(size_stream.str());
	values.push_back(value);
}