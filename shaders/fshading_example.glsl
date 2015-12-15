
#ifndef SLS_MAX_LIGHTS
#define SLS_MAX_LIGHTS 8
#endif //



uniform float Shininess;



uniform vec4 AmbientProducts[SLS_MAX_LIGHTS];
uniform vec4 DiffuseProducts[SLS_MAX_LIGHTS];
uniform vec4 SpecularProducts[SLS_MAX_LIGHTS];
uniform vec4 LightPositions[SLS_MAX_LIGHTS];
uniform float light_intensities[SLS_MAX_LIGHTS];

uniform int n_lights;


uniform mat4 ModelViewLight;

varying vec3 pos;
varying vec3 N;
varying vec2 texCoord;



void main()
{
  if (DiffuseProducts[0].w < 0.1) {
    discard;
  }

  vec4 color_sum = vec4(0.0, 0.0, 0.0, 1.0);

  for (int i=0; i< n_lights; ++i) {
    vec3 L;
    vec4 l_pos = LightPositions[i];
    vec4 d = DiffuseProducts[i];
    vec4 s = SpecularProducts[i];
    vec4 a = AmbientProducts[i];

    vec4 sum = d + s + a;
    if (sum.r + sum.g + sum.b <= 0.0) {
      continue;
    }


    float attenuation = 1.0;
    if(l_pos.w == 0.0){
      L = normalize((ModelViewLight*l_pos).xyz);
    }else{
      L = (ModelViewLight*l_pos).xyz - pos;
      L = normalize(L);

    }

    vec3 E = normalize(-pos);
    vec3 R = normalize(-reflect(L,N));

    // Compute terms in the illumination equation
    vec4 ambient = a;

    float Kd = max( dot(N, L), 0.0 );
    vec4  diffuse = Kd*d;

    float Ks = pow(max(dot(R,E),0.0),Shininess);
    vec4  specular = Ks * s;

    if( dot(L, N) < 0.0 ) {
      specular = vec4(0.0, 0.0, 0.0, 1.0);
    }

    color_sum += clamp(attenuation * (specular + ambient + diffuse), 0.0, 1.0);
  }

  
  gl_FragColor = color_sum;
  gl_FragColor = color_sum;
  gl_FragColor = clamp(gl_FragColor, 0.0, 1.0);
  gl_FragColor.a = DiffuseProducts[0].w;
  
}

