#version 150 compatibility

in vec3 vertex;
in float vertex_type;

out float depth; 
out float edge;
out vec3 pos;

void main()
{
    //TODO: this should actually be distance from the player
    depth = length(gl_ModelViewMatrix * vec4(vertex, 1.0));
    gl_Position = gl_ModelViewProjectionMatrix * vec4(vertex, 1.0);
    edge = vertex_type;
    pos = vertex.xyz;
}
