#include <iostream>
#include <stdio.h>
#include <string>
#include "string_manip.hpp"
#include <fstream>
#include <cstdlib>
#include <vector>

#define CRC_HAIL "CXHAIL"
// #define CRC_CTRL "CRC_CTRL"

#define IFPRINT if (false) cout

using namespace std;

bool set_file_compile(string fn, bool ns, vector<bool>& compile_status, vector<string> files);
bool compile_file(string fn, string compiler, string version, bool print_command, string outpt_dir="");
string remove_ext(string s);
string get_directory(string s);
string remove_directory(string s);

typedef struct{
	vector<string> include_files;
	string command = "";
	bool command_found = false;
	vector<bool> compile_status;
	string compiler = "g++";
	string version = "-std=c++11";
	bool changed_name = false;
	bool silence = false; //If true, silences all CRC output (does not include target output or error messages)
	bool show_settings = false; //show settings such as compiler, version of C/C++ to use, etc.
	bool list_dependencies = false; //List all dependencies and if they are to be compiled
	bool list_commands = false; //List the compile and run commands as they  are executed
	bool status_reports = false; //Report when compiling, running begins and ends
}action_struct;

bool scan_file(string filename, action_struct& action, bool print_error);

int main(int argc, char** argv){

	action_struct actions;

	//Check to ensure arguments were given. If not break
	if (argc == 1){
		cout << "ERROR: No file specified. ['" << argv[0] <<  " -help' for instructions]" << endl;
		return -1;
	}

	//Check to see if help screen was requested. If so break
	if (to_uppercase(string(argv[1])) == "-HELP"){
		cout << "HELP!" << endl;
		return -1;
	}

	//Read the filename from the command line
	string filename(argv[1]);

	//Append '.cpp' if no extention is given - implement std::filesystem check
	if (filename == remove_ext(filename)){
		if (ifstream(filename+".cpp")){
			filename = filename + ".cpp";
			actions.changed_name = true;
		}else if(ifstream(filename+".c")){
			filename = filename + ".c";
			actions.changed_name = true;
		}else if(ifstream(filename+".cc")){
			filename = filename + ".cc";
			actions.changed_name = true;
		}else if(ifstream(filename+".cxx")){
			filename = filename + ".cxx";
			actions.changed_name = true;
		}
	}

	//Set main file name
	string main_file = filename;
	string executable_name = remove_ext(main_file);

	if (!scan_file(main_file, actions, true)){
		cout << "ERROR: Failed to read file '" << main_file << "'" << endl;
		return -1;
	}

	//Remove duplicates
	for (int i = 0 ; i < actions.include_files.size() ; i++){
		for (int m = i+1 ; m < actions.include_files.size() ; m++){
			if (actions.include_files[i] == actions.include_files[m]){
				actions.include_files.erase(actions.include_files.begin() + m);
				actions.compile_status.erase(actions.compile_status.begin() + m);
				// cout << "Redundant dependency '" << actions.include_files[m] << "' removed at indecies " << i << " and " << m << endl;
				m--;
			}
		}
	}

	//Alert user to interpreted name if extension not explicitly set
	if (actions.changed_name && !actions.silence){
		cout << "MAIN FILE: " << filename << endl;
	}

	//Print commands
	// if (!command_found){
	// 	for (int i = 0 ; i < include_files.size() ; i++){
	// 		IFPRINT << include_files[i] << endl;
	// 	}
	// }else{
	// 	IFPRINT << "EXEC: " << command << endl;
	// }

	//Show settings if requested
	if (!actions.silence && actions.show_settings){
		cout << "COMPILER: " << actions.compiler << endl << "VERSION: " << actions.version << endl;
	}

	//List dependencies if requested
	if (actions.list_dependencies && !actions.silence){
		if (actions.include_files.size() <= 0){
			cout << "NO DEPENDENCIES" << endl;
		}else{
			cout << "DEPENDENCIES: " << endl;
			for (int i = 0 ; i < actions.include_files.size() ; i++){
				cout << "\tFILE: " << actions.include_files[i] << "\tCOMPILE: " << bool_to_str(actions.compile_status[i]) << endl;
			}
		}
	}

	if (!actions.command_found){

		//Compile files
		if (actions.status_reports  && !actions.silence) cout << "Compiling Dependencies...";
		if (actions.status_reports && actions.list_commands  && !actions.silence) cout << endl;
		for (int i = 0 ; i < actions.include_files.size() ; i++){
			if (!actions.compile_status[i]){ //Skip files marked not to compile
				continue;
			}
			if (!compile_file(remove_ext(actions.include_files[i]) + ".cpp", actions.compiler, actions.version, (actions.list_commands  && !actions.silence), get_directory(actions.include_files[i]))){ //Compile file
				cout << "ERROR: Failed to compile file '" << actions.include_files[i] << "'" << endl;
				return -1;
			}
		}
		if (actions.status_reports && !actions.list_commands  && !actions.silence) cout << '\t';
		if ( actions.status_reports  && !actions.silence) cout << "Compile successful." << endl;


		//Compile main and link
		if (actions.status_reports  && !actions.silence) cout << "Compiling Main and Linking...";
		if (actions.status_reports && actions.list_commands  && !actions.silence) cout << endl;
		string cmd = actions.compiler + " -o " + executable_name + ' ' + main_file + ' ' + actions.version;
		for (int i = 0 ; i < actions.include_files.size() ; i++){
			cmd = cmd + ' ' + remove_ext(actions.include_files[i]) + ".o";
		}
		if (actions.list_commands  && !actions.silence) cout << cmd << endl;
		if (system(cmd.c_str()) < 0){
			cout << "ERROR: CRC Failed on compile-main + link." << endl;
			return -1;
		}
		if (actions.status_reports && !actions.list_commands  && !actions.silence) cout << '\t';
		if (actions.status_reports  && !actions.silence) cout << "Compiling Main and Linking successful." << endl;

		//Run program
		if (!actions.silence){
			cout << "Compile and Link successful - executing '" << executable_name << "'" << endl;
			cout << "------------------------------------------------------------------" << endl;
		}
		system(executable_name.c_str());

	}else{

	}


	return 0;
}

/*
Set the compile status of a file's flag

fn - name of file whose flag is to be set
ns - new status for flag (boolean)
compile_status - vector of compile statuses
files - vector of file names

Returns true if successful
*/
bool set_file_compile(string fn, bool ns, vector<bool>& compile_status, vector<string> files){

	if (files.size() != compile_status.size() ){
		return false;
	}

	for (int i = 0 ; i < files.size() ; i++){
		if (files[i] == fn){
			compile_status[i] = ns;
			return true;
		}
	}

	return false;
}

bool compile_file(string fn, string compiler, string version, bool print_command, string outpt_dir){
	string cmd = compiler + " -c " + fn + ' ' + version;

	if (outpt_dir != ""){
		cmd = cmd + " -o " + outpt_dir + "/" + remove_ext(remove_directory(fn)) + ".o";
	}

	int status = system(cmd.c_str());
	if (print_command) cout << cmd << endl;
	return (status == 0);
}

string remove_ext(string s){

	for (int i = 0 ; i < s.length() ; i++){
		if (s[i] == '.'){
			return s.substr(0, i);
		}
	}

	return s;
}

bool scan_file(string filename, action_struct& actions, bool print_errors){

	//Are second order dependencies allowed (dependencies of dependencies)?
	bool SO_dependencies = true;

	//Mark where to begin recursive file scanning
	int start_scan_idx = actions.include_files.size();

	//Open the main file
	ifstream file;
	file.open(filename.c_str());
	if (!file.is_open()){
		return false;
	}

	// cout << "Filename: " << filename << endl;

	string directory = get_directory(filename);

	//Loop through file and search for include statements or directives to CRC
	string s, temp_s;
	vector<string> words;
	int line_num = 1;
	while(file.good()){
		getline(file, s);
		words = parse(s, " ");
		if (words.size() >= 2 && words[0] == "#include" && words[1][0] == '\"'){ //Look for include statements
			// cout << "Do we have a dependency...";
			temp_s = cat_tokens(words, 1, " ");
			if (temp_s.length() > 2){
				if (directory.length() > 0){
					temp_s = directory + '/' + temp_s.substr(1, temp_s.length()-2); //Remove quotes
				}else{
					temp_s = temp_s.substr(1, temp_s.length()-2); //Remove quotes
				}
				actions.include_files.push_back(temp_s);
				actions.compile_status.push_back(true);
				// cout << "Dependency '" << temp_s << "' added" << endl;
			}else{
				if (print_errors) cout << "ERROR: Failed to interpret file name on line " << line_num << endl;
			}
		}else if(words.size() >= 1 && ((words[0] == "//" && words[1] == string(CRC_HAIL) ) || (words[0] == ("//"+string(CRC_HAIL))))){ //CRC Control adjusted

			int start_word = ( words[0] == "//" )? 2 : 1; //Determine where to start when interpreting command

			for (int i = start_word ; i < words.size() ; i++){
				if (words[i] == "NCOMPILE"){
					for (int j = i+1 ; j < words.size() ; j++){ //Read all command arguments
						if (words[i+1] == "ALL"){ //Compile no files
							for (int h = 0 ; h < actions.compile_status.size() ; h++){ //Set all compile flags to false
								actions.compile_status[h] = false;
							}
						}else{
							if (!set_file_compile(words[j], false, actions.compile_status, actions.include_files)){
								cout << "SOFTWARE ERROR: Failed to set compile flag to false." << endl;
							}
						}
					}
				}else if(words[i] == "COMPILER"){ //Set new compiler
					if (words.size() < i+2){ //Ensure more words available
						cout << "ERROR: Failed to interpret CRC Control on line " << line_num << endl;
					}
					actions.compiler = words[i+1]; //Set new compiler
					i++;
				}else if(words[i] == "SILENCE"){
					actions.silence = true;
				}else if(words[i] == "SHOW_SETTINGS"){
					actions.show_settings = true;
				}else if(words[i] == "SHOW_DEPENDENCIES"){
					actions.list_dependencies = true;
				}else if(words[i] == "STATUS_REPORTS"){
					actions.status_reports = true;
				}else if(words[i] == "LIST_COMMANDS"){
					actions.list_commands = true;
				}else if(words[i] == "FIRST_ORDER_DEP"){
					SO_dependencies = false;
				}else if(words[i] == "COMMAND"){
					if (words.size()-1 <= i) break; //Ensure more words exist
					actions.command = cat_tokens(words, i+1, " ");
					actions.command_found = true;
				}
			}
		}
		line_num++;
	}

	//Remove duplicates
	for (int i = 0 ; i < actions.include_files.size() ; i++){
		for (int m = i+1 ; m < actions.include_files.size() ; m++){
			if (actions.include_files[i] == actions.include_files[m]){
				actions.include_files.erase(actions.include_files.begin() + m);
				actions.compile_status.erase(actions.compile_status.begin() + m);
				// cout << "Redundant dependency '" << actions.include_files[m] << "' removed at indecies " << i << " and " << m << endl;
			}
		}
	}

	//Create a temporary actions struct to gather 2nd order dependencies
	action_struct temp_actions = actions;

	//Find includes from dependencies
	for (int i = start_scan_idx ; i < actions.include_files.size() ; i++){

		if (scan_file(remove_ext(actions.include_files[i]) + ".hpp", temp_actions, print_errors)){

		}else if (scan_file(remove_ext(actions.include_files[i]) + ".h", temp_actions, print_errors)){

		}else if (scan_file(remove_ext(actions.include_files[i]) + ".hh", temp_actions, print_errors)){

		}else if (scan_file(remove_ext(actions.include_files[i]) + ".hxx", temp_actions, print_errors)){

		}

		if (scan_file(remove_ext(actions.include_files[i]) + ".cpp", temp_actions, print_errors)){

		}else if (scan_file(remove_ext(actions.include_files[i]) + ".c", temp_actions, print_errors)){

		}else if (scan_file(remove_ext(actions.include_files[i]) + ".cc", temp_actions, print_errors)){

		}else if (scan_file(remove_ext(actions.include_files[i]) + ".cxx", temp_actions, print_errors)){

		}

	}

	//If 2nd order dependencies disallowed, at time of transfer of include files, set all compile statuses to false
	if (SO_dependencies){
		for (int i = 0 ; i < temp_actions.include_files.size() ; i++){
			actions.include_files.push_back(temp_actions.include_files[i]);
			actions.compile_status.push_back(temp_actions.compile_status[i]);
		}
	}else{
		for (int i = 0 ; i < temp_actions.include_files.size() ; i++){
			actions.include_files.push_back(temp_actions.include_files[i]);
			actions.compile_status.push_back(false);
		}
	}
	

	//Close file
	file.close();

	return true;
}

string get_directory(string s){

	string directory = "";
	int last_dir_idx = 0;

	for (int i = 0 ; i < s.length() ; i++){
		if (s[i] == '/'){
			last_dir_idx = i;
		}
	}

	directory = s.substr(0, last_dir_idx);
	return directory;
}

string remove_directory(string s){
	return s.substr(get_directory(s).length()+1);
}
