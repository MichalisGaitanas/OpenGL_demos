#version 450 core

#define POISSON_SAMPLES 16

in vec3 frag_pos_world;
in vec4 frag_pos_light;
in vec3 normal;

out vec4 frag_col; // Final color of the fragment after lighting calculations.

uniform vec3 mesh_col;          // Mesh color.
uniform vec3 light_dir;         // Direction of the light in world coordinates.
uniform vec3 light_col;         // Light color.
uniform sampler2D sample_shadow; // Depth image texture from shadow mapping.

uniform vec3 cam_pos;   // Camera's position in world coordinates.

vec2 poisson_disk[POISSON_SAMPLES] = vec2[](
    vec2(-0.94201624, -0.39906216), 
    vec2( 0.94558609, -0.76890725), 
    vec2(-0.09418410, -0.92938870), 
    vec2( 0.34495938,  0.29387760), 
    vec2(-0.91588581,  0.45771432), 
    vec2(-0.81544232, -0.87912464), 
    vec2(-0.38277543,  0.27676845), 
    vec2( 0.97484398,  0.75648379), 
    vec2( 0.44323325, -0.97511554), 
    vec2( 0.53742981, -0.47373420), 
    vec2(-0.26496911, -0.41893023), 
    vec2( 0.79197514,  0.19090188), 
    vec2(-0.24188840,  0.99706507), 
    vec2(-0.81409955,  0.91437590), 
    vec2( 0.19984126,  0.78641367), 
    vec2( 0.14383161, -0.14100790)
);

// Function to determine shadow factor.
float get_shadow(vec3 N, vec3 L)
{
    vec3 projected_coords = frag_pos_light.xyz / frag_pos_light.w;
    projected_coords = 0.5 * projected_coords + vec3(0.5);

    if (projected_coords.x < 0.0 || projected_coords.x > 1.0 ||
        projected_coords.y < 0.0 || projected_coords.y > 1.0 ||
        projected_coords.z > 1.0)
    {
        return 0.0; // No shadow.
    }

    float min_bias = 0.0006;
    float amplifier = 0.006;
    float bias = max(amplifier * (1.0 - max(dot(N, L), 0.0)), min_bias);

    vec2 texel_size = 1.0 / textureSize(sample_shadow, 0);
    float shadow = 0.0;
    for (int i = 0; i < POISSON_SAMPLES; ++i)
    {
        vec2 offset = texel_size * poisson_disk[i];
        vec2 random_offset = (fract(sin(dot(frag_pos_world.xy, vec2(12.9898, 78.233))) * 43758.5453)) * texel_size * 0.5;
        float nearest_frag_depth = texture(sample_shadow, projected_coords.xy + offset + random_offset).r;
        if (projected_coords.z - bias > nearest_frag_depth)
        {
            shadow += 1.0;
        }
    }
    return shadow / float(POISSON_SAMPLES);
}

void main()
{
    // Normalize vectors
    vec3 N = normalize(normal);
    vec3 L = normalize(light_dir);
    vec3 V = normalize(cam_pos - frag_pos_world);

    // Compute cosines of angles
    float cosThetaI = max(dot(N, L), 0.0);
    float cosThetaR = max(dot(N, V), 0.0);

    // Compute sines of angles
    float sinThetaI = sqrt(max(0.0, 1.0 - cosThetaI * cosThetaI));
    float sinThetaR = sqrt(max(0.0, 1.0 - cosThetaR * cosThetaR));

    // Adjust thetaI and thetaR to avoid singularities
    float epsilon = 0.01;
    float thetaI = min(acos(cosThetaI), 1.57079633 - epsilon); //pi/2
    float thetaR = min(acos(cosThetaR), 1.57079633 - epsilon);

    // Compute alpha and beta
    float alpha = max(thetaI, thetaR);
    float beta = min(thetaI, thetaR);

    // Compute cosDeltaPhi directly
    float cosDeltaPhi = 0.0;
    if (sinThetaI > 0.001 && sinThetaR > 0.001)
    {
        vec3 L_perp = (L - N * cosThetaI) / sinThetaI;
        vec3 V_perp = (V - N * cosThetaR) / sinThetaR;
        cosDeltaPhi = dot(L_perp, V_perp);
        cosDeltaPhi = clamp(cosDeltaPhi, -1.0, 1.0);
    }

    // Precompute A and B for the Oren-Nayar model
    float sigma2 = 0.8 * 0.8;
    float A = 1.0 - (sigma2 / (2.0 * (sigma2 + 0.33)));
    float B = 0.45 * sigma2 / (sigma2 + 0.09);

    // Compute tanBeta safely
    float tanBeta = tan(beta);
    tanBeta = clamp(tanBeta, 0.0, 100.0);

    // Compute sinAlpha
    float sinAlpha = sin(alpha);

    // Calculate the Oren-Nayar diffuse term
    float orenNayarDiffuse = A + B * cosDeltaPhi * sinAlpha * tanBeta;
    orenNayarDiffuse *= cosThetaI;

    // Calculate the shadow factor
    float shadowFactor = 1.0 - get_shadow(N, L);

    // Compute the phase angle
    float cosPhaseAngle = clamp(dot(L, V), -1.0, 1.0);

    // Apply the phase function
    float k = 0.5; // Strength of the opposition effect
    float n = 2.0; // Sharpness of the effect
    float phaseFunction = 1.0 + k * pow(max(0.0, cosPhaseAngle), n);

    // Final color calculation
    vec3 finalColor = phaseFunction * (shadowFactor * orenNayarDiffuse) * mesh_col * light_col;
    frag_col = vec4(finalColor, 1.0);
}
