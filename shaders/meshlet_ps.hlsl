struct PSInput {
    float3 Color: COLOR0;
};

struct PSOutput {
    float4 Color: SV_TARGET;
};

PSOutput ps_main(PSInput input) {
    PSOutput output;
    output.Color = float4(input.Color, 1.0);
    return output;
}