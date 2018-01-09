struct VertexToPixel
{
    float4 Position     : POSITION;
    float4 Color        : COLOR0;
};

float4x4 world_view_projection;  // World * View * Projection transformation
float elapsed_time;  // Time elapsed

float speed_factor = 1.0;
float min_alpha = 0.1;
float max_alpha = 0.2;

VertexToPixel vertexShader(float4 in_position : POSITION, float3 in_normal : NORMAL)
{
    VertexToPixel Output = (VertexToPixel)0;
    
    Output.Position = mul(in_position, world_view_projection);
 //   Output.Color = in_color;   
 
   Output.Color.r = 0.0;  
   Output.Color.g = 0.0; 
   Output.Color.b = 1.0; 
   Output.Color.a = 1.0; 
    
    return Output;    
}

float4 pixelShader(float4 in_color : COLOR0) : COLOR0
{
	float4 out_color = in_color;
	
	float factor = (0.5 + (sin(elapsed_time * speed_factor) / 2.0)) * (max_alpha - min_alpha);
	
	out_color.a = min_alpha + factor;
	
    return out_color;
}

technique FieldTechnique
{
    pass Pass0
    {
          //set the VertexShader state to the vertex shader function
          VertexShader = compile vs_2_0 vertexShader();
          
          //set the PixelShader state to the pixel shader function          
		  PixelShader = compile ps_2_0 pixelShader(); 
    }
    
    pass Pass1
    {
          //set the VertexShader state to the vertex shader function
          VertexShader = compile vs_2_0 vertexShader();
          
          //set the PixelShader state to the pixel shader function          
		  PixelShader = compile ps_2_0 pixelShader(); 
    }
}