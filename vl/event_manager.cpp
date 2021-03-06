/**
 *	Copyright (c) 2010 - 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2010-12
 *	@file event_manager.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

#include "event_manager.hpp"

/// Base stuff
#include "base/exceptions.hpp"
#include "base/string_utils.hpp"
#include "logger.hpp"

/// Helpers
#include "input/ois_converters.hpp"

/// Input Devices
#include "input/vrpn_analog_client.hpp"
#include "input/tracker.hpp"
#include "input/tracker_serializer.hpp"
#include "input/pcan.hpp"

/// Necessary for file loading
#include "resource_manager.hpp"

vl::EventManager::EventManager(ResourceManager *res_man)
	: _frame_trigger(0)
	, _key_modifiers(KEY_MOD_NONE)
	, _trackers(new vl::Clients(this))
	, _resource_manager(res_man)
{}

vl::EventManager::~EventManager( void )
{
	removeTriggers();

	// Cleanup objects created from environment config
	for(std::vector<vl::TrackerTrigger *>::iterator iter = _tracker_triggers.begin();
		iter != _tracker_triggers.end(); ++iter)
	{
		delete *iter;
	}
	_tracker_triggers.clear();
}


vl::TrackerTrigger *
vl::EventManager::createTrackerTrigger( std::string const &name )
{
	vl::TrackerTrigger *trigger = _findTrackerTrigger( name );
	if( !trigger )
	{
		trigger = new vl::TrackerTrigger;
		trigger->setName( name );
		_tracker_triggers.push_back( trigger );
	}

	return trigger;
}

vl::TrackerTrigger *
vl::EventManager::getTrackerTrigger(const std::string& name)
{
	vl::TrackerTrigger *trigger = _findTrackerTrigger( name );
	if( trigger )
	{ return trigger; }

	BOOST_THROW_EXCEPTION( vl::null_pointer() );
}

bool
vl::EventManager::hasTrackerTrigger(const std::string& name)
{
	return( _findTrackerTrigger(name) );
}

vl::KeyTrigger *
vl::EventManager::createKeyTrigger(OIS::KeyCode kc, KEY_MOD mod)
{
	vl::KeyTrigger *trigger = _find_key_trigger(kc, std::bitset<8>(mod));

	if( !trigger )
	{
		trigger = new vl::KeyTrigger;
		trigger->setKey(kc);
		trigger->setModifiers(mod);

		_key_triggers.push_back(trigger);
	}

	return trigger;
}

vl::KeyTrigger *
vl::EventManager::getKeyTrigger(OIS::KeyCode kc, KEY_MOD mod)
{
	vl::KeyTrigger *trigger = _find_key_trigger(kc, std::bitset<8>(mod));
	if( trigger )
	{ return trigger; }

	BOOST_THROW_EXCEPTION( vl::null_pointer() );
}

bool
vl::EventManager::hasKeyTrigger(OIS::KeyCode kc, KEY_MOD mod)
{
	return _find_key_trigger(kc, std::bitset<8>(mod));
}

void
vl::EventManager::keyPressed(OIS::KeyCode kc)
{
	bool valid = true;
	if( getModifier(kc) != KEY_MOD_NONE )
	{
		// Add modifiers
		std::bitset<8> new_key_modifiers = _key_modifiers | std::bitset<8>(getModifier(kc));

		/// Just checking that we actually have a new modifier
		if( new_key_modifiers == _key_modifiers )
		{
			std::cout << "vl::EventManager::keyReleased : Modifiers are equal for some weird reason." << std::endl;
			valid = false; 
		}
		else
		{
			_update_key_modifers(new_key_modifiers);
		}	// if valid
	}
	else
	{ valid = _keyDown(kc); }

	// Modifiers can also be used as event triggers.

	/// Check that we don't invoke the event if the key was already down
	if(valid)
	{
		vl::KeyTrigger *trigger = _find_best_match(kc, _key_modifiers);
		if(trigger)
		{ trigger->update(vl::KeyTrigger::KS_DOWN); }
	}
}

void 
vl::EventManager::keyReleased(OIS::KeyCode kc)
{
	bool valid = true;

	// TODO if modifier is removed we should release the event with modifiers
	if( getModifier(kc) != KEY_MOD_NONE )
	{
		// Remove modifiers
		// Negate the modifiers, this will produce only once if none of them is released
		// and zero to the one that has been released. So and will produce all the once
		// not released.
		std::bitset<8> new_key_modifiers = _key_modifiers & (~std::bitset<8>(getModifier(kc)));

		if( new_key_modifiers == _key_modifiers )
		{
			std::cout << "vl::EventManager::keyReleased : Modifiers are equal for some weird reason." << std::endl;
			valid = false; 
		}
		else
		{
			_update_key_modifers(new_key_modifiers);
		}	// if valid
	}
	else
	{ valid = _keyUp(kc); }

	// Modifiers can also be used as event triggers.

	// Check if the there is a trigger for this event
	// Only check if the key was down and is now up
	if(valid)
	{
		vl::KeyTrigger *trigger = _find_best_match(kc, _key_modifiers);
		if(trigger)
		{ trigger->update(vl::KeyTrigger::KS_UP); }
	}
}


/// mouse related:
vl::MouseTrigger*
vl::EventManager::createMouseTrigger(void)
{
	vl::MouseTrigger *trigger = new vl::MouseTrigger();
	_mouse_triggers.push_back(trigger);
	return trigger;
}

void
vl::EventManager::destroyMouseTrigger(vl::MouseTrigger *trigger)
{
	std::vector<vl::MouseTrigger *>::iterator iter 
		= std::find(_mouse_triggers.begin(), _mouse_triggers.end(), trigger);

	if(iter != _mouse_triggers.end())
	{
		delete *iter;
		_mouse_triggers.erase(iter);
	}
}

vl::JoystickTrigger*
vl::EventManager::createJoystickTrigger(void)
{
	vl::JoystickTrigger *trigger = new vl::JoystickTrigger();
	_joystick_triggers.push_back(trigger);
	return trigger;
}

void
vl::EventManager::destroyJoystickTrigger(vl::JoystickTrigger *trigger)
{
	std::vector<vl::JoystickTrigger *>::iterator iter 
		= std::find(_joystick_triggers.begin(), _joystick_triggers.end(), trigger);

	if(iter != _joystick_triggers.end())
	{
		delete *iter;
		_joystick_triggers.erase(iter);
	}
}




void
vl::EventManager::mousePressed(vl::MouseEvent const &evt, vl::MouseEvent::BUTTON b_id)
{
	
	for(std::vector<vl::MouseTrigger *>::iterator iter = _mouse_triggers.begin(); iter != _mouse_triggers.end(); ++iter)
	{
		(*iter)->update(evt, vl::MouseTrigger::MS_PRESSED, b_id);
	}
	
}

void
vl::EventManager::mouseReleased(vl::MouseEvent const &evt, vl::MouseEvent::BUTTON b_id)
{
	
	for(std::vector<vl::MouseTrigger *>::iterator iter = _mouse_triggers.begin(); iter != _mouse_triggers.end(); ++iter)
	{
		(*iter)->update(evt, vl::MouseTrigger::MS_RELEASED, b_id);
	}
	
}

void
vl::EventManager::mouseMoved(vl::MouseEvent const &evt)
{
	
	for(std::vector<vl::MouseTrigger *>::iterator iter = _mouse_triggers.begin(); iter != _mouse_triggers.end(); ++iter)
	{
		(*iter)->update(evt, vl::MouseTrigger::MS_MOVED);
	}
	
}

void vl::EventManager::updateGameJoystick(vl::JoystickEvent const& evt, int index)
{
	for(std::vector< vl::JoystickTrigger* >::iterator iter = _joystick_triggers.begin(); iter != _joystick_triggers.end(); ++iter)
	{
		(*iter)->update(evt, index);
	}
	//std::clog << "update funktio, event_manager.cpp: " << std::endl << evt;
}


vl::FrameTrigger *
vl::EventManager::getFrameTrigger( void )
{
	if( !_frame_trigger )
	{ _frame_trigger = new vl::FrameTrigger; }

	return _frame_trigger;
}

vl::TimeTrigger *
vl::EventManager::createTimeTrigger(void)
{
	vl::TimeTrigger *trigger = new TimeTrigger;
	_time_triggers.push_back(trigger);
	return trigger;
}

void
vl::EventManager::destroyTimeTrigger(vl::TimeTrigger *trigger)
{
	std::vector<TimeTrigger *>::iterator iter 
		= std::find(_time_triggers.begin(), _time_triggers.end(), trigger);

	if(iter != _time_triggers.end())
	{
		delete *iter;
		_time_triggers.erase(iter);
	}
}

vl::PCANRefPtr
vl::EventManager::getPCAN(void)
{
	if(!_pcan)
	{
		_pcan.reset(new PCAN());
	}

	return _pcan;
}

vl::vrpn_analog_client_ref_ptr
vl::EventManager::createAnalogClient(std::string const &name)
{
	vrpn_analog_client_ref_ptr client;
	std::map<std::string, vrpn_analog_client_ref_ptr>::iterator iter
		= _analog_clients.find(name);
	if(iter == _analog_clients.end())
	{
		client.reset(new vrpn_analog_client);
		client->_create(name);
		_analog_clients[name] = client;
	}
	else
	{
		client = iter->second;
	}
	
	return client;
}

void
vl::EventManager::removeTriggers(void)
{
	// We don't destroy tracker triggers because this function is ment
	// to use when reloading or reseting python context
	// and tracker triggers are created from environment config.

	for(std::vector<vl::KeyTrigger *>::iterator iter = _key_triggers.begin();
		iter != _key_triggers.end(); ++iter)
	{
		delete *iter;
	}
	_key_triggers.clear();

	delete _frame_trigger;
	_frame_trigger = 0;

	for(std::vector<TimeTrigger *>::iterator iter = _time_triggers.begin();
		iter != _time_triggers.end(); ++iter)
	{
		delete *iter;
	}
	_time_triggers.clear();

	for(std::vector<MouseTrigger *>::iterator iter = _mouse_triggers.begin();
		iter != _mouse_triggers.end(); ++iter)
	{
		delete *iter;
	}
	_mouse_triggers.clear();

	for(std::vector< JoystickTrigger* >::iterator iter = _joystick_triggers.begin();
		iter != _joystick_triggers.end(); ++iter)
	{
		delete *iter;
	}
	_joystick_triggers.clear();

}

void
vl::EventManager::mainloop(vl::time const &elapsed_time)
{
	// Processing order should be
	// - tracking
	// - input devices
	// - timers
	// - frame trigger
	// for the moment we have no control over input devices though
	// (might need to add buffer for them)

	// Process Tracking
	// If we have a tracker object update it, the update will handle all the
	// callbacks and appropriate updates (head matrix and scene nodes).
	// needs to be processed even if paused so we have perspective modifications
	for( size_t i = 0; i < _trackers->getNTrackers(); ++i )
	{
		_trackers->getTrackerPtr(i)->mainloop();
	}
	
	// Process analog tracking devices
	for(std::map<std::string, vrpn_analog_client_ref_ptr>::iterator iter = _analog_clients.begin(); 
		iter != _analog_clients.end(); ++iter )
	{ iter->second->mainloop(); }
	
	

	if(_pcan)
	{ _pcan->mainloop(); }

	for(std::vector<TimeTrigger *>::iterator iter = _time_triggers.begin();
		iter != _time_triggers.end(); ++iter)
	{
		(*iter)->update(elapsed_time);
	}

	if(_frame_trigger)
	{ _frame_trigger->update(elapsed_time); }
}



/// ------------------------------- File Loading -----------------------------
void
vl::EventManager::loadTrackingFiles(std::vector<std::string> const &files)
{
	assert(_resource_manager && _trackers);

	std::cout << vl::TRACE << "Processing " << files.size()
		<< " tracking files." << std::endl;

	/// @todo This part is the great time consumer, need to pin point the time hog
	for( std::vector<std::string>::const_iterator iter = files.begin();
		 iter != files.end(); ++iter )
	{
		// Read a file
		vl::TextResource resource;
		_resource_manager->loadResource(*iter, resource);

		vl::TrackerSerializer ser(_trackers);
		ser.parseTrackers(resource);
	}
}




/// --------------------------- Protected ------------------------------------
vl::TrackerTrigger *
vl::EventManager::_findTrackerTrigger(const std::string& name)
{
	for( size_t i = 0; i < _tracker_triggers.size(); ++i )
	{
		vl::TrackerTrigger *trigger = _tracker_triggers.at(i);
		if( trigger->getName() == name )
		{ return trigger; }
	}

	return 0;
}

vl::KeyTrigger *
vl::EventManager::_find_key_trigger(OIS::KeyCode kc, std::bitset<8> mod)
{
	for( size_t i = 0; i < _key_triggers.size(); ++i )
	{
		vl::KeyTrigger *trigger = _key_triggers.at(i);
		assert(trigger);

		// exact match
		if( trigger->getKey() == kc && std::bitset<8>(trigger->getModifiers()) == mod )
		{ return trigger; }
	}

	return 0;
}

vl::KeyTrigger *
vl::EventManager::_find_best_match(OIS::KeyCode kc, std::bitset<8> mod)
{
	/// Find all the KeyTriggers with matchin keycode
	std::vector<KeyTrigger *> matching_kc;

	for(size_t i = 0; i < _key_triggers.size(); ++i)
	{
		vl::KeyTrigger *trigger = _key_triggers.at(i);
		assert(trigger);
		if( trigger->getKey() == kc )
		{
			std::bitset<8> to_match_mod(trigger->getModifiers());
			// Any trigger that needs more modifiers than supplied will be discarded
			if((to_match_mod & mod) == to_match_mod)
			{ matching_kc.push_back(_key_triggers.at(i)); }
		}
	}

	vl::KeyTrigger *best_match = 0;
	for(size_t i = 0; i < matching_kc.size(); ++i)
	{
		/// Find KeyTrigger with modifiers most closest to mod parameter
		vl::KeyTrigger *trigger = matching_kc.at(i);

		if(!best_match)
		{ best_match = trigger; }
		else
		{
			std::bitset<8> best_match_mod(best_match->getModifiers());
			std::bitset<8> test_mod(trigger->getModifiers());
			// Remove the desired flags so we are left with the unwanted
			/// XOR will give us the bits that differ between the two
			/// As the mod can not have any extra bits here 
			/// all the differing bits are in the matched.
			std::bitset<8> best_left_mod(best_match_mod ^ mod);
			std::bitset<8> possible_left_mod(test_mod ^ mod);
			// Which one has the less of the unwanted flags
			if( possible_left_mod.count() < best_left_mod.count() )
			{
				best_match = trigger;
			}
		}
	}

	return best_match;
}

bool
vl::EventManager::_keyDown( OIS::KeyCode kc )
{
	std::vector<OIS::KeyCode>::iterator iter = std::find( _keys_down.begin(), _keys_down.end(), kc );
	if( iter == _keys_down.end() )
	{
		_keys_down.push_back(kc);
		return true;
	}
	return false;
}

bool
vl::EventManager::_keyUp( OIS::KeyCode kc )
{
	std::vector<OIS::KeyCode>::iterator iter = std::find( _keys_down.begin(), _keys_down.end(), kc );
	if( iter != _keys_down.end() )
	{
		_keys_down.erase(iter);
		return true;
	}
	return false;
}

void 
vl::EventManager::_update_key_modifers(std::bitset<8> new_mod)
{
	/// If there is a different trigger for the keys that are down
	/// with the new modifiers release the old and set the new
	for( size_t i = 0; i < _keys_down.size(); ++i )
	{
		// Option would be to release all the keys and then repress them
		// with the new modifiers which would try to find the best match
		// Even one step further would be to do this for all keys
		// which would allow arbitary key combinations to be used
		// Problematic for trigger events that work only for release or press
		// they are evoked even though the keys state is not changed.
		//
		// Current implementation provides fallbacks with less modifiers
		// but lacks the ability to chain keys other than modifiers.

		/// If there is a different trigger for the keys that are down
		/// with the new modifiers release the old and set the new

		OIS::KeyCode kc = _keys_down.at(i);
		vl::KeyTrigger *new_trigger = _find_best_match(kc, new_mod);
		vl::KeyTrigger *old_trigger = _find_best_match(kc, _key_modifiers);

		/// Only update triggers if they are different
		if( new_trigger != old_trigger )
		{
			if(old_trigger)
			{ old_trigger->update(vl::KeyTrigger::KS_UP); }
			if(new_trigger)
			{ new_trigger->update(vl::KeyTrigger::KS_DOWN); }
		}

	}	// for keys

	_key_modifiers = new_mod;
}
