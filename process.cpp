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
	// group_id = NO_GROUP;
}

void Process::init_check_array()
{
	checked.resize(other_members.size(), false);
}

void Process::send_check_p1()
{
	send_atomic_broadcast_p1(Message(PRESENT_CHECK));
}

void Process::check_failure_p1()
{
	int checked_list_size = checked.size();
	for (int i = 0; i < checked_list_size; i++)
	{
		if (!checked[i])
		{
			print("Oh no! " + to_string(i) + "   " + to_string(get_process_id()));
		}
	}
}

void Process::send_atomic_broadcast_p1(Message m)
{
	print(to_string(process_id) + " sends atomic broadcast : " + m.get_content_str());

	if (m.get_content() == NEW_GROUP)
	{
		other_members.clear();
		
		// stored in the group_id_array
		Group* group = new Group(this, m.get_time_stamp() + BIG_DELTA);
		m.set_group_id(group->get_id());
		group->add_member(this, process_id);
	}

	int list_size = process_list.size();

	pthread_t thread_ids[list_size];

	ThreadArgs** thread_args_array = (ThreadArgs**) malloc(list_size * sizeof(ThreadArgs*));

	for (int i = 0; i < list_size; i++)
	{
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
	print(to_string(process_id) + " received atomic broadcast from " +
		to_string(sender->get_process_id()) + " : " + m.get_content_str());

	// sleep(1);

	switch (m.get_content())
	{
		case NEW_GROUP:
		{
			send_atomic_broadcast_p1(PRESENT_ADD);

			Group* group = group_id_table[m.get_group_id()];
			group->add_member(this, process_id);

			other_members.clear();
			other_members.push_back(sender->get_process_id());

			break;
		}
		case PRESENT_ADD:
		{
			// redo this ?
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

bool Process::operator==(const Process rhs)
{
	return process_id == rhs.get_process_id();
}
