#version 150 compatibility

in vec3 position;
in float on_edge;

out float frag_depth;
out float frag_on_edge;

void main()
{
    //TODO: this should actually be distance from the player
    frag_depth = length(gl_ModelViewMatrix * vec4(position, 1.0));
    gl_Position = gl_ModelViewProjectionMatrix * vec4(position, 1.0);
    frag_on_edge = on_edge;
}
