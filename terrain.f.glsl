#version 150 compatibility

in float frag_depth;
in float frag_on_edge;

void main()
{
    float color = 1.0 - smoothstep(1.0, 150, frag_depth);
    float width = 0.015 + 2.0 * smoothstep(1.0, 100.0, frag_depth);

    if(frag_on_edge > 1.0 - width) {
        gl_FragColor.r = color;
    } else if(frag_on_edge > 1.0 - width*10){
        gl_FragColor.r = 0.0;
    } else {
        gl_FragColor.r = 0.0;
        discard;
    }
    gl_FragColor.gb = gl_FragColor.rr;
    gl_FragColor.b = gl_FragColor.b/(frag_on_edge);
    gl_FragColor.r = gl_FragColor.r/(frag_on_edge+width/10);
    //gl_FragColor.a = 0.7;// 0.2 + gl_FragColor.r * 0.2;
    gl_FragColor.a = 1;
}
