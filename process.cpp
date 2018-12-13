// Periodic Broadcast Protocol

#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <unistd.h>

#include <pthread.h>

#include "groupMembership.h"
#include "message.h"
#include "group.h"
#include "process.h"


Process::Process(int id)
{
	process_id = id;
	active = true;
	group_id = NO_GROUP;
}

void Process::init_check_array()
{
	checked.resize(other_members.size());

	int checked_list_size = checked.size();
	for (int i = 0; i < checked_list_size; i++){
		checked[i] = false;
	}
}

void Process::send_check_p1()
{
	send_atomic_broadcast_p1(Message(PRESENT_CHECK, group_id));
}

int Process::check_failure_p1()
{
	if (!active){
		return -1;
	}

	bool failed = false;
	int checked_list_size = checked.size();
	for (int i = 0; i < checked_list_size; i++)
	{
		if (!checked[i])
		{
			// print("checked list size " + to_string(checked_list_size) + " i is " + to_string(i));

			print("Process " + to_string(process_id) + " sees that process " +
						to_string(other_members[i]) + " has failed!");
			failed = true;
			break;
		}
	}

	return failed ? 1 : 0;

	// // If everything is good you can leave now
	// if (!failed)
	// {
	// 	return;
	// }

	// // Find out if you are the leader
	// // This will minimize conflicts
	// int list_size = other_members.size();
	// for (int i = 0; i < list_size; i++)
	// {
	// 	if (other_members[i] < process_id)
	// 	{
	// 		return;
	// 	}
	// }

	// // If you have reached this point you are the leader and
	// // it is your responsibility to create a new group
	// print("Process " + to_string(process_id) + " will create a new group!");

	// send_atomic_broadcast_p1(Message(NEW_GROUP));
}

// private
void Process::leave_group()
{
	if (group_id >= 0)
	{
		group_id_table[this->group_id]->remove_member(this);
	}

	group_id = NO_GROUP;
}

void Process::fail()
{
	print("\nProcess " + to_string(process_id) + " failed at " + to_string(get_current_time()) + "\n");
	active = false;
}

bool Process::is_active() {
	return active;
}

void Process::send_atomic_broadcast_p1(Message m)
{
	if (!active)
	{
		return;
	}

	print(to_string(process_id) + " sends atomic broadcast : " + m.get_content_str());

	atomic_messages_sent++;

	if (m.get_content() == NEW_GROUP)
	{
		other_members.clear();
		
		// stored in the group_id_array
		Group* group = new Group(this, m.get_time_stamp() + BIG_DELTA);
		m.set_group_id(group->get_id());

		// if (group_id >= 0)
		// {
		// 	// Leave your current group
		// 	group_id_table[this->group_id]->remove_member(this);
		// }
		
		leave_group();

		group->add_member(this);
		this->group_id = group->get_id();
	} 

	int list_size = process_list.size();

	pthread_t thread_ids[list_size];

	ThreadArgs** thread_args_array = (ThreadArgs**) malloc(list_size * sizeof(ThreadArgs*));

	for (int i = 0; i < list_size; i++)
	{
		// Don't send it to yourself
		if (process_list[i]->get_process_id() == process_id)
		{
			continue;
		}

		thread_args_array[i] = (ThreadArgs*) malloc(sizeof(ThreadArgs));

		thread_args_array[i]->receiver = process_list[i];
		thread_args_array[i]->sender = this;
		thread_args_array[i]->message = m;
	}

	for (int i = 0; i < list_size; i++)
	{
		if (process_list[i]->get_process_id() == process_id)
		{
			continue;
		}

		pthread_create(&thread_ids[i], NULL, atomic_broadcast_thread_helper, (void*)thread_args_array[i]);
	} 

	for (int i = 0; i < list_size; i++)
	{
		if (process_list[i]->get_process_id() == process_id)
		{
			continue;
		}

		pthread_join(thread_ids[i], NULL);
		free(thread_args_array[i]);
	}

	free(thread_args_array);
}

// static
void* Process::atomic_broadcast_thread_helper(void* args)
{
	ThreadArgs* thread_args = (ThreadArgs*)args;

	static_cast<Process*>(thread_args->receiver)->receive_atomic_broadcast_p1(thread_args->sender, thread_args->message);

	pthread_exit(NULL);
}

// void receive_datagram_p1(Process sender, Message m)
// {
// 	print(to_string(process_id) + " received datagram from " +
// 		to_string(sender.get_process_id()) + " : " + m.get_content_str());
// }

void Process::receive_atomic_broadcast_p1(Process* sender, Message m)
{
	if (!active)
	{
		return;
	}

	print(to_string(process_id) + " received atomic broadcast from " +
		to_string(sender->get_process_id()) + " : " + m.get_content_str());

	atomic_messages_received++;

	switch (m.get_content())
	{
		case NEW_GROUP:
		{
			// It should be guaranteed that everyone finishes receives this before 
			// receiving present checks but is it?
			other_members.clear();
			other_members.push_back(sender->get_process_id());

			leave_group();

			Group* new_group = group_id_table[m.get_group_id()];
			new_group->add_member(this);
			group_id = m.get_group_id();


			// sleep(BIG_DELTA);

			send_atomic_broadcast_p1(Message(PRESENT_ADD, group_id));
			
			// print(to_string(process_id) + " now size is " + to_string(other_members.size()));

			break;
		}
		case PRESENT_ADD:
		{
			// Make sure this message applies to your group
			if (m.get_group_id() != group_id)
			{
				break;
			}

			other_members.push_back(sender->get_process_id());

			break;
		}
		case PRESENT_CHECK:
		{
			int sender_id = sender->get_process_id();
			int sender_idx = find(other_members.begin(), other_members.end(), sender_id) - other_members.begin();

			checked[sender_idx] = true;

			break;
		}
		default:
		{
			print("Unknown message.");
		}
	}
}

int Process::get_process_id() const
{
	return process_id;
}

int Process::get_group_id()
{
	return group_id;
}

bool Process::operator==(const Process rhs)
{
	return process_id == rhs.get_process_id();
}
