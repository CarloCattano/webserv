#pragma once

#include <string>

class Cgi {
public:
	Cgi();
	~Cgi();
	void run(const std::string &path, const std::string &filename);

private:
	char **_env;

	std::string _cgi;

	Cgi(const Cgi &src);
	Cgi &operator=(const Cgi &src);

	void _runPython(const std::string &path, const std::string &filename, const std::string &query);
};
