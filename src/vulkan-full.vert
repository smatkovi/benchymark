#version 450
// Vollbild-Dreieck ohne Eckpunktpuffer; uv deckt 0..1 auf der sichtbaren
// Flaeche, genau wie qt_TexCoord0 im ShaderEffect der Stufe SHADER HEAVY.
layout(location = 0) out vec2 uv;

void main()
{
    vec2 p = vec2(float((gl_VertexIndex << 1) & 2), float(gl_VertexIndex & 2));
    uv = p;
    gl_Position = vec4(p * 2.0 - 1.0, 0.0, 1.0);
}
