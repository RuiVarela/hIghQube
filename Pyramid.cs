using System;
using System.Collections.Generic;
using System.Text;

using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;

namespace Smokin
{
    class Pyramid
    {
        public struct VertexPositionNormal
         {
             public Vector3 Position;
             public Vector3 Normal;
 
             public static int size_in_bytes = 6 * 4;

             public static VertexElement[] vertex_elements = new VertexElement[]
             {
                 new VertexElement( 0, 0, VertexElementFormat.Vector3, VertexElementMethod.Default, VertexElementUsage.Position, 0 ),
                 new VertexElement( 0, sizeof(float) * 3, VertexElementFormat.Vector3, VertexElementMethod.Default, VertexElementUsage.Normal, 0 ),
             };
        };



        private float size_ = 0;
        private VertexPositionNormal[] vertices;
        private VertexDeclaration vertex_declaration;

        public Pyramid() { }

        public void render(GraphicsDeviceManager graphics)
        {

            graphics.GraphicsDevice.VertexDeclaration = vertex_declaration;
            graphics.GraphicsDevice.DrawUserPrimitives(PrimitiveType.TriangleList, vertices, 0, 6);
        }
        
        public float size
        {
            get { return size_; }
            set { size_ = value; }
        }

         public void initialize(float size, GraphicsDeviceManager graphics)
        {
            this.size = size;
            float height = size;
             float half_size = size / 4.0f;

             Vector3 topLeftFront = new Vector3(-half_size, height, half_size);
             Vector3 topRightFront = new Vector3(half_size, height, half_size);
             Vector3 topLeftBack = new Vector3(-half_size, height, -half_size);
             Vector3 topRightBack = new Vector3(half_size, height, -half_size);
            Vector3 bottom = new Vector3(0.0f, -size / 2.0f, 0.0f);

            vertices = new VertexPositionNormal[36];

            // Front face
            vertices[0].Position = topLeftFront;
            vertices[0].Normal = new Vector3(0, 0, 1);
            vertices[1].Position = bottom;
            vertices[1].Normal = new Vector3(0, 0, 1);
            vertices[2].Position = topRightFront;
            vertices[2].Normal = new Vector3(0, 0, 1);


            // Back face 
            vertices[3].Position = topLeftBack;
            vertices[3].Normal = new Vector3(0, 0, -1);
            vertices[4].Position = topRightBack;
            vertices[4].Normal = new Vector3(0, 0, -1);
            vertices[5].Position = bottom;
            vertices[5].Normal = new Vector3(0, 0, -1);
  

            // Top face
            vertices[6].Position = topLeftFront;
            vertices[6].Normal = new Vector3(0, 1, 0);
            vertices[7].Position = topRightBack;
            vertices[7].Normal = new Vector3(0, 1, 0);
            vertices[8].Position = topLeftBack;
            vertices[8].Normal = new Vector3(0, 1, 0);
            vertices[9].Position = topLeftFront;
            vertices[9].Normal = new Vector3(0, 1, 0);
            vertices[10].Position = topRightFront;
            vertices[10].Normal = new Vector3(0, 1, 0);
            vertices[11].Position = topRightBack;
            vertices[11].Normal = new Vector3(0, 1, 0);

            // Left face

            vertices[12].Position = topLeftBack;
            vertices[12].Normal = new Vector3(-1, 0, 0);
            vertices[13].Position = bottom;
            vertices[13].Normal = new Vector3(-1, 0, 0);
            vertices[14].Position = topLeftFront;
            vertices[14].Normal = new Vector3(-1, 0, 0);

            // Right face 

            vertices[15].Position = topRightBack;
            vertices[15].Normal = new Vector3(1, 0, 0);
            vertices[16].Position = topRightFront;
            vertices[16].Normal = new Vector3(1, 0, 0);
            vertices[17].Position = bottom;
            vertices[17].Normal = new Vector3(1, 0, 0);

            vertex_declaration = new VertexDeclaration(graphics.GraphicsDevice,
                                                        VertexPositionNormal.vertex_elements);
            }
    }
}
