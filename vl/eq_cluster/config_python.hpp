#ifndef EQ_OGRE_CONFIG_PYTHON_HPP
#define EQ_OGRE_CONFIG_PYTHON_HPP

#include "config.hpp"
#include "scene_node.hpp"
#include "keycode.hpp"

#include <boost/python.hpp>

namespace python = boost::python;

BOOST_PYTHON_MODULE(eqOgre_python)
{
	using namespace eqOgre;

	// NOTE renaming classes works fine
	// TODO registering objects is not working when using modules
	// or I don't know how to register the modules first
	// init the interpreter and then start calling functions one by one
	// TODO check for overloads and default arguments, they need some extra work
	python::class_<Ogre::Vector3>("Vector3", python::init<Ogre::Real, Ogre::Real, Ogre::Real>() )
		.def_readwrite("x", &Ogre::Vector3::x)
		.def_readwrite("y", &Ogre::Vector3::y)
		.def_readwrite("z", &Ogre::Vector3::z)
		.def("length", &Ogre::Vector3::length)
		.def("normalise", &Ogre::Vector3::normalise)
		// Operators
		.def(python::self + python::self )
		.def(python::self - python::self )
		.def(python::self * python::self )
		.def(python::self * Ogre::Real() )
		.def(python::self / python::self )
		.def(python::self / Ogre::Real() )
		.def(python::self += python::self )
		.def(python::self += Ogre::Real() )
		.def(python::self -= python::self )
		.def(python::self -= Ogre::Real() )
		.def(python::self *= python::self )
		.def(python::self *= Ogre::Real() )
		.def(python::self /= python::self )
		.def(python::self /= Ogre::Real() )
		// Comparison
		.def(python::self == python::self )
		.def(python::self != python::self )
		;

	python::class_<Ogre::Quaternion>("Quaternion", python::init<Ogre::Real, Ogre::Real, Ogre::Real, Ogre::Real>() )
		.def_readwrite("x", &Ogre::Quaternion::x)
		.def_readwrite("y", &Ogre::Quaternion::y)
		.def_readwrite("z", &Ogre::Quaternion::z)
		.def_readwrite("w", &Ogre::Quaternion::w)
		.def("Norm", &Ogre::Quaternion::Norm)
		.def("normalise", &Ogre::Quaternion::normalise)
		.def("equals", &Ogre::Quaternion::equals)
		.def("isNaN", &Ogre::Quaternion::isNaN)
		// Operators
		.def(python::self + python::self )
		.def(python::self - python::self )
		.def(python::self * python::self )
		.def(python::self * Ogre::Vector3() )
		.def(python::self * Ogre::Real() )
		.def(python::self == python::self )
		.def(python::self != python::self )
		;

	python::class_<Ogre::Radian>("Radian")
	;

	python::class_<Config, boost::noncopyable>("Config", python::no_init)
		.def("addEvent", &Config::addEvent)
		.def("removeEvent", &Config::removeEvent)
		.def("hasEvent", &Config::hasEvent)
		.def("addSceneNode", &Config::addSceneNode)
		.def("removeSceneNode", &Config::removeSceneNode)
	;

	python::class_<eqOgre::SceneNode>("SceneNode", python::no_init)
		// TODO the factory method should return ref counted ptr
		.def("create", &eqOgre::SceneNode::create,  python::return_value_policy<python::reference_existing_object>() ).staticmethod("create")
		.add_property("name", python::make_function( &eqOgre::SceneNode::getName, python::return_internal_reference<>() ), &eqOgre::SceneNode::setName )
		.add_property("position", python::make_function( &eqOgre::SceneNode::getPosition, python::return_internal_reference<>() ), &eqOgre::SceneNode::setPosition )
		.add_property("orientation", python::make_function( &eqOgre::SceneNode::getOrientation, python::return_internal_reference<>() ), &eqOgre::SceneNode::setOrientation )
	;

	// TODO are these of any use?
	python::class_<eqOgre::TransformationEvent::KeyPair>("KeyPair", python::init<OIS::KeyCode, OIS::KeyCode>() )
	;

	python::class_<eqOgre::TransformationEvent::KeyPairVec>("KeyPairVec", python::init<eqOgre::TransformationEvent::KeyPair, eqOgre::TransformationEvent::KeyPair, eqOgre::TransformationEvent::KeyPair>() )
	;


	void (eqOgre::TransformationEvent::*tx1)(OIS::KeyCode, OIS::KeyCode) = &eqOgre::TransformationEvent::setTransXKeys;
	void (eqOgre::TransformationEvent::*tx2)(eqOgre::TransformationEvent::KeyPair) = &eqOgre::TransformationEvent::setTransXKeys;
	void (eqOgre::TransformationEvent::*ty1)(OIS::KeyCode, OIS::KeyCode) = &eqOgre::TransformationEvent::setTransYKeys;
	void (eqOgre::TransformationEvent::*ty2)(eqOgre::TransformationEvent::KeyPair) = &eqOgre::TransformationEvent::setTransYKeys;
	void (eqOgre::TransformationEvent::*tz1)(OIS::KeyCode, OIS::KeyCode) = &eqOgre::TransformationEvent::setTransZKeys;
	void (eqOgre::TransformationEvent::*tz2)(eqOgre::TransformationEvent::KeyPair) = &eqOgre::TransformationEvent::setTransZKeys;
	void (eqOgre::TransformationEvent::*rx1)(OIS::KeyCode, OIS::KeyCode) = &eqOgre::TransformationEvent::setRotXKeys;
	void (eqOgre::TransformationEvent::*rx2)(eqOgre::TransformationEvent::KeyPair) = &eqOgre::TransformationEvent::setRotXKeys;
	void (eqOgre::TransformationEvent::*ry1)(OIS::KeyCode, OIS::KeyCode) = &eqOgre::TransformationEvent::setRotYKeys;
	void (eqOgre::TransformationEvent::*ry2)(eqOgre::TransformationEvent::KeyPair) = &eqOgre::TransformationEvent::setRotYKeys;
	void (eqOgre::TransformationEvent::*rz1)(OIS::KeyCode, OIS::KeyCode) = &eqOgre::TransformationEvent::setRotZKeys;
	void (eqOgre::TransformationEvent::*rz2)(eqOgre::TransformationEvent::KeyPair) = &eqOgre::TransformationEvent::setRotZKeys;

	python::class_<eqOgre::TransformationEvent>("TransformationEvent", python::init<eqOgre::SceneNode *>() )
		.add_property("speed", &eqOgre::TransformationEvent::getSpeed, &eqOgre::TransformationEvent::setSpeed )
		.add_property("name", python::make_function( &eqOgre::TransformationEvent::getAngularSpeed, python::return_internal_reference<>() ), &eqOgre::TransformationEvent::setAngularSpeed )
		.def("setTransXKeys", tx1)
		.def("setTransXKeys", tx2)
		.def("setTransYKeys", ty1)
		.def("setTransYKeys", ty2)
		.def("setTransZKeys", tz1)
		.def("setTransZKeys", tz2)
		.def("setRotXKeys", rx1)
		.def("setRotXKeys", rx2)
		.def("setRotYKeys", ry1)
		.def("setRotYKeys", ry2)
		.def("setRotZKeys", rz1)
		.def("setRotZKeys", rz2)
	;

	python::enum_<OIS::KeyCode> python_keycode = python::enum_<OIS::KeyCode>("KC");
	int i = 0;
	while( i < OIS::KC_MEDIASELECT )
	{
		OIS::KeyCode code = (OIS::KeyCode)(i);
		std::string keyName = getKeyName( code );
		if( !keyName.empty() )
		{
			python_keycode.value( keyName.c_str(), code );
		}

		++i;
	}

}

#endif