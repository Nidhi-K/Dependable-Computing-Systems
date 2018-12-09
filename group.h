class Process;

class Group
{
	private:

	double creation_time;
	Process* creator;

	int id;
	vector<Process*> members;

	pthread_t check_failure_thread;

	public:

	Group (Process* creator, double time_stamp);

	static Group* look_up_group(int group_id);

	static void* check_failure_helper(void* group);

	void check_failure();

	double get_id();

	int get_group_size();

	vector<Process*> get_members_list();

	void add_member(Process* p, int p_id);

	void remove_member(Process* p);
};