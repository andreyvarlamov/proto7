#version 330

in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec4 vertexColor;
out vec2 fragTexCoord;
out vec4 fragColor;

uniform mat4 mvp;
uniform vec3 offset;

void main()
{
    fragTexCoord  = vertexTexCoord;
    fragColor = vertexColor;
    mat4 s = mat4(2.0, 0.0, 0.0, 0.0,
                  0.0, 2.0, 0.0, 0.0,
                  0.0, 0.0, 1.0, 0.0,
                  0.0, 0.0, 0.0, 1.0);
    
    vec4 o = vec4(vertexPosition, 1.0) - vec4(offset, 0.0);
    vec4 so = s * o;
    vec4 p = so + vec4(offset, 0.0);
    gl_Position = mvp*p;
}
