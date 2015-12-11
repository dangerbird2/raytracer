attribute  vec4 vPosition;
attribute  vec3 vNormal;
attribute  vec2 vTexCoord;

uniform mat4 ModelViewEarth;
uniform mat4 Projection;
uniform mat4 NormalMatrix;

varying vec3 pos;
varying vec3 N;
varying vec2 texCoord;

//TODO multiple lighting

uniform vec4 AmbientProducts[SLS_MAX_LIGHTS];
uniform vec4 DiffuseProducts[SLS_MAX_LIGHTS];
uniform vec4 SpecularProducts[SLS_MAX_LIGHTS];
uniform vec4 LightPositions[SLS_MAX_LIGHTS];

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
