#pragma once

#include "Global.h"

bool operator!=(const char_ability& lhs, const char_ability& rhs);
bool operator!=(const obj_info& lhs, const obj_info& rhs);

class EventHandle
{
public:
	EventHandle();
	~EventHandle();

	void check_obj(Send_datatype data);
	void check_key();
	obj_info update_char(int x, int y);
	inline void move_char(int x, int y);
	inline void set_bubble(int char_x, int char_y);

	bool return_key_UP() const;
	bool return_key_DOWN() const;
	bool return_key_LEFT() const;
	bool return_key_RIGHT() const;
	bool return_key_BUBBLE() const;
	bool return_key_ITEM() const;
private:
	Send_datatype buf;
	obj_info tem = { 0,0,0,0,0,0,{0,0} };
	obj_info char_info;
	int wParam;

	bool m_Key_UP;
	bool m_Key_DOWN;
	bool m_Key_LEFT;
	bool m_Key_RIGHT;
	bool m_Key_BUBBLE;
	bool m_Key_ITEM;
};

