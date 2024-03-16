#pragma once

#include <string>

class Cgi {
  public:
	Cgi();
	~Cgi();
	std::string run() const;

  private:
	std::string _cgi;

	Cgi(const Cgi &src);
	Cgi &operator=(const Cgi &src);
};
