/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-12
 *
 */
#include "event_manager.hpp"

#include "base/exceptions.hpp"

vl::EventManager::EventManager( void )
	: _frame_trigger(0)
	, _key_modifiers(KEY_MOD_NONE)
{}

vl::EventManager::~EventManager( void )
{
	delete _frame_trigger;

	for( std::vector<vl::TrackerTrigger *>::iterator iter = _tracker_triggers.begin();
		iter != _tracker_triggers.end(); ++iter )
	{
		delete *iter;
	}

	for( std::vector<vl::KeyTrigger *>::iterator iter = _key_triggers.begin();
		iter != _key_triggers.end(); ++iter )
	{
		delete *iter;
	}
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
	if( _findTrackerTrigger( name ) )
	{ return true; }

	return false;
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
			/// If there is a different trigger for the keys that are down
			/// with the new modifiers release the old and set the new
			/// @todo this is same for both key released and key pressed
			/// because the only variable is the different new and old modifiers
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

				// Only release a trigger if we have one with a better match
				vl::KeyTrigger *new_trigger = _find_best_match(kc, new_key_modifiers);
				vl::KeyTrigger *old_trigger = _find_best_match(kc, _key_modifiers);
				/// Update triggers if they are different
				if( new_trigger != old_trigger )
				{
					if(old_trigger)
					{ old_trigger->update(vl::KeyTrigger::KS_UP); }
					if(new_trigger)
					{ new_trigger->update(vl::KeyTrigger::KS_DOWN); }
				}
			}	// for keys

			_key_modifiers = new_key_modifiers;
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
			/// If there is a different trigger for the keys that are down
			/// with the new modifiers release the old and set the new
			for( size_t i = 0; i < _keys_down.size(); ++i )
			{
				vl::KeyTrigger *new_trigger = _find_best_match(kc, new_key_modifiers);
				vl::KeyTrigger *old_trigger = _find_best_match(kc, _key_modifiers);
				/// Update triggers if they are different
				if( new_trigger != old_trigger )
				{
					if(old_trigger)
					{ old_trigger->update(vl::KeyTrigger::KS_UP); }
					if(new_trigger)
					{ new_trigger->update(vl::KeyTrigger::KS_DOWN); }
				}
			}	// for keys
			
			_key_modifiers = new_key_modifiers;
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


vl::FrameTrigger *
vl::EventManager::getFrameTrigger( void )
{
	if( !_frame_trigger )
	{ _frame_trigger = new vl::FrameTrigger; }

	return _frame_trigger;
}


/// ----------- Protected -------------
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
