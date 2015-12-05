uniform vec4 AmbientProduct, DiffuseProduct, SpecularProduct;
uniform vec4 LightPosition;
uniform float Shininess;

uniform mat4 ModelViewLight;

varying vec3 pos;
varying vec3 N;
varying vec2 texCoord;

void main()
{
  
  vec3 L;
  if(LightPosition.w == 0.0){
    L = normalize(normalize(LightPosition.xyz)- pos);
  }else{
    L = normalize( (ModelViewLight*LightPosition).xyz - pos );
  }
  
  vec3 E = normalize( -pos );
  vec3 R = normalize(-reflect(L,N));
  
  
  // Compute terms in the illumination equation
  vec4 ambient = AmbientProduct;
  
  float Kd = max( dot(N, L), 0.0 );
  vec4  diffuse = Kd*DiffuseProduct;
  
  float Ks = pow(max(dot(R,E),0.0),Shininess);
  vec4  specular = Ks * SpecularProduct;
  
  if( dot(L, N) < 0.0 ) {
    specular = vec4(0.0, 0.0, 0.0, 1.0);
  }
  
  
  gl_FragColor = ambient + diffuse + specular;
  gl_FragColor = clamp(gl_FragColor, 0.0, 1.0);
  gl_FragColor.a = 1.0;
  
}

