#include <map>
#include <string>

class Config {
public:
	Config();
	~Config();

	Config(const std::string filename);
	bool getConfig();
	std::string get(const std::string &section, const std::string &key) const;
	std::string trimInput(const std::string &str) const;

private:
	std::string filename;
	std::map<std::string, std::map<std::string, std::string> > settings;
};