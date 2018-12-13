// Periodic Broadcast Membership Protocol

#include <iostream>
#include <sys/time.h>
#include <unistd.h>
#include <vector>
#include <unordered_map>
#include <string>

#include <pthread.h>

#include "groupMembership.h"
#include "message.h"
#include "group.h"
#include "process.h"

vector<Process*> process_list;
int process_count = 0;

unordered_map<int, Group*> group_id_table;
int group_count = 0;

int atomic_messages_sent = 0;
int atomic_messages_received = 0;

pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct timeval TimeStamp;
TimeStamp start_time;

void print(string s)
{
	pthread_mutex_lock(&print_mutex);
	cout << s << endl;
	pthread_mutex_unlock(&print_mutex);
}

void start_timer()
{
	gettimeofday(&start_time, NULL);
}

double get_current_time()
{
	TimeStamp current_time;
	gettimeofday(&current_time, NULL);

	double time = (double)(current_time.tv_sec - start_time.tv_sec) * 1000 +
					(double)(current_time.tv_usec - start_time.tv_usec) / 1000;
	return time;
}

void make_some_processors_fail(double probability) {

	int list_size = process_list.size();
	for (int i = 0; i < list_size; i++)
	{
		double r = (double)rand() / RAND_MAX;
		if (r < probability)
		{
			process_list[i]->fail();
		}
	}
}

void make_specific_processor_fail(int i) {
	// we don't have a way to go from id -> process 
	// Instead you have to specify the index in the process_list array

	process_list[i]->fail();
}

void atomic_broadcast_protocol(int n, int initiator_id)
{

	if (initiator_id >= n)
	{
		print("Initiator id must be smaller than n. Abort!!!");
		exit(1);
	}

	for (int i = 0; i < n; i++)
	{
		process_list.push_back(new Process(process_count++));
	}

	// P0 initiates the broadcast

	process_list[initiator_id]->send_atomic_broadcast_p1(Message(NEW_GROUP));

	print(to_string(get_current_time()));

	// sleep(FAIL_TIME);
	// make_specific_processor_fail(1);

	while (get_current_time() < 1000 * PROGRAM_EXEC_TIME) {
		sleep(FAIL_TIME);
		make_some_processors_fail(0.05);
	}
	
}

void print_all_data()
{

	// print("\n\n");

	// int list_size = process_list.size();
	// for (int i = 0; i < list_size; i++)
	// {
	// 	print("Process " + to_string(process_list[i].get_process_id()));
	// }

	print("\n");

	unordered_map<int, Group*>::iterator it;
	for (it = group_id_table.begin(); it != group_id_table.end(); it++)
	{
		it->second->print_members_list();
	}

	print("Total atomic messages sent: " + to_string(atomic_messages_sent));
	print("Total atomic messages received: " + to_string(atomic_messages_received));
}

int main(int argc, char *argv[])
{
	srand (386);

	if (argc < 2)
	{
		print("Missing args. Abort!!!");
		exit(1);
	}

	start_timer();
	
	atomic_broadcast_protocol(stoi(argv[1]), 0);

	sleep(PROGRAM_EXEC_TIME);

	double duration = get_current_time();
	print("Time elapsed is " + to_string(duration) + " milliseconds.");

	print_all_data();

	return 0;
}
