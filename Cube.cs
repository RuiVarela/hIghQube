using System;
using System.Collections.Generic;
using System.Text;

using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;

namespace Smokin
{
    class Cube
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

        public Cube() { }

        public void render(GraphicsDeviceManager graphics)
        {

            graphics.GraphicsDevice.VertexDeclaration = vertex_declaration;
            graphics.GraphicsDevice.DrawUserPrimitives(PrimitiveType.TriangleList, vertices, 0, 12);
        }
        
        public float size
        {
            get { return size_; }
            set { size_ = value; }
        }

         public void initialize(float size, GraphicsDeviceManager graphics)
        {
            this.size = size;

             float half_size = size / 2.0f;

            Vector3 topLeftFront = new Vector3(-half_size, half_size, half_size);
            Vector3 bottomLeftFront = new Vector3(-half_size, -half_size, half_size);
            Vector3 topRightFront = new Vector3(half_size, half_size, half_size);
            Vector3 bottomRightFront = new Vector3(half_size, -half_size, half_size);
            Vector3 topLeftBack = new Vector3(-half_size, half_size, -half_size);
            Vector3 topRightBack = new Vector3(half_size, half_size, -half_size);
            Vector3 bottomLeftBack = new Vector3(-half_size, -half_size, -half_size);
            Vector3 bottomRightBack = new Vector3(half_size, -half_size, -half_size);



            vertices = new VertexPositionNormal[36];

            // Front face
            vertices[0].Position = topLeftFront;
            vertices[0].Normal = new Vector3(0, 0, 1);
            vertices[1].Position = bottomLeftFront;
            vertices[1].Normal = new Vector3(0, 0, 1);
            vertices[2].Position = topRightFront;
            vertices[2].Normal = new Vector3(0, 0, 1);
            vertices[3].Position = bottomLeftFront;
            vertices[3].Normal = new Vector3(0, 0, 1);
            vertices[4].Position = bottomRightFront;
            vertices[4].Normal = new Vector3(0, 0, 1);
            vertices[5].Position = topRightFront;
            vertices[5].Normal = new Vector3(0, 0, 1);


            // Back face 
            vertices[6].Position = topLeftBack;
            vertices[6].Normal = new Vector3(0, 0, -1);
            vertices[7].Position = topRightBack;
            vertices[7].Normal = new Vector3(0, 0, -1);
            vertices[8].Position = bottomLeftBack;
            vertices[8].Normal = new Vector3(0, 0, -1);
            vertices[9].Position = bottomLeftBack;
            vertices[9].Normal = new Vector3(0, 0, -1);
            vertices[10].Position = topRightBack;
            vertices[10].Normal = new Vector3(0, 0, -1);
            vertices[11].Position = bottomRightBack;
            vertices[11].Normal = new Vector3(0, 0, -1);

            // Top face
            vertices[12].Position = topLeftFront;
            vertices[12].Normal = new Vector3(0, 1, 0);
            vertices[13].Position = topRightBack;
            vertices[13].Normal = new Vector3(0, 1, 0);
            vertices[14].Position = topLeftBack;
            vertices[14].Normal = new Vector3(0, 1, 0);
            vertices[15].Position = topLeftFront;
            vertices[15].Normal = new Vector3(0, 1, 0);
            vertices[16].Position = topRightFront;
            vertices[16].Normal = new Vector3(0, 1, 0);
            vertices[17].Position = topRightBack;
            vertices[17].Normal = new Vector3(0, 1, 0);

            // Bottom face
            vertices[18].Position = bottomLeftFront;
            vertices[18].Normal = new Vector3(0, -1, 0);
            vertices[19].Position = bottomLeftBack;
            vertices[19].Normal = new Vector3(0, -1, 0);
            vertices[20].Position = bottomRightBack;
            vertices[20].Normal = new Vector3(0, -1, 0);
            vertices[21].Position = bottomLeftFront;
            vertices[21].Normal = new Vector3(0, -1, 0);
            vertices[22].Position = bottomRightBack;
            vertices[22].Normal = new Vector3(0, -1, 0);
            vertices[23].Position = bottomRightFront;
            vertices[23].Normal = new Vector3(0, -1, 0);

            // Left face
            vertices[24].Position = topLeftFront;
            vertices[24].Normal = new Vector3(-1, 0, 0);
            vertices[25].Position = bottomLeftBack;
            vertices[25].Normal = new Vector3(-1, 0, 0);
            vertices[26].Position = bottomLeftFront;
            vertices[26].Normal = new Vector3(-1, 0, 0);
            vertices[27].Position = topLeftBack;
            vertices[27].Normal = new Vector3(-1, 0, 0);
            vertices[28].Position = bottomLeftBack;
            vertices[28].Normal = new Vector3(-1, 0, 0);
            vertices[29].Position = topLeftFront;
            vertices[29].Normal = new Vector3(-1, 0, 0);

            // Right face 
            vertices[30].Position = topRightFront;
            vertices[30].Normal = new Vector3(1, 0, 0);
            vertices[31].Position = bottomRightFront;
            vertices[31].Normal = new Vector3(1, 0, 0);
            vertices[32].Position = bottomRightBack;
            vertices[32].Normal = new Vector3(1, 0, 0);
            vertices[33].Position = topRightBack;
            vertices[33].Normal = new Vector3(1, 0, 0);
            vertices[34].Position = topRightFront;
            vertices[34].Normal = new Vector3(1, 0, 0);
            vertices[35].Position = bottomRightBack;
            vertices[35].Normal = new Vector3(1, 0, 0);

            vertex_declaration = new VertexDeclaration(graphics.GraphicsDevice,
                                                        VertexPositionNormal.vertex_elements);
            }
    }
}
