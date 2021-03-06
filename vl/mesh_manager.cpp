/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-05
 *	@file mesh_manager.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

/// Interface
#include "mesh_manager.hpp"

/// Necessary for writing and reading mesh files
#include "mesh_serializer.hpp"

/// Necessary for not implemented exception
#include "base/exceptions.hpp"
/// Necessary for M_PI
#include "math/math.hpp"

#include "resource_manager.hpp"

// Necessary for comparing sub mesh materials
#include "material.hpp"

#include <Procedural.h>

namespace {

template <typename T>
vl::MeshRefPtr make_to_mesh(std::string const &name, Procedural::MeshGenerator<T> const &generator)
{
	vl::MeshRefPtr mesh(new vl::Mesh(name));
	
	Procedural::TriangleBuffer tbuffer;
	generator.addToTriangleBuffer(tbuffer);	

	vl::SubMesh *sub = mesh->createSubMesh();
	sub->setMaterial("BaseWhiteNoLighting");

	// Using independent vertex data because of instancing
	// also shared vertex data provides very little benefits

	// copy vertex data
	sub->vertexData = new vl::VertexData;
	sub->useSharedGeometry = false;
	
	size_t offset = 0;
	sub->vertexData->vertexDeclaration.addElement(0, offset, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
	offset += vl::VertexDeclaration::getTypeSize(Ogre::VET_FLOAT3);
	sub->vertexData->vertexDeclaration.addElement(0, offset, Ogre::VET_FLOAT3, Ogre::VES_NORMAL);
	offset += vl::VertexDeclaration::getTypeSize(Ogre::VET_FLOAT3);
	sub->vertexData->vertexDeclaration.addElement(0, offset, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES);
	offset += vl::VertexDeclaration::getTypeSize(Ogre::VET_FLOAT2);

	size_t vertexSize = offset;
	vl::VertexBufferRefPtr buf = vl::VertexBuffer::create(vertexSize, tbuffer.getVertices().size());
	// @todo modify to use the new Iterators (Position, Normal and UV)
	size_t vertex = 0;
	for(std::vector<Procedural::TriangleBuffer::Vertex>::const_iterator it 
			= tbuffer.getVertices().begin(); it != tbuffer.getVertices().end(); ++it, ++vertex)
	{
		offset = 0;
		
		buf->write(vertex*vertexSize + offset, it->mPosition);
		offset += sizeof(it->mPosition);

		buf->write(vertex*vertexSize + offset, it->mNormal);
		offset += sizeof(it->mNormal);

		buf->write(vertex*vertexSize + offset, it->mUV);
		offset += sizeof(it->mUV);
	}

	sub->vertexData->setBinding(0, buf);

	// copy index data

	// @todo should check if we need to use 32-bit buffer
	// forcing the use of 32 bit index buffer
	//sub->indexData.setIndexSize(vl::IT_32BIT);
	// Nope we should move to using 32-bit index buffers everywhere
	// because the memory overhead is not too high any way.
	// 16-bit buffers can be used in the GPU pipeline when the software buffer is converted.
	sub->indexData.setIndexCount(tbuffer.getIndices().size());
	for(size_t i = 0; i < tbuffer.getIndices().size(); ++i)
	{
		sub->indexData.set(i, (uint32_t)tbuffer.getIndices().at(i));
	}

	/* tangent vector calculation is not yet implemented */

	mesh->calculateBounds();

	return mesh;
}

}	// unamed namespace


/// Callbacks
void 
vl::ManagerMeshLoadedCallback::meshLoaded(vl::MeshRefPtr mesh)
{
	assert(owner);
	owner->meshLoaded(name, mesh);
}

vl::MasterMeshLoaderCallback::MasterMeshLoaderCallback(vl::ResourceManagerRefPtr res_man)
	: manager(res_man)
{}

/// Blocks till the mesh is loaded and passes a valid mesh to callback
void 
vl::MasterMeshLoaderCallback::loadMesh(std::string const &fileName, vl::MeshLoadedCallback *cb)
{
	if(!manager)
	{
		BOOST_THROW_EXCEPTION(vl::null_pointer());
	}
	assert(cb);

	Resource data;
	manager->loadMeshResource(fileName, data);

	/// @todo this needs to convert the Resource to a Mesh
	MeshSerializer ser;
	MeshRefPtr mesh(new Mesh(fileName));
	ser.readMesh(mesh, data);

	cb->meshLoaded(mesh);
}

/// ------------------------------- MeshManager ------------------------------
vl::MeshRefPtr
vl::MeshManager::loadMesh(std::string const &file_name)
{
	vl::MeshRefPtr mesh;
	/// Mesh has already been created
	if(hasMesh(file_name))
	{
		mesh = getMesh(file_name);
	}
	else
	{
		if(!_load_callback)
		{ BOOST_THROW_EXCEPTION(vl::null_pointer()); }

		MeshLoadedCallback *cb = new ManagerMeshLoadedCallback(file_name, this);
		/// Blocking callback
		_load_callback->loadMesh(file_name, cb);
		delete cb;	// can delete because loader was blocking
		
		// mesh is now loaded
		assert(hasMesh(file_name));
		mesh = getMesh(file_name);
	}

	return mesh;
}

void
vl::MeshManager::loadMesh(std::string const &file_name,  MeshLoadedCallback *cb)
{
	assert(cb);

	if(hasMesh(file_name))
	{
		cb->meshLoaded(getMesh(file_name));
	}
	else
	{
		ListenerMap::iterator iter = _waiting_for_loading.find(file_name);
		if(iter != _waiting_for_loading.end())
		{
			/// assume that this is not called twice with same cb
			iter->second.push_back(cb);
			// No need to call load it's already called
		}
		else
		{
			_waiting_for_loading[file_name].push_back(cb);
			_load_callback->loadMesh(file_name, new ManagerMeshLoadedCallback(file_name, this));
		}
	}
}

void
vl::MeshManager::writeMesh(vl::MeshRefPtr, std::string const &file_name)
{
	std::clog << "vl::MeshManager::writeMesh" << std::endl;
	BOOST_THROW_EXCEPTION(vl::not_implemented());
}

vl::MeshRefPtr
vl::MeshManager::createPlane(std::string const &name, Ogre::Real size_x, Ogre::Real size_y, 
	Ogre::Vector3 normal, uint16_t tesselation_x, uint16_t tesselation_y)
{
	if(normal.isZeroLength() || tesselation_x == 0 || tesselation_y == 0 || size_x <= 0 || size_y <= 0)
	{
		// @todo replace with real exception
		BOOST_THROW_EXCEPTION(vl::exception());
	}

	/// For now retrieve mesh instead of making it an error
	if(hasMesh(name))
	{ return getMesh(name); }

	/// Mesh is created with the assumption that normal is +y
	/// so we rotate every vertex to the normal
	Ogre::Quaternion vert_rot = Ogre::Vector3(0, 1, 0).getRotationTo(normal);

	MeshRefPtr mesh(new Mesh(name));
	SubMesh *sub = mesh->createSubMesh();

	sub->vertexData = new VertexData;
	sub->useSharedGeometry = false;

	/// @todo add tangents
	size_t offset = 0;
	sub->vertexData->vertexDeclaration.addElement(0, offset, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
	offset += VertexDeclaration::getTypeSize(Ogre::VET_FLOAT3);
	sub->vertexData->vertexDeclaration.addElement(0, offset, Ogre::VET_FLOAT3, Ogre::VES_NORMAL);
	offset += VertexDeclaration::getTypeSize(Ogre::VET_FLOAT3);
	sub->vertexData->vertexDeclaration.addElement(0, offset, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES);
	offset += VertexDeclaration::getTypeSize(Ogre::VET_FLOAT2);

	uint16_t const M = tesselation_x;
	uint16_t const N = tesselation_y;

	size_t const vertexSize = offset;
	size_t const n_vertices = (M+1)*(N+1);
	vl::VertexBufferRefPtr buf = vl::VertexBuffer::create(vertexSize, n_vertices);

	size_t vertex_count = 0;
	for(uint16_t m = 0; m < M+1; ++m)
	{
		for(uint16_t n = 0; n < N+1; ++n)
		{
			Ogre::Vector3 pos(size_x*m/M - size_x/2, 0, size_y*n/N - size_y/2);
			Ogre::Vector2 uv = Ogre::Vector2(((double)m)/M, ((double)n)/N);

			// @todo change to using vector iterators
			offset = 0;
			size_t vertex = vertex_count; //m*(N+1) + n;

			assert(vertex < n_vertices);
			buf->write(vertex*vertexSize + offset, vert_rot*pos);
			assert(sizeof(pos) == VertexDeclaration::getTypeSize(Ogre::VET_FLOAT3));
			offset += sizeof(pos);

			buf->write(vertex*vertexSize + offset, normal);
			assert(sizeof(normal) == VertexDeclaration::getTypeSize(Ogre::VET_FLOAT3));
			offset += sizeof(normal);

			buf->write(vertex*vertexSize + offset, uv);
			assert(sizeof(uv) == VertexDeclaration::getTypeSize(Ogre::VET_FLOAT2));
			offset += sizeof(uv);
			assert(offset == vertexSize);

			++vertex_count;
		}
	}

	sub->vertexData->setBinding(0, buf);

	mesh->calculateBounds();

	/// @todo add material (or not?) some clear default would be good
	for(uint16_t m = 0; m < M; ++m)
	{
		for(uint16_t n = 0; n < N; ++n)
		{
			sub->addFace(m*(N+1)+n, m*(N+1)+(n+1), (m+1)*(N+1)+n);
			sub->addFace((m+1)*(N+1)+n, m*(N+1)+(n+1), (m+1)*(N+1)+(n+1));
		}
	}

	_meshes[name] = mesh;
	return mesh;
}

vl::MeshRefPtr
vl::MeshManager::createSphere(std::string const &name, Ogre::Real radius, uint16_t longitude, uint16_t latitude)
{
	std::clog << "vl::MeshManager::createSphere" << std::endl;

	/// For now retrieve mesh instead of making it an error
	if(hasMesh(name))
	{ return getMesh(name); }

	Procedural::SphereGenerator generator;
	generator.setRadius(radius).setUTile(longitude).setVTile(latitude);
	vl::MeshRefPtr mesh = make_to_mesh(name, generator);

	assert(mesh);

	_meshes[name] = mesh;
	return mesh;
}

vl::MeshRefPtr
vl::MeshManager::createCube(std::string const &name, Ogre::Vector3 size)
{
	std::clog << "vl::MeshManager::createCube" << std::endl;

	/// For now retrieve mesh instead of making it an error
	if(hasMesh(name))
	{ return getMesh(name); }

	if(size.isZeroLength())
	{
		// @todo replace with real exception
		BOOST_THROW_EXCEPTION(vl::exception());
	}

	Procedural::BoxGenerator generator;
	/// @todo add configurable segments
	generator.setSize(size).setNumSegX(1).setNumSegY(1).setNumSegZ(1);
	vl::MeshRefPtr mesh = make_to_mesh(name, generator);

	assert(mesh);

	_meshes[name] = mesh;
	return mesh;
}

vl::MeshRefPtr
vl::MeshManager::createCylinder(std::string const &name, vl::scalar radius, 
		vl::scalar height, uint16_t seg_height, uint16_t seg_radius)
{
	std::clog << "vl::MeshManager::createCylinder" << std::endl;

	/// For now retrieve mesh instead of making it an error
	if(hasMesh(name))
	{ return getMesh(name); }

	Procedural::CylinderGenerator generator;
	generator.setRadius(radius).setHeight(height).setNumSegHeight(seg_height).setNumSegBase(seg_radius);
	vl::MeshRefPtr mesh = make_to_mesh(name, generator);
	
	// Fix the origin to middle of the cylinder
	assert(mesh->getNumSubMeshes() > 0);
	SubMesh *sub = mesh->getSubMesh(0);
	Ogre::Vector3 offset = mesh->getBounds().getCenter();
	
	PositionIterator iter = sub->vertexData->getPositionIterator();
	for( ; !iter.end(); ++iter)
	{
		*iter -= offset;
	}

	mesh->calculateBounds();

	_meshes[name] = mesh;
	return mesh;
}

vl::MeshRefPtr
vl::MeshManager::createCapsule(std::string const &name, vl::scalar radius, 
		vl::scalar height, uint16_t seg_height, uint16_t seg_radius, uint16_t segments)
{
	std::clog << "vl::MeshManager::createCapsule with height " << height << std::endl;

	/// For now retrieve mesh instead of making it an error
	if(hasMesh(name))
	{ return getMesh(name); }

	Procedural::CapsuleGenerator generator;
	// divide height by two because we use height to mean the actual height of
	// the capsule (as in bounding box size) not height from center.
	generator.setRadius(radius).setHeight(height/2).setNumSegHeight(seg_height).setNumRings(seg_radius).setNumSegments(segments);
	vl::MeshRefPtr mesh = make_to_mesh(name, generator);

	std::clog << mesh->getBounds() << std::endl;
	assert(mesh);

	_meshes[name] = mesh;
	return mesh;
}


vl::MeshRefPtr
vl::MeshManager::createPrefab(std::string const &type_name)
{
	vl::MeshRefPtr mesh;
	if(type_name == "prefab_plane")
	{
		if(!hasMesh(type_name))
		{
			/// Creating a mesh leaves it in the manager for as long as
			/// cleanup is called on the manager, which gives us enough
			/// time even if we don't store the ref pointer.
			mesh = createPlane(type_name, 20, 20);
		}
	}
	else if(type_name == "prefab_cube")
	{
		if(!hasMesh(type_name))
		{
			/// Creating a mesh leaves it in the manager for as long as
			/// cleanup is called on the manager, which gives us enough
			/// time even if we don't store the ref pointer.
			mesh = createCube(type_name);
		}
	}
	else if(type_name == "prefab_sphere")
	{
		if(!hasMesh(type_name))
		{
			/// Creating a mesh leaves it in the manager for as long as
			/// cleanup is called on the manager, which gives us enough
			/// time even if we don't store the ref pointer.
			mesh = createSphere(type_name);
		}
	}
	else
	{ BOOST_THROW_EXCEPTION(vl::invalid_param() << vl::desc("Invalid PREFAB type name : " + type_name)); }

	return mesh;
}

vl::MeshRefPtr 
vl::MeshManager::getMesh(std::string const &name)
{
	MeshMap::iterator iter = _meshes.find(name);
	if(iter != _meshes.end())
	{ return iter->second; }

	/// @todo replace with real exception
	BOOST_THROW_EXCEPTION(vl::exception());
}

bool 
vl::MeshManager::hasMesh(std::string const &name) const
{
	MeshMap::const_iterator iter = _meshes.find(name);
	if(iter != _meshes.end())
	{ return true; }

	return false;
}

void
vl::MeshManager::cleanup_unused(void)
{
	std::clog << "vl::MeshManager::cleanup_unused" << std::endl;
	MeshMap new_map;
	for(MeshMap::iterator iter = _meshes.begin(); iter != _meshes.end(); ++iter)
	{
		// We are sharing the mesh manager on master between the renderer
		// and application.
		// We can't use unique() because renderer side Entities are not destroyed
		// when this function is called.
		// This is so upside down, but would need some architectural changes to get
		// it working properly.
		// The mesh is in use when it has at least 3 users:
		// one master entity, one renderer entity and this list.
		// 
		// This obviously does not work when there are more than one entity using the mesh
		// because then we have N*2+1 users where N is the number of entities.
		// If renderer entities are not destroyed when this is called we still have N+1
		// users where we can not determine N.
		//
		// Probably this will need a proper solution using callbacks from Application
		// to Renderer when objects are destroyed. This would make sure all object
		// destructions happen instantly and we don't need to make multi pass algorithms
		// that first destroy something on master then something on slaves and so on.
		if(iter->second.use_count() > 2)
		{
			std::clog << "Mesh \"" << iter->first << "\" has " << iter->second.use_count()
				<< " users, not removing." << std::endl;
			new_map[iter->first] = iter->second;
		}
		// debug stuff
		else
		{
			std::clog << "Going to remove mesh \"" << iter->first << "\"." << std::endl;
		}
	}

	_meshes = new_map;
}

void
vl::MeshManager::removeAll(void)
{
	_meshes.clear();
}

vl::MeshRefPtr 
vl::MeshManager::createMesh(std::string const &name)
{
	MeshRefPtr mesh(new Mesh(name));
	_meshes[name] = mesh;
	return mesh;
}

bool
vl::MeshManager::checkMaterialUsers(vl::MaterialRefPtr mat)
{
	bool retval = false;

	// temp array for the ones that are left
	std::vector<Ogre::SubEntity *> remaining;
	for(std::vector<Ogre::SubEntity *>::iterator iter = _og_sub_entities.begin();
		iter != _og_sub_entities.end(); ++iter)
	{
		// sub mesh has the original material name before Ogre overwrote it with "BaseWhite"
		if((*iter)->getSubMesh()->getMaterialName() == mat->getName())
		{
			assert(mat->getNative().get());
			(*iter)->setMaterial(mat->getNative());
			retval = true;
		}
		else
		{ remaining.push_back(*iter); }
	}

	_og_sub_entities = remaining;

	// @todo needs to loop through Ogre meshes also... as they are copies of our meshes. Argh.
	//Ogre::MeshManager::getSingleton().
	return retval;
}

void 
vl::MeshManager::meshLoaded(std::string const &mesh_name, vl::MeshRefPtr mesh)
{
	// add to loaded stack
	assert(!hasMesh(mesh_name));
	_meshes[mesh_name] = mesh;

	// check listeners
	ListenerMap::iterator iter = _waiting_for_loading.find(mesh_name);
	if(iter != _waiting_for_loading.end())
	{
		for(size_t i = 0; i < iter->second.size(); ++i)
		{
			iter->second.at(i)->meshLoaded(mesh);
		}
		_waiting_for_loading.erase(iter);
	}
	// else this was called using a blocking loader
}

void
vl::MeshManager::_addSubEntityWithInvalidMaterial(Ogre::SubEntity *se)
{
	if(std::find(_og_sub_entities.begin(), _og_sub_entities.end(), se)
		== _og_sub_entities.end())
	{ _og_sub_entities.push_back(se); }
}
