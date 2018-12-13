// Periodic Broadcast Protocol

#define NO_GROUP -1
#define BIG_DELTA 1

#define PI 5

#define FAIL_TIME 12
#define PROGRAM_EXEC_TIME 150

using namespace std;

class Process;
class Group;

extern vector<Process*> process_list;
extern int process_count;

extern unordered_map<int, Group*> group_id_table;
extern int group_count;

extern int atomic_messages_sent;
extern int atomic_messages_received;

enum MessageContent {NEW_GROUP, PRESENT_ADD, PRESENT_CHECK};

void print(string s);

void start_timer();

double get_current_time();

void atomic_broadcast_protocol(int n, int initiator_id);

void print_all_data();

int main(int argc, char *argv[]);
