#version 430 core


#include PostProcessEffectBase.glsl
#include Constants.glsl
#include PlanetDefinition.glsl
#include ProceduralValueNoise.glsl
#include WaterHeight.glsl




float intersectPlane(vec3 origin, vec3 direction, vec3 point, vec3 normal)
{ return clamp(dot(point - origin, normal) / dot(direction, normal), -1.0, 9991999.0); }

float rand2sTimex(vec2 co){
    co *= Time;
    return fract(sin(dot(co.xy,vec2(12.9898,78.233))) * 43758.5453);
}

#define intersects(a) (a >= 0.0)
float mipmapx = 0.2;
bool isReallyUnderWater = false;
float raymarchwater3(vec3 start, vec3 end, int stepsI){
    float stepsize = 1.0 / stepsI;
    float iter = 0;
    vec3 pos = start;
    float h = 0.0;
    float hupper = waterdepth + WaterLevel;
    float hlower = WaterLevel;
    float rd = stepsize * rand2sTimex(UV);
    vec2 zer = vec2(0.0);
    for(int i=0;i<stepsI + 1;i++){
        pos = mix(start, end, iter);
        h = hlower + heightwaterXOLO(pos.xz, zer, mipmapx) * waterdepth;
        if((!isReallyUnderWater && h > pos.y) || (isReallyUnderWater && h < pos.y)) {
            return distance(pos, CameraPosition);
        }
        iter += stepsize;
    }
    return -1.0;
}
float raymarchwater2(vec3 start, vec3 end, int stepsI){
    float stepsize = 1.0 / stepsI;
    float iter = 0;
    vec3 pos = start;
    float h = 0.0;
    float hupper = waterdepth + WaterLevel;
    float hlower = WaterLevel;
    float rd = stepsize * rand2sTimex(UV);
    vec2 zer = vec2(0.0);
    for(int i=0;i<stepsI + 1;i++){
        pos = mix(start, end, iter);
        h = hlower + heightwaterXOLO(pos.xz, zer, mipmapx) * waterdepth;
        if((!isReallyUnderWater && h > pos.y) || (isReallyUnderWater && h < pos.y)) {
        //    return raymarchwater3(mix(start, end, iter - stepsize), mix(start, end, iter + stepsize), 4);
                return distance(pos, CameraPosition);
        }
        iter += stepsize;
    }
    return -1.0;
}
float raymarchwater(vec3 start, vec3 end, int stepsI){

    vec3 pos = start;
    float h = 0.0;
    float hupper = waterdepth + WaterLevel;
    float hlower = WaterLevel;
    vec2 zer = vec2(0.0);
    vec3 dir = normalize(end - start);
    for(int i=0;i<318;i++){
        h = hlower + heightwaterXOLO(pos.xz, zer, mipmapx) * waterdepth;
        if((!isReallyUnderWater && h + 0.01 > pos.y) || (isReallyUnderWater && h - 0.01 < pos.y)) {
            //return raymarchwater2(mix(start, end, iter - stepsize + rd), mix(start, end, iter + stepsize + rd),18);
            return distance(pos, CameraPosition);
        }
        pos += dir * (pos.y - h);
    }
    return -1.0;
}
float rand2sTime(vec2 co){
    co *= Time;
    return fract(sin(dot(co.xy,vec2(12.9898,78.233))) * 43758.5453);
}
float getWaterDistance(){
vec2 derivatives = vec2(dFdx(UV.x), dFdy(UV.y));
vec2 uv = UV;// + derivatives * 1.0 * vec2(rand2sTime(UV), rand2sTime(UV + 1000.0));
    vec3 dir = reconstructCameraSpaceDistance(uv, 1.0);

    float planethit = intersectPlane(CameraPosition, dir, vec3(0.0, waterdepth + WaterLevel, 0.0), vec3(0.0, 1.0, 0.0));

    float planethit2 = intersectPlane(CameraPosition, dir, vec3(0.0, WaterLevel, 0.0), vec3(0.0, 1.0, 0.0));

    //float planethit = rsi2(Ray(toplanetspace(CameraPosition), dir), Sphere(vec3(0.0), planetradius + waterdepth + WaterLevel));
    //float planethit2 =rsi2(Ray(toplanetspace(CameraPosition), dir), Sphere(vec3(0.0), planetradius + WaterLevel));

//rsi2(Ray(toplanetspace(CameraPosition), dir), planet);

    bool hitwater = true;//(planethit > 0.0 || planethit2 > 0.0);// && (length(currentData.normal) < 0.01 || currentData.cameraDistance > planethit);

    isReallyUnderWater = WaterLevel + heightwaterXOLO(CameraPosition.xz, vec2(0.0), 0.0) * waterdepth > CameraPosition.y;

    if(planethit > 0.0 || planethit2 > 0.0){
        float dist = -1.0;
        float wvw = WaterWavesScale / 10.0;
        //int steps = 1;// + int(min(32.0, max(planethit, planethit2) * 0.1) * wvw);

	//	mipmapx = textureQueryLod(waterTileTex, (CameraPosition + dir * min(planethit, planethit2)).xz * WaterScale *  octavescale1).x;
        if(planethit > 0.0 && planethit2 > 0.0){
            //if(min(planethit, planethit2) < 6000.0){
                dist = raymarchwater(CameraPosition  + dir * min(planethit, planethit2), CameraPosition + dir * max(planethit, planethit2), int(17.0 * WaterWavesScale));
            //} else {
            //    dist = max(planethit, planethit2);
            //}//
        } else {
            float H = step(0.0, planethit) * planethit + step(0.0, planethit2) * planethit2;
            dist = raymarchwater(CameraPosition, CameraPosition + dir * min(H, 2999.0), min(2000, int(1.0 * H)));
        }

        if(length(currentData.normal) > 0.01){
            hitwater = currentData.cameraDistance > dist;
        }
        return max(planethit, dist);
    }
    return -1.0;
}


vec4 shade(){
    return vec4(getWaterDistance(), 0.0, 0.0, 0.0);
}
