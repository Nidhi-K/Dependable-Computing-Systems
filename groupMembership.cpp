#include <iostream>
#include <cstdlib>
#include <sys/time.h>
#include <unistd.h>
#include <vector>
#include <string>

#include <pthread.h>

#define NO_GROUP -1

using namespace std;

pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;

void print(string s)
{
	pthread_mutex_lock(&print_mutex);
	cout << s << endl;
	pthread_mutex_unlock(&print_mutex);

}

typedef struct timeval TimeStamp;
TimeStamp start_time;

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

class Process;

enum MessageContent {NEW_GROUP, PRESENT};

class Message
{
	private:
	MessageContent content;
	double time_stamp;

	public:

	Message(MessageContent c)
	{
		time_stamp = get_current_time();
		content = c;
	}

	int get_content() {
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

vector<Process> process_list;
int process_count = 0;

void send_datagram(Process sender, Process receiver, Message m);
void send_atomic_broadcast(Message m);

class Process
{
	private:

	vector<Process> members;
	int group_id;
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
		group_id = NO_GROUP;
	}

	void send_datagram(Process receiver, Message m)
	{
		print(to_string(process_id) + " sending datagram to " +
				to_string(receiver.get_process_id()) +  " : " + m.get_content_str());

		receiver.receive_datagram_p1(*this, m);
	}

	void send_atomic_broadcast_p1(Message m)
	{

		print(to_string(process_id) + " sends atomic broadcast : " + m.get_content_str());

		int list_size = process_list.size();

		pthread_t thread_ids[list_size];

		ThreadArgs** thread_args_array = (ThreadArgs**) malloc(list_size * sizeof(ThreadArgs*));

		for (int i = 0; i < list_size; i++)
		{
			if (process_list[i].get_process_id() == process_id)
			{
				continue;
			}

			thread_args_array[i] = (ThreadArgs*) malloc(sizeof(ThreadArgs));

			thread_args_array[i]->receiver = &process_list[i];
			thread_args_array[i]->sender = this;
			thread_args_array[i]->message = m;
		}

		for (int i = 0; i < list_size; i++)
		{
			if (process_list[i].get_process_id() == process_id)
			{
				continue;
			}

			pthread_create(&thread_ids[i], NULL, atomic_broadcast_thread_helper, (void*)thread_args_array[i]);
		} 

		for (int i = 0; i < list_size; i++)
		{
			if (process_list[i].get_process_id() == process_id)
			{
				continue;
			}

			pthread_join(thread_ids[i], NULL);
			free(thread_args_array[i]);
		}
	}

	static void* atomic_broadcast_thread_helper(void* args)
	{
	
		ThreadArgs* thread_args = (ThreadArgs*)args;
		static_cast<Process*>(thread_args->receiver)->receive_atomic_broadcast_p1(*thread_args->sender, thread_args->message);

		pthread_exit(NULL);
	}

	void receive_datagram_p1(Process sender, Message m)
	{
		print(to_string(process_id) + " received datagram from " +
			to_string(sender.get_process_id()) + " : " + m.get_content_str());
	}

	void receive_atomic_broadcast_p1(Process sender, Message m)
	{
		print(to_string(process_id) + " received atomic broadcast from " +
			to_string(sender.get_process_id()) + " : " + m.get_content_str());

		sleep(1);

		switch (m.get_content())
		{
			case NEW_GROUP:
				send_atomic_broadcast_p1(PRESENT);
				break;
			case PRESENT:

				break;
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
	for (int i = 0; i < n; i++)
	{
		process_list.push_back(Process(process_count++));
	}

	// 0 initiates the broadcast
	process_list[initiator_id].send_atomic_broadcast_p1(*(new Message(NEW_GROUP)));
	// we have memory leaks oh no
}

int main()
{
	start_timer();
	
	atomic_broadcast_protocol(3, 0);

	double duration = get_current_time();
	print("Time elapsed is " + to_string(duration) + " milliseconds.");

	return 0;
}