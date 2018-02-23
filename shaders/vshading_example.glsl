/**
vertex shader shader 
*/

in vec4 vPosition;
in vec3 vNormal;
in vec2 vTexCoord;

uniform mat4 ModelViewEarth;
uniform mat4 Projection;
uniform mat4 NormalMatrix;

out vec3 pos;
out vec3 N;
out vec2 texCoord;

uniform vec4 AmbientProducts[SLS_MAX_LIGHTS];
uniform vec4 DiffuseProducts[SLS_MAX_LIGHTS];
uniform vec4 SpecularProducts[SLS_MAX_LIGHTS];
uniform vec4 LightPositions[SLS_MAX_LIGHTS];

uniform vec4 light_powers[SLS_MAX_LIGHTS];

uniform int n_lights;


void main()
{
    texCoord    = vTexCoord;

    // Transform vertex  position into eye coordinates
    pos = (ModelViewEarth * vPosition).xyz;
    
    // Transform vertex normal into eye coordinates
    vec4 Nt = NormalMatrix*vec4(vNormal, 0.0);
    N  = normalize(Nt.xyz);


    gl_Position = Projection * ModelViewEarth * vPosition;

}
