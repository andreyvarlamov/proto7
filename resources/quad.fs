#version 330

in vec2 fragTexCoord;
in vec4 fragColor;
out vec4 finalColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

uniform float apronScale;
uniform float minYUV;
uniform float heightUV;

void main()
{
#if 1
    vec4 texelColor = texture(texture0, fragTexCoord);

    float apronSize = ((apronScale - 1.0) / 2.0) / apronScale;
    vec2 normTexCoord = vec2(fragTexCoord.x, (fragTexCoord.y - minYUV) / heightUV);

    if ((normTexCoord.x > apronSize && normTexCoord.x < (1.0 - apronSize)) &&
        (normTexCoord.y > apronSize && normTexCoord.y < (1.0 - apronSize)))
    {
    }
    else
    {
        float x = normTexCoord.x - 0.5;
        float y = normTexCoord.y - 0.5;
        float d = sqrt(x*x + y*y);

        float outerRadius = 0.5;
        
        if (d > outerRadius)
        {
            discard;
        }

        float innerSqDim = (1.0 - 2.0 * apronSize);
        float innerSqDiag = sqrt(innerSqDim*innerSqDim + innerSqDim*innerSqDim);
        float innerRadius = innerSqDiag / 2.0;

        float a = 1.0 - ((d - innerRadius) / (outerRadius - innerRadius));

        // float a = 1 - (d / 0.5);

        texelColor.a = a;
    }
     
    finalColor = texelColor * colDiffuse * fragColor;
#else
    // finalColor = vec4(vec3(gl_FragCoord.z), 1.0);
    // finalColor = vec4(0.0, 0.0, 0.0, 1.0);
#endif    
}
