#include <iostream>
#include <cstdlib>
#include <sys/time.h>
#include <unistd.h>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <string>

#include <pthread.h>

#define NO_GROUP -1
#define BIG_DELTA 1000

using namespace std;

class Process;
class Group;

vector<Process*> process_list;
int process_count = 0;

unordered_map<int, Group*> group_id_table;
int group_count = 0;

pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;

enum MessageContent {NEW_GROUP, PRESENT};

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

class Group
{
	private:

	double creation_time;
	Process* creator;

	int id;
	vector<Process*> members;

	public:

	static Group* look_up_group(int group_id)
	{
		return group_id_table[group_id];
	}

	Group (Process* creator, double time_stamp)
	{
		this->creator = creator;
		creation_time = time_stamp;

		id = group_count++;
		group_id_table[id] = this;

		cout << "New group " << id << "   " << group_id_table[id] << endl;
	}

	double get_id() {
		return id;
	}

	int get_group_size() {
		return members.size();
	}

	vector<Process*> get_members_list() {
		return members;
	}

	void add_member(Process* p, int p_id)
	{
		members.push_back(p);

		// p_id is only for printing 
		print("Process " + to_string(p_id) + " is joining group " + to_string(id));
	}

	void remove_member(Process* p)
	{
		members.erase(find(members.begin(), members.end(), p));
	}
};

class Message
{
	private:
	MessageContent content;
	double time_stamp;
	int group_id;

	public:

	Message(MessageContent c)
	{
		time_stamp = get_current_time();
		content = c;
	}

	Message(MessageContent content, int more_information)
	{
		time_stamp = get_current_time();
		this->content = content;
	}

	void set_group_id(int id)
	{
		group_id = id;
	}

	int get_group_id()
	{
		return group_id;
	}

	double get_time_stamp()
	{
		return time_stamp;
	}

	int get_content()
	{
		return content;
	}

	string get_content_str()
	{
		switch (content)
		{
			case NEW_GROUP: return "NEW_GROUP " + to_string(time_stamp);
			case PRESENT: return "PRESENT " + to_string(time_stamp);
		}

	return "ERROR";
	}
};

class Process
{
	private:

	// int group_id;
	int process_id;

	typedef struct thread_args
	{
		Process* receiver;
		Process* sender;
		Message message;

	} ThreadArgs;

	public:

	Process(int id)
	{
		process_id = id;
		// group_id = NO_GROUP;
	}

	void send_atomic_broadcast_p1(Message m)
	{
		print(to_string(process_id) + " sends atomic broadcast : " + m.get_content_str());

		if (m.get_content() == NEW_GROUP)
		{
			// stored in the group_id_array
			Group* group = new Group(this, m.get_time_stamp() + BIG_DELTA);
			m.set_group_id(group->get_id());
			group->add_member(this, process_id);
		}

		int list_size = process_list.size();

		pthread_t thread_ids[list_size];
		// vector<pthread_t> thread_ids(list_size);

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

	static void* atomic_broadcast_thread_helper(void* args)
	{
	
		ThreadArgs* thread_args = (ThreadArgs*)args;
		static_cast<Process*>(thread_args->receiver)->receive_atomic_broadcast_p1(*thread_args->sender, thread_args->message);

		pthread_exit(NULL);
	}

	// void receive_datagram_p1(Process sender, Message m)
	// {
	// 	print(to_string(process_id) + " received datagram from " +
	// 		to_string(sender.get_process_id()) + " : " + m.get_content_str());
	// }

	void receive_atomic_broadcast_p1(Process sender, Message m)
	{
		print(to_string(process_id) + " received atomic broadcast from " +
			to_string(sender.get_process_id()) + " : " + m.get_content_str());

		// sleep(1);

		switch (m.get_content())
		{
			case NEW_GROUP:
			{
				send_atomic_broadcast_p1(PRESENT);

				Group* group = group_id_table[m.get_group_id()];
				group->add_member(this, process_id);

				break;
			}
			case PRESENT:
			{

				break;
			}
			default:
			{
				print("Unknown message.");
			}
		}
	}

	int get_process_id() const
	{
		return process_id;
	}

	bool operator==(const Process rhs)
	{
		return process_id == rhs.get_process_id();
	}
};

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

	// 0 initiates the broadcast
	process_list[initiator_id]->send_atomic_broadcast_p1(Message(NEW_GROUP));
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
		print("Group: " + to_string(it->first));

		vector<Process*> members = (it->second)->get_members_list();
		int members_size = members.size();
		for (int i = 0; i < members_size; i++)
		{
			print("Process " + to_string(process_list[i]->get_process_id()));
		}
	}

}

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		print("Missing args. Abort!!!");
		exit(1);
	}

	start_timer();
	
	atomic_broadcast_protocol(stoi(argv[1]), 0);

	double duration = get_current_time();
	print("Time elapsed is " + to_string(duration) + " milliseconds.");

	print_all_data();

	return 0;
}