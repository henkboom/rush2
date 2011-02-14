#version 150 compatibility

in float depth;
in float edge;
in vec3 pos;

void main()
{
    float color = max(1.1 - smoothstep(1, 100, depth), 0.3);
    float width = 0.005 + 2 * smoothstep(1, 100, depth);

    if(edge > 1 - width) {
        gl_FragColor.r = color * 1 - (1-edge)/width;
    } else {
        gl_FragColor.r = 0;
    }
    gl_FragColor.g = gl_FragColor.r;
    gl_FragColor.b = gl_FragColor.r;
    gl_FragColor.a = 1.0;
}
