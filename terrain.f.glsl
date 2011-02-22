#version 150 compatibility

in float frag_depth;
in float frag_on_edge;

void main()
{
    float color = 1.2 - smoothstep(1.0, 75, frag_depth);
    float width = 0.015 + 2.0 * smoothstep(1.0, 100.0, frag_depth);

    if(frag_on_edge > 1.0 - width) {
        gl_FragColor.r = color;
    } else {
        gl_FragColor.r = 0.0;
    }
    gl_FragColor.gb = gl_FragColor.rr;
    gl_FragColor.a = 0.2 + gl_FragColor.r * 0.2;
}
