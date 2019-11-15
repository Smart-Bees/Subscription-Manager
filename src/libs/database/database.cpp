#include "database.h"

DataBase::DataBase() {}

DataBase::~DataBase() {}

void DataBase::init(const char* hostname, const char* db_name, const char* user, const char* pass) {
	HOSTNAME = hostname;
	DATABASE = db_name;
   	USERNAME = user;
   	PASSWORD = pass;
   	SOCKET = NULL;

	dbc = new SimpleSQL::Connector();
	if(!dbc->connect(HOSTNAME, USERNAME, PASSWORD, DATABASE, PORT_NO)) {
		char ex[] = "ERROR with connecting database ";
		throw std::runtime_error(std::strcat(ex, db_name));
	}
	dbc->connect(HOSTNAME, USERNAME, PASSWORD, DATABASE, PORT_NO);
}

bool DataBase::time_compare(struct tm tm_n, struct tm tm_e) {
	if(tm_n.tm_year == tm_e.tm_year) {
		if(tm_n.tm_mon == tm_e.tm_mon) {
			if(tm_n.tm_mday == tm_e.tm_mday) {
				if(tm_n.tm_hour == tm_e.tm_hour) {
					if(tm_n.tm_min == tm_e.tm_min) {
						return true;
					} else return tm_n.tm_min < tm_e.tm_min;
				} else return tm_n.tm_hour < tm_e.tm_hour;
			} else return tm_n.tm_mday < tm_e.tm_mday;
		} else return tm_n.tm_mon < tm_e.tm_mon;
	} else return tm_n.tm_year < tm_e.tm_year;
}

bool DataBase::check_for_sub(int id) {
	if(dbc->is_connected()) {
		check_for_exist(id);
		struct tm tm_e;
		std::string request = "SELECT sub_end_date FROM subs_table WHERE user_id=" + std::to_string(id) + ";";
		auto query_res_ptr = dbc->query(request);
		auto row = query_res_ptr->get_row();
		strptime(std::string(row[0]).c_str(), "%F %H:%M", &tm_e);
		time_t now = time(NULL);
		struct tm *tm_n = localtime(&now);

		return time_compare(*tm_n, tm_e);
	}
	else
		throw std::runtime_error("Database doesn't connected");
}

DataBase::Time DataBase::time_left_counter(struct tm tm_n, struct tm tm_e) {
	long long all_seconds = difftime(mktime(&tm_e), mktime(&tm_n));
	DataBase::Time t_r;
	t_r.minutes = (all_seconds/60)%60;
	t_r.hours = (all_seconds/(60*60))%24;
	t_r.days = (all_seconds/(60*60*24));
	return t_r;
}

DataBase::Time DataBase::time_left(int id) {
	if(dbc->is_connected()) {
		check_for_exist(id);
		if(check_for_sub(id) == 0)
			return Time(0, 0, 0);
		struct tm tm_e;
		std::string request = "SELECT sub_end_date FROM subs_table WHERE user_id=" + std::to_string(id) + ";";
		auto query_res_ptr = dbc->query(request);
		auto row = query_res_ptr->get_row();
		strptime(std::string(row[0]).c_str(), "%F %H:%M", &tm_e);
		time_t now = time(NULL);
		struct tm *tm_n = localtime(&now);

		return time_left_counter(*tm_n, tm_e);
	}
	else
		throw std::runtime_error("Database doesn't connected");
}

void DataBase::update_sub(int id, std::string s_start, std::string s_end) {
	if(dbc->is_connected()) {
		check_for_exist(id);
		std::string request = "UPDATE subs_table SET sub_start_date=\'" + s_start + "\', sub_end_date=\'" + s_end + "\' WHERE user_id=" + std::to_string(id) + ";";
		dbc->query(request);
	}
	else
		throw std::runtime_error("Database doesn't connected");
}

void DataBase::insert_sub(int user_id, std::string username, std::string s_start, std::string s_end) {
	if(dbc->is_connected()) {
		check_for_exist(user_id);
		std::string request = "INSERT INTO subs_table(user_id, username, sub_start_date, sub_end_date) VALUES (\'" + std::to_string(user_id) + "\', \'" + username + "\', \'" + s_start + "\', \'" + s_end + "\');";
		dbc->query(request);
	}
	else
		throw std::runtime_error("Database doesn't connected");

}
void DataBase::delete_sub(int id) {
	if(dbc->is_connected()) {
		check_for_exist(id);
		std::string request = "DELETE FROM subs_table WHERE user_id=" + std::to_string(id) + ";";
		dbc->query(request);
	}
	else {
		throw std::runtime_error("Database doesn't connected");
	}
}

std::string DataBase::time_to_string(time_t t) {
	struct tm *tm = localtime(&t);
	char c[80];
	strftime(c, 80, "%Y.%m.%d %H:%M", tm);
	std::cout << std::string(c) << std::endl;
	return std::string(c);
}

void DataBase::check_for_exist(int id) {
	std::string request = "SELECT * FROM subs_table WHERE user_id=" + std::to_string(id) + ";";
	auto query_res_ptr = dbc->query(request);
	auto row = query_res_ptr->get_row();
	if(!row)
		throw std::runtime_error("User doesn't exist");
}

void DataBase::check_for_exist(std::string name) {
	std::string request = "SELECT * FROM subs_table WHERE username=\'" + name + "\';";
	auto query_res_ptr = dbc->query(request);
	auto row = query_res_ptr->get_row();
	if(row)
		throw std::runtime_error("User has already exist");
}
