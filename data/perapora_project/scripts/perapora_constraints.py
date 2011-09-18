# -*- coding: utf-8 -*-

import math

# Create a translation joint between two bodies along z-axis
# min and max defines the freedom of the joint in meters
# Depending on wether the original coordinates have positive or negative
# z forward, you need to use positive or negative velocity for driving
# positive z forward positive velocity
def createTranslationConstraint(sn0, sn1, transform, min, max) :
	# SliderConstraint works on x-axis, so we rotate the reference
	# z-axis to x-axis
	trans = transform #*Transform(Quaternion(-0.7071, 0, 0, 0.7071))
	#local0_trans = body0.transform_to_local(trans)
	#local1_trans = body1.transform_to_local(trans)
	body0 = game.kinematic_world.create_kinematic_body(sn0)
	body1 = game.kinematic_world.create_kinematic_body(sn1)
	constraint = game.kinematic_world.create_constraint('slider', body0, body1, trans)
	constraint.lower_limit = min
	constraint.upper_limit = max
	return constraint

# Define a hinge constraint using objects +y axis
# +y because the objects were modeled in Blender with +z as the rotation axis
# but the exporter flips y and z
def createHingeConstraint(sn0, sn1, transform, min = Radian(), max = Radian()) :
	trans = transform #*Transform(Quaternion(0.7071, 0.7071, 0, 0))
	#local0_trans = body0.transform_to_local(trans)
	#local1_trans = body1.transform_to_local(trans)
	body0 = game.kinematic_world.create_kinematic_body(sn0)
	body1 = game.kinematic_world.create_kinematic_body(sn1)
	constraint = game.kinematic_world.create_constraint('hinge', body0, body1, trans)
	constraint.lower_limit = min
	constraint.upper_limit = max
	return constraint

def createFixedConstraint(sn0, sn1, transform) :
	trans = transform #*Transform(Quaternion(0.7071, 0.7071, 0, 0))
	#local0_trans = body0.transform_to_local(trans)
	#local1_trans = body1.transform_to_local(trans)
	body0 = game.kinematic_world.create_kinematic_body(sn0)
	body1 = game.kinematic_world.create_kinematic_body(sn1)
	constraint = game.kinematic_world.create_constraint('fixed', body0, body1, trans)
	return constraint

class Cylinder:
	def __init__(self, rod, piston, rod_fixing, piston_fixing):
		self.rod= rod
		self.piston = piston
		self.rod_fixing = rod_fixing
		self.piston_fixing = piston_fixing

		rod_fixing_pos = rod_fixing.world_transformation.position
		rod_pos = rod.world_transformation.position 
		self.rod_position_diff = rod_fixing_pos - rod_pos

		piston_fixing_pos = piston_fixing.world_transformation.position
		piston_pos = piston.world_transformation.position
		self.piston_position_diff = piston_fixing_pos - piston_pos 

		print('piston position diff', self.piston_position_diff)
		print('piston position diff local', piston.world_transformation.quaternion*self.piston_position_diff)

		print('rod', self.rod)
		print('piston', self.piston)
	
	# Callback from the object we should follow
	# rod fixing point moved
	def moved(self, trans):
		# we no longer need trans but the callback is type safe
		# so the parameter needs to be there
		# we can just use the fixing points saved here
		# as anyway we need to update both rod and piston here

		# Set position first so the look at uses the correct position
		# difference is in fixing points coordinates
		t = self.rod_fixing.world_transformation
		self.rod.position = t.position - t.quaternion*self.rod_position_diff

		direction = self.rod_fixing.world_transformation.position - self.piston_fixing.world_transformation.position
		self.rod.set_direction(direction, Vector3(0, -1, 0))

		# Update piston
		# Pistons position does not change
		# yes it does we need to use the coordine frame from piston_fixing for it
		#t = self.piston_fixing.world_transformation
		piston_fixing_pos = self.piston_fixing.world_transformation.position
		# TODO these set local frames even though the positions are in world
		self.piston.set_direction(direction, Vector3(0, -1, 0))

		# TODO
		# This is incorrect for some reason
		# For some reason the the new position is in incorrect coordinate system
		q = self.piston.world_transformation.quaternion
		#q = Quaternion(0.7071, 0.7071, 0, 0) * q
		q = Quaternion(0.7071, 0, -0.7071, 0) * q
		self.piston.position = piston_fixing_pos + q*self.piston_position_diff
		#print('moving piston by : ', q*self.piston_position_diff)



camera_name = "Camera"
camera = game.scene.createSceneNode(camera_name)
camera.position = Vector3(-0.77, 0, 4)
cam = game.scene.createCamera(camera_name)
camera.attachObject(cam)
createCameraMovements(camera)
game.player.camera = camera_name

game.scene.sky = SkyDomeInfo("CloudySky")

# Create light
spot = game.scene.createSceneNode("spot")
spot_l = game.scene.createLight("spot")
#spot_l.type = "spot"
spot_l.diffuse = ColourValue(0.6, 0.6, 0.6)
spot.attachObject(spot_l)
spot.position = Vector3(0, 1.5, 3)
#spot.orientation = Quaternion(0, 0, 0.7071, 0.7071)
camera.addChild(spot)

light = game.scene.createSceneNode("light")
light_l = game.scene.createLight("light")
light_l.diffuse = ColourValue(0.6, 0.6, 0.8)
light.attachObject(light_l)
light.position = Vector3(0, 20, 0)

ground_node = game.scene.createSceneNode("ground")
ground = game.scene.createEntity("ground", PF.PLANE)
ground_node.attachObject(ground)
ground.material_name = "ground/bump_mapped/shadows"
ground.cast_shadows = False


# Create the kinematics
kiinnityslevy= game.scene.getSceneNode("cb_kiinnityslevy")
ristikpl_kaantosyl = game.scene.getSceneNode("cb_ristikpl_kaantosyl")
nivel_klevy2 = game.scene.getSceneNode("nivel_klevy2_rotz")

nivel_klevy1 = game.scene.getSceneNode("nivel_klevy1_rotz")
kaantokappale = game.scene.getSceneNode("cb_kaantokappale")
createHingeConstraint(kiinnityslevy, kaantokappale, nivel_klevy1.world_transformation)

# Ulkoputki constraints:
# Needs IK solver for adding more than one constraint
# kaantokappale, nostosylinteri, kaantosylinteri and sisaputki
ulkoputki = game.scene.getSceneNode("cb_ulkoputki")
nivel_puomi = game.scene.getSceneNode("nivel_puomi_rotz")
puomi_hinge = createHingeConstraint(kaantokappale, ulkoputki, nivel_puomi.world_transformation, Radian(-1), Radian(0))

# Cylinder nosto
nivel_nostosyl1 = game.scene.getSceneNode("nivel_nostosyl1_rotz")
createFixedConstraint(ulkoputki, nivel_nostosyl1, nivel_nostosyl1.world_transformation)

# Not creating constraints but a Cylinder handler class
nivel_nostosyl2 = game.scene.getSceneNode("nivel_nostosyl2_rotz")
syl_nosto_varsi = game.scene.getSceneNode("cb_syl_nosto_varsi")
syl_nosto_putki = game.scene.getSceneNode("cb_syl_nosto_putki")

syl_nosto = Cylinder(syl_nosto_putki, syl_nosto_varsi, nivel_nostosyl1, nivel_nostosyl2)
nivel_nostosyl1.addListener(syl_nosto.moved)


# Cylinder kaanto
nivel_kaantosyl1 = game.scene.getSceneNode("nivel_kaantosyl1_rotz")
createFixedConstraint(ulkoputki, nivel_kaantosyl1, nivel_kaantosyl1.world_transformation)

# Not creating constraints but a Cylinder handler class
syl_kaanto_varsi = game.scene.getSceneNode("cb_syl_kaanto_varsi")
nivel_kaantosyl2 = game.scene.getSceneNode("nivel_kaantosyl2_rotz")
syl_kaanto_putki = game.scene.getSceneNode("cb_syl_kaanto_putki")


sisaputki = game.scene.getSceneNode("cb_sisaputki")
nivel_telesk = game.scene.getSceneNode("nivel_telesk_trz")
teleskooppi = createTranslationConstraint(ulkoputki, sisaputki, nivel_telesk.world_transformation, 0., 1.05)

# Add rest of the objects with fixed joints
syl_tilt_putki = game.scene.getSceneNode("cb_syl_tilt_putki")
syl_tilt_varsi = game.scene.getSceneNode("cb_syl_tilt_varsi")
nivel_tiltsyl1 = game.scene.getSceneNode("nivel_tiltsyl1_rotz")
nivel_tiltsyl2 = game.scene.getSceneNode("nivel_tiltsyl2_rotz")
createFixedConstraint(sisaputki, syl_tilt_putki, nivel_tiltsyl1.world_transformation)
createFixedConstraint(syl_tilt_putki, syl_tilt_varsi, nivel_tiltsyl2.world_transformation)

tiltkappale = game.scene.getSceneNode("cb_tiltkappale")
nivel_tilt = game.scene.getSceneNode("nivel_tilt_rotz")
createFixedConstraint(sisaputki, tiltkappale, nivel_tilt.world_transformation)

syl_swing_putki = game.scene.getSceneNode("cb_syl_swing_putki")
syl_swing_varsi = game.scene.getSceneNode("cb_syl_swing_varsi")
nivel_swingsyl1 = game.scene.getSceneNode("nivel_swingsyl1_rotz")
createFixedConstraint(sisaputki, syl_swing_putki, nivel_swingsyl1.world_transformation)
createFixedConstraint(syl_swing_putki, syl_swing_varsi, syl_swing_varsi.world_transformation)

# TODO add constraint to riskikappale_swing
ristikpl_swing = game.scene.getSceneNode("cb_ristikpl_swing")
nivel_swingsyl2 = game.scene.getSceneNode("nivel_swingsyl2_rotz")
createFixedConstraint(syl_swing_putki, ristikpl_swing, nivel_tilt.world_transformation)

swingkappale = game.scene.getSceneNode("cb_swingkappale")
nivel_tiltkpl = game.scene.getSceneNode("nivel_tiltkpl_rotz")
createFixedConstraint(tiltkappale, swingkappale, nivel_tiltkpl.world_transformation)

vaantomoottori = game.scene.getSceneNode("cb_vaantomot")
nivel_vaantomoottori = game.scene.getSceneNode("nivel_vaantomot_rotz")
motor_hinge = createHingeConstraint(swingkappale, vaantomoottori, nivel_vaantomoottori.world_transformation, Radian(-math.pi/2), Radian(math.pi/2))

pulttausnivel = game.scene.getSceneNode("cb_pulttausnivel")
nivel_vaantomoottori2 = game.scene.getSceneNode("nivel_vaantomot2_rotz")
createFixedConstraint(vaantomoottori, pulttausnivel, nivel_vaantomoottori2.world_transformation)

pulttaus_varsi = game.scene.getSceneNode("cb_syl_pulttaus_varsi")
nivel_pultsyl1 = game.scene.getSceneNode("nivel_pulttsyl1_rotz")
createFixedConstraint(pulttausnivel, pulttaus_varsi, nivel_pultsyl1.world_transformation)

pulttaus_putki = game.scene.getSceneNode("cb_syl_pulttaus_putki")
nivel_pultsyl2 = game.scene.getSceneNode("nivel_pulttsyl2_rotz")
createFixedConstraint(pulttaus_varsi, pulttaus_putki, nivel_pultsyl2.world_transformation)

kehto = game.scene.getSceneNode("cb_kehto")
pulttaus_hinge = createHingeConstraint(pulttausnivel, kehto, nivel_vaantomoottori2.world_transformation, Radian(0), Radian(math.pi/4))
pulttaus_hinge.axis = Vector3(1, 0, 0)

syl_syotto_putki = game.scene.getSceneNode("cb_syl_syotto_putki")
syl_syotto_varsi = game.scene.getSceneNode("cb_syl_syotto_varsi")
createFixedConstraint(kehto, syl_syotto_putki, syl_syotto_putki.world_transformation)
createFixedConstraint(syl_syotto_putki, syl_syotto_varsi, syl_syotto_putki.world_transformation)

raide = game.scene.getSceneNode("cb_syottol_kiskot")
createFixedConstraint(kehto, raide, raide.world_transformation)

ohjuri_slaite = game.scene.getSceneNode("cb_ohjuri_slaite")
kiinnike1_slaite = game.scene.getSceneNode("cb_kiinnike1_slaite")
porakelkka = game.scene.getSceneNode("cb_porakelkka")
porakanki = game.scene.getSceneNode("cb_porakanki")
createFixedConstraint(raide, ohjuri_slaite, raide.world_transformation)
createFixedConstraint(raide, kiinnike1_slaite, raide.world_transformation)
createFixedConstraint(raide, porakelkka, raide.world_transformation)
createFixedConstraint(raide, porakanki, raide.world_transformation)

teleskooppi.actuator = True
puomi_hinge.actuator = True
motor_hinge.actuator = True
pulttaus_hinge.actuator = True

# Hide links
#game.scene.hideSceneNodes("nivel*")

# some test code
#pulttaus_hinge.target = Radian(1)

# CB mapping
game.scene.mapCollisionBarriers()

#kiinnityslevy.translate(Vector3(0, 2, 0))

# Joystick control
# Falls back to game joysticks, if none exists the movements
# are just disabled...
joy = game.event_manager.getJoystick("COM5")
joy.zero_size = 0.1

# TODO check the mapping
joy_handler = ConstraintJoystickHandler.create()
joy_handler.set_axis_constraint(1, puomi_hinge)
joy_handler.set_axis_constraint(1, 0, teleskooppi)
joy_handler.set_axis_constraint(0, 0, pulttaus_hinge)
joy_handler.set_axis_constraint(1, 1, motor_hinge)
joy_handler.velocity_multiplier = 0.4

joy.add_handler(joy_handler)

# Add tube simulation
game.enablePhysics(True)
# Tie the ends to SceneNodes
tube_info = TubeConstructionInfo()
# Some test bodies
# TODO create the bodies using the SceneNodes as references
shape = BoxShape.create(Vector3(0.1, 0.1, 0.1))
ms = game.physics_world.createMotionState(kaantokappale.world_transformation, kaantokappale)
start_body = game.physics_world.createRigidBody('kaantokappale', 0, ms, shape)
ms = game.physics_world.createMotionState(vaantomoottori.world_transformation, vaantomoottori)
end_body = game.physics_world.createRigidBody('vaantomoottori', 0, ms, shape)
tube_info.start_body = start_body
tube_info.end_body = end_body 
tube_info.start_frame = Transform(Vector3(0.4, 0, 0), Quaternion(0.7071, 0.7071, 0, 0))
tube_info.end_frame = Transform(Vector3(0, 0, 0), Quaternion(1., 0., 0, 0))
# Leght between vaantomoottori and kaantokappale is bit over 3m
tube_info.length = 4
tube_info.mass = 50
tube_info.radius = 0.05
#tube_info.stiffness = 0.6
#tube_info.damping = 0.3
tube_info.element_size = 0.1

tube = game.physics_world.createTube(tube_info)

def setBodyTransform(t):
	end_body.world_transform = t

vaantomoottori.addListener(setBodyTransform)

