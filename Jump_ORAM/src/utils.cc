#include "utils.h"

/* class Log */

Log::Log(std::string name) : log_name(name)
{
}

void Log::start()
{
	start_point = std::chrono::high_resolution_clock::now();
}

unsigned long int Log::end()
{
	auto end_point = std::chrono::high_resolution_clock::now();
	unsigned long int time = std::chrono::duration_cast<std::chrono::nanoseconds>(
			end_point - start_point).count();
	exp_logs.push_back(time);
	return exp_logs.back();
}

void Log::write_to_file()
{
	FILE *log_file = fopen(log_name.c_str(), "a");
	if (!log_file) {
		printf("[Log] Can't open file %s\n", log_name.c_str());
		exit(1);
	}
	for (int i = 0; i < exp_logs.size(); i++)
		fprintf(log_file, "%lu ", exp_logs[i]);
	fprintf(log_file, "\n");
	fclose(log_file);
}
