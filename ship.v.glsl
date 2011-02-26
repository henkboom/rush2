#version 150 compatibility

in vec3 position;

void main()
{
    gl_Position = gl_ModelViewProjectionMatrix * vec4(position, 1.0);
}
