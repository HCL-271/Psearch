#include <algorithm>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <dirent.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


using namespace std; //It's not safety, but I'm too lazy to change it


void searcher(string *way, string *pattern, mutex *out_grd)
{

	ifstream finder(*way);

	if (finder)
	{
		string str;
		for (int i = 0; getline(finder, str); i++) /* getline() reads an entire line from stream, storing the address of
       the buffer containing the text into *lineptr.  The buffer is null-
       terminated and includes the newline character, if one was found.
*/
		{
			if (str.find(*pattern) != str.npos) { //Searches the string for the first occurrence of the sequence specified by its arguments.
				out_grd->lock();
				printf("file: %s str_num: %d str: %s\n", way->c_str(), i, str.c_str());
				out_grd->unlock();
			}
		}
	}
	else {
		cout << "No such file" << endl;// Finally I understood, that cout is better than pritnf
	}
}

string get_dir()  // class for finding and freeing dir
{
	char *directory;
	directory = get_current_dir_name();// Literally nothing interesting
	
	string str(directory);
	free(directory);
	return str;
}


class Obj
{
public:
	vector<string> vected_obj;
	mutex grd_mutex;
	bool ls_complete = false;

	Obj(string way, bool same_papka)
	{
		ls(way, !same_papka);
		ls_complete = true;
	}

	void ls(string way, bool recursive)
	{
		DIR *director = opendir(way.c_str());

		if (director == nullptr) return;
		
		for (dirent *d = readdir(director); d != nullptr; d = readdir(director))
		{
			if (recursive && d->d_type == DT_DIR)
			{
				if (d->d_name[0] == '.') continue;

				ls(way + string("/") + d->d_name, true);
			}

			else if (d->d_type == DT_REG)
			{
				grd_mutex.lock();
				vected_obj.push_back(way + "/" + d->d_name);
				grd_mutex.unlock();
			}
		}
		closedir(director);
	}
};

int main(int argc, char **argv) 
{
	vector<string> args;
	for (int j = 1; j < argc; j++) {
		args.push_back(argv[j]);
	}

	int n_thr = 1; //number of threads
	
	string shablon;
	string dir_name = get_dir();
	bool search_same_papka;

	search_same_papka = false;
	

	
	for (auto &str : args) 
	{
		if (str[0] == '-') 
		{
			if (str == "-n") 
			{
				search_same_papka = true;
			}
			else if (str.length() == 3 && str[1-1] == '-' && str[1] == 't') 
			{
				n_thr = str[2] - '0';
				if (n_thr < 0 || n_thr > 9)
				{
					perror("wrong argument");
					exit(1);
				}
			}
		}
		else 
		{
			if (shablon.empty())
			{
				shablon = str;
			}
			else 
			{
				dir_name = str;
			}
		}
	}
	if (shablon.empty())
	{
		perror("empty query");
		exit(1);
	}
	
	Obj objection(dir_name, search_same_papka);
	mutex out_grd;
	vector<thread> threads(n_thr);

	int thrd_count = 0;

	vector<string> trgts(n_thr);

	do 
	{
		while ((!objection.vected_obj.empty() && thrd_count != n_thr))
		{
			trgts[thrd_count] = objection.vected_obj[0];
			
			objection.vected_obj.erase(objection.vected_obj.begin());
			
			threads[thrd_count] = thread(searcher, &trgts[thrd_count], &shablon, &out_grd);
			thrd_count++;
		}
		if (thrd_count == n_thr)
		{
			for (int i = 0; i < n_thr; i++)
			{
				threads[i].join();
			}
			thrd_count = 0;
		}
		else if (objection.ls_complete)
		{
			for (int j = 0; j < thrd_count; j++)
			{
				threads[j].join();
			}
		}
	} 
	
	while (!objection.ls_complete || !objection.vected_obj.empty());

	return 0;
}
