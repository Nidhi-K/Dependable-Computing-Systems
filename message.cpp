#include <vector>
#include <unordered_map>
#include <string>

#include "groupMembership.h"
#include "message.h"

Message::Message(MessageContent c)
{
	time_stamp = get_current_time();
	content = c;
}

Message::Message(MessageContent content, int group_id)
{
	time_stamp = get_current_time();
	this->content = content;
	this->group_id = group_id;
}

void Message::set_group_id(int id)
{
	group_id = id;
}

int Message::get_group_id()
{
	return group_id;
}

double Message::get_time_stamp()
{
	return time_stamp;
}

int Message::get_content()
{
	return content;
}

string Message::get_content_str()
{
	switch (content)
	{
		case NEW_GROUP: return "NEW_GROUP " + to_string(time_stamp);
		case PRESENT_ADD: return "PRESENT_ADD " + to_string(time_stamp);
		case PRESENT_CHECK: return "PRESENT_CHECK " + to_string(time_stamp);
	}

	return "ERROR";
}