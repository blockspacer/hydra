/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-08
 *	@file python_module.cpp
 *
 *	This file is part of Hydra VR game engine.
 */

// Interface
#include "python_module.hpp"

// Game
#include "game_manager.hpp"

// SceneGraph
#include "scene_manager.hpp"
#include "scene_node.hpp"
#include "entity.hpp"
#include "light.hpp"
#include "camera.hpp"
#include "movable_text.hpp"
#include "ray_object.hpp"

#include "player.hpp"

// For some reason necessary
#include "event_manager.hpp"

// GUI
#include "gui/gui.hpp"

// Animation framework
#include "constraints.hpp"
#include "constraint_solver.hpp"

#include "recording.hpp"

/// Necessary for exporting math
#include "math/transform.hpp"
#include "math/types.hpp"

// Necessary for hide/show system console
#include "base/system_util.hpp"

/// Overloads need to be outside the module definition

/// SceneGraph overloads
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(setSpotlightRange_ov, setSpotlightRange, 2, 3)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(hideSceneNodes_ov, hideSceneNodes, 1, 2)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(createPlane_ovs, createPlane, 3, 6)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(createCube_ovs, createCube, 1, 2)

using namespace vl;

void export_math(void)
{
	python::class_<vl::Transform>("Transform", python::init<python::optional<Ogre::Vector3, Ogre::Quaternion> >() )
		.def(python::init<Ogre::Quaternion, python::optional<Ogre::Vector3> >())
		.def("isIdentity", &vl::Transform::isIdentity)
		.def("setIdentity", &vl::Transform::setIdentity)
		.def("isPositionZero", &vl::Transform::isPositionZero)
		.def("setPositionZero", &vl::Transform::setPositionZero)
		.def("isRotationIdentity", &vl::Transform::isRotationIdentity)
		.def("setRotationIdentity", &vl::Transform::setRotationIdentity)
		.def("invert", &vl::Transform::invert)
		.def("inverted", &vl::Transform::inverted)

		.def_readwrite("position", &vl::Transform::position)
		.def_readwrite("quaternion", &vl::Transform::quaternion)
		
		.def(-python::self)
		.def(python::self *= python::self)
		.def(python::self *= Ogre::Vector3())
		.def(python::self *= Ogre::Matrix4())
		
		.def(python::self * python::self)
		.def(python::self * Ogre::Quaternion())
		.def(python::self * Ogre::Vector3())
		.def(python::self * Ogre::Matrix4())

		.def(python::self == python::self)
		.def(python::self != python::self)
		.def(python::self_ns::str(python::self_ns::self))
	;

}

void export_animation(void)
{
	python::class_<vl::ConstraintSolver, vl::ConstraintSolverRefPtr, boost::noncopyable>("ConstraintSolver", python::no_init)
		.def(python::self_ns::str(python::self_ns::self))
	;

	/// Abstract master class for all constraints, both physics and non-physics constraint
	/// derive from this
	python::class_<vl::Constraint, vl::ConstraintRefPtr, boost::noncopyable>("Constraint", python::no_init)
		.add_property("body_a", python::make_function(&vl::Constraint::getBodyA, python::return_value_policy<python::reference_existing_object>()))
		.add_property("body_a", python::make_function(&vl::Constraint::getBodyB, python::return_value_policy<python::reference_existing_object>()))
		.add_property("actuator", &vl::Constraint::isActuator, &vl::Constraint::setActuator)
		.def(python::self_ns::str(python::self_ns::self))
	;

	python::class_<vl::FixedConstraint, vl::FixedConstraintRefPtr, boost::noncopyable, python::bases<vl::Constraint> >("FixedConstraint", python::no_init)
		.def("create", &vl::FixedConstraint::create)
		.staticmethod("create")
		.def(python::self_ns::str(python::self_ns::self))
	;

	python::class_<vl::SliderConstraint, vl::SliderConstraintRefPtr, boost::noncopyable, python::bases<vl::Constraint> >("SliderConstraint", python::no_init)
		.add_property("lower_limit", &vl::SliderConstraint::getLowerLimit, &vl::SliderConstraint::setLowerLimit)
		.add_property("upper_limit", &vl::SliderConstraint::getUpperLimit, &vl::SliderConstraint::setUpperLimit)
		.add_property("speed", &vl::SliderConstraint::getActuatorSpeed, &vl::SliderConstraint::setActuatorSpeed)
		.add_property("target", &vl::SliderConstraint::getActuatorTarget, &vl::SliderConstraint::setActuatorTarget)
		.add_property("position", &vl::SliderConstraint::getPosition)
		.add_property("axis", python::make_function(&vl::SliderConstraint::getAxis, python::return_value_policy<python::copy_const_reference>()), &vl::SliderConstraint::setAxis)
		.def("create", &vl::SliderConstraint::create)
		.staticmethod("create")
		.def(python::self_ns::str(python::self_ns::self))
	;

	python::class_<vl::HingeConstraint, vl::HingeConstraintRefPtr, boost::noncopyable, python::bases<vl::Constraint> >("HingeConstraint", python::no_init)
		.add_property("lower_limit", python::make_function(&vl::HingeConstraint::getLowerLimit, python::return_value_policy<python::copy_const_reference>()), &vl::HingeConstraint::setLowerLimit)
		.add_property("upper_limit", python::make_function(&vl::HingeConstraint::getUpperLimit, python::return_value_policy<python::copy_const_reference>()), &vl::HingeConstraint::setUpperLimit)
		.add_property("speed", python::make_function(&vl::HingeConstraint::getActuatorSpeed, python::return_value_policy<python::copy_const_reference>()), &vl::HingeConstraint::setActuatorSpeed)
		.add_property("target", python::make_function(&vl::HingeConstraint::getActuatorTarget, python::return_value_policy<python::copy_const_reference>()), &vl::HingeConstraint::setActuatorTarget)
		.add_property("angle", python::make_function(&vl::HingeConstraint::getHingeAngle, python::return_value_policy<python::copy_const_reference>()))
		.add_property("axis", python::make_function(&vl::HingeConstraint::getAxis, python::return_value_policy<python::copy_const_reference>()), &vl::HingeConstraint::setAxis)
		.def("create", &vl::HingeConstraint::create)
		.staticmethod("create")
		.def(python::self_ns::str(python::self_ns::self))
	;
}

// @todo move export scene graph to separate file
void export_scene_graph(void)
{
	vl::MeshRefPtr (MeshManager::*loadMesh_ov0)(std::string const &) = &MeshManager::loadMesh;

	python::class_<vl::MeshManager, vl::MeshManagerRefPtr, boost::noncopyable>("MeshManager", python::no_init)
		.def("loadMesh", loadMesh_ov0)
		.def("createPlane", &vl::MeshManager::createPlane, createPlane_ovs())
		.def("createSphere", &vl::MeshManager::createSphere)
		.def("createCube", &vl::MeshManager::createCube, createCube_ovs())
		.def("getMesh", &vl::MeshManager::getMesh)
		.def("hasMesh", &vl::MeshManager::hasMesh)
		.def("cleanup_unused", &vl::MeshManager::cleanup_unused)
	;

	python::class_<vl::FogInfo>("FogInfo", python::init<>())
		.def(python::init<std::string, python::optional<Ogre::ColourValue, Ogre::Real, Ogre::Real, Ogre::Real> >())
		.add_property("mode", &vl::FogInfo::setMode, &vl::FogInfo::getMode)
		.def_readwrite("colour", &vl::FogInfo::colour_diffuse)
		.def_readwrite("density", &vl::FogInfo::exp_density)
		.def_readwrite("linear_start", &vl::FogInfo::linear_start)
		.def_readwrite("linear_end", &vl::FogInfo::linear_end)
		.def(python::self_ns::str(python::self_ns::self))
	;

	python::class_<vl::SkyDomeInfo>("SkyDomeInfo", python::init< python::optional<std::string> >())
		.def_readwrite("material_name", &vl::SkyDomeInfo::material_name)
		.def_readwrite("curvature", &vl::SkyDomeInfo::curvature)
		.def_readwrite("tiling", &vl::SkyDomeInfo::tiling)
		.def_readwrite("distance", &vl::SkyDomeInfo::distance)
		.def_readwrite("draw_first", &vl::SkyDomeInfo::draw_first)
		.def_readwrite("orientation", &vl::SkyDomeInfo::orientation)
		.def_readwrite("xsegments", &vl::SkyDomeInfo::xsegments)
		.def_readwrite("ysegments", &vl::SkyDomeInfo::ysegments)
		.def_readwrite("ysegments_keep", &vl::SkyDomeInfo::ysegments_keep)
		.def(python::self_ns::str(python::self_ns::self))
	;
	
	void (vl::ShadowInfo::*setShadowTechnique_ov0)(std::string const &) = &vl::ShadowInfo::setShadowTechnique;

	python::class_<vl::ShadowInfo>("ShadowInfo",  python::init< python::optional<std::string, Ogre::ColourValue, std::string> >())
		.def("enable", &vl::ShadowInfo::enable)
		.def("disable", &vl::ShadowInfo::disable)
		.def("isEnabled", &vl::ShadowInfo::isEnabled)
		.add_property("technique", &vl::ShadowInfo::getShadowTechniqueName, setShadowTechnique_ov0)
		.add_property("colour", python::make_function(&vl::ShadowInfo::getColour, python::return_value_policy<python::copy_const_reference>()), &vl::ShadowInfo::setColour)
		.add_property("camera", python::make_function(&vl::ShadowInfo::getCamera, python::return_value_policy<python::copy_const_reference>()), &vl::ShadowInfo::setCamera)
		.def(python::self_ns::str(python::self_ns::self))
	;

	vl::EntityPtr (SceneManager::*createEntity_ov0)(std::string const &, vl::PREFAB) = &SceneManager::createEntity;
	vl::EntityPtr (SceneManager::*createEntity_ov1)(std::string const &, std::string const &) = &SceneManager::createEntity;
	vl::EntityPtr (SceneManager::*createEntity_ov2)(std::string const &, std::string const &, bool) = &SceneManager::createEntity;
	vl::ShadowInfo &(SceneManager::*getShadowInfo_ov0)(void) = &vl::SceneManager::getShadowInfo;
	
	python::class_<vl::SceneManager, boost::noncopyable>("SceneManager", python::no_init)
		// TODO add remove SceneNodes
		.add_property("root", python::make_function( &SceneManager::getRootSceneNode, python::return_value_policy<python::reference_existing_object>() ) )
		.def("createSceneNode", &SceneManager::createSceneNode, python::return_value_policy<python::reference_existing_object>() )
		.def("hasSceneNode", &SceneManager::hasSceneNode )
		.def("getSceneNode", &SceneManager::getSceneNode, python::return_value_policy<python::reference_existing_object>() )
		.def("createEntity", createEntity_ov0, python::return_value_policy<python::reference_existing_object>() )
		.def("createEntity", createEntity_ov1, python::return_value_policy<python::reference_existing_object>() )
		.def("createEntity", createEntity_ov2, python::return_value_policy<python::reference_existing_object>() )
		.def("getEntity", &SceneManager::getEntity, python::return_value_policy<python::reference_existing_object>() )
		.def("hasEntity", &SceneManager::hasEntity)
		.def("createCamera", &SceneManager::createCamera, python::return_value_policy<python::reference_existing_object>() )
		.def("getCamera", &SceneManager::getCamera, python::return_value_policy<python::reference_existing_object>() )
		.def("hasCamera", &SceneManager::hasCamera)
		.def("createLight", &SceneManager::createLight, python::return_value_policy<python::reference_existing_object>() )
		.def("getLight", &SceneManager::getLight, python::return_value_policy<python::reference_existing_object>() )
		.def("hasLight", &SceneManager::hasLight)
		.def("createMovableText", &SceneManager::createMovableText, python::return_value_policy<python::reference_existing_object>() )
		.def("getMovableText", &SceneManager::getMovableText, python::return_value_policy<python::reference_existing_object>() )
		.def("hasMovableText", &SceneManager::hasMovableText)
		.def("createRayObject", &SceneManager::createRayObject, python::return_value_policy<python::reference_existing_object>() )
		.def("getRayObject", &SceneManager::getRayObject, python::return_value_policy<python::reference_existing_object>() )
		.def("hasRayObject", &SceneManager::hasRayObject)

		/// Scene parameters
		/// returns copies of the objects
		.add_property("sky", python::make_function( &vl::SceneManager::getSkyDome, python::return_value_policy<python::copy_const_reference>() ), &vl::SceneManager::setSkyDome )
		.add_property("fog", python::make_function( &vl::SceneManager::getFog, python::return_value_policy<python::copy_const_reference>() ), &vl::SceneManager::setFog )
		.add_property("ambient_light", python::make_function( &vl::SceneManager::getAmbientLight, python::return_value_policy<python::copy_const_reference>() ), &vl::SceneManager::setAmbientLight )
		.add_property("shadows", python::make_function(getShadowInfo_ov0, python::return_internal_reference<>()), &vl::SceneManager::setShadowInfo)
		.add_property("root", python::make_function(&vl::SceneManager::getRootSceneNode, python::return_value_policy<python::reference_existing_object>()))
		
		/// Selection
		.def("addToSelection", &SceneManager::addToSelection)
		.def("removeFromSelection", &SceneManager::removeFromSelection)
		.def("clearSelection", &SceneManager::clearSelection)

		.def("hideSceneNodes", &vl::SceneManager::hideSceneNodes, hideSceneNodes_ov())
		.def("mapCollisionBarriers", &vl::SceneManager::mapCollisionBarriers)
		.def("reloadScene", &SceneManager::reloadScene)
		.def(python::self_ns::str(python::self_ns::self))
	;

	python::enum_<vl::PREFAB>("PF")
		.value("NONE", PF_NONE)
		.value("PLANE", PF_PLANE)
		.value("SPHERE", PF_SPHERE)
		.value("CUBE", PF_CUBE)
	;

	python::class_<vl::MovableObject, boost::noncopyable>("MovableObject", python::no_init)
		.add_property("name", python::make_function( &vl::MovableObject::getName, python::return_value_policy<python::copy_const_reference>() ) )
		// @todo does not work, pure virtual
		//.add_property("type", python::make_function( &vl::Camera::getTypeName, python::return_value_policy<python::copy_const_reference>() ) )
		.add_property("parent", python::make_function( &vl::MovableObject::getParent,python::return_value_policy<python::reference_existing_object>() ) )
		.add_property("visible", &vl::MovableObject::getVisible, &vl::MovableObject::setVisible )
	;

	python::class_<vl::LightAttenuation>("LightAttenuation", python::init<>() )
		.def(python::init<Ogre::Real, Ogre::Real, Ogre::Real, Ogre::Real>())
		.def_readwrite("range", &vl::LightAttenuation::range)
		.def_readwrite("constant", &vl::LightAttenuation::constant)
		.def_readwrite("linear", &vl::LightAttenuation::linear)
		.def_readwrite("quadratic", &vl::LightAttenuation::quadratic)
		.def(python::self_ns::str(python::self_ns::self))
	;

	void (vl::Light::*setType_ov1)(std::string const &type) = &vl::Light::setType;

	python::class_<vl::Light, boost::noncopyable, python::bases<vl::MovableObject> >("Light", python::no_init)
		.add_property("type", &vl::Light::getLightTypeName, setType_ov1 )
		.add_property("diffuse", python::make_function( &vl::Light::getDiffuseColour, python::return_value_policy<python::copy_const_reference>() ), &vl::Light::setDiffuseColour )
		.add_property("specular", python::make_function( &vl::Light::getSpecularColour, python::return_value_policy<python::copy_const_reference>() ), &vl::Light::setSpecularColour )
		.add_property("direction", python::make_function( &vl::Light::getDirection, python::return_value_policy<python::copy_const_reference>() ), &vl::Light::setDirection )
		.add_property("position", python::make_function( &vl::Light::getPosition, python::return_value_policy<python::copy_const_reference>() ), &vl::Light::setPosition )
		.add_property("cast_shadows", &vl::Light::getCastShadows, &vl::Light::setCastShadows )
		// TODO this should use a struct and be a property
		.def("setSpotRange", &vl::Light::setSpotlightRange, setSpotlightRange_ov())
		.add_property("attenuation", python::make_function( &vl::Light::getAttenuation, python::return_value_policy<python::copy_const_reference>() ), &vl::Light::setAttenuation )
		.def(python::self_ns::str(python::self_ns::self))
	;

	python::class_<vl::Camera, boost::noncopyable, python::bases<vl::MovableObject> >("Camera", python::no_init)
		.add_property("near_clip", &vl::Camera::getNearClipDistance, &vl::Camera::setNearClipDistance )
		.add_property("far_clip", &vl::Camera::getFarClipDistance, &vl::Camera::setFarClipDistance )
		.add_property("position", python::make_function( &vl::Camera::getPosition, python::return_value_policy<python::copy_const_reference>() ), &vl::Camera::setPosition )
		.add_property("orientation", python::make_function( &vl::Camera::getOrientation, python::return_value_policy<python::copy_const_reference>() ), &vl::Camera::setOrientation )
		.def(python::self_ns::str(python::self_ns::self))
	;

	python::class_<vl::Entity, boost::noncopyable, python::bases<vl::MovableObject> >("Entity", python::no_init)
		.add_property("material_name", python::make_function( &vl::Entity::getMaterialName, python::return_value_policy<python::copy_const_reference>() ), &vl::Entity::setMaterialName )
		.add_property("cast_shadows", &vl::Entity::getCastShadows, &vl::Entity::setCastShadows )
		.add_property("mesh_name", python::make_function( &vl::Entity::getMeshName, python::return_value_policy<python::copy_const_reference>() ) )
		.add_property("prefab", &vl::Entity::getPrefab)
		.def(python::self_ns::str(python::self_ns::self))
	;

	python::class_<vl::MovableText, boost::noncopyable, python::bases<vl::MovableObject> >("MovableText", python::no_init)
		.add_property("font_name", python::make_function( &vl::MovableText::getFontName, python::return_value_policy<python::copy_const_reference>() ), &vl::MovableText::setFontName )
		.add_property("caption", python::make_function( &vl::MovableText::getCaption, python::return_value_policy<python::copy_const_reference>() ), &vl::MovableText::setCaption )
		.add_property("colour", python::make_function( &vl::MovableText::getColour, python::return_value_policy<python::copy_const_reference>() ), &vl::MovableText::setColour)
		.add_property("char_height", &vl::MovableText::getCharacterHeight, &vl::MovableText::setCharacterHeight)
		.add_property("space_width", &vl::MovableText::getSpaceWidth, &vl::MovableText::setSpaceWidth)
		.add_property("position", python::make_function( &vl::MovableText::getPosition, python::return_value_policy<python::copy_const_reference>() ), &vl::MovableText::setPosition)
		.add_property("show_on_top", &vl::MovableText::getShowOnTop, &vl::MovableText::showOnTop)
		.add_property("track_camera", &vl::MovableText::getTrackCamera, &vl::MovableText::setTrackCamera)
	;
	
	python::class_<vl::RayObject, boost::noncopyable, python::bases<vl::MovableObject> >("RayObject", python::no_init)
		.add_property("direction", python::make_function( &vl::RayObject::getDirection, python::return_value_policy<python::copy_const_reference>() ), &vl::RayObject::setDirection)
		.add_property("position", python::make_function( &vl::RayObject::getPosition, python::return_value_policy<python::copy_const_reference>() ), &vl::RayObject::setPosition)
		.add_property("material", python::make_function( &vl::RayObject::getMaterial, python::return_value_policy<python::copy_const_reference>() ), &vl::RayObject::setMaterial)
		.add_property("length", &vl::RayObject::getLength, &vl::RayObject::setLength)
		.add_property("sphere_radius", &vl::RayObject::getSphereRadius, &vl::RayObject::setSphereRadius)
		.add_property("collision_detection", &vl::RayObject::getCollisionDetection, &vl::RayObject::setCollisionDetection)
		.add_property("draw_collision_sphere", &vl::RayObject::getDrawCollisionSphere, &vl::RayObject::setDrawCollisionSphere)
		.add_property("draw_ray", &vl::RayObject::getDrawRay, &vl::RayObject::setDrawRay)
		.add_property("show_recording", &vl::RayObject::getShowRecordedRays, &vl::RayObject::showRecordedRays)
		.add_property("recording", &vl::RayObject::getRecording, &vl::RayObject::setRecording)
		.def("update", &vl::RayObject::update)
	;

	python::enum_<TransformSpace>("TS")
		.value("LOCAL", TS_LOCAL)
		.value("PARENT", TS_PARENT)
		.value("WORLD", TS_WORLD)
	;

	void (vl::SceneNode::*transform_ov0)(vl::Transform const &) = &vl::SceneNode::transform;
	void (vl::SceneNode::*transform_ov1)(Ogre::Matrix4 const &) = &vl::SceneNode::transform;

	void (vl::SceneNode::*setTransform_ov0)(vl::Transform const &) = &vl::SceneNode::setTransform;
	void (vl::SceneNode::*setTransform_ov1)(vl::Transform const &, vl::SceneNodePtr) = &vl::SceneNode::setTransform;
	void (vl::SceneNode::*setTransform_ov2)(Ogre::Matrix4 const &) = &vl::SceneNode::setTransform;

	vl::Transform const &(vl::SceneNode::*getTransform_ov0)(void) const = &vl::SceneNode::getTransform;
	vl::Transform (vl::SceneNode::*getTransform_ov1)(vl::SceneNodePtr) const = &vl::SceneNode::getTransform;
	void (vl::SceneNode::*translate_ov0)(Ogre::Vector3 const &) = &vl::SceneNode::translate;
	void (vl::SceneNode::*translate_ov1)(Ogre::Vector3 const &, vl::SceneNodePtr) = &vl::SceneNode::translate;
	void (vl::SceneNode::*translate_ov2)(Ogre::Vector3 const &, TransformSpace) = &vl::SceneNode::translate;
	void (vl::SceneNode::*rotate_ov0)(Ogre::Quaternion const &) = &vl::SceneNode::rotate;
	void (vl::SceneNode::*rotate_ov1)(Ogre::Quaternion const &, vl::SceneNodePtr) = &vl::SceneNode::rotate;
	void (vl::SceneNode::*rotate_ov2)(Ogre::Quaternion const &, TransformSpace) = &vl::SceneNode::rotate;
	SceneNodePtr (vl::SceneNode::*sn_clone_ov0)() const = &vl::SceneNode::clone;
	SceneNodePtr (vl::SceneNode::*sn_clone_ov1)(std::string const &) const = &vl::SceneNode::clone;

	// naming convention 
	// transform is the operation of transforming
	// transformation is the noun descriping that transform operation
	python::class_<vl::SceneNode, boost::noncopyable>("SceneNode", python::no_init)
		.def("attachObject", &vl::SceneNode::attachObject)
		.def("detachObject", &vl::SceneNode::detachObject)
		.def("hasObject", &vl::SceneNode::hasObject)
		.def("createChildSceneNode", &vl::SceneNode::createChildSceneNode, python::return_value_policy<python::reference_existing_object>() )
		.def("addChild", &vl::SceneNode::addChild)
		.def("removeChild", &vl::SceneNode::removeChild)
		.def("transform", transform_ov0)
		.def("transform", transform_ov1)
		.def("translate", translate_ov0)
		.def("translate", translate_ov1)
		.def("translate", translate_ov2)
		.def("rotate", rotate_ov0)
		.def("rotate", rotate_ov1)
		.def("rotate", rotate_ov2)
		.def("clone", sn_clone_ov0, python::return_value_policy<python::reference_existing_object>())
		.def("clone", sn_clone_ov1, python::return_value_policy<python::reference_existing_object>())
		.add_property("name", python::make_function( &vl::SceneNode::getName, python::return_value_policy<python::copy_const_reference>() ), &vl::SceneNode::setName )
		.add_property("transformation", python::make_function( getTransform_ov0, python::return_value_policy<python::copy_const_reference>() ), setTransform_ov0)
		.add_property("world_transformation", &vl::SceneNode::getWorldTransform, &vl::SceneNode::setWorldTransform)
		.add_property("position", python::make_function( &vl::SceneNode::getPosition, python::return_value_policy<python::copy_const_reference>() ), &vl::SceneNode::setPosition )
		.add_property("orientation", python::make_function( &vl::SceneNode::getOrientation, python::return_value_policy<python::copy_const_reference>() ), &vl::SceneNode::setOrientation )
		.add_property("scale", python::make_function( &vl::SceneNode::getScale, python::return_value_policy<python::copy_const_reference>() ), &vl::SceneNode::setScale )
		.add_property("visible", &SceneNode::getVisible, &vl::SceneNode::setVisible)
		.add_property("inherit_scale", &SceneNode::getInheritScale, &vl::SceneNode::setInheritScale)
		.add_property("show_bounding_box", &SceneNode::getShowBoundingBox, &vl::SceneNode::setShowBoundingBox)
		.add_property("parent", python::make_function(&vl::SceneNode::getParent, python::return_value_policy<python::reference_existing_object>()) )
		.def("hide", &vl::SceneNode::hide)
		.def("isHidden", &vl::SceneNode::isHidden)
		.def("show", &vl::SceneNode::show)
		.def("isShown", &vl::SceneNode::isShown)
		.def(python::self_ns::str(python::self_ns::self))
	;

}

void export_game(void)
{
	/// System utils
	python::def("show_system_console", vl::show_system_console);
	python::def("hide_system_console", vl::hide_system_console);

	python::class_<vl::time>("time", python::init<uint32_t, uint32_t>())
		.def(python::init<int>())
		.def(python::init<double>())
		.def(python::self -= python::self)
		.def(python::self - python::self)
		.def(python::self += python::self)
		.def(python::self + python::self)
		.def(python::self == python::self)
		.def(python::self != python::self)
		.def(python::self >= python::self)
		.def(python::self <= python::self)
		.def(python::self > python::self)
		.def(python::self < python::self)
		.def(python::self_ns::str(python::self_ns::self))
		.def("set_velocity", &vl::Constraint::setVelocity)
		.def("add_velocity", &vl::Constraint::addVelocity)
	;

	python::class_<vl::GameManager, boost::noncopyable>("GameManager", python::no_init)
		.add_property("scene", python::make_function( &vl::GameManager::getSceneManager, python::return_value_policy<python::reference_existing_object>() ) )
		.add_property("player", python::make_function( &vl::GameManager::getPlayer, python::return_value_policy<python::reference_existing_object>() ) )
		.add_property("event_manager", python::make_function( &vl::GameManager::getEventManager, python::return_value_policy<python::reference_existing_object>() ) )
		.add_property("gui", &vl::GameManager::getGUI)
		.add_property("stats", python::make_function( &vl::GameManager::getStats, python::return_value_policy<python::reference_existing_object>() ) )
		.add_property( "physics_world", &vl::GameManager::getPhysicsWorld)
		.def( "enableAudio", &vl::GameManager::enableAudio )
		.def( "enablePhysics", &vl::GameManager::enablePhysics )
		.add_property("logger", python::make_function( &vl::GameManager::getLogger, python::return_value_policy<python::reference_existing_object>() ) )
		.def("createBackgroundSound", &vl::GameManager::createBackgroundSound)
		.def("toggleBackgroundSound", &vl::GameManager::toggleBackgroundSound)
		.def("addInputDevice", &vl::GameManager::addInputDevice)
		.def("addConstraint", &vl::GameManager::addConstraint)
		.def("removeConstraint", &vl::GameManager::removeConstraint)
		.def("hasConstraint", &vl::GameManager::hasConstraint)
		.add_property("constraint_solver", &vl::GameManager::getConstraintSolver)
		.def("quit", &vl::GameManager::quit)
		.add_property("tracker_clients", &vl::GameManager::getTrackerClients)
		.add_property("mesh_manager", &vl::GameManager::getMeshManager)
		.def("loadRecording", &vl::GameManager::loadRecording)
		.add_property("avarage_input_handling_time", &vl::GameManager::getAvarageInputHandlingTime)
	;

	void (sink::*write1)( std::string const & ) = &sink::write;

	python::class_<vl::sink>("sink", python::no_init)
		.def("write", write1 )
	;

	// TODO add setHeadMatrix function to python
	python::class_<vl::Player, boost::noncopyable>("Player", python::no_init)
		.add_property("camera", python::make_function( &Player::getActiveCamera , python::return_value_policy<python::copy_const_reference>() ), &Player::setActiveCamera )
		.add_property("ipd", &Player::getIPD, &Player::setIPD)
		.def("takeScreenshot", &vl::Player::takeScreenshot)
		.def(python::self_ns::str(python::self_ns::self))
	;

	python::class_<vl::Stats, boost::noncopyable>("Stats", python::no_init)
		.def(python::self_ns::str(python::self_ns::self))
	;

	python::class_<vl::Recording, vl::RecordingRefPtr, boost::noncopyable>("Recording", python::no_init)
		.def(python::self_ns::str(python::self_ns::self))
	;
}