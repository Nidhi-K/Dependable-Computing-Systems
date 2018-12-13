// Periodic Broadcast Membership Protocol

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

Group::Group (Process* creator, double time_stamp)
{
	this->creator = creator;
	creation_time = time_stamp;

	group_id = group_count++;
	group_id_table[group_id] = this;

	active = true;

	pthread_create(&check_failure_thread, NULL, check_failure_helper, (void*)this);
	// pthread_join(check_failure_thread, NULL);
}

// static
Group* Group::look_up_group(int group_id)
{
	return group_id_table[group_id];
}

// static
void* Group::check_failure_helper(void* group)
{
	static_cast<Group*>(group)->check_failure();

	pthread_exit(NULL);
}

void Group::check_failure()
{
	while (active) {

		sleep(PI);

		print("\nTime to check failure in group " + to_string(group_id));

		int list_size = members.size();
		for (int i = 0; i < list_size; i++)
		{
			members[i]->init_check_array();
		}

		for (int i = 0; i < list_size; i++)
		{
			members[i]->send_check_p1();
		}

		// sleep(BIG_DELTA);

		int idx_to_create = -1;

		int failed = 0;

		for (int i = 0; i < list_size; i++)
		{
			int result = members[i]->check_failure_p1();

			if (result < 0) {
				continue;
			}

			if (idx_to_create < 0) {
				idx_to_create = i;
			}

			failed += result;
		}

		if (failed != 0) {
			print(to_string(failed) + " processes detected a failure.");

			// choose a member to create the next group
			process_list[idx_to_create]->send_atomic_broadcast_p1(Message(NEW_GROUP));
		}
	}

	// pthread_join(check_failure_thread, NULL);
	// pthread_create(&check_failure_thread, NULL, check_failure_helper, (void*)this);
}

int Group::get_id()
{
	return group_id;
}

vector<Process*> Group::print_members_list()
{
	print("Group: " + to_string(group_id));

	int members_size = members.size();
	for (int i = 0; i < members_size; i++)
	{
			print("Process " + to_string(members[i]->get_process_id()));
	}
	print("");
}

void Group::add_member(Process* p)
{
	print("Process " + to_string(p->get_process_id()) + " is joining group " + to_string(group_id));
	members.push_back(p);
}

void Group::remove_member(Process* p)
{
	print("Process " + to_string(p->get_process_id()) + " is leaving group " + to_string(group_id));
	members.erase(find(members.begin(), members.end(), p));

	// If the last member just left, delete the group?

	int active_members = 0;

	int members_size = members.size();
	for (int i = 0; i < members_size; i++) {
		if (members[i]->is_active()) {
			active_members++;
		}
	}

	if (active_members <= 0)
	{
		print("All members have left group " + to_string(group_id));
		active = false;
		pthread_join(check_failure_thread, NULL);

		for (int i = 0; i < members_size; i++) {
			members[i]->leave_group();
		}
	}
}
