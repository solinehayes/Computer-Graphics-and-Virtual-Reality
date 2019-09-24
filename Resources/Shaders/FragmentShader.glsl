#version 450 core // Minimal GL version support expected from the GPU

#define ZMIN 1
#define R 40


struct LightSource {
	vec3 position;
	vec3 color;
	float intensity;
	float ac;
	float al;
	float aq;
};

uniform LightSource lightSource[3];

struct Material {
	vec3 albedo;
	float alpha;
	sampler2D albedoTex;
	sampler2D roughnessTex;
	sampler2D metallicTex;
	sampler2D a0Tex;
	sampler2D toonTex;
	sampler2D xToonTex;
};

uniform Material material;
uniform bool cartoon, xCartoon;

in vec3 fPosition; // Shader input, linearly interpolated by default from the previous stage (here the vertex shader)
in vec3 fNormal;
in vec2 fTexCoord;


out vec4 colorResponse; // Shader output: the color response attached to this fragment

//Normal Texture mode
vec3 calculateRadiance(LightSource lightSource, vec3 fNormal, vec3 fPosition, Material material) {

	vec3 albedo=texture(material.albedoTex,fTexCoord).rgb;
	float roughness=texture(material.roughnessTex, fTexCoord).r;
	float metallic = texture(material.metallicTex, fTexCoord).r;
	float a0 = texture(material.a0Tex, fTexCoord).r;

	vec3 n = normalize (fNormal); // Linear barycentric interpolation does not preserve unit vectors
	vec3 wi = normalize (lightSource.position - fPosition);
	vec3 wo = normalize (-fPosition);
	vec3 Li = lightSource.color * lightSource.intensity;
	vec3 wh = normalize (wi + wo);

	float F = metallic + (1-metallic)*pow((1-max(0, dot(wi,wh))),5);
	float k= roughness*sqrt(2/3.1415);
	float G = dot(n,wi)/(dot(n,wi)*(1-k)+k)*dot(n,wo)/(dot(n,wo)*(1-k)+k) ;
	float D = roughness/(3.1415*pow(1+(pow(roughness,2)-1)*pow(dot(n,wh), 2),2));
	float fs= D*F*G/(4*dot(n,wi)*dot(n,wo));

	float dist=length(lightSource.position-fPosition);
	Li=Li/(lightSource.ac + lightSource.al*dist + lightSource.aq*pow(dist,2));
	vec3 fd = albedo/3.1415;
	vec3 radiance =  Li * (vec3(fs) + fd) * max(0.0, dot(n,wi))*a0;
	return radiance;
}

//Toon Mode
void toonShading(LightSource lightSource, vec3 fNormal, vec3 fPosition, Material material) {
	vec3 n = normalize (fNormal); // Linear barycentric interpolation does not preserve unit vectors
	vec3 wi = normalize (lightSource.position - fPosition);
	vec3 wo = normalize (-fPosition);
	vec3 Li = lightSource.color * lightSource.intensity;
	vec3 wh = normalize (wi + wo);

	float cosAlpha=dot(wo,n);
	float cosBeta=dot(n,wi);

	if (dot(wo, n)<0.3) {
		colorResponse=vec4(lightSource.color*vec3(0,0,0),1);
	}
	else if (abs(cosAlpha-cosBeta) <0.1){
		colorResponse=vec4(lightSource.color*vec3(1,1,1),1);
	}
	else {
		colorResponse=vec4(lightSource.color*material.albedo,1);
	}
}

//X-Toon Mode
void xToonShading(LightSource lightSource, vec3 fNormal, vec3 fPosition, Material material) {
	vec3 n = normalize (fNormal); // Linear barycentric interpolation does not preserve unit vectors
	vec3 wi = normalize (lightSource.position - fPosition);
	vec3 wo = normalize (-fPosition);
	vec3 Li = lightSource.color * lightSource.intensity;
	vec3 wh = normalize (wi + wo);

	float Dt= 1 - log(length(fPosition)/ZMIN)/log(R);

	//New XToon parametrization
	vec2 tTexCoord=vec2(dot(fNormal, wi),Dt);

	vec3 toon=texture(material.xToonTex,tTexCoord).rgb;
		colorResponse=vec4(lightSource.color*toon,1);

}


void main() {
	if (cartoon) {
		toonShading(lightSource[0], fNormal, fPosition, material);
	}
	else if (xCartoon){
		xToonShading(lightSource[0], fNormal, fPosition, material);
	}

	else {
		vec3 radiance=calculateRadiance(lightSource[0], fNormal, fPosition, material);
		colorResponse=vec4(radiance,1.0);
	}


}
