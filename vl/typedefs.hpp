/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-05
 *	@file constraints.hpp
 *
 *	This file is part of Hydra a VR game engine.
 */

#ifndef HYDRA_TYPEDEFS_HPP
#define HYDRA_TYPEDEFS_HPP

#include <map>
#include <string>
#include <vector>

/// Pointers
/// Using boost types rather than tr1 because they can be automatically mapped 
/// with boost::python
#include <memory>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/scoped_ptr.hpp>

namespace vl
{
	typedef std::map<std::string, std::string> NamedParamList;
	
	class Window;

	/// Settings
	class Settings;
	class EnvSettings;
	class ProjSettings;

	typedef boost::shared_ptr<Settings> SettingsRefPtr;
	typedef boost::shared_ptr<EnvSettings> EnvSettingsRefPtr;
	typedef boost::shared_ptr<ProjSettings> ProjSettingsRefPtr;
	
	class RendererInterface;
	class Config;

	typedef boost::shared_ptr<Config> ConfigRefPtr;
	// Auto ptr because Renderer has a single owner
	// usually created elsewhere and passed to Config or Client
	// Normal pointer for callbacks
	typedef RendererInterface *RendererPtr;
	typedef std::auto_ptr<RendererInterface> RendererUniquePtr;

	// Tracker objects
	class Sensor;
	class Tracker;
	class Clients;

	typedef boost::shared_ptr<Sensor> SensorRefPtr;
	typedef boost::shared_ptr<Tracker> TrackerRefPtr;
	typedef boost::shared_ptr<Clients> ClientsRefPtr;


	class Player;
	class GameManager;
	class EventManager;
	class ResourceManager;
	class PythonContext;

	typedef Player * PlayerPtr;
	// Can not be scoped ptr as config owns it but PythonContext needs access to it
	// change to shared_ptr and weak_ptr when possible
	typedef GameManager *GameManagerPtr;
	typedef EventManager * EventManagerPtr;
	typedef ResourceManager * ResourceManagerPtr;
	typedef boost::shared_ptr<ResourceManager> ResourceManagerRefPtr;
	typedef PythonContext * PythonContextPtr;

	/// SceneObjects
	class SceneManager;
	class SceneNode;
	class MovableObject;
	class Entity;
	class Light;
	class Camera;
	class MovableText;
	class RayObject;

	typedef SceneManager * SceneManagerPtr;
	typedef SceneNode * SceneNodePtr;
	typedef MovableObject * MovableObjectPtr;
	typedef Entity * EntityPtr;
	typedef Light * LightPtr;
	typedef Camera * CameraPtr;
	typedef MovableText * MovableTextPtr;
	typedef RayObject * RayObjectPtr;

	/// Resources
	class Mesh;
	class MeshManager;

	typedef boost::shared_ptr<Mesh> MeshRefPtr;
	typedef boost::shared_ptr<MeshManager> MeshManagerRefPtr;

	class Recording;
	typedef boost::shared_ptr<Recording> RecordingRefPtr;

	/// Containers
	typedef std::map<std::string, std::string> NamedValuePairList;
	
	typedef std::vector<SceneNodePtr> SceneNodeList;
	typedef std::vector<EntityPtr> EntityList;
	typedef std::vector<MovableObjectPtr> MovableObjectList;

	struct null_deleter
	{
		void operator()(void const *) const
		{
		}
	};

	/// Dummy struct makes easier to store all callbacks to same data structure
	struct Callback
	{
		virtual ~Callback(void) {}
	};

	/// Non-physics constraints
	class ConstraintSolver;
	class Constraint;
	class SixDofConstraint;
	class SliderConstraint;
	class HingeConstraint;
	class FixedConstraint;

	typedef boost::shared_ptr<ConstraintSolver> ConstraintSolverRefPtr;
	typedef boost::shared_ptr<Constraint> ConstraintRefPtr;
	typedef boost::shared_ptr<SixDofConstraint> SixDofConstraintRefPtr;
	typedef boost::shared_ptr<SliderConstraint> SliderConstraintRefPtr;
	typedef boost::shared_ptr<HingeConstraint> HingeConstraintRefPtr;
	typedef boost::shared_ptr<FixedConstraint> FixedConstraintRefPtr;

	class InputDevice;
	class SerialJoystick;
	typedef boost::shared_ptr<SerialJoystick> SerialJoystickRefPtr;
	typedef boost::shared_ptr<InputDevice> InputDeviceRefPtr;

namespace gui
{
	class GUI;
	class Window;
	class EditorWindow;
	class ConsoleWindow;

	typedef boost::shared_ptr<GUI> GUIRefPtr;
	typedef boost::shared_ptr<Window> WindowRefPtr;
	typedef boost::shared_ptr<EditorWindow> EditorWindowRefPtr;
	typedef boost::shared_ptr<ConsoleWindow> ConsoleWindowRefPtr;
}

namespace physics
{
	class World;
	class RigidBody;

	typedef boost::shared_ptr<World> WorldRefPtr;
	typedef boost::weak_ptr<World> WorldWeakPtr;
	typedef boost::shared_ptr<RigidBody> RigidBodyRefPtr;
	typedef boost::weak_ptr<RigidBody> RigidBodyWeakPtr;

	// collision shapes
	class CollisionShape;
	class SphereShape;
	class BoxShape;
	class ConvexHullShape;
	class StaticPlaneShape;
	class StaticTriangleMeshShape;
	class CylinderShape;
	class ConeShape;
	class CapsuleShape;

	typedef boost::shared_ptr<CollisionShape> CollisionShapeRefPtr;
	typedef boost::shared_ptr<SphereShape> SphereShapeRefPtr;
	typedef boost::shared_ptr<BoxShape> BoxShapeRefPtr;
	typedef boost::shared_ptr<ConvexHullShape> ConvexHullShapeRefPtr;
	typedef boost::shared_ptr<StaticPlaneShape> StaticPlaneShapeRefPtr;
	typedef boost::shared_ptr<StaticTriangleMeshShape> StaticTriangleMeshShapeRefPtr;
	typedef boost::shared_ptr<CylinderShape> CylinderShapeRefPtr;
	typedef boost::shared_ptr<ConeShape> ConeShapeRefPtr;
	typedef boost::shared_ptr<CapsuleShape> CapsuleShapeRefPtr;

	class MotionState;
	class Constraint;
	class SixDofConstraint;
	class SliderConstraint;
	class HingeConstraint;

	typedef boost::shared_ptr<vl::physics::Constraint> ConstraintRefPtr;
	typedef boost::shared_ptr<vl::physics::SixDofConstraint> SixDofConstraintRefPtr;
	typedef boost::shared_ptr<vl::physics::SliderConstraint> SliderConstraintRefPtr;
	typedef boost::shared_ptr<vl::physics::HingeConstraint> HingeConstraintRefPtr;
}

namespace cluster
{
	class Client;
	class Server;
	class Message;

	typedef boost::shared_ptr<Client> ClientRefPtr;
	typedef boost::shared_ptr<Server> ServerRefPtr;
	typedef boost::shared_ptr<Message> MessageRefPtr;
}

namespace ogre
{
	class Root;

	typedef boost::shared_ptr<Root> RootRefPtr;

}	// namespace ogre

}	// namespace vl

#endif	// VL_TYPEDEFS_HPP
