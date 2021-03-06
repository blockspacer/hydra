/**
 *	Copyright (c) 2010 - 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2010-05
 *	@file tracker.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-05 initial
 *	2010-06 some meat
 *	2010-11 added trigger and action support
 *
 *	Tracker interface.
 *	Real trackers should derive from this.
 *
 *	Has add callback method which user can use to add a callback functor for
 *	specific sensors.
 */

#ifndef VL_TRACKER_HPP
#define VL_TRACKER_HPP

#include "trigger.hpp"

#include "typedefs.hpp"

// Necessary for vl::scalar and vl::Transform
#include "math/math.hpp"

namespace vl
{

/// Trigger class that has a callback to an Action (new event handling design).
class TrackerTrigger : public vl::TransformActionTrigger
{
public :
	TrackerTrigger( void ) {}

	virtual std::string getTypeName( void ) const
	{ return "TrackerTrigger"; }

	/// -------------- Custom Methods ----------------

	/// Name is used for creation from XML and mapping from python
	void setName( std::string const &name )
	{ _name = name; }

	virtual std::string getName( void ) const
	{ return _name; }

protected :
	std::string _name;

};

class TrackerSensor
{
public :
	TrackerSensor( Ogre::Vector3 const &default_pos = Ogre::Vector3::ZERO,
			Ogre::Quaternion const &default_quat = Ogre::Quaternion::IDENTITY );

	/// Sensor values used when not connected to a tracker
	/// Uses different sensor data stack, so if connection is lost for long it can be
	/// defaulted to these values.
	/// For now supports default values if no tracker updates are done
	void setDefaultPosition( Ogre::Vector3 const &pos );
	void setDefaultOrientation( Ogre::Quaternion const &quat );

	void setDefaultTransform(vl::Transform const &t);

	vl::Transform const &getDefaultTransform(void) const
	{ return _default_value; }

	vl::Transform const &getCurrentTransform(void) const
	{ return _last_value; }

	void setTrigger( vl::TrackerTrigger *trigger );

	/// This will return the current trigger of a sensor
	vl::TrackerTrigger *getTrigger( void );

	/// Callback function for updating the Sensor data
	void update( vl::Transform const &data );

protected :
	vl::TrackerTrigger *_trigger;

	vl::Transform _default_value;
	vl::Transform _last_value;

};

std::ostream &
operator<<(std::ostream &os, vl::TrackerSensor const &s);

class Tracker
{
public :
	virtual ~Tracker( void ) {}

	/// @brief mainloop of the tracker for the base tracker empty
	virtual void mainloop(void) {}

	/// @brief Set a sensor does not change the number of sensors
	/// @param i sensor index
	/// @param sensor sensor definition
	void setSensor(size_t i, TrackerSensor const &sensor);

	/// @brief Set a sensor and if there is not enough sensors add it
	/// @param i sensor index
	/// @param sensor sensor definition
	void addSensor(size_t i, vl::TrackerSensor const &sensor);

	TrackerSensor &getSensor(size_t i)
	{ return _sensors.at(i); }

	TrackerSensor const &getSensor(size_t i) const
	{ return _sensors.at(i); }

	void setNSensors(size_t size);

	size_t getNSensors(void) const
	{ return _sensors.size(); }

	std::string const &getName(void) const
	{ return _name; }

	static TrackerRefPtr create(std::string const &trackerName = std::string())
	{ return TrackerRefPtr(new Tracker(trackerName)); }

	bool incorrect_quaternion;

	void setScale(Ogre::Vector3 const &s)
	{ _scale = s; }

	void setPermutation(Ogre::Vector3 const &permute)
	{ _permute = permute; }

	void setSign(Ogre::Vector3 const &s)
	{ _sign = s; }

	void setNeutralPosition(Ogre::Vector3 const &pos)
	{ _neutral_position = pos; }

	void setNeutralOrientation(Ogre::Quaternion const &q)
	{ _neutral_quaternion = q; }
	
protected :
	/// Protected constructor so that user needs to call create
	Tracker(std::string const &trackerName);

	std::string _name;

	std::vector<TrackerSensor> _sensors;

	Ogre::Vector3 _scale;
	Ogre::Vector3 _permute;
	Ogre::Vector3 _sign;
	Ogre::Vector3 _neutral_position;
	Ogre::Quaternion _neutral_quaternion;
};	// class Tracker


class Clients
{
public :
	Clients( vl::EventManagerPtr event_manager )
		: _event_manager(event_manager )
	{}

	void addTracker( TrackerRefPtr tracker )
	{ _trackers.push_back( tracker ); }

	Tracker const &getTracker(size_t index) const
	{ return *_trackers.at(index); }

	TrackerRefPtr getTrackerPtr(size_t index)
	{ return _trackers.at(index); }

	size_t getNTrackers(void) const
	{ return _trackers.size(); }

	vl::EventManagerPtr getEventManager(void)
	{ return _event_manager; }

protected :
	std::vector<TrackerRefPtr> _trackers;

	vl::EventManagerPtr _event_manager;
};

std::ostream &
operator<<(std::ostream &os, Tracker const &t);

std::ostream &
operator<<(std::ostream &os, Clients const &c);

}	// namespace vl

#endif
