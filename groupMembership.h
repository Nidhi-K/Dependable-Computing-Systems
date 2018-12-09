#define NO_GROUP -1
#define BIG_DELTA 1

#define PI 3

using namespace std;

class Process;
class Group;

extern vector<Process*> process_list;
extern int process_count;

extern unordered_map<int, Group*> group_id_table;
extern int group_count;

enum MessageContent {NEW_GROUP, PRESENT_ADD, PRESENT_CHECK};

void print(string s);

void start_timer();

double get_current_time();

void atomic_broadcast_protocol(int n, int initiator_id);

void print_all_data();

int main(int argc, char *argv[]);