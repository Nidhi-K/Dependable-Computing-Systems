#include <iostream>
#include <unistd.h>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <string>

#include <pthread.h>

#include "groupMembership.h"
#include "message.h"
#include "group.h"
#include "process.h"

// static
Group* Group::look_up_group(int group_id)
{
	return group_id_table[group_id];
}

Group::Group (Process* creator, double time_stamp)
{
	this->creator = creator;
	creation_time = time_stamp;

	id = group_count++;
	group_id_table[id] = this;

	pthread_create(&check_failure_thread, NULL, check_failure_helper, (void*)this);
}

// static
void* Group::check_failure_helper(void* group)
{
	static_cast<Group*>(group)->check_failure();

	pthread_exit(NULL);
}

void Group::check_failure()
{
	sleep(PI);

	print("Time to check failure");

	int list_size = members.size();
	for (int i = 0; i < list_size; i++)
	{
		members[i]->init_check_array();
	}

	for (int i = 0; i < list_size; i++)
	{
		members[i]->send_check_p1();
	}

	sleep(BIG_DELTA);

	for (int i = 0; i < list_size; i++)
	{
		members[i]->check_failure_p1();
	}

	pthread_join(check_failure_thread, NULL);
	pthread_create(&check_failure_thread, NULL, check_failure_helper, (void*)this);
}

double Group::get_id()
{
	return id;
}

int Group::get_group_size()
{
	return members.size();
}

vector<Process*> Group::get_members_list()
{
	return members;
}

void Group::add_member(Process* p, int p_id)
{
	members.push_back(p);

	// p_id is only for printing 
	print("Process " + to_string(p_id) + " is joining group " + to_string(id));
}

void Group::remove_member(Process* p)
{
	members.erase(find(members.begin(), members.end(), p));
}