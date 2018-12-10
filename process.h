class Group;
class Message;

class Process
{
	private:

	int group_id;
	int process_id;

	typedef struct thread_args
	{
		Process* receiver;
		Process* sender;
		Message message;

	} ThreadArgs;

	vector<int> other_members;
	vector<bool> checked;

	bool active;

	public:	

	Process(int id);

	void leave_group();

	void init_check_array();

	void send_check_p1();

	int check_failure_p1();

	void fail();
	bool is_active();

	void send_atomic_broadcast_p1(Message m);

	static void* atomic_broadcast_thread_helper(void* args);

	void receive_atomic_broadcast_p1(Process* sender, Message m);

	int get_process_id() const;
	int get_group_id();

	bool operator==(const Process rhs);
};