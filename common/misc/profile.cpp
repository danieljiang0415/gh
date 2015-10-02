#include "profile.h"

string get_ini_section_value(string& config_file_path, string& section, string& key)
{
	char store_buf[1024] = {'\0'};
	GetPrivateProfileStringA(section.c_str(), key.c_str(), "", store_buf, 1024, config_file_path.c_str());
	return string(store_buf);
}

void set_ini_section_value(string& config_file_path, string& section,string& key, string& value)
{
	WritePrivateProfileStringA(section.c_str(), key.c_str(), value.c_str(), config_file_path.c_str());
}

