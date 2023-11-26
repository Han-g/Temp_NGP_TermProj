#include "EventHandle.h"

bool operator!=(const char_ability& lhs, const char_ability& rhs) {
	return (lhs.bubble_len != rhs.bubble_len || lhs.bubble_num != rhs.bubble_num);
}
bool operator!=(const obj_info& lhs, const obj_info& rhs) {
	return (lhs.posX != rhs.posX) || (lhs.posY != rhs.posY)
		|| (lhs.velX != rhs.velX) || (lhs.velY != rhs.velY)
		|| (lhs.type != rhs.type) || (lhs.obj_status != rhs.obj_status)
		|| (lhs.ablility != rhs.ablility);
}

EventHandle::EventHandle()
{
	char_info = tem;
	wParam = 0;

	m_Key_UP = FALSE;
	m_Key_DOWN = FALSE;
	m_Key_LEFT = FALSE;
	m_Key_RIGHT = FALSE;
	m_Key_BUBBLE = FALSE;
	m_Key_ITEM = FALSE;
}

EventHandle::~EventHandle()
{

}

void EventHandle::check_obj(Send_datatype data)
{
	buf = data;
	for (const obj_info& i : data.object_info) {
		if (i.type == Char_Idle) {
			char_info = i;
			break;
		}
	}
	wParam = data.wParam;
}

void EventHandle::check_key()
{
	if (char_info != tem)
	{
		if (wParam != 0) {
			switch (wParam)
			{
			case 37: // left
				m_Key_LEFT = true;
				break;
			case 38: // up
				m_Key_UP = true;
				break;
			case 39: // right
				m_Key_RIGHT = true;
				break;
			case 40: // down
				m_Key_DOWN = true;
				break;

			case 16: // bubble  (shift)
				m_Key_BUBBLE = true;
				break;
			case 17: // item	 (ctrl)
				m_Key_ITEM = true;
				break;

			default:
				break;
			}
		}
	}
}

obj_info EventHandle::update_char(int x, int y)
{
	if (m_Key_UP || m_Key_DOWN || m_Key_LEFT || m_Key_RIGHT) {
		move_char(x, y);
	}
	if (m_Key_BUBBLE) {
		set_bubble(char_info.posX, char_info.posY);
	}
	return char_info;
}

inline void EventHandle::move_char(int x, int y)
{
	if (char_info.posX + x >= 0 && char_info.posX + x < 15) {
		char_info.posX += x;
	}

	if (char_info.posY + y >= 0 && char_info.posY + y < 15) {
		char_info.posY += y;
	}
}

inline void EventHandle::set_bubble(int char_x, int char_y)
{
	obj_info bubble;
	for (const obj_info& i : buf.object_info) {
		if (i.type == Bubble_Idle) {
			bubble = i;
			break;
		}
	}

	if (char_info.ablility.bubble_num < MAX_BUBBLE_NUM) {
		char_info.ablility.bubble_num += 1;
		bubble.posX = char_info.posX;
		bubble.posY = char_info.posY;
		bubble.type = Bubble_bomb;
	}
}

bool EventHandle::return_key_UP() const
{
	return m_Key_UP;
}

bool EventHandle::return_key_DOWN() const
{
	return m_Key_DOWN;
}

bool EventHandle::return_key_LEFT() const
{
	return m_Key_LEFT;
}

bool EventHandle::return_key_RIGHT() const
{
	return m_Key_RIGHT;
}

bool EventHandle::return_key_BUBBLE() const
{
	return m_Key_BUBBLE;
}

bool EventHandle::return_key_ITEM() const
{
	return m_Key_ITEM;
}
