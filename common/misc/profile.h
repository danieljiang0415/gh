#pragma once

#include <windows.h>

#include <string>

using namespace std;

string get_ini_section_value(string& config_file_path, string& section, string& key);

void set_ini_section_value(string& config_file_path, string& section,string& key, string& value);