class Message
{
	private:
	MessageContent content;
	double time_stamp;
	int group_id;

	public:

	Message(MessageContent c);

	Message(MessageContent content, int group_id);

	void set_group_id(int id);

	int get_group_id();

	double get_time_stamp();

	int get_content();

	string get_content_str();
};