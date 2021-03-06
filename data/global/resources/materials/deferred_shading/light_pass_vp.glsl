// Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
// Savant Simulators
// 2012-05
//
// Deferred shading light pass vertex program

#version 120

attribute vec4 vertex;

uniform mat4 g_wvp;

void main()
{
	gl_Position = g_wvp * vertex;
}

