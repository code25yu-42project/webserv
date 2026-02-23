#ifndef VIEW_HPP
# define VIEW_HPP

# include "Types.hpp"

class View {
	private:
		std::string	renderToJson(const std::string object, const std::vector<std::string> &keys, const  t_vec_vec &values);
	public:
		/* OCCF */
					View();
					~View();
		std::string	dirToJson(const t_vec_vec &values);
		std::string	redirectToJson (std::string err_page_uri);
		bool		removeJsonObject(const std::string& file_path, const std::string& target_json);	
		void		removeObjectByUserId(std::string& json_str, const std::string targer_key, const std::string target_value);
		std::string	extractKey(const std::string &json, const std::string key);
	};

#endif