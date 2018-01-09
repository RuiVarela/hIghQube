struct VertexToPixel
{
    float4 Position     : POSITION;
    float4 Color        : COLOR0;
    float LightAmount   : TEXCOORD1;
};

float3 camera_position;
float4 color;


float4x4 model;
float4x4 view; 
float4x4 projection;

float elapsed_time;  // Time elapsed

float speed_factor = 5.0;
float min_alpha = 0.7;
float max_alpha = 0.9;

float scale = 1.0;



VertexToPixel vertexShader(float4 in_position : POSITION, float4 in_normal: NORMAL)
{
    VertexToPixel Output = (VertexToPixel)0;   
    
    in_position.x *= scale;
    in_position.y *= scale;
    in_position.z *= scale;
    
    Output.Position = mul(mul(mul(in_position, model), view), projection);
    Output.Color = color;
    
    in_normal.w = 0.0;
    

	float3 world_vertex_position = mul(in_position, model);
	float3 light_direction = normalize(camera_position - world_vertex_position);
	
	
	float3 world_normal = mul(in_normal, model);
    Output.LightAmount = dot(world_normal, light_direction);
    
    return Output;    
}

float4 pixelShader(float4 shader_color : COLOR, float light_amount : TEXCOORD1) : COLOR0
{
	
	float factor = (0.5 + (sin(elapsed_time * speed_factor + 1.0) / 2.0)) * (max_alpha - min_alpha);
	
	shader_color.a = min_alpha + factor;
	
	shader_color.rgb *= saturate(light_amount);
	
    return shader_color;
}

float4 vanishPixelShader(float4 shader_color : COLOR, float light_amount : TEXCOORD1) : COLOR0
{	
	float factor = (0.5 + (sin(elapsed_time * speed_factor + 1.0) / 2.0)) * (max_alpha - min_alpha);
	
	shader_color.a = 2.0f - scale;
	
	shader_color.rgb *= saturate(light_amount);
	
    return shader_color;
}



VertexToPixel outlineVertexShader(float4 in_position : POSITION)
{
    VertexToPixel Output = (VertexToPixel)0;
    
    float4 scaled_position = in_position;
    float ouline_scale = 1.03;
    scaled_position.x *= ouline_scale;
    scaled_position.y *= ouline_scale;
    scaled_position.z *= ouline_scale;
            
    Output.Position = mul(mul(mul(scaled_position, model), view), projection);
    Output.Color.r = 1.0f;
    Output.Color.g = 0.0f;
    Output.Color.b = 0.0f;
    Output.Color.a = 1.0f;
    
    
    return Output;    
}

float4 outlinePixelShader(float4 shader_color : COLOR) : COLOR0
{
shader_color.a = 1.0;
    return shader_color;
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
          VertexShader = compile vs_2_0 outlineVertexShader();
          
          //set the PixelShader state to the pixel shader function          
		  PixelShader = compile ps_2_0 outlinePixelShader(); 
    }
    
    pass Pass2
    {
          //set the VertexShader state to the vertex shader function
          VertexShader = compile vs_2_0 vertexShader();
          
          //set the PixelShader state to the pixel shader function          
		  PixelShader = compile ps_2_0 vanishPixelShader(); 
    }
}