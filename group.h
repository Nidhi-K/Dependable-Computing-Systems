class Process;

class Group
{
	private:

	double creation_time;
	Process* creator;

	int group_id;
	vector<Process*> members;

	pthread_t check_failure_thread;

	public:

	Group (Process* creator, double time_stamp);

	static Group* look_up_group(int group_id);

	static void* check_failure_helper(void* group);

	void check_failure();

	double get_id();

	vector<Process*> print_members_list();

	void add_member(Process* p);

	void remove_member(Process* p);
};