#include "Core.hpp"

namespace Atomica
{
void InitConstructorShader() {
    
}

void InitAtomShader() {
    AtomVert = new Atomica::Shader();
    AtomVert->Init(GL_VERTEX_SHADER,
    R"(
    #version 330 core
    layout(location = 0) in vec2 aPos;
    out vec2 fragCoord;

    void main() {
        fragCoord = aPos * 0.5 + 0.5;        // map [-1,1] -> [0,1]
        gl_Position = vec4(aPos, 0.0, 1.0);
    }
    )");
    
    AtomFrag = new Atomica::Shader();
    AtomFrag->Init(GL_FRAGMENT_SHADER,
    R"(
    #version 330 core

    uniform float iTime;
    uniform vec2 iResolution;

    out vec4 fragColor;

    #define MAX_STEPS 140
    #define MAX_DIST 80.0
    #define SURF_DIST 0.001

    uniform int numProtons;
    uniform int numNeutrons;

    // --- electrons on rings
    uniform int eCount[7];

    float baseRadius = 0.2;
    float orbitGap   = 0.2;
    float electronRadius = 0.04;
    float ringTube = 0.005;
    float particleRadius = 0.08; // proton & neutron sphere size

    const float PI = 3.14159265359;

    //-------------------------------------------------------------
    // Math helpers
    //-------------------------------------------------------------
    float hash(float n){ return fract(sin(n)*43758.5453); }

    float sdSphere(vec3 p, float r){ return length(p) - r; }

    float sdRingInPlane(vec3 p, vec3 planeNormal, float orbitR, float tube) {
        float planeDist = dot(p, planeNormal);
        vec3 proj = p - planeDist * planeNormal;
        float radial = length(proj);
        float ringDist = radial - orbitR;
        return sqrt(ringDist*ringDist + planeDist*planeDist) - tube;
    }

    mat3 rotY(float a){ float c=cos(a), s=sin(a); return mat3(
        c, 0.0, s,
        0.0, 1.0, 0.0,
       -s, 0.0, c
    ); }
    mat3 rotX(float a){ float c=cos(a), s=sin(a); return mat3(
        1.0, 0.0, 0.0,
        0.0, c, -s,
        0.0, s, c
    ); }
    mat3 rotZ(float a){ float c=cos(a), s=sin(a); return mat3(
        c, -s, 0.0,
        s,  c, 0.0,
        0.0,0.0,1.0
    ); }

    void orbitFrame(float level, out mat3 R, out vec3 planeNormal) {
        float yaw = iTime * (0.3 + 0.06 * level);
        float tilt = radians(30.0 + 10.0 * sin(iTime * 0.4 + level));
        float roll = iTime * 0.2 + level;
        R = rotZ(roll) * rotX(tilt) * rotY(yaw);
        planeNormal = R * vec3(0.0, 1.0, 0.0);
    }

    vec3 nucleusPos(int i, int total){
        float offset = 2.0/float(total);
        float inc = PI * (3.0 - sqrt(5.0)); // golden angle
        float y = ((float(i) * offset) - 1.0) + (offset/2.0);
        float r = sqrt(max(0.0, 1.0 - y*y));
        float phi = float(i) * inc;
        vec3 dir = vec3(cos(phi)*r, y, sin(phi)*r);
        float nucleusRadius = baseRadius * 0.5;
        // small breathing motion
        nucleusRadius += 0.03 * sin(iTime + float(i)*2.3);
        return dir * nucleusRadius;
    }

    float map(vec3 p) {
        float d = 1e6;

        // nucleus: protons + neutrons clustered together
        int total = numProtons + numNeutrons;
        for(int i=0;i<total;i++){
            vec3 pos = nucleusPos(i,total);
            float pd = sdSphere(p - pos, particleRadius);
            d = min(d, pd);
        }

        // electron rings
        for (int i=0;i<7;++i){
            if(eCount[i]<=0) continue;
            float radius = baseRadius + orbitGap*float(i+1);
            mat3 R; vec3 n; orbitFrame(float(i),R,n);
            float ringD = sdRingInPlane(p,n,radius,ringTube);
            d = min(d, ringD);
            for (int e=0;e<eCount[i];++e){
                float baseAngle = 6.28318*float(e)/float(eCount[i]);
                float spin = iTime*(1.0+0.1*float(i));
                float ang = baseAngle + spin;
                vec3 localPos = vec3(cos(ang)*radius, 0.0, sin(ang)*radius);
                vec3 worldPos = R * localPos;
                float ed = sdSphere(p - worldPos, electronRadius);
                d = min(d, ed);
            }
        }

        return d;
    }
    vec3 calcNormal(vec3 p){
        float e=0.0008;
        vec2 k=vec2(1.0,-1.0);
        return normalize(
             k.xyy*map(p + k.xyy*e) +
             k.yyx*map(p + k.yyx*e) +
             k.yxy*map(p + k.yxy*e) +
             k.xxx*map(p + k.xxx*e)
        );
    }

    float rayMarch(vec3 ro, vec3 rd, out int hit){
        float t=0.0;
        for(int i=0;i<MAX_STEPS;i++){
            vec3 p = ro + rd * t;
            float dist = map(p);
            if(dist < SURF_DIST){ hit = 1; return t; }
            t += dist;
            if(t > MAX_DIST) break;
        }
        hit = 0; return t;
    }

    vec3 shade(vec3 pos, vec3 ro){
        vec3 nrm = calcNormal(pos);
        vec3 lightDir = normalize(vec3(0.4,0.8,-0.5));
        float diff = clamp(dot(nrm, lightDir), 0.0, 1.0);
        float spec = pow(max(dot(reflect(-lightDir, nrm), normalize(ro - pos)), 0.0), 40.0);

        // nucleus coloring based on index distribution
        int total = numProtons + numNeutrons;
        for(int i=0;i<total;i++){
            vec3 p = nucleusPos(i,total);
            if(length(pos - p) < particleRadius + 0.02){
                if(i < numProtons)
                    return vec3(1.0,0.1,0.1)*(0.4+0.6*diff) + vec3(spec);
                else
                    return vec3(1.0)*(0.3+0.7*diff) + vec3(spec);
            }
        }

        // electrons
        float bestED = 1e6;
        for(int i=0;i<7;i++){
            if(eCount[i]<=0) continue;
            float radius = baseRadius + orbitGap*float(i+1);
            mat3 R; vec3 n; orbitFrame(float(i), R, n);
            for(int e=0;e<eCount[i];e++){
                float baseAngle = 6.28318*float(e)/float(eCount[i]);
                float spin = iTime*(1.0+0.1*float(i));
                float ang = baseAngle + spin;
                vec3 localPos = vec3(cos(ang)*radius, 0.0, sin(ang)*radius);
                vec3 worldPos = R * localPos;
                float ed = length(pos - worldPos) - electronRadius;
                bestED = min(bestED, ed);
            }
        }
        if(bestED < 0.03){
            vec3 blue = vec3(0.1,0.4,1.5);
            float glow = smoothstep(0.03, 0.0, bestED) * 3.0;
            return blue * glow + vec3(0.1);
        }

        // rings
        float bestR = 1e6;
        for(int i=0;i<7;i++){
            if(eCount[i]<=0) continue;
            float radius = baseRadius + orbitGap*float(i+1);
            mat3 R; vec3 n; orbitFrame(float(i), R, n);
            float rd = sdRingInPlane(pos, n, radius, ringTube);
            bestR = min(bestR, rd);
        }
        if(bestR < 0.05){
            vec3 ringC = vec3(1.0);
            float glow = smoothstep(0.05, 0.0, bestR) * 0.8;
            return ringC * (0.3 + 0.4*diff) + vec3(glow*0.4) + vec3(spec*0.1);
        }

        return vec3(0.02,0.05,0.1) + 0.5*diff;
    }

    void main(){
        vec2 fragCoord = gl_FragCoord.xy;
        vec2 uv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;
        float camOrbit = iTime * 0.2;
        vec3 ro = vec3(sin(camOrbit)*4.3, 3.0, cos(camOrbit)*4.3);
        vec3 target = vec3(0.0);
        vec3 f = normalize(target - ro);
        vec3 r = normalize(cross(vec3(0.0,1.0,0.0), f));
        vec3 u = cross(f, r);
        vec3 rd = normalize(r*uv.x + u*uv.y + f*1.7);

        int hit;
        float t = rayMarch(ro, rd, hit);
        vec3 col = vec3(0.0);
        if(hit == 1){
            vec3 pos = ro + rd * t;
            col = shade(pos, ro);
        }

        col += 0.04 * vec3(0.08,0.15,0.35) / (length(uv) + 0.3);
        col = pow(col, vec3(0.4545));
        fragColor = vec4(col, 1.0);
    }

    )");
    
    AtomShader = new Atomica::ShaderProgram();
    std::vector<unsigned int> AtomShaderIDs = { AtomVert->GetID(), AtomFrag->GetID() };
    AtomShader->Init(AtomShaderIDs);
    
}

Atomica::Shader* AtomFrag = nullptr;
Atomica::Shader* AtomVert = nullptr;
Atomica::ShaderProgram* AtomShader = nullptr;

Atomica::Shader* ConstructorFrag = nullptr;
Atomica::Shader* ConstructorVert = nullptr;
Atomica::ShaderProgram* ConstructorShader = nullptr;
}
