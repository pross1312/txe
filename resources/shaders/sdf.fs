#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// Output fragment color
out vec4 finalColor;

// NOTE: Add here your custom variables
uniform float time;

float map01(float x) {
    return (x + 1.0f) / 2.0f;
}
void main()
{
    // Texel color fetching from texture sampler
    // NOTE: Calculate alpha using signed distance field (SDF)
    float distanceFromOutline = texture(texture0, fragTexCoord).a - 0.5;
    float distanceChangePerFragment = length(vec2(dFdx(distanceFromOutline), dFdy(distanceFromOutline)));
    float alpha = smoothstep(-distanceChangePerFragment, distanceChangePerFragment, distanceFromOutline);

    // Calculate final fragment color
    finalColor = vec4(fragColor.rgb, fragColor.a*alpha);
    // finalColor = vec4(map01(sin(time + fragTexCoord.x * 1.5)),
    //                   map01(cos(time + fragTexCoord.y * 1.5)),
    //                   map01(sin(time + fragTexCoord.x * fragTexCoord.y * 1.5) * cos(time * fragTexCoord.x * fragTexCoord.y * 1.5)),
    //                   alpha) * fragColor;
}
