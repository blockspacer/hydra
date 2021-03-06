/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-01
 *	@file keycode.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */


#ifndef HYDRA_KEYCODE_HPP
#define HYDRA_KEYCODE_HPP

#include <string>
#include <OIS/OISKeyboard.h>

namespace vl
{

/// Keyboard modifiers
enum KEY_MOD
{
	KEY_MOD_NONE = 0,		// No modifier is on
	KEY_MOD_META = 1 << 1,	// Alt modifier, either alt
	KEY_MOD_CTRL = 1 << 2,	// Control modifier, either control
	KEY_MOD_SHIFT = 1 << 3,	// Shift modifier, either shift
	KEY_MOD_SUPER = 1 << 4,	// Windows key modifier
	// TODO missing menu modifier if one likes it
};

inline vl::KEY_MOD getModifier( OIS::KeyCode kc )
{
	// Shift
	if( kc == OIS::KC_LSHIFT || kc == OIS::KC_RSHIFT )
	{ return vl::KEY_MOD_SHIFT; }
	// Alt
	else if( kc == OIS::KC_LMENU || kc == OIS::KC_RMENU )
	{ return vl::KEY_MOD_META; }
	// Control
	else if( kc == OIS::KC_RCONTROL || kc == OIS::KC_LCONTROL )
	{ return vl::KEY_MOD_CTRL; }
	// Windows
	else if( kc == OIS::KC_LWIN || kc == OIS::KC_RWIN )
	{ return vl::KEY_MOD_SUPER;	}

	return vl::KEY_MOD_NONE;
}

inline std::string getKeyName( OIS::KeyCode key )
{
	using namespace OIS;

	switch( key )
	{
		case KC_UNASSIGNED : return "UNASSIGNED";
		case KC_ESCAPE :return "ESCAPE";
		case KC_1 :return "N1";
		case KC_2 :return "N2";
		case KC_3 : return "N3";
		case KC_4 :return "N4";
		case KC_5 :return "N5";
		case KC_6 :return "N6";
		case KC_7 : return "N7";
		case KC_8 :return "N8";
		case KC_9 : return "N9";
		case KC_0 : return "N0";
		case KC_MINUS : return "MINUS";
		case KC_EQUALS : return "EQUALS";
		case KC_BACK : return "BACK";
		case KC_TAB : return "TAB";
		case KC_Q : return "Q";
		case KC_W : return "W";
		case KC_E :return "E";
		case KC_R : return "R";
		case KC_T : return "T";
		case KC_Y : return "Y";
		case KC_U : return "U";
		case KC_I : return "I";
		case KC_O : return "O";
		case KC_P : return "P";
		case KC_LBRACKET : return "LBRACKET";
		case KC_RBRACKET : return "RBRACKET";
		case KC_RETURN : return "RETURN";
		case KC_LCONTROL : return "LCONTROL";
		case KC_A : return "A";
		case KC_S : return "S";
		case KC_D : return "D";
		case KC_F : return "F";
		case KC_G : return "G";
		case KC_H : return "H";
		case KC_J : return "J";
		case KC_K : return "K";
		case KC_L : return "L";
		case KC_SEMICOLON : return "SEMICOLON";
		case KC_APOSTROPHE : return "APOSTROPHE";
		case KC_GRAVE : return "GRAVE";
		case KC_LSHIFT : return "LSHIFT";
		case KC_BACKSLASH : return "BACKSLASH";
		case KC_Z : return "Z";
		case KC_X : return "X";
		case KC_C : return "C";
		case KC_V : return "V";
		case KC_B : return "B";
		case KC_N : return "N";
		case KC_M : return "M";
		case KC_COMMA : return "COMMA";
		case KC_PERIOD : return "PERIOD";
		case KC_SLASH : return "SLASH";
		case KC_RSHIFT : return "RSHIFT";
		case KC_MULTIPLY : return "MULTIPLY";
		case KC_LMENU : return "LMENU";
		case KC_SPACE : return "SPACE";
		case KC_CAPITAL : return "CAPITAL";
		case KC_F1 : return "F1";
		case KC_F2 : return "F2";
		case KC_F3 : return "F3";
		case KC_F4 : return "F4";
		case KC_F5 : return "F5";
		case KC_F6 : return "F6";
		case KC_F7 : return "F7";
		case KC_F8 : return "F8";
		case KC_F9 : return "F9";
		case KC_F10 : return "F10";
		case KC_NUMLOCK : return "NUMLOCK";
		case KC_SCROLL : return "SCROLL";
		case KC_NUMPAD7 : return "NUMPAD7";
		case KC_NUMPAD8 : return "NUMPAD8";
		case KC_NUMPAD9 : return "NUMPAD9";
		case KC_SUBTRACT : return "SUBTRACT";
		case KC_NUMPAD4 : return "NUMPAD4";
		case KC_NUMPAD5 : return "NUMPAD5";
		case KC_NUMPAD6 : return "NUMPAD6";
		case KC_ADD : return "ADD";
		case KC_NUMPAD1 : return "NUMPAD1";
		case KC_NUMPAD2 : return "NUMPAD2";
		case KC_NUMPAD3 : return "NUMPAD3";
		case KC_NUMPAD0 : return "NUMPAD0";
		case KC_DECIMAL : return "DECIMAL";
		case KC_OEM_102 : return "OEM_102";
		case KC_F11 : return "F11";
		case KC_F12 : return "F12";
		case KC_F13 : return "F13";
		case KC_F14 : return "F14";
		case KC_F15 : return "F15";
		case KC_KANA : return "KANA";
		case KC_ABNT_C1 : return "ABNT_C1";
		case KC_CONVERT : return "CONVERT";
		case KC_NOCONVERT : return "NONCONVERT";
		case KC_YEN : return "YEN";
		case KC_ABNT_C2 : return "ABNT_C2";
		case KC_NUMPADEQUALS : return "NUMPADEQUALS";
		case KC_PREVTRACK : return "PREVTRACK";
		case KC_AT : return "AT";
		case KC_COLON : return "COLON";
		case KC_UNDERLINE : return "UNDERLINE";
		case KC_KANJI : return "KANJI";
		case KC_STOP : return "STOP";
		case KC_AX : return "AX";
		case KC_UNLABELED : return "UNLABELED";
		case KC_NEXTTRACK : return "NEXTTRACK";
		case KC_NUMPADENTER : return "NUMPADENTER";
		case KC_RCONTROL : return "RCONTROL";
		case KC_MUTE : return "MUTE";
		case KC_CALCULATOR : return "CALCULATOR";
		case KC_PLAYPAUSE : return "PLAYPAUSE";
		case KC_MEDIASTOP : return "MEDIASTOP";
		case KC_VOLUMEDOWN : return "VOLUMEDOWN";
		case KC_VOLUMEUP : return "VOLUMEUP";
		case KC_WEBHOME : return "WEBHOME";
		case KC_NUMPADCOMMA : return "NUMPADCOMMA";
		case KC_DIVIDE : return "DIVIDE";
		case KC_SYSRQ : return "SYSRQ";
		case KC_RMENU : return "RMENU";
		case KC_PAUSE : return "PAUSE";
		case KC_HOME : return "HOME";
		case KC_UP : return "UP";
		case KC_PGUP : return "PGUP";
		case KC_LEFT : return "LEFT";
		case KC_RIGHT : return "RIGHT";
		case KC_END : return "END";
		case KC_DOWN : return "DOWN";
		case KC_PGDOWN : return "PGDOWN";
		case KC_INSERT : return "INSERT";
		case KC_DELETE : return "KC_DELETE";
		case KC_LWIN : return "LWIN";
		case KC_RWIN : return "RWIN";
		case KC_APPS : return "APPS";
		case KC_POWER : return "POWER";
		case KC_SLEEP : return "SLEEP";
		case KC_WAKE : return "WAKE";
		case KC_WEBSEARCH : return "WEBSEARCH";
		case KC_WEBFAVORITES : return "WEBFAVORITES";
		case KC_WEBREFRESH : return "WEBREFRESH";
		case KC_WEBSTOP : return "WEBSTOP";
		case KC_WEBFORWARD : return "WEBFORWARD";
		case KC_WEBBACK : return "WEBBACK";
		case KC_MYCOMPUTER : return "MYCOMPUTER";
		case KC_MAIL : return "MAIL";
		case KC_MEDIASELECT : return "MEDIASELECT";
		default : return std::string();
	}
}

inline std::string getPythonKeyName( OIS::KeyCode key )
{
	return "KC." + getKeyName(key);
}

inline std::string getCppKeyName( OIS::KeyCode key )
{
	return "KC_" + getKeyName(key);
}


}

#endif // HYDRA_KEYCODE_HPP
